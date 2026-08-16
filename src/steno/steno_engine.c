/* SPDX-License-Identifier: MIT */

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include <cornix_steno/decoder.h>
#include <cornix_steno/dictionary.h>
#include <cornix_steno/engine.h>
#include <cornix_steno/led.h>
#include <cornix_steno/output.h>
#include <cornix_steno/quick.h>
#include <cornix_steno/roles.h>

LOG_MODULE_REGISTER(cornix_steno_engine, CONFIG_ZMK_LOG_LEVEL);

BUILD_ASSERT(CONFIG_CORNIX_STENO_MAX_POSITIONS <= 64,
             "Cornix STENO physical-position mask is 64-bit");
BUILD_ASSERT(CST_R_COUNT <= 64, "Cornix STENO logical-role mask is 64-bit");

struct steno_state {
    bool active;
    uint64_t down_mask;
    uint64_t accepted_mask;
    uint64_t late_added_mask;
    uint64_t release_candidate_mask;
    enum cornix_steno_role position_role[CONFIG_CORNIX_STENO_MAX_POSITIONS];
    int64_t pressed_at[CONFIG_CORNIX_STENO_MAX_POSITIONS];
    int64_t released_at[CONFIG_CORNIX_STENO_MAX_POSITIONS];
    int64_t first_press_at;

};

static struct steno_state state;
K_MUTEX_DEFINE(steno_mutex);
static struct k_work_delayable correction_work;

static uint64_t roles_from_positions_locked(uint64_t position_mask) {
    uint64_t roles = 0;
    for (uint32_t position = 0; position < CONFIG_CORNIX_STENO_MAX_POSITIONS; position++) {
        if (!(position_mask & CST_POSITION_BIT(position))) continue;
        const enum cornix_steno_role role = state.position_role[position];
        if (role > CST_R_NONE && role < CST_R_COUNT) roles |= CST_ROLE_BIT(role);
    }
    return roles;
}

static void reset_stroke_locked(void) {
    state.down_mask = 0;
    state.accepted_mask = 0;
    state.late_added_mask = 0;
    state.release_candidate_mask = 0;
    state.first_press_at = 0;
    memset(state.position_role, 0, sizeof(state.position_role));
    memset(state.pressed_at, 0, sizeof(state.pressed_at));
    memset(state.released_at, 0, sizeof(state.released_at));
}


static void process_stroke(uint64_t position_mask, uint64_t role_mask, int64_t timestamp) {
    ARG_UNUSED(timestamp);
#if IS_ENABLED(CONFIG_CORNIX_STENO_DICTIONARY)
    const struct cornix_steno_quick_entry *quick = cornix_steno_quick_lookup(role_mask);
    if (quick) {
        const int quick_err = cornix_steno_quick_invoke(quick, timestamp);
        if (quick_err < 0) {
            LOG_WRN("Quick abbreviation macro %s failed: %d", quick->slot, quick_err);
        }
        return; /* The fixed exact-mask is always consumed, even for a blank macro. */
    }
#endif

    struct cornix_steno_decoded decoded;
    const int err = cornix_steno_decode(role_mask, position_mask, &decoded);
    if (err < 0) {
        LOG_WRN("STENO decode error: %d", err);
        return;
    }
    if (decoded.kind == CST_DECODE_KEYS) {
        cornix_steno_output_enqueue_sequence(decoded.keys, decoded.key_count);
        return;
    }

#if IS_ENABLED(CONFIG_CORNIX_STENO_DICTIONARY)
    /*
     * Canonical dictionary lookup uses the complete logical role mask.
     * Direct consonant chords, ABBR_L, ABBR_R and AB2 are distinct.  Only
     * AB2 is side-flexible; the dictionary module normalizes one VEXT side.
     */
    const struct cornix_steno_dictionary_entry *entry =
        cornix_steno_dictionary_lookup(role_mask);
    if (entry) {
        cornix_steno_output_enqueue_sequence(entry->keys, entry->key_count);
        return;
    }
    LOG_DBG("No exact canonical STENO abbreviation: mask=0x%llx",
            (unsigned long long)role_mask);
#endif
    /* Unknown chords are intentionally silent rather than guessed. */
}

static void schedule_next_correction_locked(int64_t now) {
    int64_t next_delay = INT64_MAX;
    for (uint32_t position = 0; position < CONFIG_CORNIX_STENO_MAX_POSITIONS; position++) {
        const uint64_t bit = CST_POSITION_BIT(position);
        if (!(state.release_candidate_mask & bit)) continue;
        const int64_t elapsed = now - state.released_at[position];
        const int64_t remaining = CONFIG_CORNIX_STENO_KEYUP_CORRECTION_MS - elapsed;
        if (remaining < next_delay) next_delay = remaining;
    }
    if (next_delay != INT64_MAX) {
        k_work_reschedule(&correction_work, K_MSEC(MAX(next_delay, 1)));
    }
}

static bool dictionary_anchor_active_locked(void) {
    const uint64_t roles = roles_from_positions_locked(state.accepted_mask);
    const bool has_abbr = (roles & CST_ABBR_MASK) != 0;
    const bool has_vext_selector = (roles & CST_VEXT_MASK) != 0 &&
                                   (roles & CST_BASE_VOWEL_MASK) == 0 &&
                                   (roles & (CST_INITIAL_MASK | CST_FINAL_MASK)) != 0;
#if IS_ENABLED(CONFIG_CORNIX_STENO_DICTIONARY)
    /*
     * Direct canonical abbreviations have no bank key.  Once the complete
     * accepted mask is an exact entry, do not let 50 ms late-key correction
     * shrink a longer word (for example 그리고) into a shorter valid word.
     */
    const bool exact_dictionary = cornix_steno_dictionary_has_exact(roles);
#else
    const bool exact_dictionary = false;
#endif
    return has_abbr || has_vext_selector || exact_dictionary;
}

static void correction_work_handler(struct k_work *work) {
    ARG_UNUSED(work);
    const int64_t now = k_uptime_get();

    k_mutex_lock(&steno_mutex, K_FOREVER);
    if (!state.active || state.down_mask == 0) {
        state.release_candidate_mask = 0;
        k_mutex_unlock(&steno_mutex);
        return;
    }

    /*
     * Selector keys remain usable as held anchors, and a fully assembled
     * direct canonical dictionary mask is protected as well.  This preserves
     * sequential selector input and prevents a long direct exact mask from
     * being reduced to a shorter registered word by key-up correction.
     */
    if (dictionary_anchor_active_locked()) {
        state.release_candidate_mask = 0;
        k_mutex_unlock(&steno_mutex);
        return;
    }

    for (uint32_t position = 0; position < CONFIG_CORNIX_STENO_MAX_POSITIONS; position++) {
        const uint64_t bit = CST_POSITION_BIT(position);
        if (!(state.release_candidate_mask & bit)) continue;
        if (state.down_mask & bit) {
            state.release_candidate_mask &= ~bit; /* re-pressed */
            continue;
        }
        if (now - state.released_at[position] >= CONFIG_CORNIX_STENO_KEYUP_CORRECTION_MS) {
            state.accepted_mask &= ~bit;
            state.late_added_mask &= ~bit;
            state.release_candidate_mask &= ~bit;
            LOG_DBG("Removed late STENO position %u", position);
        }
    }
    schedule_next_correction_locked(now);
    k_mutex_unlock(&steno_mutex);
}

static int engine_init(void) {
    k_work_init_delayable(&correction_work, correction_work_handler);
    return 0;
}
SYS_INIT(engine_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

void cornix_steno_engine_set_active(bool active) {
    k_work_cancel_delayable(&correction_work);
    k_mutex_lock(&steno_mutex, K_FOREVER);
    state.active = active;
    reset_stroke_locked();
    k_mutex_unlock(&steno_mutex);

    /* Flash all indicators on STENO entry; restore BASE status on exit. */
    cornix_steno_led_mode_changed(active);
}

bool cornix_steno_engine_is_active(void) {
    k_mutex_lock(&steno_mutex, K_FOREVER);
    const bool active = state.active;
    k_mutex_unlock(&steno_mutex);
    return active;
}

bool cornix_steno_engine_has_down_keys(void) {
    k_mutex_lock(&steno_mutex, K_FOREVER);
    const bool has_down = state.down_mask != 0;
    k_mutex_unlock(&steno_mutex);
    return has_down;
}

void cornix_steno_engine_cancel_pending(void) {
    k_work_cancel_delayable(&correction_work);
    k_mutex_lock(&steno_mutex, K_FOREVER);
    reset_stroke_locked();
    k_mutex_unlock(&steno_mutex);

    /* Clear any physical-key LED state left by a cancelled stroke. */
    cornix_steno_led_reset_keys();
}

int cornix_steno_engine_role_pressed(enum cornix_steno_role role, uint32_t position,
                                     int64_t timestamp) {
    if (role <= CST_R_NONE || role >= CST_R_COUNT ||
        position >= CONFIG_CORNIX_STENO_MAX_POSITIONS) {
        return -EINVAL;
    }

    const uint64_t bit = CST_POSITION_BIT(position);
    k_mutex_lock(&steno_mutex, K_FOREVER);
    if (!state.active || (state.down_mask & bit)) {
        k_mutex_unlock(&steno_mutex);
        return 0;
    }

    if (state.down_mask == 0) {
        reset_stroke_locked();
        state.first_press_at = timestamp;
    } else if (timestamp - state.first_press_at > CONFIG_CORNIX_STENO_CHORD_SETTLE_MS) {
        state.late_added_mask |= bit;
    }

    state.release_candidate_mask &= ~bit;
    state.down_mask |= bit;
    state.accepted_mask |= bit;
    state.position_role[position] = role;
    state.pressed_at[position] = timestamp;
    k_mutex_unlock(&steno_mutex);
    return 0;
}

int cornix_steno_engine_role_released(enum cornix_steno_role role, uint32_t position,
                                      int64_t timestamp) {
    ARG_UNUSED(role);
    if (position >= CONFIG_CORNIX_STENO_MAX_POSITIONS) return -EINVAL;

    const uint64_t bit = CST_POSITION_BIT(position);
    uint64_t final_positions = 0;
    uint64_t final_roles = 0;
    bool finalize = false;

    k_mutex_lock(&steno_mutex, K_FOREVER);
    if (!state.active || !(state.down_mask & bit)) {
        k_mutex_unlock(&steno_mutex);
        return 0;
    }

    state.down_mask &= ~bit;
    const int64_t held_ms = timestamp - state.pressed_at[position];
    if ((state.late_added_mask & bit) && state.down_mask != 0 && held_ms >= 0 &&
        held_ms <= CONFIG_CORNIX_STENO_LATE_TAP_MAX_MS) {
        state.release_candidate_mask |= bit;
        state.released_at[position] = timestamp;
        schedule_next_correction_locked(timestamp);
    }

    if (state.down_mask == 0) {
        final_positions = state.accepted_mask;
        final_roles = roles_from_positions_locked(final_positions);
        reset_stroke_locked();
        finalize = true;
    }
    k_mutex_unlock(&steno_mutex);

    if (finalize) {
        k_work_cancel_delayable(&correction_work);
        if (final_positions && final_roles) {
            process_stroke(final_positions, final_roles, timestamp);
        }
    }
    return 0;
}

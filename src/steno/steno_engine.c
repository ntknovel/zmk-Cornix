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
#include <dt-bindings/zmk/keys.h>
#include <zmk/events/keycode_state_changed.h>

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

/* Fixed Cornix positions for the physical ㅣ+JLKI immediate navigation path. */
#define CST_NAV_POS_I UINT32_C(8)
#define CST_NAV_POS_J UINT32_C(19)
#define CST_NAV_POS_K UINT32_C(20)
#define CST_NAV_POS_L UINT32_C(21)
#define CST_NAV_VOWEL_POSITION UINT32_C(45)

enum anchored_stream_mode {
    CST_STREAM_NONE = 0,
    CST_STREAM_NAV_LEFT,
    CST_STREAM_NAV_RIGHT,
    CST_STREAM_SELECT_LEFT,
    CST_STREAM_SELECT_RIGHT,
    CST_STREAM_NUMBER,
    CST_STREAM_CORRECTION,
    CST_STREAM_UNDO,
    CST_STREAM_REDO,
};

struct steno_state {
    bool active;

    /* Normal exact-mask stroke. */
    uint64_t down_mask;
    uint64_t accepted_mask;
    uint64_t release_candidate_mask;
    uint64_t final_roll_pending_mask;
    enum cornix_steno_role position_role[CONFIG_CORNIX_STENO_MAX_POSITIONS];
    int64_t released_at[CONFIG_CORNIX_STENO_MAX_POSITIONS];
    int64_t final_roll_released_at[CONFIG_CORNIX_STENO_MAX_POSITIONS];
    int64_t pressed_at[CONFIG_CORNIX_STENO_MAX_POSITIONS];
    bool stroke_had_multiple;

    /* A valid vowel-only or double-consonant chord held for the full 80 ms
     * is latched on its first key-up.  The latch preserves every role through
     * later key-up correction and gives direct-jamo output priority over a
     * short-stroke dictionary abbreviation. */
    bool direct_hold_latched;
    uint64_t direct_hold_position_mask;
    uint64_t direct_hold_role_mask;

    /* Physical ㅣ+JLKI mode. */
    bool physical_nav_mode;
    bool physical_nav_vowel_down;
    uint64_t physical_nav_down_mask;

    /* ABBR/SYMBOL anchored repeat navigation or continuous correction. */
    enum anchored_stream_mode stream_mode;
    enum cornix_steno_role stream_anchor_role;
    uint32_t stream_anchor_position;
    bool stream_anchor_down;
    uint64_t stream_unit_down_mask;
    uint64_t stream_unit_seen_mask;
    uint64_t stream_unit_role_mask;
};

static struct steno_state state;
K_MUTEX_DEFINE(steno_mutex);
static struct k_work_delayable correction_work;

static bool is_abbr_role(enum cornix_steno_role role) {
    return role == CST_R_ABBR_L || role == CST_R_ABBR_R;
}

static bool is_symbol_role(enum cornix_steno_role role) {
    return role == CST_R_SYMBOL_L || role == CST_R_SYMBOL_R;
}

/* Double-consonant aliases: left ㅋ/ㅇ/, and mirrored right 종ㅋ/종ㅇ/.
 * The dedicated I/F_DOUBLE role remains the canonical dictionary bit. */
static bool role_is_dedicated_double_for_anchor(enum cornix_steno_role anchor,
                                                  enum cornix_steno_role role) {
    if (anchor == CST_R_ABBR_L || anchor == CST_R_SYMBOL_L)
        return role == CST_R_I_DOUBLE;
    if (anchor == CST_R_ABBR_R || anchor == CST_R_SYMBOL_R)
        return role == CST_R_F_DOUBLE;
    return false;
}


static uint32_t deferred_abbr_navigation_key(uint64_t roles) {
    const uint64_t left = CST_ROLE_BIT(CST_R_ABBR_L) | CST_ROLE_BIT(CST_R_I_DOUBLE);
    const uint64_t right = CST_ROLE_BIT(CST_R_ABBR_R) | CST_ROLE_BIT(CST_R_F_DOUBLE);
    if (roles == left) return LEFT;
    if (roles == right) return RIGHT;
    return 0;
}
static enum anchored_stream_mode stream_mode_for_anchor(enum cornix_steno_role role) {
    switch (role) {
    case CST_R_ABBR_L: return CST_STREAM_NAV_LEFT;
    case CST_R_ABBR_R: return CST_STREAM_NAV_RIGHT;
    case CST_R_SYMBOL_L: return CST_STREAM_SELECT_LEFT;
    case CST_R_SYMBOL_R: return CST_STREAM_SELECT_RIGHT;
    default: return CST_STREAM_NONE;
    }
}

static uint32_t stream_key_for_mode(enum anchored_stream_mode mode) {
    switch (mode) {
    case CST_STREAM_NAV_LEFT: return LEFT;
    case CST_STREAM_NAV_RIGHT: return RIGHT;
    case CST_STREAM_SELECT_LEFT: return LS(LEFT);
    case CST_STREAM_SELECT_RIGHT: return LS(RIGHT);
    case CST_STREAM_UNDO: return LC(Z);
    case CST_STREAM_REDO: return LC(Y);
    default: return 0;
    }
}


static uint32_t number_key_for_top_position(uint32_t position) {
    switch (position) {
    case 1: return N1; case 2: return N2; case 3: return N3; case 4: return N4; case 5: return N5;
    case 6: return N6; case 7: return N7; case 8: return N8; case 9: return N9; case 10: return N0;
    default: return 0;
    }
}

static enum anchored_stream_mode edit_stream_mode_for_pair(enum cornix_steno_role a,
                                                            enum cornix_steno_role b) {
    if ((a == CST_R_V_O && b == CST_R_V_U) ||
        (a == CST_R_V_U && b == CST_R_V_O)) return CST_STREAM_UNDO;
    if ((a == CST_R_V_A && b == CST_R_V_EO) ||
        (a == CST_R_V_EO && b == CST_R_V_A)) return CST_STREAM_REDO;
    return CST_STREAM_NONE;
}

static enum cornix_steno_role edit_stream_counterpart(enum anchored_stream_mode mode,
                                                       enum cornix_steno_role anchor) {
    if (mode == CST_STREAM_UNDO)
        return anchor == CST_R_V_O ? CST_R_V_U : CST_R_V_O;
    if (mode == CST_STREAM_REDO)
        return anchor == CST_R_V_A ? CST_R_V_EO : CST_R_V_A;
    return CST_R_NONE;
}

static bool role_mask_is_direct_vowel_hold(uint64_t roles) {
    if (!roles || (roles & ~(CST_BASE_VOWEL_MASK | CST_VEXT_MASK))) return false;
    /* Undo/Redo are commands, not direct vowel chords. */
    if (roles == (CST_ROLE_BIT(CST_R_V_O) | CST_ROLE_BIT(CST_R_V_U)) ||
        roles == (CST_ROLE_BIT(CST_R_V_A) | CST_ROLE_BIT(CST_R_V_EO))) return false;
    struct cornix_steno_decoded decoded;
    return cornix_steno_decode_correction_unit(roles, &decoded) == 0 &&
           decoded.kind == CST_DECODE_KEYS && decoded.key_count;
}

static bool role_mask_is_direct_double_hold(uint64_t roles) {
    const uint64_t i_alias = CST_ROLE_BIT(CST_R_I_DOUBLE) | CST_ROLE_BIT(CST_R_I_KH) |
                             CST_ROLE_BIT(CST_R_I_NG);
    const uint64_t i_target = CST_ROLE_BIT(CST_R_I_B) | CST_ROLE_BIT(CST_R_I_J) |
                              CST_ROLE_BIT(CST_R_I_D) | CST_ROLE_BIT(CST_R_I_G) |
                              CST_ROLE_BIT(CST_R_I_S);
    const uint64_t f_alias = CST_ROLE_BIT(CST_R_F_DOUBLE) | CST_ROLE_BIT(CST_R_F_KH) |
                             CST_ROLE_BIT(CST_R_F_NG);
    const uint64_t f_target = CST_ROLE_BIT(CST_R_F_B) | CST_ROLE_BIT(CST_R_F_J) |
                              CST_ROLE_BIT(CST_R_F_D) | CST_ROLE_BIT(CST_R_F_G) |
                              CST_ROLE_BIT(CST_R_F_S);
    const bool initial = !(roles & ~CST_INITIAL_MASK) && (roles & i_alias) &&
                         __builtin_popcountll(roles & i_target) == 1;
    const bool final = !(roles & ~CST_FINAL_MASK) && (roles & f_alias) &&
                       __builtin_popcountll(roles & f_target) == 1;
    if (!initial && !final) return false;
    struct cornix_steno_decoded decoded;
    return cornix_steno_decode_correction_unit(roles, &decoded) == 0 &&
           decoded.kind == CST_DECODE_KEYS && decoded.key_count;
}

static uint32_t physical_nav_key_for_position(uint32_t position) {
    switch (position) {
    case CST_NAV_POS_I: return UP;
    case CST_NAV_POS_J: return LEFT;
    case CST_NAV_POS_K: return DOWN;
    case CST_NAV_POS_L: return RIGHT;
    default: return 0;
    }
}

static int emit_key_tap(uint32_t key, int64_t timestamp) {
    int err = raise_zmk_keycode_state_changed_from_encoded(key, true, timestamp);
    if (err < 0) return err;
    return raise_zmk_keycode_state_changed_from_encoded(key, false, timestamp);
}

static uint64_t roles_from_positions_locked(uint64_t position_mask) {
    uint64_t roles = 0;
    for (uint32_t position = 0; position < CONFIG_CORNIX_STENO_MAX_POSITIONS; position++) {
        if (!(position_mask & CST_POSITION_BIT(position))) continue;
        const enum cornix_steno_role role = state.position_role[position];
        if (role > CST_R_NONE && role < CST_R_COUNT) roles |= CST_ROLE_BIT(role);
    }
    return roles;
}

static void reset_normal_stroke_locked(void) {
    state.down_mask = 0;
    state.accepted_mask = 0;
    state.release_candidate_mask = 0;
    state.final_roll_pending_mask = 0;
    memset(state.position_role, 0, sizeof(state.position_role));
    memset(state.released_at, 0, sizeof(state.released_at));
    memset(state.final_roll_released_at, 0, sizeof(state.final_roll_released_at));
    memset(state.pressed_at, 0, sizeof(state.pressed_at));
    state.stroke_had_multiple = false;
    state.direct_hold_latched = false;
    state.direct_hold_position_mask = 0;
    state.direct_hold_role_mask = 0;
}

static void reset_stream_locked(void) {
    state.stream_mode = CST_STREAM_NONE;
    state.stream_anchor_role = CST_R_NONE;
    state.stream_anchor_position = UINT32_MAX;
    state.stream_anchor_down = false;
    state.stream_unit_down_mask = 0;
    state.stream_unit_seen_mask = 0;
    state.stream_unit_role_mask = 0;
}

static size_t collect_physical_nav_release_keys_locked(uint32_t keys[4]) {
    size_t count = 0;
    for (uint32_t position = 0; position < CONFIG_CORNIX_STENO_MAX_POSITIONS; position++) {
        const uint64_t bit = CST_POSITION_BIT(position);
        if (!(state.physical_nav_down_mask & bit)) continue;
        const uint32_t key = physical_nav_key_for_position(position);
        if (key && count < 4) keys[count++] = key;
    }
    state.physical_nav_down_mask = 0;
    state.physical_nav_mode = false;
    state.physical_nav_vowel_down = false;
    return count;
}

static enum cornix_steno_abbreviation_category category_for_role_mask(uint64_t role_mask) {
#if IS_ENABLED(CONFIG_CORNIX_STENO_DICTIONARY)
    if (cornix_steno_quick_lookup(role_mask)) return CST_ABBR_CATEGORY_BASIC;
    const struct cornix_steno_dictionary_entry *entry =
        cornix_steno_dictionary_lookup(role_mask);
    if (entry) return (enum cornix_steno_abbreviation_category)entry->category;
#else
    ARG_UNUSED(role_mask);
#endif
    return CST_ABBR_CATEGORY_NONE;
}

static enum cornix_steno_abbreviation_category category_from_state_locked(void) {
    if (!state.accepted_mask) return CST_ABBR_CATEGORY_NONE;
    return category_for_role_mask(roles_from_positions_locked(state.accepted_mask));
}

static void process_stroke(uint64_t position_mask, uint64_t role_mask, int64_t timestamp) {
#if IS_ENABLED(CONFIG_CORNIX_STENO_DICTIONARY)
    const struct cornix_steno_quick_entry *quick = cornix_steno_quick_lookup(role_mask);
    if (quick) {
        cornix_steno_led_confirm_category(CST_ABBR_CATEGORY_BASIC);
        const int quick_err = cornix_steno_quick_invoke(quick, timestamp);
        if (quick_err < 0) LOG_WRN("Quick abbreviation macro %s failed: %d", quick->slot, quick_err);
        return;
    }
#endif

    struct cornix_steno_decoded decoded;
    const int err = cornix_steno_decode(role_mask, position_mask, &decoded);
    if (err < 0) {
        cornix_steno_led_set_category(CST_ABBR_CATEGORY_NONE);
        LOG_WRN("STENO decode error: %d", err);
        return;
    }
    if (decoded.kind == CST_DECODE_KEYS) {
        cornix_steno_led_set_category(CST_ABBR_CATEGORY_NONE);
        cornix_steno_output_enqueue_sequence(decoded.keys, decoded.key_count);
        return;
    }

#if IS_ENABLED(CONFIG_CORNIX_STENO_DICTIONARY)
    const struct cornix_steno_dictionary_entry *entry =
        cornix_steno_dictionary_lookup(role_mask);
    if (entry) {
        cornix_steno_led_confirm_category(
            (enum cornix_steno_abbreviation_category)entry->category);
        cornix_steno_output_enqueue_sequence(entry->keys, entry->key_count);
        return;
    }
#endif
    cornix_steno_led_set_category(CST_ABBR_CATEGORY_NONE);
}

static void process_correction_unit(uint64_t role_mask) {
    struct cornix_steno_decoded decoded;
    if (cornix_steno_decode_correction_unit(role_mask, &decoded) == 0 &&
        decoded.kind == CST_DECODE_KEYS && decoded.key_count) {
        cornix_steno_output_enqueue_sequence(decoded.keys, decoded.key_count);
    }
}

int cornix_steno_engine_emit_direct_role(enum cornix_steno_role role) {
    if (role <= CST_R_NONE || role >= CST_R_COUNT) return -EINVAL;
    struct cornix_steno_decoded decoded;
    if (cornix_steno_decode_correction_unit(CST_ROLE_BIT(role), &decoded) < 0 ||
        decoded.kind != CST_DECODE_KEYS || !decoded.key_count) return -EINVAL;
    return cornix_steno_output_enqueue_sequence(decoded.keys, decoded.key_count);
}

static bool bit_can_roll_final(enum cornix_steno_role role) {
    switch (role) {
    case CST_R_F_G: case CST_R_F_S: case CST_R_F_N: case CST_R_F_J:
    case CST_R_F_H: case CST_R_F_R: case CST_R_F_M: case CST_R_F_B:
    case CST_R_F_T: case CST_R_F_P:
        return true;
    default:
        return false;
    }
}

static bool is_valid_final_roll_pair(enum cornix_steno_role a,
                                     enum cornix_steno_role b) {
    const uint64_t pair = CST_ROLE_BIT(a) | CST_ROLE_BIT(b);
#define FINAL_PAIR(x, y) (CST_ROLE_BIT(x) | CST_ROLE_BIT(y))
    switch (pair) {
    case FINAL_PAIR(CST_R_F_G, CST_R_F_S):
    case FINAL_PAIR(CST_R_F_N, CST_R_F_J):
    case FINAL_PAIR(CST_R_F_N, CST_R_F_H):
    case FINAL_PAIR(CST_R_F_R, CST_R_F_G):
    case FINAL_PAIR(CST_R_F_R, CST_R_F_M):
    case FINAL_PAIR(CST_R_F_R, CST_R_F_B):
    case FINAL_PAIR(CST_R_F_R, CST_R_F_S):
    case FINAL_PAIR(CST_R_F_R, CST_R_F_T):
    case FINAL_PAIR(CST_R_F_R, CST_R_F_P):
    case FINAL_PAIR(CST_R_F_R, CST_R_F_H):
    case FINAL_PAIR(CST_R_F_B, CST_R_F_S):
        return true;
    default:
        return false;
    }
#undef FINAL_PAIR
}

static void expire_final_roll_locked(int64_t now) {
    for (uint32_t position = 0; position < CONFIG_CORNIX_STENO_MAX_POSITIONS; position++) {
        const uint64_t bit = CST_POSITION_BIT(position);
        if ((state.final_roll_pending_mask & bit) &&
            now - state.final_roll_released_at[position] > CONFIG_CORNIX_STENO_FINAL_ROLL_MS) {
            state.final_roll_pending_mask &= ~bit;
        }
    }
}

static void restore_final_roll_pair_locked(enum cornix_steno_role role, int64_t now) {
    if (!bit_can_roll_final(role)) return;
    expire_final_roll_locked(now);
    for (uint32_t position = 0; position < CONFIG_CORNIX_STENO_MAX_POSITIONS; position++) {
        const uint64_t bit = CST_POSITION_BIT(position);
        if (!(state.final_roll_pending_mask & bit)) continue;
        const enum cornix_steno_role first = state.position_role[position];
        if (!is_valid_final_roll_pair(first, role)) continue;
        state.accepted_mask |= bit;
        state.release_candidate_mask &= ~bit;
        state.final_roll_pending_mask &= ~bit;
        return;
    }
}

static void schedule_next_correction_locked(int64_t now) {
    int64_t next_delay = INT64_MAX;
    for (uint32_t position = 0; position < CONFIG_CORNIX_STENO_MAX_POSITIONS; position++) {
        const uint64_t bit = CST_POSITION_BIT(position);
        if (!(state.release_candidate_mask & bit)) continue;
        const int64_t remaining = CONFIG_CORNIX_STENO_KEYUP_CORRECTION_MS -
                                  (now - state.released_at[position]);
        if (remaining < next_delay) next_delay = remaining;
    }
    if (next_delay != INT64_MAX) {
        k_work_reschedule(&correction_work, K_MSEC(MAX(next_delay, 1)));
    }
}

static bool dictionary_anchor_active_locked(void) {
    const uint64_t roles = roles_from_positions_locked(state.accepted_mask);
    if (!roles) return false;
    if (roles & CST_ABBR_MASK) return true;
    if ((roles & CST_VEXT_MASK) && !(roles & CST_BASE_VOWEL_MASK) &&
        (roles & (CST_INITIAL_MASK | CST_FINAL_MASK))) return true;
#if IS_ENABLED(CONFIG_CORNIX_STENO_DICTIONARY)
    return cornix_steno_dictionary_has_prefix(roles);
#else
    return false;
#endif
}

static void correction_work_handler(struct k_work *work) {
    ARG_UNUSED(work);
    const int64_t now = k_uptime_get();
    enum cornix_steno_abbreviation_category category = CST_ABBR_CATEGORY_NONE;
    bool publish_category = false;

    k_mutex_lock(&steno_mutex, K_FOREVER);
    if (!state.active || state.physical_nav_mode || state.stream_mode != CST_STREAM_NONE ||
        state.down_mask == 0) {
        state.release_candidate_mask = 0;
        k_mutex_unlock(&steno_mutex);
        return;
    }
    if (dictionary_anchor_active_locked()) {
        state.release_candidate_mask = 0;
        k_mutex_unlock(&steno_mutex);
        return;
    }

    for (uint32_t position = 0; position < CONFIG_CORNIX_STENO_MAX_POSITIONS; position++) {
        const uint64_t bit = CST_POSITION_BIT(position);
        if (!(state.release_candidate_mask & bit)) continue;
        if (state.direct_hold_latched && (state.direct_hold_position_mask & bit)) {
            state.release_candidate_mask &= ~bit;
            continue;
        }
        if (state.down_mask & bit) {
            state.release_candidate_mask &= ~bit;
            continue;
        }
        if (now - state.released_at[position] >= CONFIG_CORNIX_STENO_KEYUP_CORRECTION_MS) {
            state.accepted_mask &= ~bit;
            state.release_candidate_mask &= ~bit;
            LOG_DBG("Removed early-released STENO position %u", position);
        }
    }
    expire_final_roll_locked(now);
    category = category_from_state_locked();
    publish_category = true;
    schedule_next_correction_locked(now);
    k_mutex_unlock(&steno_mutex);

    if (publish_category) cornix_steno_led_set_category(category);
}

static int engine_init(void) {
    k_work_init_delayable(&correction_work, correction_work_handler);
    reset_stream_locked();
    return 0;
}
SYS_INIT(engine_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

void cornix_steno_engine_set_active(bool active) {
    uint32_t release_keys[4] = {0};
    size_t release_count = 0;

    k_work_cancel_delayable(&correction_work);
    k_mutex_lock(&steno_mutex, K_FOREVER);
    release_count = collect_physical_nav_release_keys_locked(release_keys);
    state.active = active;
    reset_normal_stroke_locked();
    reset_stream_locked();
    k_mutex_unlock(&steno_mutex);

    for (size_t i = 0; i < release_count; i++) {
        raise_zmk_keycode_state_changed_from_encoded(release_keys[i], false, k_uptime_get());
    }
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
    const bool has_down = state.down_mask != 0 || state.physical_nav_mode ||
                          state.stream_mode != CST_STREAM_NONE;
    k_mutex_unlock(&steno_mutex);
    return has_down;
}

void cornix_steno_engine_cancel_pending(void) {
    uint32_t release_keys[4] = {0};
    size_t release_count = 0;

    k_work_cancel_delayable(&correction_work);
    k_mutex_lock(&steno_mutex, K_FOREVER);
    release_count = collect_physical_nav_release_keys_locked(release_keys);
    reset_normal_stroke_locked();
    reset_stream_locked();
    k_mutex_unlock(&steno_mutex);

    for (size_t i = 0; i < release_count; i++) {
        raise_zmk_keycode_state_changed_from_encoded(release_keys[i], false, k_uptime_get());
    }
    cornix_steno_led_reset_keys();
    cornix_steno_led_set_category(CST_ABBR_CATEGORY_NONE);
}

static bool normal_stroke_is_anchor_double_locked(enum cornix_steno_role anchor,
                                                   uint32_t anchor_position,
                                                   enum cornix_steno_role dbl,
                                                   uint32_t double_position) {
    const uint64_t mask = CST_POSITION_BIT(anchor_position) | CST_POSITION_BIT(double_position);
    return state.accepted_mask == mask &&
           state.down_mask == CST_POSITION_BIT(anchor_position) &&
           state.position_role[anchor_position] == anchor &&
           state.position_role[double_position] == dbl;
}

int cornix_steno_engine_role_pressed(enum cornix_steno_role role, uint32_t position,
                                     int64_t timestamp) {
    if (role <= CST_R_NONE || role >= CST_R_COUNT ||
        position >= CONFIG_CORNIX_STENO_MAX_POSITIONS) return -EINVAL;

    const uint64_t bit = CST_POSITION_BIT(position);
    uint32_t send_key = 0;
    enum cornix_steno_abbreviation_category category = CST_ABBR_CATEGORY_NONE;
    bool update_category = false;
    bool consumed = false;

    k_mutex_lock(&steno_mutex, K_FOREVER);
    if (!state.active) {
        k_mutex_unlock(&steno_mutex);
        return CST_ENGINE_EVENT_NORMAL;
    }

    /* Physical ㅣ+JLKI navigation has first priority. */
    if (state.physical_nav_mode) {
        send_key = physical_nav_key_for_position(position);
        if (send_key && !(state.physical_nav_down_mask & bit)) {
            state.physical_nav_down_mask |= bit;
        } else {
            send_key = 0;
        }
        k_mutex_unlock(&steno_mutex);
        if (send_key) raise_zmk_keycode_state_changed_from_encoded(send_key, true, timestamp);
        return CST_ENGINE_EVENT_CONSUMED;
    }

    if (state.stream_mode != CST_STREAM_NONE) {
        if (position == state.stream_anchor_position && role == state.stream_anchor_role) {
            state.stream_anchor_down = true;
            k_mutex_unlock(&steno_mutex);
            return CST_ENGINE_EVENT_CONSUMED;
        }
        if (state.stream_mode == CST_STREAM_NUMBER) {
            if (number_key_for_top_position(position) && !(state.stream_unit_down_mask & bit)) {
                state.stream_unit_down_mask |= bit;
                state.stream_unit_seen_mask |= bit;
            } else {
                state.stream_unit_down_mask |= bit;
                state.stream_unit_seen_mask |= bit;
            }
        } else if (state.stream_mode == CST_STREAM_CORRECTION) {
            if (!(state.stream_unit_down_mask & bit)) {
                state.stream_unit_down_mask |= bit;
                state.stream_unit_seen_mask |= bit;
                state.stream_unit_role_mask |= CST_ROLE_BIT(role);
            }
        } else if (state.stream_mode == CST_STREAM_UNDO || state.stream_mode == CST_STREAM_REDO) {
            if (role == edit_stream_counterpart(state.stream_mode, state.stream_anchor_role) &&
                !(state.stream_unit_down_mask & bit)) {
                state.stream_unit_down_mask |= bit;
                state.stream_unit_seen_mask |= bit;
            } else {
                state.stream_unit_down_mask |= bit;
                state.stream_unit_seen_mask |= bit;
            }
        } else if (role_is_dedicated_double_for_anchor(state.stream_anchor_role, role) &&
                   !(state.stream_unit_down_mask & bit)) {
            state.stream_unit_down_mask |= bit;
            state.stream_unit_seen_mask |= bit;
        } else {
            /* Swallow unrelated keys until the held anchor is released. */
            state.stream_unit_down_mask |= bit;
            state.stream_unit_seen_mask |= bit;
        }
        k_mutex_unlock(&steno_mutex);
        return CST_ENGINE_EVENT_CONSUMED;
    }

    if (state.down_mask & bit) {
        k_mutex_unlock(&steno_mutex);
        return CST_ENGINE_EVENT_NORMAL;
    }

    /* A first ABBR+dedicated-double tap is kept as a possible canonical
     * abbreviation prefix.  If the same dedicated double is tapped again
     * while only the ABBR anchor remains down, convert to repeat navigation:
     * emit the deferred first tap now and the current tap on its key-up. */
    if (state.down_mask != 0 && (state.down_mask & (state.down_mask - 1u)) == 0 &&
        role == state.position_role[position]) {
        uint32_t anchor_position = UINT32_MAX;
        enum cornix_steno_role anchor_role = CST_R_NONE;
        for (uint32_t p = 0; p < CONFIG_CORNIX_STENO_MAX_POSITIONS; p++) {
            if (state.down_mask & CST_POSITION_BIT(p)) {
                anchor_position = p;
                anchor_role = state.position_role[p];
                break;
            }
        }
        const uint64_t accepted_roles = roles_from_positions_locked(state.accepted_mask);
        if (is_abbr_role(anchor_role) &&
            role_is_dedicated_double_for_anchor(anchor_role, role) &&
            deferred_abbr_navigation_key(accepted_roles) != 0 &&
            cornix_steno_dictionary_has_prefix(accepted_roles)) {
            state.stream_mode = stream_mode_for_anchor(anchor_role);
            state.stream_anchor_role = anchor_role;
            state.stream_anchor_position = anchor_position;
            state.stream_anchor_down = true;
            state.stream_unit_down_mask = bit;
            state.stream_unit_seen_mask = bit;
            send_key = deferred_abbr_navigation_key(accepted_roles);
            reset_normal_stroke_locked();
            k_mutex_unlock(&steno_mutex);
            k_work_cancel_delayable(&correction_work);
            cornix_steno_led_set_category(CST_ABBR_CATEGORY_NONE);
            if (send_key) emit_key_tap(send_key, timestamp);
            return CST_ENGINE_EVENT_CONSUMED;
        }
    }

    /* Any new role after a direct-hold chord was latched changes the intended
     * stroke, so cancel the direct output candidate before accepting it. */
    if (state.direct_hold_latched) {
        state.direct_hold_latched = false;
        state.direct_hold_position_mask = 0;
        state.direct_hold_role_mask = 0;
    }

    /* Enter physical ㅣ+JLKI when ㅣ is the only normal role already down. */
    const uint32_t nav_key = physical_nav_key_for_position(position);
    if (nav_key && state.down_mask == CST_POSITION_BIT(CST_NAV_VOWEL_POSITION) &&
        state.accepted_mask == state.down_mask &&
        state.position_role[CST_NAV_VOWEL_POSITION] == CST_R_V_I) {
        state.physical_nav_mode = true;
        state.physical_nav_vowel_down = true;
        state.physical_nav_down_mask = bit;
        reset_normal_stroke_locked();
        send_key = nav_key;
        k_mutex_unlock(&steno_mutex);
        k_work_cancel_delayable(&correction_work);
        cornix_steno_led_set_category(CST_ABBR_CATEGORY_NONE);
        raise_zmk_keycode_state_changed_from_encoded(send_key, true, timestamp);
        return CST_ENGINE_EVENT_CONSUMED;
    }

    /* Immediate/repeatable thumb editing: hold either member of the pair and
     * tap the other. The command is emitted on the tapped key's key-up; the
     * anchor may remain held for another tap. Order is symmetric. */
    if (state.down_mask != 0 && (state.down_mask & (state.down_mask - 1u)) == 0) {
        uint32_t anchor_position = UINT32_MAX;
        enum cornix_steno_role anchor_role = CST_R_NONE;
        for (uint32_t p = 0; p < CONFIG_CORNIX_STENO_MAX_POSITIONS; p++) {
            if (state.down_mask & CST_POSITION_BIT(p)) {
                anchor_position = p;
                anchor_role = state.position_role[p];
                break;
            }
        }
        const enum anchored_stream_mode edit_mode = edit_stream_mode_for_pair(anchor_role, role);
        if (edit_mode != CST_STREAM_NONE) {
            state.stream_mode = edit_mode;
            state.stream_anchor_role = anchor_role;
            state.stream_anchor_position = anchor_position;
            state.stream_anchor_down = true;
            state.stream_unit_down_mask = bit;
            state.stream_unit_seen_mask = bit;
            reset_normal_stroke_locked();
            k_mutex_unlock(&steno_mutex);
            k_work_cancel_delayable(&correction_work);
            cornix_steno_led_set_category(CST_ABBR_CATEGORY_NONE);
            return CST_ENGINE_EVENT_CONSUMED;
        }
    }

    /* Repeating number stream: hold either SYMBOL key and tap physical Q..P.
     * Each top-row key emits 1..0 immediately on its own key-up; the SYMBOL
     * anchor may stay held for the next digit, just like repeat navigation. */
    if (number_key_for_top_position(position) && state.down_mask != 0 &&
        (state.down_mask & (state.down_mask - 1u)) == 0) {
        uint32_t anchor_position = UINT32_MAX;
        enum cornix_steno_role anchor_role = CST_R_NONE;
        for (uint32_t p = 0; p < CONFIG_CORNIX_STENO_MAX_POSITIONS; p++) {
            if (!(state.down_mask & CST_POSITION_BIT(p))) continue;
            if (is_symbol_role(state.position_role[p])) {
                anchor_position = p;
                anchor_role = state.position_role[p];
            }
            break;
        }
        if (anchor_position != UINT32_MAX) {
            state.stream_mode = CST_STREAM_NUMBER;
            state.stream_anchor_role = anchor_role;
            state.stream_anchor_position = anchor_position;
            state.stream_anchor_down = true;
            state.stream_unit_down_mask = bit;
            state.stream_unit_seen_mask = bit;
            reset_normal_stroke_locked();
            k_mutex_unlock(&steno_mutex);
            k_work_cancel_delayable(&correction_work);
            cornix_steno_led_set_category(CST_ABBR_CATEGORY_NONE);
            return CST_ENGINE_EVENT_CONSUMED;
        }
    }

    /* SYMBOL is no longer a correction modifier. Direct jamo correction is
     * produced by holding each jamo role itself, then releasing it. */

    if (state.down_mask == 0) reset_normal_stroke_locked();
    else state.stroke_had_multiple = true;
    restore_final_roll_pair_locked(role, timestamp);
    state.release_candidate_mask &= ~bit;
    state.down_mask |= bit;
    state.accepted_mask |= bit;
    state.position_role[position] = role;
    state.pressed_at[position] = timestamp;
    category = category_from_state_locked();
    update_category = true;
    k_mutex_unlock(&steno_mutex);

    if (update_category) cornix_steno_led_set_category(category);
    return consumed ? CST_ENGINE_EVENT_CONSUMED : CST_ENGINE_EVENT_NORMAL;
}

int cornix_steno_engine_role_released(enum cornix_steno_role role, uint32_t position,
                                      int64_t timestamp) {
    if (position >= CONFIG_CORNIX_STENO_MAX_POSITIONS) return -EINVAL;

    const uint64_t bit = CST_POSITION_BIT(position);
    uint64_t final_positions = 0;
    uint64_t final_roles = 0;
    uint64_t correction_roles = 0;
    uint32_t send_key = 0;
    uint32_t release_keys[4] = {0};
    size_t release_count = 0;
    bool finalize = false;
    bool process_correction = false;
    bool direct_solo_hold = false;
    bool direct_composite_hold = false;
    enum cornix_steno_role direct_role = CST_R_NONE;
    uint64_t direct_composite_roles = 0;
    bool consumed = false;
    enum cornix_steno_abbreviation_category category = CST_ABBR_CATEGORY_NONE;
    bool update_category = false;

    k_mutex_lock(&steno_mutex, K_FOREVER);
    if (!state.active) {
        k_mutex_unlock(&steno_mutex);
        return CST_ENGINE_EVENT_NORMAL;
    }

    if (state.physical_nav_mode) {
        if (position == CST_NAV_VOWEL_POSITION) {
            state.physical_nav_vowel_down = false;
            release_count = collect_physical_nav_release_keys_locked(release_keys);
        } else if (state.physical_nav_down_mask & bit) {
            state.physical_nav_down_mask &= ~bit;
            send_key = physical_nav_key_for_position(position);
        }
        k_mutex_unlock(&steno_mutex);
        if (send_key) raise_zmk_keycode_state_changed_from_encoded(send_key, false, timestamp);
        for (size_t i = 0; i < release_count; i++)
            raise_zmk_keycode_state_changed_from_encoded(release_keys[i], false, timestamp);
        return CST_ENGINE_EVENT_CONSUMED;
    }

    if (state.stream_mode != CST_STREAM_NONE) {
        consumed = true;
        if (position == state.stream_anchor_position && role == state.stream_anchor_role) {
            state.stream_anchor_down = false;
            if (state.stream_unit_down_mask == 0) reset_stream_locked();
        } else if (state.stream_unit_down_mask & bit) {
            state.stream_unit_down_mask &= ~bit;
            if (state.stream_mode == CST_STREAM_NUMBER) {
                send_key = number_key_for_top_position(position);
                state.stream_unit_seen_mask &= ~bit;
                if (!state.stream_anchor_down && state.stream_unit_down_mask == 0)
                    reset_stream_locked();
            } else if (state.stream_mode == CST_STREAM_CORRECTION) {
                if (state.stream_unit_down_mask == 0) {
                    correction_roles = state.stream_unit_role_mask;
                    state.stream_unit_role_mask = 0;
                    state.stream_unit_seen_mask = 0;
                    process_correction = correction_roles != 0;
                    if (!state.stream_anchor_down) reset_stream_locked();
                }
            } else if ((state.stream_mode == CST_STREAM_UNDO || state.stream_mode == CST_STREAM_REDO) &&
                       role == edit_stream_counterpart(state.stream_mode, state.stream_anchor_role)) {
                send_key = stream_key_for_mode(state.stream_mode);
                state.stream_unit_seen_mask &= ~bit;
                if (!state.stream_anchor_down && state.stream_unit_down_mask == 0)
                    reset_stream_locked();
            } else if (role_is_dedicated_double_for_anchor(state.stream_anchor_role, role)) {
                send_key = stream_key_for_mode(state.stream_mode);
                if (!state.stream_anchor_down && state.stream_unit_down_mask == 0)
                    reset_stream_locked();
            } else if (!state.stream_anchor_down && state.stream_unit_down_mask == 0) {
                reset_stream_locked();
            }
        }
        k_mutex_unlock(&steno_mutex);
        if (send_key) emit_key_tap(send_key, timestamp);
        if (process_correction) process_correction_unit(correction_roles);
        return CST_ENGINE_EVENT_CONSUMED;
    }

    if (!(state.down_mask & bit)) {
        k_mutex_unlock(&steno_mutex);
        return CST_ENGINE_EVENT_NORMAL;
    }

    /* Latch a complete valid direct-jamo chord at its first key-up, before the
     * 50 ms typo correction can discard an early-released member. */
    if (!state.direct_hold_latched && state.down_mask == state.accepted_mask) {
        const uint64_t roles = roles_from_positions_locked(state.accepted_mask);
        int64_t latest_press = 0;
        for (uint32_t p = 0; p < CONFIG_CORNIX_STENO_MAX_POSITIONS; p++) {
            if (!(state.accepted_mask & CST_POSITION_BIT(p))) continue;
            if (state.pressed_at[p] > latest_press) latest_press = state.pressed_at[p];
        }
        if (latest_press > 0 &&
            timestamp - latest_press >= CONFIG_CORNIX_STENO_DUAL_TAPPING_TERM_MS &&
            (role_mask_is_direct_vowel_hold(roles) || role_mask_is_direct_double_hold(roles))) {
            state.direct_hold_latched = true;
            state.direct_hold_position_mask = state.accepted_mask;
            state.direct_hold_role_mask = roles;
        }
    }

    state.down_mask &= ~bit;

    /* Dedicated double only: aliases ㅋ/ㅇ and 종ㅋ/종ㅇ never start movement.
     * If ABBR+double is a dictionary prefix, preserve the first tap for a
     * possible selector-first abbreviation and resolve it on later input or
     * ABBR release. */
    enum cornix_steno_role anchor_role = CST_R_NONE;
    uint32_t anchor_position = UINT32_MAX;
    for (uint32_t p = 0; p < CONFIG_CORNIX_STENO_MAX_POSITIONS; p++) {
        if (!(state.down_mask & CST_POSITION_BIT(p))) continue;
        if (is_abbr_role(state.position_role[p]) || is_symbol_role(state.position_role[p])) {
            anchor_role = state.position_role[p];
            anchor_position = p;
            break;
        }
    }
    if (anchor_role != CST_R_NONE &&
        role_is_dedicated_double_for_anchor(anchor_role, role) &&
        normal_stroke_is_anchor_double_locked(anchor_role, anchor_position, role, position)) {
        const uint64_t pair_roles = roles_from_positions_locked(state.accepted_mask);
        const bool defer_for_dictionary = is_abbr_role(anchor_role) &&
                                          cornix_steno_dictionary_has_prefix(pair_roles);
        if (!defer_for_dictionary) {
            state.stream_mode = stream_mode_for_anchor(anchor_role);
            state.stream_anchor_role = anchor_role;
            state.stream_anchor_position = anchor_position;
            state.stream_anchor_down = true;
            send_key = stream_key_for_mode(state.stream_mode);
            reset_normal_stroke_locked();
            consumed = true;
            k_mutex_unlock(&steno_mutex);
            k_work_cancel_delayable(&correction_work);
            cornix_steno_led_set_category(CST_ABBR_CATEGORY_NONE);
            if (send_key) emit_key_tap(send_key, timestamp);
            return CST_ENGINE_EVENT_CONSUMED;
        }
    }

    if (state.down_mask != 0) {
        state.release_candidate_mask |= bit;
        state.released_at[position] = timestamp;
        if (bit_can_roll_final(role)) {
            state.final_roll_pending_mask |= bit;
            state.final_roll_released_at[position] = timestamp;
        }
        schedule_next_correction_locked(timestamp);
        category = category_from_state_locked();
        update_category = true;
    } else {
        final_positions = state.accepted_mask;
        final_roles = roles_from_positions_locked(final_positions);
        const uint32_t deferred_nav = deferred_abbr_navigation_key(final_roles);

        if (!deferred_nav && !state.stroke_had_multiple && final_positions == bit &&
            timestamp - state.pressed_at[position] >= CONFIG_CORNIX_STENO_DUAL_TAPPING_TERM_MS) {
            struct cornix_steno_decoded direct_decoded;
            if (cornix_steno_decode_correction_unit(CST_ROLE_BIT(role), &direct_decoded) == 0 &&
                direct_decoded.kind == CST_DECODE_KEYS && direct_decoded.key_count) {
                direct_solo_hold = true;
                direct_role = role;
            }
        }
        if (!deferred_nav && state.direct_hold_latched) {
            direct_composite_hold = true;
            direct_composite_roles = state.direct_hold_role_mask;
        }
        send_key = deferred_nav;
        reset_normal_stroke_locked();
        finalize = true;
    }
    k_mutex_unlock(&steno_mutex);

    if (update_category) cornix_steno_led_set_category(category);
    if (finalize) {
        k_work_cancel_delayable(&correction_work);
        if (send_key) emit_key_tap(send_key, timestamp);
        else if (direct_solo_hold) cornix_steno_engine_emit_direct_role(direct_role);
        else if (direct_composite_hold) process_correction_unit(direct_composite_roles);
        else if (final_positions && final_roles) process_stroke(final_positions, final_roles, timestamp);
        else cornix_steno_led_set_category(CST_ABBR_CATEGORY_NONE);
    }
    return consumed ? CST_ENGINE_EVENT_CONSUMED : CST_ENGINE_EVENT_NORMAL;
}

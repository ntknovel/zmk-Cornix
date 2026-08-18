/* SPDX-License-Identifier: MIT */

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <dt-bindings/zmk/keys.h>
#include <dt-bindings/zmk/modifiers.h>
#include <zmk/events/keycode_state_changed.h>

#include <cornix_steno/dual.h>
#include <cornix_steno/engine.h>
#include <cornix_steno/output.h>
#include <cornix_steno/tab.h>

struct dual_state {
    bool work_initialized;
    bool active;
    bool committed_role;
    bool hold_down;
    bool consumed;
    enum cornix_steno_dual_policy policy;
    enum cornix_steno_role role;
    uint32_t tap_key;
    uint32_t hold_key;
    int64_t pressed_at;
    struct k_work_delayable timeout_work;
};

static struct dual_state states[CONFIG_CORNIX_STENO_MAX_POSITIONS];
K_MUTEX_DEFINE(dual_mutex);

struct paste_state {
    bool active;
    uint32_t first;
    uint32_t second;
};
static struct paste_state paste;

static void send_key_state(uint32_t key, bool pressed, int64_t timestamp) {
    if (key) raise_zmk_keycode_state_changed_from_encoded(key, pressed, timestamp);
}

static uint8_t modifier_flag_for_key(uint32_t key) {
    if (STRIP_MODS(key) == STRIP_MODS(LCTRL)) return MOD_LCTL;
    if (STRIP_MODS(key) == STRIP_MODS(LALT)) return MOD_LALT;
    return 0;
}

static bool paste_involves(uint32_t position) {
    return paste.active && (paste.first == position || paste.second == position);
}

static void timeout_handler(struct k_work *work) {
    struct k_work_delayable *delayable = k_work_delayable_from_work(work);
    struct dual_state *self = CONTAINER_OF(delayable, struct dual_state, timeout_work);
    const uint32_t position = (uint32_t)(self - states);
    uint32_t press_keys[2] = {0};
    size_t press_count = 0;
    const int64_t now = k_uptime_get();

    k_mutex_lock(&dual_mutex, K_FOREVER);
    if (!self->active || self->policy != CST_DUAL_MOD_TAP || self->consumed ||
        self->hold_down) {
        k_mutex_unlock(&dual_mutex);
        return;
    }

    if (paste_involves(position)) {
        const uint32_t pair[2] = {paste.first, paste.second};
        for (size_t i = 0; i < ARRAY_SIZE(pair); i++) {
            if (pair[i] >= ARRAY_SIZE(states)) continue;
            struct dual_state *state = &states[pair[i]];
            if (state->active && state->policy == CST_DUAL_MOD_TAP &&
                !state->consumed && !state->hold_down) {
                state->hold_down = true;
                press_keys[press_count++] = state->hold_key;
            }
        }
        paste.active = false;
    } else {
        self->hold_down = true;
        press_keys[press_count++] = self->hold_key;
    }
    k_mutex_unlock(&dual_mutex);

    for (size_t i = 0; i < press_count; i++) send_key_state(press_keys[i], true, now);
}

static void ensure_work_initialized(struct dual_state *state) {
    if (!state->work_initialized) {
        k_work_init_delayable(&state->timeout_work, timeout_handler);
        state->work_initialized = true;
    }
}

static void commit_pending_context_roles(uint32_t exclude_position, int64_t timestamp,
                                         bool consume_modifiers) {
    struct pending_commit {
        uint32_t position;
        enum cornix_steno_role role;
        int64_t pressed_at;
    } commits[CONFIG_CORNIX_STENO_MAX_POSITIONS];
    uint32_t release_keys[CONFIG_CORNIX_STENO_MAX_POSITIONS];
    size_t commit_count = 0;
    size_t release_count = 0;

    k_mutex_lock(&dual_mutex, K_FOREVER);
    for (uint32_t position = 0; position < ARRAY_SIZE(states); position++) {
        if (position == exclude_position) continue;
        struct dual_state *state = &states[position];
        if (!state->active || state->consumed) continue;

        if (!state->committed_role &&
            (state->policy == CST_DUAL_VOWEL || state->policy == CST_DUAL_EDIT)) {
            state->committed_role = true;
            commits[commit_count++] = (struct pending_commit){
                .position = position, .role = state->role, .pressed_at = state->pressed_at};
        } else if (consume_modifiers && state->policy == CST_DUAL_MOD_TAP) {
            state->consumed = true;
            k_work_cancel_delayable(&state->timeout_work);
            if (state->hold_down) {
                state->hold_down = false;
                release_keys[release_count++] = state->hold_key;
            }
            paste.active = false;
        }
    }
    k_mutex_unlock(&dual_mutex);

    for (size_t i = 0; i < release_count; i++) {
        send_key_state(release_keys[i], false, timestamp);
    }
    for (size_t i = 0; i < commit_count; i++) {
        cornix_steno_engine_role_pressed(commits[i].role, commits[i].position,
                                         commits[i].pressed_at ? commits[i].pressed_at : timestamp);
    }
}

void cornix_steno_dual_notify_other_press(uint32_t position, int64_t timestamp) {
    commit_pending_context_roles(position, timestamp, true);
}

int cornix_steno_dual_pressed(uint32_t position, enum cornix_steno_dual_policy policy,
                              enum cornix_steno_role role, uint32_t tap_key,
                              uint32_t hold_key, int64_t timestamp) {
    if (position >= ARRAY_SIZE(states)) return -EINVAL;

    if (policy != CST_DUAL_MOD_TAP) {
        cornix_steno_tab_notify_other_press(position, timestamp);
    }
    commit_pending_context_roles(position, timestamp, policy != CST_DUAL_MOD_TAP);

    bool commit_current = false;
    k_mutex_lock(&dual_mutex, K_FOREVER);
    struct dual_state *state = &states[position];
    ensure_work_initialized(state);
    if (state->active) {
        k_mutex_unlock(&dual_mutex);
        return 0;
    }

    state->active = true;
    state->committed_role = false;
    state->hold_down = false;
    state->consumed = false;
    state->policy = policy;
    state->role = role;
    state->tap_key = tap_key;
    state->hold_key = hold_key;
    state->pressed_at = timestamp;

    if (policy == CST_DUAL_MOD_TAP) {
        const uint8_t this_mod = modifier_flag_for_key(hold_key);
        for (uint32_t other = 0; other < ARRAY_SIZE(states); other++) {
            if (other == position) continue;
            struct dual_state *o = &states[other];
            if (o->active && o->policy == CST_DUAL_MOD_TAP && !o->consumed &&
                modifier_flag_for_key(o->hold_key) != this_mod) {
                paste.active = true;
                paste.first = other;
                paste.second = position;
                break;
            }
        }
        k_work_reschedule(&state->timeout_work,
                          K_MSEC(CONFIG_CORNIX_STENO_DUAL_TAPPING_TERM_MS));
        if (cornix_steno_engine_has_down_keys()) {
            state->consumed = true;
            paste.active = false;
        }
    } else if (cornix_steno_engine_has_down_keys()) {
        state->committed_role = true;
        commit_current = true;
    }
    k_mutex_unlock(&dual_mutex);

    if (policy == CST_DUAL_MOD_TAP &&
        cornix_steno_tab_modifier_pressed(position, timestamp)) {
        return 0;
    }
    if (commit_current) {
        return cornix_steno_engine_role_pressed(role, position, timestamp);
    }
    return 0;
}

int cornix_steno_dual_released(uint32_t position, int64_t timestamp) {
    if (position >= ARRAY_SIZE(states)) return -EINVAL;

    enum cornix_steno_dual_policy policy;
    enum cornix_steno_role role;
    uint32_t tap_key;
    uint32_t hold_key;
    bool committed_role;
    bool hold_down;
    bool consumed;
    int64_t pressed_at;
    bool output_paste = false;

    k_mutex_lock(&dual_mutex, K_FOREVER);
    struct dual_state *state = &states[position];
    if (!state->active) {
        k_mutex_unlock(&dual_mutex);
        return 0;
    }
    k_work_cancel_delayable(&state->timeout_work);

    policy = state->policy;
    role = state->role;
    tap_key = state->tap_key;
    hold_key = state->hold_key;
    committed_role = state->committed_role;
    hold_down = state->hold_down;
    consumed = state->consumed;
    pressed_at = state->pressed_at;

    state->active = false;
    state->committed_role = false;
    state->hold_down = false;
    state->consumed = false;

    if (paste_involves(position)) {
        const uint32_t other = paste.first == position ? paste.second : paste.first;
        if (other < ARRAY_SIZE(states) && !states[other].active) {
            output_paste = true;
            paste.active = false;
        }
        consumed = true;
    }
    k_mutex_unlock(&dual_mutex);

    if (committed_role) return cornix_steno_engine_role_released(role, position, timestamp);
    if (hold_down) {
        send_key_state(hold_key, false, timestamp);
        return 0;
    }
    if (output_paste) return cornix_steno_output_enqueue(LC(V));
    if (!consumed) {
        /* Vowel thumb tri-state: a short solo tap is Enter/Space; a solo
         * hold released after the tapping term directly emits that vowel jamo;
         * any multi-key STENO participation commits the vowel role instead.
         * Nothing fires while the key is physically held. */
        if (policy == CST_DUAL_VOWEL &&
            timestamp - pressed_at >= CONFIG_CORNIX_STENO_DUAL_TAPPING_TERM_MS) {
            return cornix_steno_engine_emit_direct_role(role);
        }
        return cornix_steno_output_enqueue(tap_key);
    }
    ARG_UNUSED(policy);
    return 0;
}

uint8_t cornix_steno_dual_consume_modifiers_for_tab(int64_t timestamp) {
    uint8_t modifiers = 0;
    uint32_t release_keys[2] = {0};
    size_t release_count = 0;

    k_mutex_lock(&dual_mutex, K_FOREVER);
    paste.active = false;
    for (uint32_t position = 0; position < ARRAY_SIZE(states); position++) {
        struct dual_state *state = &states[position];
        if (!state->active || state->policy != CST_DUAL_MOD_TAP) continue;
        modifiers |= modifier_flag_for_key(state->hold_key);
        state->consumed = true;
        k_work_cancel_delayable(&state->timeout_work);
        if (state->hold_down && release_count < ARRAY_SIZE(release_keys)) {
            release_keys[release_count++] = state->hold_key;
            state->hold_down = false;
        }
    }
    k_mutex_unlock(&dual_mutex);

    for (size_t i = 0; i < release_count; i++) {
        send_key_state(release_keys[i], false, timestamp);
    }
    return modifiers;
}

bool cornix_steno_dual_take_edit_for_encoder(void) {
    bool active = false;
    bool cancel_stroke = false;

    k_mutex_lock(&dual_mutex, K_FOREVER);
    for (uint32_t position = 0; position < ARRAY_SIZE(states); position++) {
        struct dual_state *state = &states[position];
        if (!state->active || state->policy != CST_DUAL_EDIT) continue;
        active = true;
        cancel_stroke = cancel_stroke || state->committed_role;
        state->committed_role = false;
        state->consumed = true;
    }
    k_mutex_unlock(&dual_mutex);

    if (cancel_stroke) cornix_steno_engine_cancel_pending();
    return active;
}

void cornix_steno_dual_reset(void) {
    uint32_t release_keys[CONFIG_CORNIX_STENO_MAX_POSITIONS];
    size_t release_count = 0;
    const int64_t now = k_uptime_get();

    k_mutex_lock(&dual_mutex, K_FOREVER);
    paste.active = false;
    for (uint32_t position = 0; position < ARRAY_SIZE(states); position++) {
        struct dual_state *state = &states[position];
        if (state->work_initialized) k_work_cancel_delayable(&state->timeout_work);
        if (state->hold_down) release_keys[release_count++] = state->hold_key;
        state->active = false;
        state->committed_role = false;
        state->hold_down = false;
        state->consumed = false;
        state->policy = CST_DUAL_VOWEL;
        state->role = CST_R_NONE;
        state->tap_key = 0;
        state->hold_key = 0;
        state->pressed_at = 0;
    }
    k_mutex_unlock(&dual_mutex);

    for (size_t i = 0; i < release_count; i++) {
        send_key_state(release_keys[i], false, now);
    }
}

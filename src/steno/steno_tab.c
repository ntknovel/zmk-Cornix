/* SPDX-License-Identifier: MIT */

#include <stdbool.h>
#include <stdint.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <dt-bindings/zmk/keys.h>

#include <cornix_steno/output.h>
#include <cornix_steno/tab.h>

struct tab_state {
    bool active;
    uint32_t position;
};
static struct tab_state state;
K_MUTEX_DEFINE(tab_mutex);

int cornix_steno_tab_pressed(uint32_t position, int64_t timestamp) {
    ARG_UNUSED(timestamp);
    k_mutex_lock(&tab_mutex, K_FOREVER);
    if (!state.active) {
        state.active = true;
        state.position = position;
    }
    k_mutex_unlock(&tab_mutex);
    return 0;
}

int cornix_steno_tab_released(uint32_t position, int64_t timestamp) {
    ARG_UNUSED(timestamp);
    bool fire = false;
    k_mutex_lock(&tab_mutex, K_FOREVER);
    if (state.active && state.position == position) {
        state.active = false;
        state.position = 0;
        fire = true;
    }
    k_mutex_unlock(&tab_mutex);
    return fire ? cornix_steno_output_enqueue(TAB) : 0;
}

void cornix_steno_tab_notify_other_press(uint32_t position, int64_t timestamp) {
    ARG_UNUSED(position);
    ARG_UNUSED(timestamp);
}

bool cornix_steno_tab_modifier_pressed(uint32_t position, int64_t timestamp) {
    ARG_UNUSED(position);
    ARG_UNUSED(timestamp);
    return false;
}

void cornix_steno_tab_reset(void) {
    k_mutex_lock(&tab_mutex, K_FOREVER);
    state.active = false;
    state.position = 0;
    k_mutex_unlock(&tab_mutex);
}

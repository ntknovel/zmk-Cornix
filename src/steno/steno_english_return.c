/* SPDX-License-Identifier: MIT */
#include <stdbool.h>
#include <zephyr/kernel.h>
#include <cornix_steno/english_return.h>

K_MUTEX_DEFINE(english_return_mutex);
static bool pending;

bool cornix_steno_english_return_pending(void) {
    k_mutex_lock(&english_return_mutex, K_FOREVER);
    const bool value = pending;
    k_mutex_unlock(&english_return_mutex);
    return value;
}

void cornix_steno_english_return_set_pending(bool value) {
    k_mutex_lock(&english_return_mutex, K_FOREVER);
    pending = value;
    k_mutex_unlock(&english_return_mutex);
}

bool cornix_steno_english_return_take_pending(void) {
    k_mutex_lock(&english_return_mutex, K_FOREVER);
    const bool value = pending;
    pending = false;
    k_mutex_unlock(&english_return_mutex);
    return value;
}

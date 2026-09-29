/* SPDX-License-Identifier: MIT */

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>
#include <zmk/events/keycode_state_changed.h>

#include <cornix_steno/output.h>

LOG_MODULE_REGISTER(cornix_steno_output, CONFIG_ZMK_LOG_LEVEL);

static uint32_t queue[CONFIG_CORNIX_STENO_OUTPUT_QUEUE_SIZE];
static size_t queue_head;
static size_t queue_tail;
static size_t queue_count;
static bool active_pressed;
static uint32_t active_key;
K_MUTEX_DEFINE(output_mutex);
static struct k_work_delayable output_work;

static void output_work_handler(struct k_work *work) {
    ARG_UNUSED(work);
    uint32_t key = 0;
    bool press = false;
    bool release = false;
    bool schedule = false;
    k_timeout_t delay = K_NO_WAIT;

    k_mutex_lock(&output_mutex, K_FOREVER);
    if (active_pressed) {
        key = active_key;
        active_pressed = false;
        active_key = 0;
        release = true;
        schedule = queue_count > 0;
        delay = K_MSEC(CONFIG_CORNIX_STENO_OUTPUT_GAP_MS);
    } else if (queue_count > 0) {
        key = queue[queue_head];
        queue_head = (queue_head + 1U) % ARRAY_SIZE(queue);
        queue_count--;
        active_key = key;
        active_pressed = true;
        press = true;
        schedule = true;
        delay = K_MSEC(CONFIG_CORNIX_STENO_OUTPUT_HOLD_MS);
    }
    k_mutex_unlock(&output_mutex);

    if (press) {
        int err = raise_zmk_keycode_state_changed_from_encoded(key, true, k_uptime_get());
        if (err < 0) LOG_WRN("STENO key press event failed: %d", err);
    } else if (release) {
        int err = raise_zmk_keycode_state_changed_from_encoded(key, false, k_uptime_get());
        if (err < 0) LOG_WRN("STENO key release event failed: %d", err);
    }

    if (schedule) {
        k_work_reschedule(&output_work, delay);
    }
}

static int output_init(void) {
    k_work_init_delayable(&output_work, output_work_handler);
    return 0;
}
SYS_INIT(output_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

int cornix_steno_output_enqueue(uint32_t encoded_key) {
    return cornix_steno_output_enqueue_sequence(&encoded_key, 1);
}

int cornix_steno_output_enqueue_sequence(const uint32_t *keys, size_t count) {
    if (!keys || count == 0) return 0;

    k_mutex_lock(&output_mutex, K_FOREVER);
    if (count > ARRAY_SIZE(queue) - queue_count) {
        k_mutex_unlock(&output_mutex);
        LOG_WRN("STENO output queue full; rejected %u keys", (unsigned)count);
        return -ENOSPC;
    }
    const bool was_idle = queue_count == 0 && !active_pressed;
    for (size_t i = 0; i < count; i++) {
        queue[queue_tail] = keys[i];
        queue_tail = (queue_tail + 1U) % ARRAY_SIZE(queue);
        queue_count++;
    }
    k_mutex_unlock(&output_mutex);

    if (was_idle) k_work_reschedule(&output_work, K_NO_WAIT);
    return 0;
}

void cornix_steno_output_flush(void) {
    k_work_cancel_delayable(&output_work);
    uint32_t release_key = 0;

    k_mutex_lock(&output_mutex, K_FOREVER);
    if (active_pressed) release_key = active_key;
    queue_head = queue_tail = queue_count = 0;
    active_pressed = false;
    active_key = 0;
    k_mutex_unlock(&output_mutex);

    if (release_key) {
        raise_zmk_keycode_state_changed_from_encoded(release_key, false, k_uptime_get());
    }
}

size_t cornix_steno_output_pending(void) {
    k_mutex_lock(&output_mutex, K_FOREVER);
    const size_t pending = queue_count + (active_pressed ? 1U : 0U);
    k_mutex_unlock(&output_mutex);
    return pending;
}

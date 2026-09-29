/* SPDX-License-Identifier: MIT
 * CORNIX STENO USB Studio integration, v2.2.7.
 * No matrix, keymap, behavior timing or BLE split state is modified here.
 * Use ZMK's endpoint API to handle report release/fallback correctly.
 */
#include <stdbool.h>
#include <stddef.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/settings/settings.h>
#include <zephyr/sys/atomic.h>
#include <zmk/endpoints.h>
#include <zmk/event_manager.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/usb.h>

#ifndef CORNIX_SELECT_USB_TRANSPORT
#error "CMake must select the endpoint API from the actual ZMK headers"
#endif

#define CORNIX_USB_RETRY_LIMIT 4u
/* Only the work handler mutates the session counters. Event/settings hooks
 * request a reset atomically; they never race those counters. */
static atomic_t reset_requested = ATOMIC_INIT(1);
static bool selected_for_connection;
static unsigned int retries;
static void select_usb_work(struct k_work *work);
K_WORK_DELAYABLE_DEFINE(cornix_usb_work, select_usb_work);

static void select_usb_work(struct k_work *work) {
    (void)work;
    if (atomic_cas(&reset_requested, 1, 0)) {
        selected_for_connection = false;
        retries = 0;
    }
    if (!zmk_usb_is_hid_ready()) {
        selected_for_connection = false;
        retries = 0;
        return;
    }
    if (selected_for_connection) {
        return;
    }
    const int err = CORNIX_SELECT_USB_TRANSPORT(ZMK_TRANSPORT_USB);
    if (err == 0) {
        selected_for_connection = true;
        retries = 0;
    } else if (++retries < CORNIX_USB_RETRY_LIMIT) {
        (void)k_work_reschedule(&cornix_usb_work, K_MSEC(200));
    }
}

static int cornix_usb_listener(const zmk_event_t *eh) {
    const struct zmk_usb_conn_state_changed *ev = as_zmk_usb_conn_state_changed(eh);
    if (ev == NULL) {
        return ZMK_EV_EVENT_BUBBLE;
    }
    if (ev->conn_state != ZMK_USB_CONN_HID) {
        atomic_set(&reset_requested, 1);
        (void)k_work_cancel_delayable(&cornix_usb_work);
    } else {
        /* Deferred work keeps endpoint changes outside the USB event callback.
         * Do not wait/block the key processing thread. */
        (void)k_work_reschedule(&cornix_usb_work, K_MSEC(20));
    }
    return ZMK_EV_EVENT_BUBBLE;
}
ZMK_LISTENER(cornix_studio_usb_auto, cornix_usb_listener);
ZMK_SUBSCRIPTION(cornix_studio_usb_auto, zmk_usb_conn_state_changed);

static int cornix_studio_settings_commit(void) {
    /* Re-check after settings loading so a previously stored BLE preference
     * cannot undo automatic USB selection at startup. No private data is saved. */
    atomic_set(&reset_requested, 1);
    (void)k_work_reschedule(&cornix_usb_work, K_MSEC(20));
    return 0;
}
SETTINGS_STATIC_HANDLER_DEFINE(cornix_studio_usb, "cornix_studio_usb", NULL, NULL,
                               cornix_studio_settings_commit, NULL);

static int cornix_studio_usb_init(void) {
    /* Covers starting with a data cable already connected. USB connection
     * events also trigger the check when enumeration happens later. */
    (void)k_work_reschedule(&cornix_usb_work, K_MSEC(250));
    return 0;
}
SYS_INIT(cornix_studio_usb_init, APPLICATION, 99);

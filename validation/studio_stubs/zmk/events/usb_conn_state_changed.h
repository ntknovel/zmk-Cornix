#pragma once
#include <stddef.h>
#include <zmk/event_manager.h>
struct zmk_usb_conn_state_changed { int kind; int conn_state; };
static inline const struct zmk_usb_conn_state_changed *as_zmk_usb_conn_state_changed(const zmk_event_t *ev) {
    return ev != NULL && ev->kind == 1 ? (const void *)ev : NULL;
}

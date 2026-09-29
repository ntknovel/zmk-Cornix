#pragma once
typedef struct { int kind; int conn_state; } zmk_event_t;
#define ZMK_EV_EVENT_BUBBLE 0
#define ZMK_LISTENER(n, cb) static int (*const mock_usb_listener)(const zmk_event_t *) = cb
#define ZMK_SUBSCRIPTION(n, ev) _Static_assert(1, "subscription")

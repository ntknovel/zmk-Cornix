#pragma once
#include <stdbool.h>
enum zmk_usb_conn_state { ZMK_USB_CONN_NONE, ZMK_USB_CONN_POWERED, ZMK_USB_CONN_HID };
bool zmk_usb_is_hid_ready(void);

#include <assert.h>
#include <stdio.h>
#include "../src/studio/usb_auto_select.c"
static bool hid_ready;
static int select_calls, select_result;
static enum zmk_transport preferred = ZMK_TRANSPORT_BLE;
int k_work_reschedule(struct k_work_delayable *w, int delay) {
    w->pending = 1; w->delay = delay; return 1;
}
int k_work_cancel_delayable(struct k_work_delayable *w) { w->pending = 0; return 0; }
bool zmk_usb_is_hid_ready(void) { return hid_ready; }
int CORNIX_SELECT_USB_TRANSPORT(enum zmk_transport transport) {
    assert(transport == ZMK_TRANSPORT_USB);
    ++select_calls;
    if (select_result == 0) preferred = transport;
    return select_result;
}
static void run_work(void) {
    if (cornix_usb_work.pending) {
        cornix_usb_work.pending = 0;
        cornix_usb_work.handler(&cornix_usb_work.work);
    }
}
static void event(int state, bool ready) {
    hid_ready = ready;
    const zmk_event_t e = {1, state};
    assert(mock_usb_listener(&e) == ZMK_EV_EVENT_BUBBLE);
}
int main(void) {
    assert(mock_init_fn() == 0); run_work();
    assert(select_calls == 0);                      /* No USB at boot. */
    event(ZMK_USB_CONN_POWERED, false); run_work();
    assert(select_calls == 0);                      /* Charger is not a data host. */
    event(ZMK_USB_CONN_HID, false); run_work();
    assert(select_calls == 0);                      /* Enumeration not complete. */
    event(ZMK_USB_CONN_HID, true); run_work();
    assert(select_calls == 1 && preferred == ZMK_TRANSPORT_USB);
    for (int i = 0; i < 30; ++i) { event(ZMK_USB_CONN_HID, true); run_work(); }
    assert(select_calls == 1);                      /* No repeated writes. */
    preferred = ZMK_TRANSPORT_BLE;                  /* An explicit manual OUT_BLE remains usable. */
    event(ZMK_USB_CONN_HID, true); run_work();
    assert(select_calls == 1 && preferred == ZMK_TRANSPORT_BLE);
    event(ZMK_USB_CONN_NONE, false);
    event(ZMK_USB_CONN_HID, true); run_work();
    assert(select_calls == 2 && preferred == ZMK_TRANSPORT_USB);
    event(ZMK_USB_CONN_NONE, false);
    event(ZMK_USB_CONN_HID, true);
    event(ZMK_USB_CONN_NONE, false); run_work();
    assert(select_calls == 2);                      /* Unplug before deferred work. */
    event(ZMK_USB_CONN_HID, true); run_work();
    assert(select_calls == 3);
    preferred = ZMK_TRANSPORT_BLE;                  /* Stored BLE loaded after startup event. */
    assert(mock_settings_commit() == 0); run_work();
    assert(select_calls == 4 && preferred == ZMK_TRANSPORT_USB);
    const zmk_event_t unrelated = {2, ZMK_USB_CONN_HID};
    assert(mock_usb_listener(&unrelated) == ZMK_EV_EVENT_BUBBLE);
    assert(mock_usb_listener(NULL) == ZMK_EV_EVENT_BUBBLE); run_work();
    assert(select_calls == 4);
    event(ZMK_USB_CONN_NONE, false);
    event(ZMK_USB_CONN_HID, true); select_result = -1;
    for (int i = 0; i < 20; ++i) run_work();
    assert(select_calls == 8 && !cornix_usb_work.pending);
    event(ZMK_USB_CONN_NONE, false);
    select_result = 0; event(ZMK_USB_CONN_HID, true); run_work();
    assert(select_calls == 9 && selected_for_connection);
    event(ZMK_USB_CONN_POWERED, false);
    assert(reset_requested == 1);
    assert(mock_settings_commit() == 0); run_work();
    assert(select_calls == 9);
    puts("PASS: data-USB selection, stored BLE, reconnect, charge-only, no repeat, bounded retry");
    return 0;
}

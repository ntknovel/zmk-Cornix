#pragma once
#include <stdint.h>
struct bt_conn { int dummy; };
struct bt_conn_cb { void (*connected)(struct bt_conn *, uint8_t); };
#define BT_CONN_TYPE_LE 0
#define BT_CONN_CB_DEFINE(name) struct bt_conn_cb name
static inline int bt_hci_get_conn_handle(struct bt_conn *c, uint16_t *h) {(void)c;*h=0;return 0;}
static inline void bt_conn_foreach(int type, void (*cb)(struct bt_conn *, void *), void *u) {(void)type;(void)cb;(void)u;}

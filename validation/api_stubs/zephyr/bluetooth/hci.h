#pragma once
#include <stddef.h>
#include <zephyr/net_buf.h>
static inline struct net_buf *bt_hci_cmd_create(int op, size_t n) {(void)op;(void)n; static struct net_buf b; b.len=0; return &b;}
static inline int bt_hci_cmd_send_sync(int op, struct net_buf *b, struct net_buf **rsp) {(void)op;(void)b; static struct net_buf r; r.len=0; *rsp=&r; return 0;}

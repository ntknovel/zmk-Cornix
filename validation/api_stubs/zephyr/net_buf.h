#pragma once
#include <stddef.h>
#include <stdint.h>
struct net_buf { uint8_t data[64]; size_t len; };
static inline void *net_buf_add(struct net_buf *b, size_t n) {void *p=&b->data[b->len]; b->len+=n; return p;}
static inline void net_buf_unref(struct net_buf *b) {(void)b;}

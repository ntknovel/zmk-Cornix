#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <zephyr/device.h>
struct zmk_behavior_binding { const char *behavior_dev; uint32_t param1; uint32_t param2; };
struct zmk_behavior_binding_event { int layer; uint32_t position; int64_t timestamp; uint8_t source; };
#define ZMK_BEHAVIOR_OPAQUE 0
static inline const struct device *zmk_behavior_get_binding(const char *name) {(void)name; static struct device d; return &d;}
static inline int zmk_behavior_invoke_binding(const struct zmk_behavior_binding *b, struct zmk_behavior_binding_event e, bool p) {(void)b;(void)e;(void)p;return 0;}

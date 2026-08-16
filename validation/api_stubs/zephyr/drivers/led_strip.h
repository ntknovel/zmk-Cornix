#pragma once
#include <stddef.h>
#include <stdint.h>
#include <zephyr/device.h>
struct led_rgb { uint8_t r; uint8_t g; uint8_t b; };
static inline int led_strip_update_rgb(const struct device *d, struct led_rgb *p, size_t n) {(void)d;(void)p;(void)n;return 0;}

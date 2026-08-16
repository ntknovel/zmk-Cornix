#pragma once
#include <stdbool.h>
#include <zephyr/device.h>
struct gpio_dt_spec { const struct device *port; unsigned int pin; unsigned int dt_flags; };
#define GPIO_DT_SPEC_GET(node, prop) ((struct gpio_dt_spec){0})
#define GPIO_DT_SPEC_GET_BY_IDX(node, prop, idx) ((struct gpio_dt_spec){0})
#define GPIO_OUTPUT_INACTIVE 0
static inline bool gpio_is_ready_dt(const struct gpio_dt_spec *s) {(void)s; return true;}
static inline int gpio_pin_configure_dt(const struct gpio_dt_spec *s, int flags) {(void)s;(void)flags;return 0;}
static inline int gpio_pin_set_dt(const struct gpio_dt_spec *s, int value) {(void)s;(void)value;return 0;}

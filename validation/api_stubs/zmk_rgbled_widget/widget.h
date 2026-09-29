#pragma once
#include <stdbool.h>
#include <stdint.h>
enum status_type {
    STATUS_BATTERY = 0,
    STATUS_CONNECTIVITY = 1,
    STATUS_LAYER = 2,
    STATUS_CUSTOM = 3,
    STATUS_CAPSLOCK = 4,
};
#define WS2812_COLOR_BLACK 0
#define WS2812_COLOR_WHITE 7
static inline void indicate_battery(void) {}
static inline void indicate_connectivity(void) {}
static inline int ws2812_set_status_led(enum status_type type, uint8_t color,
                                         uint16_t duration, bool persistent) {
    (void)type; (void)color; (void)duration; (void)persistent; return 0;
}
static inline int ws2812_clear_status_led(enum status_type type) { (void)type; return 0; }

static inline int ws2812_clear_led(uint8_t led_index) { (void)led_index; return 0; }

/* SPDX-License-Identifier: MIT */
#pragma once

#include <stdbool.h>
#include <stdint.h>

/*
 * One byte is dispatched through the global split behavior:
 *   bit 7     STENO mode is active
 *   bit 6     STENO input feedback is enabled
 *   bit 5     mode-entry all-on flash is active
 *   bits 0-4  number of recognized physical STENO input keys held
 */
#define CST_LED_HELD_MASK   UINT8_C(0x1f)
#define CST_LED_ENTRY_FLASH UINT8_C(0x20)
#define CST_LED_ENABLED     UINT8_C(0x40)
#define CST_LED_ACTIVE      UINT8_C(0x80)

static inline uint8_t cornix_steno_led_clamp_held_count(uint8_t held_count) {
    return held_count > CST_LED_HELD_MASK ? CST_LED_HELD_MASK : held_count;
}

static inline uint8_t cornix_steno_led_pack_state(bool active, bool enabled,
                                                   bool entry_flash,
                                                   uint8_t held_count) {
    uint8_t packed = cornix_steno_led_clamp_held_count(held_count);
    if (active) {
        packed |= CST_LED_ACTIVE;
    }
    if (enabled) {
        packed |= CST_LED_ENABLED;
    }
    if (active && enabled && entry_flash) {
        packed |= CST_LED_ENTRY_FLASH;
    }
    return packed;
}

static inline bool cornix_steno_led_state_active(uint8_t packed_state) {
    return (packed_state & CST_LED_ACTIVE) != 0;
}

static inline bool cornix_steno_led_state_enabled(uint8_t packed_state) {
    return (packed_state & CST_LED_ENABLED) != 0;
}

static inline bool cornix_steno_led_state_entry_flash(uint8_t packed_state) {
    return (packed_state & CST_LED_ENTRY_FLASH) != 0;
}

static inline uint8_t cornix_steno_led_state_held_count(uint8_t packed_state) {
    return packed_state & CST_LED_HELD_MASK;
}

static inline bool cornix_steno_led_global_index_is_on(uint8_t packed_state,
                                                        uint8_t global_led_index) {
    if (!cornix_steno_led_state_active(packed_state) ||
        !cornix_steno_led_state_enabled(packed_state)) {
        return false;
    }
    if (cornix_steno_led_state_entry_flash(packed_state)) {
        return true;
    }
    return cornix_steno_led_state_held_count(packed_state) > global_led_index;
}

#if defined(CONFIG_CORNIX_STENO_LED) && CONFIG_CORNIX_STENO_LED
void cornix_steno_led_mode_changed(bool active);
void cornix_steno_led_key_pressed(uint32_t position);
void cornix_steno_led_key_released(uint32_t position);
void cornix_steno_led_reset_keys(void);
int cornix_steno_led_toggle(void);
bool cornix_steno_led_is_enabled(void);
int cornix_steno_led_apply_packed(uint8_t packed_state);
#else
static inline void cornix_steno_led_mode_changed(bool active) { (void)active; }
static inline void cornix_steno_led_key_pressed(uint32_t position) { (void)position; }
static inline void cornix_steno_led_key_released(uint32_t position) { (void)position; }
static inline void cornix_steno_led_reset_keys(void) {}
static inline int cornix_steno_led_toggle(void) { return 0; }
static inline bool cornix_steno_led_is_enabled(void) { return false; }
static inline int cornix_steno_led_apply_packed(uint8_t packed_state) {
    (void)packed_state;
    return 0;
}
#endif

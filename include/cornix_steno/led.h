/* SPDX-License-Identifier: MIT */
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <cornix_steno/dictionary.h>

/*
 * One byte is dispatched through the global split behavior:
 *   bit 7      STENO mode active
 *   bit 6      feedback enabled
 *   bit 5      mode-entry all-white flash
 *   bits 4..3 abbreviation category
 *   bits 2..0 recognized physical STENO keys held (saturated at four)
 */
#define CST_LED_HELD_MASK      UINT8_C(0x07)
#define CST_LED_CATEGORY_MASK  UINT8_C(0x18)
#define CST_LED_CATEGORY_SHIFT UINT8_C(3)
#define CST_LED_ENTRY_FLASH    UINT8_C(0x20)
#define CST_LED_ENABLED        UINT8_C(0x40)
#define CST_LED_ACTIVE         UINT8_C(0x80)

/* zmk-rgbled-widget uses an RGB bit mask: R=1, G=2, B=4. */
#define CST_LED_COLOR_OFF    UINT8_C(0)
#define CST_LED_COLOR_RED    UINT8_C(1)
#define CST_LED_COLOR_GREEN  UINT8_C(2)
#define CST_LED_COLOR_YELLOW UINT8_C(3)
#define CST_LED_COLOR_BLUE   UINT8_C(4)
#define CST_LED_COLOR_WHITE  UINT8_C(7)

static inline uint8_t cornix_steno_led_clamp_held_count(uint8_t held_count) {
    return held_count > 4 ? 4 : held_count;
}

static inline uint8_t cornix_steno_led_pack_state(
    bool active, bool enabled, bool entry_flash, uint8_t held_count,
    enum cornix_steno_abbreviation_category category) {
    uint8_t packed = cornix_steno_led_clamp_held_count(held_count);
    packed |= ((uint8_t)category << CST_LED_CATEGORY_SHIFT) & CST_LED_CATEGORY_MASK;
    if (active) packed |= CST_LED_ACTIVE;
    if (enabled) packed |= CST_LED_ENABLED;
    if (active && enabled && entry_flash) packed |= CST_LED_ENTRY_FLASH;
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

static inline enum cornix_steno_abbreviation_category
cornix_steno_led_state_category(uint8_t packed_state) {
    return (enum cornix_steno_abbreviation_category)(
        (packed_state & CST_LED_CATEGORY_MASK) >> CST_LED_CATEGORY_SHIFT);
}

/*
 * Actual Cornix physical order established on the user's keyboard:
 *   logical 0 = left inner, logical 1 = left outer,
 *   logical 2 = right outer, logical 3 = right inner.
 * The production widget's local index is reversed on the left half only.
 */
static inline uint8_t cornix_steno_led_global_index_for_local(bool central,
                                                               uint8_t local_index) {
    if (central) {
        return local_index == 0 ? 1 : 0;
    }
    return local_index == 0 ? 2 : 3;
}

static inline uint8_t cornix_steno_led_global_color(uint8_t packed_state,
                                                     uint8_t global_led_index) {
    if (global_led_index >= 4 || !cornix_steno_led_state_active(packed_state) ||
        !cornix_steno_led_state_enabled(packed_state)) {
        return CST_LED_COLOR_OFF;
    }
    if (cornix_steno_led_state_entry_flash(packed_state)) {
        return CST_LED_COLOR_WHITE;
    }

    switch (cornix_steno_led_state_category(packed_state)) {
    case CST_ABBR_CATEGORY_BASIC:
        return global_led_index <= 1 ? CST_LED_COLOR_GREEN : CST_LED_COLOR_OFF;
    case CST_ABBR_CATEGORY_CONNECTIVE:
        return (global_led_index == 1 || global_led_index == 2)
                   ? CST_LED_COLOR_YELLOW
                   : CST_LED_COLOR_OFF;
    case CST_ABBR_CATEGORY_TAIL:
        return global_led_index >= 2 ? CST_LED_COLOR_BLUE : CST_LED_COLOR_OFF;
    case CST_ABBR_CATEGORY_NONE:
    default:
        return cornix_steno_led_state_held_count(packed_state) > global_led_index
                   ? CST_LED_COLOR_WHITE
                   : CST_LED_COLOR_OFF;
    }
}

static inline bool cornix_steno_led_global_index_is_on(uint8_t packed_state,
                                                        uint8_t global_led_index) {
    return cornix_steno_led_global_color(packed_state, global_led_index) !=
           CST_LED_COLOR_OFF;
}

#if defined(CONFIG_CORNIX_STENO_LED) && CONFIG_CORNIX_STENO_LED
void cornix_steno_led_mode_changed(bool active);
void cornix_steno_led_key_pressed(uint32_t position);
void cornix_steno_led_key_released(uint32_t position);
void cornix_steno_led_reset_keys(void);
void cornix_steno_led_set_category(enum cornix_steno_abbreviation_category category);
void cornix_steno_led_confirm_category(enum cornix_steno_abbreviation_category category);
int cornix_steno_led_toggle(void);
bool cornix_steno_led_is_enabled(void);
int cornix_steno_led_apply_packed(uint8_t packed_state);
#else
static inline void cornix_steno_led_mode_changed(bool active) { (void)active; }
static inline void cornix_steno_led_key_pressed(uint32_t position) { (void)position; }
static inline void cornix_steno_led_key_released(uint32_t position) { (void)position; }
static inline void cornix_steno_led_reset_keys(void) {}
static inline void cornix_steno_led_set_category(
    enum cornix_steno_abbreviation_category category) { (void)category; }
static inline void cornix_steno_led_confirm_category(
    enum cornix_steno_abbreviation_category category) { (void)category; }
static inline int cornix_steno_led_toggle(void) { return 0; }
static inline bool cornix_steno_led_is_enabled(void) { return false; }
static inline int cornix_steno_led_apply_packed(uint8_t packed_state) {
    (void)packed_state;
    return 0;
}
#endif

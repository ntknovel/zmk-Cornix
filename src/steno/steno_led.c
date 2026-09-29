/* SPDX-License-Identifier: MIT */

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

#include <zephyr/devicetree.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include <zmk/battery.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/events/endpoint_changed.h>
#include <zmk_rgbled_widget/widget.h>

#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
#define CST_STENO_CENTRAL 1
#include <zmk/behavior.h>
#include <zmk/events/position_state_changed.h>
#else
#define CST_STENO_CENTRAL 0
#endif

#if IS_ENABLED(CONFIG_ZMK_USB)
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/usb.h>
#endif

#include <cornix_steno/led.h>

LOG_MODULE_REGISTER(cornix_steno_led, CONFIG_ZMK_LOG_LEVEL);

#if !DT_NODE_EXISTS(DT_NODELABEL(st_led))
#error "cornix_steno.dtsi did not define the st_led behavior"
#endif

BUILD_ASSERT(CONFIG_RGBLED_WIDGET_LED_COUNT == 2,
             "Cornix STENO feedback expects two indicator LEDs per half");

#define CST_LOCAL_LED_COUNT UINT8_C(2)
#define CST_TOTAL_LED_COUNT UINT8_C(4)

K_MUTEX_DEFINE(led_state_mutex);
static struct k_work_delayable render_work;

#if CST_STENO_CENTRAL
static struct k_work_delayable entry_flash_work;
static struct k_work_delayable category_flash_work;
static bool master_active;
static bool master_enabled = true;
static bool master_entry_flash;
static uint8_t master_held_count;
static uint64_t master_physical_down_mask;
static enum cornix_steno_abbreviation_category master_category;
static uint8_t last_published_state = UINT8_MAX;
#endif

static uint8_t applied_packed_state = CST_LED_ENABLED;
static bool rendered_active;
static bool initialized;

#if CST_STENO_CENTRAL
static uint8_t build_master_state_locked(void) {
    return cornix_steno_led_pack_state(master_active, master_enabled,
                                       master_entry_flash, master_held_count,
                                       master_category);
}

static uint8_t count_master_keys_locked(void) {
    return MIN((uint8_t)__builtin_popcountll(master_physical_down_mask),
               CST_TOTAL_LED_COUNT);
}

static int publish_packed_state(uint8_t packed_state) {
    k_mutex_lock(&led_state_mutex, K_FOREVER);
    if (packed_state == last_published_state) {
        k_mutex_unlock(&led_state_mutex);
        return 0;
    }
    last_published_state = packed_state;
    k_mutex_unlock(&led_state_mutex);

    const struct zmk_behavior_binding binding = {
        .behavior_dev = DEVICE_DT_NAME(DT_NODELABEL(st_led)),
        .param1 = packed_state,
        .param2 = 0,
    };
    const struct zmk_behavior_binding_event event = {
        .layer = 0,
        .position = 0,
        .timestamp = k_uptime_get(),
#if IS_ENABLED(CONFIG_ZMK_SPLIT)
        .source = ZMK_POSITION_STATE_CHANGE_SOURCE_LOCAL,
#endif
    };

    const int err = zmk_behavior_invoke_binding(&binding, event, true);
    if (err < 0) {
        k_mutex_lock(&led_state_mutex, K_FOREVER);
        last_published_state = UINT8_MAX;
        k_mutex_unlock(&led_state_mutex);
        LOG_WRN("Failed to publish Cornix STENO LED state: %d", err);
    }
    return err;
}
#endif

static enum status_type status_for_local_led(uint8_t local_index) {
    return local_index == 0 ? STATUS_BATTERY : STATUS_CONNECTIVITY;
}

static int clear_local_led(uint8_t local_index) {
    int err = ws2812_clear_status_led(status_for_local_led(local_index));
    if (err < 0) return err;
    return ws2812_clear_led(local_index);
}

static int set_local_led_color(uint8_t local_index, uint8_t color) {
    if (color == CST_LED_COLOR_OFF) return clear_local_led(local_index);
    int err = ws2812_clear_led(local_index);
    if (err < 0) return err;
    return ws2812_set_status_led(status_for_local_led(local_index), color, 0, true);
}

static void clear_steno_overlay(void) {
    for (uint8_t i = 0; i < CST_LOCAL_LED_COUNT; i++) {
        const int err = clear_local_led(i);
        if (err < 0) LOG_WRN("Failed to clear Cornix STENO LED %u: %d", i, err);
    }
}

static void restore_base_indicators(void) {
#if IS_ENABLED(CONFIG_ZMK_BATTERY_REPORTING)
    const uint8_t level = zmk_battery_state_of_charge();
    bool externally_powered = false;
#if IS_ENABLED(CONFIG_ZMK_USB)
    externally_powered = zmk_usb_is_powered();
#endif
    if (externally_powered ||
        (level > 0 && level <= CONFIG_RGBLED_WIDGET_BATTERY_LEVEL_CRITICAL)) {
        indicate_battery();
    }
#endif
#if IS_ENABLED(CONFIG_ZMK_BLE)
    indicate_connectivity();
#endif
}

static bool this_half_is_central(void) {
#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    return true;
#else
    return false;
#endif
}

static void render_steno_state(uint8_t packed_state) {
    const bool active = cornix_steno_led_state_active(packed_state);
    const bool enabled = cornix_steno_led_state_enabled(packed_state);

    if (!active) {
        if (rendered_active) {
            clear_steno_overlay();
            restore_base_indicators();
        }
        rendered_active = false;
        return;
    }

    rendered_active = true;
    if (!enabled) {
        clear_steno_overlay();
        return;
    }

    for (uint8_t local = 0; local < CST_LOCAL_LED_COUNT; local++) {
        const uint8_t global_index =
            cornix_steno_led_global_index_for_local(this_half_is_central(), local);
        const uint8_t color = cornix_steno_led_global_color(packed_state, global_index);
        const int err = set_local_led_color(local, color);
        if (err < 0) {
            LOG_WRN("Failed to render Cornix STENO LED %u: %d", local, err);
        }
    }
}

static void render_work_handler(struct k_work *work) {
    ARG_UNUSED(work);
    k_mutex_lock(&led_state_mutex, K_FOREVER);
    const uint8_t packed_state = applied_packed_state;
    k_mutex_unlock(&led_state_mutex);
    render_steno_state(packed_state);
}

#if CST_STENO_CENTRAL
static void entry_flash_work_handler(struct k_work *work) {
    ARG_UNUSED(work);
    k_mutex_lock(&led_state_mutex, K_FOREVER);
    master_entry_flash = false;
    const uint8_t packed_state = build_master_state_locked();
    const bool publish = master_active && master_enabled;
    k_mutex_unlock(&led_state_mutex);
    if (publish) publish_packed_state(packed_state);
}

static void category_flash_work_handler(struct k_work *work) {
    ARG_UNUSED(work);
    k_mutex_lock(&led_state_mutex, K_FOREVER);
    master_category = CST_ABBR_CATEGORY_NONE;
    const uint8_t packed_state = build_master_state_locked();
    const bool publish = master_active;
    k_mutex_unlock(&led_state_mutex);
    if (publish) publish_packed_state(packed_state);
}

void cornix_steno_led_mode_changed(bool active) {
    k_work_cancel_delayable(&entry_flash_work);
    k_work_cancel_delayable(&category_flash_work);

    k_mutex_lock(&led_state_mutex, K_FOREVER);
    master_active = active;
    master_physical_down_mask = 0;
    master_held_count = 0;
    master_category = CST_ABBR_CATEGORY_NONE;
    master_entry_flash = active && master_enabled;
    const uint8_t packed_state = build_master_state_locked();
    const bool start_flash = master_entry_flash;
    k_mutex_unlock(&led_state_mutex);

    publish_packed_state(packed_state);
    if (start_flash) {
        k_work_reschedule(&entry_flash_work,
                          K_MSEC(CONFIG_CORNIX_STENO_LED_ENTRY_FLASH_MS));
    }
}

void cornix_steno_led_key_pressed(uint32_t position) {
    if (position >= CONFIG_CORNIX_STENO_MAX_POSITIONS) return;

    k_work_cancel_delayable(&category_flash_work);
    k_work_cancel_delayable(&entry_flash_work);
    k_mutex_lock(&led_state_mutex, K_FOREVER);
    if (!master_active) {
        k_mutex_unlock(&led_state_mutex);
        return;
    }
    master_entry_flash = false;
    master_category = CST_ABBR_CATEGORY_NONE;
    master_physical_down_mask |= UINT64_C(1) << position;
    master_held_count = count_master_keys_locked();
    const uint8_t packed_state = build_master_state_locked();
    k_mutex_unlock(&led_state_mutex);
    publish_packed_state(packed_state);
}

void cornix_steno_led_key_released(uint32_t position) {
    if (position >= CONFIG_CORNIX_STENO_MAX_POSITIONS) return;
    k_mutex_lock(&led_state_mutex, K_FOREVER);
    master_physical_down_mask &= ~(UINT64_C(1) << position);
    master_held_count = count_master_keys_locked();
    const uint8_t packed_state = build_master_state_locked();
    const bool active = master_active;
    k_mutex_unlock(&led_state_mutex);
    if (active) publish_packed_state(packed_state);
}

void cornix_steno_led_reset_keys(void) {
    k_work_cancel_delayable(&category_flash_work);
    k_mutex_lock(&led_state_mutex, K_FOREVER);
    master_physical_down_mask = 0;
    master_held_count = 0;
    master_category = CST_ABBR_CATEGORY_NONE;
    const uint8_t packed_state = build_master_state_locked();
    const bool active = master_active;
    k_mutex_unlock(&led_state_mutex);
    if (active) publish_packed_state(packed_state);
}

void cornix_steno_led_set_category(enum cornix_steno_abbreviation_category category) {
    if (category < CST_ABBR_CATEGORY_NONE || category > CST_ABBR_CATEGORY_TAIL) {
        category = CST_ABBR_CATEGORY_NONE;
    }
    k_mutex_lock(&led_state_mutex, K_FOREVER);
    if (!master_active || master_category == category) {
        k_mutex_unlock(&led_state_mutex);
        return;
    }
    master_category = category;
    const uint8_t packed_state = build_master_state_locked();
    k_mutex_unlock(&led_state_mutex);
    publish_packed_state(packed_state);
}

void cornix_steno_led_confirm_category(enum cornix_steno_abbreviation_category category) {
    if (category <= CST_ABBR_CATEGORY_NONE || category > CST_ABBR_CATEGORY_TAIL) {
        cornix_steno_led_set_category(CST_ABBR_CATEGORY_NONE);
        return;
    }
    k_work_cancel_delayable(&category_flash_work);
    k_mutex_lock(&led_state_mutex, K_FOREVER);
    master_category = category;
    const uint8_t packed_state = build_master_state_locked();
    const bool active = master_active && master_enabled;
    k_mutex_unlock(&led_state_mutex);
    if (active) {
        publish_packed_state(packed_state);
        k_work_reschedule(&category_flash_work,
                          K_MSEC(CONFIG_CORNIX_STENO_LED_CATEGORY_FLASH_MS));
    }
}

int cornix_steno_led_toggle(void) {
    k_work_cancel_delayable(&entry_flash_work);
    k_work_cancel_delayable(&category_flash_work);
    k_mutex_lock(&led_state_mutex, K_FOREVER);
    master_enabled = !master_enabled;
    master_category = CST_ABBR_CATEGORY_NONE;
    master_entry_flash = master_active && master_enabled;
    const bool start_flash = master_entry_flash;
    const uint8_t packed_state = build_master_state_locked();
    k_mutex_unlock(&led_state_mutex);
    const int err = publish_packed_state(packed_state);
    if (start_flash) {
        k_work_reschedule(&entry_flash_work,
                          K_MSEC(CONFIG_CORNIX_STENO_LED_ENTRY_FLASH_MS));
    }
    return err;
}

bool cornix_steno_led_is_enabled(void) {
    k_mutex_lock(&led_state_mutex, K_FOREVER);
    const bool enabled = master_enabled;
    k_mutex_unlock(&led_state_mutex);
    return enabled;
}
#endif

int cornix_steno_led_apply_packed(uint8_t packed_state) {
    packed_state &= (CST_LED_ACTIVE | CST_LED_ENABLED | CST_LED_ENTRY_FLASH |
                     CST_LED_CATEGORY_MASK | CST_LED_HELD_MASK);

    k_mutex_lock(&led_state_mutex, K_FOREVER);
    const bool was_active = cornix_steno_led_state_active(applied_packed_state);
    applied_packed_state = packed_state;
    const bool is_active = cornix_steno_led_state_active(packed_state);
    const bool ready = initialized;
    k_mutex_unlock(&led_state_mutex);

    if (!ready) return -EAGAIN;
    if (!was_active && !is_active) return 0;
    k_work_reschedule(&render_work, K_NO_WAIT);
    return 0;
}

static int cornix_steno_led_reassert_listener_cb(const zmk_event_t *eh) {
    ARG_UNUSED(eh);
    k_mutex_lock(&led_state_mutex, K_FOREVER);
    const bool active = cornix_steno_led_state_active(applied_packed_state);
    const bool ready = initialized;
    k_mutex_unlock(&led_state_mutex);
    if (ready && active) {
        k_work_reschedule(&render_work,
                          K_MSEC(CONFIG_CORNIX_STENO_LED_REASSERT_DELAY_MS));
    }
    return 0;
}

ZMK_LISTENER(cornix_steno_led_reassert_listener,
             cornix_steno_led_reassert_listener_cb);
#if IS_ENABLED(CONFIG_ZMK_BATTERY_REPORTING)
ZMK_SUBSCRIPTION(cornix_steno_led_reassert_listener, zmk_battery_state_changed);
#endif
#if IS_ENABLED(CONFIG_ZMK_USB)
ZMK_SUBSCRIPTION(cornix_steno_led_reassert_listener, zmk_usb_conn_state_changed);
#endif
#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
#if IS_ENABLED(CONFIG_ZMK_BLE)
ZMK_SUBSCRIPTION(cornix_steno_led_reassert_listener, zmk_ble_active_profile_changed);
#endif
#if IS_ENABLED(CONFIG_RGBLED_WIDGET_CONN_SHOW_USB)
ZMK_SUBSCRIPTION(cornix_steno_led_reassert_listener, zmk_endpoint_changed);
#endif
#endif

static int cornix_steno_led_init(void) {
#if CST_STENO_CENTRAL
    k_work_init_delayable(&entry_flash_work, entry_flash_work_handler);
    k_work_init_delayable(&category_flash_work, category_flash_work_handler);
#endif
    k_work_init_delayable(&render_work, render_work_handler);

    k_mutex_lock(&led_state_mutex, K_FOREVER);
#if CST_STENO_CENTRAL
    master_active = false;
    master_enabled = true;
    master_entry_flash = false;
    master_held_count = 0;
    master_physical_down_mask = 0;
    master_category = CST_ABBR_CATEGORY_NONE;
    last_published_state = UINT8_MAX;
#endif
    applied_packed_state = CST_LED_ENABLED;
    rendered_active = false;
    initialized = true;
    k_mutex_unlock(&led_state_mutex);
    return 0;
}

SYS_INIT(cornix_steno_led_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

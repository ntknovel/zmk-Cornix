/* SPDX-License-Identifier: MIT */

#include <errno.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>

#include <zmk/behavior.h>
#include <zmk/events/position_state_changed.h>

#include <cornix_steno/quick.h>

#if !DT_NODE_EXISTS(DT_NODELABEL(quick_0))
#error "cornix.keymap must define quick_0"
#endif
#if !DT_NODE_EXISTS(DT_NODELABEL(quick_1))
#error "cornix.keymap must define quick_1"
#endif
#if !DT_NODE_EXISTS(DT_NODELABEL(quick_2))
#error "cornix.keymap must define quick_2"
#endif
#if !DT_NODE_EXISTS(DT_NODELABEL(quick_3))
#error "cornix.keymap must define quick_3"
#endif
#if !DT_NODE_EXISTS(DT_NODELABEL(quick_4))
#error "cornix.keymap must define quick_4"
#endif
#if !DT_NODE_EXISTS(DT_NODELABEL(quick_5))
#error "cornix.keymap must define quick_5"
#endif
#if !DT_NODE_EXISTS(DT_NODELABEL(quick_6))
#error "cornix.keymap must define quick_6"
#endif
#if !DT_NODE_EXISTS(DT_NODELABEL(quick_7))
#error "cornix.keymap must define quick_7"
#endif
#if !DT_NODE_EXISTS(DT_NODELABEL(quick_8))
#error "cornix.keymap must define quick_8"
#endif
#if !DT_NODE_EXISTS(DT_NODELABEL(quick_9))
#error "cornix.keymap must define quick_9"
#endif
#if !DT_NODE_EXISTS(DT_NODELABEL(quick_10))
#error "cornix.keymap must define quick_10"
#endif
#if !DT_NODE_EXISTS(DT_NODELABEL(quick_11))
#error "cornix.keymap must define quick_11"
#endif

static const char *const macro_devices[CST_QUICK_COUNT] = {
    [CST_QUICK_M0] = DEVICE_DT_NAME(DT_NODELABEL(quick_0)),
    [CST_QUICK_M1] = DEVICE_DT_NAME(DT_NODELABEL(quick_1)),
    [CST_QUICK_M2] = DEVICE_DT_NAME(DT_NODELABEL(quick_2)),
    [CST_QUICK_M3] = DEVICE_DT_NAME(DT_NODELABEL(quick_3)),
    [CST_QUICK_M4] = DEVICE_DT_NAME(DT_NODELABEL(quick_4)),
    [CST_QUICK_M5] = DEVICE_DT_NAME(DT_NODELABEL(quick_5)),
    [CST_QUICK_M6] = DEVICE_DT_NAME(DT_NODELABEL(quick_6)),
    [CST_QUICK_M7] = DEVICE_DT_NAME(DT_NODELABEL(quick_7)),
    [CST_QUICK_M8] = DEVICE_DT_NAME(DT_NODELABEL(quick_8)),
    [CST_QUICK_M9] = DEVICE_DT_NAME(DT_NODELABEL(quick_9)),
    [CST_QUICK_M10] = DEVICE_DT_NAME(DT_NODELABEL(quick_10)),
    [CST_QUICK_M11] = DEVICE_DT_NAME(DT_NODELABEL(quick_11)),
};

int cornix_steno_quick_invoke(const struct cornix_steno_quick_entry *entry,
                              int64_t timestamp) {
    if (!entry || entry->macro_slot < 0 || entry->macro_slot >= CST_QUICK_COUNT) {
        return -EINVAL;
    }

    const char *behavior_dev = macro_devices[entry->macro_slot];
    if (!behavior_dev) {
        return -ENODEV;
    }

    const struct zmk_behavior_binding binding = {
        .behavior_dev = behavior_dev,
        .param1 = 0,
        .param2 = 0,
    };
    const struct zmk_behavior_binding_event event = {
        .layer = 0,
        .position = 0,
        .timestamp = timestamp,
#if IS_ENABLED(CONFIG_ZMK_SPLIT)
        .source = ZMK_POSITION_STATE_CHANGE_SOURCE_LOCAL,
#endif
    };

    int err = zmk_behavior_invoke_binding(&binding, event, true);
    if (err < 0) {
        return err;
    }
    return zmk_behavior_invoke_binding(&binding, event, false);
}

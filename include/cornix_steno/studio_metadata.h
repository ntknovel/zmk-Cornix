/* SPDX-License-Identifier: MIT */
#pragma once
#include <errno.h>
#include <stddef.h>
#include <drivers/behavior.h>

#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
static inline int cornix_steno_studio_hid_metadata(
    const struct device *dev, struct behavior_parameter_metadata *out) {
    (void)dev;
    if (out == NULL) return -EINVAL;
    static const struct behavior_parameter_value_metadata values[] = {
        {.display_name = "Key", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_HID_USAGE},
    };
    static const struct behavior_parameter_metadata_set sets[] = {
        {.param1_values = values, .param1_values_len = 1},
    };
    *out = (struct behavior_parameter_metadata){.sets_len = 1, .sets = sets};
    return 0;
}

static inline int cornix_steno_studio_led_metadata(
    const struct device *dev, struct behavior_parameter_metadata *out) {
    (void)dev;
    if (out == NULL) return -EINVAL;
    static const struct behavior_parameter_value_metadata values[] = {
        {.display_name = "Internal LED state (do not assign)",
         .type = BEHAVIOR_PARAMETER_VALUE_TYPE_RANGE, .range = {.min = 0, .max = 255}},
    };
    static const struct behavior_parameter_metadata_set sets[] = {
        {.param1_values = values, .param1_values_len = 1},
    };
    *out = (struct behavior_parameter_metadata){.sets_len = 1, .sets = sets};
    return 0;
}
#endif

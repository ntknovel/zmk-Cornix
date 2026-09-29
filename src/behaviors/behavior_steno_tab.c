/* SPDX-License-Identifier: MIT */
#define DT_DRV_COMPAT zmk_behavior_cornix_steno_tab

#include <zephyr/device.h>
#include <zephyr/sys/util.h>
#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <cornix_steno/tab.h>
#include <cornix_steno/led.h>

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)
static int on_pressed(struct zmk_behavior_binding *binding,
                      struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);
    cornix_steno_led_key_pressed(event.position);
    return cornix_steno_tab_pressed(event.position, event.timestamp);
}
static int on_released(struct zmk_behavior_binding *binding,
                       struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);
    const int err = cornix_steno_tab_released(event.position, event.timestamp);
    cornix_steno_led_key_released(event.position);
    return err;
}
static const struct behavior_driver_api api = {
    .binding_pressed = on_pressed,
    .binding_released = on_released,
    .locality = BEHAVIOR_LOCALITY_CENTRAL,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .get_parameter_metadata = zmk_behavior_get_empty_param_metadata,
#endif
};
#define STENO_TAB_INST(n)                                                                         \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                               \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &api);
DT_INST_FOREACH_STATUS_OKAY(STENO_TAB_INST)
#endif

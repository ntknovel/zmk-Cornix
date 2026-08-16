/* SPDX-License-Identifier: MIT */
#define DT_DRV_COMPAT zmk_behavior_cornix_steno_role

#include <errno.h>
#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <cornix_steno/dual.h>
#include <cornix_steno/engine.h>
#include <cornix_steno/led.h>
#include <cornix_steno/roles.h>
#include <cornix_steno/tab.h>

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)
struct behavior_steno_role_config { enum cornix_steno_role role; };

static int on_pressed(struct zmk_behavior_binding *binding,
                      struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    if (!dev) return -ENODEV;
    const struct behavior_steno_role_config *cfg = dev->config;
    cornix_steno_led_key_pressed(event.position);
    cornix_steno_tab_notify_other_press(event.position, event.timestamp);
    cornix_steno_dual_notify_other_press(event.position, event.timestamp);
    return cornix_steno_engine_role_pressed(cfg->role, event.position, event.timestamp);
}

static int on_released(struct zmk_behavior_binding *binding,
                       struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    if (!dev) return -ENODEV;
    const struct behavior_steno_role_config *cfg = dev->config;
    const int err = cornix_steno_engine_role_released(cfg->role, event.position, event.timestamp);
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

#define STENO_ROLE_INST(n)                                                                        \
    static const struct behavior_steno_role_config config_##n = {                                 \
        .role = DT_INST_PROP(n, role),                                                             \
    };                                                                                             \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, &config_##n, POST_KERNEL,                         \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &api);
DT_INST_FOREACH_STATUS_OKAY(STENO_ROLE_INST)
#endif

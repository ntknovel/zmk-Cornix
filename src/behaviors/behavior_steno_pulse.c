/* SPDX-License-Identifier: MIT */
#define DT_DRV_COMPAT zmk_behavior_cornix_steno_pulse

#include <errno.h>
#include <stdbool.h>
#include <zephyr/device.h>
#include <zephyr/sys/util.h>
#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <cornix_steno/dual.h>
#include <cornix_steno/engine.h>
#include <cornix_steno/output.h>
#include <cornix_steno/tab.h>

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)
struct behavior_steno_pulse_config { uint32_t keycode; bool cancel_stroke; };

static int on_pressed(struct zmk_behavior_binding *binding,
                      struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);
    ARG_UNUSED(event);
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_released(struct zmk_behavior_binding *binding,
                       struct zmk_behavior_binding_event event) {
    ARG_UNUSED(event);
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    if (!dev) return -ENODEV;
    const struct behavior_steno_pulse_config *cfg = dev->config;
    if (cfg->cancel_stroke) {
        cornix_steno_output_flush();
        cornix_steno_tab_reset();
        cornix_steno_dual_reset();
        cornix_steno_engine_cancel_pending();
    }
    const int err = cornix_steno_output_enqueue(cfg->keycode);
    return err ? err : ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api api = {
    .binding_pressed = on_pressed,
    .binding_released = on_released,
    .locality = BEHAVIOR_LOCALITY_CENTRAL,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .get_parameter_metadata = zmk_behavior_get_empty_param_metadata,
#endif
};
#define STENO_PULSE_INST(n)                                                                       \
    static const struct behavior_steno_pulse_config config_##n = {                                \
        .keycode = DT_INST_PROP(n, keycode),                                                      \
        .cancel_stroke = DT_INST_PROP_OR(n, cancel_stroke, false),                                \
    };                                                                                             \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, &config_##n, POST_KERNEL,                         \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &api);
DT_INST_FOREACH_STATUS_OKAY(STENO_PULSE_INST)
#endif

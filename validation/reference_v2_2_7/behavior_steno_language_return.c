/* SPDX-License-Identifier: MIT */
#define DT_DRV_COMPAT zmk_behavior_cornix_steno_language_return

#include <errno.h>
#include <stdbool.h>
#include <zephyr/device.h>
#include <zephyr/sys/util.h>
#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/keymap.h>
#include <dt-bindings/zmk/keys.h>
#include <cornix_steno/dual.h>
#include <cornix_steno/engine.h>
#include <cornix_steno/english_return.h>
#include <cornix_steno/output.h>
#include <cornix_steno/tab.h>

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)
struct behavior_steno_language_return_config { uint8_t layer; };

static int on_pressed(struct zmk_behavior_binding *binding,
                      struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);
    ARG_UNUSED(event);
    return ZMK_BEHAVIOR_OPAQUE;
}

static void reset_steno_input(void) {
    cornix_steno_output_flush();
    cornix_steno_tab_reset();
    cornix_steno_dual_reset();
    cornix_steno_engine_cancel_pending();
}

static int on_released(struct zmk_behavior_binding *binding,
                       struct zmk_behavior_binding_event event) {
    ARG_UNUSED(event);
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    if (!dev) return -ENODEV;
    const struct behavior_steno_language_return_config *cfg = dev->config;

    if (zmk_keymap_layer_active(cfg->layer)) {
        reset_steno_input();
        const int err = cornix_steno_output_enqueue(RALT);
        if (err) return err;
        cornix_steno_engine_set_active(false);
        cornix_steno_english_return_set_pending(true);
        const int layer_err = zmk_keymap_layer_deactivate(cfg->layer, false);
        return layer_err ? layer_err : ZMK_BEHAVIOR_OPAQUE;
    }

    if (cornix_steno_english_return_take_pending()) {
        reset_steno_input();
        const int err = cornix_steno_output_enqueue(RALT);
        if (err) return err;
        cornix_steno_engine_set_active(true);
        const int layer_err = zmk_keymap_layer_activate(cfg->layer, false);
        return layer_err ? layer_err : ZMK_BEHAVIOR_OPAQUE;
    }

    /* Ordinary BASE keeps its original right-encoder Ctrl+F action. */
    const int err = cornix_steno_output_enqueue(LC(F));
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

#define STENO_LANGUAGE_RETURN_INST(n)                                                              \
    static const struct behavior_steno_language_return_config config_##n = {                        \
        .layer = DT_INST_PROP(n, layer),                                                            \
    };                                                                                              \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, &config_##n, POST_KERNEL,                         \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &api);
DT_INST_FOREACH_STATUS_OKAY(STENO_LANGUAGE_RETURN_INST)
#endif

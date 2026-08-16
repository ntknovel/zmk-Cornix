/*
 * Runtime BLE transmit-power behavior for ZMK on nRF52840.
 *
 * User-visible levels:
 *   0 dBm -> +4 dBm -> +8 dBm
 *
 * The compile-time radio default remains +8 dBm. Runtime changes are applied
 * to active BLE connection handles only, so advertising and reboot recovery
 * stay at maximum output. The selected runtime level is deliberately not
 * written to flash; every reboot starts at +8 dBm.
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_rf_power

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/hci_vs.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/net_buf.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/util.h>

#include <drivers/behavior.h>
#include <zmk/behavior.h>

LOG_MODULE_REGISTER(zmk_behavior_rf_power, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

BUILD_ASSERT(IS_ENABLED(CONFIG_BT_HAS_HCI_VS),
             "RF power behavior requires Zephyr vendor-specific HCI support");
BUILD_ASSERT(IS_ENABLED(CONFIG_BT_CTLR_TX_PWR_DYNAMIC_CONTROL),
             "RF power behavior requires CONFIG_BT_CTLR_TX_PWR_DYNAMIC_CONTROL=y");

enum rf_power_command {
    RF_POWER_DOWN = 0,
    RF_POWER_ZERO = 1,
    RF_POWER_UP = 2,
    RF_POWER_MAX = 3,
};

enum rf_power_level {
    RF_POWER_LEVEL_ZERO = 0,
    RF_POWER_LEVEL_PLUS_4 = 4,
    RF_POWER_LEVEL_PLUS_8 = 8,
};

/*
 * Relative commands must be converted on the central into one absolute level
 * before a global behavior is dispatched to all split parts.
 */
#define RF_POWER_ABS_MAGIC 0x52504600U
#define RF_POWER_ABS_MASK 0xFFFFFF00U
#define RF_POWER_ENCODE_ABS(level) (RF_POWER_ABS_MAGIC | ((uint8_t)(level)))

struct behavior_rf_power_config {
    uint8_t command;
};

static atomic_t current_tx_power_dbm = ATOMIC_INIT(RF_POWER_LEVEL_PLUS_8);

static int8_t normalize_level(int level) {
    if (level <= RF_POWER_LEVEL_ZERO) {
        return RF_POWER_LEVEL_ZERO;
    }

    if (level <= RF_POWER_LEVEL_PLUS_4) {
        return RF_POWER_LEVEL_PLUS_4;
    }

    return RF_POWER_LEVEL_PLUS_8;
}

static int command_target(uint8_t command, int current_level) {
    const int8_t current = normalize_level(current_level);

    switch (command) {
    case RF_POWER_DOWN:
        if (current >= RF_POWER_LEVEL_PLUS_8) {
            return RF_POWER_LEVEL_PLUS_4;
        }
        return RF_POWER_LEVEL_ZERO;

    case RF_POWER_ZERO:
        return RF_POWER_LEVEL_ZERO;

    case RF_POWER_UP:
        if (current <= RF_POWER_LEVEL_ZERO) {
            return RF_POWER_LEVEL_PLUS_4;
        }
        return RF_POWER_LEVEL_PLUS_8;

    case RF_POWER_MAX:
        return RF_POWER_LEVEL_PLUS_8;

    default:
        return -EINVAL;
    }
}

static bool decode_absolute_target(uint32_t value, int8_t *target) {
    if ((value & RF_POWER_ABS_MASK) != RF_POWER_ABS_MAGIC) {
        return false;
    }

    const int8_t decoded = (int8_t)(value & 0xFFU);
    if (decoded != RF_POWER_LEVEL_ZERO && decoded != RF_POWER_LEVEL_PLUS_4 &&
        decoded != RF_POWER_LEVEL_PLUS_8) {
        return false;
    }

    *target = decoded;
    return true;
}

static int set_connection_tx_power(struct bt_conn *conn, int8_t requested_dbm,
                                   int8_t *selected_dbm) {
    uint16_t handle;
    int err = bt_hci_get_conn_handle(conn, &handle);
    if (err) {
        return err;
    }

    struct net_buf *buf =
        bt_hci_cmd_create(BT_HCI_OP_VS_WRITE_TX_POWER_LEVEL,
                          sizeof(struct bt_hci_cp_vs_write_tx_power_level));
    if (!buf) {
        return -ENOBUFS;
    }

    struct bt_hci_cp_vs_write_tx_power_level *cp =
        net_buf_add(buf, sizeof(struct bt_hci_cp_vs_write_tx_power_level));
    cp->handle_type = BT_HCI_VS_LL_HANDLE_TYPE_CONN;
    cp->handle = sys_cpu_to_le16(handle);
    cp->tx_power_level = requested_dbm;

    struct net_buf *rsp = NULL;
    err = bt_hci_cmd_send_sync(BT_HCI_OP_VS_WRITE_TX_POWER_LEVEL, buf, &rsp);
    if (err) {
        return err;
    }

    const struct bt_hci_rp_vs_write_tx_power_level *rp = (const void *)rsp->data;
    if (selected_dbm) {
        *selected_dbm = rp->selected_tx_power;
    }

    net_buf_unref(rsp);
    return 0;
}

struct apply_context {
    int8_t target_dbm;
    int first_error;
    uint8_t applied_count;
};

static void apply_to_connection(struct bt_conn *conn, void *user_data) {
    struct apply_context *context = user_data;
    int8_t selected_dbm = context->target_dbm;

    const int err = set_connection_tx_power(conn, context->target_dbm, &selected_dbm);
    if (err) {
        /*
         * bt_conn_foreach() may encounter an object which is not fully
         * connected yet. Those transient cases are not treated as a failure.
         */
        if (err != -ENOTCONN && err != -EINVAL && context->first_error == 0) {
            context->first_error = err;
        }
        return;
    }

    context->applied_count++;
    LOG_INF("BLE TX power set to %d dBm", selected_dbm);
}

static int apply_tx_power(int8_t target_dbm) {
    atomic_set(&current_tx_power_dbm, target_dbm);

    struct apply_context context = {
        .target_dbm = target_dbm,
        .first_error = 0,
        .applied_count = 0,
    };

    bt_conn_foreach(BT_CONN_TYPE_LE, apply_to_connection, &context);

    /*
     * No active BLE link is not an error. The connection callback below
     * applies the selected value when a new link appears.
     */
    if (context.first_error) {
        LOG_WRN("BLE TX power %d dBm requested; a link failed (%d)", target_dbm,
                context.first_error);
        return context.first_error;
    }

    if (context.applied_count == 0) {
        LOG_DBG("BLE TX power %d dBm queued for the next connection", target_dbm);
    }

    return 0;
}

static void on_connected(struct bt_conn *conn, uint8_t conn_err) {
    if (conn_err) {
        return;
    }

    const int8_t target_dbm = (int8_t)atomic_get(&current_tx_power_dbm);
    int8_t selected_dbm = target_dbm;
    const int err = set_connection_tx_power(conn, target_dbm, &selected_dbm);
    if (err) {
        LOG_WRN("Could not apply BLE TX power on a new connection (%d)", err);
        return;
    }

    LOG_INF("New BLE connection uses TX power %d dBm", selected_dbm);
}

BT_CONN_CB_DEFINE(rf_power_connection_callbacks) = {
    .connected = on_connected,
};

static int on_binding_convert_central_state_dependent_params(
    struct zmk_behavior_binding *binding, struct zmk_behavior_binding_event event) {
    ARG_UNUSED(event);

    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    if (!dev) {
        return -ENODEV;
    }

    const struct behavior_rf_power_config *config = dev->config;
    const int target = command_target(config->command, atomic_get(&current_tx_power_dbm));
    if (target < 0) {
        return target;
    }

    binding->param1 = RF_POWER_ENCODE_ABS((int8_t)target);
    return 0;
}

static int on_binding_pressed(struct zmk_behavior_binding *binding,
                              struct zmk_behavior_binding_event event) {
    ARG_UNUSED(event);

    int8_t target_dbm;
    if (!decode_absolute_target(binding->param1, &target_dbm)) {
        const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
        if (!dev) {
            return -ENODEV;
        }

        const struct behavior_rf_power_config *config = dev->config;
        const int target =
            command_target(config->command, atomic_get(&current_tx_power_dbm));
        if (target < 0) {
            return target;
        }

        target_dbm = (int8_t)target;
    }

    const int err = apply_tx_power(target_dbm);
    return err ? err : ZMK_BEHAVIOR_OPAQUE;
}

static int on_binding_released(struct zmk_behavior_binding *binding,
                               struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);
    ARG_UNUSED(event);
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_rf_power_driver_api = {
    .binding_convert_central_state_dependent_params =
        on_binding_convert_central_state_dependent_params,
    .binding_pressed = on_binding_pressed,
    .binding_released = on_binding_released,
    .locality = BEHAVIOR_LOCALITY_GLOBAL,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .get_parameter_metadata = zmk_behavior_get_empty_param_metadata,
#endif
};

#define RF_POWER_INST(n)                                                                         \
    static const struct behavior_rf_power_config behavior_rf_power_config_##n = {                \
        .command = DT_INST_PROP(n, command),                                                      \
    };                                                                                            \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, &behavior_rf_power_config_##n, POST_KERNEL,      \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_rf_power_driver_api);

DT_INST_FOREACH_STATUS_OKAY(RF_POWER_INST)

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */

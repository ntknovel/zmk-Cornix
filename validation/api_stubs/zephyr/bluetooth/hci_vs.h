#pragma once
#include <stdint.h>
#define BT_HCI_OP_VS_WRITE_TX_POWER_LEVEL 1
#define BT_HCI_VS_LL_HANDLE_TYPE_CONN 0
struct bt_hci_cp_vs_write_tx_power_level { uint8_t handle_type; uint16_t handle; int8_t tx_power_level; };
struct bt_hci_rp_vs_write_tx_power_level { uint8_t status; int8_t selected_tx_power; };

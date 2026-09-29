/* SPDX-License-Identifier: MIT */
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <cornix_steno/roles.h>
int cornix_steno_dual_pressed(uint32_t position, enum cornix_steno_dual_policy policy,
                              enum cornix_steno_role role, uint32_t tap_key,
                              uint32_t hold_key, int64_t timestamp);
int cornix_steno_dual_released(uint32_t position, int64_t timestamp);
void cornix_steno_dual_notify_other_press(uint32_t position, int64_t timestamp);
uint8_t cornix_steno_dual_consume_modifiers_for_tab(int64_t timestamp);
bool cornix_steno_dual_take_edit_for_encoder(void);
void cornix_steno_dual_reset(void);

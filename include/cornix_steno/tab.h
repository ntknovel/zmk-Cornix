/* SPDX-License-Identifier: MIT */
#pragma once
#include <stdbool.h>
#include <stdint.h>
int cornix_steno_tab_pressed(uint32_t position, int64_t timestamp);
int cornix_steno_tab_released(uint32_t position, int64_t timestamp);
void cornix_steno_tab_notify_other_press(uint32_t position, int64_t timestamp);
bool cornix_steno_tab_modifier_pressed(uint32_t position, int64_t timestamp);
void cornix_steno_tab_reset(void);

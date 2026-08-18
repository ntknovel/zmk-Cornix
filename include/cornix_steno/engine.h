/* SPDX-License-Identifier: MIT */
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <cornix_steno/roles.h>

/* Positive result means the physical event was consumed by an immediate mode. */
#define CST_ENGINE_EVENT_NORMAL   0
#define CST_ENGINE_EVENT_CONSUMED 1

void cornix_steno_engine_set_active(bool active);
bool cornix_steno_engine_is_active(void);
bool cornix_steno_engine_has_down_keys(void);
void cornix_steno_engine_cancel_pending(void);
int cornix_steno_engine_emit_direct_role(enum cornix_steno_role role);
int cornix_steno_engine_role_pressed(enum cornix_steno_role role, uint32_t position,
                                     int64_t timestamp);
int cornix_steno_engine_role_released(enum cornix_steno_role role, uint32_t position,
                                      int64_t timestamp);

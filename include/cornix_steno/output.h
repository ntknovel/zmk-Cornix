/* SPDX-License-Identifier: MIT */
#pragma once
#include <stddef.h>
#include <stdint.h>
int cornix_steno_output_enqueue(uint32_t encoded_key);
int cornix_steno_output_enqueue_sequence(const uint32_t *keys, size_t count);
void cornix_steno_output_flush(void);
size_t cornix_steno_output_pending(void);

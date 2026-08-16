/* SPDX-License-Identifier: MIT */
#pragma once
#include <stdint.h>
#define CST_DECODE_MAX_KEYS 8
enum cornix_steno_decode_kind { CST_DECODE_NONE = 0, CST_DECODE_KEYS };
struct cornix_steno_decoded {
    enum cornix_steno_decode_kind kind;
    uint32_t keys[CST_DECODE_MAX_KEYS];
    uint8_t key_count;
};
int cornix_steno_decode(uint64_t role_mask, uint64_t position_mask,
                        struct cornix_steno_decoded *decoded);

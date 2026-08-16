/* SPDX-License-Identifier: MIT */
#pragma once
#include <stddef.h>
#include <stdint.h>
struct cornix_steno_quick_entry {
    const char *slot;
    uint64_t role_mask;
    const uint32_t *keys;
    uint16_t key_count;
};
const struct cornix_steno_quick_entry *cornix_steno_quick_lookup(uint64_t role_mask);
size_t cornix_steno_quick_count(void);

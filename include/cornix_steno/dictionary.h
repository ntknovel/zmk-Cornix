/* SPDX-License-Identifier: MIT */
#pragma once
#include <stddef.h>
#include <stdint.h>
struct cornix_steno_dictionary_entry {
    uint8_t bank;
    uint64_t role_mask;
    const uint32_t *keys;
    uint16_t key_count;
};
const struct cornix_steno_dictionary_entry *
cornix_steno_dictionary_lookup(uint8_t bank, uint64_t role_mask);
size_t cornix_steno_dictionary_count(void);

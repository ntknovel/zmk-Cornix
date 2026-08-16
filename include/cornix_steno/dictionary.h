/* SPDX-License-Identifier: MIT */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum cornix_steno_abbreviation_category {
    CST_ABBR_CATEGORY_NONE = 0,
    CST_ABBR_CATEGORY_BASIC = 1,
    CST_ABBR_CATEGORY_CONNECTIVE = 2,
    CST_ABBR_CATEGORY_TAIL = 3,
};

struct cornix_steno_dictionary_entry {
    uint64_t role_mask;
    uint8_t category;
    const uint32_t *keys;
    uint16_t key_count;
};

/*
 * Canonical exact-mask lookup.
 * ABBR_L and ABBR_R remain distinct. AB2 is side-flexible except for the
 * explicitly side-specific right-VEXT tail entry.
 */
uint64_t cornix_steno_dictionary_normalize_mask(uint64_t role_mask);
const struct cornix_steno_dictionary_entry *
cornix_steno_dictionary_lookup(uint64_t role_mask);
bool cornix_steno_dictionary_has_exact(uint64_t role_mask);
bool cornix_steno_dictionary_has_prefix(uint64_t role_mask);
size_t cornix_steno_dictionary_count(void);

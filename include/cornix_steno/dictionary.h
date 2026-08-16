/* SPDX-License-Identifier: MIT */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct cornix_steno_dictionary_entry {
    uint64_t role_mask;
    const uint32_t *keys;
    uint16_t key_count;
};

/*
 * Canonical exact-mask lookup.
 *
 * ABBR_L and ABBR_R remain distinct.  The only side-flexible selector is
 * AB2: a consonant-only stroke containing exactly one VEXT key is normalized
 * to VEXT_L before lookup, so either physical VEXT side may invoke it.
 */
uint64_t cornix_steno_dictionary_normalize_mask(uint64_t role_mask);
const struct cornix_steno_dictionary_entry *
cornix_steno_dictionary_lookup(uint64_t role_mask);
bool cornix_steno_dictionary_has_exact(uint64_t role_mask);
size_t cornix_steno_dictionary_count(void);

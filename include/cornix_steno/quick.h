/* SPDX-License-Identifier: MIT */
#pragma once
#include <stddef.h>
#include <stdint.h>

enum cornix_steno_quick_slot {
    CST_QUICK_M0 = 0,
    CST_QUICK_M1 = 1,
    CST_QUICK_M2 = 2,
    CST_QUICK_M3 = 3,
    CST_QUICK_M4 = 4,
    CST_QUICK_M5 = 5,
    CST_QUICK_M6 = 6,
    CST_QUICK_M7 = 7,
    CST_QUICK_M8 = 8,
    CST_QUICK_M9 = 9,
    CST_QUICK_M10 = 10,
    CST_QUICK_M11 = 11,
    CST_QUICK_COUNT,
};

struct cornix_steno_quick_entry {
    const char *slot;
    uint64_t role_mask;
    enum cornix_steno_quick_slot macro_slot;
};

const struct cornix_steno_quick_entry *cornix_steno_quick_lookup(uint64_t role_mask);
size_t cornix_steno_quick_count(void);
int cornix_steno_quick_invoke(const struct cornix_steno_quick_entry *entry,
                              int64_t timestamp);

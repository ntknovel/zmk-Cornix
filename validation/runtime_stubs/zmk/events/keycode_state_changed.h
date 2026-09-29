#pragma once
#include <stdbool.h>
#include <stdint.h>
int test_raise_keycode(uint32_t key, bool state, int64_t ts);
static inline int raise_zmk_keycode_state_changed_from_encoded(uint32_t key, bool state, int64_t ts) {
    return test_raise_keycode(key, state, ts);
}

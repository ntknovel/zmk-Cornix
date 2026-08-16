/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include <cornix_steno/led.h>

static void assert_lit_count(uint8_t packed, uint8_t expected) {
    uint8_t actual = 0;
    for (uint8_t i = 0; i < 4; i++) {
        actual += cornix_steno_led_global_index_is_on(packed, i) ? 1 : 0;
    }
    assert(actual == expected);
}

int main(void) {
    const uint8_t base = cornix_steno_led_pack_state(false, true, false, 4);
    assert(!cornix_steno_led_state_active(base));
    assert_lit_count(base, 0);

    const uint8_t disabled = cornix_steno_led_pack_state(true, false, false, 4);
    assert(cornix_steno_led_state_active(disabled));
    assert(!cornix_steno_led_state_enabled(disabled));
    assert_lit_count(disabled, 0);

    const uint8_t flash = cornix_steno_led_pack_state(true, true, true, 0);
    assert(cornix_steno_led_state_entry_flash(flash));
    assert_lit_count(flash, 4);

    for (uint8_t count = 0; count <= 4; count++) {
        const uint8_t packed = cornix_steno_led_pack_state(true, true, false, count);
        assert(cornix_steno_led_state_held_count(packed) == count);
        assert_lit_count(packed, count);
        for (uint8_t i = 0; i < 4; i++) {
            assert(cornix_steno_led_global_index_is_on(packed, i) == (i < count));
        }
    }

    /* Counts above physical capacity saturate at render time: all four are on. */
    const uint8_t many = cornix_steno_led_pack_state(true, true, false, 12);
    assert(cornix_steno_led_state_held_count(many) == 12);
    assert_lit_count(many, 4);

    puts("Cornix STENO sequential LED-state tests: PASS");
    return 0;
}

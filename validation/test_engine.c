#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dt-bindings/zmk/keys.h>
#include <zephyr/kernel.h>
#include <cornix_steno/engine.h>
#include <cornix_steno/roles.h>
#include <cornix_steno/quick.h>

extern uint32_t test_output_keys[];
extern size_t test_output_count;
extern size_t test_quick_invoke_count;
extern enum cornix_steno_quick_slot test_quick_last_slot;
void test_output_reset(void);
void test_quick_invoke_reset(void);

static int failures;

static void expect(const char *name, const uint32_t *keys, size_t count) {
    if (test_output_count != count ||
        (count && memcmp(test_output_keys, keys, count * sizeof(keys[0])))) {
        fprintf(stderr, "FAIL %s got=%zu expected=%zu\n", name,
                test_output_count, count);
        failures++;
    }
    test_output_reset();
}

static void press(enum cornix_steno_role role, uint32_t position, int64_t time) {
    test_time_set(time);
    cornix_steno_engine_role_pressed(role, position, time);
}

static void release(enum cornix_steno_role role, uint32_t position, int64_t time) {
    test_time_set(time);
    cornix_steno_engine_role_released(role, position, time);
}

int main(void) {
    cornix_steno_engine_set_active(true);

    /* Ordinary Korean chord and ordinary 50 ms typo recovery remain intact. */
    press(CST_R_I_G, 14, 0);
    press(CST_R_V_A, 44, 8);
    press(CST_R_F_N, 8, 12);
    release(CST_R_I_G, 14, 30);
    release(CST_R_V_A, 44, 35);
    release(CST_R_F_N, 8, 40);
    { const uint32_t e[] = {R, K, S}; expect("normal gan", e, 3); }

    press(CST_R_I_G, 14, 100);
    press(CST_R_V_A, 44, 108);
    press(CST_R_F_N, 8, 112);
    press(CST_R_I_NG, 16, 200);
    release(CST_R_I_NG, 16, 215);
    test_time_set(215);
    test_time_advance(55);
    release(CST_R_I_G, 14, 280);
    release(CST_R_V_A, 44, 285);
    release(CST_R_F_N, 8, 290);
    { const uint32_t e[] = {R, K, S}; expect("late accidental removed", e, 3); }

    press(CST_R_I_G, 14, 400);
    release(CST_R_I_G, 14, 410);
    expect("solo consonant silent", NULL, 0);

    /* Direct canonical exact mask: no ABBR/VEXT bank marker is required. */
    press(CST_R_I_G, 14, 500);
    press(CST_R_F_R, 20, 506);
    release(CST_R_I_G, 14, 530);
    release(CST_R_F_R, 20, 536);
    { const uint32_t e[] = {R, M, F, J, S}; expect("direct 그런", e, 5); }

    /*
     * The long direct exact mask must not shrink to the shorter registered
     * mask when its distinguishing role is a late tap released first.
     */
    press(CST_R_I_G, 14, 600);
    press(CST_R_I_R, 15, 606);
    press(CST_R_I_DOUBLE, 17, 700); /* deliberately later than settle */
    release(CST_R_I_DOUBLE, 17, 740);
    test_time_set(740);
    test_time_advance(60);          /* correction must preserve exact 그리고 */
    release(CST_R_I_G, 14, 830);
    release(CST_R_I_R, 15, 836);
    { const uint32_t e[] = {R, M, F, L, R, H}; expect("direct 그리고 no shrink", e, 6); }

    /* ABBR_L remains a held selector, but ABBR_R is a distinct selector. */
    press(CST_R_ABBR_L, 12, 900);
    press(CST_R_I_G, 14, 980);
    release(CST_R_I_G, 14, 1030);
    test_time_advance(60);
    press(CST_R_F_R, 20, 1110);
    release(CST_R_F_R, 20, 1160);
    test_time_advance(60);
    release(CST_R_ABBR_L, 12, 1240);
    { const uint32_t e[] = {R, M, F, J, A}; expect("ABBR_L sequential 그럼", e, 5); }

    press(CST_R_ABBR_R, 23, 1300);
    press(CST_R_I_G, 14, 1380);
    release(CST_R_I_G, 14, 1430);
    test_time_advance(60);
    press(CST_R_F_R, 20, 1510);
    release(CST_R_F_R, 20, 1560);
    test_time_advance(60);
    release(CST_R_ABBR_R, 23, 1640);
    expect("ABBR_R is not mirrored to 그럼", NULL, 0);

    /* Direct and selector-based tail forms are different exact masks. */
    press(CST_R_F_NG, 19, 1700);
    press(CST_R_F_D, 34, 1706);
    release(CST_R_F_NG, 19, 1730);
    release(CST_R_F_D, 34, 1736);
    { const uint32_t e[] = {D, U, LS(T), E, K}; expect("direct 였다", e, 5); }

    press(CST_R_ABBR_R, 23, 1800);
    press(CST_R_F_NG, 19, 1806);
    press(CST_R_F_D, 34, 1812);
    release(CST_R_F_NG, 19, 1840);
    release(CST_R_F_D, 34, 1846);
    release(CST_R_ABBR_R, 23, 1852);
    { const uint32_t e[] = {D, J, Q, T, E, K}; expect("ABBR_R 없다", e, 6); }

    press(CST_R_ABBR_L, 12, 1900);
    press(CST_R_F_NG, 19, 1906);
    press(CST_R_F_D, 34, 1912);
    release(CST_R_F_NG, 19, 1940);
    release(CST_R_F_D, 34, 1946);
    release(CST_R_ABBR_L, 12, 1952);
    { const uint32_t e[] = {D, J, LS(T), E, K}; expect("ABBR_L 었다", e, 5); }

    /* AB2 accepts either physical VEXT side and can be held sequentially. */
    press(CST_R_VEXT_R, 37, 2000);
    press(CST_R_F_NG, 19, 2080);
    release(CST_R_F_NG, 19, 2130);
    test_time_advance(60);
    press(CST_R_F_D, 34, 2210);
    release(CST_R_F_D, 34, 2260);
    test_time_advance(60);
    release(CST_R_VEXT_R, 37, 2340);
    { const uint32_t e[] = {D, L, D, J, LS(T), E, K}; expect("AB2 right sequential 이었다", e, 7); }

    /* Quick macros remain independent and are dispatched before the dictionary. */
    test_quick_invoke_reset();
    press(CST_R_ABBR_L, 12, 2400);
    press(CST_R_V_EU, 42, 2480);
    release(CST_R_V_EU, 42, 2530);
    test_time_advance(60);
    release(CST_R_ABBR_L, 12, 2610);
    if (test_quick_invoke_count != 1 || test_quick_last_slot != CST_QUICK_M0) {
        fprintf(stderr, "FAIL held quick M0 count=%zu slot=%d\n",
                test_quick_invoke_count, (int)test_quick_last_slot);
        failures++;
    }
    expect("held quick no marker punctuation", NULL, 0);

    press(CST_R_I_G, 14, 2700);
    cornix_steno_engine_set_active(false);
    release(CST_R_I_G, 14, 2720);
    expect("mode exit cancel", NULL, 0);

    if (failures) return EXIT_FAILURE;
    puts("Cornix STENO v1.7.0 canonical dictionary engine tests: PASS");
    return EXIT_SUCCESS;
}

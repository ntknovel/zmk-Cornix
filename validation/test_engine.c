#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dt-bindings/zmk/keys.h>
#include <zephyr/kernel.h>
#include <cornix_steno/engine.h>
#include <cornix_steno/quick.h>
#include <cornix_steno/roles.h>
#include "role_position_map.h"

extern uint32_t test_output_keys[];
extern size_t test_output_count;
void test_output_reset(void);
struct test_key_event { uint32_t key; bool state; int64_t ts; };
extern struct test_key_event test_key_events[];
extern size_t test_key_event_count;
void test_key_events_reset(void);
extern size_t test_quick_invoke_count;
extern enum cornix_steno_quick_slot test_quick_last_slot;
void test_quick_invoke_reset(void);
static int failures;

static void expect(const char *name, const uint32_t *keys, size_t count) {
    if (test_output_count != count ||
        (count && memcmp(test_output_keys, keys, count * sizeof(keys[0])))) {
        fprintf(stderr, "FAIL %s got=%zu expected=%zu\n", name, test_output_count, count);
        failures++;
    }
    test_output_reset();
}
static void expect_events(const char *name, uint32_t key, size_t taps) {
    if (test_key_event_count != taps * 2) {
        fprintf(stderr, "FAIL %s events=%zu expected=%zu\n", name,
                test_key_event_count, taps * 2); failures++;
    } else {
        for (size_t i = 0; i < taps; i++) {
            if (test_key_events[i*2].key != key || !test_key_events[i*2].state ||
                test_key_events[i*2+1].key != key || test_key_events[i*2+1].state) {
                fprintf(stderr, "FAIL %s bad tap %zu\n", name, i); failures++; break;
            }
        }
    }
    test_key_events_reset();
}
static void expect_event_sequence(const char *name, const uint32_t *keys, size_t count) {
    if (test_key_event_count != count * 2) {
        fprintf(stderr, "FAIL %s events=%zu expected=%zu\n", name,
                test_key_event_count, count * 2); failures++;
    } else {
        for (size_t i = 0; i < count; i++) {
            if (test_key_events[i*2].key != keys[i] || !test_key_events[i*2].state ||
                test_key_events[i*2+1].key != keys[i] || test_key_events[i*2+1].state) {
                fprintf(stderr, "FAIL %s bad tap %zu\n", name, i); failures++; break;
            }
        }
    }
    test_key_events_reset();
}

static int press(enum cornix_steno_role x, uint32_t ignored, int64_t t) {
    (void)ignored;
    const uint32_t p = cst_test_position_for_role(x);
    if (p == UINT32_MAX) { fprintf(stderr, "FAIL missing physical position for role %d\n", x); failures++; return -1; }
    test_time_set(t); return cornix_steno_engine_role_pressed(x, p, t);
}
static int release(enum cornix_steno_role x, uint32_t ignored, int64_t t) {
    (void)ignored;
    const uint32_t p = cst_test_position_for_role(x);
    if (p == UINT32_MAX) { fprintf(stderr, "FAIL missing physical position for role %d\n", x); failures++; return -1; }
    test_time_set(t); return cornix_steno_engine_role_released(x, p, t);
}

int main(void) {
    cornix_steno_engine_set_active(true);

    /* Normal release skew inside 50 ms keeps all keys. */
    press(CST_R_I_G,4,0); press(CST_R_V_A,44,8); press(CST_R_F_N,21,12);
    release(CST_R_I_G,4,30); release(CST_R_V_A,44,35); release(CST_R_F_N,21,40);
    const uint32_t gan[]={R,K,S}; expect("normal gan",gan,3);

    /* A wrong key released early is removed while the intended chord stays held. */
    press(CST_R_I_G,4,100); press(CST_R_V_A,44,108); press(CST_R_F_N,21,112);
    press(CST_R_I_NG,15,120); release(CST_R_I_NG,15,130);
    test_time_set(130); test_time_advance(55);
    release(CST_R_I_G,4,210); release(CST_R_V_A,44,220); release(CST_R_F_N,21,230);
    expect("early accidental removed",gan,3);

    /* Direct canonical abbreviation: 그런. */
    press(CST_R_I_G,4,300); press(CST_R_F_R,19,310);
    release(CST_R_I_G,4,330); test_time_advance(55); release(CST_R_F_R,19,400);
    const uint32_t geureon[]={R,M,F,J,S}; expect("direct abbreviation prefix protected",geureon,5);

    /* ABBR anchor held while consonants are tapped sequentially: 그럼. */
    press(CST_R_ABBR_L,12,500); press(CST_R_I_G,4,600); release(CST_R_I_G,4,620);
    test_time_advance(55); press(CST_R_F_R,19,690); release(CST_R_F_R,19,710);
    release(CST_R_ABBR_L,12,730);
    const uint32_t geureom[]={R,M,F,J,A}; expect("held abbreviation anchor",geureom,5);

    /* Quick M0 remains GUI-editable and invokes the macro dispatcher. */
    test_quick_invoke_reset();
    press(CST_R_ABBR_L,12,800); press(CST_R_V_EU,42,810);
    release(CST_R_V_EU,42,830); release(CST_R_ABBR_L,12,840);
    if (test_quick_invoke_count != 1 || test_quick_last_slot != CST_QUICK_M0) {
        fprintf(stderr,"FAIL quick M0 count=%zu slot=%d\n",test_quick_invoke_count,
                test_quick_last_slot); failures++;
    }
    expect("quick no dictionary queue",NULL,0);

    /* ABBR + same-side double repeats plain horizontal movement. */
    test_key_events_reset();
    press(CST_R_ABBR_L,12,900); press(CST_R_I_DOUBLE,29,910); release(CST_R_I_DOUBLE,29,920);
    press(CST_R_I_DOUBLE,29,930); release(CST_R_I_DOUBLE,29,940); release(CST_R_ABBR_L,12,950);
    expect_events("ABBR-L repeated left",LEFT,2);
    press(CST_R_ABBR_L,12,960); press(CST_R_I_KH,25,970); release(CST_R_I_KH,25,980); release(CST_R_ABBR_L,12,990);
    expect_events("ABBR-L KH alias is not navigation",LEFT,0);
    press(CST_R_ABBR_L,12,995); press(CST_R_I_NG,15,1000); release(CST_R_I_NG,15,1010); release(CST_R_ABBR_L,12,1020);
    expect_events("ABBR-L NG alias is not navigation",LEFT,0);

    press(CST_R_ABBR_R,23,1000); press(CST_R_F_DOUBLE,32,1010); release(CST_R_F_DOUBLE,32,1020);
    press(CST_R_F_DOUBLE,32,1030); release(CST_R_F_DOUBLE,32,1040); release(CST_R_ABBR_R,23,1050);
    expect_events("ABBR-R repeated right",RIGHT,2);

    /* SYMBOL + same-side double repeats Shift selection movement. */
    press(CST_R_SYMBOL_L,40,1100); press(CST_R_I_DOUBLE,29,1110); release(CST_R_I_DOUBLE,29,1120);
    press(CST_R_I_DOUBLE,29,1130); release(CST_R_I_DOUBLE,29,1140); release(CST_R_SYMBOL_L,40,1150);
    expect_events("SYMBOL-L repeated select left",LS(LEFT),2);

    press(CST_R_SYMBOL_R,47,1200); press(CST_R_F_DOUBLE,32,1210); release(CST_R_F_DOUBLE,32,1220);
    press(CST_R_F_DOUBLE,32,1230); release(CST_R_F_DOUBLE,32,1240); release(CST_R_SYMBOL_R,47,1250);
    expect_events("SYMBOL-R repeated select right",LS(RIGHT),2);

    /* Either SYMBOL side anchors an immediate repeating number stream. */
    test_key_events_reset();
    press(CST_R_SYMBOL_L,40,1300);
    press(CST_R_I_B,1,1310); release(CST_R_I_B,1,1320);
    press(CST_R_I_J,2,1330); release(CST_R_I_J,2,1340);
    press(CST_R_I_D,3,1350); release(CST_R_I_D,3,1360);
    release(CST_R_SYMBOL_L,40,1370);
    expect_event_sequence("SYMBOL-L immediate numbers", (uint32_t[]){N1,N2,N3}, 3);

    test_key_events_reset();
    press(CST_R_SYMBOL_R,47,1400);
    press(CST_R_F_S,6,1410); release(CST_R_F_S,6,1420);
    press(CST_R_F_G,7,1430); release(CST_R_F_G,7,1440);
    press(CST_R_F_D,8,1450); release(CST_R_F_D,8,1460);
    release(CST_R_SYMBOL_R,47,1470);
    expect_event_sequence("SYMBOL-R immediate numbers", (uint32_t[]){N6,N7,N8}, 3);

    /* Original logical 그러기 mask survives the new same-finger physical layout. */
    test_key_events_reset();
    press(CST_R_ABBR_L,12,1300); press(CST_R_I_G,4,1310); press(CST_R_I_R,16,1320);
    press(CST_R_I_DOUBLE,29,1330); release(CST_R_I_DOUBLE,29,1340);
    release(CST_R_I_G,4,1350); release(CST_R_I_R,16,1360); release(CST_R_ABBR_L,12,1370);
    const uint32_t geureogi[]={R,M,F,J,R,L}; expect("abbr exact survives nav",geureogi,6);
    expect_events("abbr exact no arrow",LEFT,0);

    /* Every jamo role is its own correction key on a solo 80 ms hold-release. */
    press(CST_R_I_G,4,1400); release(CST_R_I_G,4,1480);
    const uint32_t solo_g[]={R}; expect("solo-hold initial giyeok",solo_g,1);

    press(CST_R_F_N,21,1600); release(CST_R_F_N,21,1685);
    const uint32_t solo_n[]={S}; expect("solo-hold final nieun",solo_n,1);

    press(CST_R_V_I,45,1800); release(CST_R_V_I,45,1880);
    const uint32_t solo_i[]={L}; expect("solo-hold vowel i",solo_i,1);
    press(CST_R_V_O,41,1900); release(CST_R_V_O,41,1980); const uint32_t solo_o[]={H}; expect("solo-hold vowel o",solo_o,1);
    press(CST_R_V_EU,42,2000); release(CST_R_V_EU,42,2080); const uint32_t solo_eu[]={M}; expect("solo-hold vowel eu",solo_eu,1);
    press(CST_R_V_EO,44,2100); release(CST_R_V_EO,44,2180); const uint32_t solo_eo[]={J}; expect("solo-hold vowel eo",solo_eo,1);
    press(CST_R_V_A,46,2200); release(CST_R_V_A,46,2280); const uint32_t solo_a[]={K}; expect("solo-hold vowel a",solo_a,1);

    /* Compound vowels are also direct jamo when the entire vowel-only chord is held >=80 ms. */
    press(CST_R_V_A,46,2300); press(CST_R_V_I,45,2310);
    release(CST_R_V_I,45,2390); release(CST_R_V_A,46,2400);
    const uint32_t solo_ae[]={O}; expect("compound-vowel hold ae",solo_ae,1);

    press(CST_R_V_O,41,2420); press(CST_R_V_A,46,2430);
    release(CST_R_V_A,46,2510); release(CST_R_V_O,41,2520);
    const uint32_t solo_wa[]={H,K}; expect("compound-vowel hold wa",solo_wa,2);

    press(CST_R_VEXT_L,11,2540); press(CST_R_V_EO,44,2550);
    release(CST_R_V_EO,44,2630); release(CST_R_VEXT_L,11,2640);
    const uint32_t solo_yeo[]={U}; expect("extended-vowel hold yeo",solo_yeo,1);

    /* Double-consonant chords are direct jamo on the same 80 ms hold-release. */
    press(CST_R_I_KH,1,2660); press(CST_R_I_G,4,2670);
    release(CST_R_I_G,4,2750); release(CST_R_I_KH,1,2760);
    const uint32_t solo_gg1[]={LS(R)}; expect("double hold KH+G",solo_gg1,1);
    press(CST_R_I_NG,18,2780); press(CST_R_I_G,4,2790);
    release(CST_R_I_G,4,2870); release(CST_R_I_NG,18,2880);
    const uint32_t solo_gg2[]={LS(R)}; expect("double hold NG+G",solo_gg2,1);
    press(CST_R_I_DOUBLE,29,2900); press(CST_R_I_G,4,2910);
    release(CST_R_I_G,4,2990); release(CST_R_I_DOUBLE,29,3000);
    const uint32_t solo_gg3[]={LS(R)}; expect("double hold DOUBLE+G",solo_gg3,1);

    /* Undo/Redo are anchored immediate streams, symmetric in press order. */
    test_key_events_reset();
    press(CST_R_V_O,41,3020); press(CST_R_V_U,43,3030); release(CST_R_V_U,43,3040);
    press(CST_R_V_U,43,3050); release(CST_R_V_U,43,3060); release(CST_R_V_O,41,3070);
    expect_event_sequence("undo anchored repeats", (uint32_t[]){LC(Z),LC(Z)}, 2);
    test_key_events_reset();
    press(CST_R_V_EO,44,3090); press(CST_R_V_A,46,3100); release(CST_R_V_A,46,3110);
    release(CST_R_V_EO,44,3120);
    expect_event_sequence("redo reverse-order", (uint32_t[]){LC(Y)}, 1);

    /* Once another STENO role participates, even a long hold is a normal stroke. */
    press(CST_R_I_G,4,2000); press(CST_R_V_A,44,2050);
    release(CST_R_V_A,44,2070); release(CST_R_I_G,4,2200);
    const uint32_t ga[]={R,K}; expect("multi-key overrides solo hold",ga,2);

    /* Physical ㅣ+I/J/K/L navigation remains immediate. */
    test_key_events_reset();
    press(CST_R_V_I,45,1600); press(CST_R_F_D,8,1610); release(CST_R_F_D,8,1630);
    release(CST_R_V_I,45,1640);
    if (test_key_event_count != 2 || test_key_events[0].key != UP ||
        !test_key_events[0].state || test_key_events[1].key != UP ||
        test_key_events[1].state) {
        fprintf(stderr,"FAIL physical I+I navigation events=%zu\n",test_key_event_count); failures++;
    }
    test_key_events_reset(); expect("physical nav no text",NULL,0);

    /* Legal final cluster rolling restores an early first final inside 90 ms. */
    press(CST_R_I_G,4,1700); press(CST_R_V_A,44,1710); press(CST_R_F_R,19,1720);
    release(CST_R_F_R,19,1730); test_time_advance(55);
    press(CST_R_F_G,7,1800); release(CST_R_F_G,7,1810);
    release(CST_R_I_G,4,1820); release(CST_R_V_A,44,1830);
    const uint32_t dalk[]={R,K,F,R}; expect("final roll rieul-giyeok",dalk,4);

    /* Original logical right-tail exact remains protected: 이었다. */
    press(CST_R_F_D,8,1900); press(CST_R_F_DOUBLE,32,1910); release(CST_R_F_DOUBLE,32,1920);
    test_time_advance(55); press(CST_R_F_NG,20,1990); release(CST_R_F_NG,20,2000);
    release(CST_R_F_D,8,2010);
    const uint32_t ieotda[]={D,L,D,J,LS(T),E,K}; expect("tail rolling ieotda",ieotda,7);

    press(CST_R_I_G,4,2100); cornix_steno_engine_set_active(false);
    release(CST_R_I_G,4,2120); expect("mode exit cancel",NULL,0);

    if (failures) return EXIT_FAILURE;
    puts("Cornix STENO v2.2.2 audited engine tests: PASS");
    return EXIT_SUCCESS;
}

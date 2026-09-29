/* Actual STENO engine + decoder host test. NOT an ARM/ZMK firmware test. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <dt-bindings/zmk/keys.h>
#include <cornix_steno/engine.h>
#include <cornix_steno/roles.h>
extern uint32_t test_output_keys[];
extern size_t test_output_count;
void test_output_reset(void);
struct test_key_event { uint32_t key; bool state; int64_t ts; };
extern struct test_key_event test_key_events[];
extern size_t test_key_event_count;
void test_key_events_reset(void);
static int64_t clock_ms = 1000;
static void reset(void) {
    cornix_steno_engine_set_active(false);
    cornix_steno_engine_set_active(true);
    test_output_reset(); test_key_events_reset();
    clock_ms += 10000; test_time_set(clock_ms);
}
int main(void) {
    const struct { enum cornix_steno_role role; uint32_t pos, key; } cases[] = {
        {CST_R_ABBR_L,12,TILDE}, {CST_R_VEXT_R,37,RBKT},
        {CST_R_SYMBOL_L,40,LC(Z)}, {CST_R_SYMBOL_R,47,LC(Y)},
    };
    const int durations[] = {1,49,199,200,499,500,750,1500};
    size_t n = 0;
    for (size_t i=0;i<sizeof(cases)/sizeof(cases[0]);i++) {
        for (size_t j=0;j<sizeof(durations)/sizeof(durations[0]);j++) {
            reset();
            assert(cornix_steno_engine_role_pressed(cases[i].role,cases[i].pos,clock_ms)>=0);
            assert(test_output_count==0); /* role anchors must not fire on press */
            test_time_advance(durations[j]); clock_ms+=durations[j];
            assert(cornix_steno_engine_role_released(cases[i].role,cases[i].pos,clock_ms)>=0);
            assert(test_output_count==1 && test_output_keys[0]==cases[i].key);
            assert(test_key_event_count==0); n++;
        }
    }
    /* Number and Shift-selection anchors must suppress the new solo shortcuts. */
    for (unsigned right=0;right<2;right++) {
        reset();
        enum cornix_steno_role anchor=right?CST_R_SYMBOL_R:CST_R_SYMBOL_L;
        uint32_t pos=right?47:40;
        assert(cornix_steno_engine_role_pressed(anchor,pos,clock_ms)>=0);
        clock_ms+=10;test_time_set(clock_ms);
        assert(cornix_steno_engine_role_pressed(CST_R_I_B,1,clock_ms)>=0);
        clock_ms+=10;test_time_set(clock_ms);
        assert(cornix_steno_engine_role_released(CST_R_I_B,1,clock_ms)>=0);
        assert(test_output_count==0 && test_key_event_count==2);
        assert(test_key_events[0].key==N1 && test_key_events[0].state);
        assert(test_key_events[1].key==N1 && !test_key_events[1].state);
        clock_ms+=10;test_time_set(clock_ms);
        assert(cornix_steno_engine_role_released(anchor,pos,clock_ms)>=0);
        assert(test_output_count==0 && test_key_event_count==2); n++;
    }
    printf("PASS: actual STENO engine requested solo outputs / duration boundaries / number-anchor suppression (%zu cases)\n",n);
    return 0;
}

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <cornix_steno/engine.h>
#include <dt-bindings/zmk/keys.h>

bool test_engine_down;
size_t test_engine_press_count;
size_t test_engine_release_count;
size_t test_engine_cancel_count;
enum cornix_steno_role test_engine_last_role;
uint32_t test_engine_last_position;

void test_engine_capture_reset(void) {
    test_engine_down = false;
    test_engine_press_count = test_engine_release_count = test_engine_cancel_count = 0;
    test_engine_last_role = CST_R_NONE;
    test_engine_last_position = 0;
}
void cornix_steno_engine_set_active(bool active) {(void)active;}
bool cornix_steno_engine_is_active(void) {return true;}
bool cornix_steno_engine_has_down_keys(void) {return test_engine_down;}
void cornix_steno_engine_cancel_pending(void) {test_engine_cancel_count++; test_engine_down=false;}
int cornix_steno_engine_role_pressed(enum cornix_steno_role role, uint32_t position, int64_t ts) {
    (void)ts; test_engine_down=true; test_engine_press_count++; test_engine_last_role=role; test_engine_last_position=position; return 0;
}
int cornix_steno_engine_role_released(enum cornix_steno_role role, uint32_t position, int64_t ts) {
    (void)role;(void)position;(void)ts; test_engine_down=false; test_engine_release_count++; return 0;
}

int cornix_steno_engine_emit_direct_role(enum cornix_steno_role role) {
    extern int cornix_steno_output_enqueue(uint32_t key);
    switch (role) {
    case CST_R_V_U: return cornix_steno_output_enqueue(N);
    case CST_R_V_A: return cornix_steno_output_enqueue(K);
    default: return 0;
    }
}

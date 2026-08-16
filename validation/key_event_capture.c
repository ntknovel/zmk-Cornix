#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
struct test_key_event { uint32_t key; bool state; int64_t ts; };
struct test_key_event test_key_events[256];
size_t test_key_event_count;
void test_key_events_reset(void) {test_key_event_count=0;}
int test_raise_keycode(uint32_t key, bool state, int64_t ts) {
    if (test_key_event_count < 256) test_key_events[test_key_event_count++] = (struct test_key_event){key,state,ts};
    return 0;
}

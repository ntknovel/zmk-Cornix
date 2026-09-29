#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <cornix_steno/output.h>

uint32_t test_output_keys[2048];
size_t test_output_count;

void test_output_reset(void) { test_output_count = 0; }
int cornix_steno_output_enqueue(uint32_t key) {
    return cornix_steno_output_enqueue_sequence(&key, 1);
}
int cornix_steno_output_enqueue_sequence(const uint32_t *keys, size_t count) {
    if (test_output_count + count > 2048) return -ENOSPC;
    for (size_t i = 0; i < count; i++) test_output_keys[test_output_count++] = keys[i];
    return 0;
}
void cornix_steno_output_flush(void) { test_output_count = 0; }
size_t cornix_steno_output_pending(void) { return test_output_count; }

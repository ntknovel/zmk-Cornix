#include <stddef.h>
#include <stdint.h>
#include <cornix_steno/quick.h>

size_t test_quick_invoke_count;
enum cornix_steno_quick_slot test_quick_last_slot = CST_QUICK_COUNT;

void test_quick_invoke_reset(void) {
    test_quick_invoke_count = 0;
    test_quick_last_slot = CST_QUICK_COUNT;
}

int cornix_steno_quick_invoke(const struct cornix_steno_quick_entry *entry,
                              int64_t timestamp) {
    (void)timestamp;
    if (!entry) return -1;
    test_quick_invoke_count++;
    test_quick_last_slot = entry->macro_slot;
    return 0;
}

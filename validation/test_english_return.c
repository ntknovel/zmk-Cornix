#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <cornix_steno/english_return.h>

static int failures;

static void expect_bool(const char *name, bool actual, bool expected) {
    if (actual != expected) {
        fprintf(stderr, "FAIL %s actual=%d expected=%d\n", name, actual, expected);
        failures++;
    }
}

int main(void) {
    expect_bool("initial pending", cornix_steno_english_return_pending(), false);

    cornix_steno_english_return_set_pending(true);
    expect_bool("set pending", cornix_steno_english_return_pending(), true);
    expect_bool("take pending", cornix_steno_english_return_take_pending(), true);
    expect_bool("take clears", cornix_steno_english_return_pending(), false);
    expect_bool("second take empty", cornix_steno_english_return_take_pending(), false);

    cornix_steno_english_return_set_pending(true);
    cornix_steno_english_return_set_pending(false);
    expect_bool("explicit clear", cornix_steno_english_return_pending(), false);

    if (failures) return EXIT_FAILURE;
    puts("Cornix STENO English-return runtime-state tests: PASS");
    return EXIT_SUCCESS;
}

/* Host tests of the actual metadata helper, not a firmware/link test. */
#include <assert.h>
#include <stdio.h>
#include <zephyr/device.h>
#include <cornix_steno/studio_metadata.h>
int main(void) {
    struct behavior_parameter_metadata md = {0};
    assert(cornix_steno_studio_hid_metadata(NULL, NULL) == -EINVAL);
    assert(cornix_steno_studio_led_metadata(NULL, NULL) == -EINVAL);
    assert(cornix_steno_studio_hid_metadata(NULL, &md) == 0);
    assert(md.sets_len == 1 && md.sets != NULL);
    assert(md.sets[0].param1_values_len == 1 && md.sets[0].param2_values_len == 0);
    assert(md.sets[0].param1_values[0].type == BEHAVIOR_PARAMETER_VALUE_TYPE_HID_USAGE);
    assert(cornix_steno_studio_led_metadata(NULL, &md) == 0);
    assert(md.sets_len == 1 && md.sets[0].param1_values_len == 1);
    assert(md.sets[0].param2_values_len == 0);
    assert(md.sets[0].param1_values[0].type == BEHAVIOR_PARAMETER_VALUE_TYPE_RANGE);
    assert(md.sets[0].param1_values[0].range.min == 0);
    assert(md.sets[0].param1_values[0].range.max == 255);
    puts("PASS: actual HID/LED metadata helper; parameter types, ranges, zero second parameter, NULL rejection");
    return 0;
}

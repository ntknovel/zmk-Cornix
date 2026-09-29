#pragma once
#include <zmk/behavior.h>
enum behavior_locality { BEHAVIOR_LOCALITY_CENTRAL, BEHAVIOR_LOCALITY_EVENT_SOURCE, BEHAVIOR_LOCALITY_GLOBAL };
struct behavior_parameter_value_metadata {
 char *display_name;
 union { uint32_t value; struct { int32_t min; int32_t max; } range; };
 enum { BEHAVIOR_PARAMETER_VALUE_TYPE_NIL=0, BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE=1,
        BEHAVIOR_PARAMETER_VALUE_TYPE_RANGE=2, BEHAVIOR_PARAMETER_VALUE_TYPE_HID_USAGE=3,
        BEHAVIOR_PARAMETER_VALUE_TYPE_LAYER_ID=4 } type;
};
struct behavior_parameter_metadata_set {
 size_t param1_values_len;
 const struct behavior_parameter_value_metadata *param1_values;
 size_t param2_values_len;
 const struct behavior_parameter_value_metadata *param2_values;
};
struct behavior_parameter_metadata {
 size_t sets_len;
 const struct behavior_parameter_metadata_set *sets;
};
struct behavior_driver_api {
 enum behavior_locality locality;
 int (*binding_convert_central_state_dependent_params)(struct zmk_behavior_binding *, struct zmk_behavior_binding_event);
 int (*binding_pressed)(struct zmk_behavior_binding *, struct zmk_behavior_binding_event);
 int (*binding_released)(struct zmk_behavior_binding *, struct zmk_behavior_binding_event);
 int (*get_parameter_metadata)(const struct device *, struct behavior_parameter_metadata *);
};
static inline int zmk_behavior_get_empty_param_metadata(const struct device *d, struct behavior_parameter_metadata *m) {(void)d;(void)m;return 0;}
#define BEHAVIOR_DT_INST_DEFINE(...)

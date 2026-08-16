#pragma once
#include <zmk/behavior.h>
enum behavior_locality { BEHAVIOR_LOCALITY_CENTRAL, BEHAVIOR_LOCALITY_EVENT_SOURCE, BEHAVIOR_LOCALITY_GLOBAL };
struct behavior_parameter_metadata { int dummy; };
struct behavior_driver_api {
 enum behavior_locality locality;
 int (*binding_convert_central_state_dependent_params)(struct zmk_behavior_binding *, struct zmk_behavior_binding_event);
 int (*binding_pressed)(struct zmk_behavior_binding *, struct zmk_behavior_binding_event);
 int (*binding_released)(struct zmk_behavior_binding *, struct zmk_behavior_binding_event);
 int (*get_parameter_metadata)(const struct device *, struct behavior_parameter_metadata *);
};
static inline int zmk_behavior_get_empty_param_metadata(const struct device *d, struct behavior_parameter_metadata *m) {(void)d;(void)m;return 0;}
#define BEHAVIOR_DT_INST_DEFINE(...)

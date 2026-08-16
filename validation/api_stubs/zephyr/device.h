#pragma once
#include <stdbool.h>
struct device { const void *config; const void *api; };
static inline bool device_is_ready(const struct device *d) {(void)d; return true;}
#define DEVICE_DT_GET(node) ((const struct device *)0x1)
#define DEVICE_DT_NAME(node) "behavior"

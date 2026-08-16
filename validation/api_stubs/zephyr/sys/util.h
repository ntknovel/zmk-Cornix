#pragma once
#include <stddef.h>
#include <stdint.h>
#define ARG_UNUSED(x) ((void)(x))
#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))
#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))
#define BIT(n) (1UL << (n))
#define BUILD_ASSERT(cond, msg) _Static_assert(cond, msg)
#define CONTAINER_OF(ptr, type, member) ((type *)((char *)(ptr) - offsetof(type, member)))

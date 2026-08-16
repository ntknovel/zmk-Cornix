#pragma once
typedef int atomic_t;
#define ATOMIC_INIT(x) (x)
static inline int atomic_get(const atomic_t *a) {return *a;}
static inline void atomic_set(atomic_t *a, int v) {*a=v;}

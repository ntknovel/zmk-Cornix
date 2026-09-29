#pragma once
#include <stdbool.h>
typedef int atomic_t;
#define ATOMIC_INIT(x) (x)
static inline int atomic_set(atomic_t *v, int n) { int old=*v;*v=n;return old; }
static inline bool atomic_cas(atomic_t *v,int old,int n) { if(*v!=old)return false;*v=n;return true; }

#pragma once
#include <stdint.h>
struct k_work { int unused; };
struct k_work_delayable { struct k_work work; void (*handler)(struct k_work *); int pending; int delay; };
#define K_MSEC(x) (x)
#define K_WORK_DELAYABLE_DEFINE(n, h) static struct k_work_delayable n = {.handler = h}
int k_work_reschedule(struct k_work_delayable *, int);
int k_work_cancel_delayable(struct k_work_delayable *);

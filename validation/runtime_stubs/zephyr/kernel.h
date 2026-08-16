#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
struct k_mutex { int dummy; };
struct k_work { int dummy; };
struct k_work_delayable {
    struct k_work work;
    void (*handler)(struct k_work *);
    bool scheduled;
    int64_t due;
};
typedef int64_t k_timeout_t;
#define K_MUTEX_DEFINE(name) struct k_mutex name
#define K_FOREVER 0
#define K_NO_WAIT 0
#define K_MSEC(x) ((k_timeout_t)(x))
void k_mutex_lock(struct k_mutex *m, int t);
void k_mutex_unlock(struct k_mutex *m);
void k_work_init_delayable(struct k_work_delayable *w, void (*h)(struct k_work *));
int k_work_cancel_delayable(struct k_work_delayable *w);
int k_work_reschedule(struct k_work_delayable *w, k_timeout_t t);
static inline struct k_work_delayable *k_work_delayable_from_work(struct k_work *w) { return (struct k_work_delayable *)w; }
int64_t k_uptime_get(void);
void test_time_set(int64_t value);
void test_time_advance(int64_t delta);

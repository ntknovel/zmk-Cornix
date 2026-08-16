#pragma once
#include <stdint.h>
#include <stddef.h>
struct k_mutex { int dummy; };
struct k_work { int dummy; };
struct k_work_delayable { struct k_work work; void (*handler)(struct k_work *); };
typedef int64_t k_timeout_t;
#define K_MUTEX_DEFINE(name) struct k_mutex name
#define K_FOREVER 0
#define K_NO_WAIT 0
#define K_MSEC(x) ((k_timeout_t)(x))
static inline void k_mutex_lock(struct k_mutex *m, int t) {(void)m;(void)t;}
static inline void k_mutex_unlock(struct k_mutex *m) {(void)m;}
static inline void k_work_init_delayable(struct k_work_delayable *w, void (*h)(struct k_work *)) {w->handler=h;}
static inline int k_work_cancel_delayable(struct k_work_delayable *w) {(void)w;return 0;}
static inline int k_work_reschedule(struct k_work_delayable *w, k_timeout_t t) {(void)w;(void)t;return 0;}
static inline struct k_work_delayable *k_work_delayable_from_work(struct k_work *w) {return (struct k_work_delayable *)w;}
static inline int64_t k_uptime_get(void) {return 0;}
static inline void k_msleep(int32_t ms) {(void)ms;}
static inline void k_busy_wait(uint32_t us) {(void)us;}

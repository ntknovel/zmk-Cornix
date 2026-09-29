#include <stddef.h>
#include <stdint.h>
#include <zephyr/kernel.h>

static int64_t now_ms;
static struct k_work_delayable *works[32];
static size_t work_count;

void k_mutex_lock(struct k_mutex *m, int t) {(void)m;(void)t;}
void k_mutex_unlock(struct k_mutex *m) {(void)m;}

void k_work_init_delayable(struct k_work_delayable *w, void (*h)(struct k_work *)) {
    w->handler = h;
    w->scheduled = false;
    w->due = 0;
    for (size_t i = 0; i < work_count; i++) if (works[i] == w) return;
    works[work_count++] = w;
}
int k_work_cancel_delayable(struct k_work_delayable *w) { w->scheduled = false; return 0; }
int k_work_reschedule(struct k_work_delayable *w, k_timeout_t t) {
    w->scheduled = true;
    w->due = now_ms + t;
    return 0;
}
int64_t k_uptime_get(void) { return now_ms; }
void test_time_set(int64_t value) { now_ms = value; }
void test_time_advance(int64_t delta) {
    now_ms += delta;
    for (;;) {
        struct k_work_delayable *due = NULL;
        for (size_t i = 0; i < work_count; i++) {
            if (works[i]->scheduled && works[i]->due <= now_ms) {
                due = works[i];
                break;
            }
        }
        if (!due) break;
        due->scheduled = false;
        due->handler(&due->work);
    }
}

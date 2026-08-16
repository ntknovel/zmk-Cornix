#pragma once
#define SYS_INIT(fn, level, prio) \
    static void __attribute__((constructor)) _host_init_##fn(void) { (void)fn(); }

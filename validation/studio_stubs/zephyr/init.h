#pragma once
#define APPLICATION 0
#define SYS_INIT(fn, level, priority) static int (*const mock_init_fn)(void) = fn

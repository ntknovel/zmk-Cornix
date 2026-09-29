#pragma once
#define LOG_MODULE_REGISTER(...) enum { _log_module_stub = 1 }
#define LOG_WRN(...) do { } while (0)
#define LOG_INF(...) do { } while (0)
#define LOG_DBG(...) do { } while (0)
#define LOG_ERR(...) do { } while (0)

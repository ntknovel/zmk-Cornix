#pragma once
#define SETTINGS_STATIC_HANDLER_DEFINE(n, path, get, set, commit, export) static int (*const mock_settings_commit)(void) = commit

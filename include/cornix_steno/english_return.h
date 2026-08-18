/* SPDX-License-Identifier: MIT */
#pragma once
#include <stdbool.h>

bool cornix_steno_english_return_pending(void);
void cornix_steno_english_return_set_pending(bool pending);
bool cornix_steno_english_return_take_pending(void);

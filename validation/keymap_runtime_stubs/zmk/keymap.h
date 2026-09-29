#pragma once
#include <stdint.h>
#include <stdbool.h>
bool zmk_keymap_layer_active(uint8_t layer);
int zmk_keymap_layer_activate(uint8_t layer, bool locking);
int zmk_keymap_layer_deactivate(uint8_t layer, bool locking);

#pragma once
#include <stdbool.h>
#include <stdint.h>
static inline bool zmk_keymap_layer_active(uint8_t layer) {(void)layer;return false;}
static inline int zmk_keymap_layer_activate(uint8_t layer, bool locking) {(void)layer;(void)locking;return 0;}
static inline int zmk_keymap_layer_deactivate(uint8_t layer, bool locking) {(void)layer;(void)locking;return 0;}

#pragma once
#define MOD_LCTL 0x01
#define MOD_LSFT 0x02
#define MOD_LALT 0x04
#define MOD_LGUI 0x08
#define MOD_RCTL 0x10
#define MOD_RSFT 0x20
#define MOD_RALT 0x40
#define MOD_RGUI 0x80
#define SELECT_MODS(keycode) ((keycode) >> 24)
#define STRIP_MODS(keycode) ((keycode) & ~(0xFFu << 24))
#define APPLY_MODS(mods, keycode) (((uint32_t)(mods) << 24) | (uint32_t)(keycode))
#ifndef LC
#define LC(keycode) APPLY_MODS(MOD_LCTL, keycode)
#endif
#ifndef LS
#define LS(keycode) APPLY_MODS(MOD_LSFT, keycode)
#endif
#define LA(keycode) APPLY_MODS(MOD_LALT, keycode)

/* Minimal host-test key definitions. Not shipped as a ZMK replacement. */
#pragma once
#include <stdint.h>
#define A 4u
#define B 5u
#define C 6u
#define D 7u
#define E 8u
#define F 9u
#define G 10u
#define H 11u
#define I 12u
#define J 13u
#define K 14u
#define L 15u
#define M 16u
#define N 17u
#define O 18u
#define P 19u
#define Q 20u
#define R 21u
#define S 22u
#define T 23u
#define U 24u
#define V 25u
#define W 26u
#define X 27u
#define Y 28u
#define Z 29u
#define N1 30u
#define N2 31u
#define N3 32u
#define N4 33u
#define N5 34u
#define N6 35u
#define N7 36u
#define N8 37u
#define N9 38u
#define N0 39u
#define ENTER 40u
#define ESC 41u
#define BSPC 42u
#define TAB 43u
#define SPACE 44u
#define MINUS 45u
#define EQUAL 46u
#define LBKT 47u
#define RBKT 48u
#define BSLH 49u
#define SEMI 51u
#define SQT 52u
#define GRAVE 53u
#define COMMA 54u
#define DOT 55u
#define FSLH 56u
#define EXCL 0x1001u
#define QUESTION 0x1002u
#define PRCNT 0x1003u
#define CARET 0x1004u
#define DQT 0x1005u

#define TILDE 0x1006u
#define HASH 0x1007u
#define LEFT 0x1008u
#define RIGHT 0x1009u
#define UP 0x100Au
#define DOWN 0x100Bu
#define LS(k) (0x01000000u | (uint32_t)(k))
#define LC(k) (0x02000000u | (uint32_t)(k))

#define LCTRL 0xE0u
#define LSHIFT 0xE1u
#define LALT 0xE2u
#define LGUI 0xE3u
#define RCTRL 0xE4u
#define RSHIFT 0xE5u
#define RALT 0xE6u
#define RGUI 0xE7u

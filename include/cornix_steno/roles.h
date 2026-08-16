/* SPDX-License-Identifier: MIT */
#pragma once

#include <stdbool.h>
#include <stdint.h>

enum cornix_steno_role {
    CST_R_NONE = 0,
    CST_R_I_KH = 1,
    CST_R_I_B = 2,
    CST_R_I_J = 3,
    CST_R_I_H = 4,
    CST_R_I_N = 5,
    CST_R_I_D = 6,
    CST_R_I_S = 7,
    CST_R_I_G = 8,
    CST_R_I_DOUBLE = 9,
    CST_R_I_NG = 10,
    CST_R_I_P = 11,
    CST_R_I_CH = 12,
    CST_R_I_T = 13,
    CST_R_I_M = 14,
    CST_R_I_R = 15,
    CST_R_F_NG = 16,
    CST_R_F_D = 17,
    CST_R_F_B = 18,
    CST_R_F_M = 19,
    CST_R_F_T = 20,
    CST_R_F_R = 21,
    CST_R_F_DOUBLE = 22,
    CST_R_F_G = 23,
    CST_R_F_S = 24,
    CST_R_F_P = 25,
    CST_R_F_N = 26,
    CST_R_F_KH = 27,
    CST_R_F_H = 28,
    CST_R_F_J = 29,
    CST_R_F_CH = 30,
    CST_R_V_O = 31,
    CST_R_V_EU = 32,
    CST_R_V_U = 33,
    CST_R_V_A = 34,
    CST_R_V_I = 35,
    CST_R_V_EO = 36,
    CST_R_VEXT_L = 37,
    CST_R_VEXT_R = 38,
    CST_R_ABBR_L = 39,
    CST_R_ABBR_R = 40,
    CST_R_SYMBOL_L = 41,
    CST_R_SYMBOL_R = 42,
    CST_R_EDIT = 43,
    CST_R_COUNT = 44,
};

#define CST_ROLE_BIT(role) (UINT64_C(1) << (role))
#define CST_POSITION_BIT(position) (UINT64_C(1) << (position))

#define CST_INITIAL_MASK                                                                          \
    (CST_ROLE_BIT(CST_R_I_KH) | CST_ROLE_BIT(CST_R_I_B) | CST_ROLE_BIT(CST_R_I_J) |              \
     CST_ROLE_BIT(CST_R_I_H) | CST_ROLE_BIT(CST_R_I_N) | CST_ROLE_BIT(CST_R_I_D) |                \
     CST_ROLE_BIT(CST_R_I_S) | CST_ROLE_BIT(CST_R_I_G) | CST_ROLE_BIT(CST_R_I_DOUBLE) |           \
     CST_ROLE_BIT(CST_R_I_NG) | CST_ROLE_BIT(CST_R_I_P) | CST_ROLE_BIT(CST_R_I_CH) |              \
     CST_ROLE_BIT(CST_R_I_T) | CST_ROLE_BIT(CST_R_I_M) | CST_ROLE_BIT(CST_R_I_R))

#define CST_FINAL_MASK                                                                            \
    (CST_ROLE_BIT(CST_R_F_NG) | CST_ROLE_BIT(CST_R_F_D) | CST_ROLE_BIT(CST_R_F_B) |               \
     CST_ROLE_BIT(CST_R_F_M) | CST_ROLE_BIT(CST_R_F_T) | CST_ROLE_BIT(CST_R_F_R) |                \
     CST_ROLE_BIT(CST_R_F_DOUBLE) | CST_ROLE_BIT(CST_R_F_G) | CST_ROLE_BIT(CST_R_F_S) |           \
     CST_ROLE_BIT(CST_R_F_P) | CST_ROLE_BIT(CST_R_F_N) | CST_ROLE_BIT(CST_R_F_KH) |               \
     CST_ROLE_BIT(CST_R_F_H) | CST_ROLE_BIT(CST_R_F_J) | CST_ROLE_BIT(CST_R_F_CH))

#define CST_BASE_VOWEL_MASK                                                                       \
    (CST_ROLE_BIT(CST_R_V_O) | CST_ROLE_BIT(CST_R_V_EU) | CST_ROLE_BIT(CST_R_V_U) |               \
     CST_ROLE_BIT(CST_R_V_A) | CST_ROLE_BIT(CST_R_V_I) | CST_ROLE_BIT(CST_R_V_EO))
#define CST_VEXT_MASK (CST_ROLE_BIT(CST_R_VEXT_L) | CST_ROLE_BIT(CST_R_VEXT_R))
#define CST_ABBR_MASK (CST_ROLE_BIT(CST_R_ABBR_L) | CST_ROLE_BIT(CST_R_ABBR_R))
#define CST_SYMBOL_MASK (CST_ROLE_BIT(CST_R_SYMBOL_L) | CST_ROLE_BIT(CST_R_SYMBOL_R))
#define CST_MARKER_MASK (CST_VEXT_MASK | CST_ABBR_MASK | CST_SYMBOL_MASK)

enum cornix_steno_dictionary_bank {
    CST_DICT_NONE = 0,
    CST_DICT_ABBR = 1,
    CST_DICT_VEXT = 2,
};

enum cornix_steno_dual_policy {
    CST_DUAL_VOWEL = 0,
    CST_DUAL_EDIT = 1,
    CST_DUAL_MOD_TAP = 2,
};

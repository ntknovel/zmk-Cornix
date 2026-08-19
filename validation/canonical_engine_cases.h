/* Auto-generated canonical engine cases; do not edit. */
#pragma once

#define CST_CANONICAL_MAX_KEYS 16
#define CST_CANONICAL_MAX_ROLES 8

struct cst_canonical_case { const char *id; const char *output; enum cornix_steno_role roles[CST_CANONICAL_MAX_ROLES]; uint8_t role_count; uint32_t keys[CST_CANONICAL_MAX_KEYS]; uint8_t key_count; };

static const struct cst_canonical_case cst_canonical_cases[] = {
    {"BASIC_JEOMJEOM", "점점", {CST_R_I_J, CST_R_F_J}, 2, {W, J, A, W, J, A}, 6},
    {"BASIC_JINJJA", "진짜", {CST_R_I_J, CST_R_F_J, CST_R_F_DOUBLE}, 3, {W, L, S, LS(W), K}, 5},
    {"BASIC_GYEOLGUK", "결국", {CST_R_I_G, CST_R_F_G}, 2, {R, U, F, R, N, R}, 6},
    {"BASIC_GEUREOKE", "그렇게", {CST_R_I_G, CST_R_I_R, CST_R_F_G}, 3, {R, M, F, J, G, R, P}, 7},
    {"BASIC_GEUREOTGUN", "그렇군", {CST_R_I_G, CST_R_F_R, CST_R_F_G}, 3, {R, M, F, J, G, R, N, S}, 8},
    {"BASIC_GEUREON", "그런", {CST_R_I_G, CST_R_F_R}, 2, {R, M, F, J, S}, 5},
    {"BASIC_GEUREOM", "그럼", {CST_R_ABBR_L, CST_R_I_G, CST_R_F_R}, 3, {R, M, F, J, A}, 5},
    {"BASIC_GEURAEDO", "그래도", {CST_R_I_G, CST_R_I_R, CST_R_F_D}, 3, {R, M, F, O, E, H}, 6},
    {"BASIC_GEUREONDE", "그런데", {CST_R_ABBR_L, CST_R_I_G, CST_R_I_R, CST_R_F_D}, 4, {R, M, F, J, S, E, P}, 7},
    {"BASIC_GEUREOMEDO", "그럼에도", {CST_R_I_G, CST_R_I_R, CST_R_I_NG, CST_R_F_D}, 4, {R, M, F, J, A, D, P, E, H}, 9},
    {"BASIC_GEUDERO", "그대로", {CST_R_I_G, CST_R_I_D, CST_R_F_R}, 3, {R, M, E, O, F, H}, 6},
    {"BASIC_GAPJAGI", "갑자기", {CST_R_I_G, CST_R_F_J, CST_R_I_DOUBLE}, 3, {R, K, Q, W, K, R, L}, 7},
    {"BASIC_WANJEONHI", "완전히", {CST_R_I_NG, CST_R_I_J, CST_R_F_H}, 3, {D, H, K, S, W, J, S, G, L}, 9},
    {"BASIC_JOYONGHI", "조용히", {CST_R_I_NG, CST_R_F_J, CST_R_F_H}, 3, {W, H, D, Y, D, G, L}, 7},
    {"BASIC_DOEEOT", "되었", {CST_R_I_D, CST_R_I_NG}, 2, {E, H, L, D, J, LS(T)}, 6},
    {"BASIC_GEUNDE", "근데", {CST_R_I_G, CST_R_F_D}, 2, {R, M, S, E, P}, 5},
    {"BASIC_GEUTTAE", "그때", {CST_R_I_G, CST_R_F_D, CST_R_F_DOUBLE}, 3, {R, M, LS(E), O}, 4},
    {"CONN_GEDAGA", "게다가", {CST_R_I_G, CST_R_I_D, CST_R_I_DOUBLE}, 3, {R, P, E, K, R, K}, 6},
    {"CONN_GEULIGO", "그리고", {CST_R_I_G, CST_R_I_R, CST_R_I_DOUBLE}, 3, {R, M, F, L, R, H}, 6},
    {"CONN_GEUREOJA", "그러자", {CST_R_I_G, CST_R_I_R, CST_R_F_J}, 3, {R, M, F, J, W, K}, 6},
    {"CONN_GEUREOGI", "그러기", {CST_R_ABBR_L, CST_R_I_G, CST_R_I_R, CST_R_I_DOUBLE}, 4, {R, M, F, J, R, L}, 6},
    {"CONN_GEUREOTJI", "그렇지", {CST_R_ABBR_L, CST_R_I_G, CST_R_I_R, CST_R_F_J}, 4, {R, M, F, J, G, W, L}, 7},
    {"CONN_HAJIMAN", "하지만", {CST_R_I_H, CST_R_I_J, CST_R_F_M}, 3, {G, K, W, L, A, K, S}, 7},
    {"TAIL_YEOTDA", "였다", {CST_R_F_NG, CST_R_F_D}, 2, {D, U, LS(T), E, K}, 5},
    {"TAIL_EOPDA", "없다", {CST_R_ABBR_R, CST_R_F_NG, CST_R_F_D}, 3, {D, J, Q, T, E, K}, 6},
    {"TAIL_EOTDA", "었다", {CST_R_ABBR_L, CST_R_F_NG, CST_R_F_D}, 3, {D, J, LS(T), E, K}, 5},
    {"TAIL_ATDA", "았다", {CST_R_ABBR_L, CST_R_ABBR_R, CST_R_F_NG, CST_R_F_D}, 4, {D, K, LS(T), E, K}, 5},
    {"TAIL_IEOTDA", "이었다", {CST_R_F_DOUBLE, CST_R_F_NG, CST_R_F_D}, 3, {D, L, D, J, LS(T), E, K}, 7},
    {"TAIL_YEOSSEOTDA", "였었다", {CST_R_VEXT_R, CST_R_F_NG, CST_R_F_D}, 3, {D, U, LS(T), D, J, LS(T), E, K}, 8},
    {"TAIL_EOSSEOTDA", "었었다", {CST_R_VEXT_L, CST_R_ABBR_R, CST_R_F_NG, CST_R_F_D}, 4, {D, J, LS(T), D, J, LS(T), E, K}, 8},
    {"TAIL_ITDA", "있다", {CST_R_VEXT_L, CST_R_ABBR_L, CST_R_F_NG, CST_R_F_D}, 4, {D, L, LS(T), E, K}, 5},
    {"TAIL_EDA", "에다", {CST_R_ABBR_R, CST_R_I_NG, CST_R_F_D}, 3, {D, P, E, K}, 4},
    {"TAIL_HAETDA", "했다", {CST_R_F_H, CST_R_F_D}, 2, {G, O, LS(T), E, K}, 5},
    {"TAIL_DWAETDA", "됐다", {CST_R_F_DOUBLE, CST_R_F_D}, 2, {E, H, O, LS(T), E, K}, 6},
    {"TAIL_NEUNDE", "는데", {CST_R_VEXT_L, CST_R_F_N}, 2, {S, M, S, E, P}, 5},
    {"TAIL_EULJIDO", "을지도", {CST_R_F_NG, CST_R_F_J, CST_R_F_D}, 3, {D, M, F, W, L, E, H}, 7},
    {"TAIL_EOJYEOTDA", "어졌다", {CST_R_ABBR_R, CST_R_F_NG, CST_R_F_J, CST_R_F_D}, 4, {D, J, W, U, LS(T), E, K}, 7},
    {"TAIL_JIMAN", "지만", {CST_R_F_J, CST_R_F_M}, 2, {W, L, A, K, S}, 5},
    {"TAIL_SEUMNIDA", "습니다", {CST_R_VEXT_L, CST_R_F_S, CST_R_F_D}, 3, {T, M, Q, S, L, E, K}, 7},
    {"TAIL_EOSSEUMYEON", "었으면", {CST_R_VEXT_L, CST_R_ABBR_R, CST_R_F_M}, 3, {D, J, LS(T), D, M, A, U, S}, 8},
    {"TAIL_HANEUNGE", "하는게", {CST_R_F_H, CST_R_F_N, CST_R_F_G}, 3, {G, K, S, M, S, R, P}, 7},
    {"TAIL_ONEUNGE", "오는게", {CST_R_F_NG, CST_R_F_N, CST_R_F_G}, 3, {D, H, S, M, S, R, P}, 7},
    {"TAIL_GANEUNGE", "가는게", {CST_R_F_DOUBLE, CST_R_F_N, CST_R_F_G}, 3, {R, K, S, M, S, R, P}, 7},
};
#define CST_CANONICAL_CASE_COUNT 43u

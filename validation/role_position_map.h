/* Auto-generated from config/steno_role_layout.tsv. */
#pragma once

static inline uint32_t cst_test_position_for_role(enum cornix_steno_role role) {
    switch (role) {
    case CST_R_I_B: return 1u;
    case CST_R_I_J: return 2u;
    case CST_R_I_D: return 3u;
    case CST_R_I_G: return 4u;
    case CST_R_I_S: return 5u;
    case CST_R_F_S: return 6u;
    case CST_R_F_G: return 7u;
    case CST_R_F_D: return 8u;
    case CST_R_F_J: return 9u;
    case CST_R_F_B: return 10u;
    case CST_R_ABBR_L: return 12u;
    case CST_R_I_M: return 13u;
    case CST_R_I_N: return 14u;
    case CST_R_I_NG: return 15u;
    case CST_R_I_R: return 16u;
    case CST_R_I_H: return 17u;
    case CST_R_F_H: return 18u;
    case CST_R_F_R: return 19u;
    case CST_R_F_NG: return 20u;
    case CST_R_F_N: return 21u;
    case CST_R_F_M: return 22u;
    case CST_R_ABBR_R: return 23u;
    case CST_R_VEXT_L: return 24u;
    case CST_R_I_KH: return 25u;
    case CST_R_I_T: return 26u;
    case CST_R_I_CH: return 27u;
    case CST_R_I_P: return 28u;
    case CST_R_I_DOUBLE: return 29u;
    case CST_R_F_DOUBLE: return 32u;
    case CST_R_F_P: return 33u;
    case CST_R_F_CH: return 34u;
    case CST_R_F_T: return 35u;
    case CST_R_F_KH: return 36u;
    case CST_R_VEXT_R: return 37u;
    case CST_R_SYMBOL_L: return 40u;
    case CST_R_V_O: return 41u;
    case CST_R_V_EU: return 42u;
    case CST_R_V_U: return 43u;
    case CST_R_V_EO: return 44u;
    case CST_R_V_I: return 45u;
    case CST_R_V_A: return 46u;
    case CST_R_SYMBOL_R: return 47u;
    default: return UINT32_MAX;
    }
}

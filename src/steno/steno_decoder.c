/*
 * Cornix Korean STENO pure decoder.
 *
 * Logical-role and physical-position masks are converted into short two-set
 * Korean HID sequences. Timing, split transport and BLE output live elsewhere.
 * SPDX-License-Identifier: MIT
 */

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <dt-bindings/zmk/keys.h>
#include <cornix_steno/decoder.h>
#include <cornix_steno/roles.h>

#define BITR(role) CST_ROLE_BIT(role)

static unsigned bit_count(uint64_t value) {
    unsigned count = 0;
    while (value) {
        value &= value - 1;
        count++;
    }
    return count;
}

static int append_key(struct cornix_steno_decoded *decoded, uint32_t key) {
    if (decoded->key_count >= CST_DECODE_MAX_KEYS) return -ENOSPC;
    decoded->keys[decoded->key_count++] = key;
    return 0;
}

static int append_array(struct cornix_steno_decoded *decoded, const uint32_t *keys,
                        uint8_t count) {
    for (uint8_t i = 0; i < count; i++) {
        const int err = append_key(decoded, keys[i]);
        if (err) return err;
    }
    return 0;
}

static bool exact_position_pair(uint64_t position_mask, uint8_t a, uint8_t b) {
    return position_mask == (CST_POSITION_BIT(a) | CST_POSITION_BIT(b));
}

static int decode_number(uint64_t position_mask, struct cornix_steno_decoded *decoded) {
    static const struct { uint8_t top, home; uint32_t key; } numbers[] = {
        {1,13,N1}, {2,14,N2}, {3,15,N3}, {4,16,N4}, {5,17,N5},
        {6,18,N6}, {7,19,N7}, {8,20,N8}, {9,21,N9}, {10,22,N0},
    };
    for (size_t i = 0; i < sizeof(numbers)/sizeof(numbers[0]); i++) {
        if (exact_position_pair(position_mask, numbers[i].top, numbers[i].home)) {
            decoded->kind = CST_DECODE_KEYS;
            return append_key(decoded, numbers[i].key);
        }
    }
    return -ENOENT;
}

static int decode_initial(uint64_t role_mask, uint32_t keys[1], uint8_t *count) {
    const bool doubled = (role_mask & BITR(CST_R_I_DOUBLE)) != 0;
    const uint64_t base = role_mask & (CST_INITIAL_MASK & ~BITR(CST_R_I_DOUBLE));
    *count = 0;
    if (bit_count(base) != 1) return -EINVAL;

    enum cornix_steno_role role = CST_R_NONE;
    for (enum cornix_steno_role r = CST_R_I_KH; r <= CST_R_I_R; r++) {
        if (r != CST_R_I_DOUBLE && (base & BITR(r))) { role = r; break; }
    }
    if (doubled) {
        switch (role) {
        case CST_R_I_G: keys[0] = LS(R); break;
        case CST_R_I_D: keys[0] = LS(E); break;
        case CST_R_I_B: keys[0] = LS(Q); break;
        case CST_R_I_S: keys[0] = LS(T); break;
        case CST_R_I_J: keys[0] = LS(W); break;
        default: return -EINVAL;
        }
        *count = 1;
        return 0;
    }
    switch (role) {
    case CST_R_I_KH: keys[0]=Z; break; case CST_R_I_B: keys[0]=Q; break;
    case CST_R_I_J: keys[0]=W; break; case CST_R_I_H: keys[0]=G; break;
    case CST_R_I_N: keys[0]=S; break; case CST_R_I_D: keys[0]=E; break;
    case CST_R_I_S: keys[0]=T; break; case CST_R_I_G: keys[0]=R; break;
    case CST_R_I_NG: keys[0]=D; break; case CST_R_I_P: keys[0]=V; break;
    case CST_R_I_CH: keys[0]=C; break; case CST_R_I_T: keys[0]=X; break;
    case CST_R_I_M: keys[0]=A; break; case CST_R_I_R: keys[0]=F; break;
    default: return -EINVAL;
    }
    *count = 1;
    return 0;
}

static uint32_t final_single_key(enum cornix_steno_role role) {
    switch (role) {
    case CST_R_F_NG:return D; case CST_R_F_D:return E; case CST_R_F_B:return Q;
    case CST_R_F_M:return A; case CST_R_F_T:return X; case CST_R_F_R:return F;
    case CST_R_F_G:return R; case CST_R_F_S:return T; case CST_R_F_P:return V;
    case CST_R_F_N:return S; case CST_R_F_KH:return Z; case CST_R_F_H:return G;
    case CST_R_F_J:return W; case CST_R_F_CH:return C; default:return 0;
    }
}

static int decode_final(uint64_t role_mask, uint32_t keys[2], uint8_t *count) {
    const bool doubled = (role_mask & BITR(CST_R_F_DOUBLE)) != 0;
    const uint64_t base = role_mask & (CST_FINAL_MASK & ~BITR(CST_R_F_DOUBLE));
    const unsigned n = bit_count(base);
    *count = 0;
    if (doubled) {
        if (n != 1) return -EINVAL;
        if (base & BITR(CST_R_F_G)) keys[0] = LS(R);
        else if (base & BITR(CST_R_F_S)) keys[0] = LS(T);
        else return -EINVAL;
        *count = 1;
        return 0;
    }
    if (n == 1) {
        for (enum cornix_steno_role r=CST_R_F_NG; r<=CST_R_F_CH; r++) {
            if (r != CST_R_F_DOUBLE && (base & BITR(r))) {
                keys[0] = final_single_key(r);
                if (!keys[0]) return -EINVAL;
                *count = 1;
                return 0;
            }
        }
    }
    if (n != 2) return -EINVAL;
#define PAIR(a,b) (BITR(a)|BITR(b))
    switch (base) {
    case PAIR(CST_R_F_G,CST_R_F_S): keys[0]=R; keys[1]=T; break;
    case PAIR(CST_R_F_N,CST_R_F_J): keys[0]=S; keys[1]=W; break;
    case PAIR(CST_R_F_N,CST_R_F_H): keys[0]=S; keys[1]=G; break;
    case PAIR(CST_R_F_R,CST_R_F_G): keys[0]=F; keys[1]=R; break;
    case PAIR(CST_R_F_R,CST_R_F_M): keys[0]=F; keys[1]=A; break;
    case PAIR(CST_R_F_R,CST_R_F_B): keys[0]=F; keys[1]=Q; break;
    case PAIR(CST_R_F_R,CST_R_F_S): keys[0]=F; keys[1]=T; break;
    case PAIR(CST_R_F_R,CST_R_F_T): keys[0]=F; keys[1]=X; break;
    case PAIR(CST_R_F_R,CST_R_F_P): keys[0]=F; keys[1]=V; break;
    case PAIR(CST_R_F_R,CST_R_F_H): keys[0]=F; keys[1]=G; break;
    case PAIR(CST_R_F_B,CST_R_F_S): keys[0]=Q; keys[1]=T; break;
    default:return -EINVAL;
    }
#undef PAIR
    *count = 2;
    return 0;
}

static int decode_vowel(uint64_t role_mask, uint32_t keys[2], uint8_t *count) {
    const uint64_t base = role_mask & CST_BASE_VOWEL_MASK;
    const unsigned vext_count = bit_count(role_mask & CST_VEXT_MASK);
    *count = 0;
#define VB(role) BITR(role)
#define MATCH(mask) (base == (mask))
    if (vext_count == 0) {
        if (MATCH(VB(CST_R_V_A))) { keys[0]=K; *count=1; return 0; }
        if (MATCH(VB(CST_R_V_EO))) { keys[0]=J; *count=1; return 0; }
        if (MATCH(VB(CST_R_V_O))) { keys[0]=H; *count=1; return 0; }
        if (MATCH(VB(CST_R_V_U))) { keys[0]=N; *count=1; return 0; }
        if (MATCH(VB(CST_R_V_EU))) { keys[0]=M; *count=1; return 0; }
        if (MATCH(VB(CST_R_V_I))) { keys[0]=L; *count=1; return 0; }
        if (MATCH(VB(CST_R_V_A)|VB(CST_R_V_I))) { keys[0]=O; *count=1; return 0; }
        if (MATCH(VB(CST_R_V_I)|VB(CST_R_V_EO))) { keys[0]=P; *count=1; return 0; }
        if (MATCH(VB(CST_R_V_EU)|VB(CST_R_V_A))) { keys[0]=LS(O); *count=1; return 0; }
        if (MATCH(VB(CST_R_V_EU)|VB(CST_R_V_EO))) { keys[0]=LS(P); *count=1; return 0; }
        if (MATCH(VB(CST_R_V_O)|VB(CST_R_V_A))) { keys[0]=H; keys[1]=K; *count=2; return 0; }
        if (MATCH(VB(CST_R_V_O)|VB(CST_R_V_EO))) { keys[0]=H; keys[1]=O; *count=2; return 0; }
        if (MATCH(VB(CST_R_V_O)|VB(CST_R_V_I))) { keys[0]=H; keys[1]=L; *count=2; return 0; }
        if (MATCH(VB(CST_R_V_U)|VB(CST_R_V_EO))) { keys[0]=N; keys[1]=J; *count=2; return 0; }
        if (MATCH(VB(CST_R_V_U)|VB(CST_R_V_A))) { keys[0]=N; keys[1]=P; *count=2; return 0; }
        if (MATCH(VB(CST_R_V_U)|VB(CST_R_V_I))) { keys[0]=N; keys[1]=L; *count=2; return 0; }
        if (MATCH(VB(CST_R_V_EU)|VB(CST_R_V_I))) { keys[0]=M; keys[1]=L; *count=2; return 0; }
    } else if (vext_count == 1) {
        if (MATCH(VB(CST_R_V_A))) { keys[0]=I; *count=1; return 0; }
        if (MATCH(VB(CST_R_V_EO))) { keys[0]=U; *count=1; return 0; }
        if (MATCH(VB(CST_R_V_O))) { keys[0]=Y; *count=1; return 0; }
        if (MATCH(VB(CST_R_V_U))) { keys[0]=B; *count=1; return 0; }
        if (MATCH(VB(CST_R_V_I))) { keys[0]=O; *count=1; return 0; }
        if (MATCH(VB(CST_R_V_EU))) { keys[0]=P; *count=1; return 0; }
    } else if (vext_count == 2) {
        if (MATCH(VB(CST_R_V_I))) { keys[0]=LS(O); *count=1; return 0; }
        if (MATCH(VB(CST_R_V_EU))) { keys[0]=LS(P); *count=1; return 0; }
    }
#undef MATCH
#undef VB
    return -EINVAL;
}

static int exact_keys(struct cornix_steno_decoded *d, const uint32_t *keys, uint8_t n) {
    d->kind = CST_DECODE_KEYS;
    return append_array(d, keys, n);
}

int cornix_steno_decode(uint64_t role_mask, uint64_t position_mask,
                        struct cornix_steno_decoded *decoded) {
    if (!decoded) return -EINVAL;
    memset(decoded, 0, sizeof(*decoded));
    if (decode_number(position_mask, decoded) == 0) return 0;

#define EXACT1(mask,key) do { if (role_mask==(mask)) { decoded->kind=CST_DECODE_KEYS; return append_key(decoded,(key)); } } while (0)
    EXACT1(BITR(CST_R_I_DOUBLE), COMMA);
    EXACT1(BITR(CST_R_F_DOUBLE), DOT);
    EXACT1(BITR(CST_R_I_DOUBLE)|BITR(CST_R_F_DOUBLE), SPACE);
    EXACT1(BITR(CST_R_VEXT_L), LBKT);
    EXACT1(BITR(CST_R_VEXT_R), QUESTION);
    EXACT1(BITR(CST_R_ABBR_L), EXCL);
    EXACT1(BITR(CST_R_ABBR_R), DQT);
    EXACT1(BITR(CST_R_SYMBOL_L), RBKT);
    EXACT1(BITR(CST_R_SYMBOL_R), BSPC);
    EXACT1(BITR(CST_R_VEXT_L)|BITR(CST_R_ABBR_L), TILDE);
    EXACT1(BITR(CST_R_VEXT_R)|BITR(CST_R_ABBR_R), SQT);
    EXACT1(BITR(CST_R_SYMBOL_L)|BITR(CST_R_ABBR_L), PRCNT);
    EXACT1(BITR(CST_R_SYMBOL_R)|BITR(CST_R_ABBR_R), CARET);
    EXACT1(BITR(CST_R_VEXT_L)|BITR(CST_R_VEXT_R), ENTER);
    EXACT1(BITR(CST_R_ABBR_L)|BITR(CST_R_ABBR_R), HASH);
#undef EXACT1

    if (role_mask == (BITR(CST_R_VEXT_L)|BITR(CST_R_ABBR_R))) {
        const uint32_t keys[] = {DQT, DQT, LEFT};
        return exact_keys(decoded, keys, 3);
    }
    if (role_mask == (BITR(CST_R_ABBR_L)|BITR(CST_R_VEXT_R))) {
        const uint32_t keys[] = {QUESTION, EXCL};
        return exact_keys(decoded, keys, 2);
    }
    if (role_mask == (BITR(CST_R_SYMBOL_L)|BITR(CST_R_SYMBOL_R))) {
        const uint32_t keys[] = {HASH, HASH, HASH};
        return exact_keys(decoded, keys, 3);
    }

    /* Thumb-only editing commands. An ABBR marker makes these quick slots instead. */
    if (role_mask == (BITR(CST_R_V_O)|BITR(CST_R_V_U))) {
        decoded->kind=CST_DECODE_KEYS; return append_key(decoded, LC(Z));
    }
    if (role_mask == (BITR(CST_R_V_A)|BITR(CST_R_V_EO))) {
        decoded->kind=CST_DECODE_KEYS; return append_key(decoded, LC(Y));
    }
    if (role_mask == (BITR(CST_R_V_O)|BITR(CST_R_V_EU)|BITR(CST_R_V_U))) {
        decoded->kind=CST_DECODE_KEYS; return append_key(decoded, LC(C));
    }
    if (role_mask == (BITR(CST_R_V_A)|BITR(CST_R_V_I)|BITR(CST_R_V_EO))) {
        decoded->kind=CST_DECODE_KEYS; return append_key(decoded, LC(V));
    }

    const uint64_t initial_mask = role_mask & CST_INITIAL_MASK;
    const uint64_t final_mask = role_mask & CST_FINAL_MASK;
    const uint64_t vowel_mask = role_mask & (CST_BASE_VOWEL_MASK|CST_VEXT_MASK);
    const uint64_t symbol_mask = role_mask & CST_SYMBOL_MASK;
    const bool has_base_vowel = (role_mask & CST_BASE_VOWEL_MASK) != 0;

    /* Symbol + exactly one jamo group = direct correction output. */
    if (symbol_mask) {
        if (bit_count(symbol_mask) != 1 || (role_mask & CST_ABBR_MASK)) return 0;
        unsigned groups = (initial_mask ? 1U : 0U) + (final_mask ? 1U : 0U) +
                          (has_base_vowel ? 1U : 0U);
        if (groups != 1) return 0;
        uint32_t keys[2]={0}; uint8_t count=0;
        if (initial_mask) {
            if (vowel_mask || final_mask || decode_initial(initial_mask,keys,&count)) return 0;
            decoded->kind=CST_DECODE_KEYS;
            return append_array(decoded,keys,count);
        }
        if (final_mask) {
            const uint64_t base_final = final_mask & ~BITR(CST_R_F_DOUBLE);
            if (vowel_mask || initial_mask || (final_mask & BITR(CST_R_F_DOUBLE)) ||
                bit_count(base_final) != 1 || decode_final(final_mask,keys,&count)) return 0;
            decoded->kind=CST_DECODE_KEYS;
            return append_array(decoded,keys,count);
        }
        if (decode_vowel(vowel_mask,keys,&count)) return 0;
        decoded->kind=CST_DECODE_KEYS;
        return append_array(decoded,keys,count);
    }

    /* Abbreviation markers and consonant-only VEXT strokes are looked up by the engine. */
    if ((role_mask & CST_ABBR_MASK) || ((role_mask & CST_VEXT_MASK) && !has_base_vowel)) return 0;

    if (has_base_vowel) {
        uint32_t ik[1]={0}, vk[2]={0}, fk[2]={0};
        uint8_t ic=0, vc=0, fc=0;
        if (!initial_mask) return 0;
        if (decode_initial(initial_mask,ik,&ic) || decode_vowel(vowel_mask,vk,&vc)) return 0;
        if (final_mask && decode_final(final_mask,fk,&fc)) return 0;
        decoded->kind=CST_DECODE_KEYS;
        int err=append_array(decoded,ik,ic); if(err)return err;
        err=append_array(decoded,vk,vc); if(err)return err;
        return append_array(decoded,fk,fc);
    }

    /* Consonant-only strokes are reserved for exact-mask abbreviations. */
    return 0;
}

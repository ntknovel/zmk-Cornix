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
    /*
     * Dubeol number layout: the old vertical two-key number chords are retired.
     * Either symbol key mirrors the entire physical top row:
     *   SYMBOL-L or SYMBOL-R + Q/W/E/R/T/Y/U/I/O/P = 0..9
     * Physical positions are used deliberately so the number layout is stable
     * even when logical STENO consonant roles are remapped. Exactly one symbol
     * position and one top-row position must be present, so SYMBOL-L+SYMBOL-R
     * keeps its existing reserved meaning instead of becoming a number.
     */
    static const struct { uint8_t symbol, top; uint32_t key; } numbers[] = {
        {40,1,N0}, {40,2,N1}, {40,3,N2}, {40,4,N3}, {40,5,N4},
        {40,6,N5}, {40,7,N6}, {40,8,N7}, {40,9,N8}, {40,10,N9},
        {47,1,N0}, {47,2,N1}, {47,3,N2}, {47,4,N3}, {47,5,N4},
        {47,6,N5}, {47,7,N6}, {47,8,N7}, {47,9,N8}, {47,10,N9},
    };
    for (size_t i = 0; i < sizeof(numbers)/sizeof(numbers[0]); i++) {
        if (exact_position_pair(position_mask, numbers[i].symbol, numbers[i].top)) {
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


static int consonant_identity_for_role(enum cornix_steno_role role) {
    switch (role) {
    case CST_R_I_G: case CST_R_F_G: return 1;   /* ㄱ */
    case CST_R_I_N: case CST_R_F_N: return 2;   /* ㄴ */
    case CST_R_I_D: case CST_R_F_D: return 3;   /* ㄷ */
    case CST_R_I_R: case CST_R_F_R: return 4;   /* ㄹ */
    case CST_R_I_M: case CST_R_F_M: return 5;   /* ㅁ */
    case CST_R_I_B: case CST_R_F_B: return 6;   /* ㅂ */
    case CST_R_I_S: case CST_R_F_S: return 7;   /* ㅅ */
    case CST_R_I_NG: case CST_R_F_NG: return 8; /* ㅇ */
    case CST_R_I_J: case CST_R_F_J: return 9;   /* ㅈ */
    case CST_R_I_CH: case CST_R_F_CH: return 10;/* ㅊ */
    case CST_R_I_KH: case CST_R_F_KH: return 11;/* ㅋ */
    case CST_R_I_T: case CST_R_F_T: return 12;  /* ㅌ */
    case CST_R_I_P: case CST_R_F_P: return 13;  /* ㅍ */
    case CST_R_I_H: case CST_R_F_H: return 14;  /* ㅎ */
    default: return 0;
    }
}

static uint32_t consonant_key_for_identity(int identity, bool doubled) {
    if (doubled) {
        switch (identity) {
        case 1: return LS(R); /* ㄲ */
        case 3: return LS(E); /* ㄸ */
        case 6: return LS(Q); /* ㅃ */
        case 7: return LS(T); /* ㅆ */
        case 9: return LS(W); /* ㅉ */
        default: return 0;
        }
    }
    switch (identity) {
    case 1: return R; case 2: return S; case 3: return E; case 4: return F;
    case 5: return A; case 6: return Q; case 7: return T; case 8: return D;
    case 9: return W; case 10: return C; case 11: return Z; case 12: return X;
    case 13: return V; case 14: return G; default: return 0;
    }
}

int cornix_steno_decode_correction_unit(uint64_t unit_role_mask,
                                        struct cornix_steno_decoded *decoded) {
    if (!decoded) return -EINVAL;
    memset(decoded, 0, sizeof(*decoded));

    if (unit_role_mask & (CST_ABBR_MASK | CST_SYMBOL_MASK)) return -EINVAL;

    const uint64_t consonants = unit_role_mask & (CST_INITIAL_MASK | CST_FINAL_MASK);
    const uint64_t vowels = unit_role_mask & (CST_BASE_VOWEL_MASK | CST_VEXT_MASK);
    const bool doubled = (unit_role_mask &
        (BITR(CST_R_I_DOUBLE) | BITR(CST_R_F_DOUBLE))) != 0;
    const uint64_t base_consonants = consonants &
        ~(BITR(CST_R_I_DOUBLE) | BITR(CST_R_F_DOUBLE));

    if (base_consonants) {
        if (vowels) return -EINVAL;
        int identity = 0;
        for (enum cornix_steno_role role = CST_R_I_KH; role <= CST_R_F_CH; role++) {
            if (role == CST_R_I_DOUBLE || role == CST_R_F_DOUBLE ||
                !(base_consonants & BITR(role))) continue;
            const int candidate = consonant_identity_for_role(role);
            if (!candidate) return -EINVAL;
            if (identity && identity != candidate) return -EINVAL;
            identity = candidate;
        }
        if (!identity) return -EINVAL;
        const uint32_t key = consonant_key_for_identity(identity, doubled);
        if (!key) return -EINVAL;
        decoded->kind = CST_DECODE_KEYS;
        return append_key(decoded, key);
    }

    if (doubled || !(unit_role_mask & CST_BASE_VOWEL_MASK)) return -EINVAL;
    uint32_t keys[2] = {0};
    uint8_t count = 0;
    if (decode_vowel(vowels, keys, &count)) return -EINVAL;
    decoded->kind = CST_DECODE_KEYS;
    return append_array(decoded, keys, count);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dt-bindings/zmk/keys.h>
#include <cornix_steno/decoder.h>
#include <cornix_steno/dictionary.h>
#include <cornix_steno/quick.h>
#include <cornix_steno/roles.h>
#define RBIT(r) CST_ROLE_BIT(r)
static int failures;

static void expect_keys(const char *name, uint64_t roles, uint64_t positions,
                        const uint32_t *expected, size_t count) {
    struct cornix_steno_decoded d;
    int err=cornix_steno_decode(roles,positions,&d);
    if (err || d.kind != (count ? CST_DECODE_KEYS : CST_DECODE_NONE) ||
        d.key_count != count || (count && memcmp(d.keys,expected,count*sizeof(expected[0])))) {
        fprintf(stderr,"FAIL %-28s err=%d kind=%d count=%u expected=%zu\n",
                name,err,d.kind,d.key_count,count); failures++;
    }
}
static void expect_silent_or_rejected(const char *name, uint64_t roles, uint64_t positions) {
    struct cornix_steno_decoded d;
    int err = cornix_steno_decode(roles, positions, &d);
    if (err >= 0 && (d.kind != CST_DECODE_NONE || d.key_count != 0)) {
        fprintf(stderr, "FAIL %s produced kind=%d count=%u\n", name, d.kind, d.key_count);
        failures++;
    }
}

static void expect_dictionary(const char *name, uint64_t mask,
                              const uint32_t *prefix, size_t prefix_count) {
    const struct cornix_steno_dictionary_entry *e=cornix_steno_dictionary_lookup(mask);
    if (!e || e->key_count < prefix_count || memcmp(e->keys,prefix,prefix_count*sizeof(prefix[0]))) {
        fprintf(stderr,"FAIL dictionary %s\n",name); failures++;
    }
}

int main(void) {
    /* All 21 modern vowels in an ㅇ-initial syllable. */
    struct vowel_case { const char *name; uint64_t roles; uint32_t keys[2]; uint8_t n; } v[] = {
        {"a",RBIT(CST_R_V_A),{K},1},
        {"ae",RBIT(CST_R_V_A)|RBIT(CST_R_V_I),{O},1},
        {"ya",RBIT(CST_R_VEXT_L)|RBIT(CST_R_V_A),{I},1},
        {"yae",RBIT(CST_R_V_EU)|RBIT(CST_R_V_A),{LS(O)},1},
        {"eo",RBIT(CST_R_V_EO),{J},1},
        {"e",RBIT(CST_R_V_I)|RBIT(CST_R_V_EO),{P},1},
        {"yeo",RBIT(CST_R_VEXT_R)|RBIT(CST_R_V_EO),{U},1},
        {"ye",RBIT(CST_R_V_EU)|RBIT(CST_R_V_EO),{LS(P)},1},
        {"o",RBIT(CST_R_V_O),{H},1},
        {"wa",RBIT(CST_R_V_O)|RBIT(CST_R_V_A),{H,K},2},
        {"wae",RBIT(CST_R_V_O)|RBIT(CST_R_V_EO),{H,O},2},
        {"oe",RBIT(CST_R_V_O)|RBIT(CST_R_V_I),{H,L},2},
        {"yo",RBIT(CST_R_VEXT_L)|RBIT(CST_R_V_O),{Y},1},
        {"u",RBIT(CST_R_V_U),{N},1},
        {"wo",RBIT(CST_R_V_U)|RBIT(CST_R_V_EO),{N,J},2},
        {"we",RBIT(CST_R_V_U)|RBIT(CST_R_V_A),{N,P},2},
        {"wi",RBIT(CST_R_V_U)|RBIT(CST_R_V_I),{N,L},2},
        {"yu",RBIT(CST_R_VEXT_R)|RBIT(CST_R_V_U),{B},1},
        {"eu",RBIT(CST_R_V_EU),{M},1},
        {"ui",RBIT(CST_R_V_EU)|RBIT(CST_R_V_I),{M,L},2},
        {"i",RBIT(CST_R_V_I),{L},1},
    };
    for (size_t i=0;i<sizeof(v)/sizeof(v[0]);i++) {
        uint32_t out[3]={D,v[i].keys[0],v[i].keys[1]};
        expect_keys(v[i].name,RBIT(CST_R_I_NG)|v[i].roles,0,out,1+v[i].n);
    }

    const uint32_t comma[]={COMMA}; expect_keys("double-I single",RBIT(CST_R_I_DOUBLE),0,comma,1);
    const uint32_t dot[]={DOT}; expect_keys("double-F single",RBIT(CST_R_F_DOUBLE),0,dot,1);
    const uint32_t space[]={SPACE}; expect_keys("both double",RBIT(CST_R_I_DOUBLE)|RBIT(CST_R_F_DOUBLE),0,space,1);
    const uint32_t ex[]={EXCL}; expect_keys("ABBR-L single",RBIT(CST_R_ABBR_L),0,ex,1);
    const uint32_t quote[]={DQT}; expect_keys("ABBR-R single",RBIT(CST_R_ABBR_R),0,quote,1);
    const uint32_t lb[]={LBKT}; expect_keys("VEXT-L single",RBIT(CST_R_VEXT_L),0,lb,1);
    const uint32_t qmark[]={QUESTION}; expect_keys("VEXT-R single",RBIT(CST_R_VEXT_R),0,qmark,1);
    const uint32_t rb[]={RBKT}; expect_keys("SYMBOL-L single",RBIT(CST_R_SYMBOL_L),0,rb,1);
    const uint32_t bs[]={BSPC}; expect_keys("SYMBOL-R single",RBIT(CST_R_SYMBOL_R),0,bs,1);
    const uint32_t pair_quotes[]={DQT,DQT,LEFT};
    expect_keys("paired quotes",RBIT(CST_R_VEXT_L)|RBIT(CST_R_ABBR_R),0,pair_quotes,3);
    const uint32_t qi[]={QUESTION,EXCL};
    expect_keys("question exclaim",RBIT(CST_R_ABBR_L)|RBIT(CST_R_VEXT_R),0,qi,2);
    const uint32_t hashes[]={HASH,HASH,HASH};
    expect_keys("triple hash",RBIT(CST_R_SYMBOL_L)|RBIT(CST_R_SYMBOL_R),0,hashes,3);

    const uint32_t undo[]={LC(Z)}; expect_keys("undo",RBIT(CST_R_V_O)|RBIT(CST_R_V_U),0,undo,1);
    const uint32_t redo[]={LC(Y)}; expect_keys("redo",RBIT(CST_R_V_A)|RBIT(CST_R_V_EO),0,redo,1);
    const uint32_t copy[]={LC(C)}; expect_keys("copy",RBIT(CST_R_V_O)|RBIT(CST_R_V_EU)|RBIT(CST_R_V_U),0,copy,1);
    const uint32_t paste[]={LC(V)}; expect_keys("paste",RBIT(CST_R_V_A)|RBIT(CST_R_V_I)|RBIT(CST_R_V_EO),0,paste,1);

    const uint32_t direct_g[]={R};
    expect_keys("symbol direct initial",RBIT(CST_R_SYMBOL_L)|RBIT(CST_R_I_G),0,direct_g,1);
    const uint32_t direct_ae[]={O};
    expect_keys("symbol direct vowel",RBIT(CST_R_SYMBOL_R)|RBIT(CST_R_V_A)|RBIT(CST_R_V_I),0,direct_ae,1);
    expect_keys("initial-only silent",RBIT(CST_R_I_G),0,NULL,0);
    expect_keys("compound-final direct rejected",RBIT(CST_R_SYMBOL_L)|RBIT(CST_R_F_G)|RBIT(CST_R_F_S),0,NULL,0);

    /* Phantom-role audit: D+B is not a legal final and may never become M. */
    expect_silent_or_rejected("illegal final D+B",
        RBIT(CST_R_I_G)|RBIT(CST_R_V_A)|RBIT(CST_R_F_D)|RBIT(CST_R_F_B), 0);
    expect_silent_or_rejected("illegal final D+B+M",
        RBIT(CST_R_I_G)|RBIT(CST_R_V_A)|RBIT(CST_R_F_D)|RBIT(CST_R_F_B)|RBIT(CST_R_F_M), 0);

    /* Continuous correction units normalize initial/final copies. */
    struct cornix_steno_decoded correction;
    if (cornix_steno_decode_correction_unit(RBIT(CST_R_I_G), &correction) ||
        correction.kind != CST_DECODE_KEYS || correction.key_count != 1 ||
        correction.keys[0] != R) {
        fprintf(stderr, "FAIL correction initial G\n"); failures++;
    }
    if (cornix_steno_decode_correction_unit(RBIT(CST_R_F_G), &correction) ||
        correction.kind != CST_DECODE_KEYS || correction.key_count != 1 ||
        correction.keys[0] != R) {
        fprintf(stderr, "FAIL correction final G normalization\n"); failures++;
    }
    if (cornix_steno_decode_correction_unit(
            RBIT(CST_R_I_DOUBLE)|RBIT(CST_R_F_G), &correction) ||
        correction.key_count != 1 || correction.keys[0] != LS(R)) {
        fprintf(stderr, "FAIL correction double G\n"); failures++;
    }
    if (cornix_steno_decode_correction_unit(
            RBIT(CST_R_F_R)|RBIT(CST_R_F_G), &correction) >= 0) {
        fprintf(stderr, "FAIL correction compound final should be sequential\n"); failures++;
    }

    /* Either symbol key mirrors the full physical Q..P row to 0..9. */
    const uint32_t digit_keys[10]={N0,N1,N2,N3,N4,N5,N6,N7,N8,N9};
    for (uint8_t sym=0; sym<2; ++sym) {
        const uint8_t spos = sym ? 47 : 40;
        for (uint8_t i=0; i<10; ++i) {
            char name[40];
            snprintf(name,sizeof(name),"number %c via SYMBOL-%c",'0'+i,sym?'R':'L');
            expect_keys(name,0,CST_POSITION_BIT(spos)|CST_POSITION_BIT((uint8_t)(1+i)),&digit_keys[i],1);
        }
    }

    if (cornix_steno_dictionary_count()!=43) { fprintf(stderr,"FAIL dictionary count\n"); failures++; }

    /* Completed canonical exact-mask dictionary: direct masks and selectors. */
    const uint32_t geu[]={R,M};
    expect_dictionary("그런 direct",
        RBIT(CST_R_I_G)|RBIT(CST_R_F_R),geu,2);
    expect_dictionary("그리고 direct",
        RBIT(CST_R_I_G)|RBIT(CST_R_I_R)|RBIT(CST_R_I_DOUBLE),geu,2);
    expect_dictionary("그럼 ABBR_L",
        RBIT(CST_R_ABBR_L)|RBIT(CST_R_I_G)|RBIT(CST_R_F_R),geu,2);
    expect_dictionary("그럼 ABBR_R mirror",
        RBIT(CST_R_ABBR_R)|RBIT(CST_R_I_G)|RBIT(CST_R_F_R),geu,2);

    const uint32_t yeotda[]={D,U,LS(T),E,K};
    expect_dictionary("였다 direct",
        RBIT(CST_R_F_NG)|RBIT(CST_R_F_D),yeotda,5);
    const uint32_t eopda[]={D,J,Q,T,E,K};
    expect_dictionary("없다 ABBR_R",
        RBIT(CST_R_ABBR_R)|RBIT(CST_R_F_NG)|RBIT(CST_R_F_D),eopda,6);
    const uint32_t eotda[]={D,J,LS(T),E,K};
    expect_dictionary("었다 ABBR_L",
        RBIT(CST_R_ABBR_L)|RBIT(CST_R_F_NG)|RBIT(CST_R_F_D),eotda,5);

    const uint32_t ieotda[]={D,L,D,J,LS(T),E,K};
    const uint64_t od_tail=RBIT(CST_R_F_NG)|RBIT(CST_R_F_D);
    expect_dictionary("이었다 right double",
        RBIT(CST_R_F_DOUBLE)|od_tail,ieotda,7);

    const uint32_t yeosseotda[]={D,U,LS(T),D,J,LS(T),E,K};
    expect_dictionary("였었다 right VEXT",
        RBIT(CST_R_VEXT_R)|od_tail,yeosseotda,8);
    if (cornix_steno_dictionary_lookup(RBIT(CST_R_VEXT_L)|od_tail)) {
        fprintf(stderr,"FAIL left VEXT incorrectly mirrored to 였었다\n"); failures++;
    }

    const uint32_t eosseotda[]={D,J,LS(T),D,J,LS(T),E,K};
    expect_dictionary("었었다 AB2 left",
        RBIT(CST_R_VEXT_L)|RBIT(CST_R_ABBR_R)|od_tail,eosseotda,8);
    expect_dictionary("었었다 AB2 right",
        RBIT(CST_R_VEXT_R)|RBIT(CST_R_ABBR_R)|od_tail,eosseotda,8);
    if (cornix_steno_quick_count()!=12) { fprintf(stderr,"FAIL quick count\n"); failures++; }
    const struct cornix_steno_quick_entry *quick=cornix_steno_quick_lookup(
        RBIT(CST_R_ABBR_L)|RBIT(CST_R_V_EU));
    if (!quick || quick->macro_slot!=CST_QUICK_M0) {
        fprintf(stderr,"FAIL GUI quick macro M0\n"); failures++;
    }
    quick=cornix_steno_quick_lookup(RBIT(CST_R_ABBR_R)|RBIT(CST_R_V_EU));
    if (!quick || quick->macro_slot!=CST_QUICK_M11) {
        fprintf(stderr,"FAIL GUI quick macro M11\n"); failures++;
    }

    if (failures) return EXIT_FAILURE;
    puts("Cornix STENO decoder/dictionary tests: PASS (43 canonical exact masks + 12 GUI quick macros)");
    return EXIT_SUCCESS;
}

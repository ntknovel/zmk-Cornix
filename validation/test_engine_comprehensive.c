#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dt-bindings/zmk/keys.h>
#include <zephyr/kernel.h>
#include <cornix_steno/engine.h>
#include <cornix_steno/roles.h>
#include "role_position_map.h"
#include "canonical_engine_cases.h"

extern uint32_t test_output_keys[];
extern size_t test_output_count;
void test_output_reset(void);
struct test_key_event { uint32_t key; bool state; int64_t ts; };
extern struct test_key_event test_key_events[];
extern size_t test_key_event_count;
void test_key_events_reset(void);
static int failures;
static int64_t clock_ms;

static void reset_case(void) {
    cornix_steno_engine_set_active(false);
    cornix_steno_engine_set_active(true);
    test_output_reset();
    test_key_events_reset();
    clock_ms += 1000;
    test_time_set(clock_ms);
}
static int press_r(enum cornix_steno_role role) {
    uint32_t p=cst_test_position_for_role(role);
    test_time_set(++clock_ms);
    return cornix_steno_engine_role_pressed(role,p,clock_ms);
}
static int release_r(enum cornix_steno_role role) {
    uint32_t p=cst_test_position_for_role(role);
    test_time_set(++clock_ms);
    return cornix_steno_engine_role_released(role,p,clock_ms);
}
static void advance_ms(int ms) { test_time_advance(ms); clock_ms += ms; }
static bool is_selector(enum cornix_steno_role r) {
    return r==CST_R_ABBR_L||r==CST_R_ABBR_R||r==CST_R_VEXT_L||r==CST_R_VEXT_R;
}
static bool is_dedicated_double(enum cornix_steno_role r) {
    return r==CST_R_I_DOUBLE||r==CST_R_F_DOUBLE;
}
static void expect_output(const char *label,const uint32_t *keys,size_t count) {
    if (test_output_count!=count || (count&&memcmp(test_output_keys,keys,count*sizeof(*keys)))) {
        fprintf(stderr,"FAIL %s output=%zu expected=%zu\n",label,test_output_count,count); failures++;
    }
    if (test_key_event_count) {
        fprintf(stderr,"FAIL %s unexpected immediate events=%zu\n",label,test_key_event_count); failures++;
    }
}
static void run_canonical(const struct cst_canonical_case *c,int mode) {
    char label[160]; snprintf(label,sizeof(label),"canonical %s mode%d",c->id,mode);
    reset_case();
    enum cornix_steno_role selectors[8],cores[8],doubles[8]; int ns=0,nc=0,nd=0;
    for (int i=0;i<c->role_count;i++) {
        enum cornix_steno_role r=c->roles[i];
        if (is_selector(r)) selectors[ns++]=r;
        else if (is_dedicated_double(r)) doubles[nd++]=r;
        else cores[nc++]=r;
    }
    if (mode==0) {
        for(int i=0;i<c->role_count;i++) press_r(c->roles[i]);
        for(int i=c->role_count-1;i>=0;i--) release_r(c->roles[i]);
    } else if (mode==1 && ns) {
        for(int i=0;i<ns;i++) press_r(selectors[i]);
        for(int i=0;i<nc;i++){ press_r(cores[i]); release_r(cores[i]); if(i+1<nc) advance_ms(55); }
        for(int i=0;i<nd;i++){ press_r(doubles[i]); release_r(doubles[i]); }
        for(int i=ns-1;i>=0;i--) release_r(selectors[i]);
    } else if (mode==2) {
        for(int i=0;i<nc;i++) press_r(cores[i]);
        for(int i=0;i<nd;i++) press_r(doubles[i]);
        for(int i=0;i<ns;i++) press_r(selectors[i]);
        for(int i=ns-1;i>=0;i--) release_r(selectors[i]);
        for(int i=nd-1;i>=0;i--) release_r(doubles[i]);
        for(int i=nc-1;i>=0;i--) release_r(cores[i]);
    } else if (mode==3 && ns && nd) {
        /* Selector-first, dedicated-double tap fully released before core. */
        for(int i=0;i<ns;i++) press_r(selectors[i]);
        for(int i=0;i<nd;i++){ press_r(doubles[i]); release_r(doubles[i]); }
        advance_ms(55);
        for(int i=0;i<nc;i++){ press_r(cores[i]); release_r(cores[i]); if(i+1<nc) advance_ms(55); }
        for(int i=ns-1;i>=0;i--) release_r(selectors[i]);
    } else {
        for(int i=c->role_count-1;i>=0;i--) press_r(c->roles[i]);
        for(int i=0;i<c->role_count;i++) release_r(c->roles[i]);
    }
    expect_output(label,c->keys,c->key_count);
}

struct vowel_case { const char *name; enum cornix_steno_role r[3]; int n; uint32_t keys[2]; int nk; };
static const struct vowel_case vowels[] = {
 {"ae",{CST_R_V_A,CST_R_V_I},2,{O},1}, {"e",{CST_R_V_I,CST_R_V_EO},2,{P},1},
 {"yae",{CST_R_V_EU,CST_R_V_A},2,{LS(O)},1}, {"ye",{CST_R_V_EU,CST_R_V_EO},2,{LS(P)},1},
 {"wa",{CST_R_V_O,CST_R_V_A},2,{H,K},2}, {"wae",{CST_R_V_O,CST_R_V_EO},2,{H,O},2},
 {"oe",{CST_R_V_O,CST_R_V_I},2,{H,L},2}, {"wo",{CST_R_V_U,CST_R_V_EO},2,{N,J},2},
 {"we",{CST_R_V_U,CST_R_V_A},2,{N,P},2}, {"wi",{CST_R_V_U,CST_R_V_I},2,{N,L},2},
 {"ui",{CST_R_V_EU,CST_R_V_I},2,{M,L},2}, {"ya",{CST_R_VEXT_L,CST_R_V_A},2,{I},1},
 {"yeo",{CST_R_VEXT_R,CST_R_V_EO},2,{U},1}, {"yo",{CST_R_VEXT_L,CST_R_V_O},2,{Y},1},
 {"yu",{CST_R_VEXT_R,CST_R_V_U},2,{B},1}, {"vext-ae",{CST_R_VEXT_L,CST_R_V_I},2,{O},1},
 {"vext-e",{CST_R_VEXT_R,CST_R_V_EU},2,{P},1},
 {"both-yae",{CST_R_VEXT_L,CST_R_VEXT_R,CST_R_V_I},3,{LS(O)},1},
 {"both-ye",{CST_R_VEXT_L,CST_R_VEXT_R,CST_R_V_EU},3,{LS(P)},1},
};
static void run_vowel_skew(const struct vowel_case *v,int order) {
    char label[96]; snprintf(label,sizeof(label),"vowel-skew %s order%d",v->name,order);
    reset_case();
    for(int i=0;i<v->n;i++) press_r(v->r[i]);
    advance_ms(80);
    int first=order? v->n-1:0;
    release_r(v->r[first]);
    advance_ms(55);
    for(int i=0;i<v->n;i++) if(i!=first) release_r(v->r[i]);
    expect_output(label,v->keys,v->nk);
}

static const enum cornix_steno_role isel[3]={CST_R_I_DOUBLE,CST_R_I_KH,CST_R_I_NG};
static const enum cornix_steno_role itgt[5]={CST_R_I_B,CST_R_I_J,CST_R_I_D,CST_R_I_G,CST_R_I_S};
static const enum cornix_steno_role fsel[3]={CST_R_F_DOUBLE,CST_R_F_KH,CST_R_F_NG};
static const enum cornix_steno_role ftgt[5]={CST_R_F_B,CST_R_F_J,CST_R_F_D,CST_R_F_G,CST_R_F_S};
static const uint32_t dkeys[5]={LS(Q),LS(W),LS(E),LS(R),LS(T)};
static void run_double(bool final,int si,int ti,int release_order) {
    char label[96]; snprintf(label,sizeof(label),"double %c s%d t%d o%d",final?'F':'I',si,ti,release_order);
    reset_case();
    enum cornix_steno_role s=final?fsel[si]:isel[si], t=final?ftgt[ti]:itgt[ti];
    press_r(s); press_r(t); advance_ms(80);
    if(release_order==0){release_r(s);advance_ms(55);release_r(t);} else {release_r(t);advance_ms(55);release_r(s);}
    expect_output(label,&dkeys[ti],1);
}
static void expect_events(const char *label,uint32_t key,int taps) {
    if(test_output_count){fprintf(stderr,"FAIL %s unexpected text=%zu\n",label,test_output_count);failures++;}
    if(test_key_event_count!=(size_t)taps*2){fprintf(stderr,"FAIL %s events=%zu expected=%d\n",label,test_key_event_count,taps*2);failures++;return;}
    for(int i=0;i<taps;i++) if(test_key_events[i*2].key!=key||!test_key_events[i*2].state||test_key_events[i*2+1].key!=key||test_key_events[i*2+1].state){fprintf(stderr,"FAIL %s event %d\n",label,i);failures++;break;}
}

static const enum cornix_steno_role solo_i_roles[14]={
    CST_R_I_KH,CST_R_I_B,CST_R_I_J,CST_R_I_H,CST_R_I_N,CST_R_I_D,CST_R_I_S,
    CST_R_I_G,CST_R_I_NG,CST_R_I_P,CST_R_I_CH,CST_R_I_T,CST_R_I_M,CST_R_I_R};
static const enum cornix_steno_role solo_f_roles[14]={
    CST_R_F_KH,CST_R_F_B,CST_R_F_J,CST_R_F_H,CST_R_F_N,CST_R_F_D,CST_R_F_S,
    CST_R_F_G,CST_R_F_NG,CST_R_F_P,CST_R_F_CH,CST_R_F_T,CST_R_F_M,CST_R_F_R};
static const uint32_t solo_c_keys[14]={Z,Q,W,G,S,E,T,R,D,V,C,X,A,F};
static const enum cornix_steno_role solo_v_roles[6]={CST_R_V_O,CST_R_V_EU,CST_R_V_U,CST_R_V_A,CST_R_V_I,CST_R_V_EO};
static const uint32_t solo_v_keys[6]={H,M,N,K,L,J};
static void solo_hold_regressions(void) {
    for(int side=0;side<2;side++) for(int i=0;i<14;i++) {
        char label[80]; snprintf(label,sizeof(label),"solo consonant %c %d",side?'F':'I',i);
        reset_case(); enum cornix_steno_role r=side?solo_f_roles[i]:solo_i_roles[i];
        press_r(r); advance_ms(79); release_r(r); /* release helper adds the 80th ms */
        expect_output(label,&solo_c_keys[i],1);
    }
    for(int i=0;i<6;i++) {
        char label[80]; snprintf(label,sizeof(label),"solo vowel %d",i);
        reset_case(); press_r(solo_v_roles[i]); advance_ms(79); release_r(solo_v_roles[i]);
        expect_output(label,&solo_v_keys[i],1);
    }
    reset_case(); press_r(CST_R_V_I); advance_ms(78); release_r(CST_R_V_I);
    expect_output("79ms vowel not direct",NULL,0);
    reset_case(); press_r(CST_R_V_I); advance_ms(79); release_r(CST_R_V_I);
    {const uint32_t e[]={L};expect_output("80ms vowel direct",e,1);}
}

static void special_regressions(void) {
    /* Short ambiguous final strokes stay canonical. */
    reset_case(); press_r(CST_R_F_NG); press_r(CST_R_F_D); release_r(CST_R_F_NG); release_r(CST_R_F_D);
    const uint32_t yeotda[]={D,U,LS(T),E,K}; expect_output("short F_NG+F_D is yeotda",yeotda,5);
    reset_case(); press_r(CST_R_F_DOUBLE); press_r(CST_R_F_D); release_r(CST_R_F_DOUBLE); release_r(CST_R_F_D);
    const uint32_t dwaetda[]={E,H,O,LS(T),E,K}; expect_output("short F_DOUBLE+F_D is dwaetda",dwaetda,6);

    /* ABBR_R + 종ㅇ/종ㄷ sequentially must be 없다 and never Right. */
    reset_case(); press_r(CST_R_ABBR_R); press_r(CST_R_F_NG); release_r(CST_R_F_NG); advance_ms(55); press_r(CST_R_F_D); release_r(CST_R_F_D); release_r(CST_R_ABBR_R);
    const uint32_t eopda[]={D,J,Q,T,E,K}; expect_output("selector-first eopda",eopda,6);

    /* Dedicated-double first, fully released, then core: 그러기. */
    reset_case(); press_r(CST_R_ABBR_L); press_r(CST_R_I_DOUBLE); release_r(CST_R_I_DOUBLE); advance_ms(55); press_r(CST_R_I_G); release_r(CST_R_I_G); advance_ms(55); press_r(CST_R_I_R); release_r(CST_R_I_R); release_r(CST_R_ABBR_L);
    const uint32_t geureogi[]={R,M,F,J,R,L}; expect_output("double-first geureogi",geureogi,6);

    /* One ambiguous nav tap resolves on ABBR release; a second tap repeats. */
    reset_case(); press_r(CST_R_ABBR_L); press_r(CST_R_I_DOUBLE); release_r(CST_R_I_DOUBLE); release_r(CST_R_ABBR_L); expect_events("deferred one left",LEFT,1);
    reset_case(); press_r(CST_R_ABBR_L); press_r(CST_R_I_DOUBLE); release_r(CST_R_I_DOUBLE); press_r(CST_R_I_DOUBLE); release_r(CST_R_I_DOUBLE); release_r(CST_R_ABBR_L); expect_events("deferred repeated left",LEFT,2);
}
int main(void) {
    cornix_steno_engine_set_active(true);
    for(size_t i=0;i<CST_CANONICAL_CASE_COUNT;i++) for(int mode=0;mode<4;mode++) run_canonical(&cst_canonical_cases[i],mode);
    for(size_t i=0;i<sizeof(vowels)/sizeof(vowels[0]);i++) for(int o=0;o<2;o++) run_vowel_skew(&vowels[i],o);
    for(int side=0;side<2;side++) for(int s=0;s<3;s++) for(int t=0;t<5;t++) for(int o=0;o<2;o++) run_double(side,s,t,o);
    special_regressions();
    solo_hold_regressions();
    if(failures) return EXIT_FAILURE;
    printf("Cornix comprehensive audit regressions: PASS (%u canonical-order + %zu vowel-skew + 60 double-matrix + 34 solo-jamo + threshold + specials)\n",CST_CANONICAL_CASE_COUNT*4u,sizeof(vowels)/sizeof(vowels[0])*2u);
    return EXIT_SUCCESS;
}

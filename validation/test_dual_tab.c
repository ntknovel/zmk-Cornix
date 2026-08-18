#include <stdio.h>
#include <stdlib.h>
#include <dt-bindings/zmk/keys.h>
#include <zephyr/kernel.h>
#include <cornix_steno/dual.h>
#include <cornix_steno/tab.h>
extern uint32_t test_output_keys[]; extern size_t test_output_count; void test_output_reset(void);
extern size_t test_engine_press_count,test_engine_release_count; extern enum cornix_steno_role test_engine_last_role; void test_engine_capture_reset(void);
static int failures;
static void reset_all(void){cornix_steno_tab_reset();cornix_steno_dual_reset();test_output_reset();test_engine_capture_reset();test_time_set(0);} 
static void expect(const char*n,uint32_t k){if(test_output_count!=1||test_output_keys[0]!=k){fprintf(stderr,"FAIL %s count=%zu\n",n,test_output_count);failures++;}}
static void none(const char*n){if(test_output_count){fprintf(stderr,"FAIL %s unexpected\n",n);failures++;}}
int main(void){
 reset_all();cornix_steno_dual_pressed(43,CST_DUAL_VOWEL,CST_R_V_U,ENTER,0,0);cornix_steno_dual_released(43,50);expect("VU Enter",ENTER);
 reset_all();cornix_steno_dual_pressed(44,CST_DUAL_VOWEL,CST_R_V_A,SPACE,0,0);cornix_steno_dual_released(44,50);expect("VA Space",SPACE);
 reset_all();cornix_steno_dual_pressed(43,CST_DUAL_VOWEL,CST_R_V_U,ENTER,0,0);cornix_steno_dual_released(43,150);expect("VU solo-hold direct",N);
 reset_all();cornix_steno_dual_pressed(44,CST_DUAL_VOWEL,CST_R_V_A,SPACE,0,0);cornix_steno_dual_released(44,151);expect("VA solo-hold direct",K);
 reset_all();cornix_steno_dual_pressed(43,CST_DUAL_VOWEL,CST_R_V_U,ENTER,0,0);cornix_steno_dual_notify_other_press(14,20);
 if(test_engine_press_count!=1||test_engine_last_role!=CST_R_V_U){fprintf(stderr,"FAIL VU role commit\n");failures++;}
 cornix_steno_dual_released(43,60);if(test_engine_release_count!=1){fprintf(stderr,"FAIL VU release\n");failures++;}none("VU chord no Enter");
 reset_all();cornix_steno_tab_pressed(0,0);cornix_steno_tab_released(0,80);expect("Tab short one-shot",TAB);
 reset_all();cornix_steno_tab_pressed(0,0);test_time_advance(600);cornix_steno_tab_released(0,610);expect("Tab long one-shot",TAB);
 if(failures) return EXIT_FAILURE;
 puts("Cornix STENO v2.1.1 vowel tri-state/one-shot-Tab tests: PASS");
 return EXIT_SUCCESS;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dt-bindings/zmk/keys.h>
#include <zephyr/kernel.h>
#include <cornix_steno/engine.h>
#include <cornix_steno/roles.h>
extern uint32_t test_output_keys[]; extern size_t test_output_count;
void test_output_reset(void); static int failures;
static void expect(const char *name,const uint32_t *keys,size_t count){
 if(test_output_count!=count||(count&&memcmp(test_output_keys,keys,count*sizeof(keys[0])))){fprintf(stderr,"FAIL %s got=%zu expected=%zu\n",name,test_output_count,count);failures++;} test_output_reset();}
static void press(enum cornix_steno_role x,uint32_t p,int64_t t){test_time_set(t);cornix_steno_engine_role_pressed(x,p,t);} 
static void release(enum cornix_steno_role x,uint32_t p,int64_t t){test_time_set(t);cornix_steno_engine_role_released(x,p,t);} 
int main(void){
 cornix_steno_engine_set_active(true);
 press(CST_R_I_G,14,0);press(CST_R_V_A,44,8);press(CST_R_F_N,8,12);
 release(CST_R_I_G,14,30);release(CST_R_V_A,44,35);release(CST_R_F_N,8,40);
 const uint32_t gan[]={R,K,S};expect("normal gan",gan,3);

 press(CST_R_I_G,14,100);press(CST_R_V_A,44,108);press(CST_R_F_N,8,112);
 press(CST_R_I_NG,16,200);release(CST_R_I_NG,16,215);test_time_set(215);test_time_advance(55);
 release(CST_R_I_G,14,280);release(CST_R_V_A,44,285);release(CST_R_F_N,8,290);
 expect("late accidental removed",gan,3);

 press(CST_R_I_G,14,400);release(CST_R_I_G,14,410);expect("solo consonant silent",NULL,0);

 press(CST_R_ABBR_L,12,500);press(CST_R_I_G,14,505);press(CST_R_F_R,20,510);press(CST_R_F_G,21,515);
 release(CST_R_ABBR_L,12,530);release(CST_R_I_G,14,535);release(CST_R_F_R,20,540);release(CST_R_F_G,21,545);
 if(test_output_count<2||test_output_keys[0]!=R||test_output_keys[1]!=M){fprintf(stderr,"FAIL dictionary prefix count=%zu\n",test_output_count);failures++;}test_output_reset();

 press(CST_R_ABBR_L,12,600);release(CST_R_ABBR_L,12,610);const uint32_t ex[]={EXCL};expect("marker single",ex,1);

 /* Reserved blank quick slot is consumed and remains silent. */
 press(CST_R_ABBR_L,12,700);press(CST_R_V_EU,42,705);press(CST_R_V_O,41,710);press(CST_R_V_U,43,715);
 release(CST_R_ABBR_L,12,730);release(CST_R_V_EU,42,735);release(CST_R_V_O,41,740);release(CST_R_V_U,43,745);
 expect("blank quick slot",NULL,0);

 press(CST_R_I_G,14,800);cornix_steno_engine_set_active(false);release(CST_R_I_G,14,820);expect("mode exit cancel",NULL,0);
 if(failures) return EXIT_FAILURE;
 puts("Cornix STENO engine state tests: PASS");
 return EXIT_SUCCESS;
}

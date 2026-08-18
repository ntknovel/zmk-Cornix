/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <cornix_steno/led.h>

static void assert_colors(uint8_t packed, const uint8_t expected[4]) {
    for (uint8_t i=0;i<4;i++) assert(cornix_steno_led_global_color(packed,i)==expected[i]);
}

int main(void) {
    const uint8_t off[4]={0,0,0,0};
    uint8_t p=cornix_steno_led_pack_state(false,true,false,4,CST_ABBR_CATEGORY_NONE);
    assert_colors(p,off);

    p=cornix_steno_led_pack_state(true,true,true,0,CST_ABBR_CATEGORY_NONE);
    const uint8_t all_white[4]={7,7,7,7}; assert_colors(p,all_white);

    for(uint8_t count=0;count<=4;count++){
        p=cornix_steno_led_pack_state(true,true,false,count,CST_ABBR_CATEGORY_NONE);
        for(uint8_t i=0;i<4;i++) assert(cornix_steno_led_global_color(p,i)==(i<count?7:0));
    }
    p=cornix_steno_led_pack_state(true,true,false,2,CST_ABBR_CATEGORY_BASIC);
    const uint8_t basic[4]={2,2,0,0}; assert_colors(p,basic);
    p=cornix_steno_led_pack_state(true,true,false,3,CST_ABBR_CATEGORY_CONNECTIVE);
    const uint8_t conn[4]={0,3,3,0}; assert_colors(p,conn);
    p=cornix_steno_led_pack_state(true,true,false,1,CST_ABBR_CATEGORY_TAIL);
    const uint8_t tail[4]={0,0,4,4}; assert_colors(p,tail);

    assert(cornix_steno_led_global_index_for_local(true,0)==1);
    assert(cornix_steno_led_global_index_for_local(true,1)==0);
    assert(cornix_steno_led_global_index_for_local(false,0)==2);
    assert(cornix_steno_led_global_index_for_local(false,1)==3);

    puts("Cornix STENO LED order/count/category tests: PASS");
    return 0;
}

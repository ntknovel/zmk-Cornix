/* Executes the real language-return behavior with layer/output test doubles. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <dt-bindings/zmk/keys.h>
#include "../src/behaviors/behavior_steno_language_return.c"
static bool layer_on, pending, active;
static uint32_t sent[16]; static size_t count;
static unsigned resets;
static struct behavior_steno_language_return_config cfg={.layer=6};
static struct device dev={.config=&cfg,.api=&api};
const struct device *zmk_behavior_get_binding(const char *name) { return strcmp(name,"missing")==0?NULL:&dev; }
bool zmk_keymap_layer_active(uint8_t l) {assert(l==6);return layer_on;}
int zmk_keymap_layer_activate(uint8_t l,bool b) {(void)b;assert(l==6);layer_on=true;return 0;}
int zmk_keymap_layer_deactivate(uint8_t l,bool b) {(void)b;assert(l==6);layer_on=false;return 0;}
bool cornix_steno_english_return_take_pending(void) {bool p=pending;pending=false;return p;}
void cornix_steno_english_return_set_pending(bool p) {pending=p;}
void cornix_steno_output_flush(void) {count=0;}
void cornix_steno_tab_reset(void) {resets++;}
void cornix_steno_dual_reset(void) {resets++;}
void cornix_steno_engine_cancel_pending(void) {resets++;}
void cornix_steno_engine_set_active(bool a) {active=a;}
int cornix_steno_output_enqueue(uint32_t k) {assert(count<16);sent[count++]=k;return 0;}
int main(void) {
    struct zmk_behavior_binding b={.behavior_dev="st_lang_return"};
    struct zmk_behavior_binding_event e={.position=31,.timestamp=100};
    assert(on_pressed(&b,e)==0 && count==0);
    assert(on_released(&b,e)==0 && count==1 && sent[0]==K_APP && resets==0);
    layer_on=true;active=true;count=0;
    assert(on_released(&b,e)==0 && count==1 && sent[0]==RALT);
    assert(!layer_on && !active && pending && resets==3);
    assert(on_released(&b,e)==0 && count==1 && sent[0]==RALT);
    assert(layer_on && active && !pending && resets==6);
    b.behavior_dev="missing";
    assert(on_released(&b,e)==-ENODEV);
    puts("PASS: actual right-click handler: ordinary BASE Menu / STENO English exit / return / missing device");
    return 0;
}

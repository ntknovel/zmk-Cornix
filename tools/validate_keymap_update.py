#!/usr/bin/env python3
"""v2.2.8 changes plus v2.2.9 Select Shift correction; source-contract checks. Native ZMK runtime is NOT simulated here."""
from pathlib import Path
import argparse, hashlib, json, re
import yaml

EXPECTED = {
 'base':{0:'&bang_tab',11:'&kp QUESTION',12:'&mp200 LCTRL TILDE',
         24:'&mp200 LSHIFT LBKT',37:'&mp200 RSHIFT RBKT',38:'&mp200 LGUI CAPSLOCK',
         40:'&ht200 LC(Z) LBKT',41:'&lt250 NUM MINUS',42:'&mo MOUSE',
         45:'&mo NAV',46:'&lt250 FN EQUAL',47:'&kp LC(Y)',48:'&mp200 RCTRL RALT',36:'&kp FSLH'},
 'num':{9:'&kp N9',10:'&kp N0'}, 'mouse':{2:'&mkp MB1',4:'&mkp MB2'},
 'steno':{0:'&kp EXCL',11:'&kp QUESTION',38:'&kp LA(TAB)',39:'&kp LA(LS(TAB))',48:'&st_sel 0 RALT'},
}
LAYERS=['base','num','mouse','nav','fn','symbol','steno','select_nav']

def strip_comments(s): return re.sub(r'/\*.*?\*/|//[^\n]*','',s,flags=re.S)
def normalized(s): return re.sub(r'\s+','',strip_comments(s))
def layer_match(s,name):
 m=re.search(rf'\b{name}_layer\s*\{{.*?\bbindings\s*=\s*<(.*?)>;',s,re.S)
 assert m,name
 return m

def bindings(s,name):return [' '.join(x.split()) for x in re.findall(r'&[^&]+',layer_match(s,name).group(1))]
def node(s,label):
 m=re.search(rf'\b{label}:\s*\w+\s*\{{.*?\}};',s,re.S)
 assert m,label
 return m

def main(root):
 key=(root/'config/cornix.keymap').read_text()
 base=(root/'validation/reference_v2_2_7/cornix.keymap').read_text()
 audit=json.loads((root/'KEYMAP_UPDATE_v2_2_9.json').read_text())
 for rel,entry in audit['requested_runtime_changes'].items():
  assert hashlib.sha256((root/entry['reference']).read_bytes()).hexdigest()==entry['baseline_sha256'],rel
  assert hashlib.sha256((root/rel).read_bytes()).hexdigest()==entry['updated_sha256'],rel
 count=0
 for name in LAYERS:
  original=bindings(base,name);actual=bindings(key,name);assert len(original)==len(actual)==50
  expected=original[:]
  for pos,binding in EXPECTED.get(name,{}).items():expected[pos]=binding;count+=1
  assert actual==expected, name
 print(f'PASS: all 400 physical bindings; exactly {count} requested binding changes, other positions preserved')
 for i in range(1,11):assert bindings(key,'num')[i]==f'&kp N{i%10}',i
 print('PASS: NUM 01..10 use number-row N1..N0, not keypad usages; 09/10 are 9/0')
 morph=node(key,'bang_tab').group()
 for required in ['compatible="zmk,behavior-mod-morph";', '#binding-cells=<0>;',
                  'bindings=<&ht200TABEXCL>,<&kpTAB>;',
                  'mods=<(MOD_LCTL|MOD_RCTL|MOD_LALT|MOD_RALT|MOD_LGUI)>;',
                  'keep-mods=<(MOD_LCTL|MOD_RCTL|MOD_LALT|MOD_RALT|MOD_LGUI)>;']:
  assert required in normalized(morph),required
 modifier=node(key,'mp200').group()
 for required in ['compatible="zmk,behavior-hold-tap";','#binding-cells=<2>;',
                  'bindings=<&kp>,<&kp>;','flavor="hold-preferred";','tapping-term-ms=<200>;']:
  assert required in normalized(modifier),required
 for forbidden in ['quick-tap-ms','require-prior-idle-ms','retro-tap','hold-trigger-key-positions']:
  assert forbidden not in modifier,forbidden
 assert 'combo_grave' not in key
 combos=re.findall(r'\b(combo_\w+)\s*\{(.*?)\};',key,re.S)
 assert len(combos)==12,len(combos)
 for label,body in combos:
  match=re.search(rf'\b{label}\s*\{{(.*?)\}};',base,re.S)
  assert match and normalized(body)==normalized(match.group(1)),label
  positions=[int(v) for v in re.search(r'key-positions\s*=\s*<([^>]+)>',body).group(1).split()]
  assert 0 not in positions and 12 not in positions
 print('PASS: modifier-preserving Tab morph; 200ms hold-preferred modifier keys; old 00+12 grave conflict removed')
 sensor=lambda s,n: re.search(rf'\b{n}_layer\s*\{{.*?sensor-bindings\s*=\s*<([^>]+)>',s,re.S).group(1)
 # Only read sensors from layers that actually define one, with layer bounds.
 for name in LAYERS:
  block=lambda s:re.search(rf'\b{name}_layer\s*\{{(.*?)\}};',s,re.S).group(1)
  a=re.search(r'sensor-bindings\s*=\s*<([^>]+)>',block(base))
  b=re.search(r'sensor-bindings\s*=\s*<([^>]+)>',block(key))
  if name=='steno':assert normalized(b.group(1))=='&st_encLS(UP)LS(DOWN)&st_encLS(PG_UP)LS(PG_DN)'
  else:assert (normalized(a.group(1)) if a else None)==(normalized(b.group(1)) if b else None),name
 print('PASS: only STENO right encoder changed: CW Shift+PageUp / CCW Shift+PageDown')
 # Reverse every approved edit, then compare the rest of the DTS semantically.
 rev=key
 for name in LAYERS:
  m=layer_match(rev,name);orig=layer_match(base,name).group(1)
  rev=rev[:m.start(1)]+orig+rev[m.end(1):]
 rev=rev.replace('&st_enc LS(UP) LS(DOWN)  &st_enc LS(PG_UP) LS(PG_DN)',
                 '&st_enc LS(UP) LS(DOWN)  &st_enc UP DOWN')
 for label in ['mp200','bang_tab','st_sel']:
  m=node(rev,label);rev=rev[:m.start()]+rev[m.end():]
 rev=rev.replace('#include <dt-bindings/zmk/modifiers.h>','')
 grave=re.search(r'\bcombo_grave\s*\{.*?\};',base,re.S).group()
 rev=rev.replace('compatible = "zmk,combos";','compatible = "zmk,combos";'+grave)
 assert normalized(rev)==normalized(base),'unrequested keymap behavior/macro/combo edit'
 print('PASS: other native behavior definitions, all quick macros and all other sensors unchanged')
 # Reverse only the four singleton outputs. Any other decoder edit fails.
 actual=(root/'src/steno/steno_decoder.c').read_text()
 pairs={'CST_R_VEXT_R':('QUESTION','RBKT'),'CST_R_ABBR_L':('EXCL','TILDE'),
        'CST_R_SYMBOL_L':('RBKT','LC(Z)'),'CST_R_SYMBOL_R':('BSPC','LC(Y)')}
 for role,(old,new) in pairs.items():
  marker=f'EXACT1(BITR({role}), {new});';assert actual.count(marker)==1
  actual=actual.replace(marker,f'EXACT1(BITR({role}), {old});')
 assert actual==(root/'validation/reference_v2_2_7/steno_decoder.c').read_text()
 lang=(root/'src/behaviors/behavior_steno_language_return.c').read_text()
 old=(root/'validation/reference_v2_2_7/behavior_steno_language_return.c').read_text()
 assert normalized(lang.replace('cornix_steno_output_enqueue(K_APP)','cornix_steno_output_enqueue(LC(F))'))==normalized(old)
 print('PASS: decoder delta is exactly four singletons; language-return delta is only ordinary-BASE Menu fallback')
 assert bindings(key,'steno')[48]=='&st_sel 0 RALT'
 assert 'bindings=<&select_shift>,<&kp>;' in normalized(node(key,'st_sel').group())
 assert bindings(key,'base')[31]=='&st_lang_return'
 for pos,label in [(12,'st_abbr_l'),(37,'st_vext_r'),(40,'st_symbol_l'),(47,'st_symbol_r')]:
  assert bindings(key,'steno')[pos]==f'&{label}'
 print('PASS: STENO RAlt solo / restored Select Shift hold; four edited singleton keys still retain their original STENO role anchors')
 rows=yaml.safe_load((root/'build.yaml').read_text())['include']
 assert [r['artifact-name'] for r in rows]==['cornix_left_steno_studio_v2_2_9','cornix_right_steno_v2_2_9','cornix_settings_reset_v2_2_9']
 print('PASS: versioned v2.2.9 targets; ARM/USB hardware test remains separate')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('root',nargs='?',type=Path,default=Path(__file__).resolve().parents[1]);main(p.parse_args().root)

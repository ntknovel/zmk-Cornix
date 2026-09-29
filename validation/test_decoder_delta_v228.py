#!/usr/bin/env python3
"""Compile/run real old/new decoder libraries; only 4 requested singleton changes allowed."""
from pathlib import Path
import ctypes as c, itertools, re, random, subprocess, tempfile
ROOT=Path(__file__).resolve().parents[1]
class Dec(c.Structure):
 _fields_=[('kind',c.c_int),('keys',c.c_uint32*8),('count',c.c_uint8)]
roles={n:int(v) for n,v in re.findall(r'#define (CST_R_\w+) (\d+)',(ROOT/'include/dt-bindings/zmk/cornix_steno.h').read_text())}
expected={1<<roles['CST_R_ABBR_L']:(0x1001,0x1006),1<<roles['CST_R_VEXT_R']:(0x1002,48),
          1<<roles['CST_R_SYMBOL_L']:(48,0x02000000|29),1<<roles['CST_R_SYMBOL_R']:(42,0x02000000|28)}
dtsi=(ROOT/'config/cornix_steno.dtsi').read_text()
lookup={name:roles[role] for name,role in re.findall(r'(\w+):\s*\w+\s*\{.*?role\s*=\s*<(CST_R_\w+)>;.*?\};',dtsi,re.S)}
keymap=(ROOT/'config/cornix.keymap').read_text()
body=re.search(r'\bsteno_layer\s*\{.*?bindings\s*=\s*<(.*?)>;',keymap,re.S).group(1)
position={lookup[n]:p for p,n in enumerate(re.findall(r'&(\w+)',body)) if n in lookup}
assert len(position)==42
with tempfile.TemporaryDirectory(prefix='cst-decoder-delta-') as temp:
 libs=[]
 for name,source in [('before',ROOT/'validation/reference_v2_2_7/steno_decoder.c'),('after',ROOT/'src/steno/steno_decoder.c')]:
  dest=Path(temp)/(name+'.so')
  subprocess.run(['gcc','-std=c11','-Wall','-Wextra','-Werror','-shared','-fPIC','-I'+str(ROOT/'validation/stubs'),'-I'+str(ROOT/'include'),str(source),'-o',str(dest)],check=True)
  lib=c.CDLL(str(dest));lib.cornix_steno_decode.argtypes=[c.c_uint64,c.c_uint64,c.POINTER(Dec)];lib.cornix_steno_decode.restype=c.c_int
  lib.cornix_steno_decode_correction_unit.argtypes=[c.c_uint64,c.POINTER(Dec)];lib.cornix_steno_decode_correction_unit.restype=c.c_int
  libs.append(lib)
 def run(lib,mask,pos):
  d=Dec();rc=lib.cornix_steno_decode(mask,pos,c.byref(d));return rc,d.kind,tuple(d.keys[:d.count])
 count=0;differences=set()
 def test(mask,pos):
  global count
  a,b=[run(lib,mask,pos) for lib in libs]
  if a!=b:
   assert mask in expected,(mask,pos,a,b)
   old,new=expected[mask];assert a==(0,1,(old,)) and b==(0,1,(new,)),(a,b)
   differences.add(mask)
  else:assert mask not in expected,(mask,a,b)
  for lib in libs:
   d=Dec();rc=lib.cornix_steno_decode_correction_unit(mask,c.byref(d))
   val=(rc,d.kind,tuple(d.keys[:d.count]))
   if lib is libs[0]:correction=val
   else:assert correction==val,(mask,correction,val)
  count+=1
 for width in [0,1,2,3,4]:
  for ids in itertools.combinations(range(1,43),width):
   mask=sum(1<<i for i in ids);pos=sum(1<<position[i] for i in ids)
   test(mask,pos)
 rng=random.Random(228)
 for _ in range(10000):
  mask=rng.getrandbits(42)<<1
  test(mask,sum(1<<position[i] for i in position if mask&(1<<i)))
 assert len(differences)==4
 print(f'PASS: {count} actual before/after decoder and correction cases; exactly four requested singleton differences; no other output change')

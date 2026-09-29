#!/usr/bin/env python3
"""Declarative 200ms shortcut specification model, NOT native ZMK execution.
Reads the delivered bindings. Covers modifier-FIRST + Tab and modifier key
interrupts. It does not claim validation of ZMK's capture queue, BLE timing,
Studio dynamic remapping, or an arbitrary Tab-first reverse-order gesture.
"""
from pathlib import Path
import re, sys, itertools
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from validate_keymap_update import bindings, node, normalized
source=(ROOT/'config/cornix.keymap').read_text()
BASE=bindings(source,'base')
MATCH={'LCTRL','RCTRL','LALT','RALT','LGUI'}
MODIFIERS=MATCH|{'LSHIFT','RSHIFT','RGUI'}
assert 'tapping-term-ms=<200>;' in normalized(node(source,'ht200').group())
assert 'flavor="tap-preferred";' in normalized(node(source,'ht200').group())

class Model:
    def __init__(self):
        self.time=0;self.keys={};self.mods=set();self.events=[];self.layers=set()
    def emit(self,key,pressed):
        if key in MODIFIERS:
            if pressed:self.mods.add(key)
            else:self.mods.discard(key)
        self.events.append((self.time,key,pressed,frozenset(self.mods)))
    def finish_hold(self,s):
        if s['pending']:
            s['pending']=False;s['output']=s['hold'];self.emit(s['output'],True)
    def advance(self,t):
        assert t>=self.time
        self.time=t
        for s in self.keys.values():
            if s.get('pending') and t-s['since']>=200:self.finish_hold(s)
    def down(self,p,t):
        self.advance(t);assert p not in self.keys
        # Standard hold-preferred decision happens before the interrupt key.
        for s in self.keys.values():
            if s.get('pending') and s['kind']=='mp200':self.finish_hold(s)
        parts=BASE[p].split();kind=parts[0][1:]
        if kind=='bang_tab':
            parts=['&kp','TAB'] if self.mods&MATCH else ['&ht200','TAB','EXCL']
            kind=parts[0][1:]
        if kind in ('mp200','ht200'):
            self.keys[p]={'kind':kind,'pending':True,'hold':parts[1],'tap':parts[2],'since':t,'output':None}
        elif kind=='kp':
            self.keys[p]={'kind':kind,'pending':False,'output':parts[1]};self.emit(parts[1],True)
        elif kind=='mo':
            self.keys[p]={'kind':kind,'layer':parts[1]};self.layers.add(parts[1])
        else:raise ValueError(f'outside tested subset: {BASE[p]}')
    def up(self,p,t):
        self.advance(t);s=self.keys.pop(p)
        if s['kind']=='mo':self.layers.discard(s['layer']);return
        if s.get('pending'):
            self.emit(s['tap'],True);self.emit(s['tap'],False)
        else:self.emit(s['output'],False)
    def chars(self):return [(k,mods) for _,k,p,mods in self.events if p and k not in MODIFIERS]

n=0
for duration in [0,1,49,199,200,201,500,1000]:
    m=Model();m.down(0,0);m.up(0,duration)
    assert m.chars()==[('EXCL' if duration<200 else 'TAB',frozenset())]
    n+=1
for p,mod in [(12,'LCTRL'),(38,'LGUI'),(39,'LALT'),(48,'RCTRL')]:
    for delay in [0,1,49,199,200,201,999]:
        for early_modifier_release in [False,True]:
            m=Model();m.down(p,0);m.down(0,delay)
            # Even an unresolved Ctrl/GUI hold-tap resolves BEFORE 00's morph.
            assert m.chars()==[('TAB',frozenset({mod}))],(p,delay,m.events)
            if early_modifier_release:m.up(p,delay+5);m.up(0,delay+20)
            else:m.up(0,delay+20);m.up(p,delay+25)
            assert len(m.chars())==1 and not m.mods and not m.keys
            n+=1
    m=Model();m.down(p,0)
    for i in range(10):m.down(0,30+i*30);m.up(0,40+i*30)
    m.up(p,400)
    assert m.chars()==[('TAB',frozenset({mod}))]*10 and not m.mods
    n+=1
# Every nonempty combination of explicit Ctrl/Alt/LeftGUI matches without
# removing the modifier state. Extra Shift remains present as well.
for width in range(1,6):
    for mods in itertools.combinations(sorted(MATCH),width):
        for shift in [False,True]:
            m=Model();m.mods=set(mods)|({'LSHIFT'} if shift else set())
            m.down(0,0);assert m.chars()==[('TAB',frozenset(m.mods))]
            m.up(0,10);n+=1
for p,mod,tap in [(24,'LSHIFT','LBKT'),(37,'RSHIFT','RBKT'),(12,'LCTRL','TILDE'),(38,'LGUI','CAPSLOCK'),(48,'RCTRL','RALT')]:
    for duration in [1,49,199,200,201,1000]:
        m=Model();m.down(p,0);m.up(p,duration)
        assert m.chars()==([] if duration>=200 or tap=='RALT' else [(tap,frozenset())])
        assert not m.mods and not m.keys;n+=1
    for delay in [0,1,49,199,200,250]:
        for release_modifier_first in [False,True]:
            m=Model();m.down(p,0);m.down(13,delay)
            assert m.chars()==[('A',frozenset({mod}))],m.events
            if release_modifier_first:m.up(p,delay+1);m.up(13,delay+2)
            else:m.up(13,delay+1);m.up(p,delay+2)
            assert not m.mods and len(m.chars())==1;n+=1
for p,layer in [(42,'MOUSE'),(45,'NAV')]:
    for duration in [1,199,200,999]:
        m=Model();m.down(p,0);assert m.layers=={layer};m.up(p,duration)
        assert not m.events and not m.layers;n+=1
for duration in [1,199,200,201,999]:
    m=Model();m.down(40,0);m.up(40,duration)
    assert m.chars()==[('LBKT' if duration<200 else 'LC(Z)',frozenset())];n+=1
m=Model();m.down(47,0);assert m.chars()==[('LC(Y)',frozenset())];m.up(47,10);n+=1
print(f'PASS: {n} binding-driven shortcut SPECIFICATION-MODEL cases (not native ZMK firmware execution)')
print('Coverage: 199/200ms, modifier-first Ctrl/Alt/GUI+Tab, repeat-Tab, modifier release order, bracket/Shift interrupts, layer-only keys')

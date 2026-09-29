#!/usr/bin/env python3
"""Rewrite the 12 standard ZMK quick macros from Korean/mixed text.

Edit config/steno_quick_text.tsv column 3, then run this script.  The script
assumes STENO starts in Korean. Contiguous ASCII letters/digits are surrounded
with RAlt toggles so mixed strings such as "W 시" work on a Korean Windows IME.
The macros remain ordinary zmk,behavior-macro nodes and can also be edited in
Nick Coutsos' ZMK Keymap Editor. Running this script later will overwrite GUI
changes to quick_0..quick_11 only.
"""
from __future__ import annotations
import csv, re
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
TEXT_TSV=ROOT/'config/steno_quick_text.tsv'
KEYMAP=ROOT/'config/cornix.keymap'
BEGIN='        /* BEGIN CORNIX QUICK MACROS (generated from config/steno_quick_text.tsv) */'
END='        /* END CORNIX QUICK MACROS */'
DISPLAY = ['L+EU','L+O','L+U','R+A','R+I','R+EO','L+A','L+EO','L+I','R+O','R+U','R+EU']

CHO_KEYS = [
    ['R'], ['LS(R)'], ['S'], ['E'], ['LS(E)'], ['F'], ['A'],
    ['Q'], ['LS(Q)'], ['T'], ['LS(T)'], ['D'], ['W'], ['LS(W)'],
    ['C'], ['Z'], ['X'], ['V'], ['G'],
]
JUNG_KEYS = [
    ['K'], ['O'], ['I'], ['LS(O)'], ['J'], ['P'], ['U'], ['LS(P)'],
    ['H'], ['H','K'], ['H','O'], ['H','L'], ['Y'], ['N'],
    ['N','J'], ['N','P'], ['N','L'], ['B'], ['M'], ['M','L'], ['L'],
]
JONG_KEYS = [
    [], ['R'], ['LS(R)'], ['R','T'], ['S'], ['S','W'], ['S','G'],
    ['E'], ['F'], ['F','R'], ['F','A'], ['F','Q'], ['F','T'],
    ['F','X'], ['F','V'], ['F','G'], ['A'], ['Q'], ['Q','T'],
    ['T'], ['LS(T)'], ['D'], ['W'], ['C'], ['Z'], ['X'], ['V'], ['G'],
]
PUNCT={
    ' ':['SPACE'], '\t':['TAB'], '\n':['ENTER'], ',':['COMMA'], '.':['DOT'],
    '?':['QUESTION'], '!':['EXCL'], '[':['LBKT'], ']':['RBKT'], '%':['PRCNT'],
    '^':['CARET'], "'":['SQT'], '"':['DQT'], '-':['MINUS'], '_':['UNDERSCORE'],
    ':':['COLON'], ';':['SEMI'], '/':['FSLH'], '\\':['BSLH'], '~':['TILDE'],
    '#':['HASH'], '(':['LPAR'], ')':['RPAR'],
}

def hangul_keys(ch:str)->list[str]:
    n=ord(ch)-0xAC00
    jong=n%28; jung=(n//28)%21; cho=n//(28*21)
    return CHO_KEYS[cho]+JUNG_KEYS[jung]+JONG_KEYS[jong]

def ascii_key(ch:str)->str:
    if 'a'<=ch<='z': return ch.upper()
    if 'A'<=ch<='Z': return f'LS({ch})'
    if '0'<=ch<='9': return f'N{ch}'
    raise ValueError(f'unsupported ASCII alphanumeric: {ch!r}')

def text_to_macro_keys(text:str)->list[str]:
    out=[]; latin=False
    def set_latin(on:bool):
        nonlocal latin
        if latin!=on:
            out.append('RALT')
            latin=on
    for ch in text:
        code=ord(ch)
        if 0xAC00<=code<=0xD7A3:
            set_latin(False); out.extend(hangul_keys(ch))
        elif ch.isascii() and ch.isalnum():
            set_latin(True); out.append(ascii_key(ch))
        elif ch in PUNCT:
            # Space and common punctuation are mode-independent on the target layout.
            out.extend(PUNCT[ch])
        else:
            raise ValueError(f'unsupported character U+{code:04X}: {ch!r}')
    set_latin(False)
    return out

def load_rows():
    rows=[]
    with TEXT_TSV.open(encoding='utf-8',newline='') as f:
        for ln,row in enumerate(csv.reader(f,delimiter='\t'),1):
            if not row or row[0].lstrip().startswith('#'): continue
            if len(row)!=3: raise ValueError(f'{TEXT_TSV}:{ln}: expected M#, chord, output')
            mid,chord,output=(x.strip() for x in row)
            if mid!=f'M{len(rows)}': raise ValueError(f'{TEXT_TSV}:{ln}: expected M{len(rows)}, got {mid}')
            if not output: raise ValueError(f'{TEXT_TSV}:{ln}: output is blank')
            rows.append((mid,chord,output))
    if len(rows)!=12: raise ValueError(f'expected 12 rows, found {len(rows)}')
    return rows

def make_block(rows):
    lines=[BEGIN,
           '        /*',
           '         * Standard ZMK macros visible in the Keymap Editor Macros panel.',
           '         * Fixed STENO input masks are dispatched to stable M0..M11 nodes.',
           '         * Edit the bindings in the GUI, or edit steno_quick_text.tsv and rerun',
           '         * tools/update_quick_macro_text.py.',
           '         */']
    for i,(mid,chord,output) in enumerate(rows):
        keys=text_to_macro_keys(output)
        bindings=' '.join(f'&kp {k}' for k in keys) if keys else '&none'
        has_hangul=any(0xAC00<=ord(ch)<=0xD7A3 for ch in output)
        has_ascii_word=any(ch.isascii() and ch.isalnum() for ch in output)
        wait_ms=80 if has_hangul and has_ascii_word else 40
        lines += [
            f'        /* {mid}: {chord} -> {output} */',
            f'        quick_{i}: quick_{i} {{',
            '            compatible = "zmk,behavior-macro";',
            '            #binding-cells = <0>;',
            f'            display-name = "{mid} Quick {DISPLAY[i]}";',
            f'            wait-ms = <{wait_ms}>;',
            '            tap-ms = <40>;',
            f'            bindings = <{bindings}>;',
            '        };','',
        ]
    if lines[-1]=='': lines.pop()
    lines.append(END)
    return '\n'.join(lines)

def main():
    rows=load_rows(); text=KEYMAP.read_text(encoding='utf-8')
    if BEGIN not in text or END not in text:
        raise RuntimeError('quick macro markers not found in cornix.keymap')
    before,rest=text.split(BEGIN,1)
    _,after=rest.split(END,1)
    KEYMAP.write_text(before+make_block(rows)+after,encoding='utf-8',newline='\n')
    print(f'updated 12 quick macros in {KEYMAP}')

if __name__=='__main__': main()

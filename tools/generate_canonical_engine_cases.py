#!/usr/bin/env python3
from __future__ import annotations
from pathlib import Path
import csv
root=Path(__file__).resolve().parents[1]
source=root/'config/steno_dictionary.tsv'
out=root/'validation/canonical_engine_cases.h'
ROLE_ENUM={n:f'CST_R_{n}' for n in [
'I_KH','I_B','I_J','I_H','I_N','I_D','I_S','I_G','I_DOUBLE','I_NG','I_P','I_CH','I_T','I_M','I_R',
'F_NG','F_D','F_B','F_M','F_T','F_R','F_DOUBLE','F_G','F_S','F_P','F_N','F_KH','F_H','F_J','F_CH',
'V_O','V_EU','V_U','V_A','V_I','V_EO','VEXT_L','VEXT_R','ABBR_L','ABBR_R','SYMBOL_L','SYMBOL_R']}
CHO=[['R'],['LS(R)'],['S'],['E'],['LS(E)'],['F'],['A'],['Q'],['LS(Q)'],['T'],['LS(T)'],['D'],['W'],['LS(W)'],['C'],['Z'],['X'],['V'],['G']]
JUNG=[['K'],['O'],['I'],['LS(O)'],['J'],['P'],['U'],['LS(P)'],['H'],['H','K'],['H','O'],['H','L'],['Y'],['N'],['N','J'],['N','P'],['N','L'],['B'],['M'],['M','L'],['L']]
JONG=[[],['R'],['LS(R)'],['R','T'],['S'],['S','W'],['S','G'],['E'],['F'],['F','R'],['F','A'],['F','Q'],['F','T'],['F','X'],['F','V'],['F','G'],['A'],['Q'],['Q','T'],['T'],['LS(T)'],['D'],['W'],['C'],['Z'],['X'],['V'],['G']]
ASCII={' ':['SPACE'],'\t':['TAB'],'\n':['ENTER'],',':['COMMA'],'.':['DOT'],'?':['QUESTION'],'!':['EXCL'],'[':['LBKT'],']':['RBKT'],'%':['PRCNT'],'^':['CARET'],"'":['SQT'],'"':['DQT'],'#':['HASH'],'~':['TILDE']}
def keys(text):
    out=[]
    for ch in text:
        n=ord(ch)-0xAC00
        if 0<=n<11172:
            jong=n%28; jung=(n//28)%21; cho=n//(28*21)
            out += CHO[cho]+JUNG[jung]+JONG[jong]
        elif ch in ASCII: out+=ASCII[ch]
        elif 'a'<=ch<='z': out.append(ch.upper())
        elif 'A'<=ch<='Z': out.append(f'LS({ch})')
        elif ch.isdigit(): out.append('N'+ch)
        else: raise ValueError(ch)
    return out
rows=[]
with source.open(encoding='utf-8',newline='') as f:
    for row in csv.reader(f,delimiter='\t'):
        if not row or row[0].startswith('#'): continue
        roles=[x.strip() for x in row[3].split('+')]
        rows.append((row[0],row[2],roles,keys(row[2])))
lines=['/* Auto-generated canonical engine cases; do not edit. */','#pragma once','',
'#define CST_CANONICAL_MAX_KEYS 16','#define CST_CANONICAL_MAX_ROLES 8','',
'struct cst_canonical_case { const char *id; const char *output; enum cornix_steno_role roles[CST_CANONICAL_MAX_ROLES]; uint8_t role_count; uint32_t keys[CST_CANONICAL_MAX_KEYS]; uint8_t key_count; };','',
'static const struct cst_canonical_case cst_canonical_cases[] = {']
for ident,text,roles,ks in rows:
    rr=', '.join(ROLE_ENUM[x] for x in roles)
    kk=', '.join(ks)
    lines.append(f'    {{"{ident}", "{text}", {{{rr}}}, {len(roles)}, {{{kk}}}, {len(ks)}}},')
lines += ['};',f'#define CST_CANONICAL_CASE_COUNT {len(rows)}u','']
out.write_text('\n'.join(lines),encoding='utf-8')
print(f'generated {len(rows)} canonical cases -> {out}')

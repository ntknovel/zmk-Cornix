#!/usr/bin/env python3
from pathlib import Path
import csv, sys
root=Path(__file__).resolve().parents[1]
layout=root/'config/steno_role_layout.tsv'
out=root/'validation/role_position_map.h'
roles={}
with layout.open(encoding='utf-8',newline='') as f:
    for row in csv.reader(f,delimiter='\t'):
        if not row or row[0].startswith('#'): continue
        roles[row[0].strip()]=int(row[1])
roles.update({
 'V_O':41,'V_EU':42,'V_U':43,'V_EO':44,'V_I':45,'V_A':46,
 'VEXT_L':24,'VEXT_R':37,'ABBR_L':12,'ABBR_R':23,
 'SYMBOL_L':40,'SYMBOL_R':47,
})
lines=['/* Auto-generated from config/steno_role_layout.tsv. */','#pragma once','',
       'static inline uint32_t cst_test_position_for_role(enum cornix_steno_role role) {','    switch (role) {']
for name,pos in sorted(roles.items(), key=lambda kv: kv[1]):
    lines.append(f'    case CST_R_{name}: return {pos}u;')
lines += ['    default: return UINT32_MAX;','    }','}','']
out.write_text('\n'.join(lines),encoding='utf-8')
print(f'generated {len(roles)} role positions -> {out}')

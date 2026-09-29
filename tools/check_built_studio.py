#!/usr/bin/env python3
"""Check a REAL build/zephyr/.config; does not build or invent firmware."""
from __future__ import annotations
import argparse, re
from pathlib import Path

def main() -> None:
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('config',type=Path)
    p.add_argument('side',choices=['left','right','reset'])
    a=p.parse_args()
    if not a.config.is_file(): raise SystemExit('FAIL: actual compiled .config does not exist')
    v={}
    for line in a.config.read_text().splitlines():
        if line.startswith('CONFIG_') and '=' in line:
            k,val=line.split('=',1);v[k]=val
        m=re.fullmatch(r'# (CONFIG_\w+) is not set',line)
        if m:v[m[1]]='n'
    if not v: raise SystemExit('FAIL: no Kconfig values found')
    want={'CONFIG_ZMK_STUDIO':'n','CONFIG_CORNIX_STENO_STUDIO_USB_AUTO_SELECT':'n'}
    if a.side=='left':
        want={
            'CONFIG_ZMK_STUDIO':'y','CONFIG_ZMK_STUDIO_RPC':'y','CONFIG_ZMK_BEHAVIOR_METADATA':'y',
            'CONFIG_ZMK_STUDIO_LOCKING':'n','CONFIG_ZMK_STUDIO_TRANSPORT_BLE':'n',
            'CONFIG_ZMK_STUDIO_TRANSPORT_UART':'y','CONFIG_ZMK_USB':'y','CONFIG_USB_CDC_ACM':'y',
            'CONFIG_ZMK_SPLIT_ROLE_CENTRAL':'y','CONFIG_CORNIX_STENO_STUDIO_USB_AUTO_SELECT':'y',
        }
    if a.side!='reset':
        want.update({'CONFIG_CORNIX_STENO':'y','CONFIG_CORNIX_STENO_LED':'y',
                     'CONFIG_CORNIX_STENO_KEYUP_CORRECTION_MS':'0','CONFIG_CORNIX_STENO_FINAL_ROLL_MS':'90',
                     'CONFIG_CORNIX_STENO_SOLO_CORRECTION_MS':'500','CONFIG_CORNIX_STENO_DUAL_TAPPING_TERM_MS':'200',
                     'CONFIG_BT_CTLR_TX_PWR_PLUS_8':'y'})
    else:want.update({'CONFIG_CORNIX_STENO':'n','CONFIG_CORNIX_STENO_LED':'n'})
    for k,w in want.items():
        actual=v.get(k,'n')
        if actual!=w:raise SystemExit(f'FAIL: {k}={actual}; expected {w}')
        print(f'PASS: {k}={actual}')
    print('Provided compiled configuration matches:',a.side)

if __name__=='__main__': main()

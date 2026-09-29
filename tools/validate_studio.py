#!/usr/bin/env python3
"""CORNIX STENO v2.2.9 source/host tests. Never claims an ARM build."""
from __future__ import annotations
import argparse, ast, hashlib, json, re, shlex, shutil, subprocess, sys, tempfile
from pathlib import Path
import yaml


def check(ok: bool, msg: str) -> None:
    if not ok: raise RuntimeError(msg)
    print('PASS:', msg, flush=True)


def run(args: list[str], **kwargs) -> None:
    subprocess.run(args, check=True, **kwargs)


def flags(row: dict) -> dict[str,str]:
    return dict(v.removeprefix('-D').split('=', 1) for v in shlex.split(row.get('cmake-args','')))


def static_checks(root: Path) -> None:
    for p in list((root/'tools').glob('*.py')) + list((root/'validation').glob('*.py')):
        ast.parse(p.read_text(encoding='utf-8'))
    check(True, 'Python syntax')
    refs=json.loads((root/'STUDIO_PRESERVED_V2_2_6_SHA256.json').read_text())
    changes=json.loads((root/'KEYMAP_UPDATE_v2_2_9.json').read_text())['requested_runtime_changes']
    for rel,h in refs.items():
        actual=hashlib.sha256((root/rel).read_bytes()).hexdigest()
        if rel in changes:
            entry=changes[rel]
            check(hashlib.sha256((root/entry['reference']).read_bytes()).hexdigest()==h==entry['baseline_sha256'],
                  'original preserved evidence for requested change: '+rel)
            check(actual==entry['updated_sha256'], 'approved v2.2.9 requested change: '+rel)
        else:
            check(actual==h,'v2.2.6 bytes preserved: '+rel)
    cfg=json.loads((root/'STUDIO_BASELINE_AUDIT.json').read_text())
    for rel,entry in cfg['metadata_only_changes'].items():
        text=(root/rel).read_text().replace('#include <cornix_steno/studio_metadata.h>\n','')
        text=text.replace('.get_parameter_metadata = '+entry['callback']+',',
                          '.get_parameter_metadata = zmk_behavior_get_empty_param_metadata,')
        check(hashlib.sha256(text.encode()).hexdigest()==entry['baseline_sha256'],
              'metadata-only difference, original handlers unchanged: '+rel)
    for rel,marker in [('CMakeLists.txt','\n# CORNIX_STENO_USB_STUDIO_V2_2_7'),
                       ('Kconfig','\n# Studio is enabled only')]:
        original=(root/rel).read_text().split(marker)[0]
        # One separating newline was appended, preserving the entire old prefix.
        check(hashlib.sha256(original[:cfg['original_prefixes'][rel]['length']].encode()).hexdigest()
              ==cfg['original_prefixes'][rel]['sha256'], 'original config/source gates preserved: '+rel)
    rows=yaml.safe_load((root/'build.yaml').read_text())['include']
    check(len(rows)==3,'three isolated build targets')
    left,right,reset=rows
    check(left['board']=='cornix_left//zmk' and left['snippet']=='studio-rpc-usb-uart','left USB Studio snippet')
    check(flags(left)=={'CONFIG_ZMK_STUDIO':'y','CONFIG_ZMK_STUDIO_LOCKING':'n',
                      'CONFIG_ZMK_STUDIO_TRANSPORT_BLE':'n',
                      'CONFIG_CORNIX_STENO_STUDIO_USB_AUTO_SELECT':'y'}, 'left unlocked USB-only Studio flags')
    for row in [left,right]: check(row['shield']=='cornix_indicator cornix_steno_led', 'production indicator + STENO shield preserved')
    check(right['board']=='cornix_right//zmk' and not right.get('snippet') and flags(right)==
          {'CONFIG_ZMK_STUDIO':'n','CONFIG_CORNIX_STENO_STUDIO_USB_AUTO_SELECT':'n'},'right Studio off, split unchanged')
    check(reset['board']=='cornix_right//zmk' and reset['shield']=='settings_reset' and
          reset['snippet']=='studio-rpc-usb-uart nrf52840-nosd' and flags(reset)==
          {'CONFIG_ZMK_STUDIO':'n','CONFIG_CORNIX_STENO':'n','CONFIG_CORNIX_STENO_STUDIO_USB_AUTO_SELECT':'n'},
          'reset has no STENO/Studio server; original reset USB/partition snippets')
    check(len({r['artifact-name'] for r in rows})==3,'unique artifact names')
    usb=(root/'src/studio/usb_auto_select.c').read_text()
    check('k_work_reschedule' in usb and 'k_sleep' not in usb and 'k_busy_wait' not in usb,
          'USB selection deferred; no sleeping in key processing')
    for n in ['get_parameter_metadata','cornix_steno_studio_hid_metadata']:
        check(n in (root/'src/behaviors/behavior_steno_arrow.c').read_text(),'arrow metadata: '+n)
    check('cornix_steno_studio_led_metadata' in (root/'src/behaviors/behavior_steno_led.c').read_text(),
          'packed LED has u8 metadata, not empty metadata')
    for p in (root/'src/behaviors').glob('*.c'):
        check('CONFIG_ZMK_BEHAVIOR_METADATA' in p.read_text(), 'metadata branch exists: '+p.name)


def cmake_checks(root: Path, tmp: Path) -> None:
    cases=['new','old','right','reset','locking','disconnect_lock','ble','no_uart','no_steno','bad_api']
    for case in cases:
        d=tmp/('cmake_'+case); d.mkdir()
        inc=d/'zmkapp/include/zmk';inc.mkdir(parents=True)
        api='zmk_endpoints_select_transport' if case=='old' else 'zmk_endpoint_set_preferred_transport'
        (inc/'endpoints.h').write_text('' if case=='bad_api' else 'int '+api+'(enum zmk_transport transport);\n')
        body='cmake_minimum_required(VERSION 3.20)\nproject(cornix_config_fixture C)\nadd_library(app STATIC dummy.c)\n'
        body+='function(target_sources_ifdef flag)\n if(${flag})\n target_sources(${ARGN})\n endif()\nendfunction()\n'
        body+='set(APPLICATION_SOURCE_DIR "'+str(d/'zmkapp')+'")\n'
        enabled=case not in ['right','reset']
        for flag in ['CORNIX_STENO_STUDIO_USB_AUTO_SELECT','ZMK_STUDIO','ZMK_STUDIO_RPC','ZMK_USB',
                     'ZMK_SPLIT_ROLE_CENTRAL','ZMK_STUDIO_TRANSPORT_UART']:
            body+='set(CONFIG_'+flag+' '+('TRUE' if enabled else 'FALSE')+')\n'
        body+='set(CONFIG_ZMK_SPLIT TRUE)\n'
        for flag in ['CORNIX_STENO','CORNIX_STENO_LED','CORNIX_STENO_DICTIONARY']:
            body+='set(CONFIG_'+flag+' '+('FALSE' if case=='reset' else 'TRUE')+')\n'
        for match,flag in [('locking','ZMK_STUDIO_LOCKING'),('disconnect_lock','ZMK_STUDIO_LOCK_ON_DISCONNECT'),
                           ('ble','ZMK_STUDIO_TRANSPORT_BLE')]:
            if case==match: body+='set(CONFIG_'+flag+' TRUE)\n'
        if case=='no_uart': body+='set(CONFIG_ZMK_STUDIO_TRANSPORT_UART FALSE)\n'
        if case=='no_steno': body+='set(CONFIG_CORNIX_STENO FALSE)\n'
        body+='add_subdirectory("'+str(root)+'" firmware)\n'
        body+='get_target_property(s app SOURCES)\nfile(WRITE "${CMAKE_BINARY_DIR}/sources.txt" "${s}")\n'
        body+='get_target_property(d app COMPILE_DEFINITIONS)\nfile(WRITE "${CMAKE_BINARY_DIR}/definitions.txt" "${d}")\n'
        (d/'CMakeLists.txt').write_text(body); (d/'dummy.c').write_text('int dummy;\n')
        p=subprocess.run(['cmake','-S',str(d),'-B',str(d/'build')],capture_output=True,text=True)
        success=case in ['new','old','right','reset']
        check((p.returncode==0)==success,'CMake fixture: '+case)
        if success:
            sources=(d/'build/sources.txt').read_text()
            check(('usb_auto_select.c' in sources)==enabled,'central-only USB helper: '+case)
            check(('compile_checks.c' in sources)==enabled,'central-only Studio compile guards: '+case)
            check(('steno_engine.c' in sources)==enabled,'central-only STENO engine: '+case)
            check(('steno_led.c' in sources)==(case!='reset'),'left/right indicator, reset excluded: '+case)
            if enabled: check(api in (d/'build/definitions.txt').read_text(),'actual declared API chosen: '+case)
        else:
            check('CORNIX STENO:' in p.stdout+p.stderr,'configuration error from expected guard: '+case)


def physical_layout_guard_tests(root: Path, tmp: Path) -> None:
    d=tmp/'layout'; (d/'zephyr/sys').mkdir(parents=True)
    (d/'zephyr/sys/util.h').write_text('#define IS_ENABLED(x) (x)\n#define BUILD_ASSERT(x,m) _Static_assert((x),m)\n')
    names=['CORNIX_STENO','CORNIX_STENO_LED','ZMK_STUDIO','ZMK_STUDIO_RPC','ZMK_BEHAVIOR_METADATA',
           'ZMK_STUDIO_TRANSPORT_UART','USB_CDC_ACM','ZMK_USB','ZMK_SPLIT_ROLE_CENTRAL']
    flags0=[f'-DCONFIG_{n}=1' for n in names]+['-DCONFIG_ZMK_STUDIO_LOCKING=0','-DCONFIG_ZMK_STUDIO_TRANSPORT_BLE=0']
    for case in ['good50','no_layout','old_chosen_transform','no_geometry','wrong_key_count']:
        text='#define DT_HAS_CHOSEN(n) HAS_##n\n#define DT_CHOSEN(n) LAYOUT\n'
        text+=f'#define HAS_zmk_physical_layout {int(case!="no_layout")}\n'
        text+=f'#define HAS_zmk_matrix_transform {int(case=="old_chosen_transform")}\n'
        text+=f'#define DT_NODE_HAS_PROP(n,p) {int(case!="no_geometry")}\n'
        text+=f'#define DT_PROP_LEN(n,p) {49 if case=="wrong_key_count" else 50}\n'
        (d/'zephyr/devicetree.h').write_text(text)
        p=subprocess.run(['gcc','-std=c11','-Wall','-Wextra','-Werror','-I'+str(d),*flags0,'-fsyntax-only',
                          str(root/'src/studio/compile_checks.c')],capture_output=True,text=True)
        check((p.returncode==0)==(case=='good50'),'physical-layout compile guard fixture: '+case)


def host_checks(root: Path) -> None:
    with tempfile.TemporaryDirectory(prefix='cornix-studio-') as td:
        tmp=Path(td)
        cfg=(root/'validation/api_stubs/config.h').read_text().replace('#define CONFIG_ZMK_BEHAVIOR_METADATA 0',
                                                                       '#define CONFIG_ZMK_BEHAVIOR_METADATA 1')
        (tmp/'metadata_config.h').write_text(cfg)
        for cc in ['gcc','clang']:
            if not shutil.which(cc): raise RuntimeError('Required host compiler missing: '+cc)
            san=['-fsanitize=address,undefined','-fno-omit-frame-pointer'] if cc=='clang' else []
            for api in ['zmk_endpoint_set_preferred_transport','zmk_endpoints_select_transport']:
                out=tmp/(cc+'_'+api)
                old=['-DCORNIX_TEST_OLD_API=1'] if api=='zmk_endpoints_select_transport' else []
                run([cc,'-std=c11','-Wall','-Wextra','-Werror','-fno-strict-aliasing',*san,*old,
                     '-I'+str(root/'validation/studio_stubs'),'-DCORNIX_SELECT_USB_TRANSPORT='+api,
                     str(root/'validation/test_studio_usb.c'),'-o',str(out)])
                run([str(out)]);check(True,cc+' USB module actual host execution: '+api)
            out=tmp/(cc+'_metadata')
            run([cc,'-std=c11','-Wall','-Wextra','-Werror',*san,
                 '-include',str(tmp/'metadata_config.h'),'-I'+str(root/'validation/api_stubs'),
                 '-I'+str(root/'include'),str(root/'validation/test_studio_metadata.c'),'-o',str(out)])
            run([str(out)])
            for source in sorted((root/'src/behaviors').glob('*.c')):
                run([cc,'-std=c11','-Wall','-Wextra','-Werror','-Wno-unused-function','-Wno-unused-variable',
                     '-include',str(tmp/'metadata_config.h'),'-I'+str(root/'validation/api_stubs'),
                     '-I'+str(root/'include'),'-fsyntax-only',str(source)])
            check(True,cc+' metadata-enabled syntax: 10 actual behavior drivers')
        cmake_checks(root,tmp)
        physical_layout_guard_tests(root,tmp)


def main() -> None:
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('root',nargs='?',type=Path,default=Path('.'))
    root=p.parse_args().root.resolve()
    static_checks(root);host_checks(root)
    print('SOURCE/HOST STUDIO TESTS PASSED. NOT an ARM build or USB hardware test.',flush=True)

if __name__=='__main__':
    try: main()
    except (RuntimeError, OSError, ValueError, subprocess.CalledProcessError) as e:
        print('FAIL:',e,file=sys.stderr);raise SystemExit(1)

#!/usr/bin/env python3
"""Static and host validation for the Cornix ZMK BASE+STENO overlay."""
from __future__ import annotations

import argparse
import hashlib
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

try:
    import yaml
except ImportError:  # pragma: no cover
    yaml = None

LAYER_NAMES = ["base", "num", "mouse", "nav", "fn", "symbol", "steno", "select_nav"]
EXPECTED_POSITION_BEHAVIORS = {
    "base": {11: "kp", 30: "st_mode", 31: "st_lang_return", 40: "ht200", 47: "ht200"},
    "num": {30: "kp", 31: "kp"},
    "mouse": {30: "kp", 31: "kp"},
    "nav": {30: "none", 31: "none"},
    "fn": {30: "kp", 31: "kp"},
    "symbol": {30: "kp", 31: "kp"},
    "steno": {11: "ht150", 30: "st_mode", 31: "st_lang_return", 47: "st_symbol_r", 48: "select_shift", 49: "kp"},
    "select_nav": {8: "kp", 19: "kp", 20: "kp", 21: "kp"},
}
EXPECTED_STENO_BEHAVIORS = {
    0:"st_tab_once",1:"st_i_b",2:"st_i_j",3:"st_i_d",4:"st_i_g",5:"st_i_s",
    6:"st_f_s",7:"st_f_g",8:"st_f_d",9:"st_f_j",10:"st_f_b",11:"ht150",
    12:"st_abbr_l",13:"st_i_m",14:"st_i_n",15:"st_i_ng",16:"st_i_r",17:"st_i_h",
    18:"st_f_h",19:"st_f_r",20:"st_f_ng",21:"st_f_n",22:"st_f_m",23:"st_abbr_r",
    24:"st_vext_l",25:"st_i_kh",26:"st_i_t",27:"st_i_ch",28:"st_i_p",29:"st_i_double",
    30:"st_mode",31:"st_lang_return",32:"st_f_double",33:"st_f_p",34:"st_f_ch",35:"st_f_t",36:"st_f_kh",37:"st_vext_r",
    38:"kp",39:"kp",40:"st_symbol_l",41:"st_v_o",42:"st_v_eu",43:"st_vu_enter",
    44:"st_veo_space",45:"st_v_i",46:"st_v_a",47:"st_symbol_r",48:"select_shift",49:"kp",
}

EXPECTED_SELECT_NAV = ["none"]*50
for _p in (8,19,20,21): EXPECTED_SELECT_NAV[_p]="kp"
EXPECTED_ROLE_LAYOUT = {
    "I_B":(1,"LP"),"I_J":(2,"LR"),"I_D":(3,"LM"),"I_G":(4,"LI"),"I_S":(5,"LI"),
    "F_S":(6,"RI"),"F_G":(7,"RI"),"F_D":(8,"RM"),"F_J":(9,"RR"),"F_B":(10,"RP"),
    "I_M":(13,"LP"),"I_N":(14,"LR"),"I_NG":(15,"LM"),"I_R":(16,"LI"),"I_H":(17,"LI"),
    "F_H":(18,"RI"),"F_R":(19,"RI"),"F_NG":(20,"RM"),"F_N":(21,"RR"),"F_M":(22,"RP"),
    "I_KH":(25,"LP"),"I_T":(26,"LR"),"I_CH":(27,"LM"),"I_P":(28,"LI"),"I_DOUBLE":(29,"LI"),
    "F_DOUBLE":(32,"RI"),"F_P":(33,"RI"),"F_CH":(34,"RM"),"F_T":(35,"RR"),"F_KH":(36,"RP"),
}



def fail(msg: str) -> None:
    raise RuntimeError(msg)


def read(path: Path) -> str:
    if not path.is_file():
        fail(f"missing required file: {path}")
    return path.read_text(encoding="utf-8")


def extract_layer_bindings(keymap: str, name: str) -> str:
    pattern = rf"\b{name}_layer\s*\{{(?:(?!\n\s*\w+_layer\s*\{{).)*?\bbindings\s*=\s*<(.*?)>;"
    match = re.search(pattern, keymap, re.S)
    if not match:
        fail(f"could not find {name}_layer bindings")
    return match.group(1)


def validate_yaml_files(root: Path) -> int:
    if yaml is None:
        fail("PyYAML is required for package validation")
    files = [root / ".github/workflows/build.yml", root / "zephyr/module.yml", root / "build.yaml"]
    files.extend(sorted((root / "dts/bindings/behaviors").glob("*.yaml")))
    for path in files:
        with path.open("r", encoding="utf-8") as f:
            yaml.safe_load(f)
    return len(files)


def validate_keymap(root: Path) -> dict[str, list[str]]:
    keymap = read(root / "config/cornix.keymap")
    if "#define STENO   6" not in keymap or "#define SELECT_NAV 7" not in keymap:
        fail("STENO=6 and SELECT_NAV=7 are required")
    if '#include "cornix_steno.dtsi"' not in keymap:
        fail("cornix_steno.dtsi is not included")
    layers={}
    for name in LAYER_NAMES:
        block=extract_layer_bindings(keymap,name)
        behaviors=re.findall(r"&([A-Za-z_][A-Za-z0-9_]*)",block)
        if len(behaviors)!=50: fail(f"{name} layer has {len(behaviors)} bindings, expected 50")
        layers[name]=behaviors
    for layer,expected in EXPECTED_POSITION_BEHAVIORS.items():
        for position,behavior in expected.items():
            if layers[layer][position]!=behavior:
                fail(f"{layer} position {position}: expected &{behavior}, found &{layers[layer][position]}")
    if layers["steno"] != [EXPECTED_STENO_BEHAVIORS[i] for i in range(50)]:
        fail("STENO layer does not match the finalized Cornix layout")
    if layers["select_nav"] != EXPECTED_SELECT_NAV:
        fail("Select Nav must expose only physical I/J/K/L as arrows")
    normalized=" ".join(keymap.split())
    required=[
        "&kp P &kp LC(A)", "&kp LCTRL &kp LALT &ht200 LC(Y) LBKT",
        "&lt250 FN K_APP &ht200 RCTRL RBKT &kp RALT &kp BSPC",
        "&st_mode &st_lang_return", "&kp C_STOP &kp C_PLAY_PAUSE", "&kp F11 &kp F12",
        "&st_f_b &ht150 RCTRL RALT", "&st_mode &st_lang_return &st_f_double",
        "&kp LCTRL &kp LALT &st_symbol_l", "&st_v_a &st_symbol_r &select_shift &kp BSPC",
        "sensor-bindings = <&st_enc LS(UP) LS(DOWN) &st_enc UP DOWN>",
        "select_shift: select_shift", "&macro_press &mo SELECT_NAV &kp RSHIFT",
        "&kp LEFT &kp DOWN &kp RIGHT", "&rfm &st_led_toggle",
    ]
    for text in required:
        if text not in normalized: fail(f"required keymap sequence missing: {text}")
    if keymap.count("&st_mode")!=2: fail("&st_mode must appear twice")
    if keymap.count("&st_lang_return")!=2:
        fail("&st_lang_return must appear on BASE and STENO right encoder clicks")
    if re.search(r"&st_lang(?:\s|$)", keymap):
        fail("legacy one-shot &st_lang remains in the keymap")
    if keymap.count("&st_led_toggle")!=1: fail("&st_led_toggle must appear once")
    return layers


def validate_kconfig(root: Path) -> int:
    kconfig = read(root / "Kconfig")
    shared_conf = read(root / "config/cornix.conf")
    steno_conf = read(root / "config/boards/shields/cornix_steno_led/cornix_steno_led.conf")
    if re.search(r"^CONFIG_CORNIX_STENO", shared_conf, re.M):
        fail("shared cornix.conf must not enable STENO; settings_reset would inherit it")
    conf = shared_conf + "\n" + steno_conf
    assigned = sorted(set(re.findall(r"^CONFIG_(CORNIX_STENO(?:_[A-Z0-9_]+)?)=", conf, re.M)))
    for symbol in assigned:
        if symbol == "CORNIX_STENO":
            pattern = rf"menuconfig\s+{symbol}\s*\n\s*bool\s+\""
        else:
            pattern = rf"config\s+{symbol}\s*\n\s*(?:bool|int)\s+\""
        if not re.search(pattern, kconfig):
            fail(f"Kconfig symbol CONFIG_{symbol} is assigned but has no user-visible prompt")
    if "CONFIG_CORNIX_STENO_MAX_POSITIONS=50" not in steno_conf:
        fail("Cornix fixed 50-position build should use MAX_POSITIONS=50")
    for token in ("CONFIG_CORNIX_STENO_KEYUP_CORRECTION_MS=50",
                  "CONFIG_CORNIX_STENO_FINAL_ROLL_MS=90"):
        if token not in steno_conf:
            fail(f"STENO timing config missing: {token}")
    for forbidden in ("CORNIX_STENO_CHORD_SETTLE_MS", "CORNIX_STENO_LATE_TAP_MAX_MS"):
        if forbidden in kconfig or forbidden in steno_conf:
            fail(f"legacy partial key-up timing remains: {forbidden}")
    if not re.search(r"menuconfig\s+CORNIX_STENO.*?default\s+n", kconfig, re.S):
        fail("CORNIX_STENO must default to n so settings_reset stays minimal")
    return len(assigned)


def validate_role_layout(root: Path) -> None:
    path = root / "config/steno_role_layout.tsv"
    actual: dict[str, tuple[int, str]] = {}
    for line_no, raw in enumerate(read(path).splitlines(), 1):
        if not raw.strip() or raw.lstrip().startswith("#"):
            continue
        cells = raw.split("\t")
        if len(cells) < 3:
            fail(f"{path}:{line_no}: expected role, position, finger")
        role, position, finger = cells[0], int(cells[1]), cells[2]
        actual[role] = (position, finger)
    if actual != EXPECTED_ROLE_LAYOUT:
        missing = sorted(set(EXPECTED_ROLE_LAYOUT) - set(actual))
        extra = sorted(set(actual) - set(EXPECTED_ROLE_LAYOUT))
        wrong = sorted(
            role for role in set(actual) & set(EXPECTED_ROLE_LAYOUT)
            if actual[role] != EXPECTED_ROLE_LAYOUT[role]
        )
        fail(
            "final STENO role-layout mismatch: "
            f"missing={missing}, extra={extra}, wrong={wrong}"
        )


def validate_dictionary(root: Path) -> int:
    generator = root / "tools/generate_steno_dictionary.py"
    source = root / "config/steno_dictionary.tsv"
    layout = root / "config/steno_role_layout.tsv"
    generated = root / "src/steno/steno_dictionary_generated.c"
    with tempfile.TemporaryDirectory() as td:
        temp = Path(td) / "dictionary.c"
        subprocess.run(
            [sys.executable, str(generator), str(source), str(temp), "--layout", str(layout)],
            check=True,
            text=True,
            capture_output=True,
        )
        if temp.read_bytes() != generated.read_bytes():
            fail("generated dictionary source is stale; regenerate it before packaging")
    dictionary_rows = [line.split("\t") for line in source.read_text(encoding="utf-8").splitlines()
                       if line.strip() and not line.lstrip().startswith("#")]
    count = len(dictionary_rows)
    required_outputs = {
        "그런", "그럼", "그리고", "그렇게", "였다", "없다", "었다",
        "이었다", "였었다", "했다", "는데", "습니다", "하는게",
    }
    outputs = {row[2] for row in dictionary_rows if len(row) >= 4}
    missing = sorted(required_outputs - outputs)
    if missing:
        fail(f"required canonical abbreviation entries are missing: {missing}")
    if count != 43:
        fail(f"expected 43 canonical dictionary rows, found {count}")

    source_text = source.read_text(encoding="utf-8")
    if re.search(r"(?m)^.*\t[12]\t(?:I_|F_)", source_text):
        fail("legacy banked dictionary rows remain in the canonical source")
    generated_text = generated.read_text(encoding="utf-8")
    for token in [
        "cornix_steno_dictionary_normalize_mask",
        "cornix_steno_dictionary_lookup(uint64_t role_mask)",
        "cornix_steno_dictionary_has_exact",
        "cornix_steno_dictionary_has_prefix",
        ".category = CST_ABBR_CATEGORY_",
        "ABBR_L/R distinct",
        "side-specific 였었다",
    ]:
        if token not in generated_text:
            fail(f"canonical dictionary generator output missing: {token}")
    if ".bank" in generated_text or "uint8_t bank" in generated_text:
        fail("legacy dictionary bank field remains in generated C")

    engine_text = read(root / "src/steno/steno_engine.c")
    for token in (
        "dictionary_anchor_active_locked", "cornix_steno_dictionary_has_prefix",
        "CST_STREAM_NAV_LEFT", "CST_STREAM_SELECT_RIGHT", "CST_STREAM_CORRECTION",
        "is_valid_final_roll_pair", "CONFIG_CORNIX_STENO_FINAL_ROLL_MS",
        "cornix_steno_decode_correction_unit", "CST_NAV_VOWEL_POSITION",
    ):
        if token not in engine_text:
            fail(f"v1.8.0 full-parity engine support missing: {token}")


    quick_gen = root / "tools/generate_steno_quick.py"
    quick_src = root / "config/steno_quick_abbreviations.tsv"
    quick_c = root / "src/steno/steno_quick_generated.c"
    with tempfile.TemporaryDirectory() as td:
        temp = Path(td) / "quick.c"
        subprocess.run([sys.executable, str(quick_gen), str(quick_src), str(temp)],
                       check=True, text=True, capture_output=True)
        if temp.read_bytes() != quick_c.read_bytes():
            fail("generated quick-slot source is stale")
    quick_rows=[line for line in quick_src.read_text(encoding="utf-8").splitlines()
                if line.strip() and not line.lstrip().startswith("#")]
    if len(quick_rows) != 12:
        fail(f"expected 12 quick slots, found {len(quick_rows)}")
    quick_text = root / "config/steno_quick_text.tsv"
    quick_text_rows = [line for line in quick_text.read_text(encoding="utf-8").splitlines()
                       if line.strip() and not line.lstrip().startswith("#")]
    if len(quick_text_rows) != 12:
        fail(f"expected 12 quick text presets, found {len(quick_text_rows)}")
    if not (root / "tools/update_quick_macro_text.py").is_file():
        fail("text-to-macro quick updater is missing")
    return count



def validate_led_integration(root: Path) -> None:
    build = read(root / "build.yaml")
    if build.count("shield: cornix_indicator cornix_steno_led") != 2:
        fail("left and right normal builds must combine cornix_indicator + cornix_steno_led")
    if "shield: settings_reset" not in build:
        fail("settings-reset build is missing")

    west = read(root / "config/west.yml")
    for token in [
        "name: zmk-rgbled-widget",
        "revision: 7cf90cd829a2f15772f47f70d04780118510e20b",
    ]:
        if token not in west:
            fail(f"official Cornix indicator dependency missing: {token}")

    shield = root / "config/boards/shields/cornix_steno_led"
    for name in ["Kconfig.shield", "Kconfig.defconfig", "cornix_steno_led.conf",
                 "cornix_steno_led.overlay"]:
        if not (shield / name).is_file():
            fail(f"STENO LED shield file missing: {name}")

    conf = read(shield / "cornix_steno_led.conf")
    for token in [
        "CONFIG_CORNIX_STENO_LED=y",
        "CONFIG_CORNIX_STENO_LED_ENTRY_FLASH_MS=600",
        "CONFIG_CORNIX_STENO_LED_REASSERT_DELAY_MS=40",
        "CONFIG_CORNIX_STENO_LED_CATEGORY_FLASH_MS=350",
        "CONFIG_RGBLED_WIDGET_SHOW_CAPSLOCK=n",
        "CONFIG_RGBLED_WIDGET_SHOW_LAYER_CHANGE=n",
        "CONFIG_RGBLED_WIDGET_SHOW_LAYER_COLORS=n",
        "CONFIG_RGBLED_WIDGET_BATTERY_LEVEL_CRITICAL=20",
    ]:
        if token not in conf:
            fail(f"STENO LED companion config missing: {token}")
    for forbidden in ["CONFIG_SPI=y", "CONFIG_LED_STRIP=y", "CONFIG_ZMK_EXT_POWER=y"]:
        if forbidden in conf:
            fail(f"STENO companion must not duplicate cornix_indicator hardware ownership: {forbidden}")

    overlay = read(shield / "cornix_steno_led.overlay")
    for forbidden in ["&spi3", "worldsemi,ws2812-spi", "EXT_POWER", "status-ws2812"]:
        if forbidden in overlay:
            fail(f"STENO companion overlay must not redefine production indicator hardware: {forbidden}")

    dtsi = read(root / "config/cornix_steno.dtsi")
    for token in [
        "st_led: st_led",
        'compatible = "zmk,behavior-cornix-steno-led"',
        "st_led_toggle: st_led_toggle",
        'compatible = "zmk,behavior-cornix-steno-led-toggle"',
    ]:
        if token not in dtsi:
            fail(f"STENO LED behavior definition missing: {token}")
    if not re.search(r"st_led:\s*st_led\s*\{.*?#binding-cells\s*=\s*<1>;", dtsi, re.S):
        fail("global STENO LED behavior must carry one packed-state parameter")

    engine = read(root / "src/steno/steno_engine.c")
    for token in [
        "#include <cornix_steno/led.h>",
        "cornix_steno_led_mode_changed(active)",
        "cornix_steno_led_reset_keys()",
        "cornix_steno_led_confirm_category",
    ]:
        if token not in engine:
            fail(f"STENO engine LED integration missing: {token}")

    for rel in [
        "src/behaviors/behavior_steno_role.c",
        "src/behaviors/behavior_steno_dual.c",
        "src/behaviors/behavior_steno_tab.c",
    ]:
        text = read(root / rel)
        for token in ["cornix_steno_led_key_pressed", "cornix_steno_led_key_released"]:
            if token not in text:
                fail(f"{rel} does not report physical held-key state: {token}")

    hardware = read(root / "src/steno/steno_led.c")
    led_header = read(root / "include/cornix_steno/led.h")
    led_integration = hardware + "\n" + led_header
    for token in [
        "#include <zmk_rgbled_widget/widget.h>",
        "ws2812_set_status_led",
        "ws2812_clear_status_led",
        "ws2812_clear_led",
        "indicate_battery()",
        "indicate_connectivity()",
        "master_physical_down_mask",
        "CST_TOTAL_LED_COUNT",
        "CONFIG_CORNIX_STENO_LED_ENTRY_FLASH_MS",
        "CONFIG_CORNIX_STENO_LED_CATEGORY_FLASH_MS",
        "zmk_behavior_invoke_binding",
    ]:
        if token not in led_integration:
            fail(f"STENO LED widget integration missing: {token}")
    for forbidden in ["led_strip_update_rgb", "GPIO_DT_SPEC_GET", "gpio_pin_set_dt"]:
        if forbidden in hardware:
            fail(f"STENO LED code must use the production widget, not raw hardware: {forbidden}")

    behavior = read(root / "src/behaviors/behavior_steno_led.c")
    if "BEHAVIOR_LOCALITY_GLOBAL" not in behavior:
        fail("STENO LED state sync behavior must be global")
    toggle = read(root / "src/behaviors/behavior_steno_led_toggle.c")
    if "BEHAVIOR_LOCALITY_CENTRAL" not in toggle or "cornix_steno_led_toggle" not in toggle:
        fail("STENO LED On/Off behavior must execute once on the central")

    kconfig = read(root / "Kconfig")
    if "depends on RGBLED_WIDGET_WS2812" not in kconfig:
        fail("CORNIX_STENO_LED must depend on the production WS2812 widget")


def validate_release_docs(root: Path) -> None:
    version = read(root / "PACKAGE_VERSION.txt")
    readme = read(root / "README_KO.md")
    quick = read(root / "README_FIRST_KO.txt")
    audit = read(root / "FULL_PARITY_v1.8.0_KO.md")
    phantom = read(root / "CORNIX_STENO_PHANTOM_ROLE_AUDIT_v1.8.0_KO.md")
    if "v2.2.2" not in version:
        fail("release version metadata missing: v2.2.2")
    for token in (
        "약어L + 전용쌍초", "약어R + 전용쌍종",
        "기호L + 전용쌍초", "기호R + 전용쌍종",
        "80ms", "12,383",
    ):
        if token not in readme and token not in quick:
            fail(f"release documentation missing current behavior: {token}")
    for token in ("v1.6.3 약어 앵커", "v1.7.1 tail remap", "팬텀 역할 감사"):
        if token not in audit:
            fail(f"patch audit missing: {token}")
    for token in ("12,383", "종ㄷ+종ㅂ", "ZMK 표준 GPIO matrix driver"):
        if token not in phantom:
            fail(f"phantom audit missing: {token}")


def validate_sources(root: Path) -> None:
    keymap = read(root / "config/cornix.keymap")
    for label in [f"quick_{i}" for i in range(12)]:
        if not re.search(rf"{label}:\s*{label}\s*\{{.*?compatible\s*=\s*\"zmk,behavior-macro\";", keymap, re.S):
            fail(f"GUI quick macro node missing: {label}")
    dispatch = read(root / "src/steno/steno_quick_dispatch.c")
    for label in [f"quick_{i}" for i in range(12)]:
        if f"DT_NODELABEL({label})" not in dispatch:
            fail(f"quick macro dispatch missing node label: {label}")
    decoder = read(root / "src/steno/steno_decoder.c")
    for token in ("cornix_steno_decode_correction_unit", "consonant_identity_for_role"):
        if token not in decoder:
            fail(f"continuous correction decoder support missing: {token}")
    english = read(root / "src/behaviors/behavior_steno_language_return.c")
    for token in ("cornix_steno_english_return_take_pending", "RALT", "LC(F)",
                  "zmk_keymap_layer_deactivate", "zmk_keymap_layer_activate"):
        if token not in english:
            fail(f"English-return behavior missing: {token}")
    if re.search(r"^#define\s+V\s*\(", decoder, re.M):
        fail("decoder defines V(...), which collides with ZMK's V keycode macro")
    workflow = read(root / ".github/workflows/build.yml")
    for watched in ["src/**", "include/**", "dts/**", "zephyr/**", "CMakeLists.txt", "Kconfig"]:
        if f'"{watched}"' not in workflow:
            fail(f"workflow does not rebuild when {watched} changes")
    cmake = read(root / "CMakeLists.txt")
    for source in [
        "behavior_steno_role.c", "behavior_steno_dual.c", "behavior_steno_tab.c",
        "behavior_steno_mode.c", "behavior_steno_language_return.c", "behavior_steno_pulse.c", "behavior_steno_arrow.c",
        "steno_engine.c", "steno_english_return.c", "steno_decoder.c", "steno_output.c", "steno_dual.c",
        "steno_tab.c", "behavior_steno_led.c", "behavior_steno_led_toggle.c", "steno_led.c",
        "steno_dictionary_generated.c", "steno_quick_generated.c", "steno_quick_dispatch.c",
    ]:
        if source not in cmake:
            fail(f"CMakeLists.txt does not include {source}")


def run_host_decoder_test(root: Path) -> str:
    cc = shutil.which("cc") or shutil.which("gcc") or shutil.which("clang")
    if not cc:
        return "SKIP (no C compiler)"
    output = root / "validation/test_decoder"
    cmd = [
        cc, "-std=c11", "-Wall", "-Wextra", "-Werror",
        "-I" + str(root / "validation/stubs"),
        "-I" + str(root / "include"),
        str(root / "validation/test_decoder.c"),
        str(root / "src/steno/steno_decoder.c"),
        str(root / "src/steno/steno_dictionary_generated.c"),
        str(root / "src/steno/steno_quick_generated.c"),
        "-o", str(output),
    ]
    subprocess.run(cmd, check=True)
    proc = subprocess.run([str(output)], check=True, text=True, capture_output=True)
    output.unlink(missing_ok=True)
    return proc.stdout.strip()



def run_api_syntax_checks(root: Path) -> str:
    cc = shutil.which("cc") or shutil.which("gcc") or shutil.which("clang")
    if not cc:
        return "SKIP (no C compiler)"
    stubs = root / "validation/api_stubs"
    sources = sorted((root / "src/behaviors").glob("*.c")) + sorted((root / "src/steno").glob("*.c"))
    for source in sources:
        cmd = [
            cc, "-std=c11", "-Wall", "-Wextra", "-Werror",
            "-Wno-unused-function", "-Wno-unused-variable",
            "-include", str(stubs / "config.h"),
            "-I" + str(stubs), "-I" + str(root / "include"),
            "-fsyntax-only", str(source),
        ]
        subprocess.run(cmd, check=True, stdout=subprocess.DEVNULL)
    return f"PASS ({len(sources)} C sources)"


def run_engine_state_test(root: Path) -> str:
    cc = shutil.which("cc") or shutil.which("gcc") or shutil.which("clang")
    if not cc:
        return "SKIP (no C compiler)"
    output = root / "validation/test_engine"
    stubs = root / "validation/runtime_stubs"
    cmd = [
        cc, "-std=c11", "-Wall", "-Wextra", "-Werror",
        "-include", str(stubs / "config.h"),
        "-I" + str(stubs), "-I" + str(root / "validation/stubs"),
        "-I" + str(root / "include"),
        str(root / "validation/test_engine.c"),
        str(root / "validation/runtime_stub.c"),
        str(root / "validation/output_capture.c"),
        str(root / "validation/quick_invoke_stub.c"),
        str(root / "validation/key_event_capture.c"),
        str(root / "src/steno/steno_engine.c"),
        str(root / "src/steno/steno_decoder.c"),
        str(root / "src/steno/steno_dictionary_generated.c"),
        str(root / "src/steno/steno_quick_generated.c"),
        "-o", str(output),
    ]
    subprocess.run(cmd, check=True)
    proc = subprocess.run([str(output)], check=True, text=True, capture_output=True)
    output.unlink(missing_ok=True)
    return proc.stdout.strip()


def run_english_return_state_test(root: Path) -> str:
    cc = shutil.which("cc") or shutil.which("gcc") or shutil.which("clang")
    if not cc:
        return "SKIP (no C compiler)"
    output = root / "validation/test_english_return"
    stubs = root / "validation/runtime_stubs"
    cmd = [
        cc, "-std=c11", "-Wall", "-Wextra", "-Werror",
        "-include", str(stubs / "config.h"),
        "-I" + str(stubs), "-I" + str(root / "include"),
        str(root / "validation/test_english_return.c"),
        str(root / "validation/runtime_stub.c"),
        str(root / "src/steno/steno_english_return.c"),
        "-o", str(output),
    ]
    subprocess.run(cmd, check=True)
    proc = subprocess.run([str(output)], check=True, text=True, capture_output=True)
    output.unlink(missing_ok=True)
    return proc.stdout.strip()


def run_context_key_test(root: Path) -> str:
    cc = shutil.which("cc") or shutil.which("gcc") or shutil.which("clang")
    if not cc:
        return "SKIP (no C compiler)"
    output = root / "validation/test_dual_tab"
    stubs = root / "validation/runtime_stubs"
    api_stubs = root / "validation/api_stubs"
    cmd = [
        cc, "-std=c11", "-Wall", "-Wextra", "-Werror",
        "-include", str(stubs / "config.h"),
        "-I" + str(stubs), "-I" + str(api_stubs),
        "-I" + str(root / "include"),
        str(root / "validation/test_dual_tab.c"),
        str(root / "validation/runtime_stub.c"),
        str(root / "validation/output_capture.c"),
        str(root / "validation/engine_capture.c"),
        str(root / "validation/key_event_capture.c"),
        str(root / "src/steno/steno_dual.c"),
        str(root / "src/steno/steno_tab.c"),
        "-o", str(output),
    ]
    subprocess.run(cmd, check=True)
    proc = subprocess.run([str(output)], check=True, text=True, capture_output=True)
    output.unlink(missing_ok=True)
    return proc.stdout.strip()


def run_led_logic_test(root: Path) -> str:
    cc = shutil.which("cc") or shutil.which("gcc") or shutil.which("clang")
    if not cc:
        return "SKIP (no C compiler)"
    output = root / "validation/test_led_logic"
    cmd = [
        cc, "-std=c11", "-Wall", "-Wextra", "-Werror",
        "-I" + str(root / "include"),
        str(root / "validation/test_led_logic.c"),
        "-o", str(output),
    ]
    subprocess.run(cmd, check=True)
    proc = subprocess.run([str(output)], check=True, text=True, capture_output=True)
    output.unlink(missing_ok=True)
    return proc.stdout.strip()




def run_peripheral_led_syntax_check(root: Path) -> str:
    cc = shutil.which("cc") or shutil.which("gcc") or shutil.which("clang")
    if not cc:
        return "SKIP (no C compiler)"
    stubs = root / "validation/api_stubs"
    base = (stubs / "config.h").read_text(encoding="utf-8")
    peripheral = base.replace("#define CONFIG_ZMK_SPLIT_ROLE_CENTRAL 1",
                              "#define CONFIG_ZMK_SPLIT_ROLE_CENTRAL 0")
    if "#define CONFIG_ZMK_USB" not in peripheral:
        peripheral = peripheral.replace("#define CONFIG_ZMK_BLE 1",
                                        "#define CONFIG_ZMK_BLE 1\n#define CONFIG_ZMK_USB 0")
    with tempfile.TemporaryDirectory(prefix="cornix-peripheral-") as td:
        cfg = Path(td) / "config_peripheral.h"
        cfg.write_text(peripheral, encoding="utf-8")
        for source in [root / "src/steno/steno_led.c",
                       root / "src/behaviors/behavior_steno_led.c"]:
            cmd = [
                cc, "-std=c11", "-Wall", "-Wextra", "-Werror",
                "-Wno-unused-function", "-Wno-unused-variable",
                "-include", str(cfg),
                "-I" + str(stubs), "-I" + str(root / "include"),
                "-fsyntax-only", str(source),
            ]
            subprocess.run(cmd, check=True, stdout=subprocess.DEVNULL)
    return "PASS (peripheral LED receiver)"

def validate_right_build_hardening(root: Path) -> None:
    build = (root / "build.yaml").read_text(encoding="utf-8")
    reset_block = build.split("artifact-name: cornix_settings_reset")[0].rsplit("- board:", 1)[-1]
    if "studio-rpc-usb-uart" in reset_block:
        fail("settings_reset must not enable the Studio USB-UART transport on cornix_right")
    if "snippet:" in reset_block:
        fail("settings_reset must use the upstream Cornix board+shield form without extra snippets")
    shared_conf = read(root / "config/cornix.conf")
    if re.search(r"^CONFIG_CORNIX_STENO", shared_conf, re.M):
        fail("settings_reset would inherit STENO from shared cornix.conf")
    reset_conf = read(root / "config/settings_reset.conf")
    if "CONFIG_CORNIX_STENO=n" not in reset_conf:
        fail("settings_reset.conf must explicitly disable the STENO engine")

    cmake = (root / "CMakeLists.txt").read_text(encoding="utf-8")
    if "if(NOT CONFIG_ZMK_SPLIT OR CONFIG_ZMK_SPLIT_ROLE_CENTRAL)" not in cmake:
        fail("central-only STENO source gate is missing")
    if "src/steno/steno_quick_dispatch.c" not in cmake:
        fail("quick macro dispatcher missing from central source list")

    led = (root / "src/steno/steno_led.c").read_text(encoding="utf-8")
    if "CST_STENO_CENTRAL" not in led:
        fail("LED central/peripheral compile guard missing")
    if "zmk_split_peripheral_status_changed" in led:
        fail("peripheral must not subscribe to the central-only peripheral-status event")

def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("root", nargs="?", default=".", type=Path)
    args = parser.parse_args()
    root = args.root.resolve()

    yaml_count = validate_yaml_files(root)
    layers = validate_keymap(root)
    validate_role_layout(root)
    kconfig_count = validate_kconfig(root)
    dictionary_count = validate_dictionary(root)
    validate_release_docs(root)
    validate_sources(root)
    validate_led_integration(root)
    host = run_host_decoder_test(root)
    api_syntax = run_api_syntax_checks(root)
    engine_state = run_engine_state_test(root)
    english_return = run_english_return_state_test(root)
    context_keys = run_context_key_test(root)
    led_logic = run_led_logic_test(root)
    peripheral_syntax = run_peripheral_led_syntax_check(root)

    digest = hashlib.sha256((root / "config/cornix.keymap").read_bytes()).hexdigest()
    validate_right_build_hardening(root)
    print("Cornix ZMK BASE+STENO package validation: PASS")
    print(f"- YAML files: {yaml_count}")
    print(f"- Layers: {len(layers)} x 50 bindings")
    print("- ABBR/SYMBOL repeat nav + ㅣ/JLKI + Shift/JLKI selection: PASS")
    print("- Two-set consonant layout: PASS")
    print(f"- Kconfig assignments checked: {kconfig_count}")
    print(f"- Canonical exact-mask dictionary entries: {dictionary_count} + 12 GUI-editable quick macros")
    print(f"- Host decoder: {host}")
    print(f"- ZMK/Zephyr API-shaped syntax: {api_syntax}")
    print(f"- Engine state: {engine_state}")
    print(f"- English-return state: {english_return}")
    print(f"- Vowel dual / one-shot Tab: {context_keys}")
    print(f"- BASE indicator + STENO count/category LED logic: {led_logic}")
    print(f"- Right/peripheral compile shape: {peripheral_syntax}")
    print(f"- cornix.keymap SHA-256: {digest}")


if __name__ == "__main__":
    try:
        main()
    except (RuntimeError, subprocess.CalledProcessError, ValueError) as exc:
        print(f"VALIDATION FAILED: {exc}", file=sys.stderr)
        raise SystemExit(1)

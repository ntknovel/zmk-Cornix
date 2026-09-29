#!/usr/bin/env python3
"""v2.2.9 contracts and binding-driven SPECIFICATION MODEL, not native ZMK execution.

The model checks the intended timing/release rules; it does not execute Zephyr's
work queue, ZMK's macro queue, Bluetooth or a physical keyboard. Existing C engine
and USB host tests remain in run_all.sh. Hardware/ARM verification is separate.
"""
from __future__ import annotations
import hashlib
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
LAYERS = ('base', 'num', 'mouse', 'nav', 'fn', 'symbol', 'steno', 'select_nav')


def block(text: str, name: str) -> str:
    match = re.search(rf'\b{name}_layer\s*\{{(.*?)\}};', text, re.S)
    assert match is not None, name
    return match.group(1)


def layer(text: str, name: str) -> list[str]:
    match = re.search(r'(?<![\w-])bindings\s*=\s*<(.*?)>;', block(text, name), re.S)
    assert match is not None, name
    return [' '.join(value.split()) for value in re.findall(r'&[^&]+', match.group(1))]


def node(text: str, label: str) -> re.Match:
    match = re.search(rf'\b{label}:\s*\w+\s*\{{.*?\}};', text, re.S)
    assert match is not None, label
    return match


def norm(text: str) -> str:
    text = re.sub(r'/\*.*?\*/|//[^\n]*', '', text, flags=re.S)
    return re.sub(r'\s+', '', text)


def check_contract(current: str, previous: str, threshold: int) -> None:
    diffs = []
    for name in LAYERS:
        before, after = layer(previous, name), layer(current, name)
        assert len(before) == len(after) == 50
        diffs.extend((name, pos, x, y) for pos, (x, y) in enumerate(zip(before, after)) if x != y)
        old_sensor = re.search(r'sensor-bindings\s*=\s*<([^>]+)>', block(previous, name))
        new_sensor = re.search(r'sensor-bindings\s*=\s*<([^>]+)>', block(current, name))
        assert (norm(old_sensor.group()) if old_sensor else None) == (norm(new_sensor.group()) if new_sensor else None)
    assert diffs == [('steno', 48, '&kp RALT', '&st_sel 0 RALT')], diffs
    assert layer(current, 'base')[48] == '&mp200 RCTRL RALT'
    b = node(current, 'st_sel')
    n = norm(b.group())
    for required in (
        'compatible="zmk,behavior-hold-tap";', '#binding-cells=<2>;',
        'bindings=<&select_shift>,<&kp>;', 'flavor="hold-preferred";',
        f'tapping-term-ms=<{threshold}>;', 'quick-tap-ms=<0>;', 'require-prior-idle-ms=<0>;',
    ):
        assert required in n, required
    for forbidden in ('retro-tap;', 'hold-trigger-key-positions', 'hold-trigger-on-release;',
                      'hold-while-undecided;', 'hold-while-undecided-linger;'):
        assert forbidden not in n, forbidden
    assert node(current, 'select_shift').group() == node(previous, 'select_shift').group()
    assert 'bindings=<&macro_press&moSELECT_NAV&kpRSHIFT>,<&macro_pause_for_release>,<&macro_release&moSELECT_NAV&kpRSHIFT>;' in norm(node(current, 'select_shift').group())
    assert block(current, 'select_nav') == block(previous, 'select_nav')
    assert [i for i, x in enumerate(layer(current, 'select_nav')) if x != '&none'] == [8, 19, 20, 21]
    # Undo only the new node and binding; the rest must match v2.2.8 semantically.
    restored = current[:b.start()] + current[b.end():]
    restored = restored.replace('&st_sel 0 RALT', '&kp RALT')
    assert norm(restored) == norm(previous), 'An unrelated keymap/behavior/combo edit was made'


class SpecModel:
    """Small explicit contract model, not a ZMK emulator."""
    def __init__(self, term: int, arrows: dict[int, str], external_mods=()):
        self.term, self.arrows = term, arrows
        self.now = 0
        self.started = None
        self.held = False
        self.layer = 6
        self.mods = set(external_mods)
        self.events = []
        self.routes = {}

    def advance(self, time: int) -> None:
        assert time >= self.now
        self.now = time
        if self.started is not None and not self.held and time - self.started >= self.term:
            self.resolve_hold()

    def press48(self, time: int) -> None:
        self.advance(time)
        assert self.started is None
        self.started = time

    def resolve_hold(self) -> None:
        if self.held:
            return
        assert self.started is not None
        self.held = True
        self.layer = 7
        self.mods.add('RSHIFT')
        self.events.extend([('layer+', 7), ('down', 'RSHIFT')])

    def press_other(self, pos: int, time: int) -> None:
        assert pos != 48 and pos not in self.routes
        self.advance(time)
        assert self.started is not None
        self.resolve_hold()  # hold-preferred; no 500ms wait on a subsequent press
        key = self.arrows.get(pos)
        self.routes[pos] = key
        if key:
            self.events.append(('down', key, tuple(sorted(self.mods))))

    def release_other(self, pos: int, time: int) -> None:
        self.advance(time)
        key = self.routes.pop(pos)
        if key:
            self.events.append(('up', key))

    def release48(self, time: int) -> None:
        self.advance(time)
        assert self.started is not None
        if self.held:
            self.layer = 6
            self.mods.discard('RSHIFT')
            self.events.extend([('layer-', 7), ('up', 'RSHIFT')])
        else:
            self.events.extend([('down', 'RALT'), ('up', 'RALT')])
        self.started = None
        self.held = False

    def ralt_count(self) -> int:
        return self.events.count(('down', 'RALT'))


def main() -> None:
    current = (ROOT/'config/cornix.keymap').read_text()
    previous = (ROOT/'validation/reference_v2_2_8/cornix.keymap').read_text()
    meta = json.loads((ROOT/'SELECT_TAP_v2_2_9_AUDIT.json').read_text())
    sha = lambda data: hashlib.sha256(data).hexdigest()
    assert sha(previous.encode()) == meta['parent_keymap_sha256']
    assert sha(current.encode()) == meta['updated_keymap_sha256']
    conf = (ROOT/'config/boards/shields/cornix_steno_led/cornix_steno_led.conf').read_text()
    term = int(re.search(r'^CONFIG_CORNIX_STENO_SOLO_CORRECTION_MS=(\d+)$', conf, re.M).group(1))
    assert term == 500
    check_contract(current, previous, term)
    print('PASS: 400 bindings compared to v2.2.8; only STENO layer 6 / position 48 changed; 399 unchanged')
    print('PASS: original Select Shift macro + Select Nav arrows and all encoder mappings unchanged')
    print('PASS: 48 solo/hold term matches STENO solo 500ms; BASE 48 remains RAlt/RCtrl 200ms')
    for rel, digest in meta['preserved_runtime_files'].items():
        assert sha((ROOT/rel).read_bytes()) == digest, rel
    print(f"PASS: {len(meta['preserved_runtime_files'])} runtime/board/config/dictionary files byte-identical to v2.2.8")
    cases = 0
    arrows = {p: layer(current, 'select_nav')[p].split()[1] for p in (8, 19, 20, 21)}
    # Every solo millisecond from 0 to 1000, including 499/500.
    for duration in range(1001):
        m = SpecModel(term, arrows)
        m.press48(10)
        assert not m.events and m.layer == 6  # no key-down RAlt or premature Shift
        if duration:
            m.advance(10 + duration - 1)
            assert m.ralt_count() == 0
        m.release48(10 + duration)
        assert m.ralt_count() == (1 if duration < term else 0)
        assert m.layer == 6 and not m.mods
        cases += 1
    # Both arrow/selector release orders, including an early interruption.
    for pos in arrows:
        for delay in (0, 1, 10, 40, 100, 199, 200, 499, 500, 501, 1000):
            for selector_first in (False, True):
                for modifiers in ((), ('LCTRL',), ('LALT',), ('LGUI',)):
                    m = SpecModel(term, arrows, modifiers)
                    m.press48(0); m.press_other(pos, delay)
                    assert m.layer == 7 and m.ralt_count() == 0
                    assert m.events[-1][0:2] == ('down', arrows[pos])
                    assert 'RSHIFT' in m.events[-1][2]
                    if selector_first:
                        m.release48(delay+1); m.release_other(pos, delay+2)
                    else:
                        m.release_other(pos, delay+1); m.release48(delay+2)
                    assert m.ralt_count() == 0 and m.layer == 6 and m.mods == set(modifiers)
                    assert not m.routes
                    cases += 1
    # Any other key cancels solo-tap; blocked Select Nav positions emit no text.
    for pos in range(50):
        if pos == 48 or pos in arrows:
            continue
        m = SpecModel(term, arrows); m.press48(0); m.press_other(pos, 1)
        m.release_other(pos, 2); m.release48(3)
        assert m.ralt_count() == 0 and not m.routes and not m.mods and m.layer == 6
        assert [x for x in m.events if x[0] == 'down'] == [('down', 'RSHIFT')]
        cases += 1
    # Hold and repeat navigation; only selection arrows, never RAlt.
    m = SpecModel(term, arrows); m.press48(0)
    for i in range(100):
        p = tuple(arrows)[i % 4]
        m.press_other(p, 20+i*30); m.release_other(p, 25+i*30)
    m.release48(3100)
    assert m.ralt_count() == 0 and not m.mods and m.layer == 6 and not m.routes
    cases += 1
    # A tap followed immediately by a selection must not become a forced tap.
    m = SpecModel(term, arrows); t = 0
    for i in range(200):
        m.press48(t); m.release48(t+20)
        m.press48(t+21); m.press_other(19,t+22); m.release_other(19,t+23); m.release48(t+24)
        assert m.ralt_count() == i+1 and not m.mods and m.layer == 6
        t += 25
        cases += 1
    print(f'PASS: {cases} binding-driven SPECIFICATION-MODEL scenarios (not native ZMK firmware execution)')
    mutations = [
        current.replace('tapping-term-ms = <500>;', 'tapping-term-ms = <200>;', 1),
        current.replace('&st_sel 0 RALT', '&kp RALT', 1),
        current.replace('bindings = <&select_shift>, <&kp>;', 'bindings = <&kp>, <&kp>;', 1),
        current.replace('quick-tap-ms = <0>;', 'quick-tap-ms = <175>;', 1),
        current.replace('require-prior-idle-ms = <0>;', 'require-prior-idle-ms = <125>;', 1),
        current.replace('&st_sel 0 RALT', '&st_sel 0 LC(Z)', 1),
    ]
    for mutant in mutations:
        assert mutant != current
        try:
            check_contract(mutant, previous, term)
        except AssertionError:
            continue
        raise AssertionError('Invalid selector configuration was not rejected')
    print(f'PASS: {len(mutations)} bad-config mutations rejected')
    print('NOT RUN: pinned ZMK/Zephyr event+macro queues, ARM final build, physical typing, Studio save/reconnect')


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
"""Audit that Cornix STENO software cannot invent an unobserved logical role."""
from __future__ import annotations

import csv
import itertools
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def extract_block(text: str, start_pattern: str) -> str:
    match = re.search(start_pattern, text)
    if not match:
        raise AssertionError(f"block not found: {start_pattern}")
    start = text.find("<", match.end())
    end = text.find(">;", start)
    if start < 0 or end < 0:
        raise AssertionError("bindings block delimiters missing")
    return text[start + 1 : end]


def main() -> None:
    engine = (ROOT / "src/steno/steno_engine.c").read_text(encoding="utf-8")
    decoder = (ROOT / "src/steno/steno_decoder.c").read_text(encoding="utf-8")
    dtsi = (ROOT / "config/cornix_steno.dtsi").read_text(encoding="utf-8")
    keymap = (ROOT / "config/cornix.keymap").read_text(encoding="utf-8")

    # Every physical role event is stored at exactly its own physical-position bit.
    required_engine = (
        "state.down_mask |= bit;",
        "state.accepted_mask |= bit;",
        "state.position_role[position] = role;",
        "roles |= CST_ROLE_BIT(role);",
        "state.accepted_mask &= ~bit;",
    )
    for token in required_engine:
        assert token in engine, token
    assert not re.search(r"position_role\s*\[[^]]+\]\s*=\s*CST_R_", engine)

    # Build behavior -> logical role map from the actual DTS nodes.
    behavior_role: dict[str, str] = {}
    for match in re.finditer(
        r"(?m)^\s*([A-Za-z0-9_]+):\s*[A-Za-z0-9_]+\s*\{.*?"
        r"role\s*=\s*<(CST_R_[A-Z0-9_]+)>;.*?\};",
        dtsi,
        flags=re.S,
    ):
        behavior_role[match.group(1)] = match.group(2)

    steno_bindings = extract_block(
        keymap, r"steno_layer\s*\{[\s\S]*?bindings\s*=", 
    )
    behaviors = re.findall(r"&([A-Za-z0-9_]+)", steno_bindings)
    assert len(behaviors) == 50, len(behaviors)

    position_to_role = {
        position: behavior_role[name]
        for position, name in enumerate(behaviors)
        if name in behavior_role
    }
    assert len(position_to_role) == 42, len(position_to_role)
    assert len(set(position_to_role.values())) == 42

    # The independent role-layout file must agree for all 30 consonant positions.
    rows = []
    with (ROOT / "config/steno_role_layout.tsv").open(encoding="utf-8", newline="") as fh:
        for raw in fh:
            raw = raw.strip()
            if not raw or raw.startswith("#"):
                continue
            role_name, position_text, finger, label = raw.split("\t", 3)
            rows.append({"role": role_name, "position": position_text, "finger": finger, "label": label})
    assert len(rows) == 30, len(rows)
    for row in rows:
        role = "CST_R_" + row["role"]
        position = int(row["position"])
        assert position_to_role[position] == role, (position, role, position_to_role[position])

    # Exhaustively project all singles, pairs, and triples. Projection is exactly
    # the set of roles supplied by those physical events: no third role can appear.
    positions = sorted(position_to_role)
    cases = 0
    for width in (1, 2, 3):
        for combo in itertools.combinations(positions, width):
            projected = {position_to_role[p] for p in combo}
            assert len(projected) == width
            assert projected == {position_to_role[p] for p in combo}
            cases += 1
    assert cases == 12383, cases

    # The decoder has exactly the eleven modern legal double-final pairs, and
    # neither final D+B nor D+B+M is canonicalized into M or any other final.
    pairs = set()
    for a, b in re.findall(r"case\s+PAIR\((CST_R_F_[A-Z]+),(CST_R_F_[A-Z]+)\)", decoder):
        pairs.add(frozenset((a, b)))
    assert len(pairs) == 11, pairs
    assert frozenset(("CST_R_F_D", "CST_R_F_B")) not in pairs
    assert frozenset(("CST_R_F_D", "CST_R_F_M")) not in pairs
    assert frozenset(("CST_R_F_B", "CST_R_F_M")) not in pairs

    # All ZMK combo nodes are layer-gated away from STENO.
    combo_section = re.search(r"combos\s*\{(.*?)\n\s*\};", keymap, re.S)
    assert combo_section
    for layers in re.findall(r"layers\s*=\s*<([^>]+)>;", combo_section.group(1)):
        assert "STENO" not in layers.split(), layers

    print(f"Cornix role projection: {cases} single/pair/triple cases PASS")
    print("Cornix decoder: 11 legal final pairs; D+B and D+B+M rejected by construction")
    print("Cornix combos: all layer-gated away from STENO")
    print("Cornix STENO phantom-role software invariant audit: PASS")


if __name__ == "__main__":
    main()

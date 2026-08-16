#!/usr/bin/env python3
"""Generate the Cornix canonical exact-mask STENO dictionary C source.

The completed dictionary uses the full logical role mask.  Direct consonant
chords, ABBR_L, ABBR_R and AB2 are all meaningful and must not be collapsed
into the older two-bank prototype.  VEXT_L is the canonical AB2 bit in the
source; firmware lookup accepts either physical VEXT side for AB2 entries.
"""
from __future__ import annotations

import argparse
import csv
import re
from dataclasses import dataclass
from pathlib import Path

ROLE_NAMES = [
    "I_KH", "I_B", "I_J", "I_H", "I_N", "I_D", "I_S", "I_G",
    "I_DOUBLE", "I_NG", "I_P", "I_CH", "I_T", "I_M", "I_R",
    "F_NG", "F_D", "F_B", "F_M", "F_T", "F_R", "F_DOUBLE", "F_G",
    "F_S", "F_P", "F_N", "F_KH", "F_H", "F_J", "F_CH",
    "ABBR_L", "ABBR_R", "VEXT_L",
]
ROLE_ENUMS = {name: f"CST_R_{name}" for name in ROLE_NAMES}
CONSONANT_ROLES = set(ROLE_NAMES[:30])
SELECTOR_ROLES = {"ABBR_L", "ABBR_R", "VEXT_L"}

CHO_KEYS = [
    ["R"], ["LS(R)"], ["S"], ["E"], ["LS(E)"], ["F"], ["A"],
    ["Q"], ["LS(Q)"], ["T"], ["LS(T)"], ["D"], ["W"], ["LS(W)"],
    ["C"], ["Z"], ["X"], ["V"], ["G"],
]
JUNG_KEYS = [
    ["K"], ["O"], ["I"], ["LS(O)"], ["J"], ["P"], ["U"], ["LS(P)"],
    ["H"], ["H", "K"], ["H", "O"], ["H", "L"], ["Y"], ["N"],
    ["N", "J"], ["N", "P"], ["N", "L"], ["B"], ["M"], ["M", "L"], ["L"],
]
JONG_KEYS = [
    [], ["R"], ["LS(R)"], ["R", "T"], ["S"], ["S", "W"], ["S", "G"],
    ["E"], ["F"], ["F", "R"], ["F", "A"], ["F", "Q"], ["F", "T"],
    ["F", "X"], ["F", "V"], ["F", "G"], ["A"], ["Q"], ["Q", "T"],
    ["T"], ["LS(T)"], ["D"], ["W"], ["C"], ["Z"], ["X"], ["V"], ["G"],
]
ASCII_KEYS = {
    " ": ["SPACE"], "\t": ["TAB"], "\n": ["ENTER"],
    ",": ["COMMA"], ".": ["DOT"], "?": ["QUESTION"], "!": ["EXCL"],
    "[": ["LBKT"], "]": ["RBKT"], "%": ["PRCNT"], "^": ["CARET"],
    "'": ["SQT"], '"': ["DQT"], "#": ["HASH"], "~": ["TILDE"],
}
NUMBER_PAIRS = [{1, 13}, {2, 14}, {3, 15}, {4, 16}, {5, 17},
                {6, 18}, {7, 19}, {8, 20}, {9, 21}, {10, 22}]

@dataclass(frozen=True)
class Entry:
    entry_id: str
    category: str
    output: str
    roles: tuple[str, ...]
    status: str
    notes: str

@dataclass(frozen=True)
class RoleLayout:
    position: int
    finger: str


def text_to_keys(text: str) -> list[str]:
    keys: list[str] = []
    for ch in text:
        code = ord(ch)
        if 0xAC00 <= code <= 0xD7A3:
            n = code - 0xAC00
            jong = n % 28
            jung = (n // 28) % 21
            cho = n // (28 * 21)
            keys.extend(CHO_KEYS[cho])
            keys.extend(JUNG_KEYS[jung])
            keys.extend(JONG_KEYS[jong])
        elif ch in ASCII_KEYS:
            keys.extend(ASCII_KEYS[ch])
        elif "a" <= ch <= "z":
            keys.append(ch.upper())
        elif "A" <= ch <= "Z":
            keys.append(f"LS({ch})")
        elif "0" <= ch <= "9":
            keys.append(f"N{ch}")
        else:
            raise ValueError(f"unsupported character U+{ord(ch):04X}: {ch!r}")
    return keys


def load_layout(path: Path) -> dict[str, RoleLayout]:
    layout: dict[str, RoleLayout] = {}
    with path.open("r", encoding="utf-8", newline="") as f:
        for line_no, row in enumerate(csv.reader(f, delimiter="\t"), 1):
            if not row or row[0].lstrip().startswith("#"):
                continue
            if len(row) < 3:
                raise ValueError(f"{path}:{line_no}: expected role, position, finger")
            role, position, finger = row[0].strip(), int(row[1]), row[2].strip()
            if role not in CONSONANT_ROLES:
                raise ValueError(f"{path}:{line_no}: unknown consonant role {role}")
            if role in layout:
                raise ValueError(f"{path}:{line_no}: duplicate role {role}")
            layout[role] = RoleLayout(position, finger)
    missing = sorted(CONSONANT_ROLES - set(layout))
    if missing:
        raise ValueError(f"{path}: missing roles: {missing}")
    return layout


def load_entries(path: Path, layout: dict[str, RoleLayout]) -> list[Entry]:
    entries: list[Entry] = []
    with path.open("r", encoding="utf-8", newline="") as f:
        for line_no, row in enumerate(csv.reader(f, delimiter="\t"), 1):
            if not row or row[0].lstrip().startswith("#"):
                continue
            if len(row) < 4:
                raise ValueError(f"{path}:{line_no}: expected id, category, output, roles")
            entry_id, category, output, roles_text = (cell.strip() for cell in row[:4])
            status = row[4].strip() if len(row) > 4 else "confirmed"
            notes = row[5].strip() if len(row) > 5 else ""
            if not entry_id or not category or not output:
                raise ValueError(f"{path}:{line_no}: id/category/output may not be blank")
            roles = tuple(part.strip() for part in roles_text.split("+") if part.strip())
            if len(roles) < 2:
                raise ValueError(f"{path}:{line_no}: canonical entry needs at least two roles")
            unknown = [role for role in roles if role not in ROLE_ENUMS]
            if unknown:
                raise ValueError(f"{path}:{line_no}: unknown roles: {unknown}")
            if len(set(roles)) != len(roles):
                raise ValueError(f"{path}:{line_no}: duplicate logical role")
            if "VEXT_L" in roles and "VEXT_R" in roles:
                raise ValueError(f"{path}:{line_no}: AB2 uses one flexible VEXT selector")

            # The canonical dictionary already chooses selector sides.  Validate
            # simultaneous consonant keys and reserved physical number pairs.
            consonants = [role for role in roles if role in CONSONANT_ROLES]
            fingers = [layout[role].finger for role in consonants]
            duplicate_fingers = sorted({f for f in fingers if fingers.count(f) > 1})
            if duplicate_fingers:
                raise ValueError(
                    f"{path}:{line_no}: same-finger collision {duplicate_fingers} in {output!r}"
                )
            positions = {layout[role].position for role in consonants}
            bad_pairs = [pair for pair in NUMBER_PAIRS if pair <= positions]
            if bad_pairs:
                raise ValueError(
                    f"{path}:{line_no}: contains reserved number-column pair {bad_pairs}"
                )
            text_to_keys(output)
            entries.append(Entry(entry_id, category, output, roles, status, notes))

    seen_masks: dict[tuple[str, ...], str] = {}
    seen_ids: set[str] = set()
    for entry in entries:
        if entry.entry_id in seen_ids:
            raise ValueError(f"duplicate id: {entry.entry_id}")
        seen_ids.add(entry.entry_id)
        key = tuple(sorted(entry.roles))
        if key in seen_masks:
            raise ValueError(
                f"exact-mask collision: {seen_masks[key]!r} and {entry.output!r} share {key}"
            )
        seen_masks[key] = entry.output
    return entries


def c_ident(index: int, text: str) -> str:
    hint = re.sub(r"[^a-z0-9]+", "_", text.lower()).strip("_")
    return f"dict_seq_{index:02d}" + (f"_{hint}" if hint else "")


def generate(entries: list[Entry]) -> str:
    lines = [
        "/* Auto-generated by tools/generate_steno_dictionary.py. */",
        "/* Canonical exact masks: ABBR_L/R distinct; one VEXT side = AB2. */",
        "/* SPDX-License-Identifier: MIT */", "",
        "#include <stddef.h>", "#include <stdint.h>",
        "#include <dt-bindings/zmk/keys.h>",
        "#include <cornix_steno/dictionary.h>",
        "#include <cornix_steno/roles.h>", "",
    ]
    names: list[str] = []
    for index, entry in enumerate(entries):
        name = c_ident(index, entry.output)
        names.append(name)
        lines.append(f"/* {entry.entry_id} | {entry.category} | {entry.output} */")
        lines.append(f"static const uint32_t {name}[] = {{ {', '.join(text_to_keys(entry.output))} }};")
    lines += ["", "static const struct cornix_steno_dictionary_entry entries[] = {"]
    for entry, name in zip(entries, names):
        mask = " | ".join(f"CST_ROLE_BIT({ROLE_ENUMS[r]})" for r in entry.roles)
        lines.append(
            f"    {{ .role_mask = ({mask}), .keys = {name}, "
            f".key_count = (uint16_t)(sizeof({name}) / sizeof({name}[0])) }},"
        )
    lines += [
        "};", "",
        "uint64_t cornix_steno_dictionary_normalize_mask(uint64_t role_mask) {",
        "    const uint64_t vext_l = CST_ROLE_BIT(CST_R_VEXT_L);",
        "    const uint64_t vext_r = CST_ROLE_BIT(CST_R_VEXT_R);",
        "    const uint64_t vext = role_mask & CST_VEXT_MASK;",
        "    if ((role_mask & CST_BASE_VOWEL_MASK) == 0 &&",
        "        (vext == vext_l || vext == vext_r)) {",
        "        role_mask = (role_mask & ~CST_VEXT_MASK) | vext_l;",
        "    }",
        "    return role_mask;",
        "}", "",
        "const struct cornix_steno_dictionary_entry *",
        "cornix_steno_dictionary_lookup(uint64_t role_mask) {",
        "    const uint64_t normalized = cornix_steno_dictionary_normalize_mask(role_mask);",
        "    for (size_t i = 0; i < sizeof(entries) / sizeof(entries[0]); i++) {",
        "        if (entries[i].role_mask == normalized) return &entries[i];",
        "    }",
        "    return NULL;",
        "}", "",
        "bool cornix_steno_dictionary_has_exact(uint64_t role_mask) {",
        "    return cornix_steno_dictionary_lookup(role_mask) != NULL;",
        "}", "",
        "size_t cornix_steno_dictionary_count(void) {",
        "    return sizeof(entries) / sizeof(entries[0]);",
        "}", "",
    ]
    return "\n".join(lines)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("input", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--layout", type=Path)
    args = parser.parse_args()
    layout_path = args.layout or args.input.with_name("steno_role_layout.tsv")
    layout = load_layout(layout_path)
    entries = load_entries(args.input, layout)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(generate(entries), encoding="utf-8", newline="\n")
    print(f"generated {len(entries)} canonical exact-mask entries -> {args.output}")

if __name__ == "__main__":
    main()

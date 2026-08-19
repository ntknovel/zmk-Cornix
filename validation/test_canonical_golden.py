#!/usr/bin/env python3
from pathlib import Path
root=Path(__file__).resolve().parents[1]
def rows(p):
    return [ln.rstrip('\n') for ln in p.read_text(encoding='utf-8').splitlines() if ln and not ln.startswith('#')]
assert rows(root/'config/steno_dictionary.tsv') == rows(root/'validation/canonical_dictionary_golden.tsv')
print('Cornix canonical 43 logical-mask golden comparison: PASS')

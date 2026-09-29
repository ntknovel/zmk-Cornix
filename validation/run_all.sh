#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
python3 "$ROOT/tools/validate_package.py" "$ROOT"
bash "$ROOT/validation/run_host_tests.sh"
python3 "$ROOT/tools/validate_studio.py" "$ROOT"
bash "$ROOT/validation/run_keymap_v228.sh"
python3 "$ROOT/validation/test_select_tap_v229.py"
echo 'ALL SOURCE/HOST CHECKS PASSED. ARM BUILD AND USB DEVICE TEST ARE SEPARATE.'

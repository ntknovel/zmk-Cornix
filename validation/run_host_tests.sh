#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CC_BIN="${CC:-cc}"

python3 "$ROOT/tools/generate_validation_role_map.py"
python3 "$ROOT/tools/generate_canonical_engine_cases.py"
python3 "$ROOT/validation/test_canonical_golden.py"
python3 "$ROOT/validation/test_role_behavior_led_static.py"
python3 "$ROOT/validation/test_phantom_role_invariants.py"

"$CC_BIN" -std=c11 -Wall -Wextra -Werror \
  -include "$ROOT/validation/runtime_stubs/config.h" \
  -I"$ROOT/validation/runtime_stubs" -I"$ROOT/include" \
  "$ROOT/validation/test_english_return.c" \
  "$ROOT/validation/runtime_stub.c" \
  "$ROOT/src/steno/steno_english_return.c" \
  -o "$ROOT/validation/test_english_return"
"$ROOT/validation/test_english_return"
rm -f "$ROOT/validation/test_english_return"

"$CC_BIN" -std=c11 -Wall -Wextra -Werror \
  -I"$ROOT/validation/stubs" -I"$ROOT/include" \
  "$ROOT/validation/test_decoder.c" \
  "$ROOT/src/steno/steno_decoder.c" \
  "$ROOT/src/steno/steno_dictionary_generated.c" \
  "$ROOT/src/steno/steno_quick_generated.c" \
  -o "$ROOT/validation/test_decoder"
"$ROOT/validation/test_decoder"
rm -f "$ROOT/validation/test_decoder"

"$CC_BIN" -std=c11 -Wall -Wextra -Werror \
  -I"$ROOT/include" \
  "$ROOT/validation/test_led_logic.c" \
  -o "$ROOT/validation/test_led_logic"
"$ROOT/validation/test_led_logic"
rm -f "$ROOT/validation/test_led_logic"

"$CC_BIN" -std=c11 -Wall -Wextra -Werror \
  -include "$ROOT/validation/runtime_stubs/config.h" \
  -I"$ROOT/validation/runtime_stubs" \
  -I"$ROOT/validation/stubs" \
  -I"$ROOT/include" \
  "$ROOT/validation/test_engine.c" \
  "$ROOT/validation/runtime_stub.c" \
  "$ROOT/validation/output_capture.c" \
  "$ROOT/validation/quick_invoke_stub.c" \
  "$ROOT/validation/key_event_capture.c" \
  "$ROOT/src/steno/steno_engine.c" \
  "$ROOT/src/steno/steno_decoder.c" \
  "$ROOT/src/steno/steno_dictionary_generated.c" \
  "$ROOT/src/steno/steno_quick_generated.c" \
  -o "$ROOT/validation/test_engine"
"$ROOT/validation/test_engine"
rm -f "$ROOT/validation/test_engine"

"$CC_BIN" -std=c11 -Wall -Wextra -Werror \
  -include "$ROOT/validation/runtime_stubs/config.h" \
  -I"$ROOT/validation/runtime_stubs" -I"$ROOT/validation/stubs" -I"$ROOT/include" -I"$ROOT/validation" \
  "$ROOT/validation/test_engine_comprehensive.c" \
  "$ROOT/validation/runtime_stub.c" "$ROOT/validation/output_capture.c" \
  "$ROOT/validation/quick_invoke_stub.c" "$ROOT/validation/key_event_capture.c" \
  "$ROOT/src/steno/steno_engine.c" "$ROOT/src/steno/steno_decoder.c" \
  "$ROOT/src/steno/steno_dictionary_generated.c" "$ROOT/src/steno/steno_quick_generated.c" \
  -o "$ROOT/validation/test_engine_comprehensive"
"$ROOT/validation/test_engine_comprehensive"
rm -f "$ROOT/validation/test_engine_comprehensive"


"$CC_BIN" -std=c11 -Wall -Wextra -Werror \
  -include "$ROOT/validation/runtime_stubs/config.h" \
  -I"$ROOT/validation/runtime_stubs" -I"$ROOT/validation/api_stubs" -I"$ROOT/include" \
  "$ROOT/validation/test_dual_tab.c" \
  "$ROOT/validation/runtime_stub.c" \
  "$ROOT/validation/output_capture.c" \
  "$ROOT/validation/engine_capture.c" \
  "$ROOT/validation/key_event_capture.c" \
  "$ROOT/src/steno/steno_dual.c" \
  "$ROOT/src/steno/steno_tab.c" \
  -o "$ROOT/validation/test_dual_tab"
"$ROOT/validation/test_dual_tab"
rm -f "$ROOT/validation/test_dual_tab"

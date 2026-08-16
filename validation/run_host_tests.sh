#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CC_BIN="${CC:-cc}"

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
  "$ROOT/src/steno/steno_engine.c" \
  "$ROOT/src/steno/steno_decoder.c" \
  "$ROOT/src/steno/steno_dictionary_generated.c" \
  "$ROOT/src/steno/steno_quick_generated.c" \
  -o "$ROOT/validation/test_engine"
"$ROOT/validation/test_engine"
rm -f "$ROOT/validation/test_engine"


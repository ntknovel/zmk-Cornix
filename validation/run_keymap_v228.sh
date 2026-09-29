#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
python3 "$ROOT/tools/validate_keymap_update.py" "$ROOT"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
for CC in gcc clang; do
    SAN=()
    if [[ "$CC" == clang ]]; then SAN=(-fsanitize=address,undefined -fno-omit-frame-pointer); fi
    "$CC" -std=c11 -Wall -Wextra -Werror "${SAN[@]}" \
      -include "$ROOT/validation/runtime_stubs/config.h" \
      -I"$ROOT/validation/runtime_stubs" -I"$ROOT/validation/stubs" -I"$ROOT/include" \
      "$ROOT/validation/test_solo_outputs_v228.c" "$ROOT/validation/runtime_stub.c" \
      "$ROOT/validation/output_capture.c" "$ROOT/validation/quick_invoke_stub.c" \
      "$ROOT/validation/key_event_capture.c" "$ROOT/src/steno/steno_engine.c" \
      "$ROOT/src/steno/steno_decoder.c" "$ROOT/src/steno/steno_dictionary_generated.c" \
      "$ROOT/src/steno/steno_quick_generated.c" -o "$TMP/solo_$CC"
    "$TMP/solo_$CC"
    "$CC" -std=c11 -Wall -Wextra -Werror -Wno-unused-variable "${SAN[@]}" \
      -include "$ROOT/validation/api_stubs/config.h" \
      -I"$ROOT/validation/keymap_runtime_stubs" -I"$ROOT/validation/api_stubs" -I"$ROOT/include" \
      "$ROOT/validation/test_menu_return_v228.c" -o "$TMP/menu_$CC"
    "$TMP/menu_$CC"
done
python3 "$ROOT/validation/test_keymap_model_v228.py"
python3 "$ROOT/validation/test_decoder_delta_v228.py"

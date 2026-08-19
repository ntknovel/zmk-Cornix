#!/usr/bin/env python3
from pathlib import Path
root=Path(__file__).resolve().parents[1]
beh=(root/'src/behaviors/behavior_steno_role.c').read_text()
eng=(root/'src/steno/steno_engine.c').read_text()
assert 'cornix_steno_led_key_pressed(event.position);' in beh
assert 'if (err == CST_ENGINE_EVENT_CONSUMED) {\n        return ZMK_BEHAVIOR_OPAQUE;' in beh
release=beh[beh.index('static int on_released'):beh.index('static const struct behavior_driver_api')]
assert release.index('cornix_steno_led_key_released(event.position);') < release.index('cornix_steno_engine_role_released')
assert 'cornix_steno_led_key_released(' not in eng
print('Cornix physical LED edge ownership invariant: PASS')

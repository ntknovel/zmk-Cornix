# SPDX-License-Identifier: MIT

target_include_directories(app PRIVATE include)

target_sources_ifdef(CONFIG_ZMK_BEHAVIOR_RF_POWER app PRIVATE
  src/behaviors/behavior_rf_power.c
)

if(CONFIG_CORNIX_STENO)
  # ZMK split peripherals only scan positions and forward them to the central.
  # Compile keymap/HID/STENO decoding only on the central (or a non-split build).
  if(NOT CONFIG_ZMK_SPLIT OR CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    target_sources(app PRIVATE
      src/behaviors/behavior_steno_role.c
      src/behaviors/behavior_steno_dual.c
      src/behaviors/behavior_steno_tab.c
      src/behaviors/behavior_steno_mode.c
      src/behaviors/behavior_steno_language_return.c
      src/behaviors/behavior_steno_pulse.c
      src/behaviors/behavior_steno_arrow.c
      src/steno/steno_engine.c
      src/steno/steno_english_return.c
      src/steno/steno_decoder.c
      src/steno/steno_output.c
      src/steno/steno_dual.c
      src/steno/steno_tab.c
    )

    if(CONFIG_CORNIX_STENO_DICTIONARY)
      target_sources(app PRIVATE
        src/steno/steno_dictionary_generated.c
        src/steno/steno_quick_generated.c
        src/steno/steno_quick_dispatch.c
      )
    endif()

    if(CONFIG_CORNIX_STENO_LED)
      target_sources(app PRIVATE
        src/behaviors/behavior_steno_led_toggle.c
      )
    endif()
  endif()

  # The global LED receiver and local WS2812 renderer are required on both
  # halves. The central-only publisher is compile-guarded inside steno_led.c.
  if(CONFIG_CORNIX_STENO_LED)
    target_sources(app PRIVATE
      src/behaviors/behavior_steno_led.c
      src/steno/steno_led.c
    )
  endif()
endif()

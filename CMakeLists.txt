# SPDX-License-Identifier: MIT

target_include_directories(app PRIVATE include)

target_sources_ifdef(CONFIG_ZMK_BEHAVIOR_RF_POWER app PRIVATE
  src/behaviors/behavior_rf_power.c
)

if(CONFIG_CORNIX_STENO)
  target_sources(app PRIVATE
    src/behaviors/behavior_steno_role.c
    src/behaviors/behavior_steno_dual.c
    src/behaviors/behavior_steno_tab.c
    src/behaviors/behavior_steno_mode.c
    src/behaviors/behavior_steno_pulse.c
    src/behaviors/behavior_steno_arrow.c
    src/steno/steno_engine.c
    src/steno/steno_decoder.c
    src/steno/steno_output.c
    src/steno/steno_dual.c
    src/steno/steno_tab.c
  )
  if(CONFIG_CORNIX_STENO_DICTIONARY)
    target_sources(app PRIVATE
      src/steno/steno_dictionary_generated.c
      src/steno/steno_quick_generated.c
    )
  endif()
  if(CONFIG_CORNIX_STENO_LED)
    target_sources(app PRIVATE
      src/behaviors/behavior_steno_led.c
      src/behaviors/behavior_steno_led_toggle.c
      src/steno/steno_led.c
    )
  endif()
endif()

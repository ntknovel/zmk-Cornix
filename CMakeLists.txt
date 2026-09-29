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

# CORNIX_STENO_USB_STUDIO_V2_2_7

if(CONFIG_CORNIX_STENO_STUDIO_USB_AUTO_SELECT)
  if(NOT CONFIG_CORNIX_STENO OR NOT CONFIG_CORNIX_STENO_LED
      OR NOT CONFIG_ZMK_STUDIO OR NOT CONFIG_ZMK_STUDIO_RPC
      OR NOT CONFIG_ZMK_USB OR NOT CONFIG_ZMK_SPLIT_ROLE_CENTRAL
      OR NOT CONFIG_ZMK_STUDIO_TRANSPORT_UART)
    message(FATAL_ERROR "CORNIX STENO: central Studio + USB UART transport is required")
  endif()
  if(CONFIG_ZMK_STUDIO_LOCKING OR CONFIG_ZMK_STUDIO_LOCK_ON_DISCONNECT
      OR CONFIG_ZMK_STUDIO_TRANSPORT_BLE)
    message(FATAL_ERROR "CORNIX STENO: Studio must be unlocked and USB-only")
  endif()
  # Select the declared API from the actual checked-out ZMK headers. This
  # handles the endpoint API rename without assuming a header from another
  # revision. Unsupported APIs fail before the application is compiled.
  file(READ "${APPLICATION_SOURCE_DIR}/include/zmk/endpoints.h" _cornix_ep_header)
  if(_cornix_ep_header MATCHES "int[ \t\r\n]+zmk_endpoint_set_preferred_transport[ \t\r\n]*\\(")
    set(_cornix_usb_select zmk_endpoint_set_preferred_transport)
  elseif(_cornix_ep_header MATCHES "int[ \t\r\n]+zmk_endpoints_select_transport[ \t\r\n]*\\(")
    set(_cornix_usb_select zmk_endpoints_select_transport)
  else()
    message(FATAL_ERROR "CORNIX STENO: no supported USB endpoint selection API was found")
  endif()
  target_compile_definitions(app PRIVATE CORNIX_SELECT_USB_TRANSPORT=${_cornix_usb_select})
  target_sources(app PRIVATE src/studio/usb_auto_select.c src/studio/compile_checks.c)
  message(STATUS "CORNIX STENO Studio: USB RPC / locking OFF / auto USB selection ON")
endif()

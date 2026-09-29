/* SPDX-License-Identifier: MIT
 * These are real-build guards, not evidence that an ARM build has been run.
 * Inherit the pinned Cornix hardware layout; never invent/replace GPIO maps.
 */
#include <zephyr/devicetree.h>
#include <zephyr/sys/util.h>

BUILD_ASSERT(IS_ENABLED(CONFIG_CORNIX_STENO), "STENO must remain enabled");
BUILD_ASSERT(IS_ENABLED(CONFIG_CORNIX_STENO_LED), "STENO LED must remain enabled");
BUILD_ASSERT(IS_ENABLED(CONFIG_ZMK_STUDIO), "Studio must be enabled");
BUILD_ASSERT(IS_ENABLED(CONFIG_ZMK_STUDIO_RPC), "Studio RPC is required");
BUILD_ASSERT(IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA), "Studio behavior metadata is required");
BUILD_ASSERT(IS_ENABLED(CONFIG_ZMK_STUDIO_TRANSPORT_UART), "USB UART transport is required");
BUILD_ASSERT(IS_ENABLED(CONFIG_USB_CDC_ACM), "USB serial device is required");
BUILD_ASSERT(IS_ENABLED(CONFIG_ZMK_USB), "USB is required");
BUILD_ASSERT(IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL), "Studio is central-only");
BUILD_ASSERT(!IS_ENABLED(CONFIG_ZMK_STUDIO_LOCKING), "Studio locking must be disabled");
BUILD_ASSERT(!IS_ENABLED(CONFIG_ZMK_STUDIO_TRANSPORT_BLE), "Studio configuration transport is USB-only");

#if DT_HAS_CHOSEN(zmk_matrix_transform)
#error "Studio must use the physical layout, not a chosen matrix-transform"
#endif
#if !DT_HAS_CHOSEN(zmk_physical_layout)
#error "Pinned Cornix hardware must supply a chosen zmk,physical-layout"
#else
#define CST_STUDIO_LAYOUT DT_CHOSEN(zmk_physical_layout)
#if !DT_NODE_HAS_PROP(CST_STUDIO_LAYOUT, keys)
#error "Studio physical layout is missing key geometry"
#else
BUILD_ASSERT(DT_PROP_LEN(CST_STUDIO_LAYOUT, keys) == 50, "Cornix Studio layout must have 50 positions");
#endif
#endif

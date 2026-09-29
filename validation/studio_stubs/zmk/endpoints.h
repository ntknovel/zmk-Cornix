#pragma once
enum zmk_transport { ZMK_TRANSPORT_USB, ZMK_TRANSPORT_BLE };
#ifdef CORNIX_TEST_OLD_API
int zmk_endpoints_select_transport(enum zmk_transport transport);
#else
int zmk_endpoint_set_preferred_transport(enum zmk_transport transport);
#endif

/* -*- mode: c++ -*-
 * OverlayProcessing -- OVERLAY keys handling and raw HID reporting
 * Copyright (C) 2026 Dygma Lab S.L. www.dygma.com
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Owns everything about the OVERLAY keys: the tap / hold / double-tap state
 * machine, the raw HID packet layout and the transport selection (BLE or USB).
 *
 * The key events arrive through the keyboard API interface, so Kaleidoscope
 * only has to map the OVERLAY ranges to their kbdapi key types in KbdInterface.
 */

#pragma once
#include <cstdint>

#include "kbd_if.h"
#include "Time_counter.h"

#define OVERLAY_HOLD_TIMEOUT_MS     200
#define OVERLAY_TAP_WINDOW_MS       100

class OverlayProcessing {
   public:
    result_t init( void );
    void run( void );

   private:
    /* Packet types — must match constants.ts in Dygma-Lens */
    enum class packet_t : uint8_t
    {
        OVERLAY      = 0x01,  /* OVERLAY_KEY (superkey with tap/hold/double-tap) */
        LAYER        = 0x02,
        OVERLAY_TAP  = 0x03,  /* OVERLAY_TAP key (simple tap key) */
        OVERLAY_HOLD = 0x04,  /* OVERLAY_HOLD key (simple hold key) */
    };

    enum class event_t : uint8_t
    {
        RELEASE    = 0x00,
        TAP        = 0x01,
        HOLD       = 0x02,
        DOUBLE_TAP = 0x03,
    };

    /* Raw HID reporting */
    static bool host_connected( void );
    static void send_packet( packet_t type, uint8_t payload );

    /* Keyboard API interface */
    kbdif_t * p_kbdif = NULL;
    result_t kbdif_initialize( void );

    static const kbdif_handlers_t kbdif_handlers;

    static kbdapi_event_result_t kbdif_key_event_cb( void * p_instance, kbdapi_key_t * p_key );
    static kbdapi_event_result_t kbdif_led_layer_change_event_cb( void * p_instance, kbdapi_led_layer_id_t layer_id );

    /* OVERLAY_KEY tap / hold / double-tap state machine */
    enum class state_t : uint8_t { IDLE, PRESSED, FIRST_UP, HOLD_ACTIVE, DOUBLE_PRESSED };

    state_t overlay_state = state_t::IDLE;
    dl_timer_t hold_timer;
    dl_timer_t tap_window_timer;

    void overlay_key_event( kbdapi_key_t * p_key );

    /* The initial layer is pushed as soon as a host shows up */
    bool initial_report_sent = false;
    uint8_t layer_current = 0;
};

extern class OverlayProcessing OverlayProcessing;

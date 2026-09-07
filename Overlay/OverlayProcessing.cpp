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

#include "OverlayProcessing.h"

#include "kbd_if_manager.h"

#include "KeyboardioHID.h"
#include "ble_hid_service.h"    // INPUT_REPORT_LEN_RAW
#include "Ble_composite_dev.h"  // ble_connected()
#include "Adafruit_TinyUSB.h"   // TinyUSBDevice.mounted()

#define DEBUG_LOG_OVERLAY_PROCESSING    0

// USB Full Speed endpoint max is 64 bytes; 1 byte is the report ID, so 63 data bytes.
// Must match USB_RAW_HID_REPORT_SIZE in hid_report_descriptor.cpp.
#define USB_RAW_HID_REPORT_SIZE  63

// Packet framing - must match constants.ts in Dygma-Lens
#define OVERLAY_MAGIC_BYTE       0xAA

class OverlayProcessing OverlayProcessing;

/****************************************************/
/*                    Life cycle                    */
/****************************************************/

result_t OverlayProcessing::init( void )
{
    result_t result = RESULT_ERR;

    result = kbdif_initialize();
    EXIT_IF_ERR( result, "kbdif_initialize failed" );

_EXIT:
    return result;
}

void OverlayProcessing::run( void )
{
    /* Push the initial layer as soon as a host is able to receive it */
    if ( initial_report_sent == false && host_connected() == true )
    {
        initial_report_sent = true;
        send_packet( packet_t::LAYER, layer_current );
    }

    /* Resolve the OVERLAY_KEY states that are waiting on a timer */
    switch ( overlay_state )
    {
        case state_t::PRESSED:
            if ( timer_check( &hold_timer ) == true )
            {
                overlay_state = state_t::HOLD_ACTIVE;
                send_packet( packet_t::OVERLAY, (uint8_t)event_t::HOLD );
            }
            break;

        case state_t::FIRST_UP:
            if ( timer_check( &tap_window_timer ) == true )
            {
                overlay_state = state_t::IDLE;
                send_packet( packet_t::OVERLAY, (uint8_t)event_t::TAP );
            }
            break;

        default:
            break;
    }
}

/****************************************************/
/*                 Raw HID reporting                */
/****************************************************/

bool OverlayProcessing::host_connected( void )
{
    return ble_connected() || TinyUSBDevice.mounted();
}

void OverlayProcessing::send_packet( packet_t type, uint8_t payload )
{
#if DEBUG_LOG_OVERLAY_PROCESSING
    NRF_LOG_DEBUG( "OverlayProcessing: packet=0x%02X payload=0x%02X ble=%d",
                   (uint8_t)type, payload, (int)ble_connected() );
#endif

    /* BLE and USB carry the same packet, but the raw HID report length differs:
     * BLE uses the full 200-byte report, USB is capped at 63 data bytes. */
    if ( ble_connected() )
    {
        uint8_t buf[INPUT_REPORT_LEN_RAW] = {};
        buf[0] = OVERLAY_MAGIC_BYTE;
        buf[1] = (uint8_t)type;
        buf[2] = payload;
        HID().SendReport( HID_REPORTID_RAWHID, buf, INPUT_REPORT_LEN_RAW );
        return;
    }

    uint8_t buf[USB_RAW_HID_REPORT_SIZE] = {};
    buf[0] = OVERLAY_MAGIC_BYTE;
    buf[1] = (uint8_t)type;
    buf[2] = payload;
    HID().SendReport( HID_REPORTID_RAWHID, buf, USB_RAW_HID_REPORT_SIZE );
}

/****************************************************/
/*                   Keyboard API                   */
/****************************************************/

result_t OverlayProcessing::kbdif_initialize( void )
{
    result_t result = RESULT_ERR;
    kbdif_conf_t config;

    /* Prepare the kbdif configuration */
    config.p_instance = this;
    config.handlers = &kbdif_handlers;

    /* Initialize the kbdif */
    result = kbdif_init( &p_kbdif, &config );
    EXIT_IF_ERR( result, "kbdif_init failed" );

    /* Add the kbdif into the kbdif manager */
    result = kbdifmgr_add( p_kbdif );
    EXIT_IF_ERR( result, "kbdifmgr_add failed" );

_EXIT:
    return result;
}

kbdapi_event_result_t OverlayProcessing::kbdif_key_event_cb( void * p_instance, kbdapi_key_t * p_key )
{
    OverlayProcessing * p_overlay = ( OverlayProcessing * )p_instance;

    switch ( p_key->type )
    {
        case KBDAPI_KEY_TYPE_OVERLAY:
            p_overlay->overlay_key_event( p_key );
            break;

        /* Simple tap key: reports on both edges */
        case KBDAPI_KEY_TYPE_OVERLAY_TAP:
            if ( p_key->toggled_on == true )
            {
                send_packet( packet_t::OVERLAY_TAP, (uint8_t)event_t::TAP );
            }
            else if ( p_key->toggled_off == true )
            {
                send_packet( packet_t::OVERLAY_TAP, (uint8_t)event_t::RELEASE );
            }
            break;

        /* Simple hold key: reports on both edges */
        case KBDAPI_KEY_TYPE_OVERLAY_HOLD:
            if ( p_key->toggled_on == true )
            {
                send_packet( packet_t::OVERLAY_HOLD, (uint8_t)event_t::HOLD );
            }
            else if ( p_key->toggled_off == true )
            {
                send_packet( packet_t::OVERLAY_HOLD, (uint8_t)event_t::RELEASE );
            }
            break;

        default:
            return KBDAPI_EVENT_RESULT_IGNORED;
    }

    /* Consumed so the host never sees these keys as normal keycodes */
    return KBDAPI_EVENT_RESULT_CONSUMED;
}

kbdapi_event_result_t OverlayProcessing::kbdif_led_layer_change_event_cb( void * p_instance, kbdapi_led_layer_id_t layer_id )
{
    OverlayProcessing * p_overlay = ( OverlayProcessing * )p_instance;

    p_overlay->layer_current = layer_id;
    send_packet( packet_t::LAYER, layer_id );

    /* Ignored on purpose: the LED manager is the one that owns this event */
    return KBDAPI_EVENT_RESULT_IGNORED;
}

const kbdif_handlers_t OverlayProcessing::kbdif_handlers =
{
    .key_event_cb = kbdif_key_event_cb,
    .led_layer_change_event_cb = kbdif_led_layer_change_event_cb,
};

/****************************************************/
/*            OVERLAY_KEY state machine             */
/****************************************************/

void OverlayProcessing::overlay_key_event( kbdapi_key_t * p_key )
{
    if ( p_key->toggled_on == true )
    {
        switch ( overlay_state )
        {
            case state_t::IDLE:
                overlay_state = state_t::PRESSED;
                timer_set_ms( &hold_timer, OVERLAY_HOLD_TIMEOUT_MS );
                break;

            case state_t::FIRST_UP:
                /* Second press within the tap window is a double tap */
                overlay_state = state_t::DOUBLE_PRESSED;
                send_packet( packet_t::OVERLAY, (uint8_t)event_t::DOUBLE_TAP );
                break;

            default:
                break;
        }
    }
    else if ( p_key->toggled_off == true )
    {
        switch ( overlay_state )
        {
            case state_t::PRESSED:
                /* Released before the hold timeout: wait and see whether it is
                 * a plain tap or the beginning of a double tap */
                overlay_state = state_t::FIRST_UP;
                timer_set_ms( &tap_window_timer, OVERLAY_TAP_WINDOW_MS );
                break;

            case state_t::HOLD_ACTIVE:
            case state_t::DOUBLE_PRESSED:
                overlay_state = state_t::IDLE;
                send_packet( packet_t::OVERLAY, (uint8_t)event_t::RELEASE );
                break;

            default:
                break;
        }
    }
}


/* -*- mode: c++ -*-
 * Kbd_manager -- Manage keyboard general functionality, status and base communications
 * Copyright (C) 2026  Dygma Lab S.L.
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
 *
 */

#include "Kbd_commands.h"
#include "Kbd_manager.h"

#include "Ble_manager.h"
#include "Communications.h"
#include "Time_counter.h"
#include "Wire.h" // Arduino Wire wrapper for the NRF52 chips

#ifndef KEYBOARD_NEURON_FW_VERSION
#error "Firmware version is not specified."
    #define KEYBOARD_NEURON_FW_VERSION "N/A"
#endif

#ifndef APP_KS_LEFT_BOOT_ADDRESS
    #define APP_KS_LEFT_BOOT_ADDRESS    0x5A
#endif /* APP_KS_LEFT_BOOT_ADDRESS */

#ifndef APP_KS_RIGHT_BOOT_ADDRESS
    #define APP_KS_RIGHT_BOOT_ADDRESS   0x5B
#endif /* APP_KS_RIGHT_BOOT_ADDRESS */

#ifndef UPG_WIRE_CLOCK_FREQ_KHZ
    #define UPG_WIRE_CLOCK_FREQ_KHZ 100
#endif /* UPG_WIRE_CLOCK_FREQ_KHZ */

/* Key definitions */
typedef struct
{
    Communications_protocol::Devices device_com;
    kbdapi_side_type_t kbdapi_side_type;
} kbd_side_def_t;

static const kbd_side_def_t p_kbd_side_def_array[] =
{
    { .device_com = Communications_protocol::KEYSCANNER_DEFY_LEFT,  .kbdapi_side_type = KBDAPI_SIDE_TYPE_LEFT },
    { .device_com = Communications_protocol::BLE_DEFY_LEFT,         .kbdapi_side_type = KBDAPI_SIDE_TYPE_LEFT },
    { .device_com = Communications_protocol::RF_DEFY_LEFT,          .kbdapi_side_type = KBDAPI_SIDE_TYPE_LEFT },
    { .device_com = Communications_protocol::KEYSCANNER_DEFY_RIGHT, .kbdapi_side_type = KBDAPI_SIDE_TYPE_RIGHT },
    { .device_com = Communications_protocol::BLE_DEFY_RIGHT,        .kbdapi_side_type = KBDAPI_SIDE_TYPE_RIGHT },
    { .device_com = Communications_protocol::RF_DEFY_RIGHT,         .kbdapi_side_type = KBDAPI_SIDE_TYPE_RIGHT },
};
#define get_kbd_side_def( def, id ) _get_def( def, p_kbd_side_def_array, kbd_side_def_t, device_com, id )

/* External glue prototypes */
extern bool_t kbd_glue_left_wired_connected( void );
extern bool_t kbd_glue_right_wired_connected( void );
extern void kbd_glue_side_power_left_set( bool_t power );
extern void kbd_glue_side_power_right_set( bool_t power );
extern void kbd_glue_status_leds_init( void );

extern bool_t kbd_glue_slide_switch_position_usb( void );
extern bool_t kbd_glue_slide_switch_position_ble( void );

result_t KbdManager::init( void )
{
    result_t result = RESULT_ERR;

    /* Initialize the KBD command interface */
    result = kbdCommands.init();
    EXIT_IF_ERR( result, "kbdCommands.init" );

    /* Check if we can live without this reset sides */
    kbd_glue_side_power_left_set( true );
    kbd_glue_side_power_right_set( true );

    /* Initialize the status leds */
    kbd_glue_status_leds_init();


    Communications.callbacks.bind(CONNECTED, (
                                                [](const Packet &p)
                                                {
                                                   kbdManager.msg_connected_process( p );
                                                }));
    Communications.callbacks.bind(DISCONNECTED, (
                                                [](const Packet &p)
                                                {
                                                    kbdManager.msg_disconnected_process( p );
                                                }));

    Communications.callbacks.bind(HAS_KEYS, (
                                                [](const Packet &p)
                                                {
                                                    kbdManager.msg_has_keys_process( p );
                                                }));

_EXIT:
    return result;
}

inline void KbdManager::msg_connected_process( const Packet &p )
{
    if (p.header.device == BLE_DEFY_RIGHT) rightConnection[0] = BLE_DEFY_RIGHT;
    if (p.header.device == BLE_DEFY_LEFT) leftConnection[0] = BLE_DEFY_LEFT;
    if (p.header.device == KEYSCANNER_DEFY_LEFT)
        leftConnection[1] = BleManager.is_enabled() ? BLE_DEFY_LEFT : KEYSCANNER_DEFY_LEFT;
    if (p.header.device == KEYSCANNER_DEFY_RIGHT)
        rightConnection[1] = BleManager.is_enabled() ? BLE_DEFY_RIGHT : KEYSCANNER_DEFY_RIGHT;
    if (p.header.device == RF_DEFY_LEFT) leftConnection[2] = RF_DEFY_LEFT;
    if (p.header.device == RF_DEFY_RIGHT) rightConnection[2] = RF_DEFY_RIGHT;

    auto isKSLeftWired = leftSideWiredConnection();
    auto isKSRightWired = rightSideWiredConnection();
    LEDManager.com_mode_set( isKSLeftWired && isKSRightWired && !BleManager.is_enabled() );
    LEDManager.leds_enable();
}

inline void KbdManager::msg_disconnected_process( const Packet &p )
{
    if (p.header.device == BLE_DEFY_RIGHT) rightConnection[0] = BLE_DEFY_RIGHT;
    if (p.header.device == BLE_DEFY_LEFT) leftConnection[0] = BLE_DEFY_LEFT;
    if (p.header.device == KEYSCANNER_DEFY_LEFT)
        leftConnection[1] = BleManager.is_enabled() ? BLE_DEFY_LEFT : KEYSCANNER_DEFY_LEFT;
    if (p.header.device == KEYSCANNER_DEFY_RIGHT)
        rightConnection[1] = BleManager.is_enabled() ? BLE_DEFY_RIGHT : KEYSCANNER_DEFY_RIGHT;
    if (p.header.device == RF_DEFY_LEFT) leftConnection[2] = RF_DEFY_LEFT;
    if (p.header.device == RF_DEFY_RIGHT) rightConnection[2] = RF_DEFY_RIGHT;

    auto isKSLeftWired = leftSideWiredConnection();
    auto isKSRightWired = rightSideWiredConnection();
    LEDManager.com_mode_set( isKSLeftWired && isKSRightWired && !BleManager.is_enabled() );
    LEDManager.leds_enable();
}

inline void KbdManager::msg_has_keys_process( const Packet &packet )
{
    result_t result;

    const kbd_side_def_t * p_side_def;

    get_kbd_side_def( p_side_def, packet.header.device );

    result = kbdapi_key_data_add( p_side_def->kbdapi_side_type, packet.data, packet.header.size );
    ASSERT_DYGMA( result == RESULT_OK, "kbdapi_key_data_add failed" );

    UNUSED( result );
}

/********************************************************************/
/*                         Keyboard Control                         */
/********************************************************************/

void KbdManager::getChipID(char *cstring, uint16_t len)
{
    /*
        Returns the 64 bit unique device identifier.

        See: FICR - Factory information configuration registers on pag. 30 of the datasheet.

        returns a cstring.
    */

    snprintf(cstring, len, "%8lx%8lx", NRF_FICR->DEVICEID[1], NRF_FICR->DEVICEID[0]);
}

void KbdManager::get_chip_info(char *cstring, uint16_t len)
{
    /*
        See: FICR - Factory information configuration registers on pag. 30 of the datasheet.

        returns a cstring.
    */

    snprintf(cstring, len, "DEVICEID=%8lx%8lx\nPART=%lx\nVARIANT=%lx\nPACKAGE=%lx\nRAM=%ld\nFLASH=%ld", NRF_FICR->DEVICEID[1], NRF_FICR->DEVICEID[0],
             NRF_FICR->INFO.PART, NRF_FICR->INFO.VARIANT, NRF_FICR->INFO.PACKAGE, NRF_FICR->INFO.RAM, NRF_FICR->INFO.FLASH);
}

bool KbdManager::slideSwitchPositionUsb( void )
{
    return kbd_glue_slide_switch_position_usb();
}

bool KbdManager::slideSwitchPositionBle( void )
{
    return kbd_glue_slide_switch_position_ble();
}

void KbdManager::prepareForFlash( void )
{
    Wire::begin( UPG_WIRE_CLOCK_FREQ_KHZ );
}

/********************************************************************/
/*                          Keyboard sides                          */
/********************************************************************/

Communications_protocol::Devices KbdManager::leftHandDevice(void)
{
    for (const auto &connection : leftConnection)
    {
        if (connection != UNKNOWN)
        {
            return connection;
        }
    }

    return UNKNOWN;
}

Communications_protocol::Devices KbdManager::rightHandDevice(void)
{
    for (const auto &connection : rightConnection)
    {
        if (connection != UNKNOWN)
        {
            return connection;
        }
    }

    return UNKNOWN;
}

void KbdManager::setSidePower( bool power )
{
    // 0 -> reset keyboard side, 1 -> run keyboard side
    kbd_glue_side_power_left_set( power );
    kbd_glue_side_power_right_set( power );

    side_power = power;
}

bool KbdManager::getSidePower( void )
{
    return side_power;
}

uint8_t KbdManager::leftVersion( void )
{
    // TODO: Versions of keyscanner
    return 0;
    //  return KeyboardHands::hand_spi1.readVersion();
}

uint8_t KbdManager::rightVersion( void )
{
    // TODO: Versions of keyscanner
    return 0;

    //  return KeyboardHands::hand_spi2.readVersion();
}

void KbdManager::reset_sides()
{
    kbd_glue_side_power_left_set( false );
    kbd_glue_side_power_right_set( false );
    timer_delay_ms( 10 );
    kbd_glue_side_power_left_set( true );
    kbd_glue_side_power_right_set( true );
    timer_delay_ms( 50 ); // We should give a bit more time but for now lest leave it like this
}

void KbdManager::reset_right_side()
{
    kbd_glue_side_power_right_set( false );
    timer_delay_ms( 10 );
    kbd_glue_side_power_right_set( true );
    timer_delay_ms( 50 ); // We should give a bit more time but for now lest leave it like this
}

void KbdManager::reset_left_side()
{
    kbd_glue_side_power_left_set( false );
    timer_delay_ms( 10 );
    kbd_glue_side_power_left_set( true );
    timer_delay_ms( 50 ); // We should give a bit more time but for now lest leave it like this
}

bool KbdManager::rightSideWiredConnection( void )
{
    return kbd_glue_right_wired_connected();
}

bool KbdManager::leftSideWiredConnection( void )
{
    return kbd_glue_left_wired_connected();
}

uint8_t KbdManager::boot_address_left_get( void )
{
    return APP_KS_LEFT_BOOT_ADDRESS;
}

uint8_t KbdManager::boot_address_right_get( void )
{
    return APP_KS_RIGHT_BOOT_ADDRESS;
}

class KbdManager kbdManager;

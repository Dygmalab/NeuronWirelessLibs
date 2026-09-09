/*
 * Ble_hmi -- Manage Bluetooth low energy user control
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
 */

#include "Ble_hmi.h"
#include "kbd_if_manager.h"
#include "LEDEffect-Bluetooth-Pairing-Defy.h"
#include "LEDManager.h"

#define HMI_CHANNEL_ERASE_KEY_HOLD_TIMEOUT_MS       3000    /* 3 seconds */

/****************************************************/
/*                    HMI events                    */
/****************************************************/

inline void BleHmi::hmi_event_process( void * p_instance, blehmi_event_type_t event_type, blehmi_evt_param_t * p_param )
{
    if( event_cb == NULL )
    {
        return;
    }

    event_cb( p_instance, event_type, p_param );
}

/****************************************************/
/*                    HMI machine                   */
/****************************************************/

inline void BleHmi::hmi_state_set( blehmi_state_t blehmi_state )
{
    this->hmi_state = blehmi_state;
    mcu_sleep_postpone();
}

inline void BleHmi::hmi_state_enabled_set( void )
{
    result_t result = RESULT_ERR;

    /* Re-enable the keyboard key reporting */
    result = kbdapi_key_report_enable( &kbdapi_key_report_lock );
    ASSERT_DYGMA( result == RESULT_OK, "kbdapi_key_report_enable failed" );
    EXIT_IF_ERR( result, "kbdapi_key_report_enable failed" );

    /* Exit the Bluetooth led effect */
    hmi_led_effect_off();

    /* Go to the Enabled state */
    hmi_deactivate_req_flag = false;
    hmi_active_flag = false;
    hmi_state_set( BLEHMI_STATE_ENABLED );

_EXIT:
    return;
}

inline void BleHmi::hmi_state_disabled_set( void )
{
    /* First, make sure the HMI is de-activated */
    hmi_state_enabled_set();

    /* De-assert all the HMI controlling flags */
    hmi_activate_req_flag = false;
    hmi_deactivate_req_flag = false;
    hmi_active_flag = false;

    /* Now move to the disabled state */
    hmi_state_set( BLEHMI_STATE_DISABLED );
}

inline void BleHmi::hmi_state_active_set( void )
{
    result_t result = RESULT_ERR;

    /* Disable the keyboard key reporting */
    result = kbdapi_key_report_disable( &kbdapi_key_report_lock );
    ASSERT_DYGMA( result == RESULT_OK, "kbdapi_key_report_disable failed" );
    EXIT_IF_ERR( result, "kbdapi_key_report_disable failed" );

    /* Start the Bluetooth led effect */
    hmi_led_effect_on();

    /* Go to the Active state */
    hmi_activate_req_flag = false;
    hmi_active_flag = true;
    hmi_state_set( BLEHMI_STATE_ACTIVE );

_EXIT:
    return;
}

inline void BleHmi::hmi_state_reading_bond_code_set( void )
{
    /* Exit the Bluetooth led effect */
    hmi_led_effect_off();

    /* Prepare the encryption pin space */
    memset( &hmi_bond_code, 0x00, sizeof(hmi_bond_code) );
    hmi_bond_code_len = 0;

    /* Go to the HMI reading bond code state */
    hmi_state_set( BLEHMI_STATE_READING_BOND_CODE );
}

inline void BleHmi::hmi_state_channel_erase_key_wait_set( void )
{
    timer_set_ms( &hmi_timer, HMI_CHANNEL_ERASE_KEY_HOLD_TIMEOUT_MS );
    hmi_state_set( BLEHMI_STATE_CHANNEL_ERASE_KEY_WAIT );
}

inline void BleHmi::hmi_state_enabled_process( void )
{
    if( hmi_activate_req_flag == true )
    {
        hmi_state_active_set();
        return;
    }
}

inline void BleHmi::hmi_state_active_process( void )
{
    if( hmi_deactivate_req_flag == true )
    {
        hmi_state_enabled_set();
        return;
    }
}

inline void BleHmi::hmi_state_reading_bond_code_process( void )
{
    if( hmi_deactivate_req_flag == true )
    {
        hmi_state_enabled_set();
        return;
    }
}

inline void BleHmi::hmi_state_channel_erase_key_wait_process( void )
{
    blehmi_evt_param_t evt_param;

    /* Check the channel erase key press timeout has expired */
    if( timer_check( &hmi_timer ) == false )
    {
        return;
    }

    /* Report the channel erase has been requested */
    evt_param.channel_erase.channel_id = hmi_channel_erase_id;
    hmi_event_process( p_instance, BLEHMI_EVENT_TYPE_CHANNEL_ERASE, &evt_param);

    /* Wait for the erase key release */
    hmi_state_set( BLEHMI_STATE_CHANNEL_ERASE_KEY_RELEASE_WAIT );
}

inline void BleHmi::hmi_state_machine( void )
{
    switch( hmi_state )
    {
        case BLEHMI_STATE_DISABLED:

            /* Waiting for the HMI enable function call */

            break;

        case BLEHMI_STATE_ENABLED:

            hmi_state_enabled_process();

            break;

        case BLEHMI_STATE_ACTIVE:

            hmi_state_active_process();

            break;

        case BLEHMI_STATE_READING_BOND_CODE:

            hmi_state_reading_bond_code_process();

            break;

        case BLEHMI_STATE_CHANNEL_ERASE_KEY_WAIT:

            hmi_state_channel_erase_key_wait_process();

            break;

        case BLEHMI_STATE_CHANNEL_ERASE_KEY_RELEASE_WAIT:

            /* Just waiting for the erase key release event in hmi_key_process */

            break;

        default:

            ASSERT_DYGMA( false, "Unhandled BLE HMI state" );

            break;
    }
}

/****************************************************/
/*                    HMI Channels                  */
/****************************************************/

inline result_t BleHmi::hmi_channel_resolve( uint8_t * p_channel_id, kbdapi_key_t * p_key )
{
    const int colMapping[5][2] =
    {
        {1, 10},
        {2, 11},
        {3, 12},
        {4, 13},
        {5, 14}
    };

    for ( uint8_t i = 0; i < 5; i++ )
    {
        if ( ( p_key->coord.col == colMapping[i][0] || p_key->coord.col == colMapping[i][1] ) &&
             ( p_key->coord.row == 0                || p_key->coord.row == 1 ) )
        {
            *p_channel_id = i;
            return RESULT_OK;
        }
    }

    return RESULT_ERR;
}

/****************************************************/
/*                HMI Key processing                */
/****************************************************/

result_t BleHmi::hmi_key_num_to_ascii( kbdapi_key_t * p_key, char * p_ascii )
{
    uint8_t num;

    /*
     *  KBDAPI_KEY_TYPE_KBD_1_AND_EXCLAMATION_POINT,
     *  KBDAPI_KEY_TYPE_KBD_2_AND_AT,
     *  KBDAPI_KEY_TYPE_KBD_3_AND_POUND,
     *  KBDAPI_KEY_TYPE_KBD_4_AND_DOLLAR,
     *  KBDAPI_KEY_TYPE_KBD_5_AND_PERCENT,
     *  KBDAPI_KEY_TYPE_KBD_6_AND_CARAT,
     *  KBDAPI_KEY_TYPE_KBD_7_AND_AMPERSAND,
     *  KBDAPI_KEY_TYPE_KBD_8_AND_ASTERISK,
     *  KBDAPI_KEY_TYPE_KBD_9_AND_LEFT_PAREN,
     *  KBDAPI_KEY_TYPE_KBD_0_AND_RIGHT_PAREN,
    */
    if (( p_key->type < KBDAPI_KEY_TYPE_KBD_1_AND_EXCLAMATION_POINT) || (p_key->type > KBDAPI_KEY_TYPE_KBD_0_AND_RIGHT_PAREN))
    {
        /* The key is not numeric */
        return RESULT_ERR;
    }

    /* Get the numeric base of the number */
    num = p_key->type - KBDAPI_KEY_TYPE_KBD_1_AND_EXCLAMATION_POINT + 1;

    /* Calculate the ascii representation of the key */
    *p_ascii = ( num != 10 ) ? ( num + '0' ) : '0';

    return RESULT_OK;
}

inline result_t BleHmi::hmi_key_enabled_process( kbdapi_key_t * p_key )
{
    /* Accepting the Bluetooth pairing key when it is toggled on. */
    if ( p_key->type != KBDAPI_KEY_TYPE_BLUETOOTH_PAIRING || p_key->toggled_on == false )
    {
        return RESULT_ERR;  /* The key is ignored */
    }

    /* Enter the HMI Active state */
    hmi_activate();

    return RESULT_OK; /* The key is consumed */
}

inline void BleHmi::hmi_key_active_channel_change_process( kbdapi_key_t * p_key, uint8_t channel_id )
{
    blehmi_evt_param_t evt_param;

    /* Changing the channel upon the key toggled off */
    if( p_key->toggled_off == false )
    {
        return;
    }

    /* Report the chosen channel */
    evt_param.channel_change.channel_id = channel_id;
    hmi_event_process( p_instance, BLEHMI_EVENT_TYPE_CHANNEL_CHANGE, &evt_param);
}

inline void BleHmi::hmi_key_active_channel_erase_process( kbdapi_key_t * p_key, uint8_t channel_id )
{
    /* Starting the channel erase process upon the key toggle on */
    if( p_key->toggled_on == false )
    {
        return;
    }

    /* Save the channel id chosen for the erase */
    hmi_channel_erase_id = channel_id;

    /* Move to the channel erase key wait state */
    hmi_state_channel_erase_key_wait_set();
}

inline result_t BleHmi::hmi_key_active_process( kbdapi_key_t * p_key )
{
    result_t result = RESULT_ERR;
    uint8_t channel_id;

    /* Resolve the channel id to be used */
    result = hmi_channel_resolve( &channel_id, p_key );
    EXIT_IF_NOK( result );

    switch( p_key->coord.row )
    {
        case 0:

            /* Row 0: key 1, key 2, key 3, key 4, key 5, key 6, key 7, key 8, key 9, key 0 */
            hmi_key_active_channel_change_process( p_key, channel_id );

            break;

        case 1:

            /* Row 1: key Q, key W, key E, key R, key T, key Y, key U, key I, key O, key PT */
            hmi_key_active_channel_erase_process( p_key, channel_id );

            break;

        default:
            break;
    }

_EXIT:
    /* In case a Channel ID has not been resolved */
    if ( result == RESULT_ERR )
    {
        /* Check the Bluetooth Pairing key has been press in the HMI active state */
        if( p_key->type == KBDAPI_KEY_TYPE_BLUETOOTH_PAIRING )
        {
            if( p_key->toggled_on == true )
            {
                /* Exit the HMI Active state */
                hmi_deactivate();
            }

            return RESULT_OK; /* The Bluetooth pairing key is consumed */
        }

        return RESULT_ERR;  /* The key is ignored */
    }

    return RESULT_OK;
}

inline result_t BleHmi::hmi_key_reading_bond_code_process( kbdapi_key_t * p_key )
{
    result_t result = RESULT_ERR;
    blehmi_evt_param_t evt_param;

    /* The key is read upon its release */
    if( p_key->toggled_off == false )
    {
        return RESULT_OK; /* The key is always consumed in HMI reading bond code state */
    }

    /* Convert the key to numeric ASCII */
    result = hmi_key_num_to_ascii( p_key, (char *)&hmi_bond_code.code[ hmi_bond_code_len ] );
    EXIT_IF_NOK( result );

    /* The numeric ascii conversion was successful - increase the pin length */
    hmi_bond_code_len++;

    if( hmi_bond_code_len < sizeof( hmi_bond_code ) )
    {
        return RESULT_OK; /* The key is always consumed in HMI HMI reading bond code state */
    }

    /* Return to the HMI active state */
    hmi_state_active_set();

    /* Report the bond code is ready */
    evt_param.bond_code_ready.bond_code = hmi_bond_code;
    hmi_event_process( p_instance, BLEHMI_EVENT_TYPE_BOND_CODE_READY, &evt_param);

_EXIT:
    return RESULT_OK; /* The key is always consumed in HMI HMI reading bond code state */
}

inline result_t BleHmi::hmi_key_channel_erase_key_wait_process( kbdapi_key_t * p_key )
{
    result_t result = RESULT_ERR;
    uint8_t channel_id;

    /* Resolve the channel id to be used */
    result = hmi_channel_resolve( &channel_id, p_key );
    EXIT_IF_NOK( result );

    /* Check the key is the channel erase key in process */
    if( channel_id != hmi_channel_erase_id || p_key->coord.row != 1 )
    {
        return RESULT_OK; /* The key is always consumed in HMI channel erase wait state */
    }

    /* Check if the key has been released */
    if( p_key->toggled_off == true )
    {
        /* Return to the HMI active state */
        hmi_state_active_set();
    }

_EXIT:
    return RESULT_OK; /* The key is always consumed in HMI channel erase wait state */
}

inline result_t BleHmi::hmi_key_process( kbdapi_key_t * p_key )
{
    result_t result = RESULT_ERR;

    /* If the HMI is disabled, just ignore the keypress */
    if( hmi_is_enabled() == false )
    {
        return RESULT_ERR;
    }

    switch( hmi_state )
    {
        case BLEHMI_STATE_ENABLED:

            result = hmi_key_enabled_process( p_key );

            break;

        case BLEHMI_STATE_ACTIVE:

            result = hmi_key_active_process( p_key );

            break;

        case BLEHMI_STATE_READING_BOND_CODE:

            result = hmi_key_reading_bond_code_process( p_key );

            break;

        case BLEHMI_STATE_CHANNEL_ERASE_KEY_WAIT:
        case BLEHMI_STATE_CHANNEL_ERASE_KEY_RELEASE_WAIT:

            result = hmi_key_channel_erase_key_wait_process( p_key );

            break;

        default:

            ASSERT_DYGMA( false, "Keypress in invalid BLE HMI state" );

            break;
    }

    return result;
}

/****************************************************/
/*                  HMI LED effects                 */
/****************************************************/

inline void BleHmi::hmi_led_effect_set( uint8_t channel_con_id, uint8_t channel_adv_id, bool_t erase_status )
{
    LEDBluetoothPairingDefy.setPairedChannels( hmi_channels_bonded_mask );
    LEDBluetoothPairingDefy.setConnectedChannel( channel_con_id );
    LEDBluetoothPairingDefy.setAvertisingModeOn( channel_adv_id );
    LEDBluetoothPairingDefy.setEreaseDone( erase_status );
//    LEDBluetoothPairingDefy.setDefyId( defy_id );
}

inline void BleHmi::hmi_led_effect_on( void )
{
    LEDManager.led_effect_set_prio( LEDEffect::LED_EFFECT_TYPE_BLUETOOTH_PAIRING );
}

inline void BleHmi::hmi_led_effect_off( void )
{
    /* Exit the Bluetooth led effect */
    LEDManager.update_brightness( LEDManager::BRIGHTNESS_LED_EFFECT_BT_LED_EFFECT, false );
    LEDManager.led_effect_reset_prio();
    LEDManager.led_effect_set( LEDEffect::LED_EFFECT_TYPE_DEFAULT ); // Disable LED fade effect.
}

inline void BleHmi::hmi_led_effect_update( uint8_t channel_con_id, uint8_t channel_adv_id, bool_t erase_status )
{
    hmi_led_effect_set( channel_con_id, channel_adv_id, erase_status );

    /* Update the led effect only if the HMI interface is active */
    if( hmi_is_active() == false )
    {
        return;
    }

    LEDManager.led_effect_refresh();
}

inline void BleHmi::hmi_led_effect_adv( void )
{
    hmi_led_effect_update( LedModeSerializable_BluetoothPairing::Channels::NOT_CONNECTED, hmi_channel_current_id, false );

    /* In case of the advertising, we want to activate the LED effect */
    if( hmi_is_active() == false )
    {
        hmi_activate();
    }
}

inline void BleHmi::hmi_led_effect_reading_bond_code( void )
{
    ASSERT_DYGMA( hmi_is_active() == true, "BLE HMI is not expected to be inactive when entering the pairing mode" );

    hmi_state_reading_bond_code_set();
}

/****************************************************/
/*                Keyboard Interface                */
/****************************************************/

result_t BleHmi::kbdif_initialize()
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

inline kbdapi_event_result_t BleHmi::kbdif_key_event_process( kbdapi_key_t * p_key )
{
    result_t result = RESULT_ERR;

    result = hmi_key_process( p_key );

    return ( result == RESULT_OK ) ? KBDAPI_EVENT_RESULT_CONSUMED : KBDAPI_EVENT_RESULT_IGNORED;
}

kbdapi_event_result_t BleHmi::kbdif_key_event_cb( void * p_instance, kbdapi_key_t * p_key )
{
    BleHmi * p_BleHmi = ( BleHmi *)p_instance;

    return p_BleHmi->kbdif_key_event_process( p_key );
}

const kbdif_handlers_t BleHmi::kbdif_handlers =
{
    .key_event_cb = kbdif_key_event_cb,
    .command_event_cb = NULL,
};

/****************************************************/
/*                        API                       */
/****************************************************/

result_t BleHmi::hmi_init( const blehmi_config_t * p_config )
{
    result_t result = RESULT_ERR;

    /* Initialize the keyboard interface */
    result = kbdif_initialize();
    EXIT_IF_ERR( result, "kbdif_initialize failed" );

    /* Initialize the key report lock */
    result = kbdapi_key_report_lock_init( &kbdapi_key_report_lock );
    EXIT_IF_ERR( result, "kbdapi_key_report_lock_init failed" );

    /* Event callback */
    this->p_instance = p_config->p_instance;
    this->event_cb = p_config->event_cb;

    /* Initialize the flags */
    hmi_activate_req_flag = false;
    hmi_deactivate_req_flag = false;
    hmi_active_flag = false;

    /* Initialize channel IDs */
    hmi_channel_current_id = 0;
    hmi_channel_erase_id = 0;
    hmi_channels_bonded_mask = 0x00;

    /* Set the initial state */
    hmi_state = BLEHMI_STATE_DISABLED;

_EXIT:
    return result;
}

void BleHmi::hmi_enable( void )
{
    if( hmi_is_enabled() == true )
    {
        /* Already enabled */
        return;
    }

    hmi_state_enabled_set();
}

void BleHmi::hmi_disable( void )
{
    if( hmi_is_enabled() == false )
    {
        /* Already disabled */
        return;
    }

    hmi_state_disabled_set();
}

void BleHmi::hmi_activate( void )
{
    if( hmi_is_enabled() == false )
    {
        ASSERT_DYGMA( false, "BLE HMI trying to activate from invalid state" );
        return;
    }

    hmi_activate_req_flag = true;
}

void BleHmi::hmi_deactivate( void )
{
    ASSERT_DYGMA( hmi_is_active(), "BLE HMI trying to deactivate from invalid state" );

    hmi_deactivate_req_flag = true;
}

bool BleHmi::hmi_is_enabled( void )
{
    return ( hmi_state == BLEHMI_STATE_DISABLED ) ? false : true;
}

bool BleHmi::hmi_is_active( void )
{
    return hmi_active_flag;
}

void BleHmi::hmi_advertise( uint8_t channel_current_id, uint8_t channels_bonded_mask )
{
    hmi_channel_current_id = channel_current_id;
    hmi_channels_bonded_mask = channels_bonded_mask;

    hmi_led_effect_adv();
}

void BleHmi::hmi_read_bond_code( void )
{
    hmi_led_effect_reading_bond_code();
}

void BleHmi::hmi_update( uint8_t channel_current_id, uint8_t channels_bonded_mask )
{
    hmi_channel_current_id = channel_current_id;
    hmi_channels_bonded_mask = channels_bonded_mask;

    hmi_led_effect_update( LedModeSerializable_BluetoothPairing::Channels::NOT_CONNECTED, hmi_channel_current_id, false );
}

void BleHmi::hmi_run( void )
{
    hmi_state_machine();
}

class BleHmi BleHmi;

/*
 * Ble_manager -- Manage Bluetooth low energy status and functions in wireless devices
 * Copyright (C) 2020  Dygma Lab S.L.
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

#include "Ble_composite_dev.h"
#include "Ble_manager.h"
#include "Config_manager.h"
//#include "kaleidoscope/Runtime.h"
//#include "keyboard_api.h"
#include "LEDEffect-Bluetooth-Pairing-Defy.h"
//#include "LEDManager.h"
//#include "FirmwareVersion.h"
//
//#include "Do_once.h"
#include "kbd_if_manager.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"

//void device_name_evt_handler(void);
//
//#define KEY_ERASE_HOLD_TIMEOUT_MS   3000
//
//Do_once clear_pin_digits_count;
//

static const ble_device_name_t ble_device_name_local = { BLE_DEVICE_NAME };

#define BLE_MANAGER_DEBUG_LOG   1
#if BLE_MANAGER_DEBUG_LOG
#warning "The BLE Manager logs are enabled"
    #define BLE_LOG_INFO(...)       NRF_LOG_INFO( __VA_ARGS__)
    #define BLE_LOG_DEBUG(...)      NRF_LOG_DEBUG( __VA_ARGS__)
    #define BLE_LOG_FLUSH()         NRF_LOG_FLUSH()
#else  /* BLE_MANAGER_DEBUG_LOG */
    #define BLE_LOG_INFO(...)
    #define BLE_LOG_DEBUG(...)
    #define BLE_LOG_FLUSH()
#endif /* BLE_MANAGER_DEBUG_LOG */

//void BleManager::enable(void)
//{
//    TinyUSBDevice.detach();
//
//    // Init BLE and activate BLE advertising.
//    ble_module_init();
//    ble_goto_white_list_advertising_mode();
//    uint32_t peerCount = pm_peer_count();
//
//    BLE_LOG_INFO("Ble_manager: %i devices in flash.", peerCount);
//
//    pm_peer_id_t peer_list[peerCount];
//    pm_peer_id_list(peer_list,
//                    &peerCount,
//                    PM_PEER_ID_INVALID,
//                    PM_PEER_ID_LIST_ALL_ID);
//
//    // For all devices in the FDS memory region, if they exist, add it to the flashStorage memory region.
//    for (uint32_t i = 0; i < peerCount; i++)
//    {
//        bool found = false;
//        for (auto &paired_device : p_connections_config->cons)
//        {
//            if (peer_list[i] != PM_PEER_ID_INVALID &&
//                peer_list[i] == paired_device.peer_id)
//            {
//                found = true;
//                break;
//            }
//        }
//
//        if (!found)
//        {
//            // Take the first invalid and set it has valid
//            for (auto &paired_device : p_connections_config->cons)
//            {
//                if (paired_device.peer_id == PM_PEER_ID_INVALID)
//                {
//                    /* Reset the connection in the memory with the new peer id */
//                    cfgmem_connection_reset( &paired_device );
//                    cfgmem_con_peer_id_save( &paired_device, peer_list[i] );
//
//                    break;
//                }
//            }
//        }
//    }
//
//    // For all devices in the flash storage check if exist in the FDS
//    for (auto &paired_device : p_connections_config->cons)
//    {
//        bool found = false;
//        for (uint32_t i = 0; i < peerCount; i++)
//        {
//#warning "test"
//            volatile pm_peer_id_t test_peer_id = peer_list[i];
//
//            if (peer_list[i] != PM_PEER_ID_INVALID &&
//                peer_list[i] == paired_device.peer_id)
//            {
//                found = true;
//                break;
//            }
//        }
//
//        if (!found)
//        {
//            /* Reset the connection in the memory */
//            cfgmem_connection_reset( &paired_device );
//        }
//    }
//}

/********************************************************************/
/*                 Low Level BLE - Composite device                 */
/********************************************************************/

result_t BleManager::ble_ll_init( void )
{
    result_t result = RESULT_ERR;
    blecdev_conf_t config;

    config.device_name_local = ble_device_name_local;
    config.p_instance = this;
    config.event_cb = ble_ll_event_cb;

    result = blecdev_init( &config );
    EXIT_IF_ERR( result, "blecdev_init failed" );

_EXIT:
    return result;
}

void BleManager::ble_ll_whitelist_configure( void )
{
//    const channel_t * p_current_channel = &p_config->channels[p_config->current_channel_id];
//
//    /*
//        If it doesn't have any device on the channel, it goes into advertising mode with a white
//        list so that any device can find it.
//    */
//    if ( p_current_channel->peer_id == PM_PEER_ID_INVALID )
//    {
//        BLE_LOG_INFO("Ble_manager: Whitelist deactivated.");
//
//        blecdev_set_whitelist(false); // this allows the device to be visible to all devices
//    }
//    else
//    {
//        BLE_LOG_INFO("Ble_manager: Whitelist activated.");
//
//        blecdev_set_whitelist(true); // this allows the device to be visible ONLY for the paired devices
//    }
}

void BleManager::ble_ll_event_process( blecdev_event_t event )
{
    switch( event )
    {
        default:

            ASSERT_DYGMA( false, "Unhandled BLE Composite Device event" )

            break;
    }
}

void BleManager::ble_ll_event_cb( void * p_instance, blecdev_event_t event )
{
    BleManager * p_bleManager = (BleManager *)p_instance;
    p_bleManager->ble_ll_event_process( event );
}

/********************************************************************/
/*                             Channels                             */
/********************************************************************/

void BleManager::channel_init( const channel_t * p_channel )
{
    bool_t channel_paired = ( p_channel->peer_id == PM_PEER_ID_INVALID ) ? false : true;

    channel_paired_set( p_channel, channel_paired );

    BLE_LOG_DEBUG("Ble_manager: Channel %i -> %d", p_channel->id, channel_paired );
}

void BleManager::channel_paired_set( const channel_t * p_channel, bool paired )
{
    uint8_t channel_mask = ( 1 << p_channel->id );

    if ( paired )
    {
        channels_paired_mask |= channel_mask;
    }
    else
    {
        channels_paired_mask &= ~channel_mask;
    }

    LEDBluetoothPairingDefy.setPairedChannels( channels_paired_mask );
}

void BleManager::channels_init( void )
{
    uint8_t i;
    for ( i = 0; i < BLE_CHANNELS_COUNT; i++ )
    {
        channel_init( &p_config->channels[i] );
    }

    BLE_LOG_DEBUG("Ble_manager: %i channels in total.", i);

    LEDBluetoothPairingDefy.setConnectedChannel(NOT_CONNECTED);
    LEDBluetoothPairingDefy.setEreaseDone(false);
}

//void BleManager::update_channel_and_name(void)
//{
//    set_current_channel(p_connections_config->current_channel);
//
//    if (ble_device_name != nullptr)
//    {
//        set_device_name( ble_device_name );
//    }
//    else
//    {
//        set_device_name( p_connections_config->device_name_local );
//    }
//
//    pm_peer_id_t active_connection_peer_id = p_connections_config->cons[p_connections_config->current_channel].peer_id;
//
//    /*
//        If it doesn't have any device on the channel, it goes into advertising mode with a white
//        list so that any device can find it.
//    */
//    if (active_connection_peer_id == PM_PEER_ID_INVALID)
//    {
//        BLE_LOG_INFO("Ble_manager: Whitelist deactivated.");
//
//        set_whitelist(false); // this allows the device to be visible to all devices
//    }
//    else
//    {
//        BLE_LOG_INFO("Ble_manager: Whitelist activated.");
//
//        set_whitelist(true); // this allows the device to be visible ONLY for the paired devices
//    }
//}
//
//void BleManager::timer_save_conn_run(uint32_t timeout_ms)
//{
//    /*
//        Instead of saving the data immediately, a timer is started that saves it Xms later, so as not to
//        interfere with the use of the flash memory of the peer_manager module of the SDK.
//    */
//
//    if (trigger_save_conn_timer)
//    {
//        trigger_save_conn_timer = false;
//
//        BLE_LOG_DEBUG("Ble_manager: Start timer save connection.");
//
//        timer_set_ms( &timer_save_new_conn, timeout_ms );
//        timer_save_conn_start_count = true;
//    }
//
//    if (timer_save_conn_start_count)
//    {
//        if ( timer_check(&timer_save_new_conn) == true )
//        {
//            BLE_LOG_DEBUG("Ble_manager: Timeout timer save connection.");
//
//            save_connection();
//            timer_save_conn_start_count = false;
//        }
//
//        // While this timer is running, the periodic update timer remains reset.
//    }
//}
//
//void BleManager::save_connection(void)
//{
//    const uint8_t * previous_dev_addr = p_connections_config->cons[p_connections_config->current_channel].device_address;
//
//    BLE_LOG_DEBUG("Ble_manager: previous_dev_addr = %02X %02X %02X %02X %02X %02X",
//                  previous_dev_addr[0], previous_dev_addr[1],
//                  previous_dev_addr[2], previous_dev_addr[3],
//                  previous_dev_addr[4], previous_dev_addr[5]);
//
//
//    BLE_LOG_DEBUG("Ble_manager: get_connected_device_address() = %02X %02X %02X %02X %02X %02X",
//                  get_connected_device_address()[0], get_connected_device_address()[1],
//                  get_connected_device_address()[2], get_connected_device_address()[3],
//                  get_connected_device_address()[4], get_connected_device_address()[5]);
//
//    uint16_t previous_peer_id = p_connections_config->cons[p_connections_config->current_channel].peer_id;
//
//    BLE_LOG_DEBUG("Ble_manager: previous_peer_id = %i", previous_peer_id);
//    BLE_LOG_DEBUG("Ble_manager: get_connected_peer_id() = %i", get_connected_peer_id());
//
//    // It only saves it if it is different from the previous one.
//    if ( (memcmp(previous_dev_addr, get_connected_device_address(), BLE_ADDRESS_LEN) != 0) ||
//        (previous_peer_id != get_connected_peer_id()) )
//    {
//        BLE_LOG_DEBUG("*** Ble_manager: saving cons");
//
//        // Save new connection in the memory.
//        cfgmem_con_device_address_save( &p_connections_config->cons[p_connections_config->current_channel], get_connected_device_address( ) );
//        cfgmem_con_peer_id_save( &p_connections_config->cons[p_connections_config->current_channel], get_connected_peer_id( ) );
//
//        BLE_LOG_DEBUG("Ble_manager: Saving new connection...");
//    }
//}
//
//void BleManager::timer_save_name_run(uint32_t timeout_ms)
//{
//    /*
//        Instead of saving the data immediately, a timer is started that saves it Xms later, so as not to
//        interfere with the use of the flash memory of the peer_manager module of the SDK.
//    */
//
//    if (trigger_save_name_timer)
//    {
//        trigger_save_name_timer = false;
//
//        BLE_LOG_DEBUG("Ble_manager: Start timer save name.");
//
//        timer_set_ms( &timer_save_new_name, timeout_ms );
//        timer_save_name_start_count = true;
//    }
//
//    if (timer_save_name_start_count)
//    {
//        if ( timer_check(&timer_save_new_name) == true )
//        {
//            BLE_LOG_DEBUG("Ble_manager: Timeout timer save name.");
//
//            save_device_name();
//            timer_save_name_start_count = false;
//        }
//
//        // While this timer is running, the periodic update timer remains reset.
//    }
//}
//
//void BleManager::save_device_name(void)
//{
//    const char * previous_dev_name = p_connections_config->cons[p_connections_config->current_channel].device_name;
//
//    uint8_t empty[BLE_DEVICE_NAME_LEN] = {};  // Init all with 0.
//    if (memcmp(previous_dev_name, empty, BLE_DEVICE_NAME_LEN) != 0)  // If the channel was in use.
//    {
//        BLE_LOG_DEBUG("Ble_manager: previous_dev_name = %s", previous_dev_name);
//    }
//    else
//    {
//        BLE_LOG_DEBUG("Ble_manager: previous_dev_name =");
//    }
//
//    BLE_LOG_DEBUG("Ble_manager: get_connected_device_name_ptr() = %s", get_connected_device_name_ptr());
//
//    // It only saves it, if it is different from the previous one.
//    if (memcmp(previous_dev_name, get_connected_device_name_ptr(), BLE_DEVICE_NAME_LEN) != 0)
//    {
//        // Save the new device name in the memory.
//        cfgmem_con_device_name_save( &p_connections_config->cons[p_connections_config->current_channel], (char *)get_connected_device_name_ptr() );
//
//        // Save it in flash memory.
//        BLE_LOG_DEBUG("Ble_manager: Saving device name...");
//    }
//}
//
//void BleManager::set_pairing_key_press(bool press)
//{
//    pairing_key_press = press;
//}
//
//bool BleManager::get_pairing_key_press(void)
//{
//    return pairing_key_press;
//}
//
//bool BleManager::is_num_key( kbdapi_key_t * p_key )
//{
//    /*
//     *  KBDAPI_KEY_TYPE_KBD_1_AND_EXCLAMATION_POINT,
//     *  KBDAPI_KEY_TYPE_KBD_2_AND_AT,
//     *  KBDAPI_KEY_TYPE_KBD_3_AND_POUND,
//     *  KBDAPI_KEY_TYPE_KBD_4_AND_DOLLAR,
//     *  KBDAPI_KEY_TYPE_KBD_5_AND_PERCENT,
//     *  KBDAPI_KEY_TYPE_KBD_6_AND_CARAT,
//     *  KBDAPI_KEY_TYPE_KBD_7_AND_AMPERSAND,
//     *  KBDAPI_KEY_TYPE_KBD_8_AND_ASTERISK,
//     *  KBDAPI_KEY_TYPE_KBD_9_AND_LEFT_PAREN,
//     *  KBDAPI_KEY_TYPE_KBD_0_AND_RIGHT_PAREN,
//    */
//    if (( p_key->type >= KBDAPI_KEY_TYPE_KBD_1_AND_EXCLAMATION_POINT) && (p_key->type <= KBDAPI_KEY_TYPE_KBD_0_AND_RIGHT_PAREN))
//    {
//        return true;
//    }
//
//    return false;
//}
//
//char BleManager::raw_key_to_ascii( kbdapi_key_t * p_key )
//{
//    uint8_t num = p_key->type - KBDAPI_KEY_TYPE_KBD_1_AND_EXCLAMATION_POINT + 1;
//
//    if (num != 10)
//    {
//        return (num + '0'); // To ASCII num.
//    }
//    else
//    {
//        return '0';
//    }
//}
//
//void BleManager::erase_paired_device(uint8_t index_channel)
//{
//    if ( p_connections_config->cons[index_channel].peer_id != PM_PEER_ID_INVALID)
//    {
//        BLE_LOG_DEBUG("Ble_manager: Deleting paired device on channel %i...", index_channel);
//        BLE_LOG_FLUSH();
//
//        connectionState[index_channel].longPress = true;
//        uint8_t peerCount = pm_peer_count();
//        pm_peer_id_t peer_list[peerCount];
//        uint32_t list_size = peerCount;
//        pm_peer_id_list(peer_list, &list_size, PM_PEER_ID_INVALID, PM_PEER_ID_LIST_ALL_ID);
//
//        for (uint8_t i = 0; i < peerCount; ++i)
//        {
//            if (peer_list[i] != PM_PEER_ID_INVALID && peer_list[i] == p_connections_config->cons[index_channel].peer_id)
//            {
//                delete_peer_by_id(p_connections_config->cons[index_channel].peer_id);
//            }
//        }
//
//        /* Reset the connection in the memory */
//        cfgmem_connection_reset( &p_connections_config->cons[index_channel] );
//
//        set_paired_channel_led(index_channel, false); // set channel related bit to 0
//
//        if (index_channel == p_connections_config->current_channel)
//        {
//            LEDBluetoothPairingDefy.setConnectedChannel(NOT_CONNECTED);
//            ble_disconnect();
//            // We could remove this reset but a weird led effect happends when the channel is the same
//            reset_mcu();
//        }
//
//        LEDBluetoothPairingDefy.setEreaseDone(true);
//        send_led_mode(); // this is to update the led effect.
//        LEDBluetoothPairingDefy.setEreaseDone(false);
//    }
//}
//
//void BleManager::bt_layer_enter(void)
//{
//    /* Start the Bluetooth led effect */
//    LEDManager.led_effect_set_prio( LEDEffect::LED_EFFECT_TYPE_BLUETOOTH_PAIRING );
//
//    /* Disable the keyboard key reporting */
//    kbdapi_key_report_disable( &kbdapi_key_report_lock );
//
//    showing_bt_layer = true;
//}
//
//void BleManager::bt_layer_exit(void)
//{
//    showing_bt_layer = false;
//
//    /* Exit the Bluetooth led effect */
//    LEDManager.update_brightness( LEDManager::BRIGHTNESS_LED_EFFECT_BT_LED_EFFECT, false );
//    LEDManager.led_effect_reset_prio();
//    LEDManager.led_effect_set( LEDEffect::LED_EFFECT_TYPE_DEFAULT ); // Disable LED fade effect.
//
//    /* Re-enable the keyboard key reporting */
//    kbdapi_key_report_enable( &kbdapi_key_report_lock );
//}
//
//void BleManager::exit_pairing_mode(void)
//{
//    BLE_LOG_DEBUG("Ble_manager: Exit pairing mode.");
//    BLE_LOG_FLUSH();
//
//    /* Exit the Bluetooth layer */
//    bt_layer_exit();
//}
//
//void BleManager::send_led_mode(void)
//{
//    if( showing_bt_layer == false)
//    {
//        bt_layer_enter();
//    }
//    else
//    {
//        LEDManager.led_effect_refresh();
//    }
//
//    LEDManager.update_brightness( LEDManager::BRIGHTNESS_LED_EFFECT_BT_LED_EFFECT, true );
//}
//
//void BleManager::set_channel_in_use( kbdapi_key_t * p_key )
//{
//    const int colMapping[5][2] =
//    {
//        {1, 10},
//        {2, 11},
//        {3, 12},
//        {4, 13},
//        {5, 14}
//    };
//
//    for (int i = 0; i < 5; i++)
//    {
//        if ( (p_key->coord.col == colMapping[i][0] || p_key->coord.col == colMapping[i][1]) &&
//            (p_key->coord.row == 0 || p_key->coord.row == 1) )
//        {
//            channel_in_use = static_cast<Channels>(i);
//
//            break;
//        }
//    }
//}
//
//void BleManager::set_bt_name_from_specifications(const char *spec)
//{
//    ble_device_name = spec;
//}

/****************************************************/
/*                   State machine                  */
/****************************************************/

inline void BleManager::state_set( blem_state_t blem_state )
{
    this->state = blem_state;
    mcu_sleep_postpone();
}

inline void BleManager::state_disabled_process()
{
    if( enable_request_flag == true )
    {
        enable_request_flag = false;
        state_set( BLEM_STATE_ENABLE );
        return;
    }
}

inline void BleManager::state_enable_process()
{
    result_t result = RESULT_ERR;
    blecdev_enable_conf_t enable_config;

    enable_config.current_channel_id = p_config->current_channel_id;

    result = blecdev_enable( &enable_config );
    ASSERT_DYGMA( result == RESULT_OK, "blecdev_enable failed" );
    EXIT_IF_ERR( result, "blecdev_enable failed" );

    state_set( BLEM_STATE_ENABLING );

_EXIT:
    return;
}

inline void BleManager::state_enabling_process()
{
    ASSERT_DYGMA( false, "state_enabling_process not implemented" );

    enabled_flag = true;
    state_set( BLEM_STATE_ENABLED );
}

inline void BleManager::state_machine()
{
    switch( state )
    {
        case BLEM_STATE_DISABLED:

            state_disabled_process();

            break;

        case BLEM_STATE_ENABLE:

            state_enable_process();

            break;

        case BLEM_STATE_ENABLING:

            state_enabling_process();

            break;

        case BLEM_STATE_ENABLED:
            break;

        case BLEM_STATE_DISABLE:
            break;

        default:

            ASSERT_DYGMA( false, "Unhanled BLE Manager state" );

            break;
    }
}

/****************************************************/
/*                Keyboard Interface                */
/****************************************************/

result_t BleManager::kbdif_initialize()
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

kbdapi_event_result_t BleManager::kbdif_key_event_process( kbdapi_key_t * p_key )
{
    kbdapi_event_result_t result = KBDAPI_EVENT_RESULT_IGNORED;

//    /* Exit conditions. */
//    if (!ble_innited())
//    {
//        if (p_key->type == KBDAPI_KEY_TYPE_BLUETOOTH_PAIRING && p_key->toggled_on && FirmwareVersion::keyboard_is_wireless())
//        {
//            auto const &keyScanner = kaleidoscope::Runtime.device().keyScanner();
//            auto isDefyLeftWired = keyScanner.leftSideWiredConnection();
//            auto isDefyRightWired = keyScanner.rightSideWiredConnection();
//            auto isWiredMode = isDefyLeftWired || isDefyRightWired;
//
//            if (isWiredMode)
//            {
//                cfgmem_force_ble_save( true );
//
//                set_pairing_key_press(true);
//
//                reset_mcu();
//
//            }
//        }
//        else
//        {
//            return KBDAPI_EVENT_RESULT_IGNORED;
//        }
//    }
//
//    if ( p_key->toggled_off && mitm_activated && is_num_key( p_key ))
//    {
//        static uint8_t pin_digits_count = 0;
//
//        encryption_pin_number[pin_digits_count] = raw_key_to_ascii(p_key);
//
//        BLE_LOG_DEBUG("Ble_manager: encryption_pin_number[%d] = %c",
//                      pin_digits_count,
//                      encryption_pin_number[pin_digits_count]);
//        BLE_LOG_FLUSH();
//
//        pin_digits_count++;
//
//        if (pin_digits_count == 6) // PIN numbers has 6 digits.
//        {
//            BLE_LOG_DEBUG("Ble_manager: Sending pin number..");
//            BLE_LOG_FLUSH();
//
//            pin_digits_count = 0;
//            ble_send_encryption_pin(encryption_pin_number);
//            mitm_activated = false;
//
//            clear_pin_digits_count.reset();
//        }
//        else if (get_flag_security_proc_failed() &&
//                clear_pin_digits_count.do_once())
//        {
//            pin_digits_count = 0;
//        }
//
//        return KBDAPI_EVENT_RESULT_CONSUMED;
//    }
//
//    if ( !ble_is_advertising_mode() && showing_bt_layer && p_key->type == KBDAPI_KEY_TYPE_BLUETOOTH_PAIRING && p_key->toggled_off )
//    {
//        exit_pairing_mode();
//
//        return KBDAPI_EVENT_RESULT_CONSUMED;
//    }
//
//    if ( p_key->type == KBDAPI_KEY_TYPE_BLUETOOTH_PAIRING && p_key->toggled_off )
//    {
//        send_led_mode();
//
//        return KBDAPI_EVENT_RESULT_CONSUMED;
//    }
//
//    result = KBDAPI_EVENT_RESULT_IGNORED;
//
//    if (ble_is_idle())
//    {
//        ble_goto_advertising_mode();
//        LEDBluetoothPairingDefy.setAvertisingModeOn(p_connections_config->current_channel);
//        send_led_mode();
//        LEDManager.leds_enable();
//    }
//
//    if (showing_bt_layer)
//    {
//        if (((p_key->coord.col == 0 && p_key->coord.row == 0) || (p_key->coord.col == 9 && p_key->coord.row == 0)) && p_key->toggled_on)
//        {
//            reset_mcu();
//        }
//
//        /* Erease previous paired channels*/
//        if (((p_key->coord.col == 1 && p_key->coord.row == 1) ||  // Q key
//             (p_key->coord.col == 2 && p_key->coord.row == 1) ||  // W key
//             (p_key->coord.col == 3 && p_key->coord.row == 1) ||  // E key
//             (p_key->coord.col == 4 && p_key->coord.row == 1) ||  // R key
//             (p_key->coord.col == 5 && p_key->coord.row == 1) ||  // T key
//             (p_key->coord.col == 10 && p_key->coord.row == 1) || // Y key
//             (p_key->coord.col == 11 && p_key->coord.row == 1) || // U key
//             (p_key->coord.col == 12 && p_key->coord.row == 1) || // I key
//             (p_key->coord.col == 13 && p_key->coord.row == 1) || // O key
//             (p_key->coord.col == 14 && p_key->coord.row == 1)))  // PT key
//        {
//            set_channel_in_use( p_key );
//            uint8_t index_channel = channel_in_use;
//            if ( p_key->toggled_on )
//            {
//                timer_set_ms( &connectionState[index_channel].pressedTimer, KEY_ERASE_HOLD_TIMEOUT_MS);
//            }
//
//            if ( p_key->is_pressed && timer_check( &connectionState[index_channel].pressedTimer ) )
//            {
//                // TODO: create a led effect to let the user know that the erease was successful
//                erase_paired_device(index_channel);
//
//                result = KBDAPI_EVENT_RESULT_CONSUMED;
//            }
//
//            if ( p_key->toggled_off )
//            {
//                connectionState[index_channel].longPress = false;
//            }
//        }
//
//        /*Change channel in use*/
//        if (((p_key->coord.col == 1 && p_key->coord.row == 0) ||                                                  // 1 key
//             (p_key->coord.col == 2 && p_key->coord.row == 0) ||                                                  // 2 key
//             (p_key->coord.col == 3 && p_key->coord.row == 0) ||                                                  // 3 key
//             (p_key->coord.col == 4 && p_key->coord.row == 0) ||                                                  // 4 key
//             (p_key->coord.col == 5 && p_key->coord.row == 0) || (p_key->coord.col == 10 && p_key->coord.row == 0) || // 6 key
//             (p_key->coord.col == 11 && p_key->coord.row == 0) ||                                                 // 7 key
//             (p_key->coord.col == 12 && p_key->coord.row == 0) ||                                                 // 8 key
//             (p_key->coord.col == 13 && p_key->coord.row == 0) ||                                                 // 9 key
//             (p_key->coord.col == 14 && p_key->coord.row == 0))                                                   // 0 key
//            && p_key->was_pressed)
//        {
//            set_channel_in_use( p_key );
//
//            //BLE_LOG_DEBUG("Ble_manager: channel_in_use: %i", channel_in_use);
//
//            uint8_t index_channel = channel_in_use;
//            // Only if the key has not been press for longer than 2 seconds then change the gapAddress
//            if ( p_key->toggled_off )
//            {
//                // BLE_LOG_DEBUG(" ble_is_advertising_mode(): %i", ble_is_advertising_mode());
//                if (p_connections_config->current_channel != index_channel)
//                {
//                    BLE_LOG_DEBUG("Ble_manager: Changing channel %i to %i", p_connections_config->current_channel, index_channel);
//
//                    cfgmem_current_channel_save( index_channel );
//                    LEDBluetoothPairingDefy.setConnectedChannel(NOT_CONNECTED);
//                    LEDBluetoothPairingDefy.setAvertisingModeOn(p_connections_config->current_channel);
//                    send_led_mode();
//                    update_channel_and_name();
//
//                    // First we disable scanning and advertising.
//                    ble_adv_stop();
//                    delay(200);
//
//                    // Save it in flash memory - In this case, the data need to be saved instantly and not when the standard save timeout expires
//                    ConfigManager.config_save_now();
//
//                    update_current_channel();
//                    delay(200);
//
//                    // Try to change the channel.
//                    ble_disconnect();
//                    delay(200);
//
//                    // Try to reconnect again.
//                    gap_params_init();
//                    delay(200);
//
//                    ble_adv_stop();
//                    advertising_init();
//                    delay(200);
//
//                    /*
//                        If it doesn't have any device paired on the channel, it goes into
//                        advertising with a whitelist so that any device can find it.
//                    */
//                    pm_peer_id_t active_connection_peer_id = p_connections_config->cons[p_connections_config->current_channel].peer_id;
//                    if (active_connection_peer_id == PM_PEER_ID_INVALID)
//                    {
//                        BLE_LOG_INFO("Ble_manager: Whitelist deactivated.");
//
//                        ble_goto_advertising_mode();
//                    }
//                    else
//                    {
//                        BLE_LOG_INFO("Ble_manager: Whitelist activated.");
//
//                        ble_goto_white_list_advertising_mode();
//                    }
//
//                    result = KBDAPI_EVENT_RESULT_CONSUMED;
//                }
//                else if (p_connections_config->current_channel == index_channel && !ble_is_advertising_mode())
//                {
//                    exit_pairing_mode();
//                }
//            }
//
//            BLE_LOG_FLUSH();
//        }
//    }

    return result;
}

kbdapi_event_result_t BleManager::kbdif_key_event_cb( void * p_instance, kbdapi_key_t * p_key )
{
    BleManager * p_BleManager = ( BleManager *)p_instance;

    return p_BleManager->kbdif_key_event_process( p_key );
}

kbdapi_event_result_t BleManager::kbdif_command_event_cb( void * p_instance, const char * p_command )
{
    //    if (::Focus.handleHelp(command, "wireless.bluetooth.devicesMap\nwireless.bluetooth.deviceName")) return EventHandlerResult::OK;
    //
    //    if (strncmp(command, "wireless.bluetooth.", 19) != 0) return EventHandlerResult::OK;
    //    if (strcmp(command + 19, "devicesMap") == 0)
    //    {
    //        if (::Focus.isEOL())
    //        {
    //            for (const auto &connection : ble_flash_data.ble_connections)
    //            {
    //                connection.send();
    //            }
    //        }
    //        else
    //        {
    //            for (auto &connection : ble_flash_data.ble_connections)
    //            {
    //                connection.read();
    //            }
    //
    //            // Save it in flash memory.
    //            Runtime.storage().put(flash_base_addr, ble_flash_data);
    //            Runtime.storage().commit();
    //        }
    //    }
    //
    //    // This command need reset
    //    if (strcmp(command + 19, "deviceName") == 0)
    //    {
    //        if (::Focus.isEOL())
    //        {
    //            BLE_LOG_DEBUG("read request: wireless.bluetooth.deviceName");
    //
    //            for (const auto &device_name_letter : ble_flash_data.device_name_local)
    //            {
    //                ::Focus.send((uint8_t)device_name_letter);
    //            }
    //        }
    //        else
    //        {
    //            BLE_LOG_DEBUG("write request: wireless.bluetooth.deviceName");
    //
    //            for (auto &device_name_letter : ble_flash_data.device_name_local)
    //            {
    //                uint8_t aux;
    //                ::Focus.read(aux);
    //                device_name_letter = (char)aux;
    //            }
    //
    //            // Save it in flash memory.
    //            Runtime.storage().put(flash_base_addr, ble_flash_data);
    //            Runtime.storage().commit();
    //        }
    //    }

    // return EventHandlerResult::EVENT_CONSUMED;
    return KBDAPI_EVENT_RESULT_IGNORED;
}

const kbdif_handlers_t BleManager::kbdif_handlers =
{
    .key_event_cb = kbdif_key_event_cb,
    .command_event_cb = kbdif_command_event_cb,
};

/****************************************************/
/*                   Config Memory                  */
/****************************************************/

void BleManager::cfgmem_ble_name_save( const ble_device_name_t * p_name_config, const ble_device_name_t * p_device_name )
{
    result_t result = RESULT_ERR;
    ble_device_name_t ble_name;

    ASSERT_DYGMA( strlen(p_device_name->name) < (sizeof(ble_name) - 1), "The new BLE name exceeds the available space" );

    /* Fill the name cache. We use the cache to make sure the space in memory has always the same structure - filling the trailing space with 0x00 */
    memset( &ble_name, 0x00, sizeof(ble_name) );
    ble_name = *p_device_name;

    result = ConfigManager.config_item_update( p_name_config, &ble_name, sizeof(ble_device_name_t) );
    ASSERT_DYGMA( result == RESULT_OK, "ConfigManager.config_item_update failed" );

    UNUSED( result );
}

void BleManager::cfgmem_channel_id_save( const channel_t * p_channel, uint8_t id )
{
    result_t result = RESULT_ERR;

    result = ConfigManager.config_item_update( &p_channel->id, &id, sizeof( p_channel->id ) );
    ASSERT_DYGMA( result == RESULT_OK, "ConfigManager.config_item_update failed" );

    UNUSED( result );
}

void BleManager::cfgmem_channel_peer_id_save( const channel_t * p_channel, pm_peer_id_t peer_id )
{
    result_t result = RESULT_ERR;

    result = ConfigManager.config_item_update( &p_channel->peer_id, &peer_id, sizeof( p_channel->peer_id ) );
    ASSERT_DYGMA( result == RESULT_OK, "ConfigManager.config_item_update failed" );

    UNUSED( result );
}

void BleManager::cfgmem_channel_device_address_save( const channel_t * p_channel, const ble_device_addr_t * p_device_addr )
{
    result_t result = RESULT_ERR;

    result = ConfigManager.config_item_update( &p_channel->device_addr, p_device_addr, sizeof( p_channel->device_addr ) );
    ASSERT_DYGMA( result == RESULT_OK, "ConfigManager.config_item_update failed" );

    UNUSED( result );
}

void BleManager::cfgmem_channel_device_name_save( const channel_t * p_channel, const ble_device_name_t * p_device_name )
{
    cfgmem_ble_name_save( &p_channel->device_name, p_device_name );
}

void BleManager::cfgmem_device_name_local_save( const ble_device_name_t * p_device_name_local )
{
    cfgmem_ble_name_save( &p_config->device_name_local, p_device_name_local );
}

void BleManager::cfgmem_current_channel_id_save( uint8_t channel_id )
{
    result_t result = RESULT_ERR;

    BLE_LOG_INFO("*** Ble_manager: Saving current channel %d -> %d", p_config->current_channel_id, channel_id );

    result = ConfigManager.config_item_update( &p_config->current_channel_id, &channel_id, sizeof( p_config->current_channel_id) );
    ASSERT_DYGMA( result == RESULT_OK, "ConfigManager.config_item_update failed" );

    UNUSED( result );
}

void BleManager::cfgmem_force_ble_save( bool_t force_ble )
{
    result_t result = RESULT_ERR;

    result = ConfigManager.config_item_update( &p_config->force_ble, &force_ble, sizeof( p_config->force_ble) );
    ASSERT_DYGMA( result == RESULT_OK, "ConfigManager.config_item_update failed" );

    UNUSED( result );
}

void BleManager::cfgmem_channel_reset( const channel_t * p_channel, uint8_t id )
{
    ble_device_addr_t default_device_addr;
    ble_device_name_t default_device_name;

    memset( &default_device_addr, 0xFF, sizeof(default_device_addr) );
    memset( &default_device_name, 0x00, sizeof(default_device_name) );

    cfgmem_channel_id_save( p_channel, id );
    cfgmem_channel_peer_id_save( p_channel, PM_PEER_ID_INVALID );
    cfgmem_channel_device_address_save( p_channel, &default_device_addr );
    cfgmem_channel_device_name_save( p_channel, &default_device_name );
}

void BleManager::cfgmem_config_reset()
{
    uint8_t i;
    for( i = 0; i < BLE_CHANNELS_COUNT; i++)
    {
        cfgmem_channel_reset( &p_config->channels[i], i );
    }

    cfgmem_device_name_local_save( &ble_device_name_local );
    cfgmem_current_channel_id_save( 0 );
    cfgmem_force_ble_save( false );
}

/****************************************************/
/*                        API                       */
/****************************************************/

result_t BleManager::init()
{
    result_t result = RESULT_ERR;
//    uint8_t i;

    /* First, get the current BLE configuration */
    result = ConfigManager.config_item_request( ConfigManager::CFG_ITEM_TYPE_BLE_CONNECTIONS, (const void **)&p_config );
    EXIT_IF_ERR( result, "ConfigManager.config_item_request failed" );

    // For now lest think that if this variable is invalid, restart everything.
    if( p_config->current_channel_id == 0xFF )
    {
        cfgmem_config_reset();
    }

    /* Initialize the keyboard interface */
    result = kbdif_initialize();
    EXIT_IF_ERR( result, "kbdif_initialize failed" );

    result = kbdapi_key_report_lock_init( &kbdapi_key_report_lock );
    EXIT_IF_ERR( result, "kbdapi_key_report_lock_init failed" );

    BLE_LOG_DEBUG("Ble_manager: Current channel %i", p_config->current_channel_id);

    /* Initialize the Low Level BLE */
    ble_ll_init();
//    update_channel_and_name();

    /* Initialize the flags */
    enable_request_flag = false;
    enabled_flag = false;

    /* Set the initial state */
    state = BLEM_STATE_DISABLED;

//    // UX STUFFf
//    i = 0;
//    for (auto &item : p_connections_config->cons) // get all the paired devices and store in a variable.
//    {
//        if (item.peer_id == PM_PEER_ID_INVALID)
//        {
//            set_paired_channel_led(i, false); // set channel related bit to 0
//
//            BLE_LOG_DEBUG("Ble_manager: Channel %i -> 0", i);
//        }
//        else
//        {
//            set_paired_channel_led(i, true); // set channel related bit to 1
//
//            BLE_LOG_DEBUG("Ble_manager: Channel %i -> 1", i);
//        }
//
//        i++;
//    }
//
//    BLE_LOG_DEBUG("Ble_manager: %i channels in total.", i);
//
//    LEDBluetoothPairingDefy.setConnectedChannel(NOT_CONNECTED);
//    LEDBluetoothPairingDefy.setEreaseDone(false);

    BLE_LOG_FLUSH();

_EXIT:
    return result;
}

//result_t BleManager::init()
//{
//    result_t result = RESULT_ERR;
//    uint8_t i;
//
//    result = kbdif_initialize();
//    EXIT_IF_ERR( result, "kbdif_initialize failed" );
//
//    result = kbdapi_key_report_lock_init( &kbdapi_key_report_lock );
//    EXIT_IF_ERR( result, "kbdapi_key_report_lock_init failed" );
//
//    result = ConfigManager.config_item_request( ConfigManager::CFG_ITEM_TYPE_BLE_CONNECTIONS, (const void **)&p_connections_config );
//    EXIT_IF_ERR( result, "ConfigManager.config_item_request failed" );
//
//    // For now lest think that if this variable is invalid, restart everything.
//    if( p_connections_config->current_channel == 0xFF )
//    {
//        cfgmem_connections_config_reset();
//    }
//
//    BLE_LOG_DEBUG("Ble_manager: Current channel %i", p_connections_config->current_channel);
//
//    update_channel_and_name();
//    // UX STUFFf
//    i = 0;
//    for (auto &item : p_connections_config->cons) // get all the paired devices and store in a variable.
//    {
//        if (item.peer_id == PM_PEER_ID_INVALID)
//        {
//            set_paired_channel_led(i, false); // set channel related bit to 0
//
//            BLE_LOG_DEBUG("Ble_manager: Channel %i -> 0", i);
//        }
//        else
//        {
//            set_paired_channel_led(i, true); // set channel related bit to 1
//
//            BLE_LOG_DEBUG("Ble_manager: Channel %i -> 1", i);
//        }
//
//        i++;
//    }
//
//    BLE_LOG_DEBUG("Ble_manager: %i channels in total.", i);
//
//    LEDBluetoothPairingDefy.setConnectedChannel(NOT_CONNECTED);
//    LEDBluetoothPairingDefy.setEreaseDone(false);
//
//    BLE_LOG_FLUSH();
//
//_EXIT:
//    return result;
//}

result_t BleManager::enable()
{
    if( state != BLEM_STATE_DISABLED )
    {
        ASSERT_DYGMA( false, "Invalid attempt to enable the BLE" );
        return RESULT_ERR;
    }

    enable_request_flag = true;

    return RESULT_OK;
}

bool_t BleManager::is_enabled( void )
{
    return ( enabled_flag == true ) ? true : false;
}

bool_t BleManager::is_connected( void )
{
#warning "BleManager::is_connected not implemented"
    return false;
}

void BleManager::battery_level_update( uint8_t battery_level )
{
#warning "BleManager::battery_level_update not implemented"
}

void BleManager::force_ble_set( bool enabled )
{
    cfgmem_force_ble_save( enabled );
}

bool BleManager::force_ble_get( void )
{
    return p_config->force_ble;
}

void BleManager::run()
{
    state_machine();
}

//void BleManager::run()
//{
//    if (!ble_innited() || !FirmwareVersion::keyboard_is_wireless())
//    {
//        return;
//    }
//
//    timer_save_conn_run(2000);  // 2000ms timer.
//    timer_save_name_run(2000);  // 2000ms timer.
//
//    if (get_flag_security_proc_started())
//    {
//        mitm_activated = true;
//        exit_pairing_mode(); // If it is in the "enter pin number" state, set default layer.
//        clear_flag_security_proc_started();
//    }
//    else if (get_flag_security_proc_failed())
//    {
//        mitm_activated = false;
//        send_led_mode(); // If enters pin number fails, returns to advertising led mode layer.
//        clear_flag_security_proc_failed();
//    }
//
//    if (mitm_activated && ble_connected())
//    {
//        mitm_activated = false;
//    }
//
//    // Gives time to the EPPROM to update
//    static bool activated_advertising = false;
//
//    if (ble_is_advertising_mode())
//    {
//        // Just activate once.
//        if (!activated_advertising )
//        {
//            // set_pairing_led_effect();
//
//            BLE_LOG_DEBUG("Ble_manager: Channel %i, advertising mode.", p_connections_config->current_channel);
//            BLE_LOG_FLUSH();
//
//            LEDBluetoothPairingDefy.setAvertisingModeOn(p_connections_config->current_channel);
//            send_led_mode();
//            activated_advertising = true;
//        }
//    }
//    else if( ble_connected() )
//    {
//        if ( activated_advertising )
//        {
//            LEDBluetoothPairingDefy.setConnectedChannel(p_connections_config->current_channel);
//            LEDBluetoothPairingDefy.setAvertisingModeOn(NOT_ON_ADVERTISING);
//            set_paired_channel_led(p_connections_config->current_channel, true);
//            send_led_mode();
//            ble_get_device_name(device_name_evt_handler);  // Asynchronous call to the softdevice.
//            activated_advertising = false;
//            exit_pairing_mode();
//
//            trigger_save_conn_timer = true;
//        }
//    }
//    else if (activated_advertising && ble_is_idle())
//    {
//        activated_advertising = false;
//        LEDManager.leds_disable();
//        Communications_protocol::Packet p{};
//        p.header.command = Communications_protocol::SLEEP;
//        Communications.sendPacket(p);
//    }
//
//    // Reconection of side
//    if (!activated_advertising && ble_is_idle() && LEDManager.leds_enabled())
//    {
//        ble_goto_advertising_mode();
//        LEDBluetoothPairingDefy.setAvertisingModeOn(p_connections_config->current_channel);
//        send_led_mode();
//    }
//}

class BleManager BleManager;

//void device_name_evt_handler(void)
//{
//    BLE_LOG_INFO("Ble_manager: Event device name %s", get_connected_device_name_ptr());
//
//    BleManager.trigger_save_name_timer = true;
//}

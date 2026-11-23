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

#include "Ble_config.h"
#include "Ble_composite_dev.h"
#include "Ble_hmi.h"
#include "Ble_manager.h"
//#include "keyboard_api.h"
#include "FirmwareVersion.h"

#include "kbd_if_manager.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#define BLE_MANAGER_DEBUG_LOG   0
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

#define BATTERY_LEVEL_INVALID       0xFF      /* The default/initial battery level is set to an invalid value */

/********************************************************************/
/*                 Low Level BLE - Composite device                 */
/********************************************************************/

result_t BleManager::ble_ll_init( void )
{
    result_t result = RESULT_ERR;
    blecdev_conf_t config;

    config.device_name_local = *BleConfig.cfg_ble_device_name_local_get();
    config.p_instance = this;
    config.event_cb = ble_ll_event_cb;

    result = blecdev_init( &config );
    EXIT_IF_ERR( result, "blecdev_init failed" );

_EXIT:
    return result;
}

inline void BleManager::ble_ll_event_type_advertising_process( void )
{
    uint8_t channels_bond_mask = BleConfig.cfg_channels_bond_mask_get();

    /* Set the advertising state */
    state_set( BLEM_STATE_ADVERTISING );

    /* Activate the HMI advertising mode */
    BleHmi.hmi_advertise( p_channel_current->channel_id, channels_bond_mask );
}

inline void BleManager::ble_ll_event_type_advertising_failed_process( void )
{
    /* Set the advertising fail state */
    state_set( BLEM_STATE_ADVERTISING_FAIL );
}

inline void BleManager::ble_ll_event_type_sec_bond_code_req_process( void )
{
    ASSERT_DYGMA( p_channel_current->peer_id == PM_PEER_ID_INVALID, "Security pairing process unexpectedly for the already paired device" );

    /* Set the pairing state */
    state_set( BLEM_STATE_PAIRING );

    /* Activate the HMI bond code read mode */
    BleHmi.hmi_read_bond_code();
}

inline void BleManager::ble_ll_event_type_sec_bond_success_process( blecdev_evt_sec_bond_success_param_t * p_sec_bond_success_param )
{
    result_t result = RESULT_ERR;
    BleConfig::blecfg_channel_t blecfg_channel;

    ASSERT_DYGMA( p_channel_current->peer_id == PM_PEER_ID_INVALID, "Security pairing process unexpectedly for the already paired device" );

    /* Prepare the Channel bonding data */
    blecfg_channel = *p_channel_current;

    blecfg_channel.peer_id = p_sec_bond_success_param->peer_id;
    blecfg_channel.device_addr = p_sec_bond_success_param->peer_addr;

    /* Request to make the Config Channel Bond */
    result = BleConfig.cfg_channel_bond_save( &blecfg_channel );

    if( result != RESULT_OK )
    {
        ASSERT_DYGMA( false, "BleConfig.cfg_channel_bond_req failed" );

        /*
         * The code currently does not handle the result ERR or BUSY. Hence, in such a case, we restart the bluetooth connection process
         */

        result = restart( );
        ASSERT_DYGMA( result == RESULT_OK, "BLE restart failed" );
    }

    /*
     * Now we wait until the BLE Confing event is received
     */
}

inline void BleManager::ble_ll_event_type_sec_bond_failed_process( blecdev_evt_sec_bond_failed_param_t * p_sec_bond_failed_param )
{
    result_t result = RESULT_ERR;

    ASSERT_DYGMA( p_sec_bond_failed_param->peer_id == p_channel_current->peer_id, "BLE security failed to an unexpected peer" );

    /* Reset the BLE connection process */
    result = restart( );
    ASSERT_DYGMA( result == RESULT_OK, "BLE restart failed" );

    UNUSED( result );
}

inline void BleManager::ble_ll_event_type_peer_connected_process( blecdev_evt_peer_connected_param_t * p_peer_connected_param )
{
    if( state == BLEM_STATE_PAIRING )
    {
        /* Ignore the peer connected event when in the pairing state - waiting for the bond to be finished first */
        return;
    }

    ASSERT_DYGMA( p_peer_connected_param->peer_id == p_channel_current->peer_id, "BLE connected to an unexpected peer" );

    /* The connection is paired - deactivate */
    BleHmi.hmi_update_connected_channel( p_channel_current->channel_id );
    BleHmi.hmi_deactivate();

    /* Set the connected state */
    state_connected_set( );
}

inline void BleManager::ble_ll_event_type_peer_device_name_process( blecdev_evt_peer_device_name_param_t * p_peer_device_name_param )
{
    ASSERT_DYGMA( p_channel_current->peer_id != PM_PEER_ID_INVALID, "Security pairing process unexpectedly for the already paired device" );

    /*
     * We currently do not process this event
     */
}

void BleManager::ble_ll_event_process( blecdev_event_type_t event_type, blecdev_evt_param_t * p_param )
{
    result_t result = RESULT_ERR;

    /* Check the submodule-related events */
    result = BleConfig.cfg_blecdev_event_inject( event_type, p_param );
    if( result != RESULT_INCOMPLETE )
    {
        /* The event has been consumed */
        return;
    }

    switch( event_type )
    {
        case BLECDEV_EVENT_TYPE_ADVERTISING:

            ble_ll_event_type_advertising_process();

            break;

        case BLECDEV_EVENT_TYPE_ADVERTISING_FAILED:

            ble_ll_event_type_advertising_failed_process();

            break;

        case BLECDEV_EVENT_TYPE_SEC_BOND_CODE_REQ:

            ble_ll_event_type_sec_bond_code_req_process();

            break;

        case BLECDEV_EVENT_TYPE_SEC_BOND_SUCCESS:

            ble_ll_event_type_sec_bond_success_process( &p_param->sec_bond_success );

            break;

        case BLECDEV_EVENT_TYPE_SEC_BOND_FAILED:

            ble_ll_event_type_sec_bond_failed_process( &p_param->sec_bond_failed );

            break;

        case BLECDEV_EVENT_TYPE_PEER_CONNECTED:

            ble_ll_event_type_peer_connected_process( &p_param->peer_connected );

            break;

        case BLECDEV_EVENT_TYPE_PEER_DEVICE_NAME:

            ble_ll_event_type_peer_device_name_process( &p_param->peer_device_name );

            break;

        default:

            ASSERT_DYGMA( false, "Unhandled BLE Composite Device event" )

            break;
    }
}

void BleManager::ble_ll_event_cb( void * p_instance, blecdev_event_type_t event_type, blecdev_evt_param_t * p_param )
{
    BleManager * p_bleManager = (BleManager *)p_instance;
    p_bleManager->ble_ll_event_process( event_type, p_param );
}

/********************************************************************/
/*                           BLE Manager                            */
/********************************************************************/

result_t BleManager::blem_disable( void )
{
    result_t result = RESULT_ERR;

    /* Wait until softdevice-dependent ConfigManager processes are finished */
    if( BleConfig.cfg_is_busy() == true )
    {
        return RESULT_BUSY;
    }

    result = blecdev_disable();
    ASSERT_DYGMA( result == RESULT_OK || result == RESULT_BUSY, "blecdev_disable failed" );
    EXIT_IF_NOK( result );

    /* Disable the BLE config module */
    BleConfig.cfg_disable();

    state_set( BLEM_STATE_DISABLED );

_EXIT:
    return result;
}

/****************************************************/
/*                   State machine                  */
/****************************************************/

inline void BleManager::state_set( blem_state_t blem_state )
{
    this->state = blem_state;
    mcu_sleep_postpone();
}

inline void BleManager::state_connected_set( void )
{
    result_t result = RESULT_ERR;

    /* Update the battery level */
    bat_level_update();

    state_set( BLEM_STATE_CONNECTED );

    UNUSED( result );
}

inline void BleManager::state_disabled_process( void )
{
    if( FirmwareVersion::keyboard_is_wireless() == false )
    {
        /* The keyboard setup is not wireless */
        return;
    }
    else if( enable_request_flag == true )
    {
        enable_request_flag = false;
        state_set( BLEM_STATE_ENABLE );
        return;
    }
}

inline void BleManager::state_enable_process( void )
{
    result_t result = RESULT_ERR;
    blecdev_enable_conf_t enable_config;

    /* Wait until softdevice-dependent ConfigManager processes are finished */
    if( BleConfig.cfg_is_busy() == true )
    {
        return;
    }

    /* Enable the HMI machine */
    BleConfig.cfg_enable();

    /* Get the current channel in use */
    p_channel_current = BleConfig.cfg_current_channel_get();

    /* Enable the HMI machine */
    BleHmi.hmi_enable();

    /* Enable the ble composite device */
    enable_config.current_channel_id = p_channel_current->channel_id;

    result = blecdev_enable( &enable_config );
    ASSERT_DYGMA( result == RESULT_OK, "blecdev_enable failed" );
    EXIT_IF_ERR( result, "blecdev_enable failed" );

    /* Check if the current channel peer ID is not bonded. */
    if( p_channel_current->peer_id == PM_PEER_ID_INVALID )
    {
        /* The peer is not bonded on this channel, hence we start general advertising over all possible connections. */
        result = blecdev_adv_start();
        ASSERT_DYGMA( result == RESULT_OK, "blecdev_adv_start failed" );
        EXIT_IF_ERR( result, "blecdev_adv_start failed" );
    }
    else
    {
        /* The peer is bonded on this channel, hence we start connection to the peer. */
        result = blecdev_peer_conn_start( p_channel_current->peer_id );
        ASSERT_DYGMA( result == RESULT_OK, "blecdev_adv_start_whitelist failed" );
        EXIT_IF_ERR( result, "blecdev_adv_start_whitelist failed" );
    }


    enabled_flag = true;
    state_set( BLEM_STATE_ENABLED );

_EXIT:
    return;
}

inline void BleManager::state_disable_process( void )
{
    result_t result = RESULT_ERR;

    /* Disable the BLE Manager */
    result = blem_disable();
    ASSERT_DYGMA( result != RESULT_ERR, "blem_disable failed" );
    EXIT_IF_NOK( result );

    /* De-assert the enabled flag */
    enabled_flag = false;

    /* Disable the HMI machine */
    BleHmi.hmi_disable();

_EXIT:
    return;
}

inline void BleManager::state_restart_process( void )
{
    result_t result = RESULT_ERR;

    /* Disable the BLE Manager */
    result = blem_disable();
    ASSERT_DYGMA( result != RESULT_ERR, "blem_disable failed" );
    EXIT_IF_NOK( result );

    /* Re-enable the BLE again to, effectively, perform the restart */
    result = enable();
    ASSERT_DYGMA( result == RESULT_OK, "The BLE enable should always work at this point" );

    /*
     * IMPORTANT:
     * Keeping the HMI enabled as well as the enabled_flag asserted to prevent flicker upon the restart
     */

_EXIT:
    return;
}

inline void BleManager::state_advertising_fail_process( void )
{
    result_t result = RESULT_ERR;

    /* Disable the BLE Manager */
    result = blem_disable();
    ASSERT_DYGMA( result != RESULT_ERR, "blem_disable failed" );
    EXIT_IF_NOK( result );

    /* De-assert the enabled flag */
    enabled_flag = false;

    /* Disable the HMI machine */
    BleHmi.hmi_disable();

    /*
     * NOTE: Entering the true sleep should be done through some system-level Power module. Such module does not exist at this point.
     */

    /* Enter the true sleep */
    LEDManager.true_sleep_enter();

_EXIT:
    return;
}

inline void BleManager::state_escape_process( void )
{
    /* Wait until softdevice-dependent ConfigManager processes are finished */
    if( BleConfig.cfg_is_busy() == true )
    {
        return;
    }

    /* Restart the device */
    reset_mcu();
}

inline void BleManager::state_machine( void )
{
    switch( state )
    {
        case BLEM_STATE_DISABLED:

            state_disabled_process();

            break;

        case BLEM_STATE_ENABLE:

            state_enable_process();

            break;

        case BLEM_STATE_ENABLED:

            /* Waiting for blecdev / hmi events */

            break;

        case BLEM_STATE_ADVERTISING:

            /* Waiting for blecdev / hmi events */

            break;

        case BLEM_STATE_PAIRING:

            /* Waiting for blecdev / hmi events */

            break;

        case BLEM_STATE_CONNECTED:

            /* Waiting for blecdev / hmi events */

            break;

        case BLEM_STATE_DISABLE:

            state_disable_process();

            break;

        case BLEM_STATE_RESTART:

            state_restart_process();

            break;

        case BLEM_STATE_ADVERTISING_FAIL:

            state_advertising_fail_process();

            break;

        case BLEM_STATE_ESCAPE:

            state_escape_process();

            break;

        default:

            ASSERT_DYGMA( false, "Unhandled BLE Manager state" );

            break;
    }
}

/********************************************************************/
/*                            BLE Config                            */
/********************************************************************/

inline result_t BleManager::blecfg_init( void )
{
    result_t result = RESULT_ERR;
    BleConfig::blecfg_config_t config;

    config.p_instance = this;
    config.event_cb = blecfg_event_cb;

    result = BleConfig.cfg_init( &config );
    EXIT_IF_ERR( result, "blecfg_init failed" );

_EXIT:
    return result;
}

inline void BleManager::blecfg_event_type_channel_bond_save_success_process( BleConfig::blecfg_evt_channel_bond_save_success_param_t * p_channel_bond_save_success_param )
{
    uint8_t channels_bond_mask;

    /* The channel bond save process has finished successfully, signing the connection is paired */

    /* Update and deactivate HMI */
    channels_bond_mask = BleConfig.cfg_channels_bond_mask_get();

    BleHmi.hmi_update_bonded_channels( channels_bond_mask );
    BleHmi.hmi_update_connected_channel( p_channel_current->channel_id );
    BleHmi.hmi_deactivate();

    /* Set the connected state */
    state_connected_set( );
}

inline void BleManager::blecfg_event_type_channel_bond_save_failed_process( BleConfig::blecfg_evt_channel_bond_save_failed_param_t * p_channel_bond_save_failed_param )
{
    result_t result = RESULT_ERR;

    /* The channel bond save signing an unexpected issue. Restart the BLE connection process */

    /* Reset the BLE connection process */
    result = restart( );
    ASSERT_DYGMA( result == RESULT_OK, "BLE restart failed" );

    UNUSED( result );
}

inline void BleManager::blecfg_event_type_channel_erase_success_process( BleConfig::blecfg_evt_channel_erase_success_param_t * p_channel_erase_success_param )
{
    result_t result = RESULT_ERR;
    uint8_t channels_bond_mask;

    /* Check if we have erased the currently used channel */
    if( p_channel_current != NULL && p_channel_erase_success_param->channel_id == p_channel_current->channel_id )
    {
        /* The channel is not valid any more. Reset the BLE connection process. */
        result = restart( );
        ASSERT_DYGMA( result == RESULT_OK, "BLE restart failed" );
    }
    else
    {
        /* Another channel has been erased -> Update the HMI module */

        channels_bond_mask = BleConfig.cfg_channels_bond_mask_get();

        BleHmi.hmi_update_bonded_channels( channels_bond_mask );
    }

    UNUSED( result );
}

inline void BleManager::blecfg_event_type_channel_erase_failed_process( BleConfig::blecfg_evt_channel_erase_failed_param_t * p_channel_erase_failed_param )
{
    result_t result = RESULT_ERR;

    /* The erase process failed signing an unexpected issue. Restart the BLE connection process */

    /* Reset the BLE connection process */
    result = restart( );
    ASSERT_DYGMA( result == RESULT_OK, "BLE restart failed" );

    UNUSED( result );
}

inline void BleManager::blecfg_event_process( BleConfig::blecfg_event_type_t event_type, BleConfig::blecfg_evt_param_t * p_param )
{
    switch( event_type )
    {
        case BleConfig::BLECFG_EVENT_TYPE_CHANNEL_BOND_SAVE_SUCCESS:

            blecfg_event_type_channel_bond_save_success_process( &p_param->channel_bond_save_success );

            break;

        case BleConfig::BLECFG_EVENT_TYPE_CHANNEL_BOND_SAVE_FAILED:

            blecfg_event_type_channel_bond_save_failed_process( &p_param->channel_bond_save_failed );

            break;

        case BleConfig::BLECFG_EVENT_TYPE_CHANNEL_ERASE_SUCCESS:

            blecfg_event_type_channel_erase_success_process( &p_param->channel_erase_success );

            break;

        case BleConfig::BLECFG_EVENT_TYPE_CHANNEL_ERASE_FAILED:

            blecfg_event_type_channel_erase_failed_process( &p_param->channel_erase_failed );

            break;

        default:

            ASSERT_DYGMA( false, "Unhandled BLE Config event" );

            break;
    }
}

void BleManager::blecfg_event_cb( void * p_instance, BleConfig::blecfg_event_type_t event_type, BleConfig::blecfg_evt_param_t * p_param  )
{
    BleManager * p_bleManager = (BleManager *)p_instance;
    p_bleManager->blecfg_event_process( event_type, p_param );
}

/********************************************************************/
/*                             BLE HMI                              */
/********************************************************************/

inline result_t BleManager::blehmi_init( void )
{
    result_t result = RESULT_ERR;
    BleHmi::blehmi_config_t config;

    config.p_instance = this;
    config.event_cb = blehmi_event_cb;

    result = BleHmi.hmi_init( &config );
    EXIT_IF_ERR( result, "blehmi_init failed" );

_EXIT:
    return result;
}

inline void BleManager::blehmi_event_type_channel_change_process( BleHmi::blehmi_evt_channel_change_param_t * p_channel_change_param )
{
    result_t result = RESULT_ERR;

    if( p_channel_change_param->channel_id == p_channel_current->channel_id )
    {
        /* If the the BLE is connected via this channel, then deactivate the HMI and continue with normal BLE operation */
        if( is_connected() == true )
        {
            BleHmi.hmi_deactivate();
        }

        /* Leave the function as the channel is not being changed */
        return;
    }

    /* Save the new channel id */
    BleConfig.cfg_current_channel_set( p_channel_change_param->channel_id );

    /* Reset the BLE connection process */
    result = restart( );
    ASSERT_DYGMA( result == RESULT_OK, "BLE restart failed" );

    UNUSED( result );
}

inline void BleManager::blehmi_event_type_channel_erase_process( BleHmi::blehmi_evt_channel_erase_param_t * p_channel_erase_param )
{
    result_t result = RESULT_ERR;

    result = BleConfig.cfg_channel_id_erase( p_channel_erase_param->channel_id );
    ASSERT_DYGMA( result == RESULT_OK, "cfg_channel_id_erase failed" );

    UNUSED( result );
}

inline void BleManager::blehmi_event_type_bond_code_ready_process( BleHmi::blehmi_evt_bond_code_ready_param_t * p_bond_code_ready_param )
{
    result_t result = RESULT_ERR;

    /* The bond code is ready - provide it to the ble composite device */
    result = blecdev_sec_bond_code_send( &p_bond_code_ready_param->bond_code );
    ASSERT_DYGMA( result == RESULT_OK, "blecdev_sec_bond_code_send failed" );

    UNUSED( result );
}

inline void BleManager::blehmi_event_type_deactivate_request_process( void )
{
    /* If the the BLE is connected, then deactivate the HMI and continue with normal BLE operation. Otherwise ignore the request */
    if( is_connected() == true )
    {
        BleHmi.hmi_deactivate();
    }
}

inline void BleManager::blehmi_event_type_escape_request_process( void )
{
    /*
     * When the escape HMI event is received, we go straight to the escape state which results in the system restart
     */
    state_set( BLEM_STATE_ESCAPE );
}

inline void BleManager::blehmi_event_process( BleHmi::blehmi_event_type_t event_type, BleHmi::blehmi_evt_param_t * p_param )
{
    switch( event_type )
    {
        case BleHmi::BLEHMI_EVENT_TYPE_CHANNEL_CHANGE:

            blehmi_event_type_channel_change_process( &p_param->channel_change );

            break;

        case BleHmi::BLEHMI_EVENT_TYPE_CHANNEL_ERASE:

            blehmi_event_type_channel_erase_process( &p_param->channel_erase );

            break;

        case BleHmi::BLEHMI_EVENT_TYPE_BOND_CODE_READY:

            blehmi_event_type_bond_code_ready_process( &p_param->bond_code_ready );

            break;

        case BleHmi::BLEHMI_EVENT_TYPE_DEACTIVATE_REQ:

            blehmi_event_type_deactivate_request_process( );

            break;

        case BleHmi::BLEHMI_EVENT_TYPE_ESCAPE_REQ:

            blehmi_event_type_escape_request_process( );

            break;

        default:

            ASSERT_DYGMA( false, "Unhandled BLE HMI event" );

            break;
    }
}

void BleManager::blehmi_event_cb( void * p_instance, BleHmi::blehmi_event_type_t event_type, BleHmi::blehmi_evt_param_t * p_param )
{
    BleManager * p_bleManager = (BleManager *)p_instance;
    p_bleManager->blehmi_event_process( event_type, p_param );
}

/****************************************************/
/*                Battery reporting                 */
/****************************************************/

void BleManager::bat_level_update( void )
{
    result_t result = RESULT_ERR;

    /* Check if the BLE peer is connected and the battery level has been set */
    if( is_connected() == false || battery_level == BATTERY_LEVEL_INVALID )
    {
        return;
    }

    /* Update the battery level */
    result = blecdev_battery_level_set( battery_level );
    ASSERT_DYGMA( result != RESULT_ERR, "blecdev_battery_level_set failed" );

    UNUSED( result );
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
    .key_event_cb = NULL,
    .command_event_cb = kbdif_command_event_cb,
};

/****************************************************/
/*                        API                       */
/****************************************************/

result_t BleManager::init()
{
    result_t result = RESULT_ERR;

    /* Initialize the BLE config interface */
    result = blecfg_init();
    EXIT_IF_ERR( result, "blecfg_init failed" );

    /* Initialize the keyboard interface */
    result = kbdif_initialize();
    EXIT_IF_ERR( result, "kbdif_initialize failed" );

    /* Initialize the Low Level BLE */
    result = ble_ll_init();
    EXIT_IF_ERR( result, "ble_ll_init failed" );

    /* Initialize the BLE HMI interface */
    result = blehmi_init();
    EXIT_IF_ERR( result, "blehmi_init failed" );

    /* Battery value */
    battery_level = BATTERY_LEVEL_INVALID;

    /* Initialize the flags */
    enable_request_flag = false;
    enabled_flag = false;

    /* Set the initial state */
    state = BLEM_STATE_DISABLED;

    BLE_LOG_FLUSH();

_EXIT:
    return result;
}

result_t BleManager::enable( void )
{
    if( state != BLEM_STATE_DISABLED )
    {
        ASSERT_DYGMA( false, "Invalid attempt to enable the BLE" );
        return RESULT_ERR;
    }

    enable_request_flag = true;

    return RESULT_OK;
}

result_t BleManager::disable( void )
{
    if( state == BLEM_STATE_DISABLED )
    {
        /* The module is already disabled */
        return RESULT_OK;
    }

    /* Move directly to the disable state */
    state_set( BLEM_STATE_DISABLE );

    return RESULT_OK;
}

result_t BleManager::restart( void )
{
    if( state == BLEM_STATE_DISABLED )
    {
        ASSERT_DYGMA( false, "Invalid attempt to restart disabled the BLE" );
        return RESULT_ERR;
    }

    /* Move to the restart state */
    state_set( BLEM_STATE_RESTART );

    return RESULT_OK;
}

bool_t BleManager::is_enabled( void )
{
    return enabled_flag;
}

bool_t BleManager::is_connected( void )
{
    return ( state == BLEM_STATE_CONNECTED ) ? true : false;
}

void BleManager::battery_level_update( uint8_t bat_level )
{
    /* Update the battery level */
    battery_level = bat_level;

    /* Trigger the BLE battery level update */
    bat_level_update( );
}

void BleManager::force_ble_set( bool enabled )
{
    BleConfig.cfg_force_ble_set( enabled );
}

bool BleManager::force_ble_get( void )
{
    return BleConfig.cfg_force_ble_get();
}

void BleManager::run()
{
    blecdev_run();

    state_machine();

    BleConfig.cfg_run();
    BleHmi.hmi_run();
}

class BleManager BleManager;

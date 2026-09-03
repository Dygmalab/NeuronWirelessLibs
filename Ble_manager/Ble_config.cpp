/*
 * Ble_config -- Manage Bluetooth low energy configuration
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

#include "Ble_config.h"

/********************************************************************/
/*                 Low Level BLE - Composite device                 */
/********************************************************************/

inline result_t BleConfig::ble_ll_event_type_peer_erased( void )
{
    ASSERT_DYGMA( cfg_state == BLECFG_STATE_PEER_ERASE_WAIT, "Unexpected BLE peer erase detected (success)" );

    /* Refresh the list of the peers */
    cfg_state_load_peers_set();

    return RESULT_OK;   /* The event has been consumed */
}

inline result_t BleConfig::ble_ll_event_type_peer_erase_failed( void )
{
    ASSERT_DYGMA( cfg_state == BLECFG_STATE_PEER_ERASE_WAIT, "Unexpected BLE peer erase detected (failure)" );
    ASSERT_DYGMA( false, "BLE peer erase not expected to fail." );

//    /* Refresh the list of the peers, it will restart t */
//    cfg_state_peer_erase_set( cfg_peer_erase_id );

    return RESULT_OK;   /* The event has been consumed */
}

inline result_t BleConfig::ble_ll_event_process( blecdev_event_type_t event_type, blecdev_evt_param_t * p_param )
{
    result_t result = RESULT_INCOMPLETE;

    switch( event_type )
    {
        case BLECDEV_EVENT_TYPE_PEER_ERASED:

            result = ble_ll_event_type_peer_erased();

            break;

        case BLECDEV_EVENT_TYPE_PEER_ERASE_FAILED:

            result = ble_ll_event_type_peer_erase_failed();

            break;

        default:

            result = RESULT_INCOMPLETE;

            break;
    }

    return result;
}

/****************************************************/
/*                      Channels                    */
/****************************************************/

inline result_t BleConfig::cfg_channel_load( pm_peer_id_t peer_id )
{
    result_t result = RESULT_ERR;

    blecfg_peer_app_data_t peer_app_data;
    uint32_t peer_app_data_len;
    blecfg_channel_t * p_channel;

    /* Get the peer application data */
    peer_app_data_len =  sizeof( peer_app_data ); /* Initializing the size of the peer app data space */
    result = blecdev_peer_app_data_get( peer_id, &peer_app_data, &peer_app_data_len );
    EXIT_IF_ERR( result, "blecdev_peer_app_data_get failed" );

    /* Check the loaded peer APP data */
    if( peer_app_data.magic_number != BLECFG_PEER_APP_DATA_MAGIC_NUMBER || peer_app_data.version != BLECFG_PEER_APP_DATA_VERSION )
    {
        return RESULT_ERR;
    }

    /* Load the cfg channel */
    p_channel = &cfg_channels[ peer_app_data.channel_id ];

    p_channel->loaded = true;
    p_channel->channel_id = peer_app_data.channel_id;           /* The ID of the channel in the list of the channels */
    p_channel->peer_id = peer_id;

_EXIT:
    return result;
}

//inline result_t BleConfig::cfg_channel_erase( pm_peer_id_t peer_id )
//{
//    result_t result = RESULT_ERR;
//
//    /* Save the peer if to be erased */
//    cfg_peer_erase_id = peer_id;
//
//
//_EXIT:
//    return result;
//}

/****************************************************/
/*                   State machine                  */
/****************************************************/

inline void BleConfig::cfg_state_set( blecfg_state_t blecfg_state )
{
    this->cfg_state = blecfg_state;
    mcu_sleep_postpone();
}

inline void BleConfig::cfg_state_load_peers_set( void )
{
    /* Clear the channels first */
    memset( cfg_channels, 0x00, sizeof( cfg_channels ) );

    /* Set the Load peers state */
    cfg_state_set( BLECFG_STATE_LOAD_PEERS );
}

inline void BleConfig::cfg_state_peer_erase_set( pm_peer_id_t peer_id )
{
    cfg_peer_erase_id = peer_id;

    /* Set the Load peers state */
    cfg_state_set( BLECFG_STATE_PEER_ERASE );
}

inline void BleConfig::cfg_state_load_peers_process( void )
{
    result_t result = RESULT_ERR;

    pm_peer_id_t peer_list[ BLECFG_CHANNELS_COUNT ];
    uint32_t peer_cnt;
    uint32_t peer_id;

    /* Get the list of peers */
    peer_cnt = BLECFG_CHANNELS_COUNT;          /* Initializing the size of the peer list */
    result = blecdev_peer_list_get( peer_list, &peer_cnt );
    ASSERT_DYGMA( result == RESULT_OK, "blecdev_peer_list_get failed" );

    for( peer_id = 0; peer_id < peer_cnt; peer_id++ )
    {
        result = cfg_channel_load( peer_id );
        if( result == RESULT_ERR )
        {
            /* The channel could not be loaded from this peer. It does not contain APP data. This can happen when the peer has been saved in the peer manager
             * but we could not add our app data to that peer. E.g. due to a system failure during the process. We need to clear the peer from the peer memory */
            cfg_state_peer_erase_set( peer_id );
            return;
        }
    }

    /* The channels are loaded, move to the enabled state */
    cfg_state_set( BLECFG_STATE_ENABLED );
}

inline void BleConfig::cfg_state_peer_erase_process( void )
{
    result_t result = RESULT_ERR;

    result = blecdev_peer_erase( cfg_peer_erase_id );
    ASSERT_DYGMA( result == RESULT_OK, "blecdev_peer_erase failed" );
    EXIT_IF_NOK( result );

    /* Wait for the peer deletion finish */
    cfg_state_set( BLECFG_STATE_PEER_ERASE_WAIT );

_EXIT:
    return;

    UNUSED( result );
}

inline void BleConfig::cfg_state_machine( void )
{
    switch( cfg_state )
    {
        case BLECFG_STATE_DISABLED:

            /* Waiting for the CFG enable function call */

            break;

        case BLECFG_STATE_LOAD_PEERS:

            cfg_state_load_peers_process();

            break;

        case BLECFG_STATE_PEER_ERASE:

            cfg_state_peer_erase_process();

            break;

        case BLECFG_STATE_PEER_ERASE_WAIT:

            /* Waiting for the peer erase finish  */

            break;

        default:

            ASSERT_DYGMA( false, "Unhanled BLE Config state" );

            break;
    }
}

/****************************************************/
/*                        API                       */
/****************************************************/

result_t BleConfig::cfg_init( void )
{
    /* Prepare the config channels */
    memset( cfg_channels, 0x00, sizeof(cfg_channels) );

    /* Prepare the peer handling values */
    cfg_peer_erase_id = PM_PEER_ID_INVALID;

    /* Initially in the disabled state */
    cfg_state = BLECFG_STATE_DISABLED;

    return RESULT_OK;
}

result_t BleConfig::cfg_blecdev_event_inject( blecdev_event_type_t event_type, blecdev_evt_param_t * p_param )
{
    return ble_ll_event_process( event_type, p_param );
}

void BleConfig::cfg_enable( void )
{
    if( cfg_is_enabled() == true )
    {
        /* Already enabled */
        return;
    }

    /* Start with loading the peers */
    cfg_state_load_peers_set();
}

void BleConfig::cfg_disable( void )
{
    if( cfg_is_enabled() == false )
    {
        /* Already disabled */
        return;
    }

//    cfg_state_disabled_set();
}

bool BleConfig::cfg_is_enabled( void )
{
    return ( cfg_state == BLECFG_STATE_DISABLED ) ? false : true;
}

void BleConfig::cfg_run( void )
{
    cfg_state_machine();
}

class BleConfig BleConfig;

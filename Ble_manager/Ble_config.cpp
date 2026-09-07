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
#include "Config_manager.h"

static const ble_device_name_t ble_device_name_local = { BLE_DEVICE_NAME };

/****************************************************/
/*                    HMI events                    */
/****************************************************/

inline void BleConfig::cfg_event_process( void * p_instance, blecfg_event_type_t event_type )
{
    if( event_cb == NULL )
    {
        return;
    }

    event_cb( p_instance, event_type );
}

/********************************************************************/
/*                 Low Level BLE - Composite device                 */
/********************************************************************/

inline result_t BleConfig::ble_ll_event_type_peer_erased( void )
{
    if( cfg_state != BLECFG_STATE_PEER_ERASE_WAIT )
    {
        ASSERT_DYGMA( false, "Unexpected BLE peer erase detected (success)" );

        return RESULT_OK;   /* The event has been consumed */
    }

    /* Clear the erase peer ID */
    cfg_peer_erase_id = PM_PEER_ID_INVALID;

    /* Refresh the list of channels */
    cfg_channels_load();

    /* Return to the enabled state */
    cfg_state_set( BLECFG_STATE_ENABLED );

    /* Report the peer erase success event */
    cfg_event_process( p_instance, BLECFG_EVENT_TYPE_CHANNEL_ERASE_SUCCESS );

    return RESULT_OK;   /* The event has been consumed */
}

inline result_t BleConfig::ble_ll_event_type_peer_erase_failed( void )
{
    ASSERT_DYGMA( false, "BLE peer erase not expected to fail." );

    if( cfg_state != BLECFG_STATE_PEER_ERASE_WAIT )
    {
        ASSERT_DYGMA( false, "Unexpected BLE peer erase detected (failure)" );

        return RESULT_OK;   /* The event has been consumed */
    }

    /*
     * We currently do not expect the erase to fail. As a failsafe, we just cancel the erase process by refreshing the list of channels and
     * returning to the enabled state.
     */

    /* Clear the erase peer ID */
    cfg_peer_erase_id = PM_PEER_ID_INVALID;

    /* Refresh the list of channels */
    cfg_channels_load();

    /* Return to the enabled state */
    cfg_state_set( BLECFG_STATE_ENABLED );

    /* Report the peer erase failure event */
    cfg_event_process( p_instance, BLECFG_EVENT_TYPE_CHANNEL_ERASE_FAILED );

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

inline void BleConfig::cfg_channel_reset( blecfg_channel_t * p_channel, uint8_t channel_id )
{
    memset( p_channel, 0x00, sizeof( blecfg_channel_t ) );

    p_channel->channel_id = channel_id;
    p_channel->peer_id = PM_PEER_ID_INVALID;
}


inline result_t BleConfig::cfg_channel_erase( blecfg_channel_t * p_channel )
{
    return cfg_channel_peer_erase( p_channel->peer_id );
}

inline result_t BleConfig::cfg_channel_bond_make( const blecfg_channel_t * p_blecfg_channel )
{
    blecfg_channel_t * p_channel;

    ASSERT_DYGMA( p_blecfg_channel->channel_id <= BLECFG_CHANNELS_COUNT, "Invalid CFG Channel ID" );

    /* Check if the bond process is already in progress */
    if( cfg_channel_bond_id != BLECFG_CHANNEL_ID_INVALID )
    {
        return RESULT_BUSY;
    }

    /* Get the Config channel and populate it with the p_blecfg_channel data */
    p_channel = &cfg_channels[p_blecfg_channel->channel_id];

    /* Check if the channel is already bound */
    if( p_channel->peer_id != PM_PEER_ID_INVALID )
    {
        ASSERT_DYGMA( false, "Trying to make a bond on already occupied BLE channel" );
        return RESULT_ERR;
    }

    /* Update the channel data */
    *p_channel = *p_blecfg_channel;

    /* Set the Channel Bond ID to let the state machine know the bonding is requested */
    cfg_channel_bond_id = p_channel->channel_id;

    return RESULT_OK;
}

inline void BleConfig::cfg_channel_bond_resolve( blecfg_channel_t * p_channel )
{
    bool_t channel_paired;
    uint8_t channel_mask = ( 1 << p_channel->channel_id );

    /* Get the channel paired status */
    channel_paired = ( p_channel->peer_id == PM_PEER_ID_INVALID ) ? false : true;

    if ( channel_paired == true )
    {
        cfg_channels_bond_mask |= channel_mask;
    }
    else
    {
        cfg_channels_bond_mask &= ~channel_mask;
    }
}

//void BleManager::channels_update( void )
//{
//    uint8_t i;
//
//    for ( i = 0; i < BLE_CHANNELS_COUNT; i++ )
//    {
//        channel_paired_set( &p_config->channels[i] );
//    }
//
//    BLE_LOG_DEBUG("Ble_manager: %i channels in total.", i);
//
//    /* Set the current channel for the later use */
//    p_channel_current = &p_config->channels[ p_config->current_channel_id ];
//}

//void BleManager::channel_paired_set( const channel_t * p_channel )
//{
//    bool_t channel_paired;
//    uint8_t channel_mask = ( 1 << p_channel->id );
//
//    /* Get the channel paired status */
//    channel_paired = ( p_channel->peer_id == PM_PEER_ID_INVALID ) ? false : true;
//
//    if ( channel_paired == true )
//    {
//        channels_paired_mask |= channel_mask;
//    }
//    else
//    {
//        channels_paired_mask &= ~channel_mask;
//    }
//
//    BLE_LOG_DEBUG("Ble_manager: Channel %i -> %d", p_channel->id, channel_paired );
//}

inline result_t BleConfig::cfg_channel_peer_load( pm_peer_id_t peer_id )
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

    ASSERT_DYGMA( p_channel->channel_id == peer_app_data.channel_id, "BLE Config Channel inconsistent" );
    p_channel->peer_id = peer_id;
    p_channel->device_addr = peer_app_data.device_addr;

_EXIT:
    return result;
}

inline result_t BleConfig::cfg_channel_peer_erase( pm_peer_id_t peer_id )
{
    if( cfg_peer_erase_id != PM_PEER_ID_INVALID )
    {
        return RESULT_BUSY;
    }

    /* Save the peer id to be erased */
    cfg_peer_erase_id = peer_id;

    return RESULT_OK;
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

inline result_t BleConfig::cfg_channels_init( void )
{
    /* Initialy reset all channels */
    cfg_channels_reset();

    return RESULT_OK;
}

inline void BleConfig::cfg_channels_reset( void )
{
    uint8_t i;

    /* Reset the bond channels mask */
    cfg_channels_bond_mask = 0x00;

    /* Reset the channels */
    for( i = 0; i < BLECFG_CHANNELS_COUNT; i++ )
    {
        cfg_channel_reset( &cfg_channels[i], i );
    }
}

inline result_t BleConfig::cfg_channels_load( void )
{
    result_t result = RESULT_ERR;
    result_t result_aux = RESULT_ERR;

    pm_peer_id_t peer_list[ BLECFG_CHANNELS_COUNT ];
    uint32_t peer_cnt;
    uint32_t peer_id;

    /* Initialy reset all channels */
    cfg_channels_reset();

    /* Get the list of peers */
    peer_cnt = BLECFG_CHANNELS_COUNT;          /* Initializing the size of the peer list */
    result = blecdev_peer_list_get( peer_list, &peer_cnt );
    ASSERT_DYGMA( result == RESULT_OK, "blecdev_peer_list_get failed" );
    EXIT_IF_ERR( result, "blecdev_peer_list_get failed" );

    for( peer_id = 0; peer_id < peer_cnt; peer_id++ )
    {
        result_aux = cfg_channel_peer_load( peer_id );
        if( result_aux == RESULT_ERR )
        {
            /* The channel could not be loaded from this peer. It does not contain APP data. This can happen when the peer has been saved in the peer manager
             * but we could not add our app data to that peer. E.g. due to a system failure during the process. We need to clear the peer from the peer memory */
            cfg_channel_peer_erase( peer_id );
        }
    }

    /* Resolve the channel bonds  */
    cfg_channels_bonds_resolve();

_EXIT:
    return result;
}

inline void BleConfig::cfg_channels_bonds_resolve( void )
{
    uint8_t i;

    for ( i = 0; i < BLECFG_CHANNELS_COUNT; i++ )
    {
        cfg_channel_bond_resolve( &cfg_channels[ i ] );
    }
}

/****************************************************/
/*                   State machine                  */
/****************************************************/

inline void BleConfig::cfg_state_set( blecfg_state_t blecfg_state )
{
    this->cfg_state = blecfg_state;
    mcu_sleep_postpone();
}

inline void BleConfig::cfg_state_disabled_set( void )
{
    /* Clear the handling values */
    cfg_channel_bond_id = BLECFG_CHANNEL_ID_INVALID;
    cfg_peer_erase_id = PM_PEER_ID_INVALID;

    /* Now move to the disabled state */
    cfg_state_set( BLECFG_STATE_DISABLED );
}

//inline void BleConfig::cfg_state_load_peers_set( void )
//{
//    /* Clear the channels first */
//    cfg_channels_reset();
//
//    /* Set the Load peers state */
//    cfg_state_set( BLECFG_STATE_LOAD_PEERS );
//}

inline void BleConfig::cfg_state_peer_erase_set( pm_peer_id_t peer_id )
{
    cfg_peer_erase_id = peer_id;

    /* Set the Load peers state */
    cfg_state_set( BLECFG_STATE_PEER_ERASE );
}

//inline void BleConfig::cfg_state_load_peers_process( void )
//{
//    result_t result = RESULT_ERR;
//
//    pm_peer_id_t peer_list[ BLECFG_CHANNELS_COUNT ];
//    uint32_t peer_cnt;
//    uint32_t peer_id;
//
//    /* Get the list of peers */
//    peer_cnt = BLECFG_CHANNELS_COUNT;          /* Initializing the size of the peer list */
//    result = blecdev_peer_list_get( peer_list, &peer_cnt );
//    ASSERT_DYGMA( result == RESULT_OK, "blecdev_peer_list_get failed" );
//
//    for( peer_id = 0; peer_id < peer_cnt; peer_id++ )
//    {
//        result = cfg_channel_load( peer_id );
//        if( result == RESULT_ERR )
//        {
//            /* The channel could not be loaded from this peer. It does not contain APP data. This can happen when the peer has been saved in the peer manager
//             * but we could not add our app data to that peer. E.g. due to a system failure during the process. We need to clear the peer from the peer memory */
//            cfg_state_peer_erase_set( peer_id );
//            return;
//        }
//    }
//
//    /* The channels are loaded, move to the enabled state */
//    cfg_state_set( BLECFG_STATE_ENABLED );
//}

inline void BleConfig::cfg_state_enabled_process( void )
{
    if( cfg_peer_erase_id != PM_PEER_ID_INVALID )
    {
        cfg_state_set( BLECFG_STATE_PEER_ERASE );
    }
    else if( cfg_channel_bond_id != BLECFG_CHANNEL_ID_INVALID )
    {
        cfg_state_set( BLECFG_STATE_BOND_MAKE );
    }
}

inline void BleConfig::cfg_state_bond_make_process( void )
{
    result_t result = RESULT_ERR;
    blecfg_peer_app_data_t peer_app_data;

    blecfg_channel_t * p_channel = &cfg_channels[ cfg_channel_bond_id ];

    /* Prepare the bonding peer app data */
    peer_app_data.magic_number = BLECFG_PEER_APP_DATA_MAGIC_NUMBER;
    peer_app_data.version = BLECFG_PEER_APP_DATA_VERSION;
    peer_app_data.channel_id = p_channel->channel_id;
    peer_app_data.device_addr = p_channel->device_addr;

    /* Request the peer app data store */
    result = blecdev_peer_app_data_store( p_channel->peer_id, &peer_app_data, sizeof( peer_app_data ) );
    ASSERT_DYGMA( result == RESULT_OK, "blecdev_peer_app_data_store failed" );

    UNUSED( result );
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

        case BLECFG_STATE_ENABLED:

            cfg_state_enabled_process();

            break;

        case BLECFG_STATE_BOND_MAKE:

            cfg_state_bond_make_process();

            break;

        case BLECFG_STATE_BOND_MAKE_WAIT:

            /* Waiting for the channel bond make finish  */

            break;

//        case BLECFG_STATE_LOAD_PEERS:
//
//            cfg_state_load_peers_process();
//
//            break;

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
/*                   Config Memory                  */
/****************************************************/

void BleConfig::cfgmem_ble_name_save( const ble_device_name_t * p_name_config, const ble_device_name_t * p_device_name )
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

//void BleConfig::cfgmem_channel_id_save( const channel_t * p_channel, uint8_t id )
//{
//    result_t result = RESULT_ERR;
//
//    result = ConfigManager.config_item_update( &p_channel->id, &id, sizeof( p_channel->id ) );
//    ASSERT_DYGMA( result == RESULT_OK, "ConfigManager.config_item_update failed" );
//
//    UNUSED( result );
//}
//
//void BleConfig::cfgmem_channel_peer_id_save( const channel_t * p_channel, pm_peer_id_t peer_id )
//{
//    result_t result = RESULT_ERR;
//
//    result = ConfigManager.config_item_update( &p_channel->peer_id, &peer_id, sizeof( p_channel->peer_id ) );
//    ASSERT_DYGMA( result == RESULT_OK, "ConfigManager.config_item_update failed" );
//
//    UNUSED( result );
//}
//
//void BleConfig::cfgmem_channel_device_address_save( const channel_t * p_channel, const ble_device_addr_t * p_device_addr )
//{
//    result_t result = RESULT_ERR;
//
//    result = ConfigManager.config_item_update( &p_channel->device_addr, p_device_addr, sizeof( p_channel->device_addr ) );
//    ASSERT_DYGMA( result == RESULT_OK, "ConfigManager.config_item_update failed" );
//
//    UNUSED( result );
//}
//
//void BleConfig::cfgmem_channel_device_name_save( const channel_t * p_channel, const ble_device_name_t * p_device_name )
//{
//    cfgmem_ble_name_save( &p_channel->device_name, p_device_name );
//}

void BleConfig::cfgmem_device_name_local_save( const ble_device_name_t * p_device_name_local )
{
    cfgmem_ble_name_save( &p_ble_config->device_name_local, p_device_name_local );
}

void BleConfig::cfgmem_current_channel_id_save( uint8_t channel_id )
{
    result_t result = RESULT_ERR;

    result = ConfigManager.config_item_update( &p_ble_config->current_channel_id, &channel_id, sizeof( p_ble_config->current_channel_id) );
    ASSERT_DYGMA( result == RESULT_OK, "ConfigManager.config_item_update failed" );

    UNUSED( result );
}

void BleConfig::cfgmem_force_ble_save( bool_t force_ble )
{
    result_t result = RESULT_ERR;

    result = ConfigManager.config_item_update( &p_ble_config->force_ble, &force_ble, sizeof( p_ble_config->force_ble) );
    ASSERT_DYGMA( result == RESULT_OK, "ConfigManager.config_item_update failed" );

    UNUSED( result );
}

//void BleConfig::cfgmem_channel_reset( const channel_t * p_channel, uint8_t id )
//{
//    ble_device_addr_t default_device_addr;
//    ble_device_name_t default_device_name;
//
//    memset( &default_device_addr, 0xFF, sizeof(default_device_addr) );
//    memset( &default_device_name, 0x00, sizeof(default_device_name) );
//
//    cfgmem_channel_id_save( p_channel, id );
//    cfgmem_channel_peer_id_save( p_channel, PM_PEER_ID_INVALID );
//    cfgmem_channel_device_address_save( p_channel, &default_device_addr );
//    cfgmem_channel_device_name_save( p_channel, &default_device_name );
//}

void BleConfig::cfgmem_config_reset()
{
//    uint8_t i;
//    for( i = 0; i < BLE_CHANNELS_COUNT; i++)
//    {
//        cfgmem_channel_reset( &p_ble_config->channels[i], i );
//    }

    cfgmem_device_name_local_save( &ble_device_name_local );
    cfgmem_current_channel_id_save( 0 );
//    cfgmem_force_ble_save( false );
}

/****************************************************/
/*                        API                       */
/****************************************************/

result_t BleConfig::cfg_init( const blecfg_config_t * p_config )
{
    result_t result = RESULT_ERR;

    /* First, get the current BLE configuration */
    result = ConfigManager.config_item_request( ConfigManager::CFG_ITEM_TYPE_BLE_CONNECTIONS, (const void **)&p_ble_config );
    EXIT_IF_ERR( result, "ConfigManager.config_item_request failed" );

    // For now lest think that if this variable is invalid, restart everything.
    if( p_ble_config->current_channel_id == 0xFF )
    {
        cfgmem_config_reset();
    }

    /* Prepare the config channels */
    result = cfg_channels_init();
    EXIT_IF_ERR( result, "cfg_channels_init failed" );

    /* Prepare the handling values */
    cfg_channel_bond_id = BLECFG_CHANNEL_ID_INVALID;
    cfg_peer_erase_id = PM_PEER_ID_INVALID;

    /* Event callback */
    this->p_instance = p_config->p_instance;
    this->event_cb = p_config->event_cb;

    /* Initially in the disabled state */
    cfg_state = BLECFG_STATE_DISABLED;


_EXIT:
    return result;
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

    /* Load the channels */
    cfg_channels_load();

    /* Move to the enabled state */
    cfg_state_set( BLECFG_STATE_ENABLED );
}

void BleConfig::cfg_disable( void )
{
    if( cfg_is_enabled() == false )
    {
        /* Already disabled */
        return;
    }

    cfg_state_disabled_set();
}

bool BleConfig::cfg_is_enabled( void )
{
    return ( cfg_state == BLECFG_STATE_DISABLED ) ? false : true;
}

const ble_device_name_t * BleConfig::cfg_ble_device_name_local_get( void )
{
    return &p_ble_config->device_name_local;
}

void BleConfig::cfg_current_channel_set( uint8_t channel_id )
{
    ASSERT_DYGMA( channel_id < BLECFG_CHANNELS_COUNT, "BLE Config trying to save invalid channel id" );

    cfgmem_current_channel_id_save( channel_id );
}

const BleConfig::blecfg_channel_t * BleConfig::cfg_current_channel_get( void )
{
    return &cfg_channels[ p_ble_config->current_channel_id ];
}

result_t BleConfig::cfg_channel_bond_save( const blecfg_channel_t * p_blecfg_channel )
{
    if( cfg_is_enabled() == false )
    {
        return RESULT_ERR;
    }

    return cfg_channel_bond_make( p_blecfg_channel );
}

uint8_t BleConfig::cfg_channels_bond_mask_get( void )
{
    return cfg_channels_bond_mask;
}

void BleConfig::cfg_force_ble_set( bool enabled )
{
    cfgmem_force_ble_save( enabled );
}

bool BleConfig::cfg_force_ble_get( void )
{
    return p_ble_config->force_ble;
}

void BleConfig::cfg_run( void )
{
    cfg_state_machine();
}

class BleConfig BleConfig;

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

#pragma once

#include "dl_middleware.h"

#include "Ble_composite_dev.h"
#include "ble_types.h"


#define BLECFG_CHANNELS_COUNT                   5
#define BLECFG_CHANNEL_ID_INVALID               0xFF

#define BLECFG_PEER_APP_DATA_MAGIC_NUMBER       0x44594731      /* DYG1 */
#define BLECFG_PEER_APP_DATA_VERSION            1               /* The first version of the peer app data */

class BleConfig
{
    public:

        typedef struct PACK
        {
//            channel_t channels[BLE_CHANNELS_COUNT];
            ble_device_name_t device_name_local;    /* The local BLE device name */
            uint8_t current_channel_id;             /* The ID of the currently selected channel */
            bool_t force_ble;
        } ble_config_t;

    public:

        typedef enum
        {
            BLECFG_EVENT_TYPE_CHANNEL_BOND_SAVE_SUCCESS = 1,
            BLECFG_EVENT_TYPE_CHANNEL_BOND_SAVE_FAILED,
            BLECFG_EVENT_TYPE_CHANNEL_ERASE_SUCCESS,
            BLECFG_EVENT_TYPE_CHANNEL_ERASE_FAILED,
        } blecfg_event_type_t;

        typedef struct
        {
            uint8_t channel_id;
        } blecfg_evt_channel_bond_save_success_param_t;

        typedef struct
        {
            uint8_t channel_id;
        } blecfg_evt_channel_bond_save_failed_param_t;

        typedef struct
        {
            uint8_t channel_id;
        } blecfg_evt_channel_erase_success_param_t;

        typedef struct
        {
            uint8_t channel_id;
        } blecfg_evt_channel_erase_failed_param_t;

        typedef union
        {
            blecfg_evt_channel_bond_save_success_param_t channel_bond_save_success;
            blecfg_evt_channel_bond_save_failed_param_t channel_bond_save_failed;
            blecfg_evt_channel_erase_success_param_t channel_erase_success;
            blecfg_evt_channel_erase_failed_param_t channel_erase_failed;
        } blecfg_evt_param_t;

        typedef void (* blecfg_event_cb)( void * p_instance, blecfg_event_type_t event_type, blecfg_evt_param_t * p_param );

        typedef struct PACK
        {
//            bool_t loaded;

            uint8_t channel_id;               /* The ID of the channel in the list of the channels */
            pm_peer_id_t peer_id;
            ble_device_addr_t device_addr;
//            ble_device_name_t device_name;  /* The name of the remote device connected via the channel */
        } blecfg_channel_t;

        typedef struct
        {
            /* Event callback */
            void * p_instance;
            blecfg_event_cb event_cb;
        } blecfg_config_t;


        result_t cfg_init( const blecfg_config_t * p_config );
        result_t cfg_blecdev_event_inject( blecdev_event_type_t event_type, blecdev_evt_param_t * p_param );

        void cfg_enable( void );
        void cfg_disable( void );

        bool cfg_is_enabled( void );
        bool cfg_is_busy( void );

        const ble_device_name_t * cfg_ble_device_name_local_get( void );

        void cfg_current_channel_set( uint8_t channel_id );
        const blecfg_channel_t * cfg_current_channel_get( void );
        result_t cfg_channel_bond_save( const blecfg_channel_t * p_blecfg_channel );
        result_t cfg_channel_id_erase( uint8_t channel_id );

        uint8_t cfg_channels_bond_mask_get( void );
        void cfg_force_ble_set( bool enabled );
        bool cfg_force_ble_get( void );

        void cfg_run( void );

    private:

        typedef struct PACK
        {
            uint32_t magic_number;
            uint8_t version;
            uint8_t channel_id;                     /* The ID of the channel in the list of the channels */
//            pm_peer_id_t peer_id;
            ble_device_addr_t device_addr;
//            ble_device_name_t device_name;  /* The name of the remote device connected via the channel */
        } blecfg_peer_app_data_t;

        typedef enum
        {
            BLECFG_STATE_DISABLED = 1,

            BLECFG_STATE_ENABLED,
            BLECFG_STATE_BOND_MAKE,
            BLECFG_STATE_BOND_MAKE_WAIT,
//            BLECFG_STATE_LOAD_PEERS,
            BLECFG_STATE_PEER_ERASE,
            BLECFG_STATE_PEER_ERASE_WAIT,
        } blecfg_state_t;

        const ble_config_t * p_ble_config = nullptr;

        blecfg_state_t cfg_state;

        blecfg_channel_t cfg_channels[ BLECFG_CHANNELS_COUNT ];
        uint8_t cfg_channels_bond_mask;

        uint8_t cfg_channel_bond_id;
        uint8_t cfg_channel_erase_id;
        pm_peer_id_t cfg_peer_erase_id;

        /* Event callback */
        void * p_instance;
        blecfg_event_cb event_cb;

        inline void cfg_event_process( void * p_instance, blecfg_event_type_t event_type, blecfg_evt_param_t * p_param );

        inline result_t ble_ll_event_type_peer_app_data_stored( void );
        inline result_t ble_ll_event_type_peer_app_data_store_failed( void );
        inline result_t ble_ll_event_type_peer_erased( void );
        inline result_t ble_ll_event_type_peer_erase_failed( void );
        inline result_t ble_ll_event_process( blecdev_event_type_t event_type, blecdev_evt_param_t * p_param );

        inline void cfg_channel_reset( blecfg_channel_t * p_channel, uint8_t channel_id );
        inline result_t cfg_channel_erase( blecfg_channel_t * p_channel );
        inline result_t cfg_channel_bond_make( const blecfg_channel_t * p_blecfg_channel );
        inline void cfg_channel_bond_resolve( blecfg_channel_t * p_channel );
        inline result_t cfg_channel_peer_load( pm_peer_id_t peer_id );
        inline result_t cfg_channel_peer_erase( pm_peer_id_t peer_id );

        inline result_t cfg_channels_init( void );
        inline void cfg_channels_reset( void );
        inline result_t cfg_channels_load( void );
        inline void cfg_channels_bonds_resolve( void );

        inline void cfg_state_set( blecfg_state_t blecfg_state );
        inline void cfg_state_disabled_set( void );
//        inline void cfg_state_load_peers_set( void );
//        inline void cfg_state_peer_erase_set( pm_peer_id_t peer_id );
        inline void cfg_state_disabled_process( void );
        inline void cfg_state_enabled_process( void );
        inline void cfg_state_bond_make_process( void );
//        inline void cfg_state_load_peers_process( void );
        inline void cfg_state_peer_erase_process( void );
        inline void cfg_state_machine( void );

        void cfgmem_ble_name_save( const ble_device_name_t * p_name_config, const ble_device_name_t * p_device_name );

//        void cfgmem_channel_id_save( const channel_t * p_channel, uint8_t id );
//        void cfgmem_channel_peer_id_save( const channel_t * p_channel, pm_peer_id_t peer_id );
//        void cfgmem_channel_device_address_save( const channel_t * p_channel, const ble_device_addr_t * p_device_addr );
//        void cfgmem_channel_device_name_save( const channel_t * p_channel, const ble_device_name_t * p_device_name );

        void cfgmem_device_name_local_save( const ble_device_name_t * p_device_name_local );
        void cfgmem_current_channel_id_save( uint8_t channel_id );
        void cfgmem_force_ble_save( bool_t force_ble );

//        void cfgmem_channel_reset( const channel_t * p_channel, uint8_t id );
        void cfgmem_config_reset();
};

extern class BleConfig BleConfig;

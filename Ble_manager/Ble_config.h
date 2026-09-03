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

class BleConfig
{
    public:

        typedef struct PACK
        {
            bool_t loaded;

            uint8_t channel_id;               /* The ID of the channel in the list of the channels */
            pm_peer_id_t peer_id;
//            ble_device_addr_t device_addr;
//            ble_device_name_t device_name;  /* The name of the remote device connected via the channel */
        } blecfg_channel_t;

//        typedef struct PACK
//        {
//            channel_t channels[BLE_CHANNELS_COUNT];
//            ble_device_name_t device_name_local;    /* The local BLE device name */
//            uint8_t current_channel_id;             /* The ID of the currently selected channel */
//            bool_t force_ble;
//        } config_t;

        result_t cfg_init( void );
        result_t cfg_blecdev_event_inject( blecdev_event_type_t event_type, blecdev_evt_param_t * p_param );

        void cfg_enable( void );
        void cfg_disable( void );

        bool cfg_is_enabled( void );

        void cfg_run( void );

    private:

#define BLECFG_CHANNELS_COUNT                   5
#define BLECFG_PEER_APP_DATA_MAGIC_NUMBER       0x44594731      /* DYG1 */
#define BLECFG_PEER_APP_DATA_VERSION            1               /* The first version of the peer app data */

        typedef struct PACK
        {
            uint32_t magic_number;
            uint8_t version;
            uint8_t channel_id;                     /* The ID of the channel in the list of the channels */
//            pm_peer_id_t peer_id;
//            ble_device_addr_t device_addr;
//            ble_device_name_t device_name;  /* The name of the remote device connected via the channel */
        } blecfg_peer_app_data_t;

        typedef enum
        {
            BLECFG_STATE_DISABLED = 1,

            BLECFG_STATE_ENABLED,
            BLECFG_STATE_LOAD_PEERS,
            BLECFG_STATE_PEER_ERASE,
            BLECFG_STATE_PEER_ERASE_WAIT,
        } blecfg_state_t;

        blecfg_state_t cfg_state;

        blecfg_channel_t cfg_channels[ BLECFG_CHANNELS_COUNT ];

        pm_peer_id_t cfg_peer_erase_id;

        inline result_t ble_ll_event_type_peer_erased( void );
        inline result_t ble_ll_event_type_peer_erase_failed( void );
        inline result_t ble_ll_event_process( blecdev_event_type_t event_type, blecdev_evt_param_t * p_param );

        inline result_t cfg_channel_load( pm_peer_id_t peer_id );
//        inline result_t cfg_channel_erase( pm_peer_id_t peer_id );

        inline void cfg_state_set( blecfg_state_t blecfg_state );
        inline void cfg_state_load_peers_set( void );
        inline void cfg_state_peer_erase_set( pm_peer_id_t peer_id );
        inline void cfg_state_disabled_process( void );
        inline void cfg_state_load_peers_process( void );
        inline void cfg_state_peer_erase_process( void );
        inline void cfg_state_machine( void );
};

extern class BleConfig BleConfig;

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
 *
 */

#pragma once

#include "dl_middleware.h"

#include "Ble_config.h"
#include "Ble_composite_dev.h"
#include "Ble_hmi.h"
#include "ble_types.h"
#include "kbd_if.h"
#include "Time_counter.h"

class BleManager
{
  public:

    result_t init( void );

    result_t enable( void );
    result_t disable( void );
    result_t restart( void );

    bool_t is_enabled( void );
    bool_t is_connected( void );

    void battery_level_update( uint8_t bat_level );

    void force_ble_set( bool enabled );
    bool force_ble_get( void );

    void run();

  private:

    typedef enum
    {
        BLEM_STATE_DISABLED = 1,
        BLEM_STATE_ENABLE,
        BLEM_STATE_ENABLED,
        BLEM_STATE_ADVERTISING,
        BLEM_STATE_PAIRING,
        BLEM_STATE_CONNECTED,

        BLEM_STATE_DISABLE,
        BLEM_STATE_RESTART,
        BLEM_STATE_ADVERTISING_FAIL,

        BLEM_STATE_ESCAPE,
    } blem_state_t;

    blem_state_t state;

    const BleConfig::blecfg_channel_t * p_channel_current = NULL;

    kbdif_t * p_kbdif = NULL;

    /* Battery value */
    uint8_t battery_level;

    /* Flags */
    bool_t enable_request_flag = false;
    bool_t enabled_flag = false;

    result_t ble_ll_init( void );

    inline void ble_ll_event_type_advertising_process( void );
    inline void ble_ll_event_type_advertising_failed_process( void );
    inline void ble_ll_event_type_sec_bond_code_req_process( void );
    inline void ble_ll_event_type_sec_bond_success_process( blecdev_evt_sec_bond_success_param_t * p_sec_bond_success_param );
    inline void ble_ll_event_type_sec_bond_failed_process( blecdev_evt_sec_bond_failed_param_t * p_sec_bond_failed_param );
    inline void ble_ll_event_type_peer_connected_process( blecdev_evt_peer_connected_param_t * p_peer_connected_param );
    inline void ble_ll_event_type_peer_device_name_process( blecdev_evt_peer_device_name_param_t * p_peer_device_name_param );
    void ble_ll_event_process( blecdev_event_type_t event_type, blecdev_evt_param_t * p_param );

    result_t blem_disable( void );

    inline void state_set( blem_state_t blem_state );
    inline void state_connected_set( void );
    inline void state_disabled_process( void );
    inline void state_enable_process( void );
    inline void state_disable_process( void );
    inline void state_restart_process( void );
    inline void state_advertising_fail_process( void );
    inline void state_escape_process( void );
    inline void state_machine( void );

    inline result_t blecfg_init( void );
    inline void blecfg_event_type_channel_bond_save_success_process( BleConfig::blecfg_evt_channel_bond_save_success_param_t * p_channel_bond_save_success_param );
    inline void blecfg_event_type_channel_bond_save_failed_process( BleConfig::blecfg_evt_channel_bond_save_failed_param_t * p_channel_bond_save_failed_param );
    inline void blecfg_event_type_channel_erase_success_process( BleConfig::blecfg_evt_channel_erase_success_param_t * p_channel_erase_success_param );
    inline void blecfg_event_type_channel_erase_failed_process( BleConfig::blecfg_evt_channel_erase_failed_param_t * p_channel_erase_failed_param );
    inline void blecfg_event_process( BleConfig::blecfg_event_type_t event_type, BleConfig::blecfg_evt_param_t * p_param );

    inline result_t blehmi_init( void );
    inline void blehmi_event_type_channel_change_process( BleHmi::blehmi_evt_channel_change_param_t * p_channel_change_param );
    inline void blehmi_event_type_channel_erase_process( BleHmi::blehmi_evt_channel_erase_param_t * p_channel_erase_param );
    inline void blehmi_event_type_bond_code_ready_process( BleHmi::blehmi_evt_bond_code_ready_param_t * p_bond_code_ready_param );
    inline void blehmi_event_type_deactivate_request_process( void );
    inline void blehmi_event_type_escape_request_process( void );
    inline void blehmi_event_process( BleHmi::blehmi_event_type_t event_type, BleHmi::blehmi_evt_param_t * p_param );

    void bat_level_update( void );

    result_t kbdif_initialize(void);

  private:
    static const kbdif_handlers_t kbdif_handlers;

    static kbdapi_event_result_t kbdif_command_event_cb( void * p_instance, const char * p_command );

    static void ble_ll_event_cb( void * p_instance, blecdev_event_type_t event_type, blecdev_evt_param_t * p_param );
    static void blecfg_event_cb( void * p_instance, BleConfig::blecfg_event_type_t event_type, BleConfig::blecfg_evt_param_t * p_param );
    static void blehmi_event_cb( void * p_instance, BleHmi::blehmi_event_type_t event_type, BleHmi::blehmi_evt_param_t * p_param );
};

extern class BleManager BleManager;

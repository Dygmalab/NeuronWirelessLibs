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

#pragma once

#include "dl_middleware.h"

#include "ble_types.h"
#include "kbd_if.h"
#include "keyboard_api.h"
#include "Time_counter.h"

class BleHmi
{
    public:

        typedef enum
        {
            BLEHMI_EVENT_TYPE_CHANNEL_CHANGE = 1,
            BLEHMI_EVENT_TYPE_CHANNEL_ERASE,
            BLEHMI_EVENT_TYPE_BOND_CODE_READY,
        } blehmi_event_type_t;

        typedef struct
        {
            uint8_t channel_id;
        } blehmi_evt_channel_change_param_t;

        typedef struct
        {
            uint8_t channel_id;
        } blehmi_evt_channel_erase_param_t;

        typedef struct
        {
            ble_bond_code bond_code;
        } blehmi_evt_bond_code_ready_param_t;

        typedef union
        {
            blehmi_evt_channel_change_param_t channel_change;
            blehmi_evt_channel_erase_param_t channel_erase;
            blehmi_evt_bond_code_ready_param_t bond_code_ready;
        } blehmi_evt_param_t;

        typedef void (* blehmi_event_cb)( void * p_instance, blehmi_event_type_t event_type, blehmi_evt_param_t * p_param );

        typedef struct
        {
            /* Event callback */
            void * p_instance;
            blehmi_event_cb event_cb;
        } blehmi_config_t;

        result_t hmi_init( const blehmi_config_t * p_config );

        void hmi_enable( void );
        void hmi_disable( void );
        void hmi_activate( void );
        void hmi_deactivate( void );
        bool hmi_is_enabled( void );
        bool hmi_is_active( void );

        void hmi_advertise( uint8_t channel_current_id, uint8_t channels_bonded_mask );
        void hmi_read_bond_code( void );

        void hmi_run( void );

    private:

        typedef enum
        {
            BLEHMI_STATE_DISABLED = 1,
            BLEHMI_STATE_ENABLED,
            BLEHMI_STATE_ACTIVE,
            BLEHMI_STATE_READING_BOND_CODE,
            BLEHMI_STATE_CHANNEL_ERASE_KEY_WAIT,
            BLEHMI_STATE_CHANNEL_ERASE,
        } blehmi_state_t;

        blehmi_state_t hmi_state;

        kbdif_t * p_kbdif = NULL;
        kbdapi_key_report_lock_t kbdapi_key_report_lock;

        bool_t hmi_activate_req_flag;
        bool_t hmi_deactivate_req_flag;
        bool_t hmi_active_flag;

        ble_bond_code_t hmi_bond_code;
        uint8_t hmi_bond_code_len;

        uint8_t hmi_channel_current_id;
        uint8_t hmi_channel_erase_id;
        uint8_t hmi_channels_bonded_mask;

        dl_timer_t hmi_timer;

        /* Event callback */
        void * p_instance;
        blehmi_event_cb event_cb;

        inline void hmi_event_process( void * p_instance, blehmi_event_type_t event_type, blehmi_evt_param_t * p_param );

        inline void hmi_state_set( blehmi_state_t blehmi_state );
        inline void hmi_state_enabled_set( void );
        inline void hmi_state_disabled_set( void );
        inline void hmi_state_active_set( void );
        inline void hmi_state_reading_bond_code_set( void );
        inline void hmi_state_channel_erase_key_wait_set( void );
        inline void hmi_state_enabled_process( void );
        inline void hmi_state_active_process( void );
        inline void hmi_state_reading_bond_code_process( void );
        inline void hmi_state_channel_erase_key_wait_process( void );
        inline void hmi_state_machine( void );

        inline result_t hmi_channel_resolve( uint8_t * p_channel_id, kbdapi_key_t * p_key );

        inline result_t hmi_key_num_to_ascii( kbdapi_key_t * p_key, char * p_ascii );
        inline result_t hmi_key_enabled_process( kbdapi_key_t * p_key );
        inline void hmi_key_active_channel_change_process( kbdapi_key_t * p_key, uint8_t channel_id );
        inline void hmi_key_active_channel_erase_process( kbdapi_key_t * p_key, uint8_t channel_id );
        inline result_t hmi_key_active_process( kbdapi_key_t * p_key );
        inline result_t hmi_key_reading_bond_code_process( kbdapi_key_t * p_key );
        inline result_t hmi_key_channel_erase_key_wait_process( kbdapi_key_t * p_key );
        inline result_t hmi_key_process( kbdapi_key_t * p_key );

        inline void hmi_led_effect_set( uint8_t channel_con_id, uint8_t channel_adv_id, bool_t erase_status );
        inline void hmi_led_effect_on( void );
        inline void hmi_led_effect_off( void );
        inline void hmi_led_effect_update( uint8_t channel_con_id, uint8_t channel_adv_id, bool_t erase_status );
        inline void hmi_led_effect_adv( void );
        inline void hmi_led_effect_reading_bond_code( void );

        result_t kbdif_initialize(void);
        inline kbdapi_event_result_t kbdif_key_event_process( kbdapi_key_t * p_key );

    private:
      static const kbdif_handlers_t kbdif_handlers;

      static kbdapi_event_result_t kbdif_key_event_cb( void * p_instance, kbdapi_key_t * p_key );
};

extern class BleHmi BleHmi;

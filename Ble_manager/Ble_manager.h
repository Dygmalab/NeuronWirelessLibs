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

#include "Ble_composite_dev.h"
#include "ble_types.h"
#include "kbd_if.h"
#include "keyboard_api.h"
#include "Time_counter.h"

#define BLE_CHANNELS_COUNT      5
//#define BLE_ADDRESS_LEN         6
//#define BLE_DEVICE_NAME_LEN     32  // Same value as flag _BLE_DEVICE_NAME_LEN defined in the Ble_composite_dev.c file.

class BleManager
{
  public:
//    typedef char ble_name_t[BLE_DEVICE_NAME_LEN];
//    typedef uint8_t ble_address_t[BLE_ADDRESS_LEN];

    typedef struct PACK
    {
        uint8_t id;                     /* The ID of the channel in the list of the channels */
        pm_peer_id_t peer_id;
        ble_device_addr_t device_addr;
        ble_device_name_t device_name;  /* The name of the remote device connected via the channel */
    } channel_t;

    typedef struct PACK
    {
        channel_t channels[BLE_CHANNELS_COUNT];
        ble_device_name_t device_name_local;    /* The local BLE device name */
        uint8_t current_channel_id;             /* The ID of the currently selected channel */
        bool_t force_ble;
    } config_t;

  public:
//    bool trigger_save_name_timer = false;

    result_t init( void );

    result_t enable( void );
    result_t disable( void );
    result_t restart( void );
//    bool getForceBle(void);
//    void setForceBle(bool enabled);
//    void set_bt_name_from_specifications(const char *spec);
//
//    void set_pairing_key_press(bool press);
//    bool get_pairing_key_press(void);
//
//    void send_led_mode(void);

#warning "Check the use of the BLE is_enabled function throughout the system"
    bool_t is_enabled( void );
    bool_t is_connected( void );

    void battery_level_update( uint8_t battery_level );

    void force_ble_set( bool enabled );
    bool force_ble_get( void );

    void run();

  private:

//    bool pairing_key_press = false;
//
    enum Channels: uint8_t
    {
        CHANNEL_0 = 0,
        CHANNEL_1,
        CHANNEL_2,
        CHANNEL_3,
        CHANNEL_4,
        NOT_CONNECTED,
        NOT_ON_ADVERTISING
    };

//    // THIS ENUM IS NOT BEING USED
////    enum Pressed_keys: uint8_t
////    {
////        KEY_E = 8,
////        KEY_Q = 20,
////        KEY_R = 21,
////        KEY_T = 23,
////        KEY_W = 26,
////        KEY_1 = 30,
////        KEY_2 = 31,
////        KEY_3 = 32,
////        KEY_4 = 33,
////        KEY_5 = 34,
////    };

    typedef enum
    {
        BLEM_STATE_DISABLED = 1,
        BLEM_STATE_ENABLE,
        BLEM_STATE_ENABLED,
        BLEM_STATE_ADVERTISING,
        BLEM_STATE_PAIRING,
        BLEM_STATE_CONNECTED,
        BLEM_STATE_DISABLE,
    } blem_state_t;

    typedef enum
    {
        BLEM_HMI_STATE_DISABLED = 1,
        BLEM_HMI_STATE_ENABLED,
        BLEM_HMI_STATE_ACTIVE,
        BLEM_HMI_STATE_PAIRING,
        BLEM_HMI_STATE_CHANNEL_ERASE_KEY_WAIT,
        BLEM_HMI_STATE_CHANNEL_ERASE,
    } blem_hmi_state_t;

//    struct ConnectionKeyState
//    {
//        bool longPress;
//        dl_timer_t pressedTimer;
//    };
//    ConnectionKeyState connectionState[BLE_CONNECTIONS_COUNT];

    const config_t * p_config = nullptr;

    blem_state_t state;
    blem_hmi_state_t hmi_state;

//    const char *ble_device_name = nullptr;
    uint8_t channels_paired_mask = 0;           /* The mask of the paired channels */
    const channel_t * p_channel_current = NULL;
    const channel_t * p_channel_erase = NULL;   /* The channel chosen for erase */
//    uint8_t channel_in_use = NOT_CONNECTED;
//    bool showing_bt_layer = false;
//    bool mitm_activated = false;
//    char encryption_pin_number[6] = {};
//
//    bool trigger_save_conn_timer = false;
//    bool timer_save_conn_start_count = false;
//    dl_timer_t timer_save_new_conn = 0;
//
//    bool timer_save_name_start_count = false;
//    dl_timer_t timer_save_new_name = 0;

    kbdif_t * p_kbdif = NULL;
    kbdapi_key_report_lock_t kbdapi_key_report_lock;

    /* Flags */
    bool_t enable_request_flag;
    bool_t enabled_flag;
    bool_t restart_flag;

    bool_t hmi_activate_req_flag;
    bool_t hmi_deactivate_req_flag;
    bool_t hmi_active_flag;

    ble_bond_code_t hmi_bond_code;
    uint8_t hmi_bond_code_len;

    dl_timer_t hmi_timer;

    result_t channels_init( void );
    void channels_update( void );
    void channel_paired_set( const channel_t * p_channel );

    result_t ble_ll_init( void );
    void ble_ll_whitelist_configure( void );

    inline void ble_ll_event_type_advertising_process( void );
    inline void ble_ll_event_type_sec_bond_code_req_process( void );
    inline void ble_ll_event_type_sec_bond_success_process( blecdev_evt_sec_bond_success_param_t * p_sec_bond_success_param );
    inline void ble_ll_event_type_sec_bond_failed_process( blecdev_evt_sec_bond_failed_param_t * p_sec_bond_failed_param );
    inline void ble_ll_event_type_peer_connected_process( blecdev_evt_peer_connected_param_t * p_peer_connected_param );
    inline void ble_ll_event_type_peer_device_name_process( blecdev_evt_peer_device_name_param_t * p_peer_device_name_param );
    void ble_ll_event_process( blecdev_event_type_t event_type, blecdev_evt_param_t * p_param );

    inline void state_set( blem_state_t blem_state );
    inline void state_disabled_process( void );
    inline void state_enable_process( void );
    inline void state_disable_process( void );
    inline void state_machine( void );

    inline void hmi_state_set( blem_hmi_state_t blem_hmi_state );
    inline void hmi_state_enabled_set( void );
    inline void hmi_state_disabled_set( void );
    inline void hmi_state_active_set( void );
    inline void hmi_state_pairing_set( void );
    inline void hmi_state_channel_erase_key_wait_set( void );
    inline void hmi_state_enabled_process( void );
    inline void hmi_state_active_process( void );
    inline void hmi_state_pairing_process( void );
    inline void hmi_state_channel_erase_key_wait_process( void );
    inline void hmi_state_machine( void );

    inline void hmi_enable( void );
    inline void hmi_disable( void );
    inline void hmi_activate( void );
    inline void hmi_deactivate( void );
    inline bool hmi_is_enabled( void );
    inline bool hmi_is_active( void );

    inline result_t hmi_channel_resolve( uint8_t * p_channel_id, kbdapi_key_t * p_key );
    inline void hmi_channel_change( uint8_t channel_id );
    inline void hmi_channel_erase( uint8_t channel_id );

    inline result_t hmi_key_num_to_ascii( kbdapi_key_t * p_key, char * p_ascii );
    inline result_t hmi_key_enabled_process( kbdapi_key_t * p_key );
    inline void hmi_key_active_channel_change_process( kbdapi_key_t * p_key, uint8_t channel_id );
    inline void hmi_key_active_channel_erase_process( kbdapi_key_t * p_key, uint8_t channel_id );
    inline result_t hmi_key_active_process( kbdapi_key_t * p_key );
    inline result_t hmi_key_pairing_process( kbdapi_key_t * p_key );
    inline result_t hmi_key_channel_erase_key_wait_process( kbdapi_key_t * p_key );
    inline result_t hmi_key_process( kbdapi_key_t * p_key );

    inline void hmi_led_effect_set( uint8_t channel_con_id, uint8_t channel_adv_id, bool_t erase_status );
    inline void hmi_led_effect_on( void );
    inline void hmi_led_effect_off( void );
    inline void hmi_led_effect_update( uint8_t channel_con_id, uint8_t channel_adv_id, bool_t erase_status );
    inline void hmi_led_effect_adv( void );
    inline void hmi_led_effect_pairing( void );
    inline void hmi_led_effect_con( void );

    result_t kbdif_initialize(void);
    inline kbdapi_event_result_t kbdif_key_event_process( kbdapi_key_t * p_key );

//    void timer_save_conn_run(uint32_t timeout_ms);
//    void save_connection(void);
//
//    void timer_save_name_run(uint32_t timeout_ms);
//    void save_device_name(void);

//    void set_paired_channel_led(uint8_t channel, bool turnOn);

//    void set_channel_in_use( kbdapi_key_t * p_key_);
//    void erase_paired_device(uint8_t index_channel);
//    bool is_num_key(kbdapi_key_t * p_key);
//    char raw_key_to_ascii(kbdapi_key_t * p_key);
//    void update_channel_and_name(void);
//
//    void bt_layer_enter(void);
//    void bt_layer_exit(void);
//
//    /*
//     * Disables the advertising LED effect.
//     * This method disables the breathe effect.
//     */
//    void exit_pairing_mode(void);

    void cfgmem_ble_name_save( const ble_device_name_t * p_name_config, const ble_device_name_t * p_device_name );

    void cfgmem_channel_id_save( const channel_t * p_channel, uint8_t id );
    void cfgmem_channel_peer_id_save( const channel_t * p_channel, pm_peer_id_t peer_id );
    void cfgmem_channel_device_address_save( const channel_t * p_channel, const ble_device_addr_t * p_device_addr );
    void cfgmem_channel_device_name_save( const channel_t * p_channel, const ble_device_name_t * p_device_name );

    void cfgmem_device_name_local_save( const ble_device_name_t * p_device_name_local );
    void cfgmem_current_channel_id_save( uint8_t channel_id );
    void cfgmem_force_ble_save( bool_t force_ble );

    void cfgmem_channel_reset( const channel_t * p_channel, uint8_t id );
    void cfgmem_config_reset();

  private:
    static const kbdif_handlers_t kbdif_handlers;

    static kbdapi_event_result_t kbdif_key_event_cb( void * p_instance, kbdapi_key_t * p_key );
    static kbdapi_event_result_t kbdif_command_event_cb( void * p_instance, const char * p_command );

    static void ble_ll_event_cb( void * p_instance, blecdev_event_type_t event_type, blecdev_evt_param_t * p_param );
};

extern class BleManager BleManager;

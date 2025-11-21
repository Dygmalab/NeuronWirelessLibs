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
 * Author: Gustavo Gomez Lopez @Noteolvides
 * Mantainer: Juan Hauara @JuanHauara
 */

#pragma once

#include "Ble_composite_dev.h"
#include "kbd_if.h"

#define BLE_CONNECTIONS_COUNT   5
#define BLE_ADDRESS_LEN         6
#define BLE_DEVICE_NAME_LEN     32  // Same value as flag _BLE_DEVICE_NAME_LEN defined in the Ble_composite_dev.c file.

class Ble_connections
{
  public:

//    void set_peer_id(uint16_t id)
//    {
//        peer_id = id;
//    }
//
//    uint16_t get_peer_id(void)
//    {
//        return peer_id;
//    }
//
//    void set_device_addr(uint8_t *addr, uint8_t addr_len)
//    {
//        memcpy((void *)device_address, (const void *)addr, addr_len);
//    }
//
//    uint8_t *get_device_addr(void)
//    {
//        return device_address;
//    }
//
//    void set_device_name(uint8_t *name, uint8_t len)
//    {
//        memcpy(device_name, name, len);
//    }
//
//    uint8_t *get_device_name_ptr(void)
//    {
//        return device_name;
//    }
//
//    void reset(void)
//    {
//        peer_id = 0xFFFF;
//        memset(device_address, 0xFF, BLE_ADDRESS_LEN);
//        memset(device_name, 0x00, BLE_DEVICE_NAME_LEN);
//    }

  private:
    pm_peer_id_t peer_id = PM_PEER_ID_INVALID;  // 0xFFFF | libraries/SDK/nRF5_SDK_17.1.0_ddde560/components/ble/peer_manager/peer_manager_types.h;
    uint8_t device_address[BLE_ADDRESS_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t device_name[BLE_DEVICE_NAME_LEN] = {};
};

class Ble_flash_data
{
  public:
    Ble_connections ble_connections[BLE_CONNECTIONS_COUNT];
    char keyb_ble_name[BLE_DEVICE_NAME_LEN];
    uint8_t currentChannel;
    bool forceBle;

//    void reset(void)
//    {
//        for (auto &item : ble_connections)
//        {
//            item.reset();
//        }
//
//        currentChannel = 0;
//        forceBle = false;
//        strcpy(keyb_ble_name, BLE_DEVICE_NAME);
//    }
};

class BleManager
{
  public:
    typedef char ble_name_t[BLE_DEVICE_NAME_LEN];
    typedef uint8_t ble_address_t[BLE_ADDRESS_LEN];

    typedef struct PACK
    {
        pm_peer_id_t peer_id;
        ble_address_t device_address;
        ble_name_t device_name;
    } connection_t;

    typedef struct PACK
    {
        connection_t cons[BLE_CONNECTIONS_COUNT];
        ble_name_t keyb_ble_name;
        uint8_t current_channel;
        bool_t force_ble;
    } connections_config_t;

  public:
    bool trigger_save_name_timer = false;

    result_t init( void );

    void enable(void);
    bool getForceBle(void);
    void setForceBle(bool enabled);
    void set_bt_name_from_specifications(const char *spec);

    void set_pairing_key_press(bool press);
    bool get_pairing_key_press(void);

    void send_led_mode(void);

    void run();

  private:

    bool pairing_key_press = false;

    enum Channels: uint8_t
    {
        CHANNEL_0,
        CHANNEL_1,
        CHANNEL_2,
        CHANNEL_3,
        CHANNEL_4,
        NOT_CONNECTED,
        NOT_ON_ADVERTISING
    };

    // THIS ENUM IS NOT BEING USED
//    enum Pressed_keys: uint8_t
//    {
//        KEY_E = 8,
//        KEY_Q = 20,
//        KEY_R = 21,
//        KEY_T = 23,
//        KEY_W = 26,
//        KEY_1 = 30,
//        KEY_2 = 31,
//        KEY_3 = 32,
//        KEY_4 = 33,
//        KEY_5 = 34,
//    };

    struct ConnectionKeyState
    {
        bool longPress;
        uint64_t timePressed;
    };
    ConnectionKeyState connectionState[BLE_CONNECTIONS_COUNT];

    const connections_config_t * p_connections_config = nullptr;

    uint16_t flash_base_addr = 0;
    Ble_flash_data ble_flash_data;

    const char *ble_device_name = nullptr;
    uint8_t channels = 0;
    uint8_t channel_in_use = NOT_CONNECTED;
    bool showing_bt_layer = false;
    bool mitm_activated = false;
    char encryption_pin_number[6] = {};

    bool trigger_save_conn_timer = false;
    bool timer_save_conn_start_count = false;
    uint32_t ti_save_new_conn = 0;

    bool timer_save_name_start_count = false;
    uint32_t ti_save_new_name = 0;

    kbdif_t * p_kbdif = NULL;

    result_t kbdif_initialize(void);
    kbdapi_event_result_t kbdif_key_event_process( kbdapi_key_t * p_key );

    void timer_save_conn_run(uint32_t timeout_ms);
    void save_connection(void);

    void timer_save_name_run(uint32_t timeout_ms);
    void save_device_name(void);

    void set_paired_channel_led(uint8_t channel, bool turnOn);

    void set_channel_in_use( kbdapi_key_t * p_key_);
    void erase_paired_device(uint8_t index_channel);
    bool is_num_key(kbdapi_key_t * p_key);
    char raw_key_to_ascii(kbdapi_key_t * p_key);
    void update_channel_and_name(void);
    /*
     * Disables the advertising LED effect.
     * This method disables the breathe effect.
     */
    void exit_pairing_mode(void);

    void cfgmem_ble_name_save( const ble_name_t name_config, const char * p_name );

    void cfgmem_con_peer_id_save( const connection_t * p_connection, pm_peer_id_t peer_id );
    void cfgmem_con_device_address_save( const connection_t * p_connection, ble_address_t device_address );
    void cfgmem_con_device_name_save( const connection_t * p_connection, const char * p_device_name );

    void cfgmem_keyb_ble_name_save( const char * p_keyb_ble_name );
    void cfgmem_current_channel_save( uint8_t current_channel );
    void cfgmem_force_ble_save( bool_t force_ble );

    void cfgmem_connection_reset( const connection_t * p_connection );
    void cfgmem_connections_config_reset();

  private:
    static const kbdif_handlers_t kbdif_handlers;

    static kbdapi_event_result_t kbdif_key_event_cb( void * p_instance, kbdapi_key_t * p_key );
    static kbdapi_event_result_t kbdif_command_event_cb( void * p_instance, const char * p_command );
};

extern class BleManager BleManager;

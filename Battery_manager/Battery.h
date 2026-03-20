/* -*- mode: c++ -*-
 * Battery_manager -- Manage battery levels and status in wireless devices
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
 * Author: Alejandro Parcet, @alexpargon
 *
 */
#pragma once
#include <cstdint>

#include "kbd_if.h"
#include "Time_counter.h"

#define DISCONNECT_GRACE_TIMEOUT_MS     3000
#define LAST_STATUS_PACKET_TIMEOUT_MS   1500

class Battery {
   public:
    typedef struct PACK
    {
        uint8_t saving_mode;;
    } battery_conf_t;

   public:
    result_t init( void );
    void run( void );

   public:
    static uint8_t get_battery_status_left(void);
    static uint8_t get_battery_status_right(void);

   private:
    kbdif_t * p_kbdif = NULL;
    result_t kbdif_initialize(void);

   private:

    static const battery_conf_t * p_battery_conf;

    struct bat_status_side_t
    {
        uint8_t last_status_received;
        uint8_t status_confirm_count;

        // Debounce for DISCONNECTED (status 4)
        dl_timer_t disconnect_grace_started_timer;
        bool disconnect_pending;

        dl_timer_t last_status_packet_timer;
        bool status_requested;

        void disconnect_grace_started_timer_reset()
        {
            timer_set_ms( &disconnect_grace_started_timer, DISCONNECT_GRACE_TIMEOUT_MS );
        }

        bool disconnect_grace_started_timer_check()
        {
            return timer_check( &disconnect_grace_started_timer );
        }

        void last_status_packet_timer_reset()
        {
            timer_set_ms( &last_status_packet_timer, LAST_STATUS_PACKET_TIMEOUT_MS );
        }

        bool last_status_packet_timer_check()
        {
            return timer_check( &last_status_packet_timer );
        }

        void reset()
        {
            last_status_received = 4;
            status_confirm_count = 0;
            disconnect_pending = false;
        }

        void startDisconnect()
        {
            disconnect_pending = true;
            disconnect_grace_started_timer_reset();
        }

        void cancelDisconnect()
        {
            disconnect_pending = false;
        }

        void requestStatus()
        {
            status_requested = true;
        }

        void resetStatusRequested()
        {
            status_requested = false;
        }
    };

    static void set_battery_status_left(uint8_t status);
    static void set_battery_status_right(uint8_t status);

    static uint8_t battery_level;
    static uint8_t status_left;
    static uint8_t status_right;
    static uint8_t battery_level_left;
    static uint8_t battery_level_right;

    static bat_status_side_t right;
    static bat_status_side_t left; 

    static const kbdif_handlers_t kbdif_handlers;

    static kbdapi_event_result_t kbdif_key_event_cb( void * p_instance, kbdapi_key_t * p_key );
    static kbdapi_event_result_t kbdif_command_event_cb( void * p_instance, const char * p_command );

    static void cfgmem_saving_mode_config_save( uint8_t saving_mode );
};

extern class Battery Battery;

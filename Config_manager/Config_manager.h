/*
 * Config_manager -- Manage System configurable parameters in the non-volatile
 *                   memory
 * Copyright (C) 2025  Dygma Lab S.L.
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

#include "middleware.h"
#include "Time_counter.h"

#include "kbd_memory.h"
#include "LEDManager.h"

class ConfigManager
{
    public:

        typedef enum
        {
            CFG_ITEM_TYPE_DEVICE_SPEC = 1,

            CFG_ITEM_TYPE_LEDS_FADE_EFFECT = 10,
            CFG_ITEM_TYPE_LEDS_IDLELEDS,
            CFG_ITEM_TYPE_LEDS_BRIGHTNESS,

            CFG_ITEM_TYPE_BAT_SAVING_MODE = 20,

            CFG_ITEM_TYPE_BLE_CONNECTIONS = 30,

            CFG_ITEM_TYPE_RF = 40,
        } cfg_item_type_t;

        typedef result_t (* cfg_item_request_cb)( cfg_item_type_t item_type, const void ** pp_item );
        typedef result_t (* cfg_item_request_kbdmem_cb)( kbdmem_item_type_t item_type, const void ** pp_item );

        typedef struct
        {
            uint8_t * p_config_cache;
            uint16_t config_cache_size;

            /* Callbacks */
            cfg_item_request_cb item_request_cb;
            cfg_item_request_kbdmem_cb item_request_kbdmem_cb;
        } ConfigManager_config_t;

    public:

        void init( const ConfigManager_config_t * p_config );

        result_t config_item_request( cfg_item_type_t item_type, const void ** pp_item );
        result_t config_item_update( const void * p_config_item, const void * p_new_item, uint16_t item_size );

        void config_save_now( void );

        void run( void );

    private:
        uint8_t * p_cache;      /* Must come from within the RAM space */
        uint16_t cache_size;

        /* Callbacks */
        cfg_item_request_cb item_request_cb = nullptr;
        cfg_item_request_kbdmem_cb item_request_kbdmem_cb = nullptr;

        bool_t item_validity_check( const void * p_item_add, uint16_t item_size );

        void config_load( void );
        void config_save_request( void );
        void config_save( void );

        /********************************************/
        /*           Keyboard API memory            */
        /********************************************/

    private:

        void kbdmem_ll_init( void );
        INLINE result_t kbdmem_ll_item_request( kbdmem_item_type_t item_type, const void ** pp_item );
        INLINE result_t kbdmem_ll_data_save( const void * p_mem_target, const void * p_data, uint16_t data_len );

        static result_t kbdmem_ll_item_request_cb( void * p_instance, kbdmem_item_type_t item_type, const void ** pp_item );
        static result_t kbdmem_ll_data_save_cb( void * p_instance, const void * p_mem_target, const void * p_data, uint16_t data_len );

        /****************************************************/
        /*                     Machine                      */
        /****************************************************/

    private:

    #define CONFIG_SAVE_TIMEOUT_MS    200

        typedef enum
        {
            CONFIG_STATE_IDLE = 1,
            CONFIG_STATE_SAVE,
        } config_state_t;

        config_state_t machine_state = CONFIG_STATE_IDLE;
        dl_timer_t config_save_timer = 0;

        bool_t config_save_requested = false;

        INLINE void machine_state_set( config_state_t state );
        INLINE void machine_state_idle( void );
        INLINE void machine_state_save( void );
        INLINE void machine( void );
};

extern class ConfigManager ConfigManager;

/*
 *  EEPROM.cpp - nRF52833 EEPROM emulation
 *  Copyright (C) 2020  Dygma Lab S.L. All rights reserved.
 *
 *  Based on RP2040 EEPROM library, which is
 *  Copyright (c) 2021 Earle F. Philhower III. All rights reserved.
 *
 *  Based on ESP8266 EEPROM library, which is
 *  Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *  Mantainer: Gustavo Gomez Lopez @Noteolvides
 *  Mantainer: Juan Hauara @JuanHauara
 */

#pragma once

#include "dl_middleware.h"
#include "nrf_fstorage.h"

class EEPROMClass
{
    public:
        typedef enum
        {
            EEPROM_EVENT_TYPE_WRITE_FINISHED = 1,
            EEPROM_EVENT_TYPE_ERASE_FINISHED,
        } eeprom_event_type_t;

        typedef void (* eeprom_event_cb)( void * p_instance, eeprom_event_type_t event_type );

        typedef struct
        {
            /* Event callback */
            void * p_instance;
            eeprom_event_cb event_cb;
        } eeprom_config_t;

        result_t init( const eeprom_config_t * p_config );
        uint32_t align_get(void);

        result_t read( uint32_t addr_offset, uint8_t * p_data, size_t data_size );
        result_t write( uint32_t addr_offset, const uint8_t * p_data, size_t data_size );
        result_t erase(void);

    public:
        static void fstorage_evt_handler_nrf( nrf_fstorage_evt_t * p_evt );

    private:

        /* Addresses */
        uint32_t addr_offset_protected = 0;  /* Used to protect already written addresses to prevent multiple address writes */

        /* Flags */
        bool_t initialized_flag = false;
        bool_t eeprom_busy_flag;

        /* Event callback */
        void * p_instance;
        eeprom_event_cb event_cb;

        inline void event_handler( eeprom_event_type_t event_type );

        inline result_t fstorage_init();
        inline void fstorage_evt_handler( nrf_fstorage_evt_t * p_evt );
};

extern EEPROMClass EEPROM;

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

#define FLASH_STORAGE_NUM_PAGES                 4
#define FLASH_STORAGE_PAGE_SIZE                 4096    /* Size of the flash pages in Bytes. */

class EEPROMClass
{
    public:
        typedef enum
        {
            EEPROM_EVENT_TYPE_WRITE_FINISHED = 1,
            EEPROM_EVENT_TYPE_ERASE_FINISHED,
        } eeprom_event_type_t;

        typedef void (* eeprom_event_cb)( void * p_instance, eeprom_event_type_t event_type );

        typedef uint32_t eeprom_lock_t;

        result_t init( void );
        uint32_t align_get( void );

        result_t reserve( eeprom_lock_t * p_lock, void * p_instance, eeprom_event_cb event_cb );
        result_t release( eeprom_lock_t lock );

        result_t read( uint32_t addr_offset, uint8_t * p_data, size_t data_size );
        result_t write( eeprom_lock_t lock, uint32_t addr_offset, const uint8_t * p_data, size_t data_size );

        result_t erase_raw( eeprom_lock_t lock, uint32_t address, uint32_t page_cnt );
        result_t erase_offset( eeprom_lock_t lock, uint32_t addr_offset, uint32_t page_cnt );
        result_t erase_all( eeprom_lock_t lock );

        const void * data_ptr_get( uint32_t addr_offset );

    public:
        static void fstorage_evt_handler_nrf( nrf_fstorage_evt_t * p_evt );

    private:

        /* Reservation */
        eeprom_lock_t reserve_lock;

        /* Flags */
        bool_t initialized_flag = false;
        bool_t eeprom_busy_flag;
        bool_t is_reserved_flag;

        /* Event callback */
        void * p_instance;
        eeprom_event_cb event_cb;

        inline void event_handler( eeprom_event_type_t event_type );

        inline result_t fstorage_init();
        inline void fstorage_evt_handler( nrf_fstorage_evt_t * p_evt );
};

extern EEPROMClass EEPROM;

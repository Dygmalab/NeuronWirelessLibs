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

#include "middleware.h"

class EEPROMClass
{
    public:
        result_t init( void );

        result_t read( uint32_t addr_offset, uint8_t * p_data, size_t data_size );
        result_t write( uint32_t addr_offset, const uint8_t * p_data, size_t data_size );
        result_t erase(void);

    private:
        bool_t initialized = false;
        uint32_t addr_offset_protected = 0;  /* Used to protect already written addresses to prevent multiple address writes */
};

extern EEPROMClass EEPROM;

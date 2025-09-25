/* -*- mode: c++ -*-
 * kaleidoscope::plugin::Battery_manager -- Manage battery levels and status in wireless devices
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
#include "kaleidoscope/Runtime.h"
#include <Kaleidoscope-EEPROM-Settings.h>
#include <Kaleidoscope-Ranges.h>
#include <Arduino.h>

#include "kbd_if.h"

namespace kaleidoscope {
class Battery {
   public:
    result_t init( void );

   public:
    static uint8_t get_battery_status_left(void);
    static uint8_t get_battery_status_right(void);

   private:
    kbdif_t * p_kbdif = NULL;
    result_t kbdif_initialize(void);

   private:
    static uint8_t battery_level;
    static uint8_t saving_mode;
    static uint16_t settings_saving_;
    static uint8_t status_left;
    static uint8_t status_right;
    static uint8_t battery_level_left;
    static uint8_t battery_level_right;

    static const kbdif_handlers_t kbdif_handlers;

    static kbdapi_event_result_t kbdif_key_event_cb( void * p_instance, kbdapi_key_t * p_key );
    static kbdapi_event_result_t kbdif_command_event_cb( void * p_instance, const char * p_command );
};
}  // namespace kaleidoscope

extern kaleidoscope::Battery Battery;

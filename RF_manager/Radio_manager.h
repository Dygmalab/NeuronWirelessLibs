/* -*- mode: c++ -*-
 * kaleidoscope::plugin::RadioManager -- Manage RF Signal and status in wireless devices
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
class RadioManager {
  public:

    result_t init();
    void enable();
    void poll();

    bool isEnabled();
    bool isInited();

  private:
    kbdif_t * p_kbdif = NULL;
    result_t kbdif_initialize(void);

  private:

    enum Power :uint8_t {
         LOW_P,
         MEDIUM_P,
         HIGH_P,
     };
    static bool inited;
    static uint16_t channel_hop;
    static Power power_rf;
    static uint16_t settings_base_;
    static void setPowerRF();

    static const kbdif_handlers_t kbdif_handlers;

    static kbdapi_event_result_t kbdif_command_event_cb( void * p_instance, const char * p_command );
};

}  // namespace kaleidoscope

extern kaleidoscope::RadioManager _RadioManager;

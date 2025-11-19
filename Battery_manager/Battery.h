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
#include "kaleidoscope/plugin.h"
#include "kaleidoscope/Runtime.h"
#include <Kaleidoscope-EEPROM-Settings.h>
#include <Kaleidoscope-Ranges.h>
#include <Arduino.h>
#include <cstdint>

namespace kaleidoscope {
namespace plugin {
class Battery : public Plugin {
   public:
    EventHandlerResult onSetup();
    EventHandlerResult onFocusEvent(const char *command);
    EventHandlerResult onKeyswitchEvent(Key &mapped_key, KeyAddr key_addr, uint8_t key_state);
    EventHandlerResult beforeReportingState();

    static uint8_t get_battery_status_left(void);
    static uint8_t get_battery_status_right(void);

   private:

    struct bat_status_side_t
    {
        uint8_t last_status_received;
        uint8_t status_confirm_count;

        // Debounce for DISCONNECTED (status 4)
        uint32_t disconnect_grace_started_ms;
        bool disconnect_pending;

        uint32_t last_status_packet_ms;
        bool status_requested;

        void reset()
        {
            last_status_received = 4;
            status_confirm_count = 0;
            disconnect_pending = false;
            disconnect_grace_started_ms = 0;
            last_status_packet_ms = 0;
        }

        void startDisconnect(uint32_t now)
        {
            disconnect_pending = true;
            disconnect_grace_started_ms = now;
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
    static uint8_t saving_mode;
    static uint16_t settings_saving_;
    static uint8_t status_left;
    static uint8_t status_right;
    static uint8_t battery_level_left;
    static uint8_t battery_level_right;

    static bat_status_side_t right;
    static bat_status_side_t left; 
};

}  // namespace plugin
}  // namespace kaleidoscope

extern kaleidoscope::plugin::Battery Battery;
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

#include "Battery.h"
#include "Colormap-Defy.h"
#include "Communications.h"
#include "Kaleidoscope-FocusSerial.h"
#include "kaleidoscope/key_events.h"
#include "kaleidoscope/plugin/LEDControlDefy.h"
#include "FirmwareVersion.h"


#define NOT_CHARGING 0


namespace kaleidoscope
{
namespace plugin
{

#define EQUAL_STATE_COUNT 4
#define DISCONNECT_GRACE_MS 3000
#define DEBUG_LOG_BATTERY_MANAGER   0


uint8_t Battery::battery_level;
uint8_t Battery::saving_mode;
uint16_t Battery::settings_saving_;
uint8_t Battery::status_left = 4;
uint8_t Battery::status_right = 4;
uint8_t Battery::battery_level_left = 100;
uint8_t Battery::battery_level_right = 100;
Battery::bat_status_side_t Battery::right = {4 , 0,0,false,0,false};
Battery::bat_status_side_t Battery::left = {4 , 0,0,false,0,false}; 


EventHandlerResult Battery::onKeyswitchEvent(Key &mappedKey, KeyAddr key_addr, uint8_t keyState)
{
    if (mappedKey.getRaw() != ranges::BATTERY_LEVEL )
    {
        return EventHandlerResult::OK;
    }

    if (keyToggledOn(keyState) &&  FirmwareVersion::keyboard_is_wireless())
    {
        ::LEDControl.set_mode(9);
        ::LEDControl.set_force_mode(true);
        ColormapEffectDefy::updateBrigthness(ColormapEffectDefy::LedBrightnessControlEffect::BATTERY_STATUS, true);
    }

    if (keyToggledOff(keyState))
    {
        ColormapEffectDefy::updateBrigthness(ColormapEffectDefy::LedBrightnessControlEffect::BATTERY_STATUS, false);
        ::LEDControl.set_force_mode(false);
        ::LEDControl.set_mode(0);
    }

    return EventHandlerResult::EVENT_CONSUMED;
}

EventHandlerResult Battery::onFocusEvent(const char *command)
{
    if (::Focus.handleHelp(command,
        "wireless.battery.left.level\nwireless.battery.right.level\nwireless.battery.left.status\nwireless.battery.right.status\nwireless.battery.savingMode"))
    {
        return EventHandlerResult::OK;
    }

    if (strncmp(command, "wireless.battery.", 17) != 0)
    {
        return EventHandlerResult::OK;
    }

    if (strcmp(command + 17, "right.level") == 0)
    {
        if (::Focus.isEOL())
        {
#if DEBUG_LOG_BATTERY_MANAGER
            NRF_LOG_DEBUG("read request: wireless.battery.right.level");
#endif
            ::Focus.send(battery_level_right);
        }
    }

    if (strcmp(command + 17, "left.level") == 0)
    {
        if (::Focus.isEOL())
        {
#if DEBUG_LOG_BATTERY_MANAGER
            NRF_LOG_DEBUG("read request: wireless.battery.left.level");
#endif
            ::Focus.send(battery_level_left);
        }
    }

    if (strcmp(command + 17, "right.status") == 0)
    {
        if (::Focus.isEOL())
        {
            right.requestStatus();
            right.last_status_packet_ms = Runtime.millisAtCycleStart();

#if DEBUG_LOG_BATTERY_MANAGER
            NRF_LOG_DEBUG("read request: wireless.battery.right.status");
#endif
            ::Focus.send(status_right);
        }
    }

    if (strcmp(command + 17, "left.status") == 0)
    {
        if (::Focus.isEOL())
        {

            left.requestStatus();
            left.last_status_packet_ms = Runtime.millisAtCycleStart();

#if DEBUG_LOG_BATTERY_MANAGER
            NRF_LOG_DEBUG("read request: wireless.battery.left.status");
#endif
            ::Focus.send(status_left);
        }
    }

    if (strcmp(command + 17, "savingMode") == 0)
    {
        if (::Focus.isEOL())
        {
#if DEBUG_LOG_BATTERY_MANAGER
            NRF_LOG_DEBUG("read request: wireless.battery.savingMode");
#endif
            ::Focus.send(saving_mode);
        }
        else
        {
            ::Focus.read(saving_mode);
#if DEBUG_LOG_BATTERY_MANAGER
            NRF_LOG_DEBUG("write request: wireless.battery.savingMode");
#endif
            Communications_protocol::Packet p{};
            p.header.command = Communications_protocol::BATTERY_SAVING;
            p.header.size = 1;
            p.data[0] = saving_mode;
            Communications.sendPacket(p);
            Runtime.storage().put(settings_saving_, saving_mode);
            Runtime.storage().commit();
        }
    }

    return EventHandlerResult::EVENT_CONSUMED;
}

EventHandlerResult Battery::beforeReportingState()
{
    // Confirm pending DISCONNECTED only after grace period
    uint32_t now = Runtime.millisAtCycleStart();

    if (left.disconnect_pending && (now - left.disconnect_grace_started_ms >= DISCONNECT_GRACE_MS))
    {
        status_left = 4;
        left.cancelDisconnect();
#if DEBUG_LOG_BATTERY_MANAGER
        NRF_LOG_DEBUG("Battery LEFT: disconnect grace elapsed -> status 4");
#endif
    }

    if (right.disconnect_pending && (now - right.disconnect_grace_started_ms >= DISCONNECT_GRACE_MS))
    {
        status_right = 4;
        right.cancelDisconnect();
#if DEBUG_LOG_BATTERY_MANAGER
        NRF_LOG_DEBUG("Battery RIGHT: disconnect grace elapsed -> status 4");
#endif
    }

    if(left.status_requested && (now - left.last_status_packet_ms >= 1500))
    {
        left.last_status_packet_ms = Runtime.millisAtCycleStart();

        /* We will send the BATTERY_STATUS command to both sides. 
        * So we don't need to check if the right side has requested a status.
        */
        Communications_protocol::Packet p{};
        p.header.command = Communications_protocol::BATTERY_STATUS;
        Communications.sendPacket(p);

        left.resetStatusRequested();
    }

    return EventHandlerResult::OK;
}

bool inline filterHand(Communications_protocol::Devices incomingDevice, bool right_or_left)
{
    if (right_or_left == 1)
    {
        return incomingDevice == Communications_protocol::KEYSCANNER_DEFY_RIGHT || incomingDevice == Communications_protocol::BLE_DEFY_RIGHT ||
               incomingDevice == Communications_protocol::RF_DEFY_RIGHT;
    }
    else
    {
        return incomingDevice == Communications_protocol::KEYSCANNER_DEFY_LEFT || incomingDevice == Communications_protocol::BLE_DEFY_LEFT ||
               incomingDevice == Communications_protocol::RF_DEFY_LEFT;
    }
}

EventHandlerResult Battery::onSetup()
{
    // Save saving_mode variable in EEPROM
    settings_saving_ = ::EEPROMSettings.requestSlice(sizeof(saving_mode));
    uint8_t saving;
    Runtime.storage().get(settings_saving_, saving);
    if (saving == 0xFF)
    { // If is the first time we read from memory
        saving_mode = 0;
        Runtime.storage().put(settings_saving_, saving_mode); // Save default value 0.
        Runtime.storage().commit();
    }
    Runtime.storage().get(settings_saving_, saving_mode); // safe get

    Communications.callbacks.bind(BATTERY_STATUS, ([this](Packet const &packet) {
        if (filterHand(packet.header.device, false))
        {
            // Any valid status from LEFT cancels a pending disconnect
            if (left.disconnect_pending) { left.cancelDisconnect(); }
            set_battery_status_left( packet.data[0]);
            // Mark fresh status arrival for LEFT
            left.last_status_packet_ms = Runtime.millisAtCycleStart();
            if (left.status_requested) { left.resetStatusRequested(); }
        }

        if (filterHand(packet.header.device, true))
        {
            // Any valid status from RIGHT cancels a pending disconnect
            if (right.disconnect_pending) { right.cancelDisconnect(); }
            set_battery_status_right(packet.data[0]);
            // Mark fresh status arrival for RIGHT
            right.last_status_packet_ms = Runtime.millisAtCycleStart();
            if (right.status_requested) { right.resetStatusRequested(); }
        }

        #if DEBUG_LOG_BATTERY_MANAGER
         NRF_LOG_DEBUG("Battery Status device %i %i", packet.header.device, packet.data[0]);
        #endif
    }));

    Communications.callbacks.bind(BATTERY_LEVEL, ([this](Packet const &packet) {
        if (filterHand(packet.header.device, false))
        {
            battery_level_left = packet.data[0];
            // Activity from LEFT side implies it's not disconnected anymore
            if (left.disconnect_pending) { left.cancelDisconnect(); }
        }

        if (filterHand(packet.header.device, true))
        {
            battery_level_right = packet.data[0];
            // Activity from RIGHT side implies it's not disconnected anymore
            if (right.disconnect_pending) { right.cancelDisconnect(); }
        }

        uint16_t battery_level_mv;
        memcpy(&battery_level_mv, &packet.data[1], sizeof(battery_level_mv));
        ble_battery_level_update(min(battery_level_left, battery_level_right));

        #if DEBUG_LOG_BATTERY_MANAGER
        NRF_LOG_DEBUG("Battery level: %i device %i percentage %i mv",
        packet.header.device,
        packet.data[0],
        battery_level_mv);
        #endif
    }));

    Communications.callbacks.bind(DISCONNECTED, ([this](Packet const &packet) 
    {
        
#if DEBUG_LOG_BATTERY_MANAGER
        NRF_LOG_DEBUG("DISCONNECTED RECEIVED, starting grace window");
#endif

        if (filterHand(packet.header.device, false))
        {
            battery_level_left = 100;
            left.reset();
            left.startDisconnect(Runtime.millisAtCycleStart());
        }

        if (filterHand(packet.header.device, true))
        {
            battery_level_right = 100;
            right.reset();
            right.startDisconnect(Runtime.millisAtCycleStart());
        }

        ble_battery_level_update(min(battery_level_left, battery_level_right));
    }));

    Communications.callbacks.bind(CONNECTED, ([this](Packet packet) {
        packet.header.command = BATTERY_SAVING;
        packet.header.size = 1;
        packet.data[0] = saving_mode;
        Communications.sendPacket(packet);
    }));

    return EventHandlerResult::OK;
}

uint8_t Battery::get_battery_status_left(void)
{
    /*
        0 -> Side connected and powered from its battery or the other side's battery.
        1 o 2 -> Side connected and powered from the N2 while it is connected to the PC via USB.
        4 -> Side disconnected.
    */

    return status_left;
}

uint8_t Battery::get_battery_status_right(void)
{
    /*
        0 -> Side connected and powered from its battery or the other side's battery.
        1 o 2 -> Side connected and powered from the N2 while it is connected to the PC via USB.
        4 -> Side disconnected.
    */

    return status_right;
}

void Battery::set_battery_status_left(uint8_t battery_status)
{
    // Implement confirmation algorithm: only change battery status after receiving same value twice
    if (battery_status == static_cast<uint8_t>(left.last_status_received)) 
    {
        left.status_confirm_count++;
        if ( left.status_confirm_count >= EQUAL_STATE_COUNT)
        {
          status_left = battery_status;
          left.status_confirm_count = 0; // Reset counter after successful update
        }
    } 
    else 
    {
    // Different status received, reset tracking
    left.last_status_received = battery_status;
    left.status_confirm_count = 0; // Start counting for new status
    }
}

void Battery::set_battery_status_right(uint8_t battery_status)
{
    // Implement confirmation algorithm: only change battery status after receiving same value twice
    if (battery_status == static_cast<uint8_t>(right.last_status_received)) 
    {
        right.status_confirm_count++;
        if (right.status_confirm_count >= EQUAL_STATE_COUNT)
        {
          status_right = battery_status;
          right.status_confirm_count = 0; // Reset counter after successful update
        }
      } 
      else 
      {
        // Different status received, reset tracking
        right.last_status_received = battery_status;
        right.status_confirm_count = 1; // Start counting for new status
      }
}

} // namespace plugin
} //  namespace kaleidoscope

kaleidoscope::plugin::Battery Battery;
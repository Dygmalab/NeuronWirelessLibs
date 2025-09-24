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

#include "kbd_if_manager.h"


#define NOT_CHARGING 0


namespace kaleidoscope
{
namespace plugin
{


#define DEBUG_LOG_BATTERY_MANAGER   0


uint8_t Battery::battery_level;
uint8_t Battery::saving_mode;
uint16_t Battery::settings_saving_;
uint8_t Battery::status_left = 4;
uint8_t Battery::status_right = 4;
uint8_t Battery::battery_level_left = 100;
uint8_t Battery::battery_level_right = 100;

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
            Communications_protocol::Packet p{};
            p.header.command = Communications_protocol::BATTERY_STATUS;
            Communications.sendPacket(p);

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

result_t Battery::kbdif_initialize()
{
    result_t result = RESULT_ERR;
    kbdif_conf_t config;

    /* Prepare the kbdif configuration */
    config.p_instance = this;
    config.handlers = &kbdif_handlers;

    /* Initialize the kbdif */
    result = kbdif_init( &p_kbdif, &config );
    EXIT_IF_ERR( result, "kbdif_init failed" );

    /* Add the kbdif into the kbdif manager */
    result = kbdifmgr_add( p_kbdif );
    EXIT_IF_ERR( result, "kbdifmgr_add failed" );

_EXIT:
    return result;
}

result_t Battery::init()
{
    result_t result = RESULT_ERR;

    result = kbdif_initialize();
    EXIT_IF_ERR( result, "kbdif_initialize failed" );

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
            status_left = packet.data[0];
        }

        if (filterHand(packet.header.device, true))
        {
            status_right = packet.data[0];
        }

        #if DEBUG_LOG_BATTERY_MANAGER
        NRF_LOG_DEBUG("Battery Status device %i %i", packet.header.device, packet.data[0]);
        #endif
    }));

    Communications.callbacks.bind(BATTERY_LEVEL, ([this](Packet const &packet) {
        if (filterHand(packet.header.device, false))
        {
            battery_level_left = packet.data[0];
        }

        if (filterHand(packet.header.device, true))
        {
            battery_level_right = packet.data[0];
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

    Communications.callbacks.bind(DISCONNECTED, ([this](Packet const &packet) {
        if (filterHand(packet.header.device, false))
        {
            battery_level_left = 100;
            status_left = 4;
        }

        if (filterHand(packet.header.device, true))
        {
            battery_level_right = 100;
            status_right = 4;
        }

        ble_battery_level_update(min(battery_level_left, battery_level_right));
    }));

    Communications.callbacks.bind(CONNECTED, ([this](Packet packet) {
        packet.header.command = BATTERY_SAVING;
        packet.header.size = 1;
        packet.data[0] = saving_mode;
        Communications.sendPacket(packet);
    }));

_EXIT:
    return result;
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

kbdapi_event_result_t Battery::kbdif_key_event_cb( void * p_instance, kbdapi_key_t * p_key )
{
    if ( p_key->type != KBDAPI_KEY_TYPE_BATTERY_LEVEL )
    {
        return KBDAPI_EVENT_RESULT_IGNORED;
    }

    if ( p_key->toggled_on == true &&  FirmwareVersion::keyboard_is_wireless() )
    {
        ::LEDControl.set_mode(9);
        ::LEDControl.set_force_mode(true);
        ColormapEffectDefy::updateBrigthness(ColormapEffectDefy::LedBrightnessControlEffect::BATTERY_STATUS, true);
    }

    if ( p_key->toggled_off == true )
    {
        ColormapEffectDefy::updateBrigthness(ColormapEffectDefy::LedBrightnessControlEffect::BATTERY_STATUS, false);
        ::LEDControl.set_force_mode(false);
        ::LEDControl.set_mode(0);
    }

    return KBDAPI_EVENT_RESULT_CONSUMED;
}

const kbdif_handlers_t Battery::kbdif_handlers =
{
    .key_event_cb = kbdif_key_event_cb,
};

} // namespace plugin
} //  namespace kaleidoscope

kaleidoscope::plugin::Battery Battery;

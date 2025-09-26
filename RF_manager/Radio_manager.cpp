/* -*- mode: c++ -*-
 * RadioManager -- Manage RF Signal and status in wireless devices
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

#include "Radio_manager.h"
#include "Adafruit_USBD_Device.h"
#include "CRC_wrapper.h"
#include "Communications.h"
#include "Kaleidoscope-FocusSerial.h"
#include "nrf_gpio.h"
#include "rf_host_device_api.h"

#include "kbd_if_manager.h"

namespace kaleidoscope
{

uint16_t RadioManager::settings_base_ = 0;
bool RadioManager::inited = false;
RadioManager::Power RadioManager::power_rf = LOW_P;
uint16_t RadioManager::channel_hop;

result_t RadioManager::kbdif_initialize()
{
    result_t result = RESULT_ERR;
    kbdif_conf_t config;

    /* Prepare the kbdif configuration */
    config.p_instance = NULL;       /* The module is whole static */
    config.handlers = &kbdif_handlers;

    /* Initialize the kbdif */
    result = kbdif_init( &p_kbdif, &config );
    EXIT_IF_ERR( result, "kbdif_init failed" );

    /* Add the kbdif into the kbdif manager */
    result = kbdifmgr_add( RadioManager::p_kbdif );
    EXIT_IF_ERR( result, "kbdifmgr_add failed" );

_EXIT:
    return result;
}

result_t RadioManager::init()
{
    result_t result = RESULT_ERR;

    result = kbdif_initialize();
    EXIT_IF_ERR( result, "kbdif_initialize failed" );

    settings_base_ = kaleidoscope::plugin::EEPROMSettings::requestSlice(sizeof(power_rf));
    Runtime.storage().get(settings_base_, power_rf);
    if (power_rf == 0xFF)
    {
        power_rf = LOW_P;
        Runtime.storage().put(settings_base_, power_rf);
        Runtime.storage().commit();
    }

_EXIT:
    return result;
}

void RadioManager::enable()
{
    NRF_LOG_INFO("Working with RF");
    inited = true;
    rfgw_host_init();
    rfgw_addr_set(rfgw_addr_suggest());
    setPowerRF();
    rfgw_enable();
    // Open the Keyscanner pipes.
    rfgw_pipe_open(RFGW_PIPE_ID_KEYSCANNER_LEFT);
    rfgw_pipe_open(RFGW_PIPE_ID_KEYSCANNER_RIGHT);
}

void RadioManager::setPowerRF()
{
    if (!inited) return;
    switch (power_rf)
    {
        case LOW_P:
            rfgw_tx_power_set(RFGW_TX_POWER_0_DBM);
            break;
        case MEDIUM_P:
            rfgw_tx_power_set(RFGW_TX_POWER_4_DBM);
            break;
        case HIGH_P:
            rfgw_tx_power_set(RFGW_TX_POWER_8_DBM);
            break;
    }
}

bool RadioManager::isInited()
{
    return inited;
}

void RadioManager::poll()
{
    return rfgw_poll();
}

kbdapi_event_result_t RadioManager::kbdif_command_event_cb( void * p_instance, const char * p_command )
{
    if (::Focus.handleHelp(p_command, "wireless.rf.power\nwireless.rf.channelHop\nwireless.rf.syncPairing"))
    {
        return KBDAPI_EVENT_RESULT_IGNORED;
    }

    if (strncmp(p_command, "wireless.rf.", 12) != 0)
    {
        return KBDAPI_EVENT_RESULT_IGNORED;
    }

    // Will be apply
    if (strcmp(p_command + 12, "power") == 0)
    {
        if (::Focus.isEOL())
        {
            NRF_LOG_DEBUG("read request: wireless.rf.power");
            ::Focus.send<uint8_t>((uint8_t)power_rf);
        }
        else
        {
            uint8_t power;
            ::Focus.read(power);
            if (power <= HIGH_P)
            {
                power_rf = (RadioManager::Power)power;
                setPowerRF();
                Runtime.storage().put(settings_base_, power_rf);
                Runtime.storage().commit();
            }
        }
    }

    if (strcmp(p_command + 12, "channelHop") == 0)
    {
        if (::Focus.isEOL())
        {
            NRF_LOG_DEBUG("read request: wireless.rf.channelHop");
            ::Focus.send(channel_hop); // TODO: get set channelHop mode configuration status
        }
        else
        {
            NRF_LOG_DEBUG("write request: wireless.rf.channelHop");
            ::Focus.read(channel_hop);
        }
    }

    if (strcmp(p_command + 12, "syncPairing") == 0)
    {
        if (::Focus.isEOL())
        {
            NRF_LOG_DEBUG("sync pairing procedure: wireless.rf.syncPairing");
            Communications_protocol::Packet packet{};
            packet.header.command = Communications_protocol::RF_ADDRESS;
            uint32_t rf_address = rfgw_addr_suggest();
            packet.header.size = sizeof(rf_address);
            memcpy(packet.data, &rf_address, packet.header.size);
            Communications.sendPacket(packet);
        }
    }

    return KBDAPI_EVENT_RESULT_CONSUMED;
}

const kbdif_handlers_t RadioManager::kbdif_handlers =
{
    .key_event_cb = NULL,
    .command_event_cb = kbdif_command_event_cb,
};

} //  namespace kaleidoscope

kaleidoscope::RadioManager _RadioManager;

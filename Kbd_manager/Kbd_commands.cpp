/* -*- mode: c++ -*-
 * Kbd_commands -- Handling the commands coming from the host
 * Copyright (C) 2026  Dygma Lab S.L.
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

#include "Kaleidoscope-FocusSerial.h"
#include "Kbd_commands.h"
#include "kbd_if_manager.h"
#include "Kbd_manager.h"

#define DEBUG_LOG_KBD_COMMANDS   0

#if DEBUG_LOG_KBD_COMMANDS == 1
    #define KBD_LOG_DEBUG(...)  NRF_LOG_DEBUG(__VA_ARGS__)
#else /* DEBUG_LOG_KBD_COMMANDS */
    #define KBD_LOG_DEBUG(...)
#endif /* DEBUG_LOG_KBD_COMMANDS */

result_t KbdCommands::init( void )
{
    result_t result = RESULT_ERR;

    result = kbdif_initialize();
    EXIT_IF_ERR( result, "kbdif_initialize failed" );

_EXIT:
    return result;
}

result_t KbdCommands::kbdif_initialize()
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

kbdapi_event_result_t KbdCommands::kbdif_command_event_cb( void * p_instance, const char * p_command )
{
    if (::Focus.handleHelp(p_command,
                           "hardware.version\n"
                           "hardware.side_power\n"
                           "hardware.side_ver\n"
                           "hardware.keyscanInterval\n"
                           "hardware.firmware\n"
                           "hardware.chip_id\n"
                           "hardware.chip_info"))
        return KBDAPI_EVENT_RESULT_IGNORED;

    if (strncmp(p_command, "hardware.", 9) != 0) {
        return KBDAPI_EVENT_RESULT_IGNORED;
    }

    if (strcmp(p_command + 9, "sideLeft") == 0) {
        auto deviceLeft = kbdManager.leftHandDevice();
        ::Focus.send(static_cast<uint8_t>(deviceLeft));
        return KBDAPI_EVENT_RESULT_IGNORED;
    }

    if (strcmp(p_command + 9, "sideRight") == 0) {
        auto deviceRight = kbdManager.rightHandDevice();
        ::Focus.send(static_cast<uint8_t>(deviceRight));
        return KBDAPI_EVENT_RESULT_IGNORED;
    }

    if (strcmp(p_command + 9, "version") == 0) {
        KBD_LOG_DEBUG("read request: hardware.version");

        ::Focus.send<char *>(HARDWARE_VERSION_NAME);

        return KBDAPI_EVENT_RESULT_CONSUMED;
    }

    if (strcmp(p_command + 9, "firmware") == 0) {
        KBD_LOG_DEBUG("read request: hardware.firmware");

        ::Focus.send<char *>(KEYBOARD_NEURON_FW_VERSION);

        return KBDAPI_EVENT_RESULT_CONSUMED;
    }

    if (strcmp(p_command + 9, "chip_id") == 0) {
        KBD_LOG_DEBUG("read request: hardware.chip_id");

        char chip_id[17];
        kbdManager.getChipID(chip_id, sizeof(chip_id));
        ::Focus.send<char *>(chip_id);

        return KBDAPI_EVENT_RESULT_CONSUMED;
    }

    if (strcmp(p_command + 9, "chip_info") == 0) {
        KBD_LOG_DEBUG("read request: hardware.chip_info");

        char chip_info[100];
        kbdManager.get_chip_info(chip_info, sizeof(chip_info));
        ::Focus.send<char *>(chip_info);

        return KBDAPI_EVENT_RESULT_CONSUMED;
    }

    if (strcmp(p_command + 9, "side_power") == 0) {
        if (::Focus.isEOL()) {
            KBD_LOG_DEBUG("read request: hardware.side_power");

            ::Focus.send(kbdManager.getSidePower());

            return KBDAPI_EVENT_RESULT_CONSUMED;
        } else {
            KBD_LOG_DEBUG("write request: hardware.side_power");

            uint8_t power;
            ::Focus.read(power);
            kbdManager.setSidePower(power);

            return KBDAPI_EVENT_RESULT_CONSUMED;
        }
    }

    if (strcmp(p_command + 9, "side_ver") == 0) {
        KBD_LOG_DEBUG("read request: hardware.side_ver");
        ::Focus.send(kbdManager.leftVersion());
        ::Focus.send(kbdManager.rightVersion());
        return KBDAPI_EVENT_RESULT_CONSUMED;
    }

//    if (strcmp(p_command + 9, "keyscanInterval") == 0) {
//        if (::Focus.isEOL()) {
//            KBD_LOG_DEBUG("read request: hardware.keyscanInterval");
//
//            ::Focus.send(Runtime.device().settings.keyscanInterval());
//
//            return KBDAPI_EVENT_RESULT_CONSUMED;
//        } else {
//            KBD_LOG_DEBUG("write request: hardware.keyscanInterval");
//
//            uint8_t keyscan;
//            ::Focus.read(keyscan);
//            Runtime.device().settings.keyscanInterval(keyscan);
//
//            return KBDAPI_EVENT_RESULT_CONSUMED;
//        }
//    }

    return KBDAPI_EVENT_RESULT_IGNORED;
}

const kbdif_handlers_t KbdCommands::kbdif_handlers =
{
    .key_event_cb = NULL,
    .command_event_cb = kbdif_command_event_cb,
};

class KbdCommands kbdCommands;

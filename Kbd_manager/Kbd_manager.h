/* -*- mode: c++ -*-
 * Kbd_manager -- Manage keyboard general functionality, status and base communications
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

#pragma once

#include "dl_middleware.h"

#include "Communications.h"

class KbdManager
{
    public:

        result_t init( void );

        Communications_protocol::Devices leftHandDevice( void );
        Communications_protocol::Devices rightHandDevice( void );

        void reset_sides( void );
        void reset_right_side( void );
        void reset_left_side( void );

        void getChipID(char *cstring, uint16_t len);
        void get_chip_info(char *cstring, uint16_t len);

        void setSidePower( bool power );
        bool getSidePower( void );

        uint8_t leftVersion( void );
        uint8_t rightVersion( void );

        bool slideSwitchPositionUsb( void );
        bool slideSwitchPositionBle( void );

        void prepareForFlash( void );

        bool rightSideWiredConnection( void );
        bool leftSideWiredConnection( void );

        uint8_t boot_address_left_get( void );
        uint8_t boot_address_right_get( void );

    private:

        // BLE       WIRED       RF
        Communications_protocol::Devices leftConnection[3]{Communications_protocol::Devices::UNKNOWN, Communications_protocol::Devices::UNKNOWN, Communications_protocol::Devices::UNKNOWN};
        Communications_protocol::Devices rightConnection[3]{Communications_protocol::Devices::UNKNOWN, Communications_protocol::Devices::UNKNOWN, Communications_protocol::Devices::UNKNOWN};

        bool side_power = false;

        inline void msg_connected_process( const Packet &p );
        inline void msg_disconnected_process( const Packet &p );
        inline void msg_has_keys_process( const Packet &p );
};

extern class KbdManager kbdManager;

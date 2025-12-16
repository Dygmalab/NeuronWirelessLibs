/*
 * kaleidoscope::device::dygma::Wired -- Kaleidoscope device plugin for Dygma Wired
 *
 * Copyright (C) 2020-2023  Dygma Lab S.L.
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
 *  Mantainer: Gustavo Gomez Lopez @Noteolvides
 *  Mantainer: Juan Hauara @JuanHauara
 */

#ifndef SPICOMUNICATIONS_H_
#define SPICOMUNICATIONS_H_


#include "common.h"
#include "Spi_slave.h"
#include "Communications_protocol.h"

using namespace Communications_protocol;

// clang-format on

class SpiPort 
{
    public:
        SpiPort(uint8_t _spi_port_used);

        Spi_slave *spi_slave = nullptr;

        void init(void);
//        void deInit(void);
        void run(void);

        bool is_connected();

        bool readPacket(Packet &packet);    /* Function will provide current packet and discard it from the queue*/
        bool peekPacket(Packet &packet);    /* Function will provide current packet but keeps it in the queue */

        bool sendPacket(Packet &packet);

        void clearSend();
        void clearRead();


       private:
        uint8_t spi_port_used;
};


#endif //SPICOMUNICATIONS_H_

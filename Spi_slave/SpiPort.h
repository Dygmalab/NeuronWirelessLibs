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

// clang-format off

// SPI0
#if COMPILE_SPI0_SUPPORT
    #define PIN_MISO0         NRF_GPIO_PIN_MAP(, )
    #define PIN_MOSI0         NRF_GPIO_PIN_MAP(, )
    #define PIN_CLK0          NRF_GPIO_PIN_MAP(, )
    #define PIN_CS0           NRF_GPIO_PIN_MAP(, )
#endif

// SPI1
#if COMPILE_SPI1_SUPPORT
#if COMPILE_FOR_NEURON_2_HARDWARE_V1_0
        #define PIN_MISO1   NRF_GPIO_PIN_MAP(0, 0)
        #define PIN_MOSI1   NRF_GPIO_PIN_MAP(0, 20)
        #define PIN_CLK1    NRF_GPIO_PIN_MAP(0, 9)
        #define PIN_CS1     NRF_GPIO_PIN_MAP(0, 10)
#elif COMPILE_FOR_NEURON_2_HARDWARE_V1_1
        #define PIN_MISO1   NRF_GPIO_PIN_MAP(0, 0)
        #define PIN_MOSI1   NRF_GPIO_PIN_MAP(0, 20)
        #define PIN_CLK1    NRF_GPIO_PIN_MAP(0, 17)
        #define PIN_CS1     NRF_GPIO_PIN_MAP(0, 3)
#elif COMPILE_FOR_DEBUG_BOARD_HARDWARE_V1_0
        #define PIN_MISO1   NRF_GPIO_PIN_MAP(1, 0)
        #define PIN_MOSI1   NRF_GPIO_PIN_MAP(0, 20)
        #define PIN_CLK1    NRF_GPIO_PIN_MAP(0, 9)
        #define PIN_CS1     NRF_GPIO_PIN_MAP(0, 10)
#elif COMPILE_FOR_DEBUG_BOARD_HARDWARE_V1_1
        #define PIN_MISO1   NRF_GPIO_PIN_MAP(1, 0)
        #define PIN_MOSI1   NRF_GPIO_PIN_MAP(0, 20)
        #define PIN_CLK1    NRF_GPIO_PIN_MAP(0, 17)
        #define PIN_CS1     NRF_GPIO_PIN_MAP(0, 3)
#elif COMPILE_FOR_DEBUG_BOARD_HARDWARE_V1_2
        #define PIN_MISO1   NRF_GPIO_PIN_MAP(0, 0)
        #define PIN_MOSI1   NRF_GPIO_PIN_MAP(0, 20)
        #define PIN_CLK1    NRF_GPIO_PIN_MAP(0, 17)
        #define PIN_CS1     NRF_GPIO_PIN_MAP(0, 3)
#endif
#endif

// SPI2
#if COMPILE_SPI2_SUPPORT
#if COMPILE_FOR_NEURON_2_HARDWARE_V1_0
        #define PIN_MISO2   NRF_GPIO_PIN_MAP(1, 9)
        #define PIN_MOSI2   NRF_GPIO_PIN_MAP(0, 11)
        #define PIN_CLK2    NRF_GPIO_PIN_MAP(0, 29)
        #define PIN_CS2     NRF_GPIO_PIN_MAP(0, 2)
#elif COMPILE_FOR_NEURON_2_HARDWARE_V1_1
        #define PIN_MISO2   NRF_GPIO_PIN_MAP(1, 9)
        #define PIN_MOSI2   NRF_GPIO_PIN_MAP(0, 11)
        #define PIN_CLK2    NRF_GPIO_PIN_MAP(0, 1)
        #define PIN_CS2     NRF_GPIO_PIN_MAP(0, 2)
#elif COMPILE_FOR_DEBUG_BOARD_HARDWARE_V1_0
        #define PIN_MISO2   NRF_GPIO_PIN_MAP(1, 9)
        #define PIN_MOSI2   NRF_GPIO_PIN_MAP(0, 11)
        #define PIN_CLK2    NRF_GPIO_PIN_MAP(0, 29)
        #define PIN_CS2     NRF_GPIO_PIN_MAP(0, 2)
#elif COMPILE_FOR_DEBUG_BOARD_HARDWARE_V1_1
        #define PIN_MISO2   NRF_GPIO_PIN_MAP(1, 9)
        #define PIN_MOSI2   NRF_GPIO_PIN_MAP(0, 11)
        #define PIN_CLK2    NRF_GPIO_PIN_MAP(1, 1)
        #define PIN_CS2     NRF_GPIO_PIN_MAP(0, 2)
#elif COMPILE_FOR_DEBUG_BOARD_HARDWARE_V1_2
        #define PIN_MISO2   NRF_GPIO_PIN_MAP(1, 9)
        #define PIN_MOSI2   NRF_GPIO_PIN_MAP(0, 11)
        #define PIN_CLK2    NRF_GPIO_PIN_MAP(0, 1)
        #define PIN_CS2     NRF_GPIO_PIN_MAP(0, 2)
#endif
#endif

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

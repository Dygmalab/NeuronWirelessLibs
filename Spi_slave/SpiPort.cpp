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

#include "SpiPort.h"

#include "Fifo_buffer.h"

#ifndef BSP_SPI_PORT_SPI0
#define BSP_SPI_PORT_SPI0            0
#endif /* BSP_SPI_PORT_SPI0 */

#ifndef BSP_SPI_PORT_SPI1
#define BSP_SPI_PORT_SPI1            0
#endif /* BSP_SPI_PORT_SPI1 */

#ifndef BSP_SPI_PORT_SPI2
#define BSP_SPI_PORT_SPI2            0
#endif /* BSP_SPI_PORT_SPI2 */

#if ( BSP_SPI_PORT_SPI0 + BSP_SPI_PORT_SPI1 + BSP_SPI_PORT_SPI2 ) == 0
#error "At least one SPI port is expected to be chosen."
#endif

// SPI0
#if BSP_SPI_PORT_SPI0
Spi_slave spi0_slave(0,                /* Chip SPI port used */
                     BSP_SPI0_MISO,        /* MISO0 */
                     BSP_SPI0_MOSI,        /* MOSI0 */
                     BSP_SPI0_CLK,         /* Clock0 */
                     BSP_SPI0_CS,          /* Chip Select 0 */
                     NRF_SPIS_MODE_1); /* -> CPOL = 0 / CPHA = 1 */
#endif

// SPI1
#if BSP_SPI_PORT_SPI1
Spi_slave spi1_slave(1,                /* Chip SPI port used */
                     BSP_SPI1_MISO,        /* MISO1 */
                     BSP_SPI1_MOSI,        /* MOSI1 */
                     BSP_SPI1_CLK,         /* Clock1 */
                     BSP_SPI1_CS,          /* Chip Select 1 */
                     NRF_SPIS_MODE_1); /* -> CPOL = 0 / CPHA = 1 */
#endif

// SPI2
#if BSP_SPI_PORT_SPI2
Spi_slave spi2_slave(2,                /* Chip SPI port used */
                     BSP_SPI2_MISO,        /* MISO2 */
                     BSP_SPI2_MOSI,        /* MOSI2 */
                     BSP_SPI2_CLK,         /* Clock2 */
                     BSP_SPI2_CS,          /* Chip Select 2 */
                     NRF_SPIS_MODE_1); /* -> CPOL = 0 / CPHA = 1 */
#endif


SpiPort::SpiPort(uint8_t _spi_port_used)
  : spi_port_used(_spi_port_used) {
#if BSP_SPI_PORT_SPI0
    if (_spi_port_used == 0) {
        spi_slave = &spi0_slave;
    }
#endif

#if BSP_SPI_PORT_SPI1
    if (_spi_port_used == 1) {
        spi_slave = &spi1_slave;
    }
#endif

#if BSP_SPI_PORT_SPI2
    if (_spi_port_used == 2) {
        spi_slave = &spi2_slave;
    }
#endif

    if( spi_slave == nullptr )
    {
        ASSERT_DYGMA( false, "The requested SPI port is not supported." );
    }
}

void SpiPort::init() {
    if (spi_slave == nullptr) return;

    spi_slave->init();  // Initialice SPI slave.
}

//void SpiPort::deInit() {
//    if (spi_slave == nullptr) return;
//
//    spi_slave->deinit();
//}

void SpiPort::run() {
    if (spi_slave == nullptr) return;

    spi_slave->run();
}

bool SpiPort::is_connected() {
    if (spi_slave == nullptr) return false;

    return spi_slave->is_connected();
}

bool SpiPort::sendPacket(Packet &packet) {
    if (spi_slave == nullptr) return false;

    spi_slave->tx_fifo->put(&packet);

    return true;
}

void SpiPort::clearSend() {
    if (spi_slave == nullptr) return ;

    while (!spi_slave->tx_fifo->is_empty()){
        spi_slave->tx_fifo->removeOne();
    }

}

void SpiPort::clearRead() {
    if (spi_slave == nullptr) return ;

    while (!spi_slave->rx_fifo->is_empty()){
        spi_slave->rx_fifo->removeOne();
    }

}

bool SpiPort::readPacket(Packet &packet) {
    if (spi_slave == nullptr) return false;

    if (spi_slave->rx_fifo->is_empty()) return false;

    spi_slave->rx_fifo->get(&packet);

    return true;
}

bool SpiPort::peekPacket(Packet &packet) {
    if (spi_slave == nullptr) return false;

    if (spi_slave->rx_fifo->is_empty()) return false;

    spi_slave->rx_fifo->peek(&packet);

    return true;
}


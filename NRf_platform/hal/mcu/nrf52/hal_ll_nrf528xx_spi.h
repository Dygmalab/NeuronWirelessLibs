
/*
 * The MIT License (MIT)
 *
 * Copyright (C) 2022  Dygma Lab S.L.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
 
#ifndef __HAL_LL_NRF528XX_SPI_H_
#define __HAL_LL_NRF528XX_SPI_H_

#include "hal_mcu_gpio.h"

/* configuration */
typedef enum
{
    HAL_MCU_SPI_PERIPH_DEF_SPI0 = 1,
    HAL_MCU_SPI_PERIPH_DEF_SPI1,
    HAL_MCU_SPI_PERIPH_DEF_SPI2,
    HAL_MCU_SPI_PERIPH_DEF_SPI3,
} hal_mcu_spi_periph_def_t;

typedef enum
{
    HAL_MCU_SPI_FREQ_125K = 1,
    HAL_MCU_SPI_FREQ_250K,
    HAL_MCU_SPI_FREQ_500K,
    HAL_MCU_SPI_FREQ_1M,
    HAL_MCU_SPI_FREQ_2M,
    HAL_MCU_SPI_FREQ_4M,

} hal_mcu_spi_frequency_t;


#endif /* __HAL_LL_NRF528XX_SPI_H_ */

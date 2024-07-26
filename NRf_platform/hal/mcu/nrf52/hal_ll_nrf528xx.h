
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
 
#ifndef __HAL_LL_NRF528XX_H
#define __HAL_LL_NRF528XX_H

#include "hal/hal_config.h"

#if !defined( HAL_CFG_MCU_SERIES )

    #define HAL_CFG_MCU_SERIES      HAL_MCU_SERIES_NRF52

    #if HAL_CFG_MCU == HAL_MCU_NRF52833
        #define HAL_MCU_LL_SPEC_LINK "hal/mcu/nrf52/nrf52833/hal_ll_nrf52833_private.h"
    #else
        #undef HAL_CFG_MCU_SERIES
    #endif

#endif /* HAL_CFG_MCU_SERIES */

#if HAL_CFG_MCU_SERIES == HAL_MCU_SERIES_NRF52

    #define HAL_MCU_LL_GPIO_LINK "hal/mcu/nrf52/hal_ll_nrf528xx_gpio.h"
    #define HAL_MCU_LL_MCU_LINK "hal/mcu/nrf52/hal_ll_nrf528xx_mcu.h"
    #define HAL_MCU_LL_MUTEX_LINK "hal/mcu/nrf52/hal_ll_nrf528xx_mutex.h"
    #define HAL_MCU_LL_SPI_LINK "hal/mcu/nrf52/hal_ll_nrf528xx_spi.h"

#endif /* HAL_CFG_MCU_SERIES */

#endif /* __HAL_LL_NRF528XX_H */

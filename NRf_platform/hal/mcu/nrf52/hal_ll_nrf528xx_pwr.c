
/*
 * The MIT License (MIT)
 *
 * Copyright (C) 2026  Dygma Lab S.L.
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

#include "nrf_pwr_mgmt.h"

#include "hal/mcu/hal_mcu_pwr_ll.h"

#if HAL_CFG_MCU_SERIES == HAL_MCU_SERIES_NRF52

result_t hal_ll_mcu_pwr_init( void )
{
    uint32_t err_code;

    /* Initialize the nRF power management */
    err_code = nrf_pwr_mgmt_init( );
    ASSERT_DYGMA( err_code == NRF_SUCCESS, "nrf_pwr_mgmt_init failed." );

    return RESULT_OK;

    UNUSED( err_code );
}

void hal_ll_mcu_pwr_sleep_handle( void )
{
    nrf_pwr_mgmt_run();
}

#endif /* HAL_CFG_MCU_SERIES */


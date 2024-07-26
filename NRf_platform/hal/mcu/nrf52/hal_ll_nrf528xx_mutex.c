
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
 
#include "hal/mcu/hal_mcu_mutex_ll.h"

#include "nrf_mtx.h"

#if HAL_CFG_MCU_SERIES == HAL_MCU_SERIES_NRF52

struct hal_mcu_mutex
{
    nrf_mtx_t nrf_mtx;
};


static void _nrf_mtx_init( hal_mcu_mutex_t * _mutex )
{
    nrf_mtx_init( &_mutex->nrf_mtx );
}

void hal_ll_mcu_mutex_init( hal_mcu_mutex_t ** __mutex )
{
    /* Allocate the mutex instance */
    *__mutex = heap_alloc( sizeof(hal_mcu_mutex_t) );

    _nrf_mtx_init( *__mutex );
}


void hal_ll_mcu_mutex_destroy( hal_mcu_mutex_t * _mutex )
{
    nrf_mtx_destroy( &_mutex->nrf_mtx );
}

bool_t hal_ll_mcu_mutex_trylock( hal_mcu_mutex_t * _mutex )
{
    return nrf_mtx_trylock( &_mutex->nrf_mtx );
}

void hal_ll_mcu_mutex_unlock( hal_mcu_mutex_t * _mutex )
{
    nrf_mtx_unlock( &_mutex->nrf_mtx );
}

#endif /* HAL_CFG_MCU_SERIES */


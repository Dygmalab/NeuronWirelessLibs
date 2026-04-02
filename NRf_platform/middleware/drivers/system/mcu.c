
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

#include "mcu.h"
#include "halsep/hal_mcu_pwr.h"

typedef struct
{
    bool_t sleep_now;
} mcu_t;

static mcu_t mcu;

/* Prototypes */
static result_t _sleep_init( mcu_t * p_mcu );

result_t mcu_init( void )
{
    result_t result = RESULT_ERR;

    result = _sleep_init( &mcu );
    EXIT_IF_ERR( result, "_sleep_init failed" );

_EXIT:
    return result;
}

/*******************************************/
/*              Sleep Control              */
/*******************************************/

static result_t _sleep_init( mcu_t * p_mcu )
{
    result_t result = RESULT_ERR;

    result = hal_mcu_pwr_init();
    EXIT_IF_ERR( result, "hal_mcu_pwr_init failed" );

    mcu.sleep_now = true;

_EXIT:
    return result;
}

result_t mcu_sleep_init( void )
{
    return _sleep_init( &mcu );
}

void mcu_sleep_postpone( void )
{
    mcu.sleep_now = false;
}

void mcu_sleep_control( void )
{
    if ( mcu.sleep_now == true )
    {
        hal_mcu_pwr_sleep_handle();
    }

    mcu.sleep_now = true;
}

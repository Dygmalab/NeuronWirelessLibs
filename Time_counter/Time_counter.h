/*
 * Time Counter -- Class to count time in milliseconds and microseconds.
 *
 * You can set the resolution in microseconds, which is not recommended
 * to be less than 50us or 100us due to the error caused by the delay
 * due to the execution of the code itself.
 *
 * You can set the timer used and the capture/compare unit used through
 * the corresponding #defines. You can also set the counter frequency if you wish.
 *
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

#ifndef __TIME_COUNTER_NRF__
#define __TIME_COUNTER_NRF__

#ifdef __cplusplus
extern "C"
{
#endif

#include "dl_middleware.h"

typedef uint64_t systim_tick_t;
typedef systim_tick_t dl_timer_t;

void timer_counter_init(uint32_t micros_resolution);
systim_tick_t timer_counter_get_millis(void);
systim_tick_t timer_counter_get_micros(void);

void timer_set_ms( dl_timer_t * p_timer, uint32_t ms );
void timer_set_us( dl_timer_t * p_timer, uint32_t us );
bool timer_check( dl_timer_t * p_timer );

#ifdef __cplusplus
}
#endif

#endif // __TIME_COUNTER_NRF__


/*
 * The MIT License (MIT)
 *
 * Copyright (C) 2025  Dygma Lab S.L.
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

#include "dl_utils.h"

uint32_t array_popcount_get( uint8_t * p_array, uint32_t array_len )
{
    uint32_t popcount = 0;
    uint32_t i;

    for( i = 0; i < array_len; i++ )
    {
        popcount += __builtin_popcount( p_array[i] );
    }

    return popcount;
}

uint8_t array_bit_get( uint8_t * p_array, uint32_t array_len, uint32_t array_bit_pos )
{
    uint32_t byte_pos = array_bit_pos >> 3;         /* (array_bit_pos / 8) */
    uint32_t bit_pos = array_bit_pos & 0x00000007;  /* (array_bit_pos % 8) */

    uint8_t byte = p_array[byte_pos];
    uint8_t bit = ( byte >> bit_pos ) & 0x01;

    return bit;
}

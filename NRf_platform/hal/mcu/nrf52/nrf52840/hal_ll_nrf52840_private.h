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

#ifndef __HAL_LL_NRF52840_PRIVATE_H_
#define __HAL_LL_NRF52840_PRIVATE_H_

#include "nrf52840_bitfields.h"

/**************************** Flash ****************************/
#define NRF52_LL_FLASH_ADDRESS_MIN          0x00000000
#define NRF52_LL_FLASH_ADDRESS_MAX          0x000FFFFF

#define NRF52_LL_FLASH_SIZE                 (NRF52_LL_FLASH_ADDRESS_MAX - NRF52_LL_FLASH_ADDRESS_MIN + 1)
#define NRF52_LL_FLASH_PAGE_SIZE            4096

#define NRF52_LL_FLASH_ALIGN                4
#define NRF52_LL_FLASH_ALIGN_MASK           0x00000003

#define NRF52_LL_FLASH_ALIGN_REWRITE_MAX    2

#endif /* __HAL_LL_NRF52840_PRIVATE_H_ */

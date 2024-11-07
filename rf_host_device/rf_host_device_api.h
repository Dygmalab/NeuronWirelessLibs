/*
 * The MIT License (MIT)
 *
 * Copyright (C) 2020  Dygma Lab S.L.
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
#ifndef __RF_HOST_DEVICE_API_H_
#define __RF_HOST_DEVICE_API_H_

#ifdef __cplusplus
extern "C" {
#endif

    #include "rf_gateway.h"

    #define RFGW_PIPE_ID_KEYSCANNER_LEFT RFGW_PIPE_ID_1
    #define RFGW_PIPE_ID_KEYSCANNER_RIGHT RFGW_PIPE_ID_2

    extern void rfhdev_api_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __RF_HOST_DEVICE_API_H_ */

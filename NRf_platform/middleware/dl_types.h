
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

#ifndef __DL_TYPES_H_
#define __DL_TYPES_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/*
 * DO NOT MODIFY!!!
 *
 * The Dygma core is used among various Dygma frameworks to assure the same coding philosophy
 * and prevent conflicts when used together in a project.
 */
#ifndef DYGMA_CORE_TYPES_SPECIFIED
#define DYGMA_CORE_TYPES_SPECIFIED

    #define INLINE inline
    #define VOLATILE volatile
    #define PACK __attribute__((__packed__))

    typedef bool bool_t;
    typedef char char_t;

    typedef enum
    {
        RESULT_OK = 0,
        RESULT_ERR,
        RESULT_BUSY,
        RESULT_INCOMPLETE,
    } result_t;

    #define UNUSED(X) (void)X      /* To avoid gcc/g++ warnings */

#endif /* DYGMA_CORE_TYPES_SPECIFIED */

#endif /* __DL_TYPES_H_ */

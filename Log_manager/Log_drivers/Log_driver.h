
/*
 * The MIT License (MIT)
 *
 * Copyright (C) 2024  Dygma Lab S.L.
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

#ifndef __LOG_DRIVER_H_
#define __LOG_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "dl_middleware.h"

/* Handlers */

typedef const char * ( *logdrv_help_message_get_fn_t )( void * p_instance );
//typedef void( *logdrv_start_fn_t )( void * p_instance );
//typedef void( *logdrv_stop_fn_t )( void * p_instance );
typedef void( *logdrv_poll_fn_t )( void * p_instance );

typedef struct
{
    logdrv_help_message_get_fn_t help_message_get_fn;
    logdrv_poll_fn_t poll_fn;
} logdrv_handlers_t;

typedef struct
{
    /* Driver-specific Instance */
    void * p_instance;

/* Callbacks */
    const logdrv_handlers_t * p_handlers;

} logdrv_conf_t;

typedef struct logdrv logdrv_t;

extern result_t logdrv_init( logdrv_t ** pp_logdrv, const logdrv_conf_t * p_conf );

extern const char * logdrv_help_message_get( logdrv_t * p_logdrv );
extern void logdrv_poll( logdrv_t * p_logdrv );

#ifdef __cplusplus
}
#endif

#endif /* __LOG_DRIVER_H_ */

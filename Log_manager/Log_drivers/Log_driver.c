
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

#include "Log_driver.h"

struct logdrv
{
    /* Driver-specific Instance */
    void * p_instance;

    /* Callbacks */
    const logdrv_handlers_t * p_handlers;
};

static result_t _init( logdrv_t * p_logdrv, const logdrv_conf_t * p_conf )
{
    /* Driver-specific Instance */
    p_logdrv->p_instance = p_conf->p_instance;

    /* Callbacks */
    p_logdrv->p_handlers = p_conf->p_handlers;

    return RESULT_OK;
}

result_t logdrv_init( logdrv_t ** pp_logdrv, const logdrv_conf_t * p_conf)
{
    result_t result = RESULT_ERR;
    logdrv_t * p_logdrv;

    /* Allocate the logdrv instance */
    p_logdrv = heap_alloc( sizeof(logdrv_t) );

    result = _init( p_logdrv, p_conf );
    EXIT_IF_ERR( result, "_init failed" );

    *pp_logdrv = p_logdrv;

_EXIT:
    return result;
}

const char * logdrv_help_message_get( logdrv_t * p_logdrv )
{
    if( p_logdrv == NULL || p_logdrv->p_handlers->help_message_get_fn == NULL )
    {
        return NULL;
    }

    return p_logdrv->p_handlers->help_message_get_fn( p_logdrv->p_instance );
}

result_t logdrv_cmd_read( logdrv_t * p_logdrv, legdrv_cmd_read_param_t * p_cmd_read_param )
{
    if( p_logdrv == NULL || p_logdrv->p_handlers->cmd_read_fn == NULL )
    {
        return RESULT_ERR;
    }

    return p_logdrv->p_handlers->cmd_read_fn( p_logdrv->p_instance, p_cmd_read_param );
}

void logdrv_poll( logdrv_t * p_logdrv )
{
    if( p_logdrv == NULL || p_logdrv->p_handlers->poll_fn == NULL )
    {
        return;
    }

    p_logdrv->p_handlers->poll_fn( p_logdrv->p_instance );
}


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

#include "flash_logger.h"

#define PSTR(str) (str)

/* Help message */
static const char * help_message = PSTR( "log.read" );       /* Read the Logs from the internal buffer*/

/* Flash logger instance structure */
typedef struct
{
    logdrv_t * p_logdrv;
} flashlog_t;

/* Flash Logger log_driver handlers declaration */
static const logdrv_handlers_t logdrv_handlers;

/* Flash logger instance */
static flashlog_t flashlog;

static result_t _logdrv_init( flashlog_t * p_flashlog )
{
    result_t result = RESULT_ERR;
    logdrv_conf_t logdrv_conf;

    /* Prepare the Battery monitor driver config */
    logdrv_conf.p_instance = p_flashlog;
    logdrv_conf.p_handlers = &logdrv_handlers;

    /* Initialize the battery monitor driver */
    result = logdrv_init( &p_flashlog->p_logdrv, &logdrv_conf );
    EXIT_IF_ERR( result, "gzld logdrv_init failed" );

_EXIT:
    return result;
}

static INLINE result_t _init( flashlog_t * p_flashlog )
{
    result_t result = RESULT_ERR;

    /* Battery monitor driver */
    result = _logdrv_init( p_flashlog );
    EXIT_IF_ERR( result, "_logdrv_init failed" );

_EXIT:
    return result;
}

static INLINE const char * _help_message_get_fn( flashlog_t * p_flashlog )
{
    return help_message;
}

static INLINE result_t _cmd_read_fn( flashlog_t * p_flashlog, legdrv_cmd_read_param_t * p_cmd_read_param )
{
    p_cmd_read_param->logs = "Hey Alex, this is log for testing the API.";
    p_cmd_read_param->length = strlen( p_cmd_read_param->logs );

    return RESULT_OK;
}

/***********************************************************/
/*                       Log Driver                        */
/***********************************************************/

static const char * _logdrv_help_message_get_fn( void * p_instance )
{
    flashlog_t * p_flashlog = ( flashlog_t *)p_instance;

    return _help_message_get_fn( p_flashlog );
}

static result_t _logdrv_cmd_read_fn( void * p_instance, legdrv_cmd_read_param_t * p_cmd_read_param )
{
    flashlog_t * p_flashlog = ( flashlog_t *)p_instance;

    return _cmd_read_fn( p_flashlog, p_cmd_read_param );
}

static const logdrv_handlers_t logdrv_handlers =
{
    .help_message_get_fn = _logdrv_help_message_get_fn,
    .cmd_read_fn = _logdrv_cmd_read_fn,
    .poll_fn = NULL,
};

static INLINE logdrv_t * _logdrv_get( flashlog_t * p_flashlog )
{
    return p_flashlog->p_logdrv;
}

/***********************************************************/
/*                           API                           */
/***********************************************************/

result_t flashlog_init( void )
{
    return _init( &flashlog );
}

logdrv_t * flashlog_logdrv_get( void )
{
    return _logdrv_get( &flashlog );
}

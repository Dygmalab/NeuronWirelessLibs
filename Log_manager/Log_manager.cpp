
/*
 * kaleidoscope::plugin::Log_manager -- Manage Neuron Logs
 * Copyright (C) 2024  Dygma Lab S.L.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "kaleidoscope/plugin/FocusSerial.h"

#include "flash_logger.h"
#include "Log_driver.h"
#include "Log_manager.h"

namespace kaleidoscope
{
namespace plugin
{

void Log_manager::init(void)
{
    result_t result = RESULT_ERR;

/* Get the Log driver first */
#if DL_LOGS_TYPE_FLASH == 1

    result = flashlog_init();
    ASSERT_DYGMA( result == RESULT_OK, "flashlog_init failed" );

    p_logdrv = flashlog_logdrv_get();

#else
#error "The Log driver is not specified"
#endif

    UNUSED( result );
}

EventHandlerResult Log_manager::onFocusEvent(const char *command)
{
    result_t result;

    if ( ::Focus.handleHelp(command, logdrv_help_message_get( p_logdrv ) ) )
    {
        return EventHandlerResult::OK;
    }

    if (strncmp_P(command, PSTR("log."), 4) != 0)
    {
        return EventHandlerResult::OK;
    }

    if (strncmp_P(command + 4, PSTR("read"), 4) == 0)
    {
        legdrv_cmd_read_param_t cmd_read_param;

        result = logdrv_cmd_read( p_logdrv, &cmd_read_param );
        if( result != RESULT_OK )
        {
            return EventHandlerResult::OK;
        }

        Runtime.serialPort().write( cmd_read_param.logs, cmd_read_param.length );

        return EventHandlerResult::OK;
    }

    return EventHandlerResult::EVENT_CONSUMED;
}

} // namespace plugin
} // namespace kaleidoscope

kaleidoscope::plugin::Log_manager LogManager;


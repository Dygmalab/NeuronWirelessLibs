
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

#pragma once

#include "common.h"

//#include "Colormap-Defy.h"
#include "kaleidoscope/Runtime.h"
//#include "kaleidoscope/plugin.h"
//#include "kaleidoscope/plugin/FocusSerial.h"
#include "Log_driver.h"
//#include <Arduino.h>
//#include <Kaleidoscope-EEPROM-Settings.h>
//#include <Kaleidoscope-Ranges.h>

namespace kaleidoscope
{
namespace plugin
{
    class Log_manager : public Plugin
    {
        public:

        void init(void);
        EventHandlerResult onFocusEvent(const char *command);

        private:

        logdrv_t * p_logdrv;

    };
}
}

extern kaleidoscope::plugin::Log_manager LogManager;

#if DL_LOGS_ENABLED == 1

    #define DL_LOGS_INIT()  LogManager.init()

#else /* DL_LOGS_ENABLED */

    #define DL_LOGS_INIT()

#endif /* DL_LOGS_ENABLED */

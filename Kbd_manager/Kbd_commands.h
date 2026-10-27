/* -*- mode: c++ -*-
 * Kbd_commands -- Handling the commands coming from the host
 * Copyright (C) 2026  Dygma Lab S.L.
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
 *
 */

#pragma once

#include "dl_middleware.h"

#include "kbd_if.h"

class KbdCommands
{
    public:

        result_t init( void );

    private:

        kbdif_t * p_kbdif = NULL;
        result_t kbdif_initialize(void);

    private:

        static const kbdif_handlers_t kbdif_handlers;
        static kbdapi_event_result_t kbdif_command_event_cb( void * p_instance, const char * p_command );
};

extern class KbdCommands kbdCommands;

/* -*- mode: c++ -*-
 * RadioManager -- Manage RF Signal and status in wireless devices
 * Copyright (C) 2020  Dygma Lab S.L.
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
 * Author: Alejandro Parcet, @alexpargon
 *
 */

#pragma once

#include "kbd_if.h"

class RadioManager {

  public:
    typedef enum : uint8_t
    {
        LOW_P,
        MEDIUM_P,
        HIGH_P,
    } rf_power_t;

    typedef struct PACK
    {
        rf_power_t rf_power;
    } rf_config_t;

  public:

    result_t init();
    void enable();
    void poll();

    bool isEnabled();
    bool isInited();

  private:
    kbdif_t * p_kbdif = NULL;
    result_t kbdif_initialize(void);

  private:

    static const rf_config_t * p_rf_config;

    static bool inited;
    static uint16_t channel_hop;
    static uint16_t settings_base_;
    static void setPowerRF();

    static const kbdif_handlers_t kbdif_handlers;

    static kbdapi_event_result_t kbdif_command_event_cb( void * p_instance, const char * p_command );

    static void cfgmem_rf_power_save( rf_power_t rf_power );
};

extern class RadioManager RadioManager;

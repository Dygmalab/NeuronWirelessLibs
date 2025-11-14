/*
 * Config_manager -- Manage System configurable parameters in the non-volatile
 *                   memory
 * Copyright (C) 2025  Dygma Lab S.L.
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

#include "Config_manager.h"
#include "EEPROM.h"

bool_t ConfigManager::item_validity_check( const void * p_item_add, uint16_t item_size )
{
    /* Check the target is within the config space */
    if( p_item_add < (uint8_t *)p_cache || ((uint8_t *)p_item_add + item_size) > (uint8_t *)p_cache + cache_size )
    {
        return false;
    }

    return true;
}

result_t ConfigManager::config_item_request( cfg_item_type_t item_type, const void ** pp_item )
{
    if( item_request_cb == nullptr )
    {
        return RESULT_ERR;
    }

    return item_request_cb( item_type, pp_item );
}

result_t ConfigManager::config_item_update( const void * p_config_item, const void * p_new_item, uint16_t item_size )
{
    if( item_validity_check( p_config_item, item_size ) == false )
    {
        ASSERT_DYGMA( false, "ConfigManager::item_validity_check failed" );
        return RESULT_ERR;
    }

    if( memcmp( p_config_item, p_new_item, item_size ) == 0)
    {
        /* The configuration has not changed */
        return RESULT_OK;
    }

    /* Item is valid and within the cache space, so we can update the config item */
    memcpy( (void *)p_config_item, p_new_item, item_size );

    /* Request the save into the memory */
    config_save_request();

    return RESULT_OK;
}

void ConfigManager::config_load( void )
{
    bool_t res;

    res = EEPROM.get( 0, p_cache, cache_size );
    ASSERT_DYGMA( res == true, "EEPROM.get failed" );

    UNUSED( res );
}

void ConfigManager::config_save_request( void )
{
    config_save_requested = true;
    timer_set_ms( &config_save_timer, CONFIG_SAVE_TIMEOUT_MS );
}

void ConfigManager::config_save( void )
{
    bool_t res;

    res = EEPROM.put( 0, p_cache, cache_size );
    ASSERT_DYGMA( res == true, "EEPROM.put failed" );

    res = EEPROM.commit();
    ASSERT_DYGMA( res == true, "EEPROM.commit failed" );

    UNUSED( res );
}

/********************************************/
/*           Keyboard API memory            */
/********************************************/

void ConfigManager::kbdmem_ll_init( void )
{
    result_t result = RESULT_ERR;
    kbdmem_config_t config;

    config.p_instance = this;
    config.item_request_cb = kbdmem_ll_item_request_cb;
    config.data_save_cb = kbdmem_ll_data_save_cb;

    result = kbdmem_init( &config );
    ASSERT_DYGMA( result == RESULT_OK, "kbdmem_init failed" );

    UNUSED( result );
}

INLINE result_t ConfigManager::kbdmem_ll_item_request( kbdmem_item_type_t item_type, const void ** pp_item )
{
    if( item_request_kbdmem_cb == nullptr )
    {
        return RESULT_ERR;
    }

    return item_request_kbdmem_cb( item_type, pp_item );
}

INLINE result_t ConfigManager::kbdmem_ll_data_save( const void * p_mem_target, const void * p_data, uint16_t data_len )
{
    return config_item_update( p_mem_target, p_data, data_len );
}

result_t ConfigManager::kbdmem_ll_item_request_cb( void * p_instance, kbdmem_item_type_t item_type, const void ** pp_item )
{
    ConfigManager * p_ConfigManager = ( ConfigManager *)p_instance;

    return p_ConfigManager->kbdmem_ll_item_request( item_type, pp_item );
}

result_t ConfigManager::kbdmem_ll_data_save_cb( void * p_instance, const void * p_mem_target, const void * p_data, uint16_t data_len )
{
    ConfigManager * p_ConfigManager = ( ConfigManager *)p_instance;

    return p_ConfigManager->kbdmem_ll_data_save( p_mem_target, p_data, data_len );
}


/********************************************/
/*                 Machine                  */
/********************************************/

INLINE void ConfigManager::machine_state_set( config_state_t state )
{
    machine_state = state;
}

INLINE void ConfigManager::machine_state_idle( void )
{
    if( config_save_requested == true && timer_check( &config_save_timer ) == true )
    {
        config_save_requested = false;
        machine_state_set( CONFIG_STATE_SAVE );
    }
}

INLINE void ConfigManager::machine_state_save( void )
{
    /* Update the brightness and return to the IDLE state */
    config_save();
    machine_state_set( CONFIG_STATE_IDLE );
}

INLINE void ConfigManager::machine( void )
{
    switch( machine_state )
    {
        case CONFIG_STATE_IDLE:

            machine_state_idle();

            break;

        case CONFIG_STATE_SAVE:

            machine_state_save();

            break;

        default:

            ASSERT_DYGMA( false, "Unhandled led_manager_state_t state" );

            break;
    }
}

/********************************************/
/*                   API                    */
/********************************************/

void ConfigManager::init( const ConfigManager_config_t * p_config )
{
    /* Save the configuration cache */
    p_cache = p_config->p_config_cache;
    cache_size = p_config->config_cache_size;

    /* Save the callbacks */
    item_request_cb = p_config->item_request_cb;
    item_request_kbdmem_cb = p_config->item_request_kbdmem_cb;

    /* Initialize the EEPROM */
    EEPROM.begin( cache_size );

    /* Get the config image */
    config_load();

    /* Initialize the keyboard API memory interface */
    kbdmem_ll_init();
}

void ConfigManager::run( void )
{
    machine();
}

class ConfigManager ConfigManager;


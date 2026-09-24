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
#include "kbd_memory.h"

bool_t ConfigManager::item_validity_check( const void * p_item_add, uint16_t item_size )
{
    /* Check the target is within the config space */
    if( (uint8_t *)p_item_add < cache || ((uint8_t *)p_item_add + item_size) > cache + sizeof( cache ) )
    {
        return false;
    }

    return true;
}

result_t ConfigManager::config_item_request( const void ** pp_config_item, uint16_t item_size )
{
    uint32_t item_size_align = alignment_ceil( item_size, MCU_ALIGNMENT_SIZE );

    /* Check the cache size */
    if( ( p_cache_pointer - cache + item_size_align ) > (uint32_t)sizeof( cache ) )
    {
        ASSERT_DYGMA( false, "failed - configuration cache size exceeded" );
        return RESULT_ERR;
    }

    *pp_config_item = p_cache_pointer;
    p_cache_pointer += item_size_align;

    return RESULT_OK;
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
    result_t result = RESULT_ERR;

    result = EEPROM.read( 0, cache, sizeof( cache ) );
    ASSERT_DYGMA( result == RESULT_OK, "EEPROM.read failed" );

    UNUSED( result );
}

void ConfigManager::config_save_request( void )
{
    config_save_requested = true;
    timer_set_ms( &config_save_timer, CONFIG_SAVE_TIMEOUT_MS );

    mcu_sleep_postpone();
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

INLINE result_t ConfigManager::kbdmem_ll_item_request( const void ** pp_config_item, uint16_t item_size )
{
    return config_item_request( pp_config_item, item_size );
}

INLINE result_t ConfigManager::kbdmem_ll_data_save( const void * p_mem_target, const void * p_data, uint16_t data_len )
{
    return config_item_update( p_mem_target, p_data, data_len );
}

result_t ConfigManager::kbdmem_ll_item_request_cb( void * p_instance, const void ** pp_config_item, uint16_t item_size )
{
    ConfigManager * p_ConfigManager = ( ConfigManager *)p_instance;

    return p_ConfigManager->kbdmem_ll_item_request( pp_config_item, item_size );
}

result_t ConfigManager::kbdmem_ll_data_save_cb( void * p_instance, const void * p_mem_target, const void * p_data, uint16_t data_len )
{
    ConfigManager * p_ConfigManager = ( ConfigManager *)p_instance;

    return p_ConfigManager->kbdmem_ll_data_save( p_mem_target, p_data, data_len );
}


/********************************************/
/*                 EEPROM                   */
/********************************************/

INLINE void ConfigManager::eeprom_event_handler( EEPROMClass::eeprom_event_type_t event_type )
{
    mcu_sleep_postpone();

    switch( event_type )
    {
        case EEPROMClass::EEPROM_EVENT_TYPE_WRITE_FINISHED:

            eeprom_in_progress_flag = false;

            break;

        case EEPROMClass::EEPROM_EVENT_TYPE_ERASE_FINISHED:

            eeprom_in_progress_flag = false;

            break;

        default:

            ASSERT_DYGMA( false, "Unhandled EEPROM event type" );

            break;
    }
}

void ConfigManager::eeprom_event_cb( void * p_instance, EEPROMClass::eeprom_event_type_t event_type )
{
    ConfigManager * p_config_manager = ( ConfigManager *)p_instance;
    p_config_manager->eeprom_event_handler( event_type );
}

INLINE result_t ConfigManager::eeprom_init( void )
{
    result_t result = RESULT_ERR;
    EEPROMClass::eeprom_config_t config;

    config.p_instance = this;
    config.event_cb = eeprom_event_cb;

    result = EEPROM.init( &config );
    EXIT_IF_ERR( result, "EEPROM.init failed" );

_EXIT:
    return result;
}

/********************************************/
/*                 Machine                  */
/********************************************/

INLINE void ConfigManager::machine_state_set( config_state_t state )
{
    machine_state = state;

    mcu_sleep_postpone();
}

INLINE void ConfigManager::machine_state_idle( void )
{
    if( config_save_requested == true && timer_check( &config_save_timer ) == true )
    {
        config_save_requested = false;
        machine_state_set( CONFIG_STATE_ERASE );
    }
}

INLINE void ConfigManager::machine_state_erase( void )
{
    result_t result = RESULT_ERR;

    /* Set the EEPROM in progress flag */
    eeprom_in_progress_flag = true;

    /* Initiate the EEPROM erase process */
    result = EEPROM.erase();
    STOP_IF_ERR( result, "EEPROM.erase failed" );
    EXIT_IF_NOK( result );

    machine_state_set( CONFIG_STATE_ERASE_WAIT );

_EXIT:
    if( result != RESULT_OK )
    {
        eeprom_in_progress_flag = false;
    }

    return;
}

INLINE void ConfigManager::machine_state_erase_wait( void )
{
    if( eeprom_in_progress_flag == true )
    {
        return;
    }

    machine_state_set( CONFIG_STATE_WRITE );
}

INLINE void ConfigManager::machine_state_write( void )
{
    result_t result = RESULT_ERR;

    /* Set the EEPROM in progress flag */
    eeprom_in_progress_flag = true;

    /* Initiate the EEPROM write process */
    result = EEPROM.write( 0, cache, sizeof( cache ) );
    STOP_IF_ERR( result, "EEPROM.write failed" );
    EXIT_IF_NOK( result );

    machine_state_set( CONFIG_STATE_WRITE_WAIT );

_EXIT:
    if( result != RESULT_OK )
    {
        eeprom_in_progress_flag = false;
    }

    return;
}

INLINE void ConfigManager::machine_state_write_wait( void )
{
    if( eeprom_in_progress_flag == true )
    {
        return;
    }

    machine_state_set( CONFIG_STATE_IDLE );
}

INLINE void ConfigManager::machine( void )
{
    switch( machine_state )
    {
        case CONFIG_STATE_IDLE:

            machine_state_idle();

            break;

        case CONFIG_STATE_ERASE:

            machine_state_erase();

            break;

        case CONFIG_STATE_ERASE_WAIT:

            machine_state_erase_wait();

            break;

        case CONFIG_STATE_WRITE:

            machine_state_write();

            break;

        case CONFIG_STATE_WRITE_WAIT:

            machine_state_write_wait();

            break;

        default:

            ASSERT_DYGMA( false, "Unhandled led_manager_state_t state" );

            break;
    }
}

/********************************************/
/*                   API                    */
/********************************************/

result_t ConfigManager::init( void )
{
    result_t result = RESULT_ERR;

    /* Initialize the cache pointer */
    p_cache_pointer = cache;

    /* Initialize the EEPROM */
    result = eeprom_init();
    EXIT_IF_ERR( result, "eeprom_init failed" );

    /* Get the config image */
    config_load();

    /* Initialize the keyboard API memory interface */
    kbdmem_ll_init();

_EXIT:
    return result;
}

bool_t ConfigManager::is_busy( void )
{
    if( ( machine_state != CONFIG_STATE_IDLE ) || config_save_requested == true )
    {
        return true;
    }
    else
    {
        return false;
    }
}

void ConfigManager::run( void )
{
    machine();
}

class ConfigManager ConfigManager;


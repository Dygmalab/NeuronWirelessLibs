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

#define FLASH_IMAGE_1_ADDR_OFFSET      0
#define FLASH_IMAGE_2_ADDR_OFFSET      (FLASH_IMAGE_1_ADDR_OFFSET + FLASH_IMAGE_SIZE)

/******************************************************/
/*                 External Functions                 */
/******************************************************/

extern void reset_mcu(void);

/******************************************************/
/*                 Configuration Items                */
/******************************************************/

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
    if( p_image_primary->is_valid() == true )
    {
        p_image_primary->data_load( cache, sizeof( cache ) );
    }
    else
    {
        /* The config image is invalid. Hence we clear the cache by filling it with 0xFF which comes from original flash-clear behavior and
         * is generally accepted with config-dependent modules for detecting invalid/cleared configuration space */
        memset( cache, 0xFF, sizeof(cache) );
    }
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
/*              Config Images               */
/********************************************/

INLINE result_t ConfigManager::images_init( void )
{
    result_t result = RESULT_ERR;

    result = image_1.init( FLASH_IMAGE_1_ADDR_OFFSET, FLASH_IMAGE_SIZE );
    EXIT_IF_ERR( result, "image_1.init failed" );

    result = image_2.init( FLASH_IMAGE_2_ADDR_OFFSET, FLASH_IMAGE_SIZE );
    EXIT_IF_ERR( result, "image_2.init failed" );

    /* Resolve the primary image */
    if( image_1.sequence_num_get() >= image_2.sequence_num_get() )
    {
        p_image_primary = &image_1;
        p_image_secondary = &image_2;
    }
    else
    {
        p_image_primary = &image_2;
        p_image_secondary = &image_1;
    }

_EXIT:
    return result;
}

INLINE void ConfigManager::images_run( void )
{
    image_1.run( );
    image_2.run( );
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
        machine_state_set( CONFIG_STATE_SAVE_START );
    }
}

INLINE void ConfigManager::machine_state_save_start( void )
{
    /* Clear the configuration request flag */
    config_save_requested = false;

    /* We start with the secondary image, which has been previously recognized as the older one */
    machine_state_set( CONFIG_STATE_IMAGE_SECONDARY_SAVE );
}

INLINE void ConfigManager::machine_state_image_secondary_save( void )
{
    result_t result = RESULT_ERR;
    uint32_t sequence_num;

    /* Use the sequence number as +1 to the primary image  */
    sequence_num = p_image_primary->sequence_num_get() + 1;

    result = p_image_secondary->save( cache, sizeof( cache ), sequence_num );
    ASSERT_DYGMA( result != RESULT_ERR, "p_image_secondary->save failed" );
    EXIT_IF_NOK( result );

    machine_state_set( CONFIG_STATE_IMAGE_SECONDARY_SAVE_WAIT );

_EXIT:
    return;
}

INLINE void ConfigManager::machine_state_image_secondary_save_wait( void )
{
    if( p_image_secondary->is_busy() == true )
    {
        return;
    }
    else if( p_image_secondary->is_valid() == false )
    {
        ASSERT_DYGMA( false, "Config secondary image is not expected to be invalid after the save process finish." );

        /* The image write is really not expected to end up with an invalid image in the memory. Instead of risking the
         * primary image fails too, we reset the mcu here to preserve the primary image and, hopefully, resolve any
         * unknown background causes of this issue. */
        reset_mcu();

        return;
    }

    /* The secondary image write was successful - move to the primary image save */
    machine_state_set( CONFIG_STATE_IMAGE_PRIMARY_SAVE );
}

INLINE void ConfigManager::machine_state_image_primary_save( void )
{
    result_t result = RESULT_ERR;
    uint32_t sequence_num;

    /* Use the sequence number as +1 to the primary image  */
    sequence_num = p_image_primary->sequence_num_get() + 1;

    result = p_image_primary->save( cache, sizeof( cache ), sequence_num );
    ASSERT_DYGMA( result != RESULT_ERR, "p_image_primary->save failed" );
    EXIT_IF_NOK( result );

    machine_state_set( CONFIG_STATE_IMAGE_PRIMARY_SAVE_WAIT );

_EXIT:
    return;
}

INLINE void ConfigManager::machine_state_image_primary_save_wait( void )
{
    if( p_image_primary->is_busy() == true )
    {
        return;
    }
    else if( p_image_primary->is_valid() == false )
    {
        ASSERT_DYGMA( false, "Config primary image is not expected to be invalid after the save process finish." );

        /* The image write is really not expected to end up with an invalid image in the memory. As the primary image
         * is now invalid, we reset the mcu here to preserve the secondary image and, hopefully, resolve any
         * unknown background causes of this issue. */
        reset_mcu();

        return;
    }

    /* The primary image write was successful - move to the save finish state */
    machine_state_set( CONFIG_STATE_SAVE_FINISH );
}

INLINE void ConfigManager::machine_state_save_finish( void )
{
    /* Check the Primary and Secondary are same */
    if( ConfigImage::image_compare( p_image_primary, p_image_secondary ) == false )
    {
        /* The primary and secondary images are not consistent, restart the images save process */
        machine_state_set( CONFIG_STATE_SAVE_START );

        return;
    }

    /* The configuration save is successful */
    machine_state_set( CONFIG_STATE_IDLE );
}

INLINE void ConfigManager::machine( void )
{
    switch( machine_state )
    {
        case CONFIG_STATE_IDLE:

            machine_state_idle();

            break;

        case CONFIG_STATE_SAVE_START:

            machine_state_save_start();

            break;

        case CONFIG_STATE_IMAGE_SECONDARY_SAVE:

            machine_state_image_secondary_save();

            break;

        case CONFIG_STATE_IMAGE_SECONDARY_SAVE_WAIT:

            machine_state_image_secondary_save_wait();

            break;

        case CONFIG_STATE_IMAGE_PRIMARY_SAVE:

            machine_state_image_primary_save();

            break;

        case CONFIG_STATE_IMAGE_PRIMARY_SAVE_WAIT:

            machine_state_image_primary_save_wait();

            break;

        case CONFIG_STATE_SAVE_FINISH:

            machine_state_save_finish();

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
    result = EEPROM.init();
    EXIT_IF_ERR( result, "EEPROM.init failed" );

    /* Initialize the configuration images */
    result = images_init();
    EXIT_IF_ERR( result, "images_init failed" );

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
    images_run();
}

class ConfigManager ConfigManager;


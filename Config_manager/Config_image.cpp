/*
 * Config_image -- An image of the Dygma configuration in the flash memory
 *
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

#include "Config_image.h"

#define IMAGE_MAGIC             0x464E4F43      /* "CONF" */
#define IMAGE_FORMAT_VERSION    1

#define IMAGE_COMMITTED         0xA55AC33C
#define IMAGE_NOT_COMMITTED     0xFFFFFFFF

#define IMAGE_CRC_DEFAULT       0xFFFFFFFF

#define IMAGE_HEADER_BASE_ADDR_OFF  ( (uint32_t)&((config_image_header_t *)0x00000000)->base )
#define IMAGE_HEADER_CRC_ADDR_OFF   ( (uint32_t)&((config_image_header_t *)0x00000000)->image_crc )

#define IMAGE_HEADER_ADDRESS        ( image_address_offset )
#define IMAGE_HEADER_SIZE           CONFIG_IMAGE_HEADER_SIZE
#define IMAGE_HEADER_BASE_ADDRESS   ( IMAGE_HEADER_ADDRESS + IMAGE_HEADER_BASE_ADDR_OFF )
#define IMAGE_HEADER_BASE_SIZE      ( sizeof( config_image_header_base_t ) )
#define IMAGE_HEADER_CRC_ADDRESS    ( IMAGE_HEADER_ADDRESS + IMAGE_HEADER_CRC_ADDR_OFF )
#define IMAGE_HEADER_CRC_SIZE       ( sizeof( uint32_t ) )
#define IMAGE_DATA_ADDRESS          ( IMAGE_HEADER_ADDRESS + IMAGE_HEADER_SIZE )
#define IMAGE_DATA_SIZE             CONFIG_IMAGE_DATA_SIZE( image_size )

#define IMAGE_PAGE_CNT              ( image_size / FLASH_STORAGE_PAGE_SIZE )

#define IMAGE_SAVE_ATTEMPT_CNT      3   /* The process will attempt to write an image 3 times */

/*******************************************************/
/*                        CRC32                        */
/*******************************************************/

INLINE uint32_t ConfigImage::crc32_header_calculate( const config_image_header_t * p_header, const uint8_t * p_data, uint32_t data_len )
{
    uint32_t crc32 = IMAGE_CRC_DEFAULT;
    config_image_header_t temp_header;

    /* Create a temporary header needed for the CRC calculation */
    temp_header = *p_header;

    temp_header.image_crc = IMAGE_CRC_DEFAULT;

    /* Calculate the header part of the CRC */
    crc32 = dlcrc32_calculate_data( crc32, (const uint8_t *)&temp_header, sizeof( temp_header ) );

    /* Calculate the data part of the CRC */
    crc32 = dlcrc32_calculate_data( crc32, p_data, data_len );

    return crc32;
}

INLINE bool_t ConfigImage::crc32_header_check( const config_image_header_t * p_header, const uint8_t * p_data, uint32_t data_len )
{
    uint32_t crc_calc;

    crc_calc = crc32_header_calculate( p_header, p_data, data_len );

    return ( crc_calc == p_header->image_crc ) ? true : false;
}

/********************************************/
/*                  Image                   */
/********************************************/

INLINE void ConfigImage::image_header_init( config_image_header_t * p_header, uint32_t sequence_num )
{
    p_header->base.magic = IMAGE_MAGIC;
    p_header->base.format_version = IMAGE_FORMAT_VERSION;
    p_header->base.sequence_num = sequence_num;
    p_header->base.data_length = IMAGE_DATA_SIZE;
    p_header->image_crc = IMAGE_CRC_DEFAULT;
}

/*******************************************************/
/*                        FLASH                        */
/*******************************************************/

INLINE bool_t ConfigImage::flash_image_crc_check( void )
{
    /* Check the size of the data */
    if( p_flash_header->base.data_length > IMAGE_DATA_SIZE )
    {
        /* The size of the image data is invalid. This can happen when the image is cleared for example */
        return false;
    }

    return crc32_header_check( p_flash_header, p_flash_data, p_flash_header->base.data_length );
}

INLINE bool_t ConfigImage::flash_image_validity_check( void )
{
    if( p_flash_header->base.magic == IMAGE_MAGIC &&
        p_flash_header->base.format_version == IMAGE_FORMAT_VERSION &&
        flash_image_crc_check() == true )
    {
        return true;
    }
    else
    {
        return false;
    }
}

INLINE void ConfigImage::flash_image_load( void )
{
    p_flash_header = ( const config_image_header_t *)EEPROM.data_ptr_get( IMAGE_HEADER_ADDRESS );
    p_flash_data = ( const uint8_t *)EEPROM.data_ptr_get( IMAGE_DATA_ADDRESS );

    flag_is_valid = flash_image_validity_check();
}

INLINE void ConfigImage::flash_image_save_start( uint32_t sequence_num )
{
    /* Invalidate the image right here */
    flag_is_valid = false;

    /* Prepare the header */
    image_header_init( &new_image_header, sequence_num );

    /* Reset the number of tries */
    image_save_attempt_cnt = IMAGE_SAVE_ATTEMPT_CNT;

    /* Start the image save process */
    state_save_start_set();
}

INLINE void ConfigImage::flash_image_save_retry( void )
{
    /* Check if all save attempts have been used */
    if( image_save_attempt_cnt == 0 )
    {
        /* We failed to write the image */
        ASSERT_DYGMA( false, "The Config image failed to be written into the FLASH memory" );

        /* Continue to the IDLE state and keep the image invalid */
        state_set( CFGIMG_STATE_IDLE );

        return;
    }

    /* Re-start the image save process */
    state_save_start_set();
}

/********************************************/
/*                 EEPROM                   */
/********************************************/

INLINE void ConfigImage::eeprom_event_handler( EEPROMClass::eeprom_event_type_t event_type )
{
    mcu_sleep_postpone();

    switch( event_type )
    {
        case EEPROMClass::EEPROM_EVENT_TYPE_WRITE_FINISHED:

            flag_flash_in_progress = false;

            break;

        case EEPROMClass::EEPROM_EVENT_TYPE_ERASE_FINISHED:

            flag_flash_in_progress = false;

            break;

        default:

            ASSERT_DYGMA( false, "Unhandled EEPROM event type" );

            break;
    }
}

void ConfigImage::eeprom_event_cb( void * p_instance, EEPROMClass::eeprom_event_type_t event_type )
{
    ConfigImage * p_config_image = ( ConfigImage *)p_instance;
    p_config_image->eeprom_event_handler( event_type );
}

/*******************************************************/
/*                    State machine                    */
/*******************************************************/

INLINE void ConfigImage::state_set( cfgimg_state_t cfgimg_state )
{
    this->state = cfgimg_state;
    mcu_sleep_postpone();
}

INLINE void ConfigImage::state_save_start_set( void )
{
    /* Lower the number of attempts */
    image_save_attempt_cnt--;

    /* Move to the start  */
    state_set( CFGIMG_STATE_SAVE_START );
}

INLINE void ConfigImage::state_write_crc_set( void )
{
    /* Calculate the image CRC from the already written FLASH data */
    new_image_header.image_crc = crc32_header_calculate( p_flash_header, p_flash_data, p_flash_header->base.data_length );

    state_set( CFGIMG_STATE_WRITE_CRC );
}

INLINE void ConfigImage::state_save_start_process( void )
{
    result_t result = RESULT_ERR;

    result = EEPROM.reserve( &flash_lock, this, eeprom_event_cb );
    EXIT_IF_NOK( result );

    /* We now own the EEPROM, hence move on to ERASE step */
    state_set( CFGIMG_STATE_ERASE );

_EXIT:
    return;
}

INLINE void ConfigImage::state_erase_process( void )
{
    result_t result = RESULT_ERR;

    /* Set the FLASH in progress flag */
    flag_flash_in_progress = true;

    /* Initiate the FLASH erase process */
    result = EEPROM.erase_offset( flash_lock, image_address_offset, IMAGE_PAGE_CNT);
    STOP_IF_ERR( result, "EEPROM.erase failed" );
    EXIT_IF_NOK( result );

    state_set( CFGIMG_STATE_ERASE_WAIT );

_EXIT:
    if( result != RESULT_OK )
    {
        flag_flash_in_progress = false;
    }

    return;
}

INLINE void ConfigImage::state_erase_wait_process( void )
{
    /* Check if the erase operation is finished */
    if( flag_flash_in_progress == true )
    {
        return;
    }

    state_set( CFGIMG_STATE_WRITE_HEADER_BASE );
}

INLINE void ConfigImage::state_write_header_base_process( void )
{
    result_t result = RESULT_ERR;

    /* Set the FLASH in progress flag */
    flag_flash_in_progress = true;

    /* Initiate the FLASH write process */
    result = EEPROM.write( flash_lock, IMAGE_HEADER_BASE_ADDRESS, (uint8_t *)&new_image_header, IMAGE_HEADER_BASE_SIZE );
    STOP_IF_ERR( result, "Config image header base EEPROM.write failed" );
    EXIT_IF_NOK( result );

    state_set( CFGIMG_STATE_WRITE_HEADER_BASE_WAIT );

_EXIT:
    if( result != RESULT_OK )
    {
        flag_flash_in_progress = false;
    }

    return;
}

INLINE void ConfigImage::state_write_header_base_wait_process( void )
{
    /* Check if the write operation is finished */
    if( flag_flash_in_progress == true )
    {
        return;
    }

    /* Check the header base is stored correctly */
    if( memcmp( &p_flash_header->base, &new_image_header.base, IMAGE_HEADER_BASE_SIZE ) != 0 )
    {
        ASSERT_DYGMA( false, "Config image header base write failed" );

        /* The header base has not been saved correctly - Retry the save process */
        flash_image_save_retry();

        return;
    }

    state_set( CFGIMG_STATE_WRITE_DATA );
}

INLINE void ConfigImage::state_write_data_process( void )
{
    result_t result = RESULT_ERR;

    /* Set the FLASH in progress flag */
    flag_flash_in_progress = true;

    /* Initiate the FLASH write process */
    result = EEPROM.write( flash_lock, IMAGE_DATA_ADDRESS, (uint8_t *)p_data_cache, data_cache_len );
    STOP_IF_ERR( result, "Config image data EEPROM.write failed" );
    EXIT_IF_NOK( result );

    state_set( CFGIMG_STATE_WRITE_DATA_WAIT );

_EXIT:
    if( result != RESULT_OK )
    {
        flag_flash_in_progress = false;
    }

    return;
}

INLINE void ConfigImage::state_write_data_wait_process( void )
{
    /* Check if the write operation is finished */
    if( flag_flash_in_progress == true )
    {
        return;
    }

    /*
     * We intentionally do not compare the stored cache here. The consistency check is made as the last step in the Config manager:
     *
     * Explanation: The the p_data_cache may be changed mid-save process. This is a trade-off for currently not using a dedicated
     *              cache space and thus saving half of the RAM otherwise needed for the Config memory processing.
     */

//    /* Check the data is stored correctly */
//    if( memcmp( p_flash_data, p_data_cache, data_cache_len ) != 0 )
//    {
//        ASSERT_DYGMA( false, "Config image data write failed" );
//
//        /* The data has not been saved correctly - Retry the save process */
//        flash_image_save_retry();
//
//        return;
//    }

    /* Move on to the CRC write */
    state_write_crc_set( );
}

INLINE void ConfigImage::state_write_crc_process( void )
{
    result_t result = RESULT_ERR;

    /* Set the FLASH in progress flag */
    flag_flash_in_progress = true;

    /* Initiate the FLASH write process */
    result = EEPROM.write( flash_lock, IMAGE_HEADER_CRC_ADDRESS, (uint8_t *)&new_image_header.image_crc, IMAGE_HEADER_CRC_SIZE );
    STOP_IF_ERR( result, "Config image CRC EEPROM.write failed" );
    EXIT_IF_NOK( result );

    state_set( CFGIMG_STATE_WRITE_CRC_WAIT );

_EXIT:
    if( result != RESULT_OK )
    {
        flag_flash_in_progress = false;
    }

    return;
}

INLINE void ConfigImage::state_write_crc_wait_process( void )
{
    /* Check if the write operation is finished */
    if( flag_flash_in_progress == true )
    {
        return;
    }

    /* Check the crc is stored correctly */
    if( memcmp( &p_flash_header->image_crc, &new_image_header.image_crc, IMAGE_HEADER_CRC_SIZE ) != 0 )
    {
        ASSERT_DYGMA( false, "Config image CRC write failed" );

        /* The CRC has not been saved correctly - Retry the save process */
        flash_image_save_retry();

        return;
    }

    /* Move on to the Finish state */
    state_set( CFGIMG_STATE_SAVE_FINISH );
}

INLINE void ConfigImage::state_save_finish_process( void )
{
    result_t result = RESULT_ERR;

    /* Validate the image */
    flag_is_valid = flash_image_validity_check();

    /* Check whether the image is valid */
    if( flag_is_valid == false )
    {
        ASSERT_DYGMA( false, "Config image write validation failed" );

        /* The validation failed - Retry the save process */
        flash_image_save_retry();

        return;
    }

    /* Release the EEPROM */
    result = EEPROM.release( flash_lock );
    ASSERT_DYGMA( result == RESULT_OK, "EEPROM.release failed" );

    /* Return back to the IDLE state */
    state_set( CFGIMG_STATE_IDLE );

    UNUSED( result );
}

INLINE void ConfigImage::state_machine( void )
{
    switch( state )
    {
        case CFGIMG_STATE_IDLE:

            /* Just waiting for the "save" function to trigger the image save process */

            break;

        case CFGIMG_STATE_SAVE_START:

            state_save_start_process();

            break;

        case CFGIMG_STATE_ERASE:

            state_erase_process();

            break;

        case CFGIMG_STATE_ERASE_WAIT:

            state_erase_wait_process();

            break;

        case CFGIMG_STATE_WRITE_HEADER_BASE:

            state_write_header_base_process();

            break;

        case CFGIMG_STATE_WRITE_HEADER_BASE_WAIT:

            state_write_header_base_wait_process();

            break;

        case CFGIMG_STATE_WRITE_DATA:

            state_write_data_process();

            break;

        case CFGIMG_STATE_WRITE_DATA_WAIT:

            state_write_data_wait_process();

            break;

        case CFGIMG_STATE_WRITE_CRC:

            state_write_crc_process();

            break;

        case CFGIMG_STATE_WRITE_CRC_WAIT:

            state_write_crc_wait_process();

            break;

        case CFGIMG_STATE_SAVE_FINISH:

            state_save_finish_process();

            break;

        default:

            ASSERT_DYGMA( false, "Unhandled ConfigImage state" );

            break;
    }
}

/*******************************************************/
/*                         API                         */
/*******************************************************/

result_t ConfigImage::init( uint32_t image_address_offset, uint32_t image_size )
{
    /* Save the Image address and size */
    this->image_address_offset = image_address_offset;
    this->image_size = image_size;

    /* Flags */
    flag_is_valid = false;
    flag_flash_in_progress = false;

    /* Load the image */
    flash_image_load();

    /* Set the Idle state */
    state = CFGIMG_STATE_IDLE;

    return RESULT_OK;
}

bool_t ConfigImage::is_valid( void )
{
    return flag_is_valid;
}

bool_t ConfigImage::is_busy( void )
{
    return ( state != CFGIMG_STATE_IDLE ) ? true : false;
}

uint32_t ConfigImage::sequence_num_get( void )
{
    if( flag_is_valid == false )
    {
        /* The invalid image returns the lowest sequence number possible */
        return 0;
    }

    return p_flash_header->base.sequence_num;
}

result_t ConfigImage::save( const uint8_t * p_data_cache, uint32_t data_cache_len, uint32_t sequence_num )
{
    if( data_cache_len > IMAGE_DATA_SIZE )
    {
        ASSERT_DYGMA( false, "ConfigImage data overflow" );
        return RESULT_ERR;
    }

    /* Save the cache */
    this->p_data_cache = p_data_cache;
    this->data_cache_len = data_cache_len;

    /* Start the image save process */
    flash_image_save_start( sequence_num );

    return RESULT_OK;
}

result_t ConfigImage::data_load( uint8_t * p_data, uint32_t data_len )
{
    if( flag_is_valid == false || data_len != p_flash_header->base.data_length )
    {
        return RESULT_ERR;
    }

    /* Load the data from the FLASH */
    memcpy( p_data, p_flash_data, data_len );

    return RESULT_OK;
}

void ConfigImage::run( void )
{
    state_machine();
}

bool_t ConfigImage::image_compare( ConfigImage * p_image_1, ConfigImage * p_image_2 )
{
    const uint8_t * p_data_1;
    const uint8_t * p_data_2;

    if( p_image_1->flag_is_valid == false || p_image_2->flag_is_valid == false )
    {
        return false;
    }
    else if( p_image_1->image_size != p_image_2->image_size )
    {
        ASSERT_DYGMA( false, "The compared Config Images are not expected to have different sizes" );
        return false;
    }

    /* Get the data pointers in the FLASH memory */
    p_data_1 = ( const uint8_t *)EEPROM.data_ptr_get( p_image_1->image_address_offset );
    p_data_2 = ( const uint8_t *)EEPROM.data_ptr_get( p_image_2->image_address_offset );

    /* Compare the two memory blocks */
    return ( memcmp( p_data_1, p_data_2, p_image_1->image_size ) == 0 ) ? true : false;
}

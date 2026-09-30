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
#include "EEPROM.h"

#define IMAGE_MAGIC             0x464E4F43      /* "CONF" */
#define IMAGE_FORMAT_VERSION    1

#define IMAGE_COMMITTED         0xA55AC33C
#define IMAGE_NOT_COMMITTED     0xFFFFFFFF

#define IMAGE_CRC_DEFAULT       0xFFFFFFFF

#define IMAGE_HEADER_ADDRESS    ( image_address )
#define IMAGE_HEADER_SIZE       CONFIG_IMAGE_HEADER_SIZE
#define IMAGE_DATA_ADDRESS      ( image_address + IMAGE_HEADER_SIZE )
#define IMAGE_DATA_SIZE         CONFIG_IMAGE_DATA_SIZE( image_size )

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

//INLINE void ConfigImage::header_write_prepare( config_image_header_t * p_header )
//{
//    p_header->base.magic = IMAGE_MAGIC;
//    p_header->base.format_version = IMAGE_FORMAT_VERSION;
//    p_header->base.sequence = ;
//    p_header->base.data_length = IMAGE_DATA_SIZE;
//    p_header->base.data_crc = IMAGE_CRC_DEFAULT;
//    p_header->commit_marker = IMAGE_NOT_COMMITTED;
//}

/*******************************************************/
/*                    State machine                    */
/*******************************************************/

INLINE void ConfigImage::state_set( cfgimg_state_t cfgimg_state )
{
    this->state = cfgimg_state;
    mcu_sleep_postpone();
}

INLINE void ConfigImage::state_machine( void )
{
    switch( state )
    {
        case CFGIMG_STATE_IDLE:

            /* Just waiting for the "save" function to trigger the image save process */

            break;

//        case CFGIMG_STATE_ERASE:
//            break;
//
//        case CFGIMG_STATE_ERASE_WAIT:
//            break;
//
//        case CFGIMG_STATE_WRITE_HEADER_BASE:
//            break;
//
//        case CFGIMG_STATE_WRITE_HEADER_BASE_WAIT:
//            break;
//
//        case CFGIMG_STATE_WRITE_COMMIT_MARKER:
//            break;
//
//        case CFGIMG_STATE_WRITE_COMMIT_MARKER_WAIT:
//            break;

        default:

            ASSERT_DYGMA( false, "Unhandled ConfigImage state" );

            break;
    }
}

/*******************************************************/
/*                         API                         */
/*******************************************************/

result_t ConfigImage::init( uint32_t image_address, uint32_t image_size )
{
    /* Save the Image address and size */
    this->image_address = image_address;
    this->image_size = image_size;

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
    this->sequence_num = sequence_num;

    /* Start the image save process */
    state_set( CFGIMG_STATE_ERASE );

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

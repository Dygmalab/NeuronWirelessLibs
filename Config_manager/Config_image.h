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

#pragma once

#include "dl_middleware.h"

#include "EEPROM.h"

typedef struct __attribute__((aligned(MCU_ALIGNMENT_SIZE)))
{
    uint32_t magic;
    uint32_t format_version;
    uint32_t sequence_num;
    uint32_t data_length;
} PACK config_image_header_base_t;

typedef struct __attribute__((aligned(MCU_ALIGNMENT_SIZE)))
{
    config_image_header_base_t base;
    uint32_t image_crc;
} PACK config_image_header_t;

/************************/
/*        Macros        */
/************************/
#define CONFIG_IMAGE_HEADER_SIZE            ( sizeof( config_image_header_t ) )
#define CONFIG_IMAGE_DATA_SIZE(image_size)  ( image_size - CONFIG_IMAGE_HEADER_SIZE )

class ConfigImage
{
    public:

        result_t init( uint32_t image_address_offset, uint32_t image_size );
        bool_t is_valid( void );
        bool_t is_busy( void );
        uint32_t sequence_num_get( void );

        result_t save( const uint8_t * p_data_cache, uint32_t data_cache_len, uint32_t sequence_num );
        result_t data_load( uint8_t * p_data, uint32_t data_len );

        void run( void );

    public:

        static bool_t image_compare( ConfigImage * p_image_1, ConfigImage * p_image_2 );

    private:

        typedef enum
        {
            CFGIMG_STATE_IDLE = 1,
            CFGIMG_STATE_SAVE_START,
            CFGIMG_STATE_ERASE,
            CFGIMG_STATE_ERASE_WAIT,
            CFGIMG_STATE_WRITE_HEADER_BASE,
            CFGIMG_STATE_WRITE_HEADER_BASE_WAIT,
            CFGIMG_STATE_WRITE_DATA,
            CFGIMG_STATE_WRITE_DATA_WAIT,
            CFGIMG_STATE_WRITE_CRC,
            CFGIMG_STATE_WRITE_CRC_WAIT,
            CFGIMG_STATE_SAVE_FINISH,
        } cfgimg_state_t;

        uint32_t image_address_offset;
        uint32_t image_size;

        cfgimg_state_t state;

        EEPROMClass::eeprom_lock_t flash_lock;
        const config_image_header_t * p_flash_header;
        const uint8_t * p_flash_data;

        /* New image process */
        config_image_header_t new_image_header;
        const uint8_t * p_data_cache;
        uint32_t data_cache_len;
        uint8_t image_save_attempt_cnt;

        /* Flags */
        bool_t flag_is_valid;
        bool_t flag_flash_in_progress;

        INLINE uint32_t crc32_header_calculate( const config_image_header_t * p_header, const uint8_t * p_data, uint32_t data_len );
        INLINE bool_t crc32_header_check( const config_image_header_t * p_header, const uint8_t * p_data, uint32_t data_len );

        INLINE void image_header_init( config_image_header_t * p_header, uint32_t sequence_num );

        INLINE bool_t flash_image_crc_check( void );
        INLINE bool_t flash_image_validity_check( void );
        INLINE void flash_image_load( void );
        INLINE void flash_image_save_start( uint32_t sequence_num );
        INLINE void flash_image_save_retry( void );

        INLINE void state_set( cfgimg_state_t cfgimg_state );
        INLINE void state_save_start_set( void );
        INLINE void state_write_crc_set( void );

        INLINE void state_save_start_process( void );
        INLINE void state_erase_process( void );
        INLINE void state_erase_wait_process( void );
        INLINE void state_write_header_base_process( void );
        INLINE void state_write_header_base_wait_process( void );
        INLINE void state_write_data_process( void );
        INLINE void state_write_data_wait_process( void );
        INLINE void state_write_crc_process( void );
        INLINE void state_write_crc_wait_process( void );
        INLINE void state_save_finish_process( void );
        INLINE void state_machine( void );

    private:

        INLINE void eeprom_event_handler( EEPROMClass::eeprom_event_type_t event_type );

        static void eeprom_event_cb( void * p_instance, EEPROMClass::eeprom_event_type_t event_type );
};

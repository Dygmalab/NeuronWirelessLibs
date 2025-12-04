/*
 *  EEPROM.cpp - nRF52833 EEPROM emulation
 *  Copyright (C) 2020  Dygma Lab S.L. All rights reserved.
 *
 *  Based on RP2040 EEPROM library, which is
 *  Copyright (c) 2021 Earle F. Philhower III. All rights reserved.
 *
 *  Based on ESP8266 EEPROM library, which is
 *  Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *  Mantainer: Gustavo Gomez Lopez @Noteolvides
 *  Mantainer: Juan Hauara @JuanHauara
*/

#include "EEPROM.h"

#include "Arduino.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include "nrf_fstorage.h"

#ifdef SOFTDEVICE_PRESENT
#include "nrf_fstorage_sd.h"
#else
#include "nrf_fstorage_nvmc.h"
#endif

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
//#include "nrf_log_default_backends.h"

#ifdef __cplusplus
}
#endif


#define FLASH_STORAGE_DEBUG_READ            1
#define FLASH_STORAGE_DEBUG_ERASE_PAGE      1
#define FLASH_STORAGE_DEBUG_WRITE           1

/*
    The NRF52833 has 128 pages of 4KB each one.
    4KB = 0x1000 in hex.
    Pages are numbered from 0 to 127.
    Start address of the last page (127) = 0x0007F000.
    Last address of the last page = 512 * 1024 Bytes = 524288 Bytes = 0x00080000

    Bootloader starts at 0x00075000 (page 117) and ends at 0x0007DF53 (page 126), 10 pages in total.
    We reserve 2 pages 0x00073000 and 0x00074000 for the EEPROM class.
    The Bluetooth module uses the Nordic FDS library, and 3 pages are reserved for it from 0x00070000
    to 0x00072000 (pages 112, 113 and 114).
    Pages 39 to 113 are free to be used by the main application.

    Flash memory map:
        Page 127 (0x0007F000) -> MBR parameters storage. Last memory page.

        Page 126 (0x0007E000) -> Reserved to be use by the bootloader. Bootloader settings page.
        Page 125 (0x0007D000) -> Bootloader.
        Page 124 (0x0007C000) -> Bootloader.
        Page 123 (0x0007B000) -> Bootloader.
        Page 122 (0x0007A000) -> Bootloader.
        Page 121 (0x00079000) -> Bootloader.
        Page 120 (0x00078000) -> Bootloader.
        Page 119 (0x00077000) -> Bootloader.
        Page 118 (0x00076000) -> Bootloader.
        Page 117 (0x00075000) -> Bootloader.

        Page 116 (0x00074000) -> EEPROM class. It uses fstorage library from the SDK.
        Page 115 (0x00073000) -> EEPROM class.

        Page 114 (0x00072000) -> FDS library used by the peer manager module in the Bluetooth.
        Page 113 (0x00071000) -> FDS library.
        Page 112 (0x00070000) -> FDS library.

        Page 113 (0x00069000) -> Max Main App.
        .
        .
        Page 39  (0x00027000) -> Min Main App.

        Page 38  (0x00026000) -> Softdevice S140.
        .
        .
        Page 1   (0x00001000) -> Softdevice S140.

        Page 0   (0x00000000) -> MBR.

    Future changes:
        - Reserve only two pages for FDS.
        - Locate the two pages for FDS directly below the bootloader and then the memory pages
          used by the EEPROM class. This way it is easier to add memory pages as we need them
          to be used by Kaleidoscope.
*/
#define FLASH_STORAGE_NUM_PAGES                 2
#define FLASH_STORAGE_PAGE_SIZE                 4096    /* Size of the flash pages in Bytes. */
#define FLASH_STORAGE_SIZE                      ( FLASH_STORAGE_NUM_PAGES * FLASH_STORAGE_PAGE_SIZE )

#define FLASH_STORAGE_FIRST_PAGE_START_ADDR     flash_first_page_start_addr_get()
#define FLASH_STORAGE_LAST_PAGE_END_ADDR        flash_last_page_end_addr_get()

#define FLASH_STORAGE_ALIGN                     4       /* The FLASH data is aligned by 4 bytes */

volatile static bool flag_write_completed = false;
volatile static bool flag_erase_completed = false;

static void fstorage_evt_handler(nrf_fstorage_evt_t *evt);
static inline uint32_t flash_first_page_start_addr_get(void);
static inline uint32_t flash_last_page_end_addr_get(void);

// Creates an fstorage instance.
NRF_FSTORAGE_DEF(nrf_fstorage_t fstorage_instance) = {
    .evt_handler = fstorage_evt_handler,

    /*
        You must set these manually, even at runtime, before nrf_fstorage_init() is called.
        The function nrf5_flash_end_addr_get() can be used to retrieve the last address on the
        last page of flash available to write data.
    */
    .start_addr = FLASH_STORAGE_FIRST_PAGE_START_ADDR,
    .end_addr = FLASH_STORAGE_LAST_PAGE_END_ADDR,
};

/*
    This function is used to get the correct flash address even if no bootloader is flashed.
    The bulk of this function has been taken from Nordic's SDK fds.c module.
*/
static inline uint32_t flash_first_page_start_addr_get(void)
{
    uint32_t const bootloader_addr = BOOTLOADER_ADDRESS;
    uint32_t const page_sz         = NRF_FICR->CODEPAGESIZE;

#if defined(NRF52810_XXAA) || defined(NRF52811_XXAA)
    // Hardcode the number of flash pages, necessary for SoC emulation.
    // nRF52810 on nRF52832 and
    // nRF52811 on nRF52840
    uint32_t const code_sz = 48;
#else
   uint32_t const code_sz = NRF_FICR->CODESIZE;
#endif

    uint32_t end_addr = (bootloader_addr != 0xFFFFFFFF) ? bootloader_addr : (code_sz * page_sz);

    return end_addr - FLASH_STORAGE_SIZE;
}

static inline uint32_t flash_last_page_end_addr_get(void)
{
    return flash_first_page_start_addr_get() + FLASH_STORAGE_SIZE - 1;
}

static void fstorage_evt_handler(nrf_fstorage_evt_t *p_evt)
{
    if (p_evt->result != NRF_SUCCESS)
    {
        NRF_LOG_ERROR("EEPROM: Error while executing an fstorage operation.");
        NRF_LOG_FLUSH();

        return;
    }

    switch (p_evt->id)
    {
        case NRF_FSTORAGE_EVT_WRITE_RESULT:
        {
#if FLASH_STORAGE_DEBUG_WRITE
            NRF_LOG_DEBUG("EEPROM: Writing completed.");
            NRF_LOG_FLUSH();
#endif

            flag_write_completed = true;
        }
        break;

        case NRF_FSTORAGE_EVT_ERASE_RESULT:
        {
#if FLASH_STORAGE_DEBUG_READ or FLASH_STORAGE_DEBUG_WRITE
            NRF_LOG_DEBUG("EEPROM: Erase completed.");
            NRF_LOG_FLUSH();
#endif

            flag_erase_completed = true;
        }
        break;

        default:
        {
        }
        break;
    }
}

result_t EEPROMClass::init( void )
{
    nrf_fstorage_api_t *fs_api;

    if( initialized == true )
    {
        /* The EEPROM is already initialized */
        return RESULT_OK;
    }

#ifdef SOFTDEVICE_PRESENT
#if FLASH_STORAGE_DEBUG_READ or FLASH_STORAGE_DEBUG_WRITE
    NRF_LOG_DEBUG("EEPROM: SoftDevice is present. Using nrf_fstorage_sd driver implementation.");
    NRF_LOG_FLUSH();
#endif

    fs_api = &nrf_fstorage_sd;
#else
#if FLASH_STORAGE_DEBUG_READ or FLASH_STORAGE_DEBUG_WRITE
    NRF_LOG_DEBUG("EEPROM: SoftDevice not present. Using nrf_fstorage_nvmc driver implementation.");
    NRF_LOG_FLUSH();
#endif

    fs_api = &nrf_fstorage_nvmc;
#endif

    ret_code_t rc = nrf_fstorage_init(&fstorage_instance, fs_api, NULL);
    if( rc != NRF_SUCCESS )
    {
        return RESULT_ERR;
    }
    APP_ERROR_CHECK(rc);

    /* Initially, the whole EEPROM space is protected and forcing the erase needs to be called before any write */
    addr_offset_protected = FLASH_STORAGE_SIZE;
    initialized = true;

    return RESULT_OK;
}

uint32_t EEPROMClass::align_get(void)
{
    return FLASH_STORAGE_ALIGN;
}

result_t EEPROMClass::read( uint32_t addr_offset, uint8_t * p_data, size_t data_size )
{
    if (data_size == 0)
    {
        /* Nothing to read */
        return RESULT_OK;
    }
    else if( addr_offset + data_size > FLASH_STORAGE_SIZE )
    {
        ASSERT_DYGMA(false, "EEPROM available space overflow");
        return RESULT_ERR;
    }

    while (nrf_fstorage_is_busy( &fstorage_instance ))  // Wait until fstorage is available.
    {
        yield();  // Meanwhile execute tasks.
    }

    // At startup, EEPROMclass loads all the flash memory pages it uses.
    ret_code_t ret_code = nrf_fstorage_read( &fstorage_instance, FLASH_STORAGE_FIRST_PAGE_START_ADDR + addr_offset, p_data, data_size );
    if ( ret_code != NRF_SUCCESS )
    {
        return RESULT_ERR;
    }

#if FLASH_STORAGE_DEBUG_READ
    NRF_LOG_DEBUG("EEPROM: Loaded flash memory, ret_code = %i", ret_code);
    NRF_LOG_FLUSH();
#endif

    return RESULT_OK;
}

result_t EEPROMClass::write( uint32_t addr_offset, const uint8_t * p_data, size_t data_size )
{
    if (data_size == 0)
    {
        /* Nothing to write */
        return RESULT_OK;
    }
    else if( addr_offset < addr_offset_protected )
    {
        ASSERT_DYGMA(false, "Trying to write into protected EEPROM space");
        return RESULT_ERR;
    }
    else if( addr_offset + data_size > FLASH_STORAGE_SIZE )
    {
        ASSERT_DYGMA(false, "EEPROM available space overflow");
        return RESULT_ERR;
    }

    while (nrf_fstorage_is_busy( &fstorage_instance ))  // Wait until fstorage is available.
    {
        yield();  // Meanwhile execute tasks.
    }

#if FLASH_STORAGE_DEBUG_ERASE_PAGE
    NRF_LOG_DEBUG("EEPROM: Writing flash...");
    NRF_LOG_FLUSH();
#endif
    flag_write_completed = false;
    ret_code_t ret_code = nrf_fstorage_write(&fstorage_instance,
                                             FLASH_STORAGE_FIRST_PAGE_START_ADDR + addr_offset,
                                             p_data,
                                             data_size,
                                             NULL);
    if (ret_code != NRF_SUCCESS)
    {
        NRF_LOG_ERROR("EEPROM: Write error, ret_code = %d", ret_code);
        NRF_LOG_FLUSH();

        /*
            If error, try increasing NRF_FSTORAGE_SD_MAX_RETRIES and NRF_FSTORAGE_SD_QUEUE_SIZE.

            Error codes:
            ret = 14 -> NRF_ERROR_NULL: If p_fs or p_src is NULL.
            ret = 8  -> NRF_ERROR_INVALID_STATE: If the module is not initialized.
            ret = 9  -> NRF_ERROR_INVALID_LENGTH: If len is zero or not a multiple of the program unit, or if it is otherwise invalid.
            ret = 16 -> NRF_ERROR_INVALID_ADDR: If the address dest is outside the flash memory boundaries specified in p_fs, or if it is unaligned.
            ret = 4  -> NRF_ERROR_NO_MEM: If no memory is available to accept the operation. When using the SoftDevice implementation, this error indicates that the
           internal queue of operations is full.
        */

        flag_write_completed = true;

        return RESULT_ERR;
    }

    /*
        The operation was accepted.
        Upon completion, the NRF_FSTORAGE_ERASE_RESULT event is sent to the callback function
        registered by the instance.

        If error, try increasing NRF_FSTORAGE_SD_MAX_RETRIES and NRF_FSTORAGE_SD_QUEUE_SIZE.
    */
    while (!flag_write_completed)
    {
        yield();  // Meanwhile execute tasks.
    }

    /* Shift the protected address offset */
    addr_offset_protected = addr_offset + data_size;

    return RESULT_OK;
}

result_t EEPROMClass::erase(void)
{
    while (nrf_fstorage_is_busy( &fstorage_instance ))  // Wait until fstorage is available.
    {
        yield();  // Meanwhile execute tasks.
    }

#if FLASH_STORAGE_DEBUG_ERASE_PAGE
    NRF_LOG_DEBUG("EEPROM: Erasing flash...");
    NRF_LOG_FLUSH();
#endif
    flag_erase_completed = false;
    ret_code_t ret_code = nrf_fstorage_erase(&fstorage_instance,
                                             FLASH_STORAGE_FIRST_PAGE_START_ADDR,
                                             FLASH_STORAGE_NUM_PAGES,
                                             NULL);
    if (ret_code != NRF_SUCCESS)
    {
        NRF_LOG_ERROR("EEPROM: Erase error, ret_code = %lu", ret_code);
        NRF_LOG_FLUSH();

        /*
            If error, try increasing NRF_FSTORAGE_SD_MAX_RETRIES and NRF_FSTORAGE_SD_QUEUE_SIZE.

            Error codes:
            ret = 14 -> NRF_ERROR_NULL: If p_fs or p_src is NULL.
            ret = 8  -> NRF_ERROR_INVALID_STATE: If the module is not initialized.
            ret = 9  -> NRF_ERROR_INVALID_LENGTH: If len is zero or not a multiple of the program unit, or if it is otherwise invalid.
            ret = 16 -> NRF_ERROR_INVALID_ADDR: If the address dest is outside the flash memory boundaries specified in p_fs, or if it is unaligned.
            ret = 4  -> NRF_ERROR_NO_MEM: If no memory is available to accept the operation. When using the SoftDevice implementation, this error indicates that the
           internal queue of operations is full.
        */

        flag_erase_completed = true;

        return RESULT_ERR;
    }

    /*
        The operation was accepted.
        Upon completion, the NRF_FSTORAGE_ERASE_RESULT event is sent to the callback function
        registered by the instance.

        If error, try increasing NRF_FSTORAGE_SD_MAX_RETRIES and NRF_FSTORAGE_SD_QUEUE_SIZE.
    */
    while (!flag_erase_completed)
    {
        yield();  // Meanwhile execute tasks.
    }

    addr_offset_protected = 0;

    return RESULT_OK;
}

EEPROMClass EEPROM;

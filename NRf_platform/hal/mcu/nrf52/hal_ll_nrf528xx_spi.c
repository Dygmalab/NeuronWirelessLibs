
/*
 * The MIT License (MIT)
 *
 * Copyright (C) 2022  Dygma Lab S.L.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
 
#include "hal/mcu/hal_mcu_spi_ll.h"

#include "nrfx_spis.h"

#if HAL_CFG_MCU_SERIES == HAL_MCU_SERIES_NRF52

#define LINE_MODE_ENCODE( cpol, cpha )    ( (cpha << 4) | (cpol << 0) )
#define LINE_MODE_0                     LINE_MODE_ENCODE( HAL_MCU_SPI_CPOL_ACTIVE_HIGH,  HAL_MCU_SPI_CPHA_LEAD )
#define LINE_MODE_1                     LINE_MODE_ENCODE( HAL_MCU_SPI_CPOL_ACTIVE_HIGH,  HAL_MCU_SPI_CPHA_TRAIL )
#define LINE_MODE_2                     LINE_MODE_ENCODE( HAL_MCU_SPI_CPOL_ACTIVE_LOW,   HAL_MCU_SPI_CPHA_LEAD )
#define LINE_MODE_3                     LINE_MODE_ENCODE( HAL_MCU_SPI_CPOL_ACTIVE_LOW,   HAL_MCU_SPI_CPHA_TRAIL )

/* Peripheral definitions */
typedef struct
{
    hal_mcu_spi_periph_def_t def;

    hal_mcu_spi_t ** pp_periph;

    NRF_SPIS_Type * p_spis_reg;
    uint8_t spis_drv_inst_idx;

} periph_def_t;

typedef struct
{
    uint8_t mode;
    nrf_spis_mode_t spis_mode;
} line_mode_def_t;

typedef struct
{
    hal_mcu_spi_bit_order_t bit_order;
    nrf_spis_bit_order_t spis_bit_order;
} bit_order_def_t;

typedef struct
{
    /* Event handlers */
    void * p_instance;
    hal_mcu_spi_slave_buffers_set_done_handler_t buffers_set_done_handler;
    hal_mcu_spi_slave_transfer_done_handler_t transfer_done_handler;
} slave_t;

struct hal_mcu_spi
{
    hal_mcu_spi_role_t role;

    void * p_nrf_instance;      /* Slave  - nrfx_spis_t */

    const periph_def_t * p_periph_def;

    bool_t reserved;
    bool_t busy;

    /* Lock */
    hal_mcu_spi_lock_t lock;

    slave_t slave;
};

/* SPI peripherals */
#if NRFX_CHECK(NRFX_SPIS0_ENABLED)
static hal_mcu_spi_t * p_spi0 = NULL;
#endif /* NRFX_CHECK(NRFX_SPIS0_ENABLED) */

#if NRFX_CHECK(NRFX_SPIS1_ENABLED)
static hal_mcu_spi_t * p_spi1 = NULL;
#endif /* NRFX_CHECK(NRFX_SPIS1_ENABLED) */

#if NRFX_CHECK(NRFX_SPIS2_ENABLED)
static hal_mcu_spi_t * p_spi2 = NULL;
#endif /* NRFX_CHECK(NRFX_SPIS2_ENABLED) */

#if NRFX_CHECK(NRFX_SPIS3_ENABLED)
static hal_mcu_spi_t * p_spi3 = NULL;
#endif /* NRFX_CHECK(NRFX_SPIS3_ENABLED) */

/**************************************************************************/
/*                   SPI peripheral definitions - Slave                   */
/**************************************************************************/

#if NRFX_CHECK(NRFX_SPIS_ENABLED)
static const periph_def_t _periph_def_slave_array[] =
{

#if NRFX_CHECK(NRFX_SPIS0_ENABLED)
    { .def = HAL_MCU_SPI_PERIPH_DEF_SPI0, .pp_periph = &p_spi0,
            .p_spis_reg = NRF_SPIS0, .spis_drv_inst_idx = NRFX_SPIS0_INST_IDX },
#endif

#if NRFX_CHECK(NRFX_SPIS1_ENABLED)
    { .def = HAL_MCU_SPI_PERIPH_DEF_SPI1, .pp_periph = &p_spi1,
            .p_spis_reg = NRF_SPIS1, .spis_drv_inst_idx = NRFX_SPIS1_INST_IDX },
#endif

#if NRFX_CHECK(NRFX_SPIS2_ENABLED)
    { .def = HAL_MCU_SPI_PERIPH_DEF_SPI2, .pp_periph = &p_spi2,
            .p_spis_reg = NRF_SPIS2, .spis_drv_inst_idx = NRFX_SPIS2_INST_IDX },
#endif

#if NRFX_CHECK(NRFX_SPIS3_ENABLED)
    { .def = HAL_MCU_SPI_PERIPH_DEF_SPI3, .pp_periph = &p_spi3,
            .p_spis_reg = NRF_SPIS3, .spis_drv_inst_idx = NRFX_SPIS3_INST_IDX },
#endif
};
#define get_periph_def_slave( p_periph_def, id ) _get_def( p_periph_def, _periph_def_slave_array, periph_def_t, def, id )

#else /* NRFX_CHECK(NRFX_SPIS_ENABLED) */

#define get_periph_def_slave( p_periph_def, id ) NULL

#endif /* NRFX_CHECK(NRFX_SPIS_ENABLED) */


static const line_mode_def_t _line_mode_def_array[] =
{
    { .mode = LINE_MODE_0, .spis_mode = NRF_SPIS_MODE_0 },
    { .mode = LINE_MODE_1, .spis_mode = NRF_SPIS_MODE_1 },
    { .mode = LINE_MODE_2, .spis_mode = NRF_SPIS_MODE_2 },
    { .mode = LINE_MODE_3, .spis_mode = NRF_SPIS_MODE_3 },
};
#define get_line_mode_def( p_line_mode_def, id ) _get_def( p_line_mode_def, _line_mode_def_array, line_mode_def_t, mode, id )

/* SPI Bit Order definitions */
static const bit_order_def_t p_bit_order_def_array[] =
{
    { .bit_order = HAL_MCU_SPI_BIT_ORDER_LSB_FIRST, .spis_bit_order = NRF_SPIS_BIT_ORDER_LSB_FIRST },
    { .bit_order = HAL_MCU_SPI_BIT_ORDER_MSB_FIRST, .spis_bit_order = NRF_SPIS_BIT_ORDER_MSB_FIRST },
};
#define get_bit_order_def( p_bit_order_def, id ) _get_def( p_bit_order_def, p_bit_order_def_array, bit_order_def_t, bit_order, id )


/* Static prototypes */

#if NRFX_CHECK(NRFX_SPIS_ENABLED)
static result_t _slave_init( hal_mcu_spi_t * p_spi, const hal_mcu_spi_conf_t * p_conf );
static result_t _slave_line_configure( hal_mcu_spi_t * p_spi, const hal_mcu_spi_line_conf_t * p_line_conf );
static result_t _slave_data_transfer( hal_mcu_spi_t * p_spi, const hal_mcu_spi_transfer_conf_t * p_transfer_conf );
#else /* NRFX_CHECK(NRFX_SPIS_ENABLED) */
#define _slave_init( p_spi, p_conf ) RESULT_ERR
#define _slave_line_configure( p_spi, p_line_conf ) RESULT_ERR
#define _slave_data_transfer( p_spi, p_transfer_conf ) RESULT_ERR
#endif /* NRFX_CHECK(NRFX_SPIS_ENABLED) */

static INLINE const periph_def_t * _get_periph_def( const hal_mcu_spi_conf_t * p_conf )
{
    const periph_def_t * p_periph_def = NULL;

#if NRFX_CHECK(NRFX_SPIS_ENABLED)
    if( p_conf->role == HAL_MCU_SPI_ROLE_SLAVE )
    {
        get_periph_def_slave( p_periph_def, p_conf->def );
    }
#endif /* NRFX_CHECK(NRFX_SPIS_ENABLED) */

    return p_periph_def;
}

static result_t _spi_init( hal_mcu_spi_t * p_spi, const hal_mcu_spi_conf_t * p_conf )
{
    result_t result = RESULT_ERR;

    /* Initialize SPI */
    p_spi->role = p_conf->role;
    p_spi->reserved = false;
    p_spi->busy = false;

    /* Lock */
    p_spi->lock = 0;

    switch( p_spi->role )
    {
        case HAL_MCU_SPI_ROLE_SLAVE:

            result = _slave_init( p_spi, p_conf );
            EXIT_IF_ERR( result, "_slave_init failed" );

            break;

        default:

            ASSERT_DYGMA( false, "Invalid SPI role selection." );

            break;
    }

_EXIT:
    return result;
}

static result_t _spi_line_configure( hal_mcu_spi_t * p_spi, const hal_mcu_spi_line_conf_t * p_line_conf )
{
    result_t result = RESULT_ERR;

    switch( p_spi->role )
    {
        case HAL_MCU_SPI_ROLE_SLAVE:

            result = _slave_line_configure( p_spi, p_line_conf );
            EXIT_IF_ERR( result, "_slave_line_configure failed" );

            break;

        default:

            ASSERT_DYGMA( false, "Invalid SPI role selection." );

            break;
    }

_EXIT:
    return result;
}

result_t hal_ll_mcu_spi_init( hal_mcu_spi_t ** pp_spi, const hal_mcu_spi_conf_t * p_conf )
{
    hal_mcu_spi_t * p_spi;
    const periph_def_t * p_periph_def;
    result_t result = RESULT_ERR;

    /* Get the peripheral definition */
    p_periph_def = _get_periph_def( p_conf );
    ASSERT_DYGMA( p_periph_def != NULL, "invalid periph definition" );

    /* Check the init request validity */
    ASSERT_DYGMA( *p_periph_def->pp_periph == NULL, "Chosen SPI peripheral has already been initialized" );

    /* Allocate the peripheral */
    *p_periph_def->pp_periph = heap_alloc( sizeof(hal_mcu_spi_t) );
    p_spi = *p_periph_def->pp_periph;

    p_spi->p_periph_def = p_periph_def;

    /* initialize spi */
    result = _spi_init( p_spi, p_conf );
    EXIT_IF_ERR( result, "SPI init failed" );

    *pp_spi = p_spi;

_EXIT:
    return result;

}

bool_t hal_ll_mcu_spi_is_slave( hal_mcu_spi_t * p_spi )
{
    return ( p_spi->role == HAL_MCU_SPI_ROLE_SLAVE ) ? true : false;
}

result_t hal_ll_mcu_spi_reserve( hal_mcu_spi_t * p_spi, const hal_mcu_spi_line_conf_t * p_line_conf, hal_mcu_spi_lock_t * p_lock )
{
    result_t result = RESULT_ERR;

    if( p_spi->reserved == true )
    {
        return RESULT_BUSY;
    }

    /* Try to configure the SPI line */
    result = _spi_line_configure( p_spi, p_line_conf );
    EXIT_IF_ERR( result, "_spi_line_configure failed" );

    /* Reserve and lock the SPI peripheral */
    p_spi->reserved = true;

    p_spi->lock++;
    *p_lock = p_spi->lock;

_EXIT:
    return RESULT_OK;
}

void hal_ll_mcu_spi_release( hal_mcu_spi_t * p_spi, hal_mcu_spi_lock_t lock )
{
    /* Try to unlock the SPI peripheral */
    if( lock != p_spi->lock )
    {
        ASSERT_DYGMA( false, "Detected an attempt to unlock the SPI with a wrong lock ID" );
        return;
    }

    p_spi->reserved = false;
}

result_t hal_ll_mcu_spi_data_transfer( hal_mcu_spi_t * p_spi, const hal_mcu_spi_transfer_conf_t * p_transfer_conf )
{
    result_t result = RESULT_ERR;

    switch( p_spi->role )
    {
        case HAL_MCU_SPI_ROLE_SLAVE:

            result = _slave_data_transfer( p_spi, p_transfer_conf );
            EXIT_IF_ERR( result, "_slave_data_transfer failed" );

            break;

        default:

            ASSERT_DYGMA( false, "Invalid SPI role selection." );

            break;
    }

_EXIT:
    return result;
}

//*********************
//* Slave processing  *
//*********************

#if NRFX_CHECK(NRFX_SPIS_ENABLED)

static INLINE void _slave_buffers_set_done_handler( hal_mcu_spi_t * p_spi )
{
    if ( p_spi->slave.buffers_set_done_handler == NULL )
    {
        return;
    }

    p_spi->slave.buffers_set_done_handler( p_spi->slave.p_instance );
}

static INLINE void _slave_transfer_done_handler( hal_mcu_spi_t * p_spi, hal_mcu_spi_transfer_result_t * _transfer_result )
{
    if ( p_spi->slave.transfer_done_handler == NULL )
    {
        return;
    }

    p_spi->slave.transfer_done_handler( p_spi->slave.p_instance, _transfer_result );
}

static void _slave_event_handler( nrfx_spis_evt_t const * p_event, hal_mcu_spi_t * p_spi )
{
    hal_mcu_spi_transfer_result_t transfer_result;

    switch( p_event->evt_type )
    {
        case NRFX_SPIS_BUFFERS_SET_DONE:

            _slave_buffers_set_done_handler( p_spi );

            break;

        case NRFX_SPIS_XFER_DONE:

            transfer_result.data_in_len = p_event->rx_amount;
            transfer_result.data_out_len = p_event->tx_amount;

            p_spi->busy = false;
            _slave_transfer_done_handler( p_spi, &transfer_result );

            break;

        case NRFX_SPIS_EVT_TYPE_MAX:
            break;

        default:
            break;
    }
}

static result_t _slave_init( hal_mcu_spi_t * p_spi, const hal_mcu_spi_conf_t * p_conf )
{
    nrfx_spis_t * p_spis_instance;
    nrfx_spis_config_t spis_config = NRFX_SPIS_DEFAULT_CONFIG;
    const hal_mcu_spi_slave_conf_t * p_slave_conf = &p_conf->slave;

    result_t result = RESULT_OK;
    nrfx_err_t nrfx_err;

    /* Prepare the NRF spis instance */
    p_spi->p_nrf_instance = heap_alloc( sizeof(nrfx_spis_t) );
    p_spis_instance = ( nrfx_spis_t* )p_spi->p_nrf_instance;

    p_spis_instance->p_reg = p_spi->p_periph_def->p_spis_reg;
    p_spis_instance->drv_inst_idx = p_spi->p_periph_def->spis_drv_inst_idx;

    /* Prepare the SPI configuration */
    spis_config.csn_pin  = p_slave_conf->pin_cs;
    spis_config.miso_pin = p_slave_conf->pin_miso;
    spis_config.mosi_pin = p_slave_conf->pin_mosi;
    spis_config.sck_pin  = p_slave_conf->pin_sck;

    /* These configuration parameters are processed in _spi_line_configure */
    //spis_config.mode = ???
    //spis_config.bit_order = ???

    nrfx_err = nrfx_spis_init( p_spis_instance, &spis_config,
                               (nrfx_spis_event_handler_t)_slave_event_handler, p_spi );

    if ( nrfx_err != NRFX_SUCCESS )
    {
        result = RESULT_ERR;
        EXIT_IF_ERR( result, "nrf_drv_spis_init failed" );
    }

    result = _spi_line_configure( p_spi, &p_conf->line );
    EXIT_IF_ERR( result, "_spi_line_configure failed" );

_EXIT:
    return result;
}

static result_t _slave_line_configure( hal_mcu_spi_t * p_spi, const hal_mcu_spi_line_conf_t * p_line_conf )
{
    NRF_SPIS_Type * p_spis_reg;
    const line_mode_def_t * p_line_mode_def;
    const bit_order_def_t * p_bit_order_def;

    /* Get the line definitions */
    get_line_mode_def( p_line_mode_def, LINE_MODE_ENCODE( p_line_conf->cpol, p_line_conf->cpha) );
    get_bit_order_def( p_bit_order_def, p_line_conf->bit_order );

    ASSERT_DYGMA( p_line_mode_def != NULL, "The SPI Line Mode configuration is not valid" );
    ASSERT_DYGMA( p_bit_order_def != NULL, "The SPI Bit Order line configuration is not valid" );

    /* Get the SPIS register */
    p_spis_reg = p_spi->p_periph_def->p_spis_reg;

    /* Configure the SPI line*/
    nrf_spis_configure( p_spis_reg, p_line_mode_def->spis_mode, p_bit_order_def->spis_bit_order );

    return RESULT_OK;
}

static result_t _slave_data_transfer( hal_mcu_spi_t * p_spi, const hal_mcu_spi_transfer_conf_t * p_transfer_conf )
{
    nrfx_err_t nrfx_err;
    nrfx_spis_t * p_spis_instance = ( nrfx_spis_t* )p_spi->p_nrf_instance;

    ASSERT_DYGMA( p_spi->role = HAL_MCU_SPI_ROLE_SLAVE, "Only SPI slave may initialize the SPI transfer." );

    if( p_spi->busy == true )
    {
        return RESULT_BUSY;
    }
    p_spi->busy = true;

    /* Prepare the transfer event handlers */
    p_spi->slave.p_instance = p_transfer_conf->slave_handlers.p_instance;
    p_spi->slave.buffers_set_done_handler = p_transfer_conf->slave_handlers.buffers_set_done_handler;
    p_spi->slave.transfer_done_handler = p_transfer_conf->slave_handlers.transfer_done_handler;

    nrfx_err = nrfx_spis_buffers_set( p_spis_instance, p_transfer_conf->p_data_out, p_transfer_conf->data_out_len,
                                                      p_transfer_conf->p_data_in, p_transfer_conf->data_in_len );
    if ( nrfx_err != NRFX_SUCCESS )
    {
        p_spi->busy = false;
        return RESULT_ERR;
    }

    return RESULT_OK;
}

#endif /* NRFX_CHECK(NRFX_SPIS_ENABLED) */

#endif /* HAL_CFG_MCU_SERIES */

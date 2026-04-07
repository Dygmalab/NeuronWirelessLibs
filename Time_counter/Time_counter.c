/*
 * Time Counter -- Class to count time in milliseconds and microseconds.
 *
 * You can set the resolution in microseconds, which is not recommended
 * to be less than 50us or 100us due to the error caused by the delay
 * due to the execution of the code itself.
 *
 * You can set the timer used and the capture/compare unit used through
 * the corresponding #defines. You can also set the counter frequency if you wish.
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
 */

#include "nrf_drv_timer.h"
#include "Time_counter.h"

// Set parameters:
#define TIMER_NUMBER                    4                       /* Timer used for running the system timer */
#define TIMER_OVFLW_CHANNEL             NRF_TIMER_CC_CHANNEL0   /* CC channel used for controlling the base timer cycle and generate event when the timer rotates back to 0 */
#define TIMER_OVFLW_INTENSET            TIMER_INTENSET_COMPARE0_Msk
#define TIMER_OVFLW_INTENCLR            TIMER_INTENCLR_COMPARE0_Msk
#define TIMER_OVFLW_EVENT_TYPE          NRF_TIMER_EVENT_COMPARE0
#define TIMER_OVFLW_SHORT_CLEAR         NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK

#define TIMER_COMPARE_CHANNEL           NRF_TIMER_CC_CHANNEL1   /* CC channel used for controlling the regular timeouts within the base timer cycle */
#define TIMER_COMPARE_INTENSET          TIMER_INTENSET_COMPARE1_Msk
#define TIMER_COMPARE_INTENCLR          TIMER_INTENCLR_COMPARE1_Msk
#define TIMER_COMPARE_EVENT_TYPE        NRF_TIMER_EVENT_COMPARE1

#define TIMER_CAPTURE_CHANNEL           NRF_TIMER_CC_CHANNEL2   /* CC channel used for reading the actual value of the timmer */

#define TIMER_FREQUENCY                 NRF_TIMER_FREQ_16MHz
#define TIMER_FREQUENCY_IN_MHZ          16

#define TIMER_SYSTIM_TICK_LSB_MASK      0x7FFFFFFF              /* This is the mask of the hardware counter register value */
#define TIMER_SYSTIM_TICK_OVFLW_VAL     0x80000000              /* The value of the tick counter if it did not overflow to 0x00000000 */

#define SYSTIM_US_TO_TICK_CNT( us )     ( us * TIMER_FREQUENCY_IN_MHZ )
#define SYSTIM_MS_TO_TICK_CNT( ms )     SYSTIM_US_TO_TICK_CNT( ms * 1000 )

#define SYSTIM_TICK_CNT_TO_US(ticks)    ( ticks / TIMER_FREQUENCY_IN_MHZ )
#define SYSTIM_TICK_CNT_TO_MS(ticks)    ( SYSTIM_TICK_CNT_TO_US( ticks ) / 1000 )

#define SYSTIM_THRESHOLD_DIFFERENCE_MS_SAFE       100           //Difference between current and next time thresholds to recognize it as safe
#define SYSTIM_THRESHOLD_DIFFERENCE_MS_MIN        1             //Minimum difference between the last time threshold and the next one

static bool_t initialized = false;
static systim_tick_t systim_ticks;   //the number of ticks since the system start

static systim_tick_t systim_current_threshold;
static systim_tick_t systim_next_threshold;

static systim_tick_t systim_difference_save;
static systim_tick_t systim_difference_min;

static nrf_drv_timer_t driver_timer = NRF_DRV_TIMER_INSTANCE(TIMER_NUMBER);

// Prototypes
static void _systim_ticks_update( void );

void _interrupt_enable( void )
{
    ASSERT_DYGMA( initialized == true, "Systim has not been initialized yet." );

    driver_timer.p_reg->INTENSET = ( TIMER_OVFLW_INTENSET | TIMER_COMPARE_INTENSET );
}

void _interrupt_disable( void )
{
    ASSERT_DYGMA( initialized == true, "Systim has not been initialized yet." );

    driver_timer.p_reg->INTENCLR = ( TIMER_OVFLW_INTENCLR | TIMER_COMPARE_INTENCLR );
}

static void _systim_ticks_update( void )
{
    ASSERT_DYGMA( initialized == true, "Systim has not been initialized yet." );

    //stop interrupts
    _interrupt_disable( );

    uint32_t mcu_systim_ticks = nrf_drv_timer_capture( &driver_timer, TIMER_CAPTURE_CHANNEL );

    //clean the LSB bits
    systim_ticks &= ~TIMER_SYSTIM_TICK_LSB_MASK;
    //fill the LSB bits with new value
    systim_ticks |= ( uint32_t )mcu_systim_ticks;

    //resume interrupts
    _interrupt_enable( );
}

//NOTE: This function needs to be surrounded with Interrupt Enable/Disable if it is called outside of the interrupt
static void _systim_threshold_activate_current( void )
{
    systim_tick_t new_threshold;

    _systim_ticks_update( );

    new_threshold = systim_current_threshold;

    //check whether the threshold can be activated
    if ( systim_current_threshold == 0 )
        return;
    if ( systim_current_threshold <= systim_ticks + systim_difference_min )
    {
        new_threshold = systim_ticks + systim_difference_min;
    }
    if ( ( new_threshold - systim_ticks ) >= TIMER_SYSTIM_TICK_OVFLW_VAL )
        return;    //OVERFLOW interrupt will happen sooner

    //set the new threshold
    nrf_drv_timer_compare( &driver_timer, TIMER_COMPARE_CHANNEL, ( uint32_t )( new_threshold & TIMER_SYSTIM_TICK_LSB_MASK ), true );
}

static void _systim_threshold_activate_next( void )
{
    //compute the difference between the thresholds
    systim_tick_t threshold_difference = systim_next_threshold - systim_current_threshold;

    //move the next threshold to the current threshold position
    systim_current_threshold = systim_next_threshold;

    //set the next threshold
    if ( systim_next_threshold == 0 )
    {
        // there was no valid next threshold
    }
    else if ( threshold_difference < systim_difference_save )
    {
        // the threshold difference is not safe. Set backup threshold and thus wake up yet once to prevent system stuck if
        // a former threshold gets lost in the main system loop
        systim_next_threshold += systim_difference_save;
    }
    else
    {
        // the threshold difference is safe, there is no need for other threshold at this point
        systim_next_threshold = 0;
    }

    _systim_threshold_activate_current( );
}

//NOTE: This function needs to be surrounded with Interrupt Enable/Disable if it is called outside of the interrupt
static void _systim_threshold_set( const dl_timer_t * p_timer )
{
    //check whether the threshold is valid
    if ( *p_timer <= systim_ticks )
        return;

    //compare the new threshold with the current active one
    if ( ( systim_current_threshold == 0 ) || *p_timer < systim_current_threshold )
    {

        //shift the current threshold to the "next" position
        systim_next_threshold = systim_current_threshold;
        //set new "current" threshold
        systim_current_threshold = *p_timer;
        //activate the current threshold
        _systim_threshold_activate_current( );
    }
    else if ( *p_timer != systim_current_threshold && *p_timer < systim_next_threshold )
    {
        //set new "next" threshold. (forget the previous one)
        systim_next_threshold = *p_timer;
    }
}

static void _timer_set_threshold( dl_timer_t * p_timer, systim_tick_t threshold )
{
    //stop interrupts
    _interrupt_disable( );

    *p_timer = threshold;

    //resume interrupts
    _interrupt_enable( );

    //update the system ticks value
    _systim_ticks_update( );

    //stop interrupts
    _interrupt_disable( );
    //try to set new threshold
    _systim_threshold_set( p_timer );
    //resume interrupts
    _interrupt_enable( );
}

static void _timer_set_ticks( dl_timer_t * p_timer, systim_tick_t ticks )
{
    systim_tick_t threshold;

    _systim_ticks_update( );

    //stop interrupts
    _interrupt_disable( );

    //compute the target threshold value in number of ticks
    threshold = systim_ticks + ticks;

    //resume interrupts
    _interrupt_enable( );

    _timer_set_threshold( p_timer, threshold );
}

// Handler for driver_timer events.
static void _nrf_drv_event_handler(nrf_timer_event_t event_type, void *p_context)
{
    switch( event_type )
    {
        case TIMER_COMPARE_EVENT_TYPE:

            _systim_ticks_update( );
            _systim_threshold_activate_next( );

            break;

        case TIMER_OVFLW_EVENT_TYPE:

            systim_ticks &= ~TIMER_SYSTIM_TICK_LSB_MASK;
            systim_ticks += TIMER_SYSTIM_TICK_OVFLW_VAL;

            break;

        default:

            ASSERT_DYGMA( false, "Unknown systim HAL event not handled." );

            break;
    }
}

/*
 * NOTE: The micros_resolution_ parameter is not used actually. It is kept to be compatible with other projects which use the value in their space
 */
void timer_counter_init(uint32_t micros_resolution_)
{
    static bool already_inited = false;

    if (already_inited)
    {
        return;
    }
    already_inited = true;

    /* Initialize the system_ticks */
    systim_ticks = 0;

    systim_current_threshold = 0;
    systim_next_threshold = 0;

    systim_difference_save = SYSTIM_MS_TO_TICK_CNT( SYSTIM_THRESHOLD_DIFFERENCE_MS_SAFE );
    systim_difference_min = SYSTIM_MS_TO_TICK_CNT( SYSTIM_THRESHOLD_DIFFERENCE_MS_MIN );

    nrf_drv_timer_config_t timer_config;

    /*
        The "frequency" parameter here is actually the prescaler value, and the
        timer runs at the following frequency: f = 16MHz / 2^prescaler.
    */
    timer_config.frequency = TIMER_FREQUENCY;
    timer_config.mode = NRF_TIMER_MODE_TIMER;        // NRF_TIMER_MODE_COUNTER
    timer_config.bit_width = NRF_TIMER_BIT_WIDTH_32; // NRF_TIMER_BIT_WIDTH_8, NRF_TIMER_BIT_WIDTH_16, NRF_TIMER_BIT_WIDTH_24, NRF_TIMER_BIT_WIDTH_32
    timer_config.interrupt_priority = APP_IRQ_PRIORITY_LOW_MID;
    timer_config.p_context = NULL;

    ret_code_t ret = nrf_drv_timer_init(&driver_timer, &timer_config, _nrf_drv_event_handler);
    APP_ERROR_CHECK(ret);

    nrf_drv_timer_extended_compare(&driver_timer, TIMER_OVFLW_CHANNEL, TIMER_SYSTIM_TICK_OVFLW_VAL, TIMER_OVFLW_SHORT_CLEAR, true);

    nrf_drv_timer_enable(&driver_timer);

    initialized = true;
}

systim_tick_t timer_counter_get_micros(void)
{
    _systim_ticks_update( );

    return SYSTIM_TICK_CNT_TO_US( systim_ticks );
}

uint32_t timer_counter_get_millis(void)
{
    _systim_ticks_update( );

    return SYSTIM_TICK_CNT_TO_MS( systim_ticks );
}

void timer_set_ms( dl_timer_t * p_timer, uint32_t ms )
{
    systim_tick_t timeout_ticks = SYSTIM_MS_TO_TICK_CNT( ms );

    _timer_set_ticks( p_timer, timeout_ticks );
}

void timer_set_us( dl_timer_t * p_timer, uint32_t us )
{
    systim_tick_t timeout_ticks = SYSTIM_US_TO_TICK_CNT( us );

    _timer_set_ticks( p_timer, timeout_ticks );
}

bool timer_check( dl_timer_t * p_timer )
{
    return ( systim_ticks >= *p_timer ) ? true : false;
}

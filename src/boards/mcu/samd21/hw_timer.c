/**
* \file  hw_timer.c
*
* \brief Wrapper used by sw_timer utility using ASF timer api's
*
* Copyright (C) 2016 Atmel Corporation. All rights reserved.
*
* \asf_license_start
*
* \page License
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
* 3. The name of Atmel may not be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
* 4. This software may only be redistributed and used in connection with an
*    Atmel microcontroller product.
*
* THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
* EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
* OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
* ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
* \asf_license_stop
*
* Modified for use with Atmel START hpl_rtc functions
*/

/**************************************** INCLUDES*****************************/
#include <stdint.h>
#include <stdbool.h>
#include <utils_assert.h>
#include <utils.h>
#include <hal_atomic.h>
#include <hpl_irq.h>

#include <hpl_pm_base.h>
#include <hpl_gclk_base.h>
#include <peripheral_clk_config.h>
#include <hpl_rtc_config.h>

#include "board-config.h"
#include "gpio.h"
#include "hw_timer.h"


/**************************************** MACROS*****************************/
//#define USE_HWTMR_DEBUG

#define COMPARE_COUNT_MAX_VALUE ( uint32_t )( -1 )

/**************************************** GLOBALS*****************************/

HwTimerCallback_t HwTimerAlarmCallback = NULL;
HwTimerCallback_t HwTimerOverflowCallback = NULL;

#if defined( USE_HWTMR_DEBUG )
Gpio_t DbgHwTmrPin;
#endif

/************************************** IMPLEMENTATION************************/

/**
* \brief Initializes the hw timer module
*/
void HwTimerInit(void)
{
#if defined( USE_HWTMR_DEBUG )
    GpioInit( &DbgHwTmrPin, HWTMR_DBG_PIN_0, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
#endif

    // enable clock
	_pm_enable_bus_clock(PM_BUS_APBA, RTC);
	_gclk_enable_channel(RTC_GCLK_ID, CONF_GCLK_RTC_SRC);

    // configure timer
	hri_rtcmode0_write_CTRL_reg(RTC, RTC_MODE0_CTRL_SWRST);
	hri_rtcmode0_wait_for_sync(RTC);
	hri_rtcmode0_write_CTRL_reg(RTC,
      RTC_MODE0_CTRL_MODE(0) |
      RTC_MODE0_CTRL_PRESCALER(CONF_RTC_PRESCALER) );
	hri_rtcmode0_write_COMP_COMP_bf(RTC, 0, (uint32_t)COMPARE_COUNT_MAX_VALUE);
	hri_rtcmode0_set_INTEN_CMP0_bit(RTC);

    // start the timer
	NVIC_EnableIRQ(RTC_IRQn);
	hri_rtcmode0_write_COUNT_COUNT_bf(RTC, 0);
	hri_rtcmode0_wait_for_sync(RTC);
	hri_rtcmode0_set_CTRL_ENABLE_bit(RTC);
}

/**
* \brief This function is used to set the callback when the hw timer
* expires.
* \param callback Callback to be registered
*/
void HwTimerAlarmSetCallback(HwTimerCallback_t callback)
{
    HwTimerAlarmCallback = callback;
}

/**
* \brief This function is used to set the callback when the hw timer
* overflows.
* \param callback Callback to be registered
*/
void HwTimerOverflowSetCallback(HwTimerCallback_t callback)
{
    HwTimerOverflowCallback = callback;
}

/**
* \brief Loads the timeout in terms of ticks into the hardware
* \ticks Time value in terms of timer ticks
*/
bool HwTimerLoadAbsoluteTicks(uint32_t ticks)
{
#if defined( USE_HWTMR_DEBUG )
    GpioWrite( &DbgHwTmrPin, 1 );
#endif

    RTC_CRITICAL_SECTION_ENTER();
    hri_rtcmode0_write_COMP_reg(RTC, 0, ticks);
   	hri_rtcmode0_wait_for_sync(RTC);
    uint32_t current = hri_rtcmode0_read_COUNT_reg(RTC);
    RTC_CRITICAL_SECTION_LEAVE();

    if((ticks - current - 1) >= (COMPARE_COUNT_MAX_VALUE >> 1)) {
        // if difference is more than half of max assume timer has passed
        return false;
    }
    if((ticks - current) < 10) {
        // if too close the matching interrupt does not trigger, so handle same as passed
        return false;
    }
    return true;
}

/**
* \brief Gets the absolute time value
* \retval Absolute time in ticks
*/
uint32_t HwTimerGetTime(void)
{
   	hri_rtcmode0_wait_for_sync(RTC);
    return hri_rtcmode0_read_COUNT_reg(RTC);
}

/**
* \brief Disables the hw timer module
*/
void HwTimerDisable(void)
{

}

/**
* \brief Rtc interrupt handler
*/
void RTC_Handler(void)
{
    /* Read and mask interrupt flag register */
    uint16_t flag = hri_rtcmode0_read_INTFLAG_reg(RTC);

    if (flag & RTC_MODE0_INTFLAG_CMP0) {
#if defined( USE_HWTMR_DEBUG )
        GpioWrite( &DbgHwTmrPin, 0 );
#endif
        hri_rtcmode0_clear_interrupt_CMP0_bit(RTC);
        if (HwTimerAlarmCallback != NULL) {
            HwTimerAlarmCallback();
        }
        /* Clear interrupt flag */
    }
    else if ( flag & RTC_MODE0_INTFLAG_OVF) {
        hri_rtcmode0_clear_interrupt_OVF_bit(RTC);
        if (HwTimerOverflowCallback != NULL) {
            HwTimerOverflowCallback();
        }
    }

}

/* eof hw_timer.c */

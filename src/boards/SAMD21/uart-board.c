/*!
 * \file      uart-board.c
 *
 * \brief     Target board UART driver implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 *
 * \author    Marten Lootsma(TWTG) on behalf of Microchip/Atmel (c)2017
 */
#include <peripheral_clk_config.h>
#include <hpl_pm_base.h>
#include <hpl_gclk_base.h>
#include <hal_gpio.h>

#include <hpl/usart/usart_lite.h>

#include "board.h"
#include "uart-board.h"

// struct usart_async_descriptor Usart0;
// static uint8_t rxBuffer[128];

void UartMcuInit( Uart_t *obj, uint8_t uartId, PinNames tx, PinNames rx )
{
    obj->UartId = uartId;

    // Clock initialization
	_pm_enable_bus_clock(PM_BUS_APBC, SERCOM0);
	_gclk_enable_channel(SERCOM0_GCLK_ID_CORE, CONF_GCLK_SERCOM0_CORE_SRC);

#if 0
    // USART initialization
    usart_async_init( &Usart0, SERCOM0, rxBuffer, sizeof(rxBuffer), ( void *) NULL);
#endif

    // UASRT GPIO initialization
    gpio_set_pin_function( tx, PINMUX_PA10C_SERCOM0_PAD2 );
    gpio_set_pin_function( rx, PINMUX_PA11C_SERCOM0_PAD3 );

    USART_0_init();

#if 0
    usart_async_enable( &Usart0 );
#endif
}

void UartMcuConfig( Uart_t *obj, UartMode_t mode, uint32_t baudrate, WordLength_t wordLength, StopBits_t stopBits, Parity_t parity, FlowCtrl_t flowCtrl )
{
    //usart_async_set_baud_rate( &Usart0, baudrate );
}

void UartMcuDeInit( Uart_t *obj )
{

}

uint8_t UartMcuPutChar( Uart_t *obj, uint8_t data )
{
#if 0
    if( io_write( &Usart0.io, &data, 1 ) == 0 )
    {
        return 1; // Busy
    }
#endif
    return 0; // OK
}

uint8_t UartMcuGetChar( Uart_t *obj, uint8_t *data )
{

    if (USART_0_is_byte_received()) {
        *data = USART_0_read_byte();
        return (0);
    } else {
        return (1);
    }

#if 0
    if( io_read( &Usart0.io, data, 1 ) == 1 )
    {
        return 0; // OK
    }
    else
    {
        return 1; // Busy
    }
#endif
    return (0);
}

uint8_t UartMcuPutBuffer( Uart_t *obj, uint8_t *buffer, uint16_t size )
{
#if 0
    if( io_write( &Usart0.io, buffer, size ) == 0 )
    {
        return 1; //Error
    }
#endif
    return 0; // OK
}

uint8_t UartMcuGetBuffer( Uart_t *obj, uint8_t *buffer, uint16_t size, uint16_t *nbReadBytes )
{
#if 0
    *nbReadBytes = io_read( &Usart0.io, buffer, size );
    if( *nbReadBytes == 0 )
    {
        return 1; // Empty
    }
#endif
    return 0; // OK
}

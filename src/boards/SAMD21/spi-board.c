/*!
 * \file      spi-board.c
 *
 * \brief     Target board SPI driver implementation
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
#include <hal_spi_m_sync.h>
#include <hal_gpio.h>

#include "spi-board.h"

struct spi_m_sync_descriptor Spi0;

void SpiInit( Spi_t *obj, SpiId_t spiId, PinNames mosi, PinNames miso, PinNames sclk, PinNames nss )
{
    // clock init
	_pm_enable_bus_clock(PM_BUS_APBC, SERCOM4);
	_gclk_enable_channel(SERCOM4_GCLK_ID_CORE, CONF_GCLK_SERCOM4_CORE_SRC);

    // SPI init
    spi_m_sync_init(&Spi0, SERCOM4);

    // pin init
	gpio_set_pin_direction(miso, GPIO_DIRECTION_IN);
	gpio_set_pin_pull_mode(miso,
	                       // <y> Pull configuration
	                       // <id> pad_pull_config
	                       // <GPIO_PULL_OFF"> Off
	                       // <GPIO_PULL_UP"> Pull-up
	                       // <GPIO_PULL_DOWN"> Pull-down
	                       GPIO_PULL_OFF);
	gpio_set_pin_function(miso, PINMUX_PA12D_SERCOM4_PAD0);

	gpio_set_pin_level(mosi,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);
	// Set pin direction to output
	gpio_set_pin_direction(mosi, GPIO_DIRECTION_OUT);
	gpio_set_pin_function(mosi, PINMUX_PB10D_SERCOM4_PAD2);

	gpio_set_pin_level(sclk,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);
	// Set pin direction to output
	gpio_set_pin_direction(sclk, GPIO_DIRECTION_OUT);
	gpio_set_pin_function(sclk, PINMUX_PB11D_SERCOM4_PAD3);

    // enable SPI
    spi_m_sync_enable( &Spi0 );
}

void SpiDeInit( Spi_t *obj )
{
    GpioInit( &obj->Mosi, obj->Mosi.pin, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &obj->Miso, obj->Miso.pin, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_DOWN, 0 );
    GpioInit( &obj->Sclk, obj->Sclk.pin, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &obj->Nss, obj->Nss.pin, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 1 );
}

uint16_t SpiInOut( Spi_t *obj, uint16_t outData )
{
    // Wait for bus idle (ready to write)
    while( ( SERCOM_SPI_INTFLAG_DRE & hri_sercomspi_read_INTFLAG_reg( SERCOM4 ) ) == 0 )
    {

    }
    hri_sercomspi_clear_INTFLAG_reg( SERCOM4, SERCOM_SPI_INTFLAG_DRE );

    // Write byte
    hri_sercomspi_write_DATA_reg( SERCOM4, outData );

    // Wait for ready to read
    while( ( SERCOM_SPI_INTFLAG_RXC & hri_sercomspi_read_INTFLAG_reg( SERCOM4 ) ) == 0 )
    {

    }
    hri_sercomspi_clear_INTFLAG_reg( SERCOM4, SERCOM_SPI_INTFLAG_RXC );

    // Read byte
    outData = ( uint16_t )hri_sercomspi_read_DATA_reg( SERCOM4 );

    return outData;
}

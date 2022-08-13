/*!
 * \file      sx1276-board.c
 *
 * \brief     Target board SX1276 driver implementation
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
#include <hal_ext_irq.h>
#include <hal_gpio.h>
#include "board-config.h"
#include "delay.h"
#include "radio.h"
#include "sx1276-board.h"

/*!
 * \brief Gets the board PA selection configuration
 *
 * \param [IN] channel Channel frequency in Hz
 * \retval PaSelect RegPaConfig PaSelect value
 */
static uint8_t SX1276GetPaSelect( uint32_t channel );

/*!
 * Flag used to set the RF switch control pins in low power mode when the radio is not active.
 */
static bool RadioIsActive = false;

/*!
 * Radio driver structure initialization
 */
const struct Radio_s Radio =
{
    SX1276Init,
    SX1276GetStatus,
    SX1276SetModem,
    SX1276SetChannel,
    SX1276IsChannelFree,
    SX1276Random,
    SX1276SetRxConfig,
    SX1276SetTxConfig,
    SX1276CheckRfFrequency,
    SX1276GetTimeOnAir,
    SX1276Send,
    SX1276SetSleep,
    SX1276SetStby,
    SX1276SetRx,
    SX1276StartCad,
    SX1276SetTxContinuousWave,
    SX1276ReadRssi,
    SX1276Write,
    SX1276Read,
    SX1276WriteBuffer,
    SX1276ReadBuffer,
    SX1276SetMaxPayloadLength,
    SX1276SetPublicNetwork,
    SX1276GetWakeupTime,
    NULL, // void ( *IrqProcess )( void )
    NULL, // void ( *RxBoosted )( uint32_t timeout ) - SX126x Only
    NULL, // void ( *SetRxDutyCycle )( uint32_t rxTime, uint32_t sleepTime ) - SX126x Only
};

/*!
 * Antenna switch GPIO pins objects
 */
Gpio_t rxen_pin;
Gpio_t txen_pin;

/*!
 * Debug GPIO pins objects
 */
#if defined( USE_RADIO_DEBUG )
Gpio_t DbgPinTx;
Gpio_t DbgPinRx;
#endif

void SX1276IoInit( void )
{
    GpioInit( &SX1276.Spi.Nss, RADIO_NSS, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &rxen_pin, RXEN_PIN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &txen_pin, TXEN_PIN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    ext_irq_init( );

    GpioInit( &SX1276.DIO0, RADIO_DIO_0, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    gpio_set_pin_function( RADIO_DIO_0, PINMUX_PB16A_EIC_EXTINT0 );
    GpioInit( &SX1276.DIO1, RADIO_DIO_1, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    // Must be setup to be trigged on both edges. See CONF_EIC_SENSE11 under hpl_eic_config.h
    gpio_set_pin_function( RADIO_DIO_1, PINMUX_PA11A_EIC_EXTINT11 );
    GpioInit( &SX1276.DIO2, RADIO_DIO_2, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    gpio_set_pin_function( RADIO_DIO_2, PINMUX_PA12A_EIC_EXTINT12 );
    // XXX:
}

static void Dio0IrqHandler( void );
static void Dio1IrqHandler( void );
static void Dio2IrqHandler( void );
static void Dio3IrqHandler( void );

static Gpio_t *DioIrqs[] = {
    &SX1276.DIO0,
    &SX1276.DIO1,
    &SX1276.DIO2,
    &SX1276.DIO3
};

static ext_irq_cb_t ExtIrqHandlers[] = {
    Dio0IrqHandler,
    Dio1IrqHandler,
    Dio2IrqHandler,
    Dio3IrqHandler
};

static void DioIrqHandlerProcess( uint8_t index )
{
    if( ( DioIrqs[index] != NULL ) && ( DioIrqs[index]->IrqHandler != NULL ) )
    {
        DioIrqs[index]->IrqHandler( DioIrqs[index]->Context );
    }
}

static void Dio0IrqHandler( void )
{
    DioIrqHandlerProcess( 0 );
}

static void Dio1IrqHandler( void )
{
    DioIrqHandlerProcess( 1 );
}

static void Dio2IrqHandler( void )
{
    DioIrqHandlerProcess( 2 );
}

static void Dio3IrqHandler( void )
{
    DioIrqHandlerProcess( 3 );
}

static void IoIrqInit( uint8_t index, DioIrqHandler *irqHandler )
{
    DioIrqs[index]->IrqHandler = irqHandler;
    ext_irq_register( DioIrqs[index]->pin, ExtIrqHandlers[index] );
}

void SX1276IoIrqInit( DioIrqHandler **irqHandlers )
{
    for( int8_t i = 0; i < 4; i++ )
    {
        IoIrqInit( i, irqHandlers[i] );
    }
}

void SX1276IoDeInit( void )
{
    GpioInit( &SX1276.Spi.Nss, RADIO_NSS, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 1 );
    GpioInit( &rxen_pin, RXEN_PIN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &txen_pin, TXEN_PIN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    ext_irq_init( );

    GpioInit( &SX1276.DIO0, RADIO_DIO_0, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    gpio_set_pin_function( RADIO_DIO_0, PINMUX_PB16A_EIC_EXTINT0 );
    GpioInit( &SX1276.DIO1, RADIO_DIO_1, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    gpio_set_pin_function( RADIO_DIO_1, PINMUX_PA11A_EIC_EXTINT11 );
    GpioInit( &SX1276.DIO2, RADIO_DIO_2, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    gpio_set_pin_function( RADIO_DIO_2, PINMUX_PA12A_EIC_EXTINT12 );
    // XXX:
}

void SX1276IoDbgInit( void )
{
#if defined( USE_RADIO_DEBUG )
    GpioInit( &DbgPinTx, RADIO_DBG_PIN_TX, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &DbgPinRx, RADIO_DBG_PIN_RX, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
#endif
}

void SX1276IoTcxoInit( void )
{
    // XXX: no TCXO
}

void SX1276SetBoardTcxo( uint8_t state )
{
    // XXX: no TCXO
}

uint32_t SX1276GetBoardTcxoWakeupTime( void )
{
    return BOARD_TCXO_WAKEUP_TIME;
}

void SX1276Reset( void )
{
    // Enables the TCXO if available on the board design
    SX1276SetBoardTcxo( true );

    // Set RESET pin to 0
    GpioInit( &SX1276.Reset, RADIO_RESET, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );

    // Wait 1 ms
    DelayMs( 1 );

    // Configure RESET as input
    GpioInit( &SX1276.Reset, RADIO_RESET, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );

    // Wait 6 ms
    DelayMs( 6 );
}

// XXX: add support for +10dB PA

void
SX1276SetRfTxPower( int8_t power )
{
    uint8_t paConfig = 0;
    uint8_t paDac = 0;

    paConfig = SX1276Read( REG_PACONFIG );
    paDac = SX1276Read( REG_PADAC );

    paConfig = ( paConfig & RF_PACONFIG_PASELECT_MASK ) | SX1276GetPaSelect( SX1276.Settings.Channel );

    if ( (paConfig & RF_PACONFIG_PASELECT_PABOOST) == RF_PACONFIG_PASELECT_PABOOST )
    {
        if( power > 17 ) {
            paDac = ( paDac & RF_PADAC_20DBM_MASK ) | RF_PADAC_20DBM_ON;
        } else {
            paDac = ( paDac & RF_PADAC_20DBM_MASK ) | RF_PADAC_20DBM_OFF;
        }
        if( ( paDac & RF_PADAC_20DBM_ON ) == RF_PADAC_20DBM_ON ) {
            if( power < 5 ) {
                power = 5;
            }
            if( power > 20 ) {
                power = 20;
            }
            paConfig = ( paConfig & RF_PACONFIG_OUTPUTPOWER_MASK ) | ( uint8_t )( ( uint16_t )( power - 5 ) & 0x0F );
        } else {
            if( power < 2 ) {
                power = 2;
            }
            if( power > 17 ) {
                power = 17;
            }
            paConfig = ( paConfig & RF_PACONFIG_OUTPUTPOWER_MASK ) | ( uint8_t )( ( uint16_t )( power - 2 ) & 0x0F );
        }
    } else {
        if( power > 0 )
        {
            if( power > 15 )
            {
                power = 15;
            }
            paConfig = ( paConfig & RF_PACONFIG_MAX_POWER_MASK & RF_PACONFIG_OUTPUTPOWER_MASK ) | ( 7 << 4 ) | ( power );
        } else {
            if( power < -4 )
            {
                power = -4;
            }
            paConfig = ( paConfig & RF_PACONFIG_MAX_POWER_MASK & RF_PACONFIG_OUTPUTPOWER_MASK ) | ( 0 << 4 ) | ( power + 4 );
        }
    }
    SX1276Write( REG_PACONFIG, paConfig );
    SX1276Write( REG_PADAC, paDac );
}

static uint8_t SX1276GetPaSelect( uint32_t channel )
{
    return RF_PACONFIG_PASELECT_PABOOST;
}

void SX1276SetAntSwLowPower( bool status )
{
    // XXX: need to init/deinit the control pins?
    // Control the TCXO and Antenna switch
    if( RadioIsActive != status )
    {
        RadioIsActive = status;
    }
}

void SX1276SetAntSw( uint8_t opMode )
{
    // RXEN/TXEN

    switch (opMode) {
    case RFLR_OPMODE_TRANSMITTER:
        GpioWrite( &rxen_pin, 0 );
        GpioWrite( &txen_pin, 1 );
        break;
    case RFLR_OPMODE_RECEIVER:
    case RFLR_OPMODE_RECEIVER_SINGLE:
    case RFLR_OPMODE_CAD:
    default:
        GpioWrite( &txen_pin, 0 );
        GpioWrite( &rxen_pin, 1 );
        break;
    }

}

bool SX1276CheckRfFrequency( uint32_t frequency )
{
    // Implement check. Currently all frequencies are supported
    return true;
}

uint32_t SX1276GetDio1PinState( void )
{
    return GpioRead( &SX1276.DIO1 );
}

#if defined( USE_RADIO_DEBUG )
void SX1276DbgPinTxWrite( uint8_t state )
{
    GpioWrite( &DbgPinTx, state );
}

void SX1276DbgPinRxWrite( uint8_t state )
{
    GpioWrite( &DbgPinRx, state );
}
#endif

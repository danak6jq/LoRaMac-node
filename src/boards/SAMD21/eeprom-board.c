/*!
 * \file      eeprom-board.c
 *
 * \brief     Target board EEPROM driver implementation
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
 */
#include "utilities.h"
#include "eeprom-board.h"

#include <peripheral_clk_config.h>
#include <hpl_gclk_base.h>
#include <hpl_pm_base.h>
#include <hal_flash.h>
#include <string.h>

struct flash_descriptor FLASH_0;

#define EEPROM_BASE (FLASH_SIZE - 4096UL)
#define PAGE_SIZE	64
#define ROW_SIZE	(PAGE_SIZE * 4)

static inline
uint32_t read_unaligned_uint32(const void *data)
{
	union {
		uint32_t u32;
		uint8_t u8[4];
	} res;
	const uint8_t *d = (const uint8_t *)data;
	res.u8[0] = d[0];
	res.u8[1] = d[1];
	res.u8[2] = d[2];
	res.u8[3] = d[3];
	return res.u32;
}

static void
write(const volatile void *flash_ptr, const void *data, uint32_t size)
{

	// Calculate data boundaries
	size = (size + 3) / 4;
	volatile uint32_t *dst_addr = (volatile uint32_t *)flash_ptr;
	const uint8_t *src_addr = (uint8_t *)data;

	// Disable automatic page write
	NVMCTRL->CTRLB.bit.MANW = 1;

	// Do writes in pages
	while (size) {
		// Execute "PBC" Page Buffer Clear

		NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_PBC;
		while (NVMCTRL->INTFLAG.bit.READY == 0) { }

		// Fill page buffer
		uint32_t i;
		for (i = 0; i < (PAGE_SIZE / 4) && size; i++) {
			*dst_addr = read_unaligned_uint32(src_addr);
			src_addr += 4;
			dst_addr++;
			size--;
		}

		// Execute "WP" Write Page

		NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_WP;
		while (NVMCTRL->INTFLAG.bit.READY == 0) { }
	}
}

static void
erase(const volatile void *flash_ptr, uint32_t size)
{
	const uint8_t *ptr = (const uint8_t *)flash_ptr;
	while (size > ROW_SIZE) {

		NVMCTRL->ADDR.reg = ((uint32_t)flash_ptr) / 2;
		NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_ER;
		
		while (!NVMCTRL->INTFLAG.bit.READY) { }
		
		ptr += ROW_SIZE;
		size -= ROW_SIZE;
	}
}

static void
read(const volatile void *flash_ptr, void *data, uint32_t size)
{
	memcpy(data, (const void *)flash_ptr, size);
}

LmnStatus_t EepromMcuWriteBuffer( uint16_t addr, uint8_t *buffer, uint16_t size )
{
    write((const void *)(addr + EEPROM_BASE), buffer, size);
    return LMN_STATUS_OK;
}

LmnStatus_t EepromMcuReadBuffer( uint16_t addr, uint8_t *buffer, uint16_t size )
{
    read((const void *)(addr + EEPROM_BASE), buffer, size);
    return LMN_STATUS_OK;
}

void EepromMcuSetDeviceAddr( uint8_t addr )
{
    while( 1 )
    {
    }
}

LmnStatus_t EepromMcuGetDeviceAddr( void )
{
    while( 1 )
    {
    }
//    return 0;
}



void
initFlash()
{
	_pm_enable_bus_clock(PM_BUS_APBB, NVMCTRL);
	flash_init(&FLASH_0, NVMCTRL);
}
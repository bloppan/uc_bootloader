/**
  ******************************************************************************
  * @file    flash_memory.c
  * @author  Bernardo López Panadero
  * @brief   Flash memory managment
  *
  ******************************************************************************
  */
#ifndef INC_FLASH_MEMORY_H_
#define INC_FLASH_MEMORY_H_

#include "stm32f1xx_hal.h"
#include <stdlib.h>
#include <string.h>

#define	ADDR_FLASH_PAGE_0		0x08000000U
#define	ADDR_FLASH_PAGE_1		0x08000400U
#define	ADDR_FLASH_PAGE_127		0x0801FC00U

#define ADDR_LAST_PAGE			ADDR_FLASH_PAGE_127

#define APP_ADDRESS 			0x0800E000				// Address
#define BOOT_FLAG				ADDR_LAST_PAGE
#define APP_MAX_SIZE			(APP_ADDRESS - ADDR_LAST_PAGE - FLASH_PAGE_SIZE)

struct flash_memory {

	uint32_t type_erase;
	uint32_t type_program;
	uint32_t addr;
	uint32_t dest_addr;
	uint32_t *data;
	uint32_t length;
	uint32_t page;
	uint32_t num_pages;
	uint32_t sector;
	uint32_t num_sectors;
};

_Bool 		flash_erase(struct flash_memory *flash_info);
_Bool		flash_write(struct flash_memory *flash_info);
void 	 	flash_read_bytes(int32_t addr, int32_t length, int8_t *dest);
uint32_t	flash_read_uint32(uint32_t addr);

#endif /* INC_FLASH_MEMORY_H_ */




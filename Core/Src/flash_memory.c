/**
  ******************************************************************************
  * @file    flash_memory.c
  * @author  Bernardo López Panadero
  * @brief   Flash memory management
  *
  ******************************************************************************
 */

#include "flash_memory.h"
#include <stdlib.h>
#include <string.h>

/*
 * Lock flash memory
 */
static void flash_lock(void)
{
	/* Delete all FLAGs */
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_BSY | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR );
	HAL_FLASH_Lock();
}

/*
 * Unlock flash memory
 */
static void flash_unlock(void)
{
	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_BSY | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR );
}

/*
 * Flash erase.
 */
_Bool flash_erase(struct flash_memory *flash_info){

	FLASH_EraseInitTypeDef FLASH_EraseInitStruct;
	uint32_t	SECTORError;
	_Bool		res = 0;

	FLASH_EraseInitStruct.TypeErase 	= flash_info->type_erase;
	FLASH_EraseInitStruct.PageAddress 	= flash_info->page;
	FLASH_EraseInitStruct.NbPages 		= flash_info->num_pages;

	flash_unlock();
	if (HAL_FLASHEx_Erase(&FLASH_EraseInitStruct, &SECTORError) == HAL_OK)
		res = 1;
	flash_lock();

	return res;
}

/*
 * Flash write
 */

_Bool flash_write(struct flash_memory *flash_info)
{
	_Bool res = 0;

    if (HAL_FLASH_Program(flash_info->type_program, flash_info->addr,
    						(*(uint32_t *)flash_info->data)) == HAL_OK)
		res = 1;

    return res;
}

/*
 * Read from addr and copy to *dest a specified length of bytes
 */
void flash_read_bytes(int32_t addr, int32_t length, int8_t *dest )
{
	int8_t *from = (int8_t *)addr;
	memcpy(dest, from, length);
}

/*
 * Read from addr a uint32 data type
 */
uint32_t flash_read_uint32(uint32_t addr)
{
	return (*(uint32_t *)addr);
}




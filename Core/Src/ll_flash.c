/*
 * ll_flash.c
 *
 *  Created on: Sep 8, 2022
 *      Author: quangcaol
 */

#include "ll_flash.h"

int8_t ll_flash_unlock()
{
	if (READ_BIT(FLASH->CR,FLASH_CR_LOCK) == RESET) return 0;
	FLASH->KEYR = 0x45670123;

	FLASH->KEYR = 0xCDEF89AB;

	if (READ_BIT(FLASH->CR,FLASH_CR_LOCK) == RESET) return 0;
	return -1;
}

int8_t ll_flash_lock()
{
	FLASH->CR |= FLASH_CR_LOCK;
	return 0;
}

int8_t ll_flash_erase(uint8_t sector)
{
	if (READ_BIT(FLASH->SR,FLASH_SR_BSY) == SET) return -1;

	CLEAR_BIT(FLASH->CR, FLASH_CR_PSIZE);
	FLASH->CR |= 0 << FLASH_CR_PSIZE_Pos; //x64 parallism


	CLEAR_BIT(FLASH->CR, FLASH_CR_SNB);
	FLASH->CR |= FLASH_CR_SER | (sector << FLASH_CR_SNB_Pos);

	FLASH->CR |= FLASH_CR_STRT;

	for (uint16_t i =0;(READ_BIT(FLASH->SR,FLASH_SR_BSY) == SET) || (i < 1000);i++);

	if ( READ_BIT(FLASH->SR,FLASH_SR_EOP) != RESET) CLEAR_BIT(FLASH->SR,FLASH_SR_EOP);

	return 1;
}

int8_t ll_flash_write(uint32_t PageAddr,const uint8_t * data, uint32_t size)
{
	if (READ_BIT(FLASH->SR,FLASH_SR_BSY) == SET) return -1;

	CLEAR_BIT(FLASH->CR, FLASH_CR_PSIZE);
	FLASH->CR |= 0 << FLASH_CR_PSIZE_Pos; //x8 parallism

	SET_BIT(FLASH->CR,FLASH_CR_PG);

	__O uint8_t * begin =(uint8_t *) PageAddr;

	for (uint32_t i =0; i < size ; ++i)
	{
		begin[i] = data[i];
		while (READ_BIT(FLASH->SR,FLASH_SR_BSY) == SET);
	}

	CLEAR_BIT(FLASH->CR,FLASH_CR_PG);

	return 0;
}

int8_t ll_flash_read(uint32_t PageAddr, uint8_t * data, uint32_t size)
{
	__I uint32_t * begin = (uint32_t *) PageAddr;

	for (uint32_t i = 0 ; i < size ; i++)
	{
		data[i] = *begin;
		begin++;
	}

	return 0;
}

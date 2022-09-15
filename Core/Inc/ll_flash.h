/*
 * ll_flash.h
 *
 *  Created on: Sep 8, 2022
 *      Author: quangcaol
 */

#ifndef INC_LL_FLASH_H_
#define INC_LL_FLASH_H_

#include "main.h"

#define FLASH_KEY1 0x45670123
#define FLASH_KEY2 0xCDEF89AB

int8_t ll_flash_unlock();

int8_t ll_flash_lock();

int8_t ll_flash_write(uint32_t PageAddr, uint8_t * data, uint16_t size);

int8_t ll_flash_read(uint32_t PageAddr, uint8_t * data, uint16_t size);

int8_t ll_flash_erase(uint8_t sector);



#endif /* INC_LL_FLASH_H_ */

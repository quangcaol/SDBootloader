/*
 * ll_flash.h
 *
 *  Created on: Sep 8, 2022
 *      Author: quangcaol
 */

#ifndef INC_LL_FLASH_H_
#define INC_LL_FLASH_H_

#include "main.h"

#define SECTOR_1	1
#define SECTOR_2    	2
#define SECTOR_3    	3
#define SECTOR_4	4
#define SECTOR_5 	5
#define SECTOR_6	6
#define SECTOR_7	7

/**
 * @brief unlock flash
 * 
 * @return int8_t 1 ok
 */
int8_t ll_flash_unlock();

/**
 * @brief lock flash
 * 
 * @return int8_t 1 ok
 */
int8_t ll_flash_lock();

/**
 * @brief Write 8 bit buffer to flash
 * 
 * @param PageAddr
 * @param data 
 * @param size 
 * @return int8_t 
 */
int8_t ll_flash_write(uint32_t PageAddr,const uint8_t * data, uint32_t size);

/**
 * @brief read 8 bits buffer from flash
 * 
 * @param PageAddr 
 * @param data 
 * @param size 
 * @return int8_t 
 */
int8_t ll_flash_read(uint32_t PageAddr, uint8_t * data, uint32_t size);

/**
 * @brief Erase sector
 * 
 * @param sector 
 * @return int8_t 
 */
int8_t ll_flash_erase(uint8_t sector);



#endif /* INC_LL_FLASH_H_ */

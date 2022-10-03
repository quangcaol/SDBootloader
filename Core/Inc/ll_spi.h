/*
 * ll_spi.h
 *
 *  Created on: Sep 26, 2022
 *      Author: quangcaol
 */

#ifndef INC_LL_SPI_H_
#define INC_LL_SPI_H_

#include "main.h"

#define SPI_HANDLE SPI2

#define	HAL_SPI_ENABLE(__HANDLE__)		SET_BIT((__HANDLE__)->CR1, SPI_CR1_SPE)

#define SPI_DISABLE(__HANDLE__)			CLEAR_BIT((__HANDLE__)->CR1, SPI_CR1_SPE)

#define SPI_SET_TX(__HANDLE__)			SET_BIT((__HANDLE__)->CR1, SPI_CR1_BIDIOE)

#define SPI_SET_RX(__HANDLE__)			CLEAR_BIT((__HANDLE__)->CR1, SPI_CR1_BIDIOE)

int8_t spi_send(const uint8_t * buff,uint16_t len,uint32_t timeout);

int8_t spi_read(uint8_t * buff,uint16_t len,uint32_t timeout);

int8_t spi_send_and_read(const uint8_t * tx_buff,uint8_t * rx_buff,uint16_t len,uint32_t timeout);

int8_t spi_wait_flag(uint32_t reg,uint32_t flag,uint8_t state,uint32_t ms);


#endif /* INC_LL_SPI_H_ */

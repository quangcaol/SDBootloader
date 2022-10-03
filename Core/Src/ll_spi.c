/*
 * ll_spi.c
 *
 *  Created on: Sep 26, 2022
 *      Author: quangcaol
 */
#include "ll_spi.h"

int8_t spi_send(const uint8_t * buff,uint16_t len,uint32_t timeout)
{
	uint16_t tx_count = len;

	uint8_t * temp_buf = buff;

	if (LL_SPI_IsEnabled(SPI_HANDLE) == RESET) LL_SPI_Enable(SPI_HANDLE);

	while (tx_count > 0)
	{
		if (spi_wait_flag(SPI_HANDLE->SR, LL_SPI_SR_TXE, SET, timeout) <0) return -1;
		LL_SPI_TransmitData8(SPI_HANDLE, *temp_buf);
		temp_buf++;
		tx_count--;
	}

	return 0;
}

int8_t spi_read(uint8_t * buff,uint16_t len,uint32_t timeout)
{
	uint16_t recv_count = len;
	uint8_t tx_buff = 0xFF;

	if (LL_SPI_IsEnabled(SPI_HANDLE) == RESET) LL_SPI_Enable(SPI_HANDLE);

	while (recv_count > 0)
	{
		if (spi_wait_flag(SPI_HANDLE->SR, LL_SPI_SR_TXE, SET, timeout) < 0) return -1;
		LL_SPI_TransmitData8(SPI_HANDLE, &tx_buff);
		if (spi_wait_flag(SPI_HANDLE->SR, LL_SPI_SR_RXNE, SET, timeout) <0) return -1;
		*buff = LL_SPI_ReceiveData8(SPI_HANDLE);

		recv_count--;

		if (recv_count == 0) break;

		buff++;
	}

	return 0;
}

int8_t spi_send_and_read(const uint8_t * tx_buff,uint8_t * rx_buff,uint16_t len,uint32_t timeout)
{
	int16_t recv_count = 0;
	int16_t tx_count = len;
	int8_t switch_state = 0;
	int i = 0;


	const uint8_t * temp_tx = tx_buff;
	uint8_t * temp_rx = rx_buff;

	while (i < timeout) {i++;};

	if (LL_SPI_IsEnabled(SPI_HANDLE) == RESET) LL_SPI_Enable(SPI_HANDLE);

	while (tx_count > 0 || recv_count < (len))
	{
		if (LL_SPI_IsActiveFlag_TXE(SPI_HANDLE) && tx_count > 0)
		{
			LL_SPI_TransmitData8(SPI_HANDLE, *temp_tx);
			temp_tx++;
			tx_count--;
			switch_state = 1;
		}
		if (LL_SPI_IsActiveFlag_RXNE(SPI_HANDLE))
		{
			*temp_rx = LL_SPI_ReceiveData8(SPI_HANDLE);
			temp_rx++;
			recv_count++;
			switch_state = 0;
		}
	}



	return 0;
}

int8_t spi_wait_flag(uint32_t reg,uint32_t flag,uint8_t state,uint32_t ms)
{
	uint32_t start_tick = HAL_GetTick();

	uint8_t bit_flag = (READ_BIT(reg,flag) > 0) ? SET : RESET;

	while (bit_flag!= state)
	{
		bit_flag = (READ_BIT(reg,flag) > 0) ? SET : RESET;
		if (HAL_GetTick() - start_tick > ms)
		{
			return -1;
		}
	}

	return 0;
}


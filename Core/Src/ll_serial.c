/*
 * ll_serial.c
 *
 *  Created on: Sep 20, 2022
 *      Author: quangcaol
 */
#include "ll_serial.h"

#include "ringbuf.h"

__IO static int8_t uart_buffer[UART_BUFFER_LEN];
RINGBUF uart_stream;


/**
  * @brief This function handles USART2 global interrupt.
  */
void USART2_IRQHandler(void)
{
  /* USER CODE BEGIN USART2_IRQn 0 */
	if ((LL_USART_IsActiveFlag_RXNE(USART2) != RESET) & ((USART2->CR1 & USART_CR1_RXNEIE) != RESET))
	{
		RINGBUF_Put(&uart_stream, LL_USART_ReceiveData8(USART2));
	}
  /* USER CODE END USART2_IRQn 0 */
  /* USER CODE BEGIN USART2_IRQn 1 */

  /* USER CODE END USART2_IRQn 1 */
}

uint8_t usart_send(uint8_t * buff,uint16_t len)
{
	for (uint8_t i = 0 ; i< len; ++i)
	{
		LL_USART_TransmitData8(USART2, buff[i]);
		while(!LL_USART_IsActiveFlag_TXE(USART2));
	}
	while(!LL_USART_IsActiveFlag_TC(USART2));
}

uint8_t usart_available()
{
	return (RINGBUF_GetFill(&uart_stream) > 0) ?  1 : 0;
}

uint8_t usart_read()
{
	uint8_t ch = 0;
	RINGBUF_Get(&uart_stream, &ch);
	return ch;
}

uint8_t usart_init()
{
	RINGBUF_Init(&uart_stream, uart_buffer, UART_BUFFER_LEN);

	LL_USART_EnableIT_RXNE(USART2);
	return 1;
}

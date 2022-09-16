/*
 * bootloader.c
 *
 *  Created on: Sep 12, 2022
 *      Author: quangcaol
 */
#include "bootloader.h"


enum ftp_state{
	wait,
	down,
	quit,
	timeout
}state;

__IO static int8_t uart_buffer[UART_BUFFER_LEN];

	static uint32_t wait_ticks;

RINGBUF uart_stream;

void ll_Delay(uint32_t ms)
{
	uint32_t previous = ll_getTick();
	while(ll_getTick() - previous < ms);
}

uint32_t ll_getTick()
{
	return systick_count;
}

void uasrt_irq_handler()
{
	if ((LL_USART_IsActiveFlag_RXNE(USART2) != RESET) & ((USART2->CR1 & USART_CR1_RXNEIE) != RESET))
	{
		RINGBUF_Put(&uart_stream, LL_USART_ReceiveData8(USART2));
	}
}

int handle_ftp(uint16_t ms,struct mavlink_handle_s * handle)
{
	static uint32_t local_ticks = 0;
	int res = -1;

	local_ticks += ms;
	switch (state)
	{
	case wait:
		if (handle->uploader_ready == SET)
		{
			state = down;
			ll_flash_unlock();
			if (handle->path)
			{
				ll_flash_erase(6);
				ll_flash_erase(7);
			}
			else
			{
				ll_flash_erase(4);
				ll_flash_erase(3);
				ll_flash_erase(5);
			}
			ll_flash_lock();
			mavlink_ftp_send(handle,SET);
			wait_ticks = ll_getTick();
			break;
		}
		if ((ll_getTick() - wait_ticks) > TIMEOUT_INTERVAL) state = timeout;

		if (local_ticks > WAIT_LED_BLINK_INTERVAL)
		{
			LL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
			local_ticks = 0;
		}
		break;
	case down:
		if (handle->uploader_ready == RESET)
		{
			state = quit;
		}

		if (handle->data_ready == SET)
		{
			wait_ticks = ll_getTick();
			ll_flash_unlock();
			if (handle->path == 1)
			{
				ll_flash_write(MAIN_APP_ADDR+handle->offset, &handle->ftp.payload[DATA], handle->ftp.payload[SIZE]);
			}
			else
			{
				ll_flash_write(SIDE_APP_ADDR+handle->offset, &handle->ftp.payload[DATA], handle->ftp.payload[SIZE]);
			}
			ll_flash_lock();
			mavlink_ftp_send(handle, SET);
			handle->data_ready = 0;
			break;
		}
		if ((ll_getTick() - wait_ticks) > TIMEOUT_INTERVAL) state = timeout;
		break;

		if (local_ticks > UPLOAD_LED_BLINK_INTERVAL)
		{
			LL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
			local_ticks = 0;
		}
		break;
	case quit:
		mavlink_ftp_send(handle, SET);
		res = 0;
		break;
	case timeout:
		mavlink_ftp_send(handle, RESET);
		res = 0;
	default:
		break;
	}
	return res;
}

void jump2app(uint8_t path){
	uint32_t jumpAddress = 0;
	pFunction Jump_To_Application;

	__IO uint32_t * APP_ADDR = (path) ? (uint32_t* )MAIN_APP_ADDR : (uint32_t *)SIDE_APP_ADDR;

	if (((* APP_ADDR) & 0x2FFE0000) == 0x20020000)
	{

		LL_SYSTICK_DisableIT();
		RCC->APB1RSTR |= RCC_APB1RSTR_USART2RST; // Reset Usart
		LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_USART2);
		LL_AHB1_GRP1_DisableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
		RCC->AHB1RSTR |= RCC_AHB1RSTR_GPIOBRST & RCC_AHB1RSTR_GPIOARST &  RCC_AHB1RSTR_GPIOCRST & RCC_AHB1RSTR_GPIOHRST;
		LL_RCC_DeInit();

		SCB->SHCSR &= ~( SCB_SHCSR_USGFAULTENA_Msk | \
		                 SCB_SHCSR_BUSFAULTENA_Msk | \
		                 SCB_SHCSR_MEMFAULTENA_Msk ) ;


		jumpAddress = *(APP_ADDR + 1);
		Jump_To_Application = (pFunction) jumpAddress;
		__set_MSP(*(volatile uint32_t * )jumpAddress);
		Jump_To_Application();
	}
	else
	{
		HardFault_Handler();
	}
}

int bootloader_mainloop()
{

	struct mavlink_handle_s mavlink;

	mavlink_init(&mavlink);

	LL_SYSTICK_EnableIT();

	RINGBUF_Init(&uart_stream, uart_buffer, UART_BUFFER_LEN);

	LL_USART_EnableIT_RXNE(USART2);

	state = wait;

	wait_ticks = ll_getTick();

	for(;;)
	{
		if (RINGBUF_GetFill(&uart_stream) > 0)
		{
			uint8_t ch = 0;
			RINGBUF_Get(&uart_stream, &ch);
			mavlink_receive(&mavlink,ch);
		}

		if (handle_ftp(1, &mavlink) == 0)
		{
			break;
		}

		ll_Delay(1);
	}

	wait_ticks = ll_getTick();
	for (;;)
	{
		LL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
		ll_Delay(TERMINATE_LED_BLINK_INTERVAL);
		if ((ll_getTick() - wait_ticks) > BOOT_2_APP_INTERVAL)
		{
			if (!LL_GPIO_IsInputPinSet(B1_GPIO_Port, B1_Pin)) jump2app(0);
			jump2app(1);

		}
	}

}

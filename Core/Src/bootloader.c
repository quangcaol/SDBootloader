/*
 * bootloader.c
 *
 *  Created on: Sep 12, 2022
 *      Author: quangcaol
 */
#include "bootloader.h"




int handle_ftp(struct mavlink_handle_s * handle)
{
	static uint32_t led_ticks = 0;
	static uint32_t timeout_ticks = 0;

	int res = -1;

	led_ticks += BOOTLOADER_MAINLOOP_DELAY;
	timeout_ticks += BOOTLOADER_MAINLOOP_DELAY;

	switch (handle->state)
	{
	case wait:
		if (handle->uploader_ready == SET)
		{

			erase_app(handle->path);

			timeout_ticks = 0;

			mavlink_ftp_send(handle,SET);
			handle->state = down;
			break;
		}
		if (timeout_ticks > TIMEOUT_INTERVAL) handle->state = timeout;

		if (led_ticks > WAIT_LED_BLINK_INTERVAL)
		{
			GPIO_Toggle(LD2_GPIO_Port, LD2_Pin);
			led_ticks = 0;
		}
		break;
	case down:
		if (handle->uploader_ready == RESET)
		{
			handle->state = quit;
		}

		if (handle->error)
		{
			mavlink_ftp_send(handle, RESET);
			handle->state = timeout;
			break;
		}

		if (handle->data_ready == SET)
		{
			timeout_ticks = 0;
			ll_flash_unlock();
			if (handle->path == 1)
			{
				while (ll_flash_write(MAIN_APP_ADDR+handle->offset, &handle->ftp.payload[DATA], handle->ftp.payload[SIZE]) < 0);
			}
			else
			{
				while (ll_flash_write(SIDE_APP_ADDR+handle->offset, &handle->ftp.payload[DATA], handle->ftp.payload[SIZE]) < 0);
			}
			ll_flash_lock();

			timeout_ticks = 0;

			mavlink_ftp_send(handle, SET);
			handle->data_ready = 0;
			break;
		}
		if (timeout_ticks > TIMEOUT_INTERVAL)
		{
			handle->state = timeout;
			break;
		}

		if (led_ticks > UPLOAD_LED_BLINK_INTERVAL)
		{
			GPIO_Toggle(LD2_GPIO_Port, LD2_Pin);
			led_ticks = 0;
		}
		break;
	case quit:
		mavlink_ftp_send(handle, SET);
		res = 0;
		break;
	case timeout:
		mavlink_ftp_send(handle, RESET);

		res = 0;

		if(!handle->uploader_ready) break;

		erase_app(handle->path);

		break;
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
		peripheral_deinit();
		jumpAddress = *(APP_ADDR + 1);
		Jump_To_Application = (pFunction) jumpAddress;
		__set_MSP(*(volatile uint32_t * )jumpAddress);
		Jump_To_Application();
	}
}

void erase_app(uint8_t app)
{
	ll_flash_unlock();
	if (app)
	{
		ll_flash_erase(SECTOR_6);
		ll_flash_erase(SECTOR_7);
	}
	else {
		ll_flash_erase(SECTOR_4);
		ll_flash_erase(SECTOR_3);
		ll_flash_erase(SECTOR_5);
	}
	ll_flash_lock();
}

int bootloader_mainloop()
{

	struct mavlink_handle_s mavlink;

	ENABLE_SYSTICK();

	usart_init();

	mavlink_init(&mavlink);
	for(;;)
	{
		if (usart_available() > 0)
		{
			uint8_t ch = 0;
			ch = usart_read();
			mavlink_receive(&mavlink,ch);
		}

		if (handle_ftp(&mavlink) == 0)
		{
			break;
		}

		delay_ms(BOOTLOADER_MAINLOOP_DELAY);
	}

	uint32_t wait_ticks = get_systick();

	for (;;)
	{
		GPIO_Toggle(LD2_GPIO_Port, LD2_Pin);
		if ((get_systick() - wait_ticks) > BOOT_2_APP_INTERVAL)
		{
			if (!GPIO_Read(B1_GPIO_Port, B1_Pin)) jump2app(SIDE_APP);
			jump2app(MAIN_APP);

		}
		delay_ms(TERMINATE_LED_BLINK_INTERVAL);
	}

}

/*
 * bootloader.h
 *
 *  Created on: Sep 12, 2022
 *      Author: quangcaol
 */

#ifndef INC_BOOTLOADER_H_
#define INC_BOOTLOADER_H_

#include "ringbuf.h"
#include "ll_flash.h"
#include "mavlink_handle.h"

#define BOOTLOADER_ADDR 0x8000000
#define MAIN_APP_ADDR  0x08040000
#define SIDE_APP_ADDR  0x0800C000

#define UART_BUFFER_LEN 256

#define TIMEOUT_INTERVAL  5000 //Ms
#define BOOT_2_APP_INTERVAL 3000

#define WAIT_LED_BLINK_INTERVAL    		1000
#define UPLOAD_LED_BLINK_INTERVAL 		500
#define TERMINATE_LED_BLINK_INTERVAL	200


extern __IO uint32_t systick_count;

typedef void (*pFunction)(void);

void uasrt_irq_handler();


uint32_t ll_getTick();

void ll_Delay(uint32_t ms);

int bootloader_mainloop();

int handle_ftp(uint16_t ms,struct mavlink_handle_s * handle);

void jump2app(uint8_t path);




#endif /* INC_BOOTLOADER_H_ */

 /*
 * bootloader.h
 *
 *  Created on: Sep 12, 2022
 *      Author: quangcaol
 */

#ifndef INC_BOOTLOADER_H_
#define INC_BOOTLOADER_H_


#include "port.h"

#include "ll_flash.h"
#include "ll_serial.h"

#include "mavlink_handle.h"

/**
 * @brief Define 3 Main Address for boot 
 * User must change this field to suitable address
 * 
 */
#define BOOTLOADER_ADDR 0x8000000
#define MAIN_APP_ADDR  0x08040000
#define SIDE_APP_ADDR  0x0800C000


#define TIMEOUT_INTERVAL  5000 //Ms
#define BOOT_2_APP_INTERVAL 3000

#define WAIT_LED_BLINK_INTERVAL    		1000
#define UPLOAD_LED_BLINK_INTERVAL 		500
#define TERMINATE_LED_BLINK_INTERVAL		200

#define BOOTLOADER_MAINLOOP_DELAY		1

#define MAIN_APP 1
#define SIDE_APP 0

/**
 * @brief typedef for function pointer
 * 
 */
typedef void (*pFunction)(void);

int bootloader_mainloop();

int handle_ftp(struct mavlink_handle_s * handle);

/**
 * @brief Jump to application 
 * 
 * @param path main app is 1 
 */
void jump2app(uint8_t path);

/**
 * @brief  Erase memory app
 * 
 * @param app main app is 1
 */
void erase_app(uint8_t app);




#endif /* INC_BOOTLOADER_H_ */

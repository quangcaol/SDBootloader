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
#include "ll_serial.h" // Debuging

#include "fatfs.h"



/**
 * @brief Define 3 Main Address for boot 
 * User must change this field to suitable address
 * 
 */
#define BOOTLOADER_ADDR 	0x8000000
#define MAIN_APP_ADDR  		0x08040000
#define SIDE_APP_ADDR  		0x0800C000


#define CHUNK_OF_DATA		4096

/**
 * @brief ms 
 * 
 */
#define INITALIZATION_INTERVAL 			10000
#define UPLOAD_INTERVAL 			2000
#define BOOT_INTERVAL				2000

#define WAIT_LED_BLINK_INTERVAL    		1000
#define UPLOAD_LED_BLINK_INTERVAL 		500
#define TERMINATE_LED_BLINK_INTERVAL		200


#define BUTTON_HOLD  					1000
#define BUTTON_PRESS 					200

#define BOOTLOADER_MAINLOOP_DELAY		1


enum application {
	SIDE_APP = 0,
	MAIN_APP
};

enum bootloader_state {
	INITAL = 0,
	UPLOAD,
	BOOT
};

enum button_state {
	UNPRESS = 0,
	PRESSED,
	HOLD
};

enum upload_error {
	NO_ERROR = 0,
	MAIN_APP_ERROR,
	SIDE_APP_ERROR
};

/**
 * @brief typedef for function pointer
 * 
 */
typedef void (*pFunction)(void);

/**
 * @brief Mainloop for bootloader 
 * 
 * @return int 
 */
int bootloader_mainloop();





#endif /* INC_BOOTLOADER_H_ */

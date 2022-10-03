/*
 * port.h
 *
 *  Created on: Sep 20, 2022
 *      Author: quangcaol
 */

#ifndef INC_PORT_H_
#define INC_PORT_H_

#include "main.h"
#include "fatfs.h"


/**
 * @brief Systick macro 
 * 
 */
#define ENABLE_SYSTICK() LL_SYSTICK_EnableIT()

#define GPIO_Toggle(port,pin) LL_GPIO_TogglePin(port,pin)
#define GPIO_Read(port,pin)   LL_GPIO_IsInputPinSet(port,pin)

#define delay_ms(ms) HAL_Delay(ms);
#define get_systick() HAL_GetTick();


#define  SD_LOG 		1
#define  SERIAL_LOG		1

/**
 * @brief deinit all peripheral of bootloader program
 * 
 */
void peripheral_deinit();

#endif /* INC_PORT_H_ */

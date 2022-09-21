/*
 * port.h
 *
 *  Created on: Sep 20, 2022
 *      Author: quangcaol
 */

#ifndef INC_PORT_H_
#define INC_PORT_H_

#include "main.h"
#include "stm32f4xx_it.h" // Get systick handler
#include "ringbuf.h"

/**
 * @brief Systick macro 
 * 
 */
#define ENABLE_SYSTICK() LL_SYSTICK_EnableIT()

#define GPIO_Toggle(port,pin) LL_GPIO_TogglePin(port,pin)
#define GPIO_Read(port,pin)   LL_GPIO_IsInputPinSet(port,pin)

void delay_ms(uint32_t ms);

uint32_t get_systick();

/**
 * @brief deinit all peripheral of bootloader program
 * 
 */
void peripheral_deinit();

#endif /* INC_PORT_H_ */

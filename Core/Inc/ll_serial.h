/*
 * ll_serial.h
 *
 *  Created on: Sep 20, 2022
 *      Author: quangcaol
 */

#ifndef INC_LL_SERIAL_H_
#define INC_LL_SERIAL_H_

#include "main.h"

/**
 * @brief Define buffer memory 
 * 
 */
#define UART_BUFFER_LEN 256

/**
 * @brief send buffer
 * 
 * @param buff 
 * @param len 
 * @return uint8_t 
 */
uint8_t serial_send(uint8_t * buff,uint16_t len);

/**
 * @brief Read uasrt
 * 
 * @return uint8_t 
 */
uint8_t serial_read();

/**
 * @brief Check if receive
 * 
 * @return uint8_t if available is 1
 */
uint8_t serial_available();

/**
 * @brief init serial 
 * 
 * @return uint8_t 
 */
uint8_t serial_init();




#endif /* INC_LL_SERIAL_H_ */

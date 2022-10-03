/*
 * ll_serial.c
 *
 *  Created on: Sep 20, 2022
 *      Author: quangcaol
 */
#include "ll_serial.h"

#include "port.h"



uint8_t serial_send(uint8_t * buff,uint16_t len)
{
	for (uint8_t i = 0 ; i< len; ++i)
	{
		LL_USART_TransmitData8(USART2, buff[i]);
		while(!LL_USART_IsActiveFlag_TXE(USART2));
	}
	while(!LL_USART_IsActiveFlag_TC(USART2));
	return 1;
}

#if defined(SERIAL_LOG)
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

void user_printf(const char *fmt, ...)
{
	  static char buffer[256];
	  va_list args;
	  va_start(args, fmt);
	  vsnprintf(buffer, sizeof(buffer), fmt, args);
	  va_end(args);

	  int len = strlen(buffer);
	  serial_send(buffer, len);
}
#endif

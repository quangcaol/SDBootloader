/*
 * port.c
 *
 *  Created on: Sep 20, 2022
 *      Author: quangcaol
 */

#include "port.h"

__IO uint32_t systick_count = 0;


void delay_ms(uint32_t ms)
{
	uint32_t previous = get_systick();
	while(get_systick() - previous < ms);
}

uint32_t get_systick()
{
	return systick_count;
}

void peripheral_deinit()
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
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */
	systick_count++;
  /* USER CODE END SysTick_IRQn 0 */

  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}


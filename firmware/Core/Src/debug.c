/*
 * debug.c
 *
 *  Created on: Oct 7, 2026
 *      Author: rcall
 */

#include "debug.h"
#include "stm32l4xx.h"

// hardcodes for PA1
void debug_gpio_init(void) {
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

	// 0b01 = output
	GPIOA->MODER &= ~(0x3 << GPIO_MODER_MODE1_Pos);
	GPIOA->MODER |= (0x1 << GPIO_MODER_MODE1_Pos);
}

// hardcoded for PA1
void debug_gpio_toggle(void) {
	if (GPIOA->ODR & GPIO_ODR_OD1) {
		GPIOA->BSRR = GPIO_BSRR_BR1;
	} else {
		GPIOA->BSRR = GPIO_BSRR_BS1;
	}
}

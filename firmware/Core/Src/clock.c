/*
 * clock.c
 *
 *  Created on: Oct 9, 2026
 *      Author: rcall
 */

#include "clock.h"
#include "stm32l4xx.h"

void clock_init(void) {
	RCC->CR |= RCC_CR_MSIRGSEL;

	// MSI to 8 MHz (Range 7)
	RCC->CR &= ~RCC_CR_MSIRANGE;
	RCC->CR |= RCC_CR_MSIRANGE_7;

	RCC->CR |= RCC_CR_MSION;

	while (!(RCC->CR & RCC_CR_MSIRDY)) {};

	// I2C clock source: MSI
	RCC->CCIPR | (0x1 << RCC_CCIPR_I2C1SEL_Pos);
}

// for debugging
void clock_mco(void) {
	// gpio cfg (PA8)

	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

	// AF mode
	GPIOA->MODER &= ~GPIO_MODER_MODE8;
	GPIOA->MODER |= (0x2 << GPIO_MODER_MODE8_Pos);

	// AF0
	GPIOA->AFR[1] &= ~GPIO_AFRH_AFSEL8;

	// RCC cfg
	RCC->CFGR |= (0x8 << RCC_CFGR_MCOPRE_Pos) | RCC_MCO1SOURCE_MSI; // MCO divided by 16, 500 kHz should be observed
}

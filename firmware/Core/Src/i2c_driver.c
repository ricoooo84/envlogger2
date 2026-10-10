/*
 * i2c_driver.c
 *
 *  Created on: Oct 7, 2026
 *      Author: rcall
 */

// PA9: SCL
// PA10: SDA

#include "i2c_driver.h"

static void gpio_cfg(void) {
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

	GPIOA->MODER &= ~((0x3 << GPIO_MODER_MODE9_Pos) | ((0x3 << GPIO_MODER_MODE10_Pos)));
	GPIOA->MODER |= (0x2 << GPIO_MODER_MODE9_Pos) | (0x2 << GPIO_MODER_MODE10_Pos);

	GPIOA->OTYPER |= GPIO_OTYPER_OT9 | GPIO_OTYPER_OT10;

	GPIOA->AFR[1] |= (0x4 << GPIO_AFRH_AFSEL9_Pos) | (0x4 << GPIO_AFRH_AFSEL10_Pos);
}

static void i2c_cfg(void) {
	RCC->APB1ENR1 |= RCC_APB1ENR1_I2C1EN;

	// 400 kHz at 8 MHz
	I2C1->TIMINGR |= (0x9 << I2C_TIMINGR_SCLL_Pos) | (0x3 << I2C_TIMINGR_SCLH_Pos) |
					 (0x1 << I2C_TIMINGR_SDADEL_Pos)| (0x3 << I2C_TIMINGR_SCLDEL_Pos);
}

void i2c_init(void) {
	gpio_cfg();
	i2c_cfg();

	I2C1->CR1 |= I2C_CR1_PE;
}

static void i2c_start(uint8_t nbytes, uint8_t read, uint8_t autoend) {
	I2C1->CR2 = (nbytes << I2C_CR2_NBYTES_Pos) | (read << I2C_CR2_RD_WRN_Pos) |
				(autoend << I2C_CR2_AUTOEND_Pos) | (SLAVE_ADDR << 1);

	I2C1->CR2 |= I2C_CR2_START;
}

static i2c_status_t i2c_wait_flag(uint32_t flag) {
	uint32_t timeout = I2C_TIMEOUT;
	uint32_t isr;

	while (1) {
		isr = I2C1->ISR;

		if (isr & I2C_ISR_NACKF) return I2C_STATUS_NACK;
		if (isr & flag)          return I2C_STATUS_OK;
		if (timeout-- == 0)      return I2C_STATUS_TIMEOUT;
	}

}

// IMPORTANT!: currently, NO ERROR FLAGS ARE CLEARED WHEN THEY OCCUR
// for now, crash program when error is found
// TODO: actual error handling

i2c_status_t i2c_write_reg(uint8_t reg, uint8_t buf) {
	if (I2C1->ISR & I2C_ISR_BUSY) {
		return I2C_STATUS_BUSY;
	}

	// 1. start transaction
	// nbytes = 2 (reg then buf)
	i2c_start(2, I2C_WRITE, I2C_AUTOEND);

	// 2. write register
	I2C_RETURN_IF_ERROR(i2c_wait_flag(I2C_ISR_TXIS));
	I2C1->TXDR = reg;

	// 3. write data
	I2C_RETURN_IF_ERROR(i2c_wait_flag(I2C_ISR_TXIS));
	I2C1->TXDR = buf;

	// 4. clear stop
	I2C_RETURN_IF_ERROR(i2c_wait_flag(I2C_ISR_STOPF));
	I2C1->ICR |= I2C_ICR_STOPCF;

	return I2C_STATUS_OK;
}

i2c_status_t i2c_read_regs(uint8_t reg, uint8_t *buf, uint16_t len) {
	if (I2C1->ISR & I2C_ISR_BUSY) {
		return I2C_STATUS_BUSY;
	}

	if (len == 0 || len > 255) {
		return I2C_STATUS_INVALID_ARG;
	}

	// 1. start transaction
	i2c_start(1, I2C_WRITE, I2C_NOAUTOEND);

	// 2. write register
	I2C_RETURN_IF_ERROR(i2c_wait_flag(I2C_ISR_TXIS));
	I2C1->TXDR = reg;
	I2C_RETURN_IF_ERROR(i2c_wait_flag(I2C_ISR_TC));

	// 3. start transaction
	i2c_start(len, I2C_READ, I2C_AUTOEND);

	// 4. read registers
	for (uint16_t i = 0; i < len; i++) {
		I2C_RETURN_IF_ERROR(i2c_wait_flag(I2C_ISR_RXNE));
		buf[i] = (uint8_t)I2C1->RXDR;
	}

	// 5. clear stop
	I2C_RETURN_IF_ERROR(i2c_wait_flag(I2C_ISR_STOPF));
	I2C1->ICR |= I2C_ICR_STOPCF;

	return I2C_STATUS_OK;
}

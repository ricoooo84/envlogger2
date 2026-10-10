/*
 * i2c_driver.h
 *
 *  Created on: Oct 7, 2026
 *      Author: rcall
 */

#ifndef INC_I2C_DRIVER_H_
#define INC_I2C_DRIVER_H_

// PA9: SCL
// PA10: SDA

#include "stm32l4xx.h"

#define I2C_RETURN_IF_ERROR(_status) do {if (_status != I2C_STATUS_OK) return _status;} while(0)

#define SLAVE_ADDR		0x77
#define I2C_TIMEOUT		10000

#define I2C_NOAUTOEND		0
#define I2C_AUTOEND			1
#define I2C_READ			1
#define I2C_WRITE			0

typedef enum {
	I2C_STATUS_OK,
	I2C_STATUS_TIMEOUT,
	I2C_STATUS_NACK,
	I2C_STATUS_BUSY,
	I2C_STATUS_INVALID_ARG,

} i2c_status_t;

void i2c_init(void);
i2c_status_t i2c_write_reg(uint8_t reg, uint8_t buf);
i2c_status_t i2c_read_regs(uint8_t reg, uint8_t *buf, uint16_t len);


#endif /* INC_I2C_DRIVER_H_ */

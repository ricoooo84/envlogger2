/*
 * bme_driver.h
 *
 *  Created on: Oct 9, 2026
 *      Author: rcall
 */

#ifndef INC_BME_DRIVER_H_
#define INC_BME_DRIVER_H_

#include "i2c_driver.h"

#define BME_ADDRESS		0x76 // SDO pulled to GND

#define REG_HUM_MSB		0xFD
#define REG_TEMP_MSB	0xFA
#define REG_PRESS_MSB	0xF7
#define REG_CONFIG		0xF5
#define REG_CTRL_HUM	0xF2
#define REG_CTRL_MEAS	0xF4
#define REG_CALIB_00	0x88
#define REG_CALIB_26	0xE1
#define REG_CHIP_ID		0xD0

typedef struct {
	uint16_t dig_T1;
	int16_t dig_T2, dig_T3;
	uint16_t dig_P1;
	int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
	uint8_t dig_H1;
	int16_t dig_H2;
	uint8_t dig_H3;
	int16_t dig_H4, dig_H5;
	int8_t dig_H6;
} bme_calib_t;

typedef struct {
	int32_t raw_temp;
	int32_t raw_pres;
	int16_t raw_humid;
} bme_raw_data_t;

typedef struct {
	uint32_t temp;
	uint32_t pres;
	uint32_t humid;
	int32_t t_fine;
} bme_data_t;

typedef struct {
	bme_calib_t calib;
	bme_raw_data_t raw_data;
	bme_data_t data;
} bme_t;

typedef enum {
	BME_OSRS_H_SKIP,
	BME_OSRS_H_x1,
	BME_OSRS_H_x2,
	BME_OSRS_H_x4,
	BME_OSRS_H_x8,
	BME_OSRS_H_x16
} bme_osrs_h_t;
typedef enum {
	BME_OSRS_T_SKIP,
	BME_OSRS_T_x1 = (1 << 5),
	BME_OSRS_T_x2 = (2 << 5),
	BME_OSRS_T_x4 = (3 << 5),
	BME_OSRS_T_x8 = (4 << 5),
	BME_OSRS_T_x16 = (5 << 5),
} bme_osrs_t_t;
typedef enum {
	BME_OSRS_P_SKIP,
	BME_OSRS_P_x1 = (1 << 2),
	BME_OSRS_P_x2 = (2 << 2),
	BME_OSRS_P_x4 = (3 << 2),
	BME_OSRS_P_x8 = (4 << 2),
	BME_OSRS_P_x16 = (5 << 2),
} bme_osrs_p_t;
typedef enum {
	BME_MODE_SLEEP,
	BME_MODE_FORCED,
	BME_MODE_NORMAL = 3
} bme_mode_t;
typedef enum {
	BME_FILTER_OFF,
	BME_FILTER_2,
	BME_FILTER_4,
	BME_FILTER_8,
	BME_FILTER_16,
} bme_filter_t;


void bme_cfg(void);
void bme_calibrate(bme_t *dev);
void bme_read(bme_t *dev);

#endif /* INC_BME_DRIVER_H_ */

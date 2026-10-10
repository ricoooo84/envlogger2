/*
 * bme_driver.c
 *
 *  Created on: Oct 9, 2026
 *      Author: rcall
 */

#include "bme_driver.h"

void bme_cfg(void) {

	// write filter to REG_CONFIG
	i2c_write_reg(REG_CONFIG, BME_FILTER_OFF);

	// write osrs_h to REG_CTRL_HUM
	i2c_write_reg(REG_CTRL_HUM, BME_OSRS_H_x1);

	// write osrs_p, osrs_t, and mode to REG_CTRL_MEAS
	i2c_write_reg(REG_CTRL_MEAS, BME_OSRS_P_x1 | BME_OSRS_T_x1 | BME_MODE_NORMAL);
}

void bme_calibrate(bme_t *dev) {
	uint8_t buf1[25], buf2[7];

	// read regs into buf1
	i2c_read_regs(REG_CALIB_00, buf1, 25);

	dev->calib.dig_T1 = (buf1[1] << 8) | buf1[0];
	dev->calib.dig_T2 = (buf1[3] << 8) | buf1[2];
	dev->calib.dig_T3 = (buf1[5] << 8) | buf1[4];

	dev->calib.dig_P1 = (buf1[7] << 8) | buf1[6];
	dev->calib.dig_P2 = (buf1[9] << 8) | buf1[8];
	dev->calib.dig_P3 = (buf1[11] << 8) | buf1[10];
	dev->calib.dig_P4 = (buf1[13] << 8) | buf1[12];
	dev->calib.dig_P5 = (buf1[15] << 8) | buf1[14];
	dev->calib.dig_P6 = (buf1[17] << 8) | buf1[16];
	dev->calib.dig_P7 = (buf1[19] << 8) | buf1[18];
	dev->calib.dig_P8 = (buf1[21] << 8) | buf1[20];
	dev->calib.dig_P9 = (buf1[23] << 8) | buf1[22];

	dev->calib.dig_H1 = (buf1[25]);

	// read regs into buf2
	i2c_read_regs(REG_CALIB_26, buf2, 7);

	dev->calib.dig_H2 = (buf2[1] << 8) | buf2[0];
	dev->calib.dig_H3 = buf2[2];
	dev->calib.dig_H4 = (buf2[3] << 4) | (buf2[4] & 0xF);
	dev->calib.dig_H5 = (buf2[4] >> 4) | (buf2[5] << 4);
	dev->calib.dig_H6 = (buf2[6]);

}

static void bme_read_raw(bme_t *dev) {
	uint8_t buf[8];

	// read regs into buf
	i2c_read_regs(REG_PRESS_MSB, buf, 8);

	dev->raw_data.raw_humid = (buf[6] << 8) | buf[7];
	dev->raw_data.raw_temp = (buf[3] << 12) | (buf[4] << 4) | (buf[5] >> 4);
	dev->raw_data.raw_pres = (buf[0] << 12) | (buf[1] << 4) | (buf[2] >> 4);

}

static int32_t bme_compensate_temp(bme_t *dev) {
	int32_t var1, var2, adc_T, t_fine, T;
	adc_T = dev->raw_data.raw_temp;

	var1 = ((((adc_T >> 3) - ((int32_t)dev->calib.dig_T1 << 1))) * ((int32_t)dev->calib.dig_T2)) >> 11;
	var2 = (((((adc_T >> 4) - ((int32_t)dev->calib.dig_T1)) * ((adc_T>>4) - ((int32_t)dev->calib.dig_T1))) >> 12) *
			((int32_t)dev->calib.dig_T3)) >> 14;
	t_fine = var1 + var2;
	dev->data.t_fine = t_fine;
	T = ((t_fine) * 5 + 128) >> 8;

	return (uint32_t)T;
}

static uint32_t bme_compensate_pres(bme_t *dev) {
	int64_t var1, var2, adc_P, p;
	adc_P = dev->raw_data.raw_pres;

	var1 = ((int64_t)dev->data.t_fine) - 128000;
	var2 = var1 * var1 * (int64_t)dev->calib.dig_P6;
	var2 = var2 + ((var1*(int64_t)dev->calib.dig_P5) << 17);
	var2 = var2 + (((int64_t)dev->calib.dig_P4) << 35);
	var1 = ((var1 * var1 * (int64_t)dev->calib.dig_P3) >> 8) + ((var1 * (int64_t)dev->calib.dig_P2) << 12);
	var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dev->calib.dig_P1) >> 33;
	/* why???
	if (var1 == 0) {
		return 0;
	}
	*/
	p = 1048576 - adc_P;
	p = (((p << 31) - var2)*3125)/var1;
	var1 = (((int64_t)dev->calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
	var2 = (((int64_t)dev->calib.dig_P8) * p) >> 19;
	p = ((p + var1 + var2) >> 8) + (((int64_t)dev->calib.dig_P7)<<4);

	return (uint32_t)p;
}

static uint32_t bme_compensate_humid(bme_t *dev) {
	int32_t v_x1, adc_H;
	adc_H = dev->raw_data.raw_humid;

	v_x1 = (dev->data.t_fine - ((int32_t)76800));

	v_x1 = (((((adc_H << 14) - (((int32_t)dev->calib.dig_H4) << 20)
			- (((int32_t)dev->calib.dig_H5) * v_x1)) + ((int32_t)16384)) >> 15)
			* (((((((v_x1 * ((int32_t)dev->calib.dig_H6)) >> 10)
			* (((v_x1 * ((int32_t)dev->calib.dig_H3)) >> 11) + ((int32_t)32768))) >> 10)
			+ ((int32_t)2097152)) * ((int32_t)dev->calib.dig_H2) + 8192) >> 14));

	v_x1 = (v_x1 - (((((v_x1 >> 15) * (v_x1 >> 15)) >> 7)
			* ((int32_t)dev->calib.dig_H1)) >> 4));

	v_x1 = (v_x1 < 0) ? 0 : v_x1;
	v_x1 = (v_x1 > 419430400) ? 419430400 : v_x1;

	return (uint32_t)(v_x1 >> 12);
}

void bme_read(bme_t *dev) {
	bme_read_raw(dev);

	// compensate temp first, pres/humid depends on that data
	dev->data.temp = bme_compensate_temp(dev) * 0.01;
	dev->data.pres = bme_compensate_pres(dev) / 256.0 / 100.0;
	dev->data.humid = bme_compensate_humid(dev) / 1024.0;
}

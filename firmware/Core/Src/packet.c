/*
 * packet.c
 *
 *  Created on: Oct 6, 2026
 *      Author: rcall
 */

#include <stdint.h>
#include <string.h>
#include "packet.h"

// CRC16/CCITT written by Claude
uint16_t packet_crc(const uint8_t *data) {
	uint16_t crc = 0xFFFF;

    for (uint8_t i = 0; i < PACKET_SIZE-2; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t b = 0; b < 8; b++) {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
        }
    }

    return crc;
}

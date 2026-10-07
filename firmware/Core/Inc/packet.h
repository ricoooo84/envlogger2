/*
 * packet.h
 *
 *  Created on: Oct 6, 2026
 *      Author: rcall
 */

#ifndef INC_PACKET_H_
#define INC_PACKET_H_

#define PACKET_SIZE		16

// ordered to be 16 bytes
typedef struct {
	uint32_t timestamp;
	uint16_t num;
	uint16_t temp;
	uint16_t pres;
	uint16_t humid;
	uint8_t flags;
	uint8_t unused;
	uint16_t crc;
} packet_t;

uint16_t packet_crc(const uint8_t *data);

#endif /* INC_PACKET_H_ */

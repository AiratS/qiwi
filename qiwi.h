/* Name: qiwi.h
 * Project: QIWI network driver
 * Author: Airat Sirazov
 * Creation Date: 2019-01-11
 * License: DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
 */
 
#ifndef QIWI_H_
#define QIWI_H_
 
#include "qiwi_config.h"

// protocol config
#define QIWI_VERSION 1
#define QIWI_RECEIVER_BUFFER_SIZE    32
#define QIWI_TRANSMITTER_BUFFER_SIZE 32

#define QIWI_MAX_SUM 255

#define QIWI_MASTER    OxFF
#define QIWI_BROADCAST OxFF
#define QIWI_SPECIAL_1 0
#define QIWI_SPECIAL_2 0
#define QIWI_SPECIAL_3 0


#define QIWI_FBGN   0x00
#define QIWI_FEND   0x00

#define QIWI_FBGN_SIZE 		1
#define QIWI_HEADERS_SIZE 	4
#define QIWI_SUM_SIZE 		1
#define QIWI_END_SIZE 		1
#define QIWI_SERVICE_DATA_SIZE QIWI_FBGN_SIZE + QIWI_HEADERS_SIZE + QIWI_SUM_SIZE + QIWI_END_SIZE

#define QIWI_NO_DATA        [0x00]
#define QIWI_NO_DATA_SIZE   1
#define QIWI_ACK        	0xDD

#define QIWI_RECEIVE_ON    							QIWI_RS485_DRIVER_CONTROL_PORT |= (1 << QIWI_RS485_DRIVER_CONTROL_PIN)
#define QIWI_TRANSMITT_ON  							QIWI_RS485_DRIVER_CONTROL_PORT &= ~(1 << QIWI_RS485_DRIVER_CONTROL_PIN)
#define QIWI_RECEIVE_OFF   							QIWI_TRANSMITT_ON
#define QIWI_TRANSMITT_OFF 							QIWI_RECEIVE_ON
#define QIWI_RS485_DRIVER_DIRECTION_OUTPUT 			QIWI_RS485_DRIVER_DIRECTION_REG != (1 << QIWI_RS485_DRIVER_CONTROL_PIN)

#define FBGN     	0
#define DST_ADDR   	1
#define SRC_ADDR   	2
#define VERSION    	3
#define CMD        	4
#define DATA_START 	5

#define SUM(data_size)  DATA_START + data_size
#define FEND(data_size) DATA_START + data_size + 1


void qiwi_init();
void qiwi_set_device_addr(uint8_t addr);
void qiwi_set_on_data_received(void(*r) (uint8_t src_addr, uint8_t cmd, uint8_t* data, uint8_t size));
void qiwi_transmit(uint8_t dst_addr, uint8_t src_addr, uint8_t cmd, uint8_t* data, uint8_t size);
void qiwi_send_error(uint8_t error_code);

#endif /* QIWI_H_ */

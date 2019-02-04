/* Name: qiwi.c
 * Project: QIWI network driver
 * Author: Airat Sirazov
 * Creation Date: 2019-01-11
 * License: DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
 */
 
#include "qiwi_config.h"
#include "qiwi.h"

volatile uint8_t receiver_buffer[QIWI_RECEIVER_BUFFER_SIZE] = { 0 };
volatile uint8_t transmitter_buffer[QIWI_TRANSMITTER_BUFFER_SIZE] = { 0 };
static uint8_t device_addr;
void (*receiver)(uint8_t src_addr, uint8_t cmd, uint8_t* data, uint8_t size);

static inline init_rs485_driver_control_port()
{
	QIWI_RS485_DRIVER_DIRECTION_OUTPUT;
	QIWI_RECEIVER_ON;
}

static inline init_usart()
{
	
}

/**
 * Inits GPIO, USART module 
 *
 */
void qiwi_init()
{
	init_rs485_driver_control_port();
	init_usart();
}

void qiwi_set_device_addr(uint8_t addr)
{
	device_addr = addr;
}

/**
 * Sets receiver handler on user's function side 
 */
void qiwi_set_on_data_received(void(*r) (uint8_t src_addr, uint8_t cmd, uint8_t* data, uint8_t size))
{
	receiver = r;
}

/**
 * Fills reciever buffer until detecting FEND byte,
 * after calls receiver handler 
 */
ISR()
{
	static int idx = 0;
	uint8_t byte = UDR;
	if (byte != QIWI_FEND)
	{
		receiver_buffer[idx++] = byte;
	}
	else 
	{
		uint8_t dst_addr = receiver_buffer[DST_ADDR];
		uint8_t cmd = receiver_buffer[CMD];
		
		if (check_sum(receiver_buffer, idx) && dst_addr == device_addr && cmd != QIWI_ACK)
		{
			if (cmd != QIWI_BROADCAST) 
			{
				qiwi_ack(receiver_buffer[SRC_ADDR]);
			}
				
			receiver(receiver_buffer[SRC_ADDR], cmd, &receiver_buffer[DATA_START], DATA_SIZE(idx));
		}
		idx = 0;
	}
}

/*
 * Directly writes to USART module registers, and waits each
 * byte transmission end  
 */
static inline void usart_write_bytes(uint8_t size)
{
	uint8_t byte;
	for (register int i = 0; i < size; i++)
	{
		byte = transmitter_buffer[i];
		// 
		UDR = byte;
	}
}

/**
 * - Checks for allowed data size, if success, pulls up TI pin 
 *   on the rs485 chip to high volt.
 * - Begins to transmit byte by byte 
 * - Pulls down RE pin on the rs485 chip, after transmittion complete 
 */
void qiwi_transmit(uint8_t dst_addr, uint8_t cmd, uint8_t* data, uint8_t size)
{
	int buffer_size = 0;
	if ((QIWI_SERVICE_DATA_SIZE + size) < QIWI_TRANSMITTER_BUFFER_SIZE) 
	{
		QIWI_TRANSMITT_ON;
		write_into_transmitter_buffer(dst_addr, cmd, data, size);
		
		buffer_size = QIWI_SERVICE_DATA_SIZE + size;
		usart_write_bytes(buffer_size);
		
		QIWI_TRANSMITT_OFF;
	}
}

/**
 * Wrapper to qiwi_transmit
 * 
*/
void inline qiwi_transmit_error(uint8_t dst_addr, uint8_t error_code)
{
	qiwi_transmit(dst_addr, error_code, QIWI_NO_DATA, QIWI_NO_DATA_SIZE);
}

/**
 * Wrapper to qiwi_transmit. Don't need ack response. Transmits only once 
 */
static inline void qiwi_ack(uint8_t dst_addr)
{
	qiwi_transmit(dst_addr, QIWI_ACK, QIWI_NO_DATA, QIWI_NO_DATA_SIZE);
}

/*
 * Filling transmitter buffer
 */
static inline void write_into_transmitter_buffer(uint8_t dst_addr, uint8_t cmd, uint8_t* data, uint8_t size)
{
	transmitter_buffer[FBGN] 				= QIWI_FBGN;
	transmitter_buffer[DST_ADDR] 			= dst_addr;
	transmitter_buffer[SRC_ADDR] 			= device_addr;
	transmitter_buffer[VERSION] 			= QIWI_VERSION;
	transmitter_buffer[CMD] 				= cmd;
	transmitter_buffer_write_data(data, size);
	transmitter_buffer[SUM(size)] 			= calc_sum(transmitter_buffer, SUM(size));
	transmitter_buffer[FEND(size)] 			= QIWI_FEND;
}

/**
 * Datas from user code directily copy to transmitter 
 * buffer. It was made especially for realize 'put and forget' 
 * method. Because if user's code wants to transmitt other 
 * data, it can spoil current transmission data buffer
 */
static inline void transmitter_buffer_write_data(uint8_t* data, uint8_t size)
{
	int idx;
	for (register int i = 0; i < size; i++) 
	{
		idx = DATA_START + i;
		transmitter_buffer[idx] = data[i];
	}
}

/**
 * Calculates the check sum
 * Algorithm: Summarizes all bytes, if sum more than 255, gets only low byte
 **/
static uint8_t calc_sum(uint8_t* data, uint8_t size)
{
	uint16_t sum = 0;
	for (register int i = 0; i < size; i++) 
	{
		sum += data[i];
	}
		
	if (sum > QIWI_MAX_SUM)
	{
		sum = sum >> 8;
	}
	return (uint8_t)sum;
}

static bool check_sum(uint8_t* data, uint8_t size)
{
    uint16_t sum = 0;
	for (register int i = 0; i < size - 1; i++) 
	{
		sum += data[i];
	}
		
	if (sum > QIWI_MAX_SUM)
	{
		sum = sum >> 8;
	}
    return data[SUM(size)] == ((uint8_t)sum);
}

/*/
static bool check_sum(uint8_t size)
{
	uint8_t sum = calc_sum(receiver_buffer, size);
	return receiver_buffer[SUM(size)] == sum;
}
/*/

static inline uint8_t calc_sum(uint8_t* data, uint8_t size)
{
	uint16_t sum = 0;
	for (register int i = 0; i < size; i++)
	{
		sum += data[i];
	}
	
	if (sum > QIWI_MAX_SUM)
	{
		sum = sum >> 8;
	}
	return (uint8_t)sum;
}

/*/
static inline uint8_t calc_sum(uint8_t* data, uint8_t size)
{
	uint16_t sum = 0;
	for (register int i = 0; i < size; i++)
		sum += data[i];
		
	if (sum > QIWI_MAX_SUM)
		sum = sum >> 8;
		
	return (uint8_t)sum;
}
/*/















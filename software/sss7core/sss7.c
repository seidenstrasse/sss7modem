#include "sss7.h"

#include <string.h>

volatile enum sss7State sss7_state;

uint8_t sss7_rx_buffer[SSS7_RX_BUFFER_SIZE][SSS7_PAYLOAD_SIZE];
uint8_t sss7_rx_buffer_write;
uint8_t sss7_rx_buffer_read;
uint8_t sss7_rx_pos;

uint8_t sss7_tx_buffer[SSS7_PAYLOAD_SIZE];
uint8_t sss7_tx_pos;
uint8_t sss7_tx_crc;
volatile uint8_t sss7_tx_failed;
uint8_t sss7_tx_last_byte;
uint8_t sss7_tx_last_ack;

volatile uint8_t sss7_timeout_counter;


void sss7_init(void) {
	sss7_state = SSS7_IDLE;

	sss7_rx_pos = 0;
	sss7_rx_buffer_read = 0;
	sss7_rx_buffer_write = 0;

	sss7_tx_pos = 0;
	sss7_tx_crc = 0;
	sss7_tx_failed = 0;
}


static inline uint8_t sss7_payload_crc(uint8_t buffer[SSS7_PAYLOAD_SIZE]) {
	uint8_t crc = 0;

	for(uint8_t byte = 0; byte < SSS7_PAYLOAD_SIZE; byte++) {
		crc = crc ^ buffer[byte];
		for (uint8_t bit = 0; bit < 8; bit++)
		{
			if (crc & 0x01) {
				crc = (crc >> 1) ^ 0x8C;
			}
			else {
				crc >>= 1;
			}
		}
	}

	return crc;
}


static inline void sss7_send_byte(uint8_t byte) {
	sss7_tx_last_ack = 0;
	sss7_tx_last_byte = byte;
	uart_put_byte(byte);
}


void sss7_send(uint8_t msg[SSS7_PAYLOAD_SIZE]) {
	// Check that we can send, because we will overwritte the buffer
	if(sss7_state != SSS7_IDLE) {
		sss7_tx_failed = 1;
		return;
	}

	memcpy(sss7_tx_buffer, msg, SSS7_PAYLOAD_SIZE);

	sss7_tx_pos = 0;
	sss7_tx_crc = sss7_payload_crc(sss7_tx_buffer);

	// Commit to send state
	sss7_state = SSS7_TX_HEADER;
	sss7_tx_failed = 0;
	sss7_timeout_counter = 0;
	sss7_send_byte(SSS7_HEADER[0]);
}


void sss7_process_rx(void) {
	uint8_t byte = uart_get_byte();
	uint8_t crc = 0;
	uint8_t next_buffer_write = 0;
	sss7_timeout_counter = 0;

	switch(sss7_state) {
		case SSS7_IDLE:
			if(byte == SSS7_HEADER[0]) {
				sss7_state = SSS7_RX_HEADER;
			}
			else {
				sss7_state = SSS7_IDLE;
			}
		break;

		case SSS7_RX_HEADER:
			if(byte == SSS7_HEADER[1]) {
				sss7_state = SSS7_RX_PAYLOAD;
				sss7_rx_pos = 0;
			}
			else {
				sss7_state = SSS7_IDLE;
			}
		break;

		case SSS7_RX_PAYLOAD:
			sss7_rx_buffer[sss7_rx_buffer_write][sss7_rx_pos] = byte;
			sss7_rx_pos++;
			if(sss7_rx_pos >= SSS7_PAYLOAD_SIZE) {
				sss7_state = SSS7_RX_CRC;
			}
		break;

		case SSS7_RX_CRC:
			crc = sss7_payload_crc(sss7_rx_buffer[sss7_rx_buffer_write]);

			next_buffer_write = (sss7_rx_buffer_write + 1) % SSS7_RX_BUFFER_SIZE;
			// Ensure CRC is okay and we don't overwrite other frames
			if(byte == crc && next_buffer_write != sss7_rx_buffer_read) {
				sss7_rx_buffer_write = next_buffer_write;
			}
			sss7_state = SSS7_IDLE;
		break;

		// all the TX states
		default:
			if(byte != sss7_tx_last_byte) {
				sss7_state = SSS7_IDLE;
				sss7_tx_failed = 1;
			}
			else {
				sss7_tx_last_ack = 1;
			}
		break;
	}
}


void sss7_process_tx(void) {
	if(sss7_tx_last_ack) {
		uint8_t byte;
		switch(sss7_state) {
			case SSS7_TX_HEADER:
				sss7_send_byte(SSS7_HEADER[1]);
				sss7_state = SSS7_TX_PAYLOAD;
				sss7_tx_pos = 0;
			break;

			case SSS7_TX_PAYLOAD:
				byte = sss7_tx_buffer[sss7_tx_pos];
				sss7_send_byte(byte);
				sss7_tx_pos++;

				if(sss7_tx_pos >= SSS7_PAYLOAD_SIZE) {
					sss7_state = SSS7_TX_CRC;
				}
			break;

			case SSS7_TX_CRC:
				sss7_send_byte(sss7_tx_crc);
				sss7_state = SSS7_IDLE;
			break;

			default:
			break;
		}
	}
	else {
		sss7_tx_failed = 1;
		sss7_state = SSS7_IDLE;
	}
}

void sss7_process_ticks(uint16_t ticks) {
	if(sss7_state != SSS7_IDLE) {
		sss7_timeout_counter = sss7_timeout_counter + ticks;

		if(sss7_timeout_counter > sss7_timeout) {
			switch(sss7_state) {
				case SSS7_TX_HEADER:
				case SSS7_TX_PAYLOAD:
				case SSS7_TX_CRC:
					sss7_tx_failed = 1;
				default:
					sss7_state = SSS7_IDLE;
			}
		}
	}
}


void sss7_get_received(uint8_t msg[SSS7_PAYLOAD_SIZE]) {
	if(sss7_has_received()) {
		memcpy(msg, sss7_rx_buffer[sss7_rx_buffer_read], SSS7_PAYLOAD_SIZE);
		sss7_rx_buffer_read = (sss7_rx_buffer_read + 1) % SSS7_RX_BUFFER_SIZE;
	};
}

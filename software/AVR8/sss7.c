#include "sss7.h"

#include <avr/interrupt.h>
#include <util/crc16.h>
#include <string.h>

volatile enum sss7State sss7_state;

uint8_t sss7_rx_buffer[SSS7_RX_BUFFER_COUNT][SSS7_PAYLOAD_SIZE];
uint8_t sss7_rx_active_buffer;
uint8_t sss7_rx_oldest_buffer;
uint8_t sss7_rx_pos;

uint8_t sss7_tx_buffer[SSS7_PAYLOAD_SIZE];
uint8_t sss7_tx_pos;
uint8_t sss7_tx_crc;
volatile uint8_t sss7_tx_failed;
uint8_t sss7_tx_last_byte;
uint8_t sss7_tx_last_ack;

void sss7_init(void) {
	sss7_state = SSS7_IDLE;

	sss7_rx_pos = 0;
	sss7_rx_oldest_buffer = 0;
	sss7_rx_active_buffer = 0;

	sss7_tx_pos = 0;
	sss7_tx_crc = 0;
	sss7_tx_failed = 0;

	DDRB |= (1 << PB2)| (1 << PB3);
	PORTB |= (1 << PB2) | (1 << PB3);
}

static inline uint8_t sss7_payload_crc(uint8_t buffer[SSS7_PAYLOAD_SIZE]) {
	uint8_t crc = 0;
	for(uint8_t i = 0; i < SSS7_PAYLOAD_SIZE; i++) {
		crc = _crc_ibutton_update(crc, buffer[i]);
	}
	return crc;
}

static inline void sss7_send_byte(uint8_t byte) {
	sss7_tx_last_ack = 0;
	sss7_tx_last_byte = byte;
	UDR = byte;
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
	sss7_send_byte(SSS7_HEADER[0]);
}


ISR(USART_RXC_vect) {
	uint8_t byte = UDR;
	uint8_t crc = 0;

	switch(sss7_state) {
		case SSS7_IDLE:
			if(byte == SSS7_HEADER[0]) {
				sss7_state = SSS7_RX_HEADER;
				PORTB |= (1 << PB3);
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
			sss7_rx_buffer[sss7_rx_active_buffer][sss7_rx_pos] = byte;
			sss7_rx_pos++;
			if(sss7_rx_pos >= SSS7_PAYLOAD_SIZE) {
				sss7_state = SSS7_RX_CRC;
			}
		break;

		case SSS7_RX_CRC:
			crc = sss7_payload_crc(sss7_rx_buffer[sss7_rx_active_buffer]);
			if(byte == crc) {
				sss7_rx_active_buffer = (sss7_rx_active_buffer + 1) % SSS7_RX_BUFFER_COUNT;
			}
			PORTB &= ~(1 << PB3);
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

ISR(USART_TXC_vect) {

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

void sss7_get_received(uint8_t msg[SSS7_PAYLOAD_SIZE]) {
	if(sss7_has_received()) {
		memcpy(msg, sss7_rx_buffer[sss7_rx_oldest_buffer], SSS7_PAYLOAD_SIZE);
		sss7_rx_oldest_buffer = (sss7_rx_oldest_buffer + 1) % SSS7_RX_BUFFER_COUNT;
	};
}

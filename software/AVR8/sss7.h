#ifndef _SSS7_H_
#define _SSS7_H_

#include "stdint.h"

enum sss7State {
	SSS7_IDLE,
	SSS7_TX_HEADER,
	SSS7_TX_PAYLOAD,
	SSS7_TX_CRC,

	SSS7_RX_HEADER,
	SSS7_RX_PAYLOAD,
	SSS7_RX_CRC
};

const uint8_t SSS7_HEADER[] = {0xAA, 0xFE};

#define SSS7_PAYLOAD_SIZE 16
#define SSS7_RX_BUFFER_COUNT 2

extern volatile enum sss7State sss7_state;
extern uint8_t sss7_tx_failed;
extern uint8_t sss7_rx_active_buffer;
extern uint8_t sss7_rx_oldest_buffer;

void sss7_init();

static inline uint8_t sss7_can_send() {
	return sss7_state == SSS7_IDLE;
}

void sss7_send(uint8_t msg[SSS7_PAYLOAD_SIZE]);

static inline uint8_t sss7_send_failed() {
	return sss7_state != SSS7_IDLE && sss7_tx_failed;
}

static inline uint8_t sss7_has_received() {
	return sss7_rx_oldest_buffer < sss7_rx_active_buffer;
}



void sss7_get_received(uint8_t msg[SSS7_PAYLOAD_SIZE]);

#endif

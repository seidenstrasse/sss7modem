#ifndef _SSS7_H_
#define _SSS7_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#ifdef SSS7_THREADED
#define sss7_shared_modfier _Atomic
#else
#define sss7_shared_modfier volatile
#endif



enum sss7State {
	SSS7_IDLE,
	SSS7_TX_HEADER,
	SSS7_TX_PAYLOAD,
	SSS7_TX_CRC,

	SSS7_RX_HEADER,
	SSS7_RX_PAYLOAD,
	SSS7_RX_CRC
};

const static uint8_t SSS7_HEADER[] = {0xAA, 0xFE};
const static uint16_t sss7_timeout = 50;
const static uint16_t sss7_timeout_increment = 1;

#define SSS7_PAYLOAD_SIZE 16
#define SSS7_RX_BUFFER_SIZE 2


extern sss7_shared_modfier enum sss7State sss7_state;
extern sss7_shared_modfier uint8_t sss7_tx_failed;
extern uint8_t sss7_rx_buffer_write;
extern uint8_t sss7_rx_buffer_read;


void sss7_process_rx(void);
void sss7_process_tx(void);
void sss7_process_ticks(uint16_t ticks);

extern void uart_put_byte(uint8_t byte);
extern uint8_t uart_get_byte(void);


void sss7_init(void);

static inline uint8_t sss7_can_send(void) {
	return sss7_state == SSS7_IDLE;
}

void sss7_send(uint8_t msg[SSS7_PAYLOAD_SIZE]);

static inline uint8_t sss7_send_failed(void) {
	return sss7_tx_failed;
}

static inline uint8_t sss7_has_received(void) {
	return sss7_rx_buffer_read != sss7_rx_buffer_write;
}

void sss7_get_received(uint8_t msg[SSS7_PAYLOAD_SIZE]);

#ifdef __cplusplus
}
#endif

#endif

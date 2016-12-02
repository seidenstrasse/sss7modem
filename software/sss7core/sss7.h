#ifndef _SSS7_H_
#define _SSS7_H_

#include <stdint.h>

// In case this is used on arduino
#ifdef __cplusplus
extern "C" {
#endif

// sss7 for linux uses pthreads, which works better with _Atomic here
#ifdef SSS7_THREADED
#define sss7_shared_modfier _Atomic
#else
#define sss7_shared_modfier volatile
#endif


enum sss7State {
	SSS7_IDLE, 			// Idle state, waiting for something to happen

	SSS7_TX_HEADER,		// Sending the first header byte has started,
						// waiting for it to complete, to send the second byte

	SSS7_TX_PAYLOAD,	// Sending the paylaod of the frame

	SSS7_TX_CRC,		// Sending the last paylaod by has started,
						// waiting for the it to complete, to send crc byte

	SSS7_RX_HEADER,		// First header byte has been reived, waiting for the second one

	SSS7_RX_PAYLOAD,	// Receiving payload bytes

	SSS7_RX_CRC			// Last payload byte has been received, waiting for crc byte
};

// SSS7 Frame header
const static uint8_t SSS7_HEADER[] = {0xAA, 0xFE};
// Receive Timeout in milliseconds
const static uint16_t sss7_timeout = 150;
// TODO: Check if this is still neede
const static uint16_t sss7_timeout_increment = 1;

// Size of the payload
#define SSS7_PAYLOAD_SIZE 16
// Payloads in rx buffer
#define SSS7_RX_BUFFER_SIZE 2


// See sss7.c for more information
// State variable
extern sss7_shared_modfier enum sss7State sss7_state;
// Transmission failed flag
extern sss7_shared_modfier uint8_t sss7_tx_failed;
// rx buffer write position
extern sss7_shared_modfier uint8_t sss7_rx_buffer_write;
// tx buffer read postion
extern sss7_shared_modfier uint8_t sss7_rx_buffer_read;


// See sss7.c for more information
// Receive handler
void sss7_process_rx(void);
// Transmit handler
void sss7_process_tx(void);
// Timeout hanlder
void sss7_process_ticks(uint16_t ticks);


// Funtion to abstract sending byte with the uart
// Hardware dependent and has to be provided by the concrete implementation
extern void uart_put_byte(uint8_t byte);

// Funtion to abstract reading a byte from the uart
// Hardware dependent and has to be provided by the concrete implementation
extern uint8_t uart_get_byte(void);

// Initialization function, see sss7.c
void sss7_init(void);

// Checks if the sss7 statemachine is idle and can start sending a frame
static inline uint8_t sss7_can_send(void) {
	return sss7_state == SSS7_IDLE;
}

// Function for starting the transmission of a frame, see sss7.c
void sss7_send(uint8_t msg[SSS7_PAYLOAD_SIZE]);

// Readonly accessor to sss7_tx_failed
static inline uint8_t sss7_send_failed(void) {
	return sss7_tx_failed;
}

// Checks if there is an unread payload inside the rx buffer
static inline uint8_t sss7_has_received(void) {
	return sss7_rx_buffer_read != sss7_rx_buffer_write;
}

// Function to retrieve payloads from the rx buffer, see sss7.c
void sss7_get_received(uint8_t msg[SSS7_PAYLOAD_SIZE]);

#ifdef __cplusplus
}
#endif

#endif

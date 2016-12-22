#include "sss7.h"

#include <string.h>


// Current state
sss7_shared_modfier enum sss7State sss7_state;

// Position in the payload for receiving a frame
uint8_t sss7_rx_pos;

// Buffer for the payload for sending a frame
uint8_t sss7_tx_buffer[SSS7_PAYLOAD_SIZE];
// Position in the payload for sending a frame
uint8_t sss7_tx_pos;
// Crc for the payload used for sending a frame
uint8_t sss7_tx_crc;
// The last byte that has been send, used for collison checks in sss7_process_rx
uint8_t sss7_tx_last_byte;
// Flag set by sss7_process_rx to indicate that the received byte was sss7_tx_last_byte
uint8_t sss7_tx_last_ack;

// Flag to indicate wheter the last attempt to send a frame failed
sss7_shared_modfier uint8_t sss7_tx_failed;

// Counter that counts milliseconds since the last byte was received
// Incremented and checked by sss7_process_ticks.
// Reset by sss7_process_rx.
uint8_t sss7_timeout_counter;


// Buffers used for receiving payloads with space for SSS7_RX_BUFFER_SIZE payloads
uint8_t sss7_rx_buffer[SSS7_RX_BUFFER_SIZE][SSS7_PAYLOAD_SIZE];
// Payload in the buffer to write the received bytes to
// Note: The payload at sss7_rx_buffer_write can be modified at any time
// and should therefore never be read by sss7_get_received.
sss7_shared_modfier uint8_t sss7_rx_buffer_write;
// Next payload to read by sss7_get_received
sss7_shared_modfier uint8_t sss7_rx_buffer_read;


/*
 * Initialize sss7 state variables.
 * Resets all buffer positions to 0 and sets state to idle.
 */
void sss7_init(void) {
	sss7_state = SSS7_IDLE;

	sss7_rx_pos = 0;
	sss7_rx_buffer_read = 0;
	sss7_rx_buffer_write = 0;

	sss7_tx_pos = 0;
	sss7_tx_crc = 0;
	sss7_tx_failed = 0;
}


/*
 * Internal function to calculate the payload crc.
 * Uses the Maxim iButton 8-bit CRC also known as crc-8-maxim
 * Polynomial: x^8 + x^5 + x^4 + 1 (0x8C)
 * Initial value: 0x0
 */
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


/*
 * Internal function to send a single byte.
 * Assumes that it can use uart_put_byte to actually send the byte.
 */
static inline void sss7_send_byte(uint8_t byte) {
	sss7_tx_last_ack = 0;
	sss7_tx_last_byte = byte;
	uart_put_byte(byte);
}


/*
 * Triggers the transmission of a frame.
 */
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

/*
 * Handler function for received bytes.
 * Should be called for every received byte, by either interrupts or an eventloop.
 * It also resets the timeout timer.
 * uart_get_byte() is used to retreive the received byte.
 *
 * Note: This handler should by called BEFORE sss7_process_tx when sending.
 */
void sss7_process_rx(void) {
	uint8_t byte = uart_get_byte();
	uint8_t crc = 0;
	uint8_t next_buffer_write = 0;

	// Reset timeout counter
	sss7_timeout_counter = 0;

	switch((int) sss7_state) {
		// Handle the states for reiceiving a frame first
		case SSS7_IDLE:
			// First received byte should be the first header byte
			if(byte == SSS7_HEADER[0]) {
				sss7_state = SSS7_RX_HEADER;
			}
			else {
				sss7_state = SSS7_IDLE;
			}
		break;

		case SSS7_RX_HEADER:
			// Second received byte should be the second header byte
			if(byte == SSS7_HEADER[1]) {
				sss7_state = SSS7_RX_PAYLOAD;
				sss7_rx_pos = 0;
			}
			else {
				sss7_state = SSS7_IDLE;
			}
		break;

		case SSS7_RX_PAYLOAD:
			// Receive the payload
			sss7_rx_buffer[sss7_rx_buffer_write][sss7_rx_pos] = byte;
			sss7_rx_pos++;
			// Check if the payload has been fully received
			if(sss7_rx_pos >= SSS7_PAYLOAD_SIZE) {
				sss7_state = SSS7_RX_CRC;
			}
		break;

		case SSS7_RX_CRC:
			// Last byte of the frame
			crc = sss7_payload_crc(sss7_rx_buffer[sss7_rx_buffer_write]);

			// Calculate the next rx buffer position to use
			next_buffer_write = (sss7_rx_buffer_write + 1) % SSS7_RX_BUFFER_SIZE;
			// If the crc is not okay or there are no free buffer positions left,
			// the next frame will be written to the same rx buffer postion.
			// This effectively drops the currrent frame.
			if(byte == crc && next_buffer_write != sss7_rx_buffer_read) {
				sss7_rx_buffer_write = next_buffer_write;
			}

			// Go back to idle state
			sss7_state = SSS7_IDLE;
		break;

		// Handle all the TX states
		default:
			// Collision check:
			// If the byte received is not the last byte sent, something went wrong
			if(byte != sss7_tx_last_byte) {
				// Set the failure flag
				sss7_tx_failed = 1;
				// Reset to state to idle
				sss7_state = SSS7_IDLE;
			}
			else {
				// Set the ack flag which is checked by sss7_process_tx
				sss7_tx_last_ack = 1;
			}
		break;
	}
}


/*
 * Handler function for sent bytes.
 * Should be called for every compltely sent byte, by either interrupts or an eventloop.
 * Uses uart_put_byte() to send the next byte.
 *
 * Note: This handler should by called AFTER sss7_process_rx when sending.
 */
void sss7_process_tx(void) {
	// This handler is called since a byte has been sent and it should have been,
	// read back from the bus and handled by sss7_process_rx by now.
	// So if the last byte sent was acked by everthing is alright
	if(sss7_tx_last_ack) {
		uint8_t byte;
		switch((int) sss7_state) {
			case SSS7_TX_HEADER:
				// First byte of the frame is already sent by sss7_send_byte,
				// this handler is called since its transmission completed.
				// Therefore the second header byte is sent.
				sss7_send_byte(SSS7_HEADER[1]);
				// Reset the position inside the payload
				sss7_tx_pos = 0;
				sss7_state = SSS7_TX_PAYLOAD;
			break;

			case SSS7_TX_PAYLOAD:
				// Send the next payload byte and increment position
				byte = sss7_tx_buffer[sss7_tx_pos];
				sss7_send_byte(byte);
				sss7_tx_pos++;

				// All the payload bytes are sent, continue with crc
				if(sss7_tx_pos >= SSS7_PAYLOAD_SIZE) {
					sss7_state = SSS7_TX_CRC;
				}
			break;

			case SSS7_TX_CRC:
				// Send the crc precalculated by sss_send
				sss7_send_byte(sss7_tx_crc);
				// Reset the state to idle
				sss7_state = SSS7_TX_FINALIZE;
			break;

			case SSS7_TX_FINALIZE:
				//Not much to do here
				sss7_state = SSS7_IDLE;
			break;
			
			default:
				// Controlflow should never ever end up here.
				// Calling the tx handler while being in rx state is messed up.
			break;
		}
	}
	// If the byte was not acked, something went wrong
	else {
		// Set failure flag
		sss7_tx_failed = 1;
		// Reset state to idle
		sss7_state = SSS7_IDLE;
	}
}


/*
 * Handler function for timeout timer updates.
 * Should be called every once in a while either by interrupts by an eventloop.
 * The ticks parameter is the number of milliseconds that have passed since the last invocation.
 *
 * The timeout basically resolves the scenario where an partial frame is received without errors,
 * but never finished.
 */
void sss7_process_ticks(uint16_t ticks) {
	// There are no timeouts in idle state.
	if(sss7_state != SSS7_IDLE) {
		sss7_timeout_counter = sss7_timeout_counter + ticks;

		if(sss7_timeout_counter > sss7_timeout) {
			switch((int) sss7_state) {
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

/*
 * Function to retreive a received frame from the rx buffer.
 */
void sss7_get_received(uint8_t msg[SSS7_PAYLOAD_SIZE]) {
	// Make sure that sss7_rx_buffer_write != sss7_rx_buffer_read still holds
	if(sss7_has_received()) {
		// Copy the payload
		memcpy(msg, sss7_rx_buffer[sss7_rx_buffer_read], SSS7_PAYLOAD_SIZE);
		// Move to the next payload in the buffer
		sss7_rx_buffer_read = (sss7_rx_buffer_read + 1) % SSS7_RX_BUFFER_SIZE;
	};
}

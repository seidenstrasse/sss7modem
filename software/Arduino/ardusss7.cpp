#include "ardusss7.h"

#include <avr/io.h>
#include <avr/interrupt.h>

SSS7Wrapper SSS7;

SSS7Wrapper::SSS7Wrapper() {
	sss7_init();

	this->setupUart();
	this->setupTimer();
}

uint8_t SSS7Wrapper::canSend() {
	return sss7_can_send();
}

void SSS7Wrapper::send(uint8_t msg[SSS7_PAYLOAD_SIZE]) {
	return sss7_send(msg);
}

uint8_t SSS7Wrapper::sendFailed() {
	return sss7_send_failed();
}
uint8_t SSS7Wrapper::hasReceived() {
	return sss7_has_received();
}

void SSS7Wrapper::getReceived(uint8_t msg[SSS7_PAYLOAD_SIZE]) {
	sss7_get_received(msg);
}


void SSS7Wrapper::setupUart() {
	UBRR1H = UBRR_VAL >> 8;		// Setting baudrate
	UBRR1L = UBRR_VAL & 0xFF;

	UCSR1B = (1 << TXEN1) | (1 << RXEN1); // Enable TX and RX
	UCSR1C = (1 << UCSZ11) | (1 << UCSZ10);  // Asynchronous 8N1

	// flush UDR
	do
	{
		UDR1;
	}
	while (UCSR1A & (1 << RXC1));

	// reset tx and rx complete flags
	UCSR1A = (1 << RXC1) | (1 << TXC1);

	UCSR1B |= (1 << TXCIE1) | (1 << RXCIE1);  // enable tx and rx interrupts
}

void uart_put_byte(uint8_t byte) {
	UDR1 = byte;
}

uint8_t uart_get_byte() {
	return UDR1;
}

ISR(USART_RXC1_vect) {
	sss7_process_rx();
}

ISR(USART_TXC1_vect) {
	sss7_process_tx();
}

//TODO: Setup Timer

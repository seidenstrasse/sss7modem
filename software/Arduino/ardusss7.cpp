#include "ardusss7.h"

#include <avr/io.h>
#include <avr/interrupt.h>

SSS7Wrapper SSS7;

void SSS7Wrapper::init() {
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
	UBRR2H = UBRR_VAL >> 8;		// Setting baudrate
	UBRR2L = UBRR_VAL & 0xFF;

	UCSR2B = (1 << TXEN2) | (1 << RXEN2); // Enable TX and RX
	UCSR2C = (1 << UCSZ21) | (1 << UCSZ20);  // Asynchronous 8N1

	// flush UDR
	do
	{
		UDR2;
	}
	while (UCSR2A & (1 << RXC2));

	// reset tx and rx complete flags
	UCSR2A = (1 << RXC2) | (1 << TXC2);

	UCSR2B |= (1 << TXCIE2) | (1 << RXCIE2);  // enable tx and rx interrupts
}

void uart_put_byte(uint8_t byte) {
	UDR2 = byte;
}

uint8_t uart_get_byte() {
	return UDR2;
}

ISR(USART2_RX_vect) {
	sss7_process_rx();
}

ISR(USART2_TX_vect) {
	sss7_process_tx();
}


void SSS7Wrapper::setupTimer() {
	TCCR4B = 0;
	TCNT4 = 65535 - 16000;	//Preload for 16000 ticks to overflow

	// Take the Timer by force ...
	TCCR4A = 0;
	TCCR4B = (1 << CS40);	// Prescaler 1
	TCCR4C = 0;

	TIMSK4 = (1 << TOIE4);
}

ISR(TIMER4_OVF_vect) {
	TCNT4 = 65535 - 16000;	//Preload for 16000 ticks to overflow

	sss7_process_ticks(sss7_timeout_increment);
}

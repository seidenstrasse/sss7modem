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
	UBRR0H = UBRR_VAL >> 8;		// Setting baudrate
	UBRR0L = UBRR_VAL & 0xFF;

	UCSR0B = (1 << TXEN0) | (1 << RXEN0); // Enable TX and RX
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);  // Asynchronous 8N1

	// flush UDR
	do
	{
		UDR0;
	}
	while (UCSR0A & (1 << RXC0));

	// reset tx and rx complete flags
	UCSR0A = (1 << RXC0) | (1 << TXC0);

	UCSR0B |= (1 << TXCIE0) | (1 << RXCIE0);  // enable tx and rx interrupts
}

void uart_put_byte(uint8_t byte) {
	UDR0 = byte;
}

uint8_t uart_get_byte() {
	return UDR0;
}

ISR(USART0_RX_vect) {
	sss7_process_rx();
}

ISR(USART0_TX_vect) {
	sss7_process_tx();
}


void SSS7Wrapper::setupTimer() {
	TCCR1B = 0;
	TCNT1 = 65535 - 16000;	//Preload for 16000 ticks to overflow

	// Take the Timer by force ...
	TCCR1A = 0;
	TCCR1B = (1 << CS10);	// Prescaler 1
	TCCR1C = 0;

	TIMSK1 = (1 << TOIE1);
}

ISR(TIMER1_OVF_vect) {
	TCNT1 = 65535 - 16000;	//Preload for 16000 ticks to overflow

	sss7_process_ticks(sss7_timeout_increment);
}

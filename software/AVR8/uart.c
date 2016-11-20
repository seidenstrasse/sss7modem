#include "uart.h"

#include <avr/interrupt.h>
#include <util/crc16.h>

#include "sss7.h"

void uart_init(void) {
	UBRRH = UBRR_VAL >> 8;		// Setting baudrate
	UBRRL = UBRR_VAL & 0xFF;

	UCSRB = (1 << TXEN) | (1 << RXEN); // Enable TX and RX
	UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);  // Asynchronous 8N1

	// flush UDR
    do
    {
        UDR;
    }
    while (UCSRA & (1 << RXC));

	// reset tx and rx complete flags
	UCSRA = (1 << RXEN) | (1 << TXC);

	UCSRB |= (1 << TXCIE) | (1 << RXCIE);  // enable tx and rx interrupts
}


ISR(USART_RXC_vect) {
	sss7_process_rx();
}

ISR(USART_TXC_vect) {
	sss7_process_tx();
}

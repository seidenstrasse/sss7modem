#include "timer.h"

#include <avr/interrupt.h>
#include <avr/io.h>

#include "sss7.h"

/*
* Timer setting:
* MCU running at 16MHz:
* Prescaler is 64 which results in 250000 ticks per second
* Preloading the counter with 6 leads to 1000 overflow interrupts per second
* or one overflow every millisecond.
*/

void timer_init(void) {
	TCNT0 = 6;	//Preload for 250 ticks to overflow
	TIMSK |= (1 << TOIE0);
	TCCR0 = (1 << CS00) | (1 << CS01);	// Prescaler 64
}



ISR(TIMER0_OVF_vect) {
	TCNT0 = 6; //Preload for 250 ticks to overflow

	sss7_process_ticks(sss7_timeout_increment);
}

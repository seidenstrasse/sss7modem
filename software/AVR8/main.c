#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>

#include "uart.h"
#include "sss7.h"




int main(void) {

uint8_t msg[SSS7_PAYLOAD_SIZE];
memset(msg, 0, SSS7_PAYLOAD_SIZE);
msg[0] = 'H';
msg[1] = 'e';
msg[2] = 'l';
msg[3] = 'l';
msg[4] = 'o';
msg[5] = ' ';
msg[6] = 'W';
msg[7] = 'o';
msg[8] = 'r';
msg[9] = 'l';
msg[10] = 'd';



uart_init();
sss7_init();
sei();

while(1) {

    if(sss7_can_send()) {
        sss7_send(msg);
    }
    while(!sss7_can_send());
    if(sss7_send_failed()) {
        PORTB ^= (1 << PB2);
    }

    _delay_ms(250);
}


}

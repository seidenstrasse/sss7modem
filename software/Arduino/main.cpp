#include <Arduino.h>
#include "ardusss7.h"

void setup() {
	Serial.begin(9600);

	Serial.print("Initializing SSS7...");
	SSS7.init();
	Serial.println("Done !");

};


void loop() {
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


    while(1) {

		Serial.println("Waiting to send");
        while(!SSS7.canSend());
		Serial.println("Sending ...");
        SSS7.send(msg);
        while(!SSS7.canSend());
		//Serial.println("Finished Sending");
        if(SSS7.sendFailed()) {
            Serial.println("Send failed");
        }

        _delay_ms(1000);
    }

};

#include <Arduino.h>
#include "ardusss7.h"

void setup() {
	//Serial.begin(9600);

	//Serial.print("Initializing SSS7...");
	SSS7.init();
	//Serial.println("Done !");

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

	pinMode(13, OUTPUT);

    while(1) {
        while(!SSS7.canSend());

        SSS7.send(msg);
        while(!SSS7.canSend());

        if(SSS7.sendFailed()) {
            // Do something
        }


		if(SSS7.hasReceived()) {
			uint8_t msg1[SSS7_PAYLOAD_SIZE];
			SSS7.getReceived(msg1);

			digitalWrite(13, HIGH);
			_delay_ms(500);
		}
		digitalWrite(13, LOW);

        _delay_ms(1000);
    }

/*
	uint8_t msg[5][SSS7_PAYLOAD_SIZE];
	for(uint8_t i = 0; i < 5; i++) {
		memset(msg[i], 0, SSS7_PAYLOAD_SIZE);
	}

	msg[0][0] = 0x06;
	msg[0][1] = 0;
	msg[0][2] = 2;

	msg[1][0] = 0x00;
	msg[1][1] = 11;
	msg[1][2] = 0;

	msg[2][0] = 0x00;
	msg[2][1] = 12;
	msg[2][2] = 0;

	msg[3][0] = 0x00;
	msg[3][1] = 13;
	msg[3][2] = 0;

	msg[4][0] = 0x00;
	msg[4][1] = 8;
	msg[4][2] = 0;

    while(1) {

		for(uint8_t i = 0; i < 5; i++) {
			Serial.println("Waiting to send");
	        while(!SSS7.canSend());
			Serial.print("Sending ");
			Serial.print(i);
			Serial.println(" ...");
	        SSS7.send(msg[i]);
			while(!SSS7.canSend());
			Serial.println("Finished Sending");
			if(SSS7.sendFailed()) {
				Serial.println("Send failed");
			}


	        _delay_ms(1000);
		}
    }
	*/

};

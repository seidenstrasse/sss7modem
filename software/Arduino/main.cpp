#include <Arduino.h>
#include "ardusss7.h"
#include "ss7content.h"
#include <AccelStepper.h>

#define ENDSTOP1 6
#define S1_ENABLE 2
#define MICROSTEP A5

#define MAXPOSITIONS 6
#define DISTANCE 1000
uint16_t position[MAXPOSITIONS]{60,1060,2060,3085,4085,5085};

#define ID 3

void init_stepper(){
    pinMode(ENDSTOP1, INPUT_PULLUP);
    
    pinMode(S1_ENABLE, OUTPUT); //ENABLE STEPPER
    digitalWrite(S1_ENABLE, LOW);
    
    pinMode(MICROSTEP, OUTPUT);
    digitalWrite(MICROSTEP, LOW);
    
    stepper.setAcceleration(1500);
    stepper.setMaxSpeed(3000);
    
}

void home(){
    if(!digitalRead(ENDSTOP1)){
        stepper.runToNewPosition(100);
    }
    uint16_t stepout = 0;
    while(digitalRead(ENDSTOP1 && (stepout <= (DISTANCE*MAXPOSITIONS)))){
        stepper.moveTo(-10);
        stepout += 10;
        stepper.run();
        stepper.setCurrentPosition(0);
        // stepper.runToNewPosition(0);
        //stepper.run();
        digitalWrite(11,LOW); //Kein plan
    }
}

void select(uint8_t tube){
    if(tube>0 && tube <= MAXPOSITIONS){
        stepper.runToNewPosition(position(tube));
    }
    else{
        home();
    }
}


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
		//Serial.println("Waiting to send");
        //while(!SSS7.canSend());
		//Serial.println("Sending ...");
        //SSS7.send(msg);
        //while(!SSS7.canSend());
		//Serial.println("Finished Sending");
        //if(SSS7.sendFailed()) {
        //    Serial.println("Send failed");
       // }
		if(SSS7.hasReceived()) {
			uint8_t msg1[SSS7_PAYLOAD_SIZE];
			SSS7.getReceived(msg1);
			Serial.print("Got data:");
			Serial.println((char*)msg1);
            if((msg1[0]==MSG_CONNECT)&&(msg[2]==ID)){
                select(msg1[3]);
            }
		}

    }

};

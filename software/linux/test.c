#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "libsss7.h"

int main(int argc, char const *argv[]) {
	libsss7_start("/dev/ttyUSB0");

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


    //while(1) {

        while(!libsss7_can_send());
        libsss7_send(msg);
        while(!libsss7_can_send());
        if(libsss7_send_failed()) {
			printf("Send failed\n");
        }
        sleep(5);
    //}


	libsss7_send(msg);

	libsss7_stop();
	return 0;
}

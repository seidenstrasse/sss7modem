#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "libsss7.h"

int main(int argc, char const *argv[]) {
	libsss7_start("/dev/ttyUSB0");

	uint8_t msg[LIBSSS7_PAYLOAD_SIZE];
    memset(msg, 0, LIBSSS7_PAYLOAD_SIZE);
    msg[0] = 'H';
    msg[1] = 'a';
    msg[2] = 'l';
    msg[3] = 'l';
    msg[4] = 'o';
    msg[5] = ' ';
    msg[6] = 'W';
    msg[7] = 'e';
    msg[8] = 'l';
    msg[9] = 't';


    while(!libsss7_can_send());
    libsss7_send(msg);
    while(!libsss7_can_send());
	printf("Send done\n");
    if(libsss7_send_failed()) {
		printf("Send failed\n");
    }

	while(!libsss7_has_received());
	uint8_t tmp[LIBSSS7_PAYLOAD_SIZE + 1];
	memset(tmp, 0, LIBSSS7_PAYLOAD_SIZE + 1);
	libsss7_get_received(tmp);
	printf("Received: %s\n", tmp);




	libsss7_send(msg);

	libsss7_stop();
	return 0;
}

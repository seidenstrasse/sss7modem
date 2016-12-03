#ifndef _LIBSSS7_H_
#define _LIBSSS7_H_

#include<stdint.h>

#define LIBSSS7_PAYLOAD_SIZE 16

// See libsss7.c for more detailed descriptions

// Setup function
int libsss7_start(char *serialport);

// Checks if a frame can be sent
int libsss7_can_send(void);

// Send a frame with the payload msg
void libsss7_send(uint8_t msg[LIBSSS7_PAYLOAD_SIZE]);

// Check if the last transmission failed
int libsss7_send_failed(void);

// Check if there are received payloads in the rx buffer
int libsss7_has_received(void);

// Get a payload from the rx buffer
void libsss7_get_received(uint8_t *msg);

// Teardown function
void libsss7_stop();

#endif

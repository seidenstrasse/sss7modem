#ifndef _LIBSSS7_H_
#define _LIBSSS7_H_

#include "sss7.h"

int libsss7_start(char *serialport);

int libsss7_can_send(void);

void libsss7_send(uint8_t *msg);

int libsss7_send_failed(void);

int libsss7_has_received(void);

void libsss7_get_received(uint8_t *msg);

void libsss7_stop();

#endif

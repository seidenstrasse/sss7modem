#!/usr/bin/env python2

from ctypes import *


LIBSSS7_PATH = "../linux/bin/libsss7.so"
LIBSSS7_PAYLOAD_SIZE = 16

_SSS7_PAYLOAD_TYPE = c_ubyte * LIBSSS7_PAYLOAD_SIZE

_LIB_SSS7 = cdll.LoadLibrary(LIBSSS7_PATH)

# int libsss7_start(char *serialport);
_LIB_SSS7.libsss7_start.argtypes = [c_char_p]
_LIB_SSS7.libsss7_start.restype = c_int

# int libsss7_can_send(void);
_LIB_SSS7.libsss7_can_send.argtypes = None
_LIB_SSS7.libsss7_can_send.restype = c_int

# void libsss7_send(uint8_t msg[LIBSSS7_PAYLOAD_SIZE]);
_LIB_SSS7.libsss7_send.argtypes = [_SSS7_PAYLOAD_TYPE]
_LIB_SSS7.libsss7_send.restype = c_int

# int libsss7_send_failed(void);
_LIB_SSS7.libsss7_send_failed.argtypes = None
_LIB_SSS7.libsss7_send_failed.restype = c_int

# int libsss7_has_received(void);
_LIB_SSS7.libsss7_has_received.argtypes = None
_LIB_SSS7.libsss7_has_received.restype = c_int

# libsss7_get_received(uint8_t *msg);
_LIB_SSS7.libsss7_get_received.argtypes = [_SSS7_PAYLOAD_TYPE]
_LIB_SSS7.libsss7_get_received.restype = c_int

# void libsss7_stop();
_LIB_SSS7.libsss7_stop.argtypes = None
_LIB_SSS7.libsss7_stop.restype = None

class _SSS7(object):

    def start(self, port):
        return _LIB_SSS7.libsss7_start(port) == 0

    def can_send(self):
        return _LIB_SSS7.libsss7_can_send() == 1

    def send(self,msg):
        msg += [0] * (LIBSSS7_PAYLOAD_SIZE - len(msg))
        payload = _SSS7_PAYLOAD_TYPE(*msg)
        _LIB_SSS7.libsss7_send(payload)

    def send_failed(self):
        tmp = _LIB_SSS7.libsss7_send_failed()
        print tmp
        return tmp == 1

    def has_received(self):
        return _LIB_SSS7.libsss7_has_received()

    def get_received(self):
        payload = _SSS7_PAYLOAD_TYPE()
        _LIB_SSS7.libsss7_get_received(payload)
        return [payload[i] for i in range(0, LIBSSS7_PAYLOAD_SIZE)]

    def stop(self):
        _LIB_SSS7.libsss7_stop()

SSS7 = _SSS7()

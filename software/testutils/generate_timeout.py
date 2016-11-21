#!/usr/bin/env python2

import sys
import serial


def send_byte(ser, byte):
    ser.reset_input_buffer()
    ser.write(byte)
    read_byte = ser.read()
    if read_byte != byte:
        print "Written %s read %s" % (hex(ord(byte)), hex(ord(read_byte)))
        sys.exit(-1)


def main():
    if len(sys.argv) != 2:
        print "Usage %s <port>" % (sys.argv[0])
        sys.exit(-1)

    ser = serial.Serial(sys.argv[1], 9600, timeout=0.40)

    send_byte(ser, chr(0xAA))
    send_byte(ser, chr(0xFE))

    for byte in "Unfinished":
        send_byte(ser, byte)



if __name__ == '__main__':
    main()

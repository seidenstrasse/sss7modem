#!/usr/bin/env python2

import sys
import serial
import crcmod.predefined


def send_byte(ser, byte):
    ser.reset_input_buffer()
    ser.write(byte)
    read_byte = ser.read()
    if read_byte != byte:
        print "Written %s read %s" % (hex(ord(read_byte)), hex(ord(byte)))
        sys.exit(-1)



def main():
    if len(sys.argv) != 3:
        print "Usage %s <port> <payload>" % (sys.argv[0])
        sys.exit(-1)

    payload = sys.argv[2]
    payload += chr(0) * (16 - len(payload))

    ser = serial.Serial(sys.argv[1], 9600, timeout=0.40)
    crc = crcmod.predefined.Crc('crc-8-maxim')

    send_byte(ser, chr(0xAA))
    send_byte(ser, chr(0xFE))

    for byte in payload:
        crc.update(byte)
        send_byte(ser, byte)

    crc_sum = crc.digest()
    send_byte(ser, crc_sum)

    print "Send sucessfully !"

if __name__ == '__main__':
    main()

#!/usr/bin/env python2

import sys
import serial
import crcmod.predefined


def hexdump(data):
    res = ""
    for char in data:
        res += "%X " % (ord(char))

    return res



def main():
    if len(sys.argv) != 2:
        print "Usage %s <port>" % (sys.argv[0])
        sys.exit(-1)

    ser = serial.Serial(sys.argv[1], 9600, timeout=0.40)

    data = ""
    while True:
        while data == "":
            data = ser.read(19)

        if len(data) != 19:
            print "No Frame:\t" + hexdump(data)


        header_ok = data.startswith(chr(0xAA) + chr(0xFE))

        line = ""
        if header_ok:
            line += "Header Ok       "
        else:
            line += "Header not Ok   "

        payload = data[2:18]
        crc = crcmod.predefined.Crc('crc-8-maxim')
        crc.update(payload)
        crc_sum = crc.digest()
        if data[18] == crc_sum:
            line += "CRC Ok      "
        else:
            line += "CRC not Ok  "

        line += hexdump(payload)
        print line


if __name__ == '__main__':
    main()

#!/usr/bin/env python2

import sys
import serial

def hexdump(data):
    res = ""
    for char in data:
        res += "%X " % (ord(char))

    return res

if len(sys.argv) < 2:
    print "Usage %s <port>" % (sys.argv[0])
    sys.exit(-1)

ser = serial.Serial(sys.argv[1], 9600, timeout=0.40)




while True:
    data = ser.read(19)
    if data != "" and len(data) != 19:
        print "No Frame:\t" + hexdump(data)

    header_ok = data.startswith(chr(0xAA) + chr(0xFE))

    line = ""
    if header_ok:
        line += "Header OK\t"
    else:
        line += "Header not OK\t"

    line += hexdump(data)
    print line

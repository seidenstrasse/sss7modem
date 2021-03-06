#!/usr/bin/env python2

from time import sleep

from sss7 import SSS7

def to_byte_list(data):
    return [ord(c) for c in data]

def to_string(data):
    data = [chr(c) for c in data]
    return "".join(data)

def main():
    SSS7.start("/dev/ttyUSB0")

    while not SSS7.can_send() :
        sleep(0.1)

    SSS7.send(to_byte_list("Hello python"))
    while not SSS7.can_send():
        sleep(0.1)

    if SSS7.send_failed():
        print "Send failed"


    while not SSS7.has_received():
        sleep(0.1)

    print to_string(SSS7.get_received())



    SSS7.stop()


if __name__ == '__main__':
    main()

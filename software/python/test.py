#!/usr/bin/env python2

from time import sleep

from sss7 import SSS7

packets = [[1, 0, 33, 2],
           [1, 0, 22, 2]]

def to_byte_list(data):
    return [ord(c) for c in data]

def to_string(data):
    data = [chr(c) for c in data]
    return "".join(data)

def main():
    SSS7.start("/dev/ttyUSB0")

    for packet in packets:
        while not SSS7.can_send():
            sleep(0.1)

        print "Paket: %s" % packet
        SSS7.send(packet);

        #while(not SSS7.has_received()):
        #    sleep(0.01)

        #SSS7.get_received()
        raw_input("Next ?")


    #for i in [1,2,0]:
        #while not SSS7.can_send():
        #    sleep(0.1)

        #print "Moving to %d" % i
        #SSS7.send([8, 0, 255, i]);

        #while(not SSS7.has_received()):
        #    sleep(0.01)

        #SSS7.get_received()
        raw_input("Next ?")



    SSS7.stop()


if __name__ == '__main__':
    main()

#!/usr/bin/env python2

import sys

from cmd import Cmd
from time import sleep

from sss7 import SSS7


class Prompt(Cmd):
    prompt = "SSS7> "
    air_args = {'push' : 1,
                'pull' : 2,
                'off': 0}

    connect_routers = {'11' : ['0', '1', '2', '50'],
                       '22' : ['0', '1', '2', '50'],
                       '33' : ['0', '1', '2', '3', '4', '50']}

    def do_help(self, args):
        print "Commands are:"
        print "raw <byte 0> ... <byte 15>"
        print "connnect <router> <output>"
        print "air <push/pull/off>"
        print "exit"

    def do_exit(self, args):
        sys.exit()

    def do_raw(self, args):
        args = args.strip().split()

        if len(args) > 16:
            print "Payload can only contain 16 bytes"

        payload = []
        for byte in args:

            try:
                byte = int(byte)
            except ValueError:
                print "Arguments should be integers"
                return

            if byte > 255 or byte < 0:
                print "Payload bytes must be in range 0..255"
                return
            payload += [byte]

        self.send_payload(payload)


    def do_connect(self, args):
        args = args.strip().split()

        if len(args) != 2:
            print "Usage: connnect <router> <output>"
            return

        try:
            router = int(args[0])
            output = int(args[1])
        except ValueError:
            print "Arguments should be integers"
            return

        self.send_payload_wait([1, 0, router, output])


    def complete_connect(self, text, line, begin_index, end_index):
        args = line.strip().split()[1:]

        matches = []
        if len(args) == 0 or (len(args) == 1 and not line.endswith(' ')):
            matches = self.connect_routers.keys()
            matches = filter(lambda x: x.startswith(text), matches)

        elif len(args) == 1 or (len(args) == 2 and not line.endswith(' ')):
            if args[0] in self.connect_routers:
                matches = self.connect_routers[args[0]]

        matches = filter(lambda x: x.startswith(text), matches)
        return matches


    def do_air(self, args):
        args = args.strip().split()

        if len(args) != 1 or not args[0] in self.air_args:
            print "Usage: air <push/pull/off>"
            return

        command = self.air_args[args[0]]
        if args[0] == "off":
            self.send_payload([8, 0, 255, command])
        else:
            self.send_payload_wait([8, 0, 255, command])


    def complete_air(self, text, line, begin_index, end_index):
        matches = self.air_args.keys()
        matches = filter(lambda x: x.startswith(text), matches)
        return matches


    def send_payload(self, payload):
        while not SSS7.can_send():
            sleep(0.1)

        print "Sending payload %s" % payload
        SSS7.send(payload)

        while not SSS7.can_send():
            sleep(0.1)

        if not SSS7.send_failed():
            print "Sucess !"
        else:
            print "Failed !"


    def send_payload_wait(self, payload):
            while not SSS7.can_send():
                sleep(0.1)

            print "Sending payload %s" % payload
            SSS7.send(payload)

            while not SSS7.can_send():
                sleep(0.1)

            if SSS7.send_failed():
                print "Failed !"

            timeout = 20
            while timeout > 0 and not SSS7.has_received():
                sleep(1)
                timeout -= 1
                sys.stdout.write('.')
                sys.stdout.flush()

            print ""

            if not SSS7.has_received():
                print "No response"
                return

            print "Received %s" % SSS7.get_received()


def main():
    if len(sys.argv) != 2:
        print "Usage %s <port>" % sys.argv[0]
        sys.exit(0)

    if not SSS7.start(sys.argv[1]):
        print "Could not start SSS7"
        sys.exit(-1)

    prompt = Prompt()
    prompt.cmdloop()


if __name__ == '__main__':
    main()

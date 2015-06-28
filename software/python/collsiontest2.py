#!/usr/bin/env python2

import random
import time

from sss7bus import SSS7Bus


bus = SSS7Bus('/dev/ttyUSB1')

counter = 0

while True:
	bus.send_message("Timestamp:%d:%d" % (counter, time.time()))
	while bus.has_message():
		bus.read_message()

	counter += 1

	time.sleep((500.0 + random.randint(0,500)) / 1000.0)

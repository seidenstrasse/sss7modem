#!/usr/bin/env python2

import random
import time

from sss7bus import SSS7Bus


bus = SSS7Bus('/dev/ttyUSB0')

framecount = 0
lost = 0

while True:
	bus.send_message("Hello World!")

	more = True
	while more:
		msg = bus.read_message()
		print msg
		print framecount
		frame = int(msg.split(':')[1])
		delta = frame - framecount
		if delta > 1 and framecount <> 0:
			lost += delta

		framecount = frame

		if frame % 100 == 0 and frame > 0:
			print "Lost Frames: %d %d %f" % (frame, lost, 100.0 * lost / frame)

		more = bus.has_message()

	time.sleep((500.0 + random.randint(0,500)) / 1000.0)

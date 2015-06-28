#!/usr/bin/env python2

import serial
import random
import time
from crcmod.predefined import PredefinedCrc


# Timings in bit times
TIMEOUT = 48
CLEARCHANNEL = 2 * TIMEOUT


class SSS7Bus(object):

	def __init__(self, port, baudrate=9600):
		random.seed()

		self._bit_time = 1.0 / baudrate

		self._serial = serial.Serial(port, baudrate, timeout=self._bit_time * TIMEOUT)

		self._buffer = []


	def _debug(self, msg):
		print "[SSS7Bus] %s" % msg


	def _flush_input_buffer(self):
		while self._serial.inWaiting() > 0:
			self._debug("Flushing input buffer")
			self._read_frame()


	def _can_send(self):
		self._flush_input_buffer()

		self._serial.timeout = self._bit_time * CLEARCHANNEL
		self._debug("Checking if bus is idle for %f seconds" % (self._serial.timeout))

		first_byte = self._serial.read(1)
		if len(first_byte) == 0:
			self._debug("Bus seems idle")
			return True

		self._debug("Bus is not idle, reading frames")
		self._read_frame_rest(first_byte)

		return False


	def _read_frame(self):
		self._debug("Trying to read a frame from the bus")
		self._serial.timeout = timeout=self._bit_time * TIMEOUT

		first_byte = self._serial.read(1)
		if len(first_byte) == 0:
			self._debug("Timeout reading first byte")
			return False

		return self._read_frame_rest(first_byte)


	def _read_frame_rest(self, first_byte):
		if ord(first_byte) <> 0xAA:
			self._debug("Wrong first byte: %s" % hex(ord(first_byte)))
			return False

		second_byte = self._serial.read(1)
		if len(second_byte) == 0:
			self._debug("Timeout reading second byte")
			return False

		if ord(second_byte) <> 0xFE:
			self._debug("Wrong second byte")
			return False


		length_byte = self._serial.read(1)
		if len(length_byte) == 0:
			self._debug("Timeout reading length byte")
			return False

		length = ord(length_byte) - 1 # we read the size length byte already

		self._serial.timeout = timeout=self._bit_time * TIMEOUT

		self._debug("Trying to read remaing %d bytes of frame" % length)
		data = self._serial.read(length)

		# if read returns less then length, no new byte has been received for timeout seconds
		if len(data) != length:
			self._debug("Timeout reading frame payload")
			return False

		crc = data[-2:]
		payload = data[:-2]

		crc16 = PredefinedCrc('crc16')
		crc16.update(payload)
		real_crc = crc16.digest()

		if real_crc != crc:
			self._debug("Wrong crc for frame payload")
			return True

		self._debug("Sucessfully read rest of frame")
		self._buffer.insert(0,payload)

		return True



	def _send_byte(self, byte):
		self._serial.write(byte)
		return byte == self._serial.read(1)


	def _send_frame(self, frame):
		while not self._can_send():
			pass

		for byte in frame:
			result = self._send_byte(byte)
			if not result:
				self._debug("Sending frame failed with collision")
				return False

		self._debug("Successfully send frame")
		return True


	def send_message(self, msg, priority=5):
		if len(msg) > 253:
			raise ValueError("Message length can not exceed 253 byte")

		crc16 = PredefinedCrc('crc16')
		crc16.update(msg)
		msg_crc = crc16.digest()

		frame = chr(0xAA) + chr(0xFE) + chr(len(msg) + 3) + msg + msg_crc

		result = self._send_frame(frame)

		while not result:
			backoff = TIMEOUT * self._bit_time * (4 + priority + random.randint(1,5))
			self._debug("Collision occured backing off %f" % backoff)
			time.sleep(backoff)
			result = self._send_frame(frame)


	def has_message(self):
		return len(self._buffer) > 0

	def read_message(self):
		while not self.has_message():
			self._read_frame()

		return self._buffer.pop()

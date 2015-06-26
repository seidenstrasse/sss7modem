#!/usr/bin/env python2

import serial
import random
import time
from crcmod.predefined import PredefinedCrc


# Timings in bit times
TIMEOUT = 24
CLEARCHANNEL = 48


class SSS7Bus(object):
	ERR_SUCCESS = 0
	ERR_TIMEOUT_ON_LENGTH = -1
	ERR_TIMEOUT_ON_PAYLOAD = -2
	ERR_CRC_ERRROR = -3


	def __init__(self, port, baudrate=9600):
		self._bit_time = 1.0 / baudrate

		self._serial = serial.Serial(port, baudrate, timeout=self._bit_time * TIMEOUT)

		self._buffer = []

	
	def _debug(self, msg):
		print "[SSS7Bus] %s" % msg


	def _read_frame(self, clear_channel=False):
		if clear_channel :
			self._serial.timeout = self._bit_time * CLEARCHANNEL
			self._debug("Checking if bus is idle for %f seconds" % (self._bit_time * CLEARCHANNEL))
		else:
			self._debug("Trying to read a frame from the bus")
			self._serial.timeout = timeout=self._bit_time * TIMEOUT
		
		length_byte = self._serial.read(1)
		if len(length_byte) == 0:
			if clear_channel:
				self._debug("Bus was idle")
			else:	
				self._debug("Timeout reading frame length")
			return self.ERR_TIMEOUT_ON_LENGTH

		if clear_channel:
			self._debug("Bus was not idle, trying to read frame")

		length = ord(length_byte) - 1 # we read the size length byte already
		
		self._serial.timeout = timeout=self._bit_time * TIMEOUT
	
		self._debug("Trying to read remaing %d bytes of frame" % length)
		data = self._serial.read(length)
		
		# if read returns less then length, no new byte has been received for timeout seconds
		if len(data) != length:
			self._debug("Timeout reading frame payload")
			return self.ERR_TIMEOUT_ON_PAYLOAD

		crc = data[-2:]
		payload = data[:-2]
		
		crc16 = PredefinedCrc('crc16')
		crc16.update(payload)
		real_crc = crc16.digest()

		if real_crc != crc:
			self._debug("Wrong crc for frame payload")
			return self.ERR_CRC_ERRROR

		self._debug("Sucessfully read rest of frame")
		self._buffer.insert(0,data)
	
		return self.ERR_SUCCESS


	def _flush_input_buffer(self):
		while self._serial.inWaiting() > 0:
			self._debug("Flushing input buffer")
			self._read_frame()

	def _can_send(self):
		self._flush_input_buffer()
		return self._read_frame(True) == self.ERR_TIMEOUT_ON_LENGTH

	
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
		crc16 = PredefinedCrc('crc16')
		crc16.update(msg)
		msg_crc = crc16.digest()

		frame = chr(len(msg) + 3) + msg + msg_crc

		result = self._send_frame(frame)

		while not result:
			backoff = 8 * self._bit_time * (priority + random.randint(1,5))
			self._debug("Collision occured backing off %f" % backoff)
			time.sleep(backoff)
			result = self._send_frame(frame)


	def read_message(self):
		while len(self._buffer) == 0:
			self._read_frame()

		return self._buffer.pop()

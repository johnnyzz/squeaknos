#!/usr/bin/python

import socket
from select import select
import struct

s1 = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
s1.connect(('./disksbien/serial1'))

exit = False

while not exit:

	str_data = input("Next address? ")
	data = struct.pack('1I', int(str_data)) # + '\x00'*12

	s1.send(data)

	print "Server -> SqueakNOS: %r" % data

	if (str_data == "q"):
		exit = True


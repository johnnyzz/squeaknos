#!/usr/bin/python

import socket
from select import select
import struct
import sys
from time import gmtime, strftime

def chunks(l, n):
    return [l[i:i+n] for i in range(0, len(l), n)]

s1 = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
s1.connect(('./vmdk/serial1'))

data = "1"
while data:

	data = s1.recv(100000)


	if len(data) % 2 == 1:
		data = data + s1.recv(1)

	splitted_data = chunks (data, 2)
	#print(splitted_data)

	#pair = splitted_data[0]
	#print "ord1:", ord(pair[0]), "ord2:", ord(pair[1]) << 4
	real_data = [ chr(ord(pair[0]) + (ord(pair[1]) << 4)) for pair in splitted_data ]
	

	sys.stdout.write("".join(real_data))

	if "".join(real_data) == "\n":
		sys.stdout.write(strftime("%H:%M:%S", gmtime())+" >> ")

        sys.stdout.flush()

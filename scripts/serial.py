#!/usr/bin/python

import time
import socket
from select import select

debug = 0

t1 = 0
t2 = 0
t0 = time.time()

s2 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s2.connect(('127.0.0.1',1234))

s1 = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
s1.connect(('./disks/serial1'))

while 1:
	r,w,x = select([s1, s2],[],[])
	#r,w,x = select([s1],[],[])

	if s1 in r:
		data = s1.recv(100000)
		if not data: break
		if debug:
			print "SqueakNOS -> Server: %r" % data
		s2.send(data)
		t1 += len(data)
	if s2 in r:
		data = s2.recv(100000)
		if not data: break
		s1.send(data)
		t2 += len(data)
		t = time.time()
		if debug:
			print "Server -> SqueakNOS: %r" % data
		print "%f (%f) Server: %d\tClient: %d" % (t, t-t0,t2, t1)
		t0 = t

t = time.time()
print "%f (%f) Server: %d\tClient: %d" % (t, t-t0,t2, t1)

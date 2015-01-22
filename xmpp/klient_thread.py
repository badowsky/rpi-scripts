# -*- coding: utf-8 -*-
import socket
import sys
import time

host = 'localhost'
port = 50000
size = 1024
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((host,port))
sys.stdout.write('%')

while 1:
    line = str(time.ctime(time.time())) + '\n'
    s.send(line)
    data = s.recv(size)
    sys.stdout.write(data)
    time.sleep(5)
s.close()
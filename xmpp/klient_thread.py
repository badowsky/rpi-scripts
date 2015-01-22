# -*- coding: utf-8 -*-
import socket
import sys
import time

host = 'localhost'
port = 50000
size = 1024
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((host,port))


while 1:
    line = input()
    s.send(line.encode())
s.close()

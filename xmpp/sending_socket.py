import socket
import sys

s = socket.socket()
s.connect(("127.0.0.1", 8019))

while True:
    cmd = sys.stdin.readline().strip().lower()
    if cmd == 'exit' or cmd == 'quit':
        break
    s.send(str.encode(cmd))

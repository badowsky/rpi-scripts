# -*- coding: utf-8 -*-
import socket
import sys
import time
from optparse import OptionParser


host = '127.0.0.1'
port = 5005
size = 1024
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((host,port))

if __name__ == '__main__':
    # Setup the command line arguments.
    optp = OptionParser()


    # Get a message.
    optp.add_option("-m", "--message", dest="msg",
                    help="Message what you want to send.")
    opts, args = optp.parse_args()

    s.send(opts.msg.encode())
    s.close()

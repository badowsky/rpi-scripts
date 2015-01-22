# -*- coding: utf-8 -*-
import select 
import socket 
import sys 
import threading 
import time

 
class Server(threading.Thread): 
    def __init__(self):
	    threading.Thread.__init__(self) 
        self.host = '' 
        self.port = 50000 
        self.backlog = 5 
        self.size = 1024 
        self.server = None 
        self.threads = [] 
 
    def open_socket(self): 
        try: 
            self.server = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
            self.server.bind((self.host,self.port)) 
            self.server.listen(5) 
        except socket.error, (value,message): 
            if self.server: 
                self.server.close() 
            print "Could not open socket: " + message 
            sys.exit(1) 
 
    def run(self): 
        self.open_socket() 
        input = [self.server,sys.stdin] 
        running = 1 
        while running: 
            inputready,outputready,exceptready = select.select(input,[],[]) 
 
            for s in inputready: 
 
                if s == self.server: 
                    # handle the server socket 
                    c = ServerClient(self.server.accept()) 
                    c.start() 
                    self.threads.append(c) 
 
                elif s == sys.stdin: 
                    # handle standard input 
                    junk = sys.stdin.readline() 
                    running = 0 
 
        # close all threads 
 
        self.server.close() 
        for c in self.threads: 
            c.join() 
 
class ServerClient(threading.Thread): 
    def __init__(self,(client,address)): 
        threading.Thread.__init__(self) 
        self.client = client 
        self.address = address 
        self.size = 1024
	print("nowy klient {addr}".format(addr=address))
 
    def run(self): 
        running = 1
	print("RUN")
        while running:
	        print("running loop")
            data = self.client.recv(self.size)
	        print("recived")
            if data:
		        print("data")
                self.client.send(data) 
            else:
		        print("no data - closing")
                self.client.close() 
                running = 0 
 
if __name__ == "__main__": 
    s = Server() 
    s.start()
    while 1:
        time.sleep(1)
        print("Poszło")
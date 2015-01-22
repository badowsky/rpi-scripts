# -*- coding: utf-8 -*-
import select 
import socket 
import sys 
import threading 
import time
import logging
import getpass
from optparse import OptionParser

import sleekxmpp

# Python versions before 3.0 do not use UTF-8 encoding
# by default. To ensure that Unicode is handled properly
# throughout SleekXMPP, we will set the default encoding
# ourselves to UTF-8.
if sys.version_info < (3, 0):
    reload(sys)
    sys.setdefaultencoding('utf8')
else:
    raw_input = input


class EchoBot(sleekxmpp.ClientXMPP):

    """
    A simple SleekXMPP bot that will echo messages it
    receives, along with a short thank you message.
    """

    def __init__(self, jid, password):
        sleekxmpp.ClientXMPP.__init__(self, jid, password)

        # The session_start event will be triggered when
        # the bot establishes its connection with the server
        # and the XML streams are ready for use. We want to
        # listen for this event so that we we can initialize
        # our roster.
        self.add_event_handler("session_start", self.start)

        # The message event is triggered whenever a message
        # stanza is received. Be aware that that includes
        # MUC messages and error messages.
        self.add_event_handler("message", self.message)

    def start(self, event):
        """
        Process the session_start event.

        Typical actions for the session_start event are
        requesting the roster and broadcasting an initial
        presence stanza.

        Arguments:
            event -- An empty dictionary. The session_start
                     event does not provide any additional
                     data.
        """
        self.send_presence()
        self.get_roster()

    def message(self, msg):
        """
        Process incoming message stanzas. Be aware that this also
        includes MUC messages and error messages. It is usually
        a good idea to check the messages's type before processing
        or sending replies.

        Arguments:
            msg -- The received message stanza. See the documentation
                   for stanza objects and the Message stanza to see
                   how it may be used.
        """
        if msg['type'] in ('chat', 'normal'):
            msg.reply("Thanks for sending\n%(body)s" % msg).send()
 
class Server(threading.Thread): 
    def __init__(self, xmpp_client):
        threading.Thread.__init__(self)
        self.host = '' 
        self.port = 50000 
        self.backlog = 5 
        self.size = 1024 
        self.server = None 
        self.threads = []

        self.single_thread = threading.Lock()
        self.xmpp_client = xmpp_client

        self.xmpp_client.register_plugin('xep_0030') # Service Discovery
        self.xmpp_client.register_plugin('xep_0004') # Data Forms
        self.xmpp_client.register_plugin('xep_0060') # PubSub
        self.xmpp_client.register_plugin('xep_0199') # XMPP Ping

        # If you are working with an OpenFire server, you may need
        # to adjust the SSL version used:
        # xmpp.ssl_version = ssl.PROTOCOL_SSLv3

        # If you want to verify the SSL certificates offered by a server:
        # xmpp.ca_certs = "path/to/ca/cert"

        # Connect to the XMPP server and start processing XMPP stanzas.
        if self.xmpp_client.connect():
            # If you do not have the dnspython library installed, you will need
            # to manually specify the name of the server if it does not match
            # the one in the JID. For example, to use Google Talk you would
            # need to use:
            #
            # if xmpp.connect(('talk.google.com', 5222)):
            #     ...
            self.xmpp_client.process(block=False)
            print("Connected")
        else:
            print("Unable to connect.")

    def open_socket(self): 
        try: 
            self.server = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
            self.server.bind((self.host,self.port)) 
            self.server.listen(5) 
        except socket.error as err: 
            if self.server: 
                self.server.close() 
            print("Could not open socket: " + err.message) 
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
                    c = ServerClient(self.server.accept(), self) 
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
    def __init__(self, conn, server): 
        threading.Thread.__init__(self) 
        self.client = conn[0]#client 
        self.address = conn[1]#address
        self.size = 1024
        self.server=server
        print("nowy klient {addr}".format(addr=self.address))

    def run(self): 
        running = 1
        print("RUN")
        while running:
            print("running loop")
            data = self.client.recv(self.size)
            print("recived")
            if data:
                print("data")
                with self.server.single_thread:
                    self.server.xmpp_client.send_message(mto="mbadowsky@gmail.com",
                                                         mbody="test",
                                                         mtype='chat')
                self.client.send(data) 
            else:
                print("no data - closing")
                self.client.close() 
                running = 0 
 
if __name__ == '__main__':
    # Setup the command line arguments.
    optp = OptionParser()

    # Output verbosity options.
    optp.add_option('-q', '--quiet', help='set logging to ERROR',
                    action='store_const', dest='loglevel',
                    const=logging.ERROR, default=logging.INFO)
    optp.add_option('-d', '--debug', help='set logging to DEBUG',
                    action='store_const', dest='loglevel',
                    const=logging.DEBUG, default=logging.INFO)
    optp.add_option('-v', '--verbose', help='set logging to COMM',
                    action='store_const', dest='loglevel',
                    const=5, default=logging.INFO)

    # JID and password options.
    optp.add_option("-l", "--login", dest="jid",
                    help="JID to use")
    optp.add_option("-p", "--password", dest="password",
                    help="password to use")

    opts, args = optp.parse_args()

    # Setup logging.
    logging.basicConfig(level=opts.loglevel,
                        format='%(levelname)-8s %(message)s')

    if opts.jid is None:
        opts.jid = raw_input("Username: ")
    if opts.password is None:
        opts.password = getpass.getpass("Password: ")

    # Setup the EchoBot and register plugins. Note that while plugins may
    # have interdependencies, the order in which you register them does
    # not matter.
    xmpp = EchoBot(opts.jid, opts.password)

    s = Server(xmpp) 
    s.start()


#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import select 
import socket 
import sys
import errno
import threading 
import time
import logging
import getpass
import daemon
from optparse import OptionParser
from MsgParser import MsgParser

import sleekxmpp

logger = logging.getLogger(__name__)

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

        self.parser = MsgParser()

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
            self.parser.process(msg['body'])
            msg.reply("Thanks for sending\n%(body)s" % msg).send()
 
class Server(threading.Thread): 
    def __init__(self, xmpp_client):
        threading.Thread.__init__(self)
        self.host = '127.0.0.1' 
        self.port = 5005 
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
            logger.info("Connected to XMPP server.")
            #self.run()
        else:
            logger.error("Unable to connect.")

    def open_socket(self): 
        try: 
            #self.server = server
            self.server = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
            self.server.bind((self.host,self.port)) 
            self.server.listen(5)
            logger.info("Socked created - listen to socket: host: {} port: {}".format(self.host, self.port)) 
        except socket.error as err: 
            if self.server: 
                self.server.close() 
            logger.error("Could not open socket: " + errno.errorcode[err.errno]) 
            sys.exit(1) 
 
    def run(self): 
        self.open_socket() 
        #input = [self.server,sys.stdin] 
        running = 1 
        while running: 
            #inputready,outputready,exceptready = select.select(input,[],[]) 
 
            #for s in inputready: 
 
                #if s == self.server: 
                    # handle the server socket 
            logger.info("Oczekuje polaczenia...")
            c = ServerClient(self.server.accept(), self) 
            c.start() 
            self.threads.append(c) 
 
                #elif s == sys.stdin: 
                    # handle standard input 
                    #junk = sys.stdin.readline() 
                    #running = 0 
 
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
        logger.debug("New client with address: {addr}.".format(addr=self.address))

    def run(self): 
        running = 1
        while running:
            try:
                data = self.client.recv(self.size)
            except socket.error as err:
                logger.debug("Connection closed suddenly.")
                data = ''
            
            if data:
                logger.debug("Recived and sending message: " + data.decode())
                with self.server.single_thread:
                    self.server.xmpp_client.send_message(mto="mbadowsky@gmail.com",
                                                         mbody=data.decode(),
                                                         mtype='chat')
                self.client.send(data) 
            else:
                logger.debug("No data - closing connection.")
                self.client.close() 
                running = 0 
 
def daemon (stdin='/dev/null', stdout='/dev/null', stderr='/dev/null'):
    try:
        pid = os.fork( )
        if pid > 0:
            sys.exit(0)
    except OSError, e:
        sys.stderr.write("fork #1 failed: (%d) %sn" % (e.errno, e.strerror))
        sys.exit(1)

    os.chdir("/")
    os.umask(0)
    os.setsid()

    try:
        pid = os.fork( )
        if pid > 0:
            sys.exit(0)
    except OSError, e:
        sys.stderr.write("fork #2 failed: (%d) %sn" % (e.errno, e.strerror))
        sys.exit(1)
    # The process is now daemonized, redirect standard file descriptors.
    for f in sys.stdout, sys.stderr: f.flush()
    si = file(stdin, 'r')
    so = file(stdout, 'a+')
    se = file(stderr, 'a+', 0)
    os.dup2(si.fileno(), sys.stdin.fileno())
    os.dup2(so.fileno(), sys.stdout.fileno())
    os.dup2(se.fileno(), sys.stderr.fileno())

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
                    help="Login to use", default="bador.rpi@gmail.com")
    optp.add_option("-p", "--password", dest="password",
                    help="Password to use", default="ba1805di")

    opts, args = optp.parse_args()

    # Setup logging.
    logging.basicConfig(filename='/home/pi/rpi-scripts/xmpp/server_thread.log',
                        #level=opts.loglevel,
                        level=logging.DEBUG,
                        format='%(levelname)-8s %(message)s')

    if opts.jid is None:
        opts.jid = raw_input("Username: ")
    if opts.password is None:
        opts.password = getpass.getpass("Password: ")

    # Setup the EchoBot and register plugins. Note that while plugins may
    # have interdependencies, the order in which you register them does
    # not matter.
    #with daemon.DaemonContext(stdout=fh):
    xmpp = EchoBot(opts.jid, opts.password)
    s = Server(xmpp)
    s.start()
    demon()

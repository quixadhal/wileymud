# -*- coding: utf-8 -*- line endings: unix -*-
__author__ = 'quixadhal'

import sys
import socket
import select
import datetime

from Mud.log_system import Logger
logger = Logger()

class TelnetServer(object):
    def __init__(self, port):
        if not isinstance(port, integer):
            logger.fatal('Invalid port: %r', port)
            raise ValueError('Cannot start a TelnetServer without a valid port number')

        try:
            self.game_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.game_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self.game_socket.bind(('', port)) # address, port
            self.game_socket.listen(5)
        except socket.error as err:
            logger.fatal('Cannot create main game socket: %s', err)
            raise

        self.connections = []


    def close(self):
        for c in self.connections:
            c.close()
        self.game_socket.close()


    def poll(self):
        self.read_list = [ self.game_socket ]
        self.write_list = []
        self.close_list = []

        for c in self.connections:
            if c.connected:
                self.read_list.append(c)
            else:
                self.close_list.append(c)

        for c in self.connections:
            if c.pending and c.connected:
                self.write_list.append(c)

        try:
            rlist, wlist, xlist = select.select(self.read_list, self.write_list, [], 0)
        except select.error as err:
            logger.fatal("SELECT socket error '{}'.".format(str(err)))
            raise

        for c in rlist:
            if c == self.game_socket:
                # New connection
                try:
                    sock, addr = c.accept()
                except socket.error as err:
                    logger.error("ACCEPT failure '{}:{}'.".format(str(err[0]), str(err[1])))
                    continue

                # Check for max connections
                #if options.max_connections is not None:
                #    if len(self.connections) >= options.max_connections:
                #        logger.warning("Too many connections, refusing newcomer.")
                #        self.close_list.append(c)
                #        continue

                # Spawn a new client!
                newbie = TelnetClient(sock, addr)

                if newbie is not None:
                    self.connections.append(newbie)
                    # call self.on_connect() function
            else:
                # grab data, if any
                try:
                    c.recv()
                except ConnectionLost:
                    logger.warning("Connection lost.")
                    self.close_list.append(c)

        for c in wlist:
            c.send()

        for c in self.close_list:
            c.close()
            del self.connections[c]


class ConnectionLost(Exception):
    pass


class TelnetState(object):
    def __init__(self):
        self.SE     = 240       # End of subnegotiation parameters
        self.NOP    = 241       # No operation
        self.DATMK  = 242       # Data stream portion of a sync.
        self.BREAK  = 243       # NVT Character BRK
        self.IP     = 244       # Interrupt Process
        self.AO     = 245       # Abort Output
        self.AYT    = 246       # Are you there
        self.EC     = 247       # Erase Character
        self.EL     = 248       # Erase Line
        self.GA     = 249       # The Go Ahead Signal
        self.SB     = 250       # Sub-option to follow
        self.WILL   = 251       # Will; request or confirm option begin
        self.WONT   = 252       # Wont; deny option request
        self.DO     = 253       # Do = Request or confirm remote option
        self.DONT   = 254       # Don't = Demand or confirm option halt
        self.IAC    = 255       # Interpret as Command
        self.SEND   = 1         # Sub-process negotiation SEND command
        self.IS     = 0         # Sub-process negotiation IS command

        self.BINARY = 0         # Transmit Binary
        self.ECHO   = 1         # Echo characters back to sender
        self.RECON  = 2         # Reconnection
        self.SGA    = 3         # Suppress Go-Ahead
        self.TTYPE  = 24        # Terminal Type
        self.NAWS   = 31        # Negotiate About Window Size
        self.LINEMO = 34        # Line Mode

        self.processing = bytearray()   # Our accumulator for raw socket input
        self.sb_buffer  = bytearray()   # A place to gather subnegotiation data
        self.in_IAC     = False         # This means we got an IAC and are processing
        self.in_SB      = False         # This means we got an SB and are processing
        self.in_THREE   = False         # This means we got a WILL/WONT/DO/DONT
        self.got_CR     = False         # TELNET says a bare CR should become CR NUL

        self.local_telopts = {}         # Used to store OUR settings for each opt we know
        self.remote_telopts = {}        # Used to store THEIR settings for each opt we know

    def process(data, client):
        """
        The process method of the TelnetState class takes a raw chunk of byte data,
        typically a bytes string as it comes from the socket.recv() call, and attempts
        to process any TELNET sequences within it.  It will remove these sequences and
        re-encode the remaining data as a regular utf-8 string, to be returned to the
        caller.

        In order to respond to TELNET requests, you must also pass in an instance of
        a TelnetClient object, so we have a socket to use for sending replies to the
        remote end.

        Several helper methods will also exist, which can be called separately to create
        byte sequences to send out a socket, should you wish to initiate a TELNET
        sequence, rather than simply responding to them.

        :return: A string containing filtered data
        :rtype: string
        """
        if not isinstance(data, bytes) and not isinstance(data, bytearray):
            logger.error('Invalid data type: got %r, expected bytes or bytearray', type(data))
            raise TypeError('data argument must be a bytes string or bytearray, not %r' % type(data))
        if not isinstance(client, TelnetClient):
            logger.error('Invalid client type: got %r, expected TelnetClient object', type(client))
            raise TypeError('client argument must be a TelnetClient object, not %r' % type(client))

        for b in data:
            if not self.in_IAC:
                if b == self.IAC:
                    self.in_IAC = True
                elif self.got_CR:
                    if b == b'\x00':
                        # TELNET says CR NUL is encoding for a bare CR
                        self.got_CR = False
                        self.processing.append(b'\r')
                    elif b == b'\r':
                        # The first CR wasn't encoded right, but maybe this one will be?
                        self.processing.append(b'\r')
                    elif b == b'\n':
                        # TELNET CR LF is a full line ending.  If we're unix, we may want
                        # this to become a bare LF again.  If so, comment out the next line.
                        self.got_CR = False
                        self.processing.append(b'\r')
                        self.processing.append(b'\n')
                    else:
                        # OK, It was CR and then something random, toss them both in
                        self.got_CR = False
                        self.processing.append(b'\r')
                        self.processing.append(b)
                else:
                    self.processing.append(b)
            else:
                if self.in_SB:
                    if b == self.SE:
                        self.in_SB = False
                        self.in_IAC = False
                        self.handle_sb(self.sb_buffer, client)
                    else:
                        self.sb_buffer.append(b)
                elif self.in_THREE:
                    self.in_THREE = False
                    self.in_IAC = False
                    self.handle_three(self.in_THREE, b, client)
                else:
                    if b == self.IAC:
                        # IAC IAC is a single 255 escaped
                        self.in_IAC = False
                        self.self.processing.append(b)
                    elif b == self.SB:
                        self.in_SB = True
                    elif b in [ self.WILL, self.WONT, self.DO, self.DONT ]:
                        self.in_THREE = b
                    else:
                        self.handle_two(b, client)

        if self.in_IAC or self.in_SB or self.in_THREE or self.got_CR:
            # Hmmmm, we ran off the buffer but still are in an incomplete state...
            #
            # What we should probably do is figure out how many bytes into our
            # incomplete state we are, and chop the processing buffer so it ONLY
            # contains those bytes, returning all the previous bytes to the
            # caller, converted to a string.
            #
            # That way, if the next packet has more to process, we won't lose
            # anything.  The downside is, if it was a false positive, we might
            # hold up data that could have been used for a loop cycle.
            #
            # If got_CR is active, we're expecting either an LF or a NUL.
            # So we only need to keep the previous byte.
            #
            # If in_IAC and in_THREE are both active, we're expecting the
            # final opcode byte of the WILL/WONT/DO/DONT message, so we need
            # to keep the previous two bytes.
            #
            # If in_IAC and in_SB are both active, we are somewhere in the middle
            # of an arbitrary length subnegotiation.  We need to keep the IAC,
            # the SB, and whatever has built up in the sb_buffer so far.
            #
            # If it's only in_IAC, we only need that IAC byte.
            #
            # The good news is that we don't reset state, so as long as the
            # same class instance calls process() with additional data, we
            # SHOULD just continue from where we left off.
            pass


    def handle_sb(self, data, client):
        pass

    def handle_two(self, data, client):
        pass

    def handle_three(self, op, data, client):
        pass


class TelnetClient(object):
    def __init__(self, sock, addr):
        self.connected = True               # Set this to false to disconnect
        self.pending = False                # Data is ready to be sent to the user
        self._read_buffer = ''              # Data from the socket
        self._write_buffer = ''             # Data to the socket
        self.bytes_received = 0             # How much data we've received, in total
        self.bytes_sent = 0                 # How much data we've sent out, in total
        self.last_received = None           # When did we last get data FROM the user?
        self.last_sent = None               # When did we last send data TO the user?
        self.remote_address = addr[0]       # The remote IP address of our socket
        self.remote_port = addr[1]          # The remote port of our socket
        self.socket = sock                  # The full socket object
        self.telnet_state = TelnetState()   # Keep track of TELNET stuff

    def recv(self):
        try:
            data = self.socket.recv(4096)   # Read in a block of raw bytes
        except socket.error as err:
            logger.error('socket RECV error %s from %s:%d', err, self.remote_address, self.remote_port)
            self.connected = False          # poll() will remove us in the next loop
            raise ConnectionLost()          # stop further processing

        length = len(data)                  # How many bytes did we actually get?
        if length == 0:
            logger.debug('socket RECV empty, client dropped from %s:%d', self.remote_address, self.remote_port)
            self.connected = False          # poll() will remove us in the next loop
            raise ConnectionLost()          # stop further processing

        self.bytes_received += length
        self.last_received = datetime.now(timezone.utc) # "aware" datetime object

        content = self.telnet_state.process(data)       # Strip out TELNET sequences and handle them







    @property
    def fileno(self):
        return self.socket.fileno

    @fileno.setter
    def fileno(self, data):
        if not data:
            raise ValueError('The connection cannot have an empty descriptor')
        elif type(data) is not int:
            raise TypeError('The connection descriptor must be an integer')
        elif data < 0:
            raise ValueError('The connection descriptor must be a positive integer')
        else:
            self._connection.fileno = data

    def send(self, data):
        try:
            self._write_buffer.append(str(data))
        except ValueError as err:
            logger.error("Could not convert data to a string for transmission: {}.".format(str(err))




DEFAULT_PORT = 23
DEFAULT_TIMEOUT = 0.5
# Cap sockets to 512 on Windows because winsock can only process 512 at time
# Cap sockets to 1000 on UNIX because you can only have 1024 file descriptors
if sys.platform == 'win32':
    MAX_CONNECTIONS = 500
else:
    MAX_CONNECTIONS = 1000


# --[ Stub functions ]----------------------------------------------------------
def _on_connect(client):
    """
    Placeholder new connection handler.
    """
    logger.info("++ Opened connection to {}, sending greeting...".format(client.addrport()))
    client.send("Greetings from Miniboa-py3!\n")


def _on_disconnect(client):
    """
    Placeholder lost connection handler.
    """
    logger.info("-- Lost connection to %s".format(client.addrport()))


def _term_handler(text: str or None, input_type='pyku', output_type='ANSI'):
    """
    Placeholder for terminal/color handler.
    """
    return text


# --[ Telnet Server ]-----------------------------------------------------------
class TelnetServer(object):
    """
    Poll sockets for new connections and sending/receiving data from clients.
    """

    def __init__(self, port=DEFAULT_PORT, address='', on_connect=_on_connect,
                 on_disconnect=_on_disconnect, max_connections=MAX_CONNECTIONS,
                 timeout=DEFAULT_TIMEOUT, term_handler=_term_handler):
        """
        Create a new Telnet Server.

        port -- Port to listen for new connection on.  On UNIX-like platforms,
            you made need root access to use ports under 1025.

        address -- Address of the LOCAL network interface to listen on.  You
            can usually leave this blank unless you want to restrict traffic
            to a specific network device.  This will usually NOT be the same
            as the Internet address of your server.

        on_connect -- function to call with new telnet connections

        on_disconnect -- function to call when a client's connection dies,
            either through a terminated session or client.active being set
            to False.

        max_connections -- maximum simultaneous the server will accept at once

        timeout -- amount of time that Poll() will wait from user input
            before returning.  Also frees a slice of CPU time.

        term_handler -- function to convert color/terminal tokens into
            byte sequences the remote terminal can use.
        """

        self.port = port
        self.address = address
        self.on_connect = on_connect
        self.on_disconnect = on_disconnect
        self.max_connections = min(max_connections, MAX_CONNECTIONS)
        self.timeout = timeout
        self.term_handler = term_handler

        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

        try:
            server_socket.bind((address, port))
            server_socket.listen(5)
        except socket.error as err:
            logger.critical("Unable to create the server socket: " + str(err))
            raise

        self.server_socket = server_socket
        self.server_fileno = server_socket.fileno()

        # Dictionary of active clients,
        # key = file descriptor, value = TelnetClient (see miniboa.telnet)
        self.clients = {}

    def stop(self):
        """
        Disconnects the clients and shuts down the server
        """
        for clients in self.client_list():
            clients.sock.close()
        self.server_socket.close()

    def client_count(self):
        """
        Returns the number of active connections.
        """
        return len(self.clients)

    def client_list(self):
        """
        Returns a list of connected clients.
        """
        return self.clients.values()

    def poll(self):
        """
        Perform a non-blocking scan of recv and send states on the server
        and client connection sockets.  Process new connection requests,
        read incoming data, and send outgoing data.  Sends and receives may
        be partial.
        """
        # Build a list of connections to test for receive data pending
        recv_list = [self.server_fileno]  # always add the server

        del_list = []  # list of clients to delete after polling

        for client in self.clients.values():
            if client.active:
                recv_list.append(client.fileno)
            else:
                self.on_disconnect(client)
                del_list.append(client.fileno)
                client.sock.close()

        # Delete inactive connections from the dictionary
        for client in del_list:
            del self.clients[client]

        # Build a list of connections that need to send data
        send_list = []
        for client in self.clients.values():
            if client.send_pending:
                send_list.append(client.fileno)

        # Get active socket file descriptors from select.select()
        try:
            rlist, slist, elist = select.select(recv_list, send_list, [],
                                                self.timeout)
        except select.error as err:
            # If we can't even use select(), game over man, game over
            logger.critical("SELECT socket error '{}'".format(str(err)))
            raise

        # Process socket file descriptors with data to receive
        for sock_fileno in rlist:

            # If it's coming from the server's socket then this is a new connection request.
            if sock_fileno == self.server_fileno:

                try:
                    sock, addr_tup = self.server_socket.accept()
                except socket.error as err:
                    logger.error("ACCEPT socket error '{}:{}'.".format(err[0], err[1]))
                    continue

                # Check for maximum connections
                if self.client_count() >= self.max_connections:
                    logger.warning("Refusing new connection, maximum already in use.")
                    sock.close()
                    continue

                # Create the client instance
                new_client = TelnetClient(sock, addr_tup, self.term_handler)

                # Add the connection to our dictionary and call handler
                self.clients[new_client.fileno] = new_client
                self.on_connect(new_client)

            else:
                # Call the connection's receive method
                try:
                    self.clients[sock_fileno].socket_recv()
                except ConnectionLost:
                    self.clients[sock_fileno].deactivate()

        # Process sockets with data to send
        for sock_fileno in slist:
            # Call the connection's send method
            self.clients[sock_fileno].socket_send()

# ---[ Telnet Notes ]-----------------------------------------------------------
# (See RFC 854 for more information)
#
# Negotiating a Local Option
# --------------------------
#
# Side A begins with:
#
#    "IAC WILL/WONT XX"   Meaning "I would like to [use|not use] option XX."
#
# Side B replies with either:
#
#    "IAC DO XX"     Meaning "OK, you may use option XX."
#    "IAC DONT XX"   Meaning "No, you cannot use option XX."
#
#
# Negotiating a Remote Option
# ----------------------------
#
# Side A begins with:
#
#    "IAC DO/DONT XX"  Meaning "I would like YOU to [use|not use] option XX."
#
# Side B replies with either:
#
#    "IAC WILL XX"   Meaning "I will begin using option XX"
#    "IAC WONT XX"   Meaning "I will not begin using option XX"
#
#
# The syntax is designed so that if both parties receive simultaneous requests
# for the same option, each will see the other's request as a positive
# acknowledgement of it's own.
#
# If a party receives a request to enter a mode that it is already in, the
# request should not be acknowledged.

UNKNOWN = -1

# --[ Telnet Commands ]---------------------------------------------------------
SE = chr(240)  # End of subnegotiation parameters
NOP = chr(241)  # No operation
DATMK = chr(242)  # Data stream portion of a sync.
BREAK = chr(243)  # NVT Character BRK
IP = chr(244)  # Interrupt Process
AO = chr(245)  # Abort Output
AYT = chr(246)  # Are you there
EC = chr(247)  # Erase Character
EL = chr(248)  # Erase Line
GA = chr(249)  # The Go Ahead Signal
SB = chr(250)  # Sub-option to follow
WILL = chr(251)  # Will; request or confirm option begin
WONT = chr(252)  # Wont; deny option request
DO = chr(253)  # Do = Request or confirm remote option
DONT = chr(254)  # Don't = Demand or confirm option halt
IAC = chr(255)  # Interpret as Command
SEND = chr(1)  # Sub-process negotiation SEND command
IS = chr(0)  # Sub-process negotiation IS command

# --[ Telnet Options ]----------------------------------------------------------
BINARY = chr(0)  # Transmit Binary
ECHO = chr(1)  # Echo characters back to sender
RECON = chr(2)  # Reconnection
SGA = chr(3)  # Suppress Go-Ahead
TTYPE = chr(24)  # Terminal Type
NAWS = chr(31)  # Negotiate About Window Size
LINEMO = chr(34)  # Line Mode


# --[ Lost Connection Exception Handler ]---------------------------------------
class ConnectionLost(Exception):
    """
    Custom exception to signal a lost connection to the Telnet Server.
    """
    pass


# --[ Telnet Option ]-----------------------------------------------------------
class TelnetOption(object):
    """
    Simple class used to track the status of an extended Telnet option.
    """

    def __init__(self):
        self.local_option = UNKNOWN  # Local state of an option
        self.remote_option = UNKNOWN  # Remote state of an option
        self.reply_pending = False  # Are we expecting a reply?


# --[ Telnet Client ]-----------------------------------------------------------
class TelnetClient(object):
    """
    Represents a client connection via Telnet.

    First argument is the socket discovered by the Telnet Server.
    Second argument is the tuple (ip address, port number).
    Third (optional) argument is the terminal token handler.
    """

    def __init__(self, sock, addr_tup, term_handler=_term_handler):
        self.protocol = 'telnet'
        self.active = True  # Turns False when the connection is lost
        self.sock = sock  # The connection's socket
        self.fileno = sock.fileno()  # The socket's file descriptor
        self.address = addr_tup[0]  # The client's remote TCP/IP address
        self.port = addr_tup[1]  # The client's remote port
        self.terminal_type = 'ANSI'  # set via request_terminal_type()
        self.term_handler = term_handler
        self.use_ansi = True
        self.columns = 80
        self.rows = 24
        self.send_pending = False
        self.send_buffer = ''
        self.recv_buffer = ''
        self.bytes_sent = 0
        self.bytes_received = 0
        self.cmd_ready = False
        self.command_list = []
        self.connect_time = time.time()
        self.last_input_time = time.time()

        # State variables for interpreting incoming telnet commands
        self.telnet_got_iac = False  # Are we inside an IAC sequence?
        self.telnet_got_cmd = None  # Did we get a telnet command?
        self.telnet_got_sb = False  # Are we inside a subnegotiation?
        self.telnet_opt_dict = {}  # Mapping for up to 256 TelnetOptions
        self.telnet_echo = False  # Echo input back to the client?
        self.telnet_echo_password = False  # Echo back '*' for passwords?
        self.telnet_sb_buffer = ''  # Buffer for sub-negotiations

    def get_command(self):
        """
        Get a line of text that was received from the client. The class's
        cmd_ready attribute will be true if lines are available.
        """
        cmd = None
        count = len(self.command_list)
        if count > 0:
            cmd = self.command_list.pop(0)

        # If that was the last line, turn off lines_pending
        if count == 1:
            self.cmd_ready = False
        return cmd

    def send(self, text: str, ttype: str or None):
        """
        Send raw text to the distant end.
        """
        if ttype is None:
            ttype = self.terminal_type

        if text and isinstance(text, str):
            text = text.replace('\n\r', '\n')  # Old DikuMUD backwards line endings
            text = text.replace('\r\n', '\n')  # Fold TELNET/MS line endings down to UNIX

            text = self.term_handler(text, 'pyku', ttype)  # Convert color tokens to terminal codes

            text = text.replace('\r', '\r\0')  # Map any remaining lone-CR's to TELNET spec
            text = text.replace('\n', '\r\n')  # Map line endings to TELNET spec

            self.send_buffer += text
            self.send_pending = True

    def deactivate(self):
        """
        Set the client to disconnect on the next server poll.
        """
        self.active = False

    def addrport(self):
        """
        Return the client's IP address and port number as a string.
        """
        return "{}:{}".format(self.address, self.port)

    def idle(self):
        """
        Returns the number of seconds that have elasped since the client
        last sent us some input.
        """
        return time.time() - self.last_input_time

    def duration(self):
        """
        Returns the number of seconds the client has been connected.
        """
        return time.time() - self.connect_time

    def request_do_sga(self):
        """
        Request client to Suppress Go-Ahead.  See RFC 858.
        """
        self._iac_do(SGA)
        self._note_reply_pending(SGA, True)

    def request_will_echo(self):
        """
        Tell the client that we would like to echo their text.  See RFC 857.
        """
        self._iac_will(ECHO)
        self._note_reply_pending(ECHO, True)
        self.telnet_echo = True

    def request_wont_echo(self):
        """
        Tell the client that we would like to stop echoing their text.
        See RFC 857.
        """
        self._iac_wont(ECHO)
        self._note_reply_pending(ECHO, True)
        self.telnet_echo = False

    def password_mode_on(self):
        """
        Tell client we will echo (but don't) so typed passwords don't show.
        """
        self._iac_will(ECHO)
        self._note_reply_pending(ECHO, True)

    def password_mode_off(self):
        """
        Tell client we are done echoing (we lied) and show typing again.
        """
        self._iac_wont(ECHO)
        self._note_reply_pending(ECHO, True)

    def request_naws(self):
        """
        Request to Negotiate About Window Size.  See RFC 1073.
        """
        self._iac_do(NAWS)
        self._note_reply_pending(NAWS, True)

    def request_terminal_type(self):
        """
        Begins the Telnet negotiations to request the terminal type from
        the client.  See RFC 779.
        """
        self._iac_do(TTYPE)
        self._note_reply_pending(TTYPE, True)

    def socket_send(self):
        """
        Called by TelnetServer when send data is ready.
        """
        if len(self.send_buffer):
            try:
                # convert to ansi before sending
                sent = self.sock.send(bytes(self.send_buffer, "cp1252"))
            except socket.error as err:
                logger.error("SEND error '{}' from {}".format(err, self.addrport()))
                self.active = False
                return
            self.bytes_sent += sent
            self.send_buffer = self.send_buffer[sent:]
        else:
            self.send_pending = False

    def socket_recv(self):
        """
        Called by TelnetServer when recv data is ready.
        """
        try:
            # Encode recieved bytes in ansi
            data = str(self.sock.recv(2048), "cp1252")
        except socket.error as err:
            logger.error("RECEIVE socket error '{}' from {}".format(err, self.addrport()))
            raise ConnectionLost()

        # Did they close the connection?
        size = len(data)
        if size == 0:
            logger.debug("No data received, client closed connection")
            raise ConnectionLost()

        # Update some trackers
        self.last_input_time = time.time()
        self.bytes_received += size

        # Test for telnet commands
        for byte in data:
            self._iac_sniffer(byte)

        # Look for newline characters to get whole lines from the buffer
        while True:
            mark = self.recv_buffer.find('\n')
            if mark == -1:
                break
            cmd = self.recv_buffer[:mark].strip()
            self.command_list.append(cmd)
            self.cmd_ready = True
            self.recv_buffer = self.recv_buffer[mark + 1:]

    def _recv_byte(self, byte):
        """
        Non-printable filtering currently disabled because it did not play
        well with extended character sets.
        """
        # Filter out non-printing characters
        # if (byte >= ' ' and byte <= '~') or byte == '\n':
        if self.telnet_echo:
            self._echo_byte(byte)
        self.recv_buffer += byte

    def _echo_byte(self, byte):
        """
        Echo a character back to the client and convert LF into CR\LF.
        """
        if byte == '\n':
            self.send_buffer += '\r'
        if self.telnet_echo_password:
            self.send_buffer += '*'
        else:
            self.send_buffer += byte

    def _iac_sniffer(self, byte):
        """
        Watches incoming data for Telnet IAC sequences.
        Passes the data, if any, with the IAC commands stripped to
        _recv_byte().
        """
        # Are we not currently in an IAC sequence coming from the client?
        if self.telnet_got_iac is False:
            if byte == IAC:
                # Well, we are now
                self.telnet_got_iac = True
                return
            # Are we currenty in a sub-negotion?
            elif self.telnet_got_sb is True:
                # Sanity check on length
                if len(self.telnet_sb_buffer) < 64:
                    self.telnet_sb_buffer += byte
                else:
                    self.telnet_got_sb = False
                    self.telnet_sb_buffer = ""
                return
            else:
                # Just a normal NVT character
                self._recv_byte(byte)
                return
        # Byte handling when already in an IAC sequence sent from the client
        else:
            # Did we get sent a second IAC?
            if byte == IAC and self.telnet_got_sb is True:
                # Must be an escaped 255 (IAC + IAC)
                self.telnet_sb_buffer += byte
                self.telnet_got_iac = False
                return
            # Do we already have an IAC + CMD?
            elif self.telnet_got_cmd:
                # Yes, so handle the option
                self._three_byte_cmd(byte)
                return
            # We have IAC but no CMD
            else:
                # Is this the middle byte of a three-byte command?
                if byte == DO:
                    self.telnet_got_cmd = DO
                    return
                elif byte == DONT:
                    self.telnet_got_cmd = DONT
                    return
                elif byte == WILL:
                    self.telnet_got_cmd = WILL
                    return
                elif byte == WONT:
                    self.telnet_got_cmd = WONT
                    return
                else:
                    # Nope, must be a two-byte command
                    self._two_byte_cmd(byte)

    def _two_byte_cmd(self, cmd):
        """
        Handle incoming Telnet commands that are two bytes long.
        """
        logger.debug("Got two byte cmd '{}'".format(ord(cmd)))

        if cmd == SB:
            # Begin capturing a sub-negotiation string
            self.telnet_got_sb = True
            self.telnet_sb_buffer = ''
        elif cmd == SE:
            # Stop capturing a sub-negotiation string
            self.telnet_got_sb = False
            self._sb_decoder()
        elif cmd == NOP:
            pass
        elif cmd == DATMK:
            pass
        elif cmd == IP:
            pass
        elif cmd == AO:
            pass
        elif cmd == AYT:
            pass
        elif cmd == EC:
            pass
        elif cmd == EL:
            pass
        elif cmd == GA:
            pass
        else:
            logger.warning("Send an invalid 2 byte command")

        self.telnet_got_iac = False
        self.telnet_got_cmd = None

    def _three_byte_cmd(self, option):
        """
        Handle incoming Telnet commands that are three bytes long.
        """
        cmd = self.telnet_got_cmd
        logger.debug("Got three byte cmd {}:{}".format(ord(cmd), ord(option)))

        # Incoming DO's and DONT's refer to the status of this end
        if cmd == DO:
            if option == BINARY or option == SGA or option == ECHO:
                if self._check_reply_pending(option):
                    self._note_reply_pending(option, False)
                    self._note_local_option(option, True)

                elif (self._check_local_option(option) is False or
                              self._check_local_option(option) is UNKNOWN):
                    self._note_local_option(option, True)
                    self._iac_will(option)
                    # Just nod unless setting echo
                    if option == ECHO:
                        self.telnet_echo = True

            else:
                # All other options = Default to refusing once
                if self._check_local_option(option) is UNKNOWN:
                    self._note_local_option(option, False)
                    self._iac_wont(option)

        elif cmd == DONT:
            if option == BINARY or option == SGA or option == ECHO:
                if self._check_reply_pending(option):
                    self._note_reply_pending(option, False)
                    self._note_local_option(option, False)

                elif (self._check_local_option(option) is True or
                              self._check_local_option(option) is UNKNOWN):
                    self._note_local_option(option, False)
                    self._iac_wont(option)
                    # Just nod unless setting echo
                    if option == ECHO:
                        self.telnet_echo = False
            else:
                # All other options = Default to ignoring
                pass

        # Incoming WILL's and WONT's refer to the status of the client
        elif cmd == WILL:
            if option == ECHO:
                # Nutjob client offering to echo the server...
                if self._check_remote_option(ECHO) is UNKNOWN:
                    self._note_remote_option(ECHO, False)
                    # No no, bad client!
                    self._iac_dont(ECHO)

            elif option == NAWS or option == SGA:
                if self._check_reply_pending(option):
                    self._note_reply_pending(option, False)
                    self._note_remote_option(option, True)

                elif (self._check_remote_option(option) is False or
                              self._check_remote_option(option) is UNKNOWN):
                    self._note_remote_option(option, True)
                    self._iac_do(option)
                    # Client should respond with SB (for NAWS)

            elif option == TTYPE:
                if self._check_reply_pending(TTYPE):
                    self._note_reply_pending(TTYPE, False)
                    self._note_remote_option(TTYPE, True)
                    # Tell them to send their terminal type
                    self.send("{}{}{}{}{}{}".format(IAC, SB, TTYPE, SEND, IAC, SE))

                elif (self._check_remote_option(TTYPE) is False or
                              self._check_remote_option(TTYPE) is UNKNOWN):
                    self._note_remote_option(TTYPE, True)
                    self._iac_do(TTYPE)

        elif cmd == WONT:
            if option == ECHO:
                # Client states it wont echo us -- good, they're not supposed to.
                if self._check_remote_option(ECHO) is UNKNOWN:
                    self._note_remote_option(ECHO, False)
                    self._iac_dont(ECHO)

            elif option == SGA or option == TTYPE:
                if self._check_reply_pending(option):
                    self._note_reply_pending(option, False)
                    self._note_remote_option(option, False)

                elif (self._check_remote_option(option) is True or
                              self._check_remote_option(option) is UNKNOWN):
                    self._note_remote_option(option, False)
                    self._iac_dont(option)

                    # Should TTYPE be below this?

            else:
                # All other options = Default to ignoring
                pass
        else:
            logger.warning("Send an invalid 3 byte command")

        self.telnet_got_iac = False
        self.telnet_got_cmd = None

    def _sb_decoder(self):
        """
        Figures out what to do with a received sub-negotiation block.
        """
        bloc = self.telnet_sb_buffer
        if len(bloc) > 2:

            if bloc[0] == TTYPE and bloc[1] == IS:
                self.terminal_type = bloc[2:]
                logger.debug("Terminal type = '{}'".format(self.terminal_type))

            if bloc[0] == NAWS:
                if len(bloc) != 5:
                    logger.warning("Bad length on NAWS SB: " + str(len(bloc)))
                else:
                    self.columns = (256 * ord(bloc[1])) + ord(bloc[2])
                    self.rows = (256 * ord(bloc[3])) + ord(bloc[4])

                logger.info("Screen is {} x {}".format(self.columns, self.rows))

        self.telnet_sb_buffer = ''

    # ---[ State Juggling for Telnet Options ]----------------------------------

    # Sometimes verbiage is tricky. I use 'note' rather than 'set' here
    # because (to me) set infers something happened.

    def _check_local_option(self, option):
        """Test the status of local negotiated Telnet options."""
        if option not in self.telnet_opt_dict:
            self.telnet_opt_dict[option] = TelnetOption()
        return self.telnet_opt_dict[option].local_option

    def _note_local_option(self, option, state):
        """Record the status of local negotiated Telnet options."""
        if option not in self.telnet_opt_dict:
            self.telnet_opt_dict[option] = TelnetOption()
        self.telnet_opt_dict[option].local_option = state

    def _check_remote_option(self, option):
        """Test the status of remote negotiated Telnet options."""
        if option not in self.telnet_opt_dict:
            self.telnet_opt_dict[option] = TelnetOption()
        return self.telnet_opt_dict[option].remote_option

    def _note_remote_option(self, option, state):
        """Record the status of local negotiated Telnet options."""
        if option not in self.telnet_opt_dict:
            self.telnet_opt_dict[option] = TelnetOption()
        self.telnet_opt_dict[option].remote_option = state

    def _check_reply_pending(self, option):
        """Test the status of requested Telnet options."""
        if option not in self.telnet_opt_dict:
            self.telnet_opt_dict[option] = TelnetOption()
        return self.telnet_opt_dict[option].reply_pending

    def _note_reply_pending(self, option, state):
        """Record the status of requested Telnet options."""
        if option not in self.telnet_opt_dict:
            self.telnet_opt_dict[option] = TelnetOption()
        self.telnet_opt_dict[option].reply_pending = state

    # ---[ Telnet Command Shortcuts ]-------------------------------------------

    def _iac_do(self, option):
        """Send a Telnet IAC "DO" sequence."""
        self.send("{}{}{}".format(IAC, DO, option))

    def _iac_dont(self, option):
        """Send a Telnet IAC "DONT" sequence."""
        self.send("{}{}{}".format(IAC, DONT, option))

    def _iac_will(self, option):
        """Send a Telnet IAC "WILL" sequence."""
        self.send("{}{}{}".format(IAC, WILL, option))

    def _iac_wont(self, option):
        """Send a Telnet IAC "WONT" sequence."""
        self.send("{}{}{}".format(IAC, WONT, option))

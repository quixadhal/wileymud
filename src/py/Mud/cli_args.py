# -*- coding: utf-8 -*- line endings: unix -*-
__author__ = 'quixadhal'

import sys
import argparse

from Mud.log_system import Logger
logger = Logger()

class _CLI_ErrorParser(argparse.ArgumentParser):
    """
    The _CLI_ErrorParser is a subclass of argparse for the sole purpose of
    allowing us to route error messages to the standard logging system.

    :return: An object to handle argparse error output
    :rtype: object
    """

    def error(self, message):
        logger.fatal(message)
        sys.exit(2)


class _CLI_Args(object):
    """
    The _CLI_Args class is just a data holder so we can nicely use
    the member notation to get at the values.  This is actually what
    we return when someone tries to make a new instance of CLI_Args.

    :return: An object holding the CLI argument values
    :rtype: object
    """

    def __init__(self):
        self.wizlock = None
        self.debug = None
        self.specials = None
        self.logfile = None
        self.pidfile = None
        self.directory = None
        self.port = None

    def __str__(self):
        return ', '.join([ '='.join([k,str(v)]) for k,v in self.__dict__.items() ])


class _Arg_Range(argparse.Action):
    def __init__(self, min=None, max=None, *args, **kwargs):
        self.min = min
        self.max = max
        #kwargs["metavar"] = "%s [%d-%d]" % (kwargs['dest'], self.min, self.max)
        super(_Arg_Range, self).__init__(*args, **kwargs)

    def __call__(self, parser, namespace, value, option_string=None):
        if not (self.min <= value <= self.max):
            msg = 'invalid %s number: %r (choose from [%d-%d])' % \
                (self.dest, value, self.min, self.max)
            raise argparse.ArgumentError(self, msg)
        setattr(namespace, self.dest, value)


class CLI_Args(object):
    """
    The CLI_Args class uses argparse to pick apart command line arguments
    and stores the results in the object.  It will then allow the arguments
    passed to be queried.

    :return: CLI argument values
    :rtype: object
    """

    __instance = None       # There can only be one


    def __new__(cls):
        if cls.__instance is None:
            parser = _CLI_ErrorParser(description='WileyMUD game server')
            parser.add_argument('-w', '--wizlock', action='store_true', help='only allow wizards to connect')
            parser.add_argument('-D', '--debug', action='store_true', help='enable extra debugging')
            parser.add_argument('-s', '--specials', action='store_false', help='suppress special procedures')
            parser.add_argument('-L', '--logfile', type=str, help='file to write logs to')
            parser.add_argument('-P', '--pidfile', type=str, help='file to write process ID to for init')
            parser.add_argument('-d', '--directory', type=str, help='working directory to switch to')
            parser.add_argument('port', nargs='?', type=int, action=_Arg_Range, default=3000, min=1024, max=65535, help='player port number [%(min)d-%(max)d] for new connections')
            args = parser.parse_args()
            cls.__instance = _CLI_Args()
            cls.__instance.wizlock = args.wizlock
            cls.__instance.debug = args.debug
            cls.__instance.specials = args.specials
            cls.__instance.logfile = args.logfile
            cls.__instance.pidfile = args.pidfile
            cls.__instance.directory = args.directory
            cls.__instance.port = args.port
        return cls.__instance


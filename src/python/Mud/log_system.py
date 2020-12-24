# -*- coding: utf-8 -*- line endings: unix -*-
__author__ = 'quixadhal'

import logging
import datetime

# Define a few custom log levels
# WileyMUD had INFO, ERROR, FATAL, BOOT, AUTH, KILL, DEATH, RESET, and IMC
# Default Python logging levels are CRITICAL 50, ERROR 40, WARNING 30, INFO 20, DEBUG 10
#
# Python logging works by having a debug level set, and any error class whose priority
# is above that level is suppressed.


class Logger(object):
    """
    The Logger class is simply a singleton wrapper to ensure we properly
    initialize a single copy of our master logging object, and then return
    that copy, no matter how many times callers might try to get a "new"
    logging object.

    We modify settings to the global logger, so it is beneficial to ensure
    this is run at the very top of every module in your project, including
    before other imports happen, so that THEY also end up using the same
    logging code.

    :return: The master logging object
    :rtype: object
    """

    class LogReformatter(logging.Formatter):
        """
        The LogReformatter class allows us to change the formatting to properly
        align the text strings of a multi-line log entry to line up under the
        initial one, padding over the timestamp and other information.

        We also change the timestamp to add milliseconds as a faction, rather
        than the bizzare comma version that's the default.

        :return: An object which does proper formatting of our log entries
        :rtype: object
        """

        #ut = datetime.datetime.fromtimestamp(time.time())
        #print(ut.astimezone().tzinfo)

        time_converter = datetime.datetime.fromtimestamp

        def format(self, record):
            message = super().format(record)
            barpos = message.find('| ')
            if barpos >= 0:
                message = message.replace('\n', '\n' + ' '.ljust(barpos + 2)) 
            return message

        def formatTime(self, record, datefmt=None):
            timestamp = self.time_converter(record.created).astimezone()
            if datefmt:
                result = timestamp.strftime(datefmt)
            else:
                part = timestamp.strftime('%Y-%m-%d %H:%M:%S')
                result = "%s.%03d %s" % (part, record.msecs, timestamp.tzinfo)
            return result


    __instance = None       # Allow us to always return a singleton object


    def __new__(cls):
        if cls.__instance is None:
            format_string = '%(asctime)s %(levelname)-8s %(module)16s| %(message)s'
            logging.basicConfig(format=format_string, level=logging.DEBUG)
            logging.addLevelName(39, 'AUTH')
            logging.addLevelName(38, 'PK')
            logging.addLevelName(31, 'BOOT')
            logging.addLevelName(29, 'RESET')
            logging.addLevelName(21, 'KILL')
            logging.Logger.auth = cls.auth_log
            logging.Logger.pk = cls.pk_log
            logging.Logger.boot = cls.boot_log
            logging.Logger.reset = cls.reset_log
            logging.Logger.kill = cls.kill_log

            cls.__instance = logging.getLogger()
            f = cls.LogReformatter(format_string)
            h = logging.StreamHandler()
            h.setFormatter(f)
            cls.__instance.handlers = []
            cls.__instance.addHandler(h)

        return cls.__instance


    def auth_log(self, message, *args, **kws):
        if self.level <= 39:
            self._log(39, message, args, **kws)


    def pk_log(self, message, *args, **kws):
        if self.level <= 38:
            self._log(38, message, args, **kws)


    def boot_log(self, message, *args, **kws):
        if self.level <= 31:
            self._log(31, message, args, **kws)


    def reset_log(self, message, *args, **kws):
        if self.level <= 29:
            self._log(29, message, args, **kws)


    def kill_log(self, message, *args, **kws):
        if self.level <= 21:
            self._log(21, message, args, **kws)

#!/usr/bin/python3 -B
# -*- coding: utf-8 -*- line endings: unix -*-
__author__ = 'quixadhal'

import os
import sys
import datetime
sys.path.append(os.path.join(os.getcwd(), "Mud"))

# Normally, we always import logging and initialize it BEFORE anything
# else, but in this case, we first need to add to our module search
# path, so we'll trust nothing will explode in os or sys at the top
# level here.

from Mud.log_system import Logger
logger = Logger()

from Mud.cli_args import CLI_Args
from Mud.signals import setup_signals

def changedir(pathname):
    if pathname is not None:
        try:
            os.chdir(pathname) 
        except FileNotFoundError as err:
            logger.critical('Invalid directory: %s', err)
            sys.exit(2)
        except PermissionError as err:
            logger.critical('Invalid directory: %s', err)
            sys.exit(2)
    return True


def makepid(pathname):
    if pathname is not None:
        try:
            f = open(pathname, 'w')
            f.write(str(os.getpid()))
            f.close()
        except FileNotFoundError as err:
            logger.critical('Invalid PID file: %s', err)
            sys.exit(2)
        except PermissionError as err:
            logger.critical('Invalid PID file: %s', err)
            sys.exit(2)
    return True


def removefile(pathname):
    if pathname is not None:
        try:
            os.unlink(pathname)
        except FileNotFoundError as err:
            return True
        except PermissionError as err:
            logger.critical('Invalid file: %s', err)
            sys.exit(2)
    return True


if __name__ == '__main__':
    logger.boot('System initializing.')
    start_time = datetime.datetime.utcnow()
    args = CLI_Args()

    changedir(args.directory)
    makepid(args.pidfile)
    removefile(args.logfile)
    logger.start_filelog(args.logfile)

    logger.boot('System booting.')
    logger.info(args)
    logger.info("Player port is %d" % args.port)

    # Here is where we call the outer game loop
    # In wiley's comm.c, that would be run_the_game()

    # set descriptor list to empty
    setup_signals()
    # s = init_socket(port) # Main socket connection
    # load_db()
    # init_whod(port) # web server who daemon
    # i3_startup(false, 3000, false)
    # game_loop()
    # i3_shutdown()
    # close_sockets(s)
    # close_whod()
    # unload_db()
    # if reboot message about rebooting, return reboot code
    # else message about shutdown, return shutdown code

    # And here we'd be done with that...

    run_length = datetime.datetime.utcnow() - start_time
    logger.info('System was up for %s' % run_length)

    logger.critical('System halting.')
    removefile(args.pidfile)
    logger.stop_filelog()
    logger.critical('System halted.')

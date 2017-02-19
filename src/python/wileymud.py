#!/usr/bin/python3 -B
# -*- coding: utf-8 -*- line endings: unix -*-
__author__ = 'quixadhal'

import os
import sys
sys.path.append(os.path.join(os.getcwd(), "Mud"))

# Normally, we always import logging and initialize it BEFORE anything
# else, but in this case, we first need to add to our module search
# path, so we'll trust nothing will explode in os or sys at the top
# level here.

from Mud.log_system import Logger
logger = Logger()
from Mud.db_system import Database
db = Database()


if __name__ == '__main__':
    logger.boot('System booting.')
    session = db.Session()

    from Mud.option import Option
    options = session.query(Option).first()
    if options is None:
        logger.critical('Database failed to initialize!')
        sys.exit()

    logger.boot('Using database version %s, created on %s', options.version, options.date_created)
    logger.boot('Port number is %d', options.gameport)
    logger.boot('Wizlock is %s', options.wizlock)

    logger.critical('System halted.')

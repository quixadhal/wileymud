# -*- coding: utf-8 -*- line endings: unix -*-
__author__ = 'quixadhal'

import sys

from Mud.log_system import Logger
logger = Logger()

class _GameData(object):
    """
    This is just a data holder.  This is what gets returned when
    you try to make a new Game_Data object.

    :return: An object holding the game dataset
    :rtype: object
    """

    def __init__(self):
        self.loaded = False

    def __str__(self):
        return ', '.join([ '='.join([k,str(v)]) for k,v in self.__dict__.items() ])

    def import_wileymud(self):
        self.loaded = False
        logger.boot('Importing data from WileyMUD files.')
        self.loaded = True
        logger.boot('Import completed!')
        return self.loaded


class Game_Data(object):
    """
    This class reads in the old WileyMUD data files, as well as entries
    from the SQL database.  Once all objects have been instantiated and
    are in a ready-to-run state, we can export them to whatever
    format we want to use going forward, be that an ORM, all SQL, or
    JSON.

    :return: A nested dictionary of the game dataset
    :rtype: object
    """

    __instance = None       # There can only be one


    def __new__(cls):
        if cls.__instance is None:
            cls.__instance = _GameData()
        return cls.__instance


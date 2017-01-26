# -*- coding: utf-8 -*- line endings: unix -*-
__author__ = 'quixadhal'

from sqlalchemy import Column, Integer, String, Boolean, DateTime
from sqlalchemy.sql import func
from log_system import Logger
from db_system import DataBase

logger = Logger()


class Option(DataBase):
    __tablename__ = 'option'

    id = Column(Integer, primary_key=True)
    date_created = Column(DateTime, default=func.now())
    version = Column(String)
    gameport = Column(Integer, default=4400)
    wizlock = Column(Boolean, default=False)
    debug = Column(Boolean, default=False)
    specials = Column(Boolean, default=True)
    pidfile = Column(String)

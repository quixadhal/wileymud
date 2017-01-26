# -*- coding: utf-8 -*- line endings: unix -*-
__author__ = 'quixadhal'

from sqlalchemy import Column
from sqlalchemy.types import DateTime, Integer, String
from sqlalchemy.sql import func
from log_system import Logger
from db_system import DataBase

logger = Logger()


class Log(DataBase):
    __tablename__ = 'log'

    id = Column(Integer, primary_key=True)
    level = Column(String)
    module = Column(String)
    function = Column(String)
    line = Column(Integer)
    trace = Column(String)
    msg = Column(String)
    date_created = Column(DateTime, default=func.now())

    def __init__(self, level=None, module=None, function=None, line=None, trace=None, msg=None, created=None):
        self.level = level
        self.module = module
        self.function = function
        self.line = line
        self.trace = trace
        self.msg = msg
        self.date_created = created

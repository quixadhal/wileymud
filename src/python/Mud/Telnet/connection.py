# -*- coding: utf-8 -*- line endings: unix -*-
__author__ = 'quixadhal'

from sqlalchemy import Column, Integer, String, Boolean, DateTime
from sqlalchemy.sql import func
from log_system import Logger
from db_system import DataBase

logger = Logger()


class Connection(DataBase):
    __tablename__ = 'connection'

    fileno = Column(Integer, primary_key=True)
    login_date = Column(DateTime, default=func.now())
    remote_address = Column(String)
    remote_port = Column(Integer)
    local_address = Column(String)
    local_port = Column(Integer)
    warmboot_restored = Column(Boolean, default=False)
    # When doing a warmboot, we will go through each
    # connection and restore their socket objects to
    # the file descriptor listed, setting each to True
    # to ensure we reconnect everyone.
    connected = Column(Boolean, default=True)
    disconnect_time = Column(DateTime)
    # When we detect a lost connection, we set the disconnect_time
    # If the user reconnects within a timeout value, we update the
    # connection, rather than logging them out.
    username = Column(String)
    # This allows us to reconnect players who become disconnected
    # without having to log their characters out and back in
    # For now, it's just a string... it may need to change
    # based on future account system details

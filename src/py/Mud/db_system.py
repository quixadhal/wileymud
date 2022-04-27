# -*- coding: utf-8 -*- line endings: unix -*-
__author__ = 'quixadhal'

import os
import sys
from sqlalchemy.ext.declarative import declarative_base
from datetime import datetime
from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from sqlalchemy.orm import scoped_session
from sqlalchemy.orm.exc import MultipleResultsFound, NoResultFound
from sqlalchemy.exc import IntegrityError, OperationalError
from sqlalchemy import Column, Integer, String, Boolean, DateTime, Sequence
import log_system

logger = log_system.init_logging()
sys.path.append(os.getcwd())
#DB_FILE = 'i3.db'

#SQLEngine = create_engine('sqlite:///' + DB_FILE)
SQLEngine = create_engine('postgresql://wiley:tardis69@localhost/py3')
DataBase = declarative_base()
SessionFactory = sessionmaker(bind=SQLEngine)
Session = scoped_session(SessionFactory)


class ErrorLog(DataBase):
    __tablename__ = 'errorlog'
    id = Column(Integer, primary_key=True, autoincrement=True)
    created = Column(DateTime)
    relativecreated = Column(Integer)
    name = Column(String)
    levelno = Column(Integer)
    levelname = Column(String)
    message = Column(String)
    filename = Column(String)
    pathname = Column(String)
    lineno = Column(Integer)
    msecs = Column(Integer)
    exc_text = Column(String)
    thread = Column(String)

    def __repr__(self):
        return "<ErrorLog(" + \
               "id = %d," \
               "created = '%s'," \
               "relativecreated = %d," \
               "name = '%s'," \
               "levelno = %d," \
               "levelname = '%s'," \
               "message = '%s'," \
               "filename = '%s'," \
               "pathname = '%s'," \
               "lineno = %d," \
               "msecs = %d," \
               "exc_text = '%s'," \
               "thread = '%s'," \
               ")>" % (
                   self.id,
                   self.created,
                   self.relativecreated,
                   self.name,
                   self.levelno,
                   self.levelname,
                   self.message,
                   self.filename,
                   self.pathname,
                   self.lineno,
                   self.msecs,
                   self.exc_text,
                   self.thread
               )


def init_db():
    connection = SQLEngine.connect()
    import config
    from config import Config
    DataBase.metadata.create_all(SQLEngine)
    session = Session()
    try:
        config = session.query(Config).order_by(Config.router_name).first()
    except (NoResultFound, IntegrityError, OperationalError):
        logger.error('No result found, initializing database')
        DataBase.metadata.create_all(SQLEngine)
        config.setup_default()
    return config

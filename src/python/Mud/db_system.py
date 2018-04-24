# -*- coding: utf-8 -*- line endings: unix -*-
__author__ = 'quixadhal'

import os
import sys
import logging
import datetime
import traceback
from sqlalchemy.ext.declarative import declarative_base
from datetime import datetime
from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from sqlalchemy.orm import scoped_session
from alembic.migration import MigrationContext
from alembic.config import Config
from alembic.script import ScriptDirectory
from alembic import command
from log_system import Logger

logger = Logger()
DB_FILE = 'wiley.db'
ALEMBIC_CONFIG = 'Mud/alembic/alembic.ini'

SQLEngine = create_engine('sqlite:///' + DB_FILE)
DataBase = declarative_base()
__SessionFactory = sessionmaker(bind=SQLEngine)
Session = scoped_session(__SessionFactory)

# WHenever a new database element is added or changed, be sure to do
# > alembic -c Mud/alembic/alembic.ini revision --autogenerate -m 'message about the change'
# and then also go hand tweak the database script it creates so you're sure it will
# properly upgrade the database.


class Database(object):
    __instance = None

    def __new__(cls):
        if cls.__instance is None:
            i = object.__new__(cls)

            i.SQLEngine = SQLEngine
            i.DataBase = DataBase
            i.Session = Session

            i.connection = SQLEngine.connect()
            i.context = MigrationContext.configure(i.connection)
            i.current_revision = i.context.get_current_revision()
            logger.boot('Database revision: %s', i.current_revision)

            i.config = Config(ALEMBIC_CONFIG)
            i.script = ScriptDirectory.from_config(i.config)
            i.head_revision = i.script.get_current_head()
            if i.current_revision is None or i.current_revision != i.head_revision:
                logger.boot('Upgrading database to version %s.', i.head_revision)
                command.upgrade(i.config, 'head')
                from option import Option
                from log import Log
                session = Session()
                options = session.query(Option).first()
                if options is None:
                    options = Option()
                    session.add(options)
                options.version = i.head_revision
                session.commit()
                i.current_revision = i.head_revision

            cls.__instance = i
            h = SQLAlchemyHandler()
            logger.addHandler(h)
            return cls.__instance


class SQLAlchemyHandler(logging.Handler):

    time_converter = datetime.fromtimestamp

    def emit(self, record):
        from log import Log
        timestamp = self.time_converter(record.created)
        trace = None
        exc = record.exc_info
        if exc:
            trace = traceback.format_exc(exc)
        log = Log(
                level = record.levelname,
                module = record.module,
                function = record.funcName,
                line = record.lineno,
                trace = trace,
                msg = record.getMessage(),
                created = timestamp
                )
        session = Session()
        session.add(log)
        session.commit()

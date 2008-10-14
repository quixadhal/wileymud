/*
 * SQuirreLMUD 1.0 - Copyright (C) 2003 - Chris Meshkin
 *
 * This is the low-level SQL interface code for PostgreSQL 7.x
 *
 * File:        $Source: /home/cvs/root/squirrel/src/sql.c,v $
 * Revision:    $Revision: 1.2 $
 * Date:        $Date: 2003/07/26 06:28:08 $
 * Author:      $Author: quixadhal $
 * CVS_ID:      $Id: sql.c,v 1.2 2003/07/26 06:28:08 quixadhal Exp $
 * $Log: sql.c,v $
 * Revision 1.2  2003/07/26 06:28:08  quixadhal
 * Progress, now have a working bug system
 *
 * Revision 1.1  2003/07/26 03:57:39  quixadhal
 * Version 0
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

/* #include "config.h" */
#include "mud.h"

/* #define TESTING */

#define _SQL_C
#include "sql.h"

#ifdef USE_POSTGRES
PGconn *dbc = NULL;
#endif /* USE_POSTGRES */

#ifdef USE_POSTGRES
sql_connect_data pgcdata = {
  DBHOST,
  DBPORT,
  DBNAME,
  DBOPTS,
  DBUSER,
  DBPASS
};

PGconn *sql_open(sql_connect_data *cdata)
{
  char *s = NULL;

  dbc = PQsetdbLogin(cdata->dbhost, cdata->dbport,
                     cdata->dbopts,
                     NULL, cdata->dbname,
                     cdata->dbuser, cdata->dbpasswd);
  if(dbc) {
    s = sql_version();
    if(!s || !*s) {
      sql_close();
      dbc = NULL;
      return NULL;
    }
  }
  return dbc;
}
#endif /* USE_POSTGRES */

#ifdef USE_POSTGRES
void sql_close(void)
{
  if(dbc) {
    PQfinish(dbc);
  }
}
#endif /* USE_POSTGRES */

#ifdef USE_POSTGRES
char *sql_error(ExecStatusType estat)
{
  static char last_err[GIANT_BUF];
  char *err_str = NULL;
  char *pgerr = NULL;

  if(estat) err_str = PQresStatus(estat);
  if(dbc) pgerr = PQerrorMessage(dbc);
  bzero(last_err, GIANT_BUF);
  snprintf(last_err, GIANT_BUF-1, "%s%s%s",
           err_str?err_str:"",
           pgerr?" ":"",
           pgerr?pgerr:"");

#ifndef TESTING
  log_string("SQL: %s", last_err);
#else
  printf("SQL: %s\n", last_err);
#endif
  return last_err;
}
#endif /* USE_POSTGRES */

#ifdef USE_POSTGRES
int sql_execute(const char *query, ... )
{
  PGresult *rs = NULL;
  ExecStatusType rstat;
  long int rows = 0;
  char tmp[GIANT_BUF];
  va_list param;

  if(!dbc)
    return -1;
  if(PQstatus(dbc) == CONNECTION_BAD)
    return -1;

  va_start( param, query );
  vsnprintf( tmp, GIANT_BUF, query, param );
  va_end( param );

  rs = PQexec(dbc, "BEGIN");
  if ((rstat = PQresultStatus(rs)) != PGRES_COMMAND_OK) {
    PQclear(rs);
    sql_error(rstat);
    return -1;
  }
  PQclear(rs);
  rs = PQexec(dbc, tmp);
  if ((rstat = PQresultStatus(rs)) != PGRES_COMMAND_OK) {
    PQclear(rs);
    sql_error(rstat);
    rs = PQexec(dbc, "ROLLBACK");
    PQclear(rs);
    return -1;
  } else {
    rows = atol(PQcmdTuples(rs));
  }
  PQclear(rs);
  rs = PQexec(dbc, "COMMIT");
  if ((rstat = PQresultStatus(rs)) != PGRES_COMMAND_OK) {
    PQclear(rs);
    sql_error(rstat);
    return -1;
  }
  PQclear(rs);
  return rows;
}
#endif /* USE_POSTGRES */

#ifdef USE_POSTGRES
sql_query_result * sql_query(const char *query, ... )
{
  PGresult *rs = NULL;
  ExecStatusType rstat;
  char tmp[GIANT_BUF];
  char tquery[GIANT_BUF];
  va_list param;
  sql_query_result *result = NULL;
  register int i;
  register int j;

  if(!dbc)
    return NULL;
  if(PQstatus(dbc) == CONNECTION_BAD)
    return NULL;

  snprintf( tquery, GIANT_BUF, "DECLARE bucket CURSOR FOR %s", query );

  va_start( param, query );
  vsnprintf( tmp, GIANT_BUF, tquery, param );
  va_end( param );

  rs = PQexec(dbc, "BEGIN");
  if ((rstat = PQresultStatus(rs)) != PGRES_COMMAND_OK) {
    PQclear(rs);
    sql_error(rstat);
    return NULL;
  }
  PQclear(rs);

  rs = PQexec(dbc, tmp);
  if ((rstat = PQresultStatus(rs)) != PGRES_COMMAND_OK) {
    PQclear(rs);
    sql_error(rstat);
    rs = PQexec(dbc, "ROLLBACK");
    PQclear(rs);
    return NULL;
  }
  PQclear(rs);
  rs = PQexec(dbc, "FETCH ALL IN bucket");
  if ((rstat = PQresultStatus(rs)) != PGRES_TUPLES_OK) {
    PQclear(rs);
    sql_error(rstat);
    rs = PQexec(dbc, "ROLLBACK");
    PQclear(rs);
    return NULL;
  }

  result = (sql_query_result *)calloc(1, sizeof(sql_query_result *));
  result->cols = PQnfields(rs);
  result->rows = PQntuples(rs);
  result->data = (char ***) calloc(result->rows, sizeof(char **));
  for(i = 0; i < result->rows; i++) {
    result->data[i] = (char **) calloc(result->cols, sizeof(char *));
    for(j = 0; j < result->cols; j++) {
      result->data[i][j] = (char *)strdup(PQgetvalue(rs, i, j));
    }
  }

  PQclear(rs);
  rs = PQexec(dbc, "CLOSE bucket");
  PQclear(rs);
  rs = PQexec(dbc, "END");
  PQclear(rs);

  return result;
}
#endif /* USE_POSTGRES */

#ifdef USE_POSTGRES
void sql_free_result(sql_query_result *result)
{
  register int i;
  register int j;

  if(result) {
    if(result->data) {
      for(i = 0; i < result->rows; i++) {
        if(result->data[i]) {
          for(j = 0; j < result->cols; j++) {
            if(result->data[i][j])
              free(result->data[i][j]);
          }
          free(result->data[i]);
        }
      }
      free(result->data);
    }
    free(result);
  }
  return;
}
#endif /* USE_POSTGRES */

#ifdef USE_POSTGRES
char *sql_last_error(void)
{
  static char last_err[GIANT_BUF];
  char *pgerr = NULL;

  if(!dbc)
    return "No database handle!";
  pgerr = PQerrorMessage(dbc);
  if(pgerr && *pgerr) {
    strncpy(last_err, pgerr, GIANT_BUF-1);
  } else {
    bzero(last_err, GIANT_BUF);
  }
  return last_err;
}
#endif /* USE_POSTGRES */

#ifdef USE_POSTGRES
const char *sql_escape(const char *str)
{
  static char to[GIANT_BUF*2 +1];
  char from[GIANT_BUF];
  size_t to_len = 0;

  bzero(from, GIANT_BUF);
  strncpy(from, str, GIANT_BUF-1);
  to_len = PQescapeString(to, from, strlen(from));
  return to;
}
#endif /* USE_POSTGRES */

#ifdef USE_POSTGRES
char *sql_version(void)
{
  static char version[GIANT_BUF];
  PGresult *rs = NULL;
  ExecStatusType rstat;

  bzero(version, GIANT_BUF);
  if (dbc) {
    if (PQstatus(dbc) == CONNECTION_OK) {
      rs = PQexec(dbc, "BEGIN");
      if ((rstat = PQresultStatus(rs)) == PGRES_COMMAND_OK) {
        PQclear(rs);
        rs = PQexec(dbc, "DECLARE mycurser CURSOR FOR select version()");
	if ((rstat = PQresultStatus(rs)) == PGRES_COMMAND_OK) {
          PQclear(rs);
          rs = PQexec(dbc, "FETCH ALL in mycurser");
	    if ((rstat = PQresultStatus(rs)) == PGRES_TUPLES_OK) {
            strncpy(version, PQgetvalue(rs, 0, 0), GIANT_BUF);
            version[GIANT_BUF-1] = '\0';
          } else {
            version[0] = '\0';
            sql_error(rstat);
          }
          PQclear(rs);
          rs = PQexec(dbc, "CLOSE mycurser");
          PQclear(rs);
          rs = PQexec(dbc, "END");
        } else {
          sql_error(rstat);
        }
      } else {
        sql_error(rstat);
      }
      PQclear(rs);
    }
  }
  return version;
}
#endif /* USE_POSTGRES */

#ifdef USE_POSTGRES
char *sql_status(void)
{
  static char sbuf[GIANT_BUF];

  bzero(sbuf, GIANT_BUF);
  snprintf( sbuf + strlen(sbuf), GIANT_BUF - strlen(sbuf),
            "%20.20s: %s\n", "Database Type", "PostgreSQL");
  snprintf( sbuf + strlen(sbuf), GIANT_BUF - strlen(sbuf),
            "%20.20s: %s\n", "Host", PQhost(dbc));
  snprintf( sbuf + strlen(sbuf), GIANT_BUF - strlen(sbuf),
            "%20.20s: %s\n", "Port", PQport(dbc));
  snprintf( sbuf + strlen(sbuf), GIANT_BUF - strlen(sbuf),
            "%20.20s: %s\n", "Database", PQdb(dbc));
  snprintf( sbuf + strlen(sbuf), GIANT_BUF - strlen(sbuf),
            "%20.20s: %s\n", "User", PQuser(dbc));
  snprintf( sbuf + strlen(sbuf), GIANT_BUF - strlen(sbuf),
            "%20.20s: %s\n", "Password", PQpass(dbc));
  snprintf( sbuf + strlen(sbuf), GIANT_BUF - strlen(sbuf),
            "%20.20s: %s\n", "TTY", PQtty(dbc));
  snprintf( sbuf + strlen(sbuf), GIANT_BUF - strlen(sbuf),
            "%20.20s: %s\n", "Options", PQoptions(dbc));
  snprintf( sbuf + strlen(sbuf), GIANT_BUF - strlen(sbuf),
            "%20.20s: %d\n", "Backend PID", PQbackendPID(dbc));
  snprintf( sbuf + strlen(sbuf), GIANT_BUF - strlen(sbuf),
            "%20.20s: %d\n", "PID", getpid());
  snprintf( sbuf + strlen(sbuf), GIANT_BUF - strlen(sbuf),
            "%20.20s: %s\n", "Last Error", sql_last_error());
  snprintf( sbuf + strlen(sbuf), GIANT_BUF - strlen(sbuf),
            "%20.20s: %s\n", "Version", sql_version());
  return sbuf;
}
#endif /* USE_POSTGRES */

#ifdef USE_POSTGRES
pid_t sql_backend(void)
{
  return (pid_t) PQbackendPID(dbc);
}
#endif /* USE_POSTGRES */

#ifdef USE_POSTGRES
int sql_table_exists(char *table)
{
  PGresult *rs = NULL;
  ExecStatusType rstat;
  char sqlbuf[GIANT_BUF];
  int result = 0;

  bzero(sqlbuf, GIANT_BUF);
  if (dbc && table && *table) {
    if (PQstatus(dbc) == CONNECTION_OK) {
      rs = PQexec(dbc, "BEGIN");
      if ((rstat = PQresultStatus(rs)) == PGRES_COMMAND_OK) {
        PQclear(rs);
        snprintf(sqlbuf, GIANT_BUF, "DECLARE mycurser CURSOR FOR "
                 "SELECT relname FROM pg_class WHERE relname = '%s'",
                 table);
        rs = PQexec(dbc, sqlbuf);
	if ((rstat = PQresultStatus(rs)) == PGRES_COMMAND_OK) {
          PQclear(rs);
          rs = PQexec(dbc, "FETCH ALL in mycurser");
	  if ((rstat = PQresultStatus(rs)) == PGRES_TUPLES_OK) {
            if(PQntuples(rs) > 0) 
              result = 1;
          } else {
            sql_error(rstat);
          }
          PQclear(rs);
          rs = PQexec(dbc, "CLOSE mycurser");
          PQclear(rs);
          rs = PQexec(dbc, "END");
        } else {
          sql_error(rstat);
        }
      } else {
        sql_error(rstat);
      }
      PQclear(rs);
    }
  }
  return result;
}
#endif /* USE_POSTGRES */

#ifdef USE_POSTGRES
int sql_field_exists(char *table, char *field)
{
  PGresult *rs = NULL;
  ExecStatusType rstat;
  char sqlbuf[GIANT_BUF];
  int result = 0;

  bzero(sqlbuf, GIANT_BUF);
  if (dbc && table && *table && field && *field) {
    if (PQstatus(dbc) == CONNECTION_OK) {
      rs = PQexec(dbc, "BEGIN");
      if ((rstat = PQresultStatus(rs)) == PGRES_COMMAND_OK) {
        PQclear(rs);
        snprintf(sqlbuf, GIANT_BUF, "DECLARE mycurser CURSOR FOR "
                 "SELECT * FROM %s LIMIT 1",
                 table);
        rs = PQexec(dbc, sqlbuf);
	if ((rstat = PQresultStatus(rs)) == PGRES_COMMAND_OK) {
          PQclear(rs);
          rs = PQexec(dbc, "FETCH ALL in mycurser");
	  if ((rstat = PQresultStatus(rs)) == PGRES_TUPLES_OK) {
            if(PQfnumber(rs, field) >= 0)
              result = 1;
          } else {
            sql_error(rstat);
          }
          PQclear(rs);
          rs = PQexec(dbc, "CLOSE mycurser");
          PQclear(rs);
          rs = PQexec(dbc, "END");
        } else {
          sql_error(rstat);
        }
      } else {
        sql_error(rstat);
      }
      PQclear(rs);
    }
  }
  return result;
}
#endif /* USE_POSTGRES */

#ifdef USE_POSTGRES
int sql_truncate(char *table)
{
  PGresult *rs = NULL;
  ExecStatusType rstat;
  char query[GIANT_BUF];

  bzero(query, GIANT_BUF);
  if(!dbc)
    return 0;
  if(PQstatus(dbc) == CONNECTION_BAD)
    return 0;

  rs = PQexec(dbc, "BEGIN");
  if ((rstat = PQresultStatus(rs)) != PGRES_COMMAND_OK) {
    PQclear(rs);
    sql_error(rstat);
    return 0;
  }
  PQclear(rs);
  snprintf(query, GIANT_BUF, "TRUNCATE %s", table);
  rs = PQexec(dbc, query);
  if ((rstat = PQresultStatus(rs)) != PGRES_COMMAND_OK) {
    PQclear(rs);
    sql_error(rstat);
    rs = PQexec(dbc, "ROLLBACK");
    PQclear(rs);
    return 0;
  }
  PQclear(rs);
  rs = PQexec(dbc, "COMMIT");
  if ((rstat = PQresultStatus(rs)) != PGRES_COMMAND_OK) {
    PQclear(rs);
    sql_error(rstat);
    return 0;
  }
  PQclear(rs);
  return 1;
}
#endif /* USE_POSTGRES */

#ifdef TESTING
int main(int argc, char **argv)
{
  sql_query_result *result = NULL;
  sql_connect_data connect_me = {
    NULL, NULL, "socketmud", NULL, "quixadhal", NULL
  };
  register int i = 0;
  register int j = 0;

  sql_open(&connect_me);
  printf("%s\n", sql_status());
  printf("foo exists: %d\n", sql_table_exists("foo"));
  printf("log_types exists: %d\n", sql_table_exists("log_types"));
  printf("foo exists: %d\n", sql_table_exists("foo"));
  result = sql_query("SELECT * FROM log_types ORDER BY log_type_id");
  if(result) {
    if(result->data) {
      for(i = 0; i < result->rows; i++) {
        if(result->data[i]) {
          printf("row %d:\t", i);
          for(j = 0; j < result->cols; j++) {
            if(result->data[i][j])
              printf("%s\t", result->data[i][j]);
          }
          printf("%s", "\n");
        }
      }
    }
  } else {
    printf("%s\n", sql_last_error());
  }
  sql_free_result(result);
  result = NULL;
  sql_close();
  return 1;
}
#endif /* TESTING */

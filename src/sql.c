#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "global.h"
#include "bug.h"

#define __SQL_C__
#include "sql.h"

int db_connected = 0;

int init_sql()
{
  if (db_connected)
    return 1;

#if defined(USE_MYSQL)
  return init_mysql();
#elif defined(USE_PGSQL)
  return init_pgsql();
#endif
  return 0;
}

int close_sql( void )
{
  if (db_connected) {
#if defined(USE_MYSQL)
    return close_mysql();
#elif defined(USE_PGSQL)
    return close_pgsql();
#endif
  }
  return 1;
}

const char *escape_sql( const char *str )
{
   const char *s = NULL;
#if defined(USE_MYSQL)
   s = escape_mysql( str );
#elif defined(USE_PGSQL)
   s = escape_pgsql( str );
#endif
   return s;
}

const char *sanitize_query_sql( char *fmt, ... )
{
   va_list argp;
   int i = 0;
   double j = 0.0;
   char *s = NULL, *out = NULL, *p = NULL;
   char safe[MAX_STRING_LENGTH];
   static char query[MAX_STRING_LENGTH];

   bzero(query, MAX_STRING_LENGTH);
   bzero(safe, MAX_STRING_LENGTH);

   va_start( argp, fmt );
   for( p = fmt, out = query; *p != '\0'; p++ )
   {
      if( *p != '%' )
      {
         *out++ = *p;
         continue;
      }

      switch( *++p )
      {
         case 'c':
            i = va_arg( argp, int );
            out += sprintf( out, "%c", i );
            break;

         case 's':
            s = va_arg( argp, char * );
            if( !s )
            {
               out += sprintf( out, " " );
               break;
            }
            out += sprintf( out, "%s", escape_sql( s ) );
            bzero( safe, MAX_STRING_LENGTH );
            break;

         case 'd':
            i = va_arg( argp, int );
            out += sprintf( out, "%d", i );
            break;

         case 'f':
            j = va_arg( argp, double );
            out += sprintf( out, "%f", j );
            break;

         case '%':
            out += sprintf( out, "%%" );
            break;
      }
   }
   va_end( argp );
   *out = '\0';
   return query;
}

int execute_sql(const char *query, ... )
{
  int result;
  va_list param;

  va_start( param, query );
#if defined(USE_MYSQL)
  result = NULL; /* FIXME */
#elif defined(USE_PGSQL)
  result = execute_pgsql(query, param);
#endif
  va_end( param );
}

query_result * query_sql(const char *query, ... )
{
  query_result *result;
  va_list param;

  va_start( param, query );
#if defined(USE_MYSQL)
  result = NULL; /* FIXME */
#elif defined(USE_PGSQL)
  result = query_pgsql(query, param);
#endif
  va_end( param );
}

void free_result_sql(query_result *result)
{
#if defined(USE_MYSQL)
  /* FIXME */
#elif defined(USE_PGSQL)
  free_result_pgsql(result);
#endif
}

#if defined(USE_MYSQL)
MYSQL myconn;

int init_mysql()
{
   if( !mysql_init( &myconn ) )
   {
      mysql_close( &myconn );
      log_error( "%s: mysql_init() failed.", __FUNCTION__ );
      log_info( "Error: %s.", mysql_error(&myconn) );
      return 0;
   }

   if( !mysql_real_connect( &myconn, sysdata->dbserver, sysdata->dbuser, sysdata->dbpass, sysdata->dbname, 0, NULL, 0 ) )
   {
      mysql_close( &myconn );
      log_error( "%s: mysql_real_connect() failed.", __FUNCTION__ );
      log_info( "Error: %s.", mysql_error(&myconn) );
      return 0;
   }
   mysql_options( &myconn, MYSQL_OPT_RECONNECT, "1" );
   log_string( "Connection to mysql database established." );
   //add_event( 1800, ev_mysql_ping, NULL );
   db_connected = 1;
   return 1;
}

int close_mysql( void )
{
   mysql_close( &myconn );
   db_connected = 0;
   return 1;
}

const char *escape_mysql(const char *str)
{
  static char to[MAX_STRING_LENGTH*2 +1];
  char from[MAX_STRING_LENGTH];
  size_t to_len = 0;

  bzero(from, MAX_STRING_LENGTH);
  strncpy(from, str, MAX_STRING_LENGTH-1);
  to_len = mysql_real_escape_string( &myconn, to, from, strlen(from) );
  return to;
}

#elif defined(USE_PGSQL)

PGconn *dbc = NULL;

int init_pgsql()
{
  static char connect_string[MAX_STRING_LENGTH];

  snprintf(connect_string, MAX_STRING_LENGTH,
           "host=%s port=%d user=%s password=%s dbname=%s connect_timeout=%d",
           DB_HOST, DB_PORT, DB_USER, DB_PASSWD, DB_NAME, 30);
  dbc = PQconnectdb(connect_string);
  if( !dbc )
  {
      log_error( "%s: PQconnectdb() failed.", __FUNCTION__ );
      log_info( "Error: %s.", "PQconnectdb() failed." );
      return 0;
  }
  db_connected = 1;
  return 1;
}

int close_pgsql()
{
  if(dbc) {
    PQfinish(dbc);
  }
  db_connected = 0;
  return 1;
}

const char *escape_pgsql(const char *str)
{
  static char to[MAX_STRING_LENGTH*2 +1];
  char from[MAX_STRING_LENGTH];
  size_t to_len = 0;

  bzero(from, MAX_STRING_LENGTH);
  strncpy(from, str, MAX_STRING_LENGTH-1);
  to_len = PQescapeString(to, from, strlen(from));
  return to;
}

/* This function is typically used for inserts, updates and deletions.
 * Return value is number of rows affected (-1 for failure)
 */
int execute_pgsql(const char *query, va_list param )
{
  PGresult *rs = NULL;
  ExecStatusType rstat;
  long int rows = 0;
  char tmp[MAX_STRING_LENGTH];

  if(!dbc)
    return -1;
  if(PQstatus(dbc) == CONNECTION_BAD)
    return -1;

  vsnprintf( tmp, MAX_STRING_LENGTH, query, param );

  rs = PQexec(dbc, "BEGIN");
  if ((rstat = PQresultStatus(rs)) != PGRES_COMMAND_OK) {
    PQclear(rs);
    error_pgsql(rstat);
    return -1;
  }
  PQclear(rs);
  rs = PQexec(dbc, tmp);
  if ((rstat = PQresultStatus(rs)) != PGRES_COMMAND_OK) {
    PQclear(rs);
    error_pgsql(rstat);
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
    error_pgsql(rstat);
    return -1;
  }
  PQclear(rs);
  return rows;
}

/* This is typically used to fetch data, results are in a nested array */
query_result * query_pgsql(const char *query, va_list param )
{
  PGresult *rs = NULL;
  ExecStatusType rstat;
  char tmp[MAX_STRING_LENGTH];
  char tquery[MAX_STRING_LENGTH];
  query_result *result = NULL;
  register int i;
  register int j;

  if(!dbc)
    return NULL;
  if(PQstatus(dbc) == CONNECTION_BAD)
    return NULL;

  snprintf( tquery, MAX_STRING_LENGTH, "DECLARE bucket CURSOR FOR %s", query );

  vsnprintf( tmp, MAX_STRING_LENGTH, tquery, param );

  rs = PQexec(dbc, "BEGIN");
  if ((rstat = PQresultStatus(rs)) != PGRES_COMMAND_OK) {
    PQclear(rs);
    error_pgsql(rstat);
    return NULL;
  }
  PQclear(rs);

  rs = PQexec(dbc, tmp);
  if ((rstat = PQresultStatus(rs)) != PGRES_COMMAND_OK) {
    PQclear(rs);
    error_pgsql(rstat);
    rs = PQexec(dbc, "ROLLBACK");
    PQclear(rs);
    return NULL;
  }
  PQclear(rs);
  rs = PQexec(dbc, "FETCH ALL IN bucket");
  if ((rstat = PQresultStatus(rs)) != PGRES_TUPLES_OK) {
    PQclear(rs);
    error_pgsql(rstat);
    rs = PQexec(dbc, "ROLLBACK");
    PQclear(rs);
    return NULL;
  }

  result = (query_result *)calloc(1, sizeof(query_result));
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

/* Please remember to free your result set when you've picked out
 * what you want!
 */
void free_result_pgsql(query_result *result)
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

char *error_pgsql(ExecStatusType estat)
{
  static char last_err[MAX_STRING_LENGTH];
  char *err_str = NULL;
  char *pgerr = NULL;

  if(estat) err_str = PQresStatus(estat);
  if(dbc) pgerr = PQerrorMessage(dbc);
  bzero(last_err, MAX_STRING_LENGTH);
  snprintf(last_err, MAX_STRING_LENGTH-1, "%s%s%s",
           err_str ? err_str : "",
           pgerr ? " " : "",
           pgerr ? pgerr : "");

  log_info( "SQL: %s", last_err );
  return last_err;
}

char *last_error_pgsql(void)
{
  static char last_err[MAX_STRING_LENGTH];
  char *pgerr = NULL;

  if(!dbc)
    return "No database handle!";
  pgerr = PQerrorMessage(dbc);
  if(pgerr && *pgerr) {
    strncpy(last_err, pgerr, MAX_STRING_LENGTH-1);
  } else {
    bzero(last_err, MAX_STRING_LENGTH);
  }
  return last_err;
}

int table_exists_pgsql(char *table)
{
  PGresult *rs = NULL;
  ExecStatusType rstat;
  char sqlbuf[MAX_STRING_LENGTH];
  int result = 0;

  bzero(sqlbuf, MAX_STRING_LENGTH);
  if (dbc && table && *table) {
    if (PQstatus(dbc) == CONNECTION_OK) {
      rs = PQexec(dbc, "BEGIN");
      if ((rstat = PQresultStatus(rs)) == PGRES_COMMAND_OK) {
        PQclear(rs);
        snprintf(sqlbuf, MAX_STRING_LENGTH, "DECLARE mycurser CURSOR FOR "
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
            error_pgsql(rstat);
          }
          PQclear(rs);
          rs = PQexec(dbc, "CLOSE mycurser");
          PQclear(rs);
          rs = PQexec(dbc, "END");
        } else {
          error_pgsql(rstat);
        }
      } else {
        error_pgsql(rstat);
      }
      PQclear(rs);
    }
  }
  return result;
}

int field_exists_pgsql(char *table, char *field)
{
  PGresult *rs = NULL;
  ExecStatusType rstat;
  char sqlbuf[MAX_STRING_LENGTH];
  int result = 0;

  bzero(sqlbuf, MAX_STRING_LENGTH);
  if (dbc && table && *table && field && *field) {
    if (PQstatus(dbc) == CONNECTION_OK) {
      rs = PQexec(dbc, "BEGIN");
      if ((rstat = PQresultStatus(rs)) == PGRES_COMMAND_OK) {
        PQclear(rs);
        snprintf(sqlbuf, MAX_STRING_LENGTH, "DECLARE mycurser CURSOR FOR "
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
            error_pgsql(rstat);
          }
          PQclear(rs);
          rs = PQexec(dbc, "CLOSE mycurser");
          PQclear(rs);
          rs = PQexec(dbc, "END");
        } else {
          error_pgsql(rstat);
        }
      } else {
        error_pgsql(rstat);
      }
      PQclear(rs);
    }
  }
  return result;
}

/* This is used to empty a table, much more efficient than mass deletion */
int truncate_pgsql(char *table)
{
  PGresult *rs = NULL;
  ExecStatusType rstat;
  char query[MAX_STRING_LENGTH];

  bzero(query, MAX_STRING_LENGTH);
  if(!dbc)
    return 0;
  if(PQstatus(dbc) == CONNECTION_BAD)
    return 0;

  rs = PQexec(dbc, "BEGIN");
  if ((rstat = PQresultStatus(rs)) != PGRES_COMMAND_OK) {
    PQclear(rs);
    error_pgsql(rstat);
    return 0;
  }
  PQclear(rs);
  snprintf(query, MAX_STRING_LENGTH, "TRUNCATE %s", table);
  rs = PQexec(dbc, query);
  if ((rstat = PQresultStatus(rs)) != PGRES_COMMAND_OK) {
    PQclear(rs);
    error_pgsql(rstat);
    rs = PQexec(dbc, "ROLLBACK");
    PQclear(rs);
    return 0;
  }
  PQclear(rs);
  rs = PQexec(dbc, "COMMIT");
  if ((rstat = PQresultStatus(rs)) != PGRES_COMMAND_OK) {
    PQclear(rs);
    error_pgsql(rstat);
    return 0;
  }
  PQclear(rs);
  return 1;
}

char *version_pgsql(void)
{
  static char version[MAX_STRING_LENGTH];
  PGresult *rs = NULL;
  ExecStatusType rstat;

  bzero(version, MAX_STRING_LENGTH);
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
            strncpy(version, PQgetvalue(rs, 0, 0), MAX_STRING_LENGTH);
            version[MAX_STRING_LENGTH-1] = '\0';
          } else {
            version[0] = '\0';
            error_pgsql(rstat);
          }
          PQclear(rs);
          rs = PQexec(dbc, "CLOSE mycurser");
          PQclear(rs);
          rs = PQexec(dbc, "END");
        } else {
          error_pgsql(rstat);
        }
      } else {
        error_pgsql(rstat);
      }
      PQclear(rs);
    }
  }
  return version;
}

char *status_pgsql(void)
{
  static char sbuf[MAX_STRING_LENGTH];

  bzero(sbuf, MAX_STRING_LENGTH);
  snprintf( sbuf + strlen(sbuf), MAX_STRING_LENGTH - strlen(sbuf),
            "%20.20s: %s\n", "Database Type", "PostgreSQL");
  snprintf( sbuf + strlen(sbuf), MAX_STRING_LENGTH - strlen(sbuf),
            "%20.20s: %s\n", "Host", PQhost(dbc));
  snprintf( sbuf + strlen(sbuf), MAX_STRING_LENGTH - strlen(sbuf),
            "%20.20s: %s\n", "Port", PQport(dbc));
  snprintf( sbuf + strlen(sbuf), MAX_STRING_LENGTH - strlen(sbuf),
            "%20.20s: %s\n", "Database", PQdb(dbc));
  snprintf( sbuf + strlen(sbuf), MAX_STRING_LENGTH - strlen(sbuf),
            "%20.20s: %s\n", "User", PQuser(dbc));
  snprintf( sbuf + strlen(sbuf), MAX_STRING_LENGTH - strlen(sbuf),
            "%20.20s: %s\n", "Password", PQpass(dbc));
  snprintf( sbuf + strlen(sbuf), MAX_STRING_LENGTH - strlen(sbuf),
            "%20.20s: %s\n", "TTY", PQtty(dbc));
  snprintf( sbuf + strlen(sbuf), MAX_STRING_LENGTH - strlen(sbuf),
            "%20.20s: %s\n", "Options", PQoptions(dbc));
  snprintf( sbuf + strlen(sbuf), MAX_STRING_LENGTH - strlen(sbuf),
            "%20.20s: %d\n", "Backend PID", PQbackendPID(dbc));
  snprintf( sbuf + strlen(sbuf), MAX_STRING_LENGTH - strlen(sbuf),
            "%20.20s: %d\n", "PID", getpid());
  snprintf( sbuf + strlen(sbuf), MAX_STRING_LENGTH - strlen(sbuf),
            "%20.20s: %s\n", "Last Error", last_error_pgsql());
  snprintf( sbuf + strlen(sbuf), MAX_STRING_LENGTH - strlen(sbuf),
            "%20.20s: %s\n", "Version", version_pgsql());
  return sbuf;
}

#endif

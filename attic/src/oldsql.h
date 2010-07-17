/*
 * SQuirreLMUD 1.0 - Copyright (C) 2003 - Chris Meshkin
 *
 * This contains settings and declarations for the SQL
 * subsystem of the game.
 *
 * File:        $Source: /home/cvs/root/squirrel/src/inc/sql.h,v $
 * Revision:    $Revision: 1.2 $
 * Date:        $Date: 2003/07/26 06:28:16 $
 * Author:      $Author: quixadhal $
 * CVS_ID:      $Id: sql.h,v 1.2 2003/07/26 06:28:16 quixadhal Exp $
 * $Log: sql.h,v $
 * Revision 1.2  2003/07/26 06:28:16  quixadhal
 * Progress, now have a working bug system
 *
 * Revision 1.1  2003/07/26 03:57:39  quixadhal
 * Version 0
 *
 */

#ifndef _SQL_H
#define _SQL_H 1

#define TINY_BUF        32
#define SMALL_BUF       256
#define NORMAL_BUF      1024
#define LARGE_BUF       4096
#define GIANT_BUF       16384

/* Only define ONE database type!  Right now, that's easy since I only support one :) */
#define USE_POSTGRES 1
/* #define USE_MYSQL 0 */

/* These are the parameters used to connect to the database */
#define DBHOST  NULL
#define DBPORT  NULL
#define DBNAME  "socketmud"
#define DBOPTS  NULL
#define DBUSER  NULL
#define DBPASS  NULL

#ifdef USE_POSTGRES
#include "libpq-fe.h"

typedef struct s_sql_connect_data {
  char *dbhost;
  char *dbport;
  char *dbname;
  char *dbopts;
  char *dbuser;
  char *dbpasswd;
} sql_connect_data;

typedef struct s_sql_query_result {
  long int rows;
  long int cols;
  char ***data;
} sql_query_result;

#ifndef _SQL_C
extern PGconn *dbc;
extern sql_connect_data pgcdata;
#endif /* _SQL_C */

PGconn *sql_open(sql_connect_data *cdata);
void sql_close(void);
char *sql_error(ExecStatusType estat);
int sql_execute(const char *query, ... );
sql_query_result * sql_query(const char *query, ... );
void sql_free_result(sql_query_result *result);
char *sql_last_error(void);
const char *sql_escape(const char *str);
char *sql_version(void);
char *sql_status(void);
pid_t sql_backend(void);
int sql_table_exists(char *table);
int sql_field_exists(char *table, char *field);
int sql_truncate(char *table);
#endif /* USE_POSTGRES */

#endif /* _SQL_H */

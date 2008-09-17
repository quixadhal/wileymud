#ifndef __SQL_H__
#define __SQL_H__

/* This first set of function prototypes is the generic API
 * for anything else in the game to use.  The functions below
 * are specific versions (and support functions) for particular
 * RDMBS's, and shouldn't be used outside the sql file set.
 * 				-Quixadhal.
 */

typedef struct s_query_result {
  long int rows;
  long int cols;
  char ***data;
} query_result;

int init_sql( );
int close_sql( );
const char *escape_sql(const char *str);
const char *sanitize_query_sql( char *fmt, ... );

int execute_sql(const char *query, ... );
query_result * query_sql(const char *query, ... );
void free_result_sql(query_result *result);

#ifndef __SQL_C__
extern int db_connected;
#endif

#if defined(USE_MYSQL)

#include <mysql/mysql.h>

#ifndef __SQL_C__
extern MYSQL myconn;
#endif

int init_mysql( );
int close_mysql( );
const char *escape_mysql(const char *str);

#elif defined(USE_PGSQL)

#include <postgresql/libpq-fe.h>

#define DB_HOST		"localhost"
#define DB_PORT		5432
#define DB_USER		"quixadhal"
#define DB_PASSWD	"tardis69"
#define DB_NAME		"wiley"

#ifndef __SQL_C__
extern PGconn *dbc;
#endif

int init_pgsql();
int close_pgsql();
const char *escape_pgsql(const char *str);

int execute_pgsql(const char *query, va_list param );
query_result * query_pgsql(const char *query, va_list param );
void free_result_pgsql(query_result *result);

char *error_pgsql(ExecStatusType estat);
char *last_error_pgsql(void);
int table_exists_pgsql(char *table);
int field_exists_pgsql(char *table, char *field);
int truncate_pgsql(char *table);

char *version_pgsql(void);
char *status_pgsql(void);

#endif

#endif

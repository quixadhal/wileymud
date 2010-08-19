#ifndef __SQL_H__
#define __SQL_H__

int init_sql( void );
void close_sql( void );
int verify_sql( void );
char *version_sql( void );
int compare_dates_file_sql( const char *filename, const char *tablename, const char *fieldname );

#ifndef __SQL_C__
extern int db_connected;
extern int db_logging;

extern int db_log_info;
extern int db_log_errors;
extern int db_log_boot;
extern int db_log_auth;
extern int db_log_kills;
extern int db_log_resets;
extern int db_log_imc;
#endif

#endif

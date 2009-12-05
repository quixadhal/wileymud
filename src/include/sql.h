#ifndef __SQL_H__
#define __SQL_H__

int init_sql( );
void close_sql( );
char *version_sql( void );
int compare_dates_file_sql( const char *filename, const char *tablename, const char *fieldname );

#ifndef __SQL_C__
extern int db_connected;
extern int db_logging;
#endif

#endif

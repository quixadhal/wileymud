#ifndef __SQL_H__
#define __SQL_H__

int init_sql( );
void close_sql( );
char *version_sql( void );
int compare_dates_file_sql( char *filename, char *tablename, char *fieldname );

#ifndef __SQL_C__
extern int db_connected;
#endif

#endif

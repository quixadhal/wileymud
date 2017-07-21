#ifndef _SQL_H
#define _SQL_H

#include <sqlite3.h>

#define SQL_DB  "i3/wiley.db"

#ifndef _SQL_C
extern sqlite3 *db;

#endif

void sql_startup(void);
void sql_shutdown(void);
void allchan_sql( int is_emote, char *channel, char *speaker, char *mud, char *message );
int is_url( int is_emote, char *channel, char *speaker, char *mud, char *message );
int is_bot( int is_emote, char *channel, char *speaker, char *mud, char *message );

#endif

#ifndef _SQL_H
#define _SQL_H

#include <postgresql/libpq-fe.h>

#define SQL_DB  "i3/wiley.db"
#define URL_DELAY                       ((int)(PULSE_PER_SECOND * 1.5));

#define UNTINY_SQL              "../bin/untiny_sql.pl"
#define UNTINY                  "../bin/untiny"
#define PERL                    "/usr/bin/perl"

#ifdef _SQL_C
void setup_pinkfish_map_table(void);
void setup_hours_table(void);
void setup_channels_table(void);
void setup_speakers_table(void);
void setup_i3log_table(void);
void setup_urls_table(void);
void setup_logfile_table(void);
#else
extern PGconn *db_i3log;
#endif

//void sql_connect(const char *db_name);
//void sql_disconnect(const char *db_name);
//char *sql_version(const char *db_name);
void sql_connect(PGconn **db);
void sql_disconnect(PGconn **db);
char *sql_version(PGconn **db);
void sql_startup(void);
void sql_shutdown(void);
void allchan_sql( int is_emote, const char *channel, const char *speaker, const char *mud, const char *message );
int is_url( int is_emote, const char *channel, const char *speaker, const char *mud, const char *message );
int is_bot( int is_emote, const char *channel, const char *speaker, const char *mud, const char *message );
void add_url( const char *channel, const char *speaker, const char *mud, const char *url );
void bug_sql( const char *logtype, const char *filename, const char *function, int line,
              const char *area_file, int area_line, 
              const char *character, int character_room,
              const char *victim, int victim_room, 
              const char *message );
void addspeaker_sql( const char *speaker, const char *pinkfish );

#endif

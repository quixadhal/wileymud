#ifndef _SQL_H
#define _SQL_H

#include <sqlite3.h>

#define SQL_DB  "i3/wiley.db"
#define URL_DELAY                       ((int)(PULSE_PER_SECOND * 1.5));

#define UNTINY_SQL              "../bin/untiny_sql.pl"
#define UNTINY                  "../bin/untiny"
#define PERL                    "/usr/bin/perl"

#ifndef _SQL_C
extern sqlite3 *db;
extern int unprocessed_urls;
extern int tics_to_next_url_processing;

#else
void setup_i3log_table(void);
void setup_urls_table(void);
int process_url_callback(void *unused, int count, char **values, char **keys);
#endif

void sql_startup(void);
void sql_shutdown(void);
void allchan_sql( int is_emote, const char *channel, const char *speaker, const char *mud, const char *message );
int is_url( int is_emote, const char *channel, const char *speaker, const char *mud, const char *message );
int is_bot( int is_emote, const char *channel, const char *speaker, const char *mud, const char *message );
void add_url( const char *channel, const char *speaker, const char *mud, const char *url );
void process_urls( void );
void spawn_url_handler(void);
void bug_sql( const char *logtype, const char *filename, const char *function, int line,
              const char *area_file, int area_line, 
              const char *character, int character_room,
              const char *victim, int victim_room, 
              const char *message );
void addspeaker_sql( const char *speaker, const char *pinkfish );

#endif

#ifndef _SQL_H
#define _SQL_H

#include <postgresql/libpq-fe.h>

#define SQL_DB  "i3/wiley.db"
#define URL_DELAY                       ((int)(PULSE_PER_SECOND * 1.5));

#define UNTINY_SQL              "../bin/untiny_sql.pl"
#define UNTINY                  "../bin/untiny"
#define PERL                    "/usr/bin/perl"

struct sql_connection {
    const char *name;
    PGconn *dbc;
};

#ifdef _SQL_C
// Logging
void setup_logfile_table(void);
// I3
void setup_pinkfish_map_table(void);
void setup_hours_table(void);
void setup_channels_table(void);
void setup_speakers_table(void);
void setup_i3log_table(void);
void setup_urls_table(void);
// Messages
void setup_messages_table(void);
// Weather
void setup_weather_table(void);
// Bans
void setup_bans_table(void);
#else
extern struct sql_connection db_i3log;
extern struct sql_connection db_wileymud;
extern struct sql_connection db_logfile;
#endif

// Infrastructure
void sql_connect(struct sql_connection *db);
void sql_disconnect(struct sql_connection *db);
char *sql_version(struct sql_connection *db);
void sql_startup(void);
void sql_shutdown(void);

// Logging
void bug_sql( const char *logtype, const char *filename, const char *function, int line,
              const char *area_file, int area_line, 
              const char *character, int character_room,
              const char *victim, int victim_room, 
              const char *message );
// I3
void add_url( const char *channel, const char *speaker, const char *mud, const char *url );
int is_url( int is_emote, const char *channel, const char *speaker, const char *mud, const char *message );
int is_bot( int is_emote, const char *channel, const char *speaker, const char *mud, const char *message );
void allchan_sql( int is_emote, const char *channel, const char *speaker, const char *username, const char *mud, const char *message );
void addspeaker_sql( const char *speaker, const char *pinkfish );
void do_checkurl( struct char_data *ch, const char *argument, int cmd );
// Messages
char *update_message_from_file( const char *filename, int is_prompt );
// Weather
void load_weather(const char *filename);
void save_weather(const char *filename, struct time_info_data local_time,
                  struct weather_data local_weather);
// Bans

#endif

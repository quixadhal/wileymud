#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcre.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <time.h>
#include "global.h"
#include "bug.h"
#include "comm.h"
#include "version.h"
#include "crc32.h"
#include "interpreter.h"
#include "utils.h"
#include "db.h"
#include "weather.h"
#ifdef I3
#include "i3.h"
#endif
#define _SQL_C
#include "sql.h"

struct sql_connection db_i3log      = { "i3log", NULL };
struct sql_connection db_wileymud   = { "wileymud", NULL };
struct sql_connection db_logfile    = { "logfile", NULL };

// Infrastructure

void sql_connect(struct sql_connection *db) {
    char connect_string[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";

    if( db->dbc != NULL ) {
        // check existing status
        if (PQstatus(db->dbc) != CONNECTION_OK) {
            PQreset(db->dbc);
            if (PQstatus(db->dbc) != CONNECTION_OK) {
                log_fatal("Database connection lost: %s", PQerrorMessage(db->dbc));
                PQfinish(db->dbc);
                proper_exit(MUD_HALT);
            } else {
                log_info("Database connection restored.");
            }
        }
        return;
    } else {
        snprintf(connect_string, MAX_STRING_LENGTH, "dbname=%s user=%s password=%s",
                db->name, "wiley", "tardis69");
        db->dbc = PQconnectdb(connect_string);
        if (PQstatus(db->dbc) != CONNECTION_OK) {
            log_fatal("Cannot open database: %s", PQerrorMessage(db->dbc));
            PQfinish(db->dbc);
            proper_exit(MUD_HALT);
        }
    }
}

void sql_disconnect(struct sql_connection *db) {
    PQfinish(db->dbc);
    db->dbc = NULL;
}

char *sql_version(struct sql_connection *db) {
    static char pg_version[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    int version = 0;
    int major, minor, revision;

    sql_connect(db);
    version = PQlibVersion();
    major = version / 10000;
    minor = (version % 10000) / 100;
    revision = version % 100;
    snprintf(pg_version, MAX_INPUT_LENGTH, "%d.%02d.%02d", major, minor, revision);

    return pg_version;
}

void sql_startup(void) {
    char log_msg[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    
    // Logging
    sql_connect(&db_logfile);
    setup_logfile_table();
    log_boot("Opening SQL database for Logging.");
    log_boot("PostgreSQL Version: %s\n", sql_version(&db_logfile));

    // I3
    log_boot("Opening SQL database for I3.");
    sql_connect(&db_i3log);
    setup_pinkfish_map_table();
    setup_hours_table();
    setup_channels_table();
    setup_speakers_table();
    setup_i3log_table();
    setup_urls_table();
    snprintf(log_msg, MAX_STRING_LENGTH, "%%^GREEN%%^WileyMUD Version: %s (%s), PostgreSQL Version %s.%%^RESET%%^", VERSION_BUILD, VERSION_DATE, sql_version(&db_i3log));
    allchan_log(0,"wiley", "Cron", "Cron", "WileyMUD", log_msg);

    // Messages
    log_boot("Opening SQL database for WileyMUD.");
    sql_connect(&db_wileymud);
    setup_messages_table();

    // Weather
    setup_weather_table();

    // Bans
    //setup_bans_table();
}

void sql_shutdown(void) {
    char log_msg[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    log_boot("Shutting down SQL database.");
    snprintf(log_msg, MAX_STRING_LENGTH, "%%^RED%%^WileyMUD Version: %s (%s), PostgreSQL Version %s.%%^RESET%%^", VERSION_BUILD, VERSION_DATE, sql_version(&db_i3log));
    allchan_log(0,"wiley", "Cron", "Cron", "WileyMUD", log_msg);
    sql_disconnect(&db_i3log);
    sql_disconnect(&db_wileymud);
}

// Logging

void setup_logfile_table(void) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    char *sql = "CREATE TABLE IF NOT EXISTS logfile ( "
                "    created TIMESTAMP WITHOUT TIME ZONE NOT NULL DEFAULT (now() AT TIME ZONE 'UTC'), "
                "    local TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT now(), "
                "    logtype TEXT DEFAULT 'INFO', "
                "    filename TEXT, "
                "    function TEXT, "
                "    line INTEGER, "
                "    area_file TEXT, "
                "    area_line INTEGER, "
                "    character TEXT, "
                "    character_room INTEGER, "
                "    victim TEXT, "
                "    victim_room INTEGER, "
                "    message TEXT "
                "); ";
    char *sql2 = "CREATE INDEX IF NOT EXISTS ix_logfile_local ON logfile (local);";
    char *sql3 = "CREATE INDEX IF NOT EXISTS ix_logfile_logtype ON logfile (logtype);";
    char *sql4 = "CREATE OR REPLACE VIEW logtail AS "
                 "SELECT to_char(logfile.local, 'YYYY-MM-DD HH24:MI:SS'::text) AS local, "
                 "       logfile.logtype, "
                 "       logfile.message "
                 "FROM logfile "
                 "ORDER BY (to_char(logfile.local, 'YYYY-MM-DD HH24:MI:SS'::text)) "
                 "OFFSET (( SELECT count(*) AS count FROM logfile logfile_1)) - 20; ";

    res = PQexec(db_logfile.dbc, sql);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create log table: %s", PQerrorMessage(db_logfile.dbc));
        PQclear(res);
	proper_exit(MUD_HALT);
    }
    PQclear(res);

    res = PQexec(db_logfile.dbc, sql2);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create logfile local index: %s", PQerrorMessage(db_logfile.dbc));
        PQclear(res);
	proper_exit(MUD_HALT);
    }
    PQclear(res);

    res = PQexec(db_logfile.dbc, sql3);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create logfile logtype index: %s", PQerrorMessage(db_logfile.dbc));
        PQclear(res);
	proper_exit(MUD_HALT);
    }
    PQclear(res);

    res = PQexec(db_logfile.dbc, sql4);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create logtail view: %s", PQerrorMessage(db_logfile.dbc));
        PQclear(res);
	proper_exit(MUD_HALT);
    }
    PQclear(res);
}

void bug_sql( const char *logtype, const char *filename, const char *function, int line,
              const char *area_file, int area_line, 
              const char *character, int character_room,
              const char *victim, int victim_room, 
              const char *message ) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    const char *sql = "INSERT INTO logfile ( logtype, filename, function, line, "
                      "area_file, area_line, character, character_room, victim, "
                      "victim_room, message ) "
                      "VALUES ($1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11);";
    const char *param_val[11];
    int param_len[11];
    int param_bin[11] = {0,0,0,1,0,1,0,1,0,1,0};
    int param_line;
    int param_area_line;
    int param_char_room;
    int param_vic_room;

    if( !db_logfile.dbc ) {
        // If we are not connected, and we've TRIED to connect, punt.
        return;
    }

    param_line = htonl(line);
    param_area_line = htonl(area_line);
    param_char_room = htonl(character_room);
    param_vic_room = htonl(victim_room);

    param_val[0] = logtype;
    param_val[1] = filename;
    param_val[2] = function;
    param_val[3] = (char *)&param_line;
    param_val[4] = area_file;
    param_val[5] = (char *)&param_area_line;
    param_val[6] = character;
    param_val[7] = (char *)&param_char_room;
    param_val[8] = victim;
    param_val[9] = (char *)&param_vic_room;
    param_val[10] = message;

    param_len[0] = logtype ? strlen(logtype) : 0;
    param_len[1] = filename ? strlen(filename) : 0;
    param_len[2] = function ? strlen(function) : 0;
    param_len[3] = sizeof(param_line);
    param_len[4] = area_file ? strlen(area_file) : 0;
    param_len[5] = sizeof(param_area_line);
    param_len[6] = character ? strlen(character) : 0;
    param_len[7] = sizeof(param_char_room);
    param_len[8] = victim ? strlen(victim) : 0;
    param_len[9] = sizeof(param_vic_room);
    param_len[10] = message ? strlen(message) : 0;

    sql_connect(&db_logfile);
    res = PQexecParams(db_logfile.dbc, sql, 11, NULL, param_val, param_len, param_bin, 0);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        // In case of an error, we defined a special logging level that does NOT
        // attempt to log to SQL, so we can still emit errors if the database is
        // not available.
        log_sql("Cannot insert log message: %s", PQerrorMessage(db_logfile.dbc));
    }
    PQclear(res);
}

// I3

void setup_pinkfish_map_table(void) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    char *sql = "CREATE TABLE IF NOT EXISTS pinkfish_map ( "
                "    pinkfish TEXT PRIMARY KEY NOT NULL, "
                "    html TEXT NOT NULL "
                "); ";

    sql_connect(&db_i3log);
    res = PQexec(db_i3log.dbc, sql);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create pinkfish_map table: %s", PQerrorMessage(db_i3log.dbc));
        PQclear(res);
	proper_exit(MUD_HALT);
    }
    PQclear(res);
}

void setup_hours_table(void) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    char *sql = "CREATE TABLE IF NOT EXISTS hours ( "
                "    hour INTEGER PRIMARY KEY NOT NULL, "
                "    pinkfish TEXT NOT NULL REFERENCES pinkfish_map (pinkfish) "
                "); ";

    sql_connect(&db_i3log);
    res = PQexec(db_i3log.dbc, sql);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create hours table: %s", PQerrorMessage(db_i3log.dbc));
        PQclear(res);
	proper_exit(MUD_HALT);
    }
    PQclear(res);
}

void setup_channels_table(void) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    char *sql = "CREATE TABLE IF NOT EXISTS channels ( "
                "    channel TEXT PRIMARY KEY NOT NULL, "
                "    pinkfish TEXT NOT NULL REFERENCES pinkfish_map (pinkfish) "
                "); ";

    sql_connect(&db_i3log);
    res = PQexec(db_i3log.dbc, sql);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create channels table: %s", PQerrorMessage(db_i3log.dbc));
        PQclear(res);
	proper_exit(MUD_HALT);
    }
    PQclear(res);
}

void setup_speakers_table(void) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    char *sql = "CREATE TABLE IF NOT EXISTS speakers ( "
                "    speaker TEXT PRIMARY KEY NOT NULL, "
                "    pinkfish TEXT NOT NULL REFERENCES pinkfish_map (pinkfish) "
                "); ";

    sql_connect(&db_i3log);
    res = PQexec(db_i3log.dbc, sql);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create speakers table: %s", PQerrorMessage(db_i3log.dbc));
        PQclear(res);
	proper_exit(MUD_HALT);
    }
    PQclear(res);
}

void setup_i3log_table(void) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    char *sql = "CREATE TABLE IF NOT EXISTS i3log ( "
                "    created TIMESTAMP WITHOUT TIME ZONE NOT NULL DEFAULT (now() AT TIME ZONE 'UTC'), "
                "    local TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT now(), "
                "    is_emote BOOLEAN, "
                "    is_url BOOLEAN, "
                "    is_bot BOOLEAN, "
                "    channel TEXT NOT NULL, "
                "    speaker TEXT NOT NULL, "
                "    username TEXT NOT NULL, "
                "    mud TEXT NOT NULL, "
                "    message TEXT "
                "); ";
    char *sql2 = "CREATE INDEX IF NOT EXISTS ix_i3log_local ON i3log (local);";
    char *sql3 = "ALTER TABLE i3log DROP CONSTRAINT IF EXISTS ix_i3log_row;";
    char *sql4 = "ALTER TABLE i3log ADD CONSTRAINT ix_i3log_row UNIQUE " 
                 "    (created, local, is_emote, is_url, is_bot, "
                 "     channel, speaker, mud, message);";
    char *sql5 = "DROP VIEW IF EXISTS page_view;";
    char *sql6 = "CREATE VIEW page_view AS "
                 "SELECT i3log.local, "
                 "       i3log.is_emote, "
                 "       i3log.is_url, "
                 "       i3log.is_bot, "
                 "       i3log.channel, "
                 "       i3log.speaker, "
                 "       i3log.mud, "
                 "       i3log.message, "
                 "       to_char(i3log.local, 'YYYY-MM-DD')  the_date, "
                 "       to_char(i3log.local, 'HH24:MI:SS')  the_time, "
                 "       to_char(i3log.local, 'YYYY')        the_year, "
                 "       to_char(i3log.local, 'MM')          the_month, "
                 "       to_char(i3log.local, 'DD')          the_day, "
                 "       to_char(i3log.local, 'HH24')        the_hour, "
                 "       to_char(i3log.local, 'MI')          the_minute, "
                 "       to_char(i3log.local, 'SS')          the_second, "
                 "       date_part('hour', i3log.local)      int_hour, "
                 "       hours.pinkfish                      hour_color, "
                 "       channels.pinkfish                   channel_color, "
                 "       speakers.pinkfish                   speaker_color, "
                 "       pinkfish_map_hour.html              hour_html, "
                 "       pinkfish_map_channel.html           channel_html, "
                 "       pinkfish_map_speaker.html           speaker_html "
                 "  FROM i3log "
              "LEFT JOIN hours "
              "       ON (date_part('hour', i3log.local) = hours.hour) "
              "LEFT JOIN channels "
              "       ON (lower(i3log.channel) = channels.channel) "
              "LEFT JOIN speakers "
              "       ON (lower(i3log.username) = speakers.speaker) "
              "LEFT JOIN pinkfish_map pinkfish_map_hour "
              "       ON (hours.pinkfish = pinkfish_map_hour.pinkfish) "
              "LEFT JOIN pinkfish_map pinkfish_map_channel "
              "       ON (channels.pinkfish = pinkfish_map_channel.pinkfish) "
              "LEFT JOIN pinkfish_map pinkfish_map_speaker "
              "       ON (speakers.pinkfish = pinkfish_map_speaker.pinkfish); ";

    res = PQexec(db_i3log.dbc, sql);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create i3log table: %s", PQerrorMessage(db_i3log.dbc));
        PQclear(res);
	proper_exit(MUD_HALT);
    }
    PQclear(res);

    res = PQexec(db_i3log.dbc, sql2);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create i3log local index: %s", PQerrorMessage(db_i3log.dbc));
        PQclear(res);
	proper_exit(MUD_HALT);
    }
    PQclear(res);

    res = PQexec(db_i3log.dbc, sql3);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot drop i3log row constraint: %s", PQerrorMessage(db_i3log.dbc));
        PQclear(res);
	proper_exit(MUD_HALT);
    }
    PQclear(res);

    res = PQexec(db_i3log.dbc, sql4);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create i3log row constraint: %s", PQerrorMessage(db_i3log.dbc));
        PQclear(res);
	proper_exit(MUD_HALT);
    }
    PQclear(res);

    res = PQexec(db_i3log.dbc, sql5);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot drop page view: %s", PQerrorMessage(db_i3log.dbc));
        PQclear(res);
	proper_exit(MUD_HALT);
    }
    PQclear(res);

    res = PQexec(db_i3log.dbc, sql6);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create page view: %s", PQerrorMessage(db_i3log.dbc));
        PQclear(res);
	proper_exit(MUD_HALT);
    }
    PQclear(res);
}

void setup_urls_table(void) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    char *sql = "CREATE TABLE IF NOT EXISTS urls ( "
                "    created TIMESTAMP WITHOUT TIME ZONE NOT NULL DEFAULT (now() AT TIME ZONE 'UTC'), "
                "    processed BOOLEAN, "
                "    channel TEXT NOT NULL, "
                "    speaker TEXT NOT NULL, "
                "    mud TEXT NOT NULL, "
                "    url TEXT, "
                "    message TEXT, "
                "    checksum TEXT, "
                "    hits INTEGER DEFAULT 1 "
                "); ";
    char *sql2 = "DROP INDEX IF EXISTS ix_urls_checksum;";
    char *sql3 = "CREATE UNIQUE INDEX ix_urls_checksum ON urls (checksum);";

    sql_connect(&db_i3log);
    res = PQexec(db_i3log.dbc, sql);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create urls table: %s", PQerrorMessage(db_i3log.dbc));
        PQclear(res);
	proper_exit(MUD_HALT);
    }
    PQclear(res);

    res = PQexec(db_i3log.dbc, sql2);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot drop urls checksum index: %s", PQerrorMessage(db_i3log.dbc));
        PQclear(res);
	proper_exit(MUD_HALT);
    }
    PQclear(res);

    res = PQexec(db_i3log.dbc, sql3);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create urls checksum index: %s", PQerrorMessage(db_i3log.dbc));
        PQclear(res);
	proper_exit(MUD_HALT);
    }
    PQclear(res);
}

void add_url( const char *channel, const char *speaker, const char *mud, const char *url ) {
    u_int32_t crc;
    char checksum[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    PGresult *res = NULL;
    ExecStatusType st = 0;
    const char *sql = "INSERT INTO urls ( url, channel, speaker, mud, checksum ) "
                      "VALUES ($1,$2,$3,$4,$5) "
                      "ON CONFLICT (checksum) "
                      "DO UPDATE SET hits = urls.hits + 1;";
    const char *param_val[5];
    int param_len[5];
    int param_bin[5] = {0,0,0,0,0};

    crc = crc32(url, strlen(url));
    snprintf(checksum, MAX_INPUT_LENGTH, "%08x", crc);

    param_val[0] = url;
    param_val[1] = channel;
    param_val[2] = speaker;
    param_val[3] = mud;
    param_val[4] = checksum;

    param_len[0] = url ? strlen(url) : 0;
    param_len[1] = channel ? strlen(channel) : 0;
    param_len[2] = speaker ? strlen(speaker) : 0;
    param_len[3] = mud ? strlen(mud) : 0;
    param_len[4] = strlen(checksum);

    sql_connect(&db_i3log);
    res = PQexecParams(db_i3log.dbc, sql, 5, NULL, param_val, param_len, param_bin, 0);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot insert url: %s", PQerrorMessage(db_i3log.dbc));
        //PQclear(res);
	//proper_exit(MUD_HALT);
    }
    PQclear(res);

    log_info("add_url done, added %s: %s", checksum, url);
}

int is_url( int is_emote, const char *channel, const char *speaker, const char *mud, const char *message ) {
    const char          *regexp_pattern     = "(https?\\:\\/\\/[\\w.-]+(?:\\.[\\w\\.-]+)+(?:\\:[\\d]+)?[\\w\\-\\.\\~\\:\\/\\?\\#[\\]\\@\\!\\$\\&\\'\\(\\)\\*\\+\\,\\;\\=\\%]+)";
    //"(https?\\:\\/\\/[\\w.-]+(?:\\.[\\w\\.-]+)+[\\w\\-\\._~:/?#[\\]@!\\$&'\\(\\)\\*\\+,;=.]+)";
    int                  regexp_opts        = PCRE_CASELESS|PCRE_MULTILINE;
    static pcre         *regexp_compiled    = NULL;
    static pcre_extra   *regexp_studied     = NULL;
    const char          *regexp_error       = NULL;
    int                  regexp_err_offset  = 0;
    const char          *regexp_match       = NULL;
    static int           regexp_broken      = 0;
    int                  regexp_rv;
    int                  regexp_matchpos[30];
    char                *url_stripped       = NULL;

    if(regexp_broken) {
        // URL processing is screwed up, so trying again won't help.
        return 0;
    }
    if(!strcasecmp(channel, "url")) {
        // Anything on the URL channel is never treated as a url.  Recursion is recursion.
        return 0;
    }
    if(!strcasecmp(speaker, "cron") && !strcasecmp(mud, "wileymud")) {
        // Also ignore automated responses from this mud
        return 0;
    }

    if(!regexp_compiled) { // Haven't compiled yet
        regexp_compiled = pcre_compile(regexp_pattern, regexp_opts, &regexp_error, &regexp_err_offset, NULL);
        log_info("SQL regexp: /%s/", regexp_pattern);
        if(!regexp_compiled) {
            log_error("regexp failed to compile");
            regexp_broken = 1;
            return 0;
        }
        regexp_studied = pcre_study(regexp_compiled, 0, &regexp_error);
        if(regexp_error != NULL) {
            log_error("regexp study failed");
            regexp_broken = 1;
            return 0;
        }
    }

    url_stripped = i3_strip_colors(message);
    regexp_rv = pcre_exec(regexp_compiled, regexp_studied,
                          url_stripped, strlen(url_stripped),
                          0, 0, // START POS, OPTIONS
                          regexp_matchpos, 30);

    if(regexp_rv <= 0) {
        // Either an error, or no URL match found
        return 0;
    }

    // This version will become active once the external script is redesigned to use
    // the database, and once the main loop is taught to read from the database and
    // not a file.  The new script will handle all unprocessed urls at once.
    while( regexp_rv > 0 ) {
        if (pcre_get_substring(url_stripped, regexp_matchpos, regexp_rv, 0, &regexp_match) >= 0) {
            log_info("Found URL ( %s ) in channel ( %s )", regexp_match, channel);
            add_url(channel, speaker, mud, regexp_match);
        }
        regexp_rv = pcre_exec(regexp_compiled, regexp_studied,
                              url_stripped, strlen(url_stripped),
                              regexp_matchpos[1], 0,
                              regexp_matchpos, 30);
    }

    // For now, this seems OK, but there's a nagging suspicion I have that if
    // a high volume of URL's came in, in separate messages, one right after the
    // other, forking a "do all the urls not done" might cause errors.
    //
    // In most cases I can think of, even if the same records somehow got written
    // to multiple times, the results SHOULD always be the same, so it shouldn't
    // matter, other than some extra work done.
    //
    // But keep this in mind if a problem does show up, we may need to move this
    // higher up, so it only runs once per N tics.

    //spawn_url_handler();
    return 1;
}

int is_bot( int is_emote, const char *channel, const char *speaker, const char *mud, const char *message ) {
    if(!strcasecmp(speaker, "cron") && !strcasecmp(mud, "wileymud")) return 1;
    if(!strcasecmp(speaker, "urlbot") && !strcasecmp(mud, "wileymud")) return 1;
    if(!strcasecmp(speaker, "system") && !strcasecmp(mud, "bloodlines")) return 1;
    if(!strcasecmp(speaker, "gribbles") && !strcasecmp(mud, "rock the halo")) return 1;
    if(!strcasecmp(speaker, "timbot") && !strcasecmp(mud, "timmud")) return 1;

    if(!strcasecmp(channel, "inews") && !strcasecmp(mud, "rock the halo")) return 1;
    if(!strcasecmp(channel, "mudnews") && !strcasecmp(mud, "rock the halo")) return 1;
    if(!strcasecmp(channel, "sport") && !strcasecmp(mud, "rock the halo")) return 1;
    if(!strcasecmp(channel, "admin") && !strcasecmp(mud, "rock the halo")) return 1;

    if(!strcasecmp(channel, "inews") && !strcasecmp(mud, "rth")) return 1;
    if(!strcasecmp(channel, "mudnews") && !strcasecmp(mud, "rth")) return 1;
    if(!strcasecmp(channel, "sport") && !strcasecmp(mud, "rth")) return 1;
    if(!strcasecmp(channel, "admin") && !strcasecmp(mud, "rth")) return 1;

    if(!strcasecmp(channel, "admin") && !strcasecmp(mud, "bloodlines")) return 1;

    return 0;
}

// This gets passed "visname", rather than "originator_username"
// The result is that the log messages are logged by the fake name people can use
// not the username that works for tells.
//
// We could add a column and ALSO store username, if we wanted...
//
void allchan_sql( int is_emote, const char *channel, const char *speaker, const char *username, const char *mud, const char *message ) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    const char *sql = "INSERT INTO i3log ( channel, speaker, username, mud, message, is_emote, is_url, is_bot ) "
                      "VALUES ($1,$2,$3,$4,$5,$6,$7,$8);";
    const char *param_val[8];
    int param_len[8];
    //int param_bin[7] = {0,0,0,0,1,1,1}; // Are booleans considered binary?
    int param_bin[8] = {0,0,0,0,0,0,0,0};
    char param_emote[2];
    char param_url[2];
    char param_bot[2];

    // Note that is_url() calls add_url() 0 or more times, as urls are found in the message.
    int url = is_url(is_emote, channel, speaker, mud, message);
    int bot = is_bot(is_emote, channel, speaker, mud, message);

    //is_emote = htonl(is_emote);
    //url = htonl(url);
    //bot = htonl(bot);
    sprintf(param_emote, "%s", (is_emote ? "t" : "f"));
    sprintf(param_url, "%s", (url ? "t" : "f"));
    sprintf(param_bot, "%s", (bot ? "t" : "f"));

    param_val[0] = channel;
    param_val[1] = speaker;
    param_val[2] = username;
    param_val[3] = mud;
    param_val[4] = message;
    //param_val[4] = (char *)&is_emote;
    //param_val[5] = (char *)&url;
    //param_val[6] = (char *)&bot;
    param_val[5] = param_emote;
    param_val[6] = param_url;
    param_val[7] = param_bot;

    param_len[0] = channel ? strlen(channel) : 0;
    param_len[1] = speaker ? strlen(speaker) : 0;
    param_len[2] = username ? strlen(username) : 0;
    param_len[3] = mud ? strlen(mud) : 0;
    param_len[4] = message ? strlen(message) : 0;
    //param_len[4] = sizeof(is_emote);
    //param_len[5] = sizeof(url);
    //param_len[6] = sizeof(bot);
    param_len[5] = strlen(param_emote);
    param_len[6] = strlen(param_url);
    param_len[7] = strlen(param_bot);

    sql_connect(&db_i3log);
    res = PQexecParams(db_i3log.dbc, sql, 8, NULL, param_val, param_len, param_bin, 0);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot insert message: %s", PQerrorMessage(db_i3log.dbc));
        //PQclear(res);
	//proper_exit(MUD_HALT);
    }
    PQclear(res);
}

void addspeaker_sql( const char *speaker, const char *pinkfish ) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    const char *sql = "INSERT INTO speakers (speaker, pinkfish) "
                      "VALUES ($1,$2) "
                      "ON CONFLICT (speaker) "
                      "DO NOTHING; ";
    const char *param_val[2];
    int param_len[2];
    int param_bin[2] = {0,0};

    param_val[0] = speaker;
    param_val[1] = pinkfish;

    param_len[0] = speaker ? strlen(speaker) : 0;
    param_len[1] = pinkfish ? strlen(pinkfish) : 0;

    sql_connect(&db_i3log);
    res = PQexecParams(db_i3log.dbc, sql, 2, NULL, param_val, param_len, param_bin, 0);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot add speaker: %s", PQerrorMessage(db_i3log.dbc));
        //PQclear(res);
	//proper_exit(MUD_HALT);
    }
    PQclear(res);
}

void do_checkurl( struct char_data *ch, const char *argument, int cmd )
{
    static char                             buf[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    static char                             tmp[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    PGresult *res = NULL;
    ExecStatusType st = 0;
    const char *sql =   "SELECT to_char(created AT TIME ZONE 'US/Pacific', "
                            "'YYYY-MM-DD HH24:MI:SS ') "
                            "||(select abbrev from pg_timezone_names where name = 'US/Pacific') "
                        "AS created, speaker, hits "
                        "FROM urls "
                        "WHERE url = $1;";
    const char *param_val[1];
    int param_len[1];
    int param_bin[1] = {0};
    int rows = 0;
    int columns = 0;
    int *col_len = NULL;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    only_argument(argument, tmp);
    if (*tmp) {
        param_val[0] = tmp;
        param_len[0] = *tmp ? strlen(tmp) : 0;

        sql_connect(&db_i3log);
        res = PQexecParams(db_i3log.dbc, sql, 1, NULL, param_val, param_len, param_bin, 0);
        st = PQresultStatus(res);
        if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
            log_fatal("Cannot get url info: %s", PQerrorMessage(db_i3log.dbc));
            //PQclear(res);
            //proper_exit(MUD_HALT);
        }
        rows = PQntuples(res);
        log_info("checkurl %s, got %d rows", tmp, rows);
        if( rows > 0 ) {
            columns = PQnfields(res);
            //log_info("checkurl %s, got %d columns", tmp, columns);
            if( columns > 0 ) {
                col_len = (int *)calloc(columns, sizeof(int));

                for( int j = 0; j < columns; j++) {
                    char *col_name = PQfname(res, j);
                    col_len[j] = MAX(strlen(col_name), col_len[j]);
                    for(int i = 0; i < rows; i++) {
                        char *val = PQgetvalue(res, i, j);
                        col_len[j] = MAX(strlen(val), col_len[j]);
                        col_len[j] = MAX(30, col_len[j]);
                    }
                }

                for( int j = 0; j < columns; j++) {
                    scprintf(buf, MAX_STRING_LENGTH, "%*.*s ",
                            col_len[j], col_len[j], PQfname(res, j));
                }
                scprintf(buf, MAX_STRING_LENGTH, "\r\n");

                for( int j = 0; j < columns; j++) {
                    scprintf(buf, MAX_STRING_LENGTH, "%*.*s ",
                            col_len[j], col_len[j], "------------------------------");
                }
                scprintf(buf, MAX_STRING_LENGTH, "\r\n");

                for( int i = 0; i < rows; i++ ) {
                    for( int j = 0; j < columns; j++) {
                        int nil = PQgetisnull(res, i, j);
                        if(!nil) {
                            scprintf(buf, MAX_STRING_LENGTH, "%*.*s ",
                                    col_len[j], col_len[j], PQgetvalue(res, i, j));
                        } else {
                            scprintf(buf, MAX_STRING_LENGTH, "%*.*s ",
                                    col_len[j], col_len[j], "");
                        }
                    }
                    scprintf(buf, MAX_STRING_LENGTH, "\r\n");
                }

                if(col_len) free(col_len);
            }
            log_info("checkurl %s\n%s\n", tmp, buf);
            cprintf(ch, "%s\r\n", buf);
        }
        PQclear(res);
    } else {
        cprintf(ch, "URL is not known, yet!\r\n");
    }
}

// Messages

void setup_messages_table(void) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    char *sql = "CREATE TABLE IF NOT EXISTS messages ( "
                "    created TIMESTAMP WITHOUT TIME ZONE NOT NULL DEFAULT (now() AT TIME ZONE 'UTC'), "
                "    updated TIMESTAMP WITHOUT TIME ZONE NOT NULL DEFAULT (now() AT TIME ZONE 'UTC'), "
                "    filename TEXT PRIMARY KEY NOT NULL, "
                "    message TEXT "
                "); ";
    char *sql2 = "CREATE INDEX IF NOT EXISTS ix_messages_updated ON messages (updated);";
    char *sql3 = "CREATE INDEX IF NOT EXISTS ix_messages_filename ON messages (filename);";

    sql_connect(&db_wileymud);
    res = PQexec(db_wileymud.dbc, sql);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create messages table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
	proper_exit(MUD_HALT);
    }
    PQclear(res);

    res = PQexec(db_wileymud.dbc, sql2);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create messages updated index: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
	proper_exit(MUD_HALT);
    }
    PQclear(res);

    res = PQexec(db_wileymud.dbc, sql3);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create messages filename index: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
	proper_exit(MUD_HALT);
    }
    PQclear(res);
}

char *update_message_from_file( const char *filename, int is_prompt ) {
    static char                             tmp[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    time_t file_timestamp = -1;
    time_t sql_timestamp = -1;

    PGresult *res = NULL;
    ExecStatusType st = 0;
    const char *sql =   "SELECT extract(EPOCH from updated) AS the_time, "
                        "       message "
                        "FROM   messages "
                        "WHERE  filename = $1;";
    const char *sql2 =  "INSERT INTO messages(filename, message) "
                        "VALUES ($1, $2) "
                        "ON CONFLICT (filename) "
                        "DO UPDATE SET "
                        "    updated = now() AT TIME ZONE 'UTC', "
                        "    message = $2;";
    const char *param_val[2];
    int param_len[2];
    int param_bin[2] = {0,0};
    int rows = 0;
    int columns = 0;

    file_timestamp = file_date(filename);

    param_val[0] = filename;
    param_len[0] = *filename ? strlen(filename) : 0;

    sql_connect(&db_wileymud);
    res = PQexecParams(db_wileymud.dbc, sql, 1, NULL, param_val, param_len, param_bin, 0);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_error("Message does not exist in database: %s", PQerrorMessage(db_wileymud.dbc));
        //PQclear(res);
        //proper_exit(MUD_HALT);
    } else {
        rows = PQntuples(res);
        if( rows > 0 ) {
            columns = PQnfields(res);
            if( columns > 0 ) {
                // The PQgetvalue() API will return the integer we expect as a
                // NULL terminated string, so we can convert it with atoi().
                sql_timestamp = (time_t)atoi(PQgetvalue(res,0,0));
                strlcpy(tmp, PQgetvalue(res,0,1), MAX_STRING_LENGTH);
            }
        }
    }
    PQclear(res);

    // OK, at this point, we have either -1 or a valid timestamp in both
    // variables.  If the SQL entry didn't exist but the file did, the
    // file date > -1, so upload the file contents... likewise if the file
    // is newer than the SQL.
    //
    // If the file didn't exist, but the SQL did, that's fine... file <= sql.
    //
    // If the file didn't exist AND the SQL also didn't exist, we have a problem.
    if( file_timestamp == -1 && sql_timestamp == -1 ) {
        log_fatal("No data available for %s!", filename);
        proper_exit(MUD_HALT);
    }

    if( file_timestamp > sql_timestamp ) {
        // OK, the file is actually newer than the SQL (or the SQL didn't exist)
        // So, we want to upload the newer version.
        param_val[0] = filename;
        param_len[0] = *filename ? strlen(filename) : 0;

        log_boot("  Message %s has been updated on disk, updating SQL to match!", filename);
        if(is_prompt) {
            file_to_prompt(filename, tmp);
        } else {
            file_to_string(filename, tmp);
        }

        param_val[1] = tmp;
        param_len[1] = *tmp ? strlen(tmp) : 0;

        res = PQexecParams(db_wileymud.dbc, sql2, 2, NULL, param_val, param_len, param_bin, 0);
        st = PQresultStatus(res);
        if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
            log_fatal("Cannot insert message: %s", PQerrorMessage(db_wileymud.dbc));
            //PQclear(res);
            //proper_exit(MUD_HALT);
        }
        PQclear(res);
    } else {
        log_boot("  Using message %s from SQL database.", filename);
    }

    // And at THIS point, we either have the message content from the SQL query we used
    // earlier, or the version read in from the disk file and then uploaded.  Either way,
    // we return that string for our caller to use (or not).

    return tmp;
}

// Weather

void setup_weather_table(void) {
    //time_t beginning_of_time = 650336715;	/* Fri Aug 10 21:05:15 1990 */
    PGresult *res = NULL;
    ExecStatusType st = 0;
    char *sql = "CREATE TABLE IF NOT EXISTS weather ( "
                "    updated TIMESTAMP WITHOUT TIME ZONE NOT NULL DEFAULT (now() AT TIME ZONE 'UTC'), "
                "    hours INTEGER, "
                "    day INTEGER, "
                "    month INTEGER, "
                "    year INTEGER, "
                "    pressure INTEGER, "
                "    change INTEGER, "
                "    sky INTEGER, "
                "    sunlight INTEGER, "
                "    wind_speed INTEGER, "
                "    wind_direction INTEGER, "
                "    moon INTEGER "
                "); ";
    char *sql2 = "SELECT count(*) FROM weather;";
    char *sql3 = "INSERT INTO weather (updated) VALUES (to_timestamp(650336715) AT TIME ZONE 'UTC');";
    int rows = 0;
    int columns = 0;
    int count = 0;

    sql_connect(&db_wileymud);
    res = PQexec(db_wileymud.dbc, sql);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create weather table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
	proper_exit(MUD_HALT);
    }
    PQclear(res);

    res = PQexec(db_wileymud.dbc, sql2);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot get row count of weather table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
	proper_exit(MUD_HALT);
    }
    rows = PQntuples(res);
    if(rows > 0) {
        columns = PQnfields(res);
        if( columns > 0 ) {
            count = atoi(PQgetvalue(res,0,0));
        }
    }
    PQclear(res);

    if(count < 1) {
        res = PQexec(db_wileymud.dbc, sql3);
        st = PQresultStatus(res);
        if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
            log_fatal("Cannot insert placeholder row: %s", PQerrorMessage(db_wileymud.dbc));
            PQclear(res);
            proper_exit(MUD_HALT);
        }
        PQclear(res);
    } else if(count > 1) {
        log_error("There are %d rows in the weather table, instead of just ONE!", count);
    }
}

void load_weather(const char *filename) {
    static char                             tmp[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    time_t file_timestamp = -1;
    time_t sql_timestamp = -1;
    PGresult *res = NULL;
    ExecStatusType st = 0;
    const char *sql =   "SELECT extract(EPOCH from updated) AS the_time, "
                        "       hours, "
                        "       day, "
                        "       month, "
                        "       year, "
                        "       pressure, "
                        "       change, "
                        "       sky, "
                        "       sunlight, "
                        "       wind_speed, "
                        "       wind_direction, "
                        "       moon "
                        "FROM   weather "
                        "LIMIT  1;";
    const char *param_val[1];
    int param_len[1];
    int param_bin[1] = {0};
    int rows = 0;
    int columns = 0;
    struct time_info_data local_time;
    struct weather_data local_weather;
    time_t beginning_of_time = 650336715;	/* Fri Aug 10 21:05:15 1990 */

    file_timestamp = file_date(filename);

    sql_connect(&db_wileymud);
    res = PQexecParams(db_wileymud.dbc, sql, 0, NULL, param_val, param_len, param_bin, 0);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_error("Weather info does not exist in database: %s", PQerrorMessage(db_wileymud.dbc));
        //PQclear(res);
        //proper_exit(MUD_HALT);
    } else {
        rows = PQntuples(res);
        if( rows > 0 ) {
            columns = PQnfields(res);
            if( columns > 0 ) {
                // The PQgetvalue() API will return the integer we expect as a
                // NULL terminated string, so we can convert it with atoi().
                sql_timestamp = (time_t)atoi(PQgetvalue(res,0,0));
                local_time.hours = atoi(PQgetvalue(res,0,1));
                local_time.day = atoi(PQgetvalue(res,0,2));
                local_time.month = atoi(PQgetvalue(res,0,3));
                local_time.year = atoi(PQgetvalue(res,0,4));
                local_weather.pressure = atoi(PQgetvalue(res,0,5));
                local_weather.change = atoi(PQgetvalue(res,0,6));
                local_weather.sky = atoi(PQgetvalue(res,0,7));
                local_weather.sunlight = atoi(PQgetvalue(res,0,8));
                local_weather.wind_speed = atoi(PQgetvalue(res,0,9));
                local_weather.wind_direction = atoi(PQgetvalue(res,0,10));
                local_weather.moon = atoi(PQgetvalue(res,0,11));
            }
        }
    }
    PQclear(res);

    if( file_timestamp == -1 && sql_timestamp == -1 ) {
        log_fatal("No weather data available from %s!", filename);
        proper_exit(MUD_HALT);
    }

    if( file_timestamp > sql_timestamp ) {
        FILE *fp = NULL;
        log_boot("  Weather has been updated in %s, updating SQL to match!", filename);
        if (!(fp = fopen(filename, "r"))) {
            log_error("  Weather file %s cannot be opened, using defaults.", filename);
            local_time = mud_time_passed(time(0), beginning_of_time);
            // reset_weather();
            local_weather.pressure = 960;
            local_weather.change = 0;
            local_weather.sky = SKY_CLOUDLESS;
            local_weather.wind_speed = 0;
            local_weather.wind_direction = WIND_DEAD;
        } else {
            time_t local_current_time;

            fgets(tmp, MAX_INPUT_LENGTH, fp);
            fscanf(fp, "%ld\n", &local_current_time);
            fscanf(fp, "%d\n", &local_time.hours);
            fscanf(fp, "%d\n", &local_time.day);
            fscanf(fp, "%d\n", &local_time.month);
            fscanf(fp, "%d\n", &local_time.year);
            fgets(tmp, MAX_INPUT_LENGTH, fp);
            fscanf(fp, "%d\n", &local_weather.pressure);
            fscanf(fp, "%d\n", &local_weather.change);
            fscanf(fp, "%d\n", &local_weather.sky);
            fscanf(fp, "%d\n", &local_weather.sunlight);
            fscanf(fp, "%d\n", &local_weather.wind_speed);
            fscanf(fp, "%d\n", &local_weather.wind_direction);
            fscanf(fp, "%d\n", &local_weather.moon);
            FCLOSE(fp);
        }
        save_weather(NULL, local_time, local_weather);
    } else {
        log_boot("  Using weather data from SQL database.");
    }

    time_info = local_time;
    weather_info = local_weather;
    weather_and_time(1);
    log_info("   Current Gametime: %dH %dD %dM %dY.",
	     time_info.hours, time_info.day, time_info.month, time_info.year);
}

void save_weather(const char *filename, struct time_info_data local_time,
                  struct weather_data local_weather) {
    static char                             tmp[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    PGresult *res = NULL;
    ExecStatusType st = 0;
    const char *sql =   "UPDATE weather SET "
                        "    updated = now() AT TIME ZONE 'UTC', "
                        "    hours = $1, "
                        "    day = $2, "
                        "    month = $3, "
                        "    year = $4, "
                        "    pressure = $5, "
                        "    change = $6, "
                        "    sky = $7, "
                        "    sunlight = $8, "
                        "    wind_speed = $9, "
                        "    wind_direction = $10, "
                        "    moon = $11;";
    char *param_val[11];
    int param_len[11];
    int param_bin[11] = {0,0,0,0,0,0,0,0,0,0,0};
    FILE *fp = NULL;

    if(filename && *filename) {
        if (!(fp = fopen(filename, "w"))) {
            log_error("Cannot save weather data to %s.", filename);
        } else {
            log_info("Saving weather data to %s.", filename);
            fprintf(fp, "# Time -last,hours,day,month,year\n");
            fprintf(fp, "%ld\n", time(0));
            fprintf(fp, "%d\n", local_time.hours);
            fprintf(fp, "%d\n", local_time.day);
            fprintf(fp, "%d\n", local_time.month);
            fprintf(fp, "%d\n", local_time.year);
            fprintf(fp, "# Weather -pressure,change,sky,sunlight,windspeed,direction\n");
            fprintf(fp, "%d\n", local_weather.pressure);
            fprintf(fp, "%d\n", local_weather.change);
            fprintf(fp, "%d\n", local_weather.sky);
            fprintf(fp, "%d\n", local_weather.sunlight);
            fprintf(fp, "%d\n", local_weather.wind_speed);
            fprintf(fp, "%d\n", local_weather.wind_direction);
            fprintf(fp, "%d\n", local_weather.moon);
            FCLOSE(fp);
        }
    }

    snprintf(tmp, MAX_INPUT_LENGTH, "%d", local_time.hours);
    param_val[0] = strdup(tmp);
    param_len[0] = *tmp ? strlen(tmp) : 0;
    snprintf(tmp, MAX_INPUT_LENGTH, "%d", local_time.day);
    param_val[1] = strdup(tmp);
    param_len[1] = *tmp ? strlen(tmp) : 0;
    snprintf(tmp, MAX_INPUT_LENGTH, "%d", local_time.month);
    param_val[2] = strdup(tmp);
    param_len[2] = *tmp ? strlen(tmp) : 0;
    snprintf(tmp, MAX_INPUT_LENGTH, "%d", local_time.year);
    param_val[3] = strdup(tmp);
    param_len[3] = *tmp ? strlen(tmp) : 0;

    snprintf(tmp, MAX_INPUT_LENGTH, "%d", local_weather.pressure);
    param_val[4] = strdup(tmp);
    param_len[4] = *tmp ? strlen(tmp) : 0;
    snprintf(tmp, MAX_INPUT_LENGTH, "%d", local_weather.change);
    param_val[5] = strdup(tmp);
    param_len[5] = *tmp ? strlen(tmp) : 0;
    snprintf(tmp, MAX_INPUT_LENGTH, "%d", local_weather.sky);
    param_val[6] = strdup(tmp);
    param_len[6] = *tmp ? strlen(tmp) : 0;
    snprintf(tmp, MAX_INPUT_LENGTH, "%d", local_weather.sunlight);
    param_val[7] = strdup(tmp);
    param_len[7] = *tmp ? strlen(tmp) : 0;
    snprintf(tmp, MAX_INPUT_LENGTH, "%d", local_weather.wind_speed);
    param_val[8] = strdup(tmp);
    param_len[8] = *tmp ? strlen(tmp) : 0;
    snprintf(tmp, MAX_INPUT_LENGTH, "%d", local_weather.wind_direction);
    param_val[9] = strdup(tmp);
    param_len[9] = *tmp ? strlen(tmp) : 0;
    snprintf(tmp, MAX_INPUT_LENGTH, "%d", local_weather.moon);
    param_val[10] = strdup(tmp);
    param_len[10] = *tmp ? strlen(tmp) : 0;

    res = PQexecParams(db_wileymud.dbc, sql, 11, NULL, (const char **)param_val, param_len, param_bin, 0);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot update weather: %s", PQerrorMessage(db_wileymud.dbc));
        //PQclear(res);
        //proper_exit(MUD_HALT);
    }
    PQclear(res);
    free(param_val[0]);
    free(param_val[1]);
    free(param_val[2]);
    free(param_val[3]);
    free(param_val[4]);
    free(param_val[5]);
    free(param_val[6]);
    free(param_val[7]);
    free(param_val[8]);
    free(param_val[9]);
    free(param_val[10]);
}

// Bans

void setup_bans_table(void) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    char *sql = "CREATE TABLE IF NOT EXISTS bans ( "
                "    updated TIMESTAMP WITHOUT TIME ZONE NOT NULL DEFAULT (now() AT TIME ZONE 'UTC'), "
                "    expires TIMESTAMP WITHOUT TIME ZONE, "
                "    name TEXT, "
                "    ip TEXT, "
                "    banned_by TEXT, "
                "    reason TEXT "
                "); ";
    char *sql2 = "CREATE INDEX IF NOT EXISTS ix_bans_name ON bans (name);";
    char *sql3 = "CREATE INDEX IF NOT EXISTS ix_bans_ip ON bans (ip);";

    sql_connect(&db_wileymud);
    res = PQexec(db_wileymud.dbc, sql);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create bans table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
	proper_exit(MUD_HALT);
    }
    PQclear(res);

    res = PQexec(db_wileymud.dbc, sql2);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create bans name index: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
	proper_exit(MUD_HALT);
    }
    PQclear(res);

    res = PQexec(db_wileymud.dbc, sql3);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create bans ip index: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
	proper_exit(MUD_HALT);
    }
    PQclear(res);
}


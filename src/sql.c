#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcre.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include "global.h"
#include "bug.h"
#include "comm.h"
#include "version.h"
#include "crc32.h"
#include "utils.h"
#ifdef I3
#include "i3.h"
#endif
#define _SQL_C
#include "sql.h"

PGconn *db = NULL;

void sql_connect(void) {
    if( db != NULL ) {
        // check existing status
        if (PQstatus(db) != CONNECTION_OK) {
            PQreset(db);
            if (PQstatus(db) != CONNECTION_OK) {
                log_fatal("Database connection lost: %s", PQerrorMessage(db));
                PQfinish(db);
                proper_exit(MUD_HALT);
            } else {
                log_info("Database connection restored.");
            }
        }
        return;
    } else {
        db = PQconnectdb("dbname=i3log user=wiley password=tardis69");
        if (PQstatus(db) != CONNECTION_OK) {
            log_fatal("Cannot open database: %s", PQerrorMessage(db));
            PQfinish(db);
            proper_exit(MUD_HALT);
        }
    }
}

void sql_disconnect(void) {
    PQfinish(db);
    db = NULL;
}

char *sql_version(void) {
    static char pg_version[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    int version = 0;
    int major, minor, revision;

    sql_connect();
    version = PQlibVersion();
    major = version / 10000;
    minor = (version % 10000) / 100;
    revision = version % 100;
    snprintf(pg_version, MAX_INPUT_LENGTH, "%d.%02d.%02d", major, minor, revision);

    return pg_version;
}

void sql_startup(void) {
    char log_msg[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    
    log_boot("Opening SQL database.");
    sql_connect();
    log_boot("PostgreSQL Version: %s\n", sql_version());
    setup_pinkfish_map_table();
    setup_hours_table();
    setup_channels_table();
    setup_speakers_table();
    setup_i3log_table();
    setup_urls_table();
    setup_log_table();
    snprintf(log_msg, MAX_STRING_LENGTH, "%%^GREEN%%^WileyMUD Version: %s (%s), PostgreSQL Version %s.%%^RESET%%^", VERSION_BUILD, VERSION_DATE, sql_version());
    allchan_log(0,"wiley", "Cron", "WileyMUD", log_msg);
}

void sql_shutdown(void) {
    char log_msg[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    log_boot("Shutting down SQL database.");
    snprintf(log_msg, MAX_STRING_LENGTH, "%%^RED%%^WileyMUD Version: %s (%s), PostgreSQL Version %s.%%^RESET%%^", VERSION_BUILD, VERSION_DATE, sql_version());
    allchan_log(0,"wiley", "Cron", "WileyMUD", log_msg);
    sql_disconnect();
}

/*
 * All the setup_() functions here assume we're already connected.
 */

void setup_pinkfish_map_table(void) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    char *sql = "CREATE TABLE IF NOT EXISTS pinkfish_map ( "
                "    pinkfish TEXT PRIMARY KEY NOT NULL, "
                "    html TEXT NOT NULL "
                "); ";

    sql_connect();
    res = PQexec(db, sql);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create pinkfish_map table: %s", PQerrorMessage(db));
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

    sql_connect();
    res = PQexec(db, sql);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create hours table: %s", PQerrorMessage(db));
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

    sql_connect();
    res = PQexec(db, sql);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create channels table: %s", PQerrorMessage(db));
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

    sql_connect();
    res = PQexec(db, sql);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create speakers table: %s", PQerrorMessage(db));
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
              "       ON (lower(i3log.speaker) = speakers.speaker) "
              "LEFT JOIN pinkfish_map pinkfish_map_hour "
              "       ON (hours.pinkfish = pinkfish_map_hour.pinkfish) "
              "LEFT JOIN pinkfish_map pinkfish_map_channel "
              "       ON (channels.pinkfish = pinkfish_map_channel.pinkfish) "
              "LEFT JOIN pinkfish_map pinkfish_map_speaker "
              "       ON (speakers.pinkfish = pinkfish_map_speaker.pinkfish); ";

    sql_connect();
    res = PQexec(db, sql);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create i3log table: %s", PQerrorMessage(db));
        PQclear(res);
	proper_exit(MUD_HALT);
    }
    PQclear(res);

    res = PQexec(db, sql2);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create i3log local index: %s", PQerrorMessage(db));
        PQclear(res);
	proper_exit(MUD_HALT);
    }
    PQclear(res);

    res = PQexec(db, sql3);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot drop i3log row constraint: %s", PQerrorMessage(db));
        PQclear(res);
	proper_exit(MUD_HALT);
    }
    PQclear(res);

    res = PQexec(db, sql4);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create i3log row constraint: %s", PQerrorMessage(db));
        PQclear(res);
	proper_exit(MUD_HALT);
    }
    PQclear(res);

    res = PQexec(db, sql5);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot drop page view: %s", PQerrorMessage(db));
        PQclear(res);
	proper_exit(MUD_HALT);
    }
    PQclear(res);

    res = PQexec(db, sql6);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create page view: %s", PQerrorMessage(db));
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
                "    checksum TEXT "
                "); ";
    char *sql2 = "DROP INDEX IF EXISTS ix_urls_checksum;";
    char *sql3 = "CREATE UNIQUE INDEX ix_urls_checksum ON urls (checksum);";

    sql_connect();
    res = PQexec(db, sql);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create urls table: %s", PQerrorMessage(db));
        PQclear(res);
	proper_exit(MUD_HALT);
    }
    PQclear(res);

    res = PQexec(db, sql2);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot drop urls checksum index: %s", PQerrorMessage(db));
        PQclear(res);
	proper_exit(MUD_HALT);
    }
    PQclear(res);

    res = PQexec(db, sql3);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create urls checksum index: %s", PQerrorMessage(db));
        PQclear(res);
	proper_exit(MUD_HALT);
    }
    PQclear(res);

}

void setup_log_table(void) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    char *sql = "CREATE TABLE IF NOT EXISTS log ( "
                "    created TIMESTAMP WITHOUT TIME ZONE NOT NULL DEFAULT (now() AT TIME ZONE 'UTC'), "
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

    sql_connect();
    res = PQexec(db, sql);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create log table: %s", PQerrorMessage(db));
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
                      "DO NOTHING; ";
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

    sql_connect();
    res = PQexecParams(db, sql, 5, NULL, param_val, param_len, param_bin, 0);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot insert url: %s", PQerrorMessage(db));
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

void allchan_sql( int is_emote, const char *channel, const char *speaker, const char *mud, const char *message ) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    const char *sql = "INSERT INTO i3log ( channel, speaker, mud, message, is_emote, is_url, is_bot ) "
                      "VALUES ($1,$2,$3,$4,$5,$6,$7);";
    const char *param_val[7];
    int param_len[7];
    //int param_bin[7] = {0,0,0,0,1,1,1}; // Are booleans considered binary?
    int param_bin[7] = {0,0,0,0,0,0,0};
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
    param_val[2] = mud;
    param_val[3] = message;
    //param_val[4] = (char *)&is_emote;
    //param_val[5] = (char *)&url;
    //param_val[6] = (char *)&bot;
    param_val[4] = param_emote;
    param_val[5] = param_url;
    param_val[6] = param_bot;

    param_len[0] = channel ? strlen(channel) : 0;
    param_len[1] = speaker ? strlen(speaker) : 0;
    param_len[2] = mud ? strlen(mud) : 0;
    param_len[3] = message ? strlen(message) : 0;
    //param_len[4] = sizeof(is_emote);
    //param_len[5] = sizeof(url);
    //param_len[6] = sizeof(bot);
    param_len[4] = strlen(param_emote);
    param_len[5] = strlen(param_url);
    param_len[6] = strlen(param_bot);

    sql_connect();
    res = PQexecParams(db, sql, 7, NULL, param_val, param_len, param_bin, 0);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot insert message: %s", PQerrorMessage(db));
        //PQclear(res);
	//proper_exit(MUD_HALT);
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
    const char *sql = "INSERT INTO log ( logtype, filename, function, line, "
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

    return; // remove me for production

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

    sql_connect();
    res = PQexecParams(db, sql, 11, NULL, param_val, param_len, param_bin, 0);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot insert log message: %s", PQerrorMessage(db));
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

    sql_connect();
    res = PQexecParams(db, sql, 2, NULL, param_val, param_len, param_bin, 0);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot add speaker: %s", PQerrorMessage(db));
        //PQclear(res);
	//proper_exit(MUD_HALT);
    }
    PQclear(res);
}


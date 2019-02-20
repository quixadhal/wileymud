#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcre.h>
#include <unistd.h>
#include <sys/types.h>
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

sqlite3 *db = NULL;

void sql_connect(void) {
    int rc = sqlite3_open(SQL_DB, &db);
    if (rc != SQLITE_OK) {
        log_fatal("Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
	proper_exit(MUD_HALT);
    }
}

void sql_disconnect(void) {
    sqlite3_close(db);
    db = NULL;
}

void sql_startup(void) {
    char log_msg[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    log_boot("Opening SQL database.");
    sql_connect();
    log_boot("SQLite Version: %s\n", sqlite3_libversion());
    setup_i3log_table();
    setup_urls_table();
    setup_log_table();
    setup_pinkfish_map_table();
    setup_hours_table();
    setup_channels_table();
    setup_speakers_table();
    snprintf(log_msg, MAX_STRING_LENGTH, "%%^GREEN%%^WileyMUD Version: %s (%s), SQLite Version %s.%%^RESET%%^", VERSION_BUILD, VERSION_DATE, sqlite3_libversion());
    allchan_log(0,"wiley", "Cron", "WileyMUD", log_msg);
}

void sql_shutdown(void) {
    char log_msg[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    log_boot("Shutting down SQL database.");
    snprintf(log_msg, MAX_STRING_LENGTH, "%%^RED%%^WileyMUD Version: %s (%s), SQLite Version %s.%%^RESET%%^", VERSION_BUILD, VERSION_DATE, sqlite3_libversion());
    allchan_log(0,"wiley", "Cron", "WileyMUD", log_msg);
    sql_disconnect();
}

/*
 * All the setup_() functions here assume we're already connected.
 */

void setup_i3log_table(void) {
    char *err_msg = NULL;
    char *sql = "CREATE TABLE IF NOT EXISTS i3log ( "
                "created DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%f', 'NOW','utc')), "
                "is_emote INTEGER, "
                "is_url INTEGER, "
                "is_bot INTEGER, "
                "channel TEXT, "
                "speaker TEXT, "
                "mud TEXT, "
                "message TEXT "
                ");";

    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        log_fatal("Cannot create %s: %s\n", "i3log table", err_msg);
        sqlite3_free(err_msg);
	proper_exit(MUD_HALT);
    }
}

void setup_urls_table(void) {
    char *err_msg = NULL;
    // This table stores urls that have been found in I3 messages.
    // When they are put in the table, the message field is left NULL, so an
    // external helper process can find and process them, filling in that data.
    // Once the message is NOT NULL, and processed is NULL, they will be emitted
    // to the url channel of I3, and then processed will become 1.
    char *sql = "CREATE TABLE IF NOT EXISTS urls ( "
                "created DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%f', 'NOW','utc')), "
                "processed INTEGER, "
                "channel TEXT, "
                "speaker TEXT, "
                "mud TEXT, "
                "url TEXT, "
                "message TEXT, "
                "checksum TEXT "
                ");";

    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        log_fatal("Cannot create %s: %s\n", "url table", err_msg);
        sqlite3_free(err_msg);
	proper_exit(MUD_HALT);
    }
}

void setup_log_table(void) {
    char *err_msg = NULL;
    char *sql = "CREATE TABLE IF NOT EXISTS log ( "
                "created DATETIME DEFAULT (STRFTIME('%Y-%m-%d %H:%M:%f', 'NOW','utc')), "
                "logtype TEXT DEFAULT 'INFO', "
                "filename TEXT, "
                "function TEXT, "
                "line INTEGER, "
                "area_file TEXT, "
                "area_line INTEGER, "
                "character TEXT, "
                "character_room INTEGER, "
                "victim TEXT, "
                "victim_room INTEGER, "
                "message TEXT "
                ");";

    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        log_fatal("Cannot create %s: %s\n", "log table", err_msg);
        sqlite3_free(err_msg);
	proper_exit(MUD_HALT);
    }
}

void setup_pinkfish_map_table(void) {
    char *err_msg = NULL;
    char *sql = "CREATE TABLE IF NOT EXISTS pinkfish_map ( "
                "pinkfish TEXT PRIMARY KEY NOT NULL, "
                "html TEXT "
                ");";

    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        log_fatal("Cannot create %s: %s\n", "pinkfish_map table", err_msg);
        sqlite3_free(err_msg);
	proper_exit(MUD_HALT);
    }
}

void setup_hours_table(void) {
    char *err_msg = NULL;
    char *sql = "CREATE TABLE IF NOT EXISTS hours ( "
                "hour TEXT PRIMARY KEY NOT NULL, "
                "pinkfish TEXT "
                ");";

    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        log_fatal("Cannot create %s: %s\n", "hours table", err_msg);
        sqlite3_free(err_msg);
	proper_exit(MUD_HALT);
    }
}

void setup_channels_table(void) {
    char *err_msg = NULL;
    char *sql = "CREATE TABLE IF NOT EXISTS channels ( "
                "channel TEXT PRIMARY KEY NOT NULL, "
                "pinkfish TEXT "
                ");";

    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        log_fatal("Cannot create %s: %s\n", "channels table", err_msg);
        sqlite3_free(err_msg);
	proper_exit(MUD_HALT);
    }
}

void setup_speakers_table(void) {
    char *err_msg = NULL;
    char *sql = "CREATE TABLE IF NOT EXISTS speakers ( "
                "speaker TEXT PRIMARY KEY NOT NULL, "
                "pinkfish TEXT "
                ");";

    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        log_fatal("Cannot create %s: %s\n", "speakers table", err_msg);
        sqlite3_free(err_msg);
	proper_exit(MUD_HALT);
    }
}

//int                                     unprocessed_urls = 0;
//int                                     tics_to_next_url_processing = URL_DELAY;
//int                                     first_processing = 1;

void add_url( const char *channel, const char *speaker, const char *mud, const char *url ) {

    //log_info("add_url entered");

    int rc;
    u_int32_t crc;
    char *err_msg = NULL;
    sqlite3_stmt *insert_stmt = NULL;
    char *sql = "INSERT INTO urls ( url, channel, speaker, mud, checksum ) VALUES (?,?,?,?,?);";
    //char *sql = "INSERT INTO urls ( url, channel, speaker, mud ) VALUES (?,?,?,?);";
    char checksum[MAX_INPUT_LENGTH];

    crc = crc32(url, strlen(url));
    snprintf(checksum, MAX_INPUT_LENGTH, "%08x", crc);

    sql_connect();
    rc = sqlite3_prepare_v2( db, sql, -1, &insert_stmt, NULL );
    if (rc != SQLITE_OK) {
        log_fatal("SQL statement error %s: %s\n", "urls insert", sqlite3_errmsg(db));
	proper_exit(MUD_REBOOT);
    }
    rc = sqlite3_bind_text( insert_stmt, 1, url, strlen(url), NULL );
    if (rc != SQLITE_OK) {
        log_fatal("SQL parameter error %s: %s\n", "urls url", sqlite3_errmsg(db));
	proper_exit(MUD_REBOOT);
    }
    rc = sqlite3_bind_text( insert_stmt, 2, channel, strlen(channel), NULL );
    if (rc != SQLITE_OK) {
        log_fatal("SQL parameter error %s: %s\n", "urls channel", sqlite3_errmsg(db));
	proper_exit(MUD_REBOOT);
    }
    rc = sqlite3_bind_text( insert_stmt, 3, speaker, strlen(speaker), NULL );
    if (rc != SQLITE_OK) {
        log_fatal("SQL parameter error %s: %s\n", "urls speaker", sqlite3_errmsg(db));
	proper_exit(MUD_REBOOT);
    }
    rc = sqlite3_bind_text( insert_stmt, 4, mud, strlen(mud), NULL );
    if (rc != SQLITE_OK) {
        log_fatal("SQL parameter error %s: %s\n", "urls mud", sqlite3_errmsg(db));
	proper_exit(MUD_REBOOT);
    }
    rc = sqlite3_bind_text( insert_stmt, 5, checksum, strlen(checksum), NULL );
    if (rc != SQLITE_OK) {
        log_fatal("SQL parameter error %s: %s\n", "urls mud", sqlite3_errmsg(db));
	proper_exit(MUD_REBOOT);
    }

    for(int x = 0; x < 4; x++) {
        rc = sqlite3_step(insert_stmt);
        if (rc == SQLITE_DONE) {
            sqlite3_finalize(insert_stmt);
            break;
        } else if (rc == SQLITE_BUSY && x < 3) {
            usleep(10000); // Horrible, but what else can we do?
            continue;
        } else {
            log_fatal("SQL insert error %s: %s\n", "urls insert", err_msg);
            sqlite3_free(err_msg);
            proper_exit(MUD_REBOOT);
        }
    }
//    unprocessed_urls++;

//    log_info("add_url done, unprocessed urls == %d", unprocessed_urls);
    log_info("add_url done, added %s: %s", checksum, url);
    sql_disconnect();
}

void process_urls( void ) {
    // This routine will look in the urls table for any entries which have
    // a non-NULL message field and a NULL processed field.  That means our
    // external helper app has done the lookups and pushed the results into
    // the table, but we haven't yet sent those results out over I3.
    //
    // For each result we find, we need to update the row to set processed
    // to 1, so we don't do it again later.

//    if( unprocessed_urls < 0 ) {
//        // This can happen if we have leftovers from a previous run,
//        // or if we screw up and fail to process one and end up doing
//        // it twice somehow.
//        unprocessed_urls = 0;
//    }

//    if( tics_to_next_url_processing > 0 ) {
//        tics_to_next_url_processing--;
//        return;
//    }

//    if( !first_processing && unprocessed_urls < 1 ) {
//        //log_info("process_urls no work to do");
//        return;
//    }

    //log_info("process_urls started with work to do");
//    tics_to_next_url_processing = URL_DELAY;
    char *err_msg = NULL;
    char *sql = "  SELECT created, url, message "
                "    FROM urls "
                "   WHERE processed IS NOT 1 AND message IS NOT NULL "
                "ORDER BY created ASC;";
    int rc;

    sql_connect();
    for(int x = 0; x < 4; x++) {
        rc = sqlite3_exec(db, sql, process_url_callback, 0, &err_msg);
        if (rc == SQLITE_OK) {
            break;
        } else if (rc == SQLITE_BUSY && x < 3) {
            usleep(10000); // Horrible, but what else can we do?
            continue;
        } else {
            log_fatal("SQL statement error %s: %s\n", "urls select", sqlite3_errmsg(db));
            sqlite3_free(err_msg);
            proper_exit(MUD_REBOOT);
        }
    }

//    if( first_processing ) {
//        first_processing = 0;
//    }

    //log_info("process_urls done");
    sql_disconnect();
}

// SQLite can use callbacks for each row retrieved from a select statement.
// I am going to ASSUME that SQLite is not actually multi-threaded, and thus the
// database connection is still open from the previous function.
// Likewise, since the previous function will close it, we don't have to.
int process_url_callback(void *unused, int count, char **values, char **keys) {
    //log_info("process_url_callback entered");

    char *err_msg = NULL;
    sqlite3_stmt *update_stmt = NULL;
    char *sql = "  UPDATE urls "
                "     SET processed = 1 "
                "   WHERE created = ? AND url = ? AND processed IS NOT 1 AND message IS NOT NULL;";

    //log_info("Count = %d", count);
    if( count < 3 ) {
        log_error("process_url_callback aborted, expected 3 arguments, got %d", count);
        if( count >= 1 && values[0] ) log_error("created = %s", values[0]);
        if( count >= 2 && values[1] ) log_error("url = %s", values[1]);
        if( count >= 3 && values[2] ) log_error("message = %s", values[2]);
        return 0;
    }

    if( values[2] ) {
        // We have a message string!
        i3_npc_speak("url", "URLbot", values[2]);
    }
    // Now we need to mark this as processed...
    int rc = sqlite3_prepare_v2( db, sql, -1, &update_stmt, NULL );
    if (rc != SQLITE_OK) {
        log_fatal("SQL statement error %s: %s\n", "urls update", sqlite3_errmsg(db));
	proper_exit(MUD_REBOOT);
    }
    rc = sqlite3_bind_text( update_stmt, 1, values[0], strlen(values[0]), NULL );
    if (rc != SQLITE_OK) {
        log_fatal("SQL parameter error %s: %s\n", "urls created", sqlite3_errmsg(db));
	proper_exit(MUD_REBOOT);
    }
    rc = sqlite3_bind_text( update_stmt, 2, values[1], strlen(values[1]), NULL );
    if (rc != SQLITE_OK) {
        log_fatal("SQL parameter error %s: %s\n", "urls url", sqlite3_errmsg(db));
	proper_exit(MUD_REBOOT);
    }

    rc = sqlite3_step(update_stmt);
    if (rc != SQLITE_DONE) {
        log_fatal("SQL update error %s: %s\n", "urls update", err_msg);
        sqlite3_free(err_msg);
	proper_exit(MUD_REBOOT);
    }
    sqlite3_finalize(update_stmt);
//    unprocessed_urls--;
//    log_info("process_url_callback done, unprocessed_urls == %d", unprocessed_urls);
    log_info("process_url_callback done, processed %s", values[1]);
    return 0;
}

void spawn_url_handler(void) {
    int                  regexp_pid;

    regexp_pid = fork();
    if( regexp_pid == 0 ) {
        // We are the new kid, so go run our perl script.
        log_info("Forking %s", UNTINY);
        //execl(PERL, PERL, "-w", UNTINY_SQL, (char *)NULL);
        execl(PERL, PERL, "-w", UNTINY, "--sql", (char *)NULL);
        log_error("It is not possible to be here!");
    } else {
        // Normally, we need to track the child pid, but we're already
        // ignoring SIGCHLD in signals.c, and doing so prevents zombies.
    }
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
    char *err_msg = NULL;
    sqlite3_stmt *insert_stmt = NULL;
    char *sql = "INSERT INTO i3log ( channel, speaker, mud, message, is_emote, is_url, is_bot ) "
                "VALUES (?,?,?,?,?,?,?);";

    int url = is_url(is_emote, channel, speaker, mud, message);
    int bot = is_bot(is_emote, channel, speaker, mud, message);
    int rc;

    sql_connect();
    rc = sqlite3_prepare_v2( db, sql, -1, &insert_stmt, NULL );
    if (rc != SQLITE_OK) {
        log_fatal("SQL statement error %s: %s\n", "i3log insert", sqlite3_errmsg(db));
	proper_exit(MUD_REBOOT);
    }
    rc = sqlite3_bind_text( insert_stmt, 1, channel, strlen(channel), NULL );
    if (rc != SQLITE_OK) {
        log_fatal("SQL parameter error %s: %s\n", "i3log channel", sqlite3_errmsg(db));
	proper_exit(MUD_REBOOT);
    }
    rc = sqlite3_bind_text( insert_stmt, 2, speaker, strlen(speaker), NULL );
    if (rc != SQLITE_OK) {
        log_fatal("SQL parameter error %s: %s\n", "i3log speaker", sqlite3_errmsg(db));
	proper_exit(MUD_REBOOT);
    }
    rc = sqlite3_bind_text( insert_stmt, 3, mud, strlen(mud), NULL );
    if (rc != SQLITE_OK) {
        log_fatal("SQL parameter error %s: %s\n", "i3log mud", sqlite3_errmsg(db));
	proper_exit(MUD_REBOOT);
    }
    rc = sqlite3_bind_text( insert_stmt, 4, message, strlen(message), NULL );
    if (rc != SQLITE_OK) {
        log_fatal("SQL parameter error %s: %s\n", "i3log message", sqlite3_errmsg(db));
	proper_exit(MUD_REBOOT);
    }
    rc = sqlite3_bind_int( insert_stmt, 5, is_emote );
    if (rc != SQLITE_OK) {
        log_fatal("SQL parameter error %s: %s\n", "i3log is_emote", sqlite3_errmsg(db));
	proper_exit(MUD_REBOOT);
    }
    rc = sqlite3_bind_int( insert_stmt, 6, url );
    if (rc != SQLITE_OK) {
        log_fatal("SQL parameter error %s: %s\n", "i3log is_url", sqlite3_errmsg(db));
	proper_exit(MUD_REBOOT);
    }
    rc = sqlite3_bind_int( insert_stmt, 7, bot );
    if (rc != SQLITE_OK) {
        log_fatal("SQL parameter error %s: %s\n", "i3log is_bot", sqlite3_errmsg(db));
	proper_exit(MUD_REBOOT);
    }

    for(int x = 0; x < 4; x++) {
        rc = sqlite3_step(insert_stmt);
        if (rc == SQLITE_DONE) {
            sqlite3_finalize(insert_stmt);
            break;
        } else if (rc == SQLITE_BUSY && x < 3) {
            usleep(10000); // Horrible, but what else can we do?
            continue;
        } else {
            log_fatal("SQL insert error %s: %s\n", "i3log insert", err_msg);
            sqlite3_free(err_msg);
            proper_exit(MUD_REBOOT);
        }
    }
    sql_disconnect();
}

void bug_sql( const char *logtype, const char *filename, const char *function, int line,
              const char *area_file, int area_line, 
              const char *character, int character_room,
              const char *victim, int victim_room, 
              const char *message ) {

    char *err_msg = NULL;
    sqlite3_stmt *insert_stmt = NULL;
    char *sql = "INSERT INTO log ( logtype, filename, function, line, area_file, area_line, "
                "character, character_room, victim, victim_room, message ) "
                "VALUES (?,?,?,?,?,?,?,?,?,?,?);";

    return; // remove me for production

    sql_connect();
    int rc = sqlite3_prepare_v2( db, sql, -1, &insert_stmt, NULL );
    if (rc != SQLITE_OK) {
        log_fatal("SQL statement error %s: %s\n", "log insert", sqlite3_errmsg(db));
	proper_exit(MUD_REBOOT);
    }
    if(logtype && *logtype) {
        rc = sqlite3_bind_text( insert_stmt, 1, logtype, strlen(logtype), NULL );
        if (rc != SQLITE_OK) {
            log_fatal("SQL parameter error %s: %s\n", "log logtype", sqlite3_errmsg(db));
            proper_exit(MUD_REBOOT);
        }
    }
    if(filename && *filename) {
        rc = sqlite3_bind_text( insert_stmt, 2, filename, strlen(filename), NULL );
        if (rc != SQLITE_OK) {
            log_fatal("SQL parameter error %s: %s\n", "log filename", sqlite3_errmsg(db));
            proper_exit(MUD_REBOOT);
        }
    }
    if(function && *function) {
        rc = sqlite3_bind_text( insert_stmt, 3, function, strlen(function), NULL );
        if (rc != SQLITE_OK) {
            log_fatal("SQL parameter error %s: %s\n", "log function", sqlite3_errmsg(db));
            proper_exit(MUD_REBOOT);
        }
    }
    if(line > 0) {
        rc = sqlite3_bind_int( insert_stmt, 4, line );
        if (rc != SQLITE_OK) {
            log_fatal("SQL parameter error %s: %s\n", "log line", sqlite3_errmsg(db));
            proper_exit(MUD_REBOOT);
        }
    }
    if(area_file && *area_file) {
        rc = sqlite3_bind_text( insert_stmt, 5, area_file, strlen(area_file), NULL );
        if (rc != SQLITE_OK) {
            log_fatal("SQL parameter error %s: %s\n", "log area_file", sqlite3_errmsg(db));
            proper_exit(MUD_REBOOT);
        }
    }
    if(area_line > 0) {
        rc = sqlite3_bind_int( insert_stmt, 6, area_line );
        if (rc != SQLITE_OK) {
            log_fatal("SQL parameter error %s: %s\n", "log area_line", sqlite3_errmsg(db));
            proper_exit(MUD_REBOOT);
        }
    }
    if(character && *character) {
        rc = sqlite3_bind_text( insert_stmt, 7, character, strlen(character), NULL );
        if (rc != SQLITE_OK) {
            log_fatal("SQL parameter error %s: %s\n", "log character", sqlite3_errmsg(db));
            proper_exit(MUD_REBOOT);
        }
    }
    if(character_room > 0) {
        rc = sqlite3_bind_int( insert_stmt, 8, character_room );
        if (rc != SQLITE_OK) {
            log_fatal("SQL parameter error %s: %s\n", "log character_room", sqlite3_errmsg(db));
            proper_exit(MUD_REBOOT);
        }
    }
    if(victim && *victim) {
        rc = sqlite3_bind_text( insert_stmt, 9, victim, strlen(victim), NULL );
        if (rc != SQLITE_OK) {
            log_fatal("SQL parameter error %s: %s\n", "log victim", sqlite3_errmsg(db));
            proper_exit(MUD_REBOOT);
        }
    }
    if(victim_room > 0) {
        rc = sqlite3_bind_int( insert_stmt, 10, victim_room );
        if (rc != SQLITE_OK) {
            log_fatal("SQL parameter error %s: %s\n", "log victim_room", sqlite3_errmsg(db));
            proper_exit(MUD_REBOOT);
        }
    }
    rc = sqlite3_bind_text( insert_stmt, 11, message, strlen(message), NULL );
    if (rc != SQLITE_OK) {
        log_fatal("SQL parameter error %s: %s\n", "log message", sqlite3_errmsg(db));
	proper_exit(MUD_REBOOT);
    }

    for(int x = 0; x < 4; x++) {
        rc = sqlite3_step(insert_stmt);
        if (rc == SQLITE_DONE) {
            sqlite3_finalize(insert_stmt);
            break;
        } else if (rc == SQLITE_BUSY && x < 3) {
            usleep(10000); // Horrible, but what else can we do?
            continue;
        } else {
            log_fatal("SQL insert error %s: %s\n", "log insert", err_msg);
            sqlite3_free(err_msg);
            proper_exit(MUD_REBOOT);
        }
    }
    sql_disconnect();
}

void addspeaker_sql( const char *speaker, const char *pinkfish ) {
    char *err_msg = NULL;
    sqlite3_stmt *insert_stmt = NULL;
    char *sql = "INSERT OR IGNORE INTO speakers ( speaker, pinkfish) "
                "VALUES (?,?);";

    log_info("ADD_SPEAKER: %s = %s", VNULL(speaker), VNULL(pinkfish));
    if(!speaker || !*speaker || !pinkfish || !*pinkfish)
        return;

    sql_connect();
    int rc = sqlite3_prepare_v2( db, sql, -1, &insert_stmt, NULL );
    if (rc != SQLITE_OK) {
        log_fatal("SQL statement error %s: %s\n", "speakers insert", sqlite3_errmsg(db));
	proper_exit(MUD_REBOOT);
    }
    rc = sqlite3_bind_text( insert_stmt, 1, speaker, strlen(speaker), NULL );
    if (rc != SQLITE_OK) {
        log_fatal("SQL parameter error %s: %s\n", "speakers speaker", sqlite3_errmsg(db));
	proper_exit(MUD_REBOOT);
    }
    rc = sqlite3_bind_text( insert_stmt, 2, pinkfish, strlen(pinkfish), NULL );
    if (rc != SQLITE_OK) {
        log_fatal("SQL parameter error %s: %s\n", "speakers pinkfish", sqlite3_errmsg(db));
	proper_exit(MUD_REBOOT);
    }

    for(int x = 0; x < 4; x++) {
        rc = sqlite3_step(insert_stmt);
        if (rc == SQLITE_DONE) {
            sqlite3_finalize(insert_stmt);
            break;
        } else if (rc == SQLITE_BUSY && x < 3) {
            usleep(10000); // Horrible, but what else can we do?
            continue;
        } else {
            log_fatal("SQL insert error %s: %s\n", "speakers insert", err_msg);
            sqlite3_free(err_msg);
            proper_exit(MUD_REBOOT);
        }
    }
    sql_disconnect();
}


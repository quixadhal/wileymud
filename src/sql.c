#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcre.h>
#include <unistd.h>
#include "global.h"
#include "bug.h"
#include "comm.h"
#include "version.h"
#ifdef I3
#include "i3.h"
#endif
#define _SQL_C
#include "sql.h"

sqlite3 *db = NULL;
void setup_i3log_table(void);

void sql_startup(void) {
    char log_msg[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    int rc = sqlite3_open(SQL_DB, &db);
    log_boot("Opening SQL database.");
    if (rc != SQLITE_OK) {
        log_fatal("Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
	proper_exit(MUD_HALT);
    }
    log_boot("SQLite Version: %s\n", sqlite3_libversion());
    setup_i3log_table();
    snprintf(log_msg, MAX_STRING_LENGTH, "%%^GREEN%%^WileyMUD Version: %s (%s), SQLite Version %s.%%^RESET%%^", VERSION_BUILD, VERSION_DATE, sqlite3_libversion());
    allchan_log(0,"wiley", "Cron", "WileyMUD", log_msg);
}

void sql_shutdown(void) {
    char log_msg[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    log_boot("Shutting down SQL database.");
    snprintf(log_msg, MAX_STRING_LENGTH, "%%^RED%%^WileyMUD Version: %s (%s), SQLite Version %s.%%^RESET%%^", VERSION_BUILD, VERSION_DATE, sqlite3_libversion());
    allchan_log(0,"wiley", "Cron", "WileyMUD", log_msg);
    sqlite3_close(db);
}

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
        sqlite3_close(db);
	proper_exit(MUD_HALT);
    }
}

int is_url( int is_emote, char *channel, char *speaker, char *mud, char *message ) {
    const char          *regexp_pattern     = "(https?\\:\\/\\/[\\w.-]+(?:\\.[\\w\\.-]+)+[\\w\\-\\._~:/?#[\\]@!\\$&'\\(\\)\\*\\+,;=.]+)";
    int                  regexp_opts        = PCRE_CASELESS|PCRE_MULTILINE;
    static pcre         *regexp_compiled    = NULL;
    static pcre_extra   *regexp_studied     = NULL;
    const char          *regexp_error       = NULL;
    int                  regexp_err_offset  = 0;
    const char          *regexp_match       = NULL;
    static int           regexp_broken      = 0;
    int                  regexp_rv;
    int                  regexp_matchpos[30];
    int                  regexp_pid;
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
    // We have one or more URLs, so this is where we process them.
    // This used to be in i3.c, but we might as well move it here.
    // The idea is simple, fork a process for each URL to run an
    // external perl script that nicely handles URL lookups, dumping
    // the results in a file that gets checked elsewhere for output.
    while( regexp_rv > 0 ) {
        if (pcre_get_substring(url_stripped, regexp_matchpos, regexp_rv, 0, &regexp_match) >= 0) {
            log_info("Found URL ( %s ) in channel ( %s )", regexp_match, channel);
            regexp_pid = fork();
            if( regexp_pid == 0 ) {
                // We are the new kid, so go run our perl script.
                //i3log("Launching %s -w %s %s %s", PERL, UNTINY, regexp_match, channel->local_name);
                freopen(I3_URL_DUMP, "a", stdout);
                execl(PERL, PERL, "-w", UNTINY, regexp_match, channel, (char *)NULL);
                log_error("It is not possible to be here!");
            } else {
                // Normally, we need to track the child pid, but we're already
                // ignoring SIGCHLD in signals.c, and doing so prevents zombies.
            }
        }
        regexp_rv = pcre_exec(regexp_compiled, regexp_studied,
                              url_stripped, strlen(url_stripped),
                              regexp_matchpos[1], 0,
                              regexp_matchpos, 30);
    }
    return 1;
}

int is_bot( int is_emote, char *channel, char *speaker, char *mud, char *message ) {
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

void allchan_sql( int is_emote, char *channel, char *speaker, char *mud, char *message ) {
    char *err_msg = NULL;
    sqlite3_stmt *insert_stmt = NULL;
    char *sql = "INSERT INTO i3log ( channel, speaker, mud, message, is_emote, is_url, is_bot ) "
                "VALUES (?,?,?,?,?,?,?);";

    int url = is_url(is_emote, channel, speaker, mud, message);
    int bot = is_bot(is_emote, channel, speaker, mud, message);

    int rc = sqlite3_prepare_v2( db, sql, -1, &insert_stmt, NULL );
    if (rc != SQLITE_OK) {
        log_fatal("SQL statement error %s: %s\n", "i3log insert", sqlite3_errmsg(db));
        sqlite3_close(db);
	proper_exit(MUD_HALT);
    }
    rc = sqlite3_bind_text( insert_stmt, 1, channel, strlen(channel), NULL );
    if (rc != SQLITE_OK) {
        log_fatal("SQL parameter error %s: %s\n", "i3log channel", sqlite3_errmsg(db));
        sqlite3_close(db);
	proper_exit(MUD_HALT);
    }
    rc = sqlite3_bind_text( insert_stmt, 2, speaker, strlen(speaker), NULL );
    if (rc != SQLITE_OK) {
        log_fatal("SQL parameter error %s: %s\n", "i3log speaker", sqlite3_errmsg(db));
        sqlite3_close(db);
	proper_exit(MUD_HALT);
    }
    rc = sqlite3_bind_text( insert_stmt, 3, mud, strlen(mud), NULL );
    if (rc != SQLITE_OK) {
        log_fatal("SQL parameter error %s: %s\n", "i3log mud", sqlite3_errmsg(db));
        sqlite3_close(db);
	proper_exit(MUD_HALT);
    }
    rc = sqlite3_bind_text( insert_stmt, 4, message, strlen(message), NULL );
    if (rc != SQLITE_OK) {
        log_fatal("SQL parameter error %s: %s\n", "i3log message", sqlite3_errmsg(db));
        sqlite3_close(db);
	proper_exit(MUD_HALT);
    }
    rc = sqlite3_bind_int( insert_stmt, 5, is_emote );
    if (rc != SQLITE_OK) {
        log_fatal("SQL parameter error %s: %s\n", "i3log is_emote", sqlite3_errmsg(db));
        sqlite3_close(db);
	proper_exit(MUD_HALT);
    }
    rc = sqlite3_bind_int( insert_stmt, 6, url );
    if (rc != SQLITE_OK) {
        log_fatal("SQL parameter error %s: %s\n", "i3log is_url", sqlite3_errmsg(db));
        sqlite3_close(db);
	proper_exit(MUD_HALT);
    }
    rc = sqlite3_bind_int( insert_stmt, 7, bot );
    if (rc != SQLITE_OK) {
        log_fatal("SQL parameter error %s: %s\n", "i3log is_bot", sqlite3_errmsg(db));
        sqlite3_close(db);
	proper_exit(MUD_HALT);
    }

    rc = sqlite3_step(insert_stmt);
    if (rc != SQLITE_DONE) {
        log_fatal("SQL insert error %s: %s\n", "i3log insert", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
	proper_exit(MUD_HALT);
    }
    sqlite3_finalize(insert_stmt);
}


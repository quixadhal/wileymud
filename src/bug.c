#include <stdio.h>
#include <stdlib.h>
/* #include <unistd.h> */
#include <sys/types.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
//#include <sys/timeb.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "global.h"
#include "utils.h"
#include "comm.h"
#include "multiclass.h"
#include "db.h"
#include "sql.h"

#define _BUG_C
#include "bug.h"

const char *LogNames[] = {"INFO", "ERROR", "FATAL", "BOOT", "AUTH", "KILL", "DEATH", "RESET", "IMC", "SQL", "NOSQL", "I3"};

/* Handy query for checking logs:
 *
 * select to_char(date_trunc('second', log_date), 'HH24:MI:SS'), log_types.name, log_entry
 * from logfile join log_types using (log_type_id)
 * where log_date > now() - interval '1 day'
 * order by log_date desc
 * limit 20;
 */

/*
 * Things we want to have when logging events.....
 *
 * BugFile is the filename you want to log to, NULL means stderr
 *
 * File, Func, Line can all be provided by the compiler as
 * __FILE__, __PRETTY_FUNCTION__, and __LINE__
 *
 * Level is the minimum character level which will see the
 * bug if they're logged in.
 *
 * The AreaFile and AreaLine are the file and line number
 * we were reading while booting the world database.
 *
 * Type is the type of error, typically things like
 * LOG_INFO, LOG_ERROR, LOG_FATAL, LOG_BOOT, LOG_AUTH
 *
 * ch is the char_data pointer for the player/mob
 * obj is an obj_data pointer, if you have one
 * room is.... the room_data pointer.
 *
 * Str is, of course, the message, and it gets printed
 * using varargs, so you can have this be a printf type
 * set of macros.
 */
void bug_logger(unsigned int Type, const char *BugFile, const char *File, const char *Func, int Line,
                const char *AreaFile, int AreaLine, struct char_data *ch, struct char_data *victim, unsigned int Level,
                const char *Str, ...)
{
    va_list arg;
    char Result[MAX_STRING_LENGTH] = "\0\0\0";
    char Temp[MAX_STRING_LENGTH] = "\0\0\0";
    FILE *fp = NULL;
    // struct timeb                            right_now;
    struct timespec right_now;
    struct tm *now_part = NULL;

    bzero(Result, MAX_STRING_LENGTH);
    va_start(arg, Str);
    if (Str && *Str)
    {
        struct descriptor_data *i;

        snprintf(Result, MAX_STRING_LENGTH, "%s> ", LogNames[Type]);
        vsnprintf(Temp, MAX_STRING_LENGTH, Str, arg);
        strlcat(Result, Temp, MAX_STRING_LENGTH);
        for (i = descriptor_list; i; i = i->next)
            if ((!i->connected) && (GetMaxLevel(i->character) >= Level) &&
                (IS_SET(i->character->specials.new_act, NEW_PLR_LOGS)))
                write_to_q(Result, &i->output, 1);
        bzero(Result, MAX_STRING_LENGTH);
    }
    else
        strlcpy(Temp, "PING!", MAX_STRING_LENGTH);
    va_end(arg);
    // ftime(&right_now);
    clock_gettime(CLOCK_REALTIME, &right_now);
    now_part = localtime((const time_t *)&right_now);
    snprintf(Result, MAX_STRING_LENGTH, "<: %04d%02d%02d.%02d%02d%02d.%03d", now_part->tm_year + 1900,
             now_part->tm_mon + 1, now_part->tm_mday, now_part->tm_hour, now_part->tm_min, now_part->tm_sec,
             // right_now.millitm);
             ((int)(right_now.tv_nsec / 1000000)));
    scprintf(Result, MAX_STRING_LENGTH, " - %s -", LogNames[Type]);
    if (File || Func || Line)
    {
        strlcat(Result, " (", MAX_STRING_LENGTH);
        if (File && *File)
        {
            strlcat(Result, File, MAX_STRING_LENGTH);
        }
        if (Func && *Func)
            scprintf(Result, MAX_STRING_LENGTH, ";%s", Func);
        if (Line)
            scprintf(Result, MAX_STRING_LENGTH, ",%d)", Line);
        else
            strlcat(Result, ")", MAX_STRING_LENGTH);
    }
    if (ch || victim)
    {
        if (ch)
            scprintf(Result, MAX_STRING_LENGTH, " ch \"%s\" [#%d]", NAME(ch), ch->in_room);
        if (victim)
            scprintf(Result, MAX_STRING_LENGTH, " victim \"%s\" [#%d]", NAME(victim), victim->in_room);
        /*
            if (obj)
              scprintf(Result, MAX_STRING_LENGTH, " obj \"%s\" [#%d]",
                      SAFE_ONAME(obj), obj->in_room);
            if (room)
              scprintf(Result, MAX_STRING_LENGTH, " room \"%s\" [#%d]",
                      room->name?room->name:"", room->number);
        */
        strlcat(Result, "\n", MAX_STRING_LENGTH);
    }
    else if (File || Func || Line)
        strlcat(Result, "\n", MAX_STRING_LENGTH);

    strlcat(Result, " : ", MAX_STRING_LENGTH);
    strlcat(Result, Temp, MAX_STRING_LENGTH);

    if (BugFile && *BugFile)
    {
        if (!(fp = fopen(BugFile, "a")))
        {
            perror(BugFile);
            if (ch)
                cprintf(ch, "Could not open the file!\r\n");
        }
        else
        {
            fprintf(fp, "%s\n", Result);
            FCLOSE(fp);
        }
    }

    if (stderr)
    {
        fprintf(stderr, "%s\n", Result);
        fflush(stderr);
    }

    // Here is where we would log to SQL too!
    if (Type != LOG_NOSQL)
    {
        // If the Type is LOG_NOSQL, it's an error with the database and
        // we have to make sure we don't make a loop of infinite failure.
        bug_sql(LogNames[Type], File, Func, Line, NULL, 0, ch ? NAME(ch) : NULL, ch ? ch->in_room : 0,
                victim ? NAME(victim) : NULL, victim ? victim->in_room : 0, Temp);
    }
}

void setup_logfile_table(void)
{
    PGresult *res = NULL;
    ExecStatusType st = (ExecStatusType) 0;
    const char *sql = "CREATE TABLE IF NOT EXISTS logfile ( "
                "    created TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT now(), "
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
    const char *sql2 = "CREATE INDEX IF NOT EXISTS ix_logfile_created ON logfile (created);";
    const char *sql3 = "CREATE INDEX IF NOT EXISTS ix_logfile_logtype ON logfile (logtype);";
    const char *sql4 = "CREATE OR REPLACE VIEW logtail AS "
                 "SELECT to_char(logfile.created, 'YYYY-MM-DD HH24:MI:SS'::text) AS created, "
                 "       logfile.logtype, "
                 "       logfile.message "
                 "FROM logfile "
                 "ORDER BY (to_char(logfile.created, 'YYYY-MM-DD HH24:MI:SS'::text)) "
                 "OFFSET (( SELECT count(*) AS count FROM logfile logfile_1)) - 20; ";

    res = PQexec(db_logfile.dbc, sql);
    st = PQresultStatus(res);
    if (st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE)
    {
        log_fatal("Cannot create log table: %s", PQerrorMessage(db_logfile.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);

    res = PQexec(db_logfile.dbc, sql2);
    st = PQresultStatus(res);
    if (st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE)
    {
        log_fatal("Cannot create logfile created index: %s", PQerrorMessage(db_logfile.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);

    res = PQexec(db_logfile.dbc, sql3);
    st = PQresultStatus(res);
    if (st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE)
    {
        log_fatal("Cannot create logfile logtype index: %s", PQerrorMessage(db_logfile.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);

    res = PQexec(db_logfile.dbc, sql4);
    st = PQresultStatus(res);
    if (st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE)
    {
        log_fatal("Cannot create logtail view: %s", PQerrorMessage(db_logfile.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);
}

void bug_sql(const char *logtype, const char *filename, const char *function, int line, const char *area_file,
             int area_line, const char *character, int character_room, const char *victim, int victim_room,
             const char *message)
{
    PGresult *res = NULL;
    ExecStatusType st = (ExecStatusType) 0;
    const char *sql = "INSERT INTO logfile ( logtype, filename, function, line, "
                      "area_file, area_line, character, character_room, victim, "
                      "victim_room, message ) "
                      "VALUES ($1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11);";
    const char *param_val[11];
    int param_len[11];
    int param_bin[11] = {0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0};
    int param_line;
    int param_area_line;
    int param_char_room;
    int param_vic_room;

    if (!db_logfile.dbc)
    {
        // If we are not connected, and we've TRIED to connect, punt.
        return;
    }

    param_line = htonl(line);
    param_area_line = htonl(area_line);
    param_char_room = htonl(character_room);
    param_vic_room = htonl(victim_room);

    param_val[0] = (logtype && logtype[0]) ? logtype : NULL;
    param_val[1] = (filename && filename[0]) ? filename : NULL;
    param_val[2] = (function && function[0]) ? function : NULL;
    param_val[3] = (line > 0) ? (char *)&param_line : NULL;
    param_val[4] = (area_file && area_file[0]) ? area_file : NULL;
    param_val[5] = (area_line > 0) ? (char *)&param_area_line : NULL;
    param_val[6] = (character && character[0]) ? character : NULL;
    param_val[7] = (character_room > 0) ? (char *)&param_char_room : NULL;
    param_val[8] = (victim && victim[0]) ? victim : NULL;
    param_val[9] = (victim_room > 0) ? (char *)&param_vic_room : NULL;
    param_val[10] = (message && message[0]) ? message : NULL;

    param_len[0] = (logtype && logtype[0]) ? strlen(logtype) : 0;
    param_len[1] = (filename && filename[0]) ? strlen(filename) : 0;
    param_len[2] = (function && function[0]) ? strlen(function) : 0;
    param_len[3] = (line > 0) ? sizeof(param_line) : 0;
    param_len[4] = (area_file && area_file[0]) ? strlen(area_file) : 0;
    param_len[5] = (area_line > 0) ? sizeof(param_area_line) : 0;
    param_len[6] = (character && character[0]) ? strlen(character) : 0;
    param_len[7] = (character_room > 0) ? sizeof(param_char_room) : 0;
    param_len[8] = (victim && victim[0]) ? strlen(victim) : 0;
    param_len[9] = (victim_room > 0) ? sizeof(param_vic_room) : 0;
    param_len[10] = (message && message[0]) ? strlen(message) : 0;

    sql_connect(&db_logfile);
    res = PQexecParams(db_logfile.dbc, sql, 11, NULL, param_val, param_len, param_bin, 0);
    st = PQresultStatus(res);
    if (st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE)
    {
        // In case of an error, we defined a special logging level that does NOT
        // attempt to log to SQL, so we can still emit errors if the database is
        // not available.
        log_nosql("Cannot insert log message: %s", PQerrorMessage(db_logfile.dbc));
    }
    PQclear(res);
}

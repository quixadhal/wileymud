#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcre.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <time.h>
#include <ctype.h>
#include "global.h"
#include "bug.h"
#include "utils.h"
#include "comm.h" // for proper_exit() and MUD_HALT
#include "stringmap.h"
#include "modify.h"     // for page_string()
#include "multiclass.h" // for GetMaxLevel() used by ISIMMORTAL()

#include "sql.h"
#define _HELP_C
#include "help.h"

struct help_keyword *help_keywords = NULL;
stringMap *help_messages = NULL;
int help_message_count = 0;
int help_keyword_count = 0;

void setup_help_table(void)
{
    PGresult *res = NULL;
    ExecStatusType st = (ExecStatusType) 0;
    const char *sqla = "CREATE TABLE IF NOT EXISTS help_messages ( "
                 "    updated TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT now(), "
                 "    immortal BOOLEAN NOT NULL DEFAULT false, "
                 "    set_by TEXT NOT NULL DEFAULT 'SYSTEM', "
                 "    id INTEGER PRIMARY KEY NOT NULL, "
                 "    message TEXT "
                 "); ";
    const char *sqlb = "CREATE TABLE IF NOT EXISTS help_keywords ( "
                 "    id INTEGER NOT NULL REFERENCES help_messages(id), "
                 "    keyword TEXT PRIMARY KEY NOT NULL "
                 "); ";
    // We need to verify that our magic keywords are present, and that
    // entries exist for them.
    const char *sql2 = "SELECT id FROM help_keywords WHERE lower(keyword) = lower($1);";
    const char *sql2b = "SELECT id FROM help_messages WHERE id = $1;";

    // We might need this if we have an issue...
    const char *sql3 = "SELECT coalesce(MAX(id), 0) FROM help_messages;";

    // If anything wasn't there, we'll need to put placeholders in...
    const char *sql4a = "INSERT INTO help_messages (id, message, immortal) VALUES ($1,$2,$3) "
                  "ON CONFLICT DO NOTHING;";
    const char *sql4b = "INSERT INTO help_keywords (id, keyword) VALUES ($1,$2) "
                  "ON CONFLICT DO NOTHING;";

    const char *param_val[3];
    int param_len[3];
    int param_bin[3] = {0, 0, 0};
    const char *keywords[3] = {"HELP", "CONTENTS", "WIZHELP"};
    const char *wizzness[3] = {"0", "0", "1"};
    const char *messages[3] = {// HELP
                         "The help system is not fully installed, complain to your admin!\r\n",
                         // CONTENTS
                         "The help system is empty, your admin is lazy!\r\n",
                         // WIZHELP
                         "You forgot to install the help files, dumbass.\r\n"};
    int rows = 0;
    int columns = 0;

    sql_connect(&db_wileymud);
    res = PQexec(db_wileymud.dbc, sqla);
    st = PQresultStatus(res);
    if (st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE)
    {
        log_fatal("Cannot create help_messages table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);

    res = PQexec(db_wileymud.dbc, sqlb);
    st = PQresultStatus(res);
    if (st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE)
    {
        log_fatal("Cannot create help_keywords table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }

    // Make sure any hard-coded help entries the code uses are in place!
    for (int i = 0; i < 3; i++)
    {
        char tmp[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
        int this_keyword_id = 0;
        int this_message_id = 0;

        param_val[0] = keywords[i][0] ? keywords[i] : NULL;
        param_len[0] = keywords[i][0] ? strlen(keywords[i]) : 0;
        res = PQexecParams(db_wileymud.dbc, sql2, 1, NULL, param_val, param_len, param_bin, 0);
        st = PQresultStatus(res);
        if (st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE)
        {
            log_fatal("Cannot fetch keyword id for %s from help_messages table: %s", keywords[i],
                      PQerrorMessage(db_wileymud.dbc));
            PQclear(res);
            proper_exit(MUD_HALT);
        }
        rows = PQntuples(res);
        columns = PQnfields(res);
        if (rows < 1 || columns < 1)
        {
            // We did NOT find the keyword we needed, fudge one?
            PQclear(res);
            log_sql("Did not find keyword id for %s", keywords[i]);
        }
        else
        {
            this_keyword_id = (int)atoi(PQgetvalue(res, 0, 0));
            PQclear(res);
            log_sql("Found keyword id %d for %s", this_keyword_id, keywords[i]);

            // We found the keyword for our message, now we need to ensure the message is there.
            snprintf(tmp, MAX_INPUT_LENGTH, "%d", i);
            param_val[0] = tmp[0] ? tmp : NULL;
            param_len[0] = tmp[0] ? strlen(tmp) : 0;
            res = PQexecParams(db_wileymud.dbc, sql2b, 1, NULL, param_val, param_len, param_bin, 0);
            st = PQresultStatus(res);
            if (st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE)
            {
                log_fatal("Cannot fetch keyword id for %d from help_messages table: %s", i,
                          PQerrorMessage(db_wileymud.dbc));
                PQclear(res);
                proper_exit(MUD_HALT);
            }
            rows = PQntuples(res);
            columns = PQnfields(res);
            if (rows < 1 || columns < 1)
            {
                // We had a keyword, but no message... grrrr....
                PQclear(res);
                log_sql("Did not find message for id %d from %s", this_keyword_id, keywords[i]);
            }
            else
            {
                this_message_id = (int)atoi(PQgetvalue(res, 0, 0));
                PQclear(res);
                log_sql("Found message id %d for keyword id %d from %s", this_message_id, this_keyword_id, keywords[i]);
            }
        }

        if (this_keyword_id < 1 || this_message_id < 1)
        {
            // If either id is zero, we failed to find something.
            // In either case, we should get the MAX(id) of the message table

            res = PQexec(db_wileymud.dbc, sql3);
            st = PQresultStatus(res);
            if (st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE)
            {
                log_fatal("Cannot get max id from messages table: %s", PQerrorMessage(db_wileymud.dbc));
                PQclear(res);
                proper_exit(MUD_HALT);
            }
            rows = PQntuples(res);
            columns = PQnfields(res);
            if (rows < 1 || columns < 1)
            {
                // We can't get a MAX(id), this is bad...
                log_fatal("Invalid max id from messages table: %s", PQerrorMessage(db_wileymud.dbc));
                PQclear(res);
                proper_exit(MUD_HALT);
            }
            else
            {
                this_message_id = (int)atoi(PQgetvalue(res, 0, 0));
                PQclear(res);
            }

            if (this_keyword_id < 1)
            {
                // Now, if we also didn't have a keyword, we'll just use the MAX(id) + 1
                // If we had one, cool.. use it.
                this_keyword_id = this_message_id + 1;
                log_sql("Inserting message id %d for keyword %s", this_keyword_id, keywords[i]);

                snprintf(tmp, MAX_INPUT_LENGTH, "%d", this_keyword_id);
                param_val[0] = tmp[0] ? tmp : NULL;
                param_len[0] = tmp[0] ? strlen(tmp) : 0;
                param_val[1] = messages[i][0] ? messages[i] : NULL;
                param_len[1] = messages[i][0] ? strlen(messages[i]) : 0;
                param_val[2] = wizzness[i][0] ? wizzness[i] : NULL;
                param_len[2] = wizzness[i][0] ? strlen(wizzness[i]) : 0;
                res = PQexecParams(db_wileymud.dbc, sql4a, 3, NULL, param_val, param_len, param_bin, 0);
                st = PQresultStatus(res);
                if (st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE)
                {
                    log_fatal("Cannot insert message %d to help_messages table: %s", this_keyword_id,
                              PQerrorMessage(db_wileymud.dbc));
                    PQclear(res);
                    proper_exit(MUD_HALT);
                }
                PQclear(res);

                log_sql("Inserting keyword %d to mach...", this_keyword_id);
                snprintf(tmp, MAX_INPUT_LENGTH, "%d", this_keyword_id);
                param_val[0] = tmp[0] ? tmp : NULL;
                param_len[0] = tmp[0] ? strlen(tmp) : 0;
                param_val[1] = keywords[i][0] ? keywords[i] : NULL;
                param_len[1] = keywords[i][0] ? strlen(keywords[i]) : 0;

                res = PQexecParams(db_wileymud.dbc, sql4b, 2, NULL, param_val, param_len, param_bin, 0);
                st = PQresultStatus(res);
                if (st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE)
                {
                    log_fatal("Cannot insert keyword %d to help_keywords table: %s", i,
                              PQerrorMessage(db_wileymud.dbc));
                    PQclear(res);
                    proper_exit(MUD_HALT);
                }
                PQclear(res);
            }
            else
            {
                // We had one, but we still have to restore the missing message entry...
                // This really should never happen because of the foreign key contraint.
                log_sql("Inserting message id %d for keyword %s", this_keyword_id, keywords[i]);

                snprintf(tmp, MAX_INPUT_LENGTH, "%d", this_keyword_id);
                param_val[0] = tmp[0] ? tmp : NULL;
                param_len[0] = tmp[0] ? strlen(tmp) : 0;
                param_val[1] = messages[i][0] ? messages[i] : NULL;
                param_len[1] = messages[i][0] ? strlen(messages[i]) : 0;
                param_val[2] = wizzness[i][0] ? wizzness[i] : NULL;
                param_len[2] = wizzness[i][0] ? strlen(wizzness[i]) : 0;
                res = PQexecParams(db_wileymud.dbc, sql4a, 3, NULL, param_val, param_len, param_bin, 0);
                st = PQresultStatus(res);
                if (st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE)
                {
                    log_fatal("Cannot insert message %d to help_messages table: %s", this_keyword_id,
                              PQerrorMessage(db_wileymud.dbc));
                    PQclear(res);
                    proper_exit(MUD_HALT);
                }
                PQclear(res);
            }
        }
    }
}

void load_help(void)
{
    PGresult *res = NULL;
    ExecStatusType st = (ExecStatusType) 0;
    const char *sql = "SELECT count(*) FROM help_messages;";
    const char *sql2 = "SELECT count(*) FROM help_keywords;";
    const char *sql3 = "SELECT extract('epoch' FROM updated) AS updated, "
                       "immortal::integer, "
                       "set_by, id, message "
                       "FROM help_messages;";
    const char *sql4 = "SELECT id, keyword "
                       "FROM help_keywords "
                       "ORDER BY lower(keyword) ASC;";
    int rows = 0;
    int columns = 0;

    sql_connect(&db_wileymud);
    res = PQexec(db_wileymud.dbc, sql);
    st = PQresultStatus(res);
    if (st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE)
    {
        log_fatal("Cannot get count of messages in help table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    rows = PQntuples(res);
    columns = PQnfields(res);
    if (rows > 0 && columns > 0)
    {
        help_message_count = (int)atoi(PQgetvalue(res, 0, 0));
    }
    else
    {
        log_fatal("Invalid result set from help table!");
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);

    res = PQexec(db_wileymud.dbc, sql2);
    st = PQresultStatus(res);
    if (st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE)
    {
        log_fatal("Cannot get count of keywords in help table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    rows = PQntuples(res);
    columns = PQnfields(res);
    if (rows > 0 && columns > 0)
    {
        help_keyword_count = (int)atoi(PQgetvalue(res, 0, 0));
    }
    else
    {
        log_fatal("Invalid result set from help table!");
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);

    stringmap_destroy(help_messages);
    help_messages = stringmap_init();

    res = PQexec(db_wileymud.dbc, sql3);
    st = PQresultStatus(res);
    if (st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE)
    {
        log_fatal("Cannot get messages from help table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    rows = PQntuples(res);
    columns = PQnfields(res);
    if (rows > 0 && columns > 4)
    {
        for (int i = 0; i < rows; i++)
        {
            struct help_message help_me;

            help_me.updated = (time_t)atol(PQgetvalue(res, i, 0));
            help_me.immortal = (int)atoi(PQgetvalue(res, i, 1));
            strlcpy(help_me.set_by, PQgetvalue(res, i, 2), MAX_INPUT_LENGTH);
            help_me.id = (int)atoi(PQgetvalue(res, i, 3));
            strlcpy(help_me.message, PQgetvalue(res, i, 4), MAX_STRING_LENGTH);

            // stringmap already is case insensitive, and will replace values
            // for existing keys.  In this case, our keys are numbers as char * strings.
            stringmap_add(help_messages, PQgetvalue(res, i, 3), (void *)&help_me, sizeof(struct help_message));
        }
    }
    else
    {
        log_fatal("Invalid result set from help table!");
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);

    if (help_keywords)
        free(help_keywords);
    help_keywords = (struct help_keyword *)calloc(help_keyword_count, sizeof(struct help_keyword));

    res = PQexec(db_wileymud.dbc, sql4);
    st = PQresultStatus(res);
    if (st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE)
    {
        log_fatal("Cannot get keywords from help table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    rows = PQntuples(res);
    columns = PQnfields(res);
    if (rows > 0 && columns > 1)
    {
        for (int i = 0; i < rows; i++)
        {
            help_keywords[i].id = (int)atoi(PQgetvalue(res, i, 0));
            strlcpy(help_keywords[i].keyword, PQgetvalue(res, i, 1), MAX_INPUT_LENGTH);
        }
    }
    else
    {
        log_fatal("Invalid result set from help table!");
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);
}

// ka is the thing we are looking for, kb is one element in the thing we are searching
static int _keyword_cmp(const void *a, const void *b)
{
    struct help_keyword *ka = (struct help_keyword *)a;
    struct help_keyword *kb = (struct help_keyword *)b;

    return strncasecmp(ka->keyword, kb->keyword, strlen(ka->keyword));
}

static int _exact_keyword_cmp(const void *a, const void *b)
{
    struct help_keyword *ka = (struct help_keyword *)a;
    struct help_keyword *kb = (struct help_keyword *)b;

    return strcasecmp(ka->keyword, kb->keyword);
}

int find_help_by_keyword(const char *keyword)
{
    struct help_keyword needle;
    struct help_keyword *result;

    strlcpy(needle.keyword, keyword, MAX_INPUT_LENGTH);
    needle.id = 0;

    result = (struct help_keyword *)bsearch(&needle, help_keywords, help_keyword_count, sizeof(struct help_keyword), _exact_keyword_cmp);
    if (!result)
        result = (struct help_keyword *)bsearch(&needle, help_keywords, help_keyword_count, sizeof(struct help_keyword), _keyword_cmp);

    return result ? result->id : -1;
}

struct help_message *get_help_by_id(const int id)
{
    char tmp[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    snprintf(tmp, MAX_INPUT_LENGTH, "%d", id);
    return (struct help_message *)stringmap_find(help_messages, tmp);
}

int do_help(struct char_data *ch, const char *argument, int cmd)
{
    struct help_message *help_me = NULL;
    int id = 0;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (!ch->desc)
        return TRUE;

    for (; isspace(*argument); argument++)
        ;

    if (IS_IMMORTAL(ch) && !strncasecmp(argument, "--reload", 8))
    {
        cprintf(ch, "Reloading help system from SQL...");
        load_help();
        cprintf(ch, "done.\r\n");
        return TRUE;
    }
    if (!strncasecmp(argument, "--list", 8))
    {
        char result[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
        int column = 0;

        scprintf(result, MAX_STRING_LENGTH, "Help is available for...\r\n");
        for (int i = 0; i < help_keyword_count; i++)
        {
            help_me = get_help_by_id(help_keywords[i].id);
            if (help_me && !IS_IMMORTAL(ch) && help_me->immortal)
                continue;

            // Yes, this will print out duplicates for those cases where one help
            // entry shares multiple keywords... for now.
            if (strlen(help_keywords[i].keyword) > 18 && column < 3)
            {
                scprintf(result, MAX_STRING_LENGTH, "%-36.36s  ", help_keywords[i].keyword);
                column++;
            }
            else
            {
                scprintf(result, MAX_STRING_LENGTH, "%-18.18s ", help_keywords[i].keyword);
            }
            if (++column > 3 || i == (help_keyword_count - 1))
            {
                column = 0;
                scprintf(result, MAX_STRING_LENGTH, "\r\n");
            }
        }
        scprintf(result, MAX_STRING_LENGTH, "\r\n");
        page_string(ch->desc, result, 0);
        return TRUE;
    }

    id = find_help_by_keyword(*argument ? argument : "contents");
    if (id != -1)
        help_me = get_help_by_id(id);

    if (help_me && (IS_IMMORTAL(ch) || !(help_me->immortal)))
        page_string(ch->desc, help_me->message, 0);
    else
        cprintf(ch, "No help available for that!\r\n");
    return TRUE;
}

int do_wizhelp(struct char_data *ch, const char *argument, int cmd)
{
    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (!ch->desc)
        return TRUE;

    if (!IS_IMMORTAL(ch))
    {
        cprintf(ch, "You are not a wizard, Harry.  Try normal help.\r\n");
        return TRUE;
    }
    else
    {
        do_help(ch, argument, cmd);
        return TRUE;
    }
    return TRUE;
}

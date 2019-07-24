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
#include "comm.h"       // for proper_exit() and MUD_HALT
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

void setup_help_table(void) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    char *sql = "CREATE TABLE IF NOT EXISTS help_messages ( "
                "    updated TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT now(), "
                "    immortal BOOLEAN NOT NULL DEFAULT false, "
                "    set_by TEXT NOT NULL DEFAULT 'SYSTEM', "
                "    id INTEGER PRIMARY KEY NOT NULL, "
                "    message TEXT "
                "); ";
    char *sql2 = "CREATE TABLE IF NOT EXISTS help_keywords ( "
                "    id INTEGER NOT NULL REFERENCES help_messages(id), "
                "    keyword TEXT PRIMARY KEY NOT NULL "
                "); ";
    char *sql3 = "INSERT INTO help_messages (id, message, immortal) VALUES ($1,$2,$3) "
                 "ON CONFLICT DO NOTHING;";
                 //"ON CONFLICT (id) DO UPDATE SET updated = now(), message = $2, immortal = $3;";
    char *sql4 = "INSERT INTO help_keywords (id, keyword) VALUES ($1,$2) ON CONFLICT DO NOTHING;";
    const char *param_val[3];
    int param_len[3];
    int param_bin[3] = {0,0,0};
    char *keywords[3] = { "help", "contents", "wizhelp" };
    char *wizzness[3] = { "0", "0", "1" };
    char *messages[3] = {
        // HELP
        "The following help is available:\r\n"
        "\r\n"
        "HELP help       - This text.\r\n"
        "HELP            - List the available commands.\r\n"
        "HELP --list     - List the available commands by keyword.\r\n"
        "HELP --reload   - Immortal only, forces the help data to be refreshed.\r\n"
        "\r\n"
        "Help searches for a partial match of the entered word, including any\r\n"
        "spaces that may follow the word.\r\n"
        "\r\n"
        "Example:\r\n"
        " > 'help magic mis'\r\n"
        "    will find the help text for the magic missile spell.\r\n"
        "\r\n"
        " > 'help mag '\r\n"
        "   will not match anything.\r\n"
        "\r\n"
        " > 'help mag'\r\n"
        "   will match 'magic user' or 'magic missile' depending on first\r\n"
        "   occurrence in the help file.\r\n",
        // CONTENTS
        "                          ---=>>> WileyMUD III <<<=---\r\n"
        "\r\n"
        " --Communication--    --Equipment--       --Movement--      --Experience--   \r\n"
        "  ' say              wear, wield        n,s,e,w,u,d        practice <skill>  \r\n"
        "  , group tell       remove, hold       enter, leave       gain              \r\n"
        "  \" tell             use <item>         follow, flee       prac <m/c/w/t/r>  \r\n"
        "  : emote            recite <scroll>    land                *add known to    \r\n"
        "  @ whizz tell       quaff <potion>     sit, stand          show only your   \r\n"
        "    shout, whisper                      rest, sleep         known skills.    \r\n"
        "\r\n"
        "     --Groups--         --Shops--          --Combat--         --Food--       \r\n"
        "  follow             buy, sell          consider, kill     eat, taste        \r\n"
        "  group, grep        list               cast, swat         drink, sip        \r\n"
        "  gtell, split       value              kick, bash                           \r\n"
        "                     rent, offer        rescue, backstab                     \r\n"
        "                                        flee                                 \r\n"
        "\r\n"
        "     --Items--         --Corpses--       --Environment--     --Personal--    \r\n"
        "  get, drop, put     get all corpse     look               score, inventory  \r\n"
        "  buy, sell          bury               look in            nosummon,noshout  \r\n"
        "  give               desecrate          search             noteleport        \r\n"
        "  steal, pick                                              help              \r\n"
        "  open, close                                              who               \r\n"
        "  lock, unlock                                             quit (careful!)   \r\n",
        // WIZHELP
        "The following privileged comands MAY be available to you:\r\n"
        "\r\n"
        "  force      transfer   goto       shutdow    string     stat       load     \r\n"
        "  purge      shutdown   at         snoop      advance    snowball   reroll   \r\n"
        "  restore    switch     users      wizhelp    slay       nohassle   wall     \r\n"
        "  stealth    pset       @          wizlock    highfive   world      spells   \r\n"
        "  show       debug      invisible  mkzone     wiznet     rentmode   pretitle \r\n"
        "  logs       whod       restoreall players    reset      event      zpurge   \r\n"
        "  ticks      setreboot  home                                                 \r\n"
    };

    sql_connect(&db_wileymud);
    res = PQexec(db_wileymud.dbc, sql);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create help_messages table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);

    res = PQexec(db_wileymud.dbc, sql2);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create help_keywords table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }

    // Make sure any hard-coded help entries the code uses are in place!
    for(int i = 0; i < 3; i++) {
        char tmp[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

        snprintf(tmp, MAX_INPUT_LENGTH, "%d", i);
        param_val[0] = tmp[0] ? tmp : NULL;
        param_len[0] = tmp[0] ? strlen(tmp) : 0;
        param_val[1] = messages[i][0] ? messages[i] : NULL;
        param_len[1] = messages[i][0] ? strlen(messages[i]) : 0;
        param_val[2] = wizzness[i][0] ? wizzness[i] : NULL;
        param_len[2] = wizzness[i][0] ? strlen(wizzness[i]) : 0;

        res = PQexecParams(db_wileymud.dbc, sql3, 3, NULL, param_val, param_len, param_bin, 0);
        st = PQresultStatus(res);
        if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
            log_fatal("Cannot insert message %d to help_messages table: %s", i, PQerrorMessage(db_wileymud.dbc));
            PQclear(res);
            proper_exit(MUD_HALT);
        }
        PQclear(res);

        param_val[1] = keywords[i][0] ? keywords[i] : NULL;
        param_len[1] = keywords[i][0] ? strlen(keywords[i]) : 0;

        res = PQexecParams(db_wileymud.dbc, sql4, 2, NULL, param_val, param_len, param_bin, 0);
        st = PQresultStatus(res);
        if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
            log_fatal("Cannot insert keyword %d to help_keywords table: %s", i, PQerrorMessage(db_wileymud.dbc));
            PQclear(res);
            proper_exit(MUD_HALT);
        }
        PQclear(res);
    }
}

void load_help(void) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    const char *sql  = "SELECT count(*) FROM help_messages;";
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
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot get count of messages in help table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    rows = PQntuples(res);
    columns = PQnfields(res);
    if(rows > 0 && columns > 0) {
        help_message_count = (int) atoi(PQgetvalue(res,0,0));
    } else {
        log_fatal("Invalid result set from help table!");
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);

    res = PQexec(db_wileymud.dbc, sql2);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot get count of keywords in help table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    rows = PQntuples(res);
    columns = PQnfields(res);
    if(rows > 0 && columns > 0) {
        help_keyword_count = (int) atoi(PQgetvalue(res,0,0));
    } else {
        log_fatal("Invalid result set from help table!");
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);

    stringmap_destroy(help_messages);
    help_messages = stringmap_init();

    res = PQexec(db_wileymud.dbc, sql3);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot get messages from help table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    rows = PQntuples(res);
    columns = PQnfields(res);
    if(rows > 0 && columns > 4) {
        for(int i = 0; i < rows; i++) {
            struct help_message help_me;

            help_me.updated = (time_t) atol(PQgetvalue(res,i,0));
            help_me.immortal = (int) atoi(PQgetvalue(res,i,1));
            strlcpy(help_me.set_by, PQgetvalue(res,i,2), MAX_INPUT_LENGTH);
            help_me.id = (int) atoi(PQgetvalue(res,i,3));
            strlcpy(help_me.message, PQgetvalue(res,i,4), MAX_STRING_LENGTH);

            // stringmap already is case insensitive, and will replace values
            // for existing keys.  In this case, our keys are numbers as char * strings.
            stringmap_add(help_messages, PQgetvalue(res,i,3), (void *)&help_me, sizeof(struct help_message));
        }
    } else {
        log_fatal("Invalid result set from help table!");
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);


    if(help_keywords)
        free(help_keywords);
    help_keywords = (struct help_keyword *)calloc(help_keyword_count, sizeof(struct help_keyword));

    res = PQexec(db_wileymud.dbc, sql4);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot get keywords from help table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    rows = PQntuples(res);
    columns = PQnfields(res);
    if(rows > 0 && columns > 1) {
        for(int i = 0; i < rows; i++) {
            help_keywords[i].id = (int) atoi(PQgetvalue(res,i,0));
            strlcpy(help_keywords[i].keyword, PQgetvalue(res,i,1), MAX_INPUT_LENGTH);
        }
    } else {
        log_fatal("Invalid result set from help table!");
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);
}

// ka is the thing we are looking for, kb is one element in the thing we are searching
static int _keyword_cmp(const void *a, const void *b) {
    struct help_keyword *ka = (struct help_keyword *) a;
    struct help_keyword *kb = (struct help_keyword *) b;

    return strncasecmp(ka->keyword, kb->keyword, strlen(ka->keyword));
}

int find_help_by_keyword(const char *keyword) {
    struct help_keyword needle;
    struct help_keyword *result;

    strlcpy(needle.keyword, keyword, MAX_INPUT_LENGTH);
    needle.id = 0;

    result = bsearch(&needle, help_keywords, help_keyword_count,
                     sizeof(struct help_keyword), _keyword_cmp);

    return result ? result->id : -1;
}

struct help_message *get_help_by_id(const int id) {
    char tmp[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    snprintf(tmp, MAX_INPUT_LENGTH, "%d", id);
    return stringmap_find(help_messages, tmp);
}

void do_help(struct char_data *ch, const char *argument, int cmd)
{
    struct help_message *help_me = NULL;
    int id = 0;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (!ch->desc)
	return;

    for (; isspace(*argument); argument++);

    if(IS_IMMORTAL(ch) && !strncasecmp(argument, "--reload", 8)) {
        cprintf(ch, "Reloading help system from SQL...");
        load_help();
        cprintf(ch, "done.\r\n");
        return;
    }
    if(!strncasecmp(argument, "--list", 8)) {
        char result[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
        int column = 0;

        scprintf(result, MAX_STRING_LENGTH, "Help is available for...\r\n");
        for(int i = 0; i < help_keyword_count; i++) {
            help_me = get_help_by_id(help_keywords[i].id);
            if(help_me && !IS_IMMORTAL(ch) && help_me->immortal)
                continue;

            // Yes, this will print out duplicates for those cases where one help
            // entry shares multiple keywords... for now.
            scprintf(result, MAX_STRING_LENGTH, "%-12.12s", help_keywords[i].keyword);
            if(++column > 5 || i == (help_keyword_count - 1)) {
                column = 0;
                scprintf(result, MAX_STRING_LENGTH, "\r\n");
            }
        }
        scprintf(result, MAX_STRING_LENGTH, "\r\n");
        page_string(ch->desc, result, 0);
        return;
    }

    id = find_help_by_keyword( *argument ? argument : "contents" );
    if(id != -1)
        help_me = get_help_by_id(id);

    if(help_me && ( IS_IMMORTAL(ch) || !(help_me->immortal)))
        page_string(ch->desc, help_me->message, 0);
    else
        cprintf(ch, "No help available for that!\r\n");
}

void do_wizhelp(struct char_data *ch, const char *argument, int cmd)
{
    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (!ch->desc)
	return;

    if (!IS_IMMORTAL(ch)) {
        cprintf(ch, "You are not a wizard, Harry.  Try normal help.\r\n");
        return;
    } else {
        do_help(ch, argument, cmd);
        return;
    }
}


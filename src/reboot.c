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
#include "utils.h"
#include "comm.h"           // for proper_exit()
#include "interpreter.h"    // for WizLock
#include "db.h"             // for time_info and weather_info
#include "weather.h"        // for save_weather()
#include "sql.h"
#ifdef I3
#include "i3.h"             // for i3_log_dead()
#endif
#define _REBOOT_C
#include "reboot.h"

struct reboot_data          reboot;

void setup_reboot_table(void) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    // frequency indicates when the next reboot should be scheduled after the
    // current one happens.  We use PostgreSQL's interval type to store this,
    // but the UI will have simple keywords such as
    // 'monthly', 'weekly', 'daily', 'twice' for twice a day.
    // We may allow an addition integer argument to mean every N periods,
    // so "setreboot daily 3" would mean reboot every 3 days.
    // frequency is a nullable column, so one-shot reboots can be done, but
    // the UI would be clunky unless we feed the argument directly to SQL and
    // the user knows what PostgreSQL supports for interval strings.
    char *sql = "CREATE TABLE IF NOT EXISTS reboot ( "
                "    updated TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT now(), "
                "    enabled BOOLEAN NOT NULL DEFAULT false, "
                "    next_reboot TIMESTAMP WITH TIME ZONE DEFAULT (now() + '1 day'::interval), "
                "    frequency INTERVAL DEFAULT '1 day'::interval, "
                "    set_by TEXT NOT NULL DEFAULT 'SYSTEM'"
                "); ";
    char *sql2 = "SELECT count(*) FROM reboot;";
    char *sql3 = "INSERT INTO reboot (enabled) VALUES (false);";
    char *sql4 = "DELETE FROM reboot WHERE updated <> (SELECT max(UPDATED) FROM reboot);";
    int rows = 0;
    int columns = 0;
    int count = 0;

    sql_connect(&db_wileymud);
    res = PQexec(db_wileymud.dbc, sql);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create reboot table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);

    res = PQexec(db_wileymud.dbc, sql2);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot get row count of reboot table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    rows = PQntuples(res);
    columns = PQnfields(res);
    if(rows > 0 && columns > 0) {
        count = atoi(PQgetvalue(res,0,0));
    } else {
        log_fatal("Invalid result set from row count!");
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);

    if(count < 1) {
        // Insert our default value.
        res = PQexec(db_wileymud.dbc, sql3);
        st = PQresultStatus(res);
        if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
            log_fatal("Cannot insert placeholder row to reboot table: %s", PQerrorMessage(db_wileymud.dbc));
            PQclear(res);
            proper_exit(MUD_HALT);
        }
        PQclear(res);
    } else if(count > 1) {
        // That shouldn't happen... fix it!
        log_error("There are %d rows in the reboot table, instead of just ONE!", count);
        res = PQexec(db_wileymud.dbc, sql4);
        st = PQresultStatus(res);
        if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
            log_fatal("Cannot remove old rows from reboot table: %s", PQerrorMessage(db_wileymud.dbc));
            PQclear(res);
            proper_exit(MUD_HALT);
        }
        PQclear(res);
    }
}

void load_reboot(void) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    const char *sql =   "SELECT extract('epoch' FROM updated) AS updated, "
                        "enabled::integer, "
                        "extract('epoch' FROM next_reboot) AS next_reboot, "
                        "extract('epoch' FROM frequency) AS frequencey, "
                        "set_by, "
                        "next_reboot AS next_reboot_text " // We grab this as text for display
                        "FROM reboot LIMIT 1;";
    int rows = 0;
    int columns = 0;
    struct reboot_data the_boot = { 0, 0, 0, 0, "", 0, "" };

    sql_connect(&db_wileymud);
    res = PQexec(db_wileymud.dbc, sql);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot get reboot data from reboot table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }

    rows = PQntuples(res);
    columns = PQnfields(res);
    if(rows > 0 && columns > 5) {
        log_boot("  Loading reboot data from SQL database.");
        the_boot.updated = (time_t) atol(PQgetvalue(res,0,0));
        the_boot.enabled = (int) atoi(PQgetvalue(res,0,1));
        the_boot.next_reboot = (time_t) atol(PQgetvalue(res,0,2));
        the_boot.frequency = (time_t) atol(PQgetvalue(res,0,3));
        strlcpy(the_boot.set_by, PQgetvalue(res,0,4), MAX_INPUT_LENGTH);
        the_boot.last_message = (time_t) time(0); // temporary field for reboot message spam
        strlcpy(the_boot.next_reboot_text, PQgetvalue(res,0,5), MAX_INPUT_LENGTH); // temp
    } else {
        log_fatal("Invalid result set from reboot table!");
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);

    reboot = the_boot;
}

int set_first_reboot(void) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    const char *sql =   "UPDATE reboot "
                        "SET next_reboot = now() + frequency "
                        "WHERE frequency IS NOT NULL;";

    sql_connect(&db_wileymud);
    res = PQexec(db_wileymud.dbc, sql);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot set next_reboot in reboot table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        //proper_exit(MUD_HALT);
        return 0;
    }
    PQclear(res);
    log_boot("Next scheduled reboot set in SQL database.");
    load_reboot();
    return 1;
}

int set_next_reboot(void) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    const char *sql =   "UPDATE reboot "
                        "SET next_reboot = next_reboot + frequency "
                        "WHERE frequency IS NOT NULL;";

    sql_connect(&db_wileymud);
    res = PQexec(db_wileymud.dbc, sql);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot update next_reboot in reboot table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        //proper_exit(MUD_HALT);
        return 0;
    }
    PQclear(res);
    log_boot("Next scheduled reboot updated in SQL database.");
    load_reboot();
    return 1;
}

int toggle_reboot(struct char_data *ch) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    const char *sql =   "UPDATE reboot SET enabled = NOT enabled, "
                        "updated = now(), "
                        "set_by = $1;";
    const char *param_val[1];
    int param_len[1];
    int param_bin[1] = {0};
    char set_by[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    strlcpy(set_by, GET_NAME(ch), MAX_INPUT_LENGTH);
    param_val[0] = (set_by[0]) ? set_by : NULL;
    param_len[0] = (set_by[0]) ? strlen(set_by) : 0;

    sql_connect(&db_wileymud);
    res = PQexecParams(db_wileymud.dbc, sql, 1, NULL, param_val, param_len, param_bin, 0);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_error("Cannot toggle reboot in reboot table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        return 0;
        //proper_exit(MUD_HALT);
    }
    PQclear(res);
    load_reboot();
    return 1;
}

int set_reboot_interval(struct char_data *ch, const char *mode, int number) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    const char *sql =   "UPDATE reboot SET frequency = $1::interval, "
                        "updated = now(), "
                        "set_by = $2;";
    const char *param_val[2];
    int param_len[2];
    int param_bin[2] = {0,0};
    char set_by[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char interval[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    int next_interval = 0;

    if(!strcasecmp(mode, "week")) {
        next_interval = (60 * 60 * 24 * 7 * number);
    } else if(!strcasecmp(mode, "day")) {
        next_interval = (60 * 60 * 24 * number);
    } else if(!strcasecmp(mode, "hour")) {
        next_interval = (60 * 60 * number);
    }

    snprintf(interval, MAX_INPUT_LENGTH, "%d %s", number, mode);
    param_val[0] = (interval[0]) ? interval : NULL;
    param_len[0] = (interval[0]) ? strlen(interval) : 0;
    strlcpy(set_by, GET_NAME(ch), MAX_INPUT_LENGTH);
    param_val[1] = (set_by[0]) ? set_by : NULL;
    param_len[1] = (set_by[0]) ? strlen(set_by) : 0;

    sql_connect(&db_wileymud);
    res = PQexecParams(db_wileymud.dbc, sql, 2, NULL, param_val, param_len, param_bin, 0);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_error("Cannot adjust reboot frequency: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        return 0;
        //proper_exit(MUD_HALT);
    }
    PQclear(res);
    load_reboot();

    if((reboot.next_reboot < (time(0) + next_interval)) ||
       (reboot.next_reboot > (time(0) + (2 * next_interval)))) {
        // If our next reboot is 0 (NULL) or less than our interval, set it
        // to be one interval's worth from now.
        set_first_reboot();
        load_reboot();
    }
    return 1;
}

void check_reboot(void)
{
    time_t                                  now;
    time_t                                  time_left;
    struct tm                              *t_info = NULL;
    char                                   *tmstr = NULL;

    if (DEBUG > 2)
	log_info("called %s with no arguments", __PRETTY_FUNCTION__);

    now = time(0);
    time_left = reboot.next_reboot - now;

    if (reboot.enabled == FALSE) {
        if (now >= reboot.next_reboot) {
            set_next_reboot(); // Here, we schedule the next reboot, if any...
        }
        return;
    }

    // More than an hour away, nothing interesting to do yet.
    if (time_left > (60 * 60))
        return;

    t_info = localtime(&now);
    tmstr = asctime(t_info);
    *(tmstr + strlen(tmstr) - 1) = '\0';

    // Note that this is only run every PULSE_REBOOT ticks, which is
    // currently set at 599, or roughly once every 2.5 minutes.  So
    // the finer grained messages won't work the way you might expect.
    //
    // The cleanest way to fix this is to make PULSE_REBOOT a proper
    // integer rather than a #define, in whic case we can adjust the
    // granularity.. since if it's an hour away, you probably only want
    // a message every 10-15 minutes, but if it's a minute away, every
    // 10 seconds is reasonable.

    allprintf("\x007\r\nBroadcast message from SYSTEM (tty0) %s...\r\n\r\n", tmstr);

    if (now >= reboot.next_reboot) {
        // Time to die!
        allprintf("Automatic reboot.  Come back in a few minutes!\r\n");
        allprintf("\x007The system is going down NOW !!\r\n\x007\r\n");
        log_boot("Rebooting!");
        set_next_reboot(); // Here, we schedule the next reboot, if any...
        diku_shutdown = diku_reboot = 1;
    } else if (time_left <= 60) {
        // 60 seconds or less
        WizLock = 1;
        if(now < (reboot.last_message + 15)) {
            allprintf("Automatic reboot.  Game is now Whizz-Locked!\r\n");
            allprintf("\x007The system is going DOWN in %ld seconds!  You are all going to DIE!\r\n", time_left);
            reboot.last_message = now;
        }
    } else if (time_left <= (60 * 10)) {
        // 10 minutes or less
        WizLock = 1;
        if(now < (reboot.last_message + (60 * 3))) {
            allprintf("Automatic reboot.  Game is now Whizz-Locked!\r\n");
            allprintf("\x007The system is going DOWN in %ld minutes!  You are NOT prepared!\r\n", MIN(1, (time_left / 60)));
            reboot.last_message = now;
        }
    } else {
        // One hour to go
        if(now < (reboot.last_message + (60 * 10))) {
            // Only spam every 10 minutes...
            allprintf("\x007Automatic reboot in %ld minutes!  You should start finding an inn room.\r\n", MIN(1, (time_left / 60)));
            reboot.last_message = now;
        }
    }
}

void do_reboot(struct char_data *ch, const char *argument, int cmd)
{
    time_t                                  tc = (time_t) 0;
    struct tm                              *t_info = NULL;
    char                                   *tmstr = NULL;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_NPC(ch))
	return;

    tc = time(0);
    t_info = localtime(&tc);
    tmstr = asctime(t_info);
    *(tmstr + strlen(tmstr) - 1) = '\0';

    log_boot("REBOOT by %s at %d:%d", GET_NAME(ch), t_info->tm_hour + 1, t_info->tm_min);
    allprintf("\x007\r\nBroadcast message from %s (tty0) %s...\r\n\r\n", GET_NAME(ch),
              tmstr);
    allprintf("\x007Rebooting.  Come back in a few minutes!\r\n");
    allprintf("\x007The system is going down NOW !!\r\n\r\n");
    i3_log_dead();
    diku_shutdown = diku_reboot = 1;
    //update_time_and_weather();
    save_weather(TIME_FILE, time_info, weather_info);
}

void do_shutdown(struct char_data *ch, const char *argument, int cmd)
{
    time_t                                  tc = (time_t) 0;
    struct tm                              *t_info = NULL;
    char                                   *tmstr = NULL;
    char                                    arg[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_NPC(ch))
	return;

    tc = time(0);
    t_info = localtime(&tc);
    tmstr = asctime(t_info);
    *(tmstr + strlen(tmstr) - 1) = '\0';

    one_argument(argument, arg);

    if (!*arg) {
	log_boot("SHUTDOWN by %s at %d:%d", GET_NAME(ch), t_info->tm_hour + 1, t_info->tm_min);
	allprintf("\x007\r\nBroadcast message from %s (tty0) %s...\r\n\r\n", GET_NAME(ch),
		  tmstr);
	allprintf("\x007The system is going down NOW !!\r\n\x007\r\n");
        i3_log_dead();
	diku_shutdown = 1;
	//update_time_and_weather();
        save_weather(TIME_FILE, time_info, weather_info);
    } else if (!str_cmp(arg, "-k")) {
	log_info("FAKE REBOOT by %s at %d:%d", GET_NAME(ch),
		 t_info->tm_hour + 1, t_info->tm_min);
	allprintf("\x007\r\nBroadcast message from %s (tty0) %s...\r\n\r\n", GET_NAME(ch),
		  tmstr);
	allprintf("\x007Rebooting.  Come back in a few minutes!\r\n");
	allprintf("\x007The system is going down NOW !!\r\n\r\n");
    } else if (!str_cmp(arg, "-r")) {
	log_boot("REBOOT by %s at %d:%d", GET_NAME(ch), t_info->tm_hour + 1, t_info->tm_min);
	allprintf("\x007\r\nBroadcast message from %s (tty0) %s...\r\n\r\n", GET_NAME(ch),
		  tmstr);
	allprintf("\x007Rebooting.  Come back in a few minutes!\r\n");
	allprintf("\x007The system is going down NOW !!\r\n\r\n");
        i3_log_dead();
	diku_shutdown = diku_reboot = 1;
	//update_time_and_weather();
        save_weather(TIME_FILE, time_info, weather_info);
    } else
	cprintf(ch, "Go shut down someone your own size.\r\n");
}

void do_shutdow(struct char_data *ch, const char *argument, int cmd)
{
    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    cprintf(ch, "If you want to shut something down - say so!\r\n");
}

void show_reboot_info(struct char_data *ch) {
    cprintf(ch, "Automatic reboots are currently %s.\r\n",
            reboot.enabled ? "enabled" : "disabled");
    cprintf(ch, "The next scheduled reboot %s at %s RLT.\r\n",
        reboot.enabled ? "is" : "would be", reboot.next_reboot_text);
}

void do_setreboot(struct char_data *ch, const char *argument, int cmd)
{
    char    command_verb[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char    value_arg[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_NPC(ch))
	return;

    argument = one_argument(argument, command_verb);
    if (*command_verb) {
        if(!str_cmp(command_verb, "list") || !str_cmp(command_verb, "show")) {
            show_reboot_info(ch);
            return;
        } else if(!str_cmp(command_verb, "enable") || !str_cmp(command_verb, "on")) {
            if(!reboot.enabled)
                toggle_reboot(ch);
            show_reboot_info(ch);
            return;
        } else if(!str_cmp(command_verb, "disable") || !str_cmp(command_verb, "off")) {
            if(reboot.enabled)
                toggle_reboot(ch);
            show_reboot_info(ch);
            return;
        } else if(!str_cmp(command_verb, "weekly") || !str_cmp(command_verb, "week")) {
            int number = 1;

            argument = one_argument(argument, value_arg);
            if(*value_arg) {
                number = atoi(value_arg);
                if(number <= 1)
                    number = 1;
            }
            set_reboot_interval(ch, "week", number);
            show_reboot_info(ch);
            return;
        } else if(!str_cmp(command_verb, "daily") || !str_cmp(command_verb, "day")) {
            int number = 1;

            argument = one_argument(argument, value_arg);
            if(*value_arg) {
                number = atoi(value_arg);
                if(number <= 1)
                    number = 1;
            }
            set_reboot_interval(ch, "day", number);
            show_reboot_info(ch);
            return;
        } else if(!str_cmp(command_verb, "twice")) {
            set_reboot_interval(ch, "hour", 12);
            show_reboot_info(ch);
            return;
        } else if(!str_cmp(command_verb, "hourly") || !str_cmp(command_verb, "hour")) {
            int number = 1;

            argument = one_argument(argument, value_arg);
            if(*value_arg) {
                number = atoi(value_arg);
                if(number <= 1)
                    number = 1;
            }
            set_reboot_interval(ch, "hour", number);
            show_reboot_info(ch);
            return;
        }
    }
    cprintf(ch, "Usage: setreboot list\r\n"
                "       setreboot enable|disable\r\n"
                "       setreboot week [number]\r\n"
                "       setreboot day [number]\r\n"
                "       setreboot twice (twice per day)\r\n"
                "       setreboot hour [number]\r\n");
}


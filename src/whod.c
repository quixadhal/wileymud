/*
 * Opens a port (the port # above the port # the game itself is
 * being run). On this port people can connect to see who's on.
 * The player wont have to enter the game to see if it's worth
 * playing at the time, thus saving money.
 *
 * Change the following #define-statements to adjust the
 * WHOD to your server.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "version.h"
#include "global.h"
#include "bug.h"
#include "db.h"
#include "comm.h"
#include "utils.h"
#include "interpreter.h"
#include "multiclass.h"
#include "i3.h"
#include "sha256.h"
#include "modify.h"
#include "sql.h"
#define _WHOD_C
#include "whod.h"

/*
 * In function run_the_game(int port):
 *   ...
 *   init_whod(port);
 *   log_boot("Entering game loop.");
 *   game_loop(s);
 *   close_sockets(s);
 *   close_whod();
 *   ...
 *
 *   In function game_loop ():
 *   ...
 *   sigsetmask(mask);
 *   whod_loop();
 *   if (select(maxdesc + 1, &input_set, &output_set, &exc_set, &null_time) < 0)
 *   ...
 */

struct whod_data                        whod = {
    650336715,          // updated, beginning of time
    0,                  // enabled
    3001,               // port
    "SYSTEM",           // set_by
    1,                  // show_idle
    0,                  // show_level
    1,                  // show_name
    1,                  // show_title
    0,                  // show_room (out of game)
    0,                  // show_site (connection address)
    1,                  // show_room_ingame
    0,                  // show_linkdead
    5,                  // temporary, request state, start out as WHOD_CLOSED
    0,                  // temporary, socket descriptor 
    0,                  // temporary, disconnect timeout
};

void setup_whod_table(void) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    char *sql = "CREATE TABLE IF NOT EXISTS whod ( "
                "    updated TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT now(), "
                "    enabled BOOLEAN NOT NULL DEFAULT false, "
                "    port INTEGER NOT NULL DEFAULT 3001,"
                "    set_by TEXT NOT NULL DEFAULT 'SYSTEM',"
                "    show_idle BOOLEAN NOT NULL DEFAULT true, "
                "    show_level BOOLEAN NOT NULL DEFAULT false, "
                "    show_name BOOLEAN NOT NULL DEFAULT true, "
                "    show_title BOOLEAN NOT NULL DEFAULT true, "
                "    show_room BOOLEAN NOT NULL DEFAULT false, "
                "    show_site BOOLEAN NOT NULL DEFAULT false, "
                "    show_room_ingame BOOLEAN NOT NULL DEFAULT true, "
                "    show_linkdead BOOLEAN NOT NULL DEFAULT false "
                "); ";
    char *sql2 = "SELECT count(*) FROM whod;";
    char *sql3 = "INSERT INTO whod (enabled) VALUES (false);";
    char *sql4 = "DELETE FROM whod WHERE updated <> (SELECT max(UPDATED) FROM whod);";
    int rows = 0;
    int columns = 0;
    int count = 0;

    sql_connect(&db_wileymud);
    res = PQexec(db_wileymud.dbc, sql);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create whod table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);

    res = PQexec(db_wileymud.dbc, sql2);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot get row count of whod table: %s", PQerrorMessage(db_wileymud.dbc));
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
            log_fatal("Cannot insert placeholder row to whod table: %s", PQerrorMessage(db_wileymud.dbc));
            PQclear(res);
            proper_exit(MUD_HALT);
        }
        PQclear(res);
    } else if(count > 1) {
        // That shouldn't happen... fix it!
        log_error("There are %d rows in the whod table, instead of just ONE!", count);
        res = PQexec(db_wileymud.dbc, sql4);
        st = PQresultStatus(res);
        if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
            log_fatal("Cannot remove old rows from whod table: %s", PQerrorMessage(db_wileymud.dbc));
            PQclear(res);
            proper_exit(MUD_HALT);
        }
        PQclear(res);
    }
}

void load_whod(void) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    const char *sql =   "SELECT extract('epoch' FROM updated) AS updated, "
                        "enabled::integer, "
                        "port, "
                        "set_by, "
                        "show_idle::integer, "
                        "show_level::integer, "
                        "show_name::integer, "
                        "show_title::integer, "
                        "show_room::integer, "
                        "show_site::integer, "
                        "show_room_ingame::integer, "
                        "show_linkdead::integer "
                        "FROM whod LIMIT 1;";
    int rows = 0;
    int columns = 0;

    sql_connect(&db_wileymud);
    res = PQexec(db_wileymud.dbc, sql);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot get whod data from whod table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }

    rows = PQntuples(res);
    columns = PQnfields(res);
    if(rows > 0 && columns > 11) {
        log_boot("  Loading whod data from SQL database.");
        whod.updated = (time_t) atol(PQgetvalue(res,0,0));
        whod.enabled = (int) atoi(PQgetvalue(res,0,1));
        whod.port = (int) atoi(PQgetvalue(res,0,2));
        strlcpy(whod.set_by, PQgetvalue(res,0,3), MAX_INPUT_LENGTH);
        whod.show_idle = (int) atoi(PQgetvalue(res,0,4));
        whod.show_level = (int) atoi(PQgetvalue(res,0,5));
        whod.show_name = (int) atoi(PQgetvalue(res,0,6));
        whod.show_title = (int) atoi(PQgetvalue(res,0,7));
        whod.show_room = (int) atoi(PQgetvalue(res,0,8));
        whod.show_site = (int) atoi(PQgetvalue(res,0,9));
        whod.show_room_ingame = (int) atoi(PQgetvalue(res,0,10));
        whod.show_linkdead = (int) atoi(PQgetvalue(res,0,11));
    } else {
        log_fatal("Invalid result set from whod table!");
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);
}

int toggle_whod_flag(struct char_data *ch, const char *param) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    const char *sql_proto = "UPDATE whod SET %s = NOT %s, "
                            "updated = now(), "
                            "set_by = $1;";
    const char *param_val[1];
    int param_len[1];
    int param_bin[1] = {0};
    char set_by[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char value_param[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char sql[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    strlcpy(set_by, GET_NAME(ch), MAX_INPUT_LENGTH);
    // All our parameter names start with show_, but we don't want to force
    // the user to type that in a command, so if what they sent us doesn't start
    // with show_, we prefix it, so "show_idle" and "idle" should both work.
    if(strcasecmp(param, "enabled") && strncasecmp(param, "show_", 5)) {
        strlcpy(value_param, "show_", MAX_INPUT_LENGTH);
    }
    strlcat(value_param, param, MAX_INPUT_LENGTH);

    for(int i = 0; i < strlen(value_param); i++) {
        if(isupper(value_param[i]))
            value_param[i] = tolower(value_param[i]);
    }
    snprintf(sql, MAX_INPUT_LENGTH, sql_proto, value_param, value_param);
    log_sql("WHOD -- %s", sql);

    param_val[0] = (set_by[0]) ? set_by : NULL;
    param_len[0] = (set_by[0]) ? strlen(set_by) : 0;

    sql_connect(&db_wileymud);
    res = PQexecParams(db_wileymud.dbc, sql, 1, NULL, param_val, param_len, param_bin, 0);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_error("Cannot toggle %s in whod table: %s", value_param, PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        return 0;
        //proper_exit(MUD_HALT);
    }
    PQclear(res);
    load_whod();
    return 1;
}

int whod_flag_value(const char *param) {
    char value_param[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    // All our parameter names start with show_, but we don't want to force
    // the user to type that in a command, so if what they sent us doesn't start
    // with show_, we prefix it, so "show_idle" and "idle" should both work.
    if(strcasecmp(param, "enabled") && strncasecmp(param, "show_", 5)) {
        strlcpy(value_param, "show_", MAX_INPUT_LENGTH);
    }
    strlcat(value_param, param, MAX_INPUT_LENGTH);

    for(int i = 0; i < strlen(value_param); i++) {
        if(isupper(value_param[i]))
            value_param[i] = tolower(value_param[i]);
    }

    if(!strcasecmp(value_param, "show_idle")) {
        return whod.show_idle;
    } else if(!strcasecmp(value_param, "show_level")) {
        return whod.show_level;
    } else if(!strcasecmp(value_param, "show_name")) {
        return whod.show_name;
    } else if(!strcasecmp(value_param, "show_title")) {
        return whod.show_title;
    } else if(!strcasecmp(value_param, "show_room")) {
        return whod.show_room;
    } else if(!strcasecmp(value_param, "show_site")) {
        return whod.show_site;
    } else if(!strcasecmp(value_param, "show_room_ingame")) {
        return whod.show_room_ingame;
    } else if(!strcasecmp(value_param, "show_linkdead")) {
        return whod.show_linkdead;
    }

    return -1;
}

int set_whod_port(struct char_data *ch, int number) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    const char *sql =   "UPDATE whod SET port = $1, "
                        "updated = now(), "
                        "set_by = $2;";
    const char *param_val[2];
    int param_len[2];
    int param_bin[2] = {0,0};
    char set_by[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char value_param[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    strlcpy(set_by, GET_NAME(ch), MAX_INPUT_LENGTH);
    snprintf(value_param, MAX_INPUT_LENGTH, "%d", number);

    param_val[0] = (value_param[0]) ? value_param : NULL;
    param_len[0] = (value_param[0]) ? strlen(value_param) : 0;
    param_val[1] = (set_by[0]) ? set_by : NULL;
    param_len[1] = (set_by[0]) ? strlen(set_by) : 0;

    sql_connect(&db_wileymud);
    res = PQexecParams(db_wileymud.dbc, sql, 2, NULL, param_val, param_len, param_bin, 0);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_error("Cannot set port in whod table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        return 0;
        //proper_exit(MUD_HALT);
    }
    PQclear(res);
    load_whod();
    return 1;
}

void init_whod(void) {
    if (DEBUG > 2)
	log_info("called %s with no arguments", __PRETTY_FUNCTION__);

    load_whod();
    if(whod.enabled) {
        whod.socket = init_socket(whod.port);
        whod.state = WHOD_OPEN;
        log_boot("WHOD port %d opened.", whod.port);
    } else {
        log_boot("WHOD not enabled.");
    }
}

void close_whod(void) {
    if (DEBUG > 2)
	log_info("called %s with no arguments", __PRETTY_FUNCTION__);

    if (whod.state != WHOD_CLOSED) {
	close(whod.socket);
	whod.state = WHOD_CLOSED;
	log_boot("WHOD port %d closed.", whod.port);
    }
}

void whod_loop(void) {
    unsigned int                            size = 0;
    fd_set                                  in;
    unsigned long                           hostlong = 0L;
    struct timeval                          timeout;
    struct sockaddr_in                      newaddr;
    struct hostent                         *hent = NULL;
    static int                              newdesc = 0;

    if (DEBUG > 2)
	log_info("called %s with no arguments", __PRETTY_FUNCTION__);

    if (!whod.enabled && whod.state == WHOD_CLOSED)
        return;

    switch (whod.state) {
	case WHOD_OPENING:
            if(whod.socket)
                close(whod.socket);
	    whod.socket = init_socket(whod.port);
	    log_boot("WHOD port opened.");
	    whod.state = WHOD_OPEN;
	    break;

	case WHOD_OPEN:
	    timeout.tv_sec = 0;
	    timeout.tv_usec = 100;

	    FD_ZERO(&in);
	    FD_SET(whod.socket, &in);

	    select(whod.socket + 1, &in, (fd_set *) 0, (fd_set *) 0, &timeout);

	    if (FD_ISSET(whod.socket, &in)) {
		size = sizeof(newaddr);
		getsockname(whod.socket, (struct sockaddr *)&newaddr, &size);

		if ((newdesc = accept(whod.socket, (struct sockaddr *)&newaddr, &size)) < 0) {
		    log_error("WHOD - Accept");
		    return;
		}
		if ((hent = gethostbyaddr((char *)&newaddr.sin_addr, sizeof(newaddr.sin_addr),
                                          AF_INET)))
		    log_info("WHO request from %s served.", VNULL(hent->h_name));
		else {
		    hostlong = htonl(newaddr.sin_addr.s_addr);
		    log_info("WHO request from %lu.%lu.%lu.%lu served.",
			     (hostlong & 0xff000000) >> 24,
			     (hostlong & 0x00ff0000) >> 16,
			     (hostlong & 0x0000ff00) >> 8, (hostlong & 0x000000ff) >> 0);
		}

		WRITE(newdesc, whod_text(NULL));

		whod.disconnect_time = time(NULL) + WHOD_DELAY_TIME;
		whod.state = WHOD_DELAY;
	    } else if (!whod.enabled) {
		whod.state = WHOD_CLOSING;
	    }
	    break;

	case WHOD_DELAY:
	    if (time(NULL) >= whod.disconnect_time)
		whod.state = WHOD_END;
	    break;

	case WHOD_END:
            if(newdesc)
	        close(newdesc);

	    if (!whod.enabled)
		whod.state = WHOD_CLOSING;
	    else
		whod.state = WHOD_OPEN;
	    break;

	case WHOD_CLOSING:
	    close_whod();
	    whod.state = WHOD_CLOSED;
	    break;

	case WHOD_CLOSED:
	    if (whod.enabled)
		whod.state = WHOD_OPENING;
	    break;

        default:
            log_error("You should not be here! whod.state = %d", whod.state);
            break;
    }
}

char *idle_time_string(struct char_data *ch) {
    static char     buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    time_t          ttime = 0;
    time_t          thour = 0;
    time_t          tmin = 0;
    time_t          tsec = 0;

    buf[0] = '\0';
    if(ch && IS_PC(ch)) {
        if (!(ch->desc)) {
            strlcpy(buf, "linkdead", MAX_INPUT_LENGTH);
        } else {
            ttime = GET_IDLE_TIME(ch);
            thour = ttime / 3600;
            ttime -= thour * 3600;
            tmin = ttime / 60;
            ttime -= tmin * 60;
            tsec = ttime;
            if ((thour < 1) && (tmin < 1) && (tsec <= 15))
                strlcpy(buf, "playing", MAX_INPUT_LENGTH);
            else if(thour > 99)
                strlcpy(buf, "ZEN", MAX_INPUT_LENGTH);
            else
                snprintf(buf, MAX_INPUT_LENGTH, "%02ld:%02ld:%02ld", thour, tmin, tsec);
        }
    }
    return buf;
}

char *player_level_string(struct char_data *ch) {
    static char     buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    buf[0] = '\0';
    if(ch && IS_PC(ch)) {
        if (GetMaxLevel(ch) >= WIZ_MAX_LEVEL)
            strlcpy(buf, "[ God ]", MAX_INPUT_LENGTH);
        else if (GetMaxLevel(ch) == WIZ_MAX_LEVEL - 1)
            strlcpy(buf, "[Power]", MAX_INPUT_LENGTH);
        else if (GetMaxLevel(ch) >= WIZ_MIN_LEVEL)
            strlcpy(buf, "[Whizz]", MAX_INPUT_LENGTH);
        else
            snprintf(buf, MAX_INPUT_LENGTH, "[ %3d ]", GetMaxLevel(ch));
    }
    return buf;
}

char *player_full_name(struct char_data *ch, int show_title) {
    static char     buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    buf[0] = '\0';
    if(ch && IS_PC(ch)) {
        if (show_title && GET_PRETITLE(ch) )
            scprintf(buf, MAX_INPUT_LENGTH, "%s ", GET_PRETITLE(ch));

        scprintf(buf, MAX_INPUT_LENGTH, "%s", GET_NAME(ch));

        if (show_title)
            scprintf(buf, MAX_INPUT_LENGTH, " %s", GET_TITLE(ch));
    }
    return buf;
}

char *player_room_name(struct char_data *ch) {
    static char     buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    buf[0] = '\0';
    if(ch && IS_PC(ch)) {
        struct room_data *rp = NULL;

        rp = real_roomp(ch->in_room);
        if(rp) {
            scprintf(buf, MAX_INPUT_LENGTH, "%s", rp->name);
            if(ch && IS_IMMORTAL(ch))
                scprintf(buf, MAX_INPUT_LENGTH, " [#%d]", ch->in_room);
        } else {
            scprintf(buf, MAX_INPUT_LENGTH, "%s", "Swirling Chaos!");
        }
    }
    return buf;
}

char *player_site_name(struct char_data *ch) {
    static char     buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    buf[0] = '\0';
    if(ch && IS_PC(ch)) {
        if(ch->desc) {
            if (ch->desc->host[0] != '\0') {
                scprintf(buf, MAX_INPUT_LENGTH, "%s", ch->desc->host);
            } else {
                scprintf(buf, MAX_INPUT_LENGTH, "%s", ch->desc->ip);
            }
        } else {
            scprintf(buf, MAX_INPUT_LENGTH, "%s", "disconnected");
        }
    }
    return buf;
}

char *whod_text(struct char_data *viewer) {
    static char                             buf[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    int                                     players = 0;
    int                                     gods = 0;
    struct char_data                       *ch = NULL;

    snprintf(buf, MAX_STRING_LENGTH, "%s\r\n", VERSION_STR);

    for (ch = character_list; ch; ch = ch->next) {
        if (IS_NPC(ch))
            continue;

        if ((INVIS_LEVEL(ch) < 2) && (GetMaxLevel(ch) <= WIZ_MAX_LEVEL) &&
            !IS_AFFECTED(ch, AFF_HIDE) && !IS_AFFECTED(ch, AFF_INVISIBLE)) {
            if (GetMaxLevel(ch) >= WIZ_MIN_LEVEL)
                gods++;
            else
                players++;

            if (whod.show_idle)
                scprintf(buf, MAX_STRING_LENGTH, "%8.8s ", idle_time_string(ch));

            if (whod.show_level)
                scprintf(buf, MAX_STRING_LENGTH, "%8.8s ", player_level_string(ch));

            if (whod.show_name)
                scprintf(buf, MAX_STRING_LENGTH, "%s ", player_full_name(ch, whod.show_title));

            // Showing the player's room outside the game promotes griefing, but
            // we may not want to show it IN game either, hence the split of settings.
            // If the viewer is NULL, we are out of game and use the show_room setting,
            // otherwise we use the show_room_ingame setting.
            if (viewer ? whod.show_room_ingame : whod.show_room)
                scprintf(buf, MAX_STRING_LENGTH, "- %s ", player_room_name(ch));

            if (whod.show_site)
                scprintf(buf, MAX_STRING_LENGTH, "(%s)", player_site_name(ch));

            strlcat(buf, "\r\n", MAX_STRING_LENGTH);
            // WRITE(newdesc, buf); 
            // *buf = '\0';
        }
    }
    strlcat(buf, "\r\n", MAX_STRING_LENGTH);
    scprintf(buf, MAX_STRING_LENGTH, "     Visible Players: %3d\r\n", players);
    scprintf(buf, MAX_STRING_LENGTH, "        Visible Gods: %3d\r\n", gods);
    scprintf(buf, MAX_STRING_LENGTH, "Wiley start time was: %s\r\n", timestamp(Uptime, 0));
    scprintf(buf, MAX_STRING_LENGTH, " Quixadhal's time is: %s\r\n", timestamp(-1, 0));

    return buf;
}

void show_whod_info(struct char_data *ch) {
    cprintf(ch, "Current WHOD mode:\r\n------------------\r\n");
    cprintf(ch, "    Enabled:              %s\r\n", BOOLSTR(whod.enabled));
    cprintf(ch, "    Port:                 %d\r\n", whod.port);
    cprintf(ch, "\r\n");
    cprintf(ch, "    Show Idle:            %s\r\n", ENABLED(whod.show_idle));
    cprintf(ch, "    Show Level:           %s\r\n", ENABLED(whod.show_level));
    cprintf(ch, "    Show Name:            %s\r\n", ENABLED(whod.show_name));
    cprintf(ch, "    Show Title:           %s\r\n", ENABLED(whod.show_title));
    cprintf(ch, "    Show Room (in game):  %s\r\n", ENABLED(whod.show_room_ingame));
    cprintf(ch, "    Show Room (external): %s\r\n", ENABLED(whod.show_room));
    cprintf(ch, "    Show Site:            %s\r\n", ENABLED(whod.show_site));
    cprintf(ch, "    Show Linkdead Status: %s\r\n", ENABLED(whod.show_linkdead));
    cprintf(ch, "\r\n");
    cprintf(ch, "    Last Updated:         %s by %s.\r\n",
            timestamp(whod.updated, 0), whod.set_by);
    cprintf(ch, "\r\n");
}

void do_whod(struct char_data *ch, const char *arg, int cmd)
{
    char    command_verb[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char    value_arg[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG)
	log_info("called by %s with %s, %s, %d", __PRETTY_FUNCTION__,
                 SAFE_NAME(ch), VNULL(arg), cmd);

    if (IS_NPC(ch))
	return;

    arg = one_argument(arg, command_verb);
    if (*command_verb) {
        if(!str_cmp(command_verb, "status") || !str_cmp(command_verb, "list")) {
            show_whod_info(ch);
            return;
        } else if(!str_cmp(command_verb, "enable") || !str_cmp(command_verb, "on")) {
            if(!whod.enabled) {
                toggle_whod_flag(ch, "enabled");
                init_whod();
            }
            show_whod_info(ch);
            return;
        } else if(!str_cmp(command_verb, "disable") || !str_cmp(command_verb, "off")) {
            if(whod.enabled) {
                toggle_whod_flag(ch, "enabled");
                close_whod();
            }
            show_whod_info(ch);
            return;
        } else if(!str_cmp(command_verb, "port")) {
            int number = 3001;

            arg = one_argument(arg, value_arg);
            if(*value_arg) {
                number = atoi(value_arg);
                // ports below 1000 require root privs.
                if(number < 1001)
                    number = 1000;
                if(number != whod.port) {
                    close_whod();
                    set_whod_port(ch, number);
                    init_whod();
                }
            }
            show_whod_info(ch);
            return;
        } else if(!str_cmp(command_verb, "show") || !str_cmp(command_verb, "display")) {
            int value = -1;

            arg = one_argument(arg, value_arg);
            if((value = whod_flag_value(value_arg)) < 0) {
                cprintf(ch, "%s is not a valid flag for whod!\r\n", value_arg);
            } else {
                if(!value)
                    toggle_whod_flag(ch, value_arg);
                show_whod_info(ch);
                return;
            }
        } else if(!str_cmp(command_verb, "hide")) {
            int value = -1;

            arg = one_argument(arg, value_arg);
            if((value = whod_flag_value(value_arg)) < 0) {
                cprintf(ch, "%s is not a valid flag for whod!\r\n", value_arg);
            } else {
                if(value)
                    toggle_whod_flag(ch, value_arg);
                show_whod_info(ch);
                return;
            }
        } else if(!str_cmp(command_verb, "toggle")) {
            int value = -1;

            arg = one_argument(arg, value_arg);
            if((value = whod_flag_value(value_arg)) < 0) {
                cprintf(ch, "%s is not a valid flag for whod!\r\n", value_arg);
            } else {
                toggle_whod_flag(ch, value_arg);
                show_whod_info(ch);
                return;
            }
        } 
    }
    cprintf(ch, "Usage: whod status\r\n"
                "       whod enable|disable\r\n"
                "       whod port <number>\r\n"
                "       whod show|hide|toggle <column>\r\n"
                "\r\n"
                "       column can be one of:\r\n"
                "       idle, level, name, title, room\r\n"
                "       room_ingame, or linkdead\r\n");
}

// The mess down here is what makes the JSON files our web page uses
// to make pretty mudlist info along with our who data.

#define WEB_DIR         "../public_html/"
#define MUDLIST_PAGE    WEB_DIR "mudlist.html"
#define PUBLIC_GFX      "gfx/mud/"
#define MUDLIST_GFX     WEB_DIR PUBLIC_GFX
#define MUDLIST_WIDTH   192 // 240
#define MUDLIST_HEIGHT  120 // 150

#define URL_HOME        "http://wileymud.themud.org/~wiley"
#define MUDLIST_ICON    URL_HOME "/gfx/mud.png"
#define LOG_ICON        URL_HOME "/gfx/log.png"
#define DISCORD_ICON    URL_HOME "/gfx/discord.png"
#define SERVER_ICON     URL_HOME "/gfx/server_icon.png"
#define WILEY_ICON      URL_HOME "/gfx/wileymud3.png"
#define ICON_WIDTH      48
//#define WILEY_WIDTH     247
//#define WILEY_HEIGHT    48
#define WILEY_WIDTH     165
#define WILEY_HEIGHT    32

#define MUDLIST_URL     URL_HOME "/mudlist.html"
#define LOG_URL         URL_HOME "/logpages"
#define DISCORD_URL     "https://discord.gg/kUduSsJ"
#define SERVER_URL      URL_HOME "/server.php"

#define JSON_MUDLIST_PAGE "../public_html/" "mudlist.json"

void                                    generate_json_mudlist(void)
{
    FILE                                   *fp = NULL;
#ifdef I3
    I3_MUD                                 *mud;
#endif
    int                                     did_one = 0;
    time_t                                  now;
    char                                    nowtimebuf[MAX_INPUT_LENGTH];
    //char                                    uptimebuf[MAX_INPUT_LENGTH];
    struct char_data                       *ch = NULL;
    int                                     mortals = 0;
    int                                     gods = 0;

    if(!(fp = fopen(JSON_MUDLIST_PAGE, "w"))) {
        log_error("Cannot open %s!", JSON_MUDLIST_PAGE);
        return;
    }

    now = time((time_t *) 0);
    strftime(nowtimebuf, sizeof(nowtimebuf), RFC1123FMT, localtime(&now));
    //strftime(uptimebuf, sizeof(uptimebuf), RFC1123FMT, localtime((time_t *) & Uptime));

    fprintf(fp, "{\n");
    fprintf(fp, "    \"version\" : {\n");
    fprintf(fp, "        \"build\" : \"%s\",\n", json_escape(VERSION_BUILD));
    fprintf(fp, "        \"date\" : \"%s\"\n", json_escape(VERSION_DATE));
    fprintf(fp, "    },\n");
    fprintf(fp, "    \"time\" : \"%5.5s %s\",\n", &nowtimebuf[17], &nowtimebuf[26]);
    //fprintf(fp, "    \"boot\" : \"%s\",\n", uptimebuf);
    fprintf(fp, "    \"boot\" : \"%s\",\n", json_escape(timestamp(Uptime, 0)));
    fprintf(fp, "    \"player_list\" : [\n");

    did_one = 0;
    for (ch = character_list; ch; ch = ch->next) {
	if (IS_PC(ch)) {
	    if ((INVIS_LEVEL(ch) < 2) && (GetMaxLevel(ch) <= WIZ_MAX_LEVEL) &&
		!IS_AFFECTED(ch, AFF_HIDE) && !IS_AFFECTED(ch, AFF_INVISIBLE)) {

                if( did_one ) {
                    fprintf(fp, ",\n");
                } else {
                    did_one = 1;
                }

                fprintf(fp, "        {\n");
                fprintf(fp, "            \"name\" : \"%s\",\n", GET_NAME(ch) ? json_escape(GET_NAME(ch)) : "");
                fprintf(fp, "            \"title\" : \"%s\",\n", GET_TITLE(ch) ? json_escape(GET_TITLE(ch)) : "");
                fprintf(fp, "            \"pretitle\" : \"%s\",\n", GET_PRETITLE(ch) ? json_escape(GET_PRETITLE(ch)) : "");
                fprintf(fp, "            \"idle\" : \"%ld\",\n", GET_IDLE_TIME(ch));
                fprintf(fp, "            \"level\" : \"%d\",\n", GetMaxLevel(ch));
                fprintf(fp, "            \"rank\" : \"%s\"\n", 
                        GetMaxLevel(ch) >= WIZ_MAX_LEVEL ? "God" :
                        GetMaxLevel(ch) == WIZ_MAX_LEVEL - 1 ? "Power" :
                        GetMaxLevel(ch) >= WIZ_MIN_LEVEL ? "Whizz" :
                        "mortal");
                fprintf(fp, "        }");

                if( GetMaxLevel(ch) >= WIZ_MIN_LEVEL ) {
                    gods++;
                } else {
                    mortals++;
                }
            }
        }
    }
    if( did_one ) {
        fprintf(fp, "\n");
    }
    fprintf(fp, "    ],\n");
    fprintf(fp, "    \"players\" : {\n");
    fprintf(fp, "        \"gods\" : \"%d\",\n", gods);
    fprintf(fp, "        \"mortals\" : \"%d\"\n", mortals);
    fprintf(fp, "    },\n");

    did_one = 0;
    fprintf(fp, "    \"mudlist\" : [ \n");
#ifdef I3
    for (mud = first_mud; mud; mud = mud->next) {
        if( mud == NULL )
            continue;
        if( mud->name == NULL )
            continue;
        if( mud->mud_type == NULL )
            continue;
        if( mud->mudlib == NULL )
            continue;
        if( mud->ipaddress == NULL )
            continue;
        if( did_one ) {
            fprintf(fp, ",\n");
        } else {
            did_one = 1;
        }
        fprintf(fp, "        {\n");
        fprintf(fp, "            \"name\"      : \"%s\", \n", json_escape(mud->name));
        fprintf(fp, "            \"md5\"       : \"%s\", \n", json_escape(md5_hex(mud->name)));
        fprintf(fp, "            \"type\"      : \"%s\", \n", json_escape(mud->mud_type));
        fprintf(fp, "            \"mudlib\"    : \"%s\", \n", json_escape(mud->mudlib));
        fprintf(fp, "            \"ipaddress\" : \"%s\", \n", json_escape(mud->ipaddress));
        fprintf(fp, "            \"port\"      : %d,\n", mud->player_port);
        fprintf(fp, "            \"online\"    : %d\n", mud->status == -1 ? 1 : 0);
        fprintf(fp, "        }");
    }
    if( did_one ) {
        fprintf(fp, "\n");
    }
#endif
    fprintf(fp, "    ]\n");
    fprintf(fp, "}\n");

    fclose(fp);
    fp = NULL;
    return;
}

void do_who(struct char_data *ch, const char *argument, int cmd)
{
    struct descriptor_data                 *d = NULL;
    struct char_data                       *person = NULL;
    int                                     count = 0;
    long                                    ttime = 0;
    long                                    thour = 0;
    long                                    tmin = 0;
    long                                    tsec = 0;
    char                                    buf[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    //time_t                                  now = 0;
    //char                                    uptimebuf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    //char                                    nowtimebuf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

/*
 * if (IS_NPC(ch))
 *   return;
 */

    //now = time((time_t *) 0);
    //strftime(nowtimebuf, sizeof(nowtimebuf), RFC1123FMT, localtime(&now));
    //strftime(uptimebuf, sizeof(uptimebuf), RFC1123FMT, localtime((time_t *) &Uptime));

    page_printf(ch, "%s\r\n", VERSION_STR);

    for (d = descriptor_list; d; d = d->next) {
	person = d->character;

	if (!person || !person->desc)
	    continue;
	if (!IS_PC(person))
	    continue;
	if (d->connected || !CAN_SEE(ch, person))
	    continue;
	if (cmd == 234 && real_roomp(person->in_room)->zone != real_roomp(ch->in_room)->zone)
	    continue;
	count++;
	bzero(buf, MAX_STRING_LENGTH);

	if (whod.show_idle) {
	    if (!(person->desc)) {
		strlcat(buf, "linkdead ", MAX_STRING_LENGTH);
	    } else {
		ttime = GET_IDLE_TIME(person);
		thour = ttime / 3600;
		ttime -= thour * 3600;
		tmin = ttime / 60;
		ttime -= tmin * 60;
		tsec = ttime;
		if (!thour && !tmin && (tsec <= 15)) {
		    strlcat(buf, " playing ", MAX_STRING_LENGTH);
                } else {
		    scprintf(buf, MAX_STRING_LENGTH, "%02ld:%02ld:%02ld ", thour, tmin, tsec);
                }
	    }
	}
	if (whod.show_level) {
	    if (GetMaxLevel(person) >= WIZ_MAX_LEVEL)
		strlcat(buf, "[ God ] ", MAX_STRING_LENGTH);
	    else if (GetMaxLevel(person) == WIZ_MAX_LEVEL - 1)
		strlcat(buf, "[Power] ", MAX_STRING_LENGTH);
	    else if (GetMaxLevel(person) >= WIZ_MIN_LEVEL)
		strlcat(buf, "[Whizz] ", MAX_STRING_LENGTH);
	    else
		scprintf(buf, MAX_STRING_LENGTH, "[ %3d ] ", GetMaxLevel(person));
	}
        if (whod.show_name) {
            if (whod.show_title)
                if (GET_PRETITLE(person)) {
                    scprintf(buf, MAX_STRING_LENGTH, "%s ", GET_PRETITLE(person));
                }

            scprintf(buf, MAX_STRING_LENGTH, "%s ", GET_NAME(person));

            if (whod.show_title) {
                scprintf(buf, MAX_STRING_LENGTH, "%s ", GET_TITLE(person));
            }
        }

	if (whod.show_room_ingame) {
	    scprintf(buf, MAX_STRING_LENGTH, "- %s ", real_roomp(person->in_room)->name);
	    if (GetMaxLevel(ch) >= LOW_IMMORTAL) {
		scprintf(buf, MAX_STRING_LENGTH, "[#%d]", person->in_room);
            }
	}
	if (whod.show_site) {
	    if (person->desc->host[0] != '\0') {
		scprintf(buf, MAX_STRING_LENGTH, "(%s)", person->desc->host);
            } else if (person->desc->ip[0] != '\0') {
		scprintf(buf, MAX_STRING_LENGTH, "(%s)", person->desc->ip);
            }
	}
	strlcat(buf, "\r\n", MAX_STRING_LENGTH);
	page_printf(ch, "%s", buf);
    }
    snprintf(buf, MAX_STRING_LENGTH, "\r\nTotal visible players on %s: %d\r\n", MUDNAME, count);
    scprintf(buf, MAX_STRING_LENGTH, "Wiley start time was: %s\r\n", timestamp(Uptime, 0));
    scprintf(buf, MAX_STRING_LENGTH, " Quixadhal's time is: %s\r\n", timestamp(-1, 0));
    page_printf(ch, "%s", buf);
}

void do_users(struct char_data *ch, const char *argument, int cmd)
{
    struct descriptor_data                 *d = NULL;
    int                                     flag = 0;
    char                                    buf[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    char                                    line[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    strlcpy(buf, "Connections:\r\n------------\r\n", MAX_STRING_LENGTH);

    for (d = descriptor_list; d; d = d->next) {
	flag = 0;
	if (d->character && d->character->player.name) {
	    if (GetMaxLevel(ch) > d->character->invis_level)
		flag = 1;

	    if (flag) {
		if (d->original)
		    snprintf(line, MAX_INPUT_LENGTH, "%-16s: ", d->original->player.name);
		else
		    snprintf(line, MAX_INPUT_LENGTH, "%-16s: ", d->character->player.name);
	    }
	} else {
	    strlcpy(line, connected_types[d->connected], MAX_INPUT_LENGTH);
	    strlcat(line, "\r\n", MAX_INPUT_LENGTH);
	}

	if (flag)
	    scprintf(line, MAX_INPUT_LENGTH, "[%s@%s/%s]\r\n", d->username,
		    d->host[0] ? d->host : "unknown", d->ip[0] ? d->ip : "unknown");
	if (flag)
	    strlcat(buf, line, MAX_STRING_LENGTH);
    }
    cprintf(ch, "%s\r\n", buf);
}

void do_players(struct char_data *ch, const char *argument, int cmd)
{
    char                                    buf[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    int                                     i = 0;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    cprintf(ch, "Player List for WileyMUD III\r\n\r\n");
    bzero(buf, MAX_STRING_LENGTH);
    for (i = 0; i < number_of_players; i++) {
	if (!list_of_players[i])
	    continue;
	if (strlen(buf) + strlen(list_of_players[i] + 2) >= MAX_STRING_LENGTH) {
	    page_string(ch->desc, buf, 1);
	    bzero(buf, MAX_STRING_LENGTH);
	}
	scprintf(buf, MAX_STRING_LENGTH, "%s\r", list_of_players[i]);
    }
    if (strlen(buf))
	page_string(ch->desc, buf, 1);
}


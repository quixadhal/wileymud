#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <time.h>

#include "global.h"
#include "utils.h"
#include "comm.h"
#include "modify.h"
#include "interpreter.h"
#include "db.h"
#include "bug.h"
#include "sql.h"

#define __BAN_C__
#include "ban.h"

struct ban_data *ban_list = NULL;
int              ban_list_count = 0;

/* Returns TRUE if this character name would be acceptable for a
 * new player.
 * Returns FALSE if it fails any checks, including being an
 * already existing character or mob, an illegal name, banned, etc.
 */
int acceptable_name(const char *name)
{
    char                                    tmp_name[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    struct char_file_u                      tmp_store;
    struct char_data                        tmp_char;

    if (!valid_parse_name(name, tmp_name))
	return FALSE;
    if (check_playing(NULL, tmp_name))
	return FALSE;
    /*
     * We don't check ValidPlayer here 
     */
    if (fread_char(tmp_name, &tmp_store, &tmp_char) > -1)
	return FALSE;
    if (load_char(tmp_name, &tmp_store) > -1)
	return FALSE;
    if (already_mob_name(tmp_name))
	return FALSE;
    if (banned_name(tmp_name))
	return FALSE;

    return TRUE;
}

int banned_name(char *name)
{
    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, VNULL(name));

    if (!ban_list || ban_list_count < 1)
        return FALSE;

    for(int i = 0; i < ban_list_count; i++) {
        if(!strcasecmp(ban_list[i].ban_type, "NAME"))
            if(!str_cmp(ban_list[i].name, name))
                return TRUE;
    }

    return FALSE;
}

int banned_ip(char *ip)
{
    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, VNULL(ip));

    if (!ban_list || ban_list_count < 1)
        return FALSE;

    for(int i = 0; i < ban_list_count; i++) {
        if(!strcasecmp(ban_list[i].ban_type, "IP"))
            if(!str_cmp(ban_list[i].ip, ip))
                return TRUE;
    }

    return FALSE;
}

int banned_at(char *name, char *ip)
{
    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, VNULL(name));

    if (!ban_list || ban_list_count < 1)
        return FALSE;

    for(int i = 0; i < ban_list_count; i++) {
        if(!strcasecmp(ban_list[i].ban_type, "NAME_AT_IP"))
            if(!str_cmp(ban_list[i].name, name) && !str_cmp(ban_list[i].ip, ip))
                return TRUE;
    }

    return FALSE;
}

void show_bans(struct char_data *ch)
{
    int visible_bans = 0;

    for(int i = 0; i < ban_list_count; i++) {
        if( !strcasecmp(ban_list[i].ban_type, "NAME") ||
            !strcasecmp(ban_list[i].ban_type, "IP"))
            visible_bans++;
    }
    cprintf(ch, "Ban List:\r\n");
    if(visible_bans < 1) {
        cprintf(ch, "       %-20s\r\n", "none");
    } else {
        cprintf(ch, "%-10s %4s %-16s %-16s %s\r\n",
                    "Date Set",
                    "Type",
                    "Name/IP",
                    "Set By",
                    "Reason"
                    );
        cprintf(ch, "%-10s %4s %-16s %-16s %s\r\n",
                    "----------",
                    "----",
                    "----------------",
                    "----------------",
                    "----------------------------"
                    );
        for(int i = 0; i < ban_list_count; i++) {
            if(!strcasecmp(ban_list[i].ban_type, "NAME"))
                cprintf(ch, "%-10s %4s %-16s %-16s %s\r\n",
                        ban_list[i].updated_text,
                        "NAME",
                        ban_list[i].name,
                        ban_list[i].set_by,
                        ban_list[i].reason
                        );
        }
        for(int i = 0; i < ban_list_count; i++) {
            if(!strcasecmp(ban_list[i].ban_type, "IP"))
                cprintf(ch, "%-10s %4s %-16s %-16s %s\r\n",
                        ban_list[i].updated_text,
                        "IP",
                        ban_list[i].ip,
                        ban_list[i].set_by,
                        ban_list[i].reason
                        );
        }
    }
}

void do_ban(struct char_data *ch, const char *argument, int cmd)
{
    char                                    ban_type[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    char                                    buf[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    struct ban_data                         new_ban;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_NPC(ch)) {
	cprintf(ch, "You're a mob, you can't ban anyone.\r\n");
	return;
    }

    new_ban.updated = -1;
    new_ban.expires = -1;
    new_ban.enabled = 1;
    strlcpy(new_ban.set_by, GET_NAME(ch), MAX_INPUT_LENGTH);
    new_ban.reason[0] = '\0';

    if (argument && *argument) {
	argument = one_argument(argument, ban_type);
	only_argument(argument, buf);
        // For now, we only accept name|ip and the value.
        // Eventually, we'll want to allow name@ip and also expiration dates, and
        // a reason for the ban.
	if (*ban_type) {
	    if (!str_cmp(ban_type, "list")) {
                show_bans(ch);
                return;
            } else if (!str_cmp(ban_type, "name")) {
		if (*buf) {
                    // OK, global name ban attempt.
                    // First, we check acceptable_name() because that will make sure
                    // we aren't banning something that is already in the game.
		    if (!acceptable_name(buf)) {
			cprintf(ch, "%s is already an invalid choice.\r\n", buf);
			return;
		    }

                    strlcpy(new_ban.ban_type, "NAME", MAX_INPUT_LENGTH);
                    strlcpy(new_ban.name, buf, MAX_INPUT_LENGTH);
                    new_ban.ip[0] = '\0';

                    if(add_ban(&new_ban)) {
		        cprintf(ch, "%s is now banned!\r\n", new_ban.name);
                        log_auth(ch, "BAN name %s is now banned!", new_ban.name);
                    } else {
                        cprintf(ch, "Could not add ban for %s.\r\n", new_ban.name);
                        log_auth(ch, "Failed to add ban for %s.\r\n", new_ban.name);
                    }
		    return;
		}
	    } else if (!str_cmp(ban_type, "ip") || !str_cmp(ban_type, "address")
		       || !str_cmp(ban_type, "site")) {
		if (*buf) {
		    // No banning localhost! 
		    if (!str_cmp("127.0.0.1", buf)) {
			cprintf(ch, "You cannot ban localhost!\r\n");
			return;
		    }

                    strlcpy(new_ban.ban_type, "IP", MAX_INPUT_LENGTH);
                    new_ban.name[0] = '\0';
                    strlcpy(new_ban.ip, buf, MAX_INPUT_LENGTH);

                    if(add_ban(&new_ban)) {
		        cprintf(ch, "%s is now banned!\r\n", new_ban.ip);
                        log_auth(ch, "BAN IP %s is now banned!", new_ban.ip);
                    } else {
                        cprintf(ch, "Could not add ban for %s.\r\n", new_ban.ip);
                        log_auth(ch, "Failed to add ban for %s.\r\n", new_ban.ip);
                    }

		    return;
		}
	    }
	}
    }
    cprintf(ch, "Usage: ban list\r\n       ban name <name-to-ban>\r\n       ban ip <ip-to-ban>\r\n");
}

void do_unban(struct char_data *ch, const char *argument, int cmd)
{
    char                                    ban_type[MAX_STRING_LENGTH] = "\0\0\0";
    char                                    buf[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    struct ban_data                         new_ban;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_NPC(ch)) {
	cprintf(ch, "You're a mob, you can't unban anyone.\r\n");
	return;
    }

    new_ban.updated = -1;
    new_ban.expires = -1;
    new_ban.enabled = 0;
    strlcpy(new_ban.set_by, GET_NAME(ch), MAX_INPUT_LENGTH);
    new_ban.reason[0] = '\0';

    if (argument && *argument) {
	argument = one_argument(argument, ban_type);
	only_argument(argument, buf);
        // For now, we only accept name|ip and the value.
        // Eventually, we'll want to allow name@ip and also expiration dates, and
        // a reason for the ban.
	if (*ban_type) {
	    if (!str_cmp(ban_type, "list")) {
                show_bans(ch);
                return;
            } else if (!str_cmp(ban_type, "name")) {
		if (*buf) {
		    if (!banned_name(buf)) {
			cprintf(ch, "%s is not banned.\r\n", buf);
			return;
		    }

                    strlcpy(new_ban.ban_type, "NAME", MAX_INPUT_LENGTH);
                    strlcpy(new_ban.name, buf, MAX_INPUT_LENGTH);
                    new_ban.ip[0] = '\0';

                    if(remove_ban(&new_ban)) {
		        cprintf(ch, "%s is no longer banned!\r\n", new_ban.name);
                        log_auth(ch, "BAN name %s has been unbanned!", new_ban.name);
                    } else {
                        cprintf(ch, "Could not remove ban for %s.\r\n", new_ban.name);
                        log_auth(ch, "Failed to remove ban for %s.\r\n", new_ban.name);
                    }
		    return;
		}
	    } else if (!str_cmp(ban_type, "ip") || !str_cmp(ban_type, "address")
		       || !str_cmp(ban_type, "site")) {
		if (*buf) {
		    if (!banned_ip(buf)) {
			cprintf(ch, "%s is not banned.\r\n", buf);
			return;
		    }

                    strlcpy(new_ban.ban_type, "IP", MAX_INPUT_LENGTH);
                    new_ban.name[0] = '\0';
                    strlcpy(new_ban.ip, buf, MAX_INPUT_LENGTH);

                    if(remove_ban(&new_ban)) {
		        cprintf(ch, "%s is no longer banned!\r\n", new_ban.ip);
                        log_auth(ch, "BAN IP %s has been unbanned!", new_ban.ip);
                    } else {
                        cprintf(ch, "Could not remove ban for %s.\r\n", new_ban.ip);
                        log_auth(ch, "Failed to remove ban for %s.\r\n", new_ban.ip);
                    }
		    return;
		}
	    }
	}
    }
    cprintf(ch, "Usage: unban list\r\n       unban name <name-to-ban>\r\n       unban ip <ip-to-ban>\r\n");
}

void setup_bans_table(void) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    char *sql = "CREATE TABLE IF NOT EXISTS bans ( "
                "    updated TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT now(), "
                "    expires TIMESTAMP WITH TIME ZONE, "
                "    enabled BOOLEAN DEFAULT true, "
                "    ban_type TEXT NOT NULL, "
                "    name TEXT, "
                "    ip TEXT, "
                "    set_by TEXT DEFAULT 'SYSTEM', "
                "    reason TEXT DEFAULT 'Naughty.', "
                "    CONSTRAINT bans_type_name UNIQUE(ban_type, name), "
                "    CONSTRAINT bans_type_ip UNIQUE(ban_type, ip), "
                "    CONSTRAINT bans_type_name_ip UNIQUE(ban_type, name, ip) "
                "); ";
    char *sql2 = "CREATE INDEX IF NOT EXISTS ix_bans_enabled ON bans (enabled);";
    char *sql3 = "CREATE INDEX IF NOT EXISTS ix_bans_type ON bans (ban_type);";
    char *sql4 = "CREATE INDEX IF NOT EXISTS ix_bans_name ON bans (name);";
    char *sql5 = "CREATE INDEX IF NOT EXISTS ix_bans_ip ON bans (ip);";

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
        log_fatal("Cannot create bans enabled index: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);

    res = PQexec(db_wileymud.dbc, sql3);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create bans type index: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);

    res = PQexec(db_wileymud.dbc, sql4);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create bans name index: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);

    res = PQexec(db_wileymud.dbc, sql5);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create bans ip index: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);
}

void load_bans(void) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    char *sql = "SELECT count(*) FROM bans WHERE enabled;";
    char *sql2 = "SELECT extract(EPOCH from updated) AS updated, "
                 "       extract(EPOCH from expires) AS expires, "
                 "       enabled::integer, ban_type, name, ip, set_by, reason, "
                 "       to_char(updated AT TIME ZONE 'US/Pacific', 'YYYY-MM-DD') "
                 "           AS updated_text "
                 "FROM bans "
                 "WHERE enabled;";
    int rows = 0;
    int columns = 0;
    int count = 0;

    unload_bans();

    sql_connect(&db_wileymud);
    res = PQexec(db_wileymud.dbc, sql);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot get row count of bans table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    rows = PQntuples(res);
    columns = PQnfields(res);
    if(rows > 0 && columns > 0 ) {
        count = atoi(PQgetvalue(res,0,0));
    }
    PQclear(res);

    if(count > 0) {
        log_boot("  Loading ban data from SQL database.");
        ban_list_count = count;

        res = PQexec(db_wileymud.dbc, sql2);
        st = PQresultStatus(res);
        if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
            log_fatal("Cannot load bans from bans table: %s", PQerrorMessage(db_wileymud.dbc));
            PQclear(res);
            proper_exit(MUD_HALT);
        }
        rows = PQntuples(res);
        columns = PQnfields(res);
        if( rows > 0 && columns > 0 ) {
            ban_list = (struct ban_data *)calloc(count, sizeof(struct ban_data));
            for( int i = 0; i < rows; i++) {
                ban_list[i].updated = (time_t)atoi(PQgetvalue(res,i,0));
                ban_list[i].expires = (time_t)atoi(PQgetvalue(res,i,1));
                // We typecast enabled as integer to avoid 't' and 'f'.
                ban_list[i].enabled = (int)atoi(PQgetvalue(res,i,2));
                strlcpy(ban_list[i].ban_type, PQgetvalue(res,i,3), MAX_INPUT_LENGTH);
                strlcpy(ban_list[i].name, PQgetvalue(res,i,4), MAX_INPUT_LENGTH);
                strlcpy(ban_list[i].ip, PQgetvalue(res,i,5), MAX_INPUT_LENGTH);
                strlcpy(ban_list[i].set_by, PQgetvalue(res,i,6), MAX_INPUT_LENGTH);
                strlcpy(ban_list[i].reason, PQgetvalue(res,i,7), MAX_INPUT_LENGTH);
                strlcpy(ban_list[i].updated_text, PQgetvalue(res,i,8), MAX_INPUT_LENGTH);
            }
        }
        PQclear(res);
    } else {
        log_boot("  There are no bans to load, yet.");
    }
}

void unload_bans(void) {
    if(ban_list)
        free(ban_list);
    ban_list = NULL;
    ban_list_count = 0;
    log_info("Ban list unloaded");
}

int ban_address(const char *ip_addr, const char *reason) {
    struct ban_data                         new_ban;

    new_ban.updated = time(NULL);
    new_ban.expires = -1;
    new_ban.enabled = 1;
    strcpy(new_ban.ban_type, "IP");
    new_ban.name[0] = '\0';
    strlcpy(new_ban.ip, ip_addr, MAX_INPUT_LENGTH);
    strcpy(new_ban.set_by, "SYSTEM");
    strlcpy(new_ban.reason, (reason && *reason) ? reason : "Spam", MAX_INPUT_LENGTH);

    return add_ban(&new_ban);
}

// NOTE:  Because the ban unix timestamps are integers, if you want to pass in
//        NULL for database storage as NULL, you have to pass in a -1 for that field
//        and this code has to handle it.
int add_ban(struct ban_data *pal) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    const char *sql_name = "INSERT INTO bans ( ban_type, expires, name, set_by, reason ) "
                           "VALUES ('NAME', to_timestamp($1),$2,$3,$4) "
                           "ON CONFLICT ON CONSTRAINT bans_type_name "
                           "DO UPDATE SET updated = now(), "
                           "              enabled = true, "
                           "              ban_type = 'NAME', "
                           "              expires = to_timestamp($1), "
                           "              name = $2, ip = NULL, "
                           "              set_by = $3, reason = $4;";
    const char *sql_ip   = "INSERT INTO bans ( ban_type, expires, ip, set_by, reason ) "
                           "VALUES ('IP', to_timestamp($1),$2,$3,$4) "
                           "ON CONFLICT ON CONSTRAINT bans_type_ip "
                           "DO UPDATE SET updated = now(), "
                           "              enabled = true, "
                           "              ban_type = 'IP', "
                           "              expires = to_timestamp($1), "
                           "              name = NULL, ip = $2, "
                           "              set_by = $3, reason = $4;";
    const char *sql_at   = "INSERT INTO bans ( ban_type, expires, name, ip, set_by, reason ) "
                           "VALUES ('NAME_AT_IP', to_timestamp($1),$2,$3,$4,$5) "
                           "ON CONFLICT ON CONSTRAINT bans_type_name_ip "
                           "DO UPDATE SET updated = now(), "
                           "              enabled = true, "
                           "              ban_type = 'NAME_AT_IP', "
                           "              expires = to_timestamp($1), "
                           "              name = $2, ip = $3, "
                           "              set_by = $4, reason = $5;";

    const char *param_val[5];
    int param_len[5];
    int param_bin[5] = {1,0,0,0,0};
    time_t param_expires;

    if(pal->expires >= 0)
        param_expires = htonl(pal->expires);
    param_val[0] = (pal->expires >= 0) ? (char *)&param_expires : NULL;
    param_len[0] = (pal->expires >= 0) ? sizeof(param_expires) : 0;

    sql_connect(&db_wileymud);
    if(!strcasecmp(pal->ban_type, "NAME")) {
        param_val[1] = (pal->name[0]) ? pal->name : NULL;
        param_val[2] = (pal->set_by[0]) ? pal->set_by : NULL;
        param_val[3] = (pal->reason[0]) ? pal->reason : NULL;
        param_len[1] = (pal->name[0]) ? strlen(pal->name) : 0;
        param_len[2] = (pal->set_by[0]) ? strlen(pal->set_by) : 0;
        param_len[3] = (pal->reason[0]) ? strlen(pal->reason) : 0;

        res = PQexecParams(db_wileymud.dbc, sql_name, 4, NULL, param_val, param_len, param_bin, 0);
    } else if(!strcasecmp(pal->ban_type, "IP")) {
        param_val[1] = (pal->ip[0]) ? pal->ip : NULL;
        param_val[2] = (pal->set_by[0]) ? pal->set_by : NULL;
        param_val[3] = (pal->reason[0]) ? pal->reason : NULL;
        param_len[1] = (pal->ip[0]) ? strlen(pal->ip) : 0;
        param_len[2] = (pal->set_by[0]) ? strlen(pal->set_by) : 0;
        param_len[3] = (pal->reason[0]) ? strlen(pal->reason) : 0;

        res = PQexecParams(db_wileymud.dbc, sql_ip, 4, NULL, param_val, param_len, param_bin, 0);
    } else if(!strcasecmp(pal->ban_type, "NAME_AT_IP")) {
        param_val[1] = (pal->name[0]) ? pal->name : NULL;
        param_val[2] = (pal->ip[0]) ? pal->ip : NULL;
        param_val[3] = (pal->set_by[0]) ? pal->set_by : NULL;
        param_val[4] = (pal->reason[0]) ? pal->reason : NULL;
        param_len[1] = (pal->name[0]) ? strlen(pal->name) : 0;
        param_len[2] = (pal->ip[0]) ? strlen(pal->ip) : 0;
        param_len[3] = (pal->set_by[0]) ? strlen(pal->set_by) : 0;
        param_len[4] = (pal->reason[0]) ? strlen(pal->reason) : 0;

        res = PQexecParams(db_wileymud.dbc, sql_at, 5, NULL, param_val, param_len, param_bin, 0);
    } else {
        log_error("INVALID ban type, %s.", VNULL(pal->ban_type));
        return 0;
    }
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_error("Cannot add ban: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        return 0;
        //proper_exit(MUD_HALT);
    }
    PQclear(res);

    load_bans();
    return 1;
}

int remove_ban(struct ban_data *pal) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    const char *param_val[2];
    int param_len[2];
    int param_bin[2] = {0,0};
    char *sql_name =  "UPDATE bans SET updated = now(), enabled = false WHERE ban_type = 'NAME' AND name = $1 AND ip IS NULL;";
    char *sql_ip   =  "UPDATE bans SET updated = now(), enabled = false WHERE ban_type = 'IP' AND name IS NULL AND ip = $1;";
    char *sql_at   =  "UPDATE bans SET updated = now(), enabled = false WHERE ban_type = 'NAME_AT_IP' AND name = $1 AND ip = $2;";
    int rows = 0;

    sql_connect(&db_wileymud);
    if(!strcasecmp(pal->ban_type, "NAME")) {
        param_val[0] = (pal->name[0]) ? pal->name : NULL;
        param_len[0] = (pal->name[0]) ? strlen(pal->name) : 0;

        res = PQexecParams(db_wileymud.dbc, sql_name, 1, NULL, param_val, param_len, param_bin, 0);
    } else if(!strcasecmp(pal->ban_type, "IP")) {
        param_val[0] = (pal->ip[0]) ? pal->ip : NULL;
        param_len[0] = (pal->ip[0]) ? strlen(pal->ip) : 0;

        res = PQexecParams(db_wileymud.dbc, sql_ip, 1, NULL, param_val, param_len, param_bin, 0);
    } else if(!strcasecmp(pal->ban_type, "NAME_AT_IP")) {
        param_val[0] = (pal->name[0]) ? pal->name : NULL;
        param_val[1] = (pal->ip[0]) ? pal->ip : NULL;
        param_len[0] = (pal->name[0]) ? strlen(pal->name) : 0;
        param_len[1] = (pal->ip[0]) ? strlen(pal->ip) : 0;

        res = PQexecParams(db_wileymud.dbc, sql_at, 2, NULL, param_val, param_len, param_bin, 0);
    } else {
        log_error("INVALID ban type, %s.", VNULL(pal->ban_type));
        return 0;
    }

    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_error("Cannot disable ban from bans table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        return 0;
        //proper_exit(MUD_HALT);
    }

    rows = atoi(PQcmdTuples(res));
    if(rows > 0) {
        log_info("Disabled %d rows from bans table for (%s,%s)", rows, pal->name, pal->ip);
    } else {
        log_info("No rows disabled from bans table for (%s,%s)", pal->name, pal->ip);
    }
    PQclear(res);

    load_bans();
    return 1;
}


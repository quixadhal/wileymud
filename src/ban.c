#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

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
        for(int i = 0; i < ban_list_count; i++) {
            if(!strcasecmp(ban_list[i].ban_type, "NAME"))
                cprintf(ch, "NAME - %-20s\r\n", ban_list[i].name);
        }
        for(int i = 0; i < ban_list_count; i++) {
            if(!strcasecmp(ban_list[i].ban_type, "IP"))
                cprintf(ch, "  IP - %-20s\r\n", ban_list[i].ip);
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


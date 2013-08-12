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

#define __BAN_C__
#include "ban.h"

static char                           **banned_names = NULL;
static char                           **banned_ips = NULL;
static char                           **banned_at = NULL;
static int                              banned_names_count = 0;
static int                              banned_ips_count = 0;
static int                              banned_at_count = 0;

void unload_bans(void)
{
    if (banned_names) {
	log_boot("- Unloading banned name list");
	for (int i = 0; i < banned_names_count; i++)
	    if (banned_names[i])
		free(banned_names[i]);

	free(banned_names);
	banned_names = NULL;
	banned_names_count = 0;
    }
    if (banned_ips) {
	log_boot("- Unloading banned ip list");
	for (int i = 0; i < banned_ips_count; i++)
	    if (banned_ips[i])
		free(banned_ips[i]);

	free(banned_ips);
	banned_ips = NULL;
	banned_ips_count = 0;
    }
    if (banned_at) {
	log_boot("- Unloading banned name@ip list");
	for (int i = 0; i < banned_at_count; i++)
	    if (banned_at[i])
		free(banned_at[i]);

	free(banned_at);
	banned_at = NULL;
	banned_at_count = 0;
    }
}

void load_bans(void)
{
    FILE                                   *fp = NULL;
    int                                     tmp_count;

    unload_bans();

    log_boot("- Loading banned name list from %s", BANNED_NAME_FILE);
    if (fp = fopen(BANNED_NAME_FILE, "r")) {
	fscanf(fp, "%d", &tmp_count);

	banned_names = calloc(tmp_count, sizeof(char *));
	for (int i = 0; i < tmp_count; i++) {
	    char                                    tmp[MAX_STRING_LENGTH];

	    fscanf(fp, "% s", &tmp);
	    banned_names[i] = strdup(tmp);
	}
	fclose(fp);
    }

    log_boot("- Loading banned ip list from %s", BANNED_IP_FILE);
    if (fp = fopen(BANNED_IP_FILE, "r")) {
	fscanf(fp, "%d", &tmp_count);

	banned_ips = calloc(tmp_count, sizeof(char *));
	for (int i = 0; i < tmp_count; i++) {
	    char                                    tmp[MAX_STRING_LENGTH];

	    fscanf(fp, "% s", &tmp);
	    banned_ips[i] = strdup(tmp);
	}
	fclose(fp);
    }

    log_boot("- Loading banned name@ip list from %s", BANNED_AT_FILE);
    if (fp = fopen(BANNED_AT_FILE, "r")) {
	fscanf(fp, "%d", &tmp_count);

	banned_at = calloc(tmp_count, sizeof(char *));
	for (int i = 0; i < tmp_count; i++) {
	    char                                    tmp[MAX_STRING_LENGTH];

	    fscanf(fp, "% s", &tmp);
	    banned_at[i] = strdup(tmp);
	}
	fclose(fp);
    }
}

/* Returns TRUE if this character name would be acceptable for a
 * new player.
 * Returns FALSE if it fails any checks, including being an
 * already existing character or mob, an illegal name, banned, etc.
 */
int acceptable_name(const char *name)
{
    char                                    tmp_name[20] = "\0\0\0";
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

void do_ban(struct char_data *ch, const char *argument, int cmd)
{
    char                                    ban_type[MAX_STRING_LENGTH] = "\0\0\0";
    int                                     i = 0;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_NPC(ch)) {
	cprintf(ch, "You're a mob, you can't ban anyone.\r\n");
	return;
    }
    if (argument && *argument) {
	argument = one_argument(argument, ban_type);
	only_argument(argument, buf);
	if (*ban_type) {
	    if (!str_cmp(ban_type, "name")) {
		if (*buf) {
		    /*
		     * First, we need to make sure it isn't already a mob or player. then we can try adding it to the
		     * ban table. 
		     */
		    if (!acceptable_name(buf)) {
			cprintf(ch, "%s is already an invalid choice.\r\n", buf);
			return;
		    }

		    banned_names_count++;
		    banned_names = realloc(banned_names, banned_names_count * sizeof(char *));
		    banned_names[banned_names_count - 1] = strdup(buf);
		    cprintf(ch, "%s is now banned!\r\n", buf);
		    log_auth(ch, "BAN %s has been banned by %s!", buf, GET_NAME(ch));
		    save_bans();
		    return;
		} else {
		    cprintf(ch, "Banned names:\r\n");
		    for (i = 0; i < banned_names_count; i++) {
			cprintf(ch, "%-20s\r\n", banned_names[i]);
		    }
		    return;
		}
	    } else if (!str_cmp(ban_type, "ip") || !str_cmp(ban_type, "address")
		       || !str_cmp(ban_type, "site")) {
		if (*buf) {
		    /*
		     * No banning localhost! 
		     */
		    if (str_cmp("127.0.0.1", buf)) {
			cprintf(ch, "You cannot ban localhost!\r\n");
			return;
		    }
		    banned_ips_count++;
		    banned_ips = realloc(banned_ips, banned_ips_count * sizeof(char *));
		    banned_ips[banned_ips_count - 1] = strdup(buf);
		    cprintf(ch, "%s is now banned!\r\n", buf);
		    log_auth(ch, "BAN %s has been banned by %s!", buf, GET_NAME(ch));
		    save_bans();
		    return;
		} else {
		    cprintf(ch, "Banned IP addresses:\r\n");
		    for (i = 0; i < banned_ips_count; i++) {
			cprintf(ch, "%-20s\r\n", banned_ips[i]);
		    }
		    return;
		}
	    }
	}
    }
    cprintf(ch, "Usage: ban < name|ip > [ name|address ]\r\n");
}

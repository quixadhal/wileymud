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

static char                           **banned_names = NULL;
static char                           **banned_ips = NULL;
static char                           **banned_at_names = NULL;
static char                           **banned_at_ips = NULL;
static int                              banned_names_count = 0;
static int                              banned_ips_count = 0;
static int                              banned_at_count = 0;

void unload_bans(void)
{
    int                                     i;

    if (banned_names) {
	log_boot("- Unloading banned name list");
	for (i = 0; i < banned_names_count; i++)
	    if (banned_names[i])
		free(banned_names[i]);

	free(banned_names);
	banned_names = NULL;
	banned_names_count = 0;
    }
    if (banned_ips) {
	log_boot("- Unloading banned ip list");
	for (i = 0; i < banned_ips_count; i++)
	    if (banned_ips[i])
		free(banned_ips[i]);

	free(banned_ips);
	banned_ips = NULL;
	banned_ips_count = 0;
    }
    if (banned_at_names && banned_at_ips) {
	log_boot("- Unloading banned name@ip list");
	for (i = 0; i < banned_at_count; i++) {
	    if (banned_at_names[i])
		free(banned_at_names[i]);
	    if (banned_at_ips[i])
		free(banned_at_ips[i]);
        }

	free(banned_at_names);
	free(banned_at_ips);
	banned_at_names = NULL;
	banned_at_ips = NULL;
	banned_at_count = 0;
    }
}

void load_bans(void)
{
    FILE                                   *fp = NULL;
    int                                     i;

    unload_bans();

    log_boot("- Loading banned name list from %s", BAN_FILE);
    if ((fp = fopen(BAN_FILE, "r"))) {
        banned_names_count = fread_number(fp);
	banned_names = calloc(banned_names_count, sizeof(char *));
	for (i = 0; i < banned_names_count; i++) {
            banned_names[i] = strdup(new_fread_string(fp));
	}

        log_boot("- Loading banned ip list from %s", BAN_FILE);
        banned_ips_count = fread_number(fp);
	banned_ips = calloc(banned_ips_count, sizeof(char *));
	for (i = 0; i < banned_ips_count; i++) {
            banned_ips[i] = strdup(new_fread_string(fp));
	}

        log_boot("- Loading banned name@ip list from %s", BAN_FILE);
        banned_at_count = fread_number(fp);
	banned_at_names = calloc(banned_at_count, sizeof(char *));
	banned_at_ips = calloc(banned_at_count, sizeof(char *));
	for (i = 0; i < banned_at_count; i++) {
            banned_at_names[i] = strdup(new_fread_string(fp));
            banned_at_ips[i] = strdup(new_fread_string(fp));
	}
	fclose(fp);
    }
}

void save_bans(void)
{
    FILE                                   *fp = NULL;
    int                                     i;

    if(!(fp = fopen(BAN_FILE, "w"))) {
        log_error("Cannot open %s for writing!\n", BAN_FILE);
        return;
    }
    log_boot("- Saving ban data to %s", BAN_FILE);
    fprintf(fp, "%d\n", banned_names_count);
    for (i = 0; i < banned_names_count; i++) {
        fprintf(fp, "%s~\n", banned_names[i]);
    }
    fprintf(fp, "%d\n", banned_ips_count);
    for (i = 0; i < banned_ips_count; i++) {
        fprintf(fp, "%s~\n", banned_ips[i]);
    }
    fprintf(fp, "%d\n", banned_at_count);
    for (i = 0; i < banned_at_count; i++) {
        fprintf(fp, "%s~\n%s~\n", banned_at_names[i], banned_at_ips[i]);
    }
    fclose(fp);
}

int banned_name(char *name)
{
    int i;

    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, VNULL(name));

    if (banned_names_count < 1)
        return FALSE;

    for(i = 0; i < banned_names_count; i++) {
        if (!str_cmp(banned_names[i], name))
            return TRUE;
    }

    return FALSE;
}

int banned_ip(char *ip)
{
    int i;

    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, VNULL(ip));

    if (banned_ips_count < 1)
        return FALSE;

    for(i = 0; i < banned_ips_count; i++) {
        if (!str_cmp(banned_ips[i], ip))
            return TRUE;
    }

    return FALSE;
}

int banned_at(char *name, char *ip)
{
    int i;

    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, VNULL(name));

    if (banned_at_count < 1)
        return FALSE;

    for(i = 0; i < banned_at_count; i++) {
        if (!str_cmp(banned_at_names[i], name) && !str_cmp(banned_at_ips[i], ip))
            return TRUE;
    }

    return FALSE;
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
    char                                    ban_type[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    char                                    buf[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
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
		    if (!str_cmp("127.0.0.1", buf)) {
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

void do_unban(struct char_data *ch, const char *argument, int cmd)
{
    char                                    ban_type[MAX_STRING_LENGTH] = "\0\0\0";
    char                                    buf[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    int                                     i = 0;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_NPC(ch)) {
	cprintf(ch, "You're a mob, you can't unban anyone.\r\n");
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
		    if (!banned_name(buf)) {
			cprintf(ch, "%s is not banned.\r\n", buf);
			return;
		    }
                    for (i = 0; i < banned_names_count; i++) {
                        if (!str_cmp(banned_names[i], buf)) {
                            char **tmp_foo = calloc(banned_names_count - 1, sizeof(char *));
                            int j = 0;
                            for( j = 0; j < i; j++ ) {
                                tmp_foo[j] = banned_names[j];
                            }
                            for( j = i + 1; j < banned_names_count; j++ ) {
                                tmp_foo[j-1] = banned_names[j];
                            }
                            free(banned_names[i]);
                            banned_names_count--;
                            free(banned_names);
                            banned_names = tmp_foo;
                            cprintf(ch, "%s is no longer banned!\r\n", buf);
                            log_auth(ch, "BAN %s has been unbanned by %s!", buf, GET_NAME(ch));
                            save_bans();
                            return;
                        }
                    }
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
		    if (!banned_ip(buf)) {
			cprintf(ch, "%s is not banned.\r\n", buf);
			return;
		    }
                    for (i = 0; i < banned_names_count; i++) {
                        if (!str_cmp(banned_ips[i], buf)) {
                            char **tmp_foo = calloc(banned_ips_count - 1, sizeof(char *));
                            int j = 0;
                            for( j = 0; j < i; j++ ) {
                                tmp_foo[j] = banned_ips[j];
                            }
                            for( j = i + 1; j < banned_ips_count; j++ ) {
                                tmp_foo[j-1] = banned_ips[j];
                            }
                            free(banned_ips[i]);
                            banned_ips_count--;
                            free(banned_ips);
                            banned_ips = tmp_foo;
                            cprintf(ch, "%s is no longer banned!\r\n", buf);
                            log_auth(ch, "BAN %s has been unbanned by %s!", buf, GET_NAME(ch));
                            save_bans();
                            return;
                        }
                    }
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
    cprintf(ch, "Usage: unban < name|ip > [ name|address ]\r\n");
}


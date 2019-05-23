#ifndef _BAN_H
#define _BAN_H

#define BANNED_NAME_FILE  "adm/banned_names"
#define BANNED_IP_FILE    "adm/banned_ips"
#define BAN_FILE          "adm/bans"

void					unload_bans(void);
void					load_bans(void);
void					save_bans(void);
int                                     acceptable_name( const char *name );
void                                    do_ban(struct char_data *ch, const char *argument, int cmd);
void                                    do_unban(struct char_data *ch, const char *argument, int cmd);

int                                     banned_ip(char *ip);
int                                     banned_name(char *name);
int                                     banned_at(char *name, char *ip);

#endif

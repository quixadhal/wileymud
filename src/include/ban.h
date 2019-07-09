#ifndef _BAN_H
#define _BAN_H

// If name is set, but ip is NULL, the ban is for that character name
// no matter where they might try to connect from, and character creation
// disallows the use of that name entirely.
//
// If ip is set, but name is NULL, the ban is for all connections from
// that address, and the connection is dropped as early as possible.
//
// If both are set, the ban is a particular user@address.  If the user
// logs in from elsewhere, they will be allowed... and new characters of
// that name can be made if that user doesn't exist anymore.
struct ban {
    int     updated;                        // The ban was last updated at this time.
    int     expires;                        // The ban expires at this timestamp, never if 0.
    char    name[MAX_INPUT_LENGTH];         // The character name banned.
    char    ip[MAX_INPUT_LENGTH];           // The address banned.
    char    banned_by[MAX_INPUT_LENGTH];    // The immortal that banned you.
    char    reason[MAX_STRING_LENGTH];      // The reason for being banned, if not NULL.
};

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

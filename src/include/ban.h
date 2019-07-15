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
struct ban_data {
    time_t  updated;                        // The ban was last updated at this time.
    time_t  expires;                        // The ban expires at this timestamp, never if 0.
    int     enabled;                        // Is this ban enabled or disabled?
    char    ban_type[MAX_INPUT_LENGTH];     // Needed because of NULL != NULL issues.
    char    name[MAX_INPUT_LENGTH];         // The character name banned.
    char    ip[MAX_INPUT_LENGTH];           // The address banned.
    char    set_by[MAX_INPUT_LENGTH];       // The immortal that banned you.
    char    reason[MAX_STRING_LENGTH];      // The reason for being banned, if not NULL.
};

#ifndef _BAN_C
extern struct ban_data  *ban_list;
extern int              ban_list_count;
#endif

int         acceptable_name(const char *name);
int         banned_ip(char *ip);
int         banned_name(char *name);
int         banned_at(char *name, char *ip);

void        do_ban(struct char_data *ch, const char *argument, int cmd);
void        do_unban(struct char_data *ch, const char *argument, int cmd);

#endif

/****************************************************************************
 *                   ^     +----- |  / ^     ^ |     | +-\                  *
 *                  / \    |      | /  |\   /| |     | |  \                 *
 *                 /   \   +---   |<   | \ / | |     | |  |                 *
 *                /-----\  |      | \  |  v  | |     | |  /                 *
 *               /       \ |      |  \ |     | +-----+ +-/                  *
 ****************************************************************************
 * AFKMud Copyright 1997-2006 by Roger Libiez (Samson),                     *
 * Levi Beckerson (Whir), Michael Ward (Tarl), Erik Wolfe (Dwip),           *
 * Cameron Carroll (Cam), Cyberfox, Karangi, Rathian, Raine, and Adjani.    *
 * All Rights Reserved.                                                     *
 * Registered with the United States Copyright Office. TX 5-877-286         *
 *                                                                          *
 * External contributions from Xorith, Quixadhal, Zarius, and many others.  *
 *                                                                          *
 * Original SMAUG 1.4a written by Thoric (Derek Snider) with Altrag,        *
 * Blodkai, Haus, Narn, Scryn, Swordbearer, Tricops, Gorog, Rennard,        *
 * Grishnakh, Fireblade, and Nivek.                                         *
 *                                                                          *
 * Original MERC 2.1 code by Hatchet, Furey, and Kahn.                      *
 *                                                                          *
 * Original DikuMUD code by: Hans Staerfeldt, Katja Nyboe, Tom Madsen,      *
 * Michael Seifert, and Sebastian Hammer.                                   *
 ****************************************************************************
 *                       Intermud-3 Network Module                          *
 ****************************************************************************/

/*
 * Copyright (c) 2000 Fatal Dimensions
 *
 * See the file "LICENSE" or information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/* Ported to Smaug 1.4a by Samson of Alsherok.
 * Consolidated for cross-codebase compatibility by Samson of Alsherok.
 * Modifications and enhancements to the code
 * Copyright (c)2001-2003 Roger Libiez ( Samson )
 * Registered with the United States Copyright Office
 * TX 5-562-404
 *
 * I condensed the 14 or so Fatal Dimensions source code files into this
 * one file, because I for one find it far easier to maintain when all of
 * the functions are right here in one file.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fnmatch.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <signal.h>
#include "global.h"
#include "mudlimits.h"
#include "sql.h"
#include "bug.h"
#include "utils.h"
#include "db.h"
#include "multiclass.h"
#include "comm.h"
#include "modify.h"
#include "interpreter.h"
#include "version.h"
#include "stringmap.h"
#include "scheduler.h"
#include "discord.h"
#define _I3_C
#include "i3.h"

/* Global variables for I3 */

#define I3_PACKET_STATE_NONE 0
#define I3_PACKET_STATE_GOT_SIZE 1
#define I3_PACKET_STATE_READING 2
#define I3_PACKET_STATE_GOT_PACKET 3
#define I3_PACKET_STATE_PROCESSING 4
#define I3_PACKET_STATE_LEFTOVERS 5
#define I3_PACKET_STATE_MISSING 6

char I3_incoming_packet_size[4];
uint32_t I3_incoming_packet_length = 0;
char *I3_incoming_packet = NULL;
long I3_incoming_packet_read = 0;
int I3_packet_processing_state = I3_PACKET_STATE_NONE;
char *I3_incoming_packet_leftovers = NULL;
long I3_incoming_packet_strlen = 0;
stringMap *speaker_db = NULL;
int speaker_count = 0;

stringMap *pinkfish_to_ansi_db = NULL;
int pinkfish_to_ansi_count = 0;
stringMap *pinkfish_to_i3_db = NULL;
int pinkfish_to_i3_count = 0;
stringMap *pinkfish_to_xterm256_db = NULL;
int pinkfish_to_xterm256_count = 0;
stringMap *pinkfish_to_greyscale_db = NULL;
int pinkfish_to_greyscale_count = 0;
stringMap *pinkfish_to_null_db = NULL;
int pinkfish_to_null_count = 0;

char I3_input_buffer[IPS];
char I3_output_buffer[OPS];
char I3_currentpacket[IPS];
bool packetdebug = FALSE; /* Packet debugging toggle, can be turned on to check
                           * outgoing packets */
long I3_input_pointer = 0;
long I3_output_pointer = 4;
#define I3_THISMUD (this_i3mud->name)
char *I3_ROUTER_NAME;
char *I3_ROUTER_IP;
const char *manual_router;
int I3_socket;
int i3wait;              /* Number of game loops to wait before attempting to
                          * reconnect when a socket dies */
int i3justconnected = 0; // So we can say something for the logs.
time_t ucache_clock;     /* Timer for pruning the ucache */
long channel_m_received;
long channel_m_sent;
long bytes_received;
long bytes_sent;
time_t i3_time = 0; /* Current clock time for the client */
time_t connected_at = 0;
time_t connect_started_at = 0;
int connection_timeouts = 0;
time_t uptime = 0;
time_t record_uptime = 0;
time_t lag_spike = 0;
time_t record_lag_spike = 0;

I3_MUD *this_i3mud = NULL;
I3_MUD *first_mud;
I3_MUD *last_mud;

I3_CHANNEL *first_I3chan;
I3_CHANNEL *last_I3chan;
I3_BAN *first_i3ban;
I3_BAN *last_i3ban;
UCACHE_DATA *first_ucache;
UCACHE_DATA *last_ucache;
ROUTER_DATA *first_router;
ROUTER_DATA *last_router;
I3_COLOR *first_i3_color;
I3_COLOR *last_i3_color;
I3_CMD_DATA *first_i3_command;
I3_CMD_DATA *last_i3_command;
I3_HELP_DATA *first_i3_help;
I3_HELP_DATA *last_i3_help;

int router_reconnect_short_delay = 15 * PULSE_PER_SECOND;
int router_reconnect_medium_delay = 60 * PULSE_PER_SECOND;
int router_reconnect_long_delay = 300 * PULSE_PER_SECOND;

// These two are for the cases where we're going to be handling multiple requests that
// could end up with the same timestamp (even with milliseconds).  In those cases, we need
// to add an incrementing value so we can still sort by time (timestamp + sub_second_counter)
time_t last_second = 0;
int sub_second_counter = 0;

// To move away from "ticks", we need to start using microsecond timing info.
int64_t time_to_taunt = (int64_t)1730793600 * (int64_t)1000000;
int64_t timeout_marker = (int64_t)1730793600 * (int64_t)1000000;
int expecting_timeout = 0;

void i3_printf(CHAR_DATA *ch, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
void i3wrap_printf(CHAR_DATA *ch, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
void i3page_printf(CHAR_DATA *ch, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
I3_HEADER *I3_get_header(char **pps);
void I3_send_channel_listen(I3_CHANNEL *channel, bool lconnect);
void I3_write_channel_config(void);
const char *i3_funcname(I3_FUN *func);
I3_FUN *i3_function(const char *func);
void I3_saveconfig(void);
void to_channel(const char *argument, char *xchannel, int level);
void I3_connection_close(bool reconnect);
char *i3rankbuffer(CHAR_DATA *ch);
char *I3_nameescape(const char *ps);
char *I3_nameremap(const char *ps);
void i3_npc_chat(const char *chan_name, const char *actor, const char *message);
void i3_npc_speak(const char *chan_name, const char *actor, const char *message);
void I3_packet_cleanup(void);

// void                                    i3_nuke_url_file(void);
// void                                    i3_check_urls(void);

const char *perm_names[] = {"Notset", "None", "Mort", "Imm", "Admin", "Imp"};

/*******************************************
 * String buffering and logging functions. *
 ******************************************/

const char *i3one_argument(const char *argument, char *arg_first)
{
    char cEnd;
    int count;

    count = 0;

    if (arg_first)
        arg_first[0] = '\0';

    if (!argument || argument[0] == '\0')
        return NULL;

    while (isspace(*argument))
        argument++;

    cEnd = ' ';
    if (*argument == '\'' || *argument == '"')
        cEnd = *argument++;

    while (*argument != '\0' && ++count <= MAX_INPUT_LENGTH - 1)
    {
        if (*argument == cEnd)
        {
            argument++;
            break;
        }

        if (arg_first)
            *arg_first++ = *argument++;
        else
            argument++;
    }

    if (arg_first)
        *arg_first = '\0';

    while (isspace(*argument))
        argument++;

    return argument;
}

char *I3_imctag_to_i3tag(const char *txt)
{
    I3_COLOR *color;
    static char tbuf[MAX_STRING_LENGTH];

    *tbuf = '\0';
    if (!txt || *txt == '\0')
        return tbuf;

    strlcpy(tbuf, txt, MAX_STRING_LENGTH);
    for (color = first_i3_color; color; color = color->next)
        strlcpy(tbuf, strrep(tbuf, color->imctag, color->i3tag), MAX_STRING_LENGTH);

    return tbuf;
}

char *I3_imctag_to_mudtag(CHAR_DATA *ch, const char *txt)
{
    I3_COLOR *color;
    static char tbuf[MAX_STRING_LENGTH];

    *tbuf = '\0';
    if (!txt || *txt == '\0')
        return tbuf;

    if (I3IS_SET(I3FLAG(ch), I3_COLORFLAG))
    {
        strlcpy(tbuf, txt, MAX_STRING_LENGTH);
        for (color = first_i3_color; color; color = color->next)
            strlcpy(tbuf, strrep(tbuf, color->imctag, color->mudtag), MAX_STRING_LENGTH);
    }
    else
        strlcpy(tbuf, i3_strip_colors(txt), MAX_STRING_LENGTH);

    return tbuf;
}

#if 0

char *i3_strip_colors(const char *txt)
{
    I3_COLOR *color;
    static char tbuf[MAX_STRING_LENGTH];

    strlcpy(tbuf, txt, MAX_STRING_LENGTH);

    for (color = first_i3_color; color; color = color->next)
        strlcpy(tbuf, strrep(tbuf, color->i3tag, ""), MAX_STRING_LENGTH);

    for (color = first_i3_color; color; color = color->next)
        strlcpy(tbuf, strrep(tbuf, color->mudtag, ""), MAX_STRING_LENGTH);

    return tbuf;
}

char *I3_mudtag_to_i3tag(const char *txt)
{
    I3_COLOR *color;
    static char tbuf[MAX_STRING_LENGTH];

    *tbuf = '\0';
    if (!txt || *txt == '\0')
        return tbuf;

    strlcpy(tbuf, txt, MAX_STRING_LENGTH);
    for (color = first_i3_color; color; color = color->next)
        strlcpy(tbuf, strrep(tbuf, color->mudtag, color->i3tag), MAX_STRING_LENGTH);

    return tbuf;
}

char *I3_i3tag_to_mudtag(CHAR_DATA *ch, const char *txt)
{
    I3_COLOR *color;
    static char tbuf[MAX_STRING_LENGTH];

    *tbuf = '\0';
    if (!txt || *txt == '\0')
	return tbuf;

    if (I3IS_SET(I3FLAG(ch), I3_COLORFLAG)) {
	strlcpy(tbuf, txt, MAX_STRING_LENGTH);
	for (color = first_i3_color; color; color = color->next)
	    strlcpy(tbuf, strrep(tbuf, color->i3tag, color->mudtag), MAX_STRING_LENGTH);
    } else
	strlcpy(tbuf, i3_strip_colors(txt), MAX_STRING_LENGTH);

    return tbuf;
}

#else

char *i3_strip_colors(const char *txt)
{
    return pinkfish_to_null(txt);
}

char *I3_mudtag_to_i3tag(const char *txt)
{
    return pinkfish_to_i3(txt);
}

char *I3_i3tag_to_mudtag(CHAR_DATA *ch, const char *txt)
{
    return (I3IS_SET(I3FLAG(ch), I3_COLORFLAG)) ? pinkfish_to_xterm(txt) : pinkfish_to_null(txt);
}

#endif

/********************************
 * User level output functions. *
 *******************************/

/* Generic substitute for cprintf that has color support */
void i3_printf(CHAR_DATA *ch, const char *fmt, ...)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buf, MAX_STRING_LENGTH, fmt, args);
    va_end(args);

    strlcpy(buf2, I3_i3tag_to_mudtag(ch, buf), MAX_STRING_LENGTH);
    cprintf(ch, "%s", buf2);
}

void i3wrap_printf(CHAR_DATA *ch, const char *fmt, ...)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buf, MAX_STRING_LENGTH, fmt, args);
    va_end(args);

    strlcpy(buf2, I3_i3tag_to_mudtag(ch, buf), MAX_STRING_LENGTH);
    // cprintf(ch, "%s", buf2);
    // cprintf(ch, "%s", color_wrap(78, 96, "      ", buf2));
    cprintf(ch, "%s",
            color_wrap(MAX(18, ch->desc->telnet.cols - 12), MAX(18, ch->desc->telnet.cols - 2), "      ", buf2));
}

/* Generic page_printf type function */
void i3page_printf(CHAR_DATA *ch, const char *fmt, ...)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    va_list args;

    va_start(args, fmt);
    // log_info("NL-fmt  %s for %s", strpbrk(fmt, "\n") ? "FOUND" : "NOT FOUND", fmt);
    vsnprintf(buf, MAX_STRING_LENGTH, fmt, args);
    va_end(args);

    // log_info("NL-pre  %s for %s", strpbrk(buf, "\n") ? "FOUND" : "NOT FOUND", buf);
    strlcpy(buf2, I3_i3tag_to_mudtag(ch, buf), MAX_STRING_LENGTH);
    // log_info("NL-post %s for %s", strpbrk(buf2, "\n") ? "FOUND" : "NOT FOUND", buf2);
    page_string(ch->desc, buf2, 1);
}

/********************************
 * Low level utility functions. *
 ********************************/

int i3todikugender(int gender)
{
    int sex = 0;

    if (gender == 0)
        sex = SEX_MALE;

    if (gender == 1)
        sex = SEX_FEMALE;

    if (gender > 1)
        sex = SEX_NEUTRAL;

    return sex;
}

int dikutoi3gender(int gender)
{
    int sex = 0;

    if (gender > 2 || gender < 0)
        sex = 2; /* I3 neuter */

    if (gender == SEX_MALE)
        sex = 0; /* I3 Male */

    if (gender == SEX_FEMALE)
        sex = 1; /* I3 Female */

    return sex;
}

int get_permvalue(const char *flag)
{
    unsigned int x;

    for (x = 0; x < (sizeof(perm_names) / sizeof(perm_names[0])); x++)
        if (!strcasecmp(flag, perm_names[x]))
            return x;
    return -1;
}

/*  I3_getarg: extract a single argument (with given max length) from
 *  argument to arg; if arg==NULL, just skip an arg, don't copy it out
 */
char *I3_getarg(char *argument, char *arg, int maxlen)
{
    int len = 0;

    if (!argument || argument[0] == '\0')
    {
        if (arg)
            arg[0] = '\0';

        return argument;
    }

    while (*argument && isspace(*argument))
        argument++;

    if (arg)
        while (*argument && !isspace(*argument) && len < maxlen - 1)
            *arg++ = *argument++, len++;
    else
        while (*argument && !isspace(*argument))
            argument++;

    while (*argument && !isspace(*argument))
        argument++;

    while (*argument && isspace(*argument))
        argument++;

    if (arg)
        *arg = '\0';

    return argument;
}

/* Check for a name in a list */
bool I3_hasname(char *list, const char *name)
{
    char *p;
    char arg[MAX_INPUT_LENGTH];

    if (!list)
        return FALSE;

    p = I3_getarg(list, arg, MAX_INPUT_LENGTH);
    while (arg[0])
    {
        if (!strcasecmp(name, arg))
            return TRUE;
        p = I3_getarg(p, arg, MAX_INPUT_LENGTH);
    }
    return FALSE;
}

/* Add a name to a list */
void I3_flagchan(char **list, const char *name)
{
    char buf[MAX_STRING_LENGTH];

    if (I3_hasname(*list, name))
        return;

    if (*list && *list[0] != '\0')
        snprintf(buf, MAX_STRING_LENGTH, "%s %s", *list, name);
    else
        strlcpy(buf, name, MAX_STRING_LENGTH);

    I3STRFREE(*list);
    *list = I3STRALLOC(buf);
}

/* Remove a name from a list */
void I3_unflagchan(char **list, const char *name)
{
    char buf[MAX_STRING_LENGTH], arg[MAX_INPUT_LENGTH];
    char *p;

    buf[0] = '\0';
    p = I3_getarg(*list, arg, MAX_INPUT_LENGTH);
    while (arg[0])
    {
        if (strcasecmp(arg, name))
        {
            if (buf[0])
                strlcat(buf, " ", MAX_STRING_LENGTH);
            strlcat(buf, arg, MAX_STRING_LENGTH);
        }
        p = I3_getarg(p, arg, MAX_INPUT_LENGTH);
    }
    I3STRFREE(*list);
    *list = I3STRALLOC(buf);
}

bool i3_str_prefix(const char *astr, const char *bstr)
{
    if (!astr)
    {
        log_error("Strn_cmp: null astr.");
        return TRUE;
    }

    if (!bstr)
    {
        log_error("Strn_cmp: null bstr.");
        return TRUE;
    }

    for (; *astr; astr++, bstr++)
    {
        if (LOWER(*astr) != LOWER(*bstr))
            return TRUE;
    }
    return FALSE;
}

/*
 * Returns an initial-capped string.
 */
char *i3capitalize(const char *str)
{
    static char strcap[MAX_STRING_LENGTH];
    int i;

    for (i = 0; str[i] != '\0'; i++)
        strcap[i] = tolower(str[i]);
    strcap[i] = '\0';
    strcap[0] = toupper(strcap[0]);
    return strcap;
}

/* Borrowed from Samson's new_auth snippet - checks to see if a particular player exists in the mud.
 * This is called from i3locate and i3finger to report on offline characters.
 */
bool i3exists_player(char *name)
{
    struct stat fst;
    char buf[MAX_INPUT_LENGTH];

    /*
     * Stands to reason that if there ain't a name to look at, they damn well don't exist!
     */
    if (!name || !strcasecmp(name, ""))
        return FALSE;

    snprintf(buf, MAX_INPUT_LENGTH, "%s%c/%s", PLAYER_DIR, tolower(name[0]), i3capitalize(name));

    if (stat(buf, &fst) != -1)
        return TRUE;
    else
        return FALSE;
}

bool verify_i3layout(const char *fmt, int num)
{
    const char *c;
    int i = 0;

    c = fmt;
    while ((c = strchr(c, '%')) != NULL)
    {
        if (*(c + 1) == '%')
        { /* %% */
            c += 2;
            continue;
        }

        if (*(c + 1) != 's') /* not %s */
            return FALSE;

        c++;
        i++;
    }

    if (i != num)
        return FALSE;

    return TRUE;
}

/*
 * This means we're in the process of connecting to the I3 network, but may not have
 * fully finished yet.  This will be false only if there is no I3 socket, which happens
 * before any connection attempts, or after a disconnect without a reconnect.
 */
bool is_connecting(void)
{
    if (I3_socket < 1)
        return FALSE;

    return TRUE;
}

/*
 * This says the I3 connection is actually up and functional, or was... it returns true
 * only after we have received the startup_reply packet.
 */
bool is_connected(void)
{
    if (connected_at)
        return TRUE;

    return FALSE;
}

/*
 * Add backslashes in front of the " and \'s
 */
char *I3_escape(const char *ps)
{
    static char xnew[MAX_STRING_LENGTH];
    char *pnew = xnew;

    while (ps[0])
    {
        if (ps[0] == '"')
        {
            pnew[0] = '\\';
            pnew++;
        }
        if (ps[0] == '\\')
        {
            pnew[0] = '\\';
            pnew++;
        }
        if (ps[0] == '\r' || ps[0] == '\n')
        {
            ps++;
            continue;
        }
        pnew[0] = ps[0];
        pnew++;
        ps++;
    }
    pnew[0] = '\0';
    return xnew;
}

/* Searches through the channel list to see if one exists with the localname supplied to it. */
I3_CHANNEL *find_I3_channel_by_localname(const char *name)
{
    I3_CHANNEL *channel = NULL;

    for (channel = first_I3chan; channel; channel = channel->next)
    {
        if (!channel->local_name)
            continue;

        if (!strcasecmp(channel->local_name, name))
            return channel;
    }
    return NULL;
}

/* Searches through the channel list to see if one exists with the I3 channel name supplied to it.*/
I3_CHANNEL *find_I3_channel_by_name(const char *name)
{
    I3_CHANNEL *channel = NULL;

    if (!name || !*name)
        return NULL;

    for (channel = first_I3chan; channel; channel = channel->next)
    {
        if (!strcasecmp(channel->I3_name, name))
            return channel;
    }
    return NULL;
}

/* Sets up a channel on the mud for the first time, configuring its default layout.
 * If you don't like the default layout of channels, this is where you should edit it to your liking.
 */
I3_CHANNEL *new_I3_channel(void)
{
    I3_CHANNEL *cnew;

    I3CREATE(cnew, I3_CHANNEL, 1);

    I3LINK(cnew, first_I3chan, last_I3chan, next, prev);
    return cnew;
}

/* Deletes a channel's information from the mud. */
void destroy_I3_channel(I3_CHANNEL *channel)
{
    int x;

    if (channel == NULL)
    {
        log_error("%s", "destroy_I3_channel: Null parameter");
        return;
    }

    I3STRFREE(channel->local_name);
    I3STRFREE(channel->host_mud);
    I3STRFREE(channel->I3_name);
    I3STRFREE(channel->layout_m);
    I3STRFREE(channel->layout_e);

    for (x = 0; x < MAX_I3HISTORY; x++)
    {
        // if (channel->history[x] && channel->history[x][0] != '\0')
        if (channel->history[x] != NULL)
            I3STRFREE(channel->history[x]);
    }

    I3UNLINK(channel, first_I3chan, last_I3chan, next, prev);
    I3DISPOSE(channel);
}

/* Finds a mud with the name supplied on the mudlist */
I3_MUD *find_I3_mud_by_name(const char *name)
{
    I3_MUD *mud;

    for (mud = first_mud; mud; mud = mud->next)
    {
        if (!strcasecmp(mud->name, name))
            return mud;
    }
    return NULL;
}

I3_MUD *new_I3_mud(char *name)
{
    I3_MUD *cnew, *mud_prev;

    I3CREATE(cnew, I3_MUD, 1);

    cnew->name = I3STRALLOC(name);

    for (mud_prev = first_mud; mud_prev; mud_prev = mud_prev->next)
        if (strcasecmp(mud_prev->name, name) >= 0)
            break;

    if (!mud_prev)
        I3LINK(cnew, first_mud, last_mud, next, prev);
    else
        I3INSERT(cnew, mud_prev, first_mud, next, prev);

    return cnew;
}

I3_MUD *create_I3_mud()
{
    I3_MUD *mud = NULL;

    I3CREATE(mud, I3_MUD, 1);

    /* make sure string pointers are NULL */
    mud->name = NULL;
    mud->ipaddress = NULL;
    mud->mudlib = NULL;
    mud->base_mudlib = NULL;
    mud->driver = NULL;
    mud->mud_type = NULL;
    mud->open_status = NULL;
    mud->admin_email = NULL;
    mud->telnet = NULL;
    mud->web_wrong = NULL;

    mud->banner = NULL;
    mud->web = NULL;
    mud->time = NULL;
    mud->daemon = NULL;

    mud->routerName = NULL;

    /* default values */
    mud->status = -1;
    mud->player_port = 0;
    mud->imud_tcp_port = 0;
    mud->imud_udp_port = 0;

    mud->tell = FALSE;
    mud->beep = FALSE;
    mud->emoteto = FALSE;
    mud->who = FALSE;
    mud->finger = FALSE;
    mud->locate = FALSE;
    mud->channel = FALSE;
    mud->news = FALSE;
    mud->mail = FALSE;
    mud->file = FALSE;
    mud->auth = FALSE;
    mud->ucache = FALSE;

    mud->smtp = 0;
    mud->ftp = 0;
    mud->nntp = 0;
    mud->http = 0;
    mud->pop3 = 0;
    mud->rcp = 0;
    mud->amrcp = 0;

    mud->jeamland = 0;

    mud->autoconnect = FALSE;
    mud->password = 0;
    mud->mudlist_id = 0;
    mud->chanlist_id = 0;
    mud->minlevel = 1;    /* Minimum default level before I3 will acknowledge you exist */
    mud->immlevel = 2;    /* Default immortal level */
    mud->adminlevel = 51; /* Default administration level */
    mud->implevel = 60;   /* Default implementor level */

    return mud;
}

void destroy_I3_mud(I3_MUD *mud)
{
    if (mud == NULL)
    {
        log_error("%s", "destroy_I3_mud: Null parameter");
        return;
    }

    I3STRFREE(mud->name);
    I3STRFREE(mud->ipaddress);
    I3STRFREE(mud->mudlib);
    I3STRFREE(mud->base_mudlib);
    I3STRFREE(mud->driver);
    I3STRFREE(mud->mud_type);
    I3STRFREE(mud->open_status);
    I3STRFREE(mud->admin_email);
    I3STRFREE(mud->telnet);
    I3STRFREE(mud->web_wrong);

    I3STRFREE(mud->banner);
    I3STRFREE(mud->web);
    I3STRFREE(mud->time);
    I3STRFREE(mud->daemon);

    I3STRFREE(mud->routerName);
    if (mud != this_i3mud)
        I3UNLINK(mud, first_mud, last_mud, next, prev);
    I3DISPOSE(mud);
}

/*
 * Returns a CHAR_DATA class which matches the string
 *
 */
CHAR_DATA *I3_find_user(const char *name)
{
    DESCRIPTOR_DATA *d;
    CHAR_DATA *vch = NULL;

    for (d = first_descriptor; d; d = d->next)
    {
        if ((vch = d->character ? d->character : d->original) != NULL && !strcasecmp(CH_I3NAME(vch), name) &&
            d->connected == CON_PLAYING)
            return vch;
    }
    return NULL;
}

void i3_message_to_players(char *str)
{
    DESCRIPTOR_DATA *d;
    CHAR_DATA *vch = NULL;

    for (d = first_descriptor; d; d = d->next)
    {
        vch = d->original ? d->original : d->character;

        if (!vch)
            continue;

        if (!I3_hasname(I3LISTEN(vch), "wiley") || I3_hasname(I3DENY(vch), "wiley"))
            continue;

        if (d->connected == CON_PLAYING)
        {
            i3wrap_printf(vch, "%s%%^RESET%%^\r\n", str);
        }
    }
}

/* Beefed up to include wildcard ignores and user-level IP ignores.
 * Be careful when setting IP based ignores - Last resort measure, etc.
 */
bool i3ignoring(CHAR_DATA *ch, const char *ignore)
{
    I3_IGNORE *temp;
    I3_MUD *mud;
    char *ps;
    char ipbuf[MAX_INPUT_LENGTH], mudname[MAX_INPUT_LENGTH];

    /*
     * Wildcard support thanks to Xorith
     */
    for (temp = FIRST_I3IGNORE(ch); temp; temp = temp->next)
    {
        if (!fnmatch(temp->name, ignore, 0))
            return TRUE;
    }

    /*
     * In theory, getting this far should be the result of an IP:Port ban
     */
    ps = (char *)strchr(ignore, '@');

    if (ignore[0] == '\0' || ps == NULL)
        return FALSE;

    ps[0] = '\0';
    strlcpy(mudname, ps + 1, MAX_INPUT_LENGTH);

    for (mud = first_mud; mud; mud = mud->next)
    {
        if (!strcasecmp(mud->name, mudname))
        {
            snprintf(ipbuf, MAX_INPUT_LENGTH, "%s:%d", mud->ipaddress, mud->player_port);
            for (temp = FIRST_I3IGNORE(ch); temp; temp = temp->next)
            {
                if (!strcasecmp(temp->name, ipbuf))
                    return TRUE;
            }
        }
    }
    return FALSE;
}

/* Be careful with an IP ban - Last resort measure, etc. */
bool i3banned(const char *ignore)
{
    I3_BAN *temp;
    I3_MUD *mud;
    char *ps;
    char mudname[MAX_INPUT_LENGTH], ipbuf[MAX_INPUT_LENGTH];

    /*
     * Wildcard support thanks to Xorith
     */
    for (temp = first_i3ban; temp; temp = temp->next)
    {
        if (!fnmatch(temp->name, ignore, 0))
            return TRUE;
    }

    /*
     * In theory, getting this far should be the result of an IP:Port ban
     */
    ps = (char *)strchr(ignore, '@');

    if (!ignore || ignore[0] == '\0' || ps == NULL)
        return FALSE;

    ps[0] = '\0';
    strlcpy(mudname, ps + 1, MAX_INPUT_LENGTH);

    for (mud = first_mud; mud; mud = mud->next)
    {
        if (!strcasecmp(mud->name, mudname))
        {
            snprintf(ipbuf, MAX_INPUT_LENGTH, "%s:%d", mud->ipaddress, mud->player_port);
            for (temp = first_i3ban; temp; temp = temp->next)
            {
                if (!strcasecmp(temp->name, ipbuf))
                    return TRUE;
            }
        }
    }
    return FALSE;
}

bool i3check_permissions(CHAR_DATA *ch, int checkvalue, int targetvalue, bool enforceequal)
{
    if (checkvalue < 0 || checkvalue > I3PERM_IMP)
    {
        i3_printf(ch, "Invalid permission setting.\r\n");
        return FALSE;
    }

    if (checkvalue > I3PERM(ch))
    {
        i3_printf(ch, "You cannot set permissions higher than your own.\r\n");
        return FALSE;
    }

    if (checkvalue == I3PERM(ch) && I3PERM(ch) != I3PERM_IMP && enforceequal)
    {
        i3_printf(ch, "You cannot set permissions equal to your own. Someone higher up must do this.\r\n");
        return FALSE;
    }

    if (I3PERM(ch) < targetvalue)
    {
        i3_printf(ch, "You cannot alter the permissions of someone or something above your own.\r\n");
        return FALSE;
    }
    return TRUE;
}

/*
 * Read a number from a file. [Taken from Smaug's fread_number]
 */
int i3fread_number(FILE *fp)
{
    int num;
    bool sign;
    char c;

    do
    {
        if (feof(fp))
        {
            log_info("%s", "i3fread_number: EOF encountered on read.");
            return 0;
        }
        c = getc(fp);
    } while (isspace(c));

    num = 0;

    sign = FALSE;
    if (c == '+')
    {
        c = getc(fp);
    }
    else if (c == '-')
    {
        sign = TRUE;
        c = getc(fp);
    }

    if (!isdigit(c))
    {
        log_info("i3fread_number: bad format. (%c)", c);
        return 0;
    }

    while (isdigit(c))
    {
        if (feof(fp))
        {
            log_info("%s", "i3fread_number: EOF encountered on read.");
            return num;
        }
        num = num * 10 + c - '0';
        c = getc(fp);
    }

    if (sign)
        num = 0 - num;

    if (c == '|')
        num += i3fread_number(fp);
    else if (c != ' ')
        ungetc(c, fp);

    return num;
}

/*
 * Read to end of line into static buffer [Taken from Smaug's fread_line]
 */
char *i3fread_line(FILE *fp)
{
    char line[MAX_STRING_LENGTH];
    char *pline;
    char c;
    int ln;

    pline = line;
    line[0] = '\0';
    ln = 0;

    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
        if (feof(fp))
        {
            log_error("%s", "i3fread_line: EOF encountered on read.");
            strlcpy(line, "", MAX_STRING_LENGTH);
            return I3STRALLOC(line);
        }
        c = getc(fp);
    } while (isspace(c));

    ungetc(c, fp);

    do
    {
        if (feof(fp))
        {
            log_error("%s", "i3fread_line: EOF encountered on read.");
            *pline = '\0';
            return I3STRALLOC(line);
        }
        c = getc(fp);
        *pline++ = c;
        ln++;
        if (ln >= (MAX_STRING_LENGTH - 1))
        {
            log_error("%s", "i3fread_line: line too long");
            break;
        }
    } while (c != '\n' && c != '\r');

    do
    {
        c = getc(fp);
    } while (c == '\n' || c == '\r');

    ungetc(c, fp);
    pline--;
    *pline = '\0';

    /*
     * Since tildes generally aren't found at the end of lines, this seems workable. Will enable reading old configs.
     */
    if (line[strlen(line) - 1] == '~')
        line[strlen(line) - 1] = '\0';
    return I3STRALLOC(line);
}

/*
 * Read one word (into static buffer). [Taken from Smaug's fread_word]
 */
char *i3fread_word(FILE *fp)
{
    static char word[MAX_INPUT_LENGTH];
    char *pword;
    char cEnd;

    do
    {
        if (feof(fp))
        {
            log_info("%s", "i3fread_word: EOF encountered on read.");
            word[0] = '\0';
            return word;
        }
        cEnd = getc(fp);
    } while (isspace(cEnd));

    if (cEnd == '\'' || cEnd == '"')
    {
        pword = word;
    }
    else
    {
        word[0] = cEnd;
        pword = word + 1;
        cEnd = ' ';
    }

    for (; pword < word + MAX_INPUT_LENGTH; pword++)
    {
        if (feof(fp))
        {
            log_info("%s", "i3fread_word: EOF encountered on read.");
            *pword = '\0';
            return word;
        }
        *pword = getc(fp);
        if (cEnd == ' ' ? isspace(*pword) : *pword == cEnd)
        {
            if (cEnd == ' ')
                ungetc(*pword, fp);
            *pword = '\0';
            return word;
        }
    }

    log_info("%s", "i3fread_word: word too long");
    return NULL;
}

char *i3fread_rest_of_line(FILE *fp)
{
    static char word[MAX_STRING_LENGTH];
    char *pword;
    char c;

    do
    {
        if (feof(fp))
        {
            log_info("%s", "i3fread_rest_of_line: EOF encountered on read.");
            word[0] = '\0';
            return word;
        }
        c = getc(fp);
    } while (isspace(c));

    word[0] = c;
    pword = word + 1;

    for (; pword < word + MAX_STRING_LENGTH; pword++)
    {
        if (feof(fp))
        {
            log_info("%s", "i3fread_rest_of_line: EOF encountered on read.");
            *pword = '\0';
            return word;
        }
        c = getc(fp);
        if (c == '\n' || c == '\r')
        {
            do
            {
                c = getc(fp);
            } while (c == '\n' || c == '\r');
            if (!feof(fp))
                ungetc(c, fp);
            *pword = '\0';
            return word;
        }
        else
        {
            *pword = c;
        }
    }

    log_info("%s", "i3fread_rest_of_line: line too long");
    return NULL;
}

/*
 * Read a letter from a file. [Taken from Smaug's fread_letter]
 */
char i3fread_letter(FILE *fp)
{
    char c;

    do
    {
        if (feof(fp))
        {
            log_info("%s", "i3fread_letter: EOF encountered on read.");
            return '\0';
        }
        c = getc(fp);
    } while (isspace(c));

    return c;
}

/*
 * Read to end of line (for comments). [Taken from Smaug's fread_to_eol]
 */
void i3fread_to_eol(FILE *fp)
{
    char c;

    do
    {
        if (feof(fp))
        {
            log_info("%s", "i3fread_to_eol: EOF encountered on read.");
            return;
        }
        c = getc(fp);
    } while (c != '\n' && c != '\r');

    do
    {
        c = getc(fp);
    } while (c == '\n' || c == '\r');

    ungetc(c, fp);
}

/*
 * Read and allocate space for a string from a file.
 */
char *i3fread_string(FILE *fp)
{
    static char buf[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    char *ack = NULL;
    int flag = FALSE;
    int c = 0;
    static char Empty[1] = "";

    bzero(buf, MAX_STRING_LENGTH);
    ack = buf;
    flag = 0;
    do
    {
        c = getc(fp);
    } while (isspace(c));

    if ((*ack++ = c) == '~')
        return Empty;
    if (((int)(*ack++ = c)) == '\0xA2')
        return Empty;

    for (;;)
    {
        if (ack > &buf[MAX_STRING_LENGTH - 1])
        {
            log_error("new_fread_string: MAX_STRING %d exceeded, truncating.", MAX_STRING_LENGTH);
            return buf;
        }
        switch ((int)(*ack = getc(fp)))
        {
        default:
            flag = 0;
            ack++;
            break;
        case EOF:
            log_error("Fread_string: EOF");
            return buf;
        case '\r':
            break;
        case '~':
        case '\0xA2':
            ack++;
            flag = 1;
            break;
        case '\n':
            if (flag)
            {
                if (ack > buf)
                {
                    ack--;
                    *ack = '\0';
                }
                return buf;
            }
            else
            {
                flag = 0;
                ack++;
                *ack++ = '\r';
            }
            break;
        }
    }
}

/******************************************
 * Packet handling and routing functions. *
 ******************************************/

/*
 * Write a string into the send-buffer. Does not yet send it.
 */
void I3_write_buffer(const char *msg)
{
    long newsize = I3_output_pointer + strlen(msg);

    if (newsize > OPS - 1)
    {
        log_error("I3_write_buffer: buffer too large (would become %ld)", newsize);
        return;
    }
    strlcpy(I3_output_buffer + I3_output_pointer, msg, newsize);
    I3_output_pointer = newsize;
}

/* Use this function in place of I3_write_buffer ONLY if the text to be sent could
 * contain color tags to parse into Pinkfish codes. Otherwise it will mess up the packet.
 */
void send_to_i3(const char *text)
{
    char buf[MAX_STRING_LENGTH];

    snprintf(buf, MAX_STRING_LENGTH, "%s", I3_mudtag_to_i3tag(text));
    I3_write_buffer(buf);
}

/*
 * Put a I3-header in the send-buffer. If a field is NULL it will
 * be replaced by a 0 (zero).
 */
void I3_write_header(const char *identifier, const char *originator_mudname, const char *originator_username,
                     const char *target_mudname, const char *target_username)
{
    I3_write_buffer("({\"");
    I3_write_buffer(identifier);
    I3_write_buffer("\",5,");
    if (originator_mudname)
    {
        I3_write_buffer("\"");
        I3_write_buffer(originator_mudname);
        I3_write_buffer("\",");
    }
    else
        I3_write_buffer("0,");

    if (originator_username)
    {
        I3_write_buffer("\"");
        I3_write_buffer(I3_nameescape(originator_username));
        I3_write_buffer("\",");
    }
    else
        I3_write_buffer("0,");

    if (target_mudname)
    {
        I3_write_buffer("\"");
        I3_write_buffer(target_mudname);
        I3_write_buffer("\",");
    }
    else
        I3_write_buffer("0,");

    if (target_username)
    {
        I3_write_buffer("\"");
        I3_write_buffer(target_username);
        I3_write_buffer("\",");
    }
    else
        I3_write_buffer("0,");
}

/*
 * Gets the next I3 field, that is when the amount of {[("'s and
 * ")]}'s match each other when a , is read. It's not foolproof, it
 * should honestly be some kind of statemachine, which does error-
 * checking. Right now I trust the I3-router to send proper packets
 * only. How naive :-) [Indeed Edwin, but I suppose we have little choice :P - Samson]
 *
 * ps will point to the beginning of the next field.
 *
 */
char *I3_get_field(char *packet, char **ps)
{
    int count[MAX_INPUT_LENGTH];
    char has_apostrophe = 0, has_backslash = 0;
    char foundit = 0;

    bzero(count, sizeof(count));

    *ps = packet;
    while (1)
    {
        switch (*ps[0])
        {
        case '{':
            if (!has_apostrophe)
                count[(int)'{']++;
            break;
        case '}':
            if (!has_apostrophe)
                count[(int)'}']++;
            break;
        case '[':
            if (!has_apostrophe)
                count[(int)'[']++;
            break;
        case ']':
            if (!has_apostrophe)
                count[(int)']']++;
            break;
        case '(':
            if (!has_apostrophe)
                count[(int)'(']++;
            break;
        case ')':
            if (!has_apostrophe)
                count[(int)')']++;
            break;
        case '\\':
            if (has_backslash)
                has_backslash = 0;
            else
                has_backslash = 1;
            break;
        case '"':
            if (has_backslash)
            {
                has_backslash = 0;
            }
            else
            {
                if (has_apostrophe)
                    has_apostrophe = 0;
                else
                    has_apostrophe = 1;
            }
            break;
        case ',':
        case ':':
            if (has_apostrophe)
                break;
            if (has_backslash)
                break;
            if (count[(int)'{'] != count[(int)'}'])
                break;
            if (count[(int)'['] != count[(int)']'])
                break;
            if (count[(int)'('] != count[(int)')'])
                break;
            foundit = 1;
            break;
        }
        if (foundit)
            break;
        (*ps)++;
    }
    *ps[0] = '\0';
    (*ps)++;
    return *ps;
}

/*
 * Writes the string into the socket, prefixed by the size.
 */
bool I3_write_packet(char *msg)
{
    int oldsize, size, check, x;
    char *s = I3_output_buffer;

    oldsize = size = strlen(msg + 4);
    s[3] = size % 256;
    size >>= 8;
    s[2] = size % 256;
    size >>= 8;
    s[1] = size % 256;
    size >>= 8;
    s[0] = size % 256;

    // So, to log outgoing packets, we need the type.
    // At this point, it's embedded into the string, so we have to pluck
    // it out again....
    // ({"identifier",5
    // Additionally, we've set aside 4 bytes for the mud-mode crap...
    // So, if properly formed, the type starts at byte msg[7]
    // and will go until we hit another double quote...
    {
        char *id_start, *id_end;
        int id_len;
        char id_buf[MAX_INPUT_LENGTH];

        id_start = (msg + 7);
        id_end = strchr(id_start, '"');
        id_len = (id_end - id_start + 1);
        strlcpy(id_buf, id_start, id_len);

        // i3_packet_log(id_buf, oldsize, (msg + 4));
        i3_packet_log(id_buf, oldsize, NULL);
    }

    /*
     * Scan for \r used in Diku client packets and change to NULL
     */
    for (x = 0; x < oldsize + 4; x++)
        if (msg[x] == '\r' && x > 3)
            msg[x] = '\0';

    bytes_sent += oldsize + 4;
    if (packetdebug)
    {
        log_info("I3_PACKET Sending: %d Bytes...", oldsize);
    }
    check = send(I3_socket, msg, oldsize + 4, 0);
    if (packetdebug)
    {
        // Normally, this works just fine, but for some bizzare reason that
        // I don't understand yet.. .if you have packetdebug enabled before
        // the I3 connection has been established, the above send() will
        // return -1 and fail.
        // Oh, and this ONLY happens for *Kelly.  *dalet is fine with it.
        log_info("I3_PACKET Sent: %d Bytes...", check);
        log_info("I3_PACKET Packet Sent: %s", msg + 4);
    }

#if 0
    if (!check || (check < 0 && errno != EAGAIN && errno != EWOULDBLOCK)) {
	if (check < 0)
	    log_info("%s", "Write error on socket.");
	else
	    log_info("%s", "EOF encountered on socket write.");
	I3_connection_close(TRUE);
	return FALSE;
    }

    if (check < 0)					       /* EAGAIN */
	return TRUE;

    //bytes_sent += check;
#endif

    if (check == 0)
    {
        log_error("I3_PACKET EOF encountered on socket write.");
        I3_connection_close(TRUE);
        return FALSE;
    }

    if (check < 0)
    {
        switch (errno)
        {
        // Linux morons -- EAGAIN and EWOULDBLOCK are not actually the same error,
        // even if you tend to do the same thing when they happen.  'tards.

        // case EAGAIN:
        case EWOULDBLOCK:
            if (packetdebug)
            {
                log_info("I3_PACKET Socket would block, retrying later.");
            }
            return TRUE;
            break;
        case EMSGSIZE:
            log_error("I3_PACKET Message too big for tcp/ip?");
            I3_connection_close(TRUE);
            return FALSE;
            break;
        case ENOTCONN:
            log_error("I3_PACKET Socket not actualy connected?");
            I3_connection_close(TRUE);
            return FALSE;
            break;
        default:
            {
                char timeout_msg[MAX_INPUT_LENGTH];

                log_error("I3_PACKET Socket error: %d (%s)", errno, strerror(errno));
                *timeout_msg = '\0';
                if( errno != 0 ) {
                    snprintf(timeout_msg, MAX_INPUT_LENGTH, "I3_PACKET (%s) Socket error: %d (%s)", I3_ROUTER_NAME, errno, strerror(errno));
                } else {
                    snprintf(timeout_msg, MAX_INPUT_LENGTH, "I3_PACKET (%s) Socket error: errno not set, yet %d returned...", I3_ROUTER_NAME, check);
                }
                allchan_log(0, (char *)"wiley", (char *)"Cron", (char *)"Cron", (char *)"WileyMUD", (char *)timeout_msg);
                I3_connection_close(TRUE);
            }
            return FALSE;
        }
    }

    I3_output_pointer = 4;
    return TRUE;
}

void I3_send_packet(void)
{
    I3_write_packet(I3_output_buffer);
}

/* The all important startup packet. This is what will be initially sent upon trying to connect
 * to the I3 router. It is therefore quite important that the information here be exactly correct.
 * If anything is wrong, your packet will be dropped by the router as invalid and your mud simply
 * won't connect to I3. DO NOT USE COLOR TAGS FOR ANY OF THIS INFORMATION!!!
 * Bugs fixed in this on 8-31-03 for improperly sent tags. Streamlined for less packet bloat.
 */
void I3_startup_packet(void)
{
    char s[MAX_INPUT_LENGTH];
    char *strtime;
    struct timeval last_time;
    ROUTER_DATA *router;

    if (!is_connecting())
        return;

    I3_output_pointer = 4;
    I3_output_buffer[0] = '\0';

    for (router = first_router; router; router = router->next)
    {
        if (!strcasecmp(router->name, I3_ROUTER_NAME))
        {
            router->password = this_i3mud->password;
            router->mudlist_id = this_i3mud->mudlist_id;
            router->chanlist_id = this_i3mud->chanlist_id;
            break;
        }
    }
    I3_saverouters();

    log_info("Sending startup_packet to %s", I3_ROUTER_NAME);

    I3_write_header("startup-req-3", this_i3mud->name, NULL, I3_ROUTER_NAME, NULL);

    snprintf(s, MAX_INPUT_LENGTH, "%d", this_i3mud->password);
    I3_write_buffer(s);
    I3_write_buffer(",");
    snprintf(s, MAX_INPUT_LENGTH, "%d", this_i3mud->mudlist_id);
    I3_write_buffer(s);
    I3_write_buffer(",");
    snprintf(s, MAX_INPUT_LENGTH, "%d", this_i3mud->chanlist_id);
    I3_write_buffer(s);
    I3_write_buffer(",");
    snprintf(s, MAX_INPUT_LENGTH, "%d", this_i3mud->player_port);
    I3_write_buffer(s);
    I3_write_buffer(",0,0,\"");

    I3_write_buffer(this_i3mud->mudlib);
    I3_write_buffer("\",\"");
    I3_write_buffer(this_i3mud->base_mudlib);
    I3_write_buffer("\",\"");
    I3_write_buffer(this_i3mud->driver);
    I3_write_buffer("\",\"");
    I3_write_buffer(this_i3mud->mud_type);
    I3_write_buffer("\",\"");
    I3_write_buffer(this_i3mud->open_status);
    I3_write_buffer("\",\"");
    I3_write_buffer(this_i3mud->admin_email);
    I3_write_buffer("\",");

    /*
     * Begin first mapping set
     */
    I3_write_buffer("([");
    if (this_i3mud->emoteto)
        I3_write_buffer("\"emoteto\":1,");
    if (this_i3mud->news)
        I3_write_buffer("\"news\":1,");
    if (this_i3mud->ucache)
        I3_write_buffer("\"ucache\":1,");
    if (this_i3mud->auth)
        I3_write_buffer("\"auth\":1,");
    if (this_i3mud->locate)
        I3_write_buffer("\"locate\":1,");
    if (this_i3mud->finger)
        I3_write_buffer("\"finger\":1,");
    if (this_i3mud->channel)
        I3_write_buffer("\"channel\":1,");
    if (this_i3mud->who)
        I3_write_buffer("\"who\":1,");
    if (this_i3mud->tell)
        I3_write_buffer("\"tell\":1,");
    if (this_i3mud->beep)
        I3_write_buffer("\"beep\":1,");
    if (this_i3mud->mail)
        I3_write_buffer("\"mail\":1,");
    if (this_i3mud->file)
        I3_write_buffer("\"file\":1,");
    if (this_i3mud->http)
    {
        snprintf(s, MAX_INPUT_LENGTH, "\"http\":%d,", this_i3mud->http);
        I3_write_buffer(s);
    }
    if (this_i3mud->smtp)
    {
        snprintf(s, MAX_INPUT_LENGTH, "\"smtp\":%d,", this_i3mud->smtp);
        I3_write_buffer(s);
    }
    if (this_i3mud->pop3)
    {
        snprintf(s, MAX_INPUT_LENGTH, "\"pop3\":%d,", this_i3mud->pop3);
        I3_write_buffer(s);
    }
    if (this_i3mud->ftp)
    {
        snprintf(s, MAX_INPUT_LENGTH, "\"ftp\":%d,", this_i3mud->ftp);
        I3_write_buffer(s);
    }
    if (this_i3mud->nntp)
    {
        snprintf(s, MAX_INPUT_LENGTH, "\"nntp\":%d,", this_i3mud->nntp);
        I3_write_buffer(s);
    }
    if (this_i3mud->rcp)
    {
        snprintf(s, MAX_INPUT_LENGTH, "\"rcp\":%d,", this_i3mud->rcp);
        I3_write_buffer(s);
    }
    if (this_i3mud->amrcp)
    {
        snprintf(s, MAX_INPUT_LENGTH, "\"amrcp\":%d,", this_i3mud->amrcp);
        I3_write_buffer(s);
    }
    I3_write_buffer("]),([");

    /*
     * END first set of "mappings", start of second set
     */
    if (this_i3mud->web && this_i3mud->web[0] != '\0')
    {
        snprintf(s, MAX_INPUT_LENGTH, "\"url\":\"%s\",", this_i3mud->web);
        I3_write_buffer(s);
    }
    strtime = ctime(&i3_time);
    strtime[strlen(strtime) - 1] = '\0';
    snprintf(s, MAX_INPUT_LENGTH, "\"time\":\"%s\",", strtime);
    I3_write_buffer(s);

    I3_write_buffer("]),})\r");
    I3_send_packet();

    if (i3_time == 0)
    {
        // We called this before the first run of i3_loop(), from outside.
        // Either that, or this code has travelled back in time, and I'm going to be rich!
        gettimeofday(&last_time, NULL);
        i3_time = (time_t)last_time.tv_sec;
    }
    connect_started_at = i3_time;
}

/* This function saves the password, mudlist ID, and chanlist ID that are used by the mud.
 * The password value is returned from the I3 router upon your initial connection.
 * The mudlist and chanlist ID values are updated as needed while your mud is connected.
 * Do not modify the file it generates because doing so may prevent your mud from reconnecting
 * to the router in the future. This file will be rewritten each time the i3_shutdown function
 * is called, or any of the id values change.
 */
void I3_save_id(void)
{
    FILE *fp;

    if (!(fp = fopen(I3_PASSWORD_FILE, "w")))
    {
        log_info("%s", "Couldn't write to I3 password file.");
        return;
    }

    fprintf(fp, "%s", "#PASSWORD\n");
    fprintf(fp, "%d %d %d\n", this_i3mud->password, this_i3mud->mudlist_id, this_i3mud->chanlist_id);
    I3FCLOSE(fp);
}

/* The second most important packet your mud will deal with. If you never get this
 * coming back from the I3 router, something was wrong with your startup packet
 * or the router may be jammed up. Whatever the case, if you don't get a reply back
 * your mud won't be acknowledged as connected.
 */
void I3_process_startup_reply(I3_HEADER *header, char *s)
{
    ROUTER_DATA *router;
    I3_CHANNEL *channel;
    char *ps = s;
    char *next_ps;

    // Recevies the router list. Nothing much to do here until there's more than 1 router.
    I3_get_field(ps, &next_ps);
    // log_info("%s", ps);
    ps = next_ps;

    // Receives your mud's updated password, which may or may not be the same as what it sent out before
    I3_get_field(ps, &next_ps);
    this_i3mud->password = atoi(ps);

    log_info("Received startup_reply from %s", header->originator_mudname);

    I3_save_id();

    for (router = first_router; router; router = router->next)
    {
        if (!strcasecmp(router->name, header->originator_mudname))
        {
            router->reconattempts = 0;
            I3_ROUTER_NAME = router->name;
            I3_ROUTER_IP = router->ip;
            router->password = this_i3mud->password;
            break;
        }
    }
    I3_saverouters();

    i3wait = 0;
    expecting_timeout = 0;
    i3justconnected = 1; // This is just a flag to let us process on_connect things
                         // later, after we've had a chance to get channels setup
    uptime = 0;
    connected_at = i3_time;
    time_to_taunt = getTimestamp() + I3_TAUNT_DELAY;

    log_info("%s", "Intermud-3 Network connection complete.");

    for (channel = first_I3chan; channel; channel = channel->next)
    {
        // if (channel->local_name && channel->local_name[0] != '\0') {
        log_info("Subscribing to %s", channel->I3_name);
        I3_send_channel_listen(channel, TRUE);
        //}
    }
}

void I3_process_chanack(I3_HEADER *header, char *s)
{
    CHAR_DATA *ch;
    char *next_ps, *ps = s;

    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);

    if (!(ch = I3_find_user(header->target_username)))
        log_info("%s", ps);
    else
        i3_printf(ch, "%%^GREEN%%^%%^BOLD%%^%s%%^RESET%%^\r\n", ps);
}

void I3_send_error(const char *mud, const char *user, const char *code, const char *message)
{
    if (!is_connected())
        return;

    I3_write_header("error", this_i3mud->name, 0, mud, user);
    I3_write_buffer("\"");
    I3_write_buffer(code);
    I3_write_buffer("\",\"");
    I3_write_buffer(I3_escape(message));
    I3_write_buffer("\",0,})\r");
    I3_send_packet();
}

void I3_process_error(I3_HEADER *header, char *s)
{
    CHAR_DATA *ch;
    char *next_ps, *ps = s;
    char type[MAX_INPUT_LENGTH], error[MAX_STRING_LENGTH];

    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    strlcpy(type, ps, MAX_INPUT_LENGTH);
    ps = next_ps;

    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);

    /*
     * Since VargonMUD likes to spew errors for no good reason....
     */
    if (!strcasecmp(header->originator_mudname, "VargonMUD"))
        return;
    if (!strcasecmp(header->originator_mudname, "Ulario"))
        return;

    snprintf(error, MAX_STRING_LENGTH, "Error: from %s to %s@%s\r\n%s: %s", header->originator_mudname,
             header->target_username, header->target_mudname, type, ps);

    if (!(ch = I3_find_user(header->target_username)))
        log_info("%s", error);
    else
        i3_printf(ch, "%%^RED%%^%%^BOLD%%^%s%%^RESET%%^\r\n", error);
}

int I3_get_ucache_gender(char *name)
{
    UCACHE_DATA *user;

    for (user = first_ucache; user; user = user->next)
    {
        if (!strcasecmp(user->name, name))
            return user->gender;
    }

    /*
     * -1 means you aren't in the list and need to be put there.
     */
    return -1;
}

/* Saves the ucache info to disk because it would just be spamcity otherwise */
void I3_save_ucache(void)
{
    FILE *fp;
    UCACHE_DATA *user;

    if (!(fp = fopen(I3_UCACHE_FILE, "w")))
    {
        log_info("%s", "Couldn't write to I3 ucache file.");
        return;
    }

    for (user = first_ucache; user; user = user->next)
    {
        fprintf(fp, "%s", "#UCACHE\n");
        fprintf(fp, "Name %s\n", user->name);
        fprintf(fp, "Sex  %d\n", user->gender);
        fprintf(fp, "Time %ld\n", (long int)user->time);
        fprintf(fp, "%s", "End\n\n");
    }
    fprintf(fp, "%s", "#END\n");
    I3FCLOSE(fp);
}

void I3_prune_ucache(void)
{
    UCACHE_DATA *ucache, *next_ucache;

    for (ucache = first_ucache; ucache; ucache = next_ucache)
    {
        next_ucache = ucache->next;

        /*
         * Info older than 30 days is removed since this person likely hasn't logged in at all
         */
        if (i3_time - ucache->time >= 2592000)
        {
            I3STRFREE(ucache->name);
            I3UNLINK(ucache, first_ucache, last_ucache, next, prev);
            I3DISPOSE(ucache);
        }
    }
    I3_save_ucache();
}

/* Updates user info if they exist, adds them if they don't. */
void I3_ucache_update(char *name, int gender)
{
    UCACHE_DATA *user;

    for (user = first_ucache; user; user = user->next)
    {
        if (!strcasecmp(user->name, name))
        {
            user->gender = gender;
            user->time = i3_time;
            return;
        }
    }
    I3CREATE(user, UCACHE_DATA, 1);
    user->name = I3STRALLOC(name);
    user->gender = gender;
    user->time = i3_time;
    I3LINK(user, first_ucache, last_ucache, next, prev);

    I3_save_ucache();
}

void I3_send_ucache_update(const char *visname, int gender)
{
    char buf[10];

    if (!is_connected())
        return;

    I3_write_header("ucache-update", this_i3mud->name, NULL, NULL, NULL);
    I3_write_buffer("\"");
    I3_write_buffer(visname);
    I3_write_buffer("\",\"");
    I3_write_buffer(visname);
    I3_write_buffer("\",");
    snprintf(buf, 10, "%d", gender);
    I3_write_buffer(buf);
    I3_write_buffer(",})\r");
    I3_send_packet();
}

void I3_process_ucache_update(I3_HEADER *header, char *s)
{
    char *ps = s, *next_ps;
    char username[MAX_INPUT_LENGTH], visname[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    int sex, gender;

    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    strlcpy(username, ps, MAX_INPUT_LENGTH);

    ps = next_ps;
    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    strlcpy(visname, ps, MAX_INPUT_LENGTH);

    ps = next_ps;
    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    gender = atoi(ps);

    snprintf(buf, MAX_STRING_LENGTH, "%s@%s", visname, header->originator_mudname);

    sex = I3_get_ucache_gender(buf);

    if (sex == gender)
        return;

    I3_ucache_update(buf, gender);
}

void I3_send_chan_user_req(char *targetmud, char *targetuser)
{
    if (!is_connected())
        return;

    I3_write_header("chan-user-req", this_i3mud->name, NULL, targetmud, NULL);
    I3_write_buffer("\"");
    I3_write_buffer(targetuser);
    I3_write_buffer("\",})\r");
    I3_send_packet();
}

void I3_process_chan_user_req(I3_HEADER *header, char *s)
{
    char buf[MAX_STRING_LENGTH];
    char *ps = s, *next_ps;
    int gender;

    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);

    snprintf(buf, MAX_STRING_LENGTH, "%s@%s", header->target_username, this_i3mud->name);
    gender = I3_get_ucache_gender(buf);

    /*
     * Gender of -1 means they aren't in the mud's ucache table, don't waste a packet on a reply
     */
    if (gender == -1)
        return;

    I3_write_header("chan-user-reply", this_i3mud->name, NULL, header->originator_mudname, NULL);
    I3_write_buffer("\"");
    I3_write_buffer(ps);
    I3_write_buffer("\",\"");
    I3_write_buffer(ps);
    I3_write_buffer("\",");
    snprintf(buf, MAX_STRING_LENGTH, "%d", gender);
    I3_write_buffer(buf);
    I3_write_buffer(",})\r");
    I3_send_packet();
}

void I3_process_chan_user_reply(I3_HEADER *header, char *s)
{
    char *ps = s, *next_ps;
    char username[MAX_INPUT_LENGTH], visname[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    int sex, gender;

    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    strlcpy(username, ps, MAX_INPUT_LENGTH);

    ps = next_ps;
    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    strlcpy(visname, ps, MAX_INPUT_LENGTH);

    ps = next_ps;
    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    gender = atoi(ps);

    snprintf(buf, MAX_STRING_LENGTH, "%s@%s", visname, header->originator_mudname);

    sex = I3_get_ucache_gender(buf);

    if (sex == gender)
        return;

    I3_ucache_update(buf, gender);
}

void I3_process_mudlist(I3_HEADER *header, char *s)
{
    char *ps = s, *next_ps;
    I3_MUD *mud = NULL;
    char mud_name[MAX_INPUT_LENGTH];
    ROUTER_DATA *router;

    I3_get_field(ps, &next_ps);
    this_i3mud->mudlist_id = atoi(ps);
    I3_save_id();

    for (router = first_router; router; router = router->next)
    {
        if (!strcasecmp(router->name, I3_ROUTER_NAME))
        {
            router->mudlist_id = this_i3mud->mudlist_id;
            break;
        }
    }
    I3_saverouters();

    // No, this is bad... only do this every 5 minutes under pulse control
    // If we haven't processed the mudlist recently, do it now!
    //if (pulse_mudlist > (PULSE_MUDLIST / 10))
    //    pulse_mudlist = 0;

    ps = next_ps;
    ps += 2;

    while (1)
    {
        char *next_ps2;

        I3_get_field(ps, &next_ps);
        remove_quotes(&ps);
        strlcpy(mud_name, ps, MAX_INPUT_LENGTH);

        ps = next_ps;
        I3_get_field(ps, &next_ps2);

        if (ps[0] != '0')
        {
            mud = find_I3_mud_by_name(mud_name);
            if (!mud)
                mud = new_I3_mud(mud_name);

            ps += 2;
            I3_get_field(ps, &next_ps);
            mud->status = atoi(ps);
            ps = next_ps;

            I3_get_field(ps, &next_ps);
            remove_quotes(&ps);
            I3STRFREE(mud->ipaddress);
            mud->ipaddress = I3STRALLOC(ps);
            ps = next_ps;

            I3_get_field(ps, &next_ps);
            mud->player_port = atoi(ps);
            ps = next_ps;

            I3_get_field(ps, &next_ps);
            mud->imud_tcp_port = atoi(ps);
            ps = next_ps;

            I3_get_field(ps, &next_ps);
            mud->imud_udp_port = atoi(ps);
            ps = next_ps;

            I3_get_field(ps, &next_ps);
            remove_quotes(&ps);
            I3STRFREE(mud->mudlib);
            mud->mudlib = I3STRALLOC(ps);
            ps = next_ps;

            I3_get_field(ps, &next_ps);
            remove_quotes(&ps);
            I3STRFREE(mud->base_mudlib);
            mud->base_mudlib = I3STRALLOC(ps);
            ps = next_ps;

            I3_get_field(ps, &next_ps);
            remove_quotes(&ps);
            I3STRFREE(mud->driver);
            mud->driver = I3STRALLOC(ps);
            ps = next_ps;

            I3_get_field(ps, &next_ps);
            remove_quotes(&ps);
            I3STRFREE(mud->mud_type);
            mud->mud_type = I3STRALLOC(ps);
            ps = next_ps;

            I3_get_field(ps, &next_ps);
            remove_quotes(&ps);
            I3STRFREE(mud->open_status);
            mud->open_status = I3STRALLOC(ps);
            ps = next_ps;

            I3_get_field(ps, &next_ps);
            remove_quotes(&ps);
            I3STRFREE(mud->admin_email);
            mud->admin_email = I3STRALLOC(ps);
            ps = next_ps;

            I3_get_field(ps, &next_ps);

            ps += 2;
            while (1)
            {
                char *next_ps3;
                char key[MAX_INPUT_LENGTH];

                if (ps[0] == ']')
                    break;

                I3_get_field(ps, &next_ps3);
                remove_quotes(&ps);
                strlcpy(key, ps, MAX_INPUT_LENGTH);
                ps = next_ps3;
                I3_get_field(ps, &next_ps3);

                switch (key[0])
                {
                case 'a':
                    if (!strcasecmp(key, "auth"))
                    {
                        mud->auth = ps[0] == '0' ? 0 : 1;
                        break;
                    }
                    if (!strcasecmp(key, "amrcp"))
                    {
                        mud->amrcp = atoi(ps);
                        break;
                    }
                    break;
                case 'b':
                    if (!strcasecmp(key, "beep"))
                    {
                        mud->beep = ps[0] == '0' ? 0 : 1;
                        break;
                    }
                    break;
                case 'c':
                    if (!strcasecmp(key, "channel"))
                    {
                        mud->channel = ps[0] == '0' ? 0 : 1;
                        break;
                    }
                    break;
                case 'e':
                    if (!strcasecmp(key, "emoteto"))
                    {
                        mud->emoteto = ps[0] == '0' ? 0 : 1;
                        break;
                    }
                    break;
                case 'f':
                    if (!strcasecmp(key, "file"))
                    {
                        mud->file = ps[0] == '0' ? 0 : 1;
                        break;
                    }
                    if (!strcasecmp(key, "finger"))
                    {
                        mud->finger = ps[0] == '0' ? 0 : 1;
                        break;
                    }
                    if (!strcasecmp(key, "ftp"))
                    {
                        mud->ftp = atoi(ps);
                        break;
                    }
                    break;
                case 'h':
                    if (!strcasecmp(key, "http"))
                    {
                        mud->http = atoi(ps);
                        break;
                    }
                    break;
                case 'l':
                    if (!strcasecmp(key, "locate"))
                    {
                        mud->locate = ps[0] == '0' ? 0 : 1;
                        break;
                    }
                    break;
                case 'm':
                    if (!strcasecmp(key, "mail"))
                    {
                        mud->mail = ps[0] == '0' ? 0 : 1;
                        break;
                    }
                    break;
                case 'n':
                    if (!strcasecmp(key, "news"))
                    {
                        mud->news = ps[0] == '0' ? 0 : 1;
                        break;
                    }
                    if (!strcasecmp(key, "nntp"))
                    {
                        mud->nntp = atoi(ps);
                        break;
                    }
                    break;
                case 'p':
                    if (!strcasecmp(key, "pop3"))
                    {
                        mud->pop3 = atoi(ps);
                        break;
                    }
                    break;
                case 'r':
                    if (!strcasecmp(key, "rcp"))
                    {
                        mud->rcp = atoi(ps);
                        break;
                    }
                    break;
                case 's':
                    if (!strcasecmp(key, "smtp"))
                    {
                        mud->smtp = atoi(ps);
                        break;
                    }
                    break;
                case 't':
                    if (!strcasecmp(key, "tell"))
                    {
                        mud->tell = ps[0] == '0' ? 0 : 1;
                        break;
                    }
                    break;
                case 'u':
                    if (!strcasecmp(key, "ucache"))
                    {
                        mud->ucache = ps[0] == '0' ? 0 : 1;
                        break;
                    }
                    if (!strcasecmp(key, "url"))
                    {
                        remove_quotes(&ps);
                        I3STRFREE(mud->web_wrong);
                        mud->web_wrong = I3STRALLOC(ps);
                        break;
                    }
                    break;
                case 'w':
                    if (!strcasecmp(key, "who"))
                    {
                        mud->who = ps[0] == '0' ? 0 : 1;
                        break;
                    }
                    break;
                default:
                    break;
                }

                ps = next_ps3;
                if (ps[0] == ']')
                    break;
            }
            ps = next_ps;

            I3_get_field(ps, &next_ps);
            ps = next_ps;
        }
        else
        {
            if ((mud = find_I3_mud_by_name(mud_name)) != NULL)
                destroy_I3_mud(mud);
        }
        ps = next_ps2;
        if (ps[0] == ']')
            break;
    }
}

void I3_process_chanlist_reply(I3_HEADER *header, char *s)
{
    char *ps = s, *next_ps;
    I3_CHANNEL *channel;
    char chan[MAX_INPUT_LENGTH];
    ROUTER_DATA *router;
    int is_new = 0;

    log_info("I3_process_chanlist_reply: %s", "Got chanlist-reply packet!");

    I3_get_field(ps, &next_ps);
    this_i3mud->chanlist_id = atoi(ps);
    I3_save_id();

    for (router = first_router; router; router = router->next)
    {
        if (!strcasecmp(router->name, I3_ROUTER_NAME))
        {
            router->chanlist_id = this_i3mud->chanlist_id;
            break;
        }
    }
    I3_saverouters();

    ps = next_ps;
    ps += 2;

    while (1)
    {
        char *next_ps2;

        // ChanName
        I3_get_field(ps, &next_ps);
        remove_quotes(&ps);
        strlcpy(chan, ps, MAX_INPUT_LENGTH);

        ps = next_ps;
        I3_get_field(ps, &next_ps2);
        if (ps[0] != '0')
        {
            if (!(channel = find_I3_channel_by_name(chan)))
            {
                channel = new_I3_channel();
                channel->I3_name = I3STRALLOC(chan);
                is_new = 1;
                // log_info("New channel %s has been added from router %s", channel->I3_name, I3_ROUTER_NAME);
            }
            else
            {
                is_new = 0;
                // log_info("Channel %s has been updated from router %s", channel->I3_name, I3_ROUTER_NAME);
            }

            // ChanMud
            ps += 2;
            I3_get_field(ps, &next_ps);
            remove_quotes(&ps);
            I3STRFREE(channel->host_mud);
            channel->host_mud = I3STRALLOC(ps);
            ps = next_ps;
            // ChanStatus
            I3_get_field(ps, &next_ps);
            channel->status = atoi(ps);
            log_info("I3 Channel %s: %s@%s (local %s) is %s", (is_new ? "ADDED" : "UPDATED"), channel->I3_name,
                     channel->host_mud, ((channel->local_name && channel->local_name[0]) ? channel->local_name : "--"),
                     (channel->status == 0 ? "public" : "private"));
        }
        else
        {
            if ((channel = find_I3_channel_by_name(chan)) != NULL)
            {
                if (channel->local_name && channel->local_name[0] != '\0')
                {
                    // log_info("Locally configured channel %s has been purged from router %s", channel->local_name,
                    // I3_ROUTER_NAME);
                }
                log_info("I3 Channel %s: %s@%s (local %s) is %s", "DELETED", channel->I3_name, channel->host_mud,
                         ((channel->local_name && channel->local_name[0]) ? channel->local_name : "--"),
                         (channel->status == 0 ? "public" : "private"));
                destroy_I3_channel(channel);
                I3_write_channel_config();
            }
        }
        ps = next_ps2;
        if (ps[0] == ']')
            break;
    }
    log_info("I3_process_chanlist_reply: %s", "Saving channel config data.");
    I3_write_channel_config();
}

void I3_send_channel_message(I3_CHANNEL *channel, const char *name, const char *message)
{
    char buf[MAX_STRING_LENGTH];

    if (!is_connected())
        return;

    strlcpy(buf, message, MAX_STRING_LENGTH);

    // log_info("I3_send_channel(%s@%s, %s, %s)", channel->I3_name, channel->host_mud, name, message);
    I3_write_header("channel-m", this_i3mud->name, name, NULL, NULL);
    // log_info("I3_send_channel() header setup.");
    I3_write_buffer("\"");
    I3_write_buffer(channel->I3_name);
    I3_write_buffer("\",\"");
    I3_write_buffer(I3_nameremap(name));
    // log_info("I3_send_channel() name remap %s to %s.", name, I3_nameremap(name));
    I3_write_buffer("\",\"");
    send_to_i3(I3_escape(buf));
    // log_info("I3_send_channel() escaped buffer.");
    I3_write_buffer("\",})\r");
    lag_spike = i3_time;
    I3_send_packet();
    channel_m_sent++;
    // log_info("I3_send_channel() done.");
}

void I3_send_channel_emote(I3_CHANNEL *channel, const char *name, const char *message)
{
    char buf[MAX_STRING_LENGTH];

    if (!is_connected())
        return;

    if (strstr(message, "$N") == NULL)
        snprintf(buf, MAX_STRING_LENGTH, "$N %s", message);
    else
        strlcpy(buf, message, MAX_STRING_LENGTH);

    I3_write_header("channel-e", this_i3mud->name, name, NULL, NULL);
    I3_write_buffer("\"");
    I3_write_buffer(channel->I3_name);
    I3_write_buffer("\",\"");
    I3_write_buffer(I3_nameremap(name));
    I3_write_buffer("\",\"");
    send_to_i3(I3_escape(buf));
    I3_write_buffer("\",})\r");
    I3_send_packet();
}

void I3_send_channel_t(I3_CHANNEL *channel, const char *name, char *tmud, char *tuser, char *msg_o, char *msg_t,
                       char *tvis)
{
    if (!is_connected())
        return;

    I3_write_header("channel-t", this_i3mud->name, name, NULL, NULL);
    I3_write_buffer("\"");
    I3_write_buffer(channel->I3_name);
    I3_write_buffer("\",\"");
    I3_write_buffer(tmud);
    I3_write_buffer("\",\"");
    I3_write_buffer(tuser);
    I3_write_buffer("\",\"");
    send_to_i3(I3_escape(msg_o));
    I3_write_buffer("\",\"");
    send_to_i3(I3_escape(msg_t));
    I3_write_buffer("\",\"");
    I3_write_buffer(name);
    I3_write_buffer("\",\"");
    I3_write_buffer(tvis);
    I3_write_buffer("\",})\r");
    I3_send_packet();
}

int I3_token(char type, char *str, char *oname, char *tname)
{
    char code[500];
    char *p;

    switch (type)
    {
    default:
        code[0] = type;
        code[1] = '\0';
        return 1;
    case '$':
        strlcpy(code, "$", 500);
        break;
    case ' ':
        strlcpy(code, " ", 500);
        break;
    case 'N': /* Originator's name */
        strlcpy(code, oname, 500);
        break;
    case 'O': /* Target's name */
        strlcpy(code, tname, 500);
        break;
    }
    p = code;
    while (*p != '\0')
    {
        *str = *p++;
        *++str = '\0';
    }
    return strlen(code);
}

void I3_message_convert(char *buffer, const char *txt, char *oname, char *tname)
{
    const char *point;
    int skip = 0;

    for (point = txt; *point; point++)
    {
        if (*point == '$')
        {
            point++;
            if (*point == '\0')
                point--;
            else
                skip = I3_token(*point, buffer, oname, tname);
            while (skip-- > 0)
                ++buffer;
            continue;
        }
        *buffer = *point;
        *++buffer = '\0';
    }
    *buffer = '\0';
}

char *I3_convert_channel_message(const char *message, char *sname, char *tname)
{
    static char msgbuf[MAX_STRING_LENGTH];

    strcpy(msgbuf, "ERROR");
    /*
     * Sanity checks - if any of these are NULL, bad things will happen - Samson 6-29-01
     */
    if (!message)
    {
        log_error("%s", "I3_convert_channel_message: NULL message!");
        return msgbuf;
    }

    if (!sname)
    {
        log_error("%s", "I3_convert_channel_message: NULL sname!");
        return msgbuf;
    }

    if (!tname)
    {
        log_error("%s", "I3_convert_channel_message: NULL tname!");
        return msgbuf;
    }

    I3_message_convert(msgbuf, message, sname, tname);
    return msgbuf;
}

void update_chanhistory(I3_CHANNEL *channel, char *message)
{
    char msg[MAX_STRING_LENGTH], buf[MAX_STRING_LENGTH];
    struct tm *local;
    time_t t;
    int x;

    if (!channel)
    {
        log_error("%s", "update_chanhistory: NULL channel received!");
        return;
    }

    if (!message || message[0] == '\0')
    {
        log_error("%s", "update_chanhistory: NULL message received!");
        return;
    }

    strlcpy(msg, message, MAX_STRING_LENGTH);
    for (x = 0; x < MAX_I3HISTORY; x++)
    {
        if (channel->history[x] == NULL)
        {
            t = time(NULL);
            local = localtime(&t);
            snprintf(buf, MAX_STRING_LENGTH,
                     "%%^RED%%^%%^BOLD%%^%-4.4d-%-2.2d-%-2.2d%%^RESET%%^ %%^GREEN%%^%%^BOLD%%^%s",
                     local->tm_year + 1900, local->tm_mon + 1, local->tm_mday, msg);
            channel->history[x] = I3STRALLOC(buf);

            if (I3IS_SET(channel->flags, I3CHAN_LOG) && channel->local_name && channel->local_name[0])
            {
                FILE *fp;

                snprintf(buf, MAX_STRING_LENGTH, "%s%s.log", I3_DIR, channel->local_name);
                if (!(fp = fopen(buf, "a")))
                {
                    perror(buf);
                    log_error("Could not open file %s!", buf);
                }
                else
                {
                    fprintf(fp, "%s\n", i3_strip_colors(channel->history[x]));
                    I3FCLOSE(fp);
                }
            }
            break;
        }

        if (x == MAX_I3HISTORY - 1)
        {
            int y;

            for (y = 1; y < MAX_I3HISTORY; y++)
            {
                int z = y - 1;

                if (channel->history[z] != NULL)
                {
                    I3STRFREE(channel->history[z]);
                    channel->history[z] = I3STRALLOC(channel->history[y]);
                }
            }

            t = time(NULL);
            local = localtime(&t);
            snprintf(buf, MAX_STRING_LENGTH,
                     "%%^RED%%^%%^BOLD%%^%-4.4d-%-2.2d-%-2.2d%%^RESET%%^ %%^GREEN%%^%%^BOLD%%^%s",
                     local->tm_year + 1900, local->tm_mon + 1, local->tm_mday, msg);
            I3STRFREE(channel->history[x]);
            channel->history[x] = I3STRALLOC(buf);

            if (I3IS_SET(channel->flags, I3CHAN_LOG) && channel->local_name && channel->local_name[0])
            {
                FILE *fp;

                snprintf(buf, MAX_STRING_LENGTH, "%s%s.log", I3_DIR, channel->local_name);
                if (!(fp = fopen(buf, "a")))
                {
                    perror(buf);
                    log_error("Could not open file %s!", buf);
                }
                else
                {
                    fprintf(fp, "%s\n", i3_strip_colors(channel->history[x]));
                    I3FCLOSE(fp);
                }
            }
        }
    }
}

/* Handles the support for channel filtering.
 * Pretty basic right now. Any truly useful filtering would have to be at the discretion of the channel owner anyway.
 */
void I3_chan_filter_m(I3_CHANNEL *channel, I3_HEADER *header, char *s)
{
    char *ps = s, *next_ps;
    char visname[MAX_INPUT_LENGTH], newmsg[MAX_STRING_LENGTH];

    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    strlcpy(visname, ps, MAX_INPUT_LENGTH);

    ps = next_ps;
    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    strlcpy(newmsg, ps, MAX_STRING_LENGTH);
    snprintf(newmsg, MAX_STRING_LENGTH, "%s%s", ps, " (filtered M)");

    I3_write_header("chan-filter-reply", this_i3mud->name, NULL, I3_ROUTER_NAME, NULL);
    I3_write_buffer("\"");
    I3_write_buffer(channel->I3_name);
    I3_write_buffer("\",({\"channel-m\",5,\"");
    I3_write_buffer(header->originator_mudname);
    I3_write_buffer("\",\"");
    I3_write_buffer(header->originator_username);
    I3_write_buffer("\",0,0,\"");
    I3_write_buffer(channel->I3_name);
    I3_write_buffer("\",\"");
    I3_write_buffer(visname);
    I3_write_buffer("\",\"");
    I3_write_buffer(newmsg);
    I3_write_buffer("\",}),})\r");

    I3_send_packet();
}

/* Handles the support for channel filtering.
 * Pretty basic right now. Any truly useful filtering would have to be at the discretion of the channel owner anyway.
 */
void I3_chan_filter_e(I3_CHANNEL *channel, I3_HEADER *header, char *s)
{
    char *ps = s, *next_ps;
    char visname[MAX_INPUT_LENGTH], newmsg[MAX_STRING_LENGTH];

    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    strlcpy(visname, ps, MAX_INPUT_LENGTH);

    ps = next_ps;
    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    snprintf(newmsg, MAX_STRING_LENGTH, "%s%s", ps, " (filtered E)");

    I3_write_header("chan-filter-reply", this_i3mud->name, NULL, I3_ROUTER_NAME, NULL);
    I3_write_buffer("\"");
    I3_write_buffer(channel->I3_name);
    I3_write_buffer("\",({\"channel-e\",5,\"");
    I3_write_buffer(header->originator_mudname);
    I3_write_buffer("\",\"");
    I3_write_buffer(header->originator_username);
    I3_write_buffer("\",0,0,\"");
    I3_write_buffer(channel->I3_name);
    I3_write_buffer("\",\"");
    I3_write_buffer(visname);
    I3_write_buffer("\",\"");
    I3_write_buffer(newmsg);
    I3_write_buffer("\",}),})\r");

    I3_send_packet();
}

/* Handles the support for channel filtering.
 * Pretty basic right now. Any truly useful filtering would have to be at the discretion of the channel owner anyway.
 */
void I3_chan_filter_t(I3_CHANNEL *channel, I3_HEADER *header, char *s)
{
    char *ps = s, *next_ps;
    char targetmud[MAX_INPUT_LENGTH], targetuser[MAX_INPUT_LENGTH], message_o[MAX_STRING_LENGTH],
        message_t[MAX_STRING_LENGTH];
    char visname_o[MAX_INPUT_LENGTH];

    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    strlcpy(targetmud, ps, MAX_INPUT_LENGTH);

    ps = next_ps;
    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    strlcpy(targetuser, ps, MAX_INPUT_LENGTH);

    ps = next_ps;
    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    snprintf(message_o, MAX_STRING_LENGTH, "%s%s", ps, " (filtered T)");

    ps = next_ps;
    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    snprintf(message_t, MAX_STRING_LENGTH, "%s%s", ps, " (filtered T)");

    ps = next_ps;
    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    strlcpy(visname_o, ps, MAX_INPUT_LENGTH);

    ps = next_ps;
    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);

    I3_write_header("chan-filter-reply", this_i3mud->name, NULL, I3_ROUTER_NAME, NULL);
    I3_write_buffer("\"");
    I3_write_buffer(channel->I3_name);
    I3_write_buffer("\",({\"channel-t\",5,\"");
    I3_write_buffer(header->originator_mudname);
    I3_write_buffer("\",\"");
    I3_write_buffer(header->originator_username);
    I3_write_buffer("\",0,0,\"");
    I3_write_buffer(channel->I3_name);
    I3_write_buffer("\",\"");
    I3_write_buffer(targetmud);
    I3_write_buffer("\",\"");
    I3_write_buffer(targetuser);
    I3_write_buffer("\",\"");
    send_to_i3(I3_escape(message_o));
    I3_write_buffer("\",\"");
    send_to_i3(I3_escape(message_t));
    I3_write_buffer("\",\"");
    I3_write_buffer(visname_o);
    I3_write_buffer("\",\"");
    I3_write_buffer(ps);
    I3_write_buffer("\",}),})\r");

    I3_send_packet();
}

void I3_process_channel_filter(I3_HEADER *header, char *s)
{
    char *ps = s, *next_ps;
    char ptype[MAX_INPUT_LENGTH];
    I3_CHANNEL *channel = NULL;
    I3_HEADER *second_header;
    char channel_name[MAX_INPUT_LENGTH];

    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);

    if (!(channel = find_I3_channel_by_name(ps)))
    {
        log_info("I3_process_channel_filter: received unknown channel (%s)", ps);
        return;
    }

    if (!channel->local_name || !channel->local_name[0])
    {
        // return;         // We don't listen to it.
        // But let's go ahead and log it anyways...
        strlcpy(channel_name, ps, MAX_INPUT_LENGTH);
    }
    else
    {
        // Situation normal, copy this to simplify the code down there...
        strlcpy(channel_name, channel->local_name, MAX_INPUT_LENGTH);
    }

    ps = next_ps;
    ps += 2;
    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    strlcpy(ptype, ps, MAX_INPUT_LENGTH);

    second_header = I3_get_header(&ps);

    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);

    if (!strcasecmp(ptype, "channel-m"))
        I3_chan_filter_m(channel, second_header, next_ps);
    if (!strcasecmp(ptype, "channel-e"))
        I3_chan_filter_e(channel, second_header, next_ps);
    if (!strcasecmp(ptype, "channel-t"))
        I3_chan_filter_t(channel, second_header, next_ps);

    I3DISPOSE(second_header);
}

#define I3_ALLCHAN_LOG I3_DIR "i3.allchan.log"

// The speaker here is visname, not originator_username.
// We should probably pass in both, so we can use both.
//
void allchan_log(int is_emote, char *channel, char *speaker, char *username, char *mud, char *str)
{
    FILE *fp = NULL;
    struct tm *local = NULL;
    struct timeval last_time;

    if (i3_time == 0)
    {
        // We called this before the first run of i3_loop(), from outside.
        // Either that, or this code has travelled back in time, and I'm going to be rich!
        gettimeofday(&last_time, NULL);
        i3_time = (time_t)last_time.tv_sec;
    }
    local = localtime(&i3_time);

    if (last_second != i3_time)
    {
        last_second = i3_time;
        sub_second_counter = 0;
    }
    else
    {
        sub_second_counter++;
    }

    if (!(fp = fopen(I3_ALLCHAN_LOG, "a")))
    {
        log_error("Could not open file %s!", I3_ALLCHAN_LOG);
    }
    else
    {
        fprintf(fp, "%04d.%02d.%02d-%02d.%02d,%02d%03d\t%s\t%s@%s\t%c\t%s\n", local->tm_year + 1900, local->tm_mon + 1,
                local->tm_mday, local->tm_hour, local->tm_min, local->tm_sec, sub_second_counter, channel, speaker, mud,
                is_emote ? 't' : 'f', str);
        I3FCLOSE(fp);
    }
    allchan_sql(is_emote, channel, speaker, username, mud, str);
#ifdef DISCORD
    allchan_discord(is_emote, channel, speaker, username, mud, str);
#endif
}

char *color_time(struct tm *local)
{
    static char timestamp[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0\0";
    const char *hours[] = {
        (const char *)"%^BLACK%^%^BOLD%^",
        (const char *)"%^BLACK%^%^BOLD%^",
        (const char *)"%^BLACK%^%^BOLD%^",
        (const char *)"%^BLACK%^%^BOLD%^",
        (const char *)"%^RED%^",
        (const char *)"%^RED%^",
        (const char *)"%^ORANGE%^",
        (const char *)"%^ORANGE%^",
        (const char *)"%^YELLOW%^",
        (const char *)"%^YELLOW%^",
        (const char *)"%^GREEN%^",
        (const char *)"%^GREEN%^",
        (const char *)"%^GREEN%^%^BOLD%^",
        (const char *)"%^GREEN%^%^BOLD%^",
        (const char *)"%^WHITE%^",
        (const char *)"%^WHITE%^",
        (const char *)"%^CYAN%^%^BOLD%^",
        (const char *)"%^CYAN%^%^BOLD%^",
        (const char *)"%^CYAN%^",
        (const char *)"%^CYAN%^",
        (const char *)"%^BLUE%^%^BOLD%^",
        (const char *)"%^BLUE%^%^BOLD%^",
        (const char *)"%^BLUE%^",
        (const char *)"%^BLUE%^"
    };

    snprintf(timestamp, MAX_INPUT_LENGTH, "%s%-2.2d:%s%-2.2d%%^RESET%%^", hours[local->tm_hour], local->tm_hour,
             hours[local->tm_hour], local->tm_min);
    return timestamp;
}

char *color_speaker(const char *speaker)
{
    static char result[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0\0";
    char lowercase_name[MAX_INPUT_LENGTH];
    const char *found = NULL;
    const char *colormap[] = {
        (const char *)"%^BLACK%^%^BOLD%^",
        (const char *)"%^RED%^",
        (const char *)"%^GREEN%^",
        (const char *)"%^ORANGE%^",
        (const char *)"%^BLUE%^",
        (const char *)"%^MAGENTA%^",
        (const char *)"%^CYAN%^",
        (const char *)"%^WHITE%^",
        (const char *)"%^RED%^%^BOLD%^",
        (const char *)"%^GREEN%^%^BOLD%^",
        (const char *)"%^YELLOW%^",
        (const char *)"%^MAGENTA%^%^BOLD%^",
        (const char *)"%^BLUE%^%^BOLD%^",
        (const char *)"%^CYAN%^%^BOLD%^",
        (const char *)"%^WHITE%^%^BOLD%^"
    };

    if (speaker && *speaker)
    {
        bzero(lowercase_name, MAX_INPUT_LENGTH);
        for (int i = 0; i < strlen(speaker) && i < MAX_INPUT_LENGTH; i++)
        {
            if (isalpha(speaker[i]))
                lowercase_name[i] = tolower(speaker[i]);
            else
                lowercase_name[i] = speaker[i];
        }

        found = (const char *)stringmap_find(speaker_db, lowercase_name);
        if (found)
        {
            // Simply return the color code found.
            snprintf(result, MAX_INPUT_LENGTH, "%s", found);
            // For now, always add so it can catch up...
            // addspeaker_sql(lowercase_name, found);
        }
        else
        {
            // Add them as a new speaker and THEN return the color code.
            found = colormap[speaker_count % (sizeof(colormap) / sizeof(colormap[0]))];
            stringmap_add(speaker_db, lowercase_name, (void *)found, strlen(found) + 1);
            speaker_count++;
            I3_saveSpeakers();
            snprintf(result, MAX_INPUT_LENGTH, "%s", found);
            addspeaker_sql(lowercase_name, found);
        }
    }
    return result;
}

void I3_process_channel_t(I3_HEADER *header, char *s)
{
    char *ps = s;
    char *next_ps;
    DESCRIPTOR_DATA *d;
    CHAR_DATA *vch = NULL;
    char targetmud[MAX_INPUT_LENGTH], targetuser[MAX_INPUT_LENGTH], message_o[MAX_STRING_LENGTH],
        message_t[MAX_STRING_LENGTH], buf[MAX_STRING_LENGTH];
    char visname_o[MAX_INPUT_LENGTH], sname[MAX_INPUT_LENGTH], tname[MAX_INPUT_LENGTH], lname[MAX_INPUT_LENGTH],
        layout[MAX_INPUT_LENGTH], tmsg[MAX_STRING_LENGTH], omsg[MAX_STRING_LENGTH];
    I3_CHANNEL *channel = NULL;
    struct tm *local = localtime(&i3_time);
    char channel_name[MAX_INPUT_LENGTH];

    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);

    if (!(channel = find_I3_channel_by_name(ps)))
    {
        log_info("I3_process_channel_t: received unknown channel (%s)", ps);
        return; // We totally don't recognize this channel.
    }

    if (!channel->local_name || !channel->local_name[0])
    {
        // return;         // We don't listen to it.
        // But let's go ahead and log it anyways...
        strlcpy(channel_name, ps, MAX_INPUT_LENGTH);
    }
    else
    {
        // Situation normal, copy this to simplify the code down there...
        strlcpy(channel_name, channel->local_name, MAX_INPUT_LENGTH);
    }

    ps = next_ps;
    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    strlcpy(targetmud, ps, MAX_INPUT_LENGTH);

    ps = next_ps;
    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    strlcpy(targetuser, ps, MAX_INPUT_LENGTH);

    ps = next_ps;
    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    strlcpy(message_o, ps, MAX_STRING_LENGTH);

    ps = next_ps;
    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    strlcpy(message_t, ps, MAX_STRING_LENGTH);

    ps = next_ps;
    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    strlcpy(visname_o, ps, MAX_INPUT_LENGTH);

    ps = next_ps;
    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);

    snprintf(sname, MAX_INPUT_LENGTH, "%s@%s", visname_o, header->originator_mudname);
    snprintf(tname, MAX_INPUT_LENGTH, "%s@%s", ps, targetmud);

    snprintf(omsg, MAX_STRING_LENGTH, "%s", I3_convert_channel_message(message_o, sname, tname));
    snprintf(tmsg, MAX_STRING_LENGTH, "%s", I3_convert_channel_message(message_t, sname, tname));

    strcpy(layout, "%s ");
    strcat(layout, channel->layout_e);

    allchan_log(1, channel_name, visname_o, header->originator_username, header->originator_mudname, omsg);

    for (d = first_descriptor; d; d = d->next)
    {
        vch = d->original ? d->original : d->character;

        if (!vch)
            continue;

        if (!I3_hasname(I3LISTEN(vch), channel_name) || I3_hasname(I3DENY(vch), channel_name))
            continue;

        snprintf(lname, MAX_INPUT_LENGTH, "%s@%s", CH_I3NAME(vch), this_i3mud->name);

        if (d->connected == CON_PLAYING && !i3ignoring(vch, sname))
        {
            if (!strcasecmp(lname, tname))
            {
                sprintf(buf, layout, color_time(local), channel_name, tmsg);
                i3_printf(vch, "%s%%^RESET%%^\r\n", buf);
            }
            else
            {
                sprintf(buf, layout, color_time(local), channel_name, omsg);
                i3_printf(vch, "%s%%^RESET%%^\r\n", buf);
            }
        }
    }
    update_chanhistory(channel, omsg);
    time_to_taunt = getTimestamp() + I3_TAUNT_DELAY;
}

void I3_process_channel_m(I3_HEADER *header, char *s)
{
    char *ps = s;
    char *next_ps;
    DESCRIPTOR_DATA *d;
    CHAR_DATA *vch = NULL;
    char visname[MAX_INPUT_LENGTH], tmpvisname[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH], tps[MAX_STRING_LENGTH],
        format[MAX_INPUT_LENGTH];
    I3_CHANNEL *channel;
    struct tm *local = localtime(&i3_time);
    int len;
    char speaker_color[MAX_INPUT_LENGTH];
    char mud_color[MAX_INPUT_LENGTH];
    char magic_visname[MAX_INPUT_LENGTH];
    char channel_name[MAX_INPUT_LENGTH];

    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);

    if (!(channel = find_I3_channel_by_name(ps)))
    {
        log_info("channel_m: received unknown channel (%s)", ps);
        return; // We totally don't recognize this channel.
    }

    if (!channel->local_name || !channel->local_name[0])
    {
        // return;         // We don't listen to it.
        // But let's go ahead and log it anyways...
        strlcpy(channel_name, ps, MAX_INPUT_LENGTH);
    }
    else
    {
        // Situation normal, copy this to simplify the code down there...
        strlcpy(channel_name, channel->local_name, MAX_INPUT_LENGTH);
    }

    channel_m_received++;
    if (lag_spike > 0)
    {
        lag_spike = i3_time - lag_spike;
        if (lag_spike > record_lag_spike)
            record_lag_spike = lag_spike;
        lag_spike = 0;
    }

    ps = next_ps;
    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    strlcpy(visname, ps, MAX_INPUT_LENGTH);

    ps = next_ps;
    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);

    /* Try to squash multiple trailing newlines in the packet message */
    strlcpy(tps, ps, MAX_STRING_LENGTH);
    len = strlen(tps);
    while (ISNEWL(tps[len - 1]) && len > 1)
    {
        tps[len - 1] = '\0';
        len--;
    }

    strcpy(format, "%s ");
    strcat(format, channel->layout_m);
    snprintf(tmpvisname, MAX_INPUT_LENGTH, "%s@%s", visname, header->originator_mudname);
    if (!strcasecmp(visname, header->originator_username))
    {
        strlcpy(magic_visname, visname, MAX_INPUT_LENGTH);
    }
    else
    {
        snprintf(magic_visname, MAX_INPUT_LENGTH, "%s(%s)", visname, header->originator_username);
    }

    // snprintf(buf, MAX_STRING_LENGTH, format, color_time(local), channel_name, visname, header->originator_mudname,
    // tps);

    // We omit the RESET from the end of the speaker name, so it can also catch the @ if the channel
    // format string doesn't override it.
    snprintf(speaker_color, MAX_INPUT_LENGTH, "%s%s", color_speaker(header->originator_username), magic_visname);
    snprintf(mud_color, MAX_INPUT_LENGTH, "%s%s%%^RESET%%^", color_speaker(header->originator_username),
             header->originator_mudname);
    snprintf(buf, MAX_STRING_LENGTH, format, color_time(local), channel_name, speaker_color, mud_color, tps);

    allchan_log(0, channel_name, visname, header->originator_username, header->originator_mudname, tps);

    for (d = first_descriptor; d; d = d->next)
    {
        if (!d || !d->character)
            continue;

        if (d->connected != CON_PLAYING)
            continue;

        vch = d->original ? d->original : d->character;

        if (!vch)
            continue;

        if (!I3_hasname(I3LISTEN(vch), channel_name) || I3_hasname(I3DENY(vch), channel_name))
            continue;

        if (d->connected == CON_PLAYING)
        {
            if (i3ignoring(vch, visname))
                continue;
            if (i3ignoring(vch, tmpvisname))
                continue;
            i3wrap_printf(vch, "%s%%^RESET%%^\r\n", buf);
        }
    }
    update_chanhistory(channel, buf);
    time_to_taunt = getTimestamp() + I3_TAUNT_DELAY;
}

void I3_process_channel_e(I3_HEADER *header, char *s)
{
    char *ps = s;
    char *next_ps;
    DESCRIPTOR_DATA *d;
    CHAR_DATA *vch = NULL;
    char visname[MAX_INPUT_LENGTH], tmpvisname[MAX_INPUT_LENGTH], msg[MAX_STRING_LENGTH], buf[MAX_STRING_LENGTH],
        tps[MAX_STRING_LENGTH], format[MAX_INPUT_LENGTH];
    I3_CHANNEL *channel;
    struct tm *local = localtime(&i3_time);
    int len;
    char speaker_color[MAX_INPUT_LENGTH];
    char mud_color[MAX_INPUT_LENGTH];
    int hack_len = 0;
    char channel_name[MAX_INPUT_LENGTH];

    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);

    if (!(channel = find_I3_channel_by_name(ps)))
    {
        log_info("channel_e: received unknown channel (%s)", ps);
        return; // We totally don't recognize this channel.
    }

    if (!channel->local_name || !channel->local_name[0])
    {
        // return;         // We don't listen to it.
        // But let's go ahead and log it anyways...
        strlcpy(channel_name, ps, MAX_INPUT_LENGTH);
    }
    else
    {
        // Situation normal, copy this to simplify the code down there...
        strlcpy(channel_name, channel->local_name, MAX_INPUT_LENGTH);
    }

    ps = next_ps;
    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    // snprintf(visname, MAX_INPUT_LENGTH, "%s@%s", ps, header->originator_mudname);
    strlcpy(visname, ps, MAX_INPUT_LENGTH);

    ps = next_ps;
    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);

    /* Try to squash multiple trailing newlines in the packet message */
    strlcpy(tps, ps, MAX_STRING_LENGTH);
    len = strlen(tps);
    while (ISNEWL(tps[len - 1]) && len > 1)
    {
        tps[len - 1] = '\0';
        len--;
    }

    // strcpy(format, "%%^GREEN%%^%%^BOLD%%^%-2.2d%%^WHITE%%^%%^BOLD%%^:%%^GREEN%%^%%^BOLD%%^%-2.2d%%^RESET%%^ ");
    // strcat(format, channel->layout_e);
    // snprintf(buf, MAX_STRING_LENGTH, format, local->tm_hour, local->tm_min, channel_name, msg);

    strcpy(format, "%s ");
    strcat(format, channel->layout_e);
    snprintf(tmpvisname, MAX_INPUT_LENGTH, "%s@%s", visname, header->originator_mudname);

    // We omit the RESET from the end of the speaker name, so it can also catch the @ if the channel
    // format string doesn't override it.
    snprintf(speaker_color, MAX_INPUT_LENGTH, "%s%s", color_speaker(header->originator_username), visname);
    snprintf(mud_color, MAX_INPUT_LENGTH, "%s%s%%^RESET%%^", color_speaker(header->originator_username),
             header->originator_mudname);
    snprintf(msg, MAX_STRING_LENGTH, "%s", I3_convert_channel_message(tps, tmpvisname, tmpvisname));

    // Emotes suck.  The message itself has user@mud embedded in it, so to do things right,
    // we need to skip over that for the actual message, but still handle it with hackery
    strcat(format, "@%s%s");
    hack_len = strlen(tmpvisname);

    snprintf(buf, MAX_STRING_LENGTH, format, color_time(local), channel_name, speaker_color, mud_color, &msg[hack_len]);

    allchan_log(1, channel_name, visname, header->originator_username, header->originator_mudname, msg);

    for (d = first_descriptor; d; d = d->next)
    {
        vch = d->original ? d->original : d->character;

        if (!vch)
            continue;

        if (!I3_hasname(I3LISTEN(vch), channel_name) || I3_hasname(I3DENY(vch), channel_name))
            continue;

        if (d->connected == CON_PLAYING)
        {
            if (i3ignoring(vch, visname))
                continue;
            if (i3ignoring(vch, tmpvisname))
                continue;
            i3wrap_printf(vch, "%s%%^RESET%%^\r\n", buf);
        }
    }
    update_chanhistory(channel, buf);
    time_to_taunt = getTimestamp() + I3_TAUNT_DELAY;
}

void I3_process_chan_who_req(I3_HEADER *header, char *s)
{
    CHAR_DATA *vch;
    DESCRIPTOR_DATA *d;
    char *ps = s, *next_ps;
    char buf[MAX_STRING_LENGTH], ibuf[MAX_INPUT_LENGTH];
    I3_CHANNEL *channel;

    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);

    snprintf(ibuf, MAX_INPUT_LENGTH, "%s@%s", header->originator_username, header->originator_mudname);

    if (!(channel = find_I3_channel_by_name(ps)))
    {
        snprintf(buf, MAX_STRING_LENGTH, "The channel you specified (%s) is unknown at %s", ps, this_i3mud->name);
        I3_send_error(header->originator_mudname, header->originator_username, "unk-channel", buf);
        log_info("chan_who_req: received unknown channel (%s)", ps);
        return;
    }

    if (!channel->local_name)
    {
        snprintf(buf, MAX_STRING_LENGTH, "The channel you specified (%s) is not registered at %s", ps,
                 this_i3mud->name);
        I3_send_error(header->originator_mudname, header->originator_username, "unk-channel", buf);
        return;
    }

    I3_write_header("chan-who-reply", this_i3mud->name, NULL, header->originator_mudname, header->originator_username);
    I3_write_buffer("\"");
    I3_write_buffer(channel->I3_name);
    I3_write_buffer("\",({");

    for (d = first_descriptor; d; d = d->next)
    {
        vch = d->original ? d->original : d->character;

        if (!vch)
            continue;

        if (I3ISINVIS(vch))
            continue;

        if (I3_hasname(I3LISTEN(vch), channel->local_name) && !i3ignoring(vch, ibuf) &&
            !I3_hasname(I3DENY(vch), channel->local_name))
        {
            I3_write_buffer("\"");
            I3_write_buffer(CH_I3NAME(vch));
            I3_write_buffer("\",");
        }
    }
    I3_write_buffer("}),})\r");
    I3_send_packet();
}

void I3_process_chan_who_reply(I3_HEADER *header, char *s)
{
    char *ps = s, *next_ps;
    CHAR_DATA *ch;

    if (!(ch = I3_find_user(header->target_username)))
    {
        log_error("I3_process_chan_who_reply(): user %s not found.", header->target_username);
        return;
    }

    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    i3_printf(ch, "%%^WHITE%%^%%^BOLD%%^Users listening to %s on %s:%%^RESET%%^\r\n\r\n", ps,
              header->originator_mudname);

    ps = next_ps;
    I3_get_field(ps, &next_ps);
    ps += 2;
    while (1)
    {
        if (ps[0] == '}')
        {
            i3_printf(ch, "%%^CYAN%%^No information returned or no people listening.%%^RESET%%^\r\n");
            return;
        }
        I3_get_field(ps, &next_ps);
        remove_quotes(&ps);
        i3_printf(ch, "%%^CYAN%%^%s%%^RESET%%^\r\n", ps);

        ps = next_ps;
        if (ps[0] == '}')
            break;
    }
}

void I3_send_chan_who(CHAR_DATA *ch, I3_CHANNEL *channel, I3_MUD *mud)
{
    if (!is_connected())
        return;

    I3_write_header("chan-who-req", this_i3mud->name, CH_I3NAME(ch), mud->name, NULL);
    I3_write_buffer("\"");
    I3_write_buffer(channel->I3_name);
    I3_write_buffer("\",})\r");
    I3_send_packet();
}

void I3_send_beep(CHAR_DATA *ch, const char *to, I3_MUD *mud)
{
    if (!is_connected())
        return;

    I3_escape(to);
    I3_write_header("beep", this_i3mud->name, CH_I3NAME(ch), mud->name, to);
    I3_write_buffer("\"");
    I3_write_buffer(CH_I3NAME(ch));
    I3_write_buffer("\",})\r");
    I3_send_packet();
}

void I3_process_beep(I3_HEADER *header, char *s)
{
    char buf[MAX_INPUT_LENGTH];
    char *ps = s, *next_ps;
    CHAR_DATA *ch;

    snprintf(buf, MAX_INPUT_LENGTH, "%s@%s", header->originator_username, header->originator_mudname);

    if (!(ch = I3_find_user(header->target_username)))
    {
        if (!i3exists_player(header->target_username))
            I3_send_error(header->originator_mudname, header->originator_username, "unk-user", "No such player.");
        else
            I3_send_error(header->originator_mudname, header->originator_username, "unk-user",
                          "That player is offline.");
        return;
    }

    if (I3PERM(ch) < I3PERM_MORT)
    {
        I3_send_error(header->originator_mudname, header->originator_username, "unk-user", "No such player.");
        return;
    }

    if (I3ISINVIS(ch) || i3ignoring(ch, buf))
    {
        I3_send_error(header->originator_mudname, header->originator_username, "unk-user", "That player is offline.");
        return;
    }

    if (I3IS_SET(I3FLAG(ch), I3_BEEP))
    {
        snprintf(buf, MAX_INPUT_LENGTH, "%s is not accepting beeps.", CH_I3NAME(ch));
        I3_send_error(header->originator_mudname, header->originator_username, "unk-user", buf);
        return;
    }

    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);

    i3_printf(ch, "%%^YELLOW%%^\a%s@%s i3beeps you.%%^RESET%%^\r\n", ps, header->originator_mudname);
}

void I3_send_tell(CHAR_DATA *ch, const char *to, const char *mud, const char *message)
{
    if (!is_connected())
        return;

    I3_escape(to);
    I3_write_header("tell", this_i3mud->name, CH_I3NAME(ch), mud, to);
    I3_write_buffer("\"");
    I3_write_buffer(CH_I3NAME(ch));
    I3_write_buffer("\",\"");
    send_to_i3(I3_escape(message));
    I3_write_buffer("\",})\r");
    I3_send_packet();
}

void i3_update_tellhistory(CHAR_DATA *ch, const char *msg)
{
    char new_msg[MAX_STRING_LENGTH];
    time_t t = time(NULL);
    struct tm *local = localtime(&t);
    int x;

    snprintf(new_msg, MAX_STRING_LENGTH, "%%^RED%%^%%^BOLD%%^[%-2.2d:%-2.2d] %s", local->tm_hour, local->tm_min, msg);

    for (x = 0; x < MAX_I3TELLHISTORY; x++)
    {
        if (I3TELLHISTORY(ch, x) == NULL)
        {
            I3TELLHISTORY(ch, x) = I3STRALLOC(new_msg);
            break;
        }

        if (x == MAX_I3TELLHISTORY - 1)
        {
            int i;

            for (i = 1; i < MAX_I3TELLHISTORY; i++)
            {
                I3STRFREE(I3TELLHISTORY(ch, i - 1));
                I3TELLHISTORY(ch, i - 1) = I3STRALLOC(I3TELLHISTORY(ch, i));
            }
            I3STRFREE(I3TELLHISTORY(ch, x));
            I3TELLHISTORY(ch, x) = I3STRALLOC(new_msg);
        }
    }
}

void I3_process_tell(I3_HEADER *header, char *s)
{
    char buf[MAX_INPUT_LENGTH], usr[MAX_INPUT_LENGTH];
    char *ps = s, *next_ps;
    CHAR_DATA *ch;
    struct tm *local = localtime(&i3_time);

    snprintf(buf, MAX_INPUT_LENGTH, "%s@%s", header->originator_username, header->originator_mudname);

    if (!(ch = I3_find_user(header->target_username)))
    {
        if (!i3exists_player(header->target_username))
            I3_send_error(header->originator_mudname, header->originator_username, "unk-user", "No such player.");
        else
            I3_send_error(header->originator_mudname, header->originator_username, "unk-user",
                          "That player is offline.");
        return;
    }

    if (I3PERM(ch) < I3PERM_MORT)
    {
        I3_send_error(header->originator_mudname, header->originator_username, "unk-user", "No such player.");
        return;
    }

    if (I3ISINVIS(ch) || i3ignoring(ch, buf))
    {
        I3_send_error(header->originator_mudname, header->originator_username, "unk-user", "That player is offline.");
        return;
    }

    if (I3IS_SET(I3FLAG(ch), I3_TELL))
    {
        snprintf(buf, MAX_INPUT_LENGTH, "%s is not accepting tells.", CH_I3NAME(ch));
        I3_send_error(header->originator_mudname, header->originator_username, "unk-user", buf);
        return;
    }

    if (CH_I3AFK(ch))
    {
        snprintf(buf, MAX_INPUT_LENGTH, "%s is currently AFK. Try back later.", CH_I3NAME(ch));
        I3_send_error(header->originator_mudname, header->originator_username, "unk-user", buf);
        return;
    }

    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);

    snprintf(usr, MAX_INPUT_LENGTH, "%s@%s", ps, header->originator_mudname);
    // snprintf(buf, MAX_INPUT_LENGTH, "'%s@%s'", ps, header->originator_mudname);
    // snprintf(buf, MAX_INPUT_LENGTH, "%s@%s", header->originator_username, header->originator_mudname);

    I3STRFREE(I3REPLYNAME(ch));
    I3STRFREE(I3REPLYMUD(ch));
    I3REPLYNAME(ch) = I3STRALLOC(header->originator_username);
    I3REPLYMUD(ch) = I3STRALLOC(header->originator_mudname);

    ps = next_ps;
    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);

    snprintf(buf, MAX_INPUT_LENGTH, "%s %%^CYAN%%^%%^BOLD%%^%s%%^RESET%%^ %%^YELLOW%%^i3tells you: %%^RESET%%^%s",
             color_time(local), usr, ps);
    i3_printf(ch, "%s%%^RESET%%^\r\n", buf);
    i3_update_tellhistory(ch, buf);
    time_to_taunt = getTimestamp() + I3_TAUNT_DELAY;
}

void I3_send_who(CHAR_DATA *ch, char *mud)
{
    if (!is_connected())
        return;

    I3_escape(mud);
    I3_write_header("who-req", this_i3mud->name, CH_I3NAME(ch), mud, NULL);
    I3_write_buffer("})\r");
    I3_send_packet();
}

char *i3centerline(const char *str, int len)
{
    char stripped[MAX_STRING_LENGTH];
    static char outbuf[MAX_STRING_LENGTH];
    int amount;

    strlcpy(stripped, i3_strip_colors(str), MAX_STRING_LENGTH);
    amount = len - strlen(stripped); /* Determine amount to put in front of line */

    if (amount < 1)
        amount = 1;

    /*
     * Justice, you are the String God!
     */
    snprintf(outbuf, MAX_STRING_LENGTH, "%*s%s%*s", (amount / 2), "", str,
             ((amount / 2) * 2) == amount ? (amount / 2) : ((amount / 2) + 1), "");

    return outbuf;
}

char *i3rankbuffer(CHAR_DATA *ch)
{
    static char rbuf[MAX_INPUT_LENGTH];

    if (I3PERM(ch) >= I3PERM_IMM)
    {
        strlcpy(rbuf, "%^YELLOW%^Staff%^RESET%^", MAX_INPUT_LENGTH);

        if (CH_I3RANK(ch) && CH_I3RANK(ch)[0] != '\0')
            snprintf(rbuf, MAX_INPUT_LENGTH, "%%^YELLOW%%^%s%%^RESET%%^", I3_mudtag_to_i3tag(CH_I3RANK(ch)));
    }
    else
    {
        strlcpy(rbuf, "%^GREEN%^Player%^RESET%^", MAX_INPUT_LENGTH);

        if (CH_I3RANK(ch) && CH_I3RANK(ch)[0] != '\0')
            snprintf(rbuf, MAX_INPUT_LENGTH, "%%^GREEN%%^BOLD%%^%s%%^RESET%%^", I3_mudtag_to_i3tag(CH_I3RANK(ch)));
    }
    return rbuf;
}

void I3_write_who_line(const char *str, int idle, const char *xtra)
{
    char tmp[MAX_STRING_LENGTH];

    snprintf(tmp, MAX_STRING_LENGTH, "({\"%s\",%d,\"%s\"})", str, idle, xtra);
    send_to_i3(I3_escape(tmp));
}

void new_I3_process_who_req(I3_HEADER *header, char *s)
{
    struct descriptor_data *d = NULL;
    //struct char_data *person = NULL;
    // int                                     player_count = 0;
    // int                                     immortal_count = 0;
    // int                                     npc_count = 0;

    char boottimebuf[MAX_INPUT_LENGTH];
    // char                                    uptimebuf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    // char                                    nowtimebuf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char ibuf[MAX_INPUT_LENGTH];
    char tmp[MAX_INPUT_LENGTH];
    char header_line_one[MAX_INPUT_LENGTH];
    char header_line_two[MAX_INPUT_LENGTH];

    strftime(boottimebuf, MAX_INPUT_LENGTH, RFC1123FMT, localtime((time_t *)&Uptime));
    snprintf(ibuf, MAX_INPUT_LENGTH, "%s@%s", header->originator_username, header->originator_mudname);

    /*
     * Proper who-reply format should be:
     *
     * ({
     *     (string)  "who-req",
     *     (int)     5,
     *     (string)  originator_mudname,
     *     (string)  "0",
     *     (string)  target_mudname,
     *     (string)  target_username,
     *     (mixed *) who_data
     * })
     *
     * and who_data should be an array with each row being:
     *
     * ({
     *     (string)  user_visname
     *     (int)     idle_time,
     *     (string)  xtra_info
     * })
     */

    // Header line one
    snprintf(tmp, MAX_INPUT_LENGTH,
             "%%^RED%%^%%^BOLD%%^-=[ %%^WHITE%%^%%^BOLD%%^Who's on %s %%^RED%%^%%^BOLD%%^]=-%%^RESET%%^",
             this_i3mud->name);
    strlcpy(header_line_one, i3centerline(tmp, 78), MAX_INPUT_LENGTH);
    // Header line two
    snprintf(tmp, MAX_INPUT_LENGTH, "%%^YELLOW%%^-=[ %%^WHITE%%^%%^BOLD%%^telnet://%s:%d %%^YELLOW%%^]=-%%^RESET%%^",
             this_i3mud->telnet, this_i3mud->player_port);
    strlcpy(header_line_two, i3centerline(tmp, 78), MAX_INPUT_LENGTH);

    // Actual packet stuff
    I3_write_header("who-reply", this_i3mud->name, NULL, header->originator_mudname, header->originator_username);
    I3_write_buffer("({");
    // Header line one
    I3_write_who_line(header_line_one, -1, "");
    // Header line two
    I3_write_buffer(",");
    I3_write_who_line(header_line_two, -1, "");

    for (d = first_descriptor; d; d = d->next)
    {
        //person = d->original ? d->original : d->character;
    }

    I3_write_buffer("})");
    I3_send_packet();
}

void I3_process_who_req(I3_HEADER *header, char *s)
{
    DESCRIPTOR_DATA *d;
    CHAR_DATA *person;
    char ibuf[MAX_INPUT_LENGTH], personbuf[MAX_STRING_LENGTH], tailbuf[MAX_STRING_LENGTH];
    char smallbuf[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH], outbuf[MAX_STRING_LENGTH], stats[MAX_INPUT_LENGTH],
        rank[MAX_INPUT_LENGTH];
    int pcount = 0, xx, yy;
    long int bogusidle = 9999;
    char boottimebuf[MAX_INPUT_LENGTH];

    if (!header || !header->originator_username[0] || !header->originator_mudname[0])
    {
        log_error("Invalid header in who request");
    }

    strftime(boottimebuf, sizeof(boottimebuf), RFC1123FMT, localtime((time_t *)&Uptime));
    snprintf(ibuf, MAX_INPUT_LENGTH, "%s@%s", header->originator_username, header->originator_mudname);

    log_info("Got who request from %s", ibuf);

    I3_write_header("who-reply", this_i3mud->name, NULL, header->originator_mudname, header->originator_username);
    I3_write_buffer("({");

    I3_write_buffer("({\"");
    snprintf(buf, MAX_STRING_LENGTH, "%%^RED%%^%%^BOLD%%^-=[ %%^WHITE%%^%%^BOLD%%^Players on %s %%^RED%%^%%^BOLD%%^]=-",
             this_i3mud->name);
    strlcpy(outbuf, i3centerline(buf, 78), MAX_STRING_LENGTH);
    send_to_i3(I3_escape(outbuf));

    I3_write_buffer("\",");
    snprintf(smallbuf, MAX_INPUT_LENGTH, "%ld", -1l);
    I3_write_buffer(smallbuf);

    I3_write_buffer(",\" \",}),({\"");
    snprintf(buf, MAX_STRING_LENGTH, "%%^YELLOW%%^-=[ %%^WHITE%%^%%^BOLD%%^telnet://%s:%d %%^YELLOW%%^]=-",
             this_i3mud->telnet, this_i3mud->player_port);
    strlcpy(outbuf, i3centerline(buf, 78), MAX_STRING_LENGTH);
    send_to_i3(I3_escape(outbuf));

    I3_write_buffer("\",");
    snprintf(smallbuf, MAX_INPUT_LENGTH, "%ld", bogusidle);
    I3_write_buffer(smallbuf);

    I3_write_buffer(",\" \",}),");

    xx = 0;
    for (d = first_descriptor; d; d = d->next)
    {
        if (!d)
            break;
        person = d->original ? d->original : d->character;

        if (person && d->connected >= CON_PLAYING)
        {
            if (I3PERM(person) < I3PERM_MORT || I3PERM(person) >= I3PERM_IMM || I3ISINVIS(person) ||
                i3ignoring(person, ibuf))
                continue;

            pcount++;

            if (xx == 0)
            {
                I3_write_buffer("({\"");
                send_to_i3(I3_escape("%^BLUE%^%^BOLD%^--------------------------------=[ %^WHITE%^%^BOLD%^Players "
                                     "%^BLUE%^%^BOLD%^]=---------------------------------"));
                I3_write_buffer("\",");
                snprintf(smallbuf, MAX_INPUT_LENGTH, "%ld", bogusidle);
                I3_write_buffer(smallbuf);
                I3_write_buffer(",\" \",}),");
            }

            I3_write_buffer("({\"");

            strlcpy(rank, i3rankbuffer(person), MAX_INPUT_LENGTH);
            strlcpy(outbuf, i3centerline(rank, 20), MAX_STRING_LENGTH);
            send_to_i3(I3_escape(outbuf));

            I3_write_buffer("\",");
            snprintf(smallbuf, MAX_INPUT_LENGTH, "%ld", -1l);
            I3_write_buffer(smallbuf);
            I3_write_buffer(",\"");

            strlcpy(stats, "[", MAX_INPUT_LENGTH);
            if (CH_I3AFK(person))
                strlcat(stats, "AFK", MAX_INPUT_LENGTH);
            else
                strlcat(stats, "---", MAX_INPUT_LENGTH);
            strlcat(stats, "]%^GREEN%^%^BOLD%^", MAX_INPUT_LENGTH);

            snprintf(personbuf, MAX_STRING_LENGTH, "%s %s %s", stats, CH_I3NAME(person), CH_I3TITLE(person));
            send_to_i3(I3_escape(personbuf));
            I3_write_buffer("\",}),");
            xx++;
        }
    }

    yy = 0;
    for (d = first_descriptor; d; d = d->next)
    {
        if (!d)
            break;
        person = d->original ? d->original : d->character;

        if (person && d->connected >= CON_PLAYING)
        {
            if (I3PERM(person) < I3PERM_IMM || I3ISINVIS(person) || i3ignoring(person, ibuf))
                continue;

            pcount++;

            if (yy == 0)
            {
                I3_write_buffer("({\"");
                send_to_i3(I3_escape("%^RED%^%^BOLD%^-------------------------------=[ %^WHITE%^%^BOLD%^Immortals "
                                     "%^RED%^%^BOLD%^]=--------------------------------"));
                I3_write_buffer("\",");
                if (xx > 0)
                    snprintf(smallbuf, MAX_INPUT_LENGTH, "%ld", bogusidle * 3);
                else
                    snprintf(smallbuf, MAX_INPUT_LENGTH, "%ld", bogusidle);
                I3_write_buffer(smallbuf);
                I3_write_buffer(",\" \",}),");
            }
            I3_write_buffer("({\"");

            strlcpy(rank, i3rankbuffer(person), MAX_INPUT_LENGTH);
            strlcpy(outbuf, i3centerline(rank, 20), MAX_STRING_LENGTH);
            send_to_i3(I3_escape(outbuf));

            I3_write_buffer("\",");
            snprintf(smallbuf, MAX_INPUT_LENGTH, "%ld", -1l);
            I3_write_buffer(smallbuf);
            I3_write_buffer(",\"");

            strlcpy(stats, "[", MAX_INPUT_LENGTH);
            if (CH_I3AFK(person))
                strlcat(stats, "AFK", MAX_INPUT_LENGTH);
            else
                strlcat(stats, "---", MAX_INPUT_LENGTH);
            strlcat(stats, "]%^GREEN%^%^BOLD%^", MAX_INPUT_LENGTH);

            snprintf(personbuf, MAX_STRING_LENGTH, "%s %s %s", stats, CH_I3NAME(person), CH_I3TITLE(person));
            send_to_i3(I3_escape(personbuf));
            I3_write_buffer("\",}),");
            yy++;
        }
    }

    I3_write_buffer("({\"");
    snprintf(tailbuf, MAX_STRING_LENGTH, "%%^YELLOW%%^[%%^WHITE%%^%%^BOLD%%^%d Player%s%%^YELLOW%%^]", pcount,
             pcount == 1 ? "" : "s");
    send_to_i3(I3_escape(tailbuf));
    I3_write_buffer("\",");
    snprintf(smallbuf, MAX_INPUT_LENGTH, "%ld", bogusidle * 2);
    I3_write_buffer(smallbuf);
    I3_write_buffer(",\"");
    snprintf(tailbuf, MAX_STRING_LENGTH,
             "%%^YELLOW%%^[%%^WHITE%%^%%^BOLD%%^Homepage: %s%%^YELLOW%%^] [%%^WHITE%%^%%^BOLD%%^%d Max Since "
             "Reboot%%^YELLOW%%^]",
             HTTP_PLACEHOLDER, MAXPLAYERS_PLACEHOLDER);
    send_to_i3(I3_escape(tailbuf));
    I3_write_buffer("\",}),");

    I3_write_buffer("({\"");
    snprintf(tailbuf, MAX_STRING_LENGTH,
             "%%^YELLOW%%^[%%^WHITE%%^%%^BOLD%%^%d logins since last reboot on %s%%^YELLOW%%^]", NUMLOGINS_PLACEHOLDER,
             boottimebuf);
    send_to_i3(I3_escape(tailbuf));
    I3_write_buffer("\",");
    snprintf(smallbuf, MAX_INPUT_LENGTH, "%ld", bogusidle);
    I3_write_buffer(smallbuf);
    I3_write_buffer(",\" \",}),}),})\r");
    I3_send_packet();
}

/* This is where the incoming results of a who-reply packet are processed.
 * Note that rather than just spit the names out, I've copied the packet fields into
 * buffers to be output later. Also note that if it receives an idle value of 9999
 * the normal 30 space output will be bypassed. This is so that muds who want to
 * customize the listing headers in their who-reply packets can do so and the results
 * won't get chopped off after the 30th character. If for some reason a person on
 * the target mud just happens to have been idling for 9999 cycles, their data may
 * be displayed strangely compared to the rest. But I don't expect that 9999 is a very
 * common length of time to be idle either :P
 * Receving an idle value of 19998 may also cause odd results since this is used
 * to indicate receipt of the last line of a who, which is typically the number of
 * visible players found.
 */
void I3_process_who_reply(I3_HEADER *header, char *s)
{
    char *ps = s, *next_ps, *next_ps2;
    CHAR_DATA *ch;
    char person[MAX_STRING_LENGTH], title[MAX_INPUT_LENGTH];
    int idle;

    if (!(ch = I3_find_user(header->target_username)))
        return;

    ps += 2;

    while (1)
    {
        if (ps[0] == '}')
        {
            i3_printf(ch, "%%^WHITE%%^%%^BOLD%%^No information returned.%%^RESET%%^\r\n");
            return;
        }

        I3_get_field(ps, &next_ps);

        ps += 2;
        I3_get_field(ps, &next_ps2);
        remove_quotes(&ps);
        strlcpy(person, ps, MAX_STRING_LENGTH);
        ps = next_ps2;
        I3_get_field(ps, &next_ps2);
        idle = atoi(ps);
        ps = next_ps2;
        I3_get_field(ps, &next_ps2);
        remove_quotes(&ps);
        strlcpy(title, ps, MAX_INPUT_LENGTH);
        ps = next_ps2;

        if (idle == 9999)
            i3_printf(ch, "%s %s\r\n\r\n", person, title);
        else if (idle == 19998)
            i3_printf(ch, "\r\n%s %s\r\n", person, title);
        else if (idle == 29997)
            i3_printf(ch, "\r\n%s %s\r\n\r\n", person, title);
        else
            i3_printf(ch, "%s %s\r\n", person, title);

        ps = next_ps;
        if (ps[0] == '}')
            break;
    }
}

void I3_send_emoteto(CHAR_DATA *ch, const char *to, I3_MUD *mud, const char *message)
{
    char buf[MAX_STRING_LENGTH];

    if (!is_connected())
        return;

    if (strstr(message, "$N") == NULL)
        snprintf(buf, MAX_STRING_LENGTH, "$N %s", message);
    else
        strlcpy(buf, message, MAX_STRING_LENGTH);

    I3_escape(to);
    I3_write_header("emoteto", this_i3mud->name, CH_I3NAME(ch), mud->name, to);
    I3_write_buffer("\"");
    I3_write_buffer(CH_I3NAME(ch));
    I3_write_buffer("\",\"");
    send_to_i3(I3_escape(buf));
    I3_write_buffer("\",})\r");
    I3_send_packet();
}

void I3_process_emoteto(I3_HEADER *header, char *s)
{
    CHAR_DATA *ch;
    char *ps = s, *next_ps;
    char visname[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];

    snprintf(buf, MAX_INPUT_LENGTH, "%s@%s", header->originator_username, header->originator_mudname);

    if (!(ch = I3_find_user(header->target_username)))
    {
        if (!i3exists_player(header->target_username))
            I3_send_error(header->originator_mudname, header->originator_username, "unk-user", "No such player.");
        else
            I3_send_error(header->originator_mudname, header->originator_username, "unk-user",
                          "That player is offline.");
        return;
    }

    if (I3PERM(ch) < I3PERM_MORT)
    {
        I3_send_error(header->originator_mudname, header->originator_username, "unk-user", "No such player.");
        return;
    }

    if (I3ISINVIS(ch) || i3ignoring(ch, buf) || !ch->desc)
    {
        I3_send_error(header->originator_mudname, header->originator_username, "unk-user", "That player is offline.");
        return;
    }

    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    snprintf(visname, MAX_INPUT_LENGTH, "%s@%s", ps, header->originator_mudname);

    ps = next_ps;
    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);

    i3_printf(ch, "%%^CYAN%%^%s%%^RESET%%^\r\n", I3_convert_channel_message(ps, visname, visname));
}

void I3_send_finger(CHAR_DATA *ch, char *user, char *mud)
{
    if (!is_connected())
        return;

    I3_escape(mud);

    I3_write_header("finger-req", this_i3mud->name, CH_I3NAME(ch), mud, NULL);
    I3_write_buffer("\"");
    I3_write_buffer(I3_escape(user));
    I3_write_buffer("\",})\r");
    I3_send_packet();
}

/* The output of this was slightly modified to resemble the Finger snippet */
void I3_process_finger_reply(I3_HEADER *header, char *s)
{
    CHAR_DATA *ch;
    char *ps = s, *next_ps;
    char title[MAX_INPUT_LENGTH], email[MAX_INPUT_LENGTH], last[MAX_INPUT_LENGTH], level[MAX_INPUT_LENGTH];

    if (!(ch = I3_find_user(header->target_username)))
        return;

    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    i3_printf(ch, "%%^WHITE%%^I3FINGER information for %%^GREEN%%^%%^BOLD%%^%s@%s%%^RESET%%^\r\n", ps,
              header->originator_mudname);
    i3_printf(ch, "%%^WHITE%%^-------------------------------------------------%%^RESET%%^\r\n");
    ps = next_ps;

    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    strlcpy(title, ps, MAX_INPUT_LENGTH);
    ps = next_ps;

    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    ps = next_ps;

    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    strlcpy(email, ps, MAX_INPUT_LENGTH);
    ps = next_ps;

    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    strlcpy(last, ps, MAX_INPUT_LENGTH);
    ps = next_ps;

    I3_get_field(ps, &next_ps);
    ps = next_ps;

    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    ps = next_ps;

    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    strlcpy(level, ps, MAX_INPUT_LENGTH);
    ps = next_ps;

    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);

    i3_printf(ch, "%%^WHITE%%^Title: %%^GREEN%%^%%^BOLD%%^%s%%^RESET%%^\r\n", title);
    i3_printf(ch, "%%^WHITE%%^Level: %%^GREEN%%^%%^BOLD%%^%s%%^RESET%%^\r\n", level);
    i3_printf(ch, "%%^WHITE%%^Email: %%^GREEN%%^%%^BOLD%%^%s%%^RESET%%^\r\n", email);
    i3_printf(ch, "%%^WHITE%%^HTTP : %%^GREEN%%^%%^BOLD%%^%s%%^RESET%%^\r\n", ps);
    i3_printf(ch, "%%^WHITE%%^Last on: %%^GREEN%%^%%^BOLD%%^%s%%^RESET%%^\r\n", last);
}

void I3_process_finger_req(I3_HEADER *header, char *s)
{
    CHAR_DATA *ch;
    char *ps = s, *next_ps;
    char smallbuf[200], buf[MAX_INPUT_LENGTH];

    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);

    snprintf(buf, MAX_INPUT_LENGTH, "%s@%s", header->originator_username, header->originator_mudname);

    if (!(ch = I3_find_user(ps)))
    {
        if (!i3exists_player(ps))
            I3_send_error(header->originator_mudname, header->originator_username, "unk-user", "No such player.");
        else
            I3_send_error(header->originator_mudname, header->originator_username, "unk-user",
                          "That player is offline.");
        return;
    }

    if (I3PERM(ch) < I3PERM_MORT)
    {
        I3_send_error(header->originator_mudname, header->originator_username, "unk-user", "No such player.");
        return;
    }

    if (I3ISINVIS(ch) || i3ignoring(ch, buf))
    {
        I3_send_error(header->originator_mudname, header->originator_username, "unk-user", "That player is offline.");
        return;
    }

    if (I3IS_SET(I3FLAG(ch), I3_DENYFINGER) || I3IS_SET(I3FLAG(ch), I3_PRIVACY))
    {
        snprintf(buf, MAX_INPUT_LENGTH, "%s is not accepting fingers.", CH_I3NAME(ch));
        I3_send_error(header->originator_mudname, header->originator_username, "unk-user", buf);
        return;
    }

    i3_printf(ch, "%s@%s has requested your i3finger information.\r\n", header->originator_username,
              header->originator_mudname);

    I3_write_header("finger-reply", this_i3mud->name, NULL, header->originator_mudname, header->originator_username);
    I3_write_buffer("\"");
    I3_write_buffer(I3_escape(CH_I3NAME(ch)));
    I3_write_buffer("\",\"");
    // I3_write_buffer(I3_escape(CH_I3NAME(ch)));
    // I3_write_buffer(" ");
    send_to_i3(I3_escape(CH_I3TITLE(ch)));
    I3_write_buffer("\",\"\",\"");
#if 0
    if (ch->pcdata->email) {
	if (!IS_SET(ch->pcdata->flags, PCFLAG_PRIVACY))
	    I3_write_buffer(ch->pcdata->email);
	else
	    I3_write_buffer("[Private]");
    }
#endif
    I3_write_buffer("[Private]");
    I3_write_buffer("\",\"");
    strlcpy(smallbuf, "-1", 200); /* online since */
    I3_write_buffer(smallbuf);
    I3_write_buffer("\",");
    snprintf(smallbuf, 200, "%ld", -1l);
    I3_write_buffer(smallbuf);
    I3_write_buffer(",\"");
    I3_write_buffer("[PRIVATE]");
    I3_write_buffer("\",\"");
    snprintf(buf, MAX_INPUT_LENGTH, "%s", i3rankbuffer(ch));
    send_to_i3(buf);
    I3_write_buffer("\",\"");
#if 0
    if (ch->pcdata->homepage)
	I3_write_buffer(I3_escape(ch->pcdata->homepage));
    else
	I3_write_buffer("Not Provided");
#endif
    I3_write_buffer("[Private]");
    I3_write_buffer("\",})\r");
    I3_send_packet();
}

void I3_send_locate(CHAR_DATA *ch, const char *user)
{
    if (!is_connected())
        return;

    I3_write_header("locate-req", this_i3mud->name, CH_I3NAME(ch), NULL, NULL);
    I3_write_buffer("\"");
    I3_write_buffer(I3_escape(user));
    I3_write_buffer("\",})\r");
    I3_send_packet();
}

void I3_process_locate_reply(I3_HEADER *header, char *s)
{
    char mud_name[MAX_INPUT_LENGTH], user_name[MAX_INPUT_LENGTH], status[MAX_INPUT_LENGTH];
    char *ps = s, *next_ps;
    CHAR_DATA *ch;

    if (!(ch = I3_find_user(header->target_username)))
        return;

    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    strlcpy(mud_name, ps, MAX_INPUT_LENGTH);
    ps = next_ps;

    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    strlcpy(user_name, ps, MAX_INPUT_LENGTH);
    ps = next_ps;

    I3_get_field(ps, &next_ps);
    ps = next_ps;

    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    strlcpy(status, ps, MAX_INPUT_LENGTH);

    if (!strcasecmp(status, "active"))
        strlcpy(status, "Online", MAX_INPUT_LENGTH);

    if (!strcasecmp(status, "exists, but not logged on"))
        strlcpy(status, "Offline", MAX_INPUT_LENGTH);

    i3_printf(ch, "%%^RED%%^%%^BOLD%%^I3 Locate: %%^YELLOW%%^%s@%s: %%^CYAN%%^%s.%%^RESET%%^\r\n", user_name, mud_name,
              status);
}

void I3_process_locate_req(I3_HEADER *header, char *s)
{
    char *ps = s, *next_ps;
    char smallbuf[50], buf[MAX_INPUT_LENGTH];
    CHAR_DATA *ch;
    bool choffline = FALSE;

    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);

    snprintf(buf, MAX_INPUT_LENGTH, "%s@%s", header->originator_username, header->originator_mudname);

    if (!(ch = I3_find_user(ps)))
    {
        if (i3exists_player(ps))
            choffline = TRUE;
        else
            return;
    }

    if (ch)
    {
        if (I3PERM(ch) < I3PERM_MORT)
            return;

        if (I3ISINVIS(ch))
            choffline = TRUE;

        if (i3ignoring(ch, buf))
            choffline = TRUE;
    }

    I3_write_header("locate-reply", this_i3mud->name, NULL, header->originator_mudname, header->originator_username);
    I3_write_buffer("\"");
    I3_write_buffer(this_i3mud->name);
    I3_write_buffer("\",\"");
    if (!choffline)
        I3_write_buffer(CH_I3NAME(ch));
    else
        I3_write_buffer(i3capitalize(ps));
    I3_write_buffer("\",");
    snprintf(smallbuf, 50, "%ld", -1l);
    I3_write_buffer(smallbuf);
    if (!choffline)
        I3_write_buffer(",\"Online\",})\r");
    else
        I3_write_buffer(",\"Offline\",})\r");
    I3_send_packet();
}

void I3_send_channel_listen(I3_CHANNEL *channel, bool lconnect)
{
    if (!is_connected())
        return;

    I3_write_header("channel-listen", this_i3mud->name, NULL, I3_ROUTER_NAME, NULL);
    I3_write_buffer("\"");
    I3_write_buffer(channel->I3_name);
    I3_write_buffer("\",");
    if (lconnect)
        I3_write_buffer("1,})\r");
    else
        I3_write_buffer("0,})\r");
    I3_send_packet();
}

void I3_process_channel_adminlist_reply(I3_HEADER *header, char *s)
{
    char *ps = s, *next_ps;
    I3_CHANNEL *channel;
    CHAR_DATA *ch;

    if ((ch = I3_find_user(header->target_username)) == NULL)
    {
        log_error("I3_process_channel_adminlist_reply(): user %s not found.", header->target_username);
        return;
    }

    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    if (!(channel = find_I3_channel_by_name(ps)))
    {
        log_error("I3_process_channel_adminlist_reply(): Invalid local channel %s reply received.", ps);
        return;
    }
    i3_printf(ch, "%%^RED%%^%%^BOLD%%^The following muds are %s %s:%%^RESET%%^\r\n\r\n",
              channel->status == 0 ? "banned from" : "invited to", channel->local_name);

    ps = next_ps;
    I3_get_field(ps, &next_ps);
    ps += 2;
    while (1)
    {
        if (ps[0] == '}')
        {
            i3_printf(ch, "%%^YELLOW%%^No entries found.%%^RESET%%^\r\n");
            return;
        }

        I3_get_field(ps, &next_ps);
        remove_quotes(&ps);
        i3_printf(ch, "%%^YELLOW%%^%s%%^RESET%%^\r\n", ps);

        ps = next_ps;
        if (ps[0] == '}')
            break;
    }
}

void I3_send_channel_adminlist(CHAR_DATA *ch, char *chan_name)
{
    if (!is_connected())
        return;

    I3_write_header("chan-adminlist", this_i3mud->name, CH_I3NAME(ch), I3_ROUTER_NAME, NULL);
    I3_write_buffer("\"");
    I3_write_buffer(chan_name);
    I3_write_buffer("\",})\r");
    I3_send_packet();
}

void I3_send_channel_admin(CHAR_DATA *ch, char *chan_name, char *list)
{
    if (!is_connected())
        return;

    I3_write_header("channel-admin", this_i3mud->name, CH_I3NAME(ch), I3_ROUTER_NAME, NULL);
    I3_write_buffer("\"");
    I3_write_buffer(chan_name);
    I3_write_buffer("\",");
    I3_write_buffer(list);
    I3_write_buffer("})\r");
    I3_send_packet();
}

void I3_send_channel_add(CHAR_DATA *ch, char *arg, int type)
{
    if (!is_connected())
        return;

    I3_write_header("channel-add", this_i3mud->name, CH_I3NAME(ch), I3_ROUTER_NAME, NULL);
    I3_write_buffer("\"");
    I3_write_buffer(arg);
    I3_write_buffer("\",");
    switch (type)
    {
    default:
        log_error("%s", "I3_send_channel_add: Illegal channel type!");
        return;
    case 0:
        I3_write_buffer("0,})\r");
        break;
    case 1:
        I3_write_buffer("1,})\r");
        break;
    case 2:
        I3_write_buffer("2,})\r");
        break;
    }
    I3_send_packet();
}

void I3_send_channel_remove(CHAR_DATA *ch, I3_CHANNEL *channel)
{
    if (!is_connected())
        return;

    I3_write_header("channel-remove", this_i3mud->name, CH_I3NAME(ch), I3_ROUTER_NAME, NULL);
    I3_write_buffer("\"");
    I3_write_buffer(channel->I3_name);
    I3_write_buffer("\",})\r");
    I3_send_packet();
}

void I3_send_shutdown(int delay)
{
    I3_CHANNEL *channel;
    char s[50];

    if (!is_connecting())
        return;

    if (is_connected())
    {
        for (channel = first_I3chan; channel; channel = channel->next)
        {
            // if (channel->local_name && channel->local_name[0] != '\0')
            I3_send_channel_listen(channel, FALSE);
        }
    }

    I3_write_header("shutdown", this_i3mud->name, NULL, I3_ROUTER_NAME, NULL);
    snprintf(s, 50, "%d", delay);
    I3_write_buffer(s);
    I3_write_buffer(",})\r");

    if (!I3_write_packet(I3_output_buffer))
        I3_connection_close(FALSE);
}

/*
 * Read the header of an I3 packet. pps will point to the next field
 * of the packet.
 */
I3_HEADER *I3_get_header(char **pps)
{
    I3_HEADER *header;
    char *ps = *pps, *next_ps;

    I3CREATE(header, I3_HEADER, 1);

    I3_get_field(ps, &next_ps);
    ps = next_ps;
    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    strlcpy(header->originator_mudname, ps, MAX_INPUT_LENGTH);
    ps = next_ps;
    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    strlcpy(header->originator_username, ps, MAX_INPUT_LENGTH);
    ps = next_ps;
    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    strlcpy(header->target_mudname, ps, MAX_INPUT_LENGTH);
    ps = next_ps;
    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    strlcpy(header->target_username, ps, MAX_INPUT_LENGTH);

    *pps = next_ps;
    return header;
}

/*
 * Read the first field of an I3 packet and call the proper function to
 * process it. Afterwards the original I3 packet is completly messed up.
 *
 * Reworked on 9-5-03 by Samson to be made more efficient with regard to banned muds.
 * Also only need to gather the header information once this way instead of in multiple places.
 */
void I3_parse_packet(void)
{
    I3_HEADER *header = NULL;
    char *ps, *next_ps;
    char ptype[MAX_INPUT_LENGTH];

    ps = I3_currentpacket;
    if (ps[0] != '(' || ps[1] != '{')
        return;

    if (packetdebug)
        log_info("I3_PACKET Packet received: %s", ps);

    ps += 2;
    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    strlcpy(ptype, ps, MAX_INPUT_LENGTH);

    header = I3_get_header(&ps);

    // At this point, I3_incoming_packet_strlen holds
    // the length of the packet content (minus the length bytes
    // and the NUL byte).
    // ptype is the packet type.
    //
    // So if we want to store statistics about it, this is the
    // point to do so!

    // i3_packet_log(ptype, I3_incoming_packet_strlen, I3_incoming_packet);
    i3_packet_log(ptype, I3_incoming_packet_strlen, NULL);

    /*
     * There. Nice and simple, no?
     */
    if (i3banned(header->originator_mudname))
        return;

    if (!strcasecmp(ptype, "tell"))
        I3_process_tell(header, ps);

    if (!strcasecmp(ptype, "beep"))
        I3_process_beep(header, ps);

    if (!strcasecmp(ptype, "emoteto"))
        I3_process_emoteto(header, ps);

    if (!strcasecmp(ptype, "channel-m"))
        I3_process_channel_m(header, ps);

    if (!strcasecmp(ptype, "channel-e"))
        I3_process_channel_e(header, ps);

    if (!strcasecmp(ptype, "chan-filter-req"))
        I3_process_channel_filter(header, ps);

    if (!strcasecmp(ptype, "finger-req"))
        I3_process_finger_req(header, ps);

    if (!strcasecmp(ptype, "finger-reply"))
        I3_process_finger_reply(header, ps);

    if (!strcasecmp(ptype, "locate-req"))
        I3_process_locate_req(header, ps);

    if (!strcasecmp(ptype, "locate-reply"))
        I3_process_locate_reply(header, ps);

    if (!strcasecmp(ptype, "chan-who-req"))
        I3_process_chan_who_req(header, ps);

    if (!strcasecmp(ptype, "chan-who-reply"))
        I3_process_chan_who_reply(header, ps);

    if (!strcasecmp(ptype, "chan-adminlist-reply"))
        I3_process_channel_adminlist_reply(header, ps);

    if (!strcasecmp(ptype, "ucache-update") && this_i3mud->ucache == TRUE)
        I3_process_ucache_update(header, ps);

    if (!strcasecmp(ptype, "who-req"))
        I3_process_who_req(header, ps);

    if (!strcasecmp(ptype, "who-reply"))
        I3_process_who_reply(header, ps);

    if (!strcasecmp(ptype, "chanlist-reply"))
        I3_process_chanlist_reply(header, ps);

    if (!strcasecmp(ptype, "startup-reply"))
        I3_process_startup_reply(header, ps);

    if (!strcasecmp(ptype, "mudlist"))
        I3_process_mudlist(header, ps);

    if (!strcasecmp(ptype, "error"))
        I3_process_error(header, ps);

    if (!strcasecmp(ptype, "chan-ack"))
        I3_process_chanack(header, ps);

    if (!strcasecmp(ptype, "channel-t"))
        I3_process_channel_t(header, ps);

    if (!strcasecmp(ptype, "chan-user-req"))
        I3_process_chan_user_req(header, ps);

    if (!strcasecmp(ptype, "chan-user-reply") && this_i3mud->ucache == TRUE)
        I3_process_chan_user_reply(header, ps);

    if (!strcasecmp(ptype, "router-shutdown"))
    {
        int delay;

        I3_get_field(ps, &next_ps);
        delay = atoi(ps);

        if (delay == 0)
        {
            log_info("Router %s is shutting down.", I3_ROUTER_NAME);
            I3_connection_close(FALSE);
        }
        else
        {
            log_info("Router %s is rebooting and will be back in %d second%s.", I3_ROUTER_NAME, delay,
                     delay == 1 ? "" : "s");
            I3_connection_close(TRUE);
        }
    }
    I3DISPOSE(header);
}

/*
 * Read one I3 packet into the I3_input_buffer
 */
void I3_read_packet(void)
{
    long size;

    memmove(&size, I3_input_buffer, 4);
    size = ntohl(size);

    memmove(I3_currentpacket, I3_input_buffer + 4, size);

    if (I3_currentpacket[size - 1] != ')')
        I3_currentpacket[size - 1] = 0;
    I3_currentpacket[size] = 0;

    memmove(I3_input_buffer, I3_input_buffer + size + 4, I3_input_pointer - size - 4);
    I3_input_pointer -= size + 4;
}

void I3_handle_packet(char *packetBuffer)
{
    I3_HEADER *header = NULL;
    char *ps, *next_ps;
    char ptype[MAX_INPUT_LENGTH];

    if (packetdebug)
        log_info("I3_PACKET Packet received: %s", packetBuffer);

    ps = packetBuffer;

    ps += 2;
    I3_get_field(ps, &next_ps);
    remove_quotes(&ps);
    strlcpy(ptype, ps, MAX_INPUT_LENGTH);

    header = I3_get_header(&ps);

    // At this point, I3_incoming_packet_strlen holds
    // the length of the packet content (minus the length bytes
    // and the NUL byte).
    // ptype is the packet type.
    //
    // So if we want to store statistics about it, this is the
    // point to do so!

    // i3_packet_log(ptype, I3_incoming_packet_strlen, I3_incoming_packet);
    i3_packet_log(ptype, I3_incoming_packet_strlen, NULL);

    /*
     * There. Nice and simple, no?
     * -- a nice and simple memory leak (Quixadhal)
     */
    if (i3banned(header->originator_mudname))
    {
        I3DISPOSE(header);
        return;
    }

    if (!strcasecmp(ptype, "tell"))
        I3_process_tell(header, ps);

    if (!strcasecmp(ptype, "beep"))
        I3_process_beep(header, ps);

    if (!strcasecmp(ptype, "emoteto"))
        I3_process_emoteto(header, ps);

    if (!strcasecmp(ptype, "channel-m"))
        I3_process_channel_m(header, ps);

    if (!strcasecmp(ptype, "channel-e"))
        I3_process_channel_e(header, ps);

    if (!strcasecmp(ptype, "chan-filter-req"))
        I3_process_channel_filter(header, ps);

    if (!strcasecmp(ptype, "finger-req"))
        I3_process_finger_req(header, ps);

    if (!strcasecmp(ptype, "finger-reply"))
        I3_process_finger_reply(header, ps);

    if (!strcasecmp(ptype, "locate-req"))
        I3_process_locate_req(header, ps);

    if (!strcasecmp(ptype, "locate-reply"))
        I3_process_locate_reply(header, ps);

    if (!strcasecmp(ptype, "chan-who-req"))
        I3_process_chan_who_req(header, ps);

    if (!strcasecmp(ptype, "chan-who-reply"))
        I3_process_chan_who_reply(header, ps);

    if (!strcasecmp(ptype, "chan-adminlist-reply"))
        I3_process_channel_adminlist_reply(header, ps);

    if (!strcasecmp(ptype, "ucache-update") && this_i3mud->ucache == TRUE)
        I3_process_ucache_update(header, ps);

    if (!strcasecmp(ptype, "who-req"))
        I3_process_who_req(header, ps);

    if (!strcasecmp(ptype, "who-reply"))
        I3_process_who_reply(header, ps);

    if (!strcasecmp(ptype, "chanlist-reply"))
        I3_process_chanlist_reply(header, ps);

    if (!strcasecmp(ptype, "startup-reply"))
        I3_process_startup_reply(header, ps);

    if (!strcasecmp(ptype, "mudlist"))
        I3_process_mudlist(header, ps);

    if (!strcasecmp(ptype, "error"))
        I3_process_error(header, ps);

    if (!strcasecmp(ptype, "chan-ack"))
        I3_process_chanack(header, ps);

    if (!strcasecmp(ptype, "channel-t"))
        I3_process_channel_t(header, ps);

    if (!strcasecmp(ptype, "chan-user-req"))
        I3_process_chan_user_req(header, ps);

    if (!strcasecmp(ptype, "chan-user-reply") && this_i3mud->ucache == TRUE)
        I3_process_chan_user_reply(header, ps);

    if (!strcasecmp(ptype, "router-shutdown"))
    {
        int delay;

        I3_get_field(ps, &next_ps);
        delay = atoi(ps);

        if (delay == 0)
        {
            log_info("Router %s is shutting down.", I3_ROUTER_NAME);
            I3_connection_close(FALSE);
        }
        else
        {
            log_info("Router %s is rebooting and will be back in %d second%s.", I3_ROUTER_NAME, delay,
                     delay == 1 ? "" : "s");
            I3_connection_close(TRUE);
        }
    }

    I3DISPOSE(header);
}

/************************************
 * User login and logout functions. *
 ************************************/

void i3_initchar(CHAR_DATA *ch)
{
    log_info("Setting up I3 data");
    if (IS_NPC(ch))
        return;

    I3CREATE(CH_I3DATA(ch), I3_CHARDATA, 1);
    I3LISTEN(ch) = NULL;
    I3DENY(ch) = NULL;
    I3REPLYNAME(ch) = NULL;
    I3REPLYMUD(ch) = NULL;
    I3FLAG(ch) = 0;
    I3SET_BIT(I3FLAG(ch), I3_COLORFLAG); /* Default color to on. People can turn this off if they
                                          * hate it. */
    FIRST_I3IGNORE(ch) = NULL;
    LAST_I3IGNORE(ch) = NULL;
    I3PERM(ch) = I3PERM_MORT;

    log_info("Done");
}

void i3_freechardata(CHAR_DATA *ch)
{
    I3_IGNORE *temp, *next;
    int x;

    if (IS_NPC(ch))
        return;

    if (!CH_I3DATA(ch))
        return;

    I3STRFREE(I3LISTEN(ch));
    I3STRFREE(I3DENY(ch));
    I3STRFREE(I3REPLYNAME(ch));
    I3STRFREE(I3REPLYMUD(ch));

    if (FIRST_I3IGNORE(ch))
    {
        for (temp = FIRST_I3IGNORE(ch); temp; temp = next)
        {
            next = temp->next;
            I3STRFREE(temp->name);
            I3UNLINK(temp, FIRST_I3IGNORE(ch), LAST_I3IGNORE(ch), next, prev);
            I3DISPOSE(temp);
        }
    }
    for (x = 0; x < MAX_I3TELLHISTORY; x++)
        I3STRFREE(I3TELLHISTORY(ch, x));

    I3DISPOSE(CH_I3DATA(ch));
}

void I3_adjust_perms(CHAR_DATA *ch)
{
    if (!this_i3mud)
        return;

    /*
     * Ugly hack to let the permission system adapt freely, but retains the ability to override that adaptation
     * * in the event you need to restrict someone to a lower level, or grant someone a higher level. This of
     * * course comes at the cost of forgetting you may have done so and caused the override flag to be set, but hey.
     * * This isn't a perfect system and never will be. Samson 2-8-04.
     */
    if (!I3IS_SET(I3FLAG(ch), I3_PERMOVERRIDE))
    {
        if (CH_I3LEVEL(ch) < this_i3mud->minlevel)
            I3PERM(ch) = I3PERM_NONE;
        else if (CH_I3LEVEL(ch) >= this_i3mud->minlevel && CH_I3LEVEL(ch) < this_i3mud->immlevel)
            I3PERM(ch) = I3PERM_MORT;
        else if (CH_I3LEVEL(ch) >= this_i3mud->immlevel && CH_I3LEVEL(ch) < this_i3mud->adminlevel)
            I3PERM(ch) = I3PERM_IMM;
        else if (CH_I3LEVEL(ch) >= this_i3mud->adminlevel && CH_I3LEVEL(ch) < this_i3mud->implevel)
            I3PERM(ch) = I3PERM_ADMIN;
        else if (CH_I3LEVEL(ch) >= this_i3mud->implevel)
            I3PERM(ch) = I3PERM_IMP;
    }
}

void I3_char_login(CHAR_DATA *ch)
{
    int gender, sex;
    char buf[MAX_INPUT_LENGTH];

    if (!this_i3mud)
        return;

    I3_adjust_perms(ch);

    if (!is_connected())
    {
        if (I3PERM(ch) >= I3PERM_IMM)
            i3_printf(ch, "%%^RED%%^%%^BOLD%%^The Intermud-3 connection is down. Attempts to reconnect were abandoned "
                          "due to excessive failures.%%^RESET%%^\r\n");
        return;
    }

    if (I3PERM(ch) < I3PERM_MORT)
        return;

    if (this_i3mud->ucache == TRUE)
    {
        snprintf(buf, MAX_INPUT_LENGTH, "%s@%s", CH_I3NAME(ch), this_i3mud->name);
        gender = I3_get_ucache_gender(buf);
        sex = dikutoi3gender(CH_I3SEX(ch));

        if (gender == sex)
            return;

        I3_ucache_update(buf, sex);
        if (!I3IS_SET(I3FLAG(ch), I3_INVIS))
            I3_send_ucache_update(CH_I3NAME(ch), sex);
    }
}

bool i3_loadchar(CHAR_DATA *ch, FILE *fp, const char *word)
{
    bool fMatch = FALSE;

    if (IS_NPC(ch))
        return FALSE;

    if (I3PERM(ch) == I3PERM_NOTSET)
        I3_adjust_perms(ch);

    switch (UPPER(word[0]))
    {
    case 'I':
        KEY("i3perm", I3PERM(ch), i3fread_number(fp));
        if (!strcasecmp(word, "i3flags"))
        {
            I3FLAG(ch) = i3fread_number(fp);
            I3_char_login(ch);
            fMatch = TRUE;
            break;
        }

        if (!strcasecmp(word, "i3listen"))
        {
            I3LISTEN(ch) = i3fread_line(fp);
            if (I3LISTEN(ch) != NULL && is_connected())
            {
                I3_CHANNEL *channel = NULL;
                const char *channels = I3LISTEN(ch);
                char arg[MAX_INPUT_LENGTH];

                while (1)
                {
                    if (channels[0] == '\0')
                        break;
                    channels = i3one_argument(channels, arg);

                    if (!(channel = find_I3_channel_by_localname(arg)))
                        I3_unflagchan(&I3LISTEN(ch), arg);
                    if (channel && I3PERM(ch) < channel->i3perm)
                        I3_unflagchan(&I3LISTEN(ch), arg);
                }
            }
            fMatch = TRUE;
            break;
        }

        if (!strcasecmp(word, "i3deny"))
        {
            I3DENY(ch) = i3fread_line(fp);
            if (I3DENY(ch) != NULL && is_connected())
            {
                I3_CHANNEL *channel = NULL;
                const char *channels = I3DENY(ch);
                char arg[MAX_INPUT_LENGTH];

                while (1)
                {
                    if (channels[0] == '\0')
                        break;
                    channels = i3one_argument(channels, arg);

                    if (!(channel = find_I3_channel_by_localname(arg)))
                        I3_unflagchan(&I3DENY(ch), arg);
                    if (channel && I3PERM(ch) < channel->i3perm)
                        I3_unflagchan(&I3DENY(ch), arg);
                }
            }
            fMatch = TRUE;
            break;
        }
        if (!strcasecmp(word, "i3ignore"))
        {
            I3_IGNORE *temp;

            I3CREATE(temp, I3_IGNORE, 1);

            temp->name = i3fread_line(fp);
            I3LINK(temp, FIRST_I3IGNORE(ch), LAST_I3IGNORE(ch), next, prev);
            fMatch = TRUE;
            break;
        }
        break;
    }
    return fMatch;
}

void i3_savechar(CHAR_DATA *ch, FILE *fp)
{
    I3_IGNORE *temp;

    if (IS_NPC(ch))
        return;

    fprintf(fp, "i3perm       %d\n", I3PERM(ch));
    fprintf(fp, "i3flags      %d\n", I3FLAG(ch));
    if (I3LISTEN(ch) && I3LISTEN(ch)[0] != '\0')
        fprintf(fp, "i3listen     %s\n", I3LISTEN(ch));
    else
        fprintf(fp, "i3listen     %s\n", "wiley");

    if (I3DENY(ch) && I3DENY(ch)[0] != '\0')
        fprintf(fp, "i3deny       %s\n", I3DENY(ch));
    for (temp = FIRST_I3IGNORE(ch); temp; temp = temp->next)
        fprintf(fp, "i3ignore     %s\n", temp->name);
}

/*******************************************
 * Network Startup and Shutdown functions. *
 *******************************************/

void I3_saveSpeakers(void)
{
    FILE *fp = NULL;
    int do_reset = 1;
    stringMap *node = NULL;

    if ((fp = fopen(I3_SPEAKER_FILE, "w")) == NULL)
    {
        log_info("%s", "Couldn't write to I3 speaker file.");
        return;
    }

    log_info("Saving I3 speaker list...");
    fprintf(fp, "%s", "#COUNT\n");
    fprintf(fp, "Count   %d\n", speaker_count);
    fprintf(fp, "%s", "End\n\n");

    do_reset = 1;
    do
    {
        node = stringmap_walk(speaker_db, do_reset);
        do_reset = 0;
        if (node && node->key)
        {
            fprintf(fp, "%s", "#SPEAKER\n");
            fprintf(fp, "Name   %s\n", node->key);
            fprintf(fp, "Color  %s\n", node->value ? (char *)node->value : "%^RESET%^");
            fprintf(fp, "%s", "End\n\n");
        }
    } while (node);

    fprintf(fp, "%s", "#END\n");
    I3FCLOSE(fp);
}

void I3_readSpeakerCount(FILE *fp)
{
    const char *word;
    bool fMatch;

    for (;;)
    {
        word = feof(fp) ? "End" : i3fread_word(fp);
        fMatch = FALSE;

        switch (word[0])
        {
        case '*':
            fMatch = TRUE;
            i3fread_to_eol(fp);
            break;

        case 'E':
            if (!strcasecmp(word, "End"))
            {
                return;
            }
            break;

        case 'C':
            KEY("Count", speaker_count, i3fread_number(fp));
            break;
        }
        if (!fMatch)
            log_error("i3_readSpeakerCount: no match: %s", word);
    }
}

void I3_readSpeaker(FILE *fp)
{
    const char *word;
    bool fMatch;
    char *key = NULL;
    char *value = NULL;

    for (;;)
    {
        word = feof(fp) ? "End" : i3fread_word(fp);
        fMatch = FALSE;

        switch (word[0])
        {
        case '*':
            fMatch = TRUE;
            i3fread_to_eol(fp);
            break;

        case 'E':
            if (!strcasecmp(word, "End"))
            {
                if (key && *key && value)
                {
                    const char *check = NULL;

                    if (!(check = (const char *)stringmap_find(speaker_db, key)))
                    {
                        stringmap_add(speaker_db, key, value, strlen(value) + 1);
                    }
                }
                return;
            }
            break;

        case 'C':
            KEY("Color", value, i3fread_line(fp));
            break;

        case 'N':
            KEY("Name", key, i3fread_line(fp));
            break;
        }
        if (!fMatch)
            log_error("i3_readSpeaker: no match: %s", word);
    }
}

void I3_loadSpeakers(void)
{
    FILE *fp = NULL;

    log_info("Initializing I3 speaker list...");
    stringmap_destroy(speaker_db); // Free anything present
    speaker_db = stringmap_init(); // Setup a new empty structure

    log_info("Loading I3 speaker list...");
    if ((fp = fopen(I3_SPEAKER_FILE, "r")) == NULL)
    {
        log_info("%s", "Couldn't open I3 speaker file.");
        return;
    }

    for (;;)
    {
        char letter;
        char *word;

        letter = i3fread_letter(fp);
        if (letter == '*')
        {
            i3fread_to_eol(fp);
            continue;
        }

        if (letter != '#')
        {
            log_error("I3_loadSpeakers: # not found.");
            break;
        }

        word = i3fread_word(fp);
        if (!strcasecmp(word, "SPEAKER"))
        {
            I3_readSpeaker(fp);
            continue;
        }
        else if (!strcasecmp(word, "COUNT"))
        {
            I3_readSpeakerCount(fp);
            continue;
        }
        else if (!strcasecmp(word, "END"))
        {
            break;
        }
        else
        {
            log_error("I3_loadSpeakers: bad section: %s.", word);
            continue;
        }
    }
    I3FCLOSE(fp);
}

void I3_loadPinkfishToANSI(void)
{
    log_info("Initializing Pinkfish to ANSI conversion table...");
    stringmap_destroy(pinkfish_to_ansi_db); // Free anything present
    pinkfish_to_ansi_db = stringmap_init(); // Setup a new empty structure

    log_info("Loading Pinkfish to ANSI conversion data...");

#undef PFto_ADD
#define PFto_ADD(key, value)                                                                                           \
    do                                                                                                                 \
    {                                                                                                                  \
        stringmap_add(pinkfish_to_ansi_db, (key), (void *)(value), strlen((char *)(value)));                           \
        pinkfish_to_ansi_count++;                                                                                      \
    } while (0)

    PFto_ADD("AliceBlue", "\033[1;37m");
    PFto_ADD("AntiqueWhite", "\033[1;37m");
    PFto_ADD("AntiqueWhite1", "\033[1;37m");
    PFto_ADD("AntiqueWhite2", "\033[1;37m");
    PFto_ADD("AntiqueWhite3", "\033[37m");
    PFto_ADD("AntiqueWhite4", "\033[1;30m");
    PFto_ADD("Aquamarine", "\033[1;36m");
    PFto_ADD("Aquamarine1", "\033[1;36m");
    PFto_ADD("Aquamarine2", "\033[1;36m");
    PFto_ADD("Aquamarine3", "\033[37m");
    PFto_ADD("Aquamarine4", "\033[1;30m");
    PFto_ADD("Azure", "\033[1;37m");
    PFto_ADD("Azure1", "\033[1;37m");
    PFto_ADD("Azure2", "\033[1;37m");
    PFto_ADD("Azure3", "\033[37m");
    PFto_ADD("Azure4", "\033[37m");
    PFto_ADD("B000", "\033[40m");
    PFto_ADD("B001", "\033[40m");
    PFto_ADD("B002", "\033[44m");
    PFto_ADD("B003", "\033[44m");
    PFto_ADD("B004", "\033[44m");
    PFto_ADD("B005", "\033[44m");
    PFto_ADD("B010", "\033[40m");
    PFto_ADD("B011", "\033[40m");
    PFto_ADD("B012", "\033[44m");
    PFto_ADD("B013", "\033[44m");
    PFto_ADD("B014", "\033[44m");
    PFto_ADD("B015", "\033[44m");
    PFto_ADD("B020", "\033[42m");
    PFto_ADD("B021", "\033[42m");
    PFto_ADD("B022", "\033[46m");
    PFto_ADD("B023", "\033[46m");
    PFto_ADD("B024", "\033[46m");
    PFto_ADD("B025", "\033[46m");
    PFto_ADD("B030", "\033[42m");
    PFto_ADD("B031", "\033[42m");
    PFto_ADD("B032", "\033[46m");
    PFto_ADD("B033", "\033[46m");
    PFto_ADD("B034", "\033[46m");
    PFto_ADD("B035", "\033[46m");
    PFto_ADD("B040", "\033[42m");
    PFto_ADD("B041", "\033[42m");
    PFto_ADD("B042", "\033[46m");
    PFto_ADD("B043", "\033[46m");
    PFto_ADD("B044", "\033[46m");
    PFto_ADD("B045", "\033[46m");
    PFto_ADD("B050", "\033[42m");
    PFto_ADD("B051", "\033[42m");
    PFto_ADD("B052", "\033[46m");
    PFto_ADD("B053", "\033[46m");
    PFto_ADD("B054", "\033[46m");
    PFto_ADD("B055", "\033[46m");
    PFto_ADD("B100", "\033[40m");
    PFto_ADD("B101", "\033[40m");
    PFto_ADD("B102", "\033[44m");
    PFto_ADD("B103", "\033[44m");
    PFto_ADD("B104", "\033[44m");
    PFto_ADD("B105", "\033[44m");
    PFto_ADD("B110", "\033[40m");
    PFto_ADD("B111", "\033[40m");
    PFto_ADD("B112", "\033[40m");
    PFto_ADD("B113", "\033[44m");
    PFto_ADD("B114", "\033[44m");
    PFto_ADD("B115", "\033[44m");
    PFto_ADD("B120", "\033[42m");
    PFto_ADD("B121", "\033[40m");
    PFto_ADD("B122", "\033[40m");
    PFto_ADD("B123", "\033[44m");
    PFto_ADD("B124", "\033[44m");
    PFto_ADD("B125", "\033[44m");
    PFto_ADD("B130", "\033[42m");
    PFto_ADD("B131", "\033[42m");
    PFto_ADD("B132", "\033[42m");
    PFto_ADD("B133", "\033[46m");
    PFto_ADD("B134", "\033[46m");
    PFto_ADD("B135", "\033[46m");
    PFto_ADD("B140", "\033[42m");
    PFto_ADD("B141", "\033[42m");
    PFto_ADD("B142", "\033[42m");
    PFto_ADD("B143", "\033[46m");
    PFto_ADD("B144", "\033[46m");
    PFto_ADD("B145", "\033[46m");
    PFto_ADD("B150", "\033[42m");
    PFto_ADD("B151", "\033[42m");
    PFto_ADD("B152", "\033[42m");
    PFto_ADD("B153", "\033[46m");
    PFto_ADD("B154", "\033[46m");
    PFto_ADD("B155", "\033[46m");
    PFto_ADD("B200", "\033[41m");
    PFto_ADD("B201", "\033[41m");
    PFto_ADD("B202", "\033[45m");
    PFto_ADD("B203", "\033[45m");
    PFto_ADD("B204", "\033[45m");
    PFto_ADD("B205", "\033[45m");
    PFto_ADD("B210", "\033[41m");
    PFto_ADD("B211", "\033[40m");
    PFto_ADD("B212", "\033[40m");
    PFto_ADD("B213", "\033[44m");
    PFto_ADD("B214", "\033[44m");
    PFto_ADD("B215", "\033[44m");
    PFto_ADD("B220", "\033[43m");
    PFto_ADD("B221", "\033[40m");
    PFto_ADD("B222", "\033[47m");
    PFto_ADD("B223", "\033[47m");
    PFto_ADD("B224", "\033[47m");
    PFto_ADD("B225", "\033[44m");
    PFto_ADD("B230", "\033[43m");
    PFto_ADD("B231", "\033[42m");
    PFto_ADD("B232", "\033[47m");
    PFto_ADD("B233", "\033[47m");
    PFto_ADD("B234", "\033[47m");
    PFto_ADD("B235", "\033[47m");
    PFto_ADD("B240", "\033[43m");
    PFto_ADD("B241", "\033[42m");
    PFto_ADD("B242", "\033[47m");
    PFto_ADD("B243", "\033[47m");
    PFto_ADD("B244", "\033[47m");
    PFto_ADD("B245", "\033[46m");
    PFto_ADD("B250", "\033[43m");
    PFto_ADD("B251", "\033[42m");
    PFto_ADD("B252", "\033[42m");
    PFto_ADD("B253", "\033[47m");
    PFto_ADD("B254", "\033[46m");
    PFto_ADD("B255", "\033[46m");
    PFto_ADD("B300", "\033[41m");
    PFto_ADD("B301", "\033[41m");
    PFto_ADD("B302", "\033[45m");
    PFto_ADD("B303", "\033[45m");
    PFto_ADD("B304", "\033[45m");
    PFto_ADD("B305", "\033[45m");
    PFto_ADD("B310", "\033[41m");
    PFto_ADD("B311", "\033[41m");
    PFto_ADD("B312", "\033[41m");
    PFto_ADD("B313", "\033[45m");
    PFto_ADD("B314", "\033[45m");
    PFto_ADD("B315", "\033[45m");
    PFto_ADD("B320", "\033[43m");
    PFto_ADD("B321", "\033[41m");
    PFto_ADD("B322", "\033[47m");
    PFto_ADD("B323", "\033[47m");
    PFto_ADD("B324", "\033[47m");
    PFto_ADD("B325", "\033[47m");
    PFto_ADD("B330", "\033[43m");
    PFto_ADD("B331", "\033[43m");
    PFto_ADD("B332", "\033[47m");
    PFto_ADD("B333", "\033[47m");
    PFto_ADD("B334", "\033[47m");
    PFto_ADD("B335", "\033[47m");
    PFto_ADD("B340", "\033[43m");
    PFto_ADD("B341", "\033[43m");
    PFto_ADD("B342", "\033[47m");
    PFto_ADD("B343", "\033[47m");
    PFto_ADD("B344", "\033[47m");
    PFto_ADD("B345", "\033[47m");
    PFto_ADD("B350", "\033[43m");
    PFto_ADD("B351", "\033[43m");
    PFto_ADD("B352", "\033[47m");
    PFto_ADD("B353", "\033[47m");
    PFto_ADD("B354", "\033[47m");
    PFto_ADD("B355", "\033[47m");
    PFto_ADD("B400", "\033[41m");
    PFto_ADD("B401", "\033[41m");
    PFto_ADD("B402", "\033[45m");
    PFto_ADD("B403", "\033[45m");
    PFto_ADD("B404", "\033[45m");
    PFto_ADD("B405", "\033[45m");
    PFto_ADD("B410", "\033[41m");
    PFto_ADD("B411", "\033[41m");
    PFto_ADD("B412", "\033[41m");
    PFto_ADD("B413", "\033[45m");
    PFto_ADD("B414", "\033[45m");
    PFto_ADD("B415", "\033[45m");
    PFto_ADD("B420", "\033[43m");
    PFto_ADD("B421", "\033[41m");
    PFto_ADD("B422", "\033[47m");
    PFto_ADD("B423", "\033[47m");
    PFto_ADD("B424", "\033[47m");
    PFto_ADD("B425", "\033[45m");
    PFto_ADD("B430", "\033[43m");
    PFto_ADD("B431", "\033[43m");
    PFto_ADD("B432", "\033[47m");
    PFto_ADD("B433", "\033[47m");
    PFto_ADD("B434", "\033[47m");
    PFto_ADD("B435", "\033[47m");
    PFto_ADD("B440", "\033[43m");
    PFto_ADD("B441", "\033[43m");
    PFto_ADD("B442", "\033[47m");
    PFto_ADD("B443", "\033[47m");
    PFto_ADD("B444", "\033[47m");
    PFto_ADD("B445", "\033[47m");
    PFto_ADD("B450", "\033[43m");
    PFto_ADD("B451", "\033[43m");
    PFto_ADD("B452", "\033[43m");
    PFto_ADD("B453", "\033[47m");
    PFto_ADD("B454", "\033[47m");
    PFto_ADD("B455", "\033[47m");
    PFto_ADD("B500", "\033[41m");
    PFto_ADD("B501", "\033[41m");
    PFto_ADD("B502", "\033[45m");
    PFto_ADD("B503", "\033[45m");
    PFto_ADD("B504", "\033[45m");
    PFto_ADD("B505", "\033[45m");
    PFto_ADD("B510", "\033[41m");
    PFto_ADD("B511", "\033[41m");
    PFto_ADD("B512", "\033[41m");
    PFto_ADD("B513", "\033[45m");
    PFto_ADD("B514", "\033[45m");
    PFto_ADD("B515", "\033[45m");
    PFto_ADD("B520", "\033[43m");
    PFto_ADD("B521", "\033[41m");
    PFto_ADD("B522", "\033[41m");
    PFto_ADD("B523", "\033[47m");
    PFto_ADD("B524", "\033[45m");
    PFto_ADD("B525", "\033[45m");
    PFto_ADD("B530", "\033[43m");
    PFto_ADD("B531", "\033[43m");
    PFto_ADD("B532", "\033[47m");
    PFto_ADD("B533", "\033[47m");
    PFto_ADD("B534", "\033[47m");
    PFto_ADD("B535", "\033[47m");
    PFto_ADD("B540", "\033[43m");
    PFto_ADD("B541", "\033[43m");
    PFto_ADD("B542", "\033[43m");
    PFto_ADD("B543", "\033[47m");
    PFto_ADD("B544", "\033[47m");
    PFto_ADD("B545", "\033[47m");
    PFto_ADD("B550", "\033[43m");
    PFto_ADD("B551", "\033[43m");
    PFto_ADD("B552", "\033[43m");
    PFto_ADD("B553", "\033[47m");
    PFto_ADD("B554", "\033[47m");
    PFto_ADD("B555", "\033[47m");
    PFto_ADD("BG00", "\033[40m");
    PFto_ADD("BG01", "\033[40m");
    PFto_ADD("BG02", "\033[40m");
    PFto_ADD("BG03", "\033[40m");
    PFto_ADD("BG04", "\033[40m");
    PFto_ADD("BG05", "\033[40m");
    PFto_ADD("BG06", "\033[40m");
    PFto_ADD("BG07", "\033[40m");
    PFto_ADD("BG08", "\033[40m");
    PFto_ADD("BG09", "\033[40m");
    PFto_ADD("BG10", "\033[40m");
    PFto_ADD("BG11", "\033[40m");
    PFto_ADD("BG12", "\033[40m");
    PFto_ADD("BG13", "\033[40m");
    PFto_ADD("BG14", "\033[47m");
    PFto_ADD("BG15", "\033[47m");
    PFto_ADD("BG16", "\033[47m");
    PFto_ADD("BG17", "\033[47m");
    PFto_ADD("BG18", "\033[47m");
    PFto_ADD("BG19", "\033[47m");
    PFto_ADD("BG20", "\033[47m");
    PFto_ADD("BG21", "\033[47m");
    PFto_ADD("BG22", "\033[47m");
    PFto_ADD("BG23", "\033[47m");
    PFto_ADD("BG24", "\033[47m");
    PFto_ADD("BG25", "\033[47m");
    PFto_ADD("BLACK", "\033[30m");
    PFto_ADD("BLUE", "\033[34m");
    PFto_ADD("BOLD", "\033[1m");
    PFto_ADD("B_AliceBlue", "\033[47m");
    PFto_ADD("B_AntiqueWhite", "\033[47m");
    PFto_ADD("B_AntiqueWhite1", "\033[47m");
    PFto_ADD("B_AntiqueWhite2", "\033[47m");
    PFto_ADD("B_AntiqueWhite3", "\033[47m");
    PFto_ADD("B_AntiqueWhite4", "\033[40m");
    PFto_ADD("B_Aquamarine", "\033[46m");
    PFto_ADD("B_Aquamarine1", "\033[46m");
    PFto_ADD("B_Aquamarine2", "\033[46m");
    PFto_ADD("B_Aquamarine3", "\033[47m");
    PFto_ADD("B_Aquamarine4", "\033[40m");
    PFto_ADD("B_Azure", "\033[47m");
    PFto_ADD("B_Azure1", "\033[47m");
    PFto_ADD("B_Azure2", "\033[47m");
    PFto_ADD("B_Azure3", "\033[47m");
    PFto_ADD("B_Azure4", "\033[47m");
    PFto_ADD("B_BLACK", "\033[40m");
    PFto_ADD("B_BLUE", "\033[44m");
    PFto_ADD("B_Beige", "\033[47m");
    PFto_ADD("B_Bisque", "\033[47m");
    PFto_ADD("B_Bisque1", "\033[47m");
    PFto_ADD("B_Bisque2", "\033[47m");
    PFto_ADD("B_Bisque3", "\033[47m");
    PFto_ADD("B_Bisque4", "\033[40m");
    PFto_ADD("B_Black", "\033[40m");
    PFto_ADD("B_BlanchedAlmond", "\033[47m");
    PFto_ADD("B_Blue", "\033[44m");
    PFto_ADD("B_Blue1", "\033[44m");
    PFto_ADD("B_Blue2", "\033[44m");
    PFto_ADD("B_Blue3", "\033[44m");
    PFto_ADD("B_Blue4", "\033[44m");
    PFto_ADD("B_BlueViolet", "\033[44m");
    PFto_ADD("B_Brown", "\033[41m");
    PFto_ADD("B_Brown1", "\033[41m");
    PFto_ADD("B_Brown2", "\033[41m");
    PFto_ADD("B_Brown3", "\033[41m");
    PFto_ADD("B_Brown4", "\033[41m");
    PFto_ADD("B_Burlywood", "\033[47m");
    PFto_ADD("B_Burlywood1", "\033[47m");
    PFto_ADD("B_Burlywood2", "\033[47m");
    PFto_ADD("B_Burlywood3", "\033[47m");
    PFto_ADD("B_Burlywood4", "\033[40m");
    PFto_ADD("B_CYAN", "\033[46m");
    PFto_ADD("B_CadetBlue", "\033[47m");
    PFto_ADD("B_CadetBlue1", "\033[46m");
    PFto_ADD("B_CadetBlue2", "\033[46m");
    PFto_ADD("B_CadetBlue3", "\033[47m");
    PFto_ADD("B_CadetBlue4", "\033[40m");
    PFto_ADD("B_Chartreuse", "\033[43m");
    PFto_ADD("B_Chartreuse1", "\033[43m");
    PFto_ADD("B_Chartreuse2", "\033[43m");
    PFto_ADD("B_Chartreuse3", "\033[43m");
    PFto_ADD("B_Chartreuse4", "\033[42m");
    PFto_ADD("B_Chocolate", "\033[41m");
    PFto_ADD("B_Chocolate1", "\033[41m");
    PFto_ADD("B_Chocolate2", "\033[41m");
    PFto_ADD("B_Chocolate3", "\033[41m");
    PFto_ADD("B_Chocolate4", "\033[41m");
    PFto_ADD("B_Coral", "\033[41m");
    PFto_ADD("B_Coral1", "\033[41m");
    PFto_ADD("B_Coral2", "\033[41m");
    PFto_ADD("B_Coral3", "\033[41m");
    PFto_ADD("B_Coral4", "\033[40m");
    PFto_ADD("B_CornflowerBlue", "\033[44m");
    PFto_ADD("B_Cornsilk", "\033[47m");
    PFto_ADD("B_Cornsilk1", "\033[47m");
    PFto_ADD("B_Cornsilk2", "\033[47m");
    PFto_ADD("B_Cornsilk3", "\033[47m");
    PFto_ADD("B_Cornsilk4", "\033[40m");
    PFto_ADD("B_Cyan", "\033[46m");
    PFto_ADD("B_Cyan1", "\033[46m");
    PFto_ADD("B_Cyan2", "\033[46m");
    PFto_ADD("B_Cyan3", "\033[46m");
    PFto_ADD("B_Cyan4", "\033[46m");
    PFto_ADD("B_DARKGREY", "\033[40m");
    PFto_ADD("B_DarkBlue", "\033[44m");
    PFto_ADD("B_DarkCyan", "\033[46m");
    PFto_ADD("B_DarkGoldenrod", "\033[43m");
    PFto_ADD("B_DarkGoldenrod1", "\033[43m");
    PFto_ADD("B_DarkGoldenrod2", "\033[43m");
    PFto_ADD("B_DarkGoldenrod3", "\033[43m");
    PFto_ADD("B_DarkGoldenrod4", "\033[40m");
    PFto_ADD("B_DarkGray", "\033[47m");
    PFto_ADD("B_DarkGreen", "\033[42m");
    PFto_ADD("B_DarkGrey", "\033[47m");
    PFto_ADD("B_DarkKhaki", "\033[47m");
    PFto_ADD("B_DarkMagenta", "\033[45m");
    PFto_ADD("B_DarkOliveGreen", "\033[40m");
    PFto_ADD("B_DarkOliveGreen1", "\033[43m");
    PFto_ADD("B_DarkOliveGreen2", "\033[43m");
    PFto_ADD("B_DarkOliveGreen3", "\033[42m");
    PFto_ADD("B_DarkOliveGreen4", "\033[40m");
    PFto_ADD("B_DarkOrange", "\033[43m");
    PFto_ADD("B_DarkOrange1", "\033[43m");
    PFto_ADD("B_DarkOrange2", "\033[43m");
    PFto_ADD("B_DarkOrange3", "\033[43m");
    PFto_ADD("B_DarkOrange4", "\033[41m");
    PFto_ADD("B_DarkOrchid", "\033[45m");
    PFto_ADD("B_DarkOrchid1", "\033[45m");
    PFto_ADD("B_DarkOrchid2", "\033[45m");
    PFto_ADD("B_DarkOrchid3", "\033[45m");
    PFto_ADD("B_DarkOrchid4", "\033[40m");
    PFto_ADD("B_DarkRed", "\033[41m");
    PFto_ADD("B_DarkSalmon", "\033[41m");
    PFto_ADD("B_DarkSeaGreen", "\033[47m");
    PFto_ADD("B_DarkSeaGreen1", "\033[47m");
    PFto_ADD("B_DarkSeaGreen2", "\033[47m");
    PFto_ADD("B_DarkSeaGreen3", "\033[47m");
    PFto_ADD("B_DarkSeaGreen4", "\033[40m");
    PFto_ADD("B_DarkSlateBlue", "\033[40m");
    PFto_ADD("B_DarkSlateGray", "\033[40m");
    PFto_ADD("B_DarkSlateGray1", "\033[46m");
    PFto_ADD("B_DarkSlateGray2", "\033[46m");
    PFto_ADD("B_DarkSlateGray3", "\033[47m");
    PFto_ADD("B_DarkSlateGray4", "\033[40m");
    PFto_ADD("B_DarkSlateGrey", "\033[40m");
    PFto_ADD("B_DarkTurquoise", "\033[46m");
    PFto_ADD("B_DarkViolet", "\033[45m");
    PFto_ADD("B_DeepPink", "\033[45m");
    PFto_ADD("B_DeepPink1", "\033[45m");
    PFto_ADD("B_DeepPink2", "\033[45m");
    PFto_ADD("B_DeepPink3", "\033[45m");
    PFto_ADD("B_DeepPink4", "\033[40m");
    PFto_ADD("B_DeepSkyBlue", "\033[46m");
    PFto_ADD("B_DeepSkyBlue1", "\033[46m");
    PFto_ADD("B_DeepSkyBlue2", "\033[46m");
    PFto_ADD("B_DeepSkyBlue3", "\033[46m");
    PFto_ADD("B_DeepSkyBlue4", "\033[46m");
    PFto_ADD("B_DimGray", "\033[40m");
    PFto_ADD("B_DimGrey", "\033[40m");
    PFto_ADD("B_DodgerBlue", "\033[44m");
    PFto_ADD("B_DodgerBlue1", "\033[44m");
    PFto_ADD("B_DodgerBlue2", "\033[44m");
    PFto_ADD("B_DodgerBlue3", "\033[46m");
    PFto_ADD("B_DodgerBlue4", "\033[40m");
    PFto_ADD("B_Firebrick", "\033[41m");
    PFto_ADD("B_Firebrick1", "\033[41m");
    PFto_ADD("B_Firebrick2", "\033[41m");
    PFto_ADD("B_Firebrick3", "\033[41m");
    PFto_ADD("B_Firebrick4", "\033[41m");
    PFto_ADD("B_FloralWhite", "\033[47m");
    PFto_ADD("B_ForestGreen", "\033[42m");
    PFto_ADD("B_GREEN", "\033[42m");
    PFto_ADD("B_GREY", "\033[47m");
    PFto_ADD("B_Gainsboro", "\033[47m");
    PFto_ADD("B_GhostWhite", "\033[47m");
    PFto_ADD("B_Gold", "\033[43m");
    PFto_ADD("B_Gold1", "\033[43m");
    PFto_ADD("B_Gold2", "\033[43m");
    PFto_ADD("B_Gold3", "\033[43m");
    PFto_ADD("B_Gold4", "\033[43m");
    PFto_ADD("B_Goldenrod", "\033[43m");
    PFto_ADD("B_Goldenrod1", "\033[43m");
    PFto_ADD("B_Goldenrod2", "\033[43m");
    PFto_ADD("B_Goldenrod3", "\033[43m");
    PFto_ADD("B_Goldenrod4", "\033[40m");
    PFto_ADD("B_Gray", "\033[47m");
    PFto_ADD("B_Gray0", "\033[40m");
    PFto_ADD("B_Gray1", "\033[40m");
    PFto_ADD("B_Gray10", "\033[40m");
    PFto_ADD("B_Gray100", "\033[47m");
    PFto_ADD("B_Gray11", "\033[40m");
    PFto_ADD("B_Gray12", "\033[40m");
    PFto_ADD("B_Gray13", "\033[40m");
    PFto_ADD("B_Gray14", "\033[40m");
    PFto_ADD("B_Gray15", "\033[40m");
    PFto_ADD("B_Gray16", "\033[40m");
    PFto_ADD("B_Gray17", "\033[40m");
    PFto_ADD("B_Gray18", "\033[40m");
    PFto_ADD("B_Gray19", "\033[40m");
    PFto_ADD("B_Gray2", "\033[40m");
    PFto_ADD("B_Gray20", "\033[40m");
    PFto_ADD("B_Gray21", "\033[40m");
    PFto_ADD("B_Gray22", "\033[40m");
    PFto_ADD("B_Gray23", "\033[40m");
    PFto_ADD("B_Gray24", "\033[40m");
    PFto_ADD("B_Gray25", "\033[40m");
    PFto_ADD("B_Gray26", "\033[40m");
    PFto_ADD("B_Gray27", "\033[40m");
    PFto_ADD("B_Gray28", "\033[40m");
    PFto_ADD("B_Gray29", "\033[40m");
    PFto_ADD("B_Gray3", "\033[40m");
    PFto_ADD("B_Gray30", "\033[40m");
    PFto_ADD("B_Gray31", "\033[40m");
    PFto_ADD("B_Gray32", "\033[40m");
    PFto_ADD("B_Gray33", "\033[40m");
    PFto_ADD("B_Gray34", "\033[40m");
    PFto_ADD("B_Gray35", "\033[40m");
    PFto_ADD("B_Gray36", "\033[40m");
    PFto_ADD("B_Gray37", "\033[40m");
    PFto_ADD("B_Gray38", "\033[40m");
    PFto_ADD("B_Gray39", "\033[40m");
    PFto_ADD("B_Gray4", "\033[40m");
    PFto_ADD("B_Gray40", "\033[40m");
    PFto_ADD("B_Gray41", "\033[40m");
    PFto_ADD("B_Gray42", "\033[40m");
    PFto_ADD("B_Gray43", "\033[40m");
    PFto_ADD("B_Gray44", "\033[40m");
    PFto_ADD("B_Gray45", "\033[40m");
    PFto_ADD("B_Gray46", "\033[40m");
    PFto_ADD("B_Gray47", "\033[40m");
    PFto_ADD("B_Gray48", "\033[40m");
    PFto_ADD("B_Gray49", "\033[40m");
    PFto_ADD("B_Gray5", "\033[40m");
    PFto_ADD("B_Gray50", "\033[40m");
    PFto_ADD("B_Gray51", "\033[40m");
    PFto_ADD("B_Gray52", "\033[40m");
    PFto_ADD("B_Gray53", "\033[40m");
    PFto_ADD("B_Gray54", "\033[47m");
    PFto_ADD("B_Gray55", "\033[47m");
    PFto_ADD("B_Gray56", "\033[47m");
    PFto_ADD("B_Gray57", "\033[47m");
    PFto_ADD("B_Gray58", "\033[47m");
    PFto_ADD("B_Gray59", "\033[47m");
    PFto_ADD("B_Gray6", "\033[40m");
    PFto_ADD("B_Gray60", "\033[47m");
    PFto_ADD("B_Gray61", "\033[47m");
    PFto_ADD("B_Gray62", "\033[47m");
    PFto_ADD("B_Gray63", "\033[47m");
    PFto_ADD("B_Gray64", "\033[47m");
    PFto_ADD("B_Gray65", "\033[47m");
    PFto_ADD("B_Gray66", "\033[47m");
    PFto_ADD("B_Gray67", "\033[47m");
    PFto_ADD("B_Gray68", "\033[47m");
    PFto_ADD("B_Gray69", "\033[47m");
    PFto_ADD("B_Gray7", "\033[40m");
    PFto_ADD("B_Gray70", "\033[47m");
    PFto_ADD("B_Gray71", "\033[47m");
    PFto_ADD("B_Gray72", "\033[47m");
    PFto_ADD("B_Gray73", "\033[47m");
    PFto_ADD("B_Gray74", "\033[47m");
    PFto_ADD("B_Gray75", "\033[47m");
    PFto_ADD("B_Gray76", "\033[47m");
    PFto_ADD("B_Gray77", "\033[47m");
    PFto_ADD("B_Gray78", "\033[47m");
    PFto_ADD("B_Gray79", "\033[47m");
    PFto_ADD("B_Gray8", "\033[40m");
    PFto_ADD("B_Gray80", "\033[47m");
    PFto_ADD("B_Gray81", "\033[47m");
    PFto_ADD("B_Gray82", "\033[47m");
    PFto_ADD("B_Gray83", "\033[47m");
    PFto_ADD("B_Gray84", "\033[47m");
    PFto_ADD("B_Gray85", "\033[47m");
    PFto_ADD("B_Gray86", "\033[47m");
    PFto_ADD("B_Gray87", "\033[47m");
    PFto_ADD("B_Gray88", "\033[47m");
    PFto_ADD("B_Gray89", "\033[47m");
    PFto_ADD("B_Gray9", "\033[40m");
    PFto_ADD("B_Gray90", "\033[47m");
    PFto_ADD("B_Gray91", "\033[47m");
    PFto_ADD("B_Gray92", "\033[47m");
    PFto_ADD("B_Gray93", "\033[47m");
    PFto_ADD("B_Gray94", "\033[47m");
    PFto_ADD("B_Gray95", "\033[47m");
    PFto_ADD("B_Gray96", "\033[47m");
    PFto_ADD("B_Gray97", "\033[47m");
    PFto_ADD("B_Gray98", "\033[47m");
    PFto_ADD("B_Gray99", "\033[47m");
    PFto_ADD("B_Green", "\033[42m");
    PFto_ADD("B_Green1", "\033[42m");
    PFto_ADD("B_Green2", "\033[42m");
    PFto_ADD("B_Green3", "\033[42m");
    PFto_ADD("B_Green4", "\033[42m");
    PFto_ADD("B_GreenYellow", "\033[43m");
    PFto_ADD("B_Grey", "\033[47m");
    PFto_ADD("B_Grey0", "\033[40m");
    PFto_ADD("B_Grey1", "\033[40m");
    PFto_ADD("B_Grey10", "\033[40m");
    PFto_ADD("B_Grey100", "\033[47m");
    PFto_ADD("B_Grey11", "\033[40m");
    PFto_ADD("B_Grey12", "\033[40m");
    PFto_ADD("B_Grey13", "\033[40m");
    PFto_ADD("B_Grey14", "\033[40m");
    PFto_ADD("B_Grey15", "\033[40m");
    PFto_ADD("B_Grey16", "\033[40m");
    PFto_ADD("B_Grey17", "\033[40m");
    PFto_ADD("B_Grey18", "\033[40m");
    PFto_ADD("B_Grey19", "\033[40m");
    PFto_ADD("B_Grey2", "\033[40m");
    PFto_ADD("B_Grey20", "\033[40m");
    PFto_ADD("B_Grey21", "\033[40m");
    PFto_ADD("B_Grey22", "\033[40m");
    PFto_ADD("B_Grey23", "\033[40m");
    PFto_ADD("B_Grey24", "\033[40m");
    PFto_ADD("B_Grey25", "\033[40m");
    PFto_ADD("B_Grey26", "\033[40m");
    PFto_ADD("B_Grey27", "\033[40m");
    PFto_ADD("B_Grey28", "\033[40m");
    PFto_ADD("B_Grey29", "\033[40m");
    PFto_ADD("B_Grey3", "\033[40m");
    PFto_ADD("B_Grey30", "\033[40m");
    PFto_ADD("B_Grey31", "\033[40m");
    PFto_ADD("B_Grey32", "\033[40m");
    PFto_ADD("B_Grey33", "\033[40m");
    PFto_ADD("B_Grey34", "\033[40m");
    PFto_ADD("B_Grey35", "\033[40m");
    PFto_ADD("B_Grey36", "\033[40m");
    PFto_ADD("B_Grey37", "\033[40m");
    PFto_ADD("B_Grey38", "\033[40m");
    PFto_ADD("B_Grey39", "\033[40m");
    PFto_ADD("B_Grey4", "\033[40m");
    PFto_ADD("B_Grey40", "\033[40m");
    PFto_ADD("B_Grey41", "\033[40m");
    PFto_ADD("B_Grey42", "\033[40m");
    PFto_ADD("B_Grey43", "\033[40m");
    PFto_ADD("B_Grey44", "\033[40m");
    PFto_ADD("B_Grey45", "\033[40m");
    PFto_ADD("B_Grey46", "\033[40m");
    PFto_ADD("B_Grey47", "\033[40m");
    PFto_ADD("B_Grey48", "\033[40m");
    PFto_ADD("B_Grey49", "\033[40m");
    PFto_ADD("B_Grey5", "\033[40m");
    PFto_ADD("B_Grey50", "\033[40m");
    PFto_ADD("B_Grey51", "\033[40m");
    PFto_ADD("B_Grey52", "\033[40m");
    PFto_ADD("B_Grey53", "\033[40m");
    PFto_ADD("B_Grey54", "\033[47m");
    PFto_ADD("B_Grey55", "\033[47m");
    PFto_ADD("B_Grey56", "\033[47m");
    PFto_ADD("B_Grey57", "\033[47m");
    PFto_ADD("B_Grey58", "\033[47m");
    PFto_ADD("B_Grey59", "\033[47m");
    PFto_ADD("B_Grey6", "\033[40m");
    PFto_ADD("B_Grey60", "\033[47m");
    PFto_ADD("B_Grey61", "\033[47m");
    PFto_ADD("B_Grey62", "\033[47m");
    PFto_ADD("B_Grey63", "\033[47m");
    PFto_ADD("B_Grey64", "\033[47m");
    PFto_ADD("B_Grey65", "\033[47m");
    PFto_ADD("B_Grey66", "\033[47m");
    PFto_ADD("B_Grey67", "\033[47m");
    PFto_ADD("B_Grey68", "\033[47m");
    PFto_ADD("B_Grey69", "\033[47m");
    PFto_ADD("B_Grey7", "\033[40m");
    PFto_ADD("B_Grey70", "\033[47m");
    PFto_ADD("B_Grey71", "\033[47m");
    PFto_ADD("B_Grey72", "\033[47m");
    PFto_ADD("B_Grey73", "\033[47m");
    PFto_ADD("B_Grey74", "\033[47m");
    PFto_ADD("B_Grey75", "\033[47m");
    PFto_ADD("B_Grey76", "\033[47m");
    PFto_ADD("B_Grey77", "\033[47m");
    PFto_ADD("B_Grey78", "\033[47m");
    PFto_ADD("B_Grey79", "\033[47m");
    PFto_ADD("B_Grey8", "\033[40m");
    PFto_ADD("B_Grey80", "\033[47m");
    PFto_ADD("B_Grey81", "\033[47m");
    PFto_ADD("B_Grey82", "\033[47m");
    PFto_ADD("B_Grey83", "\033[47m");
    PFto_ADD("B_Grey84", "\033[47m");
    PFto_ADD("B_Grey85", "\033[47m");
    PFto_ADD("B_Grey86", "\033[47m");
    PFto_ADD("B_Grey87", "\033[47m");
    PFto_ADD("B_Grey88", "\033[47m");
    PFto_ADD("B_Grey89", "\033[47m");
    PFto_ADD("B_Grey9", "\033[40m");
    PFto_ADD("B_Grey90", "\033[47m");
    PFto_ADD("B_Grey91", "\033[47m");
    PFto_ADD("B_Grey92", "\033[47m");
    PFto_ADD("B_Grey93", "\033[47m");
    PFto_ADD("B_Grey94", "\033[47m");
    PFto_ADD("B_Grey95", "\033[47m");
    PFto_ADD("B_Grey96", "\033[47m");
    PFto_ADD("B_Grey97", "\033[47m");
    PFto_ADD("B_Grey98", "\033[47m");
    PFto_ADD("B_Grey99", "\033[47m");
    PFto_ADD("B_Honeydew", "\033[47m");
    PFto_ADD("B_Honeydew1", "\033[47m");
    PFto_ADD("B_Honeydew2", "\033[47m");
    PFto_ADD("B_Honeydew3", "\033[47m");
    PFto_ADD("B_Honeydew4", "\033[40m");
    PFto_ADD("B_HotPink", "\033[45m");
    PFto_ADD("B_HotPink1", "\033[45m");
    PFto_ADD("B_HotPink2", "\033[41m");
    PFto_ADD("B_HotPink3", "\033[41m");
    PFto_ADD("B_HotPink4", "\033[40m");
    PFto_ADD("B_IndianRed", "\033[41m");
    PFto_ADD("B_IndianRed1", "\033[41m");
    PFto_ADD("B_IndianRed2", "\033[41m");
    PFto_ADD("B_IndianRed3", "\033[41m");
    PFto_ADD("B_IndianRed4", "\033[40m");
    PFto_ADD("B_Ivory", "\033[47m");
    PFto_ADD("B_Ivory1", "\033[47m");
    PFto_ADD("B_Ivory2", "\033[47m");
    PFto_ADD("B_Ivory3", "\033[47m");
    PFto_ADD("B_Ivory4", "\033[47m");
    PFto_ADD("B_Khaki", "\033[43m");
    PFto_ADD("B_Khaki1", "\033[43m");
    PFto_ADD("B_Khaki2", "\033[43m");
    PFto_ADD("B_Khaki3", "\033[47m");
    PFto_ADD("B_Khaki4", "\033[40m");
    PFto_ADD("B_LIGHTBLUE", "\033[44m");
    PFto_ADD("B_LIGHTCYAN", "\033[46m");
    PFto_ADD("B_LIGHTGREEN", "\033[42m");
    PFto_ADD("B_LIGHTRED", "\033[41m");
    PFto_ADD("B_Lavender", "\033[47m");
    PFto_ADD("B_LavenderBlush", "\033[47m");
    PFto_ADD("B_LavenderBlush1", "\033[47m");
    PFto_ADD("B_LavenderBlush2", "\033[47m");
    PFto_ADD("B_LavenderBlush3", "\033[47m");
    PFto_ADD("B_LavenderBlush4", "\033[40m");
    PFto_ADD("B_LawnGreen", "\033[43m");
    PFto_ADD("B_LemonChiffon", "\033[47m");
    PFto_ADD("B_LemonChiffon1", "\033[47m");
    PFto_ADD("B_LemonChiffon2", "\033[47m");
    PFto_ADD("B_LemonChiffon3", "\033[47m");
    PFto_ADD("B_LemonChiffon4", "\033[40m");
    PFto_ADD("B_LightBlue", "\033[47m");
    PFto_ADD("B_LightBlue1", "\033[47m");
    PFto_ADD("B_LightBlue2", "\033[47m");
    PFto_ADD("B_LightBlue3", "\033[47m");
    PFto_ADD("B_LightBlue4", "\033[40m");
    PFto_ADD("B_LightCoral", "\033[41m");
    PFto_ADD("B_LightCyan", "\033[47m");
    PFto_ADD("B_LightCyan1", "\033[47m");
    PFto_ADD("B_LightCyan2", "\033[47m");
    PFto_ADD("B_LightCyan3", "\033[47m");
    PFto_ADD("B_LightCyan4", "\033[40m");
    PFto_ADD("B_LightGoldenrod", "\033[43m");
    PFto_ADD("B_LightGoldenrod1", "\033[43m");
    PFto_ADD("B_LightGoldenrod2", "\033[43m");
    PFto_ADD("B_LightGoldenrod3", "\033[47m");
    PFto_ADD("B_LightGoldenrod4", "\033[40m");
    PFto_ADD("B_LightGoldenrodYellow", "\033[47m");
    PFto_ADD("B_LightGray", "\033[47m");
    PFto_ADD("B_LightGreen", "\033[47m");
    PFto_ADD("B_LightGrey", "\033[47m");
    PFto_ADD("B_LightPink", "\033[47m");
    PFto_ADD("B_LightPink1", "\033[47m");
    PFto_ADD("B_LightPink2", "\033[47m");
    PFto_ADD("B_LightPink3", "\033[47m");
    PFto_ADD("B_LightPink4", "\033[40m");
    PFto_ADD("B_LightSalmon", "\033[41m");
    PFto_ADD("B_LightSalmon1", "\033[41m");
    PFto_ADD("B_LightSalmon2", "\033[41m");
    PFto_ADD("B_LightSalmon3", "\033[41m");
    PFto_ADD("B_LightSalmon4", "\033[40m");
    PFto_ADD("B_LightSeaGreen", "\033[46m");
    PFto_ADD("B_LightSkyBlue", "\033[46m");
    PFto_ADD("B_LightSkyBlue1", "\033[47m");
    PFto_ADD("B_LightSkyBlue2", "\033[47m");
    PFto_ADD("B_LightSkyBlue3", "\033[47m");
    PFto_ADD("B_LightSkyBlue4", "\033[40m");
    PFto_ADD("B_LightSlateBlue", "\033[44m");
    PFto_ADD("B_LightSlateGray", "\033[47m");
    PFto_ADD("B_LightSlateGrey", "\033[47m");
    PFto_ADD("B_LightSteelBlue", "\033[47m");
    PFto_ADD("B_LightSteelBlue1", "\033[47m");
    PFto_ADD("B_LightSteelBlue2", "\033[47m");
    PFto_ADD("B_LightSteelBlue3", "\033[47m");
    PFto_ADD("B_LightSteelBlue4", "\033[40m");
    PFto_ADD("B_LightYellow", "\033[47m");
    PFto_ADD("B_LightYellow1", "\033[47m");
    PFto_ADD("B_LightYellow2", "\033[47m");
    PFto_ADD("B_LightYellow3", "\033[47m");
    PFto_ADD("B_LightYellow4", "\033[40m");
    PFto_ADD("B_LimeGreen", "\033[42m");
    PFto_ADD("B_Linen", "\033[47m");
    PFto_ADD("B_MAGENTA", "\033[45m");
    PFto_ADD("B_Magenta", "\033[45m");
    PFto_ADD("B_Magenta1", "\033[45m");
    PFto_ADD("B_Magenta2", "\033[45m");
    PFto_ADD("B_Magenta3", "\033[45m");
    PFto_ADD("B_Magenta4", "\033[45m");
    PFto_ADD("B_Maroon", "\033[41m");
    PFto_ADD("B_Maroon1", "\033[45m");
    PFto_ADD("B_Maroon2", "\033[45m");
    PFto_ADD("B_Maroon3", "\033[45m");
    PFto_ADD("B_Maroon4", "\033[40m");
    PFto_ADD("B_MediumAquamarine", "\033[47m");
    PFto_ADD("B_MediumBlue", "\033[44m");
    PFto_ADD("B_MediumOrchid", "\033[45m");
    PFto_ADD("B_MediumOrchid1", "\033[45m");
    PFto_ADD("B_MediumOrchid2", "\033[45m");
    PFto_ADD("B_MediumOrchid3", "\033[45m");
    PFto_ADD("B_MediumOrchid4", "\033[40m");
    PFto_ADD("B_MediumPurple", "\033[44m");
    PFto_ADD("B_MediumPurple1", "\033[47m");
    PFto_ADD("B_MediumPurple2", "\033[44m");
    PFto_ADD("B_MediumPurple3", "\033[44m");
    PFto_ADD("B_MediumPurple4", "\033[40m");
    PFto_ADD("B_MediumSeaGreen", "\033[42m");
    PFto_ADD("B_MediumSlateBlue", "\033[44m");
    PFto_ADD("B_MediumSpringGreen", "\033[46m");
    PFto_ADD("B_MediumTurquoise", "\033[46m");
    PFto_ADD("B_MediumVioletRed", "\033[45m");
    PFto_ADD("B_MidnightBlue", "\033[44m");
    PFto_ADD("B_MintCream", "\033[47m");
    PFto_ADD("B_MistyRose", "\033[47m");
    PFto_ADD("B_MistyRose1", "\033[47m");
    PFto_ADD("B_MistyRose2", "\033[47m");
    PFto_ADD("B_MistyRose3", "\033[47m");
    PFto_ADD("B_MistyRose4", "\033[40m");
    PFto_ADD("B_Moccasin", "\033[47m");
    PFto_ADD("B_NavajoWhite", "\033[47m");
    PFto_ADD("B_NavajoWhite1", "\033[47m");
    PFto_ADD("B_NavajoWhite2", "\033[47m");
    PFto_ADD("B_NavajoWhite3", "\033[47m");
    PFto_ADD("B_NavajoWhite4", "\033[40m");
    PFto_ADD("B_Navy", "\033[44m");
    PFto_ADD("B_NavyBlue", "\033[44m");
    PFto_ADD("B_ORANGE", "\033[43m");
    PFto_ADD("B_OldLace", "\033[47m");
    PFto_ADD("B_OliveDrab", "\033[40m");
    PFto_ADD("B_OliveDrab1", "\033[43m");
    PFto_ADD("B_OliveDrab2", "\033[43m");
    PFto_ADD("B_OliveDrab3", "\033[43m");
    PFto_ADD("B_OliveDrab4", "\033[40m");
    PFto_ADD("B_Orange", "\033[43m");
    PFto_ADD("B_Orange1", "\033[43m");
    PFto_ADD("B_Orange2", "\033[43m");
    PFto_ADD("B_Orange3", "\033[43m");
    PFto_ADD("B_Orange4", "\033[40m");
    PFto_ADD("B_OrangeRed", "\033[41m");
    PFto_ADD("B_OrangeRed1", "\033[41m");
    PFto_ADD("B_OrangeRed2", "\033[41m");
    PFto_ADD("B_OrangeRed3", "\033[41m");
    PFto_ADD("B_OrangeRed4", "\033[41m");
    PFto_ADD("B_Orchid", "\033[45m");
    PFto_ADD("B_Orchid1", "\033[45m");
    PFto_ADD("B_Orchid2", "\033[45m");
    PFto_ADD("B_Orchid3", "\033[45m");
    PFto_ADD("B_Orchid4", "\033[40m");
    PFto_ADD("B_PINK", "\033[45m");
    PFto_ADD("B_PaleGoldenrod", "\033[47m");
    PFto_ADD("B_PaleGreen", "\033[47m");
    PFto_ADD("B_PaleGreen1", "\033[47m");
    PFto_ADD("B_PaleGreen2", "\033[47m");
    PFto_ADD("B_PaleGreen3", "\033[42m");
    PFto_ADD("B_PaleGreen4", "\033[40m");
    PFto_ADD("B_PaleTurquoise", "\033[47m");
    PFto_ADD("B_PaleTurquoise1", "\033[47m");
    PFto_ADD("B_PaleTurquoise2", "\033[47m");
    PFto_ADD("B_PaleTurquoise3", "\033[47m");
    PFto_ADD("B_PaleTurquoise4", "\033[40m");
    PFto_ADD("B_PaleVioletRed", "\033[41m");
    PFto_ADD("B_PaleVioletRed1", "\033[47m");
    PFto_ADD("B_PaleVioletRed2", "\033[41m");
    PFto_ADD("B_PaleVioletRed3", "\033[41m");
    PFto_ADD("B_PaleVioletRed4", "\033[40m");
    PFto_ADD("B_PapayaWhip", "\033[47m");
    PFto_ADD("B_PeachPuff", "\033[47m");
    PFto_ADD("B_PeachPuff1", "\033[47m");
    PFto_ADD("B_PeachPuff2", "\033[47m");
    PFto_ADD("B_PeachPuff3", "\033[47m");
    PFto_ADD("B_PeachPuff4", "\033[40m");
    PFto_ADD("B_Peru", "\033[41m");
    PFto_ADD("B_Pink", "\033[47m");
    PFto_ADD("B_Pink1", "\033[47m");
    PFto_ADD("B_Pink2", "\033[47m");
    PFto_ADD("B_Pink3", "\033[47m");
    PFto_ADD("B_Pink4", "\033[40m");
    PFto_ADD("B_Plum", "\033[47m");
    PFto_ADD("B_Plum1", "\033[47m");
    PFto_ADD("B_Plum2", "\033[47m");
    PFto_ADD("B_Plum3", "\033[47m");
    PFto_ADD("B_Plum4", "\033[40m");
    PFto_ADD("B_PowderBlue", "\033[47m");
    PFto_ADD("B_Purple", "\033[45m");
    PFto_ADD("B_Purple1", "\033[44m");
    PFto_ADD("B_Purple2", "\033[44m");
    PFto_ADD("B_Purple3", "\033[45m");
    PFto_ADD("B_Purple4", "\033[40m");
    PFto_ADD("B_RED", "\033[41m");
    PFto_ADD("B_Red", "\033[41m");
    PFto_ADD("B_Red1", "\033[41m");
    PFto_ADD("B_Red2", "\033[41m");
    PFto_ADD("B_Red3", "\033[41m");
    PFto_ADD("B_Red4", "\033[41m");
    PFto_ADD("B_RosyBrown", "\033[47m");
    PFto_ADD("B_RosyBrown1", "\033[47m");
    PFto_ADD("B_RosyBrown2", "\033[47m");
    PFto_ADD("B_RosyBrown3", "\033[47m");
    PFto_ADD("B_RosyBrown4", "\033[40m");
    PFto_ADD("B_RoyalBlue", "\033[44m");
    PFto_ADD("B_RoyalBlue1", "\033[44m");
    PFto_ADD("B_RoyalBlue2", "\033[44m");
    PFto_ADD("B_RoyalBlue3", "\033[44m");
    PFto_ADD("B_RoyalBlue4", "\033[40m");
    PFto_ADD("B_SaddleBrown", "\033[41m");
    PFto_ADD("B_Salmon", "\033[41m");
    PFto_ADD("B_Salmon1", "\033[41m");
    PFto_ADD("B_Salmon2", "\033[41m");
    PFto_ADD("B_Salmon3", "\033[41m");
    PFto_ADD("B_Salmon4", "\033[40m");
    PFto_ADD("B_SandyBrown", "\033[41m");
    PFto_ADD("B_SeaGreen", "\033[40m");
    PFto_ADD("B_SeaGreen1", "\033[42m");
    PFto_ADD("B_SeaGreen2", "\033[42m");
    PFto_ADD("B_SeaGreen3", "\033[42m");
    PFto_ADD("B_SeaGreen4", "\033[40m");
    PFto_ADD("B_Seashell", "\033[47m");
    PFto_ADD("B_Seashell1", "\033[47m");
    PFto_ADD("B_Seashell2", "\033[47m");
    PFto_ADD("B_Seashell3", "\033[47m");
    PFto_ADD("B_Seashell4", "\033[40m");
    PFto_ADD("B_Sienna", "\033[40m");
    PFto_ADD("B_Sienna1", "\033[41m");
    PFto_ADD("B_Sienna2", "\033[41m");
    PFto_ADD("B_Sienna3", "\033[41m");
    PFto_ADD("B_Sienna4", "\033[40m");
    PFto_ADD("B_SkyBlue", "\033[46m");
    PFto_ADD("B_SkyBlue1", "\033[46m");
    PFto_ADD("B_SkyBlue2", "\033[46m");
    PFto_ADD("B_SkyBlue3", "\033[47m");
    PFto_ADD("B_SkyBlue4", "\033[40m");
    PFto_ADD("B_SlateBlue", "\033[44m");
    PFto_ADD("B_SlateBlue1", "\033[44m");
    PFto_ADD("B_SlateBlue2", "\033[44m");
    PFto_ADD("B_SlateBlue3", "\033[44m");
    PFto_ADD("B_SlateBlue4", "\033[40m");
    PFto_ADD("B_SlateGray", "\033[40m");
    PFto_ADD("B_SlateGray1", "\033[47m");
    PFto_ADD("B_SlateGray2", "\033[47m");
    PFto_ADD("B_SlateGray3", "\033[47m");
    PFto_ADD("B_SlateGray4", "\033[40m");
    PFto_ADD("B_SlateGrey", "\033[40m");
    PFto_ADD("B_Snow", "\033[47m");
    PFto_ADD("B_Snow1", "\033[47m");
    PFto_ADD("B_Snow2", "\033[47m");
    PFto_ADD("B_Snow3", "\033[47m");
    PFto_ADD("B_Snow4", "\033[47m");
    PFto_ADD("B_SpringGreen", "\033[46m");
    PFto_ADD("B_SpringGreen1", "\033[46m");
    PFto_ADD("B_SpringGreen2", "\033[46m");
    PFto_ADD("B_SpringGreen3", "\033[46m");
    PFto_ADD("B_SpringGreen4", "\033[42m");
    PFto_ADD("B_SteelBlue", "\033[44m");
    PFto_ADD("B_SteelBlue1", "\033[46m");
    PFto_ADD("B_SteelBlue2", "\033[46m");
    PFto_ADD("B_SteelBlue3", "\033[44m");
    PFto_ADD("B_SteelBlue4", "\033[40m");
    PFto_ADD("B_Tan", "\033[47m");
    PFto_ADD("B_Tan1", "\033[41m");
    PFto_ADD("B_Tan2", "\033[41m");
    PFto_ADD("B_Tan3", "\033[41m");
    PFto_ADD("B_Tan4", "\033[40m");
    PFto_ADD("B_Thistle", "\033[47m");
    PFto_ADD("B_Thistle1", "\033[47m");
    PFto_ADD("B_Thistle2", "\033[47m");
    PFto_ADD("B_Thistle3", "\033[47m");
    PFto_ADD("B_Thistle4", "\033[40m");
    PFto_ADD("B_Tomato", "\033[41m");
    PFto_ADD("B_Tomato1", "\033[41m");
    PFto_ADD("B_Tomato2", "\033[41m");
    PFto_ADD("B_Tomato3", "\033[41m");
    PFto_ADD("B_Tomato4", "\033[40m");
    PFto_ADD("B_Turquoise", "\033[46m");
    PFto_ADD("B_Turquoise1", "\033[46m");
    PFto_ADD("B_Turquoise2", "\033[46m");
    PFto_ADD("B_Turquoise3", "\033[46m");
    PFto_ADD("B_Turquoise4", "\033[46m");
    PFto_ADD("B_Violet", "\033[45m");
    PFto_ADD("B_VioletRed", "\033[45m");
    PFto_ADD("B_VioletRed1", "\033[41m");
    PFto_ADD("B_VioletRed2", "\033[41m");
    PFto_ADD("B_VioletRed3", "\033[41m");
    PFto_ADD("B_VioletRed4", "\033[40m");
    PFto_ADD("B_WHITE", "\033[47m");
    PFto_ADD("B_Wheat", "\033[47m");
    PFto_ADD("B_Wheat1", "\033[47m");
    PFto_ADD("B_Wheat2", "\033[47m");
    PFto_ADD("B_Wheat3", "\033[47m");
    PFto_ADD("B_Wheat4", "\033[40m");
    PFto_ADD("B_White", "\033[47m");
    PFto_ADD("B_WhiteSmoke", "\033[47m");
    PFto_ADD("B_YELLOW", "\033[43m");
    PFto_ADD("B_Yellow", "\033[43m");
    PFto_ADD("B_Yellow1", "\033[43m");
    PFto_ADD("B_Yellow2", "\033[43m");
    PFto_ADD("B_Yellow3", "\033[43m");
    PFto_ADD("B_Yellow4", "\033[43m");
    PFto_ADD("B_YellowGreen", "\033[43m");
    PFto_ADD("Beige", "\033[1;37m");
    PFto_ADD("Bisque", "\033[1;37m");
    PFto_ADD("Bisque1", "\033[1;37m");
    PFto_ADD("Bisque2", "\033[37m");
    PFto_ADD("Bisque3", "\033[37m");
    PFto_ADD("Bisque4", "\033[1;30m");
    PFto_ADD("Black", "\033[30m");
    PFto_ADD("BlanchedAlmond", "\033[1;37m");
    PFto_ADD("Blue", "\033[34m");
    PFto_ADD("Blue1", "\033[34m");
    PFto_ADD("Blue2", "\033[34m");
    PFto_ADD("Blue3", "\033[34m");
    PFto_ADD("Blue4", "\033[34m");
    PFto_ADD("BlueViolet", "\033[1;34m");
    PFto_ADD("Brown", "\033[31m");
    PFto_ADD("Brown1", "\033[1;31m");
    PFto_ADD("Brown2", "\033[1;31m");
    PFto_ADD("Brown3", "\033[1;31m");
    PFto_ADD("Brown4", "\033[31m");
    PFto_ADD("Burlywood", "\033[37m");
    PFto_ADD("Burlywood1", "\033[37m");
    PFto_ADD("Burlywood2", "\033[37m");
    PFto_ADD("Burlywood3", "\033[37m");
    PFto_ADD("Burlywood4", "\033[1;30m");
    PFto_ADD("CLEARLINE", "\033[L\033[G");
    PFto_ADD("CURS_DOWN", "\033[B");
    PFto_ADD("CURS_LEFT", "\033[D");
    PFto_ADD("CURS_RIGHT", "\033[C");
    PFto_ADD("CURS_UP", "\033[A");
    PFto_ADD("CYAN", "\033[36m");
    PFto_ADD("CadetBlue", "\033[37m");
    PFto_ADD("CadetBlue1", "\033[1;36m");
    PFto_ADD("CadetBlue2", "\033[1;36m");
    PFto_ADD("CadetBlue3", "\033[37m");
    PFto_ADD("CadetBlue4", "\033[1;30m");
    PFto_ADD("Chartreuse", "\033[33m");
    PFto_ADD("Chartreuse1", "\033[33m");
    PFto_ADD("Chartreuse2", "\033[33m");
    PFto_ADD("Chartreuse3", "\033[33m");
    PFto_ADD("Chartreuse4", "\033[32m");
    PFto_ADD("Chocolate", "\033[1;31m");
    PFto_ADD("Chocolate1", "\033[1;31m");
    PFto_ADD("Chocolate2", "\033[1;31m");
    PFto_ADD("Chocolate3", "\033[1;31m");
    PFto_ADD("Chocolate4", "\033[31m");
    PFto_ADD("Coral", "\033[1;31m");
    PFto_ADD("Coral1", "\033[1;31m");
    PFto_ADD("Coral2", "\033[1;31m");
    PFto_ADD("Coral3", "\033[1;31m");
    PFto_ADD("Coral4", "\033[1;30m");
    PFto_ADD("CornflowerBlue", "\033[1;34m");
    PFto_ADD("Cornsilk", "\033[1;37m");
    PFto_ADD("Cornsilk1", "\033[1;37m");
    PFto_ADD("Cornsilk2", "\033[1;37m");
    PFto_ADD("Cornsilk3", "\033[37m");
    PFto_ADD("Cornsilk4", "\033[1;30m");
    PFto_ADD("Cyan", "\033[1;36m");
    PFto_ADD("Cyan1", "\033[1;36m");
    PFto_ADD("Cyan2", "\033[36m");
    PFto_ADD("Cyan3", "\033[36m");
    PFto_ADD("Cyan4", "\033[36m");
    PFto_ADD("DARKGREY", "\033[1;30m");
    PFto_ADD("DarkBlue", "\033[34m");
    PFto_ADD("DarkCyan", "\033[36m");
    PFto_ADD("DarkGoldenrod", "\033[33m");
    PFto_ADD("DarkGoldenrod1", "\033[33m");
    PFto_ADD("DarkGoldenrod2", "\033[33m");
    PFto_ADD("DarkGoldenrod3", "\033[33m");
    PFto_ADD("DarkGoldenrod4", "\033[1;30m");
    PFto_ADD("DarkGray", "\033[37m");
    PFto_ADD("DarkGreen", "\033[32m");
    PFto_ADD("DarkGrey", "\033[37m");
    PFto_ADD("DarkKhaki", "\033[37m");
    PFto_ADD("DarkMagenta", "\033[35m");
    PFto_ADD("DarkOliveGreen", "\033[1;30m");
    PFto_ADD("DarkOliveGreen1", "\033[1;33m");
    PFto_ADD("DarkOliveGreen2", "\033[1;33m");
    PFto_ADD("DarkOliveGreen3", "\033[1;32m");
    PFto_ADD("DarkOliveGreen4", "\033[1;30m");
    PFto_ADD("DarkOrange", "\033[33m");
    PFto_ADD("DarkOrange1", "\033[33m");
    PFto_ADD("DarkOrange2", "\033[33m");
    PFto_ADD("DarkOrange3", "\033[33m");
    PFto_ADD("DarkOrange4", "\033[31m");
    PFto_ADD("DarkOrchid", "\033[35m");
    PFto_ADD("DarkOrchid1", "\033[1;35m");
    PFto_ADD("DarkOrchid2", "\033[35m");
    PFto_ADD("DarkOrchid3", "\033[35m");
    PFto_ADD("DarkOrchid4", "\033[1;30m");
    PFto_ADD("DarkRed", "\033[31m");
    PFto_ADD("DarkSalmon", "\033[1;31m");
    PFto_ADD("DarkSeaGreen", "\033[37m");
    PFto_ADD("DarkSeaGreen1", "\033[37m");
    PFto_ADD("DarkSeaGreen2", "\033[37m");
    PFto_ADD("DarkSeaGreen3", "\033[37m");
    PFto_ADD("DarkSeaGreen4", "\033[1;30m");
    PFto_ADD("DarkSlateBlue", "\033[1;30m");
    PFto_ADD("DarkSlateGray", "\033[1;30m");
    PFto_ADD("DarkSlateGray1", "\033[1;36m");
    PFto_ADD("DarkSlateGray2", "\033[1;36m");
    PFto_ADD("DarkSlateGray3", "\033[37m");
    PFto_ADD("DarkSlateGray4", "\033[1;30m");
    PFto_ADD("DarkSlateGrey", "\033[1;30m");
    PFto_ADD("DarkTurquoise", "\033[36m");
    PFto_ADD("DarkViolet", "\033[35m");
    PFto_ADD("DeepPink", "\033[35m");
    PFto_ADD("DeepPink1", "\033[35m");
    PFto_ADD("DeepPink2", "\033[35m");
    PFto_ADD("DeepPink3", "\033[35m");
    PFto_ADD("DeepPink4", "\033[1;30m");
    PFto_ADD("DeepSkyBlue", "\033[36m");
    PFto_ADD("DeepSkyBlue1", "\033[36m");
    PFto_ADD("DeepSkyBlue2", "\033[36m");
    PFto_ADD("DeepSkyBlue3", "\033[36m");
    PFto_ADD("DeepSkyBlue4", "\033[36m");
    PFto_ADD("DimGray", "\033[1;30m");
    PFto_ADD("DimGrey", "\033[1;30m");
    PFto_ADD("DodgerBlue", "\033[1;34m");
    PFto_ADD("DodgerBlue1", "\033[1;34m");
    PFto_ADD("DodgerBlue2", "\033[1;34m");
    PFto_ADD("DodgerBlue3", "\033[36m");
    PFto_ADD("DodgerBlue4", "\033[1;30m");
    PFto_ADD("ENDTERM", "");
    PFto_ADD("F000", "\033[30m");
    PFto_ADD("F001", "\033[30m");
    PFto_ADD("F002", "\033[34m");
    PFto_ADD("F003", "\033[34m");
    PFto_ADD("F004", "\033[34m");
    PFto_ADD("F005", "\033[34m");
    PFto_ADD("F010", "\033[30m");
    PFto_ADD("F011", "\033[1;30m");
    PFto_ADD("F012", "\033[34m");
    PFto_ADD("F013", "\033[34m");
    PFto_ADD("F014", "\033[34m");
    PFto_ADD("F015", "\033[1;34m");
    PFto_ADD("F020", "\033[32m");
    PFto_ADD("F021", "\033[32m");
    PFto_ADD("F022", "\033[36m");
    PFto_ADD("F023", "\033[36m");
    PFto_ADD("F024", "\033[36m");
    PFto_ADD("F025", "\033[36m");
    PFto_ADD("F030", "\033[32m");
    PFto_ADD("F031", "\033[32m");
    PFto_ADD("F032", "\033[36m");
    PFto_ADD("F033", "\033[36m");
    PFto_ADD("F034", "\033[36m");
    PFto_ADD("F035", "\033[36m");
    PFto_ADD("F040", "\033[32m");
    PFto_ADD("F041", "\033[32m");
    PFto_ADD("F042", "\033[36m");
    PFto_ADD("F043", "\033[36m");
    PFto_ADD("F044", "\033[36m");
    PFto_ADD("F045", "\033[36m");
    PFto_ADD("F050", "\033[32m");
    PFto_ADD("F051", "\033[1;32m");
    PFto_ADD("F052", "\033[36m");
    PFto_ADD("F053", "\033[36m");
    PFto_ADD("F054", "\033[36m");
    PFto_ADD("F055", "\033[1;36m");
    PFto_ADD("F100", "\033[30m");
    PFto_ADD("F101", "\033[1;30m");
    PFto_ADD("F102", "\033[34m");
    PFto_ADD("F103", "\033[34m");
    PFto_ADD("F104", "\033[34m");
    PFto_ADD("F105", "\033[1;34m");
    PFto_ADD("F110", "\033[1;30m");
    PFto_ADD("F111", "\033[1;30m");
    PFto_ADD("F112", "\033[1;30m");
    PFto_ADD("F113", "\033[1;34m");
    PFto_ADD("F114", "\033[1;34m");
    PFto_ADD("F115", "\033[1;34m");
    PFto_ADD("F120", "\033[32m");
    PFto_ADD("F121", "\033[1;30m");
    PFto_ADD("F122", "\033[1;30m");
    PFto_ADD("F123", "\033[1;34m");
    PFto_ADD("F124", "\033[1;34m");
    PFto_ADD("F125", "\033[1;34m");
    PFto_ADD("F130", "\033[32m");
    PFto_ADD("F131", "\033[1;32m");
    PFto_ADD("F132", "\033[1;32m");
    PFto_ADD("F133", "\033[36m");
    PFto_ADD("F134", "\033[1;36m");
    PFto_ADD("F135", "\033[1;36m");
    PFto_ADD("F140", "\033[32m");
    PFto_ADD("F141", "\033[1;32m");
    PFto_ADD("F142", "\033[1;32m");
    PFto_ADD("F143", "\033[1;36m");
    PFto_ADD("F144", "\033[1;36m");
    PFto_ADD("F145", "\033[1;36m");
    PFto_ADD("F150", "\033[1;32m");
    PFto_ADD("F151", "\033[1;32m");
    PFto_ADD("F152", "\033[1;32m");
    PFto_ADD("F153", "\033[1;36m");
    PFto_ADD("F154", "\033[1;36m");
    PFto_ADD("F155", "\033[1;36m");
    PFto_ADD("F200", "\033[31m");
    PFto_ADD("F201", "\033[31m");
    PFto_ADD("F202", "\033[35m");
    PFto_ADD("F203", "\033[35m");
    PFto_ADD("F204", "\033[35m");
    PFto_ADD("F205", "\033[35m");
    PFto_ADD("F210", "\033[31m");
    PFto_ADD("F211", "\033[1;30m");
    PFto_ADD("F212", "\033[1;30m");
    PFto_ADD("F213", "\033[1;34m");
    PFto_ADD("F214", "\033[1;34m");
    PFto_ADD("F215", "\033[1;34m");
    PFto_ADD("F220", "\033[33m");
    PFto_ADD("F221", "\033[1;30m");
    PFto_ADD("F222", "\033[37m");
    PFto_ADD("F223", "\033[37m");
    PFto_ADD("F224", "\033[37m");
    PFto_ADD("F225", "\033[1;34m");
    PFto_ADD("F230", "\033[33m");
    PFto_ADD("F231", "\033[1;32m");
    PFto_ADD("F232", "\033[37m");
    PFto_ADD("F233", "\033[37m");
    PFto_ADD("F234", "\033[37m");
    PFto_ADD("F235", "\033[37m");
    PFto_ADD("F240", "\033[33m");
    PFto_ADD("F241", "\033[1;32m");
    PFto_ADD("F242", "\033[37m");
    PFto_ADD("F243", "\033[37m");
    PFto_ADD("F244", "\033[37m");
    PFto_ADD("F245", "\033[1;36m");
    PFto_ADD("F250", "\033[33m");
    PFto_ADD("F251", "\033[1;32m");
    PFto_ADD("F252", "\033[1;32m");
    PFto_ADD("F253", "\033[37m");
    PFto_ADD("F254", "\033[1;36m");
    PFto_ADD("F255", "\033[1;36m");
    PFto_ADD("F300", "\033[31m");
    PFto_ADD("F301", "\033[31m");
    PFto_ADD("F302", "\033[35m");
    PFto_ADD("F303", "\033[35m");
    PFto_ADD("F304", "\033[35m");
    PFto_ADD("F305", "\033[35m");
    PFto_ADD("F310", "\033[31m");
    PFto_ADD("F311", "\033[1;31m");
    PFto_ADD("F312", "\033[1;31m");
    PFto_ADD("F313", "\033[35m");
    PFto_ADD("F314", "\033[1;35m");
    PFto_ADD("F315", "\033[1;35m");
    PFto_ADD("F320", "\033[33m");
    PFto_ADD("F321", "\033[1;31m");
    PFto_ADD("F322", "\033[37m");
    PFto_ADD("F323", "\033[37m");
    PFto_ADD("F324", "\033[37m");
    PFto_ADD("F325", "\033[37m");
    PFto_ADD("F330", "\033[33m");
    PFto_ADD("F331", "\033[33m");
    PFto_ADD("F332", "\033[37m");
    PFto_ADD("F333", "\033[37m");
    PFto_ADD("F334", "\033[37m");
    PFto_ADD("F335", "\033[37m");
    PFto_ADD("F340", "\033[33m");
    PFto_ADD("F341", "\033[1;33m");
    PFto_ADD("F342", "\033[37m");
    PFto_ADD("F343", "\033[37m");
    PFto_ADD("F344", "\033[37m");
    PFto_ADD("F345", "\033[37m");
    PFto_ADD("F350", "\033[33m");
    PFto_ADD("F351", "\033[1;33m");
    PFto_ADD("F352", "\033[37m");
    PFto_ADD("F353", "\033[37m");
    PFto_ADD("F354", "\033[37m");
    PFto_ADD("F355", "\033[1;37m");
    PFto_ADD("F400", "\033[31m");
    PFto_ADD("F401", "\033[31m");
    PFto_ADD("F402", "\033[35m");
    PFto_ADD("F403", "\033[35m");
    PFto_ADD("F404", "\033[35m");
    PFto_ADD("F405", "\033[35m");
    PFto_ADD("F410", "\033[31m");
    PFto_ADD("F411", "\033[1;31m");
    PFto_ADD("F412", "\033[1;31m");
    PFto_ADD("F413", "\033[1;35m");
    PFto_ADD("F414", "\033[1;35m");
    PFto_ADD("F415", "\033[1;35m");
    PFto_ADD("F420", "\033[33m");
    PFto_ADD("F421", "\033[1;31m");
    PFto_ADD("F422", "\033[37m");
    PFto_ADD("F423", "\033[37m");
    PFto_ADD("F424", "\033[37m");
    PFto_ADD("F425", "\033[1;35m");
    PFto_ADD("F430", "\033[33m");
    PFto_ADD("F431", "\033[1;33m");
    PFto_ADD("F432", "\033[37m");
    PFto_ADD("F433", "\033[37m");
    PFto_ADD("F434", "\033[37m");
    PFto_ADD("F435", "\033[37m");
    PFto_ADD("F440", "\033[33m");
    PFto_ADD("F441", "\033[1;33m");
    PFto_ADD("F442", "\033[37m");
    PFto_ADD("F443", "\033[37m");
    PFto_ADD("F444", "\033[37m");
    PFto_ADD("F445", "\033[1;37m");
    PFto_ADD("F450", "\033[33m");
    PFto_ADD("F451", "\033[1;33m");
    PFto_ADD("F452", "\033[1;33m");
    PFto_ADD("F453", "\033[37m");
    PFto_ADD("F454", "\033[1;37m");
    PFto_ADD("F455", "\033[1;37m");
    PFto_ADD("F500", "\033[31m");
    PFto_ADD("F501", "\033[1;31m");
    PFto_ADD("F502", "\033[35m");
    PFto_ADD("F503", "\033[35m");
    PFto_ADD("F504", "\033[35m");
    PFto_ADD("F505", "\033[1;35m");
    PFto_ADD("F510", "\033[1;31m");
    PFto_ADD("F511", "\033[1;31m");
    PFto_ADD("F512", "\033[1;31m");
    PFto_ADD("F513", "\033[1;35m");
    PFto_ADD("F514", "\033[1;35m");
    PFto_ADD("F515", "\033[1;35m");
    PFto_ADD("F520", "\033[33m");
    PFto_ADD("F521", "\033[1;31m");
    PFto_ADD("F522", "\033[1;31m");
    PFto_ADD("F523", "\033[37m");
    PFto_ADD("F524", "\033[1;35m");
    PFto_ADD("F525", "\033[1;35m");
    PFto_ADD("F530", "\033[33m");
    PFto_ADD("F531", "\033[1;33m");
    PFto_ADD("F532", "\033[37m");
    PFto_ADD("F533", "\033[37m");
    PFto_ADD("F534", "\033[37m");
    PFto_ADD("F535", "\033[1;37m");
    PFto_ADD("F540", "\033[33m");
    PFto_ADD("F541", "\033[1;33m");
    PFto_ADD("F542", "\033[1;33m");
    PFto_ADD("F543", "\033[37m");
    PFto_ADD("F544", "\033[1;37m");
    PFto_ADD("F545", "\033[1;37m");
    PFto_ADD("F550", "\033[1;33m");
    PFto_ADD("F551", "\033[1;33m");
    PFto_ADD("F552", "\033[1;33m");
    PFto_ADD("F553", "\033[1;37m");
    PFto_ADD("F554", "\033[1;37m");
    PFto_ADD("F555", "\033[1;37m");
    PFto_ADD("FLASH", "\033[5m");
    PFto_ADD("Firebrick", "\033[31m");
    PFto_ADD("Firebrick1", "\033[1;31m");
    PFto_ADD("Firebrick2", "\033[1;31m");
    PFto_ADD("Firebrick3", "\033[31m");
    PFto_ADD("Firebrick4", "\033[31m");
    PFto_ADD("FloralWhite", "\033[1;37m");
    PFto_ADD("ForestGreen", "\033[32m");
    PFto_ADD("G00", "\033[30m");
    PFto_ADD("G01", "\033[30m");
    PFto_ADD("G02", "\033[30m");
    PFto_ADD("G03", "\033[30m");
    PFto_ADD("G04", "\033[30m");
    PFto_ADD("G05", "\033[1;30m");
    PFto_ADD("G06", "\033[1;30m");
    PFto_ADD("G07", "\033[1;30m");
    PFto_ADD("G08", "\033[1;30m");
    PFto_ADD("G09", "\033[1;30m");
    PFto_ADD("G10", "\033[1;30m");
    PFto_ADD("G11", "\033[1;30m");
    PFto_ADD("G12", "\033[1;30m");
    PFto_ADD("G13", "\033[1;30m");
    PFto_ADD("G14", "\033[37m");
    PFto_ADD("G15", "\033[37m");
    PFto_ADD("G16", "\033[37m");
    PFto_ADD("G17", "\033[37m");
    PFto_ADD("G18", "\033[37m");
    PFto_ADD("G19", "\033[37m");
    PFto_ADD("G20", "\033[37m");
    PFto_ADD("G21", "\033[37m");
    PFto_ADD("G22", "\033[37m");
    PFto_ADD("G23", "\033[1;37m");
    PFto_ADD("G24", "\033[1;37m");
    PFto_ADD("G25", "\033[1;37m");
    PFto_ADD("GREEN", "\033[32m");
    PFto_ADD("GREY", "\033[37m");
    PFto_ADD("Gainsboro", "\033[37m");
    PFto_ADD("GhostWhite", "\033[1;37m");
    PFto_ADD("Gold", "\033[33m");
    PFto_ADD("Gold1", "\033[33m");
    PFto_ADD("Gold2", "\033[33m");
    PFto_ADD("Gold3", "\033[33m");
    PFto_ADD("Gold4", "\033[33m");
    PFto_ADD("Goldenrod", "\033[33m");
    PFto_ADD("Goldenrod1", "\033[33m");
    PFto_ADD("Goldenrod2", "\033[33m");
    PFto_ADD("Goldenrod3", "\033[33m");
    PFto_ADD("Goldenrod4", "\033[1;30m");
    PFto_ADD("Gray", "\033[37m");
    PFto_ADD("Gray0", "\033[30m");
    PFto_ADD("Gray1", "\033[30m");
    PFto_ADD("Gray10", "\033[30m");
    PFto_ADD("Gray100", "\033[1;37m");
    PFto_ADD("Gray11", "\033[30m");
    PFto_ADD("Gray12", "\033[30m");
    PFto_ADD("Gray13", "\033[30m");
    PFto_ADD("Gray14", "\033[30m");
    PFto_ADD("Gray15", "\033[30m");
    PFto_ADD("Gray16", "\033[30m");
    PFto_ADD("Gray17", "\033[1;30m");
    PFto_ADD("Gray18", "\033[1;30m");
    PFto_ADD("Gray19", "\033[1;30m");
    PFto_ADD("Gray2", "\033[30m");
    PFto_ADD("Gray20", "\033[1;30m");
    PFto_ADD("Gray21", "\033[1;30m");
    PFto_ADD("Gray22", "\033[1;30m");
    PFto_ADD("Gray23", "\033[1;30m");
    PFto_ADD("Gray24", "\033[1;30m");
    PFto_ADD("Gray25", "\033[1;30m");
    PFto_ADD("Gray26", "\033[1;30m");
    PFto_ADD("Gray27", "\033[1;30m");
    PFto_ADD("Gray28", "\033[1;30m");
    PFto_ADD("Gray29", "\033[1;30m");
    PFto_ADD("Gray3", "\033[30m");
    PFto_ADD("Gray30", "\033[1;30m");
    PFto_ADD("Gray31", "\033[1;30m");
    PFto_ADD("Gray32", "\033[1;30m");
    PFto_ADD("Gray33", "\033[1;30m");
    PFto_ADD("Gray34", "\033[1;30m");
    PFto_ADD("Gray35", "\033[1;30m");
    PFto_ADD("Gray36", "\033[1;30m");
    PFto_ADD("Gray37", "\033[1;30m");
    PFto_ADD("Gray38", "\033[1;30m");
    PFto_ADD("Gray39", "\033[1;30m");
    PFto_ADD("Gray4", "\033[30m");
    PFto_ADD("Gray40", "\033[1;30m");
    PFto_ADD("Gray41", "\033[1;30m");
    PFto_ADD("Gray42", "\033[1;30m");
    PFto_ADD("Gray43", "\033[1;30m");
    PFto_ADD("Gray44", "\033[1;30m");
    PFto_ADD("Gray45", "\033[1;30m");
    PFto_ADD("Gray46", "\033[1;30m");
    PFto_ADD("Gray47", "\033[1;30m");
    PFto_ADD("Gray48", "\033[1;30m");
    PFto_ADD("Gray49", "\033[1;30m");
    PFto_ADD("Gray5", "\033[30m");
    PFto_ADD("Gray50", "\033[1;30m");
    PFto_ADD("Gray51", "\033[1;30m");
    PFto_ADD("Gray52", "\033[1;30m");
    PFto_ADD("Gray53", "\033[1;30m");
    PFto_ADD("Gray54", "\033[37m");
    PFto_ADD("Gray55", "\033[37m");
    PFto_ADD("Gray56", "\033[37m");
    PFto_ADD("Gray57", "\033[37m");
    PFto_ADD("Gray58", "\033[37m");
    PFto_ADD("Gray59", "\033[37m");
    PFto_ADD("Gray6", "\033[30m");
    PFto_ADD("Gray60", "\033[37m");
    PFto_ADD("Gray61", "\033[37m");
    PFto_ADD("Gray62", "\033[37m");
    PFto_ADD("Gray63", "\033[37m");
    PFto_ADD("Gray64", "\033[37m");
    PFto_ADD("Gray65", "\033[37m");
    PFto_ADD("Gray66", "\033[37m");
    PFto_ADD("Gray67", "\033[37m");
    PFto_ADD("Gray68", "\033[37m");
    PFto_ADD("Gray69", "\033[37m");
    PFto_ADD("Gray7", "\033[30m");
    PFto_ADD("Gray70", "\033[37m");
    PFto_ADD("Gray71", "\033[37m");
    PFto_ADD("Gray72", "\033[37m");
    PFto_ADD("Gray73", "\033[37m");
    PFto_ADD("Gray74", "\033[37m");
    PFto_ADD("Gray75", "\033[37m");
    PFto_ADD("Gray76", "\033[37m");
    PFto_ADD("Gray77", "\033[37m");
    PFto_ADD("Gray78", "\033[37m");
    PFto_ADD("Gray79", "\033[37m");
    PFto_ADD("Gray8", "\033[30m");
    PFto_ADD("Gray80", "\033[37m");
    PFto_ADD("Gray81", "\033[37m");
    PFto_ADD("Gray82", "\033[37m");
    PFto_ADD("Gray83", "\033[37m");
    PFto_ADD("Gray84", "\033[37m");
    PFto_ADD("Gray85", "\033[37m");
    PFto_ADD("Gray86", "\033[37m");
    PFto_ADD("Gray87", "\033[1;37m");
    PFto_ADD("Gray88", "\033[1;37m");
    PFto_ADD("Gray89", "\033[1;37m");
    PFto_ADD("Gray9", "\033[30m");
    PFto_ADD("Gray90", "\033[1;37m");
    PFto_ADD("Gray91", "\033[1;37m");
    PFto_ADD("Gray92", "\033[1;37m");
    PFto_ADD("Gray93", "\033[1;37m");
    PFto_ADD("Gray94", "\033[1;37m");
    PFto_ADD("Gray95", "\033[1;37m");
    PFto_ADD("Gray96", "\033[1;37m");
    PFto_ADD("Gray97", "\033[1;37m");
    PFto_ADD("Gray98", "\033[1;37m");
    PFto_ADD("Gray99", "\033[1;37m");
    PFto_ADD("Green", "\033[32m");
    PFto_ADD("Green1", "\033[32m");
    PFto_ADD("Green2", "\033[32m");
    PFto_ADD("Green3", "\033[32m");
    PFto_ADD("Green4", "\033[32m");
    PFto_ADD("GreenYellow", "\033[33m");
    PFto_ADD("Grey", "\033[37m");
    PFto_ADD("Grey0", "\033[30m");
    PFto_ADD("Grey1", "\033[30m");
    PFto_ADD("Grey10", "\033[30m");
    PFto_ADD("Grey100", "\033[1;37m");
    PFto_ADD("Grey11", "\033[30m");
    PFto_ADD("Grey12", "\033[30m");
    PFto_ADD("Grey13", "\033[30m");
    PFto_ADD("Grey14", "\033[30m");
    PFto_ADD("Grey15", "\033[30m");
    PFto_ADD("Grey16", "\033[30m");
    PFto_ADD("Grey17", "\033[1;30m");
    PFto_ADD("Grey18", "\033[1;30m");
    PFto_ADD("Grey19", "\033[1;30m");
    PFto_ADD("Grey2", "\033[30m");
    PFto_ADD("Grey20", "\033[1;30m");
    PFto_ADD("Grey21", "\033[1;30m");
    PFto_ADD("Grey22", "\033[1;30m");
    PFto_ADD("Grey23", "\033[1;30m");
    PFto_ADD("Grey24", "\033[1;30m");
    PFto_ADD("Grey25", "\033[1;30m");
    PFto_ADD("Grey26", "\033[1;30m");
    PFto_ADD("Grey27", "\033[1;30m");
    PFto_ADD("Grey28", "\033[1;30m");
    PFto_ADD("Grey29", "\033[1;30m");
    PFto_ADD("Grey3", "\033[30m");
    PFto_ADD("Grey30", "\033[1;30m");
    PFto_ADD("Grey31", "\033[1;30m");
    PFto_ADD("Grey32", "\033[1;30m");
    PFto_ADD("Grey33", "\033[1;30m");
    PFto_ADD("Grey34", "\033[1;30m");
    PFto_ADD("Grey35", "\033[1;30m");
    PFto_ADD("Grey36", "\033[1;30m");
    PFto_ADD("Grey37", "\033[1;30m");
    PFto_ADD("Grey38", "\033[1;30m");
    PFto_ADD("Grey39", "\033[1;30m");
    PFto_ADD("Grey4", "\033[30m");
    PFto_ADD("Grey40", "\033[1;30m");
    PFto_ADD("Grey41", "\033[1;30m");
    PFto_ADD("Grey42", "\033[1;30m");
    PFto_ADD("Grey43", "\033[1;30m");
    PFto_ADD("Grey44", "\033[1;30m");
    PFto_ADD("Grey45", "\033[1;30m");
    PFto_ADD("Grey46", "\033[1;30m");
    PFto_ADD("Grey47", "\033[1;30m");
    PFto_ADD("Grey48", "\033[1;30m");
    PFto_ADD("Grey49", "\033[1;30m");
    PFto_ADD("Grey5", "\033[30m");
    PFto_ADD("Grey50", "\033[1;30m");
    PFto_ADD("Grey51", "\033[1;30m");
    PFto_ADD("Grey52", "\033[1;30m");
    PFto_ADD("Grey53", "\033[1;30m");
    PFto_ADD("Grey54", "\033[37m");
    PFto_ADD("Grey55", "\033[37m");
    PFto_ADD("Grey56", "\033[37m");
    PFto_ADD("Grey57", "\033[37m");
    PFto_ADD("Grey58", "\033[37m");
    PFto_ADD("Grey59", "\033[37m");
    PFto_ADD("Grey6", "\033[30m");
    PFto_ADD("Grey60", "\033[37m");
    PFto_ADD("Grey61", "\033[37m");
    PFto_ADD("Grey62", "\033[37m");
    PFto_ADD("Grey63", "\033[37m");
    PFto_ADD("Grey64", "\033[37m");
    PFto_ADD("Grey65", "\033[37m");
    PFto_ADD("Grey66", "\033[37m");
    PFto_ADD("Grey67", "\033[37m");
    PFto_ADD("Grey68", "\033[37m");
    PFto_ADD("Grey69", "\033[37m");
    PFto_ADD("Grey7", "\033[30m");
    PFto_ADD("Grey70", "\033[37m");
    PFto_ADD("Grey71", "\033[37m");
    PFto_ADD("Grey72", "\033[37m");
    PFto_ADD("Grey73", "\033[37m");
    PFto_ADD("Grey74", "\033[37m");
    PFto_ADD("Grey75", "\033[37m");
    PFto_ADD("Grey76", "\033[37m");
    PFto_ADD("Grey77", "\033[37m");
    PFto_ADD("Grey78", "\033[37m");
    PFto_ADD("Grey79", "\033[37m");
    PFto_ADD("Grey8", "\033[30m");
    PFto_ADD("Grey80", "\033[37m");
    PFto_ADD("Grey81", "\033[37m");
    PFto_ADD("Grey82", "\033[37m");
    PFto_ADD("Grey83", "\033[37m");
    PFto_ADD("Grey84", "\033[37m");
    PFto_ADD("Grey85", "\033[37m");
    PFto_ADD("Grey86", "\033[37m");
    PFto_ADD("Grey87", "\033[1;37m");
    PFto_ADD("Grey88", "\033[1;37m");
    PFto_ADD("Grey89", "\033[1;37m");
    PFto_ADD("Grey9", "\033[30m");
    PFto_ADD("Grey90", "\033[1;37m");
    PFto_ADD("Grey91", "\033[1;37m");
    PFto_ADD("Grey92", "\033[1;37m");
    PFto_ADD("Grey93", "\033[1;37m");
    PFto_ADD("Grey94", "\033[1;37m");
    PFto_ADD("Grey95", "\033[1;37m");
    PFto_ADD("Grey96", "\033[1;37m");
    PFto_ADD("Grey97", "\033[1;37m");
    PFto_ADD("Grey98", "\033[1;37m");
    PFto_ADD("Grey99", "\033[1;37m");
    PFto_ADD("HOME", "\033[H");
    PFto_ADD("Honeydew", "\033[1;37m");
    PFto_ADD("Honeydew1", "\033[1;37m");
    PFto_ADD("Honeydew2", "\033[1;37m");
    PFto_ADD("Honeydew3", "\033[37m");
    PFto_ADD("Honeydew4", "\033[1;30m");
    PFto_ADD("HotPink", "\033[1;35m");
    PFto_ADD("HotPink1", "\033[1;35m");
    PFto_ADD("HotPink2", "\033[1;31m");
    PFto_ADD("HotPink3", "\033[1;31m");
    PFto_ADD("HotPink4", "\033[1;30m");
    PFto_ADD("INITTERM", "\033[H\033[2J");
    PFto_ADD("ITALIC", "\033[3m");
    PFto_ADD("IndianRed", "\033[1;31m");
    PFto_ADD("IndianRed1", "\033[1;31m");
    PFto_ADD("IndianRed2", "\033[1;31m");
    PFto_ADD("IndianRed3", "\033[1;31m");
    PFto_ADD("IndianRed4", "\033[1;30m");
    PFto_ADD("Ivory", "\033[1;37m");
    PFto_ADD("Ivory1", "\033[1;37m");
    PFto_ADD("Ivory2", "\033[1;37m");
    PFto_ADD("Ivory3", "\033[37m");
    PFto_ADD("Ivory4", "\033[37m");
    PFto_ADD("Khaki", "\033[1;33m");
    PFto_ADD("Khaki1", "\033[1;33m");
    PFto_ADD("Khaki2", "\033[1;33m");
    PFto_ADD("Khaki3", "\033[37m");
    PFto_ADD("Khaki4", "\033[1;30m");
    PFto_ADD("LIGHTBLUE", "\033[1;34m");
    PFto_ADD("LIGHTCYAN", "\033[1;36m");
    PFto_ADD("LIGHTGREEN", "\033[1;32m");
    PFto_ADD("LIGHTRED", "\033[1;31m");
    PFto_ADD("Lavender", "\033[1;37m");
    PFto_ADD("LavenderBlush", "\033[1;37m");
    PFto_ADD("LavenderBlush1", "\033[1;37m");
    PFto_ADD("LavenderBlush2", "\033[1;37m");
    PFto_ADD("LavenderBlush3", "\033[37m");
    PFto_ADD("LavenderBlush4", "\033[1;30m");
    PFto_ADD("LawnGreen", "\033[33m");
    PFto_ADD("LemonChiffon", "\033[1;37m");
    PFto_ADD("LemonChiffon1", "\033[1;37m");
    PFto_ADD("LemonChiffon2", "\033[37m");
    PFto_ADD("LemonChiffon3", "\033[37m");
    PFto_ADD("LemonChiffon4", "\033[1;30m");
    PFto_ADD("LightBlue", "\033[37m");
    PFto_ADD("LightBlue1", "\033[1;37m");
    PFto_ADD("LightBlue2", "\033[37m");
    PFto_ADD("LightBlue3", "\033[37m");
    PFto_ADD("LightBlue4", "\033[1;30m");
    PFto_ADD("LightCoral", "\033[1;31m");
    PFto_ADD("LightCyan", "\033[1;37m");
    PFto_ADD("LightCyan1", "\033[1;37m");
    PFto_ADD("LightCyan2", "\033[1;37m");
    PFto_ADD("LightCyan3", "\033[37m");
    PFto_ADD("LightCyan4", "\033[1;30m");
    PFto_ADD("LightGoldenrod", "\033[1;33m");
    PFto_ADD("LightGoldenrod1", "\033[1;33m");
    PFto_ADD("LightGoldenrod2", "\033[1;33m");
    PFto_ADD("LightGoldenrod3", "\033[37m");
    PFto_ADD("LightGoldenrod4", "\033[1;30m");
    PFto_ADD("LightGoldenrodYellow", "\033[1;37m");
    PFto_ADD("LightGray", "\033[37m");
    PFto_ADD("LightGreen", "\033[37m");
    PFto_ADD("LightGrey", "\033[37m");
    PFto_ADD("LightPink", "\033[37m");
    PFto_ADD("LightPink1", "\033[37m");
    PFto_ADD("LightPink2", "\033[37m");
    PFto_ADD("LightPink3", "\033[37m");
    PFto_ADD("LightPink4", "\033[1;30m");
    PFto_ADD("LightSalmon", "\033[1;31m");
    PFto_ADD("LightSalmon1", "\033[1;31m");
    PFto_ADD("LightSalmon2", "\033[1;31m");
    PFto_ADD("LightSalmon3", "\033[1;31m");
    PFto_ADD("LightSalmon4", "\033[1;30m");
    PFto_ADD("LightSeaGreen", "\033[36m");
    PFto_ADD("LightSkyBlue", "\033[1;36m");
    PFto_ADD("LightSkyBlue1", "\033[37m");
    PFto_ADD("LightSkyBlue2", "\033[37m");
    PFto_ADD("LightSkyBlue3", "\033[37m");
    PFto_ADD("LightSkyBlue4", "\033[1;30m");
    PFto_ADD("LightSlateBlue", "\033[1;34m");
    PFto_ADD("LightSlateGray", "\033[37m");
    PFto_ADD("LightSlateGrey", "\033[37m");
    PFto_ADD("LightSteelBlue", "\033[37m");
    PFto_ADD("LightSteelBlue1", "\033[1;37m");
    PFto_ADD("LightSteelBlue2", "\033[37m");
    PFto_ADD("LightSteelBlue3", "\033[37m");
    PFto_ADD("LightSteelBlue4", "\033[1;30m");
    PFto_ADD("LightYellow", "\033[1;37m");
    PFto_ADD("LightYellow1", "\033[1;37m");
    PFto_ADD("LightYellow2", "\033[1;37m");
    PFto_ADD("LightYellow3", "\033[37m");
    PFto_ADD("LightYellow4", "\033[1;30m");
    PFto_ADD("LimeGreen", "\033[1;32m");
    PFto_ADD("Linen", "\033[1;37m");
    PFto_ADD("MAGENTA", "\033[35m");
    PFto_ADD("Magenta", "\033[1;35m");
    PFto_ADD("Magenta1", "\033[1;35m");
    PFto_ADD("Magenta2", "\033[35m");
    PFto_ADD("Magenta3", "\033[35m");
    PFto_ADD("Magenta4", "\033[35m");
    PFto_ADD("Maroon", "\033[1;31m");
    PFto_ADD("Maroon1", "\033[1;35m");
    PFto_ADD("Maroon2", "\033[35m");
    PFto_ADD("Maroon3", "\033[35m");
    PFto_ADD("Maroon4", "\033[1;30m");
    PFto_ADD("MediumAquamarine", "\033[37m");
    PFto_ADD("MediumBlue", "\033[34m");
    PFto_ADD("MediumOrchid", "\033[1;35m");
    PFto_ADD("MediumOrchid1", "\033[1;35m");
    PFto_ADD("MediumOrchid2", "\033[1;35m");
    PFto_ADD("MediumOrchid3", "\033[35m");
    PFto_ADD("MediumOrchid4", "\033[1;30m");
    PFto_ADD("MediumPurple", "\033[1;34m");
    PFto_ADD("MediumPurple1", "\033[37m");
    PFto_ADD("MediumPurple2", "\033[1;34m");
    PFto_ADD("MediumPurple3", "\033[1;34m");
    PFto_ADD("MediumPurple4", "\033[1;30m");
    PFto_ADD("MediumSeaGreen", "\033[1;32m");
    PFto_ADD("MediumSlateBlue", "\033[1;34m");
    PFto_ADD("MediumSpringGreen", "\033[36m");
    PFto_ADD("MediumTurquoise", "\033[1;36m");
    PFto_ADD("MediumVioletRed", "\033[35m");
    PFto_ADD("MidnightBlue", "\033[34m");
    PFto_ADD("MintCream", "\033[1;37m");
    PFto_ADD("MistyRose", "\033[1;37m");
    PFto_ADD("MistyRose1", "\033[1;37m");
    PFto_ADD("MistyRose2", "\033[37m");
    PFto_ADD("MistyRose3", "\033[37m");
    PFto_ADD("MistyRose4", "\033[1;30m");
    PFto_ADD("Moccasin", "\033[1;37m");
    PFto_ADD("NavajoWhite", "\033[37m");
    PFto_ADD("NavajoWhite1", "\033[37m");
    PFto_ADD("NavajoWhite2", "\033[37m");
    PFto_ADD("NavajoWhite3", "\033[37m");
    PFto_ADD("NavajoWhite4", "\033[1;30m");
    PFto_ADD("Navy", "\033[34m");
    PFto_ADD("NavyBlue", "\033[34m");
    PFto_ADD("ORANGE", "\033[33m");
    PFto_ADD("OldLace", "\033[1;37m");
    PFto_ADD("OliveDrab", "\033[1;30m");
    PFto_ADD("OliveDrab1", "\033[1;33m");
    PFto_ADD("OliveDrab2", "\033[33m");
    PFto_ADD("OliveDrab3", "\033[33m");
    PFto_ADD("OliveDrab4", "\033[1;30m");
    PFto_ADD("Orange", "\033[33m");
    PFto_ADD("Orange1", "\033[33m");
    PFto_ADD("Orange2", "\033[33m");
    PFto_ADD("Orange3", "\033[33m");
    PFto_ADD("Orange4", "\033[1;30m");
    PFto_ADD("OrangeRed", "\033[1;31m");
    PFto_ADD("OrangeRed1", "\033[1;31m");
    PFto_ADD("OrangeRed2", "\033[31m");
    PFto_ADD("OrangeRed3", "\033[31m");
    PFto_ADD("OrangeRed4", "\033[31m");
    PFto_ADD("Orchid", "\033[1;35m");
    PFto_ADD("Orchid1", "\033[1;35m");
    PFto_ADD("Orchid2", "\033[1;35m");
    PFto_ADD("Orchid3", "\033[1;35m");
    PFto_ADD("Orchid4", "\033[1;30m");
    PFto_ADD("PINK", "\033[1;35m");
    PFto_ADD("PaleGoldenrod", "\033[37m");
    PFto_ADD("PaleGreen", "\033[37m");
    PFto_ADD("PaleGreen1", "\033[37m");
    PFto_ADD("PaleGreen2", "\033[37m");
    PFto_ADD("PaleGreen3", "\033[1;32m");
    PFto_ADD("PaleGreen4", "\033[1;30m");
    PFto_ADD("PaleTurquoise", "\033[37m");
    PFto_ADD("PaleTurquoise1", "\033[1;37m");
    PFto_ADD("PaleTurquoise2", "\033[37m");
    PFto_ADD("PaleTurquoise3", "\033[37m");
    PFto_ADD("PaleTurquoise4", "\033[1;30m");
    PFto_ADD("PaleVioletRed", "\033[1;31m");
    PFto_ADD("PaleVioletRed1", "\033[37m");
    PFto_ADD("PaleVioletRed2", "\033[1;31m");
    PFto_ADD("PaleVioletRed3", "\033[1;31m");
    PFto_ADD("PaleVioletRed4", "\033[1;30m");
    PFto_ADD("PapayaWhip", "\033[1;37m");
    PFto_ADD("PeachPuff", "\033[37m");
    PFto_ADD("PeachPuff1", "\033[37m");
    PFto_ADD("PeachPuff2", "\033[37m");
    PFto_ADD("PeachPuff3", "\033[37m");
    PFto_ADD("PeachPuff4", "\033[1;30m");
    PFto_ADD("Peru", "\033[1;31m");
    PFto_ADD("Pink", "\033[37m");
    PFto_ADD("Pink1", "\033[37m");
    PFto_ADD("Pink2", "\033[37m");
    PFto_ADD("Pink3", "\033[37m");
    PFto_ADD("Pink4", "\033[1;30m");
    PFto_ADD("Plum", "\033[37m");
    PFto_ADD("Plum1", "\033[1;37m");
    PFto_ADD("Plum2", "\033[37m");
    PFto_ADD("Plum3", "\033[37m");
    PFto_ADD("Plum4", "\033[1;30m");
    PFto_ADD("PowderBlue", "\033[37m");
    PFto_ADD("Purple", "\033[35m");
    PFto_ADD("Purple1", "\033[1;34m");
    PFto_ADD("Purple2", "\033[1;34m");
    PFto_ADD("Purple3", "\033[35m");
    PFto_ADD("Purple4", "\033[1;30m");
    PFto_ADD("RED", "\033[31m");
    PFto_ADD("RESET", "\033[0m");
    PFto_ADD("RESTORE", "\033[u");
    PFto_ADD("REVERSE", "\033[7m");
    PFto_ADD("Red", "\033[31m");
    PFto_ADD("Red1", "\033[31m");
    PFto_ADD("Red2", "\033[31m");
    PFto_ADD("Red3", "\033[31m");
    PFto_ADD("Red4", "\033[31m");
    PFto_ADD("RosyBrown", "\033[37m");
    PFto_ADD("RosyBrown1", "\033[37m");
    PFto_ADD("RosyBrown2", "\033[37m");
    PFto_ADD("RosyBrown3", "\033[37m");
    PFto_ADD("RosyBrown4", "\033[1;30m");
    PFto_ADD("RoyalBlue", "\033[1;34m");
    PFto_ADD("RoyalBlue1", "\033[1;34m");
    PFto_ADD("RoyalBlue2", "\033[1;34m");
    PFto_ADD("RoyalBlue3", "\033[1;34m");
    PFto_ADD("RoyalBlue4", "\033[1;30m");
    PFto_ADD("SAVE", "\033[s");
    PFto_ADD("STRIKETHRU", "\033[9m");
    PFto_ADD("SaddleBrown", "\033[31m");
    PFto_ADD("Salmon", "\033[1;31m");
    PFto_ADD("Salmon1", "\033[1;31m");
    PFto_ADD("Salmon2", "\033[1;31m");
    PFto_ADD("Salmon3", "\033[1;31m");
    PFto_ADD("Salmon4", "\033[1;30m");
    PFto_ADD("SandyBrown", "\033[1;31m");
    PFto_ADD("SeaGreen", "\033[1;30m");
    PFto_ADD("SeaGreen1", "\033[1;32m");
    PFto_ADD("SeaGreen2", "\033[1;32m");
    PFto_ADD("SeaGreen3", "\033[1;32m");
    PFto_ADD("SeaGreen4", "\033[1;30m");
    PFto_ADD("Seashell", "\033[1;37m");
    PFto_ADD("Seashell1", "\033[1;37m");
    PFto_ADD("Seashell2", "\033[1;37m");
    PFto_ADD("Seashell3", "\033[37m");
    PFto_ADD("Seashell4", "\033[1;30m");
    PFto_ADD("Sienna", "\033[1;30m");
    PFto_ADD("Sienna1", "\033[1;31m");
    PFto_ADD("Sienna2", "\033[1;31m");
    PFto_ADD("Sienna3", "\033[1;31m");
    PFto_ADD("Sienna4", "\033[1;30m");
    PFto_ADD("SkyBlue", "\033[1;36m");
    PFto_ADD("SkyBlue1", "\033[1;36m");
    PFto_ADD("SkyBlue2", "\033[1;36m");
    PFto_ADD("SkyBlue3", "\033[37m");
    PFto_ADD("SkyBlue4", "\033[1;30m");
    PFto_ADD("SlateBlue", "\033[1;34m");
    PFto_ADD("SlateBlue1", "\033[1;34m");
    PFto_ADD("SlateBlue2", "\033[1;34m");
    PFto_ADD("SlateBlue3", "\033[1;34m");
    PFto_ADD("SlateBlue4", "\033[1;30m");
    PFto_ADD("SlateGray", "\033[1;30m");
    PFto_ADD("SlateGray1", "\033[1;37m");
    PFto_ADD("SlateGray2", "\033[37m");
    PFto_ADD("SlateGray3", "\033[37m");
    PFto_ADD("SlateGray4", "\033[1;30m");
    PFto_ADD("SlateGrey", "\033[1;30m");
    PFto_ADD("Snow", "\033[1;37m");
    PFto_ADD("Snow1", "\033[1;37m");
    PFto_ADD("Snow2", "\033[1;37m");
    PFto_ADD("Snow3", "\033[37m");
    PFto_ADD("Snow4", "\033[37m");
    PFto_ADD("SpringGreen", "\033[36m");
    PFto_ADD("SpringGreen1", "\033[36m");
    PFto_ADD("SpringGreen2", "\033[36m");
    PFto_ADD("SpringGreen3", "\033[36m");
    PFto_ADD("SpringGreen4", "\033[32m");
    PFto_ADD("SteelBlue", "\033[1;34m");
    PFto_ADD("SteelBlue1", "\033[1;36m");
    PFto_ADD("SteelBlue2", "\033[1;36m");
    PFto_ADD("SteelBlue3", "\033[1;34m");
    PFto_ADD("SteelBlue4", "\033[1;30m");
    PFto_ADD("Tan", "\033[37m");
    PFto_ADD("Tan1", "\033[1;31m");
    PFto_ADD("Tan2", "\033[1;31m");
    PFto_ADD("Tan3", "\033[1;31m");
    PFto_ADD("Tan4", "\033[1;30m");
    PFto_ADD("Thistle", "\033[37m");
    PFto_ADD("Thistle1", "\033[1;37m");
    PFto_ADD("Thistle2", "\033[1;37m");
    PFto_ADD("Thistle3", "\033[37m");
    PFto_ADD("Thistle4", "\033[1;30m");
    PFto_ADD("Tomato", "\033[1;31m");
    PFto_ADD("Tomato1", "\033[1;31m");
    PFto_ADD("Tomato2", "\033[1;31m");
    PFto_ADD("Tomato3", "\033[1;31m");
    PFto_ADD("Tomato4", "\033[1;30m");
    PFto_ADD("Turquoise", "\033[1;36m");
    PFto_ADD("Turquoise1", "\033[1;36m");
    PFto_ADD("Turquoise2", "\033[36m");
    PFto_ADD("Turquoise3", "\033[36m");
    PFto_ADD("Turquoise4", "\033[36m");
    PFto_ADD("UNDERLINE", "\033[4m");
    PFto_ADD("Violet", "\033[1;35m");
    PFto_ADD("VioletRed", "\033[35m");
    PFto_ADD("VioletRed1", "\033[1;31m");
    PFto_ADD("VioletRed2", "\033[1;31m");
    PFto_ADD("VioletRed3", "\033[1;31m");
    PFto_ADD("VioletRed4", "\033[1;30m");
    PFto_ADD("WHITE", "\033[1;37m");
    PFto_ADD("Wheat", "\033[37m");
    PFto_ADD("Wheat1", "\033[1;37m");
    PFto_ADD("Wheat2", "\033[37m");
    PFto_ADD("Wheat3", "\033[37m");
    PFto_ADD("Wheat4", "\033[1;30m");
    PFto_ADD("White", "\033[1;37m");
    PFto_ADD("WhiteSmoke", "\033[1;37m");
    PFto_ADD("YELLOW", "\033[1;33m");
    PFto_ADD("Yellow", "\033[1;33m");
    PFto_ADD("Yellow1", "\033[1;33m");
    PFto_ADD("Yellow2", "\033[33m");
    PFto_ADD("Yellow3", "\033[33m");
    PFto_ADD("Yellow4", "\033[33m");
    PFto_ADD("YellowGreen", "\033[33m");

    log_info("Pinkfish to ANSI conversion data loaded.");
}

void I3_loadPinkfishToI3(void)
{
    log_info("Initializing Pinkfish to I3 conversion table...");
    stringmap_destroy(pinkfish_to_i3_db); // Free anything present
    pinkfish_to_i3_db = stringmap_init(); // Setup a new empty structure

    log_info("Loading Pinkfish to I3 conversion data...");

#undef PFto_ADD
#define PFto_ADD(key, value)                                                                                           \
    do                                                                                                                 \
    {                                                                                                                  \
        stringmap_add(pinkfish_to_i3_db, (key), (void *)(value), strlen((char *)(value)));                             \
        pinkfish_to_i3_count++;                                                                                        \
    } while (0)

    PFto_ADD("AliceBlue", "%^BOLD%^%^WHITE%^");
    PFto_ADD("AntiqueWhite", "%^BOLD%^%^WHITE%^");
    PFto_ADD("AntiqueWhite1", "%^BOLD%^%^WHITE%^");
    PFto_ADD("AntiqueWhite2", "%^BOLD%^%^WHITE%^");
    PFto_ADD("AntiqueWhite3", "%^WHITE%^");
    PFto_ADD("AntiqueWhite4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Aquamarine", "%^BOLD%^%^CYAN%^");
    PFto_ADD("Aquamarine1", "%^BOLD%^%^CYAN%^");
    PFto_ADD("Aquamarine2", "%^BOLD%^%^CYAN%^");
    PFto_ADD("Aquamarine3", "%^WHITE%^");
    PFto_ADD("Aquamarine4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Azure", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Azure1", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Azure2", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Azure3", "%^WHITE%^");
    PFto_ADD("Azure4", "%^WHITE%^");
    PFto_ADD("B000", "%^B_BLACK%^");
    PFto_ADD("B001", "%^B_BLACK%^");
    PFto_ADD("B002", "%^B_BLUE%^");
    PFto_ADD("B003", "%^B_BLUE%^");
    PFto_ADD("B004", "%^B_BLUE%^");
    PFto_ADD("B005", "%^B_BLUE%^");
    PFto_ADD("B010", "%^B_BLACK%^");
    PFto_ADD("B011", "%^B_BLACK%^");
    PFto_ADD("B012", "%^B_BLUE%^");
    PFto_ADD("B013", "%^B_BLUE%^");
    PFto_ADD("B014", "%^B_BLUE%^");
    PFto_ADD("B015", "%^B_BLUE%^");
    PFto_ADD("B020", "%^B_GREEN%^");
    PFto_ADD("B021", "%^B_GREEN%^");
    PFto_ADD("B022", "%^B_CYAN%^");
    PFto_ADD("B023", "%^B_CYAN%^");
    PFto_ADD("B024", "%^B_CYAN%^");
    PFto_ADD("B025", "%^B_CYAN%^");
    PFto_ADD("B030", "%^B_GREEN%^");
    PFto_ADD("B031", "%^B_GREEN%^");
    PFto_ADD("B032", "%^B_CYAN%^");
    PFto_ADD("B033", "%^B_CYAN%^");
    PFto_ADD("B034", "%^B_CYAN%^");
    PFto_ADD("B035", "%^B_CYAN%^");
    PFto_ADD("B040", "%^B_GREEN%^");
    PFto_ADD("B041", "%^B_GREEN%^");
    PFto_ADD("B042", "%^B_CYAN%^");
    PFto_ADD("B043", "%^B_CYAN%^");
    PFto_ADD("B044", "%^B_CYAN%^");
    PFto_ADD("B045", "%^B_CYAN%^");
    PFto_ADD("B050", "%^B_GREEN%^");
    PFto_ADD("B051", "%^B_GREEN%^");
    PFto_ADD("B052", "%^B_CYAN%^");
    PFto_ADD("B053", "%^B_CYAN%^");
    PFto_ADD("B054", "%^B_CYAN%^");
    PFto_ADD("B055", "%^B_CYAN%^");
    PFto_ADD("B100", "%^B_BLACK%^");
    PFto_ADD("B101", "%^B_BLACK%^");
    PFto_ADD("B102", "%^B_BLUE%^");
    PFto_ADD("B103", "%^B_BLUE%^");
    PFto_ADD("B104", "%^B_BLUE%^");
    PFto_ADD("B105", "%^B_BLUE%^");
    PFto_ADD("B110", "%^B_BLACK%^");
    PFto_ADD("B111", "%^B_BLACK%^");
    PFto_ADD("B112", "%^B_BLACK%^");
    PFto_ADD("B113", "%^B_BLUE%^");
    PFto_ADD("B114", "%^B_BLUE%^");
    PFto_ADD("B115", "%^B_BLUE%^");
    PFto_ADD("B120", "%^B_GREEN%^");
    PFto_ADD("B121", "%^B_BLACK%^");
    PFto_ADD("B122", "%^B_BLACK%^");
    PFto_ADD("B123", "%^B_BLUE%^");
    PFto_ADD("B124", "%^B_BLUE%^");
    PFto_ADD("B125", "%^B_BLUE%^");
    PFto_ADD("B130", "%^B_GREEN%^");
    PFto_ADD("B131", "%^B_GREEN%^");
    PFto_ADD("B132", "%^B_GREEN%^");
    PFto_ADD("B133", "%^B_CYAN%^");
    PFto_ADD("B134", "%^B_CYAN%^");
    PFto_ADD("B135", "%^B_CYAN%^");
    PFto_ADD("B140", "%^B_GREEN%^");
    PFto_ADD("B141", "%^B_GREEN%^");
    PFto_ADD("B142", "%^B_GREEN%^");
    PFto_ADD("B143", "%^B_CYAN%^");
    PFto_ADD("B144", "%^B_CYAN%^");
    PFto_ADD("B145", "%^B_CYAN%^");
    PFto_ADD("B150", "%^B_GREEN%^");
    PFto_ADD("B151", "%^B_GREEN%^");
    PFto_ADD("B152", "%^B_GREEN%^");
    PFto_ADD("B153", "%^B_CYAN%^");
    PFto_ADD("B154", "%^B_CYAN%^");
    PFto_ADD("B155", "%^B_CYAN%^");
    PFto_ADD("B200", "%^B_RED%^");
    PFto_ADD("B201", "%^B_RED%^");
    PFto_ADD("B202", "%^B_MAGENTA%^");
    PFto_ADD("B203", "%^B_MAGENTA%^");
    PFto_ADD("B204", "%^B_MAGENTA%^");
    PFto_ADD("B205", "%^B_MAGENTA%^");
    PFto_ADD("B210", "%^B_RED%^");
    PFto_ADD("B211", "%^B_BLACK%^");
    PFto_ADD("B212", "%^B_BLACK%^");
    PFto_ADD("B213", "%^B_BLUE%^");
    PFto_ADD("B214", "%^B_BLUE%^");
    PFto_ADD("B215", "%^B_BLUE%^");
    PFto_ADD("B220", "%^B_ORANGE%^");
    PFto_ADD("B221", "%^B_BLACK%^");
    PFto_ADD("B222", "%^B_WHITE%^");
    PFto_ADD("B223", "%^B_WHITE%^");
    PFto_ADD("B224", "%^B_WHITE%^");
    PFto_ADD("B225", "%^B_BLUE%^");
    PFto_ADD("B230", "%^B_ORANGE%^");
    PFto_ADD("B231", "%^B_GREEN%^");
    PFto_ADD("B232", "%^B_WHITE%^");
    PFto_ADD("B233", "%^B_WHITE%^");
    PFto_ADD("B234", "%^B_WHITE%^");
    PFto_ADD("B235", "%^B_WHITE%^");
    PFto_ADD("B240", "%^B_ORANGE%^");
    PFto_ADD("B241", "%^B_GREEN%^");
    PFto_ADD("B242", "%^B_WHITE%^");
    PFto_ADD("B243", "%^B_WHITE%^");
    PFto_ADD("B244", "%^B_WHITE%^");
    PFto_ADD("B245", "%^B_CYAN%^");
    PFto_ADD("B250", "%^B_ORANGE%^");
    PFto_ADD("B251", "%^B_GREEN%^");
    PFto_ADD("B252", "%^B_GREEN%^");
    PFto_ADD("B253", "%^B_WHITE%^");
    PFto_ADD("B254", "%^B_CYAN%^");
    PFto_ADD("B255", "%^B_CYAN%^");
    PFto_ADD("B300", "%^B_RED%^");
    PFto_ADD("B301", "%^B_RED%^");
    PFto_ADD("B302", "%^B_MAGENTA%^");
    PFto_ADD("B303", "%^B_MAGENTA%^");
    PFto_ADD("B304", "%^B_MAGENTA%^");
    PFto_ADD("B305", "%^B_MAGENTA%^");
    PFto_ADD("B310", "%^B_RED%^");
    PFto_ADD("B311", "%^B_RED%^");
    PFto_ADD("B312", "%^B_RED%^");
    PFto_ADD("B313", "%^B_MAGENTA%^");
    PFto_ADD("B314", "%^B_MAGENTA%^");
    PFto_ADD("B315", "%^B_MAGENTA%^");
    PFto_ADD("B320", "%^B_ORANGE%^");
    PFto_ADD("B321", "%^B_RED%^");
    PFto_ADD("B322", "%^B_WHITE%^");
    PFto_ADD("B323", "%^B_WHITE%^");
    PFto_ADD("B324", "%^B_WHITE%^");
    PFto_ADD("B325", "%^B_WHITE%^");
    PFto_ADD("B330", "%^B_ORANGE%^");
    PFto_ADD("B331", "%^B_ORANGE%^");
    PFto_ADD("B332", "%^B_WHITE%^");
    PFto_ADD("B333", "%^B_WHITE%^");
    PFto_ADD("B334", "%^B_WHITE%^");
    PFto_ADD("B335", "%^B_WHITE%^");
    PFto_ADD("B340", "%^B_ORANGE%^");
    PFto_ADD("B341", "%^B_YELLOW%^");
    PFto_ADD("B342", "%^B_WHITE%^");
    PFto_ADD("B343", "%^B_WHITE%^");
    PFto_ADD("B344", "%^B_WHITE%^");
    PFto_ADD("B345", "%^B_WHITE%^");
    PFto_ADD("B350", "%^B_ORANGE%^");
    PFto_ADD("B351", "%^B_YELLOW%^");
    PFto_ADD("B352", "%^B_WHITE%^");
    PFto_ADD("B353", "%^B_WHITE%^");
    PFto_ADD("B354", "%^B_WHITE%^");
    PFto_ADD("B355", "%^B_WHITE%^");
    PFto_ADD("B400", "%^B_RED%^");
    PFto_ADD("B401", "%^B_RED%^");
    PFto_ADD("B402", "%^B_MAGENTA%^");
    PFto_ADD("B403", "%^B_MAGENTA%^");
    PFto_ADD("B404", "%^B_MAGENTA%^");
    PFto_ADD("B405", "%^B_MAGENTA%^");
    PFto_ADD("B410", "%^B_RED%^");
    PFto_ADD("B411", "%^B_RED%^");
    PFto_ADD("B412", "%^B_RED%^");
    PFto_ADD("B413", "%^B_MAGENTA%^");
    PFto_ADD("B414", "%^B_MAGENTA%^");
    PFto_ADD("B415", "%^B_MAGENTA%^");
    PFto_ADD("B420", "%^B_ORANGE%^");
    PFto_ADD("B421", "%^B_RED%^");
    PFto_ADD("B422", "%^B_WHITE%^");
    PFto_ADD("B423", "%^B_WHITE%^");
    PFto_ADD("B424", "%^B_WHITE%^");
    PFto_ADD("B425", "%^B_MAGENTA%^");
    PFto_ADD("B430", "%^B_ORANGE%^");
    PFto_ADD("B431", "%^B_YELLOW%^");
    PFto_ADD("B432", "%^B_WHITE%^");
    PFto_ADD("B433", "%^B_WHITE%^");
    PFto_ADD("B434", "%^B_WHITE%^");
    PFto_ADD("B435", "%^B_WHITE%^");
    PFto_ADD("B440", "%^B_ORANGE%^");
    PFto_ADD("B441", "%^B_YELLOW%^");
    PFto_ADD("B442", "%^B_WHITE%^");
    PFto_ADD("B443", "%^B_WHITE%^");
    PFto_ADD("B444", "%^B_WHITE%^");
    PFto_ADD("B445", "%^B_WHITE%^");
    PFto_ADD("B450", "%^B_ORANGE%^");
    PFto_ADD("B451", "%^B_YELLOW%^");
    PFto_ADD("B452", "%^B_YELLOW%^");
    PFto_ADD("B453", "%^B_WHITE%^");
    PFto_ADD("B454", "%^B_WHITE%^");
    PFto_ADD("B455", "%^B_WHITE%^");
    PFto_ADD("B500", "%^B_RED%^");
    PFto_ADD("B501", "%^B_RED%^");
    PFto_ADD("B502", "%^B_MAGENTA%^");
    PFto_ADD("B503", "%^B_MAGENTA%^");
    PFto_ADD("B504", "%^B_MAGENTA%^");
    PFto_ADD("B505", "%^B_MAGENTA%^");
    PFto_ADD("B510", "%^B_RED%^");
    PFto_ADD("B511", "%^B_RED%^");
    PFto_ADD("B512", "%^B_RED%^");
    PFto_ADD("B513", "%^B_MAGENTA%^");
    PFto_ADD("B514", "%^B_MAGENTA%^");
    PFto_ADD("B515", "%^B_MAGENTA%^");
    PFto_ADD("B520", "%^B_ORANGE%^");
    PFto_ADD("B521", "%^B_RED%^");
    PFto_ADD("B522", "%^B_RED%^");
    PFto_ADD("B523", "%^B_WHITE%^");
    PFto_ADD("B524", "%^B_MAGENTA%^");
    PFto_ADD("B525", "%^B_MAGENTA%^");
    PFto_ADD("B530", "%^B_ORANGE%^");
    PFto_ADD("B531", "%^B_YELLOW%^");
    PFto_ADD("B532", "%^B_WHITE%^");
    PFto_ADD("B533", "%^B_WHITE%^");
    PFto_ADD("B534", "%^B_WHITE%^");
    PFto_ADD("B535", "%^B_WHITE%^");
    PFto_ADD("B540", "%^B_ORANGE%^");
    PFto_ADD("B541", "%^B_YELLOW%^");
    PFto_ADD("B542", "%^B_YELLOW%^");
    PFto_ADD("B543", "%^B_WHITE%^");
    PFto_ADD("B544", "%^B_WHITE%^");
    PFto_ADD("B545", "%^B_WHITE%^");
    PFto_ADD("B550", "%^B_YELLOW%^");
    PFto_ADD("B551", "%^B_YELLOW%^");
    PFto_ADD("B552", "%^B_YELLOW%^");
    PFto_ADD("B553", "%^B_WHITE%^");
    PFto_ADD("B554", "%^B_WHITE%^");
    PFto_ADD("B555", "%^B_WHITE%^");
    PFto_ADD("BG00", "%^B_BLACK%^");
    PFto_ADD("BG01", "%^B_BLACK%^");
    PFto_ADD("BG02", "%^B_BLACK%^");
    PFto_ADD("BG03", "%^B_BLACK%^");
    PFto_ADD("BG04", "%^B_BLACK%^");
    PFto_ADD("BG05", "%^B_BLACK%^");
    PFto_ADD("BG06", "%^B_BLACK%^");
    PFto_ADD("BG07", "%^B_BLACK%^");
    PFto_ADD("BG08", "%^B_BLACK%^");
    PFto_ADD("BG09", "%^B_BLACK%^");
    PFto_ADD("BG10", "%^B_BLACK%^");
    PFto_ADD("BG11", "%^B_BLACK%^");
    PFto_ADD("BG12", "%^B_BLACK%^");
    PFto_ADD("BG13", "%^B_BLACK%^");
    PFto_ADD("BG14", "%^B_WHITE%^");
    PFto_ADD("BG15", "%^B_WHITE%^");
    PFto_ADD("BG16", "%^B_WHITE%^");
    PFto_ADD("BG17", "%^B_WHITE%^");
    PFto_ADD("BG18", "%^B_WHITE%^");
    PFto_ADD("BG19", "%^B_WHITE%^");
    PFto_ADD("BG20", "%^B_WHITE%^");
    PFto_ADD("BG21", "%^B_WHITE%^");
    PFto_ADD("BG22", "%^B_WHITE%^");
    PFto_ADD("BG23", "%^B_WHITE%^");
    PFto_ADD("BG24", "%^B_WHITE%^");
    PFto_ADD("BG25", "%^B_WHITE%^");
    PFto_ADD("BLACK", "%^BLACK%^");
    PFto_ADD("BLUE", "%^BLUE%^");
    PFto_ADD("BOLD", "%^BOLD%^");
    PFto_ADD("B_AliceBlue", "%^B_WHITE%^");
    PFto_ADD("B_AntiqueWhite", "%^B_WHITE%^");
    PFto_ADD("B_AntiqueWhite1", "%^B_WHITE%^");
    PFto_ADD("B_AntiqueWhite2", "%^B_WHITE%^");
    PFto_ADD("B_AntiqueWhite3", "%^B_WHITE%^");
    PFto_ADD("B_AntiqueWhite4", "%^B_BLACK%^");
    PFto_ADD("B_Aquamarine", "%^B_CYAN%^");
    PFto_ADD("B_Aquamarine1", "%^B_CYAN%^");
    PFto_ADD("B_Aquamarine2", "%^B_CYAN%^");
    PFto_ADD("B_Aquamarine3", "%^B_WHITE%^");
    PFto_ADD("B_Aquamarine4", "%^B_BLACK%^");
    PFto_ADD("B_Azure", "%^B_WHITE%^");
    PFto_ADD("B_Azure1", "%^B_WHITE%^");
    PFto_ADD("B_Azure2", "%^B_WHITE%^");
    PFto_ADD("B_Azure3", "%^B_WHITE%^");
    PFto_ADD("B_Azure4", "%^B_WHITE%^");
    PFto_ADD("B_BLACK", "%^B_BLACK%^");
    PFto_ADD("B_BLUE", "%^B_BLUE%^");
    PFto_ADD("B_Beige", "%^B_WHITE%^");
    PFto_ADD("B_Bisque", "%^B_WHITE%^");
    PFto_ADD("B_Bisque1", "%^B_WHITE%^");
    PFto_ADD("B_Bisque2", "%^B_WHITE%^");
    PFto_ADD("B_Bisque3", "%^B_WHITE%^");
    PFto_ADD("B_Bisque4", "%^B_BLACK%^");
    PFto_ADD("B_Black", "%^B_BLACK%^");
    PFto_ADD("B_BlanchedAlmond", "%^B_WHITE%^");
    PFto_ADD("B_Blue", "%^B_BLUE%^");
    PFto_ADD("B_Blue1", "%^B_BLUE%^");
    PFto_ADD("B_Blue2", "%^B_BLUE%^");
    PFto_ADD("B_Blue3", "%^B_BLUE%^");
    PFto_ADD("B_Blue4", "%^B_BLUE%^");
    PFto_ADD("B_BlueViolet", "%^B_BLUE%^");
    PFto_ADD("B_Brown", "%^B_RED%^");
    PFto_ADD("B_Brown1", "%^B_RED%^");
    PFto_ADD("B_Brown2", "%^B_RED%^");
    PFto_ADD("B_Brown3", "%^B_RED%^");
    PFto_ADD("B_Brown4", "%^B_RED%^");
    PFto_ADD("B_Burlywood", "%^B_WHITE%^");
    PFto_ADD("B_Burlywood1", "%^B_WHITE%^");
    PFto_ADD("B_Burlywood2", "%^B_WHITE%^");
    PFto_ADD("B_Burlywood3", "%^B_WHITE%^");
    PFto_ADD("B_Burlywood4", "%^B_BLACK%^");
    PFto_ADD("B_CYAN", "%^B_CYAN%^");
    PFto_ADD("B_CadetBlue", "%^B_WHITE%^");
    PFto_ADD("B_CadetBlue1", "%^B_CYAN%^");
    PFto_ADD("B_CadetBlue2", "%^B_CYAN%^");
    PFto_ADD("B_CadetBlue3", "%^B_WHITE%^");
    PFto_ADD("B_CadetBlue4", "%^B_BLACK%^");
    PFto_ADD("B_Chartreuse", "%^B_ORANGE%^");
    PFto_ADD("B_Chartreuse1", "%^B_ORANGE%^");
    PFto_ADD("B_Chartreuse2", "%^B_ORANGE%^");
    PFto_ADD("B_Chartreuse3", "%^B_ORANGE%^");
    PFto_ADD("B_Chartreuse4", "%^B_GREEN%^");
    PFto_ADD("B_Chocolate", "%^B_RED%^");
    PFto_ADD("B_Chocolate1", "%^B_RED%^");
    PFto_ADD("B_Chocolate2", "%^B_RED%^");
    PFto_ADD("B_Chocolate3", "%^B_RED%^");
    PFto_ADD("B_Chocolate4", "%^B_RED%^");
    PFto_ADD("B_Coral", "%^B_RED%^");
    PFto_ADD("B_Coral1", "%^B_RED%^");
    PFto_ADD("B_Coral2", "%^B_RED%^");
    PFto_ADD("B_Coral3", "%^B_RED%^");
    PFto_ADD("B_Coral4", "%^B_BLACK%^");
    PFto_ADD("B_CornflowerBlue", "%^B_BLUE%^");
    PFto_ADD("B_Cornsilk", "%^B_WHITE%^");
    PFto_ADD("B_Cornsilk1", "%^B_WHITE%^");
    PFto_ADD("B_Cornsilk2", "%^B_WHITE%^");
    PFto_ADD("B_Cornsilk3", "%^B_WHITE%^");
    PFto_ADD("B_Cornsilk4", "%^B_BLACK%^");
    PFto_ADD("B_Cyan", "%^B_CYAN%^");
    PFto_ADD("B_Cyan1", "%^B_CYAN%^");
    PFto_ADD("B_Cyan2", "%^B_CYAN%^");
    PFto_ADD("B_Cyan3", "%^B_CYAN%^");
    PFto_ADD("B_Cyan4", "%^B_CYAN%^");
    PFto_ADD("B_DARKGREY", "%^B_BLACK%^");
    PFto_ADD("B_DarkBlue", "%^B_BLUE%^");
    PFto_ADD("B_DarkCyan", "%^B_CYAN%^");
    PFto_ADD("B_DarkGoldenrod", "%^B_ORANGE%^");
    PFto_ADD("B_DarkGoldenrod1", "%^B_ORANGE%^");
    PFto_ADD("B_DarkGoldenrod2", "%^B_ORANGE%^");
    PFto_ADD("B_DarkGoldenrod3", "%^B_ORANGE%^");
    PFto_ADD("B_DarkGoldenrod4", "%^B_BLACK%^");
    PFto_ADD("B_DarkGray", "%^B_WHITE%^");
    PFto_ADD("B_DarkGreen", "%^B_GREEN%^");
    PFto_ADD("B_DarkGrey", "%^B_WHITE%^");
    PFto_ADD("B_DarkKhaki", "%^B_WHITE%^");
    PFto_ADD("B_DarkMagenta", "%^B_MAGENTA%^");
    PFto_ADD("B_DarkOliveGreen", "%^B_BLACK%^");
    PFto_ADD("B_DarkOliveGreen1", "%^B_YELLOW%^");
    PFto_ADD("B_DarkOliveGreen2", "%^B_YELLOW%^");
    PFto_ADD("B_DarkOliveGreen3", "%^B_GREEN%^");
    PFto_ADD("B_DarkOliveGreen4", "%^B_BLACK%^");
    PFto_ADD("B_DarkOrange", "%^B_ORANGE%^");
    PFto_ADD("B_DarkOrange1", "%^B_ORANGE%^");
    PFto_ADD("B_DarkOrange2", "%^B_ORANGE%^");
    PFto_ADD("B_DarkOrange3", "%^B_ORANGE%^");
    PFto_ADD("B_DarkOrange4", "%^B_RED%^");
    PFto_ADD("B_DarkOrchid", "%^B_MAGENTA%^");
    PFto_ADD("B_DarkOrchid1", "%^B_MAGENTA%^");
    PFto_ADD("B_DarkOrchid2", "%^B_MAGENTA%^");
    PFto_ADD("B_DarkOrchid3", "%^B_MAGENTA%^");
    PFto_ADD("B_DarkOrchid4", "%^B_BLACK%^");
    PFto_ADD("B_DarkRed", "%^B_RED%^");
    PFto_ADD("B_DarkSalmon", "%^B_RED%^");
    PFto_ADD("B_DarkSeaGreen", "%^B_WHITE%^");
    PFto_ADD("B_DarkSeaGreen1", "%^B_WHITE%^");
    PFto_ADD("B_DarkSeaGreen2", "%^B_WHITE%^");
    PFto_ADD("B_DarkSeaGreen3", "%^B_WHITE%^");
    PFto_ADD("B_DarkSeaGreen4", "%^B_BLACK%^");
    PFto_ADD("B_DarkSlateBlue", "%^B_BLACK%^");
    PFto_ADD("B_DarkSlateGray", "%^B_BLACK%^");
    PFto_ADD("B_DarkSlateGray1", "%^B_CYAN%^");
    PFto_ADD("B_DarkSlateGray2", "%^B_CYAN%^");
    PFto_ADD("B_DarkSlateGray3", "%^B_WHITE%^");
    PFto_ADD("B_DarkSlateGray4", "%^B_BLACK%^");
    PFto_ADD("B_DarkSlateGrey", "%^B_BLACK%^");
    PFto_ADD("B_DarkTurquoise", "%^B_CYAN%^");
    PFto_ADD("B_DarkViolet", "%^B_MAGENTA%^");
    PFto_ADD("B_DeepPink", "%^B_MAGENTA%^");
    PFto_ADD("B_DeepPink1", "%^B_MAGENTA%^");
    PFto_ADD("B_DeepPink2", "%^B_MAGENTA%^");
    PFto_ADD("B_DeepPink3", "%^B_MAGENTA%^");
    PFto_ADD("B_DeepPink4", "%^B_BLACK%^");
    PFto_ADD("B_DeepSkyBlue", "%^B_CYAN%^");
    PFto_ADD("B_DeepSkyBlue1", "%^B_CYAN%^");
    PFto_ADD("B_DeepSkyBlue2", "%^B_CYAN%^");
    PFto_ADD("B_DeepSkyBlue3", "%^B_CYAN%^");
    PFto_ADD("B_DeepSkyBlue4", "%^B_CYAN%^");
    PFto_ADD("B_DimGray", "%^B_BLACK%^");
    PFto_ADD("B_DimGrey", "%^B_BLACK%^");
    PFto_ADD("B_DodgerBlue", "%^B_BLUE%^");
    PFto_ADD("B_DodgerBlue1", "%^B_BLUE%^");
    PFto_ADD("B_DodgerBlue2", "%^B_BLUE%^");
    PFto_ADD("B_DodgerBlue3", "%^B_CYAN%^");
    PFto_ADD("B_DodgerBlue4", "%^B_BLACK%^");
    PFto_ADD("B_Firebrick", "%^B_RED%^");
    PFto_ADD("B_Firebrick1", "%^B_RED%^");
    PFto_ADD("B_Firebrick2", "%^B_RED%^");
    PFto_ADD("B_Firebrick3", "%^B_RED%^");
    PFto_ADD("B_Firebrick4", "%^B_RED%^");
    PFto_ADD("B_FloralWhite", "%^B_WHITE%^");
    PFto_ADD("B_ForestGreen", "%^B_GREEN%^");
    PFto_ADD("B_GREEN", "%^B_GREEN%^");
    PFto_ADD("B_GREY", "%^B_WHITE%^");
    PFto_ADD("B_Gainsboro", "%^B_WHITE%^");
    PFto_ADD("B_GhostWhite", "%^B_WHITE%^");
    PFto_ADD("B_Gold", "%^B_ORANGE%^");
    PFto_ADD("B_Gold1", "%^B_ORANGE%^");
    PFto_ADD("B_Gold2", "%^B_ORANGE%^");
    PFto_ADD("B_Gold3", "%^B_ORANGE%^");
    PFto_ADD("B_Gold4", "%^B_ORANGE%^");
    PFto_ADD("B_Goldenrod", "%^B_ORANGE%^");
    PFto_ADD("B_Goldenrod1", "%^B_ORANGE%^");
    PFto_ADD("B_Goldenrod2", "%^B_ORANGE%^");
    PFto_ADD("B_Goldenrod3", "%^B_ORANGE%^");
    PFto_ADD("B_Goldenrod4", "%^B_BLACK%^");
    PFto_ADD("B_Gray", "%^B_WHITE%^");
    PFto_ADD("B_Gray0", "%^B_BLACK%^");
    PFto_ADD("B_Gray1", "%^B_BLACK%^");
    PFto_ADD("B_Gray10", "%^B_BLACK%^");
    PFto_ADD("B_Gray100", "%^B_WHITE%^");
    PFto_ADD("B_Gray11", "%^B_BLACK%^");
    PFto_ADD("B_Gray12", "%^B_BLACK%^");
    PFto_ADD("B_Gray13", "%^B_BLACK%^");
    PFto_ADD("B_Gray14", "%^B_BLACK%^");
    PFto_ADD("B_Gray15", "%^B_BLACK%^");
    PFto_ADD("B_Gray16", "%^B_BLACK%^");
    PFto_ADD("B_Gray17", "%^B_BLACK%^");
    PFto_ADD("B_Gray18", "%^B_BLACK%^");
    PFto_ADD("B_Gray19", "%^B_BLACK%^");
    PFto_ADD("B_Gray2", "%^B_BLACK%^");
    PFto_ADD("B_Gray20", "%^B_BLACK%^");
    PFto_ADD("B_Gray21", "%^B_BLACK%^");
    PFto_ADD("B_Gray22", "%^B_BLACK%^");
    PFto_ADD("B_Gray23", "%^B_BLACK%^");
    PFto_ADD("B_Gray24", "%^B_BLACK%^");
    PFto_ADD("B_Gray25", "%^B_BLACK%^");
    PFto_ADD("B_Gray26", "%^B_BLACK%^");
    PFto_ADD("B_Gray27", "%^B_BLACK%^");
    PFto_ADD("B_Gray28", "%^B_BLACK%^");
    PFto_ADD("B_Gray29", "%^B_BLACK%^");
    PFto_ADD("B_Gray3", "%^B_BLACK%^");
    PFto_ADD("B_Gray30", "%^B_BLACK%^");
    PFto_ADD("B_Gray31", "%^B_BLACK%^");
    PFto_ADD("B_Gray32", "%^B_BLACK%^");
    PFto_ADD("B_Gray33", "%^B_BLACK%^");
    PFto_ADD("B_Gray34", "%^B_BLACK%^");
    PFto_ADD("B_Gray35", "%^B_BLACK%^");
    PFto_ADD("B_Gray36", "%^B_BLACK%^");
    PFto_ADD("B_Gray37", "%^B_BLACK%^");
    PFto_ADD("B_Gray38", "%^B_BLACK%^");
    PFto_ADD("B_Gray39", "%^B_BLACK%^");
    PFto_ADD("B_Gray4", "%^B_BLACK%^");
    PFto_ADD("B_Gray40", "%^B_BLACK%^");
    PFto_ADD("B_Gray41", "%^B_BLACK%^");
    PFto_ADD("B_Gray42", "%^B_BLACK%^");
    PFto_ADD("B_Gray43", "%^B_BLACK%^");
    PFto_ADD("B_Gray44", "%^B_BLACK%^");
    PFto_ADD("B_Gray45", "%^B_BLACK%^");
    PFto_ADD("B_Gray46", "%^B_BLACK%^");
    PFto_ADD("B_Gray47", "%^B_BLACK%^");
    PFto_ADD("B_Gray48", "%^B_BLACK%^");
    PFto_ADD("B_Gray49", "%^B_BLACK%^");
    PFto_ADD("B_Gray5", "%^B_BLACK%^");
    PFto_ADD("B_Gray50", "%^B_BLACK%^");
    PFto_ADD("B_Gray51", "%^B_BLACK%^");
    PFto_ADD("B_Gray52", "%^B_BLACK%^");
    PFto_ADD("B_Gray53", "%^B_BLACK%^");
    PFto_ADD("B_Gray54", "%^B_WHITE%^");
    PFto_ADD("B_Gray55", "%^B_WHITE%^");
    PFto_ADD("B_Gray56", "%^B_WHITE%^");
    PFto_ADD("B_Gray57", "%^B_WHITE%^");
    PFto_ADD("B_Gray58", "%^B_WHITE%^");
    PFto_ADD("B_Gray59", "%^B_WHITE%^");
    PFto_ADD("B_Gray6", "%^B_BLACK%^");
    PFto_ADD("B_Gray60", "%^B_WHITE%^");
    PFto_ADD("B_Gray61", "%^B_WHITE%^");
    PFto_ADD("B_Gray62", "%^B_WHITE%^");
    PFto_ADD("B_Gray63", "%^B_WHITE%^");
    PFto_ADD("B_Gray64", "%^B_WHITE%^");
    PFto_ADD("B_Gray65", "%^B_WHITE%^");
    PFto_ADD("B_Gray66", "%^B_WHITE%^");
    PFto_ADD("B_Gray67", "%^B_WHITE%^");
    PFto_ADD("B_Gray68", "%^B_WHITE%^");
    PFto_ADD("B_Gray69", "%^B_WHITE%^");
    PFto_ADD("B_Gray7", "%^B_BLACK%^");
    PFto_ADD("B_Gray70", "%^B_WHITE%^");
    PFto_ADD("B_Gray71", "%^B_WHITE%^");
    PFto_ADD("B_Gray72", "%^B_WHITE%^");
    PFto_ADD("B_Gray73", "%^B_WHITE%^");
    PFto_ADD("B_Gray74", "%^B_WHITE%^");
    PFto_ADD("B_Gray75", "%^B_WHITE%^");
    PFto_ADD("B_Gray76", "%^B_WHITE%^");
    PFto_ADD("B_Gray77", "%^B_WHITE%^");
    PFto_ADD("B_Gray78", "%^B_WHITE%^");
    PFto_ADD("B_Gray79", "%^B_WHITE%^");
    PFto_ADD("B_Gray8", "%^B_BLACK%^");
    PFto_ADD("B_Gray80", "%^B_WHITE%^");
    PFto_ADD("B_Gray81", "%^B_WHITE%^");
    PFto_ADD("B_Gray82", "%^B_WHITE%^");
    PFto_ADD("B_Gray83", "%^B_WHITE%^");
    PFto_ADD("B_Gray84", "%^B_WHITE%^");
    PFto_ADD("B_Gray85", "%^B_WHITE%^");
    PFto_ADD("B_Gray86", "%^B_WHITE%^");
    PFto_ADD("B_Gray87", "%^B_WHITE%^");
    PFto_ADD("B_Gray88", "%^B_WHITE%^");
    PFto_ADD("B_Gray89", "%^B_WHITE%^");
    PFto_ADD("B_Gray9", "%^B_BLACK%^");
    PFto_ADD("B_Gray90", "%^B_WHITE%^");
    PFto_ADD("B_Gray91", "%^B_WHITE%^");
    PFto_ADD("B_Gray92", "%^B_WHITE%^");
    PFto_ADD("B_Gray93", "%^B_WHITE%^");
    PFto_ADD("B_Gray94", "%^B_WHITE%^");
    PFto_ADD("B_Gray95", "%^B_WHITE%^");
    PFto_ADD("B_Gray96", "%^B_WHITE%^");
    PFto_ADD("B_Gray97", "%^B_WHITE%^");
    PFto_ADD("B_Gray98", "%^B_WHITE%^");
    PFto_ADD("B_Gray99", "%^B_WHITE%^");
    PFto_ADD("B_Green", "%^B_GREEN%^");
    PFto_ADD("B_Green1", "%^B_GREEN%^");
    PFto_ADD("B_Green2", "%^B_GREEN%^");
    PFto_ADD("B_Green3", "%^B_GREEN%^");
    PFto_ADD("B_Green4", "%^B_GREEN%^");
    PFto_ADD("B_GreenYellow", "%^B_ORANGE%^");
    PFto_ADD("B_Grey", "%^B_WHITE%^");
    PFto_ADD("B_Grey0", "%^B_BLACK%^");
    PFto_ADD("B_Grey1", "%^B_BLACK%^");
    PFto_ADD("B_Grey10", "%^B_BLACK%^");
    PFto_ADD("B_Grey100", "%^B_WHITE%^");
    PFto_ADD("B_Grey11", "%^B_BLACK%^");
    PFto_ADD("B_Grey12", "%^B_BLACK%^");
    PFto_ADD("B_Grey13", "%^B_BLACK%^");
    PFto_ADD("B_Grey14", "%^B_BLACK%^");
    PFto_ADD("B_Grey15", "%^B_BLACK%^");
    PFto_ADD("B_Grey16", "%^B_BLACK%^");
    PFto_ADD("B_Grey17", "%^B_BLACK%^");
    PFto_ADD("B_Grey18", "%^B_BLACK%^");
    PFto_ADD("B_Grey19", "%^B_BLACK%^");
    PFto_ADD("B_Grey2", "%^B_BLACK%^");
    PFto_ADD("B_Grey20", "%^B_BLACK%^");
    PFto_ADD("B_Grey21", "%^B_BLACK%^");
    PFto_ADD("B_Grey22", "%^B_BLACK%^");
    PFto_ADD("B_Grey23", "%^B_BLACK%^");
    PFto_ADD("B_Grey24", "%^B_BLACK%^");
    PFto_ADD("B_Grey25", "%^B_BLACK%^");
    PFto_ADD("B_Grey26", "%^B_BLACK%^");
    PFto_ADD("B_Grey27", "%^B_BLACK%^");
    PFto_ADD("B_Grey28", "%^B_BLACK%^");
    PFto_ADD("B_Grey29", "%^B_BLACK%^");
    PFto_ADD("B_Grey3", "%^B_BLACK%^");
    PFto_ADD("B_Grey30", "%^B_BLACK%^");
    PFto_ADD("B_Grey31", "%^B_BLACK%^");
    PFto_ADD("B_Grey32", "%^B_BLACK%^");
    PFto_ADD("B_Grey33", "%^B_BLACK%^");
    PFto_ADD("B_Grey34", "%^B_BLACK%^");
    PFto_ADD("B_Grey35", "%^B_BLACK%^");
    PFto_ADD("B_Grey36", "%^B_BLACK%^");
    PFto_ADD("B_Grey37", "%^B_BLACK%^");
    PFto_ADD("B_Grey38", "%^B_BLACK%^");
    PFto_ADD("B_Grey39", "%^B_BLACK%^");
    PFto_ADD("B_Grey4", "%^B_BLACK%^");
    PFto_ADD("B_Grey40", "%^B_BLACK%^");
    PFto_ADD("B_Grey41", "%^B_BLACK%^");
    PFto_ADD("B_Grey42", "%^B_BLACK%^");
    PFto_ADD("B_Grey43", "%^B_BLACK%^");
    PFto_ADD("B_Grey44", "%^B_BLACK%^");
    PFto_ADD("B_Grey45", "%^B_BLACK%^");
    PFto_ADD("B_Grey46", "%^B_BLACK%^");
    PFto_ADD("B_Grey47", "%^B_BLACK%^");
    PFto_ADD("B_Grey48", "%^B_BLACK%^");
    PFto_ADD("B_Grey49", "%^B_BLACK%^");
    PFto_ADD("B_Grey5", "%^B_BLACK%^");
    PFto_ADD("B_Grey50", "%^B_BLACK%^");
    PFto_ADD("B_Grey51", "%^B_BLACK%^");
    PFto_ADD("B_Grey52", "%^B_BLACK%^");
    PFto_ADD("B_Grey53", "%^B_BLACK%^");
    PFto_ADD("B_Grey54", "%^B_WHITE%^");
    PFto_ADD("B_Grey55", "%^B_WHITE%^");
    PFto_ADD("B_Grey56", "%^B_WHITE%^");
    PFto_ADD("B_Grey57", "%^B_WHITE%^");
    PFto_ADD("B_Grey58", "%^B_WHITE%^");
    PFto_ADD("B_Grey59", "%^B_WHITE%^");
    PFto_ADD("B_Grey6", "%^B_BLACK%^");
    PFto_ADD("B_Grey60", "%^B_WHITE%^");
    PFto_ADD("B_Grey61", "%^B_WHITE%^");
    PFto_ADD("B_Grey62", "%^B_WHITE%^");
    PFto_ADD("B_Grey63", "%^B_WHITE%^");
    PFto_ADD("B_Grey64", "%^B_WHITE%^");
    PFto_ADD("B_Grey65", "%^B_WHITE%^");
    PFto_ADD("B_Grey66", "%^B_WHITE%^");
    PFto_ADD("B_Grey67", "%^B_WHITE%^");
    PFto_ADD("B_Grey68", "%^B_WHITE%^");
    PFto_ADD("B_Grey69", "%^B_WHITE%^");
    PFto_ADD("B_Grey7", "%^B_BLACK%^");
    PFto_ADD("B_Grey70", "%^B_WHITE%^");
    PFto_ADD("B_Grey71", "%^B_WHITE%^");
    PFto_ADD("B_Grey72", "%^B_WHITE%^");
    PFto_ADD("B_Grey73", "%^B_WHITE%^");
    PFto_ADD("B_Grey74", "%^B_WHITE%^");
    PFto_ADD("B_Grey75", "%^B_WHITE%^");
    PFto_ADD("B_Grey76", "%^B_WHITE%^");
    PFto_ADD("B_Grey77", "%^B_WHITE%^");
    PFto_ADD("B_Grey78", "%^B_WHITE%^");
    PFto_ADD("B_Grey79", "%^B_WHITE%^");
    PFto_ADD("B_Grey8", "%^B_BLACK%^");
    PFto_ADD("B_Grey80", "%^B_WHITE%^");
    PFto_ADD("B_Grey81", "%^B_WHITE%^");
    PFto_ADD("B_Grey82", "%^B_WHITE%^");
    PFto_ADD("B_Grey83", "%^B_WHITE%^");
    PFto_ADD("B_Grey84", "%^B_WHITE%^");
    PFto_ADD("B_Grey85", "%^B_WHITE%^");
    PFto_ADD("B_Grey86", "%^B_WHITE%^");
    PFto_ADD("B_Grey87", "%^B_WHITE%^");
    PFto_ADD("B_Grey88", "%^B_WHITE%^");
    PFto_ADD("B_Grey89", "%^B_WHITE%^");
    PFto_ADD("B_Grey9", "%^B_BLACK%^");
    PFto_ADD("B_Grey90", "%^B_WHITE%^");
    PFto_ADD("B_Grey91", "%^B_WHITE%^");
    PFto_ADD("B_Grey92", "%^B_WHITE%^");
    PFto_ADD("B_Grey93", "%^B_WHITE%^");
    PFto_ADD("B_Grey94", "%^B_WHITE%^");
    PFto_ADD("B_Grey95", "%^B_WHITE%^");
    PFto_ADD("B_Grey96", "%^B_WHITE%^");
    PFto_ADD("B_Grey97", "%^B_WHITE%^");
    PFto_ADD("B_Grey98", "%^B_WHITE%^");
    PFto_ADD("B_Grey99", "%^B_WHITE%^");
    PFto_ADD("B_Honeydew", "%^B_WHITE%^");
    PFto_ADD("B_Honeydew1", "%^B_WHITE%^");
    PFto_ADD("B_Honeydew2", "%^B_WHITE%^");
    PFto_ADD("B_Honeydew3", "%^B_WHITE%^");
    PFto_ADD("B_Honeydew4", "%^B_BLACK%^");
    PFto_ADD("B_HotPink", "%^B_MAGENTA%^");
    PFto_ADD("B_HotPink1", "%^B_MAGENTA%^");
    PFto_ADD("B_HotPink2", "%^B_RED%^");
    PFto_ADD("B_HotPink3", "%^B_RED%^");
    PFto_ADD("B_HotPink4", "%^B_BLACK%^");
    PFto_ADD("B_IndianRed", "%^B_RED%^");
    PFto_ADD("B_IndianRed1", "%^B_RED%^");
    PFto_ADD("B_IndianRed2", "%^B_RED%^");
    PFto_ADD("B_IndianRed3", "%^B_RED%^");
    PFto_ADD("B_IndianRed4", "%^B_BLACK%^");
    PFto_ADD("B_Ivory", "%^B_WHITE%^");
    PFto_ADD("B_Ivory1", "%^B_WHITE%^");
    PFto_ADD("B_Ivory2", "%^B_WHITE%^");
    PFto_ADD("B_Ivory3", "%^B_WHITE%^");
    PFto_ADD("B_Ivory4", "%^B_WHITE%^");
    PFto_ADD("B_Khaki", "%^B_YELLOW%^");
    PFto_ADD("B_Khaki1", "%^B_YELLOW%^");
    PFto_ADD("B_Khaki2", "%^B_YELLOW%^");
    PFto_ADD("B_Khaki3", "%^B_WHITE%^");
    PFto_ADD("B_Khaki4", "%^B_BLACK%^");
    PFto_ADD("B_LIGHTBLUE", "%^B_BLUE%^");
    PFto_ADD("B_LIGHTCYAN", "%^B_CYAN%^");
    PFto_ADD("B_LIGHTGREEN", "%^B_GREEN%^");
    PFto_ADD("B_LIGHTRED", "%^B_RED%^");
    PFto_ADD("B_Lavender", "%^B_WHITE%^");
    PFto_ADD("B_LavenderBlush", "%^B_WHITE%^");
    PFto_ADD("B_LavenderBlush1", "%^B_WHITE%^");
    PFto_ADD("B_LavenderBlush2", "%^B_WHITE%^");
    PFto_ADD("B_LavenderBlush3", "%^B_WHITE%^");
    PFto_ADD("B_LavenderBlush4", "%^B_BLACK%^");
    PFto_ADD("B_LawnGreen", "%^B_ORANGE%^");
    PFto_ADD("B_LemonChiffon", "%^B_WHITE%^");
    PFto_ADD("B_LemonChiffon1", "%^B_WHITE%^");
    PFto_ADD("B_LemonChiffon2", "%^B_WHITE%^");
    PFto_ADD("B_LemonChiffon3", "%^B_WHITE%^");
    PFto_ADD("B_LemonChiffon4", "%^B_BLACK%^");
    PFto_ADD("B_LightBlue", "%^B_WHITE%^");
    PFto_ADD("B_LightBlue1", "%^B_WHITE%^");
    PFto_ADD("B_LightBlue2", "%^B_WHITE%^");
    PFto_ADD("B_LightBlue3", "%^B_WHITE%^");
    PFto_ADD("B_LightBlue4", "%^B_BLACK%^");
    PFto_ADD("B_LightCoral", "%^B_RED%^");
    PFto_ADD("B_LightCyan", "%^B_WHITE%^");
    PFto_ADD("B_LightCyan1", "%^B_WHITE%^");
    PFto_ADD("B_LightCyan2", "%^B_WHITE%^");
    PFto_ADD("B_LightCyan3", "%^B_WHITE%^");
    PFto_ADD("B_LightCyan4", "%^B_BLACK%^");
    PFto_ADD("B_LightGoldenrod", "%^B_YELLOW%^");
    PFto_ADD("B_LightGoldenrod1", "%^B_YELLOW%^");
    PFto_ADD("B_LightGoldenrod2", "%^B_YELLOW%^");
    PFto_ADD("B_LightGoldenrod3", "%^B_WHITE%^");
    PFto_ADD("B_LightGoldenrod4", "%^B_BLACK%^");
    PFto_ADD("B_LightGoldenrodYellow", "%^B_WHITE%^");
    PFto_ADD("B_LightGray", "%^B_WHITE%^");
    PFto_ADD("B_LightGreen", "%^B_WHITE%^");
    PFto_ADD("B_LightGrey", "%^B_WHITE%^");
    PFto_ADD("B_LightPink", "%^B_WHITE%^");
    PFto_ADD("B_LightPink1", "%^B_WHITE%^");
    PFto_ADD("B_LightPink2", "%^B_WHITE%^");
    PFto_ADD("B_LightPink3", "%^B_WHITE%^");
    PFto_ADD("B_LightPink4", "%^B_BLACK%^");
    PFto_ADD("B_LightSalmon", "%^B_RED%^");
    PFto_ADD("B_LightSalmon1", "%^B_RED%^");
    PFto_ADD("B_LightSalmon2", "%^B_RED%^");
    PFto_ADD("B_LightSalmon3", "%^B_RED%^");
    PFto_ADD("B_LightSalmon4", "%^B_BLACK%^");
    PFto_ADD("B_LightSeaGreen", "%^B_CYAN%^");
    PFto_ADD("B_LightSkyBlue", "%^B_CYAN%^");
    PFto_ADD("B_LightSkyBlue1", "%^B_WHITE%^");
    PFto_ADD("B_LightSkyBlue2", "%^B_WHITE%^");
    PFto_ADD("B_LightSkyBlue3", "%^B_WHITE%^");
    PFto_ADD("B_LightSkyBlue4", "%^B_BLACK%^");
    PFto_ADD("B_LightSlateBlue", "%^B_BLUE%^");
    PFto_ADD("B_LightSlateGray", "%^B_WHITE%^");
    PFto_ADD("B_LightSlateGrey", "%^B_WHITE%^");
    PFto_ADD("B_LightSteelBlue", "%^B_WHITE%^");
    PFto_ADD("B_LightSteelBlue1", "%^B_WHITE%^");
    PFto_ADD("B_LightSteelBlue2", "%^B_WHITE%^");
    PFto_ADD("B_LightSteelBlue3", "%^B_WHITE%^");
    PFto_ADD("B_LightSteelBlue4", "%^B_BLACK%^");
    PFto_ADD("B_LightYellow", "%^B_WHITE%^");
    PFto_ADD("B_LightYellow1", "%^B_WHITE%^");
    PFto_ADD("B_LightYellow2", "%^B_WHITE%^");
    PFto_ADD("B_LightYellow3", "%^B_WHITE%^");
    PFto_ADD("B_LightYellow4", "%^B_BLACK%^");
    PFto_ADD("B_LimeGreen", "%^B_GREEN%^");
    PFto_ADD("B_Linen", "%^B_WHITE%^");
    PFto_ADD("B_MAGENTA", "%^B_MAGENTA%^");
    PFto_ADD("B_Magenta", "%^B_MAGENTA%^");
    PFto_ADD("B_Magenta1", "%^B_MAGENTA%^");
    PFto_ADD("B_Magenta2", "%^B_MAGENTA%^");
    PFto_ADD("B_Magenta3", "%^B_MAGENTA%^");
    PFto_ADD("B_Magenta4", "%^B_MAGENTA%^");
    PFto_ADD("B_Maroon", "%^B_RED%^");
    PFto_ADD("B_Maroon1", "%^B_MAGENTA%^");
    PFto_ADD("B_Maroon2", "%^B_MAGENTA%^");
    PFto_ADD("B_Maroon3", "%^B_MAGENTA%^");
    PFto_ADD("B_Maroon4", "%^B_BLACK%^");
    PFto_ADD("B_MediumAquamarine", "%^B_WHITE%^");
    PFto_ADD("B_MediumBlue", "%^B_BLUE%^");
    PFto_ADD("B_MediumOrchid", "%^B_MAGENTA%^");
    PFto_ADD("B_MediumOrchid1", "%^B_MAGENTA%^");
    PFto_ADD("B_MediumOrchid2", "%^B_MAGENTA%^");
    PFto_ADD("B_MediumOrchid3", "%^B_MAGENTA%^");
    PFto_ADD("B_MediumOrchid4", "%^B_BLACK%^");
    PFto_ADD("B_MediumPurple", "%^B_BLUE%^");
    PFto_ADD("B_MediumPurple1", "%^B_WHITE%^");
    PFto_ADD("B_MediumPurple2", "%^B_BLUE%^");
    PFto_ADD("B_MediumPurple3", "%^B_BLUE%^");
    PFto_ADD("B_MediumPurple4", "%^B_BLACK%^");
    PFto_ADD("B_MediumSeaGreen", "%^B_GREEN%^");
    PFto_ADD("B_MediumSlateBlue", "%^B_BLUE%^");
    PFto_ADD("B_MediumSpringGreen", "%^B_CYAN%^");
    PFto_ADD("B_MediumTurquoise", "%^B_CYAN%^");
    PFto_ADD("B_MediumVioletRed", "%^B_MAGENTA%^");
    PFto_ADD("B_MidnightBlue", "%^B_BLUE%^");
    PFto_ADD("B_MintCream", "%^B_WHITE%^");
    PFto_ADD("B_MistyRose", "%^B_WHITE%^");
    PFto_ADD("B_MistyRose1", "%^B_WHITE%^");
    PFto_ADD("B_MistyRose2", "%^B_WHITE%^");
    PFto_ADD("B_MistyRose3", "%^B_WHITE%^");
    PFto_ADD("B_MistyRose4", "%^B_BLACK%^");
    PFto_ADD("B_Moccasin", "%^B_WHITE%^");
    PFto_ADD("B_NavajoWhite", "%^B_WHITE%^");
    PFto_ADD("B_NavajoWhite1", "%^B_WHITE%^");
    PFto_ADD("B_NavajoWhite2", "%^B_WHITE%^");
    PFto_ADD("B_NavajoWhite3", "%^B_WHITE%^");
    PFto_ADD("B_NavajoWhite4", "%^B_BLACK%^");
    PFto_ADD("B_Navy", "%^B_BLUE%^");
    PFto_ADD("B_NavyBlue", "%^B_BLUE%^");
    PFto_ADD("B_ORANGE", "%^B_ORANGE%^");
    PFto_ADD("B_OldLace", "%^B_WHITE%^");
    PFto_ADD("B_OliveDrab", "%^B_BLACK%^");
    PFto_ADD("B_OliveDrab1", "%^B_YELLOW%^");
    PFto_ADD("B_OliveDrab2", "%^B_ORANGE%^");
    PFto_ADD("B_OliveDrab3", "%^B_ORANGE%^");
    PFto_ADD("B_OliveDrab4", "%^B_BLACK%^");
    PFto_ADD("B_Orange", "%^B_ORANGE%^");
    PFto_ADD("B_Orange1", "%^B_ORANGE%^");
    PFto_ADD("B_Orange2", "%^B_ORANGE%^");
    PFto_ADD("B_Orange3", "%^B_ORANGE%^");
    PFto_ADD("B_Orange4", "%^B_BLACK%^");
    PFto_ADD("B_OrangeRed", "%^B_RED%^");
    PFto_ADD("B_OrangeRed1", "%^B_RED%^");
    PFto_ADD("B_OrangeRed2", "%^B_RED%^");
    PFto_ADD("B_OrangeRed3", "%^B_RED%^");
    PFto_ADD("B_OrangeRed4", "%^B_RED%^");
    PFto_ADD("B_Orchid", "%^B_MAGENTA%^");
    PFto_ADD("B_Orchid1", "%^B_MAGENTA%^");
    PFto_ADD("B_Orchid2", "%^B_MAGENTA%^");
    PFto_ADD("B_Orchid3", "%^B_MAGENTA%^");
    PFto_ADD("B_Orchid4", "%^B_BLACK%^");
    PFto_ADD("B_PINK", "%^B_MAGENTA%^");
    PFto_ADD("B_PaleGoldenrod", "%^B_WHITE%^");
    PFto_ADD("B_PaleGreen", "%^B_WHITE%^");
    PFto_ADD("B_PaleGreen1", "%^B_WHITE%^");
    PFto_ADD("B_PaleGreen2", "%^B_WHITE%^");
    PFto_ADD("B_PaleGreen3", "%^B_GREEN%^");
    PFto_ADD("B_PaleGreen4", "%^B_BLACK%^");
    PFto_ADD("B_PaleTurquoise", "%^B_WHITE%^");
    PFto_ADD("B_PaleTurquoise1", "%^B_WHITE%^");
    PFto_ADD("B_PaleTurquoise2", "%^B_WHITE%^");
    PFto_ADD("B_PaleTurquoise3", "%^B_WHITE%^");
    PFto_ADD("B_PaleTurquoise4", "%^B_BLACK%^");
    PFto_ADD("B_PaleVioletRed", "%^B_RED%^");
    PFto_ADD("B_PaleVioletRed1", "%^B_WHITE%^");
    PFto_ADD("B_PaleVioletRed2", "%^B_RED%^");
    PFto_ADD("B_PaleVioletRed3", "%^B_RED%^");
    PFto_ADD("B_PaleVioletRed4", "%^B_BLACK%^");
    PFto_ADD("B_PapayaWhip", "%^B_WHITE%^");
    PFto_ADD("B_PeachPuff", "%^B_WHITE%^");
    PFto_ADD("B_PeachPuff1", "%^B_WHITE%^");
    PFto_ADD("B_PeachPuff2", "%^B_WHITE%^");
    PFto_ADD("B_PeachPuff3", "%^B_WHITE%^");
    PFto_ADD("B_PeachPuff4", "%^B_BLACK%^");
    PFto_ADD("B_Peru", "%^B_RED%^");
    PFto_ADD("B_Pink", "%^B_WHITE%^");
    PFto_ADD("B_Pink1", "%^B_WHITE%^");
    PFto_ADD("B_Pink2", "%^B_WHITE%^");
    PFto_ADD("B_Pink3", "%^B_WHITE%^");
    PFto_ADD("B_Pink4", "%^B_BLACK%^");
    PFto_ADD("B_Plum", "%^B_WHITE%^");
    PFto_ADD("B_Plum1", "%^B_WHITE%^");
    PFto_ADD("B_Plum2", "%^B_WHITE%^");
    PFto_ADD("B_Plum3", "%^B_WHITE%^");
    PFto_ADD("B_Plum4", "%^B_BLACK%^");
    PFto_ADD("B_PowderBlue", "%^B_WHITE%^");
    PFto_ADD("B_Purple", "%^B_MAGENTA%^");
    PFto_ADD("B_Purple1", "%^B_BLUE%^");
    PFto_ADD("B_Purple2", "%^B_BLUE%^");
    PFto_ADD("B_Purple3", "%^B_MAGENTA%^");
    PFto_ADD("B_Purple4", "%^B_BLACK%^");
    PFto_ADD("B_RED", "%^B_RED%^");
    PFto_ADD("B_Red", "%^B_RED%^");
    PFto_ADD("B_Red1", "%^B_RED%^");
    PFto_ADD("B_Red2", "%^B_RED%^");
    PFto_ADD("B_Red3", "%^B_RED%^");
    PFto_ADD("B_Red4", "%^B_RED%^");
    PFto_ADD("B_RosyBrown", "%^B_WHITE%^");
    PFto_ADD("B_RosyBrown1", "%^B_WHITE%^");
    PFto_ADD("B_RosyBrown2", "%^B_WHITE%^");
    PFto_ADD("B_RosyBrown3", "%^B_WHITE%^");
    PFto_ADD("B_RosyBrown4", "%^B_BLACK%^");
    PFto_ADD("B_RoyalBlue", "%^B_BLUE%^");
    PFto_ADD("B_RoyalBlue1", "%^B_BLUE%^");
    PFto_ADD("B_RoyalBlue2", "%^B_BLUE%^");
    PFto_ADD("B_RoyalBlue3", "%^B_BLUE%^");
    PFto_ADD("B_RoyalBlue4", "%^B_BLACK%^");
    PFto_ADD("B_SaddleBrown", "%^B_RED%^");
    PFto_ADD("B_Salmon", "%^B_RED%^");
    PFto_ADD("B_Salmon1", "%^B_RED%^");
    PFto_ADD("B_Salmon2", "%^B_RED%^");
    PFto_ADD("B_Salmon3", "%^B_RED%^");
    PFto_ADD("B_Salmon4", "%^B_BLACK%^");
    PFto_ADD("B_SandyBrown", "%^B_RED%^");
    PFto_ADD("B_SeaGreen", "%^B_BLACK%^");
    PFto_ADD("B_SeaGreen1", "%^B_GREEN%^");
    PFto_ADD("B_SeaGreen2", "%^B_GREEN%^");
    PFto_ADD("B_SeaGreen3", "%^B_GREEN%^");
    PFto_ADD("B_SeaGreen4", "%^B_BLACK%^");
    PFto_ADD("B_Seashell", "%^B_WHITE%^");
    PFto_ADD("B_Seashell1", "%^B_WHITE%^");
    PFto_ADD("B_Seashell2", "%^B_WHITE%^");
    PFto_ADD("B_Seashell3", "%^B_WHITE%^");
    PFto_ADD("B_Seashell4", "%^B_BLACK%^");
    PFto_ADD("B_Sienna", "%^B_BLACK%^");
    PFto_ADD("B_Sienna1", "%^B_RED%^");
    PFto_ADD("B_Sienna2", "%^B_RED%^");
    PFto_ADD("B_Sienna3", "%^B_RED%^");
    PFto_ADD("B_Sienna4", "%^B_BLACK%^");
    PFto_ADD("B_SkyBlue", "%^B_CYAN%^");
    PFto_ADD("B_SkyBlue1", "%^B_CYAN%^");
    PFto_ADD("B_SkyBlue2", "%^B_CYAN%^");
    PFto_ADD("B_SkyBlue3", "%^B_WHITE%^");
    PFto_ADD("B_SkyBlue4", "%^B_BLACK%^");
    PFto_ADD("B_SlateBlue", "%^B_BLUE%^");
    PFto_ADD("B_SlateBlue1", "%^B_BLUE%^");
    PFto_ADD("B_SlateBlue2", "%^B_BLUE%^");
    PFto_ADD("B_SlateBlue3", "%^B_BLUE%^");
    PFto_ADD("B_SlateBlue4", "%^B_BLACK%^");
    PFto_ADD("B_SlateGray", "%^B_BLACK%^");
    PFto_ADD("B_SlateGray1", "%^B_WHITE%^");
    PFto_ADD("B_SlateGray2", "%^B_WHITE%^");
    PFto_ADD("B_SlateGray3", "%^B_WHITE%^");
    PFto_ADD("B_SlateGray4", "%^B_BLACK%^");
    PFto_ADD("B_SlateGrey", "%^B_BLACK%^");
    PFto_ADD("B_Snow", "%^B_WHITE%^");
    PFto_ADD("B_Snow1", "%^B_WHITE%^");
    PFto_ADD("B_Snow2", "%^B_WHITE%^");
    PFto_ADD("B_Snow3", "%^B_WHITE%^");
    PFto_ADD("B_Snow4", "%^B_WHITE%^");
    PFto_ADD("B_SpringGreen", "%^B_CYAN%^");
    PFto_ADD("B_SpringGreen1", "%^B_CYAN%^");
    PFto_ADD("B_SpringGreen2", "%^B_CYAN%^");
    PFto_ADD("B_SpringGreen3", "%^B_CYAN%^");
    PFto_ADD("B_SpringGreen4", "%^B_GREEN%^");
    PFto_ADD("B_SteelBlue", "%^B_BLUE%^");
    PFto_ADD("B_SteelBlue1", "%^B_CYAN%^");
    PFto_ADD("B_SteelBlue2", "%^B_CYAN%^");
    PFto_ADD("B_SteelBlue3", "%^B_BLUE%^");
    PFto_ADD("B_SteelBlue4", "%^B_BLACK%^");
    PFto_ADD("B_Tan", "%^B_WHITE%^");
    PFto_ADD("B_Tan1", "%^B_RED%^");
    PFto_ADD("B_Tan2", "%^B_RED%^");
    PFto_ADD("B_Tan3", "%^B_RED%^");
    PFto_ADD("B_Tan4", "%^B_BLACK%^");
    PFto_ADD("B_Thistle", "%^B_WHITE%^");
    PFto_ADD("B_Thistle1", "%^B_WHITE%^");
    PFto_ADD("B_Thistle2", "%^B_WHITE%^");
    PFto_ADD("B_Thistle3", "%^B_WHITE%^");
    PFto_ADD("B_Thistle4", "%^B_BLACK%^");
    PFto_ADD("B_Tomato", "%^B_RED%^");
    PFto_ADD("B_Tomato1", "%^B_RED%^");
    PFto_ADD("B_Tomato2", "%^B_RED%^");
    PFto_ADD("B_Tomato3", "%^B_RED%^");
    PFto_ADD("B_Tomato4", "%^B_BLACK%^");
    PFto_ADD("B_Turquoise", "%^B_CYAN%^");
    PFto_ADD("B_Turquoise1", "%^B_CYAN%^");
    PFto_ADD("B_Turquoise2", "%^B_CYAN%^");
    PFto_ADD("B_Turquoise3", "%^B_CYAN%^");
    PFto_ADD("B_Turquoise4", "%^B_CYAN%^");
    PFto_ADD("B_Violet", "%^B_MAGENTA%^");
    PFto_ADD("B_VioletRed", "%^B_MAGENTA%^");
    PFto_ADD("B_VioletRed1", "%^B_RED%^");
    PFto_ADD("B_VioletRed2", "%^B_RED%^");
    PFto_ADD("B_VioletRed3", "%^B_RED%^");
    PFto_ADD("B_VioletRed4", "%^B_BLACK%^");
    PFto_ADD("B_WHITE", "%^B_WHITE%^");
    PFto_ADD("B_Wheat", "%^B_WHITE%^");
    PFto_ADD("B_Wheat1", "%^B_WHITE%^");
    PFto_ADD("B_Wheat2", "%^B_WHITE%^");
    PFto_ADD("B_Wheat3", "%^B_WHITE%^");
    PFto_ADD("B_Wheat4", "%^B_BLACK%^");
    PFto_ADD("B_White", "%^B_WHITE%^");
    PFto_ADD("B_WhiteSmoke", "%^B_WHITE%^");
    PFto_ADD("B_YELLOW", "%^B_YELLOW%^");
    PFto_ADD("B_Yellow", "%^B_YELLOW%^");
    PFto_ADD("B_Yellow1", "%^B_YELLOW%^");
    PFto_ADD("B_Yellow2", "%^B_ORANGE%^");
    PFto_ADD("B_Yellow3", "%^B_ORANGE%^");
    PFto_ADD("B_Yellow4", "%^B_ORANGE%^");
    PFto_ADD("B_YellowGreen", "%^B_ORANGE%^");
    PFto_ADD("Beige", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Bisque", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Bisque1", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Bisque2", "%^WHITE%^");
    PFto_ADD("Bisque3", "%^WHITE%^");
    PFto_ADD("Bisque4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Black", "%^BLACK%^");
    PFto_ADD("BlanchedAlmond", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Blue", "%^BLUE%^");
    PFto_ADD("Blue1", "%^BLUE%^");
    PFto_ADD("Blue2", "%^BLUE%^");
    PFto_ADD("Blue3", "%^BLUE%^");
    PFto_ADD("Blue4", "%^BLUE%^");
    PFto_ADD("BlueViolet", "%^BOLD%^%^BLUE%^");
    PFto_ADD("Brown", "%^RED%^");
    PFto_ADD("Brown1", "%^BOLD%^%^RED%^");
    PFto_ADD("Brown2", "%^BOLD%^%^RED%^");
    PFto_ADD("Brown3", "%^BOLD%^%^RED%^");
    PFto_ADD("Brown4", "%^RED%^");
    PFto_ADD("Burlywood", "%^WHITE%^");
    PFto_ADD("Burlywood1", "%^WHITE%^");
    PFto_ADD("Burlywood2", "%^WHITE%^");
    PFto_ADD("Burlywood3", "%^WHITE%^");
    PFto_ADD("Burlywood4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("CLEARLINE", "%^CLEARLINE%^");
    PFto_ADD("CURS_DOWN", "%^CURS_DOWN%^");
    PFto_ADD("CURS_LEFT", "%^CURS_LEFT%^");
    PFto_ADD("CURS_RIGHT", "%^CURS_RIGHT%^");
    PFto_ADD("CURS_UP", "%^CURS_UP%^");
    PFto_ADD("CYAN", "%^CYAN%^");
    PFto_ADD("CadetBlue", "%^WHITE%^");
    PFto_ADD("CadetBlue1", "%^BOLD%^%^CYAN%^");
    PFto_ADD("CadetBlue2", "%^BOLD%^%^CYAN%^");
    PFto_ADD("CadetBlue3", "%^WHITE%^");
    PFto_ADD("CadetBlue4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Chartreuse", "%^ORANGE%^");
    PFto_ADD("Chartreuse1", "%^ORANGE%^");
    PFto_ADD("Chartreuse2", "%^ORANGE%^");
    PFto_ADD("Chartreuse3", "%^ORANGE%^");
    PFto_ADD("Chartreuse4", "%^GREEN%^");
    PFto_ADD("Chocolate", "%^BOLD%^%^RED%^");
    PFto_ADD("Chocolate1", "%^BOLD%^%^RED%^");
    PFto_ADD("Chocolate2", "%^BOLD%^%^RED%^");
    PFto_ADD("Chocolate3", "%^BOLD%^%^RED%^");
    PFto_ADD("Chocolate4", "%^RED%^");
    PFto_ADD("Coral", "%^BOLD%^%^RED%^");
    PFto_ADD("Coral1", "%^BOLD%^%^RED%^");
    PFto_ADD("Coral2", "%^BOLD%^%^RED%^");
    PFto_ADD("Coral3", "%^BOLD%^%^RED%^");
    PFto_ADD("Coral4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("CornflowerBlue", "%^BOLD%^%^BLUE%^");
    PFto_ADD("Cornsilk", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Cornsilk1", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Cornsilk2", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Cornsilk3", "%^WHITE%^");
    PFto_ADD("Cornsilk4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Cyan", "%^BOLD%^%^CYAN%^");
    PFto_ADD("Cyan1", "%^BOLD%^%^CYAN%^");
    PFto_ADD("Cyan2", "%^CYAN%^");
    PFto_ADD("Cyan3", "%^CYAN%^");
    PFto_ADD("Cyan4", "%^CYAN%^");
    PFto_ADD("DARKGREY", "%^BOLD%^%^BLACK%^");
    PFto_ADD("DarkBlue", "%^BLUE%^");
    PFto_ADD("DarkCyan", "%^CYAN%^");
    PFto_ADD("DarkGoldenrod", "%^ORANGE%^");
    PFto_ADD("DarkGoldenrod1", "%^ORANGE%^");
    PFto_ADD("DarkGoldenrod2", "%^ORANGE%^");
    PFto_ADD("DarkGoldenrod3", "%^ORANGE%^");
    PFto_ADD("DarkGoldenrod4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("DarkGray", "%^WHITE%^");
    PFto_ADD("DarkGreen", "%^GREEN%^");
    PFto_ADD("DarkGrey", "%^WHITE%^");
    PFto_ADD("DarkKhaki", "%^WHITE%^");
    PFto_ADD("DarkMagenta", "%^MAGENTA%^");
    PFto_ADD("DarkOliveGreen", "%^BOLD%^%^BLACK%^");
    PFto_ADD("DarkOliveGreen1", "%^YELLOW%^");
    PFto_ADD("DarkOliveGreen2", "%^YELLOW%^");
    PFto_ADD("DarkOliveGreen3", "%^BOLD%^%^GREEN%^");
    PFto_ADD("DarkOliveGreen4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("DarkOrange", "%^ORANGE%^");
    PFto_ADD("DarkOrange1", "%^ORANGE%^");
    PFto_ADD("DarkOrange2", "%^ORANGE%^");
    PFto_ADD("DarkOrange3", "%^ORANGE%^");
    PFto_ADD("DarkOrange4", "%^RED%^");
    PFto_ADD("DarkOrchid", "%^MAGENTA%^");
    PFto_ADD("DarkOrchid1", "%^BOLD%^%^MAGENTA%^");
    PFto_ADD("DarkOrchid2", "%^MAGENTA%^");
    PFto_ADD("DarkOrchid3", "%^MAGENTA%^");
    PFto_ADD("DarkOrchid4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("DarkRed", "%^RED%^");
    PFto_ADD("DarkSalmon", "%^BOLD%^%^RED%^");
    PFto_ADD("DarkSeaGreen", "%^WHITE%^");
    PFto_ADD("DarkSeaGreen1", "%^WHITE%^");
    PFto_ADD("DarkSeaGreen2", "%^WHITE%^");
    PFto_ADD("DarkSeaGreen3", "%^WHITE%^");
    PFto_ADD("DarkSeaGreen4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("DarkSlateBlue", "%^BOLD%^%^BLACK%^");
    PFto_ADD("DarkSlateGray", "%^BOLD%^%^BLACK%^");
    PFto_ADD("DarkSlateGray1", "%^BOLD%^%^CYAN%^");
    PFto_ADD("DarkSlateGray2", "%^BOLD%^%^CYAN%^");
    PFto_ADD("DarkSlateGray3", "%^WHITE%^");
    PFto_ADD("DarkSlateGray4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("DarkSlateGrey", "%^BOLD%^%^BLACK%^");
    PFto_ADD("DarkTurquoise", "%^CYAN%^");
    PFto_ADD("DarkViolet", "%^MAGENTA%^");
    PFto_ADD("DeepPink", "%^MAGENTA%^");
    PFto_ADD("DeepPink1", "%^MAGENTA%^");
    PFto_ADD("DeepPink2", "%^MAGENTA%^");
    PFto_ADD("DeepPink3", "%^MAGENTA%^");
    PFto_ADD("DeepPink4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("DeepSkyBlue", "%^CYAN%^");
    PFto_ADD("DeepSkyBlue1", "%^CYAN%^");
    PFto_ADD("DeepSkyBlue2", "%^CYAN%^");
    PFto_ADD("DeepSkyBlue3", "%^CYAN%^");
    PFto_ADD("DeepSkyBlue4", "%^CYAN%^");
    PFto_ADD("DimGray", "%^BOLD%^%^BLACK%^");
    PFto_ADD("DimGrey", "%^BOLD%^%^BLACK%^");
    PFto_ADD("DodgerBlue", "%^BOLD%^%^BLUE%^");
    PFto_ADD("DodgerBlue1", "%^BOLD%^%^BLUE%^");
    PFto_ADD("DodgerBlue2", "%^BOLD%^%^BLUE%^");
    PFto_ADD("DodgerBlue3", "%^CYAN%^");
    PFto_ADD("DodgerBlue4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("ENDTERM", "%^ENDTERM%^");
    PFto_ADD("F000", "%^BLACK%^");
    PFto_ADD("F001", "%^BLACK%^");
    PFto_ADD("F002", "%^BLUE%^");
    PFto_ADD("F003", "%^BLUE%^");
    PFto_ADD("F004", "%^BLUE%^");
    PFto_ADD("F005", "%^BLUE%^");
    PFto_ADD("F010", "%^BLACK%^");
    PFto_ADD("F011", "%^BOLD%^%^BLACK%^");
    PFto_ADD("F012", "%^BLUE%^");
    PFto_ADD("F013", "%^BLUE%^");
    PFto_ADD("F014", "%^BLUE%^");
    PFto_ADD("F015", "%^BOLD%^%^BLUE%^");
    PFto_ADD("F020", "%^GREEN%^");
    PFto_ADD("F021", "%^GREEN%^");
    PFto_ADD("F022", "%^CYAN%^");
    PFto_ADD("F023", "%^CYAN%^");
    PFto_ADD("F024", "%^CYAN%^");
    PFto_ADD("F025", "%^CYAN%^");
    PFto_ADD("F030", "%^GREEN%^");
    PFto_ADD("F031", "%^GREEN%^");
    PFto_ADD("F032", "%^CYAN%^");
    PFto_ADD("F033", "%^CYAN%^");
    PFto_ADD("F034", "%^CYAN%^");
    PFto_ADD("F035", "%^CYAN%^");
    PFto_ADD("F040", "%^GREEN%^");
    PFto_ADD("F041", "%^GREEN%^");
    PFto_ADD("F042", "%^CYAN%^");
    PFto_ADD("F043", "%^CYAN%^");
    PFto_ADD("F044", "%^CYAN%^");
    PFto_ADD("F045", "%^CYAN%^");
    PFto_ADD("F050", "%^GREEN%^");
    PFto_ADD("F051", "%^BOLD%^%^GREEN%^");
    PFto_ADD("F052", "%^CYAN%^");
    PFto_ADD("F053", "%^CYAN%^");
    PFto_ADD("F054", "%^CYAN%^");
    PFto_ADD("F055", "%^BOLD%^%^CYAN%^");
    PFto_ADD("F100", "%^BLACK%^");
    PFto_ADD("F101", "%^BOLD%^%^BLACK%^");
    PFto_ADD("F102", "%^BLUE%^");
    PFto_ADD("F103", "%^BLUE%^");
    PFto_ADD("F104", "%^BLUE%^");
    PFto_ADD("F105", "%^BOLD%^%^BLUE%^");
    PFto_ADD("F110", "%^BOLD%^%^BLACK%^");
    PFto_ADD("F111", "%^BOLD%^%^BLACK%^");
    PFto_ADD("F112", "%^BOLD%^%^BLACK%^");
    PFto_ADD("F113", "%^BOLD%^%^BLUE%^");
    PFto_ADD("F114", "%^BOLD%^%^BLUE%^");
    PFto_ADD("F115", "%^BOLD%^%^BLUE%^");
    PFto_ADD("F120", "%^GREEN%^");
    PFto_ADD("F121", "%^BOLD%^%^BLACK%^");
    PFto_ADD("F122", "%^BOLD%^%^BLACK%^");
    PFto_ADD("F123", "%^BOLD%^%^BLUE%^");
    PFto_ADD("F124", "%^BOLD%^%^BLUE%^");
    PFto_ADD("F125", "%^BOLD%^%^BLUE%^");
    PFto_ADD("F130", "%^GREEN%^");
    PFto_ADD("F131", "%^BOLD%^%^GREEN%^");
    PFto_ADD("F132", "%^BOLD%^%^GREEN%^");
    PFto_ADD("F133", "%^CYAN%^");
    PFto_ADD("F134", "%^BOLD%^%^CYAN%^");
    PFto_ADD("F135", "%^BOLD%^%^CYAN%^");
    PFto_ADD("F140", "%^GREEN%^");
    PFto_ADD("F141", "%^BOLD%^%^GREEN%^");
    PFto_ADD("F142", "%^BOLD%^%^GREEN%^");
    PFto_ADD("F143", "%^BOLD%^%^CYAN%^");
    PFto_ADD("F144", "%^BOLD%^%^CYAN%^");
    PFto_ADD("F145", "%^BOLD%^%^CYAN%^");
    PFto_ADD("F150", "%^BOLD%^%^GREEN%^");
    PFto_ADD("F151", "%^BOLD%^%^GREEN%^");
    PFto_ADD("F152", "%^BOLD%^%^GREEN%^");
    PFto_ADD("F153", "%^BOLD%^%^CYAN%^");
    PFto_ADD("F154", "%^BOLD%^%^CYAN%^");
    PFto_ADD("F155", "%^BOLD%^%^CYAN%^");
    PFto_ADD("F200", "%^RED%^");
    PFto_ADD("F201", "%^RED%^");
    PFto_ADD("F202", "%^MAGENTA%^");
    PFto_ADD("F203", "%^MAGENTA%^");
    PFto_ADD("F204", "%^MAGENTA%^");
    PFto_ADD("F205", "%^MAGENTA%^");
    PFto_ADD("F210", "%^RED%^");
    PFto_ADD("F211", "%^BOLD%^%^BLACK%^");
    PFto_ADD("F212", "%^BOLD%^%^BLACK%^");
    PFto_ADD("F213", "%^BOLD%^%^BLUE%^");
    PFto_ADD("F214", "%^BOLD%^%^BLUE%^");
    PFto_ADD("F215", "%^BOLD%^%^BLUE%^");
    PFto_ADD("F220", "%^ORANGE%^");
    PFto_ADD("F221", "%^BOLD%^%^BLACK%^");
    PFto_ADD("F222", "%^WHITE%^");
    PFto_ADD("F223", "%^WHITE%^");
    PFto_ADD("F224", "%^WHITE%^");
    PFto_ADD("F225", "%^BOLD%^%^BLUE%^");
    PFto_ADD("F230", "%^ORANGE%^");
    PFto_ADD("F231", "%^BOLD%^%^GREEN%^");
    PFto_ADD("F232", "%^WHITE%^");
    PFto_ADD("F233", "%^WHITE%^");
    PFto_ADD("F234", "%^WHITE%^");
    PFto_ADD("F235", "%^WHITE%^");
    PFto_ADD("F240", "%^ORANGE%^");
    PFto_ADD("F241", "%^BOLD%^%^GREEN%^");
    PFto_ADD("F242", "%^WHITE%^");
    PFto_ADD("F243", "%^WHITE%^");
    PFto_ADD("F244", "%^WHITE%^");
    PFto_ADD("F245", "%^BOLD%^%^CYAN%^");
    PFto_ADD("F250", "%^ORANGE%^");
    PFto_ADD("F251", "%^BOLD%^%^GREEN%^");
    PFto_ADD("F252", "%^BOLD%^%^GREEN%^");
    PFto_ADD("F253", "%^WHITE%^");
    PFto_ADD("F254", "%^BOLD%^%^CYAN%^");
    PFto_ADD("F255", "%^BOLD%^%^CYAN%^");
    PFto_ADD("F300", "%^RED%^");
    PFto_ADD("F301", "%^RED%^");
    PFto_ADD("F302", "%^MAGENTA%^");
    PFto_ADD("F303", "%^MAGENTA%^");
    PFto_ADD("F304", "%^MAGENTA%^");
    PFto_ADD("F305", "%^MAGENTA%^");
    PFto_ADD("F310", "%^RED%^");
    PFto_ADD("F311", "%^BOLD%^%^RED%^");
    PFto_ADD("F312", "%^BOLD%^%^RED%^");
    PFto_ADD("F313", "%^MAGENTA%^");
    PFto_ADD("F314", "%^BOLD%^%^MAGENTA%^");
    PFto_ADD("F315", "%^BOLD%^%^MAGENTA%^");
    PFto_ADD("F320", "%^ORANGE%^");
    PFto_ADD("F321", "%^BOLD%^%^RED%^");
    PFto_ADD("F322", "%^WHITE%^");
    PFto_ADD("F323", "%^WHITE%^");
    PFto_ADD("F324", "%^WHITE%^");
    PFto_ADD("F325", "%^WHITE%^");
    PFto_ADD("F330", "%^ORANGE%^");
    PFto_ADD("F331", "%^ORANGE%^");
    PFto_ADD("F332", "%^WHITE%^");
    PFto_ADD("F333", "%^WHITE%^");
    PFto_ADD("F334", "%^WHITE%^");
    PFto_ADD("F335", "%^WHITE%^");
    PFto_ADD("F340", "%^ORANGE%^");
    PFto_ADD("F341", "%^YELLOW%^");
    PFto_ADD("F342", "%^WHITE%^");
    PFto_ADD("F343", "%^WHITE%^");
    PFto_ADD("F344", "%^WHITE%^");
    PFto_ADD("F345", "%^WHITE%^");
    PFto_ADD("F350", "%^ORANGE%^");
    PFto_ADD("F351", "%^YELLOW%^");
    PFto_ADD("F352", "%^WHITE%^");
    PFto_ADD("F353", "%^WHITE%^");
    PFto_ADD("F354", "%^WHITE%^");
    PFto_ADD("F355", "%^BOLD%^%^WHITE%^");
    PFto_ADD("F400", "%^RED%^");
    PFto_ADD("F401", "%^RED%^");
    PFto_ADD("F402", "%^MAGENTA%^");
    PFto_ADD("F403", "%^MAGENTA%^");
    PFto_ADD("F404", "%^MAGENTA%^");
    PFto_ADD("F405", "%^MAGENTA%^");
    PFto_ADD("F410", "%^RED%^");
    PFto_ADD("F411", "%^BOLD%^%^RED%^");
    PFto_ADD("F412", "%^BOLD%^%^RED%^");
    PFto_ADD("F413", "%^BOLD%^%^MAGENTA%^");
    PFto_ADD("F414", "%^BOLD%^%^MAGENTA%^");
    PFto_ADD("F415", "%^BOLD%^%^MAGENTA%^");
    PFto_ADD("F420", "%^ORANGE%^");
    PFto_ADD("F421", "%^BOLD%^%^RED%^");
    PFto_ADD("F422", "%^WHITE%^");
    PFto_ADD("F423", "%^WHITE%^");
    PFto_ADD("F424", "%^WHITE%^");
    PFto_ADD("F425", "%^BOLD%^%^MAGENTA%^");
    PFto_ADD("F430", "%^ORANGE%^");
    PFto_ADD("F431", "%^YELLOW%^");
    PFto_ADD("F432", "%^WHITE%^");
    PFto_ADD("F433", "%^WHITE%^");
    PFto_ADD("F434", "%^WHITE%^");
    PFto_ADD("F435", "%^WHITE%^");
    PFto_ADD("F440", "%^ORANGE%^");
    PFto_ADD("F441", "%^YELLOW%^");
    PFto_ADD("F442", "%^WHITE%^");
    PFto_ADD("F443", "%^WHITE%^");
    PFto_ADD("F444", "%^WHITE%^");
    PFto_ADD("F445", "%^BOLD%^%^WHITE%^");
    PFto_ADD("F450", "%^ORANGE%^");
    PFto_ADD("F451", "%^YELLOW%^");
    PFto_ADD("F452", "%^YELLOW%^");
    PFto_ADD("F453", "%^WHITE%^");
    PFto_ADD("F454", "%^BOLD%^%^WHITE%^");
    PFto_ADD("F455", "%^BOLD%^%^WHITE%^");
    PFto_ADD("F500", "%^RED%^");
    PFto_ADD("F501", "%^BOLD%^%^RED%^");
    PFto_ADD("F502", "%^MAGENTA%^");
    PFto_ADD("F503", "%^MAGENTA%^");
    PFto_ADD("F504", "%^MAGENTA%^");
    PFto_ADD("F505", "%^BOLD%^%^MAGENTA%^");
    PFto_ADD("F510", "%^BOLD%^%^RED%^");
    PFto_ADD("F511", "%^BOLD%^%^RED%^");
    PFto_ADD("F512", "%^BOLD%^%^RED%^");
    PFto_ADD("F513", "%^BOLD%^%^MAGENTA%^");
    PFto_ADD("F514", "%^BOLD%^%^MAGENTA%^");
    PFto_ADD("F515", "%^BOLD%^%^MAGENTA%^");
    PFto_ADD("F520", "%^ORANGE%^");
    PFto_ADD("F521", "%^BOLD%^%^RED%^");
    PFto_ADD("F522", "%^BOLD%^%^RED%^");
    PFto_ADD("F523", "%^WHITE%^");
    PFto_ADD("F524", "%^BOLD%^%^MAGENTA%^");
    PFto_ADD("F525", "%^BOLD%^%^MAGENTA%^");
    PFto_ADD("F530", "%^ORANGE%^");
    PFto_ADD("F531", "%^YELLOW%^");
    PFto_ADD("F532", "%^WHITE%^");
    PFto_ADD("F533", "%^WHITE%^");
    PFto_ADD("F534", "%^WHITE%^");
    PFto_ADD("F535", "%^BOLD%^%^WHITE%^");
    PFto_ADD("F540", "%^ORANGE%^");
    PFto_ADD("F541", "%^YELLOW%^");
    PFto_ADD("F542", "%^YELLOW%^");
    PFto_ADD("F543", "%^WHITE%^");
    PFto_ADD("F544", "%^BOLD%^%^WHITE%^");
    PFto_ADD("F545", "%^BOLD%^%^WHITE%^");
    PFto_ADD("F550", "%^YELLOW%^");
    PFto_ADD("F551", "%^YELLOW%^");
    PFto_ADD("F552", "%^YELLOW%^");
    PFto_ADD("F553", "%^BOLD%^%^WHITE%^");
    PFto_ADD("F554", "%^BOLD%^%^WHITE%^");
    PFto_ADD("F555", "%^BOLD%^%^WHITE%^");
    PFto_ADD("FLASH", "%^FLASH%^");
    PFto_ADD("Firebrick", "%^RED%^");
    PFto_ADD("Firebrick1", "%^BOLD%^%^RED%^");
    PFto_ADD("Firebrick2", "%^BOLD%^%^RED%^");
    PFto_ADD("Firebrick3", "%^RED%^");
    PFto_ADD("Firebrick4", "%^RED%^");
    PFto_ADD("FloralWhite", "%^BOLD%^%^WHITE%^");
    PFto_ADD("ForestGreen", "%^GREEN%^");
    PFto_ADD("G00", "%^BLACK%^");
    PFto_ADD("G01", "%^BLACK%^");
    PFto_ADD("G02", "%^BLACK%^");
    PFto_ADD("G03", "%^BLACK%^");
    PFto_ADD("G04", "%^BLACK%^");
    PFto_ADD("G05", "%^BOLD%^%^BLACK%^");
    PFto_ADD("G06", "%^BOLD%^%^BLACK%^");
    PFto_ADD("G07", "%^BOLD%^%^BLACK%^");
    PFto_ADD("G08", "%^BOLD%^%^BLACK%^");
    PFto_ADD("G09", "%^BOLD%^%^BLACK%^");
    PFto_ADD("G10", "%^BOLD%^%^BLACK%^");
    PFto_ADD("G11", "%^BOLD%^%^BLACK%^");
    PFto_ADD("G12", "%^BOLD%^%^BLACK%^");
    PFto_ADD("G13", "%^BOLD%^%^BLACK%^");
    PFto_ADD("G14", "%^WHITE%^");
    PFto_ADD("G15", "%^WHITE%^");
    PFto_ADD("G16", "%^WHITE%^");
    PFto_ADD("G17", "%^WHITE%^");
    PFto_ADD("G18", "%^WHITE%^");
    PFto_ADD("G19", "%^WHITE%^");
    PFto_ADD("G20", "%^WHITE%^");
    PFto_ADD("G21", "%^WHITE%^");
    PFto_ADD("G22", "%^WHITE%^");
    PFto_ADD("G23", "%^BOLD%^%^WHITE%^");
    PFto_ADD("G24", "%^BOLD%^%^WHITE%^");
    PFto_ADD("G25", "%^BOLD%^%^WHITE%^");
    PFto_ADD("GREEN", "%^GREEN%^");
    PFto_ADD("GREY", "%^WHITE%^");
    PFto_ADD("Gainsboro", "%^WHITE%^");
    PFto_ADD("GhostWhite", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Gold", "%^ORANGE%^");
    PFto_ADD("Gold1", "%^ORANGE%^");
    PFto_ADD("Gold2", "%^ORANGE%^");
    PFto_ADD("Gold3", "%^ORANGE%^");
    PFto_ADD("Gold4", "%^ORANGE%^");
    PFto_ADD("Goldenrod", "%^ORANGE%^");
    PFto_ADD("Goldenrod1", "%^ORANGE%^");
    PFto_ADD("Goldenrod2", "%^ORANGE%^");
    PFto_ADD("Goldenrod3", "%^ORANGE%^");
    PFto_ADD("Goldenrod4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Gray", "%^WHITE%^");
    PFto_ADD("Gray0", "%^BLACK%^");
    PFto_ADD("Gray1", "%^BLACK%^");
    PFto_ADD("Gray10", "%^BLACK%^");
    PFto_ADD("Gray100", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Gray11", "%^BLACK%^");
    PFto_ADD("Gray12", "%^BLACK%^");
    PFto_ADD("Gray13", "%^BLACK%^");
    PFto_ADD("Gray14", "%^BLACK%^");
    PFto_ADD("Gray15", "%^BLACK%^");
    PFto_ADD("Gray16", "%^BLACK%^");
    PFto_ADD("Gray17", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Gray18", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Gray19", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Gray2", "%^BLACK%^");
    PFto_ADD("Gray20", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Gray21", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Gray22", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Gray23", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Gray24", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Gray25", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Gray26", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Gray27", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Gray28", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Gray29", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Gray3", "%^BLACK%^");
    PFto_ADD("Gray30", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Gray31", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Gray32", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Gray33", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Gray34", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Gray35", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Gray36", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Gray37", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Gray38", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Gray39", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Gray4", "%^BLACK%^");
    PFto_ADD("Gray40", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Gray41", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Gray42", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Gray43", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Gray44", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Gray45", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Gray46", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Gray47", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Gray48", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Gray49", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Gray5", "%^BLACK%^");
    PFto_ADD("Gray50", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Gray51", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Gray52", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Gray53", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Gray54", "%^WHITE%^");
    PFto_ADD("Gray55", "%^WHITE%^");
    PFto_ADD("Gray56", "%^WHITE%^");
    PFto_ADD("Gray57", "%^WHITE%^");
    PFto_ADD("Gray58", "%^WHITE%^");
    PFto_ADD("Gray59", "%^WHITE%^");
    PFto_ADD("Gray6", "%^BLACK%^");
    PFto_ADD("Gray60", "%^WHITE%^");
    PFto_ADD("Gray61", "%^WHITE%^");
    PFto_ADD("Gray62", "%^WHITE%^");
    PFto_ADD("Gray63", "%^WHITE%^");
    PFto_ADD("Gray64", "%^WHITE%^");
    PFto_ADD("Gray65", "%^WHITE%^");
    PFto_ADD("Gray66", "%^WHITE%^");
    PFto_ADD("Gray67", "%^WHITE%^");
    PFto_ADD("Gray68", "%^WHITE%^");
    PFto_ADD("Gray69", "%^WHITE%^");
    PFto_ADD("Gray7", "%^BLACK%^");
    PFto_ADD("Gray70", "%^WHITE%^");
    PFto_ADD("Gray71", "%^WHITE%^");
    PFto_ADD("Gray72", "%^WHITE%^");
    PFto_ADD("Gray73", "%^WHITE%^");
    PFto_ADD("Gray74", "%^WHITE%^");
    PFto_ADD("Gray75", "%^WHITE%^");
    PFto_ADD("Gray76", "%^WHITE%^");
    PFto_ADD("Gray77", "%^WHITE%^");
    PFto_ADD("Gray78", "%^WHITE%^");
    PFto_ADD("Gray79", "%^WHITE%^");
    PFto_ADD("Gray8", "%^BLACK%^");
    PFto_ADD("Gray80", "%^WHITE%^");
    PFto_ADD("Gray81", "%^WHITE%^");
    PFto_ADD("Gray82", "%^WHITE%^");
    PFto_ADD("Gray83", "%^WHITE%^");
    PFto_ADD("Gray84", "%^WHITE%^");
    PFto_ADD("Gray85", "%^WHITE%^");
    PFto_ADD("Gray86", "%^WHITE%^");
    PFto_ADD("Gray87", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Gray88", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Gray89", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Gray9", "%^BLACK%^");
    PFto_ADD("Gray90", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Gray91", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Gray92", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Gray93", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Gray94", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Gray95", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Gray96", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Gray97", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Gray98", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Gray99", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Green", "%^GREEN%^");
    PFto_ADD("Green1", "%^GREEN%^");
    PFto_ADD("Green2", "%^GREEN%^");
    PFto_ADD("Green3", "%^GREEN%^");
    PFto_ADD("Green4", "%^GREEN%^");
    PFto_ADD("GreenYellow", "%^ORANGE%^");
    PFto_ADD("Grey", "%^WHITE%^");
    PFto_ADD("Grey0", "%^BLACK%^");
    PFto_ADD("Grey1", "%^BLACK%^");
    PFto_ADD("Grey10", "%^BLACK%^");
    PFto_ADD("Grey100", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Grey11", "%^BLACK%^");
    PFto_ADD("Grey12", "%^BLACK%^");
    PFto_ADD("Grey13", "%^BLACK%^");
    PFto_ADD("Grey14", "%^BLACK%^");
    PFto_ADD("Grey15", "%^BLACK%^");
    PFto_ADD("Grey16", "%^BLACK%^");
    PFto_ADD("Grey17", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Grey18", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Grey19", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Grey2", "%^BLACK%^");
    PFto_ADD("Grey20", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Grey21", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Grey22", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Grey23", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Grey24", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Grey25", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Grey26", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Grey27", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Grey28", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Grey29", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Grey3", "%^BLACK%^");
    PFto_ADD("Grey30", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Grey31", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Grey32", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Grey33", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Grey34", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Grey35", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Grey36", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Grey37", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Grey38", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Grey39", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Grey4", "%^BLACK%^");
    PFto_ADD("Grey40", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Grey41", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Grey42", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Grey43", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Grey44", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Grey45", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Grey46", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Grey47", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Grey48", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Grey49", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Grey5", "%^BLACK%^");
    PFto_ADD("Grey50", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Grey51", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Grey52", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Grey53", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Grey54", "%^WHITE%^");
    PFto_ADD("Grey55", "%^WHITE%^");
    PFto_ADD("Grey56", "%^WHITE%^");
    PFto_ADD("Grey57", "%^WHITE%^");
    PFto_ADD("Grey58", "%^WHITE%^");
    PFto_ADD("Grey59", "%^WHITE%^");
    PFto_ADD("Grey6", "%^BLACK%^");
    PFto_ADD("Grey60", "%^WHITE%^");
    PFto_ADD("Grey61", "%^WHITE%^");
    PFto_ADD("Grey62", "%^WHITE%^");
    PFto_ADD("Grey63", "%^WHITE%^");
    PFto_ADD("Grey64", "%^WHITE%^");
    PFto_ADD("Grey65", "%^WHITE%^");
    PFto_ADD("Grey66", "%^WHITE%^");
    PFto_ADD("Grey67", "%^WHITE%^");
    PFto_ADD("Grey68", "%^WHITE%^");
    PFto_ADD("Grey69", "%^WHITE%^");
    PFto_ADD("Grey7", "%^BLACK%^");
    PFto_ADD("Grey70", "%^WHITE%^");
    PFto_ADD("Grey71", "%^WHITE%^");
    PFto_ADD("Grey72", "%^WHITE%^");
    PFto_ADD("Grey73", "%^WHITE%^");
    PFto_ADD("Grey74", "%^WHITE%^");
    PFto_ADD("Grey75", "%^WHITE%^");
    PFto_ADD("Grey76", "%^WHITE%^");
    PFto_ADD("Grey77", "%^WHITE%^");
    PFto_ADD("Grey78", "%^WHITE%^");
    PFto_ADD("Grey79", "%^WHITE%^");
    PFto_ADD("Grey8", "%^BLACK%^");
    PFto_ADD("Grey80", "%^WHITE%^");
    PFto_ADD("Grey81", "%^WHITE%^");
    PFto_ADD("Grey82", "%^WHITE%^");
    PFto_ADD("Grey83", "%^WHITE%^");
    PFto_ADD("Grey84", "%^WHITE%^");
    PFto_ADD("Grey85", "%^WHITE%^");
    PFto_ADD("Grey86", "%^WHITE%^");
    PFto_ADD("Grey87", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Grey88", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Grey89", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Grey9", "%^BLACK%^");
    PFto_ADD("Grey90", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Grey91", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Grey92", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Grey93", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Grey94", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Grey95", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Grey96", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Grey97", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Grey98", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Grey99", "%^BOLD%^%^WHITE%^");
    PFto_ADD("HOME", "%^HOME%^");
    PFto_ADD("Honeydew", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Honeydew1", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Honeydew2", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Honeydew3", "%^WHITE%^");
    PFto_ADD("Honeydew4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("HotPink", "%^BOLD%^%^MAGENTA%^");
    PFto_ADD("HotPink1", "%^BOLD%^%^MAGENTA%^");
    PFto_ADD("HotPink2", "%^BOLD%^%^RED%^");
    PFto_ADD("HotPink3", "%^BOLD%^%^RED%^");
    PFto_ADD("HotPink4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("INITTERM", "%^INITTERM%^");
    PFto_ADD("ITALIC", "%^ITALIC%^");
    PFto_ADD("IndianRed", "%^BOLD%^%^RED%^");
    PFto_ADD("IndianRed1", "%^BOLD%^%^RED%^");
    PFto_ADD("IndianRed2", "%^BOLD%^%^RED%^");
    PFto_ADD("IndianRed3", "%^BOLD%^%^RED%^");
    PFto_ADD("IndianRed4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Ivory", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Ivory1", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Ivory2", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Ivory3", "%^WHITE%^");
    PFto_ADD("Ivory4", "%^WHITE%^");
    PFto_ADD("Khaki", "%^YELLOW%^");
    PFto_ADD("Khaki1", "%^YELLOW%^");
    PFto_ADD("Khaki2", "%^YELLOW%^");
    PFto_ADD("Khaki3", "%^WHITE%^");
    PFto_ADD("Khaki4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("LIGHTBLUE", "%^BOLD%^%^BLUE%^");
    PFto_ADD("LIGHTCYAN", "%^BOLD%^%^CYAN%^");
    PFto_ADD("LIGHTGREEN", "%^BOLD%^%^GREEN%^");
    PFto_ADD("LIGHTRED", "%^BOLD%^%^RED%^");
    PFto_ADD("Lavender", "%^BOLD%^%^WHITE%^");
    PFto_ADD("LavenderBlush", "%^BOLD%^%^WHITE%^");
    PFto_ADD("LavenderBlush1", "%^BOLD%^%^WHITE%^");
    PFto_ADD("LavenderBlush2", "%^BOLD%^%^WHITE%^");
    PFto_ADD("LavenderBlush3", "%^WHITE%^");
    PFto_ADD("LavenderBlush4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("LawnGreen", "%^ORANGE%^");
    PFto_ADD("LemonChiffon", "%^BOLD%^%^WHITE%^");
    PFto_ADD("LemonChiffon1", "%^BOLD%^%^WHITE%^");
    PFto_ADD("LemonChiffon2", "%^WHITE%^");
    PFto_ADD("LemonChiffon3", "%^WHITE%^");
    PFto_ADD("LemonChiffon4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("LightBlue", "%^WHITE%^");
    PFto_ADD("LightBlue1", "%^BOLD%^%^WHITE%^");
    PFto_ADD("LightBlue2", "%^WHITE%^");
    PFto_ADD("LightBlue3", "%^WHITE%^");
    PFto_ADD("LightBlue4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("LightCoral", "%^BOLD%^%^RED%^");
    PFto_ADD("LightCyan", "%^BOLD%^%^WHITE%^");
    PFto_ADD("LightCyan1", "%^BOLD%^%^WHITE%^");
    PFto_ADD("LightCyan2", "%^BOLD%^%^WHITE%^");
    PFto_ADD("LightCyan3", "%^WHITE%^");
    PFto_ADD("LightCyan4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("LightGoldenrod", "%^YELLOW%^");
    PFto_ADD("LightGoldenrod1", "%^YELLOW%^");
    PFto_ADD("LightGoldenrod2", "%^YELLOW%^");
    PFto_ADD("LightGoldenrod3", "%^WHITE%^");
    PFto_ADD("LightGoldenrod4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("LightGoldenrodYellow", "%^BOLD%^%^WHITE%^");
    PFto_ADD("LightGray", "%^WHITE%^");
    PFto_ADD("LightGreen", "%^WHITE%^");
    PFto_ADD("LightGrey", "%^WHITE%^");
    PFto_ADD("LightPink", "%^WHITE%^");
    PFto_ADD("LightPink1", "%^WHITE%^");
    PFto_ADD("LightPink2", "%^WHITE%^");
    PFto_ADD("LightPink3", "%^WHITE%^");
    PFto_ADD("LightPink4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("LightSalmon", "%^BOLD%^%^RED%^");
    PFto_ADD("LightSalmon1", "%^BOLD%^%^RED%^");
    PFto_ADD("LightSalmon2", "%^BOLD%^%^RED%^");
    PFto_ADD("LightSalmon3", "%^BOLD%^%^RED%^");
    PFto_ADD("LightSalmon4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("LightSeaGreen", "%^CYAN%^");
    PFto_ADD("LightSkyBlue", "%^BOLD%^%^CYAN%^");
    PFto_ADD("LightSkyBlue1", "%^WHITE%^");
    PFto_ADD("LightSkyBlue2", "%^WHITE%^");
    PFto_ADD("LightSkyBlue3", "%^WHITE%^");
    PFto_ADD("LightSkyBlue4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("LightSlateBlue", "%^BOLD%^%^BLUE%^");
    PFto_ADD("LightSlateGray", "%^WHITE%^");
    PFto_ADD("LightSlateGrey", "%^WHITE%^");
    PFto_ADD("LightSteelBlue", "%^WHITE%^");
    PFto_ADD("LightSteelBlue1", "%^BOLD%^%^WHITE%^");
    PFto_ADD("LightSteelBlue2", "%^WHITE%^");
    PFto_ADD("LightSteelBlue3", "%^WHITE%^");
    PFto_ADD("LightSteelBlue4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("LightYellow", "%^BOLD%^%^WHITE%^");
    PFto_ADD("LightYellow1", "%^BOLD%^%^WHITE%^");
    PFto_ADD("LightYellow2", "%^BOLD%^%^WHITE%^");
    PFto_ADD("LightYellow3", "%^WHITE%^");
    PFto_ADD("LightYellow4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("LimeGreen", "%^BOLD%^%^GREEN%^");
    PFto_ADD("Linen", "%^BOLD%^%^WHITE%^");
    PFto_ADD("MAGENTA", "%^MAGENTA%^");
    PFto_ADD("Magenta", "%^BOLD%^%^MAGENTA%^");
    PFto_ADD("Magenta1", "%^BOLD%^%^MAGENTA%^");
    PFto_ADD("Magenta2", "%^MAGENTA%^");
    PFto_ADD("Magenta3", "%^MAGENTA%^");
    PFto_ADD("Magenta4", "%^MAGENTA%^");
    PFto_ADD("Maroon", "%^BOLD%^%^RED%^");
    PFto_ADD("Maroon1", "%^BOLD%^%^MAGENTA%^");
    PFto_ADD("Maroon2", "%^MAGENTA%^");
    PFto_ADD("Maroon3", "%^MAGENTA%^");
    PFto_ADD("Maroon4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("MediumAquamarine", "%^WHITE%^");
    PFto_ADD("MediumBlue", "%^BLUE%^");
    PFto_ADD("MediumOrchid", "%^BOLD%^%^MAGENTA%^");
    PFto_ADD("MediumOrchid1", "%^BOLD%^%^MAGENTA%^");
    PFto_ADD("MediumOrchid2", "%^BOLD%^%^MAGENTA%^");
    PFto_ADD("MediumOrchid3", "%^MAGENTA%^");
    PFto_ADD("MediumOrchid4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("MediumPurple", "%^BOLD%^%^BLUE%^");
    PFto_ADD("MediumPurple1", "%^WHITE%^");
    PFto_ADD("MediumPurple2", "%^BOLD%^%^BLUE%^");
    PFto_ADD("MediumPurple3", "%^BOLD%^%^BLUE%^");
    PFto_ADD("MediumPurple4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("MediumSeaGreen", "%^BOLD%^%^GREEN%^");
    PFto_ADD("MediumSlateBlue", "%^BOLD%^%^BLUE%^");
    PFto_ADD("MediumSpringGreen", "%^CYAN%^");
    PFto_ADD("MediumTurquoise", "%^BOLD%^%^CYAN%^");
    PFto_ADD("MediumVioletRed", "%^MAGENTA%^");
    PFto_ADD("MidnightBlue", "%^BLUE%^");
    PFto_ADD("MintCream", "%^BOLD%^%^WHITE%^");
    PFto_ADD("MistyRose", "%^BOLD%^%^WHITE%^");
    PFto_ADD("MistyRose1", "%^BOLD%^%^WHITE%^");
    PFto_ADD("MistyRose2", "%^WHITE%^");
    PFto_ADD("MistyRose3", "%^WHITE%^");
    PFto_ADD("MistyRose4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Moccasin", "%^BOLD%^%^WHITE%^");
    PFto_ADD("NavajoWhite", "%^WHITE%^");
    PFto_ADD("NavajoWhite1", "%^WHITE%^");
    PFto_ADD("NavajoWhite2", "%^WHITE%^");
    PFto_ADD("NavajoWhite3", "%^WHITE%^");
    PFto_ADD("NavajoWhite4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Navy", "%^BLUE%^");
    PFto_ADD("NavyBlue", "%^BLUE%^");
    PFto_ADD("ORANGE", "%^ORANGE%^");
    PFto_ADD("OldLace", "%^BOLD%^%^WHITE%^");
    PFto_ADD("OliveDrab", "%^BOLD%^%^BLACK%^");
    PFto_ADD("OliveDrab1", "%^YELLOW%^");
    PFto_ADD("OliveDrab2", "%^ORANGE%^");
    PFto_ADD("OliveDrab3", "%^ORANGE%^");
    PFto_ADD("OliveDrab4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Orange", "%^ORANGE%^");
    PFto_ADD("Orange1", "%^ORANGE%^");
    PFto_ADD("Orange2", "%^ORANGE%^");
    PFto_ADD("Orange3", "%^ORANGE%^");
    PFto_ADD("Orange4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("OrangeRed", "%^BOLD%^%^RED%^");
    PFto_ADD("OrangeRed1", "%^BOLD%^%^RED%^");
    PFto_ADD("OrangeRed2", "%^RED%^");
    PFto_ADD("OrangeRed3", "%^RED%^");
    PFto_ADD("OrangeRed4", "%^RED%^");
    PFto_ADD("Orchid", "%^BOLD%^%^MAGENTA%^");
    PFto_ADD("Orchid1", "%^BOLD%^%^MAGENTA%^");
    PFto_ADD("Orchid2", "%^BOLD%^%^MAGENTA%^");
    PFto_ADD("Orchid3", "%^BOLD%^%^MAGENTA%^");
    PFto_ADD("Orchid4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("PINK", "%^BOLD%^%^MAGENTA%^");
    PFto_ADD("PaleGoldenrod", "%^WHITE%^");
    PFto_ADD("PaleGreen", "%^WHITE%^");
    PFto_ADD("PaleGreen1", "%^WHITE%^");
    PFto_ADD("PaleGreen2", "%^WHITE%^");
    PFto_ADD("PaleGreen3", "%^BOLD%^%^GREEN%^");
    PFto_ADD("PaleGreen4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("PaleTurquoise", "%^WHITE%^");
    PFto_ADD("PaleTurquoise1", "%^BOLD%^%^WHITE%^");
    PFto_ADD("PaleTurquoise2", "%^WHITE%^");
    PFto_ADD("PaleTurquoise3", "%^WHITE%^");
    PFto_ADD("PaleTurquoise4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("PaleVioletRed", "%^BOLD%^%^RED%^");
    PFto_ADD("PaleVioletRed1", "%^WHITE%^");
    PFto_ADD("PaleVioletRed2", "%^BOLD%^%^RED%^");
    PFto_ADD("PaleVioletRed3", "%^BOLD%^%^RED%^");
    PFto_ADD("PaleVioletRed4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("PapayaWhip", "%^BOLD%^%^WHITE%^");
    PFto_ADD("PeachPuff", "%^WHITE%^");
    PFto_ADD("PeachPuff1", "%^WHITE%^");
    PFto_ADD("PeachPuff2", "%^WHITE%^");
    PFto_ADD("PeachPuff3", "%^WHITE%^");
    PFto_ADD("PeachPuff4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Peru", "%^BOLD%^%^RED%^");
    PFto_ADD("Pink", "%^WHITE%^");
    PFto_ADD("Pink1", "%^WHITE%^");
    PFto_ADD("Pink2", "%^WHITE%^");
    PFto_ADD("Pink3", "%^WHITE%^");
    PFto_ADD("Pink4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Plum", "%^WHITE%^");
    PFto_ADD("Plum1", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Plum2", "%^WHITE%^");
    PFto_ADD("Plum3", "%^WHITE%^");
    PFto_ADD("Plum4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("PowderBlue", "%^WHITE%^");
    PFto_ADD("Purple", "%^MAGENTA%^");
    PFto_ADD("Purple1", "%^BOLD%^%^BLUE%^");
    PFto_ADD("Purple2", "%^BOLD%^%^BLUE%^");
    PFto_ADD("Purple3", "%^MAGENTA%^");
    PFto_ADD("Purple4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("RED", "%^RED%^");
    PFto_ADD("RESET", "%^RESET%^");
    PFto_ADD("RESTORE", "%^RESTORE%^");
    PFto_ADD("REVERSE", "%^REVERSE%^");
    PFto_ADD("Red", "%^RED%^");
    PFto_ADD("Red1", "%^RED%^");
    PFto_ADD("Red2", "%^RED%^");
    PFto_ADD("Red3", "%^RED%^");
    PFto_ADD("Red4", "%^RED%^");
    PFto_ADD("RosyBrown", "%^WHITE%^");
    PFto_ADD("RosyBrown1", "%^WHITE%^");
    PFto_ADD("RosyBrown2", "%^WHITE%^");
    PFto_ADD("RosyBrown3", "%^WHITE%^");
    PFto_ADD("RosyBrown4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("RoyalBlue", "%^BOLD%^%^BLUE%^");
    PFto_ADD("RoyalBlue1", "%^BOLD%^%^BLUE%^");
    PFto_ADD("RoyalBlue2", "%^BOLD%^%^BLUE%^");
    PFto_ADD("RoyalBlue3", "%^BOLD%^%^BLUE%^");
    PFto_ADD("RoyalBlue4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("SAVE", "%^SAVE%^");
    PFto_ADD("STRIKETHRU", "%^STRIKETHRU%^");
    PFto_ADD("SaddleBrown", "%^RED%^");
    PFto_ADD("Salmon", "%^BOLD%^%^RED%^");
    PFto_ADD("Salmon1", "%^BOLD%^%^RED%^");
    PFto_ADD("Salmon2", "%^BOLD%^%^RED%^");
    PFto_ADD("Salmon3", "%^BOLD%^%^RED%^");
    PFto_ADD("Salmon4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("SandyBrown", "%^BOLD%^%^RED%^");
    PFto_ADD("SeaGreen", "%^BOLD%^%^BLACK%^");
    PFto_ADD("SeaGreen1", "%^BOLD%^%^GREEN%^");
    PFto_ADD("SeaGreen2", "%^BOLD%^%^GREEN%^");
    PFto_ADD("SeaGreen3", "%^BOLD%^%^GREEN%^");
    PFto_ADD("SeaGreen4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Seashell", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Seashell1", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Seashell2", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Seashell3", "%^WHITE%^");
    PFto_ADD("Seashell4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Sienna", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Sienna1", "%^BOLD%^%^RED%^");
    PFto_ADD("Sienna2", "%^BOLD%^%^RED%^");
    PFto_ADD("Sienna3", "%^BOLD%^%^RED%^");
    PFto_ADD("Sienna4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("SkyBlue", "%^BOLD%^%^CYAN%^");
    PFto_ADD("SkyBlue1", "%^BOLD%^%^CYAN%^");
    PFto_ADD("SkyBlue2", "%^BOLD%^%^CYAN%^");
    PFto_ADD("SkyBlue3", "%^WHITE%^");
    PFto_ADD("SkyBlue4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("SlateBlue", "%^BOLD%^%^BLUE%^");
    PFto_ADD("SlateBlue1", "%^BOLD%^%^BLUE%^");
    PFto_ADD("SlateBlue2", "%^BOLD%^%^BLUE%^");
    PFto_ADD("SlateBlue3", "%^BOLD%^%^BLUE%^");
    PFto_ADD("SlateBlue4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("SlateGray", "%^BOLD%^%^BLACK%^");
    PFto_ADD("SlateGray1", "%^BOLD%^%^WHITE%^");
    PFto_ADD("SlateGray2", "%^WHITE%^");
    PFto_ADD("SlateGray3", "%^WHITE%^");
    PFto_ADD("SlateGray4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("SlateGrey", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Snow", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Snow1", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Snow2", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Snow3", "%^WHITE%^");
    PFto_ADD("Snow4", "%^WHITE%^");
    PFto_ADD("SpringGreen", "%^CYAN%^");
    PFto_ADD("SpringGreen1", "%^CYAN%^");
    PFto_ADD("SpringGreen2", "%^CYAN%^");
    PFto_ADD("SpringGreen3", "%^CYAN%^");
    PFto_ADD("SpringGreen4", "%^GREEN%^");
    PFto_ADD("SteelBlue", "%^BOLD%^%^BLUE%^");
    PFto_ADD("SteelBlue1", "%^BOLD%^%^CYAN%^");
    PFto_ADD("SteelBlue2", "%^BOLD%^%^CYAN%^");
    PFto_ADD("SteelBlue3", "%^BOLD%^%^BLUE%^");
    PFto_ADD("SteelBlue4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Tan", "%^WHITE%^");
    PFto_ADD("Tan1", "%^BOLD%^%^RED%^");
    PFto_ADD("Tan2", "%^BOLD%^%^RED%^");
    PFto_ADD("Tan3", "%^BOLD%^%^RED%^");
    PFto_ADD("Tan4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Thistle", "%^WHITE%^");
    PFto_ADD("Thistle1", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Thistle2", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Thistle3", "%^WHITE%^");
    PFto_ADD("Thistle4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Tomato", "%^BOLD%^%^RED%^");
    PFto_ADD("Tomato1", "%^BOLD%^%^RED%^");
    PFto_ADD("Tomato2", "%^BOLD%^%^RED%^");
    PFto_ADD("Tomato3", "%^BOLD%^%^RED%^");
    PFto_ADD("Tomato4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("Turquoise", "%^BOLD%^%^CYAN%^");
    PFto_ADD("Turquoise1", "%^BOLD%^%^CYAN%^");
    PFto_ADD("Turquoise2", "%^CYAN%^");
    PFto_ADD("Turquoise3", "%^CYAN%^");
    PFto_ADD("Turquoise4", "%^CYAN%^");
    PFto_ADD("UNDERLINE", "%^UNDERLINE%^");
    PFto_ADD("Violet", "%^BOLD%^%^MAGENTA%^");
    PFto_ADD("VioletRed", "%^MAGENTA%^");
    PFto_ADD("VioletRed1", "%^BOLD%^%^RED%^");
    PFto_ADD("VioletRed2", "%^BOLD%^%^RED%^");
    PFto_ADD("VioletRed3", "%^BOLD%^%^RED%^");
    PFto_ADD("VioletRed4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("WHITE", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Wheat", "%^WHITE%^");
    PFto_ADD("Wheat1", "%^BOLD%^%^WHITE%^");
    PFto_ADD("Wheat2", "%^WHITE%^");
    PFto_ADD("Wheat3", "%^WHITE%^");
    PFto_ADD("Wheat4", "%^BOLD%^%^BLACK%^");
    PFto_ADD("White", "%^BOLD%^%^WHITE%^");
    PFto_ADD("WhiteSmoke", "%^BOLD%^%^WHITE%^");
    PFto_ADD("YELLOW", "%^YELLOW%^");
    PFto_ADD("Yellow", "%^YELLOW%^");
    PFto_ADD("Yellow1", "%^YELLOW%^");
    PFto_ADD("Yellow2", "%^ORANGE%^");
    PFto_ADD("Yellow3", "%^ORANGE%^");
    PFto_ADD("Yellow4", "%^ORANGE%^");
    PFto_ADD("YellowGreen", "%^ORANGE%^");

    log_info("Pinkfish to I3 conversion data loaded.");
}

void I3_loadPinkfishToXterm256(void)
{
    log_info("Initializing Pinkfish to Xterm256 conversion table...");
    stringmap_destroy(pinkfish_to_xterm256_db); // Free anything present
    pinkfish_to_xterm256_db = stringmap_init(); // Setup a new empty structure

    log_info("Loading Pinkfish to Xterm256 conversion data...");

#undef PFto_ADD
#define PFto_ADD(key, value)                                                                                           \
    do                                                                                                                 \
    {                                                                                                                  \
        stringmap_add(pinkfish_to_xterm256_db, (key), (void *)(value), strlen((char *)(value)));                       \
        pinkfish_to_xterm256_count++;                                                                                  \
    } while (0)

    PFto_ADD("AliceBlue", "\033[38;5;15m");
    PFto_ADD("AntiqueWhite", "\033[38;5;224m");
    PFto_ADD("AntiqueWhite1", "\033[38;5;230m");
    PFto_ADD("AntiqueWhite2", "\033[38;5;187m");
    PFto_ADD("AntiqueWhite3", "\033[38;5;181m");
    PFto_ADD("AntiqueWhite4", "\033[38;5;244m");
    PFto_ADD("Aquamarine", "\033[38;5;122m");
    PFto_ADD("Aquamarine1", "\033[38;5;122m");
    PFto_ADD("Aquamarine2", "\033[38;5;115m");
    PFto_ADD("Aquamarine3", "\033[38;5;79m");
    PFto_ADD("Aquamarine4", "\033[38;5;66m");
    PFto_ADD("Azure", "\033[38;5;15m");
    PFto_ADD("Azure1", "\033[38;5;15m");
    PFto_ADD("Azure2", "\033[38;5;255m");
    PFto_ADD("Azure3", "\033[38;5;251m");
    PFto_ADD("Azure4", "\033[38;5;102m");
    PFto_ADD("B000", "\033[48;5;16m");
    PFto_ADD("B001", "\033[48;5;17m");
    PFto_ADD("B002", "\033[48;5;18m");
    PFto_ADD("B003", "\033[48;5;19m");
    PFto_ADD("B004", "\033[48;5;20m");
    PFto_ADD("B005", "\033[48;5;21m");
    PFto_ADD("B010", "\033[48;5;22m");
    PFto_ADD("B011", "\033[48;5;23m");
    PFto_ADD("B012", "\033[48;5;24m");
    PFto_ADD("B013", "\033[48;5;25m");
    PFto_ADD("B014", "\033[48;5;26m");
    PFto_ADD("B015", "\033[48;5;27m");
    PFto_ADD("B020", "\033[48;5;28m");
    PFto_ADD("B021", "\033[48;5;29m");
    PFto_ADD("B022", "\033[48;5;30m");
    PFto_ADD("B023", "\033[48;5;31m");
    PFto_ADD("B024", "\033[48;5;32m");
    PFto_ADD("B025", "\033[48;5;33m");
    PFto_ADD("B030", "\033[48;5;34m");
    PFto_ADD("B031", "\033[48;5;35m");
    PFto_ADD("B032", "\033[48;5;36m");
    PFto_ADD("B033", "\033[48;5;37m");
    PFto_ADD("B034", "\033[48;5;38m");
    PFto_ADD("B035", "\033[48;5;39m");
    PFto_ADD("B040", "\033[48;5;40m");
    PFto_ADD("B041", "\033[48;5;41m");
    PFto_ADD("B042", "\033[48;5;42m");
    PFto_ADD("B043", "\033[48;5;43m");
    PFto_ADD("B044", "\033[48;5;44m");
    PFto_ADD("B045", "\033[48;5;45m");
    PFto_ADD("B050", "\033[48;5;46m");
    PFto_ADD("B051", "\033[48;5;47m");
    PFto_ADD("B052", "\033[48;5;48m");
    PFto_ADD("B053", "\033[48;5;49m");
    PFto_ADD("B054", "\033[48;5;50m");
    PFto_ADD("B055", "\033[48;5;51m");
    PFto_ADD("B100", "\033[48;5;52m");
    PFto_ADD("B101", "\033[48;5;53m");
    PFto_ADD("B102", "\033[48;5;54m");
    PFto_ADD("B103", "\033[48;5;55m");
    PFto_ADD("B104", "\033[48;5;56m");
    PFto_ADD("B105", "\033[48;5;57m");
    PFto_ADD("B110", "\033[48;5;58m");
    PFto_ADD("B111", "\033[48;5;59m");
    PFto_ADD("B112", "\033[48;5;60m");
    PFto_ADD("B113", "\033[48;5;61m");
    PFto_ADD("B114", "\033[48;5;62m");
    PFto_ADD("B115", "\033[48;5;63m");
    PFto_ADD("B120", "\033[48;5;64m");
    PFto_ADD("B121", "\033[48;5;65m");
    PFto_ADD("B122", "\033[48;5;66m");
    PFto_ADD("B123", "\033[48;5;67m");
    PFto_ADD("B124", "\033[48;5;68m");
    PFto_ADD("B125", "\033[48;5;69m");
    PFto_ADD("B130", "\033[48;5;70m");
    PFto_ADD("B131", "\033[48;5;71m");
    PFto_ADD("B132", "\033[48;5;72m");
    PFto_ADD("B133", "\033[48;5;73m");
    PFto_ADD("B134", "\033[48;5;74m");
    PFto_ADD("B135", "\033[48;5;75m");
    PFto_ADD("B140", "\033[48;5;76m");
    PFto_ADD("B141", "\033[48;5;77m");
    PFto_ADD("B142", "\033[48;5;78m");
    PFto_ADD("B143", "\033[48;5;79m");
    PFto_ADD("B144", "\033[48;5;80m");
    PFto_ADD("B145", "\033[48;5;81m");
    PFto_ADD("B150", "\033[48;5;82m");
    PFto_ADD("B151", "\033[48;5;83m");
    PFto_ADD("B152", "\033[48;5;84m");
    PFto_ADD("B153", "\033[48;5;85m");
    PFto_ADD("B154", "\033[48;5;86m");
    PFto_ADD("B155", "\033[48;5;87m");
    PFto_ADD("B200", "\033[48;5;88m");
    PFto_ADD("B201", "\033[48;5;89m");
    PFto_ADD("B202", "\033[48;5;90m");
    PFto_ADD("B203", "\033[48;5;91m");
    PFto_ADD("B204", "\033[48;5;92m");
    PFto_ADD("B205", "\033[48;5;93m");
    PFto_ADD("B210", "\033[48;5;94m");
    PFto_ADD("B211", "\033[48;5;95m");
    PFto_ADD("B212", "\033[48;5;96m");
    PFto_ADD("B213", "\033[48;5;97m");
    PFto_ADD("B214", "\033[48;5;98m");
    PFto_ADD("B215", "\033[48;5;99m");
    PFto_ADD("B220", "\033[48;5;100m");
    PFto_ADD("B221", "\033[48;5;101m");
    PFto_ADD("B222", "\033[48;5;102m");
    PFto_ADD("B223", "\033[48;5;103m");
    PFto_ADD("B224", "\033[48;5;104m");
    PFto_ADD("B225", "\033[48;5;105m");
    PFto_ADD("B230", "\033[48;5;106m");
    PFto_ADD("B231", "\033[48;5;107m");
    PFto_ADD("B232", "\033[48;5;108m");
    PFto_ADD("B233", "\033[48;5;109m");
    PFto_ADD("B234", "\033[48;5;110m");
    PFto_ADD("B235", "\033[48;5;111m");
    PFto_ADD("B240", "\033[48;5;112m");
    PFto_ADD("B241", "\033[48;5;113m");
    PFto_ADD("B242", "\033[48;5;114m");
    PFto_ADD("B243", "\033[48;5;115m");
    PFto_ADD("B244", "\033[48;5;116m");
    PFto_ADD("B245", "\033[48;5;117m");
    PFto_ADD("B250", "\033[48;5;118m");
    PFto_ADD("B251", "\033[48;5;119m");
    PFto_ADD("B252", "\033[48;5;120m");
    PFto_ADD("B253", "\033[48;5;121m");
    PFto_ADD("B254", "\033[48;5;122m");
    PFto_ADD("B255", "\033[48;5;123m");
    PFto_ADD("B300", "\033[48;5;124m");
    PFto_ADD("B301", "\033[48;5;125m");
    PFto_ADD("B302", "\033[48;5;126m");
    PFto_ADD("B303", "\033[48;5;127m");
    PFto_ADD("B304", "\033[48;5;128m");
    PFto_ADD("B305", "\033[48;5;129m");
    PFto_ADD("B310", "\033[48;5;130m");
    PFto_ADD("B311", "\033[48;5;131m");
    PFto_ADD("B312", "\033[48;5;132m");
    PFto_ADD("B313", "\033[48;5;133m");
    PFto_ADD("B314", "\033[48;5;134m");
    PFto_ADD("B315", "\033[48;5;135m");
    PFto_ADD("B320", "\033[48;5;136m");
    PFto_ADD("B321", "\033[48;5;137m");
    PFto_ADD("B322", "\033[48;5;138m");
    PFto_ADD("B323", "\033[48;5;139m");
    PFto_ADD("B324", "\033[48;5;140m");
    PFto_ADD("B325", "\033[48;5;141m");
    PFto_ADD("B330", "\033[48;5;142m");
    PFto_ADD("B331", "\033[48;5;143m");
    PFto_ADD("B332", "\033[48;5;144m");
    PFto_ADD("B333", "\033[48;5;145m");
    PFto_ADD("B334", "\033[48;5;146m");
    PFto_ADD("B335", "\033[48;5;147m");
    PFto_ADD("B340", "\033[48;5;148m");
    PFto_ADD("B341", "\033[48;5;149m");
    PFto_ADD("B342", "\033[48;5;150m");
    PFto_ADD("B343", "\033[48;5;151m");
    PFto_ADD("B344", "\033[48;5;152m");
    PFto_ADD("B345", "\033[48;5;153m");
    PFto_ADD("B350", "\033[48;5;154m");
    PFto_ADD("B351", "\033[48;5;155m");
    PFto_ADD("B352", "\033[48;5;156m");
    PFto_ADD("B353", "\033[48;5;157m");
    PFto_ADD("B354", "\033[48;5;158m");
    PFto_ADD("B355", "\033[48;5;159m");
    PFto_ADD("B400", "\033[48;5;160m");
    PFto_ADD("B401", "\033[48;5;161m");
    PFto_ADD("B402", "\033[48;5;162m");
    PFto_ADD("B403", "\033[48;5;163m");
    PFto_ADD("B404", "\033[48;5;164m");
    PFto_ADD("B405", "\033[48;5;165m");
    PFto_ADD("B410", "\033[48;5;166m");
    PFto_ADD("B411", "\033[48;5;167m");
    PFto_ADD("B412", "\033[48;5;168m");
    PFto_ADD("B413", "\033[48;5;169m");
    PFto_ADD("B414", "\033[48;5;170m");
    PFto_ADD("B415", "\033[48;5;171m");
    PFto_ADD("B420", "\033[48;5;172m");
    PFto_ADD("B421", "\033[48;5;173m");
    PFto_ADD("B422", "\033[48;5;174m");
    PFto_ADD("B423", "\033[48;5;175m");
    PFto_ADD("B424", "\033[48;5;176m");
    PFto_ADD("B425", "\033[48;5;177m");
    PFto_ADD("B430", "\033[48;5;178m");
    PFto_ADD("B431", "\033[48;5;179m");
    PFto_ADD("B432", "\033[48;5;180m");
    PFto_ADD("B433", "\033[48;5;181m");
    PFto_ADD("B434", "\033[48;5;182m");
    PFto_ADD("B435", "\033[48;5;183m");
    PFto_ADD("B440", "\033[48;5;184m");
    PFto_ADD("B441", "\033[48;5;185m");
    PFto_ADD("B442", "\033[48;5;186m");
    PFto_ADD("B443", "\033[48;5;187m");
    PFto_ADD("B444", "\033[48;5;188m");
    PFto_ADD("B445", "\033[48;5;189m");
    PFto_ADD("B450", "\033[48;5;190m");
    PFto_ADD("B451", "\033[48;5;191m");
    PFto_ADD("B452", "\033[48;5;192m");
    PFto_ADD("B453", "\033[48;5;193m");
    PFto_ADD("B454", "\033[48;5;194m");
    PFto_ADD("B455", "\033[48;5;195m");
    PFto_ADD("B500", "\033[48;5;196m");
    PFto_ADD("B501", "\033[48;5;197m");
    PFto_ADD("B502", "\033[48;5;198m");
    PFto_ADD("B503", "\033[48;5;199m");
    PFto_ADD("B504", "\033[48;5;200m");
    PFto_ADD("B505", "\033[48;5;201m");
    PFto_ADD("B510", "\033[48;5;202m");
    PFto_ADD("B511", "\033[48;5;203m");
    PFto_ADD("B512", "\033[48;5;204m");
    PFto_ADD("B513", "\033[48;5;205m");
    PFto_ADD("B514", "\033[48;5;206m");
    PFto_ADD("B515", "\033[48;5;207m");
    PFto_ADD("B520", "\033[48;5;208m");
    PFto_ADD("B521", "\033[48;5;209m");
    PFto_ADD("B522", "\033[48;5;210m");
    PFto_ADD("B523", "\033[48;5;211m");
    PFto_ADD("B524", "\033[48;5;212m");
    PFto_ADD("B525", "\033[48;5;213m");
    PFto_ADD("B530", "\033[48;5;214m");
    PFto_ADD("B531", "\033[48;5;215m");
    PFto_ADD("B532", "\033[48;5;216m");
    PFto_ADD("B533", "\033[48;5;217m");
    PFto_ADD("B534", "\033[48;5;218m");
    PFto_ADD("B535", "\033[48;5;219m");
    PFto_ADD("B540", "\033[48;5;220m");
    PFto_ADD("B541", "\033[48;5;221m");
    PFto_ADD("B542", "\033[48;5;222m");
    PFto_ADD("B543", "\033[48;5;223m");
    PFto_ADD("B544", "\033[48;5;224m");
    PFto_ADD("B545", "\033[48;5;225m");
    PFto_ADD("B550", "\033[48;5;226m");
    PFto_ADD("B551", "\033[48;5;227m");
    PFto_ADD("B552", "\033[48;5;228m");
    PFto_ADD("B553", "\033[48;5;229m");
    PFto_ADD("B554", "\033[48;5;230m");
    PFto_ADD("B555", "\033[48;5;231m");
    PFto_ADD("BG00", "\033[48;5;0m");
    PFto_ADD("BG01", "\033[48;5;232m");
    PFto_ADD("BG02", "\033[48;5;233m");
    PFto_ADD("BG03", "\033[48;5;234m");
    PFto_ADD("BG04", "\033[48;5;235m");
    PFto_ADD("BG05", "\033[48;5;236m");
    PFto_ADD("BG06", "\033[48;5;237m");
    PFto_ADD("BG07", "\033[48;5;238m");
    PFto_ADD("BG08", "\033[48;5;239m");
    PFto_ADD("BG09", "\033[48;5;240m");
    PFto_ADD("BG10", "\033[48;5;241m");
    PFto_ADD("BG11", "\033[48;5;242m");
    PFto_ADD("BG12", "\033[48;5;243m");
    PFto_ADD("BG13", "\033[48;5;244m");
    PFto_ADD("BG14", "\033[48;5;245m");
    PFto_ADD("BG15", "\033[48;5;246m");
    PFto_ADD("BG16", "\033[48;5;247m");
    PFto_ADD("BG17", "\033[48;5;248m");
    PFto_ADD("BG18", "\033[48;5;249m");
    PFto_ADD("BG19", "\033[48;5;250m");
    PFto_ADD("BG20", "\033[48;5;251m");
    PFto_ADD("BG21", "\033[48;5;252m");
    PFto_ADD("BG22", "\033[48;5;253m");
    PFto_ADD("BG23", "\033[48;5;254m");
    PFto_ADD("BG24", "\033[48;5;255m");
    PFto_ADD("BG25", "\033[48;5;15m");
    PFto_ADD("BLACK", "\033[38;5;0m");
    PFto_ADD("BLUE", "\033[38;5;4m");
    PFto_ADD("BOLD", "\033[1m");
    PFto_ADD("B_AliceBlue", "\033[48;5;15m");
    PFto_ADD("B_AntiqueWhite", "\033[48;5;224m");
    PFto_ADD("B_AntiqueWhite1", "\033[48;5;230m");
    PFto_ADD("B_AntiqueWhite2", "\033[48;5;187m");
    PFto_ADD("B_AntiqueWhite3", "\033[48;5;181m");
    PFto_ADD("B_AntiqueWhite4", "\033[48;5;244m");
    PFto_ADD("B_Aquamarine", "\033[48;5;122m");
    PFto_ADD("B_Aquamarine1", "\033[48;5;122m");
    PFto_ADD("B_Aquamarine2", "\033[48;5;115m");
    PFto_ADD("B_Aquamarine3", "\033[48;5;79m");
    PFto_ADD("B_Aquamarine4", "\033[48;5;66m");
    PFto_ADD("B_Azure", "\033[48;5;15m");
    PFto_ADD("B_Azure1", "\033[48;5;15m");
    PFto_ADD("B_Azure2", "\033[48;5;255m");
    PFto_ADD("B_Azure3", "\033[48;5;251m");
    PFto_ADD("B_Azure4", "\033[48;5;102m");
    PFto_ADD("B_BLACK", "\033[48;5;0m");
    PFto_ADD("B_BLUE", "\033[48;5;4m");
    PFto_ADD("B_Beige", "\033[48;5;230m");
    PFto_ADD("B_Bisque", "\033[48;5;223m");
    PFto_ADD("B_Bisque1", "\033[48;5;223m");
    PFto_ADD("B_Bisque2", "\033[48;5;187m");
    PFto_ADD("B_Bisque3", "\033[48;5;180m");
    PFto_ADD("B_Bisque4", "\033[48;5;244m");
    PFto_ADD("B_Black", "\033[48;5;0m");
    PFto_ADD("B_BlanchedAlmond", "\033[48;5;224m");
    PFto_ADD("B_Blue", "\033[48;5;21m");
    PFto_ADD("B_Blue1", "\033[48;5;21m");
    PFto_ADD("B_Blue2", "\033[48;5;20m");
    PFto_ADD("B_Blue3", "\033[48;5;20m");
    PFto_ADD("B_Blue4", "\033[48;5;18m");
    PFto_ADD("B_BlueViolet", "\033[48;5;98m");
    PFto_ADD("B_Brown", "\033[48;5;1m");
    PFto_ADD("B_Brown1", "\033[48;5;9m");
    PFto_ADD("B_Brown2", "\033[48;5;9m");
    PFto_ADD("B_Brown3", "\033[48;5;167m");
    PFto_ADD("B_Brown4", "\033[48;5;88m");
    PFto_ADD("B_Burlywood", "\033[48;5;180m");
    PFto_ADD("B_Burlywood1", "\033[48;5;222m");
    PFto_ADD("B_Burlywood2", "\033[48;5;180m");
    PFto_ADD("B_Burlywood3", "\033[48;5;180m");
    PFto_ADD("B_Burlywood4", "\033[48;5;101m");
    PFto_ADD("B_CYAN", "\033[48;5;6m");
    PFto_ADD("B_CadetBlue", "\033[48;5;66m");
    PFto_ADD("B_CadetBlue1", "\033[48;5;123m");
    PFto_ADD("B_CadetBlue2", "\033[48;5;116m");
    PFto_ADD("B_CadetBlue3", "\033[48;5;110m");
    PFto_ADD("B_CadetBlue4", "\033[48;5;66m");
    PFto_ADD("B_Chartreuse", "\033[48;5;118m");
    PFto_ADD("B_Chartreuse1", "\033[48;5;118m");
    PFto_ADD("B_Chartreuse2", "\033[48;5;112m");
    PFto_ADD("B_Chartreuse3", "\033[48;5;76m");
    PFto_ADD("B_Chartreuse4", "\033[48;5;64m");
    PFto_ADD("B_Chocolate", "\033[48;5;166m");
    PFto_ADD("B_Chocolate1", "\033[48;5;208m");
    PFto_ADD("B_Chocolate2", "\033[48;5;172m");
    PFto_ADD("B_Chocolate3", "\033[48;5;166m");
    PFto_ADD("B_Chocolate4", "\033[48;5;94m");
    PFto_ADD("B_Coral", "\033[48;5;209m");
    PFto_ADD("B_Coral1", "\033[48;5;209m");
    PFto_ADD("B_Coral2", "\033[48;5;9m");
    PFto_ADD("B_Coral3", "\033[48;5;167m");
    PFto_ADD("B_Coral4", "\033[48;5;95m");
    PFto_ADD("B_CornflowerBlue", "\033[48;5;68m");
    PFto_ADD("B_Cornsilk", "\033[48;5;230m");
    PFto_ADD("B_Cornsilk1", "\033[48;5;230m");
    PFto_ADD("B_Cornsilk2", "\033[48;5;254m");
    PFto_ADD("B_Cornsilk3", "\033[48;5;251m");
    PFto_ADD("B_Cornsilk4", "\033[48;5;244m");
    PFto_ADD("B_Cyan", "\033[48;5;51m");
    PFto_ADD("B_Cyan1", "\033[48;5;51m");
    PFto_ADD("B_Cyan2", "\033[48;5;44m");
    PFto_ADD("B_Cyan3", "\033[48;5;44m");
    PFto_ADD("B_Cyan4", "\033[48;5;30m");
    PFto_ADD("B_DARKGREY", "\033[48;5;8m");
    PFto_ADD("B_DarkBlue", "\033[48;5;18m");
    PFto_ADD("B_DarkCyan", "\033[48;5;30m");
    PFto_ADD("B_DarkGoldenrod", "\033[48;5;136m");
    PFto_ADD("B_DarkGoldenrod1", "\033[48;5;214m");
    PFto_ADD("B_DarkGoldenrod2", "\033[48;5;178m");
    PFto_ADD("B_DarkGoldenrod3", "\033[48;5;172m");
    PFto_ADD("B_DarkGoldenrod4", "\033[48;5;94m");
    PFto_ADD("B_DarkGray", "\033[48;5;248m");
    PFto_ADD("B_DarkGreen", "\033[48;5;22m");
    PFto_ADD("B_DarkGrey", "\033[48;5;248m");
    PFto_ADD("B_DarkKhaki", "\033[48;5;143m");
    PFto_ADD("B_DarkMagenta", "\033[48;5;90m");
    PFto_ADD("B_DarkOliveGreen", "\033[48;5;239m");
    PFto_ADD("B_DarkOliveGreen1", "\033[48;5;156m");
    PFto_ADD("B_DarkOliveGreen2", "\033[48;5;149m");
    PFto_ADD("B_DarkOliveGreen3", "\033[48;5;149m");
    PFto_ADD("B_DarkOliveGreen4", "\033[48;5;65m");
    PFto_ADD("B_DarkOrange", "\033[48;5;208m");
    PFto_ADD("B_DarkOrange1", "\033[48;5;208m");
    PFto_ADD("B_DarkOrange2", "\033[48;5;172m");
    PFto_ADD("B_DarkOrange3", "\033[48;5;166m");
    PFto_ADD("B_DarkOrange4", "\033[48;5;94m");
    PFto_ADD("B_DarkOrchid", "\033[48;5;97m");
    PFto_ADD("B_DarkOrchid1", "\033[48;5;135m");
    PFto_ADD("B_DarkOrchid2", "\033[48;5;134m");
    PFto_ADD("B_DarkOrchid3", "\033[48;5;98m");
    PFto_ADD("B_DarkOrchid4", "\033[48;5;54m");
    PFto_ADD("B_DarkRed", "\033[48;5;88m");
    PFto_ADD("B_DarkSalmon", "\033[48;5;174m");
    PFto_ADD("B_DarkSeaGreen", "\033[48;5;108m");
    PFto_ADD("B_DarkSeaGreen1", "\033[48;5;157m");
    PFto_ADD("B_DarkSeaGreen2", "\033[48;5;151m");
    PFto_ADD("B_DarkSeaGreen3", "\033[48;5;114m");
    PFto_ADD("B_DarkSeaGreen4", "\033[48;5;243m");
    PFto_ADD("B_DarkSlateBlue", "\033[48;5;60m");
    PFto_ADD("B_DarkSlateGray", "\033[48;5;238m");
    PFto_ADD("B_DarkSlateGray1", "\033[48;5;123m");
    PFto_ADD("B_DarkSlateGray2", "\033[48;5;116m");
    PFto_ADD("B_DarkSlateGray3", "\033[48;5;116m");
    PFto_ADD("B_DarkSlateGray4", "\033[48;5;66m");
    PFto_ADD("B_DarkSlateGrey", "\033[48;5;238m");
    PFto_ADD("B_DarkTurquoise", "\033[48;5;44m");
    PFto_ADD("B_DarkViolet", "\033[48;5;92m");
    PFto_ADD("B_DeepPink", "\033[48;5;198m");
    PFto_ADD("B_DeepPink1", "\033[48;5;198m");
    PFto_ADD("B_DeepPink2", "\033[48;5;162m");
    PFto_ADD("B_DeepPink3", "\033[48;5;162m");
    PFto_ADD("B_DeepPink4", "\033[48;5;89m");
    PFto_ADD("B_DeepSkyBlue", "\033[48;5;39m");
    PFto_ADD("B_DeepSkyBlue1", "\033[48;5;39m");
    PFto_ADD("B_DeepSkyBlue2", "\033[48;5;38m");
    PFto_ADD("B_DeepSkyBlue3", "\033[48;5;32m");
    PFto_ADD("B_DeepSkyBlue4", "\033[48;5;24m");
    PFto_ADD("B_DimGray", "\033[48;5;242m");
    PFto_ADD("B_DimGrey", "\033[48;5;242m");
    PFto_ADD("B_DodgerBlue", "\033[48;5;33m");
    PFto_ADD("B_DodgerBlue1", "\033[48;5;33m");
    PFto_ADD("B_DodgerBlue2", "\033[48;5;32m");
    PFto_ADD("B_DodgerBlue3", "\033[48;5;32m");
    PFto_ADD("B_DodgerBlue4", "\033[48;5;24m");
    PFto_ADD("B_Firebrick", "\033[48;5;1m");
    PFto_ADD("B_Firebrick1", "\033[48;5;9m");
    PFto_ADD("B_Firebrick2", "\033[48;5;9m");
    PFto_ADD("B_Firebrick3", "\033[48;5;160m");
    PFto_ADD("B_Firebrick4", "\033[48;5;88m");
    PFto_ADD("B_FloralWhite", "\033[48;5;15m");
    PFto_ADD("B_ForestGreen", "\033[48;5;28m");
    PFto_ADD("B_GREEN", "\033[48;5;2m");
    PFto_ADD("B_GREY", "\033[48;5;7m");
    PFto_ADD("B_Gainsboro", "\033[48;5;188m");
    PFto_ADD("B_GhostWhite", "\033[48;5;15m");
    PFto_ADD("B_Gold", "\033[48;5;220m");
    PFto_ADD("B_Gold1", "\033[48;5;220m");
    PFto_ADD("B_Gold2", "\033[48;5;178m");
    PFto_ADD("B_Gold3", "\033[48;5;178m");
    PFto_ADD("B_Gold4", "\033[48;5;100m");
    PFto_ADD("B_Goldenrod", "\033[48;5;178m");
    PFto_ADD("B_Goldenrod1", "\033[48;5;214m");
    PFto_ADD("B_Goldenrod2", "\033[48;5;178m");
    PFto_ADD("B_Goldenrod3", "\033[48;5;172m");
    PFto_ADD("B_Goldenrod4", "\033[48;5;94m");
    PFto_ADD("B_Gray", "\033[48;5;250m");
    PFto_ADD("B_Gray0", "\033[48;5;0m");
    PFto_ADD("B_Gray1", "\033[48;5;0m");
    PFto_ADD("B_Gray10", "\033[48;5;234m");
    PFto_ADD("B_Gray100", "\033[48;5;15m");
    PFto_ADD("B_Gray11", "\033[48;5;234m");
    PFto_ADD("B_Gray12", "\033[48;5;234m");
    PFto_ADD("B_Gray13", "\033[48;5;234m");
    PFto_ADD("B_Gray14", "\033[48;5;235m");
    PFto_ADD("B_Gray15", "\033[48;5;235m");
    PFto_ADD("B_Gray16", "\033[48;5;235m");
    PFto_ADD("B_Gray17", "\033[48;5;235m");
    PFto_ADD("B_Gray18", "\033[48;5;236m");
    PFto_ADD("B_Gray19", "\033[48;5;236m");
    PFto_ADD("B_Gray2", "\033[48;5;232m");
    PFto_ADD("B_Gray20", "\033[48;5;236m");
    PFto_ADD("B_Gray21", "\033[48;5;237m");
    PFto_ADD("B_Gray22", "\033[48;5;237m");
    PFto_ADD("B_Gray23", "\033[48;5;237m");
    PFto_ADD("B_Gray24", "\033[48;5;237m");
    PFto_ADD("B_Gray25", "\033[48;5;238m");
    PFto_ADD("B_Gray26", "\033[48;5;238m");
    PFto_ADD("B_Gray27", "\033[48;5;238m");
    PFto_ADD("B_Gray28", "\033[48;5;238m");
    PFto_ADD("B_Gray29", "\033[48;5;239m");
    PFto_ADD("B_Gray3", "\033[48;5;232m");
    PFto_ADD("B_Gray30", "\033[48;5;239m");
    PFto_ADD("B_Gray31", "\033[48;5;239m");
    PFto_ADD("B_Gray32", "\033[48;5;8m");
    PFto_ADD("B_Gray33", "\033[48;5;8m");
    PFto_ADD("B_Gray34", "\033[48;5;240m");
    PFto_ADD("B_Gray35", "\033[48;5;240m");
    PFto_ADD("B_Gray36", "\033[48;5;240m");
    PFto_ADD("B_Gray37", "\033[48;5;241m");
    PFto_ADD("B_Gray38", "\033[48;5;241m");
    PFto_ADD("B_Gray39", "\033[48;5;241m");
    PFto_ADD("B_Gray4", "\033[48;5;232m");
    PFto_ADD("B_Gray40", "\033[48;5;241m");
    PFto_ADD("B_Gray41", "\033[48;5;242m");
    PFto_ADD("B_Gray42", "\033[48;5;242m");
    PFto_ADD("B_Gray43", "\033[48;5;242m");
    PFto_ADD("B_Gray44", "\033[48;5;242m");
    PFto_ADD("B_Gray45", "\033[48;5;243m");
    PFto_ADD("B_Gray46", "\033[48;5;243m");
    PFto_ADD("B_Gray47", "\033[48;5;243m");
    PFto_ADD("B_Gray48", "\033[48;5;243m");
    PFto_ADD("B_Gray49", "\033[48;5;244m");
    PFto_ADD("B_Gray5", "\033[48;5;232m");
    PFto_ADD("B_Gray50", "\033[48;5;244m");
    PFto_ADD("B_Gray51", "\033[48;5;244m");
    PFto_ADD("B_Gray52", "\033[48;5;102m");
    PFto_ADD("B_Gray53", "\033[48;5;102m");
    PFto_ADD("B_Gray54", "\033[48;5;245m");
    PFto_ADD("B_Gray55", "\033[48;5;245m");
    PFto_ADD("B_Gray56", "\033[48;5;245m");
    PFto_ADD("B_Gray57", "\033[48;5;246m");
    PFto_ADD("B_Gray58", "\033[48;5;246m");
    PFto_ADD("B_Gray59", "\033[48;5;246m");
    PFto_ADD("B_Gray6", "\033[48;5;233m");
    PFto_ADD("B_Gray60", "\033[48;5;246m");
    PFto_ADD("B_Gray61", "\033[48;5;247m");
    PFto_ADD("B_Gray62", "\033[48;5;247m");
    PFto_ADD("B_Gray63", "\033[48;5;247m");
    PFto_ADD("B_Gray64", "\033[48;5;247m");
    PFto_ADD("B_Gray65", "\033[48;5;248m");
    PFto_ADD("B_Gray66", "\033[48;5;248m");
    PFto_ADD("B_Gray67", "\033[48;5;248m");
    PFto_ADD("B_Gray68", "\033[48;5;248m");
    PFto_ADD("B_Gray69", "\033[48;5;249m");
    PFto_ADD("B_Gray7", "\033[48;5;233m");
    PFto_ADD("B_Gray70", "\033[48;5;249m");
    PFto_ADD("B_Gray71", "\033[48;5;249m");
    PFto_ADD("B_Gray72", "\033[48;5;7m");
    PFto_ADD("B_Gray73", "\033[48;5;7m");
    PFto_ADD("B_Gray74", "\033[48;5;250m");
    PFto_ADD("B_Gray75", "\033[48;5;250m");
    PFto_ADD("B_Gray76", "\033[48;5;251m");
    PFto_ADD("B_Gray77", "\033[48;5;251m");
    PFto_ADD("B_Gray78", "\033[48;5;251m");
    PFto_ADD("B_Gray79", "\033[48;5;251m");
    PFto_ADD("B_Gray8", "\033[48;5;233m");
    PFto_ADD("B_Gray80", "\033[48;5;252m");
    PFto_ADD("B_Gray81", "\033[48;5;252m");
    PFto_ADD("B_Gray82", "\033[48;5;252m");
    PFto_ADD("B_Gray83", "\033[48;5;252m");
    PFto_ADD("B_Gray84", "\033[48;5;253m");
    PFto_ADD("B_Gray85", "\033[48;5;253m");
    PFto_ADD("B_Gray86", "\033[48;5;253m");
    PFto_ADD("B_Gray87", "\033[48;5;188m");
    PFto_ADD("B_Gray88", "\033[48;5;188m");
    PFto_ADD("B_Gray89", "\033[48;5;254m");
    PFto_ADD("B_Gray9", "\033[48;5;233m");
    PFto_ADD("B_Gray90", "\033[48;5;254m");
    PFto_ADD("B_Gray91", "\033[48;5;254m");
    PFto_ADD("B_Gray92", "\033[48;5;255m");
    PFto_ADD("B_Gray93", "\033[48;5;255m");
    PFto_ADD("B_Gray94", "\033[48;5;255m");
    PFto_ADD("B_Gray95", "\033[48;5;255m");
    PFto_ADD("B_Gray96", "\033[48;5;255m");
    PFto_ADD("B_Gray97", "\033[48;5;15m");
    PFto_ADD("B_Gray98", "\033[48;5;15m");
    PFto_ADD("B_Gray99", "\033[48;5;15m");
    PFto_ADD("B_Green", "\033[48;5;46m");
    PFto_ADD("B_Green1", "\033[48;5;46m");
    PFto_ADD("B_Green2", "\033[48;5;40m");
    PFto_ADD("B_Green3", "\033[48;5;40m");
    PFto_ADD("B_Green4", "\033[48;5;28m");
    PFto_ADD("B_GreenYellow", "\033[48;5;155m");
    PFto_ADD("B_Grey", "\033[48;5;250m");
    PFto_ADD("B_Grey0", "\033[48;5;0m");
    PFto_ADD("B_Grey1", "\033[48;5;0m");
    PFto_ADD("B_Grey10", "\033[48;5;234m");
    PFto_ADD("B_Grey100", "\033[48;5;15m");
    PFto_ADD("B_Grey11", "\033[48;5;234m");
    PFto_ADD("B_Grey12", "\033[48;5;234m");
    PFto_ADD("B_Grey13", "\033[48;5;234m");
    PFto_ADD("B_Grey14", "\033[48;5;235m");
    PFto_ADD("B_Grey15", "\033[48;5;235m");
    PFto_ADD("B_Grey16", "\033[48;5;235m");
    PFto_ADD("B_Grey17", "\033[48;5;235m");
    PFto_ADD("B_Grey18", "\033[48;5;236m");
    PFto_ADD("B_Grey19", "\033[48;5;236m");
    PFto_ADD("B_Grey2", "\033[48;5;232m");
    PFto_ADD("B_Grey20", "\033[48;5;236m");
    PFto_ADD("B_Grey21", "\033[48;5;237m");
    PFto_ADD("B_Grey22", "\033[48;5;237m");
    PFto_ADD("B_Grey23", "\033[48;5;237m");
    PFto_ADD("B_Grey24", "\033[48;5;237m");
    PFto_ADD("B_Grey25", "\033[48;5;238m");
    PFto_ADD("B_Grey26", "\033[48;5;238m");
    PFto_ADD("B_Grey27", "\033[48;5;238m");
    PFto_ADD("B_Grey28", "\033[48;5;238m");
    PFto_ADD("B_Grey29", "\033[48;5;239m");
    PFto_ADD("B_Grey3", "\033[48;5;232m");
    PFto_ADD("B_Grey30", "\033[48;5;239m");
    PFto_ADD("B_Grey31", "\033[48;5;239m");
    PFto_ADD("B_Grey32", "\033[48;5;8m");
    PFto_ADD("B_Grey33", "\033[48;5;8m");
    PFto_ADD("B_Grey34", "\033[48;5;240m");
    PFto_ADD("B_Grey35", "\033[48;5;240m");
    PFto_ADD("B_Grey36", "\033[48;5;240m");
    PFto_ADD("B_Grey37", "\033[48;5;241m");
    PFto_ADD("B_Grey38", "\033[48;5;241m");
    PFto_ADD("B_Grey39", "\033[48;5;241m");
    PFto_ADD("B_Grey4", "\033[48;5;232m");
    PFto_ADD("B_Grey40", "\033[48;5;241m");
    PFto_ADD("B_Grey41", "\033[48;5;242m");
    PFto_ADD("B_Grey42", "\033[48;5;242m");
    PFto_ADD("B_Grey43", "\033[48;5;242m");
    PFto_ADD("B_Grey44", "\033[48;5;242m");
    PFto_ADD("B_Grey45", "\033[48;5;243m");
    PFto_ADD("B_Grey46", "\033[48;5;243m");
    PFto_ADD("B_Grey47", "\033[48;5;243m");
    PFto_ADD("B_Grey48", "\033[48;5;243m");
    PFto_ADD("B_Grey49", "\033[48;5;244m");
    PFto_ADD("B_Grey5", "\033[48;5;232m");
    PFto_ADD("B_Grey50", "\033[48;5;244m");
    PFto_ADD("B_Grey51", "\033[48;5;244m");
    PFto_ADD("B_Grey52", "\033[48;5;102m");
    PFto_ADD("B_Grey53", "\033[48;5;102m");
    PFto_ADD("B_Grey54", "\033[48;5;245m");
    PFto_ADD("B_Grey55", "\033[48;5;245m");
    PFto_ADD("B_Grey56", "\033[48;5;245m");
    PFto_ADD("B_Grey57", "\033[48;5;246m");
    PFto_ADD("B_Grey58", "\033[48;5;246m");
    PFto_ADD("B_Grey59", "\033[48;5;246m");
    PFto_ADD("B_Grey6", "\033[48;5;233m");
    PFto_ADD("B_Grey60", "\033[48;5;246m");
    PFto_ADD("B_Grey61", "\033[48;5;247m");
    PFto_ADD("B_Grey62", "\033[48;5;247m");
    PFto_ADD("B_Grey63", "\033[48;5;247m");
    PFto_ADD("B_Grey64", "\033[48;5;247m");
    PFto_ADD("B_Grey65", "\033[48;5;248m");
    PFto_ADD("B_Grey66", "\033[48;5;248m");
    PFto_ADD("B_Grey67", "\033[48;5;248m");
    PFto_ADD("B_Grey68", "\033[48;5;248m");
    PFto_ADD("B_Grey69", "\033[48;5;249m");
    PFto_ADD("B_Grey7", "\033[48;5;233m");
    PFto_ADD("B_Grey70", "\033[48;5;249m");
    PFto_ADD("B_Grey71", "\033[48;5;249m");
    PFto_ADD("B_Grey72", "\033[48;5;7m");
    PFto_ADD("B_Grey73", "\033[48;5;7m");
    PFto_ADD("B_Grey74", "\033[48;5;250m");
    PFto_ADD("B_Grey75", "\033[48;5;250m");
    PFto_ADD("B_Grey76", "\033[48;5;251m");
    PFto_ADD("B_Grey77", "\033[48;5;251m");
    PFto_ADD("B_Grey78", "\033[48;5;251m");
    PFto_ADD("B_Grey79", "\033[48;5;251m");
    PFto_ADD("B_Grey8", "\033[48;5;233m");
    PFto_ADD("B_Grey80", "\033[48;5;252m");
    PFto_ADD("B_Grey81", "\033[48;5;252m");
    PFto_ADD("B_Grey82", "\033[48;5;252m");
    PFto_ADD("B_Grey83", "\033[48;5;252m");
    PFto_ADD("B_Grey84", "\033[48;5;253m");
    PFto_ADD("B_Grey85", "\033[48;5;253m");
    PFto_ADD("B_Grey86", "\033[48;5;253m");
    PFto_ADD("B_Grey87", "\033[48;5;188m");
    PFto_ADD("B_Grey88", "\033[48;5;188m");
    PFto_ADD("B_Grey89", "\033[48;5;254m");
    PFto_ADD("B_Grey9", "\033[48;5;233m");
    PFto_ADD("B_Grey90", "\033[48;5;254m");
    PFto_ADD("B_Grey91", "\033[48;5;254m");
    PFto_ADD("B_Grey92", "\033[48;5;255m");
    PFto_ADD("B_Grey93", "\033[48;5;255m");
    PFto_ADD("B_Grey94", "\033[48;5;255m");
    PFto_ADD("B_Grey95", "\033[48;5;255m");
    PFto_ADD("B_Grey96", "\033[48;5;255m");
    PFto_ADD("B_Grey97", "\033[48;5;15m");
    PFto_ADD("B_Grey98", "\033[48;5;15m");
    PFto_ADD("B_Grey99", "\033[48;5;15m");
    PFto_ADD("B_Honeydew", "\033[48;5;255m");
    PFto_ADD("B_Honeydew1", "\033[48;5;255m");
    PFto_ADD("B_Honeydew2", "\033[48;5;254m");
    PFto_ADD("B_Honeydew3", "\033[48;5;251m");
    PFto_ADD("B_Honeydew4", "\033[48;5;102m");
    PFto_ADD("B_HotPink", "\033[48;5;205m");
    PFto_ADD("B_HotPink1", "\033[48;5;205m");
    PFto_ADD("B_HotPink2", "\033[48;5;169m");
    PFto_ADD("B_HotPink3", "\033[48;5;168m");
    PFto_ADD("B_HotPink4", "\033[48;5;95m");
    PFto_ADD("B_IndianRed", "\033[48;5;167m");
    PFto_ADD("B_IndianRed1", "\033[48;5;9m");
    PFto_ADD("B_IndianRed2", "\033[48;5;9m");
    PFto_ADD("B_IndianRed3", "\033[48;5;167m");
    PFto_ADD("B_IndianRed4", "\033[48;5;95m");
    PFto_ADD("B_Ivory", "\033[48;5;15m");
    PFto_ADD("B_Ivory1", "\033[48;5;15m");
    PFto_ADD("B_Ivory2", "\033[48;5;255m");
    PFto_ADD("B_Ivory3", "\033[48;5;251m");
    PFto_ADD("B_Ivory4", "\033[48;5;102m");
    PFto_ADD("B_Khaki", "\033[48;5;222m");
    PFto_ADD("B_Khaki1", "\033[48;5;228m");
    PFto_ADD("B_Khaki2", "\033[48;5;186m");
    PFto_ADD("B_Khaki3", "\033[48;5;180m");
    PFto_ADD("B_Khaki4", "\033[48;5;101m");
    PFto_ADD("B_LIGHTBLUE", "\033[48;5;12m");
    PFto_ADD("B_LIGHTCYAN", "\033[48;5;14m");
    PFto_ADD("B_LIGHTGREEN", "\033[48;5;10m");
    PFto_ADD("B_LIGHTRED", "\033[48;5;9m");
    PFto_ADD("B_Lavender", "\033[48;5;189m");
    PFto_ADD("B_LavenderBlush", "\033[48;5;15m");
    PFto_ADD("B_LavenderBlush1", "\033[48;5;15m");
    PFto_ADD("B_LavenderBlush2", "\033[48;5;254m");
    PFto_ADD("B_LavenderBlush3", "\033[48;5;251m");
    PFto_ADD("B_LavenderBlush4", "\033[48;5;102m");
    PFto_ADD("B_LawnGreen", "\033[48;5;118m");
    PFto_ADD("B_LemonChiffon", "\033[48;5;230m");
    PFto_ADD("B_LemonChiffon1", "\033[48;5;230m");
    PFto_ADD("B_LemonChiffon2", "\033[48;5;187m");
    PFto_ADD("B_LemonChiffon3", "\033[48;5;181m");
    PFto_ADD("B_LemonChiffon4", "\033[48;5;244m");
    PFto_ADD("B_LightBlue", "\033[48;5;152m");
    PFto_ADD("B_LightBlue1", "\033[48;5;159m");
    PFto_ADD("B_LightBlue2", "\033[48;5;152m");
    PFto_ADD("B_LightBlue3", "\033[48;5;110m");
    PFto_ADD("B_LightBlue4", "\033[48;5;66m");
    PFto_ADD("B_LightCoral", "\033[48;5;210m");
    PFto_ADD("B_LightCyan", "\033[48;5;195m");
    PFto_ADD("B_LightCyan1", "\033[48;5;195m");
    PFto_ADD("B_LightCyan2", "\033[48;5;254m");
    PFto_ADD("B_LightCyan3", "\033[48;5;251m");
    PFto_ADD("B_LightCyan4", "\033[48;5;102m");
    PFto_ADD("B_LightGoldenrod", "\033[48;5;186m");
    PFto_ADD("B_LightGoldenrod1", "\033[48;5;222m");
    PFto_ADD("B_LightGoldenrod2", "\033[48;5;186m");
    PFto_ADD("B_LightGoldenrod3", "\033[48;5;180m");
    PFto_ADD("B_LightGoldenrod4", "\033[48;5;101m");
    PFto_ADD("B_LightGoldenrodYellow", "\033[48;5;230m");
    PFto_ADD("B_LightGray", "\033[48;5;252m");
    PFto_ADD("B_LightGreen", "\033[48;5;114m");
    PFto_ADD("B_LightGrey", "\033[48;5;252m");
    PFto_ADD("B_LightPink", "\033[48;5;217m");
    PFto_ADD("B_LightPink1", "\033[48;5;217m");
    PFto_ADD("B_LightPink2", "\033[48;5;181m");
    PFto_ADD("B_LightPink3", "\033[48;5;174m");
    PFto_ADD("B_LightPink4", "\033[48;5;95m");
    PFto_ADD("B_LightSalmon", "\033[48;5;210m");
    PFto_ADD("B_LightSalmon1", "\033[48;5;210m");
    PFto_ADD("B_LightSalmon2", "\033[48;5;174m");
    PFto_ADD("B_LightSalmon3", "\033[48;5;173m");
    PFto_ADD("B_LightSalmon4", "\033[48;5;95m");
    PFto_ADD("B_LightSeaGreen", "\033[48;5;6m");
    PFto_ADD("B_LightSkyBlue", "\033[48;5;117m");
    PFto_ADD("B_LightSkyBlue1", "\033[48;5;153m");
    PFto_ADD("B_LightSkyBlue2", "\033[48;5;152m");
    PFto_ADD("B_LightSkyBlue3", "\033[48;5;110m");
    PFto_ADD("B_LightSkyBlue4", "\033[48;5;66m");
    PFto_ADD("B_LightSlateBlue", "\033[48;5;105m");
    PFto_ADD("B_LightSlateGray", "\033[48;5;102m");
    PFto_ADD("B_LightSlateGrey", "\033[48;5;102m");
    PFto_ADD("B_LightSteelBlue", "\033[48;5;146m");
    PFto_ADD("B_LightSteelBlue1", "\033[48;5;153m");
    PFto_ADD("B_LightSteelBlue2", "\033[48;5;152m");
    PFto_ADD("B_LightSteelBlue3", "\033[48;5;146m");
    PFto_ADD("B_LightSteelBlue4", "\033[48;5;244m");
    PFto_ADD("B_LightYellow", "\033[48;5;230m");
    PFto_ADD("B_LightYellow1", "\033[48;5;230m");
    PFto_ADD("B_LightYellow2", "\033[48;5;254m");
    PFto_ADD("B_LightYellow3", "\033[48;5;251m");
    PFto_ADD("B_LightYellow4", "\033[48;5;102m");
    PFto_ADD("B_LimeGreen", "\033[48;5;77m");
    PFto_ADD("B_Linen", "\033[48;5;255m");
    PFto_ADD("B_MAGENTA", "\033[48;5;5m");
    PFto_ADD("B_Magenta", "\033[48;5;201m");
    PFto_ADD("B_Magenta1", "\033[48;5;201m");
    PFto_ADD("B_Magenta2", "\033[48;5;164m");
    PFto_ADD("B_Magenta3", "\033[48;5;164m");
    PFto_ADD("B_Magenta4", "\033[48;5;90m");
    PFto_ADD("B_Maroon", "\033[48;5;131m");
    PFto_ADD("B_Maroon1", "\033[48;5;205m");
    PFto_ADD("B_Maroon2", "\033[48;5;169m");
    PFto_ADD("B_Maroon3", "\033[48;5;162m");
    PFto_ADD("B_Maroon4", "\033[48;5;89m");
    PFto_ADD("B_MediumAquamarine", "\033[48;5;79m");
    PFto_ADD("B_MediumBlue", "\033[48;5;20m");
    PFto_ADD("B_MediumOrchid", "\033[48;5;134m");
    PFto_ADD("B_MediumOrchid1", "\033[48;5;171m");
    PFto_ADD("B_MediumOrchid2", "\033[48;5;170m");
    PFto_ADD("B_MediumOrchid3", "\033[48;5;134m");
    PFto_ADD("B_MediumOrchid4", "\033[48;5;96m");
    PFto_ADD("B_MediumPurple", "\033[48;5;104m");
    PFto_ADD("B_MediumPurple1", "\033[48;5;141m");
    PFto_ADD("B_MediumPurple2", "\033[48;5;104m");
    PFto_ADD("B_MediumPurple3", "\033[48;5;98m");
    PFto_ADD("B_MediumPurple4", "\033[48;5;60m");
    PFto_ADD("B_MediumSeaGreen", "\033[48;5;72m");
    PFto_ADD("B_MediumSlateBlue", "\033[48;5;98m");
    PFto_ADD("B_MediumSpringGreen", "\033[48;5;48m");
    PFto_ADD("B_MediumTurquoise", "\033[48;5;79m");
    PFto_ADD("B_MediumVioletRed", "\033[48;5;126m");
    PFto_ADD("B_MidnightBlue", "\033[48;5;18m");
    PFto_ADD("B_MintCream", "\033[48;5;15m");
    PFto_ADD("B_MistyRose", "\033[48;5;224m");
    PFto_ADD("B_MistyRose1", "\033[48;5;224m");
    PFto_ADD("B_MistyRose2", "\033[48;5;188m");
    PFto_ADD("B_MistyRose3", "\033[48;5;181m");
    PFto_ADD("B_MistyRose4", "\033[48;5;244m");
    PFto_ADD("B_Moccasin", "\033[48;5;223m");
    PFto_ADD("B_NavajoWhite", "\033[48;5;223m");
    PFto_ADD("B_NavajoWhite1", "\033[48;5;223m");
    PFto_ADD("B_NavajoWhite2", "\033[48;5;186m");
    PFto_ADD("B_NavajoWhite3", "\033[48;5;180m");
    PFto_ADD("B_NavajoWhite4", "\033[48;5;101m");
    PFto_ADD("B_Navy", "\033[48;5;18m");
    PFto_ADD("B_NavyBlue", "\033[48;5;18m");
    PFto_ADD("B_ORANGE", "\033[48;5;3m");
    PFto_ADD("B_OldLace", "\033[48;5;230m");
    PFto_ADD("B_OliveDrab", "\033[48;5;64m");
    PFto_ADD("B_OliveDrab1", "\033[48;5;155m");
    PFto_ADD("B_OliveDrab2", "\033[48;5;149m");
    PFto_ADD("B_OliveDrab3", "\033[48;5;113m");
    PFto_ADD("B_OliveDrab4", "\033[48;5;64m");
    PFto_ADD("B_Orange", "\033[48;5;214m");
    PFto_ADD("B_Orange1", "\033[48;5;214m");
    PFto_ADD("B_Orange2", "\033[48;5;172m");
    PFto_ADD("B_Orange3", "\033[48;5;172m");
    PFto_ADD("B_Orange4", "\033[48;5;94m");
    PFto_ADD("B_OrangeRed", "\033[48;5;202m");
    PFto_ADD("B_OrangeRed1", "\033[48;5;202m");
    PFto_ADD("B_OrangeRed2", "\033[48;5;166m");
    PFto_ADD("B_OrangeRed3", "\033[48;5;166m");
    PFto_ADD("B_OrangeRed4", "\033[48;5;88m");
    PFto_ADD("B_Orchid", "\033[48;5;176m");
    PFto_ADD("B_Orchid1", "\033[48;5;213m");
    PFto_ADD("B_Orchid2", "\033[48;5;176m");
    PFto_ADD("B_Orchid3", "\033[48;5;169m");
    PFto_ADD("B_Orchid4", "\033[48;5;96m");
    PFto_ADD("B_PINK", "\033[48;5;13m");
    PFto_ADD("B_PaleGoldenrod", "\033[48;5;187m");
    PFto_ADD("B_PaleGreen", "\033[48;5;120m");
    PFto_ADD("B_PaleGreen1", "\033[48;5;120m");
    PFto_ADD("B_PaleGreen2", "\033[48;5;114m");
    PFto_ADD("B_PaleGreen3", "\033[48;5;114m");
    PFto_ADD("B_PaleGreen4", "\033[48;5;65m");
    PFto_ADD("B_PaleTurquoise", "\033[48;5;152m");
    PFto_ADD("B_PaleTurquoise1", "\033[48;5;159m");
    PFto_ADD("B_PaleTurquoise2", "\033[48;5;152m");
    PFto_ADD("B_PaleTurquoise3", "\033[48;5;116m");
    PFto_ADD("B_PaleTurquoise4", "\033[48;5;66m");
    PFto_ADD("B_PaleVioletRed", "\033[48;5;174m");
    PFto_ADD("B_PaleVioletRed1", "\033[48;5;211m");
    PFto_ADD("B_PaleVioletRed2", "\033[48;5;174m");
    PFto_ADD("B_PaleVioletRed3", "\033[48;5;168m");
    PFto_ADD("B_PaleVioletRed4", "\033[48;5;95m");
    PFto_ADD("B_PapayaWhip", "\033[48;5;230m");
    PFto_ADD("B_PeachPuff", "\033[48;5;223m");
    PFto_ADD("B_PeachPuff1", "\033[48;5;223m");
    PFto_ADD("B_PeachPuff2", "\033[48;5;181m");
    PFto_ADD("B_PeachPuff3", "\033[48;5;180m");
    PFto_ADD("B_PeachPuff4", "\033[48;5;101m");
    PFto_ADD("B_Peru", "\033[48;5;173m");
    PFto_ADD("B_Pink", "\033[48;5;217m");
    PFto_ADD("B_Pink1", "\033[48;5;217m");
    PFto_ADD("B_Pink2", "\033[48;5;181m");
    PFto_ADD("B_Pink3", "\033[48;5;174m");
    PFto_ADD("B_Pink4", "\033[48;5;95m");
    PFto_ADD("B_Plum", "\033[48;5;176m");
    PFto_ADD("B_Plum1", "\033[48;5;219m");
    PFto_ADD("B_Plum2", "\033[48;5;182m");
    PFto_ADD("B_Plum3", "\033[48;5;176m");
    PFto_ADD("B_Plum4", "\033[48;5;96m");
    PFto_ADD("B_PowderBlue", "\033[48;5;152m");
    PFto_ADD("B_Purple", "\033[48;5;93m");
    PFto_ADD("B_Purple1", "\033[48;5;99m");
    PFto_ADD("B_Purple2", "\033[48;5;98m");
    PFto_ADD("B_Purple3", "\033[48;5;92m");
    PFto_ADD("B_Purple4", "\033[48;5;54m");
    PFto_ADD("B_RED", "\033[48;5;1m");
    PFto_ADD("B_Red", "\033[48;5;196m");
    PFto_ADD("B_Red1", "\033[48;5;196m");
    PFto_ADD("B_Red2", "\033[48;5;160m");
    PFto_ADD("B_Red3", "\033[48;5;160m");
    PFto_ADD("B_Red4", "\033[48;5;88m");
    PFto_ADD("B_RosyBrown", "\033[48;5;138m");
    PFto_ADD("B_RosyBrown1", "\033[48;5;217m");
    PFto_ADD("B_RosyBrown2", "\033[48;5;181m");
    PFto_ADD("B_RosyBrown3", "\033[48;5;174m");
    PFto_ADD("B_RosyBrown4", "\033[48;5;243m");
    PFto_ADD("B_RoyalBlue", "\033[48;5;62m");
    PFto_ADD("B_RoyalBlue1", "\033[48;5;69m");
    PFto_ADD("B_RoyalBlue2", "\033[48;5;12m");
    PFto_ADD("B_RoyalBlue3", "\033[48;5;62m");
    PFto_ADD("B_RoyalBlue4", "\033[48;5;24m");
    PFto_ADD("B_SaddleBrown", "\033[48;5;94m");
    PFto_ADD("B_Salmon", "\033[48;5;210m");
    PFto_ADD("B_Salmon1", "\033[48;5;209m");
    PFto_ADD("B_Salmon2", "\033[48;5;173m");
    PFto_ADD("B_Salmon3", "\033[48;5;173m");
    PFto_ADD("B_Salmon4", "\033[48;5;95m");
    PFto_ADD("B_SandyBrown", "\033[48;5;215m");
    PFto_ADD("B_SeaGreen", "\033[48;5;65m");
    PFto_ADD("B_SeaGreen1", "\033[48;5;84m");
    PFto_ADD("B_SeaGreen2", "\033[48;5;78m");
    PFto_ADD("B_SeaGreen3", "\033[48;5;78m");
    PFto_ADD("B_SeaGreen4", "\033[48;5;65m");
    PFto_ADD("B_Seashell", "\033[48;5;255m");
    PFto_ADD("B_Seashell1", "\033[48;5;255m");
    PFto_ADD("B_Seashell2", "\033[48;5;254m");
    PFto_ADD("B_Seashell3", "\033[48;5;251m");
    PFto_ADD("B_Seashell4", "\033[48;5;102m");
    PFto_ADD("B_Sienna", "\033[48;5;95m");
    PFto_ADD("B_Sienna1", "\033[48;5;209m");
    PFto_ADD("B_Sienna2", "\033[48;5;173m");
    PFto_ADD("B_Sienna3", "\033[48;5;167m");
    PFto_ADD("B_Sienna4", "\033[48;5;94m");
    PFto_ADD("B_SkyBlue", "\033[48;5;116m");
    PFto_ADD("B_SkyBlue1", "\033[48;5;117m");
    PFto_ADD("B_SkyBlue2", "\033[48;5;110m");
    PFto_ADD("B_SkyBlue3", "\033[48;5;74m");
    PFto_ADD("B_SkyBlue4", "\033[48;5;66m");
    PFto_ADD("B_SlateBlue", "\033[48;5;62m");
    PFto_ADD("B_SlateBlue1", "\033[48;5;105m");
    PFto_ADD("B_SlateBlue2", "\033[48;5;98m");
    PFto_ADD("B_SlateBlue3", "\033[48;5;62m");
    PFto_ADD("B_SlateBlue4", "\033[48;5;60m");
    PFto_ADD("B_SlateGray", "\033[48;5;244m");
    PFto_ADD("B_SlateGray1", "\033[48;5;153m");
    PFto_ADD("B_SlateGray2", "\033[48;5;152m");
    PFto_ADD("B_SlateGray3", "\033[48;5;110m");
    PFto_ADD("B_SlateGray4", "\033[48;5;244m");
    PFto_ADD("B_SlateGrey", "\033[48;5;244m");
    PFto_ADD("B_Snow", "\033[48;5;15m");
    PFto_ADD("B_Snow1", "\033[48;5;15m");
    PFto_ADD("B_Snow2", "\033[48;5;255m");
    PFto_ADD("B_Snow3", "\033[48;5;251m");
    PFto_ADD("B_Snow4", "\033[48;5;245m");
    PFto_ADD("B_SpringGreen", "\033[48;5;48m");
    PFto_ADD("B_SpringGreen1", "\033[48;5;48m");
    PFto_ADD("B_SpringGreen2", "\033[48;5;42m");
    PFto_ADD("B_SpringGreen3", "\033[48;5;41m");
    PFto_ADD("B_SpringGreen4", "\033[48;5;29m");
    PFto_ADD("B_SteelBlue", "\033[48;5;67m");
    PFto_ADD("B_SteelBlue1", "\033[48;5;75m");
    PFto_ADD("B_SteelBlue2", "\033[48;5;74m");
    PFto_ADD("B_SteelBlue3", "\033[48;5;68m");
    PFto_ADD("B_SteelBlue4", "\033[48;5;60m");
    PFto_ADD("B_Tan", "\033[48;5;180m");
    PFto_ADD("B_Tan1", "\033[48;5;215m");
    PFto_ADD("B_Tan2", "\033[48;5;173m");
    PFto_ADD("B_Tan3", "\033[48;5;173m");
    PFto_ADD("B_Tan4", "\033[48;5;95m");
    PFto_ADD("B_Thistle", "\033[48;5;182m");
    PFto_ADD("B_Thistle1", "\033[48;5;225m");
    PFto_ADD("B_Thistle2", "\033[48;5;254m");
    PFto_ADD("B_Thistle3", "\033[48;5;251m");
    PFto_ADD("B_Thistle4", "\033[48;5;102m");
    PFto_ADD("B_Tomato", "\033[48;5;9m");
    PFto_ADD("B_Tomato1", "\033[48;5;9m");
    PFto_ADD("B_Tomato2", "\033[48;5;9m");
    PFto_ADD("B_Tomato3", "\033[48;5;167m");
    PFto_ADD("B_Tomato4", "\033[48;5;94m");
    PFto_ADD("B_Turquoise", "\033[48;5;80m");
    PFto_ADD("B_Turquoise1", "\033[48;5;51m");
    PFto_ADD("B_Turquoise2", "\033[48;5;44m");
    PFto_ADD("B_Turquoise3", "\033[48;5;38m");
    PFto_ADD("B_Turquoise4", "\033[48;5;30m");
    PFto_ADD("B_Violet", "\033[48;5;176m");
    PFto_ADD("B_VioletRed", "\033[48;5;162m");
    PFto_ADD("B_VioletRed1", "\033[48;5;204m");
    PFto_ADD("B_VioletRed2", "\033[48;5;168m");
    PFto_ADD("B_VioletRed3", "\033[48;5;168m");
    PFto_ADD("B_VioletRed4", "\033[48;5;89m");
    PFto_ADD("B_WHITE", "\033[48;5;15m");
    PFto_ADD("B_Wheat", "\033[48;5;223m");
    PFto_ADD("B_Wheat1", "\033[48;5;223m");
    PFto_ADD("B_Wheat2", "\033[48;5;187m");
    PFto_ADD("B_Wheat3", "\033[48;5;180m");
    PFto_ADD("B_Wheat4", "\033[48;5;101m");
    PFto_ADD("B_White", "\033[48;5;15m");
    PFto_ADD("B_WhiteSmoke", "\033[48;5;255m");
    PFto_ADD("B_YELLOW", "\033[48;5;11m");
    PFto_ADD("B_Yellow", "\033[48;5;226m");
    PFto_ADD("B_Yellow1", "\033[48;5;226m");
    PFto_ADD("B_Yellow2", "\033[48;5;184m");
    PFto_ADD("B_Yellow3", "\033[48;5;184m");
    PFto_ADD("B_Yellow4", "\033[48;5;100m");
    PFto_ADD("B_YellowGreen", "\033[48;5;113m");
    PFto_ADD("Beige", "\033[38;5;230m");
    PFto_ADD("Bisque", "\033[38;5;223m");
    PFto_ADD("Bisque1", "\033[38;5;223m");
    PFto_ADD("Bisque2", "\033[38;5;187m");
    PFto_ADD("Bisque3", "\033[38;5;180m");
    PFto_ADD("Bisque4", "\033[38;5;244m");
    PFto_ADD("Black", "\033[38;5;0m");
    PFto_ADD("BlanchedAlmond", "\033[38;5;224m");
    PFto_ADD("Blue", "\033[38;5;21m");
    PFto_ADD("Blue1", "\033[38;5;21m");
    PFto_ADD("Blue2", "\033[38;5;20m");
    PFto_ADD("Blue3", "\033[38;5;20m");
    PFto_ADD("Blue4", "\033[38;5;18m");
    PFto_ADD("BlueViolet", "\033[38;5;98m");
    PFto_ADD("Brown", "\033[38;5;1m");
    PFto_ADD("Brown1", "\033[38;5;9m");
    PFto_ADD("Brown2", "\033[38;5;9m");
    PFto_ADD("Brown3", "\033[38;5;167m");
    PFto_ADD("Brown4", "\033[38;5;88m");
    PFto_ADD("Burlywood", "\033[38;5;180m");
    PFto_ADD("Burlywood1", "\033[38;5;222m");
    PFto_ADD("Burlywood2", "\033[38;5;180m");
    PFto_ADD("Burlywood3", "\033[38;5;180m");
    PFto_ADD("Burlywood4", "\033[38;5;101m");
    PFto_ADD("CLEARLINE", "\033[L\033[G");
    PFto_ADD("CURS_DOWN", "\033[B");
    PFto_ADD("CURS_LEFT", "\033[D");
    PFto_ADD("CURS_RIGHT", "\033[C");
    PFto_ADD("CURS_UP", "\033[A");
    PFto_ADD("CYAN", "\033[38;5;6m");
    PFto_ADD("CadetBlue", "\033[38;5;66m");
    PFto_ADD("CadetBlue1", "\033[38;5;123m");
    PFto_ADD("CadetBlue2", "\033[38;5;116m");
    PFto_ADD("CadetBlue3", "\033[38;5;110m");
    PFto_ADD("CadetBlue4", "\033[38;5;66m");
    PFto_ADD("Chartreuse", "\033[38;5;118m");
    PFto_ADD("Chartreuse1", "\033[38;5;118m");
    PFto_ADD("Chartreuse2", "\033[38;5;112m");
    PFto_ADD("Chartreuse3", "\033[38;5;76m");
    PFto_ADD("Chartreuse4", "\033[38;5;64m");
    PFto_ADD("Chocolate", "\033[38;5;166m");
    PFto_ADD("Chocolate1", "\033[38;5;208m");
    PFto_ADD("Chocolate2", "\033[38;5;172m");
    PFto_ADD("Chocolate3", "\033[38;5;166m");
    PFto_ADD("Chocolate4", "\033[38;5;94m");
    PFto_ADD("Coral", "\033[38;5;209m");
    PFto_ADD("Coral1", "\033[38;5;209m");
    PFto_ADD("Coral2", "\033[38;5;9m");
    PFto_ADD("Coral3", "\033[38;5;167m");
    PFto_ADD("Coral4", "\033[38;5;95m");
    PFto_ADD("CornflowerBlue", "\033[38;5;68m");
    PFto_ADD("Cornsilk", "\033[38;5;230m");
    PFto_ADD("Cornsilk1", "\033[38;5;230m");
    PFto_ADD("Cornsilk2", "\033[38;5;254m");
    PFto_ADD("Cornsilk3", "\033[38;5;251m");
    PFto_ADD("Cornsilk4", "\033[38;5;244m");
    PFto_ADD("Cyan", "\033[38;5;51m");
    PFto_ADD("Cyan1", "\033[38;5;51m");
    PFto_ADD("Cyan2", "\033[38;5;44m");
    PFto_ADD("Cyan3", "\033[38;5;44m");
    PFto_ADD("Cyan4", "\033[38;5;30m");
    PFto_ADD("DARKGREY", "\033[38;5;8m");
    PFto_ADD("DarkBlue", "\033[38;5;18m");
    PFto_ADD("DarkCyan", "\033[38;5;30m");
    PFto_ADD("DarkGoldenrod", "\033[38;5;136m");
    PFto_ADD("DarkGoldenrod1", "\033[38;5;214m");
    PFto_ADD("DarkGoldenrod2", "\033[38;5;178m");
    PFto_ADD("DarkGoldenrod3", "\033[38;5;172m");
    PFto_ADD("DarkGoldenrod4", "\033[38;5;94m");
    PFto_ADD("DarkGray", "\033[38;5;248m");
    PFto_ADD("DarkGreen", "\033[38;5;22m");
    PFto_ADD("DarkGrey", "\033[38;5;248m");
    PFto_ADD("DarkKhaki", "\033[38;5;143m");
    PFto_ADD("DarkMagenta", "\033[38;5;90m");
    PFto_ADD("DarkOliveGreen", "\033[38;5;239m");
    PFto_ADD("DarkOliveGreen1", "\033[38;5;156m");
    PFto_ADD("DarkOliveGreen2", "\033[38;5;149m");
    PFto_ADD("DarkOliveGreen3", "\033[38;5;149m");
    PFto_ADD("DarkOliveGreen4", "\033[38;5;65m");
    PFto_ADD("DarkOrange", "\033[38;5;208m");
    PFto_ADD("DarkOrange1", "\033[38;5;208m");
    PFto_ADD("DarkOrange2", "\033[38;5;172m");
    PFto_ADD("DarkOrange3", "\033[38;5;166m");
    PFto_ADD("DarkOrange4", "\033[38;5;94m");
    PFto_ADD("DarkOrchid", "\033[38;5;97m");
    PFto_ADD("DarkOrchid1", "\033[38;5;135m");
    PFto_ADD("DarkOrchid2", "\033[38;5;134m");
    PFto_ADD("DarkOrchid3", "\033[38;5;98m");
    PFto_ADD("DarkOrchid4", "\033[38;5;54m");
    PFto_ADD("DarkRed", "\033[38;5;88m");
    PFto_ADD("DarkSalmon", "\033[38;5;174m");
    PFto_ADD("DarkSeaGreen", "\033[38;5;108m");
    PFto_ADD("DarkSeaGreen1", "\033[38;5;157m");
    PFto_ADD("DarkSeaGreen2", "\033[38;5;151m");
    PFto_ADD("DarkSeaGreen3", "\033[38;5;114m");
    PFto_ADD("DarkSeaGreen4", "\033[38;5;243m");
    PFto_ADD("DarkSlateBlue", "\033[38;5;60m");
    PFto_ADD("DarkSlateGray", "\033[38;5;238m");
    PFto_ADD("DarkSlateGray1", "\033[38;5;123m");
    PFto_ADD("DarkSlateGray2", "\033[38;5;116m");
    PFto_ADD("DarkSlateGray3", "\033[38;5;116m");
    PFto_ADD("DarkSlateGray4", "\033[38;5;66m");
    PFto_ADD("DarkSlateGrey", "\033[38;5;238m");
    PFto_ADD("DarkTurquoise", "\033[38;5;44m");
    PFto_ADD("DarkViolet", "\033[38;5;92m");
    PFto_ADD("DeepPink", "\033[38;5;198m");
    PFto_ADD("DeepPink1", "\033[38;5;198m");
    PFto_ADD("DeepPink2", "\033[38;5;162m");
    PFto_ADD("DeepPink3", "\033[38;5;162m");
    PFto_ADD("DeepPink4", "\033[38;5;89m");
    PFto_ADD("DeepSkyBlue", "\033[38;5;39m");
    PFto_ADD("DeepSkyBlue1", "\033[38;5;39m");
    PFto_ADD("DeepSkyBlue2", "\033[38;5;38m");
    PFto_ADD("DeepSkyBlue3", "\033[38;5;32m");
    PFto_ADD("DeepSkyBlue4", "\033[38;5;24m");
    PFto_ADD("DimGray", "\033[38;5;242m");
    PFto_ADD("DimGrey", "\033[38;5;242m");
    PFto_ADD("DodgerBlue", "\033[38;5;33m");
    PFto_ADD("DodgerBlue1", "\033[38;5;33m");
    PFto_ADD("DodgerBlue2", "\033[38;5;32m");
    PFto_ADD("DodgerBlue3", "\033[38;5;32m");
    PFto_ADD("DodgerBlue4", "\033[38;5;24m");
    PFto_ADD("ENDTERM", "");
    PFto_ADD("F000", "\033[38;5;16m");
    PFto_ADD("F001", "\033[38;5;17m");
    PFto_ADD("F002", "\033[38;5;18m");
    PFto_ADD("F003", "\033[38;5;19m");
    PFto_ADD("F004", "\033[38;5;20m");
    PFto_ADD("F005", "\033[38;5;21m");
    PFto_ADD("F010", "\033[38;5;22m");
    PFto_ADD("F011", "\033[38;5;23m");
    PFto_ADD("F012", "\033[38;5;24m");
    PFto_ADD("F013", "\033[38;5;25m");
    PFto_ADD("F014", "\033[38;5;26m");
    PFto_ADD("F015", "\033[38;5;27m");
    PFto_ADD("F020", "\033[38;5;28m");
    PFto_ADD("F021", "\033[38;5;29m");
    PFto_ADD("F022", "\033[38;5;30m");
    PFto_ADD("F023", "\033[38;5;31m");
    PFto_ADD("F024", "\033[38;5;32m");
    PFto_ADD("F025", "\033[38;5;33m");
    PFto_ADD("F030", "\033[38;5;34m");
    PFto_ADD("F031", "\033[38;5;35m");
    PFto_ADD("F032", "\033[38;5;36m");
    PFto_ADD("F033", "\033[38;5;37m");
    PFto_ADD("F034", "\033[38;5;38m");
    PFto_ADD("F035", "\033[38;5;39m");
    PFto_ADD("F040", "\033[38;5;40m");
    PFto_ADD("F041", "\033[38;5;41m");
    PFto_ADD("F042", "\033[38;5;42m");
    PFto_ADD("F043", "\033[38;5;43m");
    PFto_ADD("F044", "\033[38;5;44m");
    PFto_ADD("F045", "\033[38;5;45m");
    PFto_ADD("F050", "\033[38;5;46m");
    PFto_ADD("F051", "\033[38;5;47m");
    PFto_ADD("F052", "\033[38;5;48m");
    PFto_ADD("F053", "\033[38;5;49m");
    PFto_ADD("F054", "\033[38;5;50m");
    PFto_ADD("F055", "\033[38;5;51m");
    PFto_ADD("F100", "\033[38;5;52m");
    PFto_ADD("F101", "\033[38;5;53m");
    PFto_ADD("F102", "\033[38;5;54m");
    PFto_ADD("F103", "\033[38;5;55m");
    PFto_ADD("F104", "\033[38;5;56m");
    PFto_ADD("F105", "\033[38;5;57m");
    PFto_ADD("F110", "\033[38;5;58m");
    PFto_ADD("F111", "\033[38;5;59m");
    PFto_ADD("F112", "\033[38;5;60m");
    PFto_ADD("F113", "\033[38;5;61m");
    PFto_ADD("F114", "\033[38;5;62m");
    PFto_ADD("F115", "\033[38;5;63m");
    PFto_ADD("F120", "\033[38;5;64m");
    PFto_ADD("F121", "\033[38;5;65m");
    PFto_ADD("F122", "\033[38;5;66m");
    PFto_ADD("F123", "\033[38;5;67m");
    PFto_ADD("F124", "\033[38;5;68m");
    PFto_ADD("F125", "\033[38;5;69m");
    PFto_ADD("F130", "\033[38;5;70m");
    PFto_ADD("F131", "\033[38;5;71m");
    PFto_ADD("F132", "\033[38;5;72m");
    PFto_ADD("F133", "\033[38;5;73m");
    PFto_ADD("F134", "\033[38;5;74m");
    PFto_ADD("F135", "\033[38;5;75m");
    PFto_ADD("F140", "\033[38;5;76m");
    PFto_ADD("F141", "\033[38;5;77m");
    PFto_ADD("F142", "\033[38;5;78m");
    PFto_ADD("F143", "\033[38;5;79m");
    PFto_ADD("F144", "\033[38;5;80m");
    PFto_ADD("F145", "\033[38;5;81m");
    PFto_ADD("F150", "\033[38;5;82m");
    PFto_ADD("F151", "\033[38;5;83m");
    PFto_ADD("F152", "\033[38;5;84m");
    PFto_ADD("F153", "\033[38;5;85m");
    PFto_ADD("F154", "\033[38;5;86m");
    PFto_ADD("F155", "\033[38;5;87m");
    PFto_ADD("F200", "\033[38;5;88m");
    PFto_ADD("F201", "\033[38;5;89m");
    PFto_ADD("F202", "\033[38;5;90m");
    PFto_ADD("F203", "\033[38;5;91m");
    PFto_ADD("F204", "\033[38;5;92m");
    PFto_ADD("F205", "\033[38;5;93m");
    PFto_ADD("F210", "\033[38;5;94m");
    PFto_ADD("F211", "\033[38;5;95m");
    PFto_ADD("F212", "\033[38;5;96m");
    PFto_ADD("F213", "\033[38;5;97m");
    PFto_ADD("F214", "\033[38;5;98m");
    PFto_ADD("F215", "\033[38;5;99m");
    PFto_ADD("F220", "\033[38;5;100m");
    PFto_ADD("F221", "\033[38;5;101m");
    PFto_ADD("F222", "\033[38;5;102m");
    PFto_ADD("F223", "\033[38;5;103m");
    PFto_ADD("F224", "\033[38;5;104m");
    PFto_ADD("F225", "\033[38;5;105m");
    PFto_ADD("F230", "\033[38;5;106m");
    PFto_ADD("F231", "\033[38;5;107m");
    PFto_ADD("F232", "\033[38;5;108m");
    PFto_ADD("F233", "\033[38;5;109m");
    PFto_ADD("F234", "\033[38;5;110m");
    PFto_ADD("F235", "\033[38;5;111m");
    PFto_ADD("F240", "\033[38;5;112m");
    PFto_ADD("F241", "\033[38;5;113m");
    PFto_ADD("F242", "\033[38;5;114m");
    PFto_ADD("F243", "\033[38;5;115m");
    PFto_ADD("F244", "\033[38;5;116m");
    PFto_ADD("F245", "\033[38;5;117m");
    PFto_ADD("F250", "\033[38;5;118m");
    PFto_ADD("F251", "\033[38;5;119m");
    PFto_ADD("F252", "\033[38;5;120m");
    PFto_ADD("F253", "\033[38;5;121m");
    PFto_ADD("F254", "\033[38;5;122m");
    PFto_ADD("F255", "\033[38;5;123m");
    PFto_ADD("F300", "\033[38;5;124m");
    PFto_ADD("F301", "\033[38;5;125m");
    PFto_ADD("F302", "\033[38;5;126m");
    PFto_ADD("F303", "\033[38;5;127m");
    PFto_ADD("F304", "\033[38;5;128m");
    PFto_ADD("F305", "\033[38;5;129m");
    PFto_ADD("F310", "\033[38;5;130m");
    PFto_ADD("F311", "\033[38;5;131m");
    PFto_ADD("F312", "\033[38;5;132m");
    PFto_ADD("F313", "\033[38;5;133m");
    PFto_ADD("F314", "\033[38;5;134m");
    PFto_ADD("F315", "\033[38;5;135m");
    PFto_ADD("F320", "\033[38;5;136m");
    PFto_ADD("F321", "\033[38;5;137m");
    PFto_ADD("F322", "\033[38;5;138m");
    PFto_ADD("F323", "\033[38;5;139m");
    PFto_ADD("F324", "\033[38;5;140m");
    PFto_ADD("F325", "\033[38;5;141m");
    PFto_ADD("F330", "\033[38;5;142m");
    PFto_ADD("F331", "\033[38;5;143m");
    PFto_ADD("F332", "\033[38;5;144m");
    PFto_ADD("F333", "\033[38;5;145m");
    PFto_ADD("F334", "\033[38;5;146m");
    PFto_ADD("F335", "\033[38;5;147m");
    PFto_ADD("F340", "\033[38;5;148m");
    PFto_ADD("F341", "\033[38;5;149m");
    PFto_ADD("F342", "\033[38;5;150m");
    PFto_ADD("F343", "\033[38;5;151m");
    PFto_ADD("F344", "\033[38;5;152m");
    PFto_ADD("F345", "\033[38;5;153m");
    PFto_ADD("F350", "\033[38;5;154m");
    PFto_ADD("F351", "\033[38;5;155m");
    PFto_ADD("F352", "\033[38;5;156m");
    PFto_ADD("F353", "\033[38;5;157m");
    PFto_ADD("F354", "\033[38;5;158m");
    PFto_ADD("F355", "\033[38;5;159m");
    PFto_ADD("F400", "\033[38;5;160m");
    PFto_ADD("F401", "\033[38;5;161m");
    PFto_ADD("F402", "\033[38;5;162m");
    PFto_ADD("F403", "\033[38;5;163m");
    PFto_ADD("F404", "\033[38;5;164m");
    PFto_ADD("F405", "\033[38;5;165m");
    PFto_ADD("F410", "\033[38;5;166m");
    PFto_ADD("F411", "\033[38;5;167m");
    PFto_ADD("F412", "\033[38;5;168m");
    PFto_ADD("F413", "\033[38;5;169m");
    PFto_ADD("F414", "\033[38;5;170m");
    PFto_ADD("F415", "\033[38;5;171m");
    PFto_ADD("F420", "\033[38;5;172m");
    PFto_ADD("F421", "\033[38;5;173m");
    PFto_ADD("F422", "\033[38;5;174m");
    PFto_ADD("F423", "\033[38;5;175m");
    PFto_ADD("F424", "\033[38;5;176m");
    PFto_ADD("F425", "\033[38;5;177m");
    PFto_ADD("F430", "\033[38;5;178m");
    PFto_ADD("F431", "\033[38;5;179m");
    PFto_ADD("F432", "\033[38;5;180m");
    PFto_ADD("F433", "\033[38;5;181m");
    PFto_ADD("F434", "\033[38;5;182m");
    PFto_ADD("F435", "\033[38;5;183m");
    PFto_ADD("F440", "\033[38;5;184m");
    PFto_ADD("F441", "\033[38;5;185m");
    PFto_ADD("F442", "\033[38;5;186m");
    PFto_ADD("F443", "\033[38;5;187m");
    PFto_ADD("F444", "\033[38;5;188m");
    PFto_ADD("F445", "\033[38;5;189m");
    PFto_ADD("F450", "\033[38;5;190m");
    PFto_ADD("F451", "\033[38;5;191m");
    PFto_ADD("F452", "\033[38;5;192m");
    PFto_ADD("F453", "\033[38;5;193m");
    PFto_ADD("F454", "\033[38;5;194m");
    PFto_ADD("F455", "\033[38;5;195m");
    PFto_ADD("F500", "\033[38;5;196m");
    PFto_ADD("F501", "\033[38;5;197m");
    PFto_ADD("F502", "\033[38;5;198m");
    PFto_ADD("F503", "\033[38;5;199m");
    PFto_ADD("F504", "\033[38;5;200m");
    PFto_ADD("F505", "\033[38;5;201m");
    PFto_ADD("F510", "\033[38;5;202m");
    PFto_ADD("F511", "\033[38;5;203m");
    PFto_ADD("F512", "\033[38;5;204m");
    PFto_ADD("F513", "\033[38;5;205m");
    PFto_ADD("F514", "\033[38;5;206m");
    PFto_ADD("F515", "\033[38;5;207m");
    PFto_ADD("F520", "\033[38;5;208m");
    PFto_ADD("F521", "\033[38;5;209m");
    PFto_ADD("F522", "\033[38;5;210m");
    PFto_ADD("F523", "\033[38;5;211m");
    PFto_ADD("F524", "\033[38;5;212m");
    PFto_ADD("F525", "\033[38;5;213m");
    PFto_ADD("F530", "\033[38;5;214m");
    PFto_ADD("F531", "\033[38;5;215m");
    PFto_ADD("F532", "\033[38;5;216m");
    PFto_ADD("F533", "\033[38;5;217m");
    PFto_ADD("F534", "\033[38;5;218m");
    PFto_ADD("F535", "\033[38;5;219m");
    PFto_ADD("F540", "\033[38;5;220m");
    PFto_ADD("F541", "\033[38;5;221m");
    PFto_ADD("F542", "\033[38;5;222m");
    PFto_ADD("F543", "\033[38;5;223m");
    PFto_ADD("F544", "\033[38;5;224m");
    PFto_ADD("F545", "\033[38;5;225m");
    PFto_ADD("F550", "\033[38;5;226m");
    PFto_ADD("F551", "\033[38;5;227m");
    PFto_ADD("F552", "\033[38;5;228m");
    PFto_ADD("F553", "\033[38;5;229m");
    PFto_ADD("F554", "\033[38;5;230m");
    PFto_ADD("F555", "\033[38;5;231m");
    PFto_ADD("FLASH", "\033[5m");
    PFto_ADD("Firebrick", "\033[38;5;1m");
    PFto_ADD("Firebrick1", "\033[38;5;9m");
    PFto_ADD("Firebrick2", "\033[38;5;9m");
    PFto_ADD("Firebrick3", "\033[38;5;160m");
    PFto_ADD("Firebrick4", "\033[38;5;88m");
    PFto_ADD("FloralWhite", "\033[38;5;15m");
    PFto_ADD("ForestGreen", "\033[38;5;28m");
    PFto_ADD("G00", "\033[38;5;0m");
    PFto_ADD("G01", "\033[38;5;232m");
    PFto_ADD("G02", "\033[38;5;233m");
    PFto_ADD("G03", "\033[38;5;234m");
    PFto_ADD("G04", "\033[38;5;235m");
    PFto_ADD("G05", "\033[38;5;236m");
    PFto_ADD("G06", "\033[38;5;237m");
    PFto_ADD("G07", "\033[38;5;238m");
    PFto_ADD("G08", "\033[38;5;239m");
    PFto_ADD("G09", "\033[38;5;240m");
    PFto_ADD("G10", "\033[38;5;241m");
    PFto_ADD("G11", "\033[38;5;242m");
    PFto_ADD("G12", "\033[38;5;243m");
    PFto_ADD("G13", "\033[38;5;244m");
    PFto_ADD("G14", "\033[38;5;245m");
    PFto_ADD("G15", "\033[38;5;246m");
    PFto_ADD("G16", "\033[38;5;247m");
    PFto_ADD("G17", "\033[38;5;248m");
    PFto_ADD("G18", "\033[38;5;249m");
    PFto_ADD("G19", "\033[38;5;250m");
    PFto_ADD("G20", "\033[38;5;251m");
    PFto_ADD("G21", "\033[38;5;252m");
    PFto_ADD("G22", "\033[38;5;253m");
    PFto_ADD("G23", "\033[38;5;254m");
    PFto_ADD("G24", "\033[38;5;255m");
    PFto_ADD("G25", "\033[38;5;15m");
    PFto_ADD("GREEN", "\033[38;5;2m");
    PFto_ADD("GREY", "\033[38;5;7m");
    PFto_ADD("Gainsboro", "\033[38;5;188m");
    PFto_ADD("GhostWhite", "\033[38;5;15m");
    PFto_ADD("Gold", "\033[38;5;220m");
    PFto_ADD("Gold1", "\033[38;5;220m");
    PFto_ADD("Gold2", "\033[38;5;178m");
    PFto_ADD("Gold3", "\033[38;5;178m");
    PFto_ADD("Gold4", "\033[38;5;100m");
    PFto_ADD("Goldenrod", "\033[38;5;178m");
    PFto_ADD("Goldenrod1", "\033[38;5;214m");
    PFto_ADD("Goldenrod2", "\033[38;5;178m");
    PFto_ADD("Goldenrod3", "\033[38;5;172m");
    PFto_ADD("Goldenrod4", "\033[38;5;94m");
    PFto_ADD("Gray", "\033[38;5;250m");
    PFto_ADD("Gray0", "\033[38;5;0m");
    PFto_ADD("Gray1", "\033[38;5;0m");
    PFto_ADD("Gray10", "\033[38;5;234m");
    PFto_ADD("Gray100", "\033[38;5;15m");
    PFto_ADD("Gray11", "\033[38;5;234m");
    PFto_ADD("Gray12", "\033[38;5;234m");
    PFto_ADD("Gray13", "\033[38;5;234m");
    PFto_ADD("Gray14", "\033[38;5;235m");
    PFto_ADD("Gray15", "\033[38;5;235m");
    PFto_ADD("Gray16", "\033[38;5;235m");
    PFto_ADD("Gray17", "\033[38;5;235m");
    PFto_ADD("Gray18", "\033[38;5;236m");
    PFto_ADD("Gray19", "\033[38;5;236m");
    PFto_ADD("Gray2", "\033[38;5;232m");
    PFto_ADD("Gray20", "\033[38;5;236m");
    PFto_ADD("Gray21", "\033[38;5;237m");
    PFto_ADD("Gray22", "\033[38;5;237m");
    PFto_ADD("Gray23", "\033[38;5;237m");
    PFto_ADD("Gray24", "\033[38;5;237m");
    PFto_ADD("Gray25", "\033[38;5;238m");
    PFto_ADD("Gray26", "\033[38;5;238m");
    PFto_ADD("Gray27", "\033[38;5;238m");
    PFto_ADD("Gray28", "\033[38;5;238m");
    PFto_ADD("Gray29", "\033[38;5;239m");
    PFto_ADD("Gray3", "\033[38;5;232m");
    PFto_ADD("Gray30", "\033[38;5;239m");
    PFto_ADD("Gray31", "\033[38;5;239m");
    PFto_ADD("Gray32", "\033[38;5;8m");
    PFto_ADD("Gray33", "\033[38;5;8m");
    PFto_ADD("Gray34", "\033[38;5;240m");
    PFto_ADD("Gray35", "\033[38;5;240m");
    PFto_ADD("Gray36", "\033[38;5;240m");
    PFto_ADD("Gray37", "\033[38;5;241m");
    PFto_ADD("Gray38", "\033[38;5;241m");
    PFto_ADD("Gray39", "\033[38;5;241m");
    PFto_ADD("Gray4", "\033[38;5;232m");
    PFto_ADD("Gray40", "\033[38;5;241m");
    PFto_ADD("Gray41", "\033[38;5;242m");
    PFto_ADD("Gray42", "\033[38;5;242m");
    PFto_ADD("Gray43", "\033[38;5;242m");
    PFto_ADD("Gray44", "\033[38;5;242m");
    PFto_ADD("Gray45", "\033[38;5;243m");
    PFto_ADD("Gray46", "\033[38;5;243m");
    PFto_ADD("Gray47", "\033[38;5;243m");
    PFto_ADD("Gray48", "\033[38;5;243m");
    PFto_ADD("Gray49", "\033[38;5;244m");
    PFto_ADD("Gray5", "\033[38;5;232m");
    PFto_ADD("Gray50", "\033[38;5;244m");
    PFto_ADD("Gray51", "\033[38;5;244m");
    PFto_ADD("Gray52", "\033[38;5;102m");
    PFto_ADD("Gray53", "\033[38;5;102m");
    PFto_ADD("Gray54", "\033[38;5;245m");
    PFto_ADD("Gray55", "\033[38;5;245m");
    PFto_ADD("Gray56", "\033[38;5;245m");
    PFto_ADD("Gray57", "\033[38;5;246m");
    PFto_ADD("Gray58", "\033[38;5;246m");
    PFto_ADD("Gray59", "\033[38;5;246m");
    PFto_ADD("Gray6", "\033[38;5;233m");
    PFto_ADD("Gray60", "\033[38;5;246m");
    PFto_ADD("Gray61", "\033[38;5;247m");
    PFto_ADD("Gray62", "\033[38;5;247m");
    PFto_ADD("Gray63", "\033[38;5;247m");
    PFto_ADD("Gray64", "\033[38;5;247m");
    PFto_ADD("Gray65", "\033[38;5;248m");
    PFto_ADD("Gray66", "\033[38;5;248m");
    PFto_ADD("Gray67", "\033[38;5;248m");
    PFto_ADD("Gray68", "\033[38;5;248m");
    PFto_ADD("Gray69", "\033[38;5;249m");
    PFto_ADD("Gray7", "\033[38;5;233m");
    PFto_ADD("Gray70", "\033[38;5;249m");
    PFto_ADD("Gray71", "\033[38;5;249m");
    PFto_ADD("Gray72", "\033[38;5;7m");
    PFto_ADD("Gray73", "\033[38;5;7m");
    PFto_ADD("Gray74", "\033[38;5;250m");
    PFto_ADD("Gray75", "\033[38;5;250m");
    PFto_ADD("Gray76", "\033[38;5;251m");
    PFto_ADD("Gray77", "\033[38;5;251m");
    PFto_ADD("Gray78", "\033[38;5;251m");
    PFto_ADD("Gray79", "\033[38;5;251m");
    PFto_ADD("Gray8", "\033[38;5;233m");
    PFto_ADD("Gray80", "\033[38;5;252m");
    PFto_ADD("Gray81", "\033[38;5;252m");
    PFto_ADD("Gray82", "\033[38;5;252m");
    PFto_ADD("Gray83", "\033[38;5;252m");
    PFto_ADD("Gray84", "\033[38;5;253m");
    PFto_ADD("Gray85", "\033[38;5;253m");
    PFto_ADD("Gray86", "\033[38;5;253m");
    PFto_ADD("Gray87", "\033[38;5;188m");
    PFto_ADD("Gray88", "\033[38;5;188m");
    PFto_ADD("Gray89", "\033[38;5;254m");
    PFto_ADD("Gray9", "\033[38;5;233m");
    PFto_ADD("Gray90", "\033[38;5;254m");
    PFto_ADD("Gray91", "\033[38;5;254m");
    PFto_ADD("Gray92", "\033[38;5;255m");
    PFto_ADD("Gray93", "\033[38;5;255m");
    PFto_ADD("Gray94", "\033[38;5;255m");
    PFto_ADD("Gray95", "\033[38;5;255m");
    PFto_ADD("Gray96", "\033[38;5;255m");
    PFto_ADD("Gray97", "\033[38;5;15m");
    PFto_ADD("Gray98", "\033[38;5;15m");
    PFto_ADD("Gray99", "\033[38;5;15m");
    PFto_ADD("Green", "\033[38;5;46m");
    PFto_ADD("Green1", "\033[38;5;46m");
    PFto_ADD("Green2", "\033[38;5;40m");
    PFto_ADD("Green3", "\033[38;5;40m");
    PFto_ADD("Green4", "\033[38;5;28m");
    PFto_ADD("GreenYellow", "\033[38;5;155m");
    PFto_ADD("Grey", "\033[38;5;250m");
    PFto_ADD("Grey0", "\033[38;5;0m");
    PFto_ADD("Grey1", "\033[38;5;0m");
    PFto_ADD("Grey10", "\033[38;5;234m");
    PFto_ADD("Grey100", "\033[38;5;15m");
    PFto_ADD("Grey11", "\033[38;5;234m");
    PFto_ADD("Grey12", "\033[38;5;234m");
    PFto_ADD("Grey13", "\033[38;5;234m");
    PFto_ADD("Grey14", "\033[38;5;235m");
    PFto_ADD("Grey15", "\033[38;5;235m");
    PFto_ADD("Grey16", "\033[38;5;235m");
    PFto_ADD("Grey17", "\033[38;5;235m");
    PFto_ADD("Grey18", "\033[38;5;236m");
    PFto_ADD("Grey19", "\033[38;5;236m");
    PFto_ADD("Grey2", "\033[38;5;232m");
    PFto_ADD("Grey20", "\033[38;5;236m");
    PFto_ADD("Grey21", "\033[38;5;237m");
    PFto_ADD("Grey22", "\033[38;5;237m");
    PFto_ADD("Grey23", "\033[38;5;237m");
    PFto_ADD("Grey24", "\033[38;5;237m");
    PFto_ADD("Grey25", "\033[38;5;238m");
    PFto_ADD("Grey26", "\033[38;5;238m");
    PFto_ADD("Grey27", "\033[38;5;238m");
    PFto_ADD("Grey28", "\033[38;5;238m");
    PFto_ADD("Grey29", "\033[38;5;239m");
    PFto_ADD("Grey3", "\033[38;5;232m");
    PFto_ADD("Grey30", "\033[38;5;239m");
    PFto_ADD("Grey31", "\033[38;5;239m");
    PFto_ADD("Grey32", "\033[38;5;8m");
    PFto_ADD("Grey33", "\033[38;5;8m");
    PFto_ADD("Grey34", "\033[38;5;240m");
    PFto_ADD("Grey35", "\033[38;5;240m");
    PFto_ADD("Grey36", "\033[38;5;240m");
    PFto_ADD("Grey37", "\033[38;5;241m");
    PFto_ADD("Grey38", "\033[38;5;241m");
    PFto_ADD("Grey39", "\033[38;5;241m");
    PFto_ADD("Grey4", "\033[38;5;232m");
    PFto_ADD("Grey40", "\033[38;5;241m");
    PFto_ADD("Grey41", "\033[38;5;242m");
    PFto_ADD("Grey42", "\033[38;5;242m");
    PFto_ADD("Grey43", "\033[38;5;242m");
    PFto_ADD("Grey44", "\033[38;5;242m");
    PFto_ADD("Grey45", "\033[38;5;243m");
    PFto_ADD("Grey46", "\033[38;5;243m");
    PFto_ADD("Grey47", "\033[38;5;243m");
    PFto_ADD("Grey48", "\033[38;5;243m");
    PFto_ADD("Grey49", "\033[38;5;244m");
    PFto_ADD("Grey5", "\033[38;5;232m");
    PFto_ADD("Grey50", "\033[38;5;244m");
    PFto_ADD("Grey51", "\033[38;5;244m");
    PFto_ADD("Grey52", "\033[38;5;102m");
    PFto_ADD("Grey53", "\033[38;5;102m");
    PFto_ADD("Grey54", "\033[38;5;245m");
    PFto_ADD("Grey55", "\033[38;5;245m");
    PFto_ADD("Grey56", "\033[38;5;245m");
    PFto_ADD("Grey57", "\033[38;5;246m");
    PFto_ADD("Grey58", "\033[38;5;246m");
    PFto_ADD("Grey59", "\033[38;5;246m");
    PFto_ADD("Grey6", "\033[38;5;233m");
    PFto_ADD("Grey60", "\033[38;5;246m");
    PFto_ADD("Grey61", "\033[38;5;247m");
    PFto_ADD("Grey62", "\033[38;5;247m");
    PFto_ADD("Grey63", "\033[38;5;247m");
    PFto_ADD("Grey64", "\033[38;5;247m");
    PFto_ADD("Grey65", "\033[38;5;248m");
    PFto_ADD("Grey66", "\033[38;5;248m");
    PFto_ADD("Grey67", "\033[38;5;248m");
    PFto_ADD("Grey68", "\033[38;5;248m");
    PFto_ADD("Grey69", "\033[38;5;249m");
    PFto_ADD("Grey7", "\033[38;5;233m");
    PFto_ADD("Grey70", "\033[38;5;249m");
    PFto_ADD("Grey71", "\033[38;5;249m");
    PFto_ADD("Grey72", "\033[38;5;7m");
    PFto_ADD("Grey73", "\033[38;5;7m");
    PFto_ADD("Grey74", "\033[38;5;250m");
    PFto_ADD("Grey75", "\033[38;5;250m");
    PFto_ADD("Grey76", "\033[38;5;251m");
    PFto_ADD("Grey77", "\033[38;5;251m");
    PFto_ADD("Grey78", "\033[38;5;251m");
    PFto_ADD("Grey79", "\033[38;5;251m");
    PFto_ADD("Grey8", "\033[38;5;233m");
    PFto_ADD("Grey80", "\033[38;5;252m");
    PFto_ADD("Grey81", "\033[38;5;252m");
    PFto_ADD("Grey82", "\033[38;5;252m");
    PFto_ADD("Grey83", "\033[38;5;252m");
    PFto_ADD("Grey84", "\033[38;5;253m");
    PFto_ADD("Grey85", "\033[38;5;253m");
    PFto_ADD("Grey86", "\033[38;5;253m");
    PFto_ADD("Grey87", "\033[38;5;188m");
    PFto_ADD("Grey88", "\033[38;5;188m");
    PFto_ADD("Grey89", "\033[38;5;254m");
    PFto_ADD("Grey9", "\033[38;5;233m");
    PFto_ADD("Grey90", "\033[38;5;254m");
    PFto_ADD("Grey91", "\033[38;5;254m");
    PFto_ADD("Grey92", "\033[38;5;255m");
    PFto_ADD("Grey93", "\033[38;5;255m");
    PFto_ADD("Grey94", "\033[38;5;255m");
    PFto_ADD("Grey95", "\033[38;5;255m");
    PFto_ADD("Grey96", "\033[38;5;255m");
    PFto_ADD("Grey97", "\033[38;5;15m");
    PFto_ADD("Grey98", "\033[38;5;15m");
    PFto_ADD("Grey99", "\033[38;5;15m");
    PFto_ADD("HOME", "\033[H");
    PFto_ADD("Honeydew", "\033[38;5;255m");
    PFto_ADD("Honeydew1", "\033[38;5;255m");
    PFto_ADD("Honeydew2", "\033[38;5;254m");
    PFto_ADD("Honeydew3", "\033[38;5;251m");
    PFto_ADD("Honeydew4", "\033[38;5;102m");
    PFto_ADD("HotPink", "\033[38;5;205m");
    PFto_ADD("HotPink1", "\033[38;5;205m");
    PFto_ADD("HotPink2", "\033[38;5;169m");
    PFto_ADD("HotPink3", "\033[38;5;168m");
    PFto_ADD("HotPink4", "\033[38;5;95m");
    PFto_ADD("INITTERM", "\033[H\033[2J");
    PFto_ADD("ITALIC", "\033[3m");
    PFto_ADD("IndianRed", "\033[38;5;167m");
    PFto_ADD("IndianRed1", "\033[38;5;9m");
    PFto_ADD("IndianRed2", "\033[38;5;9m");
    PFto_ADD("IndianRed3", "\033[38;5;167m");
    PFto_ADD("IndianRed4", "\033[38;5;95m");
    PFto_ADD("Ivory", "\033[38;5;15m");
    PFto_ADD("Ivory1", "\033[38;5;15m");
    PFto_ADD("Ivory2", "\033[38;5;255m");
    PFto_ADD("Ivory3", "\033[38;5;251m");
    PFto_ADD("Ivory4", "\033[38;5;102m");
    PFto_ADD("Khaki", "\033[38;5;222m");
    PFto_ADD("Khaki1", "\033[38;5;228m");
    PFto_ADD("Khaki2", "\033[38;5;186m");
    PFto_ADD("Khaki3", "\033[38;5;180m");
    PFto_ADD("Khaki4", "\033[38;5;101m");
    PFto_ADD("LIGHTBLUE", "\033[38;5;12m");
    PFto_ADD("LIGHTCYAN", "\033[38;5;14m");
    PFto_ADD("LIGHTGREEN", "\033[38;5;10m");
    PFto_ADD("LIGHTRED", "\033[38;5;9m");
    PFto_ADD("Lavender", "\033[38;5;189m");
    PFto_ADD("LavenderBlush", "\033[38;5;15m");
    PFto_ADD("LavenderBlush1", "\033[38;5;15m");
    PFto_ADD("LavenderBlush2", "\033[38;5;254m");
    PFto_ADD("LavenderBlush3", "\033[38;5;251m");
    PFto_ADD("LavenderBlush4", "\033[38;5;102m");
    PFto_ADD("LawnGreen", "\033[38;5;118m");
    PFto_ADD("LemonChiffon", "\033[38;5;230m");
    PFto_ADD("LemonChiffon1", "\033[38;5;230m");
    PFto_ADD("LemonChiffon2", "\033[38;5;187m");
    PFto_ADD("LemonChiffon3", "\033[38;5;181m");
    PFto_ADD("LemonChiffon4", "\033[38;5;244m");
    PFto_ADD("LightBlue", "\033[38;5;152m");
    PFto_ADD("LightBlue1", "\033[38;5;159m");
    PFto_ADD("LightBlue2", "\033[38;5;152m");
    PFto_ADD("LightBlue3", "\033[38;5;110m");
    PFto_ADD("LightBlue4", "\033[38;5;66m");
    PFto_ADD("LightCoral", "\033[38;5;210m");
    PFto_ADD("LightCyan", "\033[38;5;195m");
    PFto_ADD("LightCyan1", "\033[38;5;195m");
    PFto_ADD("LightCyan2", "\033[38;5;254m");
    PFto_ADD("LightCyan3", "\033[38;5;251m");
    PFto_ADD("LightCyan4", "\033[38;5;102m");
    PFto_ADD("LightGoldenrod", "\033[38;5;186m");
    PFto_ADD("LightGoldenrod1", "\033[38;5;222m");
    PFto_ADD("LightGoldenrod2", "\033[38;5;186m");
    PFto_ADD("LightGoldenrod3", "\033[38;5;180m");
    PFto_ADD("LightGoldenrod4", "\033[38;5;101m");
    PFto_ADD("LightGoldenrodYellow", "\033[38;5;230m");
    PFto_ADD("LightGray", "\033[38;5;252m");
    PFto_ADD("LightGreen", "\033[38;5;114m");
    PFto_ADD("LightGrey", "\033[38;5;252m");
    PFto_ADD("LightPink", "\033[38;5;217m");
    PFto_ADD("LightPink1", "\033[38;5;217m");
    PFto_ADD("LightPink2", "\033[38;5;181m");
    PFto_ADD("LightPink3", "\033[38;5;174m");
    PFto_ADD("LightPink4", "\033[38;5;95m");
    PFto_ADD("LightSalmon", "\033[38;5;210m");
    PFto_ADD("LightSalmon1", "\033[38;5;210m");
    PFto_ADD("LightSalmon2", "\033[38;5;174m");
    PFto_ADD("LightSalmon3", "\033[38;5;173m");
    PFto_ADD("LightSalmon4", "\033[38;5;95m");
    PFto_ADD("LightSeaGreen", "\033[38;5;6m");
    PFto_ADD("LightSkyBlue", "\033[38;5;117m");
    PFto_ADD("LightSkyBlue1", "\033[38;5;153m");
    PFto_ADD("LightSkyBlue2", "\033[38;5;152m");
    PFto_ADD("LightSkyBlue3", "\033[38;5;110m");
    PFto_ADD("LightSkyBlue4", "\033[38;5;66m");
    PFto_ADD("LightSlateBlue", "\033[38;5;105m");
    PFto_ADD("LightSlateGray", "\033[38;5;102m");
    PFto_ADD("LightSlateGrey", "\033[38;5;102m");
    PFto_ADD("LightSteelBlue", "\033[38;5;146m");
    PFto_ADD("LightSteelBlue1", "\033[38;5;153m");
    PFto_ADD("LightSteelBlue2", "\033[38;5;152m");
    PFto_ADD("LightSteelBlue3", "\033[38;5;146m");
    PFto_ADD("LightSteelBlue4", "\033[38;5;244m");
    PFto_ADD("LightYellow", "\033[38;5;230m");
    PFto_ADD("LightYellow1", "\033[38;5;230m");
    PFto_ADD("LightYellow2", "\033[38;5;254m");
    PFto_ADD("LightYellow3", "\033[38;5;251m");
    PFto_ADD("LightYellow4", "\033[38;5;102m");
    PFto_ADD("LimeGreen", "\033[38;5;77m");
    PFto_ADD("Linen", "\033[38;5;255m");
    PFto_ADD("MAGENTA", "\033[38;5;5m");
    PFto_ADD("Magenta", "\033[38;5;201m");
    PFto_ADD("Magenta1", "\033[38;5;201m");
    PFto_ADD("Magenta2", "\033[38;5;164m");
    PFto_ADD("Magenta3", "\033[38;5;164m");
    PFto_ADD("Magenta4", "\033[38;5;90m");
    PFto_ADD("Maroon", "\033[38;5;131m");
    PFto_ADD("Maroon1", "\033[38;5;205m");
    PFto_ADD("Maroon2", "\033[38;5;169m");
    PFto_ADD("Maroon3", "\033[38;5;162m");
    PFto_ADD("Maroon4", "\033[38;5;89m");
    PFto_ADD("MediumAquamarine", "\033[38;5;79m");
    PFto_ADD("MediumBlue", "\033[38;5;20m");
    PFto_ADD("MediumOrchid", "\033[38;5;134m");
    PFto_ADD("MediumOrchid1", "\033[38;5;171m");
    PFto_ADD("MediumOrchid2", "\033[38;5;170m");
    PFto_ADD("MediumOrchid3", "\033[38;5;134m");
    PFto_ADD("MediumOrchid4", "\033[38;5;96m");
    PFto_ADD("MediumPurple", "\033[38;5;104m");
    PFto_ADD("MediumPurple1", "\033[38;5;141m");
    PFto_ADD("MediumPurple2", "\033[38;5;104m");
    PFto_ADD("MediumPurple3", "\033[38;5;98m");
    PFto_ADD("MediumPurple4", "\033[38;5;60m");
    PFto_ADD("MediumSeaGreen", "\033[38;5;72m");
    PFto_ADD("MediumSlateBlue", "\033[38;5;98m");
    PFto_ADD("MediumSpringGreen", "\033[38;5;48m");
    PFto_ADD("MediumTurquoise", "\033[38;5;79m");
    PFto_ADD("MediumVioletRed", "\033[38;5;126m");
    PFto_ADD("MidnightBlue", "\033[38;5;18m");
    PFto_ADD("MintCream", "\033[38;5;15m");
    PFto_ADD("MistyRose", "\033[38;5;224m");
    PFto_ADD("MistyRose1", "\033[38;5;224m");
    PFto_ADD("MistyRose2", "\033[38;5;188m");
    PFto_ADD("MistyRose3", "\033[38;5;181m");
    PFto_ADD("MistyRose4", "\033[38;5;244m");
    PFto_ADD("Moccasin", "\033[38;5;223m");
    PFto_ADD("NavajoWhite", "\033[38;5;223m");
    PFto_ADD("NavajoWhite1", "\033[38;5;223m");
    PFto_ADD("NavajoWhite2", "\033[38;5;186m");
    PFto_ADD("NavajoWhite3", "\033[38;5;180m");
    PFto_ADD("NavajoWhite4", "\033[38;5;101m");
    PFto_ADD("Navy", "\033[38;5;18m");
    PFto_ADD("NavyBlue", "\033[38;5;18m");
    PFto_ADD("ORANGE", "\033[38;5;3m");
    PFto_ADD("OldLace", "\033[38;5;230m");
    PFto_ADD("OliveDrab", "\033[38;5;64m");
    PFto_ADD("OliveDrab1", "\033[38;5;155m");
    PFto_ADD("OliveDrab2", "\033[38;5;149m");
    PFto_ADD("OliveDrab3", "\033[38;5;113m");
    PFto_ADD("OliveDrab4", "\033[38;5;64m");
    PFto_ADD("Orange", "\033[38;5;214m");
    PFto_ADD("Orange1", "\033[38;5;214m");
    PFto_ADD("Orange2", "\033[38;5;172m");
    PFto_ADD("Orange3", "\033[38;5;172m");
    PFto_ADD("Orange4", "\033[38;5;94m");
    PFto_ADD("OrangeRed", "\033[38;5;202m");
    PFto_ADD("OrangeRed1", "\033[38;5;202m");
    PFto_ADD("OrangeRed2", "\033[38;5;166m");
    PFto_ADD("OrangeRed3", "\033[38;5;166m");
    PFto_ADD("OrangeRed4", "\033[38;5;88m");
    PFto_ADD("Orchid", "\033[38;5;176m");
    PFto_ADD("Orchid1", "\033[38;5;213m");
    PFto_ADD("Orchid2", "\033[38;5;176m");
    PFto_ADD("Orchid3", "\033[38;5;169m");
    PFto_ADD("Orchid4", "\033[38;5;96m");
    PFto_ADD("PINK", "\033[38;5;13m");
    PFto_ADD("PaleGoldenrod", "\033[38;5;187m");
    PFto_ADD("PaleGreen", "\033[38;5;120m");
    PFto_ADD("PaleGreen1", "\033[38;5;120m");
    PFto_ADD("PaleGreen2", "\033[38;5;114m");
    PFto_ADD("PaleGreen3", "\033[38;5;114m");
    PFto_ADD("PaleGreen4", "\033[38;5;65m");
    PFto_ADD("PaleTurquoise", "\033[38;5;152m");
    PFto_ADD("PaleTurquoise1", "\033[38;5;159m");
    PFto_ADD("PaleTurquoise2", "\033[38;5;152m");
    PFto_ADD("PaleTurquoise3", "\033[38;5;116m");
    PFto_ADD("PaleTurquoise4", "\033[38;5;66m");
    PFto_ADD("PaleVioletRed", "\033[38;5;174m");
    PFto_ADD("PaleVioletRed1", "\033[38;5;211m");
    PFto_ADD("PaleVioletRed2", "\033[38;5;174m");
    PFto_ADD("PaleVioletRed3", "\033[38;5;168m");
    PFto_ADD("PaleVioletRed4", "\033[38;5;95m");
    PFto_ADD("PapayaWhip", "\033[38;5;230m");
    PFto_ADD("PeachPuff", "\033[38;5;223m");
    PFto_ADD("PeachPuff1", "\033[38;5;223m");
    PFto_ADD("PeachPuff2", "\033[38;5;181m");
    PFto_ADD("PeachPuff3", "\033[38;5;180m");
    PFto_ADD("PeachPuff4", "\033[38;5;101m");
    PFto_ADD("Peru", "\033[38;5;173m");
    PFto_ADD("Pink", "\033[38;5;217m");
    PFto_ADD("Pink1", "\033[38;5;217m");
    PFto_ADD("Pink2", "\033[38;5;181m");
    PFto_ADD("Pink3", "\033[38;5;174m");
    PFto_ADD("Pink4", "\033[38;5;95m");
    PFto_ADD("Plum", "\033[38;5;176m");
    PFto_ADD("Plum1", "\033[38;5;219m");
    PFto_ADD("Plum2", "\033[38;5;182m");
    PFto_ADD("Plum3", "\033[38;5;176m");
    PFto_ADD("Plum4", "\033[38;5;96m");
    PFto_ADD("PowderBlue", "\033[38;5;152m");
    PFto_ADD("Purple", "\033[38;5;93m");
    PFto_ADD("Purple1", "\033[38;5;99m");
    PFto_ADD("Purple2", "\033[38;5;98m");
    PFto_ADD("Purple3", "\033[38;5;92m");
    PFto_ADD("Purple4", "\033[38;5;54m");
    PFto_ADD("RED", "\033[38;5;1m");
    PFto_ADD("RESET", "\033[0m");
    PFto_ADD("RESTORE", "\033[u");
    PFto_ADD("REVERSE", "\033[7m");
    PFto_ADD("Red", "\033[38;5;196m");
    PFto_ADD("Red1", "\033[38;5;196m");
    PFto_ADD("Red2", "\033[38;5;160m");
    PFto_ADD("Red3", "\033[38;5;160m");
    PFto_ADD("Red4", "\033[38;5;88m");
    PFto_ADD("RosyBrown", "\033[38;5;138m");
    PFto_ADD("RosyBrown1", "\033[38;5;217m");
    PFto_ADD("RosyBrown2", "\033[38;5;181m");
    PFto_ADD("RosyBrown3", "\033[38;5;174m");
    PFto_ADD("RosyBrown4", "\033[38;5;243m");
    PFto_ADD("RoyalBlue", "\033[38;5;62m");
    PFto_ADD("RoyalBlue1", "\033[38;5;69m");
    PFto_ADD("RoyalBlue2", "\033[38;5;12m");
    PFto_ADD("RoyalBlue3", "\033[38;5;62m");
    PFto_ADD("RoyalBlue4", "\033[38;5;24m");
    PFto_ADD("SAVE", "\033[s");
    PFto_ADD("STRIKETHRU", "\033[9m");
    PFto_ADD("SaddleBrown", "\033[38;5;94m");
    PFto_ADD("Salmon", "\033[38;5;210m");
    PFto_ADD("Salmon1", "\033[38;5;209m");
    PFto_ADD("Salmon2", "\033[38;5;173m");
    PFto_ADD("Salmon3", "\033[38;5;173m");
    PFto_ADD("Salmon4", "\033[38;5;95m");
    PFto_ADD("SandyBrown", "\033[38;5;215m");
    PFto_ADD("SeaGreen", "\033[38;5;65m");
    PFto_ADD("SeaGreen1", "\033[38;5;84m");
    PFto_ADD("SeaGreen2", "\033[38;5;78m");
    PFto_ADD("SeaGreen3", "\033[38;5;78m");
    PFto_ADD("SeaGreen4", "\033[38;5;65m");
    PFto_ADD("Seashell", "\033[38;5;255m");
    PFto_ADD("Seashell1", "\033[38;5;255m");
    PFto_ADD("Seashell2", "\033[38;5;254m");
    PFto_ADD("Seashell3", "\033[38;5;251m");
    PFto_ADD("Seashell4", "\033[38;5;102m");
    PFto_ADD("Sienna", "\033[38;5;95m");
    PFto_ADD("Sienna1", "\033[38;5;209m");
    PFto_ADD("Sienna2", "\033[38;5;173m");
    PFto_ADD("Sienna3", "\033[38;5;167m");
    PFto_ADD("Sienna4", "\033[38;5;94m");
    PFto_ADD("SkyBlue", "\033[38;5;116m");
    PFto_ADD("SkyBlue1", "\033[38;5;117m");
    PFto_ADD("SkyBlue2", "\033[38;5;110m");
    PFto_ADD("SkyBlue3", "\033[38;5;74m");
    PFto_ADD("SkyBlue4", "\033[38;5;66m");
    PFto_ADD("SlateBlue", "\033[38;5;62m");
    PFto_ADD("SlateBlue1", "\033[38;5;105m");
    PFto_ADD("SlateBlue2", "\033[38;5;98m");
    PFto_ADD("SlateBlue3", "\033[38;5;62m");
    PFto_ADD("SlateBlue4", "\033[38;5;60m");
    PFto_ADD("SlateGray", "\033[38;5;244m");
    PFto_ADD("SlateGray1", "\033[38;5;153m");
    PFto_ADD("SlateGray2", "\033[38;5;152m");
    PFto_ADD("SlateGray3", "\033[38;5;110m");
    PFto_ADD("SlateGray4", "\033[38;5;244m");
    PFto_ADD("SlateGrey", "\033[38;5;244m");
    PFto_ADD("Snow", "\033[38;5;15m");
    PFto_ADD("Snow1", "\033[38;5;15m");
    PFto_ADD("Snow2", "\033[38;5;255m");
    PFto_ADD("Snow3", "\033[38;5;251m");
    PFto_ADD("Snow4", "\033[38;5;245m");
    PFto_ADD("SpringGreen", "\033[38;5;48m");
    PFto_ADD("SpringGreen1", "\033[38;5;48m");
    PFto_ADD("SpringGreen2", "\033[38;5;42m");
    PFto_ADD("SpringGreen3", "\033[38;5;41m");
    PFto_ADD("SpringGreen4", "\033[38;5;29m");
    PFto_ADD("SteelBlue", "\033[38;5;67m");
    PFto_ADD("SteelBlue1", "\033[38;5;75m");
    PFto_ADD("SteelBlue2", "\033[38;5;74m");
    PFto_ADD("SteelBlue3", "\033[38;5;68m");
    PFto_ADD("SteelBlue4", "\033[38;5;60m");
    PFto_ADD("Tan", "\033[38;5;180m");
    PFto_ADD("Tan1", "\033[38;5;215m");
    PFto_ADD("Tan2", "\033[38;5;173m");
    PFto_ADD("Tan3", "\033[38;5;173m");
    PFto_ADD("Tan4", "\033[38;5;95m");
    PFto_ADD("Thistle", "\033[38;5;182m");
    PFto_ADD("Thistle1", "\033[38;5;225m");
    PFto_ADD("Thistle2", "\033[38;5;254m");
    PFto_ADD("Thistle3", "\033[38;5;251m");
    PFto_ADD("Thistle4", "\033[38;5;102m");
    PFto_ADD("Tomato", "\033[38;5;9m");
    PFto_ADD("Tomato1", "\033[38;5;9m");
    PFto_ADD("Tomato2", "\033[38;5;9m");
    PFto_ADD("Tomato3", "\033[38;5;167m");
    PFto_ADD("Tomato4", "\033[38;5;94m");
    PFto_ADD("Turquoise", "\033[38;5;80m");
    PFto_ADD("Turquoise1", "\033[38;5;51m");
    PFto_ADD("Turquoise2", "\033[38;5;44m");
    PFto_ADD("Turquoise3", "\033[38;5;38m");
    PFto_ADD("Turquoise4", "\033[38;5;30m");
    PFto_ADD("UNDERLINE", "\033[4m");
    PFto_ADD("Violet", "\033[38;5;176m");
    PFto_ADD("VioletRed", "\033[38;5;162m");
    PFto_ADD("VioletRed1", "\033[38;5;204m");
    PFto_ADD("VioletRed2", "\033[38;5;168m");
    PFto_ADD("VioletRed3", "\033[38;5;168m");
    PFto_ADD("VioletRed4", "\033[38;5;89m");
    PFto_ADD("WHITE", "\033[38;5;15m");
    PFto_ADD("Wheat", "\033[38;5;223m");
    PFto_ADD("Wheat1", "\033[38;5;223m");
    PFto_ADD("Wheat2", "\033[38;5;187m");
    PFto_ADD("Wheat3", "\033[38;5;180m");
    PFto_ADD("Wheat4", "\033[38;5;101m");
    PFto_ADD("White", "\033[38;5;15m");
    PFto_ADD("WhiteSmoke", "\033[38;5;255m");
    PFto_ADD("YELLOW", "\033[38;5;11m");
    PFto_ADD("Yellow", "\033[38;5;226m");
    PFto_ADD("Yellow1", "\033[38;5;226m");
    PFto_ADD("Yellow2", "\033[38;5;184m");
    PFto_ADD("Yellow3", "\033[38;5;184m");
    PFto_ADD("Yellow4", "\033[38;5;100m");
    PFto_ADD("YellowGreen", "\033[38;5;113m");

    log_info("Pinkfish to Xterm256 conversion data loaded.");
}

void I3_loadPinkfishToGreyscale(void)
{
    log_info("Initializing Pinkfish to Greyscale conversion table...");
    stringmap_destroy(pinkfish_to_greyscale_db); // Free anything present
    pinkfish_to_greyscale_db = stringmap_init(); // Setup a new empty structure

    log_info("Loading Pinkfish to Greyscale conversion data...");

#undef PFto_ADD
#define PFto_ADD(key, value)                                                                                           \
    do                                                                                                                 \
    {                                                                                                                  \
        stringmap_add(pinkfish_to_greyscale_db, (key), (void *)(value), strlen((char *)(value)));                      \
        pinkfish_to_greyscale_count++;                                                                                 \
    } while (0)

    PFto_ADD("AliceBlue", "\033[38;5;255m");
    PFto_ADD("AntiqueWhite", "\033[38;5;255m");
    PFto_ADD("AntiqueWhite1", "\033[38;5;255m");
    PFto_ADD("AntiqueWhite2", "\033[38;5;253m");
    PFto_ADD("AntiqueWhite3", "\033[38;5;250m");
    PFto_ADD("AntiqueWhite4", "\033[38;5;244m");
    PFto_ADD("Aquamarine", "\033[38;5;251m");
    PFto_ADD("Aquamarine1", "\033[38;5;251m");
    PFto_ADD("Aquamarine2", "\033[38;5;250m");
    PFto_ADD("Aquamarine3", "\033[38;5;247m");
    PFto_ADD("Aquamarine4", "\033[38;5;242m");
    PFto_ADD("Azure", "\033[38;5;255m");
    PFto_ADD("Azure1", "\033[38;5;255m");
    PFto_ADD("Azure2", "\033[38;5;255m");
    PFto_ADD("Azure3", "\033[38;5;251m");
    PFto_ADD("Azure4", "\033[38;5;245m");
    PFto_ADD("B000", "\033[48;5;232m");
    PFto_ADD("B001", "\033[48;5;234m");
    PFto_ADD("B002", "\033[48;5;236m");
    PFto_ADD("B003", "\033[48;5;237m");
    PFto_ADD("B004", "\033[48;5;239m");
    PFto_ADD("B005", "\033[48;5;240m");
    PFto_ADD("B010", "\033[48;5;234m");
    PFto_ADD("B011", "\033[48;5;237m");
    PFto_ADD("B012", "\033[48;5;239m");
    PFto_ADD("B013", "\033[48;5;240m");
    PFto_ADD("B014", "\033[48;5;241m");
    PFto_ADD("B015", "\033[48;5;243m");
    PFto_ADD("B020", "\033[48;5;236m");
    PFto_ADD("B021", "\033[48;5;239m");
    PFto_ADD("B022", "\033[48;5;240m");
    PFto_ADD("B023", "\033[48;5;242m");
    PFto_ADD("B024", "\033[48;5;243m");
    PFto_ADD("B025", "\033[48;5;244m");
    PFto_ADD("B030", "\033[48;5;237m");
    PFto_ADD("B031", "\033[48;5;240m");
    PFto_ADD("B032", "\033[48;5;242m");
    PFto_ADD("B033", "\033[48;5;244m");
    PFto_ADD("B034", "\033[48;5;245m");
    PFto_ADD("B035", "\033[48;5;246m");
    PFto_ADD("B040", "\033[48;5;239m");
    PFto_ADD("B041", "\033[48;5;241m");
    PFto_ADD("B042", "\033[48;5;243m");
    PFto_ADD("B043", "\033[48;5;245m");
    PFto_ADD("B044", "\033[48;5;246m");
    PFto_ADD("B045", "\033[48;5;247m");
    PFto_ADD("B050", "\033[48;5;240m");
    PFto_ADD("B051", "\033[48;5;243m");
    PFto_ADD("B052", "\033[48;5;244m");
    PFto_ADD("B053", "\033[48;5;246m");
    PFto_ADD("B054", "\033[48;5;247m");
    PFto_ADD("B055", "\033[48;5;248m");
    PFto_ADD("B100", "\033[48;5;234m");
    PFto_ADD("B101", "\033[48;5;237m");
    PFto_ADD("B102", "\033[48;5;239m");
    PFto_ADD("B103", "\033[48;5;240m");
    PFto_ADD("B104", "\033[48;5;241m");
    PFto_ADD("B105", "\033[48;5;243m");
    PFto_ADD("B110", "\033[48;5;237m");
    PFto_ADD("B111", "\033[48;5;240m");
    PFto_ADD("B112", "\033[48;5;241m");
    PFto_ADD("B113", "\033[48;5;243m");
    PFto_ADD("B114", "\033[48;5;244m");
    PFto_ADD("B115", "\033[48;5;245m");
    PFto_ADD("B120", "\033[48;5;239m");
    PFto_ADD("B121", "\033[48;5;241m");
    PFto_ADD("B122", "\033[48;5;243m");
    PFto_ADD("B123", "\033[48;5;245m");
    PFto_ADD("B124", "\033[48;5;246m");
    PFto_ADD("B125", "\033[48;5;247m");
    PFto_ADD("B130", "\033[48;5;240m");
    PFto_ADD("B131", "\033[48;5;243m");
    PFto_ADD("B132", "\033[48;5;245m");
    PFto_ADD("B133", "\033[48;5;246m");
    PFto_ADD("B134", "\033[48;5;248m");
    PFto_ADD("B135", "\033[48;5;249m");
    PFto_ADD("B140", "\033[48;5;241m");
    PFto_ADD("B141", "\033[48;5;244m");
    PFto_ADD("B142", "\033[48;5;246m");
    PFto_ADD("B143", "\033[48;5;248m");
    PFto_ADD("B144", "\033[48;5;249m");
    PFto_ADD("B145", "\033[48;5;250m");
    PFto_ADD("B150", "\033[48;5;243m");
    PFto_ADD("B151", "\033[48;5;245m");
    PFto_ADD("B152", "\033[48;5;247m");
    PFto_ADD("B153", "\033[48;5;249m");
    PFto_ADD("B154", "\033[48;5;250m");
    PFto_ADD("B155", "\033[48;5;251m");
    PFto_ADD("B200", "\033[48;5;236m");
    PFto_ADD("B201", "\033[48;5;239m");
    PFto_ADD("B202", "\033[48;5;240m");
    PFto_ADD("B203", "\033[48;5;242m");
    PFto_ADD("B204", "\033[48;5;243m");
    PFto_ADD("B205", "\033[48;5;244m");
    PFto_ADD("B210", "\033[48;5;239m");
    PFto_ADD("B211", "\033[48;5;241m");
    PFto_ADD("B212", "\033[48;5;243m");
    PFto_ADD("B213", "\033[48;5;245m");
    PFto_ADD("B214", "\033[48;5;246m");
    PFto_ADD("B215", "\033[48;5;247m");
    PFto_ADD("B220", "\033[48;5;240m");
    PFto_ADD("B221", "\033[48;5;243m");
    PFto_ADD("B222", "\033[48;5;245m");
    PFto_ADD("B223", "\033[48;5;246m");
    PFto_ADD("B224", "\033[48;5;248m");
    PFto_ADD("B225", "\033[48;5;249m");
    PFto_ADD("B230", "\033[48;5;242m");
    PFto_ADD("B231", "\033[48;5;245m");
    PFto_ADD("B232", "\033[48;5;246m");
    PFto_ADD("B233", "\033[48;5;248m");
    PFto_ADD("B234", "\033[48;5;249m");
    PFto_ADD("B235", "\033[48;5;250m");
    PFto_ADD("B240", "\033[48;5;243m");
    PFto_ADD("B241", "\033[48;5;246m");
    PFto_ADD("B242", "\033[48;5;248m");
    PFto_ADD("B243", "\033[48;5;249m");
    PFto_ADD("B244", "\033[48;5;250m");
    PFto_ADD("B245", "\033[48;5;252m");
    PFto_ADD("B250", "\033[48;5;244m");
    PFto_ADD("B251", "\033[48;5;247m");
    PFto_ADD("B252", "\033[48;5;249m");
    PFto_ADD("B253", "\033[48;5;250m");
    PFto_ADD("B254", "\033[48;5;252m");
    PFto_ADD("B255", "\033[48;5;253m");
    PFto_ADD("B300", "\033[48;5;237m");
    PFto_ADD("B301", "\033[48;5;240m");
    PFto_ADD("B302", "\033[48;5;242m");
    PFto_ADD("B303", "\033[48;5;244m");
    PFto_ADD("B304", "\033[48;5;245m");
    PFto_ADD("B305", "\033[48;5;246m");
    PFto_ADD("B310", "\033[48;5;240m");
    PFto_ADD("B311", "\033[48;5;243m");
    PFto_ADD("B312", "\033[48;5;245m");
    PFto_ADD("B313", "\033[48;5;246m");
    PFto_ADD("B314", "\033[48;5;248m");
    PFto_ADD("B315", "\033[48;5;249m");
    PFto_ADD("B320", "\033[48;5;242m");
    PFto_ADD("B321", "\033[48;5;245m");
    PFto_ADD("B322", "\033[48;5;246m");
    PFto_ADD("B323", "\033[48;5;248m");
    PFto_ADD("B324", "\033[48;5;249m");
    PFto_ADD("B325", "\033[48;5;250m");
    PFto_ADD("B330", "\033[48;5;244m");
    PFto_ADD("B331", "\033[48;5;246m");
    PFto_ADD("B332", "\033[48;5;248m");
    PFto_ADD("B333", "\033[48;5;250m");
    PFto_ADD("B334", "\033[48;5;251m");
    PFto_ADD("B335", "\033[48;5;252m");
    PFto_ADD("B340", "\033[48;5;245m");
    PFto_ADD("B341", "\033[48;5;248m");
    PFto_ADD("B342", "\033[48;5;249m");
    PFto_ADD("B343", "\033[48;5;251m");
    PFto_ADD("B344", "\033[48;5;252m");
    PFto_ADD("B345", "\033[48;5;253m");
    PFto_ADD("B350", "\033[48;5;246m");
    PFto_ADD("B351", "\033[48;5;249m");
    PFto_ADD("B352", "\033[48;5;250m");
    PFto_ADD("B353", "\033[48;5;252m");
    PFto_ADD("B354", "\033[48;5;253m");
    PFto_ADD("B355", "\033[48;5;254m");
    PFto_ADD("B400", "\033[48;5;239m");
    PFto_ADD("B401", "\033[48;5;241m");
    PFto_ADD("B402", "\033[48;5;243m");
    PFto_ADD("B403", "\033[48;5;245m");
    PFto_ADD("B404", "\033[48;5;246m");
    PFto_ADD("B405", "\033[48;5;247m");
    PFto_ADD("B410", "\033[48;5;241m");
    PFto_ADD("B411", "\033[48;5;244m");
    PFto_ADD("B412", "\033[48;5;246m");
    PFto_ADD("B413", "\033[48;5;248m");
    PFto_ADD("B414", "\033[48;5;249m");
    PFto_ADD("B415", "\033[48;5;250m");
    PFto_ADD("B420", "\033[48;5;243m");
    PFto_ADD("B421", "\033[48;5;246m");
    PFto_ADD("B422", "\033[48;5;248m");
    PFto_ADD("B423", "\033[48;5;249m");
    PFto_ADD("B424", "\033[48;5;250m");
    PFto_ADD("B425", "\033[48;5;252m");
    PFto_ADD("B430", "\033[48;5;245m");
    PFto_ADD("B431", "\033[48;5;248m");
    PFto_ADD("B432", "\033[48;5;249m");
    PFto_ADD("B433", "\033[48;5;251m");
    PFto_ADD("B434", "\033[48;5;252m");
    PFto_ADD("B435", "\033[48;5;253m");
    PFto_ADD("B440", "\033[48;5;246m");
    PFto_ADD("B441", "\033[48;5;249m");
    PFto_ADD("B442", "\033[48;5;250m");
    PFto_ADD("B443", "\033[48;5;252m");
    PFto_ADD("B444", "\033[48;5;253m");
    PFto_ADD("B445", "\033[48;5;254m");
    PFto_ADD("B450", "\033[48;5;247m");
    PFto_ADD("B451", "\033[48;5;250m");
    PFto_ADD("B452", "\033[48;5;252m");
    PFto_ADD("B453", "\033[48;5;253m");
    PFto_ADD("B454", "\033[48;5;254m");
    PFto_ADD("B455", "\033[48;5;255m");
    PFto_ADD("B500", "\033[48;5;240m");
    PFto_ADD("B501", "\033[48;5;243m");
    PFto_ADD("B502", "\033[48;5;244m");
    PFto_ADD("B503", "\033[48;5;246m");
    PFto_ADD("B504", "\033[48;5;247m");
    PFto_ADD("B505", "\033[48;5;248m");
    PFto_ADD("B510", "\033[48;5;243m");
    PFto_ADD("B511", "\033[48;5;245m");
    PFto_ADD("B512", "\033[48;5;247m");
    PFto_ADD("B513", "\033[48;5;249m");
    PFto_ADD("B514", "\033[48;5;250m");
    PFto_ADD("B515", "\033[48;5;251m");
    PFto_ADD("B520", "\033[48;5;244m");
    PFto_ADD("B521", "\033[48;5;247m");
    PFto_ADD("B522", "\033[48;5;249m");
    PFto_ADD("B523", "\033[48;5;250m");
    PFto_ADD("B524", "\033[48;5;252m");
    PFto_ADD("B525", "\033[48;5;253m");
    PFto_ADD("B530", "\033[48;5;246m");
    PFto_ADD("B531", "\033[48;5;249m");
    PFto_ADD("B532", "\033[48;5;250m");
    PFto_ADD("B533", "\033[48;5;252m");
    PFto_ADD("B534", "\033[48;5;253m");
    PFto_ADD("B535", "\033[48;5;254m");
    PFto_ADD("B540", "\033[48;5;247m");
    PFto_ADD("B541", "\033[48;5;250m");
    PFto_ADD("B542", "\033[48;5;252m");
    PFto_ADD("B543", "\033[48;5;253m");
    PFto_ADD("B544", "\033[48;5;254m");
    PFto_ADD("B545", "\033[48;5;255m");
    PFto_ADD("B550", "\033[48;5;248m");
    PFto_ADD("B551", "\033[48;5;251m");
    PFto_ADD("B552", "\033[48;5;253m");
    PFto_ADD("B553", "\033[48;5;254m");
    PFto_ADD("B554", "\033[48;5;255m");
    PFto_ADD("B555", "\033[48;5;255m");
    PFto_ADD("BG00", "\033[48;5;232m");
    PFto_ADD("BG01", "\033[48;5;232m");
    PFto_ADD("BG02", "\033[48;5;233m");
    PFto_ADD("BG03", "\033[48;5;234m");
    PFto_ADD("BG04", "\033[48;5;235m");
    PFto_ADD("BG05", "\033[48;5;236m");
    PFto_ADD("BG06", "\033[48;5;237m");
    PFto_ADD("BG07", "\033[48;5;238m");
    PFto_ADD("BG08", "\033[48;5;239m");
    PFto_ADD("BG09", "\033[48;5;240m");
    PFto_ADD("BG10", "\033[48;5;241m");
    PFto_ADD("BG11", "\033[48;5;242m");
    PFto_ADD("BG12", "\033[48;5;243m");
    PFto_ADD("BG13", "\033[48;5;244m");
    PFto_ADD("BG14", "\033[48;5;245m");
    PFto_ADD("BG15", "\033[48;5;246m");
    PFto_ADD("BG16", "\033[48;5;247m");
    PFto_ADD("BG17", "\033[48;5;248m");
    PFto_ADD("BG18", "\033[48;5;249m");
    PFto_ADD("BG19", "\033[48;5;250m");
    PFto_ADD("BG20", "\033[48;5;251m");
    PFto_ADD("BG21", "\033[48;5;252m");
    PFto_ADD("BG22", "\033[48;5;253m");
    PFto_ADD("BG23", "\033[48;5;254m");
    PFto_ADD("BG24", "\033[48;5;255m");
    PFto_ADD("BG25", "\033[48;5;255m");
    PFto_ADD("BLACK", "\033[38;5;232m");
    PFto_ADD("BLUE", "\033[38;5;237m");
    PFto_ADD("BOLD", "\033[1m");
    PFto_ADD("B_AliceBlue", "\033[48;5;255m");
    PFto_ADD("B_AntiqueWhite", "\033[48;5;255m");
    PFto_ADD("B_AntiqueWhite1", "\033[48;5;255m");
    PFto_ADD("B_AntiqueWhite2", "\033[48;5;253m");
    PFto_ADD("B_AntiqueWhite3", "\033[48;5;250m");
    PFto_ADD("B_AntiqueWhite4", "\033[48;5;244m");
    PFto_ADD("B_Aquamarine", "\033[48;5;251m");
    PFto_ADD("B_Aquamarine1", "\033[48;5;251m");
    PFto_ADD("B_Aquamarine2", "\033[48;5;250m");
    PFto_ADD("B_Aquamarine3", "\033[48;5;247m");
    PFto_ADD("B_Aquamarine4", "\033[48;5;242m");
    PFto_ADD("B_Azure", "\033[48;5;255m");
    PFto_ADD("B_Azure1", "\033[48;5;255m");
    PFto_ADD("B_Azure2", "\033[48;5;255m");
    PFto_ADD("B_Azure3", "\033[48;5;251m");
    PFto_ADD("B_Azure4", "\033[48;5;245m");
    PFto_ADD("B_BLACK", "\033[48;5;232m");
    PFto_ADD("B_BLUE", "\033[48;5;237m");
    PFto_ADD("B_Beige", "\033[48;5;255m");
    PFto_ADD("B_Bisque", "\033[48;5;254m");
    PFto_ADD("B_Bisque1", "\033[48;5;254m");
    PFto_ADD("B_Bisque2", "\033[48;5;252m");
    PFto_ADD("B_Bisque3", "\033[48;5;249m");
    PFto_ADD("B_Bisque4", "\033[48;5;244m");
    PFto_ADD("B_Black", "\033[48;5;232m");
    PFto_ADD("B_BlanchedAlmond", "\033[48;5;254m");
    PFto_ADD("B_Blue", "\033[48;5;240m");
    PFto_ADD("B_Blue1", "\033[48;5;240m");
    PFto_ADD("B_Blue2", "\033[48;5;239m");
    PFto_ADD("B_Blue3", "\033[48;5;238m");
    PFto_ADD("B_Blue4", "\033[48;5;236m");
    PFto_ADD("B_BlueViolet", "\033[48;5;245m");
    PFto_ADD("B_Brown", "\033[48;5;239m");
    PFto_ADD("B_Brown1", "\033[48;5;244m");
    PFto_ADD("B_Brown2", "\033[48;5;243m");
    PFto_ADD("B_Brown3", "\033[48;5;241m");
    PFto_ADD("B_Brown4", "\033[48;5;238m");
    PFto_ADD("B_Burlywood", "\033[48;5;249m");
    PFto_ADD("B_Burlywood1", "\033[48;5;252m");
    PFto_ADD("B_Burlywood2", "\033[48;5;251m");
    PFto_ADD("B_Burlywood3", "\033[48;5;248m");
    PFto_ADD("B_Burlywood4", "\033[48;5;242m");
    PFto_ADD("B_CYAN", "\033[48;5;244m");
    PFto_ADD("B_CadetBlue", "\033[48;5;245m");
    PFto_ADD("B_CadetBlue1", "\033[48;5;253m");
    PFto_ADD("B_CadetBlue2", "\033[48;5;251m");
    PFto_ADD("B_CadetBlue3", "\033[48;5;249m");
    PFto_ADD("B_CadetBlue4", "\033[48;5;243m");
    PFto_ADD("B_Chartreuse", "\033[48;5;244m");
    PFto_ADD("B_Chartreuse1", "\033[48;5;244m");
    PFto_ADD("B_Chartreuse2", "\033[48;5;243m");
    PFto_ADD("B_Chartreuse3", "\033[48;5;241m");
    PFto_ADD("B_Chartreuse4", "\033[48;5;238m");
    PFto_ADD("B_Chocolate", "\033[48;5;243m");
    PFto_ADD("B_Chocolate1", "\033[48;5;245m");
    PFto_ADD("B_Chocolate2", "\033[48;5;244m");
    PFto_ADD("B_Chocolate3", "\033[48;5;242m");
    PFto_ADD("B_Chocolate4", "\033[48;5;239m");
    PFto_ADD("B_Coral", "\033[48;5;247m");
    PFto_ADD("B_Coral1", "\033[48;5;246m");
    PFto_ADD("B_Coral2", "\033[48;5;245m");
    PFto_ADD("B_Coral3", "\033[48;5;243m");
    PFto_ADD("B_Coral4", "\033[48;5;239m");
    PFto_ADD("B_CornflowerBlue", "\033[48;5;247m");
    PFto_ADD("B_Cornsilk", "\033[48;5;255m");
    PFto_ADD("B_Cornsilk1", "\033[48;5;255m");
    PFto_ADD("B_Cornsilk2", "\033[48;5;254m");
    PFto_ADD("B_Cornsilk3", "\033[48;5;251m");
    PFto_ADD("B_Cornsilk4", "\033[48;5;244m");
    PFto_ADD("B_Cyan", "\033[48;5;248m");
    PFto_ADD("B_Cyan1", "\033[48;5;248m");
    PFto_ADD("B_Cyan2", "\033[48;5;247m");
    PFto_ADD("B_Cyan3", "\033[48;5;245m");
    PFto_ADD("B_Cyan4", "\033[48;5;240m");
    PFto_ADD("B_DARKGREY", "\033[48;5;240m");
    PFto_ADD("B_DarkBlue", "\033[48;5;236m");
    PFto_ADD("B_DarkCyan", "\033[48;5;240m");
    PFto_ADD("B_DarkGoldenrod", "\033[48;5;242m");
    PFto_ADD("B_DarkGoldenrod1", "\033[48;5;246m");
    PFto_ADD("B_DarkGoldenrod2", "\033[48;5;245m");
    PFto_ADD("B_DarkGoldenrod3", "\033[48;5;243m");
    PFto_ADD("B_DarkGoldenrod4", "\033[48;5;239m");
    PFto_ADD("B_DarkGray", "\033[48;5;248m");
    PFto_ADD("B_DarkGreen", "\033[48;5;235m");
    PFto_ADD("B_DarkGrey", "\033[48;5;248m");
    PFto_ADD("B_DarkKhaki", "\033[48;5;247m");
    PFto_ADD("B_DarkMagenta", "\033[48;5;240m");
    PFto_ADD("B_DarkOliveGreen", "\033[48;5;239m");
    PFto_ADD("B_DarkOliveGreen1", "\033[48;5;250m");
    PFto_ADD("B_DarkOliveGreen2", "\033[48;5;249m");
    PFto_ADD("B_DarkOliveGreen3", "\033[48;5;246m");
    PFto_ADD("B_DarkOliveGreen4", "\033[48;5;242m");
    PFto_ADD("B_DarkOrange", "\033[48;5;244m");
    PFto_ADD("B_DarkOrange1", "\033[48;5;244m");
    PFto_ADD("B_DarkOrange2", "\033[48;5;243m");
    PFto_ADD("B_DarkOrange3", "\033[48;5;241m");
    PFto_ADD("B_DarkOrange4", "\033[48;5;238m");
    PFto_ADD("B_DarkOrchid", "\033[48;5;245m");
    PFto_ADD("B_DarkOrchid1", "\033[48;5;248m");
    PFto_ADD("B_DarkOrchid2", "\033[48;5;247m");
    PFto_ADD("B_DarkOrchid3", "\033[48;5;245m");
    PFto_ADD("B_DarkOrchid4", "\033[48;5;240m");
    PFto_ADD("B_DarkRed", "\033[48;5;236m");
    PFto_ADD("B_DarkSalmon", "\033[48;5;248m");
    PFto_ADD("B_DarkSeaGreen", "\033[48;5;247m");
    PFto_ADD("B_DarkSeaGreen1", "\033[48;5;253m");
    PFto_ADD("B_DarkSeaGreen2", "\033[48;5;251m");
    PFto_ADD("B_DarkSeaGreen3", "\033[48;5;248m");
    PFto_ADD("B_DarkSeaGreen4", "\033[48;5;243m");
    PFto_ADD("B_DarkSlateBlue", "\033[48;5;240m");
    PFto_ADD("B_DarkSlateGray", "\033[48;5;238m");
    PFto_ADD("B_DarkSlateGray1", "\033[48;5;253m");
    PFto_ADD("B_DarkSlateGray2", "\033[48;5;252m");
    PFto_ADD("B_DarkSlateGray3", "\033[48;5;249m");
    PFto_ADD("B_DarkSlateGray4", "\033[48;5;243m");
    PFto_ADD("B_DarkSlateGrey", "\033[48;5;238m");
    PFto_ADD("B_DarkTurquoise", "\033[48;5;245m");
    PFto_ADD("B_DarkViolet", "\033[48;5;243m");
    PFto_ADD("B_DeepPink", "\033[48;5;245m");
    PFto_ADD("B_DeepPink1", "\033[48;5;245m");
    PFto_ADD("B_DeepPink2", "\033[48;5;244m");
    PFto_ADD("B_DeepPink3", "\033[48;5;242m");
    PFto_ADD("B_DeepPink4", "\033[48;5;239m");
    PFto_ADD("B_DeepSkyBlue", "\033[48;5;246m");
    PFto_ADD("B_DeepSkyBlue1", "\033[48;5;246m");
    PFto_ADD("B_DeepSkyBlue2", "\033[48;5;245m");
    PFto_ADD("B_DeepSkyBlue3", "\033[48;5;243m");
    PFto_ADD("B_DeepSkyBlue4", "\033[48;5;239m");
    PFto_ADD("B_DimGray", "\033[48;5;242m");
    PFto_ADD("B_DimGrey", "\033[48;5;242m");
    PFto_ADD("B_DodgerBlue", "\033[48;5;245m");
    PFto_ADD("B_DodgerBlue1", "\033[48;5;245m");
    PFto_ADD("B_DodgerBlue2", "\033[48;5;245m");
    PFto_ADD("B_DodgerBlue3", "\033[48;5;243m");
    PFto_ADD("B_DodgerBlue4", "\033[48;5;239m");
    PFto_ADD("B_Firebrick", "\033[48;5;239m");
    PFto_ADD("B_Firebrick1", "\033[48;5;243m");
    PFto_ADD("B_Firebrick2", "\033[48;5;242m");
    PFto_ADD("B_Firebrick3", "\033[48;5;241m");
    PFto_ADD("B_Firebrick4", "\033[48;5;238m");
    PFto_ADD("B_FloralWhite", "\033[48;5;255m");
    PFto_ADD("B_ForestGreen", "\033[48;5;238m");
    PFto_ADD("B_GREEN", "\033[48;5;237m");
    PFto_ADD("B_GREY", "\033[48;5;250m");
    PFto_ADD("B_Gainsboro", "\033[48;5;253m");
    PFto_ADD("B_GhostWhite", "\033[48;5;255m");
    PFto_ADD("B_Gold", "\033[48;5;247m");
    PFto_ADD("B_Gold1", "\033[48;5;247m");
    PFto_ADD("B_Gold2", "\033[48;5;246m");
    PFto_ADD("B_Gold3", "\033[48;5;244m");
    PFto_ADD("B_Gold4", "\033[48;5;240m");
    PFto_ADD("B_Goldenrod", "\033[48;5;245m");
    PFto_ADD("B_Goldenrod1", "\033[48;5;247m");
    PFto_ADD("B_Goldenrod2", "\033[48;5;246m");
    PFto_ADD("B_Goldenrod3", "\033[48;5;244m");
    PFto_ADD("B_Goldenrod4", "\033[48;5;240m");
    PFto_ADD("B_Gray", "\033[48;5;250m");
    PFto_ADD("B_Gray0", "\033[48;5;232m");
    PFto_ADD("B_Gray1", "\033[48;5;232m");
    PFto_ADD("B_Gray10", "\033[48;5;234m");
    PFto_ADD("B_Gray100", "\033[48;5;255m");
    PFto_ADD("B_Gray11", "\033[48;5;234m");
    PFto_ADD("B_Gray12", "\033[48;5;234m");
    PFto_ADD("B_Gray13", "\033[48;5;234m");
    PFto_ADD("B_Gray14", "\033[48;5;235m");
    PFto_ADD("B_Gray15", "\033[48;5;235m");
    PFto_ADD("B_Gray16", "\033[48;5;235m");
    PFto_ADD("B_Gray17", "\033[48;5;235m");
    PFto_ADD("B_Gray18", "\033[48;5;236m");
    PFto_ADD("B_Gray19", "\033[48;5;236m");
    PFto_ADD("B_Gray2", "\033[48;5;232m");
    PFto_ADD("B_Gray20", "\033[48;5;236m");
    PFto_ADD("B_Gray21", "\033[48;5;237m");
    PFto_ADD("B_Gray22", "\033[48;5;237m");
    PFto_ADD("B_Gray23", "\033[48;5;237m");
    PFto_ADD("B_Gray24", "\033[48;5;237m");
    PFto_ADD("B_Gray25", "\033[48;5;238m");
    PFto_ADD("B_Gray26", "\033[48;5;238m");
    PFto_ADD("B_Gray27", "\033[48;5;238m");
    PFto_ADD("B_Gray28", "\033[48;5;238m");
    PFto_ADD("B_Gray29", "\033[48;5;239m");
    PFto_ADD("B_Gray3", "\033[48;5;232m");
    PFto_ADD("B_Gray30", "\033[48;5;239m");
    PFto_ADD("B_Gray31", "\033[48;5;239m");
    PFto_ADD("B_Gray32", "\033[48;5;239m");
    PFto_ADD("B_Gray33", "\033[48;5;240m");
    PFto_ADD("B_Gray34", "\033[48;5;240m");
    PFto_ADD("B_Gray35", "\033[48;5;240m");
    PFto_ADD("B_Gray36", "\033[48;5;240m");
    PFto_ADD("B_Gray37", "\033[48;5;241m");
    PFto_ADD("B_Gray38", "\033[48;5;241m");
    PFto_ADD("B_Gray39", "\033[48;5;241m");
    PFto_ADD("B_Gray4", "\033[48;5;232m");
    PFto_ADD("B_Gray40", "\033[48;5;241m");
    PFto_ADD("B_Gray41", "\033[48;5;242m");
    PFto_ADD("B_Gray42", "\033[48;5;242m");
    PFto_ADD("B_Gray43", "\033[48;5;242m");
    PFto_ADD("B_Gray44", "\033[48;5;242m");
    PFto_ADD("B_Gray45", "\033[48;5;243m");
    PFto_ADD("B_Gray46", "\033[48;5;243m");
    PFto_ADD("B_Gray47", "\033[48;5;243m");
    PFto_ADD("B_Gray48", "\033[48;5;243m");
    PFto_ADD("B_Gray49", "\033[48;5;244m");
    PFto_ADD("B_Gray5", "\033[48;5;232m");
    PFto_ADD("B_Gray50", "\033[48;5;244m");
    PFto_ADD("B_Gray51", "\033[48;5;244m");
    PFto_ADD("B_Gray52", "\033[48;5;244m");
    PFto_ADD("B_Gray53", "\033[48;5;245m");
    PFto_ADD("B_Gray54", "\033[48;5;245m");
    PFto_ADD("B_Gray55", "\033[48;5;245m");
    PFto_ADD("B_Gray56", "\033[48;5;245m");
    PFto_ADD("B_Gray57", "\033[48;5;246m");
    PFto_ADD("B_Gray58", "\033[48;5;246m");
    PFto_ADD("B_Gray59", "\033[48;5;246m");
    PFto_ADD("B_Gray6", "\033[48;5;233m");
    PFto_ADD("B_Gray60", "\033[48;5;246m");
    PFto_ADD("B_Gray61", "\033[48;5;247m");
    PFto_ADD("B_Gray62", "\033[48;5;247m");
    PFto_ADD("B_Gray63", "\033[48;5;247m");
    PFto_ADD("B_Gray64", "\033[48;5;247m");
    PFto_ADD("B_Gray65", "\033[48;5;248m");
    PFto_ADD("B_Gray66", "\033[48;5;248m");
    PFto_ADD("B_Gray67", "\033[48;5;248m");
    PFto_ADD("B_Gray68", "\033[48;5;248m");
    PFto_ADD("B_Gray69", "\033[48;5;249m");
    PFto_ADD("B_Gray7", "\033[48;5;233m");
    PFto_ADD("B_Gray70", "\033[48;5;249m");
    PFto_ADD("B_Gray71", "\033[48;5;249m");
    PFto_ADD("B_Gray72", "\033[48;5;250m");
    PFto_ADD("B_Gray73", "\033[48;5;250m");
    PFto_ADD("B_Gray74", "\033[48;5;250m");
    PFto_ADD("B_Gray75", "\033[48;5;250m");
    PFto_ADD("B_Gray76", "\033[48;5;251m");
    PFto_ADD("B_Gray77", "\033[48;5;251m");
    PFto_ADD("B_Gray78", "\033[48;5;251m");
    PFto_ADD("B_Gray79", "\033[48;5;251m");
    PFto_ADD("B_Gray8", "\033[48;5;233m");
    PFto_ADD("B_Gray80", "\033[48;5;252m");
    PFto_ADD("B_Gray81", "\033[48;5;252m");
    PFto_ADD("B_Gray82", "\033[48;5;252m");
    PFto_ADD("B_Gray83", "\033[48;5;252m");
    PFto_ADD("B_Gray84", "\033[48;5;253m");
    PFto_ADD("B_Gray85", "\033[48;5;253m");
    PFto_ADD("B_Gray86", "\033[48;5;253m");
    PFto_ADD("B_Gray87", "\033[48;5;253m");
    PFto_ADD("B_Gray88", "\033[48;5;254m");
    PFto_ADD("B_Gray89", "\033[48;5;254m");
    PFto_ADD("B_Gray9", "\033[48;5;233m");
    PFto_ADD("B_Gray90", "\033[48;5;254m");
    PFto_ADD("B_Gray91", "\033[48;5;254m");
    PFto_ADD("B_Gray92", "\033[48;5;255m");
    PFto_ADD("B_Gray93", "\033[48;5;255m");
    PFto_ADD("B_Gray94", "\033[48;5;255m");
    PFto_ADD("B_Gray95", "\033[48;5;255m");
    PFto_ADD("B_Gray96", "\033[48;5;255m");
    PFto_ADD("B_Gray97", "\033[48;5;255m");
    PFto_ADD("B_Gray98", "\033[48;5;255m");
    PFto_ADD("B_Gray99", "\033[48;5;255m");
    PFto_ADD("B_Green", "\033[48;5;240m");
    PFto_ADD("B_Green1", "\033[48;5;240m");
    PFto_ADD("B_Green2", "\033[48;5;239m");
    PFto_ADD("B_Green3", "\033[48;5;238m");
    PFto_ADD("B_Green4", "\033[48;5;236m");
    PFto_ADD("B_GreenYellow", "\033[48;5;247m");
    PFto_ADD("B_Grey", "\033[48;5;250m");
    PFto_ADD("B_Grey0", "\033[48;5;232m");
    PFto_ADD("B_Grey1", "\033[48;5;232m");
    PFto_ADD("B_Grey10", "\033[48;5;234m");
    PFto_ADD("B_Grey100", "\033[48;5;255m");
    PFto_ADD("B_Grey11", "\033[48;5;234m");
    PFto_ADD("B_Grey12", "\033[48;5;234m");
    PFto_ADD("B_Grey13", "\033[48;5;234m");
    PFto_ADD("B_Grey14", "\033[48;5;235m");
    PFto_ADD("B_Grey15", "\033[48;5;235m");
    PFto_ADD("B_Grey16", "\033[48;5;235m");
    PFto_ADD("B_Grey17", "\033[48;5;235m");
    PFto_ADD("B_Grey18", "\033[48;5;236m");
    PFto_ADD("B_Grey19", "\033[48;5;236m");
    PFto_ADD("B_Grey2", "\033[48;5;232m");
    PFto_ADD("B_Grey20", "\033[48;5;236m");
    PFto_ADD("B_Grey21", "\033[48;5;237m");
    PFto_ADD("B_Grey22", "\033[48;5;237m");
    PFto_ADD("B_Grey23", "\033[48;5;237m");
    PFto_ADD("B_Grey24", "\033[48;5;237m");
    PFto_ADD("B_Grey25", "\033[48;5;238m");
    PFto_ADD("B_Grey26", "\033[48;5;238m");
    PFto_ADD("B_Grey27", "\033[48;5;238m");
    PFto_ADD("B_Grey28", "\033[48;5;238m");
    PFto_ADD("B_Grey29", "\033[48;5;239m");
    PFto_ADD("B_Grey3", "\033[48;5;232m");
    PFto_ADD("B_Grey30", "\033[48;5;239m");
    PFto_ADD("B_Grey31", "\033[48;5;239m");
    PFto_ADD("B_Grey32", "\033[48;5;239m");
    PFto_ADD("B_Grey33", "\033[48;5;240m");
    PFto_ADD("B_Grey34", "\033[48;5;240m");
    PFto_ADD("B_Grey35", "\033[48;5;240m");
    PFto_ADD("B_Grey36", "\033[48;5;240m");
    PFto_ADD("B_Grey37", "\033[48;5;241m");
    PFto_ADD("B_Grey38", "\033[48;5;241m");
    PFto_ADD("B_Grey39", "\033[48;5;241m");
    PFto_ADD("B_Grey4", "\033[48;5;232m");
    PFto_ADD("B_Grey40", "\033[48;5;241m");
    PFto_ADD("B_Grey41", "\033[48;5;242m");
    PFto_ADD("B_Grey42", "\033[48;5;242m");
    PFto_ADD("B_Grey43", "\033[48;5;242m");
    PFto_ADD("B_Grey44", "\033[48;5;242m");
    PFto_ADD("B_Grey45", "\033[48;5;243m");
    PFto_ADD("B_Grey46", "\033[48;5;243m");
    PFto_ADD("B_Grey47", "\033[48;5;243m");
    PFto_ADD("B_Grey48", "\033[48;5;243m");
    PFto_ADD("B_Grey49", "\033[48;5;244m");
    PFto_ADD("B_Grey5", "\033[48;5;232m");
    PFto_ADD("B_Grey50", "\033[48;5;244m");
    PFto_ADD("B_Grey51", "\033[48;5;244m");
    PFto_ADD("B_Grey52", "\033[48;5;244m");
    PFto_ADD("B_Grey53", "\033[48;5;245m");
    PFto_ADD("B_Grey54", "\033[48;5;245m");
    PFto_ADD("B_Grey55", "\033[48;5;245m");
    PFto_ADD("B_Grey56", "\033[48;5;245m");
    PFto_ADD("B_Grey57", "\033[48;5;246m");
    PFto_ADD("B_Grey58", "\033[48;5;246m");
    PFto_ADD("B_Grey59", "\033[48;5;246m");
    PFto_ADD("B_Grey6", "\033[48;5;233m");
    PFto_ADD("B_Grey60", "\033[48;5;246m");
    PFto_ADD("B_Grey61", "\033[48;5;247m");
    PFto_ADD("B_Grey62", "\033[48;5;247m");
    PFto_ADD("B_Grey63", "\033[48;5;247m");
    PFto_ADD("B_Grey64", "\033[48;5;247m");
    PFto_ADD("B_Grey65", "\033[48;5;248m");
    PFto_ADD("B_Grey66", "\033[48;5;248m");
    PFto_ADD("B_Grey67", "\033[48;5;248m");
    PFto_ADD("B_Grey68", "\033[48;5;248m");
    PFto_ADD("B_Grey69", "\033[48;5;249m");
    PFto_ADD("B_Grey7", "\033[48;5;233m");
    PFto_ADD("B_Grey70", "\033[48;5;249m");
    PFto_ADD("B_Grey71", "\033[48;5;249m");
    PFto_ADD("B_Grey72", "\033[48;5;250m");
    PFto_ADD("B_Grey73", "\033[48;5;250m");
    PFto_ADD("B_Grey74", "\033[48;5;250m");
    PFto_ADD("B_Grey75", "\033[48;5;250m");
    PFto_ADD("B_Grey76", "\033[48;5;251m");
    PFto_ADD("B_Grey77", "\033[48;5;251m");
    PFto_ADD("B_Grey78", "\033[48;5;251m");
    PFto_ADD("B_Grey79", "\033[48;5;251m");
    PFto_ADD("B_Grey8", "\033[48;5;233m");
    PFto_ADD("B_Grey80", "\033[48;5;252m");
    PFto_ADD("B_Grey81", "\033[48;5;252m");
    PFto_ADD("B_Grey82", "\033[48;5;252m");
    PFto_ADD("B_Grey83", "\033[48;5;252m");
    PFto_ADD("B_Grey84", "\033[48;5;253m");
    PFto_ADD("B_Grey85", "\033[48;5;253m");
    PFto_ADD("B_Grey86", "\033[48;5;253m");
    PFto_ADD("B_Grey87", "\033[48;5;253m");
    PFto_ADD("B_Grey88", "\033[48;5;254m");
    PFto_ADD("B_Grey89", "\033[48;5;254m");
    PFto_ADD("B_Grey9", "\033[48;5;233m");
    PFto_ADD("B_Grey90", "\033[48;5;254m");
    PFto_ADD("B_Grey91", "\033[48;5;254m");
    PFto_ADD("B_Grey92", "\033[48;5;255m");
    PFto_ADD("B_Grey93", "\033[48;5;255m");
    PFto_ADD("B_Grey94", "\033[48;5;255m");
    PFto_ADD("B_Grey95", "\033[48;5;255m");
    PFto_ADD("B_Grey96", "\033[48;5;255m");
    PFto_ADD("B_Grey97", "\033[48;5;255m");
    PFto_ADD("B_Grey98", "\033[48;5;255m");
    PFto_ADD("B_Grey99", "\033[48;5;255m");
    PFto_ADD("B_Honeydew", "\033[48;5;255m");
    PFto_ADD("B_Honeydew1", "\033[48;5;255m");
    PFto_ADD("B_Honeydew2", "\033[48;5;254m");
    PFto_ADD("B_Honeydew3", "\033[48;5;251m");
    PFto_ADD("B_Honeydew4", "\033[48;5;245m");
    PFto_ADD("B_HotPink", "\033[48;5;249m");
    PFto_ADD("B_HotPink1", "\033[48;5;249m");
    PFto_ADD("B_HotPink2", "\033[48;5;248m");
    PFto_ADD("B_HotPink3", "\033[48;5;246m");
    PFto_ADD("B_HotPink4", "\033[48;5;241m");
    PFto_ADD("B_IndianRed", "\033[48;5;244m");
    PFto_ADD("B_IndianRed1", "\033[48;5;247m");
    PFto_ADD("B_IndianRed2", "\033[48;5;246m");
    PFto_ADD("B_IndianRed3", "\033[48;5;244m");
    PFto_ADD("B_IndianRed4", "\033[48;5;240m");
    PFto_ADD("B_Ivory", "\033[48;5;255m");
    PFto_ADD("B_Ivory1", "\033[48;5;255m");
    PFto_ADD("B_Ivory2", "\033[48;5;255m");
    PFto_ADD("B_Ivory3", "\033[48;5;251m");
    PFto_ADD("B_Ivory4", "\033[48;5;245m");
    PFto_ADD("B_Khaki", "\033[48;5;252m");
    PFto_ADD("B_Khaki1", "\033[48;5;253m");
    PFto_ADD("B_Khaki2", "\033[48;5;251m");
    PFto_ADD("B_Khaki3", "\033[48;5;248m");
    PFto_ADD("B_Khaki4", "\033[48;5;243m");
    PFto_ADD("B_LIGHTBLUE", "\033[48;5;245m");
    PFto_ADD("B_LIGHTCYAN", "\033[48;5;251m");
    PFto_ADD("B_LIGHTGREEN", "\033[48;5;245m");
    PFto_ADD("B_LIGHTRED", "\033[48;5;245m");
    PFto_ADD("B_Lavender", "\033[48;5;255m");
    PFto_ADD("B_LavenderBlush", "\033[48;5;255m");
    PFto_ADD("B_LavenderBlush1", "\033[48;5;255m");
    PFto_ADD("B_LavenderBlush2", "\033[48;5;254m");
    PFto_ADD("B_LavenderBlush3", "\033[48;5;251m");
    PFto_ADD("B_LavenderBlush4", "\033[48;5;245m");
    PFto_ADD("B_LawnGreen", "\033[48;5;244m");
    PFto_ADD("B_LemonChiffon", "\033[48;5;255m");
    PFto_ADD("B_LemonChiffon1", "\033[48;5;255m");
    PFto_ADD("B_LemonChiffon2", "\033[48;5;253m");
    PFto_ADD("B_LemonChiffon3", "\033[48;5;250m");
    PFto_ADD("B_LemonChiffon4", "\033[48;5;244m");
    PFto_ADD("B_LightBlue", "\033[48;5;252m");
    PFto_ADD("B_LightBlue1", "\033[48;5;254m");
    PFto_ADD("B_LightBlue2", "\033[48;5;252m");
    PFto_ADD("B_LightBlue3", "\033[48;5;250m");
    PFto_ADD("B_LightBlue4", "\033[48;5;244m");
    PFto_ADD("B_LightCoral", "\033[48;5;248m");
    PFto_ADD("B_LightCyan", "\033[48;5;255m");
    PFto_ADD("B_LightCyan1", "\033[48;5;255m");
    PFto_ADD("B_LightCyan2", "\033[48;5;254m");
    PFto_ADD("B_LightCyan3", "\033[48;5;251m");
    PFto_ADD("B_LightCyan4", "\033[48;5;245m");
    PFto_ADD("B_LightGoldenrod", "\033[48;5;251m");
    PFto_ADD("B_LightGoldenrod1", "\033[48;5;252m");
    PFto_ADD("B_LightGoldenrod2", "\033[48;5;251m");
    PFto_ADD("B_LightGoldenrod3", "\033[48;5;248m");
    PFto_ADD("B_LightGoldenrod4", "\033[48;5;243m");
    PFto_ADD("B_LightGoldenrodYellow", "\033[48;5;255m");
    PFto_ADD("B_LightGray", "\033[48;5;252m");
    PFto_ADD("B_LightGreen", "\033[48;5;249m");
    PFto_ADD("B_LightGrey", "\033[48;5;252m");
    PFto_ADD("B_LightPink", "\033[48;5;252m");
    PFto_ADD("B_LightPink1", "\033[48;5;252m");
    PFto_ADD("B_LightPink2", "\033[48;5;250m");
    PFto_ADD("B_LightPink3", "\033[48;5;248m");
    PFto_ADD("B_LightPink4", "\033[48;5;242m");
    PFto_ADD("B_LightSalmon", "\033[48;5;249m");
    PFto_ADD("B_LightSalmon1", "\033[48;5;249m");
    PFto_ADD("B_LightSalmon2", "\033[48;5;248m");
    PFto_ADD("B_LightSalmon3", "\033[48;5;246m");
    PFto_ADD("B_LightSalmon4", "\033[48;5;241m");
    PFto_ADD("B_LightSeaGreen", "\033[48;5;244m");
    PFto_ADD("B_LightSkyBlue", "\033[48;5;251m");
    PFto_ADD("B_LightSkyBlue1", "\033[48;5;253m");
    PFto_ADD("B_LightSkyBlue2", "\033[48;5;252m");
    PFto_ADD("B_LightSkyBlue3", "\033[48;5;249m");
    PFto_ADD("B_LightSkyBlue4", "\033[48;5;243m");
    PFto_ADD("B_LightSlateBlue", "\033[48;5;248m");
    PFto_ADD("B_LightSlateGray", "\033[48;5;245m");
    PFto_ADD("B_LightSlateGrey", "\033[48;5;245m");
    PFto_ADD("B_LightSteelBlue", "\033[48;5;251m");
    PFto_ADD("B_LightSteelBlue1", "\033[48;5;254m");
    PFto_ADD("B_LightSteelBlue2", "\033[48;5;252m");
    PFto_ADD("B_LightSteelBlue3", "\033[48;5;249m");
    PFto_ADD("B_LightSteelBlue4", "\033[48;5;244m");
    PFto_ADD("B_LightYellow", "\033[48;5;255m");
    PFto_ADD("B_LightYellow1", "\033[48;5;255m");
    PFto_ADD("B_LightYellow2", "\033[48;5;254m");
    PFto_ADD("B_LightYellow3", "\033[48;5;251m");
    PFto_ADD("B_LightYellow4", "\033[48;5;245m");
    PFto_ADD("B_LimeGreen", "\033[48;5;241m");
    PFto_ADD("B_Linen", "\033[48;5;255m");
    PFto_ADD("B_MAGENTA", "\033[48;5;244m");
    PFto_ADD("B_Magenta", "\033[48;5;248m");
    PFto_ADD("B_Magenta1", "\033[48;5;248m");
    PFto_ADD("B_Magenta2", "\033[48;5;247m");
    PFto_ADD("B_Magenta3", "\033[48;5;245m");
    PFto_ADD("B_Magenta4", "\033[48;5;240m");
    PFto_ADD("B_Maroon", "\033[48;5;242m");
    PFto_ADD("B_Maroon1", "\033[48;5;247m");
    PFto_ADD("B_Maroon2", "\033[48;5;246m");
    PFto_ADD("B_Maroon3", "\033[48;5;244m");
    PFto_ADD("B_Maroon4", "\033[48;5;240m");
    PFto_ADD("B_MediumAquamarine", "\033[48;5;247m");
    PFto_ADD("B_MediumBlue", "\033[48;5;238m");
    PFto_ADD("B_MediumOrchid", "\033[48;5;247m");
    PFto_ADD("B_MediumOrchid1", "\033[48;5;251m");
    PFto_ADD("B_MediumOrchid2", "\033[48;5;249m");
    PFto_ADD("B_MediumOrchid3", "\033[48;5;247m");
    PFto_ADD("B_MediumOrchid4", "\033[48;5;242m");
    PFto_ADD("B_MediumPurple", "\033[48;5;247m");
    PFto_ADD("B_MediumPurple1", "\033[48;5;250m");
    PFto_ADD("B_MediumPurple2", "\033[48;5;248m");
    PFto_ADD("B_MediumPurple3", "\033[48;5;246m");
    PFto_ADD("B_MediumPurple4", "\033[48;5;241m");
    PFto_ADD("B_MediumSeaGreen", "\033[48;5;243m");
    PFto_ADD("B_MediumSlateBlue", "\033[48;5;247m");
    PFto_ADD("B_MediumSpringGreen", "\033[48;5;245m");
    PFto_ADD("B_MediumTurquoise", "\033[48;5;247m");
    PFto_ADD("B_MediumVioletRed", "\033[48;5;243m");
    PFto_ADD("B_MidnightBlue", "\033[48;5;237m");
    PFto_ADD("B_MintCream", "\033[48;5;255m");
    PFto_ADD("B_MistyRose", "\033[48;5;255m");
    PFto_ADD("B_MistyRose1", "\033[48;5;255m");
    PFto_ADD("B_MistyRose2", "\033[48;5;253m");
    PFto_ADD("B_MistyRose3", "\033[48;5;250m");
    PFto_ADD("B_MistyRose4", "\033[48;5;244m");
    PFto_ADD("B_Moccasin", "\033[48;5;253m");
    PFto_ADD("B_NavajoWhite", "\033[48;5;253m");
    PFto_ADD("B_NavajoWhite1", "\033[48;5;253m");
    PFto_ADD("B_NavajoWhite2", "\033[48;5;251m");
    PFto_ADD("B_NavajoWhite3", "\033[48;5;249m");
    PFto_ADD("B_NavajoWhite4", "\033[48;5;243m");
    PFto_ADD("B_Navy", "\033[48;5;235m");
    PFto_ADD("B_NavyBlue", "\033[48;5;235m");
    PFto_ADD("B_ORANGE", "\033[48;5;244m");
    PFto_ADD("B_OldLace", "\033[48;5;255m");
    PFto_ADD("B_OliveDrab", "\033[48;5;241m");
    PFto_ADD("B_OliveDrab1", "\033[48;5;248m");
    PFto_ADD("B_OliveDrab2", "\033[48;5;247m");
    PFto_ADD("B_OliveDrab3", "\033[48;5;245m");
    PFto_ADD("B_OliveDrab4", "\033[48;5;240m");
    PFto_ADD("B_Orange", "\033[48;5;245m");
    PFto_ADD("B_Orange1", "\033[48;5;245m");
    PFto_ADD("B_Orange2", "\033[48;5;244m");
    PFto_ADD("B_Orange3", "\033[48;5;242m");
    PFto_ADD("B_Orange4", "\033[48;5;239m");
    PFto_ADD("B_OrangeRed", "\033[48;5;242m");
    PFto_ADD("B_OrangeRed1", "\033[48;5;242m");
    PFto_ADD("B_OrangeRed2", "\033[48;5;241m");
    PFto_ADD("B_OrangeRed3", "\033[48;5;240m");
    PFto_ADD("B_OrangeRed4", "\033[48;5;237m");
    PFto_ADD("B_Orchid", "\033[48;5;249m");
    PFto_ADD("B_Orchid1", "\033[48;5;252m");
    PFto_ADD("B_Orchid2", "\033[48;5;251m");
    PFto_ADD("B_Orchid3", "\033[48;5;248m");
    PFto_ADD("B_Orchid4", "\033[48;5;243m");
    PFto_ADD("B_PINK", "\033[48;5;251m");
    PFto_ADD("B_PaleGoldenrod", "\033[48;5;253m");
    PFto_ADD("B_PaleGreen", "\033[48;5;250m");
    PFto_ADD("B_PaleGreen1", "\033[48;5;250m");
    PFto_ADD("B_PaleGreen2", "\033[48;5;249m");
    PFto_ADD("B_PaleGreen3", "\033[48;5;246m");
    PFto_ADD("B_PaleGreen4", "\033[48;5;241m");
    PFto_ADD("B_PaleTurquoise", "\033[48;5;253m");
    PFto_ADD("B_PaleTurquoise1", "\033[48;5;254m");
    PFto_ADD("B_PaleTurquoise2", "\033[48;5;253m");
    PFto_ADD("B_PaleTurquoise3", "\033[48;5;250m");
    PFto_ADD("B_PaleTurquoise4", "\033[48;5;244m");
    PFto_ADD("B_PaleVioletRed", "\033[48;5;247m");
    PFto_ADD("B_PaleVioletRed1", "\033[48;5;250m");
    PFto_ADD("B_PaleVioletRed2", "\033[48;5;248m");
    PFto_ADD("B_PaleVioletRed3", "\033[48;5;246m");
    PFto_ADD("B_PaleVioletRed4", "\033[48;5;241m");
    PFto_ADD("B_PapayaWhip", "\033[48;5;255m");
    PFto_ADD("B_PeachPuff", "\033[48;5;253m");
    PFto_ADD("B_PeachPuff1", "\033[48;5;253m");
    PFto_ADD("B_PeachPuff2", "\033[48;5;252m");
    PFto_ADD("B_PeachPuff3", "\033[48;5;249m");
    PFto_ADD("B_PeachPuff4", "\033[48;5;243m");
    PFto_ADD("B_Peru", "\033[48;5;245m");
    PFto_ADD("B_Pink", "\033[48;5;253m");
    PFto_ADD("B_Pink1", "\033[48;5;252m");
    PFto_ADD("B_Pink2", "\033[48;5;251m");
    PFto_ADD("B_Pink3", "\033[48;5;248m");
    PFto_ADD("B_Pink4", "\033[48;5;243m");
    PFto_ADD("B_Plum", "\033[48;5;251m");
    PFto_ADD("B_Plum1", "\033[48;5;254m");
    PFto_ADD("B_Plum2", "\033[48;5;253m");
    PFto_ADD("B_Plum3", "\033[48;5;250m");
    PFto_ADD("B_Plum4", "\033[48;5;244m");
    PFto_ADD("B_PowderBlue", "\033[48;5;252m");
    PFto_ADD("B_Purple", "\033[48;5;246m");
    PFto_ADD("B_Purple1", "\033[48;5;246m");
    PFto_ADD("B_Purple2", "\033[48;5;245m");
    PFto_ADD("B_Purple3", "\033[48;5;243m");
    PFto_ADD("B_Purple4", "\033[48;5;240m");
    PFto_ADD("B_RED", "\033[48;5;237m");
    PFto_ADD("B_Red", "\033[48;5;240m");
    PFto_ADD("B_Red1", "\033[48;5;240m");
    PFto_ADD("B_Red2", "\033[48;5;239m");
    PFto_ADD("B_Red3", "\033[48;5;238m");
    PFto_ADD("B_Red4", "\033[48;5;236m");
    PFto_ADD("B_RosyBrown", "\033[48;5;247m");
    PFto_ADD("B_RosyBrown1", "\033[48;5;253m");
    PFto_ADD("B_RosyBrown2", "\033[48;5;251m");
    PFto_ADD("B_RosyBrown3", "\033[48;5;248m");
    PFto_ADD("B_RosyBrown4", "\033[48;5;243m");
    PFto_ADD("B_RoyalBlue", "\033[48;5;244m");
    PFto_ADD("B_RoyalBlue1", "\033[48;5;246m");
    PFto_ADD("B_RoyalBlue2", "\033[48;5;245m");
    PFto_ADD("B_RoyalBlue3", "\033[48;5;243m");
    PFto_ADD("B_RoyalBlue4", "\033[48;5;239m");
    PFto_ADD("B_SaddleBrown", "\033[48;5;239m");
    PFto_ADD("B_Salmon", "\033[48;5;248m");
    PFto_ADD("B_Salmon1", "\033[48;5;248m");
    PFto_ADD("B_Salmon2", "\033[48;5;247m");
    PFto_ADD("B_Salmon3", "\033[48;5;245m");
    PFto_ADD("B_Salmon4", "\033[48;5;240m");
    PFto_ADD("B_SandyBrown", "\033[48;5;248m");
    PFto_ADD("B_SeaGreen", "\033[48;5;240m");
    PFto_ADD("B_SeaGreen1", "\033[48;5;248m");
    PFto_ADD("B_SeaGreen2", "\033[48;5;247m");
    PFto_ADD("B_SeaGreen3", "\033[48;5;245m");
    PFto_ADD("B_SeaGreen4", "\033[48;5;240m");
    PFto_ADD("B_Seashell", "\033[48;5;255m");
    PFto_ADD("B_Seashell1", "\033[48;5;255m");
    PFto_ADD("B_Seashell2", "\033[48;5;254m");
    PFto_ADD("B_Seashell3", "\033[48;5;251m");
    PFto_ADD("B_Seashell4", "\033[48;5;245m");
    PFto_ADD("B_Sienna", "\033[48;5;241m");
    PFto_ADD("B_Sienna1", "\033[48;5;246m");
    PFto_ADD("B_Sienna2", "\033[48;5;245m");
    PFto_ADD("B_Sienna3", "\033[48;5;243m");
    PFto_ADD("B_Sienna4", "\033[48;5;239m");
    PFto_ADD("B_SkyBlue", "\033[48;5;250m");
    PFto_ADD("B_SkyBlue1", "\033[48;5;251m");
    PFto_ADD("B_SkyBlue2", "\033[48;5;250m");
    PFto_ADD("B_SkyBlue3", "\033[48;5;247m");
    PFto_ADD("B_SkyBlue4", "\033[48;5;242m");
    PFto_ADD("B_SlateBlue", "\033[48;5;245m");
    PFto_ADD("B_SlateBlue1", "\033[48;5;248m");
    PFto_ADD("B_SlateBlue2", "\033[48;5;247m");
    PFto_ADD("B_SlateBlue3", "\033[48;5;244m");
    PFto_ADD("B_SlateBlue4", "\033[48;5;240m");
    PFto_ADD("B_SlateGray", "\033[48;5;244m");
    PFto_ADD("B_SlateGray1", "\033[48;5;254m");
    PFto_ADD("B_SlateGray2", "\033[48;5;252m");
    PFto_ADD("B_SlateGray3", "\033[48;5;249m");
    PFto_ADD("B_SlateGray4", "\033[48;5;244m");
    PFto_ADD("B_SlateGrey", "\033[48;5;244m");
    PFto_ADD("B_Snow", "\033[48;5;255m");
    PFto_ADD("B_Snow1", "\033[48;5;255m");
    PFto_ADD("B_Snow2", "\033[48;5;255m");
    PFto_ADD("B_Snow3", "\033[48;5;251m");
    PFto_ADD("B_Snow4", "\033[48;5;245m");
    PFto_ADD("B_SpringGreen", "\033[48;5;244m");
    PFto_ADD("B_SpringGreen1", "\033[48;5;244m");
    PFto_ADD("B_SpringGreen2", "\033[48;5;243m");
    PFto_ADD("B_SpringGreen3", "\033[48;5;241m");
    PFto_ADD("B_SpringGreen4", "\033[48;5;238m");
    PFto_ADD("B_SteelBlue", "\033[48;5;244m");
    PFto_ADD("B_SteelBlue1", "\033[48;5;249m");
    PFto_ADD("B_SteelBlue2", "\033[48;5;248m");
    PFto_ADD("B_SteelBlue3", "\033[48;5;246m");
    PFto_ADD("B_SteelBlue4", "\033[48;5;241m");
    PFto_ADD("B_Tan", "\033[48;5;249m");
    PFto_ADD("B_Tan1", "\033[48;5;248m");
    PFto_ADD("B_Tan2", "\033[48;5;247m");
    PFto_ADD("B_Tan3", "\033[48;5;245m");
    PFto_ADD("B_Tan4", "\033[48;5;240m");
    PFto_ADD("B_Thistle", "\033[48;5;252m");
    PFto_ADD("B_Thistle1", "\033[48;5;255m");
    PFto_ADD("B_Thistle2", "\033[48;5;254m");
    PFto_ADD("B_Thistle3", "\033[48;5;251m");
    PFto_ADD("B_Thistle4", "\033[48;5;245m");
    PFto_ADD("B_Tomato", "\033[48;5;245m");
    PFto_ADD("B_Tomato1", "\033[48;5;245m");
    PFto_ADD("B_Tomato2", "\033[48;5;244m");
    PFto_ADD("B_Tomato3", "\033[48;5;243m");
    PFto_ADD("B_Tomato4", "\033[48;5;239m");
    PFto_ADD("B_Turquoise", "\033[48;5;248m");
    PFto_ADD("B_Turquoise1", "\033[48;5;248m");
    PFto_ADD("B_Turquoise2", "\033[48;5;247m");
    PFto_ADD("B_Turquoise3", "\033[48;5;245m");
    PFto_ADD("B_Turquoise4", "\033[48;5;240m");
    PFto_ADD("B_Violet", "\033[48;5;251m");
    PFto_ADD("B_VioletRed", "\033[48;5;244m");
    PFto_ADD("B_VioletRed1", "\033[48;5;247m");
    PFto_ADD("B_VioletRed2", "\033[48;5;246m");
    PFto_ADD("B_VioletRed3", "\033[48;5;244m");
    PFto_ADD("B_VioletRed4", "\033[48;5;240m");
    PFto_ADD("B_WHITE", "\033[48;5;255m");
    PFto_ADD("B_Wheat", "\033[48;5;253m");
    PFto_ADD("B_Wheat1", "\033[48;5;254m");
    PFto_ADD("B_Wheat2", "\033[48;5;252m");
    PFto_ADD("B_Wheat3", "\033[48;5;249m");
    PFto_ADD("B_Wheat4", "\033[48;5;243m");
    PFto_ADD("B_White", "\033[48;5;255m");
    PFto_ADD("B_WhiteSmoke", "\033[48;5;255m");
    PFto_ADD("B_YELLOW", "\033[48;5;251m");
    PFto_ADD("B_Yellow", "\033[48;5;248m");
    PFto_ADD("B_Yellow1", "\033[48;5;248m");
    PFto_ADD("B_Yellow2", "\033[48;5;247m");
    PFto_ADD("B_Yellow3", "\033[48;5;245m");
    PFto_ADD("B_Yellow4", "\033[48;5;240m");
    PFto_ADD("B_YellowGreen", "\033[48;5;245m");
    PFto_ADD("Beige", "\033[38;5;255m");
    PFto_ADD("Bisque", "\033[38;5;254m");
    PFto_ADD("Bisque1", "\033[38;5;254m");
    PFto_ADD("Bisque2", "\033[38;5;252m");
    PFto_ADD("Bisque3", "\033[38;5;249m");
    PFto_ADD("Bisque4", "\033[38;5;244m");
    PFto_ADD("Black", "\033[38;5;232m");
    PFto_ADD("BlanchedAlmond", "\033[38;5;254m");
    PFto_ADD("Blue", "\033[38;5;240m");
    PFto_ADD("Blue1", "\033[38;5;240m");
    PFto_ADD("Blue2", "\033[38;5;239m");
    PFto_ADD("Blue3", "\033[38;5;238m");
    PFto_ADD("Blue4", "\033[38;5;236m");
    PFto_ADD("BlueViolet", "\033[38;5;245m");
    PFto_ADD("Brown", "\033[38;5;239m");
    PFto_ADD("Brown1", "\033[38;5;244m");
    PFto_ADD("Brown2", "\033[38;5;243m");
    PFto_ADD("Brown3", "\033[38;5;241m");
    PFto_ADD("Brown4", "\033[38;5;238m");
    PFto_ADD("Burlywood", "\033[38;5;249m");
    PFto_ADD("Burlywood1", "\033[38;5;252m");
    PFto_ADD("Burlywood2", "\033[38;5;251m");
    PFto_ADD("Burlywood3", "\033[38;5;248m");
    PFto_ADD("Burlywood4", "\033[38;5;242m");
    PFto_ADD("CLEARLINE", "\033[L\033[G");
    PFto_ADD("CURS_DOWN", "\033[B");
    PFto_ADD("CURS_LEFT", "\033[D");
    PFto_ADD("CURS_RIGHT", "\033[C");
    PFto_ADD("CURS_UP", "\033[A");
    PFto_ADD("CYAN", "\033[38;5;244m");
    PFto_ADD("CadetBlue", "\033[38;5;245m");
    PFto_ADD("CadetBlue1", "\033[38;5;253m");
    PFto_ADD("CadetBlue2", "\033[38;5;251m");
    PFto_ADD("CadetBlue3", "\033[38;5;249m");
    PFto_ADD("CadetBlue4", "\033[38;5;243m");
    PFto_ADD("Chartreuse", "\033[38;5;244m");
    PFto_ADD("Chartreuse1", "\033[38;5;244m");
    PFto_ADD("Chartreuse2", "\033[38;5;243m");
    PFto_ADD("Chartreuse3", "\033[38;5;241m");
    PFto_ADD("Chartreuse4", "\033[38;5;238m");
    PFto_ADD("Chocolate", "\033[38;5;243m");
    PFto_ADD("Chocolate1", "\033[38;5;245m");
    PFto_ADD("Chocolate2", "\033[38;5;244m");
    PFto_ADD("Chocolate3", "\033[38;5;242m");
    PFto_ADD("Chocolate4", "\033[38;5;239m");
    PFto_ADD("Coral", "\033[38;5;247m");
    PFto_ADD("Coral1", "\033[38;5;246m");
    PFto_ADD("Coral2", "\033[38;5;245m");
    PFto_ADD("Coral3", "\033[38;5;243m");
    PFto_ADD("Coral4", "\033[38;5;239m");
    PFto_ADD("CornflowerBlue", "\033[38;5;247m");
    PFto_ADD("Cornsilk", "\033[38;5;255m");
    PFto_ADD("Cornsilk1", "\033[38;5;255m");
    PFto_ADD("Cornsilk2", "\033[38;5;254m");
    PFto_ADD("Cornsilk3", "\033[38;5;251m");
    PFto_ADD("Cornsilk4", "\033[38;5;244m");
    PFto_ADD("Cyan", "\033[38;5;248m");
    PFto_ADD("Cyan1", "\033[38;5;248m");
    PFto_ADD("Cyan2", "\033[38;5;247m");
    PFto_ADD("Cyan3", "\033[38;5;245m");
    PFto_ADD("Cyan4", "\033[38;5;240m");
    PFto_ADD("DARKGREY", "\033[38;5;240m");
    PFto_ADD("DarkBlue", "\033[38;5;236m");
    PFto_ADD("DarkCyan", "\033[38;5;240m");
    PFto_ADD("DarkGoldenrod", "\033[38;5;242m");
    PFto_ADD("DarkGoldenrod1", "\033[38;5;246m");
    PFto_ADD("DarkGoldenrod2", "\033[38;5;245m");
    PFto_ADD("DarkGoldenrod3", "\033[38;5;243m");
    PFto_ADD("DarkGoldenrod4", "\033[38;5;239m");
    PFto_ADD("DarkGray", "\033[38;5;248m");
    PFto_ADD("DarkGreen", "\033[38;5;235m");
    PFto_ADD("DarkGrey", "\033[38;5;248m");
    PFto_ADD("DarkKhaki", "\033[38;5;247m");
    PFto_ADD("DarkMagenta", "\033[38;5;240m");
    PFto_ADD("DarkOliveGreen", "\033[38;5;239m");
    PFto_ADD("DarkOliveGreen1", "\033[38;5;250m");
    PFto_ADD("DarkOliveGreen2", "\033[38;5;249m");
    PFto_ADD("DarkOliveGreen3", "\033[38;5;246m");
    PFto_ADD("DarkOliveGreen4", "\033[38;5;242m");
    PFto_ADD("DarkOrange", "\033[38;5;244m");
    PFto_ADD("DarkOrange1", "\033[38;5;244m");
    PFto_ADD("DarkOrange2", "\033[38;5;243m");
    PFto_ADD("DarkOrange3", "\033[38;5;241m");
    PFto_ADD("DarkOrange4", "\033[38;5;238m");
    PFto_ADD("DarkOrchid", "\033[38;5;245m");
    PFto_ADD("DarkOrchid1", "\033[38;5;248m");
    PFto_ADD("DarkOrchid2", "\033[38;5;247m");
    PFto_ADD("DarkOrchid3", "\033[38;5;245m");
    PFto_ADD("DarkOrchid4", "\033[38;5;240m");
    PFto_ADD("DarkRed", "\033[38;5;236m");
    PFto_ADD("DarkSalmon", "\033[38;5;248m");
    PFto_ADD("DarkSeaGreen", "\033[38;5;247m");
    PFto_ADD("DarkSeaGreen1", "\033[38;5;253m");
    PFto_ADD("DarkSeaGreen2", "\033[38;5;251m");
    PFto_ADD("DarkSeaGreen3", "\033[38;5;248m");
    PFto_ADD("DarkSeaGreen4", "\033[38;5;243m");
    PFto_ADD("DarkSlateBlue", "\033[38;5;240m");
    PFto_ADD("DarkSlateGray", "\033[38;5;238m");
    PFto_ADD("DarkSlateGray1", "\033[38;5;253m");
    PFto_ADD("DarkSlateGray2", "\033[38;5;252m");
    PFto_ADD("DarkSlateGray3", "\033[38;5;249m");
    PFto_ADD("DarkSlateGray4", "\033[38;5;243m");
    PFto_ADD("DarkSlateGrey", "\033[38;5;238m");
    PFto_ADD("DarkTurquoise", "\033[38;5;245m");
    PFto_ADD("DarkViolet", "\033[38;5;243m");
    PFto_ADD("DeepPink", "\033[38;5;245m");
    PFto_ADD("DeepPink1", "\033[38;5;245m");
    PFto_ADD("DeepPink2", "\033[38;5;244m");
    PFto_ADD("DeepPink3", "\033[38;5;242m");
    PFto_ADD("DeepPink4", "\033[38;5;239m");
    PFto_ADD("DeepSkyBlue", "\033[38;5;246m");
    PFto_ADD("DeepSkyBlue1", "\033[38;5;246m");
    PFto_ADD("DeepSkyBlue2", "\033[38;5;245m");
    PFto_ADD("DeepSkyBlue3", "\033[38;5;243m");
    PFto_ADD("DeepSkyBlue4", "\033[38;5;239m");
    PFto_ADD("DimGray", "\033[38;5;242m");
    PFto_ADD("DimGrey", "\033[38;5;242m");
    PFto_ADD("DodgerBlue", "\033[38;5;245m");
    PFto_ADD("DodgerBlue1", "\033[38;5;245m");
    PFto_ADD("DodgerBlue2", "\033[38;5;245m");
    PFto_ADD("DodgerBlue3", "\033[38;5;243m");
    PFto_ADD("DodgerBlue4", "\033[38;5;239m");
    PFto_ADD("ENDTERM", "");
    PFto_ADD("F000", "\033[38;5;232m");
    PFto_ADD("F001", "\033[38;5;234m");
    PFto_ADD("F002", "\033[38;5;236m");
    PFto_ADD("F003", "\033[38;5;237m");
    PFto_ADD("F004", "\033[38;5;239m");
    PFto_ADD("F005", "\033[38;5;240m");
    PFto_ADD("F010", "\033[38;5;234m");
    PFto_ADD("F011", "\033[38;5;237m");
    PFto_ADD("F012", "\033[38;5;239m");
    PFto_ADD("F013", "\033[38;5;240m");
    PFto_ADD("F014", "\033[38;5;241m");
    PFto_ADD("F015", "\033[38;5;243m");
    PFto_ADD("F020", "\033[38;5;236m");
    PFto_ADD("F021", "\033[38;5;239m");
    PFto_ADD("F022", "\033[38;5;240m");
    PFto_ADD("F023", "\033[38;5;242m");
    PFto_ADD("F024", "\033[38;5;243m");
    PFto_ADD("F025", "\033[38;5;244m");
    PFto_ADD("F030", "\033[38;5;237m");
    PFto_ADD("F031", "\033[38;5;240m");
    PFto_ADD("F032", "\033[38;5;242m");
    PFto_ADD("F033", "\033[38;5;244m");
    PFto_ADD("F034", "\033[38;5;245m");
    PFto_ADD("F035", "\033[38;5;246m");
    PFto_ADD("F040", "\033[38;5;239m");
    PFto_ADD("F041", "\033[38;5;241m");
    PFto_ADD("F042", "\033[38;5;243m");
    PFto_ADD("F043", "\033[38;5;245m");
    PFto_ADD("F044", "\033[38;5;246m");
    PFto_ADD("F045", "\033[38;5;247m");
    PFto_ADD("F050", "\033[38;5;240m");
    PFto_ADD("F051", "\033[38;5;243m");
    PFto_ADD("F052", "\033[38;5;244m");
    PFto_ADD("F053", "\033[38;5;246m");
    PFto_ADD("F054", "\033[38;5;247m");
    PFto_ADD("F055", "\033[38;5;248m");
    PFto_ADD("F100", "\033[38;5;234m");
    PFto_ADD("F101", "\033[38;5;237m");
    PFto_ADD("F102", "\033[38;5;239m");
    PFto_ADD("F103", "\033[38;5;240m");
    PFto_ADD("F104", "\033[38;5;241m");
    PFto_ADD("F105", "\033[38;5;243m");
    PFto_ADD("F110", "\033[38;5;237m");
    PFto_ADD("F111", "\033[38;5;240m");
    PFto_ADD("F112", "\033[38;5;241m");
    PFto_ADD("F113", "\033[38;5;243m");
    PFto_ADD("F114", "\033[38;5;244m");
    PFto_ADD("F115", "\033[38;5;245m");
    PFto_ADD("F120", "\033[38;5;239m");
    PFto_ADD("F121", "\033[38;5;241m");
    PFto_ADD("F122", "\033[38;5;243m");
    PFto_ADD("F123", "\033[38;5;245m");
    PFto_ADD("F124", "\033[38;5;246m");
    PFto_ADD("F125", "\033[38;5;247m");
    PFto_ADD("F130", "\033[38;5;240m");
    PFto_ADD("F131", "\033[38;5;243m");
    PFto_ADD("F132", "\033[38;5;245m");
    PFto_ADD("F133", "\033[38;5;246m");
    PFto_ADD("F134", "\033[38;5;248m");
    PFto_ADD("F135", "\033[38;5;249m");
    PFto_ADD("F140", "\033[38;5;241m");
    PFto_ADD("F141", "\033[38;5;244m");
    PFto_ADD("F142", "\033[38;5;246m");
    PFto_ADD("F143", "\033[38;5;248m");
    PFto_ADD("F144", "\033[38;5;249m");
    PFto_ADD("F145", "\033[38;5;250m");
    PFto_ADD("F150", "\033[38;5;243m");
    PFto_ADD("F151", "\033[38;5;245m");
    PFto_ADD("F152", "\033[38;5;247m");
    PFto_ADD("F153", "\033[38;5;249m");
    PFto_ADD("F154", "\033[38;5;250m");
    PFto_ADD("F155", "\033[38;5;251m");
    PFto_ADD("F200", "\033[38;5;236m");
    PFto_ADD("F201", "\033[38;5;239m");
    PFto_ADD("F202", "\033[38;5;240m");
    PFto_ADD("F203", "\033[38;5;242m");
    PFto_ADD("F204", "\033[38;5;243m");
    PFto_ADD("F205", "\033[38;5;244m");
    PFto_ADD("F210", "\033[38;5;239m");
    PFto_ADD("F211", "\033[38;5;241m");
    PFto_ADD("F212", "\033[38;5;243m");
    PFto_ADD("F213", "\033[38;5;245m");
    PFto_ADD("F214", "\033[38;5;246m");
    PFto_ADD("F215", "\033[38;5;247m");
    PFto_ADD("F220", "\033[38;5;240m");
    PFto_ADD("F221", "\033[38;5;243m");
    PFto_ADD("F222", "\033[38;5;245m");
    PFto_ADD("F223", "\033[38;5;246m");
    PFto_ADD("F224", "\033[38;5;248m");
    PFto_ADD("F225", "\033[38;5;249m");
    PFto_ADD("F230", "\033[38;5;242m");
    PFto_ADD("F231", "\033[38;5;245m");
    PFto_ADD("F232", "\033[38;5;246m");
    PFto_ADD("F233", "\033[38;5;248m");
    PFto_ADD("F234", "\033[38;5;249m");
    PFto_ADD("F235", "\033[38;5;250m");
    PFto_ADD("F240", "\033[38;5;243m");
    PFto_ADD("F241", "\033[38;5;246m");
    PFto_ADD("F242", "\033[38;5;248m");
    PFto_ADD("F243", "\033[38;5;249m");
    PFto_ADD("F244", "\033[38;5;250m");
    PFto_ADD("F245", "\033[38;5;252m");
    PFto_ADD("F250", "\033[38;5;244m");
    PFto_ADD("F251", "\033[38;5;247m");
    PFto_ADD("F252", "\033[38;5;249m");
    PFto_ADD("F253", "\033[38;5;250m");
    PFto_ADD("F254", "\033[38;5;252m");
    PFto_ADD("F255", "\033[38;5;253m");
    PFto_ADD("F300", "\033[38;5;237m");
    PFto_ADD("F301", "\033[38;5;240m");
    PFto_ADD("F302", "\033[38;5;242m");
    PFto_ADD("F303", "\033[38;5;244m");
    PFto_ADD("F304", "\033[38;5;245m");
    PFto_ADD("F305", "\033[38;5;246m");
    PFto_ADD("F310", "\033[38;5;240m");
    PFto_ADD("F311", "\033[38;5;243m");
    PFto_ADD("F312", "\033[38;5;245m");
    PFto_ADD("F313", "\033[38;5;246m");
    PFto_ADD("F314", "\033[38;5;248m");
    PFto_ADD("F315", "\033[38;5;249m");
    PFto_ADD("F320", "\033[38;5;242m");
    PFto_ADD("F321", "\033[38;5;245m");
    PFto_ADD("F322", "\033[38;5;246m");
    PFto_ADD("F323", "\033[38;5;248m");
    PFto_ADD("F324", "\033[38;5;249m");
    PFto_ADD("F325", "\033[38;5;250m");
    PFto_ADD("F330", "\033[38;5;244m");
    PFto_ADD("F331", "\033[38;5;246m");
    PFto_ADD("F332", "\033[38;5;248m");
    PFto_ADD("F333", "\033[38;5;250m");
    PFto_ADD("F334", "\033[38;5;251m");
    PFto_ADD("F335", "\033[38;5;252m");
    PFto_ADD("F340", "\033[38;5;245m");
    PFto_ADD("F341", "\033[38;5;248m");
    PFto_ADD("F342", "\033[38;5;249m");
    PFto_ADD("F343", "\033[38;5;251m");
    PFto_ADD("F344", "\033[38;5;252m");
    PFto_ADD("F345", "\033[38;5;253m");
    PFto_ADD("F350", "\033[38;5;246m");
    PFto_ADD("F351", "\033[38;5;249m");
    PFto_ADD("F352", "\033[38;5;250m");
    PFto_ADD("F353", "\033[38;5;252m");
    PFto_ADD("F354", "\033[38;5;253m");
    PFto_ADD("F355", "\033[38;5;254m");
    PFto_ADD("F400", "\033[38;5;239m");
    PFto_ADD("F401", "\033[38;5;241m");
    PFto_ADD("F402", "\033[38;5;243m");
    PFto_ADD("F403", "\033[38;5;245m");
    PFto_ADD("F404", "\033[38;5;246m");
    PFto_ADD("F405", "\033[38;5;247m");
    PFto_ADD("F410", "\033[38;5;241m");
    PFto_ADD("F411", "\033[38;5;244m");
    PFto_ADD("F412", "\033[38;5;246m");
    PFto_ADD("F413", "\033[38;5;248m");
    PFto_ADD("F414", "\033[38;5;249m");
    PFto_ADD("F415", "\033[38;5;250m");
    PFto_ADD("F420", "\033[38;5;243m");
    PFto_ADD("F421", "\033[38;5;246m");
    PFto_ADD("F422", "\033[38;5;248m");
    PFto_ADD("F423", "\033[38;5;249m");
    PFto_ADD("F424", "\033[38;5;250m");
    PFto_ADD("F425", "\033[38;5;252m");
    PFto_ADD("F430", "\033[38;5;245m");
    PFto_ADD("F431", "\033[38;5;248m");
    PFto_ADD("F432", "\033[38;5;249m");
    PFto_ADD("F433", "\033[38;5;251m");
    PFto_ADD("F434", "\033[38;5;252m");
    PFto_ADD("F435", "\033[38;5;253m");
    PFto_ADD("F440", "\033[38;5;246m");
    PFto_ADD("F441", "\033[38;5;249m");
    PFto_ADD("F442", "\033[38;5;250m");
    PFto_ADD("F443", "\033[38;5;252m");
    PFto_ADD("F444", "\033[38;5;253m");
    PFto_ADD("F445", "\033[38;5;254m");
    PFto_ADD("F450", "\033[38;5;247m");
    PFto_ADD("F451", "\033[38;5;250m");
    PFto_ADD("F452", "\033[38;5;252m");
    PFto_ADD("F453", "\033[38;5;253m");
    PFto_ADD("F454", "\033[38;5;254m");
    PFto_ADD("F455", "\033[38;5;255m");
    PFto_ADD("F500", "\033[38;5;240m");
    PFto_ADD("F501", "\033[38;5;243m");
    PFto_ADD("F502", "\033[38;5;244m");
    PFto_ADD("F503", "\033[38;5;246m");
    PFto_ADD("F504", "\033[38;5;247m");
    PFto_ADD("F505", "\033[38;5;248m");
    PFto_ADD("F510", "\033[38;5;243m");
    PFto_ADD("F511", "\033[38;5;245m");
    PFto_ADD("F512", "\033[38;5;247m");
    PFto_ADD("F513", "\033[38;5;249m");
    PFto_ADD("F514", "\033[38;5;250m");
    PFto_ADD("F515", "\033[38;5;251m");
    PFto_ADD("F520", "\033[38;5;244m");
    PFto_ADD("F521", "\033[38;5;247m");
    PFto_ADD("F522", "\033[38;5;249m");
    PFto_ADD("F523", "\033[38;5;250m");
    PFto_ADD("F524", "\033[38;5;252m");
    PFto_ADD("F525", "\033[38;5;253m");
    PFto_ADD("F530", "\033[38;5;246m");
    PFto_ADD("F531", "\033[38;5;249m");
    PFto_ADD("F532", "\033[38;5;250m");
    PFto_ADD("F533", "\033[38;5;252m");
    PFto_ADD("F534", "\033[38;5;253m");
    PFto_ADD("F535", "\033[38;5;254m");
    PFto_ADD("F540", "\033[38;5;247m");
    PFto_ADD("F541", "\033[38;5;250m");
    PFto_ADD("F542", "\033[38;5;252m");
    PFto_ADD("F543", "\033[38;5;253m");
    PFto_ADD("F544", "\033[38;5;254m");
    PFto_ADD("F545", "\033[38;5;255m");
    PFto_ADD("F550", "\033[38;5;248m");
    PFto_ADD("F551", "\033[38;5;251m");
    PFto_ADD("F552", "\033[38;5;253m");
    PFto_ADD("F553", "\033[38;5;254m");
    PFto_ADD("F554", "\033[38;5;255m");
    PFto_ADD("F555", "\033[38;5;255m");
    PFto_ADD("FLASH", "\033[5m");
    PFto_ADD("Firebrick", "\033[38;5;239m");
    PFto_ADD("Firebrick1", "\033[38;5;243m");
    PFto_ADD("Firebrick2", "\033[38;5;242m");
    PFto_ADD("Firebrick3", "\033[38;5;241m");
    PFto_ADD("Firebrick4", "\033[38;5;238m");
    PFto_ADD("FloralWhite", "\033[38;5;255m");
    PFto_ADD("ForestGreen", "\033[38;5;238m");
    PFto_ADD("G00", "\033[38;5;232m");
    PFto_ADD("G01", "\033[38;5;232m");
    PFto_ADD("G02", "\033[38;5;233m");
    PFto_ADD("G03", "\033[38;5;234m");
    PFto_ADD("G04", "\033[38;5;235m");
    PFto_ADD("G05", "\033[38;5;236m");
    PFto_ADD("G06", "\033[38;5;237m");
    PFto_ADD("G07", "\033[38;5;238m");
    PFto_ADD("G08", "\033[38;5;239m");
    PFto_ADD("G09", "\033[38;5;240m");
    PFto_ADD("G10", "\033[38;5;241m");
    PFto_ADD("G11", "\033[38;5;242m");
    PFto_ADD("G12", "\033[38;5;243m");
    PFto_ADD("G13", "\033[38;5;244m");
    PFto_ADD("G14", "\033[38;5;245m");
    PFto_ADD("G15", "\033[38;5;246m");
    PFto_ADD("G16", "\033[38;5;247m");
    PFto_ADD("G17", "\033[38;5;248m");
    PFto_ADD("G18", "\033[38;5;249m");
    PFto_ADD("G19", "\033[38;5;250m");
    PFto_ADD("G20", "\033[38;5;251m");
    PFto_ADD("G21", "\033[38;5;252m");
    PFto_ADD("G22", "\033[38;5;253m");
    PFto_ADD("G23", "\033[38;5;254m");
    PFto_ADD("G24", "\033[38;5;255m");
    PFto_ADD("G25", "\033[38;5;255m");
    PFto_ADD("GREEN", "\033[38;5;237m");
    PFto_ADD("GREY", "\033[38;5;250m");
    PFto_ADD("Gainsboro", "\033[38;5;253m");
    PFto_ADD("GhostWhite", "\033[38;5;255m");
    PFto_ADD("Gold", "\033[38;5;247m");
    PFto_ADD("Gold1", "\033[38;5;247m");
    PFto_ADD("Gold2", "\033[38;5;246m");
    PFto_ADD("Gold3", "\033[38;5;244m");
    PFto_ADD("Gold4", "\033[38;5;240m");
    PFto_ADD("Goldenrod", "\033[38;5;245m");
    PFto_ADD("Goldenrod1", "\033[38;5;247m");
    PFto_ADD("Goldenrod2", "\033[38;5;246m");
    PFto_ADD("Goldenrod3", "\033[38;5;244m");
    PFto_ADD("Goldenrod4", "\033[38;5;240m");
    PFto_ADD("Gray", "\033[38;5;250m");
    PFto_ADD("Gray0", "\033[38;5;232m");
    PFto_ADD("Gray1", "\033[38;5;232m");
    PFto_ADD("Gray10", "\033[38;5;234m");
    PFto_ADD("Gray100", "\033[38;5;255m");
    PFto_ADD("Gray11", "\033[38;5;234m");
    PFto_ADD("Gray12", "\033[38;5;234m");
    PFto_ADD("Gray13", "\033[38;5;234m");
    PFto_ADD("Gray14", "\033[38;5;235m");
    PFto_ADD("Gray15", "\033[38;5;235m");
    PFto_ADD("Gray16", "\033[38;5;235m");
    PFto_ADD("Gray17", "\033[38;5;235m");
    PFto_ADD("Gray18", "\033[38;5;236m");
    PFto_ADD("Gray19", "\033[38;5;236m");
    PFto_ADD("Gray2", "\033[38;5;232m");
    PFto_ADD("Gray20", "\033[38;5;236m");
    PFto_ADD("Gray21", "\033[38;5;237m");
    PFto_ADD("Gray22", "\033[38;5;237m");
    PFto_ADD("Gray23", "\033[38;5;237m");
    PFto_ADD("Gray24", "\033[38;5;237m");
    PFto_ADD("Gray25", "\033[38;5;238m");
    PFto_ADD("Gray26", "\033[38;5;238m");
    PFto_ADD("Gray27", "\033[38;5;238m");
    PFto_ADD("Gray28", "\033[38;5;238m");
    PFto_ADD("Gray29", "\033[38;5;239m");
    PFto_ADD("Gray3", "\033[38;5;232m");
    PFto_ADD("Gray30", "\033[38;5;239m");
    PFto_ADD("Gray31", "\033[38;5;239m");
    PFto_ADD("Gray32", "\033[38;5;239m");
    PFto_ADD("Gray33", "\033[38;5;240m");
    PFto_ADD("Gray34", "\033[38;5;240m");
    PFto_ADD("Gray35", "\033[38;5;240m");
    PFto_ADD("Gray36", "\033[38;5;240m");
    PFto_ADD("Gray37", "\033[38;5;241m");
    PFto_ADD("Gray38", "\033[38;5;241m");
    PFto_ADD("Gray39", "\033[38;5;241m");
    PFto_ADD("Gray4", "\033[38;5;232m");
    PFto_ADD("Gray40", "\033[38;5;241m");
    PFto_ADD("Gray41", "\033[38;5;242m");
    PFto_ADD("Gray42", "\033[38;5;242m");
    PFto_ADD("Gray43", "\033[38;5;242m");
    PFto_ADD("Gray44", "\033[38;5;242m");
    PFto_ADD("Gray45", "\033[38;5;243m");
    PFto_ADD("Gray46", "\033[38;5;243m");
    PFto_ADD("Gray47", "\033[38;5;243m");
    PFto_ADD("Gray48", "\033[38;5;243m");
    PFto_ADD("Gray49", "\033[38;5;244m");
    PFto_ADD("Gray5", "\033[38;5;232m");
    PFto_ADD("Gray50", "\033[38;5;244m");
    PFto_ADD("Gray51", "\033[38;5;244m");
    PFto_ADD("Gray52", "\033[38;5;244m");
    PFto_ADD("Gray53", "\033[38;5;245m");
    PFto_ADD("Gray54", "\033[38;5;245m");
    PFto_ADD("Gray55", "\033[38;5;245m");
    PFto_ADD("Gray56", "\033[38;5;245m");
    PFto_ADD("Gray57", "\033[38;5;246m");
    PFto_ADD("Gray58", "\033[38;5;246m");
    PFto_ADD("Gray59", "\033[38;5;246m");
    PFto_ADD("Gray6", "\033[38;5;233m");
    PFto_ADD("Gray60", "\033[38;5;246m");
    PFto_ADD("Gray61", "\033[38;5;247m");
    PFto_ADD("Gray62", "\033[38;5;247m");
    PFto_ADD("Gray63", "\033[38;5;247m");
    PFto_ADD("Gray64", "\033[38;5;247m");
    PFto_ADD("Gray65", "\033[38;5;248m");
    PFto_ADD("Gray66", "\033[38;5;248m");
    PFto_ADD("Gray67", "\033[38;5;248m");
    PFto_ADD("Gray68", "\033[38;5;248m");
    PFto_ADD("Gray69", "\033[38;5;249m");
    PFto_ADD("Gray7", "\033[38;5;233m");
    PFto_ADD("Gray70", "\033[38;5;249m");
    PFto_ADD("Gray71", "\033[38;5;249m");
    PFto_ADD("Gray72", "\033[38;5;250m");
    PFto_ADD("Gray73", "\033[38;5;250m");
    PFto_ADD("Gray74", "\033[38;5;250m");
    PFto_ADD("Gray75", "\033[38;5;250m");
    PFto_ADD("Gray76", "\033[38;5;251m");
    PFto_ADD("Gray77", "\033[38;5;251m");
    PFto_ADD("Gray78", "\033[38;5;251m");
    PFto_ADD("Gray79", "\033[38;5;251m");
    PFto_ADD("Gray8", "\033[38;5;233m");
    PFto_ADD("Gray80", "\033[38;5;252m");
    PFto_ADD("Gray81", "\033[38;5;252m");
    PFto_ADD("Gray82", "\033[38;5;252m");
    PFto_ADD("Gray83", "\033[38;5;252m");
    PFto_ADD("Gray84", "\033[38;5;253m");
    PFto_ADD("Gray85", "\033[38;5;253m");
    PFto_ADD("Gray86", "\033[38;5;253m");
    PFto_ADD("Gray87", "\033[38;5;253m");
    PFto_ADD("Gray88", "\033[38;5;254m");
    PFto_ADD("Gray89", "\033[38;5;254m");
    PFto_ADD("Gray9", "\033[38;5;233m");
    PFto_ADD("Gray90", "\033[38;5;254m");
    PFto_ADD("Gray91", "\033[38;5;254m");
    PFto_ADD("Gray92", "\033[38;5;255m");
    PFto_ADD("Gray93", "\033[38;5;255m");
    PFto_ADD("Gray94", "\033[38;5;255m");
    PFto_ADD("Gray95", "\033[38;5;255m");
    PFto_ADD("Gray96", "\033[38;5;255m");
    PFto_ADD("Gray97", "\033[38;5;255m");
    PFto_ADD("Gray98", "\033[38;5;255m");
    PFto_ADD("Gray99", "\033[38;5;255m");
    PFto_ADD("Green", "\033[38;5;240m");
    PFto_ADD("Green1", "\033[38;5;240m");
    PFto_ADD("Green2", "\033[38;5;239m");
    PFto_ADD("Green3", "\033[38;5;238m");
    PFto_ADD("Green4", "\033[38;5;236m");
    PFto_ADD("GreenYellow", "\033[38;5;247m");
    PFto_ADD("Grey", "\033[38;5;250m");
    PFto_ADD("Grey0", "\033[38;5;232m");
    PFto_ADD("Grey1", "\033[38;5;232m");
    PFto_ADD("Grey10", "\033[38;5;234m");
    PFto_ADD("Grey100", "\033[38;5;255m");
    PFto_ADD("Grey11", "\033[38;5;234m");
    PFto_ADD("Grey12", "\033[38;5;234m");
    PFto_ADD("Grey13", "\033[38;5;234m");
    PFto_ADD("Grey14", "\033[38;5;235m");
    PFto_ADD("Grey15", "\033[38;5;235m");
    PFto_ADD("Grey16", "\033[38;5;235m");
    PFto_ADD("Grey17", "\033[38;5;235m");
    PFto_ADD("Grey18", "\033[38;5;236m");
    PFto_ADD("Grey19", "\033[38;5;236m");
    PFto_ADD("Grey2", "\033[38;5;232m");
    PFto_ADD("Grey20", "\033[38;5;236m");
    PFto_ADD("Grey21", "\033[38;5;237m");
    PFto_ADD("Grey22", "\033[38;5;237m");
    PFto_ADD("Grey23", "\033[38;5;237m");
    PFto_ADD("Grey24", "\033[38;5;237m");
    PFto_ADD("Grey25", "\033[38;5;238m");
    PFto_ADD("Grey26", "\033[38;5;238m");
    PFto_ADD("Grey27", "\033[38;5;238m");
    PFto_ADD("Grey28", "\033[38;5;238m");
    PFto_ADD("Grey29", "\033[38;5;239m");
    PFto_ADD("Grey3", "\033[38;5;232m");
    PFto_ADD("Grey30", "\033[38;5;239m");
    PFto_ADD("Grey31", "\033[38;5;239m");
    PFto_ADD("Grey32", "\033[38;5;239m");
    PFto_ADD("Grey33", "\033[38;5;240m");
    PFto_ADD("Grey34", "\033[38;5;240m");
    PFto_ADD("Grey35", "\033[38;5;240m");
    PFto_ADD("Grey36", "\033[38;5;240m");
    PFto_ADD("Grey37", "\033[38;5;241m");
    PFto_ADD("Grey38", "\033[38;5;241m");
    PFto_ADD("Grey39", "\033[38;5;241m");
    PFto_ADD("Grey4", "\033[38;5;232m");
    PFto_ADD("Grey40", "\033[38;5;241m");
    PFto_ADD("Grey41", "\033[38;5;242m");
    PFto_ADD("Grey42", "\033[38;5;242m");
    PFto_ADD("Grey43", "\033[38;5;242m");
    PFto_ADD("Grey44", "\033[38;5;242m");
    PFto_ADD("Grey45", "\033[38;5;243m");
    PFto_ADD("Grey46", "\033[38;5;243m");
    PFto_ADD("Grey47", "\033[38;5;243m");
    PFto_ADD("Grey48", "\033[38;5;243m");
    PFto_ADD("Grey49", "\033[38;5;244m");
    PFto_ADD("Grey5", "\033[38;5;232m");
    PFto_ADD("Grey50", "\033[38;5;244m");
    PFto_ADD("Grey51", "\033[38;5;244m");
    PFto_ADD("Grey52", "\033[38;5;244m");
    PFto_ADD("Grey53", "\033[38;5;245m");
    PFto_ADD("Grey54", "\033[38;5;245m");
    PFto_ADD("Grey55", "\033[38;5;245m");
    PFto_ADD("Grey56", "\033[38;5;245m");
    PFto_ADD("Grey57", "\033[38;5;246m");
    PFto_ADD("Grey58", "\033[38;5;246m");
    PFto_ADD("Grey59", "\033[38;5;246m");
    PFto_ADD("Grey6", "\033[38;5;233m");
    PFto_ADD("Grey60", "\033[38;5;246m");
    PFto_ADD("Grey61", "\033[38;5;247m");
    PFto_ADD("Grey62", "\033[38;5;247m");
    PFto_ADD("Grey63", "\033[38;5;247m");
    PFto_ADD("Grey64", "\033[38;5;247m");
    PFto_ADD("Grey65", "\033[38;5;248m");
    PFto_ADD("Grey66", "\033[38;5;248m");
    PFto_ADD("Grey67", "\033[38;5;248m");
    PFto_ADD("Grey68", "\033[38;5;248m");
    PFto_ADD("Grey69", "\033[38;5;249m");
    PFto_ADD("Grey7", "\033[38;5;233m");
    PFto_ADD("Grey70", "\033[38;5;249m");
    PFto_ADD("Grey71", "\033[38;5;249m");
    PFto_ADD("Grey72", "\033[38;5;250m");
    PFto_ADD("Grey73", "\033[38;5;250m");
    PFto_ADD("Grey74", "\033[38;5;250m");
    PFto_ADD("Grey75", "\033[38;5;250m");
    PFto_ADD("Grey76", "\033[38;5;251m");
    PFto_ADD("Grey77", "\033[38;5;251m");
    PFto_ADD("Grey78", "\033[38;5;251m");
    PFto_ADD("Grey79", "\033[38;5;251m");
    PFto_ADD("Grey8", "\033[38;5;233m");
    PFto_ADD("Grey80", "\033[38;5;252m");
    PFto_ADD("Grey81", "\033[38;5;252m");
    PFto_ADD("Grey82", "\033[38;5;252m");
    PFto_ADD("Grey83", "\033[38;5;252m");
    PFto_ADD("Grey84", "\033[38;5;253m");
    PFto_ADD("Grey85", "\033[38;5;253m");
    PFto_ADD("Grey86", "\033[38;5;253m");
    PFto_ADD("Grey87", "\033[38;5;253m");
    PFto_ADD("Grey88", "\033[38;5;254m");
    PFto_ADD("Grey89", "\033[38;5;254m");
    PFto_ADD("Grey9", "\033[38;5;233m");
    PFto_ADD("Grey90", "\033[38;5;254m");
    PFto_ADD("Grey91", "\033[38;5;254m");
    PFto_ADD("Grey92", "\033[38;5;255m");
    PFto_ADD("Grey93", "\033[38;5;255m");
    PFto_ADD("Grey94", "\033[38;5;255m");
    PFto_ADD("Grey95", "\033[38;5;255m");
    PFto_ADD("Grey96", "\033[38;5;255m");
    PFto_ADD("Grey97", "\033[38;5;255m");
    PFto_ADD("Grey98", "\033[38;5;255m");
    PFto_ADD("Grey99", "\033[38;5;255m");
    PFto_ADD("HOME", "\033[H");
    PFto_ADD("Honeydew", "\033[38;5;255m");
    PFto_ADD("Honeydew1", "\033[38;5;255m");
    PFto_ADD("Honeydew2", "\033[38;5;254m");
    PFto_ADD("Honeydew3", "\033[38;5;251m");
    PFto_ADD("Honeydew4", "\033[38;5;245m");
    PFto_ADD("HotPink", "\033[38;5;249m");
    PFto_ADD("HotPink1", "\033[38;5;249m");
    PFto_ADD("HotPink2", "\033[38;5;248m");
    PFto_ADD("HotPink3", "\033[38;5;246m");
    PFto_ADD("HotPink4", "\033[38;5;241m");
    PFto_ADD("INITTERM", "\033[H\033[2J");
    PFto_ADD("ITALIC", "\033[3m");
    PFto_ADD("IndianRed", "\033[38;5;244m");
    PFto_ADD("IndianRed1", "\033[38;5;247m");
    PFto_ADD("IndianRed2", "\033[38;5;246m");
    PFto_ADD("IndianRed3", "\033[38;5;244m");
    PFto_ADD("IndianRed4", "\033[38;5;240m");
    PFto_ADD("Ivory", "\033[38;5;255m");
    PFto_ADD("Ivory1", "\033[38;5;255m");
    PFto_ADD("Ivory2", "\033[38;5;255m");
    PFto_ADD("Ivory3", "\033[38;5;251m");
    PFto_ADD("Ivory4", "\033[38;5;245m");
    PFto_ADD("Khaki", "\033[38;5;252m");
    PFto_ADD("Khaki1", "\033[38;5;253m");
    PFto_ADD("Khaki2", "\033[38;5;251m");
    PFto_ADD("Khaki3", "\033[38;5;248m");
    PFto_ADD("Khaki4", "\033[38;5;243m");
    PFto_ADD("LIGHTBLUE", "\033[38;5;245m");
    PFto_ADD("LIGHTCYAN", "\033[38;5;251m");
    PFto_ADD("LIGHTGREEN", "\033[38;5;245m");
    PFto_ADD("LIGHTRED", "\033[38;5;245m");
    PFto_ADD("Lavender", "\033[38;5;255m");
    PFto_ADD("LavenderBlush", "\033[38;5;255m");
    PFto_ADD("LavenderBlush1", "\033[38;5;255m");
    PFto_ADD("LavenderBlush2", "\033[38;5;254m");
    PFto_ADD("LavenderBlush3", "\033[38;5;251m");
    PFto_ADD("LavenderBlush4", "\033[38;5;245m");
    PFto_ADD("LawnGreen", "\033[38;5;244m");
    PFto_ADD("LemonChiffon", "\033[38;5;255m");
    PFto_ADD("LemonChiffon1", "\033[38;5;255m");
    PFto_ADD("LemonChiffon2", "\033[38;5;253m");
    PFto_ADD("LemonChiffon3", "\033[38;5;250m");
    PFto_ADD("LemonChiffon4", "\033[38;5;244m");
    PFto_ADD("LightBlue", "\033[38;5;252m");
    PFto_ADD("LightBlue1", "\033[38;5;254m");
    PFto_ADD("LightBlue2", "\033[38;5;252m");
    PFto_ADD("LightBlue3", "\033[38;5;250m");
    PFto_ADD("LightBlue4", "\033[38;5;244m");
    PFto_ADD("LightCoral", "\033[38;5;248m");
    PFto_ADD("LightCyan", "\033[38;5;255m");
    PFto_ADD("LightCyan1", "\033[38;5;255m");
    PFto_ADD("LightCyan2", "\033[38;5;254m");
    PFto_ADD("LightCyan3", "\033[38;5;251m");
    PFto_ADD("LightCyan4", "\033[38;5;245m");
    PFto_ADD("LightGoldenrod", "\033[38;5;251m");
    PFto_ADD("LightGoldenrod1", "\033[38;5;252m");
    PFto_ADD("LightGoldenrod2", "\033[38;5;251m");
    PFto_ADD("LightGoldenrod3", "\033[38;5;248m");
    PFto_ADD("LightGoldenrod4", "\033[38;5;243m");
    PFto_ADD("LightGoldenrodYellow", "\033[38;5;255m");
    PFto_ADD("LightGray", "\033[38;5;252m");
    PFto_ADD("LightGreen", "\033[38;5;249m");
    PFto_ADD("LightGrey", "\033[38;5;252m");
    PFto_ADD("LightPink", "\033[38;5;252m");
    PFto_ADD("LightPink1", "\033[38;5;252m");
    PFto_ADD("LightPink2", "\033[38;5;250m");
    PFto_ADD("LightPink3", "\033[38;5;248m");
    PFto_ADD("LightPink4", "\033[38;5;242m");
    PFto_ADD("LightSalmon", "\033[38;5;249m");
    PFto_ADD("LightSalmon1", "\033[38;5;249m");
    PFto_ADD("LightSalmon2", "\033[38;5;248m");
    PFto_ADD("LightSalmon3", "\033[38;5;246m");
    PFto_ADD("LightSalmon4", "\033[38;5;241m");
    PFto_ADD("LightSeaGreen", "\033[38;5;244m");
    PFto_ADD("LightSkyBlue", "\033[38;5;251m");
    PFto_ADD("LightSkyBlue1", "\033[38;5;253m");
    PFto_ADD("LightSkyBlue2", "\033[38;5;252m");
    PFto_ADD("LightSkyBlue3", "\033[38;5;249m");
    PFto_ADD("LightSkyBlue4", "\033[38;5;243m");
    PFto_ADD("LightSlateBlue", "\033[38;5;248m");
    PFto_ADD("LightSlateGray", "\033[38;5;245m");
    PFto_ADD("LightSlateGrey", "\033[38;5;245m");
    PFto_ADD("LightSteelBlue", "\033[38;5;251m");
    PFto_ADD("LightSteelBlue1", "\033[38;5;254m");
    PFto_ADD("LightSteelBlue2", "\033[38;5;252m");
    PFto_ADD("LightSteelBlue3", "\033[38;5;249m");
    PFto_ADD("LightSteelBlue4", "\033[38;5;244m");
    PFto_ADD("LightYellow", "\033[38;5;255m");
    PFto_ADD("LightYellow1", "\033[38;5;255m");
    PFto_ADD("LightYellow2", "\033[38;5;254m");
    PFto_ADD("LightYellow3", "\033[38;5;251m");
    PFto_ADD("LightYellow4", "\033[38;5;245m");
    PFto_ADD("LimeGreen", "\033[38;5;241m");
    PFto_ADD("Linen", "\033[38;5;255m");
    PFto_ADD("MAGENTA", "\033[38;5;244m");
    PFto_ADD("Magenta", "\033[38;5;248m");
    PFto_ADD("Magenta1", "\033[38;5;248m");
    PFto_ADD("Magenta2", "\033[38;5;247m");
    PFto_ADD("Magenta3", "\033[38;5;245m");
    PFto_ADD("Magenta4", "\033[38;5;240m");
    PFto_ADD("Maroon", "\033[38;5;242m");
    PFto_ADD("Maroon1", "\033[38;5;247m");
    PFto_ADD("Maroon2", "\033[38;5;246m");
    PFto_ADD("Maroon3", "\033[38;5;244m");
    PFto_ADD("Maroon4", "\033[38;5;240m");
    PFto_ADD("MediumAquamarine", "\033[38;5;247m");
    PFto_ADD("MediumBlue", "\033[38;5;238m");
    PFto_ADD("MediumOrchid", "\033[38;5;247m");
    PFto_ADD("MediumOrchid1", "\033[38;5;251m");
    PFto_ADD("MediumOrchid2", "\033[38;5;249m");
    PFto_ADD("MediumOrchid3", "\033[38;5;247m");
    PFto_ADD("MediumOrchid4", "\033[38;5;242m");
    PFto_ADD("MediumPurple", "\033[38;5;247m");
    PFto_ADD("MediumPurple1", "\033[38;5;250m");
    PFto_ADD("MediumPurple2", "\033[38;5;248m");
    PFto_ADD("MediumPurple3", "\033[38;5;246m");
    PFto_ADD("MediumPurple4", "\033[38;5;241m");
    PFto_ADD("MediumSeaGreen", "\033[38;5;243m");
    PFto_ADD("MediumSlateBlue", "\033[38;5;247m");
    PFto_ADD("MediumSpringGreen", "\033[38;5;245m");
    PFto_ADD("MediumTurquoise", "\033[38;5;247m");
    PFto_ADD("MediumVioletRed", "\033[38;5;243m");
    PFto_ADD("MidnightBlue", "\033[38;5;237m");
    PFto_ADD("MintCream", "\033[38;5;255m");
    PFto_ADD("MistyRose", "\033[38;5;255m");
    PFto_ADD("MistyRose1", "\033[38;5;255m");
    PFto_ADD("MistyRose2", "\033[38;5;253m");
    PFto_ADD("MistyRose3", "\033[38;5;250m");
    PFto_ADD("MistyRose4", "\033[38;5;244m");
    PFto_ADD("Moccasin", "\033[38;5;253m");
    PFto_ADD("NavajoWhite", "\033[38;5;253m");
    PFto_ADD("NavajoWhite1", "\033[38;5;253m");
    PFto_ADD("NavajoWhite2", "\033[38;5;251m");
    PFto_ADD("NavajoWhite3", "\033[38;5;249m");
    PFto_ADD("NavajoWhite4", "\033[38;5;243m");
    PFto_ADD("Navy", "\033[38;5;235m");
    PFto_ADD("NavyBlue", "\033[38;5;235m");
    PFto_ADD("ORANGE", "\033[38;5;244m");
    PFto_ADD("OldLace", "\033[38;5;255m");
    PFto_ADD("OliveDrab", "\033[38;5;241m");
    PFto_ADD("OliveDrab1", "\033[38;5;248m");
    PFto_ADD("OliveDrab2", "\033[38;5;247m");
    PFto_ADD("OliveDrab3", "\033[38;5;245m");
    PFto_ADD("OliveDrab4", "\033[38;5;240m");
    PFto_ADD("Orange", "\033[38;5;245m");
    PFto_ADD("Orange1", "\033[38;5;245m");
    PFto_ADD("Orange2", "\033[38;5;244m");
    PFto_ADD("Orange3", "\033[38;5;242m");
    PFto_ADD("Orange4", "\033[38;5;239m");
    PFto_ADD("OrangeRed", "\033[38;5;242m");
    PFto_ADD("OrangeRed1", "\033[38;5;242m");
    PFto_ADD("OrangeRed2", "\033[38;5;241m");
    PFto_ADD("OrangeRed3", "\033[38;5;240m");
    PFto_ADD("OrangeRed4", "\033[38;5;237m");
    PFto_ADD("Orchid", "\033[38;5;249m");
    PFto_ADD("Orchid1", "\033[38;5;252m");
    PFto_ADD("Orchid2", "\033[38;5;251m");
    PFto_ADD("Orchid3", "\033[38;5;248m");
    PFto_ADD("Orchid4", "\033[38;5;243m");
    PFto_ADD("PINK", "\033[38;5;251m");
    PFto_ADD("PaleGoldenrod", "\033[38;5;253m");
    PFto_ADD("PaleGreen", "\033[38;5;250m");
    PFto_ADD("PaleGreen1", "\033[38;5;250m");
    PFto_ADD("PaleGreen2", "\033[38;5;249m");
    PFto_ADD("PaleGreen3", "\033[38;5;246m");
    PFto_ADD("PaleGreen4", "\033[38;5;241m");
    PFto_ADD("PaleTurquoise", "\033[38;5;253m");
    PFto_ADD("PaleTurquoise1", "\033[38;5;254m");
    PFto_ADD("PaleTurquoise2", "\033[38;5;253m");
    PFto_ADD("PaleTurquoise3", "\033[38;5;250m");
    PFto_ADD("PaleTurquoise4", "\033[38;5;244m");
    PFto_ADD("PaleVioletRed", "\033[38;5;247m");
    PFto_ADD("PaleVioletRed1", "\033[38;5;250m");
    PFto_ADD("PaleVioletRed2", "\033[38;5;248m");
    PFto_ADD("PaleVioletRed3", "\033[38;5;246m");
    PFto_ADD("PaleVioletRed4", "\033[38;5;241m");
    PFto_ADD("PapayaWhip", "\033[38;5;255m");
    PFto_ADD("PeachPuff", "\033[38;5;253m");
    PFto_ADD("PeachPuff1", "\033[38;5;253m");
    PFto_ADD("PeachPuff2", "\033[38;5;252m");
    PFto_ADD("PeachPuff3", "\033[38;5;249m");
    PFto_ADD("PeachPuff4", "\033[38;5;243m");
    PFto_ADD("Peru", "\033[38;5;245m");
    PFto_ADD("Pink", "\033[38;5;253m");
    PFto_ADD("Pink1", "\033[38;5;252m");
    PFto_ADD("Pink2", "\033[38;5;251m");
    PFto_ADD("Pink3", "\033[38;5;248m");
    PFto_ADD("Pink4", "\033[38;5;243m");
    PFto_ADD("Plum", "\033[38;5;251m");
    PFto_ADD("Plum1", "\033[38;5;254m");
    PFto_ADD("Plum2", "\033[38;5;253m");
    PFto_ADD("Plum3", "\033[38;5;250m");
    PFto_ADD("Plum4", "\033[38;5;244m");
    PFto_ADD("PowderBlue", "\033[38;5;252m");
    PFto_ADD("Purple", "\033[38;5;246m");
    PFto_ADD("Purple1", "\033[38;5;246m");
    PFto_ADD("Purple2", "\033[38;5;245m");
    PFto_ADD("Purple3", "\033[38;5;243m");
    PFto_ADD("Purple4", "\033[38;5;240m");
    PFto_ADD("RED", "\033[38;5;237m");
    PFto_ADD("RESET", "\033[0m");
    PFto_ADD("RESTORE", "\033[u");
    PFto_ADD("REVERSE", "\033[7m");
    PFto_ADD("Red", "\033[38;5;240m");
    PFto_ADD("Red1", "\033[38;5;240m");
    PFto_ADD("Red2", "\033[38;5;239m");
    PFto_ADD("Red3", "\033[38;5;238m");
    PFto_ADD("Red4", "\033[38;5;236m");
    PFto_ADD("RosyBrown", "\033[38;5;247m");
    PFto_ADD("RosyBrown1", "\033[38;5;253m");
    PFto_ADD("RosyBrown2", "\033[38;5;251m");
    PFto_ADD("RosyBrown3", "\033[38;5;248m");
    PFto_ADD("RosyBrown4", "\033[38;5;243m");
    PFto_ADD("RoyalBlue", "\033[38;5;244m");
    PFto_ADD("RoyalBlue1", "\033[38;5;246m");
    PFto_ADD("RoyalBlue2", "\033[38;5;245m");
    PFto_ADD("RoyalBlue3", "\033[38;5;243m");
    PFto_ADD("RoyalBlue4", "\033[38;5;239m");
    PFto_ADD("SAVE", "\033[s");
    PFto_ADD("STRIKETHRU", "\033[9m");
    PFto_ADD("SaddleBrown", "\033[38;5;239m");
    PFto_ADD("Salmon", "\033[38;5;248m");
    PFto_ADD("Salmon1", "\033[38;5;248m");
    PFto_ADD("Salmon2", "\033[38;5;247m");
    PFto_ADD("Salmon3", "\033[38;5;245m");
    PFto_ADD("Salmon4", "\033[38;5;240m");
    PFto_ADD("SandyBrown", "\033[38;5;248m");
    PFto_ADD("SeaGreen", "\033[38;5;240m");
    PFto_ADD("SeaGreen1", "\033[38;5;248m");
    PFto_ADD("SeaGreen2", "\033[38;5;247m");
    PFto_ADD("SeaGreen3", "\033[38;5;245m");
    PFto_ADD("SeaGreen4", "\033[38;5;240m");
    PFto_ADD("Seashell", "\033[38;5;255m");
    PFto_ADD("Seashell1", "\033[38;5;255m");
    PFto_ADD("Seashell2", "\033[38;5;254m");
    PFto_ADD("Seashell3", "\033[38;5;251m");
    PFto_ADD("Seashell4", "\033[38;5;245m");
    PFto_ADD("Sienna", "\033[38;5;241m");
    PFto_ADD("Sienna1", "\033[38;5;246m");
    PFto_ADD("Sienna2", "\033[38;5;245m");
    PFto_ADD("Sienna3", "\033[38;5;243m");
    PFto_ADD("Sienna4", "\033[38;5;239m");
    PFto_ADD("SkyBlue", "\033[38;5;250m");
    PFto_ADD("SkyBlue1", "\033[38;5;251m");
    PFto_ADD("SkyBlue2", "\033[38;5;250m");
    PFto_ADD("SkyBlue3", "\033[38;5;247m");
    PFto_ADD("SkyBlue4", "\033[38;5;242m");
    PFto_ADD("SlateBlue", "\033[38;5;245m");
    PFto_ADD("SlateBlue1", "\033[38;5;248m");
    PFto_ADD("SlateBlue2", "\033[38;5;247m");
    PFto_ADD("SlateBlue3", "\033[38;5;244m");
    PFto_ADD("SlateBlue4", "\033[38;5;240m");
    PFto_ADD("SlateGray", "\033[38;5;244m");
    PFto_ADD("SlateGray1", "\033[38;5;254m");
    PFto_ADD("SlateGray2", "\033[38;5;252m");
    PFto_ADD("SlateGray3", "\033[38;5;249m");
    PFto_ADD("SlateGray4", "\033[38;5;244m");
    PFto_ADD("SlateGrey", "\033[38;5;244m");
    PFto_ADD("Snow", "\033[38;5;255m");
    PFto_ADD("Snow1", "\033[38;5;255m");
    PFto_ADD("Snow2", "\033[38;5;255m");
    PFto_ADD("Snow3", "\033[38;5;251m");
    PFto_ADD("Snow4", "\033[38;5;245m");
    PFto_ADD("SpringGreen", "\033[38;5;244m");
    PFto_ADD("SpringGreen1", "\033[38;5;244m");
    PFto_ADD("SpringGreen2", "\033[38;5;243m");
    PFto_ADD("SpringGreen3", "\033[38;5;241m");
    PFto_ADD("SpringGreen4", "\033[38;5;238m");
    PFto_ADD("SteelBlue", "\033[38;5;244m");
    PFto_ADD("SteelBlue1", "\033[38;5;249m");
    PFto_ADD("SteelBlue2", "\033[38;5;248m");
    PFto_ADD("SteelBlue3", "\033[38;5;246m");
    PFto_ADD("SteelBlue4", "\033[38;5;241m");
    PFto_ADD("Tan", "\033[38;5;249m");
    PFto_ADD("Tan1", "\033[38;5;248m");
    PFto_ADD("Tan2", "\033[38;5;247m");
    PFto_ADD("Tan3", "\033[38;5;245m");
    PFto_ADD("Tan4", "\033[38;5;240m");
    PFto_ADD("Thistle", "\033[38;5;252m");
    PFto_ADD("Thistle1", "\033[38;5;255m");
    PFto_ADD("Thistle2", "\033[38;5;254m");
    PFto_ADD("Thistle3", "\033[38;5;251m");
    PFto_ADD("Thistle4", "\033[38;5;245m");
    PFto_ADD("Tomato", "\033[38;5;245m");
    PFto_ADD("Tomato1", "\033[38;5;245m");
    PFto_ADD("Tomato2", "\033[38;5;244m");
    PFto_ADD("Tomato3", "\033[38;5;243m");
    PFto_ADD("Tomato4", "\033[38;5;239m");
    PFto_ADD("Turquoise", "\033[38;5;248m");
    PFto_ADD("Turquoise1", "\033[38;5;248m");
    PFto_ADD("Turquoise2", "\033[38;5;247m");
    PFto_ADD("Turquoise3", "\033[38;5;245m");
    PFto_ADD("Turquoise4", "\033[38;5;240m");
    PFto_ADD("UNDERLINE", "\033[4m");
    PFto_ADD("Violet", "\033[38;5;251m");
    PFto_ADD("VioletRed", "\033[38;5;244m");
    PFto_ADD("VioletRed1", "\033[38;5;247m");
    PFto_ADD("VioletRed2", "\033[38;5;246m");
    PFto_ADD("VioletRed3", "\033[38;5;244m");
    PFto_ADD("VioletRed4", "\033[38;5;240m");
    PFto_ADD("WHITE", "\033[38;5;255m");
    PFto_ADD("Wheat", "\033[38;5;253m");
    PFto_ADD("Wheat1", "\033[38;5;254m");
    PFto_ADD("Wheat2", "\033[38;5;252m");
    PFto_ADD("Wheat3", "\033[38;5;249m");
    PFto_ADD("Wheat4", "\033[38;5;243m");
    PFto_ADD("White", "\033[38;5;255m");
    PFto_ADD("WhiteSmoke", "\033[38;5;255m");
    PFto_ADD("YELLOW", "\033[38;5;251m");
    PFto_ADD("Yellow", "\033[38;5;248m");
    PFto_ADD("Yellow1", "\033[38;5;248m");
    PFto_ADD("Yellow2", "\033[38;5;247m");
    PFto_ADD("Yellow3", "\033[38;5;245m");
    PFto_ADD("Yellow4", "\033[38;5;240m");
    PFto_ADD("YellowGreen", "\033[38;5;245m");

    log_info("Pinkfish to Greyscale conversion data loaded.");
}

void I3_loadPinkfishToNull(void)
{
    log_info("Initializing Pinkfish to NULL conversion table...");
    stringmap_destroy(pinkfish_to_null_db); // Free anything present
    pinkfish_to_null_db = stringmap_init(); // Setup a new empty structure

    log_info("Loading Pinkfish to NULL conversion data...");

#undef PFto_ADD
#define PFto_ADD(key, value)                                                                                           \
    do                                                                                                                 \
    {                                                                                                                  \
        stringmap_add(pinkfish_to_null_db, (key), (void *)(value), strlen((char *)(value)));                           \
        pinkfish_to_null_count++;                                                                                      \
    } while (0)

    PFto_ADD("AliceBlue", "");
    PFto_ADD("AntiqueWhite", "");
    PFto_ADD("AntiqueWhite1", "");
    PFto_ADD("AntiqueWhite2", "");
    PFto_ADD("AntiqueWhite3", "");
    PFto_ADD("AntiqueWhite4", "");
    PFto_ADD("Aquamarine", "");
    PFto_ADD("Aquamarine1", "");
    PFto_ADD("Aquamarine2", "");
    PFto_ADD("Aquamarine3", "");
    PFto_ADD("Aquamarine4", "");
    PFto_ADD("Azure", "");
    PFto_ADD("Azure1", "");
    PFto_ADD("Azure2", "");
    PFto_ADD("Azure3", "");
    PFto_ADD("Azure4", "");
    PFto_ADD("B000", "");
    PFto_ADD("B001", "");
    PFto_ADD("B002", "");
    PFto_ADD("B003", "");
    PFto_ADD("B004", "");
    PFto_ADD("B005", "");
    PFto_ADD("B010", "");
    PFto_ADD("B011", "");
    PFto_ADD("B012", "");
    PFto_ADD("B013", "");
    PFto_ADD("B014", "");
    PFto_ADD("B015", "");
    PFto_ADD("B020", "");
    PFto_ADD("B021", "");
    PFto_ADD("B022", "");
    PFto_ADD("B023", "");
    PFto_ADD("B024", "");
    PFto_ADD("B025", "");
    PFto_ADD("B030", "");
    PFto_ADD("B031", "");
    PFto_ADD("B032", "");
    PFto_ADD("B033", "");
    PFto_ADD("B034", "");
    PFto_ADD("B035", "");
    PFto_ADD("B040", "");
    PFto_ADD("B041", "");
    PFto_ADD("B042", "");
    PFto_ADD("B043", "");
    PFto_ADD("B044", "");
    PFto_ADD("B045", "");
    PFto_ADD("B050", "");
    PFto_ADD("B051", "");
    PFto_ADD("B052", "");
    PFto_ADD("B053", "");
    PFto_ADD("B054", "");
    PFto_ADD("B055", "");
    PFto_ADD("B100", "");
    PFto_ADD("B101", "");
    PFto_ADD("B102", "");
    PFto_ADD("B103", "");
    PFto_ADD("B104", "");
    PFto_ADD("B105", "");
    PFto_ADD("B110", "");
    PFto_ADD("B111", "");
    PFto_ADD("B112", "");
    PFto_ADD("B113", "");
    PFto_ADD("B114", "");
    PFto_ADD("B115", "");
    PFto_ADD("B120", "");
    PFto_ADD("B121", "");
    PFto_ADD("B122", "");
    PFto_ADD("B123", "");
    PFto_ADD("B124", "");
    PFto_ADD("B125", "");
    PFto_ADD("B130", "");
    PFto_ADD("B131", "");
    PFto_ADD("B132", "");
    PFto_ADD("B133", "");
    PFto_ADD("B134", "");
    PFto_ADD("B135", "");
    PFto_ADD("B140", "");
    PFto_ADD("B141", "");
    PFto_ADD("B142", "");
    PFto_ADD("B143", "");
    PFto_ADD("B144", "");
    PFto_ADD("B145", "");
    PFto_ADD("B150", "");
    PFto_ADD("B151", "");
    PFto_ADD("B152", "");
    PFto_ADD("B153", "");
    PFto_ADD("B154", "");
    PFto_ADD("B155", "");
    PFto_ADD("B200", "");
    PFto_ADD("B201", "");
    PFto_ADD("B202", "");
    PFto_ADD("B203", "");
    PFto_ADD("B204", "");
    PFto_ADD("B205", "");
    PFto_ADD("B210", "");
    PFto_ADD("B211", "");
    PFto_ADD("B212", "");
    PFto_ADD("B213", "");
    PFto_ADD("B214", "");
    PFto_ADD("B215", "");
    PFto_ADD("B220", "");
    PFto_ADD("B221", "");
    PFto_ADD("B222", "");
    PFto_ADD("B223", "");
    PFto_ADD("B224", "");
    PFto_ADD("B225", "");
    PFto_ADD("B230", "");
    PFto_ADD("B231", "");
    PFto_ADD("B232", "");
    PFto_ADD("B233", "");
    PFto_ADD("B234", "");
    PFto_ADD("B235", "");
    PFto_ADD("B240", "");
    PFto_ADD("B241", "");
    PFto_ADD("B242", "");
    PFto_ADD("B243", "");
    PFto_ADD("B244", "");
    PFto_ADD("B245", "");
    PFto_ADD("B250", "");
    PFto_ADD("B251", "");
    PFto_ADD("B252", "");
    PFto_ADD("B253", "");
    PFto_ADD("B254", "");
    PFto_ADD("B255", "");
    PFto_ADD("B300", "");
    PFto_ADD("B301", "");
    PFto_ADD("B302", "");
    PFto_ADD("B303", "");
    PFto_ADD("B304", "");
    PFto_ADD("B305", "");
    PFto_ADD("B310", "");
    PFto_ADD("B311", "");
    PFto_ADD("B312", "");
    PFto_ADD("B313", "");
    PFto_ADD("B314", "");
    PFto_ADD("B315", "");
    PFto_ADD("B320", "");
    PFto_ADD("B321", "");
    PFto_ADD("B322", "");
    PFto_ADD("B323", "");
    PFto_ADD("B324", "");
    PFto_ADD("B325", "");
    PFto_ADD("B330", "");
    PFto_ADD("B331", "");
    PFto_ADD("B332", "");
    PFto_ADD("B333", "");
    PFto_ADD("B334", "");
    PFto_ADD("B335", "");
    PFto_ADD("B340", "");
    PFto_ADD("B341", "");
    PFto_ADD("B342", "");
    PFto_ADD("B343", "");
    PFto_ADD("B344", "");
    PFto_ADD("B345", "");
    PFto_ADD("B350", "");
    PFto_ADD("B351", "");
    PFto_ADD("B352", "");
    PFto_ADD("B353", "");
    PFto_ADD("B354", "");
    PFto_ADD("B355", "");
    PFto_ADD("B400", "");
    PFto_ADD("B401", "");
    PFto_ADD("B402", "");
    PFto_ADD("B403", "");
    PFto_ADD("B404", "");
    PFto_ADD("B405", "");
    PFto_ADD("B410", "");
    PFto_ADD("B411", "");
    PFto_ADD("B412", "");
    PFto_ADD("B413", "");
    PFto_ADD("B414", "");
    PFto_ADD("B415", "");
    PFto_ADD("B420", "");
    PFto_ADD("B421", "");
    PFto_ADD("B422", "");
    PFto_ADD("B423", "");
    PFto_ADD("B424", "");
    PFto_ADD("B425", "");
    PFto_ADD("B430", "");
    PFto_ADD("B431", "");
    PFto_ADD("B432", "");
    PFto_ADD("B433", "");
    PFto_ADD("B434", "");
    PFto_ADD("B435", "");
    PFto_ADD("B440", "");
    PFto_ADD("B441", "");
    PFto_ADD("B442", "");
    PFto_ADD("B443", "");
    PFto_ADD("B444", "");
    PFto_ADD("B445", "");
    PFto_ADD("B450", "");
    PFto_ADD("B451", "");
    PFto_ADD("B452", "");
    PFto_ADD("B453", "");
    PFto_ADD("B454", "");
    PFto_ADD("B455", "");
    PFto_ADD("B500", "");
    PFto_ADD("B501", "");
    PFto_ADD("B502", "");
    PFto_ADD("B503", "");
    PFto_ADD("B504", "");
    PFto_ADD("B505", "");
    PFto_ADD("B510", "");
    PFto_ADD("B511", "");
    PFto_ADD("B512", "");
    PFto_ADD("B513", "");
    PFto_ADD("B514", "");
    PFto_ADD("B515", "");
    PFto_ADD("B520", "");
    PFto_ADD("B521", "");
    PFto_ADD("B522", "");
    PFto_ADD("B523", "");
    PFto_ADD("B524", "");
    PFto_ADD("B525", "");
    PFto_ADD("B530", "");
    PFto_ADD("B531", "");
    PFto_ADD("B532", "");
    PFto_ADD("B533", "");
    PFto_ADD("B534", "");
    PFto_ADD("B535", "");
    PFto_ADD("B540", "");
    PFto_ADD("B541", "");
    PFto_ADD("B542", "");
    PFto_ADD("B543", "");
    PFto_ADD("B544", "");
    PFto_ADD("B545", "");
    PFto_ADD("B550", "");
    PFto_ADD("B551", "");
    PFto_ADD("B552", "");
    PFto_ADD("B553", "");
    PFto_ADD("B554", "");
    PFto_ADD("B555", "");
    PFto_ADD("BG00", "");
    PFto_ADD("BG01", "");
    PFto_ADD("BG02", "");
    PFto_ADD("BG03", "");
    PFto_ADD("BG04", "");
    PFto_ADD("BG05", "");
    PFto_ADD("BG06", "");
    PFto_ADD("BG07", "");
    PFto_ADD("BG08", "");
    PFto_ADD("BG09", "");
    PFto_ADD("BG10", "");
    PFto_ADD("BG11", "");
    PFto_ADD("BG12", "");
    PFto_ADD("BG13", "");
    PFto_ADD("BG14", "");
    PFto_ADD("BG15", "");
    PFto_ADD("BG16", "");
    PFto_ADD("BG17", "");
    PFto_ADD("BG18", "");
    PFto_ADD("BG19", "");
    PFto_ADD("BG20", "");
    PFto_ADD("BG21", "");
    PFto_ADD("BG22", "");
    PFto_ADD("BG23", "");
    PFto_ADD("BG24", "");
    PFto_ADD("BG25", "");
    PFto_ADD("BLACK", "");
    PFto_ADD("BLUE", "");
    PFto_ADD("BOLD", "");
    PFto_ADD("B_AliceBlue", "");
    PFto_ADD("B_AntiqueWhite", "");
    PFto_ADD("B_AntiqueWhite1", "");
    PFto_ADD("B_AntiqueWhite2", "");
    PFto_ADD("B_AntiqueWhite3", "");
    PFto_ADD("B_AntiqueWhite4", "");
    PFto_ADD("B_Aquamarine", "");
    PFto_ADD("B_Aquamarine1", "");
    PFto_ADD("B_Aquamarine2", "");
    PFto_ADD("B_Aquamarine3", "");
    PFto_ADD("B_Aquamarine4", "");
    PFto_ADD("B_Azure", "");
    PFto_ADD("B_Azure1", "");
    PFto_ADD("B_Azure2", "");
    PFto_ADD("B_Azure3", "");
    PFto_ADD("B_Azure4", "");
    PFto_ADD("B_BLACK", "");
    PFto_ADD("B_BLUE", "");
    PFto_ADD("B_Beige", "");
    PFto_ADD("B_Bisque", "");
    PFto_ADD("B_Bisque1", "");
    PFto_ADD("B_Bisque2", "");
    PFto_ADD("B_Bisque3", "");
    PFto_ADD("B_Bisque4", "");
    PFto_ADD("B_Black", "");
    PFto_ADD("B_BlanchedAlmond", "");
    PFto_ADD("B_Blue", "");
    PFto_ADD("B_Blue1", "");
    PFto_ADD("B_Blue2", "");
    PFto_ADD("B_Blue3", "");
    PFto_ADD("B_Blue4", "");
    PFto_ADD("B_BlueViolet", "");
    PFto_ADD("B_Brown", "");
    PFto_ADD("B_Brown1", "");
    PFto_ADD("B_Brown2", "");
    PFto_ADD("B_Brown3", "");
    PFto_ADD("B_Brown4", "");
    PFto_ADD("B_Burlywood", "");
    PFto_ADD("B_Burlywood1", "");
    PFto_ADD("B_Burlywood2", "");
    PFto_ADD("B_Burlywood3", "");
    PFto_ADD("B_Burlywood4", "");
    PFto_ADD("B_CYAN", "");
    PFto_ADD("B_CadetBlue", "");
    PFto_ADD("B_CadetBlue1", "");
    PFto_ADD("B_CadetBlue2", "");
    PFto_ADD("B_CadetBlue3", "");
    PFto_ADD("B_CadetBlue4", "");
    PFto_ADD("B_Chartreuse", "");
    PFto_ADD("B_Chartreuse1", "");
    PFto_ADD("B_Chartreuse2", "");
    PFto_ADD("B_Chartreuse3", "");
    PFto_ADD("B_Chartreuse4", "");
    PFto_ADD("B_Chocolate", "");
    PFto_ADD("B_Chocolate1", "");
    PFto_ADD("B_Chocolate2", "");
    PFto_ADD("B_Chocolate3", "");
    PFto_ADD("B_Chocolate4", "");
    PFto_ADD("B_Coral", "");
    PFto_ADD("B_Coral1", "");
    PFto_ADD("B_Coral2", "");
    PFto_ADD("B_Coral3", "");
    PFto_ADD("B_Coral4", "");
    PFto_ADD("B_CornflowerBlue", "");
    PFto_ADD("B_Cornsilk", "");
    PFto_ADD("B_Cornsilk1", "");
    PFto_ADD("B_Cornsilk2", "");
    PFto_ADD("B_Cornsilk3", "");
    PFto_ADD("B_Cornsilk4", "");
    PFto_ADD("B_Cyan", "");
    PFto_ADD("B_Cyan1", "");
    PFto_ADD("B_Cyan2", "");
    PFto_ADD("B_Cyan3", "");
    PFto_ADD("B_Cyan4", "");
    PFto_ADD("B_DARKGREY", "");
    PFto_ADD("B_DarkBlue", "");
    PFto_ADD("B_DarkCyan", "");
    PFto_ADD("B_DarkGoldenrod", "");
    PFto_ADD("B_DarkGoldenrod1", "");
    PFto_ADD("B_DarkGoldenrod2", "");
    PFto_ADD("B_DarkGoldenrod3", "");
    PFto_ADD("B_DarkGoldenrod4", "");
    PFto_ADD("B_DarkGray", "");
    PFto_ADD("B_DarkGreen", "");
    PFto_ADD("B_DarkGrey", "");
    PFto_ADD("B_DarkKhaki", "");
    PFto_ADD("B_DarkMagenta", "");
    PFto_ADD("B_DarkOliveGreen", "");
    PFto_ADD("B_DarkOliveGreen1", "");
    PFto_ADD("B_DarkOliveGreen2", "");
    PFto_ADD("B_DarkOliveGreen3", "");
    PFto_ADD("B_DarkOliveGreen4", "");
    PFto_ADD("B_DarkOrange", "");
    PFto_ADD("B_DarkOrange1", "");
    PFto_ADD("B_DarkOrange2", "");
    PFto_ADD("B_DarkOrange3", "");
    PFto_ADD("B_DarkOrange4", "");
    PFto_ADD("B_DarkOrchid", "");
    PFto_ADD("B_DarkOrchid1", "");
    PFto_ADD("B_DarkOrchid2", "");
    PFto_ADD("B_DarkOrchid3", "");
    PFto_ADD("B_DarkOrchid4", "");
    PFto_ADD("B_DarkRed", "");
    PFto_ADD("B_DarkSalmon", "");
    PFto_ADD("B_DarkSeaGreen", "");
    PFto_ADD("B_DarkSeaGreen1", "");
    PFto_ADD("B_DarkSeaGreen2", "");
    PFto_ADD("B_DarkSeaGreen3", "");
    PFto_ADD("B_DarkSeaGreen4", "");
    PFto_ADD("B_DarkSlateBlue", "");
    PFto_ADD("B_DarkSlateGray", "");
    PFto_ADD("B_DarkSlateGray1", "");
    PFto_ADD("B_DarkSlateGray2", "");
    PFto_ADD("B_DarkSlateGray3", "");
    PFto_ADD("B_DarkSlateGray4", "");
    PFto_ADD("B_DarkSlateGrey", "");
    PFto_ADD("B_DarkTurquoise", "");
    PFto_ADD("B_DarkViolet", "");
    PFto_ADD("B_DeepPink", "");
    PFto_ADD("B_DeepPink1", "");
    PFto_ADD("B_DeepPink2", "");
    PFto_ADD("B_DeepPink3", "");
    PFto_ADD("B_DeepPink4", "");
    PFto_ADD("B_DeepSkyBlue", "");
    PFto_ADD("B_DeepSkyBlue1", "");
    PFto_ADD("B_DeepSkyBlue2", "");
    PFto_ADD("B_DeepSkyBlue3", "");
    PFto_ADD("B_DeepSkyBlue4", "");
    PFto_ADD("B_DimGray", "");
    PFto_ADD("B_DimGrey", "");
    PFto_ADD("B_DodgerBlue", "");
    PFto_ADD("B_DodgerBlue1", "");
    PFto_ADD("B_DodgerBlue2", "");
    PFto_ADD("B_DodgerBlue3", "");
    PFto_ADD("B_DodgerBlue4", "");
    PFto_ADD("B_Firebrick", "");
    PFto_ADD("B_Firebrick1", "");
    PFto_ADD("B_Firebrick2", "");
    PFto_ADD("B_Firebrick3", "");
    PFto_ADD("B_Firebrick4", "");
    PFto_ADD("B_FloralWhite", "");
    PFto_ADD("B_ForestGreen", "");
    PFto_ADD("B_GREEN", "");
    PFto_ADD("B_GREY", "");
    PFto_ADD("B_Gainsboro", "");
    PFto_ADD("B_GhostWhite", "");
    PFto_ADD("B_Gold", "");
    PFto_ADD("B_Gold1", "");
    PFto_ADD("B_Gold2", "");
    PFto_ADD("B_Gold3", "");
    PFto_ADD("B_Gold4", "");
    PFto_ADD("B_Goldenrod", "");
    PFto_ADD("B_Goldenrod1", "");
    PFto_ADD("B_Goldenrod2", "");
    PFto_ADD("B_Goldenrod3", "");
    PFto_ADD("B_Goldenrod4", "");
    PFto_ADD("B_Gray", "");
    PFto_ADD("B_Gray0", "");
    PFto_ADD("B_Gray1", "");
    PFto_ADD("B_Gray10", "");
    PFto_ADD("B_Gray100", "");
    PFto_ADD("B_Gray11", "");
    PFto_ADD("B_Gray12", "");
    PFto_ADD("B_Gray13", "");
    PFto_ADD("B_Gray14", "");
    PFto_ADD("B_Gray15", "");
    PFto_ADD("B_Gray16", "");
    PFto_ADD("B_Gray17", "");
    PFto_ADD("B_Gray18", "");
    PFto_ADD("B_Gray19", "");
    PFto_ADD("B_Gray2", "");
    PFto_ADD("B_Gray20", "");
    PFto_ADD("B_Gray21", "");
    PFto_ADD("B_Gray22", "");
    PFto_ADD("B_Gray23", "");
    PFto_ADD("B_Gray24", "");
    PFto_ADD("B_Gray25", "");
    PFto_ADD("B_Gray26", "");
    PFto_ADD("B_Gray27", "");
    PFto_ADD("B_Gray28", "");
    PFto_ADD("B_Gray29", "");
    PFto_ADD("B_Gray3", "");
    PFto_ADD("B_Gray30", "");
    PFto_ADD("B_Gray31", "");
    PFto_ADD("B_Gray32", "");
    PFto_ADD("B_Gray33", "");
    PFto_ADD("B_Gray34", "");
    PFto_ADD("B_Gray35", "");
    PFto_ADD("B_Gray36", "");
    PFto_ADD("B_Gray37", "");
    PFto_ADD("B_Gray38", "");
    PFto_ADD("B_Gray39", "");
    PFto_ADD("B_Gray4", "");
    PFto_ADD("B_Gray40", "");
    PFto_ADD("B_Gray41", "");
    PFto_ADD("B_Gray42", "");
    PFto_ADD("B_Gray43", "");
    PFto_ADD("B_Gray44", "");
    PFto_ADD("B_Gray45", "");
    PFto_ADD("B_Gray46", "");
    PFto_ADD("B_Gray47", "");
    PFto_ADD("B_Gray48", "");
    PFto_ADD("B_Gray49", "");
    PFto_ADD("B_Gray5", "");
    PFto_ADD("B_Gray50", "");
    PFto_ADD("B_Gray51", "");
    PFto_ADD("B_Gray52", "");
    PFto_ADD("B_Gray53", "");
    PFto_ADD("B_Gray54", "");
    PFto_ADD("B_Gray55", "");
    PFto_ADD("B_Gray56", "");
    PFto_ADD("B_Gray57", "");
    PFto_ADD("B_Gray58", "");
    PFto_ADD("B_Gray59", "");
    PFto_ADD("B_Gray6", "");
    PFto_ADD("B_Gray60", "");
    PFto_ADD("B_Gray61", "");
    PFto_ADD("B_Gray62", "");
    PFto_ADD("B_Gray63", "");
    PFto_ADD("B_Gray64", "");
    PFto_ADD("B_Gray65", "");
    PFto_ADD("B_Gray66", "");
    PFto_ADD("B_Gray67", "");
    PFto_ADD("B_Gray68", "");
    PFto_ADD("B_Gray69", "");
    PFto_ADD("B_Gray7", "");
    PFto_ADD("B_Gray70", "");
    PFto_ADD("B_Gray71", "");
    PFto_ADD("B_Gray72", "");
    PFto_ADD("B_Gray73", "");
    PFto_ADD("B_Gray74", "");
    PFto_ADD("B_Gray75", "");
    PFto_ADD("B_Gray76", "");
    PFto_ADD("B_Gray77", "");
    PFto_ADD("B_Gray78", "");
    PFto_ADD("B_Gray79", "");
    PFto_ADD("B_Gray8", "");
    PFto_ADD("B_Gray80", "");
    PFto_ADD("B_Gray81", "");
    PFto_ADD("B_Gray82", "");
    PFto_ADD("B_Gray83", "");
    PFto_ADD("B_Gray84", "");
    PFto_ADD("B_Gray85", "");
    PFto_ADD("B_Gray86", "");
    PFto_ADD("B_Gray87", "");
    PFto_ADD("B_Gray88", "");
    PFto_ADD("B_Gray89", "");
    PFto_ADD("B_Gray9", "");
    PFto_ADD("B_Gray90", "");
    PFto_ADD("B_Gray91", "");
    PFto_ADD("B_Gray92", "");
    PFto_ADD("B_Gray93", "");
    PFto_ADD("B_Gray94", "");
    PFto_ADD("B_Gray95", "");
    PFto_ADD("B_Gray96", "");
    PFto_ADD("B_Gray97", "");
    PFto_ADD("B_Gray98", "");
    PFto_ADD("B_Gray99", "");
    PFto_ADD("B_Green", "");
    PFto_ADD("B_Green1", "");
    PFto_ADD("B_Green2", "");
    PFto_ADD("B_Green3", "");
    PFto_ADD("B_Green4", "");
    PFto_ADD("B_GreenYellow", "");
    PFto_ADD("B_Grey", "");
    PFto_ADD("B_Grey0", "");
    PFto_ADD("B_Grey1", "");
    PFto_ADD("B_Grey10", "");
    PFto_ADD("B_Grey100", "");
    PFto_ADD("B_Grey11", "");
    PFto_ADD("B_Grey12", "");
    PFto_ADD("B_Grey13", "");
    PFto_ADD("B_Grey14", "");
    PFto_ADD("B_Grey15", "");
    PFto_ADD("B_Grey16", "");
    PFto_ADD("B_Grey17", "");
    PFto_ADD("B_Grey18", "");
    PFto_ADD("B_Grey19", "");
    PFto_ADD("B_Grey2", "");
    PFto_ADD("B_Grey20", "");
    PFto_ADD("B_Grey21", "");
    PFto_ADD("B_Grey22", "");
    PFto_ADD("B_Grey23", "");
    PFto_ADD("B_Grey24", "");
    PFto_ADD("B_Grey25", "");
    PFto_ADD("B_Grey26", "");
    PFto_ADD("B_Grey27", "");
    PFto_ADD("B_Grey28", "");
    PFto_ADD("B_Grey29", "");
    PFto_ADD("B_Grey3", "");
    PFto_ADD("B_Grey30", "");
    PFto_ADD("B_Grey31", "");
    PFto_ADD("B_Grey32", "");
    PFto_ADD("B_Grey33", "");
    PFto_ADD("B_Grey34", "");
    PFto_ADD("B_Grey35", "");
    PFto_ADD("B_Grey36", "");
    PFto_ADD("B_Grey37", "");
    PFto_ADD("B_Grey38", "");
    PFto_ADD("B_Grey39", "");
    PFto_ADD("B_Grey4", "");
    PFto_ADD("B_Grey40", "");
    PFto_ADD("B_Grey41", "");
    PFto_ADD("B_Grey42", "");
    PFto_ADD("B_Grey43", "");
    PFto_ADD("B_Grey44", "");
    PFto_ADD("B_Grey45", "");
    PFto_ADD("B_Grey46", "");
    PFto_ADD("B_Grey47", "");
    PFto_ADD("B_Grey48", "");
    PFto_ADD("B_Grey49", "");
    PFto_ADD("B_Grey5", "");
    PFto_ADD("B_Grey50", "");
    PFto_ADD("B_Grey51", "");
    PFto_ADD("B_Grey52", "");
    PFto_ADD("B_Grey53", "");
    PFto_ADD("B_Grey54", "");
    PFto_ADD("B_Grey55", "");
    PFto_ADD("B_Grey56", "");
    PFto_ADD("B_Grey57", "");
    PFto_ADD("B_Grey58", "");
    PFto_ADD("B_Grey59", "");
    PFto_ADD("B_Grey6", "");
    PFto_ADD("B_Grey60", "");
    PFto_ADD("B_Grey61", "");
    PFto_ADD("B_Grey62", "");
    PFto_ADD("B_Grey63", "");
    PFto_ADD("B_Grey64", "");
    PFto_ADD("B_Grey65", "");
    PFto_ADD("B_Grey66", "");
    PFto_ADD("B_Grey67", "");
    PFto_ADD("B_Grey68", "");
    PFto_ADD("B_Grey69", "");
    PFto_ADD("B_Grey7", "");
    PFto_ADD("B_Grey70", "");
    PFto_ADD("B_Grey71", "");
    PFto_ADD("B_Grey72", "");
    PFto_ADD("B_Grey73", "");
    PFto_ADD("B_Grey74", "");
    PFto_ADD("B_Grey75", "");
    PFto_ADD("B_Grey76", "");
    PFto_ADD("B_Grey77", "");
    PFto_ADD("B_Grey78", "");
    PFto_ADD("B_Grey79", "");
    PFto_ADD("B_Grey8", "");
    PFto_ADD("B_Grey80", "");
    PFto_ADD("B_Grey81", "");
    PFto_ADD("B_Grey82", "");
    PFto_ADD("B_Grey83", "");
    PFto_ADD("B_Grey84", "");
    PFto_ADD("B_Grey85", "");
    PFto_ADD("B_Grey86", "");
    PFto_ADD("B_Grey87", "");
    PFto_ADD("B_Grey88", "");
    PFto_ADD("B_Grey89", "");
    PFto_ADD("B_Grey9", "");
    PFto_ADD("B_Grey90", "");
    PFto_ADD("B_Grey91", "");
    PFto_ADD("B_Grey92", "");
    PFto_ADD("B_Grey93", "");
    PFto_ADD("B_Grey94", "");
    PFto_ADD("B_Grey95", "");
    PFto_ADD("B_Grey96", "");
    PFto_ADD("B_Grey97", "");
    PFto_ADD("B_Grey98", "");
    PFto_ADD("B_Grey99", "");
    PFto_ADD("B_Honeydew", "");
    PFto_ADD("B_Honeydew1", "");
    PFto_ADD("B_Honeydew2", "");
    PFto_ADD("B_Honeydew3", "");
    PFto_ADD("B_Honeydew4", "");
    PFto_ADD("B_HotPink", "");
    PFto_ADD("B_HotPink1", "");
    PFto_ADD("B_HotPink2", "");
    PFto_ADD("B_HotPink3", "");
    PFto_ADD("B_HotPink4", "");
    PFto_ADD("B_IndianRed", "");
    PFto_ADD("B_IndianRed1", "");
    PFto_ADD("B_IndianRed2", "");
    PFto_ADD("B_IndianRed3", "");
    PFto_ADD("B_IndianRed4", "");
    PFto_ADD("B_Ivory", "");
    PFto_ADD("B_Ivory1", "");
    PFto_ADD("B_Ivory2", "");
    PFto_ADD("B_Ivory3", "");
    PFto_ADD("B_Ivory4", "");
    PFto_ADD("B_Khaki", "");
    PFto_ADD("B_Khaki1", "");
    PFto_ADD("B_Khaki2", "");
    PFto_ADD("B_Khaki3", "");
    PFto_ADD("B_Khaki4", "");
    PFto_ADD("B_LIGHTBLUE", "");
    PFto_ADD("B_LIGHTCYAN", "");
    PFto_ADD("B_LIGHTGREEN", "");
    PFto_ADD("B_LIGHTRED", "");
    PFto_ADD("B_Lavender", "");
    PFto_ADD("B_LavenderBlush", "");
    PFto_ADD("B_LavenderBlush1", "");
    PFto_ADD("B_LavenderBlush2", "");
    PFto_ADD("B_LavenderBlush3", "");
    PFto_ADD("B_LavenderBlush4", "");
    PFto_ADD("B_LawnGreen", "");
    PFto_ADD("B_LemonChiffon", "");
    PFto_ADD("B_LemonChiffon1", "");
    PFto_ADD("B_LemonChiffon2", "");
    PFto_ADD("B_LemonChiffon3", "");
    PFto_ADD("B_LemonChiffon4", "");
    PFto_ADD("B_LightBlue", "");
    PFto_ADD("B_LightBlue1", "");
    PFto_ADD("B_LightBlue2", "");
    PFto_ADD("B_LightBlue3", "");
    PFto_ADD("B_LightBlue4", "");
    PFto_ADD("B_LightCoral", "");
    PFto_ADD("B_LightCyan", "");
    PFto_ADD("B_LightCyan1", "");
    PFto_ADD("B_LightCyan2", "");
    PFto_ADD("B_LightCyan3", "");
    PFto_ADD("B_LightCyan4", "");
    PFto_ADD("B_LightGoldenrod", "");
    PFto_ADD("B_LightGoldenrod1", "");
    PFto_ADD("B_LightGoldenrod2", "");
    PFto_ADD("B_LightGoldenrod3", "");
    PFto_ADD("B_LightGoldenrod4", "");
    PFto_ADD("B_LightGoldenrodYellow", "");
    PFto_ADD("B_LightGray", "");
    PFto_ADD("B_LightGreen", "");
    PFto_ADD("B_LightGrey", "");
    PFto_ADD("B_LightPink", "");
    PFto_ADD("B_LightPink1", "");
    PFto_ADD("B_LightPink2", "");
    PFto_ADD("B_LightPink3", "");
    PFto_ADD("B_LightPink4", "");
    PFto_ADD("B_LightSalmon", "");
    PFto_ADD("B_LightSalmon1", "");
    PFto_ADD("B_LightSalmon2", "");
    PFto_ADD("B_LightSalmon3", "");
    PFto_ADD("B_LightSalmon4", "");
    PFto_ADD("B_LightSeaGreen", "");
    PFto_ADD("B_LightSkyBlue", "");
    PFto_ADD("B_LightSkyBlue1", "");
    PFto_ADD("B_LightSkyBlue2", "");
    PFto_ADD("B_LightSkyBlue3", "");
    PFto_ADD("B_LightSkyBlue4", "");
    PFto_ADD("B_LightSlateBlue", "");
    PFto_ADD("B_LightSlateGray", "");
    PFto_ADD("B_LightSlateGrey", "");
    PFto_ADD("B_LightSteelBlue", "");
    PFto_ADD("B_LightSteelBlue1", "");
    PFto_ADD("B_LightSteelBlue2", "");
    PFto_ADD("B_LightSteelBlue3", "");
    PFto_ADD("B_LightSteelBlue4", "");
    PFto_ADD("B_LightYellow", "");
    PFto_ADD("B_LightYellow1", "");
    PFto_ADD("B_LightYellow2", "");
    PFto_ADD("B_LightYellow3", "");
    PFto_ADD("B_LightYellow4", "");
    PFto_ADD("B_LimeGreen", "");
    PFto_ADD("B_Linen", "");
    PFto_ADD("B_MAGENTA", "");
    PFto_ADD("B_Magenta", "");
    PFto_ADD("B_Magenta1", "");
    PFto_ADD("B_Magenta2", "");
    PFto_ADD("B_Magenta3", "");
    PFto_ADD("B_Magenta4", "");
    PFto_ADD("B_Maroon", "");
    PFto_ADD("B_Maroon1", "");
    PFto_ADD("B_Maroon2", "");
    PFto_ADD("B_Maroon3", "");
    PFto_ADD("B_Maroon4", "");
    PFto_ADD("B_MediumAquamarine", "");
    PFto_ADD("B_MediumBlue", "");
    PFto_ADD("B_MediumOrchid", "");
    PFto_ADD("B_MediumOrchid1", "");
    PFto_ADD("B_MediumOrchid2", "");
    PFto_ADD("B_MediumOrchid3", "");
    PFto_ADD("B_MediumOrchid4", "");
    PFto_ADD("B_MediumPurple", "");
    PFto_ADD("B_MediumPurple1", "");
    PFto_ADD("B_MediumPurple2", "");
    PFto_ADD("B_MediumPurple3", "");
    PFto_ADD("B_MediumPurple4", "");
    PFto_ADD("B_MediumSeaGreen", "");
    PFto_ADD("B_MediumSlateBlue", "");
    PFto_ADD("B_MediumSpringGreen", "");
    PFto_ADD("B_MediumTurquoise", "");
    PFto_ADD("B_MediumVioletRed", "");
    PFto_ADD("B_MidnightBlue", "");
    PFto_ADD("B_MintCream", "");
    PFto_ADD("B_MistyRose", "");
    PFto_ADD("B_MistyRose1", "");
    PFto_ADD("B_MistyRose2", "");
    PFto_ADD("B_MistyRose3", "");
    PFto_ADD("B_MistyRose4", "");
    PFto_ADD("B_Moccasin", "");
    PFto_ADD("B_NavajoWhite", "");
    PFto_ADD("B_NavajoWhite1", "");
    PFto_ADD("B_NavajoWhite2", "");
    PFto_ADD("B_NavajoWhite3", "");
    PFto_ADD("B_NavajoWhite4", "");
    PFto_ADD("B_Navy", "");
    PFto_ADD("B_NavyBlue", "");
    PFto_ADD("B_ORANGE", "");
    PFto_ADD("B_OldLace", "");
    PFto_ADD("B_OliveDrab", "");
    PFto_ADD("B_OliveDrab1", "");
    PFto_ADD("B_OliveDrab2", "");
    PFto_ADD("B_OliveDrab3", "");
    PFto_ADD("B_OliveDrab4", "");
    PFto_ADD("B_Orange", "");
    PFto_ADD("B_Orange1", "");
    PFto_ADD("B_Orange2", "");
    PFto_ADD("B_Orange3", "");
    PFto_ADD("B_Orange4", "");
    PFto_ADD("B_OrangeRed", "");
    PFto_ADD("B_OrangeRed1", "");
    PFto_ADD("B_OrangeRed2", "");
    PFto_ADD("B_OrangeRed3", "");
    PFto_ADD("B_OrangeRed4", "");
    PFto_ADD("B_Orchid", "");
    PFto_ADD("B_Orchid1", "");
    PFto_ADD("B_Orchid2", "");
    PFto_ADD("B_Orchid3", "");
    PFto_ADD("B_Orchid4", "");
    PFto_ADD("B_PINK", "");
    PFto_ADD("B_PaleGoldenrod", "");
    PFto_ADD("B_PaleGreen", "");
    PFto_ADD("B_PaleGreen1", "");
    PFto_ADD("B_PaleGreen2", "");
    PFto_ADD("B_PaleGreen3", "");
    PFto_ADD("B_PaleGreen4", "");
    PFto_ADD("B_PaleTurquoise", "");
    PFto_ADD("B_PaleTurquoise1", "");
    PFto_ADD("B_PaleTurquoise2", "");
    PFto_ADD("B_PaleTurquoise3", "");
    PFto_ADD("B_PaleTurquoise4", "");
    PFto_ADD("B_PaleVioletRed", "");
    PFto_ADD("B_PaleVioletRed1", "");
    PFto_ADD("B_PaleVioletRed2", "");
    PFto_ADD("B_PaleVioletRed3", "");
    PFto_ADD("B_PaleVioletRed4", "");
    PFto_ADD("B_PapayaWhip", "");
    PFto_ADD("B_PeachPuff", "");
    PFto_ADD("B_PeachPuff1", "");
    PFto_ADD("B_PeachPuff2", "");
    PFto_ADD("B_PeachPuff3", "");
    PFto_ADD("B_PeachPuff4", "");
    PFto_ADD("B_Peru", "");
    PFto_ADD("B_Pink", "");
    PFto_ADD("B_Pink1", "");
    PFto_ADD("B_Pink2", "");
    PFto_ADD("B_Pink3", "");
    PFto_ADD("B_Pink4", "");
    PFto_ADD("B_Plum", "");
    PFto_ADD("B_Plum1", "");
    PFto_ADD("B_Plum2", "");
    PFto_ADD("B_Plum3", "");
    PFto_ADD("B_Plum4", "");
    PFto_ADD("B_PowderBlue", "");
    PFto_ADD("B_Purple", "");
    PFto_ADD("B_Purple1", "");
    PFto_ADD("B_Purple2", "");
    PFto_ADD("B_Purple3", "");
    PFto_ADD("B_Purple4", "");
    PFto_ADD("B_RED", "");
    PFto_ADD("B_Red", "");
    PFto_ADD("B_Red1", "");
    PFto_ADD("B_Red2", "");
    PFto_ADD("B_Red3", "");
    PFto_ADD("B_Red4", "");
    PFto_ADD("B_RosyBrown", "");
    PFto_ADD("B_RosyBrown1", "");
    PFto_ADD("B_RosyBrown2", "");
    PFto_ADD("B_RosyBrown3", "");
    PFto_ADD("B_RosyBrown4", "");
    PFto_ADD("B_RoyalBlue", "");
    PFto_ADD("B_RoyalBlue1", "");
    PFto_ADD("B_RoyalBlue2", "");
    PFto_ADD("B_RoyalBlue3", "");
    PFto_ADD("B_RoyalBlue4", "");
    PFto_ADD("B_SaddleBrown", "");
    PFto_ADD("B_Salmon", "");
    PFto_ADD("B_Salmon1", "");
    PFto_ADD("B_Salmon2", "");
    PFto_ADD("B_Salmon3", "");
    PFto_ADD("B_Salmon4", "");
    PFto_ADD("B_SandyBrown", "");
    PFto_ADD("B_SeaGreen", "");
    PFto_ADD("B_SeaGreen1", "");
    PFto_ADD("B_SeaGreen2", "");
    PFto_ADD("B_SeaGreen3", "");
    PFto_ADD("B_SeaGreen4", "");
    PFto_ADD("B_Seashell", "");
    PFto_ADD("B_Seashell1", "");
    PFto_ADD("B_Seashell2", "");
    PFto_ADD("B_Seashell3", "");
    PFto_ADD("B_Seashell4", "");
    PFto_ADD("B_Sienna", "");
    PFto_ADD("B_Sienna1", "");
    PFto_ADD("B_Sienna2", "");
    PFto_ADD("B_Sienna3", "");
    PFto_ADD("B_Sienna4", "");
    PFto_ADD("B_SkyBlue", "");
    PFto_ADD("B_SkyBlue1", "");
    PFto_ADD("B_SkyBlue2", "");
    PFto_ADD("B_SkyBlue3", "");
    PFto_ADD("B_SkyBlue4", "");
    PFto_ADD("B_SlateBlue", "");
    PFto_ADD("B_SlateBlue1", "");
    PFto_ADD("B_SlateBlue2", "");
    PFto_ADD("B_SlateBlue3", "");
    PFto_ADD("B_SlateBlue4", "");
    PFto_ADD("B_SlateGray", "");
    PFto_ADD("B_SlateGray1", "");
    PFto_ADD("B_SlateGray2", "");
    PFto_ADD("B_SlateGray3", "");
    PFto_ADD("B_SlateGray4", "");
    PFto_ADD("B_SlateGrey", "");
    PFto_ADD("B_Snow", "");
    PFto_ADD("B_Snow1", "");
    PFto_ADD("B_Snow2", "");
    PFto_ADD("B_Snow3", "");
    PFto_ADD("B_Snow4", "");
    PFto_ADD("B_SpringGreen", "");
    PFto_ADD("B_SpringGreen1", "");
    PFto_ADD("B_SpringGreen2", "");
    PFto_ADD("B_SpringGreen3", "");
    PFto_ADD("B_SpringGreen4", "");
    PFto_ADD("B_SteelBlue", "");
    PFto_ADD("B_SteelBlue1", "");
    PFto_ADD("B_SteelBlue2", "");
    PFto_ADD("B_SteelBlue3", "");
    PFto_ADD("B_SteelBlue4", "");
    PFto_ADD("B_Tan", "");
    PFto_ADD("B_Tan1", "");
    PFto_ADD("B_Tan2", "");
    PFto_ADD("B_Tan3", "");
    PFto_ADD("B_Tan4", "");
    PFto_ADD("B_Thistle", "");
    PFto_ADD("B_Thistle1", "");
    PFto_ADD("B_Thistle2", "");
    PFto_ADD("B_Thistle3", "");
    PFto_ADD("B_Thistle4", "");
    PFto_ADD("B_Tomato", "");
    PFto_ADD("B_Tomato1", "");
    PFto_ADD("B_Tomato2", "");
    PFto_ADD("B_Tomato3", "");
    PFto_ADD("B_Tomato4", "");
    PFto_ADD("B_Turquoise", "");
    PFto_ADD("B_Turquoise1", "");
    PFto_ADD("B_Turquoise2", "");
    PFto_ADD("B_Turquoise3", "");
    PFto_ADD("B_Turquoise4", "");
    PFto_ADD("B_Violet", "");
    PFto_ADD("B_VioletRed", "");
    PFto_ADD("B_VioletRed1", "");
    PFto_ADD("B_VioletRed2", "");
    PFto_ADD("B_VioletRed3", "");
    PFto_ADD("B_VioletRed4", "");
    PFto_ADD("B_WHITE", "");
    PFto_ADD("B_Wheat", "");
    PFto_ADD("B_Wheat1", "");
    PFto_ADD("B_Wheat2", "");
    PFto_ADD("B_Wheat3", "");
    PFto_ADD("B_Wheat4", "");
    PFto_ADD("B_White", "");
    PFto_ADD("B_WhiteSmoke", "");
    PFto_ADD("B_YELLOW", "");
    PFto_ADD("B_Yellow", "");
    PFto_ADD("B_Yellow1", "");
    PFto_ADD("B_Yellow2", "");
    PFto_ADD("B_Yellow3", "");
    PFto_ADD("B_Yellow4", "");
    PFto_ADD("B_YellowGreen", "");
    PFto_ADD("Beige", "");
    PFto_ADD("Bisque", "");
    PFto_ADD("Bisque1", "");
    PFto_ADD("Bisque2", "");
    PFto_ADD("Bisque3", "");
    PFto_ADD("Bisque4", "");
    PFto_ADD("Black", "");
    PFto_ADD("BlanchedAlmond", "");
    PFto_ADD("Blue", "");
    PFto_ADD("Blue1", "");
    PFto_ADD("Blue2", "");
    PFto_ADD("Blue3", "");
    PFto_ADD("Blue4", "");
    PFto_ADD("BlueViolet", "");
    PFto_ADD("Brown", "");
    PFto_ADD("Brown1", "");
    PFto_ADD("Brown2", "");
    PFto_ADD("Brown3", "");
    PFto_ADD("Brown4", "");
    PFto_ADD("Burlywood", "");
    PFto_ADD("Burlywood1", "");
    PFto_ADD("Burlywood2", "");
    PFto_ADD("Burlywood3", "");
    PFto_ADD("Burlywood4", "");
    PFto_ADD("CLEARLINE", "");
    PFto_ADD("CURS_DOWN", "");
    PFto_ADD("CURS_LEFT", "");
    PFto_ADD("CURS_RIGHT", "");
    PFto_ADD("CURS_UP", "");
    PFto_ADD("CYAN", "");
    PFto_ADD("CadetBlue", "");
    PFto_ADD("CadetBlue1", "");
    PFto_ADD("CadetBlue2", "");
    PFto_ADD("CadetBlue3", "");
    PFto_ADD("CadetBlue4", "");
    PFto_ADD("Chartreuse", "");
    PFto_ADD("Chartreuse1", "");
    PFto_ADD("Chartreuse2", "");
    PFto_ADD("Chartreuse3", "");
    PFto_ADD("Chartreuse4", "");
    PFto_ADD("Chocolate", "");
    PFto_ADD("Chocolate1", "");
    PFto_ADD("Chocolate2", "");
    PFto_ADD("Chocolate3", "");
    PFto_ADD("Chocolate4", "");
    PFto_ADD("Coral", "");
    PFto_ADD("Coral1", "");
    PFto_ADD("Coral2", "");
    PFto_ADD("Coral3", "");
    PFto_ADD("Coral4", "");
    PFto_ADD("CornflowerBlue", "");
    PFto_ADD("Cornsilk", "");
    PFto_ADD("Cornsilk1", "");
    PFto_ADD("Cornsilk2", "");
    PFto_ADD("Cornsilk3", "");
    PFto_ADD("Cornsilk4", "");
    PFto_ADD("Cyan", "");
    PFto_ADD("Cyan1", "");
    PFto_ADD("Cyan2", "");
    PFto_ADD("Cyan3", "");
    PFto_ADD("Cyan4", "");
    PFto_ADD("DARKGREY", "");
    PFto_ADD("DarkBlue", "");
    PFto_ADD("DarkCyan", "");
    PFto_ADD("DarkGoldenrod", "");
    PFto_ADD("DarkGoldenrod1", "");
    PFto_ADD("DarkGoldenrod2", "");
    PFto_ADD("DarkGoldenrod3", "");
    PFto_ADD("DarkGoldenrod4", "");
    PFto_ADD("DarkGray", "");
    PFto_ADD("DarkGreen", "");
    PFto_ADD("DarkGrey", "");
    PFto_ADD("DarkKhaki", "");
    PFto_ADD("DarkMagenta", "");
    PFto_ADD("DarkOliveGreen", "");
    PFto_ADD("DarkOliveGreen1", "");
    PFto_ADD("DarkOliveGreen2", "");
    PFto_ADD("DarkOliveGreen3", "");
    PFto_ADD("DarkOliveGreen4", "");
    PFto_ADD("DarkOrange", "");
    PFto_ADD("DarkOrange1", "");
    PFto_ADD("DarkOrange2", "");
    PFto_ADD("DarkOrange3", "");
    PFto_ADD("DarkOrange4", "");
    PFto_ADD("DarkOrchid", "");
    PFto_ADD("DarkOrchid1", "");
    PFto_ADD("DarkOrchid2", "");
    PFto_ADD("DarkOrchid3", "");
    PFto_ADD("DarkOrchid4", "");
    PFto_ADD("DarkRed", "");
    PFto_ADD("DarkSalmon", "");
    PFto_ADD("DarkSeaGreen", "");
    PFto_ADD("DarkSeaGreen1", "");
    PFto_ADD("DarkSeaGreen2", "");
    PFto_ADD("DarkSeaGreen3", "");
    PFto_ADD("DarkSeaGreen4", "");
    PFto_ADD("DarkSlateBlue", "");
    PFto_ADD("DarkSlateGray", "");
    PFto_ADD("DarkSlateGray1", "");
    PFto_ADD("DarkSlateGray2", "");
    PFto_ADD("DarkSlateGray3", "");
    PFto_ADD("DarkSlateGray4", "");
    PFto_ADD("DarkSlateGrey", "");
    PFto_ADD("DarkTurquoise", "");
    PFto_ADD("DarkViolet", "");
    PFto_ADD("DeepPink", "");
    PFto_ADD("DeepPink1", "");
    PFto_ADD("DeepPink2", "");
    PFto_ADD("DeepPink3", "");
    PFto_ADD("DeepPink4", "");
    PFto_ADD("DeepSkyBlue", "");
    PFto_ADD("DeepSkyBlue1", "");
    PFto_ADD("DeepSkyBlue2", "");
    PFto_ADD("DeepSkyBlue3", "");
    PFto_ADD("DeepSkyBlue4", "");
    PFto_ADD("DimGray", "");
    PFto_ADD("DimGrey", "");
    PFto_ADD("DodgerBlue", "");
    PFto_ADD("DodgerBlue1", "");
    PFto_ADD("DodgerBlue2", "");
    PFto_ADD("DodgerBlue3", "");
    PFto_ADD("DodgerBlue4", "");
    PFto_ADD("ENDTERM", "");
    PFto_ADD("F000", "");
    PFto_ADD("F001", "");
    PFto_ADD("F002", "");
    PFto_ADD("F003", "");
    PFto_ADD("F004", "");
    PFto_ADD("F005", "");
    PFto_ADD("F010", "");
    PFto_ADD("F011", "");
    PFto_ADD("F012", "");
    PFto_ADD("F013", "");
    PFto_ADD("F014", "");
    PFto_ADD("F015", "");
    PFto_ADD("F020", "");
    PFto_ADD("F021", "");
    PFto_ADD("F022", "");
    PFto_ADD("F023", "");
    PFto_ADD("F024", "");
    PFto_ADD("F025", "");
    PFto_ADD("F030", "");
    PFto_ADD("F031", "");
    PFto_ADD("F032", "");
    PFto_ADD("F033", "");
    PFto_ADD("F034", "");
    PFto_ADD("F035", "");
    PFto_ADD("F040", "");
    PFto_ADD("F041", "");
    PFto_ADD("F042", "");
    PFto_ADD("F043", "");
    PFto_ADD("F044", "");
    PFto_ADD("F045", "");
    PFto_ADD("F050", "");
    PFto_ADD("F051", "");
    PFto_ADD("F052", "");
    PFto_ADD("F053", "");
    PFto_ADD("F054", "");
    PFto_ADD("F055", "");
    PFto_ADD("F100", "");
    PFto_ADD("F101", "");
    PFto_ADD("F102", "");
    PFto_ADD("F103", "");
    PFto_ADD("F104", "");
    PFto_ADD("F105", "");
    PFto_ADD("F110", "");
    PFto_ADD("F111", "");
    PFto_ADD("F112", "");
    PFto_ADD("F113", "");
    PFto_ADD("F114", "");
    PFto_ADD("F115", "");
    PFto_ADD("F120", "");
    PFto_ADD("F121", "");
    PFto_ADD("F122", "");
    PFto_ADD("F123", "");
    PFto_ADD("F124", "");
    PFto_ADD("F125", "");
    PFto_ADD("F130", "");
    PFto_ADD("F131", "");
    PFto_ADD("F132", "");
    PFto_ADD("F133", "");
    PFto_ADD("F134", "");
    PFto_ADD("F135", "");
    PFto_ADD("F140", "");
    PFto_ADD("F141", "");
    PFto_ADD("F142", "");
    PFto_ADD("F143", "");
    PFto_ADD("F144", "");
    PFto_ADD("F145", "");
    PFto_ADD("F150", "");
    PFto_ADD("F151", "");
    PFto_ADD("F152", "");
    PFto_ADD("F153", "");
    PFto_ADD("F154", "");
    PFto_ADD("F155", "");
    PFto_ADD("F200", "");
    PFto_ADD("F201", "");
    PFto_ADD("F202", "");
    PFto_ADD("F203", "");
    PFto_ADD("F204", "");
    PFto_ADD("F205", "");
    PFto_ADD("F210", "");
    PFto_ADD("F211", "");
    PFto_ADD("F212", "");
    PFto_ADD("F213", "");
    PFto_ADD("F214", "");
    PFto_ADD("F215", "");
    PFto_ADD("F220", "");
    PFto_ADD("F221", "");
    PFto_ADD("F222", "");
    PFto_ADD("F223", "");
    PFto_ADD("F224", "");
    PFto_ADD("F225", "");
    PFto_ADD("F230", "");
    PFto_ADD("F231", "");
    PFto_ADD("F232", "");
    PFto_ADD("F233", "");
    PFto_ADD("F234", "");
    PFto_ADD("F235", "");
    PFto_ADD("F240", "");
    PFto_ADD("F241", "");
    PFto_ADD("F242", "");
    PFto_ADD("F243", "");
    PFto_ADD("F244", "");
    PFto_ADD("F245", "");
    PFto_ADD("F250", "");
    PFto_ADD("F251", "");
    PFto_ADD("F252", "");
    PFto_ADD("F253", "");
    PFto_ADD("F254", "");
    PFto_ADD("F255", "");
    PFto_ADD("F300", "");
    PFto_ADD("F301", "");
    PFto_ADD("F302", "");
    PFto_ADD("F303", "");
    PFto_ADD("F304", "");
    PFto_ADD("F305", "");
    PFto_ADD("F310", "");
    PFto_ADD("F311", "");
    PFto_ADD("F312", "");
    PFto_ADD("F313", "");
    PFto_ADD("F314", "");
    PFto_ADD("F315", "");
    PFto_ADD("F320", "");
    PFto_ADD("F321", "");
    PFto_ADD("F322", "");
    PFto_ADD("F323", "");
    PFto_ADD("F324", "");
    PFto_ADD("F325", "");
    PFto_ADD("F330", "");
    PFto_ADD("F331", "");
    PFto_ADD("F332", "");
    PFto_ADD("F333", "");
    PFto_ADD("F334", "");
    PFto_ADD("F335", "");
    PFto_ADD("F340", "");
    PFto_ADD("F341", "");
    PFto_ADD("F342", "");
    PFto_ADD("F343", "");
    PFto_ADD("F344", "");
    PFto_ADD("F345", "");
    PFto_ADD("F350", "");
    PFto_ADD("F351", "");
    PFto_ADD("F352", "");
    PFto_ADD("F353", "");
    PFto_ADD("F354", "");
    PFto_ADD("F355", "");
    PFto_ADD("F400", "");
    PFto_ADD("F401", "");
    PFto_ADD("F402", "");
    PFto_ADD("F403", "");
    PFto_ADD("F404", "");
    PFto_ADD("F405", "");
    PFto_ADD("F410", "");
    PFto_ADD("F411", "");
    PFto_ADD("F412", "");
    PFto_ADD("F413", "");
    PFto_ADD("F414", "");
    PFto_ADD("F415", "");
    PFto_ADD("F420", "");
    PFto_ADD("F421", "");
    PFto_ADD("F422", "");
    PFto_ADD("F423", "");
    PFto_ADD("F424", "");
    PFto_ADD("F425", "");
    PFto_ADD("F430", "");
    PFto_ADD("F431", "");
    PFto_ADD("F432", "");
    PFto_ADD("F433", "");
    PFto_ADD("F434", "");
    PFto_ADD("F435", "");
    PFto_ADD("F440", "");
    PFto_ADD("F441", "");
    PFto_ADD("F442", "");
    PFto_ADD("F443", "");
    PFto_ADD("F444", "");
    PFto_ADD("F445", "");
    PFto_ADD("F450", "");
    PFto_ADD("F451", "");
    PFto_ADD("F452", "");
    PFto_ADD("F453", "");
    PFto_ADD("F454", "");
    PFto_ADD("F455", "");
    PFto_ADD("F500", "");
    PFto_ADD("F501", "");
    PFto_ADD("F502", "");
    PFto_ADD("F503", "");
    PFto_ADD("F504", "");
    PFto_ADD("F505", "");
    PFto_ADD("F510", "");
    PFto_ADD("F511", "");
    PFto_ADD("F512", "");
    PFto_ADD("F513", "");
    PFto_ADD("F514", "");
    PFto_ADD("F515", "");
    PFto_ADD("F520", "");
    PFto_ADD("F521", "");
    PFto_ADD("F522", "");
    PFto_ADD("F523", "");
    PFto_ADD("F524", "");
    PFto_ADD("F525", "");
    PFto_ADD("F530", "");
    PFto_ADD("F531", "");
    PFto_ADD("F532", "");
    PFto_ADD("F533", "");
    PFto_ADD("F534", "");
    PFto_ADD("F535", "");
    PFto_ADD("F540", "");
    PFto_ADD("F541", "");
    PFto_ADD("F542", "");
    PFto_ADD("F543", "");
    PFto_ADD("F544", "");
    PFto_ADD("F545", "");
    PFto_ADD("F550", "");
    PFto_ADD("F551", "");
    PFto_ADD("F552", "");
    PFto_ADD("F553", "");
    PFto_ADD("F554", "");
    PFto_ADD("F555", "");
    PFto_ADD("FLASH", "");
    PFto_ADD("Firebrick", "");
    PFto_ADD("Firebrick1", "");
    PFto_ADD("Firebrick2", "");
    PFto_ADD("Firebrick3", "");
    PFto_ADD("Firebrick4", "");
    PFto_ADD("FloralWhite", "");
    PFto_ADD("ForestGreen", "");
    PFto_ADD("G00", "");
    PFto_ADD("G01", "");
    PFto_ADD("G02", "");
    PFto_ADD("G03", "");
    PFto_ADD("G04", "");
    PFto_ADD("G05", "");
    PFto_ADD("G06", "");
    PFto_ADD("G07", "");
    PFto_ADD("G08", "");
    PFto_ADD("G09", "");
    PFto_ADD("G10", "");
    PFto_ADD("G11", "");
    PFto_ADD("G12", "");
    PFto_ADD("G13", "");
    PFto_ADD("G14", "");
    PFto_ADD("G15", "");
    PFto_ADD("G16", "");
    PFto_ADD("G17", "");
    PFto_ADD("G18", "");
    PFto_ADD("G19", "");
    PFto_ADD("G20", "");
    PFto_ADD("G21", "");
    PFto_ADD("G22", "");
    PFto_ADD("G23", "");
    PFto_ADD("G24", "");
    PFto_ADD("G25", "");
    PFto_ADD("GREEN", "");
    PFto_ADD("GREY", "");
    PFto_ADD("Gainsboro", "");
    PFto_ADD("GhostWhite", "");
    PFto_ADD("Gold", "");
    PFto_ADD("Gold1", "");
    PFto_ADD("Gold2", "");
    PFto_ADD("Gold3", "");
    PFto_ADD("Gold4", "");
    PFto_ADD("Goldenrod", "");
    PFto_ADD("Goldenrod1", "");
    PFto_ADD("Goldenrod2", "");
    PFto_ADD("Goldenrod3", "");
    PFto_ADD("Goldenrod4", "");
    PFto_ADD("Gray", "");
    PFto_ADD("Gray0", "");
    PFto_ADD("Gray1", "");
    PFto_ADD("Gray10", "");
    PFto_ADD("Gray100", "");
    PFto_ADD("Gray11", "");
    PFto_ADD("Gray12", "");
    PFto_ADD("Gray13", "");
    PFto_ADD("Gray14", "");
    PFto_ADD("Gray15", "");
    PFto_ADD("Gray16", "");
    PFto_ADD("Gray17", "");
    PFto_ADD("Gray18", "");
    PFto_ADD("Gray19", "");
    PFto_ADD("Gray2", "");
    PFto_ADD("Gray20", "");
    PFto_ADD("Gray21", "");
    PFto_ADD("Gray22", "");
    PFto_ADD("Gray23", "");
    PFto_ADD("Gray24", "");
    PFto_ADD("Gray25", "");
    PFto_ADD("Gray26", "");
    PFto_ADD("Gray27", "");
    PFto_ADD("Gray28", "");
    PFto_ADD("Gray29", "");
    PFto_ADD("Gray3", "");
    PFto_ADD("Gray30", "");
    PFto_ADD("Gray31", "");
    PFto_ADD("Gray32", "");
    PFto_ADD("Gray33", "");
    PFto_ADD("Gray34", "");
    PFto_ADD("Gray35", "");
    PFto_ADD("Gray36", "");
    PFto_ADD("Gray37", "");
    PFto_ADD("Gray38", "");
    PFto_ADD("Gray39", "");
    PFto_ADD("Gray4", "");
    PFto_ADD("Gray40", "");
    PFto_ADD("Gray41", "");
    PFto_ADD("Gray42", "");
    PFto_ADD("Gray43", "");
    PFto_ADD("Gray44", "");
    PFto_ADD("Gray45", "");
    PFto_ADD("Gray46", "");
    PFto_ADD("Gray47", "");
    PFto_ADD("Gray48", "");
    PFto_ADD("Gray49", "");
    PFto_ADD("Gray5", "");
    PFto_ADD("Gray50", "");
    PFto_ADD("Gray51", "");
    PFto_ADD("Gray52", "");
    PFto_ADD("Gray53", "");
    PFto_ADD("Gray54", "");
    PFto_ADD("Gray55", "");
    PFto_ADD("Gray56", "");
    PFto_ADD("Gray57", "");
    PFto_ADD("Gray58", "");
    PFto_ADD("Gray59", "");
    PFto_ADD("Gray6", "");
    PFto_ADD("Gray60", "");
    PFto_ADD("Gray61", "");
    PFto_ADD("Gray62", "");
    PFto_ADD("Gray63", "");
    PFto_ADD("Gray64", "");
    PFto_ADD("Gray65", "");
    PFto_ADD("Gray66", "");
    PFto_ADD("Gray67", "");
    PFto_ADD("Gray68", "");
    PFto_ADD("Gray69", "");
    PFto_ADD("Gray7", "");
    PFto_ADD("Gray70", "");
    PFto_ADD("Gray71", "");
    PFto_ADD("Gray72", "");
    PFto_ADD("Gray73", "");
    PFto_ADD("Gray74", "");
    PFto_ADD("Gray75", "");
    PFto_ADD("Gray76", "");
    PFto_ADD("Gray77", "");
    PFto_ADD("Gray78", "");
    PFto_ADD("Gray79", "");
    PFto_ADD("Gray8", "");
    PFto_ADD("Gray80", "");
    PFto_ADD("Gray81", "");
    PFto_ADD("Gray82", "");
    PFto_ADD("Gray83", "");
    PFto_ADD("Gray84", "");
    PFto_ADD("Gray85", "");
    PFto_ADD("Gray86", "");
    PFto_ADD("Gray87", "");
    PFto_ADD("Gray88", "");
    PFto_ADD("Gray89", "");
    PFto_ADD("Gray9", "");
    PFto_ADD("Gray90", "");
    PFto_ADD("Gray91", "");
    PFto_ADD("Gray92", "");
    PFto_ADD("Gray93", "");
    PFto_ADD("Gray94", "");
    PFto_ADD("Gray95", "");
    PFto_ADD("Gray96", "");
    PFto_ADD("Gray97", "");
    PFto_ADD("Gray98", "");
    PFto_ADD("Gray99", "");
    PFto_ADD("Green", "");
    PFto_ADD("Green1", "");
    PFto_ADD("Green2", "");
    PFto_ADD("Green3", "");
    PFto_ADD("Green4", "");
    PFto_ADD("GreenYellow", "");
    PFto_ADD("Grey", "");
    PFto_ADD("Grey0", "");
    PFto_ADD("Grey1", "");
    PFto_ADD("Grey10", "");
    PFto_ADD("Grey100", "");
    PFto_ADD("Grey11", "");
    PFto_ADD("Grey12", "");
    PFto_ADD("Grey13", "");
    PFto_ADD("Grey14", "");
    PFto_ADD("Grey15", "");
    PFto_ADD("Grey16", "");
    PFto_ADD("Grey17", "");
    PFto_ADD("Grey18", "");
    PFto_ADD("Grey19", "");
    PFto_ADD("Grey2", "");
    PFto_ADD("Grey20", "");
    PFto_ADD("Grey21", "");
    PFto_ADD("Grey22", "");
    PFto_ADD("Grey23", "");
    PFto_ADD("Grey24", "");
    PFto_ADD("Grey25", "");
    PFto_ADD("Grey26", "");
    PFto_ADD("Grey27", "");
    PFto_ADD("Grey28", "");
    PFto_ADD("Grey29", "");
    PFto_ADD("Grey3", "");
    PFto_ADD("Grey30", "");
    PFto_ADD("Grey31", "");
    PFto_ADD("Grey32", "");
    PFto_ADD("Grey33", "");
    PFto_ADD("Grey34", "");
    PFto_ADD("Grey35", "");
    PFto_ADD("Grey36", "");
    PFto_ADD("Grey37", "");
    PFto_ADD("Grey38", "");
    PFto_ADD("Grey39", "");
    PFto_ADD("Grey4", "");
    PFto_ADD("Grey40", "");
    PFto_ADD("Grey41", "");
    PFto_ADD("Grey42", "");
    PFto_ADD("Grey43", "");
    PFto_ADD("Grey44", "");
    PFto_ADD("Grey45", "");
    PFto_ADD("Grey46", "");
    PFto_ADD("Grey47", "");
    PFto_ADD("Grey48", "");
    PFto_ADD("Grey49", "");
    PFto_ADD("Grey5", "");
    PFto_ADD("Grey50", "");
    PFto_ADD("Grey51", "");
    PFto_ADD("Grey52", "");
    PFto_ADD("Grey53", "");
    PFto_ADD("Grey54", "");
    PFto_ADD("Grey55", "");
    PFto_ADD("Grey56", "");
    PFto_ADD("Grey57", "");
    PFto_ADD("Grey58", "");
    PFto_ADD("Grey59", "");
    PFto_ADD("Grey6", "");
    PFto_ADD("Grey60", "");
    PFto_ADD("Grey61", "");
    PFto_ADD("Grey62", "");
    PFto_ADD("Grey63", "");
    PFto_ADD("Grey64", "");
    PFto_ADD("Grey65", "");
    PFto_ADD("Grey66", "");
    PFto_ADD("Grey67", "");
    PFto_ADD("Grey68", "");
    PFto_ADD("Grey69", "");
    PFto_ADD("Grey7", "");
    PFto_ADD("Grey70", "");
    PFto_ADD("Grey71", "");
    PFto_ADD("Grey72", "");
    PFto_ADD("Grey73", "");
    PFto_ADD("Grey74", "");
    PFto_ADD("Grey75", "");
    PFto_ADD("Grey76", "");
    PFto_ADD("Grey77", "");
    PFto_ADD("Grey78", "");
    PFto_ADD("Grey79", "");
    PFto_ADD("Grey8", "");
    PFto_ADD("Grey80", "");
    PFto_ADD("Grey81", "");
    PFto_ADD("Grey82", "");
    PFto_ADD("Grey83", "");
    PFto_ADD("Grey84", "");
    PFto_ADD("Grey85", "");
    PFto_ADD("Grey86", "");
    PFto_ADD("Grey87", "");
    PFto_ADD("Grey88", "");
    PFto_ADD("Grey89", "");
    PFto_ADD("Grey9", "");
    PFto_ADD("Grey90", "");
    PFto_ADD("Grey91", "");
    PFto_ADD("Grey92", "");
    PFto_ADD("Grey93", "");
    PFto_ADD("Grey94", "");
    PFto_ADD("Grey95", "");
    PFto_ADD("Grey96", "");
    PFto_ADD("Grey97", "");
    PFto_ADD("Grey98", "");
    PFto_ADD("Grey99", "");
    PFto_ADD("HOME", "");
    PFto_ADD("Honeydew", "");
    PFto_ADD("Honeydew1", "");
    PFto_ADD("Honeydew2", "");
    PFto_ADD("Honeydew3", "");
    PFto_ADD("Honeydew4", "");
    PFto_ADD("HotPink", "");
    PFto_ADD("HotPink1", "");
    PFto_ADD("HotPink2", "");
    PFto_ADD("HotPink3", "");
    PFto_ADD("HotPink4", "");
    PFto_ADD("INITTERM", "");
    PFto_ADD("ITALIC", "");
    PFto_ADD("IndianRed", "");
    PFto_ADD("IndianRed1", "");
    PFto_ADD("IndianRed2", "");
    PFto_ADD("IndianRed3", "");
    PFto_ADD("IndianRed4", "");
    PFto_ADD("Ivory", "");
    PFto_ADD("Ivory1", "");
    PFto_ADD("Ivory2", "");
    PFto_ADD("Ivory3", "");
    PFto_ADD("Ivory4", "");
    PFto_ADD("Khaki", "");
    PFto_ADD("Khaki1", "");
    PFto_ADD("Khaki2", "");
    PFto_ADD("Khaki3", "");
    PFto_ADD("Khaki4", "");
    PFto_ADD("LIGHTBLUE", "");
    PFto_ADD("LIGHTCYAN", "");
    PFto_ADD("LIGHTGREEN", "");
    PFto_ADD("LIGHTRED", "");
    PFto_ADD("Lavender", "");
    PFto_ADD("LavenderBlush", "");
    PFto_ADD("LavenderBlush1", "");
    PFto_ADD("LavenderBlush2", "");
    PFto_ADD("LavenderBlush3", "");
    PFto_ADD("LavenderBlush4", "");
    PFto_ADD("LawnGreen", "");
    PFto_ADD("LemonChiffon", "");
    PFto_ADD("LemonChiffon1", "");
    PFto_ADD("LemonChiffon2", "");
    PFto_ADD("LemonChiffon3", "");
    PFto_ADD("LemonChiffon4", "");
    PFto_ADD("LightBlue", "");
    PFto_ADD("LightBlue1", "");
    PFto_ADD("LightBlue2", "");
    PFto_ADD("LightBlue3", "");
    PFto_ADD("LightBlue4", "");
    PFto_ADD("LightCoral", "");
    PFto_ADD("LightCyan", "");
    PFto_ADD("LightCyan1", "");
    PFto_ADD("LightCyan2", "");
    PFto_ADD("LightCyan3", "");
    PFto_ADD("LightCyan4", "");
    PFto_ADD("LightGoldenrod", "");
    PFto_ADD("LightGoldenrod1", "");
    PFto_ADD("LightGoldenrod2", "");
    PFto_ADD("LightGoldenrod3", "");
    PFto_ADD("LightGoldenrod4", "");
    PFto_ADD("LightGoldenrodYellow", "");
    PFto_ADD("LightGray", "");
    PFto_ADD("LightGreen", "");
    PFto_ADD("LightGrey", "");
    PFto_ADD("LightPink", "");
    PFto_ADD("LightPink1", "");
    PFto_ADD("LightPink2", "");
    PFto_ADD("LightPink3", "");
    PFto_ADD("LightPink4", "");
    PFto_ADD("LightSalmon", "");
    PFto_ADD("LightSalmon1", "");
    PFto_ADD("LightSalmon2", "");
    PFto_ADD("LightSalmon3", "");
    PFto_ADD("LightSalmon4", "");
    PFto_ADD("LightSeaGreen", "");
    PFto_ADD("LightSkyBlue", "");
    PFto_ADD("LightSkyBlue1", "");
    PFto_ADD("LightSkyBlue2", "");
    PFto_ADD("LightSkyBlue3", "");
    PFto_ADD("LightSkyBlue4", "");
    PFto_ADD("LightSlateBlue", "");
    PFto_ADD("LightSlateGray", "");
    PFto_ADD("LightSlateGrey", "");
    PFto_ADD("LightSteelBlue", "");
    PFto_ADD("LightSteelBlue1", "");
    PFto_ADD("LightSteelBlue2", "");
    PFto_ADD("LightSteelBlue3", "");
    PFto_ADD("LightSteelBlue4", "");
    PFto_ADD("LightYellow", "");
    PFto_ADD("LightYellow1", "");
    PFto_ADD("LightYellow2", "");
    PFto_ADD("LightYellow3", "");
    PFto_ADD("LightYellow4", "");
    PFto_ADD("LimeGreen", "");
    PFto_ADD("Linen", "");
    PFto_ADD("MAGENTA", "");
    PFto_ADD("Magenta", "");
    PFto_ADD("Magenta1", "");
    PFto_ADD("Magenta2", "");
    PFto_ADD("Magenta3", "");
    PFto_ADD("Magenta4", "");
    PFto_ADD("Maroon", "");
    PFto_ADD("Maroon1", "");
    PFto_ADD("Maroon2", "");
    PFto_ADD("Maroon3", "");
    PFto_ADD("Maroon4", "");
    PFto_ADD("MediumAquamarine", "");
    PFto_ADD("MediumBlue", "");
    PFto_ADD("MediumOrchid", "");
    PFto_ADD("MediumOrchid1", "");
    PFto_ADD("MediumOrchid2", "");
    PFto_ADD("MediumOrchid3", "");
    PFto_ADD("MediumOrchid4", "");
    PFto_ADD("MediumPurple", "");
    PFto_ADD("MediumPurple1", "");
    PFto_ADD("MediumPurple2", "");
    PFto_ADD("MediumPurple3", "");
    PFto_ADD("MediumPurple4", "");
    PFto_ADD("MediumSeaGreen", "");
    PFto_ADD("MediumSlateBlue", "");
    PFto_ADD("MediumSpringGreen", "");
    PFto_ADD("MediumTurquoise", "");
    PFto_ADD("MediumVioletRed", "");
    PFto_ADD("MidnightBlue", "");
    PFto_ADD("MintCream", "");
    PFto_ADD("MistyRose", "");
    PFto_ADD("MistyRose1", "");
    PFto_ADD("MistyRose2", "");
    PFto_ADD("MistyRose3", "");
    PFto_ADD("MistyRose4", "");
    PFto_ADD("Moccasin", "");
    PFto_ADD("NavajoWhite", "");
    PFto_ADD("NavajoWhite1", "");
    PFto_ADD("NavajoWhite2", "");
    PFto_ADD("NavajoWhite3", "");
    PFto_ADD("NavajoWhite4", "");
    PFto_ADD("Navy", "");
    PFto_ADD("NavyBlue", "");
    PFto_ADD("ORANGE", "");
    PFto_ADD("OldLace", "");
    PFto_ADD("OliveDrab", "");
    PFto_ADD("OliveDrab1", "");
    PFto_ADD("OliveDrab2", "");
    PFto_ADD("OliveDrab3", "");
    PFto_ADD("OliveDrab4", "");
    PFto_ADD("Orange", "");
    PFto_ADD("Orange1", "");
    PFto_ADD("Orange2", "");
    PFto_ADD("Orange3", "");
    PFto_ADD("Orange4", "");
    PFto_ADD("OrangeRed", "");
    PFto_ADD("OrangeRed1", "");
    PFto_ADD("OrangeRed2", "");
    PFto_ADD("OrangeRed3", "");
    PFto_ADD("OrangeRed4", "");
    PFto_ADD("Orchid", "");
    PFto_ADD("Orchid1", "");
    PFto_ADD("Orchid2", "");
    PFto_ADD("Orchid3", "");
    PFto_ADD("Orchid4", "");
    PFto_ADD("PINK", "");
    PFto_ADD("PaleGoldenrod", "");
    PFto_ADD("PaleGreen", "");
    PFto_ADD("PaleGreen1", "");
    PFto_ADD("PaleGreen2", "");
    PFto_ADD("PaleGreen3", "");
    PFto_ADD("PaleGreen4", "");
    PFto_ADD("PaleTurquoise", "");
    PFto_ADD("PaleTurquoise1", "");
    PFto_ADD("PaleTurquoise2", "");
    PFto_ADD("PaleTurquoise3", "");
    PFto_ADD("PaleTurquoise4", "");
    PFto_ADD("PaleVioletRed", "");
    PFto_ADD("PaleVioletRed1", "");
    PFto_ADD("PaleVioletRed2", "");
    PFto_ADD("PaleVioletRed3", "");
    PFto_ADD("PaleVioletRed4", "");
    PFto_ADD("PapayaWhip", "");
    PFto_ADD("PeachPuff", "");
    PFto_ADD("PeachPuff1", "");
    PFto_ADD("PeachPuff2", "");
    PFto_ADD("PeachPuff3", "");
    PFto_ADD("PeachPuff4", "");
    PFto_ADD("Peru", "");
    PFto_ADD("Pink", "");
    PFto_ADD("Pink1", "");
    PFto_ADD("Pink2", "");
    PFto_ADD("Pink3", "");
    PFto_ADD("Pink4", "");
    PFto_ADD("Plum", "");
    PFto_ADD("Plum1", "");
    PFto_ADD("Plum2", "");
    PFto_ADD("Plum3", "");
    PFto_ADD("Plum4", "");
    PFto_ADD("PowderBlue", "");
    PFto_ADD("Purple", "");
    PFto_ADD("Purple1", "");
    PFto_ADD("Purple2", "");
    PFto_ADD("Purple3", "");
    PFto_ADD("Purple4", "");
    PFto_ADD("RED", "");
    PFto_ADD("RESET", "");
    PFto_ADD("RESTORE", "");
    PFto_ADD("REVERSE", "");
    PFto_ADD("Red", "");
    PFto_ADD("Red1", "");
    PFto_ADD("Red2", "");
    PFto_ADD("Red3", "");
    PFto_ADD("Red4", "");
    PFto_ADD("RosyBrown", "");
    PFto_ADD("RosyBrown1", "");
    PFto_ADD("RosyBrown2", "");
    PFto_ADD("RosyBrown3", "");
    PFto_ADD("RosyBrown4", "");
    PFto_ADD("RoyalBlue", "");
    PFto_ADD("RoyalBlue1", "");
    PFto_ADD("RoyalBlue2", "");
    PFto_ADD("RoyalBlue3", "");
    PFto_ADD("RoyalBlue4", "");
    PFto_ADD("SAVE", "");
    PFto_ADD("STRIKETHRU", "");
    PFto_ADD("SaddleBrown", "");
    PFto_ADD("Salmon", "");
    PFto_ADD("Salmon1", "");
    PFto_ADD("Salmon2", "");
    PFto_ADD("Salmon3", "");
    PFto_ADD("Salmon4", "");
    PFto_ADD("SandyBrown", "");
    PFto_ADD("SeaGreen", "");
    PFto_ADD("SeaGreen1", "");
    PFto_ADD("SeaGreen2", "");
    PFto_ADD("SeaGreen3", "");
    PFto_ADD("SeaGreen4", "");
    PFto_ADD("Seashell", "");
    PFto_ADD("Seashell1", "");
    PFto_ADD("Seashell2", "");
    PFto_ADD("Seashell3", "");
    PFto_ADD("Seashell4", "");
    PFto_ADD("Sienna", "");
    PFto_ADD("Sienna1", "");
    PFto_ADD("Sienna2", "");
    PFto_ADD("Sienna3", "");
    PFto_ADD("Sienna4", "");
    PFto_ADD("SkyBlue", "");
    PFto_ADD("SkyBlue1", "");
    PFto_ADD("SkyBlue2", "");
    PFto_ADD("SkyBlue3", "");
    PFto_ADD("SkyBlue4", "");
    PFto_ADD("SlateBlue", "");
    PFto_ADD("SlateBlue1", "");
    PFto_ADD("SlateBlue2", "");
    PFto_ADD("SlateBlue3", "");
    PFto_ADD("SlateBlue4", "");
    PFto_ADD("SlateGray", "");
    PFto_ADD("SlateGray1", "");
    PFto_ADD("SlateGray2", "");
    PFto_ADD("SlateGray3", "");
    PFto_ADD("SlateGray4", "");
    PFto_ADD("SlateGrey", "");
    PFto_ADD("Snow", "");
    PFto_ADD("Snow1", "");
    PFto_ADD("Snow2", "");
    PFto_ADD("Snow3", "");
    PFto_ADD("Snow4", "");
    PFto_ADD("SpringGreen", "");
    PFto_ADD("SpringGreen1", "");
    PFto_ADD("SpringGreen2", "");
    PFto_ADD("SpringGreen3", "");
    PFto_ADD("SpringGreen4", "");
    PFto_ADD("SteelBlue", "");
    PFto_ADD("SteelBlue1", "");
    PFto_ADD("SteelBlue2", "");
    PFto_ADD("SteelBlue3", "");
    PFto_ADD("SteelBlue4", "");
    PFto_ADD("Tan", "");
    PFto_ADD("Tan1", "");
    PFto_ADD("Tan2", "");
    PFto_ADD("Tan3", "");
    PFto_ADD("Tan4", "");
    PFto_ADD("Thistle", "");
    PFto_ADD("Thistle1", "");
    PFto_ADD("Thistle2", "");
    PFto_ADD("Thistle3", "");
    PFto_ADD("Thistle4", "");
    PFto_ADD("Tomato", "");
    PFto_ADD("Tomato1", "");
    PFto_ADD("Tomato2", "");
    PFto_ADD("Tomato3", "");
    PFto_ADD("Tomato4", "");
    PFto_ADD("Turquoise", "");
    PFto_ADD("Turquoise1", "");
    PFto_ADD("Turquoise2", "");
    PFto_ADD("Turquoise3", "");
    PFto_ADD("Turquoise4", "");
    PFto_ADD("UNDERLINE", "");
    PFto_ADD("Violet", "");
    PFto_ADD("VioletRed", "");
    PFto_ADD("VioletRed1", "");
    PFto_ADD("VioletRed2", "");
    PFto_ADD("VioletRed3", "");
    PFto_ADD("VioletRed4", "");
    PFto_ADD("WHITE", "");
    PFto_ADD("Wheat", "");
    PFto_ADD("Wheat1", "");
    PFto_ADD("Wheat2", "");
    PFto_ADD("Wheat3", "");
    PFto_ADD("Wheat4", "");
    PFto_ADD("White", "");
    PFto_ADD("WhiteSmoke", "");
    PFto_ADD("YELLOW", "");
    PFto_ADD("Yellow", "");
    PFto_ADD("Yellow1", "");
    PFto_ADD("Yellow2", "");
    PFto_ADD("Yellow3", "");
    PFto_ADD("Yellow4", "");
    PFto_ADD("YellowGreen", "");

    log_info("Pinkfish to NULL conversion data loaded.");
}

void I3_savecolor(void)
{
    FILE *fp;
    I3_COLOR *color;

    if ((fp = fopen(I3_COLOR_FILE, "w")) == NULL)
    {
        log_info("%s", "Couldn't write to I3 color file.");
        return;
    }

    for (color = first_i3_color; color; color = color->next)
    {
        fprintf(fp, "%s", "#COLOR\n");
        fprintf(fp, "Name   %s\n", color->name);
        fprintf(fp, "Mudtag %s\n", color->mudtag);
        fprintf(fp, "IMCtag %s\n", color->imctag);
        fprintf(fp, "I3tag  %s\n", color->i3tag);
        fprintf(fp, "%s", "End\n\n");
    }
    fprintf(fp, "%s", "#END\n");
    I3FCLOSE(fp);
}

void I3_readcolor(I3_COLOR *color, FILE *fp)
{
    const char *word;
    bool fMatch;

    for (;;)
    {
        word = feof(fp) ? "End" : i3fread_word(fp);
        fMatch = FALSE;

        switch (word[0])
        {
        case '*':
            fMatch = TRUE;
            i3fread_to_eol(fp);
            break;

        case 'E':
            if (!strcasecmp(word, "End"))
                return;
            break;

        case 'I':
            KEY("IMCtag", color->imctag, i3fread_line(fp));
            KEY("I3tag", color->i3tag, i3fread_line(fp));
            break;

        case 'M':
            KEY("Mudtag", color->mudtag, i3fread_line(fp));
            break;

        case 'N':
            KEY("Name", color->name, i3fread_line(fp));
            break;
        }
        if (!fMatch)
            log_error("i3_readcolor: no match: %s", word);
    }
}

void I3_load_color_table(void)
{
    FILE *fp;
    I3_COLOR *color;

    first_i3_color = last_i3_color = NULL;

    log_info("%s", "Loading color table...");

    if (!(fp = fopen(I3_COLOR_FILE, "r")))
    {
        log_info("%s", "No color table found.");
        return;
    }

    for (;;)
    {
        char letter;
        char *word;

        letter = i3fread_letter(fp);
        if (letter == '*')
        {
            i3fread_to_eol(fp);
            continue;
        }

        if (letter != '#')
        {
            log_error("%s", "i3_load_color_table: # not found.");
            break;
        }

        word = i3fread_word(fp);
        if (!strcasecmp(word, "COLOR"))
        {
            I3CREATE(color, I3_COLOR, 1);

            I3_readcolor(color, fp);
            I3LINK(color, first_i3_color, last_i3_color, next, prev);
            continue;
        }
        else if (!strcasecmp(word, "END"))
            break;
        else
        {
            log_error("i3_load_color_table: bad section: %s.", word);
            continue;
        }
    }
    I3FCLOSE(fp);
}

void I3_savehelps(void)
{
    FILE *fp;
    I3_HELP_DATA *help;

    if ((fp = fopen(I3_HELP_FILE, "w")) == NULL)
    {
        log_info("%s", "Couldn't write to I3 help file.");
        return;
    }

    for (help = first_i3_help; help; help = help->next)
    {
        fprintf(fp, "%s", "#HELP\n");
        fprintf(fp, "Name %s\n", help->name);
        fprintf(fp, "Perm %s\n", perm_names[help->level]);
        // fprintf(fp, "Text %s¢\n", help->text);
        fprintf(fp, "Text %s%c\n", help->text, '\0xA2');
        fprintf(fp, "%s", "End\n\n");
    }
    fprintf(fp, "%s", "#END\n");
    I3FCLOSE(fp);
}

void I3_readhelp(I3_HELP_DATA *help, FILE *fp)
{
    const char *word;
    // char                                    hbuf[MAX_STRING_LENGTH];
    int permvalue;
    bool fMatch;

    for (;;)
    {
        word = feof(fp) ? "End" : i3fread_word(fp);
        fMatch = FALSE;

        switch (word[0])
        {
        case '*':
            fMatch = TRUE;
            i3fread_to_eol(fp);
            break;

        case 'E':
            if (!strcasecmp(word, "End"))
                return;
            break;

        case 'N':
            KEY("Name", help->name, i3fread_line(fp));
            break;

        case 'P':
            if (!strcasecmp(word, "Perm"))
            {
                word = i3fread_word(fp);
                permvalue = get_permvalue(word);

                if (permvalue < 0 || permvalue > I3PERM_IMP)
                {
                    log_error("i3_readhelp: Command %s loaded with invalid permission. Set to Imp.", help->name);
                    help->level = I3PERM_IMP;
                }
                else
                    help->level = permvalue;
                fMatch = TRUE;
                break;
            }
            break;

        case 'T':
            if (!strcasecmp(word, "Text"))
            {
                help->text = I3STRALLOC(i3fread_string(fp));
                fMatch = TRUE;
                break;
            }
            /*
    if (!strcasecmp(word, "Text")) {
        int                                     num = 0;

        //while ((hbuf[num] = fgetc(fp)) != EOF && hbuf[num] != '¢'
        while ((hbuf[num] = fgetc(fp)) != EOF && (int) hbuf[num] != (int) '\0xA2'
           && num < (MAX_STRING_LENGTH - 2))
        num++;
        hbuf[num] = '\0';
        help->text = I3STRALLOC(hbuf);
        fMatch = TRUE;
        break;
    }
    KEY("Text", help->text, i3fread_line(fp));
            */
            break;
        }
        if (!fMatch)
            log_error("i3_readhelp: no match: %s", word);
    }
}

void I3_load_helps(void)
{
    FILE *fp;
    I3_HELP_DATA *help;

    first_i3_help = last_i3_help = NULL;

    log_info("%s", "Loading I3 help file...");

    if (!(fp = fopen(I3_HELP_FILE, "r")))
    {
        log_info("%s", "No help file found.");
        return;
    }

    for (;;)
    {
        char letter;
        char *word;

        letter = i3fread_letter(fp);
        if (letter == '*')
        {
            i3fread_to_eol(fp);
            continue;
        }

        if (letter != '#')
        {
            log_error("%s", "i3_load_helps: # not found.");
            break;
        }

        word = i3fread_word(fp);
        if (!strcasecmp(word, "HELP"))
        {
            I3CREATE(help, I3_HELP_DATA, 1);

            I3_readhelp(help, fp);
            I3LINK(help, first_i3_help, last_i3_help, next, prev);
            continue;
        }
        else if (!strcasecmp(word, "END"))
            break;
        else
        {
            log_error("i3_load_helps: bad section: %s.", word);
            continue;
        }
    }
    I3FCLOSE(fp);
}

void I3_savecommands(void)
{
    FILE *fp;
    I3_CMD_DATA *cmd;
    I3_ALIAS *alias;

    if (!(fp = fopen(I3_CMD_FILE, "w")))
    {
        log_info("%s", "Couldn't write to I3 command file.");
        return;
    }

    for (cmd = first_i3_command; cmd; cmd = cmd->next)
    {
        fprintf(fp, "%s", "#COMMAND\n");
        fprintf(fp, "Name      %s\n", cmd->name);
        if (cmd->function != NULL)
            fprintf(fp, "Code      %s\n", i3_funcname(cmd->function));
        else
            fprintf(fp, "%s", "Code      NULL\n");
        fprintf(fp, "Perm      %s\n", perm_names[cmd->level]);
        fprintf(fp, "Connected %d\n", cmd->connected);
        for (alias = cmd->first_alias; alias; alias = alias->next)
            fprintf(fp, "Alias     %s\n", alias->name);
        fprintf(fp, "%s", "End\n\n");
    }
    fprintf(fp, "%s", "#END\n");
    I3FCLOSE(fp);
}

void I3_readcommand(I3_CMD_DATA *cmd, FILE *fp)
{
    I3_ALIAS *alias;
    const char *word;
    int permvalue;
    bool fMatch;

    for (;;)
    {
        word = feof(fp) ? "End" : i3fread_word(fp);
        fMatch = FALSE;

        switch (word[0])
        {
        case '*':
            fMatch = TRUE;
            i3fread_to_eol(fp);
            break;

        case 'E':
            if (!strcasecmp(word, "End"))
                return;
            break;

        case 'A':
            if (!strcasecmp(word, "Alias"))
            {
                I3CREATE(alias, I3_ALIAS, 1);

                alias->name = i3fread_line(fp);
                I3LINK(alias, cmd->first_alias, cmd->last_alias, next, prev);
                fMatch = TRUE;
                break;
            }
            break;

        case 'C':
            KEY("Connected", cmd->connected, i3fread_number(fp));
            if (!strcasecmp(word, "Code"))
            {
                word = i3fread_word(fp);
                cmd->function = i3_function(word);
                if (cmd->function == NULL)
                    log_error("i3_readcommand: Command %s loaded with invalid function. Set to NULL.", cmd->name);
                fMatch = TRUE;
                break;
            }
            break;

        case 'N':
            KEY("Name", cmd->name, i3fread_line(fp));
            break;

        case 'P':
            if (!strcasecmp(word, "Perm"))
            {
                word = i3fread_word(fp);
                permvalue = get_permvalue(word);

                if (permvalue < 0 || permvalue > I3PERM_IMP)
                {
                    log_error("i3_readcommand: Command %s loaded with invalid permission. Set to Imp.", cmd->name);
                    cmd->level = I3PERM_IMP;
                }
                else
                    cmd->level = permvalue;
                fMatch = TRUE;
                break;
            }
            break;
        }
        if (!fMatch)
            log_error("i3_readcommand: no match: %s", word);
    }
}

bool I3_load_commands(void)
{
    FILE *fp;
    I3_CMD_DATA *cmd;

    first_i3_command = last_i3_command = NULL;

    log_info("%s", "Loading I3 command table...");

    if (!(fp = fopen(I3_CMD_FILE, "r")))
    {
        log_info("%s", "No command table found.");
        return FALSE;
    }

    for (;;)
    {
        char letter;
        char *word;

        letter = i3fread_letter(fp);
        if (letter == '*')
        {
            i3fread_to_eol(fp);
            continue;
        }

        if (letter != '#')
        {
            log_error("%s", "i3_load_commands: # not found.");
            break;
        }

        word = i3fread_word(fp);
        if (!strcasecmp(word, "COMMAND"))
        {
            I3CREATE(cmd, I3_CMD_DATA, 1);

            I3_readcommand(cmd, fp);
            I3LINK(cmd, first_i3_command, last_i3_command, next, prev);
            continue;
        }
        else if (!strcasecmp(word, "END"))
            break;
        else
        {
            log_error("i3_load_commands: bad section: %s.", word);
            continue;
        }
    }
    I3FCLOSE(fp);
    return TRUE;
}

void I3_saverouters(void)
{
    FILE *fp;
    ROUTER_DATA *router;

    if (!(fp = fopen(I3_ROUTER_FILE, "w")))
    {
        log_info("%s", "Couldn't write to I3 router file.");
        return;
    }

    for (router = first_router; router; router = router->next)
    {
        fprintf(fp, "%s", "#ROUTER\n");
        fprintf(fp, "Name %s\n", router->name);
        fprintf(fp, "IP   %s\n", router->ip);
        fprintf(fp, "Port %d\n", router->port);
        fprintf(fp, "Password %d\n", router->password);
        fprintf(fp, "MudlistID %d\n", router->mudlist_id);
        fprintf(fp, "ChanlistID %d\n", router->chanlist_id);
        fprintf(fp, "%s", "End\n\n");
    }
    fprintf(fp, "%s", "#END\n");
    I3FCLOSE(fp);
    save_routers();
}

void I3_readrouter(ROUTER_DATA *router, FILE *fp)
{
    const char *word;
    bool fMatch;

    for (;;)
    {
        word = feof(fp) ? "End" : i3fread_word(fp);
        fMatch = FALSE;

        switch (word[0])
        {
        case '*':
            fMatch = TRUE;
            i3fread_to_eol(fp);
            break;

        case 'C':
            KEY("ChanlistID", router->chanlist_id, i3fread_number(fp));
            break;

        case 'E':
            if (!strcasecmp(word, "End"))
                return;
            break;

        case 'I':
            KEY("IP", router->ip, i3fread_line(fp));
            break;

        case 'M':
            KEY("MudlistID", router->mudlist_id, i3fread_number(fp));
            break;

        case 'N':
            KEY("Name", router->name, i3fread_line(fp));
            break;

        case 'P':
            KEY("Port", router->port, i3fread_number(fp));
            KEY("Password", router->password, i3fread_number(fp));
            break;
        }
        if (!fMatch)
            log_error("i3_readrouter: no match: %s", word);
    }
}

bool I3_load_routers(void)
{
    FILE *fp;
    ROUTER_DATA *router;

    first_router = last_router = NULL;

    log_info("%s", "Loading I3 router data...");

    if (!(fp = fopen(I3_ROUTER_FILE, "r")))
    {
        log_info("%s", "No router data found.");
        return FALSE;
    }

    for (;;)
    {
        char letter;
        char *word;

        letter = i3fread_letter(fp);
        if (letter == '*')
        {
            i3fread_to_eol(fp);
            continue;
        }

        if (letter != '#')
        {
            log_error("%s", "i3_load_routers: # not found.");
            break;
        }

        word = i3fread_word(fp);
        if (!strcasecmp(word, "ROUTER"))
        {
            I3CREATE(router, ROUTER_DATA, 1);

            I3_readrouter(router, fp);
            if (!router->name || router->name[0] == '\0' || !router->ip || router->ip[0] == '\0' || router->port <= 0)
            {
                I3STRFREE(router->name);
                I3STRFREE(router->ip);
                I3DISPOSE(router);
            }
            else
                I3LINK(router, first_router, last_router, next, prev);
            continue;
        }
        else if (!strcasecmp(word, "END"))
            break;
        else
        {
            log_error("i3_load_routers: bad section: %s.", word);
            continue;
        }
    }
    I3FCLOSE(fp);

    if (!first_router || !last_router)
        return FALSE;

    return TRUE;
}

ROUTER_DATA *i3_find_router(const char *name)
{
    ROUTER_DATA *router;

    for (router = first_router; router; router = router->next)
    {
        if (!strcasecmp(router->name, name))
            return router;
    }
    return NULL;
}

void I3_readucache(UCACHE_DATA *user, FILE *fp)
{
    const char *word;
    bool fMatch;

    for (;;)
    {
        word = feof(fp) ? "End" : i3fread_word(fp);
        fMatch = FALSE;

        switch (UPPER(word[0]))
        {
        case '*':
            fMatch = TRUE;
            i3fread_to_eol(fp);
            break;

        case 'N':
            KEY("Name", user->name, i3fread_line(fp));
            break;

        case 'S':
            KEY("Sex", user->gender, i3fread_number(fp));
            break;

        case 'T':
            KEY("Time", user->time, i3fread_number(fp));
            break;

        case 'E':
            if (!strcasecmp(word, "End"))
                return;
            break;
        }
        if (!fMatch)
            log_error("I3_readucache: no match: %s", word);
    }
}

void I3_load_ucache(void)
{
    FILE *fp;
    UCACHE_DATA *user;

    first_ucache = last_ucache = NULL;

    log_info("%s", "Loading ucache data...");

    if (!(fp = fopen(I3_UCACHE_FILE, "r")))
    {
        log_info("%s", "No ucache data found.");
        return;
    }

    for (;;)
    {
        char letter;
        char *word;

        letter = i3fread_letter(fp);
        if (letter == '*')
        {
            i3fread_to_eol(fp);
            continue;
        }

        if (letter != '#')
        {
            log_error("%s", "I3_load_ucahe: # not found.");
            break;
        }

        word = i3fread_word(fp);
        if (!strcasecmp(word, "UCACHE"))
        {
            I3CREATE(user, UCACHE_DATA, 1);

            I3_readucache(user, fp);
            I3LINK(user, first_ucache, last_ucache, next, prev);
            continue;
        }
        else if (!strcasecmp(word, "END"))
            break;
        else
        {
            log_error("I3_load_ucache: bad section: %s.", word);
            continue;
        }
    }
    I3FCLOSE(fp);
}

void setup_i3_tables(void)
{
    setup_i3_config_table();
    setup_routers_table();
}

void setup_i3_config_table(void)
{
    PGresult *res = NULL;
    ExecStatusType st = (ExecStatusType) 0;
    const char *sql = "CREATE TABLE IF NOT EXISTS i3_config ( "
                // General config values
                "    updated TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT now(), "
                "    thismud TEXT NOT NULL, "
                "    autoconnect BOOLEAN NOT NULL DEFAULT false, "
                "    telnet TEXT, "
                "    web TEXT, "
                "    adminemail TEXT, "
                "    openstatus TEXT, "
                "    mudtype TEXT, "
                "    mudlib TEXT, "
                "    minlevel INTEGER NOT NULL DEFAULT 0, "
                "    immlevel INTEGER NOT NULL DEFAULT 0, "
                "    adminlevel INTEGER NOT NULL DEFAULT 0, "
                "    implevel INTEGER NOT NULL DEFAULT 0, "
                // Services, true means supported
                "    tell BOOLEAN NOT NULL DEFAULT false, "
                "    beep BOOLEAN NOT NULL DEFAULT false, "
                "    emoteto BOOLEAN NOT NULL DEFAULT false, "
                "    who BOOLEAN NOT NULL DEFAULT false, "
                "    finger BOOLEAN NOT NULL DEFAULT false, "
                "    locate BOOLEAN NOT NULL DEFAULT false, "
                "    channel BOOLEAN NOT NULL DEFAULT false, "
                "    news BOOLEAN NOT NULL DEFAULT false, "
                "    mail BOOLEAN NOT NULL DEFAULT false, "
                "    file BOOLEAN NOT NULL DEFAULT false, "
                "    auth BOOLEAN NOT NULL DEFAULT false, "
                "    ucache BOOLEAN NOT NULL DEFAULT false, "
                // Port numbers, 0 means not supported
                "    smtp INTEGER NOT NULL DEFAULT 0, "
                "    ftp INTEGER NOT NULL DEFAULT 0, "
                "    nntp INTEGER NOT NULL DEFAULT 0, "
                "    http INTEGER NOT NULL DEFAULT 0, "
                "    pop3 INTEGER NOT NULL DEFAULT 0, "
                "    rcp INTEGER NOT NULL DEFAULT 0, "
                "    amrcp INTEGER NOT NULL DEFAULT 0 "
                "); ";

    sql_connect(&db_wileymud);
    res = PQexec(db_wileymud.dbc, sql);
    st = PQresultStatus(res);
    if (st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE)
    {
        log_fatal("Cannot create i3_config table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);
}

void save_i3_config(void)
{
    PGresult *res = NULL;
    ExecStatusType st = (ExecStatusType) 0;
    const char *nuke_sql = "TRUNCATE i3_config;";
    const char *sql = "INSERT INTO i3_config ( "
                // General config values
                "    thismud, "
                "    autoconnect, "
                "    telnet, "
                "    web, "
                "    adminemail, "
                "    openstatus, "
                "    mudtype, "
                "    mudlib, "
                "    minlevel, "
                "    immlevel, "
                "    adminlevel, "
                "    implevel, "
                // Services, true means supported
                "    tell, "
                "    beep, "
                "    emoteto, "
                "    who, "
                "    finger, "
                "    locate, "
                "    channel, "
                "    news, "
                "    mail, "
                "    file, "
                "    auth, "
                "    ucache, "
                // Port numbers, 0 means not supported
                "    smtp, "
                "    ftp, "
                "    nntp, "
                "    http, "
                "    pop3, "
                "    rcp, "
                "    amrcp"
                ") VALUES ("
                // General config values
                "    $1, $2::BOOLEAN, $3, $4, $5, $6, $7, $8, "
                "    $9, $10, $11, $12, "
                // Services, true means supported
                "    $13::BOOLEAN, $14::BOOLEAN, $15::BOOLEAN, $16::BOOLEAN, "
                "    $17::BOOLEAN, $18::BOOLEAN, $19::BOOLEAN, $20::BOOLEAN, "
                "    $21::BOOLEAN, $22::BOOLEAN, $23::BOOLEAN, $24::BOOLEAN, "
                // Port numbers, 0 means not supported
                "    $25, $26, $27, $28, $29, $30, $31 "
                "); ";

    const char *param_val[31];
    int param_len[31];
    int param_bin[31] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0
    };
    char minlevel[MAX_INPUT_LENGTH];
    char immlevel[MAX_INPUT_LENGTH];
    char adminlevel[MAX_INPUT_LENGTH];
    char implevel[MAX_INPUT_LENGTH];
    char smtp[MAX_INPUT_LENGTH];
    char ftp[MAX_INPUT_LENGTH];
    char nntp[MAX_INPUT_LENGTH];
    char http[MAX_INPUT_LENGTH];
    char pop3[MAX_INPUT_LENGTH];
    char rcp[MAX_INPUT_LENGTH];
    char amrcp[MAX_INPUT_LENGTH];

    param_val[0] = (this_i3mud->name[0]) ? this_i3mud->name : NULL;
    param_len[0] = (this_i3mud->name[0]) ? strlen(this_i3mud->name) : 0;
    param_val[1] = (this_i3mud->autoconnect) ? "1": "0";
    param_len[1] = 1;
    param_val[2] = (this_i3mud->telnet[0]) ? this_i3mud->telnet : NULL;
    param_len[2] = (this_i3mud->telnet[0]) ? strlen(this_i3mud->telnet) : 0;
    param_val[3] = (this_i3mud->web[0]) ? this_i3mud->web : NULL;
    param_len[3] = (this_i3mud->web[0]) ? strlen(this_i3mud->web) : 0;
    param_val[4] = (this_i3mud->admin_email[0]) ? this_i3mud->admin_email : NULL;
    param_len[4] = (this_i3mud->admin_email[0]) ? strlen(this_i3mud->admin_email) : 0;
    param_val[5] = (this_i3mud->open_status[0]) ? this_i3mud->open_status : NULL;
    param_len[5] = (this_i3mud->open_status[0]) ? strlen(this_i3mud->open_status) : 0;
    param_val[6] = (this_i3mud->mud_type) ? this_i3mud->mud_type : NULL;
    param_len[6] = (this_i3mud->mud_type) ? strlen(this_i3mud->mud_type) : 0;
    param_val[7] = (this_i3mud->mudlib) ? this_i3mud->mudlib : NULL;
    param_len[7] = (this_i3mud->mudlib) ? strlen(this_i3mud->mudlib) : 0;

    snprintf(minlevel, MAX_INPUT_LENGTH, "%d", this_i3mud->minlevel);
    param_val[8] = (this_i3mud->minlevel > 0) ? minlevel: "0";
    param_len[8] = (this_i3mud->minlevel > 0) ? strlen(minlevel) : 1;
    snprintf(immlevel, MAX_INPUT_LENGTH, "%d", this_i3mud->immlevel);
    param_val[9] = (this_i3mud->immlevel > 0) ? immlevel: "0";
    param_len[9] = (this_i3mud->immlevel > 0) ? strlen(immlevel) : 1;
    snprintf(adminlevel, MAX_INPUT_LENGTH, "%d", this_i3mud->adminlevel);
    param_val[10] = (this_i3mud->adminlevel > 0) ? adminlevel: "0";
    param_len[10] = (this_i3mud->adminlevel > 0) ? strlen(adminlevel) : 1;
    snprintf(implevel, MAX_INPUT_LENGTH, "%d", this_i3mud->implevel);
    param_val[11] = (this_i3mud->implevel > 0) ? implevel: "0";
    param_len[11] = (this_i3mud->implevel > 0) ? strlen(implevel) : 1;

    param_val[12] = (this_i3mud->tell) ? "1": "0";
    param_len[12] = 1;
    param_val[13] = (this_i3mud->beep) ? "1": "0";
    param_len[13] = 1;
    param_val[14] = (this_i3mud->emoteto) ? "1": "0";
    param_len[14] = 1;
    param_val[15] = (this_i3mud->who) ? "1": "0";
    param_len[15] = 1;
    param_val[16] = (this_i3mud->finger) ? "1": "0";
    param_len[16] = 1;
    param_val[17] = (this_i3mud->locate) ? "1": "0";
    param_len[17] = 1;
    param_val[18] = (this_i3mud->channel) ? "1": "0";
    param_len[18] = 1;
    param_val[19] = (this_i3mud->news) ? "1": "0";
    param_len[19] = 1;
    param_val[20] = (this_i3mud->mail) ? "1": "0";
    param_len[20] = 1;
    param_val[21] = (this_i3mud->file) ? "1": "0";
    param_len[21] = 1;
    param_val[22] = (this_i3mud->auth) ? "1": "0";
    param_len[22] = 1;
    param_val[23] = (this_i3mud->ucache) ? "1": "0";
    param_len[23] = 1;

    snprintf(smtp, MAX_INPUT_LENGTH, "%d", this_i3mud->smtp);
    param_val[24] = (this_i3mud->smtp > 0) ? smtp: "0";
    param_len[24] = (this_i3mud->smtp > 0) ? strlen(smtp) : 1;
    snprintf(ftp, MAX_INPUT_LENGTH, "%d", this_i3mud->ftp);
    param_val[25] = (this_i3mud->ftp > 0) ? ftp: "0";
    param_len[25] = (this_i3mud->ftp > 0) ? strlen(ftp) : 1;
    snprintf(nntp, MAX_INPUT_LENGTH, "%d", this_i3mud->nntp);
    param_val[26] = (this_i3mud->nntp > 0) ? nntp: "0";
    param_len[26] = (this_i3mud->nntp > 0) ? strlen(nntp) : 1;
    snprintf(http, MAX_INPUT_LENGTH, "%d", this_i3mud->http);
    param_val[27] = (this_i3mud->http > 0) ? http: "0";
    param_len[27] = (this_i3mud->http > 0) ? strlen(http) : 1;
    snprintf(pop3, MAX_INPUT_LENGTH, "%d", this_i3mud->pop3);
    param_val[28] = (this_i3mud->pop3 > 0) ? pop3: "0";
    param_len[28] = (this_i3mud->pop3 > 0) ? strlen(pop3) : 1;
    snprintf(rcp, MAX_INPUT_LENGTH, "%d", this_i3mud->rcp);
    param_val[29] = (this_i3mud->rcp > 0) ? rcp: "0";
    param_len[29] = (this_i3mud->rcp > 0) ? strlen(rcp) : 1;
    snprintf(amrcp, MAX_INPUT_LENGTH, "%d", this_i3mud->amrcp);
    param_val[30] = (this_i3mud->amrcp > 0) ? amrcp: "0";
    param_len[30] = (this_i3mud->amrcp > 0) ? strlen(amrcp) : 1;

    res = PQexec(db_wileymud.dbc, nuke_sql);
    st = PQresultStatus(res);
    if (st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE)
    {
        log_fatal("Cannot empty i3 config table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);

    res = PQexecParams(db_wileymud.dbc, sql, 31, NULL, param_val, param_len, param_bin, 0);
    st = PQresultStatus(res);
    if (st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE)
    {
        log_fatal("Cannot update i3 config table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);
}

int load_i3_config(int mudport)
{
    time_t file_timestamp = -1;
    time_t sql_timestamp = -1;

    PGresult *res = NULL;
    ExecStatusType st = (ExecStatusType) 0;
    const char *sql = "SELECT extract('epoch' FROM updated) AS updated, "
                // General config values
                "    thismud, "
                "    autoconnect::INTEGER, "
                "    telnet, "
                "    web, "
                "    adminemail, "
                "    openstatus, "
                "    mudtype, "
                "    mudlib, "
                "    minlevel, "
                "    immlevel, "
                "    adminlevel, "
                "    implevel, "
                // Services, true means supported
                "    tell::INTEGER, "
                "    beep::INTEGER, "
                "    emoteto::INTEGER, "
                "    who::INTEGER, "
                "    finger::INTEGER, "
                "    locate::INTEGER, "
                "    channel::INTEGER, "
                "    news::INTEGER, "
                "    mail::INTEGER, "
                "    file::INTEGER, "
                "    auth::INTEGER, "
                "    ucache::INTEGER, "
                // Port numbers, 0 means not supported
                "    smtp, "
                "    ftp, "
                "    nntp, "
                "    http, "
                "    pop3, "
                "    rcp, "
                "    amrcp "
                " FROM i3_config ORDER BY updated DESC LIMIT 1;";
    int rows = 0;
    int columns = 0;

    const char *sql_time = "SELECT extract('epoch' FROM updated) AS updated "
        "FROM i3_config "
        "WHERE updated = ( "
        "   SELECT MAX(updated) FROM i3_config "
        ") "
        "LIMIT 1;";
    // overkill, we really should never have more than 1 row anyways...

    file_timestamp = file_date(I3_CONFIG_FILE);

    sql_connect(&db_wileymud);
    res = PQexec(db_wileymud.dbc, sql_time);
    st = PQresultStatus(res);
    if (st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE)
    {
        log_error("i3_config table has no data: %s", PQerrorMessage(db_wileymud.dbc));
        //PQclear(res);
        //proper_exit(MUD_HALT);
    }
    else
    {
        rows = PQntuples(res);
        columns = PQnfields(res);
        if (rows > 0 && columns > 0)
        {
            sql_timestamp = (time_t)atoi(PQgetvalue(res, 0, 0));
        }
    }
    PQclear(res);

    // At this point, we know the state.  If both are -1, we bail entirely.
    // If sql is -1, we must insert the file data
    // If file is -1, we just use the sql
    // If both are numbers, we only used the file if it's newer, and then
    // update the sql to match.
    if (file_timestamp == -1 && sql_timestamp == -1)
    {
        log_fatal("No I3 configuration data available!");
        proper_exit(MUD_HALT);
    }

    if (file_timestamp > sql_timestamp)
    {
        // load via the old code
        log_boot("  Using I3 configuration from file.");
        if (!I3_read_config(mudport))
        {
            I3_socket = -1;
            return 0;
        }
        else
        {
            save_i3_config();
        }
        return 1;
    }
    else
    {
        log_boot("  Using I3 configuration from SQL database.");
        log_boot("  Not really, it seems to be broken, FIX ME!");
        if (!I3_read_config(mudport))
        {
            I3_socket = -1;
            return 0;
        }
        else
        {
            save_i3_config();
        }
        return 1;
    }


    // FIXME

    res = PQexec(db_wileymud.dbc, sql);
    st = PQresultStatus(res);
    if (st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE)
    {
        log_fatal("Cannot get i3 settings from i3_config table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }

    rows = PQntuples(res);
    columns = PQnfields(res);
    if (rows > 0 && columns > 31)
    {
        log_boot("  Loading i3 configuration data from SQL database.");
    }
    else
    {
        log_fatal("Invalid result set from i3_config table!");
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);

    if (this_i3mud != NULL)
        destroy_I3_mud(this_i3mud);
    this_i3mud = NULL;

    this_i3mud = create_I3_mud();
    this_i3mud->player_port = mudport; /* Passed in from the mud's startup script */

    I3STRFREE(this_i3mud->name);
    this_i3mud->name = I3STRALLOC(PQgetvalue(res, 0, 1));
    this_i3mud->autoconnect = (int)atoi(PQgetvalue(res, 0, 2));
    I3STRFREE(this_i3mud->telnet);
    this_i3mud->telnet = I3STRALLOC(PQgetvalue(res, 0, 3));
    I3STRFREE(this_i3mud->web);
    this_i3mud->web = I3STRALLOC(PQgetvalue(res, 0, 4));
    I3STRFREE(this_i3mud->admin_email);
    this_i3mud->admin_email = I3STRALLOC(PQgetvalue(res, 0, 5));
    I3STRFREE(this_i3mud->open_status);
    this_i3mud->open_status = I3STRALLOC(PQgetvalue(res, 0, 6));

    {
        char lib_buf[MAX_STRING_LENGTH];

        //this_i3mud->mud_type = I3STRALLOC(PQgetvalue(res, 0, 7));
        //this_i3mud->mudlib = I3STRALLOC(PQgetvalue(res, 0, 8));

        //We actually have this hard coded...
        I3STRFREE(this_i3mud->mud_type);
        this_i3mud->mud_type = I3STRALLOC(CODETYPE);

        snprintf(lib_buf, MAX_STRING_LENGTH, "%s %s", CODEBASE, CODEVERSION);
        I3STRFREE(this_i3mud->base_mudlib);
        this_i3mud->base_mudlib = I3STRALLOC(lib_buf);
        // In our case, these are always the same
        I3STRFREE(this_i3mud->mudlib);
        this_i3mud->mudlib = I3STRALLOC(lib_buf);

        // Also hard coded
        I3STRFREE(this_i3mud->driver);
        this_i3mud->driver = I3STRALLOC(I3DRIVER);
    }

    this_i3mud->minlevel = (int)atoi(PQgetvalue(res, 0, 9));
    this_i3mud->immlevel = (int)atoi(PQgetvalue(res, 0, 10));
    this_i3mud->adminlevel = (int)atoi(PQgetvalue(res, 0, 11));
    this_i3mud->implevel = (int)atoi(PQgetvalue(res, 0, 12));

    this_i3mud->tell = (int)atoi(PQgetvalue(res, 0, 13));
    this_i3mud->beep = (int)atoi(PQgetvalue(res, 0, 14));
    this_i3mud->emoteto = (int)atoi(PQgetvalue(res, 0, 15));
    this_i3mud->who = (int)atoi(PQgetvalue(res, 0, 16));
    this_i3mud->finger = (int)atoi(PQgetvalue(res, 0, 17));
    this_i3mud->locate = (int)atoi(PQgetvalue(res, 0, 18));
    this_i3mud->channel = (int)atoi(PQgetvalue(res, 0, 19));
    this_i3mud->news = (int)atoi(PQgetvalue(res, 0, 20));
    this_i3mud->mail = (int)atoi(PQgetvalue(res, 0, 21));
    this_i3mud->file = (int)atoi(PQgetvalue(res, 0, 22));
    this_i3mud->auth = (int)atoi(PQgetvalue(res, 0, 23));
    this_i3mud->ucache = (int)atoi(PQgetvalue(res, 0, 24));

    this_i3mud->smtp = (int)atoi(PQgetvalue(res, 0, 25));
    this_i3mud->ftp = (int)atoi(PQgetvalue(res, 0, 26));
    this_i3mud->nntp = (int)atoi(PQgetvalue(res, 0, 27));
    this_i3mud->http = (int)atoi(PQgetvalue(res, 0, 28));
    this_i3mud->pop3 = (int)atoi(PQgetvalue(res, 0, 29));
    this_i3mud->rcp = (int)atoi(PQgetvalue(res, 0, 30));
    this_i3mud->amrcp = (int)atoi(PQgetvalue(res, 0, 31));

    // In the file version, we save both routers and config
    if (first_router)
    {
        //I3_saverouters();
        //I3_saveconfig();
    }

    return 0;
}

void setup_routers_table(void)
{
    PGresult *res = NULL;
    ExecStatusType st = (ExecStatusType) 0;
    const char *sql = "CREATE TABLE IF NOT EXISTS routers ( "
                "    updated TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT now(), "
                "    name TEXT PRIMARY KEY NOT NULL, "
                "    ip TEXT NOT NULL, "
                "    port INTEGER NOT NULL, "
                "    password INTEGER NOT NULL DEFAULT 0, "
                "    mudlist_id INTEGER NOT NULL DEFAULT 0, "
                "    chanlist_id INTEGER NOT NULL DEFAULT 0, "
                "    selected BOOLEAN NOT NULL DEFAULT false "
                "); ";

    sql_connect(&db_wileymud);
    res = PQexec(db_wileymud.dbc, sql);
    st = PQresultStatus(res);
    if (st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE)
    {
        log_fatal("Cannot create routers table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);
}

void save_routers(void)
{
    PGresult *res = NULL;
    ExecStatusType st = (ExecStatusType) 0;
    const char *sql = "INSERT INTO routers ( "
                "    name, "
                "    ip, "
                "    port, "
                "    password, "
                "    mudlist_id, "
                "    chanlist_id, "
                "    selected "
                ") VALUES ("
                "    $1, $2, $3, $4, $5, $6, $7::BOOLEAN "
                ") "
                "ON CONFLICT (name) "
                "DO UPDATE SET "
                "    ip = EXCLUDED.ip, "
                "    port = EXCLUDED.port, "
                "    password = EXCLUDED.password, "
                "    mudlist_id = EXCLUDED.mudlist_id, "
                "    chanlist_id = EXCLUDED.chanlist_id, "
                "    selected = EXCLUDED.selected, "
                "    updated = now(); ";
    const char *param_val[7];
    int param_len[7];
    int param_bin[7] = {
        0, 0, 0, 0, 0, 0, 0
    };
    char port[MAX_INPUT_LENGTH];
    char password[MAX_INPUT_LENGTH];
    char mudlist_id[MAX_INPUT_LENGTH];
    char chanlist_id[MAX_INPUT_LENGTH];
    ROUTER_DATA *router;

    for (router = first_router; router; router = router->next)
    {
        *port = '\0';
        *password = '\0';
        *mudlist_id = '\0';
        *chanlist_id = '\0';

        param_val[0] = (router->name[0]) ? router->name : NULL;
        param_len[0] = (router->name[0]) ? strlen(router->name) : 0;
        param_val[1] = (router->ip[0]) ? router->ip : NULL;
        param_len[1] = (router->ip[0]) ? strlen(router->ip) : 0;
        snprintf(port, MAX_INPUT_LENGTH, "%d", router->port);
        param_val[2] = (router->port > 0) ? port: "0";
        param_len[2] = (router->port > 0) ? strlen(port) : 1;
        snprintf(password, MAX_INPUT_LENGTH, "%d", router->password);
        param_val[3] = (router->port > 0) ? password: "0";
        param_len[3] = (router->port > 0) ? strlen(password) : 1;
        snprintf(mudlist_id, MAX_INPUT_LENGTH, "%d", router->mudlist_id);
        param_val[4] = (router->mudlist_id > 0) ? mudlist_id: "0";
        param_len[4] = (router->mudlist_id > 0) ? strlen(mudlist_id) : 1;
        snprintf(chanlist_id, MAX_INPUT_LENGTH, "%d", router->chanlist_id);
        param_val[5] = (router->chanlist_id > 0) ? chanlist_id: "0";
        param_len[5] = (router->chanlist_id > 0) ? strlen(chanlist_id) : 1;
        param_val[6] = (router == first_router) ? "1": "0";
        param_len[6] = (router == first_router) ? 1 : 1;

        res = PQexecParams(db_wileymud.dbc, sql, 7, NULL, param_val, param_len, param_bin, 0);
        st = PQresultStatus(res);
        if (st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE)
        {
            log_fatal("Cannot update routers table: %s", PQerrorMessage(db_wileymud.dbc));
            PQclear(res);
            proper_exit(MUD_HALT);
        }
        PQclear(res);
    }
}

// NOT FINISHED YET, use I3_load_routers still...
void load_routers(int mudport)
{
    time_t file_timestamp = -1;
    time_t sql_timestamp = -1;

    PGresult *res = NULL;
    ExecStatusType st = (ExecStatusType) 0;

    const char *sql = "SELECT extract('epoch' FROM updated) AS updated, "
                "    name, "
                "    ip, "
                "    port, "
                "    password, "
                "    mudlist_id, "
                "    chanlist_id, "
                "    selected::INTEGER "
                " FROM routers ORDER BY selected, updated DESC;";
    int rows = 0;
    int columns = 0;

    const char *sql_time = "SELECT extract('epoch' FROM updated) AS updated "
        "FROM routers "
        "WHERE updated = ( "
        "   SELECT MAX(updated) FROM routers "
        ") "
        "LIMIT 1;";

    file_timestamp = file_date(I3_CONFIG_FILE);

    sql_connect(&db_wileymud);
    res = PQexec(db_wileymud.dbc, sql_time);
    st = PQresultStatus(res);
    if (st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE)
    {
        log_error("router table has no data: %s", PQerrorMessage(db_wileymud.dbc));
        //PQclear(res);
        //proper_exit(MUD_HALT);
    }
    else
    {
        rows = PQntuples(res);
        columns = PQnfields(res);
        if (rows > 0 && columns > 0)
        {
            sql_timestamp = (time_t)atoi(PQgetvalue(res, 0, 0));
        }
    }
    PQclear(res);

    // At this point, we know the state.  If both are -1, we bail entirely.
    // If sql is -1, we must insert the file data
    // If file is -1, we just use the sql
    // If both are numbers, we only used the file if it's newer, and then
    // update the sql to match.
    if (file_timestamp == -1 && sql_timestamp == -1)
    {
        log_fatal("No router configuration data available!");
        proper_exit(MUD_HALT);
    }

    if (file_timestamp > sql_timestamp)
    {
        // load via the old code
        I3_load_routers();
        save_routers();
        log_boot("  Using router configuration from file.");
        return;
    }

    log_boot("  Using router configuration from SQL database.");

    res = PQexec(db_wileymud.dbc, sql);
    st = PQresultStatus(res);
    if (st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK)
    {
        log_fatal("Cannot get router list from routers table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }

    // Now loop through the results and push them into the old data structure


}

void I3_saveconfig(void)
{
    FILE *fp;

    if (!(fp = fopen(I3_CONFIG_FILE, "w")))
    {
        log_info("%s", "Couldn't write to i3.config file.");
        return;
    }

    fprintf(fp, "%s", "$I3CONFIG\n\n");
    fprintf(fp, "%s", "# This file will now allow you to use tildes.\n");
    fprintf(fp, "%s", "# Set autoconnect to 1 to automatically connect at bootup.\n");
    fprintf(fp, "%s", "# This information can be edited online using 'i3config'\n");
    fprintf(fp, "thismud      %s\n", this_i3mud->name);
    fprintf(fp, "autoconnect  %d\n", this_i3mud->autoconnect);
    fprintf(fp, "telnet       %s\n", this_i3mud->telnet);
    fprintf(fp, "web          %s\n", this_i3mud->web);
    fprintf(fp, "adminemail   %s\n", this_i3mud->admin_email);
    fprintf(fp, "openstatus   %s\n", this_i3mud->open_status);
    if (this_i3mud->mud_type)
        fprintf(fp, "mudtype      %s\n", this_i3mud->mud_type);
    if (this_i3mud->mudlib && strcasecmp(this_i3mud->mudlib, this_i3mud->base_mudlib))
        fprintf(fp, "mudlib       %s\n", this_i3mud->mudlib);
    fprintf(fp, "minlevel     %d\n", this_i3mud->minlevel);
    fprintf(fp, "immlevel     %d\n", this_i3mud->immlevel);
    fprintf(fp, "adminlevel   %d\n", this_i3mud->adminlevel);
    fprintf(fp, "implevel     %d\n\n", this_i3mud->implevel);

    fprintf(fp, "%s", "\n# Information below this point cannot be edited online.\n");
    fprintf(fp, "%s", "# The services provided by your mud.\n");
    fprintf(fp, "%s", "# Do not turn things on unless you KNOW your mud properly supports them!\n");
    fprintf(fp, "%s",
            "# Refer to http://cie.imaginary.com/protocols/intermud3.html for public packet specifications.\n");
    fprintf(fp, "tell         %d\n", this_i3mud->tell);
    fprintf(fp, "beep         %d\n", this_i3mud->beep);
    fprintf(fp, "emoteto      %d\n", this_i3mud->emoteto);
    fprintf(fp, "who          %d\n", this_i3mud->who);
    fprintf(fp, "finger       %d\n", this_i3mud->finger);
    fprintf(fp, "locate       %d\n", this_i3mud->locate);
    fprintf(fp, "channel      %d\n", this_i3mud->channel);
    fprintf(fp, "news         %d\n", this_i3mud->news);
    fprintf(fp, "mail         %d\n", this_i3mud->mail);
    fprintf(fp, "file         %d\n", this_i3mud->file);
    fprintf(fp, "auth         %d\n", this_i3mud->auth);
    fprintf(fp, "ucache       %d\n\n", this_i3mud->ucache);
    fprintf(fp, "%s", "# Port numbers for OOB services. Leave as 0 if your mud does not support these.\n");
    fprintf(fp, "smtp         %d\n", this_i3mud->smtp);
    fprintf(fp, "ftp          %d\n", this_i3mud->ftp);
    fprintf(fp, "nntp         %d\n", this_i3mud->nntp);
    fprintf(fp, "http         %d\n", this_i3mud->http);
    fprintf(fp, "pop3         %d\n", this_i3mud->pop3);
    fprintf(fp, "rcp          %d\n", this_i3mud->rcp);
    fprintf(fp, "amrcp        %d\n", this_i3mud->amrcp);
    fprintf(fp, "%s", "end\n");
    fprintf(fp, "%s", "$END\n");
    I3FCLOSE(fp);
    save_i3_config();
}

void I3_fread_config_file(FILE *fin)
{
    const char *word;
    bool fMatch;

    for (;;)
    {
        word = feof(fin) ? "end" : i3fread_word(fin);
        fMatch = FALSE;

        switch (word[0])
        {
        case '#':
            fMatch = TRUE;
            i3fread_to_eol(fin);
            break;

        case 'a':
            KEY("adminemail", this_i3mud->admin_email, i3fread_line(fin));
            KEY("adminlevel", this_i3mud->adminlevel, i3fread_number(fin));
            KEY("amrcp", this_i3mud->amrcp, i3fread_number(fin));
            KEY("auth", this_i3mud->auth, i3fread_number(fin));
            KEY("autoconnect", this_i3mud->autoconnect, i3fread_number(fin));
            break;

        case 'b':
            KEY("beep", this_i3mud->beep, i3fread_number(fin));
            break;

        case 'c':
            KEY("channel", this_i3mud->channel, i3fread_number(fin));
            break;

        case 'e':
            KEY("emoteto", this_i3mud->emoteto, i3fread_number(fin));
            if (!strcasecmp(word, "end"))
            {
                char lib_buf[MAX_STRING_LENGTH];

                /*
                 * Adjust base_mudlib information based on already supplied info (mud.h). -Orion
                 * Modified for AFKMud use by Samson.
                 */
                I3STRFREE(this_i3mud->mud_type);
                this_i3mud->mud_type = I3STRALLOC(CODETYPE);

                snprintf(lib_buf, MAX_STRING_LENGTH, "%s %s", CODEBASE, CODEVERSION);
                I3STRFREE(this_i3mud->base_mudlib);
                this_i3mud->base_mudlib = I3STRALLOC(lib_buf);

                if (!this_i3mud->mudlib || strcasecmp(this_i3mud->mudlib, this_i3mud->base_mudlib))
                {
                    if (this_i3mud->mudlib)
                        I3STRFREE(this_i3mud->mudlib);
                    this_i3mud->mudlib = I3STRALLOC(lib_buf);
                }

                I3STRFREE(this_i3mud->driver);
                this_i3mud->driver = I3STRALLOC(I3DRIVER);

                /*
                 * Convert to new router file
                 */
                if (first_router)
                {
                    I3_saverouters();
                    I3_saveconfig();
                }
                return;
            }
            break;

        case 'f':
            KEY("file", this_i3mud->file, i3fread_number(fin));
            KEY("finger", this_i3mud->finger, i3fread_number(fin));
            KEY("ftp", this_i3mud->ftp, i3fread_number(fin));
            break;

        case 'h':
            KEY("http", this_i3mud->http, i3fread_number(fin));
            break;

        case 'i':
            KEY("immlevel", this_i3mud->immlevel, i3fread_number(fin));
            KEY("implevel", this_i3mud->implevel, i3fread_number(fin));
            break;

        case 'l':
            KEY("locate", this_i3mud->locate, i3fread_number(fin));
            break;

        case 'm':
            KEY("mail", this_i3mud->mail, i3fread_number(fin));
            KEY("minlevel", this_i3mud->minlevel, i3fread_number(fin));
            KEY("mudlib", this_i3mud->mudlib, i3fread_line(fin));
            KEY("mudtype", this_i3mud->mud_type, i3fread_line(fin));
            break;

        case 'n':
            KEY("news", this_i3mud->news, i3fread_number(fin));
            KEY("nntp", this_i3mud->nntp, i3fread_number(fin));
            break;

        case 'o':
            KEY("openstatus", this_i3mud->open_status, i3fread_line(fin));
            break;

        case 'p':
            KEY("pop3", this_i3mud->pop3, i3fread_number(fin));
            break;

        case 'r':
            KEY("rcp", this_i3mud->rcp, i3fread_number(fin));
            /*
             * Router config loading is legacy support here - routers are configured in their own file now.
             */
            if (!strcasecmp(word, "router"))
            {
                ROUTER_DATA *router;
                char rname[MAX_INPUT_LENGTH], rip[MAX_INPUT_LENGTH], *ln;
                int rport;

                ln = i3fread_line(fin);
                sscanf(ln, "%s %s %d", rname, rip, &rport);

                I3CREATE(router, ROUTER_DATA, 1);

                router->name = I3STRALLOC(rname);
                router->ip = I3STRALLOC(rip);
                router->port = rport;
                router->reconattempts = 0;
                I3LINK(router, first_router, last_router, next, prev);
                fMatch = TRUE;
                I3DISPOSE(ln);
                break;
            }
            break;

        case 's':
            KEY("smtp", this_i3mud->smtp, i3fread_number(fin));
            break;

        case 't':
            KEY("tell", this_i3mud->tell, i3fread_number(fin));
            KEY("telnet", this_i3mud->telnet, i3fread_line(fin));
            KEY("thismud", this_i3mud->name, i3fread_line(fin));
            break;

        case 'u':
            KEY("ucache", this_i3mud->ucache, i3fread_number(fin));
            break;

        case 'w':
            KEY("web", this_i3mud->web, i3fread_line(fin));
            KEY("who", this_i3mud->who, i3fread_number(fin));
            break;
        }
        if (!fMatch)
            log_error("I3_fread_config_file: Bad keyword: %s\r\n", word);
    }
}

bool I3_read_config(int mudport)
{
    FILE *fin, *fp;

    if (this_i3mud != NULL)
        destroy_I3_mud(this_i3mud);
    this_i3mud = NULL;

    log_info("%s", "Loading Intermud-3 network data...");

    if (!(fin = fopen(I3_CONFIG_FILE, "r")))
    {
        log_info("%s", "Can't open configuration file: i3.config");
        log_info("%s", "Network configuration aborted.");
        return FALSE;
    }

    for (;;)
    {
        char letter;
        char *word;

        letter = i3fread_letter(fin);

        if (letter == '#')
        {
            i3fread_to_eol(fin);
            continue;
        }

        if (letter != '$')
        {
            log_error("%s", "I3_read_config: $ not found");
            break;
        }

        word = i3fread_word(fin);
        if (!strcasecmp(word, "I3CONFIG") && this_i3mud == NULL)
        {
            this_i3mud = create_I3_mud();
            this_i3mud->player_port = mudport; /* Passed in from the mud's startup script */
            I3_fread_config_file(fin);
            continue;
        }
        else if (!strcasecmp(word, "END"))
            break;
        else
        {
            log_error("I3_read_config: Bad section in config file: %s", word);
            continue;
        }
    }
    I3FCLOSE(fin);

    if (!this_i3mud)
    {
        log_info("%s", "Error during configuration load.");
        log_info("%s", "Network configuration aborted.");
        return FALSE;
    }

    if ((fp = fopen(I3_PASSWORD_FILE, "r")) != NULL && this_i3mud != NULL)
    {
        char *word;

        word = i3fread_word(fp);

        if (!strcasecmp(word, "#PASSWORD"))
        {
            char *ln = i3fread_line(fp);
            int pass, mud, chan;

            pass = mud = chan = 0;
            sscanf(ln, "%d %d %d", &pass, &mud, &chan);
            this_i3mud->password = pass;
            this_i3mud->mudlist_id = mud;
            this_i3mud->chanlist_id = chan;
            I3DISPOSE(ln);
        }
        I3FCLOSE(fp);
    }

    if (!this_i3mud->name || this_i3mud->name[0] == '\0')
    {
        log_info("%s", "Mud name not loaded in configuration file.");
        log_info("%s", "Network configuration aborted.");
        destroy_I3_mud(this_i3mud);
        return FALSE;
    }

    if (!this_i3mud->telnet || this_i3mud->telnet[0] == '\0')
        this_i3mud->telnet = I3STRALLOC("Address not configured");

    if (!this_i3mud->web || this_i3mud->web[0] == '\0')
        this_i3mud->web = I3STRALLOC("Address not configured");

    // I3_THISMUD = this_i3mud->name;
    save_i3_config();
    return TRUE;
}

void I3_readban(I3_BAN *ban, FILE *fin)
{
    const char *word;
    bool fMatch;

    for (;;)
    {
        word = feof(fin) ? "End" : i3fread_word(fin);
        fMatch = FALSE;

        switch (UPPER(word[0]))
        {
        case '*':
            fMatch = TRUE;
            i3fread_to_eol(fin);
            break;

        case 'N':
            KEY("Name", ban->name, i3fread_line(fin));
            break;

        case 'E':
            if (!strcasecmp(word, "End"))
                return;
            break;
        }
        if (!fMatch)
            log_error("I3_readban: no match: %s", word);
    }
}

void I3_loadbans(void)
{
    FILE *fin;
    I3_BAN *ban;

    first_i3ban = NULL;
    last_i3ban = NULL;

    log_info("%s", "Loading ban list...");

    if ((fin = fopen(I3_BAN_FILE, "r")) == NULL)
    {
        log_info("%s", "No ban list defined.");
        return;
    }

    for (;;)
    {
        char letter;
        char *word;

        letter = i3fread_letter(fin);
        if (letter == '*')
        {
            i3fread_to_eol(fin);
            continue;
        }

        if (letter != '#')
        {
            log_error("%s", "I3_loadbans: # not found.");
            break;
        }

        word = i3fread_word(fin);
        if (!strcasecmp(word, "I3BAN"))
        {
            I3CREATE(ban, I3_BAN, 1);

            I3_readban(ban, fin);
            if (!ban->name)
                I3DISPOSE(ban);
            else
                I3LINK(ban, first_i3ban, last_i3ban, next, prev);
            continue;
        }
        else if (!strcasecmp(word, "END"))
            break;
        else
        {
            log_error("I3_loadbans: bad section: %s.", word);
            continue;
        }
    }
    I3FCLOSE(fin);
}

void I3_write_bans(void)
{
    FILE *fout;
    I3_BAN *ban;

    if ((fout = fopen(I3_BAN_FILE, "w")) == NULL)
    {
        log_info("%s", "Couldn't write to ban list file.");
        return;
    }

    for (ban = first_i3ban; ban; ban = ban->next)
    {
        fprintf(fout, "%s", "#I3BAN\n");
        fprintf(fout, "Name   %s\n", ban->name);
        fprintf(fout, "%s", "End\n\n");
    }
    fprintf(fout, "%s", "#END\n");
    I3FCLOSE(fout);
}

void I3_readchannel(I3_CHANNEL *channel, FILE *fin)
{
    const char *word;
    bool fMatch;

    for (;;)
    {
        word = feof(fin) ? "End" : i3fread_word(fin);
        fMatch = FALSE;

        switch (UPPER(word[0]))
        {
        case '*':
            fMatch = TRUE;
            i3fread_to_eol(fin);
            break;

        case 'C':
            KEY("ChanName", channel->I3_name, i3fread_line(fin));
            KEY("ChanMud", channel->host_mud, i3fread_line(fin));
            KEY("ChanLocal", channel->local_name, i3fread_line(fin));
            KEY("ChanLayM", channel->layout_m, i3fread_line(fin));
            KEY("ChanLayE", channel->layout_e, i3fread_line(fin));
            KEY("ChanLevel", channel->i3perm, i3fread_number(fin));
            KEY("ChanStatus", channel->status, i3fread_number(fin));
            KEY("ChanFlags", channel->flags, i3fread_number(fin));
            break;

        case 'E':
            if (!strcasecmp(word, "End"))
            {
                /*
                 * Legacy support to convert channel permissions
                 */
                if (channel->i3perm > I3PERM_IMP)
                {
                    /*
                     * The I3PERM_NONE condition should realistically never happen....
                     */
                    if (channel->i3perm < this_i3mud->minlevel)
                        channel->i3perm = I3PERM_NONE;
                    else if (channel->i3perm >= this_i3mud->minlevel && channel->i3perm < this_i3mud->immlevel)
                        channel->i3perm = I3PERM_MORT;
                    else if (channel->i3perm >= this_i3mud->immlevel && channel->i3perm < this_i3mud->adminlevel)
                        channel->i3perm = I3PERM_IMM;
                    else if (channel->i3perm >= this_i3mud->adminlevel && channel->i3perm < this_i3mud->implevel)
                        channel->i3perm = I3PERM_ADMIN;
                    else if (channel->i3perm >= this_i3mud->implevel)
                        channel->i3perm = I3PERM_IMP;
                }
                return;
            }
            break;
        }
        if (!fMatch)
            log_error("I3_readchannel: no match: %s", word);
    }
}

void I3_loadchannels(void)
{
    FILE *fin;
    I3_CHANNEL *channel;

    first_I3chan = last_I3chan = NULL;

    log_info("%s", "Loading channels...");

    if (!(fin = fopen(I3_CHANNEL_FILE, "r")))
    {
        log_info("%s", "No channel config file found.");
        return;
    }

    for (;;)
    {
        char letter;
        char *word;

        letter = i3fread_letter(fin);
        if (letter == '*')
        {
            i3fread_to_eol(fin);
            continue;
        }

        if (letter != '#')
        {
            log_error("%s", "I3_loadchannels: # not found.");
            break;
        }

        word = i3fread_word(fin);
        if (!strcasecmp(word, "I3CHAN"))
        {
            int x;

            I3CREATE(channel, I3_CHANNEL, 1);

            I3_readchannel(channel, fin);

            if (channel->local_name && !strcmp(channel->local_name, "(null)"))
            {
                I3STRFREE(channel->local_name);
                channel->local_name = NULL;
            }
            if (channel->layout_m && !strcmp(channel->layout_m, "(null)"))
            {
                I3STRFREE(channel->layout_m);
                channel->layout_m = I3STRALLOC("[%s] %s@%s: %%^RESET%%^%s");
            }
            if (channel->layout_e && !strcmp(channel->layout_e, "(null)"))
            {
                I3STRFREE(channel->layout_e);
                channel->layout_e = I3STRALLOC("[%s] %%^RESET%%^%s");
            }
            for (x = 0; x < MAX_I3HISTORY; x++)
                channel->history[x] = NULL;
            I3LINK(channel, first_I3chan, last_I3chan, next, prev);
            continue;
        }
        else if (!strcasecmp(word, "END"))
            break;
        else
        {
            log_error("I3_loadchannels: bad section: %s.", word);
            continue;
        }
    }
    I3FCLOSE(fin);
}

void I3_write_channel_config(void)
{
    FILE *fout;
    I3_CHANNEL *channel;

    if ((fout = fopen(I3_CHANNEL_FILE, "w")) == NULL)
    {
        log_info("%s", "Couldn't write to channel config file.");
        return;
    }

    for (channel = first_I3chan; channel; channel = channel->next)
    {
        // if( channel->local_name )
        // {

        // Skip stupid channels with empty names or leading whitespace
        if (!channel->I3_name || !channel->I3_name[0] || isspace(channel->I3_name[0]))
            continue;
        if (!channel->host_mud || !channel->host_mud[0] || isspace(channel->host_mud[0]))
            continue;

        fprintf(fout, "%s", "#I3CHAN\n");
        fprintf(fout, "ChanName   %s\n", channel->I3_name);
        fprintf(fout, "ChanMud    %s\n", channel->host_mud);
        fprintf(fout, "ChanLocal  %s\n", channel->local_name);
        fprintf(fout, "ChanLayM   %s\n", channel->layout_m);
        fprintf(fout, "ChanLayE   %s\n", channel->layout_e);
        fprintf(fout, "ChanLevel  %d\n", channel->i3perm);
        fprintf(fout, "ChanStatus %d\n", channel->status);
        fprintf(fout, "ChanFlags  %ld\n", (long int)channel->flags);
        fprintf(fout, "%s", "End\n\n");
        // }
    }
    fprintf(fout, "%s", "#END\n");
    I3FCLOSE(fout);
}

/* Used only during copyovers */
void fread_mudlist(FILE *fin, I3_MUD *mud)
{
    const char *word;
    char *ln;
    bool fMatch;
    int x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12;

    for (;;)
    {
        word = feof(fin) ? "End" : i3fread_word(fin);
        fMatch = FALSE;

        switch (UPPER(word[0]))
        {
        case '*':
            fMatch = TRUE;
            i3fread_to_eol(fin);
            break;

        case 'B':
            KEY("Banner", mud->banner, i3fread_line(fin));
            KEY("Baselib", mud->base_mudlib, i3fread_line(fin));
            break;

        case 'D':
            KEY("Daemon", mud->daemon, i3fread_line(fin));
            KEY("Driver", mud->driver, i3fread_line(fin));
            break;

        case 'E':
            KEY("Email", mud->admin_email, i3fread_line(fin));
            if (!strcasecmp(word, "End"))
            {
                return;
            }

        case 'I':
            KEY("IP", mud->ipaddress, i3fread_line(fin));
            break;

        case 'M':
            KEY("Mudlib", mud->mudlib, i3fread_line(fin));
            break;

        case 'O':
            KEY("Openstatus", mud->open_status, i3fread_line(fin));
            if (!strcasecmp(word, "OOBPorts"))
            {
                ln = i3fread_line(fin);
                x1 = x2 = x3 = x4 = x5 = x6 = x7 = 0;

                sscanf(ln, "%d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7);
                mud->smtp = x1;
                mud->ftp = x2;
                mud->nntp = x3;
                mud->http = x4;
                mud->pop3 = x5;
                mud->rcp = x6;
                mud->amrcp = x7;
                fMatch = TRUE;
                I3DISPOSE(ln);
                break;
            }
            break;

        case 'P':
            if (!strcasecmp(word, "Ports"))
            {
                ln = i3fread_line(fin);
                x1 = x2 = x3 = 0;

                sscanf(ln, "%d %d %d ", &x1, &x2, &x3);
                mud->player_port = x1;
                mud->imud_tcp_port = x2;
                mud->imud_udp_port = x3;
                fMatch = TRUE;
                I3DISPOSE(ln);
                break;
            }
            break;

        case 'S':
            KEY("Status", mud->status, i3fread_number(fin));
            if (!strcasecmp(word, "Services"))
            {
                ln = i3fread_line(fin);
                x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = x9 = x10 = x11 = x12 = 0;

                sscanf(ln, "%d %d %d %d %d %d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8, &x9, &x10,
                       &x11, &x12);
                mud->tell = x1;
                mud->beep = x2;
                mud->emoteto = x3;
                mud->who = x4;
                mud->finger = x5;
                mud->locate = x6;
                mud->channel = x7;
                mud->news = x8;
                mud->mail = x9;
                mud->file = x10;
                mud->auth = x11;
                mud->ucache = x12;
                fMatch = TRUE;
                I3DISPOSE(ln);
                break;
            }
            break;

        case 'T':
            KEY("Telnet", mud->telnet, i3fread_line(fin));
            KEY("Time", mud->time, i3fread_line(fin));
            KEY("Type", mud->mud_type, i3fread_line(fin));
            break;

        case 'W':
            KEY("Web", mud->web, i3fread_line(fin));
            break;
        }

        if (!fMatch)
            log_error("I3_readmudlist: no match: %s", word);
    }
}

/* Called only during copyovers */
void I3_loadmudlist(void)
{
    FILE *fin;
    I3_MUD *mud;

    if (!(fin = fopen(I3_MUDLIST_FILE, "r")))
        return;

    for (;;)
    {
        char letter;
        const char *word;

        letter = i3fread_letter(fin);
        if (letter == '*')
        {
            i3fread_to_eol(fin);
            continue;
        }

        if (letter != '#')
        {
            log_error("%s", "I3_loadmudlist: # not found.");
            break;
        }

        word = i3fread_word(fin);
        if (!strcasecmp(word, "ROUTER"))
        {
            I3STRFREE(this_i3mud->routerName);
            this_i3mud->routerName = i3fread_line(fin);
            I3_ROUTER_NAME = this_i3mud->routerName;
            continue;
        }
        if (!strcasecmp(word, "MUDLIST"))
        {
            word = i3fread_word(fin);
            if (!strcasecmp(word, "Name"))
            {
                char *tmpname;

                tmpname = i3fread_line(fin);
                mud = new_I3_mud(tmpname);
                fread_mudlist(fin, mud);
                I3STRFREE(tmpname);
            }
            else
            {
                log_error("%s", "fread_mudlist: No mudname saved, skipping entry.");
                i3fread_to_eol(fin);
                for (;;)
                {
                    word = feof(fin) ? "End" : i3fread_word(fin);
                    if (strcasecmp(word, "End"))
                        i3fread_to_eol(fin);
                    else
                        break;
                }
            }
            continue;
        }
        else if (!strcasecmp(word, "END"))
            break;
        else
        {
            log_error("I3_loadmudlist: bad section: %s.", word);
            continue;
        }
    }
    I3FCLOSE(fin);
    unlink(I3_MUDLIST_FILE);
}

/* Called only during copyovers */
void I3_loadchanlist(void)
{
    FILE *fin;
    I3_CHANNEL *channel;

    if (!(fin = fopen(I3_CHANLIST_FILE, "r")))
        return;

    for (;;)
    {
        char letter;
        char *word;

        letter = i3fread_letter(fin);
        if (letter == '*')
        {
            i3fread_to_eol(fin);
            continue;
        }

        if (letter != '#')
        {
            log_error("%s", "I3_loadchanlist: # not found.");
            break;
        }

        word = i3fread_word(fin);
        if (!strcasecmp(word, "I3CHAN"))
        {
            int x;
            I3CREATE(channel, I3_CHANNEL, 1);

            I3_readchannel(channel, fin);

            for (x = 0; x < MAX_I3HISTORY; x++)
                channel->history[x] = NULL;
            I3LINK(channel, first_I3chan, last_I3chan, next, prev);
            continue;
        }
        else if (!strcasecmp(word, "END"))
            break;
        else
        {
            log_error("I3_loadchanlist: bad section: %s.", word);
            continue;
        }
    }
    I3FCLOSE(fin);
    unlink(I3_CHANLIST_FILE);
}

/* Called only during copyovers */
void I3_savemudlist(void)
{
    FILE *fp;
    I3_MUD *mud;

    if (!(fp = fopen(I3_MUDLIST_FILE, "w")))
    {
        log_error("%s", "I3_savemudlist: Unable to write to mudlist file.");
        return;
    }

    fprintf(fp, "#ROUTER %s\n", I3_ROUTER_NAME);
    for (mud = first_mud; mud; mud = mud->next)
    {
        // Don't store muds that are down, who cares? They'll update themselves anyway
        if (mud->status == 0)
            continue;

        // Skip stupid muds with empty names or leading whitespace
        if (!mud->name || !mud->name[0] || isspace(mud->name[0]))
            continue;

        fprintf(fp, "%s", "#MUDLIST\n");
        fprintf(fp, "Name		%s\n", mud->name);
        fprintf(fp, "Status		%d\n", mud->status);
        fprintf(fp, "IP			%s\n", mud->ipaddress);
        fprintf(fp, "Mudlib		%s\n", mud->mudlib);
        fprintf(fp, "Baselib		%s\n", mud->base_mudlib);
        fprintf(fp, "Driver		%s\n", mud->driver);
        fprintf(fp, "Type		%s\n", mud->mud_type);
        fprintf(fp, "Openstatus	%s\n", mud->open_status);
        fprintf(fp, "Email		%s\n", mud->admin_email);
        if (mud->telnet)
            fprintf(fp, "Telnet		%s\n", mud->telnet);
        if (mud->web)
            fprintf(fp, "Web		%s\n", mud->web);
        if (mud->banner)
            fprintf(fp, "Banner		%s\n", mud->banner);
        if (mud->daemon)
            fprintf(fp, "Dameon		%s\n", mud->daemon);
        if (mud->time)
            fprintf(fp, "Time		%s\n", mud->time);
        fprintf(fp, "Ports %d %d %d\n", mud->player_port, mud->imud_tcp_port, mud->imud_udp_port);
        fprintf(fp, "Services %d %d %d %d %d %d %d %d %d %d %d %d\n", mud->tell, mud->beep, mud->emoteto, mud->who,
                mud->finger, mud->locate, mud->channel, mud->news, mud->mail, mud->file, mud->auth, mud->ucache);
        fprintf(fp, "OOBports %d %d %d %d %d %d %d\n", mud->smtp, mud->ftp, mud->nntp, mud->http, mud->pop3, mud->rcp,
                mud->amrcp);
        fprintf(fp, "%s", "End\n\n");
    }
    fprintf(fp, "%s", "#END\n");
    I3FCLOSE(fp);
}

/* Called only during copyovers */
void I3_savechanlist(void)
{
    FILE *fp;
    I3_CHANNEL *channel;

    if (!(fp = fopen(I3_CHANLIST_FILE, "w")))
    {
        log_error("%s", "I3_savechanlist: Unable to write to chanlist file.");
        return;
    }

    for (channel = first_I3chan; channel; channel = channel->next)
    {
        // Don't save local channels, they are stored elsewhere
        if (channel->local_name)
            continue;

        // Skip stupid channels with empty names or leading whitespace
        if (!channel->I3_name || !channel->I3_name[0] || isspace(channel->I3_name[0]))
            continue;

        if (!channel->host_mud || !channel->host_mud[0] || isspace(channel->host_mud[0]))
            continue;

        fprintf(fp, "%s", "#I3CHAN\n");
        fprintf(fp, "ChanMud		%s\n", channel->host_mud);
        fprintf(fp, "ChanName		%s\n", channel->I3_name);
        fprintf(fp, "ChanStatus	%d\n", channel->status);
        fprintf(fp, "%s", "End\n\n");
    }
    fprintf(fp, "%s", "#END\n");
    I3FCLOSE(fp);
}

/* Used during copyovers */
void I3_loadhistory(void)
{
    char filename[MAX_INPUT_LENGTH];
    FILE *tempfile;
    I3_CHANNEL *tempchan = NULL;
    int x;

    for (tempchan = first_I3chan; tempchan; tempchan = tempchan->next)
    {
        if (!tempchan->local_name)
            continue;

        snprintf(filename, MAX_INPUT_LENGTH, "%s%s.hist", I3_DIR, tempchan->local_name);

        if (!(tempfile = fopen(filename, "r")))
            continue;

        for (x = 0; x < MAX_I3HISTORY; x++)
        {
            if (feof(tempfile))
                tempchan->history[x] = NULL;
            else
                tempchan->history[x] = i3fread_line(tempfile);
        }
        I3FCLOSE(tempfile);
        unlink(filename);
    }
}

/* Used during copyovers */
void I3_savehistory(void)
{
    char filename[MAX_INPUT_LENGTH];
    FILE *tempfile;
    I3_CHANNEL *tempchan = NULL;
    int x;

    for (tempchan = first_I3chan; tempchan; tempchan = tempchan->next)
    {
        if (!tempchan->local_name)
            continue;

        if (!tempchan->history[0])
            continue;

        snprintf(filename, MAX_INPUT_LENGTH, "%s%s.hist", I3_DIR, tempchan->local_name);

        if (!(tempfile = fopen(filename, "w")))
            continue;

        for (x = 0; x < MAX_I3HISTORY; x++)
        {
            if (tempchan->history[x] != NULL)
                fprintf(tempfile, "%s\n", tempchan->history[x]);
        }
        I3FCLOSE(tempfile);
    }
}

/*
 * Setup a TCP session to the router. Returns socket or <0 if failed.
 *
 */
int I3_connection_open(ROUTER_DATA *router)
{
    struct sockaddr_in sa;
    struct hostent *hostp;
    int x = 1;

    log_info("Attempting connect to %s on port %d", router->ip, router->port);

    I3_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (I3_socket < 0)
    {
        log_info("%s", "Cannot create socket!");
        I3_connection_close(TRUE);
        return -1;
    }

    if ((x = fcntl(I3_socket, F_GETFL, 0)) < 0)
    {
        log_info("%s", "I3_connection_open: fcntl(F_GETFL)");
        I3_connection_close(TRUE);
        return -1;
    }

    if (fcntl(I3_socket, F_SETFL, x | O_NONBLOCK) < 0)
    {
        log_info("%s", "I3_connection_open: fcntl(F_SETFL)");
        I3_connection_close(TRUE);
        return -1;
    }

    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;

    // 45.64.56.66
    if (!inet_aton(router->ip, &sa.sin_addr))
    {
        // kelly.irn.themud.org
        hostp = gethostbyname(router->ip);
        if (!hostp)
        {
            log_info("%s", "I3_connection_open: Cannot resolve router hostname.");
            I3_connection_close(TRUE);
            return -1;
        }
        memcpy(&sa.sin_addr, hostp->h_addr, hostp->h_length);
    }

    sa.sin_port = htons(router->port);

    if (connect(I3_socket, (struct sockaddr *)&sa, sizeof(sa)) < 0)
    {
        if (errno != EINPROGRESS)
        {
            log_info("I3_connection_open: Unable to connect to router %s", router->name);
            I3_connection_close(TRUE);
            return -1;
        }
    }
    I3_ROUTER_NAME = router->name;
    I3_ROUTER_IP = router->ip;
    log_info("Connected to Intermud-3 router %s", router->name);
    return I3_socket;
}

/*
 * Close the socket to the router.
 */
void I3_connection_close(bool reconnect)
{
    ROUTER_DATA *router = NULL;
    bool rfound = FALSE;

    I3_input_pointer = 0;
    I3_output_pointer = 4;
    bzero(I3_input_buffer, IPS);
    bzero(I3_output_buffer, OPS);
    bzero(I3_currentpacket, IPS);
    I3_packet_cleanup();

    // Mark ourselves as NOT connected
    connect_started_at = 0;
    connected_at = 0;

    for (router = first_router; router; router = router->next)
    {
        if (!strcasecmp(router->name, I3_ROUTER_NAME))
        {
            rfound = TRUE;
            break;
        }
    }

    if (!rfound)
    {
        log_info("I3_connection_close: Disconnecting from router (%s).", I3_ROUTER_NAME);
        if (I3_socket > 0)
        {
            close(I3_socket);
            I3_socket = -1;
        }
        return;
    }

    log_info("Closing connection to Intermud-3 router %s", router->name);
    if (I3_socket > 0)
    {
        close(I3_socket);
        I3_socket = -1;
    }

    if (reconnect)
    {
        if (router->reconattempts <= 3)
        {
            i3wait = router_reconnect_short_delay;
            log_info("%s", "Will attempt to reconnect in approximately 15 seconds.");
        }
        else if (router->reconattempts <= 6)
        {
            i3wait = router_reconnect_medium_delay;
            log_info("%s", "Will attempt to reconnect in approximately 1 minute.");
        }
        else if (router->reconattempts <= 9)
        {
            i3wait = router_reconnect_long_delay;
            log_info("%s", "Will attempt to reconnect in approximately 5 minutes.");
        }
        else if (router->next != NULL)
        {
            log_info("Unable to reach %s. Abandoning connection.", router->name);
            log_info("Bytes sent: %ld. Bytes received: %ld.", bytes_sent, bytes_received);
            bytes_sent = 0;
            bytes_received = 0;
            channel_m_sent = 0;
            channel_m_received = 0;
            i3wait = router_reconnect_short_delay;
            router->reconattempts = 0;
            router = router->next;
            log_info("Will attempt new connection to %s in approximately 15 seconds.", router->name);
        }
        else
        {
            log_info("Unable to reach %s. Abandoning connection.", router->name);
            log_info("Bytes sent: %ld. Bytes received: %ld.", bytes_sent, bytes_received);
            bytes_sent = 0;
            bytes_received = 0;
            channel_m_sent = 0;
            channel_m_received = 0;
            i3wait = router_reconnect_short_delay;
            router->reconattempts = 0;
            router = first_router;
            log_info("Will attempt new connection to %s in approximately 15 seconds.", router->name);
        }
    }
    log_info("Bytes sent: %ld. Bytes received: %ld.", bytes_sent, bytes_received);
    bytes_sent = 0;
    bytes_received = 0;
    channel_m_sent = 0;
    channel_m_received = 0;
}

/* Free up all the data lists once the connection is down. No sense in wasting memory on it. */
void free_i3data(bool complete)
{
    I3_MUD *mud, *next_mud;
    I3_CHANNEL *channel, *next_chan;
    I3_BAN *ban, *next_ban;
    UCACHE_DATA *ucache, *next_ucache;
    ROUTER_DATA *router, *router_next;
    I3_CMD_DATA *cmd, *cmd_next;
    I3_ALIAS *alias, *alias_next;
    I3_HELP_DATA *help, *help_next;
    I3_COLOR *color, *color_next;

    if (first_i3ban)
    {
        for (ban = first_i3ban; ban; ban = next_ban)
        {
            next_ban = ban->next;
            I3STRFREE(ban->name);
            I3UNLINK(ban, first_i3ban, last_i3ban, next, prev);
            I3DISPOSE(ban);
        }
    }

    if (first_I3chan)
    {
        for (channel = first_I3chan; channel; channel = next_chan)
        {
            next_chan = channel->next;
            destroy_I3_channel(channel);
        }
    }

    if (first_mud)
    {
        for (mud = first_mud; mud; mud = next_mud)
        {
            next_mud = mud->next;
            destroy_I3_mud(mud);
        }
    }

    if (first_ucache)
    {
        for (ucache = first_ucache; ucache; ucache = next_ucache)
        {
            next_ucache = ucache->next;
            I3STRFREE(ucache->name);
            I3UNLINK(ucache, first_ucache, last_ucache, next, prev);
            I3DISPOSE(ucache);
        }
    }

    if (complete == TRUE)
    {
        for (router = first_router; router; router = router_next)
        {
            router_next = router->next;
            I3STRFREE(router->name);
            I3STRFREE(router->ip);
            I3UNLINK(router, first_router, last_router, next, prev);
            I3DISPOSE(router);
        }

        for (cmd = first_i3_command; cmd; cmd = cmd_next)
        {
            cmd_next = cmd->next;

            for (alias = cmd->first_alias; alias; alias = alias_next)
            {
                alias_next = alias->next;

                I3STRFREE(alias->name);
                I3UNLINK(alias, cmd->first_alias, cmd->last_alias, next, prev);
                I3DISPOSE(alias);
            }
            I3STRFREE(cmd->name);
            I3UNLINK(cmd, first_i3_command, last_i3_command, next, prev);
            I3DISPOSE(cmd);
        }

        for (help = first_i3_help; help; help = help_next)
        {
            help_next = help->next;
            I3STRFREE(help->name);
            I3STRFREE(help->text);
            I3UNLINK(help, first_i3_help, last_i3_help, next, prev);
            I3DISPOSE(help);
        }

        for (color = first_i3_color; color; color = color_next)
        {
            color_next = color->next;
            I3STRFREE(color->name);
            I3STRFREE(color->mudtag);
            I3STRFREE(color->imctag);
            I3STRFREE(color->i3tag);
            I3UNLINK(color, first_i3_color, last_i3_color, next, prev);
            I3DISPOSE(color);
        }
    }
}

/*
 * Shutdown the connection to the router.
 */
void i3_shutdown(int delay, CHAR_DATA *ch)
{
    if (I3_socket < 1)
        return;

    I3_savehistory();
    free_i3data(FALSE);

    /*
     * Flush the outgoing buffer
     */
    if (I3_output_pointer != 4)
        I3_write_packet(I3_output_buffer);

    I3_send_shutdown(delay);
    I3_connection_close(FALSE);
    I3_input_pointer = 0;
    I3_output_pointer = 4;
    I3_save_id();
    usleep(200000); /* Never lag the MUD for a full 2 seconds! 0.2 is acceptable. */
    if (ch)
        i3_printf(ch, "Intermud-3 router connection closed.\r\n");
}

/*
 * Connect to the router and send the startup-packet.
 * Mud port is passed in from main() so that the information passed along to the I3
 * network regarding the mud's operational port is now determined by the mud's own
 * startup script instead of the I3 config file.
 */
void router_connect(const char *router_name, bool forced, int mudport, bool isconnected)
{
    ROUTER_DATA *router;
    bool rfound = FALSE;

    bytes_sent = 0;
    bytes_received = 0;
    channel_m_sent = 0;
    channel_m_received = 0;

    manual_router = router_name;

    /*
     * The Command table is required for operation. Like.... duh?
     */
    if (first_i3_command == NULL)
    {
        if (!I3_load_commands())
        {
            log_info("%s", "router_connect: Unable to load command table!");
            I3_socket = -1;
            return;
        }
    }

    //if (!I3_read_config(mudport))
    if (!load_i3_config(mudport))
    {
        I3_socket = -1;
        return;
    }

    if (first_router == NULL)
    {
        if (!I3_load_routers())
        {
            log_info("%s", "router_connect: No router configurations were found!");
            I3_socket = -1;
            return;
        }
        I3_ROUTER_NAME = first_router->name;
        I3_ROUTER_IP = first_router->ip;
    }

    /*
     * Help information should persist even when the network is not connected...
     */
    if (first_i3_help == NULL)
        I3_load_helps();

    /*
     * ... as should the color table.
     */
    if (first_i3_color == NULL)
        I3_load_color_table();

    if ((!this_i3mud->autoconnect && !forced && !isconnected) || (isconnected && I3_socket < 1))
    {
        log_info("Intermud-3 network data loaded. Autoconnect not set. Will need to connect manually.");
        I3_socket = -1;
        return;
    }
    else
    {
        char timeout_msg[MAX_INPUT_LENGTH];

        *timeout_msg = '\0';
        log_info("%s", "Intermud-3 network data loaded. Initialiazing network connection...");
        snprintf(timeout_msg, MAX_INPUT_LENGTH, "Connecting to Intermud-3 router (%s)", I3_ROUTER_NAME);
        allchan_log(0, (char *)"wiley", (char *)"Cron", (char *)"Cron", (char *)"WileyMUD", (char *)timeout_msg);
    }

    I3_loadchannels();
    I3_loadbans();

    I3_loadSpeakers();

    if (this_i3mud->ucache == TRUE)
    {
        I3_load_ucache();
        I3_prune_ucache();
        ucache_clock = i3_time + 86400;
    }

    if (I3_socket < 1)
    {
        for (router = first_router; router; router = router->next)
        {
            if (router_name && strcasecmp(router_name, router->name))
                continue;

            if (router->reconattempts <= 9)
            {
                rfound = TRUE;
                I3_socket = I3_connection_open(router);
                break;
            }
        }
    }

    if (!rfound && !isconnected)
    {
        log_info("%s", "Unable to connect. No available routers found.");
        allchan_log(0, (char *)"wiley", (char *)"Cron", (char *)"Cron", (char *)"WileyMUD",
                    (char *)"%^RED%^Not connected to I3.  No routers available.%^RESET%^");
        i3_message_to_players((char *)"\r\n%^RED%^I3 is down again.  No routers available.%^RESET%^");
        I3_socket = -1;
        return;
    }

    if (I3_socket < 1)
    {
        i3wait = 100;
        return;
    }

    usleep(100000);

    log_info("%s", "Intermud-3 Network initialized.");

    if (!isconnected)
    {
        I3_startup_packet();
        expecting_timeout = 1;
        timeout_marker = getTimestamp() + I3_TIMEOUT_DELAY;
    }
    else
    {
        I3_loadmudlist();
        I3_loadchanlist();
    }
    I3_loadhistory();
}

/* Wrapper for router_connect now - so we don't need to force older client installs to adjust. */
void i3_startup(bool forced, int mudport, bool isconnected)
{
    time_to_taunt = getTimestamp() + I3_TAUNT_DELAY;
    // i3_nuke_url_file();

    log_boot("I3 Startup beginning!");
    // NOTE: I3_loadPinkfishTo___() are called from sql_startup() in sql.c now.

    if (I3_read_config(mudport))
        router_connect(NULL, forced, mudport, isconnected);
    else
        log_error("i3_startup: %s", "Configuration failed!");
}

int do_taunt_from_log(void)
{
    FILE *fp = NULL;
    int taunt_count = 0;
    char line[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0\0";
    char taunt[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0\0";
    int taunt_pid = 0;
    int i = 0;
    int taunt_selection = 0;
    char year[5] = "\0\0\0\0";
    char month[3] = "\0\0";
    char day[3] = "\0\0";
    char hour[3] = "\0\0";
    char minute[3] = "\0\0";
    char second[3] = "\0\0";
    char speaker[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0\0";
    char message[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0\0";
    char *ss = NULL;
    char *se = NULL;
    char *t = NULL;

    taunt_pid = fork();
    if (taunt_pid == 0)
    {
        // We're up!  Do the needful!
        if (!(fp = fopen(I3_ALLCHAN_LOG, "r")))
        {
            log_error("Cannot open I3 log file: %s!", I3_ALLCHAN_LOG);
            exit(1);
        }
        else
        {
            while (fgets(line, MAX_STRING_LENGTH - 2, fp))
            {
                taunt_count++;
            }
            rewind(fp);
            taunt_selection = random() % taunt_count;
            for (i = taunt_selection; i > 0; i--)
            {
                fgets(line, MAX_STRING_LENGTH - 2, fp);
            }
            fclose(fp);
            /* 2009.09.21-12.10,28000	imud_gossip	Sinistrad@Dead Souls Dev	f	Not out yet tho */
            strncpy(year, &line[0], 4);
            strncpy(month, &line[5], 2);
            strncpy(day, &line[8], 2);
            strncpy(hour, &line[11], 2);
            strncpy(minute, &line[14], 2);
            strncpy(second, &line[17], 2);
            // Skip to channel (throw away)
            ss = strchr(line, '\t');
            if (!ss || !*ss)
                return TRUE;
            ss++;

            // Skip to speaker (ss to se-2)
            ss = strchr(ss, '\t');
            if (!ss || !*ss)
                return TRUE;
            ss++;

            // Skip to is_emote (se to t-2)
            se = strchr(ss, '\t');
            if (!se || !*se)
                return TRUE;
            se++;

            // Get to message (t to EOL)
            t = strchr(se, '\t');
            if (!t || !*t)
                return TRUE;
            t++;

            bzero(speaker, MAX_STRING_LENGTH);
            strncpy(speaker, ss, (se - ss - 1));

            bzero(message, MAX_STRING_LENGTH);
            strncpy(message, t, MAX_STRING_LENGTH);

            snprintf(taunt, MAX_STRING_LENGTH,
                     "%%^RED%%^%%^BOLD%%^[%-s-%s-%s %s:%s]%%^RESET%%^ %%^YELLOW%%^%s%%^RESET%%^ once said "
                     "%%^GREEN%%^%%^BOLD%%^%s%%^RESET%%^",
                     year, month, day, hour, minute, speaker, message);
            i3_npc_speak("wiley", "Cron", taunt);
            exit(0);
        }
    }
    else
    {
        // Zombie patrol should be handled by ignoring SIGCHLD
    }
    return TRUE;
}

#define I3_TAUNT_FILE I3_DIR "i3.taunts"

char *i3_taunt_line()
{
    FILE *fp = NULL;
    struct stat fst;
    static int taunt_count = 0;
    static char **taunt_list = NULL;
    static int last_changed = 0;
    static int already_using_defaults = 0;
    int i = 0;
    char line[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0\0";

    if (stat(I3_TAUNT_FILE, &fst) != -1)
    {
        if (fst.st_mtime > last_changed)
        {
            /* File has been updated, so reload it */
            last_changed = fst.st_mtime;

            if (taunt_list)
            {
                for (i = 0; i < taunt_count; i++)
                {
                    if (taunt_list[i])
                    {
                        free(taunt_list[i]);
                        taunt_list[i] = NULL;
                    }
                }
                free(taunt_list);
                taunt_list = NULL;
                taunt_count = 0;
            }
            if (!(fp = fopen(I3_TAUNT_FILE, "r")))
            {
                log_error("Cannot open I3 taunt file: %s!", I3_TAUNT_FILE);
                already_using_defaults = 0;
                goto no_taunt_file;
            }
            else
            {
                while (fgets(line, MAX_STRING_LENGTH - 2, fp))
                {
                    taunt_count++;
                }
                rewind(fp);
                taunt_list = (char **)calloc(taunt_count, sizeof(char *));
                for (i = 0; i < taunt_count; i++)
                {
                    taunt_list[i] = (char *)strdup(fgets(line, MAX_STRING_LENGTH - 2, fp));
                }
                fclose(fp);
                already_using_defaults = 0;
            }
        }
    }
    else
    {
    no_taunt_file:
        /* No file, so use a small set of built-in taunts. */
        if (!already_using_defaults)
        {
            if (taunt_list)
            {
                for (i = 0; i < taunt_count; i++)
                {
                    if (taunt_list[i])
                    {
                        free(taunt_list[i]);
                        taunt_list[i] = NULL;
                    }
                }
                free(taunt_list);
                taunt_list = NULL;
                taunt_count = 0;
            }

            taunt_count = 10;
            taunt_list = (char **)calloc(taunt_count, sizeof(char *));
            taunt_list[0] = strdup("Ummmm.. go away, we already got one.");
            taunt_list[1] = strdup("Connection closed by foreign host");
            taunt_list[2] = strdup("NO CARRIER");
            taunt_list[3] = strdup("I wish this connection would stay open.");
            taunt_list[4] = strdup("I hate you!");
            taunt_list[5] = strdup("Why will you not die?");
            taunt_list[6] = strdup("WTF are you still doing here?");
            taunt_list[7] = strdup("I hate ALL of you!");
            taunt_list[8] = strdup("When I am dictator, things will run smoothly...");
            taunt_list[9] = strdup("SUFFER!  You will all SUFFER!");
            already_using_defaults = 1;
        }
    }

    /* We should have taunt data now */
    return taunt_list[number(0, taunt_count - 1)];
}

#define I3_USERMAP_FILE I3_DIR "i3.usermap"

char *I3_nameremap(const char *ps)
{
    FILE *fp = NULL;
    struct stat fst;
    static int map_count = 0;
    static char **key_list = NULL;
    static char **value_list = NULL;
    static int last_changed = 0;
    int i = 0;
    char line[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0\0";
    static char remapped[MAX_STRING_LENGTH];
    char *s;

    strcpy(remapped, ps);

    if (stat(I3_USERMAP_FILE, &fst) != -1)
    {
        if (fst.st_mtime > last_changed)
        {
            /* File has been updated, so reload it */
            last_changed = fst.st_mtime;

            if (key_list)
            {
                for (i = 0; i < map_count; i++)
                {
                    if (key_list[i])
                    {
                        free(key_list[i]);
                        key_list[i] = NULL;
                    }
                }
                free(key_list);
                key_list = NULL;
            }
            if (value_list)
            {
                for (i = 0; i < map_count; i++)
                {
                    if (value_list[i])
                    {
                        free(value_list[i]);
                        value_list[i] = NULL;
                    }
                }
                free(value_list);
                value_list = NULL;
            }
            map_count = 0;

            if (!(fp = fopen(I3_USERMAP_FILE, "r")))
            {
                log_error("Cannot open I3 usermap file: %s!", I3_USERMAP_FILE);
                return remapped;
            }
            else
            {
                while (fgets(line, MAX_STRING_LENGTH - 2, fp))
                {
                    map_count++;
                }
                rewind(fp);
                key_list = (char **)calloc(map_count, sizeof(char *));
                value_list = (char **)calloc(map_count, sizeof(char *));
                for (i = 0; i < map_count; i++)
                {
                    s = i3fread_word(fp);
                    if (s && *s)
                    {
                        key_list[i] = strdup(s);
                        s = i3fread_rest_of_line(fp);
                        if (s && *s)
                        {
                            value_list[i] = strdup(s);
                        }
                        else
                        {
                            value_list[i] = strdup(key_list[i]);
                        }
                    }
                    else
                    {
                        // We have to put something here to avoid NULL pointers.
                        key_list[i] = strdup("INVALID_NAME");
                        // If the key wasn't valid, the value probably can't be either.
                        // I guess just skip to the next and hope.
                        value_list[i] = strdup("INVALID_NAME");
                    }
                }
                fclose(fp);
            }
        }
    }

    if (map_count > 0)
    {
        for (i = 0; i < map_count; i++)
        {
            if (strcasecmp(key_list[i], ps) == 0)
            {
                strcpy(remapped, value_list[i]);
                break;
            }
        }
    }

    return remapped;
}

void i3_nuke_url_file()
{
    struct stat fst;

    if (stat(I3_URL_DUMP, &fst) != -1)
    {
        truncate(I3_URL_DUMP, 0);
    }
}

void i3_check_urls()
{
    FILE *fp = NULL;
    struct stat fst;
    char line[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0\0";
    static int last_changed = 0;
    int i = 0;
    int j = 0;
    // int              x = 0;

    if (stat(I3_URL_DUMP, &fst) != -1)
    {
        if (fst.st_mtime > last_changed)
        {
            log_info("URL file is ready to process!");
            /* File has been updated, so reload it */
            last_changed = fst.st_mtime;

            if (!(fp = fopen(I3_URL_DUMP, "r")))
            {
                log_error("No URL DUMP file: %s!", I3_URL_DUMP);
            }
            else
            {
                while (fgets(line, MAX_STRING_LENGTH - 2, fp))
                {
                    /*
                    // Remove trailing newlines, if any
                    while(*line && ((x = strlen(line)) > 0)) {
                        if(ISNEWL(line[x-1])) {
                            line[x-1] = '\0';
                        }
                    }
                    // If anything is left, add a proper newline
                    if(*line && x > 0 && !ISNEWL(line[x-1])) {
                        line[x-1] = '\r';
                        line[x] = '\n';
                        line[x+1] = '\0';
                    }
                    if(*line || strlen(line) < 3) {
                        // Nothing but the newline is here, skip it
                        continue;
                    }
                    */
                    i++;
                    if (*line)
                    {
                        i3_npc_speak("url", "URLbot", line);
                        j++;
                    }
                }
                fclose(fp);
                fp = NULL;
                truncate(I3_URL_DUMP, 0);
                log_info("%d lines read, %d results sent, from URL file %s.", i, j, I3_URL_DUMP);
            }
        }
    }
}

void i3_log_alive()
{
    struct tm *tm_info = NULL;
    time_t tc = (time_t)0;
    char taunt[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";

    tc = time(0);
    tm_info = localtime(&tc);

    snprintf(taunt, MAX_STRING_LENGTH,
             "%%^RED%%^%%^BOLD%%^[%-4.4d-%-2.2d-%-2.2d %-2.2d:%-2.2d]%%^RESET%%^ %%^GREEN%%^%%^BOLD%%^%s "
             "(%s)%%^RESET%%^ %%^YELLOW%%^*** Welcome to WileyMUD III, Quixadhal's Version %s (%s) ***%%^RESET%%^",
             tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday, tm_info->tm_hour, tm_info->tm_min,
             "It's ALIVE!\r\n", I3_ROUTER_NAME, VERSION_BUILD, VERSION_DATE);
    i3_npc_speak("wiley", "Cron", taunt);
}

void i3_log_dead()
{
    struct tm *tm_info = NULL;
    time_t tc = (time_t)0;
    char taunt[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";

    tc = time(0);
    tm_info = localtime(&tc);

    snprintf(taunt, MAX_STRING_LENGTH,
             "%%^RED%%^%%^BOLD%%^[%-4.4d-%-2.2d-%-2.2d %-2.2d:%-2.2d]%%^RESET%%^ %%^RED%%^%%^BOLD%%^%s (%s)%%^RESET%%^ "
             "%%^YELLOW%%^*** Welcome to WileyMUD III, Quixadhal's Version %s (%s) ***%%^RESET%%^",
             tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday, tm_info->tm_hour, tm_info->tm_min,
             "It's going DOWN!\r\n", I3_ROUTER_NAME, VERSION_BUILD, VERSION_DATE);
    allchan_log(0, (char *)"wiley", (char *)"Cron", (char *)"Cron", (char *)"WileyMUD", taunt);
}

/*
 * This just makes sure we have a clean state to grab a new packet
 */
void I3_packet_cleanup(void)
{
    I3_packet_processing_state = I3_PACKET_STATE_NONE;
    I3_incoming_packet_length = 0;
    bzero(I3_incoming_packet_size, 4);
    if (I3_incoming_packet)
        free(I3_incoming_packet);
    I3_incoming_packet = NULL;
    I3_incoming_packet_read = 0;
    if (I3_incoming_packet_leftovers)
        free(I3_incoming_packet_leftovers);
    I3_incoming_packet_leftovers = NULL;
    I3_incoming_packet_strlen = 0;
}

/*
 * Check for a packet and if one available read it and parse it.
 * Also checks to see if the mud should attempt to reconnect to the router.
 * This is an input only loop. Attempting to use it to send buffered output
 * just wasn't working out, so output has gone back to sending packets to the
 * router as soon as they're assembled.
 */
void i3_loop(void)
{
    ROUTER_DATA *router;
    int ret;
    // long                                    size;
    fd_set in_set, out_set, exc_set;
    static struct timeval last_time;
    static struct timeval null_time;
    bool rfound = FALSE;

#if 0
    struct tm *tm_info = NULL;
    time_t tc = (time_t)0;
#endif

    gettimeofday(&last_time, NULL);
    i3_time = (time_t)last_time.tv_sec;

    FD_ZERO(&in_set);
    FD_ZERO(&out_set);
    FD_ZERO(&exc_set);

    if (i3wait > 0)
        i3wait--;

    if (expecting_timeout && diffTimestamp(timeout_marker, -1) <= 0)
    {
        char timeout_msg[MAX_INPUT_LENGTH];

        expecting_timeout = 0;
        *timeout_msg = '\0';
        log_info("I3 Client timeout (%s).", I3_ROUTER_NAME);
        snprintf(timeout_msg, MAX_INPUT_LENGTH, "%%^RED%%^I3 Client timeout (%s).%%^RESET%%^", I3_ROUTER_NAME);
        allchan_log(0, (char *)"wiley", (char *)"Cron", (char *)"Cron", (char *)"WileyMUD", (char *)timeout_msg);
        I3_connection_close(TRUE);
        connection_timeouts++;
        return;
    }

#if 0
    tc = time(0);
    tm_info = localtime(&tc);

    /* We reboot our router every Monday, Wedensday, and Friday, at 4:45AM.  This makes the I3
     * connection die, but we can't seem to recognize this, so bounce I3 at 5AM on those days.
     */
    if ((tm_info->tm_wday == 1) || (tm_info->tm_wday == 3) || (tm_info->tm_wday == 5)) {
        if ((tm_info->tm_hour == 5) && (tm_info->tm_min == 0) && (tm_info->tm_sec == 0)) {
	    log_info("I3 Client is rebooting for scheduled router reboot.");
            allchan_log(0,(char *)"wiley", (char *)"Cron", (char *)"Cron", (char *)"WileyMUD", (char *)"%^RED%^I3 Client is rebooting for scheduled router reboot.%^RESET%^");
            I3_connection_close(TRUE);
            return;
        }
    }
#endif

    /*
     * This condition can only occur if you were previously connected and the socket was closed.
     * * Tries 3 times, then attempts connection to an alternate router, if it has one.
     */
    if (i3wait == 1)
    {
        for (router = first_router; router; router = router->next)
        {
            if (manual_router && strcasecmp(router->name, manual_router))
                continue;

            if (router->reconattempts <= 9)
            {
                rfound = TRUE;
                break;
            }
        }

        if (!rfound)
        {
            i3wait = -2;
            log_info("%s", "I3 Unable to reconnect. No routers responding.");
            return;
        }
        I3_socket = I3_connection_open(router);
        if (I3_socket < 1)
        {
            if (router->reconattempts <= 3)
                i3wait = router_reconnect_short_delay;
            else if (router->reconattempts <= 6)
                i3wait = router_reconnect_medium_delay;
            else
                i3wait = router_reconnect_long_delay;
            return;
        }

        usleep(100000);

        log_info("Connection to Intermud-3 router %s %s.", router->name,
                 router->reconattempts > 0 ? "reestablished" : "established");
        router->reconattempts++;
        I3_startup_packet();
        expecting_timeout = 1;
        timeout_marker = getTimestamp() + I3_TIMEOUT_DELAY;
        return;
    }

    if (!is_connecting())
        return;

    if (i3justconnected)
    {
        i3justconnected = 0;
        i3_log_alive();
    }

    if (connected_at > 0)
    {
        uptime = i3_time - connected_at;
        if (uptime > record_uptime)
            record_uptime = uptime;
        // In theory, save this and load it to persist across reboots.
    }

    if (diffTimestamp(time_to_taunt, -1) <= 0)
    {
        time_to_taunt = getTimestamp() + I3_TAUNT_DELAY;
        i3_do_ping("Cron", "intergossip", "Dead Souls Dev");
        // This seems like a good time to check the I3 statistics too.
        i3_daily_summary();
        //piss_off_shentino();
        log_info("Next ping in %s", stringTimestamp(time_to_taunt - getTimestamp()));
    }

    // Will prune the cache once every 24hrs after bootup time
    if (ucache_clock <= i3_time)
    {
        ucache_clock = i3_time + 86400;
        I3_prune_ucache();
    }

    // Flush the output buffer?
    if (I3_output_pointer != 4)
        I3_write_packet(I3_output_buffer);

    FD_SET(I3_socket, &in_set);
    FD_SET(I3_socket, &out_set);
    FD_SET(I3_socket, &exc_set);

    if (select(I3_socket + 1, &in_set, &out_set, &exc_set, &null_time) < 0)
    {
        perror("i3_loop: select: Unable to poll I3_socket!");
        I3_connection_close(TRUE);
        return;
    }

    if (FD_ISSET(I3_socket, &exc_set))
    {
        char timeout_msg[MAX_INPUT_LENGTH];

        FD_CLR(I3_socket, &in_set);
        FD_CLR(I3_socket, &out_set);
        log_info("%s", "Exception raised on I3 socket.");
        *timeout_msg = '\0';
        snprintf(timeout_msg, MAX_INPUT_LENGTH, "Exception raised on I3 socket to (%s)", I3_ROUTER_NAME);
        allchan_log(0, (char *)"wiley", (char *)"Cron", (char *)"Cron", (char *)"WileyMUD", (char *)timeout_msg);
        I3_connection_close(TRUE);
        return;
    }

    if (FD_ISSET(I3_socket, &in_set))
    {
        /* An I3 packet (or part of one) has arrived */
        switch (I3_packet_processing_state)
        {
        case I3_PACKET_STATE_NONE:
            /* We want to grab the size first! */
            I3_packet_cleanup();
            ret = read(I3_socket, I3_incoming_packet_size, 4);
            if (!ret || (ret < 0 && errno != EAGAIN && errno != EWOULDBLOCK))
            {
                FD_CLR(I3_socket, &out_set);
                if (ret < 0)
                    log_info("Read error on I3 socket.");
                else
                    log_info("EOF encountered on I3 socket read.");
                I3_connection_close(TRUE);
                return;
            }
            if (ret < 0)
            { /* EAGAIN */
                return;
            }
            /* NOTE:  32-bit "int" is assumed here, C sucks, we use uint32_t to try and force it */
            memcpy(&I3_incoming_packet_length, I3_incoming_packet_size, 4);
            I3_incoming_packet_length = ntohl(I3_incoming_packet_length);
            I3_packet_processing_state = I3_PACKET_STATE_GOT_SIZE;
            /* break; */
        case I3_PACKET_STATE_GOT_SIZE:
            /* We now knwo the size we expect */
            if (I3_incoming_packet_length <= 0)
            {
                /* But it's invalid? */
                FD_CLR(I3_socket, &out_set);
                log_error("Invalid packet size, reboot the sucker!");
                I3_connection_close(TRUE);
                return;
            }
            I3_incoming_packet = (char *)calloc(I3_incoming_packet_length + 1, sizeof(char));
            I3_packet_processing_state = I3_PACKET_STATE_READING;
            /* break; */
        case I3_PACKET_STATE_READING:
            /* OK, we have a buffer ready, try to read it! */
            ret = read(I3_socket, I3_incoming_packet + I3_incoming_packet_read,
                       I3_incoming_packet_length - I3_incoming_packet_read);
            if (!ret || (ret < 0 && errno != EAGAIN && errno != EWOULDBLOCK))
            {
                FD_CLR(I3_socket, &out_set);
                if (ret < 0)
                    log_info("Read error on I3 socket.");
                else
                    log_info("EOF encountered on I3 socket read.");
                I3_connection_close(TRUE);
                return;
            }
            if (ret < 0)
            { /* EAGAIN */
                return;
            }
            if (ret < (I3_incoming_packet_length - I3_incoming_packet_read))
            {
                /* Still more to read! */
                I3_incoming_packet_read += ret;
                return;
            }
            I3_incoming_packet_read += ret;
            I3_packet_processing_state = I3_PACKET_STATE_GOT_PACKET;
            /* break; */
        case I3_PACKET_STATE_GOT_PACKET:
            /* Now we have a complete packet, according to the mudmode data */
            bytes_received += (I3_incoming_packet_length + 4);

            /* Incoming_packet_length is 1 larger than the string content of the packet
             * because it counts the NUL byte.
             */
            I3_incoming_packet_strlen = strlen(I3_incoming_packet);
            if (I3_incoming_packet_strlen == (I3_incoming_packet_length - 1))
            {
                /* Perfect, we got the packet exactly right */
            }
            else if (I3_incoming_packet_strlen >= I3_incoming_packet_length)
            {
                /* This means we didn't get our NUL byte, and the packet size
                 * was wrong (too small).  Our only choice here is to keep
                 * reading until we find the NUL byte.
                 */
                log_error("I3 Packet size mismatch!  No NUL byte found in read!");
                log_error("I3_incoming_packet_length: %d", I3_incoming_packet_length);
                log_error("strlen of packet: %lu", I3_incoming_packet_strlen);
                log_error("Packet: %s", I3_incoming_packet);
            }
            else
            {
                /* And this means we read too much, because the packet size
                 * was wrong (too big).  We now have leftovers that are really
                 * part of the NEXT packet.
                 */
                log_error("I3 Packet size mismatch! Read too much!");
                log_error("I3_incoming_packet_length: %d", I3_incoming_packet_length);
                log_error("strlen of packet: %lu", I3_incoming_packet_strlen);
                log_error("Packet: %s", I3_incoming_packet);
            }

            if (I3_incoming_packet[0] != '(' || I3_incoming_packet[1] != '{' ||
                I3_incoming_packet[I3_incoming_packet_length - 3] != '}' ||
                I3_incoming_packet[I3_incoming_packet_length - 2] != ')')
            {
                log_error("Invalid packet data, throw it away!");
                log_error("I3_incoming_packet_length: %d", I3_incoming_packet_length);
                log_error("strlen of packet: %lu", I3_incoming_packet_strlen);
                log_error("Packet: %s", I3_incoming_packet);
                I3_packet_cleanup();
                return;
            }
            I3_packet_processing_state = I3_PACKET_STATE_PROCESSING;
            /* break; */
        case I3_PACKET_STATE_PROCESSING:
            I3_handle_packet(I3_incoming_packet);
            I3_packet_cleanup();
            break;
        case I3_PACKET_STATE_LEFTOVERS:
            break;
        case I3_PACKET_STATE_MISSING:
            break;
        default:
            log_error("How did the code get here???");
            FD_CLR(I3_socket, &out_set);
            I3_connection_close(TRUE);
            break;
        }
    }
    return;

#if 0
	ret = read(I3_socket, I3_input_buffer + I3_input_pointer, MAX_STRING_LENGTH);
	if (!ret || (ret < 0 && errno != EAGAIN && errno != EWOULDBLOCK)) {
	    FD_CLR(I3_socket, &out_set);
	    if (ret < 0)
		log_info("%s", "Read error on I3 socket.");
	    else
		log_info("%s", "EOF encountered on I3 socket read.");
	    I3_connection_close(TRUE);
	    return;
	}
	if (ret < 0)					       /* EAGAIN */
	    return;
	if (ret == MAX_STRING_LENGTH) {
            char debug_str[33];

            memcpy(&size, I3_input_buffer, 4);
            size = ntohl(size);
	    log_info("String overflow in I3 socket read!");
	    log_info("Packet size appears to be %ld!", size);
	    log_info("MAX_STRING is %d", MAX_STRING_LENGTH);
	    log_info("IPS is %d", IPS);
            bzero(debug_str, 33);
            strncpy(debug_str, I3_input_buffer, 32);
            log_info("Leading string buffer is: %s", debug_str);
        }

	I3_input_pointer += ret;
	bytes_received += ret;
	if (packetdebug)
	    log_info("I3_PACKET Bytes received: %d", ret);
    }

    memcpy(&size, I3_input_buffer, 4);
    size = ntohl(size);

    if (size >= IPS) {
        log_error("%s", "I3 packet TOO LARGE!");
        do {
            char garbage[MAX_STRING_LENGTH];

            /* We can't read the packet, so throw it away */
            ret = read(I3_socket, garbage, MAX_STRING_LENGTH);
            if (!ret || (ret < 0 && errno != EAGAIN && errno != EWOULDBLOCK)) {
                FD_CLR(I3_socket, &out_set);
                if (ret < 0)
                    log_info("%s", "Read error on I3 socket.");
                else
                    log_info("%s", "EOF encountered on I3 socket read.");
                I3_connection_close(TRUE);
                return;
            }
            if(ret < 0)
                continue;

            I3_input_pointer += ret;
            bytes_received += ret;
        } while( size > I3_input_pointer );
        I3_input_pointer = 0;
        I3_output_pointer = 4;
        bzero(I3_input_buffer, IPS);
        bzero(I3_output_buffer, OPS);
        bzero(I3_currentpacket, IPS);
        return;
    }

    if (size <= I3_input_pointer - 4) {
	I3_read_packet();
	I3_parse_packet();
    }
    return;
#endif
}

/*****************************************
 * User level commands and social hooks. *
 *****************************************/

/* This is very possibly going to be spammy as hell */
I3_CMD(I3_show_ucache_contents)
{
    UCACHE_DATA *user;
    int users = 0;

    i3page_printf(ch, "%%^WHITE%%^Cached user information%%^RESET%%^\r\n");
    i3page_printf(
        ch, "%%^WHITE%%^User                          | Gender ( 0 = Male, 1 = Female, 2 = Neuter )%%^RESET%%^\r\n");
    i3page_printf(
        ch, "%%^WHITE%%^---------------------------------------------------------------------------%%^RESET%%^\r\n");
    for (user = first_ucache; user; user = user->next)
    {
        i3page_printf(ch, "%%^WHITE%%^%-30s %d%%^RESET%%^\r\n", user->name, user->gender);
        users++;
    }
    i3page_printf(ch, "%%^WHITE%%^%d users being cached.%%^RESET%%^\r\n", users);
}

I3_CMD(I3_beep)
{
    char *ps;
    char mud[MAX_INPUT_LENGTH];
    I3_MUD *pmud;

    if (I3IS_SET(I3FLAG(ch), I3_DENYBEEP))
    {
        i3_printf(ch, "You are not allowed to use i3beeps.\r\n");
        return;
    }

    if (!argument || argument[0] == '\0')
    {
        i3_printf(ch, "%%^WHITE%%^Usage: i3beep user@mud%%^RESET%%^\r\n");
        i3_printf(ch, "%%^WHITE%%^Usage: i3beep [on]/[off]%%^RESET%%^\r\n");
        return;
    }

    if (!strcasecmp(argument, "on"))
    {
        I3REMOVE_BIT(I3FLAG(ch), I3_BEEP);
        i3_printf(ch, "You now send and receive i3beeps.\r\n");
        return;
    }

    if (!strcasecmp(argument, "off"))
    {
        I3SET_BIT(I3FLAG(ch), I3_BEEP);
        i3_printf(ch, "You no longer send and receive i3beeps.\r\n");
        return;
    }

    if (I3IS_SET(I3FLAG(ch), I3_BEEP))
    {
        i3_printf(ch, "Your i3beeps are turned off.\r\n");
        return;
    }

    if (I3ISINVIS(ch))
    {
        i3_printf(ch, "You are invisible.\r\n");
        return;
    }

    ps = (char *)strchr(argument, '@');

    if (!argument || argument[0] == '\0' || ps == NULL)
    {
        i3_printf(
            ch,
            "%%^YELLOW%%^You should specify a person@mud.%%^RESET%%^\r\n"
            "(use %%^WHITE%%^%%^BOLD%%^i3mudlist%%^YELLOW%%^ to get an overview of the muds available)%%^RESET%%^\r\n");
        return;
    }

    ps[0] = '\0';
    ps++;
    strlcpy(mud, ps, MAX_INPUT_LENGTH);

    if (!(pmud = find_I3_mud_by_name(mud)))
    {
        i3_printf(
            ch,
            "%%^YELLOW%%^No such mud known.%%^RESET%%^\r\n"
            "(use %%^WHITE%%^%%^BOLD%%^i3mudlist%%^YELLOW%%^ to get an overview of the muds available)%%^RESET%%^\r\n");
        return;
    }

#if 0
    if (!strcasecmp(this_i3mud->name, pmud->name)) {
	i3_printf(ch, "Use your mud's own internal system for that.\r\n");
	return;
    }
#endif

    if (pmud->status >= 0)
    {
        i3_printf(ch, "%s is marked as down.\r\n", pmud->name);
        return;
    }

    if (pmud->beep == 0)
        i3_printf(ch, "%s does not support the 'beep' command. Sending anyway.\r\n", pmud->name);

    I3_send_beep(ch, argument, pmud);
    i3_printf(ch, "%%^YELLOW%%^You i3beep %s@%s.%%^RESET%%^\r\n", i3capitalize(argument), pmud->name);
}

I3_CMD(I3_tell)
{
    char to[MAX_INPUT_LENGTH], *ps;
    char mud[MAX_INPUT_LENGTH];
    I3_MUD *pmud;
    struct tm *local = localtime(&i3_time);

    if (I3IS_SET(I3FLAG(ch), I3_DENYTELL))
    {
        i3_printf(ch, "You are not allowed to use i3tells.\r\n");
        return;
    }

    if (!argument || argument[0] == '\0')
    {
        int x;

        i3_printf(ch, "%%^WHITE%%^Usage: i3tell <user@mud> <message>%%^RESET%%^\r\n");
        i3_printf(ch, "%%^WHITE%%^Usage: i3tell [on]/[off]%%^RESET%%^\r\n\r\n");
        i3_printf(ch, "%%^CYAN%%^The last %d things you were told over I3:%%^RESET%%^\r\n", MAX_I3TELLHISTORY);

        for (x = 0; x < MAX_I3TELLHISTORY; x++)
        {
            if (I3TELLHISTORY(ch, x) == NULL)
                break;
            i3_printf(ch, "%s\r\n", I3TELLHISTORY(ch, x));
        }
        return;
    }

    if (!strcasecmp(argument, "on"))
    {
        I3REMOVE_BIT(I3FLAG(ch), I3_TELL);
        i3_printf(ch, "You now send and receive i3tells.\r\n");
        return;
    }

    if (!strcasecmp(argument, "off"))
    {
        I3SET_BIT(I3FLAG(ch), I3_TELL);
        i3_printf(ch, "You no longer send and receive i3tells.\r\n");
        return;
    }

    if (I3IS_SET(I3FLAG(ch), I3_TELL))
    {
        i3_printf(ch, "Your i3tells are turned off.\r\n");
        return;
    }

    if (I3ISINVIS(ch))
    {
        i3_printf(ch, "You are invisible.\r\n");
        return;
    }

    argument = i3one_argument(argument, to);
    ps = (char *)strchr(to, '@');

    if (to[0] == '\0' || argument[0] == '\0' || ps == NULL)
    {
        i3_printf(
            ch,
            "%%^YELLOW%%^You should specify a person and a mud.%%^RESET%%^\r\n"
            "(use %%^WHITE%%^%%^BOLD%%^i3mudlist%%^YELLOW%%^ to get an overview of the muds available)%%^RESET%%^\r\n");
        return;
    }

    ps[0] = '\0';
    ps++;
    strlcpy(mud, ps, MAX_INPUT_LENGTH);

    if (!(pmud = find_I3_mud_by_name(mud)))
    {
        i3_printf(
            ch,
            "%%^YELLOW%%^No such mud known.%%^RESET%%^\r\n"
            "(use %%^WHITE%%^%%^BOLD%%^i3mudlist%%^YELLOW%%^ to get an overview of the muds available)%%^RESET%%^\r\n");
        return;
    }

#if 0
    if (!strcasecmp(this_i3mud->name, pmud->name)) {
	i3_printf(ch, "Use your mud's own internal system for that.\r\n");
	return;
    }
#endif

    if (pmud->status >= 0)
    {
        i3_printf(ch, "%s is marked as down.\r\n", pmud->name);
        return;
    }

    if (pmud->tell == 0)
    {
        i3_printf(ch, "%s does not support the 'tell' command.\r\n", pmud->name);
        return;
    }

    I3_send_tell(ch, to, pmud->name, argument);
    snprintf(mud, MAX_INPUT_LENGTH,
             "%s %%^YELLOW%%^You i3tell %%^CYAN%%^%%^BOLD%%^%s@%s%%^RESET%%^%%^YELLOW%%^: %%^RESET%%^%s",
             color_time(local), i3capitalize(to), pmud->name, argument);
    i3_printf(ch, "%s%%^RESET%%^\r\n", mud);
    i3_update_tellhistory(ch, mud);
}

I3_CMD(I3_reply)
{
    char buf[MAX_STRING_LENGTH];
    char to[MAX_INPUT_LENGTH];
    char mud[MAX_INPUT_LENGTH];
    char *ps;
    I3_MUD *pmud;
    struct tm *local = localtime(&i3_time);

    if (I3IS_SET(I3FLAG(ch), I3_DENYTELL))
    {
        i3_printf(ch, "You are not allowed to use i3tells.\r\n");
        return;
    }

    if (!argument || argument[0] == '\0')
    {
        i3_printf(ch, "Usage: i3reply <message>\r\n");
        return;
    }

    if (I3IS_SET(I3FLAG(ch), I3_TELL))
    {
        i3_printf(ch, "Your i3tells are turned off.\r\n");
        return;
    }

    if (I3ISINVIS(ch))
    {
        i3_printf(ch, "You are invisible.\r\n");
        return;
    }

    if (!i3_str_prefix("set ", argument))
    {
        argument += 4;
        argument = i3one_argument(argument, to);

        i3_printf(ch, "%%^GREEN%%^DEBUG: argument == %s\r\n", argument);
        i3_printf(ch, "%%^GREEN%%^DEBUG: to == %s\r\n", to);

        ps = (char *)strchr(to, '@');
        if (to[0] == '\0' || ps == NULL)
        {
            i3_printf(ch, "%%^YELLOW%%^You should specify a person and a mud.%%^RESET%%^\r\n"
                          "(use %%^WHITE%%^%%^BOLD%%^i3mudlist%%^YELLOW%%^ to get an overview of the muds "
                          "available)%%^RESET%%^\r\n");
            return;
        }
        ps[0] = '\0';
        ps++;
        strlcpy(mud, ps, MAX_INPUT_LENGTH);

        i3_printf(ch, "%%^GREEN%%^DEBUG: to == %s\r\n", to);
        i3_printf(ch, "%%^GREEN%%^DEBUG: mud == %s\r\n", mud);

        if (!(pmud = find_I3_mud_by_name(mud)))
        {
            i3_printf(ch, "%%^YELLOW%%^No such mud known.%%^RESET%%^\r\n"
                          "(use %%^WHITE%%^%%^BOLD%%^i3mudlist%%^YELLOW%%^ to get an overview of the muds "
                          "available)%%^RESET%%^\r\n");
            return;
        }

#if 0
        if (!strcasecmp(this_i3mud->name, pmud->name)) {
            i3_printf(ch, "Use your mud's own internal system for that.\r\n");
            return;
        }
#endif

        if (pmud->status >= 0)
        {
            i3_printf(ch, "%s is marked as down.\r\n", pmud->name);
            return;
        }

        if (pmud->tell == 0)
        {
            i3_printf(ch, "%s does not support the 'tell' command.\r\n", pmud->name);
            return;
        }

        snprintf(buf, MAX_INPUT_LENGTH, "%s@%s", to, pmud->name);
        I3STRFREE(I3REPLYNAME(ch));
        I3STRFREE(I3REPLYMUD(ch));
        I3REPLYNAME(ch) = I3STRALLOC(to);
        I3REPLYMUD(ch) = I3STRALLOC(pmud->name);
        i3_printf(ch, "i3reply target set to %s@%s.\r\n", I3REPLYNAME(ch), I3REPLYMUD(ch));
        return;
    }

    if (!I3REPLYNAME(ch) || !I3REPLYMUD(ch))
    {
        i3_printf(ch, "You have not yet received an i3tell?!?\r\n");
        return;
    }

    if (!strcasecmp(argument, "show"))
    {
        i3_printf(ch, "The last i3tell you received was from %s@%s.\r\n", I3REPLYNAME(ch), I3REPLYMUD(ch));
        return;
    }

    I3_send_tell(ch, I3REPLYNAME(ch), I3REPLYMUD(ch), argument);
    snprintf(buf, MAX_STRING_LENGTH,
             "%s %%^YELLOW%%^You i3reply to %%^CYAN%%^%%^BOLD%%^%s@%s%%^RESET%%^%%^YELLOW%%^: %%^RESET%%^%s",
             color_time(local), i3capitalize(I3REPLYNAME(ch)), I3REPLYMUD(ch), argument);
    i3_printf(ch, "%s%%^RESET%%^\r\n", buf);
    i3_update_tellhistory(ch, buf);

    /*
    snprintf(buf, MAX_STRING_LENGTH, "%s %s", I3REPLY(ch), argument);
    I3_tell(ch, buf);
    */
}

I3_CMD(I3_mudlisten)
{
    I3_CHANNEL *channel;
    char arg[MAX_INPUT_LENGTH];

    if (!argument || argument[0] == '\0')
    {
        i3_printf(ch, "%%^WHITE%%^Usage: i3mudlisten [all/none]%%^RESET%%^\r\n");
        i3_printf(ch, "%%^WHITE%%^Usage: i3mudlisten <localchannel> [on/off]%%^RESET%%^\r\n");
        return;
    }

    if (!strcasecmp(argument, "all"))
    {
        for (channel = first_I3chan; channel; channel = channel->next)
        {
            // if (!channel->local_name || channel->local_name[0] == '\0')
            //	continue;

            i3_printf(ch, "Subscribing to %s.\r\n", channel->I3_name);
            I3_send_channel_listen(channel, TRUE);
        }
        i3_printf(ch, "%%^YELLOW%%^The mud is now subscribed to all available local I3 channels.%%^RESET%%^\r\n");
        return;
    }

    if (!strcasecmp(argument, "none"))
    {
        for (channel = first_I3chan; channel; channel = channel->next)
        {
            // if (!channel->local_name || channel->local_name[0] == '\0')
            //	continue;

            i3_printf(ch, "Unsubscribing from %s.\r\n", channel->I3_name);
            I3_send_channel_listen(channel, FALSE);
        }
        i3_printf(ch, "%%^YELLOW%%^The mud is now unsubscribed from all available local I3 channels.%%^RESET%%^\r\n");
        return;
    }

    argument = i3one_argument(argument, arg);
    if (!(channel = find_I3_channel_by_localname(arg)))
    {
        i3_printf(ch, "No such channel configured locally.\r\n");
        return;
    }

    if (!strcasecmp(argument, "on"))
    {
        i3_printf(ch, "Turning %s channel on.\r\n", channel->local_name);
        I3_send_channel_listen(channel, TRUE);
        return;
    }

    if (!strcasecmp(argument, "off"))
    {
        i3_printf(ch, "Turning %s channel off.\r\n", channel->local_name);
        I3_send_channel_listen(channel, FALSE);
        return;
    }
    I3_mudlisten(ch, "");
}

I3_CMD(I3_mudlist)
{
    I3_MUD *mud;
    char filter[MAX_INPUT_LENGTH];
    int mudcount = 0;
    bool all = FALSE;

    argument = i3one_argument(argument, filter);

    if (!strcasecmp(filter, "all"))
    {
        all = TRUE;
        argument = i3one_argument(argument, filter);
    }

    if (first_mud == NULL)
    {
        i3_printf(ch, "There are no muds to list!?\r\n");
        return;
    }

    i3page_printf(ch, "%%^WHITE%%^%%^BOLD%%^%-30s%-10.10s%-25.25s%-15.15s %s%%^RESET%%^\r\n", "Name", "Type", "Mudlib",
                  "Address", "Port");
    for (mud = first_mud; mud; mud = mud->next)
    {
        if (mud == NULL)
        {
            log_error("%s", "I3_mudlist: NULL mud found in listing!");
            continue;
        }

        if (mud->name == NULL)
        {
            log_error("%s", "I3_mudlist: NULL mud name found in listing!");
            continue;
        }

        if (filter[0] && i3_str_prefix(filter, mud->name) && (mud->mud_type && i3_str_prefix(filter, mud->mud_type)) &&
            (mud->mudlib && i3_str_prefix(filter, mud->mudlib)))
            continue;

        if (!all && mud->status == 0)
            continue;

        mudcount++;

        switch (mud->status)
        {
        case -1:
            i3page_printf(ch, "%%^CYAN%%^%-30s%-10.10s%-25.25s%-15.15s %d%%^RESET%%^\r\n", mud->name, mud->mud_type,
                          mud->mudlib, mud->ipaddress, mud->player_port);
            break;
        case 0:
            i3page_printf(ch, "%%^RED%%^%%^BOLD%%^%-26s(down)%%^RESET%%^\r\n", mud->name);
            break;
        default:
            i3page_printf(ch, "%%^YELLOW%%^%-26s(rebooting, back in %d seconds)%%^RESET%%^\r\n", mud->name,
                          mud->status);
            break;
        }
    }
    i3page_printf(ch, "%%^WHITE%%^%%^BOLD%%^%d total muds listed.%%^RESET%%^\r\n", mudcount);
}

I3_CMD(I3_chanlist)
{
    I3_CHANNEL *channel;
    bool all = FALSE, found = FALSE;
    char filter[MAX_INPUT_LENGTH];

    *filter = '\0';
    argument = i3one_argument(argument, filter);

    if (!strcasecmp(filter, "all") && is_connected())
    {
        all = TRUE;
        argument = i3one_argument(argument, filter);
        i3page_printf(ch, "%%^CYAN%%^Showing ALL known channels.%%^RESET%%^\r\n\r\n");
    }

    i3page_printf(
        ch, "%%^CYAN%%^Local name          Perm    I3 Name             Hosted at           Status%%^RESET%%^\r\n");
    i3page_printf(
        ch, "%%^CYAN%%^-------------------------------------------------------------------------------%%^RESET%%^\r\n");
    for (channel = first_I3chan; channel; channel = channel->next)
    {
        found = FALSE;

        if (!all && !channel->local_name && (filter[0] == '\0'))
            continue;

        if (I3PERM(ch) < I3PERM_ADMIN && !channel->local_name)
            continue;

        if (I3PERM(ch) < channel->i3perm)
            continue;

        if (!all && filter[0] != '\0' && i3_str_prefix(filter, channel->I3_name) &&
            i3_str_prefix(filter, channel->host_mud))
            continue;

        if (channel->local_name && I3_hasname(I3LISTEN(ch), channel->local_name))
            found = TRUE;

        i3page_printf(ch,
                      "%%^CYAN%%^%%^BOLD%%^%c "
                      "%%^WHITE%%^%%^BOLD%%^%-18s%%^YELLOW%%^%-8s%%^BLUE%%^%%^BOLD%%^%-20s%%^MAGENTA%%^%%^BOLD%%^%-20s%"
                      "-8s%%^RESET%%^\r\n",
                      found ? '*' : ' ', channel->local_name ? channel->local_name : "Not configured",
                      perm_names[channel->i3perm], channel->I3_name, channel->host_mud,
                      channel->status == 0 ? "%^GREEN%^%^BOLD%^Public" : "%^RED%^%^BOLD%^Private");
    }
    i3page_printf(ch, "%%^CYAN%%^%%^BOLD%%^*: You are listening to these channels.%%^RESET%%^\r\n");
}

I3_CMD(I3_setup_channel)
{
    CHAR_DATA *vch;
    DESCRIPTOR_DATA *d;
    char localname[MAX_INPUT_LENGTH], I3_name[MAX_INPUT_LENGTH];
    I3_CHANNEL *channel, *channel2;
    int permvalue = I3PERM_MORT;

    if (!argument || argument[0] == '\0')
    {
        i3_printf(ch, "Usage: i3setchan <i3channelname> <localname> [permission]\r\n");
        return;
    }

    argument = i3one_argument(argument, I3_name);
    argument = i3one_argument(argument, localname);

    if (!(channel = find_I3_channel_by_name(I3_name)))
    {
        i3_printf(ch, "%%^YELLOW%%^Unknown channel%%^RESET%%^\r\n"
                      "(use %%^WHITE%%^%%^BOLD%%^i3chanlist%%^YELLOW%%^ to get an overview of the channels "
                      "available)%%^RESET%%^\r\n");
        return;
    }

    if (localname[0] == '\0')
    {
        if (!channel->local_name)
        {
            i3_printf(ch, "Channel %s@%s isn't configured.%%^RESET%%^\r\n", channel->I3_name, channel->host_mud);
            return;
        }

        if (channel->i3perm > I3PERM(ch))
        {
            i3_printf(ch, "You do not have sufficient permission to remove the %s channel.\r\n", channel->local_name);
            return;
        }

        for (d = first_descriptor; d; d = d->next)
        {
            vch = d->original ? d->original : d->character;

            if (!vch)
                continue;

            if (I3_hasname(I3LISTEN(vch), channel->local_name))
                I3_unflagchan(&I3LISTEN(vch), channel->local_name);
            if (I3_hasname(I3DENY(vch), channel->local_name))
                I3_unflagchan(&I3DENY(vch), channel->local_name);
        }
        log_info("setup_channel: removing %s as %s@%s", channel->local_name, channel->I3_name, channel->host_mud);
        I3_send_channel_listen(channel, FALSE);
        I3STRFREE(channel->local_name);
        I3_write_channel_config();
    }
    else
    {
        if (channel->local_name)
        {
            i3_printf(ch, "Channel %s@%s is already known as %s.\r\n", channel->I3_name, channel->host_mud,
                      channel->local_name);
            return;
        }
        if ((channel2 = find_I3_channel_by_localname(localname)))
        {
            i3_printf(ch, "Channel %s@%s is already known as %s.\r\n", channel2->I3_name, channel2->host_mud,
                      channel2->local_name);
            return;
        }

        if (argument && argument[0] != '\0')
        {
            permvalue = get_permvalue(argument);
            if (permvalue < 0 || permvalue > I3PERM_IMP)
            {
                i3_printf(ch, "Invalid permission setting.\r\n");
                return;
            }
            if (permvalue > I3PERM(ch))
            {
                i3_printf(ch, "You cannot assign a permission value above your own.\r\n");
                return;
            }
        }
        channel->local_name = I3STRALLOC(localname);
        channel->i3perm = permvalue;
        I3STRFREE(channel->layout_m);
        I3STRFREE(channel->layout_e);
        channel->layout_m = I3STRALLOC(
            "%%^RED%%^%%^BOLD%%^[%%^WHITE%%^%%^BOLD%%^%s%%^RED%%^%%^BOLD%%^] %%^CYAN%%^%%^BOLD%%^%s@%s: %%^CYAN%%^%s");
        channel->layout_e = I3STRALLOC("%%^RED%%^%%^BOLD%%^[%%^WHITE%%^%%^BOLD%%^%s%%^RED%%^%%^BOLD%%^] %%^CYAN%%^%s");
        i3_printf(ch, "%s@%s is now locally known as %s\r\n", channel->I3_name, channel->host_mud, channel->local_name);
        log_info("setup_channel: setting up %s@%s as %s", channel->I3_name, channel->host_mud, channel->local_name);
        I3_send_channel_listen(channel, TRUE);
        I3_write_channel_config();
    }
}

I3_CMD(I3_edit_channel)
{
    char localname[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    I3_CHANNEL *channel;

    if (!argument || argument[0] == '\0')
    {
        i3_printf(ch, "%%^WHITE%%^Usage: i3 editchan <localname> localname <new localname>%%^RESET%%^\r\n");
        i3_printf(ch, "%%^WHITE%%^Usage: i3 editchan <localname> perm <type>%%^RESET%%^\r\n");
        return;
    }

    argument = i3one_argument(argument, localname);

    if ((channel = find_I3_channel_by_localname(localname)) == NULL)
    {
        i3_printf(ch, "%%^YELLOW%%^Unknown local channel%%^RESET%%^\r\n"
                      "(use %%^WHITE%%^%%^BOLD%%^i3chanlist%%^YELLOW%%^ to get an overview of the channels "
                      "available)%%^RESET%%^\r\n");
        return;
    }

    argument = i3one_argument(argument, arg2);

    if (channel->i3perm > I3PERM(ch))
    {
        i3_printf(ch, "You do not have sufficient permissions to edit this channel.\r\n");
        return;
    }

    if (!strcasecmp(arg2, "localname"))
    {
        i3_printf(ch, "Local channel %s renamed to %s.\r\n", channel->local_name, argument);
        I3STRFREE(channel->local_name);
        channel->local_name = I3STRALLOC(argument);
        I3_write_channel_config();
        return;
    }

    if (!strcasecmp(arg2, "perm") || !strcasecmp(arg2, "permission"))
    {
        int permvalue = get_permvalue(argument);

        if (permvalue < 0 || permvalue > I3PERM_IMP)
        {
            i3_printf(ch, "Invalid permission setting.\r\n");
            return;
        }
        if (permvalue > I3PERM(ch))
        {
            i3_printf(ch, "You cannot set a permission higher than your own.\r\n");
            return;
        }
        if (channel->i3perm > I3PERM(ch))
        {
            i3_printf(ch, "You cannot edit a channel above your permission level.\r\n");
            return;
        }
        channel->i3perm = permvalue;
        i3_printf(ch, "Local channel %s permission changed to %s.\r\n", channel->local_name, argument);
        I3_write_channel_config();
        return;
    }
    I3_edit_channel(ch, "");
}

I3_CMD(I3_chan_who)
{
    char channel_name[MAX_INPUT_LENGTH];
    I3_CHANNEL *channel;
    I3_MUD *mud;

    argument = i3one_argument(argument, channel_name);

    if (channel_name[0] == '\0' || !argument || argument[0] == '\0')
    {
        i3_printf(ch, "Usage: i3chanwho <local channel> <mud>\r\n");
        return;
    }

    if ((channel = find_I3_channel_by_localname(channel_name)) == NULL)
    {
        i3_printf(ch, "%%^YELLOW%%^Unknown channel.%%^RESET%%^\r\n"
                      "(use %%^WHITE%%^%%^BOLD%%^i3chanlist%%^YELLOW%%^ to get an overview of the channels "
                      "available)%%^RESET%%^\r\n");
        return;
    }

    if (!(mud = find_I3_mud_by_name(argument)))
    {
        i3_printf(
            ch,
            "%%^YELLOW%%^Unknown mud.%%^RESET%%^\r\n"
            "(use %%^WHITE%%^%%^BOLD%%^i3mudlist%%^YELLOW%%^ to get an overview of the muds available)%%^RESET%%^\r\n");
        return;
    }

    if (mud->status >= 0)
    {
        i3_printf(ch, "%s is marked as down.\r\n", mud->name);
        return;
    }

    I3_send_chan_who(ch, channel, mud);
}

I3_CMD(I3_listen_channel)
{
    I3_CHANNEL *channel;

    if (!argument || argument[0] == '\0')
    {
        i3_printf(ch, "%%^CYAN%%^Currently tuned into:%%^RESET%%^\r\n");
        i3_printf(ch, "%%^WHITE%%^%%^BOLD%%^%s%%^RESET%%^\r\n",
                  (I3LISTEN(ch) && I3LISTEN(ch)[0] != '\0') ? I3LISTEN(ch) : "None");
        return;
    }

    if (!strcasecmp(argument, "all"))
    {
        for (channel = first_I3chan; channel; channel = channel->next)
        {
            if (!channel->local_name || channel->local_name[0] == '\0')
                continue;

            if (I3PERM(ch) >= channel->i3perm && !I3_hasname(I3LISTEN(ch), channel->local_name))
                I3_flagchan(&I3LISTEN(ch), channel->local_name);
        }
        i3_printf(ch, "%%^YELLOW%%^You are now listening to all available I3 channels.%%^RESET%%^\r\n");
        return;
    }

    if (!strcasecmp(argument, "none"))
    {
        for (channel = first_I3chan; channel; channel = channel->next)
        {
            if (!channel->local_name || channel->local_name[0] == '\0')
                continue;

            if (I3_hasname(I3LISTEN(ch), channel->local_name))
                I3_unflagchan(&I3LISTEN(ch), channel->local_name);
        }
        i3_printf(ch, "%%^YELLOW%%^You no longer listen to any available I3 channels.%%^RESET%%^\r\n");
        return;
    }

    if ((channel = find_I3_channel_by_localname(argument)) == NULL)
    {
        i3_printf(ch, "%%^YELLOW%%^Unknown channel.%%^RESET%%^\r\n"
                      "(use %%^WHITE%%^%%^BOLD%%^i3chanlist%%^YELLOW%%^ to get an overview of the channels "
                      "available)%%^RESET%%^\r\n");
        return;
    }

    if (I3_hasname(I3LISTEN(ch), channel->local_name))
    {
        i3_printf(ch, "You no longer listen to %s\r\n", channel->local_name);
        I3_unflagchan(&I3LISTEN(ch), channel->local_name);
    }
    else
    {
        if (I3PERM(ch) < channel->i3perm)
        {
            i3_printf(ch, "Channel %s is above your permission level.\r\n", channel->local_name);
            return;
        }
        i3_printf(ch, "You now listen to %s\r\n", channel->local_name);
        I3_flagchan(&I3LISTEN(ch), channel->local_name);
    }
}

I3_CMD(I3_deny_channel)
{
    char vic_name[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    I3_CHANNEL *channel;

    argument = i3one_argument(argument, vic_name);

    if (vic_name[0] == '\0' || !argument || argument[0] == '\0')
    {
        i3_printf(ch, "%%^WHITE%%^Usage: i3deny <person> <local channel name>%%^RESET%%^\r\n");
        i3_printf(ch, "%%^WHITE%%^Usage: i3deny <person> [tell/beep/finger]%%^RESET%%^\r\n");
        return;
    }

    if (!(victim = I3_find_user(vic_name)))
    {
        i3_printf(ch, "No such person is currently online.\r\n");
        return;
    }

    if (I3PERM(ch) <= I3PERM(victim))
    {
        i3_printf(ch, "You cannot alter their settings.\r\n");
        return;
    }

    if (!strcasecmp(argument, "tell"))
    {
        if (!I3IS_SET(I3FLAG(victim), I3_DENYTELL))
        {
            I3SET_BIT(I3FLAG(victim), I3_DENYTELL);
            i3_printf(ch, "%s can no longer use i3tells.\r\n", CH_I3NAME(victim));
            return;
        }
        I3REMOVE_BIT(I3FLAG(victim), I3_DENYTELL);
        i3_printf(ch, "%s can use i3tells again.\r\n", CH_I3NAME(victim));
        return;
    }

    if (!strcasecmp(argument, "beep"))
    {
        if (!I3IS_SET(I3FLAG(victim), I3_DENYBEEP))
        {
            I3SET_BIT(I3FLAG(victim), I3_DENYBEEP);
            i3_printf(ch, "%s can no longer use i3beeps.\r\n", CH_I3NAME(victim));
            return;
        }
        I3REMOVE_BIT(I3FLAG(victim), I3_DENYBEEP);
        i3_printf(ch, "%s can use i3beeps again.\r\n", CH_I3NAME(victim));
        return;
    }

    if (!strcasecmp(argument, "finger"))
    {
        if (!I3IS_SET(I3FLAG(victim), I3_DENYFINGER))
        {
            I3SET_BIT(I3FLAG(victim), I3_DENYFINGER);
            i3_printf(ch, "%s can no longer use i3fingers.\r\n", CH_I3NAME(victim));
            return;
        }
        I3REMOVE_BIT(I3FLAG(victim), I3_DENYFINGER);
        i3_printf(ch, "%s can use i3fingers again.\r\n", CH_I3NAME(victim));
        return;
    }

    if (!(channel = find_I3_channel_by_localname(argument)))
    {
        i3_printf(ch, "%%^YELLOW%%^Unknown channel.%%^RESET%%^\r\n"
                      "(use %%^WHITE%%^%%^BOLD%%^i3chanlist%%^YELLOW%%^ to get an overview of the channels "
                      "available)%%^RESET%%^\r\n");
        return;
    }

    if (I3_hasname(I3DENY(ch), channel->local_name))
    {
        i3_printf(ch, "%s can now listen to %s\r\n", CH_I3NAME(victim), channel->local_name);
        I3_unflagchan(&I3DENY(ch), channel->local_name);
    }
    else
    {
        i3_printf(ch, "%s can no longer listen to %s\r\n", CH_I3NAME(victim), channel->local_name);
        I3_flagchan(&I3DENY(ch), channel->local_name);
    }
}

I3_CMD(I3_mudinfo)
{
    I3_MUD *mud;

    if (!argument || argument[0] == '\0')
    {
        i3_printf(ch, "Usage: i3mudinfo <mudname>\r\n");
        return;
    }

    if (!(mud = find_I3_mud_by_name(argument)))
    {
        i3_printf(
            ch,
            "%%^YELLOW%%^Unknown mud.%%^RESET%%^\r\n"
            "(use %%^WHITE%%^%%^BOLD%%^i3mudlist%%^YELLOW%%^ to get an overview of the muds available)%%^RESET%%^\r\n");
        return;
    }

    i3_printf(ch, "%%^WHITE%%^%%^BOLD%%^Information about %s%%^RESET%%^\r\n\r\n", mud->name);
    if (mud->status == 0)
        i3_printf(ch, "%%^WHITE%%^Status     : Currently down%%^RESET%%^\r\n");
    else if (mud->status > 0)
        i3_printf(ch, "%%^WHITE%%^Status     : Currently rebooting, back in %d seconds%%^RESET%%^\r\n", mud->status);
    i3_printf(ch, "%%^WHITE%%^MUD port   : %s %d%%^RESET%%^\r\n", mud->ipaddress, mud->player_port);
    i3_printf(ch, "%%^WHITE%%^Base mudlib: %s%%^RESET%%^\r\n", mud->base_mudlib);
    i3_printf(ch, "%%^WHITE%%^Mudlib     : %s%%^RESET%%^\r\n", mud->mudlib);
    i3_printf(ch, "%%^WHITE%%^Driver     : %s%%^RESET%%^\r\n", mud->driver);
    i3_printf(ch, "%%^WHITE%%^Type       : %s%%^RESET%%^\r\n", mud->mud_type);
    i3_printf(ch, "%%^WHITE%%^Open status: %s%%^RESET%%^\r\n", mud->open_status);
    i3_printf(ch, "%%^WHITE%%^Admin      : %s%%^RESET%%^\r\n", mud->admin_email);
    if (mud->web)
        i3_printf(ch, "%%^WHITE%%^URL        : %s%%^RESET%%^\r\n", mud->web);
    if (mud->web_wrong && !mud->web)
        i3_printf(ch, "%%^WHITE%%^URL        : %s%%^RESET%%^\r\n", mud->web_wrong);
    if (mud->daemon)
        i3_printf(ch, "%%^WHITE%%^Daemon     : %s%%^RESET%%^\r\n", mud->daemon);
    if (mud->time)
        i3_printf(ch, "%%^WHITE%%^Time       : %s%%^RESET%%^\r\n", mud->time);
    if (mud->banner)
        i3_printf(ch, "%%^WHITE%%^Banner:%%^RESET%%^\r\n%s\r\n", mud->banner);

    i3_printf(ch, "%%^WHITE%%^Supports   : ");
    if (mud->tell)
        i3_printf(ch, "%%^WHITE%%^tell, ");
    if (mud->beep)
        i3_printf(ch, "%%^WHITE%%^beep, ");
    if (mud->emoteto)
        i3_printf(ch, "%%^WHITE%%^emoteto, ");
    if (mud->who)
        i3_printf(ch, "%%^WHITE%%^who, ");
    if (mud->finger)
        i3_printf(ch, "%%^WHITE%%^finger, ");
    if (mud->locate)
        i3_printf(ch, "%%^WHITE%%^locate, ");
    if (mud->channel)
        i3_printf(ch, "%%^WHITE%%^channel, ");
    if (mud->news)
        i3_printf(ch, "%%^WHITE%%^news, ");
    if (mud->mail)
        i3_printf(ch, "%%^WHITE%%^mail, ");
    if (mud->file)
        i3_printf(ch, "%%^WHITE%%^file, ");
    if (mud->auth)
        i3_printf(ch, "%%^WHITE%%^auth, ");
    if (mud->ucache)
        i3_printf(ch, "%%^WHITE%%^ucache, ");
    i3_printf(ch, "%%^RESET%%^\r\n");

    i3_printf(ch, "%%^WHITE%%^Supports   : ");
    if (mud->smtp)
        i3_printf(ch, "%%^WHITE%%^smtp (port %d), ", mud->smtp);
    if (mud->http)
        i3_printf(ch, "%%^WHITE%%^http (port %d), ", mud->http);
    if (mud->ftp)
        i3_printf(ch, "%%^WHITE%%^ftp  (port %d), ", mud->ftp);
    if (mud->pop3)
        i3_printf(ch, "%%^WHITE%%^pop3 (port %d), ", mud->pop3);
    if (mud->nntp)
        i3_printf(ch, "%%^WHITE%%^nntp (port %d), ", mud->nntp);
    if (mud->rcp)
        i3_printf(ch, "%%^WHITE%%^rcp  (port %d), ", mud->rcp);
    if (mud->amrcp)
        i3_printf(ch, "%%^WHITE%%^amrcp (port %d), ", mud->amrcp);
    i3_printf(ch, "%%^RESET%%^\r\n");
}

I3_CMD(I3_chanlayout)
{
    I3_CHANNEL *channel = NULL;
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    if (!argument || argument[0] == '\0')
    {
        i3_printf(ch, "%%^WHITE%%^Usage: i3chanlayout <localchannel|all> <layout> <format...>%%^RESET%%^\r\n");
        i3_printf(ch, "%%^WHITE%%^Layout can be one of these: layout_e layout_m%%^RESET%%^\r\n");
        i3_printf(ch, "%%^WHITE%%^Format can be any way you want it to look, provided you have the proper number of "
                      "%%s tags in it.%%^RESET%%^\r\n");
        return;
    }

    argument = i3one_argument(argument, arg1);
    argument = i3one_argument(argument, arg2);

    if (arg1[0] == '\0')
    {
        I3_chanlayout(ch, "");
        return;
    }
    if (arg2[0] == '\0')
    {
        if (!(channel = find_I3_channel_by_localname(arg1)))
        {
            I3_chanlayout(ch, "");
            return;
        }
        else
        {
            i3_printf(ch, "Message layout for <%%^GREEN%%^%s%%^RESET%%^>: \"%%^YELLOW%%^", arg1);
            cprintf(ch, "%s", channel->layout_m);
            i3_printf(ch, "%%^RESET%%^\"\r\n");
            i3_printf(ch, "Emote layout for <%%^GREEN%%^%s%%^RESET%%^>: \"%%^YELLOW%%^", arg1);
            cprintf(ch, "%s", channel->layout_e);
            i3_printf(ch, "%%^RESET%%^\"\r\n");
            return;
        }
    }
    if (!argument || argument[0] == '\0')
    {
        I3_chanlayout(ch, "");
        return;
    }

    if (!(channel = find_I3_channel_by_localname(arg1)))
    {
        i3_printf(ch, "%%^YELLOW%%^Unknown channel.%%^RESET%%^\r\n"
                      "(use %%^WHITE%%^%%^BOLD%%^i3chanlist%%^YELLOW%%^ to get an overview of the channels "
                      "available)%%^RESET%%^\r\n");
        return;
    }

    if (!strcasecmp(arg2, "layout_e"))
    {
        if (!verify_i3layout(argument, 2))
        {
            i3_printf(ch, "Incorrect format for layout_e. You need exactly 2 %%s's.\r\n");
            return;
        }
        I3STRFREE(channel->layout_e);
        channel->layout_e = I3STRALLOC(argument);
        i3_printf(ch, "Channel layout_e changed.\r\n");
        I3_write_channel_config();
        return;
    }

    if (!strcasecmp(arg2, "layout_m"))
    {
        if (!verify_i3layout(argument, 4))
        {
            i3_printf(ch, "Incorrect format for layout_m. You need exactly 4 %%s's.\r\n");
            return;
        }
        I3STRFREE(channel->layout_m);
        channel->layout_m = I3STRALLOC(argument);
        i3_printf(ch, "Channel layout_m changed.\r\n");
        I3_write_channel_config();
        return;
    }
    I3_chanlayout(ch, "");
}

I3_CMD(I3_bancmd)
{
    I3_BAN *temp;

    if (!argument || argument[0] == '\0')
    {
        i3_printf(ch, "%%^GREEN%%^%%^BOLD%%^The mud currently has the following ban list:%%^RESET%%^\r\n\r\n");

        if (!first_i3ban)
            i3_printf(ch, "%%^YELLOW%%^Nothing%%^RESET%%^\r\n");
        else
        {
            for (temp = first_i3ban; temp; temp = temp->next)
                i3_printf(ch, "%%^YELLOW%%^\t  - %s%%^RESET%%^\r\n", temp->name);
        }
        i3_printf(
            ch, "\r\n%%^YELLOW%%^To add a ban, just specify a target. Suggested targets being user@mud or IP:Port\r\n");
        i3_printf(
            ch, "%%^YELLOW%%^User@mud bans can also have wildcard specifiers, such as *@Mud or User@*%%^RESET%%^\r\n");
        return;
    }

    if (!fnmatch(argument, this_i3mud->name, 0))
    {
        i3_printf(ch, "%%^YELLOW%%^You don't really want to do that....%%^RESET%%^\r\n");
        return;
    }

    for (temp = first_i3ban; temp; temp = temp->next)
    {
        if (!strcasecmp(temp->name, argument))
        {
            I3STRFREE(temp->name);
            I3UNLINK(temp, first_i3ban, last_i3ban, next, prev);
            I3DISPOSE(temp);
            I3_write_bans();
            i3_printf(ch, "%%^YELLOW%%^The mud no longer bans %s.%%^RESET%%^\r\n", argument);
            return;
        }
    }
    I3CREATE(temp, I3_BAN, 1);
    temp->name = I3STRALLOC(argument);
    I3LINK(temp, first_i3ban, last_i3ban, next, prev);
    I3_write_bans();
    i3_printf(ch, "%%^YELLOW%%^The mud now bans all incoming traffic from %s.%%^RESET%%^\r\n", temp->name);
}

I3_CMD(I3_ignorecmd)
{
    I3_IGNORE *temp;
    char buf[MAX_INPUT_LENGTH];

    if (!argument || argument[0] == '\0')
    {
        i3_printf(ch, "%%^GREEN%%^%%^BOLD%%^You are currently ignoring the following:%%^RESET%%^\r\n\r\n");

        if (!FIRST_I3IGNORE(ch))
        {
            i3_printf(ch, "%%^YELLOW%%^Nobody%%^RESET%%^\r\n\r\n");
            i3_printf(ch, "%%^YELLOW%%^To add an ignore, just specify a target. Suggested targets being user@mud or "
                          "IP:Port%%^RESET%%^\r\n");
            i3_printf(ch, "%%^YELLOW%%^User@mud ignores can also have wildcard specifiers, such as *@Mud or "
                          "User@*%%^RESET%%^\r\n");
            return;
        }
        for (temp = FIRST_I3IGNORE(ch); temp; temp = temp->next)
            i3_printf(ch, "%%^YELLOW%%^\t  - %s%%^RESET%%^\r\n", temp->name);

        return;
    }

    snprintf(buf, MAX_INPUT_LENGTH, "%s@%s", CH_I3NAME(ch), this_i3mud->name);
    if (!strcasecmp(buf, argument))
    {
        i3_printf(ch, "%%^YELLOW%%^You don't really want to do that....%%^RESET%%^\r\n");
        return;
    }

    if (!fnmatch(argument, this_i3mud->name, 0))
    {
        i3_printf(ch, "%%^YELLOW%%^Ignoring your own mud would be silly.%%^RESET%%^\r\n");
        return;
    }

    for (temp = FIRST_I3IGNORE(ch); temp; temp = temp->next)
    {
        if (!strcasecmp(temp->name, argument))
        {
            I3STRFREE(temp->name);
            I3UNLINK(temp, FIRST_I3IGNORE(ch), LAST_I3IGNORE(ch), next, prev);
            I3DISPOSE(temp);
            i3_printf(ch, "%%^YELLOW%%^You are no longer ignoring %s.%%^RESET%%^\r\n", argument);
            return;
        }
    }

    I3CREATE(temp, I3_IGNORE, 1);
    temp->name = I3STRALLOC(argument);
    I3LINK(temp, FIRST_I3IGNORE(ch), LAST_I3IGNORE(ch), next, prev);
    i3_printf(ch, "%%^YELLOW%%^You now ignore %s.%%^RESET%%^\r\n", temp->name);
}

I3_CMD(I3_invis)
{
    if (I3INVIS(ch))
    {
        I3REMOVE_BIT(I3FLAG(ch), I3_INVIS);
        i3_printf(ch, "You are now i3visible.\r\n");
    }
    else
    {
        I3SET_BIT(I3FLAG(ch), I3_INVIS);
        i3_printf(ch, "You are now i3invisible.\r\n");
    }
}

I3_CMD(I3_debug)
{
    packetdebug = !packetdebug;

    if (packetdebug)
        i3_printf(ch, "I3_PACKET Packet debugging enabled.\r\n");
    else
        i3_printf(ch, "I3_PACKET Packet debugging disabled.\r\n");
}

I3_CMD(I3_send_user_req)
{
    char user[MAX_INPUT_LENGTH], mud[MAX_INPUT_LENGTH];
    char *ps;
    I3_MUD *pmud;

    if (!argument || argument[0] == '\0')
    {
        i3_printf(
            ch,
            "%%^YELLOW%%^Query who at which mud?%%^RESET%%^\r\n"
            "(use %%^WHITE%%^%%^BOLD%%^i3mudlist%%^YELLOW%%^ to get an overview of the muds available)%%^RESET%%^\r\n");
        return;
    }
    if (!(ps = (char *)strchr(argument, '@')))
    {
        i3_printf(
            ch,
            "%%^YELLOW%%^You should specify a person and a mud.%%^RESET%%^\r\n"
            "(use %%^WHITE%%^%%^BOLD%%^i3mudlist%%^YELLOW%%^ to get an overview of the muds available)%%^RESET%%^\r\n");
        return;
    }

    ps[0] = '\0';
    strlcpy(user, argument, MAX_INPUT_LENGTH);
    strlcpy(mud, ps + 1, MAX_INPUT_LENGTH);

    if (user[0] == '\0' || mud[0] == '\0')
    {
        i3_printf(
            ch,
            "%%^YELLOW%%^You should specify a person and a mud.%%^RESET%%^\r\n"
            "(use %%^WHITE%%^%%^BOLD%%^i3mudlist%%^YELLOW%%^ to get an overview of the muds available)%%^RESET%%^\r\n");
        return;
    }

    if (!(pmud = find_I3_mud_by_name(mud)))
    {
        i3_printf(
            ch,
            "%%^YELLOW%%^No such mud known.%%^RESET%%^\r\n"
            "(use %%^WHITE%%^%%^BOLD%%^i3mudlist%%^YELLOW%%^ to get an overview of the muds available)%%^RESET%%^\r\n");
        return;
    }

    if (pmud->status >= 0)
    {
        i3_printf(ch, "%s is marked as down.\r\n", pmud->name);
        return;
    }

    I3_send_chan_user_req(pmud->name, user);
}

I3_CMD(I3_admin_channel)
{
    I3_CHANNEL *channel = NULL;
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];

    if (!argument || argument[0] == '\0')
    {
        i3_printf(ch, "%%^WHITE%%^Usage: i3adminchan <localchannel> <add|remove> <mudname>%%^RESET%%^\r\n");
        i3_printf(ch, "%%^WHITE%%^Usage: i3adminchan <localchannel> list%%^RESET%%^\r\n");
        return;
    }
    argument = i3one_argument(argument, arg1);
    argument = i3one_argument(argument, arg2);

    if (arg1[0] == '\0')
    {
        I3_admin_channel(ch, "");
        return;
    }

    if (!(channel = find_I3_channel_by_localname(arg1)))
    {
        i3_printf(ch, "No such channel with that name here.\r\n");
        return;
    }

    if (arg2[0] == '\0')
    {
        I3_admin_channel(ch, "");
        return;
    }

    if (!strcasecmp(arg2, "list"))
    {
        I3_send_channel_adminlist(ch, channel->I3_name);
        i3_printf(ch, "Sending request for administrative list.\r\n");
        return;
    }

    if (!argument || argument[0] == '\0')
    {
        I3_admin_channel(ch, "");
        return;
    }

    if (!strcasecmp(arg2, "add"))
    {
        snprintf(buf, MAX_STRING_LENGTH, "({\"%s\",}),({}),", argument);
        I3_send_channel_admin(ch, channel->I3_name, buf);
        i3_printf(ch, "Sending administrative list addition.\r\n");
        return;
    }

    if (!strcasecmp(arg2, "remove"))
    {
        snprintf(buf, MAX_STRING_LENGTH, "({}),({\"%s\",}),", argument);
        I3_send_channel_admin(ch, channel->I3_name, buf);
        i3_printf(ch, "Sending administrative list removal.\r\n");
        return;
    }
    I3_admin_channel(ch, "");
}

I3_CMD(I3_disconnect)
{
    if (!is_connecting())
    {
        i3_printf(ch, "The MUD isn't connected to the Intermud-3 router.\r\n");
        return;
    }

    i3_printf(ch, "Disconnecting from Intermud-3 router (%s).\r\n", I3_ROUTER_NAME);

    i3_shutdown(0, ch);
}

I3_CMD(I3_connect)
{
    ROUTER_DATA *router;

    if (is_connecting())
    {
        i3_printf(ch, "The MUD is already connected to Intermud-3 router %s\r\n", I3_ROUTER_NAME);
        return;
    }

    if (!argument || argument[0] == '\0')
    {
        char timeout_msg[MAX_INPUT_LENGTH];

        *timeout_msg = '\0';
        i3_printf(ch, "Connecting to Intermud-3 router (%s)\r\n", I3_ROUTER_NAME);
        snprintf(timeout_msg, MAX_INPUT_LENGTH, "Connecting to Intermud-3 router (%s)", I3_ROUTER_NAME);
        allchan_log(0, (char *)"wiley", (char *)"Cron", (char *)"Cron", (char *)"WileyMUD", (char *)timeout_msg);
        router_connect(NULL, TRUE, this_i3mud->player_port, FALSE);
        return;
    }

    for (router = first_router; router; router = router->next)
    {
        if (!strcasecmp(router->name, argument))
        {
            char timeout_msg[MAX_INPUT_LENGTH];

            *timeout_msg = '\0';
            router->reconattempts = 0;
            i3_printf(ch, "Connecting to Intermud-3 router (%s)\r\n", argument);
            snprintf(timeout_msg, MAX_INPUT_LENGTH, "Connecting to Intermud-3 router (%s)", argument);
            allchan_log(0, (char *)"wiley", (char *)"Cron", (char *)"Cron", (char *)"WileyMUD", (char *)timeout_msg);
            router_connect(argument, TRUE, this_i3mud->player_port, FALSE);
            return;
        }
    }

    i3_printf(ch, "%s is not configured as a router for this mud.\r\n", argument);
    i3_printf(ch, "If you wish to add it, use the i3router command to provide its information.\r\n");
}

I3_CMD(I3_reload)
{
    int mudport = this_i3mud->player_port;

    if (is_connecting())
    {
        i3_printf(ch, "Disconnecting from I3 router %s...\r\n", I3_ROUTER_NAME);
        i3_shutdown(0, ch);
    }
    i3_printf(ch, "Reloading I3 configuration...\r\n");
    if (I3_read_config(mudport))
    {
        i3_printf(ch, "Reconnecting to I3 router...\r\n");
        router_connect(NULL, FALSE, this_i3mud->player_port, FALSE);
    }
    i3_printf(ch, "Done!\r\n");
}

I3_CMD(I3_addchan)
{
    I3_CHANNEL *channel;
    char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    int type, x;

    argument = i3one_argument(argument, arg);
    argument = i3one_argument(argument, arg2);

    if (!argument || argument[0] == '\0' || arg[0] == '\0' || arg2[0] == '\0')
    {
        i3_printf(ch, "%%^WHITE%%^Usage: i3addchan <channelname> <localname> <type>%%^RESET%%^\r\n\r\n");
        i3_printf(ch, "%%^WHITE%%^Channelname should be the name seen on 'chanlist all'%%^RESET%%^\r\n");
        i3_printf(ch, "%%^WHITE%%^Localname should be the local name you want it listed as.%%^RESET%%^\r\n");
        i3_printf(ch, "%%^WHITE%%^Type can be one of the following:%%^RESET%%^\r\n\r\n");
        i3_printf(ch, "%%^WHITE%%^0: selectively banned%%^RESET%%^\r\n");
        i3_printf(ch, "%%^WHITE%%^1: selectively admitted%%^RESET%%^\r\n");
        i3_printf(ch, "%%^WHITE%%^2: filtered - valid for selectively admitted ONLY%%^RESET%%^\r\n");
        return;
    }

    if ((channel = find_I3_channel_by_name(arg)) != NULL)
    {
        i3_printf(ch, "%%^RED%%^%%^BOLD%%^%s is already hosted by %s.%%^RESET%%^\r\n", channel->I3_name,
                  channel->host_mud);
        // i3_printf(ch, "%%^RED%%^%%^BOLD%%^But we will try adding it anyways...%%^RESET%%^\r\n");
        // I3_send_channel_add(ch, arg, type);
        return;
    }

    if ((channel = find_I3_channel_by_localname(arg2)) != NULL)
    {
        i3_printf(ch, "%%^RED%%^%%^BOLD%%^Channel %s@%s is already locally configured as %s.%%^RESET%%^\r\n",
                  channel->I3_name, channel->host_mud, channel->local_name);
        return;
    }

    if (!isdigit(argument[0]))
    {
        i3_printf(ch, "%%^RED%%^%%^BOLD%%^Invalid type. Must be numerical.%%^RESET%%^\r\n");
        I3_addchan(ch, "");
        return;
    }

    type = atoi(argument);
    if (type < 0 || type > 2)
    {
        i3_printf(ch, "%%^RED%%^%%^BOLD%%^Invalid channel type.%%^RESET%%^\r\n");
        I3_addchan(ch, "");
        return;
    }

    i3_printf(ch, "%%^GREEN%%^%%^BOLD%%^Adding channel to router: %%^WHITE%%^%%^BOLD%%^%s%%^RESET%%^\r\n", arg);
    I3_send_channel_add(ch, arg, type);

    I3CREATE(channel, I3_CHANNEL, 1);
    channel->I3_name = I3STRALLOC(arg);
    channel->host_mud = I3STRALLOC(this_i3mud->name);
    channel->local_name = I3STRALLOC(arg2);
    channel->i3perm = I3PERM_ADMIN;
    channel->layout_m = I3STRALLOC(
        "%%^RED%%^%%^BOLD%%^[%%^WHITE%%^%%^BOLD%%^%s%%^RED%%^%%^BOLD%%^] %%^CYAN%%^%%^BOLD%%^%s@%s: %%^CYAN%%^%s");
    channel->layout_e = I3STRALLOC("%%^RED%%^%%^BOLD%%^[%%^WHITE%%^%%^BOLD%%^%s%%^RED%%^%%^BOLD%%^] %%^CYAN%%^%s");
    for (x = 0; x < MAX_I3HISTORY; x++)
        channel->history[x] = NULL;
    I3LINK(channel, first_I3chan, last_I3chan, next, prev);

    if (type != 0)
    {
        snprintf(buf, MAX_STRING_LENGTH, "({\"%s\",}),({}),", this_i3mud->name);
        I3_send_channel_admin(ch, channel->I3_name, buf);
        i3_printf(ch, "%%^GREEN%%^%%^BOLD%%^Sending command to add %s to the invite list.%%^RESET%%^\r\n",
                  this_i3mud->name);
    }

    i3_printf(ch, "%%^YELLOW%%^%s@%s %%^WHITE%%^%%^BOLD%%^is now locally known as %%^YELLOW%%^%s%%^RESET%%^\r\n",
              channel->I3_name, channel->host_mud, channel->local_name);
    I3_send_channel_listen(channel, TRUE);
    I3_write_channel_config();
}

I3_CMD(I3_removechan)
{
    I3_CHANNEL *channel = NULL;

    if (!argument || argument[0] == '\0')
    {
        i3_printf(ch, "%%^WHITE%%^Usage: i3removechan <channel>%%^RESET%%^\r\n");
        i3_printf(ch, "%%^WHITE%%^Channelname should be the name seen on 'chanlist all'%%^RESET%%^\r\n");
        return;
    }

    if ((channel = find_I3_channel_by_name(argument)) == NULL)
    {
        i3_printf(ch, "%%^RED%%^%%^BOLD%%^No channel by that name exists.%%^RESET%%^\r\n");
        return;
    }

    if (strcasecmp(channel->host_mud, this_i3mud->name))
    {
        i3_printf(ch, "%%^RED%%^%%^BOLD%%^%s does not host this channel and cannot remove it.%%^RESET%%^\r\n",
                  this_i3mud->name);
        // i3_printf(ch, "%%^RED%%^%%^BOLD%%^But we will try removing it anyways...%%^RESET%%^\r\n");
        // I3_send_channel_remove(ch, channel);
        return;
    }

    i3_printf(ch, "%%^YELLOW%%^Removing channel from router: %%^WHITE%%^%%^BOLD%%^%s%%^RESET%%^\r\n", channel->I3_name);
    I3_send_channel_remove(ch, channel);

    i3_printf(ch, "%%^RED%%^%%^BOLD%%^Destroying local channel entry for %%^WHITE%%^%%^BOLD%%^%s%%^RESET%%^\r\n",
              channel->I3_name);
    destroy_I3_channel(channel);
    I3_write_channel_config();
}

I3_CMD(I3_setconfig)
{
    char arg[MAX_INPUT_LENGTH];

    argument = i3one_argument(argument, arg);

    if (arg[0] == '\0')
    {
        i3_printf(ch, "%%^GREEN%%^%%^BOLD%%^Configuration info for your mud. Changes save when edited.%%^RESET%%^\r\n");
        i3_printf(ch, "%%^GREEN%%^%%^BOLD%%^You can set the following:%%^RESET%%^\r\n\r\n");
        i3_printf(ch,
                  "%%^WHITE%%^Show       : %%^GREEN%%^%%^BOLD%%^Displays your current congfiguration.%%^RESET%%^\r\n");
        i3_printf(ch, "%%^WHITE%%^Autoconnect: %%^GREEN%%^%%^BOLD%%^A toggle. Either on or off. Your mud will connect "
                      "automatically with it on.%%^RESET%%^\r\n");
        i3_printf(ch, "%%^WHITE%%^Mudname    : %%^GREEN%%^%%^BOLD%%^The name you want displayed on I3 for your "
                      "mud.%%^RESET%%^\r\n");
        i3_printf(ch, "%%^WHITE%%^Telnet     : %%^GREEN%%^%%^BOLD%%^The telnet address for your mud. Do not include "
                      "the port number.%%^RESET%%^\r\n");
        i3_printf(ch, "%%^WHITE%%^Web        : %%^GREEN%%^%%^BOLD%%^The website address for your mud. In the form of: "
                      "www.address.com%%^RESET%%^\r\n");
        i3_printf(ch, "%%^WHITE%%^Email      : %%^GREEN%%^%%^BOLD%%^The email address of your mud's administrator. "
                      "Needs to be valid!!%%^RESET%%^\r\n");
        i3_printf(ch, "%%^WHITE%%^Status     : %%^GREEN%%^%%^BOLD%%^The open status of your mud. IE: Public, "
                      "Development, etc.%%^RESET%%^\r\n");
        i3_printf(ch, "%%^WHITE%%^Mudtype    : %%^GREEN%%^%%^BOLD%%^What you call the basic type of your "
                      "codebase.%%^RESET%%^\r\n");
        i3_printf(ch, "%%^WHITE%%^Mudlib     : %%^GREEN%%^%%^BOLD%%^What you call the current version of your "
                      "codebase.%%^RESET%%^\r\n");
        i3_printf(ch, "%%^WHITE%%^Minlevel   : %%^GREEN%%^%%^BOLD%%^Minimum level at which I3 will recognize your "
                      "players.%%^RESET%%^\r\n");
        i3_printf(ch, "%%^WHITE%%^Immlevel   : %%^GREEN%%^%%^BOLD%%^The level at which immortal commands become "
                      "available.%%^RESET%%^\r\n");
        i3_printf(ch, "%%^WHITE%%^Adminlevel : %%^GREEN%%^%%^BOLD%%^The level at which administrative commands become "
                      "available.%%^RESET%%^\r\n");
        i3_printf(ch, "%%^WHITE%%^Implevel   : %%^GREEN%%^%%^BOLD%%^The level at which implementor commands become "
                      "available.%%^RESET%%^\r\n");
        return;
    }

    if (!strcasecmp(arg, "show"))
    {
        i3_printf(ch, "%%^WHITE%%^Mudname       : %%^GREEN%%^%%^BOLD%%^%s%%^RESET%%^\r\n", this_i3mud->name);
        i3_printf(ch, "%%^WHITE%%^Autoconnect   : %%^GREEN%%^%%^BOLD%%^%s%%^RESET%%^\r\n",
                  this_i3mud->autoconnect == TRUE ? "Enabled" : "Disabled");
        i3_printf(ch, "%%^WHITE%%^Telnet        : %%^GREEN%%^%%^BOLD%%^%s:%d%%^RESET%%^\r\n", this_i3mud->telnet,
                  this_i3mud->player_port);
        i3_printf(ch, "%%^WHITE%%^Web           : %%^GREEN%%^%%^BOLD%%^%s%%^RESET%%^\r\n", this_i3mud->web);
        i3_printf(ch, "%%^WHITE%%^Email         : %%^GREEN%%^%%^BOLD%%^%s%%^RESET%%^\r\n", this_i3mud->admin_email);
        i3_printf(ch, "%%^WHITE%%^Status        : %%^GREEN%%^%%^BOLD%%^%s%%^RESET%%^\r\n", this_i3mud->open_status);
        i3_printf(ch, "%%^WHITE%%^Mudtype       : %%^GREEN%%^%%^BOLD%%^%s%%^RESET%%^\r\n", this_i3mud->mud_type);
        i3_printf(ch, "%%^WHITE%%^Mudlib        : %%^GREEN%%^%%^BOLD%%^%s%%^RESET%%^\r\n", this_i3mud->mudlib);
        i3_printf(ch, "%%^WHITE%%^Minlevel      : %%^GREEN%%^%%^BOLD%%^%d%%^RESET%%^\r\n", this_i3mud->minlevel);
        i3_printf(ch, "%%^WHITE%%^Immlevel      : %%^GREEN%%^%%^BOLD%%^%d%%^RESET%%^\r\n", this_i3mud->immlevel);
        i3_printf(ch, "%%^WHITE%%^Adminlevel    : %%^GREEN%%^%%^BOLD%%^%d%%^RESET%%^\r\n", this_i3mud->adminlevel);
        i3_printf(ch, "%%^WHITE%%^Implevel      : %%^GREEN%%^%%^BOLD%%^%d%%^RESET%%^\r\n", this_i3mud->implevel);
        return;
    }

    if (!strcasecmp(arg, "autoconnect"))
    {
        this_i3mud->autoconnect = !this_i3mud->autoconnect;

        if (this_i3mud->autoconnect)
            i3_printf(ch, "Autoconnect enabled.\r\n");
        else
            i3_printf(ch, "Autoconnect disabled.\r\n");
        I3_saveconfig();
        return;
    }

    if (!argument || argument[0] == '\0')
    {
        I3_setconfig(ch, "");
        return;
    }

    if (!strcasecmp(arg, "implevel") && I3PERM(ch) == I3PERM_IMP)
    {
        int value = atoi(argument);

        this_i3mud->implevel = value;
        I3_saveconfig();
        i3_printf(ch, "Implementor level changed to %d\r\n", value);
        return;
    }

    if (!strcasecmp(arg, "adminlevel"))
    {
        int value = atoi(argument);

        this_i3mud->adminlevel = value;
        I3_saveconfig();
        i3_printf(ch, "Admin level changed to %d\r\n", value);
        return;
    }

    if (!strcasecmp(arg, "immlevel"))
    {
        int value = atoi(argument);

        this_i3mud->immlevel = value;
        I3_saveconfig();
        i3_printf(ch, "Immortal level changed to %d\r\n", value);
        return;
    }

    if (!strcasecmp(arg, "minlevel"))
    {
        int value = atoi(argument);

        this_i3mud->minlevel = value;
        I3_saveconfig();
        i3_printf(ch, "Minimum level changed to %d\r\n", value);
        return;
    }

    if (is_connecting())
    {
        i3_printf(ch, "%s may not be changed while the mud is connected.\r\n", arg);
        return;
    }

    if (!strcasecmp(arg, "mudname"))
    {
        I3STRFREE(this_i3mud->name);
        this_i3mud->name = I3STRALLOC(argument);
        // I3_THISMUD = argument;
        unlink(I3_PASSWORD_FILE);
        I3_saveconfig();
        i3_printf(ch, "Mud name changed to %s\r\n", argument);
        return;
    }

    if (!strcasecmp(arg, "telnet"))
    {
        I3STRFREE(this_i3mud->telnet);
        this_i3mud->telnet = I3STRALLOC(argument);
        I3_saveconfig();
        i3_printf(ch, "Telnet address changed to %s:%d\r\n", argument, this_i3mud->player_port);
        return;
    }

    if (!strcasecmp(arg, "web"))
    {
        I3STRFREE(this_i3mud->web);
        this_i3mud->web = I3STRALLOC(argument);
        I3_saveconfig();
        i3_printf(ch, "Website changed to %s\r\n", argument);
        return;
    }

    if (!strcasecmp(arg, "email"))
    {
        I3STRFREE(this_i3mud->admin_email);
        this_i3mud->admin_email = I3STRALLOC(argument);
        I3_saveconfig();
        i3_printf(ch, "Admin email changed to %s\r\n", argument);
        return;
    }

    if (!strcasecmp(arg, "status"))
    {
        I3STRFREE(this_i3mud->open_status);
        this_i3mud->open_status = I3STRALLOC(argument);
        I3_saveconfig();
        i3_printf(ch, "Status changed to %s\r\n", argument);
        return;
    }

    if (!strcasecmp(arg, "mudlib"))
    {
        I3STRFREE(this_i3mud->mudlib);
        this_i3mud->mudlib = I3STRALLOC(argument);
        I3_saveconfig();
        i3_printf(ch, "Mudlib changed to %s\r\n", argument);
        return;
    }
    if (!strcasecmp(arg, "mudtype"))
    {
        I3STRFREE(this_i3mud->mud_type);
        this_i3mud->mud_type = I3STRALLOC(argument);
        I3_saveconfig();
        i3_printf(ch, "Mudtype changed to %s\r\n", argument);
        return;
    }

    I3_setconfig(ch, "");
}

I3_CMD(I3_permstats)
{
    CHAR_DATA *victim;

    if (!argument || argument[0] == '\0')
    {
        i3_printf(ch, "Usage: i3perms <user>\r\n");
        return;
    }

    if (!(victim = I3_find_user(argument)))
    {
        i3_printf(ch, "No such person is currently online.\r\n");
        return;
    }

    if (I3PERM(victim) < 0 || I3PERM(victim) > I3PERM_IMP)
    {
        i3_printf(ch, "%%^RED%%^%%^BOLD%%^%s has an invalid permission setting!%%^RESET%%^\r\n", CH_I3NAME(victim));
        return;
    }

    i3_printf(ch, "%%^GREEN%%^%%^BOLD%%^Permissions for %s: %s%%^RESET%%^\r\n", CH_I3NAME(victim),
              perm_names[I3PERM(victim)]);
    i3_printf(ch, "%%^GREEN%%^These permissions were obtained %s.%%^RESET%%^\r\n",
              I3IS_SET(I3FLAG(victim), I3_PERMOVERRIDE) ? "manually via i3permset" : "automatically by level");
}

I3_CMD(I3_permset)
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];
    int permvalue;

    argument = i3one_argument(argument, arg);

    if (arg[0] == '\0')
    {
        i3_printf(ch, "%%^WHITE%%^Usage: i3permset <user> <permission>%%^RESET%%^\r\n");
        i3_printf(ch, "%%^WHITE%%^Permission can be one of: None, Mort, Imm, Admin, Imp%%^RESET%%^\r\n");
        return;
    }

    if (!(victim = I3_find_user(arg)))
    {
        i3_printf(ch, "No such person is currently online.\r\n");
        return;
    }

    if (!strcasecmp(argument, "override"))
        permvalue = -1;
    else
    {
        permvalue = get_permvalue(argument);

        if (!i3check_permissions(ch, permvalue, I3PERM(victim), TRUE))
            return;
    }

    /*
     * Just something to avoid looping through the channel clean-up --Xorith
     */
    if (I3PERM(victim) == permvalue)
    {
        i3_printf(ch, "%s already has a permission level of %s.\r\n", CH_I3NAME(victim), perm_names[permvalue]);
        return;
    }

    if (permvalue == -1)
    {
        I3REMOVE_BIT(I3FLAG(victim), I3_PERMOVERRIDE);
        i3_printf(ch, "%%^YELLOW%%^Permission flag override has been removed from %s%%^RESET%%^\r\n",
                  CH_I3NAME(victim));
        return;
    }

    I3PERM(victim) = permvalue;
    I3SET_BIT(I3FLAG(victim), I3_PERMOVERRIDE);

    i3_printf(ch, "%%^YELLOW%%^Permission level for %s has been changed to %s%%^RESET%%^\r\n", CH_I3NAME(victim),
              perm_names[permvalue]);
    /*
     * Channel Clean-Up added by Xorith 9-24-03
     */
    /*
     * Note: Let's not clean up I3_DENY for a player. Never know...
     */
    if (I3LISTEN(victim) != NULL)
    {
        I3_CHANNEL *channel = NULL;
        const char *channels = I3LISTEN(victim);

        while (1)
        {
            if (channels[0] == '\0')
                break;
            channels = i3one_argument(channels, arg);

            if (!(channel = find_I3_channel_by_localname(arg)))
                I3_unflagchan(&I3LISTEN(victim), arg);
            if (channel && I3PERM(victim) < channel->i3perm)
            {
                I3_unflagchan(&I3LISTEN(victim), arg);
                i3_printf(ch,
                          "%%^WHITE%%^%%^BOLD%%^Removing '%s' level channel: '%s', exceeding new permission of "
                          "'%s'%%^RESET%%^\r\n",
                          perm_names[channel->i3perm], channel->local_name, perm_names[I3PERM(victim)]);
            }
        }
    }
}

I3_CMD(I3_who)
{
    I3_MUD *mud;

    if (!argument || argument[0] == '\0')
    {
        i3_printf(ch, "Usage: i3who <mudname>\r\n");
        return;
    }

    if (!(mud = find_I3_mud_by_name(argument)))
    {
        i3_printf(
            ch,
            "%%^YELLOW%%^No such mud known.%%^RESET%%^\r\n"
            "(use %%^WHITE%%^%%^BOLD%%^i3mudlist%%^YELLOW%%^ to get an overview of the muds available)%%^RESET%%^\r\n");
        return;
    }

    if (mud->status >= 0)
    {
        i3_printf(ch, "%s is marked as down.\r\n", mud->name);
        return;
    }

    if (mud->who == 0)
        i3_printf(ch, "%s does not support the 'who' command. Sending anyway.\r\n", mud->name);

    I3_send_who(ch, mud->name);
}

I3_CMD(I3_locate)
{
    if (!argument || argument[0] == '\0')
    {
        i3_printf(ch, "Usage: i3locate <person>\r\n");
        return;
    }
    I3_send_locate(ch, argument);
}

I3_CMD(I3_finger)
{
    char user[MAX_INPUT_LENGTH], mud[MAX_INPUT_LENGTH];
    char *ps;
    I3_MUD *pmud;

    if (I3IS_SET(I3FLAG(ch), I3_DENYFINGER))
    {
        i3_printf(ch, "You are not allowed to use i3finger.\r\n");
        return;
    }

    if (!argument || argument[0] == '\0')
    {
        i3_printf(ch, "%%^WHITE%%^Usage: i3finger <user@mud>%%^RESET%%^\r\n");
        i3_printf(ch, "%%^WHITE%%^Usage: i3finger privacy%%^RESET%%^\r\n");
        return;
    }

    if (!strcasecmp(argument, "privacy"))
    {
        if (!I3IS_SET(I3FLAG(ch), I3_PRIVACY))
        {
            I3SET_BIT(I3FLAG(ch), I3_PRIVACY);
            i3_printf(ch, "I3 finger privacy flag set.\r\n");
            return;
        }
        I3REMOVE_BIT(I3FLAG(ch), I3_PRIVACY);
        i3_printf(ch, "I3 finger privacy flag removed.\r\n");
        return;
    }

    if (I3ISINVIS(ch))
    {
        i3_printf(ch, "You are invisible.\r\n");
        return;
    }

    if ((ps = (char *)strchr(argument, '@')) == NULL)
    {
        i3_printf(
            ch,
            "%%^YELLOW%%^You should specify a person and a mud.%%^RESET%%^\r\n"
            "(use %%^WHITE%%^%%^BOLD%%^i3mudlist%%^YELLOW%%^ to get an overview of the muds available)%%^RESET%%^\r\n");
        return;
    }

    ps[0] = '\0';
    strlcpy(user, argument, MAX_INPUT_LENGTH);
    strlcpy(mud, ps + 1, MAX_INPUT_LENGTH);

    if (user[0] == '\0' || mud[0] == '\0')
    {
        i3_printf(
            ch,
            "%%^YELLOW%%^You should specify a person and a mud.%%^RESET%%^\r\n"
            "(use %%^WHITE%%^%%^BOLD%%^i3mudlist%%^YELLOW%%^ to get an overview of the muds available)%%^RESET%%^\r\n");
        return;
    }

    if (!(pmud = find_I3_mud_by_name(mud)))
    {
        i3_printf(
            ch,
            "%%^YELLOW%%^No such mud known.%%^RESET%%^\r\n"
            "(use %%^WHITE%%^%%^BOLD%%^i3mudlist%%^YELLOW%%^ to get an overview of the muds available)%%^RESET%%^\r\n");
        return;
    }

#if 0
    if (!strcasecmp(this_i3mud->name, pmud->name)) {
	i3_printf(ch, "Use your mud's own internal system for that.\r\n");
	return;
    }
#endif

    if (pmud->status >= 0)
    {
        i3_printf(ch, "%s is marked as down.\r\n", pmud->name);
        return;
    }

    if (pmud->finger == 0)
        i3_printf(ch, "%s does not support the 'finger' command. Sending anyway.\r\n", pmud->name);

    I3_send_finger(ch, user, pmud->name);
}

I3_CMD(I3_emote)
{
    char to[MAX_INPUT_LENGTH], *ps;
    char mud[MAX_INPUT_LENGTH];
    I3_MUD *pmud;

    if (I3ISINVIS(ch))
    {
        i3_printf(ch, "You are invisible.\r\n");
        return;
    }

    if (!argument || argument[0] == '\0')
    {
        i3_printf(ch, "Usage: i3emoteto <person@mud> <emote message>\r\n");
        return;
    }

    argument = i3one_argument(argument, to);
    ps = (char *)strchr(to, '@');

    if (to[0] == '\0' || argument[0] == '\0' || ps == NULL)
    {
        i3_printf(
            ch,
            "%%^YELLOW%%^You should specify a person and a mud.%%^RESET%%^\r\n"
            "(use %%^WHITE%%^%%^BOLD%%^i3mudlist%%^YELLOW%%^ to get an overview of the muds available)%%^RESET%%^\r\n");
        return;
    }

    ps[0] = '\0';
    ps++;
    strlcpy(mud, ps, MAX_INPUT_LENGTH);

    if (!(pmud = find_I3_mud_by_name(mud)))
    {
        i3_printf(ch, "%%^YELLOW%%^No such mud known.%%^RESET%%^\r\n"
                      "( use %%^WHITE%%^%%^BOLD%%^i3mudlist%%^YELLOW%%^ to get an overview of the muds "
                      "available)%%^RESET%%^\r\n");
        return;
    }

    if (pmud->status >= 0)
    {
        i3_printf(ch, "%s is marked as down.\r\n", pmud->name);
        return;
    }

    if (pmud->emoteto == 0)
        i3_printf(ch, "%s does not support the 'emoteto' command. Sending anyway.\r\n", pmud->name);

    I3_send_emoteto(ch, to, pmud, argument);
}

I3_CMD(I3_router)
{
    ROUTER_DATA *router;
    char cmd[MAX_INPUT_LENGTH];

    if (!argument || argument[0] == '\0')
    {
        i3_printf(ch, "%%^WHITE%%^Usage: i3router add <router_name> <router_ip> <router_port>%%^RESET%%^\r\n");
        i3_printf(ch, "%%^WHITE%%^Usage: i3router remove <router_name>%%^RESET%%^\r\n");
        i3_printf(ch, "%%^WHITE%%^Usage: i3router select <router_name>%%^RESET%%^\r\n");
        i3_printf(ch, "%%^WHITE%%^Usage: i3router list%%^RESET%%^\r\n");
        return;
    }
    argument = i3one_argument(argument, cmd);

    if (!strcasecmp(cmd, "list"))
    {
        i3_printf(ch, "%%^RED%%^%%^BOLD%%^The mud has the following routers configured:%%^RESET%%^\r\n");
        i3_printf(ch, "%%^WHITE%%^%%^BOLD%%^Router Name     Router IP/DNS                  Router Port%%^RESET%%^\r\n");
        for (router = first_router; router; router = router->next)
            i3_printf(ch, "%%^CYAN%%^%-15.15s %%^CYAN%%^%-30.30s %d%%^RESET%%^\r\n", router->name, router->ip,
                      router->port);
        return;
    }

    if (!argument || argument[0] == '\0')
    {
        I3_router(ch, "");
        return;
    }

    if (!strcasecmp(cmd, "remove"))
    {
        for (router = first_router; router; router = router->next)
        {
            if (!strcasecmp(router->name, argument) || !strcasecmp(router->ip, argument))
            {
                I3STRFREE(router->name);
                I3STRFREE(router->ip);
                I3UNLINK(router, first_router, last_router, next, prev);
                I3DISPOSE(router);
                i3_printf(ch,
                          "%%^YELLOW%%^Router %%^WHITE%%^%%^BOLD%%^%s%%^YELLOW%%^ has been removed from your "
                          "configuration.%%^RESET%%^\r\n",
                          argument);
                I3_saverouters();
                return;
            }
        }
        i3_printf(ch,
                  "%%^YELLOW%%^No router named %%^WHITE%%^%%^BOLD%%^%s%%^YELLOW%%^ exists in your "
                  "configuration.%%^RESET%%^\r\n",
                  argument);
        return;
    }

    if (!strcasecmp(cmd, "select"))
    {
        for (router = first_router; router; router = router->next)
        {
            if (!strcasecmp(router->name, argument) || !strcasecmp(router->ip, argument))
            {
                // Remove the router from where it was
                I3UNLINK(router, first_router, last_router, next, prev);
                // Add it to the top
                I3HEADLINK(router, first_router, last_router, next, prev);

                i3_printf(
                    ch,
                    "%%^YELLOW%%^Router %%^WHITE%%^%%^BOLD%%^%s%%^YELLOW%%^ has been selected for use.%%^RESET%%^\r\n",
                    argument);
                I3_saverouters();
                return I3_reload(ch, argument);
            }
        }
        i3_printf(ch,
                  "%%^YELLOW%%^No router named %%^WHITE%%^%%^BOLD%%^%s%%^YELLOW%%^ exists in your "
                  "configuration.%%^RESET%%^\r\n",
                  argument);
        return;
    }

    if (!strcasecmp(cmd, "add"))
    {
        ROUTER_DATA *temp;
        char rtname[MAX_INPUT_LENGTH];
        char rtip[MAX_INPUT_LENGTH];
        int rtport;

        argument = i3one_argument(argument, rtname);
        argument = i3one_argument(argument, rtip);

        if (rtname[0] == '\0' || rtip[0] == '\0' || !argument || argument[0] == '\0')
        {
            I3_router(ch, "");
            return;
        }

        if (rtname[0] != '*')
        {
            i3_printf(ch, "%%^YELLOW%%^A router name must begin with a %%^WHITE%%^%%^BOLD%%^*%%^YELLOW%%^ to be "
                          "valid.%%^RESET%%^\r\n");
            return;
        }

        for (temp = first_router; temp; temp = temp->next)
        {
            if (!strcasecmp(temp->name, rtname))
            {
                i3_printf(ch,
                          "%%^YELLOW%%^A router named %%^WHITE%%^%%^BOLD%%^%s%%^YELLOW%%^ is already in your "
                          "configuration.%%^RESET%%^\r\n",
                          rtname);
                return;
            }
        }

        if (!is_number(argument))
        {
            i3_printf(ch, "%%^YELLOW%%^Port must be a numerical value.%%^RESET%%^\r\n");
            return;
        }

        rtport = atoi(argument);
        if (rtport < 1 || rtport > 65535)
        {
            i3_printf(ch, "%%^YELLOW%%^Invalid port value specified.%%^RESET%%^\r\n");
            return;
        }

        I3CREATE(router, ROUTER_DATA, 1);
        router->name = I3STRALLOC(rtname);
        router->ip = I3STRALLOC(rtip);
        router->port = rtport;
        router->reconattempts = 0;
        I3LINK(router, first_router, last_router, next, prev);

        i3_printf(ch,
                  "%%^YELLOW%%^Router: %%^WHITE%%^%%^BOLD%%^%s %s %d%%^YELLOW%%^ has been added to your "
                  "configuration.%%^RESET%%^\r\n",
                  router->name, router->ip, router->port);
        I3_saverouters();
        return;
    }
    I3_router(ch, "");
}

I3_CMD(I3_stats)
{
    I3_MUD *mud;
    I3_CHANNEL *channel;
    int mud_count = 0, chan_count = 0;

    for (mud = first_mud; mud; mud = mud->next)
        mud_count++;

    for (channel = first_I3chan; channel; channel = channel->next)
        chan_count++;

    i3_printf(ch, "%%^CYAN%%^General Statistics:%%^RESET%%^\r\n\r\n");

    if (is_connected())
        i3_printf(ch, "%%^CYAN%%^Currently connected to     : %%^WHITE%%^%%^BOLD%%^%s (%s)%%^RESET%%^\r\n",
                  I3_ROUTER_NAME, I3_ROUTER_IP);
    else if (is_connecting())
        i3_printf(ch, "%%^CYAN%%^Currently connecting to    : %%^YELLOW%%^%%^BOLD%%^%s (%s)%%^RESET%%^\r\n",
                  I3_ROUTER_NAME, I3_ROUTER_IP);
    else
        i3_printf(ch, "%%^CYAN%%^Currently connecting to    : %%^RED%%^%%^BOLD%%^%s%%^RESET%%^\r\n", "Nowhere");

    if (is_connected())
        i3_printf(ch, "%%^CYAN%%^Connected on descriptor    : %%^WHITE%%^%%^BOLD%%^%d%%^RESET%%^\r\n", I3_socket);
    else if (is_connecting())
        i3_printf(ch, "%%^CYAN%%^Connecting on descriptor   : %%^YELLOW%%^%%^BOLD%%^%d%%^RESET%%^\r\n", I3_socket);

    if (is_connected())
    {
        i3_printf(ch, "%%^CYAN%%^Connected for              : %%^WHITE%%^%%^BOLD%%^%s%%^RESET%%^\r\n",
                  time_elapsed(connected_at, 0));
        i3_printf(
            ch, "%%^CYAN%%^Time to connect            : %%^WHITE%%^%%^BOLD%%^%s%%^RESET%%^\r\n",
            time_elapsed(connect_started_at - ((I3_TIMEOUT_DELAY / 1000000) * connection_timeouts), connected_at));
    }
    else if (is_connecting())
    {
        i3_printf(ch, "%%^CYAN%%^Connecting for             : %%^YELLOW%%^%%^BOLD%%^%s%%^RESET%%^\r\n",
                  time_elapsed(connect_started_at - ((I3_TIMEOUT_DELAY / 1000000) * connection_timeouts), 0));
    }

    if (expecting_timeout)
    {
        i3_printf(ch, "%%^CYAN%%^Time remaining             : %%^YELLOW%%^%%^BOLD%%^%s%%^RESET%%^\r\n",
                  time_elapsed(connect_started_at, (int)(diffTimestamp(timeout_marker, -1) / (int64_t)1000000)));
    }

    if (connection_timeouts > 0)
    {
        i3_printf(ch, "%%^CYAN%%^Connection timeouts        : %%^YELLOW%%^%%^BOLD%%^%s (%d)%%^RESET%%^\r\n",
                  time_elapsed(0, (I3_TIMEOUT_DELAY / 1000000) * connection_timeouts), connection_timeouts);
    }

    if (record_uptime > 0)
    {
        i3_printf(ch, "%%^CYAN%%^Longest Uptime             : %%^GREEN%%^%%^BOLD%%^%s%%^RESET%%^\r\n",
                  time_elapsed(0, record_uptime));
    }

    if (record_lag_spike > 0)
    {
        i3_printf(ch, "%%^CYAN%%^Longest LAG Spike          : %%^RED%%^%%^BOLD%%^%s%%^RESET%%^\r\n",
                  time_elapsed(record_lag_spike, 0));
    }

    i3_printf(ch, "%%^CYAN%%^Bytes sent                 : %%^WHITE%%^%%^BOLD%%^%ld%%^RESET%%^\r\n", bytes_sent);
    i3_printf(ch, "%%^CYAN%%^Bytes received             : %%^WHITE%%^%%^BOLD%%^%ld%%^RESET%%^\r\n", bytes_received);
    i3_printf(ch, "%%^CYAN%%^Channel messages sent      : %%^WHITE%%^%%^BOLD%%^%ld%%^RESET%%^\r\n", channel_m_sent);
    i3_printf(ch, "%%^CYAN%%^Channel messages received  : %%^WHITE%%^%%^BOLD%%^%ld%%^RESET%%^\r\n", channel_m_received);
    i3_printf(ch, "%%^CYAN%%^Known muds                 : %%^WHITE%%^%%^BOLD%%^%d%%^RESET%%^\r\n", mud_count);
    i3_printf(ch, "%%^CYAN%%^Known channels             : %%^WHITE%%^%%^BOLD%%^%d%%^RESET%%^\r\n", chan_count);
}

I3_CMD(i3_help)
{
    I3_HELP_DATA *help;
    char buf[MAX_STRING_LENGTH];
    int col, perm;

    if (!argument || argument[0] == '\0')
    {
        strlcpy(buf, "%^GREEN%^Help is available for the following commands:%^RESET%^\r\n", MAX_STRING_LENGTH);
        strlcat(buf, "%^GREEN%^%^BOLD%^---------------------------------------------%^RESET%^\r\n", MAX_STRING_LENGTH);
        for (perm = I3PERM_MORT; perm <= I3PERM(ch); perm++)
        {
            col = 0;
            snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf),
                     "%%^RESET%%^\r\n%%^GREEN%%^%s helps:%%^RESET%%^\r\n", perm_names[perm]);
            for (help = first_i3_help; help; help = help->next)
            {
                if (help->level != perm)
                    continue;

                snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "%-15s", help->name);
                if (++col % 5 == 0)
                    strlcat(buf, "%^RESET%^\r\n", MAX_STRING_LENGTH);
            }
            if (col % 5 != 0)
                strlcat(buf, "%^RESET%^\r\n", MAX_STRING_LENGTH);
        }
        strlcat(buf, "%^RESET%^\r\n", MAX_STRING_LENGTH);
        i3page_printf(ch, "%s", buf);
        return;
    }

    for (help = first_i3_help; help; help = help->next)
    {
        if (!strcasecmp(help->name, argument))
        {
            if (!help->text || help->text[0] == '\0')
                i3_printf(
                    ch,
                    "%%^GREEN%%^No inforation available for topic %%^WHITE%%^%%^BOLD%%^%s%%^GREEN%%^.%%^RESET%%^\r\n",
                    help->name);
            else
                i3_printf(ch, "%%^GREEN%%^%s%%^RESET%%^\r\n", help->text);
            return;
        }
    }
    i3_printf(ch, "%%^GREEN%%^No help exists for topic %%^WHITE%%^%%^BOLD%%^%s%%^GREEN%%^.%%^RESET%%^\r\n", argument);
}

I3_CMD(i3_hedit)
{
    I3_HELP_DATA *help;
    char name[MAX_INPUT_LENGTH], cmd[MAX_INPUT_LENGTH];
    bool found = FALSE;

    argument = i3one_argument(argument, name);
    argument = i3one_argument(argument, cmd);

    if (name[0] == '\0' || cmd[0] == '\0' || !argument || argument[0] == '\0')
    {
        i3_printf(ch, "%%^WHITE%%^Usage: i3hedit <topic> [name|perm] <field>%%^RESET%%^\r\n");
        i3_printf(ch, "%%^WHITE%%^Where <field> can be either name, or permission level.%%^RESET%%^\r\n");
        return;
    }

    for (help = first_i3_help; help; help = help->next)
    {
        if (!strcasecmp(help->name, name))
        {
            found = TRUE;
            break;
        }
    }

    if (!found)
    {
        i3_printf(ch,
                  "%%^GREEN%%^No help exists for topic %%^WHITE%%^%%^BOLD%%^%s%%^GREEN%%^. You will need to add it to "
                  "the helpfile manually.%%^RESET%%^\r\n",
                  name);
        return;
    }

    if (!strcasecmp(cmd, "name"))
    {
        i3_printf(ch, "%%^WHITE%%^%%^BOLD%%^%s %%^GREEN%%^has been renamed to %%^WHITE%%^%%^BOLD%%^%s.%%^RESET%%^\r\n",
                  help->name, argument);
        I3STRFREE(help->name);
        help->name = I3STRALLOC(argument);
        I3_savehelps();
        return;
    }

    if (!strcasecmp(cmd, "perm"))
    {
        int permvalue = get_permvalue(argument);

        if (!i3check_permissions(ch, permvalue, help->level, FALSE))
            return;

        i3_printf(ch,
                  "%%^GREEN%%^Permission level for %%^WHITE%%^%%^BOLD%%^%s %%^GREEN%%^has been changed to "
                  "%%^WHITE%%^%%^BOLD%%^%s.%%^RESET%%^\r\n",
                  help->name, perm_names[permvalue]);
        help->level = permvalue;
        I3_savehelps();
        return;
    }
    i3_hedit(ch, "");
}

I3_CMD(I3_other)
{
    I3_CMD_DATA *cmd;
    char buf[MAX_STRING_LENGTH];
    int col, perm;

    strlcpy(buf, "%^GREEN%^The following commands are available:%^RESET%^\r\n", MAX_STRING_LENGTH);
    strlcat(buf, "%^GREEN%^%^BOLD%^-------------------------------------%^RESET%^\r\n", MAX_STRING_LENGTH);
    for (perm = I3PERM_MORT; perm <= I3PERM(ch); perm++)
    {
        col = 0;
        snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf),
                 "%%^RESET%%^\r\n%%^GREEN%%^%s commands:%%^GREEN%%^%%^BOLD%%^\r\n", perm_names[perm]);
        for (cmd = first_i3_command; cmd; cmd = cmd->next)
        {
            if (cmd->level != perm)
                continue;

            snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "%-15s", cmd->name);
            if (++col % 5 == 0)
                strlcat(buf, "%^RESET%^\r\n%^GREEN%^%^BOLD%^", MAX_STRING_LENGTH);
        }
        if (col % 5 != 0)
            strlcat(buf, "%^RESET%^\r\n%^GREEN%^%^BOLD%^", MAX_STRING_LENGTH);
    }
    i3page_printf(ch, "%s", buf);
    i3page_printf(ch, "%%^RESET%%^\r\n%%^GREEN%%^For information about a specific command, see "
                      "%%^WHITE%%^%%^BOLD%%^i3help <command>%%^GREEN%%^.%%^RESET%%^\r\n");
}

I3_CMD(I3_afk)
{
    if (I3IS_SET(I3FLAG(ch), I3_AFK))
    {
        I3REMOVE_BIT(I3FLAG(ch), I3_AFK);
        i3_printf(ch, "You are no longer AFK to I3.\r\n");
    }
    else
    {
        I3SET_BIT(I3FLAG(ch), I3_AFK);
        i3_printf(ch, "You are now AFK to I3.\r\n");
    }
}

I3_CMD(I3_color)
{
    if (I3IS_SET(I3FLAG(ch), I3_COLORFLAG))
    {
        I3REMOVE_BIT(I3FLAG(ch), I3_COLORFLAG);
        i3_printf(ch, "I3 color is now off.\r\n");
    }
    else
    {
        I3SET_BIT(I3FLAG(ch), I3_COLORFLAG);
        i3_printf(
            ch,
            "%%^RED%%^%%^BOLD%%^I3 c%%^YELLOW%%^o%%^GREEN%%^%%^BOLD%%^l%%^BLUE%%^%%^BOLD%%^o%%^MAGENTA%%^%%^BOLD%%^r "
            "%%^RED%%^%%^BOLD%%^is now on. Enjoy :)%%^RESET%%^\r\n");
    }
}

I3_CMD(i3_cedit)
{
    I3_CMD_DATA *cmd, *tmp;
    I3_ALIAS *alias, *alias_next;
    char name[MAX_INPUT_LENGTH], option[MAX_INPUT_LENGTH];
    bool found = FALSE, aliasfound = FALSE;

    argument = i3one_argument(argument, name);
    argument = i3one_argument(argument, option);

    if (name[0] == '\0' || option[0] == '\0')
    {
        i3_printf(ch, "Usage: i3cedit <command> <create|delete|alias|rename|code|permission|connected> <field>.\r\n");
        return;
    }

    for (cmd = first_i3_command; cmd; cmd = cmd->next)
    {
        if (!strcasecmp(cmd->name, name))
        {
            found = TRUE;
            break;
        }
        for (alias = cmd->first_alias; alias; alias = alias->next)
        {
            if (!strcasecmp(alias->name, name))
                aliasfound = TRUE;
        }
    }

    if (!strcasecmp(option, "create"))
    {
        if (found)
        {
            i3_printf(ch,
                      "%%^GREEN%%^A command named %%^WHITE%%^%%^BOLD%%^%s %%^GREEN%%^already exists.%%^RESET%%^\r\n",
                      name);
            return;
        }

        if (aliasfound)
        {
            i3_printf(ch, "%%^GREEN%%^%s already exists as an alias for another command.%%^RESET%%^\r\n", name);
            return;
        }

        I3CREATE(cmd, I3_CMD_DATA, 1);
        cmd->name = I3STRALLOC(name);
        cmd->level = I3PERM(ch);
        cmd->connected = FALSE;
        i3_printf(ch, "%%^GREEN%%^Command %%^WHITE%%^%%^BOLD%%^%s %%^GREEN%%^created.%%^RESET%%^\r\n", cmd->name);
        if (argument && argument[0] != '\0')
        {
            cmd->function = i3_function(argument);
            if (cmd->function == NULL)
                i3_printf(ch,
                          "%%^GREEN%%^Function %%^WHITE%%^%%^BOLD%%^%s %%^GREEN%%^does not exist - set to "
                          "NULL.%%^RESET%%^\r\n",
                          argument);
        }
        else
        {
            i3_printf(ch, "%%^GREEN%%^Function set to NULL.%%^RESET%%^\r\n");
            cmd->function = NULL;
        }
        I3LINK(cmd, first_i3_command, last_i3_command, next, prev);
        I3_savecommands();
        return;
    }

    if (!found)
    {
        i3_printf(ch, "%%^GREEN%%^No command named %%^WHITE%%^%%^BOLD%%^%s %%^GREEN%%^exists.%%^RESET%%^\r\n", name);
        return;
    }

    if (!i3check_permissions(ch, cmd->level, cmd->level, FALSE))
        return;

    if (!strcasecmp(option, "delete"))
    {
        i3_printf(ch, "%%^GREEN%%^Command %%^WHITE%%^%%^BOLD%%^%s %%^GREEN%%^has been deleted.%%^RESET%%^\r\n",
                  cmd->name);
        for (alias = cmd->first_alias; alias; alias = alias_next)
        {
            alias_next = alias->next;

            I3UNLINK(alias, cmd->first_alias, cmd->last_alias, next, prev);
            I3STRFREE(alias->name);
            I3DISPOSE(alias);
        }
        I3UNLINK(cmd, first_i3_command, last_i3_command, next, prev);
        I3STRFREE(cmd->name);
        I3DISPOSE(cmd);
        I3_savecommands();
        return;
    }

    /*
     * MY GOD! What an inefficient mess you've made Samson!
     */
    if (!strcasecmp(option, "alias"))
    {
        for (alias = cmd->first_alias; alias; alias = alias_next)
        {
            alias_next = alias->next;

            if (!strcasecmp(alias->name, argument))
            {
                i3_printf(ch,
                          "%%^WHITE%%^%%^BOLD%%^%s %%^GREEN%%^has been removed as an alias for "
                          "%%^WHITE%%^%%^BOLD%%^%s%%^RESET%%^\r\n",
                          argument, cmd->name);
                I3UNLINK(alias, cmd->first_alias, cmd->last_alias, next, prev);
                I3STRFREE(alias->name);
                I3DISPOSE(alias);
                I3_savecommands();
                return;
            }
        }

        for (tmp = first_i3_command; tmp; tmp = tmp->next)
        {
            if (!strcasecmp(tmp->name, argument))
            {
                i3_printf(ch, "%%^WHITE%%^%%^BOLD%%^%s %%^GREEN%%^is already a command name.%%^RESET%%^\r\n", argument);
                return;
            }
            for (alias = tmp->first_alias; alias; alias = alias->next)
            {
                if (!strcasecmp(argument, alias->name))
                {
                    i3_printf(ch,
                              "%%^WHITE%%^%%^BOLD%%^%s %%^GREEN%%^is already an alias for "
                              "%%^WHITE%%^%%^BOLD%%^%s%%^RESET%%^\r\n",
                              argument, tmp->name);
                    return;
                }
            }
        }

        I3CREATE(alias, I3_ALIAS, 1);
        alias->name = I3STRALLOC(argument);
        I3LINK(alias, cmd->first_alias, cmd->last_alias, next, prev);
        i3_printf(
            ch,
            "%%^WHITE%%^%%^BOLD%%^%s %%^GREEN%%^has been added as an alias for %%^WHITE%%^%%^BOLD%%^%s%%^RESET%%^\r\n",
            alias->name, cmd->name);
        I3_savecommands();
        return;
    }

    if (!strcasecmp(option, "connected"))
    {
        cmd->connected = !cmd->connected;

        if (cmd->connected)
            i3_printf(ch,
                      "%%^GREEN%%^Command %%^WHITE%%^%%^BOLD%%^%s %%^GREEN%%^will now require a connection to I3 to "
                      "use.%%^RESET%%^\r\n",
                      cmd->name);
        else
            i3_printf(ch,
                      "%%^GREEN%%^Command %%^WHITE%%^%%^BOLD%%^%s %%^GREEN%%^will no longer require a connection to I3 "
                      "to use.%%^RESET%%^\r\n",
                      cmd->name);
        I3_savecommands();
        return;
    }

    if (!strcasecmp(option, "show"))
    {
        char buf[MAX_STRING_LENGTH];

        i3_printf(ch, "%%^GREEN%%^Command       : %%^WHITE%%^%%^BOLD%%^%s%%^RESET%%^\r\n", cmd->name);
        i3_printf(ch, "%%^GREEN%%^Permission    : %%^WHITE%%^%%^BOLD%%^%s%%^RESET%%^\r\n", perm_names[cmd->level]);
        i3_printf(ch, "%%^GREEN%%^Function      : %%^WHITE%%^%%^BOLD%%^%s%%^RESET%%^\r\n", i3_funcname(cmd->function));
        i3_printf(ch, "%%^GREEN%%^Connection Req: %%^WHITE%%^%%^BOLD%%^%s%%^RESET%%^\r\n",
                  cmd->connected ? "Yes" : "No");
        if (cmd->first_alias)
        {
            int col = 0;

            strlcpy(buf, "%^GREEN%^Aliases       : %^WHITE%^%^BOLD%^", MAX_STRING_LENGTH);
            for (alias = cmd->first_alias; alias; alias = alias->next)
            {
                snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "%s ", alias->name);
                if (++col % 10 == 0)
                    strlcat(buf, "\r\n", MAX_STRING_LENGTH);
            }
            if (col % 10 != 0)
                strlcat(buf, "\r\n", MAX_STRING_LENGTH);
            i3_printf(ch, "%s%%^RESET%%^", buf);
        }
        return;
    }

    if (!argument || argument[0] == '\0')
    {
        i3_printf(ch, "Required argument missing.\r\n");
        i3_cedit(ch, "");
        return;
    }

    if (!strcasecmp(option, "rename"))
    {
        i3_printf(ch,
                  "%%^GREEN%%^Command %%^WHITE%%^%%^BOLD%%^%s %%^GREEN%%^has been renamed to "
                  "%%^WHITE%%^%%^BOLD%%^%s.%%^RESET%%^\r\n",
                  cmd->name, argument);
        I3STRFREE(cmd->name);
        cmd->name = I3STRALLOC(argument);
        I3_savecommands();
        return;
    }

    if (!strcasecmp(option, "code"))
    {
        cmd->function = i3_function(argument);
        if (cmd->function == NULL)
            i3_printf(
                ch,
                "%%^GREEN%%^Function %%^WHITE%%^%%^BOLD%%^%s %%^GREEN%%^does not exist - set to NULL.%%^RESET%%^\r\n",
                argument);
        else
            i3_printf(ch, "%%^GREEN%%^Function set to %%^WHITE%%^%%^BOLD%%^%s.%%^RESET%%^\r\n", argument);
        I3_savecommands();
        return;
    }

    if (!strcasecmp(option, "perm") || !strcasecmp(option, "permission"))
    {
        int permvalue = get_permvalue(argument);

        if (!i3check_permissions(ch, permvalue, cmd->level, FALSE))
            return;

        cmd->level = permvalue;
        i3_printf(ch,
                  "%%^GREEN%%^Command %%^WHITE%%^%%^BOLD%%^%s %%^GREEN%%^permission level has been changed to "
                  "%%^WHITE%%^%%^BOLD%%^%s.%%^RESET%%^\r\n",
                  cmd->name, perm_names[permvalue]);
        I3_savecommands();
        return;
    }
    i3_cedit(ch, "");
}

char *I3_find_social(CHAR_DATA *ch, char *sname, char *person, char *mud, bool victim)
{
    static char socname[MAX_STRING_LENGTH];

    socname[0] = '\0';
    i3_printf(ch, "~YSocial ~W%s~Y does not exist on this mud.~!\r\n", sname);

#if 0
    SOCIALTYPE                             *social;
    char                                   *c;

    socname[0] = '\0';

    for (c = sname; *c; *c = tolower(*c), c++);

    if (!(social = find_social(sname))) {
	i3_printf(ch, "%%^YELLOW%%^Social %%^WHITE%%^%%^BOLD%%^%s%%^YELLOW%%^ does not exist on this mud.%%^RESET%%^\r\n", sname);
	return socname;
    }

    if (person && person[0] != '\0' && mud && mud[0] != '\0') {
	if (person && person[0] != '\0' && !strcasecmp(person, CH_I3NAME(ch))
	    && mud && mud[0] != '\0' && !strcasecmp(mud, this_i3mud->name)) {
	    if (!social->others_auto) {
		i3_printf(ch, "%%^YELLOW%%^Social %%^WHITE%%^%%^BOLD%%^%s%%^YELLOW%%^: Missing others_auto.%%^RESET%%^\r\n", social->name);
		return socname;
	    }
	    strlcpy(socname, social->others_auto, MAX_STRING_LENGTH);
	} else {
	    if (!victim) {
		if (!social->others_found) {
		    i3_printf(ch, "%%^YELLOW%%^Social %%^WHITE%%^%%^BOLD%%^%s%%^YELLOW%%^: Missing others_found.%%^RESET%%^\r\n", social->name);
		    return socname;
		}
		strlcpy(socname, social->others_found, MAX_STRING_LENGTH);
	    } else {
		if (!social->vict_found) {
		    i3_printf(ch, "%%^YELLOW%%^Social %%^WHITE%%^%%^BOLD%%^%s%%^YELLOW%%^: Missing vict_found.%%^RESET%%^\r\n", social->name);
		    return socname;
		}
		strlcpy(socname, social->vict_found, MAX_STRING_LENGTH);
	    }
	}
    } else {
	if (!social->others_no_arg) {
	    i3_printf(ch, "%%^YELLOW%%^Social %%^WHITE%%^%%^BOLD%%^%s%%^YELLOW%%^: Missing others_no_arg.%%^RESET%%^\r\n", social->name);
	    return socname;
	}
	strlcpy(socname, social->others_no_arg, MAX_STRING_LENGTH);
    }
#endif
    return socname;
}

/* Revised 10/10/03 by Xorith: Recognize the need to capitalize for a new sentence. */
char *i3act_string(const char *format, CHAR_DATA *ch, CHAR_DATA *vic)
{
    static const char *he_she[] = {"it", "he", "she"};
    static const char *him_her[] = {"it", "him", "her"};
    static const char *his_her[] = {"its", "his", "her"};
    static char buf[MAX_STRING_LENGTH];
    char tmp_str[MAX_STRING_LENGTH];
    const char *i = "";
    char *point;
    bool should_upper = FALSE;

    if (!format || format[0] == '\0' || !ch)
        return NULL;

    point = buf;

    while (*format != '\0')
    {
        if (*format == '.' || *format == '?' || *format == '!')
            should_upper = TRUE;
        else if (should_upper == TRUE && !isspace(*format) && *format != '$')
            should_upper = FALSE;

        if (*format != '$')
        {
            *point++ = *format++;
            continue;
        }
        ++format;

        if ((!vic) && (*format == 'N' || *format == 'E' || *format == 'M' || *format == 'S' || *format == 'K'))
            i = " !!!!! ";
        else
        {
            switch (*format)
            {
            default:
                i = " !!!!! ";
                break;
            case 'n':
                i = "$N";
                break;
            case 'N':
                i = "$O";
                break;
            case 'e':
                i = should_upper ? i3capitalize(he_she[URANGE(0, CH_I3SEX(ch), 2)])
                                 : he_she[URANGE(0, CH_I3SEX(ch), 2)];
                break;

            case 'E':
                i = should_upper ? i3capitalize(he_she[URANGE(0, CH_I3SEX(vic), 2)])
                                 : he_she[URANGE(0, CH_I3SEX(vic), 2)];
                break;

            case 'm':
                i = should_upper ? i3capitalize(him_her[URANGE(0, CH_I3SEX(ch), 2)])
                                 : him_her[URANGE(0, CH_I3SEX(ch), 2)];
                break;

            case 'M':
                i = should_upper ? i3capitalize(him_her[URANGE(0, CH_I3SEX(vic), 2)])
                                 : him_her[URANGE(0, CH_I3SEX(vic), 2)];
                break;

            case 's':
                i = should_upper ? i3capitalize(his_her[URANGE(0, CH_I3SEX(ch), 2)])
                                 : his_her[URANGE(0, CH_I3SEX(ch), 2)];
                break;

            case 'S':
                i = should_upper ? i3capitalize(his_her[URANGE(0, CH_I3SEX(vic), 2)])
                                 : his_her[URANGE(0, CH_I3SEX(vic), 2)];
                break;

            case 'k':
                i3one_argument(CH_I3NAME(ch), tmp_str);
                i = (char *)tmp_str;
                break;
            case 'K':
                i3one_argument(CH_I3NAME(vic), tmp_str);
                i = (char *)tmp_str;
                break;
                break;
            }
        }
        ++format;
        while ((*point = *i) != '\0')
            ++point, ++i;
    }
    *point = 0;
    point++;
    *point = '\0';

    buf[0] = UPPER(buf[0]);
    return buf;
}

CHAR_DATA *I3_make_skeleton(char *name)
{
    CHAR_DATA *skeleton;

    CREATE(skeleton, CHAR_DATA, 1);

    skeleton->player.name = I3STRALLOC(name);
    skeleton->player.short_descr = I3STRALLOC(name);
    skeleton->in_room = 1; /* LIMBO */

    return skeleton;
}

void I3_purge_skeleton(CHAR_DATA *skeleton)
{
    if (!skeleton)
        return;

    I3STRFREE(skeleton->player.name);
    I3STRFREE(skeleton->player.short_descr);
    I3DISPOSE(skeleton);
}

void I3_send_social(I3_CHANNEL *channel, CHAR_DATA *ch, const char *argument)
{
    CHAR_DATA *skeleton = NULL;
    char *ps;
    char socbuf_o[MAX_STRING_LENGTH], socbuf_t[MAX_STRING_LENGTH], msg_o[MAX_STRING_LENGTH], msg_t[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH], person[MAX_INPUT_LENGTH], mud[MAX_INPUT_LENGTH], user[MAX_INPUT_LENGTH],
        buf[MAX_STRING_LENGTH];
    unsigned int x;

    person[0] = '\0';
    mud[0] = '\0';

    /*
     * Name of social, remainder of argument is assumed to hold the target
     */
    argument = i3one_argument(argument, arg1);

    snprintf(user, MAX_INPUT_LENGTH, "%s@%s", CH_I3NAME(ch), this_i3mud->name);
    if (!strcasecmp(user, argument))
    {
        i3_printf(ch, "Cannot target yourself due to the nature of I3 socials.\r\n");
        return;
    }

    if (argument && argument[0] != '\0')
    {
        if (!(ps = (char *)strchr(argument, '@')))
        {
            i3_printf(ch, "You need to specify a person@mud for a target.\r\n");
            return;
        }
        else
        {
            for (x = 0; x < strlen(argument); x++)
            {
                person[x] = argument[x];
                if (person[x] == '@')
                    break;
            }
            person[x] = '\0';

            ps[0] = '\0';
            strlcpy(mud, ps + 1, MAX_INPUT_LENGTH);
        }
    }

    snprintf(socbuf_o, MAX_STRING_LENGTH, "%s", I3_find_social(ch, arg1, person, mud, FALSE));

    if (socbuf_o[0] != '\0')
        snprintf(socbuf_t, MAX_STRING_LENGTH, "%s", I3_find_social(ch, arg1, person, mud, TRUE));

    if ((socbuf_o[0] != '\0') && (socbuf_t[0] != '\0'))
    {
        if (argument && argument[0] != '\0')
        {
            int sex;

            snprintf(buf, MAX_STRING_LENGTH, "%s@%s", person, mud);
            sex = I3_get_ucache_gender(buf);
            if (sex == -1)
            {
                /*
                 * Greg said to "just punt and call them all males".
                 * * I decided to meet him halfway and at least request data before punting :)
                 */
                I3_send_chan_user_req(mud, person);
                sex = SEX_MALE;
            }
            else
                sex = i3todikugender(sex);

            skeleton = I3_make_skeleton(buf);
            CH_I3SEX(skeleton) = sex;
        }

        strlcpy(msg_o, (char *)i3act_string(socbuf_o, ch, skeleton), MAX_STRING_LENGTH);
        strlcpy(msg_t, (char *)i3act_string(socbuf_t, ch, skeleton), MAX_STRING_LENGTH);

        if (!skeleton)
            I3_send_channel_emote(channel, CH_I3NAME(ch), msg_o);
        else
        {
            strlcpy(buf, person, MAX_STRING_LENGTH);
            buf[0] = tolower(buf[0]);
            I3_send_channel_t(channel, CH_I3NAME(ch), mud, buf, msg_o, msg_t, person);
        }
        if (skeleton)
            I3_purge_skeleton(skeleton);
    }
}

I3_CMD(I3_taunt)
{
    char to[MAX_INPUT_LENGTH], *ps;
    char mud[MAX_INPUT_LENGTH];
    I3_MUD *pmud;
    char *taunt;

    if (!argument || argument[0] == '\0')
    {
        do_taunt_from_log();
    }
    else
    {
        argument = i3one_argument(argument, to);
        ps = strchr(to, '@');

        if (to[0] == '\0' || ps == NULL)
        {
            i3_printf(ch, "%%^YELLOW%%^You should specify a person and a mud.%%^RESET%%^\r\n"
                          "(use %%^WHITE%%^%%^BOLD%%^i3mudlist%%^YELLOW%%^ to get an overview of the muds "
                          "available)%%^RESET%%^\r\n");
            return;
        }

        ps[0] = '\0';
        ps++;
        strlcpy(mud, ps, MAX_INPUT_LENGTH);

        if (!(pmud = find_I3_mud_by_name(mud)))
        {
            i3_printf(ch, "%%^YELLOW%%^No such mud known.%%^RESET%%^\r\n"
                          "(use %%^WHITE%%^%%^BOLD%%^i3mudlist%%^YELLOW%%^ to get an overview of the muds "
                          "available)%%^RESET%%^\r\n");
            return;
        }

#if 0
        if (!strcasecmp(this_i3mud->name, pmud->name)) {
            i3_printf(ch, "Use your mud's own internal system for that.\r\n");
            return;
        }
#endif

        if (pmud->status >= 0)
        {
            i3_printf(ch, "%s is marked as down.\r\n", pmud->name);
            return;
        }

        if (pmud->tell == 0)
        {
            i3_printf(ch, "%s does not support the 'tell' command.\r\n", pmud->name);
            return;
        }

        // I3_send_tell(ch, to, pmud->name, argument);
        // i3_npc_speak("wiley", "Cron", taunt);

        taunt = i3_taunt_line();

        I3_escape(to);
        I3_write_header("tell", this_i3mud->name, "Taunter", pmud->name, to);
        I3_write_buffer("\"");
        I3_write_buffer("Taunter");
        I3_write_buffer("\",\"");
        send_to_i3(I3_escape(taunt));
        I3_write_buffer("\",})\r");
        I3_send_packet();
        i3_printf(ch, "%%^YELLOW%%^You TAUNT %%^CYAN%%^%%^BOLD%%^%s@%s%%^RESET%%^ with: %s\r\n", to, pmud->name, taunt);
    }
}

I3_CMD(I3_speaker_color)
{
    const char *found = NULL;
    char lowercase_name[MAX_INPUT_LENGTH];
    char speaker[MAX_INPUT_LENGTH];

    if (!argument || argument[0] == '\0')
    {
        i3_printf(ch, "Usage: i3speaker_color <speaker> <color>\r\n");
        return;
    }

    bzero(speaker, MAX_INPUT_LENGTH);
    argument = i3one_argument(argument, speaker);

    if (!*speaker)
    {
        i3_printf(ch, "Usage: i3speaker_color <speaker> <color>\r\n");
        return;
    }

    bzero(lowercase_name, MAX_INPUT_LENGTH);
    for (int i = 0; i < strlen(speaker) && i < MAX_INPUT_LENGTH; i++)
    {
        if (isalpha(speaker[i]))
            lowercase_name[i] = tolower(speaker[i]);
        else
            lowercase_name[i] = speaker[i];
    }

    found = (const char *)stringmap_find(speaker_db, lowercase_name);
    if (found)
    {
        if (!argument || !*argument)
        {
            i3_printf(ch, "%%^YELLOW%%^No color sequence specified.%%^RESET%%^\r\n");
        }
        else
        {
            stringmap_add(speaker_db, lowercase_name, (void *)argument, strlen(argument) + 1);
            I3_saveSpeakers();
            i3_printf(ch, "%%^GREEN%%^Color sequence for %s saved.%%^RESET%%^\r\n", speaker);
        }
    }
    else
    {
        i3_printf(ch, "%%^YELLOW%%^No such speaker known.%%^RESET%%^\r\n");
    }
}

I3_CMD(I3_reload_speakers)
{
    i3_printf(ch, "Reloading I3 Speaker and Color data...");

    I3_loadSpeakers();
    I3_loadPinkfishToANSI();
    I3_loadPinkfishToI3();
    I3_loadPinkfishToXterm256();
    I3_loadPinkfishToGreyscale();

    i3_printf(ch, "done.");
}

const char *i3_funcname(I3_FUN *func)
{
    if (func == I3_other)
        return "I3_other";
    if (func == I3_listen_channel)
        return "I3_listen_channel";
    if (func == I3_chanlist)
        return "I3_chanlist";
    if (func == I3_mudlist)
        return "I3_mudlist";
    if (func == I3_invis)
        return "I3_invis";
    if (func == I3_who)
        return "I3_who";
    if (func == I3_locate)
        return "I3_locate";
    if (func == I3_tell)
        return "I3_tell";
    if (func == I3_reply)
        return "I3_reply";
    if (func == I3_emote)
        return "I3_emote";
    if (func == I3_beep)
        return "I3_beep";
    if (func == I3_ignorecmd)
        return "I3_ignorecmd";
    if (func == I3_finger)
        return "I3_finger";
    if (func == I3_mudinfo)
        return "I3_mudinfo";
    if (func == I3_color)
        return "I3_color";
    if (func == I3_afk)
        return "I3_afk";
    if (func == I3_chan_who)
        return "I3_chan_who";
    if (func == I3_connect)
        return "I3_connect";
    if (func == I3_disconnect)
        return "I3_disconnect";
    if (func == I3_reload)
        return "I3_reload";
    if (func == I3_send_user_req)
        return "I3_send_user_req";
    if (func == I3_permstats)
        return "I3_permstats";
    if (func == I3_deny_channel)
        return "I3_deny_channel";
    if (func == I3_permset)
        return "I3_permset";
    if (func == I3_chanlayout)
        return "I3_chanlayout";
    if (func == I3_admin_channel)
        return "I3_admin_channel";
    if (func == I3_addchan)
        return "I3_addchan";
    if (func == I3_removechan)
        return "I3_removechan";
    if (func == I3_edit_channel)
        return "I3_edit_channel";
    if (func == I3_mudlisten)
        return "I3_mudlisten";
    if (func == I3_router)
        return "I3_router";
    if (func == I3_bancmd)
        return "I3_bancmd";
    if (func == I3_setconfig)
        return "I3_setconfig";
    if (func == I3_setup_channel)
        return "I3_setup_channel";
    if (func == I3_stats)
        return "I3_stats";
    if (func == I3_show_ucache_contents)
        return "I3_show_ucache_contents";
    if (func == I3_debug)
        return "I3_debug";
    if (func == i3_hedit)
        return "i3_hedit";
    if (func == i3_help)
        return "i3_help";
    if (func == i3_cedit)
        return "i3_cedit";
    if (func == I3_taunt)
        return "I3_taunt";
    if (func == I3_speaker_color)
        return "I3_speaker_color";
    if (func == I3_reload_speakers)
        return "I3_reload_speakers";

    return "";
}

I3_FUN *i3_function(const char *func)
{
    if (!strcasecmp(func, "I3_other"))
        return I3_other;
    if (!strcasecmp(func, "I3_listen_channel"))
        return I3_listen_channel;
    if (!strcasecmp(func, "I3_chanlist"))
        return I3_chanlist;
    if (!strcasecmp(func, "I3_mudlist"))
        return I3_mudlist;
    if (!strcasecmp(func, "I3_invis"))
        return I3_invis;
    if (!strcasecmp(func, "I3_who"))
        return I3_who;
    if (!strcasecmp(func, "I3_locate"))
        return I3_locate;
    if (!strcasecmp(func, "I3_tell"))
        return I3_tell;
    if (!strcasecmp(func, "I3_reply"))
        return I3_reply;
    if (!strcasecmp(func, "I3_emote"))
        return I3_emote;
    if (!strcasecmp(func, "I3_beep"))
        return I3_beep;
    if (!strcasecmp(func, "I3_ignorecmd"))
        return I3_ignorecmd;
    if (!strcasecmp(func, "I3_finger"))
        return I3_finger;
    if (!strcasecmp(func, "I3_mudinfo"))
        return I3_mudinfo;
    if (!strcasecmp(func, "I3_color"))
        return I3_color;
    if (!strcasecmp(func, "I3_afk"))
        return I3_afk;
    if (!strcasecmp(func, "I3_chan_who"))
        return I3_chan_who;
    if (!strcasecmp(func, "I3_connect"))
        return I3_connect;
    if (!strcasecmp(func, "I3_disconnect"))
        return I3_disconnect;
    if (!strcasecmp(func, "I3_reload"))
        return I3_reload;
    if (!strcasecmp(func, "I3_send_user_req"))
        return I3_send_user_req;
    if (!strcasecmp(func, "I3_permstats"))
        return I3_permstats;
    if (!strcasecmp(func, "I3_deny_channel"))
        return I3_deny_channel;
    if (!strcasecmp(func, "I3_permset"))
        return I3_permset;
    if (!strcasecmp(func, "I3_admin_channel"))
        return I3_admin_channel;
    if (!strcasecmp(func, "I3_bancmd"))
        return I3_bancmd;
    if (!strcasecmp(func, "I3_setconfig"))
        return I3_setconfig;
    if (!strcasecmp(func, "I3_setup_channel"))
        return I3_setup_channel;
    if (!strcasecmp(func, "I3_chanlayout"))
        return I3_chanlayout;
    if (!strcasecmp(func, "I3_addchan"))
        return I3_addchan;
    if (!strcasecmp(func, "I3_removechan"))
        return I3_removechan;
    if (!strcasecmp(func, "I3_edit_channel"))
        return I3_edit_channel;
    if (!strcasecmp(func, "I3_mudlisten"))
        return I3_mudlisten;
    if (!strcasecmp(func, "I3_router"))
        return I3_router;
    if (!strcasecmp(func, "I3_stats"))
        return I3_stats;
    if (!strcasecmp(func, "I3_show_ucache_contents"))
        return I3_show_ucache_contents;
    if (!strcasecmp(func, "I3_debug"))
        return I3_debug;
    if (!strcasecmp(func, "i3_help"))
        return i3_help;
    if (!strcasecmp(func, "i3_cedit"))
        return i3_cedit;
    if (!strcasecmp(func, "i3_hedit"))
        return i3_hedit;
    if (!strcasecmp(func, "I3_taunt"))
        return I3_taunt;
    if (!strcasecmp(func, "I3_speaker_color"))
        return I3_speaker_color;
    if (!strcasecmp(func, "I3_reload_speakers"))
        return I3_reload_speakers;

    return NULL;
}

/*
 * This is how channels are interpreted. If they are not commands
 * or socials, this function will go through the list of channels
 * and send it to it if the name matches the local channel name.
 */
bool i3_command_hook(CHAR_DATA *ch, const char *lcommand, const char *argument)
{
    I3_CMD_DATA *cmd;
    I3_ALIAS *alias;
    I3_CHANNEL *channel;
    int x = 0;
    char buffer[MAX_STRING_LENGTH];
    const char *s = NULL;
    char *b = buffer;
    int token_count = 15;
    const char *token[] = {
        "%^RESET%^%^RED%^",           "%^RESET%^%^RED%^%^BOLD%^",   "%^RESET%^%^ORANGE%^",
        "%^RESET%^%^YELLOW%^",        "%^RESET%^%^GREEN%^",         "%^RESET%^%^GREEN%^%^BOLD%^",
        "%^RESET%^%^WHITE%^",         "%^RESET%^%^WHITE%^%^BOLD%^", "%^RESET%^%^CYAN%^%^BOLD%^",
        "%^RESET%^%^CYAN%^",          "%^RESET%^%^BLUE%^%^BOLD%^",  "%^RESET%^%^BLUE%^",
        "%^RESET%^%^BLACK%^%^BOLD%^", "%^RESET%^%^MAGENTA%^",       "%^RESET%^%^MAGENTA%^%^BOLD%^",
    };
    int color = 0;
    int prev_color = 0;
    int color_dir = 1;
    const char *original_argument;
    int emote_hack = 0;
    int bg_token_count = 10;
    const char *bg_token[] = {
        "%^RESET%^%^BLACK%^%^B_RED%^",     "%^RESET%^%^WHITE%^%^B_RED%^",   "%^RESET%^%^BLACK%^%^B_YELLOW%^",
        "%^RESET%^%^BLACK%^%^B_GREEN%^",   "%^RESET%^%^WHITE%^%^B_GREEN%^", "%^RESET%^%^BLACK%^%^B_WHITE%^",
        "%^RESET%^%^BLACK%^%^B_CYAN%^",    "%^RESET%^%^WHITE%^%^B_BLUE%^",  "%^RESET%^%^BLACK%^%^B_MAGENTA%^",
        "%^RESET%^%^WHITE%^%^B_MAGENTA%^",
    };

    if (IS_NPC(ch))
        return FALSE;

    if (!ch->desc)
        return FALSE;

    if (!this_i3mud)
    {
        log_info("%s", "Ooops. I3 called with missing configuration!");
        return FALSE;
    }

    if (first_i3_command == NULL)
    {
        log_info("%s", "Dammit! No command data is loaded!");
        return FALSE;
    }

    if (I3PERM(ch) <= I3PERM_NONE)
    {
        log_error("Permission %d vs. %d", I3PERM(ch), I3PERM_NONE);
        return FALSE;
    }

    /*
     * Simple command interpreter menu. Nothing overly fancy etc, but it beats trying to tie directly into the mud's
     * own internal structures. Especially with the differences in codebases.
     */
    for (cmd = first_i3_command; cmd; cmd = cmd->next)
    {
        if (I3PERM(ch) < cmd->level)
            continue;

        for (alias = cmd->first_alias; alias; alias = alias->next)
        {
            if (!strcasecmp(lcommand, alias->name))
            {
                lcommand = cmd->name;
                break;
            }
        }

        if (!strcasecmp(lcommand, cmd->name))
        {
            if (cmd->connected == TRUE && !is_connecting())
            {
                i3_printf(ch, "The mud is not currently connected to I3.\r\n");
                return TRUE;
            }

            if (cmd->function == NULL)
            {
                i3_printf(ch, "That command has no code set. Inform the administration.\r\n");
                log_error("i3_command_hook: Command %s has no code set!", cmd->name);
                return TRUE;
            }

            (*cmd->function)(ch, argument);
            return TRUE;
        }
    }

    // Assumed to be going for a channel if it gets this far

    if (!(channel = find_I3_channel_by_localname(lcommand)))
        return FALSE;

    if (I3PERM(ch) < channel->i3perm)
        return FALSE;

    if (I3_hasname(I3DENY(ch), channel->local_name))
    {
        i3_printf(ch, "You have been denied the use of %s by the administration.\r\n", channel->local_name);
        return TRUE;
    }

    if (!argument || argument[0] == '\0')
    {
        return TRUE;
    }

    if (!is_connecting())
    {
        i3_printf(ch, "The mud is not currently connected to I3.\r\n");
        return TRUE;
    }

    if (!I3_hasname(I3LISTEN(ch), channel->local_name))
    {
        i3_printf(ch,
                  "%%^YELLOW%%^You were trying to send something to an I3 "
                  "channel but you're not listening to it.%%^RESET%%^\r\nPlease use the command "
                  "'%%^WHITE%%^%%^BOLD%%^i3listen %s%%^YELLOW%%^' to listen to it.%%^RESET%%^\r\n",
                  channel->local_name);
        return TRUE;
    }

    switch (argument[0])
    {
    case ',':
    case ':':
        // Strip the token
        argument++;
        // while (isspace(*argument)) argument++;
        I3_send_channel_emote(channel, CH_I3NAME(ch), argument);
        break;
    case '@':
        // Strip the token
        argument++;
        // while (isspace(*argument)) argument++;
        I3_send_social(channel, ch, argument);
        break;
    case '/':
        // Strip the / and then any spaces between it and the command word
        original_argument = argument;
        argument++;
        while (*argument && isspace(*argument))
            argument++;
        if (!strncasecmp(argument, "help", 4))
        {
            i3_printf(ch, "%%^GREEN%%^%%^BOLD%%^Channel subcommands available for %s are:%%^RESET%%^\r\n",
                      channel->local_name);
            i3_printf(ch, "%%^GREEN%%^%%^BOLD%%^    :emote      - Sends message as an \"emote\", which usually prints "
                          "differently.%%^RESET%%^\r\n");
            i3_printf(ch, "%%^GREEN%%^%%^BOLD%%^    @social     - Sends message as a \"social\", which we don't really "
                          "have.%%^RESET%%^\r\n");
            i3_printf(ch, "%%^GREEN%%^%%^BOLD%%^    /help       - This helpful message.%%^RESET%%^\r\n");
            i3_printf(ch, "%%^GREEN%%^%%^BOLD%%^    /history    - Shows the last %d messages.%%^RESET%%^\r\n",
                      MAX_I3HISTORY);
            if (I3PERM(ch) >= I3PERM_ADMIN)
            {
                i3_printf(ch, "%%^GREEN%%^%%^BOLD%%^    /log        - Toggle file logging on or off.%%^RESET%%^\r\n");
                i3_printf(ch,
                          "%%^GREEN%%^%%^BOLD%%^    /colorize   - Adds annoying colors to the message.%%^RESET%%^\r\n");
                i3_printf(ch, "%%^GREEN%%^%%^BOLD%%^    /rainbow    - Adds differently annoying colors to the "
                              "message.%%^RESET%%^\r\n");
                i3_printf(
                    ch, "%%^GREEN%%^%%^BOLD%%^    /hex        - Display message as hexadecimal codes.%%^RESET%%^\r\n");
                i3_printf(ch,
                          "%%^GREEN%%^%%^BOLD%%^    /bin        - Display message as binary digits.%%^RESET%%^\r\n");
                i3_printf(ch, "%%^GREEN%%^%%^BOLD%%^    /background - Makes every word have a different background "
                              "color.%%^RESET%%^\r\n");
            }
            return TRUE;
        }
        else if (I3PERM(ch) >= I3PERM_ADMIN && !strncasecmp(argument, "log", 3))
        {
            if (!I3IS_SET(channel->flags, I3CHAN_LOG))
            {
                I3SET_BIT(channel->flags, I3CHAN_LOG);
                i3_printf(ch,
                          "%%^RED%%^%%^BOLD%%^File logging enabled for %s, PLEASE don't forget to undo this when it "
                          "isn't needed!%%^RESET%%^\r\n",
                          channel->local_name);
            }
            else
            {
                I3REMOVE_BIT(channel->flags, I3CHAN_LOG);
                i3_printf(ch, "%%^GREEN%%^%%^BOLD%%^File logging disabled for %s.%%^RESET%%^\r\n", channel->local_name);
            }
            I3_write_channel_config();
            return TRUE;
        }
        else if (!strncasecmp(argument, "history", 7) || !strncasecmp(argument, "hist", 4))
        {
            i3_printf(ch, "%%^CYAN%%^The last %d %s messages:%%^RESET%%^\r\n", MAX_I3HISTORY, channel->local_name);
            for (x = 0; x < MAX_I3HISTORY; x++)
            {
                if (channel->history[x] != NULL)
                    i3_printf(ch, "%s%%^RESET%%^\r\n", channel->history[x]);
                else
                    break;
            }
            return TRUE;
        }
        else if (I3PERM(ch) >= I3PERM_ADMIN && !strncasecmp(argument, "colorize", 8))
        {
            argument += 8;
            // Skip the command verb + one space
            if (*argument && isspace(*argument))
                argument++;
            if (*argument == ':' || *argument == ',')
                emote_hack = 1;
            bzero(buffer, MAX_STRING_LENGTH);
            b = buffer;

            prev_color = color = random() % token_count;
            strncpy(b, token[color], strlen(token[color]));
            b += strlen(token[color]);
            for (s = argument; *s && strlen(b) < (MAX_STRING_LENGTH - 10);)
            {
                char *u = NULL;

                u = utf8_check((char *)s);
                if (u && *u)
                {
                    // Something unicode!
                    *b = *s;
                    b++;
                    s++;

                    strncpy(b, u, strlen(u));
                    b += strlen(u);
                    s += strlen(u);

                    if (isspace(*s) || ispunct(*s))
                    {
                        for (int pc = 0; pc < 10; pc++)
                        {
                            color = random() % bg_token_count;
                            if (color != prev_color)
                                break;
                        }
                        prev_color = color;
                        strncpy(b, token[color], strlen(token[color]));
                        b += strlen(token[color]);
                    }
                }
                else
                {
                    *b = *s;
                    b++;
                    s++;
                    if (isspace(*s) || ispunct(*s))
                    {
                        for (int pc = 0; pc < 10; pc++)
                        {
                            color = random() % bg_token_count;
                            if (color != prev_color)
                                break;
                        }
                        prev_color = color;
                        strncpy(b, token[color], strlen(token[color]));
                        b += strlen(token[color]);
                    }
                }
            }
            strcpy(b, "%^RESET%^");
            if (emote_hack)
            {
                I3_send_channel_emote(channel, CH_I3NAME(ch), buffer);
                emote_hack = 0;
            }
            else
            {
                I3_send_channel_message(channel, CH_I3NAME(ch), buffer);
            }
        }
        else if (I3PERM(ch) >= I3PERM_ADMIN && !strncasecmp(argument, "rainbow", 7))
        {
            argument += 7;
            // Skip the command verb + one space
            if (*argument && isspace(*argument))
                argument++;
            if (*argument == ':' || *argument == ',')
                emote_hack = 1;
            bzero(buffer, MAX_STRING_LENGTH);
            b = buffer;

            color = 0;
            color_dir = 1;
            strncpy(b, token[color], strlen(token[color]));
            b += strlen(token[color]);
            for (s = argument; *s && strlen(b) < (MAX_STRING_LENGTH - 10);)
            {
                char *u = NULL;

                u = utf8_check((char *)s);
                if (u && *u)
                {
                    // Something unicode!
                    *b = *s;
                    b++;
                    s++;

                    strncpy(b, u, strlen(u));
                    b += strlen(u);
                    s += strlen(u);

                    color += color_dir;
                    if (color == 0)
                    {
                        color_dir = 1;
                    }
                    else if (color == token_count)
                    {
                        color_dir = -1;
                        color = token_count - 1;
                    }
                    color = color % token_count;
                    strncpy(b, token[color], strlen(token[color]));
                    b += strlen(token[color]);
                }
                else
                {
                    // a normal character, not utf8
                    if (isspace(*s))
                    {
                        *b = *s;
                        b++;
                        s++;
                    }
                    else
                    {
                        *b = *s;
                        b++;
                        s++;
                        color += color_dir;
                        if (color == 0)
                        {
                            color_dir = 1;
                        }
                        else if (color == token_count)
                        {
                            color_dir = -1;
                            color = token_count - 1;
                        }
                        color = color % token_count;
                        strncpy(b, token[color], strlen(token[color]));
                        b += strlen(token[color]);
                    }
                }
            }
            strcpy(b, "%^RESET%^");
            if (emote_hack)
            {
                I3_send_channel_emote(channel, CH_I3NAME(ch), buffer);
                emote_hack = 0;
            }
            else
            {
                I3_send_channel_message(channel, CH_I3NAME(ch), buffer);
            }
        }
        else if (I3PERM(ch) >= I3PERM_ADMIN && !strncasecmp(argument, "background", 10))
        {
            argument += 10;
            // Skip the command verb + one space
            if (*argument && isspace(*argument))
                argument++;
            if (*argument == ':' || *argument == ',')
                emote_hack = 1;
            bzero(buffer, MAX_STRING_LENGTH);
            b = buffer;

            prev_color = color = random() % bg_token_count;
            strncpy(b, bg_token[color], strlen(bg_token[color]));
            b += strlen(bg_token[color]);
            for (s = argument; *s && strlen(b) < (MAX_STRING_LENGTH - 10);)
            {
                char *u = NULL;

                u = utf8_check((char *)s);
                if (u && *u)
                {
                    // Something unicode!
                    *b = *s;
                    b++;
                    s++;

                    strncpy(b, u, strlen(u));
                    b += strlen(u);
                    s += strlen(u);

                    if (isspace(*s))
                    {
                        for (int pc = 0; pc < 10; pc++)
                        {
                            color = random() % bg_token_count;
                            if (color != prev_color)
                                break;
                        }
                        prev_color = color;
                        strncpy(b, bg_token[color], strlen(bg_token[color]));
                        b += strlen(bg_token[color]);
                    }
                }
                else
                {
                    *b = *s;
                    b++;
                    s++;
                    if (isspace(*s))
                    {
                        for (int pc = 0; pc < 10; pc++)
                        {
                            color = random() % bg_token_count;
                            if (color != prev_color)
                                break;
                        }
                        prev_color = color;
                        strncpy(b, bg_token[color], strlen(bg_token[color]));
                        b += strlen(bg_token[color]);
                    }
                }
            }
            strcpy(b, "%^RESET%^");
            if (emote_hack)
            {
                I3_send_channel_emote(channel, CH_I3NAME(ch), buffer);
                emote_hack = 0;
            }
            else
            {
                I3_send_channel_message(channel, CH_I3NAME(ch), buffer);
            }
        }
        else if (I3PERM(ch) >= I3PERM_ADMIN && !strncasecmp(argument, "hex", 3))
        {
            argument += 3;
            // Skip the command verb + one space
            if (*argument && isspace(*argument))
                argument++;
            if (*argument == ':' || *argument == ',')
                emote_hack = 1;
            bzero(buffer, MAX_STRING_LENGTH);
            b = buffer;

            for (s = argument; *s && strlen(buffer) < (MAX_STRING_LENGTH - 10); s++)
            {
                int ss = (int)*s;
                scprintf(buffer, MAX_STRING_LENGTH - 10, "%0x", ss);
                if (*(s + 1))
                    strlcat(buffer, " ", MAX_STRING_LENGTH - 10);
            }
            strlcat(buffer, "%^RESET%^", MAX_STRING_LENGTH);
            if (emote_hack)
            {
                I3_send_channel_emote(channel, CH_I3NAME(ch), buffer);
                emote_hack = 0;
            }
            else
            {
                I3_send_channel_message(channel, CH_I3NAME(ch), buffer);
            }
        }
        else if (I3PERM(ch) >= I3PERM_ADMIN && !strncasecmp(argument, "bin", 3))
        {
            argument += 3;
            // Skip the command verb + one space
            if (*argument && isspace(*argument))
                argument++;
            if (*argument == ':' || *argument == ',')
                emote_hack = 1;
            bzero(buffer, MAX_STRING_LENGTH);
            b = buffer;

            for (s = argument; *s && strlen(buffer) < (MAX_STRING_LENGTH - 10); s++)
            {
                int ss = (int)*s;
                scprintf(buffer, MAX_STRING_LENGTH - 10, "%d%d%d%d%d%d%d%d", (ss & 0x80) ? 1 : 0, (ss & 0x40) ? 1 : 0,
                         (ss & 0x20) ? 1 : 0, (ss & 0x10) ? 1 : 0, (ss & 0x08) ? 1 : 0, (ss & 0x04) ? 1 : 0,
                         (ss & 0x02) ? 1 : 0, (ss & 0x01) ? 1 : 0);
                if (*(s + 1))
                    strlcat(buffer, " ", MAX_STRING_LENGTH - 10);
            }
            strlcat(buffer, "%^RESET%^", MAX_STRING_LENGTH);
            if (emote_hack)
            {
                I3_send_channel_emote(channel, CH_I3NAME(ch), buffer);
                emote_hack = 0;
            }
            else
            {
                I3_send_channel_message(channel, CH_I3NAME(ch), buffer);
            }
        }
        else
        {
            /* No match for known subcommands, so just send it */
            I3_send_channel_message(channel, CH_I3NAME(ch), original_argument);
        }
        break;
    default:
        I3_send_channel_message(channel, CH_I3NAME(ch), argument);
        break;
    }
    return TRUE;
}

void i3_do_ping(const char *fake_user, const char *chan_name, const char *mud_name)
{
    if (!is_connected())
        return;

    I3_write_header("chan-who-req", this_i3mud->name, fake_user, mud_name, NULL);
    I3_write_buffer("\"");
    I3_write_buffer(chan_name);
    I3_write_buffer("\",})\r");
    I3_send_packet();
    log_info("Sending PING from %s to %s@%s.", fake_user, chan_name, mud_name);
}

void i3_npc_speak(const char *chan_name, const char *actor, const char *message)
{
    I3_CHANNEL *channel;

    if (!is_connected())
    {
        log_info("Not connected!");
        return;
    }
    if (!(channel = find_I3_channel_by_localname(chan_name)))
    {
        log_info("Can't find local channel %s.", chan_name);
        return;
    }

    while (isspace(*message))
        message++;
    log_info("Sending [%s] from %s to %s.", message, actor, chan_name);
    I3_send_channel_message(channel, actor, message);
}

void i3_npc_chat(const char *chan_name, const char *actor, const char *message)
{
    I3_CHANNEL *channel;

    if (!is_connected())
    {
        log_info("Not connected!");
        return;
    }
    if (!(channel = find_I3_channel_by_localname(chan_name)))
    {
        log_info("Can't find local channel %s.", chan_name);
        return;
    }

    while (isspace(*message))
        message++;
    log_info("Sending [%s] from %s to %s.", message, actor, chan_name);
    I3_send_channel_emote(channel, actor, message);
}

void i3_npc_tell(const char *target_name, const char *target_mud, const char *actor, const char *message)
{
    if (!is_connected())
    {
        log_info("Not connected!");
        return;
    }

    log_info("Sending [%s] from %s to %s@%s.", message, actor, target_name, target_mud);

    I3_write_header("tell", this_i3mud->name, actor, target_mud, I3_escape(target_name));
    I3_write_buffer("\"");
    I3_write_buffer(actor);
    I3_write_buffer("\",\"");
    send_to_i3(I3_escape(message));
    I3_write_buffer("\",})\r");
    I3_send_packet();
}

char *I3_nameescape(const char *ps)
{
    static char xnew[MAX_STRING_LENGTH];
    char *pnew = xnew;
    char c;

    while (ps[0])
    {
        c = (char)tolower((int)*ps);
        ps++;
        if (c < 'a' || c > 'z')
            continue;
        pnew[0] = c;
        pnew++;
    }
    pnew[0] = '\0';
    return xnew;
}

char *I3_old_nameremap(const char *ps)
{
    static char xnew[MAX_STRING_LENGTH];

    if (!strcasecmp(ps, "quixadhal"))
    {
        /* strcpy(xnew, "Quixadhal, the Lost"); */
        /* strcpy(xnew, "きけさだる"); */
        /* strcpy(xnew, "Dread Lord Quixadhal"); */
        /* strcpy(xnew, "Off-Topic Quixadhal"); */
        /* strcpy(xnew, "Cromulent Quixadhal"); */
    }
    else
    {
        strcpy(xnew, ps);
    }
    return xnew;
}

char *explode_pinkfish_to(const char *src, int target)
{
    static char result[MAX_STRING_LENGTH];
    char **parts = NULL;
    int part_count = 0;
    stringMap *pinkfish_target_db = NULL;

    bzero(result, MAX_STRING_LENGTH);

    if (!src || !*src)
        return result;

    // Pinkfish codes are unfortunate.
    // They are implemented with a delimiter token that makes sense
    // for the LPMUD environment where they originated, but makes
    // life difficult for us.
    //
    // In an LPMUD, you'd write lpc code to do this:
    //
    // parts = explode("%^", src);
    // for(i = 0; i < sizeof(parts); i++) {
    //     for(j = 0; j < sizeof(pinkfish_keys); j++) {
    //         if(!strcmp(parts[i], pinkfish_keys[j])) {
    //             parts[i] = pinkfish_values[j];
    //         }
    //     }
    // }
    //
    // result = implode("", parts);

    switch (target)
    {
    default:
    case I3_PINKFISH_NULL:
        pinkfish_target_db = pinkfish_to_null_db;
        break;
    case I3_PINKFISH_ANSI:
        pinkfish_target_db = pinkfish_to_ansi_db;
        break;
    case I3_PINKFISH_I3:
        pinkfish_target_db = pinkfish_to_i3_db;
        break;
    case I3_PINKFISH_XTERM:
        pinkfish_target_db = pinkfish_to_xterm256_db;
        break;
    case I3_PINKFISH_GREY:
        pinkfish_target_db = pinkfish_to_greyscale_db;
        break;
    }

    // This will break the string up into an array of parts,
    // which were between the delimiter symbol "%^".
    // If there was no delimiter, this should just be one part.
    // Note that if the delimiter was leading or trailing, there
    // should be an empty string as a part.
    part_count = explode(src, "%^", &parts);

    for (int i = 0; i < part_count; i++)
    {
        if (parts[i])
        {
            const char *match = (const char *)stringmap_find(pinkfish_target_db, parts[i]);
            if (match)
            {
                // This is a Pinkfish code we recognize!
                // log_info("pinkfish_to: found \"%s\", remapped to \"ESC%s\" by MODE %d", parts[i], (const unsigned
                // char *)(match +1), target);
                free(parts[i]);
                if (target == I3_PINKFISH_NULL)
                {
                    parts[i] = strdup("");
                }
                else
                {
                    parts[i] = strdup(match);
                }
            }
            else
            {
                // log_info("NO MATCH: \"%s\"", parts[i]);
            }
        }
    }

    // Now that we've replaced any that we recognized, implode!
    for (int i = 0; i < part_count; i++)
    {
        strlcat(result, parts[i], MAX_STRING_LENGTH);
    }

    // And finally, we free the mallocs...
    for (int i = 0; i < part_count; i++)
    {
        if (parts[i])
        {
            free(parts[i]);
        }
    }
    if (parts)
    {
        free(parts);
    }

    return result;
}

char *pinkfish_to(const char *src, int target)
{
    static char result[MAX_STRING_LENGTH];
    stringMap *pinkfish_target_db = NULL;
    char *sp = NULL;
    char *tok = NULL;
    char word[MAX_STRING_LENGTH];
    const char *match = NULL;

    bzero(result, MAX_STRING_LENGTH);

    if (!src || !*src)
        return result;

    switch (target)
    {
    default:
    case I3_PINKFISH_NULL:
        pinkfish_target_db = pinkfish_to_null_db;
        break;
    case I3_PINKFISH_ANSI:
        pinkfish_target_db = pinkfish_to_ansi_db;
        break;
    case I3_PINKFISH_I3:
        pinkfish_target_db = pinkfish_to_i3_db;
        break;
    case I3_PINKFISH_XTERM:
        pinkfish_target_db = pinkfish_to_xterm256_db;
        break;
    case I3_PINKFISH_GREY:
        pinkfish_target_db = pinkfish_to_greyscale_db;
        break;
    }

    // Walk the string, finding our delimiters.
    sp = (char *)src;
    if (!(tok = strstr(sp, "%^")))
    {
        // No delimiters at all, catch and release.
        strlcpy(result, src, MAX_STRING_LENGTH);
    }
    else
    {
        // We have at least one!
        if (tok == sp)
        {
            // We are sitting on it
            sp += 2;
        }
        else
        {
            // There is stuff before it to process
            strlcpy(word, sp, tok - sp + 1);
            sp = (tok + 2);
            strlcat(result, word, MAX_STRING_LENGTH);
        }

        while ((tok = strstr(sp, "%^")) != NULL)
        {
            // From now on, each token we find will mark a potential
            // replacment.
            strlcpy(word, sp, tok - sp + 1);
            sp = (tok + 2);
            match = (const char *)stringmap_find(pinkfish_target_db, word);
            if (match)
            {
                // Hey, a Pinkfish code!
                strlcat(result, match, MAX_STRING_LENGTH);
            }
            else
            {
                // Nope, just a word
                strlcat(result, word, MAX_STRING_LENGTH);
            }
        }

        // Now that we've exhausted all of the tokens, if there's
        // anything left, we just copy it verbatum.
        if (*sp)
        {
            strlcat(result, sp, MAX_STRING_LENGTH);
        }
    }

    return result;
}

#if 0
char *pinkfish_to_ansi(const char *src)
{
    static char result[MAX_STRING_LENGTH];
    char **parts        = NULL;
    int part_count      = 0;

    bzero(result, MAX_STRING_LENGTH);

    if(!src || !*src)
        return result;

    // Pinkfish codes are unfortunate.
    // They are implemented with a delimiter token that makes sense
    // for the LPMUD environment where they originated, but makes
    // life difficult for us.
    //
    // In an LPMUD, you'd write lpc code to do this:
    //
    // parts = explode("%^", src);
    // for(i = 0; i < sizeof(parts); i++) {
    //     for(j = 0; j < sizeof(pinkfish_keys); j++) {
    //         if(!strcmp(parts[i], pinkfish_keys[j])) {
    //             parts[i] = pinkfish_values[j];
    //         }
    //     }
    // }
    //
    // result = implode("", parts);

    part_count = explode(src, "%^", &parts);
    if(part_count < 2) {
        // We only have the whole string (or an error).
        strlcpy(result, src, MAX_STRING_LENGTH);

        for(int i = 0; i < part_count; i++) {
            if(parts[i]) {
                free(parts[i]);
            }
        }
        if(parts) {
            free(parts);
        }
    } else {
        // We got some stuff back to play with!
        // Assuming we already setup a pinkfish_to_ansi_db stringMap...
        for(int i = 0; i < part_count; i++) {
            if(parts[i]) {
                const char *match = stringmap_find(pinkfish_to_ansi_db, parts[i]);
                if(match) {
                    // This is a Pinkfish code we recognize!
                    free(parts[i]);
                    parts[i] = strdup(match);
                }
            }
        }

        // Now we've replaced all the matches, the way LPC would do it.
        // We can now "implode" the results.
        for(int i = 0; i < part_count; i++) {
            strlcat(result, parts[i], MAX_STRING_LENGTH);
        }

        // And finally, we free the mallocs...
        for(int i = 0; i < part_count; i++) {
            if(parts[i]) {
                free(parts[i]);
            }
        }
        if(parts) {
            free(parts);
        }
    }

    return result;
}
#endif

void i3_daily_summary()
{
    struct tm *ytm_info = NULL;
    time_t ytc = (time_t)0;
    struct stat yst;
    char output[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    int messages = 0;
    int speakers = 0;
    char logpage_url[MAX_INPUT_LENGTH] = "http://wileymud.themud.org/log/";
    char yesterday[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char yesterfile[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char yesternuke[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char yestertouch[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char yesterurl[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char *target_channel_list[] = {
        (char *)"intergossip",
        (char *)"dchat",
        (char *)"intercre"
    };
    int target_channel_count = 3;
    char *target_channel = target_channel_list[0];

    const char *sql = "SELECT ( "
                "SELECT count(*) FROM ( "
                "SELECT DISTINCT username "
                "FROM i3log "
                "WHERE date(local) = date(now()) - '1 day'::interval AND NOT is_bot "
                ") s "
                ") AS speakers, ( "
                "SELECT count(*) "
                "FROM i3log "
                "WHERE date(local) = date(now()) - '1 day'::interval AND NOT is_bot "
                ") AS messages;";
    PGresult *res = NULL;
    ExecStatusType st = (ExecStatusType) 0;
    int rows = 0;
    int columns = 0;

    ytc = time(0) - 86400;
    ytm_info = localtime(&ytc);
    // Rotate the channel to "spam" every day.
    // We'll use the day of the year, so the wraparound is less noticeable.
    target_channel = target_channel_list[ytm_info->tm_yday % target_channel_count];

    snprintf(yesterday, MAX_INPUT_LENGTH, "%-4.4d-%-2.2d-%-2.2d", ytm_info->tm_year + 1900, ytm_info->tm_mon + 1,
             ytm_info->tm_mday);
    snprintf(yesterfile, MAX_INPUT_LENGTH, "%s/%s.i3_done", I3_DIR, yesterday);
    snprintf(yesternuke, MAX_INPUT_LENGTH, "/usr/bin/rm %s/*.i3_done", I3_DIR);
    snprintf(yestertouch, MAX_INPUT_LENGTH, "/usr/bin/touch %s", yesterfile);
    snprintf(yesterurl, MAX_INPUT_LENGTH, "%s?noscroll&date=%s", logpage_url, yesterday);

    if (stat(yesterfile, &yst) != -1)
    {
        // We already did this today, for yesterday.
        //log_info("Summary has already been done: %s exists.", yesterfile);
        return;
    }
    else
    {
        // We haven't done it yet!
        log_info("Summary needs to be done: %s does not exist.", yesterfile);
        system(yesternuke);
        system(yestertouch);

        // SQL Stuff...
        sql_connect(&db_i3log);
        res = PQexec(db_i3log.dbc, sql);
        st = PQresultStatus(res);
        if (st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE)
        {
            log_fatal("Cannot get message count from i3log table: %s", PQerrorMessage(db_wileymud.dbc));
            PQclear(res);
            proper_exit(MUD_HALT);
        }
        rows = PQntuples(res);
        columns = PQnfields(res);
        if (rows > 0 && columns > 1)
        {
            speakers = (int)atoi(PQgetvalue(res, 0, 0));
            messages = (int)atoi(PQgetvalue(res, 0, 1));
        }
        else
        {
            log_fatal("Invalid result set from i3log!");
            PQclear(res);
            proper_exit(MUD_HALT);
        }
        PQclear(res);

        snprintf(output, MAX_STRING_LENGTH, "I3 activity seen on %s: %d messages from %d speakers.  Logs and MUD-list at %s",
                 yesterday, messages, speakers, yesterurl);
        // The last parameter is the URL to link.  logpage_url is "now" and never changes,
        // yesterurl is the date matching the activity, and will change each day.
        i3_npc_speak(target_channel, "Cron", output);
        log_info("Summary done.");
    }
}

void piss_off_shentino()
{
    const char *sql = "SELECT message FROM i3log WHERE username = 'shentino' "
        "OFFSET (SELECT floor(random() * count(*)) FROM i3log WHERE username = 'shentino') "
        "LIMIT 1;";
    PGresult *res = NULL;
    ExecStatusType st = (ExecStatusType) 0;
    int rows = 0;
    int columns = 0;
    char *messageQuote = NULL;

    /*
    const char *messages[] = {
        "This is not a denial of service attack against the router, not by a long shot, even on a basic level because I'm getting useful information back.",
        "But i3 is not a private network and I've already BEEN invited to use its api in a conforming manner.",
        "It is very worth the negligible burden it places on the router",
        "I'm already hosting it on a cloud VM",
        "You should be thanking me",
        "You're just looking to pin some excuse to say that what I'm doing is bad",
        "You're starting with the assumption that I need a good reason to do this.",
        "Perhaps, but still, no demonstrable harm.  The marginal burden of my pings is negligible.",
        "There is no material burden or harm from what I'm doing",
        "Cratylus expressly opened up the i3 router to being connected to by muds, so my permission is implicit, PROVIDED I do not abuse it or violate the presented API.",
        "im not going to forbid you from doing it shentino, but i think one second interval is unnecessarily tight for the floppy network this is",
        "Well that's actually a good question.  The purpose kinda...evolved",
        NULL
    };
    const int messageCount = 12;
    */

    const char *hiragana[] = {
        "あ", "か", "さ", "た", "な", "は", "ま", "や", "ら", "わ",
        "い", "き", "し", "ち", "に", "ひ", "み",       "り",
        "う", "く", "す", "つ", "ぬ", "ふ", "む", "ゆ", "る",
        "え", "け", "せ", "て", "ね", "へ", "め",       "れ",
        "お", "こ", "そ", "と", "の", "ほ", "も", "よ", "ろ", "を",
        "ん",

              "が", "ざ", "だ",       "ば", "ぱ",
              "ぎ", "じ", "ぢ",       "び", "ぴ",
              "ぐ", "ず", "づ",       "ぶ", "ぷ",
              "げ", "ぜ", "で",       "べ", "ぺ",
              "ご", "ぞ", "ど",       "ぼ", "ぽ",

              "きゃ", "しゃ", "ちゃ", "にゃ", "ひゃ", "みゃ",         "りゃ",
              "きゅ", "しゅ", "ちゅ", "にゅ", "ひゅ", "みゅ",         "りゅ",
              "きょ", "しょ", "ちょ", "にょ", "ひょ", "みょ",         "りょ",

              "ぎゃ", "じゃ",                 "びゃ", "ぴゃ",
              "ぎゅ", "じゅ",                 "びゅ", "ぴゅ",
              "ぎょ", "じょ",                 "びょ", "ぴょ",
        NULL
    };

    const char *katakana[] = {
        "ア", "カ", "サ", "タ", "ナ", "ハ", "マ", "ヤ", "ラ", "ワ",
        "イ", "キ", "シ", "チ", "ニ", "ヒ", "ミ",       "リ",
        "ウ", "ク", "ス", "ツ", "ヌ", "フ", "ム", "ユ", "ル",
        "エ", "ケ", "セ", "テ", "ネ", "ヘ", "メ",       "レ",
        "オ", "コ", "ソ", "ト", "ノ", "ホ", "モ", "ヨ", "ロ", "ヲ",
        "ン",

              "ガ", "ザ", "ダ",       "バ", "パ",
              "ギ", "ジ", "ヂ",       "ビ", "ピ",
              "グ", "ズ", "ヅ",       "ブ", "プ",
              "ゲ", "ゼ", "デ",       "ベ", "ペ",
              "ゴ", "ゾ", "ド",       "ボ", "ポ",

              "キャ", "シャ", "チャ", "ニャ", "ヒャ", "ミャ",         "リャ",
              "キュ", "シュ", "チュ", "ニュ", "ヒュ", "ミュ",         "リュ",
              "キョ", "ショ", "チョ", "ニョ", "ヒョ", "ミョ",         "リョ",

              "ギャ", "ジャ",                 "ビャ", "ピャ",
              "ギュ", "ジュ",                 "ビュ", "ピュ",
              "ギョ", "ジョ",                 "ビョ", "ピョ",
        NULL
    };

    const char *romanji[] = {
        "a",   "ka",  "sa",  "ta",  "na",  "ha",  "ma",  "ya",  "ra",  "wa",
        "i",   "ki",  "shi", "chi", "ni",  "hi",  "mi",         "ri",
        "u",   "ku",  "su",  "tsu", "nu",  "fu",  "mu",  "yu",  "ru",
        "e",   "ke",  "se",  "te",  "ne",  "he",  "me",         "re",
        "o",   "ko",  "so",  "to",  "no",  "ho",  "mo",  "yo",  "ro",  "wo",
        "n",

               "ga",  "za",  "da",         "ba",  "pa",
               "gi",  "ji",  "ji",         "bi",  "pi",
               "gu",  "zu",  "zu",         "bu",  "pu",
               "ge",  "ze",  "de",         "be",  "pe",
               "go",  "zo",  "do",         "bo",  "po",

               "kya", "sha", "cha", "nya", "hya", "mya",        "rya",
               "kyu", "shu", "chu", "nyu", "hyu", "myu",        "ryu",
               "kyo", "sho", "cho", "nyo", "hyo", "myo",        "ryo",

               "gya", "ja",                "bya", "pya",
               "gyu", "ju",                "byu", "pyu",
               "gyo", "jo",                "byo", "pyo",
        NULL
    };

    //const char *openQuote = "「";
    //const char *closeQuote = "」";
    const int symbolCount = 104;
    char hiraganaBuffer[MAX_INPUT_LENGTH];
    char katakanaBuffer[MAX_INPUT_LENGTH];
    char romanjiBuffer[MAX_INPUT_LENGTH];
    int nameLength = 0;
    int nameLength2 = 0;

    sql_connect(&db_i3log);
    res = PQexec(db_i3log.dbc, sql);
    st = PQresultStatus(res);
    if (st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE)
    {
        log_fatal("Cannot get shentino quote from i3log table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    rows = PQntuples(res);
    columns = PQnfields(res);
    if (rows > 0 && columns > 0)
    {
        messageQuote = PQgetvalue(res, 0, 0);
    }
    else
    {
        log_fatal("Invalid result set from i3log!");
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);

    bzero(hiraganaBuffer, MAX_INPUT_LENGTH);
    bzero(katakanaBuffer, MAX_INPUT_LENGTH);
    bzero(romanjiBuffer, MAX_INPUT_LENGTH);

    srandom(time(0) * getpid());

    nameLength = (random() % 4) + 2;
    nameLength2 = (random() % 3) + 1;
    if((random() % 100) < 50)
    {
        nameLength2 = 0;
    }

    for(int i = 0; i < nameLength; i++)
    {
        int r = random() % symbolCount;

        strcat(hiraganaBuffer, hiragana[r]);
        strcat(katakanaBuffer, katakana[r]);
        strcat(romanjiBuffer, romanji[r]);
    }
    if(nameLength2 > 1) {
        strcat(romanjiBuffer, " ");

        int useWhitespace = random() % 2;
        // It's not unusual to not use whitespace in Japanese,
        // but instead just flip character sets for clarity.

        if(useWhitespace)
        {
            strcat(hiraganaBuffer, " ");
            strcat(katakanaBuffer, " ");
        }

        for(int i = 0; i < nameLength2; i++)
        {
            int r = random() % symbolCount;

            strcat(romanjiBuffer, romanji[r]);
            if(useWhitespace)
            {
                strcat(hiraganaBuffer, hiragana[r]);
                strcat(katakanaBuffer, katakana[r]);
            }
            else
            {
                strcat(hiraganaBuffer, katakana[r]);
                strcat(katakanaBuffer, hiragana[r]);
            }
        }
    }

    //printf("Foreign Name is %s%s%s (%s)\n", openQuote, katakanaBuffer, closeQuote, romanjiBuffer);
    //printf("Message is \"%s\"\n", messages[random() % messageCount]);
    //printf("%s says, \"%s\"\n", katakanaBuffer, messages[random() % messageCount]);
    if((random() % 100) < 50)
    {
        //i3_npc_tell(katakanaBuffer, "Ulario", "GrumpyRouter", messages[random() % messageCount]);
        i3_npc_tell(katakanaBuffer, "Ulario", "GrumpyRouter", messageQuote);
    }
    else
    {
        //i3_npc_tell(romanjiBuffer, "Ulario", "GrumpyRouter", messages[random() % messageCount]);
        i3_npc_tell(romanjiBuffer, "Ulario", "GrumpyRouter", messageQuote);
    }
}

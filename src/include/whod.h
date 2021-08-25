#ifndef _WHOD_H
#define _WHOD_H

#define MUDNAME "WileyMUD"
#define OLDRFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"
#define RFC1123FMT "%a, %d %b %Y %H:%M:%S %Z"

/*
 * *** The following statement indicates the WHOD default mode
 * Modes can be:
 * SHOW_NAME
 * SHOW_TITLE
 * SHOW_SITE
 * SHOW_ON
 * SHOW_OFF
 * SHOW_LEVEL
 * SHOW_IDLE
 * SHOW_ROOM
 */

#define DEFAULT_MODE (SHOW_ON | SHOW_IDLE | SHOW_NAME | SHOW_TITLE | SHOW_ROOM_INGAME)

#define INVIS_LEVEL(ch) ((ch)->invis_level)

#define WIZ_MIN_LEVEL 50
#define WIZ_MAX_LEVEL 60

#define WHOD_DELAY_TIME 1
#define DISPLAY_LINKDEAD 1

#define WHOD_OPENING 1
#define WHOD_OPEN 2
#define WHOD_DELAY 3
#define WHOD_END 4
#define WHOD_CLOSED 5
#define WHOD_CLOSING 6

#define SHOW_ON 0x00000001
#define SHOW_OFF 0x00000002
#define SHOW_IDLE 0x00000004
#define SHOW_LEVEL 0x00000008
#define SHOW_NAME 0x00000010
#define SHOW_TITLE 0x00000020
#define SHOW_ROOM 0x00000040
#define SHOW_SITE 0x00000080
#define SHOW_ROOM_INGAME 0x00000100

#define WRITE(d, msg)                                                                                                  \
    if ((write((d), (msg), strlen(msg))) < 0)                                                                          \
    {                                                                                                                  \
        perror("whod.c - write");                                                                                      \
    }

#define WILEY_ADDRESS "wileymud.themud.org"
#define WILEY_PORT 3000

#define JSON_MUDLIST_PAGE                                                                                              \
    "../public_html/"                                                                                                  \
    "mudlist.json"
#define NEW_JSON_MUDLIST_PAGE                                                                                          \
    "../public_html/log/"                                                                                              \
    "mudlist.json"

struct whod_data
{
    time_t updated;
    int enabled;
    int port;
    char set_by[MAX_INPUT_LENGTH];
    int show_idle;
    int show_level;
    int show_name;
    int show_title;
    int show_room;
    int show_site;
    int show_room_ingame;
    int show_linkdead;
    int state;              // Temporary, for tracking request state
    int socket;             // Temporary, the socket descriptor
    time_t disconnect_time; // Temporary, tracking disconnection timeouts
};

#ifndef _WHOD_C
extern struct whod_data whod;
#endif

void setup_whod_table(void);
void load_whod(void);
int toggle_whod_flag(struct char_data *ch, const char *param);
int whod_flag_value(const char *param);
int set_whod_port(struct char_data *ch, int number);

void init_whod(void);
void close_whod(void);
void whod_loop(void);
char *idle_time_string(struct char_data *ch);
char *player_level_string(struct char_data *ch);
char *player_full_name(struct char_data *ch, int show_title);
char *player_room_name(struct char_data *ch);
char *player_site_name(struct char_data *ch);
char *whod_text(struct char_data *viewer);
void show_whod_info(struct char_data *ch);
void do_whod(struct char_data *ch, const char *arg, int cmd);

void generate_json_mudlist(const char *filename);
void do_who(struct char_data *ch, const char *argument, int cmd);
void do_users(struct char_data *ch, const char *argument, int cmd);
void do_players(struct char_data *ch, const char *argument, int cmd);

#endif

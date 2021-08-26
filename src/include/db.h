/*
 * file: db.h , Database module.                          Part of DIKUMUD
 * Usage: Loading/Saving chars booting world.
 */

#ifndef _DB_H
#define _DB_H

/* data files used by the game system */

#define DFLT_DIR "../lib"              /* default data directory */
#define WORLD_FILE "wld/tinyworld.wld" /* room definitions */
#define MOB_FILE "wld/tinyworld.mob"   /* monster prototypes */
#define OBJ_FILE "wld/tinyworld.obj"   /* object prototypes */
#define ZONE_FILE "wld/tinyworld.zon"  /* zone defs & command tables */
#define SHOP_FILE "wld/tinyworld.shp"
#define CREDITS_FILE "etc/credits" /* for the 'credits' command */
#define NEWS_FILE "etc/news"       /* for the 'news' command */
#define MOTD_FILE "etc/motd"       /* messages of today */
#define HELP_PAGE_FILE "man/help"
#define WMOTD_FILE "etc/wmotd" /* messages of today */
#define GREETINGS_FILE "etc/greetings"
#define LOGIN_MENU_FILE "etc/login.menu"
#define SEX_MENU_FILE "etc/sex.menu"
#define RACE_MENU_FILE "etc/race.menu"
#define CLASS_MENU_FILE "etc/class.menu"
#define RACE_HELP_FILE "etc/race.help"
#define CLASS_HELP_FILE "etc/class.help"
#define STORY_FILE "etc/story"
#define SUICIDE_WARN_FILE "etc/suicide.warn"
#define SUICIDE_DONE_FILE "etc/suicide.done"
#define PLAYER_FILE "adm/players"  /* the player database */
#define MESS_FILE "etc/messages"   /* damage message */
#define SOCMESS_FILE "etc/actions" /* messgs for social acts */
#define INFO_FILE "etc/info"       /* for INFO */
#define WIZLIST_FILE "etc/wizlist" /* for WIZLIST */
#define POSEMESS_FILE "etc/poses"  /* for 'pose'-command */
#define BOARD_FILE_PATH "boards"
#define OBJ_SAVE_FILE "ply/pcobjs.obj"
#define MKZONE_PATH "wld/mkzone"
#define IDEA_FILE "log/ideas" /* for the 'idea'-command */
#define TYPO_FILE "log/typos" /* 'typo' */
#define BUG_FILE "log/bugs"   /* 'bug' */
#define PASSWD_FILE "adm/passwd"
#define PASSWD_NEW "adm/passwd.new"
#define PASSWD_OFF "adm/passwd.off"

#define DEFAULT_HOME 3001 /* when a player is created */

#define KEY(literal, field, value)                                                                                     \
    if (!str_cmp((word), (literal)))                                                                                   \
    {                                                                                                                  \
        field = (value);                                                                                               \
        fMatch = TRUE;                                                                                                 \
        break;                                                                                                         \
    }

#define CKEY(literal, field, value)                                                                                    \
    if (!str_cmp((word), (literal)))                                                                                   \
    {                                                                                                                  \
        char *v;                                                                                                       \
        v = (value);                                                                                                   \
        strcpy((field), v ? v : "");                                                                                   \
        fMatch = TRUE;                                                                                                 \
        break;                                                                                                         \
    }

#define SKEY(string, field)                                                                                            \
    {                                                                                                                  \
        if (!str_cmp((word), (string)))                                                                                \
        {                                                                                                              \
            if (field)                                                                                                 \
                free(field);                                                                                           \
            field = fread_string(fp);                                                                                  \
            fMatch = TRUE;                                                                                             \
            break;                                                                                                     \
        }                                                                                                              \
    }

#define REAL 0
#define VIRTUAL 1

#define LOG_ZONE_ERROR(ch, type, zone, cmd)                                                                            \
    {                                                                                                                  \
        log_error("error in zone %s cmd %d (%c) resolving %s number", zone_table[zone].name, cmd, ch, type);           \
    }

#define ZO_DEAD 999
#define ZCMD zone_table[zone].cmd[cmd_no]
#define ZNAME zone_table[zone].name

/* structure for the reset commands */
struct reset_com
{
    char command; /* current command */
    char if_flag; /* if TRUE: exe only if preceding exe'd */
    int arg1;     /* */
    int arg2;     /* Arguments to the command */
    int arg3;     /* */

    /*
     * *  Commands:              *
     * *  'M': Read a mobile     *
     * *  'O': Read an object    *
     * *  'L': Has a leader      *
     * *  'G': Give obj to mob   *
     * *  'P': Put obj in obj    *
     * *  'G': Obj to char       *
     * *  'E': Obj to char equip *
     * *  'D': Set state of door *
     */
};

/* zone definition structure. for the 'zone-table'   */
struct zone_data
{
    char *name;   /* name of this zone */
    int lifespan; /* how long between resets (minutes) */
    int age;      /* current age of this zone (minutes) */
    int top;      /* upper limit for rooms in this zone */

    int reset_mode;        /* conditions for reset (see below) */
    struct reset_com *cmd; /* command table for reset */

    /*
     * *  Reset mode:                              *
     * *  0: Don't reset, and don't update age.    *
     * *  1: Reset if no PC's are located in zone. *
     * *  2: Just reset.                           *
     */
};

/* element in monster and object index-tables   */
struct index_data
{
    int vnum; /* virtual number of this mob/obj */
    long pos;    /* file position of this field */
    int number;  /* number of existing units of this mob/obj */
    ifuncp func;
    char *name;
};

struct obj_index_data
{
    int vnum; /* virtual number of this mob/obj */
    long pos;    /* file position of this field */
    int number;  /* number of existing units of this mob/obj */
    ifuncp func;
    char *name;
    struct obj_data *data_template; /* for preloading of objects */
};

/* for queueing zones for update   */
struct reset_q_element
{
    int zone_to_reset; /* ref to zone_data */
    struct reset_q_element *next;
};

/* structure for the update queue     */
struct reset_q_type
{
    struct reset_q_element *head;
    struct reset_q_element *tail;
}; /* reset_q; */

struct player_index_element
{
    char *name;
    int nr;
};

struct player_list_entry
{
    char name[80];
    char last_site[80];
    char last_login[80];
    int max_level;
};

#ifndef _DB_C
extern int top_of_world;
extern struct hash_header room_db;
extern struct reset_q_type reset_q;

extern struct obj_data *object_list;
extern struct char_data *character_list;

extern struct zone_data *zone_table;
extern int top_of_zone_table;
extern struct message_list fight_messages[MAX_MESSAGES];
extern struct player_index_element *player_table;
extern int top_of_p_table;
extern int top_of_p_file;

extern char credits[MAX_STRING_LENGTH];
extern char news[MAX_STRING_LENGTH];
extern char motd[MAX_STRING_LENGTH];
extern char help[MAX_STRING_LENGTH];
extern char wizhelp[MAX_STRING_LENGTH];
extern char info[MAX_STRING_LENGTH];
extern char wizlist[MAX_STRING_LENGTH];
extern char wmotd[MAX_STRING_LENGTH];
extern char greetings[MAX_STRING_LENGTH];
extern char login_menu[MAX_STRING_LENGTH];
extern char sex_menu[MAX_STRING_LENGTH];
extern char race_menu[MAX_STRING_LENGTH];
extern char class_menu[MAX_STRING_LENGTH];
extern char race_help[MAX_STRING_LENGTH];
extern char class_help[MAX_STRING_LENGTH];
extern char the_story[MAX_STRING_LENGTH];
extern char suicide_warn[MAX_STRING_LENGTH];
extern char suicide_done[MAX_STRING_LENGTH];

extern FILE *mob_f;
extern FILE *obj_f;

extern struct index_data *mob_index;
extern struct index_data *obj_index;

extern int top_of_mobt;
extern int top_of_objt;

extern struct time_info_data time_info;
extern struct weather_data weather_info;

extern char TMPbuff[1620];
extern int TMPbuff_ptr;
extern int ROOMcount;
extern int GLINEcount;
extern int LASTroomnumber;

extern char **list_of_players;
extern int number_of_players;
extern int actual_players;

#endif

void load_db(void);
void unload_db(void);

#if 0
void                                    build_player_index(void);

#endif
struct index_data *generate_indices(FILE *fl, int *top);
void cleanout_room(struct room_data *rp);
void completely_cleanout_room(struct room_data *rp);
void load_one_room(FILE *fl, struct room_data *rp);
void boot_world(void);
void allocate_room(int room_number);
void setup_dir(FILE *fl, int room, int dir);
void renum_zone_table(void);
void boot_zones(void);
void fread_dice(FILE *fp, long int *x, long int *y, long int *z);
struct char_data *read_mobile(int nr, int type);
struct obj_data *read_object(int nr, int type);
void zone_update(void);
void reset_zone(int zone);
int is_empty(int zone_nr);
int load_char(char *name, struct char_file_u *char_element);
void store_to_char(struct char_file_u *st, struct char_data *ch);
void char_to_store(struct char_data *ch, struct char_file_u *st);
int create_entry(char *name);
void save_char(struct char_data *ch, short int load_room);
void new_save_char(struct char_file_u *ch, char *filename, struct char_data *xch);
int compare(struct player_index_element *arg1, struct player_index_element *arg2);
void free_char(struct char_data *ch);
void free_obj(struct obj_data *obj);
int file_to_string(const char *name, char *buf);
int file_to_prompt(const char *name, char *buf);
void reset_char(struct char_data *ch);
void clear_char(struct char_data *ch);
void clear_object(struct obj_data *obj);
void init_char(struct char_data *ch);
struct room_data *real_roomp(int vnum);
int real_mobile(int vnum);
int real_object(int vnum);

char *fix_string(const char *str);
char *fread_string(FILE *fl);
char *fread_word(FILE *fp);
int fread_number(FILE *fp);
void fread_to_eol(FILE *fp);
int fread_char(char *name, struct char_file_u *char_element, struct char_data *xch);
char *new_fread_string(FILE *fp);

#endif

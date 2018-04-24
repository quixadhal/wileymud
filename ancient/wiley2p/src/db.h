/* ************************************************************************
*  file: db.h , Database module.                          Part of DIKUMUD *
*  Usage: Loading/Saving chars booting world.                             *
************************************************************************* */


/* data files used by the game system */

#define DFLT_DIR          "lib"           /* default data directory     */
#define WORLD_FILE        "wld/tinyworld.wld" /* room definitions           */
#define MOB_FILE          "wld/tinyworld.mob" /* monster prototypes         */
#define OBJ_FILE          "wld/tinyworld.obj" /* object prototypes          */
#define ZONE_FILE         "wld/tinyworld.zon" /* zone defs & command tables */
#define CREDITS_FILE      "etc/credits"       /* for the 'credits' command  */
#define NEWS_FILE         "etc/news"          /* for the 'news' command     */
#define MOTD_FILE         "etc/motd"          /* messages of today          */
#define WMOTD_FILE        "etc/wmotd"         /* messages of today          */
#define PLAYER_FILE       "ply/players"       /* the player database        */
#define TIME_FILE         "etc/time"          /* game calendar information  */
#define IDEA_FILE         "etc/ideas"         /* for the 'idea'-command     */
#define TYPO_FILE         "etc/typos"         /*         'typo'             */
#define BUG_FILE          "etc/bugs"          /*         'bug'              */
#define MESS_FILE         "etc/messages"      /* damage message             */
#define SOCMESS_FILE      "etc/actions"       /* messgs for social acts     */
#define HELP_KWRD_FILE    "man/help_table"    /* for HELP <keywrd>          */
#define HELP_PAGE_FILE    "man/help"          /* for HELP <CR>              */
#define INFO_FILE         "etc/info"          /* for INFO                   */
#define WIZLIST_FILE      "etc/wizlist"       /* for WIZLIST                */
#define POSEMESS_FILE     "etc/poses"         /* for 'pose'-command         */
#define BOARD_FILE_PATH   "etc"
#define REBOOT_FILE	  "etc/reboot"
#define OBJ_SAVE_FILE	  "ply/pcobjs.obj"
#define SHOP_FILE	  "wld/tinyworld.shp"
#define MKZONE_PATH	  "wld/mkzone"

#define DEFAULT_HOME		3	/* when a player is created */

/* public procedures in db.c */

void boot_db(void);
void save_char(struct char_data *ch, sh_int load_room);
int create_entry(char *name);
void zone_update(void);
void init_char(struct char_data *ch);
void clear_char(struct char_data *ch);
void clear_object(struct obj_data *obj);
void reset_char(struct char_data *ch);
void free_char(struct char_data *ch);
struct room_data *real_roomp(int virtual);
char *fread_string(FILE *fl);
int real_object(int virtual);
int real_mobile(int virtual);

#define REAL 0
#define VIRTUAL 1

struct obj_data *read_object(int nr, int type);
struct char_data *read_mobile(int nr, int type);


/* structure for the reset commands */
struct reset_com
{
	char command;   /* current command                      */ 
	bool if_flag;   /* if TRUE: exe only if preceding exe'd */
	int arg1;       /*                                      */
	int arg2;       /* Arguments to the command             */
	int arg3;       /*                                      */

	/* 
	*  Commands:              *
	*  'M': Read a mobile     *
	*  'O': Read an object    *
	*  'L': Has a leader      *
	*  'G': Give obj to mob   *
	*  'P': Put obj in obj    *
	*  'G': Obj to char       *
	*  'E': Obj to char equip *
	*  'D': Set state of door *
	*/
};



/* zone definition structure. for the 'zone-table'   */
struct zone_data
{
	char *name;             /* name of this zone                  */
	int lifespan;           /* how long between resets (minutes)  */
	int age;                /* current age of this zone (minutes) */
	int top;                /* upper limit for rooms in this zone */

	int reset_mode;         /* conditions for reset (see below)   */
	struct reset_com *cmd;  /* command table for reset	           */

	/*
	*  Reset mode:                              *
	*  0: Don't reset, and don't update age.    *
	*  1: Reset if no PC's are located in zone. *
	*  2: Just reset.                           *
	*/
};




/* element in monster and object index-tables   */
struct index_data
{
	int virtual;    /* virtual number of this mob/obj           */
	long pos;       /* file position of this field              */
	int number;     /* number of existing units of this mob/obj	*/
	int (*func)();  /* special procedure for this mob/obj       */
	char *name;
};


struct obj_index_data
{
	int virtual;    /* virtual number of this mob/obj           */
	long pos;       /* file position of this field              */
	int number;     /* number of existing units of this mob/obj	*/
	int (*func)();  /* special procedure for this mob/obj       */
	char *name;
	struct obj_data *template; /* for preloading of objects */
};




/* for queueing zones for update   */
struct reset_q_element
{
	int zone_to_reset;            /* ref to zone_data */
	struct reset_q_element *next;	
};



/* structure for the update queue     */
struct reset_q_type
{
	struct reset_q_element *head;
	struct reset_q_element *tail;
} reset_q;



struct player_index_element
{
	char *name;
	int nr;
};


struct help_index_element
{
	char *keyword;
	long pos;
};

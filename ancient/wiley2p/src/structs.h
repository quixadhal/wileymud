/* ************************************************************************
*  file: structs.h , Structures        .                  Part of DIKUMUD *
*  Usage: Declarations of central data structures                         *
************************************************************************* */

#include <sys/types.h>

typedef char sbyte;
typedef unsigned char ubyte;
typedef short int sh_int;
typedef unsigned short int ush_int;
typedef char bool;
typedef char byte;

#define WLD_FILE_DIRECTORY 	"wld"
#define MAX_WLD_FILE_ENTRIES	25

#define MAX_DESC_LENGTH		1600	/* 80 chars, 20 rows */
#define MAX_MY_STRING_LENGTH  	80	/* 80 chars, 1 row */
#define MAX_NUM_EXITS   	6

#define DEFAULT_ROOM_NAME 		0
#define DEFAULT_ROOM_ZONE 		0
#define DEFAULT_ROOM_SECT 		1
#define DEFAULT_ROOM_RIVER_DIR		-1
#define DEFAULT_ROOM_RIVER_SPEED	0
#define DEFAULT_ROOM_TELE_TIME		-1
#define DEFAULT_ROOM_TELE_TARG		-1
#define DEFAULT_ROOM_TELE_LOOK		1
#define DEFAULT_ROOM_DESC		'\0'
#define DEFAULT_ROOM_EX_DESC		'\0'
#define DEFAULT_ROOM_FLAGS		0
#define DEFAULT_ROOM_SOUND		'\0'
#define DEFAULT_ROOM_DISTANT_SOUND	'\0'
#define DEFAULT_ROOM_LIGHT		0
#define DEFAULT_ROOM_FUNCT		0
#define DEFAULT_ROOM_CONTENTS		0
#define DEFAULT_ROOM_PEOPLE		0

#define DEFAULT_EXIT_GENERAL_DESCRIPTION '\0'
#define DEFAULT_EXIT_KEYWORD '\0'
#define DEFAULT_EXIT_EXIT_INFO 0
#define DEFAULT_EXIT_KEY -1
#define DEFAULT_EXIT_TO_ROOM 0
#define DEFAULT_EXIT_ALIAS '\0'

#define MAX_CONDITIONS	6
#define MAX_NUMBER_OF_CLASSES 12

#define MAGE_LEVEL_IND    0
#define CLERIC_LEVEL_IND  1
#define WARRIOR_LEVEL_IND 2
#define THIEF_LEVEL_IND   3
#define RANGER_LEVEL_IND  4
#define DRUID_LEVEL_IND   5

#define FIRE_DAMAGE 1
#define COLD_DAMAGE 2
#define ELEC_DAMAGE 3
#define BLOW_DAMAGE 4
#define ACID_DAMAGE 5

#define HATE_SEX   1
#define HATE_RACE  2
#define HATE_CHAR  4
#define HATE_CLASS 8
#define HATE_EVIL  16
#define HATE_GOOD  32
#define HATE_VNUM  64

#define FEAR_SEX   1
#define FEAR_RACE  2
#define FEAR_CHAR  4
#define FEAR_CLASS 8
#define FEAR_EVIL  16
#define FEAR_GOOD  32
#define FEAR_VNUM  64

#define OP_SEX   1
#define OP_RACE  2
#define OP_CHAR  3
#define OP_CLASS 4
#define OP_EVIL  5
#define OP_GOOD  6
#define OP_VNUM  7

#define ABS_MAX_LVL  61
#define MAX_MORT     50
#define LOW_IMMORTAL 51
#define IMMORTAL     51
#define CREATOR      52
#define SAINT        53
#define DEMIGOD      54
#define LESSER_GOD   55
#define GOD          56
#define GREATER_GOD  57
#define SILLYLORD    58
#define IMPLEMENTOR  59
#define LOKI         60

#define IMM_FIRE        1
#define IMM_COLD        2
#define IMM_ELEC        4
#define IMM_ENERGY      8
#define IMM_BLUNT      16
#define IMM_PIERCE     32
#define IMM_SLASH      64
#define IMM_ACID      128
#define IMM_POISON    256
#define IMM_DRAIN     512
#define IMM_SLEEP    1024
#define IMM_CHARM    2048
#define IMM_HOLD     4096
#define IMM_NONMAG   8192
#define IMM_PLUS1   16384
#define IMM_PLUS2   32768
#define IMM_PLUS3   65536
#define IMM_PLUS4  131072

#define PULSE_RIVER    10
#define PULSE_TELEPORT      10
#define PULSE_SOUND	30	

#define MAX_ROOMS   5000

#define MAX_MAIL_MSGS 25        /* Max number of messages. */
#define MAX_MAIL_LENGTH 2048    /* that should be enough */
#define MAIL_PATH "mail"        /* this is offset by the location of lib */

struct MailRecord
{
    char from[40];
    char title[40];
    long int date_sent;
    long int date_read;
    char m[MAX_MAIL_LENGTH];
    int keep;
    int new;
};

struct Mail
{
    int NumberOfMessages;
    struct MailRecord *mail_record[MAX_MAIL_MSGS];
};

struct nodes
{
  int visited;
  int ancestor;
};

struct room_q
{
  int room_nr;
  struct room_q *next_q;
};

struct string_block {
  int	size;
  char	*data;
};


/*
  memory stuff 
*/

struct char_list {
  struct char_data *op_ch;
  char name[50];
  struct char_list *next;
};

typedef struct {
       struct char_list  *clist;
       int    sex;   /*number 1=male,2=female,3=both,4=neut,5=m&n,6=f&n,7=all*/
       int    race;  /*number */
       int    class; /* 1=m,2=c,4=f,8=t */
       int    vnum;  /* # */
       int    evil;  /* align < evil = attack */
       int    good;  /* align > good = attack */
}  Opinion;




/*
   old stuff.
*/

#define PULSE_ZONE     240
#define PULSE_MOBILE   25 
#define PULSE_VIOLENCE 6 
#define WAIT_SEC       4
#define WAIT_ROUND     4

#define MAX_STRING_LENGTH   4096
#define MAX_INPUT_LENGTH     160
#define MAX_MESSAGES          60
#define MAX_ITEMS            153

#define MESS_ATTACKER 1
#define MESS_VICTIM   2
#define MESS_ROOM     3

#define SECS_PER_REAL_MIN  60
#define SECS_PER_REAL_HOUR (60*SECS_PER_REAL_MIN)
#define SECS_PER_REAL_DAY  (24*SECS_PER_REAL_HOUR)
#define SECS_PER_REAL_YEAR (365*SECS_PER_REAL_DAY)

#define SECS_PER_MUD_HOUR  	75
#define HOURS_PER_MUD_DAY	24
#define DAYS_PER_MUD_MONTH	35
#define MONTHS_PER_MUD_YEAR	17
#define SECS_PER_MUD_DAY   (HOURS_PER_MUD_DAY*SECS_PER_MUD_HOUR)
#define SECS_PER_MUD_MONTH (DAYS_PER_MUD_MONTH*SECS_PER_MUD_DAY)
#define SECS_PER_MUD_YEAR  (MONTHS_PER_MUD_YEAR*SECS_PER_MUD_MONTH)

/* The following defs are for obj_data  */

/* For 'type_flag' */

#define ITEM_LIGHT      1
#define ITEM_SCROLL     2
#define ITEM_WAND       3
#define ITEM_STAFF      4
#define ITEM_WEAPON     5
#define ITEM_FIREWEAPON 6
#define ITEM_MISSILE    7
#define ITEM_TREASURE   8
#define ITEM_ARMOR      9
#define ITEM_POTION    10 
#define ITEM_WORN      11
#define ITEM_OTHER     12
#define ITEM_TRASH     13
#define ITEM_TRAP      14
#define ITEM_CONTAINER 15
#define ITEM_NOTE      16
#define ITEM_DRINKCON  17
#define ITEM_KEY       18
#define ITEM_FOOD      19
#define ITEM_MONEY     20
#define ITEM_PEN       21
#define ITEM_BOAT      22
#define ITEM_AUDIO     23
#define ITEM_BOARD     24
#define ITEM_KENNEL    25

/* Bitvector For 'wear_flags' */
#define ITEM_TAKE              1 
#define ITEM_WEAR_FINGER       2
#define ITEM_WEAR_NECK         4
#define ITEM_WEAR_BODY         8
#define ITEM_WEAR_HEAD        16
#define ITEM_WEAR_LEGS        32
#define ITEM_WEAR_FEET        64
#define ITEM_WEAR_HANDS      128 
#define ITEM_WEAR_ARMS       256
#define ITEM_WEAR_SHIELD     512
#define ITEM_WEAR_ABOUT     1024 
#define ITEM_WEAR_WAISTE    2048
#define ITEM_WEAR_WRIST     4096
#define ITEM_WIELD          8192
#define ITEM_HOLD          16384
#define ITEM_WIELD_TWOH    32768

/* UNUSED, CHECKS ONLY FOR ITEM_LIGHT #define ITEM_LIGHT_SOURCE  65536 */

/* Bitvector for 'extra_flags' */

#define ITEM_GLOW            1
#define ITEM_HUM             2
#define ITEM_METAL           4  /* undefined...  */
#define ITEM_MINERAL         8  /* undefined?    */
#define ITEM_ORGANIC        16  /* undefined?    */
#define ITEM_INVISIBLE      32
#define ITEM_MAGIC          64
#define ITEM_NODROP        128
#define ITEM_BLESS         256
#define ITEM_ANTI_GOOD     512 /* not usable by good people    */
#define ITEM_ANTI_EVIL    1024 /* not usable by evil people    */
#define ITEM_ANTI_NEUTRAL 2048 /* not usable by neutral people */
#define ITEM_ANTI_CLERIC  4096
#define ITEM_ANTI_MAGE    8192
#define ITEM_ANTI_THIEF   16384
#define ITEM_ANTI_FIGHTER 32768
#define ITEM_ANTI_RANGER  65536  
#define ITEM_PARISH	  131072

/* Some different kind of liquids */

#define LIQ_WATER      0
#define LIQ_BEER       1
#define LIQ_WINE       2
#define LIQ_ALE        3
#define LIQ_DARKALE    4
#define LIQ_WHISKY     5
#define LIQ_LEMONADE   6
#define LIQ_FIREBRT    7
#define LIQ_LOCALSPC   8
#define LIQ_SLIME      9
#define LIQ_MILK       10
#define LIQ_TEA        11
#define LIQ_COFFE      12
#define LIQ_BLOOD      13
#define LIQ_SALTWATER  14
#define LIQ_COKE       15

/* for containers  - value[1] */

#define CONT_CLOSEABLE      1
#define CONT_PICKPROOF      2
#define CONT_CLOSED         4
#define CONT_LOCKED         8

struct extra_descr_data
{
	char *keyword;                 /* Keyword in look/examine          */
	char *description;             /* What to see                      */
	struct extra_descr_data *next; /* Next in list                     */
};

#define MAX_OBJ_AFFECT 2         /* Used in OBJ_FILE_ELEM *DO*NOT*CHANGE* */
#define OBJ_NOTIMER    -7000000

struct obj_flag_data
{
	int value[4];       /* Values of the item (see list)    */
	byte type_flag;     /* Type of item                     */
	int wear_flags;     /* Where you can wear it            */
	long extra_flags;    /* If it hums,glows etc             */
	int weight;         /* Weigt what else                  */
	int cost;           /* Value when sold (gp.)            */
	int cost_per_day;   /* Cost to keep pr. real day        */
	int timer;          /* Timer for object                 */
	long bitvector;     /* To set chars bits                */
};

/* Used in OBJ_FILE_ELEM *DO*NOT*CHANGE* */
struct obj_affected_type {
	byte location;      /* Which ability to change (APPLY_XXX) */
	byte modifier;      /* How much it changes by              */
};


/* ======================== Structure for object ========================= */
struct obj_data
{
	sh_int item_number;            /* Where in data-base               */
	int in_room;                /* In what room -1 when conta/carr  */ 
	struct obj_flag_data obj_flags;/* Object information               */
	struct obj_affected_type
	    affected[MAX_OBJ_AFFECT];  /* Which abilities in PC to change  */

	struct char_data *killer;      /* for use with corpses */
        sh_int char_vnum;              /* for ressurection     */
	long   char_f_pos;             /* for ressurection     */
	char *name;                    /* Title of object :get etc.        */
	char *description ;            /* When in room                     */
	char *short_description;       /* when worn/carry/in cont.         */
 	char *action_description;      /* What to write when used          */
 	struct extra_descr_data *ex_description; /* extra descriptions     */
	struct char_data *carried_by;  /* Carried by :NULL in room/conta   */
	byte   eq_pos;                 /* what is the equip. pos?          */
	struct char_data *equipped_by; /* equipped by :NULL in room/conta  */

	struct obj_data *in_obj;       /* In what object NULL when none    */
	struct obj_data *contains;     /* Contains objects                 */

	struct obj_data *next_content; /* For 'contains' lists             */
	struct obj_data *next;         /* For the object list              */
};
/* ======================================================================= */

/* The following defs are for room_data  */

#define NOWHERE    -1    /* nil reference for room-database    */

/* Bitvector For 'room_flags' */

#define DARK           1
#define DEATH          2
#define NO_MOB         4
#define INDOORS        8
#define PEACEFUL      16  /* No fighting */
#define NO_STEAL      32  /* No Thieving */
#define NO_SUM        64  /* no summoning */
#define NO_MAGIC     128
#define TUNNEL       256 /* ? */
#define PRIVATE      512
#define SOUND	    1024

/* For 'dir_option' */

#define NORTH          0
#define EAST           1
#define SOUTH          2
#define WEST           3
#define UP             4
#define DOWN           5

#define EX_ISDOOR      	1
#define EX_CLOSED      	2
#define EX_LOCKED      	4
#define EX_SECRET	8
#define EX_TRAPPED	16
#define EX_PICKPROOF    32
#define EX_ALIAS	64

/* For 'Sector types' */

#define SECT_INSIDE          0
#define SECT_CITY            1
#define SECT_FIELD           2
#define SECT_FOREST          3
#define SECT_HILLS           4
#define SECT_MOUNTAIN        5
#define SECT_WATER_SWIM      6
#define SECT_WATER_NOSWIM    7
#define SECT_AIR             8
#define SECT_UNDERWATER      9

struct room_direction_data
{
	char *general_description;       /* When look DIR.                  */ 
	char *keyword;                   /* for open/close                  */	
	sh_int exit_info;                /* Exit info                       */
	int key;		                   /* Key's number (-1 for no key)    */
	int to_room;                  /* Where direction leeds (NOWHERE) */
	char *exit_alias;		/* exit aliasing */
};

/* ========================= Structure for room ========================== */
struct room_data
{
	sh_int number;               /* Rooms number                       */
	sh_int zone;                 /* Room zone (for resetting)          */
	int sector_type;             /* sector type (move/hide)            */

        int river_dir;               /* dir of flow on river               */
        int river_speed;             /* speed of flow on river             */

	int  tele_time;              /* time to a teleport                 */
	int  tele_targ;              /* target room of a teleport          */
	char tele_look;              /* do a do_look or not when 
					teleported                         */


	char *name;                  /* Rooms name 'You are ...'           */
	char *description;           /* Shown when entered                 */
	struct extra_descr_data *ex_description; /* for examine/look       */
	struct room_direction_data *dir_option[6]; /* Directions           */
	sh_int room_flags;           /* DEATH,DARK ... etc                 */ 

	char *sound;		     /* sound made in this room            */
	char *distant_sound;	     /* sound made in ajoinging room       */

	byte light;                  /* Number of lightsources in room     */

	int (*funct)();              /* special procedure                  */
         
	struct obj_data *contents;   /* List of items in room              */
	struct char_data *people;    /* List of NPC / PC in room           */
};
/* ======================================================================== */

/* The following defs and structures are related to char_data   */

/* For 'equipment' */

#define WEAR_LIGHT      0
#define WEAR_FINGER_R   1
#define WEAR_FINGER_L   2
#define WEAR_NECK_1     3
#define WEAR_NECK_2     4
#define WEAR_BODY       5
#define WEAR_HEAD       6
#define WEAR_LEGS       7
#define WEAR_FEET       8
#define WEAR_HANDS      9
#define WEAR_ARMS      10
#define WEAR_SHIELD    11
#define WEAR_ABOUT     12
#define WEAR_WAISTE    13
#define WEAR_WRIST_R   14
#define WEAR_WRIST_L   15
#define WIELD          16
#define HOLD           17
#define WIELD_TWOH	18

/* For 'char_payer_data' */

/*
**  #2 has been used!!!!  Don't try using the last of the 3, because it is
**  the keeper of active/inactive status for dead characters for ressurection!
*/

#define MAX_TOUNGE  3     /* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
#define MAX_SKILLS  200   /* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
#define MAX_WEAR    19 
#define MAX_AFFECT  25    /* Used in CHAR_FILE_U *DO*NOT*CHANGE* */

/* Predifined  conditions */
#define DRUNK        0
#define FULL         1
#define THIRST       2

/* Bitvector for 'affected_by' */
#define AFF_BLIND             0x00000001
#define AFF_INVISIBLE         0x00000002
#define AFF_DETECT_EVIL       0x00000004
#define AFF_DETECT_INVISIBLE  0x00000008
#define AFF_DETECT_MAGIC      0x00000010
#define AFF_SENSE_LIFE        0x00000020
#define AFF_SILENCED          0x00000040  /* this may have to go */
#define AFF_SANCTUARY         0x00000080
#define AFF_GROUP             0x00000100

/* there is one missing here....... */
#define AFF_CURSE             0x00000400
#define AFF_FLYING            0x00000800
#define AFF_POISON            0x00001000
#define AFF_PROTECT_EVIL      0x00002000
#define AFF_PARALYSIS         0x00004000
#define AFF_INFRAVISION       0x00008000
#define AFF_WATERBREATH       0x00010000
#define AFF_SLEEP             0x00020000
#define AFF_DRUG_FREE         0x00040000  /* i.e. can't be stoned */
#define AFF_SNEAK             0x00080000
#define AFF_HIDE              0x00100000
#define AFF_FEAR              0x00200000  /* this may have to go */
#define AFF_CHARM             0x00400000
#define AFF_FOLLOW            0x00800000
#define AFF_UNDEF_1           0x01000000  /* saved objects?? */
#define AFF_TRUE_SIGHT        0x02000000
#define AFF_SCRYING           0x04000000   /* seeing other rooms */
#define AFF_FIRESHIELD        0x08000000
#define AFF_RIDE              0x10000000
#define AFF_UNDEF_6           0x20000000
#define AFF_UNDEF_7           0x40000000
#define AFF_UNDEF_8           0x80000000

/* modifiers to char's abilities */

#define APPLY_NONE              0
#define APPLY_STR               1
#define APPLY_DEX               2
#define APPLY_INT               3
#define APPLY_WIS               4
#define APPLY_CON               5
#define APPLY_SEX               6
#define APPLY_CLASS             7
#define APPLY_LEVEL             8
#define APPLY_AGE               9
#define APPLY_CHAR_WEIGHT      10
#define APPLY_CHAR_HEIGHT      11
#define APPLY_MANA             12
#define APPLY_HIT              13
#define APPLY_MOVE             14
#define APPLY_GOLD             15
#define APPLY_EXP              16
#define APPLY_AC               17
#define APPLY_ARMOR            17
#define APPLY_HITROLL          18
#define APPLY_DAMROLL          19
#define APPLY_SAVING_PARA      20
#define APPLY_SAVING_ROD       21
#define APPLY_SAVING_PETRI     22
#define APPLY_SAVING_BREATH    23
#define APPLY_SAVING_SPELL     24
#define APPLY_SAVE_ALL         25
#define APPLY_IMMUNE           26
#define APPLY_SUSC             27
#define APPLY_M_IMMUNE         28
#define APPLY_SPELL            29
#define APPLY_WEAPON_SPELL     30
#define APPLY_EAT_SPELL        31
#define APPLY_BACKSTAB         32
#define APPLY_KICK             33
#define APPLY_SNEAK            34
#define APPLY_HIDE             35
#define APPLY_BASH             36
#define APPLY_PICK             37
#define APPLY_STEAL            38
#define APPLY_TRACK            39
#define APPLY_HITNDAM          40

/* 'class' for PC's */
#define CLASS_ALL ( CLASS_MAGIC_USER | CLASS_CLERIC | CLASS_WARRIOR | CLASS_THIEF | CLASS_RANGER | CLASS_DRUID )
#define CLASS_MAGIC_USER  1
#define CLASS_CLERIC      2
#define CLASS_WARRIOR     4
#define CLASS_THIEF       8
#define CLASS_RANGER	  16
#define CLASS_DRUID	  32

/* sex */
#define SEX_NEUTRAL   0
#define SEX_MALE      1
#define SEX_FEMALE    2

/* positions */
#define POSITION_DEAD       0
#define POSITION_MORTALLYW  1
#define POSITION_INCAP      2
#define POSITION_STUNNED    3
#define POSITION_SLEEPING   4
#define POSITION_RESTING    5
#define POSITION_SITTING    6
#define POSITION_FIGHTING   7
#define POSITION_STANDING   8
#define POSITION_MOUNTED    9


/* for mobile actions: specials.act */
#define ACT_SPEC       (1<<0)  /* special routine to be called if exist   */
#define ACT_SENTINEL   (1<<1)  /* this mobile not to be moved             */
#define ACT_SCAVENGER  (1<<2)  /* pick up stuff lying around              */
#define ACT_ISNPC      (1<<3)  /* This bit is set for use with IS_NPC()   */
#define ACT_NICE_THIEF (1<<4)  /* Set if a thief should NOT be killed     */
#define ACT_AGGRESSIVE (1<<5)  /* Set if automatic attack on NPC's        */
#define ACT_STAY_ZONE  (1<<6)  /* MOB Must stay inside its own zone       */
#define ACT_WIMPY      (1<<7)  /* MOB Will flee when injured, and if      */
                               /* aggressive only attack sleeping players */
#define ACT_ANNOYING   (1<<8)  /* MOB is so utterly irritating that other */
                               /* monsters will attack it...              */
#define ACT_HATEFUL    (1<<9)  /* MOB will attack a PC or NPC matching a  */
                               /* specified name                          */
#define ACT_AFRAID    (1<<10)  /* MOB is afraid of a certain PC or NPC,   */
                               /* and will always run away ....           */
#define ACT_IMMORTAL  (1<<11)  /* MOB is a natural event, can't be kiled  */
#define ACT_HUNTING   (1<<12)  /* MOB is hunting someone                  */
#define ACT_DEADLY    (1<<13)  /* MOB has deadly poison                   */
#define ACT_POLYSELF  (1<<14)  /* MOB is a polymorphed person             */
#define ACT_POLYOTHER (1<<15)  /* MOB is a polymorphed person             */
#define ACT_GUARDIAN  (1<<16)  /* MOB will guard master                   */
#define ACT_USE_ITEM  (1<<17)  /* Will use an item when picked up         */
#define ACT_FIGHTER_MOVES (1<<18) /* mob will attack like a fighter */
#define ACT_FOOD_PROVIDE (1<<19)
#define ACT_PROTECTOR (1<<20)    /* mob will come to aid those that call */
				 /* this is inter zonal only             */
#define ACT_MOUNT     (1<<21)	/* mobile can act as a mount	*/

/* For players : specials.act */
#define PLR_BRIEF     (1<<0)
#define PLR_COMPACT   (1<<2)
#define PLR_DONTSET   (1<<3) /* Dont EVER set */
#define PLR_WIMPY     (1<<4) /* character will flee when seriously injured */
#define PLR_NOHASSLE  (1<<5) /* char won't be attacked by aggressives.      */
#define PLR_STEALTH   (1<<6) /* char won't be seen in a variety of situations */
#define PLR_HUNTING   (1<<7) /* the player is hunt somene,doa track each look */
#define PLR_DEAF      (1<<8) /* The player does not hear shouts */
#define PLR_ECHO      (1<<9) /* Messages (tells, shout,etc) echo back */
#define PLR_NOSHOUT   (1<<14)/* the player is not allowed to shout */
#define PLR_PAGER     (1<<15)/* Use the Game pager or not */
#define PLR_LOGS      (1<<16)/* Player is writing tothe board */

/* the following are used in ch->specials.new_act */

#define NEW_PLR_KILLOK (1<<0) /* players can kill each other */
#define NEW_PLR_NOTELL (1<<1) /* players can't recieve tells */
#define NEW_PLR_SUMMON (1<<2) /* This player is a pc killer */
#define NEW_PLR_TELEPORT (1<<3) /* This player is a pc thief */

#define PRIV_GIVE_PRIV  (1<<0) /* can this player give privs */
#define PRIV_FORCE	(1<<1)
#define PRIV_TRANS	(1<<2) /* can the player use trans */
#define PRIV_SHUTDOWN	(1<<3)
#define PRIV_LOAD_PURGE	(1<<4) /* can the playe load-purge mobs/objects */
#define PRIV_RESTORE	(1<<5) /* can the player Heal to full */
#define PRIV_RESTORE_A  (1<<6) /* can the player heal everyone to full */
#define PRIV_STAT	(1<<7) /* can you stat anything in the game */
#define PRIV_SWITCH	(1<<8)
#define PRIV_REROLL_ADVANCE_PSET (1<<9) /* can the player use the commands
					   reroll to change stats
					   advance to give levels
					   pset to mod abilities
                                         */
#define PRIV_SNOOP	(1<<10)
#define PRIV_STRING_TITLE	(1<<11)
#define PRIV_BUILDER	(1<<12)	/* This priv will allow the player to use
				   form, which forms a room,
				   cust, which allows in game changes,
				   sroom, to save the rooms
				   lroom, to load the rooms
				   mkzone, to make a zone
				   show, to show lots of things
				   */
#define PRIV_PRETITLE	(1<<13)
#define PRIV_WHOD	(1<<14)
#define PRIV_SYSTEM	(1<<15)
#define PRIV_LOGS	(1<<16)
#define PRIV_DEBUG	(1<<17)
#define PRIV_GOTO_AT	(1<<18)

/* These data contain information about a players time data */
struct time_data
{
  time_t birth;    /* This represents the characters age                */
  time_t logon;    /* Time of the last logon (used to calculate played) */
  int played;      /* This is the total accumulated time played in secs */
};

struct char_player_data
{
	char *name;    	    /* PC / NPC s name (kill ...  )         */
	char *short_descr;  /* for 'actions'                        */
	char *long_descr;   /* for 'look'.. Only here for testing   */
	char *description;  /* Extra descriptions                   */
	char *pre_title;
	char *title;        /* PC / NPC s title                     */
	char *poof_in;
	char *poof_out;
	char *guild_name;
	char *sounds;       /* Sound that the monster makes (in room) */
	char *distant_snds; /* Sound that the monster makes (other) */
	byte sex;           /* PC / NPC s sex                       */
	byte class;         /* PC s class or NPC alignment          */
	byte level[6];         /* PC / NPC s level                     */
	int hometown;       /* PC s Hometown (zone)                 */
	bool talks[MAX_TOUNGE]; /* PC s Tounges 0 for NPC           */
 	struct time_data time; /* PC s AGE in days                 */
	ubyte weight;       /* PC / NPC s weight                    */
	ubyte height;       /* PC / NPC s height                    */
};


/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_ability_data
{
	sbyte str; 
	sbyte str_add;      /* 000 - 100 if strength 18             */
	sbyte intel;
	sbyte wis; 
	sbyte dex; 
	sbyte con;
	sbyte d1;	/* these are added for later use */
	sbyte d2;
	sbyte d3;
	sbyte d4; 
};


/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_point_data
{

        sh_int mana;         
        sh_int max_mana;

	sh_int hit;   
	sh_int max_hit;      /* Max hit for NPC                         */
	sh_int move;  
	sh_int max_move;     /* Max move for NPC                        */
        sh_int blah1;
        sh_int blah2;
        sh_int blah3;

	sh_int armor;        /* Internal -100..100, external -10..10 AC */
	int gold;            /* Money carried                           */
        int bankgold;        /* gold in the bank.                       */
	int exp;             /* The experience of the player            */
        int i1;
        int i2;
	int i3;
	int i4;
        int wiz_priv;		/* what kind of wiz privs they get */

	sbyte hitroll;       /* Any bonus or penalty to the hit roll    */
	sbyte damroll;       /* Any bonus or penalty to the damage roll */
	sbyte s1;
	sbyte s2;
	sbyte s3;
};


struct char_special_data
{
	struct char_data *fighting; /* Opponent                             */
	struct char_data *hunting;  /* Hunting person..                     */
	struct char_data *mounted_on;
	struct char_data *ridden_by;

	long affected_by;  /* Bitvector for spells/skills affected by */ 
        long long_blah1;
        long long_blah2;

	byte position;           /* Standing or ...                         */
	byte default_pos;        /* Default position for NPC                */
	unsigned long act;      /* flags for NPC behavior                  */
        int new_act;            /* new flags */
	byte spells_to_learn;    /* How many can you learn yet this level   */

	int carry_weight;        /* Carried weight                          */
	byte carry_items;        /* Number of items carried                 */
	int timer;               /* Timer for update                        */
	int was_in_room;      /* storage of location for linkdead people */
	sh_int apply_saving_throw[5]; /* Saving throw (Bonuses)             */
	sbyte conditions[3];      /* Drunk full etc.                        */

	byte damnodice;           /* The number of damage dice's            */
	byte damsizedice;         /* The size of the damage dice's          */
	byte last_direction;      /* The last direction the monster went    */
	int attack_type;          /* The Attack Type Bitvector for NPC's    */
	int alignment;            /* +-1000 for alignments                  */
};


/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_skill_data
{
	byte learned;           /* % chance for success 0 = not learned   */
	bool recognise;         /* If you can recognise the scroll etc.   */
};

/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct affected_type
{
	short type;           /* The type of spell that caused this      */
	sh_int duration;      /* For how long its effects will last      */
	sbyte modifier;       /* This is added to apropriate ability     */
	byte location;        /* Tells which ability to change(APPLY_XXX)*/
	long bitvector;       /* Tells which bits to set (AFF_XXX)       */
	struct affected_type *next;
};

struct follow_type
{
	struct char_data *follower;
	struct follow_type *next;
};

/* ================== Structure for player/non-player ===================== */
struct char_data
{
	sh_int nr;                            /* monster nr (pos in file)    */
	int in_room;                       /* Location                    */
	struct Mail *mail;
/* 
  will need several new affects
  maybe these should go in points?  (and thusly be saved and restored?)
*/

	sh_int immune;                        /* Immunities                  */
	sh_int M_immune;                      /* Meta Immunities             */
        sh_int susc;                          /* susceptibilities            */
	sh_int mult_att;                      /* the number of attacks       */
	byte   attackers;

	sh_int fallspeed;                     /* rate of descent for player */
	sh_int race;
	sh_int hunt_dist;                    /* max dist the player can hunt */

        unsigned short hatefield;
        unsigned short fearfield;

	Opinion hates;
	Opinion fears;

	sh_int  persist;
	int     old_room;

	int     act_ptr;    /* numeric argument for the mobile actions */

	struct char_player_data player;       /* Normal data                 */
	struct char_ability_data abilities;   /* Abilities                   */
	struct char_ability_data tmpabilities;/* The abilities we will use   */
	struct char_point_data points;        /* Points                      */
	struct char_special_data specials;    /* Special plaing constants    */
	struct char_skill_data skills[MAX_SKILLS]; /* Skills                 */

	struct affected_type *affected;       /* affected by what spells     */
	struct obj_data *equipment[MAX_WEAR]; /* Equipment array             */

	struct obj_data *carrying;            /* Head of list                */
	struct descriptor_data *desc;         /* NULL for mobiles            */
	struct char_data *orig;               /* Special for polymorph       */

	struct char_data *next_in_room;     /* For room->people - list       */
	struct char_data *next;             /* For either monster or ppl-lis */
	struct char_data *next_fighting;    /* For fighting list             */

	struct follow_type *followers;        /* List of chars followers     */
	struct char_data *master;             /* Who is char following?      */
	int	invis_level;		      /* visibility of gods */

};


/* ======================================================================== */

/* This structure is purely intended to be an easy way to transfer */
/* and return information about time (real or mudwise).            */

struct time_info_data
{
  byte hours;
  byte day;
  byte month;
  sh_int year;
};

/* How much light is in the land ? */
#define SUN_DARK	0
#define SUN_RISE	1
#define SUN_LIGHT	2
#define SUN_SET		3

/* And how is the sky ? */
#define SKY_CLOUDLESS	0
#define SKY_CLOUDY	1
#define SKY_RAINING	2
#define SKY_LIGHTNING	3

/* Which way is the wind blowing */
#define WIND_NORTH	0
#define WIND_EAST	1
#define WIND_SOUTH	2
#define WIND_WEST	3
#define WIND_DEAD	4

struct weather_data
{
  int pressure;	/* How is the pressure ( Mb ) */
  int change;	/* How fast and what way does it change. */
  int sky;	/* How is the sky. */
  int sunlight;	/* And how much sun. */
  int wind_speed;	/* how hard is the wind blowing */
  int wind_direction;	/* which direction is the wind blowing */
  int moon;		/* what is the moon like */
};

/* ***********************************************************************
*  file element for player file. BEWARE: Changing it will ruin the file  *
*********************************************************************** */


struct char_file_u
{
	char name[20];
	char pwd[11]; 
	char title[80];
	char pre_title[80];
	byte sex;
	byte class;
	char last_connect_site[49];
	byte level[MAX_NUMBER_OF_CLASSES];
	time_t birth;  /* Time of birth of character     */
	int    played;    /* Number of secs played in total */

	int   race;
	ubyte weight;
	ubyte height;

	char poof_in[80];
	char poof_out[80];

	sh_int hometown;
	char description[240];
	bool talks[MAX_TOUNGE];

	sh_int load_room;            /* Which room to place char in  */

	struct char_ability_data abilities;
	struct char_point_data points;
	struct char_skill_data skills[MAX_SKILLS];
	struct affected_type affected[MAX_AFFECT];

	/* specials */

	int spells_to_learn;  
	int skills_to_learn;
	int alignment;     

	time_t last_logon;  /* Time (in secs) of last logon */
	ubyte act;          /* ACT Flags                    */
        int new_act;

	/* char data */
	sh_int apply_saving_throw[5];
	int conditions[MAX_CONDITIONS];

        sh_int sh_save_blah1;
        sh_int sh_save_blah2;
        sh_int sh_save_blah3;
        sh_int sh_save_blah4;

        int save_blah1;
        int save_blah2;
        int save_blah3;
        int save_blah4;

};



/* ***********************************************************************
*  file element for object file. BEWARE: Changing it will ruin the file  *
*********************************************************************** */

struct obj_cost { /* used in act.other.c:do_save as
		     well as in reception2.c */
	int total_cost;
	int no_carried;
	bool ok;
};

#if 0
#define MAX_OBJ_SAVE 99 /* Used in OBJ_FILE_U *DO*NOT*CHANGE* */

struct obj_file_elem 
{
	sh_int item_number;

	int value[4];
	int extra_flags;
	int weight;
	int timer;
	long bitvector;
	struct obj_affected_type affected[MAX_OBJ_AFFECT];
};

struct obj_file_u
{
	char owner[20];    /* Name of player                     */
	int gold_left;     /* Number of goldcoins left at owner  */
	int total_cost;    /* The cost for all items, per day    */
	long last_update;  /* Time in seconds, when last updated */

	struct obj_file_elem objects[MAX_OBJ_SAVE];
};
#endif

/* ***********************************************************
*  The following structures are related to descriptor_data   *
*********************************************************** */

struct txt_block
{
	char *text;
	struct txt_block *next;
};

struct txt_q
{
	struct txt_block *head;
	struct txt_block *tail;
};



/* modes of connectedness */

#define CON_PLYNG   0
#define CON_NME	    1
#define CON_NMECNF  2
#define CON_PWDNRM  3
#define CON_PWDGET  4
#define CON_PWDCNF  5
#define CON_QSEX    6
#define CON_RMOTD   7
#define CON_SLCT    8
#define CON_EXDSCR  9
#define CON_QCLASS  10
#define CON_LDEAD   11
#define CON_PWDNEW  12
#define CON_PWDNCNF 13
#define CON_WIZLOCK 14
#define CON_QRACE   15
#define CON_RACPAR  16

struct snoop_data
{
	struct char_data *snooping;	
		/* Who is this char snooping */
	struct char_data *snoop_by;
		/* And who is snooping on this char */
};

struct descriptor_data
{
	int descriptor;	            /* file descriptor for socket */
        long idle_time;		/* for ilde time duh */
	char host[50];                /* hostname                   */
        char usr_name[20];	      /* user name */
	char pwd[12];                 /* password                   */
	int pos;                      /* position in player-file    */
	int connected;                /* mode of 'connectedness'    */
	int wait;                     /* wait for how many loops    */
	char *showstr_head;		/* for paging through texts	*/
	char *showstr_point;		/*       -                    */
	char **str;                   /* for the modify-str system  */
	int max_str;                  /* -                          */
	int prompt_mode;              /* control of prompt-printing */
	char buf[MAX_STRING_LENGTH];  /* buffer for raw input       */
	char last_input[MAX_INPUT_LENGTH];/* the last input         */
	struct txt_q output;          /* q of strings to send       */
	struct txt_q input;           /* q of unprocessed input     */
	struct char_data *character;  /* linked to char             */
        struct char_data *original;   /* original char              */
	struct snoop_data snoop;      /* to snoop people.	         */
	struct descriptor_data *next; /* link to next descriptor    */
};

struct msg_type 
{
	char *attacker_msg;  /* message to attacker */
	char *victim_msg;    /* message to victim   */
	char *room_msg;      /* message to room     */
};

struct message_type
{
	struct msg_type die_msg;      /* messages when death            */
	struct msg_type miss_msg;     /* messages when miss             */
	struct msg_type hit_msg;      /* messages when hit              */
	struct msg_type sanctuary_msg;/* messages when hit on sanctuary */
	struct msg_type god_msg;      /* messages when hit on god       */
	struct message_type *next;/* to next messages of this kind.*/
};

struct message_list
{
	int a_type;               /* Attack type 						 */
	int number_of_attacks;	  /* How many attack messages to chose from. */
	struct message_type *msg; /* List of messages.				 */
};

struct dex_skill_type
{
	sh_int p_pocket;
	sh_int p_locks;
	sh_int traps;
	sh_int sneak;
	sh_int hide;
};

struct dex_app_type
{
	sh_int reaction;
	sh_int miss_att;
	sh_int defensive;
};

struct str_app_type
{
	sh_int tohit;    /* To Hit (THAC0) Bonus/Penalty        */
	sh_int todam;    /* Damage Bonus/Penalty                */
	sh_int carry_w;  /* Maximum weight that can be carrried */
	sh_int wield_w;  /* Maximum weight that can be wielded  */
};

struct wis_app_type
{
	byte bonus;       /* how many bonus skills a player can */
	                  /* practice pr. level                 */
};

struct int_app_type
{
	byte learn;       /* how many % a player learns a spell/skill */
};

struct con_app_type
{
	sh_int hitp;
	sh_int shock;
};

/************************************************************/

typedef void (*funcp)();

struct breather {
  int	vnum;
  int	cost;
  funcp	*breaths;
};

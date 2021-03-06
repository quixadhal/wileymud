#ifndef _PARSE_WILEY_H
#define _PARSE_WILEY_H

/*
 * Here are the definitions needed to handle the zone file.
 */

#define ZONE_RESET_NEVER	0
#define ZONE_RESET_PC		1
#define ZONE_RESET_ALWAYS	2

#define ZONE_CMD_MOBILE		'M'
#define ZONE_MOBILE		0
#define ZONE_MAX		1
#define ZONE_ROOM		2
#define ZONE_CMD_OBJECT		'O'
#define ZONE_OBJECT		0
#define ZONE_CMD_EQUIP		'E'
#define ZONE_POSITION		2
#define ZONE_CMD_PUT		'P'
#define ZONE_TARGET_OBJ		2
#define ZONE_CMD_DOOR		'D'
#define ZONE_DOOR_ROOM		0
#define ZONE_DOOR_EXIT		1
#define ZONE_DOOR_STATE		2
#define ZONE_CMD_GIVE		'G'
#define ZONE_CMD_REMOVE		'R'
#define ZONE_REMOVE_ROOM	0
#define ZONE_REMOVE_OBJ		1
#define ZONE_CMD_LEAD		'L'
#define ZONE_GROUP		1
#define ZONE_CMD_HATE		'H'
#define ZONE_HATE_TYPE		0
#define ZONE_HATE_VALUE		1
#define ZONE_CMD_END		'S'

/*
 * This is for the main world file.
 */

#define EXIT_OPEN			0
#define EXIT_DOOR			1
#define EXIT_NOPICK			2
#define EXIT_SECRET			3
#define EXIT_SECRET_NOPICK		4
#define EXIT_ALIAS_MAGIC		4
#define EXIT_OPEN_ALIAS			5		       /* contradiction? the docs say add 4... */
#define EXIT_DOOR_ALIAS			6
#define EXIT_NOPICK_ALIAS		7
#define EXIT_SECRET_ALIAS		8
#define EXIT_SECRET_NOPICK_ALIAS	9

#define EXIT_INVALID		-1			       /* exit was corrupted somehow */
#define EXIT_NORTH		0
#define EXIT_EAST		1
#define EXIT_SOUTH		2
#define EXIT_WEST		3
#define EXIT_UP			4
#define EXIT_DOWN		5

#define EXIT_OK			0
#define EXIT_NON_EUCLIDEAN	1
#define EXIT_ONE_WAY		2
#define EXIT_DESCRIPTION_ONLY	3
#define EXIT_NO_TARGET		4
#define EXIT_UNKNOWN		5

#define ROOM_DARK		0x00000001
#define ROOM_DEATH		0x00000002
#define ROOM_NOMOB		0x00000004
#define ROOM_INDOORS		0x00000008
#define ROOM_NOATTACK		0x00000010
#define ROOM_NOSTEAL		0x00000020
#define ROOM_NOSUMMON		0x00000040
#define ROOM_NOMAGIC		0x00000080
#define ROOM_UNUSED		0x00000100
#define ROOM_PRIVATE		0x00000200
#define ROOM_SOUND		0x00000400

#define SECT_TELEPORT		-1
#define SECT_INDOORS		0
#define SECT_CITY		1
#define SECT_FIELD		2
#define SECT_FOREST		3
#define SECT_HILLS		4
#define SECT_MOUNTAIN		5
#define SECT_WATER_SWIM		6
#define SECT_WATER_NOSWIM	7
#define SECT_AIR		8
#define SECT_UNDERWATER		9

#define SPECIAL_ROOM    1
#define SPECIAL_OBJECT  2
#define SPECIAL_MOB     3

#ifndef _PARSE_WILEY_C
extern const int                        SectorCosts[];
extern const int                        RevDir[];
extern const int       SpecialCount;
extern const special   Specials[];
#endif

/*
 * And these are the shopkeeper definitions.
 */

#define SHOP_SELLCOUNT		5
#define SHOP_TRADECOUNT		5
#define SHOP_MESSAGECOUNT	7

#define WEAR_LIGHT	0
#define WEAR_FINGER_R	1
#define WEAR_FINGER_L	2
#define WEAR_NECK_1	3
#define WEAR_NECK_2	4
#define WEAR_BODY	5
#define WEAR_HEAD	6
#define WEAR_LEGS	7
#define WEAR_FEET	8
#define WEAR_HANDS	9
#define WEAR_ARMS	10
#define WEAR_SHIELD	11
#define WEAR_ABOUT	12
#define WEAR_WAIST	13
#define WEAR_WRIST_R	14
#define WEAR_WRIST_L	15
#define WIELD		16
#define HOLD		17
#define TWOH		18

#define DOOR_OPEN	0
#define DOOR_CLOSED	1
#define DOOR_LOCKED	2

#define SEX_NEUTER	0
#define SEX_MALE	1
#define SEX_FEMALE	2

#define RACE_HALFBREED	0
#define RACE_HUMAN	1
#define RACE_ELVEN	2
#define RACE_DWARF	3
#define RACE_HALFLING	4
#define RACE_GNOME	5
#define RACE_REPTILE	6
#define RACE_SPECIAL	7
#define RACE_LYCANTH	8
#define RACE_DRAGON	9
#define RACE_UNDEAD	10
#define RACE_ORC	11
#define RACE_INSECT	12
#define RACE_ARACHNID	13
#define RACE_DINOSAUR	14
#define RACE_FISH	15
#define RACE_BIRD	16
#define RACE_GIANT	17
#define RACE_PREDATOR	18
#define RACE_PARASITE	19
#define RACE_SLIME	20
#define RACE_DEMON	21
#define RACE_SNAKE	22
#define RACE_HERBIV	23
#define RACE_TREE	24
#define RACE_VEGGIE	25
#define RACE_ELEMENT	26
#define RACE_PLANAR	27
#define RACE_DEVIL	28
#define RACE_GHOST	29
#define RACE_GOBLIN	30
#define RACE_TROLL	31
#define RACE_VEGMAN	32
#define RACE_MFLAYER	33
#define RACE_PRIMATE	34
#define RACE_ANIMAL	35
#define RACE_FAERY	36
#define RACE_PLANT	37

#define PULSE_PER_SECOND	4
/* #define PULSE_UPDATE	(75 * 5) */
#define PULSE_UPDATE	(70 * PULSE_PER_SECOND)
#define PULSE_VARIABLE	(30 * PULSE_PER_SECOND)

#define PULSE_VIOLENCE    6				       /* combat rounds */
#define PULSE_RIVER      10				       /* These two must be 10 to make the timings */
#define PULSE_TELEPORT   10				       /* in the world files come out right. */
#define PULSE_NATURE     19				       /* This is checks for falling/drowning */
#define PULSE_MOBILE     25				       /* mob movement/specials/etc */
#define PULSE_SOUND      33				       /* room sounds */
#define PULSE_ZONE      239				       /* zone updates */
#define PULSE_REBOOT    599				       /* reboot checking */
#define PULSE_SHOUT    1203				       /* replacement for the astral traveller */
#define PULSE_DUMP     3601				       /* dump player list */
#define PULSE_MAX    999999

#define WAIT_SEC       4
#define WAIT_ROUND     4

#define ALIGN_REALLY_VILE -1000
#define ALIGN_VILE -900
#define ALIGN_VERY_EVIL -700
#define ALIGN_EVIL -350
#define ALIGN_WICKED -100
#define ALIGN_NEUTRAL 0
#define ALIGN_NICE 100
#define ALIGN_GOOD 350
#define ALIGN_VERY_GOOD 700
#define ALIGN_HOLY 900
#define ALIGN_REALLY_HOLY 1000

#define FIRE_DAMAGE 1
#define COLD_DAMAGE 2
#define ELEC_DAMAGE 3
#define BLOW_DAMAGE 4
#define ACID_DAMAGE 5

#define MAGE_LEVEL_IND    0
#define CLERIC_LEVEL_IND  1
#define WARRIOR_LEVEL_IND 2
#define THIEF_LEVEL_IND   3
#define RANGER_LEVEL_IND  4
#define DRUID_LEVEL_IND   5

#define CLASS_MAGE	1
#define CLASS_CLERIC	2
#define CLASS_WARRIOR	4
#define CLASS_THIEF	8
#define CLASS_RANGER	16
#define CLASS_DRUID	32

#define MAX_CLASS	5

/* 'class' for PC's */
#define CLASS_MAGIC_USER  1
#define CLASS_CLERIC      2
#define CLASS_WARRIOR     4
#define CLASS_THIEF       8
#define CLASS_RANGER	  16
#define CLASS_DRUID	  32
#define CLASS_ALL ( CLASS_MAGIC_USER | CLASS_CLERIC | CLASS_WARRIOR | CLASS_THIEF | CLASS_RANGER | CLASS_DRUID )
#define CLASS_WIZARD ( CLASS_MAGIC_USER | CLASS_RANGER )
#define CLASS_PRIEST ( CLASS_CLERIC | CLASS_DRUID | CLASS_RANGER )
#define CLASS_MAGICAL ( CLASS_MAGIC_USER | CLASS_CLERIC | CLASS_RANGER | CLASS_DRUID )
#define CLASS_FIGHTER ( CLASS_WARRIOR | CLASS_RANGER )
#define CLASS_SNEAK ( CLASS_THIEF | CLASS_RANGER )

#define HATE_SEX	1
#define HATE_RACE	2
#define HATE_CHAR	4
#define HATE_CLASS	8
#define HATE_EVIL	16
#define HATE_GOOD	32
#define HATE_VNUM	64
#define HATE_RICH	128

#define FEAR_SEX   1
#define FEAR_RACE  2
#define FEAR_CHAR  4
#define FEAR_CLASS 8
#define FEAR_EVIL  16
#define FEAR_GOOD  32
#define FEAR_VNUM  64
#define FEAR_RICH  128

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

#define MAX_IMM 18

/* Here are the object flags */

/* For 'type_flag' */

#define ITEM_LIGHT      1
#define ITEM_SCROLL     2
#define ITEM_WAND       3
#define ITEM_STAFF      4
#define ITEM_WEAPON     5				       /* These are not correct, flipped for sanity */
#define ITEM_FIREWEAPON 6
#define ITEM_MISSILE    7				       /* These are not correct, flipped for sanity */
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

#define MAX_WEAR 16

/* UNUSED, CHECKS ONLY FOR ITEM_LIGHT #define ITEM_LIGHT_SOURCE  65536 */

/* Bitvector for 'extra_flags' */

#define ITEM_GLOW            1
#define ITEM_HUM             2
#define ITEM_METAL           4				       /* undefined...  */
#define ITEM_MINERAL         8				       /* undefined? */
#define ITEM_ORGANIC        16				       /* undefined? */
#define ITEM_INVISIBLE      32
#define ITEM_MAGIC          64
#define ITEM_NODROP        128
#define ITEM_BLESS         256
#define ITEM_ANTI_GOOD     512				       /* not usable by good people */
#define ITEM_ANTI_EVIL    1024				       /* not usable by evil people */
#define ITEM_ANTI_NEUTRAL 2048				       /* not usable by neutral people */
#define ITEM_ANTI_CLERIC  4096
#define ITEM_ANTI_MAGE    8192
#define ITEM_ANTI_THIEF   16384
#define ITEM_ANTI_FIGHTER 32768
#define ITEM_ANTI_RANGER  65536
#define ITEM_PARISH	  131072

#define MAX_ITEM_FLAGS 18

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

#define MAX_CONT_CLOSE 4

/* Bitvector for 'affected_by' */
#define AFF_BLIND             0x00000001
#define AFF_INVISIBLE         0x00000002
#define AFF_DETECT_EVIL       0x00000004
#define AFF_DETECT_INVISIBLE  0x00000008
#define AFF_DETECT_MAGIC      0x00000010
#define AFF_SENSE_LIFE        0x00000020
#define AFF_SILENCED          0x00000040		       /* this may have to go */
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
#define AFF_DRUG_FREE         0x00040000		       /* i.e. can't be stoned */
#define AFF_SNEAK             0x00080000
#define AFF_HIDE              0x00100000
#define AFF_FEAR              0x00200000		       /* this may have to go */
#define AFF_CHARM             0x00400000
#define AFF_FOLLOW            0x00800000
#define AFF_UNDEF_1           0x01000000		       /* saved objects?? */
#define AFF_TRUE_SIGHT        0x02000000
#define AFF_SCRYING           0x04000000		       /* seeing other rooms */
#define AFF_FIRESHIELD        0x08000000
#define AFF_RIDE              0x10000000
#define AFF_UNDEF_6           0x20000000
#define AFF_UNDEF_7           0x40000000
#define AFF_UNDEF_8           0x80000000

#define MAX_AFFECTED_BY 32
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
#define ACT_SPEC       (1<<0)				       /* special routine to be called if exist */
#define ACT_SENTINEL   (1<<1)				       /* this mobile not to be moved */
#define ACT_SCAVENGER  (1<<2)				       /* pick up stuff lying around */
#define ACT_ISNPC      (1<<3)				       /* This bit is set for use with IS_NPC() */
#define ACT_NICE_THIEF (1<<4)				       /* Set if a thief should NOT be killed */
#define ACT_AGGRESSIVE (1<<5)				       /* Set if automatic attack on NPC's */
#define ACT_STAY_ZONE  (1<<6)				       /* MOB Must stay inside its own zone */
#define ACT_WIMPY      (1<<7)				       /* MOB Will flee when injured, and if */
			       /*
			        * aggressive only attack sleeping players 
			        */
#define ACT_ANNOYING   (1<<8)				       /* MOB is so utterly irritating that other */
			       /*
			        * monsters will attack it...  
			        */
#define ACT_HATEFUL    (1<<9)				       /* MOB will attack a PC or NPC matching a */
			       /*
			        * specified name 
			        */
#define ACT_AFRAID    (1<<10)				       /* MOB is afraid of a certain PC or NPC, */
			       /*
			        * and will always run away ....  
			        */
#define ACT_IMMORTAL  (1<<11)				       /* MOB is a natural event, can't be kiled */
#define ACT_HUNTING   (1<<12)				       /* MOB is hunting someone */
#define ACT_DEADLY    (1<<13)				       /* MOB has deadly poison */
#define ACT_POLYSELF  (1<<14)				       /* MOB is a polymorphed person */
#define ACT_POLYOTHER (1<<15)				       /* MOB is a polymorphed person */
#define ACT_GUARDIAN  (1<<16)				       /* MOB will guard master */
#define ACT_USE_ITEM  (1<<17)				       /* Will use an item when picked up */
#define ACT_FIGHTER_MOVES (1<<18)			       /* mob will attack like a fighter */
#define ACT_FOOD_PROVIDE (1<<19)
#define ACT_PROTECTOR (1<<20)				       /* mob will come to aid those that call */
				 /*
				  * this is inter zonal only 
				  */
#define ACT_MOUNT     (1<<21)				       /* mobile can act as a mount */
#define ACT_SWITCH    (1<<22)				       /* we are a wizard switch to a mob */

#define MAX_ACT 23

 /**/
#define SPELL_RESERVED_DBC            0			       /* SKILL NUMBER ZERO */
#define SPELL_ARMOR                   1
#define SPELL_TELEPORT                2
#define SPELL_BLESS                   3
#define SPELL_BLINDNESS               4
#define SPELL_BURNING_HANDS           5
#define SPELL_CALL_LIGHTNING          6
#define SPELL_CHARM_PERSON            7
#define SPELL_CHILL_TOUCH             8
#define SPELL_CLONE                   9
#define SPELL_COLOUR_SPRAY           10
#define SPELL_CONTROL_WEATHER        11
#define SPELL_CREATE_FOOD            12
#define SPELL_CREATE_WATER           13
#define SPELL_CURE_BLIND             14
#define SPELL_CURE_CRITIC            15
#define SPELL_CURE_LIGHT             16
#define SPELL_CURSE                  17
#define SPELL_DETECT_EVIL            18
#define SPELL_DETECT_INVISIBLE       19
#define SPELL_DETECT_MAGIC           20
#define SPELL_DETECT_POISON          21
#define SPELL_DISPEL_EVIL            22
#define SPELL_EARTHQUAKE             23
#define SPELL_ENCHANT_WEAPON         24
#define SPELL_ENERGY_DRAIN           25
#define SPELL_FIREBALL               26
#define SPELL_HARM                   27
#define SPELL_HEAL                   28
#define SPELL_INVISIBLE              29
#define SPELL_LIGHTNING_BOLT         30
#define SPELL_LOCATE_OBJECT          31
#define SPELL_MAGIC_MISSILE          32
#define SPELL_POISON                 33
#define SPELL_PROTECT_FROM_EVIL      34
#define SPELL_REMOVE_CURSE           35
#define SPELL_SANCTUARY              36
#define SPELL_SHOCKING_GRASP         37
#define SPELL_SLEEP                  38
#define SPELL_STRENGTH               39
#define SPELL_SUMMON                 40
#define SPELL_VENTRILOQUATE          41
#define SPELL_WORD_OF_RECALL         42
#define SPELL_REMOVE_POISON          43
#define SPELL_SENSE_LIFE             44
/* Skills 45 - 52 already in use! */
/* Skills 45 - 52 already in use! */
/* Skills 45 - 52 already in use! */
/* Skills 45 - 52 already in use! */
/* Skills 45 - 52 already in use! */
/* Skills 45 - 52 already in use! */
/* Skills 45 - 52 already in use! */
/* Skills 45 - 52 already in use! */
#define SPELL_IDENTIFY               53
#define SPELL_INFRAVISION            54
#define SPELL_CAUSE_LIGHT            55
#define SPELL_CAUSE_CRITICAL         56
#define SPELL_FLAMESTRIKE            57
#define SPELL_DISPEL_GOOD            58
#define SPELL_WEAKNESS               59
#define SPELL_DISPEL_MAGIC           60
#define SPELL_KNOCK                  61
#define SPELL_KNOW_ALIGNMENT         62
#define SPELL_ANIMATE_DEAD           63
#define SPELL_PARALYSIS              64
#define SPELL_REMOVE_PARALYSIS       65
#define SPELL_FEAR                   66
#define SPELL_ACID_BLAST             67
#define SPELL_WATER_BREATH           68
#define SPELL_FLY                    69
#define SPELL_CONE_OF_COLD           70
#define SPELL_METEOR_SWARM           71
#define SPELL_ICE_STORM              72
#define SPELL_SHIELD                 73
#define SPELL_MON_SUM_1              74
#define SPELL_MON_SUM_2              75
#define SPELL_MON_SUM_3              76
#define SPELL_MON_SUM_4              77
#define SPELL_MON_SUM_5              78
#define SPELL_MON_SUM_6              79
#define SPELL_MON_SUM_7              80
#define SPELL_FIRESHIELD             81
#define SPELL_CHARM_MONSTER          82
#define SPELL_CURE_SERIOUS           83
#define SPELL_CAUSE_SERIOUS          84
#define SPELL_REFRESH                85
#define SPELL_SECOND_WIND            86
#define SPELL_TURN                   87
#define SPELL_SUCCOR                 88
#define SPELL_LIGHT                  89
#define SPELL_CONT_LIGHT             90
#define SPELL_CALM                   91
#define SPELL_STONE_SKIN             92
#define SPELL_CONJURE_ELEMENTAL      93
#define SPELL_TRUE_SIGHT             94
#define SPELL_MINOR_CREATE           95
#define SPELL_FAERIE_FIRE            96
#define SPELL_FAERIE_FOG             97
#define SPELL_CACAODEMON             98
#define SPELL_POLY_SELF              99
#define SPELL_MANA                  100
#define SPELL_ASTRAL_WALK           101
#define SPELL_FLY_GROUP             102
#define SPELL_AID                   103
#define SPELL_SHELTER               104
#define SPELL_DRAGON_BREATH         105
#define SPELL_GOODBERRY             106
#define SPELL_VISIONS               107
#define SPELL_MAJOR_TRACK           108
#define SPELL_GOLEM                 109
#define SPELL_FAMILIAR              110
#define SPELL_CHANGESTAFF           111
#define SPELL_HOLY_WORD             112
#define SPELL_UNHOLY_WORD           113
#define SPELL_PWORD_KILL            114
#define SPELL_PWORD_BLIND           115
#define SPELL_CHAIN_LIGHTNING       116
#define SPELL_SCARE                 117
#define SPELL_COMMAND               119
#define SPELL_CHANGE_FORM           120			       /* druid... */
#define SPELL_FEEBLEMIND            121
#define SPELL_SHILLELAGH            122
/* 123? */
#define SPELL_FLAME_BLADE           124
#define SPELL_ANIMAL_GROWTH         125
#define SPELL_INSECT_GROWTH         126
#define SPELL_CREEPING_DEATH        127
#define SPELL_COMMUNE               128			       /* whatzone */
#define SPELL_ANIMAL_SUM_1          129
#define SPELL_ANIMAL_SUM_2          130
#define SPELL_ANIMAL_SUM_3          131
#define SPELL_FIRE_SERVANT          132
#define SPELL_EARTH_SERVANT         133
#define SPELL_WATER_SERVANT         134
#define SPELL_WIND_SERVANT          135
#define SPELL_REINCARNATE           136
#define SPELL_CHARM_VEGGIE          137
#define SPELL_VEGGIE_GROWTH         138
#define SPELL_TREE                  139
#define SPELL_ANIMATE_ROCK          140
#define SPELL_TREE_TRAVEL           141
#define SPELL_TRAVELLING            142			       /* faster move outdoors */
#define SPELL_ANIMAL_FRIENDSHIP     143
#define SPELL_INVIS_TO_ANIMALS      144
#define SPELL_SLOW_POISON           145
#define SPELL_ENTANGLE              146
#define SPELL_SNARE                 147
#define SPELL_GUST_OF_WIND          148
#define SPELL_BARKSKIN              149
#define SPELL_SUNRAY                150
#define SPELL_WARP_WEAPON           151
#define SPELL_HEAT_STUFF            152
#define SPELL_FIND_TRAPS            153
#define SPELL_FIRESTORM             154
#define SPELL_HASTE                 155			       /* other */
#define SPELL_SLOW                  156
#define SPELL_DUST_DEVIL            157
#define SPELL_KNOW_MONSTER          158
#define SPELL_TRANSPORT_VIA_PLANT   159
#define SPELL_SPEAK_WITH_PLANT      160
#define SPELL_SILENCE               161
#define SPELL_SENDING               162
#define SPELL_TELEPORT_WO_ERROR     163
#define SPELL_PORTAL                164
#define SPELL_DRAGON_RIDE           165
#define SPELL_MOUNT                 166
#define SPELL_GREEN_SLIME            199
#define SPELL_GEYSER                 200
#define SPELL_FIRE_BREATH            201
#define SPELL_GAS_BREATH             202
#define SPELL_FROST_BREATH           203
#define SPELL_ACID_BREATH            204
#define SPELL_LIGHTNING_BREATH       205
#define SKILL_SNEAK             45			       /* r,t */
#define SKILL_HIDE              46			       /* r,t */
#define SKILL_STEAL             47			       /* t */
#define SKILL_BACKSTAB          48			       /* t */
#define SKILL_PICK_LOCK         49			       /* t */
#define SKILL_KICK              50			       /* f */
#define SKILL_BASH              51			       /* f */
#define SKILL_RESCUE            52			       /* f,r */
#define SKILL_DOOR_BASH	 	149			       /* f,r */
#define SKILL_READ_MAGIC 	150			       /* f,m,c,t,r */
#define SKILL_SCRIBE		151			       /* m,c */
#define SKILL_BREW		152			       /* m,c */
#define SKILL_PUNCH		153			       /* f,c,r */
#define SKILL_TWO_HANDED	154			       /* f,m,c,t,r */
#define SKILL_TWO_WEAPON	155			       /* NONE */
#define SKILL_BANDAGE		156			       /* f,m,c,t,r */
#define SKILL_SEARCH		157			       /* t,r */
#define SKILL_SWIMMING		158			       /* f,m,c,t,r */
#define SKILL_ENDURANCE		159			       /* f,m,c,r,t */
#define SKILL_BARE_HAND		160			       /* f,m,c,t,r */
#define SKILL_BLIND_FIGHTING	161			       /* f,c,m,r,t */
#define SKILL_PARRY		162			       /* f,r,t NOT PUT IN */
#define SKILL_APRAISE		163			       /* f,m,c,t,r */
#define SKILL_SPEC_SMITE	164			       /* f */
#define SKILL_SPEC_STAB		165
#define SKILL_SPEC_WHIP		166
#define SKILL_SPEC_SLASH	167
#define SKILL_SPEC_SMASH	168
#define SKILL_SPEC_CLEAVE	169
#define SKILL_SPEC_CRUSH	170
#define SKILL_SPEC_BLUDGE	171
#define SKILL_SPEC_PIERCE	172			       /* f */
#define SKILL_PEER		173			       /* t,r */
#define SKILL_DETECT_NOISE	174			       /* t,r */
#define SKILL_DODGE		175			       /* m,c,t,r NOT PUT IN */
#define SKILL_BARTER		176			       /* m,c,t */
#define SKILL_KNOCK_OUT		177			       /* m,c,t NOT PUT IN */
#define SKILL_SPELLCRAFT	178			       /* m,c */
#define SKILL_MEDITATION	179			       /* m,c */
#define SKILL_TRACK             180			       /* r,t */
#define SKILL_FIND_TRAP         181			       /* t */
#define SKILL_DISARM_TRAP       182			       /* t */
#define SKILL_DISARM            183			       /* f,r */
#define SKILL_BASH_W_SHIELD     184			       /* not a useable skill, but included with bash */
#define SKILL_RIDE		185
#define TYPE_HIT                     206
#define TYPE_BLUDGEON                207
#define TYPE_PIERCE                  208
#define TYPE_SLASH                   209
#define TYPE_WHIP                    210		       /* EXAMPLE */
#define TYPE_CLAW                    211		       /* NO MESSAGES WRITTEN YET! */
#define TYPE_BITE                    212		       /* NO MESSAGES WRITTEN YET! */
#define TYPE_STING                   213		       /* NO MESSAGES WRITTEN YET! */
#define TYPE_CRUSH                   214		       /* NO MESSAGES WRITTEN YET! */
#define TYPE_CLEAVE                  215
#define TYPE_STAB                    216
#define TYPE_SMASH                   217
#define TYPE_SMITE                   218
#define TYPE_SUFFERING               220
#define TYPE_HUNGER                  221

zones                                  *load_zones(char *infile);
char                                   *zone_reset_name(int Reset);
int                                     remap_zone_vnum(zones *Zones, int VNum);
char                                   *zone_name(zones *Zones, int VNum);

char                                   *exit_name(int Direction);
char                                   *exit_name_lower(int Direction);
int                                     remap_room_vnum(rooms *Rooms, int VNum);
char                                   *room_name(rooms *Rooms, int VNum);
void                                    fix_exit_vnums(rooms *Rooms);
void                                    check_duplicate_rooms(rooms *Rooms);
void                                    verify_exits(rooms *Rooms);
void                                    fix_zone_ids(zones *Zones, rooms *Rooms);
rooms                                  *load_rooms(char *infile, zones *Zones);
char                                   *room_flag_name(int Flag);
char                                   *sector_name(int Sector);
char                                   *exittype_name(int Type);
char                                   *room_flags(int Flags);

shops                                  *load_shops(char *infile);
char                                   *shop_attitude_name(int Attitude);
char                                   *shop_immortal_name(int Immortal);
int                                     remap_shop_vnum(shops *Shops, int VNum);

char                                   *equip_name(int Position);
char                                   *doorstate_name(int State);
char                                   *sex_name(int State);
char                                   *race_name(int Race);

char                                   *alignment_name(int Alignment);
char                                   *damage_type_name(int Type);
char                                   *class_name(int Class); /* returns ALL classes of a character */
char                                   *hate_name(int Type, int Value);

#define fear_name(x,y) hate_name((x),(y))
char                                   *immunity_name(int Imms);
char                                   *item_type_name(int Type);
char                                   *item_equip_name(int Position);
char                                   *item_flag_name(int Type);
char                                   *liquid_name(int Type);
char                                   *container_closeable(int Value);
char                                   *affected_by_name(int Flag);
char                                   *apply_name(int Flag);
char                                   *position_name(int Flag);
char                                   *act_name(int Flag);
char                                   *item_wear_name(int Type);

int                                     remap_obj_vnum(objects *Objects, int VNum);
char                                   *obj_name(objects *Objects, int VNum);
void                                    check_duplicate_objs(objects *Objects);
void                                    set_obj_zones(zones *Zones, objects *Objects);
objects                                *load_objects(char *infile, zones *Zones);
char                                   *spell_name(int Spell);

#define skill_name(x) spell_name(x)
#define MAX_SKILLS 222
#define ASSIGN_SPELL( num, a, b, name, c, d, e, f, g, h, i, j, k, l, m, n, o, p ) { spell[(num)]= my_strdup(name); }

char                                   *damage_name(int Type);

int                                     remap_mob_vnum(mobs *Mobs, int VNum);
char                                   *mob_name(mobs *Mobs, int VNum);
void                                    check_duplicate_mobs(mobs *Mobs);
void                                    set_mob_zones(zones *Zones, mobs *Mobs);
mobs                                   *load_mobs(char *infile, zones *Zones);

void                                    check_room_zone_mismatch(zones *Zones, rooms *Rooms);
void                                    check_object_zone_mismatch(zones *Zones,
								   objects *Objects);
void                                    check_mob_zone_mismatch(zones *Zones, mobs *Mobs);

#endif

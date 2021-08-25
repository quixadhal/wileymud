#ifndef _DUMP_AFK_H
#define _DUMP_AFK_H

#include "smaug_common.h"

/*
 * This file holds defined values for the diku target.
 * They are used in converting the original WileyMUD files into
 * the target diku format.
 */

#define AFK_AREA_VERSION_WRITE 18
#define AFK_AREA_LEVEL_LIMIT_LOWER 0
#define AFK_AREA_LEVEL_LIMIT_UPPER 115
#define AFK_CLIMATE_TEMP_NORMAL 2
#define AFK_CLIMATE_PRECIP_NORMAL 2
#define AFK_CLIMATE_WIND_NORMAL 2

/*
 * Room flags.           Holy cow!  Talked about stripped away..
 * Used in #ROOMS.       Those merc guys know how to strip code down.
 *			 Lets put it all back... ;)
 */
/* Roomflags converted to Extended BV - Samson 8-11-98 */
/* NOTE: If this list grows to 65, raise BFS_MARK in track.c - Samson */
/* Don't forget to add the flag to build.c!!! */
/* Current # of flags: 42 */
typedef enum
{
    AFK_ROOM_DARK,
    AFK_ROOM_DEATH,
    AFK_ROOM_NO_MOB,
    AFK_ROOM_INDOORS,
    AFK_ROOM_SAFE,
    AFK_ROOM_NOCAMP,
    AFK_ROOM_NO_SUMMON,
    AFK_ROOM_NO_MAGIC,
    AFK_ROOM_TUNNEL,
    AFK_ROOM_PRIVATE,
    AFK_ROOM_SILENCE,
    AFK_ROOM_NOSUPPLICATE,
    AFK_ROOM_ARENA,
    AFK_ROOM_NOMISSILE,
    AFK_ROOM_NO_RECALL,
    AFK_ROOM_NO_PORTAL,
    AFK_ROOM_NO_ASTRAL,
    AFK_ROOM_NODROP,
    AFK_ROOM_CLANSTOREROOM,
    AFK_ROOM_TELEPORT,
    AFK_ROOM_TELESHOWDESC,
    AFK_ROOM_NOFLOOR,
    AFK_ROOM_SOLITARY,
    AFK_ROOM_PET_SHOP,
    AFK_ROOM_DONATION,
    AFK_ROOM_NODROPALL,
    AFK_ROOM_LOGSPEECH,
    AFK_ROOM_PROTOTYPE,
    AFK_ROOM_NOTELEPORT,
    AFK_ROOM_NOSCRY,
    AFK_ROOM_CAVE,
    AFK_ROOM_CAVERN,
    AFK_ROOM_NOBEACON,
    AFK_ROOM_AUCTION,
    AFK_ROOM_MAP,
    AFK_ROOM_FORGE,
    AFK_ROOM_GUILDINN,
    AFK_ROOM_DELETED,
    AFK_ROOM_ISOLATED,
    AFK_ROOM_WATCHTOWER,
    AFK_ROOM_NOQUIT,
    AFK_ROOM_TELENOFLY,
    AFK_ROOM_MAX
} afk_room_flags;

/*
 * Exit flags.			EX_RES# are reserved for use by the
 * Used in #ROOMS.		SMAUG development team ( No RES flags left, sorry. - Samson )
 */
/* Converted to Extended BV - Samson 7-23-00 */
/* Don't forget to add the flag to build.c!!! */
/* Current # of flags: 35 */
typedef enum
{
    AFK_EX_ISDOOR,
    AFK_EX_CLOSED,
    AFK_EX_LOCKED,
    AFK_EX_SECRET,
    AFK_EX_SWIM,
    AFK_EX_PICKPROOF,
    AFK_EX_FLY,
    AFK_EX_CLIMB,
    AFK_EX_DIG,
    AFK_EX_EATKEY,
    AFK_EX_NOPASSDOOR,
    AFK_EX_HIDDEN,
    AFK_EX_PASSAGE,
    AFK_EX_PORTAL,
    AFK_EX_OVERLAND,
    AFK_EX_ASLIT,
    AFK_EX_xCLIMB,
    AFK_EX_xENTER,
    AFK_EX_xLEAVE,
    AFK_EX_xAUTO,
    AFK_EX_NOFLEE,
    AFK_EX_xSEARCHABLE,
    AFK_EX_BASHED,
    AFK_EX_BASHPROOF,
    AFK_EX_NOMOB,
    AFK_EX_WINDOW,
    AFK_EX_xLOOK,
    AFK_EX_ISBOLT,
    AFK_EX_BOLTED,
    AFK_EX_FORTIFIED,
    AFK_EX_HEAVY,
    AFK_EX_MEDIUM,
    AFK_EX_LIGHT,
    AFK_EX_CRUMBLING,
    AFK_EX_DESTROYED,
    MAX_EXFLAG
} afk_exit_flags;

/*
 * Sector types.
 * Used in #ROOMS and on Overland maps.
 */
/* Current number of types: 34 */
typedef enum
{
    AFK_SECT_INDOORS,
    AFK_SECT_CITY,
    AFK_SECT_FIELD,
    AFK_SECT_FOREST,
    AFK_SECT_HILLS,
    AFK_SECT_MOUNTAIN,
    AFK_SECT_WATER_SWIM,
    AFK_SECT_WATER_NOSWIM,
    AFK_SECT_AIR,
    AFK_SECT_UNDERWATER,
    AFK_SECT_DESERT,
    AFK_SECT_RIVER,
    AFK_SECT_OCEANFLOOR,
    AFK_SECT_UNDERGROUND,
    AFK_SECT_JUNGLE,
    AFK_SECT_SWAMP,
    AFK_SECT_TUNDRA,
    AFK_SECT_ICE,
    AFK_SECT_OCEAN,
    AFK_SECT_LAVA,
    AFK_SECT_SHORE,
    AFK_SECT_TREE,
    AFK_SECT_STONE,
    AFK_SECT_QUICKSAND,
    AFK_SECT_WALL,
    AFK_SECT_GLACIER,
    AFK_SECT_EXIT,
    AFK_SECT_TRAIL,
    AFK_SECT_BLANDS,
    AFK_SECT_GRASSLAND,
    AFK_SECT_SCRUB,
    AFK_SECT_BARREN,
    AFK_SECT_BRIDGE,
    AFK_SECT_ROAD,
    AFK_SECT_LANDING,
    AFK_SECT_MAX
} afk_sector_types;

/*
 * Item types.
 * Used in #OBJECTS.
 */
/* Don't forget to add the flag to build.c!!! */
/* also don't forget to add new item types to the find_oftype function in afk.c */
/* Current # of types: 68 */
typedef enum
{
    AFK_ITEM_NONE,
    AFK_ITEM_LIGHT,
    AFK_ITEM_SCROLL,
    AFK_ITEM_WAND,
    AFK_ITEM_STAFF,
    AFK_ITEM_WEAPON,
    AFK_ITEM_unused1,
    AFK_ITEM_unused2,
    AFK_ITEM_TREASURE,
    AFK_ITEM_ARMOR,
    AFK_ITEM_POTION,
    AFK_ITEM_unused3,
    AFK_ITEM_FURNITURE,
    AFK_ITEM_TRASH,
    AFK_ITEM_unused4,
    AFK_ITEM_CONTAINER,
    AFK_ITEM_unused5,
    AFK_ITEM_DRINK_CON,
    AFK_ITEM_KEY,
    AFK_ITEM_FOOD,
    AFK_ITEM_MONEY,
    AFK_ITEM_PEN,
    AFK_ITEM_BOAT,
    AFK_ITEM_CORPSE_NPC,
    AFK_ITEM_CORPSE_PC,
    AFK_ITEM_FOUNTAIN,
    AFK_ITEM_PILL,
    AFK_ITEM_BLOOD,
    AFK_ITEM_BLOODSTAIN,
    AFK_ITEM_SCRAPS,
    AFK_ITEM_PIPE,
    AFK_ITEM_HERB_CON,
    AFK_ITEM_HERB,
    AFK_ITEM_INCENSE,
    AFK_ITEM_FIRE,
    AFK_ITEM_BOOK,
    AFK_ITEM_SWITCH,
    AFK_ITEM_LEVER,
    AFK_ITEM_PULLCHAIN,
    AFK_ITEM_BUTTON,
    AFK_ITEM_DIAL,
    AFK_ITEM_RUNE,
    AFK_ITEM_RUNEPOUCH,
    AFK_ITEM_MATCH,
    AFK_ITEM_TRAP,
    AFK_ITEM_MAP,
    AFK_ITEM_PORTAL,
    AFK_ITEM_PAPER,
    AFK_ITEM_TINDER,
    AFK_ITEM_LOCKPICK,
    AFK_ITEM_SPIKE,
    AFK_ITEM_DISEASE,
    AFK_ITEM_OIL,
    AFK_ITEM_FUEL,
    AFK_ITEM_PIECE,
    AFK_ITEM_TREE,
    AFK_ITEM_MISSILE_WEAPON,
    AFK_ITEM_PROJECTILE,
    AFK_ITEM_QUIVER,
    AFK_ITEM_SHOVEL,
    AFK_ITEM_SALVE,
    AFK_ITEM_COOK,
    AFK_ITEM_KEYRING,
    AFK_ITEM_ODOR,
    AFK_ITEM_CAMPGEAR,
    AFK_ITEM_DRINK_MIX,
    AFK_ITEM_INSTRUMENT,
    AFK_ITEM_ORE,
    MAX_AFK_ITEM_TYPE
} afk_item_types;

/*
 * Extra flags.
 * Used in #OBJECTS. Rearranged for better compatibility with Shard areas - Samson
 */
/* Don't forget to add the flag to o_flags in build.c and handler.c!!! */
/* Current # of flags: 65 */
typedef enum
{
    AFK_ITEM_GLOW,
    AFK_ITEM_HUM,
    AFK_ITEM_METAL,
    AFK_ITEM_MINERAL,
    AFK_ITEM_ORGANIC,
    AFK_ITEM_INVIS,
    AFK_ITEM_MAGIC,
    AFK_ITEM_NODROP,
    AFK_ITEM_BLESS,
    AFK_ITEM_ANTI_GOOD,
    AFK_ITEM_ANTI_EVIL,
    AFK_ITEM_ANTI_NEUTRAL,
    AFK_ITEM_ANTI_CLERIC,
    AFK_ITEM_ANTI_MAGE,
    AFK_ITEM_ANTI_ROGUE,
    AFK_ITEM_ANTI_WARRIOR,
    AFK_ITEM_INVENTORY,
    AFK_ITEM_NOREMOVE,
    AFK_ITEM_TWOHAND,
    AFK_ITEM_EVIL,
    AFK_ITEM_DONATION,
    AFK_ITEM_CLANOBJECT,
    AFK_ITEM_CLANCORPSE,
    AFK_ITEM_ANTI_BARD,
    AFK_ITEM_HIDDEN,
    AFK_ITEM_ANTI_DRUID,
    AFK_ITEM_POISONED,
    AFK_ITEM_COVERING,
    AFK_ITEM_DEATHROT,
    AFK_ITEM_BURIED,
    AFK_ITEM_PROTOTYPE,
    AFK_ITEM_NOLOCATE,
    AFK_ITEM_GROUNDROT,
    AFK_ITEM_ANTI_MONK,
    AFK_ITEM_LOYAL,
    AFK_ITEM_BRITTLE,
    AFK_ITEM_RESISTANT,
    AFK_ITEM_IMMUNE,
    AFK_ITEM_ANTI_MEN,
    AFK_ITEM_ANTI_WOMEN,
    AFK_ITEM_ANTI_NEUTER,
    AFK_ITEM_ANTI_HERMA,
    AFK_ITEM_ANTI_SUN,
    AFK_ITEM_ANTI_RANGER,
    AFK_ITEM_ANTI_PALADIN,
    AFK_ITEM_ANTI_NECRO,
    AFK_ITEM_ANTI_APAL,
    AFK_ITEM_ONLY_CLERIC,
    AFK_ITEM_ONLY_MAGE,
    AFK_ITEM_ONLY_ROGUE,
    AFK_ITEM_ONLY_WARRIOR,
    AFK_ITEM_ONLY_BARD,
    AFK_ITEM_ONLY_DRUID,
    AFK_ITEM_ONLY_MONK,
    AFK_ITEM_ONLY_RANGER,
    AFK_ITEM_ONLY_PALADIN,
    AFK_ITEM_ONLY_NECRO,
    AFK_ITEM_ONLY_APAL,
    AFK_ITEM_AUCTION,
    AFK_ITEM_ONMAP,
    AFK_ITEM_PERSONAL,
    AFK_ITEM_LODGED,
    AFK_ITEM_SINDHAE,
    AFK_ITEM_MUSTMOUNT,
    AFK_ITEM_NOAUCTION,
    MAX_AFK_ITEM_FLAG
} afk_item_extra_flags;

void dump_as_afk(zones *Zones, rooms *Rooms, shops *Shops, objects *Objects, mobs *Mobs);
int afk_qcmp_zone_vnum(const void *a, const void *b);
bool str_cmp(const char *astr, const char *bstr);
int get_rflag(char *flag);
int get_exflag(char *flag);
char *flag_string(int bitvector, char *const flagarray[]);
char *ext_flag_string(EXT_BV *bitvector, char *const flagarray[]);
char *strip_cr(char *str);
int afk_convertobjtype(int Type);
EXT_BV afk_convert_rflags(int OldValue);
EXT_BV afk_convert_exflags(int OldType);
EXT_BV afk_convert_oflags(int OldValue);
int afk_convert_wflags(int OldValue);
int get_otype(const char *type);
int get_oflag(char *flag);
int get_wflag(char *flag);
int get_magflag(char *flag);
int afk_convert_spell(int SpellNum);
int afk_convert_skill(int SkillNum);
int afk_convert_liquid(int Liquid);
int *afk_convert_item_values(int Type, int w0, int w1, int w2, int w3);
EXT_BV afk_convert_actflags(int OldValue);
EXT_BV afk_convert_aflags(int OldValue);
int afk_convertsex(int Sex);
int afk_convertposition(int Position);
int get_sex(const char *type);
int get_npc_race(char *type);
int get_npc_class(char *type);
int get_position(const char *type);
int get_actflag(char *flag);
int get_aflag(char *flag);
int get_partflag(char *flag);
int get_risflag(char *flag);
int get_attackflag(char *flag);
void afk_convert_attack(EXT_BV *NewValue, int OldValue);
EXT_BV afk_convert_imm(int OldValue);

#endif

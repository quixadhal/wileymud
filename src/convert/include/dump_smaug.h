#ifndef _DUMP_SMAUG_H
#define _DUMP_SMAUG_H

#include "smaug_common.h"

#define SMAUG_ZONE_OFFSET   250
#define SMAUG_VNUM_OFFSET   25000

/*
 * This file holds defined values for the diku target.
 * They are used in converting the original WileyMUD files into
 * the target diku format.
 */

#define SMAUG_AREA_VERSION_WRITE 1
#define SMAUG_AREA_LEVEL_LIMIT_LOWER 0
#define SMAUG_AREA_LEVEL_LIMIT_UPPER 65
#define SMAUG_CLIMATE_TEMP_NORMAL 2
#define SMAUG_CLIMATE_PRECIP_NORMAL 2
#define SMAUG_CLIMATE_WIND_NORMAL 2

/*
 * Room flags.           Holy cow!  Talked about stripped away..
 * Used in #ROOMS.       Those merc guys know how to strip code down.
 *			 Lets put it all back... ;)
 */
/* Roomflags converted to Extended BV - Samson 8-11-98 */
/* NOTE: If this list grows to 65, raise BFS_MARK in track.c - Samson */
/* Don't forget to add the flag to build.c!!! */
/* Current # of flags: 42 */
typedef enum {
    SMAUG_ROOM_DARK, SMAUG_ROOM_DEATH, SMAUG_ROOM_NO_MOB, SMAUG_ROOM_INDOORS, SMAUG_ROOM_LAWFUL,
    SMAUG_ROOM_NEUTRAL, SMAUG_ROOM_CHAOTIC,
    SMAUG_ROOM_NO_MAGIC, SMAUG_ROOM_TUNNEL, SMAUG_ROOM_PRIVATE, SMAUG_ROOM_SAFE,
    SMAUG_ROOM_SOLITARY, SMAUG_ROOM_PET_SHOP,
    SMAUG_ROOM_NO_RECALL, SMAUG_ROOM_DONATION, SMAUG_ROOM_NODROPALL, SMAUG_ROOM_SILENCE,
    SMAUG_ROOM_LOGSPEECH, SMAUG_ROOM_NODROP,
    SMAUG_ROOM_CLANSTOREROOM, SMAUG_ROOM_NO_SUMMON, SMAUG_ROOM_NO_ASTRAL, SMAUG_ROOM_TELEPORT,
    SMAUG_ROOM_TELESHOWDESC,
    SMAUG_ROOM_NOFLOOR, SMAUG_ROOM_NOSUPPLICATE, SMAUG_ROOM_ARENA, SMAUG_ROOM_NOMISSILE,
    SMAUG_ROOM_R4, SMAUG_ROOM_R5,
    SMAUG_ROOM_PROTOTYPE, SMAUG_ROOM_DND, SMAUG_ROOM_BFS_MARK, SMAUG_ROOM_MAX
} smaug_room_flags;

/*
 * Exit flags.			EX_RES# are reserved for use by the
 * Used in #ROOMS.		SMAUG development team ( No RES flags left, sorry. - Samson )
 */
/* Converted to Extended BV - Samson 7-23-00 */
/* Don't forget to add the flag to build.c!!! */
/* Current # of flags: 35 */
typedef enum {
    SMAUG_EX_ISDOOR, SMAUG_EX_CLOSED, SMAUG_EX_LOCKED, SMAUG_EX_SECRET, SMAUG_EX_SWIM,
    SMAUG_EX_PICKPROOF, SMAUG_EX_FLY,
    SMAUG_EX_CLIMB, SMAUG_EX_DIG, SMAUG_EX_EATKEY, SMAUG_EX_NOPASSDOOR, SMAUG_EX_HIDDEN,
    SMAUG_EX_PASSAGE, SMAUG_EX_PORTAL,
    SMAUG_EX_RES1, SMAUG_EX_RES2, SMAUG_EX_xCLIMB, SMAUG_EX_xENTER, SMAUG_EX_xLEAVE,
    SMAUG_EX_xAUTO, SMAUG_EX_NOFLEE,
    SMAUG_EX_xSEARCHABLE, SMAUG_EX_BASHED, SMAUG_EX_BASHPROOF, SMAUG_EX_NOMOB, SMAUG_EX_WINDOW,
    SMAUG_EX_xLOOK,
    SMAUG_EX_ISBOLT, SMAUG_EX_BOLTED, SMAUG_EXFLAG_MAX
} smaug_exit_flags;

/*
 * Sector types.
 * Used in #ROOMS and on Overland maps.
 */
/* Current number of types: 34 */
typedef enum {
    SMAUG_SECT_INSIDE, SMAUG_SECT_CITY, SMAUG_SECT_FIELD, SMAUG_SECT_FOREST, SMAUG_SECT_HILLS,
    SMAUG_SECT_MOUNTAIN,
    SMAUG_SECT_WATER_SWIM, SMAUG_SECT_WATER_NOSWIM, SMAUG_SECT_UNDERWATER, SMAUG_SECT_AIR,
    SMAUG_SECT_DESERT,
    SMAUG_SECT_DUNNO, SMAUG_SECT_OCEANFLOOR, SMAUG_SECT_UNDERGROUND, SMAUG_SECT_LAVA,
    SMAUG_SECT_SWAMP, SMAUG_SECT_ICE,
    SMAUG_SECT_MAX
} smaug_sector_types;

/*
 * Item types.
 * Used in #OBJECTS.
 */
/* Don't forget to add the flag to build.c!!! */
/* also don't forget to add new item types to the find_oftype function in afk.c */
/* Current # of types: 68 */
typedef enum {
    SMAUG_ITEM_NONE, SMAUG_ITEM_LIGHT, SMAUG_ITEM_SCROLL, SMAUG_ITEM_WAND, SMAUG_ITEM_STAFF,
    SMAUG_ITEM_WEAPON,
    SMAUG_ITEM_FIREWEAPON, SMAUG_ITEM_MISSILE, SMAUG_ITEM_TREASURE, SMAUG_ITEM_ARMOR,
    SMAUG_ITEM_POTION,
    SMAUG_ITEM_WORN, SMAUG_ITEM_FURNITURE, SMAUG_ITEM_TRASH, SMAUG_ITEM_OLDTRAP,
    SMAUG_ITEM_CONTAINER,
    SMAUG_ITEM_NOTE, SMAUG_ITEM_DRINK_CON, SMAUG_ITEM_KEY, SMAUG_ITEM_FOOD, SMAUG_ITEM_MONEY,
    SMAUG_ITEM_PEN,
    SMAUG_ITEM_BOAT, SMAUG_ITEM_CORPSE_NPC, SMAUG_ITEM_CORPSE_PC, SMAUG_ITEM_FOUNTAIN,
    SMAUG_ITEM_PILL,
    SMAUG_ITEM_BLOOD, SMAUG_ITEM_BLOODSTAIN, SMAUG_ITEM_SCRAPS, SMAUG_ITEM_PIPE,
    SMAUG_ITEM_HERB_CON,
    SMAUG_ITEM_HERB, SMAUG_ITEM_INCENSE, SMAUG_ITEM_FIRE, SMAUG_ITEM_BOOK, SMAUG_ITEM_SWITCH,
    SMAUG_ITEM_LEVER,
    SMAUG_ITEM_PULLCHAIN, SMAUG_ITEM_BUTTON, SMAUG_ITEM_DIAL, SMAUG_ITEM_RUNE,
    SMAUG_ITEM_RUNEPOUCH,
    SMAUG_ITEM_MATCH, SMAUG_ITEM_TRAP, SMAUG_ITEM_MAP, SMAUG_ITEM_PORTAL, SMAUG_ITEM_PAPER,
    SMAUG_ITEM_TINDER, SMAUG_ITEM_LOCKPICK, SMAUG_ITEM_SPIKE, SMAUG_ITEM_DISEASE,
    SMAUG_ITEM_OIL, SMAUG_ITEM_FUEL,
    SMAUG_ITEM_EMPTY1, SMAUG_ITEM_EMPTY2, SMAUG_ITEM_MISSILE_WEAPON, SMAUG_ITEM_PROJECTILE,
    SMAUG_ITEM_QUIVER,
    SMAUG_ITEM_SHOVEL, SMAUG_ITEM_SALVE, SMAUG_ITEM_COOK, SMAUG_ITEM_KEYRING, SMAUG_ITEM_ODOR,
    SMAUG_ITEM_CHANCE, SMAUG_ITEM_DRINK_MIX
} smaug_item_types;

/*
 * Extra flags.
 * Used in #OBJECTS. Rearranged for better compatibility with Shard areas - Samson
 */
/* Don't forget to add the flag to o_flags in build.c and handler.c!!! */
/* Current # of flags: 65 */
typedef enum {
    SMAUG_ITEM_GLOW, SMAUG_ITEM_HUM, SMAUG_ITEM_DARK, SMAUG_ITEM_LOYAL, SMAUG_ITEM_EVIL,
    SMAUG_ITEM_INVIS, SMAUG_ITEM_MAGIC,
    SMAUG_ITEM_NODROP, SMAUG_ITEM_BLESS, SMAUG_ITEM_ANTI_GOOD, SMAUG_ITEM_ANTI_EVIL,
    SMAUG_ITEM_ANTI_NEUTRAL,
    SMAUG_ITEM_NOREMOVE, SMAUG_ITEM_INVENTORY, SMAUG_ITEM_ANTI_MAGE, SMAUG_ITEM_ANTI_THIEF,
    SMAUG_ITEM_ANTI_WARRIOR, SMAUG_ITEM_ANTI_CLERIC, SMAUG_ITEM_ORGANIC, SMAUG_ITEM_METAL,
    SMAUG_ITEM_DONATION,
    SMAUG_ITEM_CLANOBJECT, SMAUG_ITEM_CLANCORPSE, SMAUG_ITEM_ANTI_VAMPIRE,
    SMAUG_ITEM_ANTI_DRUID,
    SMAUG_ITEM_HIDDEN, SMAUG_ITEM_POISONED, SMAUG_ITEM_COVERING, SMAUG_ITEM_DEATHROT,
    SMAUG_ITEM_BURIED,
    SMAUG_ITEM_PROTOTYPE, SMAUG_ITEM_NOLOCATE, SMAUG_ITEM_GROUNDROT, SMAUG_ITEM_LOOTABLE,
    SMAUG_ITEM_PERSONAL,
    SMAUG_ITEM_MULTI_INVOKE, SMAUG_ITEM_ENCHANTED, SMAUG_ITEM_FLAG_MAX
} smaug_item_extra_flags;

void                                    dump_as_smaug(zones *Zones, rooms *Rooms, shops *Shops,
						      objects *Objects, mobs *Mobs);
int                                     smaug_qcmp_zone_vnum(const void *a, const void *b);
bool                                    smaug_str_cmp(const char *astr, const char *bstr);
int                                     smaug_get_rflag(char *flag);
int                                     smaug_get_exflag(char *flag);
char                                   *smaug_flag_string(int bitvector,
							  char *const flagarray[]);
char                                   *smaug_ext_flag_string(EXT_BV *bitvector,
							      char *const flagarray[]);
char                                   *smaug_strip_cr(char *str);
int                                     smaug_convertobjtype(int Type);
EXT_BV                                  smaug_convert_rflags(int OldValue);
EXT_BV                                  smaug_convert_exflags(int OldType);
EXT_BV                                  smaug_convert_oflags(int OldValue);
int                                     smaug_convert_wflags(int OldValue);
int                                     smaug_get_otype(const char *type);
int                                     smaug_get_oflag(char *flag);
int                                     smaug_get_wflag(char *flag);
int                                     smaug_get_magflag(char *flag);
int                                     smaug_convert_spell(int SpellNum);
int                                     smaug_convert_skill(int SkillNum);
int                                     smaug_convert_liquid(int Liquid);
int                                    *smaug_convert_item_values(int Type, int w0, int w1,
								  int w2, int w3);
EXT_BV                                  smaug_convert_actflags(int OldValue);
EXT_BV                                  smaug_convert_aflags(int OldValue);
int                                     smaug_convertsex(int Sex);
int                                     smaug_convertposition(int Position);
int                                     smaug_get_sex(const char *type);
int                                     smaug_get_npc_race(char *type);
int                                     smaug_get_npc_class(char *type);
int                                     smaug_get_position(const char *type);
int                                     smaug_get_actflag(char *flag);
int                                     smaug_get_aflag(char *flag);
int                                     smaug_get_partflag(char *flag);
int                                     smaug_get_risflag(char *flag);
int                                     smaug_get_attackflag(char *flag);
void                                    smaug_convert_attack(EXT_BV *NewValue, int OldValue);
EXT_BV                                  smaug_convert_imm(int OldValue);
void                                    renumber_for_smaug(zones *Zones, rooms *Rooms,
							   shops *Shops, objects *Objects,
							   mobs *Mobs);

#endif

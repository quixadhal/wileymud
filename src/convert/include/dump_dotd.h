#ifndef _DUMP_DOTD_H
#define _DUMP_DOTD_H

/*
 * This file holds defined values for the diku target.
 * They are used in converting the original WileyMUD files into
 * the target diku format.
 */

/*
 * Room flags.
 * Used in #ROOMS.
 */
#define DIKU_ROOM_NONE            0x00000000
#define DIKU_ROOM_DARK            0x00000001
#define DIKU_ROOM_NO_MOB          0x00000002
#define DIKU_ROOM_INDOORS         0x00000004
#define DIKU_ROOM_IMMORTAL        0x00000008
#define DIKU_BFS_MARK             0x00000010
/*                                   2
                                     4
                                     8   */
#define DIKU_ROOM_BANK            0x00000100
#define DIKU_ROOM_PRIVATE         0x00000200
#define DIKU_ROOM_SAFE            0x00000400
#define DIKU_ROOM_SOLITARY        0x00000800
#define DIKU_ROOM_PET_SHOP        0x00001000
#define DIKU_ROOM_NO_RECALL       0x00002000

/*
 * Room flags.           Holy cow!  Talked about stripped away..
 * Used in #ROOMS.       Those merc guys know how to strip code down.
 *			 Lets put it all back... ;)
 */

#define DIKU_ROOM_DARK			BV00
#define DIKU_ROOM_DEATH			BV01
#define DIKU_ROOM_NO_MOB		BV02
#define DIKU_ROOM_INDOORS		BV03
#define DIKU_ROOM_LAWFUL		BV04
#define DIKU_ROOM_NEUTRAL		BV05
#define DIKU_ROOM_CHAOTIC		BV06
#define DIKU_ROOM_NO_MAGIC		BV07
#define DIKU_ROOM_TUNNEL		BV08
#define DIKU_ROOM_PRIVATE		BV09
#define DIKU_ROOM_SAFE		BV10
#define DIKU_ROOM_SOLITARY		BV11
#define DIKU_ROOM_PET_SHOP		BV12
#define DIKU_ROOM_NO_RECALL		BV13
#define DIKU_ROOM_DONATION		BV14
#define DIKU_ROOM_NODROPALL		BV15
#define DIKU_ROOM_SILENCE		BV16
#define DIKU_ROOM_LOGSPEECH		BV17
#define DIKU_ROOM_NODROP		BV18
#define DIKU_ROOM_CLANSTOREROOM	BV19
#define DIKU_ROOM_NO_SUMMON		BV20
#define DIKU_ROOM_NO_ASTRAL		BV21
#define DIKU_ROOM_TELEPORT		BV22
#define DIKU_ROOM_TELESHOWDESC	BV23
#define DIKU_ROOM_NOFLOOR		BV24
#define DIKU_ROOM_RECEPTION		BV25
#define DIKU_ROOM_BANK		BV26
#define DIKU_ROOM_RIV_SRC		BV27
#define DIKU_ROOM_ARENA		BV28
#define DIKU_ROOM_NOMISSILE          BV29
#define DIKU_ROOM_PROTOTYPE	     	BV30

/*
 * Sector types.
 * Used in #ROOMS.
 */
#define DIKU_SECT_INSIDE                   0		       /* cant see weather */
#define DIKU_SECT_CITY                     1		       /* outdoor mv as inside */
#define DIKU_SECT_FIELD                    2		       /* illuminated by moon */
#define DIKU_SECT_FOREST                   3		       /* not illuminated */
#define DIKU_SECT_HILLS                    4		       /* slower moving */
#define DIKU_SECT_MOUNTAIN                 5		       /* requires grapple */
#define DIKU_SECT_WATER_SWIM               6		       /* dropped objs float away */
#define DIKU_SECT_WATER_NOSWIM             7		       /* 4 pulse time on objs */
#define DIKU_SECT_UNDERWATER               8		       /* needs breathing help */
#define DIKU_SECT_AIR                      9		       /* requires FLY or vehicle */
#define DIKU_SECT_DESERT                  10		       /* hmm. no recall zone */
#define DIKU_SECT_ICELAND                 11		       /* colder */
#define DIKU_SECT_MAX                     12

/*
 * Sector types.
 * Used in #ROOMS.
 */
typedef enum {
    SECT_INSIDE, SECT_CITY, SECT_FIELD, SECT_FOREST, SECT_HILLS, SECT_MOUNTAIN,
    SECT_WATER_SWIM, SECT_WATER_NOSWIM, SECT_UNDERWATER, SECT_AIR, SECT_DESERT,
    SECT_DUNNO, SECT_OCEANFLOOR, SECT_UNDERGROUND, SECT_TREE, SECT_FIRE,
    SECT_QUICKSAND, SECT_ETHER, SECT_GLACIER, SECT_EARTH,
    SECT_MAX
} sector_types;

void                                    dump_as_dotd(zones *Zones, rooms *Rooms, shops *Shops);

#endif

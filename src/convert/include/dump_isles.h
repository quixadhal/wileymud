#ifndef _DUMP_ISLES_H
#define _DUMP_ISLES_H

/*
 * This file holds defined values for the diku target.
 * They are used in converting the original WileyMUD files into
 * the target diku format.
 */

/*
 * Room flags.
 * Used in #ROOMS.
 */
#define ISLES_ROOM_NONE            0x00000000
#define ISLES_ROOM_DARK            0x00000001
#define ISLES_ROOM_NO_MOB          0x00000002
#define ISLES_ROOM_INDOORS         0x00000004
#define ISLES_ROOM_IMMORTAL        0x00000008
#define ISLES_BFS_MARK             0x00000010
/*                                   2
                                     4
                                     8   */
#define ISLES_ROOM_BANK            0x00000100
#define ISLES_ROOM_PRIVATE         0x00000200
#define ISLES_ROOM_SAFE            0x00000400
#define ISLES_ROOM_SOLITARY        0x00000800
#define ISLES_ROOM_PET_SHOP        0x00001000
#define ISLES_ROOM_NO_RECALL       0x00002000

/*
 * Sector types.
 * Used in #ROOMS.
 */
#define ISLES_SECT_INSIDE                   0	/* cant see weather        */
#define ISLES_SECT_CITY                     1	/* outdoor mv as inside    */
#define ISLES_SECT_FIELD                    2	/* illuminated by moon     */
#define ISLES_SECT_FOREST                   3	/* not illuminated         */
#define ISLES_SECT_HILLS                    4	/* slower moving           */
#define ISLES_SECT_MOUNTAIN                 5	/* requires grapple        */
#define ISLES_SECT_WATER_SWIM               6	/* dropped objs float away */
#define ISLES_SECT_WATER_NOSWIM             7	/* 4 pulse time on objs    */
#define ISLES_SECT_UNDERWATER               8	/* needs breathing help    */
#define ISLES_SECT_AIR                      9	/* requires FLY or vehicle */
#define ISLES_SECT_DESERT                  10	/* hmm. no recall zone     */
#define ISLES_SECT_ICELAND                 11	/* colder                  */
#define ISLES_SECT_MAX                     12


void dump_as_isles(zones *Zones, rooms *Rooms, shops *Shops);
int isles_qcmp_zone_vnum(const void *a, const void *b);

#endif

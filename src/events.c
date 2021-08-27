/*
 * events.c - special events that whizzes might want to do sometimes.
 */

#include <stdio.h>
#include <stdlib.h>
/* #include <unistd.h> */
#include <sys/types.h>
#include <string.h>
#include <ctype.h>

#include "global.h"
#include "bug.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "mudlimits.h"
#include "spell_parser.h"
#include "fight.h"
#include "spec_procs.h"
#include "opinion.h"
#include "hash.h"
#define _DIKU_EVENTS_C
#include "events.h"

static int mob_count = 0;
static int obj_count = 0;
static int gold_count = 0;

static int event_scatter_goodies_zone(int rnum, struct room_data *rp, void *data);
static int event_fill_zone_with_mobs(int rnum, struct room_data *rp, void *data);

static void event_rats_invade_zone(struct char_data *ch, const char *arg);
static void event_undead_invade_zone(struct char_data *ch, const char *arg);
static void event_zombie_master(struct char_data *ch, const char *arg);
static void event_scatter_goodies(struct char_data *ch, const char *arg);

int do_event(struct char_data *ch, const char *argument, int cmd)
{
    static const char *event_list[] = {NULL,
                                       "rats - Rats invade whatever zone you are standing in. [1+]",
                                       "undead - The undead rise and devour the zone you are in. [4-10]",
                                       "xenthia - The Lady of the Dead rises and enters the world. [10-15]",
                                       "goodies - Goodies fall to earth, with a few bandits too. [2-15]",
                                       NULL};
    static const efuncp event_code[] = {NULL,
                                       event_rats_invade_zone,
                                       event_undead_invade_zone,
                                       event_zombie_master,
                                       event_scatter_goodies,
                                       NULL};
    int i = 0;
    char buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    int found = FALSE;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (IS_NPC(ch))
        return TRUE;
    only_argument(argument, buf);
    if (!*buf)
    {
        cprintf(ch, "usage:  event { list | <event name> }\r\n");
        return TRUE;
    }
    if (!strcasecmp(buf, "list"))
    {
        cprintf(ch, "The following events are defined:\r\n");
        for (i = 1; event_list[i]; i++)
        {
            cprintf(ch, "    %s\r\n", event_list[i]);
        }
        return TRUE;
    }
    for (i = 1; event_list[i]; i++)
    {
        if (!strncasecmp(buf, event_list[i], strlen(buf)))
        {
            found = i;
            break;
        }
    }
    if (!found)
        return TRUE;
    cprintf(ch, "Doing Event [#%d] %s\r\n", found, event_list[found]);
    log_info("%s does Event [#%d] %s", GET_NAME(ch), found, event_list[found]);
    event_code[found](ch, argument);
    return TRUE;
}

static int event_scatter_goodies_zone(int rnum, struct room_data *rp, void *data)
{
    int i = 0;
    int exit_found = 0;
    int gold = 0;
    struct char_data *monster = NULL;
    struct obj_data *object = NULL;
    struct obj_data *coins = NULL;
    struct event_goodies  *stuff = data;

    if (DEBUG > 1)
        log_info("called %s with %d, %08zx, %08zx", __PRETTY_FUNCTION__, rnum, (size_t)rp, (size_t)stuff);

    if (!rp || rp->number < stuff->bottom || rp->number > stuff->top)
        return FALSE;
    if (IS_SET(rp->room_flags, (NO_MOB | PEACEFUL | PRIVATE)))
        return FALSE;
    exit_found = 0;
    for (i = 0; i < MAX_NUM_EXITS; i++) /* neswud */
        if (rp->dir_option[i])
        {
            exit_found = 1;
            break;
        }
    if (!exit_found)
        return FALSE;
    if (number(0, 99) >= stuff->chance)
        return FALSE;
    gold = dice(stuff->gold_dice, stuff->gold_die) + stuff->gold_mod;
    gold_count += gold;
    coins = create_money(gold);
    obj_to_room(coins, rnum);
    rprintf(rnum, "In a brilliant green flash, a pile of %s appears before you!\r\n", coins->short_description);
    if (number(0, 99) < stuff->mob_chance)
    {
        i = number(1, stuff->mob_count) - 1;
        if ((monster = read_mobile(stuff->mob_vnum[i], VIRTUAL)))
        {
            GET_GOLD(monster) = 0;
            mob_count++;
            char_to_room(monster, rnum);
            act("A rift of red light rips open and $N rushes out!", FALSE, monster, 0, monster, TO_ROOM);
        }
    }
    if (number(0, 99) < stuff->obj_chance)
    {
        i = number(1, stuff->obj_count) - 1;
        if ((object = read_object(stuff->obj_vnum[i], VIRTUAL)))
        {
            obj_count++;
            obj_to_room(object, rnum);
            rprintf(rnum, "In a shimmering of blue light, %s %s forms!\r\n", SANA(object), object->short_description);
        }
    }
    return TRUE;
}

static int event_fill_zone_with_mobs(int rnum, struct room_data *rp, void *data)
{
    int i = 0;
    int j = 0;
    int couldbe = 0;
    int exit_found = FALSE;
    struct char_data *monster = NULL;
    struct obj_data *object = NULL;
    struct event_mob_in_zone *mobs = data;

    if (DEBUG > 1)
        log_info("called %s with %d, %08zx, %08zx", __PRETTY_FUNCTION__, rnum, (size_t)rp, (size_t)mobs);

    if (!rp || rp->number < mobs->bottom || rp->number > mobs->top)
        return FALSE;
    if (IS_SET(rp->room_flags, (NO_MOB | PEACEFUL | PRIVATE)))
        return FALSE;
    exit_found = 0;
    for (i = 0; i < MAX_NUM_EXITS; i++) /* neswud */
        if (rp->dir_option[i])
        {
            exit_found = 1;
            break;
        }
    if (!exit_found)
        return FALSE;
    couldbe = number(mobs->atleast, mobs->atmost);
    for (j = 0; j < couldbe; j++)
    {
        if (number(0, 99) >= mobs->chance)
            continue;
        i = number(1, mobs->count) - 1;
        if (!(monster = read_mobile(mobs->mobset[i].vnum, VIRTUAL)))
            continue;
        monster->points.max_hit = dice(mobs->mobset[i].hp_dice, mobs->mobset[i].hp_die) + mobs->mobset[i].hp_mod;
        GET_HIT(monster) = GET_MAX_HIT(monster);
        GET_EXP(monster) =
            (dice(mobs->mobset[i].exp_dice, mobs->mobset[i].exp_die) + mobs->mobset[i].exp_mod) * GET_MAX_HIT(monster);
        GET_GOLD(monster) = number(mobs->mobset[i].gold_dice, mobs->mobset[i].gold_die) + mobs->mobset[i].gold_mod;
        if (mobs->mobset[i].obj_vnum >= 0)
        {
            if (number(0, 99) < mobs->mobset[i].obj_chance)
            {
                if ((object = read_object(mobs->mobset[i].obj_vnum, VIRTUAL)))
                    obj_to_char(object, monster);
            }
        }
        char_to_room(monster, rnum);
        mob_count++;
        act("In a shimmering column of blue light, $N appears!", FALSE, monster, 0, monster, TO_ROOM);
    }
    return TRUE;
}

static void event_scatter_goodies(struct char_data *ch, const char *arg)
{
    int the_objects[] = {5023, 6131, 5932, 15011, 7010, 5937, 1901, 9015};
    int the_mobs[] = {1200, 9618, 1400, 5441, 5054, 5055, 15047, 9620, 1202, 9601};
    struct event_goodies junk = {0, 0, 50, 4, 6, -3, 7, 8, the_objects, 11, 10, the_mobs};
    int zone = 0;
    struct room_data *rp = NULL;

    if (DEBUG > 1)
        log_info("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(arg));

    if ((rp = real_roomp(ch->in_room)))
        zone = rp->zone;
    else
        return;

    junk.bottom = zone ? (zone_table[zone - 1].top + 1) : 0;
    junk.top = zone_table[zone].top;
    if (IS_SET(ch->specials.act, PLR_STEALTH))
        zprintf(zone, "\r\nSuddenly, you fell the winds HOWL into being!\r\nYou can hear the sounds of things FALLING "
                      "all around you!\r\n\r\n");
    else
        zprintf(zone,
                "\r\nYou hear %s chanting, and suddenly a wind HOWLS in from %s direction!\r\nYou can hear the sounds "
                "of things FALLING all around you!\r\n\r\n",
                GET_NAME(ch), HSHR(ch));
    mob_count = obj_count = gold_count = 0;
    hash_iterate(&room_db, event_scatter_goodies_zone, &junk);
    cprintf(ch, "You just added %d critters, %d things, and %d gold to %s [#%d].\r\n", mob_count, obj_count, gold_count,
            zone_table[zone].name, zone);
    log_info("%s added %d critters, %d things, and %d gold to %s [#%d].\r\n", GET_NAME(ch), mob_count, obj_count,
             gold_count, zone_table[zone].name, zone);
}

static void event_rats_invade_zone(struct char_data *ch, const char *arg)
{
    struct event_mob_set mobset[11] = {
        /* vnum, hp: xdy+z, exp: xdy+z, gold: xdy+z, object %, obj vnum */
        {4618, 6, 8, 8, 1, 6, 4, 2, 6, 0, 2, 4602}, /* special large rat */
        {4618, 4, 6, 5, 1, 6, 4, 1, 6, 0, 0, -1},   /* large rat */
        {4618, 4, 6, 3, 1, 6, 4, 1, 4, 0, 0, -1},   /* large rat */
        {3432, 3, 6, 1, 1, 6, 4, 1, 2, -1, 0, -1},  /* disgusting rat */
        {3432, 2, 6, 1, 1, 6, 4, 0, 0, 0, 0, -1},   /* disgusting rat */
        {3432, 2, 6, 1, 1, 6, 4, 0, 0, 0, 0, -1},   /* disgusting rat */
        {3432, 2, 6, 1, 1, 6, 4, 0, 0, 0, 0, -1},   /* disgusting rat */
        {3433, 4, 6, 1, 1, 6, 4, 1, 4, -1, 0, -1},  /* giant rat */
        {3433, 4, 6, 1, 1, 6, 4, 1, 4, -1, 0, -1},  /* giant rat */
        {3433, 4, 6, 5, 1, 6, 4, 1, 4, -1, 0, -1},  /* giant rat */
        {5056, 3, 8, 5, 2, 6, 10, 1, 2, -1, 0, -1}  /* black cat */
    };
    struct event_mob_in_zone mobs = {0, 0, 60, 1, 8, 11, mobset};
    int zone = 0;
    struct room_data *rp = NULL;

    if (DEBUG > 1)
        log_info("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(arg));

    if ((rp = real_roomp(ch->in_room)))
        zone = rp->zone;
    else
        return;

    mobs.bottom = zone ? (zone_table[zone - 1].top + 1) : 0;
    mobs.top = zone_table[zone].top;
    if (IS_SET(ch->specials.act, PLR_STEALTH))
        zprintf(zone,
                "\r\nYou feel a great surge of power!\r\nYou hear odd scurrying sounds all around you...\r\n\r\n");
    else
        zprintf(zone,
                "\r\nIn a puff of acrid smoke, you see %s snap %s fingers!\r\nYou hear odd scurrying sounds all around "
                "you...\r\n\r\n",
                GET_NAME(ch), HSHR(ch));
    mob_count = 0;
    hash_iterate(&room_db, event_fill_zone_with_mobs, &mobs);
    cprintf(ch, "You just added %d rats to %s [#%d].\r\n", mob_count, zone_table[zone].name, zone);
    log_info("%s added %d rats to %s [#%d].", GET_NAME(ch), mob_count, zone_table[zone].name, zone);
}

static void event_undead_invade_zone(struct char_data *ch, const char *arg)
{
    struct event_mob_set mobset[64] = {
        /* vnum, hp: xdy+z, exp: xdy+z, gold: xdy+z, object %, obj vnum */
        {9002, 9, 8, 40, 7, 8, 6, 0, 0, 0, 0, -1}, /* ghoul */
        {9002, 8, 8, 30, 5, 8, 6, 0, 0, 0, 0, -1}, /* ghoul */
        {9001, 8, 8, 20, 3, 6, 4, 0, 0, 0, 0, -1}, /* juju zombie */
        {9001, 7, 8, 20, 2, 6, 4, 0, 0, 0, 0, -1}, /* juju zombie */
        {9001, 7, 8, 20, 2, 6, 4, 0, 0, 0, 0, -1}, /* juju zombie */
        {9001, 6, 8, 20, 2, 6, 4, 0, 0, 0, 0, -1}, /* juju zombie */
        {9002, 5, 8, 20, 4, 8, 6, 0, 0, 0, 0, -1}, /* ghoul */
        {9002, 5, 8, 0, 4, 8, 6, 0, 0, 0, 0, -1},  /* ghoul */
        {9002, 4, 8, 20, 4, 8, 6, 0, 0, 0, 0, -1}, /* ghoul */
        {9002, 4, 8, 20, 4, 8, 6, 0, 0, 0, 0, -1}, /* ghoul */
        {4616, 7, 8, 20, 2, 6, 4, 0, 0, 0, 0, -1}, /* banshee */
        {4616, 6, 8, 20, 2, 6, 4, 0, 0, 0, 0, -1}, /* banshee */
        {4616, 6, 8, 20, 2, 6, 4, 0, 0, 0, 0, -1}, /* banshee */
        {4616, 5, 8, 20, 2, 6, 4, 0, 0, 0, 0, -1}, /* banshee */
        {4616, 5, 8, 20, 2, 6, 4, 0, 0, 0, 0, -1}, /* banshee */
        {4615, 6, 8, 20, 1, 6, 4, 0, 0, 0, 0, -1}, /* shadow */
        {4615, 6, 8, 20, 1, 6, 4, 0, 0, 0, 0, -1}, /* shadow */
        {4615, 5, 8, 20, 1, 6, 4, 0, 0, 0, 0, -1}, /* shadow */
        {4615, 5, 8, 20, 1, 6, 4, 0, 0, 0, 0, -1}, /* shadow */
        {4615, 5, 8, 20, 1, 6, 4, 0, 0, 0, 0, -1}, /* shadow */
        {4615, 5, 8, 20, 1, 6, 4, 0, 0, 0, 0, -1}, /* shadow */
        {4615, 4, 8, 20, 1, 6, 4, 0, 0, 0, 0, -1}, /* shadow */
        {4613, 6, 8, 20, 1, 6, 4, 0, 0, 0, 0, -1}, /* poltergeist */
        {4613, 5, 8, 20, 1, 6, 4, 0, 0, 0, 0, -1}, /* poltergeist */
        {4613, 4, 8, 20, 1, 6, 4, 0, 0, 0, 0, -1}, /* poltergeist */
        {4613, 4, 8, 20, 1, 6, 4, 0, 0, 0, 0, -1}, /* poltergeist */
        {4613, 4, 8, 20, 1, 6, 4, 0, 0, 0, 0, -1}, /* poltergeist */
        {4613, 4, 8, 20, 1, 6, 4, 0, 0, 0, 0, -1}, /* poltergeist */
        {4613, 4, 8, 20, 1, 6, 4, 0, 0, 0, 0, -1}, /* poltergeist */
        {9003, 5, 8, 20, 1, 6, 4, 0, 0, 0, 0, -1}, /* skeleton */
        {9003, 4, 8, 20, 1, 6, 4, 0, 0, 0, 0, -1}, /* skeleton */
        {9003, 4, 8, 20, 1, 6, 4, 0, 0, 0, 0, -1}, /* skeleton */
        {9003, 4, 8, 20, 1, 6, 4, 0, 0, 0, 0, -1}, /* skeleton */
        {9003, 3, 8, 20, 1, 6, 4, 0, 0, 0, 0, -1}, /* skeleton */
        {9003, 3, 8, 20, 1, 6, 4, 0, 0, 0, 0, -1}, /* skeleton */
        {9003, 3, 8, 20, 1, 6, 4, 0, 0, 0, 0, -1}, /* skeleton */
        {5300, 5, 8, 5, 1, 6, 4, 0, 0, 0, 0, -1},  /* sewer skeleton */
        {5300, 4, 8, 5, 1, 6, 4, 0, 0, 0, 0, -1},  /* sewer skeleton */
        {5300, 3, 8, 5, 1, 6, 4, 0, 0, 0, 0, -1},  /* sewer skeleton */
        {5300, 3, 8, 5, 1, 6, 4, 0, 0, 0, 0, -1},  /* sewer skeleton */
        {5300, 2, 8, 5, 1, 6, 4, 0, 0, 0, 0, -1},  /* sewer skeleton */
        {5300, 2, 8, 5, 1, 6, 4, 0, 0, 0, 0, -1},  /* sewer skeleton */
        {5300, 2, 8, 5, 1, 6, 4, 0, 0, 0, 0, -1},  /* sewer skeleton */
        {5300, 2, 8, 5, 1, 6, 4, 0, 0, 0, 0, -1},  /* sewer skeleton */
        {5300, 2, 8, 5, 1, 6, 4, 0, 0, 0, 0, -1},  /* sewer skeleton */
        {5300, 2, 8, 5, 1, 6, 4, 0, 0, 0, 0, -1},  /* sewer skeleton */
        {5300, 2, 8, 5, 1, 6, 4, 0, 0, 0, 0, -1},  /* sewer skeleton */
        {5300, 2, 8, 5, 1, 6, 4, 0, 0, 0, 0, -1},  /* sewer skeleton */
        {4603, 5, 8, 5, 1, 6, 4, 0, 0, 0, 0, -1},  /* small skeleton */
        {4603, 4, 8, 5, 1, 6, 4, 0, 0, 0, 0, -1},  /* small skeleton */
        {4603, 3, 8, 5, 1, 6, 4, 0, 0, 0, 0, -1},  /* small skeleton */
        {4603, 2, 8, 5, 1, 6, 4, 0, 0, 0, 0, -1},  /* small skeleton */
        {4603, 2, 8, 5, 1, 6, 4, 0, 0, 0, 0, -1},  /* small skeleton */
        {4603, 2, 8, 5, 1, 6, 4, 0, 0, 0, 0, -1},  /* small skeleton */
        {4603, 1, 8, 5, 1, 6, 4, 0, 0, 0, 0, -1},  /* small skeleton */
        {4603, 1, 8, 5, 1, 6, 4, 0, 0, 0, 0, -1},  /* small skeleton */
        {4603, 1, 8, 5, 1, 6, 4, 0, 0, 0, 0, -1},  /* small skeleton */
        {4603, 1, 8, 5, 1, 6, 4, 0, 0, 0, 0, -1},  /* small skeleton */
        {4603, 1, 8, 5, 1, 6, 4, 0, 0, 0, 0, -1},  /* small skeleton */
        {4603, 1, 8, 5, 1, 6, 4, 0, 0, 0, 0, -1},  /* small skeleton */
        {4603, 1, 8, 5, 1, 6, 4, 0, 0, 0, 0, -1},  /* small skeleton */
        {4603, 1, 8, 5, 1, 6, 4, 0, 0, 0, 0, -1},  /* small skeleton */
        {4603, 1, 8, 5, 1, 6, 4, 0, 0, 0, 0, -1},  /* small skeleton */
        {4603, 1, 8, 5, 1, 6, 4, 0, 0, 0, 0, -1}   /* small skeleton */
    };
    struct event_mob_in_zone mobs = {0, 0, 50, 1, 3, 64, mobset};
    int zone = 0;
    struct room_data *rp = NULL;

    if (DEBUG > 1)
        log_info("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(arg));

    if ((rp = real_roomp(ch->in_room)))
        zone = rp->zone;
    else
        return;
    /* don't code things like this here.... if a whizz annoys people, it is his
     * job to make it up... but if there is a good reason, he should be able to
     * do ANYTHING.
     *
     *  if((rp->zone == 10)||(rp->zone == 11)) {
     *	cprintf(ch,"You may not load this event into the newbie area.\r\n");
     *	return; }
     *
     */
    mobs.bottom = zone ? (zone_table[zone - 1].top + 1) : 0;
    mobs.top = zone_table[zone].top;
    if (IS_SET(ch->specials.act, PLR_STEALTH))
        zprintf(
            zone,
            "\r\nSuddenly, the warmth is snatched from the air around you.\r\nYou feel the icy cold touch of the grave "
            "as you gasp in anticipation...\r\nThe wind begins to howl around you as things move about.\r\n\r\n");
    else
        zprintf(zone,
                "\r\n%s's voice booms all around you, \"Go forth ancient ones!\r\nKill the puny mortals and feast on "
                "their bones!\"\r\nThe air grows still and cold as you feel... things... begin to move.\r\n",
                GET_NAME(ch));
    mob_count = 0;
    hash_iterate(&room_db, event_fill_zone_with_mobs, &mobs);
    cprintf(ch, "You just added %d undead spirits to %s [#%d].\r\n", mob_count, zone_table[zone].name, zone);
    log_info("%s added %d undead to %s [#%d].", GET_NAME(ch), mob_count, zone_table[zone].name, zone);
}

static void event_zombie_master(struct char_data *ch, const char *arg)
{
    struct room_data *rp = NULL;
    struct char_data *master = NULL;
    struct char_data *mob = NULL;
    int i = 0;
    int j = 0;

    if (DEBUG > 1)
        log_info("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(arg));

    if ((rp = real_roomp(ch->in_room)))
    {
        master = read_mobile(666, VIRTUAL); /* xenthia, lady of the dead */
        j = dice(1, 4) + 3;
        if (IS_SET(ch->specials.act, PLR_STEALTH))
            allprintf("\r\nYou feel a darkening of the land.\r\nYou hear a low moaning wind arise nearby...\r\n\r\n");
        else
            allprintf("\r\n%s begins a low incantation, and a bolt of ebon lightning strikes %s upraised hands!\r\nYou "
                      "hear a low moaning wind arise nearby...\r\n\r\n",
                      GET_NAME(ch), HSHR(ch));
        for (i = 0; i < j; i++)
        {
            mob = read_mobile(100, VIRTUAL); /* zombie */
            char_to_room(mob, ch->in_room);
            SET_BIT(mob->specials.affected_by, AFF_CHARM);
            GET_EXP(mob) = number(300, 500);
            add_follower(mob, master);
            mob->points.max_hit = dice(4, 10) + 10;
            mob->points.hit = mob->points.max_hit;
            AddHatred(master->followers->follower, OP_VNUM, ZM_NEMESIS);
            SET_BIT(master->followers->follower->specials.act, ACT_GUARDIAN);
            SET_BIT(master->followers->follower->specials.act, ACT_USE_ITEM);
            SET_BIT(master->followers->follower->specials.affected_by, AFF_FLYING);
        }
        char_to_room(master, ch->in_room);
    }
}

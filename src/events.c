/*
 * events.c - special events that whizzes might want to do sometimes.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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
#include "limits.h"
#include "constants.h"
#include "spell_parser.h"
#include "fight.h"
#include "spec_procs.h"
#define _DIKU_EVENTS_C
#include "events.h"

static int mob_count;

static void event_fill_zone_with_mobs( int rnum, struct room_data *rp,
                                       struct event_mob_in_zone *mobs );
static void event_rats_invade_zone(struct char_data *ch, char *arg);
static void event_zombie_master(struct char_data *ch, char *arg);

void do_event(struct char_data *ch, char *argument, int cmd)
{
  static const char *event_list[] = {
    NULL,
    "rats - Rats invade whatever zone you are standing in.",
    "xenthia - The Lady of the Dead rises and enters the world.",
    NULL
  };
  static const funcp event_code[] = {
    NULL,
    event_rats_invade_zone,
    event_zombie_master,
    NULL
  };
  int i;
  char buf[256];
  int found= 0;

  if(DEBUG)
    log("do_reset");
  if(IS_NPC(ch))
    return;
  only_argument(argument, buf);
  if(!*buf) {
    cprintf(ch, "usage:  event { list | <event name> }\n\r");
    return;
  }
  if(!strcasecmp(buf, "list")) {
    cprintf(ch, "The following events are defined:\n\r");
    for(i= 1; event_list[i]; i++) {
      cprintf(ch, "    %s\n\r", event_list[i]);
    }
    return;
  }
  for(i= 1; event_list[i]; i++) {
    if(!strncasecmp(buf, event_list[i], strlen(buf))) {
      found= i;
      break;
    }
  }
  if(!found)
    return;
  cprintf(ch, "Doing Event [#%d] %s\n\r", found, event_list[found]);
  log("%s does Event [#%d] %s", GET_NAME(ch), found, event_list[found]);
  event_code[found](ch, argument);
}

static void event_fill_zone_with_mobs( int rnum, struct room_data *rp,
                                       struct event_mob_in_zone *mobs )
{
  int i, j, couldbe, exit_found;
  struct char_data *monster;
  struct obj_data *object;

  if(!rp || rp->number < mobs->bottom || rp->number > mobs->top)
    return;
  if(IS_SET(rp->room_flags, (NO_MOB|PEACEFUL|PRIVATE)))
    return;
  exit_found= 0;
  for(i= 0; i< 6; i++) /* neswud */
    if(rp->dir_option[i]) {
      exit_found= 1;
      break;
    }
  if(!exit_found)
    return;
  couldbe= number(mobs->atleast, mobs->atmost);
  for(j= 0; j< couldbe; j++) {
    if(number(0,99) >= mobs->chance)
      continue;
    i= number(1, mobs->count)- 1;
    if(!(monster= read_mobile(mobs->mobset[i].vnum, VIRTUAL)))
      continue;
    monster->points.max_hit= dice(mobs->mobset[i].hp_dice,
            mobs->mobset[i].hp_die) + mobs->mobset[i].hp_mod;
    GET_HIT(monster)= GET_MAX_HIT(monster);
    GET_EXP(monster)= (dice(mobs->mobset[i].exp_dice, mobs->mobset[i].exp_die)
                       + mobs->mobset[i].exp_mod)* GET_MAX_HIT(monster);
    GET_GOLD(monster)= number(mobs->mobset[i].gold_dice,
                              mobs->mobset[i].gold_die)
                     + mobs->mobset[i].gold_mod;
    if(mobs->mobset[i].obj_vnum >= 0) {
      if(number(0, 99) < mobs->mobset[i].obj_chance) {
        if(object= read_object(mobs->mobset[i].obj_vnum, VIRTUAL))
          obj_to_char(object, monster);
      }
    }
    char_to_room(monster, rnum);
    mob_count++;
    act("In a shimmering column of blue light, $N appears!", FALSE,
        monster, 0, monster, TO_ROOM);
  }
}

static void event_rats_invade_zone(struct char_data *ch, char *arg)
{
  struct event_mob_set mobset[10] = {
    { 4618, 6, 8, 8, 1, 6, 4, 2, 6, 0, 10, 4602 }, /* special large rat */
    { 4618, 4, 6, 5, 1, 6, 4, 1, 6, 0, 0, -1 }, /* large rat */
    { 4618, 4, 6, 3, 1, 6, 4, 1, 4, 0, 0, -1 }, /* large rat */
    { 3432, 3, 6, 1, 1, 6, 4, 1, 2, -1, 0, -1 }, /* disgusting rat */
    { 3432, 2, 6, 1, 1, 6, 4, 0, 0, 0, 0, -1 }, /* disgusting rat */
    { 3432, 2, 6, 1, 1, 6, 4, 0, 0, 0, 0, -1 }, /* disgusting rat */
    { 3432, 2, 6, 1, 1, 6, 4, 0, 0, 0, 0, -1 }, /* disgusting rat */
    { 3433, 4, 6, 1, 1, 6, 4, 1, 4, -1, 0, -1 }, /* giant rat */ 
    { 3433, 4, 6, 1, 1, 6, 4, 1, 4, -1, 0, -1 }, /* giant rat */ 
    { 3433, 4, 6, 5, 1, 6, 4, 1, 4, -1, 0, -1 } /* giant rat */ 
  };
  struct event_mob_in_zone mobs = { 0, 0, 60, 1, 6, 10, &mobset };
  int zone;
  struct room_data *rp;

  if(rp= real_roomp(ch->in_room))
    zone= rp->zone;
  else
    return;

  mobs.bottom= zone? (zone_table[zone- 1].top+1): 0;
  mobs.top= zone_table[zone].top;
  if (IS_SET(ch->specials.act, PLR_STEALTH))
    zprintf(zone, "\n\rYou feel a great surge of power!\n\rYou hear odd scurrying sounds all around you...\n\r\n\r");
  else
    zprintf(zone, "\n\rIn a puff of acrid smoke, you see %s snap %s fingers!\n\rYou hear odd scurrying sounds all around you...\n\r\n\r", GET_NAME(ch), HSHR(ch));
  mob_count= 0;
  hash_iterate(&room_db, event_fill_zone_with_mobs, &mobs);
  cprintf(ch, "You just added %d rats to %s [#%d].\n\r", mob_count,
          zone_table[zone].name, zone);
  log("%s added %d rats to %s [#%d].", GET_NAME(ch), mob_count,
          zone_table[zone].name, zone);
}

static void event_zombie_master(struct char_data *ch, char *arg)
{
  struct room_data *rp;
  struct char_data *master;
  struct char_data *mob;
  char buf[MAX_STRING_LENGTH];
  int i, j, zone;

  if(rp= real_roomp(ch->in_room)) {
    zone= rp->zone;
    master = read_mobile(666, VIRTUAL); /* xenthia, lady of the dead */
    j= dice(1,6)+ 3;
    if (IS_SET(ch->specials.act, PLR_STEALTH))
      aprintf("\n\rYou feel a darkening of the land.\n\rYou hear a low moaning wind arise nearby...\n\r\n\r");
    else
      aprintf("\n\r%s begins a low incantation, and a bolt of ebon lightning strikes %s upraised hands!\n\rYou hear a low moaning wind arise nearby...\n\r\n\r", GET_NAME(ch), HSHR(ch));
    for(i= 0; i< j; i++) {
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

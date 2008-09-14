#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include "global.h"
#include "bug.h"
#include "utils.h"

#include "act_move.h"
#include "comm.h"
#include "constants.h"
#include "db.h"
#include "handler.h"
#include "hash.h"
#include "interpreter.h"
#include "multiclass.h"
#include "opinion.h"
#include "spells.h"
#include "fight.h"

#define _TRACKING_C
#include "tracking.h"

/* predicates for find_path function */

 int is_target_room_p(int room, void *tgt_room)
{
  if (DEBUG > 3)
    dlog("called %s with %d, %08x", __PRETTY_FUNCTION__, room, tgt_room);

  return room == (int)tgt_room;
}

 int named_object_on_ground(int room, void *c_data)
{
  char                                   *name = c_data;

  if (DEBUG > 3)
    dlog("called %s with %d, %08x", __PRETTY_FUNCTION__, room, c_data);

  return NULL != get_obj_in_list(name, real_roomp(room)->contents);
}

 int named_mobile_in_room(int room, struct hunting_data *c_data)
{
  struct char_data                       *scan = NULL;

  if (DEBUG > 3)
    dlog("called %s with %d, %08x", __PRETTY_FUNCTION__, room, c_data);

  for (scan = real_roomp(room)->people; scan; scan = scan->next_in_room)
    if (isname(c_data->name, scan->player.name)) {
      *(c_data->victim) = scan;
      return 1;
    }
  return 0;
}

/* Perform breadth first search on rooms from start (in_room)
 * until end (tgt_room) is reached. Then return the correct
 * direction to take from start to reach end.
 * thoth@manatee.cis.ufl.edu
 * if dvar<0 then search THROUGH closed but not locked doors,
 * for mobiles that know how to open doors.
 */

static void donothing(void)
{
  if (DEBUG > 3)
    dlog("called %s with no arguments", __PRETTY_FUNCTION__);

  return;
}

 int choose_exit(int in_room, int tgt_room, int depth)
{
  if (DEBUG > 3)
    dlog("called %s with %d, %d, %d", __PRETTY_FUNCTION__, in_room, tgt_room, depth);

  return find_path(in_room, is_target_room_p, (void *)tgt_room, depth);
}

 int go_direction(struct char_data *ch, int dir)
{
  if (DEBUG > 3)
    dlog("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), dir);

  if (ch->specials.fighting)
    return 0;

  if (!IS_SET(EXIT(ch, dir)->exit_info, EX_CLOSED)) {
    return do_move(ch, "", dir + 1);
  } else if (IsHumanoid(ch) && !IS_SET(EXIT(ch, dir)->exit_info, EX_LOCKED)) {
    open_door(ch, dir);
  }
  return 0;
}

int find_path(int in_room, ifuncp predicate, void *c_data, int depth)
{
  struct room_q                          *tmp_q = NULL;
  struct room_q                          *q_head = NULL;
  struct room_q                          *q_tail = NULL;
  int                                     i = 0;
  int                                     tmp_room = 0;
  int                                     count = 0;
  int                                     thru_doors = 0;
  struct room_data                       *herep = NULL;
  struct room_data                       *therep = NULL;
  struct room_direction_data             *exitp = NULL;
  static struct hash_header               x_room;

  if (DEBUG > 2)
    dlog("called %s with %d, %08x, %d", __PRETTY_FUNCTION__, in_room, c_data, depth);

  /*
   * If start = destination we are done 
   */
  if ((predicate) (in_room, c_data))
    return -1;

  if (depth < 0) {
    thru_doors = TRUE;
    depth = -depth;
  } else {
    thru_doors = FALSE;
  }

  if (x_room.buckets)					       /* junk left over from a previous track */
    destroy_hash_table(&x_room, donothing);
  init_hash_table(&x_room, sizeof(int), 2048);
  hash_enter(&x_room, in_room, (void *)-1);

  /*
   * initialize queue 
   */
  CREATE(q_head, struct room_q, 1);

  q_tail = q_head;
  q_tail->room_nr = in_room;
  q_tail->next_q = 0;

  while (q_head) {
    herep = real_roomp(q_head->room_nr);
    /*
     * for each room test all directions 
     */
    for (i = 0; i < MAX_NUM_EXITS; i++) {
      exitp = herep->dir_option[i];
      if (exit_ok(exitp, &therep) && (thru_doors ? GO_OK_SMARTER : GO_OK)) {
	/*
	 * next room 
	 */
	tmp_room = herep->dir_option[i]->to_room;
	if (!((predicate) (tmp_room, c_data))) {
	  /*
	   * shall we add room to queue ? 
	   */
	  /*
	   * count determines total breadth and depth 
	   */
	  if (!hash_find(&x_room, tmp_room) && (count < depth)
	      && !IS_SET(RM_FLAGS(tmp_room), DEATH)) {
	    count++;
	    /*
	     * mark room as visted and put on queue 
	     */
	    CREATE(tmp_q, struct room_q, 1);

	    tmp_q->room_nr = tmp_room;
	    tmp_q->next_q = 0;
	    q_tail->next_q = tmp_q;
	    q_tail = tmp_q;

	    /*
	     * ancestor for first layer is the direction 
	     */
	    hash_enter(&x_room, tmp_room,
		       ((int)hash_find(&x_room, q_head->room_nr) == -1) ?
		       (void *)(i + 1) : hash_find(&x_room, q_head->room_nr));
	  }
	} else {
	  /*
	   * have reached our goal so free queue 
	   */
	  tmp_room = q_head->room_nr;
	  for (; q_head; q_head = tmp_q) {
	    tmp_q = q_head->next_q;
	    DESTROY(q_head);
	  }
	  /*
	   * return direction if first layer 
	   */
	  if ((int)hash_find(&x_room, tmp_room) == -1)
	    return (i);
	  else						       /* else return the ancestor */
	    return (-1 + (int)hash_find(&x_room, tmp_room));
	}
      }
    }
    /*
     * free queue head and point to next entry 
     */
    tmp_q = q_head->next_q;
    DESTROY(q_head);
    q_head = tmp_q;
  }
  /*
   * couldn't find path 
   */
  return (-1);
}

void MobHunt(struct char_data *ch)
{
  int                                     res = 0;
  int                                     k = 0;

  if (DEBUG > 2)
    dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  if (ch->persist <= 0) {
    res = choose_exit(ch->in_room, ch->old_room, 2000);
    if (res > -1) {
      go_direction(ch, res);
    } else {
      if (ch->specials.hunting) {
	if (ch->specials.hunting->in_room == ch->in_room) {
	  if (CanHate(ch, ch->specials.hunting) &&
	      (!IS_AFFECTED(ch->specials.hunting, AFF_HIDE))) {
	    if (check_peaceful(ch, "You CAN'T fight here!\n\r")) {
	      act("$n fumes at $N", TRUE, ch, 0, ch->specials.hunting, TO_ROOM);
	    } else {
	      if (IsHumanoid(ch)) {
		act("$n screams 'Time to die, $N'", TRUE, ch, 0, ch->specials.hunting, TO_ROOM);
	      } else if (IsAnimal(ch)) {
		act("$n growls.", TRUE, ch, 0, 0, TO_ROOM);
	      }
	      hit(ch, ch->specials.hunting, 0);
	      return;
	    }
	  }
	}
      }
      REMOVE_BIT(ch->specials.act, ACT_HUNTING);
      ch->specials.hunting = 0;
      ch->hunt_dist = 0;
    }
  } else if (ch->specials.hunting) {
    if (ch->hunt_dist <= 50)
      ch->hunt_dist = 50;
    for (k = 1; k <= 2 && ch->specials.hunting; k++) {
      ch->persist -= 1;
      res = dir_track(ch, ch->specials.hunting);
      if (res != -1) {
	go_direction(ch, res);
      } else {
	ch->persist = 0;
	ch->specials.hunting = 0;
	ch->hunt_dist = 0;
      }
    }
  } else {
    ch->persist = 0;
  }
}

int dir_track(struct char_data *ch, struct char_data *vict)
{
  int                                     code = 0;

  if (DEBUG > 2)
    dlog("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), SAFE_NAME(vict));

  if ((!ch) || (!vict))
    return (-1);

  code = choose_exit(ch->in_room, vict->in_room, ch->hunt_dist);

  if ((!ch) || (!vict))
    return (-1);

  if (code == -1) {
    if (ch->in_room == vict->in_room) {
      cprintf(ch, "\n\rTrack -> You have found your target!\n\r");
    } else {
      cprintf(ch, "\n\rTrack -> You have lost the trail.\n\r");
    }
    return (-1);					       /* false to continue the hunt */
  } else {
    cprintf(ch, "\n\rTrack -> You see a faint trail %sward\n\r", dirs[code]);
    return (code);
  }
}

int track(struct char_data *ch, struct char_data *vict)
{
  int                                     code = 0;

  if (DEBUG > 2)
    dlog("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), SAFE_NAME(vict));

  if ((!ch) || (!vict))
    return (-1);

  code = choose_exit(ch->in_room, vict->in_room, ch->hunt_dist);

  if ((!ch) || (!vict))
    return (-1);

  if (ch->in_room == vict->in_room) {
    cprintf(ch, "\n\rTrack -> You have found your target!\n\r");
    return (FALSE);					       /* false to continue the hunt */
  }
  if (code == -1) {
    cprintf(ch, "\n\rTrack -> You have lost the trail.\n\r");
    return (FALSE);
  } else {
    cprintf(ch, "\n\rTrack -> You see a faint trail %sward\n\r", dirs[code]);
    return (TRUE);
  }
}

void do_track(struct char_data *ch, char *argument, int cmd)
{
  int                                     dist = 0;
  int                                     code = 0;
  int                                     cost = 0;
  char                                    name[256] = "\0";
  struct hunting_data                     huntd;

  if (DEBUG)
    dlog("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

  only_argument(argument, name);

  dist = ch->skills[SKILL_TRACK].learned;

  if (IS_SET(ch->player.class, CLASS_RANGER)) {
    dist *= 2;
    cost = 15 - (GET_LEVEL(ch, RANGER_LEVEL_IND) / 10);
  } else if (IS_SET(ch->player.class, CLASS_THIEF)) {
    cost = 50 - GET_LEVEL(ch, THIEF_LEVEL_IND);
    dist = dist;
  } else {
    dist = dist / 2;
    cost = 50 - (GET_LEVEL(ch, BestThiefClass(ch)) / 2);
  }

  if (!dist) {
    cprintf(ch, "You do not know of this skill yet!\n\r");
    return;
  }
  if (GET_MANA(ch) < cost) {
    cprintf(ch, "You can not seem to concentrate on the trail...\n\r");
    return;
  }
  GET_MANA(ch) -= cost;

  switch (GET_RACE(ch)) {
    case RACE_ELVEN:
      dist += 10;					       /* even better */
      break;
    case RACE_DEVIL:
    case RACE_DEMON:
      dist = MAX_ROOMS;					       /* as good as can be */
      break;
    default:
      break;
  }

  if (GetMaxLevel(ch) >= IMMORTAL)
    dist = MAX_ROOMS;

  ch->hunt_dist = dist;

  ch->specials.hunting = 0;
  huntd.name = name;
  huntd.victim = &ch->specials.hunting;
  code = find_path(ch->in_room, (ifuncp)named_mobile_in_room, &huntd, -dist);

  WAIT_STATE(ch, PULSE_VIOLENCE * 1);

  if (code == -1) {
    cprintf(ch, "You are unable to find traces of one.\n\r");
    return;
  } else {
    if (IS_LIGHT(ch->in_room)) {
      SET_BIT(ch->specials.act, PLR_HUNTING);
      cprintf(ch, "You see traces of your quarry to the %s\n\r", dirs[code]);
      if (ch->skills[SKILL_TRACK].learned < 50)
	ch->skills[SKILL_TRACK].learned += 2;
    } else {
      ch->specials.hunting = 0;
      cprintf(ch, "It's too dark in here to track...\n\r");
      return;
    }
  }
}

/*
 * file: utility.c, Utility module.                       Part of DIKUMUD
 * Usage: Utility procedures
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */

#include <stdio.h>
#include <stdlib.h>
/* #include <unistd.h> */
#include <sys/types.h>
/* #include <malloc.h> */
#include <string.h>
#include <assert.h>
#include <time.h>

#include "global.h"
#include "bug.h"
#include "spells.h"
#include "constants.h"
#include "db.h"
#include "opinion.h"
#include "comm.h"
#include "hash.h"
#include "multiclass.h"
#include "handler.h"
#include "fight.h"
#include "act_info.h"
#include "reception.h"
#include "act_off.h"
#include "magic_utils.h"
#include "mudlimits.h"
#include "act_skills.h"
#define _UTILS_C
#include "utils.h"

 int MobVnum(struct char_data *c)
{
  if (DEBUG > 3)
    dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(c));

  if (!c || IS_PC(c))
    return 0;
  if (IS_MOB(c)) {
    return mob_index[c->nr].virtual;
  } else {
    return -1;
  }
}

 int ObjVnum(struct obj_data *o)
{
  if (DEBUG > 3)
    dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_ONAME(o));

  if (!o)
    return 0;
  if (o->item_number >= 0)
    return obj_index[o->item_number].virtual;
  else
    return -1;
}

 int percent(int value, int total)
{
  if (DEBUG > 3)
    dlog("called %s with %d, %d", __PRETTY_FUNCTION__, value, total);

  if (!total)
    return 0;
  return ((value * 100) / total);
}

 char                            *ordinal(int x)
{
  if (DEBUG > 3)
    dlog("called %s with %d", __PRETTY_FUNCTION__, x);

  if (x < 14 && x > 10)
    x = 4;
  else
    x %= 10;
  switch (x) {
    case 1:
      return "st";
    case 2:
      return "nd";
    case 3:
      return "rd";
    default:
      return "th";
  }
}

int GetItemClassRestrictions(struct obj_data *obj)
{
  int                                     total = 0;

  if (DEBUG > 3)
    dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_ONAME(obj));

  if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_MAGE))
    total += CLASS_MAGIC_USER;
  if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_THIEF))
    total += CLASS_THIEF;
  if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_RANGER))
    total += CLASS_RANGER;
  if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_FIGHTER))
    total += CLASS_WARRIOR;
  if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_CLERIC))
    total += CLASS_CLERIC;

  return (total);
}

int CAN_SEE(struct char_data *s, struct char_data *o)
{
  if (DEBUG > 3)
    dlog("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(s), SAFE_NAME(o));

  if (!o || !s)
    return (FALSE);

  if ((s->in_room == -1) || (o->in_room == -1))
    return (FALSE);

  if (o->invis_level >= GetMaxLevel(s))
    return FALSE;

  if (IS_IMMORTAL(s))
    return (TRUE);

  if (IS_AFFECTED(s, AFF_TRUE_SIGHT))
    return (TRUE);

  if (IS_AFFECTED(s, AFF_BLIND) && s != o)
    return (FALSE);

  if (IS_AFFECTED(o, AFF_HIDE))
    return (FALSE);

  if (IS_AFFECTED(o, AFF_INVISIBLE)) {
    if (IS_IMMORTAL(o))
      return (FALSE);
    if (!IS_AFFECTED(s, AFF_DETECT_INVISIBLE))
      return (FALSE);
  }
  if ((IS_DARK(s->in_room) || IS_DARK(o->in_room)) && (!IS_AFFECTED(s, AFF_INFRAVISION)))
    return (FALSE);

  return (TRUE);

}

int exit_ok(struct room_direction_data *room_exit, struct room_data **rpp)
{
  struct room_data                       *rp = NULL;

  if (DEBUG > 3)
    dlog("called %s with %08x, %08x", __PRETTY_FUNCTION__, room_exit, rpp);

  if (rpp == NULL)
    rpp = &rp;
  if (!room_exit) {
    *rpp = NULL;
    return FALSE;
  }
  *rpp = real_roomp(room_exit->to_room);
  return (*rpp != NULL);
}

 int IsImmune(struct char_data *ch, int bit)
{
  if (DEBUG > 3)
    dlog("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), bit);

  if (!ch)
    return 0;
  return IS_SET(bit, ch->M_immune);
}

 int IsResist(struct char_data *ch, int bit)
{
  if (DEBUG > 3)
    dlog("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), bit);

  if (!ch)
    return 0;
  if (GetMaxLevel(ch) >= LOW_IMMORTAL)
    return 1;
  return IS_SET(bit, ch->immune);
}

 int IsSusc(struct char_data *ch, int bit)
{
  if (DEBUG > 3)
    dlog("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), bit);

  if (!ch)
    return 0;
  return IS_SET(bit, ch->susc);
}

/* creates a random number in interval [from;to] */
 int number(int from, int to)
{
  if (DEBUG > 3)
    dlog("called %s with %d, %d", __PRETTY_FUNCTION__, from, to);

  if (to - from + 1)
    return ((random() % (to - from + 1)) + from);
  else
    return from;
}

/* simulates dice roll */
 int dice(int rolls, int size)
{
  int                                     r = 0;
  int                                     sum = 0;

  if (DEBUG > 3)
    dlog("called %s with %d, %d", __PRETTY_FUNCTION__, rolls, size);

  if (size < 1 || rolls < 1)
    return 0;
  if (size == 1)
    return rolls;
  for (r = 1; r <= rolls; r++)
    sum += ((random() % size) + 1);
  return sum;
}

 int fuzz(int x)
{
  if (DEBUG > 3)
    dlog("called %s with %d", __PRETTY_FUNCTION__, x);

  if (!x)
    return 0;
  if (x < 0)
    x = -x;
  return ((random() % ((2 * x) + 1)) - x);
}

int scan_number(char *text, int *rval)
{
  int                                     length = 0;

  if (DEBUG > 3)
    dlog("called %s with %s, %08x", __PRETTY_FUNCTION__, VNULL(text), rval);

  if (!text || !*text)
    return 0;
  if (1 != sscanf(text, " %i %n", rval, &length))
    return 0;
  if (text[length] != 0)
    return 0;
  return 1;
}

 int str_cmp(const char *arg1, const char *arg2)
{
  if (DEBUG > 3)
    dlog("called %s with %s, %s", __PRETTY_FUNCTION__, VNULL(arg1), VNULL(arg2));

  if (!arg1 || !arg2)
    return 1;
  return strcasecmp(arg1, arg2);
}

 int strn_cmp(const char *arg1, const char *arg2, const int n)
{
  if (DEBUG > 3)
    dlog("called %s with %s, %s, %d", __PRETTY_FUNCTION__, VNULL(arg1), VNULL(arg2), n);

  if (!arg1 || !arg2)
    return 1;
  return strncasecmp(arg1, arg2, n);
}

 void sprintbit(unsigned long vektor, const char *names[], char *result)
{
  long                                    nr = 0L;

  if (DEBUG > 3)
    dlog("called %s with %lu, %08x, %08x", __PRETTY_FUNCTION__, vektor, names, result);

  *result = '\0';
  for (nr = 0; vektor; vektor >>= 1) {
    if (IS_SET(1, vektor)) {
      if (*names[nr] != '\n') {
	strcat(result, names[nr]);
	strcat(result, " ");
      } else {
	strcat(result, "UNDEFINED");
	strcat(result, " ");
      }
    }
    if (*names[nr] != '\n')
      nr++;
  }
  if (!*result)
    strcat(result, "NOBITS");
}

 void sprinttype(int type, const char *names[], char *result)
{
  int                                     nr = 0;

  if (DEBUG > 3)
    dlog("called %s with %d, %08x, %08x", __PRETTY_FUNCTION__, type, names, result);

  for (nr = 0; (*names[nr] != '\n'); nr++);
  if (type < nr)
    strcpy(result, names[type]);
  else
    strcpy(result, "UNDEFINED");
}

/* Calculate the REAL time passed over the last t2-t1 centuries (secs) */
struct time_info_data real_time_passed(time_t t2, time_t t1)
{
  long                                    secs = 0L;
  struct time_info_data                   now;

  if (DEBUG > 3)
    dlog("called %s with %u, %u", __PRETTY_FUNCTION__, (long)t2, (long)t1);

  secs = (long)(t2 - t1);
  now.hours = (secs / SECS_PER_REAL_HOUR) % 24;		       /* 0..23 hours */
  secs -= SECS_PER_REAL_HOUR * now.hours;
  now.day = (secs / SECS_PER_REAL_DAY);			       /* 0..34 days */
  secs -= SECS_PER_REAL_DAY * now.day;
  now.month = -1;
  now.year = -1;
  return now;
}

/* Calculate the MUD time passed over the last t2-t1 centuries (secs) */
struct time_info_data mud_time_passed(time_t t2, time_t t1)
{
  long                                    secs = 0L;
  struct time_info_data                   now;

  if (DEBUG > 3)
    dlog("called %s with %u, %u", __PRETTY_FUNCTION__, (long)t2, (long)t1);

  secs = (long)(t2 - t1);
  now.hours = (secs / SECS_PER_MUD_HOUR) % 24;		       /* 0..23 hours */
  secs -= SECS_PER_MUD_HOUR * now.hours;
  now.day = (secs / SECS_PER_MUD_DAY) % 35;		       /* 0..34 days */
  secs -= SECS_PER_MUD_DAY * now.day;
  now.month = (secs / SECS_PER_MUD_MONTH) % 17;		       /* 0..16 months */
  secs -= SECS_PER_MUD_MONTH * now.month;
  now.year = (secs / SECS_PER_MUD_YEAR);		       /* 0..XX? years */
  return now;
}

struct time_info_data age(struct char_data *ch)
{
  struct time_info_data                   player_age;

  if (DEBUG > 3)
    dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  player_age = mud_time_passed(time(0), ch->player.time.birth);
  player_age.year += 17;				       /* All players start at 17 */
  return player_age;
}

int in_group(struct char_data *ch1, struct char_data *ch2)
{
  if (DEBUG > 3)
    dlog("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch1), SAFE_NAME(ch2));

  /* 
   * three possibilities ->
   * 1.  char is char2's master
   * 2.  char2 is char's master
   * 3.  char and char2 follow same.
   * 
   * otherwise not true.
   */

  if ((!ch1) || (!ch2))
    return (0);
  if (ch1 == ch2)
    return (TRUE);
  if ((!ch1->master) && (!ch2->master))
    return (0);
  if (ch2->master)
    if (!strcmp(GET_NAME(ch1), GET_NAME(ch2->master))) {
      return (1);
    }
  if (ch1->master)
    if (!strcmp(GET_NAME(ch1->master), GET_NAME(ch2))) {
      return (1);
    }
  if ((ch2->master) && (ch1->master))
    if (!strcmp(GET_NAME(ch1->master), GET_NAME(ch2->master))) {
      return (1);
    }
  return (0);
}

/*
 * more new procedures 
 */

/*
 * these two procedures give the player the ability to buy 2*bread
 * or put all.bread in bag, or put 2*bread in bag...
 */

int getall(char *name, char *newname)
{
  char                                    arg[40] = "\0\0\0";
  char                                    tmpname[80] = "\0\0\0";
  char                                    otname[80] = "\0\0\0";
  char                                    prd = '\0';

  if (DEBUG > 3)
    dlog("called %s with %s, %s", __PRETTY_FUNCTION__, VNULL(name), VNULL(newname));

  sscanf(name, "%s ", otname);				       /* reads up to first space */

  if (strlen(otname) < 5)
    return (FALSE);

  sscanf(otname, "%3s%c%s", arg, &prd, tmpname);

  if (prd != '.')
    return (FALSE);
  if (tmpname == NULL)
    return (FALSE);
  if (strcmp(arg, "all"))
    return (FALSE);

  while (*name != '.')
    name++;

  name++;

  for (; (*newname = *name); name++, newname++);
  return (TRUE);
}

int getabunch(char *name, char *newname)
{
  int                                     num = 0;
  char                                    tmpname[80] = "\0\0\0";

  if (DEBUG > 3)
    dlog("called %s with %s, %s", __PRETTY_FUNCTION__, VNULL(name), VNULL(newname));

  sscanf(name, "%d*%s", &num, tmpname);
  if (tmpname[0] == '\0')
    return (FALSE);
  if (num < 1)
    return (FALSE);
  if (num > 9)
    num = 9;

  while (*name != '*')
    name++;

  name++;

  for (; (*newname = *name); name++, newname++);

  return (num);

}

int DetermineExp(struct char_data *mob, int exp_flags)
{
  int                                     base = 0;
  int                                     phit = 0;
  int                                     sab = 0;

  /* 
   * reads in the monster, and adds the flags together 
   * for simplicity, 1 exceptional ability is 2 special abilities 
   */

  if (DEBUG > 3)
    dlog("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(mob), exp_flags);

  if (GetMaxLevel(mob) < 0)
    return (1);

  switch (GetMaxLevel(mob)) {

    case 0:
      base = 5;
      phit = 1;
      sab = 10;
      break;

    case 1:
      base = 10;
      phit = 1;
      sab = 15;
      break;

    case 2:
      base = 20;
      phit = 2;
      sab = 20;
      break;

    case 3:
      base = 35;
      phit = 3;
      sab = 25;
      break;

    case 4:
      base = 60;
      phit = 4;
      sab = 30;
      break;

    case 5:
      base = 90;
      phit = 5;
      sab = 40;
      break;

    case 6:
      base = 150;
      phit = 6;
      sab = 75;
      break;

    case 7:
      base = 225;
      phit = 8;
      sab = 125;
      break;

    case 8:
      base = 600;
      phit = 12;
      sab = 175;
      break;

    case 9:
      base = 900;
      phit = 14;
      sab = 300;
      break;

    case 10:
      base = 1100;
      phit = 15;
      sab = 450;
      break;

    case 11:
      base = 1300;
      phit = 16;
      sab = 700;
      break;

    case 12:
      base = 1550;
      phit = 17;
      sab = 700;
      break;

    case 13:
      base = 1800;
      phit = 18;
      sab = 950;
      break;

    case 14:
      base = 2100;
      phit = 19;
      sab = 950;
      break;

    case 15:
      base = 2400;
      phit = 20;
      sab = 1250;
      break;

    case 16:
      base = 2700;
      phit = 23;
      sab = 1250;
      break;

    case 17:
      base = 3000;
      phit = 25;
      sab = 1550;
      break;

    case 18:
      base = 3500;
      phit = 28;
      sab = 1550;
      break;

    case 19:
      base = 4000;
      phit = 30;
      sab = 2100;
      break;

    case 20:
      base = 4500;
      phit = 33;
      sab = 2100;
      break;

    case 21:
      base = 5000;
      phit = 35;
      sab = 2600;
      break;

    case 22:
      base = 6000;
      phit = 40;
      sab = 3000;
      break;

    case 23:
      base = 7000;
      phit = 45;
      sab = 3500;
      break;

    case 24:
      base = 8000;
      phit = 50;
      sab = 4000;
      break;

    case 25:
      base = 9000;
      phit = 55;
      sab = 4500;
      break;

    case 26:
      base = 10000;
      phit = 60;
      sab = 5000;
      break;

    case 27:
      base = 12000;
      phit = 70;
      sab = 6000;
      break;

    case 28:
      base = 14000;
      phit = 80;
      sab = 7000;
      break;

    case 29:
      base = 16000;
      phit = 90;
      sab = 8000;
      break;

    case 30:
      base = 20000;
      phit = 100;
      sab = 10000;
      break;

    default:
      base = 25000;
      phit = 150;
      sab = 20000;
      break;
  }

  return (base + (phit * GET_HIT(mob)) + (sab * exp_flags));

}

void down_river(int current_pulse)
{
  struct char_data                       *ch = NULL;
  struct char_data                       *tmp = NULL;
  struct obj_data                        *obj_object = NULL;
  struct obj_data                        *next_obj = NULL;
  int                                     rd = 0;
  int                                     or = 0;
  struct room_data                       *rp = NULL;

  if (DEBUG > 3)
    dlog("called %s with %d", __PRETTY_FUNCTION__, current_pulse);

  if (current_pulse < 0)
    return;

  for (ch = character_list; ch; ch = tmp) {
    tmp = ch->next;
    if (!IS_NPC(ch)) {
      if (ch->in_room != NOWHERE) {
	if (real_roomp(ch->in_room)->sector_type == SECT_WATER_NOSWIM)
	  if ((real_roomp(ch->in_room))->river_speed > 0) {
	    if ((current_pulse % (real_roomp(ch->in_room))->river_speed) == 0) {
	      if (((real_roomp(ch->in_room))->river_dir < MAX_NUM_EXITS) &&
		  ((real_roomp(ch->in_room))->river_dir >= 0)) {
		rd = (real_roomp(ch->in_room))->river_dir;
		for (obj_object = (real_roomp(ch->in_room))->contents;
		     obj_object; obj_object = next_obj) {
		  next_obj = obj_object->next_content;
		  if ((real_roomp(ch->in_room))->dir_option[rd]) {
		    obj_from_room(obj_object);
		    obj_to_room(obj_object, (real_roomp(ch->in_room))->dir_option[rd]->to_room);
		  }
		}
/*
 * flyers don't get moved
 */
		if (!IS_AFFECTED(ch, AFF_FLYING)) {
		  rp = real_roomp(ch->in_room);
		  if (rp && rp->dir_option[rd] && rp->dir_option[rd]->to_room &&
		      (EXIT(ch, rd)->to_room != NOWHERE)) {
		    if (ch->specials.fighting) {
		      stop_fighting(ch);
		    }
		    cprintf(ch, "You drift %s...\n\r", dirs[rd]);
		    if (MOUNTED(ch)) {
		      or = ch->in_room;
		      char_from_room(ch);
		      char_from_room(MOUNTED(ch));
		      char_to_room(ch, (real_roomp(or))->dir_option[rd]->to_room);
		      char_to_room(MOUNTED(ch), (real_roomp(or))->dir_option[rd]->to_room);
		      do_look(ch, "", 15);
		    } else if (RIDDEN(ch)) {
		      or = ch->in_room;
		      char_from_room(ch);
		      char_from_room(RIDDEN(ch));
		      char_to_room(ch, (real_roomp(or))->dir_option[rd]->to_room);
		      char_to_room(RIDDEN(ch), (real_roomp(or))->dir_option[rd]->to_room);
		      do_look(ch, "", 15);
		    } else {
		      or = ch->in_room;
		      char_from_room(ch);
		      char_to_room(ch, (real_roomp(or))->dir_option[rd]->to_room);
		      do_look(ch, "", 15);
		    }

		    if (IS_SET(RM_FLAGS(ch->in_room), DEATH) && GetMaxLevel(ch) < LOW_IMMORTAL) {
		      death_cry(ch);
		      zero_rent(ch);
		      extract_char(ch);
		    }
		  }
		}
	      }
	    }
	  }
      }
    }
  }
}

/* these are all very arbitrary */
 int IsHumanoid(struct char_data *ch)
{
  if (DEBUG > 3)
    dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  switch (GET_RACE(ch)) {
    case RACE_HALFBREED:
    case RACE_HUMAN:
    case RACE_GNOME:
    case RACE_ELVEN:
    case RACE_DWARF:
    case RACE_HALFLING:
    case RACE_ORC:
    case RACE_LYCANTH:
    case RACE_UNDEAD:
    case RACE_GIANT:
    case RACE_GOBLIN:
    case RACE_DEVIL:
    case RACE_TROLL:
    case RACE_VEGMAN:
    case RACE_MFLAYER:
    case RACE_DEMON:
    case RACE_GHOST:
    case RACE_PRIMATE:
      return TRUE;
    default:
      return FALSE;
  }
}

 int IsAnimal(struct char_data *ch)
{
  if (DEBUG > 3)
    dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  switch (GET_RACE(ch)) {
    case RACE_PREDATOR:
    case RACE_FISH:
    case RACE_BIRD:
    case RACE_HERBIV:
    case RACE_ANIMAL:
    case RACE_LYCANTH:
      return TRUE;
    default:
      return FALSE;
  }
}

 int IsUndead(struct char_data *ch)
{
  if (DEBUG > 3)
    dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  switch (GET_RACE(ch)) {
    case RACE_UNDEAD:
    case RACE_GHOST:
      return TRUE;
    default:
      return FALSE;
  }
}

 int IsLycanthrope(struct char_data *ch)
{
  if (DEBUG > 3)
    dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  switch (GET_RACE(ch)) {
    case RACE_LYCANTH:
      return TRUE;
    default:
      return FALSE;
  }
}

 int IsDiabolic(struct char_data *ch)
{
  if (DEBUG > 3)
    dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  switch (GET_RACE(ch)) {
    case RACE_DEMON:
    case RACE_DEVIL:
      return TRUE;
    default:
      return FALSE;
  }
}

 int IsReptile(struct char_data *ch)
{
  if (DEBUG > 3)
    dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  switch (GET_RACE(ch)) {
    case RACE_REPTILE:
    case RACE_DRAGON:
    case RACE_DINOSAUR:
    case RACE_SNAKE:
      return TRUE;
    default:
      return FALSE;
  }
}

 int IsDraconic(struct char_data *ch)
{
  if (DEBUG > 3)
    dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  switch (GET_RACE(ch)) {
    case RACE_DRAGON:
      return TRUE;
    default:
      return FALSE;
  }
}

 int IsAvian(struct char_data *ch)
{
  if (DEBUG > 3)
    dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  switch (GET_RACE(ch)) {
    case RACE_BIRD:
    case RACE_DRAGON:
    case RACE_GHOST:					       /* insubstantial? */
      return TRUE;
    default:
      return FALSE;
  }
}

 int IsExtraPlanar(struct char_data *ch)
{
  if (DEBUG > 3)
    dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  switch (GET_RACE(ch)) {
    case RACE_DEMON:
    case RACE_DEVIL:
    case RACE_PLANAR:
    case RACE_ELEMENT:
      return TRUE;
    default:
      return FALSE;
  }
}

 int HasHands(struct char_data *ch)
{
  if (DEBUG > 3)
    dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  if (IsHumanoid(ch) || IsDiabolic(ch) || IsDraconic(ch))
    return TRUE;
  else
    return FALSE;
}

 int IsPerson(struct char_data *ch)
{
  if (DEBUG > 3)
    dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  switch (GET_RACE(ch)) {
    case RACE_HUMAN:
    case RACE_ELVEN:
    case RACE_DWARF:
    case RACE_HALFLING:
    case RACE_GNOME:
      return TRUE;
    default:
      return FALSE;
  }
}

void SetHunting(struct char_data *ch, struct char_data *tch)
{
  int                                     persist = 0;
  int                                     dist = 0;

  if (DEBUG > 3)
    dlog("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), SAFE_NAME(tch));

  persist = GetMaxLevel(ch);
  /*   persist *= (int) GET_ALIGNMENT(ch) / 100; */
  persist *= (int)GET_ALIGNMENT(ch) / 200;

  if (persist < 0)
    persist = -persist;

  dist = GET_INT(ch) + GetMaxLevel(ch);

  /*   dist = GET_ALIGNMENT(tch) - GET_ALIGNMENT(ch); */

  dist = (dist > 0) ? dist : -dist;
  if (DoesHate(ch, tch))
    dist *= 2;

  SET_BIT(ch->specials.act, ACT_HUNTING);
  ch->specials.hunting = tch;
  ch->hunt_dist = dist;
  ch->persist = persist;
  ch->old_room = ch->in_room;

  if (GetMaxLevel(tch) >= IMMORTAL) {
    cprintf(tch, ">>%s is hunting you from %s\n\r",
	    ch->player.short_descr, (real_roomp(ch->in_room))->name);
  }
}

void CallForGuard(struct char_data *ch, struct char_data *vict, int lev)
{
  struct char_data                       *i = NULL;

  if (DEBUG > 3)
    dlog("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), SAFE_NAME(vict), lev);

  if (lev == 0)
    lev = 3;

  for (i = character_list; i && lev > 0; i = i->next) {
    if (IS_NPC(i) && (i != ch)) {
      if (!i->specials.fighting) {
	if ((mob_index[i->nr].virtual == GUARD_VNUM)
	    || (mob_index[i->nr].virtual == GUARD2_VNUM)) {
	  if (number(1, 6) >= 3) {
	    if (!IS_SET(i->specials.act, ACT_HUNTING)) {
	      if (vict) {
		SetHunting(i, vict);
		lev--;
	      }
	    }
	  }
	}
      }
    }
  }
}

void CallForAGuard(struct char_data *ch, struct char_data *vict, int lev)
{
  struct char_data                       *point = NULL;

  if (DEBUG > 3)
    dlog("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), SAFE_NAME(vict), lev);

  if (lev == 0)
    lev = 3;

  for (point = character_list; point && (lev > 0); point = point->next) {
    if (IS_NPC(point) &&
	(ch != point) &&
	(!point->specials.fighting) &&
	(real_roomp(ch->in_room)->zone == real_roomp(point->in_room)->zone) &&
	(IS_SET(point->specials.act, ACT_PROTECTOR))) {
      if (number(0, 1)) {
	if (!IS_SET(point->specials.act, ACT_HUNTING)) {
	  if (vict) {
	    SetHunting(point, vict);
	    lev--;
	  }
	}
      }
    }
  }
}

void StandUp(struct char_data *ch)
{
  if (DEBUG > 3)
    dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  if ((GET_POS(ch) < POSITION_STANDING) && (GET_POS(ch) > POSITION_STUNNED)) {
    if (ch->points.hit > (ch->points.max_hit / 2))
      act("$n quickly stands up.", 1, ch, 0, 0, TO_ROOM);
    else if (ch->points.hit > (ch->points.max_hit / 6))
      act("$n slowly stands up.", 1, ch, 0, 0, TO_ROOM);
    else
      act("$n gets to $s feet very slowly.", 1, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POSITION_STANDING;
  }
}

void FighterMove(struct char_data *ch)
{
  int                                     num = 0;

  if (DEBUG > 3)
    dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  num = number(1, 4);

  switch (num) {
    case 1:
      if (!ch->skills[SKILL_BASH].learned)
	ch->skills[SKILL_BASH].learned = 10 + GetMaxLevel(ch) * 4;
      do_bash(ch, GET_NAME(ch->specials.fighting), 0);
      break;
    case 2:
      if (ch->equipment[WIELD] || ch->equipment[WIELD_TWOH]) {
	if (!ch->skills[SKILL_DISARM].learned)
	  ch->skills[SKILL_DISARM].learned = 10 + GetMaxLevel(ch) * 4;
	do_disarm(ch, GET_NAME(ch->specials.fighting), 0);
      } else {
	if (!ch->skills[SKILL_DISARM].learned)
	  ch->skills[SKILL_DISARM].learned = 60 + GetMaxLevel(ch) * 4;
	do_disarm(ch, GET_NAME(ch->specials.fighting), 0);
      }
      break;
    case 3:
    case 4:
      if (!ch->skills[SKILL_KICK].learned)
	ch->skills[SKILL_KICK].learned = 10 + GetMaxLevel(ch) * 4;
      do_kick(ch, GET_NAME(ch->specials.fighting), 0);
      break;
  }
}

void DevelopHatred(struct char_data *ch, struct char_data *v)
{
  if (DEBUG > 3)
    dlog("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), SAFE_NAME(v));

  if (DoesHate(ch, v))
    return;

  if (ch == v)
    return;

/*
 * diff = GET_ALIGNMENT(ch) - GET_ALIGNMENT(v);
 * (diff > 0) ? diff : diff = -diff;
 * 
 * diff /= 20;
 * 
 * patience = (int) 100 * (float) (GET_HIT(ch) / GET_MAX_HIT(ch));
 * 
 * var = number(1,40) - 20;
 * 
 * if (patience+var < diff)
 * AddHated(ch, v);
 */

  AddHated(ch, v);

}

void Teleport(int current_pulse)
{
  struct char_data                       *ch = NULL;
  struct char_data                       *tmp = NULL;
  struct char_data                       *pers = NULL;
  struct obj_data                        *obj_object = NULL;
  struct obj_data                        *temp_obj = NULL;
  int                                     or = 0;
  struct room_data                       *rp = NULL;
  struct room_data                       *dest = NULL;

  if (DEBUG > 3)
    dlog("called %s with %d", __PRETTY_FUNCTION__, current_pulse);

  if (current_pulse < 0)
    return;

  for (ch = character_list; ch; ch = ch->next) {
    if (IS_NPC(ch))
      continue;
    rp = real_roomp(ch->in_room);
    if (rp &&
	(rp)->tele_targ > 0 &&
	rp->tele_targ != rp->number &&
	(rp)->tele_time > 0 && (current_pulse % (rp)->tele_time) == 0) {

      dest = real_roomp(rp->tele_targ);
      if (!dest) {
	dlog("invalid tele_target:ROOM %s", rp->name);
	continue;
      }
      obj_object = (rp)->contents;
      while (obj_object) {
	temp_obj = obj_object->next_content;
	obj_from_room(obj_object);
	obj_to_room(obj_object, (rp)->tele_targ);
	obj_object = temp_obj;
      }

      while (rp->people /* should never fail */ ) {

	/*
	 * find an NPC in the room 
	 */
	for (tmp = rp->people; tmp; tmp = tmp->next_in_room) {
	  if (IS_NPC(tmp))
	    break;
	}

	if (tmp == NULL)
	  break;					       /* we've run out of NPCs */

	or = tmp->in_room;
	char_from_room(tmp);				       /* the list of people in the room has changed */
	char_to_room(tmp, rp->tele_targ);
	if (IS_SET(dest->room_flags, DEATH)) {
	  death_cry(tmp);
	  if (IS_NPC(tmp) && (IS_SET(tmp->specials.act, ACT_POLYSELF))) {
	    /*
	     *   take char from storage, to room     
	     */
	    pers = tmp->desc->original;
	    char_from_room(pers);
	    char_to_room(pers, tmp->in_room);
	    SwitchStuff(tmp, pers);
	    zero_rent(pers);
	    extract_char(tmp);
	    tmp = pers;
	  }
	  zero_rent(tmp);
	  extract_char(tmp);
	}
      }
      or = ch->in_room;
      char_from_room(ch);
      char_to_room(ch, rp->tele_targ);
      if (rp->tele_look) {
	do_look(ch, "", 15);
      }
      if (IS_SET(dest->room_flags, DEATH) && GetMaxLevel(ch) < LOW_IMMORTAL) {
	death_cry(ch);

	if (IS_NPC(ch) && (IS_SET(ch->specials.act, ACT_POLYSELF))) {
	  /*
	   *   take char from storage, to room     
	   */
	  pers = ch->desc->original;
	  char_from_room(pers);
	  char_to_room(pers, ch->in_room);
	  SwitchStuff(ch, pers);
	  zero_rent(ch);
	  extract_char(ch);
	  ch = pers;
	}
	zero_rent(ch);
	extract_char(ch);
      }
    }
  }
}

int HasObject(struct char_data *ch, int ob_num)
{
  int                                     j = 0;
  int                                     found = FALSE;
  struct obj_data                        *i = NULL;

  if (DEBUG > 3)
    dlog("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), ob_num);

  /*
   * equipment too
   */

  for (j = 0; j < MAX_WEAR; j++)
    if (ch->equipment[j])
      found += RecCompObjNum(ch->equipment[j], ob_num);

  if (found > 0)
    return (TRUE);

  /*
   * carrying 
   */
  for (i = ch->carrying; i; i = i->next_content)
    found += RecCompObjNum(i, ob_num);

  if (found > 0)
    return (TRUE);
  else
    return (FALSE);
}

int room_of_object(struct obj_data *obj)
{
  if (DEBUG > 3)
    dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_ONAME(obj));

  if (obj->in_room != NOWHERE)
    return obj->in_room;
  else if (obj->carried_by)
    return obj->carried_by->in_room;
  else if (obj->equipped_by)
    return obj->equipped_by->in_room;
  else if (obj->in_obj)
    return room_of_object(obj->in_obj);
  else
    return NOWHERE;
}

struct char_data                       *char_holding(struct obj_data *obj)
{
  if (DEBUG > 3)
    dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_ONAME(obj));

  if (obj->in_room != NOWHERE)
    return NULL;
  else if (obj->carried_by)
    return obj->carried_by;
  else if (obj->equipped_by)
    return obj->equipped_by;
  else if (obj->in_obj)
    return char_holding(obj->in_obj);
  else
    return NULL;
}

int RecCompObjNum(struct obj_data *o, int obj_num)
{
  int                                     total = 0;
  struct obj_data                        *i = NULL;

  if (DEBUG > 3)
    dlog("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_ONAME(o), obj_num);

  if (obj_index[o->item_number].virtual == obj_num)
    total = 1;

  if (ITEM_TYPE(o) == ITEM_CONTAINER) {
    for (i = o->contains; i; i = i->next_content)
      total += RecCompObjNum(i, obj_num);
  }
  return (total);

}

void RestoreChar(struct char_data *ch)
{
  if (DEBUG > 3)
    dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  GET_MANA(ch) = GET_MAX_MANA(ch);
  GET_HIT(ch) = GET_MAX_HIT(ch);
  GET_MOVE(ch) = GET_MAX_MOVE(ch);

  if (GetMaxLevel(ch) < LOW_IMMORTAL) {
    GET_COND(ch, THIRST) = 24;
    GET_COND(ch, FULL) = 24;
  } else {
    GET_COND(ch, THIRST) = -1;
    GET_COND(ch, FULL) = -1;
  }
}

 void RemAllAffects(struct char_data *ch)
{
  if (DEBUG > 3)
    dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  spell_dispel_magic(IMPLEMENTOR, ch, ch, 0);
}

 char                            *pain_level(struct char_data *ch)
{
  int                                     health_percent = 0;

  if (DEBUG > 3)
    dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  if (GET_MAX_HIT(ch) > 0)
    health_percent = (100 * GET_HIT(ch)) / GET_MAX_HIT(ch);
  else
    health_percent = -1;				       /* How could MAX_HIT be < 1?? */
  if (health_percent >= 100)
    return ("is in an excellent condition");
  else if (health_percent >= 90)
    return ("has a few scratches");
  else if (health_percent >= 75)
    return ("has some small wounds and bruises");
  else if (health_percent >= 50)
    return ("has quite a few wounds");
  else if (health_percent >= 30)
    return ("has some big nasty wounds and scratches");
  else if (health_percent >= 15)
    return ("looks pretty hurt");
  else if (health_percent >= 0)
    return ("is in an awful condition");
  else
    return ("is bleeding awfully from big wounds");
}

 int IsWizard(struct char_data *ch)
{
  if (DEBUG > 3)
    dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  if (!ch)
    return 0;
  return ch->player.class & CLASS_WIZARD;
}

 int IsPriest(struct char_data *ch)
{
  if (DEBUG > 3)
    dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  if (!ch)
    return 0;
  return ch->player.class & CLASS_PRIEST;
}

 int IsMagical(struct char_data *ch)
{
  if (DEBUG > 3)
    dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  if (!ch)
    return 0;
  return ch->player.class & CLASS_MAGICAL;
}

 int IsFighter(struct char_data *ch)
{
  if (DEBUG > 3)
    dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  if (!ch)
    return 0;
  return ch->player.class & CLASS_FIGHTER;
}

 int IsSneak(struct char_data *ch)
{
  if (DEBUG > 3)
    dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  if (!ch)
    return 0;
  return ch->player.class & CLASS_SNEAK;
}

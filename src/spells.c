/*
 * file: spells.c , handling of magic.                   Part of DIKUMUD
 * Usage : Procedures handling all offensive magic.
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */

#include <stdio.h>
#include <stdlib.h>
/* #include <unistd.h> */
#include <sys/types.h>
#include <strings.h>

#include "global.h"
#include "bug.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "interpreter.h"
#include "handler.h"
#include "constants.h"
#include "multiclass.h"
#include "act_move.h"
#define _SPELLS_C
#include "spells.h"
#include "spell_parser.h"
#include "fight.h"

void cast_armor(char level, struct char_data *ch, const char *arg, int type, struct char_data *victim,
		struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      if (affected_by_spell(victim, SPELL_ARMOR) || affected_by_spell(victim, SPELL_STONE_SKIN)) {
	cprintf(ch, "Nothing seems to happen.\r\n");
	return;
      }
      if (ch != victim)
	act("$N is protected.", FALSE, ch, 0, victim, TO_CHAR);

      spell_armor(level, ch, victim, 0);
      break;
    case SPELL_TYPE_POTION:
      if (affected_by_spell(ch, SPELL_ARMOR))
	return;
      spell_armor(level, ch, ch, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj)
	return;
      if (!victim)
	victim = ch;
      if (affected_by_spell(victim, SPELL_ARMOR))
	return;
      spell_armor(level, ch, ch, 0);
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj)
	return;
      if (affected_by_spell(victim, SPELL_ARMOR))
	return;
      spell_armor(level, ch, ch, 0);
      break;
    default:
      log_error("Serious screw-up in armor!");
      break;
  }
}

void cast_teleport(char level, struct char_data *ch, const char *arg, int type,
		   struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SPELL:
      if (!victim)
	victim = ch;
      spell_teleport(level, ch, victim, 0);
      break;

    case SPELL_TYPE_WAND:
      if (!victim)
	victim = ch;
      spell_teleport(level, ch, victim, 0);
      break;

    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (victim != ch)
	  spell_teleport(level, ch, victim, 0);
      break;

    default:
      log_error("Serious screw-up in teleport!");
      break;
  }
}

void cast_bless(char level, struct char_data *ch, const char *arg, int type, struct char_data *victim,
		struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      if (tar_obj) {					       /* It's an object */
	if (IS_SET(tar_obj->obj_flags.extra_flags, ITEM_BLESS)) {
	  cprintf(ch, "Nothing seems to happen.\r\n");
	  return;
	}
	spell_bless(level, ch, 0, tar_obj);

      } else {						       /* Then it is a PC | NPC */

	if (affected_by_spell(victim, SPELL_BLESS) || (GET_POS(victim) == POSITION_FIGHTING)) {
	  cprintf(ch, "Nothing seems to happen.\r\n");
	  return;
	}
	spell_bless(level, ch, victim, 0);
      }
      break;
    case SPELL_TYPE_POTION:
      if (affected_by_spell(ch, SPELL_BLESS) || (GET_POS(ch) == POSITION_FIGHTING))
	return;
      spell_bless(level, ch, ch, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj) {					       /* It's an object */
	if (IS_SET(tar_obj->obj_flags.extra_flags, ITEM_BLESS))
	  return;
	spell_bless(level, ch, 0, tar_obj);

      } else {						       /* Then it is a PC | NPC */

	if (!victim)
	  victim = ch;

	if (affected_by_spell(victim, SPELL_BLESS) || (GET_POS(victim) == POSITION_FIGHTING))
	  return;
	spell_bless(level, ch, victim, 0);
      }
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj) {					       /* It's an object */
	if (IS_SET(tar_obj->obj_flags.extra_flags, ITEM_BLESS))
	  return;
	spell_bless(level, ch, 0, tar_obj);

      } else {						       /* Then it is a PC | NPC */

	if (affected_by_spell(victim, SPELL_BLESS) || (GET_POS(victim) == POSITION_FIGHTING))
	  return;
	spell_bless(level, ch, victim, 0);
      }
      break;
    default:
      log_error("Serious screw-up in bless!");
      break;
  }
}

void cast_blindness(char level, struct char_data *ch, const char *arg, int type,
		    struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      if (IS_AFFECTED(victim, AFF_BLIND)) {
	cprintf(ch, "Nothing seems to happen.\r\n");
	return;
      }
      spell_blindness(level, ch, victim, 0);
      break;
    case SPELL_TYPE_POTION:
      if (IS_AFFECTED(ch, AFF_BLIND))
	return;
      spell_blindness(level, ch, ch, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj)
	return;
      if (!victim)
	victim = ch;
      if (IS_AFFECTED(victim, AFF_BLIND))
	return;
      spell_blindness(level, ch, victim, 0);
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj)
	return;
      if (!victim)
	victim = ch;
      if (IS_AFFECTED(victim, AFF_BLIND))
	return;
      spell_blindness(level, ch, victim, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (!in_group(ch, victim))
	  if (!(IS_AFFECTED(victim, AFF_BLIND)))
	    spell_blindness(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in blindness!");
      break;
  }
}

void cast_burning_hands(char level, struct char_data *ch, const char *arg, int type,
			struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
      spell_burning_hands(level, ch, 0, 0);
      break;
    default:
      log_error("Serious screw-up in burning hands!");
      break;
  }
}

void cast_call_lightning(char level, struct char_data *ch, const char *arg, int type,
			 struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_call_lightning(level, ch, victim, 0);
      break;
    case SPELL_TYPE_POTION:
      if (OUTSIDE(ch) && (weather_info.sky >= SKY_RAINING)) {
	spell_call_lightning(level, ch, ch, 0);
      }
      break;
    case SPELL_TYPE_SCROLL:
      if (OUTSIDE(ch) && (weather_info.sky >= SKY_RAINING)) {
	if (victim)
	  spell_call_lightning(level, ch, victim, 0);
	else if (!tar_obj)
	  spell_call_lightning(level, ch, ch, 0);
      }
      break;
    case SPELL_TYPE_STAFF:
      if (OUTSIDE(ch) && (weather_info.sky >= SKY_RAINING)) {
	for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	  if (!in_group(victim, ch))
	    spell_call_lightning(level, ch, victim, 0);
      }
      break;
    default:
      log_error("Serious screw-up in call lightning!");
      break;
  }
}

void cast_charm_person(char level, struct char_data *ch, const char *arg, int type,
		       struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_charm_person(level, ch, victim, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (!victim)
	return;
      spell_charm_person(level, ch, victim, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (!in_group(victim, ch))
	  spell_charm_person(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in charm person!");
      break;
  }
}

void cast_chill_touch(char level, struct char_data *ch, const char *arg, int type,
		      struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_chill_touch(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in chill touch!");
      break;
  }
}

#define GLASS_MIRROR	1133

void cast_clone(char level, struct char_data *ch, const char *arg, int type, struct char_data *victim,
		struct obj_data *tar_obj)
{
  struct char_data                       *this_victim = NULL;
  struct char_data                       *next_victim = NULL;
  struct obj_data                        *sac = NULL;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  if (!ch->equipment[HOLD]) {
    cprintf(ch, "You must be holding the component for this spell.\r\n");
    return;
  }
  sac = unequip_char(ch, HOLD);
  if (sac) {
    obj_to_char(sac, ch);
    if (ObjVnum(sac) != GLASS_MIRROR) {
      cprintf(ch, "That is not the correct item.\r\n");
      return;
    }
  } else {
    cprintf(ch, "You must be holding the component for this spell.\r\n");
    return;
  }
  cprintf(ch, "You shatter %s %s into thousands of tiny shards.\r\n", SANA(sac),
	  sac->short_description);
  act("$n shatters %s %s into thousands of tiny shards.\r\n", FALSE, ch, 0, 0,
      TO_ROOM, SANA(sac), sac->short_description);
  obj_from_char(sac);
  extract_obj(sac);

  for (this_victim = character_list; this_victim; this_victim = next_victim) {
    next_victim = this_victim->next;
    if ((ch->in_room == this_victim->in_room)) {
      if (IS_IMMORTAL(this_victim))
	continue;
      if (affected_by_spell(this_victim, SPELL_SHIELD)) {
	act("Tiny shards of glass knife through the air bouncing off your shield!\r\n",
	    FALSE, ch, 0, this_victim, TO_VICT);
	continue;
      }
      damage(this_victim, this_victim, dice(2, 4), TYPE_SUFFERING);
      act("Tiny shards of glass knife through the air and strike you!\r\n",
	  FALSE, ch, 0, this_victim, TO_VICT);
    }
  }

  switch (type) {
    case SPELL_TYPE_SPELL:
      if (victim) {
	spell_clone(level, ch, victim, 0);
      } else {
	cprintf(ch, "You create a duplicate of %s %s.\r\n", SANA(tar_obj),
		tar_obj->short_description);
	act("$n creates a duplicate of %s %s,\r\n", FALSE, ch, 0, 0,
            TO_ROOM, SANA(tar_obj), tar_obj->short_description);
	spell_clone(level, ch, 0, tar_obj);
      }
      break;
    default:
      log_error("Serious screw-up in clone!");
      break;
  }
}

void cast_colour_spray(char level, struct char_data *ch, const char *arg, int type,
		       struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_colour_spray(level, ch, victim, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (victim)
	spell_colour_spray(level, ch, victim, 0);
      else if (!tar_obj)
	spell_colour_spray(level, ch, ch, 0);
      break;
    case SPELL_TYPE_WAND:
      if (victim)
	spell_colour_spray(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in colour spray!");
      break;
  }
}

void cast_control_weather(char level, struct char_data *ch, const char *arg, int type,
			  struct char_data *victim, struct obj_data *tar_obj)
{
  char                                    buffer[MAX_STRING_LENGTH] = "\0\0\0";

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:

      one_argument(arg, buffer);

      if (str_cmp("better", buffer) && str_cmp("worse", buffer)) {
	cprintf(ch, "Do you want it to get better or worse?\r\n");
	return;
      }
      if (!OUTSIDE(ch)) {
	cprintf(ch, "You need to be outside.\r\n");
      }
      if (!str_cmp("better", buffer)) {
	if (weather_info.sky == SKY_CLOUDLESS)
	  return;
	if (weather_info.sky == SKY_CLOUDY) {
	  oprintf("The clouds disappear.\r\n");
	  weather_info.sky = SKY_CLOUDLESS;
	}
	if (weather_info.sky == SKY_RAINING) {
	  if ((time_info.month > 3) && (time_info.month < 14))
	    oprintf("The rain has stopped.\r\n");
	  else
	    oprintf("The snow has stopped. \r\n");
	  weather_info.sky = SKY_CLOUDY;
	}
	if (weather_info.sky == SKY_LIGHTNING) {
	  if ((time_info.month > 3) && (time_info.month < 14))
	    oprintf("The lightning has gone, but it is still raining.\r\n");
	  else
	    oprintf("The blizzard is over, but it is still snowing.\r\n");
	  weather_info.sky = SKY_RAINING;
	}
	return;
      } else {
	if (weather_info.sky == SKY_CLOUDLESS) {
	  oprintf("The sky is getting cloudy.\r\n");
	  weather_info.sky = SKY_CLOUDY;
	  return;
	}
	if (weather_info.sky == SKY_CLOUDY) {
	  if ((time_info.month > 3) && (time_info.month < 14))
	    oprintf("It starts to rain.\r\n");
	  else
	    oprintf("It starts to snow. \r\n");
	  weather_info.sky = SKY_RAINING;
	}
	if (weather_info.sky == SKY_RAINING) {
	  if ((time_info.month > 3) && (time_info.month < 14))
	    oprintf("You are caught in lightning storm.\r\n");
	  else
	    oprintf("You are caught in a blizzard. \r\n");
	  weather_info.sky = SKY_LIGHTNING;
	}
	if (weather_info.sky == SKY_LIGHTNING) {
	  return;
	}
	return;
      }
      break;

    default:
      log_error("Serious screw-up in control weather!");
      break;
  }
}

void cast_create_food(char level, struct char_data *ch, const char *arg, int type,
		      struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      act("$n magically creates a mushroom.", FALSE, ch, 0, 0, TO_ROOM);
      spell_create_food(level, ch, 0, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj)
	return;
      if (victim)
	return;
      spell_create_food(level, ch, 0, 0);
      break;
    default:
      log_error("Serious screw-up in create food!");
      break;
  }
}

void cast_create_water(char level, struct char_data *ch, const char *arg, int type,
		       struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      if (tar_obj->obj_flags.type_flag != ITEM_DRINKCON) {
	cprintf(ch, "It is unable to hold water.\r\n");
	return;
      }
      spell_create_water(level, ch, 0, tar_obj);
      break;
    default:
      log_error("Serious screw-up in create water!");
      break;
  }
}

void cast_cure_blind(char level, struct char_data *ch, const char *arg, int type,
		     struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_cure_blind(level, ch, victim, 0);
      break;
    case SPELL_TYPE_POTION:
      spell_cure_blind(level, ch, ch, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (victim != ch)
	  spell_cure_blind(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in cure blind!");
      break;
  }
}

/* Offensive */

void cast_shocking_grasp(char level, struct char_data *ch, const char *arg, int type,
			 struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_shocking_grasp(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in shocking grasp!");
      break;
  }
}

#define HAMMER_1	5055
#define HAMMER_2	5932
#define HAMMER_3	9713

void cast_new_earthquake(char level, struct char_data *ch, const char *arg, int type,
			 struct char_data *victim, struct obj_data *tar_obj)
{
  int                                     vnum = 0;
  int                                     done = FALSE;
  struct obj_data                        *sac = NULL;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  if (ch->equipment[HOLD]) {
    vnum = ObjVnum(ch->equipment[HOLD]);
    if (vnum == HAMMER_1 || vnum == HAMMER_2 || vnum == HAMMER_3) {
      if ((sac = unequip_char(ch, HOLD))) {
	obj_to_char(sac, ch);
	done = 1;
      }
    }
  }
  if (!done) {
    if (ch->equipment[WIELD]) {
      vnum = ObjVnum(ch->equipment[WIELD]);
      if (vnum == HAMMER_1 || vnum == HAMMER_2 || vnum == HAMMER_3) {
	if ((sac = unequip_char(ch, WIELD))) {
	  obj_to_char(sac, ch);
	  done = 1;
	}
      }
    }
  }
  if (!done) {
    if (ch->equipment[WIELD_TWOH]) {
      vnum = ObjVnum(ch->equipment[WIELD_TWOH]);
      if (vnum == HAMMER_1 || vnum == HAMMER_2 || vnum == HAMMER_3) {
	if ((sac = unequip_char(ch, WIELD_TWOH))) {
	  obj_to_char(sac, ch);
	  done = 1;
	}
      }
    }
  }
  if (!done) {
    cprintf(ch, "You must have the component for this spell ready.\r\n");
    return;
  }
  act("You smash $o into the ground and cause it to split asunder!\r\n",
      FALSE, ch, sac, 0, TO_VICT);
  act("$n smashes $o into the ground and the earth splits open!\r\n",
      FALSE, ch, sac, 0, TO_ROOM);
  obj_from_char(sac);
  extract_obj(sac);

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
      spell_new_earthquake(level, ch, 0, 0);
      break;
    default:
      log_error("Serious screw-up in new earthquake!");
      break;
  }
}

void cast_earthquake(char level, struct char_data *ch, const char *arg, int type,
		     struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
      spell_earthquake(level, ch, 0, 0);
      break;
    default:
      log_error("Serious screw-up in earthquake!");
      break;
  }
}

void cast_energy_drain(char level, struct char_data *ch, const char *arg, int type,
		       struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_energy_drain(level, ch, victim, 0);
      break;
    case SPELL_TYPE_POTION:
      spell_energy_drain(level, ch, ch, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (victim)
	spell_energy_drain(level, ch, victim, 0);
      else if (!tar_obj)
	spell_energy_drain(level, ch, ch, 0);
      break;
    case SPELL_TYPE_WAND:
      if (victim)
	spell_energy_drain(level, ch, victim, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (!in_group(ch, victim))
	  if (victim != ch)
	    spell_energy_drain(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in energy drain!");
      break;
  }
}

void cast_fireball(char level, struct char_data *ch, const char *arg, int type,
		   struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
      spell_fireball(level, ch, 0, 0);
      break;
    default:
      log_error("Serious screw-up in fireball");
      break;
  }

/*
 * switch (type) {
 * case SPELL_TYPE_SPELL:
 * spell_fireball(level, ch, victim, 0);
 * break;
 * case SPELL_TYPE_SCROLL:
 * if(victim)
 * spell_fireball(level, ch, victim, 0);
 * else if(!tar_obj)
 * spell_fireball(level, ch, ch, 0);
 * break;
 * case SPELL_TYPE_WAND:
 * if(victim)
 * spell_fireball(level, ch, victim, 0);
 * break;
 * default : 
 * log_error("Serious screw-up in fireball!");
 * break;
 * 
 * }
 */

}

void cast_harm(char level, struct char_data *ch, const char *arg, int type, struct char_data *victim,
	       struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_harm(level, ch, victim, 0);
      break;
    case SPELL_TYPE_POTION:
      spell_harm(level, ch, ch, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (!in_group(ch, victim))
	  spell_harm(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in harm!");
      break;

  }
}

void cast_lightning_bolt(char level, struct char_data *ch, const char *arg, int type,
			 struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_lightning_bolt(level, ch, victim, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (victim)
	spell_lightning_bolt(level, ch, victim, 0);
      else if (!tar_obj)
	spell_lightning_bolt(level, ch, ch, 0);
      break;
    case SPELL_TYPE_WAND:
      if (victim)
	spell_lightning_bolt(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in lightning bolt!");
      break;

  }
}

void cast_acid_blast(char level, struct char_data *ch, const char *arg, int type,
		     struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_acid_blast(level, ch, victim, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (victim)
	spell_acid_blast(level, ch, victim, 0);
      else
	spell_acid_blast(level, ch, ch, 0);
      break;
    case SPELL_TYPE_WAND:
      if (victim)
	spell_acid_blast(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in acid blast!");
      break;

  }
}

void cast_cone_of_cold(char level, struct char_data *ch, const char *arg, int type,
		       struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
      spell_cone_of_cold(level, ch, 0, 0);
      break;

    default:
      log_error("Serious screw-up in cone of cold!");
      break;

  }
}

void cast_ice_storm(char level, struct char_data *ch, const char *arg, int type,
		    struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
      spell_ice_storm(level, ch, 0, 0);
      break;

    default:
      log_error("Serious screw-up in acid blast!");
      break;

  }
}

void cast_meteor_swarm(char level, struct char_data *ch, const char *arg, int type,
		       struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_meteor_swarm(level, ch, victim, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (victim)
	spell_meteor_swarm(level, ch, victim, 0);
      else
	spell_meteor_swarm(level, ch, ch, 0);
      break;
    case SPELL_TYPE_WAND:
      if (victim)
	spell_meteor_swarm(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in meteor swarm!");
      break;

  }
}

void cast_flamestrike(char level, struct char_data *ch, const char *arg, int type,
		      struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_flamestrike(level, ch, victim, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (victim)
	spell_flamestrike(level, ch, victim, 0);
      else if (!tar_obj)
	spell_flamestrike(level, ch, ch, 0);
      break;
    case SPELL_TYPE_WAND:
      if (victim)
	spell_flamestrike(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in flamestrike!");
      break;

  }
}

void cast_magic_missile(char level, struct char_data *ch, const char *arg, int type,
			struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_magic_missile(level, ch, victim, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (victim)
	spell_magic_missile(level, ch, victim, 0);
      else if (!tar_obj)
	spell_magic_missile(level, ch, ch, 0);
      break;
    case SPELL_TYPE_WAND:
      if (victim)
	spell_magic_missile(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in magic missile!");
      break;

  }
}

void cast_cause_light(char level, struct char_data *ch, const char *arg, int type,
		      struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_cause_light(level, ch, victim, 0);
      break;
    case SPELL_TYPE_POTION:
      spell_cause_light(level, ch, ch, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (!in_group(ch, victim))
	  spell_cause_light(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in cause light wounds!");
      break;

  }
}

void cast_cause_serious(char level, struct char_data *ch, const char *arg, int type,
			struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_cause_serious(level, ch, victim, 0);
      break;
    case SPELL_TYPE_POTION:
      spell_cause_serious(level, ch, ch, 0);
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj)
	return;
      if (!victim)
	victim = ch;
      spell_cause_serious(level, ch, victim, 0);
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (!in_group(ch, victim))
	  spell_cause_serious(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in cause serious wounds!");
      break;

  }
}

void cast_cause_critic(char level, struct char_data *ch, const char *arg, int type,
		       struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_cause_critical(level, ch, victim, 0);
      break;
    case SPELL_TYPE_POTION:
      spell_cause_critical(level, ch, ch, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (!in_group(ch, victim))
	  spell_cause_critical(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in cause critical!");
      break;

  }
}

void cast_geyser(char level, struct char_data *ch, const char *arg, int type,
		 struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
      spell_geyser(level, ch, 0, 0);
      break;
    default:
      log_error("Serious screw-up in geyser!");
      break;
  }
}

void cast_green_slime(char level, struct char_data *ch, const char *arg, int type,
		      struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_green_slime(level, ch, victim, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (victim)
	spell_green_slime(level, ch, victim, 0);
      else if (!tar_obj)
	spell_green_slime(level, ch, ch, 0);
      break;
    case SPELL_TYPE_WAND:
      if (victim)
	spell_green_slime(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in green Slime!");
      break;
  }
}

/*
 * file: spells2.c , Implementation of magic.             Part of DIKUMUD
 * Usage : All the non-offensive magic handling routines.
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */

void cast_resurrection(char level, struct char_data *ch, const char *arg, int type,
		       struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      if (!tar_obj)
	return;
      spell_resurrection(level, ch, 0, tar_obj);
      break;
    case SPELL_TYPE_STAFF:
      if (!tar_obj)
	return;
      spell_resurrection(level, ch, 0, tar_obj);
      break;
    default:
      log_error("Serious problem in 'resurrection'");
      break;
  }

}

void cast_mana(char level, struct char_data *ch, const char *arg, int type, struct char_data *victim,
	       struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_POTION:
      spell_mana(level, ch, ch, 0);
      break;
    case SPELL_TYPE_WAND:
      if (!victim)
	victim = ch;
      spell_mana(level, ch, victim, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (victim != ch)
	  spell_mana(level, ch, victim, 0);
    default:
      log_error("Serious problem in 'mana'");
      break;
  }

}

void cast_stone_skin(char level, struct char_data *ch, const char *arg, int type,
		     struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      if (affected_by_spell(ch, SPELL_STONE_SKIN) ||
	  affected_by_spell(ch, SPELL_ARMOR) || affected_by_spell(ch, SPELL_SHIELD)) {
	cprintf(ch, "Nothing seems to happen.\r\n");
	return;
      }
      spell_stone_skin(level, ch, ch, 0);
      break;
    case SPELL_TYPE_POTION:
      if (affected_by_spell(ch, SPELL_STONE_SKIN))
	return;
      spell_stone_skin(level, ch, ch, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj)
	return;
      if (affected_by_spell(ch, SPELL_STONE_SKIN))
	return;
      spell_stone_skin(level, ch, ch, 0);
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj)
	return;
      if (affected_by_spell(ch, SPELL_STONE_SKIN))
	return;
      spell_stone_skin(level, ch, ch, 0);
      break;
    default:
      log_error("Serious screw-up in stone_skin!");
      break;
  }
}

void cast_astral_walk(char level, struct char_data *ch, const char *arg, int type,
		      struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SPELL:

      if (!victim)
	cprintf(ch, "Yes, but who do you wish to walk to?\n");
      else
	spell_astral_walk(level, ch, victim, 0);
      break;

    default:
      log_error("Serious screw-up in astral walk!");
      break;
  }
}

void cast_visions(char level, struct char_data *ch, const char *arg, int type,
		  struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SPELL:

      if (!victim)
	cprintf(ch, "Yes, but who do you wish to spy on?\n");
      else
	spell_visions(level, ch, victim, 0);
      break;

    default:
      log_error("Serious screw-up in visions!");
      break;
  }
}

void cast_infravision(char level, struct char_data *ch, const char *arg, int type,
		      struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      if (IS_AFFECTED(victim, AFF_INFRAVISION)) {
	cprintf(ch, "Nothing seems to happen.\r\n");
	return;
      }
      spell_infravision(level, ch, victim, 0);
      break;
    case SPELL_TYPE_POTION:
      if (IS_AFFECTED(ch, AFF_INFRAVISION))
	return;
      spell_infravision(level, ch, ch, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj)
	return;
      if (!victim)
	victim = ch;
      if (IS_AFFECTED(victim, AFF_INFRAVISION))
	return;
      spell_infravision(level, ch, victim, 0);
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj)
	return;
      if (IS_AFFECTED(victim, AFF_INFRAVISION))
	return;
      spell_infravision(level, ch, victim, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (victim != ch)
	  if (!(IS_AFFECTED(victim, AFF_INFRAVISION)))
	    spell_infravision(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in infravision!");
      break;
  }

}

void cast_true_seeing(char level, struct char_data *ch, const char *arg, int type,
		      struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      if (IS_AFFECTED(victim, AFF_TRUE_SIGHT)) {
	cprintf(ch, "Nothing seems to happen.\r\n");
	return;
      }
      spell_true_seeing(level, ch, victim, 0);
      break;
    case SPELL_TYPE_POTION:
      if (IS_AFFECTED(ch, AFF_TRUE_SIGHT))
	return;
      spell_true_seeing(level, ch, ch, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj)
	return;
      if (!victim)
	victim = ch;
      if (IS_AFFECTED(victim, AFF_TRUE_SIGHT))
	return;
      spell_true_seeing(level, ch, victim, 0);
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj)
	return;
      if (IS_AFFECTED(victim, AFF_TRUE_SIGHT))
	return;
      spell_true_seeing(level, ch, victim, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (victim != ch)
	  if (!(IS_AFFECTED(victim, AFF_TRUE_SIGHT)))
	    spell_true_seeing(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in true_seeing!");
      break;
  }

}

void cast_light(char level, struct char_data *ch, const char *arg, int type, struct char_data *victim,
		struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_light(level, ch, ch, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj)
	return;
      spell_calm(level, ch, ch, 0);
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj)
	return;
      spell_calm(level, ch, ch, 0);
      break;
    default:
      log_error("Serious screw-up in light!");
      break;
  }
}

void cast_cont_light(char level, struct char_data *ch, const char *arg, int type,
		     struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_cont_light(level, ch, ch, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj)
	return;
      spell_cont_light(level, ch, ch, 0);
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj)
	return;
      spell_cont_light(level, ch, ch, 0);
      break;
    default:
      log_error("Serious screw-up in continual light!");
      break;
  }
}

void cast_calm(char level, struct char_data *ch, const char *arg, int type, struct char_data *victim,
	       struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_calm(level, ch, victim, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj)
	return;
      if (!victim)
	victim = ch;
      spell_calm(level, ch, victim, 0);
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj)
	return;
      if (!victim)
	victim = ch;
      spell_calm(level, ch, ch, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	spell_calm(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in continual light!");
      break;
  }
}

void cast_water_breath(char level, struct char_data *ch, const char *arg, int type,
		       struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_water_breath(level, ch, victim, 0);
      break;
    case SPELL_TYPE_POTION:
      spell_water_breath(level, ch, victim, 0);
      break;
    case SPELL_TYPE_WAND:
      spell_water_breath(level, ch, victim, 0);
      break;

    default:
      log_error("Serious screw-up in water breath");
      break;
  }
}

void cast_flying(char level, struct char_data *ch, const char *arg, int type,
		 struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  if (MOUNTED(ch)) {
    cprintf(ch, "Not while you are mounted!\r\n");
    return;
  }
  if (MOUNTED(victim)) {
    cprintf(ch, "Not while they are mounted!\r\n");
    return;
  }
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_fly(level, ch, victim, 0);
      break;
    case SPELL_TYPE_POTION:
      spell_fly(level, ch, victim, 0);
      break;
    case SPELL_TYPE_WAND:
      spell_fly(level, ch, victim, 0);
      break;

    default:
      log_error("Serious screw-up in fly");
      break;
  }
}

void cast_goodberry(char level, struct char_data *ch, const char *arg, int type,
		    struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_goodberry(level, ch, ch, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj)
	return;
      spell_goodberry(level, ch, ch, 0);
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj)
	return;
      spell_goodberry(level, ch, ch, 0);
      break;
    default:
      log_error("Serious screw-up in continual light!");
      break;
  }
}

void cast_cure_critic(char level, struct char_data *ch, const char *arg, int type,
		      struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_cure_critic(level, ch, victim, 0);
      break;
    case SPELL_TYPE_POTION:
      spell_cure_critic(level, ch, ch, 0);
      break;
    case SPELL_TYPE_WAND:
      if (!victim)
	victim = ch;
      spell_cure_critic(level, ch, victim, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (victim != ch)
	  spell_cure_critic(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in cure critic!");
      break;

  }
}

void cast_cure_light(char level, struct char_data *ch, const char *arg, int type,
		     struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_cure_light(level, ch, victim, 0);
      break;
    case SPELL_TYPE_POTION:
      spell_cure_light(level, ch, ch, 0);
      break;
    case SPELL_TYPE_WAND:
      if (!victim)
	victim = ch;
      spell_cure_light(level, ch, victim, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (victim != ch)
	  spell_cure_light(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in cure light!");
      break;
  }
}

void cast_cure_serious(char level, struct char_data *ch, const char *arg, int type,
		       struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_cure_serious(level, ch, victim, 0);
      break;
    case SPELL_TYPE_POTION:
      spell_cure_serious(level, ch, ch, 0);
      break;
    case SPELL_TYPE_WAND:
      if (!victim)
	victim = ch;
      spell_cure_serious(level, ch, victim, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (victim != ch)
	  spell_cure_serious(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in cure serious!");
      break;
  }
}

void cast_refresh(char level, struct char_data *ch, const char *arg, int type,
		  struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_refresh(level, ch, victim, 0);
      break;
    case SPELL_TYPE_POTION:
      spell_refresh(level, ch, ch, 0);
      break;
    case SPELL_TYPE_WAND:
      if (!victim)
	victim = ch;
      spell_refresh(level, ch, victim, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (victim != ch)
	  spell_refresh(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in refresh!");
      break;
  }
}

void cast_second_wind(char level, struct char_data *ch, const char *arg, int type,
		      struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_second_wind(level, ch, victim, 0);
      break;
    case SPELL_TYPE_POTION:
      spell_second_wind(level, ch, ch, 0);
      break;
    case SPELL_TYPE_WAND:

    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (victim != ch)
	  spell_second_wind(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in second_wind!");
      break;
  }
}

void cast_shield(char level, struct char_data *ch, const char *arg, int type,
		 struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      if (affected_by_spell(victim, SPELL_STONE_SKIN)) {
	cprintf(ch, "Nothing seems to happen.\r\n");
	return;
      }
      spell_shield(level, ch, victim, 0);
      break;
    case SPELL_TYPE_POTION:
      spell_shield(level, ch, ch, 0);
      break;
    case SPELL_TYPE_WAND:
      if (!victim)
	victim = ch;
      spell_shield(level, ch, victim, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (victim != ch)
	  spell_shield(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in shield!");
      break;
  }

}

void cast_curse(char level, struct char_data *ch, const char *arg, int type, struct char_data *victim,
		struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      if (tar_obj)					       /* It is an object */
	spell_curse(level, ch, 0, tar_obj);
      else {						       /* Then it is a PC | NPC */
	spell_curse(level, ch, victim, 0);
      }
      break;
    case SPELL_TYPE_POTION:
      spell_curse(level, ch, ch, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj)					       /* It is an object */
	spell_curse(level, ch, 0, tar_obj);
      else {						       /* Then it is a PC | NPC */
	if (!victim)
	  victim = ch;
	spell_curse(level, ch, victim, 0);
      }
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (victim != ch)
	  spell_curse(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in curse!");
      break;
  }
}

void cast_detect_evil(char level, struct char_data *ch, const char *arg, int type,
		      struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      if (affected_by_spell(victim, SPELL_DETECT_EVIL)) {
	cprintf(victim, "Nothing seems to happen.\r\n");
	return;
      }
      spell_detect_evil(level, ch, victim, 0);
      break;
    case SPELL_TYPE_POTION:
      if (affected_by_spell(ch, SPELL_DETECT_EVIL))
	return;
      spell_detect_evil(level, ch, ch, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (victim != ch)
	  if (!(IS_AFFECTED(victim, AFF_DETECT_EVIL)))
	    spell_detect_evil(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in detect evil!");
      break;
  }
}

void cast_detect_invisibility(char level, struct char_data *ch, const char *arg, int type,
			      struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      if (IS_AFFECTED(victim, AFF_DETECT_INVISIBLE)) {
	cprintf(victim, "Nothing seems to happen.\r\n");
	return;
      }
      spell_detect_invisibility(level, ch, victim, 0);
      break;
    case SPELL_TYPE_POTION:
      if (IS_AFFECTED(ch, AFF_DETECT_INVISIBLE))
	return;
      spell_detect_invisibility(level, ch, ch, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (in_group(ch, victim))
	  if (!(IS_AFFECTED(victim, AFF_DETECT_INVISIBLE)))
	    spell_detect_invisibility(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in detect invisibility!");
      break;
  }
}

void cast_detect_magic(char level, struct char_data *ch, const char *arg, int type,
		       struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      if (affected_by_spell(victim, SPELL_DETECT_MAGIC)) {
	cprintf(victim, "Nothing seems to happen.\r\n");
	return;
      }
      spell_detect_magic(level, ch, victim, 0);
      break;
    case SPELL_TYPE_POTION:
      if (affected_by_spell(ch, SPELL_DETECT_MAGIC))
	return;
      spell_detect_magic(level, ch, ch, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (victim != ch)
	  if (!(IS_AFFECTED(victim, SPELL_DETECT_MAGIC)))
	    spell_detect_magic(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in detect magic!");
      break;
  }
}

void cast_detect_poison(char level, struct char_data *ch, const char *arg, int type,
			struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_detect_poison(level, ch, victim, tar_obj);
      break;
    case SPELL_TYPE_POTION:
      spell_detect_poison(level, ch, ch, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj) {
	spell_detect_poison(level, ch, 0, tar_obj);
	return;
      }
      if (!victim)
	victim = ch;
      spell_detect_poison(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in detect poison!");
      break;
  }
}

void cast_dispel_evil(char level, struct char_data *ch, const char *arg, int type,
		      struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_dispel_evil(level, ch, victim, 0);
      break;
    case SPELL_TYPE_POTION:
      spell_dispel_evil(level, ch, ch, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj)
	return;
      if (!victim)
	victim = ch;
      spell_dispel_evil(level, ch, victim, 0);
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj)
	return;
      spell_dispel_evil(level, ch, victim, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (!in_group(victim, ch))
	  spell_dispel_evil(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in dispel evil!");
      break;
  }
}

void cast_dispel_good(char level, struct char_data *ch, const char *arg, int type,
		      struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_dispel_good(level, ch, victim, 0);
      break;
    case SPELL_TYPE_POTION:
      spell_dispel_good(level, ch, ch, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj)
	return;
      if (!victim)
	victim = ch;
      spell_dispel_good(level, ch, victim, 0);
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj)
	return;
      spell_dispel_good(level, ch, victim, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (!in_group(victim, ch))
	  spell_dispel_good(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in dispel good!");
      break;
  }
}

void cast_faerie_fire(char level, struct char_data *ch, const char *arg, int type,
		      struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_faerie_fire(level, ch, victim, 0);
      break;
    case SPELL_TYPE_POTION:
      spell_faerie_fire(level, ch, ch, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj)
	return;
      if (!victim)
	victim = ch;
      spell_faerie_fire(level, ch, victim, 0);
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj)
	return;
      spell_faerie_fire(level, ch, victim, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (!in_group(victim, ch))
	  spell_faerie_fire(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in dispel good!");
      break;
  }
}

void cast_enchant_weapon(char level, struct char_data *ch, const char *arg, int type,
			 struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_enchant_weapon(level, ch, 0, tar_obj);
      break;

    case SPELL_TYPE_SCROLL:
      if (!tar_obj)
	return;
      spell_enchant_weapon(level, ch, 0, tar_obj);
      break;
    default:
      log_error("Serious screw-up in enchant weapon!");
      break;
  }
}

void cast_enchant_armor(char level, struct char_data *ch, const char *arg, int type,
			struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
/*                      spell_enchant_armor(level, ch, 0,tar_obj);
 * break;
 */
    case SPELL_TYPE_SCROLL:
/*                      if(!tar_obj) return;
 * spell_enchant_armor(level, ch, 0,tar_obj);
 * break;
 */
    default:
      log_error("Serious screw-up in enchant armor!");
      break;
  }
}

void cast_heal(char level, struct char_data *ch, const char *arg, int type, struct char_data *victim,
	       struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      act("$n heals $N.", FALSE, ch, 0, victim, TO_NOTVICT);
      act("You heal $N.", FALSE, ch, 0, victim, TO_CHAR);
      spell_heal(level, ch, victim, 0);
      break;
    case SPELL_TYPE_POTION:
      spell_heal(level, ch, ch, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (victim != ch)
	  spell_heal(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in heal!");
      break;
  }
}

void cast_invisibility(char level, struct char_data *ch, const char *arg, int type,
		       struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      if (tar_obj) {
	if (IS_SET(tar_obj->obj_flags.extra_flags, ITEM_INVISIBLE))
	  cprintf(ch, "Nothing new seems to happen.\r\n");
	else
	  spell_invisibility(level, ch, 0, tar_obj);
      } else {						       /* victim */
	if (IS_AFFECTED(victim, AFF_INVISIBLE))
	  cprintf(ch, "Nothing new seems to happen.\r\n");
	else
	  spell_invisibility(level, ch, victim, 0);
      }
      break;
    case SPELL_TYPE_POTION:
      if (!IS_AFFECTED(ch, AFF_INVISIBLE))
	spell_invisibility(level, ch, ch, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj) {
	if (!(IS_SET(tar_obj->obj_flags.extra_flags, ITEM_INVISIBLE)))
	  spell_invisibility(level, ch, 0, tar_obj);
      } else {						       /* victim */
	if (!victim)
	  victim = ch;

	if (!(IS_AFFECTED(victim, AFF_INVISIBLE)))
	  spell_invisibility(level, ch, victim, 0);
      }
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj) {
	if (!(IS_SET(tar_obj->obj_flags.extra_flags, ITEM_INVISIBLE)))
	  spell_invisibility(level, ch, 0, tar_obj);
      } else {						       /* victim */
	if (!(IS_AFFECTED(victim, AFF_INVISIBLE)))
	  spell_invisibility(level, ch, victim, 0);
      }
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (victim != ch)
	  if (!(IS_AFFECTED(victim, AFF_INVISIBLE)))
	    spell_invisibility(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in invisibility!");
      break;
  }
}

void cast_locate_object(char level, struct char_data *ch, const char *arg, int type,
			struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_locate_object(level, ch, 0, (char *)tar_obj);
      break;
    default:
      log_error("Serious screw-up in locate object!");
      break;
  }
}

void cast_poison(char level, struct char_data *ch, const char *arg, int type,
		 struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_poison(level, ch, victim, tar_obj);
      break;
    case SPELL_TYPE_POTION:
      spell_poison(level, ch, ch, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (victim != ch)
	  spell_poison(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in poison!");
      break;
  }
}

void cast_protection_from_evil(char level, struct char_data *ch, const char *arg, int type,
			       struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_protection_from_evil(level, ch, victim, 0);
      break;
    case SPELL_TYPE_POTION:
      spell_protection_from_evil(level, ch, ch, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj)
	return;
      if (!victim)
	victim = ch;
      spell_protection_from_evil(level, ch, victim, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (victim != ch)
	  spell_protection_from_evil(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in protection from evil!");
      break;
  }
}

void cast_remove_curse(char level, struct char_data *ch, const char *arg, int type,
		       struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_remove_curse(level, ch, victim, tar_obj);
      break;
    case SPELL_TYPE_POTION:
      spell_remove_curse(level, ch, ch, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj) {
	spell_remove_curse(level, ch, 0, tar_obj);
	return;
      }
      if (!victim)
	victim = ch;
      spell_remove_curse(level, ch, victim, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (victim != ch)
	  spell_remove_curse(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in remove curse!");
      break;
  }
}

void cast_remove_poison(char level, struct char_data *ch, const char *arg, int type,
			struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_remove_poison(level, ch, victim, tar_obj);
      break;
    case SPELL_TYPE_POTION:
      spell_remove_poison(level, ch, ch, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (victim != ch)
	  spell_remove_poison(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in remove poison!");
      break;
  }
}

void cast_remove_paralysis(char level, struct char_data *ch, const char *arg, int type,
			   struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_remove_paralysis(level, ch, victim, tar_obj);
      break;
    case SPELL_TYPE_POTION:
      spell_remove_paralysis(level, ch, ch, 0);
      break;
    case SPELL_TYPE_WAND:
      if (!victim)
	victim = ch;
      spell_remove_paralysis(level, ch, victim, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (victim != ch)
	  spell_remove_paralysis(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in remove paralysis!");
      break;
  }
}

void cast_sanctuary(char level, struct char_data *ch, const char *arg, int type,
		    struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_sanctuary(level, ch, victim, 0);
      break;

    case SPELL_TYPE_WAND:
    case SPELL_TYPE_POTION:
      spell_sanctuary(level, ch, ch, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj)
	return;
      if (!victim)
	victim = ch;
      spell_sanctuary(level, ch, victim, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (victim != ch)
	  spell_sanctuary(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in sanctuary!");
      break;
  }
}

void cast_fireshield(char level, struct char_data *ch, const char *arg, int type,
		     struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_fireshield(level, ch, victim, 0);
      break;

    case SPELL_TYPE_WAND:
    case SPELL_TYPE_POTION:
      spell_sanctuary(level, ch, ch, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj)
	return;
      if (!victim)
	victim = ch;
      spell_fireshield(level, ch, victim, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (victim != ch)
	  spell_fireshield(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in fireshield!");
      break;
  }
}

void cast_sleep(char level, struct char_data *ch, const char *arg, int type, struct char_data *victim,
		struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_sleep(level, ch, victim, 0);
      break;
    case SPELL_TYPE_POTION:
      spell_sleep(level, ch, ch, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj)
	return;
      if (!victim)
	victim = ch;
      spell_sleep(level, ch, victim, 0);
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj)
	return;
      spell_sleep(level, ch, victim, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (victim != ch)
	  spell_sleep(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in sleep!");
      break;
  }
}

void cast_strength(char level, struct char_data *ch, const char *arg, int type,
		   struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_strength(level, ch, victim, 0);
      break;
    case SPELL_TYPE_POTION:
      spell_strength(level, ch, ch, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj)
	return;
      if (!victim)
	victim = ch;
      spell_strength(level, ch, victim, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (victim != ch)
	  spell_strength(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in strength!");
      break;
  }
}

void cast_ventriloquate(char level, struct char_data *ch, const char *arg, int type,
			struct char_data *victim, struct obj_data *tar_obj)
{
  struct char_data                       *tmp_ch = NULL;
  char                                    buf1[MAX_STRING_LENGTH] = "\0\0\0";
  char                                    buf2[MAX_STRING_LENGTH] = "\0\0\0";
  char                                    buf3[MAX_STRING_LENGTH] = "\0\0\0";

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  if (type != SPELL_TYPE_SPELL) {
    log_error("Attempt to ventriloquate by non-cast-spell.");
    return;
  }
  for (; *arg && (*arg == ' '); arg++);
  if (tar_obj) {
    sprintf(buf1, "The %s says '%s'\r\n", fname(tar_obj->name), arg);
    sprintf(buf2, "Someone makes it sound like the %s says '%s'.\r\n",
	    fname(tar_obj->name), arg);
  } else {
    sprintf(buf1, "%s says '%s'\r\n", GET_NAME(victim), arg);
    sprintf(buf2, "Someone makes it sound like %s says '%s'\r\n", GET_NAME(victim), arg);
  }

  sprintf(buf3, "Someone says, '%s'\r\n", arg);

  for (tmp_ch = real_roomp(ch->in_room)->people; tmp_ch; tmp_ch = tmp_ch->next_in_room) {

    if ((tmp_ch != ch) && (tmp_ch != victim)) {
      if (saves_spell(tmp_ch, SAVING_SPELL))
	cprintf(tmp_ch, "%s", buf2);
      else
	cprintf(tmp_ch, "%s", buf1);
    } else {
      if (tmp_ch == victim)
	cprintf(victim, "%s", buf3);
    }
  }
}

void cast_word_of_recall(char level, struct char_data *ch, const char *arg, int type,
			 struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_word_of_recall(level, ch, ch, 0);
      break;
    case SPELL_TYPE_POTION:
      spell_word_of_recall(level, ch, ch, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj)
	return;
      if (!victim)
	victim = ch;
      spell_word_of_recall(level, ch, victim, 0);
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj)
	return;
      spell_word_of_recall(level, ch, victim, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (victim != ch)
	  spell_word_of_recall(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in word of recall!");
      break;
  }
}

void cast_summon(char level, struct char_data *ch, const char *arg, int type,
		 struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_summon(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in summon!");
      break;
  }
}

void cast_charm_monster(char level, struct char_data *ch, const char *arg, int type,
			struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_charm_monster(level, ch, victim, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (!victim)
	return;
      spell_charm_monster(level, ch, victim, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (!in_group(victim, ch))
	  spell_charm_monster(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in charm monster!");
      break;
  }
}

void cast_sense_life(char level, struct char_data *ch, const char *arg, int type,
		     struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_sense_life(level, ch, ch, 0);
      break;
    case SPELL_TYPE_POTION:
      spell_sense_life(level, ch, ch, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (victim != ch)
	  spell_sense_life(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in sense life!");
      break;
  }
}

void cast_identify(char level, struct char_data *ch, const char *arg, int type,
		   struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SCROLL:
      spell_identify(level, ch, victim, tar_obj);
      break;
    default:
      log_error("Serious screw-up in identify!");
      break;
  }
}

#define MAX_BREATHS 3
struct pbreath {
  int                                     vnum,
                                          spell[MAX_BREATHS];
} breath_potions[] = {

  {
    3970, {
    201, 0}
  }, {
    3971, {
    202, 0}
  }, {
    3972, {
    203, 0}
  }, {
    3973, {
    204, 0}
  }, {
    3974, {
    205, 0}
  }, {
0},};

void cast_dragon_breath(char level, struct char_data *ch, const char *arg, int type,
			struct char_data *victim, struct obj_data *tar_obj)
{
  struct pbreath                         *scan = NULL;
  int                                     i = 0;
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  for (scan = breath_potions;
       scan->vnum && scan->vnum != obj_index[tar_obj->item_number].virtual; scan++);
  if (scan->vnum == 0) {
    char                                    buf[MAX_STRING_LENGTH];

    cprintf(ch, "Hey, this potion isn't in my list!\r\n");
    sprintf(buf, "unlisted breath potion %s %d", tar_obj->short_description,
	    obj_index[tar_obj->item_number].virtual);
    log_error(buf);
    return;
  }
  for (i = 0; i < MAX_BREATHS && scan->spell[i]; i++) {
    if (!affected_by_spell(ch, scan->spell[i])) {
      af.type = scan->spell[i];
      af.duration = 1 + dice(1, 2);
      if (GET_CON(ch) < 4) {
	cprintf(ch,
		"You are too weak to stomach the potion and spew it all over the floor.\r\n");
	act("$n gags and pukes glowing goop all over the floor.", FALSE, ch, 0, ch, TO_NOTVICT);
	break;
      }
      if (level > MIN(GET_CON(ch) - 1, GetMaxLevel(ch))) {
	cprintf(ch, "!GACK! You are too weak to handle the full power of the potion.\r\n");
	act("$n gags and flops around on the floor a bit.", FALSE, ch, 0, ch, TO_NOTVICT);
	level = MIN(GET_CON(ch) - 1, GetMaxLevel(ch));
      }
      af.modifier = -level;
      af.location = APPLY_CON;
      af.bitvector = 0;
      affect_to_char(ch, &af);
      cprintf(ch, "You feel powerful forces build within your stomach...\r\n");
    }
  }
}

void cast_fire_breath(char level, struct char_data *ch, const char *arg, int type,
		      struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_fire_breath(level, ch, victim, 0);
      break;						       /* It's a spell.. But people can'c cast it! */
    default:
      log_error("Serious screw-up in firebreath!");
      break;
  }
}

void cast_frost_breath(char level, struct char_data *ch, const char *arg, int type,
		       struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_frost_breath(level, ch, victim, 0);
      break;						       /* It's a spell.. But people can'c cast it! */
    default:
      log_error("Serious screw-up in frostbreath!");
      break;
  }
}

void cast_acid_breath(char level, struct char_data *ch, const char *arg, int type,
		      struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_acid_breath(level, ch, victim, 0);
      break;						       /* It's a spell.. But people can'c cast it! */
    default:
      log_error("Serious screw-up in acidbreath!");
      break;
  }
}

void cast_gas_breath(char level, struct char_data *ch, const char *arg, int type,
		     struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_gas_breath(level, ch, victim, 0);
      break;
      /*
       * THIS ONE HURTS!! 
       */
    default:
      log_error("Serious screw-up in gasbreath!");
      break;
  }
}

void cast_lightning_breath(char level, struct char_data *ch, const char *arg, int type,
			   struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_lightning_breath(level, ch, victim, 0);
      break;						       /* It's a spell.. But people can'c cast it! */
    default:
      log_error("Serious screw-up in lightningbreath!");
      break;
  }
}

void cast_knock(char level, struct char_data *ch, const char *arg, int type, struct char_data *victim,
		struct obj_data *tar_obj)
{
  int                                     door = 0;
  int                                     other_room = 0;
  char                                    dir[MAX_INPUT_LENGTH] = "\0\0\0";
  char                                    buf[MAX_STRING_LENGTH] = "\0\0\0";
  char                                    otype[MAX_INPUT_LENGTH] = "\0\0\0";
  struct room_direction_data             *back = NULL;
  struct obj_data                        *obj = NULL;
  struct char_data                       *this_victim = NULL;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:{

	argument_interpreter(arg, otype, dir);

	if (!otype) {
	  cprintf(ch, "Knock on what?\r\n");
	  return;
	}
	if (generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &this_victim, &obj)) {
	  if (obj->obj_flags.type_flag != ITEM_CONTAINER) {
	    sprintf(buf, " %s is not a container.\r\n ", obj->name);
	  } else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED)) {
	    sprintf(buf, " Silly! %s isn't even closed!\r\n ", obj->name);
	  } else if (obj->obj_flags.value[2] < 0) {
	    sprintf(buf, "%s doesn't have a lock...\r\n", obj->name);
	  } else if (!IS_SET(obj->obj_flags.value[1], CONT_LOCKED)) {
	    sprintf(buf, "Hehe.. %s wasn't even locked.\r\n", obj->name);
	  } else
/*
 * if (IS_SET(obj->obj_flags.value[1], CONT_PICKPROOF)) 
 * {
 * sprintf(buf,"%s resists your magic.\r\n",obj->name);
 * }
 * else
 */
	  {
	    REMOVE_BIT(obj->obj_flags.value[1], CONT_LOCKED);
	    sprintf(buf, "<Click>\r\n");
	    act("$n magically opens $p", FALSE, ch, obj, 0, TO_ROOM);
	  }
	  cprintf(ch, "%s", buf);
	  return;
	} else if ((door = find_door(ch, otype, dir)) >= 0) {
	  if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
	    cprintf(ch, "That's absurd.\r\n");
	  else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
	    cprintf(ch, "You realize that the door is already open.\r\n");
	  else if (EXIT(ch, door)->key < 0)
	    cprintf(ch, "You can't seem to spot any lock to knock.\r\n");
	  else if (!IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))
	    cprintf(ch, "Oh.. it wasn't locked at all.\r\n");
	  else
/*
 * if (IS_SET(EXIT(ch, door)->exit_info, EX_PICKPROOF))
 * cprintf(ch, "You seem to be unable to knock this...\r\n");
 * else
 */
	  {
	    REMOVE_BIT(EXIT(ch, door)->exit_info, EX_LOCKED);
	    if (EXIT(ch, door)->keyword)
	      act("$n magically opens the lock of the $F.", 0, ch, 0,
		  EXIT(ch, door)->keyword, TO_ROOM);
	    else
	      act("$n magically opens the lock.", TRUE, ch, 0, 0, TO_ROOM);
	    cprintf(ch, "The lock quickly yields to your skills.\r\n");
	    if ((other_room = EXIT(ch, door)->to_room) != NOWHERE)
	      if ((back = real_roomp(other_room)->dir_option[rev_dir[door]]))
		if (back->to_room == ch->in_room)
		  REMOVE_BIT(back->exit_info, EX_LOCKED);
	  }
	}
      }
      break;
    default:
      log_error("serious error in Knock.");
      break;
  }
}

void cast_know_alignment(char level, struct char_data *ch, const char *arg, int type,
			 struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_know_alignment(level, ch, victim, tar_obj);
      break;
    case SPELL_TYPE_POTION:
      spell_know_alignment(level, ch, ch, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (!victim)
	victim = ch;
      spell_know_alignment(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in know alignment!");
      break;
  }
}

void cast_weakness(char level, struct char_data *ch, const char *arg, int type,
		   struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_weakness(level, ch, victim, 0);
      break;
    case SPELL_TYPE_POTION:
      spell_weakness(level, ch, ch, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj)
	return;
      if (!victim)
	victim = ch;
      spell_weakness(level, ch, victim, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (!in_group(victim, ch))
	  spell_weakness(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in weakness!");
      break;
  }
}

void cast_dispel_magic(char level, struct char_data *ch, const char *arg, int type,
		       struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_dispel_magic(level, ch, victim, tar_obj);
      break;
    case SPELL_TYPE_POTION:
      spell_dispel_magic(level, ch, ch, 0);
      break;
    case SPELL_TYPE_SCROLL:
/*
 * if(tar_obj) {
 * spell_dispel_magic(level, ch, 0, tar_obj);
 * return;
 * }
 */
      if (!victim)
	victim = ch;
      spell_dispel_magic(level, ch, victim, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (victim != ch)
	  spell_dispel_magic(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in dispel magic");
      break;
  }
}

void cast_animate_dead(char level, struct char_data *ch, const char *arg, int type,
		       struct char_data *victim, struct obj_data *tar_obj)
{
  struct obj_data                        *i = NULL;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
      if (tar_obj) {
	if (IS_CORPSE(tar_obj)) {
	  spell_animate_dead(level, ch, 0, tar_obj);
	} else {
	  cprintf(ch, "That's not a corpse!\r\n");
	  return;
	}
      } else {
	cprintf(ch, "That isn't a corpse!\r\n");
	return;
      }
      break;
    case SPELL_TYPE_POTION:
      cprintf(ch, "Your body revolts against the magic liquid.\r\n");
      ch->points.hit = 0;
      break;
    case SPELL_TYPE_STAFF:
      for (i = real_roomp(ch->in_room)->contents; i; i = i->next_content) {
	if (GET_ITEM_TYPE(i) == ITEM_CONTAINER && i->obj_flags.value[3]) {
	  spell_animate_dead(level, ch, 0, i);
	}
      }
      break;
    default:
      log_error("Serious screw-up in animate_dead!");
      break;
  }
}

void cast_succor(char level, struct char_data *ch, const char *arg, int type,
		 struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
      spell_succor(level, ch, 0, 0);
  }

}

void cast_paralyze(char level, struct char_data *ch, const char *arg, int type,
		   struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_paralyze(level, ch, victim, 0);
      break;
    case SPELL_TYPE_POTION:
      spell_paralyze(level, ch, ch, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj)
	return;
      if (!victim)
	victim = ch;
      spell_paralyze(level, ch, victim, 0);
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj)
	return;
      spell_paralyze(level, ch, victim, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (victim != ch)
	  spell_paralyze(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in paralyze");
      break;
  }
}

void cast_fear(char level, struct char_data *ch, const char *arg, int type, struct char_data *victim,
	       struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_fear(level, ch, victim, 0);
      break;
    case SPELL_TYPE_POTION:
      spell_fear(level, ch, ch, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj)
	return;
      if (!victim)
	victim = ch;
      spell_fear(level, ch, victim, 0);
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj)
	return;
      if (!victim)
	victim = ch;
      spell_fear(level, ch, victim, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (!in_group(victim, ch))
	  spell_fear(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in fear");
      break;
  }
}

void cast_turn(char level, struct char_data *ch, const char *arg, int type, struct char_data *victim,
	       struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_turn(level, ch, victim, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj)
	return;
      if (!victim)
	victim = ch;
      spell_turn(level, ch, victim, 0);
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj)
	return;
      if (!victim)
	victim = ch;
      spell_turn(level, ch, victim, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim; victim = victim->next_in_room)
	if (!in_group(victim, ch))
	  spell_turn(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in turn");
      break;
  }
}

void cast_faerie_fog(char level, struct char_data *ch, const char *arg, int type,
		     struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
      spell_faerie_fog(level, ch, 0, 0);
      break;
    default:
      log_error("Serious screw-up in faerie fog!");
      break;
  }
}

const struct PolyType                   PolyList[] = {
/* name         level #number */
  {"", 0, 200},
  {"goblin", 10, 201},
  {"orc", 10, 202},
  {"badger", 10, 203},
  {"beetle", 10, 204},
  {"troglodyte", 10, 205},
  {"cow", 10, 206},
  {"dog", 10, 207},
  {"frog", 10, 208},
  {"hobgoblin", 11, 209},
  {"spider", 11, 210},
  {"gnoll", 11, 211},
  {"ant", 11, 213},
  {"lemure", 12, 216},
  {"toad", 12, 217},
  {"stirge", 12, 218},
  {"fighter", 12, 220},
  {"bugbear", 13, 221},
  {"ghoul", 13, 223},
  {"lizard", 13, 224},
  {"rat", 13, 225},
  {"ogre", 14, 226},
  {"tick", 14, 228},
  {"weazal", 15, 229},
  {"black dragon", 16, 230},
  {"ape", 16, 231},
  {"blue dragon", 17, 233},
  {"gargoyle", 17, 235},
  {"ghast", 17, 236},
  {"lupan", 18, 237},
  {"owlbear", 18, 238},
  {"shadow", 18, 239},
  {"displacer", 19, 242},
  {"white dragon", 20, 243},
  {"hill giant", 20, 261},
  {"troll", 20, 262},
  {"red dragon", 51, 5030}
};

#define LAST_POLY_MOB 37

void cast_poly_self(char level, struct char_data *ch, const char *arg, int type,
		    struct char_data *victim, struct obj_data *tar_obj)
{
  char                                    buffer[40] = "\0\0\0";
  int                                     mobn = 0;
  int                                     X = LAST_POLY_MOB;
  int                                     found = FALSE;
  struct char_data                       *mob = NULL;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  /*
   * one_argument(arg, buffer); 
   */
  only_argument(arg, buffer);
  if (IS_NPC(ch)) {
    cprintf(ch, "You don't really want to do that.\r\n");
    return;
  }
  switch (type) {
    case SPELL_TYPE_SPELL:
      {
	while (!found) {
	  if (PolyList[X].level > level) {
	    X--;
	  } else {
	    if (number(0, 99) < 10 || !str_cmp(PolyList[X].name, buffer)) {
	      mobn = PolyList[X].number;
	      found = TRUE;
	    } else {
	      X--;
	    }
	    if (X < 0)
	      break;
	  }
	}

	if (!found) {
	  for (X = LAST_POLY_MOB; X >= 0 && PolyList[X].level > level; X--);
	  mobn = PolyList[number(0, X)].number;
	}
	{
	  mob = read_mobile(mobn, VIRTUAL);
	  if (mob) {
	    spell_poly_self(level, ch, mob, 0);
	  } else {
	    cprintf(ch, "You couldn't summon an image of that creature\r\n");
	  }
	  return;
	}

      }
      break;

    default:{
	log_error("Problem in poly_self");
      }
      break;
  }
}

#define NUT_CRACKED 1131

void cast_shelter(char level, struct char_data *ch, const char *arg, int type,
		  struct char_data *victim, struct obj_data *tar_obj)
{
  struct obj_data                        *sac = NULL;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  if (!ch->equipment[HOLD]) {
    cprintf(ch, " You must be holding the component for this spell.\r\n");
    return;
  }
  sac = unequip_char(ch, HOLD);
  if (sac) {
    obj_to_char(sac, ch);
    if (ObjVnum(sac) != NUT_CRACKED) {
      cprintf(ch, "That is not the correct item.\r\n");
      return;
    }
  } else {
    cprintf(ch, "You must be holding the component for this spell.\r\n");
    return;
  }

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_shelter(level, ch, ch, sac);
      break;
    default:
      log_error("serious screw-up in shelter.");
      break;
  }
}

#define LONG_SWORD   3032
#define SHIELD       5045
#define BOAT         6101
#define BAG          3017
#define WATER_BARREL 3005
#define BREAD        3010
#define PAPER        6
#define PEN          5
#define SIGN         35
#define GOOD_WALNUT  1130
#define BAD_WALNUT   1132

void cast_minor_creation(char level, struct char_data *ch, const char *arg, int type,
			 struct char_data *victim, struct obj_data *tar_obj)
{
  char                                    buffer[40] = "\0\0\0";
  int                                     obj = 0;
  struct obj_data                        *o = NULL;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  one_argument(arg, buffer);

  if (!str_cmp(buffer, "sword")) {
    obj = LONG_SWORD;
  } else if (!str_cmp(buffer, "shield")) {
    obj = SHIELD;
  } else if (!str_cmp(buffer, "boat")) {
    obj = BOAT;
  } else if (!str_cmp(buffer, "raft")) {
    obj = BOAT;
  } else if (!str_cmp(buffer, "bag")) {
    obj = BAG;
  } else if (!str_cmp(buffer, "barrel")) {
    obj = WATER_BARREL;
  } else if (!str_cmp(buffer, "bread")) {
    obj = BREAD;
  } else if (!str_cmp(buffer, "walnut")) {
    if (number(0, 99) < 2)
      obj = GOOD_WALNUT;
    else
      obj = BAD_WALNUT;
  } else if (!str_cmp(buffer, "sign")) {
    obj = SIGN;
  } else if (!str_cmp(buffer, "paper")) {
    obj = PAPER;
  } else if (!str_cmp(buffer, "pen")) {
    obj = PEN;
  } else {
    cprintf(ch, "There is nothing of that available\r\n");
    return;
  }

  o = read_object(obj, VIRTUAL);
  if (!o) {
    cprintf(ch, "There is nothing of that available\r\n");
    return;
  }
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
      spell_minor_create(level, ch, 0, o);
      break;
    default:
      log_error("serious screw-up in minor_create.");
      break;
  }

}

#define FIRE_ELEMENTAL  10
#define WATER_ELEMENTAL 11
#define AIR_ELEMENTAL   13
#define EARTH_ELEMENTAL 12

#define RED_STONE       1120
#define PALE_BLUE_STONE 1124
#define CLEAR_STONE     1125
#define GREY_STONE      1126

void cast_conjure_elemental(char level, struct char_data *ch, const char *arg, int type,
			    struct char_data *victim, struct obj_data *tar_obj)
{
  char                                    buffer[40] = "\0\0\0";
  int                                     mob = 0;
  int                                     obj = 0;
  struct obj_data                        *sac= NULL;
  struct char_data                       *el= NULL;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  one_argument(arg, buffer);

  if (!str_cmp(buffer, "fire")) {
    mob = FIRE_ELEMENTAL;
    obj = RED_STONE;
  } else if (!str_cmp(buffer, "water")) {
    mob = WATER_ELEMENTAL;
    obj = PALE_BLUE_STONE;
  } else if (!str_cmp(buffer, "air")) {
    mob = AIR_ELEMENTAL;
    obj = CLEAR_STONE;
  } else if (!str_cmp(buffer, "earth")) {
    mob = EARTH_ELEMENTAL;
    obj = GREY_STONE;
  } else {
    cprintf(ch, "There are no elementals of that type available\r\n");
    return;
  }
  if (!ch->equipment[HOLD]) {
    cprintf(ch, " You must be holding the correct stone\r\n");
    return;
  }
  sac = unequip_char(ch, HOLD);
  if (sac) {
    obj_to_char(sac, ch);
    if (ObjVnum(sac) != obj) {
      cprintf(ch, "You must have the correct item to sacrifice.\r\n");
      return;
    }
    el = read_mobile(mob, VIRTUAL);
    if (!el) {
      cprintf(ch, "There are no elementals of that type available\r\n");
      return;
    }
  } else {
    cprintf(ch, "You must be holding the correct item to sacrifice.\r\n");
    return;
  }

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
      spell_conjure_elemental(level, ch, el, sac);
      break;
    default:
      log_error("serious screw-up in conjure_elemental.");
      break;
  }
}

#define DEMON_TYPE_I     20
#define DEMON_TYPE_II    21
#define DEMON_TYPE_III   22
#define DEMON_TYPE_IV    23
#define DEMON_TYPE_V     24
#define DEMON_TYPE_VI    25

#define SWORD_ANCIENTS   25000
#define SHADOWSHIV       25014
#define FIRE_SWORD       25015
#define SILVER_TRIDENT   25016
#define JEWELLED_DAGGER  25019
#define SWORD_SHARPNESS  25017

void cast_cacaodemon(char level, struct char_data *ch, const char *arg, int type,
		     struct char_data *victim, struct obj_data *tar_obj)
{
  char                                    buffer[40] = "\0\0\0";
  int                                     mob = 0;
  int                                     obj = 0;
  struct obj_data                        *sac = NULL;
  struct char_data                       *el = NULL;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  one_argument(arg, buffer);

  if (!str_cmp(buffer, "one")) {
    mob = DEMON_TYPE_I;
    obj = SWORD_SHARPNESS;
  } else if (!str_cmp(buffer, "two")) {
    mob = DEMON_TYPE_II;
    obj = JEWELLED_DAGGER;
  } else if (!str_cmp(buffer, "three")) {
    mob = DEMON_TYPE_III;
    obj = SILVER_TRIDENT;
  } else if (!str_cmp(buffer, "four")) {
    mob = DEMON_TYPE_IV;
    obj = FIRE_SWORD;
  } else if (!str_cmp(buffer, "five")) {
    mob = DEMON_TYPE_V;
    obj = SHADOWSHIV;
  } else if (!str_cmp(buffer, "six")) {
    mob = DEMON_TYPE_VI;
    obj = SWORD_ANCIENTS;
  } else {
    cprintf(ch, "There are no demons of that type available\r\n");
    return;
  }
  if (!ch->equipment[WIELD]) {
    cprintf(ch, " You must be wielding the correct item\r\n");
    return;
  }
  sac = unequip_char(ch, WIELD);
  if (sac) {
    obj_to_char(sac, ch);
    if (ObjVnum(sac) != obj) {
      cprintf(ch, "You must have the correct item to sacrifice.\r\n");
      return;
    }
    el = read_mobile(mob, VIRTUAL);
    if (!el) {
      cprintf(ch, "There are no demons of that type available\r\n");
      return;
    }
  } else {
    cprintf(ch, "You must be holding the correct item to sacrifice.\r\n");
    return;
  }

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
      spell_cacaodemon(level, ch, el, sac);
      break;
    default:
      log_error("serious screw-up in conjure_elemental.");
      break;
  }

}

void cast_mon_sum1(char level, struct char_data *ch, const char *arg, int type,
		   struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
      spell_Create_Monster(5, ch, 0, 0);
      break;
    default:
      log_error("Serious screw-up in monster_summoning_1");
      break;
  }
}

void cast_mon_sum2(char level, struct char_data *ch, const char *arg, int type,
		   struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
      spell_Create_Monster(7, ch, 0, 0);
      break;
    default:
      log_error("Serious screw-up in monster_summoning_1");
      break;
  }
}

void cast_mon_sum3(char level, struct char_data *ch, const char *arg, int type,
		   struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
      spell_Create_Monster(9, ch, 0, 0);
      break;
    default:
      log_error("Serious screw-up in monster_summoning_1");
      break;
  }
}

void cast_mon_sum4(char level, struct char_data *ch, const char *arg, int type,
		   struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
      spell_Create_Monster(11, ch, 0, 0);
      break;
    default:
      log_error("Serious screw-up in monster_summoning_1");
      break;
  }
}

void cast_mon_sum5(char level, struct char_data *ch, const char *arg, int type,
		   struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
      spell_Create_Monster(13, ch, 0, 0);
      break;
    default:
      log_error("Serious screw-up in monster_summoning_1");
      break;
  }
}

void cast_mon_sum6(char level, struct char_data *ch, const char *arg, int type,
		   struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
      spell_Create_Monster(15, ch, 0, 0);
      break;
    default:
      log_error("Serious screw-up in monster_summoning_1");
      break;
  }
}

void cast_mon_sum7(char level, struct char_data *ch, const char *arg, int type,
		   struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
      spell_Create_Monster(17, ch, 0, 0);
      break;
    default:
      log_error("Serious screw-up in monster_summoning_1");
      break;
  }
}

/*
 * file: spells2.c , Implementation of magic.             Part of DIKUMUD
 * Usage : All the non-offensive magic handling routines.
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */

void cast_fly_group(char level, struct char_data *ch, const char *arg, int type,
		    struct char_data *victim, struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
      spell_fly_group(level, ch, 0, 0);
      break;
    case SPELL_TYPE_POTION:
      spell_fly(level, ch, victim, 0);
      break;
    default:
      log_error("Serious screw-up in fly");
      break;
  }
}

void cast_aid(char level, struct char_data *ch, const char *arg, int type, struct char_data *victim,
	      struct obj_data *tar_obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %d, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), VNULL(arg), type, SAFE_NAME(victim), SAFE_ONAME(tar_obj));

  if (!victim)
    victim = ch;

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
      spell_aid(level, ch, victim, 0);
      break;
    default:
      log_error("serious screw-up in scare.");
      break;
  }
}

/*
 * file: spells.c , handling of magic.                   Part of DIKUMUD
 * Usage : Procedures handling all offensive magic.
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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
#define _SPELLS_C
#include "spells.h"
#include "spell_parser.h"

void cast_armor(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    if (affected_by_spell(tar_ch, SPELL_ARMOR) ||
	affected_by_spell(tar_ch, SPELL_STONE_SKIN)) {
      send_to_char("Nothing seems to happen.\n\r", ch);
      return;
    }
    if (ch != tar_ch)
      act("$N is protected.", FALSE, ch, 0, tar_ch, TO_CHAR);

    spell_armor(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    if (affected_by_spell(ch, SPELL_ARMOR))
      return;
    spell_armor(level, ch, ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj)
      return;
    if (!tar_ch)
      tar_ch = ch;
    if (affected_by_spell(tar_ch, SPELL_ARMOR))
      return;
    spell_armor(level, ch, ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj)
      return;
    if (affected_by_spell(tar_ch, SPELL_ARMOR))
      return;
    spell_armor(level, ch, ch, 0);
    break;
  default:
    log("Serious screw-up in armor!");
    break;
  }
}

void cast_teleport(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_POTION:
  case SPELL_TYPE_SPELL:
    if (!tar_ch)
      tar_ch = ch;
    spell_teleport(level, ch, tar_ch, 0);
    break;

  case SPELL_TYPE_WAND:
    if (!tar_ch)
      tar_ch = ch;
    spell_teleport(level, ch, tar_ch, 0);
    break;

  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_teleport(level, ch, tar_ch, 0);
    break;

  default:
    log("Serious screw-up in teleport!");
    break;
  }
}

void cast_bless(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  struct affected_type af;

  switch (type) {
  case SPELL_TYPE_SPELL:
    if (tar_obj) {		       /* It's an object */
      if (IS_SET(tar_obj->obj_flags.extra_flags, ITEM_BLESS)) {
	send_to_char("Nothing seems to happen.\n\r", ch);
	return;
      }
      spell_bless(level, ch, 0, tar_obj);

    } else {			       /* Then it is a PC | NPC */

      if (affected_by_spell(tar_ch, SPELL_BLESS) ||
	  (GET_POS(tar_ch) == POSITION_FIGHTING)) {
	send_to_char("Nothing seems to happen.\n\r", ch);
	return;
      }
      spell_bless(level, ch, tar_ch, 0);
    }
    break;
  case SPELL_TYPE_POTION:
    if (affected_by_spell(ch, SPELL_BLESS) ||
	(GET_POS(ch) == POSITION_FIGHTING))
      return;
    spell_bless(level, ch, ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) {		       /* It's an object */
      if (IS_SET(tar_obj->obj_flags.extra_flags, ITEM_BLESS))
	return;
      spell_bless(level, ch, 0, tar_obj);

    } else {			       /* Then it is a PC | NPC */

      if (!tar_ch)
	tar_ch = ch;

      if (affected_by_spell(tar_ch, SPELL_BLESS) ||
	  (GET_POS(tar_ch) == POSITION_FIGHTING))
	return;
      spell_bless(level, ch, tar_ch, 0);
    }
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) {		       /* It's an object */
      if (IS_SET(tar_obj->obj_flags.extra_flags, ITEM_BLESS))
	return;
      spell_bless(level, ch, 0, tar_obj);

    } else {			       /* Then it is a PC | NPC */

      if (affected_by_spell(tar_ch, SPELL_BLESS) ||
	  (GET_POS(tar_ch) == POSITION_FIGHTING))
	return;
      spell_bless(level, ch, tar_ch, 0);
    }
    break;
  default:
    log("Serious screw-up in bless!");
    break;
  }
}

void cast_blindness(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  struct affected_type af;

  switch (type) {
  case SPELL_TYPE_SPELL:
    if (IS_AFFECTED(tar_ch, AFF_BLIND)) {
      send_to_char("Nothing seems to happen.\n\r", ch);
      return;
    }
    spell_blindness(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    if (IS_AFFECTED(ch, AFF_BLIND))
      return;
    spell_blindness(level, ch, ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj)
      return;
    if (!tar_ch)
      tar_ch = ch;
    if (IS_AFFECTED(tar_ch, AFF_BLIND))
      return;
    spell_blindness(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj)
      return;
    if (!tar_ch)
      tar_ch = ch;
    if (IS_AFFECTED(tar_ch, AFF_BLIND))
      return;
    spell_blindness(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (!in_group(ch, tar_ch))
	if (!(IS_AFFECTED(tar_ch, AFF_BLIND)))
	  spell_blindness(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in blindness!");
    break;
  }
}

void cast_burning_hands(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_STAFF:
  case SPELL_TYPE_SCROLL:
    spell_burning_hands(level, ch, 0, 0);
    break;
  default:
    log("Serious screw-up in burning hands!");
    break;
  }
}

void cast_call_lightning(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj)
{
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
    log("Serious screw-up in call lightning!");
    break;
  }
}

void cast_charm_person(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_charm_person(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (!tar_ch)
      return;
    spell_charm_person(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (!in_group(tar_ch, ch))
	spell_charm_person(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in charm person!");
    break;
  }
}

void cast_chill_touch(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_chill_touch(level, ch, victim, 0);
    break;
  default:
    log("Serious screw-up in chill touch!");
    break;
  }
}

#define GLASS_MIRROR	1133

void cast_clone(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  struct char_data *vict;
  struct char_data *victim, *next_victim;
  struct obj_data *sac;
  char buf[MAX_STRING_LENGTH];

  if (!ch->equipment[HOLD]) {
    send_to_char("You must be holding the component for this spell.\n\r", ch);
    return;
  }
  sac = unequip_char(ch, HOLD);
  if (sac) {
    obj_to_char(sac, ch);
    if (ObjVnum(sac) != GLASS_MIRROR) {
      send_to_char("That is not the correct item.\n\r", ch);
      return;
    }
  } else {
    send_to_char("You must be holding the component for this spell.\n\r", ch);
    return;
  }
  cprintf(ch, "You shatter %s %s into thousands of tiny shards.\n\r", SANA(sac),sac->short_description);
  sprintf(buf, "$n shatters %s %s into thousands of tiny shards.\n\r", SANA(sac),sac->short_description);
  act(buf, FALSE, ch, 0, 0, TO_ROOM);
  obj_from_char(sac);
  extract_obj(sac);

  for (victim = character_list; victim; victim = next_victim) {
    next_victim = victim->next;
    if ((ch->in_room == victim->in_room)) {
      if (IS_IMMORTAL(victim)) continue;
      if (affected_by_spell(victim, SPELL_SHIELD)) {
        act("Tiny shards of glass knife through the air bouncing off your shield!\n\r",
            FALSE, ch, 0, victim, TO_VICT);
        continue;
      }
      damage(victim, victim, dice(2,4), TYPE_SUFFERING);
      act("Tiny shards of glass knife through the air and strike you!\n\r",
          FALSE, ch, 0, victim, TO_VICT);
    }
  }

  switch (type) {
    case SPELL_TYPE_SPELL:
      if (tar_ch) {        
        spell_clone(level,ch,tar_ch,0);
      } else {
        cprintf(buf, "You create a duplicate of %s %s.\n\r",SANA(tar_obj),tar_obj->short_description);
        sprintf(buf, "$n creates a duplicate of %s %s,\n\r",SANA(tar_obj),tar_obj->short_description);
        act(buf, FALSE, ch, 0, 0, TO_ROOM);
        spell_clone(level,ch,0,tar_obj);
      }
      break;
    default: 
      log("Serious screw-up in clone!");
      break;
  }
}

void cast_colour_spray(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj)
{
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
    log("Serious screw-up in colour spray!");
    break;
  }
}

void cast_control_weather(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  char buffer[MAX_STRING_LENGTH];

  switch (type) {
  case SPELL_TYPE_SPELL:

    one_argument(arg, buffer);

    if (str_cmp("better", buffer) && str_cmp("worse", buffer)) {
      send_to_char("Do you want it to get better or worse?\n\r", ch);
      return;
    }
    if (!OUTSIDE(ch)) {
      send_to_char("You need to be outside.\n\r", ch);
    }
    if (!str_cmp("better", buffer)) {
      if (weather_info.sky == SKY_CLOUDLESS)
	return;
      if (weather_info.sky == SKY_CLOUDY) {
	send_to_outdoor("The clouds disappear.\n\r");
	weather_info.sky = SKY_CLOUDLESS;
      }
      if (weather_info.sky == SKY_RAINING) {
	if ((time_info.month > 3) && (time_info.month < 14))
	  send_to_outdoor("The rain has stopped.\n\r");
	else
	  send_to_outdoor("The snow has stopped. \n\r");
	weather_info.sky = SKY_CLOUDY;
      }
      if (weather_info.sky == SKY_LIGHTNING) {
	if ((time_info.month > 3) && (time_info.month < 14))
	  send_to_outdoor("The lightning has gone, but it is still raining.\n\r");
	else
	  send_to_outdoor("The blizzard is over, but it is still snowing.\n\r");
	weather_info.sky = SKY_RAINING;
      }
      return;
    } else {
      if (weather_info.sky == SKY_CLOUDLESS) {
	send_to_outdoor("The sky is getting cloudy.\n\r");
	weather_info.sky = SKY_CLOUDY;
	return;
      }
      if (weather_info.sky == SKY_CLOUDY) {
	if ((time_info.month > 3) && (time_info.month < 14))
	  send_to_outdoor("It starts to rain.\n\r");
	else
	  send_to_outdoor("It starts to snow. \n\r");
	weather_info.sky = SKY_RAINING;
      }
      if (weather_info.sky == SKY_RAINING) {
	if ((time_info.month > 3) && (time_info.month < 14))
	  send_to_outdoor("You are caught in lightning storm.\n\r");
	else
	  send_to_outdoor("You are caught in a blizzard. \n\r");
	weather_info.sky = SKY_LIGHTNING;
      }
      if (weather_info.sky == SKY_LIGHTNING) {
	return;
      }
      return;
    }
    break;

  default:
    log("Serious screw-up in control weather!");
    break;
  }
}

void cast_create_food(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{

  switch (type) {
  case SPELL_TYPE_SPELL:
    act("$n magically creates a mushroom.", FALSE, ch, 0, 0, TO_ROOM);
    spell_create_food(level, ch, 0, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj)
      return;
    if (tar_ch)
      return;
    spell_create_food(level, ch, 0, 0);
    break;
  default:
    log("Serious screw-up in create food!");
    break;
  }
}

void cast_create_water(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    if (tar_obj->obj_flags.type_flag != ITEM_DRINKCON) {
      send_to_char("It is unable to hold water.\n\r", ch);
      return;
    }
    spell_create_water(level, ch, 0, tar_obj);
    break;
  default:
    log("Serious screw-up in create water!");
    break;
  }
}

void cast_cure_blind(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_cure_blind(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_cure_blind(level, ch, ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_cure_blind(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in cure blind!");
    break;
  }
}

/* Offensive */

void cast_shocking_grasp(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_shocking_grasp(level, ch, victim, 0);
    break;
  default:
    log("Serious screw-up in shocking grasp!");
    break;
  }
}

void cast_earthquake(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_STAFF:
    spell_earthquake(level, ch, 0, 0);
    break;
  default:
    log("Serious screw-up in earthquake!");
    break;
  }
}

void cast_energy_drain(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj)
{
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
    for (victim = real_roomp(ch->in_room)->people;
	 victim; victim = victim->next_in_room)
      if (!in_group(ch, victim))
	if (victim != ch)
	  spell_energy_drain(level, ch, victim, 0);
    break;
  default:
    log("Serious screw-up in energy drain!");
    break;
  }
}

void cast_fireball(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj)
{

  switch (type) {
  case SPELL_TYPE_WAND:
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_STAFF:
    spell_fireball(level, ch, 0, 0);
    break;
  default:
    log("Serious screw-up in fireball");
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
 * log("Serious screw-up in fireball!");
 * break;
 * 
 * }
 */

}

void cast_harm(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_harm(level, ch, victim, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_harm(level, ch, ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (victim = real_roomp(ch->in_room)->people;
	 victim; victim = victim->next_in_room)
      if (!in_group(ch, victim))
	spell_harm(level, ch, victim, 0);
    break;
  default:
    log("Serious screw-up in harm!");
    break;

  }
}

void cast_lightning_bolt(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj)
{
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
    log("Serious screw-up in lightning bolt!");
    break;

  }
}

void cast_acid_blast(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj)
{
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
    log("Serious screw-up in acid blast!");
    break;

  }
}

void cast_cone_of_cold(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj)
{

  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_WAND:
    spell_cone_of_cold(level, ch, 0, 0);
    break;

  default:
    log("Serious screw-up in cone of cold!");
    break;

  }
}

void cast_ice_storm(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_WAND:
    spell_ice_storm(level, ch, 0, 0);
    break;

  default:
    log("Serious screw-up in acid blast!");
    break;

  }
}

void cast_meteor_swarm(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj)
{

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
    log("Serious screw-up in meteor swarm!");
    break;

  }
}

void cast_flamestrike(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj)
{
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
    log("Serious screw-up in flamestrike!");
    break;

  }
}

void cast_magic_missile(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj)
{
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
    log("Serious screw-up in magic missile!");
    break;

  }
}

void cast_cause_light(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_cause_light(level, ch, victim, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_cause_light(level, ch, ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (victim = real_roomp(ch->in_room)->people;
	 victim; victim = victim->next_in_room)
      if (!in_group(ch, victim))
	spell_cause_light(level, ch, victim, 0);
    break;
  default:
    log("Serious screw-up in cause light wounds!");
    break;

  }
}

void cast_cause_serious(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj)
{
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
    for (victim = real_roomp(ch->in_room)->people;
	 victim; victim = victim->next_in_room)
      if (!in_group(ch, victim))
	spell_cause_serious(level, ch, victim, 0);
    break;
  default:
    log("Serious screw-up in cause serious wounds!");
    break;

  }
}

void cast_cause_critic(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_cause_critical(level, ch, victim, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_cause_critical(level, ch, ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (victim = real_roomp(ch->in_room)->people;
	 victim; victim = victim->next_in_room)
      if (!in_group(ch, victim))
	spell_cause_critical(level, ch, victim, 0);
    break;
  default:
    log("Serious screw-up in cause critical!");
    break;

  }
}

void cast_geyser(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_WAND:
  case SPELL_TYPE_STAFF:
    spell_geyser(level, ch, 0, 0);
    break;
  default:
    log("Serious screw-up in geyser!");
    break;
  }
}

void cast_green_slime(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj)
{
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
    log("Serious screw-up in green Slime!");
    break;
  }
}

/*
 * file: spells2.c , Implementation of magic.             Part of DIKUMUD
 * Usage : All the non-offensive magic handling routines.
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */

void cast_resurrection(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{

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
    log("Serious problem in 'resurrection'");
    break;
  }

}

void cast_mana(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{

  switch (type) {
  case SPELL_TYPE_POTION:
    spell_mana(level, ch, ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (!tar_ch)
      tar_ch = ch;
    spell_mana(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_mana(level, ch, tar_ch, 0);
  default:
    log("Serious problem in 'mana'");
    break;
  }

}

void cast_stone_skin(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    if (affected_by_spell(ch, SPELL_STONE_SKIN) ||
	affected_by_spell(ch, SPELL_ARMOR) ||
	affected_by_spell(ch, SPELL_SHIELD)) {
      send_to_char("Nothing seems to happen.\n\r", ch);
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
    log("Serious screw-up in stone_skin!");
    break;
  }
}

void cast_astral_walk(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_WAND:
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_POTION:
  case SPELL_TYPE_SPELL:

    if (!tar_ch)
      send_to_char("Yes, but who do you wish to walk to?\n", ch);
    else
      spell_astral_walk(level, ch, tar_ch, 0);
    break;

  default:
    log("Serious screw-up in astral walk!");
    break;
  }
}

void cast_infravision(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{

  struct affected_type af;

  switch (type) {
  case SPELL_TYPE_SPELL:
    if (IS_AFFECTED(tar_ch, AFF_INFRAVISION)) {
      send_to_char("Nothing seems to happen.\n\r", ch);
      return;
    }
    spell_infravision(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    if (IS_AFFECTED(ch, AFF_INFRAVISION))
      return;
    spell_infravision(level, ch, ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj)
      return;
    if (!tar_ch)
      tar_ch = ch;
    if (IS_AFFECTED(tar_ch, AFF_INFRAVISION))
      return;
    spell_infravision(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj)
      return;
    if (IS_AFFECTED(tar_ch, AFF_INFRAVISION))
      return;
    spell_infravision(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	if (!(IS_AFFECTED(tar_ch, AFF_INFRAVISION)))
	  spell_infravision(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in infravision!");
    break;
  }

}

void cast_true_seeing(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{

  switch (type) {
  case SPELL_TYPE_SPELL:
    if (IS_AFFECTED(tar_ch, AFF_TRUE_SIGHT)) {
      send_to_char("Nothing seems to happen.\n\r", ch);
      return;
    }
    spell_true_seeing(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    if (IS_AFFECTED(ch, AFF_TRUE_SIGHT))
      return;
    spell_true_seeing(level, ch, ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj)
      return;
    if (!tar_ch)
      tar_ch = ch;
    if (IS_AFFECTED(tar_ch, AFF_TRUE_SIGHT))
      return;
    spell_true_seeing(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj)
      return;
    if (IS_AFFECTED(tar_ch, AFF_TRUE_SIGHT))
      return;
    spell_true_seeing(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	if (!(IS_AFFECTED(tar_ch, AFF_TRUE_SIGHT)))
	  spell_true_seeing(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in true_seeing!");
    break;
  }

}

void cast_light(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
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
    log("Serious screw-up in light!");
    break;
  }
}

void cast_cont_light(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
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
    log("Serious screw-up in continual light!");
    break;
  }
}

void cast_calm(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_calm(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj)
      return;
    if (!tar_ch)
      tar_ch = ch;
    spell_calm(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj)
      return;
    if (!tar_ch)
      tar_ch = ch;
    spell_calm(level, ch, ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      spell_calm(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in continual light!");
    break;
  }
}

void cast_water_breath(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{

  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_water_breath(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_water_breath(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_WAND:
    spell_water_breath(level, ch, tar_ch, 0);
    break;

  default:
    log("Serious screw-up in water breath");
    break;
  }
}

void cast_flying(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{

  if (MOUNTED(ch)) {
    send_to_char("Not while you are mounted!\n\r", ch);
    return;
  }
  if (MOUNTED(tar_ch)) {
    send_to_char("Not while they are mounted!\n\r", ch);
    return;
  }
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_fly(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_fly(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_WAND:
    spell_fly(level, ch, tar_ch, 0);
    break;

  default:
    log("Serious screw-up in fly");
    break;
  }
}

void cast_cure_critic(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_cure_critic(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_cure_critic(level, ch, ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (!tar_ch)
      tar_ch = ch;
    spell_cure_critic(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_cure_critic(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in cure critic!");
    break;

  }
}

void cast_cure_light(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_cure_light(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_cure_light(level, ch, ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (!tar_ch)
      tar_ch = ch;
    spell_cure_light(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_cure_light(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in cure light!");
    break;
  }
}

void cast_cure_serious(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_cure_serious(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_cure_serious(level, ch, ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (!tar_ch)
      tar_ch = ch;
    spell_cure_serious(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_cure_serious(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in cure serious!");
    break;
  }
}

void cast_refresh(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_refresh(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_refresh(level, ch, ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (!tar_ch)
      tar_ch = ch;
    spell_refresh(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_refresh(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in refresh!");
    break;
  }
}

void cast_second_wind(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_second_wind(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_second_wind(level, ch, ch, 0);
    break;
  case SPELL_TYPE_WAND:

  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_second_wind(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in second_wind!");
    break;
  }
}

void cast_shield(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    if (affected_by_spell(tar_ch, SPELL_STONE_SKIN)) {
      send_to_char("Nothing seems to happen.\n\r", ch);
      return;
    }
    spell_shield(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_shield(level, ch, ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (!tar_ch)
      tar_ch = ch;
    spell_shield(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_shield(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in shield!");
    break;
  }

}

void cast_curse(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    if (tar_obj)		       /* It is an object */
      spell_curse(level, ch, 0, tar_obj);
    else {			       /* Then it is a PC | NPC */
      spell_curse(level, ch, tar_ch, 0);
    }
    break;
  case SPELL_TYPE_POTION:
    spell_curse(level, ch, ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj)		       /* It is an object */
      spell_curse(level, ch, 0, tar_obj);
    else {			       /* Then it is a PC | NPC */
      if (!tar_ch)
	tar_ch = ch;
      spell_curse(level, ch, tar_ch, 0);
    }
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_curse(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in curse!");
    break;
  }
}

void cast_detect_evil(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    if (affected_by_spell(tar_ch, SPELL_DETECT_EVIL)) {
      send_to_char("Nothing seems to happen.\n\r", tar_ch);
      return;
    }
    spell_detect_evil(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    if (affected_by_spell(ch, SPELL_DETECT_EVIL))
      return;
    spell_detect_evil(level, ch, ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	if (!(IS_AFFECTED(tar_ch, AFF_DETECT_EVIL)))
	  spell_detect_evil(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in detect evil!");
    break;
  }
}

void cast_detect_invisibility(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    if (IS_AFFECTED(tar_ch, AFF_DETECT_INVISIBLE)) {
      send_to_char("Nothing seems to happen.\n\r", tar_ch);
      return;
    }
    spell_detect_invisibility(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    if (IS_AFFECTED(ch, AFF_DETECT_INVISIBLE))
      return;
    spell_detect_invisibility(level, ch, ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (in_group(ch, tar_ch))
	if (!(IS_AFFECTED(tar_ch, AFF_DETECT_INVISIBLE)))
	  spell_detect_invisibility(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in detect invisibility!");
    break;
  }
}

void cast_detect_magic(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    if (affected_by_spell(tar_ch, SPELL_DETECT_MAGIC)) {
      send_to_char("Nothing seems to happen.\n\r", tar_ch);
      return;
    }
    spell_detect_magic(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    if (affected_by_spell(ch, SPELL_DETECT_MAGIC))
      return;
    spell_detect_magic(level, ch, ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	if (!(IS_AFFECTED(tar_ch, SPELL_DETECT_MAGIC)))
	  spell_detect_magic(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in detect magic!");
    break;
  }
}

void cast_detect_poison(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_detect_poison(level, ch, tar_ch, tar_obj);
    break;
  case SPELL_TYPE_POTION:
    spell_detect_poison(level, ch, ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) {
      spell_detect_poison(level, ch, 0, tar_obj);
      return;
    }
    if (!tar_ch)
      tar_ch = ch;
    spell_detect_poison(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in detect poison!");
    break;
  }
}

void cast_dispel_evil(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_dispel_evil(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_dispel_evil(level, ch, ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj)
      return;
    if (!tar_ch)
      tar_ch = ch;
    spell_dispel_evil(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj)
      return;
    spell_dispel_evil(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (!in_group(tar_ch, ch))
	spell_dispel_evil(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in dispel evil!");
    break;
  }
}

void cast_dispel_good(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_dispel_good(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_dispel_good(level, ch, ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj)
      return;
    if (!tar_ch)
      tar_ch = ch;
    spell_dispel_good(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj)
      return;
    spell_dispel_good(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (!in_group(tar_ch, ch))
	spell_dispel_good(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in dispel good!");
    break;
  }
}

void cast_faerie_fire(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_faerie_fire(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_faerie_fire(level, ch, ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj)
      return;
    if (!tar_ch)
      tar_ch = ch;
    spell_faerie_fire(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj)
      return;
    spell_faerie_fire(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (!in_group(tar_ch, ch))
	spell_faerie_fire(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in dispel good!");
    break;
  }
}

void cast_enchant_weapon(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
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
    log("Serious screw-up in enchant weapon!");
    break;
  }
}

void cast_enchant_armor(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
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
    log("Serious screw-up in enchant armor!");
    break;
  }
}

void cast_heal(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    act("$n heals $N.", FALSE, ch, 0, tar_ch, TO_NOTVICT);
    act("You heal $N.", FALSE, ch, 0, tar_ch, TO_CHAR);
    spell_heal(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_heal(level, ch, ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_heal(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in heal!");
    break;
  }
}

void cast_invisibility(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    if (tar_obj) {
      if (IS_SET(tar_obj->obj_flags.extra_flags, ITEM_INVISIBLE))
	send_to_char("Nothing new seems to happen.\n\r", ch);
      else
	spell_invisibility(level, ch, 0, tar_obj);
    } else {			       /* tar_ch */
      if (IS_AFFECTED(tar_ch, AFF_INVISIBLE))
	send_to_char("Nothing new seems to happen.\n\r", ch);
      else
	spell_invisibility(level, ch, tar_ch, 0);
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
    } else {			       /* tar_ch */
      if (!tar_ch)
	tar_ch = ch;

      if (!(IS_AFFECTED(tar_ch, AFF_INVISIBLE)))
	spell_invisibility(level, ch, tar_ch, 0);
    }
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) {
      if (!(IS_SET(tar_obj->obj_flags.extra_flags, ITEM_INVISIBLE)))
	spell_invisibility(level, ch, 0, tar_obj);
    } else {			       /* tar_ch */
      if (!(IS_AFFECTED(tar_ch, AFF_INVISIBLE)))
	spell_invisibility(level, ch, tar_ch, 0);
    }
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	if (!(IS_AFFECTED(tar_ch, AFF_INVISIBLE)))
	  spell_invisibility(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in invisibility!");
    break;
  }
}

void cast_locate_object(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_locate_object(level, ch, 0, tar_obj);
    break;
  default:
    log("Serious screw-up in locate object!");
    break;
  }
}

void cast_poison(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_poison(level, ch, tar_ch, tar_obj);
    break;
  case SPELL_TYPE_POTION:
    spell_poison(level, ch, ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_poison(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in poison!");
    break;
  }
}

void cast_protection_from_evil(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_protection_from_evil(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_protection_from_evil(level, ch, ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj)
      return;
    if (!tar_ch)
      tar_ch = ch;
    spell_protection_from_evil(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_protection_from_evil(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in protection from evil!");
    break;
  }
}

void cast_remove_curse(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_remove_curse(level, ch, tar_ch, tar_obj);
    break;
  case SPELL_TYPE_POTION:
    spell_remove_curse(level, ch, ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) {
      spell_remove_curse(level, ch, 0, tar_obj);
      return;
    }
    if (!tar_ch)
      tar_ch = ch;
    spell_remove_curse(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_remove_curse(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in remove curse!");
    break;
  }
}

void cast_remove_poison(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_remove_poison(level, ch, tar_ch, tar_obj);
    break;
  case SPELL_TYPE_POTION:
    spell_remove_poison(level, ch, ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_remove_poison(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in remove poison!");
    break;
  }
}

void cast_remove_paralysis(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_remove_paralysis(level, ch, tar_ch, tar_obj);
    break;
  case SPELL_TYPE_POTION:
    spell_remove_paralysis(level, ch, ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (!tar_ch)
      tar_ch = ch;
    spell_remove_paralysis(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_remove_paralysis(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in remove paralysis!");
    break;
  }
}

void cast_sanctuary(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_sanctuary(level, ch, tar_ch, 0);
    break;

  case SPELL_TYPE_WAND:
  case SPELL_TYPE_POTION:
    spell_sanctuary(level, ch, ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj)
      return;
    if (!tar_ch)
      tar_ch = ch;
    spell_sanctuary(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_sanctuary(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in sanctuary!");
    break;
  }
}

void cast_fireshield(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_fireshield(level, ch, tar_ch, 0);
    break;

  case SPELL_TYPE_WAND:
  case SPELL_TYPE_POTION:
    spell_sanctuary(level, ch, ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj)
      return;
    if (!tar_ch)
      tar_ch = ch;
    spell_fireshield(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_fireshield(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in fireshield!");
    break;
  }
}

void cast_sleep(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_sleep(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_sleep(level, ch, ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj)
      return;
    if (!tar_ch)
      tar_ch = ch;
    spell_sleep(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj)
      return;
    spell_sleep(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_sleep(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in sleep!");
    break;
  }
}

void cast_strength(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_strength(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_strength(level, ch, ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj)
      return;
    if (!tar_ch)
      tar_ch = ch;
    spell_strength(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_strength(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in strength!");
    break;
  }
}

void cast_ventriloquate(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  struct char_data *tmp_ch;
  char buf1[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char buf3[MAX_STRING_LENGTH];

  if (type != SPELL_TYPE_SPELL) {
    log("Attempt to ventriloquate by non-cast-spell.");
    return;
  }
  for (; *arg && (*arg == ' '); arg++);
  if (tar_obj) {
    sprintf(buf1, "The %s says '%s'\n\r", fname(tar_obj->name), arg);
    sprintf(buf2, "Someone makes it sound like the %s says '%s'.\n\r",
	    fname(tar_obj->name), arg);
  } else {
    sprintf(buf1, "%s says '%s'\n\r", GET_NAME(tar_ch), arg);
    sprintf(buf2, "Someone makes it sound like %s says '%s'\n\r",
	    GET_NAME(tar_ch), arg);
  }

  sprintf(buf3, "Someone says, '%s'\n\r", arg);

  for (tmp_ch = real_roomp(ch->in_room)->people; tmp_ch;
       tmp_ch = tmp_ch->next_in_room) {

    if ((tmp_ch != ch) && (tmp_ch != tar_ch)) {
      if (saves_spell(tmp_ch, SAVING_SPELL))
	send_to_char(buf2, tmp_ch);
      else
	send_to_char(buf1, tmp_ch);
    } else {
      if (tmp_ch == tar_ch)
	send_to_char(buf3, tar_ch);
    }
  }
}

void cast_word_of_recall(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
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
    if (!tar_ch)
      tar_ch = ch;
    spell_word_of_recall(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj)
      return;
    spell_word_of_recall(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_word_of_recall(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in word of recall!");
    break;
  }
}

void cast_summon(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {

  case SPELL_TYPE_SPELL:
    spell_summon(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in summon!");
    break;
  }
}

void cast_charm_monster(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_charm_monster(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (!tar_ch)
      return;
    spell_charm_monster(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (!in_group(tar_ch, ch))
	spell_charm_monster(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in charm monster!");
    break;
  }
}

void cast_sense_life(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_sense_life(level, ch, ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_sense_life(level, ch, ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_sense_life(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in sense life!");
    break;
  }
}

void cast_identify(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SCROLL:
    spell_identify(level, ch, tar_ch, tar_obj);
    break;
  default:
    log("Serious screw-up in identify!");
    break;
  }
}

#define MAX_BREATHS 3
struct pbreath {
  int vnum, spell[MAX_BREATHS];
} breath_potions[] = {

  {
    3970, {
      201, 0
    }
  },
  {
    3971, {
      202, 0
    }
  },
  {
    3972, {
      203, 0
    }
  },
  {
    3973, {
      204, 0
    }
  },
  {
    3974, {
      205, 0
    }
  },
  {
    0
  },
};

void cast_dragon_breath(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *potion)
{
  struct pbreath *scan;
  int i;
  struct affected_type af;

  for (scan = breath_potions;
  scan->vnum && scan->vnum != obj_index[potion->item_number].virtual;
       scan++);
  if (scan->vnum == 0) {
    char buf[MAX_STRING_LENGTH];

    send_to_char("Hey, this potion isn't in my list!\n\r", ch);
    sprintf(buf, "unlisted breath potion %s %d", potion->short_description,
	    obj_index[potion->item_number].virtual);
    log(buf);
    return;
  }
  for (i = 0; i < MAX_BREATHS && scan->spell[i]; i++) {
    if (!affected_by_spell(ch, scan->spell[i])) {
      af.type = scan->spell[i];
      af.duration = 1 + dice(1, 2);
      if (GET_CON(ch) < 4) {
	send_to_char("You are too weak to stomach the potion and spew it all over the floor.\n\r", ch);
	act("$n gags and pukes glowing goop all over the floor.",
	    FALSE, ch, 0, ch, TO_NOTVICT);
	break;
      }
      if (level > MIN(GET_CON(ch) - 1, GetMaxLevel(ch))) {
	send_to_char("!GACK! You are too weak to handle the full power of the potion.\n\r", ch);
	act("$n gags and flops around on the floor a bit.",
	    FALSE, ch, 0, ch, TO_NOTVICT);
	level = MIN(GET_CON(ch) - 1, GetMaxLevel(ch));
      }
      af.modifier = -level;
      af.location = APPLY_CON;
      af.bitvector = 0;
      affect_to_char(ch, &af);
      send_to_char("You feel powerful forces build within your stomach...\n\r", ch);
    }
  }
}

void cast_fire_breath(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_fire_breath(level, ch, tar_ch, 0);
    break;			       /* It's a spell.. But people can'c cast it! */
  default:
    log("Serious screw-up in firebreath!");
    break;
  }
}

void cast_frost_breath(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_frost_breath(level, ch, tar_ch, 0);
    break;			       /* It's a spell.. But people can'c cast it! */
  default:
    log("Serious screw-up in frostbreath!");
    break;
  }
}

void cast_acid_breath(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_acid_breath(level, ch, tar_ch, 0);
    break;			       /* It's a spell.. But people can'c cast it! */
  default:
    log("Serious screw-up in acidbreath!");
    break;
  }
}

void cast_gas_breath(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_gas_breath(level, ch, tar_ch, 0);
    break;
    /* THIS ONE HURTS!! */
  default:
    log("Serious screw-up in gasbreath!");
    break;
  }
}

void cast_lightning_breath(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_lightning_breath(level, ch, tar_ch, 0);
    break;			       /* It's a spell.. But people can'c cast it! */
  default:
    log("Serious screw-up in lightningbreath!");
    break;
  }
}

void cast_knock(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  BYTE percent;
  int door, other_room;
  char dir[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  char otype[MAX_INPUT_LENGTH];
  struct room_direction_data *back;
  struct obj_data *obj;
  struct char_data *victim;

  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_WAND:{

      argument_interpreter(arg, otype, dir);

      if (!otype) {
	send_to_char("Knock on what?\n\r", ch);
	return;
      }
      if (generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj)) {
	if (obj->obj_flags.type_flag != ITEM_CONTAINER) {
	  sprintf(buf, " %s is not a container.\n\r ", obj->name);
	} else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED)) {
	  sprintf(buf, " Silly! %s isn't even closed!\n\r ", obj->name);
	} else if (obj->obj_flags.value[2] < 0) {
	  sprintf(buf, "%s doesn't have a lock...\n\r", obj->name);
	} else if (!IS_SET(obj->obj_flags.value[1], CONT_LOCKED)) {
	  sprintf(buf, "Hehe.. %s wasn't even locked.\n\r", ch);
	} else
/*
 * if (IS_SET(obj->obj_flags.value[1], CONT_PICKPROOF)) 
 * {
 * sprintf(buf,"%s resists your magic.\n\r",obj->name);
 * }
 * else
 */
	{
	  REMOVE_BIT(obj->obj_flags.value[1], CONT_LOCKED);
	  sprintf(buf, "<Click>\n\r");
	  act("$n magically opens $p", FALSE, ch, obj, 0, TO_ROOM);
	}
	send_to_char(buf, ch);
	return;
      } else if ((door = find_door(ch, otype, dir)) >= 0) {
	if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
	  send_to_char("That's absurd.\n\r", ch);
	else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
	  send_to_char("You realize that the door is already open.\n\r", ch);
	else if (EXIT(ch, door)->key < 0)
	  send_to_char("You can't seem to spot any lock to knock.\n\r", ch);
	else if (!IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))
	  send_to_char("Oh.. it wasn't locked at all.\n\r", ch);
	else
/*
 * if (IS_SET(EXIT(ch, door)->exit_info, EX_PICKPROOF))
 * send_to_char("You seem to be unable to knock this...\n\r", ch);
 * else
 */
	{
	  REMOVE_BIT(EXIT(ch, door)->exit_info, EX_LOCKED);
	  if (EXIT(ch, door)->keyword)
	    act("$n magically opens the lock of the $F.", 0, ch, 0,
		EXIT(ch, door)->keyword, TO_ROOM);
	  else
	    act("$n magically opens the lock.", TRUE, ch, 0, 0, TO_ROOM);
	  send_to_char("The lock quickly yields to your skills.\n\r", ch);
	  if ((other_room = EXIT(ch, door)->to_room) != NOWHERE)
	    if (back = real_roomp(other_room)->dir_option[rev_dir[door]])
	      if (back->to_room == ch->in_room)
		REMOVE_BIT(back->exit_info, EX_LOCKED);
	}
      }
    }
    break;
  default:
    log("serious error in Knock.");
    break;
  }
}

void cast_know_alignment(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_know_alignment(level, ch, tar_ch, tar_obj);
    break;
  case SPELL_TYPE_POTION:
    spell_know_alignment(level, ch, ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (!tar_ch)
      tar_ch = ch;
    spell_know_alignment(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in know alignment!");
    break;
  }
}

void cast_weakness(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_weakness(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_weakness(level, ch, ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj)
      return;
    if (!tar_ch)
      tar_ch = ch;
    spell_weakness(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (!in_group)
	spell_weakness(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in weakness!");
    break;
  }
}

void cast_dispel_magic(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_dispel_magic(level, ch, tar_ch, tar_obj);
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
    if (!tar_ch)
      tar_ch = ch;
    spell_dispel_magic(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_dispel_magic(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in dispel magic");
    break;
  }
}

void cast_animate_dead(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{

  struct obj_data *i;

  switch (type) {

  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_WAND:
    if (tar_obj) {
      if (IS_CORPSE(tar_obj)) {
	spell_animate_dead(level, ch, 0, tar_obj);
      } else {
	send_to_char("That's not a corpse!\n\r", ch);
	return;
      }
    } else {
      send_to_char("That isn't a corpse!\n\r", ch);
      return;
    }
    break;
  case SPELL_TYPE_POTION:
    send_to_char("Your body revolts against the magic liquid.\n\r", ch);
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
    log("Serious screw-up in animate_dead!");
    break;
  }
}

void cast_succor(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{

  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_WAND:
  case SPELL_TYPE_STAFF:
    spell_succor(level, ch, 0, 0);
  }

}

void cast_paralyze(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{

  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_paralyze(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_paralyze(level, ch, ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj)
      return;
    if (!tar_ch)
      tar_ch = ch;
    spell_paralyze(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj)
      return;
    spell_paralyze(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (tar_ch != ch)
	spell_paralyze(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in paralyze");
    break;
  }
}

void cast_fear(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{

  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_fear(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_fear(level, ch, ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj)
      return;
    if (!tar_ch)
      tar_ch = ch;
    spell_fear(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj)
      return;
    if (!tar_ch)
      tar_ch = ch;
    spell_fear(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (!in_group(tar_ch, ch))
	spell_fear(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in fear");
    break;
  }
}

void cast_turn(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{

  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_turn(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj)
      return;
    if (!tar_ch)
      tar_ch = ch;
    spell_turn(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj)
      return;
    if (!tar_ch)
      tar_ch = ch;
    spell_turn(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = real_roomp(ch->in_room)->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (!in_group(tar_ch, ch))
	spell_turn(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in turn");
    break;
  }
}

void cast_faerie_fog(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *victim, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_STAFF:
  case SPELL_TYPE_SCROLL:
    spell_faerie_fog(level, ch, 0, 0);
    break;
  default:
    log("Serious screw-up in faerie fog!");
    break;
  }
}

const struct PolyType PolyList[] =
{
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

void cast_poly_self(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  char buffer[40];
  int mobn, X = LAST_POLY_MOB, found = FALSE;
  struct char_data *mob;

  /* one_argument(arg, buffer); */
  only_argument(arg, buffer);
  if (IS_NPC(ch)) {
    send_to_char("You don't really want to do that.\n\r", ch);
    return;
  }
  switch (type) {
  case SPELL_TYPE_SPELL:
    {
      while (!found) {
	if (PolyList[X].level > level) {
	  X--;
	} else {
	  if (!str_cmp(PolyList[X].name, buffer)) {
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
	send_to_char("Couldn't find any of those\n\r", ch);
	return;
      } else {
	mob = read_mobile(mobn, VIRTUAL);
	if (mob) {
	  spell_poly_self(level, ch, mob, 0);
	} else {
	  send_to_char("You couldn't summon an image of that creature\n\r", ch);
	}
	return;
      }

    }
    break;

  default:{
      log("Problem in poly_self");
    }
    break;
  }
}

#define NUT_CRACKED 1131

void cast_shelter(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  int mob, obj;
  struct obj_data *sac;
  struct char_data *el;

  if (!ch->equipment[HOLD]) {
    send_to_char(" You must be holding the component for this spell.\n\r", ch);
    return;
  }
  sac = unequip_char(ch, HOLD);
  if (sac) {
    obj_to_char(sac, ch);
    if (ObjVnum(sac) != NUT_CRACKED) {
      send_to_char("That is not the correct item.\n\r", ch);
      return;
    }
  } else {
    send_to_char("You must be holding the component for this spell.\n\r", ch);
    return;
  }

  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_shelter(level, ch, ch, sac);
    break;
  default:
    log("serious screw-up in shelter.");
    break;
  }
}

#define LONG_SWORD   3032
#define SHIELD       5045
#define BOAT         6101
#define BAG          3017
#define WATER_BARREL 3005
#define BREAD        3010

void cast_minor_creation(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  char buffer[40];
  int mob, obj;
  struct obj_data *o;

  one_argument(arg, buffer);

  if (!str_cmp(buffer, "sword")) {
    obj = LONG_SWORD;
  } else if (!str_cmp(buffer, "shield")) {
    obj = SHIELD;
  } else if (!str_cmp(buffer, "canoe")) {
    obj = BOAT;
  } else if (!str_cmp(buffer, "bag")) {
    obj = BAG;
  } else if (!str_cmp(buffer, "barrel")) {
    obj = WATER_BARREL;
  } else if (!str_cmp(buffer, "bread")) {
    obj = BREAD;
  } else {
    send_to_char("There is nothing of that available\n\r", ch);
    return;
  }

  o = read_object(obj, VIRTUAL);
  if (!o) {
    send_to_char("There is nothing of that available\n\r", ch);
    return;
  }
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
    spell_minor_create(level, ch, 0, o);
    break;
  default:
    log("serious screw-up in minor_create.");
    break;
  }

}

#define FIRE_ELEMENTAL  10
#define WATER_ELEMENTAL 11
#define AIR_ELEMENTAL   13
#define EARTH_ELEMENTAL 12

#define RED_STONE       1120
#define PALE_BLUE_STONE 1121
#define CLEAR_STONE     1122
#define GREY_STONE      1123

void cast_conjure_elemental(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  char buffer[40];
  int mob, obj;
  struct obj_data *sac;
  struct char_data *el;

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
    send_to_char("There are no elementals of that type available\n\r", ch);
    return;
  }
  if (!ch->equipment[HOLD]) {
    send_to_char(" You must be holding the correct stone\n\r", ch);
    return;
  }
  sac = unequip_char(ch, HOLD);
  if (sac) {
    obj_to_char(sac, ch);
    if (ObjVnum(sac) != obj) {
      send_to_char("You must have the correct item to sacrifice.\n\r", ch);
      return;
    }
    el = read_mobile(mob, VIRTUAL);
    if (!el) {
      send_to_char("There are no elementals of that type available\n\r", ch);
      return;
    }
  } else {
    send_to_char("You must be holding the correct item to sacrifice.\n\r", ch);
    return;
  }

  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
    spell_conjure_elemental(level, ch, el, sac);
    break;
  default:
    log("serious screw-up in conjure_elemental.");
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

void cast_cacaodemon(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  char buffer[40];
  int mob, obj;
  struct obj_data *sac;
  struct char_data *el;

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
    send_to_char("There are no demons of that type available\n\r", ch);
    return;
  }
  if (!ch->equipment[WIELD]) {
    send_to_char(" You must be wielding the correct item\n\r", ch);
    return;
  }
  sac = unequip_char(ch, WIELD);
  if (sac) {
    obj_to_char(sac, ch);
    if (ObjVnum(sac) != obj) {
      send_to_char("You must have the correct item to sacrifice.\n\r", ch);
      return;
    }
    el = read_mobile(mob, VIRTUAL);
    if (!el) {
      send_to_char("There are no demons of that type available\n\r", ch);
      return;
    }
  } else {
    send_to_char("You must be holding the correct item to sacrifice.\n\r", ch);
    return;
  }

  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
    spell_cacaodemon(level, ch, el, sac);
    break;
  default:
    log("serious screw-up in conjure_elemental.");
    break;
  }

}

void cast_mon_sum1(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{

  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_WAND:
  case SPELL_TYPE_STAFF:
    spell_Create_Monster(5, ch, 0, 0);
    break;
  default:
    log("Serious screw-up in monster_summoning_1");
    break;
  }
}

void cast_mon_sum2(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{

  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_WAND:
  case SPELL_TYPE_STAFF:
    spell_Create_Monster(7, ch, 0, 0);
    break;
  default:
    log("Serious screw-up in monster_summoning_1");
    break;
  }
}

void cast_mon_sum3(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{

  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_WAND:
  case SPELL_TYPE_STAFF:
    spell_Create_Monster(9, ch, 0, 0);
    break;
  default:
    log("Serious screw-up in monster_summoning_1");
    break;
  }
}

void cast_mon_sum4(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{

  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_WAND:
  case SPELL_TYPE_STAFF:
    spell_Create_Monster(11, ch, 0, 0);
    break;
  default:
    log("Serious screw-up in monster_summoning_1");
    break;
  }
}

void cast_mon_sum5(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{

  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_WAND:
  case SPELL_TYPE_STAFF:
    spell_Create_Monster(13, ch, 0, 0);
    break;
  default:
    log("Serious screw-up in monster_summoning_1");
    break;
  }
}

void cast_mon_sum6(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{

  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_WAND:
  case SPELL_TYPE_STAFF:
    spell_Create_Monster(15, ch, 0, 0);
    break;
  default:
    log("Serious screw-up in monster_summoning_1");
    break;
  }
}

void cast_mon_sum7(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{

  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_WAND:
  case SPELL_TYPE_STAFF:
    spell_Create_Monster(17, ch, 0, 0);
    break;
  default:
    log("Serious screw-up in monster_summoning_1");
    break;
  }
}

/*
 * file: spells2.c , Implementation of magic.             Part of DIKUMUD
 * Usage : All the non-offensive magic handling routines.
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */

void cast_fly_group(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{

  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_WAND:
  case SPELL_TYPE_STAFF:
    spell_fly_group(level, ch, 0, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_fly(level, ch, tar_ch, 0);
    break;
  default:
    log("Serious screw-up in fly");
    break;
  }
}

void cast_aid(BYTE level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
  if (!tar_ch)
    tar_ch = ch;

  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_WAND:
  case SPELL_TYPE_STAFF:
    spell_aid(level, ch, tar_ch, 0);
    break;
  default:
    log("serious screw-up in scare.");
    break;
  }
}

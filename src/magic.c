/*
 * ************************************************************************
 * *  file: magic.c , Implementation of spells.              Part of DIKUMUD *
 * *  Usage : The actual effect of magic.                                    *
 * *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
 * ************************************************************************* 
 */

#include <stdio.h>
#include <assert.h>
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "spells.h"
#include "handler.h"
#include "limits.h"
#include "db.h"

/*
 * Extern structures 
 */
extern struct hash_header        room_db;
extern struct obj_data          *object_list;
extern struct char_data         *character_list;
extern int                       DEBUG;

/*
 * Extern procedures 
 */

void                             set_title(struct char_data *ch);
void                             gain_exp(struct char_data *ch, int gain);
void                             damage(struct char_data *ch, struct char_data *victim,
					int damage, int weapontype);
bool                             saves_spell(struct char_data *ch, sh_int spell);
void                             weight_change_object(struct obj_data *obj, int weight);
int                              dice(int number, int size);
char                             in_group(struct char_data *ch1, struct char_data *ch2);
void                             set_fighting(struct char_data *ch, struct char_data *vict);
bool                             ImpSaveSpell(struct char_data *ch, sh_int save_type, int mod);
int                              IsPerson(struct char_data *ch);
int                              IsExtraPlanar(struct char_data *ch);

/*
 * Offensive Spells 
 */

void 
spell_magic_missile(byte level, struct char_data *ch,
		    struct char_data *victim, struct obj_data *obj)
{
  int                              dam;
  int                              missiles;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));
  if (DEBUG)
    dlog("spell_magic_missle");

  if (!CheckKill(ch, victim))
    return;

  missiles = (GET_LEVEL(ch, MAGE_LEVEL_IND) / 2) + 1;
  missiles = MIN(5, missiles);
  dam = dice(missiles, 4) + missiles;

  if (affected_by_spell(victim, SPELL_SHIELD))
    dam = 0;

  damage(ch, victim, dam, SPELL_MAGIC_MISSILE);
}

void 
spell_chill_touch(byte level, struct char_data *ch,
		  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type             af;
  int                              dam;
  int                              high_die;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));
  if (DEBUG)
    dlog("spell_chill_touch");
  if (!CheckKill(ch, victim))
    return;
  high_die = (GET_LEVEL(ch, MAGE_LEVEL_IND) / 3);

  dam = dice(high_die, 4);

  af.type = SPELL_CHILL_TOUCH;
  af.duration = 12;
  af.modifier = -1;
  af.location = APPLY_STR;
  af.bitvector = 0;
  affect_join(victim, &af, TRUE, FALSE);

  if (saves_spell(victim, SAVING_SPELL)) {
    dam = 0;
  }
  damage(ch, victim, dam, SPELL_CHILL_TOUCH);
}

void 
spell_burning_hands(byte level, struct char_data *ch,
		    struct char_data *victim, struct obj_data *obj)
{
  int                              dam;
  struct char_data                *tmp_victim,
                                  *temp,
                                  *ptr;
  int                              high_die;

  assert(ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));
  if (DEBUG)
    dlog("spell_burning_hands");
  dam = dice(1, 6) + MIN(20, level);

  if (dam < 1)
    dam = 1;

  send_to_char("Searing flame fans out in front of you!\n\r", ch);
  act("$n sends a fan of flame shooting from the fingertips!\n\r", FALSE, ch, 0, 0, TO_ROOM);
  AreaEffectSpell(ch, dam, SPELL_BURNING_HANDS, 0, 0);

#ifdef 0
  ptr = real_roomp(ch->in_room)->people;
  for (tmp_victim = ptr; tmp_victim; tmp_victim = tmp_victim->next_in_room) {
    if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
      if (IS_MORTAL(tmp_victim)) {
	act("You are seared by the burning flame!\n\r", FALSE, ch, 0, tmp_victim, TO_VICT);
	if (saves_spell(tmp_victim, SAVING_SPELL))
	  dam >>= 1;
	damage(ch, tmp_victim, dam, SPELL_BURNING_HANDS);
      } else
	send_to_char("Your divine nature saves you from a burning fan...\n\r", tmp_victim);
    }
  }
#endif
}

void 
spell_shocking_grasp(byte level, struct char_data *ch,
		     struct char_data *victim, struct obj_data *obj)
{
  int                              dam;
  int                              classes;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));
  if (DEBUG)
    dlog("spell_shocking_graps");
  if (!CheckKill(ch, victim))
    return;

  classes = HowManyClasses(ch);

  dam = dice(1, 8) + level;

  if (classes > 1)
    dam -= dice(classes, 3);

  if (dam < 1)
    dam = 1;

  if (saves_spell(victim, SAVING_SPELL))
    dam >>= 1;
  damage(ch, victim, dam, SPELL_SHOCKING_GRASP);
}

void 
spell_lightning_bolt(byte level, struct char_data *ch,
		     struct char_data *victim, struct obj_data *obj)
{
  int                              dam;
  int                              high_die;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));
  if (DEBUG)
    dlog("spell_lightning_bolt");
  if (!CheckKill(ch, victim))
    return;

  high_die = 5 + ((level / 5) - 1);

  dam = dice(high_die, 6);

  if (saves_spell(victim, SAVING_SPELL))
    dam >>= 1;

  damage(ch, victim, dam, SPELL_LIGHTNING_BOLT);
}

void 
spell_colour_spray(byte level, struct char_data *ch,
		   struct char_data *victim, struct obj_data *obj)
{
  int                              dam;
  struct char_data                *tmp_victim,
                                  *temp;
  struct affected_type             af;

  assert(ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  if (HowManyClasses(ch) == 1)
    dam = dice(1, 6);
  else
    dam = dice(1, 4);

  for (tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
    temp = tmp_victim->next;
    if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim) && CheckKill(ch, victim)) {
      if ((IS_IMMORTAL(tmp_victim)) && (IS_PC(tmp_victim))) {
	send_to_char("Some puny mortal tries to blind you with color spray\n\r", tmp_victim);
      } else {
	damage(ch, tmp_victim, dam, SPELL_COLOUR_SPRAY);

	if (GetMaxLevel(tmp_victim) <= (GET_LEVEL(ch, MAGE_LEVEL_IND) + 3)) {
	  if (!saves_spell(tmp_victim, SAVING_SPELL)) {
	    if (!affected_by_spell(tmp_victim, SPELL_BLINDNESS)) {
	      act("$n seems to be blinded!", TRUE, tmp_victim, 0, 0, TO_ROOM);
	      send_to_char("You have been blinded!\n\r", tmp_victim);

	      af.type = SPELL_BLINDNESS;
	      af.location = APPLY_HITROLL;
	      af.modifier = -4;
	      af.duration = dice(2, 4);
	      af.bitvector = AFF_BLIND;
	      affect_to_char(tmp_victim, &af);

	      af.location = APPLY_AC;
	      af.modifier = +20;	/*
					 * Make AC Worse! 
					 */
	      affect_to_char(tmp_victim, &af);

	      if ((!tmp_victim->specials.fighting) && (tmp_victim != ch))
		set_fighting(tmp_victim, ch);
	    }
	  }
	}
      }
    }
  }
}

/*
 * Drain XP, MANA, HP - caster gains HP and MANA 
 */
void 
spell_energy_drain(byte level, struct char_data *ch,
		   struct char_data *victim, struct obj_data *obj)
{
  int                              dam,
                                   xp,
                                   mana;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));
  if (DEBUG)
    dlog("spell_energy_drain");

  if (!CheckKill(ch, victim))
    return;
  if (!saves_spell(victim, SAVING_SPELL)) {
    if (IS_GOOD(ch) && IS_EVIL(victim) && IS_NPC(victim)) {
      GET_ALIGNMENT(ch) += 100;
      if (GET_ALIGNMENT(ch) > 1000)
	GET_ALIGNMENT(ch) = 1000;
    }
    if (IS_EVIL(ch) && IS_GOOD(victim) && IS_NPC(victim)) {
      GET_ALIGNMENT(ch) -= 100;
      if (GET_ALIGNMENT(ch) < -1000)
	GET_ALIGNMENT(ch) = -1000;
    }
    if (IS_PC(victim))
      GET_ALIGNMENT(ch) = -1000;

    if (GetMaxLevel(victim) <= 1) {
      damage(ch, victim, 100, SPELL_ENERGY_DRAIN);	/*
							 * Kill the sucker 
							 */
    } else if ((IS_PC(victim)) && (GetMaxLevel(victim) >= LOW_IMMORTAL)) {
      send_to_char("Some puny mortal just tried to drain you...\n\r", victim);
    } else {
      if (IS_NOT_SET(victim->M_immune, IMM_DRAIN)) {
	send_to_char("Your life energy is drained!\n\r", victim);
	dam = 1;
	if (IS_PC(victim)) {
	  if (HowManyClasses(ch) == 1) {
	    damage(ch, victim, dam, SPELL_ENERGY_DRAIN);
	    drop_level(victim, BestClass(victim));
	    set_title(victim);
	  } else {
	    dam = 50;
	    damage(ch, victim, dam, SPELL_ENERGY_DRAIN);
	  }
	} else {
	  dam = dice(MIN((level / 3), 6), 6);
	  damage(ch, victim, dam, SPELL_ENERGY_DRAIN);
	}
      } else {
	if (IS_NOT_SET(ch->M_immune, IMM_DRAIN)) {
	  send_to_char("Your spell backfires!\n\r", ch);
	  if (IS_PC(ch)) {
	    drop_level(ch, BestClass(ch));
	    set_title(ch);
	  } else {
	    dam = dice(MIN((level / 3), 6), 8);		/*
							 * nasty spell 
							 */
	    damage(ch, victim, dam, SPELL_ENERGY_DRAIN);
	  }
	} else {
	  send_to_char("Your spell fails utterly.\n\r", ch);
	}
      }
    }
  } else {
    damage(ch, victim, 0, SPELL_ENERGY_DRAIN);	/*
						 * Miss 
						 */
  }
}

void 
spell_fireball(byte level, struct char_data *ch,
	       struct char_data *victim, struct obj_data *obj)
{
  int                              dam;
  struct char_data                *tmp_victim,
                                  *temp;
  int                              high_die;

  assert(ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));
  if (DEBUG)
    dlog("spell_fireball");
  high_die = (level / 5) + 2;
  dam = dice(high_die, 6);

  AreaEffectSpell(ch, dam, SPELL_FIREBALL, 1, "You feel a blast of hot air.\n\r");

#ifdef 0
  for (tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
    temp = tmp_victim->next;
    if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
      if ((IS_IMMORTAL(tmp_victim)) && (IS_PC(tmp_victim))) {
	send_to_char("Some puny mortal tries to toast you with a fireball", tmp_victim);
      } else {
	if (saves_spell(tmp_victim, SAVING_SPELL))
	  dam >>= 1;
	damage(ch, tmp_victim, dam, SPELL_FIREBALL);
      }
    } else {
      if (real_roomp(ch->in_room)->zone == real_roomp(tmp_victim->in_room)->zone)
	send_to_char("You feel a blast of hot air.\n\r", tmp_victim);
    }
  }
#endif
}

void 
spell_earthquake(byte level, struct char_data *ch,
		 struct char_data *victim, struct obj_data *obj)
{
  int                              dam;
  char                             buf[128];

  struct char_data                *tmp_victim,
                                  *temp;

  assert(ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));
  if (DEBUG)
    dlog("spell_earthquake");
  dam = dice(1, 8) + level;

  send_to_char("The ground trembles beneath your feet!\n\r", ch);
  act("$n makes the ground tremble and shiver\n\r", FALSE, ch, 0, 0, TO_ROOM);
  sprintf(buf, "The ground trembless beneath your feet....\n\r");
  AreaEffectSpell(ch, dam, SPELL_EARTHQUAKE, 1, buf);

#ifdef 0
  for (tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
    temp = tmp_victim->next;
    if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
      if ((IS_MORTAL(tmp_victim)) || (IS_NPC(tmp_victim))) {
	damage(ch, tmp_victim, dam, SPELL_EARTHQUAKE);
	act("You fall and hurt yourself!!\n\r", FALSE, ch, 0, tmp_victim, TO_VICT);
	GET_POS(tmp_victim) = POSITION_SITTING;
	WAIT_STATE(tmp_victim, (PULSE_VIOLENCE * 3));
      }
    } else {
      if (real_roomp(ch->in_room)->zone == real_roomp(tmp_victim->in_room)->zone)
	send_to_char("The ground trembles...", tmp_victim);
    }
  }
#endif
}

void 
spell_dispel_evil(byte level, struct char_data *ch,
		  struct char_data *victim, struct obj_data *obj)
{
  int                              dam;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  if (!CheckKill(ch, victim))
    return;

  if (IsExtraPlanar(victim)) {
    if (IS_EVIL(ch)) {
      victim = ch;
    } else {
      if (IS_GOOD(victim)) {
	act("Good protects $N.", FALSE, ch, 0, victim, TO_CHAR);
	return;
      }
    }

    if (!saves_spell(victim, SAVING_SPELL)) {
      act("$n forces $N from this plane.", TRUE, ch, 0, victim, TO_ROOM);
      act("You force $N from this plane.", TRUE, ch, 0, victim, TO_CHAR);
      act("$n forces you from this plane.", TRUE, ch, 0, victim, TO_VICT);
      gain_exp(ch, MIN(GET_EXP(victim) / 2, 50000));
      extract_char(victim);
    }
  } else {
    act("$N laughs at you.", TRUE, ch, 0, victim, TO_CHAR);
    act("$N laughs at $n.", TRUE, ch, 0, victim, TO_NOTVICT);
    act("You laugh at $n.", TRUE, ch, 0, victim, TO_VICT);
  }
}

void 
spell_call_lightning(byte level, struct char_data *ch,
		     struct char_data *victim, struct obj_data *obj)
{
  int                              dam;
  int                              high_die;
  int                              added_dam;

  extern struct weather_data       weather_info;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  if (DEBUG)
    dlog("spell_call_lightning");
  if (!CheckKill(ch, victim))
    return;

  high_die = 1 + (level / 5);

  dam = dice(high_die, 6);
  added_dam = dice(2, 6);

  if (IS_REALLY_VILE(ch) || IS_REALLY_HOLY(ch) || (OUTSIDE(ch) && (weather_info.sky >= SKY_RAINING))) {
    if (saves_spell(victim, SAVING_SPELL))
      dam >> 1;
    dam += added_dam;
    damage(ch, victim, dam, SPELL_CALL_LIGHTNING);
  }
}

void 
spell_harm(byte level, struct char_data *ch,
	   struct char_data *victim, struct obj_data *obj)
{
  int                              dam;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  if (DEBUG)
    dlog("spell_harm");
  if (!CheckKill(ch, victim))
    return;

  dam = (2 * GET_LEVEL(ch, CLERIC_LEVEL_IND));

  if (IS_REALLY_HOLY(ch))
    dam = (GET_LEVEL(ch, CLERIC_LEVEL_IND));
  else if (IS_REALLY_VILE(ch))
    dam = MIN(200, GET_HIT(victim)) - dice(1, 4);

  if (dam < 1)
    dam = 1;

  if (saves_spell(victim, SAVING_SPELL))
    dam = (2 * GET_LEVEL(ch, CLERIC_LEVEL_IND));

  damage(ch, victim, dam, SPELL_HARM);
}

/*
 * spells2.c - Not directly offensive spells 
 */

void 
spell_armor(byte level, struct char_data *ch,
	    struct char_data *victim, struct obj_data *obj)
{
  struct affected_type             af;
  int                              last;

  assert(victim);
  assert((level >= 0) && (level <= ABS_MAX_LVL));

  if (!affected_by_spell(victim, SPELL_ARMOR)) {
    af.type = SPELL_ARMOR;
    af.duration = GET_LEVEL(ch, BestMagicClass(ch));
    af.modifier = -20;
    af.location = APPLY_AC;
    af.bitvector = 0;

    affect_to_char(victim, &af);
    send_to_char("You feel someone protecting you.\n\r", victim);
  } else {
    send_to_char("Nothing New seems to happen\n\r", ch);
  }
}

void 
spell_shelter(byte level, struct char_data *ch,
	      struct char_data *victim, struct obj_data *obj)
{
  struct obj_cost                  cost;
  int                              i,
                                   save_room;
  char                            *tmp_desc;
  struct extra_descr_data         *ext;
  int                              found = 0;

  if (IS_NPC(ch))
    return;

  cost.total_cost = 0;		/*
				 * Minimum cost 
				 */
  cost.no_carried = 0;
  cost.ok = TRUE;		/*
				 * Use if any "-1" objects 
				 */

  add_obj_cost(ch, 0, ch->carrying, &cost);

  for (i = 0; i < MAX_WEAR; i++)
    add_obj_cost(ch, 0, ch->equipment[i], &cost);

  if (!cost.ok)
    return;

  obj_from_char(obj);
  extract_obj(obj);
  cost.total_cost = 0;

  GET_HOME(ch) = ch->in_room;
  save_obj(ch, &cost, 1);
  save_room = ch->in_room;
  send_to_char("You open a small rip in the fabric of space!\n\r", ch);
  send_to_char
    ("As you step through you leave the world of WileyII behind....\n\r", ch);
  act("$n opens a small rip in the fabric of space and steps through!"
      ,FALSE, ch, 0, 0, TO_ROOM);
  extract_char(ch);
  ch->in_room = save_room;
  save_char(ch, ch->in_room);
  return;
}

void 
spell_astral_walk(byte level, struct char_data *ch,
		  struct char_data *victim, struct obj_data *obj)
{
  int                              location;
  struct room_data                *rp;

  assert(ch && victim);
  location = victim->in_room;
  rp = real_roomp(location);

  if (GetMaxLevel(victim) > MAX_MORT || !rp || IS_SET(rp->room_flags, PRIVATE) ||
      IS_SET(rp->room_flags, NO_SUM) || IS_SET(rp->room_flags, NO_MAGIC)) {
    send_to_char("You failed.\n\r", ch);
    return;
  }
  if (dice(1, 8) == 8) {
    send_to_char("You failed.\n\r", ch);
    return;
  } else {
    act("$n opens a door to another dimension and steps through!", FALSE, ch, 0, 0, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, location);
    do_look(ch, "", 15);
    act("You are blinded for a moment as $n appears in a flash of light!", FALSE, ch, 0, 0, TO_ROOM);
    do_look(ch, "", 15);
  }
}

void 
spell_teleport(byte level, struct char_data *ch,
	       struct char_data *victim, struct obj_data *obj)
{
  int                              to_room;
  extern int                       top_of_world;	/*

							 * ref to the top element of world 
							 */
  struct room_data                *room;

  assert(ch && victim);
  if (IS_SET(victim->specials.new_act, NEW_PLR_TELEPORT)) {
    send_to_char("Sorry, that does not want to be teleported.\n\r", ch);
    return;
  }
  if (victim != ch) {
    if (saves_spell(victim, SAVING_SPELL)) {
      send_to_char("Your spell has no effect.\n\r", ch);
      if (IS_NPC(victim)) {
	if (!victim->specials.fighting)
	  set_fighting(victim, ch);
      } else {
	send_to_char("You feel strange, but the effect fades.\n\r", victim);
      }
      return;
    } else {
      ch = victim;		/*
				 * the character (target) is now the victim 
				 */
    }
  }
  do {
    to_room = number(0, top_of_world);
    room = real_roomp(to_room);
    if (room) {
      if (IS_SET(room->room_flags, PRIVATE) || IS_SET(room->room_flags, NO_SUM))
	room = 0;
    }
  } while (!room);

  if (MOUNTED(ch)) {
    char_from_room(ch);
    char_from_room(MOUNTED(ch));
    char_to_room(ch, to_room);
    char_to_room(MOUNTED(ch), to_room);
    act("$n and a mount slowly fade out of existence.", FALSE, ch, 0, 0, TO_ROOM);
    act("$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM);
  } else if (RIDDEN(ch)) {
    char_from_room(RIDDEN(ch));
    char_from_room(ch);
    char_to_room(RIDDEN(ch), to_room);
    char_to_room(ch, to_room);
    act("$n and a rider slowly fade out of existence.", FALSE, ch, 0, 0, TO_ROOM);
    act("$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM);
  } else {
    char_from_room(ch);
    char_to_room(ch, to_room);
    act("$n slowly fades out of existence.", FALSE, ch, 0, 0, TO_ROOM);
    act("$n slowly fades in to existence.", FALSE, ch, 0, 0, TO_ROOM);
  }

  /*
   * do_look(ch, "", 0); 
   */

  if (IS_SET(real_roomp(to_room)->room_flags, DEATH) && GetMaxLevel(ch) < LOW_IMMORTAL) {
    death_cry(ch);
    zero_rent(ch);
    extract_char(ch);
  }
}

void 
spell_bless(byte level, struct char_data *ch,
	    struct char_data *victim, struct obj_data *obj)
{
  struct affected_type             af;

  assert(ch && (victim || obj));
  assert((level >= 0) && (level <= ABS_MAX_LVL));
  if (DEBUG)
    dlog("spell_bless");
  if (obj) {
    if ((10 * GET_LEVEL(ch, CLERIC_LEVEL_IND) > GET_OBJ_WEIGHT(obj)) &&
	(GET_POS(ch) != POSITION_FIGHTING) && !IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)) {
      SET_BIT(obj->obj_flags.extra_flags, ITEM_BLESS);
      act("$p briefly glows.", FALSE, ch, obj, 0, TO_CHAR);
    }
  } else {
    if ((GET_POS(victim) != POSITION_FIGHTING) && (!affected_by_spell(victim, SPELL_BLESS))) {
      send_to_char("You feel righteous.\n\r", victim);
      af.type = SPELL_BLESS;
      af.duration = level / 3;
      af.modifier = 1;
      af.location = APPLY_HITROLL;
      af.bitvector = 0;
      affect_to_char(victim, &af);
      af.location = APPLY_SAVING_SPELL;
      af.modifier = -1;		/*
				 * Make better 
				 */
      affect_to_char(victim, &af);
    }
  }
}

void 
spell_blindness(byte level, struct char_data *ch,
		struct char_data *victim, struct obj_data *obj)
{
  struct affected_type             af;

  assert(ch && victim);
  assert((level >= 0) && (level <= ABS_MAX_LVL));
  if (DEBUG)
    dlog("spell_blindness");
  if (!CheckKill(ch, victim))
    return;

  if (saves_spell(victim, SAVING_SPELL) || affected_by_spell(victim, SPELL_BLINDNESS)) {
    if ((!victim->specials.fighting) && (victim != ch))
      set_fighting(victim, ch);
    return;
  }
  act("$n seems to be blinded!", TRUE, victim, 0, 0, TO_ROOM);
  send_to_char("You have been blinded!\n\r", victim);

  af.type = SPELL_BLINDNESS;
  af.location = APPLY_HITROLL;
  af.modifier = -4;		/*
				 * Make hitroll worse 
				 */
  af.duration = dice(2, 4);
  af.bitvector = AFF_BLIND;
  affect_to_char(victim, &af);
  af.location = APPLY_AC;
  af.modifier = +20;		/*
				 * Make AC Worse! 
				 */
  affect_to_char(victim, &af);

  if ((!victim->specials.fighting) && (victim != ch))
    set_fighting(victim, ch);
}

void 
spell_clone(byte level, struct char_data *ch,
	    struct char_data *victim, struct obj_data *obj)
{

  assert(ch && (victim || obj));
  assert((level >= 0) && (level <= ABS_MAX_LVL));
  if (DEBUG)
    dlog("spell_clone");
  send_to_char("Clone is not ready yet.", ch);

  if (obj) {
  } else {
  }
}

void 
spell_control_weather(byte level, struct char_data *ch,
		      struct char_data *victim, struct obj_data *obj)
{
  /*
   * Control Weather is not possible here!!! 
   */
  /*
   * Better/Worse can not be transferred     
   */
}

void 
spell_create_food(byte level, struct char_data *ch,
		  struct char_data *victim, struct obj_data *obj)
{
  struct obj_data                 *tmp_obj;

  assert(ch);
  assert((level >= 0) && (level <= ABS_MAX_LVL));
  if (DEBUG)
    dlog("spell_create_food");
  CREATE(tmp_obj, struct obj_data, 1);

  clear_object(tmp_obj);

  tmp_obj->name = (char *)strdup("mushroom");
  tmp_obj->short_description = (char *)strdup("A Magic Mushroom");
  tmp_obj->description = (char *)strdup("A really delicious looking magic mushroom lies here.");

  tmp_obj->obj_flags.type_flag = ITEM_FOOD;
  tmp_obj->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD;
  tmp_obj->obj_flags.value[0] = 5 + level;
  tmp_obj->obj_flags.weight = 1;
  tmp_obj->obj_flags.cost = 10;
  tmp_obj->obj_flags.cost_per_day = 1;

  tmp_obj->next = object_list;
  object_list = tmp_obj;
  obj_to_room(tmp_obj, ch->in_room);
  tmp_obj->item_number = -1;
  act("$p suddenly appears.", TRUE, ch, tmp_obj, 0, TO_ROOM);
  act("$p suddenly appears.", TRUE, ch, tmp_obj, 0, TO_CHAR);
}

void 
spell_create_water(byte level, struct char_data *ch,
		   struct char_data *victim, struct obj_data *obj)
{
  int                              water;

  extern struct weather_data       weather_info;
  void                             name_to_drinkcon(struct obj_data *obj, int type);
  void                             name_from_drinkcon(struct obj_data *obj);

  assert(ch && obj);
  if (DEBUG)
    dlog("spell_create_water");
  if (GET_ITEM_TYPE(obj) == ITEM_DRINKCON) {
    if ((obj->obj_flags.value[2] != LIQ_WATER) && (obj->obj_flags.value[1] != 0)) {
      name_from_drinkcon(obj);
      obj->obj_flags.value[2] = LIQ_SLIME;
      name_to_drinkcon(obj, LIQ_SLIME);
    } else {
      water = 2 * level * ((weather_info.sky >= SKY_RAINING) ? 2 : 1);
      /*
       * Calculate water it can contain, or water created 
       */
      water = MIN(obj->obj_flags.value[0] - obj->obj_flags.value[1], water);

      if (water > 0) {
	obj->obj_flags.value[2] = LIQ_WATER;
	obj->obj_flags.value[1] += water;

	weight_change_object(obj, water);

	name_from_drinkcon(obj);
	name_to_drinkcon(obj, LIQ_WATER);
	act("$p is partially filled.", FALSE, ch, obj, 0, TO_CHAR);
      }
    }
  }
}

void 
spell_cure_blind(byte level, struct char_data *ch,
		 struct char_data *victim, struct obj_data *obj)
{
  assert(victim);
  assert((level >= 0) && (level <= ABS_MAX_LVL));
  if (DEBUG)
    dlog("spell-cure_blind");
  if (affected_by_spell(victim, SPELL_BLINDNESS)) {
    affect_from_char(victim, SPELL_BLINDNESS);
    send_to_char("Your vision returns!\n\r", victim);
  }
}

void 
spell_cure_critic(byte level, struct char_data *ch,
		  struct char_data *victim, struct obj_data *obj)
{
  int                              healpoints;

  assert(victim);
  assert((level >= 0) && (level <= ABS_MAX_LVL));

  if (DEBUG)
    dlog("spell_cure_critic");
  if (HowManyClasses(ch) == 1)
    healpoints = dice(3, 8) + 3;
  else
    healpoints = dice(3, 7);

  if (IS_REALLY_HOLY(ch))
    healpoints += dice(1, 5);
  else if (IS_REALLY_VILE(ch))
    healpoints -= dice(1, 5);
  else if (IS_HOLY(ch))
    healpoints += dice(1, 3);
  else if (IS_VILE(ch))
    healpoints -= dice(1, 3);

  if (healpoints < 1)
    healpoints = 1;

  if ((healpoints + GET_HIT(victim)) > hit_limit(victim))
    GET_HIT(victim) = hit_limit(victim);
  else
    GET_HIT(victim) += healpoints;

  send_to_char("You feel better!\n\r", victim);
  update_pos(victim);
}

void 
spell_cure_light(byte level, struct char_data *ch,
		 struct char_data *victim, struct obj_data *obj)
{
  int                              healpoints;

  assert(victim);
  assert((level >= 0) && (level <= ABS_MAX_LVL));
  if (DEBUG)
    dlog("spell_cure_light");
  if (HowManyClasses(ch) == 1)
    healpoints = dice(2, 4) + 1;
  else
    healpoints = dice(1, 8);

  if (IS_REALLY_HOLY(ch))
    healpoints += dice(1, 2);
  else if (IS_REALLY_VILE(ch))
    healpoints -= dice(1, 2);
  else if (IS_HOLY(ch))
    healpoints += 1;
  else if (IS_VILE(ch))
    healpoints -= 1;

  if (healpoints < 1)
    healpoints = 1;

  if ((healpoints + GET_HIT(victim)) > hit_limit(victim))
    GET_HIT(victim) = hit_limit(victim);
  else
    GET_HIT(victim) += healpoints;

  send_to_char("You feel better!\n\r", victim);
  update_pos(victim);
}

void 
spell_curse(byte level, struct char_data *ch,
	    struct char_data *victim, struct obj_data *obj)
{
  struct affected_type             af;

  assert(victim || obj);
  assert((level >= 0) && (level <= ABS_MAX_LVL));
  if (DEBUG)
    dlog("spell_curse");
  if (obj) {
    SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD);
    SET_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);

    /*
     * LOWER ATTACK DICE BY -1 
     */
    if (obj->obj_flags.type_flag == ITEM_WEAPON) {
      obj->obj_flags.value[2]--;
      if (obj->obj_flags.value[2] < 0)
	obj->obj_flags.value[2] = 0;
    }
    act("$p glows red.", FALSE, ch, obj, 0, TO_CHAR);
  } else {
    if (saves_spell(victim, SAVING_SPELL) || affected_by_spell(victim, SPELL_CURSE)) {
      if (IS_NPC(victim) && !victim->specials.fighting)
	set_fighting(victim, ch);
      return;
    }
    af.type = SPELL_CURSE;
    af.duration = 24;
    af.modifier = -1;
    af.location = APPLY_HITROLL;
    af.bitvector = AFF_CURSE;
    affect_to_char(victim, &af);

    af.location = APPLY_SAVING_PARA;
    af.modifier = 1;		/*
				 * Make worse 
				 */
    affect_to_char(victim, &af);

    act("$n briefly reveals a red aura!", FALSE, victim, 0, 0, TO_ROOM);
    if (IS_NPC(victim) && !victim->specials.fighting) {
      set_fighting(victim, ch);
      return;
    }
    act("You feel very uncomfortable.", FALSE, victim, 0, 0, TO_CHAR);

  }
}

void 
spell_detect_evil(byte level, struct char_data *ch,
		  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type             af;
  int                              dur;

  assert(victim);
  assert((level >= 0) && (level <= ABS_MAX_LVL));
  if (DEBUG)
    dlog("spell_detect_evil");
  if (affected_by_spell(victim, SPELL_DETECT_EVIL))
    return;

  dur = 20;

  if (IS_REALLY_HOLY(ch))
    dur += dice(1, 8);
  else if (IS_REALLY_VILE(ch))
    dur -= dice(1, 8);
  else if (IS_HOLY(ch))
    dur += dice(1, 4);
  else if (IS_VILE(ch))
    dur -= dice(1, 4);

  af.type = SPELL_DETECT_EVIL;
  af.duration = dur;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_DETECT_EVIL;
  affect_to_char(victim, &af);
  act("$n's eyes briefly glow bright white", FALSE, victim, 0, 0, TO_ROOM);
  send_to_char("Your eyes tingle.\n\r", victim);
}

void 
spell_detect_invisibility(byte level, struct char_data *ch,
		     struct char_data *victim, struct obj_data *obj)
{
  struct affected_type             af;

  assert(ch && victim);
  assert((level >= 0) && (level <= ABS_MAX_LVL));
  if (DEBUG)
    dlog("spell_detect_invisibility");
  if (affected_by_spell(victim, SPELL_DETECT_INVISIBLE))
    return;

  af.type = SPELL_DETECT_INVISIBLE;
  af.duration = 5 + level;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_DETECT_INVISIBLE;

  affect_to_char(victim, &af);
  act("$n's eyes briefly glow yellow", FALSE, victim, 0, 0, TO_ROOM);
  send_to_char("Your eyes tingle.\n\r", victim);
}

void 
spell_detect_magic(byte level, struct char_data *ch,
		   struct char_data *victim, struct obj_data *obj)
{
  struct affected_type             af;

  assert(victim);
  assert((level >= 0) && (level <= ABS_MAX_LVL));
  if (DEBUG)
    dlog("spell_detect_magic");
  if (affected_by_spell(victim, SPELL_DETECT_MAGIC))
    return;

  af.type = SPELL_DETECT_MAGIC;
  af.duration = 2 + level;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_DETECT_MAGIC;

  affect_to_char(victim, &af);
  send_to_char("Your eyes tingle.\n\r", victim);
}

void 
spell_detect_poison(byte level, struct char_data *ch,
		    struct char_data *victim, struct obj_data *obj)
{
  assert(ch && (victim || obj));

  if (DEBUG)
    dlog("spell_detect_poison");
  if (victim) {
    if (victim == ch)
      if (IS_AFFECTED(victim, AFF_POISON))
	send_to_char("You can sense poison in your blood.\n\r", ch);
      else
	send_to_char("You feel healthy.\n\r", ch);
    else if (IS_AFFECTED(victim, AFF_POISON)) {
      act("You sense that $E is poisoned.", FALSE, ch, 0, victim, TO_CHAR);
    } else {
      act("You sense that $E is poisoned", FALSE, ch, 0, victim, TO_CHAR);
    }
  } else {			/*
				 * It's an object 
				 */
    if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
	(obj->obj_flags.type_flag == ITEM_FOOD)) {
      if (obj->obj_flags.value[3])
	act("Poisonous fumes are revealed.", FALSE, ch, 0, 0, TO_CHAR);
      else
	send_to_char("It looks very delicious.\n\r", ch);
    }
  }
}

void 
spell_enchant_weapon(byte level, struct char_data *ch,
		     struct char_data *victim, struct obj_data *obj)
{
  int                              i;

  assert(ch && obj);
  assert(MAX_OBJ_AFFECT >= 2);
  if (DEBUG)
    dlog("spell_enchant_weapon");
  if ((GET_ITEM_TYPE(obj) == ITEM_WEAPON) &&
      IS_NOT_SET(obj->obj_flags.extra_flags, ITEM_MAGIC)) {
    for (i = 0; i < MAX_OBJ_AFFECT; i++)
      if (obj->affected[i].location != APPLY_NONE)
	return;

    SET_BIT(obj->obj_flags.extra_flags, ITEM_MAGIC);
    obj->affected[0].location = APPLY_HITROLL;
    obj->affected[0].modifier = 1;
    if (level > 20)
      obj->affected[0].modifier += 1;
    if (level > 40)
      obj->affected[0].modifier += 1;
    if (level > MAX_MORT)
      obj->affected[0].modifier += 1;

    obj->affected[1].location = APPLY_DAMROLL;
    obj->affected[1].modifier = 1;
    if (level > 15)
      obj->affected[1].modifier += 1;
    if (level > 30)
      obj->affected[1].modifier += 1;
    if (level > MAX_MORT)
      obj->affected[1].modifier += 1;

    if (IS_GOOD(ch)) {
      SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_EVIL);
      act("$p glows blue.", FALSE, ch, obj, 0, TO_CHAR);
    } else if (IS_EVIL(ch)) {
      SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD);
      act("$p glows red.", FALSE, ch, obj, 0, TO_CHAR);
    } else {
      act("$p glows yellow.", FALSE, ch, obj, 0, TO_CHAR);
    }
  }
}

void 
spell_heal(byte level, struct char_data *ch,
	   struct char_data *victim, struct obj_data *obj)
{
  int                              dam;

  assert(victim);
  if (DEBUG)
    dlog("spell_heal");
  dam = (2 * GET_LEVEL(ch, CLERIC_LEVEL_IND));

  if (IS_REALLY_HOLY(ch))
    dam = MIN(200, GET_HIT(ch)) - dice(1, 4);
  else if (IS_REALLY_VILE(ch))
    dam = GET_LEVEL(ch, CLERIC_LEVEL_IND);

  if (HowManyClasses(ch) > 1)
    dam -= dice(HowManyClasses(ch), 4);

  if (dam < 1)
    dam = 1;

  GET_HIT(victim) += dam;

  if (GET_HIT(victim) >= hit_limit(victim))
    GET_HIT(victim) = hit_limit(victim) - dice(1, 4);

  update_pos(victim);
  send_to_char("A deity has smiled down on you....\n\r", victim);
}

void 
spell_invisibility(byte level, struct char_data *ch,
		   struct char_data *victim, struct obj_data *obj)
{
  struct affected_type             af;

  assert((ch && obj) || victim);
  if (DEBUG)
    dlog("spell_invisibility");
  if (obj) {
    if (!IS_SET(obj->obj_flags.extra_flags, ITEM_INVISIBLE)) {
      act("$p turns invisible.", FALSE, ch, obj, 0, TO_CHAR);
      act("$p turns invisible.", TRUE, ch, obj, 0, TO_ROOM);
      SET_BIT(obj->obj_flags.extra_flags, ITEM_INVISIBLE);
    }
  } else {			/*
				 * Then it is a PC | NPC 
				 */
    if (!affected_by_spell(victim, SPELL_INVISIBLE)) {
      act("$n slowly fades out of existence.", TRUE, victim, 0, 0, TO_ROOM);
      send_to_char("You vanish.\n\r", victim);
      af.type = SPELL_INVISIBLE;
      af.duration = 1 + (level / 2);
      af.modifier = -40;
      af.location = APPLY_AC;
      af.bitvector = AFF_INVISIBLE;
      affect_to_char(victim, &af);
    }
  }
}

void 
spell_locate_object(byte level, struct char_data *ch,
		    struct char_data *victim, char *obj)
{
  struct obj_data                 *i;
  char                             name[256];
  char                             buf[MAX_STRING_LENGTH];
  int                              j;

  assert(ch);
  if (DEBUG)
    dlog("spell_locate_object");
  strcpy(name, obj);
  j = level >> 1;

  for (i = object_list; i && (j > 0); i = i->next)
    if (isname(name, i->name)) {
      if (i->carried_by) {
	if (strlen(PERS(i->carried_by, ch)) > 0) {
	  sprintf(buf, "%s carried by %s.\n\r", i->short_description, PERS(i->carried_by, ch));
	  send_to_char(buf, ch);
	}
      } else if (i->equipped_by) {
	if (strlen(PERS(i->equipped_by, ch)) > 0) {
	  sprintf(buf, "%s equipped by %s.\n\r", i->short_description, PERS(i->equipped_by, ch));
	  send_to_char(buf, ch);
	}
      } else if (i->in_obj) {
	sprintf(buf, "%s in %s.\n\r", i->short_description, i->in_obj->short_description);
	send_to_char(buf, ch);
      } else {
	sprintf(buf, "%s in %s.\n\r", i->short_description,
		(i->in_room == NOWHERE ? "use but uncertain." : real_roomp(i->in_room)->name));
	send_to_char(buf, ch);
	j--;
      }
    }
  if (j == 0)
    send_to_char("You are very confused.\n\r", ch);
  if (j == level >> 1)
    send_to_char("No such object.\n\r", ch);
}

void 
spell_poison(byte level, struct char_data *ch,
	     struct char_data *victim, struct obj_data *obj)
{
  struct affected_type             af;

  assert(victim || obj);
  if (DEBUG)
    dlog("spell_poison");
  if (victim) {
    if (!CheckKill(ch, victim))
      return;

    if (IS_NOT_SET(ch->specials.act, ACT_DEADLY)) {
      if (!ImpSaveSpell(victim, SAVING_PARA, 0)) {
	af.type = SPELL_POISON;
	af.duration = 6;
	af.modifier = -2;
	af.location = APPLY_STR;
	af.bitvector = AFF_POISON;
	affect_join(victim, &af, FALSE, FALSE);
	send_to_char("You feel very sick.\n\r", victim);
	if (!victim->specials.fighting)
	  set_fighting(victim, ch);
      } else {
	if (!victim->specials.fighting)
	  set_fighting(victim, ch);
	return;
      }
    } else {
      if (!ImpSaveSpell(victim, SAVING_PARA, 0)) {
	act("Deadly poison fills your veins.", TRUE, ch, 0, 0, TO_CHAR);
	damage(ch, victim, MAX(10, GET_HIT(victim) * 2), SPELL_POISON);
	if (!victim->specials.fighting)
	  set_fighting(victim, ch);
      } else {
	if (!victim->specials.fighting)
	  set_fighting(victim, ch);
	return;
      }
    }
  } else {			/*
				 * Object poison 
				 */
    if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
	(obj->obj_flags.type_flag == ITEM_FOOD)) {
      obj->obj_flags.value[3] = 1;
    }
  }
}

void 
spell_protection_from_evil(byte level, struct char_data *ch,
		     struct char_data *victim, struct obj_data *obj)
{
  struct affected_type             af;

  assert(victim);
  if (DEBUG)
    dlog("spell_protection_from_evil");
  if (!affected_by_spell(victim, SPELL_PROTECT_FROM_EVIL)) {
    af.type = SPELL_PROTECT_FROM_EVIL;
    af.duration = 1 + (level / 2);
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_PROTECT_EVIL;
    affect_to_char(victim, &af);
    send_to_char("You have a righteous feeling!\n\r", victim);
  }
}

void 
spell_remove_curse(byte level, struct char_data *ch,
		   struct char_data *victim, struct obj_data *obj)
{
  struct affected_type             af;

  assert(ch && (victim || obj));

  if (DEBUG)
    dlog("spell_remove_curse");
  if (obj) {
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP)) {
      act("$p briefly glows blue.", TRUE, ch, obj, 0, TO_CHAR);
      REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);
    }
  } else {			/*
				 * Then it is a PC | NPC 
				 */
    if (affected_by_spell(victim, SPELL_CURSE)) {
      act("$n briefly glows red, then blue.", FALSE, victim, 0, 0, TO_ROOM);
      act("You feel better.", FALSE, victim, 0, 0, TO_CHAR);
      affect_from_char(victim, SPELL_CURSE);
    }
  }
}

void 
spell_remove_poison(byte level, struct char_data *ch,
		    struct char_data *victim, struct obj_data *obj)
{

  assert(ch && (victim || obj));

  if (DEBUG)
    dlog("spell_remove_poison");

  if (victim) {
    if (affected_by_spell(victim, SPELL_POISON)) {
      affect_from_char(victim, SPELL_POISON);
      act("A warm feeling runs through your body.", FALSE, victim, 0, 0, TO_CHAR);
      act("$N looks better.", FALSE, ch, 0, victim, TO_ROOM);
    }
  } else {
    if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
	(obj->obj_flags.type_flag == ITEM_FOOD)) {
      obj->obj_flags.value[3] = 0;
      act("The $p steams briefly.", FALSE, ch, obj, 0, TO_CHAR);
    }
  }
}

void 
spell_fireshield(byte level, struct char_data *ch,
		 struct char_data *victim, struct obj_data *obj)
{
  struct affected_type             af;

  if (DEBUG)
    dlog("spell_fireshield");
  if (!affected_by_spell(victim, SPELL_FIRESHIELD)) {
    act("$n is surrounded by a glowing red aura.", TRUE, victim, 0, 0, TO_ROOM);
    act("You start glowing red.", TRUE, victim, 0, 0, TO_CHAR);

    af.type = SPELL_FIRESHIELD;
    af.duration = 2 + level;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_FIRESHIELD;
    affect_to_char(victim, &af);
  }
}

void 
spell_sanctuary(byte level, struct char_data *ch,
		struct char_data *victim, struct obj_data *obj)
{
  struct affected_type             af;

  if (DEBUG)
    dlog("spell_sanctuary");
  if (!affected_by_spell(victim, SPELL_SANCTUARY)) {
    act("$n is surrounded by a white aura.", TRUE, victim, 0, 0, TO_ROOM);
    act("You start glowing.", TRUE, victim, 0, 0, TO_CHAR);

    af.type = SPELL_SANCTUARY;
    af.duration = (level / 2);
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_SANCTUARY;
    affect_to_char(victim, &af);
  }
}

void 
spell_sleep(byte level, struct char_data *ch,
	    struct char_data *victim, struct obj_data *obj)
{
  struct affected_type             af;

  assert(victim);
  if (DEBUG)
    dlog("spell-sleep");
  if (!CheckKill(ch, victim))
    return;

  if (IsImmune(victim, IMM_SLEEP)) {
    FailSleep(victim, ch);
    return;
  }
  if (IsResist(victim, IMM_SLEEP)) {
    if (saves_spell(victim, SAVING_SPELL)) {
      FailSleep(victim, ch);
      return;
    }
    if (saves_spell(victim, SAVING_SPELL)) {
      FailSleep(victim, ch);
      return;
    }
  } else if (!IsSusc(victim, IMM_SLEEP)) {
    if (saves_spell(victim, SAVING_SPELL)) {
      FailSleep(victim, ch);
      return;
    }
  }
  af.type = SPELL_SLEEP;
  af.duration = 1;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_SLEEP;
  affect_join(victim, &af, FALSE, FALSE);

  if (GET_POS(victim) > POSITION_SLEEPING) {
    act("You feel very sleepy .....", FALSE, victim, 0, 0, TO_CHAR);
    act("$n goes to sleep.", TRUE, victim, 0, 0, TO_ROOM);
    GET_POS(victim) = POSITION_SLEEPING;
  }
}

void 
spell_strength(byte level, struct char_data *ch,
	       struct char_data *victim, struct obj_data *obj)
{
  struct affected_type             af;

  assert(victim);
  if (DEBUG)
    dlog("spell_strength");
  if (!affected_by_spell(victim, SPELL_STRENGTH)) {
    act("You feel stronger.", FALSE, victim, 0, 0, TO_CHAR);
    act("$n seems stronger!\n\r", FALSE, victim, 0, 0, TO_ROOM);
    af.type = SPELL_STRENGTH;
    af.duration = 1 + MIN((level / 4), 10) - HowManyClasses(ch);
    if (af.duration < 1)
      af.duration = 1;

    if (IS_NPC(victim))
      af.modifier = number(1, 6);
    else {
      if (HasClass(victim, CLASS_WARRIOR) || HasClass(victim, CLASS_RANGER))
	af.modifier = number(1, 4);
      else if (HasClass(victim, CLASS_CLERIC) ||
	       HasClass(victim, CLASS_THIEF) ||
	       HasClass(victim, CLASS_DRUID))
	af.modifier = number(1, 3);
      else
	af.modifier = number(1, 2);
    }
    af.location = APPLY_STR;
    af.bitvector = 0;
    affect_to_char(victim, &af);
  } else {
    act("Nothing seems to happen.", FALSE, ch, 0, 0, TO_CHAR);
  }
}

void 
spell_ventriloquate(byte level, struct char_data *ch,
		    struct char_data *victim, struct obj_data *obj)
{
  /*
   * Not possible!! No argument! 
   */
}

void 
spell_word_of_recall(byte level, struct char_data *ch,
		     struct char_data *victim, struct obj_data *obj)
{
  extern int                       top_of_world;
  int                              location;
  bool                             found = FALSE;

  assert(victim);
  if (DEBUG)
    dlog("spell_world_or_recall");
  if (IS_NPC(victim))
    return;

  location = GET_HOME(ch);

  if (!real_roomp(location)) {
    send_to_char("You are completely lost.\n\r", victim);
    return;
  }
  /*
   * a location has been found. 
   */

  act("$n disappears.", TRUE, victim, 0, 0, TO_ROOM);
  char_from_room(victim);
  char_to_room(victim, location);
  act("$n appears in the middle of the room.", TRUE, victim, 0, 0, TO_ROOM);
  do_look(victim, "", 15);
}

void 
spell_summon(byte level, struct char_data *ch,
	     struct char_data *victim, struct obj_data *obj)
{
  sh_int                           target;
  struct char_data                *tmp;
  struct obj_data                 *o,
                                  *n;
  int                              j;

  assert(ch && victim);
  if (DEBUG)
    dlog("spell_summon");
  if (IS_NPC(victim) && saves_spell(victim, SAVING_SPELL)) {
    send_to_char("You failed.\n\r", ch);
    return;
  }
  if (IS_SET(victim->specials.new_act, NEW_PLR_SUMMON)) {
    send_to_char("Sorry, that does not want to be summoned.\n\r", ch);
    return;
  }
  if (IS_SET(real_roomp(victim->in_room)->room_flags, NO_SUM)) {
    send_to_char("You cannot penetrate the magical defenses of that area.\n\r", ch);
    return;
  }
  tmp = victim;
  act("$n disappears suddenly.", TRUE, tmp, 0, 0, TO_ROOM);
  target = ch->in_room;

  if (MOUNTED(tmp)) {
    char_from_room(tmp);
    char_from_room(MOUNTED(tmp));
    char_to_room(tmp, target);
    char_to_room(MOUNTED(tmp), target);
    act("$n and a mount slowly fade out of existence.", FALSE, tmp, 0, 0, TO_ROOM);
    act("$n slowly fades into existence.", FALSE, tmp, 0, 0, TO_ROOM);
  } else if (RIDDEN(tmp)) {
    char_from_room(RIDDEN(tmp));
    char_from_room(tmp);
    char_to_room(RIDDEN(tmp), target);
    char_to_room(tmp, target);
    act("$n and a rider slowly fade out of existence.", FALSE, ch, 0, 0, TO_ROOM);
    act("$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM);
  } else {
    char_from_room(tmp);
    char_to_room(tmp, target);
    act("$n slowly fades out of existence.", FALSE, ch, 0, 0, TO_ROOM);
    act("$n slowly fades in to existence.", FALSE, ch, 0, 0, TO_ROOM);
  }

  act("$n arrives suddenly.", TRUE, tmp, 0, 0, TO_ROOM);
  act("$n has summoned you!", FALSE, ch, 0, tmp, TO_VICT);

  if (IS_PC(victim))
    do_look(tmp, "", 15);
  if (IS_NPC(victim)) {
    if (MOUNTED(victim))
      if (IS_NPC(MOUNTED(victim)))
	set_fighting(victim, ch);

    if (RIDDEN(victim))
      if (IS_NPC(RIDDEN(victim)))
	set_fighting(victim, ch);
  }
}

void 
spell_charm_person(byte level, struct char_data *ch,
		   struct char_data *victim, struct obj_data *obj)
{
  struct affected_type             af;

  void                             add_follower(struct char_data *ch, struct char_data *leader);
  bool                             circle_follow(struct char_data *ch, struct char_data *victim);
  void                             stop_follower(struct char_data *ch);

  assert(ch && victim);
  if (victim == ch) {
    send_to_char("You like yourself even better!\n\r", ch);
    return;
  }
  if (IS_PC(victim)) {
    send_to_char("You can no longer charm players.\n\r", ch);
    return;
  }
  if (!CheckKill(ch, victim))
    return;

  if (!IS_AFFECTED(victim, AFF_CHARM) && !IS_AFFECTED(ch, AFF_CHARM)) {
    if (circle_follow(victim, ch)) {
      send_to_char("Sorry, following in circles can not be allowed.\n\r", ch);
      return;
    }
    if (!IsPerson(victim)) {
      send_to_char("Umm,  that's not a person....\n\r", ch);
      return;
    }
    if (IsImmune(victim, IMM_CHARM)) {
      FailCharm(victim, ch);
      return;
    }
    if (IsResist(victim, IMM_CHARM)) {
      if (saves_spell(victim, SAVING_PARA)) {
	FailCharm(victim, ch);
	return;
      }
      if (saves_spell(victim, SAVING_PARA)) {
	FailCharm(victim, ch);
	return;
      }
    } else {
      if (!IsSusc(victim, IMM_CHARM)) {
	if (saves_spell(victim, SAVING_PARA)) {
	  FailCharm(victim, ch);
	  return;
	}
      }
    }

    if (victim->master)
      stop_follower(victim);

    add_follower(victim, ch);

    af.type = SPELL_CHARM_PERSON;

    if (GET_INT(victim))
      af.duration = 20 - GET_INT(victim);
    else
      af.duration = 12;

    af.modifier = 0;
    af.location = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char(victim, &af);

    send_to_char("They are now charmed.....\n\r", ch);
    act("Isn't $n just such a nice fellow?", FALSE, ch, 0, victim, TO_VICT);
  }
}

void 
spell_charm_monster(byte level, struct char_data *ch,
		    struct char_data *victim, struct obj_data *obj)
{
  struct affected_type             af;

  void                             add_follower(struct char_data *ch, struct char_data *leader);
  bool                             circle_follow(struct char_data *ch, struct char_data *victim);
  void                             stop_follower(struct char_data *ch);

  assert(ch && victim);

  if (victim == ch) {
    send_to_char("You like yourself even better!\n\r", ch);
    return;
  }
  if (!CheckKill(ch, victim))
    return;

  if (!IS_AFFECTED(victim, AFF_CHARM) && !IS_AFFECTED(ch, AFF_CHARM)) {
    if (circle_follow(victim, ch)) {
      send_to_char("Sorry, following in circles can not be allowed.\n\r", ch);
      return;
    }
    if (IsImmune(victim, IMM_CHARM)) {
      FailCharm(victim, ch);
      return;
    }
    if (IsResist(victim, IMM_CHARM)) {
      if (saves_spell(victim, SAVING_PARA)) {
	FailCharm(victim, ch);
	return;
      }
      if (saves_spell(victim, SAVING_PARA)) {
	FailCharm(victim, ch);
	return;
      }
    } else {
      if (!IsSusc(victim, IMM_CHARM)) {
	if (saves_spell(victim, SAVING_PARA)) {
	  FailCharm(victim, ch);
	  return;
	}
      }
    }

    if (victim->master)
      stop_follower(victim);

    add_follower(victim, ch);

    af.type = SPELL_CHARM_PERSON;

    if (GET_INT(victim))
      af.duration = 20 - GET_INT(victim);
    else
      af.duration = 14;

    af.modifier = 0;
    af.location = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char(victim, &af);

    act("Isn't $n just such a nice fellow?", FALSE, ch, 0, victim, TO_VICT);
  }
}

void 
spell_sense_life(byte level, struct char_data *ch,
		 struct char_data *victim, struct obj_data *obj)
{
  struct affected_type             af;

  assert(victim);

  if (DEBUG)
    dlog("spell_sense_life");
  if (!affected_by_spell(victim, SPELL_SENSE_LIFE)) {
    send_to_char("Your feel your awareness improve.\n\r", ch);

    af.type = SPELL_SENSE_LIFE;
    af.duration = 1 + (level / 2);
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_SENSE_LIFE;
    affect_to_char(victim, &af);
  }
}

/*
 * ***************************************************************************
 * *                     Not cast-able spells                                  *
 * * ************************************************************************* 
 */

void                             sprintbit(unsigned long, char *[], char *);

void 
spell_identify(byte level, struct char_data *ch,
	       struct char_data *victim, struct obj_data *obj)
{
  char                             buf[256],
                                   buf2[256];
  int                              i;
  bool                             found;
  int                              index;

  struct time_info_data            age(struct char_data *ch);

  /*
   * Spell Names 
   */
  extern char                     *spells[];

  /*
   * For Objects 
   */
  extern char                     *item_types[];
  extern char                     *extra_bits[];
  extern char                     *apply_types[];
  extern char                     *affected_bits[];

  assert(ch && (obj || victim));
  if (DEBUG)
    dlog("spell_indentify");

  if (obj) {
    send_to_char("You feel informed:\n\r", ch);
    sprintf(buf, "Object '%s', Item type: ", obj->name);
    sprinttype(GET_ITEM_TYPE(obj), item_types, buf2);
    strcat(buf, buf2);
    strcat(buf, "\n\r");
    send_to_char(buf, ch);

    if (obj->obj_flags.bitvector) {
      send_to_char("Item aids in abilities:  ", ch);
      sprintbit(obj->obj_flags.bitvector, affected_bits, buf);
      strcat(buf, "\n\r");
      send_to_char(buf, ch);
    }
    send_to_char("Item is: ", ch);
    sprintbit(obj->obj_flags.extra_flags, extra_bits, buf);
    strcat(buf, "\n\r");
    send_to_char(buf, ch);

    sprintf(buf, "Weight: %d, Value: %d\n\r", obj->obj_flags.weight, obj->obj_flags.cost);
    send_to_char(buf, ch);

    switch (GET_ITEM_TYPE(obj)) {
    case ITEM_SCROLL:
    case ITEM_POTION:
      sprintf(buf, "Level %d spells of:\n\r", obj->obj_flags.value[0]);
      send_to_char(buf, ch);
      for (index = 1; index < 4; index++)
	if (obj->obj_flags.value[index] >= 1) {
	  sprinttype(obj->obj_flags.value[index] - 1, spells, buf);
	  strcat(buf, "\n\r");
	  send_to_char(buf, ch);
	}
      break;

    case ITEM_WAND:
    case ITEM_STAFF:
      sprintf(buf, "Has %d chages, with %d charges left.\n\r",
	      obj->obj_flags.value[1],
	      obj->obj_flags.value[2]);
      send_to_char(buf, ch);

      sprintf(buf, "Level %d spell of:\n\r", obj->obj_flags.value[0]);
      send_to_char(buf, ch);

      if (obj->obj_flags.value[3] >= 1) {
	sprinttype(obj->obj_flags.value[3] - 1, spells, buf);
	strcat(buf, "\n\r");
	send_to_char(buf, ch);
      }
      break;

    case ITEM_WEAPON:
      sprintf(buf, "Damage Dice is '%dD%d'\n\r",
	      obj->obj_flags.value[1],
	      obj->obj_flags.value[2]);
      send_to_char(buf, ch);
      break;

    case ITEM_ARMOR:
      sprintf(buf, "AC-apply is %d\n\r", obj->obj_flags.value[0]);
      send_to_char(buf, ch);
      break;
    }

    found = FALSE;

    for (i = 0; i < MAX_OBJ_AFFECT; i++) {
      if ((obj->affected[i].location != APPLY_NONE) && (obj->affected[i].modifier != 0)) {
	if (!found) {
	  send_to_char("Can affect you as :\n\r", ch);
	  found = TRUE;
	}
	sprinttype(obj->affected[i].location, apply_types, buf2);
	sprintf(buf, "    Affects : %s By %d\n\r", buf2, obj->affected[i].modifier);
	send_to_char(buf, ch);
      }
    }
  }
}

/*
 * ***************************************************************************
 * *                     NPC spells..                                          *
 * * ************************************************************************* 
 */

void 
spell_fire_breath(byte level, struct char_data *ch,
		  struct char_data *victim, struct obj_data *obj)
{
  int                              dam;
  int                              hpch;
  struct obj_data                 *burn;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  hpch = GET_MAX_HIT(ch);
  hpch *= level;
  hpch /= GetMaxLevel(ch);
  if (hpch < 10)
    hpch = 10;

  dam = hpch;

  if (saves_spell(victim, SAVING_BREATH))
    dam >>= 1;

  damage(ch, victim, dam, SPELL_FIRE_BREATH);

  /*
   * And now for the damage on inventory 
   */

/*
 * DamageStuff(victim, FIRE_DAMAGE);
 */

  for (burn = victim->carrying;
       burn && (burn->obj_flags.type_flag != ITEM_SCROLL) &&
       (burn->obj_flags.type_flag != ITEM_WAND) &&
       (burn->obj_flags.type_flag != ITEM_STAFF) &&
       (burn->obj_flags.type_flag != ITEM_BOAT);
       burn = burn->next_content) {
    if (!saves_spell(victim, SAVING_BREATH)) {
      if (burn) {
	act("$o burns", 0, victim, burn, 0, TO_CHAR);
	extract_obj(burn);
      }
    }
  }
}

void 
spell_frost_breath(byte level, struct char_data *ch,
		   struct char_data *victim, struct obj_data *obj)
{
  int                              dam;
  int                              hpch;
  struct obj_data                 *frozen;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  hpch = GET_MAX_HIT(ch);
  hpch *= level;
  hpch /= GetMaxLevel(ch);
  if (hpch < 10)
    hpch = 10;

  dam = hpch;

  if (saves_spell(victim, SAVING_BREATH))
    dam >>= 1;

  damage(ch, victim, dam, SPELL_FROST_BREATH);

  /*
   * And now for the damage on inventory 
   */

  for (frozen = victim->carrying;
       frozen && (frozen->obj_flags.type_flag != ITEM_DRINKCON) &&
       (frozen->obj_flags.type_flag != ITEM_POTION);
       frozen = frozen->next_content) {
    if (!saves_spell(victim, SAVING_BREATH)) {
      if (frozen) {
	act("$o shatters.", 0, victim, frozen, 0, TO_CHAR);
	extract_obj(frozen);
      }
    }
  }
}

void 
spell_acid_breath(byte level, struct char_data *ch,
		  struct char_data *victim, struct obj_data *obj)
{
  int                              dam;
  int                              hpch;
  int                              damaged;

  int                              apply_ac(struct char_data *ch, int eq_pos);

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  hpch = GET_MAX_HIT(ch);
  hpch *= level;
  hpch /= GetMaxLevel(ch);
  if (hpch < 10)
    hpch = 10;

  dam = hpch;

  if (saves_spell(victim, SAVING_BREATH))
    dam >>= 1;

  damage(ch, victim, dam, SPELL_ACID_BREATH);

}

void 
spell_gas_breath(byte level, struct char_data *ch,
		 struct char_data *victim, struct obj_data *obj)
{
  int                              dam;
  int                              hpch;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  hpch = GET_MAX_HIT(ch);
  hpch *= level;
  hpch /= GetMaxLevel(ch);
  if (hpch < 10)
    hpch = 10;

  dam = hpch;

  if (saves_spell(victim, SAVING_BREATH))
    dam >>= 1;

  damage(ch, victim, dam, SPELL_GAS_BREATH);

}

void 
spell_lightning_breath(byte level, struct char_data *ch,
		     struct char_data *victim, struct obj_data *obj)
{
  int                              dam;
  int                              hpch;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  hpch = GET_MAX_HIT(ch);
  hpch *= level;
  hpch /= GetMaxLevel(ch);
  if (hpch < 10)
    hpch = 10;

  dam = hpch;

  if (saves_spell(victim, SAVING_BREATH))
    dam >>= 1;

  damage(ch, victim, dam, SPELL_LIGHTNING_BREATH);

}

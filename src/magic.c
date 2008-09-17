/*
 * file: magic.c , Implementation of spells.              Part of DIKUMUD
 * Usage : The actual effect of magic.
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */

#include <stdio.h>
#include <stdlib.h>
/* #include <unistd.h> */
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <assert.h>

#include "global.h"
#include "bug.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "mudlimits.h"
#include "db.h"
#include "constants.h"
#include "spells.h"
#include "spell_parser.h"
#include "multiclass.h"
#include "fight.h"
#include "opinion.h"
#include "reception.h"
#include "magic_utils.h"
#include "act_off.h"
#include "act_obj.h"
#include "act_info.h"

void spell_armor(char level, struct char_data *ch, struct char_data *victim,
		 struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!victim || level < 0 || level > ABS_MAX_LVL)
    return;

  if (!affected_by_spell(victim, SPELL_ARMOR)) {
    af.type = SPELL_ARMOR;
    af.duration = GET_LEVEL(ch, BestMagicClass(ch));
    af.modifier = -20;
    af.location = APPLY_AC;
    af.bitvector = 0;

    affect_to_char(victim, &af);
    cprintf(victim, "You feel a strong protective force shimmer all around you.\n\r");
  } else {
    cprintf(ch, "You feel confident that you are still protected.\n\r");
  }
}

void spell_teleport(char level, struct char_data *ch, struct char_data *victim,
		    struct obj_data *obj)
{
  int                                     to_room = 0;
  struct room_data                       *room = NULL;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim)
    return;
  if (IS_SET(victim->specials.new_act, NEW_PLR_TELEPORT)) {
    cprintf(ch, "You try to send it away, but it just smirks at you.\n\r");
    return;
  }
  if (victim != ch) {
    if (saves_spell(victim, SAVING_SPELL)) {
      cprintf(ch, "Your target remains firmly in front of you.\n\r");
      if (IS_NPC(victim)) {
	if (!victim->specials.fighting)
	  set_fighting(victim, ch);
      } else {
	cprintf(victim, "You feel like you were somewhere else for just a moment.\n\r");
      }
      return;
    } else {
      ch = victim;					       /* the character (target) is now the victim */
    }
  }
  do {
    to_room = number(0, top_of_world);
    room = real_roomp(to_room);
    if (room) {
      if (IS_SET(room->room_flags, PRIVATE)
	  || IS_SET(room->room_flags, NO_SUM))
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

void spell_bless(char level, struct char_data *ch, struct char_data *victim,
		 struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || (!victim && !obj) || level < 0 || level > ABS_MAX_LVL)
    return;
  if (obj) {
    if ((10 * GET_LEVEL(ch, CLERIC_LEVEL_IND) > GET_OBJ_WEIGHT(obj)) &&
	(GET_POS(ch) != POSITION_FIGHTING) && !IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)) {
      SET_BIT(obj->obj_flags.extra_flags, ITEM_BLESS);
      act("$p shimmers with light for a second.", FALSE, ch, obj, 0, TO_CHAR);
    }
  } else {
    if ((GET_POS(victim) != POSITION_FIGHTING) && (!affected_by_spell(victim, SPELL_BLESS))) {
      cprintf(victim, "You feel righteous.\n\r");
      af.type = SPELL_BLESS;
      af.duration = level / 2;
      af.modifier = 1;
      af.location = APPLY_HITROLL;
      af.bitvector = 0;
      affect_to_char(victim, &af);
      af.location = APPLY_SAVING_SPELL;
      af.modifier = -1;					       /* Make better */
      affect_to_char(victim, &af);
    }
  }
}

void spell_blindness(char level, struct char_data *ch, struct char_data *victim,
		     struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim || level < 0 || level > ABS_MAX_LVL)
    return;
  if (!CheckKill(ch, victim))
    return;

  if (saves_spell(victim, SAVING_SPELL) || affected_by_spell(victim, SPELL_BLINDNESS)) {
    if ((!victim->specials.fighting) && (victim != ch))
      set_fighting(victim, ch);
    return;
  }
  act("$n clutches $s eyes and fumbles about!", TRUE, victim, 0, 0, TO_ROOM);
  cprintf(victim, "You have been blinded!\n\r");

  af.type = SPELL_BLINDNESS;
  af.location = APPLY_HITROLL;
  af.modifier = -4;					       /* Make hitroll worse */
  af.duration = dice((level / 5) + 1, 4);
  af.bitvector = AFF_BLIND;
  affect_to_char(victim, &af);
  af.location = APPLY_AC;
  af.modifier = +20;					       /* Make AC Worse! */
  affect_to_char(victim, &af);

  if ((!victim->specials.fighting) && (victim != ch))
    set_fighting(victim, ch);
}

void spell_burning_hands(char level, struct char_data *ch, struct char_data *victim,
			 struct obj_data *obj)
{
  int                                     dam = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || level < 0 || level > ABS_MAX_LVL)
    return;
  dam = dice(1, 6) + MIN(level * 2, 30);

  if (dam < 1)
    dam = 1;

  cprintf(ch, "Searing flame fans out in front of you!\n\r");
  act("$n sends a fan of flame shooting from $s fingertips!\n\r", FALSE, ch, 0, 0, TO_ROOM);
  AreaEffectSpell(ch, dam, SPELL_BURNING_HANDS, 0, 0);
}

void spell_call_lightning(char level, struct char_data *ch, struct char_data *victim,
			  struct obj_data *obj)
{
  int                                     dam = 0;
  int                                     added_dam = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim || level < 0 || level > ABS_MAX_LVL)
    return;
  if (!CheckKill(ch, victim))
    return;

  dam = dice((level / 4) + 1, 6);
  added_dam = dice((level / 9) + 1, 8);

  if (IS_REALLY_VILE(ch) || IS_REALLY_HOLY(ch)
      || (OUTSIDE(ch) && (weather_info.sky >= SKY_RAINING))) {
    if (saves_spell(victim, SAVING_SPELL))
      dam >>= 1;
    dam += added_dam;
    damage(ch, victim, dam, SPELL_CALL_LIGHTNING);
  }
}

void spell_charm_person(char level, struct char_data *ch, struct char_data *victim,
			struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim)
    return;
  if (victim == ch) {
    cprintf(ch, "You like yourself even better!\n\r");
    return;
  }
  if (!CheckKill(ch, victim))
    return;

  if (!IS_AFFECTED(victim, AFF_CHARM) && !IS_AFFECTED(ch, AFF_CHARM)) {
    if (circle_follow(victim, ch)) {
      cprintf(ch, "Sorry, following in circles can not be allowed.\n\r");
      return;
    }
    if (!IsPerson(victim)) {
      cprintf(ch, "Umm,  that's not a person....\n\r");
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

    cprintf(ch, "They are now charmed.....\n\r");
    act("Isn't $n just such a nice fellow?", FALSE, ch, 0, victim, TO_VICT);
  }
}

void spell_chill_touch(char level, struct char_data *ch, struct char_data *victim,
		       struct obj_data *obj)
{
  struct affected_type                    af;
  int                                     dam = 0;
  int                                     lvl = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim || level < 0 || level > ABS_MAX_LVL)
    return;
  if (!CheckKill(ch, victim))
    return;

  if (IsUndead(victim)) {
    if (!saves_spell(victim, SAVING_SPELL)) {
/* 
 * af.type      = SPELL_FEAR;
 * af.duration  = 4+level;
 * af.modifier  = 0;
 * af.location  = APPLY_NONE;
 * af.bitvector = 0;
 * affect_join(victim, &af, FALSE, FALSE);
 */
      do_flee(victim, "", 0);
    }
    return;
  }
  lvl = GET_LEVEL(ch, MAGE_LEVEL_IND);
  dam = dice(((lvl + 1) / 2) - 1, 4);
  af.type = SPELL_CHILL_TOUCH;
  af.duration = lvl / 3;
  af.modifier = -1 - lvl / 20;
  af.location = APPLY_STR;
  af.bitvector = 0;
  affect_join(victim, &af, TRUE, FALSE);

  if (saves_spell(victim, SAVING_SPELL)) {
    dam = 0;
  }
  damage(ch, victim, dam, SPELL_CHILL_TOUCH);
}

/* First implementation attempt by Quixadhal, 95.12.05
 *
 * The basic idea is this:
 * We attempt to create an exact duplicate of player/mob X.
 *
 * If the target is in the room, there is a base 50/50 chance of sucess.
 * If the target makes a save vs. magic, then the spell fails.
 * If the target is not in the room, they get an additional save vs. magic to
 * determine if their items could be duplicated or not.  (IE: could the spell
 * divine the exact items the target had).
 * Each item also gets a saving throw for duplication... magical items will
 * be duplicated without the enchantment.  IE: an enchanted claymore which
 * was 1d10+4 would now be a normal 1d10 claymore.
 *
 * Once duplication is complete, the clone gets a save vs. paralysis to
 * determine if it remains under the caster's control.  If so, it acts like
 * a charmed monster until the duration expires.  If not, it will immediately
 * attack the caster, and then hunt down the copy of itself (if online).  Once
 * those are both dead, it will be a normal wandering aggressive monster.
 *
 * Note:  we must write a special routine to control this behaviour.
 */
void spell_clone(char level, struct char_data *ch, struct char_data *victim,
		 struct obj_data *obj)
{
  int                                     i = 0;
  int                                     diff = 0;
  int                                     chance = 0;
  int                                     snarl = 0;
  struct char_data                       *clone = NULL;
  struct affected_type                   *af = NULL;
  struct obj_data                        *tobj = NULL;
  struct follow_type                     *k = NULL;
  char                                    buf[MAX_STRING_LENGTH] = "\0\0\0";
  struct affected_type                    aff;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || (!victim && !obj) || level < 0 || level > ABS_MAX_LVL)
    return;

/* required object is a small glass mirror, #1133 */

  if (!victim) {
    cprintf(ch, "But whom do you wish to clone?\n\r");
    return;
  }
  if (IS_IMMORTAL(victim)) {
    cprintf(ch, "Nonononononono!  That would be suicide!!!!\n\r");
    return;
  }
  if (ch == victim) {
    cprintf(ch, "You don't like yourself enough to do this.\n\r");
    return;
  }
  if (IS_NPC(victim) && (victim->in_room != ch->in_room)
      && (GetMaxLevel(victim) > (GetMaxLevel(ch) / 2))) {
    cprintf(ch, "You can't seem to get a solid lock on %s.\n\r", GET_SDESC(victim));
    return;
  }

  chance = 50;
  diff = GetMaxLevel(ch) - GetMaxLevel(victim);
  if (diff > 0)
    chance += 5 * diff;
  else
    chance -= 10 * diff;
  if (victim->in_room != ch->in_room)
    chance /= 2;
  chance = MIN(MAX(1, chance), 95);

  if (GetMaxLevel(ch) < LOKI) {
    cprintf(ch, "Sorry!  Clone is not quite ready yet.\n\r");
    cprintf(ch, "You would have a %d%% chance of it working though!\n\r", chance);
    return;
  }

  if (number(1, 100) > chance) {
    cprintf(ch, "You fail miserably, and scrape glass from your scalp.\n\r");
    act("$n bitches and moans as the spell fizzles.", FALSE, ch, 0, 0, TO_ROOM);
    return;
  }
  CREATE(clone, struct char_data, 1);
  clear_char(clone);
  clone->nr = victim->nr;				       /* this may get changed later */
  clone->in_room = ch->in_room;				       /* summons clone to caster, normally */
  clone->mail = NULL;
  clone->immune = victim->immune;
  clone->M_immune = victim->M_immune;
  clone->susc = victim->susc;
  clone->mult_att = victim->mult_att;
  clone->attackers = 0;					       /* no one is fighting us, yet... */
  clone->fallspeed = 0;					       /* used when falling? */
  clone->race = victim->race;
  clone->hunt_dist = victim->hunt_dist;
  clone->hatefield = victim->hatefield;
  clone->fearfield = victim->fearfield;
  clone->hates = victim->hates;
  clone->hates.clist = NULL;				       /* don't duplicate the whole list */
  clone->fears = victim->fears;
  clone->fears.clist = NULL;				       /* don't duplicate the whole list */
  clone->persist = victim->persist;
  clone->old_room = victim->old_room;
  clone->act_ptr = victim->act_ptr;
  clone->player = victim->player;
  if (victim->player.name)
    clone->player.name = strdup(victim->player.name);
  if (victim->player.short_descr)
    clone->player.short_descr = strdup(victim->player.short_descr);
  else
    clone->player.short_descr = strdup(clone->player.name);
  if (victim->player.long_descr)
    clone->player.long_descr = strdup(victim->player.long_descr);
  else {
    CREATE(clone->player.long_descr, char,
	   strlen(victim->player.name) + strlen(victim->player.title) + 22);
    strcpy(clone->player.long_descr, victim->player.name);
    strcat(clone->player.long_descr, " ");
    strcat(clone->player.long_descr, victim->player.title);
    strcat(clone->player.long_descr, " is standing here.\n\r");
  }
  if (victim->player.description)
    clone->player.description = strdup(victim->player.description);
  if (victim->player.pre_title)
    clone->player.pre_title = strdup(victim->player.pre_title);
  if (victim->player.title)
    clone->player.title = strdup(victim->player.title);
  if (victim->player.poof_in)
    clone->player.poof_in = strdup(victim->player.poof_in);
  if (victim->player.poof_out)
    clone->player.poof_out = strdup(victim->player.poof_out);
  if (victim->player.guild_name)
    clone->player.guild_name = strdup(victim->player.guild_name);
  if (victim->player.sounds)
    clone->player.sounds = strdup(victim->player.sounds);
  if (victim->player.distant_snds)
    clone->player.distant_snds = strdup(victim->player.distant_snds);
  clone->abilities = victim->abilities;
  clone->tmpabilities = victim->tmpabilities;
  clone->points = victim->points;
  clone->specials = victim->specials;
  clone->specials.fighting = NULL;
  clone->specials.hunting = NULL;			       /* later, we hunt our original */
  clone->specials.mounted_on = NULL;
  clone->specials.ridden_by = NULL;
  for (i = 0; i < MAX_SKILLS; i++)
    clone->skills[i] = victim->skills[i];
  for (af = victim->affected; af; af = af->next)
    affect_to_char(clone, af);
/* struct obj_data *equipment[MAX_WEAR] */
  for (i = 0; i < MAX_WEAR; i++)
    if (victim->equipment[i]) {
      if (number(1, 100) > chance) {
	continue;
      }
      if (!(tobj = read_object(victim->equipment[i]->item_number, REAL)))
	continue;
      /*
       * obj_to_char(obj, clone); 
       */
      tobj->carried_by = NULL;
      tobj->in_room = NOWHERE;
      clone->equipment[i] = tobj;
      tobj->equipped_by = clone;
      tobj->eq_pos = i;
    } else
      clone->equipment[i] = NULL;
/* struct obj_data *carrying */
  clone->carrying = NULL;
  clone->desc = NULL;
  clone->next_in_room = NULL;
  clone->next = NULL;
  clone->next_fighting = NULL;
  clone->followers = NULL;
  clone->master = NULL;
  clone->invis_level = 0;

/* now we are copied... let's fix things */
  GET_GOLD(clone) = 1;
  GET_BANK(clone) = 0;
  GET_EXP(clone) = GetMaxLevel(clone) * 100;

/* make sure they are now an NPC */
  if (IS_NPC(clone))
    mob_index[clone->nr].number++;
  else {
    clone->nr = -1;
    SET_BIT(clone->specials.act, ACT_ISNPC);
  }
/* make them hate the original they were cloned from */
  AddHated(clone, victim);

/* make them aggressive */
  SET_BIT(clone->specials.act, ACT_AGGRESSIVE);
  SET_BIT(clone->specials.act, ACT_SCAVENGER);
  SET_BIT(clone->specials.act, ACT_USE_ITEM);
  SET_BIT(clone->specials.new_act, NEW_PLR_KILLOK);

/* add them to the world character list */
  clone->next = character_list;
  character_list = clone;

  snarl = 0;
/* now make them be charmed */
  if (ch->in_room != victim->in_room)
    if (number(1, 100) > chance) {
      char_to_room(clone, ch->in_room);
      FailCharm(clone, ch);
      snarl = 1;
    }
  if (circle_follow(clone, ch)) {
    char_to_room(clone, ch->in_room);
    FailCharm(clone, ch);
    snarl = 1;
  }
  if (IsImmune(clone, IMM_CHARM)) {
    char_to_room(clone, ch->in_room);
    FailCharm(clone, ch);
    snarl = 1;
  }
  if (IsResist(clone, IMM_CHARM)) {
    if (saves_spell(clone, SAVING_PARA)) {
      char_to_room(clone, ch->in_room);
      FailCharm(clone, ch);
      snarl = 1;
    }
  } else {
    if (!IsSusc(clone, IMM_CHARM)) {
      if (number(1, 100) > chance) {
	char_to_room(clone, ch->in_room);
	FailCharm(clone, ch);
	snarl = 1;
      }
    } else {
      if (number(1, 100) > MAX(chance + chance / 2, 97)) {
	char_to_room(clone, ch->in_room);
	FailCharm(clone, ch);
	snarl = 1;
      }
    }
  }
  if (!snarl) {
    clone->master = ch;
    CREATE(k, struct follow_type, 1);

    k->follower = clone;
    k->next = ch->followers;
    ch->followers = k;

    aff.type = SPELL_CHARM_PERSON;
    if (GET_INT(clone))
      aff.duration = MAX(1, GET_LEVEL(ch, BestMagicClass(ch)) - GET_INT(clone));
    else
      aff.duration = 6;
    aff.modifier = 0;
    aff.location = 0;
    aff.bitvector = AFF_CHARM;
    affect_to_char(clone, &aff);

    char_to_room(clone, ch->in_room);
  }
  if (IS_PC(victim)) {
    cprintf(ch, "You create a duplicate of %s.\n\r", GET_NAME(victim));
    sprintf(buf, "$n creates a duplicate of %s.", GET_NAME(victim));
  } else {
    cprintf(ch, "You create a duplicate of %s.\n\r", GET_SDESC(victim));
    sprintf(buf, "$n creates a duplicate of %s.", GET_SDESC(victim));
  }
  act("%s", FALSE, ch, 0, 0, TO_ROOM, buf);
  if (snarl) {
    if (IS_PC(victim)) {
      cprintf(ch, "%s turns to you and snarls.\n\r", GET_NAME(victim));
      sprintf(buf, "%s snarls at $n.", GET_NAME(victim));
    } else {
      cprintf(ch, "%s turns to you and snarls.\n\r", GET_SDESC(victim));
      sprintf(buf, "%s snarls at $n.", GET_SDESC(victim));
    }
    act("%s", FALSE, ch, 0, 0, TO_ROOM, buf);
  }
}

void spell_colour_spray(char level, struct char_data *ch, struct char_data *victim,
			struct obj_data *obj)
{
  int                                     dam = 0;
  struct char_data                       *tmp_victim = NULL;
  struct char_data                       *temp = NULL;
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || level < 0 || level > ABS_MAX_LVL)
    return;

  dam = dice(1, 6);

  for (tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
    temp = tmp_victim->next;
    if ((ch->in_room == tmp_victim->in_room) &&
	(ch != tmp_victim) &&
	(!((IS_AFFECTED(tmp_victim, AFF_GROUP) && (((tmp_victim == ch->master)
						    || (tmp_victim->master == ch)
						    || (tmp_victim->master == ch->master))))))
	&& CheckKill(ch, tmp_victim)) {
      if (IS_PC(tmp_victim) && IS_IMMORTAL(tmp_victim) && !IS_IMMORTAL(ch)) {
	cprintf(tmp_victim, "Some puny mortal tries to blind you with color spray\n\r");
      } else {
	damage(ch, tmp_victim, dam, SPELL_COLOUR_SPRAY);

	if (GetMaxLevel(tmp_victim) <= (GET_LEVEL(ch, MAGE_LEVEL_IND) / 2)) {
	  if (!saves_spell(tmp_victim, SAVING_SPELL)) {
	    if (!affected_by_spell(tmp_victim, SPELL_PARALYSIS)) {
	      act("$n is captivated by the brilliance of the rays!", TRUE, tmp_victim, 0, 0,
		  TO_ROOM);
	      cprintf(tmp_victim, "You cannot take your gaze from the brilliant ray!\n\r");
	      af.type = SPELL_PARALYSIS;
	      af.duration = dice(1, 6);
	      af.modifier = 0;
	      af.location = APPLY_NONE;
	      af.bitvector = AFF_PARALYSIS;
	      affect_join(tmp_victim, &af, FALSE, FALSE);

	      if ((!tmp_victim->specials.fighting) && (tmp_victim != ch))
		set_fighting(tmp_victim, ch);
	    }
	  }
	} else if (GetMaxLevel(tmp_victim) <= (GET_LEVEL(ch, MAGE_LEVEL_IND) + 3)) {
	  if (!saves_spell(tmp_victim, SAVING_SPELL)) {
	    if (!affected_by_spell(tmp_victim, SPELL_BLINDNESS)) {
	      act("$n fumbles in the brilliance of the rays!", TRUE, tmp_victim, 0, 0, TO_ROOM);
	      cprintf(tmp_victim, "You have been blinded by a brilliant ray!\n\r");
	      af.type = SPELL_BLINDNESS;
	      af.location = APPLY_HITROLL;
	      af.modifier = -4;
	      af.duration = dice(2, 4);
	      af.bitvector = AFF_BLIND;
	      affect_to_char(tmp_victim, &af);
	      af.location = APPLY_AC;
	      af.modifier = +20;			       /* Make AC Worse! */
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

void spell_control_weather(char level, struct char_data *ch, struct char_data *victim,
			   struct obj_data *obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));
  /*
   * Control Weather is not possible here!!! 
   */
  /*
   * Better/Worse can not be transferred 
   */
}

void spell_create_food(char level, struct char_data *ch, struct char_data *victim,
		       struct obj_data *obj)
{
  struct obj_data                        *tmp_obj = NULL;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || level < 0 || level > ABS_MAX_LVL)
    return;
  CREATE(tmp_obj, struct obj_data, 1);

  clear_object(tmp_obj);

  if (number(0, 99) < 20) {
    tmp_obj->name = (char *)strdup("dung shit");
    tmp_obj->short_description = (char *)strdup("Some Dried Dung");
    tmp_obj->description =
      (char *)strdup("A noxious smelling, but edible, buffalo chip sits here.");
  } else if (number(0, 99) < 40) {
    tmp_obj->name = (char *)strdup("cookie");
    tmp_obj->short_description = (char *)strdup("A Cookie");
    tmp_obj->description =
      (char *)strdup("A crisp cookie with strawberry filling waits for you here!");
  } else {
    tmp_obj->name = (char *)strdup("mushroom");
    tmp_obj->short_description = (char *)strdup("A Magic Mushroom");
    tmp_obj->description =
      (char *)strdup("A really delicious looking magic mushroom lies here.");
  }

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

void spell_create_water(char level, struct char_data *ch, struct char_data *victim,
			struct obj_data *obj)
{
  int                                     water = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !obj || level < 0 || level > ABS_MAX_LVL)
    return;
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

void spell_cure_blind(char level, struct char_data *ch, struct char_data *victim,
		      struct obj_data *obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!victim || level < 0 || level > ABS_MAX_LVL)
    return;
  if (affected_by_spell(victim, SPELL_BLINDNESS)) {
    affect_from_char(victim, SPELL_BLINDNESS);
    cprintf(victim, "Your vision returns!\n\r");
  }
}

/* Offensive Spells */

void spell_magic_missile(char level, struct char_data *ch, struct char_data *victim,
			 struct obj_data *obj)
{
  int                                     dam = 0;
  int                                     missiles = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim || level < 0 || level > ABS_MAX_LVL)
    return;

  if (!CheckKill(ch, victim))
    return;

  missiles = (GET_LEVEL(ch, MAGE_LEVEL_IND) / 2) + 1;
  missiles = MIN(5, missiles);
  dam = dice(missiles, 4) + missiles;

  if (affected_by_spell(victim, SPELL_SHIELD))
    dam = 0;

  damage(ch, victim, dam, SPELL_MAGIC_MISSILE);
}

void spell_shocking_grasp(char level, struct char_data *ch, struct char_data *victim,
			  struct obj_data *obj)
{
  int                                     dam = 0;
  int                                     classes = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim || level < 0 || level > ABS_MAX_LVL)
    return;
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

void spell_lightning_bolt(char level, struct char_data *ch, struct char_data *victim,
			  struct obj_data *obj)
{
  int                                     dam = 0;
  int                                     high_die = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim || level < 0)
    return;
  if (!CheckKill(ch, victim))
    return;

  high_die = ((level / 10) + (level / 3) + 1);
  dam = dice(high_die, 6);

  if (saves_spell(victim, SAVING_SPELL))
    dam /= 2;

  damage(ch, victim, dam, SPELL_LIGHTNING_BOLT);
}

/* Drain XP, MANA, HP - caster gains HP and MANA */
void spell_energy_drain(char level, struct char_data *ch, struct char_data *victim,
			struct obj_data *obj)
{
  int                                     dam = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim || level < 0)
    return;

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
      damage(ch, victim, 100, SPELL_ENERGY_DRAIN);	       /* Kill the sucker */
    } else if (IS_PC(victim) && IS_IMMORTAL(victim) && !IS_IMMORTAL(ch)) {
      cprintf(victim, "Some puny mortal just tried to drain you...\n\r");
    } else {
      if (IS_NOT_SET(victim->M_immune, IMM_DRAIN)) {
	cprintf(victim, "Your life energy is drained!\n\r");
	dam = 1;
	if (IS_PC(victim)) {
	  if (GetMaxLevel(ch) < GetMaxLevel(victim))
	    gain_exp(ch, GET_EXP(victim) / (dice(1, 4) * 1000));
	  ch->points.hit += dice(1, 4);
	  damage(ch, victim, dam, SPELL_ENERGY_DRAIN);
	  drop_level(victim, BestClass(victim));
	  set_title(victim);
	} else {
	  dam = dice(MIN((level / 5), 9), 4);
	  damage(ch, victim, dam, SPELL_ENERGY_DRAIN);
	}
      } else {
	if (IS_NOT_SET(ch->M_immune, IMM_DRAIN)) {
	  cprintf(ch, "Your spell backfires!\n\r");
	  if (IS_PC(ch) && !IS_IMMORTAL(ch)) {
	    drop_level(ch, BestClass(ch));
	    set_title(ch);
	  } else {
	    dam = dice(MIN((level / 5), 9), 8);		       /* nasty spell */
	    damage(ch, victim, dam, SPELL_ENERGY_DRAIN);
	  }
	} else {
	  cprintf(ch, "Your spell fails utterly.\n\r");
	}
      }
    }
  } else {
    damage(ch, victim, 0, SPELL_ENERGY_DRAIN);		       /* Miss */
  }
}

void spell_fireball(char level, struct char_data *ch, struct char_data *victim,
		    struct obj_data *obj)
{
  int                                     dam = 0;
  int                                     high_die = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || level < 0 || level > ABS_MAX_LVL)
    return;
  high_die = (level / 3) + 2;
  dam = dice(high_die, 6);

  AreaEffectSpell(ch, dam, SPELL_FIREBALL, 1, "You feel a blast of hot air.\n\r");
}

void spell_new_earthquake(char level, struct char_data *ch, struct char_data *victim,
			  struct obj_data *obj)
{
  int                                     i = 0;
  int                                     localdamage = 0;
  int                                     neardamage = 0;
  int                                     found = FALSE;
  struct room_data                       *here = NULL;
  struct room_data                       *tmp = NULL;
  struct room_direction_data             *exitdata = NULL;
  struct char_data                       *tvictim = NULL;
  struct char_data                       *next_victim = NULL;
  struct room_data                       *rp[MAX_NUM_EXITS];

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || level < 0 || level > ABS_MAX_LVL)
    return;
  localdamage = dice(level, 4);
  neardamage = dice(level / 4, 4);
  if (!ch->in_room)
    return;
  if (!(here = real_roomp(ch->in_room)))
    return;
  for (i = 0; i < MAX_NUM_EXITS; i++)
    if ((exitdata = EXIT(ch, i)))
      rp[i] = real_roomp(exitdata->to_room);
  for (tvictim = character_list; tvictim; tvictim = next_victim) {
    next_victim = tvictim->next;
    if (!tvictim || !tvictim->in_room)
      continue;
    if (tvictim == ch)
      continue;
    if (IS_IMMORTAL(tvictim) && !IS_IMMORTAL(ch)) {
      cprintf(tvictim, "The ground shakes a bit as the mortals play.\n\r");
      continue;
    }
    if (!CheckKill(ch, tvictim))
      continue;
    if (IS_AFFECTED(tvictim, AFF_GROUP))
      if ((tvictim == ch->master) || (tvictim->master == ch) || (tvictim->master == ch->master))
	continue;
    if (tvictim->in_room == ch->in_room) {		       /* full SUFFERAGE! */
      damage(ch, tvictim, localdamage, SPELL_EARTHQUAKE);
      act("You fall and are pummled with debris!\n\r", FALSE, ch, 0, tvictim, TO_VICT);
      if (GET_POS(tvictim) > POSITION_SITTING)
	GET_POS(tvictim) = POSITION_SITTING;
      if (GET_POS(tvictim) > POSITION_DEAD) {
	WAIT_STATE(tvictim, (PULSE_VIOLENCE * 3));
	if (IS_MOB(tvictim))
	  AddHated(tvictim, ch);
      }
    } else {
      if (!(tmp = real_roomp(tvictim->in_room)))
	continue;
      found = FALSE;
      for (i = 0; i < MAX_NUM_EXITS; i++) {
	if (rp[i] && tmp->number == rp[i]->number) {	       /* adjacent rooms SUFFER! */
	  found = TRUE;
	  damage(tvictim, tvictim, neardamage, SPELL_EARTHQUAKE);
	  act("The ground shakes nearby and you lose your footing!\n\r",
	      FALSE, tvictim, 0, tvictim, TO_VICT);
	  if (GET_POS(tvictim) > POSITION_SITTING)
	    GET_POS(tvictim) = POSITION_SITTING;
	  if (GET_POS(tvictim) > POSITION_DEAD) {
	    WAIT_STATE(tvictim, (PULSE_VIOLENCE));
	    if (IS_MOB(tvictim))
	      AddHated(tvictim, ch);
	  }
	  break;
	}
      }
      if (!found) {
	if (here->zone == tmp->zone) {			       /* Well, tell them it moved for them */
	  act("The ground quivers all around you...\n\r", FALSE, tvictim, 0, tvictim, TO_VICT);
	}
      }
    }
  }
}

void spell_earthquake(char level, struct char_data *ch, struct char_data *victim,
		      struct obj_data *obj)
{
  int                                     dam = 0;
  char                                    buf[128] = "\0\0\0";

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || level < 0 || level > ABS_MAX_LVL)
    return;
  dam = dice(1, 8) + level;

  cprintf(ch, "The ground trembles beneath your feet!\n\r");
  act("$n makes the ground tremble and shiver\n\r", FALSE, ch, 0, 0, TO_ROOM);
  sprintf(buf, "The ground trembless beneath your feet....\n\r");
  AreaEffectSpell(ch, dam, SPELL_EARTHQUAKE, 1, buf);
}

void spell_dispel_evil(char level, struct char_data *ch, struct char_data *victim,
		       struct obj_data *obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim || level < 0 || level > ABS_MAX_LVL)
    return;
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

void spell_harm(char level, struct char_data *ch, struct char_data *victim,
		struct obj_data *obj)
{
  int                                     dam = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim || level < 0 || level > ABS_MAX_LVL)
    return;

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

/* spells2.c - Not directly offensive spells */

void spell_shelter(char level, struct char_data *ch, struct char_data *victim,
		   struct obj_data *obj)
{
  struct obj_cost                         cost;
  int                                     i = 0;
  int                                     save_room = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || level < 0 || level > ABS_MAX_LVL)
    return;
  if (IS_NPC(ch))
    return;

  cost.total_cost = 0;					       /* Minimum cost */
  cost.no_carried = 0;
  cost.ok = TRUE;					       /* Use if any "-1" objects */

  add_obj_cost(ch, 0, ch->carrying, &cost);

  for (i = 0; i < MAX_WEAR; i++)
    add_obj_cost(ch, 0, ch->equipment[i], &cost);

  if (!cost.ok)
    return;

  obj_from_char(obj);
  extract_obj(obj);
  cost.total_cost = 0;

  GET_HOME(ch) = ch->in_room;
  new_save_equipment(ch, &cost, FALSE);
  save_obj(ch, &cost, 1);
  save_room = ch->in_room;
  cprintf(ch, "You open a small rip in the fabric of space!\n\r");
  cprintf(ch, "As you step through you leave the world of WileyMUD III behind....\n\r");
  act("$n opens a small rip in the fabric of space and steps through!", FALSE, ch, 0, 0,
      TO_ROOM);
  extract_char(ch);
  ch->in_room = save_room;
  save_char(ch, ch->in_room);
  return;
}

void spell_astral_walk(char level, struct char_data *ch, struct char_data *victim,
		       struct obj_data *obj)
{
  int                                     location = 0;
  struct room_data                       *rp = NULL;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim || level < 0 || level > ABS_MAX_LVL)
    return;
  location = victim->in_room;
  rp = real_roomp(location);

  if (!rp || IS_SET(rp->room_flags, PRIVATE) ||
      IS_SET(rp->room_flags, NO_SUM) || IS_SET(rp->room_flags, NO_MAGIC)) {
    cprintf(ch, "You failed.\n\r");
    return;
  }
  if (dice(1, 8) == 8) {
    cprintf(ch, "You failed.\n\r");
    return;
  } else {
    act("$n opens a door to another dimension and steps through!", FALSE, ch, 0, 0, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, location);
    do_look(ch, "", 15);
    act("You are blinded for a moment as $n appears in a flash of light!", FALSE, ch, 0, 0,
	TO_ROOM);
    do_look(ch, "", 15);
  }
}

void spell_visions(char level, struct char_data *ch, struct char_data *victim,
		   struct obj_data *obj)
{
  int                                     location = 0;
  int                                     orig_loc = 0;
  struct room_data                       *rp = NULL;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim || level < 0 || level > ABS_MAX_LVL)
    return;
  location = victim->in_room;

  rp = real_roomp(location);

  if (!rp || IS_SET(rp->room_flags, PRIVATE) || IS_SET(rp->room_flags, NO_MAGIC)) {
    cprintf(ch, "A magical boundary obstructs your vision.\n\r");
    return;
  }
  if (dice(1, 8) == 8) {
    cprintf(ch, "You could not receive a vision.\n\r");
    return;
  } else {
    orig_loc = ch->in_room;
    char_from_room(ch);
    char_to_room(ch, location);
    cprintf(ch, "You see in your mind's eye...\n\r");
    do_look(ch, "", 15);
    char_from_room(ch);
    char_to_room(ch, orig_loc);
  }
}

void spell_cure_critic(char level, struct char_data *ch, struct char_data *victim,
		       struct obj_data *obj)
{
  int                                     healpoints = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!victim || level < 0 || level > ABS_MAX_LVL)
    return;
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

  cprintf(victim, "You feel better!\n\r");
  update_pos(victim);
  if (ch)
    cprintf(ch, "%s %s.\n\r", NAME(victim), pain_level(victim));
}

void spell_cure_light(char level, struct char_data *ch, struct char_data *victim,
		      struct obj_data *obj)
{
  int                                     healpoints = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!victim || level < 0 || level > ABS_MAX_LVL)
    return;
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

  cprintf(victim, "You feel better!\n\r");
  update_pos(victim);
  if (ch)
    cprintf(ch, "%s %s.\n\r", NAME(victim), pain_level(victim));
}

void spell_curse(char level, struct char_data *ch, struct char_data *victim,
		 struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if ((!victim && !obj) || level < 0 || level > ABS_MAX_LVL)
    return;
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
    af.modifier = 1;					       /* Make worse */
    affect_to_char(victim, &af);

    act("$n briefly reveals a red aura!", FALSE, victim, 0, 0, TO_ROOM);
    if (IS_NPC(victim) && !victim->specials.fighting) {
      set_fighting(victim, ch);
      return;
    }
    act("You feel very uncomfortable.", FALSE, victim, 0, 0, TO_CHAR);
  }
}

void spell_detect_evil(char level, struct char_data *ch, struct char_data *victim,
		       struct obj_data *obj)
{
  struct affected_type                    af;
  int                                     dur = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!victim || level < 0 || level > ABS_MAX_LVL)
    return;
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
  cprintf(victim, "Your eyes tingle.\n\r");
}

void spell_detect_invisibility(char level, struct char_data *ch, struct char_data *victim,
			       struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!victim || level < 0 || level > ABS_MAX_LVL)
    return;
  if (affected_by_spell(victim, SPELL_DETECT_INVISIBLE))
    return;

  af.type = SPELL_DETECT_INVISIBLE;
  af.duration = 5 + level;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_DETECT_INVISIBLE;

  affect_to_char(victim, &af);
  act("$n's eyes briefly glow yellow", FALSE, victim, 0, 0, TO_ROOM);
  cprintf(victim, "Your eyes tingle.\n\r");
}

void spell_detect_magic(char level, struct char_data *ch, struct char_data *victim,
			struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!victim || level < 0 || level > ABS_MAX_LVL)
    return;
  if (affected_by_spell(victim, SPELL_DETECT_MAGIC))
    return;

  af.type = SPELL_DETECT_MAGIC;
  af.duration = 2 + level;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_DETECT_MAGIC;

  affect_to_char(victim, &af);
  cprintf(victim, "Your eyes tingle.\n\r");
}

void spell_detect_poison(char level, struct char_data *ch, struct char_data *victim,
			 struct obj_data *obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || (!victim && !obj) || level < 0 || level > ABS_MAX_LVL)
    return;
  if (victim) {
    if (victim == ch)
      if (IS_AFFECTED(victim, AFF_POISON))
	cprintf(ch, "You can sense poison in your blood.\n\r");
      else
	cprintf(ch, "You feel healthy.\n\r");
    else if (IS_AFFECTED(victim, AFF_POISON)) {
      act("You sense that $E is poisoned.", FALSE, ch, 0, victim, TO_CHAR);
    } else {
      act("You sense that $E is poisoned", FALSE, ch, 0, victim, TO_CHAR);
    }
  } else {						       /* It's an object */
    if ((obj->obj_flags.type_flag == ITEM_DRINKCON) || (obj->obj_flags.type_flag == ITEM_FOOD)) {
      if (obj->obj_flags.value[3])
	act("Poisonous fumes are revealed.", FALSE, ch, 0, 0, TO_CHAR);
      else
	cprintf(ch, "It looks very delicious.\n\r");
    }
  }
}

void spell_enchant_weapon(char level, struct char_data *ch, struct char_data *victim,
			  struct obj_data *obj)
{
  int                                     i = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !obj)
    return;
  if ((GET_ITEM_TYPE(obj) == ITEM_WEAPON) && IS_NOT_SET(obj->obj_flags.extra_flags, ITEM_MAGIC)) {
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

void spell_heal(char level, struct char_data *ch, struct char_data *victim,
		struct obj_data *obj)
{
  int                                     dam = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!victim)
    return;

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
  cprintf(victim, "A deity has smiled down on you....\n\r");
}

void spell_invisibility(char level, struct char_data *ch, struct char_data *victim,
			struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!victim || (!ch && !obj))
    return;

  if (obj) {
    if (!IS_SET(obj->obj_flags.extra_flags, ITEM_INVISIBLE)) {
      act("$p turns invisible.", FALSE, ch, obj, 0, TO_CHAR);
      act("$p turns invisible.", TRUE, ch, obj, 0, TO_ROOM);
      SET_BIT(obj->obj_flags.extra_flags, ITEM_INVISIBLE);
    }
  } else {						       /* Then it is a PC | NPC */
    if (!affected_by_spell(victim, SPELL_INVISIBLE)) {
      act("$n slowly fades out of existence.", TRUE, victim, 0, 0, TO_ROOM);
      cprintf(victim, "You vanish.\n\r");
      af.type = SPELL_INVISIBLE;
      af.duration = 1 + (level / 2);
      af.modifier = -40;
      af.location = APPLY_AC;
      af.bitvector = AFF_INVISIBLE;
      affect_to_char(victim, &af);
    }
  }
}

void spell_locate_object(char level, struct char_data *ch, struct char_data *victim, char *obj)
{
  struct obj_data                        *i = NULL;
  char                                    name[256] = "\0\0\0";
  int                                     j = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), VNULL(obj));

  if (!ch || !obj)
    return;

  strcpy(name, obj);
  j = level >> 1;

  for (i = object_list; i && (j > 0); i = i->next)
    if (isname(name, i->name)) {
      if (i->carried_by) {
	if (strlen(PERS(i->carried_by, ch)) > 0) {
	  cprintf(ch, "%s carried by %s.\n\r", i->short_description, PERS(i->carried_by, ch));
	}
      } else if (i->equipped_by) {
	if (strlen(PERS(i->equipped_by, ch)) > 0) {
	  cprintf(ch, "%s equipped by %s.\n\r", i->short_description, PERS(i->equipped_by, ch));
	}
      } else if (i->in_obj) {
	cprintf(ch, "%s in %s.\n\r", i->short_description, i->in_obj->short_description);
      } else {
	cprintf(ch, "%s in %s.\n\r", i->short_description,
		(i->in_room == NOWHERE ? "use but uncertain." : real_roomp(i->in_room)->name));
	j--;
      }
    }
  if (j == 0)
    cprintf(ch, "You are very confused.\n\r");
  if (j == level >> 1)
    cprintf(ch, "No such object.\n\r");
}

void spell_poison(char level, struct char_data *ch, struct char_data *victim,
		  struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!victim && !obj)
    return;

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
	cprintf(victim, "You feel very sick.\n\r");
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
  } else {						       /* Object poison */
    if ((obj->obj_flags.type_flag == ITEM_DRINKCON) || (obj->obj_flags.type_flag == ITEM_FOOD)) {
      obj->obj_flags.value[3] = 1;
    }
  }
}

void spell_protection_from_evil(char level, struct char_data *ch, struct char_data *victim,
				struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!victim)
    return;

  if (!affected_by_spell(victim, SPELL_PROTECT_FROM_EVIL)) {
    af.type = SPELL_PROTECT_FROM_EVIL;
    af.duration = 1 + (level / 2);
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_PROTECT_EVIL;
    affect_to_char(victim, &af);
    cprintf(victim, "You have a righteous feeling!\n\r");
  }
}

void spell_remove_curse(char level, struct char_data *ch, struct char_data *victim,
			struct obj_data *obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!(ch && (victim || obj)))
    return;

  if (DEBUG)
    log_info("spell_remove_curse");
  if (obj) {
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP)) {
      act("$p briefly glows blue.", TRUE, ch, obj, 0, TO_CHAR);
      REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);
    }
  } else {						       /* Then it is a PC | NPC */
    if (affected_by_spell(victim, SPELL_CURSE)) {
      act("$n briefly glows red, then blue.", FALSE, victim, 0, 0, TO_ROOM);
      act("You feel better.", FALSE, victim, 0, 0, TO_CHAR);
      affect_from_char(victim, SPELL_CURSE);
    }
  }
}

void spell_remove_poison(char level, struct char_data *ch, struct char_data *victim,
			 struct obj_data *obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if(!(ch && (victim || obj)))
    return;

  if (victim) {
    if (affected_by_spell(victim, SPELL_POISON)) {
      affect_from_char(victim, SPELL_POISON);
      act("A warm feeling runs through your body.", FALSE, victim, 0, 0, TO_CHAR);
      act("$N looks better.", FALSE, ch, 0, victim, TO_ROOM);
    }
  } else {
    if ((obj->obj_flags.type_flag == ITEM_DRINKCON) || (obj->obj_flags.type_flag == ITEM_FOOD)) {
      obj->obj_flags.value[3] = 0;
      act("The $p steams briefly.", FALSE, ch, obj, 0, TO_CHAR);
    }
  }
}

void spell_fireshield(char level, struct char_data *ch, struct char_data *victim,
		      struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

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

void spell_sanctuary(char level, struct char_data *ch, struct char_data *victim,
		     struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

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

void spell_sleep(char level, struct char_data *ch, struct char_data *victim,
		 struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!victim)
    return;

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

void spell_strength(char level, struct char_data *ch, struct char_data *victim,
		    struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!victim)
    return;

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
	       HasClass(victim, CLASS_THIEF) || HasClass(victim, CLASS_DRUID))
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

void spell_ventriloquate(char level, struct char_data *ch, struct char_data *victim,
			 struct obj_data *obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  /*
   * Not possible!! No argument! 
   */
}

void spell_word_of_recall(char level, struct char_data *ch, struct char_data *victim,
			  struct obj_data *obj)
{
  int                                     location = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!victim)
    return;
  if (IS_NPC(victim))
    return;

  location = GET_HOME(ch); /* Shouldn't this be victim? */

  if (!real_roomp(location)) {
    cprintf(victim, "You are completely lost.\n\r");
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

void spell_summon(char level, struct char_data *ch, struct char_data *victim,
		  struct obj_data *obj)
{
  short int                               target = 0;
  struct char_data                       *tmp = NULL;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!(ch && victim))
    return;

  if (IS_NPC(victim) && saves_spell(victim, SAVING_SPELL)) {
    cprintf(ch, "You failed.\n\r");
    return;
  }
  if (IS_SET(victim->specials.new_act, NEW_PLR_SUMMON)) {
    cprintf(ch, "Sorry, that does not want to be summoned.\n\r");
    return;
  }
  if (IS_SET(real_roomp(victim->in_room)->room_flags, NO_SUM)) {
    cprintf(ch, "You cannot penetrate the magical defenses of that area.\n\r");
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

void spell_charm_monster(char level, struct char_data *ch, struct char_data *victim,
			 struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!(ch && victim))
    return;

  if (victim == ch) {
    cprintf(ch, "You like yourself even better!\n\r");
    return;
  }
  if (!CheckKill(ch, victim))
    return;

  if (!IS_AFFECTED(victim, AFF_CHARM) && !IS_AFFECTED(ch, AFF_CHARM)) {
    if (circle_follow(victim, ch)) {
      cprintf(ch, "Sorry, following in circles can not be allowed.\n\r");
      return;
    }
    if (IsPerson(victim)) {
      cprintf(ch, "How insulting!  That is a person, not a beast!\n\r");
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

    cprintf(ch, "They are now charmed.....\n\r");
    act("Isn't $n just such a nice fellow?", FALSE, ch, 0, victim, TO_VICT);
  }
}

void spell_sense_life(char level, struct char_data *ch, struct char_data *victim,
		      struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!victim)
    return;

  if (!affected_by_spell(victim, SPELL_SENSE_LIFE)) {
    cprintf(ch, "Your feel your awareness improve.\n\r");

    af.type = SPELL_SENSE_LIFE;
    af.duration = 1 + (level / 2);
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_SENSE_LIFE;
    affect_to_char(victim, &af);
  }
}

/* ***************************************************************************
 *                     Not cast-able spells                                  *
 * ************************************************************************* */

void spell_identify(char level, struct char_data *ch, struct char_data *victim,
		    struct obj_data *obj)
{
  char                                    buf[256] = "\0\0\0";
  char                                    buf2[256] = "\0\0\0";
  int                                     i = 0;
  int                                     found = FALSE;
  int                                     val_index = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!(ch && (obj || victim)))
    return;

  if (obj) {
    cprintf(ch, "You feel informed:\n\r");
    sprintf(buf, "Object '%s', Item type: ", obj->name);
    sprinttype(GET_ITEM_TYPE(obj), item_types, buf2);
    strcat(buf, buf2);
    strcat(buf, "\n\r");
    cprintf(ch, "%s", buf);

    if (obj->obj_flags.bitvector) {
      cprintf(ch, "Item aids in abilities:  ");
      sprintbit(obj->obj_flags.bitvector, affected_bits, buf);
      strcat(buf, "\n\r");
      cprintf(ch, "%s", buf);
    }
    cprintf(ch, "Item is: ");
    sprintbit(obj->obj_flags.extra_flags, extra_bits, buf);
    strcat(buf, "\n\r");
    cprintf(ch, "%s", buf);

    cprintf(ch, "Weight: %d, Value: %d\n\r", obj->obj_flags.weight, obj->obj_flags.cost);

    switch (GET_ITEM_TYPE(obj)) {
      case ITEM_SCROLL:
      case ITEM_POTION:
	cprintf(ch, "Level %d spells of:\n\r", obj->obj_flags.value[0]);
	for (val_index = 1; val_index < 4; val_index++)
	  if (obj->obj_flags.value[val_index] > 0
	      && obj->obj_flags.value[val_index] < MAX_SKILLS) {
	    strcat(buf, spell_info[obj->obj_flags.value[val_index]].name);
	    strcat(buf, "\n\r");
	    cprintf(ch, "%s", buf);
	  }
	break;

      case ITEM_WAND:
      case ITEM_STAFF:
	cprintf(ch, "Has %d chages, with %d charges left.\n\r",
		obj->obj_flags.value[1], obj->obj_flags.value[2]);

	cprintf(ch, "Level %d spell of:\n\r", obj->obj_flags.value[0]);

	if (obj->obj_flags.value[3] > 0 && obj->obj_flags.value[3] < MAX_SKILLS) {
	  strcat(buf, spell_info[obj->obj_flags.value[3]].name);
	  strcat(buf, "\n\r");
	  cprintf(ch, "%s", buf);
	}
	break;

      case ITEM_WEAPON:
	cprintf(ch, "Damage Dice is '%dD%d'\n\r",
		obj->obj_flags.value[1], obj->obj_flags.value[2]);
	break;

      case ITEM_ARMOR:
	cprintf(ch, "AC-apply is %d\n\r", obj->obj_flags.value[0]);
	break;
    }

    found = FALSE;

    for (i = 0; i < MAX_OBJ_AFFECT; i++) {
      if ((obj->affected[i].location != APPLY_NONE) && (obj->affected[i].modifier != 0)) {
	if (!found) {
	  cprintf(ch, "Can affect you as :\n\r");
	  found = TRUE;
	}
	sprinttype(obj->affected[i].location, apply_types, buf2);
	cprintf(ch, "    Affects : %s By %d\n\r", buf2, obj->affected[i].modifier);
      }
    }
  }
}

/* ***************************************************************************
 *                     NPC spells..                                          *
 * ************************************************************************* */

void spell_fire_breath(char level, struct char_data *ch, struct char_data *victim,
		       struct obj_data *obj)
{
  int                                     dam = 0;
  int                                     hpch = 0;
  struct obj_data                        *burn = NULL;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!(victim && ch))
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

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

  for (burn = victim->carrying;
       burn && (burn->obj_flags.type_flag != ITEM_SCROLL) &&
       (burn->obj_flags.type_flag != ITEM_WAND) &&
       (burn->obj_flags.type_flag != ITEM_STAFF) &&
       (burn->obj_flags.type_flag != ITEM_BOAT); burn = burn->next_content) {
    if (!saves_spell(victim, SAVING_BREATH)) {
      if (burn) {
	act("$o burns", 0, victim, burn, 0, TO_CHAR);
	extract_obj(burn);
      }
    }
  }
}

void spell_frost_breath(char level, struct char_data *ch, struct char_data *victim,
			struct obj_data *obj)
{
  int                                     dam = 0;
  int                                     hpch = 0;
  struct obj_data                        *frozen = NULL;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!(victim && ch))
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

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
       (frozen->obj_flags.type_flag != ITEM_POTION); frozen = frozen->next_content) {
    if (!saves_spell(victim, SAVING_BREATH)) {
      if (frozen) {
	act("$o shatters.", 0, victim, frozen, 0, TO_CHAR);
	extract_obj(frozen);
      }
    }
  }
}

void spell_acid_breath(char level, struct char_data *ch, struct char_data *victim,
		       struct obj_data *obj)
{
  int                                     dam = 0;
  int                                     hpch = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!(victim && ch))
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

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

void spell_gas_breath(char level, struct char_data *ch, struct char_data *victim,
		      struct obj_data *obj)
{
  int                                     dam = 0;
  int                                     hpch = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!(victim && ch))
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

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

void spell_lightning_breath(char level, struct char_data *ch, struct char_data *victim,
			    struct obj_data *obj)
{
  int                                     dam = 0;
  int                                     hpch = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!(victim && ch))
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

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

/*
 * file: magic2.c , Implementation of (new)spells.        Part of DIKUMUD
 * Usage : The actual effect of magic.
 */

/*
 * cleric spells
 */

/*
 **   requires the sacrifice of 150k coins, victim loses a con point, and
 **   caster is knocked down to 1 hp, 1 mp, 1 mana, and sits for a LONG
 **   time (if a pc)
 */

void spell_resurrection(char level, struct char_data *ch, struct char_data *victim,
			struct obj_data *obj)
{
  struct char_file_u                      st;
  struct affected_type                    af;
  struct obj_data                        *obj_object = NULL;
  struct obj_data                        *next_obj = NULL;
  FILE                                   *fl = NULL;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!obj)
    return;

  if (IS_CORPSE(obj)) {
    if (obj->char_vnum) {				       /* corpse is a npc */
      victim = read_mobile(obj->char_vnum, VIRTUAL);
      char_to_room(victim, ch->in_room);
      GET_GOLD(victim) = 0;
      GET_EXP(victim) = 0;
      GET_HIT(victim) = 1;
      GET_POS(victim) = POSITION_STUNNED;

      act("With mystic power, $n resurrects a corpse.", TRUE, ch, 0, 0, TO_ROOM);
      act("$N slowly rises from the ground.", FALSE, ch, 0, victim, TO_ROOM);

      /*
       * should be charmed and follower ch
       */

      if (IsImmune(victim, IMM_CHARM) || IsResist(victim, IMM_CHARM)) {
	act("$n says 'Thank you'", FALSE, ch, 0, victim, TO_ROOM);
      } else {
	af.type = SPELL_CHARM_PERSON;
	af.duration = 36;
	af.modifier = 0;
	af.location = 0;
	af.bitvector = AFF_CHARM;

	affect_to_char(victim, &af);

	add_follower(victim, ch);
      }

      IS_CARRYING_W(victim) = 0;
      IS_CARRYING_N(victim) = 0;

      /*
       * take all from corpse, and give to person
       */

      for (obj_object = obj->contains; obj_object; obj_object = next_obj) {
	next_obj = obj_object->next_content;
	obj_from_obj(obj_object);
	obj_to_char(obj_object, victim);
      }

      /*
       * get rid of corpse
       */
      extract_obj(obj);

    } else {						       /* corpse is a pc */

      if (GET_GOLD(ch) < 100000) {
	cprintf(ch, "The gods are not happy with your sacrifice.\n\r");
	return;
      } else {
	GET_GOLD(ch) -= 100000;
      }

      fl = fopen(PLAYER_FILE, "r+");
      if (!fl) {
	log_fatal("player file");
	proper_exit(MUD_HALT);
      }
      fseek(fl, obj->char_f_pos * sizeof(struct char_file_u), 0);
      fread(&st, sizeof(struct char_file_u), 1, fl);

      /*
       **   this is a serious kludge, and must be changed before multiple
       **   languages can be implemented
       */
      if (!st.talks[2] && st.abilities.con > 3) {
	st.points.exp *= 2;
	st.talks[2] = TRUE;
	st.abilities.con -= 1;
	act("A clear bell rings throughout the heavens", TRUE, ch, 0, 0, TO_CHAR);
	act("A ghostly spirit smiles, and says 'Thank you'", TRUE, ch, 0, 0, TO_CHAR);
	act("A clear bell rings throughout the heavens", TRUE, ch, 0, 0, TO_ROOM);
	act("A ghostly spirit smiles, and says 'Thank you'", TRUE, ch, 0, 0, TO_ROOM);
	act("$p dissappears in the blink of an eye.", TRUE, ch, obj, 0, TO_ROOM);
	act("$p dissappears in the blink of an eye.", TRUE, ch, obj, 0, TO_ROOM);
	GET_MANA(ch) = 1;
	GET_MOVE(ch) = 1;
	GET_HIT(ch) = 1;
	GET_POS(ch) = POSITION_STUNNED;
	act("$n collapses from the effort!", TRUE, ch, 0, 0, TO_ROOM);
	cprintf(ch, "You collapse from the effort\n\r");
	fseek(fl, obj->char_f_pos * sizeof(struct char_file_u), 0);
	fwrite(&st, sizeof(struct char_file_u), 1, fl);

	ObjFromCorpse(obj);

      } else {
	cprintf(ch, "The body does not have the strength to be recreated.\n\r");
      }
      FCLOSE(fl);
    }
  }
}

void spell_cause_light(char level, struct char_data *ch, struct char_data *victim,
		       struct obj_data *obj)
{
  int                                     dam = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!(victim && ch))
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (!CheckKill(ch, victim))
    return;

  if (HowManyClasses(ch) == 1)
    dam = dice(2, 4) + 1;
  else
    dam = dice(1, 8);

  if (IS_REALLY_HOLY(ch))
    dam -= dice(1, 3);
  else if (IS_HOLY(ch))
    dam -= dice(1, 2);
  else if (IS_VILE(ch))
    dam += dice(1, 2);
  else if (IS_REALLY_VILE(ch))
    dam += dice(1, 3);

  if (dam < 1)
    dam = 1;

  damage(ch, victim, dam, SPELL_CAUSE_LIGHT);
}

void spell_cause_critical(char level, struct char_data *ch, struct char_data *victim,
			  struct obj_data *obj)
{
  int                                     dam = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!(victim && ch))
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (!CheckKill(ch, victim))
    return;
  if (HowManyClasses(ch) == 1)
    dam = dice(4, 6) + 3;
  else
    dam = dice(3, 8);

  if (IS_REALLY_HOLY(ch))
    dam -= dice(1, 6);
  else if (IS_HOLY(ch))
    dam -= dice(1, 3);
  else if (IS_VILE(ch))
    dam += dice(1, 3);
  else if (IS_REALLY_VILE(ch))
    dam += dice(1, 6);

  if (dam < 1)
    dam = 1;

  damage(ch, victim, dam, SPELL_CAUSE_CRITICAL);
}

void spell_cause_serious(char level, struct char_data *ch, struct char_data *victim,
			 struct obj_data *obj)
{
  int                                     dam = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!(victim && ch))
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (!CheckKill(ch, victim))
    return;
  if (HowManyClasses(ch) == 1)
    dam = dice(4, 4) + 2;
  else
    dam = dice(2, 8);

  if (IS_REALLY_HOLY(ch))
    dam -= dice(1, 4);
  else if (IS_HOLY(ch))
    dam -= dice(1, 2);
  else if (IS_VILE(ch))
    dam += dice(1, 2);
  else if (IS_REALLY_VILE(ch))
    dam += dice(1, 4);

  if (dam < 1)
    dam = 1;

  damage(ch, victim, dam, SPELL_CAUSE_SERIOUS);

}

void spell_cure_serious(char level, struct char_data *ch, struct char_data *victim,
			struct obj_data *obj)
{
  int                                     dam = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!(victim && ch))
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (HowManyClasses(ch) == 1)
    dam = dice(4, 4) + 2;
  else
    dam = dice(2, 8);

  if (IS_REALLY_HOLY(ch))
    dam += dice(1, 4);
  else if (IS_HOLY(ch))
    dam += dice(1, 2);
  else if (IS_VILE(ch))
    dam -= dice(1, 2);
  else if (IS_REALLY_VILE(ch))
    dam -= dice(1, 4);

  if (dam < 1)
    dam = 1;

  if ((dam + GET_HIT(victim)) > hit_limit(victim))
    GET_HIT(victim) = hit_limit(victim);
  else
    GET_HIT(victim) += dam;

  cprintf(victim, "You feel better!\n\r");
  update_pos(victim);
  if (ch)
    cprintf(ch, "%s %s.\n\r", NAME(victim), pain_level(victim));
}

void spell_mana(char level, struct char_data *ch, struct char_data *victim,
		struct obj_data *obj)
{
  int                                     dam = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  dam = dice(level, 4);
  dam = MAX(dam, level * 2);

  if (GET_MANA(ch) + dam > GET_MAX_MANA(ch))
    GET_MANA(ch) = GET_MAX_MANA(ch);
  else
    GET_MANA(ch) += dam;

}

void spell_second_wind(char level, struct char_data *ch, struct char_data *victim,
		       struct obj_data *obj)
{
  int                                     dam = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!(victim && ch))
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  dam = dice(level, 8) + level;

  if ((dam + GET_MOVE(victim)) > move_limit(victim))
    GET_MOVE(victim) = move_limit(victim);
  else
    GET_MOVE(victim) += dam;

  cprintf(victim, "You feel less tired\n\r");

}

void spell_flamestrike(char level, struct char_data *ch, struct char_data *victim,
		       struct obj_data *obj)
{
  int                                     dam = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!(victim && ch))
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (!CheckKill(ch, victim))
    return;
  dam = dice((8 - HowManyClasses(ch)), 8);

  if (saves_spell(victim, SAVING_SPELL))
    dam >>= 1;

  cprintf(ch, "You summon forth a pillar of flame....\n\r");
  damage(ch, victim, dam, SPELL_FLAMESTRIKE);
}

void spell_dispel_good(char level, struct char_data *ch, struct char_data *victim,
		       struct obj_data *obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!(victim && ch))
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (!CheckKill(ch, victim))
    return;

  if (IsExtraPlanar(victim)) {
    if (IS_GOOD(ch)) {
      victim = ch;
    } else if (IS_EVIL(victim)) {
      act("Evil protects $N.", FALSE, ch, 0, victim, TO_CHAR);
      return;
    }
    if (!saves_spell(victim, SAVING_SPELL)) {
      act("$n forces $N from this plane.", TRUE, ch, 0, victim, TO_NOTVICT);
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

void spell_turn(char level, struct char_data *ch, struct char_data *victim,
		struct obj_data *obj)
{
  int                                     diff = 0;
  int                                     i = 0;
  int                                     flag = 0;
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!(victim && ch))
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (IsUndead(victim)) {
    diff = level - GetTotLevel(victim);
    if (diff <= 0) {
      act("You are powerless to affect $N", TRUE, ch, 0, victim, TO_CHAR);
      return;
    } else {
      for (i = 1; i <= diff; i++) {
	if (!saves_spell(victim, SAVING_SPELL)) {
	  if (!saves_spell(victim, SAVING_SPELL)) {
	    if (IS_GOOD(ch)) {
	      /*
	       * caster is good....kill the mob/player 
	       */
	      act("$N falls to the ground and convulses uncontrollably.",
		  TRUE, ch, 0, victim, TO_NOTVICT);
	      act("You fall to the ground, mortally wounded!", TRUE, ch, 0, victim, TO_VICT);
	      act("$N falls to the ground as you devour $E life force!",
		  TRUE, ch, 0, victim, TO_CHAR);
	      damage(ch, victim, (GET_HIT(victim) + 1), SPELL_FLAMESTRIKE);
	      update_pos(victim);
	      return;
	    } else if (IS_VILE(ch)) {
	      if (!IsImmune(victim, IMM_CHARM)) {
		/*
		 * caster is evil...enslave the mob/player 
		 */
		act("$N shivers for a moment, then stares vacantly at $n.",
		    TRUE, ch, 0, victim, TO_NOTVICT);
		act("You shiver as $n invades your mind and possesses you.",
		    TRUE, ch, 0, victim, TO_VICT);
		act("$N shivers as you utter a prayer and take possession of $n's mind.",
		    TRUE, ch, 0, victim, TO_CHAR);
		if (victim->master)
		  stop_follower(victim);
		if (circle_follow(victim, ch)) {
		  affect_from_char(ch, SPELL_CHARM_PERSON);
		  stop_follower(ch);
		}
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
		REMOVE_BIT(victim->specials.act, ACT_AGGRESSIVE);
		return;
	      }
	    } else {
	      break;
	    }
	  } else {
	    act("$n forces $N from this room.", TRUE, ch, 0, victim, TO_NOTVICT);
	    act("You force $N from this room.", TRUE, ch, 0, victim, TO_CHAR);
	    act("$n forces you from this room.", TRUE, ch, 0, victim, TO_VICT);
	    do_flee(victim, "", 0);
	    flag = 1;
	    break;
	  }
	}
      }
      if (!flag) {
	act("You laugh at $n.", TRUE, ch, 0, victim, TO_VICT);
	act("$N laughs at $n.", TRUE, ch, 0, victim, TO_NOTVICT);
	act("$N laughs at you.", TRUE, ch, 0, victim, TO_CHAR);
      }
    }
  } else {
    act("$n just tried to turn you, what a moron!", TRUE, ch, 0, victim, TO_VICT);
    act("$N thinks $n is really strange.", TRUE, ch, 0, victim, TO_NOTVICT);
    act("Um... $N isn't undead...", TRUE, ch, 0, victim, TO_CHAR);
  }
}

void spell_remove_paralysis(char level, struct char_data *ch, struct char_data *victim,
			    struct obj_data *obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!(victim && ch))
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (affected_by_spell(victim, SPELL_PARALYSIS)) {
    affect_from_char(victim, SPELL_PARALYSIS);
    act("A warm feeling runs through your body.", FALSE, victim, 0, 0, TO_CHAR);
    act("$N looks better.", FALSE, ch, 0, victim, TO_ROOM);
  }
}

void spell_holy_word(char level, struct char_data *ch, struct char_data *victim,
		     struct obj_data *obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));
}

void spell_unholy_word(char level, struct char_data *ch, struct char_data *victim,
		       struct obj_data *obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));
}

void spell_succor(char level, struct char_data *ch, struct char_data *victim,
		  struct obj_data *obj)
{
  struct obj_data                        *o = NULL;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  o = read_object(1901, VIRTUAL);
  obj_to_char(o, ch);
  act("$n waves $s hand, and creates $p", TRUE, ch, o, 0, TO_ROOM);
  act("You wave your hand and create $p.", TRUE, ch, o, 0, TO_CHAR);
}

void spell_detect_charm(char level, struct char_data *ch, struct char_data *victim,
			struct obj_data *obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));
}

void spell_true_seeing(char level, struct char_data *ch, struct char_data *victim,
		       struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!(victim && ch))
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (!IS_AFFECTED(victim, AFF_TRUE_SIGHT)) {
    if (ch != victim) {
      cprintf(victim, "Your eyes glow silver for a moment.\n\r");
      act("$n's eyes take on a silvery hue.\n\r", FALSE, victim, 0, 0, TO_ROOM);
    } else {
      cprintf(ch, "Your eyes glow silver.\n\r");
      act("$n's eyes glow silver.\n\r", FALSE, ch, 0, 0, TO_ROOM);
    }

    af.type = SPELL_TRUE_SIGHT;
    af.duration = level / 2;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_TRUE_SIGHT;
    affect_to_char(victim, &af);
  } else {
    cprintf(ch, "Nothing seems to happen\n\r");
  }
}

/*
 * magic user spells
 */

void spell_poly_self(char level, struct char_data *ch, struct char_data *victim,
		     struct obj_data *obj)
{
  /* victim is a prototype mob */

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!(victim && ch))
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (!ch->desc || ch->desc->snoop.snoop_by || ch->desc->snoop.snooping) {
    cprintf(ch, "Godly interference prevents the spell from working.");
    return;
  }
  char_to_room(victim, ch->in_room);

  act("$n's flesh melts and flows into the shape of $N", TRUE, ch, 0, victim, TO_ROOM);
  act("Your flesh melts and flows into the shape of $N", TRUE, ch, 0, victim, TO_CHAR);

  char_from_room(ch);
  char_to_room(ch, 3);

  ch->desc->character = victim;
  ch->desc->original = ch;

  victim->desc = ch->desc;
  ch->desc = 0;

  SET_BIT(victim->specials.act, ACT_POLYSELF);
  GET_EXP(victim) = 10;
}

void spell_minor_create(char level, struct char_data *ch, struct char_data *victim,
			struct obj_data *obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!(obj && ch))
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  act("$n claps $s hands together.", TRUE, ch, 0, 0, TO_ROOM);
  act("You clap your hands together.", TRUE, ch, 0, 0, TO_CHAR);
  act("In a flash of light, $p appears.", TRUE, ch, obj, 0, TO_ROOM);
  act("In a flash of light, $p appears.", TRUE, ch, obj, 0, TO_CHAR);
  obj_to_room(obj, ch->in_room);

}

void spell_stone_skin(char level, struct char_data *ch, struct char_data *victim,
		      struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (!affected_by_spell(ch, SPELL_STONE_SKIN)) {
    act("$n's skin turns grey and granite-like.", TRUE, ch, 0, 0, TO_ROOM);
    act("Your skin turns to a stone-like substance.", TRUE, ch, 0, 0, TO_CHAR);

    af.type = SPELL_STONE_SKIN;
    af.duration = (level / 4 - (dice(HowManyClasses(ch), 3)));
    if (af.duration < 1)
      af.duration = 1;

    af.modifier = -30;
    af.location = APPLY_AC;
    af.bitvector = 0;
    affect_to_char(ch, &af);

    /*
     * resistance to piercing weapons 
     */

    af.type = SPELL_STONE_SKIN;
    af.duration = (level / 8 - (dice(HowManyClasses(ch), 3)));
    if (af.duration < 1)
      af.duration = 1;
    af.modifier = 32;
    af.location = APPLY_IMMUNE;
    af.bitvector = 0;
    affect_to_char(ch, &af);
  } else
    cprintf(ch, "The spell fizzles...\n\r");
}

void spell_infravision(char level, struct char_data *ch, struct char_data *victim,
		       struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!(victim && ch))
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (!IS_AFFECTED(victim, AFF_INFRAVISION)) {
    if (ch != victim) {
      cprintf(victim, "Your eyes glow red.\n\r");
      act("$n's eyes glow red.\n\r", FALSE, victim, 0, 0, TO_ROOM);
    } else {
      cprintf(ch, "Your eyes glow red.\n\r");
      act("$n's eyes glow red.\n\r", FALSE, ch, 0, 0, TO_ROOM);
    }

    af.type = SPELL_INFRAVISION;
    af.duration = level;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_INFRAVISION;
    affect_to_char(victim, &af);
  }
}

void spell_shield(char level, struct char_data *ch, struct char_data *victim,
		  struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!(victim && ch))
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (!affected_by_spell(victim, SPELL_SHIELD)) {
    act("$N is surrounded by a strong force shield.", TRUE, ch, 0, victim, TO_NOTVICT);
    if (ch != victim) {
      act("$N is surrounded by a strong force shield.", TRUE, ch, 0, victim, TO_CHAR);
      act("You are surrounded by a strong force shield.", TRUE, ch, 0, victim, TO_VICT);
    } else {
      act("You are surrounded by a strong force shield.", TRUE, ch, 0, victim, TO_VICT);
    }

    af.type = SPELL_SHIELD;
    af.duration = level;
    af.modifier = -10;
    af.location = APPLY_AC;
    af.bitvector = 0;
    affect_to_char(victim, &af);
  } else
    cprintf(ch, "The spell fizzles....\n\r");
}

void spell_weakness(char level, struct char_data *ch, struct char_data *victim,
		    struct obj_data *obj)
{
  struct affected_type                    af;
  float                                   modifier = 0.0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!(victim && ch))
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (!CheckKill(ch, victim))
    return;

  if (!affected_by_spell(victim, SPELL_WEAKNESS))
    if (!saves_spell(victim, SAVING_SPELL)) {
      modifier = (77.0 - level) / 100.0;
      act("You feel weaker.", FALSE, victim, 0, 0, TO_VICT);
      act("$n seems weaker.", FALSE, victim, 0, 0, TO_ROOM);

      af.type = SPELL_WEAKNESS;
      af.duration = (int)level / 2;
      af.modifier = (int)0 - (victim->abilities.str * modifier);
      if (victim->abilities.str_add)
	af.modifier -= 2;
      af.location = APPLY_STR;
      af.bitvector = 0;

      affect_to_char(victim, &af);
    }
}

void spell_invis_group(char level, struct char_data *ch, struct char_data *victim,
		       struct obj_data *obj)
{
  struct char_data                       *tmp_victim = NULL;
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  for (tmp_victim = real_roomp(ch->in_room)->people; tmp_victim;
       tmp_victim = tmp_victim->next_in_room) {
    if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim))
      if (in_group(ch, tmp_victim)) {
	if (!affected_by_spell(tmp_victim, SPELL_INVISIBLE)) {

	  act("$n slowly fades out of existence.", TRUE, tmp_victim, 0, 0, TO_ROOM);
	  cprintf(tmp_victim, "You vanish.\n\r");

	  af.type = SPELL_INVISIBLE;
	  af.duration = level / 2;
	  af.modifier = -40;
	  af.location = APPLY_AC;
	  af.bitvector = AFF_INVISIBLE;
	  affect_to_char(tmp_victim, &af);
	}
      }
  }
}

void spell_acid_blast(char level, struct char_data *ch, struct char_data *victim,
		      struct obj_data *obj)
{
  int                                     dam = 0;
  int                                     high_die = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!victim || !ch)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;
  if (!CheckKill(ch, victim))
    return;

  switch (HowManyClasses(ch)) {
    case 1:
      high_die = 5 + ((level / 7) - 1);
      break;
    case 2:
      high_die = 5 + ((level / 8) - 1);
      break;
    case 3:
      high_die = 5 + ((level / 9) - 1);
      break;
    case 4:
      high_die = 5 + ((level / 10) - 1);
      break;
    default:
      high_die = 5 + ((level / 11) - 1);
      break;
  }

  dam = dice(high_die, 6);

  if (saves_spell(victim, SAVING_SPELL))
    dam >>= 1;
  damage(ch, victim, dam, SPELL_ACID_BLAST);
}

void spell_cone_of_cold(char level, struct char_data *ch, struct char_data *victim,
			struct obj_data *obj)
{
  int                                     dam = 0;
  int                                     high_die = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  switch (HowManyClasses(ch)) {
    case 1:
      high_die = 5 + ((level / 5) - 1);
      break;
    case 2:
      high_die = 4 + ((level / 6) - 1);
      break;
    case 3:
      high_die = 4 + ((level / 7) - 1);
      break;
    case 4:
      high_die = 4 + ((level / 8) - 1);
      break;
    default:
      high_die = 4 + ((level / 9) - 1);
      break;
  }

  dam = dice(high_die, 3) + 1;

  cprintf(ch, "A cone of freezing air fans out before you\n\r");
  act("$n sends a cone of ice shooting from their fingertips!\n\r", FALSE, ch, 0, 0, TO_ROOM);
  AreaEffectSpell(ch, dam, SPELL_CONE_OF_COLD, 0, 0);
}

void spell_ice_storm(char level, struct char_data *ch, struct char_data *victim,
		     struct obj_data *obj)
{
  int                                     dam = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  dam = dice(3, 20);

  cprintf(ch, "You conjure a huge cloud of ice crystals...they hover just above\n\r");
  cprintf(ch, "your foes head, the weight finally brings them crashing down on top...\n\r");
  act("$n conjures a chilling storm of ice!\n\r", FALSE, ch, 0, 0, TO_ROOM);
  AreaEffectSpell(ch, dam, SPELL_ICE_STORM, 0, 0);
}

void spell_poison_cloud(char level, struct char_data *ch, struct char_data *victim,
			struct obj_data *obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));
}

void spell_chain_lightning(char level, struct char_data *ch, struct char_data *victim,
			   struct obj_data *obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));
}

void spell_major_create(char level, struct char_data *ch, struct char_data *victim,
			struct obj_data *obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));
}

void spell_summon_obj(char level, struct char_data *ch, struct char_data *victim,
		      struct obj_data *obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));
}

void spell_pword_blind(char level, struct char_data *ch, struct char_data *victim,
		       struct obj_data *obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));
}

void spell_pword_kill(char level, struct char_data *ch, struct char_data *victim,
		      struct obj_data *obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));
}

void spell_sending(char level, struct char_data *ch, struct char_data *victim,
		   struct obj_data *obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));
}

void spell_meteor_swarm(char level, struct char_data *ch, struct char_data *victim,
			struct obj_data *obj)
{
  int                                     dam = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  dam = dice((level / 2), 6) + level;
  cprintf(ch,
	  "You draw the symbols of fire and death in the air before you...\n\rWith a Word, you send it into your foes!!\n\r");
  act("$n sends intense balls of fire swirling all around the room!\n\r", FALSE, ch, 0, 0,
      TO_ROOM);
  AreaEffectSpell(ch, dam, SPELL_METEOR_SWARM, 0, 0);
}

void spell_Create_Monster(char level, struct char_data *ch, struct char_data *victim,
			  struct obj_data *obj)
{
  struct affected_type                    af;
  struct char_data                       *mob = NULL;
  int                                     rnum = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  /*
   * load in a monster of the correct type, determined by level of the spell 
   */

  /* really simple to start out with */

  if (level <= 5) {
    rnum = number(1, 10) + 200;
    mob = read_mobile(rnum, VIRTUAL);
  } else if (level <= 7) {
    rnum = number(1, 10) + 210;
    mob = read_mobile(rnum, VIRTUAL);
  } else if (level <= 9) {
    rnum = number(1, 10) + 220;
    mob = read_mobile(rnum, VIRTUAL);
  } else if (level <= 11) {
    rnum = number(1, 10) + 230;
    mob = read_mobile(rnum, VIRTUAL);
  } else if (level <= 13) {
    rnum = number(1, 10) + 240;
    mob = read_mobile(rnum, VIRTUAL);
  } else if (level <= 15) {
    rnum = number(1, 8) + 250;
    mob = read_mobile(rnum, VIRTUAL);
  } else {
    rnum = number(0, 1) + 261;
    mob = read_mobile(rnum, VIRTUAL);
  }

  char_to_room(mob, ch->in_room);

  act("$n waves $s hand, and $N appears!", TRUE, ch, 0, mob, TO_ROOM);
  act("You wave your hand, and $N appears!", TRUE, ch, 0, mob, TO_CHAR);

  GET_EXP(mob) = 50 * GetMaxLevel(mob);
  if (number(0, 99) > 70 + GetMaxLevel(ch) - GetMaxLevel(mob)) {
    act("$N turns to $n and snarls.", FALSE, ch, 0, mob, TO_ROOM);
    act("$N turns to you and snarls!", TRUE, ch, 0, mob, TO_CHAR);
    AddHated(mob, ch);
    if (!IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
      SET_BIT(mob->specials.act, ACT_AGGRESSIVE);
    }
  } else {
    if (mob->master)
      stop_follower(mob);
    af.type = SPELL_CHARM_PERSON;
    if (GET_INT(mob))
      af.duration = 24 - GET_INT(mob);
    else
      af.duration = 12;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char(mob, &af);
    add_follower(mob, ch);
    if (IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
      REMOVE_BIT(mob->specials.act, ACT_AGGRESSIVE);
    }
  }

/*
 * if(!IS_SET(mob->specials.act, ACT_SENTINEL)) 
 * {
 * SET_BIT(mob->specials.act, ACT_SENTINEL);
 * }
 */

}

/*
 * either
 */

void spell_light(char level, struct char_data *ch, struct char_data *victim,
		 struct obj_data *obj)
{
  struct obj_data                        *tmp_obj = NULL;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

/*
 * creates a ball of light in the hands.
 */
  CREATE(tmp_obj, struct obj_data, 1);

  clear_object(tmp_obj);

  tmp_obj->name = strdup("ball light");
  tmp_obj->short_description = strdup("A ball of light");
  tmp_obj->description = strdup("There is a ball of light on the ground here.");

  tmp_obj->obj_flags.type_flag = ITEM_LIGHT;
  tmp_obj->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD;
  tmp_obj->obj_flags.value[2] = 24;
  tmp_obj->obj_flags.weight = 1;
  tmp_obj->obj_flags.cost = 10;
  tmp_obj->obj_flags.cost_per_day = 1;

  tmp_obj->next = object_list;
  object_list = tmp_obj;

  obj_to_char(tmp_obj, ch);

  tmp_obj->item_number = -1;

  act("$n intones a magic word and $p suddenly appears.", TRUE, ch, tmp_obj, 0, TO_ROOM);
  act("With a flick of your wrist, $p suddenly appears.", TRUE, ch, tmp_obj, 0, TO_CHAR);
}

void spell_fly(char level, struct char_data *ch, struct char_data *victim, struct obj_data *obj)
{
  struct affected_type                    af;
  int                                     dur = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (IS_AFFECTED(victim, AFF_FLYING)) {
    cprintf(ch, "You are already flying....\n\r");
    return;
  }
  cprintf(victim, "Your feet begin to inch above the ground!\n\r");

  if (victim != ch)
    act("$N's feet rise off the ground.", TRUE, ch, 0, victim, TO_CHAR);
  else
    cprintf(ch, "Your feet rise up off the ground.");

  act("$N's feet rise off the ground.", TRUE, ch, 0, victim, TO_NOTVICT);

  dur = 10;

  switch (HowManyClasses(ch)) {
    case 1:
      dur = GET_LEVEL(ch, BestMagicClass(ch)) / 2 + 8;
      break;
    case 2:
      dur = GET_LEVEL(ch, BestMagicClass(ch)) / 2 + 4;
      break;
    case 3:
      dur = GET_LEVEL(ch, BestMagicClass(ch)) / 2;
      break;
    case 4:
      dur = GET_LEVEL(ch, BestMagicClass(ch)) / 4 + 8;
      break;
    default:
      dur = GET_LEVEL(ch, BestMagicClass(ch)) / 6 + 8;
      break;
  }

  af.type = SPELL_FLY;
  af.duration = dur;
  af.modifier = 0;
  af.location = 0;
  af.bitvector = AFF_FLYING;
  affect_to_char(victim, &af);
}

void spell_refresh(char level, struct char_data *ch, struct char_data *victim,
		   struct obj_data *obj)
{
  int                                     dam = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  dam = dice(level, 4) + level;
  dam = MAX(dam, 30);

  if (GET_LEVEL(ch, MAGE_LEVEL_IND) > 1)
    dam >>= 1;

  if ((dam + GET_MOVE(victim)) > move_limit(victim))
    GET_MOVE(victim) = move_limit(victim);
  else
    GET_MOVE(victim) += dam;

  cprintf(victim, "You feel less tired\n\r");
}

void spell_water_breath(char level, struct char_data *ch, struct char_data *victim,
			struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  act("You feel fishy!", TRUE, ch, 0, victim, TO_VICT);
  if (victim != ch) {
    act("$N makes a face like a fish.", TRUE, ch, 0, victim, TO_CHAR);
  }
  act("$N makes a face like a fish.", TRUE, ch, 0, victim, TO_NOTVICT);

  af.type = SPELL_WATER_BREATH;
  af.duration = GET_LEVEL(ch, BestMagicClass(ch));
  af.modifier = 0;
  af.location = 0;
  af.bitvector = AFF_WATERBREATH;
  affect_to_char(victim, &af);
}

void spell_cont_light(char level, struct char_data *ch, struct char_data *victim,
		      struct obj_data *obj)
{
/*
 * creates a ball of light in the hands.
 */
  struct obj_data                        *tmp_obj = NULL;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  CREATE(tmp_obj, struct obj_data, 1);

  clear_object(tmp_obj);

  tmp_obj->name = strdup("ball light");
  tmp_obj->short_description = strdup("A bright ball of light");
  tmp_obj->description = strdup("There is a bright ball of light on the ground here.");

  tmp_obj->obj_flags.type_flag = ITEM_LIGHT;
  tmp_obj->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD;
  tmp_obj->obj_flags.value[2] = 72;
  tmp_obj->obj_flags.weight = 1;
  tmp_obj->obj_flags.cost = 40;
  tmp_obj->obj_flags.cost_per_day = 1;

  tmp_obj->next = object_list;
  object_list = tmp_obj;

  obj_to_char(tmp_obj, ch);

  tmp_obj->item_number = -1;

  act("$n intones a power word and $p suddenly appears.", TRUE, ch, tmp_obj, 0, TO_ROOM);
  act("You twiddle your thumbs and $p suddenly appears.", TRUE, ch, tmp_obj, 0, TO_CHAR);

}

void spell_animate_dead(char level, struct char_data *ch, struct char_data *victim,
			struct obj_data *corpse)
{
  struct char_data                       *mob = NULL;
  struct obj_data                        *obj_object = NULL;
  struct obj_data                        *next_obj = NULL;
  char                                    buf[MAX_STRING_LENGTH] = "\0\0\0";
  int                                     r_num = 100;	       /* virtual # for zombie */
  int                                     k = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(corpse));

  if (!ch || !corpse)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

/*
 * some sort of check for corpse hood
 */
  if ((GET_ITEM_TYPE(corpse) != ITEM_CONTAINER) || (!corpse->obj_flags.value[3])) {
    cprintf(ch, "The magic fails abruptly!\n\r");
    return;
  }
  mob = read_mobile(r_num, VIRTUAL);
  char_to_room(mob, ch->in_room);

  act("With mystic power, $n animates a corpse.", TRUE, ch, 0, 0, TO_ROOM);
  act("$N slowly rises from the ground.", FALSE, ch, 0, mob, TO_ROOM);

/*
 * zombie should be charmed and follower ch
 */

  GET_EXP(mob) = 50 * GetMaxLevel(ch);
  IS_CARRYING_W(mob) = 0;
  IS_CARRYING_N(mob) = 0;

/*
 * mob->killer = obj->killer;
 */
/*
 * take all from corpse, and give to zombie 
 */

  for (obj_object = corpse->contains; obj_object; obj_object = next_obj) {
    next_obj = obj_object->next_content;
    obj_from_obj(obj_object);
    obj_to_char(obj_object, mob);
  }
  do_wear(mob, "all", 0);

/*
 * set up descriptions and such
 */
  sprintf(buf, "%s is here, slowly animating\n\r", corpse->short_description);
  mob->player.long_descr = strdup(buf);

/*
 * set up hitpoints
 */

  mob->points.max_hit = dice(GetMaxLevel(ch) / 4, 8) + 10;
  mob->points.hit = mob->points.max_hit;

  for (k = MAGE_LEVEL_IND; k <= RANGER_LEVEL_IND; k++)
    mob->player.level[k] = ch->player.level[k] / 4;

  mob->player.sex = 0;

  GET_RACE(mob) = RACE_UNDEAD;
  mob->player.class = ch->player.class;
  if (number(0, 99) > 50 + GetMaxLevel(ch)) {
    act("$N turns to $n and snarls.", FALSE, ch, 0, mob, TO_ROOM);
    act("$N turns to you and snarls!", TRUE, ch, 0, mob, TO_CHAR);
    AddHated(mob, ch);
    if (!IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
      SET_BIT(mob->specials.act, ACT_AGGRESSIVE);
    }
  } else {
    SET_BIT(mob->specials.affected_by, AFF_CHARM);
    add_follower(mob, ch);
    if (IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
      REMOVE_BIT(mob->specials.act, ACT_AGGRESSIVE);
    }
  }

/*
 * get rid of corpse
 */
  extract_obj(corpse);
  GET_ALIGNMENT(ch) -= number(10, 100);
  if (GET_ALIGNMENT(ch) < -999)
    GET_ALIGNMENT(ch) = -1000;
}

void spell_know_alignment(char level, struct char_data *ch, struct char_data *victim,
			  struct obj_data *obj)
{
  int                                     ap = 0;
  char                                    name[100] = "\0\0\0";

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (IS_NPC(victim))
    strcpy(name, victim->player.short_descr);
  else
    strcpy(name, GET_NAME(victim));

  ap = GET_ALIGNMENT(victim);

  if (ap > 700)
    cprintf(ch, "%s has an aura as white as the driven snow.\n\r", name);
  else if (ap > 350)
    cprintf(ch, "%s is of excellent moral character.\n\r", name);
  else if (ap > 100)
    cprintf(ch, "%s is often kind and thoughtful.\n\r", name);
  else if (ap > 25)
    cprintf(ch, "%s isn't a bad sort...\n\r", name);
  else if (ap > -25)
    cprintf(ch, "%s doesn't seem to have a firm moral commitment\n\r", name);
  else if (ap > -100)
    cprintf(ch, "%s isn't the worst you've come across\n\r", name);
  else if (ap > -350)
    cprintf(ch, "%s could be a little nicer, but who couldn't?\n\r", name);
  else if (ap > -700)
    cprintf(ch, "%s probably just had a bad childhood\n\r", name);
  else
    cprintf(ch, "I'd rather just not say anything at all about %s\n\r", name);
}

void spell_dispel_magic(char level, struct char_data *ch, struct char_data *victim,
			struct obj_data *obj)
{
  int                                     yes = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

/* gets rid of infravision, invisibility, detect, etc */

  if (!CheckKill(ch, victim))
    return;

  if ((GetMaxLevel(victim) <= GetMaxLevel(ch)))
    yes = TRUE;
  else
    yes = FALSE;

  if (affected_by_spell(victim, SPELL_INVISIBLE))
    if (yes || !saves_spell(victim, SAVING_SPELL)) {
      affect_from_char(victim, SPELL_INVISIBLE);
      cprintf(victim, "You feel exposed.\n\r");
    }
  if (affected_by_spell(victim, SPELL_DETECT_INVISIBLE))
    if (yes || !saves_spell(victim, SAVING_SPELL)) {
      affect_from_char(victim, SPELL_DETECT_INVISIBLE);
      cprintf(victim, "You feel less perceptive.\n\r");
    }
  if (affected_by_spell(victim, SPELL_DETECT_EVIL))
    if (yes || !saves_spell(victim, SAVING_SPELL)) {
      affect_from_char(victim, SPELL_DETECT_EVIL);
      cprintf(victim, "You feel less morally alert.\n\r");
    }
  if (affected_by_spell(victim, SPELL_DETECT_MAGIC))
    if (yes || !saves_spell(victim, SAVING_SPELL)) {
      affect_from_char(victim, SPELL_DETECT_MAGIC);
      cprintf(victim, "You stop noticing the magic in your life.\n\r");
    }
  if (affected_by_spell(victim, SPELL_SENSE_LIFE))
    if (yes || !saves_spell(victim, SAVING_SPELL)) {
      affect_from_char(victim, SPELL_SENSE_LIFE);
      cprintf(victim, "You feel less in touch with living things.\n\r");
    }
  if (affected_by_spell(victim, SPELL_SANCTUARY)) {
    if (yes || !saves_spell(victim, SAVING_SPELL)) {
      affect_from_char(victim, SPELL_SANCTUARY);
      cprintf(victim, "You don't feel so invulnerable anymore.\n\r");
      act("The white glow around $n's body fades.", FALSE, victim, 0, 0, TO_ROOM);
    }
    /*
     *  aggressive Act.
     */
    if ((victim->attackers < MAX_ATTACKERS) && (!victim->specials.fighting) && (IS_NPC(victim))) {
      set_fighting(victim, ch);
    }
  }
  if (IS_AFFECTED(victim, AFF_SANCTUARY)) {
    if (yes || !saves_spell(victim, SAVING_SPELL)) {
      REMOVE_BIT(victim->specials.affected_by, AFF_SANCTUARY);
      cprintf(victim, "You don't feel so invulnerable anymore.\n\r");
      act("The white glow around $n's body fades.", FALSE, victim, 0, 0, TO_ROOM);
    }
    /*
     *  aggressive Act.
     */
    if ((victim->attackers < MAX_ATTACKERS) && (!victim->specials.fighting) && (IS_NPC(victim))) {
      set_fighting(victim, ch);
    }
  }
  if (affected_by_spell(victim, SPELL_PROTECT_FROM_EVIL))
    if (yes || !saves_spell(victim, SAVING_SPELL)) {
      affect_from_char(victim, SPELL_PROTECT_FROM_EVIL);
      cprintf(victim, "You feel less morally protected.\n\r");
    }
  if (affected_by_spell(victim, SPELL_INFRAVISION))
    if (yes || !saves_spell(victim, SAVING_SPELL)) {
      affect_from_char(victim, SPELL_INFRAVISION);
      cprintf(victim, "Your sight grows dimmer.\n\r");
    }
  if (affected_by_spell(victim, SPELL_SLEEP))
    if (yes || !saves_spell(victim, SAVING_SPELL)) {
      affect_from_char(victim, SPELL_SLEEP);
      cprintf(victim, "You don't feel so tired.\n\r");
    }
  if (affected_by_spell(victim, SPELL_CHARM_PERSON))
    if (yes || !saves_spell(victim, SAVING_SPELL)) {
      affect_from_char(victim, SPELL_CHARM_PERSON);
      stop_follower(victim);
      cprintf(victim, "You feel less enthused about your master.\n\r");
    }
  if (affected_by_spell(victim, SPELL_WEAKNESS))
    if (yes || !saves_spell(victim, SAVING_SPELL)) {
      affect_from_char(victim, SPELL_WEAKNESS);
      cprintf(victim, "You don't feel so weak.\n\r");
    }
  if (affected_by_spell(victim, SPELL_STRENGTH))
    if (yes || !saves_spell(victim, SAVING_SPELL)) {
      affect_from_char(victim, SPELL_STRENGTH);
      cprintf(victim, "You don't feel so strong.\n\r");
    }
  if (affected_by_spell(victim, SPELL_ARMOR))
    if (yes || !saves_spell(victim, SAVING_SPELL)) {
      affect_from_char(victim, SPELL_ARMOR);
      cprintf(victim, "You don't feel so well protected.\n\r");
    }
  if (affected_by_spell(victim, SPELL_DETECT_POISON))
    if (yes || !saves_spell(victim, SAVING_SPELL)) {
      affect_from_char(victim, SPELL_DETECT_POISON);
      cprintf(victim, "You don't feel so sensitive to fumes.\n\r");
    }
  if (affected_by_spell(victim, SPELL_BLESS))
    if (yes || !saves_spell(victim, SAVING_SPELL)) {
      affect_from_char(victim, SPELL_BLESS);
      cprintf(victim, "You don't feel so blessed.\n\r");
    }
  if (affected_by_spell(victim, SPELL_FLY))
    if (yes || !saves_spell(victim, SAVING_SPELL)) {
      affect_from_char(victim, SPELL_FLY);
      cprintf(victim, "You don't feel lighter than air anymore.\n\r");
    }
  if (affected_by_spell(victim, SPELL_WATER_BREATH))
    if (yes || !saves_spell(victim, SAVING_SPELL)) {
      affect_from_char(victim, SPELL_WATER_BREATH);
      cprintf(victim, "You don't feel so fishy anymore.\n\r");
    }
  if (affected_by_spell(victim, SPELL_FIRE_BREATH))
    if (yes || !saves_spell(victim, SAVING_SPELL)) {
      affect_from_char(victim, SPELL_FIRE_BREATH);
      cprintf(victim, "You don't feel so fiery anymore.\n\r");
    }
  if (affected_by_spell(victim, SPELL_LIGHTNING_BREATH))
    if (yes || !saves_spell(victim, SAVING_SPELL)) {
      affect_from_char(victim, SPELL_LIGHTNING_BREATH);
      cprintf(victim, "You don't feel so electric anymore.\n\r");
    }
  if (affected_by_spell(victim, SPELL_GAS_BREATH))
    if (yes || !saves_spell(victim, SAVING_SPELL)) {
      affect_from_char(victim, SPELL_GAS_BREATH);
      cprintf(victim, "You don't have gas anymore.\n\r");
    }
  if (affected_by_spell(victim, SPELL_FROST_BREATH))
    if (yes || !saves_spell(victim, SAVING_SPELL)) {
      affect_from_char(victim, SPELL_FROST_BREATH);
      cprintf(victim, "You don't feel so frosty anymore.\n\r");
    }
  if (affected_by_spell(victim, SPELL_FIRESHIELD))
    if (yes || !saves_spell(victim, SAVING_SPELL)) {
      affect_from_char(victim, SPELL_FIRESHIELD);
      cprintf(victim, "You don't feel so firey anymore.\n\r");
      act("The red glow around $n's body fades.", TRUE, ch, 0, 0, TO_ROOM);
    }
  if (affected_by_spell(victim, SPELL_FAERIE_FIRE))
    if (yes || !saves_spell(victim, SAVING_SPELL)) {
      affect_from_char(victim, SPELL_FAERIE_FIRE);
      cprintf(victim, "You don't feel so pink anymore.\n\r");
      act("The pink glow around $n's body fades.", TRUE, ch, 0, 0, TO_ROOM);
    }
  if (level >= IMPLEMENTOR) {
    if (affected_by_spell(victim, SPELL_BLINDNESS)) {
      if (yes || !saves_spell(victim, SAVING_SPELL)) {
	affect_from_char(victim, SPELL_BLINDNESS);
	cprintf(victim, "Your vision returns.\n\r");
      }
    }
    if (affected_by_spell(victim, SPELL_PARALYSIS)) {
      if (yes || !saves_spell(victim, SAVING_SPELL)) {
	affect_from_char(victim, SPELL_PARALYSIS);
	cprintf(victim, "You feel freedom of movement.\n\r");
      }
    }
    if (affected_by_spell(victim, SPELL_POISON)) {
      if (yes || !saves_spell(victim, SAVING_SPELL)) {
	affect_from_char(victim, SPELL_POISON);
      }
    }
  }
}

void spell_paralyze(char level, struct char_data *ch, struct char_data *victim,
		    struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (!CheckKill(ch, victim))
    return;

  if (!IS_AFFECTED(victim, AFF_PARALYSIS)) {
    if (IsImmune(victim, IMM_HOLD)) {
      FailPara(victim, ch);
      return;
    }
    if (IsResist(victim, IMM_HOLD)) {
      if (saves_spell(victim, SAVING_PARA)) {
	FailPara(victim, ch);
	return;
      }
      if (saves_spell(victim, SAVING_PARA)) {
	FailPara(victim, ch);
	return;
      }
    } else if (!IsSusc(victim, IMM_HOLD)) {
      if (saves_spell(victim, SAVING_PARA)) {
	FailPara(victim, ch);
	return;
      }
    }
    af.type = SPELL_PARALYSIS;
    af.duration = dice(1, 6);
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_PARALYSIS;
    affect_join(victim, &af, FALSE, FALSE);

    act("Your limbs freeze in place", FALSE, victim, 0, 0, TO_CHAR);
    act("$n is paralyzed!", TRUE, victim, 0, 0, TO_ROOM);
    GET_POS(victim) = POSITION_STUNNED;
  } else {
    cprintf(victim, "Someone tries to paralyze you AGAIN!\n\r");
  }
}

void spell_fear(char level, struct char_data *ch, struct char_data *victim,
		struct obj_data *obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (!CheckKill(ch, victim))
    return;

  if (GetMaxLevel(ch) >= GetMaxLevel(victim) - 2) {
    if (!saves_spell(victim, SAVING_SPELL)) {

/* 
 * af.type      = SPELL_FEAR;
 * af.duration  = 4+level;
 * af.modifier  = 0;
 * af.location  = APPLY_NONE;
 * af.bitvector = 0;
 * affect_join(victim, &af, FALSE, FALSE);
 */
      do_flee(victim, "", 0);

    } else {
      cprintf(victim, "You feel afraid, but the effect fades.\n\r");
      return;
    }
  }
}

void spell_prot_align_group(char level, struct char_data *ch, struct char_data *victim,
			    struct obj_data *obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));
}

void spell_calm(char level, struct char_data *ch, struct char_data *victim,
		struct obj_data *obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

/* 
 * removes aggressive bit from monsters 
 */

  if (IS_NPC(victim)) {
    if (IS_SET(victim->specials.act, ACT_AGGRESSIVE)) {
      if (!saves_spell(victim, SAVING_PARA)) {
	REMOVE_BIT(victim->specials.act, ACT_AGGRESSIVE);
      } else {
	FailCalm(victim, ch);
      }
    } else {
      cprintf(victim, "You feel calm\n\r");
    }
  } else {
    cprintf(victim, "You feel calm.\n\r");
  }
}

void spell_conjure_elemental(char level, struct char_data *ch, struct char_data *victim,
			     struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim || !obj)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;
  /*
   *   victim, in this case, is the elemental
   *   object could be the sacrificial object
   */

  /*
   * ** objects:
   * **     fire  : red stone
   * **     water : pale blue stone
   * **     earth : grey stone
   * **     air   : clear stone
   */

  act("$n gestures, and a cloud of smoke appears", TRUE, ch, 0, 0, TO_ROOM);
  act("$n gestures, and a cloud of smoke appears", TRUE, ch, 0, 0, TO_CHAR);
  act("$p explodes with a loud BANG!", TRUE, ch, obj, 0, TO_ROOM);
  act("$p explodes with a loud BANG!", TRUE, ch, obj, 0, TO_CHAR);
  obj_from_char(obj);
  extract_obj(obj);
  char_to_room(victim, ch->in_room);
  act("Out of the smoke, $N emerges", TRUE, ch, 0, victim, TO_NOTVICT);

  /*
   * charm them for a while 
   */
  if (victim->master)
    stop_follower(victim);

  add_follower(victim, ch);

  af.type = SPELL_CHARM_PERSON;
  af.duration = 24;
  af.modifier = 0;
  af.location = 0;
  af.bitvector = AFF_CHARM;
  affect_to_char(victim, &af);
}

void spell_faerie_fire(char level, struct char_data *ch, struct char_data *victim,
		       struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (!CheckKill(ch, victim))
    return;

  if (affected_by_spell(victim, SPELL_FAERIE_FIRE)) {
    cprintf(ch, "Nothing new seems to happen");
    return;
  }
  act("$n points at $N.", TRUE, ch, 0, victim, TO_ROOM);
  act("You point at $N.", TRUE, ch, 0, victim, TO_CHAR);
  act("$N is surrounded by a pink outline", TRUE, ch, 0, victim, TO_ROOM);
  act("$N is surrounded by a pink outline", TRUE, ch, 0, victim, TO_CHAR);

  af.type = SPELL_FAERIE_FIRE;
  af.duration = level / 3;
  af.modifier = 10;
  af.location = APPLY_ARMOR;
  af.bitvector = 0;

  affect_to_char(victim, &af);

}

void spell_faerie_fog(char level, struct char_data *ch, struct char_data *victim,
		      struct obj_data *obj)
{
  struct char_data                       *tmp_victim = NULL;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  act("$n snaps $s fingers, and a cloud of purple smoke billows forth",
      TRUE, ch, 0, 0, TO_ROOM);
  act("You snap your fingers, and a cloud of purple smoke billows forth",
      TRUE, ch, 0, 0, TO_CHAR);

  for (tmp_victim = real_roomp(ch->in_room)->people; tmp_victim;
       tmp_victim = tmp_victim->next_in_room) {
    if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
      if (IS_IMMORTAL(tmp_victim))
	if (tmp_victim->invis_level > GetMaxLevel(ch))
	  break;
      if (!in_group(ch, tmp_victim)) {
	if (IS_AFFECTED(tmp_victim, AFF_INVISIBLE)) {
	  if (saves_spell(tmp_victim, SAVING_SPELL)) {
	    REMOVE_BIT(tmp_victim->specials.affected_by, AFF_INVISIBLE);
	    act("$n is briefly revealed, but dissapears again.",
		TRUE, tmp_victim, 0, 0, TO_ROOM);
	    act("You are briefly revealed, but dissapear again.",
		TRUE, tmp_victim, 0, 0, TO_CHAR);
	    SET_BIT(tmp_victim->specials.affected_by, AFF_INVISIBLE);
	  } else {
	    REMOVE_BIT(tmp_victim->specials.affected_by, AFF_INVISIBLE);
	    act("$n is revealed!", TRUE, tmp_victim, 0, 0, TO_ROOM);
	    act("You are revealed!", TRUE, tmp_victim, 0, 0, TO_CHAR);
	  }
	}
      }
    }
  }
}

void spell_cacaodemon(char level, struct char_data *ch, struct char_data *victim,
		      struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim || !obj)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  act("$n gestures, and a black cloud of smoke appears", TRUE, ch, 0, 0, TO_ROOM);
  act("$n gestures, and a black cloud of smoke appears", TRUE, ch, 0, 0, TO_CHAR);
  act("$p bursts into flame and disintegrates!", TRUE, ch, obj, 0, TO_ROOM);
  act("$p bursts into flame and disintegrates!", TRUE, ch, obj, 0, TO_CHAR);
  obj_from_char(obj);
  extract_obj(obj);
  char_to_room(victim, ch->in_room);

  act("With an evil laugh, $N emerges from the smoke", TRUE, ch, 0, victim, TO_NOTVICT);

  /*
   * charm them for a while 
   */
  if (victim->master)
    stop_follower(victim);

  add_follower(victim, ch);

  af.type = SPELL_CHARM_PERSON;
  af.duration = level;
  af.modifier = 0;
  af.location = 0;
  af.bitvector = AFF_CHARM;

  affect_to_char(victim, &af);

  if (IS_SET(victim->specials.act, ACT_AGGRESSIVE))
    REMOVE_BIT(victim->specials.act, ACT_AGGRESSIVE);

  if (!IS_SET(victim->specials.act, ACT_SENTINEL))
    SET_BIT(victim->specials.act, ACT_SENTINEL);

}

/*
 * neither
 */

void spell_improved_identify(char level, struct char_data *ch, struct char_data *victim,
			     struct obj_data *obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));
}

void spell_geyser(char level, struct char_data *ch, struct char_data *victim,
		  struct obj_data *obj)
{
  int                                     dam = 0;
  struct char_data                       *tmp_victim = NULL;
  struct char_data                       *temp = NULL;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (ch->in_room < 0)
    return;
  dam = dice(level, 3);

  act("The Geyser erupts in a huge column of steam!\n\r", FALSE, ch, 0, 0, TO_ROOM);

  for (tmp_victim = real_roomp(ch->in_room)->people; tmp_victim; tmp_victim = temp) {
    temp = tmp_victim->next_in_room;
    if ((ch != tmp_victim) && (ch->in_room == tmp_victim->in_room)) {
      if ((GetMaxLevel(tmp_victim) < LOW_IMMORTAL) || (IS_NPC(tmp_victim))) {
	damage(ch, tmp_victim, dam, SPELL_GEYSER);
	act("You are seared by the boiling water!!\n\r", FALSE, ch, 0, tmp_victim, TO_VICT);
      } else {
	act("You are almost seared by the boiling water!!\n\r",
	    FALSE, ch, 0, tmp_victim, TO_VICT);
      }
    }
  }
}

void spell_green_slime(char level, struct char_data *ch, struct char_data *victim,
		       struct obj_data *obj)
{
  int                                     dam = 0;
  int                                     hpch = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  hpch = GET_MAX_HIT(ch);
  if (hpch < 10)
    hpch = 10;

  dam = (int)(hpch / 10);

  if (saves_spell(victim, SAVING_BREATH))
    dam >>= 1;

  cprintf(victim, "You are attacked by green slime!\n\r");

  damage(ch, victim, dam, SPELL_GREEN_SLIME);

}

/*
 * file: magic3.c , Implementation of (new)spells.        Part of DIKUMUD
 * Usage : The actual effect of magic.
 */

/*
 *
 *  Start of new spells for WileyIIMud - Cyric
 *
 *
 */

void spell_fly_group(char level, struct char_data *ch, struct char_data *victim,
		     struct obj_data *obj)
{
  struct affected_type                    af;
  struct char_data                       *tch = NULL;
  int                                     dur = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (real_roomp(ch->in_room) == NULL)
    return;

  for (tch = real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room) {
    if (in_group(ch, tch)) {
      if (tch != ch) {
	act("Your feet rise off the ground!", TRUE, ch, 0, tch, TO_VICT);
	act("$N's feet rise off the ground.", TRUE, ch, 0, tch, TO_CHAR);
      } else {
	cprintf(ch, "Your feet rise up off the ground.");
      }
      act("$N's feet rise off the ground.", TRUE, ch, 0, tch, TO_NOTVICT);

      dur = 10;

      switch (HowManyClasses(ch)) {
	case 1:
	  dur = GET_LEVEL(ch, BestMagicClass(ch)) / 2 + 8;
	  break;
	case 2:
	  dur = GET_LEVEL(ch, BestMagicClass(ch)) / 2 + 4;
	  break;
	case 3:
	  dur = GET_LEVEL(ch, BestMagicClass(ch)) / 2;
	  break;
	case 4:
	  dur = GET_LEVEL(ch, BestMagicClass(ch)) / 4 + 8;
	  break;
	default:
	  dur = GET_LEVEL(ch, BestMagicClass(ch)) / 6 + 8;
	  break;
      }

      af.type = SPELL_FLY;
      af.duration = dur;
      af.modifier = 0;
      af.location = 0;
      af.bitvector = AFF_FLYING;
      affect_to_char(tch, &af);
    }
  }
}

void spell_aid(char level, struct char_data *ch, struct char_data *victim, struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (affected_by_spell(victim, SPELL_AID)) {
    cprintf(ch, "Already in effect\n\r");
    return;
  }
  GET_HIT(victim) += number(2, 8);

  update_pos(victim);

  act("$n looks aided", FALSE, victim, 0, 0, TO_ROOM);
  cprintf(victim, "You feel better!\n\r");

  af.type = SPELL_AID;
  af.duration = 10;
  af.modifier = 1;
  af.location = APPLY_HITROLL;
  af.bitvector = 0;
  affect_to_char(victim, &af);
}

void spell_goodberry(char level, struct char_data *ch, struct char_data *victim,
		     struct obj_data *obj)
{
  struct obj_data                        *tmp_obj = NULL;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
    cprintf(ch, "You can't cast this spell indoors!\n\r");
    return;
  }
  CREATE(tmp_obj, struct obj_data, 1);
  clear_object(tmp_obj);

  tmp_obj->name = strdup("berry blue blueberry");
  tmp_obj->short_description = strdup("a plump blueberry");
  tmp_obj->description = strdup("A scrumptions blueberry lies here.");

  tmp_obj->obj_flags.type_flag = ITEM_FOOD;
  tmp_obj->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD;
  tmp_obj->obj_flags.value[0] = 10;
  tmp_obj->obj_flags.weight = 5;
  tmp_obj->obj_flags.cost = 1;
  tmp_obj->obj_flags.cost_per_day = 1000;

  /*
   * give it a cure light wounds spell effect
   */

  SET_BIT(tmp_obj->obj_flags.extra_flags, ITEM_MAGIC);

  tmp_obj->affected[0].location = APPLY_EAT_SPELL;
  tmp_obj->affected[0].modifier = SPELL_CURE_LIGHT;

  tmp_obj->next = object_list;
  object_list = tmp_obj;

  obj_to_char(tmp_obj, ch);

  tmp_obj->item_number = -1;

  act("$p suddenly appears.", TRUE, ch, tmp_obj, 0, TO_ROOM);
  act("$p suddenly appears in your hand.", TRUE, ch, tmp_obj, 0, TO_CHAR);
}

#ifdef DRUID_WORKING

void spell_tree_travel(char level, struct char_data *ch, struct char_data *victim,
		       struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (!affected_by_spell(ch, SPELL_TREE_TRAVEL)) {
    af.type = SPELL_TREE_TRAVEL;

    af.duration = 24;
    af.modifier = -5;
    af.location = APPLY_AC;
    af.bitvector = AFF_TREE_TRAVEL;
    affect_to_char(victim, &af);

    cprintf(victim, "You feel as one with the trees... Groovy!\n\r");
  } else {
    cprintf(ch, "Nothing seems to happen\n\r");
  }

}

void spell_transport_via_plant(char level, struct char_data *ch, struct char_data *victim,
			       struct obj_data *obj)
{
  struct room_data                       *rp = NULL;
  struct obj_data                        *o = NULL;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !obj)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  /*
   * find the tree in the room 
   */

  rp = real_roomp(ch->in_room);
  for (o = rp->contents; o; o = o->next_content) {
    if (ITEM_TYPE(o) == ITEM_TREE)
      break;
  }

  if (!o) {
    cprintf(ch, "You need to have a tree nearby\n\r");
    return;
  }
  if (ITEM_TYPE(obj) != ITEM_TREE) {
    cprintf(ch, "Thats not a tree!\n\r");
    return;
  }
  if (obj->in_room < 0) {
    cprintf(ch, "That tree is nowhere to be found\n\r");
    return;
  }
  if (!real_roomp(obj->in_room)) {
    cprintf(ch, "That tree is nowhere to be found\n\r");
    return;
  }
  act("$n touches $p, and slowly vanishes within!", FALSE, ch, o, 0, TO_ROOM);
  act("You touch $p, and join your forms.", FALSE, ch, o, 0, TO_CHAR);
  char_from_room(ch);
  char_to_room(ch, obj->in_room);
  act("$p rustles slightly, and $n magically steps from within!", FALSE, ch, obj, 0, TO_ROOM);
  act("You are instantly transported to $p!", FALSE, ch, obj, 0, TO_CHAR);
  do_look(ch, "", 0);

}

void spell_speak_with_plants(char level, struct char_data *ch, struct char_data *victim,
			     struct obj_data *obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !obj)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (ITEM_TYPE(obj) != ITEM_TREE) {
    cprintf(ch, "Sorry, you can't talk to that sort of thing\n\r");
    return;
  }
  act("%s says 'Hi $n, how ya doin?'", FALSE, ch, obj, 0, TO_CHAR, fname(obj->name));
  act("$p rustles slightly.", FALSE, ch, obj, 0, TO_ROOM);

}

#define TREE 6110

void spell_changestaff(char level, struct char_data *ch, struct char_data *victim,
		       struct obj_data *obj)
{
  struct affected_type                    af;
  struct obj_data                        *s = NULL;
  struct char_data                       *t = NULL;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  /*
   * player must be holding staff at the time 
   */

  if (!ch->equipment[HOLD]) {
    cprintf(ch, "You must be holding a staff!\n\r");
    return;
  }
  if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
    cprintf(ch, "You can't cast this spell indoors!\n\r");
    return;
  }
  s = unequip_char(ch, HOLD);
  if (ITEM_TYPE(s) != ITEM_STAFF) {
    act("$p is not sufficient to complete this spell", FALSE, ch, s, 0, TO_CHAR);
    extract_obj(s);
    return;
  }
  if (!s->obj_flags.value[2]) {
    act("$p is not sufficiently powerful to complete this spell", FALSE, ch, s, 0, TO_CHAR);
    extract_obj(s);
    return;
  }
  act("$p vanishes in a burst of flame!", FALSE, ch, s, 0, TO_ROOM);
  act("$p vanishes in a burst of flame!", FALSE, ch, s, 0, TO_CHAR);

  t = read_mobile(TREE, VIRTUAL);
  char_to_room(t, ch->in_room);

  act("$n springs up in front of you!", FALSE, t, 0, 0, TO_ROOM);

  af.type = SPELL_CHARM_PERSON;

  if (IS_PC(ch) || ch->master) {
    af.duration = 10;					       /* keep it shorter, so fewer big groups of mobs */

    af.duration += s->obj_flags.value[2];		       /* num charges */

    af.modifier = 0;
    af.location = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char(t, &af);
  } else {
    SET_BIT(t->specials.affected_by, AFF_CHARM);
  }
  add_follower(t, ch);

  extract_obj(s);

}

/* mage spells */
void spell_pword_kill(char level, struct char_data *ch, struct char_data *victim,
		      struct obj_data *obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!victim)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (GET_MAX_HIT(victim) <= 80) {
    damage(ch, victim, GET_MAX_HIT(victim) * 12, SPELL_PWORD_KILL);
  } else {
    cprintf(ch, "They are too powerful to destroy this way\n\r");
  }

}

void spell_pword_blind(char level, struct char_data *ch, struct char_data *victim,
		       struct obj_data *obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!victim)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (GET_MAX_HIT(victim) <= 100) {
    SET_BIT(victim->specials.affected_by, AFF_BLIND);
  } else {
    cprintf(ch, "They are too powerful to blind this way\n\r");
  }

}

void spell_chain_lightn(char level, struct char_data *ch, struct char_data *victim,
			struct obj_data *obj)
{
  int                                     lev = level;
  struct char_data                       *t = NULL;
  struct char_data                       *next = NULL;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!victim)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  /*
   * victim = levd6 damage 
   */
  damage(ch, victim, dice(lev, 6), SPELL_LIGHTNING_BOLT);
  lev--;

  for (t = real_roomp(ch->in_room)->people; t; t = next) {
    next = t->next_in_room;
    if (!in_group(ch, t) && t != victim && (!IS_IMMORTAL(t) || IS_IMMORTAL(ch))) {
      damage(ch, t, dice(lev, 6), SPELL_LIGHTNING_BOLT);
      lev--;
    }
  }
}

void spell_scare(char level, struct char_data *ch, struct char_data *victim,
		 struct obj_data *obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!victim)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (GetMaxLevel(victim) <= 5)
    do_flee(victim, "", 0);
}

void spell_haste(char level, struct char_data *ch, struct char_data *victim,
		 struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (affected_by_spell(victim, SPELL_HASTE)) {
    cprintf(ch, "already hasted\n");
    return;
  }
  if (IS_NPC(victim)) {
    cprintf(ch, "It doesn't seem to work\n");
    return;
  }
  if (IS_IMMUNE(victim, IMM_HOLD)) {
    act("$N seems to ignore your spell", FALSE, ch, 0, victim, TO_CHAR);
    act("$n just tried to haste you, but you ignored it.", FALSE, ch, 0, victim, TO_VICT);
    if (!in_group(ch, victim)) {
      if (!IS_PC(ch))
	hit(victim, ch, TYPE_UNDEFINED);
    }
    return;
  }
  af.type = SPELL_HASTE;
  af.duration = level;
  af.modifier = 1;
  af.location = APPLY_HASTE;
  af.bitvector = 0;
  affect_to_char(victim, &af);

  cprintf(victim, "You feel fast!\n\r");
  if (!IS_NPC(victim))
    victim->player.time.birth -= SECS_PER_MUD_YEAR;
  else {
    if (victim->desc && victim->desc->original)
      victim->desc->original->player.time.birth -= SECS_PER_MUD_YEAR;
  }

  if (!in_group(ch, victim)) {
    if (!IS_PC(ch))
      hit(victim, ch, TYPE_UNDEFINED);
  }
}

void spell_slow(char level, struct char_data *ch, struct char_data *victim,
		struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (affected_by_spell(victim, SPELL_SLOW)) {
    cprintf(ch, "already slowed\n");
    return;
  }
  if (IS_IMMUNE(victim, IMM_HOLD)) {
    act("$N seems to ignore your spell", FALSE, ch, 0, victim, TO_CHAR);
    act("$n just tried to slow you, but you ignored it.", FALSE, ch, 0, victim, TO_VICT);
    if (!in_group(ch, victim)) {
      if (!IS_PC(ch))
	hit(victim, ch, TYPE_UNDEFINED);
    }
    return;
  }
  af.type = SPELL_SLOW;
  af.duration = 10;
  af.modifier = 1;
  af.location = APPLY_SLOW;
  af.bitvector = 0;
  affect_to_char(victim, &af);

  cprintf(victim, "You feel very slow!\n\r");

  if (!in_group(ch, victim)) {
    if (!IS_PC(ch))
      hit(victim, ch, TYPE_UNDEFINED);
  }
}

#define KITTEN  3090
#define PUPPY   3091
#define BEAGLE  3092
#define ROTT    3093
#define WOLF    3094

void spell_familiar(char level, struct char_data *ch, struct char_data **victim,
		    struct obj_data *obj)
{
  struct affected_type                    af;
  struct char_data                       *f = NULL;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (affected_by_spell(ch, SPELL_FAMILIAR)) {
    cprintf(ch, "You can't have more than 1 familiar per day\n\r");
    return;
  }
  /*
   * depending on the level, one of the pet shop kids.
   */

  if (level < 2)
    f = read_mobile(KITTEN, VIRTUAL);
  else if (level < 4)
    f = read_mobile(PUPPY, VIRTUAL);
  else if (level < 6)
    f = read_mobile(BEAGLE, VIRTUAL);
  else if (level < 8)
    f = read_mobile(ROTT, VIRTUAL);
  else
    f = read_mobile(WOLF, VIRTUAL);

  char_to_room(f, ch->in_room);

  af.type = SPELL_FAMILIAR;
  af.duration = 24;
  af.modifier = -1;
  af.location = APPLY_ARMOR;
  af.bitvector = 0;
  affect_to_char(ch, &af);

  act("$n appears in a flash of light!\n\r", FALSE, f, 0, 0, TO_ROOM);

  SET_BIT(f->specials.affected_by, AFF_CHARM);
  GET_EXP(f) = 0;
  add_follower(f, ch);
  IS_CARRYING_W(f) = 0;
  IS_CARRYING_N(f) = 0;

  *victim = f;

}

/* cleric */

void spell_holyword(char level, struct char_data *ch, struct char_data *victim,
		    struct obj_data *obj)
{
  int                                     lev = 0;
  int                                     t_align = 0;
  struct char_data                       *t = NULL;
  struct char_data                       *next = NULL;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch)
    return;

  if (level > 0)
    t_align = -300;
  else {
    level = -level;
    t_align = 300;
  }

  for (t = real_roomp(ch->in_room)->people; t; t = next) {
    next = t->next_in_room;
    if ((!IS_IMMORTAL(t) || IS_IMMORTAL(ch)) && !IS_AFFECTED(t, AFF_SILENCE)) {
      if (level > 0) {
	if (GET_ALIGNMENT(t) <= t_align) {
	  if ((lev = GetMaxLevel(t)) <= 4) {
	    damage(ch, t, GET_MAX_HIT(t) * 20, SPELL_HOLY_WORD);
	  } else if (lev <= 8) {
	    damage(ch, t, 1, SPELL_HOLY_WORD);
	    spell_paralyze(level, ch, t, 0);
	  } else if (lev <= 12) {
	    damage(ch, t, 1, SPELL_HOLY_WORD);
	    spell_blindness(level, ch, t, 0);
	  } else if (lev <= 16) {
	    damage(ch, t, 0, SPELL_HOLY_WORD);
	    GET_POS(t) = POSITION_STUNNED;
	  }
	}
      } else {
	if (GET_ALIGNMENT(t) >= t_align) {
	  if ((lev = GetMaxLevel(t)) <= 4) {
	    damage(ch, t, GET_MAX_HIT(t) * 20, SPELL_UNHOLY_WORD);
	  } else if (lev <= 8) {
	    damage(ch, t, 1, SPELL_UNHOLY_WORD);
	    spell_paralyze(level, ch, t, 0);
	  } else if (lev <= 12) {
	    damage(ch, t, 1, SPELL_UNHOLY_WORD);
	    spell_blindness(level, ch, t, 0);
	  } else if (lev <= 16) {
	    damage(ch, t, 1, SPELL_UNHOLY_WORD);
	    GET_POS(t) = POSITION_STUNNED;
	  }
	}
      }
    }
  }
}

#define GOLEM 38

void spell_golem(char level, struct char_data *ch, struct char_data *victim,
		 struct obj_data *obj)
{
  int                                     count = 0;
  int                                     armor = 0;
  struct char_data                       *gol = NULL;
  struct obj_data                        *helm = NULL;
  struct obj_data                        *jacket = NULL;
  struct obj_data                        *leggings = NULL;
  struct obj_data                        *sleeves = NULL;
  struct obj_data                        *gloves = NULL;
  struct obj_data                        *boots = NULL;
  struct obj_data                        *o = NULL;
  struct room_data                       *rp = NULL;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

/* you need:  helm, jacket, leggings, sleeves, gloves, boots */

  rp = real_roomp(ch->in_room);
  if (!rp)
    return;

  for (o = rp->contents; o; o = o->next_content) {
    if (ITEM_TYPE(o) == ITEM_ARMOR) {
      if (IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_HEAD)) {
	if (!helm) {
	  count++;
	  helm = o;
	  continue;					       /* next item */
	}
      }
      if (IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_FEET)) {
	if (!boots) {
	  count++;
	  boots = o;
	  continue;					       /* next item */
	}
      }
      if (IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_BODY)) {
	if (!jacket) {
	  count++;
	  jacket = o;
	  continue;					       /* next item */
	}
      }
      if (IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_LEGS)) {
	if (!leggings) {
	  count++;
	  leggings = o;
	  continue;					       /* next item */
	}
      }
      if (IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_ARMS)) {
	if (!sleeves) {
	  count++;
	  sleeves = o;
	  continue;					       /* next item */
	}
      }
      if (IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_HANDS)) {
	if (!gloves) {
	  count++;
	  gloves = o;
	  continue;					       /* next item */
	}
      }
    }
  }

  if (count < 6) {
    cprintf(ch, "You don't have all the correct pieces!\n\r");
    return;
  }
  if (count > 6) {
    cprintf(ch, "Smells like an error to me!\n\r");
    return;
  }
  if (!boots || !sleeves || !gloves || !helm || !jacket || !leggings) {
    /*
     * shouldn't get this far 
     */
    cprintf(ch, "You don't have all the correct pieces!\n\r");
    return;
  }
  gol = read_mobile(GOLEM, VIRTUAL);
  char_to_room(gol, ch->in_room);

  /*
   * add up the armor values in the pieces 
   */
  armor = boots->obj_flags.value[0];
  armor += helm->obj_flags.value[0];
  armor += gloves->obj_flags.value[0];
  armor += (leggings->obj_flags.value[0] * 2);
  armor += (sleeves->obj_flags.value[0] * 2);
  armor += (jacket->obj_flags.value[0] * 3);

  GET_AC(gol) -= armor;

  gol->points.max_hit = dice((armor / 6), 10) + GetMaxLevel(ch);
  GET_HIT(gol) = GET_MAX_HIT(gol);

  GET_LEVEL(gol, WARRIOR_LEVEL_IND) = (armor / 6);

  SET_BIT(gol->specials.affected_by, AFF_CHARM);
  GET_EXP(gol) = 0;
  IS_CARRYING_W(gol) = 0;
  IS_CARRYING_N(gol) = 0;

  gol->player.class = CLASS_WARRIOR;

  if (GET_LEVEL(gol, WARRIOR_LEVEL_IND) > 10)
    gol->mult_att += 0.5;

  /*
   * add all the effects from all the items to the golem 
   */
  AddAffects(gol, boots);
  AddAffects(gol, gloves);
  AddAffects(gol, jacket);
  AddAffects(gol, sleeves);
  AddAffects(gol, leggings);
  AddAffects(gol, helm);

  act("$n waves $s hand over a pile of armor on the floor", FALSE, ch, 0, 0, TO_ROOM);
  act("You wave your hands over the pile of armor", FALSE, ch, 0, 0, TO_CHAR);

  act("The armor flys together to form a humanoid figure!", FALSE, ch, 0, 0, TO_ROOM);

  act("$N is quickly assembled from the pieces", FALSE, ch, 0, gol, TO_CHAR);

  add_follower(gol, ch);

  extract_obj(helm);
  extract_obj(boots);
  extract_obj(gloves);
  extract_obj(leggings);
  extract_obj(sleeves);
  extract_obj(jacket);

}

/***************/

void spell_feeblemind(char level, struct char_data *ch, struct char_data *victim,
		      struct obj_data *obj)
{
  struct affected_type                    af;
  int                                     t = 0;
  int                                     i = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (!saves_spell(victim, SAVING_SPELL)) {

    cprintf(victim, "You feel really really dumb\n\r");

    af.type = SPELL_FEEBLEMIND;
    af.duration = 24;
    af.modifier = -5;
    af.location = APPLY_INT;
    af.bitvector = 0;
    affect_to_char(victim, &af);

    af.type = SPELL_FEEBLEMIND;
    af.duration = 24;
    af.modifier = 70;
    af.location = APPLY_SPELLFAIL;
    af.bitvector = 0;
    affect_to_char(victim, &af);

    /*
     * last, but certainly not least
     */
    if (!victim->skills)
      return;

    t = number(1, 100);

    while (1) {
      for (i = 0; i < MAX_SKILLS; i++) {
	if (victim->skills[i].learned)
	  t--;
	if (t == 0) {
	  victim->skills[i].learned = 0;
	  victim->skills[i].flags = 0;
	  break;
	}
      }
    }
  }
}

void spell_shillelagh(char level, struct char_data *ch, struct char_data *victim,
		      struct obj_data *obj)
{
  int                                     i = 0;
  int                                     count = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !obj)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (!(MAX_OBJ_AFFECT >= 2))
    return;

  if ((GET_ITEM_TYPE(obj) == ITEM_WEAPON) && !IS_SET(obj->obj_flags.extra_flags, ITEM_MAGIC)) {

    if (!isname("club", obj->name)) {
      cprintf(ch, "That isn't a club!\n\r");
      return;
    }
    for (i = 0; i < MAX_OBJ_AFFECT; i++) {
      if (obj->affected[i].location == APPLY_NONE)
	count++;
      if (obj->affected[i].location == APPLY_HITNDAM ||
	  obj->affected[i].location == APPLY_HITROLL ||
	  obj->affected[i].location == APPLY_DAMROLL)
	return;
    }

    if (count < 2)
      return;
    /*
     * find the slots 
     */
    i = getFreeAffSlot(obj);

    SET_BIT(obj->obj_flags.extra_flags, ITEM_MAGIC);

    obj->affected[i].location = APPLY_HITNDAM;
    obj->affected[i].modifier = 1;

    obj->obj_flags.value[1] = 2;
    obj->obj_flags.value[2] = 4;

    act("$p glows yellow.", FALSE, ch, obj, 0, TO_CHAR);
    SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD | ITEM_ANTI_EVIL);

  }
}

void spell_flame_blade(char level, struct char_data *ch, struct char_data *victim,
		       struct obj_data *obj)
{
  struct obj_data                        *tmp_obj = NULL;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (ch->equipment[WIELD]) {
    cprintf(ch, "You can't be wielding a weapon\n\r");
    return;
  }
  CREATE(tmp_obj, struct obj_data, 1);
  clear_object(tmp_obj);

  tmp_obj->name = strdup("blade flame");
  tmp_obj->short_description = strdup("a flame blade");
  tmp_obj->description = strdup("A flame blade burns brightly here");

  tmp_obj->obj_flags.type_flag = ITEM_WEAPON;
  tmp_obj->obj_flags.wear_flags = ITEM_TAKE | ITEM_WIELD;
  tmp_obj->obj_flags.value[0] = 0;
  tmp_obj->obj_flags.value[1] = 1;
  tmp_obj->obj_flags.value[2] = 4;
  tmp_obj->obj_flags.value[3] = 3;
  tmp_obj->obj_flags.weight = 1;
  tmp_obj->obj_flags.cost = 10;
  tmp_obj->obj_flags.cost_per_day = 1;

  /*
   * give it a cure light wounds spell effect
   */

  SET_BIT(tmp_obj->obj_flags.extra_flags, ITEM_MAGIC);

  tmp_obj->affected[0].location = APPLY_DAMROLL;
  tmp_obj->affected[0].modifier = 4;

  tmp_obj->next = object_list;
  object_list = tmp_obj;

  equip_char(ch, tmp_obj, WIELD);

  tmp_obj->item_number = -1;

  act("$p appears in your hand.", TRUE, ch, tmp_obj, 0, TO_CHAR);
  act("$p appears in $n's hand.", TRUE, ch, tmp_obj, 0, TO_ROOM);
}

void spell_animal_growth(char level, struct char_data *ch, struct char_data *victim,
			 struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
    cprintf(ch, "You can't cast this spell indoors!\n\r");
    return;
  }
  if (!IsAnimal(victim)) {
    cprintf(ch, "Thats not an animal\n\r");
    return;
  }
  if (affected_by_spell(victim, SPELL_ANIMAL_GROWTH)) {
    act("$N is already affected by that spell", FALSE, ch, 0, victim, TO_CHAR);
    return;
  }
  if (GetMaxLevel(victim) * 2 > GetMaxLevel(ch)) {
    cprintf(ch, "You can't make it more powerful than you!\n\r");
    return;
  }
  if (IS_PC(victim)) {
    cprintf(ch, "It would be in bad taste to cast that on a player\n\r");
    return;
  }
  act("$n grows to double $s original size!", FALSE, victim, 0, 0, TO_ROOM);
  act("You grow to double your original size!", FALSE, victim, 0, 0, TO_CHAR);

  af.type = SPELL_ANIMAL_GROWTH;
  af.duration = 3;
  af.modifier = GET_MAX_HIT(victim);
  af.location = APPLY_HIT;
  af.bitvector = AFF_GROWTH;
  affect_to_char(victim, &af);

  af.type = SPELL_ANIMAL_GROWTH;
  af.duration = 3;
  af.modifier = 5;
  af.location = APPLY_HITNDAM;
  af.bitvector = 0;
  affect_to_char(victim, &af);

  af.type = SPELL_ANIMAL_GROWTH;
  af.duration = 3;
  af.modifier = 3;
  af.location = APPLY_SAVE_ALL;
  af.bitvector = 0;
  affect_to_char(victim, &af);
/*
 * 
 * GET_LEVEL(victim, WARRIOR_LEVEL_IND) = 2*GetMaxLevel(victim);
 */
}

void spell_insect_growth(char level, struct char_data *ch, struct char_data *victim,
			 struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (GET_RACE(victim) != RACE_INSECT) {
    cprintf(ch, "Thats not an insect.\n\r");
    return;
  }
  if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
    cprintf(ch, "You can't cast this spell indoors!\n\r");
    return;
  }
  if (affected_by_spell(victim, SPELL_INSECT_GROWTH)) {
    act("$N is already affected by that spell", FALSE, ch, 0, victim, TO_CHAR);
    return;
  }
  if (GetMaxLevel(victim) * 2 > GetMaxLevel(ch)) {
    cprintf(ch, "You can't make it more powerful than you!\n\r");
    return;
  }
  if (IS_PC(victim)) {
    cprintf(ch, "It would be in bad taste to cast that on a player\n\r");
    return;
  }
  act("$n grows to double $s original size!", FALSE, victim, 0, 0, TO_ROOM);
  act("You grow to double your original size!", FALSE, victim, 0, 0, TO_CHAR);

  af.type = SPELL_INSECT_GROWTH;
  af.duration = 3;
  af.modifier = GET_MAX_HIT(victim);
  af.location = APPLY_HIT;
  af.bitvector = AFF_GROWTH;
  affect_to_char(victim, &af);

  af.type = SPELL_INSECT_GROWTH;
  af.duration = 3;
  af.modifier = 5;
  af.location = APPLY_HITNDAM;
  af.bitvector = 0;
  affect_to_char(victim, &af);

  af.type = SPELL_INSECT_GROWTH;
  af.duration = 3;
  af.modifier = 3;
  af.location = APPLY_SAVE_ALL;
  af.bitvector = 0;
  affect_to_char(victim, &af);
/*
 * GET_LEVEL(victim, WARRIOR_LEVEL_IND) = 2*GetMaxLevel(victim);
 */
}

#define CREEPING_DEATH 39

void spell_creeping_death(char level, struct char_data *ch, struct char_data *victim, int dir)
{
  struct affected_type                    af;
  struct char_data                       *cd = NULL;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
    cprintf(ch, "You can't cast this spell indoors!\n\r");
    return;
  }
  /*
   * obj is really the direction that the death wishes to travel in 
   */

  cd = read_mobile(CREEPING_DEATH, VIRTUAL);
  if (!cd) {
    cprintf(ch, "None available\n\r");
    return;
  }
  char_to_room(cd, ch->in_room);
  cd->points.max_hit += (number(1, 4) * 100) + 600;

  cd->points.hit = cd->points.max_hit;

  act("$n makes a horrid coughing sound", FALSE, ch, 0, 0, TO_ROOM);
  cprintf(ch, "You feel an incredibly nasty feeling inside\n\r");

  act("A huge gout of poisonous insects spews forth from $n's mouth!",
      FALSE, ch, 0, 0, TO_ROOM);
  cprintf(ch, "A huge gout of insects spews out of your mouth!\n\r");
  cprintf(ch, "My GOD thats disgusting!!!!!!!!!!\n\r");

  act("The insects coalesce into a solid mass - $n", FALSE, ch, 0, 0, TO_ROOM);

  cd->act_ptr = dir;

  /*
   * move the creeping death in the proper direction 
   */

  do_move(cd, "", dir);

  GET_POS(ch) = POSITION_STUNNED;

  af.type = SPELL_CREEPING_DEATH;
  af.duration = 2;
  af.modifier = 10500;
  af.location = APPLY_SPELLFAIL;
  af.bitvector = 0;
  affect_to_char(ch, &af);

}

void spell_commune(char level, struct char_data *ch, struct char_data *victim,
		   struct obj_data *obj)
{
  struct char_data                       *c = NULL;
  struct room_data                       *rp = NULL;
  struct room_data                       *dp = NULL;
  char                                    buf[MAX_STRING_LENGTH] = "\0\0\0";
  char                                    buffer[MAX_STRING_LENGTH] = "\0\0\0";

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  /*
   * look up the creatures in the mob list, find the ones in this zone, in rooms that are outdoors, and tell the
   * caster about them 
   */

  dp = real_roomp(ch->in_room);
  if (!dp)
    return;
  if (IS_SET(dp->room_flags, INDOORS)) {
    cprintf(ch, "You can't cast this spell indoors!\n\r");
    return;
  }
  for (c = character_list; c; c = c->next) {
    rp = real_roomp(c->in_room);
    if (!rp)
      return;

    if (rp->zone == dp->zone) {
      if (!IS_SET(rp->room_flags, INDOORS)) {
	sprintf(buf, "%s is in %s\n\r", (IS_NPC(c) ? c->player.short_descr : GET_NAME(c)),
		rp->name);
	if (strlen(buf) + strlen(buffer) > MAX_STRING_LENGTH - 2)
	  break;
	strcat(buffer, buf);
	strcat(buffer, "\r");
      }
    }
  }

  page_string(ch->desc, buffer, 1);

}

#define ANISUM1  9007
#define ANISUM2  9014
#define ANISUM3  9021

void spell_animal_summon(char level, struct char_data *ch, struct char_data *victim,
			 struct obj_data *obj)
{
  struct affected_type                    af;
  struct char_data                       *mob = NULL;
  int                                     num = 0;
  int                                     i = 0;
  struct room_data                       *rp = NULL;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  /*
   * load in a monster of the correct type, determined by level of the spell 
   */

/* really simple to start out with */

  if ((rp = real_roomp(ch->in_room)) == NULL)
    return;

  if (IS_SET(rp->room_flags, TUNNEL)) {
    cprintf(ch, "There isn't enough room in here to summon that.\n\r");
    return;
  }
  if (IS_SET(rp->room_flags, INDOORS)) {
    cprintf(ch, "You can only do this outdoors\n");
    return;
  }
  if (affected_by_spell(ch, SPELL_ANIMAL_SUM_1)) {
    cprintf(ch, "You can only do this once every 48 hours!\n\r");
    return;
  }
  if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
    cprintf(ch, "You can't cast this spell indoors!\n\r");
    return;
  }
  switch (level) {
    case 1:
      num = ANISUM1;
      break;
    case 2:
      num = ANISUM2;
      break;
    case 3:
      num = ANISUM3;
      break;
  }

  act("$n performs a complicated ritual!", TRUE, ch, 0, 0, TO_ROOM);
  act("You perform the ritual of summoning", TRUE, ch, 0, 0, TO_CHAR);

  for (i = 0; i < 3; i++) {

    mob = read_mobile(num + number(0, 5), VIRTUAL);

    if (!mob)
      continue;

    char_to_room(mob, ch->in_room);
    act("$n strides into the room", FALSE, mob, 0, 0, TO_ROOM);

    /*
     * charm them for a while 
     */
    if (mob->master)
      stop_follower(mob);

    add_follower(mob, ch);

    af.type = SPELL_CHARM_PERSON;

    if (IS_PC(ch) || ch->master) {
      af.duration = 8;
      af.modifier = 0;
      af.location = 0;
      af.bitvector = AFF_CHARM;
      affect_to_char(mob, &af);

    } else {
      SET_BIT(mob->specials.affected_by, AFF_CHARM);
    }

    if (IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
      REMOVE_BIT(mob->specials.act, ACT_AGGRESSIVE);
    }
    if (!IS_SET(mob->specials.act, ACT_SENTINEL)) {
      SET_BIT(mob->specials.act, ACT_SENTINEL);
    }
  }

  af.type = SPELL_ANIMAL_SUM_1;
  af.duration = 48;
  af.modifier = 0;
  af.location = 0;
  af.bitvector = 0;
  affect_to_char(ch, &af);

}

#define FIRE_ELEMENTAL  40

void spell_elemental_summoning(char level, struct char_data *ch, struct char_data *victim,
			       int spell)
{
  int                                     vnum = 0;
  struct char_data                       *mob = NULL;
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (affected_by_spell(ch, spell)) {
    cprintf(ch, "You can only do this once per 24 hours\n\r");
    return;
  }
  vnum = spell - SPELL_FIRE_SERVANT;
  vnum += FIRE_ELEMENTAL;

  if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
    cprintf(ch, "You can't cast this spell indoors!\n\r");
    return;
  }
  mob = read_mobile(vnum, VIRTUAL);

  if (!mob) {
    cprintf(ch, "None available\n\r");
    return;
  }
  act("$n performs a complicated ritual!", TRUE, ch, 0, 0, TO_ROOM);
  act("You perform the ritual of summoning", TRUE, ch, 0, 0, TO_CHAR);

  char_to_room(mob, ch->in_room);
  act("$n appears through a momentary rift in the ether!", FALSE, mob, 0, 0, TO_ROOM);

  /*
   * charm them for a while 
   */
  if (mob->master)
    stop_follower(mob);

  add_follower(mob, ch);

  af.type = SPELL_CHARM_PERSON;

  if (IS_PC(ch) || ch->master) {
    af.duration = 24;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char(mob, &af);

  } else {
    SET_BIT(mob->specials.affected_by, AFF_CHARM);
  }

  af.type = spell;
  af.duration = 24;
  af.modifier = 0;
  af.location = 0;
  af.bitvector = 0;
  affect_to_char(ch, &af);

/*
 * adjust the bits...
 */

/*
 * get rid of aggressive, add sentinel
 */

  if (IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
    REMOVE_BIT(mob->specials.act, ACT_AGGRESSIVE);
  }
  if (!IS_SET(mob->specials.act, ACT_SENTINEL)) {
    SET_BIT(mob->specials.act, ACT_SENTINEL);
  }
}

void spell_reincarnate(char level, struct char_data *ch, struct char_data *victim,
		       struct obj_data *obj)
{
  struct char_data                       *newch = NULL;
  struct char_file_u                      st;
  struct descriptor_data                 *d = NULL;
  FILE                                   *fl = NULL;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !obj)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
    cprintf(ch, "You can't cast this spell indoors!\n\r");
    return;
  }
  if (real_roomp(ch->in_room)->sector_type != SECT_FOREST) {
    cprintf(ch, "You must cast this spell in the forest!\n\r");
    return;
  }
  if (IS_CORPSE(obj)) {

    if (obj->char_vnum) {
      cprintf(ch, "This spell only works on players\n\r");
      return;
    }
    if (obj->char_f_pos) {

      fl = fopen(PLAYER_FILE, "r+");
      if (!fl) {
	log_error("player file");
      }
      fseek(fl, obj->char_f_pos * sizeof(struct char_file_u), 0);
      fread(&st, sizeof(struct char_file_u), 1, fl);

      /*
       **   this is a serious kludge, and must be changed before multiple
       **   languages can be implemented
       */
      if (st.talks[2] && st.abilities.con > 3) {
	st.points.exp *= 2;
	st.talks[2] = TRUE;
	st.abilities.con -= 1;

	st.race = GetNewRace(&st);

	act("The forest comes alive with the sounds of birds and animals",
	    TRUE, ch, 0, 0, TO_CHAR);
	act("The forest comes alive with the sounds of birds and animals",
	    TRUE, ch, 0, 0, TO_ROOM);
	act("$p dissappears in the blink of an eye.", TRUE, ch, obj, 0, TO_CHAR);
	act("$p dissappears in the blink of an eye.", TRUE, ch, obj, 0, TO_ROOM);
	GET_MANA(ch) = 1;
	GET_MOVE(ch) = 1;
	GET_HIT(ch) = 1;
	GET_POS(ch) = POSITION_SITTING;
	act("$n collapses from the effort!", TRUE, ch, 0, 0, TO_ROOM);
	cprintf(ch, "You collapse from the effort\n\r");

	fseek(fl, obj->char_f_pos * sizeof(struct char_file_u), 0);
	fwrite(&st, sizeof(struct char_file_u), 1, fl);

	ObjFromCorpse(obj);

	CREATE(newch, struct char_data, 1);

	clear_char(newch);

	store_to_char(&st, newch);

	reset_char(newch);

	newch->next = character_list;
	character_list = newch;

	char_to_room(newch, ch->in_room);
	newch->invis_level = 51;

	set_title(newch);
	GET_HIT(newch) = 1;
	GET_MANA(newch) = 1;
	GET_MOVE(newch) = 1;
	GET_POS(newch) = POSITION_SITTING;
	save_char(newch, AUTO_RENT);

	/*
	 * if they are in the descriptor list, suck them into the game 
	 */

	for (d = descriptor_list; d; d = d->next) {
	  if (d->character && (strcmp(GET_NAME(d->character), GET_NAME(newch)) == 0)) {
	    if (STATE(d) != CON_PLAYING) {
	      free_char(d->character);
	      d->character = newch;
	      STATE(d) = CON_PLAYING;
	      newch->desc = d;
	      cprintf(newch, "You awake to find yourself changed\n\r");
	      break;
	    }
	  }
	}

      } else {
	cprintf(ch, "The spirit does not have the strength to be reincarnated\n\r");
      }
      FCLOSE(fl);
    }
  }
}

void spell_charm_veggie(char level, struct char_data *ch, struct char_data *victim,
			struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (victim == ch) {
    cprintf(ch, "You like yourself even better!\n\r");
    return;
  }
  if (!IsVeggie(victim)) {
    cprintf(ch, "This can only be used on plants!\n\r");
    return;
  }
  if (GetMaxLevel(victim) > GetMaxLevel(ch) + 10) {
    FailCharm(victim, ch);
    return;
  }
  if (!IS_AFFECTED(victim, AFF_CHARM) && !IS_AFFECTED(ch, AFF_CHARM)) {
    if (circle_follow(victim, ch)) {
      cprintf(ch, "Sorry, following in circles can not be allowed.\n\r");
      return;
    }
    if (IsImmune(victim, IMM_CHARM) || (WeaponImmune(victim))) {
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
      af.duration = 24 * 18 / GET_INT(victim);
    else
      af.duration = 24 * 18;

    af.modifier = 0;
    af.location = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char(victim, &af);

    act("Isn't $n just such a nice fellow?", FALSE, ch, 0, victim, TO_VICT);
  }
}

void spell_veggie_growth(char level, struct char_data *ch, struct char_data *victim,
			 struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (!IsVeggie(victim)) {
    cprintf(ch, "Thats not a plant-creature!\n\r");
    return;
  }
  if (affected_by_spell(victim, SPELL_VEGGIE_GROWTH)) {
    act("$N is already affected by that spell", FALSE, ch, 0, victim, TO_CHAR);
    return;
  }
  if (GetMaxLevel(victim) * 2 > GetMaxLevel(ch)) {
    cprintf(ch, "You can't make it more powerful than you!\n\r");
    return;
  }
  if (IS_PC(victim)) {
    cprintf(ch, "It would be in bad taste to cast that on a player\n\r");
    return;
  }
  if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
    cprintf(ch, "You can't cast this spell indoors!\n\r");
    return;
  }
  act("$n grows to double $s original size!", FALSE, victim, 0, 0, TO_ROOM);
  act("You grow to double your original size!", FALSE, victim, 0, 0, TO_CHAR);

  af.type = SPELL_VEGGIE_GROWTH;
  af.duration = 2 * level;
  af.modifier = GET_MAX_HIT(victim);
  af.location = APPLY_HIT;
  af.bitvector = AFF_GROWTH;
  affect_to_char(victim, &af);

  af.type = SPELL_VEGGIE_GROWTH;
  af.duration = 2 * level;
  af.modifier = 5;
  af.location = APPLY_HITNDAM;
  af.bitvector = 0;
  affect_to_char(victim, &af);

  af.type = SPELL_VEGGIE_GROWTH;
  af.duration = 2 * level;
  af.modifier = 3;
  af.location = APPLY_SAVE_ALL;
  af.bitvector = 0;
  affect_to_char(victim, &af);

  GET_LEVEL(victim, WARRIOR_LEVEL_IND) = 2 * GetMaxLevel(victim);

}

#define SAPLING  45

void spell_tree(char level, struct char_data *ch, struct char_data *victim,
		struct obj_data *obj)
{
  struct char_data                       *mob = NULL;
  int                                     mobn = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
    cprintf(ch, "You can't cast this spell indoors!\n\r");
    return;
  }
  mobn = SAPLING;
  if (level > 20) {
    mobn++;
  }
  if (level > 30) {
    mobn++;
  }
  if (level > 40) {
    mobn++;
  }
  if (level > 48) {
    mobn++;
  }
  mob = read_mobile(mobn, VIRTUAL);
  if (mob) {
    spell_poly_self(level, ch, mob, 0);
  } else {
    cprintf(ch, "You couldn't summon an image of that creature\n\r");
  }
  return;

}

#define LITTLE_ROCK  50

void spell_animate_rock(char level, struct char_data *ch, struct char_data *victim,
			struct obj_data *obj)
{
  struct char_data                       *mob = NULL;
  struct affected_type                    af;
  int                                     mobn = LITTLE_ROCK;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !obj)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (ITEM_TYPE(obj) != ITEM_ROCK) {
    cprintf(ch, "Thats not the right kind of rock\n\r");
    return;
  }
  /*
   * get the weight of the rock, make the follower based on the weight 
   */

  if (GET_OBJ_WEIGHT(obj) > 20)
    mobn++;
  if (GET_OBJ_WEIGHT(obj) > 40)
    mobn++;
  if (GET_OBJ_WEIGHT(obj) > 80)
    mobn++;
  if (GET_OBJ_WEIGHT(obj) > 160)
    mobn++;
  if (GET_OBJ_WEIGHT(obj) > 320)
    mobn++;

  mob = read_mobile(mobn, VIRTUAL);
  if (mob) {

    char_to_room(mob, ch->in_room);
    /*
     * charm them for a while 
     */
    if (mob->master)
      stop_follower(mob);

    add_follower(mob, ch);

    af.type = SPELL_ANIMATE_ROCK;
    af.duration = 24;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char(mob, &af);

    af.type = SPELL_CHARM_PERSON;

    if (IS_PC(ch) || ch->master) {
      af.duration = 24;
      af.modifier = 0;
      af.location = 0;
      af.bitvector = AFF_CHARM;
      affect_to_char(mob, &af);

    } else {
      SET_BIT(mob->specials.affected_by, AFF_CHARM);
    }

    /*
     * get rid of aggressive, add sentinel
     */

    if (IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
      REMOVE_BIT(mob->specials.act, ACT_AGGRESSIVE);
    }
    if (!IS_SET(mob->specials.act, ACT_SENTINEL)) {
      SET_BIT(mob->specials.act, ACT_SENTINEL);
    }
    extract_obj(obj);

  } else {
    cprintf(ch, "Sorry, the spell isn't working today\n\r");
    return;
  }

}

void spell_travelling(char level, struct char_data *ch, struct char_data *victim,
		      struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (affected_by_spell(victim, SPELL_TRAVELLING))
    return;

  af.type = SPELL_TRAVELLING;
  af.duration = level;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_TRAVELLING;

  affect_to_char(victim, &af);
  act("$n seems fleet of foot", FALSE, victim, 0, 0, TO_ROOM);
  cprintf(victim, "You feel fleet of foot.\n\r");
}

void spell_animal_friendship(char level, struct char_data *ch, struct char_data *victim,
			     struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (affected_by_spell(ch, SPELL_ANIMAL_FRIENDSHIP)) {
    cprintf(ch, "You can only do this 1 time per day\n\r");
    return;
  }
  if (IS_GOOD(victim) || IS_EVIL(victim)) {
    cprintf(ch, "Only neutral mobs allowed\n\r");
    return;
  }
  if (!IsAnimal(victim)) {
    cprintf(ch, "Thats no animal!\n\r");
    return;
  }
  if (GetMaxLevel(ch) < GetMaxLevel(victim)) {
    cprintf(ch, "You do not have enough willpower to charm that yet\n\r");
    return;
  }
  if (GetMaxLevel(victim) > 10) {
    cprintf(ch, "That creature is too powerful to charm\n\r");
    return;
  }
  if (IsImmune(victim, IMM_CHARM)) {
    return;
  }
  if (!saves_spell(victim, SAVING_SPELL))
    return;

  if (victim->master)
    stop_follower(victim);

  add_follower(victim, ch);

  af.type = SPELL_ANIMAL_FRIENDSHIP;
  af.duration = 24;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = 0;
  affect_to_char(ch, &af);

  af.type = SPELL_ANIMAL_FRIENDSHIP;
  af.duration = 24;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_CHARM;
  affect_to_char(victim, &af);

  /*
   * get rid of aggressive, add sentinel
   */
  REMOVE_BIT(victim->specials.act, ACT_AGGRESSIVE);

  SET_BIT(victim->specials.act, ACT_SENTINEL);

}

void spell_invis_to_animals(char level, struct char_data *ch, struct char_data *victim,
			    struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (!affected_by_spell(victim, SPELL_INVIS_TO_ANIMALS)) {

    act("$n seems to fade slightly.", TRUE, victim, 0, 0, TO_ROOM);
    cprintf(victim, "You vanish, sort of.\n\r");

    af.type = SPELL_INVIS_TO_ANIMALS;
    af.duration = 24;
    af.modifier = 0;
    af.location = APPLY_BV2;
    af.bitvector = AFF2_ANIMAL_INVIS;
    affect_to_char(victim, &af);
  }
}

void spell_slow_poison(char level, struct char_data *ch, struct char_data *victim,
		       struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (affected_by_spell(victim, SPELL_POISON)) {

    act("$n seems to fade slightly.", TRUE, victim, 0, 0, TO_ROOM);
    cprintf(victim, "You feel a bit better!.\n\r");

    af.type = SPELL_SLOW_POISON;
    af.duration = 24;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = 0;
    affect_to_char(victim, &af);
  }
}

void spell_snare(char level, struct char_data *ch, struct char_data *victim,
		 struct obj_data *obj)
{
  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
    cprintf(ch, "You can't cast this spell indoors!\n\r");
    return;
  }
  if (real_roomp(ch->in_room)->sector_type != SECT_FOREST) {
    cprintf(ch, "You must cast this spell in the forest!\n\r");
    return;
  }
  /*
   * if victim fails save, movement = 0 
   */
  if (!saves_spell(victim, SAVING_SPELL)) {
    act("Roots and vines entangle your feet!", FALSE, victim, 0, 0, TO_CHAR);
    act("Roots and vines entangle $n's feet!", FALSE, victim, 0, 0, TO_ROOM);
    GET_MOVE(victim) = 0;
  } else {
    FailSnare(ch, victim);
  }
}

void spell_entangle(char level, struct char_data *ch, struct char_data *victim,
		    struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
    cprintf(ch, "You can't cast this spell indoors!\n\r");
    return;
  }
  if (real_roomp(ch->in_room)->sector_type != SECT_FOREST) {
    cprintf(ch, "You must cast this spell in the forest!\n\r");
    return;
  }
  /*
   * if victim fails save, paralyzed for a very short time 
   */
  if (!saves_spell(victim, SAVING_SPELL)) {
    act("Roots and vines entwine around you!", FALSE, victim, 0, 0, TO_CHAR);
    act("Roots and vines surround $n!", FALSE, victim, 0, 0, TO_ROOM);

    af.type = SPELL_ENTANGLE;
    af.duration = 1;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = AFF_PARALYSIS;
    affect_to_char(victim, &af);

  } else {
    FailSnare(ch, victim);
  }
}

void spell_barkskin(char level, struct char_data *ch, struct char_data *victim,
		    struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (!affected_by_spell(victim, SPELL_BARKSKIN) && !affected_by_spell(victim, SPELL_ARMOR)) {

    af.type = SPELL_BARKSKIN;
    af.duration = 24;
    af.modifier = -10;
    af.location = APPLY_AC;
    af.bitvector = 0;
    affect_to_char(victim, &af);

    af.type = SPELL_BARKSKIN;
    af.duration = 24;
    af.modifier = -1;
    af.location = APPLY_SAVE_ALL;
    af.bitvector = 0;
    affect_to_char(victim, &af);

    af.type = SPELL_BARKSKIN;
    af.duration = 24;
    af.modifier = 1;
    af.location = APPLY_SAVING_SPELL;
    af.bitvector = 0;
    affect_to_char(victim, &af);

    cprintf(victim, "Your skin takes on a rough, bark-like texture.\n\r");
    act("$n's skin takes on a rough, bark-like texture", FALSE, ch, 0, 0, TO_ROOM);

  } else {
    cprintf(ch, "Nothing new seems to happen\n\r");
  }
}

void spell_gust_of_wind(char level, struct char_data *ch, struct char_data *victim,
			struct obj_data *obj)
{
  struct char_data                       *tmp_victim = NULL;
  struct char_data                       *temp = NULL;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  cprintf(ch, "You wave your hands, and a gust of wind boils forth!\n\r");
  act("$n sends a gust of wind towards you!\n\r", FALSE, ch, 0, 0, TO_ROOM);

  for (tmp_victim = real_roomp(ch->in_room)->people; tmp_victim; tmp_victim = temp) {
    temp = tmp_victim->next_in_room;
    if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
      if (IS_PC(tmp_victim) && IS_IMMORTAL(tmp_victim) && !IS_IMMORTAL(ch))
	return;
      if (!in_group(ch, tmp_victim)) {
	if (saves_spell(tmp_victim, SAVING_SPELL))
	  continue;
	GET_POS(tmp_victim) = POSITION_SITTING;
      } else {
	act("You are able to avoid the swirling gust\n\r", FALSE, ch, 0, tmp_victim, TO_VICT);
      }
    }
  }
}

void spell_silence(char level, struct char_data *ch, struct char_data *victim,
		   struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (!affected_by_spell(victim, SPELL_SILENCE)) {
    if (!saves_spell(victim, SAVING_SPELL)) {
      act("$n ceases to make noise!", TRUE, victim, 0, 0, TO_ROOM);
      cprintf(victim, "You can't hear anything!.\n\r");

      af.type = SPELL_SILENCE;
      af.duration = level;
      af.modifier = 0;
      af.location = 0;
      af.bitvector = AFF_SILENCE;
      affect_to_char(victim, &af);
    } else {
      cprintf(victim, "You feel quiet for a moment, but the effect fades\n\r");
      if (!IS_PC(victim)) {
	if ((!victim->specials.fighting)) {
	  set_fighting(victim, ch);
	  if (mob_index[victim->nr].func) {
	    (*mob_index[victim->nr].func) (victim, 0, "");
	  }
	}
      }
    }
  }
}

void spell_warp_weapon(char level, struct char_data *ch, struct char_data *victim,
		       struct obj_data *obj)
{

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!(ch && (victim || obj)))
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (!obj) {
    if (!victim->equipment[WIELD]) {
      act("$N doesn't have a weapon wielded!", FALSE, ch, 0, victim, TO_CHAR);
      return;
    }
    obj = victim->equipment[WIELD];
  }
  act("$p is warped and twisted by the power of the spell", FALSE, ch, obj, 0, TO_CHAR);
  act("$p is warped and twisted by the power of the spell", FALSE, ch, obj, 0, TO_ROOM);
  DamageOneItem(victim, BLOW_DAMAGE, obj);

  if (!IS_PC(victim))
    if (!victim->specials.fighting)
      set_fighting(victim, ch);
}

void spell_heat_stuff(char level, struct char_data *ch, struct char_data *victim,
		      struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (affected_by_spell(victim, SPELL_HEAT_STUFF)) {
    cprintf(victim, "Already affected\n\r");
    return;
  }
  if (HitOrMiss(ch, victim, CalcThaco(ch))) {
    af.type = SPELL_HEAT_STUFF;
    af.duration = level;
    af.modifier = -2;
    af.location = APPLY_DEX;
    af.bitvector = 0;
    affect_to_char(victim, &af);

    af.type = SPELL_HEAT_STUFF;
    af.duration = level;
    af.modifier = 0;
    af.location = APPLY_BV2;
    af.bitvector = AFF2_HEAT_STUFF;

    affect_to_char(victim, &af);
    cprintf(victim, "Your armor starts to sizzle and smoke\n\r");
    act("$N's armor starts to sizzle\n\r", FALSE, ch, 0, victim, TO_CHAR);
    act("$N's armor starts to sizzle\n\r", FALSE, ch, 0, victim, TO_NOTVICT);

    if (!IS_PC(victim))
      if (!victim->specials.fighting)
	set_fighting(victim, ch);
  }
}

#define DUST_DEVIL 60

void spell_dust_devil(char level, struct char_data *ch, struct char_data *victim,
		      struct obj_data *obj)
{
  int                                     vnum = 0;
  struct char_data                       *mob = NULL;
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (affected_by_spell(ch, SPELL_DUST_DEVIL)) {
    cprintf(ch, "You can only do this once per 24 hours\n\r");
    return;
  }
  if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
    cprintf(ch, "You can't cast this spell indoors!\n\r");
    return;
  }
  vnum = DUST_DEVIL;

  mob = read_mobile(vnum, VIRTUAL);

  if (!mob) {
    cprintf(ch, "None available\n\r");
    return;
  }
  act("$n performs a complicated ritual!", TRUE, ch, 0, 0, TO_ROOM);
  act("You perform the ritual of summoning", TRUE, ch, 0, 0, TO_CHAR);

  char_to_room(mob, ch->in_room);
  act("$n appears through a momentary rift in the ether!", FALSE, mob, 0, 0, TO_ROOM);

  /*
   * charm them for a while 
   */
  if (mob->master)
    stop_follower(mob);

  add_follower(mob, ch);

  af.type = SPELL_CHARM_PERSON;

  if (IS_PC(ch) || ch->master) {
    af.duration = 24;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char(mob, &af);

  } else {
    SET_BIT(mob->specials.affected_by, AFF_CHARM);
  }

  af.type = SPELL_DUST_DEVIL;
  af.duration = 24;
  af.modifier = 0;
  af.location = 0;
  af.bitvector = 0;
  affect_to_char(ch, &af);

/*
 * adjust the bits...
 */

/*
 * get rid of aggressive, add sentinel
 */

  if (IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
    REMOVE_BIT(mob->specials.act, ACT_AGGRESSIVE);
  }
  if (!IS_SET(mob->specials.act, ACT_SENTINEL)) {
    SET_BIT(mob->specials.act, ACT_SENTINEL);
  }
}

void spell_sunray(char level, struct char_data *ch, struct char_data *victim,
		  struct obj_data *obj)
{
  struct char_data                       *t = NULL;
  struct char_data                       *n = NULL;
  int                                     dam = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  /*
   * blind all in room
   */
  for (t = real_roomp(ch->in_room)->people; t; t = n) {
    n = t->next_in_room;
    if (!in_group(ch, t) && (!IS_IMMORTAL(t) || IS_IMMORTAL(ch))) {
      spell_blindness(level, ch, t, obj);
      /*
       * hit undead target
       */
      if (t == victim) {
	if (GET_RACE(victim) == RACE_UNDEAD || GET_RACE(victim) == RACE_VEGMAN) {
	  dam = dice(6, 8);
	  if (saves_spell(victim, SAVING_SPELL) && (GET_RACE(victim) != RACE_VEGMAN))
	    dam >>= 1;
	  damage(ch, victim, dam, SPELL_SUNRAY);
	}
      } else {
	/*
	 * damage other undead in room
	 */
	if (GET_RACE(t) == RACE_UNDEAD || GET_RACE(t) == RACE_VEGMAN) {
	  dam = dice(3, 6);
	  if (saves_spell(t, SAVING_SPELL) && (GET_RACE(t) != RACE_VEGMAN))
	    dam = 0;
	  damage(ch, t, dam, SPELL_SUNRAY);
	}
      }
    }
  }
}

void spell_know_monster(char level, struct char_data *ch, struct char_data *victim,
			struct obj_data *obj)
{
  char                                    buf[256] = "\0\0\0";
  char                                    buf2[256] = "\0\0\0";
  int                                     exp = 0;
  int                                     lev = 0;
  int                                     hits = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

/*
 * depending on level, give info.. sometimes inaccurate
 */

  if (!IS_PC(victim)) {
    act("$N belongs to the %s race.", FALSE, ch, 0, victim, TO_CHAR, RaceName[GET_RACE(victim)]);
    if (level > 5) {
      exp = GetApprox(GET_EXP(victim), 40 + level);
      act("$N is worth approximately %d experience", FALSE, ch, 0, victim, TO_CHAR, exp);
    }
    if (level > 10) {
      lev = GetApprox(GetMaxLevel(victim), 40 + level);
      act("$N fights like a %d level warrior, you think", FALSE, ch, 0, victim, TO_CHAR, lev);
    }
    if (level > 15) {
      if (IS_SET(victim->hatefield, HATE_RACE)) {
	act("$n seems to hate the %s race", FALSE, ch, 0, victim, TO_CHAR, RaceName[victim->hates.race]);
      }
      if (IS_SET(victim->hatefield, HATE_CLASS)) {
	sprintbit((unsigned)victim->hates.class, pc_class_types, buf2);
	act("$n seems to hate the %s class(es)", FALSE, ch, 0, victim, TO_CHAR, buf2);
      }
    }
    if (level > 20) {
      hits = GetApprox(GET_MAX_HIT(victim), 40 + level);
      act("$N probably has about %d hit points", FALSE, ch, 0, victim, TO_CHAR, hits);
    }
    if (level > 25) {
      if (victim->susc) {
	sprintbit(victim->susc, immunity_names, buf2);
	act("$N is susceptible to %s\n\r", FALSE, ch, 0, victim, TO_CHAR, buf2);
      }
    }
    if (level > 30) {
      if (victim->immune) {
	sprintbit(victim->immune, immunity_names, buf2);
	act("$N is resistant to %s\n\r", FALSE, ch, 0, victim, TO_CHAR, buf2);
      }
    }
    if (level > 35) {
      if (victim->M_immune) {
	sprintbit(victim->M_immune, immunity_names, buf2);
	act("$N is immune to %s\n\r", FALSE, ch, 0, victim, TO_CHAR, buf2);
      }
    }
    if (level > 40) {
      int                                     att;

      att = GetApprox((int)victim->mult_att, 30 + level);
      act("$N gets approx %d.0 attack(s) per round", FALSE, ch, 0, victim, TO_CHAR, att);
    }
    if (level > 45) {
      int                                     no,
                                              s;

      no = GetApprox(victim->specials.damnodice, 30 + level);
      s = GetApprox(victim->specials.damsizedice, 30 + level);

      act("Each does about %dd%d points of damage", FALSE, ch, 0, victim, TO_CHAR, no, s);
    }
  } else {
    cprintf(ch, "Thats not a REAL monster\n\r");
    return;
  }

}

void spell_find_traps(char level, struct char_data *ch, struct char_data *victim,
		      struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

/*
 * raise their detect traps skill
 */
  af.type = SPELL_FIND_TRAPS;
  af.duration = level;
  af.modifier = 50 + level;
  af.location = APPLY_FIND_TRAPS;
  af.bitvector = 0;
  affect_to_char(ch, &af);

}

void spell_firestorm(char level, struct char_data *ch, struct char_data *victim,
		     struct obj_data *obj)
{
/*
 * a-e -    2d8+level
 */
  int                                     dam = 0;
  struct char_data                       *tmp_victim = NULL;
  struct char_data                       *temp = NULL;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  dam = dice(2, 8) + level + 1;

  cprintf(ch, "Searing flame surround you!\n\r");
  act("$n sends a firestorm whirling across the room!\n\r", FALSE, ch, 0, 0, TO_ROOM);

  for (tmp_victim = real_roomp(ch->in_room)->people; tmp_victim; tmp_victim = temp) {
    temp = tmp_victim->next_in_room;
    if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
      if (IS_PC(tmp_victim) && IS_IMMORTAL(tmp_victim) && !IS_IMMORTAL(ch))
	return;
      if (!in_group(ch, tmp_victim)) {
	act("You are seared by the burning flame!\n\r", FALSE, ch, 0, tmp_victim, TO_VICT);
	if (saves_spell(tmp_victim, SAVING_SPELL))
	  dam >>= 1;
	MissileDamage(ch, tmp_victim, dam, SPELL_BURNING_HANDS);
      } else {
	act("You are able to avoid the flames!\n\r", FALSE, ch, 0, tmp_victim, TO_VICT);
      }
    }
  }
}

void spell_teleport_wo_error(char level, struct char_data *ch, struct char_data *victim,
			     struct obj_data *obj)
{
  int                                     location = 0;
  struct room_data                       *rp = NULL;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch || !victim)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

/* replaces the current functionality of astral walk */

  location = victim->in_room;
  rp = real_roomp(location);

  if (GetMaxLevel(victim) > MAX_MORT ||
      !rp ||
      IS_SET(rp->room_flags, PRIVATE) ||
      IS_SET(rp->room_flags, NO_SUM) ||
      IS_SET(rp->room_flags, NO_MAGIC) ||
      (IS_SET(rp->room_flags, TUNNEL) && (MobCountInRoom(rp->people) > rp->moblim))) {
    cprintf(ch, "You failed.\n\r");
    return;
  }
  if (!IsOnPmp(location)) {
    cprintf(ch, "That place is on an extra-dimensional plane!\n");
    return;
  }
  if (!IsOnPmp(ch->in_room)) {
    cprintf(ch, "You're on an extra-dimensional plane!\n\r");
    return;
  }
  if (dice(1, 20) == 20) {
    cprintf(ch, "You fail the magic, and spin out of control!\n\r");
    spell_teleport(level, ch, ch, 0);
    return;
  } else {
    act("$n opens a door to another dimension and steps through!", FALSE, ch, 0, 0, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, location);
    act("You are blinded for a moment as $n appears in a flash of light!",
	FALSE, ch, 0, 0, TO_ROOM);
    do_look(ch, "", 15);
    check_falling(ch);

    if (IS_SET(real_roomp(ch->in_room)->room_flags, DEATH) && GetMaxLevel(ch) < LOW_IMMORTAL) {
      NailThisSucker(ch);
      return;
    }
  }

}

#define PORTAL 31

void spell_portal(char level, struct char_data *ch, struct char_data *tmp_ch,
		  struct obj_data *obj)
{
  /*
   * create a magic portal 
   */
  struct obj_data                        *tmp_obj = NULL;
  struct extra_descr_data                *ed = NULL;
  struct room_data                       *rp = NULL;
  char                                    buf[512] = "\0\0\0";

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  rp = real_roomp(tmp_ch->in_room);
  tmp_obj = read_object(PORTAL, VIRTUAL);
  if (!rp || !tmp_obj) {
    cprintf(ch, "The magic fails\n\r");
    return;
  }
  sprintf(buf, "Through the mists of the portal, you can faintly see %s", rp->name);

  CREATE(ed, struct extra_descr_data, 1);
  ed->next = tmp_obj->ex_description;
  tmp_obj->ex_description = ed;
  CREATE(ed->keyword, char, strlen        (tmp_obj->name) + 1);

  strcpy(ed->keyword, tmp_obj->name);
  ed->description = strdup(buf);

  tmp_obj->obj_flags.value[0] = level / 5;
  tmp_obj->obj_flags.value[1] = tmp_ch->in_room;

  obj_to_room(tmp_obj, ch->in_room);

  act("$p suddenly appears.", TRUE, ch, tmp_obj, 0, TO_ROOM);
  act("$p suddenly appears.", TRUE, ch, tmp_obj, 0, TO_CHAR);

}

#define MOUNT_ONE 65
#define MOUNT_GOOD 69
#define MOUNT_EVIL 70
#define MOUNT_NEUT 71

void spell_mount(char level, struct char_data *ch, struct char_data *victim,
		 struct obj_data *obj)
{
  struct char_data                       *m = NULL;
  int                                     mnr = 0;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

/* create a ridable mount, and automatically mount said creature */

  mnr = MOUNT_ONE;
  if (level < 30) {
    if (level < 12)
      mnr++;
    if (level < 18)
      mnr++;
    if (level < 24)
      mnr++;
  } else {
    if (IS_EVIL(ch)) {
      mnr = MOUNT_EVIL;
    } else if (IS_GOOD(ch)) {
      mnr = MOUNT_GOOD;
    } else {
      mnr = MOUNT_NEUT;
    }
  }

  m = read_mobile(mnr, VIRTUAL);
  if (m) {
    char_to_room(m, ch->in_room);
    act("In a flash of light, $N appears", FALSE, ch, 0, m, TO_CHAR);
    act("In a flash of light, $N appears, and $n hops on $s back", FALSE, ch, 0, m, TO_ROOM);
    cprintf(ch, "You hop on your mount's back\n\r");
    MOUNTED(ch) = m;
    RIDDEN(m) = ch;
    GET_POS(ch) = POSITION_MOUNTED;
  } else {
    cprintf(ch, "horses aren't in database\n\r");
    return;
  }
}

void spell_dragon_ride(char level, struct char_data *ch, struct char_data *victim,
		       struct obj_data *obj)
{
  struct affected_type                    af;

  if (DEBUG > 1)
    log_info("called %s with %d, %s, %s, %s", __PRETTY_FUNCTION__, level, SAFE_NAME(ch), SAFE_NAME(victim), SAFE_ONAME(obj));

  if (!ch)
    return;
  if (!((level >= 1) && (level <= ABS_MAX_LVL)))
    return;

  if (affected_by_spell(ch, SPELL_DRAGON_RIDE)) {
    cprintf(ch, "Already affected\n\r");
    return;
  }
  af.type = SPELL_DRAGON_RIDE;
  af.duration = level;
  af.modifier = 0;
  af.location = 0;
  af.bitvector = AFF_DRAGON_RIDE;
  affect_to_char(ch, &af);
}

#endif							       /* DRUID_WORKING */

/*
 * file: act.other.c , Implementation of commands.        Part of DIKUMUD
 * Usage : Other commands.
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
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
#include "constants.h"
#include "spell_parser.h"
#include "reception.h"
#include "multiclass.h"
#include "fight.h"
#include "spec_procs.h"
#include "act_skills.h"
#include "act_comm.h"
#include "modify.h"
#define _ACT_OTHER_C
#include "act_other.h"

void do_gain(struct char_data *ch, char *argument, int cmd)
{
  if (DEBUG)
    dlog("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

  return;
}

void do_guard(struct char_data *ch, char *argument, int cmd)
{
  if (DEBUG)
    dlog("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

  if (!IS_NPC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) {
    cprintf(ch, "Sorry. you can't just put your brain on autopilot!\n\r");
    return;
  }
  if (IS_SET(ch->specials.act, ACT_GUARDIAN)) {
    act("$n relaxes.", FALSE, ch, 0, 0, TO_ROOM);
    cprintf(ch, "You relax.\n\r");
    REMOVE_BIT(ch->specials.act, ACT_GUARDIAN);
  } else {
    SET_BIT(ch->specials.act, ACT_GUARDIAN);
    act("$n alertly watches you.", FALSE, ch, 0, 0, TO_ROOM);
    cprintf(ch, "You snap to attention\n\r");
  }
  return;
}

void do_junk(struct char_data *ch, char *argument, int cmd)
{
  char                                    arg[100] = "\0\0\0";
  char                                    newarg[100] = "\0\0\0";
  struct obj_data                        *tmp_object = NULL;
  struct obj_data                        *old_object = NULL;
  int                                     num = 0;
  int                                     p = 0;
  int                                     count = 0;

  if (DEBUG)
    dlog("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);
/*
 *   get object name & verify
 */

  only_argument(argument, arg);
  if (*arg) {
    if (getall(arg, newarg) != FALSE) {
      num = -1;
      strcpy(arg, newarg);
    } else if ((p = getabunch(arg, newarg)) != FALSE) {
      num = p;
      strcpy(arg, newarg);
    } else {
      num = 1;
    }

    count = 0;
    while (num != 0) {
      tmp_object = get_obj_in_list_vis(ch, arg, ch->carrying);
      if (tmp_object) {
	old_object = tmp_object;
	obj_from_char(tmp_object);
	extract_obj(tmp_object);
	if (num > 0)
	  num--;
	count++;
      } else {
	if (count > 1) {
	  act("You junk %s (%d).\n\r", 1, ch, 0, 0, TO_CHAR, arg, count);
	  act("$n junks %s.\n\r", 1, ch, 0, 0, TO_ROOM, arg);
	} else if (count == 1) {
	  act("You junk %s \n\r", 1, ch, 0, 0, TO_CHAR, arg);
	  act("$n junks %s.\n\r", 1, ch, 0, 0, TO_ROOM, arg);
	} else {
	  cprintf(ch, "You don't have anything like that\n\r");
	}
	return;
      }
    }
  } else {
    cprintf(ch, "Junk what?");
    return;
  }
}

void do_qui(struct char_data *ch, char *argument, int cmd)
{
  if (DEBUG)
    dlog("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

  cprintf(ch, "You have to write quit - no less, to quit!\n\r");
  return;
}

void do_title(struct char_data *ch, char *argument, int cmd)
{
  if (DEBUG)
    dlog("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

  if (IS_NPC(ch) || !ch->desc)
    return;

  for (; isspace(*argument); argument++);

  if (*argument) {
    cprintf(ch, "Your title has been set to : <%s>\n\r", argument);
    ch->player.title = strdup(argument);
  }
}

void do_quit(struct char_data *ch, char *argument, int cmd)
{
  if (DEBUG)
    dlog("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

  if (IS_NPC(ch) || !ch->desc)
    return;

  if (GET_POS(ch) == POSITION_FIGHTING) {
    cprintf(ch, "No way! You are fighting.\n\r");
    return;
  }
  if (GET_POS(ch) < POSITION_STUNNED) {
    cprintf(ch, "You die before your time!\n\r");
    die(ch);
    return;
  }
  if (MOUNTED(ch)) {
    Dismount(ch, MOUNTED(ch), POSITION_STANDING);
  }
  act("Goodbye, friend.. Come back soon!", FALSE, ch, 0, 0, TO_CHAR);
  act("$n has left the game.", TRUE, ch, 0, 0, TO_ROOM);
  GET_HOME(ch) = 3008;
  zero_rent(ch);
  extract_char(ch);					       /* Char is saved in extract char */
}

void do_save(struct char_data *ch, char *argument, int cmd)
{
  struct obj_cost                         cost;

  if (DEBUG)
    dlog("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

  if (IS_NPC(ch))
    return;

  cprintf(ch, "Saving %s.\n\r", GET_NAME(ch));
  recep_offer(ch, NULL, &cost);
  new_save_equipment(ch, &cost, FALSE);
  save_obj(ch, &cost, FALSE);
  save_char(ch, NOWHERE);
}

void do_not_here(struct char_data *ch, char *argument, int cmd)
{
  if (DEBUG)
    dlog("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

  cprintf(ch, "Sorry, but you cannot do that here!\n\r");
}

void do_sneak(struct char_data *ch, char *argument, int cmd)
{
  int                                     percent_chance = 0;
  struct affected_type                    af;

  if (DEBUG)
    dlog("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

  if (IS_AFFECTED(ch, AFF_SNEAK)) {
    affect_from_char(ch, SKILL_SNEAK);
    if (IS_AFFECTED(ch, AFF_HIDE))
      REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);
    cprintf(ch, "You are no longer sneaky.\n\r");
    return;
  }
  cprintf(ch, "Ok, you'll try to move silently for a while.\n\r");
  cprintf(ch, "And you attempt to hide yourself.\n\r");

  percent_chance = number(1, 101);			       /* 101% is a complete failure */

  if (percent_chance > ch->skills[SKILL_SNEAK].learned + dex_app_skill[(int)GET_DEX(ch)].sneak)
    return;

  if (ch->skills[SKILL_SNEAK].learned < 50)
    ch->skills[SKILL_SNEAK].learned += 2;

  SET_BIT(ch->specials.affected_by, AFF_HIDE);
  af.type = SKILL_SNEAK;
  af.duration = GET_LEVEL(ch, BestThiefClass(ch));
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_SNEAK;
  affect_to_char(ch, &af);
}

void do_hide(struct char_data *ch, char *argument, int cmd)
{
  int                                     percent_chance = 0;
  struct affected_type                    af;

  if (DEBUG)
    dlog("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

  cprintf(ch, "Ok, you crawl into a dark corner and lurk.\n\r");

  if (IS_AFFECTED(ch, AFF_SNEAK))
    affect_from_char(ch, SKILL_SNEAK);
  if (IS_AFFECTED(ch, AFF_HIDE))
    REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);

  percent_chance = number(1, 101);			       /* 101% is a complete failure */

  if (percent_chance > ch->skills[SKILL_SNEAK].learned + dex_app_skill[(int)GET_DEX(ch)].sneak)
    return;
  if (ch->skills[SKILL_SNEAK].learned < 50)
    ch->skills[SKILL_SNEAK].learned += 2;

  SET_BIT(ch->specials.affected_by, AFF_HIDE);
  af.type = SKILL_SNEAK;
  af.duration = GET_LEVEL(ch, BestThiefClass(ch));
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_SNEAK;
  affect_to_char(ch, &af);
}

void do_steal(struct char_data *ch, char *argument, int cmd)
{
  struct char_data                       *victim = NULL;
  struct obj_data                        *obj = NULL;
  char                                    victim_name[240] = "\0\0\0";
  char                                    obj_name[240] = "\0\0\0";
  char                                    buf[240] = "\0\0\0";
  int                                     percent_chance = 0;
  int                                     gold = 0;
  int                                     eq_pos = 0;
  char                                    ohoh = FALSE;
  struct room_data                       *rp = NULL;

  if (DEBUG)
    dlog("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

  if (check_peaceful(ch, "What if he caught you?\n\r"))
    return;

  argument = one_argument(argument, obj_name);
  only_argument(argument, victim_name);

  if (!(victim = get_char_room_vis(ch, victim_name))) {
    cprintf(ch, "Steal what from who?\n\r");
    return;
  } else if (victim == ch) {
    cprintf(ch, "Come on now, that's rather stupid!\n\r");
    return;
  }
  if (!CheckKill(ch, victim))
    return;

  if ((!victim->desc) && (IS_PC(victim)))
    return;

  /*
   * 101% is a complete failure 
   */
  percent_chance = number(1, 101) - dex_app_skill[(int)GET_DEX(ch)].p_pocket;

  if (GET_POS(victim) < POSITION_SLEEPING)
    percent_chance = -1;				       /* ALWAYS SUCCESS */

  percent_chance += GetTotLevel(victim);
  if (GetMaxLevel(victim) > MAX_MORT)
    percent_chance = 101;				       /* Failure */

  if (str_cmp(obj_name, "coins") && str_cmp(obj_name, "gold")) {
    if (!(obj = get_obj_in_list_vis(victim, obj_name, victim->carrying))) {
      for (eq_pos = 0; (eq_pos < MAX_WEAR); eq_pos++)
	if (victim->equipment[eq_pos] &&
	    (isname(obj_name, victim->equipment[eq_pos]->name)) &&
	    CAN_SEE_OBJ(ch, victim->equipment[eq_pos])) {
	  obj = victim->equipment[eq_pos];
	  break;
	}
      if (!obj) {
	act("$E has not got that item.", FALSE, ch, 0, victim, TO_CHAR);
	return;
      } else {						       /* It is equipment */
	if ((GET_POS(victim) > POSITION_STUNNED)) {
	  cprintf(ch, "Steal the equipment now? Impossible!\n\r");
	  return;
	} else {
	  act("You unequip $p and steal it.", FALSE, ch, obj, 0, TO_CHAR);
	  act("$n steals $p from $N.", FALSE, ch, obj, victim, TO_NOTVICT);
	  obj_to_char(unequip_char(victim, eq_pos), ch);
	}
      }
    } else {						       /* obj found in inventory */
      percent_chance += GET_OBJ_WEIGHT(obj);		       /* Make heavy harder */
      rp = real_roomp(ch->in_room);
      if (IS_SET(rp->room_flags, NO_STEAL)) {
	percent_chance = 101;
      }
      if (AWAKE(victim) && (percent_chance > ch->skills[SKILL_STEAL].learned)) {
	ohoh = TRUE;
	act("Oops..", FALSE, ch, 0, 0, TO_CHAR);
	act("$n tried to steal something from you!", FALSE, ch, 0, victim, TO_VICT);
	act("$n tries to steal something from $N.", TRUE, ch, 0, victim, TO_NOTVICT);
      } else {						       /* Steal the item */
	if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
	  if ((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) < CAN_CARRY_W(ch)) {
	    obj_from_char(obj);
	    obj_to_char(obj, ch);
	    cprintf(ch, "Got it!\n\r");
	    if (ch->skills[SKILL_STEAL].learned < 50)
	      ch->skills[SKILL_STEAL].learned += 1;
	  }
	} else
	  cprintf(ch, "You cannot carry that much.\n\r");
      }
    }
  } else {						       /* Steal some coins */
    if (AWAKE(victim) && (percent_chance > ch->skills[SKILL_STEAL].learned)) {
      ohoh = TRUE;
      act("Oops..", FALSE, ch, 0, 0, TO_CHAR);
      act("You discover that $n has $s hands in your wallet.", FALSE, ch, 0, victim, TO_VICT);
      act("$n tries to steal gold from $N.", TRUE, ch, 0, victim, TO_NOTVICT);
    } else {
      /*
       * Steal some gold coins 
       */
      gold = (int)((GET_GOLD(victim) * number(1, 10)) / 100);
      gold = MIN(100, gold);
      if (gold > 0) {
	GET_GOLD(ch) += gold;
	GET_GOLD(victim) -= gold;
	cprintf(ch, "Bingo! You got %d gold coins.\n\r", gold);
      } else {
	cprintf(ch, "You couldn't get any gold...\n\r");
      }
    }
  }

  if (ohoh && IS_NPC(victim) && AWAKE(victim)) {
    if (IS_SET(victim->specials.act, ACT_NICE_THIEF)) {
      sprintf(buf, "%s is a damn thief!", GET_NAME(ch));
      do_shout(victim, buf, 0);
      dlog(buf);
      cprintf(ch, "Don't you ever do that again!\n\r");
    } else {
      hit(victim, ch, TYPE_UNDEFINED);
    }
  }
}

void do_practice(struct char_data *ch, char *argument, int cmd)
{
  char                                    buf[MAX_STRING_LENGTH] = "\0\0\0";
  int                                     i = 0;
  char                                    first_arg[100] = "\0\0\0";
  char                                    sec_arg[100] = "\0\0\0";
  int                                     flag = FALSE;

  struct skill_struct {
    char                                    skill_name[40];
    int                                     skill_numb;
  };

  if (DEBUG)
    dlog("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

  if (cmd != CMD_practice)
    return;

  for (; isspace(*argument); argument++);
  half_chop(argument, first_arg, sec_arg);

  if (!first_arg) {
    cprintf(ch, "You need to supply a class for that.");
    return;
  }
  switch (*first_arg) {
    case 'w':
    case 'W':
    case 'f':
    case 'F':
      {
	if (!HasClass(ch, CLASS_WARRIOR)) {
	  cprintf(ch, "I bet you wish you were a warrior.\n\r");
	  return;
	}
	sprintf(buf, "You can practise any of these MANLY skills:\n\r");
	for (i = 0; i < MAX_SKILLS; i++) {
	  if (CanUseClass(ch, i, WARRIOR_LEVEL_IND))
	    sprintf(buf + strlen(buf), "%s%s\n\r", spell_info[i].name,
		    how_good(ch->skills[i].learned));
	}
	page_string(ch->desc, buf, 1);
	return;
      }
      break;
    case 'r':
    case 'R':
      {
	if (!HasClass(ch, CLASS_RANGER)) {
	  cprintf(ch, "I wish I was a Ranger too!\n\r");
	  return;
	}
	sprintf(buf, "You can practise any of these outdoor skills:\n\r");
	for (i = 0; i < MAX_SKILLS; i++) {
	  if (CanUseClass(ch, i, RANGER_LEVEL_IND))
	    sprintf(buf + strlen(buf), "%s%s\n\r", spell_info[i].name,
		    how_good(ch->skills[i].learned));
	}
	sprintf(buf + strlen(buf), "Or these leaf-and-twig charms:\n\r");
	for (i = 0; i < MAX_SKILLS; i++) {
	  if (CanCastClass(ch, i, RANGER_LEVEL_IND))
	    sprintf(buf + strlen(buf), "%s%s\n\r", spell_info[i].name,
		    how_good(ch->skills[i].learned));
	}
	page_string(ch->desc, buf, 1);
	return;
      }
      break;
    case 't':
    case 'T':
      {
	if (!HasClass(ch, CLASS_THIEF)) {
	  cprintf(ch, "A voice whispers, 'You are not a thief.'\n\r");
	  return;
	}
	sprintf(buf, "You can practice any of these sneaky skills:\n\r");
	for (i = 0; i < MAX_SKILLS; i++) {
	  if (CanUseClass(ch, i, THIEF_LEVEL_IND))
	    sprintf(buf + strlen(buf), "%s%s\n\r", spell_info[i].name,
		    how_good(ch->skills[i].learned));
	}
	page_string(ch->desc, buf, 1);
	return;
      }
      break;
    case 'M':
    case 'm':
      {
	if (!HasClass(ch, CLASS_MAGIC_USER)) {
	  cprintf(ch, "You pretend to be a magic-user, 'Gooble-dah!'\n\r");
	  return;
	}
	flag = 0;
	if (is_abbrev(sec_arg, "known"))
	  flag = 1;
	sprintf(buf, "Your heavy spellbook contains these spells:\n\r");
	for (i = 0; i < MAX_SKILLS; i++)
	  if (CanCastClass(ch, i, MAGE_LEVEL_IND)) {
	    if (!flag) {
	      sprintf(buf + strlen(buf), "[%2d] %s%s\n\r",
		      spell_info[i].min_level[MAGE_LEVEL_IND],
		      spell_info[i].name, how_good(ch->skills[i].learned));
	    } else {
	      if (ch->skills[i].learned > 0) {
		sprintf(buf + strlen(buf), "[%2d] %s%s\n\r",
			spell_info[i].min_level[MAGE_LEVEL_IND],
			spell_info[i].name, how_good(ch->skills[i].learned));
	      }
	    }
	  }
	page_string(ch->desc, buf, 1);
	return;
      }
      break;
    case 'C':
    case 'c':
    case 'P':
    case 'p':
      {
	if (!HasClass(ch, CLASS_CLERIC)) {
	  cprintf(ch, "You feel that impersonating a cleric might be bad.\n\r");
	  return;
	}
	sprintf(buf, "You can pray for any of these miracles:\n\r");
	flag = 0;
	if (is_abbrev(sec_arg, "known"))
	  flag = 1;
	for (i = 0; i < MAX_SKILLS; i++)
	  if (CanCastClass(ch, i, CLERIC_LEVEL_IND)) {
	    if (!flag) {
	      sprintf(buf + strlen(buf), "[%2d] %s%s\n\r",
		      spell_info[i].min_level[CLERIC_LEVEL_IND],
		      spell_info[i].name, how_good(ch->skills[i].learned));
	    } else {
	      if (ch->skills[i].learned > 0) {
		sprintf(buf + strlen(buf), "[%2d] %s%s\n\r",
			spell_info[i].min_level[CLERIC_LEVEL_IND],
			spell_info[i].name, how_good(ch->skills[i].learned));
	      }
	    }
	  }
	page_string(ch->desc, buf, 1);
	return;
      }
      break;
    default:
      cprintf(ch, "Ah, but practice which class???\n\r");
  }
}

void do_idea(struct char_data *ch, char *argument, int cmd)
{
  FILE                                   *fl = NULL;
  char                                    str[MAX_INPUT_LENGTH + 20] = "\0\0\0";

  if (DEBUG)
    dlog("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

  if (IS_NPC(ch)) {
    cprintf(ch, "Monsters can't have ideas - Go away.\n\r");
    return;
  }
  /*
   * skip whites 
   */
  for (; isspace(*argument); argument++);

  if (!*argument) {
    cprintf(ch, "That doesn't sound like a good idea to me.. Sorry.\n\r");
    return;
  }
  if (!(fl = fopen(IDEA_FILE, "a"))) {
    perror("do_idea");
    cprintf(ch, "Could not open the idea-file.\n\r");
    return;
  }
  sprintf(str, "**%s: %s\n", GET_NAME(ch), argument);

  fputs(str, fl);
  FCLOSE(fl);
  cprintf(ch, "Woah!  That's pretty cool.  Thanks!\n\r");
}

void do_typo(struct char_data *ch, char *argument, int cmd)
{
  FILE                                   *fl = NULL;
  char                                    str[MAX_INPUT_LENGTH + 20] = "\0\0\0";

  if (DEBUG)
    dlog("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

  if (IS_NPC(ch)) {
    cprintf(ch, "Monsters can't spell - leave me alone.\n\r");
    return;
  }
  /*
   * skip whites 
   */
  for (; isspace(*argument); argument++);

  if (!*argument) {
    cprintf(ch, "I beg your pardon?\n\r");
    return;
  }
  if (!(fl = fopen(TYPO_FILE, "a"))) {
    perror("do_typo");
    cprintf(ch, "Could not open the typo-file.\n\r");
    return;
  }
  sprintf(str, "**%s[%d]: %s\n", GET_NAME(ch), ch->in_room, argument);
  fputs(str, fl);
  FCLOSE(fl);
  cprintf(ch, "No problem.  We can send a whizzling to fix it.\n\r");
}

void do_bug(struct char_data *ch, char *argument, int cmd)
{
  FILE                                   *fl = NULL;
  char                                    str[MAX_INPUT_LENGTH + 20] = "\0\0\0";

  if (DEBUG)
    dlog("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

  if (IS_NPC(ch)) {
    cprintf(ch, "You are a monster! Bug off!\n\r");
    return;
  }
  /*
   * skip whites 
   */
  for (; isspace(*argument); argument++);

  if (!*argument) {
    cprintf(ch, "Pardon?\n\r");
    return;
  }
  if (!(fl = fopen(BUG_FILE, "a"))) {
    perror("do_bug");
    cprintf(ch, "Could not open the bug-file.\n\r");
    return;
  }
  sprintf(str, "**%s[%d]: %s\n", GET_NAME(ch), ch->in_room, argument);
  fputs(str, fl);
  FCLOSE(fl);
  cprintf(ch, "Really?  Ok, we'll send someone to have a look.\n\r");
}

void do_brief(struct char_data *ch, char *argument, int cmd)
{
  if (DEBUG)
    dlog("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

  if (IS_NPC(ch))
    return;

  if (IS_SET(ch->specials.act, PLR_BRIEF)) {
    cprintf(ch, "Brief mode off.\n\r");
    REMOVE_BIT(ch->specials.act, PLR_BRIEF);
  } else {
    cprintf(ch, "Brief mode on.\n\r");
    SET_BIT(ch->specials.act, PLR_BRIEF);
  }
}

void do_compact(struct char_data *ch, char *argument, int cmd)
{
  if (DEBUG)
    dlog("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

  if (IS_NPC(ch))
    return;

  if (IS_SET(ch->specials.act, PLR_COMPACT)) {
    cprintf(ch, "You are now in the uncompacted mode.\n\r");
    REMOVE_BIT(ch->specials.act, PLR_COMPACT);
  } else {
    cprintf(ch, "You are now in compact mode.\n\r");
    SET_BIT(ch->specials.act, PLR_COMPACT);
  }
}

void do_group(struct char_data *ch, char *argument, int cmd)
{
  char                                    name[256] = "\0\0\0";
  struct char_data                       *victim = NULL;
  struct char_data                       *k = NULL;
  struct follow_type                     *f = NULL;
  int                                     found = FALSE;
  int                                     victlvl = 0;
  int                                     chlvl = 0;

  if (DEBUG)
    dlog("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

  only_argument(argument, name);

  if (!*name) {
    if (!IS_AFFECTED(ch, AFF_GROUP)) {
      cprintf(ch, "But you are a member of no group?!\n\r");
    } else {
      cprintf(ch, "Your group consists of:\n\r");
      if (ch->master)
	k = ch->master;
      else
	k = ch;

      if (IS_AFFECTED(k, AFF_GROUP))
	act("     $N (Head of group)", FALSE, ch, 0, k, TO_CHAR);

      for (f = k->followers; f; f = f->next)
	if (IS_AFFECTED(f->follower, AFF_GROUP))
	  act("     $N", FALSE, ch, 0, f->follower, TO_CHAR);
    }
    return;
  }
  if (!(victim = get_char_room_vis(ch, name))) {
    cprintf(ch, "No one here by that name.\n\r");
  } else {
    if (ch->master) {
      act("You can not enroll group members without being head of a group.",
	  FALSE, ch, 0, 0, TO_CHAR);
      return;
    }
    found = FALSE;

    if (victim == ch)
      found = TRUE;
    else {
      for (f = ch->followers; f; f = f->next) {
	if (f->follower == victim) {
	  found = TRUE;
	  break;
	}
      }
    }

    if (found) {
      if (IS_AFFECTED(victim, AFF_GROUP)) {
	if (victim != ch) {
	  act("$n has been kicked out of $N's group!", FALSE, victim, 0, ch, TO_ROOM);
	  act("You are no longer a member of $N's group!", FALSE, victim, 0, ch, TO_CHAR);
	} else {
	  act("You are no longer a group leader!", FALSE, victim, 0, ch, TO_CHAR);
	}
	REMOVE_BIT(victim->specials.affected_by, AFF_GROUP);
      } else {
	if (IS_IMMORTAL(victim) && !IS_IMMORTAL(ch)) {
	  act("You really don't want $n in your group.", FALSE, victim, 0, 0, TO_CHAR);
	  return;
	}
/*
 *	if ((chlvl= GetMaxLevel(ch)) >= LOW_IMMORTAL) {
 *	  act("Now now.  That would be CHEATING!", FALSE, ch, 0, 0, TO_CHAR);
 *	  return;
 *	}
 */
	chlvl = GetMaxLevel(ch);
	victlvl = GetMaxLevel(victim);
	if (IS_PC(victim) && victim != ch) {
	  int                                     toolow,
	                                          toohigh;

	  /*
	   * newbies can mix with each other 
	   */
	  toolow = (chlvl <= 4) ? 0 : MAX(3, chlvl / 2);
	  /*
	   * but newbies and adults don't mix 
	   */
	  toohigh = (chlvl <= 4) ? 5 : (chlvl + chlvl / 2);
	  if (victlvl <= toolow) {
	    act("It would be beneath you to lead the lowly $N into battle!",
		FALSE, ch, 0, victim, TO_CHAR);
	    act("$N is too mighty to pay any heed to the likes of you!",
		FALSE, victim, 0, ch, TO_CHAR);
	    act("$N tells $n to bugger off.", FALSE, victim, 0, ch, TO_ROOM);
	    return;
	  }
	  if (victlvl >= toohigh) {
	    act("You WISH $N would deign to join your group!", FALSE, ch, 0, victim, TO_CHAR);
	    act("$N BEGS you to help $M go kill the chickens.", FALSE, victim, 0, ch, TO_CHAR);
	    act("$N grovels and begs $n to join $S group.", FALSE, victim, 0, ch, TO_ROOM);
	    return;
	  }
	}
	if (victim != ch) {
	  act("$n joins the glorious cause of $N's group.", FALSE, victim, 0, ch, TO_ROOM);
	  act("You are now a member of $N's group.", FALSE, victim, 0, ch, TO_CHAR);
	} else {
	  act("You are now your own group leader.", FALSE, victim, 0, ch, TO_CHAR);
	}
	SET_BIT(victim->specials.affected_by, AFF_GROUP);
      }
    } else {
      act("$N must follow you, to enter the group", FALSE, ch, 0, victim, TO_CHAR);
    }
  }
}

void do_quaff(struct char_data *ch, char *argument, int cmd)
{
  char                                    buf[100] = "\0\0\0";
  struct obj_data                        *temp = NULL;
  int                                     i = 0;
  int                                     equipped = FALSE;

  if (DEBUG)
    dlog("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

  only_argument(argument, buf);

  if (!(temp = get_obj_in_list_vis(ch, buf, ch->carrying))) {
    temp = ch->equipment[HOLD];
    equipped = TRUE;
    if ((temp == 0) || !isname(buf, temp->name)) {
      act("You do not have that item.", FALSE, ch, 0, 0, TO_CHAR);
      return;
    }
  }
  if (!IS_IMMORTAL(ch)) {
    if (GET_COND(ch, FULL) > 20) {
      act("Your stomach can't contain anymore!", FALSE, ch, 0, 0, TO_CHAR);
      return;
    } else {
      GET_COND(ch, FULL) += 1;
    }
  }
  if (temp->obj_flags.type_flag != ITEM_POTION) {
    act("You can only quaff potions.", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  act("$n quaffs $p.", TRUE, ch, temp, 0, TO_ROOM);
  act("You quaff $p which dissolves.", FALSE, ch, temp, 0, TO_CHAR);

  /*
   * my stuff 
   */
  if (ch->specials.fighting) {
    if (equipped) {
      if (number(1, 20) > ch->abilities.dex) {
	act("$n is jolted and drops $p!  It shatters!", TRUE, ch, temp, 0, TO_ROOM);
	act("You arm is jolted and $p flies from your hand, *SMASH*",
	    TRUE, ch, temp, 0, TO_CHAR);
	if (equipped)
	  temp = unequip_char(ch, HOLD);
	extract_obj(temp);
	return;
      }
    } else {
      if (number(1, 20) > ch->abilities.dex - 4) {
	act("$n is jolted and drops $p!  It shatters!", TRUE, ch, temp, 0, TO_ROOM);
	act("You arm is jolted and $p flies from your hand, *SMASH*",
	    TRUE, ch, temp, 0, TO_CHAR);
	extract_obj(temp);
	return;
      }
    }
  }
  for (i = 1; i < 4; i++)
    if (temp->obj_flags.value[i] >= 1)
      ((*spell_info[temp->obj_flags.value[i]].spell_pointer)
       ((char)temp->obj_flags.value[0], ch, "", SPELL_TYPE_POTION, ch, temp));

  if (equipped)
    temp = unequip_char(ch, HOLD);

  extract_obj(temp);

  WAIT_STATE(ch, PULSE_VIOLENCE);

}

void do_recite(struct char_data *ch, char *argument, int cmd)
{
  char                                    buf[100] = "\0\0\0";
  struct obj_data                        *scroll = NULL;
  struct obj_data                        *obj = NULL;
  struct char_data                       *victim = NULL;
  int                                     i = 0;
  int                                     bits = 0;
  int                                     equipped = FALSE;

  if (DEBUG)
    dlog("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

  argument = one_argument(argument, buf);

  if (!(scroll = get_obj_in_list_vis(ch, buf, ch->carrying))) {
    scroll = ch->equipment[HOLD];
    equipped = TRUE;
    if ((scroll == 0) || !isname(buf, scroll->name)) {
      act("You do not have that item.", FALSE, ch, 0, 0, TO_CHAR);
      return;
    }
  }
  if (scroll->obj_flags.type_flag != ITEM_SCROLL) {
    act("Recite is normally used for scrolls.", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  if (*argument) {
    bits = generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM |
			FIND_OBJ_EQUIP | FIND_CHAR_ROOM, ch, &victim, &obj);
    if (bits == 0) {
      cprintf(ch, "No such thing around to recite the scroll on.\n\r");
      return;
    }
  } else {
    victim = ch;
  }

  if (ch->skills[SKILL_READ_MAGIC].learned < number(1, 101)) {
    if (scroll->obj_flags.value[1] != SPELL_WORD_OF_RECALL) {
      cprintf(ch, "You can't understand this...\n\r");
      return;
    }
  }
  act("$n recites $p.", TRUE, ch, scroll, 0, TO_ROOM);
  act("You recite $p which bursts into flame.", FALSE, ch, scroll, 0, TO_CHAR);

  for (i = 1; i < 4; i++)
    if (scroll->obj_flags.value[i] >= 1) {
      if (IS_SET(spell_info[scroll->obj_flags.value[i]].targets, TAR_VIOLENT) &&
	  check_peaceful(ch, "Impolite magic is banned here."))
	continue;
      ((*spell_info[scroll->obj_flags.value[i]].spell_pointer)
       ((char)scroll->obj_flags.value[0], ch, "", SPELL_TYPE_SCROLL, victim, obj));
    }
  if (equipped)
    scroll = unequip_char(ch, HOLD);
  extract_obj(scroll);
}

void do_use(struct char_data *ch, char *argument, int cmd)
{
  char                                    buf[100] = "\0\0\0";
  struct char_data                       *tmp_char = NULL;
  struct obj_data                        *tmp_object = NULL;
  struct obj_data                        *stick = NULL;
  int                                     bits = 0;

  if (DEBUG)
    dlog("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

  argument = one_argument(argument, buf);

  if (ch->equipment[HOLD] == 0 || !isname(buf, ch->equipment[HOLD]->name)) {
    act("You do not hold that item in your hand.", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  stick = ch->equipment[HOLD];

  if (stick->obj_flags.type_flag == ITEM_STAFF) {
    act("$n taps $p three times on the ground.", TRUE, ch, stick, 0, TO_ROOM);
    act("You tap $p three times on the ground.", FALSE, ch, stick, 0, TO_CHAR);
    if (stick->obj_flags.value[2] > 0) {		       /* Is there any charges left? */
      stick->obj_flags.value[2]--;
      ((*spell_info[stick->obj_flags.value[3]].spell_pointer)
       ((char)stick->obj_flags.value[0], ch, "", SPELL_TYPE_STAFF, 0, 0));
      WAIT_STATE(ch, PULSE_VIOLENCE);
    } else {
      cprintf(ch, "The staff seems powerless.\n\r");
    }
  } else if (stick->obj_flags.type_flag == ITEM_WAND) {
    bits = generic_find(argument, FIND_CHAR_ROOM | FIND_OBJ_INV |
			FIND_OBJ_ROOM | FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);
    if (bits) {
      struct spell_info_type                 *spellp;

      spellp = spell_info + (stick->obj_flags.value[3]);

      if (bits == FIND_CHAR_ROOM) {
	act("$n point $p at $N.", TRUE, ch, stick, tmp_char, TO_ROOM);
	act("You point $p at $N.", FALSE, ch, stick, tmp_char, TO_CHAR);
      } else {
	act("$n point $p at $P.", TRUE, ch, stick, tmp_object, TO_ROOM);
	act("You point $p at $P.", FALSE, ch, stick, tmp_object, TO_CHAR);
      }

      if (IS_SET(spellp->targets, TAR_VIOLENT) &&
	  check_peaceful(ch, "Impolite magic is banned here."))
	return;

      if (stick->obj_flags.value[2] > 0) {		       /* Is there any charges left? */
	stick->obj_flags.value[2]--;
	((*spellp->spell_pointer)
	 ((char)stick->obj_flags.value[0], ch, "", SPELL_TYPE_WAND, tmp_char, tmp_object));
	WAIT_STATE(ch, PULSE_VIOLENCE);
      } else {
	cprintf(ch, "The wand seems powerless.\n\r");
      }
    } else {
      cprintf(ch, "What should the wand be pointed at?\n\r");
    }
  } else {
    cprintf(ch, "Use is normally only for wand's and staff's.\n\r");
  }
}

void do_plr_noshout(struct char_data *ch, char *argument, int cmd)
{
  char                                    buf[128] = "\0\0\0";

  if (DEBUG)
    dlog("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

  if (IS_NPC(ch))
    return;

  only_argument(argument, buf);

  if (!*buf) {
    if (IS_SET(ch->specials.act, PLR_DEAF)) {
      cprintf(ch, "You can now hear shouts again.\n\r");
      REMOVE_BIT(ch->specials.act, PLR_DEAF);
    } else {
      cprintf(ch, "From now on, you won't hear shouts.\n\r");
      SET_BIT(ch->specials.act, PLR_DEAF);
    }
  } else {
    cprintf(ch, "Only the gods can shut up someone else. \n\r");
  }
}

void do_plr_notell(struct char_data *ch, char *argument, int cmd)
{
  char                                    buf[128] = "\0\0\0";

  if (DEBUG)
    dlog("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

  if (IS_NPC(ch))
    return;
  only_argument(argument, buf);

  if (IS_SET(ch->specials.new_act, NEW_PLR_NOTELL)) {
    cprintf(ch, "You can now hear tells again.\n\r");
    REMOVE_BIT(ch->specials.new_act, NEW_PLR_NOTELL);
  } else {
    cprintf(ch, "From now on, you won't hear tells.\n\r");
    SET_BIT(ch->specials.new_act, NEW_PLR_NOTELL);
  }
}

void do_plr_nosummon(struct char_data *ch, char *argument, int cmd)
{
  if (DEBUG)
    dlog("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

  if (IS_NPC(ch))
    return;

  if (IS_SET(ch->specials.new_act, NEW_PLR_KILLOK)) {
    cprintf(ch, "Sorry, registered servants of the Dread Lord cannot hide!\n\r");
    return;
  }
  if (IS_SET(ch->specials.new_act, NEW_PLR_SUMMON)) {
    cprintf(ch, "You can now be summoned.\n\r");
    REMOVE_BIT(ch->specials.new_act, NEW_PLR_SUMMON);
  } else {
    cprintf(ch, "From now on, you can't be summoned.\n\r");
    SET_BIT(ch->specials.new_act, NEW_PLR_SUMMON);
  }
}

void do_plr_noteleport(struct char_data *ch, char *argument, int cmd)
{
  if (DEBUG)
    dlog("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

  if (IS_NPC(ch))
    return;

  if (IS_SET(ch->specials.new_act, NEW_PLR_KILLOK)) {
    cprintf(ch, "Sorry, registered servants of the Dread Lord cannot cower!\n\r");
    return;
  }
  if (IS_SET(ch->specials.new_act, NEW_PLR_TELEPORT)) {
    cprintf(ch, "You can now be teleported.\n\r");
    REMOVE_BIT(ch->specials.new_act, NEW_PLR_TELEPORT);
  } else {
    cprintf(ch, "From now on, you can't be teleported.\n\r");
    SET_BIT(ch->specials.new_act, NEW_PLR_TELEPORT);
  }
}

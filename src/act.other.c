/*
 * ************************************************************************
 * *  file: act.other.c , Implementation of commands.        Part of DIKUMUD *
 * *  Usage : Other commands.                                                *
 * *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
 * ************************************************************************* 
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"

/*
 * extern variables 
 */

extern struct str_app_type       str_app[];
extern int                       DEBUG;
extern struct descriptor_data   *descriptor_list;
extern struct dex_skill_type     dex_app_skill[];
extern struct spell_info_type    spell_info[];
extern struct char_data         *character_list;
extern struct index_data        *obj_index;
extern struct time_info_data     time_info;

/*
 * extern procedures 
 */

void                             hit(struct char_data *ch, struct char_data *victim, int type);
void                             do_shout(struct char_data *ch, char *argument, int cmd);
char                            *how_good(int percent);
void                             zero_rent(struct char_data *ch);

void 
do_gain(struct char_data *ch, char *argument, int cmd)
{

}

void 
do_guard(struct char_data *ch, char *argument, int cmd)
{
  if (DEBUG)
    dlog("do_guard");
  if (!IS_NPC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) {
    send_to_char("Sorry. you can't just put your brain on autopilot!\n\r", ch);
    return;
  }
  if (IS_SET(ch->specials.act, ACT_GUARDIAN)) {
    act("$n relaxes.", FALSE, ch, 0, 0, TO_ROOM);
    send_to_char("You relax.\n\r", ch);
    REMOVE_BIT(ch->specials.act, ACT_GUARDIAN);
  } else {
    SET_BIT(ch->specials.act, ACT_GUARDIAN);
    act("$n alertly watches you.", FALSE, ch, 0, 0, TO_ROOM);
    send_to_char("You snap to attention\n\r", ch);
  }
  return;
}

void 
do_junk(struct char_data *ch, char *argument, int cmd)
{
  char                             arg[100],
                                   buf[100],
                                   newarg[100];
  struct obj_data                 *tmp_object,
                                  *old_object;
  int                              num,
                                   p,
                                   count;

  if (DEBUG)
    dlog("do_junk");
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
	  sprintf(buf, "You junk %s (%d).\n\r", arg, count);
	  act(buf, 1, ch, 0, 0, TO_CHAR);
	  sprintf(buf, "$n junks %s.\n\r", arg);
	  act(buf, 1, ch, 0, 0, TO_ROOM);
	} else if (count == 1) {
	  sprintf(buf, "You junk %s \n\r", arg);
	  act(buf, 1, ch, 0, 0, TO_CHAR);
	  sprintf(buf, "$n junks %s.\n\r", arg);
	  act(buf, 1, ch, 0, 0, TO_ROOM);
	} else {
	  send_to_char("You don't have anything like that\n\r", ch);
	}
	return;
      }
    }
  } else {
    send_to_char("Junk what?", ch);
    return;
  }
}

void 
do_qui(struct char_data *ch, char *argument, int cmd)
{
  if (DEBUG)
    dlog("do_qui");
  send_to_char("You have to write quit - no less, to quit!\n\r", ch);
  return;
}

void 
do_title(struct char_data *ch, char *argument, int cmd)
{
  char                             buf[200];

  if (IS_NPC(ch) || !ch->desc)
    return;

  for (; isspace(*argument); argument++);

  if (*argument) {
    sprintf(buf, "Your title has been set to : <%s>\n\r", argument);
    send_to_char(buf, ch);
    ch->player.title = strdup(argument);
  }
}

void 
do_quit(struct char_data *ch, char *argument, int cmd)
{
  void                             do_save(struct char_data *ch, char *argument, int cmd);
  void                             die(struct char_data *ch);

  if (DEBUG)
    dlog("do_quit");
  if (IS_NPC(ch) || !ch->desc)
    return;

  if (GET_POS(ch) == POSITION_FIGHTING) {
    send_to_char("No way! You are fighting.\n\r", ch);
    return;
  }
  if (GET_POS(ch) < POSITION_STUNNED) {
    send_to_char("You die before your time!\n\r", ch);
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
  extract_char(ch);		/*
				 * Char is saved in extract char 
				 */
}

void 
do_save(struct char_data *ch, char *argument, int cmd)
{
  char                             buf[100];
  struct obj_cost                  cost;

  if (IS_NPC(ch))
    return;

  sprintf(buf, "Saving %s.\n\r", GET_NAME(ch));
  send_to_char(buf, ch);
  recep_offer(ch, NULL, &cost);
  save_obj(ch, &cost, FALSE);
  save_char(ch, NOWHERE);
}

void 
do_not_here(struct char_data *ch, char *argument, int cmd)
{
  if (DEBUG)
    dlog("do_not_here");
  send_to_char("Sorry, but you cannot do that here!\n\r", ch);
}

void 
do_sneak(struct char_data *ch, char *argument, int cmd)
{
  struct affected_type             af;
  byte                             percent;

  if (DEBUG)
    dlog("do_sneak");
  if (IS_AFFECTED(ch, AFF_SNEAK)) {
    affect_from_char(ch, SKILL_SNEAK);
    if (IS_AFFECTED(ch, AFF_HIDE))
      REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);
    send_to_char("You are no longer sneaky.\n\r", ch);
    return;
  }
  send_to_char("Ok, you'll try to move silently for a while.\n\r", ch);
  send_to_char("And you attempt to hide yourself.\n\r", ch);

  percent = number(1, 101);	/*
				 * 101% is a complete failure 
				 */

  if (percent > ch->skills[SKILL_SNEAK].learned + dex_app_skill[GET_DEX(ch)].sneak)
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

void 
do_hide(struct char_data *ch, char *argument, int cmd)
{
  struct affected_type             af;
  byte                             percent;

  if (DEBUG)
    dlog("do_hide");
  send_to_char("Ok, you'll try to move silently for a while.\n\r", ch);
  send_to_char("and you attempt to hide yourself.\n\r", ch);

  if (IS_AFFECTED(ch, AFF_SNEAK))
    affect_from_char(ch, SKILL_SNEAK);
  if (IS_AFFECTED(ch, AFF_HIDE))
    REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);

  percent = number(1, 101);	/*
				 * 101% is a complete failure 
				 */

  if (percent > ch->skills[SKILL_SNEAK].learned + dex_app_skill[GET_DEX(ch)].sneak)
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

void 
do_steal(struct char_data *ch, char *argument, int cmd)
{
  struct char_data                *victim;
  struct obj_data                 *obj;
  char                             victim_name[240];
  char                             obj_name[240];
  char                             buf[240];
  int                              percent;
  int                              gold,
                                   eq_pos;
  bool                             ohoh = FALSE;
  struct room_data                *rp;

  if (DEBUG)
    dlog("do_steal");
  if (check_peaceful(ch, "What if he caught you?\n\r"))
    return;

  argument = one_argument(argument, obj_name);
  only_argument(argument, victim_name);

  if (!(victim = get_char_room_vis(ch, victim_name))) {
    send_to_char("Steal what from who?\n\r", ch);
    return;
  } else if (victim == ch) {
    send_to_char("Come on now, that's rather stupid!\n\r", ch);
    return;
  }
  if (!CheckKill(ch, victim))
    return;

  if ((!victim->desc) && (IS_PC(victim)))
    return;

  /*
   * 101% is a complete failure 
   */
  percent = number(1, 101) - dex_app_skill[GET_DEX(ch)].p_pocket;

  if (GET_POS(victim) < POSITION_SLEEPING)
    percent = -1;		/*
				 * ALWAYS SUCCESS 
				 */

  percent += GetTotLevel(victim);
  if (GetMaxLevel(victim) > MAX_MORT)
    percent = 101;		/*
				 * Failure 
				 */

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
      } else {			/*
				 * It is equipment 
				 */
	if ((GET_POS(victim) > POSITION_STUNNED)) {
	  send_to_char("Steal the equipment now? Impossible!\n\r", ch);
	  return;
	} else {
	  act("You unequip $p and steal it.", FALSE, ch, obj, 0, TO_CHAR);
	  act("$n steals $p from $N.", FALSE, ch, obj, victim, TO_NOTVICT);
	  obj_to_char(unequip_char(victim, eq_pos), ch);
	}
      }
    } else {			/*
				 * obj found in inventory 
				 */
      percent += GET_OBJ_WEIGHT(obj);	/*
					 * Make heavy harder 
					 */
      rp = real_roomp(ch->in_room);
      if (IS_SET(rp->room_flags, NO_STEAL)) {
	percent = 101;
      }
      if (AWAKE(victim) && (percent > ch->skills[SKILL_STEAL].learned)) {
	ohoh = TRUE;
	act("Oops..", FALSE, ch, 0, 0, TO_CHAR);
	act("$n tried to steal something from you!", FALSE, ch, 0, victim, TO_VICT);
	act("$n tries to steal something from $N.", TRUE, ch, 0, victim, TO_NOTVICT);
      } else {			/*
				 * Steal the item 
				 */
	if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
	  if ((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) < CAN_CARRY_W(ch)) {
	    obj_from_char(obj);
	    obj_to_char(obj, ch);
	    send_to_char("Got it!\n\r", ch);
	    if (ch->skills[SKILL_STEAL].learned < 50)
	      ch->skills[SKILL_STEAL].learned += 1;
	  }
	} else
	  send_to_char("You cannot carry that much.\n\r", ch);
      }
    }
  } else {			/*
				 * Steal some coins 
				 */
    if (AWAKE(victim) && (percent > ch->skills[SKILL_STEAL].learned)) {
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
	sprintf(buf, "Bingo! You got %d gold coins.\n\r", gold);
	send_to_char(buf, ch);
      } else {
	send_to_char("You couldn't get any gold...\n\r", ch);
      }
    }
  }

  if (ohoh && IS_NPC(victim) && AWAKE(victim))
    if (IS_SET(victim->specials.act, ACT_NICE_THIEF)) {
      sprintf(buf, "%s is a damn thief!", GET_NAME(ch));
      do_shout(victim, buf, 0);
      log(buf);
      send_to_char("Don't you ever do that again!\n\r", ch);
    } else {
      hit(victim, ch, TYPE_UNDEFINED);
    }
}

void 
do_practice(struct char_data *ch, char *arg, int cmd)
{
  char                             buf[256];
  int                              i;
  char                             first_arg[100],
                                   sec_arg[100];
  int                              flag;
  extern char                     *spells[];
  extern struct spell_info_type    spell_info[MAX_SPL_LIST];
  extern struct int_app_type       int_app[26];

#define RANGER_CAST_LEVEL 10

  struct skill_struct {
    char                             skill_name[40];
    int                              skill_numb;
  };

  struct skill_struct              t_skills[] =
  {
    {"sneak", SKILL_SNEAK},
    {"hide", SKILL_HIDE},
    {"steal", SKILL_STEAL},
    {"backstab", SKILL_BACKSTAB},
    {"pick", SKILL_PICK_LOCK},
    {"track", SKILL_HUNT},
    {"search", SKILL_SEARCH},
    {"peer", SKILL_PEER},
    {"find_traps", SKILL_FIND_TRAP},
/*
 * {"listen",           SKILL_DETECT_NOISE}, 
 * {"disarm_traps",    SKILL_DISARM_TRAP}, 
 */
    {"\n", -1}
  };

  struct skill_struct              wa_skills[] =
  {
    {"kick", SKILL_KICK},
    {"bash", SKILL_BASH},
    {"rescue", SKILL_RESCUE},
    {"disarm", SKILL_DISARM},
    {"punch", SKILL_PUNCH},
    {"doorbash", SKILL_DOOR_BASH},
    {"smite spec", SKILL_SPEC_SMITE},
    {"stab spec", SKILL_SPEC_STAB},
    {"whip spec", SKILL_SPEC_WHIP},
    {"slash spec", SKILL_SPEC_SLASH},
    {"smash spec", SKILL_SPEC_SMASH},
    {"cleave spec", SKILL_SPEC_CLEAVE},
    {"crush spec", SKILL_SPEC_CRUSH},
    {"bludge spec", SKILL_SPEC_BLUDGE},
    {"peirce spec", SKILL_SPEC_PIERCE},
    {'\n', -1}
  };

  struct skill_struct              r_skills[] =
  {
    {"rescue", SKILL_RESCUE},
    {"hide", SKILL_HIDE},
    {"sneak", SKILL_SNEAK},
    {"track", SKILL_HUNT},
    {"disarm", SKILL_DISARM},
    {"punch", SKILL_PUNCH},
    {"bash", SKILL_BASH},
    {"doorbash", SKILL_DOOR_BASH},
    {"search", SKILL_SEARCH},
    {"peer", SKILL_PEER},
  /*
   * { "listen", SKILL_DETECT_NOISE}, 
   */
    {"\n", -1}
  };

  struct skill_struct              r_spells[] =
  {
    {"armor", SPELL_ARMOR},
    {"create food", SPELL_CREATE_FOOD},

    {"cure light", SPELL_CURE_LIGHT},
    {"refresh", SPELL_REFRESH},
    {"faerie fire", SPELL_FAERIE_FIRE},
    {"faerie fog", SPELL_FAERIE_FOG},
    {"stone skin", SPELL_STONE_SKIN},
    {"second wind", SPELL_SECOND_WIND},
    {"cure serious", SPELL_CURE_SERIOUS},
    {"\n", -1}
  };

  if (DEBUG)
    dlog("do_practice");
  if ((cmd != 164) && (cmd != 170))
    return;

  for (; isspace(*arg); arg++);
  half_chop(arg, first_arg, sec_arg);

  if (!first_arg) {
    send_to_char("You need to supply a class for that.", ch);
    return;
  }
  switch (*first_arg) {
  case 'w':
  case 'W':
  case 'f':
  case 'F':
    {
      if (!GET_LEVEL(ch, WARRIOR_LEVEL_IND)) {
	send_to_char("I bet you think you're a warrior.\n\r", ch);
	return;
      }
      send_to_char("You can practise any of these skills:\n\r", ch);
      for (i = 0; wa_skills[i].skill_name[0] != '\n'; i++) {
	send_to_char(wa_skills[i].skill_name, ch);
	send_to_char(how_good(ch->skills[wa_skills[i].skill_numb].learned), ch);
	send_to_char("\n\r", ch);
      }
      return;
    }
    break;
  case 'r':
  case 'R':
    {
      if (!GET_LEVEL(ch, RANGER_LEVEL_IND)) {
	send_to_char("I bet you think you're a Ranger.\n\r", ch);
	return;
      }
      send_to_char("You can practise any of these skills:\n\r", ch);

      for (i = 0; r_skills[i].skill_name[0] != '\n'; i++) {
	send_to_char(r_skills[i].skill_name, ch);
	send_to_char(how_good(ch->skills[r_skills[i].skill_numb].learned), ch);
	send_to_char("\n\r", ch);
      }
      if (GET_LEVEL(ch, RANGER_LEVEL_IND) >= RANGER_CAST_LEVEL) {
	send_to_char("\n\r\n\r", ch);
	for (i = 0; r_spells[i].skill_name[0] != '\n'; i++) {
	  send_to_char(r_spells[i].skill_name, ch);
	  send_to_char(how_good(ch->skills[r_spells[i].skill_numb].learned), ch);
	  send_to_char("\n\r", ch);
	}
      }
      return;
    }
    break;
  case 't':
  case 'T':
    {
      if (!GET_LEVEL(ch, THIEF_LEVEL_IND)) {
	send_to_char("I bet you think you're a thief.\n\r", ch);
	return;
      }
      send_to_char("You can practice any of these skills:\n\r", ch);
      for (i = 0; t_skills[i].skill_name[0] != '\n'; i++) {
	send_to_char(t_skills[i].skill_name, ch);
	send_to_char(how_good(ch->skills[t_skills[i].skill_numb].learned), ch);
	send_to_char("\n\r", ch);
      }
      return;
    }
    break;
  case 'M':
  case 'm':
    {
      if (!GET_LEVEL(ch, MAGE_LEVEL_IND)) {
	send_to_char("I bet you think you're a magic-user.\n\r", ch);
	return;
      }
      flag = 0;
      if (is_abbrev(sec_arg, "known"))
	flag = 1;
      send_to_char("Your spellbook holds these spells:\n\r", ch);
      for (i = 0; *spells[i] != '\n'; i++)
	if (spell_info[i + 1].spell_pointer &&
	    (spell_info[i + 1].min_level_magic <= GET_LEVEL(ch, MAGE_LEVEL_IND))) {
	  if (!flag) {
	    sprintf(buf, "[%d] %s %s \n\r",
		    spell_info[i + 1].min_level_magic,
		    spells[i], how_good(ch->skills[i + 1].learned));
	    send_to_char(buf, ch);
	  } else {
	    if (ch->skills[i + 1].learned > 0) {
	      sprintf(buf, "[%d] %s %s \n\r",
		      spell_info[i + 1].min_level_magic,
		    spells[i], how_good(ch->skills[i + 1].learned));
	      send_to_char(buf, ch);
	    }
	  }
	}
      return;
    }
    break;
  case 'C':
  case 'c':
  case 'P':
  case 'p':
    {
      if (!GET_LEVEL(ch, CLERIC_LEVEL_IND)) {
	send_to_char("I bet you think you're a cleric.\n\r", ch);
	return;
      }
      send_to_char("You can attempt any of these spells:\n\r", ch);
      flag = 0;
      if (is_abbrev(sec_arg, "known"))
	flag = 1;
      for (i = 0; *spells[i] != '\n'; i++)
	if (spell_info[i + 1].spell_pointer &&
	    (spell_info[i + 1].min_level_cleric <= GET_LEVEL(ch, CLERIC_LEVEL_IND))) {
	  if (!flag) {
	    sprintf(buf, "[%d] %s %s \n\r",
		    spell_info[i + 1].min_level_cleric,
		    spells[i], how_good(ch->skills[i + 1].learned));
	    send_to_char(buf, ch);
	  } else {
	    if (ch->skills[i + 1].learned > 0) {
	      sprintf(buf, "[%d] %s %s \n\r",
		      spell_info[i + 1].min_level_cleric,
		    spells[i], how_good(ch->skills[i + 1].learned));
	      send_to_char(buf, ch);
	    }
	  }
	}
      return;
    }
    break;
  default:
    send_to_char("Which class???\n\r", ch);
  }

}

void 
do_idea(struct char_data *ch, char *argument, int cmd)
{
  FILE                            *fl;
  char                             str[MAX_INPUT_LENGTH + 20];
  struct descriptor_data          *i;

  if (DEBUG)
    dlog("do_idea");
  if (IS_NPC(ch)) {
    send_to_char("Monsters can't have ideas - Go away.\n\r", ch);
    return;
  }
  /*
   * skip whites 
   */
  for (; isspace(*argument); argument++);

  if (!*argument) {
    send_to_char
      ("That doesn't sound like a good idea to me.. Sorry.\n\r", ch);
    return;
  }
  if (!(fl = fopen(IDEA_FILE, "a"))) {
    perror("do_idea");
    send_to_char("Could not open the idea-file.\n\r", ch);
    return;
  }
  sprintf(str, "**%s: %s\n", GET_NAME(ch), argument);

  fputs(str, fl);
  fclose(fl);
  send_to_char("Ok. Thanks.\n\r", ch);
}

void 
do_typo(struct char_data *ch, char *argument, int cmd)
{
  FILE                            *fl;
  char                             str[MAX_INPUT_LENGTH + 20];
  struct descriptor_data          *i;

  if (DEBUG)
    dlog("do_typo");
  if (IS_NPC(ch)) {
    send_to_char("Monsters can't spell - leave me alone.\n\r", ch);
    return;
  }
  /*
   * skip whites 
   */
  for (; isspace(*argument); argument++);

  if (!*argument) {
    send_to_char("I beg your pardon?\n\r", ch);
    return;
  }
  if (!(fl = fopen(TYPO_FILE, "a"))) {
    perror("do_typo");
    send_to_char("Could not open the typo-file.\n\r", ch);
    return;
  }
  sprintf(str, "**%s[%d]: %s\n",
	  GET_NAME(ch), ch->in_room, argument);
  fputs(str, fl);
  fclose(fl);
  send_to_char("Ok. thanks.\n\r", ch);

}

void 
do_bug(struct char_data *ch, char *argument, int cmd)
{
  FILE                            *fl;
  char                             str[MAX_INPUT_LENGTH + 20];
  struct descriptor_data          *i;

  if (DEBUG)
    dlog("do_bug");
  if (IS_NPC(ch)) {
    send_to_char("You are a monster! Bug off!\n\r", ch);
    return;
  }
  /*
   * skip whites 
   */
  for (; isspace(*argument); argument++);

  if (!*argument) {
    send_to_char("Pardon?\n\r", ch);
    return;
  }
  if (!(fl = fopen(BUG_FILE, "a"))) {
    perror("do_bug");
    send_to_char("Could not open the bug-file.\n\r", ch);
    return;
  }
  sprintf(str, "**%s[%d]: %s\n",
	  GET_NAME(ch), ch->in_room, argument);
  fputs(str, fl);
  fclose(fl);
  send_to_char("Ok.\n\r", ch);
}

void 
do_brief(struct char_data *ch, char *argument, int cmd)
{
  if (DEBUG)
    dlog("do_brief");
  if (IS_NPC(ch))
    return;

  if (IS_SET(ch->specials.act, PLR_BRIEF)) {
    send_to_char("Brief mode off.\n\r", ch);
    REMOVE_BIT(ch->specials.act, PLR_BRIEF);
  } else {
    send_to_char("Brief mode on.\n\r", ch);
    SET_BIT(ch->specials.act, PLR_BRIEF);
  }
}

void 
do_compact(struct char_data *ch, char *argument, int cmd)
{
  if (DEBUG)
    dlog("do_compact");
  if (IS_NPC(ch))
    return;

  if (IS_SET(ch->specials.act, PLR_COMPACT)) {
    send_to_char("You are now in the uncompacted mode.\n\r", ch);
    REMOVE_BIT(ch->specials.act, PLR_COMPACT);
  } else {
    send_to_char("You are now in compact mode.\n\r", ch);
    SET_BIT(ch->specials.act, PLR_COMPACT);
  }
}

void 
do_group(struct char_data *ch, char *argument, int cmd)
{
  char                             name[256];
  struct char_data                *victim,
                                  *k;
  struct follow_type              *f;
  bool                             found;

  if (DEBUG)
    dlog("do_group");
  only_argument(argument, name);

  if (!*name) {
    if (!IS_AFFECTED(ch, AFF_GROUP)) {
      send_to_char("But you are a member of no group?!\n\r", ch);
    } else {
      send_to_char("Your group consists of:\n\r", ch);
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
    send_to_char("No one here by that name.\n\r", ch);
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
	act("$n has been kicked out of $N's group!", FALSE, victim, 0, ch, TO_ROOM);
	act("You are no longer a member of $N's group!", FALSE, victim, 0, ch, TO_CHAR);
	REMOVE_BIT(victim->specials.affected_by, AFF_GROUP);
      } else {
	if (GetMaxLevel(victim) >= LOW_IMMORTAL) {
	  act("You really don't want $n in your group.", FALSE, victim, 0, 0, TO_CHAR);
	  return;
	}
	if (GetMaxLevel(ch) >= LOW_IMMORTAL) {
	  act("Now now.  That would be CHEATING!", FALSE, ch, 0, 0, TO_CHAR);
	  return;

	}
	act("$n is now a member of $N's group.",
	    FALSE, victim, 0, ch, TO_ROOM);
	act("You are now a member of $N's group.",
	    FALSE, victim, 0, ch, TO_CHAR);
	SET_BIT(victim->specials.affected_by, AFF_GROUP);
      }
    } else {
      act("$N must follow you, to enter the group",
	  FALSE, ch, 0, victim, TO_CHAR);
    }
  }
}

void 
do_quaff(struct char_data *ch, char *argument, int cmd)
{
  char                             buf[100];
  struct obj_data                 *temp;
  int                              i;
  bool                             equipped;

  if (DEBUG)
    dlog("do_quaff");
  equipped = FALSE;

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
	act("$n is jolted and drops $p!  It shatters!",
	    TRUE, ch, temp, 0, TO_ROOM);
	act("You arm is jolted and $p flies from your hand, *SMASH*",
	    TRUE, ch, temp, 0, TO_CHAR);
	if (equipped)
	  temp = unequip_char(ch, HOLD);
	extract_obj(temp);
	return;
      }
    } else {
      if (number(1, 20) > ch->abilities.dex - 4) {
	act("$n is jolted and drops $p!  It shatters!",
	    TRUE, ch, temp, 0, TO_ROOM);
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
       ((byte)temp->obj_flags.value[0], ch, "", SPELL_TYPE_POTION, ch, temp));

  if (equipped)
    temp = unequip_char(ch, HOLD);

  extract_obj(temp);

  WAIT_STATE(ch, PULSE_VIOLENCE);

}

void 
do_recite(struct char_data *ch, char *argument, int cmd)
{
  char                             buf[100];
  struct obj_data                 *scroll,
                                  *obj;
  struct char_data                *victim;
  int                              i,
                                   bits;
  bool                             equipped;

  if (DEBUG)
    dlog("do_recite");
  equipped = FALSE;
  obj = 0;
  victim = 0;

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
      send_to_char("No such thing around to recite the scroll on.\n\r", ch);
      return;
    }
  } else {
    victim = ch;
  }

  if (ch->skills[SKILL_READ_MAGIC].learned < number(1, 101)) {
    if (scroll->obj_flags.value[1] != SPELL_WORD_OF_RECALL) {
      send_to_char("You can't understand this...\n\r", ch);
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
       ((byte)scroll->obj_flags.value[0], ch, "", SPELL_TYPE_SCROLL, victim, obj));
    }
  if (equipped)
    scroll = unequip_char(ch, HOLD);
  extract_obj(scroll);
}

void 
do_use(struct char_data *ch, char *argument, int cmd)
{
  char                             buf[100];
  struct char_data                *tmp_char;
  struct obj_data                 *tmp_object,
                                  *stick;

  int                              bits;

  if (DEBUG)
    dlog("do_use");
  argument = one_argument(argument, buf);

  if (ch->equipment[HOLD] == 0 ||
      !isname(buf, ch->equipment[HOLD]->name)) {
    act("You do not hold that item in your hand.", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  stick = ch->equipment[HOLD];

  if (stick->obj_flags.type_flag == ITEM_STAFF) {
    act("$n taps $p three times on the ground.", TRUE, ch, stick, 0, TO_ROOM);
    act("You tap $p three times on the ground.", FALSE, ch, stick, 0, TO_CHAR);
    if (stick->obj_flags.value[2] > 0) {	/*
						 * Is there any charges left? 
						 */
      stick->obj_flags.value[2]--;
      ((*spell_info[stick->obj_flags.value[3]].spell_pointer)
       ((byte)stick->obj_flags.value[0], ch, "", SPELL_TYPE_STAFF, 0, 0));
      WAIT_STATE(ch, PULSE_VIOLENCE);
    } else {
      send_to_char("The staff seems powerless.\n\r", ch);
    }
  } else if (stick->obj_flags.type_flag == ITEM_WAND) {
    bits = generic_find(argument, FIND_CHAR_ROOM | FIND_OBJ_INV |
	FIND_OBJ_ROOM | FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);
    if (bits) {
      struct spell_info_type          *spellp;

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

      if (stick->obj_flags.value[2] > 0) {	/*
						 * Is there any charges left? 
						 */
	stick->obj_flags.value[2]--;
	((*spellp->spell_pointer)
	 ((byte)stick->obj_flags.value[0], ch, "", SPELL_TYPE_WAND,
	  tmp_char, tmp_object));
	WAIT_STATE(ch, PULSE_VIOLENCE);
      } else {
	send_to_char("The wand seems powerless.\n\r", ch);
      }
    } else {
      send_to_char("What should the wand be pointed at?\n\r", ch);
    }
  } else {
    send_to_char("Use is normally only for wand's and staff's.\n\r", ch);
  }
}

do_plr_noshout(struct char_data *ch, char *argument, int cmd)
{
  char                             buf[128];

  if (DEBUG)
    dlog("do_plr_noshout");
  if (IS_NPC(ch))
    return;

  only_argument(argument, buf);

  if (!*buf) {
    if (IS_SET(ch->specials.act, PLR_DEAF)) {
      send_to_char("You can now hear shouts again.\n\r", ch);
      REMOVE_BIT(ch->specials.act, PLR_DEAF);
    } else {
      send_to_char("From now on, you won't hear shouts.\n\r", ch);
      SET_BIT(ch->specials.act, PLR_DEAF);
    }
  } else {
    send_to_char("Only the gods can shut up someone else. \n\r", ch);
  }
}
do_plr_notell(struct char_data *ch, char *argument, int cmd)
{
  char                             buf[128];

  if (DEBUG)
    dlog("do_plr_notell");
  if (IS_NPC(ch))
    return;
  only_argument(argument, buf);

  if (IS_SET(ch->specials.new_act, NEW_PLR_NOTELL)) {
    send_to_char("You can now hear tells again.\n\r", ch);
    REMOVE_BIT(ch->specials.new_act, NEW_PLR_NOTELL);
  } else {
    send_to_char("From now on, you won't hear tells.\n\r", ch);
    SET_BIT(ch->specials.new_act, NEW_PLR_NOTELL);
  }
}

do_plr_nosummon(struct char_data *ch, char *argument, int cmd)
{

  if (DEBUG)
    dlog("do_plr_nosummon");
  if (IS_NPC(ch))
    return;

  if (IS_SET(ch->specials.new_act, NEW_PLR_SUMMON)) {
    send_to_char("You can now be summoned.\n\r", ch);
    REMOVE_BIT(ch->specials.new_act, NEW_PLR_SUMMON);
  } else {
    send_to_char("From now on, you can't be summoned.\n\r", ch);
    SET_BIT(ch->specials.new_act, NEW_PLR_SUMMON);
  }
}

do_plr_noteleport(struct char_data *ch, char *argument, int cmd)
{

  if (DEBUG)
    dlog("do_plr_noteleport");
  if (IS_NPC(ch))
    return;

  if (IS_SET(ch->specials.new_act, NEW_PLR_TELEPORT)) {
    send_to_char("You can now be teleported.\n\r", ch);
    REMOVE_BIT(ch->specials.new_act, NEW_PLR_TELEPORT);
  } else {
    send_to_char("From now on, you can't be teleported.\n\r", ch);
    SET_BIT(ch->specials.new_act, NEW_PLR_TELEPORT);
  }
}

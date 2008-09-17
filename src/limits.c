/*
 * file: limits.c , Limit and gain control module.        Part of DIKUMUD
 * Usage: Procedures controling gain and limit.
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */

#include <stdio.h>
#include <stdlib.h>
/* #include <unistd.h> */
#include <sys/types.h>
#include <signal.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "global.h"
#include "bug.h"
#include "spells.h"
#include "comm.h"
#include "db.h"
#include "spell_parser.h"
#include "constants.h"
#include "utils.h"
#include "multiclass.h"
#include "fight.h"
#include "reception.h"
#include "interpreter.h"
#include "handler.h"
#include "act_obj.h"
#include "act_other.h"
#define _DIKU_LIMITS_C
#include "mudlimits.h"

char                                   *ClassTitles(struct char_data *ch)
{
  int                                     i = 0;
  int                                     count = 0;
  static char                             buf[256] = "\0\0\0";

  if (DEBUG > 2)
    log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  for (i = MAGE_LEVEL_IND; i <= DRUID_LEVEL_IND; i++) {
    if (GET_LEVEL(ch, i)) {
      count++;
      if (count > 1) {
	sprintf(buf + strlen(buf), "/%s", GET_CLASS_TITLE(ch, i, GET_LEVEL(ch, i)));
      } else {
	sprintf(buf, "%s", GET_CLASS_TITLE(ch, i, GET_LEVEL(ch, i)));
      }
    }
  }
  return (buf);
}

/* When age < 15 return the value p0 */
/* When age in 15..29 calculate the line between p1 & p2 */
/* When age in 30..44 calculate the line between p2 & p3 */
/* When age in 45..59 calculate the line between p3 & p4 */
/* When age in 60..79 calculate the line between p4 & p5 */
/* When age >= 80 return the value p6 */
int graf(int char_age, int p0, int p1, int p2, int p3, int p4, int p5, int p6)
{
  if (DEBUG > 2)
    log_info("called %s with %d, %d, %d, %d, %d, %d, %d, %d", __PRETTY_FUNCTION__, char_age, p0, p1, p2, p3, p4, p5, p6);

  if (char_age < 15)
    return (p0);					       /* < 15 */
  else if (char_age <= 29)
    return (int)(p1 + (((char_age - 15) * (p2 - p1)) / 15));   /* 15..29 */
  else if (char_age <= 44)
    return (int)(p2 + (((char_age - 30) * (p3 - p2)) / 15));   /* 30..44 */
  else if (char_age <= 59)
    return (int)(p3 + (((char_age - 45) * (p4 - p3)) / 15));   /* 45..59 */
  else if (char_age <= 79)
    return (int)(p4 + (((char_age - 60) * (p5 - p4)) / 20));   /* 60..79 */
  else
    return (p6);					       /* >= 80 */
}

/* The three MAX functions define a characters Effective maximum */
/* Which is NOT the same as the ch->points.max_xxxx !!!          */
int mana_limit(struct char_data *ch)
{
  int                                     max = 100;
  int                                     extra = 0;
  int                                     cl = 0;

  if (DEBUG > 2)
    log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  if (IS_PC(ch)) {
    if (HasClass(ch, CLASS_MAGIC_USER))
      extra += GET_LEVEL(ch, MAGE_LEVEL_IND) * 5;
    if (HasClass(ch, CLASS_CLERIC))
      extra += GET_LEVEL(ch, CLERIC_LEVEL_IND) * 4;
    if (HasClass(ch, CLASS_DRUID))
      extra += GET_LEVEL(ch, DRUID_LEVEL_IND) * 3;
    if (HasClass(ch, CLASS_RANGER))
      extra += (GET_LEVEL(ch, RANGER_LEVEL_IND) * 5) / 2;
    if (HasClass(ch, CLASS_THIEF))
      extra += (GET_LEVEL(ch, THIEF_LEVEL_IND) * 4) / 2;
    if (HasClass(ch, CLASS_WARRIOR))
      extra += (GET_LEVEL(ch, WARRIOR_LEVEL_IND) * 3) / 2;
    cl = HowManyClasses(ch);
    if ((cl = HowManyClasses(ch)) > 1)
      extra = ((extra * 10) / ((cl * 10) + 5));
    max += extra;
  }
  max += ch->points.max_mana;				       /* bonus mana */
  return (max);
}

int hit_limit(struct char_data *ch)
{
  int                                     max = 0;

  if (DEBUG > 2)
    log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  if (IS_PC(ch))
    max = (ch->points.max_hit) + (graf(age(ch).year, 2, 4, 17, 14, 8, 4, 3));
  else
    max = (ch->points.max_hit);

  /*
   * Class/Level calculations 
   */
  if (HowManyClasses(ch) == 1) {
    if (HasClass(ch, CLASS_RANGER))
      max += (GET_LEVEL(ch, RANGER_LEVEL_IND) / 5) + 2;
    else if (HasClass(ch, CLASS_WARRIOR))
      max += (GET_LEVEL(ch, WARRIOR_LEVEL_IND) / 2) + 1;
  }
  /*
   * Skill/Spell calculations 
   */

  return (max);
}

int move_limit(struct char_data *ch)
{
  int                                     max = 0;

  if (DEBUG > 2)
    log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  if (!IS_NPC(ch))
    max = 70 + age(ch).year + (int)GET_CON(ch) + GetTotLevel(ch);
  else
    max = ch->points.max_move;

  switch (GET_RACE(ch)) {
    case RACE_DWARF:
      max -= 15;
      break;
    case RACE_GNOME:
      max -= 10;
      break;
    case RACE_HALFLING:
      max += 5;
      break;
    case RACE_ELVEN:
      max += 10;
      break;
  }
  return (max);
}

int mana_gain(struct char_data *ch)
{
  int                                     gain = 0;

  if (DEBUG > 2)
    log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  if (IS_NPC(ch)) {
    /*
     * Neat and fast 
     */
    gain = GetTotLevel(ch);
  } else {
    gain = graf(age(ch).year, 2, 4, 6, 8, 10, 12, 16);

    if (GET_RACE(ch) == RACE_ELVEN)
      gain += 5;
    if (GET_RACE(ch) == RACE_GNOME)
      gain += 2;

    /*
     * Class calculations 
     */
    if (HasClass(ch, CLASS_MAGIC_USER))
      gain += 2;
    if (HasClass(ch, CLASS_CLERIC))
      gain += 2;
    if (HasClass(ch, CLASS_DRUID))
      gain += 1;

    /*
     * Skill/Spell calculations 
     */
    /*
     * Position calculations 
     */

    switch (GET_POS(ch)) {
      case POSITION_SLEEPING:
	gain += gain;
	break;
      case POSITION_RESTING:
	gain += (gain >> 1);				       /* Divide by 2 */
	break;
      case POSITION_SITTING:
	gain += (gain >> 2);				       /* Divide by 4 */
	break;
    }

    if (HasClass(ch, CLASS_MAGIC_USER) ||
	HasClass(ch, CLASS_CLERIC) || HasClass(ch, CLASS_DRUID))
      gain += gain;
  }

  if (IS_AFFECTED(ch, AFF_POISON))
    gain >>= 2;

  if (number(1, 101) < ch->skills[SKILL_MEDITATION].learned)
    gain += 10;

  if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0))
    /*
     * gain >>= 2; 
     */
    gain = -number(1, 4);

  return (gain);
}

int hit_gain(struct char_data *ch)
{
  int                                     gain = 0;

  if (DEBUG > 2)
    log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  if (IS_NPC(ch)) {
    gain = 8;
  } else {
    if (GET_POS(ch) == POSITION_FIGHTING) {
      gain = 1;
    } else {
      gain = graf(age(ch).year, 2, 5, 10, 18, 6, 4, 2);
    }

    /*
     * Class/Level calculations 
     */
    if (HasClass(ch, CLASS_MAGIC_USER))
      gain -= 2;
    if (HasClass(ch, CLASS_CLERIC))
      gain -= 1;
    if (HasClass(ch, CLASS_WARRIOR))
      gain += 3;
    if (HasClass(ch, CLASS_RANGER))
      gain += 2;

    /*
     * Skill/Spell calculations 
     */

    /*
     * Position calculations 
     */

    switch (GET_POS(ch)) {
      case POSITION_SLEEPING:
	gain += gain >> 1;
	break;
      case POSITION_RESTING:
	gain += gain >> 2;
	break;
      case POSITION_SITTING:
	gain += gain >> 3;
	break;
    }

    if (GET_POS(ch) == POSITION_SLEEPING)
      if (number(1, 101) < ch->skills[SKILL_MEDITATION].learned)
	gain += 3;
  }

  if (GET_RACE(ch) == RACE_DWARF)
    gain += 5;
  if (GET_RACE(ch) == RACE_HALFLING)
    gain += 2;

  if (GET_RACE(ch) == RACE_ELVEN)
    gain -= 1;

  if (GET_RACE(ch) == RACE_GNOME)
    gain += 1;

  if (IS_AFFECTED(ch, AFF_POISON)) {
    gain >>= 2;
    damage(ch, ch, 15, SPELL_POISON);
  }
  gain = MAX(gain, 1);

  if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0)) {
    gain = 0;
    /*
     * damage(ch, ch, number(2,5), TYPE_HUNGER); 
     */
    /*
     * damage(i, i, 0, TYPE_SUFFERING); 
     */
    gain = -number(2, 5);
  }

  return (gain);
}

/*
 * move gain pr. game hour 
 */
int move_gain(struct char_data *ch)
{
  int                                     gain = 0;

  if (DEBUG > 2)
    log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  if (IS_NPC(ch))
    return (GetTotLevel(ch));
  else {
    if (GET_POS(ch) != POSITION_FIGHTING)
      gain = 5 + (int)GET_CON(ch);
    else {
      if (number(1, 101) < ch->skills[SKILL_ENDURANCE].learned)
	gain = 2;
      else
	gain = 0;
    }

    if (HasClass(ch, CLASS_RANGER))
      gain += 3;
    if (HasClass(ch, CLASS_DRUID))
      gain += 2;

    /*
     * Position calculations 
     */
    switch (GET_POS(ch)) {
      case POSITION_SLEEPING:
	gain += (gain >> 1);				       /* Divide by 2 */
	break;
      case POSITION_RESTING:
	gain += (gain >> 2);				       /* Divide by 4 */
	break;
      case POSITION_SITTING:
	gain += (gain >> 3);				       /* Divide by 8 */
	break;
    }
  }

  if (GET_RACE(ch) == RACE_DWARF)
    gain += 4;
  if (GET_RACE(ch) == RACE_HALFLING)
    gain += 3;
  if (GET_RACE(ch) == RACE_ELVEN)
    gain += 1;

  if (IS_AFFECTED(ch, AFF_POISON))
    gain >>= 2;

  if (number(1, 101) < ch->skills[SKILL_ENDURANCE].learned)
    gain += 5;

  if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0))
    /*
     * gain >>= 2; 
     */
    /*
     * gain= -number(1,8); 
     */
    gain = 1;

  return (gain);
}

/* Gain maximum in various points */
void advance_level(struct char_data *ch, int class)
{
  int                                     add_hp = 0;
  int                                     i = 0;

  if (DEBUG > 1)
    log_info("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), class);

  if (GET_LEVEL(ch, class) > 0 && GET_EXP(ch) <
#ifdef MOB_LEVELING
      ((IS_PC(ch) || (IS_SET(ch->specials.act, ACT_POLYSELF) ||
		      IS_SET(ch->specials.act, ACT_POLYSELF))) ?
       titles[class][GET_LEVEL(ch, class) + 1].exp :
       (titles[class][GET_LEVEL(ch, class) + 1].exp / 20))
#else
      titles[class][GET_LEVEL(ch, class) + 1].exp
#endif
    ) {
    log_info("Bad advance_level");
    return;
  }
  GET_LEVEL(ch, class) += 1;

/* Constitution Bonus only for Fighter types */

  if ((class == RANGER_LEVEL_IND) || (class == WARRIOR_LEVEL_IND))
    add_hp = con_app[(int)GET_CON(ch)].hitp;
  else
    add_hp = MIN(con_app[(int)GET_CON(ch)].hitp, 2);

  switch (class) {
    case MAGE_LEVEL_IND:
      {
	ch->specials.pracs += MAX(3, wis_app[(int)GET_INT(ch)].bonus);
	if (GET_LEVEL(ch, MAGE_LEVEL_IND) < 30)
	  add_hp += number(2, 4);
	else
	  add_hp += 1;
      }
      break;

    case DRUID_LEVEL_IND:
      {
	if (GET_LEVEL(ch, DRUID_LEVEL_IND) < 20)
	  add_hp += number(2, 8);
	else
	  add_hp += 3;
      }
      break;

    case CLERIC_LEVEL_IND:
      {
	ch->specials.pracs += MAX(3, wis_app[(int)GET_WIS(ch)].bonus);
	if (GET_LEVEL(ch, CLERIC_LEVEL_IND) < 30)
	  add_hp += number(2, 8);
	else
	  add_hp += 3;
      }
      break;

    case THIEF_LEVEL_IND:
      {
	ch->specials.pracs += MAX(3, wis_app[(int)GET_DEX(ch)].bonus);
	if (GET_LEVEL(ch, THIEF_LEVEL_IND) < 30)
	  add_hp += number(2, 6);
	else
	  add_hp += 2;
      }
      break;

    case RANGER_LEVEL_IND:
      {
	ch->specials.pracs +=
	  MAX(3,
	      wis_app[((int)GET_DEX(ch) >=
		       (int)GET_STR(ch) ? (int)GET_DEX(ch) : (int)GET_STR(ch))].bonus);
	if (GET_LEVEL(ch, RANGER_LEVEL_IND) < 30)
	  add_hp += number(2, 10);
	else
	  add_hp += 4;
      }
      break;

    case WARRIOR_LEVEL_IND:
      {
	ch->specials.pracs += MAX(3, wis_app[(int)GET_STR(ch)].bonus);
	if (GET_LEVEL(ch, WARRIOR_LEVEL_IND) < 30)
	  add_hp += number(2, 10);
	else
	  add_hp += 4;
      }
      break;
  }

  add_hp /= HowManyClasses(ch);

  add_hp++;

  if (GET_LEVEL(ch, class) <= 5)
    add_hp++;

  ch->points.max_hit += MAX(1, add_hp);

  if (GetMaxLevel(ch) >= LOW_IMMORTAL)
    for (i = 0; i < 3; i++)
      ch->specials.conditions[i] = -1;

  ch->points.max_move = GET_MAX_MOVE(ch);
  if (IS_PC(ch))
    update_player_list_entry(ch->desc);
  log_info("%s advances to level %d.\n\r", GET_NAME(ch), GetMaxLevel(ch));
}

/* Lose in various points */
/*
 * ** Damn tricky for multi-class...
 */

void drop_level(struct char_data *ch, int class)
{
  int                                     add_hp = 0;
  int                                     lin_class = 0;

  if (DEBUG > 1)
    log_info("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), class);

/*
 * if (GetMaxLevel(ch) >= LOW_IMMORTAL)
 *   return;
 */
  if (GetMaxLevel(ch) == 1)
    return;

  add_hp = con_app[(int)GET_CON(ch)].hitp;

  switch (class) {

    case CLASS_MAGIC_USER:{
	lin_class = MAGE_LEVEL_IND;
	if (GET_LEVEL(ch, MAGE_LEVEL_IND) < 30)
	  add_hp += number(2, 4);
	else
	  add_hp += 1;
      }
      break;

    case CLASS_DRUID:{
	lin_class = DRUID_LEVEL_IND;
	if (GET_LEVEL(ch, DRUID_LEVEL_IND) < 30)
	  add_hp += number(2, 8);
	else
	  add_hp += 3;
      }
      break;

    case CLASS_CLERIC:{
	lin_class = CLERIC_LEVEL_IND;
	if (GET_LEVEL(ch, CLERIC_LEVEL_IND) < 30)
	  add_hp += number(2, 8);
	else
	  add_hp += 3;
      }
      break;

    case CLASS_THIEF:{
	lin_class = THIEF_LEVEL_IND;
	if (GET_LEVEL(ch, THIEF_LEVEL_IND) < 30)
	  add_hp += number(2, 6);
	else
	  add_hp += 2;
      }
      break;

    case CLASS_WARRIOR:{
	lin_class = WARRIOR_LEVEL_IND;
	if (GET_LEVEL(ch, WARRIOR_LEVEL_IND) < 30)
	  add_hp += number(2, 10);
	else
	  add_hp += 4;
      }
      break;

    case CLASS_RANGER:{
	lin_class = RANGER_LEVEL_IND;
	if (GET_LEVEL(ch, RANGER_LEVEL_IND) < 30)
	  add_hp += number(2, 10);
	else
	  add_hp += 4;
      }
      break;
  }

  add_hp /= HowManyClasses(ch);

  if (IS_NPC(ch)) {
    gain_exp(ch, -GET_EXP(ch) / 4);
  } else if (GetMaxLevel(ch) < 7) {
    gain_exp(ch, -GET_EXP(ch) / (12 * HowManyClasses(ch)));
  } else if (GetMaxLevel(ch) < 14) {
    gain_exp(ch, -GET_EXP(ch) / (10 * HowManyClasses(ch)));
  } else if (GetMaxLevel(ch) < 21) {
    gain_exp(ch, -GET_EXP(ch) / (8 * HowManyClasses(ch)));
  } else if (GetMaxLevel(ch) < 28) {
    gain_exp(ch, -GET_EXP(ch) / (6 * HowManyClasses(ch)));
  } else
    gain_exp(ch, -GET_EXP(ch) / (4 * HowManyClasses(ch)));

  GET_LEVEL(ch, class) -= 1;
  if (GET_LEVEL(ch, class) < 1)
    GET_LEVEL(ch, class) = 1;

  ch->points.max_hit -= MAX(1, add_hp);
  if (ch->points.max_hit < 1)
    ch->points.max_hit = 1;

  ch->specials.pracs -= MAX(3, wis_app[(int)GET_WIS(ch)].bonus);

/*
 *ch->points.exp =
 *  MIN(titles[lin_class][GET_LEVEL(ch, lin_class)].exp, GET_EXP(ch));
 */
  if (ch->points.exp < 0)
    ch->points.exp = 0;
  update_player_list_entry(ch->desc);
  log_info("%s drops to level %d.\n\r", GET_NAME(ch), GetMaxLevel(ch));
}

void set_title(struct char_data *ch)
{
  char                                    buf[256] = "\0\0\0";

  if (DEBUG > 1)
    log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  sprintf(buf, "the %s %s", RaceName[ch->race], ClassTitles(ch));
  if (GET_TITLE(ch)) {
    DESTROY(GET_TITLE(ch));
    CREATE(GET_TITLE(ch), char, strlen      (buf) + 1);
  } else {
    CREATE(GET_TITLE(ch), char, strlen      (buf) + 1);
  }
  strcpy(GET_TITLE(ch), buf);
}

void gain_exp(struct char_data *ch, int gain)
{
  int                                     i = 0;

  if (DEBUG > 2)
    log_info("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), gain);

/*  save_char(ch,NOWHERE); */

  if (!IS_IMMORTAL(ch)) {
    if (gain > 0) {
      gain = MIN(100000, gain);

      if (IS_PC(ch) || (IS_SET(ch->specials.act, ACT_POLYSELF) ||
			IS_SET(ch->specials.act, ACT_POLYSELF))) {
	gain /= HowManyClasses(ch);
      } else {
#ifdef MOB_LEVELING
	GET_EXP(ch) += gain;
	for (i = MAGE_LEVEL_IND; i <= DRUID_LEVEL_IND; i++) {
	  if (GET_LEVEL(ch, i)) {
	    if (GET_EXP(ch) >= ((titles[i][GET_LEVEL(ch, i) + 1].exp / 20))) {
	      advance_level(ch, i);
	      act("$n seems to be looking more healthy today.", FALSE, ch, 0, 0, TO_ROOM);
	    }
	  }
	}
	return;
#endif
      }

      if (IS_PC(ch) && (GetMaxLevel(ch) == 1))
	gain *= 2;

      if (IS_PC(ch) || (IS_SET(ch->specials.act, ACT_POLYSELF) ||
			IS_SET(ch->specials.act, ACT_POLYSELF))) {
	for (i = MAGE_LEVEL_IND; i <= DRUID_LEVEL_IND; i++) {
	  if (GET_LEVEL(ch, i)) {
	    if (GET_EXP(ch) >= titles[i][GET_LEVEL(ch, i) + 2].exp) {
	      cprintf(ch, "You will not gain anymore exp until you practice at a guild.\n\r");
	      GET_EXP(ch) = titles[i][GET_LEVEL(ch, i) + 2].exp - 1;
	      return;
	    } else if (GET_EXP(ch) >= titles[i][GET_LEVEL(ch, i) + 1].exp) {
	      /*
	       * do nothing..this is cool 
	       */
	    } else if (GET_EXP(ch) + gain >= titles[i][GET_LEVEL(ch, i) + 1].exp) {
	      cprintf(ch, "You have gained enough to be a(n) %s\n\r",
		      GET_CLASS_TITLE(ch, i, GET_LEVEL(ch, i) + 1));
	      cprintf(ch, "You will not gain anymore exp until you practice at a guild.\n\r");
	      if (GET_EXP(ch) + gain >= titles[i][GET_LEVEL(ch, i) + 2].exp) {
		GET_EXP(ch) = titles[i][GET_LEVEL(ch, i) + 2].exp - 1;
		return;
	      }
	    }
	  }
	}
      }
      GET_EXP(ch) += gain;
      if (IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) {
	for (i = MAGE_LEVEL_IND; i <= DRUID_LEVEL_IND; i++) {
	  if (GET_LEVEL(ch, i)) {
	    if (GET_EXP(ch) > titles[i][GET_LEVEL(ch, i) + 2].exp) {
	      GET_EXP(ch) = titles[i][GET_LEVEL(ch, i) + 2].exp - 1;
	    }
	  }
	}
      }
    }
    if (gain < 0) {
      GET_EXP(ch) += gain;
      if (GET_EXP(ch) < 0)
	GET_EXP(ch) = 0;
    }
  }
}

void gain_exp_regardless(struct char_data *ch, int gain, int class)
{
  int                                     i = 0;
  int                                     is_altered = FALSE;

  if (DEBUG > 2)
    log_info("called %s with %s, %d, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), gain, class);

  save_char(ch, NOWHERE);
  if (!IS_NPC(ch)) {
    if (gain > 0) {
      GET_EXP(ch) += gain;

      for (i = 0; (i < ABS_MAX_LVL) && (titles[class][i].exp <= GET_EXP(ch)); i++) {
	if (i > GET_LEVEL(ch, class)) {
	  cprintf(ch, "You raise a level\n\r");
/*        GET_LEVEL(ch,class) = i; */
	  advance_level(ch, class);
	  is_altered = TRUE;
	}
      }
    }
    if (gain < 0)
      GET_EXP(ch) += gain;

    if (GET_EXP(ch) < 0)
      GET_EXP(ch) = 0;
  }
  if (is_altered)
    set_title(ch);
}

void gain_condition(struct char_data *ch, int condition, int value)
{
  int                                     intoxicated = FALSE;

  if (DEBUG > 2)
    log_info("called %s with %s, %d, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), condition, value);

  if (GET_COND(ch, condition) == -1)			       /* No change */
    return;

  intoxicated = (GET_COND(ch, DRUNK) > 0);

  GET_COND(ch, condition) += value;

  GET_COND(ch, condition) = MAX(0, GET_COND(ch, condition));
  GET_COND(ch, condition) = MIN(24, GET_COND(ch, condition));

  if (GET_COND(ch, condition))
    return;

  switch (condition) {
    case FULL:
      {
	cprintf(ch, "You are hungry.\n\r");
	return;
      }
    case THIRST:
      {
	cprintf(ch, "You are thirsty.\n\r");
	return;
      }
    case DRUNK:
      {
	if (intoxicated)
	  cprintf(ch, "You are now sober.\n\r");
	return;
      }
    default:
      break;
  }
}

void check_idling(struct char_data *ch)
{
  struct obj_cost                         cost;

  if (DEBUG > 2)
    log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  ++(ch->specials.timer);

  if (ch->specials.timer > 5 && ch->specials.timer < 10) {
    do_save(ch, "", 0);
    return;
  }
  if (ch->specials.timer >= 10) {
    log_info("LOG:%s AUTOSAVE:Timer %d.", GET_NAME(ch), ch->specials.timer);

    if (ch->specials.fighting) {
      stop_fighting(ch->specials.fighting);
      stop_fighting(ch);
    }
    GET_POS(ch) = POSITION_STANDING;
    char_from_room(ch);
    char_to_room(ch, 0);
    if (IS_IMMORTAL(ch))
      GET_HOME(ch) = 1000;
    else
      GET_HOME(ch) = 3008;

    if (recep_offer(ch, NULL, &cost)) {
      cost.total_cost = 0;
      new_save_equipment(ch, &cost, FALSE);
      save_obj(ch, &cost, TRUE);
    }
    extract_char(ch);

    if (ch->desc)
      close_socket(ch->desc);

    /* ch->desc = 0; already done inside close_socket, thanks valgrind! */

    log_info("Done Auto-Saving.");
  }
}

/* Update both PC's & NPC's and objects */
void point_update(int current_pulse)
{
  struct char_data                       *i = NULL;
  struct char_data                       *next_dude = NULL;
  struct obj_data                        *j = NULL;
  struct obj_data                        *next_thing = NULL;
  int                                     count = 0;

  if (DEBUG > 2)
    log_info("called %s with %d", __PRETTY_FUNCTION__, current_pulse);

  /*
   * characters 
   */
  for (i = character_list; i; i = next_dude) {
    next_dude = i->next;
    if (GET_POS(i) >= POSITION_STUNNED) {
      if (!affected_by_spell(i, SPELL_AID)) {
	GET_HIT(i) = MIN(GET_HIT(i) + hit_gain(i), hit_limit(i));
      } else {
	if (GET_HIT(i) < hit_limit(i)) {
	  GET_HIT(i) = MIN(GET_HIT(i) + hit_gain(i), hit_limit(i));
	}
      }

      GET_MANA(i) = MIN(GET_MANA(i) + mana_gain(i), mana_limit(i));
      GET_MOVE(i) = MIN(GET_MOVE(i) + move_gain(i), move_limit(i));

      if (GET_POS(i) == POSITION_STUNNED)
	update_pos(i);
    } else if (GET_POS(i) == POSITION_INCAP) {
      /*
       * damage(i, i, 0, TYPE_SUFFERING); 
       */
      GET_HIT(i) += 1;
      update_pos(i);
    } else if (IS_PC(i) && (GET_POS(i) == POSITION_MORTALLYW))
      damage(i, i, 1, TYPE_SUFFERING);

    if (!IS_NPC(i)) {
      update_char_objects(i);
      if (GetMaxLevel(i) < CREATOR)
	check_idling(i);
    }
    gain_condition(i, FULL, -1);
    gain_condition(i, DRUNK, -1);
    gain_condition(i, THIRST, -1);
  }

  /*
   * objects 
   */
  for (j = object_list; j; j = next_thing) {
    next_thing = j->next;
    count++;

    if ((GET_ITEM_TYPE(j) == ITEM_FOOD) && IS_OBJ_STAT(j, ITEM_PARISH)) {
      if (j->obj_flags.value[0] > 0)
	j->obj_flags.value[0]--;

      switch (j->obj_flags.value[0]) {
	case 3:
	  {
	    if (j->carried_by)
	      act("$p begins to look a little brown.", FALSE, j->carried_by, j, 0, TO_CHAR);
	    else if (j->in_room != NOWHERE && (real_roomp(j->in_room)->people)) {
	      act("$p begins to look a little brown.", TRUE, real_roomp(j->in_room)->people, j,
		  0, TO_CHAR);
	      act("$p begins to look a little brown.", TRUE, real_roomp(j->in_room)->people, j,
		  0, TO_ROOM);
	    }
	  }
	  break;
	case 2:
	  {
	    if (j->carried_by)
	      act("$p begins to smell funny.", FALSE, j->carried_by, j, 0, TO_CHAR);
	    else if (j->in_room != NOWHERE && (real_roomp(j->in_room)->people)) {
	      act("$p begins to smell funny.", TRUE, real_roomp(j->in_room)->people, j, 0,
		  TO_CHAR);
	      act("$p begins to smell funny.", TRUE, real_roomp(j->in_room)->people, j, 0,
		  TO_ROOM);
	    }
	  }
	  break;
	case 1:
	  {
	    j->obj_flags.value[3] = 1;			       /* poison the sucker */
	    if (j->carried_by)
	      act("$p begins to smell spoiled.", FALSE, j->carried_by, j, 0, TO_CHAR);
	    else if (j->in_room != NOWHERE && (real_roomp(j->in_room)->people)) {
	      act("$p begins to smell spoiled.", TRUE, real_roomp(j->in_room)->people, j, 0,
		  TO_CHAR);
	      act("$p begins to smell spoiled.", TRUE, real_roomp(j->in_room)->people, j, 0,
		  TO_ROOM);
	    }
	  }
	  break;
	case 0:
	  {
	    if (j->carried_by)
	      act("$p dissolves into dust...", FALSE, j->carried_by, j, 0, TO_CHAR);
	    else if ((j->in_room != NOWHERE) && (real_roomp(j->in_room)->people)) {
	      act("$p dissolves into dust...", TRUE, real_roomp(j->in_room)->people, j, 0,
		  TO_ROOM);
	      act("$p dissolves into dust...", TRUE, real_roomp(j->in_room)->people, j, 0,
		  TO_CHAR);
	    }
	    extract_obj(j);
	  }
      }
    }
    if ((GET_ITEM_TYPE(j) == ITEM_CONTAINER) && (j->obj_flags.value[3])) {
      if (j->obj_flags.timer > 0)
	j->obj_flags.timer--;
      if (!j->obj_flags.timer) {
	if (j->carried_by) {
	  act("$p biodegrades in your hands.", FALSE, j->carried_by, j, 0, TO_CHAR);
	  ObjFromCorpse(j);
	} else if (j->in_room != NOWHERE) {
	  if ((number(0, 99) < 40) && (real_roomp(j->in_room)->zone != 10)) {
	    struct char_data                       *mob;
	    struct obj_data                        *obj_object,
	                                           *next_obj;
	    int                                     mobset[] =
	      { 9002, 9001, 4616, 4615, 4613, 9003, 4603 };
	    char                                    newbuffer[256],
	                                            newtmp[256];

	    mob = read_mobile(mobset[number(0, 6)], VIRTUAL);
	    strcpy(newtmp, j->short_description);
	    if (!strncmp(newtmp, "the corpse of ", 14)) {
	      newtmp[14] = tolower(newtmp[14]);
	      sprintf(newbuffer, "%s of %s has arisen to KILL!\n\r",
		      GET_SDESC(mob), &newtmp[14]);
	    } else {
	      sprintf(newbuffer, "%s has recently risen to KILL!\n\r", GET_SDESC(mob));
	    }
	    /*
	     * this loses memory... can we free it first? 
	     */
	    if (mob->player.long_descr)
	      DESTROY(mob->player.long_descr);
	    mob->player.long_descr = strdup(newbuffer);
	    char_to_room(mob, j->in_room);
	    GET_EXP(mob) = 75 * GetMaxLevel(mob);
	    IS_CARRYING_W(mob) = 0;
	    IS_CARRYING_N(mob) = 0;
	    for (obj_object = j->contains; obj_object; obj_object = next_obj) {
	      next_obj = obj_object->next_content;
	      obj_from_obj(obj_object);
	      obj_to_char(obj_object, mob);
	    }
	    mob->points.max_hit = dice(GetMaxLevel(mob), 8) + number(20, 10 * GetMaxLevel(mob));
	    GET_HIT(mob) = GET_MAX_HIT(mob);
	    GET_EXP(mob) = dice(GetMaxLevel(mob), 10) * 10 + number(1, 10 * GetMaxLevel(mob));
	    mob->player.sex = 0;
	    GET_RACE(mob) = RACE_UNDEAD;
	    if (!IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
	      SET_BIT(mob->specials.act, ACT_AGGRESSIVE);
	    }
	    if (IS_SET(mob->specials.act, ACT_SENTINEL)) {
	      REMOVE_BIT(mob->specials.act, ACT_SENTINEL);
	    }
	    if (real_roomp(j->in_room)->people) {
	      act("$p slowly rises as $N and screams 'DIE!'", TRUE,
		  real_roomp(j->in_room)->people, j, mob, TO_ROOM);
	      act("$p slowly rises as $N and screams 'DIE!'", TRUE,
		  real_roomp(j->in_room)->people, j, mob, TO_CHAR);
	    }
	    do_wear(mob, "all", 0);
	    extract_obj(j);
	  } else if (real_roomp(j->in_room)->people) {
	    act("$p dissolves into a lump of fertile soil.", TRUE,
		real_roomp(j->in_room)->people, j, 0, TO_ROOM);
	    act("$p dissolves into a lump of fertile soil.", TRUE,
		real_roomp(j->in_room)->people, j, 0, TO_CHAR);
	    ObjFromCorpse(j);
	  } else
	    ObjFromCorpse(j);
	}
      }
    }
  }
}

int ObjFromCorpse(struct obj_data *c)
{
  struct obj_data                        *jj = NULL;
  struct obj_data                        *next_thing = NULL;

  if (DEBUG > 2)
    log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_ONAME(c));

  for (jj = c->contains; jj; jj = next_thing) {
    next_thing = jj->next_content;			       /* Next in inventory */
    if (jj->in_obj) {
      obj_from_obj(jj);
      if (c->in_obj)
	obj_to_obj(jj, c->in_obj);
      else if (c->carried_by)
	obj_to_room(jj, c->carried_by->in_room);
      else if (c->in_room != NOWHERE)
	obj_to_room(jj, c->in_room);
      else
	return (FALSE);
    } else {
      /*
       * **  hmm..  it isn't in the object it says it is in.
       * **  deal with the memory lossage
       */
      c->contains = 0;
      extract_obj(c);
      log_error("Memory lost in ObjFromCorpse.");
      return (TRUE);
    }
  }
  extract_obj(c);
  return TRUE;
}

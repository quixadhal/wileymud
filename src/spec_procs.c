/*
 * ************************************************************************
 * *  file: spec_procs.c , Special module.                   Part of DIKUMUD *
 * *  Usage: Procedures handling special procedures for object/room/mobile   *
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
#include "s_list.h"
#include "opinion.h"
#include "hash.h"

/*
 * external vars  
 */
extern struct room_data         *world;
extern struct char_data         *character_list;
extern struct descriptor_data   *descriptor_list;
extern struct index_data        *obj_index;
extern struct time_info_data     time_info;
extern struct index_data        *mob_index;
extern struct weather_data       weather_info;
extern int                       top_of_world;
extern struct int_app_type       int_app[26];

extern struct title_type         titles[6][ABS_MAX_LVL];

/*
 * extern procedures 
 */

void                             hit(struct char_data *ch, struct char_data *victim, int type);
void                             gain_exp(struct char_data *ch, int gain);
struct char_data                *FindVictim(struct char_data *ch);
struct char_data                *char_holding(struct obj_data *obj);
void                             send_to_all(char *messg);
void                             do_shout(struct char_data *ch, char *argument, int cmd);
int                              IsUndead(struct char_data *ch);
struct time_info_data            age(struct char_data *ch);
void                             say_spell(struct char_data *ch, int spl);
int                              do_move(struct char_data *ch, char *argument, int cmd);
int                              do_open(struct char_data *ch, char *argument, int cmd);
int                              do_lock(struct char_data *ch, char *argument, int cmd);
int                              do_unlock(struct char_data *ch, char *argument, int cmd);
int                              do_close(struct char_data *ch, char *argument, int cmd);
int                              go_direction(struct char_data *ch, int dir);
void                             cast_cure_light(int i1, struct char_data *ch, char *str, int i2, struct char_data *vict, int cmd);
void                             cast_energy_drain(byte level, struct char_data *ch, char *arg, int type,
		struct char_data *tar_ch, struct obj_data *tar_obj);
void                             cast_chill_touch(byte level, struct char_data *ch, char *arg, int type,
		struct char_data *tar_ch, struct obj_data *tar_obj);
void                             cast_weakness(byte level, struct char_data *ch, char *arg, int type,
		struct char_data *tar_ch, struct obj_data *tar_obj);
void                             cast_poison(byte level, struct char_data *ch, char *arg, int type,
		struct char_data *tar_ch, struct obj_data *tar_obj);

/*
 * Data declarations 
 */

struct social_type {
  char                            *cmd;
  int                              next_line;
};

/*************************************/
/*
 * predicates for find_path function 
 */

int 
is_target_room_p(int room, void *tgt_room)
{
  return room == (int)tgt_room;
}

int 
named_object_on_ground(int room, void *c_data)
{
  char                            *name = c_data;

  return NULL != get_obj_in_list(name, real_roomp(room)->contents);
}

/*
 * predicates for find_path function 
 */
/*************************************/

/*
 * ********************************************************************
 * *  Special procedures for rooms                                       *
 * ******************************************************************** 
 */

char                            *
how_good(int percent)
{
  static char                      buf[256];

  if (percent == 0)
    strcpy(buf, " (not learned)");
  else if (percent <= 6)
    strcpy(buf, " (pitifull)");
  else if (percent <= 12)
    strcpy(buf, " (awful)");
  else if (percent <= 18)
    strcpy(buf, " (incredibly bad)");
  else if (percent <= 24)
    strcpy(buf, " (very bad)");
  else if (percent <= 30)
    strcpy(buf, " (bad)");
  else if (percent <= 36)
    strcpy(buf, " (very poor)");
  else if (percent <= 42)
    strcpy(buf, " (poor)");
  else if (percent <= 48)
    strcpy(buf, " (below average)");
  else if (percent <= 54)
    strcpy(buf, " (average)");
  else if (percent <= 60)
    strcpy(buf, " (better then average)");
  else if (percent <= 66)
    strcpy(buf, " (fair)");
  else if (percent <= 72)
    strcpy(buf, " (very fair)");
  else if (percent <= 78)
    strcpy(buf, " (good)");
  else if (percent <= 84)
    strcpy(buf, " (very good)");
  else if (percent <= 90)
    strcpy(buf, " (Superb)");
  else
    strcpy(buf, " (Master)");

  return (buf);
}

int 
GainLevel(struct char_data *ch, int class)
{

  if (GET_EXP(ch) >=
      titles[class][GET_LEVEL(ch, class) + 1].exp) {

    send_to_char("You raise a level\n\r", ch);
    advance_level(ch, class);
    set_title(ch);

  } else {
    send_to_char("You haven't got enough experience!\n\r", ch);
  }
  return (FALSE);
}

struct char_data                *
FindMobInRoomWithFunction(int room, int (*func) ())
{
  struct char_data                *temp_char,
                                  *targ;

  targ = 0;

  if (room > NOWHERE) {
    for (temp_char = real_roomp(room)->people; (!targ) && (temp_char);
	 temp_char = temp_char->next_in_room)
      if (IS_MOB(temp_char))
	if (mob_index[temp_char->nr].func == func)
	  targ = temp_char;

  } else {
    return (0);
  }

  return (targ);

}

int 
MageGuildMaster(struct char_data *ch, int cmd, char *arg)
{
  int                              number,
                                   i,
                                   percent;
  char                             buf[MAX_INPUT_LENGTH];
  struct char_data                *guildmaster;
  extern char                     *spells[];
  extern struct spell_info_type    spell_info[MAX_SPL_LIST];

  if ((cmd != 164) && (cmd != 170) && (cmd != 243))
    return (FALSE);

  guildmaster = FindMobInRoomWithFunction(ch->in_room, MageGuildMaster);

  if (!guildmaster)
    return (FALSE);

  if (HasClass(ch, CLASS_MAGIC_USER)) {
    if (cmd == 243) {		/*
				 * gain 
				 */
      if (GET_LEVEL(ch, MAGE_LEVEL_IND) < GetMaxLevel(guildmaster) - 10) {
	GainLevel(ch, MAGE_LEVEL_IND);
      } else {
	send_to_char("I cannot train you.. You must find another.\n\r", ch);
      }
      return (TRUE);
    }
    for (; isspace(*arg); arg++);

    if (!*arg) {
      sprintf(buf, "You have got %d practice sessions left.\n\r",
	      ch->specials.spells_to_learn);
      send_to_char(buf, ch);
      send_to_char("You can practise any of these spells:\n\r", ch);
      for (i = 0; *spells[i] != '\n'; i++)
	if (spell_info[i + 1].spell_pointer &&
	    (spell_info[i + 1].min_level_magic <=
	     GET_LEVEL(ch, MAGE_LEVEL_IND)) &&
	    (spell_info[i + 1].min_level_magic <=
	     GetMaxLevel(guildmaster) - 10)) {

	  sprintf(buf, "[%d] %s %s \n\r",
		  spell_info[i + 1].min_level_magic,
		  spells[i], how_good(ch->skills[i + 1].learned));
	  send_to_char(buf, ch);
	}
      return (TRUE);

    }
    number = old_search_block(arg, 0, strlen(arg), spells, FALSE);
    if (number == -1) {
      send_to_char("You do not know of this spell...\n\r", ch);
      return (TRUE);
    }
    if (GET_LEVEL(ch, MAGE_LEVEL_IND) < spell_info[number].min_level_magic) {
      send_to_char("You do not know of this spell...\n\r", ch);
      return (TRUE);
    }
    if (GetMaxLevel(guildmaster) - 10 < spell_info[number].min_level_magic) {
      do_say(guildmaster, "I don't know of this spell.", 0);
      return (TRUE);
    }
    if (ch->specials.spells_to_learn <= 0) {
      send_to_char("You do not seem to be able to practice now.\n\r", ch);
      return (TRUE);
    }
    if (ch->skills[number].learned >= 95) {
      send_to_char("You are already learned in this area.\n\r", ch);
      return (TRUE);
    }
    send_to_char("You Practice for a while...\n\r", ch);
    ch->specials.spells_to_learn--;

    percent = ch->skills[number].learned + int_app[GET_INT(ch)].learn;
    ch->skills[number].learned = MIN(95, percent);

    if (ch->skills[number].learned >= 95) {
      send_to_char("You are now learned in this area.\n\r", ch);
      return (TRUE);

    }
  } else {
    send_to_char("Oh.. i bet you think you're a magic user?\n\r", ch);
    return (FALSE);
  }

}

int 
ClericGuildMaster(struct char_data *ch, int cmd, char *arg)
{
  int                              number,
                                   i,
                                   percent;
  char                             buf[MAX_INPUT_LENGTH];
  struct char_data                *guildmaster;
  extern char                     *spells[];
  extern struct spell_info_type    spell_info[MAX_SPL_LIST];

  if ((cmd != 164) && (cmd != 170) && (cmd != 243))
    return (FALSE);

  guildmaster = FindMobInRoomWithFunction(ch->in_room, ClericGuildMaster);

  if (!guildmaster)
    return (FALSE);

  if (HasClass(ch, CLASS_CLERIC)) {
    if (cmd == 243) {		/*
				 * gain 
				 */
      if (GET_LEVEL(ch, CLERIC_LEVEL_IND) < GetMaxLevel(guildmaster) - 10) {
	GainLevel(ch, CLERIC_LEVEL_IND);
      } else {
	send_to_char("I cannot train you.. You must find another.\n\r", ch);
      }
      return (TRUE);
    }
    for (; isspace(*arg); arg++);

    if (!*arg) {
      sprintf(buf, "You have got %d practice sessions left.\n\r",
	      ch->specials.spells_to_learn);
      send_to_char(buf, ch);
      send_to_char("You can practise any of these spells:\n\r", ch);
      for (i = 0; *spells[i] != '\n'; i++)
	if (spell_info[i + 1].spell_pointer &&
	    (spell_info[i + 1].min_level_cleric <=
	     GET_LEVEL(ch, CLERIC_LEVEL_IND)) &&
	    (spell_info[i + 1].min_level_cleric <=
	     GetMaxLevel(guildmaster) - 10)) {
	  sprintf(buf, "[%d] %s %s \n\r",
		  spell_info[i + 1].min_level_cleric, spells[i],
		  how_good(ch->skills[i + 1].learned));
	  send_to_char(buf, ch);
	}
      return (TRUE);
    }
    number = old_search_block(arg, 0, strlen(arg), spells, FALSE);
    if (number == -1) {
      send_to_char("You do not know of this spell...\n\r", ch);
      return (TRUE);
    }
    if (GET_LEVEL(ch, CLERIC_LEVEL_IND) < spell_info[number].min_level_cleric) {
      send_to_char("You do not know of this spell...\n\r", ch);
      return (TRUE);
    }
    if (GetMaxLevel(guildmaster) - 10 < spell_info[number].min_level_cleric) {
      do_say(guildmaster, "I don't know of this spell.", 0);
      return (TRUE);
    }
    if (ch->specials.spells_to_learn <= 0) {
      send_to_char("You do not seem to be able to practice now.\n\r", ch);
      return (TRUE);
    }
    if (ch->skills[number].learned >= 95) {
      send_to_char("You are already learned in this area.\n\r", ch);
      return (TRUE);
    }
    send_to_char("You Practice for a while...\n\r", ch);
    ch->specials.spells_to_learn--;

    percent = ch->skills[number].learned + int_app[GET_INT(ch)].learn;
    ch->skills[number].learned = MIN(95, percent);

    if (ch->skills[number].learned >= 95) {
      send_to_char("You are now learned in this area.\n\r", ch);
      return (TRUE);
    }
  } else {
    send_to_char("What do you think you are, a cleric??\n\r", ch);
    return (FALSE);
  }

}

int 
ThiefGuildMaster(struct char_data *ch, int cmd, char *arg)
{
  int                              number,
                                   i,
                                   percent;
  char                             buf[MAX_INPUT_LENGTH];
  struct char_data                *guildmaster;

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
/*
 * {"listen",           SKILL_DETECT_NOISE}, 
 */
    {"find_traps", SKILL_FIND_TRAP},
/*
 * {"disarm_traps",     SKILL_DISARM_TRAP},             
 */
    {"\n", -1}
  };

  static char                     *tl_skills[] =
  {
    "sneak",			/*
				 * No. 45 
				 */
    "hide",
    "steal",
    "backstab",
    "pick",
    "track",
    "search",
    "peer",
/*
 * "listen",   
 */
    "find_traps",
/*
 * "disarm_traps", 
 */
    "\n"
  };

  if ((cmd != 164) && (cmd != 170) && (cmd != 243))
    return (FALSE);

  guildmaster = FindMobInRoomWithFunction(ch->in_room, ThiefGuildMaster);

  if (!guildmaster)
    return (FALSE);

  if (HasClass(ch, CLASS_THIEF)) {
    if (cmd == 243) {		/*
				 * gain 
				 */
      if (GET_LEVEL(ch, THIEF_LEVEL_IND) < GetMaxLevel(guildmaster) - 10) {
	GainLevel(ch, THIEF_LEVEL_IND);
      } else {
	send_to_char("I cannot train you.. You must find another.\n\r", ch);
      }
      return (TRUE);
    }
    if (!*arg) {
      sprintf(buf, "You have got %d practice sessions left.\n\r",
	      ch->specials.spells_to_learn);
      send_to_char(buf, ch);
      send_to_char("You can practise any of these skills:\n\r", ch);
      for (i = 0; t_skills[i].skill_name[0] != '\n'; i++) {
	send_to_char(t_skills[i].skill_name, ch);
	send_to_char(how_good(ch->skills[t_skills[i].skill_numb].learned), ch);
	send_to_char("\n\r", ch);
      }
      return (TRUE);
    }
    for (; isspace(*arg); arg++);
    number = search_block(arg, tl_skills, FALSE);
    if (number == -1) {
      send_to_char("You do not know of this skill...\n\r", ch);
      return (TRUE);
    }
    if (ch->specials.spells_to_learn <= 0) {
      send_to_char("You do not seem to be able to practice now.\n\r", ch);
      return (TRUE);
    }
    if (number != -1) {
      if (ch->skills[t_skills[number].skill_numb].learned >= 95) {
	send_to_char("You are already learned in this area.\n\r", ch);
	return (TRUE);
      }
    }
    send_to_char("You Practice for a while...\n\r", ch);
    ch->specials.spells_to_learn--;

    percent = ch->skills[t_skills[number].skill_numb].learned +
      int_app[GET_INT(ch)].learn;
    ch->skills[t_skills[number].skill_numb].learned = MIN(95, percent);

    if (ch->skills[t_skills[number].skill_numb].learned >= 95) {
      send_to_char("You are now learned in this area.\n\r", ch);
      return (TRUE);
    }
  } else {
    send_to_char("What do you think you are, a thief??\n\r", ch);
    return (FALSE);
  }
}

int 
FighterGuildMaster(struct char_data *ch, int cmd, char *arg)
{

  int                              number,
                                   i,
                                   percent;
  char                             buf[MAX_INPUT_LENGTH];
  struct char_data                *guildmaster;

  struct skill_struct {
    char                             skill_name[40];
    int                              skill_numb;
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

  char                            *wl_skills[] =
  {
    "kick",
    "bash",
    "rescue",
    "disarm",
    "punch",
    "doorbash",
    "smite spec",
    "stab spec",
    "whip spec",
    "slash spec",
    "smash spec",
    "cleave spec",
    "crush spec",
    "bludge spec",
    "peirce spec",
    "\n"
  };

  if ((cmd != 164) && (cmd != 170) && (cmd != 243))
    return (FALSE);

  guildmaster = FindMobInRoomWithFunction(ch->in_room, FighterGuildMaster);

  if (!guildmaster)
    return (FALSE);

  if (HasClass(ch, CLASS_WARRIOR)) {
    if (cmd == 243) {		/*
				 * gain 
				 */
      if (GET_LEVEL(ch, WARRIOR_LEVEL_IND) < GetMaxLevel(guildmaster) - 10) {
	GainLevel(ch, WARRIOR_LEVEL_IND);
      } else {
	send_to_char("I cannot train you.. You must find another.\n\r", ch);
      }
      return (TRUE);
    }
    if (!*arg) {
      sprintf(buf, "You have got %d practice sessions left.\n\r",
	      ch->specials.spells_to_learn);
      send_to_char(buf, ch);
      send_to_char("You can practise any of these skills:\n\r", ch);
      for (i = 0; wa_skills[i].skill_name[0] != '\n'; i++) {
	send_to_char(wa_skills[i].skill_name, ch);
	send_to_char(how_good(ch->skills[wa_skills[i].skill_numb].learned), ch);
	send_to_char("\n\r", ch);
      }
      return (TRUE);
    }
    for (; isspace(*arg); arg++);
    number = search_block(arg, wl_skills, FALSE);
    if (number == -1) {
      send_to_char("You do not have ability to practise this skill!\n\r", ch);
      return (TRUE);
    }
    if (ch->specials.spells_to_learn <= 0) {
      send_to_char("You do not seem to be able to practice now.\n\r", ch);
      return (TRUE);
    }
    if (ch->skills[wa_skills[number].skill_numb].learned >= 95) {
      send_to_char("You are already learned in this area.\n\r", ch);
      return (TRUE);
    }
    send_to_char("You Practice for a while...\n\r", ch);
    ch->specials.spells_to_learn--;
    percent = ch->skills[wa_skills[number].skill_numb].learned +
      int_app[GET_INT(ch)].learn;
    ch->skills[wa_skills[number].skill_numb].learned = MIN(95, percent);

    if (ch->skills[wa_skills[number].skill_numb].learned >= 95) {
      send_to_char("You are now learned in this area.\n\r", ch);
      return (TRUE);
    }
  } else {
    send_to_char("Oh.. i bet you think you're a fighter??\n\r", ch);
    return (FALSE);
  }
}

int 
dump(struct char_data *ch, int cmd, char *arg)
{
  struct obj_data                 *k;
  char                             buf[100];
  struct char_data                *tmp_char;
  int                              value = 0;

  void                             do_drop(struct char_data *ch, char *argument, int cmd);
  char                            *fname(char *namelist);

  for (k = real_roomp(ch->in_room)->contents; k; k = real_roomp(ch->in_room)->contents) {
    sprintf(buf, "The %s vanishes in a puff of smoke.\n\r", fname(k->name));
    for (tmp_char = real_roomp(ch->in_room)->people; tmp_char; tmp_char = tmp_char->next_in_room)
      if (CAN_SEE_OBJ(tmp_char, k))
	send_to_char(buf, tmp_char);
    extract_obj(k);
  }

  if (cmd != 60)
    return (FALSE);

  do_drop(ch, arg, cmd);

  value = 0;

  for (k = real_roomp(ch->in_room)->contents; k; k = real_roomp(ch->in_room)->contents) {
    sprintf(buf, "The %s vanish in a puff of smoke.\n\r", fname(k->name));
    for (tmp_char = real_roomp(ch->in_room)->people; tmp_char;
	 tmp_char = tmp_char->next_in_room)
      if (CAN_SEE_OBJ(tmp_char, k))
	send_to_char(buf, tmp_char);
    value += (MIN(1000, MAX(k->obj_flags.cost / 4, 1)));
    /*
     * value += MAX(1, MIN(50, k->obj_flags.cost/10));
     */
    extract_obj(k);
  }

  if (value) {
    act("You are awarded for outstanding performance.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n has been awarded for being a good citizen.", TRUE, ch, 0, 0, TO_ROOM);

    if (GetMaxLevel(ch) < 3)
      gain_exp(ch, MIN(100, value));
    else
      GET_GOLD(ch) += value;
  }
}

#ifdef 0

int 
mayor(struct char_data *ch, int cmd, char *arg)
{
  static char                      open_path[] =
  "W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";

  static char                      close_path[] =
  "W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

  static char                     *path;
  static int                       index;
  static bool                      move = FALSE;

  void                             do_move(struct char_data *ch, char *argument, int cmd);
  void                             do_open(struct char_data *ch, char *argument, int cmd);
  void                             do_lock(struct char_data *ch, char *argument, int cmd);
  void                             do_unlock(struct char_data *ch, char *argument, int cmd);
  void                             do_close(struct char_data *ch, char *argument, int cmd);

  if (!move) {
    if (time_info.hours == 6) {
      move = TRUE;
      path = open_path;
      index = 0;
    } else if (time_info.hours == 20) {
      move = TRUE;
      path = close_path;
      index = 0;
    }
  }
  if (cmd || !move || (GET_POS(ch) < POSITION_SLEEPING) ||
      (GET_POS(ch) == POSITION_FIGHTING))
    return FALSE;

  switch (path[index]) {
  case '0':
  case '1':
  case '2':
  case '3':
    do_move(ch, "", path[index] - '0' + 1);
    break;

  case 'W':
    GET_POS(ch) = POSITION_STANDING;
    act("$n awakens and groans loudly.", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'S':
    GET_POS(ch) = POSITION_SLEEPING;
    act("$n lies down and instantly falls asleep.", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'a':
    act("$n says 'Hello Honey!'", FALSE, ch, 0, 0, TO_ROOM);
    act("$n smirks.", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'b':
    act("$n says 'What a view! I must get something done about that dump!'",
	FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'c':
    act("$n says 'Vandals! Youngsters nowadays have no respect for anything!'",
	FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'd':
    act("$n says 'Good day, citizens!'", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'e':
    act("$n says 'I hereby declare the bazaar open!'", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'E':
    act("$n says 'I hereby declare Midgaard closed!'", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'O':
    do_unlock(ch, "gate", 0);
    do_open(ch, "gate", 0);
    break;

  case 'C':
    do_close(ch, "gate", 0);
    do_lock(ch, "gate", 0);
    break;

  case '.':
    move = FALSE;
    break;

  }

  index++;
  return FALSE;
}

#endif

struct char_data                *
find_mobile_here_with_spec_proc(int (*fcn) (), int rnumber)
{
  struct char_data                *temp_char;

  for (temp_char = real_roomp(rnumber)->people; temp_char;
       temp_char = temp_char->next_in_room)
    if (IS_MOB(temp_char) &&
	mob_index[temp_char->nr].func == fcn)
      return temp_char;
  return NULL;
}

/*
 * ********************************************************************
 * *  General special procedures for mobiles                                      *
 * ******************************************************************** 
 */

/*
 * SOCIAL GENERAL PROCEDURES
 * 
 * If first letter of the command is '!' this will mean that the following
 * command will be executed immediately.
 * 
 * "G",n      : Sets next line to n
 * "g",n      : Sets next line relative to n, fx. line+=n
 * "m<dir>",n : move to <dir>, <dir> is 0,1,2,3,4 or 5
 * "w",n      : Wake up and set standing (if possible)
 * "c<txt>",n : Look for a person named <txt> in the room
 * "o<txt>",n : Look for an object named <txt> in the room
 * "r<int>",n : Test if the npc in room number <int>?
 * "s",n      : Go to sleep, return false if can't go sleep
 * "e<txt>",n : echo <txt> to the room, can use $o/$p/$N depending on
 * contents of the **thing
 * "E<txt>",n : Send <txt> to person pointed to by thing
 * "B<txt>",n : Send <txt> to room, except to thing
 * "?<num>",n : <num> in [1..99]. A random chance of <num>% success rate.
 * Will as usual advance one line upon sucess, and change
 * relative n lines upon failure.
 * "O<txt>",n : Open <txt> if in sight.
 * "C<txt>",n : Close <txt> if in sight.
 * "L<txt>",n : Lock <txt> if in sight.
 * "U<txt>",n : Unlock <txt> if in sight.    
 */

/*
 * Execute a social command.                                        
 */

#ifdef 0

void 
exec_social(struct char_data *npc, char *cmd, int next_line,
	    int *cur_line, void **thing)
{
  bool                             ok;

  if (GET_POS(npc) == POSITION_FIGHTING)
    return;

  ok = TRUE;

  switch (*cmd) {

  case 'G':
    *cur_line = next_line;
    return;

  case 'g':
    *cur_line += next_line;
    return;

  case 'e':
    act(cmd + 1, FALSE, npc, *thing, *thing, TO_ROOM);
    break;

  case 'E':
    act(cmd + 1, FALSE, npc, 0, *thing, TO_VICT);
    break;

  case 'B':
    act(cmd + 1, FALSE, npc, 0, *thing, TO_NOTVICT);
    break;

  case 'm':
    do_move(npc, "", *(cmd + 1) - '0' + 1);
    break;

  case 'w':
    if (GET_POS(npc) != POSITION_SLEEPING)
      ok = FALSE;
    else
      GET_POS(npc) = POSITION_STANDING;
    break;

  case 's':
    if (GET_POS(npc) <= POSITION_SLEEPING)
      ok = FALSE;
    else
      GET_POS(npc) = POSITION_SLEEPING;
    break;

  case 'c':			/*
				 * Find char in room 
				 */
    *thing = get_char_room_vis(npc, cmd + 1);
    ok = (*thing != 0);
    break;

  case 'o':			/*
				 * Find object in room 
				 */
    *thing = get_obj_in_list_vis(npc, cmd + 1, real_roomp(npc->in_room)->contents);
    ok = (*thing != 0);
    break;

  case 'r':			/*
				 * Test if in a certain room 
				 */
    ok = (npc->in_room == atoi(cmd + 1));
    break;

  case 'O':			/*
				 * Open something 
				 */
    do_open(npc, cmd + 1, 0);
    break;

  case 'C':			/*
				 * Close something 
				 */
    do_close(npc, cmd + 1, 0);
    break;

  case 'L':			/*
				 * Lock something  
				 */
    do_lock(npc, cmd + 1, 0);
    break;

  case 'U':			/*
				 * UnLock something  
				 */
    do_unlock(npc, cmd + 1, 0);
    break;

  case '?':			/*
				 * Test a random number 
				 */
    if (atoi(cmd + 1) <= number(1, 100))
      ok = FALSE;
    break;

  default:
    break;
  }				/*
				 * End Switch 
				 */

  if (ok)
    (*cur_line)++;
  else
    (*cur_line) += next_line;
}

#endif

void 
npc_steal(struct char_data *ch, struct char_data *victim)
{
  int                              gold;

  if (IS_NPC(victim))
    return;
  if (GetMaxLevel(victim) > MAX_MORT)
    return;

  if (AWAKE(victim) && (number(0, GetMaxLevel(ch)) == 0)) {
    act("You discover that $n has $s hands in your wallet.", FALSE, ch, 0, victim, TO_VICT);
    act("$n tries to steal gold from $N.", TRUE, ch, 0, victim, TO_NOTVICT);
  } else {
    /*
     * Steal some gold coins 
     */
    gold = (int)((GET_GOLD(victim) * number(1, 10)) / 100);
    if (gold > 0) {
      GET_GOLD(ch) += gold;
      GET_GOLD(victim) -= gold;
    }
  }
}

int 
snake(struct char_data *ch, int cmd, char *arg)
{

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (GET_POS(ch) != POSITION_FIGHTING)
    return FALSE;

  if (ch->specials.fighting &&
      (ch->specials.fighting->in_room == ch->in_room)) {
    act("$n poisons $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
    act("$n poisons you!", 1, ch, 0, ch->specials.fighting, TO_VICT);
    cast_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
		ch->specials.fighting, 0);
    return TRUE;
  }
  return FALSE;
}

#ifdef 0

int 
ninja_master(struct char_data *ch, int cmd, char *arg)
{
  char                             buf[256];
  static char                     *n_skills[] =
  {
    "track",			/*
				 * No. 180 
				 */
    "disarm",			/*
				 * No. 245 
				 */
    "\n",
  };
  int                              percent = 0,
                                   number = 0;
  int                              charge,
                                   sk_num,
                                   mult;

  if (!AWAKE(ch))
    return (FALSE);

  for (; *arg == ' '; arg++);	/*
				 * ditch spaces 
				 */

  if ((cmd == 164) || (cmd == 170)) {
    /*
     * So far, just track 
     */
    if (!arg || (strlen(arg) == 0)) {
      sprintf(buf, " track:   %s\n\r", how_good(ch->skills[SKILL_HUNT].learned));
      send_to_char(buf, ch);
      sprintf(buf, " disarm:  %s\n\r", how_good(ch->skills[SKILL_DISARM].learned));
      send_to_char(buf, ch);
      return (TRUE);
    } else {
      number = old_search_block(arg, 0, strlen(arg), n_skills, FALSE);
      send_to_char("The ninja master says ", ch);
      if (number == -1) {
	send_to_char("'I do not know of this skill.'\n\r", ch);
	return (TRUE);
      }
      charge = GetMaxLevel(ch) * 100;
      switch (number) {
      case 0:
      case 1:
	sk_num = SKILL_HUNT;
	break;
      case 2:
	sk_num = SKILL_DISARM;
	mult = 1;
	if (HasClass(ch, CLASS_MAGIC_USER))
	  mult = 4;
	if (HasClass(ch, CLASS_CLERIC))
	  mult = 3;
	if (HasClass(ch, CLASS_THIEF))
	  mult = 2;
	if (HasClass(ch, CLASS_RANGER))
	  mult = 0;
	if (HasClass(ch, CLASS_DRUID))
	  mult = 1;
	if (HasClass(ch, CLASS_WARRIOR))
	  mult = 1;
	charge *= mult;

	break;
      default:
	sprintf(buf, "Strangeness in ninjamaster (%d)", number);
	log(buf);
	return;
      }
    }
    if (GET_GOLD(ch) < charge) {
      send_to_char
	("'Ah, but you do not have enough money to pay.'\n\r", ch);
      return (FALSE);
    }
    if (ch->skills[sk_num].learned >= 95) {
      send_to_char
	("'You are a master of this art, I can teach you no more.'\n\r", ch);
      return (FALSE);
    }
    if (ch->specials.spells_to_learn <= 0) {
      send_to_char
	("'You must first use the knowledge you already have.\n\r", ch);
      return (FALSE);
    }
    GET_GOLD(ch) -= charge;
    send_to_char("'We will now begin.'\n\r", ch);
    ch->specials.spells_to_learn--;

    percent = ch->skills[sk_num].learned +
      int_app[GET_INT(ch)].learn;
    ch->skills[sk_num].learned = MIN(95, percent);

    if (ch->skills[sk_num].learned >= 95) {
      send_to_char("'You are now a master of this art.'\n\r", ch);
      return (TRUE);
    }
  } else {
    return (FALSE);
  }
}

#define PGShield 25100

int 
PaladinGuildGuard(struct char_data *ch, int cmd, char *arg)
{

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (!cmd) {
    if (ch->specials.fighting) {
      if (GET_POS(ch) == POSITION_FIGHTING) {
	FighterMove(ch);
      } else {
	StandUp(ch);
      }
    }
  } else if (cmd >= 1 && cmd <= 6) {
    if (cmd == 4)
      return (FALSE);		/*
				 * can always go west 
				 */
    if (!HasObject(ch, PGShield)) {
      send_to_char
	("The guard shakes his head, and blocks your way.\n\r", ch);
      act("The guard shakes his head, and blocks $n's way.",
	  TRUE, ch, 0, 0, TO_ROOM);
      return (TRUE);
    }
  }
  return (FALSE);
}

int 
AbyssGateKeeper(struct char_data *ch, int cmd, char *arg)
{

  if (cmd || !AWAKE(ch))
    return (FALSE);
  if (!cmd) {
    if (ch->specials.fighting) {
      if (GET_POS(ch) == POSITION_FIGHTING) {
	FighterMove(ch);
      } else {
	StandUp(ch);
      }
    }
  } else if ((cmd >= 1 && cmd <= 6) && (!IS_IMMORTAL(ch))) {
    if ((cmd == 6) || (cmd == 1)) {
      send_to_char
	("The gatekeeper shakes his head, and blocks your way.\n\r", ch);
      act("The guard shakes his head, and blocks $n's way.",
	  TRUE, ch, 0, 0, TO_ROOM);
      return (TRUE);
    }
  }
  return (FALSE);
}

#endif

int 
blink(struct char_data *ch, int cmd, char *arg)
{
  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (GET_HIT(ch) < (int)GET_MAX_HIT(ch) / 3) {
    act("$n blinks.", TRUE, ch, 0, 0, TO_ROOM);
    cast_teleport(12, ch, "", SPELL_TYPE_SPELL, ch, 0);
    return (TRUE);
  } else {
    return (FALSE);
  }
}

#define NUT_NUMBER 1130
#define NUT_CRACKED_NUMBER 1131

int 
Ned_Nutsmith(struct char_data *ch, int cmd, char *arg)
{
  char                             obj_name[80],
                                   vict_name[80],
                                   buf[MAX_INPUT_LENGTH];
  int                              cost,
                                   ave;
  struct char_data                *vict;
  struct obj_data                 *obj;
  struct obj_data                 *new_obj;
  int                              (*neddy) ();		/*

							 * special procedure for this mob/obj       
							 */
  int                              Obj;

  if (!AWAKE(ch))
    return (FALSE);

  neddy = Ned_Nutsmith;
  cost = 2500;
  if (cmd == 72) {		/*
				 * give 
				 */
    arg = one_argument(arg, obj_name);
    if (!*obj_name) {
      send_to_char("Give what?\n\r", ch);
      return (FALSE);
    }
    if (!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying))) {
      send_to_char("Give what?\n\r", ch);
      return (FALSE);
    }
    arg = one_argument(arg, vict_name);
    if (!*vict_name) {
      send_to_char("To who?\n\r", ch);
      return (FALSE);
    }
    if (!(vict = get_char_room_vis(ch, vict_name))) {
      send_to_char("To who?\n\r", ch);
      return (FALSE);
    }
    /*
     * the target is the repairman, or an NPC 
     */

    if (IS_PC(vict))
      return (FALSE);

    if (mob_index[vict->nr].func == neddy) {
      act("You give $p to $N.", TRUE, ch, obj, vict, TO_CHAR);
      act("$n gives $p to $N.", TRUE, ch, obj, vict, TO_ROOM);
    } else {
      return (FALSE);
    }

    act("$N looks at $p.", TRUE, ch, obj, vict, TO_CHAR);
    act("$N looks at $p.", TRUE, ch, obj, vict, TO_ROOM);

    if (ITEM_TYPE(obj) == ITEM_FOOD) {
      act("$N says 'Hmm, let me see here.'", TRUE, ch, 0, vict, TO_ROOM);
      act("$N says 'Hmm, let me see here.'", TRUE, ch, 0, vict, TO_CHAR);
      Obj = ObjVnum(obj);
      if (Obj != NUT_NUMBER) {
	act("$N says 'Sorry, can't help you!", TRUE, ch, obj, vict, TO_ROOM);
	act("$N says 'Sorry, can't help you!", TRUE, ch, obj, vict, TO_CHAR);
	act("$N gives the $p back.", TRUE, ch, obj, vict, TO_ROOM);
	act("$N gives the $p back.", TRUE, ch, obj, vict, TO_CHAR);
	return (TRUE);
      }
      if (GET_GOLD(ch) < cost) {
	act("$N says 'HEY, you trying to stiff me!", TRUE, ch, obj, vict, TO_ROOM);
	act("$N says 'HEY, you trying to stiff me!", TRUE, ch, obj, vict, TO_CHAR);
	act("$N says 'It will cost YOU 2500 gold to have me to that!",
	    TRUE, ch, obj, vict, TO_ROOM);
	act("$N says 'It will cost YOU 2500 gold to have me to that!",
	    TRUE, ch, obj, vict, TO_CHAR);
	return;
      }
      GET_GOLD(ch) -= cost;
      sprintf(buf, "You give $N %d coins.", cost);
      act(buf, TRUE, ch, 0, vict, TO_CHAR);
      act("$n gives some money to $N.", TRUE, ch, obj, vict, TO_ROOM);
      extract_obj(obj);
      act("$N fiddles with $p.", TRUE, ch, obj, vict, TO_ROOM);
      act("$N fiddles with $p.", TRUE, ch, obj, vict, TO_CHAR);
      act("$N says 'Well, well, well, a mighty nice nut you have here.'",
	  TRUE, ch, 0, vict, TO_ROOM);
      act("$N says 'Well, well, well, a mighty nice nut you have here.'",
	  TRUE, ch, 0, vict, TO_CHAR);
      act(" KRACK ", TRUE, ch, obj, vict, TO_CHAR);
      act(" KRACK ", TRUE, ch, obj, vict, TO_ROOM);

      /*
       * load new nut here 
       */
      new_obj = read_object(NUT_CRACKED_NUMBER, VIRTUAL);
      obj_to_char(new_obj, ch);

      act("$N gives you $p.", TRUE, ch, new_obj, vict, TO_CHAR);
      act("$N gives $p to $n.", TRUE, ch, new_obj, vict, TO_ROOM);
      return (TRUE);
    }
  } else {
    if (cmd)
      return FALSE;
    return (citizen(ch, cmd, arg));
  }
}

int 
RepairGuy(struct char_data *ch, int cmd, char *arg)
{
  char                             obj_name[80],
                                   vict_name[80],
                                   buf[MAX_INPUT_LENGTH];
  int                              cost,
                                   ave;
  struct char_data                *vict;
  struct obj_data                 *obj;
  int                              (*rep_guy) ();	/*

							 * special procedure for this mob/obj       
							 */

  if (!AWAKE(ch))
    return (FALSE);

  rep_guy = RepairGuy;

  if (cmd == 72) {		/*
				 * give 
				 */
    /*
     * determine the correct obj 
     */
    arg = one_argument(arg, obj_name);
    if (!*obj_name) {
      send_to_char("Give what?\n\r", ch);
      return (FALSE);
    }
    if (!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying))) {
      send_to_char("Give what?\n\r", ch);
      return (FALSE);
    }
    arg = one_argument(arg, vict_name);
    if (!*vict_name) {
      send_to_char("To who?\n\r", ch);
      return (FALSE);
    }
    if (!(vict = get_char_room_vis(ch, vict_name))) {
      send_to_char("To who?\n\r", ch);
      return (FALSE);
    }
    /*
     * the target is the repairman, or an NPC 
     */
    if (!IS_NPC(vict))
      return (FALSE);
    if (mob_index[vict->nr].func == rep_guy) {
      /*
       * we have the repair guy, and we can give him the stuff 
       */
      act("You give $p to $N.", TRUE, ch, obj, vict, TO_CHAR);
      act("$n gives $p to $N.", TRUE, ch, obj, vict, TO_ROOM);
    } else {
      return (FALSE);
    }

    act("$N looks at $p.", TRUE, ch, obj, vict, TO_CHAR);
    act("$N looks at $p.", TRUE, ch, obj, vict, TO_ROOM);

    /*
     * make all the correct tests to make sure that everything is kosher 
     */

    if (ITEM_TYPE(obj) == ITEM_ARMOR) {
      if ((obj->obj_flags.value[1] == 0) &&
	  (obj->obj_flags.value[0] != 0)) {
	obj->obj_flags.value[1] = obj->obj_flags.value[0];
	act("$N says 'Well, I can't fix it, but I can make it last a little longer.'",
	    TRUE, ch, 0, vict, TO_ROOM);
	act("$N says 'Well, I can't fix it, but I can make it last a little longer.'",
	    TRUE, ch, 0, vict, TO_CHAR);
	act("$N says 'That armor is old, you might want to get some new stuff I can fix later on.'",
	    TRUE, ch, 0, vict, TO_ROOM);
	act("$N says 'That armor is old, you might want to get some new stuff I can fix later on.'",
	    TRUE, ch, 0, vict, TO_CHAR);
      }
      if (obj->obj_flags.value[1] > obj->obj_flags.value[0]) {
	/*
	 * get the value of the object 
	 */
	cost = obj->obj_flags.cost;
	/*
	 * divide by value[1]   
	 */
	cost /= obj->obj_flags.value[1];
	/*
	 * then cost = difference between value[0] and [1] 
	 */
	cost *= (obj->obj_flags.value[1] - obj->obj_flags.value[0]);
	if (GetMaxLevel(vict) > 25)	/*
					 * super repair guy 
					 */
	  cost *= 2;
	if (cost > GET_GOLD(ch)) {
	  act("$N says 'I'm sorry, you don't have enough money.'",
	      TRUE, ch, 0, vict, TO_ROOM);
	  act("$N says 'I'm sorry, you don't have enough money.'",
	      TRUE, ch, 0, vict, TO_CHAR);
	} else {
	  GET_GOLD(ch) -= cost;

	  sprintf(buf, "You give $N %d coins.", cost);
	  act(buf, TRUE, ch, 0, vict, TO_CHAR);
	  act("$n gives some money to $N.", TRUE, ch, obj, vict, TO_ROOM);

	  /*
	   * fix the armor 
	   */
	  act("$N fiddles with $p.", TRUE, ch, obj, vict, TO_ROOM);
	  act("$N fiddles with $p.", TRUE, ch, obj, vict, TO_CHAR);
	  if (GetMaxLevel(vict) > 25) {
	    obj->obj_flags.value[0] = obj->obj_flags.value[1];
	  } else {
	    ave = MAX(obj->obj_flags.value[0],
		      (obj->obj_flags.value[0] +
		       obj->obj_flags.value[1]) / 2);
	    obj->obj_flags.value[0] = ave;
	    obj->obj_flags.value[1] = ave;
	  }
	  act("$N says 'All fixed.'", TRUE, ch, 0, vict, TO_ROOM);
	  act("$N says 'All fixed.'", TRUE, ch, 0, vict, TO_CHAR);
	}
      } else {
	act("$N says 'Your armor looks fine to me.'", TRUE, ch, 0, vict, TO_ROOM);
	act("$N says 'Your armor looks fine to me.'", TRUE, ch, 0, vict, TO_CHAR);
      }
    } else {
      act("$N says 'That isn't armor.'", TRUE, ch, 0, vict, TO_ROOM);
      act("$N says 'That isn't armor.'", TRUE, ch, 0, vict, TO_CHAR);
    }

    act("$N gives you $p.", TRUE, ch, obj, vict, TO_CHAR);
    act("$N gives $p to $n.", TRUE, ch, obj, vict, TO_ROOM);
    return (TRUE);
  } else {
    if (cmd)
      return FALSE;
    return (citizen(ch, cmd, arg));
  }
}

int 
citizen(struct char_data *ch, int cmd, char *arg)
{
  int                              lev;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  lev = 3;

  if (ch->specials.fighting) {
    if (GET_POS(ch) == POSITION_FIGHTING) {
      FighterMove(ch);
    } else {
      StandUp(ch);
    }

    if (!number(0, 18)) {
      do_shout(ch, "Guards! Help me! Please!", 0);
    } else {
      act("$n shouts 'Guards!  Help me! Please!'", TRUE, ch, 0, 0, TO_ROOM);
    }

    if (ch->specials.fighting)
      CallForGuard(ch, ch->specials.fighting, 3);
    return (TRUE);
  } else {
    return (FALSE);
  }
}

int 
ghoul(struct char_data *ch, int cmd, char *arg)
{
  struct char_data                *tar;

  void                             cast_paralyze(byte level, struct char_data *ch, char *arg, int type,
		struct char_data *tar_ch, struct obj_data *tar_obj);

  if (cmd || !AWAKE(ch))
    return (FALSE);

  tar = ch->specials.fighting;
  if (tar && (tar->in_room == ch->in_room)) {
    if ((!IS_AFFECTED(tar, AFF_PROTECT_EVIL)) && (!IS_AFFECTED(tar, AFF_SANCTUARY))) {
      act("$n touches $N!", 1, ch, 0, tar, TO_NOTVICT);
      act("$n touches you!", 1, ch, 0, tar, TO_VICT);
      if (!IS_AFFECTED(tar, AFF_PARALYSIS)) {
	cast_paralyze(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, tar, 0);
	return TRUE;
      }
    }
  }
  return FALSE;
}

#ifdef 0

int 
WizardGuard(struct char_data *ch, int cmd, char *arg)
{
  struct char_data                *tch,
                                  *evil;
  int                              max_evil;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting) {
    if (GET_POS(ch) == POSITION_FIGHTING) {
      FighterMove(ch);
    } else {
      StandUp(ch);
    }
    CallForGuard(ch, ch->specials.fighting, 9);
  }
  max_evil = 1000;
  evil = 0;

  for (tch = real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room) {
    if (tch->specials.fighting) {
      if ((GET_ALIGNMENT(tch) < max_evil) &&
	  (IS_NPC(tch) || IS_NPC(tch->specials.fighting))) {
	max_evil = GET_ALIGNMENT(tch);
	evil = tch;
      }
    }
  }

  if (evil && (GET_ALIGNMENT(evil->specials.fighting) >= 0) &&
      !check_peaceful(ch, "")) {
    act("$n screams 'DEATH!!!!!!!!'",
	FALSE, ch, 0, 0, TO_ROOM);
    hit(ch, evil, TYPE_UNDEFINED);
    return (TRUE);
  }
  return (FALSE);
}

#endif

int 
vampire(struct char_data *ch, int cmd, char *arg)
{
  void                             cast_energy_drain(byte level, struct char_data *ch, char *arg, int type,
		struct char_data *tar_ch, struct obj_data *tar_obj);

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting && (ch->specials.fighting->in_room == ch->in_room)) {
    act("$n touches $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
    act("$n touches you!", 1, ch, 0, ch->specials.fighting, TO_VICT);
    cast_energy_drain(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch->specials.fighting, 0);
    if (ch->specials.fighting && (ch->specials.fighting->in_room == ch->in_room)) {
      cast_energy_drain(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch->specials.fighting, 0);
    }
    return TRUE;
  }
  return FALSE;
}

int 
wraith(struct char_data *ch, int cmd, char *arg)
{
  void                             cast_energy_drain(byte level, struct char_data *ch, char *arg, int type,
		struct char_data *tar_ch, struct obj_data *tar_obj);

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting && (ch->specials.fighting->in_room == ch->in_room)) {
    act("$n touches $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
    act("$n touches you!", 1, ch, 0, ch->specials.fighting, TO_VICT);
    cast_energy_drain(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch->specials.fighting, 0);
    return TRUE;
  }
  return FALSE;
}

int 
shadow(struct char_data *ch, int cmd, char *arg)
{

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting && (ch->specials.fighting->in_room == ch->in_room)) {
    act("$n touches $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
    act("$n touches you!", 1, ch, 0, ch->specials.fighting, TO_VICT);
    cast_chill_touch(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch->specials.fighting, 0);
    if (ch->specials.fighting) {
      cast_weakness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch->specials.fighting, 0);
      if (number(1, 5) == 3)
	cast_energy_drain(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch->specials.fighting, 0);
    }
    return TRUE;
  }
  return FALSE;
}

#ifdef 0

int 
geyser(struct char_data *ch, int cmd, char *arg)
{

  void                             cast_geyser(byte level, struct char_data *ch, char *arg, int type,
		struct char_data *tar_ch, struct obj_data *tar_obj);

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (number(0, 3) == 0) {
    act("You erupt.", 1, ch, 0, 0, TO_CHAR);
    cast_geyser(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, 0, 0);
    return (TRUE);
  }
}

int 
green_slime(struct char_data *ch, int cmd, char *arg)
{
  struct char_data                *cons;

  void                             cast_green_slime(byte level, struct char_data *ch, char *arg, int type,
		struct char_data *tar_ch, struct obj_data *tar_obj);

  if (cmd || !AWAKE(ch))
    return (FALSE);

  for (cons = real_roomp(ch->in_room)->people; cons; cons = cons->next_in_room)
    if ((!IS_NPC(cons)) && (GetMaxLevel(cons) < LOW_IMMORTAL))
      cast_green_slime(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, cons, 0);

}

#endif

struct breath_victim {
  struct char_data                *ch;
  int                              yesno;	/*
						 * 1 0 
						 */
  struct breath_victim            *next;
};

struct breath_victim            *
choose_victims(struct char_data *ch,
	       struct char_data *first_victim)
{
  /*
   * this is goofy, dopey extraordinaire 
   */
  struct char_data                *cons;
  struct breath_victim            *head = NULL,
                                  *temp = NULL;

  for (cons = real_roomp(ch->in_room)->people; cons; cons = cons->next_in_room) {
    temp = (void *)malloc(sizeof(*temp));
    temp->ch = cons;
    temp->next = head;
    head = temp;
    if (first_victim == cons) {
      temp->yesno = 1;
    } else if (ch == cons) {
      temp->yesno = 0;
    } else if ((in_group(first_victim, cons) ||
		cons == first_victim->master ||
		cons->master == first_victim) &&
	       (temp->yesno = (3 != dice(1, 5)))) {
      /*
       * group members will get hit 4/5 times 
       */
    } else if (cons->specials.fighting == ch) {
      /*
       * people fighting the dragon get hit 4/5 times 
       */
      temp->yesno = (3 != dice(1, 5));
    } else			/*
				 * bystanders get his 2/5 times 
				 */
      temp->yesno = (dice(1, 5) < 3);
  }
  return head;
}

void 
free_victims(struct breath_victim *head)
{
  struct breath_victim            *temp;

  while (head) {
    temp = head->next;
    free(head);
    head = temp;
  }
}

int 
breath_weapon(struct char_data *ch, struct char_data *target,
	      int mana_cost, void              (*func) ())
{
  struct breath_victim            *hitlist,
                                  *scan;
  int                              victim;

  hitlist = choose_victims(ch, target);

  act("$n rears back and inhales", 1, ch, 0, ch->specials.fighting, TO_ROOM);
  victim = 0;
  for (scan = hitlist; scan; scan = scan->next) {
    if (!scan->yesno ||
	IS_IMMORTAL(scan->ch) ||
	scan->ch->in_room != ch->in_room	/*
						 * this should not happen 
						 */
      )
      continue;
    victim = 1;
    cast_fear(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, scan->ch, 0);
  }

  if (func != NULL && victim) {
    act("$n Breathes...", 1, ch, 0, ch->specials.fighting, TO_ROOM);

    for (scan = hitlist; scan; scan = scan->next) {
      if (!scan->yesno ||
	  IS_IMMORTAL(scan->ch) ||
	  scan->ch->in_room != ch->in_room	/*
						 * this could happen if
						 * someone fled, I guess 
						 */
	)
	continue;
      func(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, scan->ch, 0);
    }
    GET_MANA(ch) -= mana_cost;
  } else {
    act("$n Breathes...coughs and sputters...",
	1, ch, 0, ch->specials.fighting, TO_ROOM);
    do_flee(ch, "", 0);
  }

  free_victims(hitlist);
}

int 
use_breath_weapon(struct char_data *ch, struct char_data *target,
		  int cost, void                   (*func) ())
{
  if (GET_MANA(ch) >= 0) {
    breath_weapon(ch, target, cost, func);
  } else if ((GET_HIT(ch) < GET_MAX_HIT(ch) / 2) &&
	     (GET_MANA(ch) >= -cost)) {
    breath_weapon(ch, target, cost, func);
  } else if ((GET_HIT(ch) < GET_MAX_HIT(ch) / 4) &&
	     (GET_MANA(ch) >= -2 * cost)) {
    breath_weapon(ch, target, cost, func);
  } else if (GET_MANA(ch) <= -3 * cost) {
    breath_weapon(ch, target, 0, NULL);		/*
						 * sputter 
						 */
  }
}

void                             cast_fire_breath();
void                             cast_frost_breath();
void                             cast_acid_breath();
void                             cast_gas_breath();
void                             cast_lightning_breath();

static funcp                     breaths[] =
{
cast_acid_breath, 0, cast_frost_breath, 0, cast_lightning_breath, 0,
  cast_fire_breath, 0,
  cast_acid_breath, cast_fire_breath, cast_lightning_breath, 0
};

struct breather                  breath_monsters[] =
{
  {5030, 15, breaths + 6},
  {-1},
};

int 
BreathWeapon(struct char_data *ch, int cmd, char *arg)
{
  char                             buf[MAX_STRING_LENGTH];
  struct breather                 *scan;
  int                              count;

  if (cmd)
    return FALSE;

  if (ch->specials.fighting &&
      (ch->specials.fighting->in_room == ch->in_room)) {

    for (scan = breath_monsters;
	 scan->vnum >= 0 && scan->vnum != mob_index[ch->nr].virtual;
	 scan++);

    if (scan->vnum < 0) {
      sprintf(buf, "monster %s tries to breath, but isn't listed.",
	      ch->player.short_descr);
      log(buf);
      return FALSE;
    }
    for (count = 0; scan->breaths[count]; count++);

    if (count < 1) {
      sprintf(buf, "monster %s has no breath weapons",
	      ch->player.short_descr);
      log(buf);
      return FALSE;
    }
    use_breath_weapon(ch, ch->specials.fighting, scan->cost,
		      scan->breaths[dice(1, count) - 1]);
  }
  return TRUE;
}

int 
DracoLich(struct char_data *ch, int cmd, char *arg)
{
}
int 
Drow(struct char_data *ch, int cmd, char *arg)
{
}
int 
Leader(struct char_data *ch, int cmd, char *arg)
{
}

int 
thief(struct char_data *ch, int cmd, char *arg)
{
  struct char_data                *cons;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (GET_POS(ch) != POSITION_STANDING)
    return FALSE;

  for (cons = real_roomp(ch->in_room)->people; cons; cons = cons->next_in_room)
    if ((!IS_NPC(cons)) && (GetMaxLevel(cons) < LOW_IMMORTAL) && (number(1, 5) == 1))
      npc_steal(ch, cons);

  return TRUE;
}

int 
magic_user(struct char_data *ch, int cmd, char *arg)
{
  struct char_data                *vict;
  byte                             lspell;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (!ch->specials.fighting)
    return FALSE;

  if ((GET_POS(ch) > POSITION_STUNNED) &&
      (GET_POS(ch) < POSITION_FIGHTING)) {
    StandUp(ch);
    return (TRUE);
  }
  /*
   * Find a dude to to evil things upon ! 
   */

  vict = FindVictim(ch);

  if (!vict)
    vict = ch->specials.fighting;

  if (!vict)
    return (FALSE);

  lspell = number(0, GetMaxLevel(ch));	/*
					 * gen number from 0 to level 
					 */

  if (lspell < 1)
    lspell = 1;

  if (IS_AFFECTED(ch, AFF_BLIND) && (lspell > 10)) {
    say_spell(ch, SPELL_CURE_BLIND);
    cast_cure_blind(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
    return TRUE;
  }
  if (IS_AFFECTED(ch, AFF_BLIND))
    return (FALSE);

  if ((vict != ch->specials.fighting) && (lspell > 13) && (number(0, 7) == 0)) {
    say_spell(ch, SPELL_SLEEP);
    cast_sleep(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    return TRUE;
  }
  if ((lspell > 5) && (number(0, 6) == 0)) {
    say_spell(ch, SPELL_WEAKNESS);
    cast_weakness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    return TRUE;
  }
  if ((lspell > 5) && (number(0, 7) == 0)) {
    say_spell(ch, SPELL_ARMOR);
    cast_armor(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
    return TRUE;
  }
  if ((lspell > 12) && (number(0, 7) == 0)) {
    say_spell(ch, SPELL_CURSE);
    cast_curse(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    return TRUE;
  }
  if ((lspell > 7) && (number(0, 5) == 0)) {
    say_spell(ch, SPELL_BLINDNESS);
    cast_blindness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    return TRUE;
  }
  switch (lspell) {
  case 1:
  case 2:
  case 3:
  case 4:
    say_spell(ch, SPELL_MAGIC_MISSILE);
    cast_magic_missile(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    break;
  case 5:
    say_spell(ch, SPELL_SHOCKING_GRASP);
    cast_shocking_grasp(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    break;
  case 6:
  case 7:
  case 8:
  case 9:
  case 10:
    say_spell(ch, SPELL_LIGHTNING_BOLT);
    cast_lightning_bolt(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    break;
  case 11:
    say_spell(ch, SPELL_DISPEL_MAGIC);
    cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    break;
  case 12:
  case 13:
  case 14:
    say_spell(ch, SPELL_COLOUR_SPRAY);
    cast_colour_spray(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    break;
  case 15:
  case 16:
    say_spell(ch, SPELL_FIREBALL);
    cast_fireball(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    break;
  case 17:
  case 18:
  case 19:
    if (IS_EVIL(ch)) {
      say_spell(ch, SPELL_ENERGY_DRAIN);
      cast_energy_drain(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      return TRUE;
    }
  default:
    say_spell(ch, SPELL_FIREBALL);
    cast_fireball(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    break;
  }
  return TRUE;
}

int 
cleric(struct char_data *ch, int cmd, char *arg)
{
  struct char_data                *vict;
  byte                             lspell,
                                   healperc = 0;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (GET_POS(ch) != POSITION_FIGHTING) {
    if ((GET_POS(ch) < POSITION_STANDING) && (GET_POS(ch) > POSITION_STUNNED)) {
      StandUp(ch);
    }
    return FALSE;
  }
  if (!ch->specials.fighting)
    return FALSE;

  /*
   * Find a dude to to evil things upon ! 
   */

  vict = FindVictim(ch);

  if (!vict)
    vict = ch->specials.fighting;

  if (!vict)
    return (FALSE);

  /*
   * gen number from 0 to level 
   */

  lspell = number(1, GetMaxLevel(ch));

  if (ch->points.hit < (ch->points.max_hit / 8))
    healperc = 7;
  else if (ch->points.hit < (ch->points.max_hit / 4))
    healperc = 5;
  if (ch->points.hit < (ch->points.max_hit / 2))
    healperc = 3;
  else if (number(1, healperc + 1) < 3) {
    if (OUTSIDE(ch) &&
	(weather_info.sky >= SKY_RAINING) &&
	(lspell >= 15) &&
	(number(0, 3) == 0)) {
      say_spell(ch, SPELL_CALL_LIGHTNING);
      cast_call_lightning(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      return (TRUE);
    }
    switch (lspell) {
    case 1:
    case 2:
    case 3:
      say_spell(ch, SPELL_CAUSE_LIGHT);
      cast_cause_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      break;
    case 4:
    case 5:
    case 6:
      say_spell(ch, SPELL_BLINDNESS);
      cast_blindness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      break;
    case 7:
      say_spell(ch, SPELL_DISPEL_MAGIC);
      cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      break;
    case 8:
      say_spell(ch, SPELL_POISON);
      cast_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      break;
    case 9:
    case 10:
      say_spell(ch, SPELL_CAUSE_CRITICAL);
      cast_cause_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      break;
    case 11:
      say_spell(ch, SPELL_FLAMESTRIKE);
      cast_flamestrike(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      break;
    case 12:
      say_spell(ch, SPELL_CURSE);
      cast_curse(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      break;
    case 13:
    case 14:
    case 15:
    case 16:
      {
	if ((GET_ALIGNMENT(vict) <= 0) && (GET_ALIGNMENT(ch) > 0)) {
	  say_spell(ch, SPELL_DISPEL_EVIL);
	  cast_dispel_evil(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
	} else if ((GET_ALIGNMENT(vict) >= 0) && (GET_ALIGNMENT(ch) < 0)) {
	  say_spell(ch, SPELL_DISPEL_GOOD);
	  cast_dispel_good(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
	} else {
	  if (!IS_SET(vict->M_immune, IMM_FIRE)) {
	    say_spell(ch, SPELL_FLAMESTRIKE);
	    cast_flamestrike(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
	  } else if (IS_AFFECTED(vict, AFF_SANCTUARY)) {
	    say_spell(ch, SPELL_DISPEL_MAGIC);
	    cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
	  } else {
	    say_spell(ch, SPELL_CAUSE_CRITICAL);
	    cast_cause_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
	  }
	}
	break;
      }
    case 17:
    case 18:
    case 19:
    default:
      say_spell(ch, SPELL_HARM);
      cast_harm(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      break;
    }

    return (TRUE);

  } else {

    if (IS_AFFECTED(ch, AFF_BLIND) && (lspell >= 4) & (number(0, 3) == 0)) {
      say_spell(ch, SPELL_CURE_BLIND);
      cast_cure_blind(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      return (TRUE);
    }
    if (IS_AFFECTED(ch, AFF_CURSE) && (lspell >= 6) && (number(0, 6) == 0)) {
      say_spell(ch, SPELL_REMOVE_CURSE);
      cast_remove_curse(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      return (TRUE);
    }
    if (IS_AFFECTED(ch, AFF_POISON) && (lspell >= 5) && (number(0, 6) == 0)) {
      say_spell(ch, SPELL_REMOVE_POISON);
      cast_remove_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      return (TRUE);
    }
    switch (lspell) {
    case 1:
    case 2:
    case 3:
    case 4:
      say_spell(ch, SPELL_ARMOR);
      cast_armor(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      break;
    case 5:
    case 6:
    case 7:
    case 8:
      say_spell(ch, SPELL_CURE_LIGHT);
      cast_cure_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      break;
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
      say_spell(ch, SPELL_CURE_CRITIC);
      cast_cure_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      break;
    case 17:
    case 18:			/*
				 * heal 
				 */
      say_spell(ch, SPELL_HEAL);
      cast_heal(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      break;
    default:
      say_spell(ch, SPELL_SANCTUARY);
      cast_sanctuary(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      break;

    }
    return (TRUE);
  }
}

/*
 * ********************************************************************
 * *  Special procedures for mobiles                                      *
 * ******************************************************************** 
 */

#ifdef 0

int 
guild_guard(struct char_data *ch, int cmd, char *arg)
{
  char                             buf[256],
                                   buf2[256];

  if (cmd > 6 || cmd < 1)
    return FALSE;

  strcpy(buf, "The guard humiliates you, and block your way.\n\r");
  strcpy(buf2, "The guard humiliates $n, and blocks $s way.");

  if ((IS_NPC(ch) && (IS_POLICE(ch))) || (GetMaxLevel(ch) >= DEMIGOD) ||
      (IS_AFFECTED(ch, AFF_SNEAK)))
    return (FALSE);

  /*
   **  Remove-For-Multi-Class
   */
  if ((ch->in_room == 3017) && (cmd == 3)) {
    if (!HasClass(ch, CLASS_MAGIC_USER)) {
      act(buf2, FALSE, ch, 0, 0, TO_ROOM);
      send_to_char(buf, ch);
      return TRUE;
    }
  } else if ((ch->in_room == 3004) && (cmd == 1)) {
    if (!HasClass(ch, CLASS_CLERIC)) {
      act(buf2, FALSE, ch, 0, 0, TO_ROOM);
      send_to_char(buf, ch);
      return TRUE;
    }
  } else if ((ch->in_room == 3027) && (cmd == 2)) {
    if (!HasClass(ch, CLASS_THIEF)) {
      act(buf2, FALSE, ch, 0, 0, TO_ROOM);
      send_to_char(buf, ch);
      return TRUE;
    }
  } else if ((ch->in_room == 3021) && (cmd == 2)) {
    if (!HasClass(ch, CLASS_WARRIOR)) {
      act(buf2, FALSE, ch, 0, 0, TO_ROOM);
      send_to_char(buf, ch);
      return TRUE;
    }
  }
  return FALSE;

}

#endif

int 
puff(struct char_data *ch, int cmd, char *arg)
{
  struct char_data                *i,
                                  *tmp,
                                  *tmp_ch;
  char                             buf[80];

  void                             do_emote(struct char_data *ch, char *argument, int cmd);
  void                             do_shout(struct char_data *ch, char *argument, int cmd);

  if (cmd)
    return (0);

  switch (number(0, 250)) {
  case 4:
  case 5:
  case 6:
  case 7:
    {
      for (i = character_list; i; i = i->next) {
	if (!IS_NPC(i)) {
	  if (number(0, 5) == 0) {
	    if (!strcmp(GET_NAME(i), "Harlequin")) {
	      do_shout(ch, "Harlequin!  I need reimbursement!", 0);
	    }
	  }
	}
	break;
      }
    }
    return (1);
  case 13:
    for (i = character_list; i; i = i->next) {
      if (!IS_NPC(i)) {
	if (number(0, 20) == 0) {
	  sprintf(buf, "%s shout Where is Puff?", GET_NAME(i));
	  do_force(ch, buf, 0);
	  do_restore(ch, GET_NAME(i), 0);
	  return (TRUE);
	}
      }
    }
    return (1);
  case 20:
    for (i = character_list; i; i = i->next) {
      if (!IS_NPC(i)) {
	if (number(0, 15) == 0) {
	  sprintf(buf, "Hiya %s!", GET_NAME(i));
	  do_shout(ch, buf, 0);
	  return (TRUE);
	}
      }
    }
    break;
  case 23:
    if (number(0, 20) == 0) {
      do_shout(ch, "Suffer, I will make you all suffer!!!!!", 0);
    }
    break;
  case 26:
    if (number(0, 10) == 0)
      do_shout(ch, "Suffer!!!!!! all will Suffer Grimwell's RATH!!!", 0);
    break;
  case 28:
    do_shout(ch, "SAVE!", 0);
    return (TRUE);
  case 32:
    if (number(0, 10) == 0) {
      do_shout(ch, "Any good weapons for sale!", 0);
      return (TRUE);
    }
    break;

  case 33:
    if (number(0, 10) == 0) {
      do_shout(ch, "Anyone want a red ring??", 0);
      return (TRUE);
    }
    break;
  case 34:
  case 38:
    for (i = character_list; i; i = i->next) {
      if (!IS_NPC(i)) {
	if (number(0, 20) == 0) {
	  if (i->in_room != NOWHERE) {
	    sprintf(buf, "save %s", GET_NAME(i));
	    do_force(ch, buf, 0);
	    return (TRUE);
	  }
	}
      }
    }
    return (TRUE);
  case 41:
    if (number(0, 10) == 0) {
      do_shout(ch, "The Vax is DEAD!!!!", 0);
      return (TRUE);
    }
    break;
  case 42:
    if (number(0, 10) == 0) {
      do_shout(ch, "Hiya!", 0);
      return (TRUE);
    }
    break;
  default:
    return (0);
  }
}

int 
regenerator(struct char_data *ch, int cmd, char *arg)
{

  if (cmd)
    return (FALSE);

  if (GET_HIT(ch) < GET_MAX_HIT(ch)) {
    GET_HIT(ch) += 9;
    GET_HIT(ch) = MIN(GET_HIT(ch), GET_MAX_HIT(ch));

    act("$n regenerates.", TRUE, ch, 0, 0, TO_ROOM);
    return (TRUE);
  }
}

int 
replicant(struct char_data *ch, int cmd, char *arg)
{
  struct char_data                *mob;

  if (cmd)
    return FALSE;

  if (GET_HIT(ch) < GET_MAX_HIT(ch)) {
    act("Drops of $n's blood hits the ground, and springs up into another one!",
	TRUE, ch, 0, 0, TO_ROOM);
    mob = read_mobile(ch->nr, REAL);
    char_to_room(mob, ch->in_room);
    act("Two undamaged opponents face you now.", TRUE, ch, 0, 0, TO_ROOM);
    GET_HIT(ch) = GET_MAX_HIT(ch);
  }
  return FALSE;
}

#ifdef 0

#define TYT_NONE 0
#define TYT_CIT  1
#define TYT_WHAT 2
#define TYT_TELL 3
#define TYT_HIT  4

int 
Tytan(struct char_data *ch, int cmd, char *arg)
{
  struct char_data                *vict;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting) {
    return (magic_user(ch, cmd, arg));
  } else {
    switch (ch->act_ptr) {
    case TYT_NONE:
      if (vict = FindVictim(ch)) {
	ch->act_ptr = TYT_CIT;
	SetHunting(ch, vict);
      }
      break;
    case TYT_CIT:
      if (ch->specials.hunting) {
	if (ch->in_room == ch->specials.hunting->in_room) {
	  act("Where is the Citadel?", TRUE, ch, 0, 0, TO_ROOM);
	  ch->act_ptr = TYT_WHAT;
	}
      } else {
	ch->act_ptr = TYT_NONE;
      }
      break;
    case TYT_WHAT:
      if (ch->specials.hunting) {
	if (ch->in_room == ch->specials.hunting->in_room) {
	  act("What must we do?", TRUE, ch, 0, 0, TO_ROOM);
	  ch->act_ptr = TYT_TELL;
	}
      } else {
	ch->act_ptr = TYT_NONE;
      }
      break;
    case TYT_TELL:
      if (ch->specials.hunting) {
	if (ch->in_room == ch->specials.hunting->in_room) {
	  act("Tell Us!  Command Us!", TRUE, ch, 0, 0, TO_ROOM);
	  ch->act_ptr = TYT_HIT;
	}
      } else {
	ch->act_ptr = TYT_NONE;
      }
      break;
    case TYT_HIT:
      if (ch->specials.hunting) {
	if (ch->in_room == ch->specials.hunting->in_room) {
	  if (!check_peaceful(ch, "")) {
	    hit(ch, ch->specials.hunting, TYPE_UNDEFINED);
	    ch->act_ptr = TYT_NONE;
	  } else {
	    ch->act_ptr = TYT_CIT;
	  }
	}
      } else {
	ch->act_ptr = TYT_NONE;
      }
      break;
    default:
      ch->act_ptr = TYT_NONE;
    }
  }
}

int 
AbbarachDragon(struct char_data *ch, int cmd, char *arg)
{

  struct char_data                *targ;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (!ch->specials.fighting) {
    targ = (struct char_data *)FindAnyVictim(ch);
    if (targ && !check_peaceful(ch, "")) {
      hit(ch, targ, TYPE_UNDEFINED);
      act("You have now payed the price of crossing.",
	  TRUE, ch, 0, 0, TO_ROOM);
      return (TRUE);
    }
  } else {
    return (BreathWeapon(ch, cmd, arg));
  }
}

#endif

int 
fido(struct char_data *ch, int cmd, char *arg)
{

  struct obj_data                 *i,
                                  *temp,
                                  *next_obj;
  struct char_data                *v,
                                  *next;
  char                             found = FALSE;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  for (v = character_list; (v && (!found)); v = next) {
    next = v->next;
    if ((IS_NPC(v)) && (mob_index[v->nr].virtual == 100) &&
	(v->in_room == ch->in_room) && CAN_SEE(ch, v)) {
      if (v->specials.fighting)
	stop_fighting(v);
      make_corpse(v);
      extract_char(v);
      found = TRUE;
    }
  }

  for (i = real_roomp(ch->in_room)->contents; i; i = i->next_content) {
    if (GET_ITEM_TYPE(i) == ITEM_CONTAINER && i->obj_flags.value[3]) {
      act("$n savagely devours a corpse.", FALSE, ch, 0, 0, TO_ROOM);
      for (temp = i->contains; temp; temp = next_obj) {
	next_obj = temp->next_content;
	obj_from_obj(temp);
	obj_to_room(temp, ch->in_room);
      }
      extract_obj(i);
      return (TRUE);
    }
  }
  return (FALSE);
}

int 
janitor(struct char_data *ch, int cmd, char *arg)
{
  struct obj_data                 *i,
                                  *temp,
                                  *next_obj;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  for (i = real_roomp(ch->in_room)->contents; i; i = i->next_content) {
    if (IS_SET(i->obj_flags.wear_flags, ITEM_TAKE)) {
      act("$n picks up some trash.", FALSE, ch, 0, 0, TO_ROOM);
      obj_from_room(i);
      obj_to_char(i, ch);
      return (TRUE);
    }
  }
  return (FALSE);
}

int 
janitor_eats(struct char_data *ch, int cmd, char *arg)
{
  struct obj_data                 *i,
                                  *temp,
                                  *next_obj;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  for (i = real_roomp(ch->in_room)->contents; i; i = i->next_content) {
    if (IS_SET(i->obj_flags.wear_flags, ITEM_TAKE)) {
      act("$n picks up some trash.", FALSE, ch, 0, 0, TO_ROOM);
      act("$n nibbles on the new found garbage.", FALSE, ch, 0, 0, TO_ROOM);
      extract_obj(i);
      return (TRUE);
    }
  }
  return (FALSE);
}

#ifdef 0

int 
tormentor(struct char_data *ch, int cmd, char *arg)
{
  if (!cmd)
    return (FALSE);
  if (IS_NPC(ch))
    return (FALSE);
  if (IS_IMMORTAL(ch))
    return (FALSE);
  return (TRUE);
}

#endif

int 
Fighter(struct char_data *ch, int cmd, char *arg)
{
  if (ch->specials.fighting) {
    if (GET_POS(ch) == POSITION_FIGHTING) {
      FighterMove(ch);
      return (TRUE);
    } else
      StandUp(ch);
  }
  return (FALSE);
}

#ifdef 0

int 
RustMonster(struct char_data *ch, int cmd, char *arg)
{
  struct char_data                *vict;
  struct obj_data                 *t_item;
  int                              t_pos;

  if (cmd || !AWAKE(ch))
    return (FALSE);

/*
 * **   find a victim
 */
  if (ch->specials.fighting) {
    vict = ch->specials.fighting;
  } else {
    vict = FindVictim(ch);
    if (!vict) {
      return (FALSE);
    }
  }

/*
 * **   choose an item of armor or a weapon that is metal
 * **  since metal isn't defined, we'll just use armor and weapons   
 */

  /*
   * **  choose a weapon first, then if no weapon, choose a shield,
   * **  if no shield, choose breast plate, then leg plate, sleeves,
   * **  helm
   */

  if (vict->equipment[WIELD]) {
    t_item = vict->equipment[WIELD];
    t_pos = WIELD;
  } else if (vict->equipment[WEAR_SHIELD]) {
    t_item = vict->equipment[WEAR_SHIELD];
    t_pos = WEAR_SHIELD;
  } else if (vict->equipment[WEAR_BODY]) {
    t_item = vict->equipment[WEAR_BODY];
    t_pos = WEAR_BODY;
  } else if (vict->equipment[WEAR_LEGS]) {
    t_item = vict->equipment[WEAR_LEGS];
    t_pos = WEAR_LEGS;
  } else if (vict->equipment[WEAR_ARMS]) {
    t_item = vict->equipment[WEAR_ARMS];
    t_pos = WEAR_ARMS;
  } else if (vict->equipment[WEAR_HEAD]) {
    t_item = vict->equipment[WEAR_HEAD];
    t_pos = WEAR_HEAD;
  } else {
    return (FALSE);
  }

/*
 * **  item makes save (or not)
 */
  if (DamageOneItem(vict, ACID_DAMAGE, t_item)) {
    t_item = unequip_char(vict, t_pos);
    if (t_item) {
/*
 * **  if it doesn't make save, falls into a pile of scraps
 */
      MakeScrap(vict, t_item);
    }
  }
  return (FALSE);

}

int 
temple_labrynth_liar(struct char_data *ch, int cmd, char *arg)
{

  if (cmd || !AWAKE(ch))
    return (0);

  switch (number(0, 15)) {
  case 0:
    do_say(ch, "I'd go west if I were you.", 0);
    return (1);
  case 1:
    do_say(ch, "I heard that Addiction is a cute babe.", 0);
    return (1);
  case 2:
    do_say(ch, "Going east will avoid the beast!", 0);
    return (1);
  case 4:
    do_say(ch, "North is the way to go.", 0);
    return (1);
  case 6:
    do_say(ch, "Dont dilly dally go south.", 0);
    return (1);
  case 8:
    do_say(ch, "Great treasure lies ahead", 0);
    return (1);
  case 10:
    do_say(ch, "I wouldn't kill the sentry if I were more than level 9. No way!", 0);
    return (1);
  case 12:
    do_say(ch, "I am a very clever liar.", 0);
    return (1);
  case 14:
    do_say(ch, "Loki is a really great guy!", 0);
    return (1);
  default:
    do_say(ch, "Then again I could be wrong!", 0);
    return (1);
  }
}

int 
temple_labrynth_sentry(struct char_data *ch, int cmd, char *arg)
{
  struct char_data                *tch;
  int                              counter;

  void                             cast_fireball(byte level, struct char_data *ch, char *arg, int type,
		struct char_data *victim, struct obj_data *tar_obj);

  if (cmd || !AWAKE(ch))
    return FALSE;

  if (GET_POS(ch) != POSITION_FIGHTING)
    return FALSE;

  if (!ch->specials.fighting)
    return FALSE;

  /*
   * Find a dude to do very evil things upon ! 
   */

  for (tch = real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room) {
    if (GetMaxLevel(tch) > 10 && CAN_SEE(ch, tch)) {
      act("The sentry snaps out of his trance and ...", 1, ch, 0, 0, TO_ROOM);
      do_say(ch, "You will die for your insolence, pig-dog!", 0);
      for (counter = 0; counter < 4; counter++)
	if (GET_POS(tch) > POSITION_SITTING)
	  cast_fireball(15, ch, "", SPELL_TYPE_SPELL, tch, 0);
	else
	  return TRUE;
      return TRUE;
    } else {
      act("The sentry looks concerned and continues to push you away",
	  1, ch, 0, 0, TO_ROOM);
      do_say(ch, "Leave me alone. My vows do not permit me to kill you!", 0);
    }
  }
  return TRUE;
}

#define WW_LOOSE 0
#define WW_FOLLOW 1

int 
Whirlwind(struct char_data *ch, int cmd, char *arg)
{
  struct char_data                *tmp;
  const char                      *names[] =
  {"Quixadhal", "", 0};
  int                              i = 0;

  if (ch->in_room == -1)
    return (FALSE);

  if (cmd == 0 && ch->act_ptr == WW_LOOSE) {
    for (tmp = real_roomp(ch->in_room)->people; tmp; tmp = tmp->next_in_room) {
      while (names[i]) {
	if (!strcmp(GET_NAME(tmp), names[i]) && ch->act_ptr == WW_LOOSE) {
	  /*
	   * start following 
	   */
	  if (circle_follow(ch, tmp))
	    return (FALSE);
	  if (ch->master)
	    stop_follower(ch);
	  add_follower(ch, tmp);
	  ch->act_ptr = WW_FOLLOW;
	}
	i++;
      }
    }
    if (ch->act_ptr == WW_LOOSE && !cmd) {
      act("The $n suddenly dissispates into nothingness.", 0, ch, 0, 0, TO_ROOM);
      extract_char(ch);
    }
  }
}

#endif

#define NN_LOOSE  0
#define NN_FOLLOW 1
#define NN_STOP   2

int 
NudgeNudge(struct char_data *ch, int cmd, char *arg)
{

  struct char_data                *vict;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting) {
    return (FALSE);
  }
  switch (ch->act_ptr) {
  case NN_LOOSE:
    /*
     * ** find a victim
     */
    vict = FindVictim(ch);
    if (!vict)
      return (FALSE);
    /*
     * start following 
     */
    if (circle_follow(ch, vict)) {
      return (FALSE);
    }
    if (ch->master)
      stop_follower(ch);
    add_follower(ch, vict);
    ch->act_ptr = NN_FOLLOW;
    do_say(ch, "Good Evenin' Squire!", 0);
    act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
    break;
  case NN_FOLLOW:
    switch (number(0, 20)) {
    case 0:
      do_say(ch, "Is your wife a goer?  Know what I mean, eh?", 0);
      act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
      break;
    case 1:
      act("$n winks at you.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n winks at you.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n winks at $N.", FALSE, ch, 0, ch->master, TO_ROOM);
      act("$n winks at $N.", FALSE, ch, 0, ch->master, TO_ROOM);
      act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
      act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
      do_say(ch, "Say no more!  Say no MORE!", 0);
      break;
    case 2:
      do_say(ch, "You been around, eh?", 0);
      do_say(ch, "...I mean you've ..... done it, eh?", 0);
      act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
      act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
      break;
    case 3:
      do_say(ch, "A nod's as good as a wink to a blind bat, eh?", 0);
      act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
      act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
      break;
    case 4:
      do_say(ch, "You're WICKED, eh!  WICKED!", 0);
      act("$n winks at you.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n winks at you.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n winks at $N.", FALSE, ch, 0, ch->master, TO_ROOM);
      act("$n winks at $N.", FALSE, ch, 0, ch->master, TO_ROOM);
      break;
    case 5:
      do_say(ch, "Wink. Wink.", 0);
      break;
    case 6:
      do_say(ch, "Nudge. Nudge.", 0);
      break;
    case 7:
    case 8:
    case 9:
    case 10:
      ch->act_ptr = NN_STOP;
      break;
    default:
      break;
    }
    break;
  case NN_STOP:
    /*
     * **  Stop following
     */
    do_say(ch, "Evening, Squire", 0);
    stop_follower(ch);
    ch->act_ptr = NN_LOOSE;
    break;
  default:
    ch->act_ptr = NN_LOOSE;
    break;
  }
}

#ifdef 0

int 
AGGRESSIVE(struct char_data *ch, int cmd, char *arg)
{
  struct char_data                *i,
                                  *next;

  if (cmd || !AWAKE(ch))
    return (FALSE);
  if (ch->in_room > -1) {
    for (i = real_roomp(ch->in_room)->people; i; i = next) {
      next = i->next_in_room;
      if (i->nr != ch->nr) {
	if (!IS_IMMORTAL(i)) {
	  hit(ch, i, TYPE_UNDEFINED);
	}
      }
    }
  }
}

#endif

int 
cityguard(struct char_data *ch, int cmd, char *arg)
{
  struct char_data                *tch,
                                  *evil,
                                  *i;
  int                              max_evil,
                                   lev;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting) {
    if (GET_POS(ch) == POSITION_FIGHTING) {
      FighterMove(ch);
    } else {
      StandUp(ch);
    }

    if (number(0, 20) > 15) {
      do_shout(ch, "To me, my fellows! I am in need of thy aid!", 0);
    } else {
      act("$n shouts 'To me, my fellows! I need thy aid!'", TRUE, ch, 0, 0, TO_ROOM);
    }

    if (ch->specials.fighting)
      CallForGuard(ch, ch->specials.fighting, 4);
    return (TRUE);
  }
  max_evil = 1000;
  evil = 0;

  if (check_peaceful(ch, ""))
    return FALSE;

  for (tch = real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room) {
    if ((IS_NPC(tch)) && (IsUndead(tch)) && CAN_SEE(ch, tch)) {
      max_evil = -1000;
      evil = tch;
      act("$n screams 'Suffer!!!  I will make you suffer!'", FALSE, ch, 0, 0, TO_ROOM);
      hit(ch, evil, TYPE_UNDEFINED);
      return (TRUE);
    }
    if (tch->specials.fighting) {
      if ((GET_ALIGNMENT(tch) < max_evil) &&
	  (IS_NPC(tch) || IS_NPC(tch->specials.fighting))) {
	max_evil = GET_ALIGNMENT(tch);
	evil = tch;
      }
    }
  }

  if (evil && (GET_ALIGNMENT(evil->specials.fighting) >= 0)) {
    act("$n screams 'Suffer!!!!! You will suffer'",
	FALSE, ch, 0, 0, TO_ROOM);
    hit(ch, evil, TYPE_UNDEFINED);
    return (TRUE);
  }
  return (FALSE);
}

#ifdef 0

int 
WarrenGuard(struct char_data *ch, int cmd, char *arg)
{
  struct char_data                *tch,
                                  *good,
                                  *i;
  int                              max_good,
                                   lev;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting) {
    if (GET_POS(ch) == POSITION_FIGHTING) {
      FighterMove(ch);
    } else {
      StandUp(ch);
    }

    return (TRUE);
  }
  max_good = -1000;
  good = 0;

  if (check_peaceful(ch, ""))
    return FALSE;

  for (tch = real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room) {
    if (tch->specials.fighting) {
      if ((GET_ALIGNMENT(tch) > max_good) &&
	  (IS_NPC(tch) || IS_NPC(tch->specials.fighting))) {
	max_good = GET_ALIGNMENT(tch);
	good = tch;
      }
    }
  }

  if (good && (GET_ALIGNMENT(good->specials.fighting) <= 0)) {
    act("$n screams 'DEATH TO GOODY-GOODIES!!!!'",
	FALSE, ch, 0, 0, TO_ROOM);
    hit(ch, good, TYPE_UNDEFINED);
    return (TRUE);
  }
  return (FALSE);
}

int 
zm_tired(struct char_data *zmaster)
{
  return GET_HIT(zmaster) < GET_MAX_HIT(zmaster) / 2 ||
    GET_MANA(zmaster) < 40;
}

int 
zm_stunned_followers(struct char_data *zmaster)
{
  struct follow_type              *fwr;

  for (fwr = zmaster->followers; fwr; fwr = fwr->next)
    if (GET_POS(fwr->follower) == POSITION_STUNNED)
      return TRUE;
  return FALSE;
}

zm_init_combat(struct char_data * zmaster, struct char_data * target)
{
  struct follow_type              *fwr;

  for (fwr = zmaster->followers; fwr; fwr = fwr->next)
    if (IS_AFFECTED(fwr->follower, AFF_CHARM) &&
	fwr->follower->specials.fighting == NULL &&
	fwr->follower->in_room == target->in_room)
      if (GET_POS(fwr->follower) == POSITION_STANDING) {
	hit(fwr->follower, target, TYPE_UNDEFINED);
      } else if (GET_POS(fwr->follower) > POSITION_SLEEPING &&
		 GET_POS(fwr->follower) < POSITION_FIGHTING) {
	do_stand(fwr->follower, "", -1);
      }
}

int 
zm_kill_fidos(struct char_data *zmaster)
{
  struct char_data                *fido_b;

  fido_b = find_mobile_here_with_spec_proc(fido, zmaster->in_room);
  if (fido_b) {
    act("$n shrilly screams 'Kill that carrion beast!'", FALSE,
	zmaster, 0, 0, TO_ROOM);
    zm_init_combat(zmaster, fido_b);
    return TRUE;
  }
  return FALSE;
}

int 
zm_kill_aggressor(struct char_data *zmaster)
{
  struct follow_type              *fwr;

  if (zmaster->specials.fighting) {
    act("$n bellows 'Kill that mortal that dares lay hands on me!'", FALSE,
	zmaster, 0, 0, TO_ROOM);
    zm_init_combat(zmaster, zmaster->specials.fighting);
    return TRUE;
  }
  for (fwr = zmaster->followers; fwr; fwr = fwr->next)
    if (fwr->follower->specials.fighting &&
	IS_AFFECTED(fwr->follower, AFF_CHARM)) {
      act("$n bellows 'Assist your brethren, my loyal servants!'", FALSE,
	  zmaster, 0, 0, TO_ROOM);
      zm_init_combat(zmaster, fwr->follower->specials.fighting);
      return TRUE;
    }
  return FALSE;
}

int 
zombie_master(struct char_data *ch, int cmd, char *arg)
#define ZM_MANA	10
#define ZM_NEMESIS 3060
{
  struct obj_data                 *temp1;
  struct char_data                *zmaster;
  char                             buf[240];
  int                              dir;

  zmaster = find_mobile_here_with_spec_proc(zombie_master, ch->in_room);

  if (cmd != 0 || ch != zmaster || !AWAKE(ch))
    return FALSE;

  if (!check_peaceful(ch, "") &&
      (zm_kill_fidos(zmaster) || zm_kill_aggressor(zmaster))
    ) {
    do_stand(zmaster, "", -1);
    return TRUE;
  }
  switch (GET_POS(zmaster)) {
  case POSITION_RESTING:
    if (!zm_tired(zmaster))
      do_stand(zmaster, "", -1);
    break;
  case POSITION_SITTING:
    if (!zm_stunned_followers(zmaster)) {
      act("$n says 'It took you long enough...'", FALSE,
	  zmaster, 0, 0, TO_ROOM);
      do_stand(zmaster, "", -1);
    }
    break;
  case POSITION_STANDING:
    if (zm_tired(zmaster)) {
      do_rest(zmaster, "", -1);
      return TRUE;
    }
    temp1 = get_obj_in_list_vis(zmaster, "corpse",
			    real_roomp(zmaster->in_room)->contents);

    if (temp1) {
      if (GET_MANA(zmaster) < ZM_MANA) {
	if (1 == dice(1, 20))
	  act("$n says 'So many bodies, so little time' and sighs.",
	      FALSE, zmaster, 0, 0, TO_ROOM);
      } else {
	act("$n says 'Wonderful, another loyal follower!' and grins maniacly.",
	    FALSE, zmaster, 0, 0, TO_ROOM);
	GET_MANA(zmaster) -= ZM_MANA;
	spell_animate_dead(GetMaxLevel(zmaster), ch, NULL, temp1);
	/*
	 * assume the new follower is top of the list? 
	 */
	AddHatred(zmaster->followers->follower, OP_VNUM, ZM_NEMESIS);
      }
      return TRUE;
    } else if (zm_stunned_followers(zmaster)) {
      do_sit(zmaster, "", -1);
      return TRUE;
    } else if (1 == dice(1, 20)) {
      act("$n searches for bodies.", FALSE, zmaster, 0, 0, TO_ROOM);
      return TRUE;
    } else if (0 <= (dir = find_path(zmaster->in_room,
			 named_object_on_ground, "corpse", -200))) {
      go_direction(zmaster, dir);
      return TRUE;
    } else if (1 == dice(1, 5)) {
      act("$n can't find any bodies.", FALSE, zmaster, 0, 0, TO_ROOM);
      return TRUE;
    } else {
      mobile_wander(zmaster);
    }
  }

  return FALSE;
}

#endif

int 
pet_shops(struct char_data *ch, int cmd, char *arg)
{
  char                             buf[MAX_STRING_LENGTH],
                                   pet_name[256];
  int                              pet_room;
  struct char_data                *pet;

  pet_room = ch->in_room + 1;

  if (cmd == 59) {		/*
				 * List 
				 */
    send_to_char("Available pets are:\n\r", ch);
    for (pet = real_roomp(pet_room)->people; pet; pet = pet->next_in_room) {
      sprintf(buf, "%8d - %s\n\r", 10 * GET_EXP(pet), pet->player.short_descr);
      send_to_char(buf, ch);
    }
    return (TRUE);
  } else if (cmd == 56) {	/*
				 * Buy 
				 */

    arg = one_argument(arg, buf);
    only_argument(arg, pet_name);

    /*
     * Pet_Name is for later use when I feel like it 
     */

    if (!(pet = get_char_room(buf, pet_room))) {
      send_to_char("There is no such pet!\n\r", ch);
      return (TRUE);
    }
    if (GET_GOLD(ch) < (GET_EXP(pet) * 10)) {
      send_to_char("You don't have enough gold!\n\r", ch);
      return (TRUE);
    }
    GET_GOLD(ch) -= GET_EXP(pet) * 10;

    pet = read_mobile(pet->nr, REAL);
    GET_EXP(pet) = 0;
    SET_BIT(pet->specials.affected_by, AFF_CHARM);

    if (*pet_name) {
      sprintf(buf, "%s %s", pet->player.name, pet_name);
      free(pet->player.name);
      pet->player.name = strdup(buf);

      sprintf(buf, "%sA small sign on a chain around the neck says 'My Name is %s'\n\r",
	      pet->player.description, pet_name);
      free(pet->player.description);
      pet->player.description = strdup(buf);
    }
    char_to_room(pet, ch->in_room);
    add_follower(pet, ch);

    IS_CARRYING_W(pet) = 0;
    IS_CARRYING_N(pet) = 0;

    send_to_char("May you enjoy your pet.\n\r", ch);
    act("$n bought $N as a pet.", FALSE, ch, 0, pet, TO_ROOM);

    return (TRUE);
  }
  /*
   * All commands except list and buy 
   */
  return (FALSE);
}

int 
bank(struct char_data *ch, int cmd, char *arg)
{

  static char                      buf[256];
  int                              money;

  money = atoi(arg);

  if (!IS_NPC(ch))
    save_char(ch, ch->in_room);

  /*
   * deposit
   */
  if (cmd == 219) {
    if (money > GET_GOLD(ch)) {
      send_to_char("You don't have enough for that!\n\r", ch);
      return (TRUE);
    } else if (money <= 0) {
      send_to_char("Go away, you bother me.\n\r", ch);
      return (TRUE);
    } else {
      send_to_char("Thank you.\n\r", ch);
      GET_GOLD(ch) = GET_GOLD(ch) - money;
      GET_BANK(ch) = GET_BANK(ch) + money;
      sprintf(buf, "Your balance is %d.\n\r", GET_BANK(ch));
      send_to_char(buf, ch);
      if (GET_BANK(ch) > 200000) {
	sprintf(buf, "%s has %d coins in the bank.", GET_NAME(ch), GET_BANK(ch));
	log(buf);
      }
      return (TRUE);
    }
    /*
     * withdraw
     */
  } else if (cmd == 220) {
    if (money > GET_BANK(ch)) {
      send_to_char("You don't have enough in the bank for that!\n\r", ch);
      return (TRUE);
    } else if (money <= 0) {
      send_to_char("Go away, you bother me.\n\r", ch);
      return (TRUE);
    } else {
      send_to_char("Thank you.\n\r", ch);
      GET_GOLD(ch) = GET_GOLD(ch) + money;
      GET_BANK(ch) = GET_BANK(ch) - money;
      sprintf(buf, "Your balance is %d.\n\r", GET_BANK(ch));
      send_to_char(buf, ch);
      return (TRUE);
    }
  } else if (cmd == 221) {
    sprintf(buf, "Your balance is %d.\n\r", GET_BANK(ch));
    send_to_char(buf, ch);
    return (TRUE);
  }
  return (FALSE);
}

/*
 * Idea of the LockSmith is functionally similar to the Pet Shop 
 */
/*
 * The problem here is that each key must somehow be associated  
 */
/*
 * with a certain player. My idea is that the players name will  
 */
/*
 * appear as the another Extra description keyword, prefixed     
 */
/*
 * by the words 'item_for_' and followed by the player name.     
 */
/*
 * The (keys) must all be stored in a room which is (virtually)  
 */
/*
 * adjacent to the room of the lock smith.                       
 */

int 
pray_for_items(struct char_data *ch, int cmd, char *arg)
{
  char                             buf[256];
  int                              key_room,
                                   gold;
  bool                             found;
  struct obj_data                 *tmp_obj,
                                  *obj;
  struct extra_descr_data         *ext;

  if (cmd != 176)		/*
				 * You must pray to get the stuff 
				 */
    return FALSE;

  key_room = 1 + ch->in_room;

  strcpy(buf, "item_for_");
  strcat(buf, GET_NAME(ch));

  gold = 0;
  found = FALSE;

  for (tmp_obj = real_roomp(key_room)->contents;
       tmp_obj;
       tmp_obj = tmp_obj->next_content)
    for (ext = tmp_obj->ex_description; ext; ext = ext->next)
      if (str_cmp(buf, ext->keyword) == 0) {
	if (gold == 0) {
	  gold = 1;
	  act("$n kneels at the altar and chants a prayer to Quixadhal.",
	      FALSE, ch, 0, 0, TO_ROOM);
	  act("You notice a faint light in Odin's eye.",
	      FALSE, ch, 0, 0, TO_CHAR);
	}
	obj = read_object(tmp_obj->item_number, REAL);
	obj_to_room(obj, ch->in_room);
	act("$p slowly fades into existence.", FALSE, ch, obj, 0, TO_ROOM);
	act("$p slowly fades into existence.", FALSE, ch, obj, 0, TO_CHAR);
	gold += obj->obj_flags.cost;
	found = TRUE;
      }
  if (found) {
    GET_GOLD(ch) -= gold;
    GET_GOLD(ch) = MAX(0, GET_GOLD(ch));
    return TRUE;
  }
  return FALSE;
}

/*
 * ********************************************************************
 * *  Special procedures for objects                                     *
 * ******************************************************************** 
 */

#ifdef 0

#define CHAL_ACT \
"You are torn out of reality!\n\r\
You roll and tumble through endless voids for what seems like eternity...\n\r\
\n\r\
After a time, a new reality comes into focus... you are elsewhere.\n\r"

int 
chalice(struct char_data *ch, int cmd, char *arg)
{
  /*
   * 222 is the normal chalice, 223 is chalice-on-altar 
   */

  struct obj_data                 *chalice;
  char                             buf1[MAX_INPUT_LENGTH],
                                   buf2[MAX_INPUT_LENGTH];
  static int                       chl = -1,
                                   achl = -1;

  if (chl < 1) {
    chl = real_object(222);
    achl = real_object(223);
  }
  switch (cmd) {
  case 10:			/*
				 * get 
				 */
    if (!(chalice = get_obj_in_list_num(chl,
				 real_roomp(ch->in_room)->contents))
	&& CAN_SEE_OBJ(ch, chalice))
      if (!(chalice = get_obj_in_list_num(achl,
					  real_roomp(ch->in_room)->contents)) && CAN_SEE_OBJ(ch, chalice))
	return (0);

    /*
     * we found a chalice.. now try to get us 
     */
    do_get(ch, arg, cmd);
    /*
     * if got the altar one, switch her 
     */
    if (chalice == get_obj_in_list_num(achl, ch->carrying)) {
      extract_obj(chalice);
      chalice = read_object(chl, VIRTUAL);
      obj_to_char(chalice, ch);
    }
    return (1);
    break;
  case 67:			/*
				 * put 
				 */
    if (!(chalice = get_obj_in_list_num(chl, ch->carrying)))
      return (0);

    argument_interpreter(arg, buf1, buf2);
    if (!str_cmp(buf1, "chalice") && !str_cmp(buf2, "altar")) {
      extract_obj(chalice);
      chalice = read_object(achl, VIRTUAL);
      obj_to_room(chalice, ch->in_room);
      send_to_char("Ok.\n\r", ch);
    }
    return (1);
    break;
  case 176:			/*
				 * pray 
				 */
    if (!(chalice = get_obj_in_list_num(achl,
				real_roomp(ch->in_room)->contents)))
      return (0);

    do_action(ch, arg, cmd);	/*
				 * pray 
				 */
    send_to_char(CHAL_ACT, ch);
    extract_obj(chalice);
    act("$n is torn out of existence!", TRUE, ch, 0, 0, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, 2500);	/*
				 * before the fiery gates 
				 */
    do_look(ch, "", 15);
    return (1);
    break;
  default:
    return (0);
    break;
  }
}

int 
kings_hall(struct char_data *ch, int cmd, char *arg)
{
  if (cmd != 176)
    return (0);

  do_action(ch, arg, 176);
  send_to_char("You feel as if some mighty force has been offended.\n\r", ch);
  send_to_char(CHAL_ACT, ch);
  act("$n is struck by an intense beam of light and vanishes.",
      TRUE, ch, 0, 0, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, 1420);	/*
				 * behind the altar 
				 */
  do_look(ch, "", 15);
  return (1);
}

#endif

/** Perform breadth first search on rooms from start (in_room) **/
/** until end (tgt_room) is reached. Then return the correct   **/
/** direction to take from start to reach end.                 **/
/*
 * thoth@manatee.cis.ufl.edu
 * if dvar<0 then search THROUGH closed but not locked doors,
 * for mobiles that know how to open doors.
 */

#define IS_DIR    (real_roomp(q_head->room_nr)->dir_option[i])
#define GO_OK  (!IS_SET(IS_DIR->exit_info,EX_CLOSED)\
		 && (IS_DIR->to_room != NOWHERE))
#define GO_OK_SMARTER  (!IS_SET(IS_DIR->exit_info,EX_LOCKED)\
		 && (IS_DIR->to_room != NOWHERE))

static void 
donothing()
{
  return;
}

int 
find_path(int in_room, int       (*predicate) (), void *c_data, int depth)
{
  struct room_q                   *tmp_q,
                                  *q_head,
                                  *q_tail;

/*
 * struct nodes x_room[MAX_ROOMS];
 */
  int                              i,
                                   tmp_room,
                                   count = 0,
                                   thru_doors;
  static struct hash_header        x_room;
  struct room_data                *herep,
                                  *therep;
  struct room_direction_data      *exitp;

  /*
   * If start = destination we are done 
   */
  if ((predicate) (in_room, c_data))
    return -1;

#if 0
  if (top_of_world > MAX_ROOMS) {
    log("TRACK Is disabled, too many rooms.\n\rContact Loki soon.\n\r");
    return -1;
  }
#endif

  if (depth < 0) {
    thru_doors = TRUE;
    depth = -depth;
  } else {
    thru_doors = FALSE;
  }

#if 0
  /*
   * zero room array 
   */
  for (i = 0; i < top_of_world && i < MAX_ROOMS; i++)
    x_room[i].visited = 0;

  /*
   * mark start room as vistited 
   */
  x_room[in_room].visited = TRUE;
  x_room[in_room].ancestor = -1;
#else
  if (x_room.buckets) {		/*
				 * junk left over from a previous track 
				 */
    destroy_hash_table(&x_room, donothing);
  }
  init_hash_table(&x_room, sizeof(int), 2048);

  hash_enter(&x_room, in_room, (void *)-1);
#endif

  /*
   * initialize queue 
   */
  q_head = (struct room_q *)malloc(sizeof(struct room_q));

  q_tail = q_head;
  q_tail->room_nr = in_room;
  q_tail->next_q = 0;

  while (q_head) {
    herep = real_roomp(q_head->room_nr);
    /*
     * for each room test all directions 
     */
    for (i = 0; i <= 5; i++) {
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

	    tmp_q = (struct room_q *)malloc(sizeof(struct room_q));

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
	    free(q_head);
	  }
	  /*
	   * return direction if first layer 
	   */
	  if ((int)hash_find(&x_room, tmp_room) == -1)
	    return (i);
	  else			/*
				 * else return the ancestor 
				 */
	    return (-1 + (int)hash_find(&x_room, tmp_room));
	}
      }
    }
    /*
     * free queue head and point to next entry 
     */
    tmp_q = q_head->next_q;
    free(q_head);
    q_head = tmp_q;
  }
  /*
   * couldn't find path 
   */
  return (-1);
}

int 
choose_exit(int in_room, int tgt_room, int depth)
{
  return find_path(in_room, is_target_room_p, (void *)tgt_room, depth);
}

int 
go_direction(struct char_data *ch, int dir)
{
  if (ch->specials.fighting)
    return;

  if (!IS_SET(EXIT(ch, dir)->exit_info, EX_CLOSED)) {
    return do_move(ch, "", dir + 1);
  } else if (IsHumanoid(ch) && !IS_SET(EXIT(ch, dir)->exit_info, EX_LOCKED)) {
    open_door(ch, dir);
    return 0;
  }
}

/*
 * house routine for saved items.
 */

int 
House(struct char_data *ch, int cmd, char *arg)
{
  struct obj_cost                  cost;
  int                              i,
                                   save_room;
  char                            *tmp_desc;
  struct extra_descr_data         *ext;
  int                              found = 0;

  if (IS_NPC(ch))
    return (FALSE);

  if (cmd != 92) {
    return (FALSE);
  } else {
    /*
     * // Verify that a person can rent here, the name of the character wil
     * // be found in the extra description of the room itself, thus leaving
     * // the name of the room to be what ever the owner wishes.
     */

    found = 0;
    for (ext = real_roomp(ch->in_room)->ex_description; ext && !found; ext = ext->next)
      if (str_cmp(GET_NAME(ch), ext->keyword) == 0) {
	found = 1;
	send_to_char("Okay, found your name in the anals.\n\r", ch);
      }
    if (!found) {
      if (strncmp(GET_NAME(ch), real_roomp(ch->in_room)->name, strlen(GET_NAME(ch)))) {
	send_to_char("Sorry, you'll have to find your own house.\n\r", ch);
	return (FALSE);
      } else {
	send_to_char("Ah, you own this room.\n\r", ch);
      }
    }
    cost.total_cost = 0;	/*
				 * Minimum cost 
				 */
    cost.no_carried = 0;
    cost.ok = TRUE;		/*
				 * Use if any "-1" objects 
				 */

    add_obj_cost(ch, 0, ch->carrying, &cost);
    for (i = 0; i < MAX_WEAR; i++)
      add_obj_cost(ch, 0, ch->equipment[i], &cost);

    if (!cost.ok) {
      return (FALSE);
    }
    cost.total_cost = 0;

    GET_HOME(ch) = ch->in_room;
    save_obj(ch, &cost, 1);
    save_room = ch->in_room;
    extract_char(ch);
    ch->in_room = save_room;
    save_char(ch, ch->in_room);
    return (TRUE);
  }
}

#ifdef 0

int 
paramedics(struct char_data *ch, int cmd, char *arg)
{
  struct char_data                *vict,
                                  *most_hurt;

  if (!cmd) {
    if (ch->specials.fighting) {
      return (cleric(ch, 0, ""));
    } else {
      if (GET_POS(ch) == POSITION_STANDING) {

	/*
	 * Find a dude to do good things upon ! 
	 */

	most_hurt = real_roomp(ch->in_room)->people;
	for (vict = real_roomp(ch->in_room)->people; vict;
	     vict = vict->next_in_room) {
	  if (((float)GET_HIT(vict) / (float)hit_limit(vict) <
	    (float)GET_HIT(most_hurt) / (float)hit_limit(most_hurt))
	      && (CAN_SEE(ch, vict)))
	    most_hurt = vict;
	}
	if (!most_hurt)
	  return (FALSE);	/*
				 * nobody here 
				 */

	if ((float)GET_HIT(most_hurt) / (float)hit_limit(most_hurt) >
	    0.66) {
	  if (number(0, 5) == 0) {
	    act("$n shrugs helplessly in unison.", 1, ch, 0, 0, TO_ROOM);
	  }
	  return TRUE;		/*
				 * not hurt enough 
				 */
	}
	if (number(0, 4) == 0) {
	  if (most_hurt != ch) {
	    act("$n looks at $N.", 1, ch, 0, most_hurt, TO_NOTVICT);
	    act("$n looks at you.", 1, ch, 0, most_hurt, TO_VICT);
	  }
	  act("$n utters the words 'judicandus dies' in unison.", 1, ch, 0, 0, TO_ROOM);
	  cast_cure_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, most_hurt, 0);
	  return (TRUE);
	}
      } else {			/*
				 * I'm asleep or sitting 
				 */
	return (FALSE);
      }
    }
  }
  return (FALSE);
}

int 
jugglernaut(struct char_data *ch, int cmd, char *arg)
{
  struct char_data                *vict;
  struct obj_data                 *tmp_obj;
  int                              i,
                                   j;

  if (cmd)
    return (FALSE);

  if (GET_POS(ch) == POSITION_STANDING) {

    if (random() % 3)
      return FALSE;

    /*
     * juggle something 
     */

    if (IS_CARRYING_N(ch) < 1)
      return FALSE;

    i = random() % IS_CARRYING_N(ch);
    j = 0;
    for (tmp_obj = ch->carrying; (tmp_obj) && (j < i); j++) {
      tmp_obj = tmp_obj->next_content;
    }

    if (random() % 6) {
      if (random() % 2) {
	act("$n tosses $p high into the air and catches it.", TRUE, ch, tmp_obj, NULL, TO_ROOM);
      } else {
	act("$n sends $p whirling.", TRUE, ch, tmp_obj, NULL, TO_ROOM);
      }
    } else {
      act("$n tosses $p but fumbles it!", TRUE, ch, tmp_obj, NULL, TO_ROOM);
      obj_from_char(tmp_obj);
      obj_to_room(tmp_obj, ch->in_room);
    }
    return (TRUE);		/*
				 * don't move, I dropped something 
				 */
  } else {			/*
				 * I'm asleep or sitting 
				 */
    return FALSE;
  }
  return (FALSE);
}

static char                     *elf_comm[] =
{
  "wake", "yawn",
  "stand", "say Well, back to work.", "get all",
  "eat bread", "wink",
  "w", "w", "s", "s", "s", "d", "open gate", "e",	/*
							 * home to gate
							 */
  "close gate",
  "e", "e", "e", "e", "n", "w", "n",	/*
					 * gate to baker 
					 */
  "give all.bread baker",	/*
				 * pretend to give a bread 
				 */
  "give all.pastry baker",	/*
				 * pretend to give a pastry 
				 */
  "say That'll be 33 coins, please.",
  "echo The baker gives some coins to the Elf",
  "wave",
  "s", "e", "n", "n", "e", "drop all.bread", "drop all.pastry",
  "w", "s", "s",		/*
				 * to main square 
				 */
  "s", "w", "w", "w", "w",	/*
				 * back to gate 
				 */
  "pat sisyphus",
  "open gate", "w", "close gate", "u", "n", "n", "n", "e", "e",		/*
									 * to home 
									 */
  "say Whew, I'm exhausted.", "rest", "$"};

int 
delivery_beast(struct char_data *ch, int cmd, char *arg)
{
  struct obj_data                 *o;

  if (cmd)
    return (FALSE);

  if (time_info.hours == 6) {
    do_drop(ch, "all.loaf", 0);
    do_drop(ch, "all.biscuit", 0);
  } else if (time_info.hours < 2) {
    if (number(0, 1)) {
      o = read_object(3012, VIRTUAL);
      obj_to_char(o, ch);
    } else {
      o = read_object(3013, VIRTUAL);
      obj_to_char(o, ch);
    }
  } else {
    if (GET_POS(ch) > POSITION_SLEEPING) {
      do_sleep(ch, "", 0);
    }
  }
}

int 
StormGiant(struct char_data *ch, int cmd, char *arg)
{
  struct char_data                *vict;

  if (cmd)
    return (FALSE);

  if (ch->specials.fighting) {
    if ((GET_POS(ch) < POSITION_FIGHTING) &&
	(GET_POS(ch) > POSITION_STUNNED)) {
      StandUp(ch);
    } else {
      if (number(0, 5)) {
	FighterMove(ch);
      } else {
	act("$n creates a lightning bolt", TRUE, ch, 0, 0, TO_ROOM);
	vict = FindVictim(ch);
	if (!vict)
	  return (FALSE);
	cast_lightning_bolt(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);

	/*
	 * do nothing 
	 */
      }
    }
    return (FALSE);
  }
}

int 
firenewt(struct char_data *ch, int cmd, char *arg)
{
  struct char_data                *vict;

  if (cmd)
    return FALSE;

  if (GET_POS(ch) != POSITION_FIGHTING)
    return FALSE;

  for (vict = real_roomp(ch->in_room)->people; vict; vict = vict->next_in_room)
    if ((ch != vict) && (vict->specials.fighting == ch)) {
      break;
    }
  if (!vict)
    return FALSE;

  return TRUE;
}

#endif

int 
eli_priest(struct char_data *ch, int cmd, char *arg)
{
  struct char_data                *vict;

  if (cmd == 257) {		/*
				 * register 
				 */
    while (*arg && isspace(*arg))
      arg++;
    if (!strncmp("me", arg, 2)) {
      SET_BIT(ch->specials.new_act, NEW_PLR_KILLOK);
      send_to_char("You have been register to kill and be killed by players\n\r", ch);
    } else {
      send_to_char("This is a permanent thing you are doing and\n\r", ch);
      send_to_char("will not be changed for ANY reason by the Gods.\n\r", ch);
      send_to_char("If you die by the hands of a player,it is by your\n\r", ch);
      send_to_char("choice and not the Admin of this Game.\n\r", ch);
      send_to_char("To fully register you must enter, 'register me'\n\r", ch);
    }
    return (TRUE);
  }
  if (cmd)
    return FALSE;

  if (GET_POS(ch) == POSITION_FIGHTING)
    return FALSE;

  if ((GET_MANA(ch) < 60) && (GET_POS(ch) != POSITION_SLEEPING)) {
    act("$n falls to the ground into a deep slumber.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POSITION_SLEEPING;
  }
  if ((GET_MANA(ch) > 90) && (GET_POS(ch) != POSITION_STANDING)) {
    GET_POS(ch) = POSITION_STANDING;
    act("$n rises to his feet from a sound sleep.", TRUE, ch, 0, 0, TO_ROOM);
    return TRUE;
  }
  if (GET_POS(ch) == POSITION_SLEEPING)
    return FALSE;

  for (vict = real_roomp(ch->in_room)->people; vict; vict = vict->next_in_room)
    if ((GET_HIT(vict) < 15) &&
	(GET_ALIGNMENT(vict) > 0) &&
	(GET_MAX_HIT(vict) != GET_HIT(vict)) &&
	((GET_POS(vict) != POSITION_FIGHTING) &&
	 (GET_POS(vict) != POSITION_SLEEPING))
      ) {
      act("$n says, You are hurt $N, let me heal you.", TRUE, ch, 0, vict, TO_ROOM);
      break;
    }
  if (!vict) {
    if (number(0, 10 > 6))
      return TRUE;
    switch (number(0, 20)) {
    case 0:
    case 1:
      act("$n says, It is so nice to be back in Shylar!", TRUE, ch, 0, 0, TO_ROOM);
      break;
    case 2:
    case 3:
      act("$n says, I heard the lands were getting tough, so I decide to move a church here.",
	  TRUE, ch, 0, 0, TO_ROOM);
      break;
    case 4:
    case 5:
      act("$n says, Have you been to my parish just north of Pastue's?",
	  TRUE, ch, 0, 0, TO_ROOM);
      break;
    }
    return TRUE;
  }
  say_spell(ch, SPELL_CURE_LIGHT);
  cast_cure_light(20, ch, "", SPELL_TYPE_SPELL, vict, 0);
  GET_MANA(ch) -= 5;
  return TRUE;
}

int 
fountain(struct char_data *ch, int cmd, char *arg)
{
  char                             buf[MAX_STRING_LENGTH],
                                   tmp[MAX_INPUT_LENGTH];

  if (cmd != 11)
    return (0);

  one_argument(arg, tmp);

  if (!*tmp || !isname(tmp, "water barrel fountain pool"))
    return (0);

  if (GET_COND(ch, THIRST) != -1) {
    if (GetMaxLevel(ch) > 50) {
      GET_COND(ch, THIRST) = -1;
    } else {
      GET_COND(ch, THIRST) += 8;
      GET_COND(ch, THIRST) = MIN(24, GET_COND(ch, THIRST));
      sprintf(buf, "$n drinks from the %s.", tmp);
      act(buf, TRUE, ch, 0, 0, TO_ROOM);
      if (GET_COND(ch, THIRST) == 24) {
	send_to_char("You are full!\n", ch);
      } else
	send_to_char("You drink the cool water.\n", ch);
    }
  }
  return 1;
}

int 
RangerGuildMaster(struct char_data *ch, int cmd, char *arg)
{
  int                              number,
                                   number2,
                                   i,
                                   percent;
  char                             buf[MAX_INPUT_LENGTH];
  struct char_data                *guildmaster;

#define RANGER_CAST_LEVEL 10

  struct skill_struct {
    char                             skill_name[40];
    int                              skill_numb;
    int                              skill_lvl;
  };

  struct skill_struct              r_skills[] =
  {
    {"rescue", SKILL_RESCUE, 0},
    {"hide", SKILL_HIDE, 0},
    {"sneak", SKILL_SNEAK, 0},
    {"track", SKILL_HUNT, 0},
    {"disarm", SKILL_DISARM, 0},
    {"punch", SKILL_PUNCH, 0},
    {"bash", SKILL_BASH, 0},
    {"doorbash", SKILL_DOOR_BASH, 0},
    {"search", SKILL_SEARCH, 0},
/*
 * { "listen",   SKILL_DETECT_NOISE,0}, 
 */
    {"peer", SKILL_PEER, 0},
    {"\n", -1}
  };

  struct skill_struct              r_spells[] =
  {
    {"armor", SPELL_ARMOR, 10},
    {"create food", SPELL_CREATE_FOOD, 10},
    {"create water", SPELL_CREATE_WATER, 10},
    {"cure light", SPELL_CURE_LIGHT, 10},
    {"refresh", SPELL_REFRESH, 11},
    {"faerie fire", SPELL_FAERIE_FIRE, 11},
    {"faerie fog", SPELL_FAERIE_FOG, 11},
    {"stone skin", SPELL_STONE_SKIN, 12},
    {"second wind", SPELL_SECOND_WIND, 12},
    {"cure serious", SPELL_CURE_SERIOUS, 14},
    {"\n", -1, -1}
  };

  char                            *rl_skills[] =
  {
    "rescue",
    "hide",
    "sneak",
    "track",
    "disarm",
    "punch",
    "bash",
    "doorbash",
    "search",
/*
 * "listen", 
 */
    "peer",
    "\n"
  };

  char                            *rl_spells[] =
  {
    "armor",
    "create food",
    "create water",
    "cure light",
    "refresh",
    "faerie fire",
    "faerie fog",
    "stone skin",
    "second wind",
    "cure serious",
    "\n"
  };

  if ((cmd != 164) && (cmd != 170) && (cmd != 243))
    return (FALSE);

  guildmaster = FindMobInRoomWithFunction(ch->in_room, RangerGuildMaster);
  if (!guildmaster)
    return (FALSE);

  if (HasClass(ch, CLASS_RANGER)) {
    if (cmd == 243) {		/*
				 * gain 
				 */
      if (GET_LEVEL(ch, RANGER_LEVEL_IND) <= (GetMaxLevel(guildmaster) - 10)) {
	GainLevel(ch, RANGER_LEVEL_IND);
      } else {
	send_to_char("I cannot train you.. You must find another.\n\r", ch);
      }
      return (TRUE);
    }
    if (!*arg) {
      sprintf(buf, "You have got %d practice sessions left.\n\r", ch->specials.spells_to_learn);
      send_to_char(buf, ch);
      send_to_char("You can practise any of these skills:\n\r", ch);
      for (i = 0; r_skills[i].skill_name[0] != '\n'; i++) {
	send_to_char(r_skills[i].skill_name, ch);
	send_to_char(how_good(ch->skills[r_skills[i].skill_numb].learned), ch);
	send_to_char("\n\r", ch);
      }

      if (GET_LEVEL(ch, RANGER_LEVEL_IND) >= 10) {
	send_to_char("\n\r\n\r", ch);
	for (i = 0; r_spells[i].skill_name[0] != '\n'; i++) {
	  if (r_spells[i].skill_lvl <= GET_LEVEL(ch, RANGER_LEVEL_IND)) {
	    send_to_char(r_spells[i].skill_name, ch);
	    send_to_char(how_good(ch->skills[r_spells[i].skill_numb].learned), ch);
	    send_to_char("\n\r", ch);
	  }
	}
      }
      return (TRUE);
    }
    for (; isspace(*arg); arg++);
    number = search_block(arg, rl_skills, FALSE);
    number2 = search_block(arg, rl_spells, FALSE);
    if ((number == -1) && (number2 == -1)) {
      send_to_char("You do not have ability to practise this skill!\n\r", ch);
      return (TRUE);
    }
    if (ch->specials.spells_to_learn <= 0) {
      send_to_char("You do not seem to be able to practice now.\n\r", ch);
      return (TRUE);
    }
    if (number != -1) {
      if (ch->skills[r_skills[number].skill_numb].learned >= 95) {
	send_to_char("You are already learned in this area.\n\r", ch);
	return (TRUE);
      }
    } else {
      if (ch->skills[r_spells[number2].skill_numb].learned >= 95) {
	send_to_char("You are already learned in this area.\n\r", ch);
	return (TRUE);
      }
      if (GET_LEVEL(ch, RANGER_LEVEL_IND) < r_spells[number2].skill_lvl) {
	send_to_char("You do not know of this skill.....\n\r", ch);
	return (TRUE);
      }
    }

    send_to_char("You Practice for a while...\n\r", ch);
    ch->specials.spells_to_learn--;

    if (number != -1) {
      percent = ch->skills[r_skills[number].skill_numb].learned
	+ int_app[GET_INT(ch)].learn;
      ch->skills[r_skills[number].skill_numb].learned = MIN(95, percent);
    } else {
      percent = ch->skills[r_spells[number2].skill_numb].learned
	+ int_app[GET_WIS(ch)].learn;
      ch->skills[r_spells[number2].skill_numb].learned = MIN(95, percent);
    }

    if (number != -1) {
      if (ch->skills[r_skills[number].skill_numb].learned >= 95) {
	send_to_char("You are now a master in this area.\n\r", ch);
	return (TRUE);
      }
    } else {
      if (ch->skills[r_spells[number2].skill_numb].learned >= 95) {
	send_to_char("You are now a master in this area.\n\r", ch);
	return (TRUE);
      }
    }
  } else {
    send_to_char("Oh.. i bet you think you're a Ranger??\n\r", ch);
    return (FALSE);
  }
}

int 
do_skills(struct char_data *ch, int cmd, char *arg)
{
  int                              number,
                                   number2,
                                   i,
                                   percent;
  char                             buf[MAX_INPUT_LENGTH];
  struct char_data                *guildmaster;

  struct skill_struct {
    char                             skill_name[40];
    int                              skill_numb;
    int                              skill_class;
    int                              skill_lvl;
  };

  struct skill_struct              r_skills[] =
  {
    {"swimming", SKILL_SWIMMING, CLASS_ALL, 1},
    {"bandage", SKILL_BANDAGE, CLASS_ALL, 1},
    {"track", SKILL_HUNT, CLASS_WARRIOR | CLASS_MAGIC_USER | CLASS_CLERIC, 3},
    {"riding", SKILL_RIDE, CLASS_ALL, 1},
    {"deciphering", SKILL_READ_MAGIC, CLASS_ALL, 3},
    {"endurance", SKILL_ENDURANCE, CLASS_ALL, 3},
    {"two_hand", SKILL_TWO_HANDED, CLASS_ALL, 5},
    {"brew", SKILL_BREW, CLASS_MAGIC_USER | CLASS_CLERIC, 65},
    {"scribe", SKILL_SCRIBE, CLASS_MAGIC_USER | CLASS_CLERIC, 65},
    {"punch", SKILL_PUNCH, CLASS_CLERIC, 2},
    {"bare_hand", SKILL_BARE_HAND, CLASS_ALL, 1},
    {"apraise", SKILL_APRAISE, CLASS_ALL, 2},
    {"bartering", SKILL_BARTER, CLASS_MAGIC_USER | CLASS_CLERIC | CLASS_THIEF, 65},
    {"spell_craft", SKILL_SPELLCRAFT, CLASS_MAGIC_USER | CLASS_CLERIC, 7},
    {"meditation", SKILL_MEDITATION, CLASS_MAGIC_USER | CLASS_CLERIC, 7},
    {"\n", -1}
  };

  char                            *rl_skills[] =
  {
    "swimming",
    "bandage",
    "track",
    "riding",
    "deciphering",
    "endurance",
    "two_hand",
    "brew",
    "scribe",
    "punch",
    "bare_hand",
    "apraise",
    "bartering",
    "spell_craft",
    "meditation",
    "\n"
  };

  sprintf(buf, "You have got %d practice sessions left.\n\r", ch->specials.spells_to_learn);
  send_to_char(buf, ch);
  for (i = 0; r_skills[i].skill_name[0] != '\n'; i++) {
    if (r_skills[i].skill_lvl <= GetMaxLevel(ch) || IS_IMMORTAL(ch)) {
      if ((IS_SET(ch->player.class, r_skills[i].skill_class) &&
      GetMaxLevel(ch) > r_skills[i].skill_lvl) || IS_IMMORTAL(ch)) {
	send_to_char(r_skills[i].skill_name, ch);
	send_to_char(how_good(ch->skills[r_skills[i].skill_numb].learned), ch);
	send_to_char("\n\r", ch);
      }
    }
  }
}

int 
GenericGuildMaster(struct char_data *ch, int cmd, char *arg)
{
  int                              number,
                                   number2,
                                   i,
                                   percent;
  char                             buf[MAX_INPUT_LENGTH];
  struct char_data                *guildmaster;

  struct skill_struct {
    char                             skill_name[40];
    int                              skill_numb;
    int                              skill_class;
    int                              skill_lvl;
  };

  struct skill_struct              r_skills[] =
  {
    {"swimming", SKILL_SWIMMING, CLASS_ALL, 1},
    {"bandage", SKILL_BANDAGE, CLASS_ALL, 1},
    {"track", SKILL_HUNT, CLASS_WARRIOR | CLASS_MAGIC_USER | CLASS_CLERIC, 3},
    {"riding", SKILL_RIDE, CLASS_ALL, 1},
    {"deciphering", SKILL_READ_MAGIC, CLASS_ALL, 3},
    {"endurance", SKILL_ENDURANCE, CLASS_ALL, 3},
    {"two_hand", SKILL_TWO_HANDED, CLASS_ALL, 5},
    {"brew", SKILL_BREW, CLASS_MAGIC_USER | CLASS_CLERIC, 65},
    {"scribe", SKILL_SCRIBE, CLASS_MAGIC_USER | CLASS_CLERIC, 65},
    {"punch", SKILL_PUNCH, CLASS_CLERIC, 2},
    {"bare_hand", SKILL_BARE_HAND, CLASS_ALL, 1},
    {"apraise", SKILL_APRAISE, CLASS_ALL, 2},
    {"bartering", SKILL_BARTER, CLASS_MAGIC_USER | CLASS_CLERIC | CLASS_THIEF, 65},
    {"spell_craft", SKILL_SPELLCRAFT, CLASS_MAGIC_USER | CLASS_CLERIC, 7},
    {"meditation", SKILL_MEDITATION, CLASS_MAGIC_USER | CLASS_CLERIC, 7},
    {"\n", -1}
  };

  char                            *rl_skills[] =
  {
    "swimming",
    "bandage",
    "track",
    "riding",
    "deciphering",
    "endurance",
    "two_hand",
    "brew",
    "scribe",
    "punch",
    "bare_hand",
    "apraise",
    "bartering",
    "spell_craft",
    "meditation",
    "\n"
  };

  if ((cmd != 164) && (cmd != 170) && (cmd != 243))
    return (FALSE);

  guildmaster = FindMobInRoomWithFunction(ch->in_room, GenericGuildMaster);

  if (!guildmaster)
    return (FALSE);

  if (cmd == 243) {
    send_to_char("I cannot train you.. You must find another.\n\r", ch);
    return (TRUE);
  }
  if (!*arg) {
    sprintf(buf, "You have got %d practice sessions left.\n\r", ch->specials.spells_to_learn);
    send_to_char(buf, ch);
    send_to_char("You can practise any of these skills:\n\r", ch);
    for (i = 0; r_skills[i].skill_name[0] != '\n'; i++) {
      if (r_skills[i].skill_lvl <= GetMaxLevel(ch) || IS_IMMORTAL(ch)) {
	if ((IS_SET(ch->player.class, r_skills[i].skill_class) &&
	     GetMaxLevel(ch) >= r_skills[i].skill_lvl) || IS_IMMORTAL(ch)) {
	  send_to_char(r_skills[i].skill_name, ch);
	  send_to_char(how_good(ch->skills[r_skills[i].skill_numb].learned), ch);
	  send_to_char("\n\r", ch);
	}
      }
    }
    return (TRUE);
  } else {
    for (; isspace(*arg); arg++);
    number = search_block(arg, rl_skills, FALSE);

    if ((number == -1)) {
      send_to_char("You do not have ability to practise this skill!\n\r", ch);
      return (TRUE);
    }
    if (ch->specials.spells_to_learn <= 0) {
      send_to_char("You do not seem to be able to practice now.\n\r", ch);
      return (TRUE);
    }
    if (number != -1) {
      if (ch->skills[r_skills[number].skill_numb].learned >= 95) {
	send_to_char("You are already learned in this area.\n\r", ch);
	return (TRUE);
      }
    }
    if (r_skills[number].skill_lvl > GetMaxLevel(ch) && !IS_IMMORTAL(ch)) {
      send_to_char("You do not know of this skill...\n\r", ch);
      return (TRUE);
    }
    send_to_char("You Practice for a while...\n\r", ch);
    ch->specials.spells_to_learn--;

    if (number != -1) {
      percent = ch->skills[r_skills[number].skill_numb].learned + int_app[GET_INT(ch)].learn;
      ch->skills[r_skills[number].skill_numb].learned = MIN(95, percent);
    }
    if (number != -1) {
      if (ch->skills[r_skills[number].skill_numb].learned >= 95) {
	send_to_char("You are now a master in this area.\n\r", ch);
	return (TRUE);
      }
    }
  }
}

/*
 * file: Interpreter.c , Command interpreter module.      Part of DIKUMUD
 * Usage: Procedures interpreting user command
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <arpa/telnet.h>

#include "global.h"
#include "bug.h"
#include "comm.h"
#include "version.h"
#include "db.h"
#include "utils.h"
#include "limits.h"
#include "constants.h"
#include "act_comm.h"
#include "act_info.h"
#include "act_move.h"
#include "act_obj.h"
#include "act_off.h"
#include "act_other.h"
#include "act_skills.h"
#include "act_social.h"
#include "act_wiz.h"
#include "spells.h"
#include "spell_parser.h"
#include "modify.h"
#include "whod.h"
#define _INTERPRETER_C
#include "interpreter.h"

struct command_info cmd_info[MAX_CMD_LIST];

char echo_on[] =
{
  IAC, WONT, TELOPT_ECHO, '\0'};
char echo_off[] =
{
  IAC, WILL, TELOPT_ECHO, '\0'};
int WizLock;

char *command[] =
{
  "north",	       /* 1 */
  "east",
  "south",
  "west",
  "up",
  "down",
  "enter",
  "exits",
  "kiss",
  "get",
  "drink",	       /* 11 */
  "eat",
  "wear",
  "wield",
  "look",
  "score",
  "say",
  "shout",
  "tell",
  "inventory",
  "qui",	       /* 21 */
  "bounce",
  "smile",
  "dance",
  "kill",
  "cackle",
  "laugh",
  "giggle",
  "shake",
  "puke",
  "growl",	       /* 31 */
  "scream",
  "insult",
  "comfort",
  "nod",
  "sigh",
  "sulk",
  "help",
  "who",
  "emote",
  "echo",	       /* 41 */
  "stand",
  "sit",
  "rest",
  "sleep",
  "wake",
  "force",
  "transfer",
  "hug",
  "snuggle",
  "cuddle",	       /* 51 */
  "nuzzle",
  "cry",
  "news",
  "equipment",
  "buy",
  "sell",
  "value",
  "list",
  "drop",
  "goto",	       /* 61 */
  "weather",
  "read",
  "pour",
  "grab",
  "remove",
  "put",
  "shutdow",
  "save",
  "hit",
  "string",	       /* 71 */
  "give",
  "quit",
  "stat",
  "guard",
  "time",
  "load",
  "purge",
  "shutdown",
  "idea",
  "typo",	       /* 81 */
  "bug",
  "whisper",
  "cast",
  "at",
  "ask",
  "order",
  "sip",
  "taste",
  "snoop",
  "follow",	       /* 91 */
  "rent",
  "offer",
  "poke",
  "advance",
  "accuse",
  "grin",
  "bow",
  "open",
  "close",
  "lock",	       /* 101 */
  "unlock",
  "leave",
  "applaud",
  "blush",
  "burp",
  "chuckle",
  "clap",
  "cough",
  "curtsey",
  "fart",	       /* 111 */
  "flip",
  "fondle",
  "frown",
  "gasp",
  "glare",
  "groan",
  "grope",
  "hiccup",
  "lick",
  "love",	       /* 121 */
  "moan",
  "nibble",
  "pout",
  "purr",
  "ruffle",
  "shiver",
  "shrug",
  "sing",
  "slap",
  "smirk",	       /* 131 */
  "snap",
  "sneeze",
  "snicker",
  "sniff",
  "snore",
  "spit",
  "squeeze",
  "stare",
  "strut",
  "thank",	       /* 141 */
  "twiddle",
  "wave",
  "whistle",
  "wiggle",
  "wink",
  "yawn",
  "snowball",
  "write",
  "hold",
  "flee",	       /* 151 */
  "sneak",
  "hide",
  "backstab",
  "pick",
  "steal",
  "bash",
  "rescue",
  "kick",
  "french",
  "comb",	       /* 161 */
  "massage",
  "tickle",
  "practice",
  "pat",
  "examine",
  "take",
  "info",
  "'",
  "practise",
  "curse",	       /* 171 */
  "use",
  "where",
  "levels",
  "reroll",
  "pray",
  ",",
  "beg",
  "bleed",
  "cringe",
  "daydream",	       /* 181 */
  "fume",
  "grovel",
  "hop",
  "nudge",
  "peer",
  "point",
  "ponder",
  "punch",
  "snarl",
  "spank",	       /* 191 */
  "steam",
  "tackle",
  "taunt",
  "think",
  "whine",
  "worship",
  "yodel",
  "brief",
  "wizlist",
  "consider",	       /* 201 */
  "group",
  "restore",
  "return",
  "switch",	       /* 205 */
  "quaff",
  "recite",
  "users",
  "pose",
  "noshout",
  "wizhelp",	       /* 211 */
  "credits",	       /* 212 */
  "compact",	       /* 213 */
  ":",		       /* emote 214 */
  "hermit",	       /* 215 */
  "slay",	       /* instead of "kill" for immorts (8/16) 216 */
  "wimp",	       /* 217 */
  "junk",	       /* 218 */
  "deposit",	       /* 219 * 9 - 4ish */
  "withdraw",	       /* 220 */
  "balance",
  "nohassle",	       /* 9 - 6 */
  "system",	       /* 9 - 16 */
  "pull",
  "stealth",	       /* 225 */
  "cust",	       /* 226 */
  "pset",	       /* 227 */
  "sroom",	       /* 228 */
  "lroom",	       /* 229 */
  "track",	       /* 230 */
  "wizlock",	       /* 231 */
  "highfive",	       /* 232 */
  "title",	       /* 233 */
  "whozone",	       /* 234 */
  "assist",	       /* 235 */
  "swat",	       /* 236 */
  "world",	       /* 237 */
  "allspells",	       /* 238 */
  "breath",	       /* 239 */
  "show",	       /* 240 */
  "debug",	       /* 241 */
  "invisible",	       /* 242 */
  "gain",	       /* 243 */
  "mkzone",	       /* 244 */
  "disarm",	       /* 245 */
  "bonk",	       /* 246 */
  "wiznet",	       /* 247 */
  "form",	       /* 248 */
  "gtell",	       /* 249 */
  "pretitle",	       /* 250 */
  "allcommands",       /* 251 */
  "grep",	       /* 252 */
  "pager",	       /* 253 */
  "appear",	       /* 254 */
  "logs",	       /* 255 */
  "sethome",	       /* 256 */
  "register",	       /* 257 */
  "send",	       /* 258 */
  "whod",	       /* 259 */
  "split",	       /* 260 */
  "notell",	       /* 261 */
  "scribe",	       /* 262 */
  "apraise",	       /* 263 */
  "bandage",	       /* 264 */
  "search",	       /* 265 */
  "skills",	       /* 266 */
  "doorbash",	       /* 267 */
  "restoreall",	       /* 268 */
  "mount",	       /* 269 */
  "dismount",	       /* 270 */
  "land",	       /* 271 */
  "nosummon",	       /* 272 */
  "noteleport",	       /* 273 */
  "\n"
};

char *fill[] =
{
  "in",
  "from",
  "with",
  "the",
  "on",
  "at",
  "to",
  "\n"
};

int search_block(char *arg, char **list, bool exact)
{
  register int i, l;

  /* Make into lower case, and get length of string */
  for (l = 0; *(arg + l); l++)
    *(arg + l) = LOWER(*(arg + l));

  if (exact) {
    for (i = 0; **(list + i) != '\n'; i++)
      if (!strcmp(arg, *(list + i)))
	return (i);
  } else {
    if (!l)
      l = 1;	       /* Avoid "" to match the first available string */
    for (i = 0; **(list + i) != '\n'; i++)
      if (!strncmp(arg, *(list + i), l))
	return (i);
  }
  return (-1);
}

int old_search_block(char *argument, int begin, int length, char **list, int mode)
{
  int guess, found, search;

  /* If the word contain 0 letters, then a match is already found */
  found = (length < 1);

  guess = 0;

  /* Search for a match */

  if (mode)
    while (NOT found AND * (list[guess]) != '\n') {
      found = (length == strlen(list[guess]));
      for (search = 0; (search < length AND found); search++)
	found = (*(argument + begin + search) == *(list[guess] + search));
      guess++;
  } else {
    while (NOT found AND * (list[guess]) != '\n') {
      found = 1;
      for (search = 0; (search < length AND found); search++)
	found = (*(argument + begin + search) == *(list[guess] + search));
      guess++;
    }
  }

  return (found ? guess : -1);
}

void command_interpreter(struct char_data *ch, char *argument)
{
  int look_at, cmd, begin;
  char buf[200];
  extern int no_specials;
  extern struct char_data *board_kludge_char;
  char debug_buf[512];

  REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);

  if (ch == board_kludge_char) {
    board_save_board(FindBoardInRoom(ch->in_room));
    board_kludge_char = 0;
  }
  /* Find first non blank */
  for (begin = 0; (*(argument + begin) == ' '); begin++);

  /* Find length of first word */
  for (look_at = 0; *(argument + begin + look_at) > ' '; look_at++) {
    /* Make all letters lower case AND find length */
    *(argument + begin + look_at) = LOWER(*(argument + begin + look_at));
  }

  if (*(argument + begin) == '\'')
    look_at = begin + 1;

  cmd = old_search_block(argument, begin, look_at, command, 0);

  if (!cmd)
    return;

  if (cmd > 0 && GetMaxLevel(ch) < cmd_info[cmd].minimum_level) {
    send_to_char("Arglebargle, glop-glyf!?!\n\r", ch);
    return;
  }
  if (cmd > 0 && (cmd_info[cmd].command_pointer != 0)) {
    if ((!IS_AFFECTED(ch, AFF_PARALYSIS)) || (cmd_info[cmd].minimum_position <= POSITION_STUNNED)) {
      if (GET_POS(ch) < cmd_info[cmd].minimum_position) {
	switch (GET_POS(ch)) {
	case POSITION_DEAD:
	  send_to_char("Lie still; you are DEAD!!! :-( \n\r", ch);
	  break;
	case POSITION_INCAP:
	case POSITION_MORTALLYW:
	  send_to_char(
			"You are in a pretty bad shape, unable to do anything!\n\r",
			ch);
	  break;

	case POSITION_STUNNED:
	  send_to_char(
			"All you can do right now, is think about the stars!\n\r", ch);
	  break;
	case POSITION_SLEEPING:
	  send_to_char("In your dreams, or what?\n\r", ch);
	  break;
	case POSITION_RESTING:
	  send_to_char("Nah... You feel too relaxed to do that..\n\r",
		       ch);
	  break;
	case POSITION_SITTING:
	  send_to_char("Maybe you should get on your feet first?\n\r", ch);
	  break;
	case POSITION_FIGHTING:
	  send_to_char("No way! You are fighting for your life!\n\r", ch);
	  break;
	}
      } else {

	if (!no_specials && special(ch, cmd, argument + begin + look_at))
	  return;

	((*cmd_info[cmd].command_pointer) (ch, argument + begin + look_at, cmd));

	if ((GetMaxLevel(ch) >= LOW_IMMORTAL) && (GetMaxLevel(ch) < IMPLEMENTOR)) {
	  sprintf(buf, "%s:%s", ch->player.name, argument);
	  log(buf);
	}
      }
      return;
    } else {
      send_to_char(" You are paralyzed, you can't do much of anything!\n\r", ch);
      return;
    }
  }
  if (check_exit_alias(ch, argument))
    return;

  if (cmd > 0 && (cmd_info[cmd].command_pointer == 0))
    send_to_char("Sorry, that command has yet to be implemented...\n\r", ch);
  else
    send_to_char("Pardon? \n\r", ch);
}

void argument_interpreter(char *argument, char *first_arg, char *second_arg)
{
  int look_at, found, begin;

  found = begin = 0;

  do {
    /* Find first non blank */
    for (; *(argument + begin) == ' '; begin++);

    /* Find length of first word */
    /* Make all letters lower case, AND copy them to first_arg */
    for (look_at = 0; *(argument + begin + look_at) > ' '; look_at++)
      *(first_arg + look_at) = LOWER(*(argument + begin + look_at));

    *(first_arg + look_at) = '\0';
    begin += look_at;

  } while (fill_word(first_arg));

  do {
    /* Find first non blank */
    for (; *(argument + begin) == ' '; begin++);

    /* Find length of first word */
    /* Make all letters lower case, AND copy them to second_arg */
    for (look_at = 0; *(argument + begin + look_at) > ' '; look_at++)
      *(second_arg + look_at) =
	LOWER(*(argument + begin + look_at));

    *(second_arg + look_at) = '\0';
    begin += look_at;

  } while (fill_word(second_arg));
}

int is_number(char *str)
{
  int look_at;

  if (*str == '\0')
    return (0);

  for (look_at = 0; *(str + look_at) != '\0'; look_at++)
    if ((*(str + look_at) < '0') || (*(str + look_at) > '9'))
      return (0);
  return (1);
}

/*
 * Quinn substituted a new one-arg for the old one.. I thought returning a 
 * char pointer would be neat, and avoiding the func-calls would save a
 * little time... If anyone feels pissed, I'm sorry.. Anyhow, the code is
 * snatched from the old one, so it outta work..
 * 
 * void one_argument(char *argument,char *first_arg )
 * {
 * static char dummy[MAX_STRING_LENGTH];
 * 
 * argument_interpreter(argument,first_arg,dummy);
 * }
 * 
 */

/*
 * find the first sub-argument of a string, return pointer to first char in
 * primary argument, following the sub-arg                  
 */
char *one_argument(char *argument, char *first_arg)
{
  int found, begin, look_at;

  found = begin = 0;

  do {
    for (; isspace(*(argument + begin)); begin++);

    for (look_at = 0; *(argument + begin + look_at) > ' '; look_at++)
      *(first_arg + look_at) = LOWER(*(argument + begin + look_at));

    *(first_arg + look_at) = '\0';
    begin += look_at;
  } while (fill_word(first_arg));

  return (argument + begin);
}

void only_argument(char *argument, char *dest)
{
  while (*argument && isspace(*argument))
    argument++;
  strcpy(dest, argument);
}

int fill_word(char *argument)
{
  return (search_block(argument, fill, TRUE) >= 0);
}

/*
 * determine if a given string is an abbreviation of another 
 */
int is_abbrev(char *arg1, char *arg2)
{
  if (!*arg1)
    return (0);

  for (; *arg1; arg1++, arg2++)
    if (LOWER(*arg1) != LOWER(*arg2))
      return (0);

  return (1);
}

/*
 * return first 'word' plus trailing substring of input string 
 */
void half_chop(char *string, char *arg1, char *arg2)
{
  for (; isspace(*string); string++);
  for (; !isspace(*arg1 = *string) && *string; string++, arg1++);
  *arg1 = '\0';
  for (; isspace(*string); string++);
  for (; *arg2 = *string; string++, arg2++);
}

int special(struct char_data *ch, int cmd, char *arg)
{
  register struct obj_data *i;
  register struct char_data *k;
  int j, test;

  if (ch->in_room == NOWHERE) {
    char_to_room(ch, 3001);
    return;
  }
  /* special in room?   */
  if (real_roomp(ch->in_room)->funct)
    if ((*real_roomp(ch->in_room)->funct) (ch, cmd, arg))
      return (1);

  /* special in equipment list?   */
  for (j = 0; j <= (MAX_WEAR - 1); j++)
    if (ch->equipment[j] && ch->equipment[j]->item_number >= 0)
      if (obj_index[ch->equipment[j]->item_number].func)
	if ((*obj_index[ch->equipment[j]->item_number].func) (ch, cmd, arg))
	  return (1);

  test++;
  /* special in inventory?   */
  for (i = ch->carrying; i; i = i->next_content)
    if (i->item_number >= 0)
      if (obj_index[i->item_number].func)
	if ((*obj_index[i->item_number].func) (ch, cmd, arg))
	  return (1);

  test++;

  /* special in mobile present?   */
  for (k = real_roomp(ch->in_room)->people; k; k = k->next_in_room)
    if (IS_MOB(k))
      if (mob_index[k->nr].func)
	if ((*mob_index[k->nr].func) (ch, cmd, arg))
	  return (1);

  test++;

  /* special in object present?   */
  for (i = real_roomp(ch->in_room)->contents; i; i = i->next_content)
    if (i->item_number >= 0)
      if (obj_index[i->item_number].func)
	if ((*obj_index[i->item_number].func) (ch, cmd, arg))
	  return (1);

  test++;

  return (0);
}

void assign_command_pointers(void)
{
  int position;

  for (position = 0; position < MAX_CMD_LIST; position++)
    cmd_info[position].command_pointer = 0;

  COMMANDO(1, POSITION_STANDING, do_move, 0);
  COMMANDO(2, POSITION_STANDING, do_move, 0);
  COMMANDO(3, POSITION_STANDING, do_move, 0);
  COMMANDO(4, POSITION_STANDING, do_move, 0);
  COMMANDO(5, POSITION_STANDING, do_move, 0);
  COMMANDO(6, POSITION_STANDING, do_move, 0);
  COMMANDO(7, POSITION_STANDING, do_enter, 0);
  COMMANDO(8, POSITION_RESTING, do_exits, 0);
  COMMANDO(9, POSITION_RESTING, do_action, 0);
  COMMANDO(10, POSITION_RESTING, do_get, 0);
  COMMANDO(11, POSITION_RESTING, do_drink, 0);
  COMMANDO(12, POSITION_RESTING, do_eat, 0);
  COMMANDO(13, POSITION_RESTING, do_wear, 0);
  COMMANDO(14, POSITION_RESTING, do_wield, 0);
  COMMANDO(15, POSITION_RESTING, do_look, 0);
  COMMANDO(16, POSITION_DEAD, do_score, 0);
  COMMANDO(17, POSITION_RESTING, do_say, 0);
  COMMANDO(18, POSITION_RESTING, do_shout, 0);
  COMMANDO(19, POSITION_DEAD, do_tell, 0);
  COMMANDO(20, POSITION_DEAD, do_inventory, 0);
  COMMANDO(21, POSITION_DEAD, do_qui, 0);
  COMMANDO(22, POSITION_STANDING, do_action, 0);
  COMMANDO(23, POSITION_RESTING, do_action, 0);
  COMMANDO(24, POSITION_STANDING, do_action, 0);
  COMMANDO(25, POSITION_FIGHTING, do_kill, 0);
  COMMANDO(26, POSITION_RESTING, do_action, 0);
  COMMANDO(27, POSITION_RESTING, do_action, 0);
  COMMANDO(28, POSITION_RESTING, do_action, 0);
  COMMANDO(29, POSITION_RESTING, do_action, 0);
  COMMANDO(30, POSITION_RESTING, do_action, 0);
  COMMANDO(31, POSITION_RESTING, do_action, 0);
  COMMANDO(32, POSITION_RESTING, do_action, 0);
  COMMANDO(33, POSITION_RESTING, do_insult, 0);
  COMMANDO(34, POSITION_RESTING, do_action, 0);
  COMMANDO(35, POSITION_RESTING, do_action, 0);
  COMMANDO(36, POSITION_RESTING, do_action, 0);
  COMMANDO(37, POSITION_RESTING, do_action, 0);
  COMMANDO(38, POSITION_DEAD, do_help, 0);
  COMMANDO(39, POSITION_DEAD, do_who, 0);
  COMMANDO(40, POSITION_SLEEPING, do_emote, 1);
  COMMANDO(41, POSITION_SLEEPING, do_echo, 1);
  COMMANDO(42, POSITION_RESTING, do_stand, 0);
  COMMANDO(43, POSITION_RESTING, do_sit, 0);
  COMMANDO(44, POSITION_RESTING, do_rest, 0);
  COMMANDO(45, POSITION_SLEEPING, do_sleep, 0);
  COMMANDO(46, POSITION_SLEEPING, do_wake, 0);
  COMMANDO(49, POSITION_RESTING, do_action, 0);
  COMMANDO(50, POSITION_RESTING, do_action, 0);
  COMMANDO(51, POSITION_RESTING, do_action, 0);
  COMMANDO(52, POSITION_RESTING, do_action, 0);
  COMMANDO(53, POSITION_RESTING, do_action, 0);
  COMMANDO(54, POSITION_SLEEPING, do_news, 0);
  COMMANDO(55, POSITION_SLEEPING, do_equipment, 0);
  COMMANDO(56, POSITION_STANDING, do_not_here, 0);
  COMMANDO(57, POSITION_STANDING, do_not_here, 0);
  COMMANDO(58, POSITION_STANDING, do_not_here, 0);
  COMMANDO(59, POSITION_STANDING, do_not_here, 0);
  COMMANDO(60, POSITION_RESTING, do_drop, 0);
  COMMANDO(62, POSITION_RESTING, do_weather, 0);
  COMMANDO(63, POSITION_RESTING, do_read, 0);
  COMMANDO(64, POSITION_STANDING, do_pour, 0);
  COMMANDO(65, POSITION_RESTING, do_grab, 0);
  COMMANDO(66, POSITION_RESTING, do_remove, 0);
  COMMANDO(67, POSITION_RESTING, do_put, 0);
  COMMANDO(69, POSITION_SLEEPING, do_save, 0);
  COMMANDO(70, POSITION_FIGHTING, do_hit, 0);
  COMMANDO(72, POSITION_RESTING, do_give, 0);
  COMMANDO(73, POSITION_DEAD, do_quit, 0);
  COMMANDO(75, POSITION_STANDING, do_guard, 1);
  COMMANDO(260, POSITION_RESTING, do_split, 1);
  COMMANDO(263, POSITION_RESTING, do_apraise, 1);
  COMMANDO(264, POSITION_FIGHTING, do_bandage, 1);
  COMMANDO(265, POSITION_STANDING, do_search, 1);
  COMMANDO(266, POSITION_STANDING, do_skills, 1);
  COMMANDO(267, POSITION_STANDING, do_doorbash, 1);
  COMMANDO(269, POSITION_STANDING, do_mount, 1);
  COMMANDO(270, POSITION_MOUNTED, do_mount, 1);
  COMMANDO(271, POSITION_DEAD, do_land, 1);
  COMMANDO(186, POSITION_STANDING, do_peer, 1);
  COMMANDO(76, POSITION_DEAD, do_time, 0);
  COMMANDO(80, POSITION_DEAD, do_idea, 0);
  COMMANDO(81, POSITION_DEAD, do_typo, 0);
  COMMANDO(82, POSITION_DEAD, do_bug, 0);
  COMMANDO(83, POSITION_RESTING, do_whisper, 0);
  COMMANDO(84, POSITION_SITTING, do_cast, 1);
  COMMANDO(86, POSITION_RESTING, do_ask, 0);
  COMMANDO(87, POSITION_RESTING, do_order, 1);
  COMMANDO(88, POSITION_RESTING, do_sip, 0);
  COMMANDO(89, POSITION_RESTING, do_taste, 0);
  COMMANDO(91, POSITION_RESTING, do_follow, 0);
  COMMANDO(92, POSITION_STANDING, do_not_here, 1);
  COMMANDO(93, POSITION_STANDING, do_not_here, 1);
  COMMANDO(94, POSITION_RESTING, do_action, 0);
  COMMANDO(256, POSITION_STANDING, do_not_here, 1);
  COMMANDO(257, POSITION_STANDING, do_not_here, 3);
  COMMANDO(258, POSITION_STANDING, do_not_here, 3);
  COMMANDO(96, POSITION_SITTING, do_action, 0);
  COMMANDO(97, POSITION_RESTING, do_action, 0);
  COMMANDO(98, POSITION_STANDING, do_action, 0);
  COMMANDO(99, POSITION_SITTING, do_open, 0);
  COMMANDO(100, POSITION_SITTING, do_close, 0);
  COMMANDO(101, POSITION_SITTING, do_lock, 0);
  COMMANDO(102, POSITION_SITTING, do_unlock, 0);
  COMMANDO(103, POSITION_STANDING, do_leave, 0);
  COMMANDO(104, POSITION_RESTING, do_action, 0);
  COMMANDO(105, POSITION_RESTING, do_action, 0);
  COMMANDO(106, POSITION_RESTING, do_action, 0);
  COMMANDO(107, POSITION_RESTING, do_action, 0);
  COMMANDO(108, POSITION_RESTING, do_action, 0);
  COMMANDO(109, POSITION_RESTING, do_action, 0);
  COMMANDO(110, POSITION_STANDING, do_action, 0);
  COMMANDO(111, POSITION_RESTING, do_action, 0);
  COMMANDO(112, POSITION_STANDING, do_action, 0);
  COMMANDO(113, POSITION_RESTING, do_action, 0);
  COMMANDO(114, POSITION_RESTING, do_action, 0);
  COMMANDO(115, POSITION_RESTING, do_action, 0);
  COMMANDO(116, POSITION_RESTING, do_action, 0);
  COMMANDO(117, POSITION_RESTING, do_action, 0);
  COMMANDO(118, POSITION_RESTING, do_action, 0);
  COMMANDO(119, POSITION_RESTING, do_action, 0);
  COMMANDO(120, POSITION_RESTING, do_action, 0);
  COMMANDO(121, POSITION_RESTING, do_action, 0);
  COMMANDO(122, POSITION_RESTING, do_action, 0);
  COMMANDO(123, POSITION_RESTING, do_action, 0);
  COMMANDO(124, POSITION_RESTING, do_action, 0);
  COMMANDO(125, POSITION_RESTING, do_action, 0);
  COMMANDO(126, POSITION_STANDING, do_action, 0);
  COMMANDO(127, POSITION_RESTING, do_action, 0);
  COMMANDO(128, POSITION_RESTING, do_action, 0);
  COMMANDO(129, POSITION_RESTING, do_action, 0);
  COMMANDO(130, POSITION_RESTING, do_action, 0);
  COMMANDO(131, POSITION_RESTING, do_action, 0);
  COMMANDO(132, POSITION_RESTING, do_action, 0);
  COMMANDO(133, POSITION_RESTING, do_action, 0);
  COMMANDO(134, POSITION_RESTING, do_action, 0);
  COMMANDO(135, POSITION_RESTING, do_action, 0);
  COMMANDO(136, POSITION_SLEEPING, do_action, 0);
  COMMANDO(137, POSITION_STANDING, do_action, 0);
  COMMANDO(138, POSITION_RESTING, do_action, 0);
  COMMANDO(139, POSITION_RESTING, do_action, 0);
  COMMANDO(140, POSITION_STANDING, do_action, 0);
  COMMANDO(141, POSITION_RESTING, do_action, 0);
  COMMANDO(142, POSITION_RESTING, do_action, 0);
  COMMANDO(143, POSITION_RESTING, do_action, 0);
  COMMANDO(144, POSITION_RESTING, do_action, 0);
  COMMANDO(145, POSITION_STANDING, do_action, 0);
  COMMANDO(146, POSITION_RESTING, do_action, 0);
  COMMANDO(147, POSITION_RESTING, do_action, 0);
  COMMANDO(149, POSITION_STANDING, do_write, 1);
  COMMANDO(150, POSITION_RESTING, do_grab, 1);
  COMMANDO(151, POSITION_FIGHTING, do_flee, 1);
  COMMANDO(152, POSITION_STANDING, do_sneak, 1);
  COMMANDO(153, POSITION_RESTING, do_hide, 1);
  COMMANDO(154, POSITION_STANDING, do_backstab, 1);
  COMMANDO(155, POSITION_STANDING, do_pick, 1);
  COMMANDO(156, POSITION_STANDING, do_steal, 1);
  COMMANDO(157, POSITION_FIGHTING, do_bash, 1);
  COMMANDO(158, POSITION_FIGHTING, do_rescue, 1);
  COMMANDO(159, POSITION_FIGHTING, do_kick, 1);
  COMMANDO(160, POSITION_RESTING, do_action, 0);
  COMMANDO(161, POSITION_RESTING, do_action, 0);
  COMMANDO(162, POSITION_RESTING, do_action, 0);
  COMMANDO(163, POSITION_RESTING, do_action, 0);
  COMMANDO(164, POSITION_RESTING, do_practice, 1);
  COMMANDO(165, POSITION_RESTING, do_action, 0);
  COMMANDO(166, POSITION_SITTING, do_examine, 0);
  COMMANDO(167, POSITION_RESTING, do_get, 0);	/* TAKE */
  COMMANDO(168, POSITION_SLEEPING, do_info, 0);
  COMMANDO(169, POSITION_RESTING, do_say, 0);
  COMMANDO(170, POSITION_RESTING, do_practice, 1);
  COMMANDO(171, POSITION_RESTING, do_action, 0);
  COMMANDO(172, POSITION_SITTING, do_use, 1);
  COMMANDO(173, POSITION_DEAD, do_where, 1);
  COMMANDO(174, POSITION_DEAD, do_levels, 0);
  COMMANDO(176, POSITION_SITTING, do_action, 0);
  COMMANDO(177, POSITION_SLEEPING, do_emote, 1);
  COMMANDO(178, POSITION_RESTING, do_action, 0);
  COMMANDO(179, POSITION_RESTING, do_not_here, 0);
  COMMANDO(180, POSITION_RESTING, do_action, 0);
  COMMANDO(181, POSITION_SLEEPING, do_action, 0);
  COMMANDO(182, POSITION_RESTING, do_action, 0);
  COMMANDO(183, POSITION_RESTING, do_action, 0);
  COMMANDO(184, POSITION_RESTING, do_action, 0);
  COMMANDO(185, POSITION_RESTING, do_action, 0);
  COMMANDO(187, POSITION_RESTING, do_action, 0);
  COMMANDO(188, POSITION_RESTING, do_action, 0);
  COMMANDO(189, POSITION_FIGHTING, do_punch, 1);
  COMMANDO(190, POSITION_RESTING, do_action, 0);
  COMMANDO(191, POSITION_RESTING, do_action, 0);
  COMMANDO(192, POSITION_RESTING, do_action, 0);
  COMMANDO(193, POSITION_RESTING, do_action, 0);
  COMMANDO(194, POSITION_RESTING, do_action, 0);
  COMMANDO(195, POSITION_RESTING, do_action, 0);
  COMMANDO(196, POSITION_RESTING, do_action, 0);
  COMMANDO(197, POSITION_RESTING, do_action, 0);
  COMMANDO(198, POSITION_RESTING, do_action, 0);
  COMMANDO(199, POSITION_DEAD, do_brief, 0);
  COMMANDO(200, POSITION_DEAD, do_wizlist, 0);
  COMMANDO(201, POSITION_RESTING, do_consider, 0);
  COMMANDO(202, POSITION_RESTING, do_group, 1);
  COMMANDO(204, POSITION_DEAD, do_return, 0);
  COMMANDO(206, POSITION_RESTING, do_quaff, 0);
  COMMANDO(207, POSITION_RESTING, do_recite, 0);
  COMMANDO(209, POSITION_STANDING, do_pose, 0);
  COMMANDO(212, POSITION_DEAD, do_credits, 0);
  COMMANDO(213, POSITION_DEAD, do_compact, 0);
  COMMANDO(214, POSITION_SLEEPING, do_emote, 1);
  COMMANDO(215, POSITION_SLEEPING, do_plr_noshout, 1);
  COMMANDO(261, POSITION_SLEEPING, do_plr_notell, 1);
  COMMANDO(272, POSITION_SLEEPING, do_plr_nosummon, 1);
  COMMANDO(273, POSITION_SLEEPING, do_plr_noteleport, 1);
  COMMANDO(217, POSITION_DEAD, do_wimp, 1);
  COMMANDO(218, POSITION_RESTING, do_junk, 1);
  COMMANDO(219, POSITION_RESTING, do_not_here, 1);
  COMMANDO(220, POSITION_RESTING, do_not_here, 1);
  COMMANDO(221, POSITION_RESTING, do_not_here, 1);
  COMMANDO(224, POSITION_STANDING, do_not_here, 1);
  COMMANDO(230, POSITION_DEAD, do_track, 1);
  COMMANDO(234, POSITION_DEAD, do_who, 0);
  COMMANDO(235, POSITION_FIGHTING, do_assist, 0);
  COMMANDO(236, POSITION_DEAD, do_swat, 1);
  COMMANDO(239, POSITION_FIGHTING, do_breath, 0);
  COMMANDO(243, POSITION_DEAD, do_gain, 1);
  COMMANDO(245, POSITION_FIGHTING, do_disarm, 1);
  COMMANDO(246, POSITION_SITTING, do_action, 1);
  COMMANDO(249, POSITION_DEAD, do_group_tell, 1);
  COMMANDO(252, POSITION_DEAD, do_group_report, 1);
  COMMANDO(253, POSITION_DEAD, do_pager, 1);
  COMMANDO(254, POSITION_DEAD, do_invis_off, 1);

  COMMANDO(247, POSITION_DEAD, do_commune, 51);
  COMMANDO(61, POSITION_SLEEPING, do_goto, 51);
  COMMANDO(208, POSITION_DEAD, do_users, 51);
  COMMANDO(211, POSITION_SLEEPING, do_wizhelp, 51);
  COMMANDO(148, POSITION_STANDING, do_action, 51);
  COMMANDO(237, POSITION_DEAD, do_world, 51);
  COMMANDO(233, POSITION_DEAD, do_title, 51);

  COMMANDO(210, POSITION_SLEEPING, do_noshout, 52);
  COMMANDO(78, POSITION_DEAD, do_purge, 52);
  COMMANDO(203, POSITION_DEAD, do_restore, 52);
  COMMANDO(48, POSITION_SLEEPING, do_trans, 52);
  COMMANDO(242, POSITION_DEAD, do_invis, 52);
  COMMANDO(251, POSITION_DEAD, do_allcommands, 1);

  COMMANDO(71, POSITION_SLEEPING, do_string, 53);
  COMMANDO(85, POSITION_DEAD, do_at, 53);
  COMMANDO(231, POSITION_DEAD, do_wizlock, 53);
  COMMANDO(238, POSITION_DEAD, do_spells, 53);

  COMMANDO(240, POSITION_DEAD, do_show, 54);
  COMMANDO(216, POSITION_STANDING, do_kill, 54);
  COMMANDO(74, POSITION_DEAD, do_stat, 54);
  COMMANDO(222, POSITION_DEAD, do_nohassle, 54);

  COMMANDO(205, POSITION_DEAD, do_switch, 55);
  COMMANDO(232, POSITION_DEAD, do_highfive, 55);
  COMMANDO(223, POSITION_DEAD, do_system, 55);
  COMMANDO(47, POSITION_SLEEPING, do_force, 55);

  COMMANDO(248, POSITION_DEAD, do_form, 56);
  COMMANDO(226, POSITION_DEAD, do_cust, 56);
  COMMANDO(229, POSITION_DEAD, do_rload, 56);
  COMMANDO(228, POSITION_DEAD, do_rsave, 56);

  COMMANDO(244, POSITION_DEAD, do_instazone, 57);
  COMMANDO(255, POSITION_DEAD, do_show_logs, 57);
  COMMANDO(250, POSITION_DEAD, do_pretitle, 57);
  COMMANDO(77, POSITION_DEAD, do_load, 57);

  COMMANDO(227, POSITION_DEAD, do_set, 58);
  COMMANDO(225, POSITION_DEAD, do_stealth, 58);
  COMMANDO(79, POSITION_DEAD, do_shutdown, 58);
  COMMANDO(175, POSITION_DEAD, do_reroll, 58);
  COMMANDO(68, POSITION_DEAD, do_shutdow, 58);

  COMMANDO(95, POSITION_DEAD, do_advance, 59);
  COMMANDO(241, POSITION_DEAD, do_debug, 59);
  COMMANDO(90, POSITION_DEAD, do_snoop, 59);
  COMMANDO(259, POSITION_DEAD, do_whod, 59);
  COMMANDO(268, POSITION_DEAD, do_restore_all, 59);

}		       /* 259 - last command number used */

/*
 * Stuff for controlling the non-playing sockets (get name, pwd etc)
 */

/*
 * locate entry in p_table with entry->name == name. -1 mrks failed search 
 */
int find_name(char *name)
{
  int i;

  for (i = 0; i <= top_of_p_table; i++) {
    if (!str_cmp((player_table + i)->name, name))
      return (i);
  }
  return (-1);
}

int _parse_name(char *arg, char *name)
{
  int i;

  for (; isspace(*arg); arg++);
  for (i = 0; *name = *arg; arg++, i++, name++)
    if ((*arg < 0) || !isalpha(*arg) || i > 15)
      return (1);

  if (!i)
    return (1);
  return (0);
}

/*
 * An improved version of _parse_name()
 */
int valid_parse_name(char *arg, char *name)
{
  register int i;
  char *hard[] = { "god", "demigod", NULL };

  if(!arg || !*arg)
    return 0;
  for(i= 0; hard[i]; i++)
    if(!strcasecmp(hard[i], arg))
      return 0;
  for(i= 0; *name= *arg; i++, arg++, name++)
    if(!*arg || !isalpha(*arg) || i> 15)
      return 0;
  return 1;
}

/*
 * Make sure they are not trying to take a mob name...
 */
int already_mob_name(char *ack_name)
{
  register int ack, blah;
  char pfft_name[80];
  char *pfft;
  extern int top_of_mobt;
  
  for (blah= FALSE, ack = 0; ack < top_of_mobt && !blah; ack++) {
    strcpy(pfft_name, mob_index[ack].name);
    if (!(pfft = (char *)strtok(pfft_name, " ")))
      continue;
    if (!strcasecmp(pfft, ack_name)) {
      blah = TRUE;
      break;
    }
    while (pfft = (char *)strtok(NULL, " ")) {
      if (!strcasecmp(pfft, ack_name)) {
        blah = TRUE;
        break;
      }
    }
  }
  return blah;
}

/*
 * See if the player has lost his link
 */
int check_reconnect(struct descriptor_data *d)
{
  struct char_data *tmp_ch;

  for (tmp_ch = character_list; tmp_ch; tmp_ch = tmp_ch->next) {
    if (
/* Doh!
 * If it is not a player, it will not have the player.name structure and
 * will SEGV before it checks IS_NPC().... please remember left-to-right.
 *             ((!str_cmp(d->usr_name), GET_NAME(tmp_ch)) &&
 *              !tmp_ch->desc && !IS_NPC(tmp_ch)) ||
 */
        (!IS_NPC(tmp_ch) && !tmp_ch->desc &&
         (!str_cmp(d->usr_name, GET_NAME(tmp_ch)))) ||
          (IS_NPC(tmp_ch) && tmp_ch->orig &&
           !str_cmp(d->usr_name, GET_NAME(tmp_ch->orig))
        )) {
      free_char(d->character);
      tmp_ch->desc = d;
      d->character = tmp_ch;
      tmp_ch->specials.timer = 0;
      if (tmp_ch->orig) {
        tmp_ch->desc->original = tmp_ch->orig;
        tmp_ch->orig = 0;
      }
      STATE(d) = CON_PLAYING;
      dprintf(d, "\n\r%sReconnecting to %s.\n\r", echo_on, GET_NAME(d->character));
      act("$n has reconnected.", TRUE, tmp_ch, 0, 0, TO_ROOM);
      if (d->character->in_room == NOWHERE)
        char_to_room(d->character, DEFAULT_HOME);
      else {
        if (d->character->in_room == 0) {
          char_from_room(d->character);
          char_to_room(d->character, DEFAULT_HOME);
        }
      }
      log("%s[%s] has reconnected.", GET_NAME(d->character), d->host);
      return TRUE;
    }
  }
  return FALSE;
}

int check_playing(struct descriptor_data *d, char *tmp_name)
{
  struct descriptor_data *k;

  for (k = descriptor_list; k; k = k->next) {
    if ((k->character != d->character) && k->character) {
      if (k->original) {
        if ( GET_NAME(k->original) &&
             !str_cmp(GET_NAME(k->original), tmp_name)) {
          return 1;
        }
      } else {
        if ( GET_NAME(k->character) &&
             !str_cmp(GET_NAME(k->character), tmp_name)) {
          return 1;
        }
      }
    }
  }
  return 0;
}

/*
 * deal with newcomers and other non-playing sockets 
 */
void nanny(struct descriptor_data *d, char *arg)
{
  char buf[100];
  int player_i, count = 0, oops = FALSE;
  char tmp_name[20];
  struct char_file_u tmp_store;
  struct char_data *tmp_ch;
  struct char_data tmp_ch2;
  struct descriptor_data *k;
  extern struct descriptor_data *descriptor_list;
  extern int WizLock;
  struct char_data *ch;

  while(isspace(*arg))
    arg++;
  ch = d->character;
  write(d->descriptor, echo_on, strlen(echo_on));

  switch (STATE(d)) {
  default:
    bug("Nanny:  illegal state %d.\n", STATE(d));
    close_socket(d);
    return;
  case CON_GET_NAME:
    log("Got Connection from: %s", d->host);
    if (!d->character) {
      CREATE(d->character, struct char_data, 1);
      clear_char(d->character);
      d->character->desc = d;
    }
    if (!*arg) {
      close_socket(d);
      return;
    }
    if (!valid_parse_name(arg, tmp_name)) {
      dprintf(d, "\rIllegal name, please try another.\n\rWHAT is your Name? ");
      return;
    }
    if (check_playing(d, tmp_name)) {
      dprintf(d, "\rSorry, %s is already playing... you might be cheating!\n\rWhat is YOUR Name? ", tmp_name);
      return;
    }
    if (!ValidPlayer(tmp_name, d->pwd)) {
      dprintf(d, "\n\rDeadMUD is currently in registration-only mode.\n\rPlease email mud@yakko.cs.wmich.edu for a character!\n\r");
      STATE(d) = CON_WIZLOCK;
      return;
    }
    strcpy(d->usr_name, tmp_name);
/*  GET_NAME(d->character) = (char *)strdup(d->usr_name); */
    if (load_char(d->usr_name, &tmp_store) > -1) {
    /* if (GetPlayerFile(d->usr_name, d->character)) */
      store_to_char(&tmp_store, d->character);
      strcpy(d->pwd, tmp_store.pwd);
      log("%s@%s loaded.", d->usr_name, d->host);
      dprintf(d, "\n\r%sWHAT is your Password? ", echo_off);
      STATE(d) = CON_GET_PASSWORD;
    } else {
      if(already_mob_name(d->usr_name)) {
        dprintf(d, "\rBut you'd be confused with a MONSTER!.\n\rWHAT is your Name? ");
        return;
      }
      GET_NAME(d->character) = (char *)strdup(d->usr_name);
      dprintf(d, "\n\r%sChoose a password for %s: ", echo_off, d->usr_name);
      STATE(d) = CON_GET_NEW_PASWORD;
      log("New player!");
    }
    return;
  case CON_GET_PASSWORD:
    if (!*arg) {
      close_socket(d);
      return;
    }
    if (strncmp(arg, d->pwd, 10)) {
      dprintf(d, "\r***BUZZ!*** Wrong password.\n\r%sGuess again: ", echo_off);
      return;
    }
    if(check_reconnect(d))
      return;
    log("%s@%s has connected.", GET_NAME(d->character), d->host);
    if (GetMaxLevel(d->character) > LOW_IMMORTAL)
      dprintf(d, "\n\r%s", wmotd);
    else
      dprintf(d, "\n\r%s", motd);
    dprintf(d, "*** Press Return: ", d);
    STATE(d) = CON_READ_MOTD;
    return;
  case CON_GET_NEW_PASWORD:
    if (!*arg || strlen(arg) > 10) {
      dprintf(d, "\rIllegal password.\n\r%sChoose a password for %s: ", echo_off, d->usr_name);
      return;
    }
    strncpy(d->pwd, arg, 10);
    *(d->pwd + 10) = '\0';
    dprintf(d, "\n\r%sPlease retype your password: ", echo_off);
    STATE(d) = CON_CONFIRM_NEW_PASSWORD;
    return;
  case CON_CONFIRM_NEW_PASSWORD:
    if (strncmp(arg, d->pwd, 10)) {
      dprintf(d, "\n\rBut those don't match!\n\r%sTry your password again: ", echo_off);
      STATE(d) = CON_GET_NEW_PASWORD;
      return;
    }
    PutPasswd(d->usr_name, d->pwd, d->host);
    dprintf(d, "\r%s", RACEMENU);
    STATE(d) = CON_GET_RACE;
    return;
  case CON_GET_RACE:
    if (!*arg) {
      dprintf(d, "\r%s", RACEMENU);
      return;
    }
    switch (*arg) {
    default:
      dprintf(d, "\rThat's not a race.\n\r%s", RACEMENU);
      STATE(d) = CON_GET_RACE;
      break;
    case '?':
      dprintf(d, "\r%s", RACEHELP);
      STATE(d) = CON_GET_RACE;
      break;
    case 'd':
    case 'D':
      GET_RACE(d->character) = RACE_DWARF;
      dprintf(d, SEXMENU);
      STATE(d) = CON_GET_SEX;
      break;
    case 'e':
    case 'E':
      GET_RACE(d->character) = RACE_ELVEN;
      dprintf(d, SEXMENU);
      STATE(d) = CON_GET_SEX;
      break;
    case 'G':
    case 'g':
      GET_RACE(d->character) = RACE_GNOME;
      dprintf(d, SEXMENU);
      STATE(d) = CON_GET_SEX;
      break;
    case 'f':
    case 'F':
      GET_RACE(d->character) = RACE_HALFLING;
      dprintf(d, SEXMENU);
      STATE(d) = CON_GET_SEX;
      break;
    case 'h':
    case 'H':
      GET_RACE(d->character) = RACE_HUMAN;
      dprintf(d, SEXMENU);
      STATE(d) = CON_GET_SEX;
      break;
    }
    return;
  case CON_GET_SEX:
    switch (*arg) {
    default:
      dprintf(d, "But how will you mate???\n\r%s", SEXMENU);
      return;
    case 'm':
    case 'M':
      d->character->player.sex = SEX_MALE;
      break;
    case 'f':
    case 'F':
      d->character->player.sex = SEX_FEMALE;
      break;
    }
    dprintf(d, CLASSMENU);
    STATE(d) = CON_GET_CLASS;
    return;

  case CON_GET_CLASS:
      d->character->player.class = 0;
      count = 0;
      oops = FALSE;
      for (; *arg && count < 3 && !oops; *arg++) {
	switch (*arg) {
	default:
	  dprintf(d, "I wish *I* could be a \"%s\" too!\n\r%s", arg, CLASSMENU);
	  STATE(d) = CON_GET_CLASS;
	  oops = TRUE;
	  break;
        case '?':
          dprintf(d, CLASSHELP);
          STATE(d) = CON_GET_CLASS;
          break;
	case 'm':
	case 'M':
	  {
	    if (!IS_SET(d->character->player.class, CLASS_MAGIC_USER))
	      d->character->player.class += CLASS_MAGIC_USER;
	    STATE(d) = CON_READ_MOTD;
	    count++;
	    break;
	  }
	case 'c':
	case 'C':
	  {
	    if (!IS_SET(d->character->player.class, CLASS_CLERIC))
	      d->character->player.class += CLASS_CLERIC;
	    STATE(d) = CON_READ_MOTD;
	    count++;
	    break;
	  }
	case 'f':
	case 'F':
	case 'w':
	case 'W':
	  {
	    if (!IS_SET(d->character->player.class, CLASS_WARRIOR))
	      d->character->player.class += CLASS_WARRIOR;
	    STATE(d) = CON_READ_MOTD;
	    count++;
	    break;
	  }
	case 't':
	case 'T':
	  {
	    if (!IS_SET(d->character->player.class, CLASS_THIEF))
	      d->character->player.class += CLASS_THIEF;
	    STATE(d) = CON_READ_MOTD;
	    count++;
	    break;
	  }
	case 'r':
	case 'R':
	  {
	    if (!IS_SET(d->character->player.class, CLASS_RANGER))
	      d->character->player.class = CLASS_RANGER;
	    STATE(d) = CON_READ_MOTD;
	    count++;
	    break;
	  }
/*
 * case 'd':
 * case 'D': {
 * if (!IS_SET(d->character->player.class, CLASS_DRUID))
 * d->character->player.class += CLASS_DRUID;
 * STATE(d) = CON_READ_MOTD;
 * count++;
 * break;
 * }
 */
	case ' ':
	case ',':
	case '\\':    /* ignore these */
	case '/':
	  break;

	}

	if ((count > 1) && IS_SET(d->character->player.class, CLASS_RANGER)) {
	  dprintf(d, "Rangers may only be single classed.\n\r%s", CLASSMENU);
	  STATE(d) = CON_GET_CLASS;
	  oops = TRUE;
	}
      }

      if (STATE(d) != CON_GET_CLASS) {
	log("%s [%s] new player.", GET_NAME(d->character), d->host);
	init_char(d->character);
	d->pos = create_entry(GET_NAME(d->character));
	save_char(d->character, NOWHERE);
	dprintf(d, "\n\r%s\n\r*** PRESS RETURN: ", motd);
	STATE(d) = CON_READ_MOTD;
      }
    return;
  case CON_READ_MOTD:
    dprintf(d, "\n\r%s", MENU);
    STATE(d) = CON_MENU_SELECT;
    if (WizLock) {
      if (GetMaxLevel(d->character) < LOW_IMMORTAL) {
	dprintf(d, "Sorry, the game is locked so the whizz's can break stuff!\n\r");
	STATE(d) = CON_WIZLOCK;
      }
    }
    return;
  case CON_WIZLOCK:
    close_socket(d);
    return;
  case CON_MENU_SELECT:
    switch (*arg) {
    default:
      dprintf(d, "Wrong option.\n\r%s", MENU);
      return;
    case '0':
      close_socket(d);
      return;
    case '1':
      reset_char(d->character);
      log("Loading %s's equipment", d->character->player.name);
      load_char_objs(d->character);
      save_char(d->character, NOWHERE);
      send_to_char(WELC_MESSG, d->character);
      d->character->next = character_list;
      character_list = d->character;
      if (d->character->in_room == NOWHERE) {
	if (GetMaxLevel(d->character) >= LOW_IMMORTAL) {
	  if (!real_roomp(GET_HOME(d->character)))
	    GET_HOME(d->character) = 1000;
	  char_to_room(d->character, GET_HOME(d->character));
	} else {
	  if (!real_roomp(GET_HOME(d->character)))
	    GET_HOME(d->character) = DEFAULT_HOME;
	  char_to_room(d->character, GET_HOME(d->character));
	}
      } else {
	if (real_roomp(d->character->in_room)) {
	  char_to_room(d->character, d->character->in_room);
	  GET_HOME(d->character) = d->character->in_room;
	} else {
	  char_to_room(d->character, DEFAULT_HOME);
	  GET_HOME(d->character) = DEFAULT_HOME;
	}
      }

      if (GetMaxLevel(d->character) < 58)
	act("$n has entered the game.", TRUE, d->character, 0, 0, TO_ROOM);
      if (GetMaxLevel(d->character) >= 58)
	d->character->invis_level = GetMaxLevel(d->character) - 1;

      STATE(d) = CON_PLAYING;
      if (!GetMaxLevel(d->character))
	do_start(d->character);
      do_look(d->character, "", 15);
      d->prompt_mode = 1;
      return;
    case '2':
      dprintf(d, "Enter a text you'd like others to see when they look at you.\n\rTerminate with a '@'.\n\r");
      if (d->character->player.description) {
	dprintf(d, "Old description :\n\r%s", d->character->player.description);
	free(d->character->player.description);
	d->character->player.description = 0;
      }
      d->str = &d->character->player.description;
      d->max_str = 240;
      STATE(d) = CON_EDIT_DESCRIPTION;
      return;
    case '3':
      dprintf(d, STORY);
      STATE(d) = CON_READ_MOTD;
      return;
    case '4':
      dprintf(d, "%sEnter a new password: ", echo_off);
      STATE(d) = CON_GET_CHANGE_PASSWORD;
      return;
    case '5':
      {
	struct descriptor_data *dd;
	struct char_data *person;
	int count = 0;

	dprintf(d, "Players Connected.\n\r\n\r");
	for (dd = descriptor_list; dd; dd = dd->next)
	  if (!dd->connected) {
	    person = dd->character;
	    if (!IS_IMMORTAL(person) || IS_IMMORTAL(d->character)) {
	      count++;
	      dprintf(d, "%s %s %s\n\r", (person->player.pre_title ? person->player.pre_title :
					    "")
		      ,GET_NAME(person), person->player.title);
	    }
	  }
	dprintf(d, "Total Connected %d\n\r", count);
	STATE(d) = CON_READ_MOTD;
	break;
      }
      return;
    case '6':
      if (IS_IMMORTAL(d->character)) {
	dprintf("\n\rSorry, you are a slave to the source...  There is no escape for you!\n\r%s", MENU);
	break;
      }
      dprintf(d, SUICIDE_MSG);
      STATE(d) = CON_SUICIDE;
      return;
    }
    return;
  case CON_SUICIDE:
    if (!strcmp(arg, "I want to DIE!")) {
      char name[80], *t_ptr, old[80], bkp[80];

      strcpy(name, d->usr_name);
      t_ptr = name;
      for (; *t_ptr != '\0'; t_ptr++)
	*t_ptr = LOWER(*t_ptr);
      sprintf(old, "ply/%c/%s.p", name[0], name);
      sprintf(bkp, "ply/%c/%s.p-dead", name[0], name);
      rename(old, bkp);
      sprintf(old, "ply/%c/%s.o", name[0], name);
      sprintf(bkp, "ply/%c/%s.o-dead", name[0], name);
      rename(old, bkp);
      log("-- SUICIDE -- %s is no more!\n", name);
      dprintf(d, SUICIDE_DONE);
      STATE(d) = CON_WIZLOCK;
      return;
    }
    dprintf(d, "You are SAVED!\n\r%s", MENU);
    STATE(d) = CON_MENU_SELECT;
    return;
  case CON_GET_CHANGE_PASSWORD:
    if (!*arg || strlen(arg) > 10) {
      dprintf(d, "\rIllegal password.\n\r%sPassword: ", echo_off);
      return;
    }
    strncpy(d->pwd, arg, 10);
    *(d->pwd + 10) = '\0';
    dprintf(d, "\n\r%sPlease retype password: ", echo_off);
    STATE(d) = CON_CONFIRM_CHANGE_PASSWORD;
    return;
  case CON_CONFIRM_CHANGE_PASSWORD:
    if (strncmp(arg, d->pwd, 10)) {
      dprintf(d, "\rPasswords don't match.\n\r%sRetype password: ", echo_off);
      STATE(d) = CON_GET_CHANGE_PASSWORD;
      return;
    }
    dprintf(d, "%s\n\rDone. You must enter the game to make the change final\n\r%s", echo_on, MENU);
    STATE(d) = CON_MENU_SELECT;
    return;
  }
}

void PutPasswd(char *who, char *pwd, char *host)
{
  FILE *pfd;
  char buf[256];

  if ((pfd = fopen(PASSWD_NEW, "a")) == NULL) {
    log("Cannot save password data for new user!\n\r");
  } else {
    sprintf(buf, "%s %s user@%s %ld\n", who, pwd, host, time(NULL));
    fprintf(pfd, buf);
    fclose(pfd);
    log(buf);
  }
}

int ValidPlayer(char *who, char *pwd)
{
  FILE *pwd_fd;
  char tname[40];
  char *t_ptr;
  char pname[40];
  char passwd[40];
  char email[80];
  long timestamp;

  if (!(pwd_fd = fopen(PASSWD_FILE, "r"))) {
    strcpy(pwd, "NOT USED");
    log("Password checking disabled!");
    return 1;
  }
  log("Searching passwd file for %s...", who);

  strcpy(tname, who);
  for (t_ptr = tname; *t_ptr; t_ptr++)
    *t_ptr = LOWER(*t_ptr);

  while (fscanf(pwd_fd, " %s %s %s %ld\n", pname, passwd, email, &timestamp) > 1) {
    if (!strcmp(tname, pname)) {
      log("Found %s in passwd file.", tname);
      if (!strcmp(passwd, "*"))
	strcpy(pwd, "IS VALID");
      else
	strcpy(pwd, passwd);
      fclose(pwd_fd);
      return 1;
    }
  }
  fclose(pwd_fd);
  return 0;
}

#if 0
int GetPlayerFile(char *name, struct char_data *where)
{
  struct char_file_u tmp_store;

  if ((load_char(name, &tmp_store)) > -1) {
    store_to_char(&tmp_store, where);
    return (1);
  }
  return (0);
}
#endif

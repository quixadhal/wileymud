/*
 * file: act.comm.c , Implementation of commands.         Part of DIKUMUD
 * Usage : Communication.
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */

#include <stdio.h>
#include <stdlib.h>
/* #include <unistd.h> */
#include <sys/types.h>
#include <string.h>

#include "global.h"
#include "bug.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "constants.h"
#include "mudlimits.h"
#include "multiclass.h"
#include "board.h"
#define _ACT_COMM_C
#include "act_comm.h"

void do_say(struct char_data *ch, const char *argument, int cmd)
{
  int                                     i = 0;

  if (DEBUG)
    log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

  for (i = 0; *(argument + i) == ' '; i++);		       /* skip leading spaces */

  if (!*(argument + i))
    cprintf(ch, "usage: say <mesg>.\r\n");
  else {
    switch (argument[strlen(argument) - 1]) {
      case '!':
	act("$n exclaims '%s'", FALSE, ch, 0, 0, TO_ROOM, argument + i);
	if (IS_NPC(ch) || (IS_SET(ch->specials.act, PLR_ECHO))) {
	  cprintf(ch, "You exclaim '%s'\r\n", argument + i);
	}
	break;
      case '?':
	act("$n asks '%s'", FALSE, ch, 0, 0, TO_ROOM, argument + i);
	if (IS_NPC(ch) || (IS_SET(ch->specials.act, PLR_ECHO))) {
	  cprintf(ch, "You ask '%s'\r\n", argument + i);
	}
	break;
      default:
	act("$n says '%s'", FALSE, ch, 0, 0, TO_ROOM, argument + i);
	if (IS_NPC(ch) || (IS_SET(ch->specials.act, PLR_ECHO))) {
	  cprintf(ch, "You say '%s'\r\n", argument + i);
	}
	break;
    }
  }
}

void do_shout(struct char_data *ch, const char *argument, int cmd)
{
  struct descriptor_data                 *i = NULL;
  struct room_data                       *rp = NULL;
  struct room_data                       *mrp = NULL;

#ifdef RADIUS_SHOUT
  struct room_direction_data             *exitp = NULL;
  struct char_data                       *v = NULL;
  int                                     x = 0;
#endif

  if (DEBUG)
    log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

  if (IS_SET(ch->specials.act, PLR_NOSHOUT)) {
    cprintf(ch, "You can't shout!!\r\n");
    return;
  }
  for (; *argument == ' '; argument++);			       /* skip leading spaces */

  if (ch->master && IS_AFFECTED(ch, AFF_CHARM)) {
    cprintf(ch->master, "You pet is trying to shout....");
    return;
  }
  if (!(*argument)) {
    cprintf(ch, "usage: shout <mesg>.\r\n");
    return;
  }

  if (IS_IMMORTAL(ch) || IS_NPC(ch)) {
    cprintf(ch, "You shout '%s'\r\n", argument);
    for (i = descriptor_list; i; i = i->next)
      if (i->character != ch && !i->connected &&
	  !IS_SET(i->character->specials.act, PLR_NOSHOUT) &&
	  !IS_SET(i->character->specials.act, PLR_DEAF) &&
	  (rp = real_roomp(i->character->in_room)) &&
          (!FindBoardInRoom(i->character->in_room)))
	act("$n shouts '%s'", 0, ch, 0, i->character, TO_VICT, argument);
  } else {
#ifdef OLD_SHOUT
    if (GET_MOVE(ch) < (GET_MAX_MOVE(ch) / 10)) {
      cprintf(ch, "You are just too tired to shout anything.\r\n");
      return;
    }
    if (GET_MANA(ch) < (GET_MAX_MANA(ch) / 15)) {
      cprintf(ch, "You can't seem to summon the energy to shout.\r\n");
      return;
    }
    if (GET_COND(ch, THIRST) < 4) {
      cprintf(ch, "Your throat is too dry to shout anything!\r\n");
      return;
    }
    GET_MOVE(ch) -= (GET_MAX_MOVE(ch) / 10);
    GET_MANA(ch) -= (GET_MAX_MANA(ch) / 15);
    GET_COND(ch, THIRST) -= 4;
    if (IS_SET(ch->specials.act, PLR_ECHO))
      cprintf(ch, "You shout '%s'\r\n", argument);
    for (i = descriptor_list; i; i = i->next)
      if (i->character != ch && !i->connected &&
	  !IS_SET(i->character->specials.act, PLR_NOSHOUT) &&
	  !IS_SET(i->character->specials.act, PLR_DEAF) &&
	  (rp = real_roomp(i->character->in_room)) && (mrp = real_roomp(ch->in_room)) &&
          (!FindBoardInRoom(i->character->in_room)))
	act("$n shouts '%s'", 0, ch, 0, i->character, TO_VICT, argument);
#endif
#ifdef ZONE_SHOUT
    if (GET_MOVE(ch) < (GET_MAX_MOVE(ch) / 10)) {
      cprintf(ch, "You are just too tired to shout anything.\r\n");
      return;
    }
    if (GET_MANA(ch) < (GET_MAX_MANA(ch) / 15)) {
      cprintf(ch, "You can't seem to summon the energy to shout.\r\n");
      return;
    }
    if (GET_COND(ch, THIRST) < 4) {
      cprintf(ch, "Your throat is too dry to shout anything!\r\n");
      return;
    }
    GET_MOVE(ch) -= (GET_MAX_MOVE(ch) / 10);
    GET_MANA(ch) -= (GET_MAX_MANA(ch) / 15);
    GET_COND(ch, THIRST) -= 4;
    if (IS_SET(ch->specials.act, PLR_ECHO))
      cprintf(ch, "You shout '%s'\r\n", argument);
    for (i = descriptor_list; i; i = i->next)
      if (i->character != ch && !i->connected &&
	  !IS_SET(i->character->specials.act, PLR_NOSHOUT) &&
	  !IS_SET(i->character->specials.act, PLR_DEAF) &&
	  (rp = real_roomp(i->character->in_room)) &&
	  (mrp = real_roomp(ch->in_room)) && (rp->zone == mrp->zone) &&
          (!FindBoardInRoom(i->character->in_room)))
	act("$n shouts '%s'", 0, ch, 0, i->character, TO_VICT, argument);
#endif
#ifdef RADIUS_SHOUT
/* This is a buggy... it corrupts memory someplace */
    if (!(mrp = real_roomp(ch->in_room))) {
      cprintf(ch, "You are not in reality... this is bad.\r\n");
      return;
    }
    for (v = mrp->people; v; v = v->next_in_room)
      if (v != ch && v->desc &&
	  !IS_SET(v->specials.act, PLR_NOSHOUT) && !IS_SET(v->specials.act, PLR_DEAF))
	act("$n shouts '%s'", 0, ch, 0, v, TO_VICT, argument);
    for (x = 0; x < MAX_NUM_EXITS; x++)
      if ((exitp = EXIT(ch, x)) && exit_ok(exitp, mrp))
	if ((rp = real_roomp(exitp->to_room)) && (rp != mrp) && (!FindBoardInRoom(v->in_room))) {
	  for (v = rp->people; v; v = v->next_in_room)
	    if (v != ch && v->desc &&
		!IS_SET(v->specials.act, PLR_NOSHOUT) && !IS_SET(v->specials.act, PLR_DEAF))
	      act("$n shouts %s '%s'", 0, ch, 0, v, TO_VICT, dir_from[rev_dir[x]], argument);
	}
#endif
  }
}

void do_commune(struct char_data *ch, const char *argument, int cmd)
{
  struct descriptor_data                 *i = NULL;

  if (DEBUG)
    log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

  for (; *argument == ' '; argument++);			       /* skip leading spaces */

  if (!(*argument))
    cprintf(ch, "Communing among the gods is fine, but WHAT?\r\n");
  else {
    if (IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
      cprintf(ch, "You wiz : '%s'\r\n", argument);	       /* part of wiznet */
    }
    for (i = descriptor_list; i; i = i->next)
      if (i->character != ch && !i->connected &&
	  !IS_SET(i->character->specials.act, PLR_NOSHOUT) &&
	  (GetMaxLevel(i->character) >= LOW_IMMORTAL))
	act("$n : '%s'", 0, ch, 0, i->character, TO_VICT, argument);
  }
}

void do_tell(struct char_data *ch, const char *argument, int cmd)
{
  struct char_data                       *vict = NULL;
  char                                    name[100] = "\0\0\0";
  char                                    message[MAX_INPUT_LENGTH + 20] = "\0\0\0";

  if (DEBUG)
    log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

  half_chop(argument, name, message);

  if (!*name || !*message) {
    cprintf(ch, "usage: tell <who> <mesg>.\r\n");
    return;
  }
  if (!(vict = get_char_vis(ch, name))) {
    cprintf(ch, "No-one by that name here..\r\n");
    return;
  }
  if (ch == vict) {
    cprintf(ch, "You try to tell yourself something.\r\n");
    return;
  }
  if (GET_POS(vict) == POSITION_SLEEPING) {
    act("$E is asleep, shhh.", FALSE, ch, 0, vict, TO_CHAR);
    return;
  }
  if (IS_NPC(vict) && !(vict->desc)) {
    cprintf(ch, "No-one by that name here..\r\n");
    return;
  }
  if (!vict->desc) {
    cprintf(ch, "They can't hear you, link dead.\r\n");
    return;
  }
  if (IS_SET(vict->specials.new_act, NEW_PLR_NOTELL) && IS_MORTAL(ch)) {
    cprintf(ch, "They are not recieving tells at the moment.\r\n");
    return;
  }
  switch (message[strlen(message) - 1]) {
    case '!':
      cprintf(vict, "%s exclaims to you '%s'\r\n", NAME(ch), message);
      if (IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO))
	cprintf(ch, "You exclaim to %s '%s'\r\n", NAME(vict), message);
      break;
    case '?':
      cprintf(vict, "%s asks you '%s'\r\n", NAME(ch), message);
      if (IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO))
	cprintf(ch, "You ask %s '%s'\r\n", NAME(vict), message);
      break;
    default:
      cprintf(vict, "%s tells you '%s'\r\n", NAME(ch), message);
      if (IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO))
	cprintf(ch, "You tell %s '%s'\r\n", NAME(vict), message);
      break;
  }
}

void do_whisper(struct char_data *ch, const char *argument, int cmd)
{
  struct char_data                       *vict = NULL;
  char                                    name[100] = "\0\0\0";
  char                                    message[MAX_INPUT_LENGTH] = "\0\0\0";

  if (DEBUG)
    log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

  half_chop(argument, name, message);

  if (!*name || !*message)
    cprintf(ch, "Who do you want to whisper to.. and what??\r\n");
  else if (!(vict = get_char_room_vis(ch, name)))
    cprintf(ch, "No-one by that name here..\r\n");
  else if (vict == ch) {
    act("$n whispers quietly to $mself.", FALSE, ch, 0, 0, TO_ROOM);
    cprintf(ch, "You can't seem to get your mouth close enough to your ear...\r\n");
  } else {
    act("$n whispers to you, '%s'", FALSE, ch, 0, vict, TO_VICT, message);
    if (IS_NPC(ch) || (IS_SET(ch->specials.act, PLR_ECHO))) {
      cprintf(ch, "You whisper to %s, '%s'\r\n",
	      (IS_NPC(vict) ? vict->player.name : GET_NAME(vict)), message);
    }
    act("$n whispers something to $N.", FALSE, ch, 0, vict, TO_NOTVICT);
  }
}

void do_ask(struct char_data *ch, const char *argument, int cmd)
{
  struct char_data                       *vict = NULL;
  char                                    name[100] = "\0\0\0";
  char                                    message[MAX_INPUT_LENGTH] = "\0\0\0";

  if (DEBUG)
    log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

  half_chop(argument, name, message);

  if (!*name || !*message)
    cprintf(ch, "Who do you want to ask something.. and what??\r\n");
  else if (!(vict = get_char_room_vis(ch, name)))
    cprintf(ch, "No-one by that name here..\r\n");
  else if (vict == ch) {
    act("$n quietly asks $mself a question.", FALSE, ch, 0, 0, TO_ROOM);
    cprintf(ch, "You think about it for a while...\r\n");
  } else {
    act("$n asks you '%s'", FALSE, ch, 0, vict, TO_VICT, message);

    if (IS_NPC(ch) || (IS_SET(ch->specials.act, PLR_ECHO))) {
      cprintf(ch, "You ask %s, '%s'\r\n",
	      (IS_NPC(vict) ? vict->player.name : GET_NAME(vict)), message);
    }
    act("$n asks $N a question.", FALSE, ch, 0, vict, TO_NOTVICT);
  }
}

void do_write(struct char_data *ch, const char *argument, int cmd)
{
  struct obj_data                        *paper = NULL;
  struct obj_data                        *pen = NULL;
  char                                    papername[MAX_INPUT_LENGTH] = "\0\0\0";
  char                                    penname[MAX_INPUT_LENGTH] = "\0\0\0";

  if (DEBUG)
    log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

  argument_interpreter(argument, papername, penname);

  if (!ch->desc)
    return;

  if (!*papername) {					       /* nothing was delivered */
    cprintf(ch, "write (on) papername (with) penname.\r\n");
    return;
  }
  if (!*penname) {
    cprintf(ch, "write (on) papername (with) penname.\r\n");
    return;
  }
  if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying))) {
    cprintf(ch, "You have no %s.\r\n", papername);
    return;
  }
  if (!(pen = get_obj_in_list_vis(ch, penname, ch->carrying))) {
    cprintf(ch, "You have no %s.\r\n", penname);
    return;
  }
  /*
   * ok.. now let's see what kind of stuff we've found 
   */
  if (pen->obj_flags.type_flag != ITEM_PEN) {
    act("$p is no good for writing with.", FALSE, ch, pen, 0, TO_CHAR);
  } else if (paper->obj_flags.type_flag != ITEM_NOTE) {
    act("You can't write on $p.", FALSE, ch, paper, 0, TO_CHAR);
  } else if (paper->action_description) {
    cprintf(ch, "There's something written on it already.\r\n");
    return;
  } else {
    /*
     * we can write - hooray! 
     */
    cprintf(ch, "Ok.. go ahead and write.. end the note with a @.\r\n");
    act("$n begins to jot down a note.", TRUE, ch, 0, 0, TO_ROOM);
    ch->desc->str = &paper->action_description;
    ch->desc->max_str = MAX_NOTE_LENGTH;
  }
}

void do_group_tell(struct char_data *ch, const char *argument, int cmd)
{
  struct char_data                       *k = NULL;
  struct char_data                       *vict = NULL;
  struct follow_type                     *f = NULL;

  if (DEBUG)
    log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

  if (!*argument) {
    cprintf(ch, "usage: gtell <mesg>.\r\n");
    return;
  }
  if (!IS_AFFECTED(ch, AFF_GROUP)) {
    cprintf(ch, "But you are a member of no group?!\r\n");
    return;
  }
  if (GET_POS(ch) == POSITION_SLEEPING) {
    cprintf(ch, "You are to tired to do this....\r\n");
    return;
  }
  if (!ch->master)
    k = ch;
  else {
    /*
     * tell the leader of the group 
     */

    k = ch->master;
    if (IS_AFFECTED(k, AFF_GROUP))
      if ((GET_POS(k) != POSITION_SLEEPING) && (k->desc) && (k != ch))
	cprintf(k, "%s tells the group '%s'\r\n",
		(IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)), argument);
  }

  /*
   * tell the followers of the group and exclude ourselfs 
   */

  for (f = k->followers; f; f = f->next) {
    vict = f->follower;
    if (IS_AFFECTED(vict, AFF_GROUP))
      if ((GET_POS(vict) != POSITION_SLEEPING) && (vict->desc) && (vict != ch))
	cprintf(vict, "%s tells the group '%s'\r\n",
		(IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)), argument);
  }

  if (IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO))
    cprintf(ch, "You tell the group '%s'\r\n", argument);
}

void do_group_report(struct char_data *ch, const char *argument, int cmd)
{
  struct char_data                       *k = NULL;
  struct char_data                       *vict = NULL;
  struct follow_type                     *f = NULL;
  char                                    message[MAX_INPUT_LENGTH + 20] = "\0\0\0";

  if (DEBUG)
    log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

  if (!IS_AFFECTED(ch, AFF_GROUP)) {
    cprintf(ch, "But you are a member of no group?!\r\n");
    return;
  }
  if (GET_POS(ch) == POSITION_SLEEPING) {
    cprintf(ch, "You are to tired to do this....\r\n");
    return;
  }
  sprintf(message,
	  "\r\nGroup Report from:Name:%s Hits:%d(%d) Mana:%d(%d) Move:%d(%d)",
	  GET_NAME(ch),
	  GET_HIT(ch),
	  GET_MAX_HIT(ch), GET_MANA(ch), GET_MAX_MANA(ch), GET_MOVE(ch), GET_MAX_MOVE(ch));

  if (!ch->master)
    k = ch;
  else {
    /*
     * tell the leader of the group 
     */

    k = ch->master;
    if (IS_AFFECTED(k, AFF_GROUP))
      if ((GET_POS(k) != POSITION_SLEEPING) && (k->desc) && (k != ch))
	cprintf(k, "%s", message);
  }

  /*
   * tell the followers of the group and exclude ourselfs 
   */

  for (f = k->followers; f; f = f->next) {
    vict = f->follower;
    if (IS_AFFECTED(vict, AFF_GROUP))
      if ((GET_POS(vict) != POSITION_SLEEPING) && (vict->desc) && (vict != ch))
	cprintf(vict, "%s", message);
  }

  if (IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO))
    cprintf(ch, "You tell the group your stats.\r\n");
}

void do_split(struct char_data *ch, const char *argument, int cmd)
{
  struct char_data                       *vict = NULL;
  struct follow_type                     *f = NULL;
  int                                     amount = 0;
  int                                     count = 0;
  int                                     share = 0;
  char                                    blah[256] = "\0\0\0";

  if (DEBUG)
    log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

  if (!IS_AFFECTED(ch, AFF_GROUP)) {
    cprintf(ch, "You are a member of no group!\r\n");
    return;
  }
  if (ch->master) {
    cprintf(ch, "You must be the leader of the group to use this.\r\n");
    return;
  }
  one_argument(argument, blah);

  if (!is_number(blah)) {
    cprintf(ch, "You must supply an integer amount.\r\n");
    return;
  }
  amount = atoi(blah);
  if (amount > GET_GOLD(ch)) {
    cprintf(ch, "You do not have that much gold.\r\n");
    return;
  }
  GET_GOLD(ch) -= amount;
  count = 1;

  for (f = ch->followers; f; f = f->next) {
    vict = f->follower;
    if (IS_AFFECTED(vict, AFF_GROUP)) {
      if ((vict->desc) && (vict != ch))
	count++;
    }
  }

  share = amount / count;
  GET_GOLD(ch) += share;

  cprintf(ch, "You split the gold into shares of %d coin(s)\r\n", share);

  for (f = ch->followers; f; f = f->next) {
    vict = f->follower;
    if (IS_AFFECTED(vict, AFF_GROUP)) {
      if ((vict->desc) && (vict != ch)) {
	cprintf(ch, "%s gives you %d coins.\r\n",
		(IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)), share);
	GET_GOLD(vict) += share;
      }
    }
  }
}

void do_land(struct char_data *ch, const char *argument, int cmd)
{
  if (DEBUG)
    log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

  if (IS_NOT_SET(ch->specials.affected_by, AFF_FLYING)) {
    cprintf(ch, "But you are not flying?\r\n");
    return;
  }
  act("$n slowly lands on the ground.", FALSE, ch, 0, 0, TO_ROOM);

  if (affected_by_spell(ch, SPELL_FLY))
    affect_from_char(ch, SPELL_FLY);
  if (IS_SET(ch->specials.affected_by, AFF_FLYING))
    REMOVE_BIT(ch->specials.affected_by, AFF_FLYING);

  cprintf(ch, "You feel the extreme pull of gravity...\r\n");
}

void do_invis_off(struct char_data *ch, const char *argument, int cmd)
{
  if (DEBUG)
    log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

  if (!IS_SET(ch->specials.affected_by, AFF_INVISIBLE)) {
    cprintf(ch, "But you are not invisible?\r\n");
    return;
  }
  act("$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM);
  act("You feel exposed.", FALSE, ch, 0, 0, TO_CHAR);

  if (affected_by_spell(ch, SPELL_INVISIBLE))
    affect_from_char(ch, SPELL_INVISIBLE);

  if (IS_SET(ch->specials.affected_by, AFF_INVISIBLE))
    REMOVE_BIT(ch->specials.affected_by, AFF_INVISIBLE);
}

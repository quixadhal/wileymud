/*
 * file: act.social.c , Implementation of commands.       Part of DIKUMUD
 * Usage : Social commands.
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>

#include "include/global.h"
#include "include/bug.h"
#include "include/utils.h"
#include "include/comm.h"
#include "include/interpreter.h"
#include "include/handler.h"
#include "include/db.h"
#include "include/spells.h"
#include "include/multiclass.h"
#define _ACT_SOCIAL_C
#include "include/act_social.h"

struct social_messg *soc_mess_list = 0;
struct pose_type pose_messages[MAX_MESSAGES];
static int list_top = -1;

char *fread_action(FILE * fl)
{
  char buf[MAX_STRING_LENGTH], *rslt;

  if (DEBUG)
    dlog("fread_action");

  for (;;) {
    fgets(buf, MAX_STRING_LENGTH, fl);
    if (feof(fl)) {
      log("Fread_action - unexpected EOF.");
      exit(0);
    }
    if (*buf == '#')
      return (0);
    else {
      *(buf + strlen(buf) - 1) = '\0';
      CREATE(rslt, char, strlen(buf) + 1);

      strcpy(rslt, buf);
      return (rslt);
    }
  }
}

void boot_social_messages(void)
{
  FILE *fl;
  int tmp, hide, min_pos;

  if (DEBUG)
    dlog("boot_social_messages");
  if (!(fl = fopen(SOCMESS_FILE, "r"))) {
    perror("boot_social_messages");
    exit(0);
  }
  for (;;) {
    fscanf(fl, " %d ", &tmp);
    if (tmp < 0)
      break;
    fscanf(fl, " %d ", &hide);
    fscanf(fl, " %d \n", &min_pos);

    /* alloc a new cell */
    if (!soc_mess_list) {
      CREATE(soc_mess_list, struct social_messg, ((list_top = 0), 1));
    } else {
      RECREATE(soc_mess_list, struct social_messg, (++list_top + 1));
    }

    /* read the stuff */
    soc_mess_list[list_top].act_nr = tmp;
    soc_mess_list[list_top].hide = hide;
    soc_mess_list[list_top].min_victim_position = min_pos;

    soc_mess_list[list_top].char_no_arg = fread_action(fl);
    soc_mess_list[list_top].others_no_arg = fread_action(fl);

    soc_mess_list[list_top].char_found = fread_action(fl);

    /* if no char_found, the rest is to be ignored */
    if (!soc_mess_list[list_top].char_found)
      continue;

    soc_mess_list[list_top].others_found = fread_action(fl);
    soc_mess_list[list_top].vict_found = fread_action(fl);

    soc_mess_list[list_top].not_found = fread_action(fl);

    soc_mess_list[list_top].char_auto = fread_action(fl);

    soc_mess_list[list_top].others_auto = fread_action(fl);
  }
  fclose(fl);
}

int find_action(int cmd)
{
  int bot, top, mid;

  if (DEBUG)
    dlog("find_action");
  bot = 0;
  top = list_top;

  if (top < 0)
    return (-1);

  for (;;) {
    mid = (bot + top) / 2;

    if (soc_mess_list[mid].act_nr == cmd)
      return (mid);
    if (bot == top)
      return (-1);

    if (soc_mess_list[mid].act_nr > cmd)
      top = --mid;
    else
      bot = ++mid;
  }
}

void do_action(struct char_data *ch, char *argument, int cmd)
{
  int act_nr;
  char buf[MAX_INPUT_LENGTH];
  struct social_messg *action;
  struct char_data *vict;

  if (DEBUG)
    dlog("do_action");
  if ((act_nr = find_action(cmd)) < 0) {
    cprintf(ch, "That action is not supported.\n\r");
    return;
  }
  action = &soc_mess_list[act_nr];

  if (action->char_found)
    only_argument(argument, buf);
  else
    *buf = '\0';

  if (!*buf) {
    cprintf(ch, action->char_no_arg);
    cprintf(ch, "\n\r");
    act(action->others_no_arg, action->hide, ch, 0, 0, TO_ROOM);
    return;
  }
  if (!(vict = get_char_room_vis(ch, buf))) {
    cprintf(ch, action->not_found);
    cprintf(ch, "\n\r");
  } else if (vict == ch) {
    cprintf(ch, action->char_auto);
    cprintf(ch, "\n\r");
    act(action->others_auto, action->hide, ch, 0, 0, TO_ROOM);
  } else {
    if (GET_POS(vict) < action->min_victim_position) {
      act("$N is not in a proper position for that.", FALSE, ch, 0, vict, TO_CHAR);
    } else {
      act(action->char_found, 0, ch, 0, vict, TO_CHAR);

      act(action->others_found, action->hide, ch, 0, vict, TO_NOTVICT);

      act(action->vict_found, action->hide, ch, 0, vict, TO_VICT);
    }
  }
}

void do_insult(struct char_data *ch, char *argument, int cmd)
{
  static char arg[MAX_STRING_LENGTH];
  struct char_data *victim;

  only_argument(argument, arg);

  if (*arg) {
    if (!(victim = get_char_room_vis(ch, arg))) {
      cprintf(ch, "Can't hear you!\n\r");
    } else {
      if (victim != ch) {
	cprintf(ch, "You insult %s.\n\r", GET_NAME(victim));

	switch (random() % 3) {
	case 0:{
	    if (GET_SEX(ch) == SEX_MALE) {
	      if (GET_SEX(victim) == SEX_MALE)
		act(
		  "$n accuses you of fighting like a woman!", FALSE,
		     ch, 0, victim, TO_VICT);
	      else
		act("$n says that women can't fight.",
		    FALSE, ch, 0, victim, TO_VICT);
	    } else {		       /* Ch == Woman */
	      if (GET_SEX(victim) == SEX_MALE)
		act("$n accuses you of having the smallest.... (brain?)",
		    FALSE, ch, 0, victim, TO_VICT);
	      else
		act("$n tells you that you'd loose a beautycontest against a troll.",
		    FALSE, ch, 0, victim, TO_VICT);
	    }
	  }
	  break;
	case 1:{
	    act("$n calls your mother a bitch!",
		FALSE, ch, 0, victim, TO_VICT);
	  }
	  break;
	default:{
	    act("$n tells you to get lost!", FALSE, ch, 0, victim, TO_VICT);
	  }
	  break;
	}			       /* end switch */

	act("$n insults $N.", TRUE, ch, 0, victim, TO_NOTVICT);
      } else {			       /* ch == victim */
	cprintf(ch, "You feel insulted.\n\r");
      }
    }
  } else
    cprintf(ch, "Sure you don't want to insult everybody.\n\r");
}

void boot_pose_messages(void)
{
  FILE *fl;
  int counter, class;

  return;

  if (!(fl = fopen(POSEMESS_FILE, "r"))) {
    perror("boot_pose_messages");
    exit(0);
  }
  for (counter = 0;; counter++) {
    fscanf(fl, " %d ", &pose_messages[counter].level);
    if (pose_messages[counter].level < 0)
      break;
    for (class = 0; class < 4; class++) {
      pose_messages[counter].poser_msg[class] = fread_action(fl);
      pose_messages[counter].room_msg[class] = fread_action(fl);
    }
  }

  fclose(fl);
}

void do_pose(struct char_data *ch, char *argument, int cmd)
{
  int to_pose;
  int counter;

  cprintf(ch, "Sorry Buggy command.\n\r");
  return;

  if ((GetMaxLevel(ch) < pose_messages[0].level) || IS_NPC(ch)) {
    cprintf(ch, "You can't do that.\n\r");
    return;
  }
  for (counter = 0; (pose_messages[counter].level < GetMaxLevel(ch)) &&
       (pose_messages[counter].level > 0); counter++);
  counter--;

  to_pose = number(0, counter);
/*
 * **  find highest level, use that.
 */

}

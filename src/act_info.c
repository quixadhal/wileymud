/*
 * file: act.informative.c , Implementation of commands.  Part of DIKUMUD
 * Usage : Informative commands.
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "include/global.h"
#include "include/bug.h"
#include "include/version.h"
#include "include/utils.h"
#include "include/comm.h"
#include "include/interpreter.h"
#include "include/handler.h"
#include "include/db.h"
#include "include/spells.h"
#include "include/mudlimits.h"
#include "include/trap.h"
#include "include/hash.h"
#include "include/constants.h"
#include "include/spell_parser.h"
#include "include/whod.h"
#include "include/multiclass.h"
#include "include/modify.h"
#include "include/act_wiz.h"
#include "include/act_skills.h"
#include "include/spec_procs.h"
#include "include/tracking.h"
#define _ACT_INFO_C
#include "include/act_info.h"

/* Procedures related to 'look' */

void argument_split_2(char *argument, char *first_arg, char *second_arg)
{
  int look_at, found, begin;

  found = begin = 0;
  if (DEBUG)
    dlog("argument_split_2");
  /* Find first non blank */
  for (; *(argument + begin) == ' '; begin++);

  /* Find length of first word */
  for (look_at = 0; *(argument + begin + look_at) > ' '; look_at++)
    /* Make all letters lower case, AND copy them to first_arg */
    *(first_arg + look_at) = LOWER(*(argument + begin + look_at));
  *(first_arg + look_at) = '\0';
  begin += look_at;

  /* Find first non blank */
  for (; *(argument + begin) == ' '; begin++);

  /* Find length of second word */
  for (look_at = 0; *(argument + begin + look_at) > ' '; look_at++)
    /* Make all letters lower case, AND copy them to second_arg */
    *(second_arg + look_at) = LOWER(*(argument + begin + look_at));
  *(second_arg + look_at) = '\0';
  begin += look_at;
}

struct obj_data *get_object_in_equip_vis(struct char_data *ch,
		    char *arg, struct obj_data *equipment[], int *j)
{

  if (DEBUG)
    dlog("get_object_in_equip_vis");
  for ((*j) = 0; (*j) < MAX_WEAR; (*j)++)
    if (equipment[(*j)])
      if (CAN_SEE_OBJ(ch, equipment[(*j)]))
	if (isname(arg, equipment[(*j)]->name))
	  return (equipment[(*j)]);

  return (0);
}

char *find_ex_description(char *word, struct extra_descr_data *list)
{
  struct extra_descr_data *i;

  if (DEBUG)
    dlog("find_ex_description");
  for (i = list; i; i = i->next)
    if (isname(word, i->keyword))
      return (i->description);

  return (0);
}

void show_obj_to_char(struct obj_data *object, struct char_data *ch, int mode)
{
  char buffer[MAX_STRING_LENGTH] = "\0";

  if (DEBUG)
    dlog("show_obj_to_char");
  if ((mode == 0) && object->description)
    strcpy(buffer, object->description);
  else if (object->short_description && ((mode == 1) ||
			 (mode == 2) || (mode == 3) || (mode == 4)))
    strcpy(buffer, object->short_description);
  else if (mode == 5) {
    if (object->obj_flags.type_flag == ITEM_NOTE) {
      if (object->action_description) {
	strcpy(buffer, "There is writing on it:\n\r\n\r");
	strcat(buffer, object->action_description);
	/* page_string(ch->desc, buffer, 1); */
      } else {
	act("It is blank.", FALSE, ch, 0, 0, TO_CHAR);
      }
    } else if ((object->obj_flags.type_flag != ITEM_DRINKCON)) {
      strcpy(buffer, "You see nothing special..");
    } else {			       /* ITEM_TYPE == ITEM_DRINKCON */
      strcpy(buffer, "It looks to be a drink container.");
    }
  }
  if (mode != 3) {
    if (object->obj_flags.type_flag == ITEM_ARMOR) {
      if (object->obj_flags.value[0] < (object->obj_flags.value[1] / 6))
	strcat(buffer, "..it is extremely beaten on.");
      else if (object->obj_flags.value[0] < (object->obj_flags.value[1] / 5))
	strcat(buffer, "..it is severely beaten on.");
      else if (object->obj_flags.value[0] < (object->obj_flags.value[1] / 4))
	strcat(buffer, "..it is badly beaten on.");
      else if (object->obj_flags.value[0] < (object->obj_flags.value[1] / 3))
	strcat(buffer, "..it looks barely useable.");
      else if (object->obj_flags.value[0] < (object->obj_flags.value[1] / 2))
	strcat(buffer, "..it is showing wear.");
      else if (object->obj_flags.value[0] < object->obj_flags.value[1])
	strcat(buffer, "..it's in fair shape.");
      else
	strcat(buffer, "");
    }
    if (IS_OBJ_STAT(object, ITEM_ANTI_GOOD) && IS_AFFECTED(ch, AFF_DETECT_EVIL))
      strcat(buffer, "..it glows red!");
    if (IS_OBJ_STAT(object, ITEM_MAGIC) && IS_AFFECTED(ch, AFF_DETECT_MAGIC))
      strcat(buffer, "..it glows blue!");
    if (IS_OBJ_STAT(object, ITEM_GLOW))
      strcat(buffer, "..it glows softly!");
    if (IS_OBJ_STAT(object, ITEM_HUM))
      strcat(buffer, "..it emits a faint hum!");
    if (IS_OBJ_STAT(object, ITEM_INVISIBLE))
      strcat(buffer, "(invisible)");
  }
  strcat(buffer, "\n\r");
  page_string(ch->desc, buffer, 1);
}

void show_mult_obj_to_char(struct obj_data *object, struct char_data *ch, int mode, int num)
{
  char buffer[MAX_STRING_LENGTH] = "\0";
  char tmp[10] = "\0";

  if (DEBUG)
    dlog("show_mult_obj_to_char");
  if ((mode == 0) && object->description)
    strcpy(buffer, object->description);
  else if (object->short_description && ((mode == 1) ||
			 (mode == 2) || (mode == 3) || (mode == 4)))
    strcpy(buffer, object->short_description);
  else if (mode == 5) {
    if (object->obj_flags.type_flag == ITEM_NOTE) {
      if (object->action_description) {
	strcpy(buffer, "There is writing on it:\n\r\n\r");
	strcat(buffer, object->action_description);
	page_string(ch->desc, buffer, 1);
      } else
	act("It's blank.", FALSE, ch, 0, 0, TO_CHAR);
      return;
    } else if ((object->obj_flags.type_flag != ITEM_DRINKCON)) {
      strcpy(buffer, "You see nothing special..");
    } else {			       /* ITEM_TYPE == ITEM_DRINKCON */
      strcpy(buffer, "It looks like a drink container.");
    }
  }
  if (mode != 3) {
    if (IS_OBJ_STAT(object, ITEM_INVISIBLE)) {
      strcat(buffer, "(invisible)");
    }
    if (IS_OBJ_STAT(object, ITEM_ANTI_GOOD) &&
	IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
      strcat(buffer, "..it glows red!");
    }
    if (IS_OBJ_STAT(object, ITEM_MAGIC) && IS_AFFECTED(ch, AFF_DETECT_MAGIC)) {
      strcat(buffer, "..it glows blue!");
    }
    if (IS_OBJ_STAT(object, ITEM_GLOW)) {
      strcat(buffer, "..it glows softly!");
    }
    if (IS_OBJ_STAT(object, ITEM_HUM)) {
      strcat(buffer, "..it emits a faint hum!");
    }
  }
  if (num > 1) {
    sprintf(tmp, "[%d]", num);
    strcat(buffer, tmp);
  }
  strcat(buffer, "\n\r");
  page_string(ch->desc, buffer, 1);
}

void list_obj_in_room(struct obj_data *list, struct char_data *ch)
{

  struct obj_data *i, *cond_ptr[50];
  int Inventory_Num = 1, num;
  int k, cond_top, cond_tot[50], found = FALSE;

  if (DEBUG)
    dlog("list_obj_in_room");

  cond_top = 0;

  for (i = list; i; i = i->next_content) {
    if (CAN_SEE_OBJ(ch, i)) {
      if (cond_top < 50) {
	found = FALSE;
	for (k = 0; (k < cond_top && !found); k++) {
	  if (cond_top > 0) {
	    if ((i->item_number == cond_ptr[k]->item_number) &&
		(i->description && cond_ptr[k]->description &&
	       !strcmp(i->description, cond_ptr[k]->description))) {
	      cond_tot[k] += 1;
	      found = TRUE;
	    }
	  }
	}
	if (!found) {
	  cond_ptr[cond_top] = i;
	  cond_tot[cond_top] = 1;
	  cond_top += 1;
	}
      } else {
	if ((ITEM_TYPE(i) == ITEM_TRAP) || (GET_TRAP_CHARGES(i) > 0)) {
	  num = number(1, 100);
	  if (num < ch->skills[SKILL_FIND_TRAP].learned / 3)
	    show_obj_to_char(i, ch, 0);
	} else {
	  show_obj_to_char(i, ch, 0);
	}
      }
    }
  }

  if (cond_top) {
    for (k = 0; k < cond_top; k++) {
      if ((ITEM_TYPE(cond_ptr[k]) == ITEM_TRAP) && (GET_TRAP_CHARGES(cond_ptr[k]) > 0)) {
	num = number(1, 100);
	if (num < ch->skills[SKILL_FIND_TRAP].learned / 3) {
	  if (cond_tot[k] > 1) {
	    cprintf(ch, "[%2d] ", Inventory_Num++);
	    show_mult_obj_to_char(cond_ptr[k], ch, 0, cond_tot[k]);
	  } else {
	    show_obj_to_char(cond_ptr[k], ch, 0);
	  }
        }
      } else {
	if (cond_tot[k] > 1) {
	  cprintf(ch, "[%2d] ", Inventory_Num++);
	  show_mult_obj_to_char(cond_ptr[k], ch, 0, cond_tot[k]);
	} else {
	  show_obj_to_char(cond_ptr[k], ch, 0);
	}
      }
    }
  }
}

void list_obj_on_char(struct obj_data *list, struct char_data *ch)
{
  struct obj_data *i, *cond_ptr[50];
  int k, cond_top, cond_tot[50], found = FALSE;

  int Num_Inventory = 1;

  cond_top = 0;

  if (DEBUG)
    dlog("list_obj_on_char");
  if (!list) {
    cprintf(ch, "   Nothing\n\r");
    return;
  }
  for (i = list; i; i = i->next_content) {
    if (CAN_SEE_OBJ(ch, i)) {
      if (cond_top < 50) {
	found = FALSE;
	for (k = 0; (k < cond_top && !found); k++) {
	  if (cond_top > 0) {
	    if ((i->item_number == cond_ptr[k]->item_number) &&
		(i->short_description && cond_ptr[k]->short_description &&
		 (!strcmp(i->short_description, cond_ptr[k]->short_description)))) {
	      cond_tot[k] += 1;
	      found = TRUE;
	    }
	  }
	}
	if (!found) {
	  cond_ptr[cond_top] = i;
	  cond_tot[cond_top] = 1;
	  cond_top += 1;
	}
      } else {
	show_obj_to_char(i, ch, 2);
      }
    }
  }

  if (cond_top) {
    for (k = 0; k < cond_top; k++) {
      if (cond_tot[k] > 1) {
	Num_Inventory += cond_tot[k] - 1;
	show_mult_obj_to_char(cond_ptr[k], ch, 2, cond_tot[k]);
      } else {
	show_obj_to_char(cond_ptr[k], ch, 2);
      }
    }
  }
}

void list_obj_to_char(struct obj_data *list, struct char_data *ch, int mode,
		      BYTE show)
{
  int Num_In_Bag = 1;
  struct obj_data *i;
  BYTE found;

  if (DEBUG)
    dlog("list_obj_on_char");
  found = FALSE;
  for (i = list; i; i = i->next_content) {
    if (CAN_SEE_OBJ(ch, i)) {
      cprintf(ch, "[%2d] ", Num_In_Bag++);
      show_obj_to_char(i, ch, mode);
      found = TRUE;
    }
  }
  if ((!found) && (show))
    cprintf(ch, "Nothing\n\r");
}

void show_char_to_char(struct char_data *i, struct char_data *ch, int mode)
{
  char buffer[MAX_STRING_LENGTH];
  int j, found, percent;
  struct obj_data *tmp_obj;

  if (DEBUG)
    dlog("show_char_to_char");
  if (mode == 0) {
    if (IS_AFFECTED(i, AFF_HIDE) || !CAN_SEE(ch, i)) {
      if (IS_AFFECTED(ch, AFF_SENSE_LIFE))
	cprintf(ch, "You sense a hidden life form in the room.\n\r");
      return;
    }
    if (!(i->player.long_descr) || (GET_POS(i) != i->specials.default_pos)) {
      /* A player char or a mobile without long descr, or not in default pos. */
      if (IS_PC(i)) {
	strcpy(buffer, GET_NAME(i));
	strcat(buffer, " ");
	if (GET_TITLE(i))
	  strcat(buffer, GET_TITLE(i));
      } else {
	strcpy(buffer, i->player.short_descr);
        *buffer= toupper(*buffer);
      }

      if (IS_AFFECTED(i, AFF_INVISIBLE))
	strcat(buffer, " (invisible)");
      if (IS_AFFECTED(i, AFF_CHARM))
	strcat(buffer, " (pet)");

      switch (GET_POS(i)) {
      case POSITION_STUNNED:
	strcat(buffer, " is lying here, stunned");
	break;
      case POSITION_INCAP:
	strcat(buffer, " is lying here, incapacitated");
	break;
      case POSITION_MORTALLYW:
	strcat(buffer, " is lying here, mortally wounded");
	break;
      case POSITION_DEAD:
	strcat(buffer, " is lying here, dead");
	break;
      case POSITION_MOUNTED:
	if (MOUNTED(i)) {
	  strcat(buffer, " is here, riding ");
	  strcat(buffer, MOUNTED(i)->player.short_descr);
	} else
	  strcat(buffer, " is standing here.");
	break;
      case POSITION_STANDING:
	strcat(buffer, " is standing here");
	break;
      case POSITION_SITTING:
	strcat(buffer, " is sitting here");
	break;
      case POSITION_RESTING:
	strcat(buffer, " is resting here");
	break;
      case POSITION_SLEEPING:
	strcat(buffer, " is sleeping here");
	break;
      case POSITION_FIGHTING:
	if (i->specials.fighting) {
	  strcat(buffer, " is here, fighting ");
	  if (i->specials.fighting == ch)
	    strcat(buffer, " YOU!");
	  else {
	    if (i->in_room == i->specials.fighting->in_room)
	      if (IS_NPC(i->specials.fighting))
		strcat(buffer, i->specials.fighting->player.short_descr);
	      else
		strcat(buffer, GET_NAME(i->specials.fighting));
	    else
	      strcat(buffer, "someone who has already left.");
	  }
	} else			       /* NIL fighting pointer */
	  strcat(buffer, " is here struggling with thin air");
	break;

      default:
	strcat(buffer, " is floating here");
	break;
      }

      if (IS_AFFECTED(i, AFF_FLYING)) {
	strcat(buffer, " inches above the ground");
      }
      strcat(buffer, ".");

      if (IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
	if (IS_REALLY_VILE(i) || IS_VILE(i) || IS_EVIL(i))
	  strcat(buffer, " (Red Aura)");
      }
      strcat(buffer, "\n\r");
      cprintf(ch, buffer);
    } else {			       /* npc with long */
      if (IS_AFFECTED(i, AFF_INVISIBLE))
	strcpy(buffer, "*");
      else
	*buffer = '\0';

      if (IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
	if (IS_REALLY_VILE(i) || IS_VILE(i) || IS_EVIL(i))
	  strcat(buffer, " (Red Aura)");
      }
      strcat(buffer, i->player.long_descr);

      cprintf(ch, buffer);
    }

    if (IS_AFFECTED(i, AFF_SANCTUARY))
      act("$n glows with a bright light!", FALSE, i, 0, ch, TO_VICT);
    if (affected_by_spell(i, SPELL_FIRESHIELD))
      act("$n is surrounded by flickering flames!", FALSE, i, 0, ch, TO_VICT);

  } else if (mode == 1) {
    if (i->player.description)
      cprintf(ch, i->player.description);
    else {
      act("You see nothing special about $m.", FALSE, i, 0, ch, TO_VICT);
    }

    if (IS_PC(i)) {
      if(ch == i) {
        if(IS_SET(i->specials.new_act, NEW_PLR_KILLOK)) {
          sprintf(buffer, "You are %s and bear the Mark of Dread Quixadhal.\n\r", RaceName[GET_RACE(i)]);
        } else {
          sprintf(buffer, "You are %s.\n\r", RaceName[GET_RACE(i)]);
        }
      } else {
        if(IS_SET(ch->specials.new_act, NEW_PLR_KILLOK)) {
          if(IS_SET(i->specials.new_act, NEW_PLR_KILLOK)) {
            sprintf(buffer, "%s is %s and also bears the Mark of the Dread Lord.\n\r", GET_NAME(i), RaceName[GET_RACE(i)]);
          } else {
            sprintf(buffer, "%s is %s.\n\r", GET_NAME(i), RaceName[GET_RACE(i)]);
          }
        } else {
          if(IS_SET(i->specials.new_act, NEW_PLR_KILLOK)) {
            sprintf(buffer, "%s is %s and bears the Mark of Dread Quixadhal.\n\r", GET_NAME(i), RaceName[GET_RACE(i)]);
          } else {
            sprintf(buffer, "%s is %s.\n\r", GET_NAME(i), RaceName[GET_RACE(i)]);
          }
        }
      }
    } else
      sprintf(buffer, "%s is %s.\n\r", MOB_NAME(i), RaceName[GET_RACE(i)]);

    cprintf(ch, buffer);

    if (MOUNTED(i)) {
      sprintf(buffer, "$n is mounted on %s", MOUNTED(i)->player.short_descr);
      act(buffer, FALSE, i, 0, ch, TO_VICT);
    }
    if (RIDDEN(i)) {
      if (CAN_SEE(ch, RIDDEN(i))) {
	sprintf(buffer, "$n is ridden by %s",
		IS_NPC(RIDDEN(i)) ? RIDDEN(i)->player.short_descr : GET_NAME(RIDDEN(i)));
	act(buffer, FALSE, i, 0, ch, TO_VICT);
      }
    }
    /* Show a character to another */

    if (GET_MAX_HIT(i) > 0)
      percent = (100 * GET_HIT(i)) / GET_MAX_HIT(i);
    else
      percent = -1;		       /* How could MAX_HIT be < 1?? */

    if (IS_NPC(i))
      strcpy(buffer, i->player.short_descr);
    else
      strcpy(buffer, GET_NAME(i));

    if (percent >= 100)
      strcat(buffer, " is in an excellent condition.\n\r");
    else if (percent >= 90)
      strcat(buffer, " has a few scratches.\n\r");
    else if (percent >= 75)
      strcat(buffer, " has some small wounds and bruises.\n\r");
    else if (percent >= 50)
      strcat(buffer, " has quite a few wounds.\n\r");
    else if (percent >= 30)
      strcat(buffer, " has some big nasty wounds and scratches.\n\r");
    else if (percent >= 15)
      strcat(buffer, " looks pretty hurt.\n\r");
    else if (percent >= 0)
      strcat(buffer, " is in an awful condition.\n\r");
    else
      strcat(buffer, " is bleeding awfully from big wounds.\n\r");

    cprintf(ch, buffer);

    found = FALSE;
    for (j = 0; j < MAX_WEAR; j++) {
      if (i->equipment[j]) {
	if (CAN_SEE_OBJ(ch, i->equipment[j])) {
	  found = TRUE;
	}
      }
    }
    if (found) {
      act("\n\r$n is using:", FALSE, i, 0, ch, TO_VICT);
      for (j = 0; j < MAX_WEAR; j++) {
	if (i->equipment[j]) {
	  if (CAN_SEE_OBJ(ch, i->equipment[j])) {
	    cprintf(ch, (char *)where[j]);
	    show_obj_to_char(i->equipment[j], ch, 1);
	  }
	}
      }
    }
    if (HasClass(ch, CLASS_THIEF) && (ch != i) &&
	(!IS_IMMORTAL(ch))) {
      found = FALSE;
      cprintf(ch, "\n\rYou attempt to peek at their inventory:\n\r");
      for (tmp_obj = i->carrying; tmp_obj;
	   tmp_obj = tmp_obj->next_content) {
	if (CAN_SEE_OBJ(ch, tmp_obj) &&
	    (number(0, MAX_MORT) < GetMaxLevel(ch))) {
	  show_obj_to_char(tmp_obj, ch, 1);
	  found = TRUE;
	}
      }
      if (!found)
	cprintf(ch, "You can't see anything.\n\r");
    } else if (IS_IMMORTAL(ch)) {
      cprintf(ch, "Inventory:\n\r");
      for (tmp_obj = i->carrying; tmp_obj;
	   tmp_obj = tmp_obj->next_content) {
	show_obj_to_char(tmp_obj, ch, 1);
	found = TRUE;
      }
      if (!found) {
	cprintf(ch, "Nothing\n\r");
      }
    }
  } else if (mode == 2) {

    /* Lists inventory */
    act("$n is carrying:", FALSE, i, 0, ch, TO_VICT);
    list_obj_on_char(i->carrying, ch);
  }
}

void show_mult_char_to_char(struct char_data *i, struct char_data *ch, int mode, int num)
{
  char buffer[MAX_STRING_LENGTH];
  char tmp[10];
  int j, found, percent;
  struct obj_data *tmp_obj;

  if (DEBUG)
    dlog("show_mult_char_to_char");
  if (mode == 0) {
    if (IS_AFFECTED(i, AFF_HIDE) || !CAN_SEE(ch, i)) {
      if (IS_AFFECTED(ch, AFF_SENSE_LIFE)) {
	if (num == 1)
	  cprintf(ch, "You sense a hidden life form in the room.\n\r");
	else
	  cprintf(ch, "You sense hidden life forma in the room.\n\r");
      }
      return;
    }
    if (!(i->player.long_descr) || (GET_POS(i) != i->specials.default_pos)) {
      /* A player char or a mobile without long descr, or not in default pos. */
      if (!IS_NPC(i)) {
	strcpy(buffer, GET_NAME(i));
	strcat(buffer, " ");
	if (GET_TITLE(i))
	  strcat(buffer, GET_TITLE(i));
      } else {
	strcpy(buffer, i->player.short_descr);
	*buffer= toupper(*buffer);
      }

      if (IS_AFFECTED(i, AFF_INVISIBLE))
	strcat(buffer, " (invisible)");
      if (IS_AFFECTED(i, AFF_CHARM))
	strcat(buffer, " (pet)");

      switch (GET_POS(i)) {
      case POSITION_STUNNED:
	strcat(buffer, " is lying here, stunned");
	break;
      case POSITION_INCAP:
	strcat(buffer, " is lying here, incapacitated");
	break;
      case POSITION_MORTALLYW:
	strcat(buffer, " is lying here, mortally wounded");
	break;
      case POSITION_DEAD:
	strcat(buffer, " is lying here, dead");
	break;
      case POSITION_STANDING:
	strcat(buffer, " is standing here");
	break;
      case POSITION_SITTING:
	strcat(buffer, " is sitting here");
	break;
      case POSITION_RESTING:
	strcat(buffer, " is resting here");
	break;
      case POSITION_SLEEPING:
	strcat(buffer, " is sleeping here");
	break;
      case POSITION_FIGHTING:
	if (i->specials.fighting) {

	  strcat(buffer, " is here, fighting ");
	  if (i->specials.fighting == ch)
	    strcat(buffer, " YOU!");
	  else {
	    if (i->in_room == i->specials.fighting->in_room)
	      if (IS_NPC(i->specials.fighting))
		strcat(buffer, i->specials.fighting->player.short_descr);
	      else
		strcat(buffer, GET_NAME(i->specials.fighting));
	    else
	      strcat(buffer, "someone who has already left.");
	  }
	} else			       /* NIL fighting pointer */
	  strcat(buffer, " is here struggling with thin air");
	break;

      default:
	strcat(buffer, " is floating here.");
	break;
      }
      if (IS_AFFECTED(i, AFF_FLYING))
	strcat(buffer, " inches above the ground");
      strcat(buffer, ".");

      if (IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
	if (IS_REALLY_VILE(i) || IS_VILE(i) || IS_EVIL(i))
	  strcat(buffer, " (Red Aura)");
      }
      if (num > 1) {
	sprintf(tmp, " [%d]", num);
	strcat(buffer, tmp);
      }
      strcat(buffer, "\n\r");
      cprintf(ch, buffer);
    } else {			       /* npc with long */

      if (IS_AFFECTED(i, AFF_INVISIBLE))
	strcpy(buffer, "*");
      else
	*buffer = '\0';

      if (IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
	if (IS_REALLY_VILE(i) || IS_VILE(i) || IS_EVIL(i))
	  strcat(buffer, " (Red Aura)");
      }
      strcat(buffer, i->player.long_descr);

      /* this gets a little annoying */

      if (num > 1) {
	while ((buffer[strlen(buffer) - 1] == '\r') ||
	       (buffer[strlen(buffer) - 1] == '\n') ||
	       (buffer[strlen(buffer) - 1] == ' ')) {
	  buffer[strlen(buffer) - 1] = '\0';
	}
	sprintf(tmp, " [%d]\n\r", num);
	strcat(buffer, tmp);
      }
      cprintf(ch, buffer);
    }

    if (IS_AFFECTED(i, AFF_SANCTUARY))
      act("$n glows with a bright light!", FALSE, i, 0, ch, TO_VICT);
    if (affected_by_spell(i, SPELL_FIRESHIELD))
      act("$n is surrounded by flickering flames!", FALSE, i, 0, ch, TO_VICT);

  } else if (mode == 1) {

    if (i->player.description)
      cprintf(ch, i->player.description);
    else {
      act("You see nothing special about $m.", FALSE, i, 0, ch, TO_VICT);
    }

    /* Show a character to another */

    if (GET_MAX_HIT(i) > 0)
      percent = (100 * GET_HIT(i)) / GET_MAX_HIT(i);
    else
      percent = -1;		       /* How could MAX_HIT be < 1?? */

    if (IS_NPC(i))
      strcpy(buffer, i->player.short_descr);
    else
      strcpy(buffer, GET_NAME(i));

    if (percent >= 100)
      strcat(buffer, " is in an excellent condition.\n\r");
    else if (percent >= 90)
      strcat(buffer, " has a few scratches.\n\r");
    else if (percent >= 75)
      strcat(buffer, " has some small wounds and bruises.\n\r");
    else if (percent >= 50)
      strcat(buffer, " has quite a few wounds.\n\r");
    else if (percent >= 30)
      strcat(buffer, " has some big nasty wounds and scratches.\n\r");
    else if (percent >= 15)
      strcat(buffer, " looks pretty hurt.\n\r");
    else if (percent >= 0)
      strcat(buffer, " is in an awful condition.\n\r");
    else
      strcat(buffer, " is bleeding awfully from big wounds.\n\r");

    cprintf(ch, buffer);
    cprintf(ch, "%s is %s.\n\r", GET_NAME(i), RaceName[GET_RACE(i)]);

    found = FALSE;
    for (j = 0; j < MAX_WEAR; j++) {
      if (i->equipment[j]) {
	if (CAN_SEE_OBJ(ch, i->equipment[j])) {
	  found = TRUE;
	}
      }
    }
    if (found) {
      act("\n\r$n is using:", FALSE, i, 0, ch, TO_VICT);
      for (j = 0; j < MAX_WEAR; j++) {
	if (i->equipment[j]) {
	  if (CAN_SEE_OBJ(ch, i->equipment[j])) {
	    cprintf(ch, (char *)where[j]);
	    show_obj_to_char(i->equipment[j], ch, 1);
	  }
	}
      }
    }
    if ((HasClass(ch, CLASS_THIEF)) && (ch != i)) {
      found = FALSE;
      cprintf(ch, "\n\rYou attempt to peek at their inventory:\n\r");
      for (tmp_obj = i->carrying; tmp_obj; tmp_obj = tmp_obj->next_content) {
	if (CAN_SEE_OBJ(ch, tmp_obj) && (number(0, MAX_MORT) < GetMaxLevel(ch))) {
	  show_obj_to_char(tmp_obj, ch, 1);
	  found = TRUE;
	}
      }
      if (!found)
	cprintf(ch, "You can't see anything.\n\r");
    }
  } else if (mode == 2) {

    /* Lists inventory */
    act("$n is carrying:", FALSE, i, 0, ch, TO_VICT);
    list_obj_on_char(i->carrying, ch);
  }
}

void list_char_in_room(struct char_data *list, struct char_data *ch)
{
  struct char_data *i, *cond_ptr[50];
  int k, cond_top, cond_tot[50], found = FALSE;

  if (DEBUG)
    dlog("list_char_in_room");
  cond_top = 0;

  for (i = list; i; i = i->next_in_room) {
    if ((ch != i) &&
	(!RIDDEN(i) || (RIDDEN(i) && !CAN_SEE(ch, RIDDEN(i)))) &&
	(IS_AFFECTED(ch, AFF_SENSE_LIFE) ||
	 (CAN_SEE(ch, i) && !IS_AFFECTED(i, AFF_HIDE)))) {
      if ((cond_top < 50) && !MOUNTED(i)) {
	found = FALSE;
	if (IS_NPC(i)) {
	  for (k = 0; (k < cond_top && !found); k++) {
	    if (cond_top > 0) {
	      if (i->nr == cond_ptr[k]->nr &&
		  (GET_POS(i) == GET_POS(cond_ptr[k])) &&
		  (i->specials.affected_by == cond_ptr[k]->specials.affected_by) &&
		  (i->specials.fighting == cond_ptr[k]->specials.fighting) &&
		  (i->player.short_descr && cond_ptr[k]->player.short_descr &&
		   0 == strcmp(i->player.short_descr, cond_ptr[k]->player.short_descr))) {
		cond_tot[k] += 1;
		found = TRUE;
	      }
	    }
	  }
	}
	if (!found) {
	  cond_ptr[cond_top] = i;
	  cond_tot[cond_top] = 1;
	  cond_top += 1;
	}
      } else {
	show_char_to_char(i, ch, 0);
      }
    }
  }

  if (cond_top) {
    for (k = 0; k < cond_top; k++) {
      if (cond_tot[k] > 1) {
	show_mult_char_to_char(cond_ptr[k], ch, 0, cond_tot[k]);
      } else {
	show_char_to_char(cond_ptr[k], ch, 0);
      }
    }
  }
}

void list_char_to_char(struct char_data *list, struct char_data *ch,
		       int mode)
{
  struct char_data *i;

  if (DEBUG)
    dlog("list_char_to_char");
  for (i = list; i; i = i->next_in_room) {
    if ((ch != i) && (IS_AFFECTED(ch, AFF_SENSE_LIFE) ||
		      (CAN_SEE(ch, i) && !IS_AFFECTED(i, AFF_HIDE))))
      show_char_to_char(i, ch, 0);
  }
}

void do_look(struct char_data *ch, char *argument, int cmd)
{
  char buffer[MAX_STRING_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  int keyword_no, res;
  int j, bits, temp;
  BYTE found;
  struct obj_data *tmp_object, *found_object;
  struct char_data *tmp_char;
  char *tmp_desc;
  static char *keywords[] =
  {
    "north",
    "east",
    "south",
    "west",
    "up",
    "down",
    "in",
    "at",
    "",				       /* Look at '' case */
    "room",
    "\n"};

  if (DEBUG)
    dlog("do_look");

  if (!ch->desc)
    return;

  if (GET_POS(ch) < POSITION_SLEEPING)
    cprintf(ch, "You can't see anything but stars!\n\r");
  else if (GET_POS(ch) == POSITION_SLEEPING)
    cprintf(ch, "You can't see anything, you're sleeping!\n\r");
  else if (IS_AFFECTED(ch, AFF_BLIND))
    cprintf(ch, "You can't see a damn thing, you're blinded!\n\r");
  else if ((IS_DARK(ch->in_room)) && (!IS_IMMORTAL(ch)) &&
	   (!IS_AFFECTED(ch, AFF_TRUE_SIGHT))) {
    cprintf(ch, "It is very dark here...\n\r");
    if (IS_AFFECTED(ch, AFF_INFRAVISION)) {
      list_char_in_room(real_roomp(ch->in_room)->people, ch);
    }
  } else if ((IS_DARKOUT(ch->in_room)) && (!IS_IMMORTAL(ch)) &&
	     (!IS_AFFECTED(ch, AFF_TRUE_SIGHT))) {
    cprintf(ch, real_roomp(ch->in_room)->name);
    cprintf(ch, "\n\rIt is quite dark here...\n\r");
    if ((IS_AFFECTED(ch, AFF_INFRAVISION)) || BRIGHT_MOON(ch->in_room)) {
      list_char_in_room(real_roomp(ch->in_room)->people, ch);
    }
  } else {

    only_argument(argument, arg1);

    if (0 == strn_cmp(arg1, "at", 2) && isspace(arg1[2])) {
      only_argument(argument + 3, arg2);
      keyword_no = 7;
    } else if (0 == strn_cmp(arg1, "in", 2) && isspace(arg1[2])) {
      only_argument(argument + 3, arg2);
      keyword_no = 6;
    } else {
      keyword_no = search_block(arg1, keywords, FALSE);
    }

    if ((keyword_no == -1) && *arg1) {
      keyword_no = 7;
      only_argument(argument, arg2);
    }
    found = FALSE;
    tmp_object = 0;
    tmp_char = 0;
    tmp_desc = 0;

    switch (keyword_no) {
      /* look <dir> */
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
      {
	struct room_direction_data *exitp;

	exitp = NULL;
	exitp = EXIT(ch, keyword_no);

	if (exitp != NULL) {
	  if (exitp->general_description)
	    cprintf(ch, exitp->general_description);
	  else
	    cprintf(ch, "You see nothing special.\n\r");

	  if (IS_SET(exitp->exit_info, EX_CLOSED) && (exitp->keyword)) {
	    if ((strcmp(fname(exitp->keyword), "secret")) &&
		(IS_NOT_SET(exitp->exit_info, EX_SECRET))) {
	      cprintf(ch, "The %s is closed.\n\r", fname(exitp->keyword));
	    }
	  } else {
	    if (IS_SET(exitp->exit_info, EX_ISDOOR) && exitp->keyword) {
	      cprintf(ch, "The %s is open.\n\r", fname(exitp->keyword));
	    }
	  }
	} else if (cmd != SKILL_PEER)
	  cprintf(ch, "You see nothing special.\n\r");

	if ((exitp != NULL) &&
	    (IS_AFFECTED(ch, AFF_SCRYING) ||
	     IS_IMMORTAL(ch) ||
	     cmd == SKILL_PEER)) {
	  struct room_data *rp;

	  cprintf(ch, "You look %swards.\n\r", dirs[keyword_no]);

	  if (exitp != NULL) {
	    if (IS_SET(exitp->exit_info, EX_CLOSED) &&
		(exitp->keyword) && cmd == SKILL_PEER) {
	      if ((strcmp(fname(exitp->keyword), "secret")) &&
		  (IS_NOT_SET(exitp->exit_info, EX_SECRET))) {
		cprintf(ch, "The %s is closed.\n\r", fname(exitp->keyword));
		return;
	      }
	    }
	  }
	  if (!exitp || !exitp->to_room) {
	    cprintf(ch, "You see nothing special.");
	    return;
	  }
	  rp = real_roomp(exitp->to_room);
	  if (!rp) {
	    cprintf(ch, "You see a dark void.\n\r");
	  } else if (exitp) {
	    sprintf(buffer, "%d look", exitp->to_room);
	    do_at(ch, buffer, 0);
	  } else {
	    cprintf(ch, "You see nothing special.\n\r");
	  }
	} else
	  cprintf(ch, "You see nothing special.\n\r");
      }
      break;

      /* look 'in'      */
    case 6:{
	if (*arg2) {
	  /* Item carried */
	  bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM |
			FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

	  if (bits) {		       /* Found something */
	    if (GET_ITEM_TYPE(tmp_object) == ITEM_DRINKCON) {
	      if (tmp_object->obj_flags.value[1] <= 0) {
		act("It is empty.", FALSE, ch, 0, 0, TO_CHAR);
	      } else {
		temp = ((tmp_object->obj_flags.value[1] * 3) / tmp_object->obj_flags.value[0]);
		cprintf(ch, "It's %sfull of a %s liquid.\n\r",
			fullness[temp], color_liquid[tmp_object->obj_flags.value[2]]);
	      }
	    } else if (GET_ITEM_TYPE(tmp_object) == ITEM_CONTAINER) {
	      if (!IS_SET(tmp_object->obj_flags.value[1], CONT_CLOSED)) {
		cprintf(ch, fname(tmp_object->name));
		switch (bits) {
		case FIND_OBJ_INV:
		  cprintf(ch, " (carried) : \n\r");
		  break;
		case FIND_OBJ_ROOM:
		  cprintf(ch, " (here) : \n\r");
		  break;
		case FIND_OBJ_EQUIP:
		  cprintf(ch, " (used) : \n\r");
		  break;
		}
		list_obj_to_char(tmp_object->contains, ch, 2, TRUE);
	      } else
		cprintf(ch, "It is closed.\n\r");
	    } else {
	      cprintf(ch, "That is not a container.\n\r");
	    }
	  } else {		       /* wrong argument */
	    cprintf(ch, "You do not see that item here.\n\r");
	  }
	} else {		       /* no argument */
	  cprintf(ch, "Look in what?!\n\r");
	}
      }
      break;

      /* look 'at'      */
    case 7:{
	if (*arg2) {
	  bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM |
			      FIND_OBJ_EQUIP | FIND_CHAR_ROOM, ch, &tmp_char, &found_object);
	  if (tmp_char) {
	    show_char_to_char(tmp_char, ch, 1);
	    if (ch != tmp_char) {
              if(IS_SET(ch->specials.new_act, NEW_PLR_KILLOK)) {
                if(IS_SET(tmp_char->specials.new_act, NEW_PLR_KILLOK)) {
	          act("$n looks at you and grins.", TRUE, ch, 0, tmp_char, TO_VICT);
	          act("$n and $N share some private joke.", TRUE, ch, 0, tmp_char, TO_NOTVICT);
                } else {
	          act("$n looks at you hungrily.", TRUE, ch, 0, tmp_char, TO_VICT);
	          act("$n looks at $N and salivates.", TRUE, ch, 0, tmp_char, TO_NOTVICT);
                }
              } else {
                if(IS_SET(tmp_char->specials.new_act, NEW_PLR_KILLOK)) {
	          act("$n looks at you and shivers.", TRUE, ch, 0, tmp_char, TO_VICT);
	          act("$n backs away from $N.", TRUE, ch, 0, tmp_char, TO_NOTVICT);
                } else {
	          act("$n looks at you.", TRUE, ch, 0, tmp_char, TO_VICT);
	          act("$n looks at $N.", TRUE, ch, 0, tmp_char, TO_NOTVICT);
                }
              }
	    }
	    return;
	  }
	  /* 
	   * Search for Extra Descriptions in room and items 
	   */

	  /* Extra description in room?? */
	  if (!found) {
	    tmp_desc = find_ex_description(arg2,
			   real_roomp(ch->in_room)->ex_description);
	    if (tmp_desc) {
	      page_string(ch->desc, tmp_desc, 0);
	      return;
	    }
	  }
	  /* extra descriptions in items */

	  /* Equipment Used */
	  if (!found) {
	    for (j = 0; j < MAX_WEAR && !found; j++) {
	      if (ch->equipment[j]) {
		if (CAN_SEE_OBJ(ch, ch->equipment[j])) {
		  tmp_desc = find_ex_description(arg2,
				  ch->equipment[j]->ex_description);
		  if (tmp_desc) {
		    page_string(ch->desc, tmp_desc, 1);
		    found = TRUE;
		  }
		}
	      }
	    }
	  }
	  /* In inventory */
	  if (!found) {
	    for (tmp_object = ch->carrying;
		 tmp_object && !found;
		 tmp_object = tmp_object->next_content) {
	      if CAN_SEE_OBJ
		(ch, tmp_object) {
		tmp_desc = find_ex_description(arg2,
					tmp_object->ex_description);
		if (tmp_desc) {
		  page_string(ch->desc, tmp_desc, 1);
		  found = TRUE;
		}
		}
	    }
	  }
	  /* Object In room */

	  if (!found) {
	    for (tmp_object = real_roomp(ch->in_room)->contents;
		 tmp_object && !found;
		 tmp_object = tmp_object->next_content) {
	      if CAN_SEE_OBJ
		(ch, tmp_object) {
		tmp_desc = find_ex_description(arg2,
					tmp_object->ex_description);
		if (tmp_desc) {
		  page_string(ch->desc, tmp_desc, 1);
		  found = TRUE;
		}
		}
	    }
	  }
	  /* wrong argument */
	  if (bits) {		       /* If an object was found */
	    if (!found)
	      show_obj_to_char(found_object, ch, 5);
	    /* Show no-description */
	    else
	      show_obj_to_char(found_object, ch, 6);
	    /* Find hum, glow etc */
	  } else if (!found) {
	    cprintf(ch, "You do not see that here.\n\r");
	  }
	} else {
	  /* no argument */
	  cprintf(ch, "Look at what?\n\r");
	}
      }
      break;

      /* look ''                */
    case 8:{
	cprintf(ch, "%s\n\r", real_roomp(ch->in_room)->name);
	if (!IS_SET(ch->specials.act, PLR_BRIEF))
	  cprintf(ch, real_roomp(ch->in_room)->description);
	if (IS_PC(ch)) {
	  if (IS_SET(ch->specials.act, PLR_HUNTING)) {
	    if (ch->specials.hunting) {
	      res = track(ch, ch->specials.hunting);
	      if (!res) {
		ch->specials.hunting = 0;
		ch->hunt_dist = 0;
		REMOVE_BIT(ch->specials.act, PLR_HUNTING);
	      }
	    } else {
	      ch->hunt_dist = 0;
	      REMOVE_BIT(ch->specials.act, PLR_HUNTING);
	    }
	  }
	} else {
	  if (IS_SET(ch->specials.act, ACT_HUNTING)) {
	    if (ch->specials.hunting) {
	      res = track(ch, ch->specials.hunting);
	      if (!res) {
		ch->specials.hunting = 0;
		ch->hunt_dist = 0;
		REMOVE_BIT(ch->specials.act, ACT_HUNTING);
	      }
	    } else {
	      ch->hunt_dist = 0;
	      REMOVE_BIT(ch->specials.act, ACT_HUNTING);
	    }
	  }
	}

	list_obj_in_room(real_roomp(ch->in_room)->contents, ch);
	list_char_in_room(real_roomp(ch->in_room)->people, ch);

      }
      break;

      /* wrong arg      */
    case -1:
      cprintf(ch, "Sorry, I didn't understand that!\n\r");
      break;

      /* look 'room' */
    case 9:{

	cprintf(ch, "%s\n\r", real_roomp(ch->in_room)->name);
	cprintf(ch, real_roomp(ch->in_room)->description);

	if (!IS_NPC(ch)) {
	  if (IS_SET(ch->specials.act, PLR_HUNTING)) {
	    if (ch->specials.hunting) {
	      res = track(ch, ch->specials.hunting);
	      if (!res) {
		ch->specials.hunting = 0;
		ch->hunt_dist = 0;
		REMOVE_BIT(ch->specials.act, PLR_HUNTING);
	      }
	    } else {
	      ch->hunt_dist = 0;
	      REMOVE_BIT(ch->specials.act, PLR_HUNTING);
	    }
	  }
	} else {
	  if (IS_SET(ch->specials.act, ACT_HUNTING)) {
	    if (ch->specials.hunting) {
	      res = track(ch, ch->specials.hunting);
	      if (!res) {
		ch->specials.hunting = 0;
		ch->hunt_dist = 0;
		REMOVE_BIT(ch->specials.act, ACT_HUNTING);
	      }
	    } else {
	      ch->hunt_dist = 0;
	      REMOVE_BIT(ch->specials.act, ACT_HUNTING);
	    }
	  }
	}

	list_obj_in_room(real_roomp(ch->in_room)->contents, ch);
	list_char_in_room(real_roomp(ch->in_room)->people, ch);

      }
      break;
    }
  }
}

/* end of look */

void do_read(struct char_data *ch, char *argument, int cmd)
{
  char buf[100];

  if (DEBUG)
    dlog("do_read");
  /* This is just for now - To be changed later.! */
  sprintf(buf, "at %s", argument);
  do_look(ch, buf, 15);
}

void do_examine(struct char_data *ch, char *argument, int cmd)
{
  char name[100], buf[100];
  int bits;
  struct char_data *tmp_char;
  struct obj_data *tmp_object;

  if (DEBUG)
    dlog("do_examine");
  sprintf(buf, "at %s", argument);
  do_look(ch, buf, 15);

  one_argument(argument, name);

  if (!*name) {
    cprintf(ch, "Examine what?\n\r");
    return;
  }
  bits = generic_find(name, FIND_OBJ_INV | FIND_OBJ_ROOM |
		      FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

  if (tmp_object) {
    if ((GET_ITEM_TYPE(tmp_object) == ITEM_DRINKCON) ||
	(GET_ITEM_TYPE(tmp_object) == ITEM_CONTAINER)) {
      cprintf(ch, "When you look inside, you see:\n\r");
      sprintf(buf, "in %s", argument);
      do_look(ch, buf, 15);
    }
  }
}

void do_search(struct char_data *ch, char *argument, int cmd)
{
  int door;
  char buf[256];
  char *exits[] =
  {
    "North",
    "East ",
    "South",
    "West ",
    "Up   ",
    "Down "
  };
  struct room_direction_data *exitdata;

  *buf = '\0';
  if (DEBUG)
    dlog("do_search");
  if (IS_DARK(ch->in_room)) {
    cprintf(ch, "It is far to dark to search...\n\r");
    return;
  }
  if (GET_MANA(ch) < 30 - (GET_LEVEL(ch, BestThiefClass(ch)) / 5)) {
    cprintf(ch, "You can not think of a place to begin looking?\n\r");
    return;
  }
  GET_MANA(ch) -= (30 - (GET_LEVEL(ch, BestThiefClass(ch)) / 5));

  cprintf(ch, "You search the area...\n\r");

  for (door = 0; door < MAX_NUM_EXITS; door++) {
    exitdata = EXIT(ch, door);
    if (exitdata) {
      if (real_roomp(exitdata->to_room)) {
	if (IS_SET(exitdata->exit_info, EX_SECRET)) {
	  if (number(1, 101) < ch->skills[SKILL_SEARCH].learned) {
	    cprintf(ch, "You find a hidden passage!\n\r");
	    sprintf(buf + strlen(buf), "%s - %s", exits[door], exitdata->keyword);
	    if (IS_SET(exitdata->exit_info, EX_CLOSED))
	      strcat(buf, " (closed)");
	    strcat(buf, "\n\r");
	  }
	}
      }
    }
  }
  cprintf(ch, "Found exits:\n\r");

  if (*buf)
    cprintf(ch, buf);
  else
    cprintf(ch, "None?\n\r");
}

void do_exits(struct char_data *ch, char *argument, int cmd)
{
  int door;
  char buf[256];
  char *exits[] =
  {
    "North",
    "East ",
    "South",
    "West ",
    "Up   ",
    "Down "
  };
  struct room_direction_data *exitdata;

  *buf = '\0';
  if (DEBUG)
    dlog("do_exits");
  for (door = 0; door < MAX_NUM_EXITS; door++) {
    exitdata = EXIT(ch, door);
    if (exitdata) {
      if (!real_roomp(exitdata->to_room)) {
	/* don't print unless immortal */
	if (IS_IMMORTAL(ch)) {
	  sprintf(buf + strlen(buf), "%s - [#%d] Swirling CHAOS!\n\r",
		  exits[door], exitdata->to_room);
	}
      } else if (exitdata->to_room != NOWHERE &&
      (!IS_SET(exitdata->exit_info, EX_CLOSED) || IS_IMMORTAL(ch))) {
	if ((IS_DARK(exitdata->to_room) || IS_DARKOUT(exitdata->to_room)) && !IS_IMMORTAL(ch))
	  sprintf(buf + strlen(buf), "%s - Too dark to tell", exits[door]);
	else if(IS_IMMORTAL(ch))
	  sprintf(buf + strlen(buf), "%s - [#%d] %s", exits[door],
                  exitdata->to_room,
		  real_roomp(exitdata->to_room)->name);
        else
	  sprintf(buf + strlen(buf), "%s - %s", exits[door],
		  real_roomp(exitdata->to_room)->name);
	if (IS_SET(exitdata->exit_info, EX_CLOSED))
	  strcat(buf, " (closed)");
	if (IS_DARK(exitdata->to_room) && IS_IMMORTAL(ch))
	  strcat(buf, " (dark)");
	strcat(buf, "\n\r");
      }
    }
  }

  cprintf(ch, "Obvious exits:\n\r");

  if (*buf)
    cprintf(ch, buf);
  else
    cprintf(ch, "None.\n\r");
}

void do_score(struct char_data *ch, char *argument, int cmd)
{
  struct time_info_data playing_time;
  static char buf[1024];
  struct affected_type *aff;
  char *tmpstr;
  int algn, isneedy = 0;
  struct char_data *target;
  static char tmpbuf[256];
  static int ack;
  int j, cnt;

  target= ch;
  if (IS_IMMORTAL(ch)) {
    one_argument(argument, tmpbuf);
    if(*tmpbuf) {
      ack = 1;
      if(!(target = get_char_room_vis(ch, tmpbuf))) {
      /* if(!(target= get_char_vis_world(ch, tmpbuf, &ack))) */
        if (!(target = get_char(tmpbuf))) {
          cprintf(ch, "You can't seem to find %s.\n\r", tmpbuf);
          return;
        }
      }
    }
  }
  cprintf(ch, "%s%s%s %s %s is %d years old.\n\r",
      (IS_SET(target->specials.new_act, NEW_PLR_KILLOK)) ? "Black " : "",
      ((age(target).year == 17) && (age(target).month == 0)) ? "Newbie " : "",
	  GET_PRETITLE(target) ? GET_PRETITLE(target) : "", GET_NAME(target),
          GET_TITLE(target),
	  GET_AGE(target));

  if (GetMaxLevel(ch) > 3) {
    cprintf(ch,
	    "      +-------------------------Abilities-----------------------+\n\r");
    if (GET_ADD(target) > 99)
      cprintf(ch,
	      "      | STR: %2d/**    DEX: %2d    CON: %2d    INT: %2d    WIS: %2d  |\n\r",
	      GET_STR(target), GET_DEX(target), GET_CON(target), GET_INT(target), GET_WIS(target));
    else
      cprintf(ch,
	      "      | STR: %2d/%2d    DEX: %2d    CON: %2d    INT: %2d    WIS: %2d  |\n\r",
	      GET_STR(target), GET_ADD(target), GET_DEX(target), GET_CON(target), GET_INT(target),
	      GET_WIS(target));
    cprintf(ch,
	    "      |     AC: %3d         To-Hit: %+3d         Damage: %+3d     |\n\r",
	    (target->points.armor / 10),
	    str_app[STRENGTH_APPLY_INDEX(target)].tohit,
	    str_app[STRENGTH_APPLY_INDEX(target)].todam);
    cprintf(ch,
	    "      +---------------------------------------------------------+\n\r");
  }
  cprintf(ch, "You have %d/%d Hit Points, %d/%d Mana, and %d/%d Movement.\n\r",
	  GET_HIT(target), GET_MAX_HIT(target),
	  GET_MANA(target), GET_MAX_MANA(target),
	  GET_MOVE(target), GET_MAX_MOVE(target));
  if (MOUNTED(target)) {
    if ((GET_LEVEL(target, RANGER_LEVEL_IND) > 0) || IS_IMMORTAL(ch) ||
	IS_AFFECTED(MOUNTED(target), AFF_CHARM)) {
      cprintf(ch, "%s has %d/%d Hit Points, and %d/%d Movement.\n\r",
	      GET_SDESC(MOUNTED(target)),
	      GET_HIT(MOUNTED(target)), GET_MAX_HIT(MOUNTED(target)),
	      GET_MOVE(MOUNTED(target)), GET_MAX_MOVE(MOUNTED(target)));
    }
  }
  if (!IS_IMMORTAL(target) && (!IS_NPC(target))) {
    if (GET_COND(target, DRUNK) > 10) {
      cprintf(ch, "You are hopelessly DRUNK!\n\r");
    }
    strcpy(buf, "You are");
    if (GET_COND(target, FULL) < 6) {
      if (GET_COND(target, FULL) < 4) {
        if (GET_COND(target, FULL) < 2) {
          strcat(buf, " starving");
          isneedy = 1;
        } else {
          strcat(buf, " hungry");
          isneedy = 1;
        }
      } else {
        strcat(buf, " getting hungry");
        isneedy = 1;
      }
    }
    if (GET_COND(target, THIRST) < 6) {
      if (GET_COND(target, THIRST) < 4) {
        if (GET_COND(target, THIRST) < 2) {
          if (isneedy)
            strcat(buf, " and");
          strcat(buf, " parched");
          isneedy = 1;
        } else {
          if (isneedy)
            strcat(buf, " and");
          strcat(buf, " thirsty");
          isneedy = 1;
        }
      } else {
        if (isneedy)
          strcat(buf, " and");
        strcat(buf, " getting thirsty");
        isneedy = 1;
      }
    }
    if (isneedy)
      cprintf(ch, "%s.\n\r", buf);
  }
  switch (GET_POS(target)) {
  case POSITION_DEAD:
    cprintf(ch, "You are DEAD!  Just be quiet and lie still.\n\r");
    break;
  case POSITION_MORTALLYW:
    cprintf(ch, "You are ALMOST DEAD, be patient!\n\r");
    break;
  case POSITION_INCAP:
    cprintf(ch, "You are unconcious and waiting to die.\n\r");
    break;
  case POSITION_STUNNED:
    cprintf(ch, "At present, you are stunned, and unable to move.\n\r");
    break;
  case POSITION_SLEEPING:
    cprintf(ch, "You are sleeping peacefully.\n\r");
    break;
  case POSITION_RESTING:
    cprintf(ch, "You're resting so you can KILL MORE CRITTERS!\n\r");
    break;
  case POSITION_SITTING:
    cprintf(ch, "You are sitting around doing nothing.\n\r");
    break;
  case POSITION_FIGHTING:
    if (target->specials.fighting)
      act("You are trying to KILL $N.\n\r", FALSE, ch, 0, target->specials.fighting, TO_CHAR);
    else
      cprintf(ch, "You're fighting yourself?\n\r");
    break;
  case POSITION_STANDING:
    cprintf(ch, "You are just standing around, looking MEAN.\n\r");
    break;
  case POSITION_MOUNTED:
    if (MOUNTED(target)) {
      cprintf(ch, "You are riding %s.\n\r", GET_SDESC(MOUNTED(target)));
    } else {
      cprintf(ch, "You are just standing around, looking MEAN.\n\r");
      break;
    }
    break;
  default:
    cprintf(ch, "You seem to be floating a few inches above the ground.\n\r");
    break;
  }

  cprintf(ch, "You carry %d Gold Soverigns and have %d in the bank.\n\r",
	  GET_GOLD(target), GET_BANK(target));
  cprintf(ch, "Your home is set to: %s\n\r", real_roomp(GET_HOME(target))->name);

  sprintf(buf,
	  "      +------------------------------+------------------------------+\n\r");
  algn = MAX(-1000, MIN(GET_ALIGNMENT(target), 1000)) / 32;
  buf[37 + algn] = '*';
  cprintf(ch, buf);
  sprintf(buf,
	  "                                                                     \n\r");
  buf[37 + algn] = '^';
  cprintf(ch, buf);
  cprintf(ch,
	  "    Evil                          Neutral                          Good\n\r");

  isneedy = 0;
  strcpy(buf, "You are NOT ");
  if (IS_SET(target->specials.new_act, NEW_PLR_NOTELL)) {
    strcat(buf, "receiving tells");
    isneedy = 1;
  }
  if (IS_SET(target->specials.act, PLR_DEAF)) {
    if (isneedy)
      strcat(buf, " or");
    strcat(buf, "listening to shouts");
    isneedy = 1;
  }
  if (isneedy)
    cprintf(ch, "%s.\n\r", buf);

  playing_time = real_time_passed((time(0) - target->player.time.logon) +
				  target->player.time.played, 0);
  cprintf(ch,
	  "You have scored %d Experience Points in %d days and %d hours!\n\r",
	  GET_EXP(target), playing_time.day, playing_time.hours);

  for(j= 0; j< ABS_MAX_CLASS; j++) {
    if (GET_LEVEL(target, j) > 0) {
      if (GET_LEVEL(target, j) >= LOKI) {
        cprintf(ch, "You are the most powerful %s in all creation!\n\r",
                class_name[j]);
        continue;
      }
      cprintf(ch,
	      "As a %d%s level %s, you need %d xp and %d gold to become ",
	      GET_LEVEL(target, j), ordinal(GET_LEVEL(target, j)),
              class_name[j],
              EXP_NEEDED(target, j),
              GOLD_NEEDED(target, j));
      switch (GET_SEX(target)) {
      case SEX_MALE:
        tmpstr = titles[j][GET_LEVEL(target, j) + 1].title_m;
        break;
      case SEX_FEMALE:
        tmpstr = titles[j][GET_LEVEL(target, j) + 1].title_f;
        break;
      default:
        tmpstr = "bug";
        break;
      }
      cprintf(ch, "%s %s.\n\r", SA_AN(tmpstr), tmpstr);
    }
  }

  buf[0]= '\0';
  if (target->affected) {
    int blessflag = 0;
    cnt= 20;
    isneedy = 0;
    for (aff = target->affected; aff; aff = aff->next) {
      switch (aff->type) {
      case SKILL_SNEAK:
	break;
      case SPELL_BLESS:
        if(!blessflag) {
          blessflag= 1;
	  if (isneedy) {
	    strcat(buf, ", ");
            cnt+= 2;
          }
          if((cnt + strlen(spell_info[aff->type].name)) > 77) {
            strcat(buf, "\n\r");
            cnt= 0;
          }
	  strcat(buf, spell_info[aff->type].name);
          cnt+= strlen(spell_info[aff->type].name);
	  isneedy = 1;
        }
        break;
      default:
	if (isneedy) {
	  strcat(buf, ", ");
          cnt+= 2;
        }
        if((cnt + strlen(spell_info[aff->type].name)) > 77) {
          strcat(buf, "\n\r");
          cnt= 0;
        }
	strcat(buf, spell_info[aff->type].name);
        cnt+= strlen(spell_info[aff->type].name);
	isneedy = 1;
	break;
      }
    }
    if(isneedy)
      cprintf(ch, "You are affected by %s.\n\r", buf);
  }
}

void do_mystat(struct char_data *ch, char *argument, int cmd)
{
  if (DEBUG)
    dlog("do_mystat");
  cprintf(ch, "Use SCORE instead.\n\r");
}

void do_time(struct char_data *ch, char *argument, int cmd)
{
  struct tm *tm_info;
  time_t tc;
  char buf[100];
  int weekday, day;

  tc = time(0);
  tm_info = localtime(&tc);

  sprintf(buf, "It is %d o'clock %s, on ",
      ((time_info.hours % 12 == 0) ? 12 : ((time_info.hours) % 12)),
	  ((time_info.hours >= 12) ? "pm" : "am"));
  weekday = ((35 * time_info.month) + time_info.day + 1) % 7;	/* 35 days in a month */
  cprintf(ch, "%s%s\n\r", buf, weekdays[weekday]);

  day = time_info.day + 1;	       /* day in [1..35] */
  cprintf(ch, "This is %d%s Day of the %s, in the %d%s Year of Dread.\n\r",
	  day, ordinal(day),
	  month_name[time_info.month],
	  time_info.year, ordinal(time_info.year));

  if (tm_info->tm_hour == 0)
    if (tm_info->tm_min == 0)
      sprintf(buf, "It is midnight in the Land of RL!\n\r");
    else
      sprintf(buf, "It is 12:%2.2dam in the Land of RL.\n\r", tm_info->tm_min);
  else if (tm_info->tm_hour == 12)
    if (tm_info->tm_min == 0)
      sprintf(buf, "It is noon in the Land of RL.\n\r");
    else
      sprintf(buf, "It is 12:%2.2dpm in the Land of RL.\n\r", tm_info->tm_min);
  else if (tm_info->tm_hour > 12)
    sprintf(buf, "It is %2d:%2.2dpm in the Land of RL.\n\r",
            tm_info->tm_hour - 12, tm_info->tm_min);
  else
    sprintf(buf, "It is %2d:%2.2dam in the Land of RL.\n\r", tm_info->tm_hour,
	    tm_info->tm_min);
  cprintf(ch, "%sThe next scheduled REBOOT is at %02d:00 RLT.\n\r",
          buf,
          (((tm_info->tm_hour +1 ) < REBOOT_AT1)?REBOOT_AT1:
          (((tm_info->tm_hour +1 ) < REBOOT_AT2)?REBOOT_AT2:
           REBOOT_AT1)));
}

void do_weather(struct char_data *ch, char *argument, int cmd)
{
  char static *sky_look[4] =
  {
    "cloudless", "cloudy", "rainy", "lit by flashes of lightning"
  };
  char static *sky_words[] =
  {
    "Cloudless", "Cloudy", "Rainy", "Lightning", "None"
  };
  char static *wind_dir[] =
  {
    "North", "East", "South", "West", "Still", "None"
  };

  if (IS_IMMORTAL(ch)) {
    cprintf(ch, "Sky Condit : %s\n\r", sky_words[weather_info.sky]);
    cprintf(ch, "Wind Speed : %d\n\r", weather_info.wind_speed);
    cprintf(ch, "Wind Direc : %s\n\r", wind_dir[weather_info.wind_direction]);
    cprintf(ch, "Change     : %d\n\r", weather_info.change);
    cprintf(ch, "Pressure   : %d\n\r", weather_info.pressure);
    cprintf(ch, "Moon Phase : %d\n\r\n\r", weather_info.moon);
  }
  if (OUTSIDE(ch)) {
    cprintf(ch, "The sky is %s and %s.\n\r", sky_look[weather_info.sky],
     (weather_info.change >= 0 ? "you feel a warm wind from south" :
      "your foot tells you bad weather is due"));
  } else
    cprintf(ch, "You have no feeling about the weather at all.\n\r");
}

void do_help(struct char_data *ch, char *argument, int cmd)
{
  int chk, bot, top, mid, minlen;
  char buf[MAX_STRING_LENGTH], buffer[MAX_STRING_LENGTH];

  if (DEBUG)
    dlog("do_help");
  if (!ch->desc)
    return;

  for (; isspace(*argument); argument++);

  if (*argument) {
    if (!help_index) {
      cprintf(ch, "No help available.\n\r");
      return;
    }
    bot = 0;
    top = top_of_helpt;

    for (;;) {
      mid = (bot + top) / 2;
      minlen = strlen(argument);

      if (!(chk = strn_cmp(argument, help_index[mid].keyword, minlen))) {
	fseek(help_fl, help_index[mid].pos, 0);
	*buffer = '\0';
	for (;;) {
	  fgets(buf, 80, help_fl);
	  if (*buf == '#')
	    break;
	  strcat(buffer, buf);
	  strcat(buffer, "\r");
	}
	page_string(ch->desc, buffer, 1);
	return;
      } else if (bot >= top) {
	cprintf(ch, "There is no help on that word.\n\r");
	return;
      } else if (chk > 0)
	bot = ++mid;
      else
	top = --mid;
    }
    return;
  }
  page_string(ch->desc, help, 0);
  /* cprintf(ch, help); */
}

void do_wizhelp(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  int no, i;

  /* First command is command[0]           */
  /* cmd_info[1] ~~ commando[0]            */
  if (DEBUG)
    dlog("do_wizhelp");
  if (IS_NPC(ch))
    return;

  cprintf(ch, "The following privileged comands are available:\n\r\n\r");

  *buf = '\0';

  for (no = 1, i = 0; *command[i] != '\n'; i++)
    if ((GetMaxLevel(ch) >= cmd_info[i + 1].minimum_level) &&
	(cmd_info[i + 1].minimum_level >= LOW_IMMORTAL)) {

      sprintf(buf + strlen(buf), "%-10s", command[i]);
      if (!(no % 7))
	strcat(buf, "\n\r");
      no++;
    }
  strcat(buf, "\n\r");
  page_string(ch->desc, buf, 1);
}

void do_allcommands(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  int no, i;

  if (DEBUG)
    dlog("do_allcommands");
  if (IS_NPC(ch))
    return;
  cprintf(ch, "The following comands are available:\n\r\n\r");
  *buf = '\0';
  for (no = 1, i = 0; *command[i] != '\n'; i++)
    if ((GetMaxLevel(ch) >= cmd_info[i + 1].minimum_level) &&
	(cmd_info[i + 1].minimum_level < LOW_IMMORTAL)) {
      sprintf(buf + strlen(buf), "%-10s", command[i]);
      if (!(no % 7))
	strcat(buf, "\n\r");
      no++;
    }
  strcat(buf, "\n\r");
  page_string(ch->desc, buf, 1);
}

void do_who(struct char_data *ch, char *argument, int cmd)
{
  struct descriptor_data *d;
  char buf[256];
  int count;
  struct char_data *person;
  long ttime;
  long thour, tmin, tsec;
  /* long ct, ot; */
  time_t ct, ot;
  char *tmstr, *otmstr;
  extern long Uptime;

  if (DEBUG)
    dlog("do_who");
/*
 * if (IS_NPC(ch))
 *   return;
 */

/*
 * sprintf(buf + strlen(buf), "\n\r*** Active players on %s ***\n\r\n\r", MUDNAME);
 * cprintf(ch, buf);
 */
  cprintf(ch, "%s\n\r", VERSION_STR);

  count = 0;
  for (d = descriptor_list; d; d = d->next) {
    person = d->character;

    if (!person || !person->desc)
      continue;
    if (!IS_PC(person))
      continue;
    if (d->connected || !CAN_SEE(ch, person))
      continue;
    if (cmd == 234 &&
	real_roomp(person->in_room)->zone != real_roomp(ch->in_room)->zone)
      continue;
    count++;
    bzero(buf, 256);

    if (IS_SET(SHOW_IDLE, whod_mode)) {
      if(!(person->desc)) {
        strcat(buf, "linkdead ");
      } else {
        ttime = GET_IDLE_TIME(person);
        thour = ttime / 3600;
        ttime -= thour * 3600;
        tmin = ttime / 60;
        ttime -= tmin * 60;
        tsec = ttime;
        if(!thour && !tmin && (tsec <= 15))
          strcat(buf, " playing ");
        else
          sprintf(buf + strlen(buf), "%02ld:%02ld:%02ld ", thour, tmin, tsec);
      }
    }
    if (IS_SET(SHOW_LEVEL, whod_mode)) {
      if (GetMaxLevel(person) >= WIZ_MAX_LEVEL)
	sprintf(buf + strlen(buf), "[ God ] ");
      else if (GetMaxLevel(person) == WIZ_MAX_LEVEL-1)
	sprintf(buf + strlen(buf), "[Power] ");
      else if (GetMaxLevel(person) >= WIZ_MIN_LEVEL)
	sprintf(buf + strlen(buf), "[Whizz] ");
      else
	sprintf(buf + strlen(buf), "[ %3d ] ", GetMaxLevel(person));
    }
    if (IS_SET(SHOW_TITLE, whod_mode))
      if (GET_PRETITLE(person))
	sprintf(buf + strlen(buf), "%s ", GET_PRETITLE(person));

    if (IS_SET(SHOW_NAME, whod_mode))
      sprintf(buf + strlen(buf), "%s ", GET_NAME(person));

    if (IS_SET(SHOW_TITLE, whod_mode))
      sprintf(buf + strlen(buf), "%s ", GET_TITLE(person));

    if (IS_SET(SHOW_ROOM, whod_mode)) {
      sprintf(buf + strlen(buf), "- %s ",
	      real_roomp(person->in_room)->name);
      if (GetMaxLevel(ch) >= LOW_IMMORTAL)
	sprintf(buf + strlen(buf), "[#%d]", person->in_room);
    }
    if (IS_SET(SHOW_SITE, whod_mode)) {
      if (person->desc->host != NULL)
	sprintf(buf + strlen(buf), "(%s)", person->desc->host);
    }
    strcat(buf, "\n\r");
    cprintf(ch, buf);
  }
  sprintf(buf, "\n\rTotal visible players on %s: %d\n\r", MUDNAME, count);
  ot = Uptime;
  otmstr = asctime(localtime(&ot));
  *(otmstr + strlen(otmstr) - 1) = '\0';
  sprintf(buf + strlen(buf), START_TIME, otmstr);

  ct = time(0);
  tmstr = asctime(localtime(&ct));
  *(tmstr + strlen(tmstr) - 1) = '\0';
  sprintf(buf + strlen(buf), GAME_TIME, tmstr);
  cprintf(ch, buf);
}

void do_users(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH], line[200];
  struct descriptor_data *d;
  int flag;

  if (DEBUG)
    dlog("do_users");
  strcpy(buf, "Connections:\n\r------------\n\r");

  for (d = descriptor_list; d; d = d->next) {
    flag = 0;
    if (d->character && d->character->player.name) {
      if (GetMaxLevel(ch) > d->character->invis_level)
	flag = 1;

      if (flag) {
	if (d->original)
	  sprintf(line, "%-16s: ", d->original->player.name);
	else
	  sprintf(line, "%-16s: ", d->character->player.name);
      }
    } else {
      strcpy(line, connected_types[d->connected]);
      strcat(line, "\n\r");
    }

    if (flag) {
      if (d->host)
	sprintf(line + strlen(line), "[%s@%s]\n\r", d->username, d->host);
      else
	strcat(line, "[Hostname unknown]\n\r");
    }
    if (flag)
      strcat(buf, line);
  }
  strcat(buf, "\n\r");
  cprintf(ch, buf);
}

void do_inventory(struct char_data *ch, char *argument, int cmd)
{
  if (DEBUG)
    dlog("do_inventory");
  cprintf(ch, "You are carrying:\n\r");
  list_obj_on_char(ch->carrying, ch);
}

void do_equipment(struct char_data *ch, char *argument, int cmd)
{
  int j, Worn_Index;
  BYTE found;

  if (DEBUG)
    dlog("do_equipment");
  cprintf(ch, "Equipment in use:\n\r");
  found = FALSE;
  for (Worn_Index = j = 0; j < MAX_WEAR; j++) {
    if (ch->equipment[j]) {
      Worn_Index++;
      cprintf(ch, "%s", where[j]);
      if (CAN_SEE_OBJ(ch, ch->equipment[j])) {
	show_obj_to_char(ch->equipment[j], ch, 1);
	found = TRUE;
      } else {
	cprintf(ch, "Something.\n\r");
	found = TRUE;
      }
    }
  }
  if (!found) {
    cprintf(ch, " Nothing.\n\r");
  }
}

void do_credits(struct char_data *ch, char *argument, int cmd)
{
  if (DEBUG)
    dlog("do_credits");
  page_string(ch->desc, credits, 0);
}

void do_news(struct char_data *ch, char *argument, int cmd)
{
  if (DEBUG)
    dlog("do_news");
  page_string(ch->desc, news, 0);
}

void do_info(struct char_data *ch, char *argument, int cmd)
{
  if (DEBUG)
    dlog("do_info");
  page_string(ch->desc, info, 0);
}

void do_wizlist(struct char_data *ch, char *argument, int cmd)
{
  if (DEBUG)
    dlog("do_wizlist");
  page_string(ch->desc, wizlist, 0);
}

static int which_number_mobile(struct char_data *ch, struct char_data *mob)
{
  struct char_data *i;
  char *name;
  int number;

  if (DEBUG)
    dlog("which_number_mobile");
  name = fname(mob->player.name);
  for (i = character_list, number = 0; i; i = i->next) {
    if (isname(name, i->player.name) && i->in_room != NOWHERE) {
      number++;
      if (i == mob)
	return number;
    }
  }
  return 0;
}

char *numbered_person(struct char_data *ch, struct char_data *person)
{
  static char buf[MAX_STRING_LENGTH];

  if (DEBUG)
    dlog("numbered_person");
  if (IS_NPC(person) && IS_IMMORTAL(ch)) {
    sprintf(buf, "%d.%s", which_number_mobile(ch, person),
	    fname(person->player.name));
  } else {
    strcpy(buf, PERS(person, ch));
  }
  return buf;
}

static void do_where_person(struct char_data *ch, struct char_data *person, struct string_block *sb)
{
  char buf[MAX_STRING_LENGTH];

  if (DEBUG)
    dlog("do_where_person");
  sprintf(buf, "%-30s- %s ", PERS(person, ch),
	  (person->in_room > -1 ? real_roomp(person->in_room)->name : "Nowhere"));

  if (GetMaxLevel(ch) >= LOW_IMMORTAL)
    sprintf(buf + strlen(buf), "[%d]", person->in_room);

  strcpy(buf + strlen(buf), "\n\r");

  append_to_string_block(sb, buf);
}

static void do_where_object(struct char_data *ch, struct obj_data *obj,
			    int recurse, struct string_block *sb)
{
  char buf[MAX_STRING_LENGTH];

  if (DEBUG)
    dlog("do_where_object");

  if (obj->in_room != NOWHERE) {       /* object in a room */
    sprintf(buf, "%-30s- %s [%d]\n\r",
	    obj->short_description,
	    real_roomp(obj->in_room)->name,
	    obj->in_room);
  } else if (obj->carried_by != NULL) {		/* object carried by monster */
    sprintf(buf, "%-30s- carried by %s\n\r",
	    obj->short_description,
	    numbered_person(ch, obj->carried_by));
  } else if (obj->equipped_by != NULL) {	/* object equipped by monster */
    sprintf(buf, "%-30s- equipped by %s\n\r",
	    obj->short_description,
	    numbered_person(ch, obj->equipped_by));
  } else if (obj->in_obj) {	       /* object in object */
    sprintf(buf, "%-30s- in %s\n\r",
	    obj->short_description,
	    obj->in_obj->short_description);
  } else {
    sprintf(buf, "%-30s- can't find it? \n\r",
	    obj->short_description);
  }
  if (*buf)
    append_to_string_block(sb, buf);

  if (recurse) {
    if (obj->in_room != NOWHERE)
      return;
    else if (obj->carried_by != NULL)
      do_where_person(ch, obj->carried_by, sb);
    else if (obj->equipped_by != NULL)
      do_where_person(ch, obj->equipped_by, sb);
    else if (obj->in_obj != NULL)
      do_where_object(ch, obj->in_obj, TRUE, sb);
  }
}

void do_where(struct char_data *ch, char *argument, int cmd)
{
  char name[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  char *nameonly;
  register struct char_data *i;
  register struct obj_data *k;
  struct descriptor_data *d;
  int number, count;
  struct string_block sb;

  if (DEBUG)
    dlog("do_where");
  only_argument(argument, name);

  if (!*name) {
    if (GetMaxLevel(ch) < LOW_IMMORTAL) {
      cprintf(ch, "What are you looking for?\n\r");
      return;
    } else {
      init_string_block(&sb);
      append_to_string_block(&sb, "Players:\n\r--------\n\r");

      for (d = descriptor_list; d; d = d->next) {
	if (d->character &&
	    (d->connected == CON_PLAYING) &&
	    (d->character->in_room != NOWHERE) &&
	    (GetMaxLevel(ch) > d->character->invis_level)) {
	  if (d->original)	       /* If switched */
	    sprintf(buf, "%-20s - %s [%d] In body of %s\n\r",
		    d->original->player.name,
		    real_roomp(d->character->in_room)->name,
		    d->character->in_room,
		    fname(d->character->player.name));
	  else
	    sprintf(buf, "%-20s - %s [%d]\n\r",
		    d->character->player.name,
		    real_roomp(d->character->in_room)->name,
		    d->character->in_room);

	  append_to_string_block(&sb, buf);
	}
      }
      page_string_block(&sb, ch);
      destroy_string_block(&sb);
      return;
    }
  }
  if (isdigit(*name)) {
    nameonly = name;
    count = number = get_number(&nameonly);
  } else {
    count = number = 0;
  }

  *buf = '\0';

  init_string_block(&sb);

  for (i = character_list; i; i = i->next)
    if (isname(name, i->player.name) && CAN_SEE(ch, i)) {
      if ((i->in_room != NOWHERE) &&
	  ((GetMaxLevel(ch) >= LOW_IMMORTAL) || (real_roomp(i->in_room)->zone ==
				  real_roomp(ch->in_room)->zone))) {
	if (number == 0 || (--count) == 0) {
	  if (number == 0) {
	    sprintf(buf, "[%2d] ", ++count);	/* I love short circuiting :) */
	    append_to_string_block(&sb, buf);
	  }
	  do_where_person(ch, i, &sb);
	  *buf = 1;
	  if (number != 0)
	    break;
	}
	if (GetMaxLevel(ch) < LOW_IMMORTAL)
	  break;
      }
    }
  /*  count = number; */

  if (GetMaxLevel(ch) >= LOW_IMMORTAL) {
    for (k = object_list; k; k = k->next)
      if (isname(name, k->name) && CAN_SEE_OBJ(ch, k)) {
	if (number == 0 || (--count) == 0) {
	  if (number == 0) {
	    sprintf(buf, "[%2d] ", ++count);
	    append_to_string_block(&sb, buf);
	  }
	  do_where_object(ch, k, number != 0, &sb);
	  *buf = 1;
	  if (number != 0)
	    break;
	}
      }
  }
  if (!*sb.data)
    cprintf(ch, "Couldn't find any such thing.\n\r");
  else
    page_string_block(&sb, ch);
  destroy_string_block(&sb);
}

void do_levels(struct char_data *ch, char *argument, int cmd)
{
  int i, RaceMax, class;
  char buf[MAX_STRING_LENGTH];

  if (DEBUG)
    dlog("do_levels");
  if (IS_NPC(ch)) {
    return;
  }
  *buf = '\0';
/*
 * **  get the class
 */

  for (; isspace(*argument); argument++);

  if (!*argument) {
    cprintf(ch, "You must supply a class!\n\r");
    return;
  }
  switch (*argument) {
  case 'C':
  case 'c':
  case 'P':
  case 'p':
    class = CLERIC_LEVEL_IND;
    break;
  case 'F':
  case 'f':
  case 'W':
  case 'w':
    class = WARRIOR_LEVEL_IND;
    break;
  case 'M':
  case 'm':
    class = MAGE_LEVEL_IND;
    break;
  case 'T':
  case 't':
    class = THIEF_LEVEL_IND;
    break;
  case 'R':
  case 'r':
    class = RANGER_LEVEL_IND;
    break;
  case 'D':
  case 'd':
    class = DRUID_LEVEL_IND;
    break;
  default:
    cprintf(ch, "I don't recognize %s\n\r", argument);
    return;
    break;
  }

  RaceMax = RacialMax[RACE_HUMAN][class];

  for (i = 1; i <= RaceMax; i++) {
    sprintf(buf + strlen(buf), "[%2d] %9d-%-9d : ", i,
	    titles[class][i].exp,
	    titles[class][i + 1].exp);

    switch (GET_SEX(ch)) {
    case SEX_MALE:
      strcat(buf, titles[class][i].title_m);
      break;
    case SEX_FEMALE:
      strcat(buf, titles[class][i].title_f);
      break;
    default:
      cprintf(ch, "Uh oh.\n\r");
      break;
    }
    strcat(buf, "\n\r");
  }
  page_string(ch->desc, buf, 1);
}

void do_consider(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char name[256];
  int diff;

  if (DEBUG)
    dlog("do_consider");
  only_argument(argument, name);

  if (!(victim = get_char_room_vis(ch, name))) {
    cprintf(ch, "Consider killing who?\n\r");
    return;
  }
  if (victim == ch) {
    cprintf(ch, "You think it would be ridiculously easy to kill yourself.\n\r"
            "Just type quit, and then pick option 6.\n\r");
    return;
  }
  if (!IS_NPC(victim)) {
    if (!IS_SET(ch->specials.new_act, NEW_PLR_KILLOK)) {
      cprintf(ch, "You can't bear the thought of killing another player!\n\r");
      cprintf(victim, "%s thinks about you and quakes in terror!\n\r",
              NAME(ch));
      return;
    } else {
      if(!IS_SET(victim->specials.new_act, NEW_PLR_KILLOK)) {
        cprintf(ch, "You wish you could cut down %s, but the fool does not serve the Dread Lord.\n\r", NAME(victim));
        cprintf(victim, "%s seems interested in you for some reason.\n\r",
                NAME(ch));
        return;
      }
    }
  }
  diff=(GetMaxLevel(victim)+(GetSecMaxLev(victim)/2)+(GetThirdMaxLev(victim)/3))
      -(GetMaxLevel(ch)+(GetSecMaxLev(ch)/2)+(GetThirdMaxLev(ch)/3))
      +fuzz(MAX(1,GetMaxLevel(ch)/5));
  if (diff <= -50) {
    cprintf(ch, "You looked at them.  Why aren't they dead?\n\r");
  } else if (diff <= -30) {
    cprintf(ch, "Why bother?  It would be a waste of 5 seconds.\n\r");
  } else if (diff <= -20) {
    cprintf(ch, "You would have to clean your weapons.\n\r");
  } else if (diff <= -15) {
    cprintf(ch, "Easy?  Understatement of the day there...\n\r");
  } else if (diff <= -10) {
    cprintf(ch, "Too easy to be believed.\n\r");
  } else if (diff <= -5) {
    cprintf(ch, "Not a problem.\n\r");
  } else if (diff <= -3) {
    cprintf(ch, "Rather easy.\n\r");
  } else if (diff <= -2) {
    cprintf(ch, "Easy.\n\r");
  } else if (diff <= -1) {
    cprintf(ch, "Looks easy.\n\r");
  } else if (diff == 0) {
    cprintf(ch, "The perfect match!\n\r");
  } else if (diff <= 1) {
    cprintf(ch, "You would need some luck!\n\r");
  } else if (diff <= 2) {
    cprintf(ch, "You would need a lot of luck!\n\r");
  } else if (diff <= 3) {
    cprintf(ch, "You would need a lot of luck and great equipment!\n\r");
  } else if (diff <= 5) {
    cprintf(ch, "Do you feel lucky, punk?\n\r");
  } else if (diff <= 10) {
    cprintf(ch, "Who do you think YOU'RE looking at, WIMP?\n\r");
  } else if (diff <= 15) {
    cprintf(ch, "Are you crazy?\n\r");
  } else if (diff <= 20) {
    cprintf(ch, "Think about this for a moment.\n\r");
  } else if (diff <= 30) {
    cprintf(ch, "You ARE mad!\n\r");
  } else {
    cprintf(ch, "Just tell me where to send the flowers.\n\r");
  }
}

void do_spells(struct char_data *ch, char *argument, int cmd)
{
  int i;
  char buf[16384];

  if (DEBUG)
    dlog("do_spells");
  if (IS_NPC(ch))
    return;
  *buf = 0;
  sprintf(buf, "Spell Name                Ma Cl Wa Th Ra Dr\n\r");
  for (i = 1; i < MAX_SKILLS; i++) {
    if(!spell_info[i].castable) continue;
    sprintf(buf + strlen(buf), "[%3d] %-20s  %2d %2d %2d %2d %2d %2d\n\r",
	    i, spell_info[i].name,
	    spell_info[i].min_level[MAGE_LEVEL_IND],
	    spell_info[i].min_level[CLERIC_LEVEL_IND],
	    spell_info[i].min_level[WARRIOR_LEVEL_IND],
	    spell_info[i].min_level[THIEF_LEVEL_IND],
	    spell_info[i].min_level[RANGER_LEVEL_IND],
	    spell_info[i].min_level[DRUID_LEVEL_IND]);
  }
  strcat(buf, "\n\r");
  page_string(ch->desc, buf, 1);
}

void do_world(struct char_data *ch, char *argument, int cmd)
{
  /* long ct, ot; */
  time_t ct, ot;
  char *tmstr, *otmstr;

  if (DEBUG)
    dlog("do_world");
  ot = Uptime;
  otmstr = asctime(localtime(&ot));
  *(otmstr + strlen(otmstr) - 1) = '\0';
  cprintf(ch, "Wiley start time was: %s\n\r", otmstr);

  ct = time(0);
  tmstr = asctime(localtime(&ct));
  *(tmstr + strlen(tmstr) - 1) = '\0';
  cprintf(ch, "Wiley's time is: %s\n\r", tmstr);
  cprintf(ch, "Total number of rooms: %d\n\r", room_db.klistlen);
  cprintf(ch, "Total number of objects: %d\n\r", top_of_objt + 1);
  cprintf(ch, "Total number of mobiles: %d\n\r", top_of_mobt + 1);
  cprintf(ch, "Total number of players: %d\n\r", number_of_players);
}

void do_skills(struct char_data *ch, int cmd, char *arg)
{
  int i;

  struct skill_struct {
    char skill_name[40];
    int skill_numb;
    int skill_class;
    int skill_lvl;
  };

  struct skill_struct r_skills[] =
  {
    {"swimming", SKILL_SWIMMING, CLASS_ALL, 1},
    {"bandage", SKILL_BANDAGE, CLASS_ALL, 1},
    {"track", SKILL_TRACK, CLASS_WARRIOR | CLASS_MAGIC_USER | CLASS_CLERIC, 3},
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

  cprintf(ch, "You have %d practice sessions left.\n\r", ch->specials.pracs);
  for (i = 0; r_skills[i].skill_name[0] != '\n'; i++) {
    if (r_skills[i].skill_lvl <= GetMaxLevel(ch) || IS_IMMORTAL(ch)) {
      if ((IS_SET(ch->player.class, r_skills[i].skill_class) &&
      GetMaxLevel(ch) > r_skills[i].skill_lvl) || IS_IMMORTAL(ch)) {
	cprintf(ch, "%s%s\n\r", r_skills[i].skill_name,
	  how_good(ch->skills[r_skills[i].skill_numb].learned));
      }
    }
  }
}

void do_players(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  int i;

  if (DEBUG)
    dlog("do_players");

  cprintf(ch, "Player List for WileyMUD III\n\r\n\r");
  bzero(buf, MAX_STRING_LENGTH);
  for(i= 0; i< number_of_players; i++) {
    if(!list_of_players[i]) continue;
    if(strlen(buf)+ strlen(list_of_players[i] + 2) >= MAX_STRING_LENGTH) {
      page_string(ch->desc, buf, 1);
      bzero(buf, MAX_STRING_LENGTH);
    }
    sprintf(buf+ strlen(buf), "%s\r", list_of_players[i]);
  }
  if(strlen(buf))
    page_string(ch->desc, buf, 1);
}

void do_ticks(struct char_data *ch, char *argument, int cmd)
{
  struct tm *tm_info;
  time_t tc;

  tc = time(0);
  tm_info = localtime(&tc);

  cprintf(ch, "Pulse Counter: %d\n\r", pulse);
  cprintf(ch, "  NEXT TICK:\t\t%1.2lf\n\r",
          pulse_update/(double)PULSE_PER_SECOND);
  cprintf(ch, "  Next Zone tick:\t%1.2lf\n\r",
          pulse_zone/(double)PULSE_PER_SECOND);
  cprintf(ch, "  Next Teleport tick:\t%1.2lf\n\r",
          pulse_teleport/(double)PULSE_PER_SECOND);
  cprintf(ch, "  Next Nature tick:\t%1.2lf\n\r",
          pulse_nature/(double)PULSE_PER_SECOND);
  cprintf(ch, "  Next Violence tick:\t%1.2lf\n\r",
          pulse_violence/(double)PULSE_PER_SECOND);
  cprintf(ch, "  Next River tick:\t%1.2lf\n\r",
          pulse_river/(double)PULSE_PER_SECOND);
  cprintf(ch, "  Next Sound tick:\t%1.2lf\n\r",
          pulse_sound/(double)PULSE_PER_SECOND);
  cprintf(ch, "  Next Reboot tick:\t%1.2lf\n\r",
          pulse_reboot/(double)PULSE_PER_SECOND);
  cprintf(ch, "  Next Dump tick:\t%1.2lf\n\r",
          pulse_dump/(double)PULSE_PER_SECOND);
  cprintf(ch, "  Next REBOOT at:\t%02d:00\n\r",
          (((tm_info->tm_hour +1 ) < REBOOT_AT1)?REBOOT_AT1:
          (((tm_info->tm_hour +1 ) < REBOOT_AT2)?REBOOT_AT2:
           REBOOT_AT1)));
}

void do_map(struct char_data *ch, char *argument, int cmd)
{
  char *template =
"\n\r"
"    U                      +----------------------+\n\r"
"    | N                    | %-20.20s |\n\r"
"    |/                     | %-6.6s  %12.12s |\n\r"
"W---/---E                  +-----------+----------+\n\r"
"   /|                                  |\n\r"
"  S |                            ______+________________,\n\r"
"    D                           / %-20.20s /\n\r"
"                               / %-6.6s  %12.12s /\n\r"
"                              /______________________/\n\r"
"                                       | /              _______________________,\n\r"
"   _______________________,            |/              / %-20.20s /\n\r"
"  / %-20.20s /_____________/______________/ %-6.6s  %12.12s /\n\r"
" / %-6.6s  %12.12s /             /|             /______________________/\n\r"
"/______________________/             / |\n\r"
"                         ___________/__+________,\n\r"
"                        / %-20.20s /\n\r"
"                       / %-6.6s  %12.12s /\n\r"
"                      /______________________/\n\r"
"                                       |\n\r"
"                           +-----------+----------+\n\r"
"                           | %-20.20s |\n\r"
"                           | %-6.6s  %12.12s |\n\r"
"                           +----------------------+\n\r";
  int door;
  struct room_direction_data *exitdata;
  char name[MAX_NUM_EXITS][21];
  char vnum[MAX_NUM_EXITS][7];
  char terrain[MAX_NUM_EXITS][13];

  if (DEBUG)
    dlog("do_map");

  for (door = 0; door < MAX_NUM_EXITS; door++) {
    bzero(name[door], 21);
    bzero(vnum[door], 7);
    bzero(terrain[door], 13);
    exitdata = EXIT(ch, door);
    if (exitdata) {
      if (!real_roomp(exitdata->to_room)) {
	if (IS_IMMORTAL(ch)) {
          sprintf(name[door], "Swirling CHAOS!");
          sprintf(vnum[door], "#%-5.5d", exitdata->to_room);
	}
      } else if (exitdata->to_room != NOWHERE) {
        if(IS_IMMORTAL(ch)) {
          sprintf(name[door], "%c%c%c%c %-15.15s",
                  (IS_DARK(exitdata->to_room)||IS_DARKOUT(exitdata->to_room))?
                  'D':' ',
                  (IS_SET(exitdata->exit_info, EX_SECRET))?'S':' ',
                  (IS_SET(exitdata->exit_info, EX_CLOSED))?'C':' ',
                  (IS_SET(exitdata->exit_info, EX_LOCKED))?'L':' ',
                  real_roomp(exitdata->to_room)->name);
          sprintf(vnum[door], "#%-5.5d", exitdata->to_room);
          sprintf(terrain[door],
                  sector_types[real_roomp(exitdata->to_room)->sector_type]);
        } else {
          if (!IS_SET(exitdata->exit_info, EX_SECRET)) {
            if (IS_SET(exitdata->exit_info, EX_CLOSED)) {
              sprintf(name[door], "A Closed Door");
            } else {
	      if (IS_DARK(exitdata->to_room) || IS_DARKOUT(exitdata->to_room)) {
                sprintf(name[door], "Darkness");
              } else {
                sprintf(name[door], "%-20.20s",
                  real_roomp(exitdata->to_room)->name);
                sprintf(terrain[door],
                  sector_types[real_roomp(exitdata->to_room)->sector_type]);
              }
            }
          }
        }
      }
    }
  }
  cprintf(ch, template, name[UP], vnum[UP], terrain[UP],
          name[NORTH], vnum[NORTH], terrain[NORTH],
          name[EAST], name[WEST], vnum[EAST], terrain[EAST],
          vnum[WEST], terrain[WEST], name[SOUTH], vnum[SOUTH],
          terrain[SOUTH], name[DOWN], vnum[DOWN], terrain[DOWN]);
}

/*
 * file: act.obj1.c , Implementation of commands.         Part of DIKUMUD
 * Usage : Commands mainly moving around objects.
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <assert.h>

#include "global.h"
#include "bug.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "trap.h"
#include "constants.h"
#include "spell_parser.h"
#define _ACT_OBJ_C
#include "act_obj.h"

/* procedures related to get */
void get(struct char_data *ch, struct obj_data *obj_object,
	 struct obj_data *sub_object)
{
  char buffer[256];

  if (DEBUG)
    dlog("get");
  if (sub_object) {
    if (!IS_SET(sub_object->obj_flags.value[1], CONT_CLOSED)) {
      obj_from_obj(obj_object);
      obj_to_char(obj_object, ch);
      act("You get $p from $P.", 0, ch, obj_object, sub_object, TO_CHAR);
      act("$n gets $p from $P.", 1, ch, obj_object, sub_object, TO_ROOM);
    } else {
      act("$P must be opened first.", 1, ch, 0, sub_object, TO_CHAR);
      return;
    }
  } else {
/*  jdb -- 11-9 */
    if (obj_object->in_room == NOWHERE) {
      obj_object->in_room = ch->in_room;
      sprintf(buffer, "OBJ:%s got %s from room -1...", GET_NAME(ch), obj_object->name);
      log(buffer);
    }
    obj_from_room(obj_object);
    obj_to_char(obj_object, ch);
    act("You get $p.", 0, ch, obj_object, 0, TO_CHAR);
    act("$n gets $p.", 1, ch, obj_object, 0, TO_ROOM);
  }
  if ((obj_object->obj_flags.type_flag == ITEM_MONEY) &&
      (obj_object->obj_flags.value[0] >= 1)) {
    obj_from_char(obj_object);
    sprintf(buffer, "There was %d coins.\n\r", obj_object->obj_flags.value[0]);
    send_to_char(buffer, ch);
    GET_GOLD(ch) += obj_object->obj_flags.value[0];
    extract_obj(obj_object);
  }
}

void do_get(struct char_data *ch, char *argument, int cmd)
{
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char buffer[MAX_STRING_LENGTH];
  struct obj_data *sub_object;
  struct obj_data *obj_object;
  struct obj_data *next_obj;
  BYTE found = FALSE;
  BYTE fail = FALSE;
  int type = 3;
  char newarg[100];
  int num, p;

  if (DEBUG)
    dlog("do_get");
  argument_interpreter(argument, arg1, arg2);

  /* get type */
  if (!*arg1)
    type = 0;

  if (*arg1 && !*arg2) {
    if (!str_cmp(arg1, "all"))
      type = 1;
    else
      type = 2;
  }
  if (*arg1 && *arg2) {
    if (!str_cmp(arg1, "all")) {
      if (!str_cmp(arg2, "all"))
	type = 3;
      else
	type = 4;
    } else {
      if (!str_cmp(arg2, "all"))
	type = 5;
      else
	type = 6;
    }
  }
  switch (type) {
    /* get */
  case 0:{
      send_to_char("Get what?\n\r", ch);
    }
    break;
    /* get all */
  case 1:{
      sub_object = 0;
      found = FALSE;
      fail = FALSE;
      for (obj_object = real_roomp(ch->in_room)->contents; obj_object; obj_object = next_obj) {
	next_obj = obj_object->next_content;
/*
 * check for a trap (traps fire often)
 */
	if (CheckForAnyTrap(ch, obj_object))
	  return;

	if (CAN_SEE_OBJ(ch, obj_object)) {
	  if ((IS_CARRYING_N(ch) + 1) <= CAN_CARRY_N(ch)) {
	    if ((IS_CARRYING_W(ch) + obj_object->obj_flags.weight) <=
		CAN_CARRY_W(ch)) {
	      if (CAN_WEAR(obj_object, ITEM_TAKE)) {
		get(ch, obj_object, sub_object);
		found = TRUE;
	      } else {
		send_to_char("You can't take that\n\r", ch);
		fail = TRUE;
	      }
	    } else {
	      sprintf(buffer, "%s : You can't carry that much weight.\n\r",
		      obj_object->short_description);
	      send_to_char(buffer, ch);
	      fail = TRUE;
	    }
	  } else {
	    sprintf(buffer, "%s : You can't carry that many items.\n\r", obj_object->short_description);
	    send_to_char(buffer, ch);
	    fail = TRUE;
	  }
	}
      }
      if (found) {
	send_to_char("OK.\n\r", ch);
      } else {
	if (!fail)
	  send_to_char("You see nothing here.\n\r", ch);
      }
    }
    break;
    /* get ??? (something) */
  case 2:
    {
      sub_object = 0;
      found = FALSE;
      fail = FALSE;

      if (getall(arg1, newarg) != FALSE) {
	strcpy(arg1, newarg);
	num = -1;
      } else if ((p = getabunch(arg1, newarg)) != FALSE) {
	strcpy(arg1, newarg);
	num = p;
      } else {
	num = 1;
      }

      while (num != 0) {
	obj_object =
	  get_obj_in_list_vis(ch, arg1, real_roomp(ch->in_room)->contents);
	if (obj_object) {
	  if (CheckForAnyTrap(ch, obj_object))
	    return;

	  if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
	    if ((IS_CARRYING_W(ch) + obj_object->obj_flags.weight) <
		CAN_CARRY_W(ch)) {
	      if (CAN_WEAR(obj_object, ITEM_TAKE)) {
		get(ch, obj_object, sub_object);
		found = TRUE;
	      } else {
		send_to_char("You can't take that\n\r", ch);
		fail = TRUE;
		num = 0;
	      }
	    } else {
	      sprintf(buffer, "%s : You can't carry that much weight.\n\r",
		      obj_object->short_description);
	      send_to_char(buffer, ch);
	      fail = TRUE;
	      num = 0;
	    }
	  } else {
	    sprintf(buffer, "%s : You can't carry that many items.\n\r",
		    obj_object->short_description);
	    send_to_char(buffer, ch);
	    fail = TRUE;
	    num = 0;
	  }
	} else {
	  if (num > 0) {
	    sprintf(buffer, "You do not see a %s here.\n\r", arg1);
	    send_to_char(buffer, ch);
	  }
	  num = 0;
	  fail = TRUE;
	}
	if (num > 0)
	  num--;
      }
    }
    break;
    /* get all all */
  case 3:
    {
      send_to_char("You must be joking?!\n\r", ch);
    }
    break;
    /* get all ??? */
  case 4:
    {
      found = FALSE;
      fail = FALSE;
      sub_object = (struct obj_data *)get_obj_vis_accessible(ch, arg2);
      if (sub_object) {
	if (GET_ITEM_TYPE(sub_object) == ITEM_CONTAINER) {
	  for (obj_object = sub_object->contains; obj_object; obj_object = next_obj) {
	    if (CheckForGetTrap(ch, obj_object))
	      return;

	    next_obj = obj_object->next_content;
	    if (CAN_SEE_OBJ(ch, obj_object)) {
	      if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
		if ((IS_CARRYING_W(ch) + obj_object->obj_flags.weight) <
		    CAN_CARRY_W(ch)) {
		  if (CAN_WEAR(obj_object, ITEM_TAKE)) {
		    get(ch, obj_object, sub_object);
		    found = TRUE;
		  } else {
		    send_to_char("You can't take that\n\r", ch);
		    fail = TRUE;
		  }
		} else {
		  sprintf(buffer,
		       "%s : You can't carry that much weight.\n\r",
			  obj_object->short_description);
		  send_to_char(buffer, ch);
		  fail = TRUE;
		}
	      } else {
		sprintf(buffer, "%s : You can't carry that many items.\n\r",
			obj_object->short_description);
		send_to_char(buffer, ch);
		fail = TRUE;
	      }
	    }
	  }
	  if (!found && !fail) {
	    sprintf(buffer, "You do not see anything in %s.\n\r",
		    sub_object->short_description);
	    send_to_char(buffer, ch);
	    fail = TRUE;
	  }
	} else {
	  sprintf(buffer, "%s is not a container.\n\r",
		  sub_object->short_description);
	  send_to_char(buffer, ch);
	  fail = TRUE;
	}
      } else {
	sprintf(buffer, "You do not see or have the %s.\n\r", arg2);
	send_to_char(buffer, ch);
	fail = TRUE;
      }
    }
    break;
  case 5:{
      send_to_char("You can't take a thing from more than one container.\n\r",
		   ch);
    }
    break;
/*  
 * take ??? from ???   (is it??) 
 */

  case 6:{
      found = FALSE;
      fail = FALSE;
      sub_object = (struct obj_data *)
	get_obj_vis_accessible(ch, arg2);
      if (sub_object) {
	if (GET_ITEM_TYPE(sub_object) == ITEM_CONTAINER) {

	  if (getall(arg1, newarg) != FALSE) {
	    num = -1;
	    strcpy(arg1, newarg);
	  } else if ((p = getabunch(arg1, newarg)) != FALSE) {
	    num = p;
	    strcpy(arg1, newarg);
	  } else {
	    num = 1;
	  }
	  while (num != 0) {
	    obj_object = get_obj_in_list_vis(ch, arg1, sub_object->contains);
	    if (obj_object) {
	      if (CheckForInsideTrap(ch, sub_object))
		return;
	      if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
		if ((IS_CARRYING_W(ch) + obj_object->obj_flags.weight) <
		    CAN_CARRY_W(ch)) {
		  if (CAN_WEAR(obj_object, ITEM_TAKE)) {
		    get(ch, obj_object, sub_object);
		    found = TRUE;
		  } else {
		    send_to_char("You can't take that\n\r", ch);
		    fail = TRUE;
		    num = 0;
		  }
		} else {
		  sprintf(buffer, "%s : You can't carry that much weight.\n\r",
			  obj_object->short_description);
		  send_to_char(buffer, ch);
		  fail = TRUE;
		  num = 0;
		}
	      } else {
		sprintf(buffer, "%s : You can't carry that many items.\n\r",
			obj_object->short_description);
		send_to_char(buffer, ch);
		fail = TRUE;
		num = 0;
	      }
	    } else {
	      if (num > 0) {
		sprintf(buffer, "%s does not contain the %s.\n\r", sub_object->short_description, arg1);
		send_to_char(buffer, ch);
	      }
	      num = 0;
	      fail = TRUE;
	    }
	    if (num > 0)
	      num--;
	  }
	} else {
	  sprintf(buffer, "%s is not a container.\n\r", sub_object->short_description);
	  send_to_char(buffer, ch);
	  fail = TRUE;
	}
      } else {
	sprintf(buffer, "You do not see or have the %s.\n\r", arg2);
	send_to_char(buffer, ch);
	fail = TRUE;
      }
    }
    break;
  }
}

void do_drop(struct char_data *ch, char *argument, int cmd)
{
  char arg[MAX_INPUT_LENGTH];
  int amount;
  char buffer[MAX_STRING_LENGTH];
  struct obj_data *tmp_object;
  struct obj_data *next_obj;
  BYTE test = FALSE;
  char newarg[100];
  char *s;
  int num, p;

  if (DEBUG)
    dlog("do_drop");
  s = one_argument(argument, arg);
  if (is_number(arg)) {
    amount = atoi(arg);
    strcpy(arg, s);

    /* 
     * if (0!=str_cmp("coins",arg) && 0!=str_cmp("coin",arg))  {
     * send_to_char("Sorry, you can't do that (yet)...\n\r",ch);
     * return;
     * }
     */

    if (amount < 0) {
      send_to_char("Sorry, you can't do that!\n\r", ch);
      return;
    }
    if (GET_GOLD(ch) < amount) {
      send_to_char("You haven't got that many coins!\n\r", ch);
      return;
    }
    send_to_char("OK.\n\r", ch);
    if (amount == 0)
      return;

    act("$n drops some gold.", FALSE, ch, 0, 0, TO_ROOM);
    tmp_object = create_money(amount);
    obj_to_room(tmp_object, ch->in_room);
    GET_GOLD(ch) -= amount;
    return;
  } else {
    only_argument(argument, arg);
  }

  if (*arg) {
    if (!str_cmp(arg, "all")) {
      for (tmp_object = ch->carrying;
	   tmp_object;
	   tmp_object = next_obj) {
	next_obj = tmp_object->next_content;
	if (!IS_SET(tmp_object->obj_flags.extra_flags, ITEM_NODROP) || IS_IMMORTAL(ch)) {
	  if (CAN_SEE_OBJ(ch, tmp_object)) {
	    sprintf(buffer, "You drop %s.\n\r", tmp_object->short_description);
	    send_to_char(buffer, ch);
	  } else {
	    send_to_char("You drop something.\n\r", ch);
	  }
	  act("$n drops $p.", 1, ch, tmp_object, 0, TO_ROOM);
	  obj_from_char(tmp_object);
	  obj_to_room(tmp_object, ch->in_room);
	  test = TRUE;
	} else {
	  if (CAN_SEE_OBJ(ch, tmp_object)) {
	    sprintf(buffer, "You can't drop  %s, it must be CURSED!\n\r", tmp_object->short_description);
	    send_to_char(buffer, ch);
	    test = TRUE;
	  }
	}
      }
      if (!test) {
	send_to_char("You do not seem to have anything.\n\r", ch);
      }
    } else {
      /* &&&&&& */
      if (getall(arg, newarg) != FALSE) {
	num = -1;
	strcpy(arg, newarg);
      } else if ((p = getabunch(arg, newarg)) != FALSE) {
	num = p;
	strcpy(arg, newarg);
      } else {
	num = 1;
      }
      while (num != 0) {
	tmp_object = get_obj_in_list_vis(ch, arg, ch->carrying);
	if (tmp_object) {
	  if (!IS_SET(tmp_object->obj_flags.extra_flags, ITEM_NODROP) || IS_IMMORTAL(ch)) {
	    sprintf(buffer, "You drop %s.\n\r", tmp_object->short_description);
	    send_to_char(buffer, ch);
	    act("$n drops $p.", 1, ch, tmp_object, 0, TO_ROOM);
	    obj_from_char(tmp_object);
	    obj_to_room(tmp_object, ch->in_room);
	  } else {
	    send_to_char("You can't drop it, it must be CURSED!\n\r", ch);
	    num = 0;
	  }
	} else {
	  if (num > 0)
	    send_to_char("You do not have that item.\n\r", ch);
	  num = 0;
	}
	if (num > 0)
	  num--;
      }
    }
  } else {
    send_to_char("Drop what?\n\r", ch);
  }
}

void do_put(struct char_data *ch, char *argument, int cmd)
{
  char buffer[256];
  char arg1[128];
  char arg2[128];
  struct obj_data *obj_object;
  struct obj_data *sub_object;
  struct char_data *tmp_char;
  int bits;
  char newarg[100];
  int num, p;

  if (DEBUG)
    dlog("do_put");
  argument_interpreter(argument, arg1, arg2);

  if (*arg1) {
    if (*arg2) {

      if (getall(arg1, newarg) != FALSE) {
	num = -1;
	strcpy(arg1, newarg);
      } else if ((p = getabunch(arg1, newarg)) != FALSE) {
	num = p;
	strcpy(arg1, newarg);
      } else {
	num = 1;
      }

      if (!strcmp(arg1, "all")) {

	send_to_char("sorry, you can't do that (yet)\n\r", ch);
	return;

      } else {
	while (num != 0) {
#if 1
	  bits = generic_find(arg1, FIND_OBJ_INV,
			      ch, &tmp_char, &obj_object);
#else
	  obj_object = get_obj_in_list_vis(ch, arg1, ch->carrying);
#endif

	  if (obj_object) {
	    bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM,
				ch, &tmp_char, &sub_object);
	    if (sub_object) {
	      if (GET_ITEM_TYPE(sub_object) == ITEM_CONTAINER) {
		if (!IS_SET(sub_object->obj_flags.value[1], CONT_CLOSED)) {
		  if (obj_object == sub_object) {
		    send_to_char("You attempt to fold it into itself, but fail.\n\r", ch);
		    return;
		  }
		  if (((sub_object->obj_flags.weight) +
		       (obj_object->obj_flags.weight)) <
		      (sub_object->obj_flags.value[0])) {
		    act("You put $p in $P", TRUE, ch, obj_object, sub_object, TO_CHAR);
		    if (bits == FIND_OBJ_INV) {
		      obj_from_char(obj_object);
		      /* make up for above line */
		      IS_CARRYING_W(ch) += GET_OBJ_WEIGHT(obj_object);

		      obj_to_obj(obj_object, sub_object);
		    } else {
		      obj_from_room(obj_object);
		      obj_to_obj(obj_object, sub_object);
		    }

		    act("$n puts $p in $P", TRUE, ch, obj_object, sub_object, TO_ROOM);
		    num--;
		  } else {
		    send_to_char("It won't fit.\n\r", ch);
		    num = 0;
		  }
		} else {
		  send_to_char("It seems to be closed.\n\r", ch);
		  num = 0;
		}
	      } else {
		sprintf(buffer, "%s is not a container.\n\r", sub_object->short_description);
		send_to_char(buffer, ch);
		num = 0;
	      }
	    } else {
	      sprintf(buffer, "You don't have the %s.\n\r", arg2);
	      send_to_char(buffer, ch);
	      num = 0;
	    }
	  } else {
	    if ((num > 0) || (num == -1)) {
	      sprintf(buffer, "You don't have the %s.\n\r", arg1);
	      send_to_char(buffer, ch);
	    }
	    num = 0;
	  }
	}
      }
    } else {
      sprintf(buffer, "Put %s in what?\n\r", arg1);
      send_to_char(buffer, ch);
    }
  } else {
    send_to_char("Put what in what?\n\r", ch);
  }
}

void do_give(struct char_data *ch, char *argument, int cmd)
{
  char obj_name[80], vict_name[80], buf[132];
  char arg[80], newarg[100];
  int amount, num, p;
  struct char_data *vict;
  struct obj_data *obj;

  if (DEBUG)
    dlog("do_give");
  argument = one_argument(argument, obj_name);
  if (is_number(obj_name)) {
    amount = atoi(obj_name);
    argument = one_argument(argument, arg);
/*
 * if (str_cmp("coins",arg) && str_cmp("coin",arg))        
 * {
 * send_to_char("Sorry, you can't do that (yet)...\n\r",ch);
 * return;
 * }
 */

    if (amount < 0) {
      send_to_char("Sorry, you can't do that!\n\r", ch);
      return;
    }
    if ((GET_GOLD(ch) < amount) && (IS_NPC(ch) || (GetMaxLevel(ch) < DEMIGOD))) {
      send_to_char("You haven't got that many coins!\n\r", ch);
      return;
    }
    argument = one_argument(argument, vict_name);

    if (!*vict_name) {
      send_to_char("To who?\n\r", ch);
      return;
    }
    if (!(vict = get_char_room_vis(ch, vict_name))) {
      send_to_char("To who?\n\r", ch);
      return;
    }
    send_to_char("Ok.\n\r", ch);
    sprintf(buf, "%s gives you %d gold coins.\n\r", PERS(ch, vict), amount);
    send_to_char(buf, vict);
    act("$n gives some gold to $N.", 1, ch, 0, vict, TO_NOTVICT);
    if (IS_NPC(ch) || (GetMaxLevel(ch) < DEMIGOD))
      GET_GOLD(ch) -= amount;
    GET_GOLD(vict) += amount;
    if ((amount > 1000) && (GetMaxLevel(ch) >= DEMIGOD)) {	/* hmmm */
      sprintf(buf, "%s gave %d coins to %s", GET_NAME(ch), amount, GET_NAME(vict));
      log(buf);
    }
    return;
  }
  argument = one_argument(argument, vict_name);
  if (!*obj_name || !*vict_name) {
    send_to_char("Give what to who?\n\r", ch);
    return;
  }
  /* &&&& */
  if (getall(obj_name, newarg) != FALSE) {
    num = -1;
    strcpy(obj_name, newarg);
  } else if ((p = getabunch(obj_name, newarg)) != FALSE) {
    num = p;
    strcpy(obj_name, newarg);
  } else {
    num = 1;
  }

  while (num != 0) {
    if (!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying))) {
      if (num >= -1)
	send_to_char("You do not seem to have anything like that.\n\r",
		     ch);
      return;
    }
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP) && !IS_IMMORTAL(ch)) {
      send_to_char("You can't let go of it! Yeech!!\n\r", ch);
      return;
    }
    if (!(vict = get_char_room_vis(ch, vict_name))) {
      send_to_char("No one by that name around here.\n\r", ch);
      return;
    }
    if (vict == ch) {
      sprintf(buf, "%s just tried to give all.X to %s", GET_NAME(ch), GET_NAME(ch));
      log(buf);
      send_to_char("Ok.\n\r", ch);
      return;
    }
    if ((1 + IS_CARRYING_N(vict)) > CAN_CARRY_N(vict)) {
      act("$N seems to have $S hands full.", 0, ch, 0, vict, TO_CHAR);
      return;
    }
    if (obj->obj_flags.weight + IS_CARRYING_W(vict) > CAN_CARRY_W(vict)) {
      act("$E can't carry that much weight.", 0, ch, 0, vict, TO_CHAR);
      return;
    }
    obj_from_char(obj);
    obj_to_char(obj, vict);
    act("$n gives $p to $N.", 1, ch, obj, vict, TO_NOTVICT);
    act("$n gives you $p.", 0, ch, obj, vict, TO_VICT);
    act("You give $p to $N", 0, ch, obj, vict, TO_CHAR);

    if (num > 0)
      num--;

  }

}

/*
 * file: act.obj2.c , Implementation of commands.         Part of DIKUMUD
 * Usage : Commands mainly using objects.
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */

void weight_change_object(struct obj_data *obj, int weight)
{
  struct obj_data *tmp_obj;
  struct char_data *tmp_ch;

  if (DEBUG)
    dlog("weight_change_object");
  if (obj->in_room != NOWHERE) {
    GET_OBJ_WEIGHT(obj) += weight;
  } else if (tmp_ch = obj->carried_by) {
    obj_from_char(obj);
    GET_OBJ_WEIGHT(obj) += weight;
    obj_to_char(obj, tmp_ch);
  } else if (tmp_obj = obj->in_obj) {
    obj_from_obj(obj);
    GET_OBJ_WEIGHT(obj) += weight;
    obj_to_obj(obj, tmp_obj);
  } else {
    log("Unknown attempt to subtract weight from an object.");
  }
}

void name_from_drinkcon(struct obj_data *obj)
{
  int i;
  char *new_name;
  char buf[100];

  if (DEBUG)
    dlog("name_from_drinkcon");
  one_argument(obj->name, buf);
  new_name = strdup(buf);
  free(obj->name);
  obj->name = new_name;
  return;

  for (i = 0; (*((obj->name) + i) != ' ') && (*((obj->name) + i) != '\0'); i++);
  if (*((obj->name) + i) == ' ') {
    *((obj->name) + i + 1) == '\0';
    new_name = strdup(obj->name);
    free(obj->name);
    obj->name = new_name;
  }
}

void name_to_drinkcon(struct obj_data *obj, int type)
{
  char *new_name;

  if (DEBUG)
    dlog("name_to_drinkcon");
  CREATE(new_name, char, strlen(obj->name) + strlen(drinknames[type]) + 2);

  sprintf(new_name, "%s %s", obj->name, drinknames[type]);
  free(obj->name);
  obj->name = new_name;
}

void do_drink(struct char_data *ch, char *argument, int cmd)
{
  char buf[255];
  struct obj_data *temp;
  struct affected_type af;
  int amount;

  if (DEBUG)
    dlog("do_drink");
  only_argument(argument, buf);

  if (!(temp = get_obj_in_list_vis(ch, buf, ch->carrying))) {
    act("You can't find it!", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  if (temp->obj_flags.type_flag != ITEM_DRINKCON) {
    act("You can't drink from that!", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  if ((GET_COND(ch, DRUNK) > 15) && (GET_COND(ch, THIRST) > 0)) {
/* The pig is drunk */
    act("You're just sloshed.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n looks really drunk.", TRUE, ch, 0, 0, TO_ROOM);
    return;
  }
  if ((GET_COND(ch, FULL) > 20) && (GET_COND(ch, THIRST) > 0)) {	/* Stomach full */
    act("Your stomach can't contain anymore!", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  if (temp->obj_flags.type_flag == ITEM_DRINKCON) {
    if (temp->obj_flags.value[1] > 0) {		/* Not empty */
      sprintf(buf, "$n drinks %s from $p", drinks[temp->obj_flags.value[2]]);
      act(buf, TRUE, ch, temp, 0, TO_ROOM);
      sprintf(buf, "You drink the %s.\n\r", drinks[temp->obj_flags.value[2]]);
      send_to_char(buf, ch);

      if (drink_aff[temp->obj_flags.value[2]][DRUNK] > 0)
	amount = (25 - GET_COND(ch, THIRST)) / drink_aff[temp->obj_flags.value[2]][DRUNK];
      else
	amount = number(3, 10);

      amount = MIN(amount, temp->obj_flags.value[1]);
/* Subtract amount */
      if (temp->obj_flags.value[0] > 20)
	weight_change_object(temp, -amount);

      gain_condition(ch, DRUNK, (int)((int)drink_aff
		   [temp->obj_flags.value[2]][DRUNK] * amount) / 4);

      gain_condition(ch, FULL, (int)((int)drink_aff
		    [temp->obj_flags.value[2]][FULL] * amount) / 4);

      gain_condition(ch, THIRST, (int)((int)drink_aff
		  [temp->obj_flags.value[2]][THIRST] * amount) / 4);

      if (GET_COND(ch, DRUNK) > 10)
	act("You feel drunk.", FALSE, ch, 0, 0, TO_CHAR);

      if (GET_COND(ch, THIRST) > 20)
	act("You do not feel thirsty.", FALSE, ch, 0, 0, TO_CHAR);

      if (GET_COND(ch, FULL) > 20)
	act("You are full.", FALSE, ch, 0, 0, TO_CHAR);

      if (temp->obj_flags.value[3]) {  /* The shit was poisoned ! */
	act("Oops, it tasted rather strange ?!!?", FALSE, ch, 0, 0, TO_CHAR);
	act("$n chokes and utters some strange sounds.",
	    TRUE, ch, 0, 0, TO_ROOM);
	af.type = SPELL_POISON;
	af.duration = amount * 3;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_POISON;
	affect_join(ch, &af, FALSE, FALSE);
      }
      /* empty the container, and no longer poison. */
      temp->obj_flags.value[1] -= amount;
      if (!temp->obj_flags.value[1]) { /* The last bit */
	temp->obj_flags.value[2] = 0;
	temp->obj_flags.value[3] = 0;
	name_from_drinkcon(temp);
      }
      if (temp->obj_flags.value[1] < 1) {	/* its empty */
	if (temp->obj_flags.value[0] < 20) {
	  extract_obj(temp);	       /* get rid of it */
	}
      }
      return;

    }
    act("It's empty already.", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
}

void do_puke(struct char_data *ch, char *argument, int cmd)
{
  char buf[100];
  int j, num;
  struct obj_data *temp;
  struct affected_type af;
  struct char_data *vict;

  if (DEBUG)
    dlog("do_puke");
  one_argument(argument, buf);

  if (!*buf) {
    act("$n blows chunks all over the room!",FALSE,ch,0,0,TO_ROOM);
    act("You puke and spew filth all over the place.",FALSE,ch,0,0,TO_CHAR);
  }
  else if (!(vict = get_char_room_vis(ch, buf))) {
    send_to_char("You can't puke on someone who isn't here.\n\r", ch);
    return;
  }
  else {
     act("$n walks up to $N and pukes up sickly green ichor all over them!",FALSE,ch,0,vict,TO_ROOM);
     act("You vomit green chunks all over $N!",FALSE,ch,0,vict,TO_CHAR);
  }
  if(IS_MORTAL(ch)) {
    gain_condition(ch, FULL, -3);
    if(GET_COND(ch, FULL) < 0) {
      GET_COND(ch, FULL) = 0;
    }
    damage(ch, ch, 1, TYPE_SUFFERING);
  }
  return;
}


void do_eat(struct char_data *ch, char *argument, int cmd)
{
  char buf[100];
  int j, num;
  struct obj_data *temp;
  struct affected_type af;

  if (DEBUG)
    dlog("do_eat");
  one_argument(argument, buf);

  if (!(temp = get_obj_in_list_vis(ch, buf, ch->carrying))) {
    act("You can't find it!", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  if ((temp->obj_flags.type_flag != ITEM_FOOD) &&
      (GetMaxLevel(ch) < DEMIGOD)) {
    act("Your stomach refuses to eat that!?!", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  if (GET_COND(ch, FULL) > 20) {       /* Stomach full */
    act("You are to full to eat more!", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  act("$n eats $p", TRUE, ch, temp, 0, TO_ROOM);
  act("You eat the $o.", FALSE, ch, temp, 0, TO_CHAR);

  gain_condition(ch, FULL, temp->obj_flags.value[0]);

  if (GET_COND(ch, FULL) > 20)
    act("You are full.", FALSE, ch, 0, 0, TO_CHAR);

  for (j = 0; j < MAX_OBJ_AFFECT; j++)
    if (temp->affected[j].location == APPLY_EAT_SPELL) {
      num = temp->affected[j].modifier;

/* hit 'em with the spell */

      ((*spell_info[num].spell_pointer) (6, ch, "", SPELL_TYPE_POTION, ch, 0));
    }
  if (temp->obj_flags.value[3] && (GetMaxLevel(ch) < LOW_IMMORTAL)) {
    act("That tasted rather strange !!", FALSE, ch, 0, 0, TO_CHAR);
    act("$n coughs and utters some strange sounds.",
	FALSE, ch, 0, 0, TO_ROOM);

    af.type = SPELL_POISON;
    af.duration = temp->obj_flags.value[0] * 2;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_POISON;
    affect_join(ch, &af, FALSE, FALSE);
  }
  extract_obj(temp);
}

void do_pour(struct char_data *ch, char *argument, int cmd)
{
  char arg1[132];
  char arg2[132];
  char buf[256];
  struct obj_data *from_obj;
  struct obj_data *to_obj;
  int temp;

  if (DEBUG)
    dlog("do_pour");
  argument_interpreter(argument, arg1, arg2);

  if (!*arg1) {			       /* No arguments */
    act("What do you want to pour from?", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  if (!(from_obj = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
    act("You can't find it!", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  if (from_obj->obj_flags.type_flag != ITEM_DRINKCON) {
    act("You can't pour from that!", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  if (from_obj->obj_flags.value[1] == 0) {
    act("The $p is empty.", FALSE, ch, from_obj, 0, TO_CHAR);
    return;
  }
  if (!*arg2) {
    act("Where do you want it? Out or in what?", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  if (!str_cmp(arg2, "out")) {
    act("$n empties $p", TRUE, ch, from_obj, 0, TO_ROOM);
    act("You empty the $p.", FALSE, ch, from_obj, 0, TO_CHAR);

    weight_change_object(from_obj, -from_obj->obj_flags.value[1]);

    from_obj->obj_flags.value[1] = 0;
    from_obj->obj_flags.value[2] = 0;
    from_obj->obj_flags.value[3] = 0;
    name_from_drinkcon(from_obj);

    return;

  }
  if (!(to_obj = get_obj_in_list_vis(ch, arg2, ch->carrying))) {
    act("You can't find it!", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  if (to_obj->obj_flags.type_flag != ITEM_DRINKCON) {
    act("You can't pour anything into that.", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  if ((to_obj->obj_flags.value[1] != 0) &&
      (to_obj->obj_flags.value[2] != from_obj->obj_flags.value[2])) {
    act("There is already another liquid in it!", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  if (!(to_obj->obj_flags.value[1] < to_obj->obj_flags.value[0])) {
    act("There is no room for more.", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  if (to_obj == from_obj) {
    act("You can't pour to-from the same container?", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  sprintf(buf, "You pour the %s into the %s.",
	  drinks[from_obj->obj_flags.value[2]], arg2);
  send_to_char(buf, ch);

  /* New alias */
  if (to_obj->obj_flags.value[1] == 0)
    name_to_drinkcon(to_obj, from_obj->obj_flags.value[2]);

  /* First same type liq. */
  to_obj->obj_flags.value[2] = from_obj->obj_flags.value[2];

/*
 * the new, improved way of doing this...
 */
  temp = from_obj->obj_flags.value[1];
  from_obj->obj_flags.value[1] = 0;
  to_obj->obj_flags.value[1] += temp;
  temp = to_obj->obj_flags.value[1] - to_obj->obj_flags.value[0];

  if (temp > 0) {
    from_obj->obj_flags.value[1] = temp;
  } else {
    name_from_drinkcon(from_obj);
  }

  if (from_obj->obj_flags.value[1] > from_obj->obj_flags.value[0])
    from_obj->obj_flags.value[1] = from_obj->obj_flags.value[0];

  /* Then the poison boogie */
  to_obj->obj_flags.value[3] =
    (to_obj->obj_flags.value[3] || from_obj->obj_flags.value[3]);

  return;
}

void do_sip(struct char_data *ch, char *argument, int cmd)
{
  struct affected_type af;
  char arg[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  struct obj_data *temp;

  if (DEBUG)
    dlog("do_sip");
  one_argument(argument, arg);

  if (!(temp = get_obj_in_list_vis(ch, arg, ch->carrying))) {
    act("You can't find it!", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  if (temp->obj_flags.type_flag != ITEM_DRINKCON) {
    act("You can't sip from that!", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  if (GET_COND(ch, DRUNK) > 10) {      /* The pig is drunk ! */
    act("You simply fail to reach your mouth!", FALSE, ch, 0, 0, TO_CHAR);
    act("$n tries to sip, but fails!", TRUE, ch, 0, 0, TO_ROOM);
    return;
  }
  if (!temp->obj_flags.value[1]) {     /* Empty */
    act("But there is nothing in it?", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  act("$n sips from the $o", TRUE, ch, temp, 0, TO_ROOM);
  sprintf(buf, "It tastes like %s.\n\r", drinks[temp->obj_flags.value[2]]);
  send_to_char(buf, ch);

  gain_condition(ch, DRUNK, (int)(drink_aff[temp->obj_flags.value[2]][DRUNK] / 4));

  gain_condition(ch, FULL, (int)(drink_aff[temp->obj_flags.value[2]][FULL] / 4));

  gain_condition(ch, THIRST, (int)(drink_aff[temp->obj_flags.value[2]][THIRST] / 4));

  weight_change_object(temp, -1);      /* Subtract one unit */

  if (GET_COND(ch, DRUNK) > 10)
    act("You feel drunk.", FALSE, ch, 0, 0, TO_CHAR);

  if (GET_COND(ch, THIRST) > 20)
    act("You do not feel thirsty.", FALSE, ch, 0, 0, TO_CHAR);

  if (GET_COND(ch, FULL) > 20)
    act("You are full.", FALSE, ch, 0, 0, TO_CHAR);

  if (temp->obj_flags.value[3] && !IS_AFFECTED(ch, AFF_POISON)) {	/* The shit was poisoned ! */
    act("But it also had a strange taste!", FALSE, ch, 0, 0, TO_CHAR);

    af.type = SPELL_POISON;
    af.duration = 3;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_POISON;
    affect_to_char(ch, &af);
  }
  temp->obj_flags.value[1]--;

  if (!temp->obj_flags.value[1]) {     /* The last bit */
    temp->obj_flags.value[2] = 0;
    temp->obj_flags.value[3] = 0;
    name_from_drinkcon(temp);
  }
  return;

}

void do_taste(struct char_data *ch, char *argument, int cmd)
{
  struct affected_type af;
  char arg[80];
  struct obj_data *temp;

  if (DEBUG)
    dlog("do_taste");
  one_argument(argument, arg);

  if (!(temp = get_obj_in_list_vis(ch, arg, ch->carrying))) {
    act("You can't find it!", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  if (temp->obj_flags.type_flag == ITEM_DRINKCON) {
    do_sip(ch, argument, 0);
    return;
  }
  if (!(temp->obj_flags.type_flag == ITEM_FOOD)) {
    act("Taste that?!? Your stomach refuses!", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  act("$n tastes the $o", FALSE, ch, temp, 0, TO_ROOM);
  act("You taste the $o", FALSE, ch, temp, 0, TO_CHAR);

  gain_condition(ch, FULL, 1);

  if (GET_COND(ch, FULL) > 20)
    act("You are full.", FALSE, ch, 0, 0, TO_CHAR);

  if (temp->obj_flags.value[3] && !IS_AFFECTED(ch, AFF_POISON)) {	/* The shit was poisoned ! */
    act("Ooups, it did not taste good at all!", FALSE, ch, 0, 0, TO_CHAR);

    af.type = SPELL_POISON;
    af.duration = 2;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_POISON;
    affect_to_char(ch, &af);
  }
  temp->obj_flags.value[0]--;

  if (!temp->obj_flags.value[0]) {     /* Nothing left */
    act("There is nothing left now.", FALSE, ch, 0, 0, TO_CHAR);
    extract_obj(temp);
  }
  return;

}

/* functions related to wear */

perform_wear(struct char_data * ch, struct obj_data * obj_object, int keyword)
{
  if (DEBUG)
    dlog("perfrom_wear");
  switch (keyword) {
  case 0:
    act("$n lights $p and holds it.", FALSE, ch, obj_object, 0, TO_ROOM);
    break;
  case 1:
    act("$n wears $p on $s finger.", TRUE, ch, obj_object, 0, TO_ROOM);
    break;
  case 2:
    act("$n wears $p around $s neck.", TRUE, ch, obj_object, 0, TO_ROOM);
    break;
  case 3:
    act("$n wears $p on $s body.", TRUE, ch, obj_object, 0, TO_ROOM);
    break;
  case 4:
    act("$n wears $p on $s head.", TRUE, ch, obj_object, 0, TO_ROOM);
    break;
  case 5:
    act("$n wears $p on $s legs.", TRUE, ch, obj_object, 0, TO_ROOM);
    break;
  case 6:
    act("$n wears $p on $s feet.", TRUE, ch, obj_object, 0, TO_ROOM);
    break;
  case 7:
    act("$n wears $p on $s hands.", TRUE, ch, obj_object, 0, TO_ROOM);
    break;
  case 8:
    act("$n wears $p on $s arms.", TRUE, ch, obj_object, 0, TO_ROOM);
    break;
  case 9:
    act("$n wears $p about $s body.", TRUE, ch, obj_object, 0, TO_ROOM);
    break;
  case 10:
    act("$n wears $p about $s waist.", TRUE, ch, obj_object, 0, TO_ROOM);
    break;
  case 11:
    act("$n wears $p around $s wrist.", TRUE, ch, obj_object, 0, TO_ROOM);
    break;
  case 12:
    act("$n wields $p.", TRUE, ch, obj_object, 0, TO_ROOM);
    break;
  case 13:
    act("$n grabs $p.", TRUE, ch, obj_object, 0, TO_ROOM);
    break;
  case 14:
    act("$n starts using $p as shield.", TRUE, ch, obj_object, 0, TO_ROOM);
    break;
  case 15:
    act("$n wields $p two handed.", TRUE, ch, obj_object, 0, TO_ROOM);
    break;

  }
}

int IsRestricted(int Mask, int Class)
{
  int i;

  if (DEBUG)
    dlog("IsRestricted");
  for (i = CLASS_MAGIC_USER; i <= CLASS_DRUID; i *= 2) {
    if (IS_SET(Mask, i) && (IS_NOT_SET(i, Class))) {
      Mask -= i;
    }
  }

  if (Mask == Class)
    return (TRUE);

  return (FALSE);
}

void wear(struct char_data *ch, struct obj_data *obj_object, int keyword)
{
  char buffer[MAX_STRING_LENGTH];
  int BitMask;

  if (DEBUG)
    dlog("wear");

  if (!IS_IMMORTAL(ch)) {
    BitMask = GetItemClassRestrictions(obj_object);
    if (IsRestricted(BitMask, ch->player.class) &&
	(IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF))) {
      send_to_char("You are forbidden to do that.\n\r", ch);
      return;
    }
  }
  if (!IsHumanoid(ch)) {
    if ((keyword != 13) || (!HasHands(ch))) {
      send_to_char("You can't wear things!\n\r", ch);
      return;
    }
  }
  switch (keyword) {
  case 0:
    {				       /* LIGHT SOURCE */
      if (ch->equipment[WEAR_LIGHT])
	send_to_char("You are already holding a light source.\n\r", ch);
      else {
	send_to_char("Ok.\n\r", ch);
	perform_wear(ch, obj_object, keyword);
	obj_from_char(obj_object);
	equip_char(ch, obj_object, WEAR_LIGHT);
	if (obj_object->obj_flags.value[2])
	  real_roomp(ch->in_room)->light++;
      }
    }
    break;

  case 1:
    {
      if (CAN_WEAR(obj_object, ITEM_WEAR_FINGER)) {
	if ((ch->equipment[WEAR_FINGER_L]) && (ch->equipment[WEAR_FINGER_R])) {
	  send_to_char("You are already wearing something on your fingers.\n\r", ch);
	} else {
	  perform_wear(ch, obj_object, keyword);
	  if (ch->equipment[WEAR_FINGER_L]) {
	    sprintf(buffer, "You put %s on your right finger.\n\r",
		    obj_object->short_description);
	    send_to_char(buffer, ch);
	    obj_from_char(obj_object);
	    equip_char(ch, obj_object, WEAR_FINGER_R);
	  } else {
	    sprintf(buffer, "You put %s on your left finger.\n\r",
		    obj_object->short_description);
	    send_to_char(buffer, ch);
	    obj_from_char(obj_object);
	    equip_char(ch, obj_object, WEAR_FINGER_L);
	  }
	}
      } else {
	send_to_char("You can't wear that on your finger.\n\r", ch);
      }
    }
    break;
  case 2:
    {
      if (CAN_WEAR(obj_object, ITEM_WEAR_NECK)) {
	if ((ch->equipment[WEAR_NECK_1]) && (ch->equipment[WEAR_NECK_2])) {
	  send_to_char("You can't wear any more around your neck.\n\r", ch);
	} else {
	  send_to_char("OK.\n\r", ch);
	  perform_wear(ch, obj_object, keyword);
	  if (ch->equipment[WEAR_NECK_1]) {
	    obj_from_char(obj_object);
	    equip_char(ch, obj_object, WEAR_NECK_2);
	  } else {
	    obj_from_char(obj_object);
	    equip_char(ch, obj_object, WEAR_NECK_1);
	  }
	}
      } else {
	send_to_char("You can't wear that around your neck.\n\r", ch);
      }
    }
    break;
  case 3:
    {
      if (CAN_WEAR(obj_object, ITEM_WEAR_BODY)) {
	if (ch->equipment[WEAR_BODY]) {
	  send_to_char("You already wear something on your body.\n\r", ch);
	} else {
	  send_to_char("OK.\n\r", ch);
	  perform_wear(ch, obj_object, keyword);
	  obj_from_char(obj_object);
	  equip_char(ch, obj_object, WEAR_BODY);
	}
      } else {
	send_to_char("You can't wear that on your body.\n\r", ch);
      }
    }
    break;
  case 4:
    {
      if (CAN_WEAR(obj_object, ITEM_WEAR_HEAD)) {
	if (ch->equipment[WEAR_HEAD]) {
	  send_to_char("You already wear something on your head.\n\r", ch);
	} else {
	  send_to_char("OK.\n\r", ch);
	  perform_wear(ch, obj_object, keyword);
	  obj_from_char(obj_object);
	  equip_char(ch, obj_object, WEAR_HEAD);
	}
      } else {
	send_to_char("You can't wear that on your head.\n\r", ch);
      }
    }
    break;
  case 5:{
      if (CAN_WEAR(obj_object, ITEM_WEAR_LEGS)) {
	if (ch->equipment[WEAR_LEGS]) {
	  send_to_char("You already wear something on your legs.\n\r", ch);
	} else {
	  send_to_char("OK.\n\r", ch);
	  perform_wear(ch, obj_object, keyword);
	  obj_from_char(obj_object);
	  equip_char(ch, obj_object, WEAR_LEGS);
	}
      } else {
	send_to_char("You can't wear that on your legs.\n\r", ch);
      }
    }
    break;
  case 6:{
      if (CAN_WEAR(obj_object, ITEM_WEAR_FEET)) {
	if (ch->equipment[WEAR_FEET]) {
	  send_to_char("You already wear something on your feet.\n\r", ch);
	} else {
	  send_to_char("OK.\n\r", ch);
	  perform_wear(ch, obj_object, keyword);
	  obj_from_char(obj_object);
	  equip_char(ch, obj_object, WEAR_FEET);
	}
      } else {
	send_to_char("You can't wear that on your feet.\n\r", ch);
      }
    }
    break;
  case 7:{
      if (CAN_WEAR(obj_object, ITEM_WEAR_HANDS)) {
	if (ch->equipment[WEAR_HANDS]) {
	  send_to_char("You already wear something on your hands.\n\r", ch);
	} else {
	  send_to_char("OK.\n\r", ch);
	  perform_wear(ch, obj_object, keyword);
	  obj_from_char(obj_object);
	  equip_char(ch, obj_object, WEAR_HANDS);
	}
      } else {
	send_to_char("You can't wear that on your hands.\n\r", ch);
      }
    }
    break;
  case 8:{
      if (CAN_WEAR(obj_object, ITEM_WEAR_ARMS)) {
	if (ch->equipment[WEAR_ARMS]) {
	  send_to_char("You already wear something on your arms.\n\r", ch);
	} else {
	  send_to_char("OK.\n\r", ch);
	  perform_wear(ch, obj_object, keyword);
	  obj_from_char(obj_object);
	  equip_char(ch, obj_object, WEAR_ARMS);
	}
      } else {
	send_to_char("You can't wear that on your arms.\n\r", ch);
      }
    }
    break;
  case 9:{
      if (CAN_WEAR(obj_object, ITEM_WEAR_ABOUT)) {
	if (ch->equipment[WEAR_ABOUT]) {
	  send_to_char("You already wear something about your body.\n\r", ch);
	} else {
	  send_to_char("OK.\n\r", ch);
	  perform_wear(ch, obj_object, keyword);
	  obj_from_char(obj_object);
	  equip_char(ch, obj_object, WEAR_ABOUT);
	}
      } else {
	send_to_char("You can't wear that about your body.\n\r", ch);
      }
    }
    break;
  case 10:{
      if (CAN_WEAR(obj_object, ITEM_WEAR_WAISTE)) {
	if (ch->equipment[WEAR_WAISTE]) {
	  send_to_char("You already wear something about your waist.\n\r",
		       ch);
	} else {
	  send_to_char("OK.\n\r", ch);
	  perform_wear(ch, obj_object, keyword);
	  obj_from_char(obj_object);
	  equip_char(ch, obj_object, WEAR_WAISTE);
	}
      } else {
	send_to_char("You can't wear that about your waist.\n\r", ch);
      }
    }
    break;
  case 11:{
      if (CAN_WEAR(obj_object, ITEM_WEAR_WRIST)) {
	if ((ch->equipment[WEAR_WRIST_L]) && (ch->equipment[WEAR_WRIST_R])) {
	  send_to_char(
			"You already wear something around both your wrists.\n\r", ch);
	} else {
	  perform_wear(ch, obj_object, keyword);
	  obj_from_char(obj_object);
	  if (ch->equipment[WEAR_WRIST_L]) {
	    sprintf(buffer, "You wear the %s around your right wrist.\n\r", obj_object->short_description);
	    send_to_char(buffer, ch);
	    equip_char(ch, obj_object, WEAR_WRIST_R);
	  } else {
	    sprintf(buffer, "You wear the %s around your left wrist.\n\r", obj_object->short_description);
	    send_to_char(buffer, ch);
	    equip_char(ch, obj_object, WEAR_WRIST_L);
	  }
	}
      } else {
	send_to_char("You can't wear that around your wrist.\n\r", ch);
      }
    }
    break;

  case 12:
    if (CAN_WEAR(obj_object, ITEM_WIELD)) {
      if (ch->equipment[WIELD] || ch->equipment[WIELD_TWOH]) {
	send_to_char("You are already wielding something.\n\r", ch);
      } else {
	if (GET_OBJ_WEIGHT(obj_object) > str_app[STRENGTH_APPLY_INDEX(ch)].wield_w) {
	  send_to_char("It is too heavy for you to use.\n\r", ch);
	} else {
	  send_to_char("OK.\n\r", ch);
	  perform_wear(ch, obj_object, keyword);
	  obj_from_char(obj_object);
	  equip_char(ch, obj_object, WIELD);
	}
      }
    } else {
      send_to_char("You can't wield that.\n\r", ch);
    }
    break;

  case 13:
    if (CAN_WEAR(obj_object, ITEM_HOLD)) {
      if (ch->equipment[HOLD]) {
	send_to_char("You are already holding something.\n\r", ch);
      } else if (ch->equipment[WIELD_TWOH]) {
	send_to_char("You are wielding a two handed blade, you can't hold things!\n\r", ch);
      } else if (ch->equipment[WEAR_SHIELD]) {
	send_to_char("Your hands are full already, you .\n\r", ch);
      } else {
	send_to_char("OK.\n\r", ch);
	perform_wear(ch, obj_object, keyword);
	obj_from_char(obj_object);
	equip_char(ch, obj_object, HOLD);
      }
    } else {
      send_to_char("You can't hold this.\n\r", ch);
    }
    break;
  case 14:
    {
      if (CAN_WEAR(obj_object, ITEM_WEAR_SHIELD)) {
	if ((ch->equipment[WEAR_SHIELD])) {
	  send_to_char("You are already using a shield\n\r", ch);
	} else if (ch->equipment[WIELD_TWOH]) {
	  send_to_char("You can not use a shield while wielding a two handed weapon!\n\r", ch);
	} else if (ch->equipment[HOLD]) {
	  send_to_char("Your hands are full already, you are holding something.\n\r", ch);
	} else {
	  perform_wear(ch, obj_object, keyword);
	  sprintf(buffer, "You start using the %s.\n\r", obj_object->short_description);
	  send_to_char(buffer, ch);
	  obj_from_char(obj_object);
	  equip_char(ch, obj_object, WEAR_SHIELD);
	}
      } else {
	send_to_char("You can't use that as a shield.\n\r", ch);
      }
    }
    break;

  case 15:
    if (CAN_WEAR(obj_object, ITEM_WIELD_TWOH)) {
      if ((ch->equipment[WIELD]) || (ch->equipment[WIELD_TWOH])) {
	send_to_char("You are already wielding something.\n\r", ch);
      } else if (ch->equipment[WEAR_SHIELD]) {
	send_to_char("You can not wield two handed weapons and use a shield!\n\r", ch);
      } else if (ch->equipment[HOLD]) {
	send_to_char("But you are holding something!\n\r", ch);
      } else {
	if (GET_OBJ_WEIGHT(obj_object) > str_app[STRENGTH_APPLY_INDEX(ch)].wield_w) {
	  send_to_char("It is too heavy for you to use.\n\r", ch);
	} else {
	  send_to_char("OK.\n\r", ch);
	  perform_wear(ch, obj_object, keyword);
	  obj_from_char(obj_object);
	  equip_char(ch, obj_object, WIELD_TWOH);
	}
      }
    } else {
      send_to_char("You can't wield that two handed.\n\r", ch);
    }
    break;

  case -1:{
      sprintf(buffer, "Wear %s where?.\n\r", obj_object->short_description);
      send_to_char(buffer, ch);
    }
    break;
  case -2:{
      sprintf(buffer, "You can't wear %s.\n\r", obj_object->short_description);
      send_to_char(buffer, ch);
    }
    break;
  default:{
      log("Unknown type called in wear.");
    }
    break;
  }
}

void do_wear(struct char_data *ch, char *argument, int cmd)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[256];
  char buffer[MAX_INPUT_LENGTH];
  struct obj_data *obj_object, *next_obj;
  int keyword;
  static char *keywords[] =
  {
    "finger",
    "neck",
    "body",
    "head",
    "legs",
    "feet",
    "hands",
    "arms",
    "about",
    "waist",
    "wrist",
    "shield",
    "\n"
  };

  if (DEBUG)
    dlog("do_wear");
  argument_interpreter(argument, arg1, arg2);
  if (*arg1) {
    if (!strcmp(arg1, "all")) {
      for (obj_object = ch->carrying; obj_object; obj_object = next_obj) {
	next_obj = obj_object->next_content;
	keyword = -2;
	if (CAN_WEAR(obj_object, ITEM_WEAR_SHIELD))
	  keyword = 14;
	if (CAN_WEAR(obj_object, ITEM_WEAR_FINGER))
	  keyword = 1;
	if (CAN_WEAR(obj_object, ITEM_WEAR_NECK))
	  keyword = 2;
	if (CAN_WEAR(obj_object, ITEM_WEAR_WRIST))
	  keyword = 11;
	if (CAN_WEAR(obj_object, ITEM_WEAR_WAISTE))
	  keyword = 10;
	if (CAN_WEAR(obj_object, ITEM_WEAR_ARMS))
	  keyword = 8;
	if (CAN_WEAR(obj_object, ITEM_WEAR_HANDS))
	  keyword = 7;
	if (CAN_WEAR(obj_object, ITEM_WEAR_FEET))
	  keyword = 6;
	if (CAN_WEAR(obj_object, ITEM_WEAR_LEGS))
	  keyword = 5;
	if (CAN_WEAR(obj_object, ITEM_WEAR_ABOUT))
	  keyword = 9;
	if (CAN_WEAR(obj_object, ITEM_WEAR_HEAD))
	  keyword = 4;
	if (CAN_WEAR(obj_object, ITEM_WEAR_BODY))
	  keyword = 3;
/*        if (CAN_WEAR(obj_object,ITEM_WIELD)) keyword = 12;  */
	if (CAN_WEAR(obj_object, ITEM_HOLD))
	  keyword = 13;
	if (keyword != -2) {
	  sprintf(buf, "%s :", obj_object->short_description);
	  send_to_char(buf, ch);
	  wear(ch, obj_object, keyword);
	}
      }
    } else {
      obj_object = get_obj_in_list_vis(ch, arg1, ch->carrying);
      if (obj_object) {
	if (*arg2) {
	  keyword = search_block(arg2, keywords, FALSE);	/* Partial Match */
	  if (keyword == -1) {
	    sprintf(buf, "%s is an unknown body location.\n\r", arg2);
	    send_to_char(buf, ch);
	  } else {
	    wear(ch, obj_object, keyword + 1);
	  }
	} else {
	  keyword = -2;
	  if (CAN_WEAR(obj_object, ITEM_WEAR_SHIELD))
	    keyword = 14;
	  if (CAN_WEAR(obj_object, ITEM_WEAR_FINGER))
	    keyword = 1;
	  if (CAN_WEAR(obj_object, ITEM_WEAR_NECK))
	    keyword = 2;
	  if (CAN_WEAR(obj_object, ITEM_WEAR_WRIST))
	    keyword = 11;
	  if (CAN_WEAR(obj_object, ITEM_WEAR_WAISTE))
	    keyword = 10;
	  if (CAN_WEAR(obj_object, ITEM_WEAR_ARMS))
	    keyword = 8;
	  if (CAN_WEAR(obj_object, ITEM_WEAR_HANDS))
	    keyword = 7;
	  if (CAN_WEAR(obj_object, ITEM_WEAR_FEET))
	    keyword = 6;
	  if (CAN_WEAR(obj_object, ITEM_WEAR_LEGS))
	    keyword = 5;
	  if (CAN_WEAR(obj_object, ITEM_WEAR_ABOUT))
	    keyword = 9;
	  if (CAN_WEAR(obj_object, ITEM_WEAR_HEAD))
	    keyword = 4;
	  if (CAN_WEAR(obj_object, ITEM_WEAR_BODY))
	    keyword = 3;
	  wear(ch, obj_object, keyword);
	}
      } else {
	sprintf(buffer, "You do not seem to have the '%s'.\n\r", arg1);
	send_to_char(buffer, ch);
      }
    }
  } else {
    send_to_char("Wear what?\n\r", ch);
  }
}

void do_wield(struct char_data *ch, char *argument, int cmd)
{
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char buffer[MAX_STRING_LENGTH];
  struct obj_data *obj_object;
  int keyword = 12;

  if (DEBUG)
    dlog("do_wield");
  argument_interpreter(argument, arg1, arg2);
  if (*arg1) {
    if (!strncmp("two", arg1, 3)) {
      if (*arg2) {
	obj_object = get_obj_in_list_vis(ch, arg2, ch->carrying);
	if (obj_object) {
	  if (CAN_WEAR(obj_object, ITEM_WIELD_TWOH)) {
	    keyword = 15;
	    wear(ch, obj_object, keyword);
	  } else {
	    send_to_char("That is not a two handed weapon!\n\r", ch);
	  }
	} else {
	  sprintf(buffer, "You do not seem to have the '%s'.\n\r", arg2);
	  send_to_char(buffer, ch);
	}
      } else {			       /* no arg2, check if they can wield it one handed, arg1 */
	obj_object = get_obj_in_list_vis(ch, arg1, ch->carrying);
	if (obj_object) {
	  keyword = 12;
	  wear(ch, obj_object, keyword);
	} else {
	  sprintf(buffer, "You do not seem to have the '%s'.\n\r", arg1);
	  send_to_char(buffer, ch);
	}
      }
    } else {
      obj_object = get_obj_in_list_vis(ch, arg1, ch->carrying);
      if (obj_object) {
	keyword = 12;
	wear(ch, obj_object, keyword);
      } else {
	sprintf(buffer, "You do not seem to have the '%s'.\n\r", arg1);
	send_to_char(buffer, ch);
      }
    }
  } else {
    send_to_char("Wield what?\n\r", ch);
  }
}

void do_grab(struct char_data *ch, char *argument, int cmd)
{
  char arg1[128];
  char arg2[128];
  char buffer[256];
  struct obj_data *obj_object;

  if (DEBUG)
    dlog("do_grab");
  argument_interpreter(argument, arg1, arg2);

  if (*arg1) {
    obj_object = get_obj_in_list(arg1, ch->carrying);
    if (obj_object) {
      if (obj_object->obj_flags.type_flag == ITEM_LIGHT)
	wear(ch, obj_object, WEAR_LIGHT);
      else
	wear(ch, obj_object, 13);
    } else {
      sprintf(buffer, "You do not seem to have the '%s'.\n\r", arg1);
      send_to_char(buffer, ch);
    }
  } else {
    send_to_char("Hold what?\n\r", ch);
  }
}

void do_remove(struct char_data *ch, char *argument, int cmd)
{
  char arg1[128], *T, *P;
  char buffer[256];
  int Rem_List[20], Num_Equip;
  struct obj_data *obj_object;
  int j;

  if (DEBUG)
    dlog("do_remove");
  one_argument(argument, arg1);

  if (*arg1) {
    if (!strcmp(arg1, "all")) {
      for (j = 0; j < MAX_WEAR; j++) {
	if (CAN_CARRY_N(ch) != IS_CARRYING_N(ch)) {
	  if (ch->equipment[j]) {
	    if ((obj_object = unequip_char(ch, j)) != NULL) {
	      obj_to_char(obj_object, ch);

	      if (obj_object->obj_flags.type_flag == ITEM_LIGHT)
		if (obj_object->obj_flags.value[2])
		  real_roomp(ch->in_room)->light--;

	      act("You stop using $p.", FALSE, ch, obj_object, 0, TO_CHAR);
	      act("$n stops using $p.", TRUE, ch, obj_object, 0, TO_ROOM);
	    }
	  }
	} else {
	  send_to_char("You can't carry any more stuff.\n\r", ch);
	  j = MAX_WEAR;
	}
      }
    }
    if (isdigit(arg1[0])) {	       /* Make a list of item numbers for stuff to remove */

      for (Num_Equip = j = 0; j < MAX_WEAR; j++) {
	if (CAN_CARRY_N(ch) != IS_CARRYING_N(ch)) {
	  if (ch->equipment[j])
	    Rem_List[Num_Equip++] = j;
	}
      }

      T = arg1;

      while (isdigit(*T) && (*T != '\0')) {
	P = T;
	if (strchr(T, ',')) {
	  P = strchr(T, ',');
	  *P = '\0';
	}
	if (atoi(T) > 0 && atoi(T) <= Num_Equip) {
	  if (CAN_CARRY_N(ch) != IS_CARRYING_N(ch)) {
	    j = Rem_List[atoi(T) - 1];
	    if (ch->equipment[j]) {
	      if ((obj_object = unequip_char(ch, j)) != NULL) {
		obj_to_char(obj_object, ch);
		if (obj_object->obj_flags.type_flag == ITEM_LIGHT)
		  if (obj_object->obj_flags.value[2])
		    real_roomp(ch->in_room)->light--;

		act("You stop using $p.", FALSE, ch, obj_object, 0, TO_CHAR);
		act("$n stops using $p.", TRUE, ch, obj_object, 0, TO_ROOM);
	      }
	    }
	  } else {
	    send_to_char("You can't carry any more stuff.\n\r", ch);
	    j = MAX_WEAR;
	  }
	} else {
	  sprintf(buffer, "You dont seem to have the %s\n\r", T);
	  send_to_char(buffer, ch);
	}

	if (T != P)
	  T = P + 1;
	else
	  *T = '\0';
      }
    } else {
      obj_object = get_object_in_equip_vis(ch, arg1, ch->equipment, &j);
      if (obj_object) {
	if (CAN_CARRY_N(ch) != IS_CARRYING_N(ch)) {
	  obj_to_char(unequip_char(ch, j), ch);

	  if (obj_object->obj_flags.type_flag == ITEM_LIGHT)
	    if (obj_object->obj_flags.value[2])
	      real_roomp(ch->in_room)->light--;

	  act("You stop using $p.", FALSE, ch, obj_object, 0, TO_CHAR);
	  act("$n stops using $p.", TRUE, ch, obj_object, 0, TO_ROOM);

	} else {
	  send_to_char("You can't carry that many items.\n\r", ch);
	}
      } else {
	send_to_char("You are not using it.\n\r", ch);
      }
    }
  } else {
    send_to_char("Remove what?\n\r", ch);
  }
}

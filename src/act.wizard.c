/*
 * ************************************************************************
 * *  file: actwiz.c , Implementation of commands.           Part of DIKUMUD *
 * *  Usage : Wizard Commands.                                               *
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
#include "race.h"

/*
 * external vars  
 */

extern struct zone_data         *zone_table;
extern int                       top_of_zone_table;
extern struct hash_header        room_db;
extern struct char_data         *character_list;
extern struct descriptor_data   *descriptor_list;
extern struct title_type         titles[6][ABS_MAX_LVL];
extern struct index_data        *mob_index;
extern struct index_data        *obj_index;
extern int                       top_of_mobt;
extern int                       top_of_objt;
extern struct int_app_type       int_app[26];
extern struct wis_app_type       wis_app[26];
extern int                       DEBUG;
extern int                       DEBUG2;
extern char                     *position_types[];
extern char                     *connected_types[];
extern char                     *room_bits[];
extern char                     *sector_types[];

/*
 * external functs 
 */

void                             set_title(struct char_data *ch);
int                              str_cmp(char *arg1, char *arg2);
struct time_info_data            age(struct char_data *ch);
void                             sprinttype(int type, char *names[], char *result);
void                             sprintbit(unsigned long, char *[], char *);
int                              mana_limit(struct char_data *ch);
int                              hit_limit(struct char_data *ch);
int                              move_limit(struct char_data *ch);
int                              mana_gain(struct char_data *ch);
int                              hit_gain(struct char_data *ch);
int                              move_gain(struct char_data *ch);
void                             RoomSave(struct char_data *ch, int start, int end);
void                             RoomLoad(struct char_data *ch, int start, int end);
void                             do_look(struct char_data *ch, char *argument, int cmd);
void                             update_time_and_weather(void);

void 
do_polymorph(struct char_data *ch, char *argument, int cmdnum)
{

}

void 
do_instazone(struct char_data *ch, char *argument, int cmdnum)
{
  char                             cmd,
                                   c,
                                   buf[80];
  int                              i,
                                   start_room,
                                   end_room,
                                   j,
                                   arg1,
                                   arg2,
                                   arg3;
  struct char_data                *p;
  struct obj_data                 *o;
  struct room_data                *room;
  FILE                            *fp;

  if (IS_NPC(ch))
    return;

  /*
   *   read in parameters (room #s)
   */
  start_room = -1;
  end_room = -1;
  sscanf(argument, "%d%c%d", &start_room, &c, &end_room);

  if ((start_room == -1) || (end_room == -1)) {
    send_to_char("mkzone <start_room> <end_room>\n\r", ch);
    return;
  }
  fp = (FILE *) MakeZoneFile(ch);

  if (!fp) {
    send_to_char("Couldn't make file.. try again later\n\r", ch);
    return;
  }
  for (i = start_room; i <= end_room; i++) {
    room = real_roomp(i);
    if (room) {

      /*
       *  first write out monsters
       */
      for (p = room->people; p; p = p->next_in_room) {
	if (IS_NPC(p)) {
	  cmd = 'M';
	  arg1 = MobVnum(p);
	  arg2 = mob_index[p->nr].number;
	  arg3 = i;
	  Zwrite(fp, cmd, 0, arg1, arg2, arg3, p->player.short_descr);
	  for (j = 0; j < MAX_WEAR; j++) {
	    if (p->equipment[j]) {
	      if (p->equipment[j]->item_number >= 0) {
		cmd = 'E';
		arg1 = ObjVnum(p->equipment[j]);
		arg2 = obj_index[p->equipment[j]->item_number].number;
		arg3 = j;
		strcpy(buf, p->equipment[j]->short_description);
		Zwrite(fp, cmd, 1, arg1, arg2, arg3,
		       buf);
		RecZwriteObj(fp, p->equipment[j]);
	      }
	    }
	  }
	  for (o = p->carrying; o; o = o->next_content) {
	    if (o->item_number >= 0) {
	      cmd = 'G';
	      arg1 = ObjVnum(o);
	      arg2 = obj_index[o->item_number].number;
	      arg3 = 0;
	      strcpy(buf, o->short_description);
	      Zwrite(fp, cmd, 1, arg1, arg2, arg3, buf);
	      RecZwriteObj(fp, o);
	    }
	  }
	}
      }
      /*
       *  write out objects in rooms
       */
      for (o = room->contents; o; o = o->next_content) {
	if (o->item_number >= 0) {
	  cmd = 'O';
	  arg1 = ObjVnum(o);
	  arg2 = obj_index[o->item_number].number;
	  arg3 = i;
	  strcpy(buf, o->short_description);
	  Zwrite(fp, cmd, 0, arg1, arg2, arg3, buf);
	  RecZwriteObj(fp, o);
	}
      }
      /*
       *  lastly.. doors
       */

      for (j = 0; j < 6; j++) {
	/*
	 *  if there is an door type exit, write it.
	 */
	if (room->dir_option[j]) {	/*
					 * is a door 
					 */
	  if (room->dir_option[j]->exit_info) {
	    cmd = 'D';
	    arg1 = i;
	    arg2 = j;
	    arg3 = 0;
	    if (IS_SET(room->dir_option[j]->exit_info, EX_CLOSED)) {
	      arg3 = 1;
	    }
	    if (IS_SET(room->dir_option[j]->exit_info, EX_LOCKED)) {
	      arg3 = 2;
	    }
	    Zwrite(fp, cmd, 0, arg1, arg2, arg3, room->name);
	  }
	}
      }
    }
  }
  fclose(fp);

}

do_highfive(struct char_data *ch, char *argument, int cmd)
{
  char                             buf[80];
  char                             mess[120];
  struct char_data                *tch;

  if (argument) {
    only_argument(argument, buf);
    if ((tch = get_char_vis(ch, buf)) != 0) {
      if ((GetMaxLevel(tch) >= DEMIGOD) && (!IS_NPC(tch))) {
	sprintf(mess, "Time stops for a moment as %s and %s high five.\n\r",
		ch->player.name, tch->player.name);
	send_to_all(mess);
      } else {
	act("$n gives you a high five", TRUE, ch, 0, tch, TO_VICT);
	act("You give a hearty high five to $N", TRUE, ch, 0, tch, TO_CHAR);
	act("$n and $N do a high five.", TRUE, ch, 0, tch, TO_NOTVICT);
      }
    } else {
      sprintf(buf, "I don't see anyone here like that.\n\r");
      send_to_char(buf, ch);
    }
  }
}

do_wizlock(struct char_data *ch, char *argument, int cmd)
{

  extern int                       WizLock;

  if (IS_NPC(ch)) {
    send_to_char("You cannot WizLock.\n\r", ch);
    return (FALSE);
  }
  if (WizLock) {
    send_to_char("WizLock is now off\n\r", ch);
    log("Wizlock is now off.");
    WizLock = FALSE;
  } else {
    send_to_char("WizLock is now on\n\r", ch);
    log("WizLock is now on.");
    WizLock = TRUE;
  }

  return;

}

do_rload(struct char_data * ch, char *argument, int cmd)
{
  char                             i;
  int                              start = -1,
                                   end = -2;

  if (IS_NPC(ch))
    return;
  if (GetMaxLevel(ch) < IMMORTAL)
    return;

  for (i = 0; *(argument + i) == ' '; i++);
  if (!*(argument + i)) {
    send_to_char("lroom <start> <end>\n\r", ch);
    return;
  }
  sscanf(argument, "%d %d", &start, &end);

  if ((start <= end) && (start != -1) && (end != -2)) {
    RoomLoad(ch, start, end);
  }
}

do_rsave(struct char_data *ch, char *argument, int cmd)
{
  char                             i,
                                   buf[256];
  int                              start = -1,
                                   end = -2;

  if (IS_NPC(ch))
    return;

  if (GetMaxLevel(ch) < IMMORTAL)
    return;

  for (i = 0; *(argument + i) == ' '; i++);
  if (!*(argument + i)) {
    send_to_char("sroom <start> <end>\n\r", ch);
    return;
  }
  sscanf(argument, "%d %d", &start, &end);

  if ((start <= end) && (start != -1) && (end != -2)) {
    sprintf(buf, "mv %s %s.bak", GET_NAME(ch), GET_NAME(ch));
    system(buf);
    RoomSave(ch, start, end);
  }
}

do_emote(struct char_data *ch, char *argument, int cmd)
{
  int                              i;
  char                             buf[MAX_INPUT_LENGTH];

  if (IS_NPC(ch) && (cmd != 0))
    return;

  for (i = 0; *(argument + i) == ' '; i++);

  if (!*(argument + i))
    send_to_char("Yes.. But what?\n\r", ch);
  else {
    sprintf(buf, "$n %s", argument + i);
    act(buf, FALSE, ch, 0, 0, TO_ROOM);
    send_to_char("Ok.\n\r", ch);
  }
}

void 
do_echo(struct char_data *ch, char *argument, int cmd)
{
  int                              i;
  char                             buf[MAX_INPUT_LENGTH];

  if (IS_NPC(ch))
    return;

  for (i = 0; *(argument + i) == ' '; i++);

  if (!*(argument + i)) {
    if (IS_SET(ch->specials.act, PLR_ECHO)) {
      send_to_char("echo off\n\r", ch);
      REMOVE_BIT(ch->specials.act, PLR_ECHO);
    } else {
      SET_BIT(ch->specials.act, PLR_ECHO);
      send_to_char("echo on\n\r", ch);
    }
  } else {
    if (IS_IMMORTAL(ch)) {
      sprintf(buf, "%s\n\r", argument + i);
      send_to_room_except(buf, ch->in_room, ch);
      send_to_char("Ok.\n\r", ch);
    }
  }
}

void 
do_system(struct char_data *ch, char *argument, int cmd)
{
  int                              i;
  char                             buf[256];

  if (IS_NPC(ch))
    return;

  for (i = 0; *(argument + i) == ' '; i++);

  if (!*(argument + i))
    send_to_char("That must be a mistake...\n\r", ch);
  else {
    sprintf(buf, "\n\r%s\n\r", argument + i);
    send_to_all(buf);
  }
}

void 
do_trans(struct char_data *ch, char *argument, int cmd)
{
  struct descriptor_data          *i;
  struct char_data                *victim;
  char                             buf[100];
  sh_int                           target;

  if (IS_NPC(ch))
    return;

  only_argument(argument, buf);
  if (!*buf)
    send_to_char("Who do you wich to transfer?\n\r", ch);
  else if (str_cmp("all", buf)) {
    if (!(victim = get_char_vis_world(ch, buf, NULL)))
      send_to_char("No-one by that name around.\n\r", ch);
    else {
      act("$n disappears in a mushroom cloud.", FALSE, victim, 0, 0, TO_ROOM);
      target = ch->in_room;
      if (MOUNTED(victim)) {
	char_from_room(victim);
	char_from_room(MOUNTED(victim));
	char_to_room(victim, target);
	char_to_room(MOUNTED(victim), target);
      } else {
	char_from_room(victim);
	char_to_room(victim, target);
      }
      act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
      act("$n has transferred you!", FALSE, ch, 0, victim, TO_VICT);
      do_look(victim, "", 15);
      send_to_char("Ok.\n\r", ch);
    }
  } else {			/*
				 * Trans All 
				 */
    for (i = descriptor_list; i; i = i->next)
      if (i->character != ch && !i->connected) {
	victim = i->character;
	if (MOUNTED(victim))
	  Dismount(victim, MOUNTED(victim), POSITION_STANDING);
	target = ch->in_room;
	char_from_room(victim);
	char_to_room(victim, target);
	act("$n disappears in a mushroom cloud.", FALSE, victim, 0, 0, TO_ROOM);
	act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
	act("$n has transferred you!", FALSE, ch, 0, victim, TO_VICT);
	do_look(victim, "", 15);
      }
    send_to_char("Ok.\n\r", ch);
  }
}

void 
do_at(struct char_data *ch, char *argument, int cmd)
{
  char                             command[MAX_INPUT_LENGTH],
                                   loc_str[MAX_INPUT_LENGTH];
  int                              loc_nr,
                                   location,
                                   original_loc;
  struct char_data                *target_mob;
  struct obj_data                 *target_obj;

  if (IS_NPC(ch))
    return;

  half_chop(argument, loc_str, command);
  if (!*loc_str) {
    send_to_char("You must supply a room number or a name.\n\r", ch);
    return;
  }
  if (isdigit(*loc_str)) {
    loc_nr = atoi(loc_str);
    if (NULL == real_roomp(loc_nr)) {
      send_to_char("No room exists with that number.\n\r", ch);
      return;
    }
    location = loc_nr;
  } else if (target_mob = get_char_vis(ch, loc_str)) {
    location = target_mob->in_room;
  } else if (target_obj = get_obj_vis_world(ch, loc_str, NULL))
    if (target_obj->in_room != NOWHERE)
      location = target_obj->in_room;
    else {
      send_to_char("The object is not available.\n\r", ch);
      return;
  } else {
    send_to_char("No such creature or object around.\n\r", ch);
    return;
  }

  /*
   * a location has been found. 
   */

  original_loc = ch->in_room;
  char_from_room(ch);
  char_to_room(ch, location);
  command_interpreter(ch, command);

  /*
   * check if the guy's still there 
   */
  for (target_mob = real_roomp(location)->people; target_mob; target_mob =
       target_mob->next_in_room)
    if (ch == target_mob) {
      char_from_room(ch);
      char_to_room(ch, original_loc);
    }
}

void 
do_form(struct char_data *ch, char *argument, int cmd)
{
  char                             buf[MAX_INPUT_LENGTH];
  int                              loc_nr,
                                   location,
                                   i;
  struct char_data                *target_mob,
                                  *pers,
                                  *v;
  struct obj_data                 *target_obj;
  extern int                       top_of_world;
  struct room_data                *rp;
  int                              zone;

  if (IS_NPC(ch))
    return;
  only_argument(argument, buf);
  if (!*buf) {
    send_to_char("Usage: FORM virtual_number.\n\r", ch);
    return;
  }
  if (!(isdigit(*buf))) {
    send_to_char("Usage: FORM virtual_number.\n\r", ch);
    return;
  }
  loc_nr = atoi(buf);
  if (real_roomp(loc_nr)) {
    send_to_char("A room exists with that Vnum!\n\r", ch);
    return;
  } else if (loc_nr < 0) {
    send_to_char("You must use a positive Vnum!\n\r", ch);
    return;
  }
  send_to_char("You have formed a room.\n\r", ch);
  allocate_room(loc_nr);
  rp = real_roomp(loc_nr);
  bzero(rp, sizeof(*rp));
  rp->number = loc_nr;
  if (top_of_zone_table >= 0) {
    for (zone = 0; rp->number > zone_table[zone].top && zone <= top_of_zone_table; zone++);
    if (zone > top_of_zone_table) {
      fprintf(stderr, "Room %d is outside of any zone.\n", rp->number);
      zone--;
    }
    rp->zone = zone;
  }
  sprintf(buf, "%d", loc_nr);
  rp->name = (char *)strdup(buf);
  rp->description = (char *)strdup("New Room\n");
}

void 
do_goto(struct char_data *ch, char *argument, int cmd)
{
  char                             buf[MAX_INPUT_LENGTH];
  int                              loc_nr,
                                   location,
                                   i;
  struct char_data                *target_mob,
                                  *pers,
                                  *v;
  struct obj_data                 *target_obj;
  extern int                       top_of_world;
  struct room_data                *rp;

  if (IS_NPC(ch))
    return;

  only_argument(argument, buf);
  if (!*buf) {
    send_to_char("You must supply a room number or a name.\n\r", ch);
    return;
  }
  if (isdigit(*buf) && NULL == index(buf, '.')) {
    loc_nr = atoi(buf);
    if (NULL == real_roomp(loc_nr)) {
      send_to_char("No room exists with that number.\n\r", ch);
      return;
    }
    location = loc_nr;
  } else if (target_mob = get_char_vis_world(ch, buf, NULL))
    location = target_mob->in_room;
  else if (target_obj = get_obj_vis_world(ch, buf, NULL))
    if (target_obj->in_room != NOWHERE)
      location = target_obj->in_room;
    else {
      send_to_char("The object is not available.\n\r", ch);
      send_to_char("Try where #.object to nail its room number.\n\r", ch);
      return;
  } else {
    send_to_char("No such creature or object around.\n\r", ch);
    return;
  }

  /*
   * a location has been found. 
   */

  if (!real_roomp(location)) {
    log("Massive error in do_goto. Everyone Off NOW.");
    return;
  }
  if (IS_SET(real_roomp(location)->room_flags, PRIVATE)) {
    for (i = 0, pers = real_roomp(location)->people; pers; pers =
	 pers->next_in_room, i++);
    if (i > 1) {
      send_to_char(
		    "There's a private conversation going on in that room.\n\r", ch);
      return;
    }
  }
  if (IS_SET(ch->specials.act, PLR_STEALTH)) {
    for (v = real_roomp(ch->in_room)->people; v; v = v->next_in_room) {
      if ((ch != v) && (GetMaxLevel(v) >= GetMaxLevel(ch))) {
	act("$n disappears into a cloud of mist.", FALSE, ch, 0, v, TO_VICT);
      }
    }
  } else {
    act("$n disappears into a cloud of mist.", FALSE, ch, 0, 0, TO_ROOM);
  }

  if (ch->specials.fighting)
    stop_fighting(ch);
  if (MOUNTED(ch)) {
    char_from_room(ch);
    char_to_room(ch, location);
    char_from_room(MOUNTED(ch));
    char_to_room(MOUNTED(ch), location);
  } else {
    char_from_room(ch);
    char_to_room(ch, location);
  }

  if (IS_SET(ch->specials.act, PLR_STEALTH)) {
    for (v = real_roomp(ch->in_room)->people; v; v = v->next_in_room) {
      if ((ch != v) && (GetMaxLevel(v) >= GetMaxLevel(ch))) {
	act("$n appears from a cloud of mist.", FALSE, ch, 0, v, TO_VICT);
      }
    }
  } else {
    act("$n appears from a cloud of mist.", FALSE, ch, 0, 0, TO_ROOM);
  }
  do_look(ch, "", 15);
}

void 
do_apraise(struct char_data *ch, char *argument, int cmd)
{
  extern char                     *spells[];
  struct affected_type            *aff;
  char                             arg1[MAX_STRING_LENGTH];
  char                             buf[MAX_STRING_LENGTH];
  char                             buf2[MAX_STRING_LENGTH];
  struct room_data                *rm = 0;
  struct char_data                *k = 0;
  struct obj_data                 *j = 0;
  struct obj_data                 *j2 = 0;
  struct extra_descr_data         *desc;
  struct follow_type              *fol;
  int                              i,
                                   virtual;
  int                              i2,
                                   count;
  bool                             found;
  int                              found_one;
  int                              chance;

  /*
   * for objects 
   */
  extern char                     *item_types[];
  extern char                     *wear_bits[];
  extern char                     *extra_bits[];
  extern char                     *drinks[];

  /*
   * for rooms 
   */
  extern char                     *dirs[];
  extern char                     *room_bits[];
  extern char                     *exit_bits[];
  extern char                     *sector_types[];

  /*
   * for chars 
   */
  extern char                     *equipment_types[];
  extern char                     *affected_bits[];
  extern char                     *immunity_names[];
  extern char                     *apply_types[];
  extern char                     *pc_class_types[];
  extern char                     *npc_class_types[];
  extern char                     *action_bits[];
  extern char                     *player_bits[];
  extern char                     *position_types[];
  extern char                     *connected_types[];

  if (IS_NPC(ch))
    return;

  found_one = 0;

  only_argument(argument, arg1);

  /*
   * no argument 
   */

  if (GET_MANA(ch) < 3) {
    send_to_char("You can't seem to concentrate enough at the moment.\n\r", ch);
    return;
  }
  chance = number(1, 101);

  if (chance > ch->skills[SKILL_APRAISE].learned) {
    send_to_char("You are unable to apraise this item.\n\r", ch);
    GET_MANA(ch) -= 1;
    return;
  }
  GET_MANA(ch) -= 3;

  if (!*arg1) {
    send_to_char("apraise what?\n\r", ch);
    return;
  } else {
    if (ch->skills[SKILL_APRAISE].learned < 50)
      ch->skills[SKILL_APRAISE].learned = MIN(50, ch->skills[SKILL_APRAISE].learned++);

    /*
     * apraise on object 
     */
    if (j = (struct obj_data *)get_obj_in_list_vis(ch, arg1, ch->carrying)) {
      sprintf(buf, "Object name: [%s]\n\rItem type: ", j->name);
      sprinttype(GET_ITEM_TYPE(j), item_types, buf2);
      strcat(buf, buf2);
      strcat(buf, "\n\r");
      send_to_char(buf, ch);

      send_to_char("Can be worn on :", ch);
      sprintbit(j->obj_flags.wear_flags, wear_bits, buf);
      strcat(buf, "\n\r");
      send_to_char(buf, ch);

      sprintf(buf, "Weight: %d, Value: %d, Cost/day: %d\n\r",
	      j->obj_flags.weight, j->obj_flags.cost,
	      j->obj_flags.cost_per_day);
      send_to_char(buf, ch);

      switch (j->obj_flags.type_flag) {
      case ITEM_LIGHT:
	sprintf(buf, "Light hours of Use : [%d]", j->obj_flags.value[2]);
	send_to_char(buf, ch);
	break;
      case ITEM_WEAPON:
	sprintf(buf, "Weapon Class:");
	switch (j->obj_flags.value[3]) {
	case 0:
	  strcat(buf, "Smiting Class.\n\r");
	  break;
	case 1:
	  strcat(buf, "Stabbing Class.\n\r");
	  break;
	case 2:
	  strcat(buf, "Whipping Class.\n\r");
	  break;
	case 3:
	  strcat(buf, "Slashing Class.\n\r");
	  break;
	case 4:
	  strcat(buf, "Smashing Class.\n\r");
	  break;
	case 5:
	  strcat(buf, "Cleaving Class.\n\r");
	  break;
	case 6:
	  strcat(buf, "Crushing Class.\n\r");
	  break;
	case 7:
	  strcat(buf, "Bludgeoning Class.\n\r");
	  break;
	case 11:
	  strcat(buf, "Piercing Class.\n\r");
	  break;
	default:
	  strcat(buf, "Foreign Class to you....\n\r");
	  break;
	}

	found_one = 0;

	for (i = 0; i < MAX_OBJ_AFFECT; i++) {
	  if (j->affected[i].location == APPLY_HITROLL || j->affected[i].location == APPLY_HITNDAM) {
	    found_one = 1;
	    switch (j->affected[i].modifier) {
	    case 1:
	      strcat(buf, "It is well balanced.\n\r");
	      break;
	    case 2:
	      strcat(buf, "It is very well balanced.\n\r");
	      break;
	    case 3:
	      strcat(buf, "It is a superb weapon.\n\r");
	      break;
	    case 4:
	      strcat(buf, "It was forged by the gods.\n\r");
	      break;
	    case 5:
	      strcat(buf, "It should not be in your hands.\n\r");
	      break;
	    default:
	      strcat(buf, "It will crack with the next blow.\n\r");
	      break;
	    }
	  }
	}

	if (!found_one)
	  strcat(buf, "It is common in accuracy.\n\r");

	found_one = 0;
	for (i = 0; i < MAX_OBJ_AFFECT; i++) {
	  if (j->affected[i].location == APPLY_DAMROLL || j->affected[i].location == APPLY_HITNDAM) {
	    found_one = 1;
	    switch (j->affected[i].modifier) {
	    case 0:
	      strcat(buf, "It will surely damage its target.\n\r");
	      break;
	    case 1:
	      strcat(buf, "It looks to be made from a strong metal.\n\r");
	      break;
	    case 2:
	      strcat(buf, "This was forged from a mystical flame.\n\r");
	      break;
	    case 3:
	      strcat(buf, "It has definite magical charms.\n\r");
	      break;
	    case 4:
	      strcat(buf, "This is definitately blessed by the gods.\n\r");
	      break;
	    case 5:
	      strcat(buf, "It is ready to lose its hilt.\n\r");
	      break;
	    default:
	      strcat(buf, "It is checked badly and most likely will break.\n\r");
	      break;
	    }
	  }
	}
	if (!found_one)
	  strcat(buf, "It has a common strength to its making.\n\r");
	send_to_char(buf, ch);

	break;
      case ITEM_ARMOR:
	sprintf(buf, "Effective AC points: [%d]\n\rWhen Repaired: [%d]",
		j->obj_flags.value[0],
		j->obj_flags.value[1]);
	if (j->obj_flags.value[0] != 0 && j->obj_flags.value[1] == 0) {
	  strcat(buf, "\n\rYou should take it to be updated at the Blacksmith\n\r");
	}
	send_to_char(buf, ch);
	break;

      }
    } else {
      send_to_char("I don't see that here.\n\r", ch);
    }
  }
}

void 
do_stat(struct char_data *ch, char *argument, int cmd)
{
  extern char                     *spells[];
  struct affected_type            *aff;
  char                             arg1[MAX_STRING_LENGTH];
  char                             buf[MAX_STRING_LENGTH];
  char                             buf2[MAX_STRING_LENGTH];
  struct room_data                *rm = 0;
  struct char_data                *k = 0;
  struct obj_data                 *j = 0;
  struct obj_data                 *j2 = 0;
  struct extra_descr_data         *desc;
  struct follow_type              *fol;
  int                              i,
                                   virtual;
  int                              i2,
                                   count;
  bool                             found;

  /*
   * for objects 
   */
  extern char                     *item_types[];
  extern char                     *wear_bits[];
  extern char                     *extra_bits[];
  extern char                     *drinks[];

  /*
   * for rooms 
   */
  extern char                     *dirs[];
  extern char                     *room_bits[];
  extern char                     *exit_bits[];
  extern char                     *sector_types[];

  /*
   * for chars 
   */
  extern char                     *equipment_types[];
  extern char                     *affected_bits[];
  extern char                     *immunity_names[];
  extern char                     *apply_types[];
  extern char                     *pc_class_types[];
  extern char                     *npc_class_types[];
  extern char                     *action_bits[];
  extern char                     *player_bits[];

  if (IS_NPC(ch))
    return;

  only_argument(argument, arg1);

  /*
   * no argument 
   */
  if (!*arg1) {
    send_to_char("Usage: stat < char | mobile | object | room >\n\r", ch);
    return;
  } else {
    /*
     * ROOM  
     */
    if (!str_cmp("room", arg1)) {
      rm = real_roomp(ch->in_room);
      sprintf(buf, "Room name: %s   Zone : %d.\n\rV-Number : %d, R-number : %d\n\r",
	      rm->name, rm->zone, rm->number, ch->in_room);
      send_to_char(buf, ch);

      sprinttype(rm->sector_type, sector_types, buf2);
      sprintf(buf, "Sector type : %s\n\r", buf2);
      send_to_char(buf, ch);

      strcpy(buf, "Special procedure : ");
      strcat(buf, (rm->funct) ? "Exists\n\r" : "No\n\r");
      send_to_char(buf, ch);

      send_to_char("Room flags: ", ch);
      sprintbit((long)rm->room_flags, room_bits, buf);
      strcat(buf, "\n\r");
      send_to_char(buf, ch);

      send_to_char("Description:\n\r", ch);
      send_to_char(rm->description, ch);

      strcpy(buf, "-----------------------\n\rExtra description keywords(s): ");
      if (rm->ex_description) {
	strcat(buf, "\n\r");
	for (desc = rm->ex_description; desc; desc = desc->next) {
	  strcat(buf, desc->keyword);
	  strcat(buf, "\n\r");
	}
	strcat(buf, "\n\r");
	send_to_char(buf, ch);
      } else {
	strcat(buf, "None\n\r");
	send_to_char(buf, ch);
      }

      strcpy(buf, "------- Chars present -------\n\r");
      for (k = rm->people; k; k = k->next_in_room) {
	if (CAN_SEE(ch, k)) {
	  strcat(buf, GET_NAME(k));
	  strcat(buf, (!IS_NPC(k) ? "(PC)\n\r" : (!IS_MOB(k) ? "(NPC)\n\r" : "(MOB)\n\r")));
	}
      }
      strcat(buf, "\n\r");
      send_to_char(buf, ch);

      strcpy(buf, "--------- Contents ---------\n\r");
      for (j = rm->contents; j; j = j->next_content) {
	strcat(buf, j->name);
	strcat(buf, "\n\r");
      }
      strcat(buf, "\n\r");
      send_to_char(buf, ch);

      send_to_char("------- Exits defined -------\n\r", ch);
      for (i = 0; i <= 5; i++) {
	if (rm->dir_option[i]) {
	  sprintf(buf, "Direction %s.\n\rKeyword : %s\n\r",
		  dirs[i], rm->dir_option[i]->keyword);
	  send_to_char(buf, ch);
	  strcpy(buf, "Description:\n\r  ");
	  if (rm->dir_option[i]->general_description)
	    strcat(buf, rm->dir_option[i]->general_description);
	  else
	    strcat(buf, "UNDEFINED\n\r");
	  send_to_char(buf, ch);
	  sprintbit(rm->dir_option[i]->exit_info, exit_bits, buf2);
	  sprintf(buf, "Exit flag: %s\n\rKey no: %d\n\rTo room (R-Number): %d\n\r\n\r",
		  buf2, rm->dir_option[i]->key,
		  rm->dir_option[i]->to_room);
	  send_to_char(buf, ch);
	}
      }
      return;
    }
    count = 1;

    /*
     * MOBILE in world 
     */
    if (k = get_char_vis_world(ch, arg1, &count)) {
      sprintf(buf2, "Name: %s  :  [R-Number %d]  ", GET_NAME(k), k->nr);
      if (IS_MOB(k)) {
	sprintf(buf2 + strlen(buf2),
		"[Load Number %d]\n\r", mob_index[k->nr].virtual);
      } else {
	strcat(buf2, "\n\r");
      }
      send_to_char(buf2, ch);
      sprintf(buf2, "Location [%d]\n\r", k->in_room);

      switch (k->player.sex) {
      case SEX_NEUTRAL:
	strcpy(buf, "Neutral-Sex");
	break;
      case SEX_MALE:
	strcpy(buf, "Male");
	break;
      case SEX_FEMALE:
	strcpy(buf, "Female");
	break;
      default:
	strcpy(buf, "ILLEGAL-SEX!!");
	break;
      }

      sprintf(buf2 + strlen(buf2), "Sex : %s - %s\n\r",
	      buf,
	      (!IS_NPC(k) ? "Pc" : (!IS_MOB(k) ? "Npc" : "Mob")));

      send_to_char(buf2, ch);

      strcpy(buf, "Short description: ");
      strcat(buf, (k->player.short_descr ? k->player.short_descr : "None"));
      strcat(buf, "\n\r");
      send_to_char(buf, ch);

      strcpy(buf, "Title: ");
      strcat(buf, (k->player.title ? k->player.title : "None"));
      strcat(buf, "\n\r");
      send_to_char(buf, ch);
      strcpy(buf, "Pre-Title: ");
      strcat(buf, (GET_PRETITLE(k) ? GET_PRETITLE(k) : "None"));
      strcat(buf, "\n\r");
      send_to_char(buf, ch);

      send_to_char("Long description: ", ch);
      if (k->player.long_descr)
	send_to_char(k->player.long_descr, ch);
      else
	send_to_char("None", ch);
      send_to_char("\n\r", ch);

      if (IS_NPC(k)) {
	strcpy(buf, "Monster Class: ");
	sprinttype(k->player.class, npc_class_types, buf2);
      } else {
	strcpy(buf, "Class: ");
	sprintbit(k->player.class, pc_class_types, buf2);
      }
      strcat(buf, buf2);

      sprintf(buf2, " :  Level [%d/%d/%d/%d/%d] : Alignment[%d]\n\r",
	      k->player.level[0], k->player.level[1],
	      k->player.level[2], k->player.level[3],
	      k->player.level[4], GET_ALIGNMENT(k));

      strcat(buf, buf2);
      send_to_char(buf, ch);

      if (IS_PC(k)) {
	sprintf(buf, "Birth : [%ld] secs, Logon[%ld] secs, Played[%ld] secs\n\r",
		k->player.time.birth,
		k->player.time.logon,
		k->player.time.played);
	send_to_char(buf, ch);

	sprintf(buf, "Age: [%d] Years,  [%d] Months,  [%d] Days,  [%d] Hours\n\r",
		age(k).year, age(k).month, age(k).day, age(k).hours);
	send_to_char(buf, ch);
      }
      sprintf(buf, "Height [%d]cm  Weight [%d]pounds \n\r",
	      GET_HEIGHT(k), GET_WEIGHT(k));
      send_to_char(buf, ch);
      strcpy(buf, "+----------------------------+\n\r");
      send_to_char(buf, ch);
      sprintf(buf, "Str:[%d/%d]  Int:[%d]  Wis:[%d]  Dex:[%d]  Con:[%d]\n\r",
	      GET_STR(k), GET_ADD(k),
	      GET_INT(k),
	      GET_WIS(k),
	      GET_DEX(k),
	      GET_CON(k));
      send_to_char(buf, ch);

      sprintf(buf,
	      "Mana p.:[%d/%d+%d]  Hit p.:[%d/%d+%d]  Move p.:[%d/%d+%d]\n\r",
	      GET_MANA(k), mana_limit(k), mana_gain(k),
	      GET_HIT(k), hit_limit(k), hit_gain(k),
	      GET_MOVE(k), move_limit(k), move_gain(k));
      send_to_char(buf, ch);

      sprintf(buf,
	      "AC:[%d/10], Coins: [%d], Exp: [%d], Hitroll: [%d], Damroll: [%d]\n\r",
	      GET_AC(k),
	      GET_GOLD(k),
	      GET_EXP(k),
	      k->points.hitroll,
	      k->points.damroll);
      send_to_char(buf, ch);

      if (IS_NPC(k)) {
	sprintf(buf, "Npc Bare Hand Damage %dd%d.\n\r",
		k->specials.damnodice, k->specials.damsizedice);
	send_to_char(buf, ch);
      }
      if (IS_PC(k)) {
	sprintf(buf, "\n\rTimer [%d] \n\r", k->specials.timer);
	send_to_char(buf, ch);
      }
      strcpy(buf, "+----------------------------+\n\r");
      send_to_char(buf, ch);

      sprinttype(GET_POS(k), position_types, buf2);
      sprintf(buf, "Position: %s : Fighting: %s", buf2,
	      ((k->specials.fighting) ? GET_NAME(k->specials.fighting) : "Nobody"));
      if (k->desc) {
	sprinttype(k->desc->connected, connected_types, buf2);
	strcat(buf, " : Connected: ");
	strcat(buf, buf2);
      }
      strcat(buf, "\n\r");
      send_to_char(buf, ch);

      strcpy(buf, "Default position: ");
      sprinttype((k->specials.default_pos), position_types, buf2);
      strcat(buf, buf2);
      if (IS_NPC(k)) {
	strcat(buf, "\n\rNPC flags: ");
	sprintbit(k->specials.act, action_bits, buf2);
      } else {
	strcat(buf, ",PC flags: ");
	sprintbit(k->specials.act, player_bits, buf2);
      }

      strcat(buf, buf2);

      if (IS_MOB(k)) {
	strcpy(buf, "\n\rMobile Special procedure : ");
	strcat(buf, (mob_index[k->nr].func ? "Exists\n\r" : "None\n\r"));
	send_to_char(buf, ch);
      }
      sprintf(buf, "Carried weight: %d   Carried items: %d\n\r",
	      IS_CARRYING_W(k),
	      IS_CARRYING_N(k));
      send_to_char(buf, ch);

      for (i = 0, j = k->carrying; j; j = j->next_content, i++);
      sprintf(buf, "Items in inventory: %d, ", i);

      for (i = 0, i2 = 0; i < MAX_WEAR; i++)
	if (k->equipment[i])
	  i2++;
      sprintf(buf2, "Items in equipment: %d\n\r", i2);
      strcat(buf, buf2);
      send_to_char(buf, ch);

      sprintf(buf, "Apply saving throws: [%d] [%d] [%d] [%d] [%d]\n\r",
	      k->specials.apply_saving_throw[0],
	      k->specials.apply_saving_throw[1],
	      k->specials.apply_saving_throw[2],
	      k->specials.apply_saving_throw[3],
	      k->specials.apply_saving_throw[4]);
      send_to_char(buf, ch);

      if (IS_PC(k)) {
	sprintf(buf, "Thirst: %d, Hunger: %d, Drunk: %d\n\r",
		k->specials.conditions[THIRST],
		k->specials.conditions[FULL],
		k->specials.conditions[DRUNK]);
	send_to_char(buf, ch);
      }
      sprintf(buf, "Master is '%s'\n\r",
	      ((k->master) ? GET_NAME(k->master) : "NOBODY"));
      send_to_char(buf, ch);
      send_to_char("Followers are:\n\r", ch);
      for (fol = k->followers; fol; fol = fol->next)
	if (CAN_SEE(ch, fol->follower))
	  act("    $N", FALSE, ch, 0, fol->follower, TO_CHAR);

      /*
       * immunities 
       */
      send_to_char("Immune to:", ch);
      sprintbit(k->M_immune, immunity_names, buf);
      strcat(buf, "\n\r");
      send_to_char(buf, ch);
      /*
       * resistances 
       */
      send_to_char("Resistant to:", ch);
      sprintbit(k->immune, immunity_names, buf);
      strcat(buf, "\n\r");
      send_to_char(buf, ch);
      /*
       * Susceptible 
       */
      send_to_char("Susceptible to:", ch);
      sprintbit(k->susc, immunity_names, buf);
      strcat(buf, "\n\r");
      send_to_char(buf, ch);

      /*
       * Showing the bitvector 
       */
      sprintbit(k->specials.affected_by, affected_bits, buf);
      send_to_char("Affected by: ", ch);
      strcat(buf, "\n\r");
      send_to_char(buf, ch);

      /*
       * Routine to show what spells a char is affected by 
       */
      if (k->affected) {
	send_to_char("\n\rAffecting Spells:\n\r--------------\n\r", ch);
	for (aff = k->affected; aff; aff = aff->next) {
	  sprintf(buf, "Spell : '%s'\n\r", spells[aff->type - 1]);
	  send_to_char(buf, ch);
	  sprintf(buf, "     Modifies %s by %d points\n\r", apply_types[aff->location], aff->modifier);
	  send_to_char(buf, ch);
	  sprintf(buf, "     Expires in %3d hours, Bits set ", aff->duration);
	  send_to_char(buf, ch);
	  sprintbit(aff->bitvector, affected_bits, buf);
	  strcat(buf, "\n\r");
	  send_to_char(buf, ch);
	}
      }
      return;
    }
    /*
     * stat on OBJECT 
     */
    if (j = (struct obj_data *)get_obj_vis_world(ch, arg1, &count)) {
      virtual = (j->item_number >= 0) ? obj_index[j->item_number].virtual : 0;
      sprintf(buf, "Object name: [%s]\n\rR-number: [%d], Load Number: [%d]\n\rItem type: ",
	      j->name, j->item_number, virtual);
      sprinttype(GET_ITEM_TYPE(j), item_types, buf2);
      strcat(buf, buf2);
      strcat(buf, "\n\r");
      send_to_char(buf, ch);
      sprintf(buf, "Short description: %s\n\rLong description:\n\r%s\n\r",
	   ((j->short_description) ? j->short_description : "None"),
	      ((j->description) ? j->description : "None"));
      send_to_char(buf, ch);
      if (j->ex_description) {
	strcpy(buf, "Extra description keyword(s):\n\r----------\n\r");
	for (desc = j->ex_description; desc; desc = desc->next) {
	  strcat(buf, desc->keyword);
	  strcat(buf, "\n\r");
	}
	strcat(buf, "----------\n\r");
	send_to_char(buf, ch);
      } else {
	strcpy(buf, "Extra description keyword(s): None\n\r");
	send_to_char(buf, ch);
      }

      send_to_char("Can be worn on :", ch);
      sprintbit(j->obj_flags.wear_flags, wear_bits, buf);
      strcat(buf, "\n\r");
      send_to_char(buf, ch);

      send_to_char("Set char bits  :", ch);
      sprintbit(j->obj_flags.bitvector, affected_bits, buf);
      strcat(buf, "\n\r");
      send_to_char(buf, ch);

      send_to_char("Extra flags: ", ch);
      sprintbit(j->obj_flags.extra_flags, extra_bits, buf);
      strcat(buf, "\n\r");
      send_to_char(buf, ch);

      sprintf(buf, "Weight: %d, Value: %d, Cost/day: %d, Timer: %d\n\r",
	      j->obj_flags.weight, j->obj_flags.cost,
	      j->obj_flags.cost_per_day, j->obj_flags.timer);
      send_to_char(buf, ch);

      strcpy(buf, "In room: ");
      if (j->in_room == NOWHERE)
	strcat(buf, "Nowhere");
      else {
	sprintf(buf2, "%d", j->in_room);
	strcat(buf, buf2);
      }
      strcat(buf, " ,In object: ");
      strcat(buf, (!j->in_obj ? "None" : fname(j->in_obj->name)));

      /*
       * strcat(buf," ,Carried by:");
       * if (j->carried_by) 
       * {
       * if (GET_NAME(j->carried_by)) 
       * {
       * if (strlen(GET_NAME(j->carried_by)) > 0) 
       * {
       * strcat(buf, (!j->carried_by) ? "Nobody" : GET_NAME(j->carried_by));
       * }
       * else
       * {
       * strcat(buf, "NonExistantPlayer");
       * }
       * }
       * else 
       * {
       * strcat(buf, "NonExistantPlayer");
       * }
       * }
       * else 
       * {
       * strcat(buf, "Nobody");
       * }
       * strcat(buf,"\n\r");
       * send_to_char(buf, ch);
       */
      switch (j->obj_flags.type_flag) {
      case ITEM_LIGHT:
	sprintf(buf, "Colour : [%d]\n\rType : [%d]\n\rHours : [%d]",
		j->obj_flags.value[0],
		j->obj_flags.value[1],
		j->obj_flags.value[2]);
	break;
      case ITEM_SCROLL:
	sprintf(buf, "Spells : %d, %d, %d, %d",
		j->obj_flags.value[0],
		j->obj_flags.value[1],
		j->obj_flags.value[2],
		j->obj_flags.value[3]);
	break;
      case ITEM_WAND:
	sprintf(buf, "Spell : %d\n\rMana : %d",
		j->obj_flags.value[0],
		j->obj_flags.value[1]);
	break;
      case ITEM_STAFF:
	sprintf(buf, "Spell : %d\n\rMana : %d",
		j->obj_flags.value[0],
		j->obj_flags.value[1]);
	break;
      case ITEM_WEAPON:
	sprintf(buf, "Tohit : %d\n\rTodam : %dD%d\n\rType : %d",
		j->obj_flags.value[0],
		j->obj_flags.value[1],
		j->obj_flags.value[2],
		j->obj_flags.value[3]);
	break;
      case ITEM_FIREWEAPON:
	sprintf(buf, "Tohit : %d\n\rTodam : %dD%d\n\rType : %d",
		j->obj_flags.value[0],
		j->obj_flags.value[1],
		j->obj_flags.value[2],
		j->obj_flags.value[3]);
	break;
      case ITEM_MISSILE:
	sprintf(buf, "Tohit : %d\n\rTodam : %d\n\rType : %d",
		j->obj_flags.value[0],
		j->obj_flags.value[1],
		j->obj_flags.value[3]);
	break;
      case ITEM_ARMOR:
	sprintf(buf, "AC-apply : [%d]\n\rFull Strength : [%d]",
		j->obj_flags.value[0],
		j->obj_flags.value[1]);

	break;
      case ITEM_POTION:
	sprintf(buf, "Spells : %d, %d, %d, %d",
		j->obj_flags.value[0],
		j->obj_flags.value[1],
		j->obj_flags.value[2],
		j->obj_flags.value[3]);
	break;
      case ITEM_TRAP:
	sprintf(buf, "level: %d, att type: %d, damage class: %d, charges: %d",
		j->obj_flags.value[0],
		j->obj_flags.value[1],
		j->obj_flags.value[2],
		j->obj_flags.value[3]);
	break;
      case ITEM_CONTAINER:
	sprintf(buf, "Max-contains : %d\n\rLocktype : %d\n\rCorpse : %s",
		j->obj_flags.value[0],
		j->obj_flags.value[1],
		j->obj_flags.value[3] ? "Yes" : "No");
	break;
      case ITEM_DRINKCON:
	sprinttype(j->obj_flags.value[2], drinks, buf2);
	sprintf(buf, "Max-contains : %d\n\rContains : %d\n\rPoisoned : %d\n\rLiquid : %s",
		j->obj_flags.value[0],
		j->obj_flags.value[1],
		j->obj_flags.value[3],
		buf2);
	break;
      case ITEM_NOTE:
	sprintf(buf, "Tounge : %d",
		j->obj_flags.value[0]);
	break;
      case ITEM_KEY:
	sprintf(buf, "Keytype : %d",
		j->obj_flags.value[0]);
	break;
      case ITEM_FOOD:
	sprintf(buf, "Makes full : %d\n\rPoisoned : %d",
		j->obj_flags.value[0],
		j->obj_flags.value[3]);
	break;
      default:
	sprintf(buf, "Values 0-3 : [%d] [%d] [%d] [%d]",
		j->obj_flags.value[0],
		j->obj_flags.value[1],
		j->obj_flags.value[2],
		j->obj_flags.value[3]);
	break;
      }
      send_to_char(buf, ch);

      strcpy(buf, "\n\rEquipment Status: ");
      if (!j->carried_by)
	strcat(buf, "NONE");
      else {
	found = FALSE;
	for (i = 0; i < MAX_WEAR; i++) {
	  if (j->carried_by->equipment[i] == j) {
	    sprinttype(i, equipment_types, buf2);
	    strcat(buf, buf2);
	    found = TRUE;
	  }
	}
	if (!found)
	  strcat(buf, "Inventory");
      }
      send_to_char(buf, ch);

      strcpy(buf, "\n\rSpecial procedure : ");
      if (j->item_number >= 0)
	strcat(buf, (obj_index[j->item_number].func ? "exists\n\r" : "No\n\r"));
      else
	strcat(buf, "No\n\r");
      send_to_char(buf, ch);

      strcpy(buf, "Contains :\n\r");
      found = FALSE;
      for (j2 = j->contains; j2; j2 = j2->next_content) {
	strcat(buf, fname(j2->name));
	strcat(buf, "\n\r");
	found == TRUE;
      }
      if (!found)
	strcpy(buf, "Contains : Nothing\n\r");
      send_to_char(buf, ch);

      send_to_char("Can affect char :\n\r", ch);
      for (i = 0; i < MAX_OBJ_AFFECT; i++) {
	sprinttype(j->affected[i].location, apply_types, buf2);
	sprintf(buf, "    Affects : %s By %d\n\r", buf2, j->affected[i].modifier);
	send_to_char(buf, ch);
      }
      return;
    } else {
      send_to_char("No mobile or object by that name in the world\n\r", ch);
    }
  }
}

void 
do_pretitle(struct char_data *ch, char *argument, int cmd)
{
  char                             name[20];
  char                             pretitle[50];
  struct char_data                *vict;

  argument = one_argument(argument, name);
  strcpy(pretitle, argument);

  if ((vict = get_char_vis(ch, name)) == NULL) {
    send_to_char("I don't see them here?\n\r", ch);
    return;
  }
  if ((strlen(pretitle) == 0)) {
    GET_PRETITLE(vict) = 0;
    return;
  }
  GET_PRETITLE(vict) = (char *)calloc(1, strlen(pretitle));
  strcpy(GET_PRETITLE(vict), pretitle);
}

void 
do_set(struct char_data *ch, char *argument, int cmd)
{
  char                             field[20],
                                   name[20],
                                   parmstr[50];
  int                              index;
  struct char_data                *mob;
  int                              parm,
                                   parm2;
  char                            *pset_list[] =
  {
    "align",
    "exp",
    "sex",
    "race",
    "hit",
    "mhit",
    "tohit",
    "bank",
    "gold",
    "prac",
    "str",
    "int",
    "wis",
    "dex",
    "con",
    "stradd",
    '\0'
  };

  char                             tmp[80];

  if (IS_NPC(ch))
    return;

  argument = one_argument(argument, name);
  argument = one_argument(argument, field);
  strncpy(tmp, argument, 79);
  argument = one_argument(argument, parmstr);

  if ((mob = get_char_vis(ch, name)) == NULL) {
    send_to_char("I don't see them here? \n\r\n\r", ch);
    send_to_char("Usage: pset <name> <att> <val>\n\r", ch);
    send_to_char("att: align, exp, sex, race, hit, mhit\n\r", ch);
    send_to_char("     tohit, bank, gold, prac\n\r", ch);
    return;
  }
  for (index = 0; pset_list[index]; index++)
    if (!strcmp(field, pset_list[index])) {
      sscanf(parmstr, "%d", &parm);
      if (!parm) {
	send_to_char("You must also supply a value\n\r", ch);
	return;
      }
      break;
    }
  switch (index) {
  case 0:
    GET_ALIGNMENT(mob) = parm;
    break;
  case 1:
    GET_EXP(mob) = parm;
    break;
  case 2:
    GET_SEX(mob) = parm;
    break;
  case 3:
    GET_RACE(mob) = parm;
    break;
  case 4:
    GET_HIT(mob) = parm;
    break;
  case 5:
    mob->points.max_hit = parm;
    break;
  case 6:
    GET_HITROLL(mob) = parm;
    break;
  case 7:
    GET_BANK(mob) = parm;
    break;
  case 8:
    GET_GOLD(mob) = parm;
    break;
  case 9:
    mob->specials.spells_to_learn = parm;
    break;
  case 10:
    mob->abilities.str = parm;
    mob->tmpabilities = mob->abilities;
    break;
  case 11:
    mob->abilities.intel = parm;
    mob->tmpabilities = mob->abilities;
    break;
  case 12:
    mob->abilities.wis = parm;
    mob->tmpabilities = mob->abilities;
    break;
  case 13:
    mob->abilities.dex = parm;
    mob->tmpabilities = mob->abilities;
    break;
  case 14:
    mob->abilities.con = parm;
    mob->tmpabilities = mob->abilities;
    break;
  case 15:
    mob->abilities.str_add = parm;
    mob->tmpabilities = mob->abilities;
    break;
  default:
    {
      send_to_char("Usage: pset <name> <att> <val>\n\r", ch);
      send_to_char("att: align, exp, sex, race, hit, mhit\n\r", ch);
      send_to_char("     tohit, bank, gold, prac,\n\r", ch);
      send_to_char("     str,int,wis,dex,con,stradd,\n\r", ch);
    };
  }
}

void 
do_shutdow(struct char_data *ch, char *argument, int cmd)
{
  send_to_char("If you want to shut something down - say so!\n\r", ch);
}

void 
do_shutdown(struct char_data *ch, char *argument, int cmd)
{
  extern int                       diku_shutdown,
                                   reboot;
  char                             buf[100],
                                   arg[MAX_INPUT_LENGTH];

  if (IS_NPC(ch))
    return;

  one_argument(argument, arg);

  if (!*arg) {
    sprintf(buf, "Shutdown by %s.", GET_NAME(ch));
    send_to_all(buf);
    log(buf);
    diku_shutdown = 1;
    update_time_and_weather();
  } else if (!str_cmp(arg, "reboot")) {
    sprintf(buf, "Reboot by %s.", GET_NAME(ch));
    send_to_all(buf);
    log(buf);
    diku_shutdown = reboot = 1;
    update_time_and_weather();
  } else
    send_to_char("Go shut down someone your own size.\n\r", ch);
}

void 
do_snoop(struct char_data *ch, char *argument, int cmd)
{
  static char                      arg[MAX_STRING_LENGTH];
  struct char_data                *victim;

  if (!ch->desc)
    return;

  if (IS_NPC(ch))
    return;

  only_argument(argument, arg);

  if (!*arg) {
    send_to_char("Snoop who ?\n\r", ch);
    return;
  }
  if (!(victim = get_char_vis(ch, arg))) {
    send_to_char("No such person around.\n\r", ch);
    return;
  }
  if (!victim->desc) {
    send_to_char("There's no link.. nothing to snoop.\n\r", ch);
    return;
  }
  if (victim == ch) {
    send_to_char("Ok, you just snoop yourself.\n\r", ch);
    if (ch->desc->snoop.snooping) {
      if (ch->desc->snoop.snooping->desc)
	ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;
      else {
	char                             buf[MAX_STRING_LENGTH];

	sprintf(buf, "caught %s snooping %s who didn't have a descriptor!",
	    ch->player.name, ch->desc->snoop.snooping->player.name);
	log(buf);
      }
      ch->desc->snoop.snooping = 0;
    }
    return;
  }
  if (victim->desc->snoop.snoop_by) {
    send_to_char("Busy already. \n\r", ch);
    return;
  }
  if (GetMaxLevel(victim) >= GetMaxLevel(ch)) {
    send_to_char("You failed.\n\r", ch);
    return;
  }
  send_to_char("Ok. \n\r", ch);

  if (ch->desc->snoop.snooping)
    if (ch->desc->snoop.snooping->desc)
      ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;

  ch->desc->snoop.snooping = victim;
  victim->desc->snoop.snoop_by = ch;
  return;
}

void 
do_switch(struct char_data *ch, char *argument, int cmd)
{
  static char                      arg[80];
  struct char_data                *victim;

  if (IS_NPC(ch))
    return;

  only_argument(argument, arg);

  if (!*arg) {
    send_to_char("Switch with who?\n\r", ch);
  } else {
    if (!(victim = get_char(arg)))
      send_to_char("They aren't here.\n\r", ch);
    else {
      if (ch == victim) {
	send_to_char("He he he... We are jolly funny today, eh?\n\r", ch);
	return;
      }
      if (!ch->desc || ch->desc->snoop.snoop_by || ch->desc->snoop.snooping) {
	send_to_char("Mixing snoop & switch is bad for your health.\n\r", ch);
	return;
      }
      if (victim->desc || (!IS_NPC(victim))) {
	send_to_char(
	  "You can't do that, the body is already in use!\n\r", ch);
      } else {
	send_to_char("Ok.\n\r", ch);

	ch->desc->character = victim;
	ch->desc->original = ch;

	victim->desc = ch->desc;
	ch->desc = 0;
      }
    }
  }
}

void 
do_return(struct char_data *ch, char *argument, int cmd)
{
  struct char_data                *mob,
                                  *per;

  if (!ch->desc)
    return;

  if (!ch->desc->original) {
    send_to_char("Huh? Talk sense I cant understand you\n\r", ch);
    return;
  } else {
    send_to_char("You return to your original body.\n\r", ch);

    if (IS_SET(ch->specials.act, ACT_POLYSELF) && cmd) {
      mob = ch;
      per = ch->desc->original;

      act("$n turns liquid, and reforms as $N", TRUE, mob, 0, per, TO_ROOM);

      char_from_room(per);
      char_to_room(per, mob->in_room);

      /*
       * SwitchStuff(mob, per); 
       */
    }
    ch->desc->character = ch->desc->original;
    ch->desc->original = 0;

    ch->desc->character->desc = ch->desc;
    ch->desc = 0;

    if (IS_SET(ch->specials.act, ACT_POLYSELF) && cmd) {
      extract_char(mob);
    }
  }
}

void 
do_force(struct char_data *ch, char *argument, int cmd)
{
  struct descriptor_data          *i;
  struct char_data                *vict;
  extern struct char_data         *board_kludge_char;
  char                             name[100],
                                   to_force[100],
                                   buf[100];

  if (IS_NPC(ch) && (cmd != 0))
    return;

  half_chop(argument, name, to_force);

  if (!*name || !*to_force)
    send_to_char("Who do you wish to force to do what?\n\r", ch);
  else if (str_cmp("all", name)) {
    if (!(vict = get_char_vis(ch, name)))
      send_to_char("No-one by that name here..\n\r", ch);
    else {
      if ((GetMaxLevel(ch) <= GetMaxLevel(vict)) && (!IS_NPC(vict)))
	send_to_char("Oh no you don't!!\n\r", ch);
      else {
	sprintf(buf, "$n has forced you to '%s'.", to_force);
	act(buf, FALSE, ch, 0, vict, TO_VICT);
	send_to_char("Ok.\n\r", ch);
	command_interpreter(vict, to_force);
      }
    }
  } else {			/*
				 * force all 
				 */
    for (i = descriptor_list; i; i = i->next)
      if (i->character != ch && !i->connected && i->character != board_kludge_char) {
	vict = i->character;
	if ((GetMaxLevel(ch) <= GetMaxLevel(vict)) &&
	    (!IS_NPC(vict)))
	  send_to_char("Oh no you don't!!\n\r", ch);
	else {
	  sprintf(buf, "$n has forced you to '%s'.", to_force);
	  act(buf, FALSE, ch, 0, vict, TO_VICT);
	  command_interpreter(vict, to_force);
	}
      }
    send_to_char("Ok.\n\r", ch);
  }
}

void 
do_load(struct char_data *ch, char *argument, int cmd)
{
  struct char_data                *mob;
  struct obj_data                 *obj;
  char                             type[100],
                                   num[100];
  int                              number;

  extern int                       top_of_mobt;
  extern int                       top_of_objt;

  if (IS_NPC(ch))
    return;

  argument = one_argument(argument, type);

  only_argument(argument, num);
  if (isdigit(*num))
    number = atoi(num);
  else
    number = -1;

  if (is_abbrev(type, "mobile")) {
    if (number < 0) {
      for (number = 0; number <= top_of_mobt; number++)
	if (isname(num, mob_index[number].name))
	  break;
      if (number > top_of_mobt)
	number = -1;
    } else {
      number = real_mobile(number);
    }
    if (number < 0 || number > top_of_mobt) {
      send_to_char("There is no such monster.\n\r", ch);
      return;
    }
    mob = read_mobile(number, REAL);
    char_to_room(mob, ch->in_room);

    act("$n makes a quaint, magical gesture with one hand.", TRUE, ch, 0, 0, TO_ROOM);
    act("$n has summoned $N from the ether!", FALSE, ch, 0, mob, TO_ROOM);
    act("You bring forth $N from the the cosmic ether.", FALSE, ch, 0, mob, TO_CHAR);
  } else if (is_abbrev(type, "object")) {
    if (number < 0) {
      for (number = 0; number <= top_of_objt; number++)
	if (isname(num, obj_index[number].name))
	  break;
      if (number > top_of_objt)
	number = -1;
    } else {
      number = real_object(number);
    }
    if (number < 0 || number > top_of_objt) {
      send_to_char("There is no such object.\n\r", ch);
      return;
    }
    obj = read_object(number, REAL);
    obj_to_char(obj, ch);
    act("$n makes a strange magical gesture.", TRUE, ch, 0, 0, TO_ROOM);
    act("$n has created $p!", FALSE, ch, obj, 0, TO_ROOM);
    act("You now have $p.", FALSE, ch, obj, 0, TO_CHAR);
  } else if (is_abbrev(type, "room")) {
    int                              start,
                                     end;

    if (GetMaxLevel(ch) < CREATOR)
      return;

    switch (sscanf(num, "%d %d", &start, &end)) {
    case 2:			/*
				 * we got both numbers 
				 */
      RoomLoad(ch, start, end);
      break;
    case 1:			/*
				 * we only got one, load it 
				 */
      RoomLoad(ch, start, start);
      break;
    default:
      send_to_char("Load? Fine!  Load we must, But what?\n\r", ch);
      break;
    }
  } else {
    send_to_char("Usage: load (object|mobile) (number|name)\n\r"
		 "       load room start [end]\n\r", ch);
  }
}

static void 
purge_one_room(int rnum, struct room_data *rp, int *range)
{
  struct char_data                *ch;
  struct obj_data                 *obj;

  if (rnum == 0 || rnum < range[0] || rnum > range[1])
    return;

  while (rp->people) {
    ch = rp->people;
    send_to_char("The gods strike down from the heavens making the", ch);
    send_to_char("world tremble.  All that's left is the Void.", ch);
    char_from_room(ch);
    char_to_room(ch, 0);	/*
				 * send character to the void 
				 */
    do_look(ch, "", 15);
    act("$n tumbles into the Void.", TRUE, ch, 0, 0, TO_ROOM);
  }

  while (rp->contents) {
    obj = rp->contents;
    obj_from_room(obj);
    obj_to_room(obj, 0);	/*
				 * send item to the void 
				 */
  }
  completely_cleanout_room(rp);	/*
				 * clear out the pointers 
				 */
  hash_remove(&room_db, rnum);	/*
				 * remove it from the database 
				 */
}

/*
 * clean a room of all mobiles and objects 
 */
void 
do_purge(struct char_data *ch, char *argument, int cmd)
{
  struct char_data                *vict,
                                  *next_v;
  struct obj_data                 *obj,
                                  *next_o;

  char                             name[100];

  if (IS_NPC(ch))
    return;

  only_argument(argument, name);

  if (*name) {			/*
				 * argument supplied. destroy single object or char 
				 */
    if (vict = get_char_room_vis(ch, name)) {
      if ((!IS_NPC(vict) || IS_SET(vict->specials.act, ACT_POLYSELF)) &&
	  (GetMaxLevel(ch) < IMPLEMENTOR)) {
	send_to_char("I'm sorry, Dave.  I can't let you do that.\n\r", ch);
	return;
      }
      act("$n disintegrates $N.", FALSE, ch, 0, vict, TO_NOTVICT);

      if (IS_NPC(vict) || (!IS_SET(ch->specials.act, ACT_POLYSELF))) {
	extract_char(vict);
      } else {
	if (vict->desc) {
	  close_socket(vict->desc);
	  vict->desc = 0;
	  extract_char(vict);
	} else {
	  extract_char(vict);
	}
      }
    } else if (obj = get_obj_in_list_vis
	       (ch, name, real_roomp(ch->in_room)->contents)) {
      act("$n destroys $p.", FALSE, ch, obj, 0, TO_ROOM);
      extract_obj(obj);
    } else {
      argument = one_argument(argument, name);
      if (0 == str_cmp("room", name)) {
	int                              range[2];

	if (GetMaxLevel(ch) < IMPLEMENTOR) {
	  send_to_char("I'm sorry, Dave.  I can't let you do that.\n\r", ch);
	  return;
	}
	argument = one_argument(argument, name);
	if (!isdigit(*name)) {
	  send_to_char("purge room start [end]", ch);
	  return;
	}
	range[0] = atoi(name);
	argument = one_argument(argument, name);
	if (isdigit(*name))
	  range[1] = atoi(name);
	else
	  range[1] = range[0];

	if (range[0] == 0 || range[1] == 0) {
	  send_to_char("usage: purge room start [end]\n\r", ch);
	  return;
	}
	hash_iterate(&room_db, purge_one_room, range);
      } else {
	send_to_char("I don't see that here.\n\r", ch);
	return;
      }
    }

    send_to_char("Ok.\n\r", ch);
  } else {			/*
				 * no argument. clean out the room 
				 */
    if (GetMaxLevel(ch) < DEMIGOD)
      return;
    if (IS_NPC(ch)) {
      send_to_char("You would only kill yourself..\n\r", ch);
      return;
    }
    act("$n gestures, the world erupts around you in flames!",
	FALSE, ch, 0, 0, TO_ROOM);
    send_to_room("The world seems a little cleaner.\n\r", ch->in_room);

    for (vict = real_roomp(ch->in_room)->people; vict; vict = next_v) {
      next_v = vict->next_in_room;
      if (IS_NPC(vict) && (!IS_SET(vict->specials.act, ACT_POLYSELF)))
	extract_char(vict);
    }

    for (obj = real_roomp(ch->in_room)->contents; obj; obj = next_o) {
      next_o = obj->next_content;
      extract_obj(obj);
    }
  }
}

/*
 * Give pointers to the five abilities 
 */
void 
roll_abilities(struct char_data *ch)
{
  int                              i,
                                   j,
                                   k,
                                   temp;
  ubyte                            table[5];
  ubyte                            rools[4];

  for (i = 0; i < 5; table[i++] = 0);

  for (i = 0; i < 5; i++) {

    for (j = 0; j < 4; j++)
      rools[j] = number(1, 6);

    temp = rools[0] + rools[1] + rools[2] + rools[3] -
      MIN(rools[0], MIN(rools[1], MIN(rools[2], rools[3])));

    for (k = 0; k < 5; k++)
      if (table[k] < temp)
	SWITCH(temp, table[k]);
  }

  ch->abilities.str_add = 0;

  switch (ch->player.class) {
  case CLASS_MAGIC_USER:{
      ch->abilities.intel = table[0];
      ch->abilities.wis = table[1];
      ch->abilities.dex = table[2];
      ch->abilities.str = table[3];
      ch->abilities.con = table[4];
    }
    break;
  case CLASS_DRUID:{
      ch->abilities.wis = table[0];
      ch->abilities.intel = table[1];
      ch->abilities.str = table[2];
      ch->abilities.dex = table[3];
      ch->abilities.con = table[4];
    }
    break;
  case CLASS_CLERIC:{
      ch->abilities.wis = table[0];
      ch->abilities.intel = table[1];
      ch->abilities.str = table[2];
      ch->abilities.dex = table[3];
      ch->abilities.con = table[4];
    }
    break;
  case CLASS_CLERIC + CLASS_MAGIC_USER:{
      ch->abilities.wis = table[0];
      ch->abilities.intel = table[1];
      ch->abilities.dex = table[2];
      ch->abilities.str = table[3];
      ch->abilities.con = table[4];
    }
    break;
  case CLASS_THIEF:{
      ch->abilities.dex = table[0];
      ch->abilities.intel = table[1];
      ch->abilities.str = table[2];
      ch->abilities.con = table[3];
      ch->abilities.wis = table[4];
    }
    break;
  case CLASS_THIEF + CLASS_MAGIC_USER:{
      ch->abilities.intel = table[0];
      ch->abilities.dex = table[1];
      ch->abilities.str = table[2];
      ch->abilities.con = table[3];
      ch->abilities.wis = table[4];
    }
    break;
  case CLASS_THIEF + CLASS_CLERIC:{
      ch->abilities.wis = table[0];
      ch->abilities.dex = table[1];
      ch->abilities.intel = table[2];
      ch->abilities.str = table[3];
      ch->abilities.con = table[4];
    }
    break;
  case CLASS_THIEF + CLASS_MAGIC_USER + CLASS_CLERIC:{
      ch->abilities.wis = table[0];
      ch->abilities.intel = table[1];
      ch->abilities.dex = table[2];
      ch->abilities.str = table[3];
      ch->abilities.con = table[4];
    }
    break;
  case CLASS_RANGER:{
      ch->abilities.str = table[0];
      ch->abilities.con = table[3];
      ch->abilities.dex = table[2];
      ch->abilities.wis = table[1];
      ch->abilities.intel = table[4];
      if (ch->abilities.str == 18)
	ch->abilities.str_add = 0;
    }
    break;
  case CLASS_WARRIOR:{
      ch->abilities.str = table[0];
      ch->abilities.con = table[1];
      ch->abilities.dex = table[2];
      ch->abilities.wis = table[3];
      ch->abilities.intel = table[4];
      if (ch->abilities.str == 18)
	ch->abilities.str_add = number(0, 100);
    }
    break;
  case CLASS_WARRIOR + CLASS_MAGIC_USER:{
      ch->abilities.str = table[0];
      ch->abilities.intel = table[1];
      ch->abilities.con = table[2];
      ch->abilities.dex = table[3];
      ch->abilities.wis = table[4];
      if (ch->abilities.str == 18)
	ch->abilities.str_add = 0;
    }
    break;
  case CLASS_WARRIOR + CLASS_CLERIC:{
      ch->abilities.wis = table[0];
      ch->abilities.str = table[1];
      ch->abilities.intel = table[2];
      ch->abilities.con = table[3];
      ch->abilities.dex = table[4];
      if (ch->abilities.str == 18)
	ch->abilities.str_add = 0;
    }
    break;
  case CLASS_WARRIOR + CLASS_THIEF:{
      ch->abilities.str = table[0];
      ch->abilities.dex = table[1];
      ch->abilities.con = table[2];
      ch->abilities.intel = table[3];
      ch->abilities.wis = table[4];
      if (ch->abilities.str == 18)
	ch->abilities.str_add = 0;
    }
    break;
  case CLASS_WARRIOR + CLASS_MAGIC_USER + CLASS_CLERIC:{
      ch->abilities.wis = table[0];
      ch->abilities.str = table[1];
      ch->abilities.intel = table[2];
      ch->abilities.dex = table[3];
      ch->abilities.con = table[4];
      if (ch->abilities.str == 18)
	ch->abilities.str_add = 0;
    }
    break;
  case CLASS_WARRIOR + CLASS_MAGIC_USER + CLASS_THIEF:{
      ch->abilities.intel = table[0];
      ch->abilities.str = table[1];
      ch->abilities.dex = table[2];
      ch->abilities.con = table[3];
      ch->abilities.wis = table[4];
      if (ch->abilities.str == 18)
	ch->abilities.str_add = 0;
    }
    break;
  case CLASS_WARRIOR + CLASS_THIEF + CLASS_CLERIC:{
      ch->abilities.str = table[0];
      ch->abilities.wis = table[1];
      ch->abilities.dex = table[2];
      ch->abilities.intel = table[3];
      ch->abilities.con = table[4];
      if (ch->abilities.str == 18)
	ch->abilities.str_add = 0;
    }
    break;
  default:
    log("Error on class (do_reroll)");
  }

  switch (GET_RACE(ch)) {
  case RACE_ELVEN:
    ch->abilities.intel += 1;
    break;
  case RACE_DWARF:
    ch->abilities.con += 1;
    ch->abilities.intel -= 1;
    break;
  case RACE_HALFLING:
    ch->abilities.dex += 1;
    ch->abilities.con -= 1;
    break;
  case RACE_GNOME:
    ch->abilities.wis += 1;
    ch->abilities.dex -= 1;
    break;
  }

  if (ch->abilities.str > 18)
    ch->abilities.str = 18;
  if (ch->abilities.dex > 18)
    ch->abilities.dex = 18;
  if (ch->abilities.intel > 18)
    ch->abilities.intel = 18;
  if (ch->abilities.wis > 18)
    ch->abilities.wis = 18;
  if (ch->abilities.con > 18)
    ch->abilities.con = 18;

  ch->tmpabilities = ch->abilities;
}

void 
do_start(struct char_data *ch)
{
  int                              i,
                                   r_num;
  struct obj_data                 *obj;

  extern struct dex_skill_type     dex_app_skill[];
  void                             advance_level(struct char_data *ch, int i);

  StartLevels(ch);
  GET_EXP(ch) = 1;
  set_title(ch);
  roll_abilities(ch);

  ch->points.max_hit = 20;

/*
 * Heafty Bread 
 */
  if ((r_num = real_object(5016)) >= 0) {
    obj = read_object(r_num, REAL);
    obj_to_char(obj, ch);	/*
				 */
    obj = read_object(r_num, REAL);
    obj_to_char(obj, ch);
  }
/*
 * Bottle of Water 
 */
  if ((r_num = real_object(3003)) >= 0) {
    obj = read_object(r_num, REAL);
    obj_to_char(obj, ch);
    obj = read_object(r_num, REAL);
    obj_to_char(obj, ch);
  }
/*
 * Club 
 */
  if ((r_num = real_object(3048)) >= 0) {
    obj = read_object(r_num, REAL);
    obj_to_char(obj, ch);
  }
/*
 * Map of Shylar 
 */
  if ((r_num = real_object(3050)) >= 0) {
    obj = read_object(r_num, REAL);
    obj_to_char(obj, ch);
  }
  if (IS_SET(ch->player.class, CLASS_RANGER)) {
    ch->skills[SKILL_HUNT].learned = 1;
    ch->skills[SKILL_DISARM].learned = 1;
  }
  if (IS_SET(ch->player.class, CLASS_THIEF)) {
    ch->skills[SKILL_SNEAK].learned = 1;
    ch->skills[SKILL_HIDE].learned = 1;
    ch->skills[SKILL_STEAL].learned = 1;
    ch->skills[SKILL_BACKSTAB].learned = 1;
    ch->skills[SKILL_PICK_LOCK].learned = 1;
  }
  GET_HIT(ch) = GET_MAX_HIT(ch);
  GET_MANA(ch) = GET_MAX_MANA(ch);
  ch->points.max_move = GET_MAX_MOVE(ch);
  GET_MOVE(ch) = GET_MAX_MOVE(ch);

  GET_COND(ch, THIRST) = 24;
  GET_COND(ch, FULL) = 24;
  GET_COND(ch, DRUNK) = 0;

  ch->player.time.played = 0;
  ch->player.time.logon = time(0);
}

void 
do_advance(struct char_data *ch, char *argument, int cmd)
{
  struct char_data                *victim;
  char                             name[100],
                                   level[100],
                                   class[100];
  int                              adv,
                                   newlevel,
                                   lin_class;

  void                             gain_exp(struct char_data *ch, int gain);

  if (IS_NPC(ch))
    return;

  argument = one_argument(argument, name);

  if (*name) {
    if (!(victim = get_char_room_vis(ch, name))) {
      send_to_char("That player is not here.\n\r", ch);
      return;
    }
  } else {
    send_to_char("Advance who?\n\r", ch);
    return;
  }

  if (IS_NPC(victim)) {
    send_to_char("NO! Not on NPC's.\n\r", ch);
    return;
  }
  argument = one_argument(argument, class);

  if (!*class) {
    send_to_char("Classes you may suply: [ M C W T R ]\n\r", ch);
    return;
  }
  switch (*class) {
  case 'M':
  case 'm':
    lin_class = MAGE_LEVEL_IND;
    break;

  case 'T':
  case 't':
    lin_class = THIEF_LEVEL_IND;
    break;

  case 'W':
  case 'w':
  case 'F':
  case 'f':
    lin_class = WARRIOR_LEVEL_IND;
    break;

  case 'C':
  case 'c':
  case 'P':
  case 'p':
    lin_class = CLERIC_LEVEL_IND;
    break;

  case 'R':
  case 'r':
    lin_class = RANGER_LEVEL_IND;
    break;

  case 'D':
  case 'd':
    lin_class = DRUID_LEVEL_IND;
    break;

  default:
    send_to_char("Classes you may use [ M C W T R ]\n\r", ch);
    return;
    break;

  }

  argument = one_argument(argument, level);

  if (GET_LEVEL(victim, lin_class) == 0)
    adv = 1;
  else if (!*level) {
    send_to_char("You must supply a level number.\n\r", ch);
    return;
  } else {
    if (!isdigit(*level)) {
      send_to_char("Third argument must be a positive integer.\n\r", ch);
      return;
    }
    if ((newlevel = atoi(level)) < GET_LEVEL(victim, lin_class)) {
      send_to_char("Can't dimish a players status (yet).\n\r", ch);
      return;
    }
    adv = newlevel - GET_LEVEL(victim, lin_class);
  }

  if (((adv + GET_LEVEL(victim, lin_class)) > 1) &&
      (GetMaxLevel(ch) < IMPLEMENTOR)) {
    send_to_char("Thou art not godly enough.\n\r", ch);
    return;
  }
  if ((adv + GET_LEVEL(victim, lin_class)) > IMPLEMENTOR) {
    send_to_char("Implementor is the highest possible level.\n\r", ch);
    return;
  }
  if (((adv + GET_LEVEL(victim, lin_class)) < 1) &&
      ((adv + GET_LEVEL(victim, lin_class)) != 1)) {
    send_to_char("1 is the lowest possible level.\n\r", ch);
    return;
  }
  send_to_char("You feel generous.\n\r", ch);
  act("$n makes some strange gestures.\n\rA strange feeling comes upon you,"
      "\n\rLike a giant hand, light comes down from\n\rabove, grabbing your "
      "body, that begins\n\rto pulse with coloured lights from inside.\n\rYo"
      "ur head seems to be filled with daemons\n\rfrom another plane as your"
      " body dissolves\n\rinto the elements of time and space itself.\n\rSudde"
      "nly a silent explosion of light snaps\n\ryou back to reality. You fee"
      "l slightly\n\rdifferent.", FALSE, ch, 0, victim, TO_VICT);

  if (GET_LEVEL(victim, lin_class) == 0) {
    do_start(victim);
  } else {
    if (GET_LEVEL(victim, lin_class) < IMPLEMENTOR) {
      int                              amount_needed,
                                       amount_have;

      amount_needed = titles[lin_class][GET_LEVEL(victim, lin_class) + adv].exp + 1;
      amount_have = GET_EXP(victim);
      gain_exp_regardless(victim, amount_needed - amount_have, lin_class);
      send_to_char("Character is now advanced.\n\r", ch);
    } else {
      send_to_char("Some idiot just tried to advance your level.\n\r", victim);
      send_to_char("IMPOSSIBLE! IDIOTIC!\n\r", ch);
    }
  }
}

void 
do_reroll(struct char_data *ch, char *argument, int cmd)
{
  struct char_data                *victim;
  char                             buf[100];

  if (IS_NPC(ch))
    return;

  only_argument(argument, buf);
  if (!*buf)
    send_to_char("Who do you wish to reroll?\n\r", ch);
  else if (!(victim = get_char(buf)))
    send_to_char("No-one by that name in the world.\n\r", ch);
  else {
    send_to_char("Rerolled...\n\r", ch);
    roll_abilities(victim);
  }
}

void 
do_restore_all(struct char_data *ch, char *arg, int cmd)
{
  struct descriptor_data          *i;
  extern struct char_data         *board_kludge_char;
  struct char_data                *vict;

  void                             do_restore(struct char_data *ch, char *arg, int cmd);

  for (i = descriptor_list; i; i = i->next)
    if (i->character != ch && !i->connected && i->character != board_kludge_char) {
      vict = i->character;
      do_restore(ch, GET_NAME(vict), 0);
    }
}

void 
do_restore(struct char_data *ch, char *argument, int cmd)
{
  struct char_data                *victim;
  char                             buf[100];
  int                              i;

  void                             update_pos(struct char_data *victim);

  only_argument(argument, buf);
  if (!*buf)
    send_to_char("Who do you wish to restore?\n\r", ch);
  else if (!(victim = get_char(buf)))
    send_to_char("No-one by that name in the world.\n\r", ch);
  else {
    GET_MANA(victim) = GET_MAX_MANA(victim);
    if (!affected_by_spell(victim, SPELL_AID)) {
      GET_HIT(victim) = GET_MAX_HIT(victim);
    } else {
      if (GET_HIT(victim) < GET_MAX_HIT(victim))
	GET_HIT(victim) = GET_MAX_HIT(victim);
    }

    GET_MOVE(victim) = GET_MAX_MOVE(victim);

    if (IS_NPC(victim))
      return;

    if (GetMaxLevel(victim) < LOW_IMMORTAL) {
      GET_COND(victim, THIRST) = 24;
      GET_COND(victim, FULL) = 24;
    } else {
      GET_COND(victim, THIRST) = -1;
      GET_COND(victim, FULL) = -1;
    }

    if (GetMaxLevel(victim) >= CREATOR) {
      for (i = 0; i < MAX_SKILLS; i++) {
	victim->skills[i].learned = 100;
	victim->skills[i].recognise = TRUE;
      }

      if (GetMaxLevel(victim) >= SILLYLORD) {
	victim->abilities.str_add = 100;
	victim->abilities.intel = 25;
	victim->abilities.wis = 25;
	victim->abilities.dex = 25;
	victim->abilities.str = 25;
	victim->abilities.con = 25;
      }
      victim->tmpabilities = victim->abilities;

    }
    update_pos(victim);
    send_to_char("Done.\n\r", ch);
    act("You have been fully healed by $N!", FALSE, victim, 0, ch, TO_CHAR);
  }
}

do_show_logs(struct char_data *ch, char *argument, int cmd)
{
  if (IS_SET(ch->specials.act, PLR_LOGS)) {
    send_to_char("You will no longer recieve the logs to your screen.\n\r", ch);
    REMOVE_BIT(ch->specials.act, PLR_LOGS);
    return;
  } else {
    send_to_char("You WILL recieve the logs to your screen.\n\r", ch);
    SET_BIT(ch->specials.act, PLR_LOGS);
    return;
  }
}

do_noshout(struct char_data * ch, char *argument, int cmd)
{
  struct char_data                *vict;
  struct obj_data                 *dummy;
  char                             buf[MAX_INPUT_LENGTH];

  if (IS_NPC(ch))
    return;

  only_argument(argument, buf);

  if (!*buf)
    if (IS_SET(ch->specials.act, PLR_NOSHOUT)) {
      send_to_char("You can now hear shouts again.\n\r", ch);
      REMOVE_BIT(ch->specials.act, PLR_NOSHOUT);
    } else {
      send_to_char("From now on, you won't hear shouts.\n\r", ch);
      SET_BIT(ch->specials.act, PLR_NOSHOUT);
  } else if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
    send_to_char("Couldn't find any such creature.\n\r", ch);
  else if (IS_NPC(vict))
    send_to_char("Can't do that to a beast.\n\r", ch);
  else if (GetMaxLevel(vict) >= GetMaxLevel(ch))
    act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
  else if (IS_SET(vict->specials.act, PLR_NOSHOUT) &&
	   (GetMaxLevel(ch) >= SAINT)) {
    send_to_char("You can shout again.\n\r", vict);
    send_to_char("NOSHOUT removed.\n\r", ch);
    REMOVE_BIT(vict->specials.act, PLR_NOSHOUT);
  } else if (GetMaxLevel(ch) >= SAINT) {
    send_to_char("The gods take away your ability to shout!\n\r", vict);
    send_to_char("NOSHOUT set.\n\r", ch);
    SET_BIT(vict->specials.act, PLR_NOSHOUT);
  } else {
    send_to_char("Sorry, you can't do that\n\r", ch);
  }
}

void 
do_pager(struct char_data *ch, char *arg, int cmd)
{
  if (IS_SET(ch->specials.act, PLR_PAGER)) {
    send_to_char("You stop using the Wiley Pager.\n\r", ch);
    REMOVE_BIT(ch->specials.act, PLR_PAGER);
  } else {
    send_to_char("You now USE the Wiley Pager.\n\r", ch);
    SET_BIT(ch->specials.act, PLR_PAGER);
  }
}

void 
do_nohassle(struct char_data *ch, char *argument, int cmd)
{
  struct char_data                *vict;
  struct obj_data                 *dummy;
  char                             buf[MAX_INPUT_LENGTH];

  if (IS_NPC(ch))
    return;

  only_argument(argument, buf);

  if (!*buf)
    if (IS_SET(ch->specials.act, PLR_NOHASSLE)) {
      send_to_char("You can now be hassled again.\n\r", ch);
      REMOVE_BIT(ch->specials.act, PLR_NOHASSLE);
    } else {
      send_to_char("From now on, you won't be hassled.\n\r", ch);
      SET_BIT(ch->specials.act, PLR_NOHASSLE);
  } else if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
    send_to_char("Couldn't find any such creature.\n\r", ch);
  else if (IS_NPC(vict))
    send_to_char("Can't do that to a beast.\n\r", ch);
  else if (GetMaxLevel(vict) > GetMaxLevel(ch))
    act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
  else
    send_to_char("The implementor won't let you set this on mortals...\n\r", ch);
}

void 
do_stealth(struct char_data *ch, char *argument, int cmd)
{
  struct char_data                *vict;
  struct obj_data                 *dummy;
  char                             buf[MAX_INPUT_LENGTH];

  if (IS_NPC(ch))
    return;

  only_argument(argument, buf);

  if (!*buf)
    if (IS_SET(ch->specials.act, PLR_STEALTH)) {
      send_to_char("STEALTH mode OFF.\n\r", ch);
      REMOVE_BIT(ch->specials.act, PLR_STEALTH);
    } else {
      send_to_char("STEALTH mode ON.\n\r", ch);
      SET_BIT(ch->specials.act, PLR_STEALTH);
  } else if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
    send_to_char("Couldn't find any such creature.\n\r", ch);
  else if (IS_NPC(vict))
    send_to_char("Can't do that to a beast.\n\r", ch);
  else if (GetMaxLevel(vict) > GetMaxLevel(ch))
    act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
  else
    send_to_char("The implementor won't let you set this on mortals...\n\r", ch);

}

static 
print_room(int rnum, struct room_data *rp, struct string_block *sb)
{
  char                             buf[MAX_STRING_LENGTH];
  int                              dink,
                                   bits,
                                   scan;

  sprintf(buf, "%5d %4d %-12s %s", rp->number, rnum,
	  sector_types[rp->sector_type], rp->name);
  strcat(buf, " [");

  dink = 0;
  for (bits = rp->room_flags, scan = 0; bits; scan++) {
    if (bits & (1 << scan)) {
      if (dink)
	strcat(buf, " ");
      strcat(buf, room_bits[scan]);
      dink = 1;
      bits ^= (1 << scan);
    }
  }
  strcat(buf, "]\n\r");

  append_to_string_block(sb, buf);
}

static void 
print_death_room(int rnum, struct room_data *rp, struct string_block *sb)
{
  if (rp && rp->room_flags & DEATH)
    print_room(rnum, rp, sb);
}

static void 
print_private_room(int rnum, struct room_data *rp, struct string_block *sb)
{
  if (rp && rp->room_flags & PRIVATE)
    print_room(rnum, rp, sb);
}

struct show_room_zone_struct {
  int                              blank;
  int                              startblank,
                                   lastblank;
  int                              bottom,
                                   top;
  struct string_block             *sb;
};

static void 
show_room_zone(int rnum, struct room_data *rp,
	       struct show_room_zone_struct *srzs)
{
  char                             buf[MAX_STRING_LENGTH];

  if (!rp || rp->number < srzs->bottom || rp->number > srzs->top)
    return;			/*
				 * optimize later
				 */

  if (srzs->blank && (srzs->lastblank + 1 != rp->number)) {
    sprintf(buf, "rooms %d-%d are blank\n\r", srzs->startblank, srzs->lastblank);
    append_to_string_block(srzs->sb, buf);
    srzs->blank = 0;
  }
  if (1 == sscanf(rp->name, "%d", &srzs->lastblank) &&
      srzs->lastblank == rp->number) {
    if (!srzs->blank) {
      srzs->startblank = srzs->lastblank;
      srzs->blank = 1;
    }
    return;
  } else if (srzs->blank) {
    sprintf(buf, "rooms %d-%d are blank\n\r", srzs->startblank, srzs->lastblank);
    append_to_string_block(srzs->sb, buf);
    srzs->blank = 0;
  }
  print_room(rnum, rp, srzs->sb);
}

void 
do_show(struct char_data *ch, char *argument, int cmd)
{
  int                              zone;
  char                             buf[MAX_STRING_LENGTH],
                                   zonenum[MAX_INPUT_LENGTH];
  struct index_data               *which_i;
  int                              bottom,
                                   top,
                                   topi;
  struct string_block              sb;

  if (IS_NPC(ch))
    return;

  argument = one_argument(argument, buf);
  init_string_block(&sb);

  if (is_abbrev(buf, "zones")) {
    struct zone_data                *zd;
    int                              bottom = 0;

    append_to_string_block(&sb,
			   "# Zone   name                                lifespan age     rooms     reset\n\r");

    for (zone = 0; zone <= top_of_zone_table; zone++) {
      char                            *mode;

      zd = zone_table + zone;
      switch (zd->reset_mode) {
      case 0:
	mode = "never";
	break;
      case 1:
	mode = "ifempty";
	break;
      case 2:
	mode = "always";
	break;
      default:
	mode = "!unknown!";
	break;
      }
      sprintf(buf, "%4d %-40s %4dm %4dm %6d-%-6d %s\n\r", zone, zd->name,
	      zd->lifespan, zd->age, bottom, zd->top, mode);
      append_to_string_block(&sb, buf);
      bottom = zd->top + 1;
    }
  } else if (is_abbrev(buf, "objects") &&
	     (which_i = obj_index, topi = top_of_objt) ||
	     is_abbrev(buf, "mobiles") &&
	     (which_i = mob_index, topi = top_of_mobt)) {
    int                              objn;
    struct index_data               *oi;

    only_argument(argument, zonenum);
    zone = -1;
    if (1 == sscanf(zonenum, "%i", &zone) && (zone < 0 || zone > top_of_zone_table)) {
      append_to_string_block(&sb, "That is not a valid zone_number\n\r");
      return;
    }
    if (zone >= 0) {
      bottom = zone ? (zone_table[zone - 1].top + 1) : 0;
      top = zone_table[zone].top;
    }
    append_to_string_block(&sb, "VNUM  rnum count names\n\r");
    for (objn = 0; objn <= topi; objn++) {
      oi = which_i + objn;
      if (zone >= 0 &&
	  (oi->virtual < bottom ||
	   oi->virtual > top) ||
	  zone < 0 &&
	  !isname(zonenum, oi->name))
	continue;		/*
				 * optimize later
				 */
      sprintf(buf, "%5d %4d %3d  %s\n\r", oi->virtual, objn, oi->number, oi->name);
      append_to_string_block(&sb, buf);
    }
  } else if (is_abbrev(buf, "rooms")) {
    only_argument(argument, zonenum);
    append_to_string_block(&sb, "VNUM  rnum type         name [BITS]\n\r");
    if (is_abbrev(zonenum, "death")) {
      hash_iterate(&room_db, print_death_room, &sb);
    } else if (is_abbrev(zonenum, "private")) {
      hash_iterate(&room_db, print_private_room, &sb);
    } else if (1 != sscanf(zonenum, "%i", &zone) || zone < 0 || zone > top_of_zone_table) {
      append_to_string_block(&sb, "I need a zone number with this command\n\r");
    } else {
      struct show_room_zone_struct     srzs;

      srzs.bottom = zone ? (zone_table[zone - 1].top + 1) : 0;
      srzs.top = zone_table[zone].top;
      srzs.blank = 0;
      srzs.sb = &sb;
      hash_iterate(&room_db, show_room_zone, &srzs);
      if (srzs.blank) {
	sprintf(buf, "rooms %d-%d are blank\n\r", srzs.startblank, srzs.lastblank);
	append_to_string_block(&sb, buf);
	srzs.blank = 0;
      }
    }
  } else {
    append_to_string_block(&sb, "Usage:\n\r"
			   "  show zones\n\r"
			 "  show (objects|mobiles) (zone#|name)\n\r"
		      "  show rooms (zone#|death|private)\n\r", ch);
  }
  page_string_block(&sb, ch);
  destroy_string_block(&sb);
}

void 
do_debug(struct char_data *ch, char *argument, int cmd)
{
  char                             arg[MAX_INPUT_LENGTH];
  int                              i;

  i = 0;
  one_argument(argument, arg);
  i = atoi(arg);

  if (i < 0 || i > 2) {
    send_to_char("valid values are 0, 1 and 2\n\r", ch);
  } else {
    if (i == 1) {
      DEBUG = 1;
      DEBUG2 = 0;
    } else if (i == 2) {
      DEBUG = 0;
      DEBUG2 = 1;
    } else {
      DEBUG = 0;
      DEBUG2 = 0;
    }
    sprintf(arg, "debug level set to %d\n\r", i);
    send_to_char(arg, ch);
  }
}

void 
do_invis(struct char_data *ch, char *argument, int cmd)
{
  char                             buf[MAX_STRING_LENGTH];
  int                              level;

  if (scan_number(argument, &level)) {
    if (level <= 0)
      level = 0;
    else {
      if (level > GetMaxLevel(ch)) {
	send_to_char("Sorry, you cant invis that high yet!\n\r", ch);
	return;
      }
    }
    ch->invis_level = level;
    sprintf(buf, "Invis level set to %d.\n\r", level);
    send_to_char(buf, ch);
  } else {
    if (ch->invis_level > 0) {
      ch->invis_level = 0;
      send_to_char("You are now totally VISIBLE.\n\r", ch);
    } else {
      ch->invis_level = GetMaxLevel(ch) - 1;
      sprintf(buf, "You are now invisible to level %d.\n\r", GetMaxLevel(ch) - 1);
      send_to_char(buf, ch);
    }
  }
}

/*
 * file: actwiz.c , Implementation of commands.           Part of DIKUMUD
 * Usage : Wizard Commands.
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */

#include <stdio.h>
#include <stdlib.h>
/* #include <unistd.h> */
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

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
#include "board.h"
#include "whod.h"
#include "reception.h"
#include "spec_procs.h"
#include "multiclass.h"
#include "act_skills.h"
#include "act_info.h"
#include "fight.h"
#include "hash.h"
#include "weather.h"
#include "modify.h"
#include "tracking.h"
#include "i3.h"
#define _ACT_WIZ_C
#include "act_wiz.h"

void do_polymorph(struct char_data *ch, const char *argument, int cmd)
{
    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    return;
}

void do_highfive(struct char_data *ch, const char *argument, int cmd)
{
    char                                    buf[80] = "\0\0\0\0\0\0\0";
    struct char_data                       *tch = NULL;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (argument) {
	only_argument(argument, buf);
	if ((tch = get_char_vis(ch, buf)) != 0) {
	    if ((GetMaxLevel(tch) >= DEMIGOD) && (!IS_NPC(tch))) {
		allprintf("Time stops for a moment as %s and %s high five.\r\n",
			  ch->player.name, tch->player.name);
	    } else {
		act("$n gives you a high five", TRUE, ch, 0, tch, TO_VICT);
		act("You give a hearty high five to $N", TRUE, ch, 0, tch, TO_CHAR);
		act("$n and $N do a high five.", TRUE, ch, 0, tch, TO_NOTVICT);
	    }
	} else {
	    cprintf(ch, "I don't see anyone here like that.\r\n");
	}
    }
}

void do_rentmode(struct char_data *ch, const char *argument, int cmd)
{
    char                                    buf[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    double                                  it = 0.0;
    FILE                                   *pfd = NULL;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_NPC(ch)) {
	cprintf(ch, "You cannot toggle rent costs.\r\n");
	return;
    }
    if (argument && *argument) {
	only_argument(argument, buf);
	if (sscanf(buf, " %lf ", &it) == 1)
	    RENT_RATE = it;
	cprintf(ch, "Rent now costs %f normal.", RENT_RATE);
	log_info("Rent now costs %f normal.", RENT_RATE);
    } else {
	if (RENT_RATE != 0.0) {
	    cprintf(ch, "Rent is now free!\r\n");
	    log_info("Rent cost is now ZERO.");
	    RENT_RATE = 0.0;
	} else {
	    cprintf(ch, "Rent is now normal.\r\n");
	    log_info("Rent cost is now normal.");
	    RENT_RATE = 1.0;
	}
    }
    if (!(pfd = fopen(RENTCOST_FILE, "w"))) {
	log_info("Cannot save rent cost!");
    } else {
	fprintf(pfd, "%f\n", RENT_RATE);
	FCLOSE(pfd);
    }
}

void do_wizlock(struct char_data *ch, const char *argument, int cmd)
{
    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_NPC(ch)) {
	cprintf(ch, "You cannot WizLock.\r\n");
	return;
    }
    if (WizLock) {
	cprintf(ch, "WizLock is now off\r\n");
	log_info("Wizlock is now off.");
	WizLock = FALSE;
    } else {
	cprintf(ch, "WizLock is now on\r\n");
	log_info("WizLock is now on.");
	WizLock = TRUE;
    }
}

void do_emote(struct char_data *ch, const char *argument, int cmd)
{
    int                                     i = 0;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_NPC(ch) && (cmd != 0) && (cmd != 214))
	return;

    for (i = 0; *(argument + i) == ' '; i++);

    if (!*(argument + i))
	cprintf(ch, "Yes.. But what?\r\n");
    else {
	act("$n %s", FALSE, ch, 0, 0, TO_ROOM, argument + i);
	if (IS_NPC(ch) || (IS_SET(ch->specials.act, PLR_ECHO))) {
	    cprintf(ch, "You emote: '%s'\r\n", argument + i);
	}
    }
}

void do_echo(struct char_data *ch, const char *argument, int cmd)
{
    int                                     i = 0;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_NPC(ch))
	return;

    for (i = 0; *(argument + i) == ' '; i++);

    if (!*(argument + i)) {
	if (IS_SET(ch->specials.act, PLR_ECHO)) {
	    cprintf(ch, "echo off\r\n");
	    REMOVE_BIT(ch->specials.act, PLR_ECHO);
	} else {
	    SET_BIT(ch->specials.act, PLR_ECHO);
	    cprintf(ch, "echo on\r\n");
	}
    } else {
	if (IS_IMMORTAL(ch)) {
	    reprintf(ch->in_room, ch, "%s\r\n", argument + i);
	    cprintf(ch, "Ok.\r\n");
	}
    }
}

void do_system(struct char_data *ch, const char *argument, int cmd)
{
    int                                     i = 0;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_NPC(ch))
	return;

    for (i = 0; *(argument + i) == ' '; i++);

    if (!*(argument + i))
	cprintf(ch, "That must be a mistake...\r\n");
    else {
	allprintf("\r\n%s\r\n", argument + i);
    }
}

void do_trans(struct char_data *ch, const char *argument, int cmd)
{
    struct descriptor_data                 *i = NULL;
    struct char_data                       *victim = NULL;
    char                                    buf[100] = "\0\0\0\0\0\0\0";
    short int                               target = 0;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_NPC(ch))
	return;

    only_argument(argument, buf);
    if (!*buf)
	cprintf(ch, "Who do you wich to transfer?\r\n");
    else if (str_cmp("all", buf)) {
	if (!(victim = get_char_vis_world(ch, buf, NULL)))
	    cprintf(ch, "No-one by that name around.\r\n");
	else if (GetMaxLevel(victim) > GetMaxLevel(ch)) {
	    cprintf(ch, "You are not strong enough to force %s to appear.\r\n", NAME(victim));
	    cprintf(victim, "%s would like to transfer you.\r\n", NAME(ch));
	} else {
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
	    cprintf(ch, "Ok.\r\n");
	}
    } else {						       /* Trans All */
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
	cprintf(ch, "Ok.\r\n");
    }
}

void do_at(struct char_data *ch, const char *argument, int cmd)
{
    char                                    command_str[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char                                    loc_str[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    int                                     location = 0;
    int                                     original_loc = 0;
    struct char_data                       *target_mob = NULL;
    struct obj_data                        *target_obj = NULL;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_NPC(ch))
	return;

    half_chop(argument, loc_str, command_str);
    if (!*loc_str) {
	cprintf(ch, "You must supply a room number or a name.\r\n");
	return;
    }
    if (!(target_mob = get_char_room_vis(ch, loc_str))) {
	if (!(target_mob = get_char(loc_str))) {
	    if (!(target_obj = get_obj_vis_world(ch, loc_str, NULL))) {
		if (!(location = atoi(loc_str))) {
		    cprintf(ch, "I have no idea where \"%s\" is...\r\n", loc_str);
		    return;
		} else if (!(real_roomp(location))) {
		    cprintf(ch, "That room exists only in your imagination.\r\n");
		    return;
		}
	    } else {
		if ((location = target_obj->in_room) == NOWHERE) {
		    cprintf(ch, "The object is not available.\r\n");
		    return;
		}
	    }
	} else {
	    if ((location = target_mob->in_room) == NOWHERE) {
		cprintf(ch, "The target mobile is not available.\r\n");
		return;
	    }
	}
    } else {
	if ((location = target_mob->in_room) == NOWHERE) {
	    cprintf(ch, "The target mobile is not available.\r\n");
	    return;
	}
    }
    /*
     * a location has been found. 
     */

    original_loc = ch->in_room;
    char_from_room(ch);
    char_to_room(ch, location);
    command_interpreter(ch, command_str);

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

void do_form(struct char_data *ch, const char *argument, int cmd)
{
    char                                    buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    int                                     loc_nr = 0;
    struct room_data                       *rp = NULL;
    int                                     zone = 0;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_NPC(ch))
	return;

    only_argument(argument, buf);
    if (!*buf) {
	cprintf(ch, "Usage: FORM virtual_number.\r\n");
	return;
    }
    if (!(isdigit(*buf))) {
	cprintf(ch, "Usage: FORM virtual_number.\r\n");
	return;
    }
    loc_nr = atoi(buf);
    if (real_roomp(loc_nr)) {
	cprintf(ch, "A room exists with that Vnum!\r\n");
	return;
    } else if (loc_nr < 0) {
	cprintf(ch, "You must use a positive Vnum!\r\n");
	return;
    }
    cprintf(ch, "You have formed a room.\r\n");
    allocate_room(loc_nr);
    rp = real_roomp(loc_nr);
    bzero(rp, sizeof(*rp));
    rp->number = loc_nr;
    if (top_of_zone_table >= 0) {
	for (zone = 0; rp->number > zone_table[zone].top && zone <= top_of_zone_table; zone++);
	if (zone > top_of_zone_table) {
	    log_info("Room %d is outside of any zone.\n", rp->number);
	    zone--;
	}
	rp->zone = zone;
    }
    snprintf(buf, MAX_INPUT_LENGTH, "%d", loc_nr);
    rp->name = (char *)strdup(buf);
    rp->description = (char *)strdup("New Room\n");
}

void do_goto(struct char_data *ch, const char *argument, int cmd)
{
    char                                    buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    int                                     loc_nr = 0;
    int                                     location = 0;
    int                                     i = 0;
    struct char_data                       *target_mob = NULL;
    struct char_data                       *pers = NULL;
    struct char_data                       *v = NULL;
    struct obj_data                        *target_obj = NULL;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_NPC(ch))
	return;

    only_argument(argument, buf);
    if (!*buf) {
	cprintf(ch, "You must supply a room number or a name.\r\n");
	return;
    }
    if (isdigit(*buf) && NULL == index(buf, '.')) {
	loc_nr = atoi(buf);
	if (NULL == real_roomp(loc_nr)) {
	    cprintf(ch, "No room exists with that number.\r\n");
	    return;
	}
	location = loc_nr;
    } else if ((target_mob = get_char_vis_world(ch, buf, NULL)))
	location = target_mob->in_room;
    else if ((target_obj = get_obj_vis_world(ch, buf, NULL)))
	if (target_obj->in_room != NOWHERE)
	    location = target_obj->in_room;
	else {
	    cprintf(ch, "The object is not available.\r\n");
	    cprintf(ch, "Try where #.object to nail its room number.\r\n");
	    return;
    } else {
	cprintf(ch, "No such creature or object around.\r\n");
	return;
    }

    /*
     * a location has been found. 
     */

    if (!real_roomp(location)) {
	log_error("Massive error in do_goto. Everyone Off NOW.");
	return;
    }
    if (IS_SET(real_roomp(location)->room_flags, PRIVATE)) {
	for (i = 0, pers = real_roomp(location)->people; pers; pers = pers->next_in_room, i++);
	if (i > 1 && IS_MORTAL(ch)) {
	    cprintf(ch, "There's a private conversation going on in that room.\r\n");
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

void do_home(struct char_data *ch, const char *argument, int cmd)
{
    char                                    buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    int                                     location = 0;
    struct char_data                       *v = NULL;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_NPC(ch))
	return;

    only_argument(argument, buf);
    if (!*buf) {
	location = GET_HOME(ch);
    } else {
	cprintf(ch, "You can't just barge into someone else's home (yet)!\r\n");
	location = GET_HOME(ch);
    }
    if (!real_roomp(location)) {
	cprintf(ch, "Hmmmm... homeless, peniless, but surely not alone.\r\n");
	return;
    }
    if (IS_SET(ch->specials.act, PLR_STEALTH)) {
	for (v = real_roomp(ch->in_room)->people; v; v = v->next_in_room) {
	    if ((ch != v) && (GetMaxLevel(v) >= GetMaxLevel(ch))) {
		act("$n stretches sensually and elongates into a wisp of vapour.", FALSE, ch, 0,
		    v, TO_VICT);
	    }
	}
    } else {
	act("$n stretches sensually and elongates into a wisp of vapour.", FALSE, ch, 0, 0,
	    TO_ROOM);
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
		act("$n arrives and immediately curls up in $s spot.", FALSE, ch, 0, v,
		    TO_VICT);
	    }
	}
    } else {
	act("$n arrives and immediately curls up in $s spot.", FALSE, ch, 0, 0, TO_ROOM);
    }
    do_look(ch, "", 15);
}

void do_apraise(struct char_data *ch, const char *argument, int cmd)
{
    char                                    arg1[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    char                                    buf[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    struct obj_data                        *j = NULL;
    int                                     i = 0;
    int                                     found_one = FALSE;
    int                                     chance = 0;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_NPC(ch))
	return;

    only_argument(argument, arg1);

    if (GET_MANA(ch) < 3) {
	cprintf(ch, "You can't seem to concentrate enough at the moment.\r\n");
	return;
    }
    chance = number(1, 101);

    if (chance > ch->skills[SKILL_APRAISE].learned) {
	cprintf(ch, "You are unable to apraise this item.\r\n");
	GET_MANA(ch) -= 1;
	return;
    }
    GET_MANA(ch) -= 3;

    if (!*arg1) {
	cprintf(ch, "apraise what?\r\n");
	return;
    } else {
	if (ch->skills[SKILL_APRAISE].learned < 50)
	    ch->skills[SKILL_APRAISE].learned++;

	/*
	 * apraise on object 
	 */
	if ((j = (struct obj_data *)get_obj_in_list_vis(ch, arg1, ch->carrying))) {
	    cprintf(ch, "Object name: [%s]\r\nItem type: ", j->name);
	    sprinttype(GET_ITEM_TYPE(j), item_types, buf);
	    cprintf(ch, "%s\r\n", buf);

	    cprintf(ch, "Can be worn on :");
	    sprintbit(j->obj_flags.wear_flags, wear_bits, buf);
	    cprintf(ch, "%s\r\n", buf);

	    cprintf(ch, "Weight: %d, Value: %d, Cost/day: %d\r\n",
		    j->obj_flags.weight, j->obj_flags.cost, j->obj_flags.cost_per_day);

	    switch (j->obj_flags.type_flag) {
		case ITEM_LIGHT:
		    cprintf(ch, "Light hours of Use : [%d]", j->obj_flags.value[2]);
		    break;
		case ITEM_WEAPON:
		    snprintf(buf, MAX_STRING_LENGTH, "Weapon Class:");
		    switch (j->obj_flags.value[3]) {
			case 0:
			    strlcat(buf, "Smiting Class.\r\n", MAX_STRING_LENGTH);
			    break;
			case 1:
			    strlcat(buf, "Stabbing Class.\r\n", MAX_STRING_LENGTH);
			    break;
			case 2:
			    strlcat(buf, "Whipping Class.\r\n", MAX_STRING_LENGTH);
			    break;
			case 3:
			    strlcat(buf, "Slashing Class.\r\n", MAX_STRING_LENGTH);
			    break;
			case 4:
			    strlcat(buf, "Smashing Class.\r\n", MAX_STRING_LENGTH);
			    break;
			case 5:
			    strlcat(buf, "Cleaving Class.\r\n", MAX_STRING_LENGTH);
			    break;
			case 6:
			    strlcat(buf, "Crushing Class.\r\n", MAX_STRING_LENGTH);
			    break;
			case 7:
			    strlcat(buf, "Bludgeoning Class.\r\n", MAX_STRING_LENGTH);
			    break;
			case 11:
			    strlcat(buf, "Piercing Class.\r\n", MAX_STRING_LENGTH);
			    break;
			default:
			    strlcat(buf, "Foreign Class to you....\r\n", MAX_STRING_LENGTH);
			    break;
		    }

		    found_one = 0;

		    for (i = 0; i < MAX_OBJ_AFFECT; i++) {
			if (j->affected[i].location == APPLY_HITROLL
			    || j->affected[i].location == APPLY_HITNDAM) {
			    found_one = 1;
			    switch (j->affected[i].modifier) {
				case 1:
				    strlcat(buf, "It is well balanced.\r\n", MAX_STRING_LENGTH);
				    break;
				case 2:
				    strlcat(buf, "It is very well balanced.\r\n", MAX_STRING_LENGTH);
				    break;
				case 3:
				    strlcat(buf, "It is a superb weapon.\r\n", MAX_STRING_LENGTH);
				    break;
				case 4:
				    strlcat(buf, "It was forged by the gods.\r\n", MAX_STRING_LENGTH);
				    break;
				case 5:
				    strlcat(buf, "It should not be in your hands.\r\n", MAX_STRING_LENGTH);
				    break;
				default:
				    strlcat(buf, "It will crack with the next blow.\r\n", MAX_STRING_LENGTH);
				    break;
			    }
			}
		    }

		    if (!found_one)
			strlcat(buf, "It is common in accuracy.\r\n", MAX_STRING_LENGTH);

		    found_one = 0;
		    for (i = 0; i < MAX_OBJ_AFFECT; i++) {
			if (j->affected[i].location == APPLY_DAMROLL
			    || j->affected[i].location == APPLY_HITNDAM) {
			    found_one = 1;
			    switch (j->affected[i].modifier) {
				case 0:
				    strlcat(buf, "It will surely damage its target.\r\n", MAX_STRING_LENGTH);
				    break;
				case 1:
				    strlcat(buf, "It looks to be made from a strong metal.\r\n", MAX_STRING_LENGTH);
				    break;
				case 2:
				    strlcat(buf, "This was forged from a mystical flame.\r\n", MAX_STRING_LENGTH);
				    break;
				case 3:
				    strlcat(buf, "It has definite magical charms.\r\n", MAX_STRING_LENGTH);
				    break;
				case 4:
				    strlcat(buf,
					   "This is definitately blessed by the gods.\r\n", MAX_STRING_LENGTH);
				    break;
				case 5:
				    strlcat(buf, "It is ready to lose its hilt.\r\n", MAX_STRING_LENGTH);
				    break;
				default:
				    strlcat(buf,
					   "It is checked badly and most likely will break.\r\n", MAX_STRING_LENGTH);
				    break;
			    }
			}
		    }
		    if (!found_one)
			strlcat(buf, "It has a common strength to its making.\r\n", MAX_STRING_LENGTH);
		    cprintf(ch, "%s", buf);

		    break;
		case ITEM_ARMOR:
		    snprintf(buf, MAX_STRING_LENGTH, "Effective AC points: [%d]\r\nWhen Repaired: [%d]",
			    j->obj_flags.value[0], j->obj_flags.value[1]);
		    if (j->obj_flags.value[0] != 0 && j->obj_flags.value[1] == 0) {
			strlcat(buf,
			       "\r\nYou should take it to be updated at the Blacksmith\r\n", MAX_STRING_LENGTH);
		    }
		    cprintf(ch, "%s", buf);
		    break;

	    }
	} else {
	    cprintf(ch, "I don't see that here.\r\n");
	}
    }
}

void do_stat(struct char_data *ch, const char *argument, int cmd)
{
    struct affected_type                   *aff = NULL;
    struct room_data                       *rm = NULL;
    struct char_data                       *k = NULL;
    struct obj_data                        *j = NULL;
    struct obj_data                        *j2 = NULL;
    struct extra_descr_data                *desc = NULL;
    struct follow_type                     *fol = NULL;
    struct room_data                       *rp = NULL;
    char                                    type[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    char                                    num[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    char                                    buf[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    char                                    buf2[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    int                                     i = 0;
    int                                     virtual = 0;
    int                                     i2 = 0;
    int                                     count = 0;
    int                                     found = FALSE;
    int                                     anumber = 0;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_NPC(ch))
	return;

    argument = one_argument(argument, type);
    only_argument(argument, num);
    if (!*num)
	anumber = -2;
    else if (isdigit(*num))
	anumber = atoi(num);
    else
	anumber = -1;

    /*
     * no argument 
     */
    if (!*type) {
	cprintf(ch, "Usage: stat < pc|mob|obj|room > [ name|vnum ]\r\n");
	return;
    }
    /*
     * ROOM 
     */
    if (!str_cmp("room", type) || !str_cmp("here", type)) {
	if (anumber < 0) {
	    if (anumber == -2)
		anumber = ch->in_room;
	    else {
		cprintf(ch, "Usage: stat room [vnum]\r\n");
		return;
	    }
	}
	rm = real_roomp(anumber);
	cprintf(ch,
		"Room Description: ---------------------------------------------------------\r\n%s",
		rm->description);
	if ((desc = rm->ex_description)) {
	    cprintf(ch,
		    "---------------------------------------------------------------------------\r\n");
	    *buf = '\0';
	    for (; desc; desc = desc->next) {
		strlcat(buf, desc->keyword, MAX_STRING_LENGTH);
		strlcat(buf, " ", MAX_STRING_LENGTH);
	    }
	    cprintf(ch, "Extras: %s\r\n", buf);
	}
	cprintf(ch,
		"---------------------------------------------------------------------------\r\n");
	sprinttype(rm->sector_type, sector_types, buf2);
	cprintf(ch, "%s [#%d], in Zone %s [#%d] is %s.\r\n",
		rm->name, rm->number, zone_table[rm->zone].name, rm->zone, buf2);
	if (rm->tele_targ > 0) {			       /* teleport room */
	    double                                  ttime =
		(double)rm->tele_time / (double)10.0;

	    rp = real_roomp(rm->tele_targ);
	    cprintf(ch, "Teleports to %s [#%d] every %3.1lf second%s",
		    rp ? rp->name : "Swirling CHAOS", rm->tele_targ,
		    ttime, (ttime != 1.0) ? "s.\r\n" : ".\r\n");
	}
	if ((rm->sector_type == SECT_WATER_SWIM) || (rm->sector_type == SECT_WATER_NOSWIM)) {
	    if (rm->river_dir != -1 && rm->dir_option[rm->river_dir]) {
		double                                  ttime =
		    (double)rm->river_speed / (double)10.0;

		rp = real_roomp(rm->dir_option[rm->river_dir]->to_room);
		cprintf(ch,
			"A River flows %s into %s [#%d] every %3.1lf second%s",
			dirs[rm->river_dir], rp ? rp->name : "Swirling CHAOS",
			rp ? rp->number : -1, ttime, (ttime != 1.0) ? "s.\r\n" : ".\r\n");
	    }
	}
	if (rm->room_flags) {
	    sprintbit((long)rm->room_flags, room_bits, buf);
	    cprintf(ch, "Flags: %s\r\n", buf);
	}
	if (rm->room_flags & SOUND) {
	    cprintf(ch, "Sound: %s", rm->sound);
	    cprintf(ch, "Sound: %s", rm->distant_sound);
	}
	if (rm->funct) {
	    cprintf(ch, "Special Procedure: %s.\r\n",
		    name_special_proc(SPECIAL_ROOM, rm->number));
	}
	for (i = 0; i < MAX_NUM_EXITS; i++) {
	    if (rm->dir_option[i]) {
		rp = real_roomp(rm->dir_option[i]->to_room);
		cprintf(ch, "Exit %s to %s [#%d] is called %s.\r\n", dirs[i],
			rp ? rp->name : "Swirling CHAOS", rp ? rp->number : -1,
			rm->dir_option[i]->keyword ? rm->dir_option[i]->keyword : dirs[i]);
		if (rm->dir_option[i]->general_description)
		    cprintf(ch, "     %s", rm->dir_option[i]->general_description);
		if (rm->dir_option[i]->exit_info) {
		    sprintbit(rm->dir_option[i]->exit_info, exit_bits, buf2);
		    cprintf(ch, "     Flags: %s\r\n", buf2);
		}
		if (rm->dir_option[i]->key > 0) {
		    cprintf(ch, "     Key: %s [#%d]\r\n",
			    obj_index[rm->dir_option[i]->key].name,
			    obj_index[rm->dir_option[i]->key].virtual);
		}
	    }
	}
	if ((k = rm->people)) {
	    cprintf(ch, "Lifeforms present:\r\n");
	    for (; k; k = k->next_in_room) {
		if (CAN_SEE(ch, k)) {
		    int                                     v = 0;

		    snprintf(buf, MAX_STRING_LENGTH, "%s", GET_NAME(k));
		    if (!(v = MobVnum(k)))
			strlcat(buf, "(PC)", MAX_STRING_LENGTH);
		    else if (v < 0)
			strlcat(buf, "(NPC)", MAX_STRING_LENGTH);
		    else
			scprintf(buf, MAX_STRING_LENGTH, " [#%d]", v);
		    cprintf(ch, "     %s\r\n", buf);
		}
	    }
	}
	if ((j = rm->contents)) {
	    cprintf(ch, "Objects present:\r\n");
	    for (; j; j = j->next_content)
		cprintf(ch, "     %s [#%d]\r\n", j->name, ObjVnum(j));
	}
	return;
    } else if (!str_cmp("mob", type) || !str_cmp("pc", type)) {
	count = 1;

	k = NULL;
	if (anumber < 0) {
	    if (anumber == -2)
		k = ch;
	}
	/*
	 * MOBILE in world 
	 */
	if (anumber >= 0) {
	    if (!(k = get_char_num(anumber))) {
		cprintf(ch, "Noone with that vnum exists, I shall load one!\r\n");
		if (!(k = read_mobile(anumber, VIRTUAL))) {
		    cprintf(ch, "No such creature exists in Reality!\r\n");
		    return;
		} else {
		    cprintf(ch, "%s appears for your inspection.\r\n", NAME(k));
		    char_to_room(k, ch->in_room);
		}
	    }
	} else if (!k) {
	    if (!str_cmp("me", num)) {
		k = ch;
	    } else if (!(k = get_char_room_vis(ch, num))) {
		if (!(k = get_char_vis_world(ch, num, &count))) {
		    int                                     x = 0;

		    cprintf(ch, "No creature exists by that name, I shall make one!\r\n");
		    for (x = 0; x < top_of_mobt; x++) {
			if (isname(num, mob_index[x].name)) {
			    if (!(k = read_mobile(x, REAL))) {
				cprintf(ch, "No such creature exists in Reality!\r\n");
				return;
			    } else {
				cprintf(ch, "%s appears for your inspection.\r\n", NAME(k));
				char_to_room(k, ch->in_room);
				x = -1;
				break;
			    }
			}
		    }
		    if (x > -1) {
			cprintf(ch, "No such creature exists in Reality!\r\n");
			return;
		    }
		}
	    }
	}
	cprintf(ch, "Name: %s  :  [R-Number %d]  ", GET_NAME(k), k->nr);
	if (IS_MOB(k))
	    cprintf(ch, "[Load Number %d]", mob_index[k->nr].virtual);
	cprintf(ch, "\r\n");

	cprintf(ch, "Location [%d]\r\n", k->in_room);

	switch (k->player.sex) {
	    case SEX_NEUTRAL:
		strlcpy(buf, "Neutral-Sex", MAX_STRING_LENGTH);
		break;
	    case SEX_MALE:
		strlcpy(buf, "Male", MAX_STRING_LENGTH);
		break;
	    case SEX_FEMALE:
		strlcpy(buf, "Female", MAX_STRING_LENGTH);
		break;
	    default:
		strlcpy(buf, "ILLEGAL-SEX!!", MAX_STRING_LENGTH);
		break;
	}

	cprintf(ch, "Sex : %s - %s\r\n",
		buf, (!IS_NPC(k) ? "Pc" : (!IS_MOB(k) ? "Npc" : "Mob")));

	cprintf(ch, "Short description: %s\r\n",
		(k->player.short_descr ? k->player.short_descr : "None"));
	cprintf(ch, "Title: %s\r\n", (k->player.title ? k->player.title : "None"));
	cprintf(ch, "Pre-Title: %s\r\n", (GET_PRETITLE(k) ? GET_PRETITLE(k) : "None"));
	cprintf(ch, "Long description: %s\r\n",
		(k->player.long_descr ? k->player.long_descr : "None"));

	if (IS_NPC(k)) {
	    strlcpy(buf, "Monster Class: ", MAX_STRING_LENGTH);
	    sprinttype(k->player.class, npc_class_types, buf2);
	} else {
	    strlcpy(buf, "Class: ", MAX_STRING_LENGTH);
	    sprintbit(k->player.class, pc_class_types, buf2);
	}
	strlcat(buf, buf2, MAX_STRING_LENGTH);

	snprintf(buf2, MAX_STRING_LENGTH, " :  Level [%d/%d/%d/%d/%d] : Alignment[%d]\r\n",
		k->player.level[0], k->player.level[1],
		k->player.level[2], k->player.level[3], k->player.level[4], GET_ALIGNMENT(k));

	strlcat(buf, buf2, MAX_STRING_LENGTH);
	cprintf(ch, "%s", buf);

	if (IS_PC(k)) {
	    cprintf(ch, "Birth : [%ld] secs, Logon[%ld] secs, Played[%d] secs\r\n",
		    k->player.time.birth, k->player.time.logon, k->player.time.played);

	    cprintf(ch, "Age: [%d] Years,  [%d] Months,  [%d] Days,  [%d] Hours\r\n",
		    age(k).year, age(k).month, age(k).day, age(k).hours);
	}
	cprintf(ch, "Height [%d]cm  Weight [%d]pounds \r\n", GET_HEIGHT(k), GET_WEIGHT(k));
	cprintf(ch, "+----------------------------+\r\n");
	cprintf(ch, "Str:[%d/%d]  Int:[%d]  Wis:[%d]  Dex:[%d]  Con:[%d]\r\n",
		GET_STR(k), GET_ADD(k), GET_INT(k), GET_WIS(k), GET_DEX(k), GET_CON(k));

	cprintf(ch,
		"Mana p.:[%d/%d+%d]  Hit p.:[%d/%d+%d]  Move p.:[%d/%d+%d]\r\n",
		GET_MANA(k), mana_limit(k), mana_gain(k),
		GET_HIT(k), hit_limit(k), hit_gain(k), GET_MOVE(k), move_limit(k),
		move_gain(k));

	cprintf(ch,
		"AC:[%d/10], Coins: [%d], Exp: [%d], Hitroll: [%d], Damroll: [%d]\r\n",
		GET_AC(k), GET_GOLD(k), GET_EXP(k), k->points.hitroll, k->points.damroll);

	if (IS_NPC(k)) {
	    cprintf(ch, "Npc Bare Hand Damage %dd%d.\r\n",
		    k->specials.damnodice, k->specials.damsizedice);
	}
	if (IS_PC(k)) {
	    cprintf(ch, "\r\nTimer [%d] \r\n", k->specials.timer);
	}
	cprintf(ch, "+----------------------------+\r\n");

	sprinttype(GET_POS(k), position_types, buf2);
	snprintf(buf, MAX_STRING_LENGTH, "Position: %s : Fighting: %s", buf2,
		((k->specials.fighting) ? GET_NAME(k->specials.fighting) : "Nobody"));
	if (k->desc) {
	    sprinttype(k->desc->connected, connected_types, buf2);
	    strlcat(buf, " : Connected: ", MAX_STRING_LENGTH);
	    strlcat(buf, buf2, MAX_STRING_LENGTH);
	}
	cprintf(ch, "%s\r\n", buf);

	strlcpy(buf, "Default position: ", MAX_STRING_LENGTH);
	sprinttype((k->specials.default_pos), position_types, buf2);
	strlcat(buf, buf2, MAX_STRING_LENGTH);
	if (IS_NPC(k)) {
	    strlcat(buf, "\r\nNPC flags: ", MAX_STRING_LENGTH);
	    sprintbit(k->specials.act, action_bits, buf2);
	} else {
	    strlcat(buf, ",PC flags: ", MAX_STRING_LENGTH);
	    sprintbit(k->specials.act, player_bits, buf2);
	}

	strlcat(buf, buf2, MAX_STRING_LENGTH);

	if (IS_MOB(k)) {
	    cprintf(ch, "\r\nMobile Special procedure : %s\r\n",
		    (mob_index[k->nr].func ? MobFunctionNameByFunc(mob_index[k->nr].func) :
		     "None"));
	}
	cprintf(ch, "Carried weight: %d   Carried items: %d\r\n",
		IS_CARRYING_W(k), IS_CARRYING_N(k));

	for (i = 0, j = k->carrying; j; j = j->next_content, i++);
	snprintf(buf, MAX_STRING_LENGTH, "Items in inventory: %d, ", i);

	for (i = 0, i2 = 0; i < MAX_WEAR; i++)
	    if (k->equipment[i])
		i2++;
	snprintf(buf2, MAX_STRING_LENGTH, "Items in equipment: %d\r\n", i2);
	strlcat(buf, buf2, MAX_STRING_LENGTH);
	cprintf(ch, "%s", buf);

	cprintf(ch, "Apply saving throws: [%d] [%d] [%d] [%d] [%d]\r\n",
		k->specials.apply_saving_throw[0],
		k->specials.apply_saving_throw[1],
		k->specials.apply_saving_throw[2],
		k->specials.apply_saving_throw[3], k->specials.apply_saving_throw[4]);

	if (IS_PC(k)) {
	    cprintf(ch, "Thirst: %d, Hunger: %d, Drunk: %d\r\n",
		    k->specials.conditions[THIRST],
		    k->specials.conditions[FULL], k->specials.conditions[DRUNK]);
	}
	cprintf(ch, "Master is '%s'\r\n", ((k->master) ? GET_NAME(k->master) : "NOBODY"));
	cprintf(ch, "Followers are:\r\n");
	for (fol = k->followers; fol; fol = fol->next)
	    if (CAN_SEE(ch, fol->follower))
		act("    $N", FALSE, ch, 0, fol->follower, TO_CHAR);

	/*
	 * immunities 
	 */
	cprintf(ch, "Immune to:");
	sprintbit(k->M_immune, immunity_names, buf);
	cprintf(ch, "%s\r\n", buf);
	/*
	 * resistances 
	 */
	cprintf(ch, "Resistant to:");
	sprintbit(k->immune, immunity_names, buf);
	cprintf(ch, "%s\r\n", buf);
	/*
	 * Susceptible 
	 */
	cprintf(ch, "Susceptible to:");
	sprintbit(k->susc, immunity_names, buf);
	cprintf(ch, "%s\r\n", buf);

	/*
	 * Showing the bitvector 
	 */
	sprintbit(k->specials.affected_by, affected_bits, buf);
	cprintf(ch, "Affected by: ");
	cprintf(ch, "%s\r\n", buf);

	/*
	 * Routine to show what spells a char is affected by 
	 */
	if (k->affected) {
	    cprintf(ch, "\r\nAffecting Spells:\r\n--------------\r\n");
	    for (aff = k->affected; aff; aff = aff->next) {
		cprintf(ch, "Spell : '%s'\r\n", spell_info[aff->type].name);
		cprintf(ch, "     Modifies %s by %d points\r\n",
			apply_types[(int)aff->location], aff->modifier);
		cprintf(ch, "     Expires in %3d hours, Bits set ", aff->duration);
		sprintbit(aff->bitvector, affected_bits, buf);
		cprintf(ch, "%s\r\n", buf);
	    }
	}
	return;
    } else if (!str_cmp("obj", type)) {
	count = 1;

	j = NULL;
	if (anumber == -2) {
	    cprintf(ch, "Usage: stat obj <name|vnum>\r\n");
	    return;
	}
	/*
	 * OBJECT in world 
	 */
	if (anumber >= 0) {
	    if (!(j = get_obj_num(anumber))) {
		cprintf(ch, "Nothing with that vnum exists, I shall load one!\r\n");
		if (!(j = read_object(anumber, VIRTUAL))) {
		    cprintf(ch, "No such object exists in Reality!\r\n");
		    return;
		} else {
		    cprintf(ch, "A new %s appears for your inspection.\r\n",
			    j->short_description);
		    obj_to_room(j, ch->in_room);
		}
	    }
	} else if (!j) {
	    if (!(j = get_obj_vis(ch, num))) {
		cprintf(ch, "No such object is visible in the Realm.\r\n");
		return;
	    }
	}
	virtual = (j->item_number >= 0) ? obj_index[j->item_number].virtual : 0;
	snprintf(buf, MAX_STRING_LENGTH, "Object name: [%s]\r\nR-number: [%d], Load Number: [%d]\r\nItem type: ",
		j->name, j->item_number, virtual);
	sprinttype(GET_ITEM_TYPE(j), item_types, buf2);
	strlcat(buf, buf2, MAX_STRING_LENGTH);
	cprintf(ch, "%s\r\n", buf);
	cprintf(ch, "Short description: %s\r\nLong description:\r\n%s\r\n",
		((j->short_description) ? j->short_description : "None"),
		((j->description) ? j->description : "None"));
	if (j->ex_description) {
	    strlcpy(buf, "Extra description keyword(s):\r\n----------\r\n", MAX_STRING_LENGTH);
	    for (desc = j->ex_description; desc; desc = desc->next) {
		strlcat(buf, desc->keyword, MAX_STRING_LENGTH);
		strlcat(buf, "\r\n", MAX_STRING_LENGTH);
	    }
	    strlcat(buf, "----------\r\n", MAX_STRING_LENGTH);
	    cprintf(ch, "%s", buf);
	} else {
	    strlcpy(buf, "Extra description keyword(s): None\r\n", MAX_STRING_LENGTH);
	    cprintf(ch, "%s", buf);
	}

	cprintf(ch, "Can be worn on :");
	sprintbit(j->obj_flags.wear_flags, wear_bits, buf);
	cprintf(ch, "%s\r\n", buf);

	cprintf(ch, "Set char bits  :");
	sprintbit(j->obj_flags.bitvector, affected_bits, buf);
	cprintf(ch, "%s\r\n", buf);

	cprintf(ch, "Extra flags: ");
	sprintbit(j->obj_flags.extra_flags, extra_bits, buf);
	cprintf(ch, "%s\r\n", buf);

	cprintf(ch, "Weight: %d, Value: %d, Cost/day: %d, Timer: %d\r\n",
		j->obj_flags.weight, j->obj_flags.cost,
		j->obj_flags.cost_per_day, j->obj_flags.timer);

	strlcpy(buf, "In room: ", MAX_STRING_LENGTH);
	if (j->in_room == NOWHERE)
	    strlcat(buf, "Nowhere", MAX_STRING_LENGTH);
	else {
	    snprintf(buf2, MAX_STRING_LENGTH, "%d", j->in_room);
	    strlcat(buf, buf2, MAX_STRING_LENGTH);
	}
	strlcat(buf, " ,In object: ", MAX_STRING_LENGTH);
	strlcat(buf, (!j->in_obj ? "None" : fname(j->in_obj->name)), MAX_STRING_LENGTH);

	/*
	 * strlcat(buf," ,Carried by:", MAX_STRING_LENGTH);
	 * if (j->carried_by) 
	 * {
	 * if (GET_NAME(j->carried_by)) 
	 * {
	 * if (strlen(GET_NAME(j->carried_by)) > 0) 
	 * {
	 * strlcat(buf, (!j->carried_by) ? "Nobody" : GET_NAME(j->carried_by), MAX_STRING_LENGTH);
	 * }
	 * else
	 * {
	 * strlcat(buf, "NonExistantPlayer", MAX_STRING_LENGTH);
	 * }
	 * }
	 * else 
	 * {
	 * strlcat(buf, "NonExistantPlayer", MAX_STRING_LENGTH);
	 * }
	 * }
	 * else 
	 * {
	 * strlcat(buf, "Nobody", MAX_STRING_LENGTH);
	 * }
	 * strlcat(buf,"\r\n", MAX_STRING_LENGTH);
	 * cprintf(ch, buf);
	 */
	switch (j->obj_flags.type_flag) {
	    case ITEM_LIGHT:
		snprintf(buf, MAX_STRING_LENGTH, "Colour : [%d]\r\nType : [%d]\r\nHours : [%d]",
			j->obj_flags.value[0], j->obj_flags.value[1], j->obj_flags.value[2]);
		break;
	    case ITEM_SCROLL:
		snprintf(buf, MAX_STRING_LENGTH, "Spells : %d, %d, %d, %d",
			j->obj_flags.value[0],
			j->obj_flags.value[1], j->obj_flags.value[2], j->obj_flags.value[3]);
		break;
	    case ITEM_WAND:
		snprintf(buf, MAX_STRING_LENGTH, "Spell : %d\r\nMana : %d", j->obj_flags.value[0],
			j->obj_flags.value[1]);
		break;
	    case ITEM_STAFF:
		snprintf(buf, MAX_STRING_LENGTH, "Spell : %d\r\nMana : %d", j->obj_flags.value[0],
			j->obj_flags.value[1]);
		break;
	    case ITEM_WEAPON:
		snprintf(buf, MAX_STRING_LENGTH, "Tohit : %d\r\nTodam : %dD%d\r\nType : %d",
			j->obj_flags.value[0],
			j->obj_flags.value[1], j->obj_flags.value[2], j->obj_flags.value[3]);
		break;
	    case ITEM_FIREWEAPON:
		snprintf(buf, MAX_STRING_LENGTH, "Tohit : %d\r\nTodam : %dD%d\r\nType : %d",
			j->obj_flags.value[0],
			j->obj_flags.value[1], j->obj_flags.value[2], j->obj_flags.value[3]);
		break;
	    case ITEM_MISSILE:
		snprintf(buf, MAX_STRING_LENGTH, "Tohit : %d\r\nTodam : %d\r\nType : %d",
			j->obj_flags.value[0], j->obj_flags.value[1], j->obj_flags.value[3]);
		break;
	    case ITEM_ARMOR:
		snprintf(buf, MAX_STRING_LENGTH, "AC-apply : [%d]\r\nFull Strength : [%d]",
			j->obj_flags.value[0], j->obj_flags.value[1]);

		break;
	    case ITEM_POTION:
		snprintf(buf, MAX_STRING_LENGTH, "Spells : %d, %d, %d, %d",
			j->obj_flags.value[0],
			j->obj_flags.value[1], j->obj_flags.value[2], j->obj_flags.value[3]);
		break;
	    case ITEM_TRAP:
		snprintf(buf, MAX_STRING_LENGTH, "level: %d, att type: %d, damage class: %d, charges: %d",
			j->obj_flags.value[0],
			j->obj_flags.value[1], j->obj_flags.value[2], j->obj_flags.value[3]);
		break;
	    case ITEM_CONTAINER:
		snprintf(buf, MAX_STRING_LENGTH, "Max-contains : %d\r\nLocktype : %d\r\nCorpse : %s",
			j->obj_flags.value[0],
			j->obj_flags.value[1], j->obj_flags.value[3] ? "Yes" : "No");
		break;
	    case ITEM_DRINKCON:
		sprinttype(j->obj_flags.value[2], drinks, buf2);
		snprintf(buf, MAX_STRING_LENGTH,
			"Max-contains : %d\r\nContains : %d\r\nPoisoned : %d\r\nLiquid : %s",
			j->obj_flags.value[0], j->obj_flags.value[1], j->obj_flags.value[3],
			buf2);
		break;
	    case ITEM_NOTE:
		snprintf(buf, MAX_STRING_LENGTH, "Tounge : %d", j->obj_flags.value[0]);
		break;
	    case ITEM_KEY:
		snprintf(buf, MAX_STRING_LENGTH, "Keytype : %d", j->obj_flags.value[0]);
		break;
	    case ITEM_FOOD:
		snprintf(buf, MAX_STRING_LENGTH, "Makes full : %d\r\nPoisoned : %d",
			j->obj_flags.value[0], j->obj_flags.value[3]);
		break;
	    default:
		snprintf(buf, MAX_STRING_LENGTH, "Values 0-3 : [%d] [%d] [%d] [%d]",
			j->obj_flags.value[0],
			j->obj_flags.value[1], j->obj_flags.value[2], j->obj_flags.value[3]);
		break;
	}
	cprintf(ch, "%s", buf);

	strlcpy(buf, "\r\nEquipment Status: ", MAX_STRING_LENGTH);
	if (!j->carried_by)
	    strlcat(buf, "NONE", MAX_STRING_LENGTH);
	else {
	    found = FALSE;
	    for (i = 0; i < MAX_WEAR; i++) {
		if (j->carried_by->equipment[i] == j) {
		    sprinttype(i, equipment_types, buf2);
		    strlcat(buf, buf2, MAX_STRING_LENGTH);
		    found = TRUE;
		}
	    }
	    if (!found)
		strlcat(buf, "Inventory", MAX_STRING_LENGTH);
	}
	cprintf(ch, "%s", buf);

	strlcpy(buf, "\r\nSpecial procedure : ", MAX_STRING_LENGTH);
	if (j->item_number >= 0)
	    strlcat(buf, (obj_index[j->item_number].func ? "exists\r\n" : "No\r\n"), MAX_STRING_LENGTH);
	else
	    strlcat(buf, "No\r\n", MAX_STRING_LENGTH);
	cprintf(ch, "%s", buf);

	strlcpy(buf, "Contains :\r\n", MAX_STRING_LENGTH);
	found = FALSE;
	for (j2 = j->contains; j2; j2 = j2->next_content) {
	    strlcat(buf, fname(j2->name), MAX_STRING_LENGTH);
	    strlcat(buf, "\r\n", MAX_STRING_LENGTH);
	    found = TRUE;
	}
	if (!found)
	    strlcpy(buf, "Contains : Nothing\r\n", MAX_STRING_LENGTH);
	cprintf(ch, "%s", buf);

	cprintf(ch, "Can affect char :\r\n");
	for (i = 0; i < MAX_OBJ_AFFECT; i++) {
	    sprinttype(j->affected[i].location, apply_types, buf2);
	    cprintf(ch, "    Affects : %s By %d\r\n", buf2, j->affected[i].modifier);
	}
	return;
    } else {
	cprintf(ch, "Usage: stat < pc|mob|obj|room > [ name|vnum ]\r\n");
	return;
    }
}

void do_pretitle(struct char_data *ch, const char *argument, int cmd)
{
    char                                    name[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char                                    pretitle[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    struct char_data                       *vict = NULL;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    argument = one_argument(argument, name);
    if (*argument == ' ')
	argument++;
    strlcpy(pretitle, argument, MAX_INPUT_LENGTH);

    if (!(vict = get_char_vis(ch, name))) {
	cprintf(ch, "I don't see them here?\r\n");
	return;
    }
    if (!strlen(pretitle)) {
	GET_PRETITLE(vict) = NULL;
	return;
    }
    GET_PRETITLE(vict) = strdup(pretitle);
}

void do_set(struct char_data *ch, const char *argument, int cmd)
{
    struct char_data                       *mob = NULL;
    char                                    field[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char                                    name[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char                                    parmstr[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char                                    tmp[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char                                    buf[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    int                                     index_value = 0;
    int                                     parm = 0;
    int                                     i = 0;
    int                                     no = 0;

    const char                             *pset_list[] = {
	"align", "exp", "sex", "race", "tohit", "dmg",
	"bank", "gold", "prac",
	"str", "int", "wis", "dex", "con", "stradd",
	"hit", "mhit", "mana", "mmana", "move", "mmove",
	"mlvl", "clvl", "wlvl", "tlvl", "rlvl", "dlvl",
	"aggr", "wander",
	NULL
    };

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_NPC(ch))
	return;

    argument = one_argument(argument, name);
    argument = one_argument(argument, field);
    strlcpy(tmp, argument, MAX_INPUT_LENGTH);
    argument = one_argument(argument, parmstr);

    if ((mob = get_char_vis(ch, name)) == NULL) {
	cprintf(ch, "I don't see them here? \r\n\r\n");
	*buf = '\0';
	strlcpy(buf, "Usage:  pset <name> <attrib> <value>\r\n", MAX_STRING_LENGTH);
	for (no = 1, i = 0; pset_list[i]; i++) {
	    scprintf(buf, MAX_STRING_LENGTH, "%-10s", pset_list[i]);
	    if (!(no % 7))
		strlcat(buf, "\r\n", MAX_STRING_LENGTH);
	    no++;
	}
	cprintf(ch, "%s\r\n", buf);
	return;
    }
    for (index_value = 0; pset_list[index_value]; index_value++)
	if (!strcmp(field, pset_list[index_value])) {
	    int                                     x;

	    x = sscanf(parmstr, "%d", &parm);
	    if (!x) {
		cprintf(ch, "You must also supply a value\r\n");
		return;
	    }
	    break;
	}
    if (IS_PC(mob) && mob != ch && GetMaxLevel(mob) >= GetMaxLevel(ch)) {
	cprintf(ch, "You wish you could set %s's stats...\r\n", GET_NAME(mob));
	return;
    }
    switch (index_value) {
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
	    GET_HITROLL(mob) = parm;
	    break;
	case 5:
	    GET_DAMROLL(mob) = parm;
	    break;
	case 6:
	    GET_BANK(mob) = parm;
	    break;
	case 7:
	    GET_GOLD(mob) = parm;
	    break;
	case 8:
	    mob->specials.pracs = parm;
	    break;
	case 9:
	    if (ch == mob && parm > 25 && GetMaxLevel(ch) < LOKI) {
		cprintf(ch, "Sure, we all want to be more powerful.\r\n");
		return;
	    }
	    mob->abilities.str = parm;
	    mob->tmpabilities = mob->abilities;
	    break;
	case 10:
	    if (ch == mob && parm > 25 && GetMaxLevel(ch) < LOKI) {
		cprintf(ch, "Sure, we all want to be more powerful.\r\n");
		return;
	    }
	    mob->abilities.intel = parm;
	    mob->tmpabilities = mob->abilities;
	    break;
	case 11:
	    if (ch == mob && parm > 25 && GetMaxLevel(ch) < LOKI) {
		cprintf(ch, "Sure, we all want to be more powerful.\r\n");
		return;
	    }
	    mob->abilities.wis = parm;
	    mob->tmpabilities = mob->abilities;
	    break;
	case 12:
	    if (ch == mob && parm > 25 && GetMaxLevel(ch) < LOKI) {
		cprintf(ch, "Sure, we all want to be more powerful.\r\n");
		return;
	    }
	    mob->abilities.dex = parm;
	    mob->tmpabilities = mob->abilities;
	    break;
	case 13:
	    if (ch == mob && parm > 25 && GetMaxLevel(ch) < LOKI) {
		cprintf(ch, "Sure, we all want to be more powerful.\r\n");
		return;
	    }
	    mob->abilities.con = parm;
	    mob->tmpabilities = mob->abilities;
	    break;
	case 14:
	    mob->abilities.str_add = parm;
	    mob->tmpabilities = mob->abilities;
	    break;
	case 15:
	    GET_HIT(mob) = parm;
	    break;
	case 16:
	    mob->points.max_hit = parm;
	    break;
	case 17:
	    GET_MANA(mob) = parm;
	    break;
	case 18:
	    mob->points.max_mana = parm;
	    break;
	case 19:
	    GET_MOVE(mob) = parm;
	    break;
	case 20:
	    mob->points.max_move = parm;
	    break;
	case 21:
	    if (ch == mob && parm > GET_LEVEL(ch, MAGE_LEVEL_IND)) {
		cprintf(ch, "Sure, we all want to be more powerful.\r\n");
		return;
	    }
	    if (ch != mob && IS_IMMORTAL(mob) && str_cmp(GET_NAME(ch), "Quixadhal")
		&& parm > GET_LEVEL(ch, MAGE_LEVEL_IND)) {
		cprintf(ch, "Ask the Dread Lord to make %s mightier!\r\n", GET_NAME(mob));
		return;
	    }
	    if (parm < 1) {
		GET_CLASS(mob) &= ~CLASS_MAGIC_USER;
	    } else {
		GET_CLASS(mob) |= CLASS_MAGIC_USER;
	    }
	    GET_LEVEL(mob, MAGE_LEVEL_IND) = parm;
	    break;
	case 22:
	    if (ch == mob && parm > GET_LEVEL(ch, CLERIC_LEVEL_IND)) {
		cprintf(ch, "Sure, we all want to be more powerful.\r\n");
		return;
	    }
	    if (ch != mob && IS_IMMORTAL(mob) && str_cmp(GET_NAME(ch), "Quixadhal")
		&& parm > GET_LEVEL(ch, CLERIC_LEVEL_IND)) {
		cprintf(ch, "Ask the Dread Lord to make %s mightier!\r\n", GET_NAME(mob));
		return;
	    }
	    if (parm < 1) {
		GET_CLASS(mob) &= ~CLASS_CLERIC;
	    } else {
		GET_CLASS(mob) |= CLASS_CLERIC;
	    }
	    GET_LEVEL(mob, CLERIC_LEVEL_IND) = parm;
	    break;
	case 23:
	    if (ch == mob && parm > GET_LEVEL(ch, WARRIOR_LEVEL_IND)) {
		cprintf(ch, "Sure, we all want to be more powerful.\r\n");
		return;
	    }
	    if (ch != mob && IS_IMMORTAL(mob) && str_cmp(GET_NAME(ch), "Quixadhal")
		&& parm > GET_LEVEL(ch, WARRIOR_LEVEL_IND)) {
		cprintf(ch, "Ask the Dread Lord to make %s mightier!\r\n", GET_NAME(mob));
		return;
	    }
	    if (parm < 1) {
		GET_CLASS(mob) &= ~CLASS_WARRIOR;
	    } else {
		GET_CLASS(mob) |= CLASS_WARRIOR;
	    }
	    GET_LEVEL(mob, WARRIOR_LEVEL_IND) = parm;
	    break;
	case 24:
	    if (ch == mob && parm > GET_LEVEL(ch, THIEF_LEVEL_IND)) {
		cprintf(ch, "Sure, we all want to be more powerful.\r\n");
		return;
	    }
	    if (ch != mob && IS_IMMORTAL(mob) && str_cmp(GET_NAME(ch), "Quixadhal")
		&& parm > GET_LEVEL(ch, THIEF_LEVEL_IND)) {
		cprintf(ch, "Ask the Dread Lord to make %s mightier!\r\n", GET_NAME(mob));
		return;
	    }
	    if (parm < 1) {
		GET_CLASS(mob) &= ~CLASS_THIEF;
	    } else {
		GET_CLASS(mob) |= CLASS_THIEF;
	    }
	    GET_LEVEL(mob, THIEF_LEVEL_IND) = parm;
	    break;
	case 25:
	    if (ch == mob && parm > GET_LEVEL(ch, RANGER_LEVEL_IND)) {
		cprintf(ch, "Sure, we all want to be more powerful.\r\n");
		return;
	    }
	    if (ch != mob && IS_IMMORTAL(mob) && str_cmp(GET_NAME(ch), "Quixadhal")
		&& parm > GET_LEVEL(ch, RANGER_LEVEL_IND)) {
		cprintf(ch, "Ask the Dread Lord to make %s mightier!\r\n", GET_NAME(mob));
		return;
	    }
	    if (parm < 1) {
		GET_CLASS(mob) &= ~CLASS_RANGER;
	    } else {
		GET_CLASS(mob) |= CLASS_RANGER;
	    }
	    GET_LEVEL(mob, RANGER_LEVEL_IND) = parm;
	    break;
	case 26:
	    if (ch == mob && parm > GET_LEVEL(ch, DRUID_LEVEL_IND)) {
		cprintf(ch, "Sure, we all want to be more powerful.\r\n");
		return;
	    }
	    if (ch != mob && IS_IMMORTAL(mob) && str_cmp(GET_NAME(ch), "Quixadhal")
		&& parm > GET_LEVEL(ch, DRUID_LEVEL_IND)) {
		cprintf(ch, "Ask the Dread Lord to make %s mightier!\r\n", GET_NAME(mob));
		return;
	    }
	    if (parm < 1) {
		GET_CLASS(mob) &= ~CLASS_DRUID;
	    } else {
		GET_CLASS(mob) |= CLASS_DRUID;
	    }
	    GET_LEVEL(mob, DRUID_LEVEL_IND) = parm;
	    break;
	case 27:
	    if (!IS_NPC(mob)) {
		cprintf(ch, "You should tell %s to be more aggressive!\r\n", GET_NAME(mob));
		return;
	    }
	    if (parm) {
		if (!IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
		    SET_BIT(mob->specials.act, ACT_AGGRESSIVE);
		    cprintf(ch, "%s is now AGGRESSIVE!\r\n", NAME(mob));
		}
	    } else {
		if (IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
		    REMOVE_BIT(mob->specials.act, ACT_AGGRESSIVE);
		    cprintf(ch, "%s is now nice.\r\n", NAME(mob));
		}
	    }
	    break;
	case 28:
	    if (!IS_NPC(mob)) {
		cprintf(ch, "You should tell %s to wander about more!\r\n", GET_NAME(mob));
		return;
	    }
	    if (parm) {
		if (IS_SET(mob->specials.act, ACT_SENTINEL)) {
		    REMOVE_BIT(mob->specials.act, ACT_SENTINEL);
		    cprintf(ch, "%s is now wandering!\r\n", NAME(mob));
		}
	    } else {
		if (!IS_SET(mob->specials.act, ACT_SENTINEL)) {
		    SET_BIT(mob->specials.act, ACT_SENTINEL);
		    cprintf(ch, "%s is now lazy.\r\n", NAME(mob));
		}
	    }
	    break;
	default:
	    *buf = '\0';
	    strlcpy(buf, "Usage:  pset <name> <attrib> <value>\r\n", MAX_STRING_LENGTH);
	    for (no = 1, i = 0; pset_list[i]; i++) {
		scprintf(buf, MAX_STRING_LENGTH, "%-10s", pset_list[i]);
		if (!(no % 7))
		    strlcat(buf, "\r\n", MAX_STRING_LENGTH);
		no++;
	    }
	    cprintf(ch, "%s\r\n", buf);
    }
}

void do_shutdow(struct char_data *ch, const char *argument, int cmd)
{
    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    cprintf(ch, "If you want to shut something down - say so!\r\n");
}

void do_shutdown(struct char_data *ch, const char *argument, int cmd)
{
    time_t                                  tc = (time_t) 0;
    struct tm                              *t_info = NULL;
    char                                   *tmstr = NULL;
    char                                    arg[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_NPC(ch))
	return;

    tc = time(0);
    t_info = localtime(&tc);
    tmstr = asctime(t_info);
    *(tmstr + strlen(tmstr) - 1) = '\0';

    one_argument(argument, arg);

    if (!*arg) {
	log_boot("SHUTDOWN by %s at %d:%d", GET_NAME(ch), t_info->tm_hour + 1, t_info->tm_min);
	allprintf("\x007\r\nBroadcast message from %s (tty0) %s...\r\n\r\n", GET_NAME(ch),
		  tmstr);
	allprintf("\x007The system is going down NOW !!\r\n\x007\r\n");
        i3_log_dead();
	diku_shutdown = 1;
	update_time_and_weather();
    } else if (!str_cmp(arg, "-k")) {
	log_info("FAKE REBOOT by %s at %d:%d", GET_NAME(ch),
		 t_info->tm_hour + 1, t_info->tm_min);
	allprintf("\x007\r\nBroadcast message from %s (tty0) %s...\r\n\r\n", GET_NAME(ch),
		  tmstr);
	allprintf("\x007Rebooting.  Come back in a few minutes!\r\n");
	allprintf("\x007The system is going down NOW !!\r\n\r\n");
    } else if (!str_cmp(arg, "-r")) {
	log_boot("REBOOT by %s at %d:%d", GET_NAME(ch), t_info->tm_hour + 1, t_info->tm_min);
	allprintf("\x007\r\nBroadcast message from %s (tty0) %s...\r\n\r\n", GET_NAME(ch),
		  tmstr);
	allprintf("\x007Rebooting.  Come back in a few minutes!\r\n");
	allprintf("\x007The system is going down NOW !!\r\n\r\n");
        i3_log_dead();
	diku_shutdown = diku_reboot = 1;
	update_time_and_weather();
    } else
	cprintf(ch, "Go shut down someone your own size.\r\n");
}

void do_snoop(struct char_data *ch, const char *argument, int cmd)
{
    static char                             arg[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    struct char_data                       *victim = NULL;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (!ch->desc)
	return;

    if (IS_NPC(ch))
	return;

    only_argument(argument, arg);

    if (!*arg) {
	cprintf(ch, "Snoop who ?\r\n");
	return;
    }
    if (!(victim = get_char_vis(ch, arg))) {
	cprintf(ch, "No such person around.\r\n");
	return;
    }
    if (!victim->desc) {
	cprintf(ch, "There's no link.. nothing to snoop.\r\n");
	return;
    }
    if (victim == ch) {
	cprintf(ch, "Ok, you just snoop yourself.\r\n");
	if (ch->desc->snoop.snooping) {
	    if (ch->desc->snoop.snooping->desc)
		ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;
	    else
		log_info("caught %s snooping %s who didn't have a descriptor!",
			 ch->player.name, ch->desc->snoop.snooping->player.name);
	    ch->desc->snoop.snooping = 0;
	}
	return;
    }
    if (victim->desc->snoop.snoop_by) {
	cprintf(ch, "Busy already. \r\n");
	return;
    }
    if (GetMaxLevel(victim) >= GetMaxLevel(ch)) {
	cprintf(ch, "You failed.\r\n");
	return;
    }
    cprintf(ch, "Ok. \r\n");

    if (ch->desc->snoop.snooping)
	if (ch->desc->snoop.snooping->desc)
	    ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;

    ch->desc->snoop.snooping = victim;
    victim->desc->snoop.snoop_by = ch;
    return;
}

void do_switch(struct char_data *ch, const char *argument, int cmd)
{
    static char                             arg[80] = "\0\0\0\0\0\0\0";
    struct char_data                       *victim = NULL;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_NPC(ch))
	return;

    only_argument(argument, arg);

    if (!*arg) {
	cprintf(ch, "Switch with who?\r\n");
    } else {
	if (!(victim = get_char_room_vis(ch, arg))) {
	    if (!(victim = get_char(arg))) {
		cprintf(ch, "They aren't here.\r\n");
		return;
	    }
	}
	{
	    if (ch == victim) {
		cprintf(ch, "He he he... We are jolly funny today, eh?\r\n");
		return;
	    }
	    if (!ch->desc || ch->desc->snoop.snoop_by || ch->desc->snoop.snooping) {
		cprintf(ch, "Mixing snoop & switch is bad for your health.\r\n");
		return;
	    }
	    if (victim->desc || (!IS_NPC(victim)) || IS_SET(victim->specials.act, ACT_SWITCH)) {
		cprintf(ch, "You can't do that, the body is already in use!\r\n");
	    } else {
		cprintf(ch, "Ok.\r\n");

		ch->desc->character = victim;
		ch->desc->original = ch;

		SET_BIT(victim->specials.act, ACT_SWITCH);
		victim->desc = ch->desc;
		ch->desc = 0;
	    }
	}
    }
}

void do_return(struct char_data *ch, const char *argument, int cmd)
{
    struct char_data                       *mob = NULL;
    struct char_data                       *per = NULL;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (!ch->desc)
	return;

    if (!ch->desc->original || (IS_NOT_SET(ch->specials.act, ACT_SWITCH) &&
				IS_NOT_SET(ch->specials.act, ACT_POLYSELF) &&
				IS_NOT_SET(ch->specials.act, ACT_POLYOTHER))) {
	cprintf(ch, "Huh?  Talk sense I can't understand you.\r\n");
	return;
    } else {
	cprintf(ch, "You return to your original body.\r\n");

	if ((IS_SET(ch->specials.act, ACT_POLYSELF) ||
	     IS_SET(ch->specials.act, ACT_POLYOTHER)) && cmd) {
	    mob = ch;
	    per = ch->desc->original;

	    act("$n turns liquid, and reforms as $N", TRUE, mob, 0, per, TO_ROOM);

	    char_from_room(per);
	    char_to_room(per, mob->in_room);

	    /*
	     * SwitchStuff(mob, per); 
	     */
	}
	if (IS_SET(ch->specials.act, ACT_SWITCH))
	    REMOVE_BIT(ch->specials.act, ACT_SWITCH);
	ch->desc->character = ch->desc->original;
	ch->desc->original = 0;

	ch->desc->character->desc = ch->desc;
	ch->desc = 0;

	if ((IS_SET(ch->specials.act, ACT_POLYSELF) ||
	     IS_SET(ch->specials.act, ACT_POLYOTHER)) && cmd) {
	    extract_char(mob);
	}
    }
}

void do_force(struct char_data *ch, const char *argument, int cmd)
{
    struct descriptor_data                 *i = NULL;
    struct char_data                       *vict = NULL;
    char                                    name[100] = "\0\0\0\0\0\0\0";
    char                                    to_force[100] = "\0\0\0\0\0\0\0";

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_NPC(ch) && (cmd != 0))
	return;

    half_chop(argument, name, to_force);

    if (!*name || !*to_force)
	cprintf(ch, "Who do you wish to force to do what?\r\n");
    else if (str_cmp("all", name)) {
	if (!(vict = get_char_vis(ch, name)))
	    cprintf(ch, "No-one by that name here..\r\n");
	else {
	    if ((GetMaxLevel(ch) <= GetMaxLevel(vict)) && (!IS_NPC(vict)))
		cprintf(ch, "Oh no you don't!!\r\n");
	    else {
		if (!IS_SET(ch->specials.act, PLR_STEALTH))
		    act("$n has forced you to '%s'.", FALSE, ch, 0, vict, TO_VICT, to_force);
		cprintf(ch, "Ok.\r\n");
		command_interpreter(vict, to_force);
	    }
	}
    } else {						       /* force all */
	for (i = descriptor_list; i; i = i->next)
	    if (i->character != ch && !i->connected && i->character != board_kludge_char) {
		vict = i->character;
		if ((GetMaxLevel(ch) <= GetMaxLevel(vict)) && (!IS_NPC(vict)))
		    cprintf(ch, "Oh no you don't!!\r\n");
		else {
		    if (!IS_SET(ch->specials.act, PLR_STEALTH))
			act("$n has forced you to '%s'.", FALSE, ch, 0, vict, TO_VICT,
			    to_force);
		    command_interpreter(vict, to_force);
		}
	    }
	cprintf(ch, "Ok.\r\n");
    }
}

void do_load(struct char_data *ch, const char *argument, int cmd)
{
    struct char_data                       *mob = NULL;
    struct obj_data                        *obj = NULL;
    char                                    type[100] = "\0\0\0\0\0\0\0";
    char                                    num[100] = "\0\0\0\0\0\0\0";
    int                                     anumber = 0;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_NPC(ch))
	return;

    argument = one_argument(argument, type);

    only_argument(argument, num);
    if (isdigit(*num))
	anumber = atoi(num);
    else
	anumber = -1;

    if (is_abbrev(type, "mobile")) {
	if (anumber < 0) {
	    for (anumber = 0; anumber <= top_of_mobt; anumber++)
		if (isname(num, mob_index[anumber].name))
		    break;
	    if (anumber > top_of_mobt)
		anumber = -1;
	} else {
	    anumber = real_mobile(anumber);
	}
	if (anumber < 0 || anumber > top_of_mobt) {
	    cprintf(ch, "There is no such monster.\r\n");
	    return;
	}
	mob = read_mobile(anumber, REAL);
	char_to_room(mob, ch->in_room);

	act("$n makes a quaint, magical gesture with one hand.", TRUE, ch, 0, 0, TO_ROOM);
	act("$n has summoned $N from the ether!", FALSE, ch, 0, mob, TO_ROOM);
	act("You bring forth $N from the the cosmic ether.", FALSE, ch, 0, mob, TO_CHAR);
    } else if (is_abbrev(type, "object")) {
	if (anumber < 0) {
	    for (anumber = 0; anumber <= top_of_objt; anumber++)
		if (isname(num, obj_index[anumber].name))
		    break;
	    if (anumber > top_of_objt)
		anumber = -1;
	} else {
	    anumber = real_object(anumber);
	}
	if (anumber < 0 || anumber > top_of_objt) {
	    cprintf(ch, "There is no such object.\r\n");
	    return;
	}
	obj = read_object(anumber, REAL);
	obj_to_char(obj, ch);
	act("$n makes a strange magical gesture.", TRUE, ch, 0, 0, TO_ROOM);
	act("$n has created $p!", FALSE, ch, obj, 0, TO_ROOM);
	act("You now have $p.", FALSE, ch, obj, 0, TO_CHAR);
    } else {
	cprintf(ch, "Usage:  load <object|mobile> <vnum|name>\r\n");
    }
}

static void purge_one_room(int rnum, struct room_data *rp, int *range)
{
    struct char_data                       *ch = NULL;
    struct obj_data                        *obj = NULL;

    if (DEBUG > 2)
	log_info("called %s with %d, %08zx, %08zx", __PRETTY_FUNCTION__, rnum, (size_t) rp,
		 (size_t) range);

    if (rnum == 0 || rnum < range[0] || rnum > range[1])
	return;

    while (rp->people) {
	ch = rp->people;
	cprintf(ch, "The gods strike down from the heavens making the");
	cprintf(ch, "world tremble.  All that's left is the Void.");
	char_from_room(ch);
	char_to_room(ch, 0);				       /* send character to the void */
	do_look(ch, "", 15);
	act("$n tumbles into the Void.", TRUE, ch, 0, 0, TO_ROOM);
    }

    while (rp->contents) {
	obj = rp->contents;
	obj_from_room(obj);
	obj_to_room(obj, 0);				       /* send item to the void */
    }
    completely_cleanout_room(rp);			       /* clear out the pointers */
    hash_remove(&room_db, rnum);			       /* remove it from the database */
}

/* clean a room of all mobiles and objects */
void do_purge(struct char_data *ch, const char *argument, int cmd)
{
    struct char_data                       *vict = NULL;
    struct char_data                       *next_v = NULL;
    struct obj_data                        *obj = NULL;
    struct obj_data                        *next_o = NULL;
    char                                    name[100] = "\0\0\0\0\0\0\0";

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_NPC(ch))
	return;

    only_argument(argument, name);

    if (*name) {					       /* argument supplied. destroy single object or char */
	if ((vict = get_char_room_vis(ch, name))) {
	    if ((!IS_NPC(vict) || IS_SET(vict->specials.act, ACT_POLYSELF)) &&
		(GetMaxLevel(ch) < IMPLEMENTOR)) {
		cprintf(ch, "I'm sorry, Dave.  I can't let you do that.\r\n");
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
	} else if ((obj = get_obj_in_list_vis(ch, name, real_roomp(ch->in_room)->contents))) {
	    act("$n destroys $p.", FALSE, ch, obj, 0, TO_ROOM);
	    extract_obj(obj);
	} else {
	    argument = one_argument(argument, name);
	    if (0 == str_cmp("room", name)) {
		int                                     range[2];

		if (GetMaxLevel(ch) < IMPLEMENTOR) {
		    cprintf(ch, "I'm sorry, Dave.  I can't let you do that.\r\n");
		    return;
		}
		argument = one_argument(argument, name);
		if (!isdigit(*name)) {
		    cprintf(ch, "purge room start [end]");
		    return;
		}
		range[0] = atoi(name);
		argument = one_argument(argument, name);
		if (isdigit(*name))
		    range[1] = atoi(name);
		else
		    range[1] = range[0];

		if (range[0] == 0 || range[1] == 0) {
		    cprintf(ch, "usage: purge room start [end]\r\n");
		    return;
		}
		hash_iterate(&room_db, (funcp)purge_one_room, range);
	    } else {
		cprintf(ch, "I don't see that here.\r\n");
		return;
	    }
	}

	cprintf(ch, "Ok.\r\n");
    } else {						       /* no argument. clean out the room */
	if (GetMaxLevel(ch) < DEMIGOD)
	    return;
	if (IS_NPC(ch)) {
	    cprintf(ch, "You would only kill yourself..\r\n");
	    return;
	}
	act("$n gestures, the world erupts around you in flames!", FALSE, ch, 0, 0, TO_ROOM);
	rprintf(ch->in_room, "The world seems a little cleaner.\r\n");

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

/* Give pointers to the five abilities */
void roll_abilities(struct char_data *ch)
{
    int                                     i = 0;
    int                                     j = 0;
    int                                     k = 0;
    int                                     temp = 0;
    unsigned char                           table[5];
    unsigned char                           rools[4];

    if (DEBUG > 2)
	log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

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
		    ch->abilities.str_add = number(0, 100);
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
	    log_error("Error on class");
    }

    switch (GET_RACE(ch)) {
	case RACE_ELVEN:
	    ch->abilities.dex += 1;
	    ch->abilities.con -= 1;
	    break;
	case RACE_DWARF:
	    ch->abilities.con += 1;
	    ch->abilities.intel -= 1;
	    break;
	case RACE_HALFLING:
	    ch->abilities.dex += 1;
	    ch->abilities.str -= 1;
	    break;
	case RACE_GNOME:
	    ch->abilities.intel += 1;
	    ch->abilities.wis -= 1;
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

void start_character(struct char_data *ch)
{
    struct obj_data                        *obj = NULL;
    int                                     r_num = 0;

    if (DEBUG > 2)
	log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

    StartLevels(ch);
    GET_EXP(ch) = 1;
    set_title(ch);
    roll_abilities(ch);

    ch->points.max_hit = 20;

/* Heafty Bread */
    if ((r_num = real_object(5016)) >= 0) {
	obj = read_object(r_num, REAL);
	obj_to_char(obj, ch);				       /* */
	obj = read_object(r_num, REAL);
	obj_to_char(obj, ch);
    }
/* Bottle of Water */
    if ((r_num = real_object(3003)) >= 0) {
	obj = read_object(r_num, REAL);
	obj_to_char(obj, ch);
	obj = read_object(r_num, REAL);
	obj_to_char(obj, ch);
    }
/* Club */
    if ((r_num = real_object(3048)) >= 0) {
	obj = read_object(r_num, REAL);
	obj_to_char(obj, ch);
    }
/* Map of Shylar */
    if ((r_num = real_object(3050)) >= 0) {
	obj = read_object(r_num, REAL);
	obj_to_char(obj, ch);
    }
/* Newbie note: added 9-25-95 by Sedna */
    if ((r_num = real_object(3105)) >= 0) {
	obj = read_object(r_num, REAL);
	obj_to_char(obj, ch);
    }
/* Torch */
    if ((r_num = real_object(3015)) >= 0) {
	obj = read_object(r_num, REAL);
	obj_to_char(obj, ch);
    }
    if (IS_SET(ch->player.class, CLASS_RANGER)) {
	ch->skills[SKILL_TRACK].learned = 13;
	ch->skills[SKILL_DISARM].learned = 7;
    }
    if (IS_SET(ch->player.class, CLASS_THIEF)) {
	ch->skills[SKILL_SNEAK].learned = 1;
	ch->skills[SKILL_HIDE].learned = 13;
	ch->skills[SKILL_STEAL].learned = 7;
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

void do_advance(struct char_data *ch, const char *argument, int cmd)
{
    struct char_data                       *victim = NULL;
    char                                    name[100] = "\0\0\0\0\0\0\0";
    char                                    level[100] = "\0\0\0\0\0\0\0";
    char                                    class[100] = "\0\0\0\0\0\0\0";
    int                                     adv = 0;
    int                                     newlevel = 0;
    int                                     lin_class = 0;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_NPC(ch))
	return;

    argument = one_argument(argument, name);

    if (*name) {
	if (!(victim = get_char_room_vis(ch, name))) {
	    cprintf(ch, "That player is not here.\r\n");
	    return;
	}
    } else {
	cprintf(ch, "Advance who?\r\n");
	return;
    }

    if (IS_NPC(victim)) {
	cprintf(ch, "NO! Not on NPC's.\r\n");
	return;
    }
    if (IS_IMMORTAL(victim)) {
	cprintf(ch, "But they are already as powerful as you can imagine!\r\n");
	return;
    }
    argument = one_argument(argument, class);

    if (!*class) {
	cprintf(ch, "Classes you may suply: [ M C W T R ]\r\n");
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
	    cprintf(ch, "Classes you may use [ M C W T R ]\r\n");
	    return;
	    break;

    }

    argument = one_argument(argument, level);

    if (GET_LEVEL(victim, lin_class) == 0)
	adv = 1;
    else if (!*level) {
	cprintf(ch, "You must supply a level number.\r\n");
	return;
    } else {
	if (!isdigit(*level)) {
	    cprintf(ch, "Third argument must be a positive integer.\r\n");
	    return;
	}
	if ((newlevel = atoi(level)) < GET_LEVEL(victim, lin_class)) {
	    int                                     i;

	    if ((i = GET_LEVEL(victim, lin_class) - newlevel) < 1) {
		cprintf(ch, "Sorry, must leave them at level 1 at least!\r\n");
		return;
	    }
	    for (; i > 0; i--)
		drop_level(victim, lin_class);
	    set_title(victim);
	    return;
	}
	adv = newlevel - GET_LEVEL(victim, lin_class);
    }

    if (((adv + GET_LEVEL(victim, lin_class)) > 1) && (GetMaxLevel(ch) < IMPLEMENTOR)) {
	cprintf(ch, "Thou art not godly enough.\r\n");
	return;
    }
    if ((adv + GET_LEVEL(victim, lin_class)) > IMPLEMENTOR) {
	cprintf(ch, "Implementor is the highest possible level.\r\n");
	return;
    }
    if (((adv + GET_LEVEL(victim, lin_class)) < 1)
	&& ((adv + GET_LEVEL(victim, lin_class)) != 1)) {
	cprintf(ch, "1 is the lowest possible level.\r\n");
	return;
    }
    cprintf(ch, "You feel generous.\r\n");
    act("$n makes some strange gestures.\r\nA strange feeling comes upon you,"
	"\r\nLike a giant hand, light comes down from\r\nabove, grabbing your "
	"body, that begins\r\nto pulse with coloured lights from inside.\r\nYo"
	"ur head seems to be filled with daemons\r\nfrom another plane as your"
	" body dissolves\r\ninto the elements of time and space itself.\r\nSudde"
	"nly a silent explosion of light snaps\r\nyou back to reality. You fee"
	"l slightly\r\ndifferent.", FALSE, ch, 0, victim, TO_VICT);

    if (GET_LEVEL(victim, lin_class) == 0) {
	start_character(victim);
    } else {
	if (GET_LEVEL(victim, lin_class) < IMPLEMENTOR) {
	    int                                     amount_needed,
	                                            amount_have;

	    amount_needed = titles[lin_class][GET_LEVEL(victim, lin_class) + adv].exp + 1;
	    amount_have = GET_EXP(victim);
	    gain_exp_regardless(victim, amount_needed - amount_have, lin_class);
	    cprintf(ch, "Character is now advanced.\r\n");
	} else {
	    cprintf(victim, "Some idiot just tried to advance your level.\r\n");
	    cprintf(ch, "IMPOSSIBLE! IDIOTIC!\r\n");
	}
    }
}

void do_reroll(struct char_data *ch, const char *argument, int cmd)
{
    struct char_data                       *victim = NULL;
    char                                    buf[100] = "\0\0\0\0\0\0\0";

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_NPC(ch))
	return;

    if (IS_IMMORTAL(ch)) {
	only_argument(argument, buf);
	if (!*buf)
	    cprintf(ch, "Who do you wish to reroll?\r\n");
	else if (!(victim = get_char(buf)))
	    cprintf(ch, "No-one by that name in the world.\r\n");
	else {
	    cprintf(ch, "Rerolled...\r\n");
	    roll_abilities(victim);
	}
    } else {
	cprintf(ch, "You feel... different!\r\n");
	roll_abilities(ch);
    }
}

void do_restore_all(struct char_data *ch, const char *argument, int cmd)
{
    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    do_restore(ch, "all", 0);
}

void restore_one_victim(struct char_data *victim)
{
    int                                     i = 0;

    if (DEBUG > 1)
	log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(victim));

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
	if (GetMaxLevel(victim) >= LOKI) {
	    for (i = 0; i < MAX_SKILLS; i++) {
		victim->skills[i].learned = 100;
		victim->skills[i].recognise = TRUE;
	    }
	    victim->abilities.str_add = 100;
	    victim->abilities.intel = 25;
	    victim->abilities.wis = 25;
	    victim->abilities.dex = 25;
	    victim->abilities.str = 25;
	    victim->abilities.con = 25;
	    victim->tmpabilities = victim->abilities;
	} else
	    for (i = 0; i < MAX_SKILLS; i++) {
		victim->skills[i].learned = number(50, 100);
		victim->skills[i].recognise = TRUE;
	    }
	if (GetMaxLevel(victim) >= LOKI) {
	    if ((strcasecmp(GET_NAME(victim), "Quixadhal"))) {
		int                                     x = 0;

		cprintf(victim, "Fool!  You DARE challenge the Dread Lord?\r\n");
		for (x = 0; x < ABS_MAX_CLASS; x++)
		    if (HasClass(victim, 1 << x))
			GET_LEVEL(victim, x) = LOW_IMMORTAL;
		save_char(victim, NOWHERE);
	    }
	}
    }
    update_pos(victim);
}

void do_restore(struct char_data *ch, const char *argument, int cmd)
{
    struct char_data                       *victim = NULL;
    struct descriptor_data                 *i = NULL;
    char                                    buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    only_argument(argument, buf);
    if (!*buf) {
	cprintf(ch, "Who do you wish to restore?\r\n");
    } else if (!strcasecmp(buf, "all")) {
	for (i = descriptor_list; i; i = i->next) {
	    if ( /* i->character != ch && */ !i->connected &&
		i->character != board_kludge_char) {
		victim = i->character;
		restore_one_victim(victim);
		if (INVIS_LEVEL(victim) < GetMaxLevel(ch))
		    cprintf(ch, "%s restored.\r\n", GET_NAME(victim));
		act("You have been fully healed by $N!", FALSE, victim, 0, ch, TO_CHAR);
	    }
	}
    } else if (GetMaxLevel(ch) < GOD) {
	cprintf(ch, "You have not the power to restore a single mortal!\r\n");
    } else if (!(victim = get_char(buf))) {
	cprintf(ch, "No-one by that name in the world.\r\n");
    } else {
	restore_one_victim(victim);
	if (INVIS_LEVEL(victim) < GetMaxLevel(ch))
	    cprintf(ch, "%s restored.\r\n", GET_NAME(victim));
	act("You have been fully healed by $N!", FALSE, victim, 0, ch, TO_CHAR);
    }
}

void do_show_logs(struct char_data *ch, const char *argument, int cmd)
{
    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_SET(ch->specials.act, PLR_LOGS)) {
	cprintf(ch, "You will no longer recieve the logs to your screen.\r\n");
	REMOVE_BIT(ch->specials.act, PLR_LOGS);
	return;
    } else {
	cprintf(ch, "You WILL recieve the logs to your screen.\r\n");
	SET_BIT(ch->specials.act, PLR_LOGS);
	return;
    }
}

void do_noshout(struct char_data *ch, const char *argument, int cmd)
{
    struct char_data                       *vict = NULL;
    struct obj_data                        *dummy = NULL;
    char                                    buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_NPC(ch))
	return;

    only_argument(argument, buf);

    if (!*buf || IS_MORTAL(ch))
	if (IS_SET(ch->specials.act, PLR_NOSHOUT)) {
	    cprintf(ch, "You can now hear shouts again.\r\n");
	    REMOVE_BIT(ch->specials.act, PLR_NOSHOUT);
	} else {
	    cprintf(ch, "From now on, you won't hear shouts.\r\n");
	    SET_BIT(ch->specials.act, PLR_NOSHOUT);
    } else if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
	cprintf(ch, "Couldn't find any such creature.\r\n");
    else if (IS_NPC(vict))
	cprintf(ch, "Can't do that to a beast.\r\n");
    else if (GetMaxLevel(vict) >= GetMaxLevel(ch))
	act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
    else if (IS_SET(vict->specials.act, PLR_NOSHOUT) && (GetMaxLevel(ch) >= SAINT)) {
	cprintf(vict, "You can shout again.\r\n");
	cprintf(ch, "NOSHOUT removed.\r\n");
	REMOVE_BIT(vict->specials.act, PLR_NOSHOUT);
    } else if (GetMaxLevel(ch) >= SAINT) {
	cprintf(vict, "The gods take away your ability to shout!\r\n");
	cprintf(ch, "NOSHOUT set.\r\n");
	SET_BIT(vict->specials.act, PLR_NOSHOUT);
    } else {
	cprintf(ch, "Sorry, you can't do that\r\n");
    }
}

void do_pager(struct char_data *ch, const char *argument, int cmd)
{
    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_SET(ch->specials.act, PLR_PAGER)) {
	cprintf(ch, "You stop using the Wiley Pager.\r\n");
	REMOVE_BIT(ch->specials.act, PLR_PAGER);
    } else {
	cprintf(ch, "You now USE the Wiley Pager.\r\n");
	SET_BIT(ch->specials.act, PLR_PAGER);
    }
}

void do_nohassle(struct char_data *ch, const char *argument, int cmd)
{
    struct char_data                       *vict = NULL;
    struct obj_data                        *dummy = NULL;
    char                                    buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_NPC(ch))
	return;

    only_argument(argument, buf);

    if (!*buf)
	if (IS_SET(ch->specials.act, PLR_NOHASSLE)) {
	    cprintf(ch, "You can now be hassled again.\r\n");
	    REMOVE_BIT(ch->specials.act, PLR_NOHASSLE);
	} else {
	    cprintf(ch, "From now on, you won't be hassled.\r\n");
	    SET_BIT(ch->specials.act, PLR_NOHASSLE);
    } else if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
	cprintf(ch, "Couldn't find any such creature.\r\n");
    else if (IS_NPC(vict))
	cprintf(ch, "Can't do that to a beast.\r\n");
    else if (GetMaxLevel(vict) > GetMaxLevel(ch))
	act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
    else
	cprintf(ch, "The implementor won't let you set this on mortals...\r\n");
}

void do_stealth(struct char_data *ch, const char *argument, int cmd)
{
    struct char_data                       *vict = NULL;
    struct obj_data                        *dummy = NULL;
    char                                    buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_NPC(ch))
	return;

    only_argument(argument, buf);

    if (!*buf)
	if (IS_SET(ch->specials.act, PLR_STEALTH)) {
	    cprintf(ch, "STEALTH mode OFF.\r\n");
	    REMOVE_BIT(ch->specials.act, PLR_STEALTH);
	} else {
	    cprintf(ch, "STEALTH mode ON.\r\n");
	    SET_BIT(ch->specials.act, PLR_STEALTH);
    } else if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
	cprintf(ch, "Couldn't find any such creature.\r\n");
    else if (IS_NPC(vict))
	cprintf(ch, "Can't do that to a beast.\r\n");
    else if (GetMaxLevel(vict) > GetMaxLevel(ch))
	act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
    else
	cprintf(ch, "The implementor won't let you set this on mortals...\r\n");

}

static void print_room(int rnum, struct room_data *rp, struct string_block *sb)
{
    char                                    buf[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    int                                     dink = 0;
    int                                     bits = 0;
    int                                     scan = 0;

    if (DEBUG > 2)
	log_info("called %s with %d, %08zx, %08zx", __PRETTY_FUNCTION__, rnum, (size_t) rp,
		 (size_t) sb);

    snprintf(buf, MAX_STRING_LENGTH, "%5d %4d %-12s %s", rp->number, rnum, sector_types[rp->sector_type], rp->name);
    strlcat(buf, " [", MAX_STRING_LENGTH);

    dink = 0;
    for (bits = rp->room_flags, scan = 0; bits; scan++) {
	if (bits & (1 << scan)) {
	    if (dink)
		strlcat(buf, " ", MAX_STRING_LENGTH);
	    strlcat(buf, room_bits[scan], MAX_STRING_LENGTH);
	    dink = 1;
	    bits ^= (1 << scan);
	}
    }
    strlcat(buf, "]\r\n", MAX_STRING_LENGTH);

    append_to_string_block(sb, buf);
}

static void print_death_room(int rnum, struct room_data *rp, struct string_block *sb)
{
    if (DEBUG > 2)
	log_info("called %s with %d, %08zx, %08zx", __PRETTY_FUNCTION__, rnum, (size_t) rp,
		 (size_t) sb);

    if (rp && rp->room_flags & DEATH)
	print_room(rnum, rp, sb);
}

static void print_private_room(int rnum, struct room_data *rp, struct string_block *sb)
{
    if (DEBUG > 2)
	log_info("called %s with %d, %08zx, %08zx", __PRETTY_FUNCTION__, rnum, (size_t) rp,
		 (size_t) sb);

    if (rp && rp->room_flags & PRIVATE)
	print_room(rnum, rp, sb);
}

static void show_room_zone(int rnum, struct room_data *rp, struct show_room_zone_struct *srzs)
{
    char                                    buf[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG > 2)
	log_info("called %s with %d, %08zx, %08zx", __PRETTY_FUNCTION__, rnum, (size_t) rp,
		 (size_t) srzs);

    if (!rp || rp->number < srzs->bottom || rp->number > srzs->top)
	return;						       /* optimize later */

    if (srzs->blank && (srzs->lastblank + 1 != rp->number)) {
	snprintf(buf, MAX_STRING_LENGTH, "rooms %d-%d are blank\r\n", srzs->startblank, srzs->lastblank);
	append_to_string_block(srzs->sb, buf);
	srzs->blank = 0;
    }
    if (1 == sscanf(rp->name, "%d", &srzs->lastblank) && srzs->lastblank == rp->number) {
	if (!srzs->blank) {
	    srzs->startblank = srzs->lastblank;
	    srzs->blank = 1;
	}
	return;
    } else if (srzs->blank) {
	snprintf(buf, MAX_STRING_LENGTH, "rooms %d-%d are blank\r\n", srzs->startblank, srzs->lastblank);
	append_to_string_block(srzs->sb, buf);
	srzs->blank = 0;
    }
    print_room(rnum, rp, srzs->sb);
}

void do_show(struct char_data *ch, const char *argument, int cmd)
{
    char                                    buf[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    char                                    zonenum[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    int                                     zone = 0;
    int                                     bottom = 0;
    int                                     top = 0;
    int                                     topi = 0;
    struct index_data                      *which_i = NULL;
    struct string_block                     sb;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_NPC(ch))
	return;

    argument = one_argument(argument, buf);
    init_string_block(&sb);

    if (is_abbrev(buf, "zones")) {
	struct zone_data                       *zd = NULL;
	int                                     zone_bottom = 0;

	snprintf(buf, MAX_STRING_LENGTH, "# Zone   name                                lifespan age     rooms     reset\r\n");
	append_to_string_block(&sb, buf);

	for (zone = 0; zone <= top_of_zone_table; zone++) {
	    const char                             *mode = NULL;

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
	    snprintf(buf, MAX_STRING_LENGTH, "%4d %-40s %4dm %4dm %6d-%-6d %s\r\n", zone, zd->name,
		    zd->lifespan, zd->age, zone_bottom, zd->top, mode);
	    append_to_string_block(&sb, buf);
	    zone_bottom = zd->top + 1;
	}
    } else if ((is_abbrev(buf, "objects") &&
		(which_i = obj_index, topi = top_of_objt)) ||
	       (is_abbrev(buf, "mobiles") && (which_i = mob_index, topi = top_of_mobt))) {
	int                                     objn;
	struct index_data                      *oi;

	only_argument(argument, zonenum);
	zone = -1;
	if (1 == sscanf(zonenum, "%i", &zone) && (zone < 0 || zone > top_of_zone_table)) {
	    snprintf(buf, MAX_STRING_LENGTH, "That is not a valid zone_number\r\n");
	    append_to_string_block(&sb, buf);
	    return;
	}
	if (zone >= 0) {
	    bottom = zone ? (zone_table[zone - 1].top + 1) : 0;
	    top = zone_table[zone].top;
	}
	snprintf(buf, MAX_STRING_LENGTH, "%5s %4s %5s %-40s %-16s %s\r\n", "VNUM", "rnum", "count", "names",
		"distance", "room");
	append_to_string_block(&sb, buf);
	for (objn = 0; objn <= topi; objn++) {
	    struct char_data                       *target_mob = NULL;
	    struct room_data                       *target_room = NULL;
	    char                                    tbuf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

	    oi = which_i + objn;
	    if ((zone >= 0 && (oi->virtual < bottom || oi->virtual > top)) ||
		(zone < 0 && !isname(zonenum, oi->name)))
		continue;				       /* optimize later */
	    target_mob = get_char_vis_world(ch, oi->name, NULL);
	    if (target_mob)
		target_room = real_roomp(target_mob->in_room);
	    if (target_mob && target_room)
		snprintf(tbuf, MAX_INPUT_LENGTH, "[#%5d] %s", target_mob->in_room, target_room->name);
	    snprintf(buf, MAX_STRING_LENGTH, "%5d %4d %5d %-40s %-16s %s\r\n", oi->virtual, objn, oi->number,
		    oi->name, target_mob ? track_distance(ch, oi->name) : "",
		    target_room ? tbuf : "");
	    append_to_string_block(&sb, buf);
	}
    } else if (is_abbrev(buf, "rooms")) {
	only_argument(argument, zonenum);
	snprintf(buf, MAX_STRING_LENGTH, "VNUM  rnum type         name [BITS]\r\n");
	append_to_string_block(&sb, buf);
	if (is_abbrev(zonenum, "death")) {
	    hash_iterate(&room_db, (funcp)print_death_room, &sb);
	} else if (is_abbrev(zonenum, "private")) {
	    hash_iterate(&room_db, (funcp)print_private_room, &sb);
	} else if (1 != sscanf(zonenum, "%i", &zone) || zone < 0 || zone > top_of_zone_table) {
	    snprintf(buf, MAX_STRING_LENGTH, "I need a zone number with this command\r\n");
	    append_to_string_block(&sb, buf);
	} else {
	    struct show_room_zone_struct            srzs;

	    srzs.bottom = zone ? (zone_table[zone - 1].top + 1) : 0;
	    srzs.top = zone_table[zone].top;
	    srzs.blank = 0;
	    srzs.sb = &sb;
	    hash_iterate(&room_db, (funcp)show_room_zone, &srzs);
	    if (srzs.blank) {
		snprintf(buf, MAX_STRING_LENGTH, "rooms %d-%d are blank\r\n", srzs.startblank, srzs.lastblank);
		append_to_string_block(&sb, buf);
		srzs.blank = 0;
	    }
	}
    } else {
	snprintf(buf, MAX_STRING_LENGTH,
                "Usage:\r\n"
		"  show zones\r\n"
		"  show (objects|mobiles) (zone#|name)\r\n"
		"  show rooms (zone#|death|private)\r\n");
	append_to_string_block(&sb, buf);
    }
    page_string_block(&sb, ch);
    destroy_string_block(&sb);
}

void do_debug(struct char_data *ch, const char *argument, int cmd)
{
    char                                    arg[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    int                                     level = 0;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    one_argument(argument, arg);

    if (!*arg) {
	if (DEBUG) {
	    DEBUG = FALSE;
	    cprintf(ch, "Debug is now off.\r\n");
	} else {
	    DEBUG = TRUE;
	    cprintf(ch, "Debug is now on.\r\n");
	}
    } else if (!strcasecmp(arg, "on")) {
	DEBUG = TRUE;
	cprintf(ch, "Debug is now on.\r\n");
    } else if (!strcasecmp(arg, "off")) {
	DEBUG = FALSE;
	cprintf(ch, "Debug is now off.\r\n");
    } else if (scan_number(arg, &level)) {
	if (level <= 0) {
	    DEBUG = FALSE;
	    cprintf(ch, "Debug is now off.\r\n");
	} else {
	    DEBUG = level;
	    cprintf(ch, "Debug set to %d\r\n", DEBUG);
	}
    } else {
	cprintf(ch, "Usage:  debug [on|off|<level>]\r\n");
    }
    return;
}

void do_invis(struct char_data *ch, const char *argument, int cmd)
{
    int                                     level = 0;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (scan_number(argument, &level)) {
	if (level <= 0)
	    level = 0;
	else {
	    if (level >= GetMaxLevel(ch)) {
		cprintf(ch, "Sorry, you cant invis that high yet!\r\n");
		return;
	    }
	}
	ch->invis_level = level;
	cprintf(ch, "Invis level set to %d.\r\n", level);
    } else {
	if (ch->invis_level > 0) {
	    ch->invis_level = 0;
	    cprintf(ch, "You are now totally VISIBLE.\r\n");
	} else {
	    ch->invis_level = GetMaxLevel(ch) - 1;
	    cprintf(ch, "You are now invisible to level %d.\r\n", GetMaxLevel(ch) - 1);
	}
    }
}

void do_reset(struct char_data *ch, const char *argument, int cmd)
{
    int                                     start = 0;
    int                                     finish = 0;
    int                                     i = 0;
    struct room_data                       *rp = NULL;
    char                                    start_level[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char                                    finish_level[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_NPC(ch))
	return;
    argument = one_argument(argument, start_level);
    if (!strcasecmp(start_level, "all")) {
	start = 0;
	finish = top_of_zone_table;
    } else if (*start_level) {
	start = atoi(start_level);
	if (start < 0)
	    start = 0;
	if (start > top_of_zone_table)
	    start = top_of_zone_table;
	argument = one_argument(argument, finish_level);
	if (*finish_level) {
	    finish = atoi(finish_level);
	    if (finish < start)
		finish = start;
	    if (finish > top_of_zone_table)
		finish = top_of_zone_table;
	} else {
	    finish = start;
	}
    } else {
	if ((rp = real_roomp(ch->in_room))) {
	    start = finish = rp->zone;
	} else {
	    return;
	}
    }
    for (i = start; i <= finish; i++) {
	if (zone_table[i].reset_mode) {
	    reset_zone(i);
	}
    }
    if (start != finish) {
	cprintf(ch, "You have reset Zones %d through %d.\r\n", start, finish);
	log_reset("Reset of Zones [#%d] to [#%d] by %s.", start, finish, GET_NAME(ch));
    } else {
	cprintf(ch, "You have reset Zone %d.\r\n", start);
	log_reset("Reset of Zone [#%d] by %s.", start, GET_NAME(ch));
    }
}

static void zone_purge_effect(int rnum, struct room_data *rp, int *zones)
{
    struct char_data                       *vict = NULL;
    struct char_data                       *next_v = NULL;
    struct obj_data                        *obj = NULL;
    struct obj_data                        *next_o = NULL;

    if (DEBUG > 2)
	log_info("called %s with %d, %08zx, %08zx", __PRETTY_FUNCTION__, rnum, (size_t) rp,
		 (size_t) zones);

    if (!rp || rp->zone < zones[0] || rp->zone > zones[1])
	return;
    rprintf(rnum, "Flames shoot skyward all around you, and it grows quiet.\r\n");

    for (vict = rp->people; vict; vict = next_v) {
	next_v = vict->next_in_room;
	if (IS_NPC(vict) && (!IS_SET(vict->specials.act, ACT_POLYSELF)))
	    extract_char(vict);
    }
    for (obj = rp->contents; obj; obj = next_o) {
	next_o = obj->next_content;
	extract_obj(obj);
    }
}

void do_zone_purge(struct char_data *ch, const char *argument, int cmd)
{
    int                                     zones[2];
    struct room_data                       *rp = NULL;
    char                                    start_level[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char                                    finish_level[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_NPC(ch))
	return;
    argument = one_argument(argument, start_level);
    if (!strcasecmp(start_level, "all")) {
	zones[0] = 0;
	zones[1] = top_of_zone_table;
    } else if (*start_level) {
	zones[0] = atoi(start_level);
	if (zones[0] < 0)
	    zones[0] = 0;
	if (zones[0] > top_of_zone_table)
	    zones[0] = top_of_zone_table;
	argument = one_argument(argument, finish_level);
	if (*finish_level) {
	    zones[1] = atoi(finish_level);
	    if (zones[1] < zones[0])
		zones[1] = zones[0];
	    if (zones[1] > top_of_zone_table)
		zones[1] = top_of_zone_table;
	} else {
	    zones[1] = zones[0];
	}
    } else {
	if ((rp = real_roomp(ch->in_room))) {
	    zones[0] = zones[1] = rp->zone;
	} else {
	    return;
	}
    }
    hash_iterate(&room_db, (funcp)zone_purge_effect, zones);
    if (zones[0] != zones[1]) {
	cprintf(ch, "You have cleaned Zones %d through %d.\r\n", zones[0], zones[1]);
	log_reset("Purge of Zones [#%d] to [#%d] by %s.", zones[0], zones[1], GET_NAME(ch));
    } else {
	cprintf(ch, "You have cleaned Zone %d.\r\n", zones[0]);
	log_reset("Purge of Zone [#%d] by %s.", zones[0], GET_NAME(ch));
    }
}

void do_not_yet_implemented(struct char_data *ch, const char *argument, int cmd)
{
    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    cprintf(ch, "This command is not yet implemented.\r\n");
}

void do_setreboot(struct char_data *ch, const char *argument, int cmd)
{
    FILE                                   *pfd = NULL;
    int                                     first = 0;
    int                                     second = 0;
    char                                    first_str[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char                                    second_str[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_NPC(ch))
	return;
    argument = one_argument(argument, first_str);
    if (*first_str) {
	first = atoi(first_str);
	if (first < 0)
	    first = 0;
	argument = one_argument(argument, second_str);
	if (*second_str) {
	    second = atoi(second_str);
	    if (second < 0)
		second = 0;
	    if (second > 59)
		second = 59;
	} else {
	    second = first;
	}
    } else {
	first = 23;
	second = 0;
    }

    if (first > 0) {
        REBOOT_HOUR = first;
        REBOOT_MIN = second;
        REBOOT_FREQ = (REBOOT_HOUR * 60 * 60 ) + (REBOOT_MIN * 60);
        REBOOT_LASTCHECK = time(0);
        REBOOT_LEFT = REBOOT_FREQ;
        REBOOT_DISABLED = 0;

        if (!(pfd = fopen(REBOOTTIME_FILE, "w"))) {
            log_info("Cannot save reboot times!");
        } else {
            fprintf(pfd, "%d %d\n", REBOOT_HOUR, REBOOT_MIN);
            FCLOSE(pfd);
        }
        cprintf(ch, "You have set the reboot frequency to %2d hours and %2d minutes.\r\n", REBOOT_HOUR, REBOOT_MIN);
        log_info("Reboot frequency set to %2d hours and %2d minutes by %s.\r\n", REBOOT_HOUR, REBOOT_MIN, GET_NAME(ch));
    } else {
        if (!(pfd = fopen(REBOOTTIME_FILE, "w"))) {
            log_info("Cannot save reboot times!");
        } else {
            fprintf(pfd, "%d %d\n", 0, 0);
            FCLOSE(pfd);
        }
        REBOOT_DISABLED = 1;
        cprintf(ch, "You have disabled automatic reboots.\r\n");
        log_info("Automatic reboot disabled by %s.\r\n", GET_NAME(ch));
    }

}

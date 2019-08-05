/*
 * file: act.informative.c , Implementation of commands.  Part of DIKUMUD
 * Usage : Informative commands.
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */

#include <stdio.h>
#include <stdlib.h>
/* #include <unistd.h> */
#include <sys/types.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "global.h"
#include "bug.h"
#include "version.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "mudlimits.h"
#include "trap.h"
#include "hash.h"
#include "spell_parser.h"
#include "whod.h"
#include "multiclass.h"
#include "modify.h"
#include "act_wiz.h"
#include "act_skills.h"
#include "spec_procs.h"
#include "tracking.h"
#include "scheduler.h"
#include "i3.h"
#include "sql.h"
#include "reboot.h"
#include "weather.h"        // time_info_data struct
#define _ACT_INFO_C
#include "act_info.h"

/* Procedures related to 'look' */

void argument_split_2(const char *argument, char *first_arg, char *second_arg)
{
    int                                     look_at = 0;
    int                                     begin = 0;

    if (DEBUG > 2)
	log_info("called %s with %s, %s, %s", __PRETTY_FUNCTION__, VNULL(argument),
		 VNULL(first_arg), VNULL(second_arg));

    /*
     * Find first non blank 
     */
    for (; *(argument + begin) == ' '; begin++);

    /*
     * Find length of first word 
     */
    for (look_at = 0; *(argument + begin + look_at) > ' '; look_at++)
	/*
	 * Make all letters lower case, AND copy them to first_arg 
	 */
	*(first_arg + look_at) = LOWER(*(argument + begin + look_at));
    *(first_arg + look_at) = '\0';
    begin += look_at;

    /*
     * Find first non blank 
     */
    for (; *(argument + begin) == ' '; begin++);

    /*
     * Find length of second word 
     */
    for (look_at = 0; *(argument + begin + look_at) > ' '; look_at++)
	/*
	 * Make all letters lower case, AND copy them to second_arg 
	 */
	*(second_arg + look_at) = LOWER(*(argument + begin + look_at));
    *(second_arg + look_at) = '\0';
    begin += look_at;
}

struct obj_data                        *get_object_in_equip_vis(struct char_data *ch,
								const char *arg,
								struct obj_data *equipment[],
								int *j)
{
    if (DEBUG > 2)
	log_info("called %s with %s, %s, %08zx, %08zx", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(arg), (size_t) equipment, (size_t) j);

    for ((*j) = 0; (*j) < MAX_WEAR; (*j)++)
	if (equipment[(*j)])
	    if (CAN_SEE_OBJ(ch, equipment[(*j)]))
		if (isname(arg, equipment[(*j)]->name))
		    return (equipment[(*j)]);

    return (0);
}

char                                   *find_ex_description(char *word,
							    struct extra_descr_data *list)
{
    struct extra_descr_data                *i = NULL;

    if (DEBUG > 2)
	log_info("called %s with %s, %08zx", __PRETTY_FUNCTION__, VNULL(word), (size_t) list);

    for (i = list; i; i = i->next)
	if (isname(word, i->keyword))
	    return (i->description);

    return (0);
}

void show_obj_to_char(struct obj_data *object, struct char_data *ch, int mode)
{
    char                                    buffer[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG > 2)
	log_info("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), mode);

    if ((mode == 0) && object->description)
	strlcpy(buffer, object->description, MAX_STRING_LENGTH);
    else if (object->short_description && ((mode == 1) ||
					   (mode == 2) || (mode == 3) || (mode == 4)))
	strlcpy(buffer, object->short_description, MAX_STRING_LENGTH);
    else if (mode == 5) {
	if (object->obj_flags.type_flag == ITEM_NOTE) {
	    if (object->action_description) {
		strlcpy(buffer, "There is writing on it:\r\n\r\n", MAX_STRING_LENGTH);
		strlcat(buffer, object->action_description, MAX_STRING_LENGTH);
		/*
		 * page_string(ch->desc, buffer, 1); 
		 */
	    } else {
		act("It is blank.", FALSE, ch, 0, 0, TO_CHAR);
	    }
	} else if ((object->obj_flags.type_flag != ITEM_DRINKCON)) {
	    strlcpy(buffer, "You see nothing special..", MAX_STRING_LENGTH);
	} else {					       /* ITEM_TYPE == ITEM_DRINKCON */
	    strlcpy(buffer, "It looks to be a drink container.", MAX_STRING_LENGTH);
	}
    }
    if (mode != 3) {
	if (object->obj_flags.type_flag == ITEM_ARMOR) {
	    if (object->obj_flags.value[0] < (object->obj_flags.value[1] / 6))
		strlcat(buffer, "..it is extremely beaten on.", MAX_STRING_LENGTH);
	    else if (object->obj_flags.value[0] < (object->obj_flags.value[1] / 5))
		strlcat(buffer, "..it is severely beaten on.", MAX_STRING_LENGTH);
	    else if (object->obj_flags.value[0] < (object->obj_flags.value[1] / 4))
		strlcat(buffer, "..it is badly beaten on.", MAX_STRING_LENGTH);
	    else if (object->obj_flags.value[0] < (object->obj_flags.value[1] / 3))
		strlcat(buffer, "..it looks barely useable.", MAX_STRING_LENGTH);
	    else if (object->obj_flags.value[0] < (object->obj_flags.value[1] / 2))
		strlcat(buffer, "..it is showing wear.", MAX_STRING_LENGTH);
	    else if (object->obj_flags.value[0] < object->obj_flags.value[1])
		strlcat(buffer, "..it's in fair shape.", MAX_STRING_LENGTH);
	    else
		strlcat(buffer, "", MAX_STRING_LENGTH);
	}
	if (IS_OBJ_STAT(object, ITEM_ANTI_GOOD) && IS_AFFECTED(ch, AFF_DETECT_EVIL))
	    strlcat(buffer, "..it glows red!", MAX_STRING_LENGTH);
	if (IS_OBJ_STAT(object, ITEM_MAGIC) && IS_AFFECTED(ch, AFF_DETECT_MAGIC))
	    strlcat(buffer, "..it glows blue!", MAX_STRING_LENGTH);
	if (IS_OBJ_STAT(object, ITEM_GLOW))
	    strlcat(buffer, "..it glows softly!", MAX_STRING_LENGTH);
	if (IS_OBJ_STAT(object, ITEM_HUM))
	    strlcat(buffer, "..it emits a faint hum!", MAX_STRING_LENGTH);
	if (IS_OBJ_STAT(object, ITEM_INVISIBLE))
	    strlcat(buffer, "(invisible)", MAX_STRING_LENGTH);
    }
    strlcat(buffer, "\r\n", MAX_STRING_LENGTH);
    if(mode == 0)
        cprintf(ch, "%s", buffer);
    else
        page_string(ch->desc, buffer, 1);
}

void show_mult_obj_to_char(struct obj_data *object, struct char_data *ch, int mode, int num)
{
    char                                    buffer[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG > 2)
	log_info("called %s with %s, %s, %d, %d", __PRETTY_FUNCTION__, SAFE_ONAME(object),
		 SAFE_NAME(ch), mode, num);

    if ((mode == 0) && object->description)
	strlcpy(buffer, object->description, MAX_STRING_LENGTH);
    else if (object->short_description && ((mode == 1) ||
					   (mode == 2) || (mode == 3) || (mode == 4)))
	strlcpy(buffer, object->short_description, MAX_STRING_LENGTH);
    else if (mode == 5) {
	if (object->obj_flags.type_flag == ITEM_NOTE) {
	    if (object->action_description) {
		strlcpy(buffer, "There is writing on it:\r\n\r\n", MAX_STRING_LENGTH);
		strlcat(buffer, object->action_description, MAX_STRING_LENGTH);
		page_string(ch->desc, buffer, 1);
	    } else
		act("It's blank.", FALSE, ch, 0, 0, TO_CHAR);
	    return;
	} else if ((object->obj_flags.type_flag != ITEM_DRINKCON)) {
	    strlcpy(buffer, "You see nothing special..", MAX_STRING_LENGTH);
	} else {					       /* ITEM_TYPE == ITEM_DRINKCON */
	    strlcpy(buffer, "It looks like a drink container.", MAX_STRING_LENGTH);
	}
    }
    if (mode != 3) {
	if (IS_OBJ_STAT(object, ITEM_INVISIBLE)) {
	    strlcat(buffer, "(invisible)", MAX_STRING_LENGTH);
	}
	if (IS_OBJ_STAT(object, ITEM_ANTI_GOOD) && IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
	    strlcat(buffer, "..it glows red!", MAX_STRING_LENGTH);
	}
	if (IS_OBJ_STAT(object, ITEM_MAGIC) && IS_AFFECTED(ch, AFF_DETECT_MAGIC)) {
	    strlcat(buffer, "..it glows blue!", MAX_STRING_LENGTH);
	}
	if (IS_OBJ_STAT(object, ITEM_GLOW)) {
	    strlcat(buffer, "..it glows softly!", MAX_STRING_LENGTH);
	}
	if (IS_OBJ_STAT(object, ITEM_HUM)) {
	    strlcat(buffer, "..it emits a faint hum!", MAX_STRING_LENGTH);
	}
    }
    if (num > 1) {
        scprintf(buffer, MAX_STRING_LENGTH, "[%d]", num);
    }
    strlcat(buffer, "\r\n", MAX_STRING_LENGTH);
    if(mode == 0)
        cprintf(ch, "%s", buffer);
    else
        page_string(ch->desc, buffer, 1);
}

void list_obj_in_room(struct obj_data *list, struct char_data *ch)
{
    struct obj_data                        *i = NULL;
    int                                     Inventory_Num = 1;
    int                                     num = 0;
    int                                     k = 0;
    int                                     cond_top = 0;
    int                                     found = FALSE;
    struct obj_data                        *cond_ptr[50];
    int                                     cond_tot[50];

    if (DEBUG > 2)
	log_info("called %s with %08zx, %s", __PRETTY_FUNCTION__, (size_t) list, SAFE_NAME(ch));

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
    struct obj_data                        *i = NULL;
    int                                     k = 0;
    int                                     cond_top = 0;
    int                                     found = FALSE;
    int                                     Num_Inventory = 1;
    struct obj_data                        *cond_ptr[50];
    int                                     cond_tot[50];

    if (DEBUG > 2)
	log_info("called %s with %08zx, %s", __PRETTY_FUNCTION__, (size_t) list, SAFE_NAME(ch));

    if (!list) {
	cprintf(ch, "   Nothing\r\n");
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

void list_obj_to_char(struct obj_data *list, struct char_data *ch, int mode, char show)
{
    struct obj_data                        *i = NULL;
    int                                     Num_In_Bag = 1;
    int                                     found = FALSE;

    if (DEBUG > 2)
	log_info("called %s with %08zx, %s, %d, '%c'", __PRETTY_FUNCTION__, (size_t) list,
		 SAFE_NAME(ch), mode, show);

    for (i = list; i; i = i->next_content) {
	if (CAN_SEE_OBJ(ch, i)) {
	    cprintf(ch, "[%2d] ", Num_In_Bag++);
	    show_obj_to_char(i, ch, mode);
	    found = TRUE;
	}
    }

    if ((!found) && (show))
	cprintf(ch, "Nothing\r\n");
}

void show_char_to_char(struct char_data *i, struct char_data *ch, int mode)
{
    int                                     j = 0;
    int                                     found = FALSE;
    int                                     health_percent = 0;
    char                                    buffer[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    struct obj_data                        *tmp_obj = NULL;

    if (DEBUG > 2)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(i), SAFE_NAME(ch),
		 mode);

    if (mode == 0) {
	if (IS_AFFECTED(i, AFF_HIDE) || !CAN_SEE(ch, i)) {
	    if (IS_AFFECTED(ch, AFF_SENSE_LIFE))
		cprintf(ch, "You sense a hidden life form in the room.\r\n");
	    return;
	}
	if (!(i->player.long_descr) || (GET_POS(i) != i->specials.default_pos)) {
	    /*
	     * A player char or a mobile without long descr, or not in default pos. 
	     */
	    if (IS_PC(i)) {
		strlcpy(buffer, GET_NAME(i), MAX_STRING_LENGTH);
		strlcat(buffer, " ", MAX_STRING_LENGTH);
		if (GET_TITLE(i))
		    strlcat(buffer, GET_TITLE(i), MAX_STRING_LENGTH);
	    } else {
		strlcpy(buffer, i->player.short_descr, MAX_STRING_LENGTH);
		*buffer = toupper(*buffer);
	    }

	    if (IS_AFFECTED(i, AFF_INVISIBLE))
		strlcat(buffer, " (invisible)", MAX_STRING_LENGTH);
	    if (IS_AFFECTED(i, AFF_CHARM))
		strlcat(buffer, " (pet)", MAX_STRING_LENGTH);

	    switch (GET_POS(i)) {
		case POSITION_STUNNED:
		    strlcat(buffer, " is lying here, stunned", MAX_STRING_LENGTH);
		    break;
		case POSITION_INCAP:
		    strlcat(buffer, " is lying here, incapacitated", MAX_STRING_LENGTH);
		    break;
		case POSITION_MORTALLYW:
		    strlcat(buffer, " is lying here, mortally wounded", MAX_STRING_LENGTH);
		    break;
		case POSITION_DEAD:
		    strlcat(buffer, " is lying here, dead", MAX_STRING_LENGTH);
		    break;
		case POSITION_MOUNTED:
		    if (MOUNTED(i)) {
			strlcat(buffer, " is here, riding ", MAX_STRING_LENGTH);
			strlcat(buffer, MOUNTED(i)->player.short_descr, MAX_STRING_LENGTH);
		    } else
			strlcat(buffer, " is standing here.", MAX_STRING_LENGTH);
		    break;
		case POSITION_STANDING:
		    strlcat(buffer, " is standing here", MAX_STRING_LENGTH);
		    break;
		case POSITION_SITTING:
		    strlcat(buffer, " is sitting here", MAX_STRING_LENGTH);
		    break;
		case POSITION_RESTING:
		    strlcat(buffer, " is resting here", MAX_STRING_LENGTH);
		    break;
		case POSITION_SLEEPING:
		    strlcat(buffer, " is sleeping here", MAX_STRING_LENGTH);
		    break;
		case POSITION_FIGHTING:
		    if (i->specials.fighting) {
			strlcat(buffer, " is here, fighting ", MAX_STRING_LENGTH);
			if (i->specials.fighting == ch)
			    strlcat(buffer, " YOU!", MAX_STRING_LENGTH);
			else {
			    if (i->in_room == i->specials.fighting->in_room)
				if (IS_NPC(i->specials.fighting))
				    strlcat(buffer, i->specials.fighting->player.short_descr, MAX_STRING_LENGTH);
				else
				    strlcat(buffer, GET_NAME(i->specials.fighting), MAX_STRING_LENGTH);
			    else
				strlcat(buffer, "someone who has already left.", MAX_STRING_LENGTH);
			}
		    } else				       /* NIL fighting pointer */
			strlcat(buffer, " is here struggling with thin air", MAX_STRING_LENGTH);
		    break;

		default:
		    strlcat(buffer, " is floating here", MAX_STRING_LENGTH);
		    break;
	    }

	    if (IS_AFFECTED(i, AFF_FLYING)) {
		strlcat(buffer, " inches above the ground", MAX_STRING_LENGTH);
	    }
	    strlcat(buffer, ".", MAX_STRING_LENGTH);

	    if (IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
		if (IS_REALLY_VILE(i) || IS_VILE(i) || IS_EVIL(i))
		    strlcat(buffer, " (Red Aura)", MAX_STRING_LENGTH);
	    }
	    strlcat(buffer, "\r\n", MAX_STRING_LENGTH);
	    cprintf(ch, "%s", buffer);
	} else {					       /* npc with long */
	    if (IS_AFFECTED(i, AFF_INVISIBLE))
		strlcpy(buffer, "*", MAX_STRING_LENGTH);
	    else
		*buffer = '\0';

	    if (IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
		if (IS_REALLY_VILE(i) || IS_VILE(i) || IS_EVIL(i))
		    strlcat(buffer, " (Red Aura)", MAX_STRING_LENGTH);
	    }
	    strlcat(buffer, i->player.long_descr, MAX_STRING_LENGTH);

	    cprintf(ch, "%s", buffer);
	}

	if (IS_AFFECTED(i, AFF_SANCTUARY))
	    act("$n glows with a bright light!", FALSE, i, 0, ch, TO_VICT);
	if (affected_by_spell(i, SPELL_FIRESHIELD))
	    act("$n is surrounded by flickering flames!", FALSE, i, 0, ch, TO_VICT);

    } else if (mode == 1) {
	if (i->player.description)
	    cprintf(ch, "%s", i->player.description);
	else {
	    act("You see nothing special about $m.", FALSE, i, 0, ch, TO_VICT);
	}

	if (IS_PC(i)) {
	    if (ch == i) {
		if (IS_SET(i->specials.new_act, NEW_PLR_KILLOK)) {
		    snprintf(buffer, MAX_STRING_LENGTH,
                           "You are %s and bear the Mark of Dread Quixadhal.\r\n",
			    RaceName[GET_RACE(i)]);
		} else {
		    snprintf(buffer, MAX_STRING_LENGTH,
                            "You are %s.\r\n", RaceName[GET_RACE(i)]);
		}
	    } else {
		if (IS_SET(ch->specials.new_act, NEW_PLR_KILLOK)) {
		    if (IS_SET(i->specials.new_act, NEW_PLR_KILLOK)) {
			snprintf(buffer, MAX_STRING_LENGTH,
				"%s is %s and also bears the Mark of the Dread Lord.\r\n",
				GET_NAME(i), RaceName[GET_RACE(i)]);
		    } else {
			snprintf(buffer, MAX_STRING_LENGTH, "%s is %s.\r\n", GET_NAME(i), RaceName[GET_RACE(i)]);
		    }
		} else {
		    if (IS_SET(i->specials.new_act, NEW_PLR_KILLOK)) {
			snprintf(buffer, MAX_STRING_LENGTH, "%s is %s and bears the Mark of Dread Quixadhal.\r\n",
				GET_NAME(i), RaceName[GET_RACE(i)]);
		    } else {
			snprintf(buffer, MAX_STRING_LENGTH, "%s is %s.\r\n", GET_NAME(i), RaceName[GET_RACE(i)]);
		    }
		}
	    }
	} else
	    snprintf(buffer, MAX_STRING_LENGTH, "%s is %s.\r\n", MOB_NAME(i), RaceName[GET_RACE(i)]);

	cprintf(ch, "%s", buffer);

	if (MOUNTED(i))
	    act("$n is mounted on %s", FALSE, i, 0, ch, TO_VICT,
		MOUNTED(i)->player.short_descr);
	if (RIDDEN(i))
	    if (CAN_SEE(ch, RIDDEN(i)))
		act("$n is ridden by %s", FALSE, i, 0, ch, TO_VICT,
		    IS_NPC(RIDDEN(i)) ? RIDDEN(i)->player.short_descr : GET_NAME(RIDDEN(i)));

	/*
	 * Show a character to another 
	 */

	if (GET_MAX_HIT(i) > 0)
	    health_percent = (100 * GET_HIT(i)) / GET_MAX_HIT(i);
	else
	    health_percent = -1;			       /* How could MAX_HIT be < 1?? */

	if (IS_NPC(i))
	    strlcpy(buffer, i->player.short_descr, MAX_STRING_LENGTH);
	else
	    strlcpy(buffer, GET_NAME(i), MAX_STRING_LENGTH);

	if (health_percent >= 100)
	    strlcat(buffer, " is in an excellent condition.\r\n", MAX_STRING_LENGTH);
	else if (health_percent >= 90)
	    strlcat(buffer, " has a few scratches.\r\n", MAX_STRING_LENGTH);
	else if (health_percent >= 75)
	    strlcat(buffer, " has some small wounds and bruises.\r\n", MAX_STRING_LENGTH);
	else if (health_percent >= 50)
	    strlcat(buffer, " has quite a few wounds.\r\n", MAX_STRING_LENGTH);
	else if (health_percent >= 30)
	    strlcat(buffer, " has some big nasty wounds and scratches.\r\n", MAX_STRING_LENGTH);
	else if (health_percent >= 15)
	    strlcat(buffer, " looks pretty hurt.\r\n", MAX_STRING_LENGTH);
	else if (health_percent >= 0)
	    strlcat(buffer, " is in an awful condition.\r\n", MAX_STRING_LENGTH);
	else
	    strlcat(buffer, " is bleeding awfully from big wounds.\r\n", MAX_STRING_LENGTH);

	cprintf(ch, "%s", buffer);

	found = FALSE;
	for (j = 0; j < MAX_WEAR; j++) {
	    if (i->equipment[j]) {
		if (CAN_SEE_OBJ(ch, i->equipment[j])) {
		    found = TRUE;
		}
	    }
	}
	if (found) {
	    act("\r\n$n is using:", FALSE, i, 0, ch, TO_VICT);
	    for (j = 0; j < MAX_WEAR; j++) {
		if (i->equipment[j]) {
		    if (CAN_SEE_OBJ(ch, i->equipment[j])) {
			cprintf(ch, "%s", where[j]);
			show_obj_to_char(i->equipment[j], ch, 1);
		    }
		}
	    }
	}
	if (HasClass(ch, CLASS_THIEF) && (ch != i) && (!IS_IMMORTAL(ch))) {
	    found = FALSE;
	    cprintf(ch, "\r\nYou attempt to peek at their inventory:\r\n");
	    for (tmp_obj = i->carrying; tmp_obj; tmp_obj = tmp_obj->next_content) {
		if (CAN_SEE_OBJ(ch, tmp_obj) && (number(0, MAX_MORT) < GetMaxLevel(ch))) {
		    show_obj_to_char(tmp_obj, ch, 1);
		    found = TRUE;
		}
	    }
	    if (!found)
		cprintf(ch, "You can't see anything.\r\n");
	} else if (IS_IMMORTAL(ch)) {
	    cprintf(ch, "Inventory:\r\n");
	    for (tmp_obj = i->carrying; tmp_obj; tmp_obj = tmp_obj->next_content) {
		show_obj_to_char(tmp_obj, ch, 1);
		found = TRUE;
	    }
	    if (!found) {
		cprintf(ch, "Nothing\r\n");
	    }
	}
    } else if (mode == 2) {

	/*
	 * Lists inventory 
	 */
	act("$n is carrying:", FALSE, i, 0, ch, TO_VICT);
	list_obj_on_char(i->carrying, ch);
    }
}

void show_mult_char_to_char(struct char_data *i, struct char_data *ch, int mode, int num)
{
    int                                     j = 0;
    int                                     found = FALSE;
    int                                     health_percent = 0;
    struct obj_data                        *tmp_obj = NULL;
    char                                    buffer[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG > 2)
	log_info("called %s with %s, %s, %d, %d", __PRETTY_FUNCTION__, SAFE_NAME(i),
		 SAFE_NAME(ch), mode, num);

    if (mode == 0) {
	if (IS_AFFECTED(i, AFF_HIDE) || !CAN_SEE(ch, i)) {
	    if (IS_AFFECTED(ch, AFF_SENSE_LIFE)) {
		if (num == 1)
		    cprintf(ch, "You sense a hidden life form in the room.\r\n");
		else
		    cprintf(ch, "You sense hidden life forma in the room.\r\n");
	    }
	    return;
	}
	if (!(i->player.long_descr) || (GET_POS(i) != i->specials.default_pos)) {
	    /*
	     * A player char or a mobile without long descr, or not in default pos. 
	     */
	    if (!IS_NPC(i)) {
		strlcpy(buffer, GET_NAME(i), MAX_STRING_LENGTH);
		strlcat(buffer, " ", MAX_STRING_LENGTH);
		if (GET_TITLE(i))
		    strlcat(buffer, GET_TITLE(i), MAX_STRING_LENGTH);
	    } else {
		strlcpy(buffer, i->player.short_descr, MAX_STRING_LENGTH);
		*buffer = toupper(*buffer);
	    }

	    if (IS_AFFECTED(i, AFF_INVISIBLE))
		strlcat(buffer, " (invisible)", MAX_STRING_LENGTH);
	    if (IS_AFFECTED(i, AFF_CHARM))
		strlcat(buffer, " (pet)", MAX_STRING_LENGTH);

	    switch (GET_POS(i)) {
		case POSITION_STUNNED:
		    strlcat(buffer, " is lying here, stunned", MAX_STRING_LENGTH);
		    break;
		case POSITION_INCAP:
		    strlcat(buffer, " is lying here, incapacitated", MAX_STRING_LENGTH);
		    break;
		case POSITION_MORTALLYW:
		    strlcat(buffer, " is lying here, mortally wounded", MAX_STRING_LENGTH);
		    break;
		case POSITION_DEAD:
		    strlcat(buffer, " is lying here, dead", MAX_STRING_LENGTH);
		    break;
		case POSITION_STANDING:
		    strlcat(buffer, " is standing here", MAX_STRING_LENGTH);
		    break;
		case POSITION_SITTING:
		    strlcat(buffer, " is sitting here", MAX_STRING_LENGTH);
		    break;
		case POSITION_RESTING:
		    strlcat(buffer, " is resting here", MAX_STRING_LENGTH);
		    break;
		case POSITION_SLEEPING:
		    strlcat(buffer, " is sleeping here", MAX_STRING_LENGTH);
		    break;
		case POSITION_FIGHTING:
		    if (i->specials.fighting) {

			strlcat(buffer, " is here, fighting ", MAX_STRING_LENGTH);
			if (i->specials.fighting == ch)
			    strlcat(buffer, " YOU!", MAX_STRING_LENGTH);
			else {
			    if (i->in_room == i->specials.fighting->in_room)
				if (IS_NPC(i->specials.fighting))
				    strlcat(buffer, i->specials.fighting->player.short_descr, MAX_STRING_LENGTH);
				else
				    strlcat(buffer, GET_NAME(i->specials.fighting), MAX_STRING_LENGTH);
			    else
				strlcat(buffer, "someone who has already left.", MAX_STRING_LENGTH);
			}
		    } else				       /* NIL fighting pointer */
			strlcat(buffer, " is here struggling with thin air", MAX_STRING_LENGTH);
		    break;

		default:
		    strlcat(buffer, " is floating here.", MAX_STRING_LENGTH);
		    break;
	    }
	    if (IS_AFFECTED(i, AFF_FLYING))
		strlcat(buffer, " inches above the ground", MAX_STRING_LENGTH);
	    strlcat(buffer, ".", MAX_STRING_LENGTH);

	    if (IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
		if (IS_REALLY_VILE(i) || IS_VILE(i) || IS_EVIL(i))
		    strlcat(buffer, " (Red Aura)", MAX_STRING_LENGTH);
	    }
	    if (num > 1) {
                scprintf(buffer, MAX_STRING_LENGTH, " [%d]", num);
	    }
	    strlcat(buffer, "\r\n", MAX_STRING_LENGTH);
	    cprintf(ch, "%s", buffer);
	} else {					       /* npc with long */

	    if (IS_AFFECTED(i, AFF_INVISIBLE))
		strlcpy(buffer, "*", MAX_STRING_LENGTH);
	    else
		*buffer = '\0';

	    if (IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
		if (IS_REALLY_VILE(i) || IS_VILE(i) || IS_EVIL(i))
		    strlcat(buffer, " (Red Aura)", MAX_STRING_LENGTH);
	    }
	    strlcat(buffer, i->player.long_descr, MAX_STRING_LENGTH);

	    /*
	     * this gets a little annoying 
	     */

	    if (num > 1) {
		while ((buffer[strlen(buffer) - 1] == '\r') ||
		       (buffer[strlen(buffer) - 1] == '\n')
		       || (buffer[strlen(buffer) - 1] == ' ')) {
		    buffer[strlen(buffer) - 1] = '\0';
		}
		scprintf(buffer, MAX_STRING_LENGTH, " [%d]\r\n", num);
	    }
	    cprintf(ch, "%s", buffer);
	}

	if (IS_AFFECTED(i, AFF_SANCTUARY))
	    act("$n glows with a bright light!", FALSE, i, 0, ch, TO_VICT);
	if (affected_by_spell(i, SPELL_FIRESHIELD))
	    act("$n is surrounded by flickering flames!", FALSE, i, 0, ch, TO_VICT);

    } else if (mode == 1) {

	if (i->player.description)
	    cprintf(ch, "%s", i->player.description);
	else {
	    act("You see nothing special about $m.", FALSE, i, 0, ch, TO_VICT);
	}

	/*
	 * Show a character to another 
	 */

	if (GET_MAX_HIT(i) > 0)
	    health_percent = (100 * GET_HIT(i)) / GET_MAX_HIT(i);
	else
	    health_percent = -1;			       /* How could MAX_HIT be < 1?? */

	if (IS_NPC(i))
	    strlcpy(buffer, i->player.short_descr, MAX_STRING_LENGTH);
	else
	    strlcpy(buffer, GET_NAME(i), MAX_STRING_LENGTH);

	if (health_percent >= 100)
	    strlcat(buffer, " is in an excellent condition.\r\n", MAX_STRING_LENGTH);
	else if (health_percent >= 90)
	    strlcat(buffer, " has a few scratches.\r\n", MAX_STRING_LENGTH);
	else if (health_percent >= 75)
	    strlcat(buffer, " has some small wounds and bruises.\r\n", MAX_STRING_LENGTH);
	else if (health_percent >= 50)
	    strlcat(buffer, " has quite a few wounds.\r\n", MAX_STRING_LENGTH);
	else if (health_percent >= 30)
	    strlcat(buffer, " has some big nasty wounds and scratches.\r\n", MAX_STRING_LENGTH);
	else if (health_percent >= 15)
	    strlcat(buffer, " looks pretty hurt.\r\n", MAX_STRING_LENGTH);
	else if (health_percent >= 0)
	    strlcat(buffer, " is in an awful condition.\r\n", MAX_STRING_LENGTH);
	else
	    strlcat(buffer, " is bleeding awfully from big wounds.\r\n", MAX_STRING_LENGTH);

	cprintf(ch, "%s", buffer);
	cprintf(ch, "%s is %s.\r\n", GET_NAME(i), RaceName[GET_RACE(i)]);

	found = FALSE;
	for (j = 0; j < MAX_WEAR; j++) {
	    if (i->equipment[j]) {
		if (CAN_SEE_OBJ(ch, i->equipment[j])) {
		    found = TRUE;
		}
	    }
	}
	if (found) {
	    act("\r\n$n is using:", FALSE, i, 0, ch, TO_VICT);
	    for (j = 0; j < MAX_WEAR; j++) {
		if (i->equipment[j]) {
		    if (CAN_SEE_OBJ(ch, i->equipment[j])) {
			cprintf(ch, "%s", where[j]);
			show_obj_to_char(i->equipment[j], ch, 1);
		    }
		}
	    }
	}
	if ((HasClass(ch, CLASS_THIEF)) && (ch != i)) {
	    found = FALSE;
	    cprintf(ch, "\r\nYou attempt to peek at their inventory:\r\n");
	    for (tmp_obj = i->carrying; tmp_obj; tmp_obj = tmp_obj->next_content) {
		if (CAN_SEE_OBJ(ch, tmp_obj) && (number(0, MAX_MORT) < GetMaxLevel(ch))) {
		    show_obj_to_char(tmp_obj, ch, 1);
		    found = TRUE;
		}
	    }
	    if (!found)
		cprintf(ch, "You can't see anything.\r\n");
	}
    } else if (mode == 2) {

	/*
	 * Lists inventory 
	 */
	act("$n is carrying:", FALSE, i, 0, ch, TO_VICT);
	list_obj_on_char(i->carrying, ch);
    }
}

void list_char_in_room(struct char_data *list, struct char_data *ch)
{
    struct char_data                       *i = NULL;
    int                                     k = 0;
    int                                     cond_top = 0;
    int                                     found = FALSE;
    int                                     cond_tot[50];
    struct char_data                       *cond_ptr[50];

    if (DEBUG > 2)
	log_info("called %s with %08zx, %s", __PRETTY_FUNCTION__, (size_t) list, SAFE_NAME(ch));

    for (i = list; i; i = i->next_in_room) {
	if ((ch != i) &&
	    (!RIDDEN(i) || (RIDDEN(i) && !CAN_SEE(ch, RIDDEN(i)))) &&
	    (IS_AFFECTED(ch, AFF_SENSE_LIFE)
	     || (CAN_SEE(ch, i) && !IS_AFFECTED(i, AFF_HIDE)))) {
	    if ((cond_top < 50) && !MOUNTED(i)) {
		found = FALSE;
		if (IS_NPC(i)) {
		    for (k = 0; (k < cond_top && !found); k++) {
			if (cond_top > 0) {
			    if (i->nr == cond_ptr[k]->nr &&
				(GET_POS(i) == GET_POS(cond_ptr[k])) &&
				(i->specials.affected_by == cond_ptr[k]->specials.affected_by)
				&& (i->specials.fighting == cond_ptr[k]->specials.fighting)
				&& (i->player.short_descr && cond_ptr[k]->player.short_descr
				    && 0 == strcmp(i->player.short_descr,
						   cond_ptr[k]->player.short_descr))) {
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

void list_char_to_char(struct char_data *list, struct char_data *ch, int mode)
{
    struct char_data                       *i = NULL;

    if (DEBUG > 2)
	log_info("called %s with %08zx, %s, %d", __PRETTY_FUNCTION__, (size_t) list,
		 SAFE_NAME(ch), mode);

    for (i = list; i; i = i->next_in_room) {
	if ((ch != i) && (IS_AFFECTED(ch, AFF_SENSE_LIFE) ||
			  (CAN_SEE(ch, i) && !IS_AFFECTED(i, AFF_HIDE))))
	    show_char_to_char(i, ch, 0);
    }
}

void do_look(struct char_data *ch, const char *argument, int cmd)
{
    char                                    buffer[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    char                                    arg1[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char                                    arg2[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    int                                     keyword_no = 0;
    int                                     res = 0;
    int                                     j = 0;
    int                                     bits = 0;
    int                                     temp = 0;
    int                                     found = FALSE;
    struct obj_data                        *tmp_object = NULL;
    struct obj_data                        *found_object = NULL;
    struct char_data                       *tmp_char = NULL;
    char                                   *tmp_desc = NULL;

    static const char                      *keywords[] = {
	"north",
	"east",
	"south",
	"west",
	"up",
	"down",
	"in",
	"at",
	"",						       /* Look at '' case */
	"room",
	"\n"
    };

    if (DEBUG > 1)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (!ch->desc)
	return;

    if (GET_POS(ch) < POSITION_SLEEPING)
	cprintf(ch, "You can't see anything but stars!\r\n");
    else if (GET_POS(ch) == POSITION_SLEEPING)
	cprintf(ch, "You can't see anything, you're sleeping!\r\n");
    else if (IS_AFFECTED(ch, AFF_BLIND))
	cprintf(ch, "You can't see a damn thing, you're blinded!\r\n");
    else if ((IS_DARK(ch->in_room)) && (!IS_IMMORTAL(ch)) && (!IS_AFFECTED(ch, AFF_TRUE_SIGHT))) {
	cprintf(ch, "It is very dark here...\r\n");
	if (IS_AFFECTED(ch, AFF_INFRAVISION)) {
	    list_char_in_room(real_roomp(ch->in_room)->people, ch);
	}
    } else if ((IS_DARKOUT(ch->in_room)) && (!IS_IMMORTAL(ch)) &&
	       (!IS_AFFECTED(ch, AFF_TRUE_SIGHT))) {
	// cprintf(ch, "\x1b[0;36m%s\x1b[0m\r\n", real_roomp(ch->in_room)->name); /* ANSI CYAN */
        if(IS_IMMORTAL(ch))
	    cprintf(ch, "[#%d] %s\r\n", ch->in_room, real_roomp(ch->in_room)->name);	/* ANSI CYAN */
        else
	    cprintf(ch, "%s\r\n", real_roomp(ch->in_room)->name);	/* ANSI CYAN */
	cprintf(ch, "\r\nIt is quite dark here...\r\n");
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
		/*
		 * look <dir> 
		 */
	    case 0:
	    case 1:
	    case 2:
	    case 3:
	    case 4:
	    case 5:
		{
		    struct room_direction_data             *exitp;

		    exitp = NULL;
		    exitp = EXIT(ch, keyword_no);

		    if (exitp != NULL) {
			if (exitp->general_description)
			    cprintf(ch, "%s", exitp->general_description);
			else
			    cprintf(ch, "You see nothing special.\r\n");

			if (IS_SET(exitp->exit_info, EX_CLOSED) && (exitp->keyword)) {
			    if ((strcmp(fname(exitp->keyword), "secret")) &&
				(IS_NOT_SET(exitp->exit_info, EX_SECRET))) {
				cprintf(ch, "The %s is closed.\r\n", fname(exitp->keyword));
			    }
			} else {
			    if (IS_SET(exitp->exit_info, EX_ISDOOR) && exitp->keyword) {
				cprintf(ch, "The %s is open.\r\n", fname(exitp->keyword));
			    }
			}
		    } else if (cmd != SKILL_PEER)
			cprintf(ch, "You see nothing special.\r\n");

		    if ((exitp != NULL) && (IS_AFFECTED(ch, AFF_SCRYING) || IS_IMMORTAL(ch)
					    || cmd == SKILL_PEER)) {
			struct room_data                       *rp;

			cprintf(ch, "You look %swards.\r\n", dirs[keyword_no]);

			if (exitp != NULL) {
			    if (IS_SET(exitp->exit_info, EX_CLOSED) && (exitp->keyword)
				&& cmd == SKILL_PEER) {
				if ((strcmp(fname(exitp->keyword), "secret"))
				    && (IS_NOT_SET(exitp->exit_info, EX_SECRET))) {
				    cprintf(ch, "The %s is closed.\r\n", fname(exitp->keyword));
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
			    cprintf(ch, "You see a dark void.\r\n");
			} else if (exitp) {
			    snprintf(buffer, MAX_STRING_LENGTH, "%d look", exitp->to_room);
			    do_at(ch, buffer, 0);
			} else {
			    cprintf(ch, "You see nothing special.\r\n");
			}
		    } else
			cprintf(ch, "You see nothing special.\r\n");
		}
		break;

		/*
		 * look 'in' 
		 */
	    case 6:{
		    if (*arg2) {
			/*
			 * Item carried 
			 */
			bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM |
					    FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

			if (bits) {			       /* Found something */
			    if (GET_ITEM_TYPE(tmp_object) == ITEM_DRINKCON) {
				if (tmp_object->obj_flags.value[1] <= 0) {
				    act("It is empty.", FALSE, ch, 0, 0, TO_CHAR);
				} else {
				    temp =
					((tmp_object->obj_flags.value[1] * 3) /
					 tmp_object->obj_flags.value[0]);
				    cprintf(ch, "It's %sfull of a %s liquid.\r\n",
					    fullness[temp],
					    color_liquid[tmp_object->obj_flags.value[2]]);
				}
			    } else if (GET_ITEM_TYPE(tmp_object) == ITEM_CONTAINER) {
				if (!IS_SET(tmp_object->obj_flags.value[1], CONT_CLOSED)) {
				    cprintf(ch, "%s", fname(tmp_object->name));
				    switch (bits) {
					case FIND_OBJ_INV:
					    cprintf(ch, " (carried) : \r\n");
					    break;
					case FIND_OBJ_ROOM:
					    cprintf(ch, " (here) : \r\n");
					    break;
					case FIND_OBJ_EQUIP:
					    cprintf(ch, " (used) : \r\n");
					    break;
				    }
				    list_obj_to_char(tmp_object->contains, ch, 2, TRUE);
				} else
				    cprintf(ch, "It is closed.\r\n");
			    } else {
				cprintf(ch, "That is not a container.\r\n");
			    }
			} else {			       /* wrong argument */
			    cprintf(ch, "You do not see that item here.\r\n");
			}
		    } else {				       /* no argument */
			cprintf(ch, "Look in what?!\r\n");
		    }
		}
		break;

		/*
		 * look 'at' 
		 */
	    case 7:{
		    if (*arg2) {
			bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM |
					    FIND_OBJ_EQUIP | FIND_CHAR_ROOM, ch, &tmp_char,
					    &found_object);
			if (tmp_char) {
			    show_char_to_char(tmp_char, ch, 1);
			    if (ch != tmp_char) {
				if (IS_SET(ch->specials.new_act, NEW_PLR_KILLOK)) {
				    if (IS_SET(tmp_char->specials.new_act, NEW_PLR_KILLOK)) {
					act("$n looks at you and grins.", TRUE, ch, 0, tmp_char,
					    TO_VICT);
					act("$n and $N share some private joke.", TRUE, ch, 0,
					    tmp_char, TO_NOTVICT);
				    } else {
					act("$n looks at you hungrily.", TRUE, ch, 0, tmp_char,
					    TO_VICT);
					act("$n looks at $N and salivates.", TRUE, ch, 0,
					    tmp_char, TO_NOTVICT);
				    }
				} else {
				    if (IS_SET(tmp_char->specials.new_act, NEW_PLR_KILLOK)) {
					act("$n looks at you and shivers.", TRUE, ch, 0,
					    tmp_char, TO_VICT);
					act("$n backs away from $N.", TRUE, ch, 0, tmp_char,
					    TO_NOTVICT);
				    } else {
					act("$n looks at you.", TRUE, ch, 0, tmp_char, TO_VICT);
					act("$n looks at $N.", TRUE, ch, 0, tmp_char,
					    TO_NOTVICT);
				    }
				}
			    }
			    return;
			}
			/*
			 * Search for Extra Descriptions in room and items 
			 */

			/*
			 * Extra description in room?? 
			 */
			if (!found) {
			    tmp_desc =
				find_ex_description(arg2,
						    real_roomp(ch->in_room)->ex_description);
			    if (tmp_desc) {
				page_string(ch->desc, tmp_desc, 0);
				return;
			    }
			}
			/*
			 * extra descriptions in items 
			 */

			/*
			 * Equipment Used 
			 */
			if (!found) {
			    for (j = 0; j < MAX_WEAR && !found; j++) {
				if (ch->equipment[j]) {
				    if (CAN_SEE_OBJ(ch, ch->equipment[j])) {
					tmp_desc =
					    find_ex_description(arg2,
								ch->
								equipment[j]->ex_description);
					if (tmp_desc) {
					    page_string(ch->desc, tmp_desc, 1);
					    found = TRUE;
					}
				    }
				}
			    }
			}
			/*
			 * In inventory 
			 */
			if (!found) {
			    for (tmp_object = ch->carrying;
				 tmp_object && !found; tmp_object = tmp_object->next_content) {
				if CAN_SEE_OBJ
				    (ch, tmp_object) {
				    tmp_desc =
					find_ex_description(arg2, tmp_object->ex_description);
				    if (tmp_desc) {
					page_string(ch->desc, tmp_desc, 1);
					found = TRUE;
				    }
				    }
			    }
			}
			/*
			 * Object In room 
			 */

			if (!found) {
			    for (tmp_object = real_roomp(ch->in_room)->contents;
				 tmp_object && !found; tmp_object = tmp_object->next_content) {
				if CAN_SEE_OBJ
				    (ch, tmp_object) {
				    tmp_desc =
					find_ex_description(arg2, tmp_object->ex_description);
				    if (tmp_desc) {
					page_string(ch->desc, tmp_desc, 1);
					found = TRUE;
				    }
				    }
			    }
			}
			/*
			 * wrong argument 
			 */
			if (bits) {			       /* If an object was found */
			    if (!found)
				show_obj_to_char(found_object, ch, 5);
			    /*
			     * Show no-description 
			     */
			    else
				show_obj_to_char(found_object, ch, 6);
			    /*
			     * Find hum, glow etc 
			     */
			} else if (!found) {
			    cprintf(ch, "You do not see that here.\r\n");
			}
		    } else {
			/*
			 * no argument 
			 */
			cprintf(ch, "Look at what?\r\n");
		    }
		}
		break;

		/*
		 * look '' 
		 */
	    case 8:{
		    // cprintf(ch, "\x1b[0;36m%s\x1b[0m\r\n", real_roomp(ch->in_room)->name); /* ANSI CYAN */
                    if(IS_IMMORTAL(ch))
	                cprintf(ch, "[#%d] %s\r\n", ch->in_room, real_roomp(ch->in_room)->name);	/* ANSI CYAN */
                    else
		        cprintf(ch, "%s\r\n", real_roomp(ch->in_room)->name);	/* ANSI CYAN */
		    if (!IS_SET(ch->specials.act, PLR_BRIEF))
			cprintf(ch, "%s", real_roomp(ch->in_room)->description);

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
		    if (IS_SET(ch->specials.new_act, NEW_PLR_AUTOEXIT)) {
                        //list_exits_in_room(ch);
                        do_exits(ch, "", cmd);
                    }
		}
		break;

		/*
		 * wrong arg 
		 */
	    case -1:
		cprintf(ch, "Sorry, I didn't understand that!\r\n");
		break;

		/*
		 * look 'room' 
		 */
	    case 9:{

		    // cprintf(ch, "\x1b[0;36m%s\x1b[0m\r\n", real_roomp(ch->in_room)->name); /* ANSI CYAN */
                    if(IS_IMMORTAL(ch))
	                cprintf(ch, "[#%d] %s\r\n", ch->in_room, real_roomp(ch->in_room)->name);	/* ANSI CYAN */
                    else
		        cprintf(ch, "%s\r\n", real_roomp(ch->in_room)->name);	/* ANSI CYAN */
		    cprintf(ch, "%s", real_roomp(ch->in_room)->description);

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
		    if (IS_SET(ch->specials.new_act, NEW_PLR_AUTOEXIT)) {
                        //list_exits_in_room(ch);
                        do_exits(ch, "", cmd);
                    }
		}
		break;
	}
    }
}

/* end of look */

void do_read(struct char_data *ch, const char *argument, int cmd)
{
    char                                    buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    /*
     * This is just for now - To be changed later.! 
     */
    snprintf(buf, MAX_INPUT_LENGTH, "at %s", argument);
    do_look(ch, buf, 15);
}

void do_examine(struct char_data *ch, const char *argument, int cmd)
{
    char                                    name[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char                                    buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    struct char_data                       *tmp_char = NULL;
    struct obj_data                        *tmp_object = NULL;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    snprintf(buf, MAX_INPUT_LENGTH, "at %s", argument);
    do_look(ch, buf, 15);

    one_argument(argument, name);

    if (!*name) {
	cprintf(ch, "Examine what?\r\n");
	return;
    }
    generic_find(name, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP, ch, &tmp_char,
		 &tmp_object);

    if (tmp_object) {
	if ((GET_ITEM_TYPE(tmp_object) == ITEM_DRINKCON) ||
	    (GET_ITEM_TYPE(tmp_object) == ITEM_CONTAINER)) {
	    cprintf(ch, "When you look inside, you see:\r\n");
	    snprintf(buf, MAX_INPUT_LENGTH, "in %s", argument);
	    do_look(ch, buf, 15);
	}
    }
}

void do_search(struct char_data *ch, const char *argument, int cmd)
{
    int                                     door = -1;
    char                                    buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    const char                             *exits[] = {
	"North",
	"East ",
	"South",
	"West ",
	"Up   ",
	"Down "
    };
    struct room_direction_data             *exitdata = NULL;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_DARK(ch->in_room)) {
	cprintf(ch, "It is far to dark to search...\r\n");
	return;
    }
    if (GET_MANA(ch) < 30 - (GET_LEVEL(ch, BestThiefClass(ch)) / 5)) {
	cprintf(ch, "You can not think of a place to begin looking?\r\n");
	return;
    }
    GET_MANA(ch) -= (30 - (GET_LEVEL(ch, BestThiefClass(ch)) / 5));

    cprintf(ch, "You search the area...\r\n");

    for (door = 0; door < MAX_NUM_EXITS; door++) {
	exitdata = EXIT(ch, door);
	if (exitdata) {
	    if (real_roomp(exitdata->to_room)) {
		if (IS_SET(exitdata->exit_info, EX_SECRET)) {
		    if (number(1, 101) < ch->skills[SKILL_SEARCH].learned) {
			cprintf(ch, "You find a hidden passage!\r\n");
                        scprintf(buf, MAX_INPUT_LENGTH,  "%s - %s", exits[door], exitdata->keyword);
			if (IS_SET(exitdata->exit_info, EX_CLOSED))
			    strlcat(buf, " (closed)", MAX_INPUT_LENGTH);
			strlcat(buf, "\r\n", MAX_INPUT_LENGTH);
		    }
		}
	    }
	}
    }
    cprintf(ch, "Found exits:\r\n");

    if (*buf)
	cprintf(ch, "%s", buf);
    else
	cprintf(ch, "None?\r\n");
}

void list_exits_in_room(struct char_data *ch)
{
    int                                     door = -1;
    char                                    buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    const char                             *exits[] = {
	"North",
	"East",
	"South",
	"West",
	"Up",
	"Down"
    };
    struct room_direction_data             *exitdata = NULL;

    if (!IS_IMMORTAL(ch))
        return;

    for (door = 0; door < MAX_NUM_EXITS; door++) {
	exitdata = EXIT(ch, door);
	if (exitdata && exitdata->to_room != NOWHERE) {
            scprintf(buf, MAX_INPUT_LENGTH, "%s [#%d]%s, ", exits[door],
                     exitdata->to_room, real_roomp(exitdata->to_room)->name);
        }
    }
    if(*buf) {
        buf[strlen(buf)-2] = '\0'; // Wipe out the trailing ", "
    }
    cprintf(ch, "Exits: %s\r\n", buf);
}

void do_exits(struct char_data *ch, const char *argument, int cmd)
{
    int                                     door = -1;
    char                                    buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    const char                             *exits[] = {
	"North",
	"East ",
	"South",
	"West ",
	"Up   ",
	"Down "
    };
    const int                               reverse_dir[6] = { 2, 3, 0, 1, 6, 5 };
    struct room_direction_data             *this_exit = NULL;
    struct room_data                       *this_room = NULL;
    int                                     this_zone = 0;
    struct room_direction_data             *target_exit = NULL;
    struct room_data                       *target_room = NULL;
    int                                     target_zone = 0;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    bzero(buf, MAX_INPUT_LENGTH);

    this_room = real_roomp((ch)->in_room);
    if(this_room) {
        this_zone = this_room->zone;

        for (door = 0; door < MAX_NUM_EXITS; door++) {
            this_exit = this_room->dir_option[door];

            if (this_exit) {
                target_room = real_roomp(this_exit->to_room);

                if (IS_IMMORTAL(ch)) {
                    // 31 is red, 32 is green, 33 is yellow, 37 is white/grey
                    if (target_room) {
                        target_zone = target_room->zone;
                        target_exit = target_room->dir_option[reverse_dir[door]];

                        // white means the target is in the same zone we are,
                        // yellow means it's a different zone.

                        scprintf(buf, MAX_INPUT_LENGTH, "  %s%s - [#%d]",
                                (target_zone == this_zone) ? "\x1b[37;1m" : "\x1b[33;1m",
                                exits[door], this_exit->to_room);

                        // green means normal euclidean,
                        // red means non-euclidean.

                        scprintf(buf, MAX_INPUT_LENGTH, " %s%s",
                                (target_exit && (target_exit->to_room == this_room->number)) ?
                                "\x1b[32;1m" : "\x1b[31;1m",
                                target_room ? target_room->name : "Swirling CHAOS!!!");

                        if(target_exit && target_exit->to_room != this_room->number) {
                            // A non-euclidean exit that returns somewhere NOT here.
                            scprintf(buf, MAX_INPUT_LENGTH, "\x1b[31;1m returns to [#%d]",
                                    target_exit->to_room);
                        }

                        if(target_zone != this_zone) {
                            scprintf(buf, MAX_INPUT_LENGTH, "\x1b[33;1m in #%d - %s",
                                    target_room->zone, zone_table[target_room->zone].name);
                        }

                        strlcat(buf, "\x1b[0m", MAX_INPUT_LENGTH);
                        if (IS_SET(this_exit->exit_info, EX_CLOSED))
                            strlcat(buf, " (closed)", MAX_INPUT_LENGTH);
                        if (IS_DARK(this_exit->to_room))
                            strlcat(buf, " (dark)", MAX_INPUT_LENGTH);

                    } else {
                        scprintf(buf, MAX_INPUT_LENGTH,
                                "  \x1b[31;1m%s - [#%d] Swirling CHAOS!!!\x1b[0m",
                                exits[door], this_exit->to_room);
                    }
                    strlcat(buf, "\r\n", MAX_INPUT_LENGTH);
                } else {
                    // Mortal view...
                    if(target_room) {
                        if (IS_SET(this_exit->exit_info, EX_CLOSED))
                            continue;
                        scprintf(buf, MAX_INPUT_LENGTH,
                                "  %s - %s\r\n", exits[door],
                                (IS_DARK(this_exit->to_room) || IS_DARKOUT(this_exit->to_room)) ?
                                "Too dark to tell" : target_room->name);
                    }
                }
            }
        }
        cprintf(ch, "Obvious exits:\r\n%s\r\n", buf[0] ? buf : "None.\r\n");
    } else {
        cprintf(ch, "You seem to be nowhere at all...\r\n");
    }
}

void do_score(struct char_data *ch, const char *argument, int cmd)
{
    static char                             buf[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    static char                             tmpbuf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

#if 0
    static int                              ack = 0;
#endif
    struct affected_type                   *aff = NULL;
    struct char_data                       *target = NULL;
    const char                             *tmpstr = NULL;
    int                                     algn = 0;
    int                                     isneedy = 0;
    int                                     j = 0;
    int                                     cnt = 0;
    struct time_info_data                   playing_time;

    target = ch;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_IMMORTAL(ch)) {
	one_argument(argument, tmpbuf);
	if (*tmpbuf) {
	    if (!(target = get_char_room_vis(ch, tmpbuf))) {
#if 0
		ack = 1;
		if (!(target = get_char_vis_world(ch, tmpbuf, &ack)))
#else
		if (!(target = get_char(tmpbuf)))
#endif
		{
		    cprintf(ch, "You can't seem to find %s.\r\n", tmpbuf);
		    return;
		}
	    }
	}
    }
    cprintf(ch, "%s%s%s %s %s is %d years old.\r\n",
	    (IS_SET(target->specials.new_act, NEW_PLR_KILLOK)) ? "Black " : "",
	    ((age(target).year == 17) && (age(target).month == 0)) ? "Newbie " : "",
	    GET_PRETITLE(target) ? GET_PRETITLE(target) : "", GET_NAME(target),
	    GET_TITLE(target), GET_AGE(target));

    cprintf(ch, "Your terminal is set as %d columns by %d rows.\r\n",
            ch->desc->telnet.cols, ch->desc->telnet.rows);

    if (GetMaxLevel(ch) > 3) {
	cprintf(ch, "      +-------------------------Abilities-----------------------+\r\n");
	if (GET_ADD(target) > 99)
	    cprintf(ch,
		    "      | STR: %2d/**    DEX: %2d    CON: %2d    INT: %2d    WIS: %2d  |\r\n",
		    GET_STR(target), GET_DEX(target), GET_CON(target), GET_INT(target),
		    GET_WIS(target));
	else
	    cprintf(ch,
		    "      | STR: %2d/%2d    DEX: %2d    CON: %2d    INT: %2d    WIS: %2d  |\r\n",
		    GET_STR(target), GET_ADD(target), GET_DEX(target), GET_CON(target),
		    GET_INT(target), GET_WIS(target));
	cprintf(ch, "      |     AC: %3d         To-Hit: %+3d         Damage: %+3d     |\r\n",
		(target->points.armor / 10), str_app[STRENGTH_APPLY_INDEX(target)].tohit,
		str_app[STRENGTH_APPLY_INDEX(target)].todam);
	cprintf(ch, "      +---------------------------------------------------------+\r\n");
    }
    cprintf(ch, "You have %d/%d Hit Points, %d/%d Mana, and %d/%d Movement.\r\n",
	    GET_HIT(target), GET_MAX_HIT(target),
	    GET_MANA(target), GET_MAX_MANA(target), GET_MOVE(target), GET_MAX_MOVE(target));
    if (MOUNTED(target)) {
	if ((GET_LEVEL(target, RANGER_LEVEL_IND) > 0) || IS_IMMORTAL(ch) ||
	    IS_AFFECTED(MOUNTED(target), AFF_CHARM)) {
	    cprintf(ch, "%s has %d/%d Hit Points, and %d/%d Movement.\r\n",
		    GET_SDESC(MOUNTED(target)),
		    GET_HIT(MOUNTED(target)), GET_MAX_HIT(MOUNTED(target)),
		    GET_MOVE(MOUNTED(target)), GET_MAX_MOVE(MOUNTED(target)));
	}
    }
    if (!IS_IMMORTAL(target) && (!IS_NPC(target))) {
	if (IS_HOPELESSLY_DRUNK(target)) {
	    cprintf(ch, "You are hopelessly DRUNK!\r\n");
	}
	strlcpy(buf, "You are", MAX_STRING_LENGTH);
	if (IS_GETTING_HUNGRY(target)) {
	    if (IS_HUNGRY(target)) {
		if (IS_STARVING(target)) {
		    strlcat(buf, " starving", MAX_STRING_LENGTH);
		    isneedy = 1;
		} else {
		    strlcat(buf, " hungry", MAX_STRING_LENGTH);
		    isneedy = 1;
		}
	    } else {
		strlcat(buf, " getting hungry", MAX_STRING_LENGTH);
		isneedy = 1;
	    }
	}
	if (IS_GETTING_THIRSTY(target)) {
	    if (IS_THIRSTY(target)) {
		if (IS_PARCHED(target)) {
		    if (isneedy)
			strlcat(buf, " and", MAX_STRING_LENGTH);
		    strlcat(buf, " parched", MAX_STRING_LENGTH);
		    isneedy = 1;
		} else {
		    if (isneedy)
			strlcat(buf, " and", MAX_STRING_LENGTH);
		    strlcat(buf, " thirsty", MAX_STRING_LENGTH);
		    isneedy = 1;
		}
	    } else {
		if (isneedy)
		    strlcat(buf, " and", MAX_STRING_LENGTH);
		strlcat(buf, " getting thirsty", MAX_STRING_LENGTH);
		isneedy = 1;
	    }
	}
	if (isneedy)
	    cprintf(ch, "%s.\r\n", buf);
    }
    switch (GET_POS(target)) {
	case POSITION_DEAD:
	    cprintf(ch, "You are DEAD!  Just be quiet and lie still.\r\n");
	    break;
	case POSITION_MORTALLYW:
	    cprintf(ch, "You are ALMOST DEAD, be patient!\r\n");
	    break;
	case POSITION_INCAP:
	    cprintf(ch, "You are unconcious and waiting to die.\r\n");
	    break;
	case POSITION_STUNNED:
	    cprintf(ch, "At present, you are stunned, and unable to move.\r\n");
	    break;
	case POSITION_SLEEPING:
	    cprintf(ch, "You are sleeping peacefully.\r\n");
	    break;
	case POSITION_RESTING:
	    cprintf(ch, "You're resting so you can KILL MORE CRITTERS!\r\n");
	    break;
	case POSITION_SITTING:
	    cprintf(ch, "You are sitting around doing nothing.\r\n");
	    break;
	case POSITION_FIGHTING:
	    if (target->specials.fighting)
		act("You are trying to KILL $N.\r\n", FALSE, ch, 0, target->specials.fighting,
		    TO_CHAR);
	    else
		cprintf(ch, "You're fighting yourself?\r\n");
	    break;
	case POSITION_STANDING:
	    cprintf(ch, "You are just standing around, looking MEAN.\r\n");
	    break;
	case POSITION_MOUNTED:
	    if (MOUNTED(target)) {
		cprintf(ch, "You are riding %s.\r\n", GET_SDESC(MOUNTED(target)));
	    } else {
		cprintf(ch, "You are just standing around, looking MEAN.\r\n");
		break;
	    }
	    break;
	default:
	    cprintf(ch, "You seem to be floating a few inches above the ground.\r\n");
	    break;
    }

    cprintf(ch, "You carry %d Gold Soverigns and have %d in the bank.\r\n",
	    GET_GOLD(target), GET_BANK(target));
    cprintf(ch, "Your home is set to: %s\r\n", real_roomp(GET_HOME(target))->name);

    snprintf(buf, MAX_STRING_LENGTH, "      +------------------------------+------------------------------+\r\n");
    algn = MAX(-1000, MIN(GET_ALIGNMENT(target), 1000)) / 32;
    buf[37 + algn] = '*';
    cprintf(ch, "%s", buf);
    snprintf(buf, MAX_STRING_LENGTH, "                                                                     \r\n");
    buf[37 + algn] = '^';
    cprintf(ch, "%s", buf);
    cprintf(ch, "    Evil                          Neutral                          Good\r\n");

    isneedy = 0;
    strlcpy(buf, "You are NOT ", MAX_STRING_LENGTH);
    if (IS_SET(target->specials.new_act, NEW_PLR_NOTELL)) {
	strlcat(buf, "receiving tells", MAX_STRING_LENGTH);
	isneedy = 1;
    }
    if (IS_SET(target->specials.act, PLR_DEAF)) {
	if (isneedy)
	    strlcat(buf, " or", MAX_STRING_LENGTH);
	strlcat(buf, "listening to shouts", MAX_STRING_LENGTH);
	isneedy = 1;
    }
    if (isneedy)
	cprintf(ch, "%s.\r\n", buf);

    playing_time = real_time_passed((time(0) - target->player.time.logon) +
				    target->player.time.played, 0);
    cprintf(ch,
	    "You have scored %d Experience Points in %d days and %d hours!\r\n",
	    GET_EXP(target), playing_time.day, playing_time.hours);

    for (j = 0; j < ABS_MAX_CLASS; j++) {
	if (GET_LEVEL(target, j) > 0) {
	    if (GET_LEVEL(target, j) >= LOKI) {
		cprintf(ch, "You are the most powerful %s in all creation!\r\n", class_name[j]);
		continue;
	    }
	    cprintf(ch,
		    "As a %d%s level %s, you need %d xp and %d gold to become ",
		    GET_LEVEL(target, j), ordinal(GET_LEVEL(target, j)),
		    class_name[j], EXP_NEEDED(target, j), GOLD_NEEDED(target, j));
	    switch (GET_SEX(target)) {
		case SEX_MALE:
		    tmpstr = titles[j][GET_LEVEL(target, j) + 1].title_m;
		    cprintf(ch, "%s %s.\r\n", SA_AN(tmpstr), tmpstr);
		    break;
		case SEX_FEMALE:
		    tmpstr = titles[j][GET_LEVEL(target, j) + 1].title_f;
		    cprintf(ch, "%s %s.\r\n", SA_AN(tmpstr), tmpstr);
		    break;
		default:
		    cprintf(ch, "a bug.\r\n");
		    break;
	    }
	}
    }

    buf[0] = '\0';
    if (target->affected) {
	int                                     blessflag = 0;

	cnt = 20;
	isneedy = 0;
	for (aff = target->affected; aff; aff = aff->next) {
	    switch (aff->type) {
		case SKILL_SNEAK:
		    break;
		case SPELL_BLESS:
		    if (!blessflag) {
			blessflag = 1;
			if (isneedy) {
			    strlcat(buf, ", ", MAX_STRING_LENGTH);
			    cnt += 2;
			}
			if ((cnt + strlen(spell_info[aff->type].name)) > 77) {
			    strlcat(buf, "\r\n", MAX_STRING_LENGTH);
			    cnt = 0;
			}
			strlcat(buf, spell_info[aff->type].name, MAX_STRING_LENGTH);
			cnt += strlen(spell_info[aff->type].name);
			isneedy = 1;
		    }
		    break;
		default:
		    if (isneedy) {
			strlcat(buf, ", ", MAX_STRING_LENGTH);
			cnt += 2;
		    }
		    if ((cnt + strlen(spell_info[aff->type].name)) > 77) {
			strlcat(buf, "\r\n", MAX_STRING_LENGTH);
			cnt = 0;
		    }
		    strlcat(buf, spell_info[aff->type].name, MAX_STRING_LENGTH);
		    cnt += strlen(spell_info[aff->type].name);
		    isneedy = 1;
		    break;
	    }
	}
	if (isneedy)
	    cprintf(ch, "You are affected by %s.\r\n", buf);
    }
    //write_to_descriptor(ch->desc->descriptor, "\xFF\xFD\x1F"); // Ask client to do NAWS to get windows size
    //log_info("TELNET sent DO_NAWS from score");
}

void do_mystat(struct char_data *ch, const char *argument, int cmd)
{
    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    cprintf(ch, "Use SCORE instead.\r\n");
}

void do_time(struct char_data *ch, const char *argument, int cmd)
{
    struct tm                              *tm_info = NULL;
    time_t                                  now = (time_t) 0;
    char                                    buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    int                                     weekday = 0;
    int                                     day = 0;

    now = time(0);
    tm_info = localtime(&now);

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    snprintf(buf, MAX_INPUT_LENGTH, "It is %d o'clock %s, on ",
	    ((time_info.hours % 12 == 0) ? 12 : ((time_info.hours) % 12)),
	    ((time_info.hours >= 12) ? "pm" : "am"));
    weekday = ((35 * time_info.month) + time_info.day + 1) % 7;	/* 35 days in a month */
    cprintf(ch, "%s%s\r\n", buf, weekdays[weekday]);

    day = time_info.day + 1;				       /* day in [1..35] */
    cprintf(ch, "This is %d%s Day of the %s, in the %d%s Year of Dread.\r\n",
	    day, ordinal(day),
	    month_name[time_info.month], time_info.year, ordinal(time_info.year));

    if (tm_info->tm_hour == 0)
	if (tm_info->tm_min == 0)
	    snprintf(buf, MAX_INPUT_LENGTH, "It is midnight in the Land of RL!\r\n");
	else
	    snprintf(buf, MAX_INPUT_LENGTH, "It is 12:%2.2dam in the Land of RL.\r\n", tm_info->tm_min);
    else if (tm_info->tm_hour == 12)
	if (tm_info->tm_min == 0)
	    snprintf(buf, MAX_INPUT_LENGTH, "It is noon in the Land of RL.\r\n");
	else
	    snprintf(buf, MAX_INPUT_LENGTH, "It is 12:%2.2dpm in the Land of RL.\r\n", tm_info->tm_min);
    else if (tm_info->tm_hour > 12)
	snprintf(buf, MAX_INPUT_LENGTH, "It is %2d:%2.2dpm in the Land of RL.\r\n",
		tm_info->tm_hour - 12, tm_info->tm_min);
    else
	snprintf(buf, MAX_INPUT_LENGTH, "It is %2d:%2.2dam in the Land of RL.\r\n", tm_info->tm_hour,
		tm_info->tm_min);

    if (reboot.enabled && reboot.next_reboot > 0) {
        cprintf(ch, "%sThe next scheduled REBOOT is at %s RLT.\r\n", buf, reboot.next_reboot_text);
    } else {
        cprintf(ch, "%s", buf);
    }
}

void do_weather(struct char_data *ch, const char *argument, int cmd)
{
    static const char                      *sky_look[4] = {
	"cloudless", "cloudy", "rainy", "lit by flashes of lightning"
    };
    static const char                      *sky_words[] = {
	"Cloudless", "Cloudy", "Rainy", "Lightning", "None"
    };
    static const char                      *wind_dir[] = {
	"North", "East", "South", "West", "Still", "None"
    };

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_IMMORTAL(ch)) {
	cprintf(ch, "Sky Condit : %s\r\n", sky_words[weather_info.sky]);
	cprintf(ch, "Wind Speed : %d\r\n", weather_info.wind_speed);
	cprintf(ch, "Wind Direc : %s\r\n", wind_dir[weather_info.wind_direction]);
	cprintf(ch, "Change     : %d\r\n", weather_info.change);
	cprintf(ch, "Pressure   : %d\r\n", weather_info.pressure);
	cprintf(ch, "Moon Phase : %d\r\n\r\n", weather_info.moon);
    }
    if (OUTSIDE(ch)) {
	cprintf(ch, "The sky is %s and %s.\r\n", sky_look[weather_info.sky],
		(weather_info.change >= 0 ? "you feel a warm wind from south" :
		 "your foot tells you bad weather is due"));
    } else
	cprintf(ch, "You have no feeling about the weather at all.\r\n");
}

void do_allcommands(struct char_data *ch, const char *argument, int cmd)
{
    int                                     no = 0;
    int                                     i = 0;
    char                                    buf[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_NPC(ch))
	return;

    cprintf(ch, "The following comands are available:\r\n\r\n");
    for (no = 1, i = 0; *command[i] != '\n'; i++)
	if ((GetMaxLevel(ch) >= cmd_info[i + 1].minimum_level) &&
	    (cmd_info[i + 1].minimum_level < LOW_IMMORTAL)) {
	    scprintf(buf, MAX_STRING_LENGTH, "%-10s", command[i]);
	    if (!(no % 7))
		strlcat(buf, "\r\n", MAX_STRING_LENGTH);
	    no++;
	}
    strlcat(buf, "\r\n", MAX_STRING_LENGTH);
    page_string(ch->desc, buf, 1);
}

void do_inventory(struct char_data *ch, const char *argument, int cmd)
{
    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    cprintf(ch, "You are carrying:\r\n");
    list_obj_on_char(ch->carrying, ch);
}

void do_equipment(struct char_data *ch, const char *argument, int cmd)
{
    int                                     j = 0;
    int                                     Worn_Index = 0;
    int                                     found = FALSE;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    cprintf(ch, "Equipment in use:\r\n");
    for (Worn_Index = j = 0; j < MAX_WEAR; j++) {
	if (ch->equipment[j]) {
	    Worn_Index++;
	    cprintf(ch, "%s", where[j]);
	    if (CAN_SEE_OBJ(ch, ch->equipment[j])) {
		show_obj_to_char(ch->equipment[j], ch, 1);
		found = TRUE;
	    } else {
		cprintf(ch, "Something.\r\n");
		found = TRUE;
	    }
	}
    }
    if (!found) {
	cprintf(ch, " Nothing.\r\n");
    }
}

void do_credits(struct char_data *ch, const char *argument, int cmd)
{
    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    page_string(ch->desc, credits, 0);
}

void do_news(struct char_data *ch, const char *argument, int cmd)
{
    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    page_string(ch->desc, news, 0);
}

void do_info(struct char_data *ch, const char *argument, int cmd)
{
    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    page_string(ch->desc, info, 0);
}

void do_wizlist(struct char_data *ch, const char *argument, int cmd)
{
    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    page_string(ch->desc, wizlist, 0);
}

static int which_number_mobile(struct char_data *ch, struct char_data *mob)
{
    struct char_data                       *i = NULL;
    char                                   *name = NULL;
    int                                     the_number = 0;

    if (DEBUG > 2)
	log_info("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), SAFE_NAME(mob));

    name = fname(mob->player.name);
    for (i = character_list, the_number = 0; i; i = i->next) {
	if (isname(name, i->player.name) && i->in_room != NOWHERE) {
	    the_number++;
	    if (i == mob)
		return the_number;
	}
    }
    return 0;
}

char                                   *numbered_person(struct char_data *ch,
							struct char_data *person)
{
    static char                             buf[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG > 2)
	log_info("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 SAFE_NAME(person));

    if (IS_NPC(person) && IS_IMMORTAL(ch)) {
	snprintf(buf, MAX_STRING_LENGTH, "%d.%s", which_number_mobile(ch, person), fname(person->player.name));
    } else {
	strlcpy(buf, PERS(person, ch), MAX_STRING_LENGTH);
    }
    return buf;
}

static void where_person(struct char_data *ch, struct char_data *person,
			 struct string_block *sb)
{
    char                                    buf[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG > 2)
	log_info("called %s with %s, %s, %08zx", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 SAFE_NAME(person), (size_t) sb);

    snprintf(buf, MAX_STRING_LENGTH, "%-30s- %s ", PERS(person, ch),
	    (person->in_room > -1 ? real_roomp(person->in_room)->name : "Nowhere"));

    if (GetMaxLevel(ch) >= LOW_IMMORTAL)
	scprintf(buf, MAX_STRING_LENGTH, "[%d]", person->in_room);

    strlcat(buf, "\r\n", MAX_STRING_LENGTH);

    append_to_string_block(sb, buf);
}

static void where_object(struct char_data *ch, struct obj_data *obj,
			 int recurse, struct string_block *sb)
{
    char                                    buf[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG > 2)
	log_info("called %s with %s, %s, %d, %08zx", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 SAFE_ONAME(obj), recurse, (size_t) sb);

    if (obj->in_room != NOWHERE) {			       /* object in a room */
	snprintf(buf, MAX_STRING_LENGTH, "%-30s- %s [%d]\r\n",
		obj->short_description, real_roomp(obj->in_room)->name, obj->in_room);
    } else if (obj->carried_by != NULL) {		       /* object carried by monster */
	snprintf(buf, MAX_STRING_LENGTH, "%-30s- carried by %s\r\n",
		obj->short_description, numbered_person(ch, obj->carried_by));
    } else if (obj->equipped_by != NULL) {		       /* object equipped by monster */
	snprintf(buf, MAX_STRING_LENGTH, "%-30s- equipped by %s\r\n",
		obj->short_description, numbered_person(ch, obj->equipped_by));
    } else if (obj->in_obj) {				       /* object in object */
	snprintf(buf, MAX_STRING_LENGTH, "%-30s- in %s\r\n", obj->short_description,
		obj->in_obj->short_description);
    } else {
	snprintf(buf, MAX_STRING_LENGTH, "%-30s- can't find it? \r\n", obj->short_description);
    }
    if (*buf)
	append_to_string_block(sb, buf);

    if (recurse) {
	if (obj->in_room != NOWHERE)
	    return;
	else if (obj->carried_by != NULL)
	    where_person(ch, obj->carried_by, sb);
	else if (obj->equipped_by != NULL)
	    where_person(ch, obj->equipped_by, sb);
	else if (obj->in_obj != NULL)
	    where_object(ch, obj->in_obj, TRUE, sb);
    }
}

void do_where(struct char_data *ch, const char *argument, int cmd)
{
    struct char_data                       *i = NULL;
    struct obj_data                        *k = NULL;
    struct descriptor_data                 *d = NULL;
    char                                   *nameonly = NULL;
    int                                     which_number = 0;
    int                                     count = 0;
    char                                    name[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char                                    buf[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    struct string_block                     sb;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    only_argument(argument, name);

    if (!*name) {
	if (GetMaxLevel(ch) < LOW_IMMORTAL) {
	    cprintf(ch, "What are you looking for?\r\n");
	    return;
	} else {
	    init_string_block(&sb);
	    snprintf(buf, MAX_STRING_LENGTH, "Players:\r\n--------\r\n");
	    append_to_string_block(&sb, buf);

	    for (d = descriptor_list; d; d = d->next) {
		if (d->character &&
		    (d->connected == CON_PLAYING) &&
		    (d->character->in_room != NOWHERE) &&
		    (GetMaxLevel(ch) > d->character->invis_level)) {
		    if (d->original)			       /* If switched */
			snprintf(buf, MAX_STRING_LENGTH, "%-20s - %s [%d] In body of %s\r\n",
				d->original->player.name,
				real_roomp(d->character->in_room)->name,
				d->character->in_room, fname(d->character->player.name));
		    else
			snprintf(buf, MAX_STRING_LENGTH, "%-20s - %s [%d]\r\n",
				d->character->player.name,
				real_roomp(d->character->in_room)->name, d->character->in_room);

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
	count = which_number = get_number(&nameonly);
    } else {
	count = which_number = 0;
    }

    *buf = '\0';

    init_string_block(&sb);

    for (i = character_list; i; i = i->next)
	if (isname(name, i->player.name) && CAN_SEE(ch, i)) {
	    if ((i->in_room != NOWHERE) &&
		((GetMaxLevel(ch) >= LOW_IMMORTAL) || (real_roomp(i->in_room)->zone ==
						       real_roomp(ch->in_room)->zone))) {
		if (which_number == 0 || (--count) == 0) {
		    if (which_number == 0) {
			snprintf(buf, MAX_STRING_LENGTH, "[%2d] ", ++count);       /* I love short circuiting :) */
			append_to_string_block(&sb, buf);
		    }
		    where_person(ch, i, &sb);
		    *buf = 1;
		    if (which_number != 0)
			break;
		}
		if (GetMaxLevel(ch) < LOW_IMMORTAL)
		    break;
	    }
	}
    /*
     * count = which_number; 
     */

    if (GetMaxLevel(ch) >= LOW_IMMORTAL) {
	for (k = object_list; k; k = k->next)
	    if (isname(name, k->name) && CAN_SEE_OBJ(ch, k)) {
		if (which_number == 0 || (--count) == 0) {
		    if (which_number == 0) {
			snprintf(buf, MAX_STRING_LENGTH, "[%2d] ", ++count);
			append_to_string_block(&sb, buf);
		    }
		    where_object(ch, k, which_number != 0, &sb);
		    *buf = 1;
		    if (which_number != 0)
			break;
		}
	    }
    }
    if (!*sb.data)
	cprintf(ch, "Couldn't find any such thing.\r\n");
    else
	page_string_block(&sb, ch);
    destroy_string_block(&sb);
}

void do_levels(struct char_data *ch, const char *argument, int cmd)
{
    int                                     i = 0;
    int                                     RaceMax = 0;
    int                                     class = 0;
    char                                    buf[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_NPC(ch))
	return;

/*
 * get the class
 */

    for (; isspace(*argument); argument++);

    if (!*argument) {
	cprintf(ch, "You must supply a class!\r\n");
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
	    cprintf(ch, "I don't recognize %s\r\n", argument);
	    return;
	    break;
    }

    RaceMax = RacialMax[RACE_HUMAN][class];

    for (i = 1; i <= RaceMax; i++) {
	scprintf(buf, MAX_STRING_LENGTH, "[%2d] %9d-%-9d : ", i,
		titles[class][i].exp, titles[class][i + 1].exp);

	switch (GET_SEX(ch)) {
	    case SEX_MALE:
		strlcat(buf, titles[class][i].title_m, MAX_STRING_LENGTH);
		break;
	    case SEX_FEMALE:
		strlcat(buf, titles[class][i].title_f, MAX_STRING_LENGTH);
		break;
	    default:
		cprintf(ch, "Uh oh.\r\n");
		break;
	}
	strlcat(buf, "\r\n", MAX_STRING_LENGTH);
    }
    page_string(ch->desc, buf, 1);
}

void do_consider(struct char_data *ch, const char *argument, int cmd)
{
    struct char_data                       *victim = NULL;
    char                                    name[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    int                                     diff = 0;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    only_argument(argument, name);

    if (!(victim = get_char_room_vis(ch, name))) {
	cprintf(ch, "Consider killing who?\r\n");
	return;
    }
    if (victim == ch) {
	cprintf(ch, "You think it would be ridiculously easy to kill yourself.\r\n"
		"Just type quit, and then pick option 6.\r\n");
	return;
    }
    if (!IS_NPC(victim)) {
	if (!IS_SET(ch->specials.new_act, NEW_PLR_KILLOK)) {
	    cprintf(ch, "You can't bear the thought of killing another player!\r\n");
	    cprintf(victim, "%s thinks about you and quakes in terror!\r\n", NAME(ch));
	    return;
	} else {
	    if (!IS_SET(victim->specials.new_act, NEW_PLR_KILLOK)) {
		cprintf(ch,
			"You wish you could cut down %s, but the fool does not serve the Dread Lord.\r\n",
			NAME(victim));
		cprintf(victim, "%s seems interested in you for some reason.\r\n", NAME(ch));
		return;
	    }
	}
    }
    diff = (GetMaxLevel(victim) + (GetSecMaxLev(victim) / 2) + (GetThirdMaxLev(victim) / 3))
	- (GetMaxLevel(ch) + (GetSecMaxLev(ch) / 2) + (GetThirdMaxLev(ch) / 3))
	+ fuzz(MAX(1, GetMaxLevel(ch) / 5));
    if (diff <= -50) {
	cprintf(ch, "You looked at them.  Why aren't they dead?\r\n");
    } else if (diff <= -30) {
	cprintf(ch, "Why bother?  It would be a waste of 5 seconds.\r\n");
    } else if (diff <= -20) {
	cprintf(ch, "You would have to clean your weapons.\r\n");
    } else if (diff <= -15) {
	cprintf(ch, "Easy?  Understatement of the day there...\r\n");
    } else if (diff <= -10) {
	cprintf(ch, "Too easy to be believed.\r\n");
    } else if (diff <= -5) {
	cprintf(ch, "Not a problem.\r\n");
    } else if (diff <= -3) {
	cprintf(ch, "Rather easy.\r\n");
    } else if (diff <= -2) {
	cprintf(ch, "Easy.\r\n");
    } else if (diff <= -1) {
	cprintf(ch, "Looks easy.\r\n");
    } else if (diff == 0) {
	cprintf(ch, "The perfect match!\r\n");
    } else if (diff <= 1) {
	cprintf(ch, "You would need some luck!\r\n");
    } else if (diff <= 2) {
	cprintf(ch, "You would need a lot of luck!\r\n");
    } else if (diff <= 3) {
	cprintf(ch, "You would need a lot of luck and great equipment!\r\n");
    } else if (diff <= 5) {
	cprintf(ch, "Do you feel lucky, punk?\r\n");
    } else if (diff <= 10) {
	cprintf(ch, "Who do you think YOU'RE looking at, WIMP?\r\n");
    } else if (diff <= 15) {
	cprintf(ch, "Are you crazy?\r\n");
    } else if (diff <= 20) {
	cprintf(ch, "Think about this for a moment.\r\n");
    } else if (diff <= 30) {
	cprintf(ch, "You ARE mad!\r\n");
    } else {
	cprintf(ch, "Just tell me where to send the flowers.\r\n");
    }
}

void do_spells(struct char_data *ch, const char *argument, int cmd)
{
    int                                     i = 0;
    char                                    buf[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_NPC(ch))
	return;

    snprintf(buf, MAX_STRING_LENGTH, "Spell Name                Ma Cl Wa Th Ra Dr\r\n");
    for (i = 1; i < MAX_SKILLS; i++) {
	if (!spell_info[i].castable)
	    continue;
	scprintf(buf, MAX_STRING_LENGTH, "[%3d] %-20s  %2d %2d %2d %2d %2d %2d\r\n",
		i, spell_info[i].name,
		spell_info[i].min_level[MAGE_LEVEL_IND],
		spell_info[i].min_level[CLERIC_LEVEL_IND],
		spell_info[i].min_level[WARRIOR_LEVEL_IND],
		spell_info[i].min_level[THIEF_LEVEL_IND],
		spell_info[i].min_level[RANGER_LEVEL_IND],
		spell_info[i].min_level[DRUID_LEVEL_IND]);
    }
    strlcat(buf, "\r\n", MAX_STRING_LENGTH);
    page_string(ch->desc, buf, 1);
}

void do_world(struct char_data *ch, const char *argument, int cmd)
{
    time_t                                  ct = (time_t) 0;
    time_t                                  ot = (time_t) 0;
    char                                   *tmstr = NULL;
    char                                   *otmstr = NULL;
    int                                     i = 0;
    int                                     mob_count = 0;
    int                                     obj_count = 0;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    ot = Uptime;
    otmstr = asctime(localtime(&ot));
    *(otmstr + strlen(otmstr) - 1) = '\0';
    cprintf(ch, "Wiley start time was: %s\r\n", otmstr);

    ct = time(0);
    tmstr = asctime(localtime(&ct));
    *(tmstr + strlen(tmstr) - 1) = '\0';

    for (i = 0; i <= top_of_mobt; i++) {
        struct index_data                      *oi;

        oi = mob_index + i;
        if(oi)
            mob_count += oi->number;

    }

    for (i = 0; i <= top_of_objt; i++) {
        struct index_data                      *oi;

        oi = obj_index + i;
        if(oi)
            obj_count += oi->number;
    }

    cprintf(ch, "Wiley's time is: %s\r\n", tmstr);
    cprintf(ch, "Total number of rooms: %d\r\n", room_db.klistlen);
    cprintf(ch, "Total number of objects: %d (%d instances)\r\n", top_of_objt + 1, obj_count);
    cprintf(ch, "Total number of mobiles: %d (%d instances)\r\n", top_of_mobt + 1, mob_count);
    cprintf(ch, "Total number of players: %d\r\n", number_of_players);
}

void do_skills(struct char_data *ch, const char *argument, int cmd)
{
    int                                     i = 0;

    struct skill_struct {
	char                                    skill_name[40];
	int                                     skill_numb;
	int                                     skill_class;
	int                                     skill_lvl;
    };

    struct skill_struct                     r_skills[] = {
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

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    cprintf(ch, "You have %d practice sessions left.\r\n", ch->specials.pracs);
    for (i = 0; r_skills[i].skill_name[0] != '\n'; i++) {
	if (r_skills[i].skill_lvl <= GetMaxLevel(ch) || IS_IMMORTAL(ch)) {
	    if ((IS_SET(ch->player.class, r_skills[i].skill_class) &&
		 GetMaxLevel(ch) > r_skills[i].skill_lvl) || IS_IMMORTAL(ch)) {
		cprintf(ch, "%s%s\r\n", r_skills[i].skill_name,
			how_good(ch->skills[r_skills[i].skill_numb].learned));
	    }
	}
    }
}

void tick_line(struct char_data *ch, const char *label, int pulse_count) {
    double seconds = 0.0;
    int minutes = 0;
    int hours = 0;
    int days = 0;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(label), pulse_count);

    seconds = (double)pulse_count / (double)PULSE_PER_SECOND;
    minutes = (int)seconds / 60;
    seconds = seconds - ((double)minutes * 60.0);
    hours = minutes / 60;
    days = hours / 24;
    minutes %= 60;
    hours %= 24;

    if(days > 0) {
        cprintf(ch, "    %-35s%d days, %2d:%02d:%05.2lf.\r\n", label, days, hours, minutes, seconds);
    } else if(hours > 0) {
        cprintf(ch, "    %-44s%2d:%02d:%05.2lf\r\n", label, hours, minutes, seconds);
    } else if(minutes > 0) {
        cprintf(ch, "    %-47s%2d:%05.2lf\r\n", label, minutes, seconds);
    } else {
        cprintf(ch, "    %-50s%5.2lf\r\n", label, seconds);
    }
}

void do_ticks(struct char_data *ch, const char *argument, int cmd)
{
    char ticktype[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    only_argument(argument, ticktype);
    if (0 == strcasecmp(ticktype, "mudlist")) {
        pulse_mudlist = 0;
    } else if (0 == strcasecmp(ticktype, "zone")) {
        pulse_zone = 0;
    } else if (0 == strcasecmp(ticktype, "nature")) {
        pulse_nature = 0;
    }
    
    cprintf(ch, "Pulse Counter: %d\r\n", pulse);

    tick_line(ch, "NEXT TICK:", pulse_update);
    tick_line(ch, "Next Zone tick:", pulse_zone);
    tick_line(ch, "Next Teleport tick:", pulse_teleport);
    tick_line(ch, "Next Nature tick:", pulse_nature);
    tick_line(ch, "Next Violance tick:", pulse_violence);
    tick_line(ch, "Next River tick:", pulse_river);
    tick_line(ch, "Next Sound tick:", pulse_sound);
    tick_line(ch, "Next Reboot tick:", pulse_reboot);
    tick_line(ch, "Next Dump tick:", pulse_dump);
    tick_line(ch, "Next Mudlist tick:", pulse_mudlist);
    tick_line(ch, "Next URL tick:", pulse_url);
    tick_line(ch, "Next URL Handler tick:", pulse_url_handler);
    tick_line(ch, "Next I3 idle tick:", timestampToTicks(diffTimestamp(time_to_taunt, -1)));

    if (reboot.enabled && reboot.next_reboot > 0 && reboot.next_reboot >= time(0)) {
        char tmp[MAX_INPUT_LENGTH];

        strlcpy(tmp, reboot.next_reboot_text, MAX_INPUT_LENGTH);
        tmp[19] = '\0'; // 2019-07-16 12:48:31
        cprintf(ch, "    %-36s%s\r\n", "Next REBOOT at:", reboot.next_reboot_text);
    }
}

void do_map(struct char_data *ch, const char *argument, int cmd)
{
    const char                             *template =
	"\r\n"
	"    U                      +----------------------+\r\n"
	"    | N                    | %-20.20s |\r\n"
	"    |/                     | %-6.6s  %12.12s |\r\n"
	"W---/---E                  +-----------+----------+\r\n"
	"   /|                                  |\r\n"
	"  S |                            ______+________________,\r\n"
	"    D                           / %-20.20s /\r\n"
	"                               / %-6.6s  %12.12s /\r\n"
	"                              /______________________/\r\n"
	"                                       | /              _______________________,\r\n"
	"   _______________________,            |/              / %-20.20s /\r\n"
	"  / %-20.20s /_____________/______________/ %-6.6s  %12.12s /\r\n"
	" / %-6.6s  %12.12s /             /|             /______________________/\r\n"
	"/______________________/             / |\r\n"
	"                         ___________/__+________,\r\n"
	"                        / %-20.20s /\r\n"
	"                       / %-6.6s  %12.12s /\r\n"
	"                      /______________________/\r\n"
	"                                       |\r\n"
	"                           +-----------+----------+\r\n"
	"                           | %-20.20s |\r\n"
	"                           | %-6.6s  %12.12s |\r\n"
	"                           +----------------------+\r\n";
    int                                     door = -1;
    struct room_direction_data             *exitdata = NULL;
    char                                    name[MAX_NUM_EXITS][21];
    char                                    vnum[MAX_NUM_EXITS][7];
    char                                    terrain[MAX_NUM_EXITS][13];

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    for (door = 0; door < MAX_NUM_EXITS; door++) {
	bzero(name[door], 21);
	bzero(vnum[door], 7);
	bzero(terrain[door], 13);
	exitdata = EXIT(ch, door);
	if (exitdata) {
	    if (!real_roomp(exitdata->to_room)) {
		if (IS_IMMORTAL(ch)) {
		    snprintf(name[door], 21, "Swirling CHAOS!");
		    snprintf(vnum[door], 7, "#%-5.5d", exitdata->to_room);
		}
	    } else if (exitdata->to_room != NOWHERE) {
		if (IS_IMMORTAL(ch)) {
		    snprintf(name[door], 21, "%c%c%c%c %-15.15s",
			    (IS_DARK(exitdata->to_room) || IS_DARKOUT(exitdata->to_room)) ?
			    'D' : ' ',
			    (IS_SET(exitdata->exit_info, EX_SECRET)) ? 'S' : ' ',
			    (IS_SET(exitdata->exit_info, EX_CLOSED)) ? 'C' : ' ',
			    (IS_SET(exitdata->exit_info, EX_LOCKED)) ? 'L' : ' ',
			    real_roomp(exitdata->to_room)->name);
		    snprintf(vnum[door], 7, "#%-5.5d", exitdata->to_room);
		    snprintf(terrain[door], 13, 
			    sector_types[real_roomp(exitdata->to_room)->sector_type], "");
		} else {
		    if (!IS_SET(exitdata->exit_info, EX_SECRET)) {
			if (IS_SET(exitdata->exit_info, EX_CLOSED)) {
			    snprintf(name[door], 21, "A Closed Door");
			} else {
			    if (IS_DARK(exitdata->to_room) || IS_DARKOUT(exitdata->to_room)) {
				snprintf(name[door], 21, "Darkness");
			    } else {
				snprintf(name[door], 21, "%-20.20s",
					real_roomp(exitdata->to_room)->name);
				snprintf(terrain[door], 13,
					sector_types[real_roomp
						     (exitdata->to_room)->sector_type], "");
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

char                                   *get_ansi_sector(struct room_data *this_room)
{
    static char                             buf[12];
    const char                             *symbols = "..-%^^~~$~";
    const char                             *colors = "0322774464";
    const char                             *bolds = "9939399393";

    strlcpy(buf, " ", 12);

    if (!this_room)
	return buf;

    if (this_room->sector_type >= 0) {
	int                                     x = this_room->sector_type;

	snprintf(buf, 12, "%c[%c%cm%c%c[0m", 27, bolds[x], colors[x], symbols[x], 27);
    } else {
	snprintf(buf, 12, "%c[%c%cm%c%c[0m", 27, '4', '5', '?', 27);
    }

    return buf;
}

struct room_data                       *walk_room_path(struct room_data *this_room,
						       const char *path)
{
    int                                     i;
    int                                     dir = -1;
    struct room_data                       *next_room = NULL;
    struct room_data                       *orig_room = NULL;
    struct room_data                       *targ_room = NULL;
    char                                   *rotated = NULL;
    int                                     r = 1;
    int                                     found = 1;

    if (!this_room)
	return NULL;
    if (!path || !*path)
	return NULL;
    if (strspn(path, "nNeEsSwWuUdD") < strlen(path))
	return NULL;

    rotated = malloc(strlen(path) + 1);
    rotated[strlen(path)] = '\0';
    orig_room = this_room;

    for (r = 0; r < strlen(path); r++) {
	memcpy(rotated, path + r, strlen(path) - r);
	if (r > 0)
	    memcpy(rotated + strlen(path) - r, path, r);
	found = 1;
	this_room = orig_room;
	for (i = 0; i < strlen(path); i++) {
	    switch (rotated[i]) {
		case 'n':
		case 'N':
		    dir = 0;
		    break;
		case 'e':
		case 'E':
		    dir = 1;
		    break;
		case 's':
		case 'S':
		    dir = 2;
		    break;
		case 'w':
		case 'W':
		    dir = 3;
		    break;
		case 'u':
		case 'U':
		    dir = 4;
		    break;
		case 'd':
		case 'D':
		    dir = 5;
		    break;
	    }
	    if (this_room->dir_option[dir] &&
		this_room->dir_option[dir]->to_room >= 0 &&
		(next_room = real_roomp(this_room->dir_option[dir]->to_room))) {
		this_room = next_room;
	    } else {
		found = 0;
		break;
	    }
	}
	if (found) {
	    targ_room = this_room;
	    break;
	}
    }

    free(rotated);
    return targ_room;
}

void do_ansimap(struct char_data *ch, const char *argument, int cmd)
{
    struct room_data                       *this_room = NULL;
    char                                   *map[9][9];
    int                                     i,
                                            j;
    char                                    tmp[MAX_INPUT_LENGTH];

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_NPC(ch)) {
	cprintf(ch, "You don't need a map.\r\n");
	return;
    }

    this_room = real_roomp(ch->in_room);
    if (!this_room)
	return;

    for (i = 0; i < 9; i++)
	for (j = 0; j < 9; j++)
	    map[i][j] = NULL;

    snprintf(tmp, MAX_INPUT_LENGTH, "\x1b[44m");
    strlcat(tmp, get_ansi_sector(this_room), MAX_INPUT_LENGTH);
    map[4][4] = strdup(tmp);

#define WRP(x) walk_room_path(this_room, x)

    /*
     * Ring 1 
     */
    map[3][4] = strdup(get_ansi_sector(WRP("n")));
    map[3][5] = strdup(get_ansi_sector(WRP("ne")));
    map[4][5] = strdup(get_ansi_sector(WRP("e")));
    map[5][5] = strdup(get_ansi_sector(WRP("es")));
    map[5][4] = strdup(get_ansi_sector(WRP("s")));
    map[5][3] = strdup(get_ansi_sector(WRP("sw")));
    map[4][3] = strdup(get_ansi_sector(WRP("w")));
    map[3][3] = strdup(get_ansi_sector(WRP("wn")));

    /*
     * Ring 2 
     */
    map[2][4] = strdup(get_ansi_sector(WRP("nn")));
    map[2][3] = strdup(get_ansi_sector(WRP("nnw")));
    map[2][5] = strdup(get_ansi_sector(WRP("nne")));
    map[2][6] = strdup(get_ansi_sector(WRP("nnee")));

    map[4][6] = strdup(get_ansi_sector(WRP("ee")));
    map[3][6] = strdup(get_ansi_sector(WRP("een")));
    map[5][6] = strdup(get_ansi_sector(WRP("ees")));
    map[6][6] = strdup(get_ansi_sector(WRP("eess")));

    map[6][4] = strdup(get_ansi_sector(WRP("ss")));
    map[6][3] = strdup(get_ansi_sector(WRP("ssw")));
    map[6][5] = strdup(get_ansi_sector(WRP("sse")));
    map[6][6] = strdup(get_ansi_sector(WRP("ssww")));

    map[4][2] = strdup(get_ansi_sector(WRP("ww")));
    map[3][2] = strdup(get_ansi_sector(WRP("wwn")));
    map[5][2] = strdup(get_ansi_sector(WRP("wws")));
    map[2][2] = strdup(get_ansi_sector(WRP("wwnn")));

    /*
     * Ring 3 
     */
    map[1][4] = strdup(get_ansi_sector(WRP("nnn")));
    map[1][3] = strdup(get_ansi_sector(WRP("nnnw")));
    map[1][5] = strdup(get_ansi_sector(WRP("nnne")));
    map[1][2] = strdup(get_ansi_sector(WRP("nnnww")));
    map[1][6] = strdup(get_ansi_sector(WRP("nnnee")));
    map[1][7] = strdup(get_ansi_sector(WRP("nnneee")));

    map[4][7] = strdup(get_ansi_sector(WRP("eee")));
    map[3][7] = strdup(get_ansi_sector(WRP("eeen")));
    map[5][7] = strdup(get_ansi_sector(WRP("eees")));
    map[6][7] = strdup(get_ansi_sector(WRP("eeess")));
    map[2][7] = strdup(get_ansi_sector(WRP("eeenn")));
    map[7][7] = strdup(get_ansi_sector(WRP("eeesss")));

    map[7][4] = strdup(get_ansi_sector(WRP("sss")));
    map[7][3] = strdup(get_ansi_sector(WRP("sssw")));
    map[7][5] = strdup(get_ansi_sector(WRP("ssse")));
    map[7][6] = strdup(get_ansi_sector(WRP("sssww")));
    map[7][2] = strdup(get_ansi_sector(WRP("sssee")));
    map[7][1] = strdup(get_ansi_sector(WRP("ssswww")));

    map[4][1] = strdup(get_ansi_sector(WRP("www")));
    map[3][1] = strdup(get_ansi_sector(WRP("wwwn")));
    map[5][1] = strdup(get_ansi_sector(WRP("wwws")));
    map[2][1] = strdup(get_ansi_sector(WRP("wwwnn")));
    map[6][1] = strdup(get_ansi_sector(WRP("wwwss")));
    map[1][1] = strdup(get_ansi_sector(WRP("wwwnnn")));

    /*
     * Ring 4 
     */
    map[0][4] = strdup(get_ansi_sector(WRP("nnnn")));
    map[0][3] = strdup(get_ansi_sector(WRP("nnnnw")));
    map[0][5] = strdup(get_ansi_sector(WRP("nnnne")));
    map[0][2] = strdup(get_ansi_sector(WRP("nnnnww")));
    map[0][6] = strdup(get_ansi_sector(WRP("nnnnee")));
    map[0][7] = strdup(get_ansi_sector(WRP("nnnneee")));
    map[0][1] = strdup(get_ansi_sector(WRP("nnnnwww")));
    map[0][8] = strdup(get_ansi_sector(WRP("nnnneeee")));

    map[4][8] = strdup(get_ansi_sector(WRP("eeee")));
    map[3][8] = strdup(get_ansi_sector(WRP("eeeen")));
    map[5][8] = strdup(get_ansi_sector(WRP("eeees")));
    map[6][8] = strdup(get_ansi_sector(WRP("eeeess")));
    map[2][8] = strdup(get_ansi_sector(WRP("eeeenn")));
    map[7][8] = strdup(get_ansi_sector(WRP("eeeesss")));
    map[1][8] = strdup(get_ansi_sector(WRP("eeeennn")));
    map[8][8] = strdup(get_ansi_sector(WRP("eeeessss")));

    map[8][4] = strdup(get_ansi_sector(WRP("ssss")));
    map[8][3] = strdup(get_ansi_sector(WRP("ssssw")));
    map[8][5] = strdup(get_ansi_sector(WRP("sssse")));
    map[8][6] = strdup(get_ansi_sector(WRP("ssssww")));
    map[8][2] = strdup(get_ansi_sector(WRP("ssssee")));
    map[8][1] = strdup(get_ansi_sector(WRP("sssswww")));
    map[8][7] = strdup(get_ansi_sector(WRP("sssseee")));
    map[8][0] = strdup(get_ansi_sector(WRP("sssswwww")));

    map[4][0] = strdup(get_ansi_sector(WRP("wwww")));
    map[3][0] = strdup(get_ansi_sector(WRP("wwwwn")));
    map[5][0] = strdup(get_ansi_sector(WRP("wwwws")));
    map[2][0] = strdup(get_ansi_sector(WRP("wwwwnn")));
    map[6][0] = strdup(get_ansi_sector(WRP("wwwwss")));
    map[1][0] = strdup(get_ansi_sector(WRP("wwwwnnn")));
    map[7][0] = strdup(get_ansi_sector(WRP("wwwwsss")));
    map[0][0] = strdup(get_ansi_sector(WRP("wwwwnnnn")));

    cprintf(ch, "   ---- ++++\n");
    cprintf(ch, "   432101234\n");
    for (i = 0; i < 9; i++) {
	cprintf(ch, "%s%d ", i < 4 ? "" : (i == 4 ? " " : "+"), i - 4);
	for (j = 0; j < 9; j++) {
	    cprintf(ch, "%s", map[i][j] == NULL ? " " : map[i][j]);
	}
	cprintf(ch, "\n");
    }

    for (i = 0; i < 9; i++)
	for (j = 0; j < 9; j++)
	    if (map[i][j])
		free(map[i][j]);

}

void do_version(struct char_data *ch, const char *argument, int cmd)
{
    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    cprintf(ch, "WileyMUD:         %s (%s)\r\n", VERSION_BUILD, VERSION_DATE);
    cprintf(ch, "PostgreSQL:       %s\r\n", sql_version(&db_i3log));
}


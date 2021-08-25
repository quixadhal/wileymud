/*
 * file: handler.c , Handler module.                      Part of DIKUMUD
 * Usage: Various routines for moving about objects/players
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */

#include <stdio.h>
#include <stdlib.h>
/* #include <unistd.h> */
#include <sys/types.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

#include "global.h"
#include "bug.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "interpreter.h"
#include "spells.h"
#include "spell_parser.h"
#include "fight.h"
#include "modify.h"
#include "multiclass.h"
#include "opinion.h"
#include "act_wiz.h"
#define _HANDLER_C
#include "handler.h"

char *fname(const char *namelist)
{
    static char holder[30] = "\0\0\0\0\0\0\0";
    char *point = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, VNULL(namelist));

    for (point = holder; isalpha(*namelist); namelist++, point++)
        *point = *namelist;

    *point = '\0';

    return (holder);
}

/*
 * str must be writable
 */
int split_string(char *str, const char *sep, char **argv)
{
    char *s = NULL;
    int argc = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %s, %08zx", __PRETTY_FUNCTION__, VNULL(str), VNULL(sep), (size_t)argv);

    s = strtok(str, sep);
    if (s)
        argv[argc++] = s;
    else
    {
        *argv = str;
        return 1;
    }

    while ((s = strtok(NULL, sep)))
    {
        argv[argc++] = s;
    }
    return argc;
}

int isname(const char *str, const char *namelist)
{
    char *argv[30];
    char *xargv[30];
    int argc = 0;
    int xargc = 0;
    int i = 0;
    int j = 0;
    int exact = FALSE;
    char buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char names[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char *s = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %s", __PRETTY_FUNCTION__, VNULL(str), VNULL(namelist));

    strcpy(buf, str);
    argc = split_string(buf, "- \t\r\n,", argv);

    strcpy(names, namelist);
    xargc = split_string(names, "- \t\r\n,", xargv);

    s = argv[argc - 1];
    s += strlen(s);
    if (*(--s) == '.')
    {
        exact = 1;
        *s = 0;
    }
    else
    {
        exact = 0;
    }
    /*
     * the string has now been split into separate words with the '-' replaced by string terminators.  pointers to the
     * beginning of each word are in argv
     */

    if (exact && argc != xargc)
        return 0;

    for (i = 0; i < argc; i++)
    {
        for (j = 0; j < xargc; j++)
        {
            if (0 == str_cmp(argv[i], xargv[j]))
            {
                xargv[j] = NULL;
                break;
            }
        }
        if (j >= xargc)
            return 0;
    }

    return 1;
}

void init_string_block(struct string_block *sb)
{
    if (DEBUG > 2)
        log_info("called %s with %08zx", __PRETTY_FUNCTION__, (size_t)sb);

    CREATE(sb->data, char, sb->size = 128);

    /*
     * Quixadhal: WHY send yourslef a SIG_ALARM if you're out of memory???
     */
#if 0
    if ((sb->data = (char *)malloc(sb->size = *128)))
	*sb->data = '\0';
    else {
	log_error("Malloc call to init_string_block failed.  Exiting.");
	kill(getpid(), 14);
    }
#endif
}

void append_to_string_block(struct string_block *sb, char *str)
{
    int len = 0;

    if (DEBUG > 2)
        log_info("called %s with %08zx, %s", __PRETTY_FUNCTION__, (size_t)sb, VNULL(str));

    len = strlen(sb->data) + strlen(str) + 1;
    if (len > sb->size)
    {
        if (len > (sb->size *= 2))
            sb->size = len;
        RECREATE(sb->data, char, sb->size);
    }
    strcat(sb->data, str);
}

void page_string_block(struct string_block *sb, struct char_data *ch)
{
    if (DEBUG > 2)
        log_info("called %s with %08zx, %s", __PRETTY_FUNCTION__, (size_t)sb, SAFE_NAME(ch));

    page_string(ch->desc, sb->data, 1);
}

void destroy_string_block(struct string_block *sb)
{
    if (DEBUG > 2)
        log_info("called %s with %08zx", __PRETTY_FUNCTION__, (size_t)sb);

    DESTROY(sb->data);
    sb->data = NULL;
}

void affect_modify(struct char_data *ch, char loc, char mod, long bitv, char add)
{
    int i = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %d, %ld, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), (int)loc, (int)mod, bitv,
                 (int)add);

    if (loc == APPLY_IMMUNE)
    {
        if (add)
        {
            SET_BIT(ch->immune, mod);
        }
        else
        {
            REMOVE_BIT(ch->immune, mod);
        }
    }
    else if (loc == APPLY_SUSC)
    {
        if (add)
        {
            SET_BIT(ch->susc, mod);
        }
        else
        {
            REMOVE_BIT(ch->susc, mod);
        }
    }
    else if (loc == APPLY_M_IMMUNE)
    {
        if (add)
        {
            SET_BIT(ch->M_immune, mod);
        }
        else
        {
            REMOVE_BIT(ch->M_immune, mod);
        }
    }
    else if (loc == APPLY_SPELL)
    {
        if (add)
        {
            SET_BIT(ch->specials.affected_by, mod);
        }
        else
        {
            REMOVE_BIT(ch->specials.affected_by, mod);
        }
    }
    else if (loc == APPLY_WEAPON_SPELL)
    {
        return;
    }
    else
    {
        if (add)
        {
            SET_BIT(ch->specials.affected_by, bitv);
        }
        else
        {
            REMOVE_BIT(ch->specials.affected_by, bitv);
            mod = -mod;
        }
    }

    switch (loc)
    {
    case APPLY_NONE:
        break;

    case APPLY_STR:
        GET_STR(ch) += mod;
        break;

    case APPLY_DEX:
        GET_DEX(ch) += mod;
        break;

    case APPLY_INT:
        GET_INT(ch) += mod;
        break;

    case APPLY_WIS:
        GET_WIS(ch) += mod;
        break;

    case APPLY_CON:
        GET_CON(ch) += mod;
        break;

    case APPLY_SEX:
        /*
         * ??? GET_SEX(ch) += mod;
         */
        break;

    case APPLY_CLASS:
        break;

    case APPLY_LEVEL:
        break;

    case APPLY_AGE:
        /*
         * ch->player.time.birth += mod;
         */
        break;

    case APPLY_CHAR_WEIGHT:
        GET_WEIGHT(ch) += mod;
        break;

    case APPLY_CHAR_HEIGHT:
        GET_HEIGHT(ch) += mod;
        break;

    case APPLY_MANA:
        ch->points.max_mana += mod;
        break;

    case APPLY_HIT:
        ch->points.max_hit += mod;
        break;

    case APPLY_MOVE:
        ch->points.max_move += mod;
        break;

    case APPLY_GOLD:
        break;

    case APPLY_EXP:
        break;

    case APPLY_AC:
        GET_AC(ch) += mod;
        break;

    case APPLY_HITROLL:
        GET_HITROLL(ch) += mod;
        break;

    case APPLY_DAMROLL:
        GET_DAMROLL(ch) += mod;
        break;

    case APPLY_SAVING_PARA:
        ch->specials.apply_saving_throw[0] += mod;
        break;

    case APPLY_SAVING_ROD:
        ch->specials.apply_saving_throw[1] += mod;
        break;

    case APPLY_SAVING_PETRI:
        ch->specials.apply_saving_throw[2] += mod;
        break;

    case APPLY_SAVING_BREATH:
        ch->specials.apply_saving_throw[3] += mod;
        break;

    case APPLY_SAVING_SPELL:
        ch->specials.apply_saving_throw[4] += mod;
        break;

    case APPLY_SAVE_ALL: {
        for (i = 0; i < 5; i++)
            ch->specials.apply_saving_throw[i] += mod;
    }
    break;
    case APPLY_IMMUNE:
        break;
    case APPLY_SUSC:
        break;
    case APPLY_M_IMMUNE:
        break;
    case APPLY_SPELL:
        break;
    case APPLY_HITNDAM:
        GET_HITROLL(ch) += mod;
        GET_DAMROLL(ch) += mod;
        break;
    case APPLY_WEAPON_SPELL:
    case APPLY_EAT_SPELL:
        break;
    case APPLY_BACKSTAB:
        ch->skills[SKILL_BACKSTAB].learned += mod;
        break;
    case APPLY_KICK:
        ch->skills[SKILL_KICK].learned += mod;
        break;
    case APPLY_SNEAK:
        ch->skills[SKILL_SNEAK].learned += mod;
        break;
    case APPLY_HIDE:
        ch->skills[SKILL_HIDE].learned += mod;
        break;
    case APPLY_BASH:
        ch->skills[SKILL_BASH].learned += mod;
        break;
    case APPLY_PICK:
        ch->skills[SKILL_PICK_LOCK].learned += mod;
        break;
    case APPLY_STEAL:
        ch->skills[SKILL_STEAL].learned += mod;
        break;
    case APPLY_TRACK:
        ch->skills[SKILL_TRACK].learned += mod;
        break;

    default:
        log_error("Unknown apply adjust attempt by %s.", ch->player.name);
        break;

    } /* switch */
}

/* This updates a character by subtracting everything he is affected by */
/* restoring original abilities, and then affecting all again           */

void affect_total(struct char_data *ch)
{
    struct affected_type *af = NULL;
    int i = 0;
    int j = 0;

    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

    for (i = 0; i < MAX_WEAR; i++)
    {
        if (ch->equipment[i])
            for (j = 0; j < MAX_OBJ_AFFECT; j++)
                affect_modify(ch, ch->equipment[i]->affected[j].location, ch->equipment[i]->affected[j].modifier,
                              ch->equipment[i]->obj_flags.bitvector, FALSE);
    }

    for (af = ch->affected; af; af = af->next)
        affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE);

    ch->tmpabilities = ch->abilities;

    for (i = 0; i < MAX_WEAR; i++)
    {
        if (ch->equipment[i])
            for (j = 0; j < MAX_OBJ_AFFECT; j++)
                affect_modify(ch, ch->equipment[i]->affected[j].location, ch->equipment[i]->affected[j].modifier,
                              ch->equipment[i]->obj_flags.bitvector, TRUE);
    }

    for (af = ch->affected; af; af = af->next)
        affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);

    /*
     * Make certain values are between 0..25, not < 0 and not > 25!
     */

    i = (IS_NPC(ch) ? 25 : 18);

    GET_DEX(ch) = MAX(0, MIN(GET_DEX(ch), i));
    GET_INT(ch) = MAX(0, MIN(GET_INT(ch), i));
    GET_WIS(ch) = MAX(0, MIN(GET_WIS(ch), i));
    GET_CON(ch) = MAX(0, MIN(GET_CON(ch), i));
    GET_STR(ch) = MAX(0, MIN(GET_STR(ch), i));
}

/* Insert an affect_type in a char_data structure
 * Automatically sets apropriate bits and apply's */
void affect_to_char(struct char_data *ch, struct affected_type *af)
{
    struct affected_type *affected_alloc = NULL;

    if (DEBUG > 1)
        log_info("called %s with %s, %08zx", __PRETTY_FUNCTION__, SAFE_NAME(ch), (size_t)af);

    CREATE(affected_alloc, struct affected_type, 1);

    *affected_alloc = *af;
    affected_alloc->next = ch->affected;
    ch->affected = affected_alloc;

    affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);
    affect_total(ch);
}

/* Remove an affected_type structure from a char (called when duration
 * reaches zero). Pointer *af must never be NIL! Frees mem and calls
 * affect_location_apply                                                */
void affect_remove(struct char_data *ch, struct affected_type *af)
{
    struct affected_type *hjp = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %08zx", __PRETTY_FUNCTION__, SAFE_NAME(ch), (size_t)af);

    if (!ch->affected)
        return;
    affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE);

    /*
     * remove structure *af from linked list
     */

    if (ch->affected == af)
    {
        /*
         * remove head of list
         */
        ch->affected = af->next;
    }
    else
    {
        for (hjp = ch->affected; (hjp->next) && (hjp->next != af); hjp = hjp->next)
            ;

        if (hjp->next != af)
        {
            log_error("Could not locate affected_type in ch->affected.");
            return;
        }
        hjp->next = af->next; /* skip the af element */
    }
    DESTROY(af);
    affect_total(ch);
}

/* Call affect_remove with every spell of spelltype "skill" */
void affect_from_char(struct char_data *ch, short skill)
{
    struct affected_type *hjp = NULL;

    if (DEBUG > 1)
        log_info("called %s with %s, %hd", __PRETTY_FUNCTION__, SAFE_NAME(ch), skill);

    for (hjp = ch->affected; hjp; hjp = hjp->next)
        if (hjp->type == skill)
            affect_remove(ch, hjp);
}

/* Return if a char is affected by a spell (SPELL_XXX), NULL indicates
 * not affected                                                        */
char affected_by_spell(struct char_data *ch, short skill)
{
    struct affected_type *hjp = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %hd", __PRETTY_FUNCTION__, SAFE_NAME(ch), skill);

    for (hjp = ch->affected; hjp; hjp = hjp->next)
        if (hjp->type == skill)
            return (TRUE);

    return (FALSE);
}

void affect_join(struct char_data *ch, struct affected_type *af, char avg_dur, char avg_mod)
{
    struct affected_type *hjp = NULL;
    char found = FALSE;

    if (DEBUG > 2)
        log_info("called %s with %s, %08zx, %d, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), (size_t)af, (int)avg_dur,
                 (int)avg_mod);

    for (hjp = ch->affected; !found && hjp; hjp = hjp->next)
    {
        if (hjp->type == af->type)
        {

            af->duration += hjp->duration;
            if (avg_dur)
                af->duration /= 2;

            af->modifier += hjp->modifier;
            if (avg_mod)
                af->modifier /= 2;

            affect_remove(ch, hjp);
            affect_to_char(ch, af);
            found = TRUE;
        }
    }
    if (!found)
        affect_to_char(ch, af);
}

/* move a player out of a room */
void char_from_room(struct char_data *ch)
{
    struct char_data *i = NULL;
    struct room_data *rp = NULL;

    if (DEBUG > 1)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

    if (ch->in_room == NOWHERE)
    {
        log_error("NOWHERE extracting char from room");
        return;
    }
    if (ch->equipment[WEAR_LIGHT])
        if (ch->equipment[WEAR_LIGHT]->obj_flags.type_flag == ITEM_LIGHT)
            if (ch->equipment[WEAR_LIGHT]->obj_flags.value[2])
            { /* Light is ON */
                real_roomp(ch->in_room)->light--;
                if (real_roomp(ch->in_room)->light < 1)
                    reprintf(ch->in_room, ch, "A source of light leaves the room....\r\n");
            }
    rp = real_roomp(ch->in_room);
    if (rp == NULL)
    {
        log_info("ERROR: char_from_room: %s was not in a valid room (%d)",
                 (!IS_NPC(ch) ? (ch)->player.name : (ch)->player.short_descr), ch->in_room);
        return;
    }
    if (ch == rp->people) /* head of list */
        rp->people = ch->next_in_room;

    else
    { /* locate the previous element */
        for (i = rp->people; i && i->next_in_room != ch; i = i->next_in_room)
            ;
        if (i)
            i->next_in_room = ch->next_in_room;
        else
        {
            log_error("SHIT, %s was not in people list of his room %d!",
                      (!IS_NPC(ch) ? (ch)->player.name : (ch)->player.short_descr), ch->in_room);
        }
    }

    ch->in_room = NOWHERE;
    ch->next_in_room = 0;
}

/* place a character in a room */
void char_to_room(struct char_data *ch, int room)
{
    struct room_data *rp = NULL;

    if (DEBUG > 1)
        log_info("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), room);

    rp = real_roomp(room);
    if (!rp)
    {
        room = 0;
        rp = real_roomp(room);
        if (!rp)
        {
            log_fatal("Cannot lookup room %d!", room);
            proper_exit(MUD_HALT);
        }
    }
    ch->next_in_room = rp->people;
    rp->people = ch;
    ch->in_room = room;

    if (ch->equipment[WEAR_LIGHT])
        if (ch->equipment[WEAR_LIGHT]->obj_flags.type_flag == ITEM_LIGHT)
            if (ch->equipment[WEAR_LIGHT]->obj_flags.value[2])
            { /* Light is ON */
                if ((rp->light < 1) && (ch->in_room))
                    reprintf(ch->in_room, ch, "A source of light enters the room...\r\n");
                rp->light++;
            }
}

/* give an object to a char   */
void obj_to_char(struct obj_data *object, struct char_data *ch)
{
    if (DEBUG > 1)
        log_info("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_ONAME(object), SAFE_NAME(ch));

    if (ch->carrying)
        object->next_content = ch->carrying;
    else
        object->next_content = 0;

    ch->carrying = object;
    object->carried_by = ch;
    object->in_room = NOWHERE;
    IS_CARRYING_W(ch) += GET_OBJ_WEIGHT(object);
    IS_CARRYING_N(ch)++;
}

/* take an object from a char */
void obj_from_char(struct obj_data *object)
{
    struct obj_data *tmp = NULL;

    if (DEBUG > 1)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_ONAME(object));

    if (!object)
    {
        object = 0;
        return;
    }
    if (!object->carried_by)
    {
        object = 0;
        return;
    }
    if (!object->carried_by->carrying)
    {
        object = 0;
        return;
    }
    if (object->carried_by->carrying == object) /* head of list */
        object->carried_by->carrying = object->next_content;

    else
    {
        for (tmp = object->carried_by->carrying; tmp && (tmp->next_content != object); tmp = tmp->next_content)
            ; /* locate
               * previous
               */

        if (!tmp)
        {
            object = 0;
            return;
        }
        tmp->next_content = object->next_content;
    }

    IS_CARRYING_W(object->carried_by) -= GET_OBJ_WEIGHT(object);
    IS_CARRYING_N(object->carried_by)--;
    object->carried_by = 0;
    object->equipped_by = 0; /* should be unnecessary, but, why risk it */
    object->next_content = 0;
}

/* Return the effect of a piece of armor in position eq_pos */
int apply_ac(struct char_data *ch, int eq_pos)
{
    if (DEBUG > 2)
        log_info("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), eq_pos);

    if (!ch->equipment[eq_pos])
        return 0;
    if (DEBUG)
        log_info("apply_ac");
    if (!(GET_ITEM_TYPE(ch->equipment[eq_pos]) == ITEM_ARMOR))
        return 0;

    switch (eq_pos)
    {
    case WEAR_BODY:
        return (3 * ch->equipment[eq_pos]->obj_flags.value[0]); /* 30% */
    case WEAR_HEAD:
        return (2 * ch->equipment[eq_pos]->obj_flags.value[0]); /* 20% */
    case WEAR_LEGS:
        return (2 * ch->equipment[eq_pos]->obj_flags.value[0]); /* 20% */
    case WEAR_FEET:
        return (ch->equipment[eq_pos]->obj_flags.value[0]); /* 10% */
    case WEAR_HANDS:
        return (ch->equipment[eq_pos]->obj_flags.value[0]); /* 10% */
    case WEAR_ARMS:
        return (ch->equipment[eq_pos]->obj_flags.value[0]); /* 10% */
    case WEAR_SHIELD:
        return (ch->equipment[eq_pos]->obj_flags.value[0]); /* 10% */
    }
    return 0;
}

void equip_char(struct char_data *ch, struct obj_data *obj, int pos)
{
    int j = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), SAFE_ONAME(obj), pos);

    if (pos < 0 || pos >= MAX_WEAR)
        return;
    if (DEBUG)
        log_info("equip_char");
    if (obj->carried_by)
    {
        log_info("EQUIP: Obj is carried_by when equip.");
        return;
    }
    if (obj->in_room != NOWHERE)
    {
        log_info("EQUIP: Obj is in_room when equip.");
        return;
    }
    if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch)) || (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch)) ||
        (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch)))
    {
        if (ch->in_room != NOWHERE)
        {
            act("You are zapped by $p and instantly drop it.", FALSE, ch, obj, 0, TO_CHAR);
            act("$n is zapped by $p and instantly drop it.", FALSE, ch, obj, 0, TO_ROOM);
            obj_to_room(obj, ch->in_room);
            return;
        }
        else
        {
            log_info("ch->in_room = NOWHERE when equipping char.");
        }
    }
    ch->equipment[pos] = obj;
    obj->equipped_by = ch;
    obj->eq_pos = pos;

    if (GET_ITEM_TYPE(obj) == ITEM_ARMOR)
        GET_AC(ch) -= apply_ac(ch, pos);

    for (j = 0; j < MAX_OBJ_AFFECT; j++)
        affect_modify(ch, obj->affected[j].location, obj->affected[j].modifier, obj->obj_flags.bitvector, TRUE);

    affect_total(ch);
}

struct obj_data *unequip_char(struct char_data *ch, int pos)
{
    int j = 0;
    struct obj_data *obj = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), pos);

    if (pos < 0 || pos >= MAX_WEAR)
        return NULL;
    if (!ch->equipment[pos])
        return NULL;
    if (DEBUG)
        log_info("unequip_char");
    obj = ch->equipment[pos];
    if (GET_ITEM_TYPE(obj) == ITEM_ARMOR)
        GET_AC(ch) += apply_ac(ch, pos);

    ch->equipment[pos] = 0;
    obj->equipped_by = NULL;
    obj->eq_pos = -1;

    for (j = 0; j < MAX_OBJ_AFFECT; j++)
        affect_modify(ch, obj->affected[j].location, obj->affected[j].modifier, obj->obj_flags.bitvector, FALSE);

    affect_total(ch);

    return (obj);
}

int get_number(char **name)
{
    int i = 0;
    char *ppos = NULL;
    char anumber[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char spare[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG > 2)
        log_info("called %s with %08zx", __PRETTY_FUNCTION__, (size_t)name);

    if ((ppos = (char *)index(*name, '.')) && ppos[1])
    {
        *ppos++ = '\0';
        strcpy(anumber, *name); /* bleed off digits */
        strcpy(spare, ppos);    /* move name to a tmp space, in case of overlap, thanks
                                 * valgrind */
        strcpy(*name, spare);

        for (i = 0; *(anumber + i); i++)
            if (!isdigit(*(anumber + i)))
                return (0);

        return (atoi(anumber));
    }
    return (1);
}

/* Search a given list for an object, and return a pointer to that object */
struct obj_data *get_obj_in_list(const char *name, struct obj_data *list)
{
    struct obj_data *i = NULL;
    int j = 0;
    int anumber = 0;
    char tmpname[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char *tmp = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %08zx", __PRETTY_FUNCTION__, VNULL(name), (size_t)list);

    strcpy(tmpname, name);
    tmp = tmpname;
    /*
     * need ---
     * special handlers for 2*thing, all.thing
     *
     * should be built into each command (get, put, buy)
     *
     * if (getall(name) == tRUE) {
     * while *(i = getobj_in_list()) != NULL) {
     * blah
     * blah
     * blah
     * }
     * } else if ((p = getabunch(name)) != NULL) {
     * while (p > 0) {
     * i = get_obj_in_list();
     * blah
     * blah
     * blah
     * p--;
     * }
     * }
     */

    if (!(anumber = get_number(&tmp)))
        return (0);

    for (i = list, j = 1; i && (j <= anumber); i = i->next_content)
        if (isname(tmp, i->name))
        {
            if (j == anumber)
                return (i);
            j++;
        }
    return (0);
}

/* Search a given list for an object number, and return a ptr to that obj */
struct obj_data *get_obj_in_list_num(int num, struct obj_data *list)
{
    struct obj_data *i = NULL;

    if (DEBUG > 2)
        log_info("called %s with %d, %08zx", __PRETTY_FUNCTION__, num, (size_t)list);

    for (i = list; i; i = i->next_content)
        if (i->item_number == num)
            return (i);

    return (0);
}

/*search the entire world for an object, and return a pointer  */
struct obj_data *get_obj(const char *name)
{
    struct obj_data *i = NULL;
    int j = 0;
    int anumber = 0;
    char tmpname[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char *tmp = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, VNULL(name));

    strcpy(tmpname, name);
    tmp = tmpname;
    if (!(anumber = get_number(&tmp)))
        return (0);

    for (i = object_list, j = 1; i && (j <= anumber); i = i->next)
        if (isname(tmp, i->name))
        {
            if (j == anumber)
                return (i);
            j++;
        }
    return (0);
}

/*search the entire world for an object number, and return a pointer  */
struct obj_data *get_obj_num(int nr)
{
    struct obj_data *i = NULL;

    if (DEBUG > 2)
        log_info("called %s with %d", __PRETTY_FUNCTION__, nr);

    for (i = object_list; i; i = i->next)
        if (i->item_number == nr)
            return (i);

    return (0);
}

/* search a room for a char, and return a pointer if found..  */
struct char_data *get_char_room(const char *name, int room)
{
    struct char_data *i = NULL;
    int j = 0;
    int anumber = 0;
    char tmpname[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char *tmp = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %d", __PRETTY_FUNCTION__, VNULL(name), room);

    strcpy(tmpname, name);
    tmp = tmpname;
    if (!(anumber = get_number(&tmp)))
        return (0);

    for (i = real_roomp(room)->people, j = 1; i && (j <= anumber); i = i->next_in_room)
        if (isname(tmp, GET_NAME(i)))
        {
            if (j == anumber)
                return (i);
            j++;
        }
    return (0);
}

/* search all over the world for a char, and return a pointer if found */
struct char_data *get_char(const char *name)
{
    struct char_data *i = NULL;
    int j = 0;
    int anumber = 0;
    char tmpname[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char *tmp = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, VNULL(name));

    strcpy(tmpname, name);
    tmp = tmpname;
    if (!(anumber = get_number(&tmp)))
        return (0);

    for (i = character_list, j = 1; i && (j <= anumber); i = i->next)
        if (isname(tmp, GET_NAME(i)))
        {
            if (j == anumber)
                return (i);
            j++;
        }
    return (0);
}

/* search all over the world for a char num, and return a pointer if found */
struct char_data *get_char_num(int nr)
{
    struct char_data *i = NULL;

    if (DEBUG > 2)
        log_info("called %s with %d", __PRETTY_FUNCTION__, nr);

    for (i = character_list; i; i = i->next)
        if (i->nr == nr)
            return (i);

    return (0);
}

/* put an object in a room */
void obj_to_room(struct obj_data *object, int room)
{
    if (DEBUG > 1)
        log_info("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_ONAME(object), room);

    if (room == -1)
        room = 4;

    object->next_content = real_roomp(room)->contents;
    real_roomp(room)->contents = object;
    object->in_room = room;
    object->carried_by = 0;
    object->equipped_by = 0; /* should be unnecessary */
}

/* Take an object from a room */
void obj_from_room(struct obj_data *object)
{
    struct obj_data *i = NULL;

    if (DEBUG > 1)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_ONAME(object));
    /*
     * remove object from room
     */

    if (object == real_roomp(object->in_room)->contents) /* head of list */
        real_roomp(object->in_room)->contents = object->next_content;

    else
    { /* locate previous element in list */
        for (i = real_roomp(object->in_room)->contents; i && (i->next_content != object); i = i->next_content)
            ;

        i->next_content = object->next_content;
    }

    object->in_room = NOWHERE;
    object->next_content = 0;
}

/* put an object in an object (quaint)  */
void obj_to_obj(struct obj_data *obj, struct obj_data *obj_to)
{
    struct obj_data *tmp_obj = NULL;

    if (DEBUG > 1)
        log_info("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_ONAME(obj), SAFE_ONAME(obj_to));

    obj->next_content = obj_to->contains;
    obj_to->contains = obj;
    obj->in_obj = obj_to;
    /*
     * (jdb)  hopefully this will fix the object problem
     */
    obj->carried_by = 0;

    for (tmp_obj = obj->in_obj; tmp_obj; GET_OBJ_WEIGHT(tmp_obj) += GET_OBJ_WEIGHT(obj), tmp_obj = tmp_obj->in_obj)
        ;
}

/* remove an object from an object */
void obj_from_obj(struct obj_data *obj)
{
    struct obj_data *tmp = NULL;
    struct obj_data *obj_from = NULL;

    if (DEBUG > 1)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_ONAME(obj));

    if (obj->in_obj)
    {
        obj_from = obj->in_obj;
        if (obj == obj_from->contains) /* head of list */
            obj_from->contains = obj->next_content;
        else
        {
            for (tmp = obj_from->contains; tmp && (tmp->next_content != obj); tmp = tmp->next_content)
                ; /* locate
                   * previous */

            if (!tmp)
            {
                log_fatal("Fatal error in object structures.");
                proper_exit(MUD_HALT);
            }
            tmp->next_content = obj->next_content;
        }

        /*
         * Subtract weight from containers container
         */
        for (tmp = obj->in_obj; tmp->in_obj; tmp = tmp->in_obj)
            GET_OBJ_WEIGHT(tmp) -= GET_OBJ_WEIGHT(obj);

        GET_OBJ_WEIGHT(tmp) -= GET_OBJ_WEIGHT(obj);

        /*
         * Subtract weight from char that carries the object
         */
        if (tmp->carried_by)
            IS_CARRYING_W(tmp->carried_by) -= GET_OBJ_WEIGHT(obj);

        obj->in_obj = 0;
        obj->next_content = 0;
    }
    else
    {
        log_fatal("Trying to object from object when in no object.");
        proper_exit(MUD_HALT);
    }
}

/* Set all carried_by to point to new owner */
void object_list_new_owner(struct obj_data *list, struct char_data *ch)
{
    if (DEBUG > 2)
        log_info("called %s with %08zx, %s", __PRETTY_FUNCTION__, (size_t)list, SAFE_NAME(ch));

    if (list)
    {
        object_list_new_owner(list->contains, ch);
        object_list_new_owner(list->next_content, ch);
        list->carried_by = ch;
    }
}

/* Extract an object from the world */
void extract_obj(struct obj_data *obj)
{
    struct obj_data *temp1 = NULL;
    struct obj_data *temp2 = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_ONAME(obj));

    if (obj->in_room != NOWHERE)
        obj_from_room(obj);
    else if (obj->carried_by)
        obj_from_char(obj);
    else if (obj->equipped_by)
    {
        if (obj->eq_pos > -1)
        {
            /*
             **  set players equipment slot to 0; that will avoid the garbage items.
             */
            obj->equipped_by->equipment[(int)obj->eq_pos] = 0;
        }
        else
        {
            log_error("Extract on equipped item in slot -1 on: %s %s", obj->equipped_by->player.name, obj->name);
            return;
        }
    }
    else if (obj->in_obj)
    {
        temp1 = obj->in_obj;
        if (temp1->contains == obj) /* head of list */
            temp1->contains = obj->next_content;
        else
        {
            for (temp2 = temp1->contains; temp2 && (temp2->next_content != obj); temp2 = temp2->next_content)
                ;

            if (temp2)
            {
                temp2->next_content = obj->next_content;
            }
        }
    }
    for (; obj->contains; extract_obj(obj->contains))
        ;
    /*
     * leaves nothing !
     */

    if (object_list == obj) /* head of list */
        object_list = obj->next;
    else
    {
        for (temp1 = object_list; temp1 && (temp1->next != obj); temp1 = temp1->next)
            ;

        if (temp1)
            temp1->next = obj->next;
    }

    if (obj->item_number >= 0)
        (obj_index[obj->item_number].number)--;
    free_obj(obj);
}

void update_object(struct obj_data *obj, int use)
{
    if (DEBUG > 2)
        log_info("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_ONAME(obj), use);

    if (obj->obj_flags.timer > 0)
        obj->obj_flags.timer -= use;
    if (obj->contains)
        update_object(obj->contains, use);
    if (obj->next_content)
        if (obj->next_content != obj)
            update_object(obj->next_content, use);
}

void update_char_objects(struct char_data *ch)
{
    int i = 0;

    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

    if (ch->equipment[WEAR_LIGHT])
        if (ch->equipment[WEAR_LIGHT]->obj_flags.type_flag == ITEM_LIGHT)
            if (ch->equipment[WEAR_LIGHT]->obj_flags.value[2] > 0)
                (ch->equipment[WEAR_LIGHT]->obj_flags.value[2])--;

    for (i = 0; i < MAX_WEAR; i++)
        if (ch->equipment[i])
            update_object(ch->equipment[i], 2);

    if (ch->carrying)
        update_object(ch->carrying, 1);
}

/* Extract a ch completely from the world, and leave his stuff behind */
void extract_char(struct char_data *ch)
{
    struct obj_data *i = NULL;
    struct obj_data *o = NULL;
    struct char_data *k = NULL;
    struct char_data *next_char = NULL;
    struct descriptor_data *t_desc = NULL;
    int l = 0;
    int was_in = FALSE;
    int j = 0;

    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

    if (IS_PC(ch) && !ch->desc)
    {
        for (t_desc = descriptor_list; t_desc; t_desc = t_desc->next)
            if (t_desc->original == ch)
                do_return(t_desc->character, "", 0);
    }
    if (ch->in_room == NOWHERE)
    {
        log_error("NOWHERE extracting char.");
        /*
         **  problem from linkdeath
         */
        char_to_room(ch, 0); /* 0 == all purpose store */
    }
    if (ch->followers || ch->master)
        die_follower(ch);

    if (ch->desc)
    {
        /*
         * Forget snooping
         */
        if (ch->desc->snoop.snooping)
            ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;

        if (ch->desc->snoop.snoop_by)
        {
            cprintf(ch->desc->snoop.snoop_by, "Your victim is no longer among us.\r\n");
            ch->desc->snoop.snoop_by->desc->snoop.snooping = 0;
        }
        ch->desc->snoop.snooping = ch->desc->snoop.snoop_by = 0;
    }
    if (ch->carrying)
    {
        /*
         * transfer ch's objects to room
         */

        if (!IS_IMMORTAL(ch))
        {
            if (real_roomp(ch->in_room)->contents)
            { /* room nonempty */
                /*
                 * locate tail of room-contents
                 */
                for (i = real_roomp(ch->in_room)->contents; i->next_content; i = i->next_content)
                    ;

                /*
                 * append ch's stuff to room-contents
                 */
                i->next_content = ch->carrying;
            }
            else
                real_roomp(ch->in_room)->contents = ch->carrying;

            /*
             * connect the stuff to the room
             */
            for (i = ch->carrying; i; i = i->next_content)
            {
                i->carried_by = 0;
                i->in_room = ch->in_room;
            }
        }
        else
        {
            cprintf(ch, "Here, you dropped some stuff, let me help you get rid of that.\r\n");
            for (i = ch->carrying; i; i = o)
            {
                o = i->next_content;
                extract_obj(i);
            }
            /*
             * equipment too
             */
            for (j = 0; j < MAX_WEAR; j++)
                if (ch->equipment[j])
                    extract_obj(unequip_char(ch, j));
        }
    }
    if (ch->specials.fighting)
        stop_fighting(ch);

    for (k = combat_list; k; k = next_char)
    {
        next_char = k->next_fighting;
        if (k->specials.fighting == ch)
            stop_fighting(k);
    }

    /*
     * Must remove from room before removing the equipment!
     */
    was_in = ch->in_room;
    char_from_room(ch);

    /*
     * clear equipment_list
     */
    for (l = 0; l < MAX_WEAR; l++)
        if (ch->equipment[l])
            obj_to_room(unequip_char(ch, l), was_in);

    if (IS_NPC(ch))
    {
        for (k = character_list; k; k = k->next)
        {
            if (k->specials.hunting)
                if (k->specials.hunting == ch)
                {
                    k->specials.hunting = 0;
                }
            if (DoesHate(k, ch))
            {
                RemHated(k, ch);
            }
            if (DoesFear(k, ch))
            {
                RemFeared(k, ch);
            }
        }
    }
    else
    {
        for (k = character_list; k; k = k->next)
        {
            if (k->specials.hunting)
                if (k->specials.hunting == ch)
                {
                    k->specials.hunting = 0;
                }
            if (DoesHate(k, ch))
            {
                ZeroHatred(k, ch);
            }
            if (DoesFear(k, ch))
            {
                ZeroFeared(k, ch);
            }
        }
    }
    /*
     * pull the char from the list
     */

    if (ch == character_list)
        character_list = ch->next;
    else
    {
        for (k = character_list; (k) && (k->next != ch); k = k->next)
            ;
        if (k)
            k->next = ch->next;
        else
        {
            log_error("Trying to remove NULL from character_list.");
            /*
             * proper_exit(MUD_HALT);
             */
        }
    }

    if (ch->desc)
    {
        if (ch->desc->original)
            do_return(ch, "", 0);
        save_char(ch, NOWHERE);
    }
    if (IS_NPC(ch))
    {
        if (ch->nr > -1) /* if mobile */
            mob_index[ch->nr].number--;
        FreeHates(ch);
        FreeFears(ch);
        free_char(ch);
    }
    else if (ch->desc)
    { /* Moved the following into an else block since */
        /*
         * valgrind pointed out that ch won't be valid for NPC's
         */
        ch->desc->connected = CON_MENU_SELECT;
        SEND_TO_Q(login_menu, ch->desc);
    }
}

/* ***********************************************************************
 * Here follows high-level versions of some earlier routines, ie functionst
 * which incorporate the actual player-data.
 * *********************************************************************** */

struct char_data *get_char_room_vis(struct char_data *ch, const char *name)
{
    struct char_data *i = NULL;
    int j = 0;
    int anumber = 0;
    char tmpname[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char *tmp = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(name));

    if ((!strcasecmp(name, "me") || !strcasecmp(name, "myself")) && CAN_SEE(ch, ch))
        return ch;

    strcpy(tmpname, name);
    tmp = tmpname;
    if (!(anumber = get_number(&tmp)))
        return (0);

    for (i = real_roomp(ch->in_room)->people, j = 1; i && (j <= anumber); i = i->next_in_room)
        if (isname(tmp, GET_NAME(i)))
            if (CAN_SEE(ch, i))
            {
                if (j == anumber)
                    return (i);
                j++;
            }
    return (0);
}

/* get a character from anywhere in the world, doesn't care much about
 * being in the same room... */
struct char_data *get_char_vis_world(struct char_data *ch, const char *name, int *count)
{
    struct char_data *i = NULL;
    int j = 0;
    int anumber = 0;
    char tmpname[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char *tmp = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %s, %08zx", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(name), (size_t)count);

    if ((!strcasecmp(name, "me") || !strcasecmp(name, "myself")) && CAN_SEE(ch, ch))
        return ch;

    strcpy(tmpname, name);
    tmp = tmpname;
    if (!(anumber = get_number(&tmp)))
        return (0);

    j = count ? *count : 1;
    for (i = character_list; i && (j <= anumber); i = i->next)
        if (isname(tmp, GET_NAME(i)))
            if (CAN_SEE(ch, i))
            {
                if (j == anumber)
                    return (i);
                j++;
            }
    if (count)
        *count = j;
    return 0;
}

struct char_data *get_char_vis(struct char_data *ch, const char *name)
{
    struct char_data *i = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(name));

    /*
     * check location
     */
    if ((i = get_char_room_vis(ch, name)))
        return (i);

    return get_char_vis_world(ch, name, NULL);
}

struct obj_data *get_obj_in_list_vis(struct char_data *ch, const char *name, struct obj_data *list)
{
    struct obj_data *i = NULL;
    int j = 0;
    int anumber = 0;
    char tmpname[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char *tmp = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %s, %08zx", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(name), (size_t)list);

    strcpy(tmpname, name);
    tmp = tmpname;
    if (!(anumber = get_number(&tmp)))
        return (0);

    for (i = list, j = 1; i && (j <= anumber); i = i->next_content)
        if (isname(tmp, i->name))
            if (CAN_SEE_OBJ(ch, i))
            {
                if (j == anumber)
                    return (i);
                j++;
            }
    return (0);
}

struct obj_data *get_obj_vis_world(struct char_data *ch, const char *name, int *count)
{
    struct obj_data *i = NULL;
    int j = 0;
    int anumber = 0;
    char tmpname[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char *tmp = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %s, %08zx", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(name), (size_t)count);

    strcpy(tmpname, name);
    tmp = tmpname;
    if (!(anumber = get_number(&tmp)))
        return (0);

    j = count ? *count : 1;

    /*
     * ok.. no luck yet. scan the entire obj list
     */
    for (i = object_list; i && (j <= anumber); i = i->next)
        if (isname(tmp, i->name))
            if (CAN_SEE_OBJ(ch, i))
            {
                if (j == anumber)
                    return (i);
                j++;
            }
    if (count)
        *count = j;
    return (0);
}

/*search the entire world for an object, and return a pointer  */
struct obj_data *get_obj_vis(struct char_data *ch, const char *name)
{
    struct obj_data *i = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(name));

    /*
     * scan items carried
     */
    if ((i = get_obj_in_list_vis(ch, name, ch->carrying)))
        return (i);

    /*
     * scan room
     */
    if ((i = get_obj_in_list_vis(ch, name, real_roomp(ch->in_room)->contents)))
        return (i);

    return get_obj_vis_world(ch, name, NULL);
}

struct obj_data *get_obj_vis_accessible(struct char_data *ch, const char *name)
{
    struct obj_data *i = NULL;
    int j = 0;
    int anumber = 0;
    char tmpname[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char *tmp = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(name));

    strcpy(tmpname, name);
    tmp = tmpname;
    if (!(anumber = get_number(&tmp)))
        return (0);

    /*
     * scan items carried
     */
    for (i = ch->carrying, j = 1; i && j <= anumber; i = i->next_content)
        if (isname(tmp, i->name) && CAN_SEE_OBJ(ch, i))
        {
            if (j == anumber)
                return (i);
            else
                j++;
        }
    for (i = real_roomp(ch->in_room)->contents; i && j <= anumber; i = i->next_content)
        if (isname(tmp, i->name) && CAN_SEE_OBJ(ch, i))
        {
            if (j == anumber)
                return (i);
            else
                j++;
        }
    return 0;
}

struct obj_data *create_money(int amount)
{
    struct obj_data *obj = NULL;
    struct extra_descr_data *new_descr = NULL;
    char buf[80] = "\0\0\0\0\0\0\0";

    if (DEBUG > 2)
        log_info("called %s with %d", __PRETTY_FUNCTION__, amount);

    if (amount <= 0)
    {
        log_fatal("Trying to create negative money.");
        proper_exit(MUD_HALT);
    }
    CREATE(obj, struct obj_data, 1);
    CREATE(new_descr, struct extra_descr_data, 1);

    clear_object(obj);

    if (amount == 1)
    {
        obj->name = strdup("coin gold");
        obj->short_description = strdup("a gold coin");
        obj->description = strdup("One miserable gold coin.");

        new_descr->keyword = strdup("coin gold");
        new_descr->description = strdup("One miserable gold coin.");
    }
    else
    {
        obj->name = strdup("coins gold");
        obj->short_description = strdup("gold coins");
        obj->description = strdup("A pile of gold coins.");

        new_descr->keyword = strdup("coins gold");
        if (amount < 10)
        {
            sprintf(buf, "There is %d coins.", amount);
            new_descr->description = strdup(buf);
        }
        else if (amount < 100)
        {
            sprintf(buf, "There is about %d coins", 10 * (amount / 10));
            new_descr->description = strdup(buf);
        }
        else if (amount < 1000)
        {
            sprintf(buf, "It looks like something round %d coins", 100 * (amount / 100));
            new_descr->description = strdup(buf);
        }
        else if (amount < 100000)
        {
            sprintf(buf, "You guess there is %d coins", 1000 * ((amount / 1000) + number(0, (amount / 1000))));
            new_descr->description = strdup(buf);
        }
        else
            new_descr->description = strdup("There is A LOT of coins");
    }

    new_descr->next = 0;
    obj->ex_description = new_descr;

    obj->obj_flags.type_flag = ITEM_MONEY;
    obj->obj_flags.wear_flags = ITEM_TAKE;
    obj->obj_flags.value[0] = amount;
    obj->obj_flags.cost = amount;
    obj->item_number = -1;

    obj->next = object_list;
    object_list = obj;

    return (obj);
}

/* Generic Find, designed to find any object/character                    */
/* Calling :                                                              */
/*  *arg     is the sting containing the string to be searched for.       */
/*           This string doesn't have to be a single word, the routine    */
/*           extracts the next word itself.                               */
/*  bitv..   All those bits that you want to "search through".            */
/*           Bit found will be result of the function                     */
/*  *ch      This is the person that is trying to "find"                  */
/*  **tar_ch Will be NULL if no character was found, otherwise points     */
/* **tar_obj Will be NULL if no object was found, otherwise points        */
/*                                                                        */
/* The routine returns a pointer to the next word in *arg (just like the  */
/* one_argument routine).                                                 */

int generic_find(const char *arg, int bitvector, struct char_data *ch, struct char_data **tar_ch,
                 struct obj_data **tar_obj)
{
    char name[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    int i = 0;
    int found = FALSE;

    static const char *ignore[] = {"the", "in", "on", "at", "\n"};

    if (DEBUG > 2)
        log_info("called %s with %d, %s, %08zx, %08zx", __PRETTY_FUNCTION__, bitvector, SAFE_NAME(ch), (size_t)tar_ch,
                 (size_t)tar_obj);

    /*
     * Eliminate spaces and "ignore" words
     */
    while (*arg && !found)
    {

        for (; *arg == ' '; arg++)
            ;

        for (i = 0; (name[i] = *(arg + i)) && (name[i] != ' '); i++)
            ;
        name[i] = 0;
        arg += i;
        if (search_block(name, ignore, TRUE) > -1)
            found = TRUE;
    }

    if (!name[0])
        return (0);

    *tar_ch = 0;
    *tar_obj = 0;

    if (IS_SET(bitvector, FIND_CHAR_ROOM))
    { /* Find person in room */
        if ((*tar_ch = get_char_room_vis(ch, name)))
        {
            return (FIND_CHAR_ROOM);
        }
    }
    if (IS_SET(bitvector, FIND_CHAR_WORLD))
    {
        if ((*tar_ch = get_char_vis(ch, name)))
        {
            return (FIND_CHAR_WORLD);
        }
    }
    if (IS_SET(bitvector, FIND_OBJ_EQUIP))
    {
        for (found = FALSE, i = 0; i < MAX_WEAR && !found; i++)
            if (ch->equipment[i] && str_cmp(name, ch->equipment[i]->name) == 0)
            {
                *tar_obj = ch->equipment[i];
                found = TRUE;
            }
        if (found)
        {
            return (FIND_OBJ_EQUIP);
        }
    }
    if (IS_SET(bitvector, FIND_OBJ_INV))
    {
        if (IS_SET(bitvector, FIND_OBJ_ROOM))
        {
            if ((*tar_obj = get_obj_vis_accessible(ch, name)))
            {
                return (FIND_OBJ_INV);
            }
        }
        else
        {
            if ((*tar_obj = get_obj_in_list_vis(ch, name, ch->carrying)))
            {
                return (FIND_OBJ_INV);
            }
        }
    }
    if (IS_SET(bitvector, FIND_OBJ_ROOM))
    {
        if ((*tar_obj = get_obj_in_list_vis(ch, name, real_roomp(ch->in_room)->contents)))
        {
            return (FIND_OBJ_ROOM);
        }
    }
    if (IS_SET(bitvector, FIND_OBJ_WORLD))
    {
        if ((*tar_obj = get_obj_vis(ch, name)))
        {
            return (FIND_OBJ_WORLD);
        }
    }
    return (0);
}

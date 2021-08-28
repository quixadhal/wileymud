/*
 * file: spell_parser.c , Basic routines and parsing      Part of DIKUMUD
 * Usage : Interpreter of spells
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */

#include <stdio.h>
#include <stdlib.h>
/* #include <unistd.h> */
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <assert.h>

#include "global.h"
#include "bug.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "interpreter.h"
#include "handler.h"
#include "spells.h"
#include "act_off.h"
#include "random.h"
#include "multiclass.h"
#include "fight.h"
#include "act_info.h"
#include "spells.h" // spell_info_type

#define _SPELL_PARSER_C
#include "spell_parser.h"

struct spell_info_type spell_info[MAX_SKILLS];

const char saving_throws[ABS_MAX_CLASS][MAX_SAVING_THROWS][ABS_MAX_LVL] = {
    {{16, 14, 14, 14, 14, 14, 13, 13, 13, 13, 13, 11, 11, 11, 11, 11, 10, 10, 10, 10, 10, 8, 6, 4, 2, 2, 2, 2, 2, 2,
      2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 2, 2, 1, 1, 1, 1, 1, 0},
     {13, 11, 11, 11, 11, 11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 5, 5, 5, 5, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1,
      1,  1,  1,  1,  1,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
     {15, 13, 13, 13, 13, 13, 11, 11, 11, 11, 11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 4, 2, 2, 2, 2, 2, 2, 2, 2,
      2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 0},
     {17, 15, 15, 15, 15, 15, 13, 13, 13, 13, 13, 11, 11, 11, 11, 11, 9, 9, 9, 9, 9, 7, 5, 3, 3, 3, 3, 3, 3, 3, 3,
      3,  3,  3,  3,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 0},
     {14, 12, 12, 12, 12, 12, 10, 10, 10, 10, 10, 8, 8, 8, 8, 8, 6, 6, 6, 6, 6, 4, 3, 2, 2, 2, 2, 2, 2, 2, 2,
      2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 0}},
    {{11, 10, 10, 10, 9, 9, 9, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1,
      1,  1,  1,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
     {16, 14, 14, 14, 13, 13, 13, 11, 11, 11, 10, 10, 10, 9, 9, 9, 8, 8, 8, 6, 6, 5, 4, 3, 3, 3, 3, 3, 3, 3,
      3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 1, 1, 1, 1, 1, 0},
     {15, 13, 13, 13, 12, 12, 12, 10, 10, 10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 5, 5, 4, 3, 2, 2, 2, 2, 2, 2, 2, 2,
      2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 0},
     {18, 16, 16, 16, 15, 15, 15, 13, 13, 13, 12, 12, 12, 11, 11, 11, 10, 10, 10, 8, 8, 7, 6, 5, 4, 4, 4, 4, 4, 4, 4,
      4,  4,  4,  4,  4,  4,  4,  4,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3, 3, 3, 2, 1, 1, 1, 1, 1, 1, 0},
     {17, 15, 15, 15, 14, 14, 14, 12, 12, 12, 11, 11, 11, 10, 10, 10, 9, 9, 9, 7, 7, 6, 5, 4, 3, 3, 3, 3, 3, 3, 3,
      3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3, 3, 3, 3, 3, 2, 2, 1, 1, 1, 1, 1, 1, 0}},
    {{15, 13, 13, 13, 13, 12, 12, 12, 12, 11, 11, 11, 11, 10, 10, 10, 10, 9, 9, 9, 9, 8, 7, 6, 5, 5, 5, 5, 5, 5, 5,
      5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5, 5, 5, 5, 5, 5, 1, 1, 1, 1, 1, 1, 0},
     {16, 14, 14, 14, 14, 12, 12, 12, 12, 10, 10, 10, 10, 8, 8, 8, 8, 6, 6, 6, 6, 4, 3, 2, 2, 2, 2, 2, 2, 2, 2,
      2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 0},
     {14, 12, 12, 12, 12, 11, 11, 11, 11, 10, 10, 10, 10, 9, 9, 9, 9, 8, 8, 8, 8, 7, 5, 3, 3, 3, 3, 3, 3, 3, 3,
      3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 0},
     {18, 16, 16, 16, 16, 15, 15, 15, 15, 14, 14, 14, 14, 13, 13, 13, 13, 12, 12, 12, 12, 11, 9, 5, 5, 5, 5, 5, 5, 5, 5,
      5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5, 1, 1, 1, 1, 1, 1, 0},
     {17, 15, 15, 15, 15, 13, 13, 13, 13, 11, 11, 11, 11, 9, 9, 9, 9, 7, 7, 7, 7, 5, 3, 1, 1, 1, 1, 1, 1, 1, 1,
      1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0}},
    {{16, 14, 14, 13, 13, 11, 11, 10, 10, 8, 8, 7, 7, 5, 5, 4, 4, 3, 3, 3, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1,
      1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
     {18, 16, 16, 15, 15, 13, 13, 12, 12, 10, 10, 9, 9, 7, 7, 6, 6, 5, 5, 5, 5, 4, 3, 2, 2, 2, 2, 2, 2, 2, 2,
      2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 0},
     {17, 15, 15, 14, 14, 12, 12, 11, 11, 9, 9, 8, 8, 6, 6, 5, 5, 4, 4, 4, 4, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1,
      1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
     {20, 17, 17, 16, 16, 13, 13, 12, 12, 9, 9, 8, 8, 5, 5, 4, 4, 4, 4, 4, 4, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1,
      1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
     {19, 17, 17, 16, 16, 14, 14, 13, 13, 11, 11, 10, 10, 8, 8, 7, 7, 6, 6, 6, 6, 4, 2, 1, 1, 1, 1, 1, 1, 1, 1,
      1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0}},
    {{16, 14, 14, 14, 14, 14, 13, 13, 13, 13, 13, 11, 11, 11, 11, 11, 10, 10, 10, 10, 10, 8, 6, 4, 2, 2, 2, 2, 2, 2,
      2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 2, 2, 1, 1, 1, 1, 1, 0},
     {13, 11, 11, 11, 11, 11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 5, 5, 5, 5, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1,
      1,  1,  1,  1,  1,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
     {15, 13, 13, 13, 13, 13, 11, 11, 11, 11, 11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 4, 2, 2, 2, 2, 2, 2, 2, 2,
      2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 0},
     {17, 15, 15, 15, 15, 15, 13, 13, 13, 13, 13, 11, 11, 11, 11, 11, 9, 9, 9, 9, 9, 7, 5, 3, 3, 3, 3, 3, 3, 3, 3,
      3,  3,  3,  3,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 0},
     {14, 12, 12, 12, 12, 12, 10, 10, 10, 10, 10, 8, 8, 8, 8, 8, 6, 6, 6, 6, 6, 4, 3, 2, 2, 2, 2, 2, 2, 2, 2,
      2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 0}},
    {{11, 10, 10, 10, 9, 9, 9, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1,
      1,  1,  1,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
     {16, 14, 14, 14, 13, 13, 13, 11, 11, 11, 10, 10, 10, 9, 9, 9, 8, 8, 8, 6, 6, 5, 4, 3, 3, 3, 3, 3, 3, 3,
      3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 1, 1, 1, 1, 1, 0},
     {15, 13, 13, 13, 12, 12, 12, 10, 10, 10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 5, 5, 4, 3, 2, 2, 2, 2, 2, 2, 2, 2,
      2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 0},
     {18, 16, 16, 16, 15, 15, 15, 13, 13, 13, 12, 12, 12, 11, 11, 11, 10, 10, 10, 8, 8, 7, 6, 5, 4, 4, 4, 4, 4, 4, 4,
      4,  4,  4,  4,  4,  4,  4,  4,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3, 3, 3, 2, 1, 1, 1, 1, 1, 1, 0},
     {17, 15, 15, 15, 14, 14, 14, 12, 12, 12, 11, 11, 11, 10, 10, 10, 9, 9, 9, 7, 7, 6, 5, 4, 3, 3, 3, 3, 3, 3, 3,
      3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3, 3, 3, 3, 3, 2, 2, 1, 1, 1, 1, 1, 1, 0}}};

int GetSpellByName(const char *name)
{
    /*
     * for now it must be a linear search... later we might make a btree or
     * a hash table..
     */
    int i = 0;

    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, VNULL(name));

    if (!name || !*name)
        return -1;
    for (i = 0; i < MAX_SKILLS; i++)
        if (spell_info[i].castable && spell_info[i].spell_pointer)
            if (is_abbrev(name, spell_info[i].name))
                return i;
    return -1;
}

int GetSkillByName(const char *name)
{
    /*
     * for now it must be a linear search... later we might make a btree or
     * a hash table..
     */
    int i = 0;

    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, VNULL(name));

    if (!name || !*name)
        return -1;
    for (i = 0; i < MAX_SKILLS; i++)
        if (spell_info[i].useable)
            if (is_abbrev(name, spell_info[i].name))
                return i;
    return -1;
}

int SPELL_LEVEL(struct char_data *ch, int sn)
{
    int i = 0;
    int lowest = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), sn);

    lowest = ABS_MAX_LVL;
    for (i = 0; i < ABS_MAX_CLASS; i++)
        if (HasClass(ch, 1 << i))
            lowest = MIN(lowest, spell_info[sn].min_level[i]);
    return lowest;
}

int SKILL_LEVEL(struct char_data *ch, int sn)
{
    return SPELL_LEVEL(ch, sn);
}

int CanCast(struct char_data *ch, int sn)
{
    int i = 0;
    int lowest = 0;
    int lowclass = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), sn);

    if (!ch || sn < 0 || sn >= MAX_SKILLS)
        return FALSE;
    lowest = ABS_MAX_LVL;
    lowclass = -1;
    for (i = 0; i < ABS_MAX_CLASS; i++)
    {
        if (!HasClass(ch, 1 << i) || !spell_info[sn].castable || !spell_info[sn].spell_pointer)
            continue;
        if ((int)GET_LEVEL(ch, i) >= spell_info[sn].min_level[i])
            if (spell_info[sn].min_level[i] < lowest)
            {
                lowest = spell_info[sn].min_level[i];
                lowclass = i;
            }
    }
    return (lowclass > -1) ? 1 : 0;
}

int CanCastClass(struct char_data *ch, int sn, int cl)
{
    if (DEBUG > 2)
        log_info("called %s with %s, %d, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), sn, cl);

    if (!ch || sn < 0 || sn >= MAX_SKILLS)
        return FALSE;
    return (HasClass(ch, 1 << cl) && spell_info[sn].castable && spell_info[sn].spell_pointer &&
            (int)GET_LEVEL(ch, cl) >= spell_info[sn].min_level[cl]);
}

int CanUse(struct char_data *ch, int sn)
{
    int i = 0;
    int lowest = 0;
    int lowclass = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), sn);

    if (!ch || sn < 0 || sn >= MAX_SKILLS)
        return FALSE;
    lowest = ABS_MAX_LVL;
    lowclass = -1;
    for (i = 0; i < ABS_MAX_CLASS; i++)
    {
        if (!HasClass(ch, 1 << i) || !spell_info[sn].useable)
            continue;
        if ((int)GET_LEVEL(ch, i) >= spell_info[sn].min_level[i])
            if (spell_info[sn].min_level[i] < lowest)
            {
                lowest = spell_info[sn].min_level[i];
                lowclass = i;
            }
    }
    return (lowclass > -1) ? 1 : 0;
}

int CanUseClass(struct char_data *ch, int sn, int cl)
{
    if (DEBUG > 2)
        log_info("called %s with %s, %d, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), sn, cl);

    if (!ch || sn < 0 || sn >= MAX_SKILLS)
        return FALSE;
    return (HasClass(ch, 1 << cl) && spell_info[sn].useable && (int)GET_LEVEL(ch, cl) >= spell_info[sn].min_level[cl]);
}

void affect_update(void)
{
    static struct affected_type *af;
    static struct affected_type *next_af_dude;
    static struct char_data *i;

    if (DEBUG > 2)
        log_info("called %s with no arguments", __PRETTY_FUNCTION__);

    for (i = character_list; i; i = i->next)
        for (af = i->affected; af; af = next_af_dude)
        {
            next_af_dude = af->next;
            if (af->duration >= 1)
            {
                af->duration--;
                if (af->duration == 0)
                {
                    if (*spell_wear_off_soon_msg[af->type])
                    {
                        cprintf(i, "%s\r\n", spell_wear_off_soon_msg[af->type]);
                    }
                }
            }
            else
            {
                if ((af->type > 0) && (af->type <= MAX_SKILLS /* MAX_EXIST_SPELL */))
                {
                    if (!af->next || (af->next->type != af->type) || (af->next->duration > 0))
                    {
                        if (*spell_wear_off_msg[af->type])
                        {
                            cprintf(i, "%s\r\n", spell_wear_off_msg[af->type]);

                            /*
                             * check to see if the exit down is connected, if so make the person
                             * fall down into that room and take 1d6 damage
                             */

                            affect_remove(i, af);
                            return;
                        }
                    }
                }
                else if (af->type >= FIRST_BREATH_WEAPON && af->type <= LAST_BREATH_WEAPON)
                {
                    bweapons[af->type - FIRST_BREATH_WEAPON](-af->modifier / 2, i, "", SPELL_TYPE_SPELL, i, 0);
                    if (!i->affected)
                        /*
                         * oops, you're dead :)
                         */
                        break;
                }
                affect_remove(i, af);
            }
        }
}

void clone_char(struct char_data *ch)
{
    struct char_data *clone = NULL;
    struct affected_type *af = NULL;
    int i = 0;

    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

    CREATE(clone, struct char_data, 1);

    clear_char(clone); /* Clear EVERYTHING! (ASSUMES CORRECT) */

    clone->player = ch->player;
    clone->abilities = ch->abilities;

    for (i = 0; i < 5; i++)
        clone->specials.apply_saving_throw[i] = ch->specials.apply_saving_throw[i];

    for (af = ch->affected; af; af = af->next)
        affect_to_char(clone, af);

    for (i = 0; i < 3; i++)
        GET_COND(clone, i) = GET_COND(ch, i);

    clone->points = ch->points;

    for (i = 0; i < MAX_SKILLS; i++)
        clone->skills[i] = ch->skills[i];

    clone->specials = ch->specials;
    clone->specials.fighting = 0;

    GET_NAME(clone) = strdup(GET_NAME(ch));

    clone->player.short_descr = strdup(ch->player.short_descr);

    clone->player.long_descr = strdup(ch->player.long_descr);

    clone->player.description = 0;
    /*
     * REMEMBER EXTRA DESCRIPTIONS
     */

    GET_TITLE(clone) = strdup(GET_TITLE(ch));

    clone->nr = ch->nr;

    if (IS_NPC(clone))
        mob_index[clone->nr].number++;
    else
    { /* Make PC's into NPC's */
        clone->nr = -1;
        SET_BIT(clone->specials.act, ACT_ISNPC);
    }

    clone->desc = 0;
    clone->followers = 0;
    clone->master = 0;

    clone->next = character_list;
    character_list = clone;

    char_to_room(clone, ch->in_room);
}

void clone_obj(struct obj_data *obj)
{
    struct obj_data *clone = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_ONAME(obj));

    CREATE(clone, struct obj_data, 1);

    *clone = *obj;

    clone->name = strdup(obj->name);
    clone->description = strdup(obj->description);
    clone->short_description = strdup(obj->short_description);
    clone->action_description = strdup(obj->action_description);
    clone->ex_description = 0;

    /*
     * REMEMBER EXTRA DESCRIPTIONS
     */
    clone->carried_by = 0;
    clone->equipped_by = 0;
    clone->in_obj = 0;
    clone->contains = 0;
    clone->next_content = 0;
    clone->next = 0;

    /*
     * VIRKER IKKE ENDNU
     */
}

/* Check if making CH follow VICTIM will create an illegal */
/* Follow "Loop/circle"                                    */
char circle_follow(struct char_data *ch, struct char_data *victim)
{
    struct char_data *k = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), SAFE_NAME(victim));

    for (k = victim; k; k = k->master)
    {
        if (k == ch)
            return TRUE;
    }

    return FALSE;
}

/* Called when stop following persons, or stopping charm */
/* This will NOT do if a character quits/dies!!          */
void stop_follower(struct char_data *ch)
{
    struct follow_type *j = NULL;
    struct follow_type *k = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

    if (!ch->master)
        return;

    if (IS_AFFECTED(ch, AFF_CHARM))
    {
        act("You realize that $N is a jerk!", FALSE, ch, 0, ch->master, TO_CHAR);
        act("$n realizes that $N is a jerk!", FALSE, ch, 0, ch->master, TO_NOTVICT);
        act("$n hates your guts!", FALSE, ch, 0, ch->master, TO_VICT);
        if (affected_by_spell(ch, SPELL_CHARM_PERSON))
            affect_from_char(ch, SPELL_CHARM_PERSON);
    }
    else
    {
        act("You stop following $N.", FALSE, ch, 0, ch->master, TO_CHAR);
        if (!IS_SET(ch->specials.act, PLR_STEALTH))
        {
            act("$n stops following $N.", FALSE, ch, 0, ch->master, TO_NOTVICT);
            act("$n stops following you.", FALSE, ch, 0, ch->master, TO_VICT);
        }
    }

    if (ch->master->followers->follower == ch)
    { /* Head of follower-list? */
        k = ch->master->followers;
        ch->master->followers = k->next;
        DESTROY(k);
    }
    else
    { /* locate follower who is not head of list */

        for (k = ch->master->followers; k->next && k->next->follower != ch; k = k->next)
            ;

        if (k->next)
        {
            j = k->next;
            k->next = j->next;
            DESTROY(j);
        }
    }

    ch->master = 0;
    REMOVE_BIT(ch->specials.affected_by, AFF_CHARM | AFF_GROUP);
}

/* Called when a character that follows/is followed dies */
void die_follower(struct char_data *ch)
{
    struct follow_type *j = NULL;
    struct follow_type *k = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

    if (ch->master)
        stop_follower(ch);

    for (k = ch->followers; k; k = j)
    {
        j = k->next;
        stop_follower(k->follower);
    }
}

/* Do NOT call this before having checked if a circle of followers */
/* will arise. CH will follow leader                               */
void add_follower(struct char_data *ch, struct char_data *leader)
{
    struct follow_type *k = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), SAFE_NAME(leader));

    if (ch->master)
        return;

    ch->master = leader;

    CREATE(k, struct follow_type, 1);

    k->follower = ch;
    k->next = leader->followers;
    leader->followers = k;

    act("You now follow $N.", FALSE, ch, 0, leader, TO_CHAR);
    if (!IS_SET(ch->specials.act, PLR_STEALTH))
    {
        act("$n starts following you.", TRUE, ch, 0, leader, TO_VICT);
        act("$n now follows $N.", TRUE, ch, 0, leader, TO_NOTVICT);
    }
}

void say_spell(struct char_data *ch, int si)
{
    char buf[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    char splwd[MAX_BUF_LENGTH] = "\0\0\0\0\0\0\0";
    int j = 0;
    int offs = 0;
    struct char_data *temp_char = NULL;

    struct syllable
    {
        char original[10];
        char replacement[10];
    };

    struct syllable syls[] = {
        {" ", " "},       {"ar", "abra"},   {"au", "mela"},    {"bless", "kato"}, {"blind", "nose"},    {"bur", "mosa"},
        {"cu", "judi"},   {"ca", "xydo"},   {"de", "oculo"},   {"en", "fido"},    {"light", "suffers"}, {"lo", "hi"},
        {"mor", "sido"},  {"move", "zak"},  {"ness", "lacri"}, {"ning", "illa"},  {"per", "duda"},      {"ra", "gru"},
        {"re", "candus"}, {"son", "sabru"}, {"se", "or"},      {"tect", "cula"},  {"tri", "infa"},      {"ven", "nofo"},
        {"a", "a"},       {"b", "b"},       {"c", "q"},        {"d", "e"},        {"e", "z"},           {"f", "y"},
        {"g", "o"},       {"h", "p"},       {"i", "u"},        {"j", "y"},        {"k", "t"},           {"l", "s"},
        {"m", "w"},       {"n", "i"},       {"o", "a"},        {"p", "t"},        {"q", "d"},           {"r", "f"},
        {"s", "g"},       {"t", "h"},       {"u", "j"},        {"v", "z"},        {"w", "x"},           {"x", "b"},
        {"y", "l"},       {"z", "k"},       {"", ""}};

    if (DEBUG > 2)
        log_info("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), si);

    strcpy(buf, "");
    strcpy(splwd, spell_info[si].name);

    offs = 0;

    while (*(splwd + offs))
    {
        for (j = 0; *(syls[j].original); j++)
            if (strncmp(syls[j].original, splwd + offs, strlen(syls[j].original)) == 0)
            {
                strcat(buf, syls[j].replacement);
                if (strlen(syls[j].original))
                    offs += strlen(syls[j].original);
                else
                    ++offs;
            }
    }

    for (temp_char = real_roomp(ch->in_room)->people; temp_char; temp_char = temp_char->next_in_room)
        if (temp_char != ch)
        {
            if (CanCast(temp_char, si - 1))
                act("$n utters the words, '%s'", FALSE, ch, 0, temp_char, TO_VICT, spell_info[si].name);
            else
                act("$n utters the words, '%s'", FALSE, ch, 0, temp_char, TO_VICT, buf);
        }
}

char saves_spell(struct char_data *ch, short int save_type)
{
    int save = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %hd", __PRETTY_FUNCTION__, SAFE_NAME(ch), save_type);

    if (save_type < 0 || save_type > 4)
    {
        save_type = 4;
    }

    /*
     * Negative apply_saving_throw makes saving throw better!
     */

    save = ch->specials.apply_saving_throw[save_type];

    if (IS_PC(ch))
    {
        /*
         * **  Remove-For-Multi-Class
         */
        save += saving_throws[BestMagicClass(ch)][save_type][(int)GET_LEVEL(ch, BestMagicClass(ch))];
        if (GetMaxLevel(ch) > MAX_MORT)
            return TRUE;
    }
    return (MAX(1, save) < number(1, 20));
}

char ImpSaveSpell(struct char_data *ch, short int save_type, int mod)
{
    int save = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %hd, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), save_type, mod);

    if (save_type < 0 || save_type > 4)
    {
        save_type = 4;
    }

    /*
     * Positive mod is better for save
     */

    /*
     * Negative apply_saving_throw makes saving throw better!
     */

    save = ch->specials.apply_saving_throw[save_type] - mod;

    if (IS_PC(ch))
    {
        /*
         * **  Remove-For-Multi-Class
         */
        save += saving_throws[BestMagicClass(ch)][save_type][(int)GET_LEVEL(ch, BestMagicClass(ch))];
        /*
         *  if (GetMaxLevel(ch) >= LOW_IMMORTAL)
         *    return TRUE;
         */
    }
    return (MAX(1, save) < number(1, 20));
}

const char *skip_spaces(const char *str)
{
    if (DEBUG > 3)
        log_info("called %s with %s", __PRETTY_FUNCTION__, VNULL(str));

    for (; *str && (*str) == ' '; str++)
        ;

    return str;
}

/* Assumes that *argument does start with first letter of chopped string */

int do_cast(struct char_data *ch, const char *argument, int cmd)
{
    struct room_data *rp = NULL;
    struct obj_data *tar_obj = NULL;
    struct char_data *tar_char = NULL;
    char name[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    int qend = 0;
    int spl = 0;
    int i = 0;
    int target_ok = FALSE;
    char spell_name[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    /*
     * if (IS_NPC(ch) && (IS_NOT_SET(ch->specials.act, ACT_POLYSELF))) return TRUE;
     */

    if (!IsHumanoid(ch) && GET_RACE(ch) != RACE_DRAGON)
    {
        cprintf(ch, "You try to form the words, but you can only growl.\r\n");
        return TRUE;
    }
    if (IsNonMagical(ch))
    {
        cprintf(ch, "Maybe you should leave the hocus pocus to the bookworms, eh?\r\n");
        return TRUE;
    }
    rp = real_roomp(ch->in_room);
    if (IS_SET(rp->room_flags, NO_MAGIC) && !IS_IMMORTAL(ch))
    {
        cprintf(ch, "Your mystical power seems feeble and useless here.\r\n");
        return TRUE;
    }
    argument = skip_spaces(argument);

    if (!(*argument))
    {
        cprintf(ch, "cast 'spell name' <target>\r\n");
        return TRUE;
    }
    if (*argument != '\'')
    {
        cprintf(ch, "Magic must always be enclosed by single quotes: '\r\n");
        return TRUE;
    }
    for (qend = 1; *(argument + qend) && (*(argument + qend) != '\''); qend++)
        ;
    /*
     * No need for this, is_abbrev() already compares without case.
     * *(argument + qend) = LOWER(*(argument + qend));
     */

    if (*(argument + qend) != '\'')
    {
        cprintf(ch, "Magic must always be enclosed by single quotes: '\r\n");
        return TRUE;
    }
    /*
     * spl = old_search_block(argument, 1, qend - 1, spells, 0);
     */
    bzero(spell_name, MAX_INPUT_LENGTH);
    strncpy(spell_name, argument + 1, qend - 1);
    if (!strlen(spell_name))
    {
        cprintf(ch, "You successfully cast Nothing!\r\n");
        return TRUE;
    }
    if ((spl = GetSpellByName(spell_name)) < 0)
    {
        cprintf(ch, "You reconsider your attempt to summon the demon %s!\r\n", spell_name);
        return TRUE;
    }
    if (CanCast(ch, spl))
    {
        if (GET_POS(ch) < spell_info[spl].minimum_position)
        {
            switch (GET_POS(ch))
            {
            case POSITION_SLEEPING:
                cprintf(ch, "You dream about your great magical powers.\r\n");
                break;
            case POSITION_RESTING:
                cprintf(ch, "You lazily think about how wonderful magic is.\r\n");
                break;
            case POSITION_SITTING:
                cprintf(ch, "You start to incant and then your butt falls asleep!\r\n");
                break;
            case POSITION_FIGHTING:
                cprintf(ch, "You start to incant and your book is knocked away!.\r\n");
                break;
            default:
                cprintf(ch, "If only you had thought of that earlier!\r\n");
                break;
            }
        }
        else
        {
            argument += qend + 1; /* Point to the last ' */
            for (; *argument == ' '; argument++)
                ;

            /*
             **************** Locate targets **************** */

            target_ok = FALSE;
            tar_char = 0;
            tar_obj = 0;

            if (!(rp = real_roomp(ch->in_room)))
                if (IS_SET(spell_info[spl].targets, TAR_VIOLENT) &&
                    check_peaceful(ch, "You cannot seem to focus your hatred."))
                    return TRUE;

            if (IS_NOT_SET(spell_info[spl].targets, TAR_IGNORE))
            {
                argument = one_argument(argument, name);
                if (*name)
                {
                    if (IS_SET(spell_info[spl].targets, TAR_CHAR_ROOM))
                    {
                        if ((tar_char = get_char_room_vis(ch, name)))
                        {
                            if (tar_char == ch || tar_char == ch->specials.fighting ||
                                tar_char->attackers < MAX_ATTACKERS || tar_char->specials.fighting == ch)
                                target_ok = TRUE;
                            else
                            {
                                cprintf(ch, "You wish everyone would stop for a moment...\r\n");
                                return TRUE;
                            }
                        }
                    }
                    if (!target_ok && IS_SET(spell_info[spl].targets, TAR_CHAR_WORLD))
                        if ((tar_char = get_char_vis(ch, name)))
                            target_ok = TRUE;

                    if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_INV))
                        if ((tar_obj = get_obj_in_list_vis(ch, name, ch->carrying)))
                            target_ok = TRUE;

                    if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_ROOM))
                        if ((tar_obj = get_obj_in_list_vis(ch, name, real_roomp(ch->in_room)->contents)))
                            target_ok = TRUE;

                    if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_WORLD))
                        if ((tar_obj = get_obj_vis(ch, name)))
                            target_ok = TRUE;

                    if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_EQUIP))
                    {
                        for (i = 0; i < MAX_WEAR && !target_ok; i++)
                            if (ch->equipment[i] && str_cmp(name, ch->equipment[i]->name) == 0)
                            {
                                tar_obj = ch->equipment[i];
                                target_ok = TRUE;
                            }
                    }
                    if (!target_ok && IS_SET(spell_info[spl].targets, TAR_SELF_ONLY))
                        if (str_cmp(GET_NAME(ch), name) == 0)
                        {
                            tar_char = ch;
                            target_ok = TRUE;
                        }
                    if (!target_ok && IS_SET(spell_info[spl].targets, TAR_NAME))
                    {
                        tar_obj = (struct obj_data *)name;
                        target_ok = TRUE;
                    }
                    if (tar_char)
                    {
                        if (IS_NPC(tar_char))
                            if (IS_SET(tar_char->specials.act, ACT_IMMORTAL))
                            {
                                cprintf(ch, "You can't cast magic on that!");
                                return TRUE;
                            }
                    }
                }
                else
                { /* No argument was typed */
                    if (IS_SET(spell_info[spl].targets, TAR_FIGHT_SELF))
                        if (ch->specials.fighting)
                        {
                            tar_char = ch;
                            target_ok = TRUE;
                        }
                    if (!target_ok && IS_SET(spell_info[spl].targets, TAR_FIGHT_VICT))
                        if (ch->specials.fighting)
                        {
                            /*
                             * WARNING, MAKE INTO POINTER
                             */
                            tar_char = ch->specials.fighting;
                            target_ok = TRUE;
                        }
                    if (!target_ok && IS_SET(spell_info[spl].targets, TAR_SELF_ONLY))
                    {
                        tar_char = ch;
                        target_ok = TRUE;
                    }
                }
            }
            else
            {
                target_ok = TRUE; /* No target, is a good target */
            }

            if (!target_ok)
            {
                if (*name)
                {
                    if (IS_SET(spell_info[spl].targets, TAR_CHAR_ROOM))
                        cprintf(ch, "I have no clue where %s might be.\r\n", name);
                    else if (IS_SET(spell_info[spl].targets, TAR_CHAR_WORLD))
                        cprintf(ch, "I don't think %s is playing now.\r\n", name);
                    else if (IS_SET(spell_info[spl].targets, TAR_OBJ_INV))
                        cprintf(ch, "You have a %s?  Where is it???\r\n", name);
                    else if (IS_SET(spell_info[spl].targets, TAR_OBJ_ROOM))
                        cprintf(ch, "I don't see any %s here...\r\n", name);
                    else if (IS_SET(spell_info[spl].targets, TAR_OBJ_WORLD))
                        cprintf(ch, "Sorry, I can't find any %s.\r\n", name);
                    else if (IS_SET(spell_info[spl].targets, TAR_OBJ_EQUIP))
                        cprintf(ch, "You wish you were wearing a %s...\r\n", name);
                }
                else
                { /* Nothing was given as argument */
                    if (spell_info[spl].targets < TAR_OBJ_INV)
                        cprintf(ch, "And just who deserves a %s?\r\n", spell_info[spl].name);
                    else
                        cprintf(ch, "What would you like to cast %s at?\r\n", spell_info[spl].name);
                }
                return TRUE;
            }
            else
            { /* TARGET IS OK */

                if ((tar_char == ch) && IS_SET(spell_info[spl].targets, TAR_SELF_NONO))
                {
                    cprintf(ch, "You can't cast %s on yourself!\r\n", spell_info[spl].name);
                    return TRUE;
                }
                else if ((tar_char != ch) && IS_SET(spell_info[spl].targets, TAR_SELF_ONLY))
                {
                    cprintf(ch, "Only you are worthy of the %s spell.\r\n", spell_info[spl].name);
                    return TRUE;
                }
                else if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == tar_char))
                {
                    cprintf(ch, "No!  Casting %s might harm your beloved master!\r\n", spell_info[spl].name);
                    return TRUE;
                }
            }

            /*
             * if (GetMaxLevel(ch) < LOW_IMMORTAL)
             */
            if (GET_MANA(ch) < USE_MANA(ch, spl))
            {
                cprintf(ch, "You mutter and wave your hands tiredly as the spell fails.\r\n");
                return TRUE;
            }
            if (spl != SPELL_VENTRILOQUATE) /* :-) */
                say_spell(ch, spl);

            if (IS_MORTAL(ch))
                WAIT_STATE(ch, spell_info[spl].delay);

            if ((spell_info[spl].spell_pointer == 0) && spl > 0)
                cprintf(ch, "Sorry, this magic has not yet been implemented\r\n");
            else
            {
                if (number(1, 100) > (ch->skills[spl].learned + (GetMaxLevel(ch) / 5)))
                { /* 101% is failure */
                    random_miscast(ch, spell_info[spl].name);
                    if (ch->skills[SKILL_SPELLCRAFT].learned > number(1, 101))
                        if (ch->skills[SKILL_MEDITATION].learned > number(1, 101))
                            GET_MANA(ch) -= MAX(1, (USE_MANA(ch, spl) / 4));
                        else
                            GET_MANA(ch) -= MAX(1, (USE_MANA(ch, spl) / 3));
                    else if (ch->skills[SKILL_MEDITATION].learned > number(1, 101))
                        GET_MANA(ch) -= MAX(1, (USE_MANA(ch, spl) / 3));
                    else
                        GET_MANA(ch) -= MAX(1, (USE_MANA(ch, spl) / 2));
                    return TRUE;
                }
                cprintf(ch, "You mutter and wave and suddenly Cast %s!\r\n", spell_info[spl].name);
                if (ch->skills[spl].learned < 60)
                {
                    if (ch->skills[SKILL_SPELLCRAFT].learned > number(1, 101))
                        ch->skills[spl].learned += 5;
                    else
                        ch->skills[spl].learned += 2;
                }
                ((*spell_info[spl].spell_pointer)((int)GET_LEVEL(ch, BestMagicClass(ch)), ch, argument,
                                                  SPELL_TYPE_SPELL, tar_char, tar_obj));
                GET_MANA(ch) -= (USE_MANA(ch, spl));
            }
        } /* if GET_POS < min_pos */
        return TRUE;
    }
    random_magic_failure(ch);
    return TRUE;
}

void assign_spell_pointers(void)
{
    /*
     * castable means the cast command can find and use it... it is thus a spell.
     * useable means it is a skill that can be applied, such as kick.
     * if it is neither, then it is for internal game use such as TYPE_SUFFERING.
     * Actually, useable more properly means it is a skill that can be practiced.
     */
    int i = 0;
    int j = 0;

    if (DEBUG > 1)
        log_info("called %s with no arguments", __PRETTY_FUNCTION__);

    for (i = 0; i < MAX_SKILLS; i++)
    {
        spell_info[i].castable = spell_info[i].useable = 0;
        spell_info[i].name = "";
        spell_info[i].spell_pointer = 0;
        spell_info[i].min_mana = spell_info[i].max_mana = 100;
        spell_info[i].targets = TAR_IGNORE;
        spell_info[i].minimum_position = POSITION_STANDING;
        spell_info[i].generic_level = LOKI;
        spell_info[i].generic_classes = CLASS_ALL;
        for (j = 0; j < ABS_MAX_CLASS; j++)
            spell_info[i].min_level[j] = LOKI;
    }

    ASSIGN_SPELL(SKILL_APRAISE, 0, 1, "appraise", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, 2, CLASS_ALL, LOKI,
                 LOKI, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SKILL_BACKSTAB, 0, 1, "backstab", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL,
                 LOKI, LOKI, LOKI, 1, LOKI, LOKI);
    ASSIGN_SPELL(SKILL_BANDAGE, 0, 1, "bandage", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, 1, CLASS_ALL, LOKI,
                 LOKI, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SKILL_BARE_HAND, 0, 1, "barehand", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, 1, CLASS_ALL,
                 LOKI, LOKI, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SKILL_BARTER, 0, 0, "barter", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI,
                 LOKI, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SKILL_BASH, 0, 1, "bash", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI,
                 LOKI, 1, LOKI, 1, LOKI);
    ASSIGN_SPELL(SKILL_BASH_W_SHIELD, 0, 0, "shield bash", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI,
                 CLASS_ALL, LOKI, LOKI, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SKILL_BLIND_FIGHTING, 0, 0, "blind fighting", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI,
                 CLASS_ALL, LOKI, LOKI, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SKILL_BREW, 0, 0, "brewing", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI,
                 LOKI, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SKILL_DETECT_NOISE, 0, 0, "hear noise", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI,
                 CLASS_ALL, LOKI, LOKI, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SKILL_DISARM, 0, 1, "disarm", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI,
                 LOKI, 1, LOKI, 1, LOKI);
    ASSIGN_SPELL(SKILL_DISARM_TRAP, 0, 0, "disarm trap", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI,
                 CLASS_ALL, LOKI, LOKI, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SKILL_DODGE, 0, 0, "dodge", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI,
                 LOKI, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SKILL_DOOR_BASH, 0, 1, "doorbash", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL,
                 LOKI, LOKI, 1, LOKI, 1, LOKI);
    ASSIGN_SPELL(SKILL_ENDURANCE, 0, 1, "endurance", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, 3, CLASS_ALL,
                 LOKI, LOKI, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SKILL_FIND_TRAP, 0, 1, "locate trap", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI,
                 CLASS_ALL, LOKI, LOKI, LOKI, 1, LOKI, LOKI);
    ASSIGN_SPELL(SKILL_HIDE, 0, 1, "hide", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI,
                 LOKI, LOKI, 1, 1, LOKI);
    ASSIGN_SPELL(SKILL_KICK, 0, 1, "kick", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI,
                 LOKI, 1, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SKILL_KNOCK_OUT, 0, 0, "knockout", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL,
                 LOKI, LOKI, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SKILL_MEDITATION, 0, 1, "meditation", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, 7, CLASS_ALL,
                 LOKI, LOKI, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SKILL_PARRY, 0, 0, "parry", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI,
                 LOKI, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SKILL_PEER, 0, 1, "peer", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI,
                 LOKI, LOKI, 1, 1, LOKI);
    ASSIGN_SPELL(SKILL_PICK_LOCK, 0, 1, "pick lock", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL,
                 LOKI, LOKI, LOKI, 1, LOKI, LOKI);
    ASSIGN_SPELL(SKILL_PUNCH, 0, 1, "punch", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, 2, CLASS_ALL, LOKI, LOKI,
                 1, LOKI, 1, LOKI);
    ASSIGN_SPELL(SKILL_READ_MAGIC, 0, 0, "read magic", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, 3, CLASS_ALL,
                 LOKI, LOKI, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SKILL_RESCUE, 0, 1, "rescue", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI,
                 LOKI, 1, LOKI, 1, LOKI);
    ASSIGN_SPELL(SKILL_RIDE, 0, 1, "riding", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, 1, CLASS_ALL, LOKI, LOKI,
                 LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SKILL_SCRIBE, 0, 0, "scribe", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI,
                 LOKI, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SKILL_SEARCH, 0, 1, "search", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI,
                 LOKI, LOKI, 1, 1, LOKI);
    ASSIGN_SPELL(SKILL_SNEAK, 0, 1, "sneak", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI,
                 LOKI, LOKI, 1, 1, LOKI);
    ASSIGN_SPELL(SKILL_SPEC_BLUDGE, 0, 1, "bludgeon spec", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI,
                 CLASS_ALL, LOKI, LOKI, 1, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SKILL_SPEC_CLEAVE, 0, 1, "cleave spec", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI,
                 CLASS_ALL, LOKI, LOKI, 1, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SKILL_SPEC_CRUSH, 0, 1, "crush spec", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI,
                 CLASS_ALL, LOKI, LOKI, 1, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SKILL_SPEC_PIERCE, 0, 1, "pierce spec", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI,
                 CLASS_ALL, LOKI, LOKI, 1, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SKILL_SPEC_SLASH, 0, 1, "slash spec", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI,
                 CLASS_ALL, LOKI, LOKI, 1, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SKILL_SPEC_SMASH, 0, 1, "smash spec", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI,
                 CLASS_ALL, LOKI, LOKI, 1, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SKILL_SPEC_SMITE, 0, 1, "smite spec", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI,
                 CLASS_ALL, LOKI, LOKI, 1, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SKILL_SPEC_STAB, 0, 1, "stab spec", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL,
                 LOKI, LOKI, 1, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SKILL_SPEC_WHIP, 0, 1, "whip spec", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL,
                 LOKI, LOKI, 1, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SKILL_SPELLCRAFT, 0, 1, "spellcraft", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, 7,
                 CLASS_MAGICAL, LOKI, LOKI, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SKILL_STEAL, 0, 1, "steal", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI,
                 LOKI, LOKI, 1, LOKI, LOKI);
    ASSIGN_SPELL(SKILL_SWIMMING, 0, 1, "swimming", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, 1, CLASS_ALL, LOKI,
                 LOKI, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SKILL_TRACK, 0, 1, "track", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, 3, CLASS_SNEAK, LOKI,
                 LOKI, LOKI, 1, 1, LOKI);
    ASSIGN_SPELL(SKILL_TWO_HANDED, 0, 1, "two handed", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, 5,
                 CLASS_FIGHTER, LOKI, LOKI, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SKILL_TWO_WEAPON, 0, 0, "dual wield", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI,
                 CLASS_ALL, LOKI, LOKI, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_MANA, 0, 0, "MANA", NULL, 12, 100, 100, TAR_IGNORE, POSITION_FIGHTING, LOKI, CLASS_ALL, LOKI,
                 LOKI, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_ACID_BLAST, 1, 0, "acid blast", cast_acid_blast, 24, 15, 50,
                 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, POSITION_FIGHTING, LOKI, CLASS_ALL, 7, LOW_IMMORTAL,
                 LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_AID, 1, 0, "aid", cast_aid, 12, 15, 50, TAR_CHAR_ROOM, POSITION_STANDING, LOKI, CLASS_ALL,
                 LOW_IMMORTAL, 10, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_ANIMATE_DEAD, 1, 0, "animate dead", cast_animate_dead, 24, 15, 50, TAR_OBJ_ROOM,
                 POSITION_STANDING, LOKI, CLASS_ALL, 10, 7, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_ARMOR, 1, 0, "armor", cast_armor, 12, 5, 50, TAR_CHAR_ROOM, POSITION_STANDING, LOKI, CLASS_ALL,
                 4, 1, LOKI, LOKI, 10, LOKI);
    ASSIGN_SPELL(SPELL_ASTRAL_WALK, 1, 0, "astral walk", cast_astral_walk, 12, 33, 50, TAR_CHAR_WORLD,
                 POSITION_STANDING, LOKI, CLASS_ALL, 21, 18, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_BLESS, 1, 0, "bless", cast_bless, 12, 5, 50, TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_CHAR_ROOM,
                 POSITION_STANDING, LOKI, CLASS_ALL, LOW_IMMORTAL, 1, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_BLINDNESS, 1, 0, "blindness", cast_blindness, 24, 5, 50,
                 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, POSITION_FIGHTING, LOKI, CLASS_ALL, 8, 6, LOKI, LOKI,
                 LOKI, LOKI);
    ASSIGN_SPELL(SPELL_BURNING_HANDS, 1, 0, "burning hands", cast_burning_hands, 24, 30, 50, TAR_IGNORE | TAR_VIOLENT,
                 POSITION_FIGHTING, LOKI, CLASS_ALL, 5, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_CACAODEMON, 1, 0, "cacaodemon", cast_cacaodemon, 24, 50, 100, TAR_IGNORE, POSITION_STANDING,
                 LOKI, CLASS_ALL, 30, 30, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_CALL_LIGHTNING, 1, 0, "call lightning", cast_call_lightning, 36, 15, 50,
                 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, POSITION_FIGHTING, LOKI, CLASS_ALL, LOW_IMMORTAL, 15,
                 LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_CALM, 1, 0, "calm", cast_calm, 24, 15, 50, TAR_CHAR_ROOM, POSITION_STANDING, LOKI, CLASS_ALL, 4,
                 2, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_CAUSE_CRITICAL, 1, 0, "cause critical", cast_cause_critic, 18, 11, 50,
                 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, POSITION_FIGHTING, LOKI, CLASS_ALL, LOW_IMMORTAL, 9,
                 LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_CAUSE_LIGHT, 1, 0, "cause light", cast_cause_light, 12, 8, 50, TAR_CHAR_ROOM | TAR_FIGHT_VICT,
                 POSITION_FIGHTING, LOKI, CLASS_ALL, LOW_IMMORTAL, 1, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_CAUSE_SERIOUS, 1, 0, "cause serious", cast_cause_serious, 12, 9, 50, TAR_CHAR_ROOM | TAR_VIOLENT,
                 POSITION_FIGHTING, LOKI, CLASS_ALL, 30, 7, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_CHARM_MONSTER, 1, 0, "charm monster", cast_charm_monster, 18, 5, 50, TAR_CHAR_ROOM | TAR_VIOLENT,
                 POSITION_STANDING, LOKI, CLASS_ALL, 8, 8, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_CHARM_PERSON, 1, 0, "charm person", cast_charm_person, 12, 5, 50,
                 TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_FIGHT_VICT | TAR_VIOLENT, POSITION_STANDING, LOKI, CLASS_ALL, 12,
                 12, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_CHILL_TOUCH, 1, 0, "chill touch", cast_chill_touch, 12, 15, 50,
                 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, POSITION_FIGHTING, LOKI, CLASS_ALL, 3, LOW_IMMORTAL,
                 LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_CLONE, 1, 0, "clone", cast_clone, 48, 50, 100, TAR_CHAR_WORLD, POSITION_STANDING, LOKI,
                 CLASS_ALL, 25, 48, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_COLOR_SPRAY, 1, 0, "color spray", cast_color_spray, 24, 15, 50, TAR_IGNORE | TAR_VIOLENT,
                 POSITION_FIGHTING, LOKI, CLASS_ALL, 11, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_CONE_OF_COLD, 1, 0, "cone of cold", cast_cone_of_cold, 24, 15, 50, TAR_IGNORE | TAR_VIOLENT,
                 POSITION_FIGHTING, LOKI, CLASS_ALL, 11, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_CONJURE_ELEMENTAL, 1, 0, "conjure elemental", cast_conjure_elemental, 24, 30, 100, TAR_IGNORE,
                 POSITION_STANDING, LOKI, CLASS_ALL, 16, 14, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_CONTROL_WEATHER, 1, 0, "control weather", cast_control_weather, 36, 25, 50, TAR_IGNORE,
                 POSITION_STANDING, LOKI, CLASS_ALL, 10, 13, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_CONT_LIGHT, 1, 0, "continual light", cast_cont_light, 24, 10, 50, TAR_IGNORE, POSITION_STANDING,
                 LOKI, CLASS_ALL, 3, 4, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_CREATE_FOOD, 1, 0, "create food", cast_create_food, 12, 5, 50, TAR_IGNORE, POSITION_STANDING,
                 LOKI, CLASS_ALL, LOW_IMMORTAL, 3, LOKI, LOKI, 11, LOKI);
    ASSIGN_SPELL(SPELL_CREATE_WATER, 1, 0, "create water", cast_create_water, 12, 5, 50, TAR_OBJ_INV | TAR_OBJ_EQUIP,
                 POSITION_STANDING, LOKI, CLASS_ALL, LOW_IMMORTAL, 2, LOKI, LOKI, 10, LOKI);
    ASSIGN_SPELL(SPELL_CURE_BLIND, 1, 0, "cure blindness", cast_cure_blind, 12, 5, 50, TAR_CHAR_ROOM, POSITION_STANDING,
                 LOKI, CLASS_ALL, LOW_IMMORTAL, 4, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_CURE_CRITIC, 1, 0, "cure critical", cast_cure_critic, 12, 11, 50, TAR_CHAR_ROOM,
                 POSITION_FIGHTING, LOKI, CLASS_ALL, LOW_IMMORTAL, 9, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_CURE_LIGHT, 1, 0, "cure light", cast_cure_light, 12, 5, 50, TAR_CHAR_ROOM, POSITION_FIGHTING,
                 LOKI, CLASS_ALL, LOW_IMMORTAL, 1, LOKI, LOKI, 10, LOKI);
    ASSIGN_SPELL(SPELL_CURE_SERIOUS, 1, 0, "cure serious", cast_cure_serious, 12, 9, 50, TAR_CHAR_ROOM,
                 POSITION_FIGHTING, LOKI, CLASS_ALL, 30, 7, LOKI, LOKI, 15, LOKI);
    ASSIGN_SPELL(SPELL_CURSE, 1, 0, "curse", cast_curse, 24, 20, 50,
                 TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_FIGHT_VICT | TAR_VIOLENT,
                 POSITION_STANDING, LOKI, CLASS_ALL, 12, 12, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_DETECT_EVIL, 1, 0, "detect evil", cast_detect_evil, 12, 5, 50, TAR_CHAR_ROOM, POSITION_STANDING,
                 LOKI, CLASS_ALL, LOW_IMMORTAL, 1, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_DETECT_INVISIBLE, 1, 0, "detect invisible", cast_detect_invisibility, 12, 5, 50, TAR_CHAR_ROOM,
                 POSITION_STANDING, LOKI, CLASS_ALL, 2, 5, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_DETECT_MAGIC, 1, 0, "detect magic", cast_detect_magic, 12, 5, 50, TAR_CHAR_ROOM,
                 POSITION_STANDING, LOKI, CLASS_ALL, 1, 3, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_DETECT_POISON, 1, 0, "detect poison", cast_detect_poison, 12, 5, 50,
                 TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP, POSITION_STANDING, LOKI, CLASS_ALL, LOW_IMMORTAL, 2, LOKI,
                 LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_DISPEL_EVIL, 1, 0, "dispel evil", cast_dispel_evil, 36, 100, 100, TAR_CHAR_ROOM | TAR_FIGHT_VICT,
                 POSITION_FIGHTING, LOKI, CLASS_ALL, LOW_IMMORTAL, 12, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_DISPEL_GOOD, 1, 0, "dispel good", cast_dispel_good, 36, 100, 100, TAR_CHAR_ROOM | TAR_FIGHT_VICT,
                 POSITION_FIGHTING, LOKI, CLASS_ALL, LOW_IMMORTAL, 12, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_DISPEL_MAGIC, 1, 0, "dispel magic", cast_dispel_magic, 12, 15, 50,
                 TAR_CHAR_ROOM | TAR_FIGHT_VICT, POSITION_FIGHTING, LOKI, CLASS_ALL, 6, 6, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_DRAGON_BREATH, 0, 0, "DRAGON BREATH", cast_dragon_breath, 0, 100, 100, TAR_IGNORE | TAR_VIOLENT,
                 POSITION_STANDING, LOKI, CLASS_ALL, LOKI, LOKI, LOKI, LOKI, LOKI, LOKI);
#if 1
    ASSIGN_SPELL(SPELL_EARTHQUAKE, 1, 0, "earthquake", cast_earthquake, 24, 15, 50, TAR_IGNORE | TAR_VIOLENT,
                 POSITION_FIGHTING, LOKI, CLASS_ALL, LOW_IMMORTAL, 8, LOKI, LOKI, LOKI, LOKI);
#else
    ASSIGN_SPELL(SPELL_EARTHQUAKE, 1, 0, "earthquake", cast_new_earthquake, 24, 15, 50, TAR_IGNORE | TAR_VIOLENT,
                 POSITION_FIGHTING, LOKI, CLASS_ALL, LOW_IMMORTAL, 8, LOKI, LOKI, LOKI, LOKI);
#endif
    ASSIGN_SPELL(SPELL_ENCHANT_WEAPON, 1, 0, "enchant weapon", cast_enchant_weapon, 48, 100, 100,
                 TAR_OBJ_INV | TAR_OBJ_EQUIP, POSITION_STANDING, LOKI, CLASS_ALL, 9, 25, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_ENERGY_DRAIN, 1, 0, "energy drain", cast_energy_drain, 36, 35, 50,
                 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, POSITION_FIGHTING, LOKI, CLASS_ALL, 17, LOW_IMMORTAL,
                 LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_FAERIE_FIRE, 1, 0, "faerie fire", cast_faerie_fire, 12, 10, 50, TAR_CHAR_ROOM | TAR_SELF_NONO,
                 POSITION_STANDING, LOKI, CLASS_ALL, 5, 3, LOKI, LOKI, 11, LOKI);
    ASSIGN_SPELL(SPELL_FAERIE_FOG, 1, 0, "faerie fog", cast_faerie_fog, 24, 20, 50, TAR_IGNORE, POSITION_STANDING, LOKI,
                 CLASS_ALL, 13, 10, LOKI, LOKI, 14, LOKI);
    ASSIGN_SPELL(SPELL_FEAR, 1, 0, "fear", cast_fear, 12, 15, 50, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,
                 POSITION_FIGHTING, LOKI, CLASS_ALL, 8, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_FIREBALL, 1, 0, "fireball", cast_fireball, 36, 15, 50, TAR_IGNORE | TAR_VIOLENT,
                 POSITION_FIGHTING, LOKI, CLASS_ALL, 15, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_FIRESHIELD, 1, 0, "fireshield", cast_fireshield, 24, 40, 50, TAR_SELF_ONLY | TAR_CHAR_ROOM,
                 POSITION_STANDING, LOKI, CLASS_ALL, 20, 19, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_FLAMESTRIKE, 1, 0, "flamestrike", cast_flamestrike, 24, 15, 50,
                 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, POSITION_FIGHTING, LOKI, CLASS_ALL, LOW_IMMORTAL, 11,
                 LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_FLY, 1, 0, "fly", cast_flying, 12, 15, 50, TAR_CHAR_ROOM, POSITION_FIGHTING, LOKI, CLASS_ALL, 3,
                 14, LOKI, LOKI, 18, LOKI);
    ASSIGN_SPELL(SPELL_FLY_GROUP, 1, 0, "group fly", cast_fly_group, 18, 30, 50, TAR_IGNORE, POSITION_FIGHTING, LOKI,
                 CLASS_ALL, 8, 22, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_GOODBERRY, 1, 0, "goodberry", cast_goodberry, 12, 40, 80, TAR_IGNORE, POSITION_STANDING, LOKI,
                 CLASS_ALL, LOKI, 40, LOKI, LOKI, 25, LOKI);
    ASSIGN_SPELL(SPELL_HARM, 1, 0, "harm", cast_harm, 36, 50, 100, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,
                 POSITION_FIGHTING, LOKI, CLASS_ALL, LOW_IMMORTAL, 17, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_HEAL, 1, 0, "heal", cast_heal, 18, 50, 100, TAR_CHAR_ROOM, POSITION_FIGHTING, LOKI, CLASS_ALL,
                 LOW_IMMORTAL, 17, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_ICE_STORM, 1, 0, "ice storm", cast_ice_storm, 12, 15, 50, TAR_IGNORE | TAR_VIOLENT,
                 POSITION_FIGHTING, LOKI, CLASS_ALL, 7, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_IDENTIFY, 0, 0, "IDENTIFY", cast_identify, 1, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI,
                 CLASS_ALL, LOKI, LOKI, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_INFRAVISION, 1, 0, "infravision", cast_infravision, 12, 7, 50, TAR_CHAR_ROOM, POSITION_STANDING,
                 LOKI, CLASS_ALL, 5, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_INVISIBLE, 1, 0, "invisibility", cast_invisibility, 12, 5, 50,
                 TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM | TAR_OBJ_EQUIP, POSITION_STANDING, LOKI, CLASS_ALL, 4,
                 LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_KNOCK, 1, 0, "knock", cast_knock, 12, 10, 50, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, 3,
                 LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_KNOW_ALIGNMENT, 1, 0, "know alignment", cast_know_alignment, 12, 10, 50,
                 TAR_CHAR_ROOM | TAR_FIGHT_VICT, POSITION_FIGHTING, LOKI, CLASS_ALL, 4, 3, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_LIGHT, 1, 0, "create light", cast_light, 12, 5, 50, TAR_IGNORE, POSITION_STANDING, LOKI,
                 CLASS_ALL, 1, 2, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_LIGHTNING_BOLT, 1, 0, "lightning bolt", cast_lightning_bolt, 24, 15, 50,
                 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, POSITION_FIGHTING, LOKI, CLASS_ALL, 9, LOW_IMMORTAL,
                 LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_LOCATE_OBJECT, 1, 0, "locate object", cast_locate_object, 12, 20, 50, TAR_NAME,
                 POSITION_STANDING, LOKI, CLASS_ALL, LOW_IMMORTAL, 4, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_MAGIC_MISSILE, 1, 0, "magic missile", cast_magic_missile, 12, 10, 50,
                 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, POSITION_FIGHTING, LOKI, CLASS_ALL, 1, LOW_IMMORTAL,
                 LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_METEOR_SWARM, 1, 0, "meteor swarm", cast_meteor_swarm, 24, 50, 50, TAR_IGNORE | TAR_VIOLENT,
                 POSITION_FIGHTING, LOKI, CLASS_ALL, 20, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_MINOR_CREATE, 1, 0, "minor creation", cast_minor_creation, 24, 30, 50, TAR_IGNORE,
                 POSITION_STANDING, LOKI, CLASS_ALL, 8, 14, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_MON_SUM_1, 1, 0, "monsum one", cast_mon_sum1, 24, 10, 50, TAR_IGNORE, POSITION_FIGHTING, LOKI,
                 CLASS_ALL, 5, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_MON_SUM_2, 1, 0, "monsum two", cast_mon_sum2, 24, 12, 50, TAR_IGNORE, POSITION_FIGHTING, LOKI,
                 CLASS_ALL, 7, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_MON_SUM_3, 1, 0, "monsum three", cast_mon_sum3, 24, 15, 50, TAR_IGNORE, POSITION_FIGHTING, LOKI,
                 CLASS_ALL, 9, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_MON_SUM_4, 1, 0, "monsum four", cast_mon_sum4, 24, 17, 50, TAR_IGNORE, POSITION_FIGHTING, LOKI,
                 CLASS_ALL, 11, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_MON_SUM_5, 1, 0, "monsum five", cast_mon_sum5, 24, 20, 50, TAR_IGNORE, POSITION_FIGHTING, LOKI,
                 CLASS_ALL, 13, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_MON_SUM_6, 1, 0, "monsum six", cast_mon_sum6, 24, 22, 50, TAR_IGNORE, POSITION_FIGHTING, LOKI,
                 CLASS_ALL, 15, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_MON_SUM_7, 1, 0, "monsum seven", cast_mon_sum7, 24, 25, 50, TAR_IGNORE, POSITION_STANDING, LOKI,
                 CLASS_ALL, 17, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_PARALYSIS, 1, 0, "paralyze", cast_paralyze, 36, 40, 50,
                 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, POSITION_FIGHTING, LOKI, CLASS_ALL, 15, 15, LOKI, LOKI,
                 LOKI, LOKI);
    ASSIGN_SPELL(SPELL_POISON, 1, 0, "poison", cast_poison, 24, 10, 50,
                 TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_OBJ_ROOM | TAR_FIGHT_VICT |
                     TAR_VIOLENT,
                 POSITION_FIGHTING, LOKI, CLASS_ALL, LOW_IMMORTAL, 8, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_POLY_SELF, 1, 0, "polymorph self", cast_poly_self, 12, 30, 50, TAR_IGNORE, POSITION_FIGHTING,
                 LOKI, CLASS_ALL, 8, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_PROTECT_FROM_EVIL, 1, 0, "protection from evil", cast_protection_from_evil, 12, 5, 50,
                 TAR_CHAR_ROOM, POSITION_STANDING, LOKI, CLASS_ALL, LOW_IMMORTAL, 6, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_REFRESH, 1, 0, "refresh", cast_refresh, 12, 5, 50, TAR_CHAR_ROOM, POSITION_STANDING, LOKI,
                 CLASS_ALL, 3, 2, LOKI, LOKI, 11, LOKI);
    ASSIGN_SPELL(SPELL_REMOVE_CURSE, 1, 0, "remove curse", cast_remove_curse, 12, 5, 50,
                 TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_OBJ_ROOM, POSITION_STANDING, LOKI, CLASS_ALL,
                 LOW_IMMORTAL, 7, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_REMOVE_PARALYSIS, 1, 0, "remove paralysis", cast_remove_paralysis, 12, 10, 50,
                 TAR_CHAR_ROOM | TAR_FIGHT_VICT, POSITION_FIGHTING, LOKI, CLASS_ALL, LOW_IMMORTAL, 4, LOKI, LOKI, LOKI,
                 LOKI);
    ASSIGN_SPELL(SPELL_REMOVE_POISON, 1, 0, "remove poison", cast_remove_poison, 12, 5, 50,
                 TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, POSITION_STANDING, LOKI, CLASS_ALL, LOW_IMMORTAL, 5, LOKI,
                 LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_SANCTUARY, 1, 0, "sanctuary", cast_sanctuary, 36, 50, 50, TAR_CHAR_ROOM, POSITION_STANDING, LOKI,
                 CLASS_ALL, LOW_IMMORTAL, 19, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_SECOND_WIND, 1, 0, "second wind", cast_second_wind, 12, 5, 50, TAR_CHAR_ROOM, POSITION_STANDING,
                 LOKI, CLASS_ALL, 12, 6, LOKI, LOKI, 16, LOKI);
    ASSIGN_SPELL(SPELL_SENSE_LIFE, 1, 0, "sense life", cast_sense_life, 12, 5, 50, TAR_CHAR_ROOM, POSITION_STANDING,
                 LOKI, CLASS_ALL, LOW_IMMORTAL, 7, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_SHELTER, 1, 0, "shelter", cast_shelter, 12, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI,
                 CLASS_ALL, 10, 10, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_SHIELD, 1, 0, "shield", cast_shield, 24, 15, 50, TAR_CHAR_ROOM, POSITION_FIGHTING, LOKI,
                 CLASS_ALL, 1, 15, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_SHOCKING_GRASP, 1, 0, "shocking grasp", cast_shocking_grasp, 12, 15, 50,
                 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, POSITION_FIGHTING, LOKI, CLASS_ALL, 1, LOW_IMMORTAL,
                 LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_SLEEP, 1, 0, "sleep", cast_sleep, 24, 15, 50, TAR_CHAR_ROOM | TAR_FIGHT_VICT, POSITION_STANDING,
                 LOKI, CLASS_ALL, 3, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_STONE_SKIN, 1, 0, "stoneskin", cast_stone_skin, 24, 20, 50, TAR_CHAR_ROOM, POSITION_STANDING,
                 LOKI, CLASS_ALL, 16, 32, LOKI, LOKI, 14, LOKI);
    ASSIGN_SPELL(SPELL_STRENGTH, 1, 0, "strength", cast_strength, 12, 10, 50, TAR_CHAR_ROOM, POSITION_STANDING, LOKI,
                 CLASS_ALL, 4, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_SUCCOR, 1, 0, "succor", cast_succor, 24, 15, 50, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL,
                 21, 18, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_SUMMON, 1, 0, "summon", cast_summon, 36, 20, 50, TAR_CHAR_WORLD, POSITION_STANDING, LOKI,
                 CLASS_ALL, 18, 16, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_TELEPORT, 1, 0, "teleport", cast_teleport, 12, 33, 50, TAR_CHAR_ROOM | TAR_FIGHT_VICT,
                 POSITION_FIGHTING, LOKI, CLASS_ALL, 8, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_TRUE_SIGHT, 1, 0, "true sight", cast_true_seeing, 24, 20, 50, TAR_CHAR_ROOM, POSITION_STANDING,
                 LOKI, CLASS_ALL, LOW_IMMORTAL, 12, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_TURN, 1, 0, "turn", cast_turn, 12, 5, 50, TAR_CHAR_ROOM, POSITION_STANDING, LOKI, CLASS_ALL,
                 LOW_IMMORTAL, 1, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_VENTRILOQUATE, 1, 0, "ventriloquate", cast_ventriloquate, 12, 5, 50,
                 TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_SELF_NONO, POSITION_STANDING, LOKI, CLASS_ALL, 1, LOW_IMMORTAL,
                 LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_VISIONS, 1, 0, "visions", cast_visions, 12, 25, 40, TAR_CHAR_WORLD, POSITION_STANDING, LOKI,
                 CLASS_ALL, LOKI, 15, LOKI, LOKI, 30, LOKI);
    ASSIGN_SPELL(SPELL_WATER_BREATH, 1, 0, "water breath", cast_water_breath, 12, 15, 50, TAR_CHAR_ROOM,
                 POSITION_FIGHTING, LOKI, CLASS_ALL, 4, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_WEAKNESS, 1, 0, "weakness", cast_weakness, 12, 10, 50,
                 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, POSITION_FIGHTING, LOKI, CLASS_ALL, 4, LOW_IMMORTAL,
                 LOKI, LOKI, LOKI, LOKI);
    ASSIGN_SPELL(SPELL_WORD_OF_RECALL, 1, 0, "word of recall", cast_word_of_recall, 12, 5, 50,
                 TAR_CHAR_ROOM | TAR_SELF_ONLY, POSITION_FIGHTING, LOKI, CLASS_ALL, LOW_IMMORTAL, 10, LOKI, LOKI, LOKI,
                 LOKI);
}

int splat(struct char_data *ch, struct room_data *rp, int height)
{
    if (DEBUG > 2)
        log_info("called %s with %s, %08zx, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), (size_t)rp, height);

    if (height > 1)
    {
        cprintf(ch, "You are smashed into tiny bits!\r\n");
        act("$n smashes into the ground at high speed", FALSE, ch, 0, 0, TO_ROOM);
        act("You are drenched with blood and gore", FALSE, ch, 0, 0, TO_ROOM);
    }
    else
    {
        if (rp->sector_type >= SECT_WATER_SWIM)
        {
            cprintf(ch, "You PLUNGE into the water... PAIN!\r\n");
            act("$n disappears into the water... SPLASH!", FALSE, ch, 0, 0, TO_ROOM);
        }
        else
        {
            cprintf(ch, "You SLAM into the ground... PAIN!\r\n");
            act("$n lands with a sickening THUMP!", FALSE, ch, 0, 0, TO_ROOM);
        }
    }
    if (!IS_IMMORTAL(ch))
    {
        DamageAllStuff(ch, BLOW_DAMAGE);
        GET_HIT(ch) -= (number(50, 100) * height);
        GET_MOVE(ch) -= number(20, 60);
        update_pos(ch);
        if (GET_HIT(ch) < -10)
        {
            log_death(NULL, ch, "%s has fallen to death", NAME(ch));
            if (!ch->desc)
                GET_GOLD(ch) = 0;
            die(ch);
        }
    }
    else
    { /* Let's still make the imp suffer a bit */
        GET_HIT(ch) -= (number(50, 100) * height);
        GET_MOVE(ch) -= number(20, 60);
        update_pos(ch);
    }
    return check_drowning(ch);
}

int check_falling(struct char_data *ch)
{
    struct room_data *rp = NULL;
    struct room_data *targ = NULL;
    int done = FALSE;
    int count = 0;

    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

    if (IS_AFFECTED(ch, AFF_FLYING))
        return FALSE;

    rp = real_roomp(ch->in_room);
    if (!rp)
        return FALSE;

    if (rp->sector_type != SECT_AIR)
        return FALSE;

    act("The world spins, and you plummet out of control", TRUE, ch, 0, 0, TO_CHAR);
    done = FALSE;
    count = 0;

    while (!done && count < 100)
    {
        if (rp->dir_option[DOWN] && rp->dir_option[DOWN]->to_room > -1)
            targ = real_roomp(rp->dir_option[DOWN]->to_room);
        else
            return splat(ch, rp, count);
        act("$n plunges towards oblivion", FALSE, ch, 0, 0, TO_ROOM);
        cprintf(ch, "You plunge from the sky\r\n");
        char_from_room(ch);
        char_to_room(ch, rp->dir_option[DOWN]->to_room);
        act("$n falls from the sky", FALSE, ch, 0, 0, TO_ROOM);
        count++;
        do_look(ch, "", 0);
        if (targ->sector_type != SECT_AIR)
        {
            if (targ->sector_type >= SECT_WATER_SWIM)
                return splat(ch, targ, 1);
            else
                return splat(ch, targ, count);
        }
        else
        {
            rp = targ;
            targ = 0;
        }
    }
    if (count >= 100)
    {
        log_error("Someone fucked up an air room.");
        char_from_room(ch);
        char_to_room(ch, GET_HOME(ch));
        do_look(ch, "", 0);
        return FALSE;
    }
    return TRUE;
}

int check_drowning(struct char_data *ch)
{
    struct room_data *rp = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

    if (IS_AFFECTED(ch, AFF_WATERBREATH))
        return FALSE;
    if (!(rp = real_roomp(ch->in_room)))
        return FALSE;

    if (rp->sector_type == SECT_UNDERWATER)
    {
        cprintf(ch, "PANIC!  You're drowning!!!!!!\r\n");
        GET_HIT(ch) -= dice(6, 6);
        GET_MOVE(ch) -= dice(5, 10);
        update_pos(ch);
        if (GET_HIT(ch) < -10)
        {
            log_death(NULL, ch, "%s killed by drowning underwater", GET_NAME(ch));
            if (!ch->desc)
                GET_GOLD(ch) = 0;
            die(ch);
        }
        return TRUE;
    }
    else if (rp->sector_type == SECT_WATER_NOSWIM)
    {
        if (number(1, 101) < ch->skills[SKILL_SWIMMING].learned / 3)
        {
            cprintf(ch, "GAK!  You swallow some water and gasp for air!\r\n");
            GET_HIT(ch) -= dice(3, 6);
            GET_MOVE(ch) -= dice(2, 12);
            update_pos(ch);
            if (!ch->desc)
            {
                GET_GOLD(ch) -= dice(10, 10);
                if (GET_GOLD(ch) < 0)
                    GET_GOLD(ch) = 0;
            }
            if (GET_HIT(ch) < -10)
            {
                log_death(NULL, ch, "%s killed by drowning in rough water", NAME(ch));
                die(ch);
            }
            return TRUE;
        }
    }
    else if (rp->sector_type == SECT_WATER_SWIM)
    {
        if (number(1, 101) < ch->skills[SKILL_SWIMMING].learned)
        {
            cprintf(ch, "GAK!  You swallow some water and gasp for air!\r\n");
            GET_HIT(ch) -= dice(1, 8);
            GET_MOVE(ch) -= dice(2, 6);
            update_pos(ch);
            if (!ch->desc)
            {
                GET_GOLD(ch) -= dice(5, 10);
                if (GET_GOLD(ch) < 0)
                    GET_GOLD(ch) = 0;
            }
            if (GET_HIT(ch) < -10)
            {
                log_death(NULL, ch, "%s killed by drowning in shallow water", NAME(ch));
                die(ch);
            }
            return TRUE;
        }
    }
    return FALSE;
}

void check_falling_obj(struct obj_data *obj, int room)
{
    struct room_data *rp = NULL;
    struct room_data *targ = NULL;
    int done = FALSE;
    int count = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_ONAME(obj), room);

    if (obj->in_room != room)
    {
        log_error("unusual object information in check_falling_obj");
        return;
    }
    rp = real_roomp(room);
    if (!rp)
        return;

    if (rp->sector_type != SECT_AIR)
        return;

    done = FALSE;
    count = 0;

    while (!done && count < 100)
    {

        if (rp->dir_option[DOWN] && rp->dir_option[DOWN]->to_room > -1)
        {
            targ = real_roomp(rp->dir_option[DOWN]->to_room);
        }
        else
        {
            /*
             * pretend that this is the smash room.
             */
            if (count > 1)
            {

                if (rp->people)
                {
                    act("$p smashes against the ground at high speed", FALSE, rp->people, obj, 0, TO_ROOM);
                    act("$p smashes against the ground at high speed", FALSE, rp->people, obj, 0, TO_CHAR);
                }
                return;
            }
            else
            {

                if (rp->people)
                {
                    act("$p lands with a loud THUMP!", FALSE, rp->people, obj, 0, TO_ROOM);
                    act("$p lands with a loud THUMP!", FALSE, rp->people, obj, 0, TO_CHAR);
                }
                return;
            }
        }

        if (rp->people)
        { /* have to reference a person */
            act("$p falls out of sight", FALSE, rp->people, obj, 0, TO_ROOM);
            act("$p falls out of sight", FALSE, rp->people, obj, 0, TO_CHAR);
        }
        obj_from_room(obj);
        obj_to_room(obj, rp->dir_option[DOWN]->to_room);
        if (targ->people)
        {
            act("$p falls from the sky", FALSE, targ->people, obj, 0, TO_ROOM);
            act("$p falls from the sky", FALSE, targ->people, obj, 0, TO_CHAR);
        }
        count++;

        if (targ->sector_type != SECT_AIR)
        {
            if (count == 1)
            {
                if (targ->people)
                {
                    act("$p lands with a loud THUMP!", FALSE, 0, obj, 0, TO_ROOM);
                    act("$p lands with a loud THUMP!", FALSE, 0, obj, 0, TO_CHAR);
                }
                return;
            }
            else
            {
                if (targ->people)
                {
                    if (targ->sector_type >= SECT_WATER_SWIM)
                    {
                        act("$p smashes against the water at high speed", FALSE, targ->people, obj, 0, TO_ROOM);
                        act("$p smashes against the water at high speed", FALSE, targ->people, obj, 0, TO_CHAR);
                    }
                    else
                    {
                        act("$p smashes against the ground at high speed", FALSE, targ->people, obj, 0, TO_ROOM);
                        act("$p smashes against the ground at high speed", FALSE, targ->people, obj, 0, TO_CHAR);
                    }
                }
                return;
            }
        }
        else
        {
            /*
             * time to try the next room
             */
            rp = targ;
            targ = 0;
        }
    }

    if (count >= 100)
    {
        log_error("Someone fucked up an air room.");
        obj_from_room(obj);
        obj_to_room(obj, 2);
        return;
    }
}

int check_nature(struct char_data *i)
{
    if (DEBUG > 3)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(i));

    if (check_falling(i))
    {
        return TRUE;
    }
    return check_drowning(i);
}

void check_all_nature(int current_pulse)
{
    struct char_data *i = NULL;

    if (DEBUG > 3)
        log_info("called %s with %d", __PRETTY_FUNCTION__, current_pulse);

    for (i = character_list; i; i = i->next)
        check_nature(i);
}

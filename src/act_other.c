/*
 * file: act.other.c , Implementation of commands.        Part of DIKUMUD
 * Usage : Other commands.
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */

#include <stdio.h>
#include <stdlib.h>
/* #include <unistd.h> */
#include <sys/types.h>
#include <string.h>
#include <ctype.h>

#include "global.h"
#include "bug.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "mudlimits.h"
#include "spell_parser.h"
#include "reception.h"
#include "multiclass.h"
#include "fight.h"
#include "spec_procs.h"
#include "act_skills.h"
#include "act_comm.h"
#include "modify.h"
#define _ACT_OTHER_C
#include "act_other.h"

void do_gain(struct char_data *ch, const char *argument, int cmd)
{
    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    return;
}

void do_guard(struct char_data *ch, const char *argument, int cmd)
{
    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (!IS_NPC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF))
    {
        cprintf(ch, "Sorry. you can't just put your brain on autopilot!\r\n");
        return;
    }
    if (IS_SET(ch->specials.act, ACT_GUARDIAN))
    {
        act("$n relaxes.", FALSE, ch, 0, 0, TO_ROOM);
        cprintf(ch, "You relax.\r\n");
        REMOVE_BIT(ch->specials.act, ACT_GUARDIAN);
    }
    else
    {
        SET_BIT(ch->specials.act, ACT_GUARDIAN);
        act("$n alertly watches you.", FALSE, ch, 0, 0, TO_ROOM);
        cprintf(ch, "You snap to attention\r\n");
    }
    return;
}

void do_junk(struct char_data *ch, const char *argument, int cmd)
{
    char arg[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char newarg[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    struct obj_data *tmp_object = NULL;

    /*
     * struct obj_data *old_object = NULL;
     */
    int num = 0;
    int p = 0;
    int count = 0;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);
    /*
     *   get object name & verify
     */

    only_argument(argument, arg);
    if (*arg)
    {
        if (getall(arg, newarg) != FALSE)
        {
            num = -1;
            strlcpy(arg, newarg, MAX_INPUT_LENGTH);
        }
        else if ((p = getabunch(arg, newarg)) != FALSE)
        {
            num = p;
            strlcpy(arg, newarg, MAX_INPUT_LENGTH);
        }
        else
        {
            num = 1;
        }

        count = 0;
        while (num != 0)
        {
            tmp_object = get_obj_in_list_vis(ch, arg, ch->carrying);
            if (tmp_object)
            {
                /*
                 * old_object = tmp_object;
                 */
                obj_from_char(tmp_object);
                extract_obj(tmp_object);
                if (num > 0)
                    num--;
                count++;
            }
            else
            {
                if (count > 1)
                {
                    act("You junk %s (%d).\r\n", 1, ch, 0, 0, TO_CHAR, arg, count);
                    act("$n junks %s.\r\n", 1, ch, 0, 0, TO_ROOM, arg);
                }
                else if (count == 1)
                {
                    act("You junk %s \r\n", 1, ch, 0, 0, TO_CHAR, arg);
                    act("$n junks %s.\r\n", 1, ch, 0, 0, TO_ROOM, arg);
                }
                else
                {
                    cprintf(ch, "You don't have anything like that\r\n");
                }
                return;
            }
        }
    }
    else
    {
        cprintf(ch, "Junk what?");
        return;
    }
}

void do_qui(struct char_data *ch, const char *argument, int cmd)
{
    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    cprintf(ch, "You have to write quit - no less, to quit!\r\n");
    return;
}

void do_title(struct char_data *ch, const char *argument, int cmd)
{
    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (IS_NPC(ch) || !ch->desc)
        return;

    for (; isspace(*argument); argument++)
        ;

    if (*argument)
    {
        cprintf(ch, "Your title has been set to : <%s>\r\n", argument);
        ch->player.title = strdup(argument);
    }
}

void do_quit(struct char_data *ch, const char *argument, int cmd)
{
    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (IS_NPC(ch) || !ch->desc)
        return;

    if (GET_POS(ch) == POSITION_FIGHTING)
    {
        cprintf(ch, "No way! You are fighting.\r\n");
        return;
    }
    if (GET_POS(ch) < POSITION_STUNNED)
    {
        cprintf(ch, "You die before your time!\r\n");
        die(ch);
        return;
    }
    if (MOUNTED(ch))
    {
        Dismount(ch, MOUNTED(ch), POSITION_STANDING);
    }
    act("Goodbye, friend.. Come back soon!", FALSE, ch, 0, 0, TO_CHAR);
    act("$n has left the game.", TRUE, ch, 0, 0, TO_ROOM);
    GET_HOME(ch) = 3008;
    zero_rent(ch);
    extract_char(ch); /* Char is saved in extract char */
}

void do_save(struct char_data *ch, const char *argument, int cmd)
{
    struct obj_cost cost;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (IS_NPC(ch))
        return;

    cprintf(ch, "Saving %s.\r\n", GET_NAME(ch));
    recep_offer(ch, NULL, &cost);
    new_save_equipment(ch, &cost, FALSE);
    save_obj(ch, &cost, FALSE);
    save_char(ch, NOWHERE);
}

void do_not_here(struct char_data *ch, const char *argument, int cmd)
{
    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    cprintf(ch, "Sorry, but you cannot do that here!\r\n");
}

void do_sneak(struct char_data *ch, const char *argument, int cmd)
{
    int percent_chance = 0;
    struct affected_type af;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (IS_AFFECTED(ch, AFF_SNEAK))
    {
        affect_from_char(ch, SKILL_SNEAK);
        if (IS_AFFECTED(ch, AFF_HIDE))
            REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);
        cprintf(ch, "You are no longer sneaky.\r\n");
        return;
    }
    cprintf(ch, "Ok, you'll try to move silently for a while.\r\n");
    cprintf(ch, "And you attempt to hide yourself.\r\n");

    percent_chance = number(1, 101); /* 101% is a complete failure */

    if (percent_chance > ch->skills[SKILL_SNEAK].learned + dex_app_skill[(int)GET_DEX(ch)].sneak)
        return;

    if (ch->skills[SKILL_SNEAK].learned < 50)
        ch->skills[SKILL_SNEAK].learned += 2;

    SET_BIT(ch->specials.affected_by, AFF_HIDE);
    af.type = SKILL_SNEAK;
    af.duration = GET_LEVEL(ch, BestThiefClass(ch));
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_SNEAK;
    affect_to_char(ch, &af);
}

void do_hide(struct char_data *ch, const char *argument, int cmd)
{
    int percent_chance = 0;
    struct affected_type af;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    cprintf(ch, "Ok, you crawl into a dark corner and lurk.\r\n");

    if (IS_AFFECTED(ch, AFF_SNEAK))
        affect_from_char(ch, SKILL_SNEAK);
    if (IS_AFFECTED(ch, AFF_HIDE))
        REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);

    percent_chance = number(1, 101); /* 101% is a complete failure */

    if (percent_chance > ch->skills[SKILL_SNEAK].learned + dex_app_skill[(int)GET_DEX(ch)].sneak)
        return;
    if (ch->skills[SKILL_SNEAK].learned < 50)
        ch->skills[SKILL_SNEAK].learned += 2;

    SET_BIT(ch->specials.affected_by, AFF_HIDE);
    af.type = SKILL_SNEAK;
    af.duration = GET_LEVEL(ch, BestThiefClass(ch));
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_SNEAK;
    affect_to_char(ch, &af);
}

void do_steal(struct char_data *ch, const char *argument, int cmd)
{
    struct char_data *victim = NULL;
    struct obj_data *obj = NULL;
    char victim_name[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char obj_name[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    int percent_chance = 0;
    int gold = 0;
    int eq_pos = 0;
    char ohoh = FALSE;
    struct room_data *rp = NULL;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (check_peaceful(ch, "What if he caught you?\r\n"))
        return;

    argument = one_argument(argument, obj_name);
    only_argument(argument, victim_name);

    if (!(victim = get_char_room_vis(ch, victim_name)))
    {
        cprintf(ch, "Steal what from who?\r\n");
        return;
    }
    else if (victim == ch)
    {
        cprintf(ch, "Come on now, that's rather stupid!\r\n");
        return;
    }
    if (!CheckKill(ch, victim))
        return;

    if ((!victim->desc) && (IS_PC(victim)))
        return;

    /*
     * 101% is a complete failure
     */
    percent_chance = number(1, 101) - dex_app_skill[(int)GET_DEX(ch)].p_pocket;

    if (GET_POS(victim) < POSITION_SLEEPING)
        percent_chance = -1; /* ALWAYS SUCCESS */

    percent_chance += GetTotLevel(victim);
    if (GetMaxLevel(victim) > MAX_MORT)
        percent_chance = 101; /* Failure */

    if (str_cmp(obj_name, "coins") && str_cmp(obj_name, "gold"))
    {
        if (!(obj = get_obj_in_list_vis(victim, obj_name, victim->carrying)))
        {
            for (eq_pos = 0; (eq_pos < MAX_WEAR); eq_pos++)
                if (victim->equipment[eq_pos] && (isname(obj_name, victim->equipment[eq_pos]->name)) &&
                    CAN_SEE_OBJ(ch, victim->equipment[eq_pos]))
                {
                    obj = victim->equipment[eq_pos];
                    break;
                }
            if (!obj)
            {
                act("$E has not got that item.", FALSE, ch, 0, victim, TO_CHAR);
                return;
            }
            else
            { /* It is equipment */
                if ((GET_POS(victim) > POSITION_STUNNED))
                {
                    cprintf(ch, "Steal the equipment now? Impossible!\r\n");
                    return;
                }
                else
                {
                    act("You unequip $p and steal it.", FALSE, ch, obj, 0, TO_CHAR);
                    act("$n steals $p from $N.", FALSE, ch, obj, victim, TO_NOTVICT);
                    obj_to_char(unequip_char(victim, eq_pos), ch);
                }
            }
        }
        else
        {                                          /* obj found in inventory */
            percent_chance += GET_OBJ_WEIGHT(obj); /* Make heavy harder */
            rp = real_roomp(ch->in_room);
            if (IS_SET(rp->room_flags, NO_STEAL))
            {
                percent_chance = 101;
            }
            if (AWAKE(victim) && (percent_chance > ch->skills[SKILL_STEAL].learned))
            {
                ohoh = TRUE;
                act("Oops..", FALSE, ch, 0, 0, TO_CHAR);
                act("$n tried to steal something from you!", FALSE, ch, 0, victim, TO_VICT);
                act("$n tries to steal something from $N.", TRUE, ch, 0, victim, TO_NOTVICT);
            }
            else
            { /* Steal the item */
                if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch)))
                {
                    if ((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) < CAN_CARRY_W(ch))
                    {
                        obj_from_char(obj);
                        obj_to_char(obj, ch);
                        cprintf(ch, "Got it!\r\n");
                        if (ch->skills[SKILL_STEAL].learned < 50)
                            ch->skills[SKILL_STEAL].learned += 1;
                    }
                }
                else
                    cprintf(ch, "You cannot carry that much.\r\n");
            }
        }
    }
    else
    { /* Steal some coins */
        if (AWAKE(victim) && (percent_chance > ch->skills[SKILL_STEAL].learned))
        {
            ohoh = TRUE;
            act("Oops..", FALSE, ch, 0, 0, TO_CHAR);
            act("You discover that $n has $s hands in your wallet.", FALSE, ch, 0, victim, TO_VICT);
            act("$n tries to steal gold from $N.", TRUE, ch, 0, victim, TO_NOTVICT);
        }
        else
        {
            /*
             * Steal some gold coins
             */
            gold = (int)((GET_GOLD(victim) * number(1, 10)) / 100);
            gold = MIN(100, gold);
            if (gold > 0)
            {
                GET_GOLD(ch) += gold;
                GET_GOLD(victim) -= gold;
                cprintf(ch, "Bingo! You got %d gold coins.\r\n", gold);
            }
            else
            {
                cprintf(ch, "You couldn't get any gold...\r\n");
            }
        }
    }

    if (ohoh && IS_NPC(victim) && AWAKE(victim))
    {
        if (IS_SET(victim->specials.act, ACT_NICE_THIEF))
        {
            snprintf(buf, MAX_INPUT_LENGTH, "%s is a damn thief!", GET_NAME(ch));
            do_shout(victim, buf, 0);
            log_info("%s", buf);
            cprintf(ch, "Don't you ever do that again!\r\n");
        }
        else
        {
            hit(victim, ch, TYPE_UNDEFINED);
        }
    }
}

void do_practice(struct char_data *ch, const char *argument, int cmd)
{
    char buf[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    int i = 0;
    char first_arg[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char sec_arg[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    int flag = FALSE;

    /*
    struct skill_struct {
    char                                    skill_name[40];
    int                                     skill_numb;
    };
    */

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (cmd != CMD_practice)
        return;

    for (; isspace(*argument); argument++)
        ;
    half_chop(argument, first_arg, sec_arg);

    if (!*first_arg)
    {
        cprintf(ch, "You need to supply a class for that.");
        return;
    }
    switch (*first_arg)
    {
    case 'w':
    case 'W':
    case 'f':
    case 'F': {
        if (!HasClass(ch, CLASS_WARRIOR))
        {
            cprintf(ch, "I bet you wish you were a warrior.\r\n");
            return;
        }
        snprintf(buf, MAX_STRING_LENGTH, "You can practice any of these MANLY skills:\r\n");
        for (i = 0; i < MAX_SKILLS; i++)
        {
            if (CanUseClass(ch, i, WARRIOR_LEVEL_IND))
                scprintf(buf, MAX_STRING_LENGTH, "%s%s\r\n", spell_info[i].name, how_good(ch->skills[i].learned));
        }
        page_string(ch->desc, buf, 1);
        return;
    }
    break;
    case 'r':
    case 'R': {
        if (!HasClass(ch, CLASS_RANGER))
        {
            cprintf(ch, "I wish I was a Ranger too!\r\n");
            return;
        }
        snprintf(buf, MAX_STRING_LENGTH, "You can practice any of these outdoor skills:\r\n");
        for (i = 0; i < MAX_SKILLS; i++)
        {
            if (CanUseClass(ch, i, RANGER_LEVEL_IND))
                scprintf(buf, MAX_STRING_LENGTH, "%s%s\r\n", spell_info[i].name, how_good(ch->skills[i].learned));
        }
        scprintf(buf, MAX_STRING_LENGTH, "Or these leaf-and-twig charms:\r\n");
        for (i = 0; i < MAX_SKILLS; i++)
        {
            if (CanCastClass(ch, i, RANGER_LEVEL_IND))
                scprintf(buf, MAX_STRING_LENGTH, "%s%s\r\n", spell_info[i].name, how_good(ch->skills[i].learned));
        }
        page_string(ch->desc, buf, 1);
        return;
    }
    break;
    case 't':
    case 'T': {
        if (!HasClass(ch, CLASS_THIEF))
        {
            cprintf(ch, "A voice whispers, 'You are not a thief.'\r\n");
            return;
        }
        snprintf(buf, MAX_STRING_LENGTH, "You can practice any of these sneaky skills:\r\n");
        for (i = 0; i < MAX_SKILLS; i++)
        {
            if (CanUseClass(ch, i, THIEF_LEVEL_IND))
                scprintf(buf, MAX_STRING_LENGTH, "%s%s\r\n", spell_info[i].name, how_good(ch->skills[i].learned));
        }
        page_string(ch->desc, buf, 1);
        return;
    }
    break;
    case 'M':
    case 'm': {
        if (!HasClass(ch, CLASS_MAGIC_USER))
        {
            cprintf(ch, "You pretend to be a magic-user, 'Gooble-dah!'\r\n");
            return;
        }
        flag = 0;
        if (is_abbrev(sec_arg, "known"))
            flag = 1;
        snprintf(buf, MAX_STRING_LENGTH, "Your heavy spellbook contains these spells:\r\n");
        for (i = 0; i < MAX_SKILLS; i++)
            if (CanCastClass(ch, i, MAGE_LEVEL_IND))
            {
                if (!flag)
                {
                    scprintf(buf, MAX_STRING_LENGTH, "[%2d] %s%s\r\n", spell_info[i].min_level[MAGE_LEVEL_IND],
                             spell_info[i].name, how_good(ch->skills[i].learned));
                }
                else
                {
                    if (ch->skills[i].learned > 0)
                    {
                        scprintf(buf, MAX_STRING_LENGTH, "[%2d] %s%s\r\n", spell_info[i].min_level[MAGE_LEVEL_IND],
                                 spell_info[i].name, how_good(ch->skills[i].learned));
                    }
                }
            }
        page_string(ch->desc, buf, 1);
        return;
    }
    break;
    case 'C':
    case 'c':
    case 'P':
    case 'p': {
        if (!HasClass(ch, CLASS_CLERIC))
        {
            cprintf(ch, "You feel that impersonating a cleric might be bad.\r\n");
            return;
        }
        snprintf(buf, MAX_STRING_LENGTH, "You can pray for any of these miracles:\r\n");
        flag = 0;
        if (is_abbrev(sec_arg, "known"))
            flag = 1;
        for (i = 0; i < MAX_SKILLS; i++)
            if (CanCastClass(ch, i, CLERIC_LEVEL_IND))
            {
                if (!flag)
                {
                    scprintf(buf, MAX_STRING_LENGTH, "[%2d] %s%s\r\n", spell_info[i].min_level[CLERIC_LEVEL_IND],
                             spell_info[i].name, how_good(ch->skills[i].learned));
                }
                else
                {
                    if (ch->skills[i].learned > 0)
                    {
                        scprintf(buf, MAX_STRING_LENGTH, "[%2d] %s%s\r\n", spell_info[i].min_level[CLERIC_LEVEL_IND],
                                 spell_info[i].name, how_good(ch->skills[i].learned));
                    }
                }
            }
        page_string(ch->desc, buf, 1);
        return;
    }
    break;
    default:
        cprintf(ch, "Ah, but practice which class???\r\n");
    }
}

void do_idea(struct char_data *ch, const char *argument, int cmd)
{
    FILE *fl = NULL;
    char str[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (IS_NPC(ch))
    {
        cprintf(ch, "Monsters can't have ideas - Go away.\r\n");
        return;
    }
    /*
     * skip whites
     */
    for (; isspace(*argument); argument++)
        ;

    if (!*argument)
    {
        cprintf(ch, "That doesn't sound like a good idea to me.. Sorry.\r\n");
        return;
    }
    if (!(fl = fopen(IDEA_FILE, "a")))
    {
        log_error("do_idea");
        cprintf(ch, "Could not open the idea-file.\r\n");
        return;
    }
    snprintf(str, MAX_INPUT_LENGTH, "**%s: %s\n", GET_NAME(ch), argument);

    fputs(str, fl);
    FCLOSE(fl);
    cprintf(ch, "Woah!  That's pretty cool.  Thanks!\r\n");
}

void do_typo(struct char_data *ch, const char *argument, int cmd)
{
    FILE *fl = NULL;
    char str[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (IS_NPC(ch))
    {
        cprintf(ch, "Monsters can't spell - leave me alone.\r\n");
        return;
    }
    /*
     * skip whites
     */
    for (; isspace(*argument); argument++)
        ;

    if (!*argument)
    {
        cprintf(ch, "I beg your pardon?\r\n");
        return;
    }
    if (!(fl = fopen(TYPO_FILE, "a")))
    {
        log_error("do_typo");
        cprintf(ch, "Could not open the typo-file.\r\n");
        return;
    }
    snprintf(str, MAX_INPUT_LENGTH, "**%s[%d]: %s\n", GET_NAME(ch), ch->in_room, argument);
    fputs(str, fl);
    FCLOSE(fl);
    cprintf(ch, "No problem.  We can send a whizzling to fix it.\r\n");
}

void do_bug(struct char_data *ch, const char *argument, int cmd)
{
    FILE *fl = NULL;
    char str[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (IS_NPC(ch))
    {
        cprintf(ch, "You are a monster! Bug off!\r\n");
        return;
    }
    /*
     * skip whites
     */
    for (; isspace(*argument); argument++)
        ;

    if (!*argument)
    {
        cprintf(ch, "Pardon?\r\n");
        return;
    }
    if (!(fl = fopen(BUG_FILE, "a")))
    {
        log_error("do_bug");
        cprintf(ch, "Could not open the bug-file.\r\n");
        return;
    }
    snprintf(str, MAX_INPUT_LENGTH, "**%s[%d]: %s\n", GET_NAME(ch), ch->in_room, argument);
    fputs(str, fl);
    FCLOSE(fl);
    cprintf(ch, "Really?  Ok, we'll send someone to have a look.\r\n");
}

void do_autoexit(struct char_data *ch, const char *argument, int cmd)
{
    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (IS_NPC(ch))
        return;

    if (IS_SET(ch->specials.new_act, NEW_PLR_AUTOEXIT))
    {
        cprintf(ch, "You will no longer see exits automatically.\r\n");
        REMOVE_BIT(ch->specials.new_act, NEW_PLR_AUTOEXIT);
    }
    else
    {
        cprintf(ch, "You will now see exits automatically.\r\n");
        SET_BIT(ch->specials.new_act, NEW_PLR_AUTOEXIT);
    }
}

void do_brief(struct char_data *ch, const char *argument, int cmd)
{
    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (IS_NPC(ch))
        return;

    if (IS_SET(ch->specials.act, PLR_BRIEF))
    {
        cprintf(ch, "Brief mode off.\r\n");
        REMOVE_BIT(ch->specials.act, PLR_BRIEF);
    }
    else
    {
        cprintf(ch, "Brief mode on.\r\n");
        SET_BIT(ch->specials.act, PLR_BRIEF);
    }
}

void do_compact(struct char_data *ch, const char *argument, int cmd)
{
    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (IS_NPC(ch))
        return;

    if (IS_SET(ch->specials.act, PLR_COMPACT))
    {
        cprintf(ch, "You are now in the uncompacted mode.\r\n");
        REMOVE_BIT(ch->specials.act, PLR_COMPACT);
    }
    else
    {
        cprintf(ch, "You are now in compact mode.\r\n");
        SET_BIT(ch->specials.act, PLR_COMPACT);
    }
}

void do_group(struct char_data *ch, const char *argument, int cmd)
{
    char name[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    struct char_data *victim = NULL;
    struct char_data *k = NULL;
    struct follow_type *f = NULL;
    int found = FALSE;
    int victlvl = 0;
    int chlvl = 0;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    only_argument(argument, name);

    if (!*name)
    {
        if (!IS_AFFECTED(ch, AFF_GROUP))
        {
            cprintf(ch, "But you are a member of no group?!\r\n");
        }
        else
        {
            cprintf(ch, "Your group consists of:\r\n");
            if (ch->master)
                k = ch->master;
            else
                k = ch;

            if (IS_AFFECTED(k, AFF_GROUP))
                act("     $N (Head of group)", FALSE, ch, 0, k, TO_CHAR);

            for (f = k->followers; f; f = f->next)
                if (IS_AFFECTED(f->follower, AFF_GROUP))
                    act("     $N", FALSE, ch, 0, f->follower, TO_CHAR);
        }
        return;
    }
    if (!(victim = get_char_room_vis(ch, name)))
    {
        cprintf(ch, "No one here by that name.\r\n");
    }
    else
    {
        if (ch->master)
        {
            act("You can not enroll group members without being head of a group.", FALSE, ch, 0, 0, TO_CHAR);
            return;
        }
        found = FALSE;

        if (victim == ch)
            found = TRUE;
        else
        {
            for (f = ch->followers; f; f = f->next)
            {
                if (f->follower == victim)
                {
                    found = TRUE;
                    break;
                }
            }
        }

        if (found)
        {
            if (IS_AFFECTED(victim, AFF_GROUP))
            {
                if (victim != ch)
                {
                    act("$n has been kicked out of $N's group!", FALSE, victim, 0, ch, TO_ROOM);
                    act("You are no longer a member of $N's group!", FALSE, victim, 0, ch, TO_CHAR);
                }
                else
                {
                    act("You are no longer a group leader!", FALSE, victim, 0, ch, TO_CHAR);
                }
                REMOVE_BIT(victim->specials.affected_by, AFF_GROUP);
            }
            else
            {
                if (IS_IMMORTAL(victim) && !IS_IMMORTAL(ch))
                {
                    act("You really don't want $n in your group.", FALSE, victim, 0, 0, TO_CHAR);
                    return;
                }
                /*
                 *	if ((chlvl= GetMaxLevel(ch)) >= LOW_IMMORTAL) {
                 *	  act("Now now.  That would be CHEATING!", FALSE, ch, 0, 0, TO_CHAR);
                 *	  return;
                 *	}
                 */
                chlvl = GetMaxLevel(ch);
                victlvl = GetMaxLevel(victim);
                if (IS_PC(victim) && victim != ch)
                {
                    int toolow, toohigh;

                    /*
                     * newbies can mix with each other
                     */
                    toolow = (chlvl <= 4) ? 0 : MAX(3, chlvl / 2);
                    /*
                     * but newbies and adults don't mix
                     */
                    toohigh = (chlvl <= 4) ? 5 : (chlvl + chlvl / 2);
                    if (victlvl <= toolow)
                    {
                        act("It would be beneath you to lead the lowly $N into battle!", FALSE, ch, 0, victim, TO_CHAR);
                        act("$N is too mighty to pay any heed to the likes of you!", FALSE, victim, 0, ch, TO_CHAR);
                        act("$N tells $n to bugger off.", FALSE, victim, 0, ch, TO_ROOM);
                        return;
                    }
                    if (victlvl >= toohigh)
                    {
                        act("You WISH $N would deign to join your group!", FALSE, ch, 0, victim, TO_CHAR);
                        act("$N BEGS you to help $M go kill the chickens.", FALSE, victim, 0, ch, TO_CHAR);
                        act("$N grovels and begs $n to join $S group.", FALSE, victim, 0, ch, TO_ROOM);
                        return;
                    }
                }
                if (victim != ch)
                {
                    act("$n joins the glorious cause of $N's group.", FALSE, victim, 0, ch, TO_ROOM);
                    act("You are now a member of $N's group.", FALSE, victim, 0, ch, TO_CHAR);
                }
                else
                {
                    act("You are now your own group leader.", FALSE, victim, 0, ch, TO_CHAR);
                }
                SET_BIT(victim->specials.affected_by, AFF_GROUP);
            }
        }
        else
        {
            act("$N must follow you, to enter the group", FALSE, ch, 0, victim, TO_CHAR);
        }
    }
}

void do_quaff(struct char_data *ch, const char *argument, int cmd)
{
    char buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    struct obj_data *temp = NULL;
    int i = 0;
    int equipped = FALSE;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    only_argument(argument, buf);

    if (!(temp = get_obj_in_list_vis(ch, buf, ch->carrying)))
    {
        temp = ch->equipment[HOLD];
        equipped = TRUE;
        if ((temp == 0) || !isname(buf, temp->name))
        {
            act("You do not have that item.", FALSE, ch, 0, 0, TO_CHAR);
            return;
        }
    }
    if (!IS_IMMORTAL(ch))
    {
        if (GET_COND(ch, FULL) > 20)
        {
            act("Your stomach can't contain anymore!", FALSE, ch, 0, 0, TO_CHAR);
            return;
        }
        else
        {
            GET_COND(ch, FULL) += 1;
        }
    }
    if (temp->obj_flags.type_flag != ITEM_POTION)
    {
        act("You can only quaff potions.", FALSE, ch, 0, 0, TO_CHAR);
        return;
    }
    act("$n quaffs $p.", TRUE, ch, temp, 0, TO_ROOM);
    act("You quaff $p which dissolves.", FALSE, ch, temp, 0, TO_CHAR);

    /*
     * my stuff
     */
    if (ch->specials.fighting)
    {
        if (equipped)
        {
            if (number(1, 20) > ch->abilities.dex)
            {
                act("$n is jolted and drops $p!  It shatters!", TRUE, ch, temp, 0, TO_ROOM);
                act("You arm is jolted and $p flies from your hand, *SMASH*", TRUE, ch, temp, 0, TO_CHAR);
                if (equipped)
                    temp = unequip_char(ch, HOLD);
                extract_obj(temp);
                return;
            }
        }
        else
        {
            if (number(1, 20) > ch->abilities.dex - 4)
            {
                act("$n is jolted and drops $p!  It shatters!", TRUE, ch, temp, 0, TO_ROOM);
                act("You arm is jolted and $p flies from your hand, *SMASH*", TRUE, ch, temp, 0, TO_CHAR);
                extract_obj(temp);
                return;
            }
        }
    }
    for (i = 1; i < 4; i++)
        if (temp->obj_flags.value[i] >= 1)
            ((*spell_info[temp->obj_flags.value[i]].spell_pointer)((char)temp->obj_flags.value[0], ch, "",
                                                                   SPELL_TYPE_POTION, ch, temp));

    if (equipped)
        temp = unequip_char(ch, HOLD);

    extract_obj(temp);

    WAIT_STATE(ch, PULSE_VIOLENCE);
}

void do_recite(struct char_data *ch, const char *argument, int cmd)
{
    char buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    struct obj_data *scroll = NULL;
    struct obj_data *obj = NULL;
    struct char_data *victim = NULL;
    int i = 0;
    int bits = 0;
    int equipped = FALSE;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    argument = one_argument(argument, buf);

    if (!(scroll = get_obj_in_list_vis(ch, buf, ch->carrying)))
    {
        scroll = ch->equipment[HOLD];
        equipped = TRUE;
        if ((scroll == 0) || !isname(buf, scroll->name))
        {
            act("You do not have that item.", FALSE, ch, 0, 0, TO_CHAR);
            return;
        }
    }
    if (scroll->obj_flags.type_flag != ITEM_SCROLL)
    {
        act("Recite is normally used for scrolls.", FALSE, ch, 0, 0, TO_CHAR);
        return;
    }
    if (*argument)
    {
        bits =
            generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP | FIND_CHAR_ROOM, ch, &victim, &obj);
        if (bits == 0)
        {
            cprintf(ch, "No such thing around to recite the scroll on.\r\n");
            return;
        }
    }
    else
    {
        victim = ch;
    }

    if (ch->skills[SKILL_READ_MAGIC].learned < number(1, 101))
    {
        if (scroll->obj_flags.value[1] != SPELL_WORD_OF_RECALL)
        {
            cprintf(ch, "You can't understand this...\r\n");
            return;
        }
    }
    act("$n recites $p.", TRUE, ch, scroll, 0, TO_ROOM);
    act("You recite $p which bursts into flame.", FALSE, ch, scroll, 0, TO_CHAR);

    for (i = 1; i < 4; i++)
        if (scroll->obj_flags.value[i] >= 1)
        {
            if (IS_SET(spell_info[scroll->obj_flags.value[i]].targets, TAR_VIOLENT) &&
                check_peaceful(ch, "Impolite magic is banned here."))
                continue;
            ((*spell_info[scroll->obj_flags.value[i]].spell_pointer)((char)scroll->obj_flags.value[0], ch, "",
                                                                     SPELL_TYPE_SCROLL, victim, obj));
        }
    if (equipped)
        scroll = unequip_char(ch, HOLD);
    extract_obj(scroll);
}

void do_use(struct char_data *ch, const char *argument, int cmd)
{
    char buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    struct char_data *tmp_char = NULL;
    struct obj_data *tmp_object = NULL;
    struct obj_data *stick = NULL;
    int bits = 0;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    argument = one_argument(argument, buf);

    if (ch->equipment[HOLD] == 0 || !isname(buf, ch->equipment[HOLD]->name))
    {
        act("You do not hold that item in your hand.", FALSE, ch, 0, 0, TO_CHAR);
        return;
    }
    stick = ch->equipment[HOLD];

    if (stick->obj_flags.type_flag == ITEM_STAFF)
    {
        act("$n taps $p three times on the ground.", TRUE, ch, stick, 0, TO_ROOM);
        act("You tap $p three times on the ground.", FALSE, ch, stick, 0, TO_CHAR);
        if (stick->obj_flags.value[2] > 0)
        { /* Is there any charges left? */
            stick->obj_flags.value[2]--;
            ((*spell_info[stick->obj_flags.value[3]].spell_pointer)((char)stick->obj_flags.value[0], ch, "",
                                                                    SPELL_TYPE_STAFF, 0, 0));
            WAIT_STATE(ch, PULSE_VIOLENCE);
        }
        else
        {
            cprintf(ch, "The staff seems powerless.\r\n");
        }
    }
    else if (stick->obj_flags.type_flag == ITEM_WAND)
    {
        bits = generic_find(argument, FIND_CHAR_ROOM | FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP, ch, &tmp_char,
                            &tmp_object);
        if (bits)
        {
            struct spell_info_type *spellp;

            spellp = spell_info + (stick->obj_flags.value[3]);

            if (bits == FIND_CHAR_ROOM)
            {
                act("$n point $p at $N.", TRUE, ch, stick, tmp_char, TO_ROOM);
                act("You point $p at $N.", FALSE, ch, stick, tmp_char, TO_CHAR);
            }
            else
            {
                act("$n point $p at $P.", TRUE, ch, stick, tmp_object, TO_ROOM);
                act("You point $p at $P.", FALSE, ch, stick, tmp_object, TO_CHAR);
            }

            if (IS_SET(spellp->targets, TAR_VIOLENT) && check_peaceful(ch, "Impolite magic is banned here."))
                return;

            if (stick->obj_flags.value[2] > 0)
            { /* Is there any charges left? */
                stick->obj_flags.value[2]--;
                ((*spellp->spell_pointer)((char)stick->obj_flags.value[0], ch, "", SPELL_TYPE_WAND, tmp_char,
                                          tmp_object));
                WAIT_STATE(ch, PULSE_VIOLENCE);
            }
            else
            {
                cprintf(ch, "The wand seems powerless.\r\n");
            }
        }
        else
        {
            cprintf(ch, "What should the wand be pointed at?\r\n");
        }
    }
    else
    {
        cprintf(ch, "Use is normally only for wand's and staff's.\r\n");
    }
}

void do_plr_noshout(struct char_data *ch, const char *argument, int cmd)
{
    char buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (IS_NPC(ch))
        return;

    only_argument(argument, buf);

    if (!*buf)
    {
        if (IS_SET(ch->specials.act, PLR_DEAF))
        {
            cprintf(ch, "You can now hear shouts again.\r\n");
            REMOVE_BIT(ch->specials.act, PLR_DEAF);
        }
        else
        {
            cprintf(ch, "From now on, you won't hear shouts.\r\n");
            SET_BIT(ch->specials.act, PLR_DEAF);
        }
    }
    else
    {
        cprintf(ch, "Only the gods can shut up someone else. \r\n");
    }
}

void do_plr_notell(struct char_data *ch, const char *argument, int cmd)
{
    char buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (IS_NPC(ch))
        return;
    only_argument(argument, buf);

    if (IS_SET(ch->specials.new_act, NEW_PLR_NOTELL))
    {
        cprintf(ch, "You can now hear tells again.\r\n");
        REMOVE_BIT(ch->specials.new_act, NEW_PLR_NOTELL);
    }
    else
    {
        cprintf(ch, "From now on, you won't hear tells.\r\n");
        SET_BIT(ch->specials.new_act, NEW_PLR_NOTELL);
    }
}

void do_plr_nosummon(struct char_data *ch, const char *argument, int cmd)
{
    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (IS_NPC(ch))
        return;

    if (IS_SET(ch->specials.new_act, NEW_PLR_KILLOK))
    {
        cprintf(ch, "Sorry, registered servants of the Dread Lord cannot hide!\r\n");
        return;
    }
    if (IS_SET(ch->specials.new_act, NEW_PLR_SUMMON))
    {
        cprintf(ch, "You can now be summoned.\r\n");
        REMOVE_BIT(ch->specials.new_act, NEW_PLR_SUMMON);
    }
    else
    {
        cprintf(ch, "From now on, you can't be summoned.\r\n");
        SET_BIT(ch->specials.new_act, NEW_PLR_SUMMON);
    }
}

void do_plr_noteleport(struct char_data *ch, const char *argument, int cmd)
{
    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (IS_NPC(ch))
        return;

    if (IS_SET(ch->specials.new_act, NEW_PLR_KILLOK))
    {
        cprintf(ch, "Sorry, registered servants of the Dread Lord cannot cower!\r\n");
        return;
    }
    if (IS_SET(ch->specials.new_act, NEW_PLR_TELEPORT))
    {
        cprintf(ch, "You can now be teleported.\r\n");
        REMOVE_BIT(ch->specials.new_act, NEW_PLR_TELEPORT);
    }
    else
    {
        cprintf(ch, "From now on, you can't be teleported.\r\n");
        SET_BIT(ch->specials.new_act, NEW_PLR_TELEPORT);
    }
}

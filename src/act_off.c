/*
 * file: act.offensive.c , Implementation of commands.    Part of DIKUMUD
 * Usage : Offensive commands.
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
#include "opinion.h"
#include "multiclass.h"
#include "spec_procs.h"
#include "fight.h"
#include "act_skills.h"
#include "act_move.h"
#include "spell_parser.h"
#include "act_info.h"
#include "breath_weapons.h"
#define _ACT_OFF_C
#include "act_off.h"

bfuncp bweapons[] = {cast_geyser,       cast_fire_breath, cast_gas_breath,
                    cast_frost_breath, cast_acid_breath, cast_lightning_breath};

int do_swat(struct char_data *ch, const char *argument, int cmd)
{
    struct char_data *vict = NULL;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (IS_NPC(ch))
        return TRUE;

    for (; isspace(*argument); argument++)
        ;
    if (!*argument)
    {
        cprintf(ch, "You must say who you want to switch to!\r\n");
        return TRUE;
    }
    vict = get_char_room_vis(ch, argument);

    if (vict == ch->specials.fighting)
    {
        cprintf(ch, "You are already fighting them!\r\n");
        return TRUE;
    }
    if (vict == NULL)
    {
        cprintf(ch, "I dont see them here?\r\n");
        return TRUE;
    }
    stop_fighting(ch);
    do_hit(ch, argument, 0);
    return TRUE;
}

int do_hit(struct char_data *ch, const char *argument, int cmd)
{
    char arg[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    struct char_data *victim = NULL;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (check_peaceful(ch, "This is a place of peace.\r\n"))
        return TRUE;

    only_argument(argument, arg);

    if (*arg)
    {
        victim = get_char_room_vis(ch, arg);
        if (victim)
        {
            if (victim == ch)
            {
                cprintf(ch, "You hit yourself..OUCH!.\r\n");
                act("$n hits $mself, and says OUCH!", FALSE, ch, 0, victim, TO_ROOM);
                return TRUE;
            }
            if (!CheckKill(ch, victim))
                return TRUE;

            if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == victim))
            {
                act("$N is your master! , you simply can't hit $M.", FALSE, ch, 0, victim, TO_CHAR);
                return TRUE;
            }
            if ((GET_POS(ch) == POSITION_STANDING || GET_POS(ch) == POSITION_MOUNTED) &&
                (victim != ch->specials.fighting))
            {
                hit(ch, victim, TYPE_UNDEFINED);
                WAIT_STATE(ch, PULSE_VIOLENCE + 1);
            }
            else
                cprintf(ch, "You do the best you can!\r\n");
        }
        else
            cprintf(ch, "They aren't here.\r\n");
    }
    else
    {
        cprintf(ch, "Hit who?\r\n");
    }
    return TRUE;
}

int do_kill(struct char_data *ch, const char *argument, int cmd)
{
    static char arg[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    struct char_data *victim = NULL;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);
    /*
     * if ((GetMaxLevel(ch) < GREATER_GOD) || IS_NPC(ch))
     */
    if (IS_NPC(ch) || (cmd != 216) || (GetMaxLevel(ch) < GREATER_GOD))
    {
        do_hit(ch, argument, 0);
        return TRUE;
    }
    only_argument(argument, arg);
    if (!*arg)
    {
        cprintf(ch, "Kill who?\r\n");
    }
    else
    {
        if (!(victim = get_char_room_vis(ch, arg)))
            cprintf(ch, "They aren't here.\r\n");
        else if (ch == victim)
            cprintf(ch, "Self-inflicted wounds are not allowed\r\n");
        else if (GetMaxLevel(ch) < GetMaxLevel(victim))
        {
            do_hit(ch, argument, 0);
            return TRUE;
        }
        else
        {
            act("You hack $M to pieces! Blood flies everywhere!", FALSE, ch, 0, victim, TO_CHAR);
            act("$N hacks you into itty bitty pieces!", FALSE, victim, 0, ch, TO_CHAR);
            act("$n brutally hacks $N into itty bitty pieces", FALSE, ch, 0, victim, TO_NOTVICT);
            if (RIDDEN(victim))
            {
                FallOffMount(RIDDEN(victim), victim);
                Dismount(RIDDEN(victim), victim, POSITION_SITTING);
            }
            else if (MOUNTED(victim))
            {
                FallOffMount(victim, MOUNTED(victim));
                Dismount(victim, MOUNTED(victim), POSITION_SITTING);
            }
            raw_kill(victim);
        }
    }
    return TRUE;
}

int do_backstab(struct char_data *ch, const char *argument, int cmd)
{
    struct char_data *victim = NULL;
    char name[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    int percent_chance = 0;
    int base = 0;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (check_peaceful(ch, "This is a place of peace.\r\n"))
        return TRUE;

    only_argument(argument, name);

    if (!(victim = get_char_room_vis(ch, name)))
    {
        cprintf(ch, "Backstab who?\r\n");
        return TRUE;
    }
    if (victim == ch)
    {
        cprintf(ch, "How can you sneak up on yourself?\r\n");
        return TRUE;
    }
    if (!ch->equipment[WIELD])
    {
        cprintf(ch, "You need to wield a one handed weapon, to make it a succes.\r\n");
        return TRUE;
    }
    if (MOUNTED(ch))
    {
        cprintf(ch, "Not while your mounted!\r\n");
        return TRUE;
    }
    if (MOUNTED(victim))
    {
        cprintf(ch, "Not while they are mounted!\r\n");
        return TRUE;
    }
    if (ch->attackers)
    {
        cprintf(ch, "There's no way to reach that back while you're fighting!\r\n");
        return TRUE;
    }
    if (!CheckKill(ch, victim))
        return TRUE;

    if (victim->attackers >= 3)
    {
        cprintf(ch, "You can't get close enough to them to backstab!\r\n");
        return TRUE;
    }
    if (ch->equipment[WIELD]->obj_flags.value[3] != 11 && ch->equipment[WIELD]->obj_flags.value[3] != 1 &&
        ch->equipment[WIELD]->obj_flags.value[3] != 10)
    {
        cprintf(ch, "Only piercing or stabbing weapons can be used for backstabbing.\r\n");
        return TRUE;
    }
    if (ch->specials.fighting)
    {
        cprintf(ch, "You're too busy to backstab\r\n");
        return TRUE;
    }
    if (victim->specials.fighting)
    {
        base = 0;
    }
    else
    {
        base = 4;
    }

    percent_chance = number(1, 101); /* 101% is a complete failure */

    if (ch->skills[SKILL_BACKSTAB].learned)
    {
        if (percent_chance > ch->skills[SKILL_BACKSTAB].learned)
        {
            if (AWAKE(victim))
            {
                damage(ch, victim, 0, SKILL_BACKSTAB);
                AddHated(victim, ch);
            }
            else
            {
                base += 2;
                GET_HITROLL(ch) += base;
                hit(ch, victim, SKILL_BACKSTAB);
                GET_HITROLL(ch) -= base;
                AddHated(victim, ch);
            }
        }
        else
        {
            GET_HITROLL(ch) += base;
            hit(ch, victim, SKILL_BACKSTAB);
            GET_HITROLL(ch) -= base;
            AddHated(victim, ch);
            if (ch->skills[SKILL_BACKSTAB].learned < 50)
                ch->skills[SKILL_BACKSTAB].learned += 2;
        }
    }
    else
    {
        damage(ch, victim, 0, SKILL_BACKSTAB);
        AddHated(victim, ch);
    }
    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    return TRUE;
}

int do_order(struct char_data *ch, const char *argument, int cmd)
{
    char name[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char message[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char message2[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char action[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char onwho[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    int found = FALSE;
    int org_room = 0;
    struct char_data *victim = NULL;
    struct char_data *onwho_ptr = NULL;
    struct follow_type *k = NULL;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    half_chop(argument, name, message);

    strlcpy(message2, message, MAX_INPUT_LENGTH);

    if (!*name || !*message)
        cprintf(ch, "Order who to do what?\r\n");
    else if (!(victim = get_char_room_vis(ch, name)) && str_cmp("part", name) && str_cmp("party", name) &&
             str_cmp("follower", name) && str_cmp("followers", name))
        cprintf(ch, "That person isn't here.\r\n");
    else if (ch == victim)
        cprintf(ch, "You order yourself, very good.\r\n");
    else
    {
        if (IS_AFFECTED(ch, AFF_CHARM))
        {
            cprintf(ch, "Your superior would not aprove of you giving orders.\r\n");
            return TRUE;
        }
        argument_interpreter(message2, action, onwho);

        /* Buggy as hell... this should be redone. */

        if (is_abbrev("kill", action) || is_abbrev("bash", action) || is_abbrev("kick", action) ||
            is_abbrev("punch", action) || is_abbrev("swat", action) || is_abbrev("backstab", action) ||
            is_abbrev("punch", action) || is_abbrev("disarm", action) || is_abbrev("steal", action) ||
            is_abbrev("hit", action))
        {
            if ((onwho_ptr = get_char_room_vis(ch, onwho)))
            {
                if (!CheckKill(ch, onwho_ptr))
                {
                    cprintf(ch, "You can't order other to do your dirty work.\r\n");
                    return TRUE;
                }
            }
        }
        if (victim)
        {
            act("$N orders you to '%s'", FALSE, victim, 0, ch, TO_CHAR, message);
            act("$n gives $N an order.", FALSE, ch, 0, victim, TO_ROOM);

            if (!(IS_IMMORTAL(ch) && !IS_IMMORTAL(victim)) &&
                ((victim->master != ch) || !IS_AFFECTED(victim, AFF_CHARM)))
                act("$n has an indifferent look.", FALSE, victim, 0, 0, TO_ROOM);
            else
            {
                cprintf(ch, "Ok.\r\n");
                command_interpreter(victim, message);
            }
        }
        else
        { /* This is order "followers" */
            act("$n issues the order '%s'.", FALSE, ch, 0, victim, TO_ROOM, message);

            org_room = ch->in_room;

            for (k = ch->followers; k; k = k->next)
            {
                if (org_room == k->follower->in_room)
                    if (IS_AFFECTED(k->follower, AFF_CHARM))
                    {
                        found = TRUE;
                        command_interpreter(k->follower, message);
                    }
            }
            if (found)
                cprintf(ch, "Ok.\r\n");
            else
                cprintf(ch, "Nobody here is a loyal subject of yours!\r\n");
        }
    }
    return TRUE;
}

int do_flee(struct char_data *ch, const char *argument, int cmd)
{
    int i = 0;
    int attempt = 0;
    int loose = 0;
    int flee_dir = 0;
    int percent_chance = 0;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (IS_IMMORTAL(ch)) /* gods should never flee! */
        return TRUE;
    if (IS_AFFECTED(ch, AFF_PARALYSIS))
        return TRUE;

    if (MOUNTED(ch))
    {
        FallOffMount(ch, MOUNTED(ch));
        Dismount(ch, MOUNTED(ch), POSITION_SITTING);
    }
    if ((GET_POS(ch) <= POSITION_SITTING) && (GET_POS(ch) > POSITION_STUNNED) && !IsAvian(ch))
    {
        act("$n scrambles madly to $s feet!", TRUE, ch, 0, 0, TO_ROOM);
        act("Panic-stricken, you scramble to your feet.", TRUE, ch, 0, 0, TO_CHAR);
        GET_POS(ch) = POSITION_STANDING;
        WAIT_STATE(ch, PULSE_VIOLENCE);
        return TRUE;
    }
    if (!(ch->specials.fighting))
    {
        for (i = 0; i < MAX_NUM_EXITS; i++)
        {
            attempt = number(0, 5); /* Select a random direction */
            if (CAN_GO(ch, attempt) && !IS_SET(real_roomp(EXIT(ch, attempt)->to_room)->room_flags, DEATH))
            {
                act("$n panics, and attempts to flee.", TRUE, ch, 0, 0, TO_ROOM);
                if ((flee_dir = MoveOne(ch, attempt)) == 1)
                {
                    /*
                     * The escape has succeded
                     */
                    cprintf(ch, "You flee head over heels.\r\n");
                    return TRUE;
                }
                else
                {
                    if (!flee_dir)
                        act("$n tries to flee, but is too exhausted!", TRUE, ch, 0, 0, TO_ROOM);
                    return TRUE;
                }
            }
        } /* for */
        /*
         * No exits was found
         */
        cprintf(ch, "PANIC! You couldn't escape!\r\n");
        return TRUE;
    }
    for (i = 0; i < MAX_NUM_EXITS; i++)
    {
        attempt = number(0, MAX_NUM_EXITS - 1); /* Select a random direction */
        if (CAN_GO(ch, attempt) && !IS_SET(real_roomp(EXIT(ch, attempt)->to_room)->room_flags, DEATH))
        {
            act("$n panics, and attempts to flee.", TRUE, ch, 0, 0, TO_ROOM);
            if ((flee_dir = MoveOne(ch, attempt)) == 1)
            {
                /*
                 * The escape has succeded. We'll be nice.
                 */
                if (GetMaxLevel(ch) >= 2)
                {
                    loose = GetMaxLevel(ch) + (GetSecMaxLev(ch) / 2) + (GetThirdMaxLev(ch) / 3);
                    loose -= GetMaxLevel(ch->specials.fighting) + (GetSecMaxLev(ch->specials.fighting) / 2) +
                             (GetThirdMaxLev(ch->specials.fighting) / 3);
                    loose *= GetMaxLevel(ch);
                }
                else
                {
                    loose = 0;
                }

                if (loose < 0)
                    loose = -1;
                if (IS_NPC(ch))
                {
                    AddFeared(ch, ch->specials.fighting);
                }
                else
                {
                    percent_chance =
                        (int)100 * (float)GET_HIT(ch->specials.fighting) / (float)GET_MAX_HIT(ch->specials.fighting);

                    if (number(1, 101) < percent_chance)
                    {
                        if ((DoesHate(ch->specials.fighting, ch)) ||
                            ((IS_GOOD(ch) && (IS_EVIL(ch->specials.fighting))) ||
                             (IS_EVIL(ch) && (IS_GOOD(ch->specials.fighting)))))
                        {
                            SetHunting(ch->specials.fighting, ch);
                        }
                    }
                }

                if (!IS_NPC(ch))
                    gain_exp(ch, -loose);

                cprintf(ch, "You flee head over heels.\r\n");
                if (ch->specials.fighting->specials.fighting == ch)
                    stop_fighting(ch->specials.fighting);
                if (ch->specials.fighting)
                    stop_fighting(ch);
                return TRUE;
            }
            else
            {
                if (!flee_dir)
                    act("$n tries to flee, but is too exhausted!", TRUE, ch, 0, 0, TO_ROOM);
                return TRUE;
            }
        }
    } /* for */

    /*
     * No exits were found
     */
    cprintf(ch, "PANIC! You couldn't escape!\r\n");
    return TRUE;
}

int do_bandage(struct char_data *ch, const char *argument, int cmd)
{
    struct char_data *victim = NULL;
    char name[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    int percent_chance = 0;
    int cost = 0;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    only_argument(argument, name);

    if (!(victim = get_char_room_vis(ch, name)))
    {
        cprintf(ch, "Bandage who?\r\n");
        return TRUE;
    }
    if (victim == ch)
    {
        cprintf(ch, "Aren't we funny today...\r\n");
        return TRUE;
    }
    if (MOUNTED(ch))
    {
        cprintf(ch, "You can't reach them from your mounts back!\r\n");
        return TRUE;
    }
    if (ch->attackers > 3)
    {
        cprintf(ch, "There's no room to use Bandage!\r\n");
        return TRUE;
    }
    if (victim->attackers >= 4)
    {
        cprintf(ch, "You can't get close enough to them to use bandage!\r\n");
        return TRUE;
    }
    if (GET_HIT(victim) > 0)
    {
        cprintf(ch, "You don't need to do this, they look stable.\r\n");
        return TRUE;
    }
    cost = 10 - (GetMaxLevel(ch) / 10);

    if (GET_MANA(ch) < cost)
    {
        cprintf(ch, "You can't seem to concentrate enough to bandage!\r\n");
        return TRUE;
    }
    act("$n quickly bandages $N.", FALSE, ch, 0, victim, TO_NOTVICT);

    percent_chance = number(1, 101); /* 101% is a complete failure */

    /*
     * some modifications to account for dexterity, and level
     */

    if (percent_chance > ch->skills[SKILL_BANDAGE].learned)
    {
        if (GET_HIT(victim) <= 0)
            GET_HIT(victim) += 1;
        update_pos(victim);
        cprintf(ch, "They are still in need of help....\r\n");
        GET_MANA(ch) -= cost / 2;
        act("$N is trying to save you, bandages have been put on your wounds!", FALSE, victim, 0, ch, TO_CHAR);
    }
    else
    {
        if (GET_HIT(victim) <= 0)
            GET_HIT(victim) = 1;
        update_pos(victim);
        cprintf(ch, "They should live...\r\n");
        GET_MANA(ch) -= cost;
        act("$N has saved you, bandages have been put on your wounds!", FALSE, victim, 0, ch, TO_CHAR);
    }
    return TRUE;
}

void slam_into_wall(struct char_data *ch, struct room_direction_data *exitp)
{
    char doorname[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG > 2)
        log_info("called %s with %s, %08zx", __PRETTY_FUNCTION__, SAFE_NAME(ch), (size_t)exitp);

    if (exitp->keyword && *exitp->keyword)
    {
        if ((strcmp(fname(exitp->keyword), "secret") == 0) || (IS_SET(exitp->exit_info, EX_SECRET)))
        {
            strlcpy(doorname, "wall", MAX_INPUT_LENGTH);
        }
        else
        {
            strlcpy(doorname, fname(exitp->keyword), MAX_INPUT_LENGTH);
        }
    }
    else
    {
        strlcpy(doorname, "barrier", MAX_INPUT_LENGTH);
    }

    cprintf(ch, "You slam your body against the %s with no effect\r\n", doorname);
    act("$n slams against the %s.\r\n", FALSE, ch, 0, 0, TO_ROOM, doorname);

    GET_HIT(ch) -= number(1, 4);

    if (GET_HIT(ch) < 0)
        GET_HIT(ch) = 0;
    GET_POS(ch) = POSITION_STUNNED;
    update_pos(ch);
}

int do_doorbash(struct char_data *ch, const char *argument, int cmd)
{
    int dir = -1;
    int ok = 0;
    struct room_direction_data *exitp = NULL;
    int was_in = FALSE;
    int roll = 0;
    char type[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char direction[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (GET_MOVE(ch) < 10)
    {
        cprintf(ch, "You're too tired to do that\r\n");
        return TRUE;
    }
    if (MOUNTED(ch))
    {
        cprintf(ch, "You can't bash a door from the back of a mount? fool.\r\n");
        return TRUE;
    }
    /*
     * make sure that the argument is a direction, or a keyword.
     */

    for (; *argument == ' '; argument++)
        ;

    argument_interpreter(argument, type, direction);

    if ((dir = find_door(ch, type, direction)) >= 0)
    {
        ok = TRUE;
    }
    else
    {
        act("$n looks around, bewildered.", FALSE, ch, 0, 0, TO_ROOM);
        return TRUE;
    }

    if (!ok)
    {
        cprintf(ch, "Error in doorbash please report\r\n");
        return TRUE;
    }
    exitp = EXIT(ch, dir);

    if (!exitp)
    {
        cprintf(ch, "Error in doorbash please report\r\n");
        return TRUE;
    }
    if (dir == 5)
    {
        if (real_roomp(exitp->to_room)->sector_type == SECT_AIR && !IS_AFFECTED(ch, AFF_FLYING))
        {
            cprintf(ch, "You have no way of getting there!\r\n");
            return TRUE;
        }
    }
    act("$n flings their body %swards", FALSE, ch, 0, 0, TO_ROOM, dirs[dir]);
    cprintf(ch, "You fling your body %swards\r\n", dirs[dir]);

    if (IS_NOT_SET(exitp->exit_info, EX_CLOSED))
    {
        was_in = ch->in_room;
        char_from_room(ch);
        char_to_room(ch, exitp->to_room);
        do_look(ch, "", 0);

        DisplayMove(ch, dir, was_in, 1);
        /*    if(!check_falling(ch))
         * {
         * if (IS_SET(RM_FLAGS(ch->in_room), DEATH) && GetMaxLevel(ch) < LOW_IMMORTAL)
         * {
         * NailThisSucker(ch);
         * return TRUE;
         * }
         * else
         * {
         * WAIT_STATE(ch, PULSE_VIOLENCE*3);
         * GET_MOVE(ch) -= 10;
         * }
         * }
         */
        WAIT_STATE(ch, PULSE_VIOLENCE * 3);
        GET_MOVE(ch) -= 10;

        return TRUE;
    }
    GET_MOVE(ch) -= 10;

    if (IS_SET(exitp->exit_info, EX_LOCKED) && IS_SET(exitp->exit_info, EX_PICKPROOF))
    {
        slam_into_wall(ch, exitp);
        return TRUE;
    }
    /*
     * now we've checked for failures, time to check for success;
     */

    if (ch->skills[SKILL_DOOR_BASH].learned)
    {
        roll = number(1, 100);
        if (roll > ch->skills[SKILL_DOOR_BASH].learned)
        {
            slam_into_wall(ch, exitp);
        }
        else
        {
            /*
             * unlock and open the door
             */
            act("$n slams into the %s, and smashes it down", FALSE, ch, 0, 0, TO_ROOM, fname(exitp->keyword));
            cprintf(ch, "You slam into the %s, and smashes open!\r\n", fname(exitp->keyword));
            raw_unlock_door(ch, exitp, dir);
            raw_open_door(ch, dir);
            /*
             * Now a dex check to keep from flying into the next room
             */
            roll = number(1, 20);
            if (roll > GET_DEX(ch))
            {
                was_in = ch->in_room;
                char_from_room(ch);
                char_to_room(ch, exitp->to_room);
                do_look(ch, "", 0);
                DisplayMove(ch, dir, was_in, 1);
                if (!check_falling(ch))
                {
                    if (IS_SET(RM_FLAGS(ch->in_room), DEATH) && GetMaxLevel(ch) < LOW_IMMORTAL)
                    {
                        return TRUE;
                    }
                }
                else
                {
                    return TRUE;
                }
                WAIT_STATE(ch, PULSE_VIOLENCE * 3);
                GET_MOVE(ch) -= 5;
                return TRUE;
            }
            else
            {
                WAIT_STATE(ch, PULSE_VIOLENCE * 1);
                GET_MOVE(ch) -= 2;
                return TRUE;
            }
        }
    }
    else
    {
        cprintf(ch, "You do not know this well enough yet.\r\n");
        slam_into_wall(ch, exitp);
    }
    return TRUE;
}

int do_bash(struct char_data *ch, const char *argument, int cmd)
{
    struct char_data *victim = NULL;
    char name[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    int percent_chance = 0;
    int cost = 0;
    int has_shield = FALSE;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (check_peaceful(ch, "This is a place of peace.\r\n"))
        return TRUE;

    only_argument(argument, name);

    if (!(victim = get_char_room_vis(ch, name)))
    {
        if (ch->specials.fighting)
        {
            victim = ch->specials.fighting;
        }
        else
        {
            cprintf(ch, "Bash who?\r\n");
            return TRUE;
        }
    }
    if (victim == ch)
    {
        cprintf(ch, "Aren't we funny today...\r\n");
        return TRUE;
    }
    if (MOUNTED(ch))
    {
        cprintf(ch, "You can't bash while mounted!\r\n");
        return TRUE;
    }
    if (!CheckKill(ch, victim))
        return TRUE;

    has_shield = 0;
    if (ch->equipment[WEAR_SHIELD])
        has_shield = 1;

    if (ch->attackers > 3)
    {
        cprintf(ch, "There's no room to bash!\r\n");
        return TRUE;
    }
    if (victim->attackers >= 4)
    {
        cprintf(ch, "You can't get close enough to them to bash!\r\n");
        return TRUE;
    }
    cost = 10 - (GET_LEVEL(ch, BestFightingClass(ch)) / 10);

    if (GET_MANA(ch) < cost)
    {
        cprintf(ch, "You can't seem to concentrate enought to bash!\r\n");
        return TRUE;
    }
    GET_MANA(ch) -= cost;

    percent_chance = number(1, 101); /* 101% is a complete failure */

    /*
     * some modifications to account for dexterity, and level
     */

    percent_chance -= dex_app[(int)GET_DEX(ch)].reaction;
    percent_chance += dex_app[(int)GET_DEX(victim)].reaction;

    if (MOUNTED(victim))
    {
        if (IS_NPC(victim))
            percent_chance += GetMaxLevel(victim);
        else
            percent_chance += ((ch->skills[SKILL_RIDE].learned) / 10);
    }
    if (percent_chance > ch->skills[SKILL_BASH].learned)
    {
        if (GET_POS(victim) > POSITION_DEAD)
        {
            if (has_shield)
                damage(ch, victim, 0, SKILL_BASH_W_SHIELD);
            else
                damage(ch, victim, 0, SKILL_BASH);
            GET_POS(ch) = POSITION_SITTING;
        }
    }
    else
    {
        if (GET_POS(victim) > POSITION_DEAD)
        {
            if (has_shield)
                damage(ch, victim, 1, SKILL_BASH_W_SHIELD);
            else
                damage(ch, victim, 1, SKILL_BASH);
            if (ch->skills[SKILL_BASH].learned < 50)
                ch->skills[SKILL_BASH].learned += 2;

            if (MOUNTED(victim))
            {
                FallOffMount(victim, MOUNTED(victim));
                Dismount(victim, MOUNTED(victim), POSITION_SITTING);
                WAIT_STATE(victim, PULSE_VIOLENCE * 3);
            }
            else
            {
                WAIT_STATE(victim, PULSE_VIOLENCE * 2);
                GET_POS(victim) = POSITION_SITTING;
            }
        }
    }
    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    return TRUE;
}

int do_punch(struct char_data *ch, const char *argument, int cmd)
{
    struct char_data *victim = NULL;
    char name[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    int percent_chance = 0;
    int dam = 0;
    int cost = 0;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (check_peaceful(ch, "This is a place of peace.\r\n"))
        return TRUE;

    only_argument(argument, name);

    if (!(victim = get_char_room_vis(ch, name)))
    {
        if (ch->specials.fighting)
        {
            victim = ch->specials.fighting;
        }
        else
        {
            cprintf(ch, "Punch who?\r\n");
            return TRUE;
        }
    }
    if (victim == ch)
    {
        cprintf(ch, "Aren't we funny today...\r\n");
        return TRUE;
    }
    if (MOUNTED(ch))
    {
        cprintf(ch, "You can't punch while mounted!\r\n");
        return TRUE;
    }
    if (!CheckKill(ch, victim))
        return TRUE;

    if (ch->attackers > 3)
    {
        cprintf(ch, "There's no room to punch!\r\n");
        return TRUE;
    }
    if (victim->attackers >= 3)
    {
        cprintf(ch, "You can't get close enough to them to punch!\r\n");
        return TRUE;
    }
    if (ch->equipment[WIELD_TWOH])
    {
        cprintf(ch, "You can't do this while wielding two handed!\r\n");
        return TRUE;
    }
    cost = 20 - (GET_LEVEL(ch, BestFightingClass(ch)) / 6);

    if (GET_MANA(ch) < cost)
    {
        cprintf(ch, "You can't seem to concentrate enough to punch!\r\n");
        return TRUE;
    }
    GET_MANA(ch) -= cost;

    percent_chance = ((10 - (GET_AC(victim) / 10)) << 1) + number(1, 101);

    /*
     * some modifications to account for dexterity,
     */

    dam = dice(0, 4) + str_app[STRENGTH_APPLY_INDEX(ch)].todam;

    if (percent_chance < ch->skills[SKILL_BARE_HAND].learned)
        dam += 2;

    if (percent_chance > ch->skills[SKILL_PUNCH].learned)
    {
        if (GET_POS(victim) > POSITION_DEAD)
        {
            damage(ch, victim, 0, SKILL_PUNCH);
        }
    }
    else
    {
        if (GET_POS(victim) > POSITION_DEAD)
        {
            damage(ch, victim, dam, SKILL_PUNCH);
            if (ch->skills[SKILL_PUNCH].learned < 50)
                ch->skills[SKILL_PUNCH].learned += 2;
            if (dam > 8)
            {
                if (MOUNTED(victim))
                {
                    FallOffMount(victim, MOUNTED(victim));
                    Dismount(victim, MOUNTED(victim), POSITION_SITTING);
                    WAIT_STATE(victim, PULSE_VIOLENCE * 2);
                }
                else
                {
                    GET_POS(victim) = POSITION_SITTING;
                    WAIT_STATE(victim, PULSE_VIOLENCE);
                }
            }
        }
    }
    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    return TRUE;
}

int do_rescue(struct char_data *ch, const char *argument, int cmd)
{
    struct char_data *victim = NULL;
    struct char_data *tmp_ch = NULL;
    int percent_chance = 0;
    int cost = 0;
    char victim_name[240] = "\0\0\0\0\0\0\0";

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (check_peaceful(ch, "No one should need rescuing here.\r\n"))
        return TRUE;

    only_argument(argument, victim_name);

    if (!(victim = get_char_room_vis(ch, victim_name)))
    {
        cprintf(ch, "Who do you want to rescue?\r\n");
        return TRUE;
    }
    if (victim == ch)
    {
        cprintf(ch, "What about fleeing instead?\r\n");
        return TRUE;
    }
    if (ch->specials.fighting == victim)
    {
        cprintf(ch, "How can you rescue someone you are trying to kill?\r\n");
        return TRUE;
    }
    if (MOUNTED(ch))
    {
        cprintf(ch, "You can't rescue while mounted!\r\n");
        return TRUE;
    }
    if (victim->attackers >= 4)
    {
        cprintf(ch, "You can't get close enough to them to rescue!\r\n");
        return TRUE;
    }
    cost = 15 - (GET_LEVEL(ch, BestFightingClass(ch)) / 10);

    if (GET_MANA(ch) < cost)
    {
        cprintf(ch, "You trip while trying to rescue!\r\n");
        return TRUE;
    }
    GET_MANA(ch) -= cost;

    for (tmp_ch = real_roomp(ch->in_room)->people; tmp_ch && (tmp_ch->specials.fighting != victim);
         tmp_ch = tmp_ch->next_in_room)
        ;

    if (!tmp_ch)
    {
        act("But nobody is fighting $M?", FALSE, ch, 0, victim, TO_CHAR);
        return TRUE;
    }
    if (!HasClass(ch, CLASS_WARRIOR) && !HasClass(ch, CLASS_RANGER))
        cprintf(ch, "But only true warriors can do this!");
    else
    {
        percent_chance = number(1, 101); /* 101% is a complete failure */
        if (percent_chance > ch->skills[SKILL_RESCUE].learned)
        {
            cprintf(ch, "You fail the rescue.\r\n");
            return TRUE;
        }
        cprintf(ch, "Huzzay! To the rescue...\r\n");
        act("You are rescued by $N, you are confused!", FALSE, victim, 0, ch, TO_CHAR);
        act("$n heroically rescues $N.", FALSE, ch, 0, victim, TO_NOTVICT);
        if (victim->specials.fighting == tmp_ch)
            stop_fighting(victim);
        if (tmp_ch->specials.fighting)
            stop_fighting(tmp_ch);
        if (ch->specials.fighting)
            stop_fighting(ch);
        set_fighting(ch, tmp_ch);
        set_fighting(tmp_ch, ch);
        WAIT_STATE(victim, 2 * PULSE_VIOLENCE);
        if (ch->skills[SKILL_RESCUE].learned < 50)
            ch->skills[SKILL_RESCUE].learned += 2;
    }
    return TRUE;
}

int do_assist(struct char_data *ch, const char *argument, int cmd)
{
    struct char_data *victim = NULL;
    struct char_data *tmp_ch = NULL;
    char victim_name[240] = "\0\0\0\0\0\0\0";

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (check_peaceful(ch, "This is a place of peace.\r\n"))
        return TRUE;

    only_argument(argument, victim_name);

    if (!(victim = get_char_room_vis(ch, victim_name)))
    {
        cprintf(ch, "Who do you want to assist?\r\n");
        return TRUE;
    }
    if (victim == ch)
    {
        cprintf(ch, "Oh, by all means, help yourself...\r\n");
        return TRUE;
    }
    if (ch->specials.fighting == victim)
    {
        cprintf(ch, "That would be very confusing!\r\n");
        return TRUE;
    }
    if (ch->specials.fighting)
    {
        cprintf(ch, "You have your hands full right now\r\n");
        return TRUE;
    }
    if (victim->attackers >= 4)
    {
        cprintf(ch, "You can't get close enough to them to assist!\r\n");
        return TRUE;
    }
    tmp_ch = victim->specials.fighting;

    if (!tmp_ch)
    {
        act("But he's not fighting anyone.", FALSE, ch, 0, victim, TO_CHAR);
        return TRUE;
    }
    hit(ch, tmp_ch, TYPE_UNDEFINED);
    WAIT_STATE(victim, PULSE_VIOLENCE + 2); /* same as hit */
    return TRUE;
}

int do_kick(struct char_data *ch, const char *argument, int cmd)
{
    struct char_data *victim = NULL;
    char name[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    int percent_chance = 0;
    int cost = 0;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (check_peaceful(ch, "This is a place of peace.\r\n"))
        return TRUE;

    only_argument(argument, name);

    if (!(victim = get_char_room_vis(ch, name)))
    {
        if (ch->specials.fighting)
        {
            victim = ch->specials.fighting;
        }
        else
        {
            cprintf(ch, "Kick who?\r\n");
            return TRUE;
        }
    }
    if (victim == ch)
    {
        cprintf(ch, "Aren't we funny today...\r\n");
        return TRUE;
    }
    if (MOUNTED(ch))
    {
        cprintf(ch, "You can't kick while mounted!\r\n");
        return TRUE;
    }
    if (MOUNTED(victim))
    {
        cprintf(ch, "You've been watching to much kung-fu!\r\n");
        return TRUE;
    }
    if (!CheckKill(ch, victim))
        return TRUE;

    if (ch->attackers > 3)
    {
        cprintf(ch, "There's no room to kick!\r\n");
        return TRUE;
    }
    if (victim->attackers >= 3)
    {
        cprintf(ch, "You can't get close enough to them to kick!\r\n");
        return TRUE;
    }
    cost = 10 - (GET_LEVEL(ch, BestFightingClass(ch)) / 10);

    if (GET_MANA(ch) < cost)
    {
        cprintf(ch, "You trip while trying to kick!\r\n");
        return TRUE;
    }
    GET_MANA(ch) -= cost;

    percent_chance = ((10 - (GET_AC(victim) / 10)) << 1) + number(1, 101);
    /*
     * 101% is a complete failure
     */

    if (percent_chance > ch->skills[SKILL_KICK].learned)
    {
        if (GET_POS(victim) > POSITION_DEAD)
            damage(ch, victim, 0, SKILL_KICK);
    }
    else
    {
        if (GET_POS(victim) > POSITION_DEAD)
        {
            /*
             * damage(ch, victim, GET_LEVEL(ch, BestFightingClass(ch))>>1, SKILL_KICK);
             */
            damage(ch, victim, dice(2, 8), SKILL_KICK);
            if (ch->skills[SKILL_KICK].learned < 50)
                ch->skills[SKILL_KICK].learned += 2;
        }
    }
    WAIT_STATE(ch, PULSE_VIOLENCE * 3);
    return TRUE;
}

int do_wimp(struct char_data *ch, const char *argument, int cmd)
{
    /*
     * sets the character in wimpy mode.
     */
    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (IS_SET(ch->specials.act, PLR_WIMPY))
    {
        REMOVE_BIT(ch->specials.act, PLR_WIMPY);
        cprintf(ch, "You are no longer a wimp.\r\n");
    }
    else
    {
        if (GetMaxLevel(ch) < 4)
        {
            SET_BIT(ch->specials.act, PLR_WIMPY);
            cprintf(ch, "You are now a wimp.\r\n");
        }
        else
            cprintf(ch, "You are an adult now, no need for wimpy mode.\r\n");
    }
    return TRUE;
}

int do_breath(struct char_data *ch, const char *argument, int cmd)
{
    struct char_data *victim = NULL;
    char name[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    int count = 0;
    int manacost = 0;
    bfuncp weapon = NULL;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (check_peaceful(ch, "That wouldn't be nice at all.\r\n"))
        return TRUE;

    only_argument(argument, name);

    for (count = FIRST_BREATH_WEAPON; count <= LAST_BREATH_WEAPON && !affected_by_spell(ch, count); count++)
        ;

    if (count > LAST_BREATH_WEAPON)
    {
        struct breather *scan;

        for (scan = breath_monsters; scan->vnum >= 0 && scan->vnum != mob_index[ch->nr].vnum; scan++)
            ;

        if (scan->vnum < 0)
        {
            cprintf(ch, "You don't have a breath weapon.\r\n");
            return TRUE;
        }
        for (count = 0; scan->breaths[count]; count++)
            ;

        if (count < 1)
        {
            log_info("monster %s has no breath weapons", ch->player.short_descr);
            cprintf(ch, "Why don't you have any breath weapons!?\r\n");
            return TRUE;
        }
        weapon = scan->breaths[dice(1, count) - 1];
        manacost = scan->cost;
        if (GET_MANA(ch) <= -3 * manacost)
        {
            weapon = NULL;
        }
    }
    else
    {
        manacost = 0;
        weapon = bweapons[count - FIRST_BREATH_WEAPON];
        affect_from_char(ch, count);
    }

    if (!(victim = get_char_room_vis(ch, name)))
    {
        if (ch->specials.fighting)
        {
            victim = ch->specials.fighting;
        }
        else
        {
            cprintf(ch, "Breath on who?\r\n");
            return TRUE;
        }
    }
    breath_weapon(ch, victim, manacost, weapon);

    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    return TRUE;
}

#if 0
int do_shoot(struct char_data *ch, const char *argument, int cmd)
{
    char                                    arg[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    struct char_data                       *victim = NULL;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (check_peaceful(ch, "You feel too peaceful to contemplate violence.\r\n"))
	return TRUE;

    only_argument(argument, arg);

    if (*arg) {
	victim = get_char_room_vis(ch, arg);
	if (victim) {
	    if (victim == ch) {
		cprintf(ch, "You can't shoot things at yourself!");
		return TRUE;
	    } else {
		if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == victim)) {
		    act("$N is just such a good friend, you simply can't shoot at $M.",
			FALSE, ch, 0, victim, TO_CHAR);
		    return TRUE;
		}
		shoot(ch, victim);
	    }
	} else {
	    cprintf(ch, "They aren't here.\r\n");
	}
    } else {
	cprintf(ch, "Shoot who?\r\n");
    }
    return TRUE;
}
#endif

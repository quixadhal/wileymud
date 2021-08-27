/*
 * file: act.movement.c , Implementation of commands      Part of DIKUMUD
 * Usage : Movement commands, close/open & lock/unlock doors.
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */

#include <stdio.h>
#include <stdlib.h>
/* #include <unistd.h> */
#include <sys/types.h>
#include <string.h>

#include "global.h"
#include "utils.h"
#include "bug.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "trap.h"
#include "whod.h"
#include "multiclass.h"
#include "fight.h"
#include "reception.h"
#include "magic_utils.h"
#include "spell_parser.h"
#include "act_info.h"
#include "act_skills.h"
#define _ACT_MOVE_C
#include "act_move.h"

/*
 * Some new movement commands for diku-mud.. a bit more object oriented.
 */

void NotLegalMove(struct char_data *ch)
{
    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

    cprintf(ch, "Alas, you cannot go that way...\r\n");
}

int check_exit_alias(struct char_data *ch, const char *argument)
{
    struct room_direction_data *exitp = NULL;
    int exit_index = -1;

#if 0
    return 0;
#endif

    if (DEBUG > 2)
        log_info("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument));

    for (exit_index = 0; exit_index < MAX_NUM_EXITS; exit_index++)
    {
        if ((exitp = EXIT(ch, exit_index)))
        {
            if (IS_SET(exitp->exit_info, EX_ALIAS))
                if (!strncmp(exitp->exit_alias, argument, strlen(exitp->exit_alias)))
                    return do_move(ch, "", exit_index + 1);
        }
    }
    return 0;
}

int ValidMove(struct char_data *ch, int cmd)
{
    struct room_direction_data *exitp = NULL;
    struct room_data *there = NULL;
    struct char_data *v = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd);

    exitp = EXIT(ch, cmd);

    if (!exit_ok(exitp, NULL))
    {
        NotLegalMove(ch);
        return FALSE;
    }
    there = real_roomp(exitp->to_room);

    if (MOUNTED(ch))
    {
        if (GET_POS(MOUNTED(ch)) < POSITION_FIGHTING)
        {
            cprintf(ch, "Your mount must be standing\r\n");
            return FALSE;
        }
        if (ch->in_room != MOUNTED(ch)->in_room)
        {
            Dismount(ch, MOUNTED(ch), POSITION_STANDING);
        }
    }
    if (IS_SET(exitp->exit_info, EX_CLOSED))
    {
        if (exitp->keyword)
        {
            if (IS_NOT_SET(exitp->exit_info, EX_SECRET) && (strcmp(fname(exitp->keyword), "secret")))
            {
                if (IS_IMMORTAL(ch))
                {
                    if (IS_SET(ch->specials.act, PLR_STEALTH))
                    {
                        if (!real_roomp(ch->in_room))
                            return FALSE;
                        for (v = real_roomp(ch->in_room)->people; v; v = v->next_in_room)
                        {
                            if ((ch != v) && (GetMaxLevel(v) >= GetMaxLevel(ch)))
                            {
                                act("$n casually walks out THROUGH the %s!", FALSE, ch, 0, v, TO_VICT,
                                    fname(exitp->keyword));
                            }
                        }
                    }
                    else
                    {
                        act("$n casually walks out THROUGH the %s!", FALSE, ch, 0, 0, TO_ROOM, fname(exitp->keyword));
                    }
                    return TRUE;
                }
                cprintf(ch, "The %s seems to be closed.\r\n", fname(exitp->keyword));
                return FALSE;
            }
            else
            {
                NotLegalMove(ch);
                return FALSE;
            }
        }
        else
        {
            NotLegalMove(ch);
            return FALSE;
        }
    }
    else
    {
        if (IS_SET(there->room_flags, INDOORS))
        {
            if (MOUNTED(ch))
            {
                cprintf(ch, "Your mount refuses to go that way\r\n");
                return FALSE;
            }
        }
        if (IS_SET(there->room_flags, DEATH))
        {
            if (MOUNTED(ch))
            {
                cprintf(ch, "Your mount refuses to go that way\r\n");
                return FALSE;
            }
        }
        return TRUE;
    }
}

int RawMove(struct char_data *ch, int dir)
{
    struct obj_data *obj = NULL;
    struct room_data *from_here = NULL;
    struct room_data *to_here = NULL;
    struct char_data *pers = NULL;
    int need_movement = FALSE;
    int has_boat = FALSE;
    int amount = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), dir);

    if (special(ch, dir + 1, "")) /* Check for special routines(North is 1) */
        return FALSE;

    if (!ValidMove(ch, dir))
    {
        return FALSE;
    }
    if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master) && (ch->in_room == ch->master->in_room))
    {
        act("$n bursts into tears.", FALSE, ch, 0, 0, TO_ROOM);
        act("You burst into tears at the thought of leaving $N", FALSE, ch, 0, ch->master, TO_CHAR);
        return FALSE;
    }
    amount = 0;
    if (IS_AFFECTED(ch, AFF_SNEAK))
    {
        amount = 2;

        if ((GET_LEVEL(ch, BestThiefClass(ch)) >= 1) && (GET_LEVEL(ch, BestThiefClass(ch)) < 5))
            amount += 13;
        else if ((GET_LEVEL(ch, BestThiefClass(ch)) >= 5) && (GET_LEVEL(ch, BestThiefClass(ch)) < 10))
            amount += 11;
        else if ((GET_LEVEL(ch, BestThiefClass(ch)) >= 10) && (GET_LEVEL(ch, BestThiefClass(ch)) < 15))
            amount += 9;
        else if ((GET_LEVEL(ch, BestThiefClass(ch)) >= 15) && (GET_LEVEL(ch, BestThiefClass(ch)) < 20))
            amount += 7;
        else if ((GET_LEVEL(ch, BestThiefClass(ch)) >= 20) && (GET_LEVEL(ch, BestThiefClass(ch)) < 25))
            amount += 5;
        else if ((GET_LEVEL(ch, BestThiefClass(ch)) >= 25) && (GET_LEVEL(ch, BestThiefClass(ch)) < 30))
            amount += 3;
        else if ((GET_LEVEL(ch, BestThiefClass(ch)) >= 30) && (GET_LEVEL(ch, BestThiefClass(ch)) < 40))
            amount += 1;

        amount -= MAX(2, wis_app[(int)GET_DEX(ch)].bonus);

        if (amount <= 0)
            amount = 1;

        GET_MANA(ch) -= amount;
        if (GET_MANA(ch) <= 0)
        {
            affect_from_char(ch, SKILL_SNEAK);
            GET_MANA(ch) = 0;
        }
    }
    from_here = real_roomp(ch->in_room);
    to_here = real_roomp(from_here->dir_option[dir]->to_room);

    if (to_here == NULL)
    {
        char_from_room(ch);
        char_to_room(ch, 0);
        cprintf(ch, "Uh-oh.  The ground melts beneath you as you fall into the swirling chaos.\r\n");
        do_look(ch, "", 15);
        return TRUE;
    }
    if (IS_AFFECTED(ch, AFF_FLYING))
        need_movement = 1;
    else
    {
        need_movement = (movement_loss[from_here->sector_type] + movement_loss[to_here->sector_type]) / 2;
    }

    /*
     **   Movement in water_nowswim
     */

    if ((from_here->sector_type == SECT_WATER_SWIM) || (to_here->sector_type == SECT_WATER_SWIM))
    {
        if ((!IS_AFFECTED(ch, AFF_WATERBREATH)) && (!IS_AFFECTED(ch, AFF_FLYING)))
        {
            has_boat = number(1, 101);
            if (has_boat < ch->skills[SKILL_SWIMMING].learned)
                need_movement = 1;
        }
    }
    if ((from_here->sector_type == SECT_WATER_NOSWIM) || (to_here->sector_type == SECT_WATER_NOSWIM))
    {
        if (MOUNTED(ch))
        {
            if (!IS_AFFECTED(MOUNTED(ch), AFF_WATERBREATH) && !IS_AFFECTED(MOUNTED(ch), AFF_FLYING))
            {
                cprintf(ch, "Your mount would have to fly or swim to go there\r\n");
                return FALSE;
            }
        }
        if ((!IS_AFFECTED(ch, AFF_WATERBREATH)) && (!IS_AFFECTED(ch, AFF_FLYING)))
        {
            has_boat = FALSE;
            /*
             * See if char is carrying a boat
             */
            for (obj = ch->carrying; obj; obj = obj->next_content)
                if (obj->obj_flags.type_flag == ITEM_BOAT)
                    has_boat = TRUE;
            if (!has_boat)
            {
                cprintf(ch, "You need a boat to go there.\r\n");
                return FALSE;
            }
            if (has_boat)
                need_movement = 1;
        }
    }
    /*
     * Movement in SECT_AIR
     */

    if ((from_here->sector_type == SECT_AIR) || (to_here->sector_type == SECT_AIR))
    {
        if (MOUNTED(ch))
        {
            if (!IS_AFFECTED(MOUNTED(ch), AFF_FLYING))
            {
                cprintf(ch, "Your mount would have to fly to go there!\r\n");
                return FALSE;
            }
        }
        if (!IS_AFFECTED(ch, AFF_FLYING))
        {
            cprintf(ch, "You would have to Fly to go there!\r\n");
            return FALSE;
        }
    }
    /*
     * Movement in SECT_UNDERWATER
     */

    if ((from_here->sector_type == SECT_UNDERWATER) || (to_here->sector_type == SECT_UNDERWATER))
    {
        if (MOUNTED(ch))
        {
            if (!IS_AFFECTED(MOUNTED(ch), AFF_WATERBREATH))
            {
                cprintf(ch, "Your mount would need gills to go there!\r\n");
                return FALSE;
            }
        }
        if (!IS_AFFECTED(ch, AFF_WATERBREATH))
        {
            cprintf(ch, "You would need to be a fish to go there!\r\n");
            return FALSE;
        }
    }
    if (!MOUNTED(ch))
    {
        if (GET_MOVE(ch) < need_movement)
        {
            cprintf(ch, "You are too exhausted.\r\n");
            return FALSE;
        }
    }
    else
    {
        if (GET_MOVE(MOUNTED(ch)) < need_movement)
        {
            cprintf(ch, "Your mount is too exhausted.\r\n");
            return FALSE;
        }
    }

    if (IS_MORTAL(ch) || MOUNTED(ch))
    {
        if (IS_NPC(ch))
        {
            GET_MOVE(ch) -= 1;
        }
        else
        {
            if (MOUNTED(ch))
            {
                GET_MOVE(MOUNTED(ch)) -= need_movement;
            }
            else
            {
                GET_MOVE(ch) -= need_movement;
            }
        }
    }
    /*
     *  nail the unlucky with traps.
     */

    if (!MOUNTED(ch))
    {
        if (CheckForMoveTrap(ch, dir))
            return FALSE;
    }
    else
    {
        if (CheckForMoveTrap(MOUNTED(ch), dir))
            return FALSE;
    }

    if (MOUNTED(ch))
    {
        char_from_room(ch);
        char_to_room(ch, from_here->dir_option[dir]->to_room);
        char_from_room(MOUNTED(ch));
        char_to_room(MOUNTED(ch), from_here->dir_option[dir]->to_room);
    }
    else
    {
        char_from_room(ch);
        char_to_room(ch, from_here->dir_option[dir]->to_room);
    }

    do_look(ch, "", 15);

    if (IS_SET(to_here->room_flags, DEATH) && IS_MORTAL(ch))
    {
        death_cry(ch);
        if (MOUNTED(ch))
            death_cry(MOUNTED(ch));

        if (IS_NPC(ch) && (IS_SET(ch->specials.act, ACT_POLYSELF)))
        {
            /*
             *   take char from storage, to room
             */
            if (MOUNTED(ch))
                extract_char(MOUNTED(ch));

            pers = ch->desc->original;
            char_from_room(pers);
            char_to_room(pers, ch->in_room);
            SwitchStuff(ch, pers);
            zero_rent(ch);
            extract_char(ch);
            ch = pers;
        }
        zero_rent(ch);
        extract_char(ch);
        return FALSE;
    }
    return TRUE;
}

int MoveOne(struct char_data *ch, int dir)
{
    int was_in = FALSE;

    if (DEBUG > 2)
        log_info("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), dir);

    was_in = ch->in_room;
    if (RawMove(ch, dir))
    { /* no error */
        DisplayOneMove(ch, dir, was_in);
        return TRUE;
    }
    else
        return FALSE;
}

int MoveGroup(struct char_data *ch, int dir)
{
    int was_in = FALSE;
    int i = 0;
    int heap_top = 0;
    int heap_tot[50];
    struct follow_type *k = NULL;
    struct follow_type *next_dude = NULL;
    struct char_data *heap_ptr[50];

    /*
     *   move the leader. (leader never duplicates)
     */

    if (DEBUG > 2)
        log_info("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), dir);

    was_in = ch->in_room;
    if (RawMove(ch, dir))
    { /* no error */
        DisplayOneMove(ch, dir, was_in);
        if (ch->followers)
        {
            heap_top = 0;
            for (k = ch->followers; k; k = next_dude)
            {
                next_dude = k->next;
                /*
                 *  compose a list of followers, w/heaping
                 */
                if ((was_in == k->follower->in_room) && (GET_POS(k->follower) >= POSITION_STANDING))
                {
                    act("You follow $N.", FALSE, k->follower, 0, ch, TO_CHAR);
                    if (k->follower->followers)
                    {
                        MoveGroup(k->follower, dir);
                    }
                    else
                    {
                        if (RawMove(k->follower, dir))
                        {
                            AddToCharHeap(heap_ptr, &heap_top, heap_tot, k->follower);
                            /*
                             * if (!AddToCharHeap(heap_ptr, &heap_top, heap_tot, k->follower))
                             * displayOneMove(k->follower, dir, was_in);
                             */
                        }
                    }
                }
            }
            /*
             *  now, print out the heaped display message
             */
            for (i = 0; i < heap_top; i++)
            {
                if (heap_tot[i] > 1)
                {
                    DisplayGroupMove(heap_ptr[i], dir, was_in, heap_tot[i]);
                }
                else
                {
                    DisplayOneMove(heap_ptr[i], dir, was_in);
                }
            }
        }
    }
    return TRUE;
}

void DisplayOneMove(struct char_data *ch, int dir, int was_in)
{
    if (DEBUG > 2)
        log_info("called %s with %s, %d, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), dir, was_in);

    DisplayMove(ch, dir, was_in, 1);
}

void DisplayGroupMove(struct char_data *ch, int dir, int was_in, int total)
{
    if (DEBUG > 2)
        log_info("called %s with %s, %d, %d, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), dir, was_in, total);

    DisplayMove(ch, dir, was_in, total);
}

// Normally, interpreter functions return TRUE if they "work", to tell the
// interpreter to stop looking for another command to try... but since DikuMUD
// used to be all void, this one returns TRUE to mean you actually moved.
// Should we ever USE the return values for any other do_ functions, we might
// need to split this out differently.
int do_move(struct char_data *ch, const char *argument, int cmd)
{
    if (DEBUG > 1)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (RIDDEN(ch))
    {
        if (RideCheck(RIDDEN(ch)))
        {
            return do_move(RIDDEN(ch), argument, cmd);
        }
        else
        {
            FallOffMount(RIDDEN(ch), ch);
            Dismount(RIDDEN(ch), ch, POSITION_SITTING);
        }
    }
    if (RIDDEN(ch))
    {
        if (RIDDEN(ch)->specials.fighting)
        {
            cprintf(ch, "You can't, your rider is fighting!\r\n");
            return FALSE;
        }
    }
    if (MOUNTED(ch))
    {
        if (MOUNTED(ch)->specials.fighting)
        {
            cprintf(ch, "You can't, your mount is fighting!\r\n");
            return FALSE;
        }
    }
    cmd -= 1;

    /*
     ** the move is valid, check for follower/master conflicts.
     */

    if (ch->attackers > 2)
    {
        cprintf(ch, "There's too many people around, no place to flee!\r\n");
        return FALSE;
    }
    if (!ch->followers && !ch->master)
    {
        return MoveOne(ch, cmd);
    }
    else
    {
        if (!ch->followers)
        {
            return MoveOne(ch, cmd);
        }
        else
        {
            return MoveGroup(ch, cmd);
        }
    }
}

/*
 * MoveOne and MoveGroup print messages.  Raw move sends success or failure.
 */

void DisplayMove(struct char_data *ch, int dir, int was_in, int total)
{
    struct char_data *tmp_ch = NULL;
    char tmp[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %d, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), dir, was_in, total);

    for (tmp_ch = real_roomp(was_in)->people; tmp_ch; tmp_ch = tmp_ch->next_in_room)
    {
        if ((!IS_AFFECTED(ch, AFF_SNEAK)) || (IS_IMMORTAL(tmp_ch)))
        {
            if ((ch != tmp_ch) && (AWAKE(tmp_ch)) && (CAN_SEE(tmp_ch, ch)))
            {
                if (total > 1)
                {
                    if (IS_NPC(ch))
                    {
                        snprintf(tmp, MAX_INPUT_LENGTH, "%s leaves %s. [%d]\r\n", ch->player.short_descr, dirs[dir],
                                 total);
                    }
                    else
                    {
                        snprintf(tmp, MAX_INPUT_LENGTH, "%s leaves %s.[%d]\r\n", GET_NAME(ch), dirs[dir], total);
                    }
                }
                else
                {
                    if (IS_NPC(ch))
                    {
                        if (MOUNTED(ch))
                        {
                            struct char_data *mount;

                            mount = MOUNTED(ch);
                            snprintf(tmp, MAX_INPUT_LENGTH, "%s leaves %s, riding on %s\r\n", ch->player.short_descr,
                                     dirs[dir], mount->player.short_descr);
                        }
                        else
                        {
                            snprintf(tmp, MAX_INPUT_LENGTH, "%s leaves %s.\r\n", ch->player.short_descr, dirs[dir]);
                        }
                    }
                    else
                    {
                        if (MOUNTED(ch))
                        {
                            snprintf(tmp, MAX_INPUT_LENGTH, "%s leaves %s, riding on %s\r\n", GET_NAME(ch), dirs[dir],
                                     MOUNTED(ch)->player.short_descr);
                        }
                        else
                        {
                            snprintf(tmp, MAX_INPUT_LENGTH, "%s leaves %s\r\n", GET_NAME(ch), dirs[dir]);
                        }
                    }
                }
                cprintf(tmp_ch, "%s", tmp);
            }
        }
    }

    for (tmp_ch = real_roomp(ch->in_room)->people; tmp_ch; tmp_ch = tmp_ch->next_in_room)
    {
        if (((!IS_AFFECTED(ch, AFF_SNEAK)) || (IS_IMMORTAL(tmp_ch))) && (CAN_SEE(tmp_ch, ch)) && (AWAKE(tmp_ch)))
        {
            if (tmp_ch != ch)
            {
                if (dir < 4)
                {
                    if (total == 1)
                    {
                        if (MOUNTED(ch))
                        {
                            snprintf(tmp, MAX_INPUT_LENGTH, "%s has arrived from the %s, riding on %s",
                                     PERS(ch, tmp_ch), dirs[rev_dir[dir]], PERS(MOUNTED(ch), tmp_ch));
                        }
                        else
                        {
                            snprintf(tmp, MAX_INPUT_LENGTH, "%s has arrived from the %s.", PERS(ch, tmp_ch),
                                     dirs[rev_dir[dir]]);
                        }
                    }
                    else
                    {
                        snprintf(tmp, MAX_INPUT_LENGTH, "%s has arrived from the %s.", PERS(ch, tmp_ch),
                                 dirs[rev_dir[dir]]);
                    }
                }
                else if (dir == 4)
                {
                    if (total == 1)
                    {
                        if (MOUNTED(ch))
                        {
                            snprintf(tmp, MAX_INPUT_LENGTH, "%s has arrived from below, riding on %s", PERS(ch, tmp_ch),
                                     PERS(MOUNTED(ch), tmp_ch));
                        }
                        else
                        {
                            snprintf(tmp, MAX_INPUT_LENGTH, "%s has arrived from below.", PERS(ch, tmp_ch));
                        }
                    }
                    else
                    {
                        snprintf(tmp, MAX_INPUT_LENGTH, "%s has arrived from below.", PERS(ch, tmp_ch));
                    }
                }
                else if (dir == 5)
                {
                    if (total == 1)
                    {
                        if (MOUNTED(ch))
                        {
                            snprintf(tmp, MAX_INPUT_LENGTH, "%s has arrived from above, riding on %s", PERS(ch, tmp_ch),
                                     PERS(MOUNTED(ch), tmp_ch));
                        }
                        else
                        {
                            snprintf(tmp, MAX_INPUT_LENGTH, "%s has arrived from above", PERS(ch, tmp_ch));
                        }
                    }
                    else
                    {
                        snprintf(tmp, MAX_INPUT_LENGTH, "%s has arrived from above.", PERS(ch, tmp_ch));
                    }
                }
                else
                {
                    if (total == 1)
                    {
                        if (MOUNTED(ch))
                        {
                            snprintf(tmp, MAX_INPUT_LENGTH, "%s has arrived from somewhere, riding on %s",
                                     PERS(ch, tmp_ch), PERS(MOUNTED(ch), tmp_ch));
                        }
                        else
                        {
                            snprintf(tmp, MAX_INPUT_LENGTH, "%s has arrived from somewhere.", PERS(ch, tmp_ch));
                        }
                    }
                    else
                    {
                        snprintf(tmp, MAX_INPUT_LENGTH, "%s has arrived from somewhere.", PERS(ch, tmp_ch));
                    }
                }

                if (total > 1)
                {
                    scprintf(tmp, MAX_INPUT_LENGTH, " [%d]", total);
                }
                strlcat(tmp, "\r\n", MAX_INPUT_LENGTH);
                cprintf(tmp_ch, "%s", tmp);
            }
        }
    }
}

void AddToCharHeap(struct char_data *heap[50], int *top, int total[50], struct char_data *k)
{
    int found = FALSE;
    int i = 0;

    if (DEBUG > 2)
        log_info("called %s with %08zx, %08zx, %08zx, %s", __PRETTY_FUNCTION__, (size_t)heap, (size_t)top,
                 (size_t)total, SAFE_NAME(k));

    if (*top <= 50)
    {
        found = FALSE;
        for (i = 0; (i < *top && !found); i++)
        {
            if (*top > 0)
            {
                if ((IS_NPC(k)) && (k->nr == heap[i]->nr) && (heap[i]->player.short_descr) &&
                    (!strcmp(k->player.short_descr, heap[i]->player.short_descr)))
                {
                    total[i] += 1;
                    found = TRUE;
                }
            }
        }
        if (!found)
        {
            heap[*top] = k;
            total[*top] = 1;
            *top += 1;
        }
    }
}

int find_door(struct char_data *ch, char *type, char *dir)
{
    int door = -1;

    const char *exit_dirs[] = {"north", "east", "south", "west", "up", "down", "\n"};
    struct room_direction_data *exitp = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(type), VNULL(dir));

    if (*dir)
    { /* a direction was specified */
        if ((door = search_block(dir, exit_dirs, FALSE)) == -1)
        { /* Partial Match */
            cprintf(ch, "That's not a direction.\r\n");
            return -1;
        }
        exitp = EXIT(ch, door);
        if (exitp)
        {
            if (!exitp->keyword)
                return door;
            if ((isname(type, exitp->keyword)) && (strcmp(type, "secret")))
            {
                return door;
            }
            else
            {
                cprintf(ch, "I see no %s there.\r\n", type);
                return -1;
            }
        }
        else
        {
            cprintf(ch, "I see no %s there.\r\n", type);
            return -1;
        }
    }
    else
    { /* try to locate the keyword */
        for (door = 0; door < MAX_NUM_EXITS; door++)
            if ((exitp = EXIT(ch, door)) && exitp->keyword && isname(type, exitp->keyword))
                return door;

        cprintf(ch, "I see no %s here.\r\n", type);
        return -1;
    }
}

/*
 * remove all necessary bits and send messages
 */
void open_door(struct char_data *ch, int dir)
{
    struct room_direction_data *exitp = NULL;
    struct room_direction_data *back = NULL;
    struct room_data *rp = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), dir);

    rp = real_roomp(ch->in_room);
    if (rp == NULL)
    {
        log_error("NULL rp in open_door() for %s.", PERS(ch, ch));
    }
    exitp = rp->dir_option[dir];

    REMOVE_BIT(exitp->exit_info, EX_CLOSED);
    if (exitp->keyword)
    {
        if (strcmp(exitp->keyword, "secret") && (IS_NOT_SET(exitp->exit_info, EX_SECRET)))
        {
            act("$n opens the %s", FALSE, ch, 0, 0, TO_ROOM, fname(exitp->keyword));
        }
        else
        {
            act("$n reveals a hidden passage!", FALSE, ch, 0, 0, TO_ROOM);
        }
    }
    else
        act("$n opens the door.", FALSE, ch, 0, 0, TO_ROOM);

    /*
     * now for opening the OTHER side of the door!
     */
    if (exit_ok(exitp, &rp) && (back = rp->dir_option[rev_dir[dir]]) && (back->to_room == ch->in_room))
    {
        REMOVE_BIT(back->exit_info, EX_CLOSED);
        if (back->keyword)
        {
            rprintf(exitp->to_room, "The %s is opened from the other side.\r\n", fname(back->keyword));
        }
        else
            rprintf(exitp->to_room, "The door is opened from the other side.\r\n");
    }
}

/*
 * remove all necessary bits and send messages
 */
void raw_open_door(struct char_data *ch, int dir)
{
    struct room_direction_data *exitp = NULL;
    struct room_direction_data *back = NULL;
    struct room_data *rp = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), dir);

    rp = real_roomp(ch->in_room);
    if (rp == NULL)
        log_error("NULL rp in open_door() for %s.", PERS(ch, ch));
    exitp = rp->dir_option[dir];

    REMOVE_BIT(exitp->exit_info, EX_CLOSED);
    /*
     * now for opening the OTHER side of the door!
     */
    if (exit_ok(exitp, &rp) && (back = rp->dir_option[rev_dir[dir]]) && (back->to_room == ch->in_room))
    {
        REMOVE_BIT(back->exit_info, EX_CLOSED);
        if (back->keyword && (strcmp("secret", fname(back->keyword))))
        {
            rprintf(exitp->to_room, "The %s is opened from the other side.\r\n", fname(back->keyword));
        }
        else
        {
            rprintf(exitp->to_room, "The door is opened from the other side.\r\n");
        }
    }
}

int do_open(struct char_data *ch, const char *argument, int cmd)
{
    int door = -1;
    char type[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char dir[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    struct obj_data *obj = NULL;
    struct char_data *victim = NULL;
    struct room_direction_data *exitp = NULL;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    argument_interpreter(argument, type, dir);

    if (!*type)
        cprintf(ch, "Open what?\r\n");
    else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj))
    {
        /*
         * this is an object
         */
        if (obj->obj_flags.type_flag != ITEM_CONTAINER)
            cprintf(ch, "That's not a container.\r\n");
        else if (IS_NOT_SET(obj->obj_flags.value[1], CONT_CLOSED))
            cprintf(ch, "But it's already open!\r\n");
        else if (IS_NOT_SET(obj->obj_flags.value[1], CONT_CLOSEABLE))
            cprintf(ch, "You can't do that.\r\n");
        else if (IS_SET(obj->obj_flags.value[1], CONT_LOCKED))
            cprintf(ch, "It seems to be locked.\r\n");
        else
        {
            REMOVE_BIT(obj->obj_flags.value[1], CONT_CLOSED);
            cprintf(ch, "Ok.\r\n");
            act("$n opens $p.", FALSE, ch, obj, 0, TO_ROOM);
        }
    }
    else if ((door = find_door(ch, type, dir)) >= 0)
    {
        /*
         * perhaps it is a door
         */
        exitp = EXIT(ch, door);
        if (IS_NOT_SET(exitp->exit_info, EX_ISDOOR))
            cprintf(ch, "That's impossible, I'm afraid.\r\n");
        else if (IS_NOT_SET(exitp->exit_info, EX_CLOSED))
            cprintf(ch, "It's already open!\r\n");
        else if (IS_SET(exitp->exit_info, EX_LOCKED))
            cprintf(ch, "It seems to be locked.\r\n");
        else
        {
            open_door(ch, door);
            cprintf(ch, "Ok.\r\n");
        }
    }
    return TRUE;
}

int do_close(struct char_data *ch, const char *argument, int cmd)
{
    int door = -1;
    char type[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char dir[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    struct room_direction_data *back = NULL;
    struct room_direction_data *exitp = NULL;
    struct obj_data *obj = NULL;
    struct char_data *victim = NULL;
    struct room_data *rp = NULL;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    argument_interpreter(argument, type, dir);

    if (!*type)
        cprintf(ch, "Close what?\r\n");
    else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj))
    {

        /*
         * this is an object
         */

        if (obj->obj_flags.type_flag != ITEM_CONTAINER)
            cprintf(ch, "That's not a container.\r\n");
        else if (IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
            cprintf(ch, "But it's already closed!\r\n");
        else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSEABLE))
            cprintf(ch, "That's impossible.\r\n");
        else
        {
            SET_BIT(obj->obj_flags.value[1], CONT_CLOSED);
            cprintf(ch, "Ok.\r\n");
            act("$n closes $p.", FALSE, ch, obj, 0, TO_ROOM);
        }
    }
    else if ((door = find_door(ch, type, dir)) >= 0)
    {

        /*
         * Or a door
         */
        exitp = EXIT(ch, door);
        if (!IS_SET(exitp->exit_info, EX_ISDOOR))
            cprintf(ch, "That's absurd.\r\n");
        else if (IS_SET(exitp->exit_info, EX_CLOSED))
            cprintf(ch, "It's already closed!\r\n");
        else
        {
            SET_BIT(exitp->exit_info, EX_CLOSED);
            if (exitp->keyword)
                act("$n closes the $F.", 0, ch, 0, exitp->keyword, TO_ROOM);
            else
                act("$n closes the door.", FALSE, ch, 0, 0, TO_ROOM);
            cprintf(ch, "Ok.\r\n");
            /*
             * now for closing the other side, too
             */
            if (exit_ok(exitp, &rp) && (back = rp->dir_option[rev_dir[door]]) && (back->to_room == ch->in_room))
            {
                SET_BIT(back->exit_info, EX_CLOSED);
                if (back->keyword)
                {
                    rprintf(exitp->to_room, "The %s closes quietly.\r\n", back->keyword);
                }
                else
                    rprintf(exitp->to_room, "The door closes quietly.\r\n");
            }
        }
    }
    return TRUE;
}

int has_key(struct char_data *ch, int key)
{
    struct obj_data *o = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), key);

    for (o = ch->carrying; o; o = o->next_content)
        if (obj_index[o->item_number].vnum == key)
            return TRUE;

    if (ch->equipment[HOLD])
        if (obj_index[ch->equipment[HOLD]->item_number].vnum == key)
            return TRUE;

    return FALSE;
}

void raw_unlock_door(struct char_data *ch, struct room_direction_data *exitp, int door)
{
    struct room_data *rp = NULL;
    struct room_direction_data *back = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %08zx, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), (size_t)exitp, door);

    REMOVE_BIT(exitp->exit_info, EX_LOCKED);
    /*
     * now for unlocking the other side, too
     */
    rp = real_roomp(exitp->to_room);
    if (rp && (back = rp->dir_option[rev_dir[door]]) && back->to_room == ch->in_room)
    {
        REMOVE_BIT(back->exit_info, EX_LOCKED);
    }
    else
    {
        log_info("Inconsistent door locks in rooms %d->%d", ch->in_room, exitp->to_room);
    }
}

void raw_lock_door(struct char_data *ch, struct room_direction_data *exitp, int door)
{
    struct room_data *rp = NULL;
    struct room_direction_data *back = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %08zx, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), (size_t)exitp, door);

    SET_BIT(exitp->exit_info, EX_LOCKED);
    /*
     * now for locking the other side, too
     */
    rp = real_roomp(exitp->to_room);
    if (rp && (back = rp->dir_option[rev_dir[door]]) && back->to_room == ch->in_room)
    {
        SET_BIT(back->exit_info, EX_LOCKED);
    }
    else
    {
        log_info("Inconsistent door locks in rooms %d->%d", ch->in_room, exitp->to_room);
    }
}

int do_lock(struct char_data *ch, const char *argument, int cmd)
{
    int door = -1;
    char type[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char dir[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    struct room_direction_data *back = NULL;
    struct room_direction_data *exitp = NULL;
    struct obj_data *obj = NULL;
    struct char_data *victim = NULL;
    struct room_data *rp = NULL;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    argument_interpreter(argument, type, dir);

    if (!*type)
        cprintf(ch, "Lock what?\r\n");
    else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj))
    {

        /*
         * this is an object
         */

        if (obj->obj_flags.type_flag != ITEM_CONTAINER)
            cprintf(ch, "That's not a container.\r\n");
        else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
            cprintf(ch, "Maybe you should close it first...\r\n");
        else if (obj->obj_flags.value[2] < 0)
            cprintf(ch, "That thing can't be locked.\r\n");
        else if (!has_key(ch, obj->obj_flags.value[2]))
            cprintf(ch, "You don't seem to have the proper key.\r\n");
        else if (IS_SET(obj->obj_flags.value[1], CONT_LOCKED))
            cprintf(ch, "It is locked already.\r\n");
        else
        {
            SET_BIT(obj->obj_flags.value[1], CONT_LOCKED);
            cprintf(ch, "*Cluck*\r\n");
            act("$n locks $p - 'cluck', it says.", FALSE, ch, obj, 0, TO_ROOM);
        }
    }
    else if ((door = find_door(ch, type, dir)) >= 0)
    {

        /*
         * a door, perhaps
         */
        exitp = EXIT(ch, door);

        if (!IS_SET(exitp->exit_info, EX_ISDOOR))
            cprintf(ch, "That's absurd.\r\n");
        else if (!IS_SET(exitp->exit_info, EX_CLOSED))
            cprintf(ch, "You have to close it first, I'm afraid.\r\n");
        else if (exitp->key < 0)
            cprintf(ch, "There does not seem to be any keyholes.\r\n");
        else if (!has_key(ch, exitp->key))
            cprintf(ch, "You don't have the proper key.\r\n");
        else if (IS_SET(exitp->exit_info, EX_LOCKED))
            cprintf(ch, "It's already locked!\r\n");
        else
        {
            SET_BIT(exitp->exit_info, EX_LOCKED);
            if (exitp->keyword)
                act("$n locks the $F.", 0, ch, 0, exitp->keyword, TO_ROOM);
            else
                act("$n locks the door.", FALSE, ch, 0, 0, TO_ROOM);
            cprintf(ch, "*Click*\r\n");
            /*
             * now for locking the other side, too
             */
            rp = real_roomp(exitp->to_room);
            if (rp && (back = rp->dir_option[rev_dir[door]]) && back->to_room == ch->in_room)
                SET_BIT(back->exit_info, EX_LOCKED);
        }
    }
    return TRUE;
}

int do_unlock(struct char_data *ch, const char *argument, int cmd)
{
    int door = -1;
    char type[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char dir[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    struct room_direction_data *back = NULL;
    struct room_direction_data *exitp = NULL;
    struct obj_data *obj = NULL;
    struct char_data *victim = NULL;
    struct room_data *rp = NULL;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    argument_interpreter(argument, type, dir);

    if (!*type)
        cprintf(ch, "Unlock what?\r\n");
    else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj))
    {

        /*
         * this is an object
         */

        if (obj->obj_flags.type_flag != ITEM_CONTAINER)
            cprintf(ch, "That's not a container.\r\n");
        else if (obj->obj_flags.value[2] < 0)
            cprintf(ch, "Odd - you can't seem to find a keyhole.\r\n");
        else if (!has_key(ch, obj->obj_flags.value[2]))
            cprintf(ch, "You don't seem to have the proper key.\r\n");
        else if (!IS_SET(obj->obj_flags.value[1], CONT_LOCKED))
            cprintf(ch, "Oh.. it wasn't locked, after all.\r\n");
        else
        {
            REMOVE_BIT(obj->obj_flags.value[1], CONT_LOCKED);
            cprintf(ch, "*Click*\r\n");
            act("$n unlocks $p.", FALSE, ch, obj, 0, TO_ROOM);
        }
    }
    else if ((door = find_door(ch, type, dir)) >= 0)
    {

        /*
         * it is a door
         */
        exitp = EXIT(ch, door);

        if (!IS_SET(exitp->exit_info, EX_ISDOOR))
            cprintf(ch, "That's absurd.\r\n");
        else if (!IS_SET(exitp->exit_info, EX_CLOSED))
            cprintf(ch, "Heck.. it ain't even closed!\r\n");
        else if (exitp->key < 0)
            cprintf(ch, "You can't seem to spot any keyholes.\r\n");
        else if (!has_key(ch, exitp->key))
            cprintf(ch, "You do not have the proper key for that.\r\n");
        else if (!IS_SET(exitp->exit_info, EX_LOCKED))
            cprintf(ch, "It's already unlocked, it seems.\r\n");
        else
        {
            REMOVE_BIT(exitp->exit_info, EX_LOCKED);
            if (exitp->keyword)
                act("$n unlocks the $F.", 0, ch, 0, exitp->keyword, TO_ROOM);
            else
                act("$n unlocks the door.", FALSE, ch, 0, 0, TO_ROOM);
            cprintf(ch, "*click*\r\n");
            /*
             * now for unlocking the other side, too
             */
            rp = real_roomp(exitp->to_room);
            if (rp && (back = rp->dir_option[rev_dir[door]]) && back->to_room == ch->in_room)
                REMOVE_BIT(back->exit_info, EX_LOCKED);
        }
    }
    return TRUE;
}

int do_pick(struct char_data *ch, const char *argument, int cmd)
{
    int percent_chance = 0;
    int door = -1;
    char type[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char dir[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    struct room_direction_data *back = NULL;
    struct room_direction_data *exitp = NULL;
    struct obj_data *obj = NULL;
    struct char_data *victim = NULL;
    struct room_data *rp = NULL;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    argument_interpreter(argument, type, dir);

    percent_chance = number(1, 101); /* 101% is a complete failure */

    if (percent_chance > (ch->skills[SKILL_PICK_LOCK].learned))
    {
        cprintf(ch, "You failed to pick the lock.\r\n");
        return TRUE;
    }
    if (!*type)
    {
        cprintf(ch, "Pick what?\r\n");
    }
    else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj))
    {
        /*
         * this is an object
         */
        if (obj->obj_flags.type_flag != ITEM_CONTAINER)
            cprintf(ch, "That's not a container.\r\n");
        else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
            cprintf(ch, "Silly - it ain't even closed!\r\n");
        else if (obj->obj_flags.value[2] < 0)
            cprintf(ch, "Odd - you can't seem to find a keyhole.\r\n");
        else if (!IS_SET(obj->obj_flags.value[1], CONT_LOCKED))
            cprintf(ch, "Oho! This thing is NOT locked!\r\n");
        else if (IS_SET(obj->obj_flags.value[1], CONT_PICKPROOF))
            cprintf(ch, "It resists your attempts at picking it.\r\n");
        else
        {
            REMOVE_BIT(obj->obj_flags.value[1], CONT_LOCKED);
            cprintf(ch, "*Click*\r\n");
            act("$n fiddles with $p.", FALSE, ch, obj, 0, TO_ROOM);
            if (ch->skills[SKILL_PICK_LOCK].learned < 50)
                ch->skills[SKILL_PICK_LOCK].learned += 2;
        }
    }
    else if ((door = find_door(ch, type, dir)) >= 0)
    {
        exitp = EXIT(ch, door);
        if (!IS_SET(exitp->exit_info, EX_ISDOOR))
            cprintf(ch, "That's absurd.\r\n");
        else if (!IS_SET(exitp->exit_info, EX_CLOSED))
            cprintf(ch, "You realize that the door is already open.\r\n");
        else if (exitp->key < 0)
            cprintf(ch, "You can't seem to spot any lock to pick.\r\n");
        else if (!IS_SET(exitp->exit_info, EX_LOCKED))
            cprintf(ch, "Oh.. it wasn't locked at all.\r\n");
        else if (IS_SET(exitp->exit_info, EX_PICKPROOF))
            cprintf(ch, "You seem to be unable to pick this lock.\r\n");
        else
        {
            REMOVE_BIT(exitp->exit_info, EX_LOCKED);
            if (exitp->keyword)
                act("$n skillfully picks the lock of the $F.", 0, ch, 0, exitp->keyword, TO_ROOM);
            else
                act("$n picks the lock.", TRUE, ch, 0, 0, TO_ROOM);
            cprintf(ch, "The lock quickly yields to your skills.\r\n");
            /*
             * now for unlocking the other side, too
             */
            rp = real_roomp(exitp->to_room);
            if (rp && (back = rp->dir_option[rev_dir[door]]) && back->to_room == ch->in_room)
                REMOVE_BIT(back->exit_info, EX_LOCKED);
            if (ch->skills[SKILL_PICK_LOCK].learned < 50)
                ch->skills[SKILL_PICK_LOCK].learned += 2;
        }
    }
    return TRUE;
}

int do_enter(struct char_data *ch, const char *argument, int cmd)
{
    int door = -1;
    char buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    struct room_direction_data *exitp = NULL;
    struct room_data *rp = NULL;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    one_argument(argument, buf);

    if (*buf)
    { /* an argument was supplied, search for door keyword */
        for (door = 0; door < MAX_NUM_EXITS; door++)
            if (exit_ok(exitp = EXIT(ch, door), NULL) && exitp->keyword && 0 == str_cmp(exitp->keyword, buf))
            {
                do_move(ch, "", ++door);
                return TRUE;
            }
        cprintf(ch, "There is no %s here.\r\n", buf);
    }
    else if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS))
    {
        cprintf(ch, "You are already indoors.\r\n");
    }
    else
    {
        /*
         * try to locate an entrance
         */
        for (door = 0; door < MAX_NUM_EXITS; door++)
            if (exit_ok(exitp = EXIT(ch, door), &rp) && !IS_SET(exitp->exit_info, EX_CLOSED) &&
                IS_SET(rp->room_flags, INDOORS))
            {
                do_move(ch, "", ++door);
                return TRUE;
            }
        cprintf(ch, "You can't seem to find anything to enter.\r\n");
    }
    return TRUE;
}

int do_leave(struct char_data *ch, const char *argument, int cmd)
{
    int door = -1;
    struct room_direction_data *exitp = NULL;
    struct room_data *rp = NULL;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (!IS_SET(RM_FLAGS(ch->in_room), INDOORS))
        cprintf(ch, "You are outside.. where do you want to go?\r\n");
    else
    {
        for (door = 0; door < MAX_NUM_EXITS; door++)
            if (exit_ok(exitp = EXIT(ch, door), &rp) && !IS_SET(exitp->exit_info, EX_CLOSED) &&
                !IS_SET(rp->room_flags, INDOORS))
            {
                do_move(ch, "", ++door);
                return TRUE;
            }
        cprintf(ch, "I see no obvious exits to the outside.\r\n");
    }
    return TRUE;
}

int do_stand(struct char_data *ch, const char *argument, int cmd)
{
    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    switch (GET_POS(ch))
    {
    case POSITION_STANDING: {
        act("You are already standing.", FALSE, ch, 0, 0, TO_CHAR);
    }
    break;
    case POSITION_SITTING: {
        act("You stand up.", FALSE, ch, 0, 0, TO_CHAR);
        act("$n clambers on $s feet.", TRUE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = POSITION_STANDING;
    }
    break;
    case POSITION_RESTING: {
        act("You stop resting, and stand up.", FALSE, ch, 0, 0, TO_CHAR);
        act("$n stops resting, and clambers on $s feet.", TRUE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = POSITION_STANDING;
    }
    break;
    case POSITION_SLEEPING: {
        act("You have to wake up first!", FALSE, ch, 0, 0, TO_CHAR);
    }
    break;
    case POSITION_MOUNTED:
        cprintf(ch, "But you are mounted?\r\n");
        break;
    case POSITION_FIGHTING: {
        act("Do you not consider fighting as standing?", FALSE, ch, 0, 0, TO_CHAR);
    }
    break;
    default: {
        act("You stop floating around, and put your feet on the ground.", FALSE, ch, 0, 0, TO_CHAR);
        act("$n stops floating around, and puts $s feet on the ground.", TRUE, ch, 0, 0, TO_ROOM);
    }
    break;
    }
    return TRUE;
}

int do_sit(struct char_data *ch, const char *argument, int cmd)
{
    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    switch (GET_POS(ch))
    {
    case POSITION_STANDING: {
        act("You sit down.", FALSE, ch, 0, 0, TO_CHAR);
        act("$n sits down.", FALSE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = POSITION_SITTING;
    }
    break;
    case POSITION_SITTING: {
        cprintf(ch, "You'r sitting already.\r\n");
    }
    break;
    case POSITION_RESTING: {
        act("You stop resting, and sit up.", FALSE, ch, 0, 0, TO_CHAR);
        act("$n stops resting.", TRUE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = POSITION_SITTING;
    }
    break;
    case POSITION_SLEEPING: {
        act("You have to wake up first.", FALSE, ch, 0, 0, TO_CHAR);
    }
    break;
    case POSITION_MOUNTED:
        cprintf(ch, "But you are mounted?\r\n");
        break;
    case POSITION_FIGHTING: {
        act("Sit down while fighting? are you MAD?", FALSE, ch, 0, 0, TO_CHAR);
    }
    break;
    default: {
        act("You stop floating around, and sit down.", FALSE, ch, 0, 0, TO_CHAR);
        act("$n stops floating around, and sits down.", TRUE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = POSITION_SITTING;
    }
    break;
    }
    return TRUE;
}

int do_rest(struct char_data *ch, const char *argument, int cmd)
{
    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    switch (GET_POS(ch))
    {
    case POSITION_STANDING: {
        cprintf(ch, "You sit down and rest your tired bones.\r\n");
        act("$n sits down and rests.", TRUE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = POSITION_RESTING;
    }
    break;
    case POSITION_SITTING: {
        cprintf(ch, "You rest your tired bones\r\n.");
        act("$n rests.", TRUE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = POSITION_RESTING;
    }
    break;
    case POSITION_RESTING: {
        cprintf(ch, "You are already resting.\r\n");
    }
    break;
    case POSITION_SLEEPING: {
        cprintf(ch, "You have to wake up first.\r\n");
    }
    break;
    case POSITION_MOUNTED:
        cprintf(ch, "But you are mounted?\r\n");
        break;
    case POSITION_FIGHTING: {
        cprintf(ch, "Rest while fighting? are you MAD?\r\n");
    }
    break;
    default: {
        act("You stop floating around, and stop to rest your tired bones.", FALSE, ch, 0, 0, TO_CHAR);
        act("$n stops floating around, and rests.", FALSE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = POSITION_SITTING;
    }
    break;
    }
    return TRUE;
}

int do_sleep(struct char_data *ch, const char *argument, int cmd)
{
    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    switch (GET_POS(ch))
    {
    case POSITION_STANDING:
    case POSITION_SITTING:
    case POSITION_RESTING: {
        cprintf(ch, "You go to sleep.\r\n");
        act("$n lies down and falls asleep.", TRUE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = POSITION_SLEEPING;
    }
    break;
    case POSITION_SLEEPING: {
        cprintf(ch, "You are already sound asleep.\r\n");
    }
    break;
    case POSITION_MOUNTED:
        cprintf(ch, "But you are mounted?\r\n");
        break;
    case POSITION_FIGHTING: {
        cprintf(ch, "Sleep while fighting? are you MAD?\r\n");
    }
    break;
    default: {
        act("You stop floating around, and lie down to sleep.", FALSE, ch, 0, 0, TO_CHAR);
        act("$n stops floating around, and lie down to sleep.", TRUE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = POSITION_SLEEPING;
    }
    break;
    }
    return TRUE;
}

int do_wake(struct char_data *ch, const char *argument, int cmd)
{
    struct char_data *tmp_char = NULL;
    char arg[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    one_argument(argument, arg);
    if (*arg)
    {
        if (GET_POS(ch) == POSITION_SLEEPING)
        {
            act("You can't wake people up if you are asleep yourself!", FALSE, ch, 0, 0, TO_CHAR);
        }
        else
        {
            tmp_char = get_char_room_vis(ch, arg);
            if (tmp_char)
            {
                if (tmp_char == ch)
                {
                    act("If you want to wake yourself up, just type 'wake'", FALSE, ch, 0, 0, TO_CHAR);
                }
                else
                {
                    if (GET_POS(tmp_char) == POSITION_SLEEPING)
                    {
                        if (IS_AFFECTED(tmp_char, AFF_SLEEP))
                        {
                            act("You can not wake $M up!", FALSE, ch, 0, tmp_char, TO_CHAR);
                        }
                        else
                        {
                            act("You wake $M up.", FALSE, ch, 0, tmp_char, TO_CHAR);
                            GET_POS(tmp_char) = POSITION_SITTING;
                            act("You are awakened by $n.", FALSE, ch, 0, tmp_char, TO_VICT);
                        }
                    }
                    else
                    {
                        act("$N is already awake.", FALSE, ch, 0, tmp_char, TO_CHAR);
                    }
                }
            }
            else
            {
                cprintf(ch, "You do not see that person here.\r\n");
            }
        }
    }
    else
    {
        if (IS_AFFECTED(ch, AFF_SLEEP))
        {
            cprintf(ch, "You can't wake up!\r\n");
        }
        else
        {
            if (GET_POS(ch) > POSITION_SLEEPING)
                cprintf(ch, "You are already awake...\r\n");
            else
            {
                cprintf(ch, "You wake, and sit up.\r\n");
                act("$n awakens.", TRUE, ch, 0, 0, TO_ROOM);
                GET_POS(ch) = POSITION_SITTING;
            }
        }
    }
    return TRUE;
}

int do_follow(struct char_data *ch, const char *argument, int cmd)
{
    char name[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    struct char_data *leader = NULL;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    only_argument(argument, name);

    if (*name)
    {
        if (!(leader = get_char_room_vis(ch, name)))
        {
            cprintf(ch, "I see no person by that name here!\r\n");
            return TRUE;
        }
    }
    else
    {
        cprintf(ch, "Who do you wish to follow?\r\n");
        return TRUE;
    }

    if (IS_AFFECTED(ch, AFF_GROUP))
    {
        REMOVE_BIT(ch->specials.affected_by, AFF_GROUP);
    }
    if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master))
    {
        act("But you only feel like following $N!", FALSE, ch, 0, ch->master, TO_CHAR);
    }
    else
    { /* Not Charmed follow person */

        if (leader == ch)
        {
            if (!ch->master)
            {
                cprintf(ch, "You are already following yourself.\r\n");
                return TRUE;
            }
            stop_follower(ch);
        }
        else
        {
            if (circle_follow(ch, leader))
            {
                act("Sorry, but following in 'loops' is not allowed", FALSE, ch, 0, 0, TO_CHAR);
                return TRUE;
            }
            if (ch->master)
                stop_follower(ch);

            add_follower(ch, leader);
        }
    }
    return TRUE;
}

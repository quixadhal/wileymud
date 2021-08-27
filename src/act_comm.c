/*
 * file: act.comm.c , Implementation of commands.         Part of DIKUMUD
 * Usage : Communication.
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */

#include <stdio.h>
#include <stdlib.h>
/* #include <unistd.h> */
#include <sys/types.h>
#include <string.h>

#include "global.h"
#include "bug.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "mudlimits.h"
#include "multiclass.h"
#include "board.h"
#define _ACT_COMM_C
#include "act_comm.h"

int do_say(struct char_data *ch, const char *argument, int cmd)
{
    int i = 0;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    for (i = 0; *(argument + i) == ' '; i++)
        ; /* skip leading spaces */

    if (!*(argument + i))
        cprintf(ch, "usage: say <mesg>.\r\n");
    else
    {
        switch (argument[strlen(argument) - 1])
        {
        case '!':
            act("\x1b[0;32m$n exclaims '%s'\x1b[0m", FALSE, ch, 0, 0, TO_ROOM, argument + i);
            if (IS_NPC(ch) || (IS_SET(ch->specials.new_act, NEW_PLR_ECHO)))
            {
                cprintf(ch, "\x1b[0;32mYou exclaim '%s'\x1b[0m\r\n", argument + i);
            }
            break;
        case '?':
            act("\x1b[0;32m$n asks '%s'\x1b[0m", FALSE, ch, 0, 0, TO_ROOM, argument + i);
            if (IS_NPC(ch) || (IS_SET(ch->specials.new_act, NEW_PLR_ECHO)))
            {
                cprintf(ch, "\x1b[0;32mYou ask '%s'\x1b[0m\r\n", argument + i);
            }
            break;
        default:
            act("\x1b[0;32m$n says '%s'\x1b[0m", FALSE, ch, 0, 0, TO_ROOM, argument + i);
            if (IS_NPC(ch) || (IS_SET(ch->specials.new_act, NEW_PLR_ECHO)))
            {
                cprintf(ch, "\x1b[0;32mYou say '%s'\x1b[0m\r\n", argument + i);
            }
            break;
        }
    }
    return TRUE;
}

int do_shout(struct char_data *ch, const char *argument, int cmd)
{
    struct descriptor_data *i = NULL;
    struct room_data *rp = NULL;
    struct room_data *mrp = NULL;

#ifdef RADIUS_SHOUT
    struct room_direction_data *exitp = NULL;
    struct char_data *v = NULL;
    int x = 0;
#endif

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (IS_SET(ch->specials.new_act, NEW_PLR_NOSHOUT))
    {
        cprintf(ch, "You can't shout!!\r\n");
        return TRUE;
    }
    for (; *argument == ' '; argument++)
        ; /* skip leading spaces */

    if (ch->master && IS_AFFECTED(ch, AFF_CHARM))
    {
        cprintf(ch->master, "You pet is trying to shout....");
        return TRUE;
    }
    if (!(*argument))
    {
        cprintf(ch, "usage: shout <mesg>.\r\n");
        return TRUE;
    }

    if (IS_IMMORTAL(ch) || IS_NPC(ch))
    {
        cprintf(ch, "\x1b[1;32mYou shout '%s'\x1b[0m\r\n", argument);
        for (i = descriptor_list; i; i = i->next)
            if (i->character != ch && !i->connected && !IS_SET(i->character->specials.new_act, NEW_PLR_NOSHOUT) &&
                !IS_SET(i->character->specials.act, PLR_DEAF) && (rp = real_roomp(i->character->in_room)) &&
                //(!FindBoardInRoom(i->character->in_room))
                (!find_board_in_room(i->character->in_room)))
                act("\x1b[1;32m$n shouts '%s'\x1b[0m", 0, ch, 0, i->character, TO_VICT, argument);
    }
    else
    {
#ifdef OLD_SHOUT
        if (GET_MOVE(ch) < (GET_MAX_MOVE(ch) / 10))
        {
            cprintf(ch, "You are just too tired to shout anything.\r\n");
            return TRUE;
        }
        if (GET_MANA(ch) < (GET_MAX_MANA(ch) / 15))
        {
            cprintf(ch, "You can't seem to summon the energy to shout.\r\n");
            return TRUE;
        }
        if (GET_COND(ch, THIRST) < 4)
        {
            cprintf(ch, "Your throat is too dry to shout anything!\r\n");
            return TRUE;
        }
        GET_MOVE(ch) -= (GET_MAX_MOVE(ch) / 10);
        GET_MANA(ch) -= (GET_MAX_MANA(ch) / 15);
        GET_COND(ch, THIRST) -= 4;
        if (IS_SET(ch->specials.new_act, NEW_PLR_ECHO))
            cprintf(ch, "\x1b[1;32mYou shout '%s'\x1b[0m\r\n", argument);
        for (i = descriptor_list; i; i = i->next)
            if (i->character != ch && !i->connected && !IS_SET(i->character->specials.new_act, NEW_PLR_NOSHOUT) &&
                !IS_SET(i->character->specials.act, PLR_DEAF) && (rp = real_roomp(i->character->in_room)) &&
                (mrp = real_roomp(ch->in_room)) &&
                //(!FindBoardInRoom(i->character->in_room))
                (!find_board_in_room(i->character->in_room)))
                act("\x1b[1;32m$n shouts '%s'\x1b[0m", 0, ch, 0, i->character, TO_VICT, argument);
#endif
#ifdef ZONE_SHOUT
        if (GET_MOVE(ch) < (GET_MAX_MOVE(ch) / 10))
        {
            cprintf(ch, "You are just too tired to shout anything.\r\n");
            return TRUE;
        }
        if (GET_MANA(ch) < (GET_MAX_MANA(ch) / 15))
        {
            cprintf(ch, "You can't seem to summon the energy to shout.\r\n");
            return TRUE;
        }
        if (GET_COND(ch, THIRST) < 4)
        {
            cprintf(ch, "Your throat is too dry to shout anything!\r\n");
            return TRUE;
        }
        GET_MOVE(ch) -= (GET_MAX_MOVE(ch) / 10);
        GET_MANA(ch) -= (GET_MAX_MANA(ch) / 15);
        GET_COND(ch, THIRST) -= 4;
        if (IS_SET(ch->specials.new_act, NEW_PLR_ECHO))
            cprintf(ch, "\x1b[1;32mYou shout '%s'\x1b[0m\r\n", argument);
        for (i = descriptor_list; i; i = i->next)
            if (i->character != ch && !i->connected && !IS_SET(i->character->specials.new_act, NEW_PLR_NOSHOUT) &&
                !IS_SET(i->character->specials.act, PLR_DEAF) && (rp = real_roomp(i->character->in_room)) &&
                (mrp = real_roomp(ch->in_room)) && (rp->zone == mrp->zone) &&
                //(!FindBoardInRoom(i->character->in_room))
                (!find_board_in_room(i->character->in_room)))
                act("\x1b[1;32m$n shouts '%s'\x1b[0m", 0, ch, 0, i->character, TO_VICT, argument);
#endif
#ifdef RADIUS_SHOUT
        /* This is a buggy... it corrupts memory someplace */
        if (!(mrp = real_roomp(ch->in_room)))
        {
            cprintf(ch, "You are not in reality... this is bad.\r\n");
            return TRUE;
        }
        for (v = mrp->people; v; v = v->next_in_room)
            if (v != ch && v->desc && !IS_SET(v->specials.new_act, NEW_PLR_NOSHOUT) &&
                !IS_SET(v->specials.act, PLR_DEAF))
                act("\x1b[1;32m$n shouts '%s'\x1b[0m", 0, ch, 0, v, TO_VICT, argument);
        for (x = 0; x < MAX_NUM_EXITS; x++)
            if ((exitp = EXIT(ch, x)) && exit_ok(exitp, mrp))
                if ((rp = real_roomp(exitp->to_room)) &&
                    (rp != mrp)
                    //&& (!FindBoardInRoom(v->in_room))
                    && (!find_board_in_room(v->in_room)))
                {
                    for (v = rp->people; v; v = v->next_in_room)
                        if (v != ch && v->desc && !IS_SET(v->specials.new_act, NEW_PLR_NOSHOUT) &&
                            !IS_SET(v->specials.act, PLR_DEAF))
                            act("\x1b[1;32m$n shouts %s '%s'\x1b[0m", 0, ch, 0, v, TO_VICT, dir_from[rev_dir[x]],
                                argument);
                }
#endif
    }
    return TRUE;
}

int do_commune(struct char_data *ch, const char *argument, int cmd)
{
    struct descriptor_data *i = NULL;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    for (; *argument == ' '; argument++)
        ; /* skip leading spaces */

    if (!(*argument))
        cprintf(ch, "Communing among the gods is fine, but WHAT?\r\n");
    else
    {
        if (IS_NPC(ch) || IS_SET(ch->specials.new_act, NEW_PLR_ECHO))
        {
            cprintf(ch, "You wiz : '%s'\r\n", argument); /* part of wiznet */
        }
        for (i = descriptor_list; i; i = i->next)
            if (i->character != ch && !i->connected && !IS_SET(i->character->specials.new_act, NEW_PLR_NOSHOUT) &&
                (GetMaxLevel(i->character) >= LOW_IMMORTAL))
                act("$n : '%s'", 0, ch, 0, i->character, TO_VICT, argument);
    }
    return TRUE;
}

int do_tell(struct char_data *ch, const char *argument, int cmd)
{
    struct char_data *vict = NULL;
    char name[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char message[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    half_chop(argument, name, message);

    if (!*name || !*message)
    {
        cprintf(ch, "usage: tell <who> <mesg>.\r\n");
        return TRUE;
    }
    if (!(vict = get_char_vis(ch, name)))
    {
        cprintf(ch, "No-one by that name here..\r\n");
        return TRUE;
    }
    if (ch == vict)
    {
        cprintf(ch, "You try to tell yourself something.\r\n");
        return TRUE;
    }
    if (GET_POS(vict) == POSITION_SLEEPING)
    {
        act("$E is asleep, shhh.", FALSE, ch, 0, vict, TO_CHAR);
        return TRUE;
    }
    if (IS_NPC(vict) && !(vict->desc))
    {
        cprintf(ch, "No-one by that name here..\r\n");
        return TRUE;
    }
    if (!vict->desc)
    {
        cprintf(ch, "They can't hear you, link dead.\r\n");
        return TRUE;
    }
    if (IS_SET(vict->specials.new_act, NEW_PLR_NOTELL) && IS_MORTAL(ch))
    {
        cprintf(ch, "They are not recieving tells at the moment.\r\n");
        return TRUE;
    }
    switch (message[strlen(message) - 1])
    {
    case '!':
        cprintf(vict, "\x1b[1;33m%s exclaims to you '%s'\x1b[0m\r\n", NAME(ch), message);
        if (IS_NPC(ch) || IS_SET(ch->specials.new_act, NEW_PLR_ECHO))
            cprintf(ch, "\x1b[1;33mYou exclaim to %s '%s'\x1b[0m\r\n", NAME(vict), message);
        break;
    case '?':
        cprintf(vict, "\x1b[1;33m%s asks you '%s'\x1b[0m\r\n", NAME(ch), message);
        if (IS_NPC(ch) || IS_SET(ch->specials.new_act, NEW_PLR_ECHO))
            cprintf(ch, "\x1b[1;33mYou ask %s '%s'\x1b[0m\r\n", NAME(vict), message);
        break;
    default:
        cprintf(vict, "\x1b[1;33m%s tells you '%s'\x1b[0m\r\n", NAME(ch), message);
        if (IS_NPC(ch) || IS_SET(ch->specials.new_act, NEW_PLR_ECHO))
            cprintf(ch, "\x1b[1;33mYou tell %s '%s'\x1b[0m\r\n", NAME(vict), message);
        break;
    }
    return TRUE;
}

int do_whisper(struct char_data *ch, const char *argument, int cmd)
{
    struct char_data *vict = NULL;
    char name[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char message[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    half_chop(argument, name, message);

    if (!*name || !*message)
        cprintf(ch, "Who do you want to whisper to.. and what??\r\n");
    else if (!(vict = get_char_room_vis(ch, name)))
        cprintf(ch, "No-one by that name here..\r\n");
    else if (vict == ch)
    {
        act("$n whispers quietly to $mself.", FALSE, ch, 0, 0, TO_ROOM);
        cprintf(ch, "You can't seem to get your mouth close enough to your ear...\r\n");
    }
    else
    {
        act("\x1b[35m$n whispers to you, '%s'\x1b[0m", FALSE, ch, 0, vict, TO_VICT, message);
        if (IS_NPC(ch) || (IS_SET(ch->specials.new_act, NEW_PLR_ECHO)))
        {
            cprintf(ch, "\x1b[35mYou whisper to %s, '%s'\x1b[0m\r\n",
                    (IS_NPC(vict) ? vict->player.name : GET_NAME(vict)), message);
        }
        act("\x1b[35m$n whispers something to $N.\x1b[0m", FALSE, ch, 0, vict, TO_NOTVICT);
    }
    return TRUE;
}

int do_ask(struct char_data *ch, const char *argument, int cmd)
{
    struct char_data *vict = NULL;
    char name[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char message[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    half_chop(argument, name, message);

    if (!*name || !*message)
        cprintf(ch, "Who do you want to ask something.. and what??\r\n");
    else if (!(vict = get_char_room_vis(ch, name)))
        cprintf(ch, "No-one by that name here..\r\n");
    else if (vict == ch)
    {
        act("$n quietly asks $mself a question.", FALSE, ch, 0, 0, TO_ROOM);
        cprintf(ch, "You think about it for a while...\r\n");
    }
    else
    {
        act("$n asks you '%s'", FALSE, ch, 0, vict, TO_VICT, message);

        if (IS_NPC(ch) || (IS_SET(ch->specials.new_act, NEW_PLR_ECHO)))
        {
            cprintf(ch, "You ask %s, '%s'\r\n", (IS_NPC(vict) ? vict->player.name : GET_NAME(vict)), message);
        }
        act("$n asks $N a question.", FALSE, ch, 0, vict, TO_NOTVICT);
    }
    return TRUE;
}

int do_write(struct char_data *ch, const char *argument, int cmd)
{
    struct obj_data *paper = NULL;
    struct obj_data *pen = NULL;
    char papername[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char penname[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    argument_interpreter(argument, papername, penname);

    if (!ch->desc)
        return TRUE;

    if (!*papername)
    { /* nothing was delivered */
        cprintf(ch, "write (on) papername (with) penname.\r\n");
        return TRUE;
    }
    if (!*penname)
    {
        cprintf(ch, "write (on) papername (with) penname.\r\n");
        return TRUE;
    }
    if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying)))
    {
        cprintf(ch, "You have no %s.\r\n", papername);
        return TRUE;
    }
    if (!(pen = get_obj_in_list_vis(ch, penname, ch->carrying)))
    {
        cprintf(ch, "You have no %s.\r\n", penname);
        return TRUE;
    }
    /*
     * ok.. now let's see what kind of stuff we've found
     */
    if (pen->obj_flags.type_flag != ITEM_PEN)
    {
        act("$p is no good for writing with.", FALSE, ch, pen, 0, TO_CHAR);
    }
    else if (paper->obj_flags.type_flag != ITEM_NOTE)
    {
        act("You can't write on $p.", FALSE, ch, paper, 0, TO_CHAR);
    }
    else if (paper->action_description)
    {
        cprintf(ch, "There's something written on it already.\r\n");
        return TRUE;
    }
    else
    {
        /*
         * we can write - hooray!
         */
        cprintf(ch, "Ok.. go ahead and write.. end the note with a @.\r\n");
        act("$n begins to jot down a note.", TRUE, ch, 0, 0, TO_ROOM);
        ch->desc->str = &paper->action_description;
        ch->desc->max_str = MAX_NOTE_LENGTH;
    }
    return TRUE;
}

int do_group_tell(struct char_data *ch, const char *argument, int cmd)
{
    struct char_data *k = NULL;
    struct char_data *vict = NULL;
    struct follow_type *f = NULL;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (!*argument)
    {
        cprintf(ch, "usage: gtell <mesg>.\r\n");
        return TRUE;
    }
    if (!IS_AFFECTED(ch, AFF_GROUP))
    {
        cprintf(ch, "But you are a member of no group?!\r\n");
        return TRUE;
    }
    if (GET_POS(ch) == POSITION_SLEEPING)
    {
        cprintf(ch, "You are to tired to do this....\r\n");
        return TRUE;
    }
    if (!ch->master)
        k = ch;
    else
    {
        /*
         * tell the leader of the group
         */

        k = ch->master;
        if (IS_AFFECTED(k, AFF_GROUP))
            if ((GET_POS(k) != POSITION_SLEEPING) && (k->desc) && (k != ch))
                cprintf(k, "\x1b[0;33m%s tells the group '%s'\x1b[0m\r\n",
                        (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)), argument);
    }

    /*
     * tell the followers of the group and exclude ourselfs
     */

    for (f = k->followers; f; f = f->next)
    {
        vict = f->follower;
        if (IS_AFFECTED(vict, AFF_GROUP))
            if ((GET_POS(vict) != POSITION_SLEEPING) && (vict->desc) && (vict != ch))
                cprintf(vict, "\x1b[0;33m%s tells the group '%s'\x1b[0m\r\n",
                        (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)), argument);
    }

    if (IS_NPC(ch) || IS_SET(ch->specials.new_act, NEW_PLR_ECHO))
        cprintf(ch, "\x1b[0;33mYou tell the group '%s'\x1b[0m\r\n", argument);
    return TRUE;
}

int do_group_report(struct char_data *ch, const char *argument, int cmd)
{
    struct char_data *k = NULL;
    struct char_data *vict = NULL;
    struct follow_type *f = NULL;
    char message[MAX_INPUT_LENGTH + 20] = "\0\0\0\0\0\0\0";

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (!IS_AFFECTED(ch, AFF_GROUP))
    {
        cprintf(ch, "But you are a member of no group?!\r\n");
        return TRUE;
    }
    if (GET_POS(ch) == POSITION_SLEEPING)
    {
        cprintf(ch, "You are to tired to do this....\r\n");
        return TRUE;
    }
    snprintf(message, MAX_INPUT_LENGTH, "\r\nGroup Report from:Name:%s Hits:%d(%d) Mana:%d(%d) Move:%d(%d)",
             GET_NAME(ch), GET_HIT(ch), GET_MAX_HIT(ch), GET_MANA(ch), GET_MAX_MANA(ch), GET_MOVE(ch),
             GET_MAX_MOVE(ch));

    if (!ch->master)
        k = ch;
    else
    {
        /*
         * tell the leader of the group
         */

        k = ch->master;
        if (IS_AFFECTED(k, AFF_GROUP))
            if ((GET_POS(k) != POSITION_SLEEPING) && (k->desc) && (k != ch))
                cprintf(k, "%s", message);
    }

    /*
     * tell the followers of the group and exclude ourselfs
     */

    for (f = k->followers; f; f = f->next)
    {
        vict = f->follower;
        if (IS_AFFECTED(vict, AFF_GROUP))
            if ((GET_POS(vict) != POSITION_SLEEPING) && (vict->desc) && (vict != ch))
                cprintf(vict, "%s", message);
    }

    if (IS_NPC(ch) || IS_SET(ch->specials.new_act, NEW_PLR_ECHO))
        cprintf(ch, "You tell the group your stats.\r\n");
    return TRUE;
}

int do_split(struct char_data *ch, const char *argument, int cmd)
{
    struct char_data *vict = NULL;
    struct follow_type *f = NULL;
    int amount = 0;
    int count = 0;
    int share = 0;
    char blah[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (!IS_AFFECTED(ch, AFF_GROUP))
    {
        cprintf(ch, "You are a member of no group!\r\n");
        return TRUE;
    }
    if (ch->master)
    {
        cprintf(ch, "You must be the leader of the group to use this.\r\n");
        return TRUE;
    }
    one_argument(argument, blah);

    if (!is_number(blah))
    {
        cprintf(ch, "You must supply an integer amount.\r\n");
        return TRUE;
    }
    amount = atoi(blah);
    if (amount > GET_GOLD(ch))
    {
        cprintf(ch, "You do not have that much gold.\r\n");
        return TRUE;
    }
    GET_GOLD(ch) -= amount;
    count = 1;

    for (f = ch->followers; f; f = f->next)
    {
        vict = f->follower;
        if (IS_AFFECTED(vict, AFF_GROUP))
        {
            if ((vict->desc) && (vict != ch))
                count++;
        }
    }

    share = amount / count;
    GET_GOLD(ch) += share;

    cprintf(ch, "You split the gold into shares of %d coin(s)\r\n", share);

    for (f = ch->followers; f; f = f->next)
    {
        vict = f->follower;
        if (IS_AFFECTED(vict, AFF_GROUP))
        {
            if ((vict->desc) && (vict != ch))
            {
                cprintf(ch, "%s gives you %d coins.\r\n", (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)), share);
                GET_GOLD(vict) += share;
            }
        }
    }
    return TRUE;
}

int do_land(struct char_data *ch, const char *argument, int cmd)
{
    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (IS_NOT_SET(ch->specials.affected_by, AFF_FLYING))
    {
        cprintf(ch, "But you are not flying?\r\n");
        return TRUE;
    }
    act("$n slowly lands on the ground.", FALSE, ch, 0, 0, TO_ROOM);

    if (affected_by_spell(ch, SPELL_FLY))
        affect_from_char(ch, SPELL_FLY);
    if (IS_SET(ch->specials.affected_by, AFF_FLYING))
        REMOVE_BIT(ch->specials.affected_by, AFF_FLYING);

    cprintf(ch, "You feel the extreme pull of gravity...\r\n");
    return TRUE;
}

int do_invis_off(struct char_data *ch, const char *argument, int cmd)
{
    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (!IS_SET(ch->specials.affected_by, AFF_INVISIBLE))
    {
        cprintf(ch, "But you are not invisible?\r\n");
        return TRUE;
    }
    act("$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM);
    act("You feel exposed.", FALSE, ch, 0, 0, TO_CHAR);

    if (affected_by_spell(ch, SPELL_INVISIBLE))
        affect_from_char(ch, SPELL_INVISIBLE);

    if (IS_SET(ch->specials.affected_by, AFF_INVISIBLE))
        REMOVE_BIT(ch->specials.affected_by, AFF_INVISIBLE);
    return TRUE;
}

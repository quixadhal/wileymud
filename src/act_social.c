/*
 * file: act.social.c , Implementation of commands.       Part of DIKUMUD
 * Usage : Social commands.
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */

#include <stdio.h>
#include <stdlib.h>
/* #include <unistd.h> */
#include <sys/types.h>
#include <signal.h>
#include <string.h>

#include "global.h"
#include "bug.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "multiclass.h"
#define _ACT_SOCIAL_C
#include "act_social.h"

struct social_messg *soc_mess_list = 0;
struct pose_type pose_messages[MAX_MESSAGES];
static int list_top = -1;

char *fread_action(FILE *fl)
{
    char buf[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    char *rslt = NULL;

    if (DEBUG > 2)
        log_info("called %s with %08zx", __PRETTY_FUNCTION__, (size_t)fl);

    for (;;)
    {
        fgets(buf, MAX_STRING_LENGTH, fl);
        if (feof(fl))
        {
            log_fatal("Fread_action - unexpected EOF.");
            proper_exit(MUD_HALT);
        }
        if (*buf == '#')
            return (0);
        else
        {
            *(buf + strlen(buf) - 1) = '\0';
            rslt = strdup(buf);
            return (rslt);
        }
    }
}

void boot_social_messages(void)
{
    FILE *fl = NULL;
    int tmp = 0;
    int hide = 0;
    int min_pos = 0;

    if (DEBUG > 1)
        log_info("called %s with no arguments", __PRETTY_FUNCTION__);

    if (!(fl = fopen(SOCMESS_FILE, "r")))
    {
        log_fatal("boot_social_messages");
        proper_exit(MUD_HALT);
    }
    for (;;)
    {
        fscanf(fl, " %d ", &tmp);
        if (tmp < 0)
            break;
        fscanf(fl, " %d ", &hide);
        fscanf(fl, " %d \n", &min_pos);

        /*
         * alloc a new cell
         */
        if (!soc_mess_list)
        {
            CREATE(soc_mess_list, struct social_messg, ((list_top = 0), 1));
        }
        else
        {
            RECREATE(soc_mess_list, struct social_messg, (++list_top + 1));
        }

        /*
         * read the stuff
         */
        soc_mess_list[list_top].act_nr = tmp;
        soc_mess_list[list_top].hide = hide;
        soc_mess_list[list_top].min_victim_position = min_pos;

        soc_mess_list[list_top].char_no_arg = fread_action(fl);
        soc_mess_list[list_top].others_no_arg = fread_action(fl);

        soc_mess_list[list_top].char_found = fread_action(fl);

        /*
         * if no char_found, the rest is to be ignored
         */
        if (!soc_mess_list[list_top].char_found)
            continue;

        soc_mess_list[list_top].others_found = fread_action(fl);
        soc_mess_list[list_top].vict_found = fread_action(fl);

        soc_mess_list[list_top].not_found = fread_action(fl);

        soc_mess_list[list_top].char_auto = fread_action(fl);

        soc_mess_list[list_top].others_auto = fread_action(fl);
    }
    FCLOSE(fl);
}

int find_action(int cmd)
{
    int bot = 0;
    int top = 0;
    int mid = 0;

    if (DEBUG > 2)
        log_info("called %s with %d", __PRETTY_FUNCTION__, cmd);

    top = list_top;

    if (top < 0)
        return (-1);

    for (;;)
    {
        mid = (bot + top) / 2;

        if (soc_mess_list[mid].act_nr == cmd)
            return (mid);
        if (bot == top)
            return (-1);

        if (soc_mess_list[mid].act_nr > cmd)
            top = --mid;
        else
            bot = ++mid;
    }
}

void do_action(struct char_data *ch, const char *argument, int cmd)
{
    int act_nr = 0;
    char buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    struct social_messg *action = NULL;
    struct char_data *vict = NULL;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if ((act_nr = find_action(cmd)) < 0)
    {
        cprintf(ch, "That action is not supported.\r\n");
        return;
    }
    action = &soc_mess_list[act_nr];

    if (action->char_found)
        only_argument(argument, buf);
    else
        *buf = '\0';

    if (!*buf)
    {
        cprintf(ch, "%s\r\n", action->char_no_arg);
        act("%s", action->hide, ch, 0, 0, TO_ROOM, action->others_no_arg);
        return;
    }
    if (!(vict = get_char_room_vis(ch, buf)))
    {
        cprintf(ch, "%s\r\n", action->not_found);
    }
    else if (vict == ch)
    {
        cprintf(ch, "%s\r\n", action->char_auto);
        act("%s", action->hide, ch, 0, 0, TO_ROOM, action->others_auto);
    }
    else
    {
        if (GET_POS(vict) < action->min_victim_position)
        {
            act("$N is not in a proper position for that.", FALSE, ch, 0, vict, TO_CHAR);
        }
        else
        {
            act("%s", 0, ch, 0, vict, TO_CHAR, action->char_found);
            act("%s", action->hide, ch, 0, vict, TO_NOTVICT, action->others_found);
            act("%s", action->hide, ch, 0, vict, TO_VICT, action->vict_found);
        }
    }
}

void do_insult(struct char_data *ch, const char *argument, int cmd)
{
    static char arg[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    struct char_data *victim = NULL;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    only_argument(argument, arg);

    if (*arg)
    {
        if (!(victim = get_char_room_vis(ch, arg)))
        {
            cprintf(ch, "Can't hear you!\r\n");
        }
        else
        {
            if (victim != ch)
            {
                cprintf(ch, "You insult %s.\r\n", GET_NAME(victim));

                switch (random() % 3)
                {
                case 0: {
                    if (GET_SEX(ch) == SEX_MALE)
                    {
                        if (GET_SEX(victim) == SEX_MALE)
                            act("$n accuses you of fighting like a woman!", FALSE, ch, 0, victim, TO_VICT);
                        else
                            act("$n says that women can't fight.", FALSE, ch, 0, victim, TO_VICT);
                    }
                    else
                    { /* Ch == Woman */
                        if (GET_SEX(victim) == SEX_MALE)
                            act("$n accuses you of having the smallest.... (brain?)", FALSE, ch, 0, victim, TO_VICT);
                        else
                            act("$n tells you that you'd loose a beautycontest against a troll.", FALSE, ch, 0, victim,
                                TO_VICT);
                    }
                }
                break;
                case 1: {
                    act("$n calls your mother a bitch!", FALSE, ch, 0, victim, TO_VICT);
                }
                break;
                default: {
                    act("$n tells you to get lost!", FALSE, ch, 0, victim, TO_VICT);
                }
                break;
                } /* end switch */

                act("$n insults $N.", TRUE, ch, 0, victim, TO_NOTVICT);
            }
            else
            { /* ch == victim */
                cprintf(ch, "You feel insulted.\r\n");
            }
        }
    }
    else
        cprintf(ch, "Sure you don't want to insult everybody.\r\n");
}

void boot_pose_messages(void)
{
    FILE *fl = NULL;
    int counter = 0;
    int chclass = 0;

    if (DEBUG > 1)
        log_info("called %s with no arguments", __PRETTY_FUNCTION__);

    return;

    if (!(fl = fopen(POSEMESS_FILE, "r")))
    {
        log_fatal("boot_pose_messages");
        proper_exit(MUD_HALT);
    }
    for (counter = 0;; counter++)
    {
        fscanf(fl, " %d ", &pose_messages[counter].level);
        if (pose_messages[counter].level < 0)
            break;
        for (chclass = 0; chclass < 4; chclass ++)
        {
            pose_messages[counter].poser_msg[chclass] = fread_action(fl);
            pose_messages[counter].room_msg[chclass] = fread_action(fl);
        }
    }

    FCLOSE(fl);
}

void do_pose(struct char_data *ch, const char *argument, int cmd)
{
    /*
     * int to_pose = 0;
     */
    int counter = 0;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    cprintf(ch, "Sorry Buggy command.\r\n");
    return;

    if ((GetMaxLevel(ch) < pose_messages[0].level) || IS_NPC(ch))
    {
        cprintf(ch, "You can't do that.\r\n");
        return;
    }
    for (counter = 0; (pose_messages[counter].level < GetMaxLevel(ch)) && (pose_messages[counter].level > 0); counter++)
        ;
    counter--;

    /*
     * to_pose = number(0, counter);
     */
    /*
     * **  find highest level, use that.
     */
}

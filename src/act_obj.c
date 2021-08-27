/*
 * file: act.obj1.c , Implementation of commands.         Part of DIKUMUD
 * Usage : Commands mainly moving around objects.
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */

#include <stdio.h>
#include <stdlib.h>
/* #include <unistd.h> */
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "global.h"
#include "bug.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "trap.h"
#include "spell_parser.h"
#include "multiclass.h"
#include "mudlimits.h"
#include "fight.h"
#include "act_info.h"
#define _ACT_OBJ_C
#include "act_obj.h"

/* procedures related to get */
void get(struct char_data *ch, struct obj_data *obj_object, struct obj_data *sub_object)
{
    if (DEBUG > 2)
        log_info("called %s with %s, %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), SAFE_ONAME(obj_object),
                 SAFE_ONAME(sub_object));

    if (sub_object)
    {
        if (!IS_SET(sub_object->obj_flags.value[1], CONT_CLOSED))
        {
            obj_from_obj(obj_object);
            obj_to_char(obj_object, ch);
            act("You get $p from $P.", 0, ch, obj_object, sub_object, TO_CHAR);
            act("$n gets $p from $P.", 1, ch, obj_object, sub_object, TO_ROOM);
        }
        else
        {
            act("$P must be opened first.", 1, ch, 0, sub_object, TO_CHAR);
            return;
        }
    }
    else
    {
        /*  jdb -- 11-9 */
        if (obj_object->in_room == NOWHERE)
        {
            obj_object->in_room = ch->in_room;
            log_error("OBJ:%s got %s from room -1...", GET_NAME(ch), obj_object->name);
        }
        obj_from_room(obj_object);
        obj_to_char(obj_object, ch);
        act("You get $p.", 0, ch, obj_object, 0, TO_CHAR);
        act("$n gets $p.", 1, ch, obj_object, 0, TO_ROOM);
    }
    if ((obj_object->obj_flags.type_flag == ITEM_MONEY) && (obj_object->obj_flags.value[0] >= 1))
    {
        obj_from_char(obj_object);
        cprintf(ch, "There was %d coins.\r\n", obj_object->obj_flags.value[0]);
        GET_GOLD(ch) += obj_object->obj_flags.value[0];
        extract_obj(obj_object);
    }
}

int do_get(struct char_data *ch, const char *argument, int cmd)
{
    char arg1[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    char arg2[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    struct obj_data *sub_object = NULL;
    struct obj_data *obj_object = NULL;
    struct obj_data *next_obj = NULL;
    char found = FALSE;
    char fail = FALSE;
    int type = 3;
    char newarg[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    int num = 0;
    int p = 0;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    argument_interpreter(argument, arg1, arg2);

    /*
     * get type
     */
    if (!*arg1)
        type = 0;

    if (*arg1 && !*arg2)
    {
        if (!str_cmp(arg1, "all"))
            type = 1;
        else
            type = 2;
    }
    if (*arg1 && *arg2)
    {
        if (!str_cmp(arg1, "all"))
        {
            if (!str_cmp(arg2, "all"))
                type = 3;
            else
                type = 4;
        }
        else
        {
            if (!str_cmp(arg2, "all"))
                type = 5;
            else
                type = 6;
        }
    }
    switch (type)
    {
        /*
         * get
         */
    case 0: {
        cprintf(ch, "Get what?\r\n");
    }
    break;
        /*
         * get all
         */
    case 1: {
        sub_object = 0;
        found = FALSE;
        fail = FALSE;
        for (obj_object = real_roomp(ch->in_room)->contents; obj_object; obj_object = next_obj)
        {
            next_obj = obj_object->next_content;
            /*
             * check for a trap (traps fire often)
             */
            if (CheckForAnyTrap(ch, obj_object))
                return TRUE;

            if (CAN_SEE_OBJ(ch, obj_object))
            {
                if ((IS_CARRYING_N(ch) + 1) <= CAN_CARRY_N(ch))
                {
                    if ((IS_CARRYING_W(ch) + obj_object->obj_flags.weight) <= CAN_CARRY_W(ch))
                    {
                        if (CAN_WEAR(obj_object, ITEM_TAKE))
                        {
                            get(ch, obj_object, sub_object);
                            found = TRUE;
                        }
                        else
                        {
                            cprintf(ch, "You can't take that\r\n");
                            fail = TRUE;
                        }
                    }
                    else
                    {
                        cprintf(ch, "%s : You can't carry that much weight.\r\n", obj_object->short_description);
                        fail = TRUE;
                    }
                }
                else
                {
                    cprintf(ch, "%s : You can't carry that many items.\r\n", obj_object->short_description);
                    fail = TRUE;
                }
            }
        }
        if (found)
        {
            cprintf(ch, "OK.\r\n");
        }
        else
        {
            if (!fail)
                cprintf(ch, "You see nothing here.\r\n");
        }
    }
    break;
        /*
         * get ??? (something)
         */
    case 2: {
        sub_object = 0;
        found = FALSE;
        fail = FALSE;

        if (getall(arg1, newarg) != FALSE)
        {
            strlcpy(arg1, newarg, MAX_STRING_LENGTH);
            num = -1;
        }
        else if ((p = getabunch(arg1, newarg)) != FALSE)
        {
            strlcpy(arg1, newarg, MAX_STRING_LENGTH);
            num = p;
        }
        else
        {
            num = 1;
        }

        while (num != 0)
        {
            obj_object = get_obj_in_list_vis(ch, arg1, real_roomp(ch->in_room)->contents);
            if (obj_object)
            {
                if (CheckForAnyTrap(ch, obj_object))
                    return TRUE;

                if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch)))
                {
                    if ((IS_CARRYING_W(ch) + obj_object->obj_flags.weight) < CAN_CARRY_W(ch))
                    {
                        if (CAN_WEAR(obj_object, ITEM_TAKE))
                        {
                            get(ch, obj_object, sub_object);
                            found = TRUE;
                        }
                        else
                        {
                            cprintf(ch, "You can't take that\r\n");
                            fail = TRUE;
                            num = 0;
                        }
                    }
                    else
                    {
                        cprintf(ch, "%s : You can't carry that much weight.\r\n", obj_object->short_description);
                        fail = TRUE;
                        num = 0;
                    }
                }
                else
                {
                    cprintf(ch, "%s : You can't carry that many items.\r\n", obj_object->short_description);
                    fail = TRUE;
                    num = 0;
                }
            }
            else
            {
                if (num > 0)
                {
                    cprintf(ch, "You do not see a %s here.\r\n", arg1);
                }
                num = 0;
                fail = TRUE;
            }
            if (num > 0)
                num--;
        }
    }
    break;
        /*
         * get all all
         */
    case 3: {
        cprintf(ch, "You must be joking?!\r\n");
    }
    break;
        /*
         * get all ???
         */
    case 4: {
        found = FALSE;
        fail = FALSE;
        sub_object = (struct obj_data *)get_obj_vis_accessible(ch, arg2);
        if (sub_object)
        {
            if (GET_ITEM_TYPE(sub_object) == ITEM_CONTAINER)
            {
                for (obj_object = sub_object->contains; obj_object; obj_object = next_obj)
                {
                    if (CheckForGetTrap(ch, obj_object))
                        return TRUE;

                    next_obj = obj_object->next_content;
                    if (CAN_SEE_OBJ(ch, obj_object))
                    {
                        if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch)))
                        {
                            if ((IS_CARRYING_W(ch) + obj_object->obj_flags.weight) < CAN_CARRY_W(ch))
                            {
                                if (CAN_WEAR(obj_object, ITEM_TAKE))
                                {
                                    get(ch, obj_object, sub_object);
                                    found = TRUE;
                                }
                                else
                                {
                                    cprintf(ch, "You can't take that\r\n");
                                    fail = TRUE;
                                }
                            }
                            else
                            {
                                cprintf(ch, "%s : You can't carry that much weight.\r\n",
                                        obj_object->short_description);
                                fail = TRUE;
                            }
                        }
                        else
                        {
                            cprintf(ch, "%s : You can't carry that many items.\r\n", obj_object->short_description);
                            fail = TRUE;
                        }
                    }
                }
                if (!found && !fail)
                {
                    cprintf(ch, "You do not see anything in %s.\r\n", sub_object->short_description);
                    fail = TRUE;
                }
            }
            else
            {
                cprintf(ch, "%s is not a container.\r\n", sub_object->short_description);
                fail = TRUE;
            }
        }
        else
        {
            cprintf(ch, "You do not see or have the %s.\r\n", arg2);
            fail = TRUE;
        }
    }
    break;
    case 5: {
        cprintf(ch, "You can't take a thing from more than one container.\r\n");
    }
    break;
        /*
         * take ??? from ???   (is it??)
         */

    case 6: {
        found = FALSE;
        fail = FALSE;
        sub_object = (struct obj_data *)get_obj_vis_accessible(ch, arg2);
        if (sub_object)
        {
            if (GET_ITEM_TYPE(sub_object) == ITEM_CONTAINER)
            {

                if (getall(arg1, newarg) != FALSE)
                {
                    num = -1;
                    strlcpy(arg1, newarg, MAX_STRING_LENGTH);
                }
                else if ((p = getabunch(arg1, newarg)) != FALSE)
                {
                    num = p;
                    strlcpy(arg1, newarg, MAX_STRING_LENGTH);
                }
                else
                {
                    num = 1;
                }
                while (num != 0)
                {
                    obj_object = get_obj_in_list_vis(ch, arg1, sub_object->contains);
                    if (obj_object)
                    {
                        if (CheckForInsideTrap(ch, sub_object))
                            return TRUE;
                        if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch)))
                        {
                            if ((IS_CARRYING_W(ch) + obj_object->obj_flags.weight) < CAN_CARRY_W(ch))
                            {
                                if (CAN_WEAR(obj_object, ITEM_TAKE))
                                {
                                    get(ch, obj_object, sub_object);
                                    found = TRUE;
                                }
                                else
                                {
                                    cprintf(ch, "You can't take that\r\n");
                                    fail = TRUE;
                                    num = 0;
                                }
                            }
                            else
                            {
                                cprintf(ch, "%s : You can't carry that much weight.\r\n",
                                        obj_object->short_description);
                                fail = TRUE;
                                num = 0;
                            }
                        }
                        else
                        {
                            cprintf(ch, "%s : You can't carry that many items.\r\n", obj_object->short_description);
                            fail = TRUE;
                            num = 0;
                        }
                    }
                    else
                    {
                        if (num > 0)
                        {
                            cprintf(ch, "%s does not contain the %s.\r\n", sub_object->short_description, arg1);
                        }
                        num = 0;
                        fail = TRUE;
                    }
                    if (num > 0)
                        num--;
                }
            }
            else
            {
                cprintf(ch, "%s is not a container.\r\n", sub_object->short_description);
                fail = TRUE;
            }
        }
        else
        {
            cprintf(ch, "You do not see or have the %s.\r\n", arg2);
            fail = TRUE;
        }
    }
    break;
    }
    return TRUE;
}

int do_drop(struct char_data *ch, const char *argument, int cmd)
{
    struct obj_data *tmp_object = NULL;
    struct obj_data *next_obj = NULL;
    const char *s = NULL;
    char arg[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char newarg[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    int test = FALSE;
    int amount = 0;
    int num = 0;
    int p = 0;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    s = one_argument(argument, arg);
    if (is_number(arg))
    {
        amount = atoi(arg);
        strlcpy(arg, s, MAX_INPUT_LENGTH);

        /*
         * if (0!=str_cmp("coins",arg) && 0!=str_cmp("coin",arg))  {
         * cprintf(ch, "Sorry, you can't do that (yet)...\r\n");
         * return TRUE;
         * }
         */

        if (amount < 0)
        {
            cprintf(ch, "Sorry, you can't do that!\r\n");
            return TRUE;
        }
        if (GET_GOLD(ch) < amount)
        {
            cprintf(ch, "You haven't got that many coins!\r\n");
            return TRUE;
        }
        cprintf(ch, "OK.\r\n");
        if (amount == 0)
            return TRUE;

        act("$n drops some gold.", FALSE, ch, 0, 0, TO_ROOM);
        tmp_object = create_money(amount);
        obj_to_room(tmp_object, ch->in_room);
        GET_GOLD(ch) -= amount;
        return TRUE;
    }
    else
    {
        only_argument(argument, arg);
    }

    if (*arg)
    {
        if (!str_cmp(arg, "all"))
        {
            for (tmp_object = ch->carrying; tmp_object; tmp_object = next_obj)
            {
                next_obj = tmp_object->next_content;
                if (!IS_SET(tmp_object->obj_flags.extra_flags, ITEM_NODROP) || IS_IMMORTAL(ch))
                {
                    if (CAN_SEE_OBJ(ch, tmp_object))
                    {
                        cprintf(ch, "You drop %s.\r\n", tmp_object->short_description);
                    }
                    else
                    {
                        cprintf(ch, "You drop something.\r\n");
                    }
                    act("$n drops $p.", 1, ch, tmp_object, 0, TO_ROOM);
                    obj_from_char(tmp_object);
                    obj_to_room(tmp_object, ch->in_room);
                    test = TRUE;
                }
                else
                {
                    if (CAN_SEE_OBJ(ch, tmp_object))
                    {
                        cprintf(ch, "You can't drop  %s, it must be CURSED!\r\n", tmp_object->short_description);
                        test = TRUE;
                    }
                }
            }
            if (!test)
            {
                cprintf(ch, "You do not seem to have anything.\r\n");
            }
        }
        else
        {
            /*
             * &&&&&&
             */
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
            while (num != 0)
            {
                tmp_object = get_obj_in_list_vis(ch, arg, ch->carrying);
                if (tmp_object)
                {
                    if (!IS_SET(tmp_object->obj_flags.extra_flags, ITEM_NODROP) || IS_IMMORTAL(ch))
                    {
                        cprintf(ch, "You drop %s.\r\n", tmp_object->short_description);
                        act("$n drops $p.", 1, ch, tmp_object, 0, TO_ROOM);
                        obj_from_char(tmp_object);
                        obj_to_room(tmp_object, ch->in_room);
                    }
                    else
                    {
                        cprintf(ch, "You can't drop it, it must be CURSED!\r\n");
                        num = 0;
                    }
                }
                else
                {
                    if (num > 0)
                        cprintf(ch, "You do not have that item.\r\n");
                    num = 0;
                }
                if (num > 0)
                    num--;
            }
        }
    }
    else
    {
        cprintf(ch, "Drop what?\r\n");
    }
    return TRUE;
}

int do_put(struct char_data *ch, const char *argument, int cmd)
{
    struct obj_data *obj_object = NULL;
    struct obj_data *sub_object = NULL;
    struct char_data *tmp_char = NULL;
    char arg1[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char arg2[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char newarg[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    int bits = 0;
    int num = 0;
    int p = 0;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    argument_interpreter(argument, arg1, arg2);

    if (*arg1)
    {
        if (*arg2)
        {

            if (getall(arg1, newarg) != FALSE)
            {
                num = -1;
                strlcpy(arg1, newarg, MAX_INPUT_LENGTH);
            }
            else if ((p = getabunch(arg1, newarg)) != FALSE)
            {
                num = p;
                strlcpy(arg1, newarg, MAX_INPUT_LENGTH);
            }
            else
            {
                num = 1;
            }

            if (!strcmp(arg1, "all"))
            {

                cprintf(ch, "sorry, you can't do that (yet)\r\n");
                return TRUE;
            }
            else
            {
                while (num != 0)
                {
#if 1
                    bits = generic_find(arg1, FIND_OBJ_INV, ch, &tmp_char, &obj_object);
#else
                    obj_object = get_obj_in_list_vis(ch, arg1, ch->carrying);
#endif

                    if (obj_object)
                    {
                        bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &tmp_char, &sub_object);
                        if (sub_object)
                        {
                            if (GET_ITEM_TYPE(sub_object) == ITEM_CONTAINER)
                            {
                                if (!IS_SET(sub_object->obj_flags.value[1], CONT_CLOSED))
                                {
                                    if (obj_object == sub_object)
                                    {
                                        cprintf(ch, "You attempt to fold it into itself, but fail.\r\n");
                                        return TRUE;
                                    }
                                    if (((sub_object->obj_flags.weight) + (obj_object->obj_flags.weight)) <
                                        (sub_object->obj_flags.value[0]))
                                    {
                                        act("You put $p in $P", TRUE, ch, obj_object, sub_object, TO_CHAR);
                                        if (bits == FIND_OBJ_INV)
                                        {
                                            obj_from_char(obj_object);
                                            /*
                                             * make up for above line
                                             */
                                            IS_CARRYING_W(ch) += GET_OBJ_WEIGHT(obj_object);

                                            obj_to_obj(obj_object, sub_object);
                                        }
                                        else
                                        {
                                            obj_from_room(obj_object);
                                            obj_to_obj(obj_object, sub_object);
                                        }

                                        act("$n puts $p in $P", TRUE, ch, obj_object, sub_object, TO_ROOM);
                                        num--;
                                    }
                                    else
                                    {
                                        cprintf(ch, "It won't fit.\r\n");
                                        num = 0;
                                    }
                                }
                                else
                                {
                                    cprintf(ch, "It seems to be closed.\r\n");
                                    num = 0;
                                }
                            }
                            else
                            {
                                cprintf(ch, "%s is not a container.\r\n", sub_object->short_description);
                                num = 0;
                            }
                        }
                        else
                        {
                            cprintf(ch, "You don't have the %s.\r\n", arg2);
                            num = 0;
                        }
                    }
                    else
                    {
                        if ((num > 0) || (num == -1))
                        {
                            cprintf(ch, "You don't have the %s.\r\n", arg1);
                        }
                        num = 0;
                    }
                }
            }
        }
        else
        {
            cprintf(ch, "Put %s in what?\r\n", arg1);
        }
    }
    else
    {
        cprintf(ch, "Put what in what?\r\n");
    }
    return TRUE;
}

int do_give(struct char_data *ch, const char *argument, int cmd)
{
    struct char_data *vict = NULL;
    struct obj_data *obj = NULL;
    char obj_name[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char vict_name[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char arg[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char newarg[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    int amount = 0;
    int num = 0;
    int p = 0;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    argument = one_argument(argument, obj_name);
    if (is_number(obj_name))
    {
        amount = atoi(obj_name);
        argument = one_argument(argument, arg);
        /*
         * if (str_cmp("coins",arg) && str_cmp("coin",arg))
         * {
         * cprintf(ch, "Sorry, you can't do that (yet)...\r\n");
         * return TRUE;
         * }
         */

        if (amount < 0)
        {
            cprintf(ch, "Sorry, you can't do that!\r\n");
            return TRUE;
        }
        if ((GET_GOLD(ch) < amount) && (IS_NPC(ch) || (GetMaxLevel(ch) < DEMIGOD)))
        {
            cprintf(ch, "You haven't got that many coins!\r\n");
            return TRUE;
        }
        argument = one_argument(argument, vict_name);

        if (!*vict_name)
        {
            cprintf(ch, "To who?\r\n");
            return TRUE;
        }
        if (!(vict = get_char_room_vis(ch, vict_name)))
        {
            cprintf(ch, "To who?\r\n");
            return TRUE;
        }
        cprintf(ch, "Ok.\r\n");
        cprintf(vict, "%s gives you %d gold coins.\r\n", PERS(ch, vict), amount);
        act("$n gives some gold to $N.", 1, ch, 0, vict, TO_NOTVICT);
        if (IS_NPC(ch) || (GetMaxLevel(ch) < DEMIGOD))
            GET_GOLD(ch) -= amount;
        GET_GOLD(vict) += amount;
        if ((amount > 1000) && (GetMaxLevel(ch) >= DEMIGOD))
        { /* hmmm */
            log_info("%s gave %d coins to %s", GET_NAME(ch), amount, GET_NAME(vict));
        }
        return TRUE;
    }
    argument = one_argument(argument, vict_name);
    if (!*obj_name || !*vict_name)
    {
        cprintf(ch, "Give what to who?\r\n");
        return TRUE;
    }
    /*
     * &&&&
     */
    if (getall(obj_name, newarg) != FALSE)
    {
        num = -1;
        strlcpy(obj_name, newarg, MAX_INPUT_LENGTH);
    }
    else if ((p = getabunch(obj_name, newarg)) != FALSE)
    {
        num = p;
        strlcpy(obj_name, newarg, MAX_INPUT_LENGTH);
    }
    else
    {
        num = 1;
    }

    while (num != 0)
    {
        if (!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying)))
        {
            if (num >= -1)
                cprintf(ch, "You do not seem to have anything like that.\r\n");
            return TRUE;
        }
        if (IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP) && !IS_IMMORTAL(ch))
        {
            cprintf(ch, "You can't let go of it! Yeech!!\r\n");
            return TRUE;
        }
        if (!(vict = get_char_room_vis(ch, vict_name)))
        {
            cprintf(ch, "No one by that name around here.\r\n");
            return TRUE;
        }
        if (vict == ch)
        {
            log_info("%s just tried to give all.X to %s", GET_NAME(ch), GET_NAME(ch));
            cprintf(ch, "Ok.\r\n");
            return TRUE;
        }
        if ((1 + IS_CARRYING_N(vict)) > CAN_CARRY_N(vict))
        {
            act("$N seems to have $S hands full.", 0, ch, 0, vict, TO_CHAR);
            return TRUE;
        }
        if (obj->obj_flags.weight + IS_CARRYING_W(vict) > CAN_CARRY_W(vict))
        {
            act("$E can't carry that much weight.", 0, ch, 0, vict, TO_CHAR);
            return TRUE;
        }
        obj_from_char(obj);
        obj_to_char(obj, vict);
        act("$n gives $p to $N.", 1, ch, obj, vict, TO_NOTVICT);
        act("$n gives you $p.", 0, ch, obj, vict, TO_VICT);
        act("You give $p to $N", 0, ch, obj, vict, TO_CHAR);

        if (num > 0)
            num--;
    }
    return TRUE;
}

/*
 * file: act.obj2.c , Implementation of commands.         Part of DIKUMUD
 * Usage : Commands mainly using objects.
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */

void weight_change_object(struct obj_data *obj, int weight)
{
    struct obj_data *tmp_obj = NULL;
    struct char_data *tmp_ch = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_ONAME(obj), weight);

    if (obj->in_room != NOWHERE)
    {
        GET_OBJ_WEIGHT(obj) += weight;
    }
    else if ((tmp_ch = obj->carried_by))
    {
        obj_from_char(obj);
        GET_OBJ_WEIGHT(obj) += weight;
        obj_to_char(obj, tmp_ch);
    }
    else if ((tmp_obj = obj->in_obj))
    {
        obj_from_obj(obj);
        GET_OBJ_WEIGHT(obj) += weight;
        obj_to_obj(obj, tmp_obj);
    }
    else
    {
        log_error("Unknown attempt to subtract weight from an object.");
    }
}

void name_from_drinkcon(struct obj_data *obj)
{
    int i = 0;
    char buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char *new_name = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_ONAME(obj));

    one_argument(obj->name, buf);
    new_name = strdup(buf);
    DESTROY(obj->name);
    obj->name = new_name;
    return;

    for (i = 0; (*((obj->name) + i) != ' ') && (*((obj->name) + i) != '\0'); i++)
        ;
    if (*((obj->name) + i) == ' ')
    {
        *((obj->name) + i + 1) = '\0';
        new_name = strdup(obj->name);
        DESTROY(obj->name);
        obj->name = new_name;
    }
}

void name_to_drinkcon(struct obj_data *obj, int type)
{
    char *new_name = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_ONAME(obj), type);

    CREATE(new_name, char, strlen(obj->name) + strlen(drinknames[type]) + 2);

    snprintf(new_name, strlen(obj->name) + strlen(drinknames[type]) + 2, "%s %s", obj->name, drinknames[type]);
    DESTROY(obj->name);
    obj->name = new_name;
}

int do_drink(struct char_data *ch, const char *argument, int cmd)
{
    struct obj_data *temp = NULL;
    char buf[255] = "\0\0\0\0\0\0\0";
    int amount = 0;
    struct affected_type af;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    only_argument(argument, buf);

    if (!(temp = get_obj_in_list_vis(ch, buf, ch->carrying)))
    {
        act("You can't find it!", FALSE, ch, 0, 0, TO_CHAR);
        return TRUE;
    }
    if (temp->obj_flags.type_flag != ITEM_DRINKCON)
    {
        act("You can't drink from that!", FALSE, ch, 0, 0, TO_CHAR);
        return TRUE;
    }
    if ((GET_COND(ch, DRUNK) > 15) && (GET_COND(ch, THIRST) > 0))
    {
        /* The pig is drunk */
        act("You're just sloshed.", FALSE, ch, 0, 0, TO_CHAR);
        act("$n looks really drunk.", TRUE, ch, 0, 0, TO_ROOM);
        return TRUE;
    }
    if ((GET_COND(ch, FULL) > 20) && (GET_COND(ch, THIRST) > 0))
    { /* Stomach full */
        act("Your stomach can't contain anymore!", FALSE, ch, 0, 0, TO_CHAR);
        return TRUE;
    }
    if (temp->obj_flags.type_flag == ITEM_DRINKCON)
    {
        if (temp->obj_flags.value[1] > 0)
        { /* Not empty */
            act("$n drinks %s from $p", TRUE, ch, temp, 0, TO_ROOM, drinks[temp->obj_flags.value[2]]);
            cprintf(ch, "You drink the %s.\r\n", drinks[temp->obj_flags.value[2]]);

            if (drink_aff[temp->obj_flags.value[2]][DRUNK] > 0)
                amount = (25 - GET_COND(ch, THIRST)) / drink_aff[temp->obj_flags.value[2]][DRUNK];
            else
                amount = number(3, 10);

            amount = MIN(amount, temp->obj_flags.value[1]);
            /* Subtract amount */
            if (temp->obj_flags.value[0] > 20)
                weight_change_object(temp, -amount);

            gain_condition(ch, DRUNK, (int)((int)drink_aff[temp->obj_flags.value[2]][DRUNK] * amount) / 4);

            gain_condition(ch, FULL, (int)((int)drink_aff[temp->obj_flags.value[2]][FULL] * amount) / 4);

            gain_condition(ch, THIRST, (int)((int)drink_aff[temp->obj_flags.value[2]][THIRST] * amount) / 4);

            if (GET_COND(ch, DRUNK) > 10)
                act("You feel drunk.", FALSE, ch, 0, 0, TO_CHAR);

            if (GET_COND(ch, THIRST) > 20)
                act("You do not feel thirsty.", FALSE, ch, 0, 0, TO_CHAR);

            if (GET_COND(ch, FULL) > 20)
                act("You are full.", FALSE, ch, 0, 0, TO_CHAR);

            if (temp->obj_flags.value[3])
            { /* The shit was poisoned ! */
                act("Oops, it tasted rather strange ?!!?", FALSE, ch, 0, 0, TO_CHAR);
                act("$n chokes and utters some strange sounds.", TRUE, ch, 0, 0, TO_ROOM);
                af.type = SPELL_POISON;
                af.duration = amount * 3;
                af.modifier = 0;
                af.location = APPLY_NONE;
                af.bitvector = AFF_POISON;
                affect_join(ch, &af, FALSE, FALSE);
            }
            /*
             * empty the container, and no longer poison.
             */
            temp->obj_flags.value[1] -= amount;
            if (!temp->obj_flags.value[1])
            { /* The last bit */
                temp->obj_flags.value[2] = 0;
                temp->obj_flags.value[3] = 0;
                name_from_drinkcon(temp);
            }
            if (temp->obj_flags.value[1] < 1)
            { /* its empty */
                if (temp->obj_flags.value[0] < 20)
                {
                    extract_obj(temp); /* get rid of it */
                }
            }
            return TRUE;
        }
        act("It's empty already.", FALSE, ch, 0, 0, TO_CHAR);
        return TRUE;
    }
    return TRUE;
}

int do_puke(struct char_data *ch, const char *argument, int cmd)
{
    char buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    struct char_data *vict = NULL;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    one_argument(argument, buf);

    if (!*buf)
    {
        act("$n blows chunks all over the room!", FALSE, ch, 0, 0, TO_ROOM);
        act("You puke and spew filth all over the place.", FALSE, ch, 0, 0, TO_CHAR);
    }
    else if (!(vict = get_char_room_vis(ch, buf)))
    {
        cprintf(ch, "You can't puke on someone who isn't here.\r\n");
        return TRUE;
    }
    else
    {
        act("$n walks up to $N and pukes up sickly green ichor all over them!", FALSE, ch, 0, vict, TO_ROOM);
        act("You vomit green chunks all over $N!", FALSE, ch, 0, vict, TO_CHAR);
    }
    if (IS_MORTAL(ch))
    {
        gain_condition(ch, FULL, -3);
        if (GET_COND(ch, FULL) < 0)
        {
            GET_COND(ch, FULL) = 0;
        }
        damage(ch, ch, 1, TYPE_SUFFERING);
    }
    return TRUE;
}

int do_eat(struct char_data *ch, const char *argument, int cmd)
{
    char buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    int j = 0;
    int num = 0;
    struct obj_data *temp = NULL;
    struct affected_type af;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    one_argument(argument, buf);

    if (!(temp = get_obj_in_list_vis(ch, buf, ch->carrying)))
    {
        act("You can't find it!", FALSE, ch, 0, 0, TO_CHAR);
        return TRUE;
    }
    if ((temp->obj_flags.type_flag != ITEM_FOOD) && (GetMaxLevel(ch) < DEMIGOD))
    {
        act("Your stomach refuses to eat that!?!", FALSE, ch, 0, 0, TO_CHAR);
        return TRUE;
    }
    if (GET_COND(ch, FULL) > 20)
    { /* Stomach full */
        act("You are to full to eat more!", FALSE, ch, 0, 0, TO_CHAR);
        return TRUE;
    }
    act("$n eats $p", TRUE, ch, temp, 0, TO_ROOM);
    act("You eat the $o.", FALSE, ch, temp, 0, TO_CHAR);

    gain_condition(ch, FULL, temp->obj_flags.value[0]);

    if (GET_COND(ch, FULL) > 20)
        act("You are full.", FALSE, ch, 0, 0, TO_CHAR);

    for (j = 0; j < MAX_OBJ_AFFECT; j++)
        if (temp->affected[j].location == APPLY_EAT_SPELL)
        {
            num = temp->affected[j].modifier;

            /* hit 'em with the spell */

            ((*spell_info[num].spell_pointer)(6, ch, "", SPELL_TYPE_POTION, ch, 0));
        }
    if (temp->obj_flags.value[3] && (GetMaxLevel(ch) < LOW_IMMORTAL))
    {
        act("That tasted rather strange !!", FALSE, ch, 0, 0, TO_CHAR);
        act("$n coughs and utters some strange sounds.", FALSE, ch, 0, 0, TO_ROOM);

        af.type = SPELL_POISON;
        af.duration = temp->obj_flags.value[0] * 2;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_POISON;
        affect_join(ch, &af, FALSE, FALSE);
    }
    extract_obj(temp);
    return TRUE;
}

int do_pour(struct char_data *ch, const char *argument, int cmd)
{
    char arg1[132] = "\0\0\0\0\0\0\0";
    char arg2[132] = "\0\0\0\0\0\0\0";
    struct obj_data *from_obj = NULL;
    struct obj_data *to_obj = NULL;
    int temp = 0;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    argument_interpreter(argument, arg1, arg2);

    if (!*arg1)
    { /* No arguments */
        act("What do you want to pour from?", FALSE, ch, 0, 0, TO_CHAR);
        return TRUE;
    }
    if (!(from_obj = get_obj_in_list_vis(ch, arg1, ch->carrying)))
    {
        act("You can't find it!", FALSE, ch, 0, 0, TO_CHAR);
        return TRUE;
    }
    if (from_obj->obj_flags.type_flag != ITEM_DRINKCON)
    {
        act("You can't pour from that!", FALSE, ch, 0, 0, TO_CHAR);
        return TRUE;
    }
    if (from_obj->obj_flags.value[1] == 0)
    {
        act("The $p is empty.", FALSE, ch, from_obj, 0, TO_CHAR);
        return TRUE;
    }
    if (!*arg2)
    {
        act("Where do you want it? Out or in what?", FALSE, ch, 0, 0, TO_CHAR);
        return TRUE;
    }
    if (!str_cmp(arg2, "out"))
    {
        act("$n empties $p", TRUE, ch, from_obj, 0, TO_ROOM);
        act("You empty the $p.", FALSE, ch, from_obj, 0, TO_CHAR);

        weight_change_object(from_obj, -from_obj->obj_flags.value[1]);

        from_obj->obj_flags.value[1] = 0;
        from_obj->obj_flags.value[2] = 0;
        from_obj->obj_flags.value[3] = 0;
        name_from_drinkcon(from_obj);

        return TRUE;
    }
    if (!(to_obj = get_obj_in_list_vis(ch, arg2, ch->carrying)))
    {
        act("You can't find it!", FALSE, ch, 0, 0, TO_CHAR);
        return TRUE;
    }
    if (to_obj->obj_flags.type_flag != ITEM_DRINKCON)
    {
        act("You can't pour anything into that.", FALSE, ch, 0, 0, TO_CHAR);
        return TRUE;
    }
    if ((to_obj->obj_flags.value[1] != 0) && (to_obj->obj_flags.value[2] != from_obj->obj_flags.value[2]))
    {
        act("There is already another liquid in it!", FALSE, ch, 0, 0, TO_CHAR);
        return TRUE;
    }
    if (!(to_obj->obj_flags.value[1] < to_obj->obj_flags.value[0]))
    {
        act("There is no room for more.", FALSE, ch, 0, 0, TO_CHAR);
        return TRUE;
    }
    if (to_obj == from_obj)
    {
        act("You can't pour to-from the same container?", FALSE, ch, 0, 0, TO_CHAR);
        return TRUE;
    }
    cprintf(ch, "You pour the %s into the %s.", drinks[from_obj->obj_flags.value[2]], arg2);

    /*
     * New alias
     */
    if (to_obj->obj_flags.value[1] == 0)
        name_to_drinkcon(to_obj, from_obj->obj_flags.value[2]);

    /*
     * First same type liq.
     */
    to_obj->obj_flags.value[2] = from_obj->obj_flags.value[2];

    /*
     * the new, improved way of doing this...
     */
    temp = from_obj->obj_flags.value[1];
    from_obj->obj_flags.value[1] = 0;
    to_obj->obj_flags.value[1] += temp;
    temp = to_obj->obj_flags.value[1] - to_obj->obj_flags.value[0];

    if (temp > 0)
    {
        from_obj->obj_flags.value[1] = temp;
    }
    else
    {
        name_from_drinkcon(from_obj);
    }

    if (from_obj->obj_flags.value[1] > from_obj->obj_flags.value[0])
        from_obj->obj_flags.value[1] = from_obj->obj_flags.value[0];

    /*
     * Then the poison boogie
     */
    to_obj->obj_flags.value[3] = (to_obj->obj_flags.value[3] || from_obj->obj_flags.value[3]);

    return TRUE;
}

int do_sip(struct char_data *ch, const char *argument, int cmd)
{
    char arg[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    struct obj_data *temp = NULL;
    struct affected_type af;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    one_argument(argument, arg);

    if (!(temp = get_obj_in_list_vis(ch, arg, ch->carrying)))
    {
        act("You can't find it!", FALSE, ch, 0, 0, TO_CHAR);
        return TRUE;
    }
    if (temp->obj_flags.type_flag != ITEM_DRINKCON)
    {
        act("You can't sip from that!", FALSE, ch, 0, 0, TO_CHAR);
        return TRUE;
    }
    if (GET_COND(ch, DRUNK) > 10)
    { /* The pig is drunk ! */
        act("You simply fail to reach your mouth!", FALSE, ch, 0, 0, TO_CHAR);
        act("$n tries to sip, but fails!", TRUE, ch, 0, 0, TO_ROOM);
        return TRUE;
    }
    if (!temp->obj_flags.value[1])
    { /* Empty */
        act("But there is nothing in it?", FALSE, ch, 0, 0, TO_CHAR);
        return TRUE;
    }
    act("$n sips from the $o", TRUE, ch, temp, 0, TO_ROOM);
    cprintf(ch, "It tastes like %s.\r\n", drinks[temp->obj_flags.value[2]]);

    gain_condition(ch, DRUNK, (int)(drink_aff[temp->obj_flags.value[2]][DRUNK] / 4));

    gain_condition(ch, FULL, (int)(drink_aff[temp->obj_flags.value[2]][FULL] / 4));

    gain_condition(ch, THIRST, (int)(drink_aff[temp->obj_flags.value[2]][THIRST] / 4));

    weight_change_object(temp, -1); /* Subtract one unit */

    if (GET_COND(ch, DRUNK) > 10)
        act("You feel drunk.", FALSE, ch, 0, 0, TO_CHAR);

    if (GET_COND(ch, THIRST) > 20)
        act("You do not feel thirsty.", FALSE, ch, 0, 0, TO_CHAR);

    if (GET_COND(ch, FULL) > 20)
        act("You are full.", FALSE, ch, 0, 0, TO_CHAR);

    if (temp->obj_flags.value[3] && !IS_AFFECTED(ch, AFF_POISON))
    { /* The shit was poisoned ! */
        act("But it also had a strange taste!", FALSE, ch, 0, 0, TO_CHAR);

        af.type = SPELL_POISON;
        af.duration = 3;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_POISON;
        affect_to_char(ch, &af);
    }
    temp->obj_flags.value[1]--;

    if (!temp->obj_flags.value[1])
    { /* The last bit */
        temp->obj_flags.value[2] = 0;
        temp->obj_flags.value[3] = 0;
        name_from_drinkcon(temp);
    }
    return TRUE;
}

int do_taste(struct char_data *ch, const char *argument, int cmd)
{
    struct affected_type af;
    char arg[80] = "\0\0\0\0\0\0\0";
    struct obj_data *temp = NULL;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    one_argument(argument, arg);

    if (!(temp = get_obj_in_list_vis(ch, arg, ch->carrying)))
    {
        act("You can't find it!", FALSE, ch, 0, 0, TO_CHAR);
        return TRUE;
    }
    if (temp->obj_flags.type_flag == ITEM_DRINKCON)
    {
        do_sip(ch, argument, 0);
        return TRUE;
    }
    if (!(temp->obj_flags.type_flag == ITEM_FOOD))
    {
        act("Taste that?!? Your stomach refuses!", FALSE, ch, 0, 0, TO_CHAR);
        return TRUE;
    }
    act("$n tastes the $o", FALSE, ch, temp, 0, TO_ROOM);
    act("You taste the $o", FALSE, ch, temp, 0, TO_CHAR);

    gain_condition(ch, FULL, 1);

    if (GET_COND(ch, FULL) > 20)
        act("You are full.", FALSE, ch, 0, 0, TO_CHAR);

    if (temp->obj_flags.value[3] && !IS_AFFECTED(ch, AFF_POISON))
    { /* The shit was poisoned ! */
        act("Ooups, it did not taste good at all!", FALSE, ch, 0, 0, TO_CHAR);

        af.type = SPELL_POISON;
        af.duration = 2;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_POISON;
        affect_to_char(ch, &af);
    }
    temp->obj_flags.value[0]--;

    if (!temp->obj_flags.value[0])
    { /* Nothing left */
        act("There is nothing left now.", FALSE, ch, 0, 0, TO_CHAR);
        extract_obj(temp);
    }
    return TRUE;
}

/* functions related to wear */

void perform_wear(struct char_data *ch, struct obj_data *obj_object, int keyword)
{
    if (DEBUG > 2)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), SAFE_ONAME(obj_object), keyword);

    switch (keyword)
    {
    case 0:
        act("$n lights $p and holds it.", FALSE, ch, obj_object, 0, TO_ROOM);
        break;
    case 1:
        act("$n wears $p on $s finger.", TRUE, ch, obj_object, 0, TO_ROOM);
        break;
    case 2:
        act("$n wears $p around $s neck.", TRUE, ch, obj_object, 0, TO_ROOM);
        break;
    case 3:
        act("$n wears $p on $s body.", TRUE, ch, obj_object, 0, TO_ROOM);
        break;
    case 4:
        act("$n wears $p on $s head.", TRUE, ch, obj_object, 0, TO_ROOM);
        break;
    case 5:
        act("$n wears $p on $s legs.", TRUE, ch, obj_object, 0, TO_ROOM);
        break;
    case 6:
        act("$n wears $p on $s feet.", TRUE, ch, obj_object, 0, TO_ROOM);
        break;
    case 7:
        act("$n wears $p on $s hands.", TRUE, ch, obj_object, 0, TO_ROOM);
        break;
    case 8:
        act("$n wears $p on $s arms.", TRUE, ch, obj_object, 0, TO_ROOM);
        break;
    case 9:
        act("$n wears $p about $s body.", TRUE, ch, obj_object, 0, TO_ROOM);
        break;
    case 10:
        act("$n wears $p about $s waist.", TRUE, ch, obj_object, 0, TO_ROOM);
        break;
    case 11:
        act("$n wears $p around $s wrist.", TRUE, ch, obj_object, 0, TO_ROOM);
        break;
    case 12:
        act("$n wields $p.", TRUE, ch, obj_object, 0, TO_ROOM);
        break;
    case 13:
        act("$n grabs $p.", TRUE, ch, obj_object, 0, TO_ROOM);
        break;
    case 14:
        act("$n starts using $p as shield.", TRUE, ch, obj_object, 0, TO_ROOM);
        break;
    case 15:
        act("$n wields $p two handed.", TRUE, ch, obj_object, 0, TO_ROOM);
        break;
    }
}

int IsRestricted(int Mask, int Class)
{
    int i = 0;

    if (DEBUG > 2)
        log_info("called %s with %d, %d", __PRETTY_FUNCTION__, Mask, Class);

    for (i = CLASS_MAGIC_USER; i <= CLASS_DRUID; i *= 2)
    {
        if (IS_SET(Mask, i) && (IS_NOT_SET(i, Class)))
        {
            Mask -= i;
        }
    }

    if (Mask == Class)
        return TRUE;

    return FALSE;
}

void wear(struct char_data *ch, struct obj_data *obj_object, int keyword)
{
    int BitMask = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), SAFE_ONAME(obj_object), keyword);

    if (!IS_IMMORTAL(ch))
    {
        BitMask = GetItemClassRestrictions(obj_object);
        if (IsRestricted(BitMask, ch->player.chclass) && (IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)))
        {
            cprintf(ch, "You are forbidden to do that.\r\n");
            return;
        }
    }
    if (!IsHumanoid(ch))
    {
        if ((keyword != 13) || (!HasHands(ch)))
        {
            cprintf(ch, "You can't wear things!\r\n");
            return;
        }
    }
    switch (keyword)
    {
    case 0: { /* LIGHT SOURCE */
        if (ch->equipment[WEAR_LIGHT])
            cprintf(ch, "You are already holding a light source.\r\n");
        else
        {
            cprintf(ch, "Ok.\r\n");
            perform_wear(ch, obj_object, keyword);
            obj_from_char(obj_object);
            equip_char(ch, obj_object, WEAR_LIGHT);
            if (obj_object->obj_flags.value[2])
                real_roomp(ch->in_room)->light++;
        }
    }
    break;

    case 1: {
        if (CAN_WEAR(obj_object, ITEM_WEAR_FINGER))
        {
            if ((ch->equipment[WEAR_FINGER_L]) && (ch->equipment[WEAR_FINGER_R]))
            {
                cprintf(ch, "You are already wearing something on your fingers.\r\n");
            }
            else
            {
                perform_wear(ch, obj_object, keyword);
                if (ch->equipment[WEAR_FINGER_L])
                {
                    cprintf(ch, "You put %s on your right finger.\r\n", obj_object->short_description);
                    obj_from_char(obj_object);
                    equip_char(ch, obj_object, WEAR_FINGER_R);
                }
                else
                {
                    cprintf(ch, "You put %s on your left finger.\r\n", obj_object->short_description);
                    obj_from_char(obj_object);
                    equip_char(ch, obj_object, WEAR_FINGER_L);
                }
            }
        }
        else
        {
            cprintf(ch, "You can't wear that on your finger.\r\n");
        }
    }
    break;
    case 2: {
        if (CAN_WEAR(obj_object, ITEM_WEAR_NECK))
        {
            if ((ch->equipment[WEAR_NECK_1]) && (ch->equipment[WEAR_NECK_2]))
            {
                cprintf(ch, "You can't wear any more around your neck.\r\n");
            }
            else
            {
                cprintf(ch, "OK.\r\n");
                perform_wear(ch, obj_object, keyword);
                if (ch->equipment[WEAR_NECK_1])
                {
                    obj_from_char(obj_object);
                    equip_char(ch, obj_object, WEAR_NECK_2);
                }
                else
                {
                    obj_from_char(obj_object);
                    equip_char(ch, obj_object, WEAR_NECK_1);
                }
            }
        }
        else
        {
            cprintf(ch, "You can't wear that around your neck.\r\n");
        }
    }
    break;
    case 3: {
        if (CAN_WEAR(obj_object, ITEM_WEAR_BODY))
        {
            if (ch->equipment[WEAR_BODY])
            {
                cprintf(ch, "You already wear something on your body.\r\n");
            }
            else
            {
                cprintf(ch, "OK.\r\n");
                perform_wear(ch, obj_object, keyword);
                obj_from_char(obj_object);
                equip_char(ch, obj_object, WEAR_BODY);
            }
        }
        else
        {
            cprintf(ch, "You can't wear that on your body.\r\n");
        }
    }
    break;
    case 4: {
        if (CAN_WEAR(obj_object, ITEM_WEAR_HEAD))
        {
            if (ch->equipment[WEAR_HEAD])
            {
                cprintf(ch, "You already wear something on your head.\r\n");
            }
            else
            {
                cprintf(ch, "OK.\r\n");
                perform_wear(ch, obj_object, keyword);
                obj_from_char(obj_object);
                equip_char(ch, obj_object, WEAR_HEAD);
            }
        }
        else
        {
            cprintf(ch, "You can't wear that on your head.\r\n");
        }
    }
    break;
    case 5: {
        if (CAN_WEAR(obj_object, ITEM_WEAR_LEGS))
        {
            if (ch->equipment[WEAR_LEGS])
            {
                cprintf(ch, "You already wear something on your legs.\r\n");
            }
            else
            {
                cprintf(ch, "OK.\r\n");
                perform_wear(ch, obj_object, keyword);
                obj_from_char(obj_object);
                equip_char(ch, obj_object, WEAR_LEGS);
            }
        }
        else
        {
            cprintf(ch, "You can't wear that on your legs.\r\n");
        }
    }
    break;
    case 6: {
        if (CAN_WEAR(obj_object, ITEM_WEAR_FEET))
        {
            if (ch->equipment[WEAR_FEET])
            {
                cprintf(ch, "You already wear something on your feet.\r\n");
            }
            else
            {
                cprintf(ch, "OK.\r\n");
                perform_wear(ch, obj_object, keyword);
                obj_from_char(obj_object);
                equip_char(ch, obj_object, WEAR_FEET);
            }
        }
        else
        {
            cprintf(ch, "You can't wear that on your feet.\r\n");
        }
    }
    break;
    case 7: {
        if (CAN_WEAR(obj_object, ITEM_WEAR_HANDS))
        {
            if (ch->equipment[WEAR_HANDS])
            {
                cprintf(ch, "You already wear something on your hands.\r\n");
            }
            else
            {
                cprintf(ch, "OK.\r\n");
                perform_wear(ch, obj_object, keyword);
                obj_from_char(obj_object);
                equip_char(ch, obj_object, WEAR_HANDS);
            }
        }
        else
        {
            cprintf(ch, "You can't wear that on your hands.\r\n");
        }
    }
    break;
    case 8: {
        if (CAN_WEAR(obj_object, ITEM_WEAR_ARMS))
        {
            if (ch->equipment[WEAR_ARMS])
            {
                cprintf(ch, "You already wear something on your arms.\r\n");
            }
            else
            {
                cprintf(ch, "OK.\r\n");
                perform_wear(ch, obj_object, keyword);
                obj_from_char(obj_object);
                equip_char(ch, obj_object, WEAR_ARMS);
            }
        }
        else
        {
            cprintf(ch, "You can't wear that on your arms.\r\n");
        }
    }
    break;
    case 9: {
        if (CAN_WEAR(obj_object, ITEM_WEAR_ABOUT))
        {
            if (ch->equipment[WEAR_ABOUT])
            {
                cprintf(ch, "You already wear something about your body.\r\n");
            }
            else
            {
                cprintf(ch, "OK.\r\n");
                perform_wear(ch, obj_object, keyword);
                obj_from_char(obj_object);
                equip_char(ch, obj_object, WEAR_ABOUT);
            }
        }
        else
        {
            cprintf(ch, "You can't wear that about your body.\r\n");
        }
    }
    break;
    case 10: {
        if (CAN_WEAR(obj_object, ITEM_WEAR_WAISTE))
        {
            if (ch->equipment[WEAR_WAISTE])
            {
                cprintf(ch, "You already wear something about your waist.\r\n");
            }
            else
            {
                cprintf(ch, "OK.\r\n");
                perform_wear(ch, obj_object, keyword);
                obj_from_char(obj_object);
                equip_char(ch, obj_object, WEAR_WAISTE);
            }
        }
        else
        {
            cprintf(ch, "You can't wear that about your waist.\r\n");
        }
    }
    break;
    case 11: {
        if (CAN_WEAR(obj_object, ITEM_WEAR_WRIST))
        {
            if ((ch->equipment[WEAR_WRIST_L]) && (ch->equipment[WEAR_WRIST_R]))
            {
                cprintf(ch, "You already wear something around both your wrists.\r\n");
            }
            else
            {
                perform_wear(ch, obj_object, keyword);
                obj_from_char(obj_object);
                if (ch->equipment[WEAR_WRIST_L])
                {
                    cprintf(ch, "You wear the %s around your right wrist.\r\n", obj_object->short_description);
                    equip_char(ch, obj_object, WEAR_WRIST_R);
                }
                else
                {
                    cprintf(ch, "You wear the %s around your left wrist.\r\n", obj_object->short_description);
                    equip_char(ch, obj_object, WEAR_WRIST_L);
                }
            }
        }
        else
        {
            cprintf(ch, "You can't wear that around your wrist.\r\n");
        }
    }
    break;

    case 12:
        if (CAN_WEAR(obj_object, ITEM_WIELD))
        {
            if (ch->equipment[WIELD] || ch->equipment[WIELD_TWOH])
            {
                cprintf(ch, "You are already wielding something.\r\n");
            }
            else
            {
                if (GET_OBJ_WEIGHT(obj_object) > str_app[STRENGTH_APPLY_INDEX(ch)].wield_w)
                {
                    cprintf(ch, "It is too heavy for you to use.\r\n");
                }
                else
                {
                    cprintf(ch, "OK.\r\n");
                    perform_wear(ch, obj_object, keyword);
                    obj_from_char(obj_object);
                    equip_char(ch, obj_object, WIELD);
                }
            }
        }
        else
        {
            cprintf(ch, "You can't wield that.\r\n");
        }
        break;

    case 13:
        if (CAN_WEAR(obj_object, ITEM_HOLD))
        {
            if (ch->equipment[HOLD])
            {
                cprintf(ch, "You are already holding something.\r\n");
            }
            else if (ch->equipment[WIELD_TWOH])
            {
                cprintf(ch, "You are wielding a two handed blade, you can't hold things!\r\n");
            }
            else if (ch->equipment[WEAR_SHIELD])
            {
                cprintf(ch, "Your hands are full already, you loser.\r\n");
            }
            else
            {
                cprintf(ch, "OK.\r\n");
                perform_wear(ch, obj_object, keyword);
                obj_from_char(obj_object);
                equip_char(ch, obj_object, HOLD);
            }
        }
        else
        {
            cprintf(ch, "You can't hold this.\r\n");
        }
        break;
    case 14: {
        if (CAN_WEAR(obj_object, ITEM_WEAR_SHIELD))
        {
            if ((ch->equipment[WEAR_SHIELD]))
            {
                cprintf(ch, "You are already using a shield\r\n");
            }
            else if (ch->equipment[WIELD_TWOH])
            {
                cprintf(ch, "You can not use a shield while wielding a two handed weapon!\r\n");
            }
            else if (ch->equipment[HOLD])
            {
                cprintf(ch, "Your hands are full already, you are holding something.\r\n");
            }
            else
            {
                perform_wear(ch, obj_object, keyword);
                cprintf(ch, "You start using the %s.\r\n", obj_object->short_description);
                obj_from_char(obj_object);
                equip_char(ch, obj_object, WEAR_SHIELD);
            }
        }
        else
        {
            cprintf(ch, "You can't use that as a shield.\r\n");
        }
    }
    break;

    case 15:
        if (CAN_WEAR(obj_object, ITEM_WIELD_TWOH))
        {
            if ((ch->equipment[WIELD]) || (ch->equipment[WIELD_TWOH]))
            {
                cprintf(ch, "You are already wielding something.\r\n");
            }
            else if (ch->equipment[WEAR_SHIELD])
            {
                cprintf(ch, "You can not wield two handed weapons and use a shield!\r\n");
            }
            else if (ch->equipment[HOLD])
            {
                cprintf(ch, "But you are holding something!\r\n");
            }
            else
            {
                if (GET_OBJ_WEIGHT(obj_object) > str_app[STRENGTH_APPLY_INDEX(ch)].wield_w)
                {
                    cprintf(ch, "It is too heavy for you to use.\r\n");
                }
                else
                {
                    cprintf(ch, "OK.\r\n");
                    perform_wear(ch, obj_object, keyword);
                    obj_from_char(obj_object);
                    equip_char(ch, obj_object, WIELD_TWOH);
                }
            }
        }
        else
        {
            cprintf(ch, "You can't wield that two handed.\r\n");
        }
        break;

    case -1: {
        cprintf(ch, "Wear %s where?.\r\n", obj_object->short_description);
    }
    break;
    case -2: {
        cprintf(ch, "You can't wear %s.\r\n", obj_object->short_description);
    }
    break;
    default: {
        log_error("Unknown type called in wear.");
    }
    break;
    }
}

int do_wear(struct char_data *ch, const char *argument, int cmd)
{
    char arg1[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char arg2[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    struct obj_data *obj_object = NULL;
    struct obj_data *next_obj = NULL;
    int keyword = 0;

    static const char *keywords[] = {"finger", "neck",  "body",  "head",  "legs",   "feet", "hands",
                                     "arms",   "about", "waist", "wrist", "shield", "\n"};

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    argument_interpreter(argument, arg1, arg2);
    if (*arg1)
    {
        if (!strcmp(arg1, "all"))
        {
            for (obj_object = ch->carrying; obj_object; obj_object = next_obj)
            {
                next_obj = obj_object->next_content;
                keyword = -2;
                if (CAN_WEAR(obj_object, ITEM_WEAR_FINGER))
                    keyword = 1;
                if (CAN_WEAR(obj_object, ITEM_WEAR_NECK))
                    keyword = 2;
                if (CAN_WEAR(obj_object, ITEM_WEAR_WRIST))
                    keyword = 11;
                if (CAN_WEAR(obj_object, ITEM_WEAR_WAISTE))
                    keyword = 10;
                if (CAN_WEAR(obj_object, ITEM_WEAR_ARMS))
                    keyword = 8;
                if (CAN_WEAR(obj_object, ITEM_WEAR_HANDS))
                    keyword = 7;
                if (CAN_WEAR(obj_object, ITEM_WEAR_FEET))
                    keyword = 6;
                if (CAN_WEAR(obj_object, ITEM_WEAR_LEGS))
                    keyword = 5;
                if (CAN_WEAR(obj_object, ITEM_WEAR_ABOUT))
                    keyword = 9;
                if (CAN_WEAR(obj_object, ITEM_WEAR_HEAD))
                    keyword = 4;
                if (CAN_WEAR(obj_object, ITEM_WEAR_BODY))
                    keyword = 3;
                if (CAN_WEAR(obj_object, ITEM_WIELD_TWOH))
                    keyword = 15;
                if (CAN_WEAR(obj_object, ITEM_WIELD))
                    keyword = 12;
                if (CAN_WEAR(obj_object, ITEM_WEAR_SHIELD))
                    keyword = 14;
                if (CAN_WEAR(obj_object, ITEM_HOLD))
                    keyword = 13;
                if (keyword != -2)
                {
                    cprintf(ch, "%s :", obj_object->short_description);
                    wear(ch, obj_object, keyword);
                }
            }
        }
        else
        {
            obj_object = get_obj_in_list_vis(ch, arg1, ch->carrying);
            if (obj_object)
            {
                if (*arg2)
                {
                    keyword = search_block(arg2, keywords, FALSE); /* Partial Match */
                    if (keyword == -1)
                    {
                        cprintf(ch, "%s is an unknown body location.\r\n", arg2);
                    }
                    else
                    {
                        wear(ch, obj_object, keyword + 1);
                    }
                }
                else
                {
                    keyword = -2;
                    if (CAN_WEAR(obj_object, ITEM_WEAR_SHIELD))
                        keyword = 14;
                    if (CAN_WEAR(obj_object, ITEM_WEAR_FINGER))
                        keyword = 1;
                    if (CAN_WEAR(obj_object, ITEM_WEAR_NECK))
                        keyword = 2;
                    if (CAN_WEAR(obj_object, ITEM_WEAR_WRIST))
                        keyword = 11;
                    if (CAN_WEAR(obj_object, ITEM_WEAR_WAISTE))
                        keyword = 10;
                    if (CAN_WEAR(obj_object, ITEM_WEAR_ARMS))
                        keyword = 8;
                    if (CAN_WEAR(obj_object, ITEM_WEAR_HANDS))
                        keyword = 7;
                    if (CAN_WEAR(obj_object, ITEM_WEAR_FEET))
                        keyword = 6;
                    if (CAN_WEAR(obj_object, ITEM_WEAR_LEGS))
                        keyword = 5;
                    if (CAN_WEAR(obj_object, ITEM_WEAR_ABOUT))
                        keyword = 9;
                    if (CAN_WEAR(obj_object, ITEM_WEAR_HEAD))
                        keyword = 4;
                    if (CAN_WEAR(obj_object, ITEM_WEAR_BODY))
                        keyword = 3;
                    wear(ch, obj_object, keyword);
                }
            }
            else
            {
                cprintf(ch, "You do not seem to have the '%s'.\r\n", arg1);
            }
        }
    }
    else
    {
        cprintf(ch, "Wear what?\r\n");
    }
    return TRUE;
}

int do_wield(struct char_data *ch, const char *argument, int cmd)
{
    char arg1[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    char arg2[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    struct obj_data *obj_object = NULL;
    int keyword = 12;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    argument_interpreter(argument, arg1, arg2);
    if (*arg1)
    {
        if (!strncmp("two", arg1, 3))
        {
            if (*arg2)
            {
                obj_object = get_obj_in_list_vis(ch, arg2, ch->carrying);
                if (obj_object)
                {
                    if (CAN_WEAR(obj_object, ITEM_WIELD_TWOH))
                    {
                        keyword = 15;
                        wear(ch, obj_object, keyword);
                    }
                    else
                    {
                        cprintf(ch, "That is not a two handed weapon!\r\n");
                    }
                }
                else
                {
                    cprintf(ch, "You do not seem to have the '%s'.\r\n", arg2);
                }
            }
            else
            { /* no arg2, check if they can wield it one handed, arg1 */
                obj_object = get_obj_in_list_vis(ch, arg1, ch->carrying);
                if (obj_object)
                {
                    keyword = 12;
                    wear(ch, obj_object, keyword);
                }
                else
                {
                    cprintf(ch, "You do not seem to have the '%s'.\r\n", arg1);
                }
            }
        }
        else
        {
            obj_object = get_obj_in_list_vis(ch, arg1, ch->carrying);
            if (obj_object)
            {
                keyword = 12;
                wear(ch, obj_object, keyword);
            }
            else
            {
                cprintf(ch, "You do not seem to have the '%s'.\r\n", arg1);
            }
        }
    }
    else
    {
        cprintf(ch, "Wield what?\r\n");
    }
    return TRUE;
}

int do_grab(struct char_data *ch, const char *argument, int cmd)
{
    char arg1[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char arg2[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    struct obj_data *obj_object = NULL;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    argument_interpreter(argument, arg1, arg2);

    if (*arg1)
    {
        obj_object = get_obj_in_list(arg1, ch->carrying);
        if (obj_object)
        {
            if (obj_object->obj_flags.type_flag == ITEM_LIGHT)
                wear(ch, obj_object, WEAR_LIGHT);
            else
                wear(ch, obj_object, 13);
        }
        else
        {
            cprintf(ch, "You do not seem to have the '%s'.\r\n", arg1);
        }
    }
    else
    {
        cprintf(ch, "Hold what?\r\n");
    }
    return TRUE;
}

int do_remove(struct char_data *ch, const char *argument, int cmd)
{
    char arg1[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char *T = NULL;
    char *P = NULL;
    int Rem_List[20];
    int Num_Equip = 0;
    struct obj_data *obj_object = NULL;
    int j = 0;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    one_argument(argument, arg1);

    if (*arg1)
    {
        if (!strcmp(arg1, "all"))
        {
            for (j = 0; j < MAX_WEAR; j++)
            {
                if (CAN_CARRY_N(ch) != IS_CARRYING_N(ch))
                {
                    if (ch->equipment[j])
                    {
                        if ((obj_object = unequip_char(ch, j)) != NULL)
                        {
                            obj_to_char(obj_object, ch);

                            if (obj_object->obj_flags.type_flag == ITEM_LIGHT)
                                if (obj_object->obj_flags.value[2])
                                    real_roomp(ch->in_room)->light--;

                            act("You stop using $p.", FALSE, ch, obj_object, 0, TO_CHAR);
                            act("$n stops using $p.", TRUE, ch, obj_object, 0, TO_ROOM);
                        }
                    }
                }
                else
                {
                    cprintf(ch, "You can't carry any more stuff.\r\n");
                    j = MAX_WEAR;
                }
            }
        }
        if (isdigit(arg1[0]))
        { /* Make a list of item numbers for stuff to remove */

            for (Num_Equip = j = 0; j < MAX_WEAR; j++)
            {
                if (CAN_CARRY_N(ch) != IS_CARRYING_N(ch))
                {
                    if (ch->equipment[j])
                        Rem_List[Num_Equip++] = j;
                }
            }

            T = arg1;

            while (isdigit(*T) && (*T != '\0'))
            {
                P = T;
                if (strchr(T, ','))
                {
                    P = strchr(T, ',');
                    *P = '\0';
                }
                if (atoi(T) > 0 && atoi(T) <= Num_Equip)
                {
                    if (CAN_CARRY_N(ch) != IS_CARRYING_N(ch))
                    {
                        j = Rem_List[atoi(T) - 1];
                        if (ch->equipment[j])
                        {
                            if ((obj_object = unequip_char(ch, j)) != NULL)
                            {
                                obj_to_char(obj_object, ch);
                                if (obj_object->obj_flags.type_flag == ITEM_LIGHT)
                                    if (obj_object->obj_flags.value[2])
                                        real_roomp(ch->in_room)->light--;

                                act("You stop using $p.", FALSE, ch, obj_object, 0, TO_CHAR);
                                act("$n stops using $p.", TRUE, ch, obj_object, 0, TO_ROOM);
                            }
                        }
                    }
                    else
                    {
                        cprintf(ch, "You can't carry any more stuff.\r\n");
                        j = MAX_WEAR;
                    }
                }
                else
                {
                    cprintf(ch, "You dont seem to have the %s\r\n", T);
                }

                if (T != P)
                    T = P + 1;
                else
                    *T = '\0';
            }
        }
        else
        {
            obj_object = get_object_in_equip_vis(ch, arg1, ch->equipment, &j);
            if (obj_object)
            {
                if (CAN_CARRY_N(ch) != IS_CARRYING_N(ch))
                {
                    obj_to_char(unequip_char(ch, j), ch);

                    if (obj_object->obj_flags.type_flag == ITEM_LIGHT)
                        if (obj_object->obj_flags.value[2])
                            real_roomp(ch->in_room)->light--;

                    act("You stop using $p.", FALSE, ch, obj_object, 0, TO_CHAR);
                    act("$n stops using $p.", TRUE, ch, obj_object, 0, TO_ROOM);
                }
                else
                {
                    cprintf(ch, "You can't carry that many items.\r\n");
                }
            }
            else
            {
                cprintf(ch, "You are not using it.\r\n");
            }
        }
    }
    else
    {
        cprintf(ch, "Remove what?\r\n");
    }
    return TRUE;
}

int do_bury(struct char_data *ch, const char *argument, int cmd)
{
    char buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    struct obj_data *vict = NULL;
    struct obj_data *obj = NULL;
    struct obj_data *next = NULL;
    int experience = 0;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    one_argument(argument, buf);

    if (!*buf)
    {
        act("$n imitates Stalin, chanting 'We will bury you!'", FALSE, ch, 0, 0, TO_ROOM);
        act("You do a nice imitation of Stalin.", FALSE, ch, 0, 0, TO_CHAR);
        return TRUE;
    }
    else if (!(vict = get_obj_vis_accessible(ch, buf)))
    {
        cprintf(ch, "You feel you should go TO the %s first.\r\n", buf);
        return TRUE;
    }
    else if (!IS_CORPSE(vict))
    {
        cprintf(ch, "You consider burying %s, but what if you can't find it again?\r\n", buf);
        return TRUE;
    }

    act("$n buries $p.", FALSE, ch, vict, ch, TO_ROOM);
    act("You bury $p.", FALSE, ch, vict, ch, TO_CHAR);
    for (obj = vict->contains; obj; obj = next)
    {
        next = obj->next_content;
        obj_from_obj(obj);
        extract_obj(obj);
    }
    extract_obj(vict);
    if (IS_MORTAL(ch))
    {
        experience = number(1, 50);
        cprintf(ch, "You gain %d experience for your charity!\r\n", experience);
        GET_EXP(ch) += experience;
        GET_ALIGNMENT(ch) += number(0, 5);
        if (GET_ALIGNMENT(ch) > 999)
            GET_ALIGNMENT(ch) = 1000;
        WAIT_STATE(ch, 4);
    }
    return TRUE;
}

int do_desecrate(struct char_data *ch, const char *argument, int cmd)
{
    char buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    struct obj_data *vict = NULL;
    struct obj_data *obj = NULL;
    struct obj_data *next = NULL;
    int experience = 0;

    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    one_argument(argument, buf);

    if (!*buf)
    {
        act("$n looks around for things to desecrate.", FALSE, ch, 0, 0, TO_ROOM);
        act("You can't seem to find anything worth desecrating.", FALSE, ch, 0, 0, TO_CHAR);
        return TRUE;
    }
    else if (!(vict = get_obj_vis_accessible(ch, buf)))
    {
        cprintf(ch, "You feel you should go TO the %s first.\r\n", buf);
        return TRUE;
    }
    else if (!IS_CORPSE(vict))
    {
        cprintf(ch, "You consider desecrating %s, but it just doesn't seem worth the bother.\r\n", buf);
        return TRUE;
    }

    act("$n stomps the bloody mush of $p into the ground and spits!", FALSE, ch, vict, ch, TO_ROOM);
    act("You stomp the bloody mush of $p into the dirt and spit!", FALSE, ch, vict, ch, TO_CHAR);
    for (obj = vict->contains; obj; obj = next)
    {
        next = obj->next_content;
        obj_from_obj(obj);
        extract_obj(obj);
    }
    extract_obj(vict);
    if (IS_MORTAL(ch))
    {
        experience = number(1, 50);
        cprintf(ch, "You gain %d experience for your depravity!\r\n", experience);
        GET_EXP(ch) += experience;
        GET_ALIGNMENT(ch) -= number(0, 5);
        if (GET_ALIGNMENT(ch) < -999)
            GET_ALIGNMENT(ch) = -1000;
        WAIT_STATE(ch, 4);
    }
    return TRUE;
}

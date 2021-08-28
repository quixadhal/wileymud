/*
 * file: shop.c , Shop module.                            Part of DIKUMUD
 * Usage: Procedures handling shops and shopkeepers.
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
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "interpreter.h"
#include "utils.h"
#include "act_comm.h"
#include "multiclass.h"
#include "act_social.h"
#include "act_wiz.h"
#include "spec_procs.h"
#include "weather.h" // time_info_data struct

#define _SHOP_C
#include "shop.h"

struct shop_data *shop_index = NULL;
int number_of_shops = 0;

int is_ok(struct char_data *keeper, struct char_data *ch, int shop_nr)
{
    if (DEBUG > 2)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(keeper), SAFE_NAME(ch), shop_nr);

    if (shop_index[shop_nr].open1 > time_info.hours)
    {
        do_say(keeper, "Come back later!", 17);
        return FALSE;
    }
    else if (shop_index[shop_nr].close1 < time_info.hours)
    {
        if (shop_index[shop_nr].open2 > time_info.hours)
        {
            do_say(keeper, "Sorry, we have closed, but come back later.", 17);
            return FALSE;
        }
        else if (shop_index[shop_nr].close2 < time_info.hours)
        {
            do_say(keeper, "Sorry, come back tomorrow.", 17);
            return FALSE;
        };
    }

    if (!(CAN_SEE(keeper, ch)))
    {
        do_say(keeper, "I don't trade with someone I can't see!", 17);
        return FALSE;
    };

    switch (shop_index[shop_nr].with_who)
    {
    case 0:
        return TRUE;
    case 1:
        return TRUE;
    default:
        return TRUE;
    };
}

int trade_with(struct obj_data *item, int shop_nr)
{
    int counter = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_ONAME(item), shop_nr);

    if (item->obj_flags.cost < 1)
        return FALSE;

    for (counter = 0; counter < MAX_TRADE; counter++)
        if (shop_index[shop_nr].type[counter] == item->obj_flags.type_flag)
            return TRUE;
    return FALSE;
}

int shop_producing(struct obj_data *item, int shop_nr)
{
    int counter = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_ONAME(item), shop_nr);

    if (item->item_number < 0)
        return FALSE;

    for (counter = 0; counter < MAX_PROD; counter++)
        if (shop_index[shop_nr].producing[counter] == item->item_number)
            return TRUE;
    return FALSE;
}

void shopping_buy(const char *arg, struct char_data *ch, struct char_data *keeper, int shop_nr)
{
    char argm[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char buf[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    char newarg[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    int num = 1;
    struct obj_data *temp1 = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %s, %s, %d", __PRETTY_FUNCTION__, VNULL(arg), SAFE_NAME(ch), SAFE_NAME(keeper),
                 shop_nr);

    if (!(is_ok(keeper, ch, shop_nr)))
        return;

    only_argument(arg, argm);
    if (!(*argm))
    {
        snprintf(buf, MAX_STRING_LENGTH, "%s what do you want to buy??", GET_NAME(ch));
        do_tell(keeper, buf, 19);
        return;
    };

    if ((num = getabunch(argm, newarg)) != FALSE)
    {
        strcpy(argm, newarg);
    }
    if (num == 0)
        num = 1;

    if (!(temp1 = get_obj_in_list_vis(ch, argm, keeper->carrying)))
    {
        snprintf(buf, MAX_STRING_LENGTH, shop_index[shop_nr].no_such_item1, GET_NAME(ch));
        do_tell(keeper, buf, 19);
        return;
    }
    if (temp1->obj_flags.cost <= 0)
    {
        snprintf(buf, MAX_STRING_LENGTH, shop_index[shop_nr].no_such_item1, GET_NAME(ch));
        do_tell(keeper, buf, 19);
        extract_obj(temp1);
        return;
    }
    if (GET_GOLD(ch) < (int)(num * (temp1->obj_flags.cost * shop_index[shop_nr].profit_buy)) &&
        GetMaxLevel(ch) < DEMIGOD)
    {
        snprintf(buf, MAX_STRING_LENGTH, shop_index[shop_nr].missing_cash2, GET_NAME(ch));
        do_tell(keeper, buf, 19);

        switch (shop_index[shop_nr].temper1)
        {
        case 0:
            do_action(keeper, GET_NAME(ch), 30);
            return;
        case 1:
            do_emote(keeper, "grins happily", 36);
            return;
        default:
            return;
        }
    }
    if ((IS_CARRYING_N(ch) + num) > (CAN_CARRY_N(ch)))
    {
        cprintf(ch, "%s : You can't carry that many items.\r\n", fname(temp1->name));
        return;
    }
    if ((IS_CARRYING_W(ch) + (num * temp1->obj_flags.weight)) > CAN_CARRY_W(ch))
    {
        cprintf(ch, "%s : You can't carry that much weight.\r\n", fname(temp1->name));
        return;
    }
    act("$n buys $p.", FALSE, ch, temp1, 0, TO_ROOM);

    snprintf(buf, MAX_STRING_LENGTH, shop_index[shop_nr].message_buy, GET_NAME(ch),
            (int)(num * (temp1->obj_flags.cost * shop_index[shop_nr].profit_buy)));

    do_tell(keeper, buf, 19);

    cprintf(ch, "You now have %s (*%d).\r\n", temp1->short_description, num);

    while (num-- > 0)
    {

        if (GetMaxLevel(ch) < DEMIGOD)
            GET_GOLD(ch) -= (int)(temp1->obj_flags.cost * shop_index[shop_nr].profit_buy);

        GET_GOLD(keeper) += (int)(temp1->obj_flags.cost * shop_index[shop_nr].profit_buy);

        /*
         * Test if producing shop !
         */
        if (shop_producing(temp1, shop_nr))
            temp1 = read_object(temp1->item_number, REAL);
        else
        {
            obj_from_char(temp1);
            if (temp1 == NULL)
            {
                cprintf(ch, "Sorry, I just ran out of those.\r\n");
                GET_GOLD(ch) += (int)(temp1->obj_flags.cost * shop_index[shop_nr].profit_buy);
                return;
            }
        }

        obj_to_char(temp1, ch);
    }
    return;
}

void shopping_sell(const char *arg, struct char_data *ch, struct char_data *keeper, int shop_nr)
{
    char argm[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char buf[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    int cost = 0;
    struct obj_data *temp1 = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %s, %s, %d", __PRETTY_FUNCTION__, VNULL(arg), SAFE_NAME(ch), SAFE_NAME(keeper),
                 shop_nr);

    if (!(is_ok(keeper, ch, shop_nr)))
        return;

    only_argument(arg, argm);

    if (!(*argm))
    {
        snprintf(buf, MAX_STRING_LENGTH, "%s What do you want to sell??", GET_NAME(ch));
        do_tell(keeper, buf, 19);
        return;
    }
    if (!(temp1 = get_obj_in_list_vis(ch, argm, ch->carrying)))
    {
        snprintf(buf, MAX_STRING_LENGTH, shop_index[shop_nr].no_such_item2, GET_NAME(ch));
        do_tell(keeper, buf, 19);
        return;
    }
    if (!(trade_with(temp1, shop_nr)) || (temp1->obj_flags.cost < 1))
    {
        snprintf(buf, MAX_STRING_LENGTH, shop_index[shop_nr].do_not_buy, GET_NAME(ch));
        do_tell(keeper, buf, 19);
        return;
    }
    if (GET_GOLD(keeper) < (int)(temp1->obj_flags.cost * shop_index[shop_nr].profit_sell))
    {
        snprintf(buf, MAX_STRING_LENGTH, shop_index[shop_nr].missing_cash1, GET_NAME(ch));
        do_tell(keeper, buf, 19);
        return;
    }
    cost = temp1->obj_flags.cost;

    if ((ITEM_TYPE(temp1) == ITEM_WAND) || (ITEM_TYPE(temp1) == ITEM_STAFF))
    {
        if (temp1->obj_flags.value[1])
        {
            cost = (int)cost * (float)(temp1->obj_flags.value[2] / (float)temp1->obj_flags.value[1]);
        }
        else
        {
            cost = 0;
        }
    }
    /*
     * else
     * if (ITEM_TYPE(temp1) == ITEM_ARMOR)
     * {
     * if(temp1->obj_flags.value[1])
     * {
     * cost = (int)cost *
     * ((float)temp1->obj_flags.value[0] / (float)temp1->obj_flags.value[1]);
     * }
     * else
     * {
     * cost = 0;
     * }
     * }
     */
    temp1->obj_flags.cost = cost;

    act("$n sells $p.", FALSE, ch, temp1, 0, TO_ROOM);

    snprintf(buf, MAX_STRING_LENGTH, shop_index[shop_nr].message_sell, GET_NAME(ch),
            (int)(temp1->obj_flags.cost * shop_index[shop_nr].profit_sell));

    do_tell(keeper, buf, 19);

    cprintf(ch, "The shopkeeper now has %s.\r\n", temp1->short_description);

    if (GET_GOLD(keeper) < (int)(temp1->obj_flags.cost * shop_index[shop_nr].profit_sell))
    {
        snprintf(buf, MAX_STRING_LENGTH, shop_index[shop_nr].missing_cash1, GET_NAME(ch));
        do_tell(keeper, buf, 19);
        return;
    }
    GET_GOLD(ch) += (int)(temp1->obj_flags.cost * shop_index[shop_nr].profit_sell);
    GET_GOLD(keeper) -= (int)(temp1->obj_flags.cost * shop_index[shop_nr].profit_sell);

    obj_from_char(temp1);
    if (temp1 == NULL)
    {
        cprintf(ch, "As far as I am concerned, you are out..\r\n");
        return;
    }
    if ((get_obj_in_list(argm, keeper->carrying)) || (GET_ITEM_TYPE(temp1) == ITEM_TRASH))
    {
        extract_obj(temp1);
    }
    else
    {
        obj_to_char(temp1, keeper);
    }
    return;
}

void shopping_value(const char *arg, struct char_data *ch, struct char_data *keeper, int shop_nr)
{
    char argm[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char buf[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    struct obj_data *temp1 = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %s, %s, %d", __PRETTY_FUNCTION__, VNULL(arg), SAFE_NAME(ch), SAFE_NAME(keeper),
                 shop_nr);

    if (!(is_ok(keeper, ch, shop_nr)))
        return;

    only_argument(arg, argm);

    if (!(*argm))
    {
        snprintf(buf, MAX_STRING_LENGTH, "%s What do you want me to valuate??", GET_NAME(ch));
        do_tell(keeper, buf, 19);
        return;
    }
    if (!(temp1 = get_obj_in_list_vis(ch, argm, ch->carrying)))
    {
        snprintf(buf, MAX_STRING_LENGTH, shop_index[shop_nr].no_such_item2, GET_NAME(ch));
        do_tell(keeper, buf, 19);
        return;
    }
    if (!(trade_with(temp1, shop_nr)))
    {
        snprintf(buf, MAX_STRING_LENGTH, shop_index[shop_nr].do_not_buy, GET_NAME(ch));
        do_tell(keeper, buf, 19);
        return;
    }
    snprintf(buf, MAX_STRING_LENGTH, "%s I'll give you %d gold coins for that!", GET_NAME(ch),
            (int)(temp1->obj_flags.cost * shop_index[shop_nr].profit_sell));
    do_tell(keeper, buf, 19);

    return;
}

void shopping_list(const char *arg, struct char_data *ch, struct char_data *keeper, int shop_nr)
{
    char buf[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    char buf2[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char buf3[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    struct obj_data *temp1 = NULL;
    int found_obj = FALSE;

    if (DEBUG > 2)
        log_info("called %s with %s, %s, %s, %d", __PRETTY_FUNCTION__, VNULL(arg), SAFE_NAME(ch), SAFE_NAME(keeper),
                 shop_nr);

    if (!(is_ok(keeper, ch, shop_nr)))
        return;

    strcpy(buf, "You can buy:\r\n");
    found_obj = FALSE;
    if (keeper->carrying)
        for (temp1 = keeper->carrying; temp1; temp1 = temp1->next_content)
            if ((CAN_SEE_OBJ(ch, temp1)) && (temp1->obj_flags.cost > 0))
            {
                found_obj = TRUE;
                if (temp1->obj_flags.type_flag != ITEM_DRINKCON)
                    snprintf(buf2, MAX_INPUT_LENGTH, "%s for %d gold coins.\r\n", (temp1->short_description),
                            (int)(temp1->obj_flags.cost * shop_index[shop_nr].profit_buy));
                else
                {
                    if (temp1->obj_flags.value[1])
                        snprintf(buf3, MAX_INPUT_LENGTH, "%s of %s", (temp1->short_description), drinks[temp1->obj_flags.value[2]]);
                    else
                        snprintf(buf3, MAX_INPUT_LENGTH, "%s", (temp1->short_description));
                    snprintf(buf2, MAX_INPUT_LENGTH, "%s for %d gold coins.\r\n", buf3,
                            (int)(temp1->obj_flags.cost * shop_index[shop_nr].profit_buy));
                }
                strcat(buf, CAP(buf2));
            };

    if (!found_obj)
        strcat(buf, "Nothing!\r\n");

    cprintf(ch, "%s", buf);
    return;
}

void shopping_kill(const char *arg, struct char_data *ch, struct char_data *keeper, int shop_nr)
{
    char buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG > 2)
        log_info("called %s with %s, %s, %s, %d", __PRETTY_FUNCTION__, VNULL(arg), SAFE_NAME(ch), SAFE_NAME(keeper),
                 shop_nr);

    switch (shop_index[shop_nr].temper2)
    {
    case 0:
        snprintf(buf, MAX_INPUT_LENGTH, "%s Don't ever try that again!", GET_NAME(ch));
        do_tell(keeper, buf, 19);
        return;

    case 1:
        snprintf(buf, MAX_INPUT_LENGTH, "%s Scram - midget!", GET_NAME(ch));
        do_tell(keeper, buf, 19);
        return;

    default:
        return;
    }
}

int shop_keeper(struct char_data *ch, int cmd, const char *arg)
{
    char argm[100] = "\0\0\0\0\0\0\0";
    struct char_data *temp_char = NULL;
    struct char_data *keeper = NULL;
    int shop_nr = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    for (temp_char = real_roomp(ch->in_room)->people; (!keeper) && (temp_char); temp_char = temp_char->next_in_room)
        if (IS_MOB(temp_char))
            if (mob_index[temp_char->nr].func == shop_keeper)
                keeper = temp_char;

    for (shop_nr = 0; shop_index[shop_nr].keeper != keeper->nr; shop_nr++)
        ;

    if (!cmd)
    {
        if (keeper->specials.fighting)
        {
            return citizen(keeper, 0, "");
        }
    }
    if ((cmd == 56) && (ch->in_room == shop_index[shop_nr].in_room))
    /*
     * Buy
     */
    {
        shopping_buy(arg, ch, keeper, shop_nr);
        return TRUE;
    }
    if ((cmd == 57) && (ch->in_room == shop_index[shop_nr].in_room))
    /*
     * Sell
     */
    {
        shopping_sell(arg, ch, keeper, shop_nr);
        return TRUE;
    }
    if ((cmd == 58) && (ch->in_room == shop_index[shop_nr].in_room))
    /*
     * value
     */
    {
        shopping_value(arg, ch, keeper, shop_nr);
        return TRUE;
    }
    if ((cmd == 59) && (ch->in_room == shop_index[shop_nr].in_room))
    /*
     * List
     */
    {
        shopping_list(arg, ch, keeper, shop_nr);
        return TRUE;
    }
    if ((cmd == 25) || (cmd == 70))
    { /* Kill or Hit */
        only_argument(arg, argm);

        if (keeper == get_char_room(argm, ch->in_room))
        {
            shopping_kill(arg, ch, keeper, shop_nr);
            return TRUE;
        }
    }
    else if ((cmd == 84) || (cmd == 207) || (cmd == 172))
    { /* Cast, recite, use */
        act("$N tells you 'No magic here - kid!'.", FALSE, ch, 0, keeper, TO_CHAR);
        return TRUE;
    }
    return FALSE;
}

void boot_the_shops(void)
{
    char *buf = NULL;
    int temp = 0;
    int count = 0;
    FILE *shop_f = NULL;

    if (DEBUG > 2)
        log_info("called %s with no arguments", __PRETTY_FUNCTION__);

    if (!(shop_f = fopen(SHOP_FILE, "r")))
    {
        log_fatal("Error in boot shop\n");
        proper_exit(MUD_HALT);
    }
    number_of_shops = 0;

    for (;;)
    {
        buf = fread_string(shop_f);
        if (*buf == '#')
        { /* a new shop */
            if (!number_of_shops)
            { /* first shop */
                CREATE(shop_index, struct shop_data, 1);
            }
            else
            {
                RECREATE(shop_index, struct shop_data, number_of_shops + 1);
            }

            for (count = 0; count < MAX_PROD; count++)
            {
                fscanf(shop_f, "%d \n", &temp);
                if (temp >= 0)
                    shop_index[number_of_shops].producing[count] = real_object(temp);
                else
                    shop_index[number_of_shops].producing[count] = temp;
            }
            fscanf(shop_f, "%f \n", &shop_index[number_of_shops].profit_buy);
            fscanf(shop_f, "%f \n", &shop_index[number_of_shops].profit_sell);
            for (count = 0; count < MAX_TRADE; count++)
            {
                fscanf(shop_f, "%d \n", &temp);
                shop_index[number_of_shops].type[count] = (char)temp;
            }
            shop_index[number_of_shops].no_such_item1 = fread_string(shop_f);
            shop_index[number_of_shops].no_such_item2 = fread_string(shop_f);
            shop_index[number_of_shops].do_not_buy = fread_string(shop_f);
            shop_index[number_of_shops].missing_cash1 = fread_string(shop_f);
            shop_index[number_of_shops].missing_cash2 = fread_string(shop_f);
            shop_index[number_of_shops].message_buy = fread_string(shop_f);
            shop_index[number_of_shops].message_sell = fread_string(shop_f);
            fscanf(shop_f, "%d \n", &shop_index[number_of_shops].temper1);
            fscanf(shop_f, "%d \n", &shop_index[number_of_shops].temper2);
            fscanf(shop_f, "%d \n", &shop_index[number_of_shops].keeper);

            shop_index[number_of_shops].keeper = real_mobile(shop_index[number_of_shops].keeper);

            fscanf(shop_f, "%d \n", &shop_index[number_of_shops].with_who);
            fscanf(shop_f, "%d \n", &shop_index[number_of_shops].in_room);
            fscanf(shop_f, "%d \n", &shop_index[number_of_shops].open1);
            fscanf(shop_f, "%d \n", &shop_index[number_of_shops].close1);
            fscanf(shop_f, "%d \n", &shop_index[number_of_shops].open2);
            fscanf(shop_f, "%d \n", &shop_index[number_of_shops].close2);

            number_of_shops++;
        }
        else if (*buf == '$') /* EOF */
            break;
    }

    FCLOSE(shop_f);
}

void assign_the_shopkeepers(void)
{
    int temp1 = 0;

    if (DEBUG > 2)
        log_info("called %s with no arguments", __PRETTY_FUNCTION__);

    for (temp1 = 0; temp1 < number_of_shops; temp1++)
        mob_index[shop_index[temp1].keeper].func = shop_keeper;
}

/*
 * file: reception.c, Special module for Inn's.           Part of DIKUMUD
 * Usage: Procedures handling saving/loading of player objects
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */

#include <stdio.h>
#include <stdlib.h>
/* #include <unistd.h> */
#include <sys/types.h>
#include <string.h>
#include <sys/time.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>

#include "global.h"
#include "bug.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "interpreter.h"
#include "utils.h"
#include "spells.h"
#include "multiclass.h"
#include "act_social.h"
#include "spec_procs.h"
#include "rent.h"    // for rent data
#include "weather.h" // for SECS_PER defines
#define _RECEPTION_C
#include "reception.h"

/*
 * Routines used for the "Offer"
 */

void add_obj_cost(struct char_data *ch, struct char_data *re, struct obj_data *obj, struct obj_cost *cost)
{
    int temp = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %s, %s, %08zx", __PRETTY_FUNCTION__, SAFE_NAME(ch), SAFE_NAME(re), SAFE_ONAME(obj),
                 (size_t)cost);

    /*
     * Add cost for an item and it's contents, and next->contents
     */

    if (obj)
    {
        if ((obj->item_number > -1) && (cost->ok))
        {
            temp = rent.enabled ? MAX(0, (int)(obj->obj_flags.cost_per_day * rent.factor)) : 0;
            cost->total_cost += temp;
            if (re)
            {
                cprintf(ch, "%30s : %d coins/day\r\n", obj->short_description, temp);
            }
            cost->no_carried++;
            add_obj_cost(ch, re, obj->contains, cost);
            add_obj_cost(ch, re, obj->next_content, cost);
        }
        else if (cost->ok)
        {
            if (re)
            {
                act("$n tells you 'I refuse storing $p'", FALSE, re, obj, ch, TO_VICT);
            }
            else
            {
                act("Sorry, but $p don't keep in storage.", FALSE, 0, obj, ch, TO_VICT);
            }
        }
    }
}

char recep_offer(struct char_data *ch, struct char_data *recep_mob, struct obj_cost *cost)
{
    int i = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %s, %08zx", __PRETTY_FUNCTION__, SAFE_NAME(ch), SAFE_NAME(recep_mob),
                 (size_t)cost);

    cost->total_cost = 0; /* Minimum cost */
    cost->no_carried = 0;
    cost->ok = TRUE; /* Use if any "-1" objects */
    add_obj_cost(ch, recep_mob, ch->carrying, cost);

    for (i = 0; i < MAX_WEAR; i++)
        add_obj_cost(ch, recep_mob, ch->equipment[i], cost);

    if (!cost->ok)
        return FALSE;

    if (cost->no_carried == 0)
    {
        if (recep_mob)
            act("$n tells you 'But you are not carrying anything?'", FALSE, recep_mob, 0, ch, TO_VICT);
        return FALSE;
    }
    if (cost->no_carried > MAX_OBJ_SAVE)
    {
        if (recep_mob)
            act("$n tells you 'Sorry, but I can't store any more than %d items.", FALSE, recep_mob, 0, ch, TO_VICT,
                MAX_OBJ_SAVE);
        return FALSE;
    }
    if (recep_mob)
    {
        act("$n tells you 'It will cost you %d coins per day'", FALSE, recep_mob, 0, ch, TO_VICT, cost->total_cost);
        if (cost->total_cost > (GET_GOLD(ch) + GET_BANK(ch)))
        {
            if (GetMaxLevel(ch) < LOW_IMMORTAL)
                act("$n tells you 'Which I can see you can't afford'", FALSE, recep_mob, 0, ch, TO_VICT);
            else
            {
                act("$n tells you 'Well, since you're a God, I guess it's okay'", FALSE, recep_mob, 0, ch, TO_VICT);
                cost->total_cost = 0;
            }
        }
    }
    if (cost->total_cost > (GET_GOLD(ch) + GET_BANK(ch)))
        return FALSE;
    else
        return TRUE;
}

/*
 * General save/load routines
 */

void update_file(FILE *fl, char *name, struct obj_file_u *st)
{
    struct rental_header rh;
    int nlength = 0;

    if (DEBUG > 2)
        log_info("called %s with %08zx, %s, %08zx", __PRETTY_FUNCTION__, (size_t)fl, VNULL(name), (size_t)st);

    if (st->nobjects == 0)
    {
        rh.inuse = 0;
        nlength = 0;
        rh.length = 0;
    }
    else
    {
        rh.inuse = 1;
        nlength = sizeof(*st) + (st->nobjects - MAX_OBJ_SAVE) * sizeof(*st->objects);
        rh.length = nlength;
    }

    strcpy(rh.owner, name);
    fwrite(&rh, sizeof(rh), 1, fl);
    fwrite(st, nlength, 1, fl);
    return;
}

/*
 * Routines used to load a characters equipment from disk
 */

void obj_store_to_char(struct char_data *ch, struct obj_file_u *st)
{
    struct obj_data *obj = NULL;
    int i = 0;
    int j = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %08zx", __PRETTY_FUNCTION__, SAFE_NAME(ch), (size_t)st);

    for (i = 0; i < st->nobjects; i++)
    {
        if (st->objects[i].item_number > -1 && real_object(st->objects[i].item_number) > -1)
        {
            obj = read_object(st->objects[i].item_number, VIRTUAL);
            obj->obj_flags.value[0] = st->objects[i].value[0];
            obj->obj_flags.value[1] = st->objects[i].value[1];
            obj->obj_flags.value[2] = st->objects[i].value[2];
            obj->obj_flags.value[3] = st->objects[i].value[3];
            obj->obj_flags.extra_flags = st->objects[i].extra_flags;
            obj->obj_flags.weight = st->objects[i].weight;
            obj->obj_flags.timer = st->objects[i].timer;
            obj->obj_flags.bitvector = st->objects[i].bitvector;

            for (j = 0; j < MAX_OBJ_AFFECT; j++)
                obj->affected[j] = st->objects[i].affected[j];

            obj_to_char(obj, ch);
        }
    }
}

void load_char_objs(struct char_data *ch)
{
    FILE *fl = NULL;
    float timegold = 0.0;
    int difference = 0;
    struct obj_file_u *st = NULL;
    char name[40] = "\0\0\0\0\0\0\0";
    char *t_ptr = NULL;
    char path[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    struct rental_header rh;

    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

    strcpy(name, GET_NAME(ch));
    t_ptr = name;
    for (; *t_ptr != '\0'; t_ptr++)
        *t_ptr = LOWER(*t_ptr);
    sprintf(path, "ply/%c/%s.o", name[0], name);

    if (!(fl = fopen(path, "r+b")))
    {
        log_info("no .o file for character");
        fl = fopen(path, "w+b");
        rh.inuse = 0;
        rh.length = 0;
        strcpy(rh.owner, "empty");
        fwrite(&rh, sizeof(rh), 1, fl);
    }
    else
        fread(&rh, sizeof(rh), 1, fl);

    if (rh.inuse == 1)
    {
        CREATE_VOID(st, char, rh.length);

        fread(st, rh.length, 1, fl);
        obj_store_to_char(ch, st);

        /*
         * if the character has been out for 12 real hours, they are fully healed
         * upon re-entry.  if they stay out for 24 full hours, all affects are
         * removed, including bad ones.
         */

        if (st->last_update + 6 * SECS_PER_REAL_HOUR < time(0))
        {
            RestoreChar(ch);
            RemAllAffects(ch);
        }
        if (ch->in_room == NOWHERE && st->last_update + 12 * SECS_PER_REAL_HOUR > time(0))
        {
            log_info("Char reconnecting after game crash");
        }
        else
        {
            if (ch->in_room == NOWHERE)
                log_info("Char reconnecting after autorent");
            timegold = (int)(((double)(st->total_cost) *
                              (((double)(time(0) - st->last_update)) / ((double)SECS_PER_REAL_DAY))));
            log_info("Char ran up charges of %g gold in rent", timegold);
            cprintf(ch, "You ran up charges of %g gold in rent.\r\n", timegold);
            /*
             * Sedna's hack begins here.
             * The butler is now friends with the banker.
             */
            difference = timegold - GET_GOLD(ch);
            GET_GOLD(ch) -= timegold;

            if (GET_GOLD(ch) < 0)
            {
                GET_BANK(ch) -= difference;
                if (GET_BANK(ch) < 0)
                {
                    log_info("Char ran out of money in rent-is flat broke");
                    cprintf(ch, "You ran out of money, you deadbeat.\r\n");
                    GET_GOLD(ch) = 0;
                    GET_BANK(ch) = 0;
                }
                else
                {
                    log_info("Char ran out of money in rent-withdrew from bank");
                    cprintf(ch, "You ran out of money, and had to make a quick trip to the bank.\r\n");
                    GET_GOLD(ch) = 0;
                }
            }
        }
        DESTROY(st);
    }
    else
    {
        log_info("Char has no rental data");
    }

    FCLOSE(fl);
    /*
     * Save char, to avoid strange data if crashing
     */

    save_char(ch, NOWHERE);
}

/*
 * Routines used to save a characters equipment from disk
 */

/* Puts object in store, at first item which has no -1 */
void put_obj_in_store(struct obj_data *obj, struct obj_file_u *st)
{
    int j = 0;
    struct obj_file_elem *oe = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %08zx", __PRETTY_FUNCTION__, SAFE_ONAME(obj), (size_t)st);

    if (st->nobjects >= MAX_OBJ_SAVE)
    {
        log_info("you want to rent more than %d items?!\n", st->nobjects);
        return;
    }
    oe = st->objects + st->nobjects;

    oe->item_number = obj_index[obj->item_number].vnum;
    oe->value[0] = obj->obj_flags.value[0];
    oe->value[1] = obj->obj_flags.value[1];
    oe->value[2] = obj->obj_flags.value[2];
    oe->value[3] = obj->obj_flags.value[3];

    oe->extra_flags = obj->obj_flags.extra_flags;
    oe->weight = obj->obj_flags.weight;
    oe->timer = obj->obj_flags.timer;
    oe->bitvector = obj->obj_flags.bitvector;
    for (j = 0; j < MAX_OBJ_AFFECT; j++)
        oe->affected[j] = obj->affected[j];

    st->nobjects++;
}

static int contained_weight(struct obj_data *container)
{
    struct obj_data *tmp = NULL;
    int rval = 0;

    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_ONAME(container));

    for (tmp = container->contains; tmp; tmp = tmp->next_content)
        rval += GET_OBJ_WEIGHT(tmp);
    return rval;
}

/* Destroy inventory after transferring it to "store inventory" */
void obj_to_store(struct obj_data *obj, struct obj_file_u *st, struct char_data *ch, int do_delete)
{
    if (DEBUG > 2)
        log_info("called %s with %s, %08zx, %s, %d", __PRETTY_FUNCTION__, SAFE_ONAME(obj), (size_t)st, SAFE_NAME(ch),
                 do_delete);

    if (!obj)
        return;

    obj_to_store(obj->contains, st, ch, do_delete);
    obj_to_store(obj->next_content, st, ch, do_delete);

    if ((obj->obj_flags.timer < 0) && (obj->obj_flags.timer != OBJ_NOTIMER))
    {
        if (do_delete)
        {
            cprintf(ch, "You're told: '%s is just old junk, I'll throw it away for you.'\r\n", obj->short_description);
            if (obj->in_obj)
                obj_from_obj(obj);
            obj_from_char(obj);
            extract_obj(obj);
        }
    }
    else if (obj->obj_flags.cost_per_day < 0)
    {
        if (do_delete)
        {
            cprintf(ch, "You're told: '%s is just old junk, I'll throw it away for you.'\r\n", obj->short_description);
            if (obj->in_obj)
                obj_from_obj(obj);
            obj_from_char(obj);
            extract_obj(obj);
        }
    }
    else if (obj->item_number == -1)
    {
        if (do_delete)
        {
            if (obj->in_obj)
                obj_from_obj(obj);
            obj_from_char(obj);
            extract_obj(obj);
        }
    }
    else
    {
        int weight = contained_weight(obj);

        GET_OBJ_WEIGHT(obj) -= weight;
        put_obj_in_store(obj, st);
        GET_OBJ_WEIGHT(obj) += weight;
        if (do_delete)
        {
            if (obj->in_obj)
                obj_from_obj(obj);
            extract_obj(obj);
        }
    }
}

/* write the vital data of a player to the player file */
void save_obj(struct char_data *ch, struct obj_cost *cost, int do_delete)
{
    FILE *fl = NULL;
    int i = 0;
    char name[40] = "\0\0\0\0\0\0\0";
    char *t_ptr = NULL;
    char path[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    static struct obj_file_u st;

    if (DEBUG > 2)
        log_info("called %s with %s, %08zx, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), (size_t)cost, do_delete);

    zero_rent(ch);

    st.nobjects = 0;
    st.gold_left = GET_GOLD(ch);
    st.total_cost = cost->total_cost;
    st.last_update = time(0);
    st.minimum_stay = 0; /* XXX where does this belong? */

    for (i = 0; i < MAX_WEAR; i++)
        if (ch->equipment[i])
        {
            if (do_delete)
            {
                obj_to_store(unequip_char(ch, i), &st, ch, do_delete);
            }
            else
            {
                obj_to_store(ch->equipment[i], &st, ch, do_delete);
            }
        }
    obj_to_store(ch->carrying, &st, ch, do_delete);

    if (do_delete)
        ch->carrying = 0;

    strcpy(name, GET_NAME(ch));
    t_ptr = name;
    for (; *t_ptr != '\0'; t_ptr++)
        *t_ptr = LOWER(*t_ptr);

    sprintf(path, "ply/%c/%s.o", name[0], name);
    if (!(fl = fopen(path, "w+b")))
    {
        log_fatal("saving PC's objects");
        proper_exit(MUD_HALT);
    }
    update_file(fl, name, &st);
    FCLOSE(fl);
}

void fwrite_obj(struct obj_data *obj, FILE *fp, int ObjId, int ContainedBy)
{
    int i = 0;
    struct extra_descr_data *ex = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %08zx, %d, %d", __PRETTY_FUNCTION__, SAFE_ONAME(obj), (size_t)fp, ObjId,
                 ContainedBy);

    fprintf(fp, "#ITEM\n");
    fprintf(fp, "ObjId              %d\n", ObjId);
    fprintf(fp, "ContainedBy        %d\n", ContainedBy);
    fprintf(fp, "Name               %s~\n", obj->name);
    fprintf(fp, "Description\n%s~\n", obj->description);
    fprintf(fp, "ShortDescr\n%s~\n", obj->short_description);
    fprintf(fp, "ActionDescr\n%s~\n", obj->action_description);
    fprintf(fp, "VNum               %d\n", ObjVnum(obj));
    fprintf(fp, "EquippedAt         %d\n", (int)obj->eq_pos);
    fprintf(fp, "Values             %d %d %d %d\n", obj->obj_flags.value[0], obj->obj_flags.value[1],
            obj->obj_flags.value[2], obj->obj_flags.value[3]);
    fprintf(fp, "ExtraFlags         %ld\n", obj->obj_flags.extra_flags);
    fprintf(fp, "Weight             %d\n", obj->obj_flags.weight);
    fprintf(fp, "Timer              %d\n", obj->obj_flags.timer);
    fprintf(fp, "BitVector          %ld\n", obj->obj_flags.bitvector);
    fprintf(fp, "Type               %d\n", (int)obj->obj_flags.type_flag);
    fprintf(fp, "WearFlags          %d\n", obj->obj_flags.wear_flags);
    fprintf(fp, "Cost               %d\n", obj->obj_flags.cost);
    fprintf(fp, "CostPerDay         %d\n", obj->obj_flags.cost_per_day);
    for (i = 0; i < MAX_OBJ_AFFECT; i++)
        fprintf(fp, "Affect             %d %d\n", (int)obj->affected[i].location, (int)obj->affected[i].modifier);
    for (ex = obj->ex_description; ex; ex = ex->next)
        fprintf(fp, "ExtraDescr         %s~\n%s~\n", ex->keyword, ex->description);
    fprintf(fp, "End\n");
}

int new_save_obj(struct char_data *ch, struct obj_data *obj, FILE *fp, int do_delete, int ObjId, int ContainedBy)
{
    if (DEBUG > 2)
        log_info("called %s with %s, %s, %08zx, %d, %d, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), SAFE_ONAME(obj),
                 (size_t)fp, do_delete, ObjId, ContainedBy);

    if (!obj)
        return ObjId - 1;

    ObjId = new_save_obj(ch, obj->contains, fp, do_delete, ObjId + 1, ObjId);
    ObjId = new_save_obj(ch, obj->next_content, fp, do_delete, ObjId + 1, ContainedBy);

    if ((obj->obj_flags.timer < 0) && (obj->obj_flags.timer != OBJ_NOTIMER))
    {
        if (do_delete)
        {
            cprintf(ch, "You think %s is just old junk and throw it away.\r\n", OBJS(obj, ch));
            if (obj->in_obj)
                obj_from_obj(obj);
            obj_from_char(obj);
            extract_obj(obj);
        }
        return ObjId - 1;
    }
    else if (obj->obj_flags.cost_per_day < 0)
    {
        if (do_delete)
        {
            cprintf(ch, "You think %s is just old junk and throw it away.\r\n", OBJS(obj, ch));
            if (obj->in_obj)
                obj_from_obj(obj);
            obj_from_char(obj);
            extract_obj(obj);
        }
        return ObjId - 1;
    }
    else if (obj->item_number == -1)
    {
        if (do_delete)
        {
            if (obj->in_obj)
                obj_from_obj(obj);
            obj_from_char(obj);
            extract_obj(obj);
        }
        return ObjId - 1;
    }
    else
    {
        int weight = contained_weight(obj);

        GET_OBJ_WEIGHT(obj) -= weight;
        fwrite_obj(obj, fp, ObjId, ContainedBy);
        GET_OBJ_WEIGHT(obj) += weight;
        if (do_delete)
        {
            if (obj->in_obj)
                obj_from_obj(obj);
            extract_obj(obj);
        }
    }
    return ObjId;
}

void new_save_equipment(struct char_data *ch, struct obj_cost *cost, int do_delete)
{
    FILE *fp = NULL;
    char name[40] = "\0\0\0\0\0\0\0";
    char filename[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char *t_ptr = NULL;
    int i = 0;
    int ObjId = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %08zx, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), (size_t)cost, do_delete);

    strcpy(name, GET_NAME(ch));
    t_ptr = name;
    for (; *t_ptr != '\0'; t_ptr++)
        *t_ptr = LOWER(*t_ptr);
    sprintf(filename, "ply/%c/%s.obj", name[0], name);

    zero_rent(ch);

    if (!(fp = fopen(filename, "w")))
    {
        log_fatal("new_save_equipment");
        proper_exit(MUD_HALT);
    }
    fprintf(fp, "#RENT\n");
    fprintf(fp, "Owner              %s~\n", GET_NAME(ch));
    fprintf(fp, "Gold               %d\n", GET_GOLD(ch));
    fprintf(fp, "TotalCost          %d\n", cost->total_cost);
    fprintf(fp, "LastUpdate         %ld\n", (long int)time(NULL));
    fprintf(fp, "MinimumStay        0\n");
    fprintf(fp, "End\n");
    fprintf(fp, "#EQUIPMENT\n");
    ObjId = -1;
    for (i = 0; i < MAX_WEAR; i++)
    {
        if (ch->equipment[i])
        {
            if (do_delete)
            {
                ObjId = new_save_obj(ch, unequip_char(ch, i), fp, do_delete, ObjId + 1, -1);
            }
            else
            {
                ObjId = new_save_obj(ch, ch->equipment[i], fp, do_delete, ObjId + 1, -1);
            }
        }
    }
    fprintf(fp, "End\n");
    fprintf(fp, "#CARRIED\n");
    ObjId = new_save_obj(ch, ch->carrying, fp, do_delete, ObjId + 1, -1);
    fprintf(fp, "End\n");
    fprintf(fp, "#END_RENT\n");
    if (do_delete)
        ch->carrying = 0;
    FCLOSE(fp);
}

int fread_object(struct obj_data *obj, FILE *fp)
{
    int fMatch = FALSE;
    int done = FALSE;
    char *word;
    static char End[4] = "End";

    if (DEBUG > 2)
        log_info("called %s with %s, %08zx", __PRETTY_FUNCTION__, SAFE_ONAME(obj), (size_t)fp);

    for (;;)
    { /* Get equipment slot information */
        word = feof(fp) ? End : fread_word(fp);
        fMatch = FALSE;

        switch (toupper(word[0]))
        {
        case '*':
            fMatch = TRUE;
            fread_to_eol(fp);
            break;
        case '#':
            fMatch = TRUE;
            fread_to_eol(fp);
            break;
        case 'E':
            if (!str_cmp(word, End))
            {
                fMatch = 1;
                done = 1;
                break;
            }
        }
        if (!fMatch)
        {
            log_error("Fread_char: no match.");
            if (!feof(fp))
                fread_to_eol(fp);
        }
        if (done)
            break;
    }
    return TRUE;
}

int new_load_equipment(struct char_data *ch, struct obj_cost *cost)
{
    FILE *fp = NULL;
    char name[40] = "\0\0\0\0\0\0\0";
    char filename[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char *t_ptr = NULL;
    static char End[4] = "End";

#if 0
    double                                  charges = 0.0;
    struct obj_data                        *obj = NULL;
    struct obj_indexing {
	int                                     ObjId;
	int                                     ContainedBy;
	struct obj_data                        *Myself;
	struct obj_data                        *MyContainer;
    }                                      *inventory;
    int                                     state = 0;
#endif
    int fMatch = FALSE;
    int done = FALSE;
    char *word = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s, %08zx", __PRETTY_FUNCTION__, SAFE_NAME(ch), (size_t)cost);

    strcpy(name, GET_NAME(ch));
    t_ptr = name;
    for (; *t_ptr != '\0'; t_ptr++)
        *t_ptr = LOWER(*t_ptr);
    sprintf(filename, "ply/%c/%s.obj", name[0], name);
    if (!(fp = fopen(filename, "r")))
    {
        log_info("%s has no rental history!", GET_NAME(ch));
        if (!(fp = fopen(filename, "w")))
        {
            log_fatal("new_load_equipment");
            proper_exit(MUD_HALT);
        }
        fprintf(fp, "#RENT\n");
        fprintf(fp, "Owner              %s~\n", GET_NAME(ch));
        fprintf(fp, "Gold               %d\n", GET_GOLD(ch));
        fprintf(fp, "TotalCost          %d\n", 0);
        fprintf(fp, "LastUpdate         %ld\n", (long int)time(NULL));
        fprintf(fp, "MinimumStay        0\n");
        fprintf(fp, "End\n");
        fprintf(fp, "#EQUIPMENT\n");
        fprintf(fp, "End\n");
        fprintf(fp, "#CARRIED\n");
        fprintf(fp, "End\n");
        fprintf(fp, "#END_RENT\n");
        FCLOSE(fp);
        return -1;
    }
    done = 0;
    for (;;)
    { /* Get rental information */
        word = feof(fp) ? End : fread_word(fp);
        fMatch = FALSE;

        switch (toupper(word[0]))
        {
        case '*':
            fMatch = TRUE;
            fread_to_eol(fp);
            break;
        case '#':
            fMatch = TRUE;
            fread_to_eol(fp);
            break;
        case 'E':
            if (!str_cmp(word, End))
            {
                fMatch = 1;
                done = 1;
                break;
            }
        case 'G':
            if (!str_cmp(word, "Gold"))
            {
                fMatch = TRUE;
                GET_GOLD(ch) = fread_number(fp);
                break;
            }
        case 'L':
            if (!str_cmp(word, "LastUpdate"))
            {
                fMatch = TRUE;
                fread_number(fp);
                break;
            }
        case 'M':
            if (!str_cmp(word, "MinimumStay"))
            {
                fMatch = TRUE;
                fread_number(fp);
                break;
            }
        case 'O':
            if (!str_cmp(word, "Owner"))
            {
                fMatch = TRUE;
                word = fread_word(fp);
                if (!str_cmp(word, name))
                    log_info("%s loading %s's equipment!", name, word);
                break;
            }
        case 'T':
            if (!str_cmp(word, "TotalCost"))
            {
                fMatch = TRUE;
                cost->total_cost = fread_number(fp);
                break;
            }
        }
        if (!fMatch)
        {
            log_error("Fread_char: no match.");
            if (!feof(fp))
                fread_to_eol(fp);
        }
        if (done)
            break;
    }
    /*
     * we've now gotten the rental header.... now we have to get the stuff
     */
    return FALSE;
}

/*
 * Routine Receptionist
 */

int receptionist(struct char_data *ch, int cmd, const char *arg)
{
    struct obj_cost cost;
    struct char_data *recep = NULL;
    struct char_data *temp_char = NULL;
    short int save_room = 0;
    short int action_tabel[9] = {23, 24, 36, 105, 106, 109, 111, 142, 147};

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (!ch->desc)
        return FALSE; /* You've forgot FALSE - NPC couldn't leave */

    for (temp_char = real_roomp(ch->in_room)->people; (temp_char) && (!recep); temp_char = temp_char->next_in_room)
        if (IS_MOB(temp_char))
            if (mob_index[temp_char->nr].func == receptionist)
                recep = temp_char;

    if (!recep)
    {
        log_fatal("No receptionist.\r\n");
        proper_exit(MUD_REBOOT);
    }
    if (IS_NPC(ch))
        return FALSE;

    if ((cmd != 256) && (cmd != 92) && (cmd != 93))
    {
        if (!cmd)
        {
            if (recep->specials.fighting)
            {
                return citizen(recep, 0, "");
            }
        }
        if (!number(0, 30))
            do_action(recep, "", action_tabel[number(0, 8)]);
        return FALSE;
    }
    if (!AWAKE(recep))
    {
        act("$e isn't able to talk to you...", FALSE, recep, 0, ch, TO_VICT);
        return TRUE;
    }
    if (!CAN_SEE(recep, ch))
    {
        act("$n says, 'I don't deal with people I can't see!'", FALSE, recep, 0, 0, TO_ROOM);
        return TRUE;
    }
    switch (cmd)
    {
    case 92: { /* Rent */
        if (recep_offer(ch, recep, &cost))
        {
            GET_HOME(ch) = ch->in_room;
            cprintf(ch, "Your home has been set to this room, %s\r\n", real_roomp(ch->in_room)->name);

            act("$n stores your stuff in the safe, and helps you into your chamber.", FALSE, recep, 0, ch, TO_VICT);
            act("$n helps $N into $S private chamber.", FALSE, recep, 0, ch, TO_NOTVICT);

            new_save_equipment(ch, &cost, FALSE);
            save_obj(ch, &cost, 1);
            save_room = ch->in_room;
            extract_char(ch);
            ch->in_room = save_room;
            save_char(ch, ch->in_room);
        }
    }
    break;
    case 93: { /* Offer */
        recep_offer(ch, recep, &cost);
        act("$N gives $n an offer.", FALSE, ch, 0, recep, TO_ROOM);
    }
    break;
    case 256: { /* sethome */
        GET_HOME(ch) = ch->in_room;
        save_char(ch, ch->in_room);
        cprintf(ch, "Your home has been set to this room, %s\r\n", real_roomp(ch->in_room)->name);
    }
    break;
    }
    return TRUE;
}

/* removes a player from the list of renters */
void zero_rent(struct char_data *ch)
{
    struct rental_header rh;
    FILE *fl = NULL;
    char name[40] = "\0\0\0\0\0\0\0";
    char *t_ptr = NULL;
    char path[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

    if (IS_NPC(ch))
        return;

    strcpy(name, GET_NAME(ch));
    t_ptr = name;
    for (; *t_ptr != '\0'; t_ptr++)
        *t_ptr = LOWER(*t_ptr);
    sprintf(path, "ply/%c/%s.o", name[0], name);
    if (!(fl = fopen(path, "w+b")))
    {
        log_fatal("saving PC's objects");
        proper_exit(MUD_HALT);
    }
    rh.inuse = 0;
    fwrite(&rh, sizeof(rh), 1, fl);
    FCLOSE(fl);
    return;
}

/*
 * This is for future use.  Right now, weights are pre-calculated so the
 * weight of a container IS the weight of itself plus all interior objects.
 * Later, to avoid cup problems, we may change it so you must query the
 * total weight of an object by summing the individual weights.
 */
int TotalWeight(struct obj_data *obj)
{
    struct obj_data *tmp = NULL;
    int rval = 0;

    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_ONAME(obj));

    if (!obj)
        return 0;
    rval = GET_OBJ_WEIGHT(obj);
    for (tmp = obj->contains; tmp; tmp = tmp->next_content)
        rval += TotalWeight(tmp);
    return rval;
}

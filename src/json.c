#include <stdio.h>
#include <stdlib.h>
/* #include <unistd.h> */
#include <sys/types.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "global.h"
#include "bug.h"
#include "version.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "mudlimits.h"
#include "trap.h"
#include "hash.h"
#include "spell_parser.h"
#include "whod.h"
#include "multiclass.h"
#include "modify.h"
#include "act_wiz.h"
#include "act_skills.h"
#include "spec_procs.h"
#include "tracking.h"
#include "scheduler.h"
#include "i3.h"
#include "sql.h"
#include "reboot.h"
#include "weather.h" // time_info_data struct
#define _JSON_C
#include "json.h"

int do_json(struct char_data *ch, const char *argument, int cmd)
{
    if (DEBUG)
        log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument), cmd);

    if (IS_NPC(ch))
        return TRUE;

    if (IS_SET(ch->specials.new_act, NEW_PLR_JSON))
    {
        cprintf(ch, "You will now see things as a normal player.\r\n");
        REMOVE_BIT(ch->specials.new_act, NEW_PLR_JSON);
    }
    else
    {
        cprintf(ch, "You will now get all output as JSON.\r\n");
        SET_BIT(ch->specials.new_act, NEW_PLR_JSON);
    }
    return TRUE;
}

#if 0

// Returns -3 for failure, -2 for lost target, -1 for target found, 0-6 for exit to follow
int _json_track(struct char_data *ch, struct char_data *vict)
{
    int code = 0;

    if ((!ch) || (!vict))
	return -3;        // error

    if (ch->in_room == vict->in_room) {
        return -1;          // Target found
    } else {
        code = choose_exit(ch->in_room, vict->in_room, ch->hunt_dist);
        if (code == -1) {
            return -2;      // Lost trail
        } else {
            return code;    // Direction 0..5 to go
        }
    }

    return -3;              // error
}

void _json_look(struct char_data *ch, const char *argument, int cmd)
{
    int res = -3;

    if (!ch->desc)
	return;

    cprintf(ch, "{\r\n");
    cprintf(ch, "    \"room\" : {\r\n");
    cprintf(ch, "        \"vnum\" : %d,\r\n", ch->in_room);
    cprintf(ch, "        \"title\": \"%s\",\r\n", real_roomp(ch->in_room)->name);
    cprintf(ch, "        \"is_dark\" : %d,\r\n", IS_DARK(ch->in_room) ? 1 : 0);
    cprintf(ch, "        \"is_dark_out\" : %d,\r\n", IS_DARKOUT(ch->in_room) ? 1 : 0);
    cprintf(ch, "        \"description\": \"%s\",\r\n", real_roomp(ch->in_room)->description);
    cprintf(ch, "        \"tracking\": ");

    if (IS_SET(ch->specials.act, IS_PC(ch) ? PLR_HUNTING : ACT_HUNTING) && ch->specials.hunting) {
        res = _json_track(ch, ch->specials.hunting);
        if( res < 0 ) {
            ch->specials.hunting = 0;
            ch->hunt_dist = 0;
            REMOVE_BIT(ch->specials.act, IS_PC(ch) ? PLR_HUNTING : ACT_HUNTING);
            cprintf(ch, "{\r\n");
            cprintf(ch, "            \"target\": {\r\n");
            cprintf(ch, "                \"vnum\" : %d,\r\n", IS_PC(ch)? -1 : mob_index[ch->nr].vnum);
            cprintf(ch, "                \"direction\" : %d\r\n", res);
            cprintf(ch, "            }\r\n");
            cprintf(ch, "        }\r\n");
        } else {
            ch->hunt_dist = 0;
            REMOVE_BIT(ch->specials.act, IS_PC(ch) ? PLR_HUNTING : ACT_HUNTING);
        }
    } else {
        cprintf(ch, "{},\r\n");
    }
    cprintf(ch, "        \"inventory\": ");
    cprintf(ch, _json_list_obj_in_room(real_roomp(ch->in_room)->contents, ch) ? "        },\r\n" : "{},\r\n");

    cprintf(ch, "        \"characters\": ");
    cprintf(ch, _json_list_char_in_room(real_roomp(ch->in_room)->contents, ch) ? "        },\r\n" : "{},\r\n");

    cprintf(ch, "    }\r\n");
    cprintf(ch, "}\r\n");

    do_exits(ch, "", cmd);
}

#endif

// This does NOT clear the buffer passed in, but appends to it.
void _json_serialize_object(int vnum, char *buf, size_t buf_len)
{
    struct obj_data *objp = NULL;
    int vnum_lookup = 0;

    if (!(objp = get_obj_num(vnum)))
    {
        // Nothing already loaded, try to load one!
        if (!(objp = read_object(vnum, VIRTUAL)))
        {
            // oops!
            return;
        }
    }
    vnum_lookup = (objp->item_number >= 0) ? obj_index[objp->item_number].vnum : 0;

    scprintf(buf, buf_len, "{\r\n");
    scprintf(buf, buf_len, "    \"vnum\" : %d,\r\n", (objp->item_number >= 0) ? obj_index[objp->item_number].vnum : 0);
    scprintf(buf, buf_len, "    \"name\" : \"%s\",\r\n", objp->name);

    scprintf(buf, buf_len, "}\r\n");
}

// This does NOT clear the buffer passed in, but appends to it.
void _json_sprintbit(unsigned long vektor, const char *names[], char *result, size_t result_len, char *prefix,
                     char *postfix, char *indent)
{
    scprintf(result, result_len, "%s", prefix); // "flags : {\r\n"
    for (int i = 0; i < sizeof(long) * 8; i++)
    {
        scprintf(result, result_len, "%s\"%s\" : %d%s\r\n", indent, names[i], IS_SET((1 << i), vektor) ? 1 : 0,
                 (i == (sizeof(long) * 8) - 1) ? "" : ",");
        // dangling , on last entry
    }
    scprintf(result, result_len, "%s", postfix); // "}\r\n"
}

// This does NOT clear the buffer passed in, but appends to it.
void _json_serialize_room(int vnum, char *buf, size_t buf_len)
{
    // char buf[buf_len] = "\0\0\0\0\0\0\0";
    struct room_data *roomp = NULL;
    struct zone_data *zonep = NULL;
    struct extra_descr_data *extra = NULL;

    roomp = real_roomp(vnum);
    zonep = &zone_table[roomp->zone];

    scprintf(buf, buf_len, "{\r\n");
    scprintf(buf, buf_len, "    \"vnum\" : %d,\r\n", roomp->number);
    scprintf(buf, buf_len, "    \"name\" : \"%s\",\r\n", roomp->name);
    scprintf(buf, buf_len, "    \"description\" : \"%s\",\r\n", roomp->description);
    scprintf(buf, buf_len, "    \"zone_number\" : %d,\r\n", roomp->zone);
    scprintf(buf, buf_len, "    \"zone_name\" : \"%s\",\r\n", zonep->name);
    scprintf(buf, buf_len, "    \"sector_type\" : %s,\r\n", sector_types[roomp->sector_type]);
    scprintf(buf, buf_len, "    \"river\" : {\r\n");
    if ((roomp->sector_type == SECT_WATER_SWIM) || (roomp->sector_type == SECT_WATER_NOSWIM))
    {
        double ttime = (double)roomp->river_speed / (double)10.0;
        struct room_data *riverp = real_roomp(roomp->dir_option[roomp->river_dir]->to_room);

        scprintf(buf, buf_len, "        \"speed\" : %3.1lf,\r\n", ttime);
        scprintf(buf, buf_len, "        \"direction\" : \"%s\",\r\n", dirs[roomp->river_dir]);
        scprintf(buf, buf_len, "        \"target_vnum\" : %d,\r\n", riverp ? riverp->number : -1);
        scprintf(buf, buf_len, "        \"target_room\" : \"%s\"\r\n", riverp ? riverp->name : "Swirling CHAOS");
    }
    scprintf(buf, buf_len, "    },\r\n");
    scprintf(buf, buf_len, "    \"teleport\" : {\r\n");
    if (roomp->tele_targ > 0)
    {
        double ttime = (double)roomp->tele_time / (double)10.0;
        struct room_data *telep = real_roomp(roomp->tele_targ);

        scprintf(buf, buf_len, "        \"speed\" : %3.1lf,\r\n", ttime);
        scprintf(buf, buf_len, "        \"target_vnum\" : %d,\r\n", telep ? telep->number : -1);
        scprintf(buf, buf_len, "        \"target_room\" : \"%s\"\r\n", telep ? telep->name : "Swirling CHAOS");
    }
    scprintf(buf, buf_len, "    },\r\n");

    scprintf(buf, buf_len, "    \"extra_descriptions\" : [\r\n");
    if ((extra = roomp->ex_description))
    {
        for (; extra; extra = extra->next)
        {
            scprintf(buf, buf_len, "        {\r\n");
            scprintf(buf, buf_len, "            \"keyword\" : \"%s\",\r\n", extra->keyword);
            scprintf(buf, buf_len, "            \"description\" : \"%s\"\r\n", extra->description);
            scprintf(buf, buf_len, "        }%s\r\n", extra->next ? "," : "");
            // dangling , on last entry
        }
    }
    scprintf(buf, buf_len, "    ],\r\n");
    scprintf(buf, buf_len, "    \"exits\" : {\r\n");
    for (int i = 0; i < 6; i++)
    {
        struct room_data *exitp = real_roomp(roomp->dir_option[i]->to_room);

        scprintf(buf, buf_len, "        \"%s\" : {\r\n", dirs[i]);
        scprintf(buf, buf_len, "            \"keyword\" : \"%s\",\r\n", roomp->dir_option[i]->keyword);
        scprintf(buf, buf_len, "            \"description\" : \"%s\",\r\n", roomp->dir_option[i]->general_description);
        _json_sprintbit((unsigned long)roomp->dir_option[i]->exit_info, exit_bits, buf, buf_len,
                        "            \"flags\" : {\r\n", "                ", "            },\r\n");
        scprintf(buf, buf_len, "            \"alias\" : \"%s\",\r\n", roomp->dir_option[i]->exit_alias);
        scprintf(buf, buf_len, "            \"key\" : \"%s\",\r\n", obj_index[roomp->dir_option[i]->key].name);
        scprintf(buf, buf_len, "            \"key_vnum\" : %d,\r\n", obj_index[roomp->dir_option[i]->key].number);
        scprintf(buf, buf_len, "            \"target_vnum\" : %d,\r\n", exitp ? exitp->number : -1);
        scprintf(buf, buf_len, "            \"target_room\" : \"%s\"\r\n", exitp ? exitp->name : "Swirling CHAOS");
        scprintf(buf, buf_len, "        }%s\r\n", (i == 5) ? "" : ",");
        // dangling , on last entry
    }
    scprintf(buf, buf_len, "    },\r\n");
    _json_sprintbit((unsigned long)roomp->room_flags, room_bits, buf, buf_len, "    \"flags\" : {\r\n", "        ",
                    "    },\r\n");
    scprintf(buf, buf_len, "    \"sound\" : {\r\n");
    if (roomp->room_flags & SOUND)
    {
        scprintf(buf, buf_len, "            \"local\" : \"%s\",\r\n", roomp->sound);
        scprintf(buf, buf_len, "            \"distant\" : \"%s\"\r\n", roomp->distant_sound);
    }
    else
    {
        scprintf(buf, buf_len, "            \"local\" : null,\r\n");
        scprintf(buf, buf_len, "            \"distant\" : null\r\n");
    }
    scprintf(buf, buf_len, "    },\r\n");
    if (roomp->funct)
    {
        scprintf(buf, buf_len, "    \"special_procedure\" : \"%s\",\r\n",
                 name_special_proc(SPECIAL_ROOM, roomp->number));
    }
    else
    {
        scprintf(buf, buf_len, "    \"special_procedure\" : null,\r\n");
    }
    // resets for this room, if any
    for (int zone = 0; zone <= top_of_zone_table; zone++)
    {
        // ZNAME and ZCMD are macros
        int last_cmd = 1;
        // struct char_data *last_mob_loaded = NULL;
        // struct char_data *mob = NULL;
        // struct obj_data *obj = NULL;
        // struct obj_data *obj_to = NULL;
        // struct room_data *rp = NULL;

        for (int cmd_no = 0;; cmd_no++)
        {
            if (ZCMD.command == 'S')
                break;
            if (last_cmd || !ZCMD.if_flag)
            {
            }
            else
            {
                last_cmd = 0;
            }
        }
    }
    // objects in room
    // chars in room
    scprintf(buf, buf_len, "}\r\n");
}

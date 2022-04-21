#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <cjson/cJSON.h>
#include "include/structs.h"
#include "include/main.h"
#include "include/utils.h"
#include "include/parse_wiley.h"

#define _DUMP_JSON_C
#include "include/dump_json.h"
#include "include/json_room.h"
#include "include/json_mob.h"
#include "include/json_obj.h"

// If you define FULL_SCHEMA, false booleans and empty objects
// will be added to the JSON output, so in all cases, all possible
// data elements will be present, otherwise, only non-empty and
// true things will be in the data.

//#define FULL_SCHEMA

#ifndef FULL_SCHEMA
#define cJSON_AddNullToObject(a, b)
#define cJSON_AddFalseToObject(a, b)
#endif

// JSON requires strings for keys...
char *vnum_to_string(int vnum)
{
    static char vnum_str[32];

    sprintf(vnum_str, "%d", vnum);
    return vnum_str;
}

// We use "dice" for some things...
cJSON *cJSON_AddDiceToObject(cJSON *this_thing, const char *KeyName, dice Dice)
{
    cJSON *dice = NULL;
    char tmp[MAX_STRING_LEN];

    dice = cJSON_AddObjectToObject(this_thing, KeyName);
    sprintf(tmp, "%dd%d%s%d", Dice.Rolls, Dice.Die, (Dice.Modifier < 0) ? "" : "+", Dice.Modifier);
    cJSON_AddStringToObject(dice, "dice", tmp);
    cJSON_AddNumberToObject(dice, "rolls", Dice.Rolls);
    cJSON_AddNumberToObject(dice, "die", Dice.Die);
    cJSON_AddNumberToObject(dice, "modifier", Dice.Modifier);

    return dice;
}

// We use "dice" for some things...
cJSON *cJSON_AddRawDiceToObject(cJSON *this_thing, const char *KeyName, int Rolls, int Die, int Modifier)
{
    cJSON *dice = NULL;
    char tmp[MAX_STRING_LEN];

    dice = cJSON_AddObjectToObject(this_thing, KeyName);
    sprintf(tmp, "%dd%d%s%d", Rolls, Die, (Modifier < 0) ? "" : "+", Modifier);
    cJSON_AddStringToObject(dice, "dice", tmp);
    cJSON_AddNumberToObject(dice, "rolls", Rolls);
    cJSON_AddNumberToObject(dice, "die", Die);
    cJSON_AddNumberToObject(dice, "modifier", Modifier);

    return dice;
}

// Tack on fixed zone header data
cJSON *process_zone_header(cJSON *this_zone, zones *Zones, int i)
{
    cJSON_AddNumberToObject(this_zone, "vnum", Zones->Zone[i].Number);
    cJSON_AddStringToObject(this_zone, "name", zone_name(Zones, Zones->Zone[i].Number));
    cJSON_AddNumberToObject(this_zone, "bottom", (!i ? 0 : Zones->Zone[i - 1].Top + 1));
    cJSON_AddNumberToObject(this_zone, "top", Zones->Zone[i].Top);
    if (Zones->Zone[i].Mode == ZONE_RESET_PC)
    {
        cJSON_AddStringToObject(this_zone, "reset_style", "Without Players");
        cJSON_AddNumberToObject(this_zone, "reset_frequency", Zones->Zone[i].Time);
    }
    else if (Zones->Zone[i].Mode == ZONE_RESET_ALWAYS)
    {
        cJSON_AddStringToObject(this_zone, "reset_style", "Always");
        cJSON_AddNumberToObject(this_zone, "reset_frequency", Zones->Zone[i].Time);
    }
    else if (Zones->Zone[i].Mode == ZONE_RESET_NEVER)
    {
        cJSON_AddStringToObject(this_zone, "reset_style", "Never");
        cJSON_AddNullToObject(this_zone, "reset_frequency");
    }
    else
    {
        cJSON_AddNullToObject(this_zone, "reset_style");
        cJSON_AddNullToObject(this_zone, "reset_frequency");
    }
    cJSON_AddNumberToObject(this_zone, "reset_commands", Zones->Zone[i].Count);

    // Here we can also dump all the rests for this zone, if we want.

    return this_zone;
}

// Find special procedure for the room/mob/object, if any.
cJSON *process_specials(cJSON *this_thing, int Vnum, int SpecialType)
{
    int special_proc = -1;
    cJSON *special = NULL;

    for (int j = 0; j < SpecialCount; j++)
    {
        if (Specials[j].ProgType == SpecialType)
        {
            if (Specials[j].Number == Vnum)
            {
                special_proc = j;
                break;
            }
        }
    }

    if (special_proc > -1)
    {
        special = cJSON_AddObjectToObject(this_thing, "special_procedure");
        cJSON_AddNumberToObject(special, "vnum", Specials[special_proc].Number);
        cJSON_AddStringToObject(special, "function", Specials[special_proc].FunctionName);
        cJSON_AddStringToObject(special, "description", Specials[special_proc].Description);
    }
    else
    {
        cJSON_AddNullToObject(this_thing, "special_procedure");
    }

    return special;
}

cJSON *process_flags(cJSON *this_thing, const char *FlagsetName, unsigned long Flags, const char **FlagNames)
{
    cJSON *flags = NULL;

#ifndef FULL_SCHEMA
    if (Flags != 0)
    {
        flags = cJSON_AddObjectToObject(this_thing, FlagsetName);
        for (int j = 0; j < sizeof(unsigned long) * 8; j++)
        {
            if (FlagNames[j] && FlagNames[j][0])
            {
                if (IS_SET((1 << j), Flags))
                {
                    cJSON_AddTrueToObject(flags, FlagNames[j]);
                }
            }
        }
    }
#else
    flags = cJSON_AddObjectToObject(this_thing, FlagsetName);
    for (int j = 0; j < sizeof(unsigned long) * 8; j++)
    {
        if (FlagNames[j] && FlagNames[j][0])
        {
            if (IS_SET((1 << j), Flags))
            {
                cJSON_AddTrueToObject(flags, FlagNames[j]);
            }
            else
            {
                cJSON_AddFalseToObject(flags, FlagNames[j]);
            }
        }
    }
#endif

    return flags;
}

// M - load mob to room                             ZONE_MOBILE, ZONE_MAX, ZONE_ROOM
// O - load obj to room                             ZONE_OBJECT, ZONE_MAX, ZONE_ROOM
// E - equip object to previous mob                 ZONE_OBJECT, ?, ZONE_POSITION
// G - give obj to previous mob                     ZONE_OBJECT, ?, ?
// D - set door state in room                       ZONE_DOOR_ROOM, ZONE_DOOR_EXIT, ZONE_DOOR_STATE
// R - remove obj from room                         ZONE_REMOVE_ROOM, ZONE_REMOVE_OBJ
// P - put obj in object                            ZONE_OBJECT, ?, ZONE_TARGET_OBJ
// L - load mob to be led by previous mob           ZONE_GROUP
// H - set hated flags of previous mob              ZONE_HATE_TYPE, ZONE_HATE_VALUE
// IfFlag - only do if previous command worked

// i is used as the room index, if -1 is passed in, rooms are not checked
// j is used as the zone index
// k is used as the reset index (within the zone)
cJSON *process_reset_segment(int *LastMob, int *LastLoc, int *LastObj, int *LeaderMob, int i, int j, int k,
                             rooms *Rooms, zones *Zones, objects *Objects, mobs *Mobs, int **reset_checkoffs)
{
    cJSON *this_reset = NULL;
    cJSON *cmd_room, *cmd_mob, *cmd_obj, *cmd_target;
    char cmd[2];
    char found_in[256];
    int mob_index = -1;
    int room_index = -1;
    int obj_index = -1;

    switch (Zones->Zone[j].Cmds[k].Command)
    {
    case ZONE_CMD_MOBILE:
        if (reset_checkoffs != NULL)
        {
            if (reset_checkoffs[j][k] < 1)
                break;
        }
        else
        {
            if (i != -1 && Zones->Zone[j].Cmds[k].Arg[ZONE_ROOM] != Rooms->Room[i].Number)
                break;
        }
        *LastMob = Zones->Zone[j].Cmds[k].Arg[ZONE_MOBILE];
        *LastLoc = Zones->Zone[j].Cmds[k].Arg[ZONE_ROOM];
        *LeaderMob = *LastMob;
        this_reset = cJSON_CreateObject();
        if (reset_checkoffs != NULL)
        {
            sprintf(found_in, "Found in zone \"%s\"[#%d], entry %d", Zones->Zone[j].Name, Zones->Zone[j].Number, k);
            cJSON_AddStringToObject(this_reset, "zone", found_in);
        }
        sprintf(cmd, "%c", Zones->Zone[j].Cmds[k].Command);
        cJSON_AddStringToObject(this_reset, "command", "load mobile to room");
        cJSON_AddNumberToObject(this_reset, "limit", Zones->Zone[j].Cmds[k].Arg[ZONE_MAX]);
        if (Zones->Zone[j].Cmds[k].IfFlag)
        {
            cJSON_AddTrueToObject(this_reset, "only_if_previous_ran");
        }
        else
        {
            cJSON_AddFalseToObject(this_reset, "only_if_previous_ran");
        }

        cmd_mob = cJSON_AddObjectToObject(this_reset, "mob");
        cJSON_AddNumberToObject(cmd_mob, "vnum", Zones->Zone[j].Cmds[k].Arg[ZONE_MOBILE]);
        cJSON_AddStringToObject(cmd_mob, "name", mob_name(Mobs, Zones->Zone[j].Cmds[k].Arg[ZONE_MOBILE]));
        mob_index = remap_mob_vnum(Mobs, Zones->Zone[j].Cmds[k].Arg[ZONE_MOBILE]);
        if (mob_index != -1 && Mobs->Mob[mob_index].Zone != Zones->Zone[j].Number)
        {
            // loading from outside ourselves!
            process_mob_zone_info("external_zone", cmd_mob, Mobs, mob_index, Zones);
        }
        else
        {
            cJSON_AddNullToObject(cmd_mob, "external_zone");
        }

        cmd_room = cJSON_AddObjectToObject(this_reset, "room");
        cJSON_AddNumberToObject(cmd_room, "vnum", Zones->Zone[j].Cmds[k].Arg[ZONE_ROOM]);
        cJSON_AddStringToObject(cmd_room, "name", room_name(Rooms, Zones->Zone[j].Cmds[k].Arg[ZONE_ROOM]));
        room_index = remap_room_vnum(Rooms, Zones->Zone[j].Cmds[k].Arg[ZONE_ROOM]);
        if (room_index != -1 && Rooms->Room[room_index].Zone != Zones->Zone[j].Number)
        {
            // loading from outside ourselves!
            process_room_zone_info("external_zone", cmd_room, Rooms, room_index, Zones);
        }
        else
        {
            cJSON_AddNullToObject(cmd_room, "external_zone");
        }
        // cJSON_AddItemToArray(resets, this_reset);
        break;

    case ZONE_CMD_OBJECT:
        if (reset_checkoffs != NULL)
        {
            if (reset_checkoffs[j][k] < 1)
                break;
        }
        else
        {
            if (i != -1 && Zones->Zone[j].Cmds[k].Arg[ZONE_ROOM] != Rooms->Room[i].Number)
                break;
        }
        *LastObj = Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT];
        *LastLoc = Zones->Zone[j].Cmds[k].Arg[ZONE_ROOM];
        this_reset = cJSON_CreateObject();
        if (reset_checkoffs != NULL)
        {
            sprintf(found_in, "Found in zone \"%s\"[#%d], entry %d", Zones->Zone[j].Name, Zones->Zone[j].Number, k);
            cJSON_AddStringToObject(this_reset, "zone", found_in);
        }
        sprintf(cmd, "%c", Zones->Zone[j].Cmds[k].Command);
        cJSON_AddStringToObject(this_reset, "command", "load object to room");
        cJSON_AddNumberToObject(this_reset, "limit", Zones->Zone[j].Cmds[k].Arg[ZONE_MAX]);
        if (Zones->Zone[j].Cmds[k].IfFlag)
        {
            cJSON_AddTrueToObject(this_reset, "only_if_previous_ran");
        }
        else
        {
            cJSON_AddFalseToObject(this_reset, "only_if_previous_ran");
        }

        cmd_obj = cJSON_AddObjectToObject(this_reset, "object");
        cJSON_AddNumberToObject(cmd_obj, "vnum", Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT]);
        cJSON_AddStringToObject(cmd_obj, "name", obj_name(Objects, Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT]));
        obj_index = remap_obj_vnum(Objects, Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT]);
        if (obj_index != -1 && Objects->Object[obj_index].Zone != Zones->Zone[j].Number)
        {
            // loading from outside ourselves!
            process_obj_zone_info("external_zone", cmd_obj, Objects, obj_index, Zones);
        }
        else
        {
            cJSON_AddNullToObject(cmd_obj, "external_zone");
        }

        cmd_room = cJSON_AddObjectToObject(this_reset, "room");
        cJSON_AddNumberToObject(cmd_room, "vnum", Zones->Zone[j].Cmds[k].Arg[ZONE_ROOM]);
        cJSON_AddStringToObject(cmd_room, "name", room_name(Rooms, Zones->Zone[j].Cmds[k].Arg[ZONE_ROOM]));
        room_index = remap_room_vnum(Rooms, Zones->Zone[j].Cmds[k].Arg[ZONE_ROOM]);
        if (room_index != -1 && Rooms->Room[room_index].Zone != Zones->Zone[j].Number)
        {
            // loading from outside ourselves!
            process_room_zone_info("external_zone", cmd_room, Rooms, room_index, Zones);
        }
        else
        {
            cJSON_AddNullToObject(cmd_room, "external_zone");
        }
        // cJSON_AddItemToArray(resets, this_reset);
        break;

    case ZONE_CMD_GIVE:
        if (reset_checkoffs != NULL)
        {
            if (reset_checkoffs[j][k] < 1)
                break;
        }
        // This has to use LastMob
        if (*LastMob < 0)
            break;
        *LastObj = Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT];
        this_reset = cJSON_CreateObject();
        if (reset_checkoffs != NULL)
        {
            sprintf(found_in, "Found in zone \"%s\"[#%d], entry %d", Zones->Zone[j].Name, Zones->Zone[j].Number, k);
            cJSON_AddStringToObject(this_reset, "zone", found_in);
        }
        sprintf(cmd, "%c", Zones->Zone[j].Cmds[k].Command);
        cJSON_AddStringToObject(this_reset, "command", "give object to mobile");
        cJSON_AddNumberToObject(this_reset, "limit", Zones->Zone[j].Cmds[k].Arg[ZONE_MAX]);
        if (Zones->Zone[j].Cmds[k].IfFlag)
        {
            cJSON_AddTrueToObject(this_reset, "only_if_previous_ran");
        }
        else
        {
            cJSON_AddFalseToObject(this_reset, "only_if_previous_ran");
        }

        cmd_obj = cJSON_AddObjectToObject(this_reset, "object");
        cJSON_AddNumberToObject(cmd_obj, "vnum", Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT]);
        cJSON_AddStringToObject(cmd_obj, "name", obj_name(Objects, Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT]));
        obj_index = remap_obj_vnum(Objects, Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT]);
        if (obj_index != -1 && Objects->Object[obj_index].Zone != Zones->Zone[j].Number)
        {
            // loading from outside ourselves!
            process_obj_zone_info("external_zone", cmd_obj, Objects, obj_index, Zones);
        }
        else
        {
            cJSON_AddNullToObject(cmd_obj, "external_zone");
        }

        cmd_mob = cJSON_AddObjectToObject(this_reset, "mob");
        cJSON_AddNumberToObject(cmd_mob, "vnum", *LastMob);
        cJSON_AddStringToObject(cmd_mob, "name", mob_name(Mobs, *LastMob));
        mob_index = remap_mob_vnum(Mobs, *LastMob);
        if (mob_index != -1 && Mobs->Mob[mob_index].Zone != Zones->Zone[j].Number)
        {
            // loading from outside ourselves!
            process_mob_zone_info("external_zone", cmd_mob, Mobs, mob_index, Zones);
        }
        else
        {
            cJSON_AddNullToObject(cmd_mob, "external_zone");
        }

        // cJSON_AddItemToArray(resets, this_reset);
        break;

    case ZONE_CMD_EQUIP:
        if (reset_checkoffs != NULL)
        {
            if (reset_checkoffs[j][k] < 1)
                break;
        }
        // This has to use LastMob
        if (*LastMob < 0)
            break;
        *LastObj = Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT];
        this_reset = cJSON_CreateObject();
        if (reset_checkoffs != NULL)
        {
            sprintf(found_in, "Found in zone \"%s\"[#%d], entry %d", Zones->Zone[j].Name, Zones->Zone[j].Number, k);
            cJSON_AddStringToObject(this_reset, "zone", found_in);
        }
        sprintf(cmd, "%c", Zones->Zone[j].Cmds[k].Command);
        cJSON_AddStringToObject(this_reset, "command", "equip object to mobile");
        cJSON_AddNumberToObject(this_reset, "limit", Zones->Zone[j].Cmds[k].Arg[ZONE_MAX]);
        if (Zones->Zone[j].Cmds[k].IfFlag)
        {
            cJSON_AddTrueToObject(this_reset, "only_if_previous_ran");
        }
        else
        {
            cJSON_AddFalseToObject(this_reset, "only_if_previous_ran");
        }

        cmd_obj = cJSON_AddObjectToObject(this_reset, "object");
        cJSON_AddNumberToObject(cmd_obj, "vnum", Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT]);
        cJSON_AddStringToObject(cmd_obj, "name", obj_name(Objects, Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT]));
        obj_index = remap_obj_vnum(Objects, Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT]);
        if (obj_index != -1 && Objects->Object[obj_index].Zone != Zones->Zone[j].Number)
        {
            // loading from outside ourselves!
            process_obj_zone_info("external_zone", cmd_obj, Objects, obj_index, Zones);
        }
        else
        {
            cJSON_AddNullToObject(cmd_obj, "external_zone");
        }

        cmd_mob = cJSON_AddObjectToObject(this_reset, "mob");
        cJSON_AddNumberToObject(cmd_mob, "vnum", *LastMob);
        cJSON_AddStringToObject(cmd_mob, "name", mob_name(Mobs, *LastMob));
        cJSON_AddStringToObject(this_reset, "position", equip_name(Zones->Zone[j].Cmds[k].Arg[ZONE_POSITION]));
        mob_index = remap_mob_vnum(Mobs, *LastMob);
        if (mob_index != -1 && Mobs->Mob[mob_index].Zone != Zones->Zone[j].Number)
        {
            // loading from outside ourselves!
            process_mob_zone_info("external_zone", cmd_mob, Mobs, mob_index, Zones);
        }
        else
        {
            cJSON_AddNullToObject(cmd_mob, "external_zone");
        }
        // cJSON_AddItemToArray(resets, this_reset);
        break;

    case ZONE_CMD_PUT:
        if (reset_checkoffs != NULL)
        {
            if (reset_checkoffs[j][k] < 1)
                break;
        }
        // We'll check LastLoc and LastObj to make sure
        // an object was loaded into THIS room.
        if (*LastObj < 0)
            break;
        this_reset = cJSON_CreateObject();
        if (reset_checkoffs != NULL)
        {
            sprintf(found_in, "Found in zone \"%s\"[#%d], entry %d", Zones->Zone[j].Name, Zones->Zone[j].Number, k);
            cJSON_AddStringToObject(this_reset, "zone", found_in);
        }
        sprintf(cmd, "%c", Zones->Zone[j].Cmds[k].Command);
        cJSON_AddStringToObject(this_reset, "command", "put object in object");
        cJSON_AddNumberToObject(this_reset, "limit", Zones->Zone[j].Cmds[k].Arg[ZONE_MAX]);
        if (Zones->Zone[j].Cmds[k].IfFlag)
        {
            cJSON_AddTrueToObject(this_reset, "only_if_previous_ran");
        }
        else
        {
            cJSON_AddFalseToObject(this_reset, "only_if_previous_ran");
        }

        cmd_obj = cJSON_AddObjectToObject(this_reset, "object");
        cJSON_AddNumberToObject(cmd_obj, "vnum", Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT]);
        cJSON_AddStringToObject(cmd_obj, "name", obj_name(Objects, Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT]));
        obj_index = remap_obj_vnum(Objects, Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT]);
        if (obj_index != -1 && Objects->Object[obj_index].Zone != Zones->Zone[j].Number)
        {
            // loading from outside ourselves!
            process_obj_zone_info("external_zone", cmd_obj, Objects, obj_index, Zones);
        }
        else
        {
            cJSON_AddNullToObject(cmd_obj, "external_zone");
        }

        cmd_target = cJSON_AddObjectToObject(this_reset, "target");
        cJSON_AddNumberToObject(cmd_target, "vnum", Zones->Zone[j].Cmds[k].Arg[ZONE_TARGET_OBJ]);
        cJSON_AddStringToObject(cmd_target, "name", obj_name(Objects, Zones->Zone[j].Cmds[k].Arg[ZONE_TARGET_OBJ]));
        obj_index = remap_obj_vnum(Objects, Zones->Zone[j].Cmds[k].Arg[ZONE_TARGET_OBJ]);
        if (obj_index != -1 && Objects->Object[obj_index].Zone != Zones->Zone[j].Number)
        {
            // loading from outside ourselves!
            process_obj_zone_info("external_zone", cmd_target, Objects, obj_index, Zones);
        }
        else
        {
            cJSON_AddNullToObject(cmd_target, "external_zone");
        }
        // cJSON_AddItemToArray(resets, this_reset);
        break;

    case ZONE_CMD_DOOR:
        if (reset_checkoffs != NULL)
        {
            if (reset_checkoffs[j][k] < 1)
                break;
        }
        else
        {
            if (i != -1 && Zones->Zone[j].Cmds[k].Arg[ZONE_DOOR_ROOM] != Rooms->Room[i].Number)
                break;
        }
        this_reset = cJSON_CreateObject();
        if (reset_checkoffs != NULL)
        {
            sprintf(found_in, "Found in zone \"%s\"[#%d], entry %d", Zones->Zone[j].Name, Zones->Zone[j].Number, k);
            cJSON_AddStringToObject(this_reset, "zone", found_in);
        }
        sprintf(cmd, "%c", Zones->Zone[j].Cmds[k].Command);
        cJSON_AddStringToObject(this_reset, "command", "set door state");
        if (Zones->Zone[j].Cmds[k].IfFlag)
        {
            cJSON_AddTrueToObject(this_reset, "only_if_previous_ran");
        }
        else
        {
            cJSON_AddFalseToObject(this_reset, "only_if_previous_ran");
        }

        cmd_target = cJSON_AddObjectToObject(this_reset, "door");
        cJSON_AddStringToObject(cmd_target, "name", exit_name(Zones->Zone[j].Cmds[k].Arg[ZONE_DOOR_EXIT]));
        cJSON_AddStringToObject(cmd_target, "state", doorstate_name(Zones->Zone[j].Cmds[k].Arg[ZONE_DOOR_STATE]));

        cmd_room = cJSON_AddObjectToObject(this_reset, "room");
        cJSON_AddNumberToObject(cmd_room, "vnum", Zones->Zone[j].Cmds[k].Arg[ZONE_DOOR_ROOM]);
        cJSON_AddStringToObject(cmd_room, "name", room_name(Rooms, Zones->Zone[j].Cmds[k].Arg[ZONE_DOOR_ROOM]));
        room_index = remap_room_vnum(Rooms, Zones->Zone[j].Cmds[k].Arg[ZONE_DOOR_ROOM]);
        if (room_index != -1 && Rooms->Room[room_index].Zone != Zones->Zone[j].Number)
        {
            // loading from outside ourselves!
            process_room_zone_info("external_zone", cmd_room, Rooms, room_index, Zones);
        }
        else
        {
            cJSON_AddNullToObject(cmd_room, "external_zone");
        }
        // cJSON_AddItemToArray(resets, this_reset);
        break;

    case ZONE_CMD_REMOVE:
        if (reset_checkoffs != NULL)
        {
            if (reset_checkoffs[j][k] < 1)
                break;
        }
        else
        {
            if (i != -1 && Zones->Zone[j].Cmds[k].Arg[ZONE_REMOVE_ROOM] != Rooms->Room[i].Number)
                break;
        }
        this_reset = cJSON_CreateObject();
        if (reset_checkoffs != NULL)
        {
            sprintf(found_in, "Found in zone \"%s\"[#%d], entry %d", Zones->Zone[j].Name, Zones->Zone[j].Number, k);
            cJSON_AddStringToObject(this_reset, "zone", found_in);
        }
        sprintf(cmd, "%c", Zones->Zone[j].Cmds[k].Command);
        cJSON_AddStringToObject(this_reset, "command", "remove object from room");
        if (Zones->Zone[j].Cmds[k].IfFlag)
        {
            cJSON_AddTrueToObject(this_reset, "only_if_previous_ran");
        }
        else
        {
            cJSON_AddFalseToObject(this_reset, "only_if_previous_ran");
        }

        cmd_obj = cJSON_AddObjectToObject(this_reset, "object");
        cJSON_AddNumberToObject(cmd_obj, "vnum", Zones->Zone[j].Cmds[k].Arg[ZONE_REMOVE_OBJ]);
        cJSON_AddStringToObject(cmd_obj, "name", obj_name(Objects, Zones->Zone[j].Cmds[k].Arg[ZONE_REMOVE_OBJ]));
        obj_index = remap_obj_vnum(Objects, Zones->Zone[j].Cmds[k].Arg[ZONE_REMOVE_OBJ]);
        if (obj_index != -1 && Objects->Object[obj_index].Zone != Zones->Zone[j].Number)
        {
            // loading from outside ourselves!
            process_obj_zone_info("external_zone", cmd_obj, Objects, obj_index, Zones);
        }
        else
        {
            cJSON_AddNullToObject(cmd_obj, "external_zone");
        }

        cmd_room = cJSON_AddObjectToObject(this_reset, "room");
        cJSON_AddNumberToObject(cmd_room, "vnum", Zones->Zone[j].Cmds[k].Arg[ZONE_REMOVE_ROOM]);
        cJSON_AddStringToObject(cmd_room, "name", room_name(Rooms, Zones->Zone[j].Cmds[k].Arg[ZONE_REMOVE_ROOM]));
        room_index = remap_room_vnum(Rooms, Zones->Zone[j].Cmds[k].Arg[ZONE_REMOVE_ROOM]);
        if (room_index != -1 && Rooms->Room[room_index].Zone != Zones->Zone[j].Number)
        {
            // loading from outside ourselves!
            process_room_zone_info("external_zone", cmd_room, Rooms, room_index, Zones);
        }
        else
        {
            cJSON_AddNullToObject(cmd_room, "external_zone");
        }
        // cJSON_AddItemToArray(resets, this_reset);
        break;

    case ZONE_CMD_LEAD:
        if (reset_checkoffs != NULL)
        {
            if (reset_checkoffs[j][k] < 1)
                break;
        }
        // This has to use LeaderMob
        if (*LeaderMob < 0)
            break;
        this_reset = cJSON_CreateObject();
        if (reset_checkoffs != NULL)
        {
            sprintf(found_in, "Found in zone \"%s\"[#%d], entry %d", Zones->Zone[j].Name, Zones->Zone[j].Number, k);
            cJSON_AddStringToObject(this_reset, "zone", found_in);
        }
        sprintf(cmd, "%c", Zones->Zone[j].Cmds[k].Command);
        cJSON_AddStringToObject(this_reset, "command", "load mobile to follow leader");
        if (Zones->Zone[j].Cmds[k].IfFlag)
        {
            cJSON_AddTrueToObject(this_reset, "only_if_previous_ran");
        }
        else
        {
            cJSON_AddFalseToObject(this_reset, "only_if_previous_ran");
        }

        cmd_mob = cJSON_AddObjectToObject(this_reset, "mob");
        cJSON_AddNumberToObject(cmd_mob, "vnum", Zones->Zone[j].Cmds[k].Arg[ZONE_MOBILE]);
        cJSON_AddStringToObject(cmd_mob, "name", mob_name(Mobs, Zones->Zone[j].Cmds[k].Arg[ZONE_MOBILE]));
        mob_index = remap_mob_vnum(Mobs, Zones->Zone[j].Cmds[k].Arg[ZONE_MOBILE]);
        if (mob_index != -1 && Mobs->Mob[mob_index].Zone != Zones->Zone[j].Number)
        {
            // loading from outside ourselves!
            process_mob_zone_info("external_zone", cmd_mob, Mobs, mob_index, Zones);
        }
        else
        {
            cJSON_AddNullToObject(cmd_mob, "external_zone");
        }

        cmd_target = cJSON_AddObjectToObject(this_reset, "leader");
        cJSON_AddNumberToObject(cmd_target, "vnum", *LeaderMob);
        cJSON_AddStringToObject(cmd_target, "name", mob_name(Mobs, *LeaderMob));
        mob_index = remap_mob_vnum(Mobs, *LeaderMob);
        if (mob_index != -1 && Mobs->Mob[mob_index].Zone != Zones->Zone[j].Number)
        {
            // loading from outside ourselves!
            process_mob_zone_info("external_zone", cmd_mob, Mobs, mob_index, Zones);
        }
        else
        {
            cJSON_AddNullToObject(cmd_mob, "external_zone");
        }

        cmd_room = cJSON_AddObjectToObject(this_reset, "room");
        cJSON_AddNumberToObject(cmd_room, "vnum", *LastLoc);
        cJSON_AddStringToObject(cmd_room, "name", room_name(Rooms, *LastLoc));
        room_index = remap_room_vnum(Rooms, *LastLoc);
        if (room_index != -1 && Rooms->Room[room_index].Zone != Zones->Zone[j].Number)
        {
            // loading from outside ourselves!
            process_room_zone_info("external_zone", cmd_room, Rooms, room_index, Zones);
        }
        else
        {
            cJSON_AddNullToObject(cmd_room, "external_zone");
        }
        // cJSON_AddItemToArray(resets, this_reset);
        *LastMob = Zones->Zone[j].Cmds[k].Arg[ZONE_MOBILE];
        break;

    case ZONE_CMD_HATE:
        if (reset_checkoffs != NULL)
        {
            if (reset_checkoffs[j][k] < 1)
                break;
        }
        // This has to use LastMob
        if (*LastMob < 0)
            break;
        this_reset = cJSON_CreateObject();
        if (reset_checkoffs != NULL)
        {
            sprintf(found_in, "Found in zone \"%s\"[#%d], entry %d", Zones->Zone[j].Name, Zones->Zone[j].Number, k);
            cJSON_AddStringToObject(this_reset, "zone", found_in);
        }
        sprintf(cmd, "%c", Zones->Zone[j].Cmds[k].Command);
        cJSON_AddStringToObject(this_reset, "command", "set hate status of mobile");
        if (Zones->Zone[j].Cmds[k].IfFlag)
        {
            cJSON_AddTrueToObject(this_reset, "only_if_previous_ran");
        }
        else
        {
            cJSON_AddFalseToObject(this_reset, "only_if_previous_ran");
        }

        cmd_mob = cJSON_AddObjectToObject(this_reset, "mob");
        cJSON_AddNumberToObject(cmd_mob, "vnum", *LastMob);
        cJSON_AddStringToObject(cmd_mob, "name", mob_name(Mobs, *LastMob));
        mob_index = remap_mob_vnum(Mobs, *LastMob);
        if (mob_index != -1 && Mobs->Mob[mob_index].Zone != Zones->Zone[j].Number)
        {
            // loading from outside ourselves!
            process_mob_zone_info("external_zone", cmd_mob, Mobs, mob_index, Zones);
        }
        else
        {
            cJSON_AddNullToObject(cmd_mob, "external_zone");
        }

        cmd_room = cJSON_AddObjectToObject(this_reset, "room");
        cJSON_AddNumberToObject(cmd_room, "vnum", *LastLoc);
        cJSON_AddStringToObject(cmd_room, "name", room_name(Rooms, *LastLoc));
        cJSON_AddStringToObject(
            this_reset, "hate",
            hate_name(Zones->Zone[j].Cmds[k].Arg[ZONE_HATE_TYPE], Zones->Zone[j].Cmds[k].Arg[ZONE_HATE_VALUE]));
        room_index = remap_room_vnum(Rooms, *LastLoc);
        if (room_index != -1 && Rooms->Room[room_index].Zone != Zones->Zone[j].Number)
        {
            // loading from outside ourselves!
            process_room_zone_info("external_zone", cmd_room, Rooms, room_index, Zones);
        }
        else
        {
            cJSON_AddNullToObject(cmd_room, "external_zone");
        }
        // cJSON_AddItemToArray(resets, this_reset);
        break;

    default:
        // Error...
        break;
    }

    return this_reset;
}

cJSON *process_orphaned_resets(cJSON *root, rooms *Rooms, zones *Zones, objects *Objects, mobs *Mobs,
                               int **reset_checkoffs)
{
    cJSON *resets = NULL;
    int orphaned_resets = 0;
    int total_resets = 0;

    for (int j = 0; j < Zones->Count; j++)
    {
        for (int k = 0; k < Zones->Zone[j].Count; k++)
        {
            if (reset_checkoffs[j][k] < 1)
            {
                orphaned_resets++;
                fprintf(stderr, "ORPHAN: %s[#%d] %d - %c %d %d %d %d\n", Zones->Zone[j].Name, Zones->Zone[j].Number, k,
                        Zones->Zone[j].Cmds[k].Command, Zones->Zone[j].Cmds[k].IfFlag, Zones->Zone[j].Cmds[k].Arg[0],
                        Zones->Zone[j].Cmds[k].Arg[1], Zones->Zone[j].Cmds[k].Arg[2]);
            }
            total_resets++;
        }
    }

    if (orphaned_resets > 0)
    {
        fprintf(stderr, "We found %d orphaned resets, out of %d total.\n", orphaned_resets, total_resets);
        resets = cJSON_AddArrayToObject(root, "orphaned_resets");
        for (int j = 0; j < Zones->Count; j++)
        {
            int LastMob = -1;
            int LastLoc = -1;
            int LastObj = -1;
            int LeaderMob = -1;
            for (int k = 0; k < Zones->Zone[j].Count; k++)
            {
                cJSON *this_reset = NULL;

                this_reset = process_reset_segment(&LastMob, &LastLoc, &LastObj, &LeaderMob, -1, j, k, Rooms, Zones,
                                                   Objects, Mobs, reset_checkoffs);
                if (this_reset)
                {
                    cJSON_AddItemToArray(resets, this_reset);
                }
            }
        }
    }
    else
    {
        cJSON_AddNullToObject(root, "orphaned_resets");
    }

    return resets;
}

cJSON *process_zone_resets(cJSON *this_zone, rooms *Rooms, zones *Zones, int j, objects *Objects, mobs *Mobs)
{
    cJSON *resets = NULL;
    int LastMob = -1;
    int LastLoc = -1;
    int LastObj = -1;
    int LeaderMob = -1;

    if (Zones->Zone[j].Count > 0)
    {
        resets = cJSON_AddArrayToObject(this_zone, "resets");
        for (int k = 0; k < Zones->Zone[j].Count; k++)
        {
            cJSON *this_reset = NULL;

            this_reset = process_reset_segment(&LastMob, &LastLoc, &LastObj, &LeaderMob, -1, j, k, Rooms, Zones,
                                               Objects, Mobs, NULL);
            if (this_reset)
            {
                cJSON_AddItemToArray(resets, this_reset);
            }
        }
    }
    else
    {
        cJSON_AddNullToObject(this_zone, "resets");
    }

    return resets;
}

void dump_as_json(zones *Zones, rooms *Rooms, objects *Objects, mobs *Mobs, shops *Shops, char *outdir)
{
    FILE *ofp = NULL;
    char *output = NULL;
    cJSON *root = NULL;
    cJSON *zones = NULL;
    //cJSON *rooms = NULL;
    //cJSON *mobiles = NULL;
    //cJSON *objects = NULL;
    int **reset_checkoffs = NULL;
    char outfile[MAX_STRING_LEN];

    snprintf(outfile, MAX_STRING_LEN, "%s/wileymud.json", outdir);
    root = cJSON_CreateObject();

    // When we process resets for each room, we will check them
    // off as applied.  Any resets NOT found by this are orphaned
    // and may have no result, or otherwise be in error.
    reset_checkoffs = calloc(Zones->Count, sizeof(int *));
    for (int j = 0; j < Zones->Count; j++)
    {
        reset_checkoffs[j] = calloc(Zones->Zone[j].Count, sizeof(int));
    }

    zones = cJSON_AddObjectToObject(root, "zones");
    for (int i = 0; i < Zones->Count; i++)
    {
        cJSON *this_zone = NULL;

        if (!Quiet)
        {
            status(stderr, "Dumping zone #%d (%s)...", Zones->Zone[i].Number, zone_name(Zones, Zones->Zone[i].Number));
            spin(stderr);
        }

        this_zone = cJSON_AddObjectToObject(zones, vnum_to_string(Zones->Zone[i].Number));

        process_zone_header(this_zone, Zones, i);
        // process_zone_resets(this_zone, Rooms, Zones, i, Objects, Mobs);

        process_rooms_in_zone(this_zone, Zones, i, Rooms, Objects, Mobs, reset_checkoffs);
        process_mobs_in_zone(this_zone, Zones, i, Rooms, Objects, Mobs, reset_checkoffs);
        process_objs_in_zone(this_zone, Zones, i, Rooms, Objects, Mobs, reset_checkoffs);

        if (!Quiet)
        {
            status(stderr, "done.");
            fprintf(stderr, "\r");
            fflush(stderr);
        }
    }

    // There shouldn't be any, but if any resets somehow escaped the room
    // or zone pass, they'd show up here...
    process_orphaned_resets(root, Rooms, Zones, Objects, Mobs, reset_checkoffs);

    // And now, we output!

    output = cJSON_Print(root);

    ofp = open_file(outfile, "w");
    fprintf(ofp, "%s", output);
    fclose(ofp);

    cJSON_Delete(root);

    // Here, we output to individual zone files, so repeat the same work again.
    for (int i = 0; i < Zones->Count; i++)
    {
        cJSON *sub_root = NULL;
        FILE *sub_fp = NULL;
        char *sub_output = NULL;

        if (!Quiet)
        {
            status(stderr, "RE-Dumping zone #%d (%s)...", Zones->Zone[i].Number, zone_name(Zones, Zones->Zone[i].Number));
            spin(stderr);
        }

        sub_root = cJSON_CreateObject();
        process_zone_header(sub_root, Zones, i);
        process_rooms_in_zone(sub_root, Zones, i, Rooms, Objects, Mobs, reset_checkoffs);
        process_mobs_in_zone(sub_root, Zones, i, Rooms, Objects, Mobs, reset_checkoffs);
        process_objs_in_zone(sub_root, Zones, i, Rooms, Objects, Mobs, reset_checkoffs);
        //process_orphaned_resets(sub_root, Rooms, Zones, Objects, Mobs, reset_checkoffs);
        sub_output = cJSON_Print(sub_root);

        snprintf(outfile, MAX_STRING_LEN, "%s/%s.json", outdir,
                zone_name(Zones, Zones->Zone[i].Number));
        sub_fp = open_file(outfile, "w");
        fprintf(sub_fp, "%s", sub_output);
        fclose(sub_fp);
        cJSON_Delete(sub_root);
        // sub_output would leak briefly, but who cares?

        if (!Quiet)
        {
            status(stderr, "done.");
            fprintf(stderr, "\r");
            fflush(stderr);
        }
    }

    if (!Quiet)
        fprintf(stderr, "done.\n");
}

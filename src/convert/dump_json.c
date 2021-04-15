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

// If you define FULL_SCHEMA, false booleans and empty objects
// will be added to the JSON output, so in all cases, all possible
// data elements will be present, otherwise, only non-empty and
// true things will be in the data.

//#define FULL_SCHEMA

#ifndef FULL_SCHEMA
#define cJSON_AddNullToObject(a,b)
#define cJSON_AddFalseToObject(a,b)
#endif

// unsigned long is currently 64 bits...
const char *room_flag_names[64] = {
    "dark", "death", "nomob", "indoors",
    "noattack", "nosteal", "nosummon", "nomagic",
    "", "private", "sound", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", ""
};

const char *act_flag_names[64] = {
    "has_special", "sentinel", "scavenger", "is_npc",
    "nice_thief", "aggressive", "stay_in_zone", "wimpy",
    "annoying", "hateful", "afraid", "immortal",
    "hunting", "deadly", "polyself", "polyother",
    "guardian", "use_item", "fighter_moves", "provides_food",
    "protector", "is_mount", "switch", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", ""
};

const char *aff_flag_names[64] = {
    "blind", "invisible", "detect_evil", "detect_invisible",
    "detect_magic", "sense_life", "silenced", "sanctuary",
    "group", "", "curse", "flying",
    "poison", "protection_from_evil", "paralysis", "infravision",
    "water_breathing", "sleep", "drug_free", "sneak",
    "hide", "fear", "charm", "follow",
    "", "true_sight", "scrying", "fireshield",
    "ride", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", ""
};

const char *wear_location_names[64] = {
    "takable", "finger", "neck", "body",
    "head", "legs", "feet", "hands",
    "arms", "shield", "cloak", "belt",
    "wrist", "wieldable", "held", "two_handed",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", ""
};

const char *item_extra_names[64] = {
    "glow", "hum", "metal", "mineral",
    "organic", "invisible", "magical", "undroppable",
    "blessed", "anti_good", "anti_evil", "anti_neutral",
    "anti_cleric", "anti_mage", "anti_thief", "anti_fighter",
    "anti_ranger", "parishable", "anti_druid", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", ""
};

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
    if (Zones->Zone[i].Mode == ZONE_RESET_PC) {
        cJSON_AddStringToObject(this_zone, "reset_style", "Without Players");
        cJSON_AddNumberToObject(this_zone, "reset_frequency", Zones->Zone[i].Time);
    } else if (Zones->Zone[i].Mode == ZONE_RESET_ALWAYS) {
        cJSON_AddStringToObject(this_zone, "reset_style", "Always");
        cJSON_AddNumberToObject(this_zone, "reset_frequency", Zones->Zone[i].Time);
    } else if (Zones->Zone[i].Mode == ZONE_RESET_NEVER) {
        cJSON_AddStringToObject(this_zone, "reset_style", "Never");
        cJSON_AddNullToObject(this_zone, "reset_frequency");
    } else {
        cJSON_AddNullToObject(this_zone, "reset_style");
        cJSON_AddNullToObject(this_zone, "reset_frequency");
    }
    cJSON_AddNumberToObject(this_zone, "reset_commands", Zones->Zone[i].Count);

    // Here we can also dump all the rests for this zone, if we want.

    return this_zone;
}

// Find special procedure for the room/mob/object, if any.
cJSON *process_specials(cJSON *this_thing, int Vnum, int SpecialType) {
    int special_proc = -1;
    cJSON *special = NULL;

    for(int j = 0; j < SpecialCount; j++) {
        if(Specials[j].ProgType == SpecialType) {
            if(Specials[j].Number == Vnum) {
                special_proc = j;
                break;
            }
        }
    }

    if(special_proc > -1) {
        special = cJSON_AddObjectToObject(this_thing, "special_procedure");
        cJSON_AddNumberToObject(special, "vnum", Specials[special_proc].Number);
        cJSON_AddStringToObject(special, "function", Specials[special_proc].FunctionName);
        cJSON_AddStringToObject(special, "description", Specials[special_proc].Description);
    } else {
        cJSON_AddNullToObject(this_thing, "special_procedure");
    }

    return special;
}

cJSON *process_flags(cJSON *this_thing, unsigned long Flags, const char **FlagNames) {
    cJSON *flags = NULL;

    flags = cJSON_AddObjectToObject(this_thing, "flags");
    for (int j = 0; j < sizeof(unsigned long) * 8; j++) {
        if(FlagNames[j] && FlagNames[j][0]) {
            if(IS_SET((1 << j), Flags)) {
                cJSON_AddTrueToObject(flags, FlagNames[j]);
            } else {
                cJSON_AddFalseToObject(flags, FlagNames[j]);
            }
        }
    }

    return flags;
}

cJSON *process_mob_zone_info(const char *KeyName, cJSON *this_mob, mobs *Mobs, int i, zones *Zones) {
    cJSON *zone = NULL;

    zone = cJSON_AddObjectToObject(this_mob, KeyName);
    cJSON_AddNumberToObject(zone, "vnum", Mobs->Mob[i].Zone);
    cJSON_AddStringToObject(zone, "name", zone_name(Zones, Mobs->Mob[i].Zone));

    return zone;
}

cJSON *process_obj_zone_info(const char *KeyName, cJSON *this_obj, objects *Objects, int i, zones *Zones) {
    cJSON *zone = NULL;

    zone = cJSON_AddObjectToObject(this_obj, KeyName);
    cJSON_AddNumberToObject(zone, "vnum", Objects->Object[i].Zone);
    cJSON_AddStringToObject(zone, "name", zone_name(Zones, Objects->Object[i].Zone));

    return zone;
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
cJSON *process_reset_segment(int *LastMob, int *LastLoc, int *LastObj, int *LeaderMob, int i, int j, int k, rooms *Rooms, zones *Zones, objects *Objects, mobs *Mobs, int **reset_checkoffs)
{
    cJSON *this_reset = NULL;
    cJSON *cmd_room, *cmd_mob, *cmd_obj, *cmd_target;
    char cmd[2];
    char found_in[256];
    int mob_index = -1;
    int room_index = -1;
    int obj_index = -1;

    switch (Zones->Zone[j].Cmds[k].Command) {
        case ZONE_CMD_MOBILE:
            if(reset_checkoffs != NULL) {
                if(reset_checkoffs[j][k] < 1)
                    break;
            } else {
                if(i != -1 && Zones->Zone[j].Cmds[k].Arg[ZONE_ROOM] != Rooms->Room[i].Number)
                    break;
            }
            *LastMob = Zones->Zone[j].Cmds[k].Arg[ZONE_MOBILE];
            *LastLoc = Zones->Zone[j].Cmds[k].Arg[ZONE_ROOM];
            *LeaderMob = *LastMob;
            this_reset  = cJSON_CreateObject();
            if(reset_checkoffs != NULL) {
                sprintf(found_in, "Found in zone \"%s\"[#%d], entry %d", Zones->Zone[j].Name, Zones->Zone[j].Number, k);
                cJSON_AddStringToObject(this_reset, "zone", found_in);
            }
            sprintf(cmd, "%c", Zones->Zone[j].Cmds[k].Command);
            cJSON_AddStringToObject(this_reset, "command", "load mobile to room");
            cJSON_AddNumberToObject(this_reset, "limit", Zones->Zone[j].Cmds[k].Arg[ZONE_MAX]);
            if( Zones->Zone[j].Cmds[k].IfFlag ) {
                cJSON_AddTrueToObject(this_reset, "only_if_previous_ran");
            } else {
                cJSON_AddFalseToObject(this_reset, "only_if_previous_ran");
            }

            cmd_mob = cJSON_AddObjectToObject(this_reset, "mob");
            cJSON_AddNumberToObject(cmd_mob, "vnum", Zones->Zone[j].Cmds[k].Arg[ZONE_MOBILE]);
            cJSON_AddStringToObject(cmd_mob, "name", mob_name(Mobs, Zones->Zone[j].Cmds[k].Arg[ZONE_MOBILE]));
            mob_index = remap_mob_vnum(Mobs, Zones->Zone[j].Cmds[k].Arg[ZONE_MOBILE]);
            if(mob_index != -1 && Mobs->Mob[mob_index].Zone != Zones->Zone[j].Number) {
                // loading from outside ourselves!
                process_mob_zone_info("external_zone", cmd_mob, Mobs, mob_index , Zones);
            } else {
                cJSON_AddNullToObject(cmd_mob, "external_zone");
            }

            cmd_room = cJSON_AddObjectToObject(this_reset, "room");
            cJSON_AddNumberToObject(cmd_room, "vnum", Zones->Zone[j].Cmds[k].Arg[ZONE_ROOM]);
            cJSON_AddStringToObject(cmd_room, "name", room_name(Rooms, Zones->Zone[j].Cmds[k].Arg[ZONE_ROOM]));
            room_index = remap_room_vnum(Rooms, Zones->Zone[j].Cmds[k].Arg[ZONE_ROOM]);
            if(room_index != -1 && Rooms->Room[room_index].Zone != Zones->Zone[j].Number) {
                // loading from outside ourselves!
                process_room_zone_info("external_zone", cmd_room, Rooms, room_index , Zones);
            } else {
                cJSON_AddNullToObject(cmd_room, "external_zone");
            }
            //cJSON_AddItemToArray(resets, this_reset);
            break;

        case ZONE_CMD_OBJECT:
            if(reset_checkoffs != NULL) {
                if(reset_checkoffs[j][k] < 1)
                    break;
            } else {
                if(i != -1 && Zones->Zone[j].Cmds[k].Arg[ZONE_ROOM] != Rooms->Room[i].Number)
                    break;
            }
            *LastObj = Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT];
            *LastLoc = Zones->Zone[j].Cmds[k].Arg[ZONE_ROOM];
            this_reset  = cJSON_CreateObject();
            if(reset_checkoffs != NULL) {
                sprintf(found_in, "Found in zone \"%s\"[#%d], entry %d", Zones->Zone[j].Name, Zones->Zone[j].Number, k);
                cJSON_AddStringToObject(this_reset, "zone", found_in);
            }
            sprintf(cmd, "%c", Zones->Zone[j].Cmds[k].Command);
            cJSON_AddStringToObject(this_reset, "command", "load object to room");
            cJSON_AddNumberToObject(this_reset, "limit", Zones->Zone[j].Cmds[k].Arg[ZONE_MAX]);
            if( Zones->Zone[j].Cmds[k].IfFlag ) {
                cJSON_AddTrueToObject(this_reset, "only_if_previous_ran");
            } else {
                cJSON_AddFalseToObject(this_reset, "only_if_previous_ran");
            }

            cmd_obj = cJSON_AddObjectToObject(this_reset, "object");
            cJSON_AddNumberToObject(cmd_obj, "vnum", Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT]);
            cJSON_AddStringToObject(cmd_obj, "name", obj_name(Objects, Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT]));
            obj_index = remap_obj_vnum(Objects, Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT]);
            if(obj_index != -1 && Objects->Object[obj_index].Zone != Zones->Zone[j].Number) {
                // loading from outside ourselves!
                process_obj_zone_info("external_zone", cmd_obj, Objects, obj_index , Zones);
            } else {
                cJSON_AddNullToObject(cmd_obj, "external_zone");
            }

            cmd_room = cJSON_AddObjectToObject(this_reset, "room");
            cJSON_AddNumberToObject(cmd_room, "vnum", Zones->Zone[j].Cmds[k].Arg[ZONE_ROOM]);
            cJSON_AddStringToObject(cmd_room, "name", room_name(Rooms, Zones->Zone[j].Cmds[k].Arg[ZONE_ROOM]));
            room_index = remap_room_vnum(Rooms, Zones->Zone[j].Cmds[k].Arg[ZONE_ROOM]);
            if(room_index != -1 && Rooms->Room[room_index].Zone != Zones->Zone[j].Number) {
                // loading from outside ourselves!
                process_room_zone_info("external_zone", cmd_room, Rooms, room_index , Zones);
            } else {
                cJSON_AddNullToObject(cmd_room, "external_zone");
            }
            //cJSON_AddItemToArray(resets, this_reset);
            break;

        case ZONE_CMD_GIVE:
            if(reset_checkoffs != NULL) {
                if(reset_checkoffs[j][k] < 1)
                    break;
            }
            // This has to use LastMob
            if(*LastMob < 0)
                break;
            *LastObj = Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT];
            this_reset  = cJSON_CreateObject();
            if(reset_checkoffs != NULL) {
                sprintf(found_in, "Found in zone \"%s\"[#%d], entry %d", Zones->Zone[j].Name, Zones->Zone[j].Number, k);
                cJSON_AddStringToObject(this_reset, "zone", found_in);
            }
            sprintf(cmd, "%c", Zones->Zone[j].Cmds[k].Command);
            cJSON_AddStringToObject(this_reset, "command", "give object to mobile");
            cJSON_AddNumberToObject(this_reset, "limit", Zones->Zone[j].Cmds[k].Arg[ZONE_MAX]);
            if( Zones->Zone[j].Cmds[k].IfFlag ) {
                cJSON_AddTrueToObject(this_reset, "only_if_previous_ran");
            } else {
                cJSON_AddFalseToObject(this_reset, "only_if_previous_ran");
            }

            cmd_obj = cJSON_AddObjectToObject(this_reset, "object");
            cJSON_AddNumberToObject(cmd_obj, "vnum", Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT]);
            cJSON_AddStringToObject(cmd_obj, "name", obj_name(Objects, Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT]));
            obj_index = remap_obj_vnum(Objects, Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT]);
            if(obj_index != -1 && Objects->Object[obj_index].Zone != Zones->Zone[j].Number) {
                // loading from outside ourselves!
                process_obj_zone_info("external_zone", cmd_obj, Objects, obj_index , Zones);
            } else {
                cJSON_AddNullToObject(cmd_obj, "external_zone");
            }

            cmd_mob = cJSON_AddObjectToObject(this_reset, "mob");
            cJSON_AddNumberToObject(cmd_mob, "vnum", *LastMob);
            cJSON_AddStringToObject(cmd_mob, "name", mob_name(Mobs, *LastMob));
            mob_index = remap_mob_vnum(Mobs, *LastMob);
            if(mob_index != -1 && Mobs->Mob[mob_index].Zone != Zones->Zone[j].Number) {
                // loading from outside ourselves!
                process_mob_zone_info("external_zone", cmd_mob, Mobs, mob_index , Zones);
            } else {
                cJSON_AddNullToObject(cmd_mob, "external_zone");
            }

            //cJSON_AddItemToArray(resets, this_reset);
            break;

        case ZONE_CMD_EQUIP:
            if(reset_checkoffs != NULL) {
                if(reset_checkoffs[j][k] < 1)
                    break;
            }
            // This has to use LastMob
            if(*LastMob < 0)
                break;
            *LastObj = Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT];
            this_reset  = cJSON_CreateObject();
            if(reset_checkoffs != NULL) {
                sprintf(found_in, "Found in zone \"%s\"[#%d], entry %d", Zones->Zone[j].Name, Zones->Zone[j].Number, k);
                cJSON_AddStringToObject(this_reset, "zone", found_in);
            }
            sprintf(cmd, "%c", Zones->Zone[j].Cmds[k].Command);
            cJSON_AddStringToObject(this_reset, "command", "equip object to mobile");
            cJSON_AddNumberToObject(this_reset, "limit", Zones->Zone[j].Cmds[k].Arg[ZONE_MAX]);
            if( Zones->Zone[j].Cmds[k].IfFlag ) {
                cJSON_AddTrueToObject(this_reset, "only_if_previous_ran");
            } else {
                cJSON_AddFalseToObject(this_reset, "only_if_previous_ran");
            }

            cmd_obj = cJSON_AddObjectToObject(this_reset, "object");
            cJSON_AddNumberToObject(cmd_obj, "vnum", Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT]);
            cJSON_AddStringToObject(cmd_obj, "name", obj_name(Objects, Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT]));
            obj_index = remap_obj_vnum(Objects, Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT]);
            if(obj_index != -1 && Objects->Object[obj_index].Zone != Zones->Zone[j].Number) {
                // loading from outside ourselves!
                process_obj_zone_info("external_zone", cmd_obj, Objects, obj_index , Zones);
            } else {
                cJSON_AddNullToObject(cmd_obj, "external_zone");
            }

            cmd_mob = cJSON_AddObjectToObject(this_reset, "mob");
            cJSON_AddNumberToObject(cmd_mob, "vnum", *LastMob);
            cJSON_AddStringToObject(cmd_mob, "name", mob_name(Mobs, *LastMob));
            cJSON_AddStringToObject(this_reset, "position", equip_name(Zones->Zone[j].Cmds[k].Arg[ZONE_POSITION]));
            mob_index = remap_mob_vnum(Mobs, *LastMob);
            if(mob_index != -1 && Mobs->Mob[mob_index].Zone != Zones->Zone[j].Number) {
                // loading from outside ourselves!
                process_mob_zone_info("external_zone", cmd_mob, Mobs, mob_index , Zones);
            } else {
                cJSON_AddNullToObject(cmd_mob, "external_zone");
            }
            //cJSON_AddItemToArray(resets, this_reset);
            break;

        case ZONE_CMD_PUT:
            if(reset_checkoffs != NULL) {
                if(reset_checkoffs[j][k] < 1)
                    break;
            }
            // We'll check LastLoc and LastObj to make sure
            // an object was loaded into THIS room.
            if(*LastObj < 0)
                break;
            this_reset  = cJSON_CreateObject();
            if(reset_checkoffs != NULL) {
                sprintf(found_in, "Found in zone \"%s\"[#%d], entry %d", Zones->Zone[j].Name, Zones->Zone[j].Number, k);
                cJSON_AddStringToObject(this_reset, "zone", found_in);
            }
            sprintf(cmd, "%c", Zones->Zone[j].Cmds[k].Command);
            cJSON_AddStringToObject(this_reset, "command", "put object in object");
            cJSON_AddNumberToObject(this_reset, "limit", Zones->Zone[j].Cmds[k].Arg[ZONE_MAX]);
            if( Zones->Zone[j].Cmds[k].IfFlag ) {
                cJSON_AddTrueToObject(this_reset, "only_if_previous_ran");
            } else {
                cJSON_AddFalseToObject(this_reset, "only_if_previous_ran");
            }

            cmd_obj = cJSON_AddObjectToObject(this_reset, "object");
            cJSON_AddNumberToObject(cmd_obj, "vnum", Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT]);
            cJSON_AddStringToObject(cmd_obj, "name", obj_name(Objects, Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT]));
            obj_index = remap_obj_vnum(Objects, Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT]);
            if(obj_index != -1 && Objects->Object[obj_index].Zone != Zones->Zone[j].Number) {
                // loading from outside ourselves!
                process_obj_zone_info("external_zone", cmd_obj, Objects, obj_index , Zones);
            } else {
                cJSON_AddNullToObject(cmd_obj, "external_zone");
            }

            cmd_target = cJSON_AddObjectToObject(this_reset, "target");
            cJSON_AddNumberToObject(cmd_target, "vnum", Zones->Zone[j].Cmds[k].Arg[ZONE_TARGET_OBJ]);
            cJSON_AddStringToObject(cmd_target, "name", obj_name(Objects, Zones->Zone[j].Cmds[k].Arg[ZONE_TARGET_OBJ]));
            obj_index = remap_obj_vnum(Objects, Zones->Zone[j].Cmds[k].Arg[ZONE_TARGET_OBJ]);
            if(obj_index != -1 && Objects->Object[obj_index].Zone != Zones->Zone[j].Number) {
                // loading from outside ourselves!
                process_obj_zone_info("external_zone", cmd_target, Objects, obj_index , Zones);
            } else {
                cJSON_AddNullToObject(cmd_target, "external_zone");
            }
            //cJSON_AddItemToArray(resets, this_reset);
            break;

        case ZONE_CMD_DOOR:
            if(reset_checkoffs != NULL) {
                if(reset_checkoffs[j][k] < 1)
                    break;
            } else {
                if(i != -1 && Zones->Zone[j].Cmds[k].Arg[ZONE_DOOR_ROOM] != Rooms->Room[i].Number)
                    break;
            }
            this_reset  = cJSON_CreateObject();
            if(reset_checkoffs != NULL) {
                sprintf(found_in, "Found in zone \"%s\"[#%d], entry %d", Zones->Zone[j].Name, Zones->Zone[j].Number, k);
                cJSON_AddStringToObject(this_reset, "zone", found_in);
            }
            sprintf(cmd, "%c", Zones->Zone[j].Cmds[k].Command);
            cJSON_AddStringToObject(this_reset, "command", "set door state");
            if( Zones->Zone[j].Cmds[k].IfFlag ) {
                cJSON_AddTrueToObject(this_reset, "only_if_previous_ran");
            } else {
                cJSON_AddFalseToObject(this_reset, "only_if_previous_ran");
            }

            cmd_target = cJSON_AddObjectToObject(this_reset, "door");
            cJSON_AddStringToObject(cmd_target, "name", exit_name(Zones->Zone[j].Cmds[k].Arg[ZONE_DOOR_EXIT]));
            cJSON_AddStringToObject(cmd_target, "state", doorstate_name(Zones->Zone[j].Cmds[k].Arg[ZONE_DOOR_STATE]));

            cmd_room = cJSON_AddObjectToObject(this_reset, "room");
            cJSON_AddNumberToObject(cmd_room, "vnum", Zones->Zone[j].Cmds[k].Arg[ZONE_DOOR_ROOM]);
            cJSON_AddStringToObject(cmd_room, "name", room_name(Rooms, Zones->Zone[j].Cmds[k].Arg[ZONE_DOOR_ROOM]));
            room_index = remap_room_vnum(Rooms, Zones->Zone[j].Cmds[k].Arg[ZONE_DOOR_ROOM]);
            if(room_index != -1 && Rooms->Room[room_index].Zone != Zones->Zone[j].Number) {
                // loading from outside ourselves!
                process_room_zone_info("external_zone", cmd_room, Rooms, room_index , Zones);
            } else {
                cJSON_AddNullToObject(cmd_room, "external_zone");
            }
            //cJSON_AddItemToArray(resets, this_reset);
            break;

        case ZONE_CMD_REMOVE:
            if(reset_checkoffs != NULL) {
                if(reset_checkoffs[j][k] < 1)
                    break;
            } else {
                if(i != -1 && Zones->Zone[j].Cmds[k].Arg[ZONE_REMOVE_ROOM] != Rooms->Room[i].Number)
                    break;
            }
            this_reset  = cJSON_CreateObject();
            if(reset_checkoffs != NULL) {
                sprintf(found_in, "Found in zone \"%s\"[#%d], entry %d", Zones->Zone[j].Name, Zones->Zone[j].Number, k);
                cJSON_AddStringToObject(this_reset, "zone", found_in);
            }
            sprintf(cmd, "%c", Zones->Zone[j].Cmds[k].Command);
            cJSON_AddStringToObject(this_reset, "command", "remove object from room");
            if( Zones->Zone[j].Cmds[k].IfFlag ) {
                cJSON_AddTrueToObject(this_reset, "only_if_previous_ran");
            } else {
                cJSON_AddFalseToObject(this_reset, "only_if_previous_ran");
            }

            cmd_obj = cJSON_AddObjectToObject(this_reset, "object");
            cJSON_AddNumberToObject(cmd_obj, "vnum", Zones->Zone[j].Cmds[k].Arg[ZONE_REMOVE_OBJ]);
            cJSON_AddStringToObject(cmd_obj, "name", obj_name(Objects, Zones->Zone[j].Cmds[k].Arg[ZONE_REMOVE_OBJ]));
            obj_index = remap_obj_vnum(Objects, Zones->Zone[j].Cmds[k].Arg[ZONE_REMOVE_OBJ]);
            if(obj_index != -1 && Objects->Object[obj_index].Zone != Zones->Zone[j].Number) {
                // loading from outside ourselves!
                process_obj_zone_info("external_zone", cmd_obj, Objects, obj_index , Zones);
            } else {
                cJSON_AddNullToObject(cmd_obj, "external_zone");
            }

            cmd_room = cJSON_AddObjectToObject(this_reset, "room");
            cJSON_AddNumberToObject(cmd_room, "vnum", Zones->Zone[j].Cmds[k].Arg[ZONE_REMOVE_ROOM]);
            cJSON_AddStringToObject(cmd_room, "name", room_name(Rooms, Zones->Zone[j].Cmds[k].Arg[ZONE_REMOVE_ROOM]));
            room_index = remap_room_vnum(Rooms, Zones->Zone[j].Cmds[k].Arg[ZONE_REMOVE_ROOM]);
            if(room_index != -1 && Rooms->Room[room_index].Zone != Zones->Zone[j].Number) {
                // loading from outside ourselves!
                process_room_zone_info("external_zone", cmd_room, Rooms, room_index , Zones);
            } else {
                cJSON_AddNullToObject(cmd_room, "external_zone");
            }
            //cJSON_AddItemToArray(resets, this_reset);
            break;

        case ZONE_CMD_LEAD:
            if(reset_checkoffs != NULL) {
                if(reset_checkoffs[j][k] < 1)
                    break;
            }
            // This has to use LeaderMob
            if(*LeaderMob < 0)
                break;
            this_reset  = cJSON_CreateObject();
            if(reset_checkoffs != NULL) {
                sprintf(found_in, "Found in zone \"%s\"[#%d], entry %d", Zones->Zone[j].Name, Zones->Zone[j].Number, k);
                cJSON_AddStringToObject(this_reset, "zone", found_in);
            }
            sprintf(cmd, "%c", Zones->Zone[j].Cmds[k].Command);
            cJSON_AddStringToObject(this_reset, "command", "load mobile to follow leader");
            if( Zones->Zone[j].Cmds[k].IfFlag ) {
                cJSON_AddTrueToObject(this_reset, "only_if_previous_ran");
            } else {
                cJSON_AddFalseToObject(this_reset, "only_if_previous_ran");
            }

            cmd_mob = cJSON_AddObjectToObject(this_reset, "mob");
            cJSON_AddNumberToObject(cmd_mob, "vnum", Zones->Zone[j].Cmds[k].Arg[ZONE_MOBILE]);
            cJSON_AddStringToObject(cmd_mob, "name", mob_name(Mobs, Zones->Zone[j].Cmds[k].Arg[ZONE_MOBILE]));
            mob_index = remap_mob_vnum(Mobs, Zones->Zone[j].Cmds[k].Arg[ZONE_MOBILE]);
            if(mob_index != -1 && Mobs->Mob[mob_index].Zone != Zones->Zone[j].Number) {
                // loading from outside ourselves!
                process_mob_zone_info("external_zone", cmd_mob, Mobs, mob_index , Zones);
            } else {
                cJSON_AddNullToObject(cmd_mob, "external_zone");
            }

            cmd_target = cJSON_AddObjectToObject(this_reset, "leader");
            cJSON_AddNumberToObject(cmd_target, "vnum", *LeaderMob);
            cJSON_AddStringToObject(cmd_target, "name", mob_name(Mobs, *LeaderMob));
            mob_index = remap_mob_vnum(Mobs, *LeaderMob);
            if(mob_index != -1 && Mobs->Mob[mob_index].Zone != Zones->Zone[j].Number) {
                // loading from outside ourselves!
                process_mob_zone_info("external_zone", cmd_mob, Mobs, mob_index , Zones);
            } else {
                cJSON_AddNullToObject(cmd_mob, "external_zone");
            }

            cmd_room = cJSON_AddObjectToObject(this_reset, "room");
            cJSON_AddNumberToObject(cmd_room, "vnum", *LastLoc);
            cJSON_AddStringToObject(cmd_room, "name", room_name(Rooms, *LastLoc));
            room_index = remap_room_vnum(Rooms, *LastLoc);
            if(room_index != -1 && Rooms->Room[room_index].Zone != Zones->Zone[j].Number) {
                // loading from outside ourselves!
                process_room_zone_info("external_zone", cmd_room, Rooms, room_index , Zones);
            } else {
                cJSON_AddNullToObject(cmd_room, "external_zone");
            }
            //cJSON_AddItemToArray(resets, this_reset);
            *LastMob = Zones->Zone[j].Cmds[k].Arg[ZONE_MOBILE];
            break;

        case ZONE_CMD_HATE:
            if(reset_checkoffs != NULL) {
                if(reset_checkoffs[j][k] < 1)
                    break;
            }
            // This has to use LastMob
            if(*LastMob < 0)
                break;
            this_reset  = cJSON_CreateObject();
            if(reset_checkoffs != NULL) {
                sprintf(found_in, "Found in zone \"%s\"[#%d], entry %d", Zones->Zone[j].Name, Zones->Zone[j].Number, k);
                cJSON_AddStringToObject(this_reset, "zone", found_in);
            }
            sprintf(cmd, "%c", Zones->Zone[j].Cmds[k].Command);
            cJSON_AddStringToObject(this_reset, "command", "set hate status of mobile");
            if( Zones->Zone[j].Cmds[k].IfFlag ) {
                cJSON_AddTrueToObject(this_reset, "only_if_previous_ran");
            } else {
                cJSON_AddFalseToObject(this_reset, "only_if_previous_ran");
            }

            cmd_mob = cJSON_AddObjectToObject(this_reset, "mob");
            cJSON_AddNumberToObject(cmd_mob, "vnum", *LastMob);
            cJSON_AddStringToObject(cmd_mob, "name", mob_name(Mobs, *LastMob));
            mob_index = remap_mob_vnum(Mobs, *LastMob);
            if(mob_index != -1 && Mobs->Mob[mob_index].Zone != Zones->Zone[j].Number) {
                // loading from outside ourselves!
                process_mob_zone_info("external_zone", cmd_mob, Mobs, mob_index , Zones);
            } else {
                cJSON_AddNullToObject(cmd_mob, "external_zone");
            }

            cmd_room = cJSON_AddObjectToObject(this_reset, "room");
            cJSON_AddNumberToObject(cmd_room, "vnum", *LastLoc);
            cJSON_AddStringToObject(cmd_room, "name", room_name(Rooms, *LastLoc));
            cJSON_AddStringToObject(this_reset, "hate", hate_name(Zones->Zone[j].Cmds[k].Arg[ZONE_HATE_TYPE], Zones->Zone[j].Cmds[k].Arg[ZONE_HATE_VALUE]));
            room_index = remap_room_vnum(Rooms, *LastLoc);
            if(room_index != -1 && Rooms->Room[room_index].Zone != Zones->Zone[j].Number) {
                // loading from outside ourselves!
                process_room_zone_info("external_zone", cmd_room, Rooms, room_index , Zones);
            } else {
                cJSON_AddNullToObject(cmd_room, "external_zone");
            }
            //cJSON_AddItemToArray(resets, this_reset);
            break;

        default:
            // Error...
            break;
    }

    return this_reset;
}

cJSON *process_orphaned_resets(cJSON *root, rooms *Rooms, zones *Zones, objects *Objects, mobs *Mobs, int **reset_checkoffs) {
    cJSON *resets = NULL;
    int orphaned_resets = 0;
    int total_resets = 0;

    for (int j = 0; j < Zones->Count; j++) {
        for (int k = 0; k < Zones->Zone[j].Count; k++) {
            if(reset_checkoffs[j][k] < 1) {
                orphaned_resets++;
                fprintf(stderr, "ORPHAN: %s[#%d] %d - %c %d %d %d %d\n",
                        Zones->Zone[j].Name,
                        Zones->Zone[j].Number,
                        k,
                        Zones->Zone[j].Cmds[k].Command,
                        Zones->Zone[j].Cmds[k].IfFlag,
                        Zones->Zone[j].Cmds[k].Arg[0],
                        Zones->Zone[j].Cmds[k].Arg[1],
                        Zones->Zone[j].Cmds[k].Arg[2]
                       );
            }
            total_resets++;
        }
    }

    if(orphaned_resets > 0) {
        fprintf(stderr, "We found %d orphaned resets, out of %d total.\n", orphaned_resets, total_resets);
        resets = cJSON_AddArrayToObject(root, "orphaned_resets");
        for (int j = 0; j < Zones->Count; j++) {
            int LastMob = -1;
            int LastLoc = -1;
            int LastObj = -1;
            int LeaderMob = -1;
            for (int k = 0; k < Zones->Zone[j].Count; k++) {
                cJSON *this_reset = NULL;

                this_reset = process_reset_segment(&LastMob, &LastLoc, &LastObj, &LeaderMob, -1, j, k, Rooms, Zones, Objects, Mobs, reset_checkoffs);
                if(this_reset) {
                    cJSON_AddItemToArray(resets, this_reset);
                }
            }
        }
    } else {
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

    if(Zones->Zone[j].Count > 0) {
        resets = cJSON_AddArrayToObject(this_zone, "resets");
        for (int k = 0; k < Zones->Zone[j].Count; k++) {
            cJSON *this_reset = NULL;

            this_reset = process_reset_segment(&LastMob, &LastLoc, &LastObj, &LeaderMob, -1, j, k, Rooms, Zones, Objects, Mobs, NULL);
            if(this_reset) {
                cJSON_AddItemToArray(resets, this_reset);
            }
        }
    } else {
        cJSON_AddNullToObject(this_zone, "resets");
    }

    return resets;
}

cJSON *process_mob_header(cJSON *this_mob, mobs *Mobs, int i)
{
    cJSON *alignment = NULL;

    cJSON_AddNumberToObject(this_mob, "vnum", Mobs->Mob[i].Number);
    cJSON_AddStringToObject(this_mob, "name", mob_name(Mobs, Mobs->Mob[i].Number));
    cJSON_AddStringToObject(this_mob, "short_description", Mobs->Mob[i].ShortDesc);
    cJSON_AddStringToObject(this_mob, "long_description", Mobs->Mob[i].LongDesc);
    cJSON_AddStringToObject(this_mob, "description", Mobs->Mob[i].Description);
    cJSON_AddNumberToObject(this_mob, "height", Mobs->Mob[i].Height);
    cJSON_AddNumberToObject(this_mob, "weight", Mobs->Mob[i].Weight);
    cJSON_AddStringToObject(this_mob, "sex", sex_name(Mobs->Mob[i].Sex));

    alignment = cJSON_AddObjectToObject(this_mob, "alignment");
    cJSON_AddStringToObject(alignment, "name", alignment_name(Mobs->Mob[i].Alignment));
    cJSON_AddNumberToObject(alignment, "value", Mobs->Mob[i].Alignment);

    cJSON_AddStringToObject(this_mob, "race", race_name(Mobs->Mob[i].Race));
    // TODO Class might be more than one, may need to write a function to make this
    // into an array...
    cJSON_AddStringToObject(this_mob, "class", class_name(Mobs->Mob[i].Class));
    cJSON_AddNumberToObject(this_mob, "level", Mobs->Mob[i].Level);
    cJSON_AddDiceToObject(this_mob, "gold", Mobs->Mob[i].Gold);
    cJSON_AddDiceToObject(this_mob, "experience", Mobs->Mob[i].Experience);

    cJSON_AddDiceToObject(this_mob, "hit_points", Mobs->Mob[i].HitPoints);
    cJSON_AddNumberToObject(this_mob, "mana_points", Mobs->Mob[i].ManaPoints);
    cJSON_AddNumberToObject(this_mob, "move_points", Mobs->Mob[i].MovePoints);

    cJSON_AddNumberToObject(this_mob, "armor_class", Mobs->Mob[i].ArmourClass);
    cJSON_AddNumberToObject(this_mob, "to_hit", Mobs->Mob[i].ToHit);

    // TODO These might also have multiple values, like Class up above...
    cJSON_AddStringToObject(this_mob, "immune", immunity_name(Mobs->Mob[i].Immunity));
    cJSON_AddStringToObject(this_mob, "resist", immunity_name(Mobs->Mob[i].Resistance));
    cJSON_AddStringToObject(this_mob, "susceptible", immunity_name(Mobs->Mob[i].Susceptible));

    cJSON_AddStringToObject(this_mob, "position", position_name(Mobs->Mob[i].Position));
    cJSON_AddStringToObject(this_mob, "default_position", position_name(Mobs->Mob[i].DefaultPosition));

    return this_mob;
}

cJSON *process_mob_abilities(cJSON *this_mob, mobs *Mobs, int i)
{
    cJSON *abilities = NULL;
    cJSON *strength = NULL;

    abilities = cJSON_AddObjectToObject(this_mob, "abilities");
    strength = cJSON_AddObjectToObject(abilities, "strength");
    cJSON_AddDiceToObject(strength, "score", Mobs->Mob[i].Strength);
    cJSON_AddDiceToObject(strength, "extra", Mobs->Mob[i].ExtraStrength);

    cJSON_AddDiceToObject(abilities, "dexterity", Mobs->Mob[i].Dexterity);
    cJSON_AddDiceToObject(abilities, "constitution", Mobs->Mob[i].Constitution);
    cJSON_AddDiceToObject(abilities, "intelligence", Mobs->Mob[i].Intelligence);
    cJSON_AddDiceToObject(abilities, "wisdom", Mobs->Mob[i].Wisdom);

    return abilities;
}

cJSON *process_mob_saving_throws(cJSON *this_mob, mobs *Mobs, int i)
{
    cJSON *saving_throws = NULL;

    saving_throws = cJSON_AddObjectToObject(this_mob, "saving_throws");
    cJSON_AddNumberToObject(saving_throws, "paralysis", Mobs->Mob[i].SavingThrow[0]);
    cJSON_AddNumberToObject(saving_throws, "rod", Mobs->Mob[i].SavingThrow[1]);
    cJSON_AddNumberToObject(saving_throws, "petrification", Mobs->Mob[i].SavingThrow[2]);
    cJSON_AddNumberToObject(saving_throws, "breath_weapons", Mobs->Mob[i].SavingThrow[3]);
    cJSON_AddNumberToObject(saving_throws, "spells", Mobs->Mob[i].SavingThrow[4]);

    return saving_throws;
}

cJSON *process_mob_sounds(cJSON *this_mob, mobs *Mobs, int i)
{
    cJSON *sound = NULL;

    if ( (Mobs->Mob[i].Sound && *Mobs->Mob[i].Sound) || (Mobs->Mob[i].DistantSound && *Mobs->Mob[i].DistantSound)) {
        sound = cJSON_AddObjectToObject(this_mob, "sound");

        if (Mobs->Mob[i].Sound && *Mobs->Mob[i].Sound) {
            cJSON_AddStringToObject(sound, "in_room", Mobs->Mob[i].Sound);
        } else {
            cJSON_AddNullToObject(sound, "in_room");
        }
        if (Mobs->Mob[i].DistantSound && *Mobs->Mob[i].DistantSound) {
            cJSON_AddStringToObject(sound, "adjacent", Mobs->Mob[i].DistantSound);
        } else {
            cJSON_AddNullToObject(sound, "adjacent");
        }
    } else {
        cJSON_AddNullToObject(this_mob, "sound");
    }

    return sound;
}

cJSON *process_mob_attacks(cJSON *this_mob, mobs *Mobs, int i)
{
    cJSON *attacks = NULL;

    if(Mobs->Mob[i].AttackCount > 0) {
        attacks = cJSON_AddArrayToObject(this_mob, "attacks");
        for(int j = 0; j < Mobs->Mob[i].AttackCount; j++) {
            cJSON *this_attack = NULL;
            char tmp[MAX_STRING_LEN];

            sprintf(tmp, "%dd%d%s%d", Mobs->Mob[i].Attack[j].Rolls,
                    Mobs->Mob[i].Attack[j].Die,
                    (Mobs->Mob[i].Attack[j].Modifier < 0) ? "" : "+", Mobs->Mob[i].Attack[j].Modifier);

            this_attack = cJSON_CreateObject();
            cJSON_AddStringToObject(this_attack, "dice", tmp);
            cJSON_AddNumberToObject(this_attack, "rolls", Mobs->Mob[i].Attack[j].Rolls);
            cJSON_AddNumberToObject(this_attack, "die", Mobs->Mob[i].Attack[j].Die);
            cJSON_AddNumberToObject(this_attack, "modifier", Mobs->Mob[i].Attack[j].Modifier);
            // TODO There is no number-to-name function for attack types yet...
            cJSON_AddNumberToObject(this_attack, "type_id", Mobs->Mob[i].Attack[j].Type);
            cJSON_AddStringToObject(this_attack, "type", damage_name(Mobs->Mob[i].Attack[j].Type));

            cJSON_AddItemToArray(attacks, this_attack);
        }
    } else {
        cJSON_AddNullToObject(this_mob, "attacks");
    }

    return attacks;
}

cJSON *process_mob_skills(cJSON *this_mob, mobs *Mobs, int i)
{
    cJSON *skills = NULL;

    if(Mobs->Mob[i].SkillCount > 0) {
        skills = cJSON_AddArrayToObject(this_mob, "skills");
        for(int j = 0; j < Mobs->Mob[i].SkillCount; j++) {
            cJSON *this_skill = NULL;

            this_skill = cJSON_CreateObject();
            // TODO There is no number-to-name function for skills yet...
            cJSON_AddNumberToObject(this_skill, "number", Mobs->Mob[i].Skill[j].Number);
            cJSON_AddStringToObject(this_skill, "name", spell_name(Mobs->Mob[i].Skill[j].Number));
            cJSON_AddNumberToObject(this_skill, "learned", Mobs->Mob[i].Skill[j].Learned);
            cJSON_AddNumberToObject(this_skill, "recognize", Mobs->Mob[i].Skill[j].Recognise);

            cJSON_AddItemToArray(skills, this_skill);
        }
    } else {
        cJSON_AddNullToObject(this_mob, "skills");
    }

    return skills;
}

cJSON *process_obj_header(cJSON *this_obj, objects *Objects, int i)
{
    cJSON_AddNumberToObject(this_obj, "vnum", Objects->Object[i].Number);
    cJSON_AddStringToObject(this_obj, "name", obj_name(Objects, Objects->Object[i].Number));
    cJSON_AddStringToObject(this_obj, "short_description", Objects->Object[i].ShortDesc);
    cJSON_AddStringToObject(this_obj, "action_description", Objects->Object[i].ActionDesc);
    cJSON_AddStringToObject(this_obj, "description", Objects->Object[i].Description);
    cJSON_AddNumberToObject(this_obj, "weight", Objects->Object[i].Weight);
    // If Value < 0, the object cannot be sold.
    cJSON_AddNumberToObject(this_obj, "value", Objects->Object[i].Value);
    cJSON_AddNumberToObject(this_obj, "rent", Objects->Object[i].Rent);
    cJSON_AddNumberToObject(this_obj, "timer", Objects->Object[i].Timer);

    return this_obj;
}

cJSON *process_obj_extra_descriptions(cJSON *this_obj, objects *Objects, int i)
{
    cJSON *extra = NULL;

    if (Objects->Object[i].ExtraCount) {

        extra = cJSON_AddArrayToObject(this_obj, "extra_descriptions");
        for (int j = 0; j < Objects->Object[i].ExtraCount; j++) {
            cJSON *this_extra = NULL;
            cJSON *keywords = NULL;

            this_extra = cJSON_CreateObject();
            if(Objects->Object[i].Extra[j].Keyword->Count > 0) {
                keywords = cJSON_CreateStringArray((const char *const *) Objects->Object[i].Extra[j].Keyword->Word, Objects->Object[i].Extra[j].Keyword->Count);
                cJSON_AddItemToObject(this_extra, "keywords", keywords);
            } else {
                cJSON_AddNullToObject(this_extra, "keywords");
            }
            cJSON_AddStringToObject(this_extra, "description", Objects->Object[i].Extra[j].Description);
            cJSON_AddItemToArray(extra, this_extra);
        }
    } else {
        cJSON_AddNullToObject(this_obj, "extra_descriptions");
    }

    return extra;
}

cJSON *process_obj_type_info(cJSON *this_obj, objects *Objects, int i)
{
    cJSON *type_details = NULL;
    int found = 0;

    // Type
    // AffectCount, Affect[]
    // Flags

    switch (Objects->Object[i].Type) {
        case ITEM_LIGHT:
            type_details = cJSON_AddObjectToObject(this_obj, "type_details");
            cJSON_AddNumberToObject(type_details, "type_id", Objects->Object[i].Type);
            cJSON_AddStringToObject(type_details, "name", item_type_name(Objects->Object[i].Type));
            cJSON_AddNumberToObject(type_details, "duration", Objects->Object[i].Flags.Value[2]);
            // TODO get useful values for these, instead of just raw numbers
            cJSON_AddNumberToObject(type_details, "light_type", Objects->Object[i].Flags.Value[1]);
            cJSON_AddNumberToObject(type_details, "light_color", Objects->Object[i].Flags.Value[0]);
            break;
        case ITEM_SCROLL:
        case ITEM_POTION:
            type_details = cJSON_AddObjectToObject(this_obj, "type_details");
            cJSON_AddNumberToObject(type_details, "type_id", Objects->Object[i].Type);
            cJSON_AddStringToObject(type_details, "name", item_type_name(Objects->Object[i].Type));
            cJSON_AddNumberToObject(type_details, "level", Objects->Object[i].Flags.Value[0]);

            for(int j = 1; j < 4; j++) {
                if(Objects->Object[i].Flags.Value[j]) {
                    found = 1;
                }
            }
            if(found) {
                cJSON *spells = NULL;

                spells = cJSON_AddArrayToObject(type_details, "spells");
                for(int j = 1; j < 4; j++) {
                    if(Objects->Object[i].Flags.Value[j]) {
                        cJSON *this_spell = NULL;

                        this_spell = cJSON_CreateObject();
                        cJSON_AddNumberToObject(this_spell, "spell_id", Objects->Object[i].Flags.Value[j]);
                        cJSON_AddStringToObject(this_spell, "name", spell_name(Objects->Object[i].Flags.Value[j]));
                        cJSON_AddNullToObject(this_spell, "current_charges");
                        cJSON_AddNullToObject(this_spell, "max_charges");
                        cJSON_AddItemToArray(spells, this_spell);
                    }
                }
            } else {
                cJSON_AddNullToObject(type_details, "spells");
            }
            break;
        case ITEM_WAND:
        case ITEM_STAFF:
            type_details = cJSON_AddObjectToObject(this_obj, "type_details");
            cJSON_AddNumberToObject(type_details, "type_id", Objects->Object[i].Type);
            cJSON_AddStringToObject(type_details, "name", item_type_name(Objects->Object[i].Type));
            cJSON_AddNumberToObject(type_details, "level", Objects->Object[i].Flags.Value[0]);
            {
                cJSON *spells = NULL;
                spells = cJSON_AddArrayToObject(type_details, "spells");
                // [2] of [1] charges of spell [3]
                {
                    cJSON *this_spell = NULL;
                    this_spell = cJSON_CreateObject();
                    cJSON_AddNumberToObject(this_spell, "spell_id", Objects->Object[i].Flags.Value[2]);
                    cJSON_AddStringToObject(this_spell, "name", spell_name(Objects->Object[i].Flags.Value[2]));
                    cJSON_AddNumberToObject(this_spell, "current_charges", Objects->Object[i].Flags.Value[1]);
                    cJSON_AddNumberToObject(this_spell, "max_charges", Objects->Object[i].Flags.Value[0]);
                    cJSON_AddItemToArray(spells, this_spell);
                }
            }
            break;
        case ITEM_WEAPON:
        case ITEM_FIREWEAPON:
        case ITEM_MISSILE:
            type_details = cJSON_AddObjectToObject(this_obj, "type_details");
            cJSON_AddNumberToObject(type_details, "type_id", Objects->Object[i].Type);
            cJSON_AddStringToObject(type_details, "name", item_type_name(Objects->Object[i].Type));
            // [1]d[2] damage of type [3]
            cJSON_AddRawDiceToObject(type_details, "damage",
                    Objects->Object[i].Flags.Value[1],
                    Objects->Object[i].Flags.Value[2], 0);
            cJSON_AddNumberToObject(type_details, "damage_type_id", Objects->Object[i].Flags.Value[3]);
            cJSON_AddStringToObject(type_details, "damage_type", damage_name(Objects->Object[i].Flags.Value[3]));
            break;
        case ITEM_ARMOR:
            type_details = cJSON_AddObjectToObject(this_obj, "type_details");
            cJSON_AddNumberToObject(type_details, "type_id", Objects->Object[i].Type);
            cJSON_AddStringToObject(type_details, "name", item_type_name(Objects->Object[i].Type));
            // value [0], armor class [1]
            cJSON_AddNumberToObject(type_details, "gold", Objects->Object[i].Flags.Value[0] / 10.0);
            cJSON_AddNumberToObject(type_details, "armor_class", Objects->Object[i].Flags.Value[1] / 10.0);
            break;
        case ITEM_TRAP:
            type_details = cJSON_AddObjectToObject(this_obj, "type_details");
            cJSON_AddNumberToObject(type_details, "type_id", Objects->Object[i].Type);
            cJSON_AddStringToObject(type_details, "name", item_type_name(Objects->Object[i].Type));
            // [3] charges, attack type [1], damage [2]
            break;
        case ITEM_CONTAINER:
            type_details = cJSON_AddObjectToObject(this_obj, "type_details");
            cJSON_AddNumberToObject(type_details, "type_id", Objects->Object[i].Type);
            cJSON_AddStringToObject(type_details, "name", item_type_name(Objects->Object[i].Type));
            break;
        case ITEM_DRINKCON:
            type_details = cJSON_AddObjectToObject(this_obj, "type_details");
            cJSON_AddNumberToObject(type_details, "type_id", Objects->Object[i].Type);
            cJSON_AddStringToObject(type_details, "name", item_type_name(Objects->Object[i].Type));
            break;
        case ITEM_NOTE:
            type_details = cJSON_AddObjectToObject(this_obj, "type_details");
            cJSON_AddNumberToObject(type_details, "type_id", Objects->Object[i].Type);
            cJSON_AddStringToObject(type_details, "name", item_type_name(Objects->Object[i].Type));
            break;
        case ITEM_KEY:
            type_details = cJSON_AddObjectToObject(this_obj, "type_details");
            cJSON_AddNumberToObject(type_details, "type_id", Objects->Object[i].Type);
            cJSON_AddStringToObject(type_details, "name", item_type_name(Objects->Object[i].Type));
            break;
        case ITEM_FOOD:
            type_details = cJSON_AddObjectToObject(this_obj, "type_details");
            cJSON_AddNumberToObject(type_details, "type_id", Objects->Object[i].Type);
            cJSON_AddStringToObject(type_details, "name", item_type_name(Objects->Object[i].Type));
            break;
        case ITEM_MONEY:
            type_details = cJSON_AddObjectToObject(this_obj, "type_details");
            cJSON_AddNumberToObject(type_details, "type_id", Objects->Object[i].Type);
            cJSON_AddStringToObject(type_details, "name", item_type_name(Objects->Object[i].Type));
            break;
        case ITEM_TRASH:
            type_details = cJSON_AddObjectToObject(this_obj, "type_details");
            cJSON_AddNumberToObject(type_details, "type_id", Objects->Object[i].Type);
            cJSON_AddStringToObject(type_details, "name", item_type_name(Objects->Object[i].Type));
            break;
        case ITEM_PEN:
            type_details = cJSON_AddObjectToObject(this_obj, "type_details");
            cJSON_AddNumberToObject(type_details, "type_id", Objects->Object[i].Type);
            cJSON_AddStringToObject(type_details, "name", item_type_name(Objects->Object[i].Type));
            break;
        case ITEM_BOARD:
            type_details = cJSON_AddObjectToObject(this_obj, "type_details");
            cJSON_AddNumberToObject(type_details, "type_id", Objects->Object[i].Type);
            cJSON_AddStringToObject(type_details, "name", item_type_name(Objects->Object[i].Type));
            break;
        case ITEM_BOAT:
            type_details = cJSON_AddObjectToObject(this_obj, "type_details");
            cJSON_AddNumberToObject(type_details, "type_id", Objects->Object[i].Type);
            cJSON_AddStringToObject(type_details, "name", item_type_name(Objects->Object[i].Type));
            break;
        case ITEM_WORN:
            type_details = cJSON_AddObjectToObject(this_obj, "type_details");
            cJSON_AddNumberToObject(type_details, "type_id", Objects->Object[i].Type);
            cJSON_AddStringToObject(type_details, "name", item_type_name(Objects->Object[i].Type));
            break;
        default:
            cJSON_AddNullToObject(this_obj, "type_details");
            break;
    }

    return this_obj;
}

void dump_as_json(zones *Zones, rooms *Rooms, objects *Objects, mobs *Mobs, shops *Shops, char *outfile)
{
    FILE *ofp = NULL;
    char *output = NULL;
    cJSON *root = NULL;
    cJSON *rooms = NULL;
    cJSON *zones = NULL;
    cJSON *mobs = NULL;
    cJSON *objects = NULL;
    int **reset_checkoffs = NULL;

    root = cJSON_CreateObject();

    // When we process resets for each room, we will check them
    // off as applied.  Any resets NOT found by this are orphaned
    // and may have no result, or otherwise be in error.
    reset_checkoffs = calloc(Zones->Count, sizeof(int *));
    for (int j = 0; j < Zones->Count; j++) {
        reset_checkoffs[j] = calloc(Zones->Zone[j].Count, sizeof(int));
    }

    zones = cJSON_AddObjectToObject(root, "zones");
    for (int i = 0; i < Zones->Count; i++) {
        cJSON *this_zone = NULL;

        if (!Quiet) {
            status(stderr, "Dumping zone #%d (%s)...", Zones->Zone[i].Number, zone_name(Zones, Zones->Zone[i].Number));
            spin(stderr);
        }

        this_zone = cJSON_AddObjectToObject(zones, vnum_to_string(Zones->Zone[i].Number));

        process_zone_header(this_zone, Zones, i);
        //process_zone_resets(this_zone, Rooms, Zones, i, Objects, Mobs);

        rooms = process_rooms_in_zone(this_zone, Zones, i, Rooms, Objects, Mobs, reset_checkoffs);

        mobs = cJSON_AddObjectToObject(this_zone, "mobs");
        for (int j = 0; j < Mobs->Count; j++) {
            cJSON *this_mob = NULL;

            if (!Quiet)
                spin(stderr);

            if(Mobs->Mob[j].Zone != Zones->Zone[i].Number) {
                continue;
            }

            this_mob = cJSON_AddObjectToObject(mobs, vnum_to_string(Mobs->Mob[j].Number));

            process_mob_header(this_mob, Mobs, j);
            process_specials(this_mob, Mobs->Mob[j].Number, SPECIAL_MOB);
            process_mob_zone_info("zone", this_mob, Mobs, j, Zones);
            process_flags(this_mob, Mobs->Mob[j].ActFlags, act_flag_names);
            process_flags(this_mob, Mobs->Mob[j].AffectedBy, aff_flag_names);
            process_mob_abilities(this_mob, Mobs, j);
            process_mob_saving_throws(this_mob, Mobs, j);
            process_mob_sounds(this_mob, Mobs, j);
            process_mob_attacks(this_mob, Mobs, j);
            process_mob_skills(this_mob, Mobs, j);
        }

        objects = cJSON_AddObjectToObject(this_zone, "objects");
        for (int j = 0; j < Objects->Count; j++) {
            cJSON *this_obj = NULL;

            if (!Quiet)
                spin(stderr);

            if(Objects->Object[j].Zone != Zones->Zone[i].Number) {
                continue;
            }

            this_obj = cJSON_AddObjectToObject(objects, vnum_to_string(Objects->Object[j].Number));

            process_obj_header(this_obj, Objects, j);
            process_obj_zone_info("zone", this_obj, Objects, j, Zones);
            process_obj_extra_descriptions(this_obj, Objects, j);
            process_flags(this_obj, Objects->Object[j].Flags.Wear, wear_location_names);
            process_flags(this_obj, Objects->Object[j].Flags.Extra, item_extra_names);
            process_obj_type_info(this_obj, Objects, j);
        }

        if (!Quiet) {
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
    if (!Quiet)
        fprintf(stderr, "done.\n");
}


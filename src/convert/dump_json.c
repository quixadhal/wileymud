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

// Tack on fixed zone header data
cJSON *process_zone_header(cJSON *this_zone, zones *Zones, int i) {
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

// Just tack on the fixed room header data
cJSON *process_room_header(cJSON *this_room, rooms *Rooms, int i) {
    cJSON_AddNumberToObject(this_room, "vnum", Rooms->Room[i].Number);
    cJSON_AddStringToObject(this_room, "name", room_name(Rooms, Rooms->Room[i].Number));
    cJSON_AddStringToObject(this_room, "sector", sector_name(Rooms->Room[i].Sector));
    cJSON_AddNumberToObject(this_room, "move_cost", Rooms->Room[i].MoveCost);
    cJSON_AddStringToObject(this_room, "description", Rooms->Room[i].Description);

    return this_room;
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

cJSON *process_room_zone_info(const char *KeyName, cJSON *this_room, rooms *Rooms, int i, zones *Zones) {
    cJSON *zone = NULL;

    zone = cJSON_AddObjectToObject(this_room, KeyName);
    cJSON_AddNumberToObject(zone, "vnum", Rooms->Room[i].Zone);
    cJSON_AddStringToObject(zone, "name", zone_name(Zones, Rooms->Room[i].Zone));

    return zone;
}

cJSON *process_room_teleport(cJSON *this_room, rooms *Rooms, int i, zones *Zones) {
    cJSON *teleport = NULL;

    if (Rooms->Room[i].Sector == SECT_TELEPORT) {
        int target_room = -1;

        target_room = remap_room_vnum(Rooms, Rooms->Room[i].TeleportTo);
        teleport = cJSON_AddObjectToObject(this_room, "teleport");
        cJSON_AddStringToObject(teleport, "sector_type", sector_name(Rooms->Room[i].TeleportSector));
        cJSON_AddNumberToObject(teleport, "time", Rooms->Room[i].TeleportTime / 10);
        cJSON_AddNumberToObject(teleport, "vnum", Rooms->Room[i].TeleportTo);
        cJSON_AddStringToObject(teleport, "name", room_name(Rooms, Rooms->Room[i].TeleportTo));
        if(Rooms->Room[i].TeleportLook) {
            cJSON_AddTrueToObject(teleport, "auto_look");
        } else {
            cJSON_AddFalseToObject(teleport, "auto_look");
        }
        if(Rooms->Room[target_room].Zone != Rooms->Room[i].Zone) {
            process_room_zone_info("external_zone", teleport, Rooms, target_room , Zones);
        } else {
            cJSON_AddNullToObject(teleport, "external_zone");
        }
    } else {
        cJSON_AddNullToObject(this_room, "teleport");
    }

    return teleport;
}

cJSON *process_room_river(cJSON *this_room, rooms *Rooms, int i, zones *Zones) {
    cJSON *river = NULL;

    if (Rooms->Room[i].Sector == SECT_WATER_NOSWIM) {
        int target_room = -1;

        for(int j = 0; j < Rooms->Room[i].ExitCount; j++) {
            if(Rooms->Room[i].Exit[j].Direction == Rooms->Room[i].RiverDirection) {
                target_room = remap_room_vnum(Rooms, Rooms->Room[i].Exit[j].Room);
            }
        }

        river = cJSON_AddObjectToObject(this_room, "river");
        cJSON_AddNumberToObject(river, "time", Rooms->Room[i].RiverSpeed / 10);
        cJSON_AddStringToObject(river, "direction", exit_name(Rooms->Room[i].RiverDirection));
        if(target_room != -1) {
            cJSON *target = NULL;

            target = cJSON_AddObjectToObject(river, "target");
            cJSON_AddNumberToObject(target, "vnum", Rooms->Room[target_room].Number);
            cJSON_AddStringToObject(target, "name", room_name(Rooms, Rooms->Room[target_room].Number));
            if(Rooms->Room[target_room].Zone != Rooms->Room[i].Zone) {
                process_room_zone_info("external_zone", target, Rooms, target_room , Zones);
            } else {
                cJSON_AddNullToObject(target, "external_zone");
            }
        }
    } else {
        cJSON_AddNullToObject(this_room, "river");
    }

    return river;
}

cJSON *process_room_sounds(cJSON *this_room, rooms *Rooms, int i) {
    cJSON *sound = NULL;

    if (Rooms->Room[i].Flags & ROOM_SOUND) {
        sound = cJSON_AddObjectToObject(this_room, "sound");
        cJSON_AddStringToObject(sound, "in_room", Rooms->Room[i].SoundText[0]);
        cJSON_AddStringToObject(sound, "adjacent", Rooms->Room[i].SoundText[1]);
    } else {
        cJSON_AddNullToObject(this_room, "sound");
    }

    return sound;
}

cJSON *process_room_extra_descriptions(cJSON *this_room, rooms *Rooms, int i) {
    cJSON *extra = NULL;

    if (Rooms->Room[i].ExtraCount) {

        extra = cJSON_AddArrayToObject(this_room, "extra_descriptions");
        for (int j = 0; j < Rooms->Room[i].ExtraCount; j++) {
            cJSON *this_extra = NULL;
            cJSON *keywords = NULL;

            this_extra = cJSON_CreateObject();
            if(Rooms->Room[i].Extra[j].Keyword->Count > 0) {
                keywords = cJSON_CreateStringArray((const char *const *) Rooms->Room[i].Extra[j].Keyword->Word, Rooms->Room[i].Extra[j].Keyword->Count);
                cJSON_AddItemToObject(this_extra, "keywords", keywords);
            } else {
                cJSON_AddNullToObject(this_extra, "keywords");
            }
            cJSON_AddStringToObject(this_extra, "description", Rooms->Room[i].Extra[j].Description);
            cJSON_AddItemToArray(extra, this_extra);
        }
    } else {
        cJSON_AddNullToObject(this_room, "extra_descriptions");
    }

    return extra;
}

cJSON *process_room_exits(cJSON *this_room, rooms *Rooms, int i, zones *Zones, objects *Objects) {
    cJSON *exits = NULL;

    if (Rooms->Room[i].ExitCount) {

        exits = cJSON_AddObjectToObject(this_room, "exits");
        for (int j = 0; j < Rooms->Room[i].ExitCount; j++) {
            cJSON *this_exit = NULL;
            cJSON *keywords = NULL;

            this_exit = cJSON_AddObjectToObject(exits, exit_name(Rooms->Room[i].Exit[j].Direction));
            cJSON_AddStringToObject(this_exit, "name", exit_name(Rooms->Room[i].Exit[j].Direction));
            cJSON_AddStringToObject(this_exit, "description", Rooms->Room[i].Exit[j].Description);
                // Not sure if these should be inside or if they have value for
                // description only exits.
                cJSON_AddStringToObject(this_exit, "type", exittype_name(Rooms->Room[i].Exit[j].Type));
                if(Rooms->Room[i].Exit[j].Keyword->Count > 0) {
                    keywords = cJSON_CreateStringArray((const char *const *)Rooms->Room[i].Exit[j].Keyword->Word, Rooms->Room[i].Exit[j].Keyword->Count);
                    cJSON_AddItemToObject(this_exit, "keywords", keywords);
                } else {
                    cJSON_AddNullToObject(this_exit, "keywords");
                }

            if(Rooms->Room[i].Exit[j].Error != EXIT_DESCRIPTION_ONLY) {
                cJSON *target = NULL;
                cJSON *reverse_target = NULL;
                int target_room = -1;

                if(Rooms->Room[i].Exit[j].KeyNumber > 0) {
                    cJSON_AddStringToObject(this_exit, "key", obj_name(Objects, Rooms->Room[i].Exit[j].KeyNumber));
                } else {
                    cJSON_AddNullToObject(this_exit, "key");
                }

                target = cJSON_AddObjectToObject(this_exit, "target");
                cJSON_AddNumberToObject(target, "vnum", Rooms->Room[i].Exit[j].Room);
                cJSON_AddStringToObject(target, "name", room_name(Rooms, Rooms->Room[i].Exit[j].Room));

                // If this exit goes to a differnet zone, we want to know which one!
                target_room = remap_room_vnum(Rooms, Rooms->Room[i].Exit[j].Room);
                if(Rooms->Room[target_room].Zone != Rooms->Room[i].Zone) {
                    process_room_zone_info("external_zone", target, Rooms, target_room , Zones);
                } else {
                    cJSON_AddNullToObject(this_exit, "external_zone");
                }

                if(Rooms->Room[i].Exit[j].Error == EXIT_ONE_WAY || Rooms->Room[i].Exit[j].Error == EXIT_NON_EUCLIDEAN) {
                    cJSON_AddTrueToObject(this_exit, "one_way");
                } else {
                    cJSON_AddFalseToObject(this_exit, "one_way");
                }
                if(Rooms->Room[i].Exit[j].Error == EXIT_NON_EUCLIDEAN) {
                    EXIT *reverse_exit = NULL;

                    cJSON_AddTrueToObject(this_exit, "non_euclidean");
                    reverse_exit = get_reverse_exit(&(Rooms->Room[i].Exit[j]), Rooms);
                    if(reverse_exit) {
                        reverse_target = cJSON_AddObjectToObject(this_exit, "reverse_target");
                        cJSON_AddNumberToObject(reverse_target, "vnum", reverse_exit->Room);
                        cJSON_AddStringToObject(reverse_target, "name", room_name(Rooms, reverse_exit->Room));
                        cJSON_AddStringToObject(reverse_target, "direction", exit_name(reverse_exit->Direction));
                    }
                } else {
                    cJSON_AddFalseToObject(this_exit, "non_euclidean");
                    cJSON_AddNullToObject(this_exit, "reverse_target");
                }
            }
        }
    } else {
        cJSON_AddNullToObject(this_room, "exits");
    }

    return exits;
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
cJSON *process_reset_segment(int *LastMob, int *LastLoc, int *LastObj, int *LeaderMob, int i, int j, int k, rooms *Rooms, zones *Zones, objects *Objects, mobs *Mobs, int **reset_checkoffs) {
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


// reset_checkoffs is an output parameter, to be modified in here.
// You must allocate it outside here and pass in handles to it.
int find_room_resets(rooms *Rooms, int i, zones *Zones, int **reset_checkoffs) {
    int FoundResets = 0;
    int LastMob = -1;
    int LastLoc = -1;
    int LastObj = -1;
    int LeaderMob = -1;

    for (int j = 0; j < Zones->Count; j++) {
        if(Zones->Zone[j].Number != Rooms->Room[i].Zone) {
            // Not our zone...
            //continue;
        }
        LastMob = -1;
        LastLoc = -1;
        LastObj = -1;
        LeaderMob = -1;
        for (int k = 0; k < Zones->Zone[j].Count; k++) {
            switch (Zones->Zone[j].Cmds[k].Command) {
                case ZONE_CMD_MOBILE:
                    if(Zones->Zone[j].Cmds[k].Arg[ZONE_ROOM] != Rooms->Room[i].Number)
                        break;
                    FoundResets++;
                    LastMob = Zones->Zone[j].Cmds[k].Arg[ZONE_MOBILE];
                    LastLoc = Zones->Zone[j].Cmds[k].Arg[ZONE_ROOM];
                    LeaderMob = LastMob;
                    reset_checkoffs[j][k] = 1;
                    break;
                case ZONE_CMD_OBJECT:
                    if(Zones->Zone[j].Cmds[k].Arg[ZONE_ROOM] != Rooms->Room[i].Number)
                        break;
                    FoundResets++;
                    LastObj = Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT];
                    LastLoc = Zones->Zone[j].Cmds[k].Arg[ZONE_ROOM];
                    reset_checkoffs[j][k] = 1;
                    break;
                case ZONE_CMD_GIVE:
                    // This has to use LastMob
                    if(LastMob < 0)
                        break;
                    FoundResets++;
                    LastObj = Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT];
                    reset_checkoffs[j][k] = 1;
                    break;
                case ZONE_CMD_EQUIP:
                    // This has to use LastMob
                    if(LastMob < 0)
                        break;
                    FoundResets++;
                    LastObj = Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT];
                    reset_checkoffs[j][k] = 1;
                    break;
                case ZONE_CMD_PUT:
                    // We'll check LastLoc and LastObj to make sure
                    // an object was loaded into THIS room.
                    if(LastObj < 0)
                        break;
                    FoundResets++;
                    reset_checkoffs[j][k] = 1;
                    break;
                case ZONE_CMD_DOOR:
                    if(Zones->Zone[j].Cmds[k].Arg[ZONE_DOOR_ROOM] != Rooms->Room[i].Number)
                        break;
                    FoundResets++;
                    reset_checkoffs[j][k] = 1;
                    break;
                case ZONE_CMD_REMOVE:
                    if(Zones->Zone[j].Cmds[k].Arg[ZONE_REMOVE_ROOM] != Rooms->Room[i].Number)
                        break;
                    FoundResets++;
                    reset_checkoffs[j][k] = 1;
                    break;
                case ZONE_CMD_LEAD:
                    // This has to use LeaderMob
                    if(LeaderMob < 0)
                        break;
                    FoundResets++;
                    LastMob = Zones->Zone[j].Cmds[k].Arg[ZONE_MOBILE];
                    reset_checkoffs[j][k] = 1;
                    break;
                case ZONE_CMD_HATE:
                    // This has to use LastMob
                    if(LastMob < 0)
                        break;
                    FoundResets++;
                    reset_checkoffs[j][k] = 1;
                    break;
                default:
                    break;
            }
        }
    }

    return FoundResets;
}

cJSON *process_room_resets(cJSON *this_room, rooms *Rooms, int i, zones *Zones, objects *Objects, mobs *Mobs, int FoundResets) {
    cJSON *resets = NULL;
    int LastMob = -1;
    int LastLoc = -1;
    int LastObj = -1;
    int LeaderMob = -1;

    if(FoundResets > 0) {
        // Look for resets that apply to this room...
        resets = cJSON_AddArrayToObject(this_room, "resets");

        for (int j = 0; j < Zones->Count; j++) {
            if(Zones->Zone[j].Number != Rooms->Room[i].Zone) {
                // Not our zone...
                //continue;
                // There are several data errors where a reset is
                // in the wrong zone, but it works in the actual MUD.
            }
            LastMob = -1;
            LastLoc = -1;
            LastObj = -1;
            LeaderMob = -1;
            for (int k = 0; k < Zones->Zone[j].Count; k++) {
                cJSON *this_reset = NULL;

                this_reset = process_reset_segment(&LastMob, &LastLoc, &LastObj, &LeaderMob, i, j, k, Rooms, Zones, Objects, Mobs, NULL);
                if(this_reset) {
                    cJSON_AddItemToArray(resets, this_reset);
                }
            }
        }
    } else {
        cJSON_AddNullToObject(this_room, "resets");
    }

    return resets;
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
    cJSON_AddNumberToObject(this_mob, "vnum", Mobs->Mob[i].Number);
    cJSON_AddStringToObject(this_mob, "name", mob_name(Mobs, Mobs->Mob[i].Number));
    cJSON_AddStringToObject(this_mob, "short_description", Mobs->Mob[i].ShortDesc);
    cJSON_AddStringToObject(this_mob, "long_description", Mobs->Mob[i].LongDesc);
    cJSON_AddStringToObject(this_mob, "description", Mobs->Mob[i].Description);

    return this_mob;
}

cJSON *process_zone_mob_info(cJSON *this_mob, mobs *Mobs, int i, zones *Zones)
{
    cJSON *zone = NULL;

    zone = cJSON_AddObjectToObject(this_mob, "zone");
    cJSON_AddNumberToObject(zone, "vnum", Mobs->Mob[i].Zone);
    cJSON_AddStringToObject(zone, "name", zone_name(Zones, Mobs->Mob[i].Zone));

    return zone;
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
        char vnum[32];

        if (!Quiet) {
            status(stderr, "Dumping zone #%d (%s)...", Zones->Zone[i].Number, zone_name(Zones, Zones->Zone[i].Number));
            spin(stderr);
        }
        sprintf(vnum, "%d", Zones->Zone[i].Number);
        this_zone = cJSON_AddObjectToObject(zones, vnum);

        process_zone_header(this_zone, Zones, i);
        //process_zone_resets(this_zone, Rooms, Zones, i, Objects, Mobs);

        rooms = cJSON_AddObjectToObject(this_zone, "rooms");
        for (int j = 0; j < Rooms->Count; j++) {
            cJSON *this_room = NULL;
            char vnum[32];
            int FoundResets = 0;

            if (!Quiet)
                spin(stderr);

            if(Rooms->Room[j].Zone != Zones->Zone[i].Number) {
                continue;
            }

            sprintf(vnum, "%d", Rooms->Room[j].Number);
            this_room = cJSON_AddObjectToObject(rooms, vnum);

            process_room_header(this_room, Rooms, j);
            process_specials(this_room, Rooms->Room[j].Number, SPECIAL_ROOM);
            process_room_zone_info("zone", this_room, Rooms, j, Zones);
            process_flags(this_room, Rooms->Room[j].Flags, room_flag_names);
            process_room_teleport(this_room, Rooms, j, Zones);
            process_room_river(this_room, Rooms, j, Zones);
            process_room_sounds(this_room, Rooms, j);
            process_room_extra_descriptions(this_room, Rooms, j);
            process_room_exits(this_room, Rooms, j, Zones, Objects);

            FoundResets = find_room_resets(Rooms, j, Zones, reset_checkoffs);
            process_room_resets(this_room, Rooms, j, Zones, Objects, Mobs, FoundResets);
        }

        mobs = cJSON_AddObjectToObject(this_zone, "mobs");
        for (int j = 0; j < Mobs->Count; j++) {
            cJSON *this_mob = NULL;
            char vnum[32];

            if (!Quiet)
                spin(stderr);

            if(Mobs->Mob[j].Zone != Zones->Zone[i].Number) {
                continue;
            }

            sprintf(vnum, "%d", Mobs->Mob[j].Number);
            this_mob = cJSON_AddObjectToObject(mobs, vnum);

            process_mob_header(this_mob, Mobs, j);
            process_specials(this_mob, Mobs->Mob[j].Number, SPECIAL_MOB);
            process_zone_mob_info(this_mob, Mobs, j, Zones);
        }

        objects = cJSON_AddObjectToObject(this_zone, "objects");
        for (int j = 0; j < Objects->Count; j++) {
            cJSON *this_object = NULL;
            char vnum[32];

            if (!Quiet)
                spin(stderr);

            if(Objects->Object[j].Zone != Zones->Zone[j].Number) {
                continue;
            }

            sprintf(vnum, "%d", Objects->Object[j].Number);
            this_object = cJSON_AddObjectToObject(objects, vnum);
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


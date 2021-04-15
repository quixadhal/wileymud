#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <cjson/cJSON.h>
#include "include/structs.h"
#include "include/main.h"
#include "include/utils.h"
#include "include/parse_wiley.h"

#define _JSON_ROOM_C
#include "include/dump_json.h"
#include "include/json_room.h"
#include "include/json_mob.h"
#include "include/json_obj.h"

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

// Just tack on the fixed room header data
cJSON *process_room_header(cJSON *this_room, rooms *Rooms, int i)
{
    cJSON_AddNumberToObject(this_room, "vnum", Rooms->Room[i].Number);
    cJSON_AddStringToObject(this_room, "name", room_name(Rooms, Rooms->Room[i].Number));
    cJSON_AddStringToObject(this_room, "sector", sector_name(Rooms->Room[i].Sector));
    cJSON_AddNumberToObject(this_room, "move_cost", Rooms->Room[i].MoveCost);
    cJSON_AddStringToObject(this_room, "description", Rooms->Room[i].Description);

    return this_room;
}

cJSON *process_room_zone_info(const char *KeyName, cJSON *this_room, rooms *Rooms, int i, zones *Zones)
{
    cJSON *zone = NULL;

    zone = cJSON_AddObjectToObject(this_room, KeyName);
    cJSON_AddNumberToObject(zone, "vnum", Rooms->Room[i].Zone);
    cJSON_AddStringToObject(zone, "name", zone_name(Zones, Rooms->Room[i].Zone));

    return zone;
}

cJSON *process_room_teleport(cJSON *this_room, rooms *Rooms, int i, zones *Zones)
{
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

cJSON *process_room_river(cJSON *this_room, rooms *Rooms, int i, zones *Zones)
{
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

cJSON *process_room_sounds(cJSON *this_room, rooms *Rooms, int i)
{
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

cJSON *process_room_extra_descriptions(cJSON *this_room, rooms *Rooms, int i)
{
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

cJSON *process_room_exits(cJSON *this_room, rooms *Rooms, int i, zones *Zones, objects *Objects)
{
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

// reset_checkoffs is an output parameter, to be modified in here.
// You must allocate it outside here and pass in handles to it.
int find_room_resets(rooms *Rooms, int i, zones *Zones, int **reset_checkoffs)
{
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

cJSON *process_room_resets(cJSON *this_room, rooms *Rooms, int i, zones *Zones, objects *Objects, mobs *Mobs, int FoundResets)
{
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

cJSON *process_room(cJSON *parent_node, zones *Zones, int j, rooms *Rooms, objects *Objects, mobs *Mobs, int **reset_checkoffs)
{
    cJSON *this_room = NULL;
    int FoundResets = 0;

    this_room = cJSON_AddObjectToObject(parent_node, vnum_to_string(Rooms->Room[j].Number));

    process_room_header(this_room, Rooms, j);
    process_specials(this_room, Rooms->Room[j].Number, SPECIAL_ROOM);
    process_room_zone_info("zone", this_room, Rooms, j, Zones);
    process_flags(this_room, "flags", Rooms->Room[j].Flags, room_flag_names);
    process_room_teleport(this_room, Rooms, j, Zones);
    process_room_river(this_room, Rooms, j, Zones);
    process_room_sounds(this_room, Rooms, j);
    process_room_extra_descriptions(this_room, Rooms, j);
    process_room_exits(this_room, Rooms, j, Zones, Objects);

    FoundResets = find_room_resets(Rooms, j, Zones, reset_checkoffs);
    process_room_resets(this_room, Rooms, j, Zones, Objects, Mobs, FoundResets);

    return this_room;
}

cJSON *process_rooms_in_zone(cJSON *this_zone, zones *Zones, int i, rooms *Rooms, objects *Objects, mobs *Mobs, int **reset_checkoffs)
{
    cJSON *rooms = NULL;

    rooms = cJSON_AddObjectToObject(this_zone, "rooms");
    for (int j = 0; j < Rooms->Count; j++) {
        if (!Quiet)
            spin(stderr);
        if(Rooms->Room[j].Zone != Zones->Zone[i].Number) {
            continue;
        }
        process_room(rooms, Zones, j, Rooms, Objects, Mobs, reset_checkoffs);
    }
    return rooms;
}

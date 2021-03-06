#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <cjson/cJSON.h>
#include "include/structs.h"
#include "include/main.h"
#include "include/utils.h"
#include "include/parse_wiley.h"

#define _JSON_C
#include "include/dump_json.h"

#define IS_SET(flag,bit) ((flag) & (bit))

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

void json_sprintbit(FILE *fp, unsigned long vektor, const char *names[], char *prefix, char *postfix, char *indent)
{
    int LastBit = 0;

    for (int i = 0; i < sizeof(long) * 8; i++) {
        if(names[i] && names[i][0])
            LastBit = i;
    }

    fprintf(fp, "%s", prefix); // "flags : {\r\n"
    for (int i = 0; i < sizeof(long) * 8; i++) {
        if(names[i] && names[i][0]) {
            fprintf(fp, "%s\"%s\" : %d%s\r\n",
                    indent, names[i],
                    IS_SET((1 << i), vektor) ? 1 : 0,
                    (i == LastBit) ? "" : ","
                    );
        }
    }
    fprintf(fp, "%s", postfix); // "}\r\n"
}

void dump_as_json(zones *Zones, rooms *Rooms, objects *Objects, mobs *Mobs, shops *Shops, char *outfile)
{
    FILE *ofp;
    char *output;
    cJSON *root, *rooms; //, *mobs, *objects;

    root = cJSON_CreateObject();
    rooms = cJSON_AddObjectToObject(root, "rooms");

    for (int i = 0; i < Rooms->Count; i++) {
        cJSON *this_room, *flags, *zone, *resets;
        char vnum[32];
        int special_proc = -1;
	int LastMob = -1;
        int LastLoc = -1;
        int LastObj = -1;
        int LeaderMob = -1;
        int FoundResets = 0;

        if (!Quiet)
            spin(stderr);
        sprintf(vnum, "%d", Rooms->Room[i].Number);
        this_room = cJSON_AddObjectToObject(rooms, vnum);

        cJSON_AddNumberToObject(this_room, "vnum", Rooms->Room[i].Number);
        cJSON_AddStringToObject(this_room, "name", room_name(Rooms, Rooms->Room[i].Number));
        cJSON_AddStringToObject(this_room, "sector", sector_name(Rooms->Room[i].Sector));
        cJSON_AddNumberToObject(this_room, "move_cost", Rooms->Room[i].MoveCost);
        cJSON_AddStringToObject(this_room, "description", Rooms->Room[i].Description);

        for(int j = 0; j < SpecialCount; j++) {
            if(Specials[j].ProgType == SPECIAL_ROOM) {
                if(Specials[j].Number == Rooms->Room[i].Number) {
                    special_proc = j;
                    break;
                }
            }
        }
        if(special_proc > -1) {
            cJSON *special;

            special = cJSON_AddObjectToObject(this_room, "special_procedure");
            cJSON_AddNumberToObject(special, "vnum", Specials[special_proc].Number);
            cJSON_AddStringToObject(special, "function", Specials[special_proc].FunctionName);
            cJSON_AddStringToObject(special, "description", Specials[special_proc].Description);
            special_proc = -1;
        }

        flags = cJSON_AddObjectToObject(this_room, "flags");
        for (int j = 0; j < sizeof(long) * 8; j++) {
            if(room_flag_names[j] && room_flag_names[j][0]) {
                if(IS_SET((1 << j), Rooms->Room[i].Flags)) {
                    cJSON_AddTrueToObject(flags, room_flag_names[j]);
                } else {
                    cJSON_AddFalseToObject(flags, room_flag_names[j]);
                }
            }
        }
        zone = cJSON_AddObjectToObject(this_room, "zone");
        cJSON_AddNumberToObject(zone, "vnum", Rooms->Room[i].Zone);
        cJSON_AddStringToObject(zone, "name", zone_name(Zones, Rooms->Room[i].Zone));

	if (Rooms->Room[i].Sector == SECT_TELEPORT) {
            cJSON *teleport;

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
        } else {
            //cJSON_AddNullToObject(this_room, "teleport");
        }

	if (Rooms->Room[i].Sector == SECT_WATER_NOSWIM) {
            cJSON *river;

            river = cJSON_AddObjectToObject(this_room, "river");
            cJSON_AddNumberToObject(river, "time", Rooms->Room[i].RiverSpeed / 10);
            cJSON_AddStringToObject(river, "direction", exit_name(Rooms->Room[i].RiverDirection));
        } else {
            //cJSON_AddNullToObject(this_room, "river");
        }

	if (Rooms->Room[i].Flags & ROOM_SOUND) {
            cJSON *sound;

            sound = cJSON_AddObjectToObject(this_room, "sound");
            cJSON_AddStringToObject(sound, "in_room", Rooms->Room[i].SoundText[0]);
            cJSON_AddStringToObject(sound, "adjacent", Rooms->Room[i].SoundText[1]);
        } else {
            //cJSON_AddNullToObject(this_room, "sound");
        }

	if (Rooms->Room[i].ExtraCount) {
            cJSON *extra;

            extra = cJSON_AddArrayToObject(this_room, "extra_descriptions");
            for (int j = 0; j < Rooms->Room[i].ExtraCount; j++) {
                cJSON *this_extra, *keywords;

                this_extra = cJSON_CreateObject();
                if(Rooms->Room[i].Extra[j].Keyword->Count > 0) {
                    keywords = cJSON_CreateStringArray((const char *const *) Rooms->Room[i].Extra[j].Keyword->Word, Rooms->Room[i].Extra[j].Keyword->Count);
                    cJSON_AddItemToObject(this_extra, "keywords", keywords);
                } else {
                    //cJSON_AddNullToObject(this_extra, "keywords");
                }
                cJSON_AddStringToObject(this_extra, "description", Rooms->Room[i].Extra[j].Description);
                cJSON_AddItemToArray(extra, this_extra);
            }
        } else {
            //cJSON_AddNullToObject(this_room, "extra_descriptions");
        }

        if (Rooms->Room[i].ExitCount) {
            cJSON *exits;

            exits = cJSON_AddObjectToObject(this_room, "exits");
            for (int j = 0; j < Rooms->Room[i].ExitCount; j++) {
                cJSON *this_exit, *keywords;

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
                        //cJSON_AddNullToObject(this_exit, "keywords");
                    }

                if(Rooms->Room[i].Exit[j].Error != EXIT_DESCRIPTION_ONLY) {
                    cJSON *target, *reverse_target;

                    if(Rooms->Room[i].Exit[j].KeyNumber > 0) {
                        cJSON_AddStringToObject(this_exit, "key", obj_name(Objects, Rooms->Room[i].Exit[j].KeyNumber));
                    } else {
                        //cJSON_AddNullToObject(this_exit, "key");
                    }

                    target = cJSON_AddObjectToObject(this_exit, "target");
                    cJSON_AddNumberToObject(target, "vnum", Rooms->Room[i].Exit[j].Room);
                    cJSON_AddStringToObject(target, "name", room_name(Rooms, Rooms->Room[i].Exit[j].Room));

                    if(Rooms->Room[i].Exit[j].Error == EXIT_ONE_WAY || Rooms->Room[i].Exit[j].Error == EXIT_NON_EUCLIDEAN) {
                        cJSON_AddTrueToObject(this_exit, "one_way");
                    } else {
                        cJSON_AddFalseToObject(this_exit, "one_way");
                    }
                    if(Rooms->Room[i].Exit[j].Error == EXIT_NON_EUCLIDEAN) {
			EXIT *foreign;

                        cJSON_AddTrueToObject(this_exit, "non_euclidean");
                        reverse_target = cJSON_AddObjectToObject(this_exit, "reverse_target");
			foreign = &Rooms->Room[Rooms->Room[i].Exit[j].RoomIndex].Exit[RevDir[Rooms->Room[i].Exit[j].Direction]];
                        cJSON_AddNumberToObject(reverse_target, "vnum", foreign->Room);
                        cJSON_AddStringToObject(reverse_target, "name", room_name(Rooms, foreign->Room));
                        cJSON_AddStringToObject(reverse_target, "direction", exit_name(foreign->Direction));
                    } else {
                        cJSON_AddFalseToObject(this_exit, "non_euclidean");
                        //cJSON_AddNullToObject(this_exit, "reverse_target");
                    }
                }
            }
        } else {
            //cJSON_AddNullToObject(this_room, "exits");
        }

        FoundResets = 0;
        for (int j = 0; j < Zones->Count; j++) {
            if(Zones->Zone[j].Number != Rooms->Room[i].Zone) {
                // Not our zone...
                continue;
            }
            for (int k = 0; k < Zones->Zone[j].Count; k++) {
                switch (Zones->Zone[j].Cmds[k].Command) {
                    case ZONE_CMD_MOBILE:
                        if(Zones->Zone[j].Cmds[k].Arg[ZONE_ROOM] != Rooms->Room[i].Number)
                            break;
                        FoundResets++;
                        break;
                    case ZONE_CMD_OBJECT:
                        if(Zones->Zone[j].Cmds[k].Arg[ZONE_ROOM] != Rooms->Room[i].Number)
                            break;
                        FoundResets++;
                        break;
                    case ZONE_CMD_GIVE:
                        // This has to use LastMob
                        if(LastMob < 0 || LastLoc < 0)
                            break;
                        FoundResets++;
                        break;
                    case ZONE_CMD_EQUIP:
                        // This has to use LastMob
                        if(LastMob < 0 || LastLoc < 0)
                            break;
                        FoundResets++;
                        break;
                    case ZONE_CMD_PUT:
                        // We'll check LastLoc and LastObj to make sure
                        // an object was loaded into THIS room.
                        if(LastObj < 0 || LastLoc < 0)
                            break;
                        FoundResets++;
                        break;
                    case ZONE_CMD_DOOR:
                        if(Zones->Zone[j].Cmds[k].Arg[ZONE_DOOR_ROOM] != Rooms->Room[i].Number)
                            break;
                        FoundResets++;
                        break;
                    case ZONE_CMD_REMOVE:
                        if(Zones->Zone[j].Cmds[k].Arg[ZONE_REMOVE_ROOM] != Rooms->Room[i].Number)
                            break;
                        FoundResets++;
                        break;
                    case ZONE_CMD_LEAD:
                        // This has to use LeaderMob
                        if(LeaderMob < 0 || LastLoc < 0)
                            break;
                        FoundResets++;
                        break;
                    case ZONE_CMD_HATE:
                        // This has to use LastMob
                        if(LastMob < 0 || LastLoc < 0)
                            break;
                        FoundResets++;
                        break;
                    default:
                        break;
                }
            }
        }

        if(FoundResets > 0) {
            // Look for resets that apply to this room...
            resets = cJSON_AddArrayToObject(this_room, "resets");

            for (int j = 0; j < Zones->Count; j++) {
                if(Zones->Zone[j].Number != Rooms->Room[i].Zone) {
                    // Not our zone...
                    continue;
                }
                for (int k = 0; k < Zones->Zone[j].Count; k++) {
                    cJSON *this_reset;
                    cJSON *cmd_room, *cmd_mob, *cmd_obj, *cmd_target;
                    char cmd[2];

                    // M - load mob to room
                    // O - load obj to room
                    // G - give obj to previous mob
                    // D - set door state in room
                    // R - remove obj from room
                    // P - put obj from room
                    // L - load mob to be led by previous mob
                    // H - set hated flags
                    // IfFlag - only do if previous command worked
                    switch (Zones->Zone[j].Cmds[k].Command) {
                        case ZONE_CMD_MOBILE:
                            if(Zones->Zone[j].Cmds[k].Arg[ZONE_ROOM] != Rooms->Room[i].Number)
                                break;
                            LastMob = Zones->Zone[j].Cmds[k].Arg[ZONE_MOBILE];
                            LastLoc = Zones->Zone[j].Cmds[k].Arg[ZONE_ROOM];
                            LeaderMob = LastMob;
                            this_reset  = cJSON_CreateObject();
                            sprintf(cmd, "%c", Zones->Zone[j].Cmds[k].Command);
                            cJSON_AddStringToObject(this_reset, "command", "load mobile to room");
                            cmd_mob = cJSON_AddObjectToObject(this_reset, "mob");
                            cJSON_AddNumberToObject(cmd_mob, "vnum", Zones->Zone[j].Cmds[k].Arg[ZONE_MOBILE]);
                            cJSON_AddStringToObject(cmd_mob, "name", mob_name(Mobs, Zones->Zone[j].Cmds[k].Arg[ZONE_MOBILE]));
                            cmd_room = cJSON_AddObjectToObject(this_reset, "room");
                            cJSON_AddNumberToObject(cmd_room, "vnum", Zones->Zone[j].Cmds[k].Arg[ZONE_ROOM]);
                            cJSON_AddStringToObject(cmd_room, "name", room_name(Rooms, Zones->Zone[j].Cmds[k].Arg[ZONE_ROOM]));
                            cJSON_AddItemToArray(resets, this_reset);
                            break;

                        case ZONE_CMD_OBJECT:
                            if(Zones->Zone[j].Cmds[k].Arg[ZONE_ROOM] != Rooms->Room[i].Number)
                                break;
                            LastObj = Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT];
                            LastLoc = Zones->Zone[j].Cmds[k].Arg[ZONE_ROOM];
                            this_reset  = cJSON_CreateObject();
                            sprintf(cmd, "%c", Zones->Zone[j].Cmds[k].Command);
                            cJSON_AddStringToObject(this_reset, "command", "load object to room");
                            cmd_obj = cJSON_AddObjectToObject(this_reset, "object");
                            cJSON_AddNumberToObject(cmd_obj, "vnum", Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT]);
                            cJSON_AddStringToObject(cmd_obj, "name", obj_name(Objects, Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT]));
                            cmd_room = cJSON_AddObjectToObject(this_reset, "room");
                            cJSON_AddNumberToObject(cmd_room, "vnum", Zones->Zone[j].Cmds[k].Arg[ZONE_ROOM]);
                            cJSON_AddStringToObject(cmd_room, "name", room_name(Rooms, Zones->Zone[j].Cmds[k].Arg[ZONE_ROOM]));
                            cJSON_AddItemToArray(resets, this_reset);
                            break;

                        case ZONE_CMD_GIVE:
                            // This has to use LastMob
                            if(LastMob < 0 || LastLoc < 0)
                                break;
                            this_reset  = cJSON_CreateObject();
                            sprintf(cmd, "%c", Zones->Zone[j].Cmds[k].Command);
                            cJSON_AddStringToObject(this_reset, "command", "give object to mobile");
                            cmd_obj = cJSON_AddObjectToObject(this_reset, "object");
                            cJSON_AddNumberToObject(cmd_obj, "vnum", Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT]);
                            cJSON_AddStringToObject(cmd_obj, "name", obj_name(Objects, Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT]));
                            cmd_mob = cJSON_AddObjectToObject(this_reset, "mob");
                            cJSON_AddNumberToObject(cmd_mob, "vnum", LastMob);
                            cJSON_AddStringToObject(cmd_mob, "name", mob_name(Mobs, LastMob));
                            cJSON_AddItemToArray(resets, this_reset);
                            break;

                        case ZONE_CMD_EQUIP:
                            // This has to use LastMob
                            if(LastMob < 0 || LastLoc < 0)
                                break;
                            this_reset  = cJSON_CreateObject();
                            sprintf(cmd, "%c", Zones->Zone[j].Cmds[k].Command);
                            cJSON_AddStringToObject(this_reset, "command", "equip object to mobile");
                            cmd_obj = cJSON_AddObjectToObject(this_reset, "object");
                            cJSON_AddNumberToObject(cmd_obj, "vnum", Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT]);
                            cJSON_AddStringToObject(cmd_obj, "name", obj_name(Objects, Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT]));
                            cmd_mob = cJSON_AddObjectToObject(this_reset, "mob");
                            cJSON_AddNumberToObject(cmd_mob, "vnum", LastMob);
                            cJSON_AddStringToObject(cmd_mob, "name", mob_name(Mobs, LastMob));
                            cJSON_AddStringToObject(this_reset, "position", equip_name(Zones->Zone[j].Cmds[k].Arg[ZONE_POSITION]));
                            cJSON_AddItemToArray(resets, this_reset);
                            break;

                        case ZONE_CMD_PUT:
                            // We'll check LastLoc and LastObj to make sure
                            // an object was loaded into THIS room.
                            if(LastObj < 0 || LastLoc < 0)
                                break;
                            this_reset  = cJSON_CreateObject();
                            sprintf(cmd, "%c", Zones->Zone[j].Cmds[k].Command);
                            cJSON_AddStringToObject(this_reset, "command", "put object in object");
                            cmd_obj = cJSON_AddObjectToObject(this_reset, "object");
                            cJSON_AddNumberToObject(cmd_obj, "vnum", Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT]);
                            cJSON_AddStringToObject(cmd_obj, "name", obj_name(Objects, Zones->Zone[j].Cmds[k].Arg[ZONE_OBJECT]));
                            cmd_target = cJSON_AddObjectToObject(this_reset, "target");
                            cJSON_AddNumberToObject(cmd_target, "vnum", Zones->Zone[j].Cmds[k].Arg[ZONE_TARGET_OBJ]);
                            cJSON_AddStringToObject(cmd_target, "name", obj_name(Objects, Zones->Zone[j].Cmds[k].Arg[ZONE_TARGET_OBJ]));
                            cJSON_AddItemToArray(resets, this_reset);
                            break;

                        case ZONE_CMD_DOOR:
                            if(Zones->Zone[j].Cmds[k].Arg[ZONE_DOOR_ROOM] != Rooms->Room[i].Number)
                                break;
                            this_reset  = cJSON_CreateObject();
                            sprintf(cmd, "%c", Zones->Zone[j].Cmds[k].Command);
                            cJSON_AddStringToObject(this_reset, "command", "set door state");
                            cmd_target = cJSON_AddObjectToObject(this_reset, "door");
                            cJSON_AddStringToObject(cmd_target, "name", exit_name(Zones->Zone[j].Cmds[k].Arg[ZONE_DOOR_EXIT]));
                            cJSON_AddStringToObject(cmd_target, "state", doorstate_name(Zones->Zone[j].Cmds[k].Arg[ZONE_DOOR_STATE]));
                            cmd_room = cJSON_AddObjectToObject(this_reset, "room");
                            cJSON_AddNumberToObject(cmd_room, "vnum", Zones->Zone[j].Cmds[k].Arg[ZONE_DOOR_ROOM]);
                            cJSON_AddStringToObject(cmd_room, "name", room_name(Rooms, Zones->Zone[j].Cmds[k].Arg[ZONE_DOOR_ROOM]));
                            cJSON_AddItemToArray(resets, this_reset);
                            break;

                        case ZONE_CMD_REMOVE:
                            if(Zones->Zone[j].Cmds[k].Arg[ZONE_REMOVE_ROOM] != Rooms->Room[i].Number)
                                break;
                            this_reset  = cJSON_CreateObject();
                            sprintf(cmd, "%c", Zones->Zone[j].Cmds[k].Command);
                            cJSON_AddStringToObject(this_reset, "command", "remove object from room");
                            cmd_obj = cJSON_AddObjectToObject(this_reset, "object");
                            cJSON_AddNumberToObject(cmd_obj, "vnum", Zones->Zone[j].Cmds[k].Arg[ZONE_REMOVE_OBJ]);
                            cJSON_AddStringToObject(cmd_obj, "name", obj_name(Objects, Zones->Zone[j].Cmds[k].Arg[ZONE_REMOVE_OBJ]));
                            cmd_room = cJSON_AddObjectToObject(this_reset, "room");
                            cJSON_AddNumberToObject(cmd_room, "vnum", Zones->Zone[j].Cmds[k].Arg[ZONE_REMOVE_ROOM]);
                            cJSON_AddStringToObject(cmd_room, "name", room_name(Rooms, Zones->Zone[j].Cmds[k].Arg[ZONE_REMOVE_ROOM]));
                            cJSON_AddItemToArray(resets, this_reset);
                            break;

                        case ZONE_CMD_LEAD:
                            // This has to use LeaderMob
                            if(LeaderMob < 0 || LastLoc < 0)
                                break;
                            this_reset  = cJSON_CreateObject();
                            sprintf(cmd, "%c", Zones->Zone[j].Cmds[k].Command);
                            cJSON_AddStringToObject(this_reset, "command", "load mobile to follow leader");
                            cmd_mob = cJSON_AddObjectToObject(this_reset, "mob");
                            cJSON_AddNumberToObject(cmd_mob, "vnum", Zones->Zone[j].Cmds[k].Arg[ZONE_MOBILE]);
                            cJSON_AddStringToObject(cmd_mob, "name", mob_name(Mobs, Zones->Zone[j].Cmds[k].Arg[ZONE_MOBILE]));
                            cmd_target = cJSON_AddObjectToObject(this_reset, "leader");
                            cJSON_AddNumberToObject(cmd_target, "vnum", LeaderMob);
                            cJSON_AddStringToObject(cmd_target, "name", mob_name(Mobs, LeaderMob));
                            cmd_room = cJSON_AddObjectToObject(this_reset, "room");
                            cJSON_AddNumberToObject(cmd_room, "vnum", LastLoc);
                            cJSON_AddStringToObject(cmd_room, "name", room_name(Rooms, LastLoc));
                            cJSON_AddItemToArray(resets, this_reset);
                            LastMob = Zones->Zone[j].Cmds[k].Arg[ZONE_MOBILE];
                            break;

                        case ZONE_CMD_HATE:
                            // This has to use LastMob
                            if(LastMob < 0 || LastLoc < 0)
                                break;
                            this_reset  = cJSON_CreateObject();
                            sprintf(cmd, "%c", Zones->Zone[j].Cmds[k].Command);
                            cJSON_AddStringToObject(this_reset, "command", "set hate status of mobile");
                            cmd_mob = cJSON_AddObjectToObject(this_reset, "mob");
                            cJSON_AddNumberToObject(cmd_mob, "vnum", LastMob);
                            cJSON_AddStringToObject(cmd_mob, "name", mob_name(Mobs, LastMob));
                            cmd_room = cJSON_AddObjectToObject(this_reset, "room");
                            cJSON_AddNumberToObject(cmd_room, "vnum", LastLoc);
                            cJSON_AddStringToObject(cmd_room, "name", room_name(Rooms, LastLoc));
                            cJSON_AddStringToObject(this_reset, "hate", hate_name(Zones->Zone[j].Cmds[k].Arg[ZONE_HATE_TYPE], Zones->Zone[j].Cmds[k].Arg[ZONE_HATE_VALUE]));
                            cJSON_AddItemToArray(resets, this_reset);
                            break;

                        default:
                            // Error...
                            break;
                    }
                }
            }
        }
    }

    /*
    objects = cJSON_AddObjectToObject(root, "objects");
    for (int i = 0; i < Mobs->Count; i++) {
    }

    mobs = cJSON_AddObjectToObject(root, "mobs");
    for (int i = 0; i < Mobs->Count; i++) {
    }
    */

    // And now, we output!

    output = cJSON_Print(root);

    ofp = open_file(outfile, "w");
    fprintf(ofp, "%s", output);
    fclose(ofp);

    cJSON_Delete(root);
    if (!Quiet)
        fprintf(stderr, "done.\n");
}


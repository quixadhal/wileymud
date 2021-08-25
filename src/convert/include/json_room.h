#ifndef _JSON_ROOM_H
#define _JSON_ROOM_H

#ifndef _JSON_ROOM_C
extern const char *room_flag_names[64];
#include <cjson/cJSON.h>
#endif

cJSON *process_room_header(cJSON *this_room, rooms *Rooms, int i);
cJSON *process_room_zone_info(const char *KeyName, cJSON *this_room, rooms *Rooms, int i, zones *Zones);
cJSON *process_room_teleport(cJSON *this_room, rooms *Rooms, int i, zones *Zones);
cJSON *process_room_river(cJSON *this_room, rooms *Rooms, int i, zones *Zones);
cJSON *process_room_sounds(cJSON *this_room, rooms *Rooms, int i);
cJSON *process_room_extra_descriptions(cJSON *this_room, rooms *Rooms, int i);
cJSON *process_room_exits(cJSON *this_room, rooms *Rooms, int i, zones *Zones, objects *Objects);
int find_room_resets(rooms *Rooms, int i, zones *Zones, int **reset_checkoffs);
cJSON *process_room_resets(cJSON *this_room, rooms *Rooms, int i, zones *Zones, objects *Objects, mobs *Mobs,
                           int FoundResets);
cJSON *process_room(cJSON *parent_node, zones *Zones, int j, rooms *Rooms, objects *Objects, mobs *Mobs,
                    int **reset_checkoffs);
cJSON *process_rooms_in_zone(cJSON *this_zone, zones *Zones, int i, rooms *Rooms, objects *Objects, mobs *Mobs,
                             int **reset_checkoffs);

#endif

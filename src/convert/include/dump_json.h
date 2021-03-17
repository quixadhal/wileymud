#ifndef _DUMP_JSON_H
#define _DUMP_JSON_H

#define IS_SET(flag,bit) ((flag) & (bit))

#ifndef _DUMP_JSON_C
extern const char *room_flag_names[64];
#include <cjson/cJSON.h>
#endif

cJSON *process_zone_header(cJSON *this_zone, zones *Zones, int i);
cJSON *process_room_header(cJSON *this_room, rooms *Rooms, int i);
cJSON *process_room_specials(cJSON *this_room, int RoomVnum);
cJSON *process_flags(cJSON *this_thing, unsigned long Flags, const char **FlagNames);
cJSON *process_zone_info(cJSON *this_room, rooms *Rooms, int i, zones *Zones);
cJSON *process_room_teleport(cJSON *this_room, rooms *Rooms, int i);
cJSON *process_room_river(cJSON *this_room, rooms *Rooms, int i);
cJSON *process_room_sounds(cJSON *this_room, rooms *Rooms, int i);
cJSON *process_room_extra_descriptions(cJSON *this_room, rooms *Rooms, int i);
cJSON *process_room_exits(cJSON *this_room, rooms *Rooms, int i, objects *Objects);
int find_room_resets(rooms *Rooms, int i, zones *Zones, int **reset_checkoffs);
cJSON *process_room_resets(cJSON *this_room, rooms *Rooms, int i, zones *Zones, objects *Objects, mobs *Mobs, int FoundResets);
cJSON *process_orphaned_resets(cJSON *root, rooms *Rooms, zones *Zones, objects *Objects, mobs *Mobs, int **reset_checkoffs);
cJSON *process_zone_resets(cJSON *this_zone, rooms *Rooms, zones *Zones, int j, objects *Objects, mobs *Mobs);

void dump_as_json(zones *Zones, rooms *Rooms, objects *Objects, mobs *Mobs, shops *Shops, char *outfile);

#endif

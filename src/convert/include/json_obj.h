#ifndef _JSON_OBJ_H
#define _JSON_OBJ_H

#ifndef _JSON_OBJ_C
extern const char *wear_location_names[64];
extern const char *item_extra_names[64];
#include <cjson/cJSON.h>
#endif

cJSON *process_obj_zone_info(const char *KeyName, cJSON *this_obj, objects *Objects, int i, zones *Zones);
cJSON *process_obj_header(cJSON *this_obj, objects *Objects, int i);
cJSON *process_obj_extra_descriptions(cJSON *this_obj, objects *Objects, int i);
cJSON *process_obj_type_info(cJSON *this_obj, objects *Objects, int i);
cJSON *process_obj(cJSON *parent_node, zones *Zones, int j, rooms *Rooms, objects *Objects, mobs *Mobs, int **reset_checkoffs);
cJSON *process_objs_in_zone(cJSON *this_zone, zones *Zones, int i, rooms *Rooms, objects *Objects, mobs *Mobs, int **reset_checkoffs);

#endif

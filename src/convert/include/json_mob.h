#ifndef _JSON_MOB_H
#define _JSON_MOB_H

#ifndef _JSON_MOB_C
extern const char *act_flag_names[64];
extern const char *aff_flag_names[64];
#include <cjson/cJSON.h>
#endif

cJSON *process_mob_zone_info(const char *KeyName, cJSON *this_mob, mobs *Mobs, int i, zones *Zones);
cJSON *process_mob_header(cJSON *this_mob, mobs *Mobs, int i);
cJSON *process_mob_abilities(cJSON *this_mob, mobs *Mobs, int i);
cJSON *process_mob_saving_throws(cJSON *this_mob, mobs *Mobs, int i);
cJSON *process_mob_sounds(cJSON *this_mob, mobs *Mobs, int i);
cJSON *process_mob_attacks(cJSON *this_mob, mobs *Mobs, int i);
cJSON *process_mob_skills(cJSON *this_mob, mobs *Mobs, int i);
cJSON *process_mob(cJSON *parent_node, zones *Zones, int j, rooms *Rooms, objects *Objects, mobs *Mobs, int **reset_checkoffs);
cJSON *process_mobs_in_zone(cJSON *this_zone, zones *Zones, int i, rooms *Rooms, objects *Objects, mobs *Mobs, int **reset_checkoffs);

#endif

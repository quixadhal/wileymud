#ifndef _DUMP_JSON_H
#define _DUMP_JSON_H

#define IS_SET(flag,bit) ((flag) & (bit))

#ifndef _DUMP_JSON_C
extern const char *room_flag_names[64];
extern const char *act_flag_names[64];
extern const char *aff_flag_names[64];
extern const char *wear_location_names[64];
extern const char *item_extra_names[64];
#include <cjson/cJSON.h>
#endif

char *vnum_to_string(int vnum);
cJSON *cJSON_AddDiceToObject(cJSON *this_thing, const char *KeyName, dice Dice);
cJSON *cJSON_AddRawDiceToObject(cJSON *this_thing, const char *KeyName, int Rolls, int Die, int Modifier);
cJSON *process_zone_header(cJSON *this_zone, zones *Zones, int i);
cJSON *process_specials(cJSON *this_thing, int Vnum, int SpecialType);
cJSON *process_flags(cJSON *this_thing, unsigned long Flags, const char **FlagNames);
cJSON *process_mob_zone_info(const char *KeyName, cJSON *this_mob, mobs *Mobs, int i, zones *Zones);
cJSON *process_obj_zone_info(const char *KeyName, cJSON *this_obj, objects *Objects, int i, zones *Zones);
cJSON *process_orphaned_resets(cJSON *root, rooms *Rooms, zones *Zones, objects *Objects, mobs *Mobs, int **reset_checkoffs);
cJSON *process_zone_resets(cJSON *this_zone, rooms *Rooms, zones *Zones, int j, objects *Objects, mobs *Mobs);
cJSON *process_mob_header(cJSON *this_mob, mobs *Mobs, int i);
cJSON *process_mob_abilities(cJSON *this_mob, mobs *Mobs, int i);
cJSON *process_mob_saving_throws(cJSON *this_mob, mobs *Mobs, int i);
cJSON *process_mob_sounds(cJSON *this_mob, mobs *Mobs, int i);
cJSON *process_mob_attacks(cJSON *this_mob, mobs *Mobs, int i);
cJSON *process_mob_skills(cJSON *this_mob, mobs *Mobs, int i);
cJSON *process_obj_header(cJSON *this_obj, objects *Objects, int i);
cJSON *process_obj_type_info(cJSON *this_obj, objects *Objects, int i);
cJSON *process_reset_segment(int *LastMob, int *LastLoc, int *LastObj, int *LeaderMob, int i, int j, int k, rooms *Rooms, zones *Zones, objects *Objects, mobs *Mobs, int **reset_checkoffs);

void dump_as_json(zones *Zones, rooms *Rooms, objects *Objects, mobs *Mobs, shops *Shops, char *outfile);

#endif

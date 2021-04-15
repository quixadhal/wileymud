#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <cjson/cJSON.h>
#include "include/structs.h"
#include "include/main.h"
#include "include/utils.h"
#include "include/parse_wiley.h"

#define _JSON_MOB_C
#include "include/dump_json.h"
#include "include/json_room.h"
#include "include/json_mob.h"
#include "include/json_obj.h"

// unsigned long is currently 64 bits...
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

cJSON *process_mob_zone_info(const char *KeyName, cJSON *this_mob, mobs *Mobs, int i, zones *Zones)
{
    cJSON *zone = NULL;

    zone = cJSON_AddObjectToObject(this_mob, KeyName);
    cJSON_AddNumberToObject(zone, "vnum", Mobs->Mob[i].Zone);
    cJSON_AddStringToObject(zone, "name", zone_name(Zones, Mobs->Mob[i].Zone));

    return zone;
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

cJSON *process_mob(cJSON *parent_node, zones *Zones, int j, rooms *Rooms, objects *Objects, mobs *Mobs, int **reset_checkoffs)
{
    cJSON *this_mob = NULL;

    this_mob = cJSON_AddObjectToObject(parent_node, vnum_to_string(Mobs->Mob[j].Number));

    process_mob_header(this_mob, Mobs, j);
    process_specials(this_mob, Mobs->Mob[j].Number, SPECIAL_MOB);
    process_mob_zone_info("zone", this_mob, Mobs, j, Zones);
    process_flags(this_mob, "act_flags", Mobs->Mob[j].ActFlags, act_flag_names);
    process_flags(this_mob, "aff_flags", Mobs->Mob[j].AffectedBy, aff_flag_names);
    process_mob_abilities(this_mob, Mobs, j);
    process_mob_saving_throws(this_mob, Mobs, j);
    process_mob_sounds(this_mob, Mobs, j);
    process_mob_attacks(this_mob, Mobs, j);
    process_mob_skills(this_mob, Mobs, j);

    return this_mob;
}

cJSON *process_mobs_in_zone(cJSON *this_zone, zones *Zones, int i, rooms *Rooms, objects *Objects, mobs *Mobs, int **reset_checkoffs)
{
    cJSON *mobiles = NULL;

    mobiles = cJSON_AddObjectToObject(this_zone, "mobs");
    for (int j = 0; j < Mobs->Count; j++) {
        if (!Quiet)
            spin(stderr);
        if(Mobs->Mob[j].Zone != Zones->Zone[i].Number) {
            continue;
        }
        process_mob(mobiles, Zones, j, Rooms, Objects, Mobs, reset_checkoffs);
    }
    return mobiles;
}

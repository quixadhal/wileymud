#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <cjson/cJSON.h>
#include "include/structs.h"
#include "include/main.h"
#include "include/utils.h"
#include "include/parse_wiley.h"

#define _JSON_OBJ_C
#include "include/dump_json.h"
#include "include/json_room.h"
#include "include/json_mob.h"
#include "include/json_obj.h"

// unsigned long is currently 64 bits...
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

cJSON *process_obj_zone_info(const char *KeyName, cJSON *this_obj, objects *Objects, int i, zones *Zones)
{
    cJSON *zone = NULL;

    zone = cJSON_AddObjectToObject(this_obj, KeyName);
    cJSON_AddNumberToObject(zone, "vnum", Objects->Object[i].Zone);
    cJSON_AddStringToObject(zone, "name", zone_name(Zones, Objects->Object[i].Zone));

    return zone;
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

cJSON *process_item_light(cJSON *this_obj, objects *Objects, int i)
{
    cJSON *type_details = NULL;

    type_details = cJSON_AddObjectToObject(this_obj, "type_details");
    cJSON_AddNumberToObject(type_details, "type_id", Objects->Object[i].Type);
    cJSON_AddStringToObject(type_details, "name", item_type_name(Objects->Object[i].Type));
    cJSON_AddNumberToObject(type_details, "duration", Objects->Object[i].Flags.Value[2]);
    // TODO get useful values for these, instead of just raw numbers
    cJSON_AddNumberToObject(type_details, "light_type", Objects->Object[i].Flags.Value[1]);
    cJSON_AddNumberToObject(type_details, "light_color", Objects->Object[i].Flags.Value[0]);

    return type_details;
}

cJSON *process_item_scroll_potion(cJSON *this_obj, objects *Objects, int i)
{
    cJSON *type_details = NULL;
    int found = 0;

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

    return type_details;
}

cJSON *process_item_wand_staff(cJSON *this_obj, objects *Objects, int i)
{
    cJSON *type_details = NULL;

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

    return type_details;
}

cJSON *process_item_weapon(cJSON *this_obj, objects *Objects, int i)
{
    cJSON *type_details = NULL;

    type_details = cJSON_AddObjectToObject(this_obj, "type_details");
    cJSON_AddNumberToObject(type_details, "type_id", Objects->Object[i].Type);
    cJSON_AddStringToObject(type_details, "name", item_type_name(Objects->Object[i].Type));
    // [1]d[2] damage of type [3]
    cJSON_AddRawDiceToObject(type_details, "damage",
            Objects->Object[i].Flags.Value[1],
            Objects->Object[i].Flags.Value[2], 0);
    cJSON_AddNumberToObject(type_details, "damage_type_id", Objects->Object[i].Flags.Value[3]);
    cJSON_AddStringToObject(type_details, "damage_type", damage_name(Objects->Object[i].Flags.Value[3]));

    return type_details;
}

cJSON *process_item_armor(cJSON *this_obj, objects *Objects, int i)
{
    cJSON *type_details = NULL;

    type_details = cJSON_AddObjectToObject(this_obj, "type_details");
    cJSON_AddNumberToObject(type_details, "type_id", Objects->Object[i].Type);
    cJSON_AddStringToObject(type_details, "name", item_type_name(Objects->Object[i].Type));
    // value [0], armor class [1]
    cJSON_AddNumberToObject(type_details, "gold", Objects->Object[i].Flags.Value[0] / 10.0);
    cJSON_AddNumberToObject(type_details, "armor_class", Objects->Object[i].Flags.Value[1] / 10.0);

    return type_details;
}

cJSON *process_item_trap(cJSON *this_obj, objects *Objects, int i)
{
    cJSON *type_details = NULL;

    type_details = cJSON_AddObjectToObject(this_obj, "type_details");
    cJSON_AddNumberToObject(type_details, "type_id", Objects->Object[i].Type);
    cJSON_AddStringToObject(type_details, "name", item_type_name(Objects->Object[i].Type));
    // [3] charges, attack type [1], damage [2]

    return type_details;
}

cJSON *process_item_container(cJSON *this_obj, objects *Objects, int i)
{
    cJSON *type_details = NULL;

    type_details = cJSON_AddObjectToObject(this_obj, "type_details");
    cJSON_AddNumberToObject(type_details, "type_id", Objects->Object[i].Type);
    cJSON_AddStringToObject(type_details, "name", item_type_name(Objects->Object[i].Type));

    return type_details;
}

cJSON *process_item_drink(cJSON *this_obj, objects *Objects, int i)
{
    cJSON *type_details = NULL;

    type_details = cJSON_AddObjectToObject(this_obj, "type_details");
    cJSON_AddNumberToObject(type_details, "type_id", Objects->Object[i].Type);
    cJSON_AddStringToObject(type_details, "name", item_type_name(Objects->Object[i].Type));

    return type_details;
}

cJSON *process_item_note(cJSON *this_obj, objects *Objects, int i)
{
    cJSON *type_details = NULL;

    type_details = cJSON_AddObjectToObject(this_obj, "type_details");
    cJSON_AddNumberToObject(type_details, "type_id", Objects->Object[i].Type);
    cJSON_AddStringToObject(type_details, "name", item_type_name(Objects->Object[i].Type));

    return type_details;
}

cJSON *process_item_key(cJSON *this_obj, objects *Objects, int i)
{
    cJSON *type_details = NULL;

    type_details = cJSON_AddObjectToObject(this_obj, "type_details");
    cJSON_AddNumberToObject(type_details, "type_id", Objects->Object[i].Type);
    cJSON_AddStringToObject(type_details, "name", item_type_name(Objects->Object[i].Type));

    return type_details;
}

cJSON *process_item_food(cJSON *this_obj, objects *Objects, int i)
{
    cJSON *type_details = NULL;

    type_details = cJSON_AddObjectToObject(this_obj, "type_details");
    cJSON_AddNumberToObject(type_details, "type_id", Objects->Object[i].Type);
    cJSON_AddStringToObject(type_details, "name", item_type_name(Objects->Object[i].Type));

    return type_details;
}

cJSON *process_item_money(cJSON *this_obj, objects *Objects, int i)
{
    cJSON *type_details = NULL;

    type_details = cJSON_AddObjectToObject(this_obj, "type_details");
    cJSON_AddNumberToObject(type_details, "type_id", Objects->Object[i].Type);
    cJSON_AddStringToObject(type_details, "name", item_type_name(Objects->Object[i].Type));

    return type_details;
}

cJSON *process_item_trash(cJSON *this_obj, objects *Objects, int i)
{
    cJSON *type_details = NULL;

    type_details = cJSON_AddObjectToObject(this_obj, "type_details");
    cJSON_AddNumberToObject(type_details, "type_id", Objects->Object[i].Type);
    cJSON_AddStringToObject(type_details, "name", item_type_name(Objects->Object[i].Type));

    return type_details;
}

cJSON *process_item_pen(cJSON *this_obj, objects *Objects, int i)
{
    cJSON *type_details = NULL;

    type_details = cJSON_AddObjectToObject(this_obj, "type_details");
    cJSON_AddNumberToObject(type_details, "type_id", Objects->Object[i].Type);
    cJSON_AddStringToObject(type_details, "name", item_type_name(Objects->Object[i].Type));

    return type_details;
}

cJSON *process_item_board(cJSON *this_obj, objects *Objects, int i)
{
    cJSON *type_details = NULL;

    type_details = cJSON_AddObjectToObject(this_obj, "type_details");
    cJSON_AddNumberToObject(type_details, "type_id", Objects->Object[i].Type);
    cJSON_AddStringToObject(type_details, "name", item_type_name(Objects->Object[i].Type));

    return type_details;
}

cJSON *process_item_boat(cJSON *this_obj, objects *Objects, int i)
{
    cJSON *type_details = NULL;

    type_details = cJSON_AddObjectToObject(this_obj, "type_details");
    cJSON_AddNumberToObject(type_details, "type_id", Objects->Object[i].Type);
    cJSON_AddStringToObject(type_details, "name", item_type_name(Objects->Object[i].Type));

    return type_details;
}

cJSON *process_item_worn(cJSON *this_obj, objects *Objects, int i)
{
    cJSON *type_details = NULL;

    type_details = cJSON_AddObjectToObject(this_obj, "type_details");
    cJSON_AddNumberToObject(type_details, "type_id", Objects->Object[i].Type);
    cJSON_AddStringToObject(type_details, "name", item_type_name(Objects->Object[i].Type));

    return type_details;
}

cJSON *process_obj_type_info(cJSON *this_obj, objects *Objects, int i)
{
    cJSON *type_details = NULL;

    // Type
    // AffectCount, Affect[]
    // Flags
    switch (Objects->Object[i].Type) {
        case ITEM_LIGHT:
            type_details = process_item_light(this_obj, Objects, i);
            break;
        case ITEM_SCROLL:
        case ITEM_POTION:
            type_details = process_item_scroll_potion(this_obj, Objects, i);
            break;
        case ITEM_WAND:
        case ITEM_STAFF:
            type_details = process_item_wand_staff(this_obj, Objects, i);
            break;
        case ITEM_WEAPON:
        case ITEM_FIREWEAPON:
        case ITEM_MISSILE:
            type_details = process_item_weapon(this_obj, Objects, i);
            break;
        case ITEM_ARMOR:
            type_details = process_item_armor(this_obj, Objects, i);
            break;
        case ITEM_TRAP:
            type_details = process_item_trap(this_obj, Objects, i);
            break;
        case ITEM_CONTAINER:
            type_details = process_item_container(this_obj, Objects, i);
            break;
        case ITEM_DRINKCON:
            type_details = process_item_drink(this_obj, Objects, i);
            break;
        case ITEM_NOTE:
            type_details = process_item_note(this_obj, Objects, i);
            break;
        case ITEM_KEY:
            type_details = process_item_key(this_obj, Objects, i);
            break;
        case ITEM_FOOD:
            type_details = process_item_food(this_obj, Objects, i);
            break;
        case ITEM_MONEY:
            type_details = process_item_money(this_obj, Objects, i);
            break;
        case ITEM_TRASH:
            type_details = process_item_trash(this_obj, Objects, i);
            break;
        case ITEM_PEN:
            type_details = process_item_pen(this_obj, Objects, i);
            break;
        case ITEM_BOARD:
            type_details = process_item_board(this_obj, Objects, i);
            break;
        case ITEM_BOAT:
            type_details = process_item_boat(this_obj, Objects, i);
            break;
        case ITEM_WORN:
            type_details = process_item_worn(this_obj, Objects, i);
            break;
        default:
            cJSON_AddNullToObject(this_obj, "type_details");
            break;
    }

    return this_obj;
}

cJSON *process_obj(cJSON *parent_node, zones *Zones, int j, rooms *Rooms, objects *Objects, mobs *Mobs, int **reset_checkoffs)
{
    cJSON *this_obj = NULL;

    this_obj = cJSON_AddObjectToObject(parent_node, vnum_to_string(Objects->Object[j].Number));

    process_obj_header(this_obj, Objects, j);
    process_obj_zone_info("zone", this_obj, Objects, j, Zones);
    process_obj_extra_descriptions(this_obj, Objects, j);
    process_flags(this_obj, "wear_flags", Objects->Object[j].Flags.Wear, wear_location_names);
    process_flags(this_obj, "extra_flags", Objects->Object[j].Flags.Extra, item_extra_names);
    process_obj_type_info(this_obj, Objects, j);

    return this_obj;
}

cJSON *process_objs_in_zone(cJSON *this_zone, zones *Zones, int i, rooms *Rooms, objects *Objects, mobs *Mobs, int **reset_checkoffs)
{
    cJSON *objects = NULL;

    objects = cJSON_AddObjectToObject(this_zone, "objects");
    for (int j = 0; j < Objects->Count; j++) {
        if (!Quiet)
            spin(stderr);
        if(Objects->Object[j].Zone != Zones->Zone[i].Number) {
            continue;
        }
        process_obj(objects, Zones, j, Rooms, Objects, Mobs, reset_checkoffs);
    }
    return objects;
}

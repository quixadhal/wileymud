/*
 * file: limits.c , Limit and gain control module.        Part of DIKUMUD
 * Usage: Procedures controling gain and limit.
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */

#include <stdio.h>
#include <stdlib.h>
/* #include <unistd.h> */
#include <sys/types.h>
#include <signal.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "global.h"
#include "bug.h"
#include "spells.h"
#include "comm.h"
#include "db.h"
#include "spell_parser.h"
#include "utils.h"
#include "multiclass.h"
#include "fight.h"
#include "reception.h"
#include "interpreter.h"
#include "handler.h"
#include "act_obj.h"
#include "act_other.h"
#include "weather.h" // age() function

#define _DIKU_LIMITS_C
#include "mudlimits.h"

#undef BOOT_IDLE /* define this to kick idle people, undefine to let them stay idle all day */

struct title_type titles[6][ABS_MAX_LVL + 1] = {
    {{"Man", "Woman", 0},
     {"Supplicant", "Supplicant", 1}, /* 1 */
     {"Apprentice", "Apprentice", 2500},
     {"Student", "Student", 5000},
     {"Scholar", "Scholar", 10000},
     {"Trickster", "Trickster", 20000},
     {"Scribe", "Scribe", 40000},
     {"Illusionist", "Illusionist", 60000},
     {"Cabalist", "Cabalist", 90000},
     {"Apparitionist", "Apparitionist", 135000},
     {"Medium", "Gypsy", 250000}, /* 10 */
     {"Sage", "Sage", 375000},
     {"Diviner", "Diviner", 625000},
     {"Alchemist", "Alchemist", 875000},
     {"Evoker", "Evoker", 1250000},
     {"Abjurer", "Abjuress", 1625000},
     {"Invoker", "Invoker", 2000000},
     {"Enchanter", "Enchantress", 2375000},
     {"Conjurer", "Conjuress", 2750000},
     {"Summoner", "Summoner", 3125000},
     {"Spiritualist", "Spiritualist", 3525000}, /* 20 */
     {"Shaman", "Shamaness", 3925000},
     {"Magician", "Magician", 4325000},
     {"Mystic", "Mystic", 4725000},
     {"Necromancer", "Necromancer", 5000000},
     {"Mentalist", "Mentalist", 5500000},
     {"Synergist", "Synergist", 6000000},
     {"Thaumaturgist", "Thaumaturgist", 6500000},
     {"Dispeller", "Dispeller", 7000000},
     {"Occultist", "Occultist", 7500000},
     {"Apprentice of the Tower", "Apprentice of the Tower", 8000000}, /* 30 */
     {"Servant of the Tower", "Servant of the Tower", 8500000},
     {"Student of the Tower", "Student of the Tower", 9000000},
     {"Guardian of the Tower", "Guardian of the Tower", 9500000},
     {"Watchman of the Tower", "Watchwoman of the Tower", 10000000},
     {"Keeper of the Tower", "Keeper of the Tower", 10750000},
     {"Master Illusionist", "Master Illusionist", 11500000},
     {"Master Spellmaster", "Master Spellmistress", 12250000},
     {"Master Enchanter", "Master Enchantress", 13000000},
     {"Master Spiritualist", "Master Spiritualist", 13750000},
     {"Master of the Tower", "Mistress of the Tower", 14500000}, /* 40 */
     {"Master of the Occult", "Mistress of the Occult", 15500000},
     {"Lord Necromancer", "Lady Necromancer", 16500000},
     {"Master of Energies", "Mistress of Energies", 17500000},
     {"Master of the Mists", "Mistress of the Mists", 18500000},
     {"Lord of the Tower", "Lady of the Tower", 19500000},
     {"Lord of the Mists", "Lady of the Mists", 20000000},
     {"High Lord of the Mists", "High Lady of the Mists", 21000000},
     {"Arch Lord of the Mists", "Arch Lady of the Mists", 22000000},
     {"Magi", "Majestrix", 23000000},
     {"Arch Magi", "Arch Majestrix", 25000000}, /* 50 */
     {"Immortal Warlock", "Immortal Enchantress", 27000000},
     {"Immortal Warlock", "Immortal Enchantress", 29000000},
     {"Immortal Warlock", "Immortal Enchantress", 31000000},
     {"Immortal Warlock", "Immortal Enchantress", 33000000},
     {"Immortal Warlock", "Immortal Enchantress", 35000000},
     {"Immortal Warlock", "Immortal Enchantress", 40000000},
     {"Avatar of Magic", "Avatar of Magic", 45000000},
     {"Source of Magic", "Source of Magic", 50000000},
     {"Demigod", "Demigoddess", 60000000},
     {"God", "Goddess", 75000000}, /* 60 */
     {"Bug", "Bug", 75000001}},

    {{"Man", "Woman", 0},
     {"Believer", "Believer", 1},
     {"Attendant", "Attendant", 1500},
     {"Acolyte", "Acolyte", 3000},
     {"Novice", "Novice", 6000},
     {"Missionary", "Missionary", 13000},
     {"Adept", "Adept", 27500},
     {"Deacon", "Deaconess", 55000},
     {"Vicar", "Vicaress", 110000},
     {"Priest", "Priestess", 225000},
     {"Minister", "Lady Minister", 450000},
     {"Canon", "Canon", 675000},
     {"Levite", "Levitess", 900000},
     {"Curate", "Curess", 1150000},
     {"Monk", "Nunne", 1400000},
     {"Healer", "Healer", 1650000},
     {"Chaplain", "Chaplain", 1900000},
     {"Expositor", "Expositress", 2250000},
     {"Bishop", "Bishop", 2600000},
     {"Arch Bishop", "Arch Lady", 2950000},
     {"Patriarch", "Matriarch", 3300000},
     {"Deacon of Shylar", "Deaconess of Shylar", 3650000},
     {"Deacon of Highstaff", "Deaconess of Highstaff", 4000000},
     {"Deacon of Gredth", "Deaconess of Gredth", 4400000},
     {"Deacon of the Reach", "Deaconess of the Reach", 4800000},
     {"Deacon of Nesthar", "Deaconess of Nesthar", 5250000},
     {"Vicar of Shylar", "Vicaress of Shylar", 5700000},
     {"Vicar of Highstaff", "Vicaress of Highstaff", 6000000},
     {"Vicar of Gredth", "Vicaress of Gredth", 6500000},
     {"Vicar of the Reach", "Vicaress of the Reach", 7000000},
     {"Vicar of Nesthar", "Vicaress of Nesthar", 7500000},
     {"Priest of Shylar", "Priestess of Shylar", 8000000},
     {"Priest of Highstaff", "Priestess of Highstaff", 8500000},
     {"Priest of Gredth", "Priestess of Gredth", 9000000},
     {"Priest of the Reach", "Priestess of the Reach", 9500000},
     {"Priest of Nesthar", "Priestess of Nesthar", 10000000},
     {"High Priest of Shylar", "High Priestess of Shylar", 10750000},
     {"High Priest of Highstaff", "High Priestess of Highstaff", 11500000},
     {"High Priest of Gredth", "High Priestess of Gredth", 12250000},
     {"High Priest of the Reach", "High Priestess of the Reach", 13000000},
     {"High Priest of Nesthar", "High Priestess of Nesthar", 13750000},
     {"Canon of the Mission", "Canon of the Mission", 14500000},
     {"Levite of the Mission", "Levitess of the Mission", 15500000},
     {"Curate of the Mission", "Curate of the Mission", 16500000},
     {"Monk of the Mission", "Nunne of the Mission", 17500000},
     {"Healer of the Mission", "Healer of the Mission", 20000000},
     {"Curate of the Realms", "Curate of the Realms", 21000000},
     {"Priest of the Realms", "Priestess of the Realms", 22000000},
     {"Bishop of the Realms", "Lady Bishop of the Realms", 23000000},
     {"High Priest of the Realms", "High Priestess of the Realms", 24000000},
     {"Patriarch of the Realms", "Matriarch of the Realms", 25000000},
     {"Immortal Cardinal", "Immortal Priestess", 27000000},
     {"Immortal Cardinal", "Immortal Priestess", 29000000},
     {"Immortal Cardinal", "Immortal Priestess", 31000000},
     {"Immortal Cardinal", "Immortal Priestess", 33000000},
     {"Immortal Cardinal", "Immortal Priestess", 35000000},
     {"Immortal Cardinal", "Immortal Priestess", 40000000},
     {"Avatar of Faith", "Avatar of Faith", 45000000},
     {"Source of Faith", "Source of Faith", 50000000},
     {"Demigod", "Demigoddess", 60000000},
     {"God", "Goddess", 75000000},
     {"Bug", "Bug", 75000001}},

    {{"Man", "Woman", 0},
     {"Swordpupil", "Swordpupil", 1},
     {"Recruit", "Recruit", 2000},
     {"Sentry", "Sentress", 4000},
     {"Fighter", "Fighter", 8000},
     {"Soldier", "Soldier", 16000},
     {"Warrior", "Warrior", 32000},
     {"Veteran", "Veteran", 64000},
     {"Swordsman", "Swordswoman", 128000},
     {"Fencer", "Fenceress", 256000},
     {"Combatant", "Combatrix", 384000},
     {"Hero", "Heroine", 512000},
     {"Myrmidon", "Myrmidon", 640000},
     {"Swashbuckler", "Swashbuckleress", 768000},
     {"Mercenary", "Mercenaress", 1024000},
     {"Swordmaster", "Swordmistress", 1250000},
     {"Lieutenant", "Lieutenant", 1500000},
     {"Champion", "Lady Champion", 1750000},
     {"Dragoon", "Lady Dragoon", 2000000},
     {"Cavalier", "Cavalier", 2250000},
     {"Knight", "Lady Knight", 2500000},
     {"Squire of the Plume", "Squire of the Plume", 3000000},
     {"Lieutenant of the Plume", "Lieutenant of the Plume", 3500000},
     {"Sargent of the Plume", "Sargent of the Plume", 4000000},
     {"Captain of the Plume", "Captain of the Plume", 4500000},
     {"Knight of the Plume", "Knight of the Plume", 5000000},
     {"Baron of the Keep", "Baroness of the Keep", 5500000},
     {"Baron of the Keep", "Baroness of the Keep", 6000000},
     {"Baron of the Keep", "Baroness of the Keep", 6500000},
     {"Baron of the Keep", "Baroness of the Keep", 7000000},
     {"Lord Baron", "Lady Baroness", 7500000},
     {"Keeps Nobleman", "Keeps Noblelady", 8000000},
     {"Keeps Nobleman", "Keeps Noblelady", 8500000},
     {"Keeps Nobleman", "Keeps Noblelady", 9000000},
     {"Keeps Nobleman", "Keeps Noblelady", 9500000},
     {"Lord Nobleman", "Lady Mistress", 10000000},
     {"Count of the Keep", "Countess of the Keep", 11000000},
     {"Count of the Keep", "Countess of the Keep", 12000000},
     {"Count of the Keep", "Countess of the Keep", 13000000},
     {"Count of the Keep", "Countess of the Keep", 14000000},
     {"Lord Count", "Lady Countess", 15000000},
     {"Court Baron", "Court Baroness", 16000000},
     {"Court Nobleman", "Court Lady", 17000000},
     {"Court Count", "Court Countess", 18000000},
     {"Chancellor", "Chancellor", 19000000},
     {"Lord Chancellor", "Lady Chancellor", 20000000},
     {"Kings Baron", "King Baroness", 21000000},
     {"Kings Nobleman", "Kings Lady", 22000000},
     {"Kings Count", "Kings Countess", 23000000},
     {"Prince of the Throne", "Princess of the Throne", 24000000},
     {"King of the Realms", "Queen of the Realms", 25000000},
     {"Immortal Warrior", "Immortal Warrior", 27000000},
     {"Immortal Warrior", "Immortal Warrior", 29000000},
     {"Immortal Warrior", "Immortal Warrior", 31000000},
     {"Immortal Warrior", "Immortal Warrior", 33000000},
     {"Immortal Warrior", "Immortal Warrior", 35000000},
     {"Immortal Warrior", "Immortal Warrior", 40000000},
     {"Avatar of Strength", "Avatar of Strength", 45000000},
     {"Source of Strength", "Source of Strength", 50000000},
     {"Demigod", "Demigoddess", 60000000},
     {"God", "Goddess", 75000000},
     {"Bug", "Bug", 75000001}},

    {{"Man", " Woman", 0},
     {"Pilferer", "Pilferess", 1},
     {"Footpad", "Footpad", 1250},
     {"Filcher", "Filcheress", 2500},
     {"Pick-Pocket", "Pick-Pocket", 5000},
     {"Sneak", "Sneak", 10000},
     {"Pincher", "Pincheress", 20000},
     {"Cut-Purse", "Cut-Purse", 30000},
     {"Snatcher", "Snatcheress", 50000},
     {"Sharper", "Sharper", 85000},
     {"Rogue", "Rogue", 125000},
     {"Robber", "Robber", 200000},
     {"Magsman", "Magswoman", 340000},
     {"Highwayman", "Highwaywoman", 560000},
     {"Burglar", "Burglaress", 780000},
     {"Thief", "Thief", 1000000},
     {"Knifer", "Knifer", 1200000},
     {"Quick-Blade", "Quick-Blade", 1400000},
     {"Killer", "Murderess", 1600000},
     {"Brigand", "Brigand", 1800000},
     {"Cut-Throat", "Cut-Throat", 2000000},
     {"Master Pilferer", "Master Pilferess", 2200000},
     {"Master Footpad", "Master Footpad", 2400000},
     {"Master Filcher", "Master Filcheress", 2600000},
     {"Master Sneak", "Master Sneak", 2800000},
     {"Apprentice Shadower", "Apprentice Shadower", 3000000},
     {"Novice Shadower", "Novice Shadower", 3300000},
     {"Shadower", "Shadower", 3600000},
     {"Dark Shadower", "Dark Shadower", 4000000},
     {"Master Shadower", "Master Shadower", 4250000},
     {"Apprentice of the House", "Apprentice of the House", 4500000},
     {"Snatcher of the House", "Snatcheress of the House", 5000000},
     {"Sharper of the House", "Sharper of the House", 5500000},
     {"Master Sharper of the House", "Master Sharper of the House", 6000000},
     {"Rogue of the House", "Rogue of the House", 6500000},
     {"Master Rogue", "Master Rogue", 7000000},
     {"Apprentice Darker", "Apprentice Darker", 7500000},
     {"Novice Darker", "Novice Darker", 8000000},
     {"Darker", "Darker", 9000000},
     {"Master Darker", "Master Darker", 10000000},
     {"Rogue of the Pale", "Rogue of the Pale", 12000000},
     {"Thief of the Pale", "Thief of the Pale", 13000000},
     {"Leader of the Pale", "Lady of the Pale", 14000000},
     {"Rogue of the Shadow", "Rogue of the Shadow", 16000000},
     {"Thief of the Shadow", "Thief of the Shadow", 17000000},
     {"Leader of the Shadow", "Leader of the Shadow", 18000000},
     {"Rogue of Midnight", "Rogue of Midnight", 21000000},
     {"Thief of Midnight", "Thief of Midnight", 22000000},
     {"Leader of Midnight", "Lady of Midnight", 23000000},
     {"Darkness", "Darkness", 24000000},
     {"Lord Midnight", "Lady Midnight", 25000000},
     {"Immortal Assasin", "Immortal Assasin", 27000000},
     {"Immortal Assasin", "Immortal Assasin", 29000000},
     {"Immortal Assasin", "Immortal Assasin", 31000000},
     {"Immortal Assasin", "Immortal Assasin", 33000000},
     {"Immortal Assasin", "Immortal Assasin", 35000000},
     {"Immortal Assasin", "Immortal Assasin", 40000000},
     {"Avatar of Deceit", "Avatar of Deceit", 45000000},
     {"Source of Deceit", "Source of Deceit", 50000000},
     {"Demigod", "Demigoddess", 60000000},
     {"God", "Goddess", 75000000},
     {"Bug", "Bug", 75000001}},

    /* Ranger */

    {{"Man", "Woman", 0},
     {"Runner", "Runneress", 1},
     {"Stryder", "Strydess", 2000},
     {"Scout", "Scoutess", 4000},
     {"Courser", "Courseress", 8000},
     {"Tracker", "Trackeress", 15000},
     {"Guide", "Guide", 30000},
     {"Pathfinder", "Pathfinder", 75000},
     {"Ranger", "Rangeress", 100000},
     {"Greater Ranger", "Greater Rangeress", 125000},
     {"Woodland Squire", "Woodland Squire", 300000}, /* 10 level */
     {"Apprentice Druid", "Apprentice Druid", 650000},
     {"Protector of the Woods", "Protector of the Woods", 1000000},
     {"Keeper of the Woods", "Keeper of the Woods", 1250000},
     {"Woods Walker", "Woods Walkeress", 1500000},
     {"Ranger Knight", "Ranger Knight", 1750000},
     {"Knight of the Moores", "Knight of the Moores", 2000000},
     {"Knight of Shylar", "Knight of Shylar", 2250000},
     {"Knight of Highstaff", "Knight of Highstaff", 2500000},
     {"Knight of the Reach", "Knight of the Reach", 2750000},
     {"Knight of Wiley", "Knight of Wiley", 3000000}, /* 20 level */
     {"Common Lord", "Common Lady", 3250000},
     {"Common Lord", "Common Lady", 3500000},
     {"Common Lord", "Common Lady", 4000000},
     {"Common Lord", "Common Lady", 4500000},
     {"Lord of the Land", "Lady of the Land", 5000000}, /* 25 level */
     {"Lord of the Land", "Lady of the Land", 5500000},
     {"Lord of the Land", "Lady of the Land", 6000000},
     {"Lord of the Land", "Lady of the Land", 6500000},
     {"Lord of the Land", "Lady of the Land", 7000000},
     {"Baron of the Land", "Baroness of the Land", 7500000}, /* 30 level */
     {"Baron of the Land", "Baroness of the Land", 8000000},
     {"Baron of the Land", "Baroness of the Land", 8500000},
     {"Baron of the Land", "Baroness of the Land", 9000000},
     {"Baron of the Land", "Baroness of the Land", 9500000},
     {"Leader of Men", "Leader of Men", 10000000}, /* 35 level */
     {"Leader of Men", "Leader of Men", 11000000},
     {"Leader of Men", "Leader of Men", 12000000},
     {"Leader of Men", "Leader of Men", 13000000},
     {"Leader of Men", "Leader of Men", 14000000},
     {"City Ruler", "City Ruler", 15000000}, /* 40 level */
     {"City Ruler", "City Ruler", 16000000},
     {"City Ruler", "City Ruler", 17000000},
     {"City Ruler", "City Ruler", 18000000},
     {"City Ruler", "City Ruler", 19000000},
     {"King of his Realm", "Queen of her Realm", 20000000},
     {"King of his Realm", "Queen of her Realm", 21000000},
     {"King of his Realm", "Queen of her Realm", 22000000},
     {"King of his Realm", "Queen of her Realm", 23000000},
     {"King of his Realm", "Queen of her Realm", 24000000},
     {"Leader of Masses", "Leader of Masses", 25000000},
     {"Immortal", "Immortal", 27000000},
     {"Immortal", "Immortal", 29000000},
     {"Immortal", "Immortal", 31000000},
     {"Immortal", "Immortal", 33000000},
     {"Immortal", "Immortal", 35000000},
     {"Immortal", "Immortal", 40000000},
     {"Avatar of Hunting", "Avatar of Hunting", 45000000},
     {"Source of Hunting", "Source of Hunting", 50000000},
     {"Demigod", "Demigoddess", 60000000},
     {"God", "Goddess", 75000000},
     {"Bug", "Bug", 75000001}},

    /* Druids */

    {{"Man", "Woman", 0},
     {"Believer", "Believer", 1},
     {"Attendant", "Attendant", 1500},
     {"Acolyte", "Acolyte", 3000},
     {"Novice", "Novice", 6000},
     {"Missionary", "Missionary", 13000},
     {"Adept", "Adept", 27500},
     {"Deacon", "Deaconess", 55000},
     {"Vicar", "Vicaress", 110000},
     {"Priest", "Priestess", 225000},
     {"Minister", "Lady Minister", 450000},
     {"Canon", "Canon", 675000},
     {"Levite", "Levitess", 900000},
     {"Curate", "Curess", 1125000},
     {"Monk", "Nunne", 1350000},
     {"Healer", "Healer", 1575000},
     {"Chaplain", "Chaplain", 1800000},
     {"Expositor", "Expositress", 2025000},
     {"Bishop", "Bishop", 2250000},
     {"Arch Bishop", "Arch Lady", 2475000},
     {"Patriarch", "Matriarch", 2700000},
     {"High Priest", "High Priestess", 3000000},
     {"High Priest", "High Priestess", 3250000},
     {"High Priest", "High Priestess", 3500000},
     {"High Priest", "High Priestess", 3750000},
     {"High Priest", "High Priestess", 4000000},
     {"High Priest", "High Priestess", 4250000},
     {"High Priest", "High Priestess", 4500000},
     {"High Priest", "High Priestess", 4750000},
     {"High Priest", "High Priestess", 5000000},
     {"High Priest", "High Priestess", 5250000},
     {"High Priest", "High Priestess", 5500000},
     {"High Priest", "High Priestess", 5750000},
     {"High Priest", "High Priestess", 6000000},
     {"High Priest", "High Priestess", 6250000},
     {"High Priest", "High Priestess", 6500000},
     {"High Priest", "High Priestess", 6750000},
     {"High Priest", "High Priestess", 7000000},
     {"High Priest", "High Priestess", 7250000},
     {"High Priest", "High Priestess", 7500000},
     {"High Priest", "High Priestess", 7750000},
     {"High Priest", "High Priestess", 8000000},
     {"High Priest", "High Priestess", 8250000},
     {"High Priest", "High Priestess", 8500000},
     {"High Priest", "High Priestess", 8750000},
     {"High Priest", "High Priestess", 9000000},
     {"High Priest", "High Priestess", 9250000},
     {"High Priest", "High Priestess", 9500000},
     {"High Priest", "High Priestess", 9750000},
     {"High Priest", "High Priestess", 10000000},
     {"High Priest", "High Priestess", 25000000},
     {"Immortal Cardinal", "Immortal Priestess", 27000000},
     {"Immortal Cardinal", "Immortal Priestess", 29000000},
     {"Immortal Cardinal", "Immortal Priestess", 31000000},
     {"Immortal Cardinal", "Immortal Priestess", 33000000},
     {"Immortal Cardinal", "Immortal Priestess", 35000000},
     {"Immortal Cardinal", "Immortal Priestess", 40000000},
     {"Avatar of Nature", "Avatar of Nature", 45000000},
     {"Source of Nature", "Source of Nature", 50000000},
     {"Demigod", "Demigoddess", 60000000},
     {"God", "Goddess", 75000000},
     {"Bug", "Bug", 75000001}},
};

char *ClassTitles(struct char_data *ch)
{
    int i = 0;
    int count = 0;
    static char buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

    for (i = MAGE_LEVEL_IND; i <= DRUID_LEVEL_IND; i++)
    {
        if (GET_LEVEL(ch, i))
        {
            count++;
            if (count > 1)
            {
                sprintf(buf + strlen(buf), "/%s", GET_CLASS_TITLE(ch, i, GET_LEVEL(ch, i)));
            }
            else
            {
                sprintf(buf, "%s", GET_CLASS_TITLE(ch, i, GET_LEVEL(ch, i)));
            }
        }
    }
    return buf;
}

/* When age < 15 return the value p0 */
/* When age in 15..29 calculate the line between p1 & p2 */
/* When age in 30..44 calculate the line between p2 & p3 */
/* When age in 45..59 calculate the line between p3 & p4 */
/* When age in 60..79 calculate the line between p4 & p5 */
/* When age >= 80 return the value p6 */
int graf(int char_age, int p0, int p1, int p2, int p3, int p4, int p5, int p6)
{
    if (DEBUG > 2)
        log_info("called %s with %d, %d, %d, %d, %d, %d, %d, %d", __PRETTY_FUNCTION__, char_age, p0, p1, p2, p3, p4, p5,
                 p6);

    if (char_age < 15)
        return p0; /* < 15 */
    else if (char_age <= 29)
        return (int)(p1 + (((char_age - 15) * (p2 - p1)) / 15)); /* 15..29 */
    else if (char_age <= 44)
        return (int)(p2 + (((char_age - 30) * (p3 - p2)) / 15)); /* 30..44 */
    else if (char_age <= 59)
        return (int)(p3 + (((char_age - 45) * (p4 - p3)) / 15)); /* 45..59 */
    else if (char_age <= 79)
        return (int)(p4 + (((char_age - 60) * (p5 - p4)) / 20)); /* 60..79 */
    else
        return p6; /* >= 80 */
}

/* The three MAX functions define a characters Effective maximum */
/* Which is NOT the same as the ch->points.max_xxxx !!!          */
int mana_limit(struct char_data *ch)
{
    int maximum = 100;
    int extra = 0;
    int cl = 0;

    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

    if (IS_PC(ch))
    {
        if (HasClass(ch, CLASS_MAGIC_USER))
            extra += GET_LEVEL(ch, MAGE_LEVEL_IND) * 5;
        if (HasClass(ch, CLASS_CLERIC))
            extra += GET_LEVEL(ch, CLERIC_LEVEL_IND) * 4;
        if (HasClass(ch, CLASS_DRUID))
            extra += GET_LEVEL(ch, DRUID_LEVEL_IND) * 3;
        if (HasClass(ch, CLASS_RANGER))
            extra += (GET_LEVEL(ch, RANGER_LEVEL_IND) * 5) / 2;
        if (HasClass(ch, CLASS_THIEF))
            extra += (GET_LEVEL(ch, THIEF_LEVEL_IND) * 4) / 2;
        if (HasClass(ch, CLASS_WARRIOR))
            extra += (GET_LEVEL(ch, WARRIOR_LEVEL_IND) * 3) / 2;
        cl = HowManyClasses(ch);
        if ((cl = HowManyClasses(ch)) > 1)
            extra = ((extra * 10) / ((cl * 10) + 5));
        maximum += extra;
    }
    maximum += ch->points.max_mana; /* bonus mana */
    return maximum;
}

int hit_limit(struct char_data *ch)
{
    int maximum = 0;

    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

    if (IS_PC(ch))
        maximum = (ch->points.max_hit) + (graf(age(ch).year, 2, 4, 17, 14, 8, 4, 3));
    else
        maximum = (ch->points.max_hit);

    /*
     * Class/Level calculations
     */
    if (HowManyClasses(ch) == 1)
    {
        if (HasClass(ch, CLASS_RANGER))
            maximum += (GET_LEVEL(ch, RANGER_LEVEL_IND) / 5) + 2;
        else if (HasClass(ch, CLASS_WARRIOR))
            maximum += (GET_LEVEL(ch, WARRIOR_LEVEL_IND) / 2) + 1;
    }
    /*
     * Skill/Spell calculations
     */

    return maximum;
}

int move_limit(struct char_data *ch)
{
    int maximum = 0;

    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

    if (!IS_NPC(ch))
        maximum = 70 + age(ch).year + (int)GET_CON(ch) + GetTotLevel(ch);
    else
        maximum = ch->points.max_move;

    switch (GET_RACE(ch))
    {
    case RACE_DWARF:
        maximum -= 15;
        break;
    case RACE_GNOME:
        maximum -= 10;
        break;
    case RACE_HALFLING:
        maximum += 5;
        break;
    case RACE_ELVEN:
        maximum += 10;
        break;
    }
    return maximum;
}

int mana_gain(struct char_data *ch)
{
    int gain = 0;

    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

    if (IS_NPC(ch))
    {
        /*
         * Neat and fast
         */
        gain = GetTotLevel(ch);
    }
    else
    {
        gain = graf(age(ch).year, 2, 4, 6, 8, 10, 12, 16);

        if (GET_RACE(ch) == RACE_ELVEN)
            gain += 5;
        if (GET_RACE(ch) == RACE_GNOME)
            gain += 2;

        /*
         * Class calculations
         */
        if (HasClass(ch, CLASS_MAGIC_USER))
            gain += 2;
        if (HasClass(ch, CLASS_CLERIC))
            gain += 2;
        if (HasClass(ch, CLASS_DRUID))
            gain += 1;

        /*
         * Skill/Spell calculations
         */
        /*
         * Position calculations
         */

        switch (GET_POS(ch))
        {
        case POSITION_SLEEPING:
            gain += gain;
            break;
        case POSITION_RESTING:
            gain += (gain >> 1); /* Divide by 2 */
            break;
        case POSITION_SITTING:
            gain += (gain >> 2); /* Divide by 4 */
            break;
        }

        if (HasClass(ch, CLASS_MAGIC_USER) || HasClass(ch, CLASS_CLERIC) || HasClass(ch, CLASS_DRUID))
            gain += gain;
    }

    if (number(1, 101) < ch->skills[SKILL_MEDITATION].learned)
        gain += 10;

    if (IS_AFFECTED(ch, AFF_POISON))
        gain >>= 2;

    if (GET_COND(ch, FULL) < 2) /* starving */
        gain >>= 2;

    if (GET_COND(ch, THIRST) < 2) /* parched */
        gain >>= 2;

    return gain;
}

int hit_gain(struct char_data *ch)
{
    int gain = 0;

    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

    if (IS_NPC(ch))
    {
        gain = 8;
    }
    else
    {
        if (GET_POS(ch) == POSITION_FIGHTING)
        {
            gain = 1;
        }
        else
        {
            gain = graf(age(ch).year, 2, 5, 10, 18, 6, 4, 2);
        }

        /*
         * Class/Level calculations
         */
        if (HasClass(ch, CLASS_MAGIC_USER))
            gain -= 2;
        if (HasClass(ch, CLASS_CLERIC))
            gain -= 1;
        if (HasClass(ch, CLASS_WARRIOR))
            gain += 3;
        if (HasClass(ch, CLASS_RANGER))
            gain += 2;

        /*
         * Skill/Spell calculations
         */

        /*
         * Position calculations
         */

        switch (GET_POS(ch))
        {
        case POSITION_SLEEPING:
            gain += gain >> 1;
            break;
        case POSITION_RESTING:
            gain += gain >> 2;
            break;
        case POSITION_SITTING:
            gain += gain >> 3;
            break;
        }

        if (GET_POS(ch) == POSITION_SLEEPING)
            if (number(1, 101) < ch->skills[SKILL_MEDITATION].learned)
                gain += 3;
    }

    if (GET_RACE(ch) == RACE_DWARF)
        gain += 5;

    if (GET_RACE(ch) == RACE_HALFLING)
        gain += 2;

    if (GET_RACE(ch) == RACE_ELVEN)
        gain -= 1;

    if (GET_RACE(ch) == RACE_GNOME)
        gain += 1;

    if (IS_AFFECTED(ch, AFF_POISON))
    {
        gain >>= 2;
        /* damage(ch, ch, 15, SPELL_POISON); */
    }

    if (IS_PC(ch) && IS_STARVING(ch))
    {
        gain >>= 2;
        /* damage(ch, ch, number(1, 3), TYPE_HUNGER); */
    }

    if (IS_PC(ch) && IS_PARCHED(ch))
    {
        gain >>= 2;
        /* damage(ch, ch, number(1, 3), TYPE_HUNGER); */
    }

    gain = MAX(gain, 1);

    // if (IS_PC(ch)) cprintf(ch, "You should be gaining %d hit points.\r\n", gain);

    return gain;
}

/*
 * move gain pr. game hour
 */
int move_gain(struct char_data *ch)
{
    int gain = 0;

    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

    if (IS_NPC(ch))
        return GetTotLevel(ch);
    else
    {
        if (GET_POS(ch) != POSITION_FIGHTING)
            gain = 5 + (int)GET_CON(ch);
        else
        {
            if (number(1, 101) < ch->skills[SKILL_ENDURANCE].learned)
                gain = 2;
            else
                gain = 0;
        }

        if (HasClass(ch, CLASS_RANGER))
            gain += 3;
        if (HasClass(ch, CLASS_DRUID))
            gain += 2;

        /*
         * Position calculations
         */
        switch (GET_POS(ch))
        {
        case POSITION_SLEEPING:
            gain += (gain >> 1); /* Divide by 2 */
            break;
        case POSITION_RESTING:
            gain += (gain >> 2); /* Divide by 4 */
            break;
        case POSITION_SITTING:
            gain += (gain >> 3); /* Divide by 8 */
            break;
        }
    }

    if (GET_RACE(ch) == RACE_DWARF)
        gain += 4;
    if (GET_RACE(ch) == RACE_HALFLING)
        gain += 3;
    if (GET_RACE(ch) == RACE_ELVEN)
        gain += 1;

    if (number(1, 101) < ch->skills[SKILL_ENDURANCE].learned)
        gain += 5;

    if (IS_AFFECTED(ch, AFF_POISON))
        gain >>= 2;

    if (GET_COND(ch, FULL) < 2) /* starving */
        gain = 1;

    if (GET_COND(ch, THIRST) < 2) /* parched */
        gain = 1;

    return gain;
}

/* Gain maximum in various points */
void advance_level(struct char_data *ch, int chclass)
{
    int add_hp = 0;
    int i = 0;

    if (DEBUG > 1)
        log_info("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), chclass);

    if (GET_LEVEL(ch, chclass) > 0 &&
        GET_EXP(ch) <
#ifdef MOB_LEVELING
            ((IS_PC(ch) || (IS_SET(ch->specials.act, ACT_POLYSELF) || IS_SET(ch->specials.act, ACT_POLYSELF)))
                 ? titles[chclass][GET_LEVEL(ch, chclass) + 1].exp
                 : (titles[chclass][GET_LEVEL(ch, chclass) + 1].exp / 20))
#else
            titles[chclass][GET_LEVEL(ch, chclass) + 1].exp
#endif
    )
    {
        log_info("Bad advance_level");
        return;
    }
    GET_LEVEL(ch, chclass) += 1;

    /* Constitution Bonus only for Fighter types */

    if ((chclass == RANGER_LEVEL_IND) || (chclass == WARRIOR_LEVEL_IND))
        add_hp = con_app[(int)GET_CON(ch)].hitp;
    else
        add_hp = MIN(con_app[(int)GET_CON(ch)].hitp, 2);

    switch (chclass)
    {
    case MAGE_LEVEL_IND: {
        ch->specials.pracs += MAX(3, wis_app[(int)GET_INT(ch)].bonus);
        if (GET_LEVEL(ch, MAGE_LEVEL_IND) < 30)
            add_hp += number(2, 4);
        else
            add_hp += 1;
    }
    break;

    case DRUID_LEVEL_IND: {
        if (GET_LEVEL(ch, DRUID_LEVEL_IND) < 20)
            add_hp += number(2, 8);
        else
            add_hp += 3;
    }
    break;

    case CLERIC_LEVEL_IND: {
        ch->specials.pracs += MAX(3, wis_app[(int)GET_WIS(ch)].bonus);
        if (GET_LEVEL(ch, CLERIC_LEVEL_IND) < 30)
            add_hp += number(2, 8);
        else
            add_hp += 3;
    }
    break;

    case THIEF_LEVEL_IND: {
        ch->specials.pracs += MAX(3, wis_app[(int)GET_DEX(ch)].bonus);
        if (GET_LEVEL(ch, THIEF_LEVEL_IND) < 30)
            add_hp += number(2, 6);
        else
            add_hp += 2;
    }
    break;

    case RANGER_LEVEL_IND: {
        ch->specials.pracs +=
            MAX(3, wis_app[((int)GET_DEX(ch) >= (int)GET_STR(ch) ? (int)GET_DEX(ch) : (int)GET_STR(ch))].bonus);
        if (GET_LEVEL(ch, RANGER_LEVEL_IND) < 30)
            add_hp += number(2, 10);
        else
            add_hp += 4;
    }
    break;

    case WARRIOR_LEVEL_IND: {
        ch->specials.pracs += MAX(3, wis_app[(int)GET_STR(ch)].bonus);
        if (GET_LEVEL(ch, WARRIOR_LEVEL_IND) < 30)
            add_hp += number(2, 10);
        else
            add_hp += 4;
    }
    break;
    }

    add_hp /= HowManyClasses(ch);

    add_hp++;

    if (GET_LEVEL(ch, chclass) <= 5)
        add_hp++;

    ch->points.max_hit += MAX(1, add_hp);

    if (GetMaxLevel(ch) >= LOW_IMMORTAL)
        for (i = 0; i < 3; i++)
            ch->specials.conditions[i] = -1;

    ch->points.max_move = GET_MAX_MOVE(ch);
    //  if (IS_PC(ch))
    //    update_player_list_entry(ch->desc);
    log_info("%s advances to level %d.\r\n", GET_NAME(ch), GetMaxLevel(ch));
}

/* Lose in various points */
/*
 * ** Damn tricky for multi-class...
 */

void drop_level(struct char_data *ch, int chclass)
{
    int add_hp = 0;
    int lin_class = 0;
    int old_style = 0;

    if (DEBUG > 1)
        log_info("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), chclass);

    /*
     * if (GetMaxLevel(ch) >= LOW_IMMORTAL)
     *   return;
     */
    if (GetMaxLevel(ch) == 1)
        return;

    add_hp = con_app[(int)GET_CON(ch)].hitp;

    switch (chclass)
    {

    case CLASS_MAGIC_USER: {
        lin_class = MAGE_LEVEL_IND;
        if (GET_LEVEL(ch, MAGE_LEVEL_IND) < 30)
            add_hp += number(2, 4);
        else
            add_hp += 1;
    }
    break;

    case CLASS_DRUID: {
        lin_class = DRUID_LEVEL_IND;
        if (GET_LEVEL(ch, DRUID_LEVEL_IND) < 30)
            add_hp += number(2, 8);
        else
            add_hp += 3;
    }
    break;

    case CLASS_CLERIC: {
        lin_class = CLERIC_LEVEL_IND;
        if (GET_LEVEL(ch, CLERIC_LEVEL_IND) < 30)
            add_hp += number(2, 8);
        else
            add_hp += 3;
    }
    break;

    case CLASS_THIEF: {
        lin_class = THIEF_LEVEL_IND;
        if (GET_LEVEL(ch, THIEF_LEVEL_IND) < 30)
            add_hp += number(2, 6);
        else
            add_hp += 2;
    }
    break;

    case CLASS_WARRIOR: {
        lin_class = WARRIOR_LEVEL_IND;
        if (GET_LEVEL(ch, WARRIOR_LEVEL_IND) < 30)
            add_hp += number(2, 10);
        else
            add_hp += 4;
    }
    break;

    case CLASS_RANGER: {
        lin_class = RANGER_LEVEL_IND;
        if (GET_LEVEL(ch, RANGER_LEVEL_IND) < 30)
            add_hp += number(2, 10);
        else
            add_hp += 4;
    }
    break;
    }

    add_hp /= HowManyClasses(ch);

    if (IS_NPC(ch))
    {
        gain_exp(ch, -GET_EXP(ch) / 4);
    }
    else if (GetMaxLevel(ch) < 7)
    {
        gain_exp(ch, -GET_EXP(ch) / (12 * HowManyClasses(ch)));
    }
    else if (GetMaxLevel(ch) < 14)
    {
        gain_exp(ch, -GET_EXP(ch) / (10 * HowManyClasses(ch)));
    }
    else if (GetMaxLevel(ch) < 21)
    {
        gain_exp(ch, -GET_EXP(ch) / (8 * HowManyClasses(ch)));
    }
    else if (GetMaxLevel(ch) < 28)
    {
        gain_exp(ch, -GET_EXP(ch) / (6 * HowManyClasses(ch)));
    }
    else
        gain_exp(ch, -GET_EXP(ch) / (4 * HowManyClasses(ch)));

    GET_LEVEL(ch, chclass) -= 1;
    if (GET_LEVEL(ch, chclass) < 1)
        GET_LEVEL(ch, chclass) = 1;

    ch->points.max_hit -= MAX(1, add_hp);
    if (ch->points.max_hit < 1)
        ch->points.max_hit = 1;

    ch->specials.pracs -= MAX(3, wis_app[(int)GET_WIS(ch)].bonus);

    if (old_style)
    {
        ch->points.exp = MIN(titles[lin_class][(int)GET_LEVEL(ch, lin_class)].exp, GET_EXP(ch));
    }

    if (ch->points.exp < 0)
        ch->points.exp = 0;
    //  update_player_list_entry(ch->desc);
    log_info("%s drops to level %d.\r\n", GET_NAME(ch), GetMaxLevel(ch));
}

void set_title(struct char_data *ch)
{
    char buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG > 1)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

    sprintf(buf, "the %s %s", RaceName[ch->race], ClassTitles(ch));
    if (GET_TITLE(ch))
    {
        DESTROY(GET_TITLE(ch));
        CREATE(GET_TITLE(ch), char, strlen(buf) + 1);
    }
    else
    {
        CREATE(GET_TITLE(ch), char, strlen(buf) + 1);
    }
    strcpy(GET_TITLE(ch), buf);
}

void gain_exp(struct char_data *ch, int gain)
{
    int i = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), gain);

    /*  save_char(ch,NOWHERE); */

    if (!IS_IMMORTAL(ch))
    {
        if (gain > 0)
        {
            gain = MIN(100000, gain);

            if (IS_PC(ch) || (IS_SET(ch->specials.act, ACT_POLYSELF) || IS_SET(ch->specials.act, ACT_POLYSELF)))
            {
                gain /= HowManyClasses(ch);
            }
            else
            {
#ifdef MOB_LEVELING
                GET_EXP(ch) += gain;
                for (i = MAGE_LEVEL_IND; i <= DRUID_LEVEL_IND; i++)
                {
                    if (GET_LEVEL(ch, i))
                    {
                        if (GET_EXP(ch) >= ((titles[i][GET_LEVEL(ch, i) + 1].exp / 20)))
                        {
                            advance_level(ch, i);
                            act("$n seems to be looking more healthy today.", FALSE, ch, 0, 0, TO_ROOM);
                        }
                    }
                }
                return;
#endif
            }

            if (IS_PC(ch) && (GetMaxLevel(ch) == 1))
                gain *= 2;

            if (IS_PC(ch) || (IS_SET(ch->specials.act, ACT_POLYSELF) || IS_SET(ch->specials.act, ACT_POLYSELF)))
            {
                for (i = MAGE_LEVEL_IND; i <= DRUID_LEVEL_IND; i++)
                {
                    if (GET_LEVEL(ch, i))
                    {
                        if (GET_EXP(ch) >= titles[i][GET_LEVEL(ch, i) + 2].exp)
                        {
                            cprintf(ch, "You will not gain anymore exp until you practice at a guild.\r\n");
                            GET_EXP(ch) = titles[i][GET_LEVEL(ch, i) + 2].exp - 1;
                            return;
                        }
                        else if (GET_EXP(ch) >= titles[i][GET_LEVEL(ch, i) + 1].exp)
                        {
                            /*
                             * do nothing..this is cool
                             */
                        }
                        else if (GET_EXP(ch) + gain >= titles[i][GET_LEVEL(ch, i) + 1].exp)
                        {
                            cprintf(ch, "You have gained enough to be a(n) %s\r\n",
                                    GET_CLASS_TITLE(ch, i, GET_LEVEL(ch, i) + 1));
                            cprintf(ch, "You will not gain anymore exp until you practice at a guild.\r\n");
                            if (GET_EXP(ch) + gain >= titles[i][GET_LEVEL(ch, i) + 2].exp)
                            {
                                GET_EXP(ch) = titles[i][GET_LEVEL(ch, i) + 2].exp - 1;
                                return;
                            }
                        }
                    }
                }
            }
            GET_EXP(ch) += gain;
            if (IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF))
            {
                for (i = MAGE_LEVEL_IND; i <= DRUID_LEVEL_IND; i++)
                {
                    if (GET_LEVEL(ch, i))
                    {
                        if (GET_EXP(ch) > titles[i][GET_LEVEL(ch, i) + 2].exp)
                        {
                            GET_EXP(ch) = titles[i][GET_LEVEL(ch, i) + 2].exp - 1;
                        }
                    }
                }
            }
        }
        if (gain < 0)
        {
            GET_EXP(ch) += gain;
            if (GET_EXP(ch) < 0)
                GET_EXP(ch) = 0;
        }
    }
}

void gain_exp_regardless(struct char_data *ch, int gain, int chclass)
{
    int i = 0;
    int is_altered = FALSE;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), gain, chclass);

    save_char(ch, NOWHERE);
    if (!IS_NPC(ch))
    {
        if (gain > 0)
        {
            GET_EXP(ch) += gain;

            for (i = 0; (i < ABS_MAX_LVL) && (titles[chclass][i].exp <= GET_EXP(ch)); i++)
            {
                if (i > GET_LEVEL(ch, chclass))
                {
                    cprintf(ch, "You raise a level\r\n");
                    /*        GET_LEVEL(ch,chclass) = i; */
                    advance_level(ch, chclass);
                    is_altered = TRUE;
                }
            }
        }
        if (gain < 0)
            GET_EXP(ch) += gain;

        if (GET_EXP(ch) < 0)
            GET_EXP(ch) = 0;
    }
    if (is_altered)
        set_title(ch);
}

void gain_condition(struct char_data *ch, int condition, int value)
{
    int intoxicated = FALSE;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), condition, value);

    if (GET_COND(ch, condition) == -1) /* No change */
        return;

    intoxicated = (GET_COND(ch, DRUNK) > 0);

    GET_COND(ch, condition) += value;

    GET_COND(ch, condition) = MAX(0, GET_COND(ch, condition));
    GET_COND(ch, condition) = MIN(24, GET_COND(ch, condition));

    /*
     * if (GET_COND(ch, condition)) return;
     */

    switch (condition)
    {
    case FULL:
        if (IS_GETTING_HUNGRY(ch))
        {
            if (IS_HUNGRY(ch))
            {
                if (IS_STARVING(ch))
                {
                    cprintf(ch, "You are starving!\r\n");
                }
                else
                {
                    cprintf(ch, "You are hungry.\r\n");
                }
            }
            else
            {
                cprintf(ch, "You are getting hungry.\r\n");
            }
        }

        break;
    case THIRST:
        if (IS_GETTING_THIRSTY(ch))
        {
            if (IS_THIRSTY(ch))
            {
                if (IS_PARCHED(ch))
                {
                    cprintf(ch, "You are parched!\r\n");
                }
                else
                {
                    cprintf(ch, "You are thirsty.\r\n");
                }
            }
            else
            {
                cprintf(ch, "You are getting thirsty.\r\n");
            }
        }
        break;
    case DRUNK:
        if (IS_HOPELESSLY_DRUNK(ch))
        {
            cprintf(ch, "You are homelessly DRUNK!\r\n");
        }
        else if (intoxicated && GET_COND(ch, DRUNK) < 1)
        {
            cprintf(ch, "You are now sober.\r\n");
        }
        break;
    default:
        break;
    }
}

void check_idling(struct char_data *ch)
{
#ifdef BOOT_IDLE
    struct obj_cost cost;
#endif

    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

    ++(ch->specials.timer);

    /*
     * if (ch->specials.timer > 5 && ch->specials.timer < 10) {
     */
    if (!ch->specials.timer % 5)
    {
        do_save(ch, "", 0);
        return;
    }
#ifdef BOOT_IDLE
    if (ch->specials.timer >= 15)
    {
        log_info("LOG:%s AUTOSAVE:Timer %d.", GET_NAME(ch), ch->specials.timer);

        if (ch->specials.fighting)
        {
            stop_fighting(ch->specials.fighting);
            stop_fighting(ch);
        }
        GET_POS(ch) = POSITION_STANDING;
        char_from_room(ch);
        char_to_room(ch, 0);
        if (IS_IMMORTAL(ch))
            GET_HOME(ch) = 1000;
        else
            GET_HOME(ch) = 3008;

        if (recep_offer(ch, NULL, &cost))
        {
            cost.total_cost = 0;
            new_save_equipment(ch, &cost, FALSE);
            save_obj(ch, &cost, TRUE);
        }
        extract_char(ch);

        if (ch->desc)
            close_socket(ch->desc);

        /*
         * ch->desc = 0; already done inside close_socket, thanks valgrind!
         */

        log_info("Done Auto-Saving.");
    }
#endif
}

/* Update both PC's & NPC's and objects */
void point_update(int current_pulse)
{
    struct char_data *i = NULL;
    struct char_data *next_dude = NULL;
    struct obj_data *j = NULL;
    struct obj_data *next_thing = NULL;
    int count = 0;

    if (DEBUG > 2)
        log_info("called %s with %d", __PRETTY_FUNCTION__, current_pulse);

    /*
     * characters
     */
    for (i = character_list; i; i = next_dude)
    {
        next_dude = i->next;
        /* So, the first thing we need to do is apply damage() effects
         * so that poison, hunger, thirst, or other spells can do their
         * periodic damage BEFORE all the calculations below happen.
         */
        if (IS_AFFECTED(i, AFF_POISON))
        {
            damage(i, i, 15, SPELL_POISON);
        }

        if (IS_PC(i) && IS_STARVING(i))
        {
            damage(i, i, number(1, 3), TYPE_HUNGER);
        }

        if (IS_PC(i) && IS_PARCHED(i))
        {
            damage(i, i, number(1, 3), TYPE_HUNGER);
        }

        if (GET_POS(i) >= POSITION_STUNNED)
        {
            if (!affected_by_spell(i, SPELL_AID))
            {
                /* A problem here... damage() calls within hit_gain() are being
                 * overridden, because this code calls GET_HIT() before the call to
                 * hit_gain(), and thus no damage can happen.
                 *
                 * Basically, the damage() calls for spells/poison/hunger need
                 * to be moved in here, so they happen BEFORE hit_gain() is
                 * calculated.
                 */
                GET_HIT(i) = MIN(GET_HIT(i) + hit_gain(i), hit_limit(i));
            }
            else
            {
                if (GET_HIT(i) < hit_limit(i))
                {
                    GET_HIT(i) = MIN(GET_HIT(i) + hit_gain(i), hit_limit(i));
                }
            }

            GET_MANA(i) = MIN(GET_MANA(i) + mana_gain(i), mana_limit(i));
            GET_MOVE(i) = MIN(GET_MOVE(i) + move_gain(i), move_limit(i));

            // if (GET_POS(i) == POSITION_STUNNED)
            // update_pos(i);
        }
        else if (GET_POS(i) == POSITION_INCAP)
        {
            /*
             * damage(i, i, 0, TYPE_SUFFERING);
             */
            GET_HIT(i) += 1;
            // update_pos(i);
        }
        else if (IS_PC(i) && (GET_POS(i) == POSITION_MORTALLYW))
        {
            damage(i, i, 1, TYPE_SUFFERING);
        }
        update_pos(i);

        if (!IS_NPC(i))
        {
            update_char_objects(i);
            if (GetMaxLevel(i) < CREATOR)
                check_idling(i);
        }

        if (GET_POS(i) != POSITION_DEAD)
        {
            gain_condition(i, FULL, -1);
            gain_condition(i, DRUNK, -1);
            gain_condition(i, THIRST, -1);
        }
    }

    /*
     * objects
     */
    for (j = object_list; j; j = next_thing)
    {
        next_thing = j->next;
        count++;

        if ((GET_ITEM_TYPE(j) == ITEM_FOOD) && IS_OBJ_STAT(j, ITEM_PARISH))
        {
            if (j->obj_flags.value[0] > 0)
                j->obj_flags.value[0]--;

            switch (j->obj_flags.value[0])
            {
            case 3: {
                if (j->carried_by)
                    act("$p begins to look a little brown.", FALSE, j->carried_by, j, 0, TO_CHAR);
                else if (j->in_room != NOWHERE && (real_roomp(j->in_room)->people))
                {
                    act("$p begins to look a little brown.", TRUE, real_roomp(j->in_room)->people, j, 0, TO_CHAR);
                    act("$p begins to look a little brown.", TRUE, real_roomp(j->in_room)->people, j, 0, TO_ROOM);
                }
            }
            break;
            case 2: {
                if (j->carried_by)
                    act("$p begins to smell funny.", FALSE, j->carried_by, j, 0, TO_CHAR);
                else if (j->in_room != NOWHERE && (real_roomp(j->in_room)->people))
                {
                    act("$p begins to smell funny.", TRUE, real_roomp(j->in_room)->people, j, 0, TO_CHAR);
                    act("$p begins to smell funny.", TRUE, real_roomp(j->in_room)->people, j, 0, TO_ROOM);
                }
            }
            break;
            case 1: {
                j->obj_flags.value[3] = 1; /* poison the sucker */
                if (j->carried_by)
                    act("$p begins to smell spoiled.", FALSE, j->carried_by, j, 0, TO_CHAR);
                else if (j->in_room != NOWHERE && (real_roomp(j->in_room)->people))
                {
                    act("$p begins to smell spoiled.", TRUE, real_roomp(j->in_room)->people, j, 0, TO_CHAR);
                    act("$p begins to smell spoiled.", TRUE, real_roomp(j->in_room)->people, j, 0, TO_ROOM);
                }
            }
            break;
            case 0: {
                if (j->carried_by)
                    act("$p dissolves into dust...", FALSE, j->carried_by, j, 0, TO_CHAR);
                else if ((j->in_room != NOWHERE) && (real_roomp(j->in_room)->people))
                {
                    act("$p dissolves into dust...", TRUE, real_roomp(j->in_room)->people, j, 0, TO_ROOM);
                    act("$p dissolves into dust...", TRUE, real_roomp(j->in_room)->people, j, 0, TO_CHAR);
                }
                extract_obj(j);
            }
            }
        }
        if ((GET_ITEM_TYPE(j) == ITEM_CONTAINER) && (j->obj_flags.value[3]))
        {
            if (j->obj_flags.timer > 0)
                j->obj_flags.timer--;
            if (!j->obj_flags.timer)
            {
                if (j->carried_by)
                {
                    act("$p biodegrades in your hands.", FALSE, j->carried_by, j, 0, TO_CHAR);
                    ObjFromCorpse(j);
                }
                else if (j->in_room != NOWHERE)
                {
                    if ((number(0, 99) < 40) && (real_roomp(j->in_room)->zone != 10) &&
                        (real_roomp(j->in_room)->zone != 11))
                    {
                        struct char_data *mob;
                        struct obj_data *obj_object, *next_obj;
                        int mobset[] = {9002, 9001, 4616, 4615, 4613, 9003, 4603};
                        char newbuffer[MAX_INPUT_LENGTH], newtmp[MAX_INPUT_LENGTH];

                        mob = read_mobile(mobset[number(0, 6)], VIRTUAL);
                        strcpy(newtmp, j->short_description);
                        if (!strncmp(newtmp, "the corpse of ", 14))
                        {
                            newtmp[14] = tolower(newtmp[14]);
                            snprintf(newbuffer, MAX_INPUT_LENGTH, "%s of %s has arisen to KILL!\r\n", GET_SDESC(mob), &newtmp[14]);
                        }
                        else
                        {
                            sprintf(newbuffer, "%s has recently risen to KILL!\r\n", GET_SDESC(mob));
                        }
                        /*
                         * this loses memory... can we free it first?
                         */
                        if (mob->player.long_descr)
                            DESTROY(mob->player.long_descr);
                        mob->player.long_descr = strdup(newbuffer);
                        char_to_room(mob, j->in_room);
                        GET_EXP(mob) = 75 * GetMaxLevel(mob);
                        IS_CARRYING_W(mob) = 0;
                        IS_CARRYING_N(mob) = 0;
                        for (obj_object = j->contains; obj_object; obj_object = next_obj)
                        {
                            next_obj = obj_object->next_content;
                            obj_from_obj(obj_object);
                            obj_to_char(obj_object, mob);
                        }
                        mob->points.max_hit = dice(GetMaxLevel(mob), 8) + number(20, 10 * GetMaxLevel(mob));
                        GET_HIT(mob) = GET_MAX_HIT(mob);
                        GET_EXP(mob) = dice(GetMaxLevel(mob), 10) * 10 + number(1, 10 * GetMaxLevel(mob));
                        mob->player.sex = 0;
                        GET_RACE(mob) = RACE_UNDEAD;
                        if (!IS_SET(mob->specials.act, ACT_AGGRESSIVE))
                        {
                            SET_BIT(mob->specials.act, ACT_AGGRESSIVE);
                        }
                        if (IS_SET(mob->specials.act, ACT_SENTINEL))
                        {
                            REMOVE_BIT(mob->specials.act, ACT_SENTINEL);
                        }
                        if (real_roomp(j->in_room)->people)
                        {
                            act("$p slowly rises as $N and screams 'DIE!'", TRUE, real_roomp(j->in_room)->people, j,
                                mob, TO_ROOM);
                            act("$p slowly rises as $N and screams 'DIE!'", TRUE, real_roomp(j->in_room)->people, j,
                                mob, TO_CHAR);
                        }
                        do_wear(mob, "all", 0);
                        extract_obj(j);
                    }
                    else if (real_roomp(j->in_room)->people)
                    {
                        act("$p dissolves into a lump of fertile soil.", TRUE, real_roomp(j->in_room)->people, j, 0,
                            TO_ROOM);
                        act("$p dissolves into a lump of fertile soil.", TRUE, real_roomp(j->in_room)->people, j, 0,
                            TO_CHAR);
                        ObjFromCorpse(j);
                    }
                    else
                        ObjFromCorpse(j);
                }
            }
        }
    }
}

int ObjFromCorpse(struct obj_data *c)
{
    struct obj_data *jj = NULL;
    struct obj_data *next_thing = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_ONAME(c));

    for (jj = c->contains; jj; jj = next_thing)
    {
        next_thing = jj->next_content; /* Next in inventory */
        if (jj->in_obj)
        {
            obj_from_obj(jj);
            if (c->in_obj)
                obj_to_obj(jj, c->in_obj);
            else if (c->carried_by)
                obj_to_room(jj, c->carried_by->in_room);
            else if (c->in_room != NOWHERE)
                obj_to_room(jj, c->in_room);
            else
                return FALSE;
        }
        else
        {
            /*
             * **  hmm..  it isn't in the object it says it is in.
             * **  deal with the memory lossage
             */
            c->contains = 0;
            extract_obj(c);
            log_error("Memory lost in ObjFromCorpse.");
            return TRUE;
        }
    }
    extract_obj(c);
    return TRUE;
}

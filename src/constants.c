/*
 * file: constants.c                                      Part of DIKUMUD
 * Usage: For constants used by the game.
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "global.h"
#include "bug.h"
#include "limits.h"
#include "trap.h"
#define _CONSTANTS_C
#include "constants.h"

const char *class_name[] =
{
  "mage", "cleric", "warrior", "thief", "ranger", "druid", NULL
};

const char *exp_needed_text[] =
{
  "YES!  One more bunny should just about do it!!!"	/* < 1% left to go */
  "Your new level should arrive aaaany second now!",	/* 1..10% */
  "You can almost TASTE that level!",	/* 10 */
"You think you catch a glimpse of a level just over the next hill.",	/* 20 */
"You should be crusing now... That level will get here in no time!",	/* 30 */
  "You're halfway to the next level!  HOORAY!",		/* > 40% */
  "You are almost at the halfway point.",	/* > 50% */
  "You are making good progress, but the way is still far away.",	/* > 60% */
  "You're finally getting somewhere, but a level is still a dream.",	/* > 70% */
  "You've got a LONG way to go for another level.",	/* > 80% */
  "It seems like you'll NEVER get a level at this rate!",	/* > 90% */
  "You think you'll die of old age before you get a level...",	/* > 100% */
};

const char *percent_hit[] =
{
  "massacred",
  "slaughtered",
  "bloodied",
  "beaten",
  "wounded",
  "hurt",
  "bruised",
  "scratched",
  "fine",
  "fine",
  "excellent"
};

const char *percent_tired[] =
{
  "exhausted",
  "beat",
  "tired",
  "weary",
  "haggard",
  "fatigued",
  "worked",
  "winded",
  "rested",
  "fresh",
  "fresh"
};

const char *spell_wear_off_msg[] =
{
  "RESERVED DB.C",
  "You feel less proctected.",
  "!Teleport!",
  "You feel less righteous.",
  "You feel a cloak of blindness disolve.",
  "!Burning Hands!",
  "!Call Lightning",
  "You feel in control of youself.",
  "You feel the spell chill touch leave you...",
  "!Clone!",
  "!Color Spray!",
  "!Control Weather!",
  "!Create Food!",
  "!Create Water!",
  "!Cure Blind!",
  "!Cure Critic!",
  "!Cure Light!",
  "You feel better now.",
  "You sense the red in your vision disappear.",
  "The detect invisible wears off.",
  "The detect magic wears off.",
  "The detect poison wears off.",
  "!Dispel Evil!",
  "!Earthquake!",
  "!Enchant Weapon!",
  "!Energy Drain!",
  "!Fireball!",
  "!Harm!",
  "!Heal",
  "You feel exposed.",
  "!Lightning Bolt!",
  "!Locate object!",
  "!Magic Missile!",
  "You feel less sick.",
  "You feel less protected.",
  "!Remove Curse!",
  "The white aura around your body fades.",
  "!Shocking Grasp!",
  "You feel less tired.",
  "You don't feel as strong.",
  "!Summon!",
  "!Ventriloquate!",
  "!Word of Recall!",
  "!Remove Poison!",
  "You feel less aware of your suroundings.",
  "",				       /* NO MESSAGE FOR SNEAK */
  "!Hide!",
  "!Steal!",
  "!Backstab!",
  "!Pick Lock!",
  "!Kick!",
  "!Bash!",
  "!Rescue!",
  "!Identify!",
  "You feel disoriented as you lose your infravision.",
  "!cause light!",
  "!cause crit!",
  "!flamestrike!",
  "!dispel good!",
  "You feel somewhat stronger now...",
  "!dispell magic",
  "!knock!",
  "!know alignment!",
  "!animate dead!",
  "You feel freedom of movement.",
  "!remove paralysis!",
  "!fear!",
  "!acid blast!",
  "You feel a tightness at your throat. ",
  "You feel heavier now, your flying ability is leaving you.",
  "!cone of cold.",
  "!meteor swarm.",
  "!ice storm.",
  "Your shield of force dissapates.",
  "monsum one, please report.",
  "monsum two, please report.",
  "monsum 3, please report.",
  "monsum 4, please report.",
  "monsum 5, please report.",
  "monsum 6, please report.",
  "monsum 7, please report.",
  "The red glow around your body fades",
  "charm monster, please report.",
  "cure serious, please report.",
  "cause serious, please report.",
  "refresh, please report.",
  "second wind, please report.",
  "turn, please report.",
  "succor, please report.",
  "create light, please report.",
  "cont light, please report.",
  "calm, please report.",
  "Your skin returns to normal.",
  "conjure elemental, please report.",
  "Your clarity of vision dissapears",
  "minor creation, please report",
  "The pink glow around your body fades.",
  "!faerie fog",
  "!cacaodemon",
  "You return to your normal state",
  "!mana",
  "!astral walk",
  "You feel heavier now, your flying ability is leaving you.",
  "You feel less righteous and not as healthy.",
};

const char *spell_wear_off_soon_msg[] =
{
  "RESERVED DB.C",
  "You begin to feel less proctected.",
  "!Teleport!",
  "You begin to feel less righteous.",
  "You begin to see shades of gray.",
  "!Burning Hands!",
  "!Call Lightning",
  "You feel your will coming back.",
  "You begin to feel warmth creep through your bones....",
  "!Clone!",
  "!Color Spray!",
  "!Control Weather!",
  "!Create Food!",
  "!Create Water!",
  "!Cure Blind!",
  "!Cure Critic!",
  "!Cure Light!",
  "You begin to feel better now.",
  "You sense a slight change in your vision.",
  "You sense a slight change in your vision.",
  "You sense a slight change in your vision.",
  "You sense a slight change in your vision.",
  "!Dispel Evil!",
  "!Earthquake!",
  "!Enchant Weapon!",
  "!Energy Drain!",
  "!Fireball!",
  "!Harm!",
  "!Heal",
  "You sense your body becoming opaque.",
  "!Lightning Bolt!",
  "!Locate object!",
  "!Magic Missile!",
  "You begin to feel less sick.",
  "You begin to feel less protected.",
  "!Remove Curse!",
  "The white aura around your body shimmers.",
  "!Shocking Grasp!",
  "You begin to feel less tired.",
  "Your muscles begin to ache.",
  "!Summon!",
  "!Ventriloquate!",
  "!Word of Recall!",
  "!Remove Poison!",
  "You begin to feel less aware of your suroundings.",
  "",				       /* NO MESSAGE FOR SNEAK */
  "!Hide!",
  "!Steal!",
  "!Backstab!",
  "!Pick Lock!",
  "!Kick!",
  "!Bash!",
  "!Rescue!",
  "!Identify!",
  "You sense a slight change in your vision.",
  "!cause light!",
  "!cause crit!",
  "!flamestrike!",
  "!dispel good!",
  "You begin to feel your strength coming back....",
  "!dispell magic",
  "!knock!",
  "!know alignment!",
  "!animate dead!",
  "You begin to feel freedom of movement.",
  "!remove paralysis!",
  "!fear!",
  "!acid blast!",
  "You begin to feel a tightness at your throat. ",
  "You begin to feel the pull of gravity on you.",
  "!cone of cold.",
  "!meteor swarm.",
  "!ice storm.",
  "The shield in front of you wavers slightly....",
  "monsum one, please report.",
  "monsum two, please report.",
  "monsum 3, please report.",
  "monsum 4, please report.",
  "monsum 5, please report.",
  "monsum 6, please report.",
  "monsum 7, please report.",
  "The red glow around your body fades slightly.",
  "charm monster, please report.",
  "cure serious, please report.",
  "cause serious, please report.",
  "refresh, please report.",
  "second wind, please report.",
  "turn, please report.",
  "succor, please report.",
  "create light, please report.",
  "cont light, please report.",
  "calm, please report.",
  "Your skin changes in hue for a second....",
  "conjure elemental, please report.",
  "Your eyes cloud somewhat for a second...",
  "minor creation, please report",
  "The pink glow around your body fades slightly.",
  "!faerie fog",
  "!cacaodemon",
  "You are beginning to return to your normal state",
  "!mana",
  "!astral walk",
  "You begin to feel the pull of gravity on you.",
  "You begin to feel less righteous and not as healthy.",
};

const int rev_dir[] =
{
  2,
  3,
  0,
  1,
  5,
  4
};

const int TrapDir[] =
{
  TRAP_EFF_NORTH,
  TRAP_EFF_EAST,
  TRAP_EFF_SOUTH,
  TRAP_EFF_WEST,
  TRAP_EFF_UP,
  TRAP_EFF_DOWN
};

const int movement_loss[] =
{
  1,				       /* Inside     */
  2,				       /* City       */
  2,				       /* Field      */
  3,				       /* Forest     */
  4,				       /* Hills      */
  6,				       /* Mountains  */
  8,				       /* Swimming   */
  10,				       /* Unswimable */
  2,				       /* Flying     */
  20				       /* Submarine  */
};

const char *dirs[] =
{
  "north",
  "east",
  "south",
  "west",
  "up",
  "down",
  "\n"
};

const char *dir_from[] =
{
  "from the north",
  "from the east",
  "from the south",
  "from the west",
  "from above",
  "from below",
  "\n"
};

const char *ItemDamType[] =
{
  "burned",
  "frozen",
  "electrified",
  "crushed",
  "corroded"
};

const char *weekdays[7] =
{
  "the Day of the Moon",
  "the Day of the Bull",
  "the Day of the Deception",
  "the Day of Thunder",
  "the Day of Freedom",
  "the day of the Great Gods",
  "the Day of the Sun"};

const char *month_name[17] =
{
  "Month of Winter",		       /* 0 */
  "Month of the Winter Wiley Wolf",
  "Month of the Frost Giant",
  "Month of the Old Forces",
  "Month of the Grand Struggle",
  "Month of the Spring",
  "Month of Nature",
  "Month of Futility",
  "Month of the Dragon",
  "Month of the Sun",
  "Month of the Heat",
  "Month of the Battle",
  "Month of the Dark Shades",
  "Month of the Shadows",
  "Month of the Long Shadows",
  "Month of the Ancient Darkness",
  "Month of the Great Evil"
};

const int sharp[] =
{
  0,
  0,
  0,
  1,				       /* Slashing */
  0,
  0,
  0,
  0,				       /* Bludgeon */
  0,
  0,
  0,
  0};				       /* Pierce   */

const char *where[] =
{
  "<used as light>      ",
  "<worn on finger>     ",
  "<worn on finger>     ",
  "<worn around neck>   ",
  "<worn around neck>   ",
  "<worn on body>       ",
  "<worn on head>       ",
  "<worn on legs>       ",
  "<worn on feet>       ",
  "<worn on hands>      ",
  "<worn on arms>       ",
  "<worn as shield>     ",
  "<worn about body>    ",
  "<worn about waist>   ",
  "<worn around wrist>  ",
  "<worn around wrist>  ",
  "<wielded>            ",
  "<held>               ",
  "<wielded two-handed> ",
};

const char *drinks[] =
{
  "water",
  "beer",
  "wine",
  "ale",
  "dark ale",
  "whisky",
  "lemonade",
  "firebreather",
  "local speciality",
  "slime mold juice",
  "milk",
  "tea",
  "coffee",
  "blood",
  "salt water",
  "coca cola",
  "\n"
};

const char *drinknames[] =
{
  "water",
  "beer",
  "wine",
  "ale",
  "ale",
  "whisky",
  "lemonade",
  "firebreather",
  "local",
  "juice",
  "milk",
  "tea",
  "coffee",
  "blood",
  "salt",
  "cola",
  "\n"
};

const int RacialMax[][4] =
{

  {(LOW_IMMORTAL - 1), (LOW_IMMORTAL - 1), (LOW_IMMORTAL - 1), (LOW_IMMORTAL - 1)},
  {(LOW_IMMORTAL - 1), (LOW_IMMORTAL - 1), (LOW_IMMORTAL - 1), (LOW_IMMORTAL - 1)},
  {(LOW_IMMORTAL - 1), (LOW_IMMORTAL - 1), (LOW_IMMORTAL - 1), (LOW_IMMORTAL - 1)},
  {(LOW_IMMORTAL - 1), (LOW_IMMORTAL - 1), (LOW_IMMORTAL - 1), (LOW_IMMORTAL - 1)},
  {(LOW_IMMORTAL - 1), (LOW_IMMORTAL - 1), (LOW_IMMORTAL - 1), (LOW_IMMORTAL - 1)},
  {(LOW_IMMORTAL - 1), (LOW_IMMORTAL - 1), (LOW_IMMORTAL - 1), (LOW_IMMORTAL - 1)}
};

/*  fire cold elec blow acid */

int ItemSaveThrows[22][5] =
{
  {15, 2, 10, 10, 10},
  {19, 2, 16, 2, 7},
  {11, 2, 2, 13, 9},
  {7, 2, 2, 10, 8},
  {6, 2, 2, 7, 13},
  {10, 10, 10, 10, 10},		       /* not defined */
  {10, 10, 10, 10, 10},		       /* not defined */
  {6, 2, 2, 7, 13},		       /* treasure */
  {6, 2, 2, 7, 13},		       /* armor */
  {7, 6, 2, 20, 5},		       /* potion */
  {10, 10, 10, 10, 10},		       /* not defined */
  {10, 10, 10, 10, 10},		       /* not defined */
  {10, 10, 10, 10, 10},		       /* not defined */
  {10, 10, 10, 10, 10},		       /* not defined */
  {19, 2, 2, 16, 7},
  {7, 6, 2, 20, 5},		       /* drinkcon */
  {6, 2, 2, 7, 13},
  {6, 3, 2, 3, 10},
  {6, 2, 2, 7, 13},		       /* treasure */
  {11, 2, 2, 13, 9},
  {7, 2, 2, 10, 8}
};

const int drink_aff[][3] =
{
  {0, 1, 10},			       /* Water    */
  {3, 2, 5},			       /* beer     */
  {5, 2, 5},			       /* wine     */
  {2, 2, 5},			       /* ale      */
  {1, 2, 5},			       /* ale      */
  {6, 1, 4},			       /* Whiskey  */
  {0, 1, 8},			       /* lemonade */
  {10, 0, 0},			       /* firebr   */
  {3, 3, 3},			       /* local    */
  {0, 4, -8},			       /* juice    */
  {0, 3, 6},
  {0, 1, 6},
  {0, 1, 6},
  {0, 2, -1},
  {0, 1, -2},
  {0, 1, 5},
  {0, 0, 0}
};

const char *color_liquid[] =
{
  "clear",
  "brown",
  "clear",
  "brown",
  "dark",
  "golden",
  "red",
  "green",
  "clear",
  "light green",
  "white",
  "brown",
  "black",
  "red",
  "clear",
  "black",
  "\n"
};

const char *fullness[] =
{
  "less than half ",
  "about half ",
  "more than half ",
  ""
};

const struct title_type titles[6][ABS_MAX_LVL + 1] =
{
  {
    {"Man", "Woman", 0},
    {"Supplicant", "Supplicant", 1},			/* 1 */
    {"Apprentice", "Apprentice", 2500},
    {"Student", "Student", 5000},
    {"Scholar", "Scholar", 10000},
    {"Trickster", "Trickster", 20000},
    {"Scribe", "Scribe", 40000},
    {"Illusionist", "Illusionist", 60000},
    {"Cabalist", "Cabalist", 90000},
    {"Apparitionist", "Apparitionist", 135000},
    {"Medium", "Gypsy", 250000},			/* 10 */
    {"Sage", "Sage", 375000},
    {"Diviner", "Diviner", 750000},
    {"Alchemist", "Alchemist", 1125000},
    {"Evoker", "Evoker", 1500000},
    {"Abjurer", "Abjuress", 1875000},
    {"Invoker", "Invoker", 2250000},
    {"Enchanter", "Enchantress", 2625000},
    {"Conjurer", "Conjuress", 3000000},
    {"Summoner", "Summoner", 3375000},
    {"Spiritualist", "Spiritualist", 3750000},		/* 20 */
    {"Shaman", "Shamaness", 4125000},
    {"Magician", "Magician", 4500000},
    {"Mystic", "Mystic", 4875000},
    {"Necromancer", "Necromancer", 5250000},
    {"Mentalist", "Mentalist", 5625000},
    {"Synergist", "Synergist", 6000000},
    {"Thaumaturgist", "Thaumaturgist", 6375000},
    {"Dispeller", "Dispeller", 6750000},
    {"Occultist", "Occultist", 7125000},
    {"Apprentice of the Tower", "Apprentice of the Tower", 7500000},	/* 30 */
    {"Servant of the Tower", "Servant of the Tower", 7875000},
    {"Student of the Tower", "Student of the Tower", 8250000},
    {"Guardian of the Tower", "Guardian of the Tower", 8625000},
    {"Watchman of the Tower", "Watchwoman of the Tower", 9000000},
    {"Keeper of the Tower", "Keeper of the Tower", 9375000},
    {"Master Illusionist", "Master Illusionist", 9750000},
    {"Master Spellmaster", "Master Spellmistress", 10125000},
    {"Master Enchanter", "Master Enchantress", 10500000},
    {"Master Spiritualist", "Master Spiritualist", 10875000},
    {"Master of the Tower", "Mistress of the Tower", 11250000},	/* 40 */
    {"Master of the Occult", "Mistress of the Occult", 11625000},
    {"Lord Necromancer", "Lady Necromancer", 12000000},
    {"Master of Energies", "Mistress of Energies", 12375000},
    {"Master of the Mists", "Mistress of the Mists", 12750000},
    {"Lord of the Tower", "Lady of the Tower", 13125000},
    {"Lord of the Mists", "Lady of the Mists", 13500000},
    {"High Lord of the Mists", "High Lady of the Mists", 13875000},
    {"Arch Lord of the Mists", "Arch Lady of the Mists", 14125000},
    {"Magi", "Majestrix", 14500000},
    {"Arch Magi", "Arch Majestrix", 14875000},			/* 50 */
    {"Immortal Warlock", "Immortal Enchantress", 15000000},
    {"Immortal Warlock", "Immortal Enchantress", 16000000},
    {"Immortal Warlock", "Immortal Enchantress", 17000000},
    {"Immortal Warlock", "Immortal Enchantress", 18000000},
    {"Immortal Warlock", "Immortal Enchantress", 19000000},
    {"Immortal Warlock", "Immortal Enchantress", 20000000},
    {"Avatar of Magic", "Avatar of Magic", 21000000},
    {"Source of Magic", "Source of Magic", 22000000},
    {"Demigod", "Demigoddess", 23000000},
    {"God", "Goddess", 24000000},				/* 60 */
    {"Bug", "Bug", 24000001}},

  {
    {"Man", "Woman", 0},
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
    {"Deacon of Shylar", "Deaconess of Shylar", 3000000},
    {"Deacon of Highstaff", "Deaconess of Highstaff", 3250000},
    {"Deacon of Gredth", "Deaconess of Gredth", 3500000},
    {"Deacon of the Reach", "Deaconess of the Reach", 3750000},
    {"Deacon of Nesthar", "Deaconess of Nesthar", 4000000},
    {"Vicar of Shylar", "Vicaress of Shylar", 4250000},
    {"Vicar of Highstaff", "Vicaress of Highstaff", 4500000},
    {"Vicar of Gredth", "Vicaress of Gredth", 4750000},
    {"Vicar of the Reach", "Vicaress of the Reach", 5000000},
    {"Vicar of Nesthar", "Vicaress of Nesthar", 5250000},
    {"Priest of Shylar", "Priestess of Shylar", 5500000},
    {"Priest of Highstaff", "Priestess of Highstaff", 5750000},
    {"Priest of Gredth", "Priestess of Gredth", 6000000},
    {"Priest of the Reach", "Priestess of the Reach", 6250000},
    {"Priest of Nesthar", "Priestess of Nesthar", 6500000},
    {"High Priest of Shylar", "High Priestess of Shylar", 6750000},
{"High Priest of Highstaff", "High Priestess of Highstaff", 7000000},
    {"High Priest of Gredth", "High Priestess of Gredth", 7250000},
{"High Priest of the Reach", "High Priestess of the Reach", 7500000},
    {"High Priest of Nesthar", "High Priestess of Nesthar", 7750000},
    {"Canon of the Mission", "Canon of the Mission", 8000000},
    {"Levite of the Mission", "Levitess of the Mission", 8250000},
    {"Curate of the Mission", "Curate of the Mission", 8500000},
    {"Monk of the Mission", "Nunne of the Mission", 8750000},
    {"Healer of the Mission", "Healer of the Mission", 9000000},
    {"Curate of the Realms", "Curate of the Realms", 9250000},
    {"Priest of the Realms", "Priestess of the Realms", 9500000},
    {"Bishop of the Realms", "Lady Bishop of the Realms", 9750000},
    {"High Priest of the Realms", "High Priestess of the Realms", 10000000},
    {"Patriarch of the Realms", "Matriarch of the Realms", 10250000},
    {"Immortal Cardinal", "Immortal Priestess", 15000000},
    {"Immortal Cardinal", "Immortal Priestess", 16000000},
    {"Immortal Cardinal", "Immortal Priestess", 17000000},
    {"Immortal Cardinal", "Immortal Priestess", 18000000},
    {"Immortal Cardinal", "Immortal Priestess", 19000000},
    {"Immortal Cardinal", "Immortal Priestess", 20000000},
    {"Avatar of Faith", "Avatar of Faith", 21000000},
    {"Source of Faith", "Source of Faith", 22000000},
    {"Demigod", "Demigoddess", 23000000},
    {"God", "Goddess", 24000000},
    {"Bug", "Bug", 24000001}},

  {
    {"Man", "Woman", 0},
    {"Swordpupil", "Swordpupil", 1},
    {"Recruit", "Recruit", 2000},
    {"Sentry", "Sentress", 4000},
    {"Fighter", "Fighter", 8000},
    {"Soldier", "Soldier", 16000},
    {"Warrior", "Warrior", 32000},
    {"Veteran", "Veteran", 64000},
    {"Swordsman", "Swordswoman", 125000},
    {"Fencer", "Fenceress", 250000},
    {"Combatant", "Combatrix", 500000},
    {"Hero", "Heroine", 750000},
    {"Myrmidon", "Myrmidon", 1000000},
    {"Swashbuckler", "Swashbuckleress", 1250000},
    {"Mercenary", "Mercenaress", 1500000},
    {"Swordmaster", "Swordmistress", 1750000},
    {"Lieutenant", "Lieutenant", 2000000},
    {"Champion", "Lady Champion", 2250000},
    {"Dragoon", "Lady Dragoon", 2500000},
    {"Cavalier", "Cavalier", 2750000},
    {"Knight", "Lady Knight", 3000000},
    {"Squire of the Plume", "Squire of the Plume", 3250000},
    {"Lieutenant of the Plume", "Lieutenant of the Plume", 3500000},
    {"Sargent of the Plume", "Sargent of the Plume", 3750000},
    {"Captain of the Plume", "Captain of the Plume", 4000000},
    {"Knight of the Plume", "Knight of the Plume", 4250000},
    {"Baron of the Keep", "Baroness of the Keep", 4500000},
    {"Baron of the Keep", "Baroness of the Keep", 4750000},
    {"Baron of the Keep", "Baroness of the Keep", 5000000},
    {"Baron of the Keep", "Baroness of the Keep", 5250000},
    {"Lord Baron", "Lady Baroness", 5500000},
    {"Keeps Nobleman", "Keeps Noblelady", 5750000},
    {"Keeps Nobleman", "Keeps Noblelady", 6000000},
    {"Keeps Nobleman", "Keeps Noblelady", 6250000},
    {"Keeps Nobleman", "Keeps Noblelady", 6500000},
    {"Lord Nobleman", "Lady Mistress", 6750000},
    {"Count of the Keep", "Countess of the Keep", 7000000},
    {"Count of the Keep", "Countess of the Keep", 7250000},
    {"Count of the Keep", "Countess of the Keep", 7500000},
    {"Count of the Keep", "Countess of the Keep", 7750000},
    {"Lord Count", "Lady Countess", 8000000},
    {"Court Baron", "Court Baroness", 8250000},
    {"Court Nobleman", "Court Lady", 8500000},
    {"Court Count", "Court Countess", 8750000},
    {"Chancellor", "Chancellor", 9000000},
    {"Lord Chancellor", "Lady Chancellor", 9250000},
    {"Kings Baron", "King Baroness", 9500000},
    {"Kings Nobleman", "Kings Lady", 9750000},
    {"Kings Count", "Kings Countess", 10000000},
    {"Prince of the Throne", "Princess of the Throne", 10250000},
    {"King of the Realms", "Queen of the Realms", 10500000},
    {"Immortal Warrior", "Immortal Warrior", 15000000},
    {"Immortal Warrior", "Immortal Warrior", 16000000},
    {"Immortal Warrior", "Immortal Warrior", 17000000},
    {"Immortal Warrior", "Immortal Warrior", 18000000},
    {"Immortal Warrior", "Immortal Warrior", 19000000},
    {"Immortal Warrior", "Immortal Warrior", 20000000},
    {"Avatar of Strength", "Avatar of Strength", 21000000},
    {"Source of Strength", "Source of Strength", 22000000},
    {"Demigod", "Demigoddess", 23000000},
    {"God", "Goddess", 24000000},
    {"Bug", "Bug", 24000001}},

  {
    {"Man", " Woman", 0},
    {"Pilferer", "Pilferess", 1},
    {"Footpad", "Footpad", 1250},
    {"Filcher", "Filcheress", 2500},
    {"Pick-Pocket", "Pick-Pocket", 5000},
    {"Sneak", "Sneak", 10000},
    {"Pincher", "Pincheress", 20000},
    {"Cut-Purse", "Cut-Purse", 30000},
    {"Snatcher", "Snatcheress", 60000},
    {"Sharper", "Sharper", 100000},
    {"Rogue", "Rogue", 140000},
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
    {"Novice Shadower", "Novice Shadower", 3200000},
    {"Shadower", "Shadower", 3400000},
    {"Dark Shadower", "Dark Shadower", 3600000},
    {"Master Shadower", "Master Shadower", 3800000},
    {"Apprentice of the House", "Apprentice of the House", 4000000},
    {"Snatcher of the House", "Snatcheress of the House", 4200000},
    {"Sharper of the House", "Sharper of the House", 4400000},
    {"Master Sharper of the House", "Master Sharper of the House", 4600000},
    {"Rogue of the House", "Rogue of the House", 4800000},
    {"Master Rogue", "Master Rogue", 5000000},
    {"Apprentice Darker", "Apprentice Darker", 5200000},
    {"Novice Darker", "Novice Darker", 5400000},
    {"Darker", "Darker", 5600000},
    {"Master Darker", "Master Darker", 5800000},
    {"Rogue of the Pale", "Rogue of the Pale", 6000000},
    {"Thief of the Pale", "Thief of the Pale", 6200000},
    {"Leader of the Pale", "Lady of the Pale", 6400000},
    {"Rogue of the Shadow", "Rogue of the Shadow", 6600000},
    {"Thief of the Shadow", "Thief of the Shadow", 6800000},
    {"Leader of the Shadow", "Leader of the Shadow", 7000000},
    {"Rogue of Midnight", "Rogue of Midnight", 7200000},
    {"Thief of Midnight", "Thief of Midnight", 7400000},
    {"Leader of Midnight", "Lady of Midnight", 7600000},
    {"Darkness", "Darkness", 7800000},
    {"Lord Midnight", "Lady Midnight", 8000000},
    {"Immortal Assasin", "Immortal Assasin", 15000000},
    {"Immortal Assasin", "Immortal Assasin", 16000000},
    {"Immortal Assasin", "Immortal Assasin", 17000000},
    {"Immortal Assasin", "Immortal Assasin", 18000000},
    {"Immortal Assasin", "Immortal Assasin", 19000000},
    {"Immortal Assasin", "Immortal Assasin", 20000000},
    {"Avatar of Deceit", "Avatar of Deceit", 21000000},
    {"Source of Deceit", "Source of Deceit", 22000000},
    {"Demigod", "Demigoddess", 23000000},
    {"God", "Goddess", 24000000},
    {"Bug", "Bug", 24000001}},

/* Ranger */

  {
    {"Man", "Woman", 0},
    {"Runner", "Runneress", 1},
    {"Stryder", "Strydess", 2000},
    {"Scout", "Scoutess", 4000},
    {"Courser", "Courseress", 8000},
    {"Tracker", "Trackeress", 16000},
    {"Guide", "Guide", 32000},
    {"Pathfinder", "Pathfinder", 64000},
    {"Ranger", "Rangeress", 125000},
    {"Greater Ranger", "Greater Rangeress", 250000},
    {"Woodland Squire", "Woodland Squire", 550000},	/* 10 level */
    {"Apprentice Druid", "Apprentice Druid", 850000},
    {"Protector of the Woods", "Protector of the Woods", 1150000},
    {"Keeper of the Woods", "Keeper of the Woods", 1450000},
    {"Woods Walker", "Woods Walkeress", 1750000},
    {"Ranger Knight", "Ranger Knight", 2050000},
    {"Knight of the Moores", "Knight of the Moores", 2350000},
    {"Knight of Shylar", "Knight of Shylar", 2650000},
    {"Knight of Highstaff", "Knight of Highstaff", 2950000},
    {"Knight of the Reach", "Knight of the Reach", 3250000},
    {"Knight of Wiley", "Knight of Wiley", 3550000},	/* 20 level */
    {"Common Lord", "Common Lady", 3850000},
    {"Common Lord", "Common Lady", 4150000},
    {"Common Lord", "Common Lady", 4450000},
    {"Common Lord", "Common Lady", 4750000},
    {"Lord of the Land", "Lady of the Land", 5050000},	/* 25 level */
    {"Lord of the Land", "Lady of the Land", 5350000},
    {"Lord of the Land", "Lady of the Land", 5650000},
    {"Lord of the Land", "Lady of the Land", 5950000},
    {"Lord of the Land", "Lady of the Land", 6250000},
    {"Baron of the Land", "Baroness of the Land", 6500000},	/* 30 level */
    {"Baron of the Land", "Baroness of the Land", 6850000},
    {"Baron of the Land", "Baroness of the Land", 7150000},
    {"Baron of the Land", "Baroness of the Land", 7450000},
    {"Baron of the Land", "Baroness of the Land", 7750000},
    {"Leader of Men", "Leader of Men", 8050000},	/* 35 level */
    {"Leader of Men", "Leader of Men", 8350000},
    {"Leader of Men", "Leader of Men", 8650000},
    {"Leader of Men", "Leader of Men", 8950000},
    {"Leader of Men", "Leader of Men", 9250000},
    {"City Ruler", "City Ruler", 9550000},	/* 40 level */
    {"City Ruler", "City Ruler", 9950000},
    {"City Ruler", "City Ruler", 10250000},
    {"City Ruler", "City Ruler", 10550000},
    {"City Ruler", "City Ruler", 10850000},
    {"King of his Realm", "Queen of her Realm", 11150000},
    {"King of his Realm", "Queen of her Realm", 11450000},
    {"King of his Realm", "Queen of her Realm", 11750000},
    {"King of his Realm", "Queen of her Realm", 12050000},
    {"King of his Realm", "Queen of her Realm", 12350000},
    {"Leader of Masses", "Leader of Masses", 12750000},
    {"Immortal", "Immortal", 15000000},
    {"Immortal", "Immortal", 16000000},
    {"Immortal", "Immortal", 17000000},
    {"Immortal", "Immortal", 18000000},
    {"Immortal", "Immortal", 19000000},
    {"Immortal", "Immortal", 20000000},
    {"Avatar of Hunting", "Avatar of Hunting", 21000000},
    {"Source of Hunting", "Source of Hunting", 22000000},
    {"Demigod", "Demigoddess", 23000000},
    {"God", "Goddess", 24000000},
    {"Bug", "Bug", 24000001}},

/* Druids */

  {
    {"Man", "Woman", 0},
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
    {"High Priest", "High Priestess", 10250000},
    {"Immortal Cardinal", "Immortal Priestess", 15000000},
    {"Immortal Cardinal", "Immortal Priestess", 16000000},
    {"Immortal Cardinal", "Immortal Priestess", 17000000},
    {"Immortal Cardinal", "Immortal Priestess", 18000000},
    {"Immortal Cardinal", "Immortal Priestess", 19000000},
    {"Immortal Cardinal", "Immortal Priestess", 20000000},
    {"Avatar of Nature", "Avatar of Nature", 21000000},
    {"Source of Nature", "Source of Nature", 22000000},
    {"Demigod", "Demigoddess", 23000000},
    {"God", "Goddess", 24000000},
    {"Bug", "Bug", 24000001}},
};

const char *RaceName[] =
{
  "Half-Breed",
  "Human",
  "Elven",
  "Dwarven",
  "Halfling",
  "Gnome",

/* player races above */

  "Reptilian",
  "Mysterion",
  "Were",
  "Dragon",
  "Undead",
  "Orcan",
  "Insectoid",
  "Arachnoid",
  "Dinosaur",
  "Icthyiod",
  "Avian",
  "Giant",
  "Carnivorous",
  "Parasitic",
  "Slime",
  "Demon",
  "Snake",
  "Herbivorous",
  "Tree",
  "Vegan",
  "Elemental",
  "Planar",
  "Devil",
  "Ghostly",
  "Goblin",
  "Troll",
  "Vegan",
  "Ilythid",
  "Primate",
  "Animal",
  "Faery",
  "Plant",
  "\n"
};

const char *item_types[] =
{
  "UNDEFINED",
  "LIGHT",
  "SCROLL",
  "WAND",
  "STAFF",
  "WEAPON",
  "FIRE WEAPON",
  "MISSILE",
  "TREASURE",
  "ARMOR",
  "POTION",
  "WORN",
  "OTHER",
  "TRASH",
  "TRAP",
  "CONTAINER",
  "NOTE",
  "LIQUID CONTAINER",
  "KEY",
  "FOOD",
  "MONEY",
  "PEN",
  "BOAT",
  "AUDIO",
  "BOW",
  "\n"
};

const char *wear_bits[] =
{
  "TAKE",
  "FINGER",
  "NECK",
  "BODY",
  "HEAD",
  "LEGS",
  "FEET",
  "HANDS",
  "ARMS",
  "SHIELD",
  "ABOUT",
  "WAIST",
  "WRIST",
  "WIELD",
  "HOLD",
  "WIELD-2H",
  "LIGHT-SOURCE",
  "\n"
};

const char *extra_bits[] =
{
  "GLOW",
  "HUM",
  "METAL",
  "MINERAL",
  "ORGANIC",
  "INVISIBLE",
  "MAGIC",
  "NODROP",
  "BLESS",
  "ANTI-GOOD",
  "ANTI-EVIL",
  "ANTI-NEUTRAL",
  "ANTI-CLERIC",
  "ANTI-MAGE",
  "ANTI-THIEF",
  "ANTI-WARRIOR",
  "ANTI-RANGER",
  "PARISH",
  "\n"
};

const char *room_bits[] =
{
  "DARK",
  "DEATH",
  "NO_MOB",
  "INDOORS",
  "PEACEFUL",
  "NOSTEAL",
  "NO_SUM",
  "NO_MAGIC",
  "TUNNEL",
  "PRIVATE",
  "SOUND",
  "\n"
};

const char *exit_bits[] =
{
  "IS-DOOR",
  "CLOSED",
  "LOCKED",
  "SECRET",
  "TRAPPED",
  "PICK-PROOF",
  "ALIASED",
  "\n"
};

const char *sector_types[] =
{
  "Inside",
  "City",
  "Field",
  "Forest",
  "Hills",
  "Mountains",
  "Water Swim",
  "Water NoSwim",
  "Air",
  "Underwater",
  "\n"
};

const char *equipment_types[] =
{
  "Special",
  "Worn on right finger",
  "Worn on left finger",
  "First worn around Neck",
  "Second worn around Neck",
  "Worn on body",
  "Worn on head",
  "Worn on legs",
  "Worn on feet",
  "Worn on hands",
  "Worn on arms",
  "Worn as shield",
  "Worn about body",
  "Worn around waist",
  "Worn around right wrist",
  "Worn around left wrist",
  "Wielded",
  "Held",
  "Wielded Two Handed",
  "\n"
};

const char *affected_bits[] =
{"BLIND",
 "INVISIBLE",
 "DETECT-EVIL",
 "DETECT-INVISIBLE",
 "DETECT-MAGIC",
 "SENCE-LIFE",
 "SILENCED",
 "SANCTUARY",
 "GROUP",
 "UNUSED",
 "CURSE",
 "FLYING",
 "POISON",
 "PROTECT-EVIL",
 "PARALYSIS",
 "INFRAVISION",
 "WATER-BREATH",
 "SLEEP",
 "DODGE",
 "SNEAK",
 "HIDE",
 "FEAR",
 "CHARM",
 "FOLLOW",
 "SAVED_OBJECTS",
 "TRUE-SIGHT",
 "SCRYING",
 "FIRESHIELD",
 "U-5",
 "U-6",
 "U-7",
 "U-8",
 "\n"
};

const char *immunity_names[] =
{"FIRE",
 "COLD",
 "ELECTRICITY",
 "ENERGY",
 "BLUNT",
 "PIERCE",
 "SLASH",
 "ACID",
 "POISON",
 "DRAIN",
 "SLEEP",
 "CHARM",
 "HOLD",
 "\n"
};

const char *apply_types[] =
{
  "NONE",
  "STR",
  "DEX",
  "INT",
  "WIS",
  "CON",
  "SEX",
  "CLASS",
  "LEVEL",
  "AGE",
  "CHAR_WEIGHT",
  "CHAR_HEIGHT",
  "MANA",
  "HIT",
  "MOVE",
  "GOLD",
  "EXP",
  "ARMOR",
  "HITROLL",
  "DAMROLL",
  "SAVING_PARA",
  "SAVING_ROD",
  "SAVING_PETRI",
  "SAVING_BREATH",
  "SAVING_SPELL",
  "SAVING_ALL",
  "RESISTANCE",
  "SUSCEPTIBILITY",
  "IMMUNITY",
  "SPELL AFFECT",
  "WEAPON SPELL",
  "EAT SPELL",
  "BACKSTAB",
  "KICK",
  "SNEAK",
  "HIDE",
  "BASH",
  "PICK",
  "STEAL",
  "TRACK",
  "HIT-N-DAM",
  "\n"
};

const char *pc_class_types[] =
{
  "Magic User",
  "Cleric",
  "Warrior",
  "Thief",
  "Ranger",
  "Druid",
  "\n"
};

const char *npc_class_types[] =
{
  "Normal",
  "Undead",
  "\n"
};

const char *action_bits[] =
{
  "SPEC",
  "SENTINEL",
  "SCAVENGER",
  "ISNPC",
  "NICE-THIEF",
  "AGGRESSIVE",
  "STAY-ZONE",
  "WIMPY",
  "ANNOYING",
  "HATEFUL",
  "AFRAID",
  "IMMORTAL",
  "HUNTING",
  "DEADLY",
  "POLY-SELF"
  "POLY-OTTHER"
  "GUARDIAN",
  "USE-ITEM",
  "FIGHTER-MOVES",
  "PROVIDE-FOOD",
  "PROTECTOR",
  "MOUNT",
  "\n"
};

const char *player_bits[] =
{
  "BRIEF",
  "DONTSET",
  "COMPACT",
  "DONTSET",
  "WIMPY",
  "NOHASSLE",
  "STEALTH",
  "HUNTING",
  "HERMIT",
  "ECHO",
  "",
  "",
  "",
  "",
  "NO-SHOUT",
  "PAGER",
  "LOGS",
  "\n"
};

const char *position_types[] =
{
  "Dead",
  "Mortally wounded",
  "Incapacitated",
  "Stunned",
  "Sleeping",
  "Resting",
  "Sitting",
  "Fighting",
  "Standing",
  "Mounted",
  "\n"
};

const char *connected_types[] =
{
  "Playing",
  "Get name",
  "Confirm name",
  "Read Password",
  "Get new password",
  "Confirm new password",
  "Get sex",
  "Read messages of today",
  "Read Menu",
  "Get extra description",
  "Get class",
  "Dead",
  "New Password",
  "New Password Confirm",
  "",
  "Get Race",
  "",
  "\n"
};

/* [class], [level] (all) */
const int thaco[6][ABS_MAX_LVL] =
{
  {100, 20, 20, 20, 19, 19, 19, 18, 18, 18, 17, 17, 17, 16, 16, 16, 15, 15, 15, 14, 14, 14, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 12, 12, 12, 12, 12, 12},
  {100, 20, 20, 20, 18, 18, 18, 16, 16, 16, 14, 14, 14, 12, 12, 12, 10, 10, 10, 8, 8, 8, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
  {100, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14, 13, 13, 12, 12, 11, 11, 10, 10, 9, 9, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
  {100, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {100, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {100, 20, 20, 20, 18, 18, 18, 16, 16, 16, 14, 14, 14, 12, 12, 12, 10, 10, 10, 8, 8, 8, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6}
};

/* [ch] strength apply (all) */
const struct str_app_type str_app[31] =
{
  {-5, -4, 1, 0},		       /* 0  */
  {-5, -4, 15, 1},		       /* 1  */
  {-3, -2, 20, 2},
  {-3, -1, 30, 3},		       /* 3  */
  {-2, -1, 40, 4},
  {-2, -1, 50, 5},		       /* 5  */
  {-1, 0, 55, 6},
  {-1, 0, 55, 7},
  {0, 0, 60, 8},
  {0, 0, 60, 9},
  {0, 0, 65, 10},		       /* 10  */
  {0, 0, 65, 11},
  {0, 0, 70, 12},
  {0, 0, 75, 13},
  {0, 0, 85, 14},
  {0, 0, 85, 15},		       /* 15  */
  {0, 1, 100, 16},
  {1, 1, 115, 18},
  {1, 2, 140, 20},		       /* 18  */
  {3, 7, 485, 40},
  {3, 8, 700, 40},		       /* 20  */
  {4, 9, 810, 40},
  {4, 10, 970, 40},
  {5, 11, 1130, 40},
  {6, 12, 1440, 40},
  {7, 14, 1750, 40},		       /* 25            */
  {1, 3, 165, 22},		       /* 18/01-50      */
  {2, 3, 190, 24},		       /* 18/51-75      */
  {2, 4, 215, 26},		       /* 18/76-90      */
  {2, 5, 265, 28},		       /* 18/91-99      */
  {3, 6, 365, 30}		       /* 18/100   (30) */
};

/* [dex] skillapply (thieves only) */
const struct dex_skill_type dex_app_skill[26] =
{
  {-99, -99, -90, -99, -60},	       /* 0 */
  {-90, -90, -60, -90, -50},	       /* 1 */
  {-80, -80, -40, -80, -45},
  {-70, -70, -30, -70, -40},
  {-60, -60, -30, -60, -35},
  {-50, -50, -20, -50, -30},	       /* 5 */
  {-40, -40, -20, -40, -25},
  {-30, -30, -15, -30, -20},
  {-20, -20, -15, -20, -15},
  {-15, -10, -10, -20, -10},
  {-10, -5, -10, -15, -5},	       /* 10 */
  {-5, 0, -5, -10, 0},
  {0, 0, 0, -5, 0},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},		       /* 15 */
  {0, 5, 0, 0, 0},
  {5, 10, 0, 5, 5},
  {10, 15, 5, 10, 10},
  {15, 20, 10, 15, 15},
  {15, 20, 10, 15, 15},		       /* 20 */
  {20, 25, 10, 15, 20},
  {20, 25, 15, 20, 20},
  {25, 25, 15, 20, 20},
  {25, 30, 15, 25, 25},
  {25, 30, 15, 25, 25}		       /* 25 */
};

/* [level] backstab multiplyer (thieves only) */
const BYTE backstab_mult[ABS_MAX_LVL] =
{
  1,				       /* 0 */
  1,				       /* 1 */
  1,
  1,
  1,
  1,				       /* 5 */
  2,
  2,
  2,
  2,
  2,				       /* 10 */
  2,
  2,
  2,
  2,
  3,				       /* 15 */
  3,
  3,
  3,
  3,
  3,				       /* 20 */
  3,
  3,
  4,
  4,				       /* 25 */
  4,
  4,
  4,
  4,
  4,				       /* 30 */
  4,
  4,
  4,
  5,
  5,				       /* 35 */
  5,
  5,
  5,
  5,
  5,				       /* 40 */
  5,
  5,
  5,
  5,
  5,				       /* 45 */
  5,
  5,
  5,
  5,
  5,				       /* 50? */
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5
};

/* [dex] apply (all) */
struct dex_app_type dex_app[26] =
{
  {-7, -7, 60},			       /* 0 */
  {-6, -6, 50},			       /* 1 */
  {-4, -4, 50},
  {-3, -3, 40},
  {-2, -2, 30},
  {-1, -1, 20},			       /* 5 */
  {0, 0, 10},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},			       /* 10 */
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, -10},			       /* 15 */
  {1, 1, -20},
  {2, 2, -30},
  {2, 2, -40},
  {3, 3, -40},
  {3, 3, -40},			       /* 20 */
  {4, 4, -50},
  {4, 4, -50},
  {4, 4, -50},
  {5, 5, -60},
  {5, 5, -60}			       /* 25 */
};

/* [con] apply (all) */
struct con_app_type con_app[26] =
{
  {-4, 20},			       /* 0 */
  {-3, 25},			       /* 1 */
  {-2, 30},
  {-2, 35},
  {-1, 40},
  {-1, 45},			       /* 5 */
  {-1, 50},
  {0, 55},
  {0, 60},
  {0, 65},
  {0, 70},			       /* 10 */
  {0, 75},
  {0, 80},
  {0, 85},
  {0, 88},
  {1, 90},			       /* 15 */
  {2, 95},
  {3, 97},
  {3, 99},
  {4, 99},
  {5, 99},			       /* 20 */
  {6, 99},
  {6, 99},
  {7, 99},
  {8, 99},
  {9, 100}			       /* 25 */
};

/* [int] apply (all) */
struct int_app_type int_app[26] =
{
  1,
  2,				       /* 1 */
  3,
  4,
  5,
  6,				       /* 5 */
  7,
  8,
  9,
  10,
  11,				       /* 10 */
  12,
  13,
  15,
  17,
  19,				       /* 15 */
  21,
  23,
  25,
  27,
  55,				       /* 20 */
  56,
  60,
  70,
  80,
  99				       /* 25 */
};

/* [wis] apply (all) */
struct wis_app_type wis_app[26] =
{
  0,				       /* 0 */
  1,				       /* 1 */
  1,
  1,
  1,
  2,				       /* 5 */
  2,
  2,
  2,
  2,
  3,				       /* 10 */
  3,
  3,
  3,
  3,
  4,				       /* 15 */
  4,
  4,
  4,				       /* 18 */
  4,
  6,				       /* 20 */
  6,
  6,
  6,
  6,
  6				       /* 25 */
};

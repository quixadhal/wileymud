#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "include/structs.h"
#include "include/main.h"
#include "include/utils.h"

#include "include/parse_wiley.h"

#define _DUMP_AFK_C
#include "include/dump_afk.h"
#include "include/afk_skills.h"

#define MSL 8192

const int afk_convertsectors[] = { AFK_SECT_INDOORS,
                               AFK_SECT_INDOORS,
                               AFK_SECT_CITY,
                               AFK_SECT_FIELD,
                               AFK_SECT_FOREST,
                               AFK_SECT_HILLS,
                               AFK_SECT_MOUNTAIN,
                               AFK_SECT_WATER_SWIM,
                               AFK_SECT_WATER_NOSWIM,
                               AFK_SECT_AIR,
                               AFK_SECT_UNDERWATER };
const int *afk_convertsector = &afk_convertsectors[1];

extern const char terrains[];
extern const char *terrain;
extern const int exit_dir_to_bit[];

int afk_qcmp_zone_vnum(const void *a, const void *b) {
  return (((const pair *) b)->y - ((const pair *) a)->y);
}





char * const afk_dir_name[]	=
{
    "north", "east", "south", "west", "up", "down",
    "northeast", "northwest", "southeast", "southwest", "somewhere"
};

char *	const	afk_area_flags	[] =
{
"nopkill", "nocamp", "noastral", "noportal", "norecall", "nosummon", "noscry",
"noteleport", "arena", "nobeacon", "noquit", "r11", "r12", "r13", "r14",
"r15", "r16", "r17", "r18", "r19","r20","r21","r22","r23","r24",
"r25","r26","r27","r28","r29","r30","r31"
}; 

char *  const   afk_ex_flags [] = 
{ 
"isdoor", "closed", "locked", "secret", "swim", "pickproof", "fly", "climb",
"dig", "eatkey", "nopassdoor", "hidden", "passage", "portal", "overland", "arrowslit",
"can_climb", "can_enter", "can_leave", "auto", "noflee", "searchable", 
"bashed", "bashproof", "nomob", "window", "can_look", "isbolt", "bolted",
"fortified", "heavy", "medium", "light", "crumbling", "destroyed", ""
};

char *	const	afk_r_flags	[] =
{
"dark", "death", "nomob", "indoors", "safe", "nocamp", "nosummon",
"nomagic", "tunnel", "private", "silence", "nosupplicate", "arena", "nomissile",
"norecall", "noportal", "noastral", "nodrop", "clanstoreroom", "teleport",
"teleshowdesc", "nofloor", "solitary", "petshop", "donation", "nodropall",
"logspeech", "proto", "noteleport", "noscry", "cave", "cavern", "nobeacon",
"auction", "map", "forge", "guildinn", "deleted", "isolated", "watchtower",
"noquit", "telenofly", ""
};

/* The names of varous sector types. Used in the OLC code in addition to
 * the display_map function and probably some other places I've long
 * since forgotten by now.
 */
char *	const	afk_sect_types [] =
{
    "indoors", "city", "field", "forest", "hills", "mountain", "water_swim",
    "water_noswim", "air", "underwater", "desert", "river", "oceanfloor",
    "underground", "jungle", "swamp", "tundra", "ice", "ocean", "lava",
    "shore", "tree", "stone", "quicksand", "wall", "glacier", "exit", 
    "trail", "blands", "grassland", "scrub", "barren", "bridge", "road",
    "landing", "\n"
};

char *  const   afk_o_types [] =
{
"none", "light", "scroll", "wand", "staff", "weapon", "UNUSED1", "UNUSED2",
"treasure", "armor", "potion", "UNUSED3", "furniture", "trash", "UNUSED4",
"container", "UNUSED5", "drinkcon", "key", "food", "money", "pen", "boat",
"corpse", "corpse_pc", "fountain", "pill", "blood", "bloodstain",
"scraps", "pipe", "herbcon", "herb", "incense", "fire", "book", "switch",
"lever", "pullchain", "button", "dial", "rune", "runepouch", "match", "trap",
"map", "portal", "paper", "tinder", "lockpick", "spike", "disease", "oil",
"fuel", "piece", "tree", "missileweapon", "projectile", "quiver", "shovel",
"salve", "cook", "keyring", "odor", "campgear", "drinkmix", "instrument", "ore"
};

char *  const   afk_o_flags [] =
{
"glow", "hum", "metal", "mineral", "organic", "invis", "magic", "nodrop", "bless",
"antigood", "antievil", "antineutral", "anticleric", "antimage",
"antirogue", "antiwarrior", "inventory", "noremove", "twohand", "evil",
"donated", "clanobject", "clancorpse", "antibard", "hidden",
"antidruid", "poisoned", "covering", "deathrot", "buried", "proto",
"nolocate", "groundrot", "antimonk", "loyal", "brittle", "resistant",
"immune", "antimen", "antiwomen", "antineuter", "antiherma", "antisun", "antiranger",
"antipaladin", "antinecro", "antiapal", "onlycleric", "onlymage", "onlyrogue",
"onlywarrior", "onlybard", "onlydruid", "onlymonk", "onlyranger", "onlypaladin",
"onlynecro", "onlyapal", "auction", "onmap", "personal", "lodged", "sindhae",
"mustmount", "noauction", ""
};

char *  const   afk_w_flags    [] =
{
"take", "finger", "finger", "neck", "neck",  "body", "head", "legs", "feet",
"hands", "arms", "shield", "about", "waist", "wrist", "wrist", "wield",
"hold", "dual", "ears", "eyes", "missile", "back", "face", "ankle", "ankle",
"lodge_rib", "lodge_arm", "lodge_leg", "r7","r8","r9","r10","r11","r12","r13"
};

char *  const   afk_mag_flags       [] =
{
"returning", "backstabber", "bane", "loyal", "haste", "drain",
"lightning_blade", "r7", "r8", "r9", "r10", "r11", "r12", "r13",
"r14", "r15", "r16", "r17", "r18", "r19","r20","r21","r22","r23",
"r24","r25","r26","r27","r28","r29","r30","r31"
};

char * const	afk_act_flags[] =
{
"npc", "sentinel", "scavenger", "innkeeper", "banker", "aggressive", "stayarea",
"wimpy", "pet", "autonomous", "practice", "immortal", "deadly", "polyself",
"meta_aggr", "guardian", "boarded", "nowander", "mountable", "mounted",
"scholar", "secretive", "hardhat", "mobinvis", "noassist", "illusion",
"pacifist", "noattack", "annoying", "auction", "proto", "mage", "warrior", "cleric", 
"rogue", "druid", "monk", "paladin", "ranger", "necromancer", "antipaladin",
"huge", "greet", "teacher", "onmap", "smith", "guildauc", "guildbank", "guildvendor",
"guildrepair", "guildforge", "idmob", "guildidmob"
};

char * const	afk_a_flags[] =
{
"blind", "invisible", "detect_evil", "detect_invis", "detect_magic",
"detect_hidden", "hold", "sanctuary", "faerie_fire", "infrared", "curse",
"spy", "poison", "protect", "paralysis", "sneak", "hide", "sleep",
"charm", "flying", "acidmist", "floating", "truesight", "detect_traps",
"scrying", "fireshield", "shockshield", "venomshield", "iceshield", "wizardeye", /* Max obj affect - stupid BV shit */
"berserk", "aqua_breath", "recurringspell", "contagious", "bladebarrier",
"silence", "animal_invis", "heat_stuff", "life_prot", "dragonride",
"growth", "tree_travel", "travelling", "telepathy", "ethereal",
"passdoor", "quiv", "_flaming", "haste", "slow", "elvensong", "bladesong",
"reverie", "tenacity", "deathsong", "possess", "notrack", "enlighten", "treetalk",
"spamguard", "bash", ""
};

char * const afk_lang_names[] = {
 "common", "elvish", "dwarven", "pixie", "ogre",
 "orcish", "trollese", "rodent", "insectoid",
 "mammal", "reptile", "dragon", "spiritual",
 "magical", "goblin", "god", "ancient",
 "halfling", "clan", "gith", "minotaur", "centaur", "gnomish", "sahuagin", ""
};

char * const afk_npc_position[] =
{
  "dead", "mortal", "incapacitated", "stunned", "sleeping",
  "resting", "sitting", "berserk", "aggressive", "fighting", "defensive", 
  "evasive", "standing", "mounted", "shove", "drag"
};

char * const afk_sex[] =
{
  "neuter", "male", "female", "hermaphrodyte"
};

char *  const   afk_npc_race[] =
{ /* starting from 0.... */
"human", "high-elf", "dwarf", "halfling", "pixie", "half-ogre", "half-orc",             /* 6  */
"half-troll", "half-elf", "gith", "minotaur", "duergar", "centaur",                     /* 12 */
"iguanadon", "gnome", "drow", "wild-elf", "insectoid", "sahuagin", "r9",                /* 19 */
"halfbreed", "reptile", "Mysterion", "lycanthrope", "dragon", "undead",                 /* 25 */
"orc", "insect", "spider", "dinosaur", "fish", "avis", "Giant",                         /* 32 */
"Carnivorous", "Parasitic", "slime", "Demonic", "snake", "Herbivorous", "Tree", /* 39 */
"Vegan", "Elemental", "Planar", "Diabolic", "ghost", "goblin", "troll",                 /* 46 */
"Vegman", "Mindflayer", "Primate", "Enfan", "golem", "Aarakocra", "troglodyte", /* 53 */
"Patryn", "Labrynthian", "Sartan", "Titan", "Smurf", "Kangaroo", "horse",               /* 60 */
"Ratperson", "Astralion", "god", "Hill Giant", "Frost Giant", "Fire Giant",             /* 66 */
"Cloud Giant", "Storm Giant", "Stone Giant", "Red Dragon", "Black Dragon",              /* 71 */
"Green Dragon", "White Dragon", "Blue Dragon", "Silver Dragon", "Gold Dragon",  /* 76 */
"Bronze Dragon", "Copper Dragon", "Brass Dragon", "Vampire", "Lich", "wight",           /* 82 */
"Ghast", "Spectre", "zombie", "skeleton", "ghoul", "Half Giant", "Deep Gnome",  /* 89 */
"gnoll", "Sylvan Elf", "Sea Elf", "Tiefling", "Aasimar", "Solar", "Planitar",   /* 96 */
"shadow", "Giant Skeleton", "Nilbog", "Houser", "Baku", "Beast Lord", "Deva",   /* 103 */
"Polaris", "Demodand", "Tarasque", "Diety", "Daemon", "Vagabond",                               /* 109 */
"gargoyle", "bear", "bat", "cat", "dog", "ant", "ape", "baboon",                                /* 117 */
"bee", "beetle", "boar", "bugbear", "ferret", "fly", "gelatin", "gorgon",               /* 125 */
"harpy", "hobgoblin", "kobold", "locust", "mold", "mule",                                       /* 131 */
"neanderthal", "ooze", "rat", "rustmonster", "shapeshifter", "shrew",                   /* 137 */
"shrieker", "stirge", "thoul", "wolf", "worm", "bovine", "canine",                      /* 144 */
"feline", "porcine", "mammal", "rodent", "amphibian", "crustacean",                     /* 150 */
"spirit", "magical", "animal", "humanoid", "monster", "???", "???", "???",              /* 158 */
"???", "???", "???"                                                                                     /* 161 */
};

char *  const   afk_npc_class[] =
{
"mage", "cleric", "rogue", "warrior", "necromancer", "druid", "ranger",
"monk", "available", "available2", "antipaladin", "paladin", "bard", "pc13",
"pc14", "pc15", "pc16", "pc17", "pc18", "pc19",
"baker", "butcher", "blacksmith", "mayor", "king", "queen"
};

char * const     afk_part_flags[] =
{
"head", "arms", "legs", "heart", "brains", "guts", "hands", "feet", "fingers",
"ear", "eye", "long_tongue", "eyestalks", "tentacles", "fins", "wings",
"tail", "scales", "claws", "fangs", "horns", "tusks", "tailattack",
"sharpscales", "beak", "haunches", "hooves", "paws", "forelegs", "feathers",
"r1", "r2"
};

char * const     afk_attack_flags[] =
{
"bite", "claws", "tail", "sting", "punch", "kick", "trip", "bash", "stun",
"gouge", "backstab", "age", "drain", "firebreath", "frostbreath",
"acidbreath", "lightnbreath", "gasbreath", "poison", "nastypoison", "gaze",
"blindness", "causeserious", "earthquake", "causecritical", "curse",
"flamestrike", "harm", "fireball", "colorspray", "weaken", "spiralblast"
};

char * const     afk_defense_flags[] =
{
"parry", "dodge", "heal", "curelight", "cureserious", "curecritical",
"dispelmagic", "dispelevil", "sanctuary", "r1", "r2", "shield", "bless",
"stoneskin", "teleport", "r3", "r4", "r5", "r6", "disarm", "r7", "grip",
"truesight", "r8", "r9", "r10", "r11", "r12"
};

char * const afk_ris_flags[] =
{
"fire", "cold", "electricity", "energy", "blunt", "pierce", "slash", "acid",
"poison", "drain", "sleep", "charm", "hold", "nonmagic", "plus1", "plus2",
"plus3", "plus4", "plus5", "plus6", "magic", "paralysis", "good", "evil", "hack",
"lash", "r5", "r6", "r7", "r8", "r9", "r10"
};

void afk_convert_attack(EXT_BV *NewValue, int OldValue) {
  switch(OldValue) {
    case TYPE_HIT: break; // xSET_BIT(*NewValue, get_attackflag("hit")); break;
    case TYPE_BLUDGEON: xSET_BIT(*NewValue, get_attackflag("punch")); break;
    case TYPE_PIERCE: break; // xSET_BIT(*NewValue, get_attackflag("hit")); break;
    case TYPE_SLASH: break; // xSET_BIT(*NewValue, get_attackflag("hit")); break;
    case TYPE_WHIP: xSET_BIT(*NewValue, get_attackflag("tail")); break;
    case TYPE_CLAW: xSET_BIT(*NewValue, get_attackflag("claws")); break;
    case TYPE_BITE: xSET_BIT(*NewValue, get_attackflag("bite")); break;
    case TYPE_STING: xSET_BIT(*NewValue, get_attackflag("sting")); break;
    case TYPE_CRUSH: xSET_BIT(*NewValue, get_attackflag("bash")); break;
    case TYPE_CLEAVE: xSET_BIT(*NewValue, get_attackflag("gouge")); break;
    case TYPE_STAB: break; // xSET_BIT(*NewValue, get_attackflag("hit")); break;
    case TYPE_SMASH: xSET_BIT(*NewValue, get_attackflag("bash")); break;
    case TYPE_SMITE: xSET_BIT(*NewValue, get_attackflag("stun")); break;
    case TYPE_SUFFERING: xSET_BIT(*NewValue, get_attackflag("poison")); break;
    case TYPE_HUNGER: break; // xSET_BIT(*NewValue, get_attackflag("hit")); break;
    default: break;
  }
}

EXT_BV afk_convert_imm(int OldValue) {
  EXT_BV NewValue;

  memset(&NewValue, '\0', sizeof(EXT_BV));
  if(OldValue & IMM_FIRE) xSET_BIT(NewValue, get_rflag("fire"));
  if(OldValue & IMM_COLD) xSET_BIT(NewValue, get_rflag("cold"));
  if(OldValue & IMM_ELEC) xSET_BIT(NewValue, get_rflag("electricity"));
  if(OldValue & IMM_ENERGY) xSET_BIT(NewValue, get_rflag("energy"));
  if(OldValue & IMM_BLUNT) xSET_BIT(NewValue, get_rflag("blunt"));
  if(OldValue & IMM_PIERCE) xSET_BIT(NewValue, get_rflag("pierce"));
  if(OldValue & IMM_SLASH) xSET_BIT(NewValue, get_rflag("slash"));
  if(OldValue & IMM_ACID) xSET_BIT(NewValue, get_rflag("acid"));
  if(OldValue & IMM_POISON) xSET_BIT(NewValue, get_rflag("poison"));
  if(OldValue & IMM_DRAIN) xSET_BIT(NewValue, get_rflag("drain"));
  if(OldValue & IMM_SLEEP) xSET_BIT(NewValue, get_rflag("sleep"));
  if(OldValue & IMM_CHARM) xSET_BIT(NewValue, get_rflag("charm"));
  if(OldValue & IMM_HOLD) xSET_BIT(NewValue, get_rflag("hold"));
  if(OldValue & IMM_NONMAG) xSET_BIT(NewValue, get_rflag("nonmagic"));
  if(OldValue & IMM_PLUS1) xSET_BIT(NewValue, get_rflag("plus1"));
  if(OldValue & IMM_PLUS2) xSET_BIT(NewValue, get_rflag("plus2"));
  if(OldValue & IMM_PLUS3) xSET_BIT(NewValue, get_rflag("plus3"));
  if(OldValue & IMM_PLUS4) xSET_BIT(NewValue, get_rflag("plus4"));

  return NewValue;
}

int afk_convertsex(int Sex) {
  switch(Sex) {
    case SEX_NEUTER: return get_sex("neuter");
    case SEX_MALE: return get_sex("male");
    case SEX_FEMALE: return get_sex("female");
    default: return get_sex("neuter");
  }
}

int afk_convertposition(int Position) {
  switch(Position) {
    case POSITION_DEAD:  return get_position("dead");
    case POSITION_MORTALLYW: return get_position("mortal");
    case POSITION_INCAP: return get_position("incapacitated");
    case POSITION_STUNNED: return get_position("stunned");
    case POSITION_SLEEPING: return get_position("sleeping");
    case POSITION_RESTING: return get_position("resting");
    case POSITION_SITTING: return get_position("sitting");
    case POSITION_FIGHTING: return get_position("fighting");
    case POSITION_STANDING: return get_position("standing");
    case POSITION_MOUNTED: return get_position("mounted");
    default: return get_position("standing");
  }
}

int afk_convertobjtype(int Type) {
  switch(Type) {
    case ITEM_LIGHT: return get_otype("light");
    case ITEM_SCROLL: return get_otype("scroll");
    case ITEM_WAND: return get_otype("wand");
    case ITEM_STAFF: return get_otype("staff");
    case ITEM_WEAPON: return get_otype("weapon");
    case ITEM_FIREWEAPON: return get_otype("weapon");
    case ITEM_MISSILE: return get_otype("projectile");
    case ITEM_TREASURE: return get_otype("treasure");
    case ITEM_ARMOR: return get_otype("armor");
    case ITEM_POTION: return get_otype("potion");
    case ITEM_TRASH: return get_otype("trash");
    case ITEM_TRAP: return get_otype("trap");
    case ITEM_CONTAINER: return get_otype("container");
    case ITEM_NOTE: return get_otype("paper");
    case ITEM_DRINKCON: return get_otype("drinkcon");
    case ITEM_KEY: return get_otype("key");
    case ITEM_FOOD: return get_otype("food");
    case ITEM_MONEY: return get_otype("money");
    case ITEM_PEN: return get_otype("pen");
    case ITEM_BOAT: return get_otype("boat");

    case ITEM_WORN:
    case ITEM_OTHER:
    case ITEM_AUDIO:
    case ITEM_BOARD:
    case ITEM_KENNEL:
    default: return get_otype("none");
  }
}

EXT_BV afk_convert_rflags(int OldValue) {
  EXT_BV NewValue;

  memset(&NewValue, '\0', sizeof(EXT_BV));
  if(OldValue & ROOM_DARK) xSET_BIT(NewValue, get_rflag("dark"));
  if(OldValue & ROOM_DEATH) xSET_BIT(NewValue, get_rflag("death"));
  if(OldValue & ROOM_NOMOB) xSET_BIT(NewValue, get_rflag("nomob"));
  if(OldValue & ROOM_INDOORS) xSET_BIT(NewValue, get_rflag("indoors"));
  if(OldValue & ROOM_NOATTACK) xSET_BIT(NewValue, get_rflag("safe"));
  /* if(OldValue & ROOM_NOSTEAL) NewValue |= */
  if(OldValue & ROOM_NOSUMMON) xSET_BIT(NewValue, get_rflag("nosummon"));
  if(OldValue & ROOM_NOMAGIC) xSET_BIT(NewValue, get_rflag("nomagic"));
  if(OldValue & ROOM_PRIVATE) xSET_BIT(NewValue, get_rflag("private"));
  /* if(OldValue & ROOM_SOUND) NewValue |= */

  return NewValue;
}

EXT_BV afk_convert_exflags(int OldType) {
  EXT_BV NewValue;

  memset(&NewValue, '\0', sizeof(EXT_BV));
  switch(OldType) {
    case EXIT_DOOR_ALIAS:
    case EXIT_DOOR:
      xSET_BIT(NewValue, get_exflag("isdoor"));
      xSET_BIT(NewValue, get_exflag("closed"));
      break;
    case EXIT_NOPICK_ALIAS:
    case EXIT_NOPICK:
      xSET_BIT(NewValue, get_exflag("isdoor"));
      xSET_BIT(NewValue, get_exflag("pickproof"));
      xSET_BIT(NewValue, get_exflag("closed"));
      break;
    case EXIT_SECRET_ALIAS:
    case EXIT_SECRET:
      xSET_BIT(NewValue, get_exflag("isdoor"));
      xSET_BIT(NewValue, get_exflag("secret"));
      xSET_BIT(NewValue, get_exflag("closed"));
      break;
    case EXIT_SECRET_NOPICK_ALIAS:
    case EXIT_SECRET_NOPICK:
      xSET_BIT(NewValue, get_exflag("isdoor"));
      xSET_BIT(NewValue, get_exflag("secret"));
      xSET_BIT(NewValue, get_exflag("pickproof"));
      xSET_BIT(NewValue, get_exflag("closed"));
      break;
    default:
      break;
  }

  return NewValue;
}

EXT_BV afk_convert_oflags(int OldValue) {
  EXT_BV NewValue;

  memset(&NewValue, '\0', sizeof(EXT_BV));
  if(OldValue & ITEM_GLOW) xSET_BIT(NewValue, get_oflag("glow"));
  if(OldValue & ITEM_HUM) xSET_BIT(NewValue, get_oflag("hum"));
  if(OldValue & ITEM_METAL) xSET_BIT(NewValue, get_oflag("metal"));
  if(OldValue & ITEM_MINERAL) xSET_BIT(NewValue, get_oflag("mineral"));
  if(OldValue & ITEM_ORGANIC) xSET_BIT(NewValue, get_oflag("organic"));
  if(OldValue & ITEM_INVISIBLE) xSET_BIT(NewValue, get_oflag("invis"));
  if(OldValue & ITEM_MAGIC) xSET_BIT(NewValue, get_oflag("magic"));
  if(OldValue & ITEM_NODROP) xSET_BIT(NewValue, get_oflag("nodrop"));
  if(OldValue & ITEM_BLESS) xSET_BIT(NewValue, get_oflag("bless"));
  if(OldValue & ITEM_ANTI_GOOD) xSET_BIT(NewValue, get_oflag("antigood"));
  if(OldValue & ITEM_ANTI_EVIL) xSET_BIT(NewValue, get_oflag("antievil"));
  if(OldValue & ITEM_ANTI_NEUTRAL) xSET_BIT(NewValue, get_oflag("antineutral"));
  if(OldValue & ITEM_ANTI_CLERIC) xSET_BIT(NewValue, get_oflag("anticleric"));
  if(OldValue & ITEM_ANTI_MAGE) xSET_BIT(NewValue, get_oflag("antimage"));
  if(OldValue & ITEM_ANTI_THIEF) xSET_BIT(NewValue, get_oflag("antirogue"));
  if(OldValue & ITEM_ANTI_FIGHTER) xSET_BIT(NewValue, get_oflag("antiwarrior"));
  if(OldValue & ITEM_ANTI_RANGER) xSET_BIT(NewValue, get_oflag("antiranger"));
  if(OldValue & ITEM_PARISH) /* No match in AFK */;

  return NewValue;
}

int afk_convert_wflags(int OldValue) {
  int NewValue = 0;

  if(OldValue & ITEM_TAKE) SET_BIT(NewValue, get_wflag("take"));
  if(OldValue & ITEM_WEAR_FINGER) SET_BIT(NewValue, get_wflag("finger"));
  if(OldValue & ITEM_WEAR_NECK) SET_BIT(NewValue, get_wflag("neck"));
  if(OldValue & ITEM_WEAR_BODY) SET_BIT(NewValue, get_wflag("body"));
  if(OldValue & ITEM_WEAR_HEAD) SET_BIT(NewValue, get_wflag("head"));
  if(OldValue & ITEM_WEAR_LEGS) SET_BIT(NewValue, get_wflag("legs"));
  if(OldValue & ITEM_WEAR_FEET) SET_BIT(NewValue, get_wflag("feet"));
  if(OldValue & ITEM_WEAR_HANDS) SET_BIT(NewValue, get_wflag("hands"));
  if(OldValue & ITEM_WEAR_ARMS) SET_BIT(NewValue, get_wflag("arms"));
  if(OldValue & ITEM_WEAR_SHIELD) SET_BIT(NewValue, get_wflag("shield"));
  if(OldValue & ITEM_WEAR_ABOUT) SET_BIT(NewValue, get_wflag("about"));
  if(OldValue & ITEM_WEAR_WAISTE) SET_BIT(NewValue, get_wflag("waist"));
  if(OldValue & ITEM_WEAR_WRIST) SET_BIT(NewValue, get_wflag("wrist"));
  if(OldValue & ITEM_WIELD) SET_BIT(NewValue, get_wflag("wield"));
  if(OldValue & ITEM_HOLD) SET_BIT(NewValue, get_wflag("hold"));
  if(OldValue & ITEM_WIELD_TWOH) SET_BIT(NewValue, get_wflag("dual"));

  return NewValue;
}

EXT_BV afk_convert_actflags(int OldValue) {
  EXT_BV NewValue;

  memset(&NewValue, '\0', sizeof(EXT_BV));

  if(OldValue & ACT_SPEC) ; // NOOP 
  if(OldValue & ACT_SENTINEL) xSET_BIT(NewValue, get_actflag("sentinel"));
  if(OldValue & ACT_SCAVENGER) xSET_BIT(NewValue, get_actflag("scavenger"));
  if(OldValue & ACT_ISNPC) ; // NOOP
  if(OldValue & ACT_NICE_THIEF) ;// NOOP
  if(OldValue & ACT_AGGRESSIVE) xSET_BIT(NewValue, get_actflag("aggressive"));
  if(OldValue & ACT_STAY_ZONE) xSET_BIT(NewValue, get_actflag("stayarea"));
  if(OldValue & ACT_WIMPY) xSET_BIT(NewValue, get_actflag("wimpy"));
  if(OldValue & ACT_ANNOYING) xSET_BIT(NewValue, get_actflag("annoying"));
  if(OldValue & ACT_HATEFUL) ; // NOOP
  if(OldValue & ACT_AFRAID) ; // NOOP
  if(OldValue & ACT_IMMORTAL) xSET_BIT(NewValue, get_actflag("immortal"));
  if(OldValue & ACT_HUNTING) ; // NOOP
  if(OldValue & ACT_DEADLY) xSET_BIT(NewValue, get_actflag("deadly"));
  if(OldValue & ACT_GUARDIAN) xSET_BIT(NewValue, get_actflag("guardian"));
  if(OldValue & ACT_USE_ITEM) ; // NOOP
  if(OldValue & ACT_FIGHTER_MOVES) xSET_BIT(NewValue, get_actflag("meta_aggr")); // Not quite
  if(OldValue & ACT_FOOD_PROVIDE) ; // NOOP
  if(OldValue & ACT_PROTECTOR) ; // NOOP
  if(OldValue & ACT_MOUNT) xSET_BIT(NewValue, get_actflag("mountable"));
  if(OldValue & ACT_SWITCH) ; // NOOP

  return NewValue;
}

EXT_BV afk_convert_aflags(int OldValue) {
  EXT_BV NewValue;

  memset(&NewValue, '\0', sizeof(EXT_BV));

  if(OldValue & AFF_BLIND) xSET_BIT(NewValue, get_aflag("blind"));
  if(OldValue & AFF_INVISIBLE) xSET_BIT(NewValue, get_aflag("invisible"));
  if(OldValue & AFF_DETECT_EVIL) xSET_BIT(NewValue, get_aflag("detect_evil"));
  if(OldValue & AFF_DETECT_INVISIBLE) xSET_BIT(NewValue, get_aflag("detect_invis"));
  if(OldValue & AFF_DETECT_MAGIC) xSET_BIT(NewValue, get_aflag("detect_magic"));
  if(OldValue & AFF_SENSE_LIFE) xSET_BIT(NewValue, get_aflag("detect_hidden")); // Not quite

  if(OldValue & AFF_SILENCED) xSET_BIT(NewValue, get_aflag("silence"));
  if(OldValue & AFF_SANCTUARY) xSET_BIT(NewValue, get_aflag("sanctuary"));
  if(OldValue & AFF_GROUP) ; // NOOP
  if(OldValue & AFF_CURSE) xSET_BIT(NewValue, get_aflag("curse"));
  if(OldValue & AFF_FLYING) xSET_BIT(NewValue, get_aflag("flying"));
  if(OldValue & AFF_POISON) xSET_BIT(NewValue, get_aflag("poison"));
  if(OldValue & AFF_PROTECT_EVIL) xSET_BIT(NewValue, get_aflag("protect"));
  if(OldValue & AFF_PARALYSIS) xSET_BIT(NewValue, get_aflag("paralysis"));
  if(OldValue & AFF_INFRAVISION) ; // NOOP
  if(OldValue & AFF_WATERBREATH) xSET_BIT(NewValue, get_aflag("aqua_breath"));
  if(OldValue & AFF_SLEEP) xSET_BIT(NewValue, get_aflag("sleep"));
  if(OldValue & AFF_DRUG_FREE) ; // NOOP
  if(OldValue & AFF_SNEAK) xSET_BIT(NewValue, get_aflag("sneak"));
  if(OldValue & AFF_HIDE) xSET_BIT(NewValue, get_aflag("hide"));
  if(OldValue & AFF_FEAR) ; // NOOP
  if(OldValue & AFF_CHARM) ; // NOOP
  if(OldValue & AFF_FOLLOW) ; // NOOP
  if(OldValue & AFF_UNDEF_1) ; // NOOP
  if(OldValue & AFF_TRUE_SIGHT) xSET_BIT(NewValue, get_aflag("truesight"));
  if(OldValue & AFF_SCRYING) xSET_BIT(NewValue, get_aflag("scrying"));
  if(OldValue & AFF_FIRESHIELD) xSET_BIT(NewValue, get_aflag("fireshield")); // Not quite
  if(OldValue & AFF_RIDE) ; // NOOP
  if(OldValue & AFF_UNDEF_6) ; // NOOP
  if(OldValue & AFF_UNDEF_7) ; // NOOP
  if(OldValue & AFF_UNDEF_8) ; // NOOP

  return NewValue;
}

/*
 * Compare strings, case insensitive.
 * Return TRUE if different
 *   (compatibility with historical functions).
 */
bool str_cmp( const char *astr, const char *bstr )
{
    if ( !astr )
    {
	if ( bstr )
	  fprintf( stderr, "str_cmp: astr: (null)  bstr: %s\n", bstr );
	return TRUE;
    }

    if ( !bstr )
    {
	if ( astr )
	  fprintf( stderr, "str_cmp: astr: %s  bstr: (null)\n", astr );
	return TRUE;
    }

    for ( ; *astr || *bstr; astr++, bstr++ )
    {
	if ( LOWER(*astr) != LOWER(*bstr) )
	    return TRUE;
    }

    return FALSE;
}

int get_rflag( char *flag )
{
    int x = 0;
    
    for ( x = 0; x < (sizeof(afk_r_flags) / sizeof(afk_r_flags[0])); x++ )
      if ( !str_cmp(flag, afk_r_flags[x]) )
        return x;
    return -1;
}

int get_exflag( char *flag )
{
    int x = 0;
    
    for ( x = 0; x < (sizeof(afk_ex_flags) / sizeof(afk_ex_flags[0])); x++ )
      if ( !str_cmp(flag, afk_ex_flags[x]) )
        return x;
    return -1;
}

int get_otype( const char *type )
{
   unsigned int x;
 
   for( x = 0; x < (sizeof(afk_o_types) / sizeof(afk_o_types[0])); x++ )
      if( !str_cmp(type, afk_o_types[x]) )
        return x;
    return -1;
}

int get_oflag( char *flag )
{
    unsigned int x;
 
    for ( x = 0; x < (sizeof(afk_o_flags) / sizeof(afk_o_flags[0])); x++ )
      if ( !str_cmp(flag, afk_o_flags[x]) )
        return x;
    return -1;
}

int get_wflag( char *flag )
{
    unsigned int x;
 
    for ( x = 0; x < (sizeof(afk_w_flags) / sizeof(afk_w_flags[0])); x++ )
      if ( !str_cmp(flag, afk_w_flags[x]) )
        return x;
    return -1;
}

int get_magflag( char *flag )
{
    unsigned int x;
 
    for ( x = 0; x < (sizeof(afk_mag_flags) / sizeof(afk_mag_flags[0])); x++ )
      if ( !str_cmp(flag, afk_mag_flags[x]) )
        return x;
    return -1;
}

int get_actflag( char *flag )
{
    unsigned int x;
 
    for ( x = 0; x < (sizeof(afk_act_flags) / sizeof(afk_act_flags[0])); x++ )
      if ( !str_cmp(flag, afk_act_flags[x]) )
        return x;
    return -1;
}

int get_aflag( char *flag )
{
    unsigned int x;
 
    for ( x = 0; x < (sizeof(afk_a_flags) / sizeof(afk_a_flags[0])); x++ )
      if ( !str_cmp(flag, afk_a_flags[x]) )
        return x;
    return -1;
}

int get_position( const char *type )
{
   unsigned int x;
 
   for( x = 0; x < (sizeof(afk_npc_position) / sizeof(afk_npc_position[0])); x++ )
      if( !str_cmp(type, afk_npc_position[x]) )
        return x;
    return -1;
}

int get_sex( const char *type )
{
   unsigned int x;
 
   for( x = 0; x < (sizeof(afk_sex) / sizeof(afk_sex[0])); x++ )
      if( !str_cmp(type, afk_sex[x]) )
        return x;
    return -1;
}

int get_npc_race( char *type )
{
    int x;

   for( x = 0; x < (sizeof(afk_npc_race) / sizeof(afk_npc_race[0])); x++ )
      if ( !str_cmp(type, afk_npc_race[x]) )
        return x;
    return -1;
}

int get_npc_class( char *type )
{
    int x;

   for( x = 0; x < (sizeof(afk_npc_class) / sizeof(afk_npc_class[0])); x++ )
      if ( !str_cmp(type, afk_npc_class[x]) )
        return x;
    return -1;
}

int get_partflag( char *flag )
{   
    unsigned int x;

    for ( x = 0; x < (sizeof(afk_part_flags) / sizeof(afk_part_flags[0])); x++ )
      if ( !str_cmp(flag, afk_part_flags[x]) )
        return x;
    return -1;
}

int get_risflag( char *flag ) 
{           
    unsigned int x;
    
    for ( x = 0; x < (sizeof(afk_ris_flags) / sizeof(afk_ris_flags[0])); x++ )
      if ( !str_cmp(flag, afk_ris_flags[x]) )
        return x;
    return -1;
}

int get_attackflag( char *flag ) 
{           
    unsigned int x;
    
    for ( x = 0; x < (sizeof(afk_attack_flags) / sizeof(afk_attack_flags[0])); x++ )
      if ( !str_cmp(flag, afk_attack_flags[x]) )
        return x;
    return -1;
}

char *flag_string( int bitvector, char * const flagarray[] )
{
    static char buf[MSL];
    int x = 0;

    memset(buf, '\0', MSL);
    for ( x = 0; x < 32 ; x++ )
      if ( IS_SET( bitvector, 1 << x ) )
      {
	strcat( buf, flagarray[x] );
	strcat( buf, " " );
      }
    if ( (x=strlen( buf )) > 0 )
      buf[--x] = '\0';
    
    return buf;
}

char *ext_flag_string( EXT_BV *bitvector, char * const flagarray[] )
{
    static char buf[MSL];
    int x = 0;

    memset(buf, '\0', MSL);
    for ( x = 0; x < MAX_BITS ; x++ )
	if ( xIS_SET( *bitvector, x ) )
	{
	    strcat( buf, flagarray[x] );
	    strcat( buf, " " );
	}
    if ( (x=strlen(buf)) > 0 )
	buf[--x] = '\0';
    
    return buf;
}

/*
 * Remove carriage returns from a line
 */
char *strip_cr( char *str )
{
    static char newstr[MSL];
    int i = 0;
    int j = 0;

    memset(newstr, '\0', MSL);

    for ( i=j=0; str[i] != '\0'; i++ )
	if ( str[i] != '\r' )
	{
	  newstr[j++] = str[i];	
	}
    newstr[j] = '\0';
    return newstr;
}


int afk_convert_spell(int SpellNum) {
  switch(SpellNum) {
    case SPELL_RESERVED_DBC:
      return -1;
    case SPELL_ARMOR:
      return -1;
    default:
      return -1;
  }
}

int afk_convert_skill(int SkillNum) {
  return SkillNum;
}

int afk_convert_liquid(int Liquid) {
  /* No conversion needed */
  return Liquid;
}

int * afk_convert_item_values(int Type, int w0, int w1, int w2, int w3) {
  static int a[11];

  a[0] = w0;
  a[1] = w1;
  a[2] = w2;
  a[3] = w3;
  a[4] = 0;
  a[5] = 0;
  a[6] = 0;
  a[7] = 0;
  a[8] = 0;
  a[9] = 0;
  a[10] = 0;

  switch(Type) {
    case ITEM_LIGHT:
      break;
    case ITEM_SCROLL:
      a[1] = afk_convert_spell(w1);
      a[2] = afk_convert_spell(w2);
      a[3] = afk_convert_spell(w3);
      break;
    case ITEM_WAND:
      a[3] = afk_convert_spell(w3);
      break;
    case ITEM_STAFF:
      a[3] = afk_convert_spell(w3);
      break;
    case ITEM_WEAPON:
    case ITEM_FIREWEAPON:
      switch(w3) {
        case 0: /* Bludgeon (smite) */
          a[3] = 4; /* crush damage */
          a[4] = 5; /* mace skill */
          break;
        case 1: /* Pierce (stab) */
          a[3] = 2; /* stab damage */
          a[4] = 2; /* dagger skill */
          break;
        case 2: /* Slash (whip) */
          a[3] = 5; /* lash damage */
          a[4] = 3; /* whip skill */
          break;
        case 3: /* Slash (slash) */
          a[3] = 1; /* slash damage */
          a[4] = 1; /* sword skill */
          break;
        case 4: /* Bludgeon (smash) */
          a[3] = 4; /* crush damage */
          a[4] = 5; /* mace skill */
          break;
        case 5: /* Slash (cleave) */
          a[3] = 3; /* hack damage */
          a[4] = 9; /* axe skill */
          break;
        case 6: /* Bludgeon (crush) */
          a[3] = 4; /* crush damage */
          a[4] = 5; /* mace skill */
          break;
        case 7: /* Bludgeon (pound) */
          a[3] = 4; /* crush damage */
          a[4] = 5; /* mace skill */
          break;
        case 8: /* Mob (claw) */
          a[3] = 1; /* slash damage */
          a[4] = 0; /* barehand skill */
          break;
        case 9: /* Mob (bite) */
          a[3] = 4; /* crush damage */
          a[4] = 0; /* barehand skill */
          break;
        case 10: /* Mob (sting) */
          a[3] = 6; /* pierce damage */
          a[4] = 0; /* barehand skill */
          break;
        case 11: /* Pierce (pierce) */
          a[3] = 6; /* pierce damage */
          a[4] = 2; /* dagger skill */
          break;
        default:
          ;
      }
      break;
    case ITEM_MISSILE:
      switch(w3) {
        case 0: /* Bludgeon (smite) */
          a[3] = 4; /* crush damage */
          a[4] = 3; /* stone type */
          break;
        case 1: /* Pierce (stab) */
          a[3] = 2; /* stab damage */
          a[4] = 0; /* bolt type */
          break;
        case 2: /* Slash (whip) */
          a[3] = 5; /* lash damage */
          a[4] = 1; /* arrow type */
          break;
        case 3: /* Slash (slash) */
          a[3] = 1; /* slash damage */
          a[4] = 1; /* arrow type */
          break;
        case 4: /* Bludgeon (smash) */
          a[3] = 4; /* crush damage */
          a[4] = 3; /* stone type */
          break;
        case 5: /* Slash (cleave) */
          a[3] = 3; /* hack damage */
          a[4] = 1; /* arrow type */
          break;
        case 6: /* Bludgeon (crush) */
          a[3] = 4; /* crush damage */
          a[4] = 3; /* stone type */
          break;
        case 7: /* Bludgeon (pound) */
          a[3] = 4; /* crush damage */
          a[4] = 3; /* stone type */
          break;
        case 8: /* Mob (claw) */
          a[3] = 1; /* slash damage */
          a[4] = 1; /* arrow type */
          break;
        case 9: /* Mob (bite) */
          a[3] = 4; /* crush damage */
          a[4] = 3; /* stone type */
          break;
        case 10: /* Mob (sting) */
          a[3] = 6; /* pierce damage */
          a[4] = 2; /* dart type */
          break;
        case 11: /* Pierce (pierce) */
          a[3] = 6; /* pierce damage */
          a[4] = 2; /* dart type */
          break;
        default:
          ;
      }
      break;
    case ITEM_TREASURE:
      break;
    case ITEM_ARMOR:
      a[2] = 0;
      a[3] = 0;
      break;
    case ITEM_POTION:
      a[1] = afk_convert_spell(w1);
      a[2] = afk_convert_spell(w2);
      a[3] = afk_convert_spell(w3);
      break;
    case ITEM_WORN:
      break;
    case ITEM_OTHER:
      break;
    case ITEM_TRASH:
      break;
    case ITEM_TRAP: /* NOT supported in AFK? */
      break;
    case ITEM_CONTAINER:
      a[3] = 0;
      break;
    case ITEM_NOTE:
      break;
    case ITEM_DRINKCON:
      a[2] = afk_convert_liquid(w2);
      break;
    case ITEM_KEY:
      /* May need a[5], unsure yet */
      break;
    case ITEM_FOOD:
      break;
    case ITEM_MONEY:
      break;
    case ITEM_PEN:
      break;
    case ITEM_BOAT:
      break;
    default:
      /* No conversion done */
      ;
  }

  return a;
}





void dump_as_afk(zones *Zones, rooms *Rooms, shops *Shops, objects *Objects, mobs *Mobs) {
  FILE *ofp, *listfp;
  char filename[256], tmpstr[256];
  int i, j, k, x;
  int m;
  /* int LastMob, LastLoc; */
  int LowRoom, HighRoom;
  pair *ZoneSort;

  sprintf(filename, "%s/%s/area.lst", OutputDir, AFK_SUBDIR);
  listfp= open_file(filename, "w");

  ZoneSort= (pair *)get_mem(Zones->Count, sizeof(pair));
  bzero(ZoneSort, sizeof(pair)*Zones->Count);
  for(i= 0; i< Zones->Count; i++) {
    sprintf(tmpstr, "%s_%d.are", remap_name(Zones->Zone[i].Name), Zones->Zone[i].Number);
    fprintf(listfp, "%s\n", tmpstr);
    sprintf(filename, "%s/%s/%s", OutputDir, AFK_SUBDIR, tmpstr);
    ofp= open_file(filename, "w");
    if(Verbose)
      fprintf(stderr, "Dump of Zone \"%s\"[#%d]...\n", remap_name(Zones->Zone[i].Name),
              Zones->Zone[i].Number);
    else if(!Quiet) {
      sprintf(tmpstr, "#%d Dump of Zone \"%s\"[#%d]...", i+1, remap_name(Zones->Zone[i].Name),
              Zones->Zone[i].Number);
      fprintf(stderr, "%s", tmpstr);
      for(x= strlen(tmpstr); x< 79; x++)
        fprintf(stderr, " ");
      for(x= strlen(tmpstr); x< 79; x++)
        fprintf(stderr, "\b");
      fflush(stderr);
    }
    LowRoom = INT_MAX;
    HighRoom = INT_MIN;
    for(j= 0; j< Rooms->Count; j++) {
      if((remap_zone_vnum(Zones, Rooms->Room[j].Zone) == i) ||
         ((Rooms->Room[j].Number >= (!i? 0: Zones->Zone[i-1].Top+1)) &&
          (Rooms->Room[j].Number <= Zones->Zone[i].Top)
      )) {
        LowRoom= min(LowRoom, Rooms->Room[j].Number);
        HighRoom= max(HighRoom, Rooms->Room[j].Number);
      }
    }
    ZoneSort[i].x= i;
    ZoneSort[i].y= LowRoom;

/* ZONE */

    fprintf( ofp, "#AREA   %s~\n\n", Zones->Zone[i].Name);
    fprintf( ofp, "#VERSION %d\n\n", AREA_VERSION_WRITE );
    fprintf( ofp, "#AUTHOR %s~\n\n", "The Wiley Gang");
    fprintf( ofp, "#VNUMS %d %d\n\n", LowRoom, HighRoom);

    /* Wiley has no level ranges */
#if 0
    fprintf( ofp, "#RANGES\n%d %d %d %d\n$\n\n",
             AREA_LEVEL_LIMIT_LOWER, AREA_LEVEL_LIMIT_UPPER,   /* SOFT (displayed) limits */
             AREA_LEVEL_LIMIT_LOWER, AREA_LEVEL_LIMIT_UPPER ); /* HARD (driver enforced) limits */
#endif

    /* Wiley doesn't give players hints about zone resets! */
#if 0
    if ( tarea->resetmsg )	// Rennard
	fprintf( ofp, "#RESETMSG %s~\n\n", tarea->resetmsg );
#endif

    if ( Zones->Zone[i].Time )
      fprintf( ofp, "#RESETFREQUENCY %d\n\n", Zones->Zone[i].Time);

    /* AFK apparently doesn't have reset modes */
    /* fprintf(ofp, "Mode\t%d\n", Zones->Zone[i].Mode); */

    /* Wiley doesn't have any zone flags. */
#if 0
    if ( tarea->flags )
        fprintf( ofp, "#FLAGS\n%s~\n\n", flag_string(tarea->flags, area_flags) );
#endif

    /* Wiley doesn't have an overland map system */
#if 0
    fprintf( ofp, "#CONTINENT %s~\n\n", continents[tarea->continent] );
#endif

    /* Wiley doesn't have coordinates */
#if 0
    fprintf( ofp, "#COORDS %d %d\n\n", tarea->x, tarea->y );
#endif

    /* Wiley doesn't have climate per zone, so all are normal */
    fprintf( ofp, "#CLIMATE %d %d %d\n\n",
             CLIMATE_TEMP_NORMAL,
             CLIMATE_PRECIP_NORMAL,
             CLIMATE_WIND_NORMAL );

    /* Wiley doesn't have neighboring zones */
#if 0
    // neighboring weather systems - FB
    for(neigh = tarea->weather->first_neighbor; neigh; neigh = neigh->next)
    	fprintf( fpout, "#NEIGHBOR %s~\n\n", neigh->name);
#endif

/* MOBS */


    fprintf( ofp, "%s", "#MOBILES\n" );
    for(j= 0; j< Mobs->Count; j++) {
      if((remap_zone_vnum(Zones, Mobs->Mob[j].Zone) == i) ||
         ((Mobs->Mob[j].Number >= (!i? 0: Zones->Zone[i-1].Top+1)) &&
          (Mobs->Mob[j].Number <= Zones->Zone[i].Top)
      )) {
        EXT_BV MobValue;
        EXT_BV AttackValues;
        EXT_BV ImmValue;
        //int WearValue = 0;
        //int *val = NULL;
        //int kval[11];
        dice avg;

        memset(&MobValue, '\0', sizeof(EXT_BV));
        memset(&AttackValues, '\0', sizeof(EXT_BV));
        memset(&ImmValue, '\0', sizeof(EXT_BV));

        if(Verbose)
          fprintf(stderr, "  (%d) Writing \"%s\"[#%d]\n", j,
                  Mobs->Mob[j].Name, Mobs->Mob[j].Number);
        else if(!Quiet)
          spin(stderr);

        fprintf(ofp, "#%d\n", Mobs->Mob[j].Number);
        fprintf(ofp, "%s~\n", strip_cr(Mobs->Mob[j].Name)); /* called keywords now */
        fprintf(ofp, "%s~\n", strip_cr(Mobs->Mob[j].ShortDesc));
        fprintf(ofp, "%s~\n", strip_cr(Mobs->Mob[j].Description)); /* LongDesc */
        fprintf(ofp, "%s~\n", strip_cr(Mobs->Mob[j].LongDesc)); /* ActionDesc elsewhere */

        MobValue = afk_convert_actflags(Mobs->Mob[j].ActFlags);
        fprintf(ofp, "%s~\n", strip_cr(ext_flag_string(&MobValue, afk_act_flags)));
        MobValue = afk_convert_aflags(Mobs->Mob[j].AffectedBy);
        fprintf(ofp, "%s~\n", strip_cr(ext_flag_string(&MobValue, afk_a_flags)));

        fprintf(ofp, "%d %d %d %d %d %d %d %f\n", Mobs->Mob[j].Alignment,
                     (Mobs->Mob[j].Gold.Rolls * Mobs->Mob[j].Gold.Die) + Mobs->Mob[j].Gold.Modifier,
                     0, /* no idea? */
                     Mobs->Mob[j].Height, Mobs->Mob[j].Weight,
                     Mobs->Mob[j].MovePoints, Mobs->Mob[j].ManaPoints,
                     (float)Mobs->Mob[j].AttackCount );
        fprintf(ofp, "%d %d %d %d ", Mobs->Mob[j].Level, Mobs->Mob[j].ToHit,
                                     Mobs->Mob[j].ArmourClass, 0 /* Mobs->Mob[j].hitplus */ );
        /* WileyMUD had multiple attacks, each defined by dice.  We'll have to average */
        if(Mobs->Mob[j].AttackCount > 0) {
          avg.Rolls = avg.Die = avg.Modifier = 0;
          for(k = 0; k < Mobs->Mob[j].AttackCount; k++) {
            avg.Rolls += Mobs->Mob[j].Attack[k].Rolls;
            avg.Die += Mobs->Mob[j].Attack[k].Die;
            avg.Modifier += Mobs->Mob[j].Attack[k].Modifier;
            afk_convert_attack(&AttackValues, Mobs->Mob[j].Attack[k].Type);
          }
          avg.Rolls /= Mobs->Mob[j].AttackCount;
          avg.Die /= Mobs->Mob[j].AttackCount;
          avg.Modifier /= Mobs->Mob[j].AttackCount;
        } else {
          avg.Rolls = Mobs->Mob[j].Level;
          avg.Die = 8;
          avg.Modifier = 0;
        }
        fprintf(ofp, "%dd%d+%d\n", avg.Rolls, avg.Die, avg.Modifier);
        /* Wiley didn't have languages -- use common */
	/* might convert to race later... */
        fprintf(ofp, "%s~\n", flag_string( 0, afk_lang_names ) );
        fprintf(ofp, "%s~\n", flag_string( 0, afk_lang_names ) );
        fprintf(ofp, "%s~\n", afk_npc_position[afk_convertposition(Mobs->Mob[j].Position)]);
        fprintf(ofp, "%s~\n", afk_npc_position[afk_convertposition(Mobs->Mob[j].DefaultPosition)]);
        fprintf(ofp, "%s~\n", afk_sex[afk_convertsex(Mobs->Mob[j].Sex)]);
	fprintf(ofp, "%s~\n", afk_npc_race[get_npc_race(race_name(Mobs->Mob[j].Race))]);
	fprintf(ofp, "%s~\n", afk_npc_class[get_npc_class(class_name(Mobs->Mob[j].Class))]);
        /* Wiley doesn't have limbs */
	/* fprintf(ofp, "%s~\n", flag_string( pMobIndex->xflags, part_flags) ); */
	fprintf(ofp, "%s~\n", flag_string( 0, afk_part_flags) );

        ImmValue = afk_convert_imm(Mobs->Mob[j].Resistance);
	fprintf(ofp, "%s~\n", ext_flag_string( &ImmValue, afk_ris_flags) ); // resist
        ImmValue = afk_convert_imm(Mobs->Mob[j].Immunity);
	fprintf(ofp, "%s~\n", ext_flag_string( &ImmValue, afk_ris_flags) ); // immune
        ImmValue = afk_convert_imm(Mobs->Mob[j].Susceptible);
	fprintf(ofp, "%s~\n", ext_flag_string( &ImmValue, afk_ris_flags) ); // susc
        ImmValue = afk_convert_imm(0);
	fprintf(ofp, "%s~\n", ext_flag_string( &ImmValue, afk_ris_flags) ); // absorb -- Wiley has none

	//fprintf(ofp, "%s~\n", ext_flag_string(&pMobIndex->attacks, attack_flags) );
	//fprintf(ofp, "%s~\n", ext_flag_string(&pMobIndex->defenses, defense_flags) );
        ImmValue = afk_convert_imm(0);
	fprintf(ofp, "%s~\n", ext_flag_string(&ImmValue, afk_attack_flags) );
        ImmValue = afk_convert_imm(0);
	fprintf(ofp, "%s~\n", ext_flag_string(&ImmValue, afk_defense_flags) );

	/* Wiley has no mudprogs! */
      }
    }
/*
    for ( vnum = tarea->low_vnum; vnum <= tarea->hi_vnum; vnum++ )
    {
	if ( (pMobIndex = get_mob_index( vnum )) == NULL )
	  continue;
	if ( IS_ACT_FLAG( pMobIndex, ACT_DELETED ) )
	{
	   log_info( "Mob Vnum %d skipped - flagged for deletion.", vnum );
	   continue;
	}
	if ( install )
	  REMOVE_ACT_FLAG( pMobIndex, ACT_PROTOTYPE );
	fprintf( fpout, "#%d\n", vnum	);
	fprintf( fpout, "%s~\n", pMobIndex->player_name	);
	fprintf( fpout, "%s~\n", pMobIndex->short_descr	);
	fprintf( fpout, "%s~\n", strip_cr(pMobIndex->long_descr) );
	fprintf( fpout, "%s~\n", strip_cr(pMobIndex->description) );
	fprintf( fpout, "%s~\n", ext_flag_string(&pMobIndex->act, act_flags) );
        fprintf( fpout, "%s~\n", ext_flag_string(&pMobIndex->affected_by, a_flags) );
	fprintf( fpout, "%d %d %d %d %d %d %d %f\n", pMobIndex->alignment,
				pMobIndex->gold, 0, pMobIndex->height, pMobIndex->weight,
				pMobIndex->max_move, pMobIndex->max_mana, pMobIndex->numattacks );
	fprintf( fpout, "%d %d %d %d ", pMobIndex->level, pMobIndex->mobthac0, pMobIndex->ac, pMobIndex->hitplus );
	fprintf( fpout, "%dd%d+%d\n",	pMobIndex->damnodice, pMobIndex->damsizedice, pMobIndex->damplus );
	fprintf( fpout, "%s~\n", flag_string( pMobIndex->speaks, lang_names ) );
	fprintf( fpout, "%s~\n", flag_string( pMobIndex->speaking, lang_names ) );
	fprintf( fpout, "%s~\n", npc_position[pMobIndex->position] );
	fprintf( fpout, "%s~\n", npc_position[pMobIndex->defposition] );
	fprintf( fpout, "%s~\n", npc_sex[pMobIndex->sex] );
	fprintf( fpout, "%s~\n", npc_race[pMobIndex->race] );
	fprintf( fpout, "%s~\n", npc_class[pMobIndex->class] );
	fprintf( fpout, "%s~\n", flag_string( pMobIndex->xflags, part_flags) );
	fprintf( fpout, "%s~\n", flag_string( pMobIndex->resistant, ris_flags) );
	fprintf( fpout, "%s~\n", flag_string( pMobIndex->immune, ris_flags) );
	fprintf( fpout, "%s~\n", flag_string( pMobIndex->susceptible, ris_flags) );
	fprintf( fpout, "%s~\n", flag_string( pMobIndex->absorb, ris_flags) );
	fprintf( fpout, "%s~\n", ext_flag_string(&pMobIndex->attacks, attack_flags) );
      fprintf( fpout, "%s~\n", ext_flag_string(&pMobIndex->defenses, defense_flags) );
	
	if ( pMobIndex->mudprogs )
	{
	  for ( mprog = pMobIndex->mudprogs; mprog; mprog = mprog->next )
		fprintf( fpout, "> %s %s~\n%s~\n", mprog_type_to_name( mprog->type ), mprog->arglist, strip_cr(mprog->comlist) );
	  fprintf( fpout, "%s", "|\n" );	  
	}
    }
*/
    fprintf( ofp, "%s", "#0\n\n\n" );

/*
#4449
Last mob~
a newly created last mob~
Some god abandoned a newly created last mob here.
~
~
npc proto~
~
0 0 0 0 0 150 100 0.000000
1 21 0 0 0d0+0
common~
common~
standing~
standing~
neuter~
human~
warrior~
~
~
~
~
~
~
~
#0


*/


/* OBJECTS */


    fprintf( ofp, "%s", "#OBJECTS\n" );
    for(j= 0; j< Objects->Count; j++) {
      if((remap_zone_vnum(Zones, Objects->Object[j].Zone) == i) ||
         ((Objects->Object[j].Number >= (!i? 0: Zones->Zone[i-1].Top+1)) &&
          (Objects->Object[j].Number <= Zones->Zone[i].Top)
      )) {
        EXT_BV ObjValue;
        int WearValue = 0;
        int *val = NULL;
        int kval[11];

        memset(&ObjValue, '\0', sizeof(EXT_BV));

        if(Verbose)
          fprintf(stderr, "  (%d) Writing \"%s\"[#%d]\n", j,
                  Objects->Object[j].Name, Objects->Object[j].Number);
        else if(!Quiet)
          spin(stderr);

        fprintf(ofp, "#%d\n", Objects->Object[j].Number);
        fprintf(ofp, "%s~\n", strip_cr(Objects->Object[j].Name)); /* called keywords now */
        fprintf(ofp, "%s~\n", strip_cr(Objects->Object[j].ShortDesc));
        fprintf(ofp, "%s~\n", strip_cr(Objects->Object[j].Description)); /* LongDesc */
        fprintf(ofp, "%s~\n", strip_cr(Objects->Object[j].ActionDesc));
        /* item type, extra flags, wear flags */
        fprintf(ofp, "%s~\n", strip_cr(afk_o_types[afk_convertobjtype(Objects->Object[j].Type)]));
        ObjValue = afk_convert_oflags(Objects->Object[j].Flags.Extra);
        fprintf(ofp, "%s~\n", strip_cr(ext_flag_string(&ObjValue, afk_o_flags)));
        WearValue = afk_convert_wflags(Objects->Object[j].Flags.Wear);
	fprintf(ofp, "%s~\n", strip_cr(flag_string(WearValue, afk_w_flags)));
        /* magic flags? Wiley don't need to steenkin magic flags! */
        fprintf(ofp, "%s~\n", strip_cr(flag_string(0, afk_mag_flags)));

        /* Now it gets ugly... flag values */
        val = afk_convert_item_values( Objects->Object[j].Type,
                                       Objects->Object[j].Flags.Value[0],
                                       Objects->Object[j].Flags.Value[1],
                                       Objects->Object[j].Flags.Value[2],
                                       Objects->Object[j].Flags.Value[3] );
        /* Tricky part... spell numbers need to be -1 here, and the NAMES need to be
           added below */
        for(k=0; k< 11; k++) /* Keep a backup copy */
          kval[k] = val[k];
        switch(afk_convertobjtype(Objects->Object[j].Type)) {
          case AFK_ITEM_PILL:
          case AFK_ITEM_POTION:
          case AFK_ITEM_SCROLL:
            val[1] = HAS_SPELL_INDEX;
            val[2] = HAS_SPELL_INDEX;
            val[3] = HAS_SPELL_INDEX;
            break;
          case AFK_ITEM_STAFF:
          case AFK_ITEM_WAND:
            val[3] = HAS_SPELL_INDEX;
            break;
          case AFK_ITEM_SALVE:
            val[4] = HAS_SPELL_INDEX;
            val[5] = HAS_SPELL_INDEX;
            break;
        }
	fprintf(ofp, "%d %d %d %d %d %d %d %d %d %d %d\n",
                     val[0], val[1], val[2], val[3],
                     val[4], val[5], val[6], val[7],
                     val[8], val[9], val[10]              );

	fprintf(ofp, "%d %d %d %d %d %s %s %s\n",
           Objects->Object[j].Weight,
           Objects->Object[j].Value,
           Objects->Object[j].Rent,
           9999, /* Unlimited */
           0, /* Layers aren't supported */
           "None", "None", "None" /* Neither are sockets */
           );

        switch(afk_convertobjtype(Objects->Object[j].Type)) {
          case AFK_ITEM_PILL: 
          case AFK_ITEM_POTION:
          case AFK_ITEM_SCROLL:
            fprintf(ofp, "'%s' '%s' '%s'\n",
                    kval[1] != -1 ? afk_skill_names[kval[1]] : "NONE",
                    kval[2] != -1 ? afk_skill_names[kval[2]] : "NONE",
                    kval[3] != -1 ? afk_skill_names[kval[3]] : "NONE" );
            break;
          case AFK_ITEM_STAFF:
          case AFK_ITEM_WAND:
            fprintf(ofp, "'%s'\n",
                    kval[3] != -1 ? afk_skill_names[kval[3]] : "NONE" );
            break;
          case AFK_ITEM_SALVE:
            fprintf(ofp, "'%s' '%s'\n",
                    kval[4] != -1 ? afk_skill_names[kval[4]] : "NONE",
                    kval[5] != -1 ? afk_skill_names[kval[5]] : "NONE" );
            break;
        }

        for(k = 0; k < Objects->Object[j].ExtraCount; k++) {
          fprintf(ofp, "E\n");
          for(m = 0; m < Objects->Object[j].Extra[k].Keyword->Count; m++) {
            fprintf(ofp, "%s%s", Objects->Object[j].Extra[k].Keyword->Word[m],
                                 (m == Objects->Object[j].Extra[k].Keyword->Count - 1)
                                   ? ""
                                   : " ");
          }
          fprintf(ofp, "~\n%s~\n", strip_cr(Objects->Object[j].Extra[k].Description));
        }

        /* Don't have to worry about weaponspell applies, no such puppies! */
        for(k = 0; k < Objects->Object[j].AffectCount; k++)
          fprintf(ofp, "A\n%d %d\n", Objects->Object[j].Affect[k].location,
                                     Objects->Object[j].Affect[k].modifier);

       /* No mobprogs either! */
      }
    }
    fprintf( ofp, "%s", "#0\n\n\n" );
/*
    for ( vnum = tarea->low_vnum; vnum <= tarea->hi_vnum; vnum++ )
    {
	if ( (pObjIndex = get_obj_index( vnum )) == NULL )
	  continue;
	if ( IS_OBJ_STAT( pObjIndex, ITEM_DELETED ) )
	{
	   log_info( "Object Vnum %d skipped - flagged for deletion.", vnum );
	   continue;
	}
	if ( install )
	  REMOVE_OBJ_STAT( pObjIndex, ITEM_PROTOTYPE );
	fprintf( fpout, "#%d\n", vnum	);
	fprintf( fpout, "%s~\n", pObjIndex->name );
	fprintf( fpout, "%s~\n", pObjIndex->short_descr );
	fprintf( fpout, "%s~\n", pObjIndex->description	);
	fprintf( fpout, "%s~\n", pObjIndex->action_desc	);
	fprintf( fpout, "%s~\n", o_types[pObjIndex->item_type] );
	fprintf( fpout, "%s~\n", ext_flag_string(&pObjIndex->extra_flags, o_flags) );
	fprintf( fpout, "%s~\n", flag_string(pObjIndex->weaafk_r_flags, w_flags) );
      fprintf( fpout, "%s~\n", flag_string(pObjIndex->magic_flags, mag_flags) );
	
	val0 = pObjIndex->value[0];
	val1 = pObjIndex->value[1];
	val2 = pObjIndex->value[2];
	val3 = pObjIndex->value[3];
	val4 = pObjIndex->value[4];
	val5 = pObjIndex->value[5];
	val6 = pObjIndex->value[6];
	val7 = pObjIndex->value[7];
	val8 = pObjIndex->value[8];
	val9 = pObjIndex->value[9];
	val10 = pObjIndex->value[10];
	switch( pObjIndex->item_type )
	{
	case ITEM_PILL:
	case ITEM_POTION:
	case ITEM_SCROLL:
	      if( IS_VALID_SN( val1 ) )
		val1 = HAS_SPELL_INDEX;
	      if( IS_VALID_SN( val2 ) )
		val2 = HAS_SPELL_INDEX;
	      if( IS_VALID_SN( val3 ) )
		val3 = HAS_SPELL_INDEX;
	    break;
	case ITEM_STAFF:
	case ITEM_WAND:
	      if( IS_VALID_SN( val3 ) )
		val3 = HAS_SPELL_INDEX;
	    break;
	case ITEM_SALVE:
	      if( IS_VALID_SN( val4 ) )
		val4 = HAS_SPELL_INDEX;
	      if( IS_VALID_SN( val5 ) )
		val5 = HAS_SPELL_INDEX;
	    break;
	}
	fprintf( fpout, "%d %d %d %d %d %d %d %d %d %d %d\n",
	   val0, val1, val2, val3, val4, val5, val6, val7, val8, val9, val10 );

	fprintf( fpout, "%d %d %d %d %d %s %s %s\n", pObjIndex->weight, pObjIndex->cost, pObjIndex->rent,
	   pObjIndex->limit, pObjIndex->layers,
	   pObjIndex->socket[0] ? pObjIndex->socket[0] : "None",
	   pObjIndex->socket[1] ? pObjIndex->socket[1] : "None",
	   pObjIndex->socket[2] ? pObjIndex->socket[2] : "None" );

    	switch ( pObjIndex->item_type )
    	{
    	case ITEM_PILL: 
    	case ITEM_POTION:
    	case ITEM_SCROLL:
           	fprintf( fpout, "'%s' '%s' '%s'\n",
		   IS_VALID_SN( pObjIndex->value[1] ) ? skill_table[pObjIndex->value[1]]->name : "NONE",
		   IS_VALID_SN( pObjIndex->value[2] ) ? skill_table[pObjIndex->value[2]]->name : "NONE",
		   IS_VALID_SN( pObjIndex->value[3] ) ? skill_table[pObjIndex->value[3]]->name : "NONE" );
        	break;
    	case ITEM_STAFF:
    	case ITEM_WAND:
            fprintf( fpout, "'%s'\n",
		   IS_VALID_SN( pObjIndex->value[3] ) ? skill_table[pObjIndex->value[3]]->name : "NONE" );
        	break;
    	case ITEM_SALVE:
            fprintf( fpout, "'%s' '%s'\n",
		   IS_VALID_SN( pObjIndex->value[4] ) ? skill_table[pObjIndex->value[4]]->name : "NONE",
		   IS_VALID_SN( pObjIndex->value[5] ) ? skill_table[pObjIndex->value[5]]->name : "NONE" );
        break;
    	}
	for ( ed = pObjIndex->first_extradesc; ed; ed = ed->next )
	   fprintf( fpout, "E\n%s~\n%s~\n",	ed->keyword, strip_cr( ed->description )	);

	for ( paf = pObjIndex->first_affect; paf; paf = paf->next )
	   fprintf( fpout, "A\n%d %d\n", paf->location,
	     ((paf->location == APPLY_WEAPONSPELL
	    || paf->location == APPLY_WEARSPELL
	    || paf->location == APPLY_REMOVESPELL
	    || paf->location == APPLY_STRIPSN
	    || paf->location == APPLY_RECURRINGSPELL)
	    && IS_VALID_SN(paf->modifier))
	    ? skill_table[paf->modifier]->slot : paf->modifier		);

	if ( pObjIndex->mudprogs )
	{
	  for ( mprog = pObjIndex->mudprogs; mprog; mprog = mprog->next )
		fprintf( fpout, "> %s %s~\n%s~\n", mprog_type_to_name( mprog->type ), mprog->arglist, strip_cr(mprog->comlist) );
	  fprintf( fpout, "%s", "|\n" );	  
	}
    }
*/

/*
#4449
Last obj~
a newly created last obj~
Some god dropped a newly created last obj here.~
~
trash~
proto~
~
~
0 0 0 0 0 0 0 0 0 0 0
1 0 0 9999 0 None None None
#0


*/


/* ROOMS */


    fprintf( ofp, "%s", "#ROOMS\n" );
    for(j= 0; j< Rooms->Count; j++) {
      if((remap_zone_vnum(Zones, Rooms->Room[j].Zone) == i) ||
         ((Rooms->Room[j].Number >= (!i? 0: Zones->Zone[i-1].Top+1)) &&
          (Rooms->Room[j].Number <= Zones->Zone[i].Top)
      )) {
        EXT_BV RoomValue;
        EXT_BV ExitValue;

        memset(&RoomValue, '\0', sizeof(EXT_BV));
        memset(&ExitValue, '\0', sizeof(EXT_BV));

        if(Verbose)
          fprintf(stderr, "  (%d) Writing \"%s\"[#%d]\n", j,
                  Rooms->Room[j].Name, Rooms->Room[j].Number);
        else if(!Quiet)
          spin(stderr);

        fprintf(ofp, "#%d\n", Rooms->Room[j].Number);
        fprintf(ofp, "%s~\n", strip_cr(Rooms->Room[j].Name));
        fprintf(ofp, "%s~\n", strip_cr(Rooms->Room[j].Description));
        fprintf(ofp, "%s~\n", ""); /* strip_cr( room->nitedesc ) // Night Description */
        if ( Rooms->Room[j].Sector == SECT_TELEPORT ) {
          fprintf(ofp, "%s~\n", strip_cr(afk_sect_types[afk_convertsector[Rooms->Room[j].TeleportSector]]));
        } else {
          fprintf(ofp, "%s~\n", strip_cr(afk_sect_types[afk_convertsector[Rooms->Room[j].Sector]]));
        }

        /* First, convert bits */
        RoomValue = afk_convert_rflags(Rooms->Room[j].Flags);
        fprintf(ofp, "%s~\n", strip_cr(ext_flag_string(&RoomValue, afk_r_flags)));

        if ( Rooms->Room[j].Sector == SECT_TELEPORT &&
             Rooms->Room[j].TeleportTime > 0 &&
             Rooms->Room[j].TeleportTo > 0 ) {
          /* Wiley has no tunnels */
          fprintf( ofp, "1 %d %d %d\n", Rooms->Room[j].TeleportTime, Rooms->Room[j].TeleportTo, 0 );
        } else {
          fprintf( ofp, "%d\n", 0 );
        }

        for(k= 0; k< Rooms->Room[j].ExitCount; k++) {
          switch(Rooms->Room[j].Exit[k].Error) {
            case EXIT_OK:
            case EXIT_NON_EUCLIDEAN:
            case EXIT_ONE_WAY:
              /* Wiley has no portals */
              /*
              if ( IS_EXIT_FLAG(xit, EX_PORTAL) ) // don't fold portals
                continue;
              */
              fprintf( ofp, "%s", "D\n" );
              fprintf( ofp, "%s~\n", strip_cr(afk_dir_name[Rooms->Room[j].Exit[k].Direction] ));
              fprintf( ofp, "%s~\n", strip_cr(Rooms->Room[j].Exit[k].Description));
              if(Rooms->Room[j].Exit[k].Keyword->Count > 0) {
                fprintf(ofp, "%s", strip_cr(Rooms->Room[j].Exit[k].Keyword->Word[0]));
                for(x= 1; x< Rooms->Room[j].Exit[k].Keyword->Count; x++) {
                  fprintf(ofp, " %s", strip_cr(Rooms->Room[j].Exit[k].Keyword->Word[x]));
                }
              }
              fprintf( ofp, "~\n" );

              ExitValue = afk_convert_exflags(Rooms->Room[j].Exit[k].Type);
              fprintf( ofp, "%s~\n", ext_flag_string(&ExitValue, afk_ex_flags) );

              /* Wiley has no PULL type exits */
              /*
              if ( xit->pull )
                fprintf( fpout, "%d %d %d %d %d %d\n", xit->key, xit->vnum, xit->x, xit->y, xit->pulltype, xit->pull );
              else
              */
              /* Coordinates do not exist in Wiley */
              fprintf( ofp, "%d %d %d %d\n", Rooms->Room[j].Exit[k].KeyNumber, Rooms->Room[j].Exit[k].Room, -1, -1 );
          }
        }

        for(k= 0; k< Rooms->Room[j].ExtraCount; k++) {
          fprintf(ofp, "E\n");
          if(Rooms->Room[j].Extra[k].Keyword->Count > 0) {
            fprintf(ofp, "%s", strip_cr(Rooms->Room[j].Extra[k].Keyword->Word[0]));
            for(x= 1; x< Rooms->Room[j].Extra[k].Keyword->Count; x++) {
              fprintf(ofp, " %s", strip_cr(Rooms->Room[j].Extra[k].Keyword->Word[x]));
            }
          }
          fprintf(ofp, "~\n%s~\n", strip_cr(Rooms->Room[j].Extra[k].Description));
        }

        /* Wiley has no mudprogs */
        /*
	if ( room->mudprogs )
	{
	  for ( mprog = room->mudprogs; mprog; mprog = mprog->next )
		fprintf( ofp, "> %s %s~\n%s~\n", mprog_type_to_name( mprog->type ), mprog->arglist, strip_cr(mprog->comlist) );
	  fprintf( ofp, "%s", "|\n" );	  
	}
        */

        fprintf( ofp, "%s", "S\n" );
      }
    }
    fprintf( ofp, "%s", "#0\n\n\n" );

    /* save resets   */
    fprintf( ofp, "%s", "#RESETS\n" );
/*
    for ( treset = tarea->first_reset; treset; treset = treset->next )
    {
	switch( treset->command ) // extra arg1 arg2 arg3
	{
	  default:  case '*': break;
	  case 'm': case 'M':
	  case 'o': case 'O':
	  case 'p': case 'P':
	  case 'e': case 'E':
	  case 'd': case 'D':
	  case 't': case 'T':
	  case 'b': case 'B':
	  case 'h': case 'H':
		fprintf( ofp, "%c %d %d %d %d\n", UPPER(treset->command),
		treset->extra, treset->arg1, treset->arg2, treset->arg3 );
		break;
	  case 'g': case 'G':
	  case 'r': case 'R':
		fprintf( ofp, "%c %d %d %d\n", UPPER(treset->command), treset->extra, treset->arg1, treset->arg2 );
		break;
	}
    }
*/
    fprintf( ofp, "%s", "S\n\n\n" );

    /* save shops */
    fprintf( ofp, "%s", "#SHOPS\n" );
/*
    for ( vnum = tarea->low_vnum; vnum <= tarea->hi_vnum; vnum++ )
    {
	if ( (pMobIndex = get_mob_index( vnum )) == NULL )
	  continue;
	if ( (pShop = pMobIndex->pShop) == NULL )
	  continue;
	fprintf( ofp, " %d   %2d %2d %2d %2d %2d   %3d %3d",
		 pShop->keeper, pShop->buy_type[0], pShop->buy_type[1], pShop->buy_type[2],
		 pShop->buy_type[3], pShop->buy_type[4], pShop->profit_buy, pShop->profit_sell );
	fprintf( ofp, "        %2d %2d    ; %s\n", pShop->open_hour, pShop->close_hour, pMobIndex->short_descr );
    }
*/
    fprintf( ofp, "%s", "0\n\n\n" );

    /* save repair shops */
    fprintf( ofp, "%s", "#REPAIRS\n" );
/*
    for ( vnum = tarea->low_vnum; vnum <= tarea->hi_vnum; vnum++ )
    {
	if ( (pMobIndex = get_mob_index( vnum )) == NULL )
	  continue;
	if ( (pRepair = pMobIndex->rShop) == NULL )
	  continue;
	fprintf( ofp, " %d   %2d %2d %2d         %3d %3d",
		 pRepair->keeper, pRepair->fix_type[0], pRepair->fix_type[1],
		 pRepair->fix_type[2], pRepair->profit_fix, pRepair->shop_type );
	fprintf( ofp, "        %2d %2d    ; %s\n", pRepair->open_hour, pRepair->close_hour, pMobIndex->short_descr );
    }
*/
    fprintf( ofp, "%s", "0\n\n\n" );

    /* save specials */
    fprintf( ofp, "%s", "#SPECIALS\n" );
/*
    for ( vnum = tarea->low_vnum; vnum <= tarea->hi_vnum; vnum++ )
    {
      if( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
            if ( pMobIndex->spec_fun )
	      fprintf( ofp, "M  %d %s\n", pMobIndex->vnum, pMobIndex->spec_funname );
    }
*/
    fprintf( ofp, "%s", "S\n\n\n" );

    /* END */

    fprintf(ofp, "#$\n");
    fclose(ofp);
    if(Verbose)
      fprintf(stderr, "done.\n");
    else if(!Quiet) {
      fprintf(stderr, "done.\r");
      fflush(stderr);
    }
  }
  if(!Quiet)
    fprintf(stderr, "\n");

#if 0
  if(!Quiet)
    fprintf(stderr, "Generating zone list...");
  qsort(ZoneSort, Zones->Count, sizeof(pair), afk_qcmp_zone_vnum);
  sprintf(filename, "%s/%s/area.lst", OutputDir, AFK_SUBDIR);
  listfp= open_file(filename, "w");
  for(i= 0; i< Zones->Count; i++) {
    if(!Quiet)
      spin(stderr);
    sprintf(filename, "%s_%d.are", remap_name(Zones->Zone[ZoneSort[i].x].Name),
            Zones->Zone[i].Number);
    fprintf(listfp, "%s\n", filename);
  }
#endif
  fprintf(listfp, "$\n");
  fclose(listfp);
  if(!Quiet)
    fprintf(stderr, "done.\n");
}


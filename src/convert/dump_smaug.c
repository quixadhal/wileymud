#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "include/structs.h"
#include "include/main.h"
#include "include/utils.h"

#include "include/parse_wiley.h"

#define _DUMP_SMAUG_C
#include "include/dump_smaug.h"
#include "include/smaug_skills.h"

#define MSL 8192

const int                               smaug_convertsectors[] = { SMAUG_SECT_INSIDE,
    SMAUG_SECT_INSIDE,
    SMAUG_SECT_CITY,
    SMAUG_SECT_FIELD,
    SMAUG_SECT_FOREST,
    SMAUG_SECT_HILLS,
    SMAUG_SECT_MOUNTAIN,
    SMAUG_SECT_WATER_SWIM,
    SMAUG_SECT_WATER_NOSWIM,
    SMAUG_SECT_AIR,
    SMAUG_SECT_UNDERWATER
};

const int                              *smaug_convertsector = &smaug_convertsectors[1];

extern const char                       terrains[];
extern const char                      *terrain;
extern const int                        exit_dir_to_bit[];

int smaug_qcmp_zone_vnum(const void *a, const void *b)
{
    return (((const pair *)b)->y - ((const pair *)a)->y);
}

char                                   *const smaug_dir_name[] = {
    "north", "east", "south", "west", "up", "down",
    "northeast", "northwest", "southeast", "southwest", "somewhere"
};

char                                   *const smaug_area_flags[] = {
    "nopkill", "freekill", "noteleport", "spelllimit", "prototype", "r5", "r6", "r7", "r8",
    "r9", "r10", "r11", "r12", "r13", "r14", "r15", "r16", "r17",
    "r18", "r19", "r20", "r21", "r22", "r23", "r24",
    "r25", "r26", "r27", "r28", "r29", "r30", "r31"
};

char                                   *const smaug_ex_flags[] = {
    "isdoor", "closed", "locked", "secret", "swim", "pickproof", "fly", "climb",
    "dig", "eatkey", "nopassdoor", "hidden", "passage", "portal", "r1", "r2",
    "can_climb", "can_enter", "can_leave", "auto", "noflee", "searchable",
    "bashed", "bashproof", "nomob", "window", "can_look", "isbolt", "bolted"
};

char                                   *const smaug_r_flags[] = {
    "dark", "death", "nomob", "indoors", "lawful", "neutral", "chaotic",
    "nomagic", "tunnel", "private", "safe", "solitary", "petshop", "norecall",
    "donation", "nodropall", "silence", "logspeech", "nodrop", "clanstoreroom",
    "nosummon", "noastral", "teleport", "teleshowdesc", "nofloor",
    "nosupplicate", "arena", "nomissile", "r4", "r5", "prototype", "dnd", "bfs_mark"
};

/* The names of varous sector types. Used in the OLC code in addition to
 * the display_map function and probably some other places I've long
 * since forgotten by now.
 */
char                                   *const smaug_sect_types[] = {
    "inside", "city", "field", "forest", "hills", "mountain", "water_swim",
    "water_noswim", "underwater", "air", "desert", "dunno", "oceanfloor",
    "underground", "lava", "swamp", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
    "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "r16"
};

char                                   *const smaug_o_types[] = {
    "none", "light", "scroll", "wand", "staff", "weapon", "_fireweapon", "_missile",
    "treasure", "armor", "potion", "_worn", "furniture", "trash", "_oldtrap",
    "container", "_note", "drinkcon", "key", "food", "money", "pen", "boat",
    "corpse", "corpse_pc", "fountain", "pill", "blood", "bloodstain",
    "scraps", "pipe", "herbcon", "herb", "incense", "fire", "book", "switch",
    "lever", "pullchain", "button", "dial", "rune", "runepouch", "match", "trap",
    "map", "portal", "paper", "tinder", "lockpick", "spike", "disease", "oil",
    "fuel", "_empty1", "_empty2", "missileweapon", "projectile", "quiver", "shovel",
    "salve", "cook", "keyring", "odor", "chance", "mix"
};

char                                   *const smaug_o_flags[] = {
    "glow", "hum", "dark", "loyal", "evil", "invis", "magic", "nodrop", "bless",
    "antigood", "antievil", "antineutral", "noremove", "inventory",
    "antimage", "antithief", "antiwarrior", "anticleric", "organic", "metal",
    "donation", "clanobject", "clancorpse", "antivampire", "antidruid",
    "hidden", "poisoned", "covering", "deathrot", "buried", "prototype",
    "nolocate", "groundrot", "lootable", "personal", "multi_invoke", "enchanted"
};

char                                   *const smaug_w_flags[] = {
    "take", "finger", "neck", "body", "head", "legs", "feet", "hands", "arms",
    "shield", "about", "waist", "wrist", "wield", "hold", "_dual_", "ears", "eyes",
    "missile", "back", "face", "ankle", "r4", "r5", "r6",
    "r7", "r8", "r9", "r10", "r11", "r12", "r13"
};

char                                   *const smaug_mag_flags[] = {
    "returning", "backstabber", "bane", "loyal", "haste", "drain",
    "lightning_blade", "r7", "r8", "r9", "r10", "r11", "r12", "r13",
    "r14", "r15", "r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23",
    "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31"
};

char                                   *const smaug_act_flags[] = {
    "npc", "sentinel", "scavenger", "r1", "r2", "aggressive", "stayarea",
    "wimpy", "pet", "train", "practice", "immortal", "deadly", "polyself",
    "meta_aggr", "guardian", "running", "nowander", "mountable", "mounted",
    "scholar", "secretive", "hardhat", "mobinvis", "noassist", "autonomous",
    "pacifist", "noattack", "annoying", "statshield", "prototype"
};

char                                   *const smaug_a_flags[] = {
    "blind", "invisible", "detect_evil", "detect_invis", "detect_magic",
    "detect_hidden", "hold", "sanctuary", "faerie_fire", "infrared", "curse",
    "_flaming", "poison", "protect", "_paralysis", "sneak", "hide", "sleep",
    "charm", "flying", "pass_door", "floating", "truesight", "detect_traps",
    "scrying", "fireshield", "shockshield", "r1", "iceshield", "possess",
    "berserk", "aqua_breath", "recurringspell", "contagious", "acidmist",
    "venomshield"
};

char                                   *const smaug_lang_names[] = {
    "common", "elvish", "dwarven", "pixie",
    "ogre", "orcish", "trollese", "rodent",
    "insectoid", "mammal", "reptile",
    "dragon", "spiritual", "magical",
    "goblin", "god", "ancient", "halfling",
    "clan", "gith", "gnome", "", "", "", "",
    "", "", "", "", "", "", "", ""			       /* pad to 32 for compat with smaug_flag_string */
};

char                                   *const smaug_npc_position[] = {
    "dead", "mortal", "incapacitated", "stunned", "sleeping",
    "resting", "sitting", "berserk", "aggressive", "fighting", "defensive",
    "evasive", "standing", "mounted", "shove", "drag"
};

char                                   *const smaug_sex[] = {
    "neuter", "male", "female"
};

char                                   *const smaug_npc_race[] = {	/* starting from 0.... */
    "human", "elf", "dwarf", "halfling", "pixie", "vampire", "half-ogre",
    "half-orc", "half-troll", "half-elf", "gith", "drow", "sea-elf",
    "lizardman", "gnome", "r5", "r6", "r7", "r8", "troll",
    "ant", "ape", "baboon", "bat", "bear", "bee",
    "beetle", "boar", "bugbear", "cat", "dog", "dragon", "ferret", "fly",
    "gargoyle", "gelatin", "ghoul", "gnoll", "gnome", "goblin", "golem",
    "gorgon", "harpy", "hobgoblin", "kobold", "lizardman", "locust",
    "lycanthrope", "minotaur", "mold", "mule", "neanderthal", "ooze", "orc",
    "rat", "rustmonster", "shadow", "shapeshifter", "shrew", "shrieker",
    "skeleton", "slime", "snake", "spider", "stirge", "thoul", "troglodyte",
    "undead", "wight", "wolf", "worm", "zombie", "bovine", "canine", "feline",
    "porcine", "mammal", "rodent", "avis", "reptile", "amphibian", "fish",
    "crustacean", "insect", "spirit", "magical", "horse", "animal", "humanoid",
    "monster", "god"
};

char                                   *const smaug_npc_class[] = {
    "mage", "cleric", "thief", "warrior", "vampire", "druid", "ranger",
    "augurer", "paladin", "nephandi", "savage", "pirate", "pc12", "pc13",
    "pc14", "pc15", "pc16", "pc17", "pc18", "pc19",
    "baker", "butcher", "blacksmith", "mayor", "king", "queen"
};

char                                   *const smaug_part_flags[] = {
    "head", "arms", "legs", "heart", "brains", "guts", "hands", "feet", "fingers",
    "ear", "eye", "long_tongue", "eyestalks", "tentacles", "fins", "wings",
    "tail", "scales", "claws", "fangs", "horns", "tusks", "tailattack",
    "sharpscales", "beak", "haunches", "hooves", "paws", "forelegs", "feathers",
    "r1", "r2"
};

char                                   *const smaug_attack_flags[] = {
    "bite", "claws", "tail", "sting", "punch", "kick", "trip", "bash", "stun",
    "gouge", "backstab", "feed", "drain", "firebreath", "frostbreath",
    "acidbreath", "lightnbreath", "gasbreath", "poison", "nastypoison", "gaze",
    "blindness", "causeserious", "earthquake", "causecritical", "curse",
    "flamestrike", "harm", "fireball", "colorspray", "weaken", "spiralblast"
};

char                                   *const smaug_defense_flags[] = {
    "parry", "dodge", "heal", "curelight", "cureserious", "curecritical",
    "dispelmagic", "dispelevil", "sanctuary", "fireshield", "shockshield",
    "shield", "bless", "stoneskin", "teleport", "monsum1", "monsum2", "monsum3",
    "monsum4", "disarm", "iceshield", "grip", "truesight", "acidmist", "venomshield"
};

char                                   *const smaug_ris_flags[] = {
    "fire", "cold", "electricity", "energy", "blunt", "pierce", "slash", "acid",
    "poison", "drain", "sleep", "charm", "hold", "nonmagic", "plus1", "plus2",
    "plus3", "plus4", "plus5", "plus6", "magic", "paralysis", "r1", "r2", "r3",
    "r4", "r5", "r6", "r7", "r8", "r9", "r10"
};

void smaug_convert_attack(EXT_BV *NewValue, int OldValue)
{
    switch (OldValue) {
	case TYPE_HIT:
	    break;					       // xSET_BIT(*NewValue, smaug_get_attackflag("hit"));
	    // break;
	case TYPE_BLUDGEON:
	    xSET_BIT(*NewValue, smaug_get_attackflag("punch"));
	    break;
	case TYPE_PIERCE:
	    break;					       // xSET_BIT(*NewValue, smaug_get_attackflag("hit"));
	    // break;
	case TYPE_SLASH:
	    break;					       // xSET_BIT(*NewValue, smaug_get_attackflag("hit"));
	    // break;
	case TYPE_WHIP:
	    xSET_BIT(*NewValue, smaug_get_attackflag("tail"));
	    break;
	case TYPE_CLAW:
	    xSET_BIT(*NewValue, smaug_get_attackflag("claws"));
	    break;
	case TYPE_BITE:
	    xSET_BIT(*NewValue, smaug_get_attackflag("bite"));
	    break;
	case TYPE_STING:
	    xSET_BIT(*NewValue, smaug_get_attackflag("sting"));
	    break;
	case TYPE_CRUSH:
	    xSET_BIT(*NewValue, smaug_get_attackflag("bash"));
	    break;
	case TYPE_CLEAVE:
	    xSET_BIT(*NewValue, smaug_get_attackflag("gouge"));
	    break;
	case TYPE_STAB:
	    break;					       // xSET_BIT(*NewValue, smaug_get_attackflag("hit"));
	    // break;
	case TYPE_SMASH:
	    xSET_BIT(*NewValue, smaug_get_attackflag("bash"));
	    break;
	case TYPE_SMITE:
	    xSET_BIT(*NewValue, smaug_get_attackflag("stun"));
	    break;
	case TYPE_SUFFERING:
	    xSET_BIT(*NewValue, smaug_get_attackflag("poison"));
	    break;
	case TYPE_HUNGER:
	    break;					       // xSET_BIT(*NewValue, smaug_get_attackflag("hit"));
	    // break;
	default:
	    break;
    }
}

EXT_BV smaug_convert_imm(int OldValue)
{
    EXT_BV                                  NewValue;

    memset(&NewValue, '\0', sizeof(EXT_BV));
    if (OldValue & IMM_FIRE)
	xSET_BIT(NewValue, smaug_get_rflag("fire"));
    if (OldValue & IMM_COLD)
	xSET_BIT(NewValue, smaug_get_rflag("cold"));
    if (OldValue & IMM_ELEC)
	xSET_BIT(NewValue, smaug_get_rflag("electricity"));
    if (OldValue & IMM_ENERGY)
	xSET_BIT(NewValue, smaug_get_rflag("energy"));
    if (OldValue & IMM_BLUNT)
	xSET_BIT(NewValue, smaug_get_rflag("blunt"));
    if (OldValue & IMM_PIERCE)
	xSET_BIT(NewValue, smaug_get_rflag("pierce"));
    if (OldValue & IMM_SLASH)
	xSET_BIT(NewValue, smaug_get_rflag("slash"));
    if (OldValue & IMM_ACID)
	xSET_BIT(NewValue, smaug_get_rflag("acid"));
    if (OldValue & IMM_POISON)
	xSET_BIT(NewValue, smaug_get_rflag("poison"));
    if (OldValue & IMM_DRAIN)
	xSET_BIT(NewValue, smaug_get_rflag("drain"));
    if (OldValue & IMM_SLEEP)
	xSET_BIT(NewValue, smaug_get_rflag("sleep"));
    if (OldValue & IMM_CHARM)
	xSET_BIT(NewValue, smaug_get_rflag("charm"));
    if (OldValue & IMM_HOLD)
	xSET_BIT(NewValue, smaug_get_rflag("hold"));
    if (OldValue & IMM_NONMAG)
	xSET_BIT(NewValue, smaug_get_rflag("nonmagic"));
    if (OldValue & IMM_PLUS1)
	xSET_BIT(NewValue, smaug_get_rflag("plus1"));
    if (OldValue & IMM_PLUS2)
	xSET_BIT(NewValue, smaug_get_rflag("plus2"));
    if (OldValue & IMM_PLUS3)
	xSET_BIT(NewValue, smaug_get_rflag("plus3"));
    if (OldValue & IMM_PLUS4)
	xSET_BIT(NewValue, smaug_get_rflag("plus4"));

    return NewValue;
}

int smaug_convertsex(int Sex)
{
    switch (Sex) {
	case SEX_NEUTER:
	    return smaug_get_sex("neuter");
	case SEX_MALE:
	    return smaug_get_sex("male");
	case SEX_FEMALE:
	    return smaug_get_sex("female");
	default:
	    return smaug_get_sex("neuter");
    }
}

int smaug_convertposition(int Position)
{
    switch (Position) {
	case POSITION_DEAD:
	    return smaug_get_position("dead");
	case POSITION_MORTALLYW:
	    return smaug_get_position("mortal");
	case POSITION_INCAP:
	    return smaug_get_position("incapacitated");
	case POSITION_STUNNED:
	    return smaug_get_position("stunned");
	case POSITION_SLEEPING:
	    return smaug_get_position("sleeping");
	case POSITION_RESTING:
	    return smaug_get_position("resting");
	case POSITION_SITTING:
	    return smaug_get_position("sitting");
	case POSITION_FIGHTING:
	    return smaug_get_position("fighting");
	case POSITION_STANDING:
	    return smaug_get_position("standing");
	case POSITION_MOUNTED:
	    return smaug_get_position("mounted");
	default:
	    return smaug_get_position("standing");
    }
}

int smaug_convertobjtype(int Type)
{
    switch (Type) {
	case ITEM_LIGHT:
	    return smaug_get_otype("light");
	case ITEM_SCROLL:
	    return smaug_get_otype("scroll");
	case ITEM_WAND:
	    return smaug_get_otype("wand");
	case ITEM_STAFF:
	    return smaug_get_otype("staff");
	case ITEM_WEAPON:
	    return smaug_get_otype("weapon");
	case ITEM_FIREWEAPON:
	    return smaug_get_otype("weapon");
	case ITEM_MISSILE:
	    return smaug_get_otype("projectile");
	case ITEM_TREASURE:
	    return smaug_get_otype("treasure");
	case ITEM_ARMOR:
	    return smaug_get_otype("armor");
	case ITEM_POTION:
	    return smaug_get_otype("potion");
	case ITEM_TRASH:
	    return smaug_get_otype("trash");
	case ITEM_TRAP:
	    return smaug_get_otype("trap");
	case ITEM_CONTAINER:
	    return smaug_get_otype("container");
	case ITEM_NOTE:
	    return smaug_get_otype("paper");
	case ITEM_DRINKCON:
	    return smaug_get_otype("drinkcon");
	case ITEM_KEY:
	    return smaug_get_otype("key");
	case ITEM_FOOD:
	    return smaug_get_otype("food");
	case ITEM_MONEY:
	    return smaug_get_otype("money");
	case ITEM_PEN:
	    return smaug_get_otype("pen");
	case ITEM_BOAT:
	    return smaug_get_otype("boat");

	case ITEM_WORN:
	case ITEM_OTHER:
	case ITEM_AUDIO:
	case ITEM_BOARD:
	case ITEM_KENNEL:
	default:
	    return smaug_get_otype("none");
    }
}

EXT_BV smaug_convert_rflags(int OldValue)
{
    EXT_BV                                  NewValue;

    memset(&NewValue, '\0', sizeof(EXT_BV));
    if (OldValue & ROOM_DARK)
	xSET_BIT(NewValue, smaug_get_rflag("dark"));
    if (OldValue & ROOM_DEATH)
	xSET_BIT(NewValue, smaug_get_rflag("death"));
    if (OldValue & ROOM_NOMOB)
	xSET_BIT(NewValue, smaug_get_rflag("nomob"));
    if (OldValue & ROOM_INDOORS)
	xSET_BIT(NewValue, smaug_get_rflag("indoors"));
    if (OldValue & ROOM_NOATTACK)
	xSET_BIT(NewValue, smaug_get_rflag("safe"));
    /*
     * if(OldValue & ROOM_NOSTEAL) NewValue |= 
     */
    if (OldValue & ROOM_NOSUMMON)
	xSET_BIT(NewValue, smaug_get_rflag("nosummon"));
    if (OldValue & ROOM_NOMAGIC)
	xSET_BIT(NewValue, smaug_get_rflag("nomagic"));
    if (OldValue & ROOM_PRIVATE)
	xSET_BIT(NewValue, smaug_get_rflag("private"));
    /*
     * if(OldValue & ROOM_SOUND) NewValue |= 
     */

    return NewValue;
}

EXT_BV smaug_convert_exflags(int OldType)
{
    EXT_BV                                  NewValue;

    memset(&NewValue, '\0', sizeof(EXT_BV));
    switch (OldType) {
	case EXIT_DOOR_ALIAS:
	case EXIT_DOOR:
	    xSET_BIT(NewValue, smaug_get_exflag("isdoor"));
	    xSET_BIT(NewValue, smaug_get_exflag("closed"));
	    break;
	case EXIT_NOPICK_ALIAS:
	case EXIT_NOPICK:
	    xSET_BIT(NewValue, smaug_get_exflag("isdoor"));
	    xSET_BIT(NewValue, smaug_get_exflag("pickproof"));
	    xSET_BIT(NewValue, smaug_get_exflag("closed"));
	    break;
	case EXIT_SECRET_ALIAS:
	case EXIT_SECRET:
	    xSET_BIT(NewValue, smaug_get_exflag("isdoor"));
	    xSET_BIT(NewValue, smaug_get_exflag("secret"));
	    xSET_BIT(NewValue, smaug_get_exflag("closed"));
	    break;
	case EXIT_SECRET_NOPICK_ALIAS:
	case EXIT_SECRET_NOPICK:
	    xSET_BIT(NewValue, smaug_get_exflag("isdoor"));
	    xSET_BIT(NewValue, smaug_get_exflag("secret"));
	    xSET_BIT(NewValue, smaug_get_exflag("pickproof"));
	    xSET_BIT(NewValue, smaug_get_exflag("closed"));
	    break;
	default:
	    break;
    }

    return NewValue;
}

EXT_BV smaug_convert_oflags(int OldValue)
{
    EXT_BV                                  NewValue;

    memset(&NewValue, '\0', sizeof(EXT_BV));
    if (OldValue & ITEM_GLOW)
	xSET_BIT(NewValue, smaug_get_oflag("glow"));
    if (OldValue & ITEM_HUM)
	xSET_BIT(NewValue, smaug_get_oflag("hum"));
    if (OldValue & ITEM_METAL)
	xSET_BIT(NewValue, smaug_get_oflag("metal"));
    if (OldValue & ITEM_MINERAL)
	xSET_BIT(NewValue, smaug_get_oflag("mineral"));
    if (OldValue & ITEM_ORGANIC)
	xSET_BIT(NewValue, smaug_get_oflag("organic"));
    if (OldValue & ITEM_INVISIBLE)
	xSET_BIT(NewValue, smaug_get_oflag("invis"));
    if (OldValue & ITEM_MAGIC)
	xSET_BIT(NewValue, smaug_get_oflag("magic"));
    if (OldValue & ITEM_NODROP)
	xSET_BIT(NewValue, smaug_get_oflag("nodrop"));
    if (OldValue & ITEM_BLESS)
	xSET_BIT(NewValue, smaug_get_oflag("bless"));
    if (OldValue & ITEM_ANTI_GOOD)
	xSET_BIT(NewValue, smaug_get_oflag("antigood"));
    if (OldValue & ITEM_ANTI_EVIL)
	xSET_BIT(NewValue, smaug_get_oflag("antievil"));
    if (OldValue & ITEM_ANTI_NEUTRAL)
	xSET_BIT(NewValue, smaug_get_oflag("antineutral"));
    if (OldValue & ITEM_ANTI_CLERIC)
	xSET_BIT(NewValue, smaug_get_oflag("anticleric"));
    if (OldValue & ITEM_ANTI_MAGE)
	xSET_BIT(NewValue, smaug_get_oflag("antimage"));
    if (OldValue & ITEM_ANTI_THIEF)
	xSET_BIT(NewValue, smaug_get_oflag("antirogue"));
    if (OldValue & ITEM_ANTI_FIGHTER)
	xSET_BIT(NewValue, smaug_get_oflag("antiwarrior"));
    if (OldValue & ITEM_ANTI_RANGER)
	xSET_BIT(NewValue, smaug_get_oflag("antiranger"));
    if (OldValue & ITEM_PARISH)				       /* No match in AFK */
	;

    return NewValue;
}

int smaug_convert_wflags(int OldValue)
{
    int                                     NewValue = 0;

    if (OldValue & ITEM_TAKE)
	SET_BIT(NewValue, smaug_get_wflag("take"));
    if (OldValue & ITEM_WEAR_FINGER)
	SET_BIT(NewValue, smaug_get_wflag("finger"));
    if (OldValue & ITEM_WEAR_NECK)
	SET_BIT(NewValue, smaug_get_wflag("neck"));
    if (OldValue & ITEM_WEAR_BODY)
	SET_BIT(NewValue, smaug_get_wflag("body"));
    if (OldValue & ITEM_WEAR_HEAD)
	SET_BIT(NewValue, smaug_get_wflag("head"));
    if (OldValue & ITEM_WEAR_LEGS)
	SET_BIT(NewValue, smaug_get_wflag("legs"));
    if (OldValue & ITEM_WEAR_FEET)
	SET_BIT(NewValue, smaug_get_wflag("feet"));
    if (OldValue & ITEM_WEAR_HANDS)
	SET_BIT(NewValue, smaug_get_wflag("hands"));
    if (OldValue & ITEM_WEAR_ARMS)
	SET_BIT(NewValue, smaug_get_wflag("arms"));
    if (OldValue & ITEM_WEAR_SHIELD)
	SET_BIT(NewValue, smaug_get_wflag("shield"));
    if (OldValue & ITEM_WEAR_ABOUT)
	SET_BIT(NewValue, smaug_get_wflag("about"));
    if (OldValue & ITEM_WEAR_WAISTE)
	SET_BIT(NewValue, smaug_get_wflag("waist"));
    if (OldValue & ITEM_WEAR_WRIST)
	SET_BIT(NewValue, smaug_get_wflag("wrist"));
    if (OldValue & ITEM_WIELD)
	SET_BIT(NewValue, smaug_get_wflag("wield"));
    if (OldValue & ITEM_HOLD)
	SET_BIT(NewValue, smaug_get_wflag("hold"));
    if (OldValue & ITEM_WIELD_TWOH)
	SET_BIT(NewValue, smaug_get_wflag("dual"));

    return NewValue;
}

EXT_BV smaug_convert_actflags(int OldValue)
{
    EXT_BV                                  NewValue;

    memset(&NewValue, '\0', sizeof(EXT_BV));

    if (OldValue & ACT_SPEC)
        ; // NOOP 
    if (OldValue & ACT_SENTINEL)
	xSET_BIT(NewValue, smaug_get_actflag("sentinel"));
    if (OldValue & ACT_SCAVENGER)
	xSET_BIT(NewValue, smaug_get_actflag("scavenger"));
    if (OldValue & ACT_ISNPC)
        ; // NOOP
    if (OldValue & ACT_NICE_THIEF)
        ; // NOOP
    if (OldValue & ACT_AGGRESSIVE)
	xSET_BIT(NewValue, smaug_get_actflag("aggressive"));
    if (OldValue & ACT_STAY_ZONE)
	xSET_BIT(NewValue, smaug_get_actflag("stayarea"));
    if (OldValue & ACT_WIMPY)
	xSET_BIT(NewValue, smaug_get_actflag("wimpy"));
    if (OldValue & ACT_ANNOYING)
	xSET_BIT(NewValue, smaug_get_actflag("annoying"));
    if (OldValue & ACT_HATEFUL)
        ; // NOOP
    if (OldValue & ACT_AFRAID)
        ; // NOOP
    if (OldValue & ACT_IMMORTAL)
	xSET_BIT(NewValue, smaug_get_actflag("immortal"));
    if (OldValue & ACT_HUNTING)
        ; // NOOP
    if (OldValue & ACT_DEADLY)
	xSET_BIT(NewValue, smaug_get_actflag("deadly"));
    if (OldValue & ACT_GUARDIAN)
	xSET_BIT(NewValue, smaug_get_actflag("guardian"));
    if (OldValue & ACT_USE_ITEM)
        ; // NOOP
    if (OldValue & ACT_FIGHTER_MOVES)
	xSET_BIT(NewValue, smaug_get_actflag("meta_aggr"));    // Not quite
    if (OldValue & ACT_FOOD_PROVIDE)
        ; // NOOP
    if (OldValue & ACT_PROTECTOR)
        ; // NOOP
    if (OldValue & ACT_MOUNT)
	xSET_BIT(NewValue, smaug_get_actflag("mountable"));
    if (OldValue & ACT_SWITCH)
        ; // NOOP

    return NewValue;
}

EXT_BV smaug_convert_aflags(int OldValue)
{
    EXT_BV                                  NewValue;

    memset(&NewValue, '\0', sizeof(EXT_BV));

    if (OldValue & AFF_BLIND)
	xSET_BIT(NewValue, smaug_get_aflag("blind"));
    if (OldValue & AFF_INVISIBLE)
	xSET_BIT(NewValue, smaug_get_aflag("invisible"));
    if (OldValue & AFF_DETECT_EVIL)
	xSET_BIT(NewValue, smaug_get_aflag("detect_evil"));
    if (OldValue & AFF_DETECT_INVISIBLE)
	xSET_BIT(NewValue, smaug_get_aflag("detect_invis"));
    if (OldValue & AFF_DETECT_MAGIC)
	xSET_BIT(NewValue, smaug_get_aflag("detect_magic"));
    if (OldValue & AFF_SENSE_LIFE)
	xSET_BIT(NewValue, smaug_get_aflag("detect_hidden"));  // Not quite

    if (OldValue & AFF_SILENCED)
	xSET_BIT(NewValue, smaug_get_aflag("silence"));
    if (OldValue & AFF_SANCTUARY)
	xSET_BIT(NewValue, smaug_get_aflag("sanctuary"));
    if (OldValue & AFF_GROUP)
        ; // NOOP
    if (OldValue & AFF_CURSE)
	xSET_BIT(NewValue, smaug_get_aflag("curse"));
    if (OldValue & AFF_FLYING)
	xSET_BIT(NewValue, smaug_get_aflag("flying"));
    if (OldValue & AFF_POISON)
	xSET_BIT(NewValue, smaug_get_aflag("poison"));
    if (OldValue & AFF_PROTECT_EVIL)
	xSET_BIT(NewValue, smaug_get_aflag("protect"));
    if (OldValue & AFF_PARALYSIS)
	xSET_BIT(NewValue, smaug_get_aflag("paralysis"));
    if (OldValue & AFF_INFRAVISION)
        ; // NOOP
    if (OldValue & AFF_WATERBREATH)
	xSET_BIT(NewValue, smaug_get_aflag("aqua_breath"));
    if (OldValue & AFF_SLEEP)
	xSET_BIT(NewValue, smaug_get_aflag("sleep"));
    if (OldValue & AFF_DRUG_FREE)
        ; // NOOP
    if (OldValue & AFF_SNEAK)
	xSET_BIT(NewValue, smaug_get_aflag("sneak"));
    if (OldValue & AFF_HIDE)
	xSET_BIT(NewValue, smaug_get_aflag("hide"));
    if (OldValue & AFF_FEAR)
        ; // NOOP
    if (OldValue & AFF_CHARM)
        ; // NOOP
    if (OldValue & AFF_FOLLOW)
        ; // NOOP
    if (OldValue & AFF_UNDEF_1)
        ; // NOOP
    if (OldValue & AFF_TRUE_SIGHT)
	xSET_BIT(NewValue, smaug_get_aflag("truesight"));
    if (OldValue & AFF_SCRYING)
	xSET_BIT(NewValue, smaug_get_aflag("scrying"));
    if (OldValue & AFF_FIRESHIELD)
	xSET_BIT(NewValue, smaug_get_aflag("fireshield"));     // Not quite
    if (OldValue & AFF_RIDE)
        ; // NOOP
    if (OldValue & AFF_UNDEF_6)
        ; // NOOP
    if (OldValue & AFF_UNDEF_7)
        ; // NOOP
    if (OldValue & AFF_UNDEF_8)
        ; // NOOP

    return NewValue;
}

/*
 * Compare strings, case insensitive.
 * Return TRUE if different
 *   (compatibility with historical functions).
 */
bool smaug_str_cmp(const char *astr, const char *bstr)
{
    if (!astr) {
	if (bstr)
	    fprintf(stderr, "smaug_str_cmp: astr: (null)  bstr: %s\n", bstr);
	return TRUE;
    }

    if (!bstr) {
	if (astr)
	    fprintf(stderr, "smaug_str_cmp: astr: %s  bstr: (null)\n", astr);
	return TRUE;
    }

    for (; *astr || *bstr; astr++, bstr++) {
	if (LOWER(*astr) != LOWER(*bstr))
	    return TRUE;
    }

    return FALSE;
}

int smaug_get_rflag(char *flag)
{
    int                                     x = 0;

    for (x = 0; x < (sizeof(smaug_r_flags) / sizeof(smaug_r_flags[0])); x++)
	if (!smaug_str_cmp(flag, smaug_r_flags[x]))
	    return x;
    return -1;
}

int smaug_get_exflag(char *flag)
{
    int                                     x = 0;

    for (x = 0; x < (sizeof(smaug_ex_flags) / sizeof(smaug_ex_flags[0])); x++)
	if (!smaug_str_cmp(flag, smaug_ex_flags[x]))
	    return x;
    return -1;
}

int smaug_get_otype(const char *type)
{
    unsigned int                            x;

    for (x = 0; x < (sizeof(smaug_o_types) / sizeof(smaug_o_types[0])); x++)
	if (!smaug_str_cmp(type, smaug_o_types[x]))
	    return x;
    return -1;
}

int smaug_get_oflag(char *flag)
{
    unsigned int                            x;

    for (x = 0; x < (sizeof(smaug_o_flags) / sizeof(smaug_o_flags[0])); x++)
	if (!smaug_str_cmp(flag, smaug_o_flags[x]))
	    return x;
    return -1;
}

int smaug_get_wflag(char *flag)
{
    unsigned int                            x;

    for (x = 0; x < (sizeof(smaug_w_flags) / sizeof(smaug_w_flags[0])); x++)
	if (!smaug_str_cmp(flag, smaug_w_flags[x]))
	    return x;
    return -1;
}

int smaug_get_magflag(char *flag)
{
    unsigned int                            x;

    for (x = 0; x < (sizeof(smaug_mag_flags) / sizeof(smaug_mag_flags[0])); x++)
	if (!smaug_str_cmp(flag, smaug_mag_flags[x]))
	    return x;
    return -1;
}

int smaug_get_actflag(char *flag)
{
    unsigned int                            x;

    for (x = 0; x < (sizeof(smaug_act_flags) / sizeof(smaug_act_flags[0])); x++)
	if (!smaug_str_cmp(flag, smaug_act_flags[x]))
	    return x;
    return -1;
}

int smaug_get_aflag(char *flag)
{
    unsigned int                            x;

    for (x = 0; x < (sizeof(smaug_a_flags) / sizeof(smaug_a_flags[0])); x++)
	if (!smaug_str_cmp(flag, smaug_a_flags[x]))
	    return x;
    return -1;
}

int smaug_get_position(const char *type)
{
    unsigned int                            x;

    for (x = 0; x < (sizeof(smaug_npc_position) / sizeof(smaug_npc_position[0])); x++)
	if (!smaug_str_cmp(type, smaug_npc_position[x]))
	    return x;
    return -1;
}

int smaug_get_sex(const char *type)
{
    unsigned int                            x;

    for (x = 0; x < (sizeof(smaug_sex) / sizeof(smaug_sex[0])); x++)
	if (!smaug_str_cmp(type, smaug_sex[x]))
	    return x;
    return -1;
}

int smaug_get_npc_race(char *type)
{
    int                                     x;

    for (x = 0; x < (sizeof(smaug_npc_race) / sizeof(smaug_npc_race[0])); x++)
	if (!smaug_str_cmp(type, smaug_npc_race[x]))
	    return x;
    return -1;
}

int smaug_get_npc_class(char *type)
{
    int                                     x;

    for (x = 0; x < (sizeof(smaug_npc_class) / sizeof(smaug_npc_class[0])); x++)
	if (!smaug_str_cmp(type, smaug_npc_class[x]))
	    return x;
    return -1;
}

int smaug_get_partflag(char *flag)
{
    unsigned int                            x;

    for (x = 0; x < (sizeof(smaug_part_flags) / sizeof(smaug_part_flags[0])); x++)
	if (!smaug_str_cmp(flag, smaug_part_flags[x]))
	    return x;
    return -1;
}

int smaug_get_risflag(char *flag)
{
    unsigned int                            x;

    for (x = 0; x < (sizeof(smaug_ris_flags) / sizeof(smaug_ris_flags[0])); x++)
	if (!smaug_str_cmp(flag, smaug_ris_flags[x]))
	    return x;
    return -1;
}

int smaug_get_attackflag(char *flag)
{
    unsigned int                            x;

    for (x = 0; x < (sizeof(smaug_attack_flags) / sizeof(smaug_attack_flags[0])); x++)
	if (!smaug_str_cmp(flag, smaug_attack_flags[x]))
	    return x;
    return -1;
}

char                                   *smaug_flag_string(int bitvector,
							  char *const flagarray[])
{
    static char                             buf[MSL];
    int                                     x = 0;

    memset(buf, '\0', MSL);
    for (x = 0; x < 32; x++)
	if (IS_SET(bitvector, 1 << x)) {
	    strcat(buf, flagarray[x]);
	    strcat(buf, " ");
	}
    if ((x = strlen(buf)) > 0)
	buf[--x] = '\0';

    return buf;
}

char                                   *smaug_ext_flag_string(EXT_BV *bitvector,
							      char *const flagarray[])
{
    static char                             buf[MSL];
    int                                     x = 0;

    memset(buf, '\0', MSL);
    for (x = 0; x < MAX_BITS; x++)
	if (xIS_SET(*bitvector, x)) {
	    strcat(buf, flagarray[x]);
	    strcat(buf, " ");
	}
    if ((x = strlen(buf)) > 0)
	buf[--x] = '\0';

    return buf;
}

/*
 * Remove carriage returns from a line
 */
char                                   *smaug_strip_cr(char *str)
{
    static char                             newstr[MSL];
    int                                     i = 0;
    int                                     j = 0;

    memset(newstr, '\0', MSL);

    for (i = j = 0; str[i] != '\0'; i++)
	if (str[i] != '\r') {
	    newstr[j++] = str[i];
	}
    newstr[j] = '\0';
    return newstr;
}

int smaug_convert_spell(int SpellNum)
{
    switch (SpellNum) {
	case SPELL_RESERVED_DBC:
	    return -1;
	case SPELL_ARMOR:
	    return -1;
	default:
	    return -1;
    }
}

int smaug_convert_skill(int SkillNum)
{
    return SkillNum;
}

int smaug_convert_liquid(int Liquid)
{
    /*
     * No conversion needed 
     */
    return Liquid;
}

int                                    *smaug_convert_item_values(int Type, int w0, int w1,
								  int w2, int w3)
{
    static int                              a[11];

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

    switch (Type) {
	case ITEM_LIGHT:
	    break;
	case ITEM_SCROLL:
	    a[1] = smaug_convert_spell(w1);
	    a[2] = smaug_convert_spell(w2);
	    a[3] = smaug_convert_spell(w3);
	    break;
	case ITEM_WAND:
	    a[3] = smaug_convert_spell(w3);
	    break;
	case ITEM_STAFF:
	    a[3] = smaug_convert_spell(w3);
	    break;
	case ITEM_WEAPON:
	case ITEM_FIREWEAPON:
	    switch (w3) {
		case 0:				       /* Bludgeon (smite) */
		    a[3] = 4;				       /* crush damage */
		    a[4] = 5;				       /* mace skill */
		    break;
		case 1:				       /* Pierce (stab) */
		    a[3] = 2;				       /* stab damage */
		    a[4] = 2;				       /* dagger skill */
		    break;
		case 2:				       /* Slash (whip) */
		    a[3] = 5;				       /* lash damage */
		    a[4] = 3;				       /* whip skill */
		    break;
		case 3:				       /* Slash (slash) */
		    a[3] = 1;				       /* slash damage */
		    a[4] = 1;				       /* sword skill */
		    break;
		case 4:				       /* Bludgeon (smash) */
		    a[3] = 4;				       /* crush damage */
		    a[4] = 5;				       /* mace skill */
		    break;
		case 5:				       /* Slash (cleave) */
		    a[3] = 3;				       /* hack damage */
		    a[4] = 9;				       /* axe skill */
		    break;
		case 6:				       /* Bludgeon (crush) */
		    a[3] = 4;				       /* crush damage */
		    a[4] = 5;				       /* mace skill */
		    break;
		case 7:				       /* Bludgeon (pound) */
		    a[3] = 4;				       /* crush damage */
		    a[4] = 5;				       /* mace skill */
		    break;
		case 8:				       /* Mob (claw) */
		    a[3] = 1;				       /* slash damage */
		    a[4] = 0;				       /* barehand skill */
		    break;
		case 9:				       /* Mob (bite) */
		    a[3] = 4;				       /* crush damage */
		    a[4] = 0;				       /* barehand skill */
		    break;
		case 10:				       /* Mob (sting) */
		    a[3] = 6;				       /* pierce damage */
		    a[4] = 0;				       /* barehand skill */
		    break;
		case 11:				       /* Pierce (pierce) */
		    a[3] = 6;				       /* pierce damage */
		    a[4] = 2;				       /* dagger skill */
		    break;
		default:
		    ;
	    }
	    break;
	case ITEM_MISSILE:
	    switch (w3) {
		case 0:				       /* Bludgeon (smite) */
		    a[3] = 4;				       /* crush damage */
		    a[4] = 3;				       /* stone type */
		    break;
		case 1:				       /* Pierce (stab) */
		    a[3] = 2;				       /* stab damage */
		    a[4] = 0;				       /* bolt type */
		    break;
		case 2:				       /* Slash (whip) */
		    a[3] = 5;				       /* lash damage */
		    a[4] = 1;				       /* arrow type */
		    break;
		case 3:				       /* Slash (slash) */
		    a[3] = 1;				       /* slash damage */
		    a[4] = 1;				       /* arrow type */
		    break;
		case 4:				       /* Bludgeon (smash) */
		    a[3] = 4;				       /* crush damage */
		    a[4] = 3;				       /* stone type */
		    break;
		case 5:				       /* Slash (cleave) */
		    a[3] = 3;				       /* hack damage */
		    a[4] = 1;				       /* arrow type */
		    break;
		case 6:				       /* Bludgeon (crush) */
		    a[3] = 4;				       /* crush damage */
		    a[4] = 3;				       /* stone type */
		    break;
		case 7:				       /* Bludgeon (pound) */
		    a[3] = 4;				       /* crush damage */
		    a[4] = 3;				       /* stone type */
		    break;
		case 8:				       /* Mob (claw) */
		    a[3] = 1;				       /* slash damage */
		    a[4] = 1;				       /* arrow type */
		    break;
		case 9:				       /* Mob (bite) */
		    a[3] = 4;				       /* crush damage */
		    a[4] = 3;				       /* stone type */
		    break;
		case 10:				       /* Mob (sting) */
		    a[3] = 6;				       /* pierce damage */
		    a[4] = 2;				       /* dart type */
		    break;
		case 11:				       /* Pierce (pierce) */
		    a[3] = 6;				       /* pierce damage */
		    a[4] = 2;				       /* dart type */
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
	    a[1] = smaug_convert_spell(w1);
	    a[2] = smaug_convert_spell(w2);
	    a[3] = smaug_convert_spell(w3);
	    break;
	case ITEM_WORN:
	    break;
	case ITEM_OTHER:
	    break;
	case ITEM_TRASH:
	    break;
	case ITEM_TRAP:				       /* NOT supported in AFK? */
	    break;
	case ITEM_CONTAINER:
	    a[3] = 0;
	    break;
	case ITEM_NOTE:
	    break;
	case ITEM_DRINKCON:
	    a[2] = smaug_convert_liquid(w2);
	    break;
	case ITEM_KEY:
	    /*
	     * May need a[5], unsure yet 
	     */
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
	    /*
	     * No conversion done 
	     */
	    ;
    }

    return a;
}

void renumber_for_smaug(zones *Zones, rooms *Rooms, shops *Shops, objects *Objects, mobs *Mobs)
{
    int                                     i,
                                            j;

    for (i = 0; i < Zones->Count; i++) {
	Zones->Zone[i].Number += SMAUG_ZONE_OFFSET;
	Zones->Zone[i].Top += SMAUG_VNUM_OFFSET;

	for (j = 0; j < Zones->Zone[i].Count; j++) {
	    switch (Zones->Zone[i].Cmds[j].Command) {
		case ZONE_CMD_MOBILE:
		    Zones->Zone[i].Cmds[j].Arg[ZONE_MOBILE] += SMAUG_VNUM_OFFSET;
		    Zones->Zone[i].Cmds[j].Arg[ZONE_ROOM] += SMAUG_VNUM_OFFSET;
		    break;
		case ZONE_CMD_OBJECT:
		    Zones->Zone[i].Cmds[j].Arg[ZONE_OBJECT] += SMAUG_VNUM_OFFSET;
		    Zones->Zone[i].Cmds[j].Arg[ZONE_ROOM] += SMAUG_VNUM_OFFSET;
		    break;
		case ZONE_CMD_GIVE:
		    Zones->Zone[i].Cmds[j].Arg[ZONE_OBJECT] += SMAUG_VNUM_OFFSET;
		    break;
		case ZONE_CMD_PUT:
		    Zones->Zone[i].Cmds[j].Arg[ZONE_OBJECT] += SMAUG_VNUM_OFFSET;
		    Zones->Zone[i].Cmds[j].Arg[ZONE_TARGET_OBJ] += SMAUG_VNUM_OFFSET;
		    break;
		case ZONE_CMD_DOOR:
		    Zones->Zone[i].Cmds[j].Arg[ZONE_ROOM] += SMAUG_VNUM_OFFSET;
		    break;
		case ZONE_CMD_REMOVE:
		    Zones->Zone[i].Cmds[j].Arg[ZONE_REMOVE_OBJ] += SMAUG_VNUM_OFFSET;
		    Zones->Zone[i].Cmds[j].Arg[ZONE_ROOM] += SMAUG_VNUM_OFFSET;
		    break;
		case ZONE_CMD_LEAD:
		    Zones->Zone[i].Cmds[j].Arg[ZONE_MOBILE] += SMAUG_VNUM_OFFSET;
		    break;
		case ZONE_CMD_HATE:
		    if (Zones->Zone[i].Cmds[j].Arg[ZONE_HATE_TYPE] == HATE_VNUM)
			Zones->Zone[i].Cmds[j].Arg[ZONE_HATE_VALUE] += SMAUG_VNUM_OFFSET;
		    break;
	    }
	}
    }

    for (i = 0; i < Rooms->Count; i++) {
	Rooms->Room[i].                         Number += SMAUG_VNUM_OFFSET;
	Rooms->Room[i].                         TeleportTo += SMAUG_VNUM_OFFSET;

	for (j = 0; j < Rooms->Room[i].ExitCount; j++) {
	    switch (Rooms->Room[i].Exit[j].Error) {
		case EXIT_DESCRIPTION_ONLY:
		    break;
		default:
		    Rooms->Room[i].                         Exit[j].Room +=SMAUG_VNUM_OFFSET;

		    break;
	    }
	}
    }

    for (i = 0; i < Shops->Count; i++) {
	Shops->Shop[i].Number += SMAUG_VNUM_OFFSET;
	Shops->Shop[i].Room                   +=SMAUG_VNUM_OFFSET;

	Shops->Shop[i].Keeper += SMAUG_VNUM_OFFSET;
	for (j = 0; j < Shops->Shop[i].SellCount; j++) {
	    if (Shops->Shop[i].Sell[j] >= 0) {
		Shops->Shop[i].Sell[j] += SMAUG_VNUM_OFFSET;
	    }
	}
    }

    for (i = 0; i < Objects->Count; i++) {
	Objects->Object[i].Number += SMAUG_VNUM_OFFSET;
    }

    for (i = 0; i < Mobs->Count; i++) {
	Mobs->Mob[i].Number += SMAUG_VNUM_OFFSET;
    }

}

void dump_as_smaug(zones *Zones, rooms *Rooms, shops *Shops, objects *Objects, mobs *Mobs)
{
    FILE                                   *ofp,
                                           *listfp;
    char                                    filename[MAX_STRING_LEN],
                                            tmpstr[MAX_STRING_LEN];
    int                                     i,
                                            j,
                                            k,
                                            x;
    int                                     m;

    /*
     * int LastMob, LastLoc; 
     */
    int                                     LowRoom,
                                            HighRoom;
    pair                                   *ZoneSort;

    renumber_for_smaug(Zones, Rooms, Shops, Objects, Mobs);

    snprintf(filename, MAX_STRING_LEN, "%s/%s/area.lst", OutputDir, SMAUG_SUBDIR);
    listfp = open_file(filename, "w");

    ZoneSort = (pair *)get_mem(Zones->Count, sizeof(pair));
    bzero(ZoneSort, sizeof(pair) * Zones->Count);
    for (i = 0; i < Zones->Count; i++) {
	snprintf(tmpstr, MAX_STRING_LEN, "%s_%d.are", remap_name(Zones->Zone[i].Name), Zones->Zone[i].Number);
	fprintf(listfp, "%s\n", tmpstr);
	snprintf(filename, MAX_STRING_LEN, "%s/%s/%s", OutputDir, SMAUG_SUBDIR, tmpstr);
	ofp = open_file(filename, "w");
	if (Verbose)
	    fprintf(stderr, "Dump of Zone \"%s\"[#%d]...\n", remap_name(Zones->Zone[i].Name),
		    Zones->Zone[i].Number);
	else if (!Quiet) {
	    snprintf(tmpstr, MAX_STRING_LEN, "#%d Dump of Zone \"%s\"[#%d]...", i + 1,
		    remap_name(Zones->Zone[i].Name), Zones->Zone[i].Number);
	    fprintf(stderr, "%s", tmpstr);
	    for (x = strlen(tmpstr); x < 79; x++)
		fprintf(stderr, " ");
	    for (x = strlen(tmpstr); x < 79; x++)
		fprintf(stderr, "\b");
	    fflush(stderr);
	}
	LowRoom = INT_MAX;
	HighRoom = INT_MIN;
	for (j = 0; j < Rooms->Count; j++) {
	    if ((remap_zone_vnum(Zones, Rooms->Room[j].Zone) == i) ||
		((Rooms->Room[j].Number >= (!i ? 0 : Zones->Zone[i - 1].Top + 1))&&
		 (Rooms->Room[j].Number <= Zones->Zone[i].Top)
		)) {
		LowRoom = min(LowRoom, Rooms->Room[j].Number);
		HighRoom = max(HighRoom, Rooms->Room[j].Number);
	    }
	}
	ZoneSort[i].x = i;
	ZoneSort[i].y = LowRoom;

/* ZONE */

	fprintf(ofp, "#FUSSAREA\n");
	fprintf(ofp, "#AREADATA\n");
	fprintf(ofp, "Version   %d\n", SMAUG_AREA_VERSION_WRITE);
	fprintf(ofp, "Name      %s~\n", Zones->Zone[i].Name);
	fprintf(ofp, "Author    %s~\n", "The Wiley Gang");
	fprintf(ofp, "WeatherX  %d\n", 0);
	fprintf(ofp, "WeatherY  %d\n", 0);
	fprintf(ofp, "Ranges    %d %d %d %d\n", SMAUG_AREA_LEVEL_LIMIT_LOWER, SMAUG_AREA_LEVEL_LIMIT_UPPER,	/* SOFT 
														 * (displayed) 
														 * limits 
														 */
		SMAUG_AREA_LEVEL_LIMIT_LOWER, SMAUG_AREA_LEVEL_LIMIT_UPPER);	/* HARD (driver enforced) limits */
	fprintf(ofp, "Economy   %d %d\n", 0, 0);
	/*
	 * fprintf( ofp, "ResetMsg %s~\n", "The Wiley Gang"); 
	 */
	if (Zones->Zone[i].Time)
	    fprintf(ofp, "ResetFreq %d\n", Zones->Zone[i].Time);
	/*
	 * fprintf( ofp, "Flags %s~\n", "The Wiley Gang"); 
	 */
	fprintf(ofp, "#ENDAREADATA\n\n");

/* MOBS */

	for (j = 0; j < Mobs->Count; j++) {
	    if ((remap_zone_vnum(Zones, Mobs->Mob[j].Zone) == i) ||
		((Mobs->Mob[j].Number >= (!i ? 0 : Zones->Zone[i - 1].Top + 1)) &&
		 (Mobs->Mob[j].Number <= Zones->Zone[i].Top)
		)) {
		EXT_BV                                  MobValue;
		EXT_BV                                  AttackValues;
		EXT_BV                                  ImmValue;

		// int WearValue = 0;
		// int *val = NULL;
		// int kval[11];
		dice                                    avg;

		memset(&MobValue, '\0', sizeof(EXT_BV));
		memset(&AttackValues, '\0', sizeof(EXT_BV));
		memset(&ImmValue, '\0', sizeof(EXT_BV));

		if (Verbose)
		    fprintf(stderr, "  (%d) Writing \"%s\"[#%d]\n", j,
			    Mobs->Mob[j].Name, Mobs->Mob[j].Number);
		else if (!Quiet)
		    spin(stderr);

		fprintf(ofp, "%s", "#MOBILE\n");
		fprintf(ofp, "Vnum      %d\n", Mobs->Mob[j].Number);
		fprintf(ofp, "Keywords  %s~\n", smaug_strip_cr(Mobs->Mob[j].Name));	/* called keywords now */
		fprintf(ofp, "Short     %s~\n", smaug_strip_cr(Mobs->Mob[j].ShortDesc));
		fprintf(ofp, "Long      %s~\n", smaug_strip_cr(Mobs->Mob[j].Description));	/* LongDesc */
		fprintf(ofp, "Desc      %s~\n", smaug_strip_cr(Mobs->Mob[j].LongDesc));	/* ActionDesc elsewhere */

		MobValue = smaug_convert_actflags(Mobs->Mob[j].ActFlags);
		fprintf(ofp, "Actflags  %s~\n",
			smaug_strip_cr(smaug_ext_flag_string(&MobValue, smaug_act_flags)));
		MobValue = smaug_convert_aflags(Mobs->Mob[j].AffectedBy);
		fprintf(ofp, "Affected  %s~\n",
			smaug_strip_cr(smaug_ext_flag_string(&MobValue, smaug_a_flags)));

		fprintf(ofp, "Stats1    %d %d %d %d %d %d\n", Mobs->Mob[j].Alignment,
			Mobs->Mob[j].Level, Mobs->Mob[j].ToHit,
			Mobs->Mob[j].ArmourClass,
			(Mobs->Mob[j].Gold.Rolls * Mobs->Mob[j].Gold.Die) +
			Mobs->Mob[j].Gold.Modifier,
			(Mobs->Mob[j].Experience.Rolls * Mobs->Mob[j].Experience.Die) +
			Mobs->Mob[j].Experience.Modifier);

		fprintf(ofp, "Stats2    %d %d %d\n",
			Mobs->Mob[j].HitPoints.Rolls, Mobs->Mob[j].HitPoints.Die,
			Mobs->Mob[j].HitPoints.Modifier);

		/*
		 * WileyMUD had multiple attacks, each defined by dice.  We'll have to average 
		 */
		if (Mobs->Mob[j].AttackCount > 0) {
		    avg.Rolls = avg.Die = avg.Modifier = 0;
		    for (k = 0; k < Mobs->Mob[j].AttackCount; k++) {
			avg.Rolls += Mobs->Mob[j].Attack[k].Rolls;
			avg.Die += Mobs->Mob[j].Attack[k].Die;
			avg.Modifier += Mobs->Mob[j].Attack[k].Modifier;
			smaug_convert_attack(&AttackValues, Mobs->Mob[j].Attack[k].Type);
		    }
		    avg.Rolls /= Mobs->Mob[j].AttackCount;
		    avg.Die /= Mobs->Mob[j].AttackCount;
		    avg.Modifier /= Mobs->Mob[j].AttackCount;
		} else {
		    avg.Rolls = Mobs->Mob[j].Level;
		    avg.Die = 8;
		    avg.Modifier = 0;
		}
		fprintf(ofp, "Stats3    %d %d %d\n", avg.Rolls, avg.Die, avg.Modifier);

		fprintf(ofp, "Stats4    %d %d %d %d %d\n", Mobs->Mob[j].Height,
			Mobs->Mob[j].Weight, Mobs->Mob[j].AttackCount, Mobs->Mob[j].ToHit,
			avg.Rolls * avg.Die / 2 + avg.Modifier);

		fprintf(ofp, "Attribs   %d %d %d %d %d %d %d\n",
			Mobs->Mob[j].Strength.Rolls * Mobs->Mob[j].Strength.Die / 2 +
			Mobs->Mob[j].Strength.Modifier,
			Mobs->Mob[j].Intelligence.Rolls * Mobs->Mob[j].Intelligence.Die / 2 +
			Mobs->Mob[j].Intelligence.Modifier,
			Mobs->Mob[j].Wisdom.Rolls * Mobs->Mob[j].Wisdom.Die / 2 +
			Mobs->Mob[j].Wisdom.Modifier,
			Mobs->Mob[j].Dexterity.Rolls * Mobs->Mob[j].Dexterity.Die / 2 +
			Mobs->Mob[j].Dexterity.Modifier,
			Mobs->Mob[j].Constitution.Rolls * Mobs->Mob[j].Constitution.Die / 2 +
			Mobs->Mob[j].Constitution.Modifier,
			/*
			 * No Charisma 
			 */ 14,
			/*
			 * No Luck 
			 */ 14);

		fprintf(ofp, "Saves     %d %d %d %d %d\n", Mobs->Mob[j].SavingThrow[0],
			Mobs->Mob[j].SavingThrow[1], Mobs->Mob[j].SavingThrow[2],
			Mobs->Mob[j].SavingThrow[3], Mobs->Mob[j].SavingThrow[4]);

		/*
		 * Wiley didn't have languages -- use common 
		 */
		/*
		 * might convert to race later... 
		 */
		fprintf(ofp, "Speaks    %s~\n", smaug_flag_string(0, smaug_lang_names));
		fprintf(ofp, "Speaking  %s~\n", smaug_flag_string(0, smaug_lang_names));

		ImmValue = smaug_convert_imm(Mobs->Mob[j].Resistance);
		fprintf(ofp, "Resist    %s~\n", smaug_ext_flag_string(&ImmValue, smaug_ris_flags));	// resist
		ImmValue = smaug_convert_imm(Mobs->Mob[j].Immunity);
		fprintf(ofp, "Immune    %s~\n", smaug_ext_flag_string(&ImmValue, smaug_ris_flags));	// immune
		ImmValue = smaug_convert_imm(Mobs->Mob[j].Susceptible);
		fprintf(ofp, "Suscept   %s~\n", smaug_ext_flag_string(&ImmValue, smaug_ris_flags));	// susc
		// ImmValue = smaug_convert_imm(0);
		// fprintf(ofp, "%s~\n", smaug_ext_flag_string( &ImmValue, smaug_ris_flags) ); // absorb -- Wiley has
		// none

		fprintf(ofp, "Position  %s~\n",
			smaug_npc_position[smaug_convertposition(Mobs->Mob[j].Position)]);
		fprintf(ofp, "DefPos    %s~\n",
			smaug_npc_position[smaug_convertposition
					   (Mobs->Mob[j].DefaultPosition)]);
		fprintf(ofp, "Gender    %s~\n", smaug_sex[smaug_convertsex(Mobs->Mob[j].Sex)]);
		fprintf(ofp, "Race      %s~\n",
			smaug_npc_race[smaug_get_npc_race(race_name(Mobs->Mob[j].Race))]);
		fprintf(ofp, "Class     %s~\n",
			smaug_npc_class[smaug_get_npc_class(class_name(Mobs->Mob[j].Class))]);
		/*
		 * Wiley doesn't have limbs 
		 */
		/*
		 * fprintf(ofp, "%s~\n", smaug_flag_string( pMobIndex->xflags, part_flags) ); 
		 */
		fprintf(ofp, "Bodyparts %s~\n", smaug_flag_string(0, smaug_part_flags));

		// Attacks and Defenses didn't exist in Wiley, maybe select some by old attack types later.
		ImmValue = smaug_convert_imm(0);
		fprintf(ofp, "Attacks   %s~\n",
			smaug_ext_flag_string(&ImmValue, smaug_attack_flags));
		ImmValue = smaug_convert_imm(0);
		fprintf(ofp, "Defenses  %s~\n",
			smaug_ext_flag_string(&ImmValue, smaug_defense_flags));

		/*
		 * Wiley has no mudprogs! 
		 */
		fprintf(ofp, "#ENDMOBILE\n\n");
	    }
	}

/* OBJECTS */

	for (j = 0; j < Objects->Count; j++) {
	    if ((remap_zone_vnum(Zones, Objects->Object[j].Zone) == i) ||
		((Objects->Object[j].Number >= (!i ? 0 : Zones->Zone[i - 1].Top + 1)) &&
		 (Objects->Object[j].Number <= Zones->Zone[i].Top)
		)) {
		EXT_BV                                  ObjValue;
		int                                     WearValue = 0;
		int                                    *val = NULL;
		int                                     kval[11];

		memset(&ObjValue, '\0', sizeof(EXT_BV));

		if (Verbose)
		    fprintf(stderr, "  (%d) Writing \"%s\"[#%d]\n", j,
			    Objects->Object[j].Name, Objects->Object[j].Number);
		else if (!Quiet)
		    spin(stderr);

		fprintf(ofp, "%s", "#OBJECT\n");
		fprintf(ofp, "Vnum      %d\n", Objects->Object[j].Number);
		fprintf(ofp, "Keywords  %s~\n", smaug_strip_cr(Objects->Object[j].Name));	/* called keywords now */
		fprintf(ofp, "Short     %s~\n", smaug_strip_cr(Objects->Object[j].ShortDesc));
		fprintf(ofp, "Long      %s~\n", smaug_strip_cr(Objects->Object[j].Description));	/* LongDesc */
		fprintf(ofp, "Action    %s~\n", smaug_strip_cr(Objects->Object[j].ActionDesc));
		/*
		 * item type, extra flags, wear flags 
		 */
		fprintf(ofp, "Type      %s~\n",
			smaug_strip_cr(smaug_o_types
				       [smaug_convertobjtype(Objects->Object[j].Type)]));
		ObjValue = smaug_convert_oflags(Objects->Object[j].Flags.Extra);
		fprintf(ofp, "Flags     %s~\n",
			smaug_strip_cr(smaug_ext_flag_string(&ObjValue, smaug_o_flags)));
		WearValue = smaug_convert_wflags(Objects->Object[j].Flags.Wear);
		fprintf(ofp, "WFlags    %s~\n",
			smaug_strip_cr(smaug_flag_string(WearValue, smaug_w_flags)));
		/*
		 * magic flags? Wiley don't need to steenkin magic flags! 
		 */
		// fprintf(ofp, "%s~\n", smaug_strip_cr(smaug_flag_string(0, smaug_mag_flags)));

		/*
		 * Now it gets ugly... flag values 
		 */
		val = smaug_convert_item_values(Objects->Object[j].Type,
						Objects->Object[j].Flags.Value[0],
						Objects->Object[j].Flags.Value[1],
						Objects->Object[j].Flags.Value[2],
						Objects->Object[j].Flags.Value[3]);
		/*
		 * Tricky part... spell numbers need to be -1 here, and the NAMES need to be added below 
		 */
		for (k = 0; k < 11; k++)		       /* Keep a backup copy */
		    kval[k] = val[k];
		switch (smaug_convertobjtype(Objects->Object[j].Type)) {
		    case SMAUG_ITEM_PILL:
		    case SMAUG_ITEM_POTION:
		    case SMAUG_ITEM_SCROLL:
			val[1] = HAS_SPELL_INDEX;
			val[2] = HAS_SPELL_INDEX;
			val[3] = HAS_SPELL_INDEX;
			break;
		    case SMAUG_ITEM_STAFF:
		    case SMAUG_ITEM_WAND:
			val[3] = HAS_SPELL_INDEX;
			break;
		    case SMAUG_ITEM_SALVE:
			val[4] = HAS_SPELL_INDEX;
			val[5] = HAS_SPELL_INDEX;
			break;
		}
		fprintf(ofp, "Values    %d %d %d %d %d %d\n",
			val[0], val[1], val[2], val[3], val[4], val[5]);

		fprintf(ofp, "Stats     %d %d %d %d %d\n", Objects->Object[j].Weight, Objects->Object[j].Value, Objects->Object[j].Rent, 0,	/* Wiley 
																		 * doesn't 
																		 * have 
																		 * item 
																		 * levels 
																		 * - 
																		 * Unlimited 
																		 */
			0				       /* Layers aren't supported */
		    );

		switch (smaug_convertobjtype(Objects->Object[j].Type)) {
		    case SMAUG_ITEM_PILL:
		    case SMAUG_ITEM_POTION:
		    case SMAUG_ITEM_SCROLL:
			fprintf(ofp, "Spells    '%s' '%s' '%s'\n",
				kval[1] != -1 ? smaug_skill_names[kval[1]] : "NONE",
				kval[2] != -1 ? smaug_skill_names[kval[2]] : "NONE",
				kval[3] != -1 ? smaug_skill_names[kval[3]] : "NONE");
			break;
		    case SMAUG_ITEM_STAFF:
		    case SMAUG_ITEM_WAND:
			fprintf(ofp, "Spells    '%s'\n",
				kval[3] != -1 ? smaug_skill_names[kval[3]] : "NONE");
			break;
		    case SMAUG_ITEM_SALVE:
			fprintf(ofp, "Spells    '%s' '%s'\n",
				kval[4] != -1 ? smaug_skill_names[kval[4]] : "NONE",
				kval[5] != -1 ? smaug_skill_names[kval[5]] : "NONE");
			break;
		}

		for (k = 0; k < Objects->Object[j].ExtraCount; k++) {
		    fprintf(ofp, "#EXDESC\n");
		    fprintf(ofp, "ExDescKey ");
		    for (m = 0; m < Objects->Object[j].Extra[k].Keyword->Count; m++) {
			fprintf(ofp, "%s%s", Objects->Object[j].Extra[k].Keyword->Word[m],
				(m == Objects->Object[j].Extra[k].Keyword->Count - 1)
				? "" : " ");
		    }
		    fprintf(ofp, "~\nExDesc    %s~\n",
			    smaug_strip_cr(Objects->Object[j].Extra[k].Description));
		    fprintf(ofp, "#ENDEXDESC\n\n");
		}

		/*
		 * Affect type, duration, location, modifier, bitflags 
		 */
		for (k = 0; k < Objects->Object[j].AffectCount; k++)
		    fprintf(ofp, "Affect    %d %d %d %d %d\n", -1, -1,
			    Objects->Object[j].Affect[k].modifier,
			    Objects->Object[j].Affect[k].location, 0);

		/*
		 * No mobprogs either! 
		 */
		fprintf(ofp, "#ENDOBJECT\n\n");
	    }
	}

/* ROOMS */

	for (j = 0; j < Rooms->Count; j++) {
	    if ((remap_zone_vnum(Zones, Rooms->Room[j].Zone) == i) ||
		((Rooms->Room[j].Number >= (!i ? 0 : Zones->Zone[i - 1].Top + 1))&&
		 (Rooms->Room[j].Number <= Zones->Zone[i].Top)
		)) {
		EXT_BV                                  RoomValue;
		EXT_BV                                  ExitValue;

		memset(&RoomValue, '\0', sizeof(EXT_BV));
		memset(&ExitValue, '\0', sizeof(EXT_BV));

		if (Verbose)
		    fprintf(stderr, "  (%d) Writing \"%s\"[#%d]\n", j,
			    Rooms->Room[j].Name, Rooms->Room[j].Number);
		else if (!Quiet)
		    spin(stderr);

		fprintf(ofp, "#ROOM\n");
		fprintf(ofp, "Vnum      %d\n", Rooms->Room[j].Number);
		fprintf(ofp, "Name      %s~\n", smaug_strip_cr(Rooms->Room[j].Name));
		fprintf(ofp, "Desc      %s~\n", smaug_strip_cr(Rooms->Room[j].Description));

		if (Rooms->Room[j].Sector == SECT_TELEPORT) {
		    fprintf(ofp, "Sector    %s~\n",
			    smaug_strip_cr(smaug_sect_types
					   [smaug_convertsector
					    [Rooms->Room[j].TeleportSector]]));
		} else {
		    fprintf(ofp, "Sector    %s~\n",
			    smaug_strip_cr(smaug_sect_types
					   [smaug_convertsector[Rooms->Room[j].Sector]]));
		}

		/*
		 * First, convert bits 
		 */
		RoomValue = smaug_convert_rflags(Rooms->Room[j].Flags);

		fprintf(ofp, "Flags     %s~\n",
			smaug_strip_cr(smaug_ext_flag_string(&RoomValue, smaug_r_flags)));

		if (Rooms->Room[j].Sector == SECT_TELEPORT &&
		    Rooms->Room[j].TeleportTime > 0 && Rooms->Room[j].TeleportTo > 0) {
		    /*
		     * Wiley has no tunnels 
		     */
		    fprintf(ofp, "Stats     %d %d %d\n", Rooms->Room[j].TeleportTime,
			    Rooms->Room[j].TeleportTo, 0);
		}

		for (k = 0; k < Rooms->Room[j].ExitCount; k++) {
		    switch (Rooms->Room[j].Exit[k].Error) {
			case EXIT_OK:
			case EXIT_NON_EUCLIDEAN:
			case EXIT_ONE_WAY:
			    /*
			     * Wiley has no portals 
			     */
			    /*
			     * if ( IS_EXIT_FLAG(xit, EX_PORTAL) ) // don't fold portals continue; 
			     */
			    fprintf(ofp, "#EXIT\n");
			    fprintf(ofp, "Direction %s~\n",
				    smaug_strip_cr(smaug_dir_name
						   [Rooms->Room[j].Exit[k].Direction]));
			    fprintf(ofp, "ToRoom    %d\n", Rooms->Room[j].Exit[k].Room);
			    fprintf(ofp, "Desc      %s~\n",
				    smaug_strip_cr(Rooms->Room[j].Exit[k].Description));
			    if (Rooms->Room[j].Exit[k].Keyword->Count > 0) {
				fprintf(ofp, "Keywords  %s",
					smaug_strip_cr(Rooms->Room[j].Exit[k].
						       Keyword->Word[0]));
				for (x = 1; x < Rooms->Room[j].Exit[k].Keyword->Count; x++) {
				    fprintf(ofp, " %s",
					    smaug_strip_cr(Rooms->Room[j].Exit[k].
							   Keyword->Word[x]));
				}
				fprintf(ofp, "~\n");
			    }

			    ExitValue = smaug_convert_exflags(Rooms->Room[j].Exit[k].Type);

			    fprintf(ofp, "Flags     %s~\n",
				    smaug_ext_flag_string(&ExitValue, smaug_ex_flags));

			    /*
			     * Coordinates do not exist in Wiley 
			     */
			    if (Rooms->Room[j].Exit[k].KeyNumber)
				fprintf(ofp, "Key       %d\n",
					Rooms->Room[j].Exit[k].KeyNumber);
			    fprintf(ofp, "#ENDEXIT\n\n");
		    }
		}

		for (k = 0; k < Rooms->Room[j].ExtraCount; k++) {
		    fprintf(ofp, "#EXDESC\n");
		    if (Rooms->Room[j].Extra[k].Keyword->Count > 0) {
			fprintf(ofp, "ExDescKey %s",
				smaug_strip_cr(Rooms->Room[j].Extra[k].Keyword->Word[0]));
			for (x = 1; x < Rooms->Room[j].Extra[k].Keyword->Count; x++) {
			    fprintf(ofp, " %s",
				    smaug_strip_cr(Rooms->Room[j].Extra[k].Keyword->Word[x]));
			}
		    }
		    fprintf(ofp, "~\nExDesc    %s~\n",
			    smaug_strip_cr(Rooms->Room[j].Extra[k].Description));
		    fprintf(ofp, "#ENDEXDESC\n\n");
		}

#if 0
		/*
		 * SmaugFUSS now puts all resets into rooms 
		 */
		for (x = 0; x < Zones->Zone[Rooms->Room[j].Zone].Count; x++) {
		    int                                     sticky = 0;

		    switch (Zones->Zone[Rooms->Room[j].Zone].Cmds[x].Command) {
			case ZONE_CMD_MOBILE:
			case ZONE_CMD_OBJECT:
			case ZONE_CMD_DOOR:
			case ZONE_CMD_REMOVE:
			    sticky = 1;
			    fprintf(ofp, "Reset %c %d %d %d %d\n",);
			    break;
			case *:
			    if (sticky)
				fprintf(ofp, "Reset %c %d %d %d %d\n",);
			    break;
		    }
		}
#endif

		/*
		 * Wiley has no mudprogs 
		 */

		fprintf(ofp, "#ENDROOM\n\n");
	    }
	}

	/*
	 * save resets 
	 */
	fprintf(ofp, "%s", "#RESETS\n");
	for (x = 0; x < Zones->Zone[i].Count; x++) {
	    switch (Zones->Zone[i].Cmds[x].Command) {
		case ZONE_CMD_MOBILE:
		case ZONE_CMD_OBJECT:
		case ZONE_CMD_EQUIP:
		case ZONE_CMD_PUT:
		case ZONE_CMD_DOOR:
		    fprintf(ofp, "Reset %c %d %d %d %d\n",
			    Zones->Zone[i].Cmds[x].Command,
			    Zones->Zone[i].Cmds[x].IfFlag,
			    Zones->Zone[i].Cmds[x].Arg[0],
			    Zones->Zone[i].Cmds[x].Arg[1], Zones->Zone[i].Cmds[x].Arg[2]);
		    break;
		case ZONE_CMD_GIVE:
		case ZONE_CMD_REMOVE:
		case ZONE_CMD_LEAD:
		case ZONE_CMD_HATE:
		    fprintf(ofp, "Reset %c %d %d %d\n",
			    Zones->Zone[i].Cmds[x].Command,
			    Zones->Zone[i].Cmds[x].IfFlag,
			    Zones->Zone[i].Cmds[x].Arg[0], Zones->Zone[i].Cmds[x].Arg[1]);
		    break;
		default:
		    break;
	    }
	}
	fprintf(ofp, "%s", "S\n\n\n");

	/*
	 * save shops 
	 */
	fprintf(ofp, "%s", "#SHOPS\n");
	fprintf(ofp, "%s", "0\n\n\n");

	/*
	 * save repair shops 
	 */
	fprintf(ofp, "%s", "#REPAIRS\n");
	fprintf(ofp, "%s", "0\n\n\n");

	/*
	 * save specials 
	 */
	fprintf(ofp, "%s", "#SPECIALS\n");
	fprintf(ofp, "%s", "S\n\n\n");

	/*
	 * END 
	 */

	fprintf(ofp, "#$\n");
	fclose(ofp);
	if (Verbose)
	    fprintf(stderr, "done.\n");
	else if (!Quiet) {
	    fprintf(stderr, "done.\r");
	    fflush(stderr);
	}
    }
    if (!Quiet)
	fprintf(stderr, "\n");

    fprintf(listfp, "$\n");
    fclose(listfp);
    if (!Quiet)
	fprintf(stderr, "done.\n");
}

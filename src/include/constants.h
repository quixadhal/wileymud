#ifndef _CONSTANTS_H
#define _CONSTANTS_H

/* Race -- Npc, otherwise */
#define RACE_HALFBREED 0
#define RACE_HUMAN     1
#define RACE_ELVEN     2
#define RACE_DWARF     3
#define RACE_HALFLING  4
#define RACE_GNOME     5

/* end of player races */

#define RACE_REPTILE  6
#define RACE_SPECIAL  7
#define RACE_LYCANTH  8
#define RACE_DRAGON   9
#define RACE_UNDEAD   10
#define RACE_ORC      11
#define RACE_INSECT   12
#define RACE_ARACHNID 13
#define RACE_DINOSAUR 14
#define RACE_FISH     15
#define RACE_BIRD     16
#define RACE_GIANT    17
#define RACE_PREDATOR 18
#define RACE_PARASITE 19
#define RACE_SLIME    20
#define RACE_DEMON    21
#define RACE_SNAKE    22
#define RACE_HERBIV   23
#define RACE_TREE     24
#define RACE_VEGGIE   25
#define RACE_ELEMENT  26
#define RACE_PLANAR   27
#define RACE_DEVIL    28
#define RACE_GHOST    29
#define RACE_GOBLIN   30
#define RACE_TROLL    31
#define RACE_VEGMAN   32
#define RACE_MFLAYER  33
#define RACE_PRIMATE  34
#define RACE_ANIMAL   35
#define RACE_FAERY    36
#define RACE_PLANT    37

#ifndef _CONSTANTS_C
extern const char                      *class_name[];
extern const char                      *exp_needed_text[];
extern const char                      *percent_hit[];
extern const char                      *percent_tired[];
extern const char                      *spell_wear_off_msg[];
extern const char                      *spell_wear_off_soon_msg[];
extern const int                        rev_dir[];
extern const int                        TrapDir[];
extern const int                        movement_loss[];
extern const char                      *dirs[];
extern const char                      *dir_from[];
extern const char                      *ItemDamType[];
extern const char                      *weekdays[7];
extern const char                      *month_name[17];
extern const int                        sharp[];
extern const char                      *where[];
extern const char                      *drinks[];
extern const char                      *drinknames[];
extern const int                        RacialMax[][4];
extern int                              ItemSaveThrows[22][5];
extern const int                        drink_aff[][3];
extern const char                      *color_liquid[];
extern const char                      *fullness[];
extern const struct title_type          titles[6][ABS_MAX_LVL + 1];
extern const char                      *RaceName[];
extern const char                      *item_types[];
extern const char                      *wear_bits[];
extern const char                      *extra_bits[];
extern const char                      *room_bits[];
extern const char                      *exit_bits[];
extern const char                      *sector_types[];
extern const char                      *equipment_types[];
extern const char                      *affected_bits[];
extern const char                      *immunity_names[];
extern const char                      *apply_types[];
extern const char                      *pc_class_types[];
extern const char                      *npc_class_types[];
extern const char                      *action_bits[];
extern const char                      *player_bits[];
extern const char                      *position_types[];
extern const char                      *connected_types[];
extern const int                        thaco[6][ABS_MAX_LVL];
extern const struct str_app_type        str_app[31];
extern const struct dex_skill_type      dex_app_skill[26];
extern const char                       backstab_mult[ABS_MAX_LVL];
extern struct dex_app_type              dex_app[26];
extern struct con_app_type              con_app[26];
extern struct int_app_type              int_app[26];
extern struct wis_app_type              wis_app[26];

#endif

#endif

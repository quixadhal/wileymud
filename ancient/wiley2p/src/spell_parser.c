/* ************************************************************************
 *  file: spell_parser.c , Basic routines and parsing      Part of DIKUMUD *
 *  Usage : Interpreter of spells                                          *
 *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
 ************************************************************************* */

#include <stdio.h>
#include <assert.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "interpreter.h" 
#include "spells.h"
#include "handler.h"

#define MANA_MU 1
#define MANA_CL 1

#define SPELLO(nr, beat, pos, mlev, clev, mana, tar, func) { \
               spell_info[nr].spell_pointer = (func);    \
               spell_info[nr].beats = (beat);            \
               spell_info[nr].minimum_position = (pos);  \
               spell_info[nr].min_usesmana = (mana);     \
               spell_info[nr].min_level_cleric = (clev); \
               spell_info[nr].min_level_magic = (mlev);  \
               spell_info[nr].targets = (tar);           \
        }


/* 100 is the MAX_MANA for a character */
#define USE_MANA(ch, sn)                            \
  MAX(spell_info[sn].min_usesmana,100/MAX(2,(2+GET_LEVEL(ch,BestMagicClass(ch))-SPELL_LEVEL(ch,sn))))

/* Global data */

extern struct room_data *world;
extern struct char_data *character_list;
extern char *spell_wear_off_msg[];
extern char *spell_wear_off_soon_msg[];


/* Extern procedures */

char *strdup(char *str);

/* Extern procedures */
void cast_animate_dead( byte level, struct char_data *ch, char *arg, int type,
           struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_conjure_elemental( byte level, struct char_data *ch, char *arg, 
          int type, struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_shelter( byte level, struct char_data *ch, char *arg, 
          int type, struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_acid_blast( byte level, struct char_data *ch, char *arg, int type,
         struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_armor( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_teleport( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_bless( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_blindness( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_burning_hands( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_call_lightning( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_charm_person( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_charm_monster( byte level, struct char_data *ch, char *arg, int si, 
      struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cacaodemon( byte level, struct char_data *ch, char *arg, int si, 
      struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_chill_touch( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_shocking_grasp( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_clone( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_colour_spray( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_control_weather( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_create_food( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_create_water( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cure_blind( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cure_critic( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cause_critic( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cure_light( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cause_light( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_curse( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cont_light( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, 
         struct obj_data *tar_obj);
void cast_calm( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, 
         struct obj_data *tar_obj);
void cast_detect_evil( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_detect_invisibility( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_detect_magic( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_detect_poison( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_dispel_evil( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_dispel_good( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_dispel_magic( byte level, struct char_data *ch, char *arg, int type,
           struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_earthquake( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_enchant_weapon( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_energy_drain( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_fear( byte level, struct char_data *ch, char *arg, int type,
         struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_fireball( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_flamestrike( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_flying( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_fly_group( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_harm( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_heal( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_infravision( byte level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_invisibility( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cone_of_cold( byte level, struct char_data *ch, char *arg, int type,
           struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_ice_storm( byte level, struct char_data *ch, char *arg, int type,
        struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_knock( byte level, struct char_data *ch, char *arg, int type,
    struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_know_alignment(byte level, struct char_data *ch, char *arg, int type,
       struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_true_seeing(byte level, struct char_data *ch, char *arg, int type,
       struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_minor_creation(byte level, struct char_data *ch, char *arg, int type,
       struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_faerie_fire(byte level, struct char_data *ch, char *arg, int type,
       struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_faerie_fog(byte level, struct char_data *ch, char *arg, int type,
       struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_mana(byte level, struct char_data *ch, char *arg, int type,
       struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_lightning_bolt( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_light( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, 
    struct obj_data *tar_obj);
void cast_locate_object( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_magic_missile( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_mon_sum1( byte level, struct char_data *ch, char *arg, int si, 
       struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_mon_sum2( byte level, struct char_data *ch, char *arg, int si, 
       struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_mon_sum3( byte level, struct char_data *ch, char *arg, int si, 
       struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_mon_sum4( byte level, struct char_data *ch, char *arg, int si, 
       struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_mon_sum5( byte level, struct char_data *ch, char *arg, int si, 
       struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_mon_sum6( byte level, struct char_data *ch, char *arg, int si, 
       struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_mon_sum7( byte level, struct char_data *ch, char *arg, int si, 
       struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_meteor_swarm( byte level, struct char_data *ch, char *arg, int si, 
           struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_poly_self( byte level, struct char_data *ch, char *arg, int si, 
     struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_poison( byte level, struct char_data *ch, char *arg, int si, 
     struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_protection_from_evil( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_remove_curse( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_sanctuary( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_sleep( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_strength( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_stone_skin( byte level, struct char_data *ch, char *arg, int si, 
         struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_summon( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_ventriloquate( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_word_of_recall( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_water_breath( byte level, struct char_data *ch, char *arg, int si, 
           struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_remove_poison( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_remove_paralysis( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_weakness( byte level, struct char_data *ch, char *arg, int type,
       struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_sense_life( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_identify( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_paralyze( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_dragon_breath( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *potion);
void cast_fireshield( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cure_serious( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cause_serious( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_refresh( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_second_wind( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_shield( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_turn( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_aid( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_succor( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_astral_walk( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_resurrection( byte level, struct char_data *ch, char *arg, int si,
struct char_data *tar_ch, struct obj_data *tar_obj);



struct spell_info_type spell_info[MAX_SPL_LIST];

char *spells[]=
{
   "armor",               /* 1 */
   "teleport",
   "bless",
   "blindness",
   "burning hands",
   "call lightning",
   "charm person",
   "chill touch",
   "clone",
   "colour spray",        /* colour spray */
   "control weather",     /* 11 */
   "create food",
   "create water",
   "cure blind",
   "cure critic",
   "cure light",
   "curse",
   "detect evil",
   "detect invisibility",
   "detect magic",
   "detect poison",       /* 21 */
   "dispel evil",
   "earthquake",
   "enchant weapon",
   "energy drain",
   "fireball",
   "harm",
   "heal",
   "invisibility",
   "lightning bolt",
   "locate object",      /* 31 */
   "magic missile",
   "poison",
   "protection from evil",
   "remove curse",
   "sanctuary",
   "shocking grasp",
   "sleep",
   "strength",
   "summon",
   "ventriloquate",      /* 41 */
   "word of recall",
   "remove poison",
   "sense life",         /* 44 */

   /* RESERVED SKILLS */
   "SKILL_SNEAK",        /* 45 */
   "SKILL_HIDE",
   "SKILL_STEAL",
   "SKILL_BACKSTAB",
   "SKILL_PICK_LOCK",
   "SKILL_KICK",         /* 50 */
   "SKILL_BASH",
   "SKILL_RESCUE",
   /* NON-CASTABLE SPELLS (Scrolls/potions/wands/staffs) */

   "identify",           /* 53 */
   "infravision",        
   "cause light",        
   "cause critical",
   "flamestrike",
   "dispel good",      
   "weakness",
   "dispel magic",
   "knock",
   "know alignment",
   "animate dead",
   "paralyze",
   "remove paralysis",
   "fear",
   "acid blast",  /* 67 */
   "water breath",
   "fly",
   "cone of cold",   /* 70 */
   "meteor swarm",
   "ice storm",
   "shield",
   "monsum one",
   "monsum two",
   "monsum three",
   "monsum four",
   "monsum five",
   "monsum six",
   "monsum seven",  /* 80 */
   "fireshield",
   "charm monster",
   "cure serious",
   "cause serious",
   "refresh",
   "second wind",
   "turn",
   "succor",
   "create light",
   "continual light", /* 90 */
   "calm",
   "stone skin",
   "conjure elemental",
   "true sight",
   "minor creation",
   "faerie fire",
   "faerie fog",
   "cacaodemon",
   "polymorph self",
   "mana",  /* 100 */
   "astral walk",
   "group fly",		/* 102 - spell_fly_group */	
   "aid",		/* 103 - spell_aid */
   "shelter",		/* 104 - spell_shelter */
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",  /* 110 */
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",  /* 120 */
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",  /* 130 */
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",  /* 140 */
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",  /* 150 */
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",  /* 160 */
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",  /* 170 */
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",  /* 180 */
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",  /* 190 */
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "****",
   "geyser",  /* 200 */
   "fire breath",
   "gas breath",
   "frost breath",
   "acid breath",
   "lightning breath",
   "****",
   "****",
   "****",
   "****",
   "****",  /* 210 */
   "****",
   "SKILL_HUNT",    /*  (180) */
   "\n"
};


const byte saving_throws[6][5][ABS_MAX_LVL] = {
{
  {16,14,14,14,14,14,13,13,13,13,13,11,11,11,11,11,10,10,10,10,10, 8, 6, 4, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0, 0, 0, 0, 0, 0},
  {13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 5, 5, 5, 5, 3, 2, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, 0, 0, 0, 0, 0, 0},
  {15,13,13,13,13,13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 4, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 0, 0, 0, 0, 0, 0, 0},
  {17,15,15,15,15,15,13,13,13,13,13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 5, 3, 3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0, 0, 0, 0, 0, 0, 0},
  {14,12,12,12,12,12,10,10,10,10,10, 8, 8, 8, 8, 8, 6, 6, 6, 6, 6, 4, 3, 2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0, 0, 0, 0, 0, 0, 0}
},
{
  {11,10,10,10, 9, 9, 9, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 2, 2, 2, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, 0, 0, 0, 0, 0, 0},
  {16,14,14,14,13,13,13,11,11,11,10,10,10, 9, 9, 9, 8, 8, 8, 6, 6, 5, 4, 3, 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,0, 0, 0, 0, 0,0},
  {15,13,13,13,12,12,12,10,10,10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 5, 5, 4, 3, 2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0,0,0,0,0},
  {18,16,16,16,15,15,15,13,13,13,12,12,12,11,11,11,10,10,10, 8, 8, 7, 6, 5, 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,0,0,0,0,0,0,0},
  {17,15,15,15,14,14,14,12,12,12,11,11,11,10,10,10, 9, 9, 9, 7, 7, 6, 5, 4, 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,0,0,0,0,0,0,0}
},
{
  {15,13,13,13,13,12,12,12,12,11,11,11,11,10,10,10,10, 9, 9, 9, 9, 8, 7, 6, 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,0,0,0,0,0,0,0},
  {16,14,14,14,14,12,12,12,12,10,10,10,10, 8, 8, 8, 8, 6, 6, 6, 6, 4, 3, 2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0},
  {14,12,12,12,12,11,11,11,11,10,10,10,10, 9, 9, 9, 9, 8, 8, 8, 8, 7, 5, 3, 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0},
  {18,16,16,16,16,15,15,15,15,14,14,14,14,13,13,13,13,12,12,12,12,11, 9, 5, 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,0,0,0,0,0,0,0},
  {17,15,15,15,15,13,13,13,13,11,11,11,11, 9, 9, 9, 9, 7, 7, 7, 7, 5, 3, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0}
}, {
  {16,14,14,13,13,11,11,10,10, 8, 8, 7, 7, 5, 5, 4, 4, 3, 3, 3, 3, 2, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0},
  {18,16,16,15,15,13,13,12,12,10,10, 9, 9, 7, 7, 6, 6, 5, 5, 5, 5, 4, 3, 2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0},
  {17,15,15,14,14,12,12,11,11, 9, 9, 8, 8, 6, 6, 5, 5, 4, 4, 4, 4, 3, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0},
  {20,17,17,16,16,13,13,12,12, 9, 9, 8, 8, 5, 5, 4, 4, 4, 4, 4, 4, 3, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0},
  {19,17,17,16,16,14,14,13,13,11,11,10,10, 8, 8, 7, 7, 6, 6, 6, 6, 4, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0}
},{
  {16,14,14,14,14,14,13,13,13,13,13,11,11,11,11,11,10,10,10,10,10, 8, 6, 4, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0, 0, 0, 0, 0, 0},
  {13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 5, 5, 5, 5, 3, 2, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, 0, 0, 0, 0, 0, 0},
  {15,13,13,13,13,13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 4, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 0, 0, 0, 0, 0, 0, 0},
  {17,15,15,15,15,15,13,13,13,13,13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 5, 3, 3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0, 0, 0, 0, 0, 0, 0},
  {14,12,12,12,12,12,10,10,10,10,10, 8, 8, 8, 8, 8, 6, 6, 6, 6, 6, 4, 3, 2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0, 0, 0, 0, 0, 0, 0}
},{
  {11,10,10,10, 9, 9, 9, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 2, 2, 2, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, 0, 0, 0, 0, 0, 0},
  {16,14,14,14,13,13,13,11,11,11,10,10,10, 9, 9, 9, 8, 8, 8, 6, 6, 5, 4, 3, 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,0, 0, 0, 0, 0,0},
  {15,13,13,13,12,12,12,10,10,10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 5, 5, 4, 3, 2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0,0,0,0,0},
  {18,16,16,16,15,15,15,13,13,13,12,12,12,11,11,11,10,10,10, 8, 8, 7, 6, 5, 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,0,0,0,0,0,0,0},
  {17,15,15,15,14,14,14,12,12,12,11,11,11,10,10,10, 9, 9, 9, 7, 7, 6, 5, 4, 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,0,0,0,0,0,0,0}
}
};

int SPELL_LEVEL(struct char_data *ch, int sn) 
{
  if((HasClass(ch, CLASS_MAGIC_USER)) && (HasClass(ch, CLASS_CLERIC))) 
  {
    return(MIN(spell_info[sn].min_level_magic,spell_info[sn].min_level_cleric));
  }
  else 
  {
    if (HasClass(ch, CLASS_MAGIC_USER)) 
    {
      return(spell_info[sn].min_level_magic);
    }
    else
    {
      return(spell_info[sn].min_level_cleric);
    }
  }
}

void affect_update( void )
{
  static struct affected_type *af, *next_af_dude;
  static struct char_data *i;

  for (i = character_list; i; i = i->next)
    for(af = i->affected; af; af = next_af_dude) 
    {
      next_af_dude = af->next;
      if (af->duration >= 1)
      {
        af->duration--;
	if(af->duration == 0)
	{
	  if(*spell_wear_off_soon_msg[af->type])
          {
	    send_to_char(spell_wear_off_soon_msg[af->type],i);
	    send_to_char("\n\r",i);
          }
        }
      }
      else
      {
        if((af->type > 0) && (af->type <= MAX_EXIST_SPELL)) 
        {
          if(!af->next || (af->next->type != af->type) || (af->next->duration > 0)) 
          {
            if(*spell_wear_off_msg[af->type]) 
            {
              send_to_char(spell_wear_off_msg[af->type], i);
              send_to_char("\n\r", i);

        /* check to see if the exit down is connected, if so make the person */
        /* fall down into that room and take 1d6 damage */

              affect_remove(i,af);
              return;
            }
          }
        }
	else 
        if(af->type>=FIRST_BREATH_WEAPON && af->type <=LAST_BREATH_WEAPON ) 
        {
          extern funcp bweapons[];
          bweapons[af->type-FIRST_BREATH_WEAPON](-af->modifier/2, i, "",SPELL_TYPE_SPELL, i, 0);
          if (!i->affected)
          /* oops, you're dead :) */
          break;
        }
        affect_remove(i, af);
      }
    }
}

void clone_char(struct char_data *ch)
{
  extern struct index_data *mob_index;
  struct char_data *clone;
  struct affected_type *af;
  int i;

  CREATE(clone, struct char_data, 1);


  clear_char(clone);       /* Clear EVERYTHING! (ASSUMES CORRECT) */

  clone->player    = ch->player;
  clone->abilities = ch->abilities;

  for (i=0; i<5; i++)
    clone->specials.apply_saving_throw[i] = ch->specials.apply_saving_throw[i];

  for (af=ch->affected; af; af = af->next)
    affect_to_char(clone, af);

  for (i=0; i<3; i++)
    GET_COND(clone,i) = GET_COND(ch, i);

  clone->points = ch->points;

  for (i=0; i<MAX_SKILLS; i++)
    clone->skills[i] = ch->skills[i];

  clone->specials = ch->specials;
  clone->specials.fighting = 0;

  GET_NAME(clone) = strdup(GET_NAME(ch));

  clone->player.short_descr = strdup(ch->player.short_descr);

  clone->player.long_descr = strdup(ch->player.long_descr);

  clone->player.description = 0;
  /* REMEMBER EXTRA DESCRIPTIONS */

  GET_TITLE(clone) = strdup(GET_TITLE(ch));

  clone->nr = ch->nr;

  if (IS_NPC(clone))
    mob_index[clone->nr].number++;
  else { /* Make PC's into NPC's */
    clone->nr = -1;
    SET_BIT(clone->specials.act, ACT_ISNPC);
  }

  clone->desc = 0;
  clone->followers = 0;
  clone->master = 0;

  clone->next = character_list;
  character_list = clone;

  char_to_room(clone, ch->in_room);
}



void clone_obj(struct obj_data *obj)
{
  struct obj_data *clone;

  CREATE(clone, struct obj_data, 1);

  *clone = *obj;

  clone->name               = strdup(obj->name);
  clone->description        = strdup(obj->description);
  clone->short_description  = strdup(obj->short_description);
  clone->action_description = strdup(obj->action_description);
  clone->ex_description     = 0;

  /* REMEMBER EXTRA DESCRIPTIONS */
  clone->carried_by         = 0;
  clone->equipped_by        = 0;
  clone->in_obj             = 0;
  clone->contains           = 0;
  clone->next_content       = 0;
  clone->next               = 0;

  /* VIRKER IKKE ENDNU */
}



/* Check if making CH follow VICTIM will create an illegal */
/* Follow "Loop/circle"                                    */
bool circle_follow(struct char_data *ch, struct char_data *victim)
{
  struct char_data *k;

  for(k=victim; k; k=k->master) {
    if (k == ch)
      return(TRUE);
  }

  return(FALSE);
}



/* Called when stop following persons, or stopping charm */
/* This will NOT do if a character quits/dies!!          */
void stop_follower(struct char_data *ch)
{
  struct follow_type *j, *k;

  if (!ch->master) return;

  if (IS_AFFECTED(ch, AFF_CHARM)) {
    act("You realize that $N is a jerk!", FALSE, ch, 0, ch->master, TO_CHAR);
    act("$n realizes that $N is a jerk!", FALSE, ch, 0, ch->master, TO_NOTVICT);
    act("$n hates your guts!", FALSE, ch, 0, ch->master, TO_VICT);
    if (affected_by_spell(ch, SPELL_CHARM_PERSON))
      affect_from_char(ch, SPELL_CHARM_PERSON);
  } else {
    act("You stop following $N.", FALSE, ch, 0, ch->master, TO_CHAR);
           if (!IS_SET(ch->specials.act,PLR_STEALTH)) {
    act("$n stops following $N.", FALSE, ch, 0, ch->master, TO_NOTVICT);
    act("$n stops following you.", FALSE, ch, 0, ch->master, TO_VICT);
     }
  }

  if (ch->master->followers->follower == ch) { /* Head of follower-list? */
    k = ch->master->followers;
    ch->master->followers = k->next;
    free(k);
  } else { /* locate follower who is not head of list */

    for(k = ch->master->followers; k->next && k->next->follower!=ch; k=k->next)  ;

    if (k->next) {
       j = k->next;
       k->next = j->next;
       free(j);
    }
  }

  ch->master = 0;
  REMOVE_BIT(ch->specials.affected_by, AFF_CHARM | AFF_GROUP);
}



/* Called when a character that follows/is followed dies */
void die_follower(struct char_data *ch)
{
  struct follow_type *j, *k;

  if (ch->master)
    stop_follower(ch);

  for (k=ch->followers; k; k=j) {
    j = k->next;
    stop_follower(k->follower);
  }
}



/* Do NOT call this before having checked if a circle of followers */
/* will arise. CH will follow leader                               */
void add_follower(struct char_data *ch, struct char_data *leader)
{
  struct follow_type *k;

  assert(!ch->master);

  ch->master = leader;

  CREATE(k, struct follow_type, 1);

  k->follower = ch;
  k->next = leader->followers;
  leader->followers = k;

        
  act("You now follow $N.", FALSE, ch, 0, leader, TO_CHAR);
        if (!IS_SET(ch->specials.act, PLR_STEALTH)) {
     act("$n starts following you.", TRUE, ch, 0, leader, TO_VICT);
     act("$n now follows $N.", TRUE, ch, 0, leader, TO_NOTVICT);
  }
}



say_spell( struct char_data *ch, int si )
{
  char buf[MAX_STRING_LENGTH], splwd[MAX_BUF_LENGTH];
  char buf2[MAX_STRING_LENGTH];

  int j, offs;
  struct char_data *temp_char;


  struct syllable 
  {
    char org[10];
    char new[10];
  };

  struct syllable syls[] = {
  { " ", " " },
  { "ar", "abra"   },
  { "au", "mela"    },
  { "bless", "kado" },
  { "blind", "nose" },
  { "bur", "mosa" },
  { "cu", "judi" },
  { "ca", "jedi" },
  { "de", "oculo"},
  { "en", "fido" },
  { "light", "dies" },
  { "lo", "hi" },
  { "mor", "sido" },
  { "move", "zak" },
  { "ness", "lacri" },
  { "ning", "illa" },
  { "per", "duda" },
  { "ra", "gru"   },
  { "re", "candus" },
  { "son", "sabru" },
  { "se",  "or"},
  { "tect", "cula" },
  { "tri", "infa" },
  { "ven", "nofo" },
  {"a", "a"},{"b","b"},{"c","q"},{"d","e"},{"e","z"},{"f","y"},{"g","o"},
  {"h", "p"},{"i","u"},{"j","y"},{"k","t"},{"l","s"},{"m","w"},{"n","i"},
  {"o", "a"},{"p","t"},{"q","d"},{"r","f"},{"s","g"},{"t","h"},{"u","j"},
  {"v", "z"},{"w","x"},{"x","b"},{"y","l"},{"z","k"}, {"",""}
 };

  strcpy(buf, "");
  strcpy(splwd, spells[si-1]);

  offs = 0;

  while(*(splwd+offs)) 
  {
    for(j=0; *(syls[j].org); j++)
      if (strncmp(syls[j].org, splwd+offs, strlen(syls[j].org))==0) 
      {
        strcat(buf, syls[j].new);
        if (strlen(syls[j].org))
          offs+=strlen(syls[j].org);
        else
          ++offs;
      }
  }


  sprintf(buf2,"$n utters the words, '%s'", buf);
  sprintf(buf, "$n utters the words, '%s'", spells[si-1]);

  for(temp_char = real_roomp(ch->in_room)->people;temp_char;temp_char = temp_char->next_in_room)
    if(temp_char != ch) 
    {
/*
**  Remove-For-Multi-Class
      if (ch->player.class == temp_char->player.class)

*/
      if((GET_LEVEL(temp_char,CLERIC_LEVEL_IND) >0)&&(GET_LEVEL(temp_char,MAGE_LEVEL_IND)>0))
        act(buf, FALSE, ch, 0, temp_char, TO_VICT);
      else
        act(buf2, FALSE, ch, 0, temp_char, TO_VICT);

    }
}

bool saves_spell(struct char_data *ch, sh_int save_type)
{
  int save;

  /* Negative apply_saving_throw makes saving throw better! */

  save = ch->specials.apply_saving_throw[save_type];

  if(IS_PC(ch)) {
/*
**  Remove-For-Multi-Class
*/
  save += saving_throws[BestMagicClass(ch)][save_type][GET_LEVEL(ch,BestMagicClass(ch))];
  if (GetMaxLevel(ch) > MAX_MORT)
    return(TRUE);
  }

  return(MAX(1,save) < number(1,20));
}

bool ImpSaveSpell(struct char_data *ch, sh_int save_type, int mod)
{
  int save;

        /* Positive mod is better for save */

  /* Negative apply_saving_throw makes saving throw better! */

  save = ch->specials.apply_saving_throw[save_type] - mod;


  if(IS_PC(ch)) {
/*
**  Remove-For-Multi-Class
*/
  save += saving_throws[BestMagicClass(ch)][save_type][GET_LEVEL(ch,BestMagicClass(ch))];
  if (GetMaxLevel(ch) >= LOW_IMMORTAL)
    return(TRUE);
  }

  return(MAX(1,save) < number(1,20));
}

char *skip_spaces(char *string)
{
  for(;*string && (*string)==' ';string++);

  return(string);
}

/* Assumes that *argument does start with first letter of chopped string */

void do_cast(struct char_data *ch, char *argument, int cmd)
{
  struct room_data *rp;
  struct obj_data *tar_obj;
  struct char_data *tar_char;
  char name[MAX_INPUT_LENGTH];
  int qend, spl, i;
  bool target_ok;
  
  if(IS_NPC(ch) && (IS_NOT_SET(ch->specials.act, ACT_POLYSELF)))
    return;

  if (!IsHumanoid(ch)) 
  {
    send_to_char("Sorry, you don't have the right form for that.\n\r",ch);
    return;
  }
  
  if(!IS_IMMORTAL(ch)) 
  {
    if (BestMagicClass(ch) == WARRIOR_LEVEL_IND) 
    {
      send_to_char("Think you had better stick to fighting...\n\r", ch);
      return;
    } else if (BestMagicClass(ch) == THIEF_LEVEL_IND) {
      send_to_char("Think you should stick to robbing and stealing...\n\r", ch);
      return;
    } else if ((BestMagicClass(ch) == RANGER_LEVEL_IND)&&
               (GET_LEVEL(ch,RANGER_LEVEL_IND)<10)) {
      send_to_char("In time, you shall have the power of nature...\n\r",ch);
      return;
    }
  }
 
  rp = real_roomp(ch->in_room);
  if(IS_SET(rp->room_flags, NO_MAGIC))
  {
    send_to_char("Your lips do not move, no magic appears.\n\r",ch);
    return;
  }
 
  argument = skip_spaces(argument);
  
  if(!(*argument)) 
  {
    send_to_char("cast 'spell name' <target>\n\r", ch);
    return;
  }
  
  if (*argument != '\'') 
  {
    send_to_char("Spells must always be enclosed by single quotes: '\n\r",ch);
    return;
  }
  
  /* Locate the last quote && lowercase the magic words (if any) */
  
  for (qend=1; *(argument+qend) && (*(argument+qend) != '\'') ; qend++)
    *(argument+qend) = LOWER(*(argument+qend));
  
  if (*(argument+qend) != '\'') {
    send_to_char("Magic must always be enclosed by single quotes: '\n\r",ch);
    return;
  }
  
  spl = old_search_block(argument, 1, qend-1,spells, 0);
  
  if (!spl) 
  {
    send_to_char("Your lips do not move, no magic appears.\n\r",ch);
    return;
  }
  
  if ((spl > 0) && (spl < MAX_SKILLS) && spell_info[spl].spell_pointer) 
  {
    if (GET_POS(ch) < spell_info[spl].minimum_position) 
    {
      switch(GET_POS(ch)) 
      {
      case POSITION_SLEEPING :
  send_to_char("You dream about great magical powers.\n\r", ch);
  break;
      case POSITION_RESTING :
  send_to_char("You can't concentrate enough while resting.\n\r",ch);
  break;
      case POSITION_SITTING :
  send_to_char("You can't do this sitting!\n\r", ch);
  break;
      case POSITION_FIGHTING :
  send_to_char("Impossible! You can't concentrate enough!.\n\r", ch);
  break;
      default:
  send_to_char("It seems like you're in pretty bad shape!\n\r",ch);
  break;
      } 
    }
    else
    {
      if (!IS_IMMORTAL(ch)) 
      {
  if ((spell_info[spl].min_level_magic>GET_LEVEL(ch,MAGE_LEVEL_IND)) &&
      (spell_info[spl].min_level_cleric>GET_LEVEL(ch,CLERIC_LEVEL_IND)) &&
            (GET_LEVEL(ch,RANGER_LEVEL_IND) < 10 ))
        {
    send_to_char("Sorry, you can't do that.\n\r", ch);
    return;
  }
      }
      
      argument+=qend+1; /* Point to the last ' */
      for(;*argument == ' '; argument++);
      
      /* **************** Locate targets **************** */
      
      target_ok = FALSE;
      tar_char = 0;
      tar_obj = 0;
      
      if (IS_SET(spell_info[spl].targets, TAR_VIOLENT) &&
    check_peaceful(ch, "This is a magic dead area."))
  return;
      
      if(IS_NOT_SET(spell_info[spl].targets, TAR_IGNORE)) 
      {
        argument = one_argument(argument, name);
        if (*name) 
        {
          if (IS_SET(spell_info[spl].targets, TAR_CHAR_ROOM)) 
          {
            if (tar_char = get_char_room_vis(ch, name)) 
            {
              if (tar_char == ch || tar_char == ch->specials.fighting ||
                  tar_char->attackers < 6 || 
                  tar_char->specials.fighting == ch)
                  target_ok = TRUE;
        else {
    send_to_char("Too much noise, you can't concentrate.\n\r", ch);
    return;
        }
      }
    }

    if (!target_ok && IS_SET(spell_info[spl].targets, TAR_CHAR_WORLD))
      if (tar_char = get_char_vis(ch, name))
        target_ok = TRUE;
    
    if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_INV))
      if (tar_obj = get_obj_in_list_vis(ch, name, ch->carrying))
        target_ok = TRUE;
    
    if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_ROOM))
      if (tar_obj = get_obj_in_list_vis(ch, name, real_roomp(ch->in_room)->contents))
        target_ok = TRUE;
    
    if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_WORLD))
      if (tar_obj = get_obj_vis(ch, name))
        target_ok = TRUE;
    
    if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_EQUIP)) {
      for(i=0; i<MAX_WEAR && !target_ok; i++)
        if (ch->equipment[i] && str_cmp(name, ch->equipment[i]->name) == 0) {
    tar_obj = ch->equipment[i];
    target_ok = TRUE;
        }
    }
    
    if (!target_ok && IS_SET(spell_info[spl].targets, TAR_SELF_ONLY))
      if (str_cmp(GET_NAME(ch), name) == 0) {
        tar_char = ch;
        target_ok = TRUE;
      }
    
    if (!target_ok && IS_SET(spell_info[spl].targets, TAR_NAME)) {
      tar_obj = (void*)name;
      target_ok = TRUE;
    }
    
    if (tar_char) {
      if (IS_NPC(tar_char)) 
        if (IS_SET(tar_char->specials.act, ACT_IMMORTAL)) {
    send_to_char("You can't cast magic on that!",ch);
    return;
        }
    }
  }
  else
  { /* No argument was typed */
    if (IS_SET(spell_info[spl].targets, TAR_FIGHT_SELF))
      if (ch->specials.fighting) 
      {
        tar_char = ch;
        target_ok = TRUE;
      }
    
    if (!target_ok && IS_SET(spell_info[spl].targets, TAR_FIGHT_VICT))
      if (ch->specials.fighting) {
        /* WARNING, MAKE INTO POINTER */
        tar_char = ch->specials.fighting;
        target_ok = TRUE;
      }
    
    if (!target_ok && IS_SET(spell_info[spl].targets, TAR_SELF_ONLY)) {
      tar_char = ch;
      target_ok = TRUE;
    }
  }
      }
      else
      {
  target_ok = TRUE; /* No target, is a good target */
      }
      
      if (!target_ok) 
      {
  if (*name) 
  {
    if (IS_SET(spell_info[spl].targets, TAR_CHAR_ROOM))
      send_to_char("Nothing here with that name.\n\r",ch);
    else
    if (IS_SET(spell_info[spl].targets, TAR_CHAR_WORLD))
      send_to_char("Nobody playing by that name.\n\r", ch);
    else if (IS_SET(spell_info[spl].targets, TAR_OBJ_INV))
      send_to_char("You are not carrying anything like that.\n\r", ch);
    else if (IS_SET(spell_info[spl].targets, TAR_OBJ_ROOM))
      send_to_char("Nothing here by that name.\n\r", ch);
    else if (IS_SET(spell_info[spl].targets, TAR_OBJ_WORLD))
      send_to_char("Nothing at all by that name.\n\r", ch);
    else if (IS_SET(spell_info[spl].targets, TAR_OBJ_EQUIP))
      send_to_char("You are not wearing anything like that.\n\r", ch);
    else if (IS_SET(spell_info[spl].targets, TAR_OBJ_WORLD))
      send_to_char("Nothing at all by that name.\n\r", ch);
  }
  else
  { /* Nothing was given as argument */
    if (spell_info[spl].targets < TAR_OBJ_INV)
      send_to_char("Who should the spell be cast upon?\n\r", ch);
    else
      send_to_char("What should the spell be cast upon?\n\r", ch);
  }
  return;
      }
      else
      { /* TARGET IS OK */

  if ((tar_char == ch) && IS_SET(spell_info[spl].targets, TAR_SELF_NONO)) 
        {
    send_to_char("You can not cast this spell upon yourself.\n\r", ch);
    return;
  }
  else
        if((tar_char!=ch)&&IS_SET(spell_info[spl].targets,TAR_SELF_ONLY)) 
        {
    send_to_char("You can only cast this spell upon yourself.\n\r", ch);
    return;
  }
  else
  if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == tar_char)) 
  {
    send_to_char("You are afraid that it could harm your master.\n\r",ch);
    return;
  }
      }
      
      if (GetMaxLevel(ch) < LOW_IMMORTAL) 
      {
  if (GET_MANA(ch) < USE_MANA(ch, spl)) 
  {
    send_to_char("You can't summon enough energy to cast the spell.\n\r", ch);
    return;
  }
      }
     
      if (spl != SPELL_VENTRILOQUATE)  /* :-) */
  say_spell(ch, spl);
      
      WAIT_STATE(ch, spell_info[spl].beats);
      
      if ((spell_info[spl].spell_pointer == 0) && spl>0)
  send_to_char("Sorry, this magic has not yet been implemented\n\r",ch);
      else
      {
  if (number(1,100) > (ch->skills[spl].learned + ( GetMaxLevel(ch)/5 ) ) )
  { /* 101% is failure */
    send_to_char("You lost your concentration!\n\r", ch);
    GET_MANA(ch) -= (USE_MANA(ch, spl)>>1);
    return;
  }
  send_to_char("You Cast!\n\r",ch);
  if(ch->skills[spl].learned < 60)
  {
    if(ch->skills[SKILL_SPELLCRAFT].learned > number(1,101))
      ch->skills[spl].learned += 5;
    else
      ch->skills[spl].learned += 2;
  }

  ((*spell_info[spl].spell_pointer)(GET_LEVEL(ch,BestMagicClass(ch)),
         ch,argument, SPELL_TYPE_SPELL, tar_char, tar_obj));
  GET_MANA(ch)-=(USE_MANA(ch, spl));
      }
      
    } /* if GET_POS < min_pos */
    return;
  }
  
  switch (number(1,5))
  {
  case 1: send_to_char("Bylle Grylle Grop Gryf???\n\r", ch); break;
  case 2: send_to_char("Olle Bolle Snop Snyf?\n\r",ch); break;
  case 3: send_to_char("Olle Grylle Bolle Bylle?!?\n\r",ch); break;
  case 4: send_to_char("Gryffe Olle Gnyffe Snop???\n\r",ch); break;
  default: send_to_char("Bolle Snylle Gryf Bylle?!!?\n\r",ch); break;
  }
}

void assign_spell_pointers(void)
{
  int i;
  
  for(i=0; i<MAX_SPL_LIST; i++)
    spell_info[i].spell_pointer = 0;
  
  
  /* From spells1.c */
  
  SPELLO(32,12,POSITION_FIGHTING, 1, LOW_IMMORTAL, 10,
   TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_magic_missile);
  
  SPELLO( 8,12,POSITION_FIGHTING, 3, LOW_IMMORTAL, 15,
   TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_chill_touch);
  
  SPELLO( 5,24,POSITION_FIGHTING, 5, LOW_IMMORTAL, 30,
   TAR_IGNORE | TAR_VIOLENT, cast_burning_hands);
  
  SPELLO(37,12,POSITION_FIGHTING, 1, LOW_IMMORTAL, 15,
   TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_shocking_grasp);
  
  SPELLO(23,24,POSITION_FIGHTING, LOW_IMMORTAL, 8, 15,
   TAR_IGNORE | TAR_VIOLENT, cast_earthquake);
  
  SPELLO(30,24,POSITION_FIGHTING, 9, LOW_IMMORTAL, 15,
   TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_lightning_bolt);
  
  SPELLO(10,24,POSITION_FIGHTING, 11,LOW_IMMORTAL, 15,
   TAR_IGNORE | TAR_VIOLENT, 0 ); /* cast colour spray */
  
  SPELLO(22,24,POSITION_FIGHTING, LOW_IMMORTAL, 12, 100,
   TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_dispel_evil);
  
  SPELLO(26,36,POSITION_FIGHTING, 15, LOW_IMMORTAL, 15,
   TAR_IGNORE | TAR_VIOLENT, cast_fireball);
  
  SPELLO( 6,36,POSITION_FIGHTING, LOW_IMMORTAL, 15, 15,
   TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_call_lightning);
  
  SPELLO(25,36,POSITION_FIGHTING, 17, LOW_IMMORTAL, 35,
   TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_energy_drain);
  
  SPELLO(27,36,POSITION_FIGHTING, LOW_IMMORTAL, 17, 50,
   TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_harm);
  
  /* Spells2.c */
  
  SPELLO( 1,12,POSITION_STANDING, 4,  1, 5, TAR_CHAR_ROOM, cast_armor);
  
  SPELLO( 2,12,POSITION_FIGHTING, 8, LOW_IMMORTAL, 33,
   TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_teleport);
  
  SPELLO( 3,12,POSITION_STANDING,LOW_IMMORTAL,  1, 5,
   TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_CHAR_ROOM, cast_bless);
  
  SPELLO( 4,24,POSITION_FIGHTING, 8,  6, 5,
   TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_blindness);
  
  SPELLO(7,12,POSITION_STANDING, 12, 12, 5,
   TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_FIGHT_VICT | TAR_VIOLENT,
   cast_charm_person);
  
  SPELLO(9,12,POSITION_STANDING,15, LOW_IMMORTAL,LOW_IMMORTAL,    
         TAR_CHAR_ROOM, cast_clone);
  
  SPELLO(11,36,POSITION_STANDING,10, 13, 25,
   TAR_IGNORE, cast_control_weather);
  
  SPELLO(12,12,POSITION_STANDING,LOW_IMMORTAL,  3, 5,
   TAR_IGNORE, cast_create_food);
  
  SPELLO(13,12,POSITION_STANDING,LOW_IMMORTAL,  2, 5,
   TAR_OBJ_INV | TAR_OBJ_EQUIP, cast_create_water);
  
  SPELLO(14,12,POSITION_STANDING,LOW_IMMORTAL,  4, 5,
   TAR_CHAR_ROOM, cast_cure_blind);
  
  SPELLO(15,12,POSITION_FIGHTING,LOW_IMMORTAL,  9, 11,
   TAR_CHAR_ROOM, cast_cure_critic);
  
  SPELLO(16,12,POSITION_FIGHTING,LOW_IMMORTAL,  1, 5,
   TAR_CHAR_ROOM, cast_cure_light);
  
  SPELLO(17,24,POSITION_STANDING,12, 12, 20,
   TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_FIGHT_VICT | TAR_VIOLENT, cast_curse);
  
  SPELLO(18,12,POSITION_STANDING, LOW_IMMORTAL, 1, 5,
   TAR_CHAR_ROOM, cast_detect_evil);
  
  SPELLO(19,12,POSITION_STANDING, 2,  5, 5,
   TAR_CHAR_ROOM, cast_detect_invisibility);
  
  SPELLO(20,12,POSITION_STANDING, 1,  3, 5,
   TAR_CHAR_ROOM, cast_detect_magic);
  
  SPELLO(21,12,POSITION_STANDING,LOW_IMMORTAL,  2, 5,
   TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP, cast_detect_poison);
  
  SPELLO(24,48,POSITION_STANDING,9, 25, 100,
   TAR_OBJ_INV | TAR_OBJ_EQUIP, cast_enchant_weapon);
  
  SPELLO(28,18,POSITION_FIGHTING,LOW_IMMORTAL,17,50,TAR_CHAR_ROOM,cast_heal);
  
  SPELLO(29,12,POSITION_STANDING, 4, LOW_IMMORTAL, 5,
   TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM | TAR_OBJ_EQUIP, cast_invisibility);
  
  SPELLO(31,12,POSITION_STANDING, LOW_IMMORTAL, 4, 20,
   TAR_NAME, cast_locate_object);
  
  SPELLO(33,24,POSITION_FIGHTING,LOW_IMMORTAL,  8, 10,
   TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_FIGHT_VICT | TAR_VIOLENT, cast_poison);
  
  SPELLO(34,12,POSITION_STANDING,LOW_IMMORTAL,  6, 5,
   TAR_CHAR_ROOM, cast_protection_from_evil);
  
  SPELLO(35,12,POSITION_STANDING,LOW_IMMORTAL, 7, 5,
   TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_OBJ_ROOM, cast_remove_curse);
  
  SPELLO(36,36,POSITION_STANDING, LOW_IMMORTAL, 19, 50,
   TAR_CHAR_ROOM, cast_sanctuary);
  
  SPELLO(38,24,POSITION_STANDING, 3, LOW_IMMORTAL, 15,
   TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_sleep);
  
  SPELLO(39,12,POSITION_STANDING, 4, LOW_IMMORTAL, 10,
   TAR_CHAR_ROOM, cast_strength);
  
  SPELLO(40,36,POSITION_STANDING, 18,  16, 20,
   TAR_CHAR_WORLD, cast_summon);
  
  SPELLO(41,12,POSITION_STANDING, 1, LOW_IMMORTAL, 5,
         TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_SELF_NONO, cast_ventriloquate);
  
  SPELLO(42,12,POSITION_FIGHTING,LOW_IMMORTAL,10,5,
   TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_word_of_recall);
  
  SPELLO(43,12,POSITION_STANDING,LOW_IMMORTAL,5,5,
   TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, cast_remove_poison);
  
  SPELLO(44,12,POSITION_STANDING,LOW_IMMORTAL,  7, 5,
   TAR_CHAR_ROOM, cast_sense_life);
  
  SPELLO(45,0,POSITION_STANDING,IMPLEMENTOR,IMPLEMENTOR,200,
   TAR_IGNORE, 0);
  SPELLO(46,0,POSITION_STANDING,IMPLEMENTOR,IMPLEMENTOR,200,
   TAR_IGNORE, 0);
  SPELLO(47,0,POSITION_STANDING,IMPLEMENTOR,IMPLEMENTOR,200,
   TAR_IGNORE, 0);
  SPELLO(48,0,POSITION_STANDING,IMPLEMENTOR+1,IMPLEMENTOR+1,200,
   TAR_IGNORE, 0);
  SPELLO(49,0,POSITION_STANDING,IMPLEMENTOR+1,IMPLEMENTOR+1,200,
   TAR_IGNORE, 0);
  SPELLO(50,0,POSITION_STANDING,IMPLEMENTOR+1,IMPLEMENTOR+1,200,
   TAR_IGNORE, 0);
  SPELLO(51,0,POSITION_STANDING,IMPLEMENTOR+1,IMPLEMENTOR+1,200,
   TAR_IGNORE, 0);
  SPELLO(52,0,POSITION_STANDING,IMPLEMENTOR+1,IMPLEMENTOR+1,200,
   TAR_IGNORE, 0);
  
  SPELLO(53,1,POSITION_STANDING,IMPLEMENTOR+1,IMPLEMENTOR+1, 100, TAR_IGNORE, cast_identify);
  
  SPELLO(54,12,POSITION_STANDING, 5, LOW_IMMORTAL, 7,
   TAR_CHAR_ROOM, cast_infravision); 
  
  SPELLO(55,12,POSITION_FIGHTING, LOW_IMMORTAL,1,8,
   TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_cause_light);
  
  SPELLO(56,18,POSITION_FIGHTING, LOW_IMMORTAL,9,11,
   TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT , cast_cause_critic);
  
  SPELLO(57,24,POSITION_FIGHTING, LOW_IMMORTAL,11,15,
   TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_flamestrike);
  
  SPELLO(58,36,POSITION_FIGHTING, LOW_IMMORTAL, 12, 15,
   TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_dispel_good);
  
  SPELLO(59,12,POSITION_FIGHTING, 4, LOW_IMMORTAL, 10,
   TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_weakness);
  
  SPELLO(60,12,POSITION_FIGHTING, 6, 6, 15,
   TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_dispel_magic);
  
  SPELLO(61,12,POSITION_STANDING, 3, LOW_IMMORTAL, 10,
   TAR_IGNORE, cast_knock);
  
  SPELLO(62,12,POSITION_FIGHTING, 4, 3, 10,
   TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_know_alignment);
  
  SPELLO(63,24,POSITION_STANDING, 10, 7, 15,
   TAR_OBJ_ROOM, cast_animate_dead);
  
  SPELLO(64,36,POSITION_FIGHTING, 15, 15, 40,
   TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_paralyze);
  
  SPELLO(65,12,POSITION_FIGHTING, LOW_IMMORTAL, 4, 10,
   TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_remove_paralysis);
  
  SPELLO( 66,12,POSITION_FIGHTING,8,LOW_IMMORTAL,15,
   TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_fear);
  
  SPELLO(67,24,POSITION_FIGHTING, 7, LOW_IMMORTAL, 15,
   TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_acid_blast);
  
  SPELLO(68,12,POSITION_FIGHTING, 4, LOW_IMMORTAL, 15,
   TAR_CHAR_ROOM, cast_water_breath);
  
  SPELLO(69,12,POSITION_FIGHTING, 3, 14, 15,
   TAR_CHAR_ROOM,cast_flying);
  
  SPELLO(70,24,POSITION_FIGHTING, 11, LOW_IMMORTAL, 15,
   TAR_IGNORE | TAR_VIOLENT,  cast_cone_of_cold);
  
  SPELLO(71,24,POSITION_FIGHTING, 20, LOW_IMMORTAL, 50,
   TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_meteor_swarm);
  
  SPELLO(72,12,POSITION_FIGHTING, 7, LOW_IMMORTAL, 15,
   TAR_IGNORE | TAR_VIOLENT,  cast_ice_storm);
  
  SPELLO(73,24,POSITION_FIGHTING,1,15,15,
   TAR_CHAR_ROOM, cast_shield);
  
  SPELLO(74,24,POSITION_FIGHTING, 5, LOW_IMMORTAL, 10,
   TAR_IGNORE, cast_mon_sum1);
  
  SPELLO(75,24,POSITION_FIGHTING, 7, LOW_IMMORTAL, 12,
   TAR_IGNORE, cast_mon_sum2);
  
  SPELLO(76,24,POSITION_FIGHTING, 9, LOW_IMMORTAL, 15,
   TAR_IGNORE, cast_mon_sum3);
  
  SPELLO(77,24,POSITION_FIGHTING, 11, LOW_IMMORTAL, 17,
   TAR_IGNORE, cast_mon_sum4);
  
  SPELLO(78,24,POSITION_FIGHTING, 13, LOW_IMMORTAL, 20,
   TAR_IGNORE, cast_mon_sum5);
  
  SPELLO(79,24,POSITION_FIGHTING, 15, LOW_IMMORTAL, 22,
   TAR_IGNORE, cast_mon_sum6);
  
  SPELLO(80,24,POSITION_STANDING, 17, LOW_IMMORTAL, 25,
   TAR_IGNORE, cast_mon_sum7);
  
  SPELLO(81,24,POSITION_STANDING, 20, 19, 40,
   TAR_SELF_ONLY | TAR_CHAR_ROOM, cast_fireshield);
  
  SPELLO(82,18,POSITION_STANDING, 8,8,5,
   TAR_CHAR_ROOM | TAR_VIOLENT, cast_charm_monster);
  
  SPELLO(83,12,POSITION_FIGHTING, 30, 7, 9,
   TAR_CHAR_ROOM, cast_cure_serious);
  
  SPELLO(84,12,POSITION_FIGHTING, 30, 7, 9,
   TAR_CHAR_ROOM | TAR_VIOLENT, cast_cause_serious);
  
  SPELLO(85,12,POSITION_STANDING, 3, 2, 5,
   TAR_CHAR_ROOM, cast_refresh);
  
  SPELLO(86,12,POSITION_STANDING, 12, 6, 5,
   TAR_CHAR_ROOM, cast_second_wind);
  
  SPELLO(87,12,POSITION_STANDING, LOW_IMMORTAL, 1, 5,
   TAR_CHAR_ROOM, cast_turn);
  
  SPELLO(88,24,POSITION_STANDING,21,18,15,TAR_IGNORE, cast_succor);
  
  SPELLO(89,12,POSITION_STANDING, 1, 2, 5,
   TAR_IGNORE, cast_light);
  
  SPELLO(90,24,POSITION_STANDING, 3, 4, 10,
   TAR_IGNORE, cast_cont_light);
  
  SPELLO(91,24,POSITION_STANDING, 4, 2, 15,
   TAR_CHAR_ROOM, cast_calm);
  
  SPELLO(92,24,POSITION_STANDING,16,32,20,
   TAR_SELF_ONLY, cast_stone_skin);
  
  SPELLO(93,24,POSITION_STANDING,16,14,30,
   TAR_IGNORE, cast_conjure_elemental);
  
  SPELLO(94,24,POSITION_STANDING, LOW_IMMORTAL, 12, 20,
   TAR_CHAR_ROOM, cast_true_seeing);
  
  SPELLO(95,24,POSITION_STANDING,8,14,30,
   TAR_IGNORE, cast_minor_creation);
  
  SPELLO(96,12,POSITION_STANDING, 5, 3, 10,
   TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT, cast_faerie_fire);
  
  SPELLO(97,24,POSITION_STANDING, 13, 10, 20,
   TAR_IGNORE, cast_faerie_fog);
  
  SPELLO(98,24,POSITION_STANDING, 30, 30, 50,
   TAR_IGNORE, cast_cacaodemon);
  
  SPELLO(99,12,POSITION_FIGHTING, 8, LOW_IMMORTAL, 30,
   TAR_IGNORE, cast_poly_self ); 
  
  
  SPELLO(100,12,POSITION_FIGHTING, IMPLEMENTOR+1, IMPLEMENTOR+1, 
   200, TAR_IGNORE, 0 ); /* spell_mana */
  
  SPELLO( 101,12,POSITION_STANDING, 21, 18, 33,
   TAR_CHAR_WORLD, cast_astral_walk);
  
  SPELLO(102,18,POSITION_FIGHTING, 8, 22, 30,
   TAR_IGNORE,cast_fly_group);

  SPELLO(103,12,POSITION_STANDING, LOW_IMMORTAL, 10, 15,
   TAR_CHAR_ROOM,cast_aid);

  SPELLO(104,12,POSITION_STANDING, 10, 10, 100,
   TAR_IGNORE,cast_shelter);

  SPELLO( 102,12,POSITION_STANDING, 50, 21, 33,
   TAR_OBJ_ROOM, 0 );
  
  SPELLO(SPELL_DRAGON_BREATH, 0, POSITION_STANDING, IMPLEMENTOR+1,
   IMPLEMENTOR+1, 200, TAR_IGNORE | TAR_VIOLENT, cast_dragon_breath);
  
  
  SPELLO(180,0,POSITION_STANDING,IMPLEMENTOR+1,IMPLEMENTOR+1,200, 
   TAR_IGNORE, 0);
  
}


int check_falling( struct char_data *ch)
{
  struct room_data *rp, *targ;
  int done, count, saved;
  char buf[256];

  if (IS_AFFECTED(ch, AFF_FLYING))
    return(FALSE);

  rp = real_roomp(ch->in_room);
  if (!rp) return(FALSE);

  if (rp->sector_type != SECT_AIR) 
    return(FALSE);

    act("The world spins, and you plummet out of control",
	TRUE, ch, 0, 0, TO_CHAR);
    saved = FALSE;

  done = FALSE;
  count = 0;

  while (!done && count < 100) {

/*
  check for an exit down.
  if there is one, go through it.
*/
    if (rp->dir_option[DOWN] && rp->dir_option[DOWN]->to_room > -1) {
      targ = real_roomp(rp->dir_option[DOWN]->to_room);
    } else {
      /*
	pretend that this is the smash room.
	*/
      if (count > 1) {

	send_to_char("You are smashed into tiny pieces.\n\r", ch);
	act("$n smashes against the ground at high speed", 
	    FALSE, ch, 0, 0, TO_ROOM);
	act("You are drenched with blood and gore", 
	    FALSE,ch, 0, 0, TO_ROOM);

/*
  should damage all their stuff
*/
	DamageAllStuff(ch, BLOW_DAMAGE);

	if (!IS_IMMORTAL(ch)) {
	  GET_HIT(ch) = 0;
	  sprintf(buf, "%s has fallen to death", GET_NAME(ch));
	  log(buf);
	if (!ch->desc)
	  GET_GOLD(ch) = 0;
	  die(ch);
	}
	return(TRUE);
	
      } else {

	send_to_char("You land with a resounding THUMP!\n\r", ch);
	GET_HIT(ch) = 0;
	GET_POS(ch) = POSITION_STUNNED;
	act("$n lands with a resounding THUMP!", FALSE, ch, 0, 0, TO_ROOM);
/*
  should damage all their stuff
*/
	DamageAllStuff(ch, BLOW_DAMAGE);

	return(TRUE);

      }
    }

    act("$n plunges towards oblivion", FALSE, ch, 0, 0, TO_ROOM);
    send_to_char("You plunge from the sky\n\r", ch);
    char_from_room(ch);
    char_to_room(ch, rp->dir_option[DOWN]->to_room);
    act("$n falls from the sky", FALSE, ch, 0, 0, TO_ROOM);
    count++;

    do_look(ch, "", 0);

    if (targ->sector_type != SECT_AIR) {
      /* do damage, or kill */
      if (count == 1) {
	send_to_char("You land with a resounding THUMP!\n\r", ch);
	GET_HIT(ch) = 0;
	GET_POS(ch) = POSITION_STUNNED;
	act("$n lands with a resounding THUMP!", FALSE, ch, 0, 0, TO_ROOM);
/*
  should damage all their stuff
*/
	DamageAllStuff(ch, BLOW_DAMAGE);

	return(TRUE);

      } else if (!saved) {
        send_to_char("You are smashed into tiny pieces.\n\r", ch);
        if (targ->sector_type >= SECT_WATER_SWIM)
	  act("$n is smashed to a pulp by $s impact with the water", 
	    FALSE, ch, 0, 0, TO_ROOM);
        else 
	  act("$n is smashed to a bloody pulp by $s impact with the ground", 
	    FALSE, ch, 0, 0, TO_ROOM);
	act("You are drenched with blood and gore", FALSE,ch, 0, 0, TO_ROOM);

/*
  should damage all their stuff
*/
	DamageAllStuff(ch, BLOW_DAMAGE);

	if (!IS_IMMORTAL(ch)) {
	  GET_HIT(ch) = 0;
	  sprintf(buf, "%s has fallen to death", GET_NAME(ch));
	  log(buf);
	if (!ch->desc)
	  GET_GOLD(ch) = 0;
	  die(ch);
	}
	return(TRUE);

      } else {
	send_to_char("You land with a resounding THUMP!\n\r", ch);
	GET_HIT(ch) = 0;
	GET_POS(ch) = POSITION_STUNNED;
	act("$n lands with a resounding THUMP!", FALSE, ch, 0, 0, TO_ROOM);
/*
  should damage all their stuff
*/
	DamageAllStuff(ch, BLOW_DAMAGE);

	return(TRUE);
	
      }
    } else {
/*
  time to try the next room
*/
      rp = targ;
      targ = 0;
    }
  }  

  if (count >= 100) {
    log("Someone fucked up an air room.");
    char_from_room(ch);
    char_to_room(ch, GET_HOME(ch));
    do_look(ch, "", 0);
    return(FALSE);
  }
}

void check_drowning( struct char_data *ch)
{
  struct room_data *rp;
  char buf[256];

  if (IS_AFFECTED(ch, AFF_WATERBREATH))
    return;

  rp = real_roomp(ch->in_room);

  if (!rp) return;

  if (rp->sector_type == SECT_UNDERWATER) {
      send_to_char("PANIC!  You're drowning!!!!!!", ch);
      GET_HIT(ch)-=number(1,30);
      GET_MOVE(ch) -= number(10,50);
      update_pos(ch);
      if (GET_HIT(ch) < -10) {
	sprintf(buf, "%s killed by drowning", GET_NAME(ch));
	log(buf);
	if (!ch->desc)
	  GET_GOLD(ch) = 0;
	die(ch);
      }
   }
}


void check_falling_obj( struct obj_data *obj, int room)
{
  struct room_data *rp, *targ;
  int done, count;

  if (obj->in_room != room) {
    log("unusual object information in check_falling_obj");
    return;
  }

  rp = real_roomp(room);
  if (!rp) return;

  if (rp->sector_type != SECT_AIR) 
    return;

  done = FALSE;
  count = 0;

  while (!done && count < 100) {

    if (rp->dir_option[DOWN] && rp->dir_option[DOWN]->to_room > -1) {
      targ = real_roomp(rp->dir_option[DOWN]->to_room);
    } else {
      /*
	pretend that this is the smash room.
	*/
      if (count > 1) {

	if (rp->people) {
	  act("$p smashes against the ground at high speed", 
	      FALSE, rp->people, obj, 0, TO_ROOM);
	  act("$p smashes against the ground at high speed", 
	      FALSE, rp->people, obj, 0, TO_CHAR);
	}
	return;

      } else {

	if (rp->people) {
	  act("$p lands with a loud THUMP!", 
	      FALSE, rp->people, obj, 0, TO_ROOM);
	  act("$p lands with a loud THUMP!", 
	      FALSE, rp->people, obj, 0, TO_CHAR);
	}
	return;

      }
    }

    if (rp->people) { /* have to reference a person */
      act("$p falls out of sight", FALSE, rp->people, obj, 0, TO_ROOM);
      act("$p falls out of sight", FALSE, rp->people, obj, 0, TO_CHAR);
    }
    obj_from_room(obj);
    obj_to_room(obj, rp->dir_option[DOWN]->to_room);
    if (targ->people) {
      act("$p falls from the sky", FALSE, targ->people, obj, 0, TO_ROOM);
      act("$p falls from the sky", FALSE, targ->people, obj, 0, TO_CHAR);
    }
    count++;

    if (targ->sector_type != SECT_AIR) {
      if (count == 1) {
	if (targ->people) {
	  act("$p lands with a loud THUMP!", FALSE, 0, obj, 0, TO_ROOM);
	  act("$p lands with a loud THUMP!", FALSE, 0, obj, 0, TO_CHAR);
	}
	return;
      } else {
	if (targ->people) {
	  if (targ->sector_type >= SECT_WATER_SWIM){	  
	    act("$p smashes against the water at high speed", 
		FALSE, targ->people, obj, 0, TO_ROOM);
	    act("$p smashes against the water at high speed", 
		FALSE, targ->people, obj, 0, TO_CHAR);
	  } else {
	    act("$p smashes against the ground at high speed", 
		FALSE, targ->people, obj, 0, TO_ROOM);
	    act("$p smashes against the ground at high speed", 
		FALSE, targ->people, obj, 0, TO_CHAR);
	  }
	}
	return;

      }
    } else {
/*
  time to try the next room
*/
      rp = targ;
      targ = 0;
    }
  }  

  if (count >= 100) {
    log("Someone fucked up an air room.");
    obj_from_room(obj);
    obj_to_room(obj, 2);
    return;
  }
}

int check_nature( struct char_data *i)
{

  if (check_falling(i)) {
    return(TRUE);
  }
  check_drowning(i);

}

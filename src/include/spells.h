/*
 * file: spells.h , Implementation of magic spells.       Part of DIKUMUD
 * Usage : Spells
 */

#ifndef _SPELLS_H
#define _SPELLS_H

#define MAX_BUF_LENGTH              240
#define STATE(d) ((d)->connected)
#define IS_IMMUNE(ch, bit) (IS_SET((ch)->M_immune, bit))

#define SPELL_TYPE_SPELL   0
#define SPELL_TYPE_POTION  1
#define SPELL_TYPE_WAND    2
#define SPELL_TYPE_STAFF   3
#define SPELL_TYPE_SCROLL  4

#define SAVING_PARA   0
#define SAVING_ROD    1
#define SAVING_PETRI  2
#define SAVING_BREATH 3
#define SAVING_SPELL  4

/* Attacktypes with grammar */

struct attack_hit_type {
  const char                                   *singular;
  const char                                   *plural;
};

#define TAR_IGNORE	 (1<< 0)
#define TAR_CHAR_ROOM	 (1<< 1)
#define TAR_CHAR_WORLD	 (1<< 2)
#define TAR_FIGHT_SELF	 (1<< 3)
#define TAR_FIGHT_VICT	 (1<< 4)
#define TAR_SELF_ONLY	 (1<< 5)			       /* Only a check, use with ei. TAR_CHAR_ROOM */
#define TAR_SELF_NONO	 (1<< 6)			       /* Only a check, use with ei. TAR_CHAR_ROOM */
#define TAR_OBJ_INV	 (1<< 7)
#define TAR_OBJ_ROOM	 (1<< 8)
#define TAR_OBJ_WORLD	 (1<< 9)
#define TAR_OBJ_EQUIP	 (1<<10)
#define TAR_NAME	 (1<<11)
#define TAR_VIOLENT	 (1<<12)
#define TAR_ROOM         (1<<13)			       /* spells which target the room */

struct spell_info_type {
  char                                    castable;	       /* Is it a castable spell? */
  char                                    useable;	       /* Is it a useable skill? */
  const char                             *name;		       /* Text name of spell */
  void                                    (*spell_pointer) (int level, struct char_data * ch,
							    const char *arg, int type,
							    struct char_data * tar_ch,
							    struct obj_data * tar_obj);
  char                                    delay;	       /* Heartbeats until ready for next */
  unsigned char                           min_mana;	       /* Amount of mana used by a spell */
  unsigned char                           max_mana;
  char                                    minimum_position;    /* Position for caster */
  short int                               targets;	       /* See below for use with TAR_XXX */
  char                                    generic_level;       /* Level for generic teachings */
  int                                     generic_classes;     /* Classes allowed to learn generic skills */
  char                                    min_level[ABS_MAX_LVL];	/* Level required for the various classes */
};

/* Possible Targets:
 * 
 * bit 0 : IGNORE TARGET
 * bit 1 : PC/NPC in room
 * bit 2 : PC/NPC in world
 * bit 3 : Object held
 * bit 4 : Object in inventory
 * bit 5 : Object in room
 * bit 6 : Object in world
 * bit 7 : If fighting, and no argument, select tar_char as self
 * bit 8 : If fighting, and no argument, select tar_char as victim (fighting)
 * bit 9 : If no argument, select self, if argument check that it IS self.
 */

/*
 *  just for polymorph spell(s)
 */

struct PolyType {
  char                                    name[20];
  int                                     level;
  int                                     number;
};

#define TYPE_UNDEFINED               -1
#define SPELL_RESERVED_DBC            0			       /* SKILL NUMBER ZERO */
#define SPELL_ARMOR                   1
void                                    spell_armor(int level, struct char_data *ch,
						    struct char_data *victim,
						    struct obj_data *obj);
void                                    cast_armor(int level, struct char_data *ch, const char *arg,
						   int type, struct char_data *tar_ch,
						   struct obj_data *tar_obj);

#define SPELL_TELEPORT                2
void                                    spell_teleport(int level, struct char_data *ch,
						       struct char_data *victim,
						       struct obj_data *obj);
void                                    cast_teleport(int level, struct char_data *ch,
						      const char *arg, int type,
						      struct char_data *tar_ch,
						      struct obj_data *tar_obj);

#define SPELL_BLESS                   3
void                                    spell_bless(int level, struct char_data *ch,
						    struct char_data *victim,
						    struct obj_data *obj);
void                                    cast_bless(int level, struct char_data *ch, const char *arg,
						   int type, struct char_data *tar_ch,
						   struct obj_data *tar_obj);

#define SPELL_BLINDNESS               4
void                                    spell_blindness(int level, struct char_data *ch,
							struct char_data *victim,
							struct obj_data *obj);
void                                    cast_blindness(int level, struct char_data *ch,
						       const char *arg, int type,
						       struct char_data *tar_ch,
						       struct obj_data *tar_obj);

#define SPELL_BURNING_HANDS           5
void                                    spell_burning_hands(int level, struct char_data *ch,
							    struct char_data *victim,
							    struct obj_data *obj);
void                                    cast_burning_hands(int level, struct char_data *ch,
							   const char *arg, int type,
							   struct char_data *victim,
							   struct obj_data *tar_obj);

#define SPELL_CALL_LIGHTNING          6
void                                    spell_call_lightning(int level, struct char_data *ch,
							     struct char_data *victim,
							     struct obj_data *obj);
void                                    cast_call_lightning(int level, struct char_data *ch,
							    const char *arg, int type,
							    struct char_data *victim,
							    struct obj_data *tar_obj);

#define SPELL_CHARM_PERSON            7
void                                    spell_charm_person(int level, struct char_data *ch,
							   struct char_data *victim,
							   struct obj_data *obj);
void                                    cast_charm_person(int level, struct char_data *ch,
							  const char *arg, int type,
							  struct char_data *tar_ch,
							  struct obj_data *tar_obj);

#define SPELL_CHILL_TOUCH             8
void                                    spell_chill_touch(int level, struct char_data *ch,
							  struct char_data *victim,
							  struct obj_data *obj);
void                                    cast_chill_touch(int level, struct char_data *ch,
							 const char *arg, int type,
							 struct char_data *victim,
							 struct obj_data *tar_obj);

#define SPELL_CLONE                   9
void                                    spell_clone(int level, struct char_data *ch,
						    struct char_data *victim,
						    struct obj_data *obj);
void                                    cast_clone(int level, struct char_data *ch, const char *arg,
						   int type, struct char_data *tar_ch,
						   struct obj_data *tar_obj);

#define SPELL_COLOUR_SPRAY           10
void                                    spell_colour_spray(int level, struct char_data *ch,
							   struct char_data *victim,
							   struct obj_data *obj);
void                                    cast_colour_spray(int level, struct char_data *ch,
							  const char *arg, int type,
							  struct char_data *victim,
							  struct obj_data *tar_obj);

#define SPELL_CONTROL_WEATHER        11
void                                    spell_control_weather(int level, struct char_data *ch,
							      struct char_data *victim,
							      struct obj_data *obj);
void                                    cast_control_weather(int level, struct char_data *ch,
							     const char *arg, int type,
							     struct char_data *tar_ch,
							     struct obj_data *tar_obj);

#define SPELL_CREATE_FOOD            12
void                                    spell_create_food(int level, struct char_data *ch,
							  struct char_data *victim,
							  struct obj_data *obj);
void                                    cast_create_food(int level, struct char_data *ch,
							 const char *arg, int type,
							 struct char_data *tar_ch,
							 struct obj_data *tar_obj);

#define SPELL_CREATE_WATER           13
void                                    spell_create_water(int level, struct char_data *ch,
							   struct char_data *victim,
							   struct obj_data *obj);
void                                    cast_create_water(int level, struct char_data *ch,
							  const char *arg, int type,
							  struct char_data *tar_ch,
							  struct obj_data *tar_obj);

#define SPELL_CURE_BLIND             14
void                                    spell_cure_blind(int level, struct char_data *ch,
							 struct char_data *victim,
							 struct obj_data *obj);
void                                    cast_cure_blind(int level, struct char_data *ch,
							const char *arg, int type,
							struct char_data *tar_ch,
							struct obj_data *tar_obj);

#define SPELL_CURE_CRITIC            15
void                                    spell_cure_critic(int level, struct char_data *ch,
							  struct char_data *victim,
							  struct obj_data *obj);
void                                    cast_cure_critic(int level, struct char_data *ch,
							 const char *arg, int type,
							 struct char_data *tar_ch,
							 struct obj_data *tar_obj);

#define SPELL_CURE_LIGHT             16
void                                    spell_cure_light(int level, struct char_data *ch,
							 struct char_data *victim,
							 struct obj_data *obj);
void                                    cast_cure_light(int level, struct char_data *ch,
							const char *arg, int type,
							struct char_data *tar_ch,
							struct obj_data *tar_obj);

#define SPELL_CURSE                  17
void                                    spell_curse(int level, struct char_data *ch,
						    struct char_data *victim,
						    struct obj_data *obj);
void                                    cast_curse(int level, struct char_data *ch, const char *arg,
						   int type, struct char_data *tar_ch,
						   struct obj_data *tar_obj);

#define SPELL_DETECT_EVIL            18
void                                    spell_detect_evil(int level, struct char_data *ch,
							  struct char_data *victim,
							  struct obj_data *obj);
void                                    cast_detect_evil(int level, struct char_data *ch,
							 const char *arg, int type,
							 struct char_data *tar_ch,
							 struct obj_data *tar_obj);

#define SPELL_DETECT_INVISIBLE       19
void                                    spell_detect_invisibility(int level,
								  struct char_data *ch,
								  struct char_data *victim,
								  struct obj_data *obj);
void                                    cast_detect_invisibility(int level,
								 struct char_data *ch,
								 const char *arg, int type,
								 struct char_data *tar_ch,
								 struct obj_data *tar_obj);

#define SPELL_DETECT_MAGIC           20
void                                    spell_detect_magic(int level, struct char_data *ch,
							   struct char_data *victim,
							   struct obj_data *obj);
void                                    cast_detect_magic(int level, struct char_data *ch,
							  const char *arg, int type,
							  struct char_data *tar_ch,
							  struct obj_data *tar_obj);

#define SPELL_DETECT_POISON          21
void                                    spell_detect_poison(int level, struct char_data *ch,
							    struct char_data *victim,
							    struct obj_data *obj);
void                                    cast_detect_poison(int level, struct char_data *ch,
							   const char *arg, int type,
							   struct char_data *tar_ch,
							   struct obj_data *tar_obj);

#define SPELL_DISPEL_EVIL            22
void                                    spell_dispel_evil(int level, struct char_data *ch,
							  struct char_data *victim,
							  struct obj_data *obj);
void                                    cast_dispel_evil(int level, struct char_data *ch,
							 const char *arg, int type,
							 struct char_data *tar_ch,
							 struct obj_data *tar_obj);

#define SPELL_EARTHQUAKE             23
void                                    spell_earthquake(int level, struct char_data *ch,
							 struct char_data *victim,
							 struct obj_data *obj);
void                                    cast_earthquake(int level, struct char_data *ch,
							const char *arg, int type,
							struct char_data *victim,
							struct obj_data *tar_obj);
void                                    spell_new_earthquake(int level, struct char_data *ch,
							     struct char_data *victim,
							     struct obj_data *obj);
void                                    cast_new_earthquake(int level, struct char_data *ch,
							    const char *arg, int type,
							    struct char_data *victim,
							    struct obj_data *tar_obj);

#define SPELL_ENCHANT_WEAPON         24
void                                    spell_enchant_weapon(int level, struct char_data *ch,
							     struct char_data *victim,
							     struct obj_data *obj);
void                                    cast_enchant_weapon(int level, struct char_data *ch,
							    const char *arg, int type,
							    struct char_data *tar_ch,
							    struct obj_data *tar_obj);
void                                    cast_enchant_armor(int level, struct char_data *ch,
							   const char *arg, int type,
							   struct char_data *tar_ch,
							   struct obj_data *tar_obj);

#define SPELL_ENERGY_DRAIN           25
void                                    spell_energy_drain(int level, struct char_data *ch,
							   struct char_data *victim,
							   struct obj_data *obj);
void                                    cast_energy_drain(int level, struct char_data *ch,
							  const char *arg, int type,
							  struct char_data *victim,
							  struct obj_data *tar_obj);

#define SPELL_FIREBALL               26
void                                    spell_fireball(int level, struct char_data *ch,
						       struct char_data *victim,
						       struct obj_data *obj);
void                                    cast_fireball(int level, struct char_data *ch,
						      const char *arg, int type,
						      struct char_data *victim,
						      struct obj_data *tar_obj);

#define SPELL_HARM                   27
void                                    spell_harm(int level, struct char_data *ch,
						   struct char_data *victim,
						   struct obj_data *obj);
void                                    cast_harm(int level, struct char_data *ch, const char *arg,
						  int type, struct char_data *victim,
						  struct obj_data *tar_obj);

#define SPELL_HEAL                   28
void                                    spell_heal(int level, struct char_data *ch,
						   struct char_data *victim,
						   struct obj_data *obj);
void                                    cast_heal(int level, struct char_data *ch, const char *arg,
						  int type, struct char_data *tar_ch,
						  struct obj_data *tar_obj);

#define SPELL_INVISIBLE              29
void                                    spell_invisibility(int level, struct char_data *ch,
							   struct char_data *victim,
							   struct obj_data *obj);
void                                    cast_invisibility(int level, struct char_data *ch,
							  const char *arg, int type,
							  struct char_data *tar_ch,
							  struct obj_data *tar_obj);

#define SPELL_LIGHTNING_BOLT         30
void                                    spell_lightning_bolt(int level, struct char_data *ch,
							     struct char_data *victim,
							     struct obj_data *obj);
void                                    cast_lightning_bolt(int level, struct char_data *ch,
							    const char *arg, int type,
							    struct char_data *victim,
							    struct obj_data *tar_obj);

#define SPELL_LOCATE_OBJECT          31
void                                    spell_locate_object(int level, struct char_data *ch,
							    struct char_data *victim,
							    char *obj);
void                                    cast_locate_object(int level, struct char_data *ch,
							   const char *arg, int type,
							   struct char_data *tar_ch,
							   struct obj_data *tar_obj);

#define SPELL_MAGIC_MISSILE          32
void                                    spell_magic_missile(int level, struct char_data *ch,
							    struct char_data *victim,
							    struct obj_data *obj);
void                                    cast_magic_missile(int level, struct char_data *ch,
							   const char *arg, int type,
							   struct char_data *victim,
							   struct obj_data *tar_obj);

#define SPELL_POISON                 33
void                                    spell_poison(int level, struct char_data *ch,
						     struct char_data *victim,
						     struct obj_data *obj);
void                                    cast_poison(int level, struct char_data *ch, const char *arg,
						    int type, struct char_data *tar_ch,
						    struct obj_data *tar_obj);

#define SPELL_PROTECT_FROM_EVIL      34
void                                    spell_protection_from_evil(int level,
								   struct char_data *ch,
								   struct char_data *victim,
								   struct obj_data *obj);
void                                    cast_protection_from_evil(int level,
								  struct char_data *ch,
								  const char *arg, int type,
								  struct char_data *tar_ch,
								  struct obj_data *tar_obj);

#define SPELL_REMOVE_CURSE           35
void                                    spell_remove_curse(int level, struct char_data *ch,
							   struct char_data *victim,
							   struct obj_data *obj);
void                                    cast_remove_curse(int level, struct char_data *ch,
							  const char *arg, int type,
							  struct char_data *tar_ch,
							  struct obj_data *tar_obj);

#define SPELL_SANCTUARY              36
void                                    spell_sanctuary(int level, struct char_data *ch,
							struct char_data *victim,
							struct obj_data *obj);
void                                    cast_sanctuary(int level, struct char_data *ch,
						       const char *arg, int type,
						       struct char_data *tar_ch,
						       struct obj_data *tar_obj);

#define SPELL_SHOCKING_GRASP         37
void                                    spell_shocking_grasp(int level, struct char_data *ch,
							     struct char_data *victim,
							     struct obj_data *obj);
void                                    cast_shocking_grasp(int level, struct char_data *ch,
							    const char *arg, int type,
							    struct char_data *victim,
							    struct obj_data *tar_obj);

#define SPELL_SLEEP                  38
void                                    spell_sleep(int level, struct char_data *ch,
						    struct char_data *victim,
						    struct obj_data *obj);
void                                    cast_sleep(int level, struct char_data *ch, const char *arg,
						   int type, struct char_data *tar_ch,
						   struct obj_data *tar_obj);

#define SPELL_STRENGTH               39
void                                    spell_strength(int level, struct char_data *ch,
						       struct char_data *victim,
						       struct obj_data *obj);
void                                    cast_strength(int level, struct char_data *ch,
						      const char *arg, int type,
						      struct char_data *tar_ch,
						      struct obj_data *tar_obj);

#define SPELL_SUMMON                 40
void                                    spell_summon(int level, struct char_data *ch,
						     struct char_data *victim,
						     struct obj_data *obj);
void                                    cast_summon(int level, struct char_data *ch, const char *arg,
						    int type, struct char_data *tar_ch,
						    struct obj_data *tar_obj);

#define SPELL_VENTRILOQUATE          41
void                                    spell_ventriloquate(int level, struct char_data *ch,
							    struct char_data *victim,
							    struct obj_data *obj);
void                                    cast_ventriloquate(int level, struct char_data *ch,
							   const char *arg, int type,
							   struct char_data *tar_ch,
							   struct obj_data *tar_obj);

#define SPELL_WORD_OF_RECALL         42
void                                    spell_word_of_recall(int level, struct char_data *ch,
							     struct char_data *victim,
							     struct obj_data *obj);
void                                    cast_word_of_recall(int level, struct char_data *ch,
							    const char *arg, int type,
							    struct char_data *tar_ch,
							    struct obj_data *tar_obj);

#define SPELL_REMOVE_POISON          43
void                                    spell_remove_poison(int level, struct char_data *ch,
							    struct char_data *victim,
							    struct obj_data *obj);
void                                    cast_remove_poison(int level, struct char_data *ch,
							   const char *arg, int type,
							   struct char_data *tar_ch,
							   struct obj_data *tar_obj);

#define SPELL_SENSE_LIFE             44
void                                    spell_sense_life(int level, struct char_data *ch,
							 struct char_data *victim,
							 struct obj_data *obj);
void                                    cast_sense_life(int level, struct char_data *ch,
							const char *arg, int type,
							struct char_data *tar_ch,
							struct obj_data *tar_obj);

/* Skills 45 - 52 already in use! */
/* Skills 45 - 52 already in use! */
/* Skills 45 - 52 already in use! */
/* Skills 45 - 52 already in use! */
/* Skills 45 - 52 already in use! */
/* Skills 45 - 52 already in use! */
/* Skills 45 - 52 already in use! */
/* Skills 45 - 52 already in use! */
#define SPELL_IDENTIFY               53
void                                    spell_improved_identify(int level,
								struct char_data *ch,
								struct char_data *victim,
								struct obj_data *obj);
void                                    spell_identify(int level, struct char_data *ch,
						       struct char_data *victim,
						       struct obj_data *obj);
void                                    cast_identify(int level, struct char_data *ch,
						      const char *arg, int type,
						      struct char_data *tar_ch,
						      struct obj_data *tar_obj);

#define SPELL_INFRAVISION            54
void                                    spell_infravision(int level, struct char_data *ch,
							  struct char_data *victim,
							  struct obj_data *obj);
void                                    cast_infravision(int level, struct char_data *ch,
							 const char *arg, int type,
							 struct char_data *tar_ch,
							 struct obj_data *tar_obj);

#define SPELL_CAUSE_LIGHT            55
void                                    spell_cause_light(int level, struct char_data *ch,
							  struct char_data *victim,
							  struct obj_data *obj);
void                                    cast_cause_light(int level, struct char_data *ch,
							 const char *arg, int type,
							 struct char_data *victim,
							 struct obj_data *tar_obj);

#define SPELL_CAUSE_CRITICAL         56
void                                    spell_cause_critical(int level, struct char_data *ch,
							     struct char_data *victim,
							     struct obj_data *obj);
void                                    cast_cause_critic(int level, struct char_data *ch,
							  const char *arg, int type,
							  struct char_data *victim,
							  struct obj_data *tar_obj);

#define SPELL_FLAMESTRIKE            57
void                                    spell_flamestrike(int level, struct char_data *ch,
							  struct char_data *victim,
							  struct obj_data *obj);
void                                    cast_flamestrike(int level, struct char_data *ch,
							 const char *arg, int type,
							 struct char_data *victim,
							 struct obj_data *tar_obj);

#define SPELL_DISPEL_GOOD            58
void                                    spell_dispel_good(int level, struct char_data *ch,
							  struct char_data *victim,
							  struct obj_data *obj);
void                                    cast_dispel_good(int level, struct char_data *ch,
							 const char *arg, int type,
							 struct char_data *tar_ch,
							 struct obj_data *tar_obj);

#define SPELL_WEAKNESS               59
void                                    spell_weakness(int level, struct char_data *ch,
						       struct char_data *victim,
						       struct obj_data *obj);
void                                    cast_weakness(int level, struct char_data *ch,
						      const char *arg, int type,
						      struct char_data *tar_ch,
						      struct obj_data *tar_obj);

#define SPELL_DISPEL_MAGIC           60
void                                    spell_dispel_magic(int level, struct char_data *ch,
							   struct char_data *victim,
							   struct obj_data *obj);
void                                    cast_dispel_magic(int level, struct char_data *ch,
							  const char *arg, int type,
							  struct char_data *tar_ch,
							  struct obj_data *tar_obj);

#define SPELL_KNOCK                  61
void                                    cast_knock(int level, struct char_data *ch, const char *arg,
						   int type, struct char_data *tar_ch,
						   struct obj_data *tar_obj);

#define SPELL_KNOW_ALIGNMENT         62
void                                    spell_know_alignment(int level, struct char_data *ch,
							     struct char_data *victim,
							     struct obj_data *obj);
void                                    cast_know_alignment(int level, struct char_data *ch,
							    const char *arg, int type,
							    struct char_data *tar_ch,
							    struct obj_data *tar_obj);

#define SPELL_ANIMATE_DEAD           63
void                                    spell_animate_dead(int level, struct char_data *ch,
							   struct char_data *victim,
							   struct obj_data *corpse);
void                                    cast_animate_dead(int level, struct char_data *ch,
							  const char *arg, int type,
							  struct char_data *tar_ch,
							  struct obj_data *tar_obj);

#define SPELL_PARALYSIS              64
void                                    spell_paralyze(int level, struct char_data *ch,
						       struct char_data *victim,
						       struct obj_data *obj);
void                                    cast_paralyze(int level, struct char_data *ch,
						      const char *arg, int type,
						      struct char_data *tar_ch,
						      struct obj_data *tar_obj);

#define SPELL_REMOVE_PARALYSIS       65
void                                    spell_remove_paralysis(int level, struct char_data *ch,
							       struct char_data *victim,
							       struct obj_data *obj);
void                                    cast_remove_paralysis(int level, struct char_data *ch,
							      const char *arg, int type,
							      struct char_data *tar_ch,
							      struct obj_data *tar_obj);

#define SPELL_FEAR                   66
void                                    spell_fear(int level, struct char_data *ch,
						   struct char_data *victim,
						   struct obj_data *obj);
void                                    cast_fear(int level, struct char_data *ch, const char *arg,
						  int type, struct char_data *tar_ch,
						  struct obj_data *tar_obj);

#define SPELL_ACID_BLAST             67
void                                    spell_acid_blast(int level, struct char_data *ch,
							 struct char_data *victim,
							 struct obj_data *obj);
void                                    cast_acid_blast(int level, struct char_data *ch,
							const char *arg, int type,
							struct char_data *victim,
							struct obj_data *tar_obj);

#define SPELL_WATER_BREATH           68
void                                    spell_water_breath(int level, struct char_data *ch,
							   struct char_data *victim,
							   struct obj_data *obj);
void                                    cast_water_breath(int level, struct char_data *ch,
							  const char *arg, int type,
							  struct char_data *tar_ch,
							  struct obj_data *tar_obj);

#define SPELL_FLY                    69
void                                    spell_fly(int level, struct char_data *ch,
						  struct char_data *victim,
						  struct obj_data *obj);
void                                    cast_flying(int level, struct char_data *ch, const char *arg,
						    int type, struct char_data *tar_ch,
						    struct obj_data *tar_obj);

#define SPELL_CONE_OF_COLD           70
void                                    spell_cone_of_cold(int level, struct char_data *ch,
							   struct char_data *victim,
							   struct obj_data *obj);
void                                    cast_cone_of_cold(int level, struct char_data *ch,
							  const char *arg, int type,
							  struct char_data *victim,
							  struct obj_data *tar_obj);

#define SPELL_METEOR_SWARM           71
void                                    spell_meteor_swarm(int level, struct char_data *ch,
							   struct char_data *victim,
							   struct obj_data *obj);
void                                    cast_meteor_swarm(int level, struct char_data *ch,
							  const char *arg, int type,
							  struct char_data *victim,
							  struct obj_data *tar_obj);

#define SPELL_ICE_STORM              72
void                                    spell_ice_storm(int level, struct char_data *ch,
							struct char_data *victim,
							struct obj_data *obj);
void                                    cast_ice_storm(int level, struct char_data *ch,
						       const char *arg, int type,
						       struct char_data *victim,
						       struct obj_data *tar_obj);

#define SPELL_SHIELD                 73
void                                    spell_shield(int level, struct char_data *ch,
						     struct char_data *victim,
						     struct obj_data *obj);
void                                    cast_shield(int level, struct char_data *ch, const char *arg,
						    int type, struct char_data *tar_ch,
						    struct obj_data *tar_obj);

#define SPELL_MON_SUM_1              74
void                                    spell_Create_Monster(int level, struct char_data *ch,
							     struct char_data *victim,
							     struct obj_data *obj);
void                                    cast_mon_sum1(int level, struct char_data *ch,
						      const char *arg, int type,
						      struct char_data *tar_ch,
						      struct obj_data *tar_obj);

#define SPELL_MON_SUM_2              75
void                                    cast_mon_sum2(int level, struct char_data *ch,
						      const char *arg, int type,
						      struct char_data *tar_ch,
						      struct obj_data *tar_obj);

#define SPELL_MON_SUM_3              76
void                                    cast_mon_sum3(int level, struct char_data *ch,
						      const char *arg, int type,
						      struct char_data *tar_ch,
						      struct obj_data *tar_obj);

#define SPELL_MON_SUM_4              77
void                                    cast_mon_sum4(int level, struct char_data *ch,
						      const char *arg, int type,
						      struct char_data *tar_ch,
						      struct obj_data *tar_obj);

#define SPELL_MON_SUM_5              78
void                                    cast_mon_sum5(int level, struct char_data *ch,
						      const char *arg, int type,
						      struct char_data *tar_ch,
						      struct obj_data *tar_obj);

#define SPELL_MON_SUM_6              79
void                                    cast_mon_sum6(int level, struct char_data *ch,
						      const char *arg, int type,
						      struct char_data *tar_ch,
						      struct obj_data *tar_obj);

#define SPELL_MON_SUM_7              80
void                                    cast_mon_sum7(int level, struct char_data *ch,
						      const char *arg, int type,
						      struct char_data *tar_ch,
						      struct obj_data *tar_obj);

#define SPELL_FIRESHIELD             81
void                                    spell_fireshield(int level, struct char_data *ch,
							 struct char_data *victim,
							 struct obj_data *obj);
void                                    cast_fireshield(int level, struct char_data *ch,
							const char *arg, int type,
							struct char_data *tar_ch,
							struct obj_data *tar_obj);

#define SPELL_CHARM_MONSTER          82
void                                    spell_charm_monster(int level, struct char_data *ch,
							    struct char_data *victim,
							    struct obj_data *obj);
void                                    cast_charm_monster(int level, struct char_data *ch,
							   const char *arg, int type,
							   struct char_data *tar_ch,
							   struct obj_data *tar_obj);

#define SPELL_CURE_SERIOUS           83
void                                    spell_cure_serious(int level, struct char_data *ch,
							   struct char_data *victim,
							   struct obj_data *obj);
void                                    cast_cure_serious(int level, struct char_data *ch,
							  const char *arg, int type,
							  struct char_data *tar_ch,
							  struct obj_data *tar_obj);

#define SPELL_CAUSE_SERIOUS          84
void                                    spell_cause_serious(int level, struct char_data *ch,
							    struct char_data *victim,
							    struct obj_data *obj);
void                                    cast_cause_serious(int level, struct char_data *ch,
							   const char *arg, int type,
							   struct char_data *victim,
							   struct obj_data *tar_obj);

#define SPELL_REFRESH                85
void                                    spell_refresh(int level, struct char_data *ch,
						      struct char_data *victim,
						      struct obj_data *obj);
void                                    cast_refresh(int level, struct char_data *ch,
						     const char *arg, int type,
						     struct char_data *tar_ch,
						     struct obj_data *tar_obj);

#define SPELL_SECOND_WIND            86
void                                    spell_second_wind(int level, struct char_data *ch,
							  struct char_data *victim,
							  struct obj_data *obj);
void                                    cast_second_wind(int level, struct char_data *ch,
							 const char *arg, int type,
							 struct char_data *tar_ch,
							 struct obj_data *tar_obj);

#define SPELL_TURN                   87
void                                    spell_turn(int level, struct char_data *ch,
						   struct char_data *victim,
						   struct obj_data *obj);
void                                    cast_turn(int level, struct char_data *ch, const char *arg,
						  int type, struct char_data *tar_ch,
						  struct obj_data *tar_obj);

#define SPELL_SUCCOR                 88
void                                    spell_succor(int level, struct char_data *ch,
						     struct char_data *victim,
						     struct obj_data *obj);
void                                    cast_succor(int level, struct char_data *ch, const char *arg,
						    int type, struct char_data *tar_ch,
						    struct obj_data *tar_obj);

#define SPELL_LIGHT                  89
void                                    spell_light(int level, struct char_data *ch,
						    struct char_data *victim,
						    struct obj_data *obj);
void                                    cast_light(int level, struct char_data *ch, const char *arg,
						   int type, struct char_data *tar_ch,
						   struct obj_data *tar_obj);

#define SPELL_CONT_LIGHT             90
void                                    spell_cont_light(int level, struct char_data *ch,
							 struct char_data *victim,
							 struct obj_data *obj);
void                                    cast_cont_light(int level, struct char_data *ch,
							const char *arg, int type,
							struct char_data *tar_ch,
							struct obj_data *tar_obj);

#define SPELL_CALM                   91
void                                    spell_calm(int level, struct char_data *ch,
						   struct char_data *victim,
						   struct obj_data *obj);
void                                    cast_calm(int level, struct char_data *ch, const char *arg,
						  int type, struct char_data *tar_ch,
						  struct obj_data *tar_obj);

#define SPELL_STONE_SKIN             92
void                                    spell_stone_skin(int level, struct char_data *ch,
							 struct char_data *victim,
							 struct obj_data *obj);
void                                    cast_stone_skin(int level, struct char_data *ch,
							const char *arg, int type,
							struct char_data *tar_ch,
							struct obj_data *tar_obj);

#define SPELL_CONJURE_ELEMENTAL      93
void                                    spell_conjure_elemental(int level,
								struct char_data *ch,
								struct char_data *victim,
								struct obj_data *obj);
void                                    cast_conjure_elemental(int level, struct char_data *ch,
							       const char *arg, int type,
							       struct char_data *tar_ch,
							       struct obj_data *tar_obj);

#define SPELL_TRUE_SIGHT             94
void                                    spell_true_seeing(int level, struct char_data *ch,
							  struct char_data *victim,
							  struct obj_data *obj);
void                                    cast_true_seeing(int level, struct char_data *ch,
							 const char *arg, int type,
							 struct char_data *tar_ch,
							 struct obj_data *tar_obj);

#define SPELL_MINOR_CREATE           95
void                                    spell_minor_create(int level, struct char_data *ch,
							   struct char_data *victim,
							   struct obj_data *obj);
void                                    cast_minor_creation(int level, struct char_data *ch,
							    const char *arg, int type,
							    struct char_data *tar_ch,
							    struct obj_data *tar_obj);

#define SPELL_FAERIE_FIRE            96
void                                    spell_faerie_fire(int level, struct char_data *ch,
							  struct char_data *victim,
							  struct obj_data *obj);
void                                    cast_faerie_fire(int level, struct char_data *ch,
							 const char *arg, int type,
							 struct char_data *tar_ch,
							 struct obj_data *tar_obj);

#define SPELL_FAERIE_FOG             97
void                                    spell_faerie_fog(int level, struct char_data *ch,
							 struct char_data *victim,
							 struct obj_data *obj);
void                                    cast_faerie_fog(int level, struct char_data *ch,
							const char *arg, int type,
							struct char_data *victim,
							struct obj_data *tar_obj);

#define SPELL_CACAODEMON             98
void                                    spell_cacaodemon(int level, struct char_data *ch,
							 struct char_data *victim,
							 struct obj_data *obj);
void                                    cast_cacaodemon(int level, struct char_data *ch,
							const char *arg, int type,
							struct char_data *tar_ch,
							struct obj_data *tar_obj);

#define SPELL_POLY_SELF              99
void                                    spell_poly_self(int level, struct char_data *ch,
							struct char_data *mob,
							struct obj_data *obj);
void                                    cast_poly_self(int level, struct char_data *ch,
						       const char *arg, int type,
						       struct char_data *tar_ch,
						       struct obj_data *tar_obj);

#define SPELL_MANA                  100
void                                    spell_mana(int level, struct char_data *ch,
						   struct char_data *victim,
						   struct obj_data *obj);
void                                    cast_mana(int level, struct char_data *ch, const char *arg,
						  int type, struct char_data *tar_ch,
						  struct obj_data *tar_obj);

#define SPELL_ASTRAL_WALK           101
void                                    spell_astral_walk(int level, struct char_data *ch,
							  struct char_data *victim,
							  struct obj_data *obj);
void                                    cast_astral_walk(int level, struct char_data *ch,
							 const char *arg, int type,
							 struct char_data *tar_ch,
							 struct obj_data *tar_obj);

#define SPELL_FLY_GROUP             102
void                                    spell_fly_group(int level, struct char_data *ch,
							struct char_data *victim,
							struct obj_data *obj);
void                                    cast_fly_group(int level, struct char_data *ch,
						       const char *arg, int type,
						       struct char_data *tar_ch,
						       struct obj_data *tar_obj);

#define SPELL_AID                   103
void                                    spell_aid(int level, struct char_data *ch,
						  struct char_data *victim,
						  struct obj_data *obj);
void                                    cast_aid(int level, struct char_data *ch, const char *arg,
						 int type, struct char_data *tar_ch,
						 struct obj_data *tar_obj);

#define SPELL_SHELTER               104
void                                    spell_shelter(int level, struct char_data *ch,
						      struct char_data *victim,
						      struct obj_data *obj);
void                                    cast_shelter(int level, struct char_data *ch,
						     const char *arg, int type,
						     struct char_data *tar_ch,
						     struct obj_data *tar_obj);

#define SPELL_DRAGON_BREATH         105
void                                    cast_dragon_breath(int level, struct char_data *ch,
							   const char *arg, int type,
							   struct char_data *tar_ch,
							   struct obj_data *potion);

#define SPELL_GOODBERRY             106
void                                    spell_goodberry(int level, struct char_data *ch,
							struct char_data *victim,
							struct obj_data *obj);
void                                    cast_goodberry(int level, struct char_data *ch,
						       const char *arg, int type,
						       struct char_data *tar_ch,
						       struct obj_data *tar_obj);

#define SPELL_VISIONS               107
void                                    spell_visions(int level, struct char_data *ch,
						      struct char_data *victim,
						      struct obj_data *obj);
void                                    cast_visions(int level, struct char_data *ch,
						     const char *arg, int type,
						     struct char_data *tar_ch,
						     struct obj_data *tar_obj);

#define MAX_EXIST_SPELL             107			       /* move this and change it */

#define SPELL_H_FEAST               103
#define SPELL_WEB                   106
#define SPELL_MINOR_TRACK           107
#define SPELL_MAJOR_TRACK           108
#define SPELL_GOLEM                 109
void                                    spell_golem(int level, struct char_data *ch,
						    struct char_data *victim,
						    struct obj_data *obj);

#define SPELL_FAMILIAR              110
void                                    spell_familiar(int level, struct char_data *ch,
						       struct char_data **victim,
						       struct obj_data *obj);

#define SPELL_CHANGESTAFF           111
void                                    spell_changestaff(int level, struct char_data *ch,
							  struct char_data *victim,
							  struct obj_data *obj);

#define SPELL_HOLY_WORD             112
void                                    spell_holy_word(int level, struct char_data *ch,
							struct char_data *victim,
							struct obj_data *obj);
void                                    spell_holyword(int level, struct char_data *ch,
						       struct char_data *victim,
						       struct obj_data *obj);

#define SPELL_UNHOLY_WORD           113
void                                    spell_unholy_word(int level, struct char_data *ch,
							  struct char_data *victim,
							  struct obj_data *obj);

#define SPELL_PWORD_KILL            114
void                                    spell_pword_kill(int level, struct char_data *ch,
							 struct char_data *victim,
							 struct obj_data *obj);

#define SPELL_PWORD_BLIND           115
void                                    spell_pword_blind(int level, struct char_data *ch,
							  struct char_data *victim,
							  struct obj_data *obj);

#define SPELL_CHAIN_LIGHTNING       116
void                                    spell_chain_lightning(int level, struct char_data *ch,
							      struct char_data *victim,
							      struct obj_data *obj);

#define SPELL_SCARE                 117
void                                    spell_scare(int level, struct char_data *ch,
						    struct char_data *victim,
						    struct obj_data *obj);

#define SPELL_COMMAND               119
#define SPELL_CHANGE_FORM           120			       /* druid... */
#define SPELL_FEEBLEMIND            121
void                                    spell_feeblemind(int level, struct char_data *ch,
							 struct char_data *victim,
							 struct obj_data *obj);

#define SPELL_SHILLELAGH            122
void                                    spell_shillelagh(int level, struct char_data *ch,
							 struct char_data *victim,
							 struct obj_data *obj);

#define SPELL_FLAME_BLADE           124
void                                    spell_flame_blade(int level, struct char_data *ch,
							  struct char_data *victim,
							  struct obj_data *obj);

#define SPELL_ANIMAL_GROWTH         125
void                                    spell_animal_growth(int level, struct char_data *ch,
							    struct char_data *victim,
							    struct obj_data *obj);

#define SPELL_INSECT_GROWTH         126
void                                    spell_insect_growth(int level, struct char_data *ch,
							    struct char_data *victim,
							    struct obj_data *obj);

#define SPELL_CREEPING_DEATH        127
void                                    spell_creeping_death(int level, struct char_data *ch,
							     struct char_data *victim, int dir);

#define SPELL_COMMUNE               128			       /* whatzone */
void                                    spell_commune(int level, struct char_data *ch,
						      struct char_data *victim,
						      struct obj_data *obj);

#define SPELL_ANIMAL_SUM_1          129
void                                    spell_animal_summon(int level, struct char_data *ch,
							    struct char_data *victim,
							    struct obj_data *obj);

#define SPELL_ANIMAL_SUM_2          130
#define SPELL_ANIMAL_SUM_3          131
#define SPELL_FIRE_SERVANT          132
void                                    spell_elemental_summoning(int level,
								  struct char_data *ch,
								  struct char_data *victim,
								  int spell);

#define SPELL_EARTH_SERVANT         133
#define SPELL_WATER_SERVANT         134
#define SPELL_WIND_SERVANT          135
#define SPELL_REINCARNATE           136
void                                    spell_reincarnate(int level, struct char_data *ch,
							  struct char_data *victim,
							  struct obj_data *obj);
void                                    spell_resurrection(int level, struct char_data *ch,
							   struct char_data *victim,
							   struct obj_data *obj);
void                                    cast_resurrection(int level, struct char_data *ch,
							  const char *arg, int type,
							  struct char_data *tar_ch,
							  struct obj_data *tar_obj);

#define SPELL_CHARM_VEGGIE          137
void                                    spell_charm_veggie(int level, struct char_data *ch,
							   struct char_data *victim,
							   struct obj_data *obj);

#define SPELL_VEGGIE_GROWTH         138
void                                    spell_veggie_growth(int level, struct char_data *ch,
							    struct char_data *victim,
							    struct obj_data *obj);

#define SPELL_TREE                  139
void                                    spell_tree(int level, struct char_data *ch,
						   struct char_data *victim,
						   struct obj_data *obj);

#define SPELL_ANIMATE_ROCK          140
void                                    spell_animate_rock(int level, struct char_data *ch,
							   struct char_data *victim,
							   struct obj_data *obj);

#define SPELL_TREE_TRAVEL           141
void                                    spell_tree_travel(int level, struct char_data *ch,
							  struct char_data *victim,
							  struct obj_data *obj);

#define SPELL_TRAVELLING            142			       /* faster move outdoors */
void                                    spell_travelling(int level, struct char_data *ch,
							 struct char_data *victim,
							 struct obj_data *obj);

#define SPELL_ANIMAL_FRIENDSHIP     143
void                                    spell_animal_friendship(int level,
								struct char_data *ch,
								struct char_data *victim,
								struct obj_data *obj);

#define SPELL_INVIS_TO_ANIMALS      144
void                                    spell_invis_to_animals(int level, struct char_data *ch,
							       struct char_data *victim,
							       struct obj_data *obj);

#define SPELL_SLOW_POISON           145
void                                    spell_slow_poison(int level, struct char_data *ch,
							  struct char_data *victim,
							  struct obj_data *obj);

#define SPELL_ENTANGLE              146
void                                    spell_entangle(int level, struct char_data *ch,
						       struct char_data *victim,
						       struct obj_data *obj);

#define SPELL_SNARE                 147
void                                    spell_snare(int level, struct char_data *ch,
						    struct char_data *victim,
						    struct obj_data *obj);

#define SPELL_GUST_OF_WIND          148
void                                    spell_gust_of_wind(int level, struct char_data *ch,
							   struct char_data *victim,
							   struct obj_data *obj);

#define SPELL_BARKSKIN              149
void                                    spell_barkskin(int level, struct char_data *ch,
						       struct char_data *victim,
						       struct obj_data *obj);

#define SPELL_SUNRAY                150
void                                    spell_sunray(int level, struct char_data *ch,
						     struct char_data *victim,
						     struct obj_data *obj);

#define SPELL_WARP_WEAPON           151
void                                    spell_warp_weapon(int level, struct char_data *ch,
							  struct char_data *victim,
							  struct obj_data *obj);

#define SPELL_HEAT_STUFF            152
void                                    spell_heat_stuff(int level, struct char_data *ch,
							 struct char_data *victim,
							 struct obj_data *obj);

#define SPELL_FIND_TRAPS            153
void                                    spell_find_traps(int level, struct char_data *ch,
							 struct char_data *victim,
							 struct obj_data *obj);

#define SPELL_FIRESTORM             154
void                                    spell_firestorm(int level, struct char_data *ch,
							struct char_data *victim,
							struct obj_data *obj);

#define SPELL_HASTE                 155			       /* other */
void                                    spell_haste(int level, struct char_data *ch,
						    struct char_data *victim,
						    struct obj_data *obj);

#define SPELL_SLOW                  156
void                                    spell_slow(int level, struct char_data *ch,
						   struct char_data *victim,
						   struct obj_data *obj);

#define SPELL_DUST_DEVIL            157
void                                    spell_dust_devil(int level, struct char_data *ch,
							 struct char_data *victim,
							 struct obj_data *obj);

#define SPELL_KNOW_MONSTER          158
void                                    spell_know_monster(int level, struct char_data *ch,
							   struct char_data *victim,
							   struct obj_data *obj);

#define SPELL_TRANSPORT_VIA_PLANT   159
void                                    spell_transport_via_plant(int level,
								  struct char_data *ch,
								  struct char_data *victim,
								  struct obj_data *obj);

#define SPELL_SPEAK_WITH_PLANT      160
void                                    spell_speak_with_plants(int level,
								struct char_data *ch,
								struct char_data *victim,
								struct obj_data *obj);

#define SPELL_SILENCE               161
void                                    spell_silence(int level, struct char_data *ch,
						      struct char_data *victim,
						      struct obj_data *obj);

#define SPELL_SENDING               162
void                                    spell_sending(int level, struct char_data *ch,
						      struct char_data *victim,
						      struct obj_data *obj);

#define SPELL_TELEPORT_WO_ERROR     163
void                                    spell_teleport_wo_error(int level,
								struct char_data *ch,
								struct char_data *victim,
								struct obj_data *obj);

#define SPELL_PORTAL                164
void                                    spell_portal(int level, struct char_data *ch,
						     struct char_data *tmp_ch,
						     struct obj_data *obj);

#define SPELL_DRAGON_RIDE           165
void                                    spell_dragon_ride(int level, struct char_data *ch,
							  struct char_data *victim,
							  struct obj_data *obj);

#define SPELL_MOUNT                 166
void                                    spell_mount(int level, struct char_data *ch,
						    struct char_data *victim,
						    struct obj_data *obj);

#define SPELL_BLADE_BARRIER				       /* room spell like deal */
#define SPELL_SUMMON_OBJ				       /* maybe */
void                                    spell_summon_obj(int level, struct char_data *ch,
							 struct char_data *victim,
							 struct obj_data *obj);

#define SPELL_GREEN_SLIME            199
void                                    spell_green_slime(int level, struct char_data *ch,
							  struct char_data *victim,
							  struct obj_data *obj);
void                                    cast_green_slime(int level, struct char_data *ch,
							 const char *arg, int type,
							 struct char_data *victim,
							 struct obj_data *tar_obj);

#define SPELL_GEYSER                 200
void                                    spell_geyser(int level, struct char_data *ch,
						     struct char_data *victim,
						     struct obj_data *obj);
void                                    cast_geyser(int level, struct char_data *ch, const char *arg,
						    int type, struct char_data *victim,
						    struct obj_data *tar_obj);

#define SPELL_FIRE_BREATH            201
void                                    spell_fire_breath(int level, struct char_data *ch,
							  struct char_data *victim,
							  struct obj_data *obj);
void                                    cast_fire_breath(int level, struct char_data *ch,
							 const char *arg, int type,
							 struct char_data *tar_ch,
							 struct obj_data *tar_obj);

#define SPELL_GAS_BREATH             202
void                                    spell_gas_breath(int level, struct char_data *ch,
							 struct char_data *victim,
							 struct obj_data *obj);
void                                    cast_gas_breath(int level, struct char_data *ch,
							const char *arg, int type,
							struct char_data *tar_ch,
							struct obj_data *tar_obj);

#define SPELL_FROST_BREATH           203
void                                    spell_frost_breath(int level, struct char_data *ch,
							   struct char_data *victim,
							   struct obj_data *obj);
void                                    cast_frost_breath(int level, struct char_data *ch,
							  const char *arg, int type,
							  struct char_data *tar_ch,
							  struct obj_data *tar_obj);

#define SPELL_ACID_BREATH            204
void                                    spell_acid_breath(int level, struct char_data *ch,
							  struct char_data *victim,
							  struct obj_data *obj);
void                                    cast_acid_breath(int level, struct char_data *ch,
							 const char *arg, int type,
							 struct char_data *tar_ch,
							 struct obj_data *tar_obj);

#define SPELL_LIGHTNING_BREATH       205
void                                    spell_lightning_breath(int level, struct char_data *ch,
							       struct char_data *victim,
							       struct obj_data *obj);
void                                    cast_lightning_breath(int level, struct char_data *ch,
							      const char *arg, int type,
							      struct char_data *tar_ch,
							      struct obj_data *tar_obj);

#define FIRST_BREATH_WEAPON	     200
#define LAST_BREATH_WEAPON	     205

/* no defines for these? */
void                                    spell_chain_lightn(int level, struct char_data *ch,
							   struct char_data *victim,
							   struct obj_data *obj);
void                                    spell_detect_charm(int level, struct char_data *ch,
							   struct char_data *victim,
							   struct obj_data *obj);
void                                    spell_invis_group(int level, struct char_data *ch,
							  struct char_data *victim,
							  struct obj_data *obj);
void                                    spell_major_create(int level, struct char_data *ch,
							   struct char_data *victim,
							   struct obj_data *obj);
void                                    spell_poison_cloud(int level, struct char_data *ch,
							   struct char_data *victim,
							   struct obj_data *obj);
void                                    spell_prot_align_group(int level, struct char_data *ch,
							       struct char_data *victim,
							       struct obj_data *obj);

/* MAX_SKILL is 200!  */

#define SKILL_SNEAK             45			       /* r,t */
#define SKILL_HIDE              46			       /* r,t */
#define SKILL_STEAL             47			       /* t */
#define SKILL_BACKSTAB          48			       /* t */
#define SKILL_PICK_LOCK         49			       /* t */
#define SKILL_KICK              50			       /* f */
#define SKILL_BASH              51			       /* f */
#define SKILL_RESCUE            52			       /* f,r */
#define SKILL_DOOR_BASH	 	149			       /* f,r */
#define SKILL_READ_MAGIC 	150			       /* f,m,c,t,r */
#define SKILL_SCRIBE		151			       /* m,c */
#define SKILL_BREW		152			       /* m,c */
#define SKILL_PUNCH		153			       /* f,c,r */
#define SKILL_TWO_HANDED	154			       /* f,m,c,t,r */
#define SKILL_TWO_WEAPON	155			       /* NONE */
#define SKILL_BANDAGE		156			       /* f,m,c,t,r */
#define SKILL_SEARCH		157			       /* t,r */
#define SKILL_SWIMMING		158			       /* f,m,c,t,r */
#define SKILL_ENDURANCE		159			       /* f,m,c,r,t */
#define SKILL_BARE_HAND		160			       /* f,m,c,t,r */
#define SKILL_BLIND_FIGHTING	161			       /* f,c,m,r,t */
#define SKILL_PARRY		162			       /* f,r,t NOT PUT IN */
#define SKILL_APRAISE		163			       /* f,m,c,t,r */
#define SKILL_SPEC_SMITE	164			       /* f */
#define SKILL_SPEC_STAB		165
#define SKILL_SPEC_WHIP		166
#define SKILL_SPEC_SLASH	167
#define SKILL_SPEC_SMASH	168
#define SKILL_SPEC_CLEAVE	169
#define SKILL_SPEC_CRUSH	170
#define SKILL_SPEC_BLUDGE	171
#define SKILL_SPEC_PIERCE	172			       /* f */
#define SKILL_PEER		173			       /* t,r */
#define SKILL_DETECT_NOISE	174			       /* t,r */
#define SKILL_DODGE		175			       /* m,c,t,r NOT PUT IN */
#define SKILL_BARTER		176			       /* m,c,t */
#define SKILL_KNOCK_OUT		177			       /* m,c,t NOT PUT IN */
#define SKILL_SPELLCRAFT	178			       /* m,c */
#define SKILL_MEDITATION	179			       /* m,c */
#define SKILL_TRACK             180			       /* r,t */
#define SKILL_FIND_TRAP         181			       /* t */
#define SKILL_DISARM_TRAP       182			       /* t */
#define SKILL_DISARM            183			       /* f,r */
#define SKILL_BASH_W_SHIELD     184			       /* not a useable skill, but included with bash */
#define SKILL_RIDE		185
/*
 * #define SKILL_SIGN                   171
 * #define SKILL_DODGE                  174
 * #define SKILL_RETREAT                176
 * #define SKILL_FEIGN_DEATH            179
 * #define SKILL_SPRING_LEAP            182
 * #define SKILL_EVALUATE               185
 * #define SKILL_SPY                    186
 * #define SKILL_SWIM                   188
 */

#define TYPE_HIT                     206
#define TYPE_BLUDGEON                207
#define TYPE_PIERCE                  208
#define TYPE_SLASH                   209
#define TYPE_WHIP                    210		       /* EXAMPLE */
#define TYPE_CLAW                    211		       /* NO MESSAGES WRITTEN YET! */
#define TYPE_BITE                    212		       /* NO MESSAGES WRITTEN YET! */
#define TYPE_STING                   213		       /* NO MESSAGES WRITTEN YET! */
#define TYPE_CRUSH                   214		       /* NO MESSAGES WRITTEN YET! */
#define TYPE_CLEAVE                  215
#define TYPE_STAB                    216
#define TYPE_SMASH                   217
#define TYPE_SMITE                   218
#define TYPE_SUFFERING               220
#define TYPE_HUNGER                  221

/* More anything but spells and weapontypes can be insterted here! */

#define MAX_TYPES 70

#endif

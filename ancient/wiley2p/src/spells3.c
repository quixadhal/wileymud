/* ************************************************************************
*  file: spells2.c , Implementation of magic.             Part of DIKUMUD *
*  Usage : All the non-offensive magic handling routines.                 *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <strings.h>
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "interpreter.h"
#include "spells.h"
#include "handler.h"
#include "poly.h"

/* Global data */


extern struct room_data *world;
extern struct char_data *character_list;
extern struct spell_info_type spell_info[MAX_SPL_LIST];
extern struct obj_data  *object_list;
extern int rev_dir[];
extern char *dirs[]; 
extern int movement_loss[];
extern struct weather_data weather_info;
extern struct time_info_data time_info;
extern struct index_data *obj_index;

/* Extern procedures */

void die(struct char_data *ch);
void update_pos( struct char_data *victim );
void damage(struct char_data *ch, struct char_data *victim,
            int damage, int weapontype);
void clone_char(struct char_data *ch);
void say_spell( struct char_data *ch, int si );
bool saves_spell(struct char_data *ch, sh_int spell);
void add_follower(struct char_data *ch, struct char_data *victim);
char in_group(struct char_data *ch1, struct char_data *ch2);
void ChangeWeather( int change);
void raw_unlock_door( struct char_data *ch, struct room_direction_data *exitp,
		     int door);
int NoSummon(struct char_data *ch);

void cast_fly_group( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
       	spell_fly_group(level,ch,0,0);
       	break;
    case SPELL_TYPE_POTION:
       	spell_fly(level,ch,tar_ch,0);
       	break;
      default : 
         log("Serious screw-up in fly");
         break;
  }
}



void cast_aid( byte level, struct char_data *ch, char *arg,
     int type, struct char_data *tar_ch, struct obj_data *tar_obj )
{
  if (!tar_ch) tar_ch = ch;

  switch(type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_WAND:
  case SPELL_TYPE_STAFF:
      spell_aid(level, ch, tar_ch, 0);
      break;
  default:
      log("serious screw-up in scare.");
      break;
  }
}


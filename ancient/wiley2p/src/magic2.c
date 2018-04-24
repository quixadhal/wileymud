/* ************************************************************************
*  file: magic2.c , Implementation of (new)spells.        Part of DIKUMUD *
*  Usage : The actual effect of magic.                                    *
************************************************************************* */

#include <stdio.h>
#include <assert.h>
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "spells.h"
#include "handler.h"
#include "limits.h"
#include "db.h"
#include "race.h"

/* Extern structures */
extern struct room_data *world;
extern struct obj_data  *object_list;
extern struct char_data *character_list;

/* Extern procedures */

void damage(struct char_data *ch, struct char_data *victim,
            int damage, int weapontype);
bool saves_spell(struct char_data *ch, sh_int spell);
void weight_change_object(struct obj_data *obj, int weight);
char *strdup(char *source);
int dice(int number, int size);
char in_group(struct char_data *ch1, struct char_data *ch2);
int IsExtraPlanar( struct char_data *ch);

/*
  cleric spells
*/

/*
 **   requires the sacrifice of 150k coins, victim loses a con point, and
 **   caster is knocked down to 1 hp, 1 mp, 1 mana, and sits for a LONG
 **   time (if a pc)
 */

void spell_resurrection(byte level, struct char_data *ch,
		     struct char_data *victim, struct obj_data *obj)
{
  struct char_file_u st;
  struct affected_type af;
  struct obj_data *obj_object, *next_obj;  
  FILE *fl;  
  
  if (!obj) return;
  
  if(IS_CORPSE(obj)) 
  {
    if(obj->char_vnum) {  /* corpse is a npc */
      victim = read_mobile(obj->char_vnum, VIRTUAL);
      char_to_room(victim, ch->in_room);
      GET_GOLD(victim)=0;
      GET_EXP(victim)=0;
      GET_HIT(victim)=1;
      GET_POS(victim)=POSITION_STUNNED;
      
      act("With mystic power, $n resurrects a corpse.", TRUE, ch, 0, 0, TO_ROOM);
      act("$N slowly rises from the ground.", FALSE, ch, 0, victim, TO_ROOM);
      
      /*
	should be charmed and follower ch
	*/
      
      if (IsImmune(victim, IMM_CHARM) || IsResist(victim, IMM_CHARM)) {
 	act("$n says 'Thank you'", FALSE, ch, 0, victim, TO_ROOM);
      }
      else
      {
        af.type      = SPELL_CHARM_PERSON;
        af.duration  = 36;
        af.modifier  = 0;
        af.location  = 0;
        af.bitvector = AFF_CHARM;
	
        affect_to_char(victim, &af);
	
       	add_follower(victim, ch);
      }
      
      IS_CARRYING_W(victim) = 0;
      IS_CARRYING_N(victim) = 0;
      
      /*
	take all from corpse, and give to person
	*/
      
      for (obj_object=obj->contains; obj_object; obj_object=next_obj) {
	next_obj = obj_object->next_content;
	obj_from_obj(obj_object);
	obj_to_char(obj_object, victim);
      }
      
      /*
	get rid of corpse
	*/
      extract_obj(obj);
      
      
    } else {          /* corpse is a pc  */
      
      if (GET_GOLD(ch) < 100000) {
	send_to_char("The gods are not happy with your sacrifice.\n\r",ch);
	return;
      } else {
	GET_GOLD(ch) -= 100000;
      }
      
      fl = fopen(PLAYER_FILE, "r+");
      if (!fl) {
	perror("player file");
	exit(1);
      }
      fseek(fl, obj->char_f_pos * sizeof(struct char_file_u), 0);
      fread(&st, sizeof(struct char_file_u), 1, fl);
      /*
       **   this is a serious kludge, and must be changed before multiple
       **   languages can be implemented
       */	
      if (!st.talks[2] && st.abilities.con > 3) {
	st.points.exp *= 2;
	st.talks[2] = TRUE;
	st.abilities.con -= 1;
	act("A clear bell rings throughout the heavens", 
	    TRUE, ch, 0, 0, TO_CHAR);
	act("A ghostly spirit smiles, and says 'Thank you'", 
	    TRUE, ch, 0, 0, TO_CHAR);
	act("A clear bell rings throughout the heavens", 
	    TRUE, ch, 0, 0, TO_ROOM);
	act("A ghostly spirit smiles, and says 'Thank you'", 
	    TRUE, ch, 0, 0, TO_ROOM);
	act("$p dissappears in the blink of an eye.", 
	    TRUE, ch, obj, 0, TO_ROOM);
	act("$p dissappears in the blink of an eye.", 
	    TRUE, ch, obj, 0, TO_ROOM);
	GET_MANA(ch) = 1;
	GET_MOVE(ch) = 1;
	GET_HIT(ch) = 1;
	GET_POS(ch) = POSITION_STUNNED;
	act("$n collapses from the effort!",TRUE, ch, 0, 0, TO_ROOM);
	send_to_char("You collapse from the effort\n\r",ch);
        fseek(fl, obj->char_f_pos * sizeof(struct char_file_u), 0);
	fwrite(&st, sizeof(struct char_file_u), 1, fl);
	ObjFromCorpse(obj);	
	
      } else {
	send_to_char
	  ("The body does not have the strength to be recreated.\n\r", ch);
      }
      fclose(fl);
    }
  }  
}

void spell_cause_light(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  if(!CheckKill(ch,victim)) return;

  if(HowManyClasses(ch) == 1)
    dam = dice(2,4)+1;
  else
    dam = dice(1,8);

  if(IS_REALLY_HOLY(ch))
    dam -= dice(1,3);
  else
  if(IS_HOLY(ch))
    dam -= dice(1,2);
  else
  if(IS_VILE(ch))
    dam += dice(1,2);
  else
  if(IS_REALLY_VILE(ch))
    dam += dice(1,3);

  if(dam<1) dam = 1;

  damage(ch, victim, dam, SPELL_CAUSE_LIGHT);
}

void spell_cause_critical(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  if(!CheckKill(ch,victim)) return;
  if(HowManyClasses(ch) == 1)
    dam = dice(4,6) + 3;
  else
    dam = dice(3,8);

  if(IS_REALLY_HOLY(ch))
    dam -= dice(1,6);
  else
  if(IS_HOLY(ch))
    dam -= dice(1,3);
  else
  if(IS_VILE(ch))
    dam += dice(1,3);
  else
  if(IS_REALLY_VILE(ch))
    dam += dice(1,6);

  if(dam<1) dam = 1;

  damage(ch, victim, dam, SPELL_CAUSE_CRITICAL);
}

void spell_cause_serious(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  if(!CheckKill(ch,victim)) return;
  if(HowManyClasses(ch) == 1)
    dam = dice(4,4) + 2;
  else
    dam = dice(2,8);

  if(IS_REALLY_HOLY(ch))
    dam -= dice(1,4);
  else
  if(IS_HOLY(ch))
    dam -= dice(1,2);
  else
  if(IS_VILE(ch))
    dam += dice(1,2);
  else
  if(IS_REALLY_VILE(ch))
    dam += dice(1,4);

  if(dam<1) dam = 1;

  damage(ch, victim, dam, SPELL_CAUSE_SERIOUS);

}

void spell_cure_serious(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  if(HowManyClasses(ch) == 1)
    dam = dice(4,4) + 2;
  else
    dam = dice(2,8);

  if(IS_REALLY_HOLY(ch))
    dam += dice(1,4);
  else
  if(IS_HOLY(ch))
    dam += dice(1,2);
  else
  if(IS_VILE(ch))
    dam -= dice(1,2);
  else
  if(IS_REALLY_VILE(ch))
    dam -= dice(1,4);

  if(dam<1) dam = 1;

  if ( (dam + GET_HIT(victim)) > hit_limit(victim) )
    GET_HIT(victim) = hit_limit(victim);
  else
    GET_HIT(victim) += dam;

  send_to_char("You feel better!\n\r", victim);
  update_pos(victim);
}

void spell_mana(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(level,4);
  dam = MAX(dam, level*2);

  if (GET_MANA(ch)+dam > GET_MAX_MANA(ch))
    GET_MANA(ch) = GET_MAX_MANA(ch);
  else
    GET_MANA(ch) += dam;

}

void spell_second_wind(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(level,8)+level;

  if((dam + GET_MOVE(victim)) > move_limit(victim) )
    GET_MOVE(victim) = move_limit(victim);
  else
    GET_MOVE(victim) += dam;

  send_to_char("You feel less tired\n\r", victim);

}

void spell_flamestrike(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL)); 

  if(!CheckKill(ch,victim)) return;
  dam = dice((8 - HowManyClasses(ch)),8);

  if(saves_spell(victim, SAVING_SPELL) )
    dam >>= 1;

  send_to_char("You summon forth a pillar of flame....\n\r",ch);
  damage(ch, victim, dam, SPELL_FLAMESTRIKE);
}

void spell_dispel_good(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(ch && victim);
   assert((level >= 1) && (level<=ABS_MAX_LVL));
  if(!CheckKill(ch,victim)) return;

  if (IsExtraPlanar(victim)) {
	if (IS_GOOD(ch)) {
       	    victim = ch;
	} else if (IS_EVIL(victim)) {
            act("Evil protects $N.", FALSE, ch, 0, victim, TO_CHAR);
  	    return;
	}
      
        if (!saves_spell(victim, SAVING_SPELL) ) {
	    act("$n forces $N from this plane.",TRUE,ch,0,victim,TO_NOTVICT);
	    act("You force $N from this plane.", TRUE, ch, 0, victim, TO_CHAR);
	    act("$n forces you from this plane.", TRUE, ch, 0, victim,TO_VICT);
	  gain_exp(ch, MIN(GET_EXP(victim)/2, 50000));
	  extract_char(victim);
	}
      } else {
	act("$N laughs at you.", TRUE, ch, 0, victim, TO_CHAR);
	act("$N laughs at $n.", TRUE,ch, 0, victim, TO_NOTVICT);
	act("You laugh at $n.", TRUE,ch,0,victim,TO_VICT);
      }
}

void spell_turn(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int diff,i;
  int flag;

  assert(ch && victim);
  assert((level >= 1) && (level<=ABS_MAX_LVL));

  flag = 0;

  if(IsUndead(victim)) 
  {
    diff = level - GetTotLevel(victim);
    if (diff <= 0) 
    {
      act("You are powerless to affect $N", TRUE, ch, 0, victim, TO_CHAR);
      return;
    }
    else
    {
      for (i = 1; i <= diff; i++) 
      {
        if (!saves_spell(victim, SAVING_SPELL) ) 
	{
	    act("$n forces $N from this room.",TRUE,ch,0,victim,TO_NOTVICT);
	    act("You force $N from this room.", TRUE, ch, 0, victim, TO_CHAR);
	    act("$n forces you from this room.", TRUE, ch, 0, victim,TO_VICT);
	    do_flee(victim,"",0);
	    flag = 1;
	    break;
	}
      }
      if (!flag)
      {
	act("You laugh at $n.", TRUE, ch, 0, victim, TO_VICT);
	act("$N laughs at $n.", TRUE, ch, 0, victim, TO_NOTVICT);
	act("$N laughs at you.", TRUE, ch, 0, victim, TO_CHAR);
      }
    }
  }
  else {
	act("$n just tried to turn you, what a moron!", TRUE, ch, 0, victim, TO_VICT);
	act("$N thinks $n is really strange.", TRUE, ch, 0, victim, TO_NOTVICT);
	act("Um... $N isn't undead...", TRUE, ch, 0, victim, TO_CHAR);
  }
}

void spell_remove_paralysis(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{

  assert(ch && victim);

  if (affected_by_spell(victim,SPELL_PARALYSIS)) 
  {
      affect_from_char(victim,SPELL_PARALYSIS);
      act("A warm feeling runs through your body.",FALSE,victim,0,0,TO_CHAR);
      act("$N looks better.",FALSE,ch,0,victim,TO_ROOM);
  } 
}

void spell_holy_word(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
}

void spell_unholy_word(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
}

void spell_succor(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
  struct obj_data *o;

  o = read_object(1901,VIRTUAL);
  obj_to_char(o,ch);
  act("$n waves $s hand, and creates $p", TRUE, ch, o, 0, TO_ROOM);
  act("You wave your hand and create $p.", TRUE, ch, o, 0, TO_CHAR);
}

void spell_detect_charm(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
}

void spell_true_seeing(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(victim && ch);

  if (!IS_AFFECTED(victim, AFF_TRUE_SIGHT)) {    
    if (ch != victim) {
       send_to_char("Your eyes glow silver for a moment.\n\r", victim);
       act("$n's eyes take on a silvery hue.\n\r", FALSE, victim, 0, 0, TO_ROOM);
    } else {
       send_to_char("Your eyes glow silver.\n\r", ch);
       act("$n's eyes glow silver.\n\r", FALSE, ch, 0, 0, TO_ROOM);
    }

    af.type      = SPELL_TRUE_SIGHT;
    af.duration  = level/2;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_TRUE_SIGHT;
    affect_to_char(victim, &af);
  } else {
    send_to_char("Nothing seems to happen\n\r", ch);
  }
}

/*
   magic user spells
*/

void spell_poly_self(byte level, struct char_data *ch,
   struct char_data *mob, struct obj_data *obj)
{

   if (!ch->desc || ch->desc->snoop.snoop_by || ch->desc->snoop.snooping) 
   {
      send_to_char("Godly interference prevents the spell from working.", ch);
      return;
   }

  char_to_room(mob, ch->in_room);

  /*  SwitchStuff(ch, mob);  */

  act("$n's flesh melts and flows into the shape of $N",TRUE, ch, 0, mob, TO_ROOM);
  act("Your flesh melts and flows into the shape of $N",TRUE, ch, 0, mob, TO_CHAR);

  char_from_room(ch);
  char_to_room(ch, 3); 
  
  ch->desc->character = mob;
  ch->desc->original = ch;
  
  mob->desc = ch->desc;
  ch->desc = 0;

  SET_BIT(mob->specials.act, ACT_POLYSELF);
  GET_EXP(mob) = 10;
}

void spell_minor_create(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{

  assert(ch && obj);
  
  act("$n claps $s hands together.", TRUE, ch, 0, 0, TO_ROOM);
  act("You clap your hands together.", TRUE, ch, 0, 0, TO_CHAR);
  act("In a flash of light, $p appears.", TRUE, ch, obj, 0, TO_ROOM);
  act("In a flash of light, $p appears.", TRUE, ch, obj, 0, TO_CHAR);
  obj_to_room(obj, ch->in_room);

}

void spell_stone_skin(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(ch);

  if(!affected_by_spell(ch, SPELL_STONE_SKIN)) 
  {
    act("$n's skin turns grey and granite-like.", TRUE, ch, 0, 0, TO_ROOM);
    act("Your skin turns to a stone-like substance.", TRUE, ch, 0, 0, TO_CHAR);

    af.type      = SPELL_STONE_SKIN;
    af.duration  = (level/4 - (dice(HowManyClasses(ch),3)));
    if(af.duration<1) af.duration = 1;

    af.modifier  = -30;
    af.location  = APPLY_AC;
    af.bitvector = 0;
    affect_to_char(ch, &af);

    /* resistance to piercing weapons */

    af.type      = SPELL_STONE_SKIN;
    af.duration  = (level/8 - (dice(HowManyClasses(ch),3)));
    if(af.duration<1) af.duration = 1;
    af.modifier  = 32;
    af.location  = APPLY_IMMUNE;
    af.bitvector = 0;
    affect_to_char(ch, &af);
  } 
  else
    send_to_char("The spell fizzles...\n\r",ch);
}

void spell_infravision(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(victim && ch);

  if (!IS_AFFECTED(victim, AFF_INFRAVISION)) 
  {
    if (ch != victim) 
    {
       send_to_char("Your eyes glow red.\n\r", victim);
       act("$n's eyes glow red.\n\r", FALSE, victim, 0, 0, TO_ROOM);
    }
    else
    {
       send_to_char("Your eyes glow red.\n\r", ch);
       act("$n's eyes glow red.\n\r", FALSE, ch, 0, 0, TO_ROOM);
    }

    af.type      = SPELL_INFRAVISION;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_INFRAVISION;
    affect_to_char(victim, &af);
  } 
}

void spell_shield(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(victim && ch);

  if (!affected_by_spell(victim, SPELL_SHIELD)) 
  {
    act("$N is surrounded by a strong force shield.", TRUE, ch, 0, victim, TO_NOTVICT);
    if (ch != victim) 
    {
       act("$N is surrounded by a strong force shield.", TRUE, ch, 0, victim, TO_CHAR);
       act("You are surrounded by a strong force shield.", TRUE, ch, 0, victim, TO_VICT);
    }
    else
    {
       act("You are surrounded by a strong force shield.", TRUE, ch, 0, victim, TO_VICT);
    }

    af.type      = SPELL_SHIELD;
    af.duration  = level;
    af.modifier  = -10;
    af.location  = APPLY_AC;
    af.bitvector = 0;
    affect_to_char(victim, &af);
  } 
  else
    send_to_char("The spell fizzles....\n\r",ch);
}

void spell_weakness(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;
  float modifier;

  assert(ch && victim);
  if(!CheckKill(ch,victim)) return;

  if (!affected_by_spell(victim,SPELL_WEAKNESS))
     if (!saves_spell(victim, SAVING_SPELL)) {
        modifier = (77.0 - level)/100.0;
        act("You feel weaker.", FALSE, victim,0,0,TO_VICT);
        act("$n seems weaker.", FALSE, victim, 0, 0, TO_ROOM);

        af.type      = SPELL_WEAKNESS;
        af.duration  = (int) level/2;
        af.modifier  = (int) 0 - (victim->abilities.str * modifier);
        if (victim->abilities.str_add) 
           af.modifier -= 2;
        af.location  = APPLY_STR;
        af.bitvector = 0;

        affect_to_char(victim, &af);
      }
}

void spell_invis_group(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
  struct char_data *tmp_victim, *temp;
  struct affected_type af;

  assert(ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL)); 

   for ( tmp_victim = real_roomp(ch->in_room)->people; tmp_victim; 
	tmp_victim = tmp_victim->next_in_room) {
      if ( (ch->in_room == tmp_victim->in_room) && (ch != tmp_victim))
         if (in_group(ch,tmp_victim)) {
       	    if (!affected_by_spell(tmp_victim, SPELL_INVISIBLE)) {

	       act("$n slowly fades out of existence.", TRUE, tmp_victim,0,0,TO_ROOM);
  	       send_to_char("You vanish.\n\r", tmp_victim);

	       af.type      = SPELL_INVISIBLE;
    	       af.duration  = level/2;
    	       af.modifier  = -40;
  	       af.location  = APPLY_AC;
	       af.bitvector = AFF_INVISIBLE;
  	       affect_to_char(tmp_victim, &af);
	     }
	 }         
    }
}


void spell_acid_blast(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
  int dam;
  int high_die;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL)); 
  if(!CheckKill(ch,victim)) return;

  switch(HowManyClasses(ch))
  {
    case 1:
      high_die = 5 + (( level / 7 ) - 1);
      break;
    case 2:
      high_die = 5 + (( level / 8 ) - 1);
      break;
    case 3:
      high_die = 5 + (( level / 9 ) - 1);
      break;
    case 4:
      high_die = 5 + (( level / 10 ) - 1);
      break;
    default:
      high_die = 5 + (( level / 11 ) - 1);
      break;
  }

  dam = dice(high_die,6);

  if ( saves_spell(victim, SAVING_SPELL) )
    dam >>= 1;
  damage(ch, victim, dam, SPELL_ACID_BLAST);
}

void spell_cone_of_cold(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{

  int dam;
  int high_die;

  struct char_data *tmp_victim, *temp;

  assert(ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  switch(HowManyClasses(ch))
  {
    case 1:
      high_die = 5 + (( level / 5 ) - 1);
      break;
    case 2:
      high_die = 4 + (( level / 6 ) - 1);
      break;
    case 3:
      high_die = 4 + (( level / 7 ) - 1);
      break;
    case 4:
      high_die = 4 + (( level / 8 ) - 1);
      break;
    default:
      high_die = 4 + (( level / 9 ) - 1);
      break;
  }

  dam = dice(high_die,3)+1;

  send_to_char("A cone of freezing air fans out before you\n\r", ch);
  act("$n sends a cone of ice shooting from their fingertips!\n\r",FALSE, ch, 0, 0, TO_ROOM);

  AreaEffectSpell(ch,dam,SPELL_CONE_OF_COLD,0,0);

#ifdef 0
  for( tmp_victim = real_roomp(ch->in_room)->people; tmp_victim; 
	tmp_victim = tmp_victim->next_in_room) 
  {
    if ( (ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) 
    {
      if((GetMaxLevel(tmp_victim)>LOW_IMMORTAL) && (IS_PC(tmp_victim)))
        return;

      act("You are chilled to the bone!\n\r",FALSE, ch, 0, tmp_victim, TO_VICT);
      if(saves_spell(tmp_victim, SAVING_SPELL) )
        dam >>= 1;
      damage(ch, tmp_victim, dam, SPELL_CONE_OF_COLD);
    }
  }
#endif
}

void spell_ice_storm(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
  int dam;
  struct char_data *tmp_victim, *temp;

  assert(ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(3,20);

  send_to_char("You conjure a huge cloud of ice crystals...they hover just above\n\r", ch);
  send_to_char("your foes head, the weight finally brings them crashing down on top...\n\r",ch);
  act("$n conjures a chilling storm of ice!\n\r", FALSE, ch, 0, 0, TO_ROOM);

  AreaEffectSpell(ch,dam,SPELL_ICE_STORM,0,0);
#ifdef 0
  for ( tmp_victim = real_roomp(ch->in_room)->people; tmp_victim; 
	tmp_victim = tmp_victim->next_in_room) 
  {
    if((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) 
    {
      if((GetMaxLevel(tmp_victim)>LOW_IMMORTAL) && (IS_PC(tmp_victim)))
          return;
      act("You are blasted by the storm\n\r",FALSE, ch, 0, tmp_victim, TO_VICT);
      if(saves_spell(tmp_victim, SAVING_SPELL) )
        dam >>= 1;
      damage(ch, tmp_victim, dam, SPELL_ICE_STORM);
    }
  }
#endif
}


void spell_poison_cloud(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
}

void spell_chain_lightning(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
}

void spell_major_create(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
}

void spell_summon_obj(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
}

void spell_pword_blind(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
}

void spell_pword_kill(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
}


void spell_sending(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
}

void spell_meteor_swarm(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
  int dam;
  int high_die;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL)); 

  if(!CheckKill(ch,victim)) return;

  switch(HowManyClasses(ch))
  {
    case 1:
      high_die = 4 + (( level / 5 ) - 1);
      break;
    case 2:
      high_die = 3 + (( level / 6 ) - 1);
      break;
    case 3:
      high_die = 3 + (( level / 7 ) - 1);
      break;
    case 4:
      high_die = 3 + (( level / 8 ) - 1);
      break;
    default:
      high_die = 2 + (( level / 9 ) - 1);
      break;
  }

  dam = dice(high_die,8);

  if(saves_spell(victim, SAVING_SPELL))
    dam >>= 1;

  damage(ch, victim, dam, SPELL_METEOR_SWARM);
}

void spell_Create_Monster(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;
  struct char_data *mob;
  int rnum;

   /* load in a monster of the correct type, determined by
      level of the spell */

/* really simple to start out with */

   if (level <= 5) 
   {
      rnum = number(1,10)+200;      
      mob = read_mobile(rnum, VIRTUAL);
   } else if (level <= 7) {
      rnum = number(1,10)+210;
      mob = read_mobile(rnum, VIRTUAL);
   } else if (level <= 9) {
      rnum = number(1,10)+220;
      mob = read_mobile(rnum, VIRTUAL);
   } else if (level <= 11) {
      rnum = number(1,10)+230;
      mob = read_mobile(rnum, VIRTUAL);
   } else if (level <= 13) {
      rnum = number(1,10)+240;
      mob = read_mobile(rnum, VIRTUAL);
   } else if (level <= 15) {
      rnum = number(1,8)+250;
      mob = read_mobile(rnum, VIRTUAL);      
   } else {
      rnum = number(0,1)+261;
      mob = read_mobile(rnum, VIRTUAL);
   }

    char_to_room(mob, ch->in_room);

    act("$n waves $s hand, and $N appears!", TRUE, ch, 0, mob, TO_ROOM);
    act("You wave your hand, and $N appears!", TRUE, ch, 0, mob, TO_CHAR);

   /* charm them for a while */
    if (mob->master)
      stop_follower(mob);

    add_follower(mob, ch);

    af.type      = SPELL_CHARM_PERSON;

    if(GET_INT(mob))
      af.duration  = 24-GET_INT(mob);
    else
      af.duration  = 12;

    af.modifier  = 0;
    af.location  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char(mob, &af);


/*
  adjust the bits...
*/

/*
 get rid of aggressive, add sentinel
*/

  if (IS_SET(mob->specials.act, ACT_AGGRESSIVE)) 
  {
    REMOVE_BIT(mob->specials.act, ACT_AGGRESSIVE);
  }

/*
  if(!IS_SET(mob->specials.act, ACT_SENTINEL)) 
  {
    SET_BIT(mob->specials.act, ACT_SENTINEL);
  }
*/

}

/*
   either
*/

void spell_light(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
   
/*
   creates a ball of light in the hands.
*/
  struct obj_data *tmp_obj;

  assert(ch);
  assert((level >= 0) && (level <= ABS_MAX_LVL));

  CREATE(tmp_obj, struct obj_data, 1);
  clear_object(tmp_obj);

  tmp_obj->name = strdup("ball light");
  tmp_obj->short_description = strdup("A ball of light");
  tmp_obj->description = strdup("There is a ball of light on the ground here.");

  tmp_obj->obj_flags.type_flag = ITEM_LIGHT;
  tmp_obj->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD;
  tmp_obj->obj_flags.value[2] = 24;
  tmp_obj->obj_flags.weight = 1;
  tmp_obj->obj_flags.cost = 10;
  tmp_obj->obj_flags.cost_per_day = 1;

  tmp_obj->next = object_list;
  object_list = tmp_obj;

  obj_to_char(tmp_obj,ch);

  tmp_obj->item_number = -1;

  act("$n intones a mistake magic and $p suddenly appears.",TRUE,ch,tmp_obj,0,TO_ROOM);
  act("With a flick of your wrist, $p suddenly appears.",TRUE,ch,tmp_obj,0,TO_CHAR);
}

void spell_fly(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;
  int dur;

  assert(ch && victim);

  if(IS_AFFECTED(victim, AFF_FLYING))
  {
    send_to_char("You are already flying....\n\r",ch);
    return;
  }

  send_to_char("Your feet begin to inch above the ground!\n\r",victim);

  if (victim != ch) 
     act("$N's feet rise off the ground.", TRUE, ch, 0, victim, TO_CHAR);
  else 
     send_to_char("Your feet rise up off the ground.", ch);

  act("$N's feet rise off the ground.", TRUE, ch, 0, victim, TO_NOTVICT);

  dur = 10;
 
  switch(HowManyClasses(ch))
  {
    case 1:
      dur = GET_LEVEL(ch,BestMagicClass(ch))/2 + 8;
      break;
    case 2:
      dur = GET_LEVEL(ch,BestMagicClass(ch))/2 + 4;
      break;
    case 3:
      dur = GET_LEVEL(ch,BestMagicClass(ch))/2;
      break;
    case 4:
      dur = GET_LEVEL(ch,BestMagicClass(ch))/4 + 8;
      break;
    default:
      dur = GET_LEVEL(ch,BestMagicClass(ch))/6 + 8;
      break;
  }

    af.type      = SPELL_FLY;
    af.duration  = dur;
    af.modifier  = 0;
    af.location  = 0;
    af.bitvector = AFF_FLYING;
    affect_to_char(victim, &af);
}

void spell_refresh(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(level,4)+level;
  dam = MAX(dam,30);

  if(GET_LEVEL(ch,MAGE_LEVEL_IND)>1)
    dam>>1;

  if((dam + GET_MOVE(victim)) > move_limit(victim))
    GET_MOVE(victim) = move_limit(victim);
  else
    GET_MOVE(victim) += dam;

  send_to_char("You feel less tired\n\r", victim);
}


void spell_water_breath(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(ch && victim);
  
  act("You feel fishy!", TRUE, ch, 0, victim, TO_VICT);
  if (victim != ch) 
  {
     act("$N makes a face like a fish.", TRUE, ch, 0, victim, TO_CHAR);
  }
  act("$N makes a face like a fish.", TRUE, ch, 0, victim, TO_NOTVICT);
  
    af.type      = SPELL_WATER_BREATH;
    af.duration  = GET_LEVEL(ch,BestMagicClass(ch));
    af.modifier  = 0;
    af.location  = 0;
    af.bitvector = AFF_WATERBREATH;
    affect_to_char(victim, &af);
}

void spell_cont_light(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
/*
   creates a ball of light in the hands.
*/
  struct obj_data *tmp_obj;

  assert(ch);
  assert((level >= 0) && (level <= ABS_MAX_LVL));

  CREATE(tmp_obj, struct obj_data, 1);
  clear_object(tmp_obj);

  tmp_obj->name = strdup("ball light");
  tmp_obj->short_description = strdup("A bright ball of light");
  tmp_obj->description = strdup("There is a bright ball of light on the ground here.");

  tmp_obj->obj_flags.type_flag = ITEM_LIGHT;
  tmp_obj->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD;
  tmp_obj->obj_flags.value[2] = 72;
  tmp_obj->obj_flags.weight = 1;
  tmp_obj->obj_flags.cost = 40;
  tmp_obj->obj_flags.cost_per_day = 1;

  tmp_obj->next = object_list;
  object_list = tmp_obj;

  obj_to_char(tmp_obj,ch);

  tmp_obj->item_number = -1;

  act("$n intones a power word and $p suddenly appears.",TRUE,ch,tmp_obj,0,TO_ROOM);
  act("You twiddle your thumbs and $p suddenly appears.",TRUE,ch,tmp_obj,0,TO_CHAR);

}

void spell_animate_dead(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *corpse)
{
	struct char_data *mob;
	struct obj_data *obj_object, *sub_object, *next_obj, *i;
	char buf[MAX_STRING_LENGTH];
	int r_num=100; /* virtual # for zombie */
	int k;
/*
 some sort of check for corpse hood
*/
        if ((GET_ITEM_TYPE(corpse)!=ITEM_CONTAINER)||
	    (!corpse->obj_flags.value[3])) {
	  send_to_char("The magic fails abruptly!\n\r",ch);
	  return;
	}

        mob = read_mobile(r_num, VIRTUAL);
      	char_to_room(mob, ch->in_room);

       	act("With mystic power, $n animates a corpse.", TRUE, ch,0,0,TO_ROOM);
       	act("$N slowly rises from the ground.", FALSE, ch, 0, mob, TO_ROOM);

/*
  zombie should be charmed and follower ch
*/

       	SET_BIT(mob->specials.affected_by, AFF_CHARM);
	GET_EXP(mob) = 300;
       	add_follower(mob, ch);
       	IS_CARRYING_W(mob) = 0;
       	IS_CARRYING_N(mob) = 0;

/*
        mob->killer = obj->killer;
*/
/*
  take all from corpse, and give to zombie 
*/

       	for (obj_object=corpse->contains; obj_object; obj_object=next_obj)
	{
     	    next_obj = obj_object->next_content;
       	    obj_from_obj(obj_object);
	    obj_to_char(obj_object, mob);
        }

/*
   set up descriptions and such
*/ 
    sprintf(buf,"%s is here, slowly animating\n\r",corpse->short_description);
    mob->player.long_descr = strdup(buf);

/*
  set up hitpoints
*/

	mob->points.max_hit = dice(3,8)+10;
	mob->points.hit = mob->points.max_hit;

	for (k=MAGE_LEVEL_IND;k<=RANGER_LEVEL_IND; k++)
    	   mob->player.level[k] = 3;

	mob->player.sex = 0;

	GET_RACE(mob) = RACE_UNDEAD;
	mob->player.class = ch->player.class;

/*
  get rid of corpse
*/
	extract_obj(corpse);
	GET_ALIGNMENT(ch)-=100;
	if(GET_ALIGNMENT(ch)<-999) GET_ALIGNMENT(ch)==-1000;
}

void spell_know_alignment(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
   int ap;
   char buf[200], name[100];

   assert(victim && ch);

   if (IS_NPC(victim))
      strcpy(name,victim->player.short_descr);
   else
      strcpy(name,GET_NAME(victim));
   
   ap = GET_ALIGNMENT(victim);
   
   if (ap > 700) 
      sprintf(buf,"%s has an aura as white as the driven snow.\n\r",name);
   else if (ap > 350)
      sprintf(buf, "%s is of excellent moral character.\n\r",name);
   else if (ap > 100)
      sprintf(buf, "%s is often kind and thoughtful.\n\r",name);
   else if (ap > 25)
      sprintf(buf, "%s isn't a bad sort...\n\r",name);
   else if (ap > -25)
      sprintf(buf, "%s doesn't seem to have a firm moral commitment\n\r",name);
   else if (ap > -100)
    sprintf(buf, "%s isn't the worst you've come across\n\r",name);
   else if (ap > -350)
    sprintf(buf, "%s could be a little nicer, but who couldn't?\n\r",name);
   else if (ap > -700)
    sprintf(buf, "%s probably just had a bad childhood\n\r",name);
   else 
     sprintf(buf,"I'd rather just not say anything at all about %s\n\r",name);

   send_to_char(buf,ch);
   
}

void spell_dispel_magic(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
   int yes=0;  

   assert(ch && victim);

/* gets rid of infravision, invisibility, detect, etc */

   if(!CheckKill(ch,victim)) return;

   if((GetMaxLevel(victim) <= GetMaxLevel(ch)))
      yes = TRUE;
   else 
     yes = FALSE;

    if (affected_by_spell(victim,SPELL_INVISIBLE)) 
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
         affect_from_char(victim,SPELL_INVISIBLE);
         send_to_char("You feel exposed.\n\r",victim);
    }
    if (affected_by_spell(victim,SPELL_DETECT_INVISIBLE))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
         affect_from_char(victim,SPELL_DETECT_INVISIBLE);
         send_to_char("You feel less perceptive.\n\r",victim);
    }
    if (affected_by_spell(victim,SPELL_DETECT_EVIL))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
         affect_from_char(victim,SPELL_DETECT_EVIL);
         send_to_char("You feel less morally alert.\n\r",victim);
    }
    if (affected_by_spell(victim,SPELL_DETECT_MAGIC))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_DETECT_MAGIC);
        send_to_char("You stop noticing the magic in your life.\n\r",victim);
    }
    if (affected_by_spell(victim,SPELL_SENSE_LIFE))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_SENSE_LIFE);
        send_to_char("You feel less in touch with living things.\n\r",victim);
    }
    if (affected_by_spell(victim,SPELL_SANCTUARY)) {
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_SANCTUARY);
        send_to_char("You don't feel so invulnerable anymore.\n\r",victim);
        act("The white glow around $n's body fades.",FALSE,victim,0,0,TO_ROOM);
      }
      /*
       *  aggressive Act.
       */
      if ((victim->attackers < 6) && (!victim->specials.fighting) &&
	    (IS_NPC(victim))) {
	  set_fighting(victim, ch);
	}
    }
    if (IS_AFFECTED(victim, AFF_SANCTUARY)) {
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
	REMOVE_BIT(victim->specials.affected_by, AFF_SANCTUARY);
	send_to_char("You don't feel so invulnerable anymore.\n\r",victim);
	act("The white glow around $n's body fades.",FALSE,victim,0,0,TO_ROOM);      }
      /*
       *  aggressive Act.
       */
      if ((victim->attackers < 6) && (!victim->specials.fighting) &&
	  (IS_NPC(victim))) {
	set_fighting(victim, ch);
      }
    }
    if (affected_by_spell(victim,SPELL_PROTECT_FROM_EVIL))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_PROTECT_FROM_EVIL);
        send_to_char("You feel less morally protected.\n\r",victim);
    }
    if (affected_by_spell(victim,SPELL_INFRAVISION))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_INFRAVISION);
        send_to_char("Your sight grows dimmer.\n\r",victim);
    }
    if (affected_by_spell(victim,SPELL_SLEEP))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_SLEEP);
        send_to_char("You don't feel so tired.\n\r",victim);
    }
    if (affected_by_spell(victim,SPELL_CHARM_PERSON))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_CHARM_PERSON);
        send_to_char("You feel less enthused about your master.\n\r",victim);
    }
    if (affected_by_spell(victim,SPELL_WEAKNESS))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_WEAKNESS);
        send_to_char("You don't feel so weak.\n\r",victim);
    }
    if (affected_by_spell(victim,SPELL_STRENGTH))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_STRENGTH);
        send_to_char("You don't feel so strong.\n\r",victim);
    }
    if (affected_by_spell(victim,SPELL_ARMOR))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_ARMOR);
        send_to_char("You don't feel so well protected.\n\r",victim);
    }
    if (affected_by_spell(victim,SPELL_DETECT_POISON))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_DETECT_POISON);
        send_to_char("You don't feel so sensitive to fumes.\n\r",victim);
    }
    
    if (affected_by_spell(victim,SPELL_BLESS))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_BLESS);
        send_to_char("You don't feel so blessed.\n\r",victim);
    }

    if (affected_by_spell(victim,SPELL_FLY))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_FLY);
        send_to_char("You don't feel lighter than air anymore.\n\r",victim);
    }

    if (affected_by_spell(victim,SPELL_WATER_BREATH))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_WATER_BREATH);
        send_to_char("You don't feel so fishy anymore.\n\r",victim);
    }

    if (affected_by_spell(victim,SPELL_FIRE_BREATH))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_FIRE_BREATH);
        send_to_char("You don't feel so fiery anymore.\n\r",victim);
    }
    if (affected_by_spell(victim,SPELL_LIGHTNING_BREATH))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_LIGHTNING_BREATH);
        send_to_char("You don't feel so electric anymore.\n\r",victim);
    }
    if (affected_by_spell(victim,SPELL_GAS_BREATH))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_GAS_BREATH);
        send_to_char("You don't have gas anymore.\n\r",victim);
    }
    if (affected_by_spell(victim,SPELL_FROST_BREATH))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_FROST_BREATH);
        send_to_char("You don't feel so frosty anymore.\n\r",victim);
    }
    if (affected_by_spell(victim,SPELL_FIRESHIELD))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_FIRESHIELD);
        send_to_char("You don't feel so firey anymore.\n\r",victim);
	act("The red glow around $n's body fades.", TRUE, ch, 0, 0, TO_ROOM);
    }
    if (affected_by_spell(victim,SPELL_FAERIE_FIRE))
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_FAERIE_FIRE);
        send_to_char("You don't feel so pink anymore.\n\r",victim);
	act("The pink glow around $n's body fades.", TRUE, ch, 0, 0, TO_ROOM);
    }


   if (level == IMPLEMENTOR)  {
    if (affected_by_spell(victim,SPELL_BLINDNESS)) {
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_BLINDNESS);
        send_to_char("Your vision returns.\n\r",victim);
      }
    }
    if (affected_by_spell(victim,SPELL_PARALYSIS)) {
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_PARALYSIS);
        send_to_char("You feel freedom of movement.\n\r",victim);
      }
    }
    if (affected_by_spell(victim,SPELL_POISON)) {
      if (yes || !saves_spell(victim, SAVING_SPELL) ) {
        affect_from_char(victim,SPELL_POISON);
      }
    }
   }
}

void spell_paralyze(byte level, struct char_data *ch,
		    struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;
  
  assert(victim);
  if(!CheckKill(ch,victim)) return;
  
  if(!IS_AFFECTED(victim, AFF_PARALYSIS)) 
  {
    if(IsImmune(victim, IMM_HOLD)) 
    {
      FailPara(victim, ch);
      return;
    }
    if(IsResist(victim, IMM_HOLD)) 
    {
      if(saves_spell(victim, SAVING_PARA)) 
      {
	FailPara(victim, ch);
	return;
      }
      if(saves_spell(victim, SAVING_PARA)) 
      {
	FailPara(victim, ch);
	return;
      }
    }
    else
    if(!IsSusc(victim, IMM_HOLD)) 
    {
      if(saves_spell(victim, SAVING_PARA)) 
      {
	FailPara(victim, ch);
	return;
      }    
    }
    
    af.type      = SPELL_PARALYSIS;
    af.duration  = dice(2,6);
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_PARALYSIS;
    affect_join(victim, &af, FALSE, FALSE);
    
    act("Your limbs freeze in place",FALSE,victim,0,0,TO_CHAR);
    act("$n is paralyzed!",TRUE,victim,0,0,TO_ROOM);
    GET_POS(victim)=POSITION_STUNNED;
  }
  else
  {
    send_to_char("Someone tries to paralyze you AGAIN!\n\r",victim);
  }
}

void spell_fear(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(victim && ch);
  if(!CheckKill(ch,victim)) return;
  
  if (GetMaxLevel(ch) >= GetMaxLevel(victim)-2) {
     if ( !saves_spell(victim, SAVING_SPELL))  {

/* 
        af.type      = SPELL_FEAR;
        af.duration  = 4+level;
        af.modifier  = 0;
        af.location  = APPLY_NONE;
	af.bitvector = 0;
        affect_join(victim, &af, FALSE, FALSE);
*/
	do_flee(victim, "", 0);

      } else {
	send_to_char("You feel afraid, but the effect fades.\n\r",victim);
	return;
      }
   }
}

void spell_prot_align_group(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
}

void spell_calm(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{  
  assert(ch && victim);
/* 
   removes aggressive bit from monsters 
*/

  if (IS_NPC(victim)) 
  {
    if (IS_SET(victim->specials.act, ACT_AGGRESSIVE)) 
    {
      if (!saves_spell(victim, SAVING_PARA)) 
      {
          REMOVE_BIT(victim->specials.act, ACT_AGGRESSIVE);
	}
	else
	{
	  FailCalm(victim, ch);
	}
     }
     else
     {
       send_to_char("You feel calm\n\r", victim);
     }
  }
  else
  {
    send_to_char("You feel calm.\n\r", victim);
  }
}

void spell_conjure_elemental(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  /*
   *   victim, in this case, is the elemental
   *   object could be the sacrificial object
   */

   assert(ch && victim && obj);

   /*
   ** objects:
   **     fire  : red stone
   **     water : pale blue stone
   **     earth : grey stone
   **     air   : clear stone
   */

   act("$n gestures, and a cloud of smoke appears", TRUE, ch, 0, 0, TO_ROOM);
   act("$n gestures, and a cloud of smoke appears", TRUE, ch, 0, 0, TO_CHAR);
   act("$p explodes with a loud BANG!", TRUE, ch, obj, 0, TO_ROOM);
   act("$p explodes with a loud BANG!", TRUE, ch, obj, 0, TO_CHAR);
   obj_from_char(obj);
   extract_obj(obj);
   char_to_room(victim, ch->in_room);
   act("Out of the smoke, $N emerges", TRUE, ch, 0, victim, TO_NOTVICT);

   /* charm them for a while */
    if (victim->master)
      stop_follower(victim);

    add_follower(victim, ch);

    af.type      = SPELL_CHARM_PERSON;
    af.duration  = 24;
    af.modifier  = 0;
    af.location  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char(victim, &af);
}

void spell_faerie_fire (byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(ch && victim);
  if(!CheckKill(ch,victim)) return;

  if (affected_by_spell(victim, SPELL_FAERIE_FIRE)) 
  {
    send_to_char("Nothing new seems to happen",ch);
    return;
  }

  act("$n points at $N.", TRUE, ch, 0, victim, TO_ROOM);
  act("You point at $N.", TRUE, ch, 0, victim, TO_CHAR);
  act("$N is surrounded by a pink outline", TRUE, ch, 0, victim, TO_ROOM);
  act("$N is surrounded by a pink outline", TRUE, ch, 0, victim, TO_CHAR);

  af.type      = SPELL_FAERIE_FIRE;
  af.duration  = level/3;
  af.modifier  = 10;
  af.location  = APPLY_ARMOR;
  af.bitvector = 0;

  affect_to_char(victim, &af);

}

void spell_faerie_fog (byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;
   struct char_data *tmp_victim;

  assert(ch);

  act("$n snaps $s fingers, and a cloud of purple smoke billows forth",
      TRUE, ch, 0, 0, TO_ROOM);
  act("You snap your fingers, and a cloud of purple smoke billows forth",
      TRUE, ch, 0, 0, TO_CHAR);


   for ( tmp_victim = real_roomp(ch->in_room)->people; tmp_victim; 
	tmp_victim = tmp_victim->next_in_room) {
      if ( (ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
         if (IS_IMMORTAL(tmp_victim))
            break;
         if (!in_group(ch, tmp_victim)) {
	   if (IS_AFFECTED(tmp_victim, AFF_INVISIBLE)) {
            if ( saves_spell(tmp_victim, SAVING_SPELL) ) {
	      REMOVE_BIT(tmp_victim->specials.affected_by, AFF_INVISIBLE);
	      act("$n is briefly revealed, but dissapears again.",
		  TRUE, tmp_victim, 0, 0, TO_ROOM);
	      act("You are briefly revealed, but dissapear again.",
		  TRUE, tmp_victim, 0, 0, TO_CHAR);
	      SET_BIT(tmp_victim->specials.affected_by, AFF_INVISIBLE);
	    } else {
	      REMOVE_BIT(tmp_victim->specials.affected_by, AFF_INVISIBLE);
	      act("$n is revealed!",
		  TRUE, tmp_victim, 0, 0, TO_ROOM);
	      act("You are revealed!",
		  TRUE, tmp_victim, 0, 0, TO_CHAR);
	    }
	   }
	 }
       }
    }
}



void spell_cacaodemon(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(ch && victim && obj);

   act("$n gestures, and a black cloud of smoke appears", TRUE, ch, 0, 0, TO_ROOM);
   act("$n gestures, and a black cloud of smoke appears", TRUE, ch, 0, 0, TO_CHAR);
   act("$p bursts into flame and disintegrates!", TRUE, ch, obj, 0, TO_ROOM);
   act("$p bursts into flame and disintegrates!", TRUE, ch, obj, 0, TO_CHAR);
   obj_from_char(obj);
   extract_obj(obj);
   char_to_room(victim, ch->in_room);

   act("With an evil laugh, $N emerges from the smoke", TRUE, ch, 0, victim, TO_NOTVICT);

   /* charm them for a while */
    if (victim->master)
      stop_follower(victim);

    add_follower(victim, ch);

    af.type      = SPELL_CHARM_PERSON;
    af.duration  = level;
    af.modifier  = 0;
    af.location  = 0;
    af.bitvector = AFF_CHARM;

    affect_to_char(victim, &af);

    if(IS_SET(victim->specials.act, ACT_AGGRESSIVE))
      REMOVE_BIT(victim->specials.act, ACT_AGGRESSIVE);

    if(!IS_SET(victim->specials.act, ACT_SENTINEL))
      SET_BIT(victim->specials.act, ACT_SENTINEL);

}

/*
 neither
*/

void spell_improved_identify(byte level, struct char_data *ch,
   struct char_data *victim, struct obj_data *obj)
{
}

void spell_geyser(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;

  struct char_data *tmp_victim, *temp;

  if (ch->in_room<0)
    return;
	dam =  dice(level,3);

  act("The Geyser erupts in a huge column of steam!\n\r",
	  FALSE, ch, 0, 0, TO_ROOM);


   for(tmp_victim = real_roomp(ch->in_room)->people; tmp_victim; tmp_victim = temp) {
      temp = tmp_victim->next_in_room;
      if ((ch != tmp_victim) && (ch->in_room == tmp_victim->in_room)) {
            if ((GetMaxLevel(tmp_victim)<LOW_IMMORTAL)||(IS_NPC(tmp_victim))) {
      	       damage(ch, tmp_victim, dam, SPELL_GEYSER);
               act("You are seared by the boiling water!!\n\r",
                   FALSE, ch, 0, tmp_victim, TO_VICT);
	    } else {
               act("You are almost seared by the boiling water!!\n\r",
                 FALSE, ch, 0, tmp_victim, TO_VICT);
	    }
      }
    }
}

void spell_green_slime(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int dam;
	int hpch;

	assert(victim && ch);
	assert((level >= 1) && (level <= ABS_MAX_LVL)); 

	hpch = GET_MAX_HIT(ch);
	if(hpch<10) hpch=10;

	dam = (int)(hpch/10);

	if ( saves_spell(victim, SAVING_BREATH) )
		dam >>= 1;

	send_to_char("You are attacked by green slime!\n\r",victim);

	damage(ch, victim, dam, SPELL_GREEN_SLIME);

}



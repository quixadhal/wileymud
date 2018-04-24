/*
**  magicutils -- stuff that makes the magic files easier to read.
*/

#include <stdio.h>
#include <assert.h>
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "spells.h"
#include "handler.h"
#include "limits.h"

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
void set_fighting(struct char_data *ch, struct char_data *vict);
bool ImpSaveSpell(struct char_data *ch, sh_int save_type, int mod);
int IsPerson( struct char_data *ch);
int IsExtraPlanar( struct char_data *ch);


SwitchStuff( struct char_data *giver, struct char_data *taker)
{
  struct obj_data *obj, *next;
  float ratio;
  int j;
  
  /*
   *  take all the stuff from the giver, put in on the
   *  taker
   */
  
  for (j = 0; j< MAX_WEAR; j++) 
  {
    if (giver->equipment[j]) 
    {
      obj = unequip_char(giver, j); equip_char(taker, obj, j);
    }
  }
  
  for (obj = giver->carrying; obj; obj = next) 
  {
    next = obj->next_content;
    obj_from_char(obj);
    obj_to_char(obj, taker);
  }
  
  /*
   *    gold...
   */

  GET_GOLD(taker) = GET_GOLD(giver);
  
  /*
   *   hit point ratio
   */

  GET_HIT(taker) = GET_MAX_HIT(taker);

/*
 * experience

  if(!IS_IMMORTAL(taker)) 
  {
     if (!IS_IMMORTAL(giver))
        GET_EXP(taker) = GET_EXP(giver);

     GET_EXP(taker) = MIN(GET_EXP(taker),100);
  }

 * humanoid monsters can cast spells

  if(IS_NPC(taker)) 
  {
    taker->player.class = giver->player.class;
    for (j = 0; j< MAX_SKILLS; j++) 
    {
      taker->skills[j].learned = giver->skills[j].learned;
      taker->skills[j].recognise = giver->skills[j].recognise;
    }
    for (j = 0;j<=3;j++) 
    {
      taker->player.level[j] = giver->player.level[j];
    }
  }
*/
  GET_MANA(taker) = GET_MANA(giver);
}


FailCharm(struct char_data *victim, struct char_data *ch)
{
  if (IS_NPC(victim)) {
    if (!victim->specials.fighting) {
      set_fighting(victim,ch);
    } 
  } else {
    send_to_char("You feel charmed, but the feeling fades.\n\r",victim);
  }
}

FailSleep(struct char_data *victim, struct char_data *ch)
{
  
  send_to_char("You feel sleepy for a moment,but then you recover\n\r",victim);
  if (IS_NPC(victim))
    if ((!victim->specials.fighting) && (GET_POS(victim) > POSITION_SLEEPING))
      set_fighting(victim,ch);
}

FailPara(struct char_data *victim, struct char_data *ch)
{
  send_to_char("You feel frozen for a moment,but then you recover\n\r",victim);
  if(IS_NPC(victim))
    if((!victim->specials.fighting) && (GET_POS(victim) > POSITION_SLEEPING))
      set_fighting(victim,ch);
}

FailCalm(struct char_data *victim, struct char_data *ch)
{
  send_to_char("You feel happy but the effect soon fades.\n\r",victim);
  if (IS_NPC(victim))
    if (!victim->specials.fighting)
      set_fighting(victim,ch);
}


void AreaEffectSpell(struct char_data *castor,int dam,int spell_type,int zflag,char *zone_mesg)
{
  struct char_data *victim,*next_victim;

  for(victim = character_list;victim;victim = next_victim)
  {
    next_victim = victim->next;
    if((castor->in_room == victim->in_room) && castor != victim)
    {
      if((IS_MORTAL(victim) || IS_NPC(victim)) && CheckKill(castor,victim))
      {
	switch(spell_type)
	{
	  case SPELL_EARTHQUAKE:
	    damage(castor,victim,dam,spell_type);
	    act("You fall and hurt yourself!!\n\r",FALSE,castor,0,victim,TO_VICT);
            GET_POS(victim) = POSITION_SITTING;
            WAIT_STATE(victim,(PULSE_VIOLENCE*3));
          break; 
	  case SPELL_BURNING_HANDS:
            act("You are seared by the burning flame!\n\r",FALSE,castor,0,victim,TO_VICT);
            if(saves_spell(victim, SAVING_SPELL))
              dam >>= 1;
            damage(castor,victim,dam,spell_type);
          break;
          case SPELL_FIREBALL:
            if(saves_spell(victim,SAVING_SPELL))
              dam >>= 1;
            damage(castor,victim,dam,spell_type);
          break;
          case SPELL_CONE_OF_COLD:
            act("You are chilled to the bone!\n\r",FALSE,castor,0,victim,TO_VICT);
            if(saves_spell(victim, SAVING_SPELL) )                                
              dam >>= 1;                                                              
            damage(castor,victim,dam,spell_type);
          break;
          case SPELL_ICE_STORM:
            act("You are blasted by the storm\n\r",FALSE, castor, 0, victim, TO_VICT);
            if(saves_spell(victim, SAVING_SPELL) )
              dam >>= 1;
            damage(castor,victim, dam,SPELL_ICE_STORM);
          break;
        }
      }
    }
    else
    {
      if(zflag)
      {
	/*
        if(GET_ZONE(castor->in_room) == GET_ZONE(victim->in_room))
        {
          send_to_char(zone_mesg,victim);
        }
	*/
      }
    }
  }
}

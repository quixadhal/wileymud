/*
 *  magicutils -- stuff that makes the magic files easier to read.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <assert.h>

#include "include/global.h"
#include "include/bug.h"
#include "include/utils.h"
#include "include/comm.h"
#include "include/db.h"
#include "include/spells.h"
#include "include/handler.h"
#include "include/mudlimits.h"
#include "include/fight.h"
#include "include/spell_parser.h"
#include "include/multiclass.h"
#define _MAGIC_UTILS_C
#include "include/magic_utils.h"

void SwitchStuff(struct char_data *giver, struct char_data *taker)
{
  struct obj_data *obj, *next;
  int j;

  /*
   *  take all the stuff from the giver, put in on the
   *  taker
   */

  for (j = 0; j < MAX_WEAR; j++) {
    if (giver->equipment[j]) {
      obj = unequip_char(giver, j);
      equip_char(taker, obj, j);
    }
  }

  for (obj = giver->carrying; obj; obj = next) {
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

void FailCharm(struct char_data *victim, struct char_data *ch)
{
  if (IS_NPC(victim)) {
    if (!victim->specials.fighting) {
      set_fighting(victim, ch);
    }
  } else {
    cprintf(victim, "You feel charmed, but the feeling fades.\n\r");
  }
}

void FailSleep(struct char_data *victim, struct char_data *ch)
{

  cprintf(victim, "You feel sleepy for a moment,but then you recover\n\r");
  if (IS_NPC(victim))
    if ((!victim->specials.fighting) && (GET_POS(victim) > POSITION_SLEEPING))
      set_fighting(victim, ch);
}

void FailPara(struct char_data *victim, struct char_data *ch)
{
  cprintf(victim, "You feel frozen for a moment,but then you recover\n\r");
  if (IS_NPC(victim))
    if ((!victim->specials.fighting) && (GET_POS(victim) > POSITION_SLEEPING))
      set_fighting(victim, ch);
}

void FailCalm(struct char_data *victim, struct char_data *ch)
{
  cprintf(victim, "You feel happy but the effect soon fades.\n\r");
  if (IS_NPC(victim))
    if (!victim->specials.fighting)
      set_fighting(victim, ch);
}

void AreaEffectSpell(struct char_data *castor, int dam, int spell_type, int zflag, char *zone_mesg)
{
  struct char_data *victim, *next_victim;

  for (victim = character_list; victim; victim = next_victim) {
    next_victim = victim->next;
    if ((castor->in_room == victim->in_room) && castor != victim) {
      if (IS_IMMORTAL(victim)) continue;
      if (!CheckKill(castor, victim)) continue;
      if (IS_AFFECTED(victim, AFF_GROUP))
        if ((victim == castor->master) ||
            (victim->master == castor) ||
            (victim->master == castor->master)) continue;
	switch (spell_type) {
	case SPELL_EARTHQUAKE:
	  damage(castor, victim, dam, spell_type);
	  act("You fall and hurt yourself!!\n\r", FALSE, castor, 0, victim, TO_VICT);
	  GET_POS(victim) = POSITION_SITTING;
	  WAIT_STATE(victim, (PULSE_VIOLENCE * 3));
	  break;
	case SPELL_BURNING_HANDS:
	  act("You are seared by the burning flame!\n\r", FALSE, castor, 0, victim, TO_VICT);
	  if (saves_spell(victim, SAVING_SPELL))
	    dam >>= 1;
	  damage(castor, victim, dam, spell_type);
	  break;
	case SPELL_FIREBALL:
	  if (saves_spell(victim, SAVING_SPELL))
	    dam >>= 1;
	  damage(castor, victim, dam, spell_type);
	  break;
	case SPELL_CONE_OF_COLD:
	  act("You are chilled to the bone!\n\r", FALSE, castor, 0, victim, TO_VICT);
	  if (saves_spell(victim, SAVING_SPELL))
	    dam >>= 1;
	  damage(castor, victim, dam, spell_type);
	  break;
	case SPELL_ICE_STORM:
	  act("You are blasted by the storm\n\r", FALSE, castor, 0, victim, TO_VICT);
	  if (saves_spell(victim, SAVING_SPELL))
	    dam >>= 1;
	  damage(castor, victim, dam, SPELL_ICE_STORM);
	  break;
        case SPELL_METEOR_SWARM:
          if (saves_spell(victim, SAVING_SPELL)) {
            act("You dive for cover but are still burned by the flames all around you.\n\r", FALSE, castor, 0, victim, TO_VICT);
            damage(castor, victim, dam/2, spell_type);
          } else {
            act("You scream in agony as a ball of flame goes through you!\n\r", FALSE, castor, 0, victim, TO_VICT);
            damage(castor, victim, dam, spell_type);
          }
          break;
	}
    } else {
      if (zflag) {
	/*
	 * if(GET_ZONE(castor->in_room) == GET_ZONE(victim->in_room))
	 * {
	 * cprintf(victim, zone_mesg);
	 * }
	 */
      }
    }
  }
}

/* ************************************************************************
*  file: mobact.c , mobile action module.                 part of dikumud *
*  usage: procedures generating 'intelligent' behavior in the mobiles.    *
*  copyright (c) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>

#include "structs.h"
#include "utils.h"
#include "handler.h"
#include "db.h"
#include "comm.h"
#include "opinion.h"
#include "trap.h"

extern struct char_data *character_list;
extern struct index_data *mob_index;
extern struct hash_header room_db;
extern struct str_app_type str_app[];
extern struct index_data *mob_index;

void hit(struct char_data *ch, struct char_data *victim, int type);
struct char_data *FindVictim( struct char_data *ch);
int SameRace( struct char_data *ch1, struct char_data *ch2);
char in_group ( struct char_data *ch1, struct char_data *ch2);
int dir_track( struct char_data *ch, struct char_data *vict);
int IsHumanoid( struct char_data *ch);
struct char_data *FindAnyVictim( struct char_data *ch);

void mobile_guardian(struct char_data *ch)
{
  if (ch->in_room > -1) 
  {
    if ((!ch->master) || (!IS_AFFECTED(ch, AFF_CHARM)))
      return;
    if (ch->master->specials.fighting) 
    {
      if (!SameRace(ch->master->specials.fighting, ch)) 
      {
	if (IsHumanoid(ch)) 
        {
	  act("$n screams 'I must protect my master!'",FALSE,ch,0,0,TO_ROOM);
	}
        else 
        {
	  act("$n growls angrily!",FALSE, ch, 0, 0, TO_ROOM);
	}
	if (CAN_SEE(ch, ch->master->specials.fighting))
	  hit(ch, ch->master->specials.fighting,0);	  
      }
    }
  }
}
  
  void mobile_wander(struct char_data *ch)
    {
      int	door;
      struct room_direction_data	*exitp;
      struct room_data	*rp;
      
      if(RIDDEN(ch))
      {
	if(RIDDEN(ch)->specials.fighting)
	  return;
        if(IS_AFFECTED(ch,AFF_CHARM))
	  return;

      }

      if (! ((GET_POS(ch) == POSITION_STANDING) &&
	     ((door = number(0, 15)) <= 5) &&
	     exit_ok(exitp=EXIT(ch,door), &rp) &&
	     !IS_SET(rp->room_flags, NO_MOB) &&
	     !IS_SET(rp->room_flags, DEATH))
	  )
	return;
      
      if (IsHumanoid(ch) ? CAN_GO_HUMAN(ch, door) : CAN_GO(ch, door)) {
	if (ch->specials.last_direction == door) {
	  ch->specials.last_direction = -1;
	} else {
	  if (!IS_SET(ch->specials.act, ACT_STAY_ZONE) ||
	      (rp->zone == real_roomp(ch->in_room)->zone)) {
	    ch->specials.last_direction = door;
	    go_direction(ch, door);
	  }
	}
      }
    }
  
void MobHunt(struct char_data *ch)
{
  int res, k;
      
  if(ch->persist <= 0) 
  {
    res = choose_exit(ch->in_room, ch->old_room, 2000);
    if (res > -1) 
    {
      go_direction(ch, res);
    }
    else
    {
      if (ch->specials.hunting) 
      {
        if (ch->specials.hunting->in_room == ch->in_room) 
	{
	  if (Hates(ch, ch->specials.hunting) && 
	    (!IS_AFFECTED(ch->specials.hunting, AFF_HIDE))) 
	  {
	    if (check_peaceful(ch, "You'd CAN'T fight here!\n\r")) 
	    {
	      act("$n fumes at $N",TRUE,ch,0,ch->specials.hunting,TO_ROOM); 
	    }
            else
	    {
	      if (IsHumanoid(ch)) 
	      {
	        act("$n screams 'Time to die, $N'", 
                     TRUE, ch, 0, ch->specials.hunting, TO_ROOM); 
              }
	      else
		if (IsAnimal(ch))
		{
	          act("$n growls.", TRUE, ch, 0, 0, TO_ROOM);
	        }
	      hit(ch,ch->specials.hunting,0);
	      return;
            }
	  }
	}
      }
      REMOVE_BIT(ch->specials.act, ACT_HUNTING);
      ch->specials.hunting = 0;
      ch->hunt_dist = 0;
    }
  }
  else
  if (ch->specials.hunting) 
  {
    if (ch->hunt_dist <= 50) 
        ch->hunt_dist = 50;
    for (k=1;k<=2 && ch->specials.hunting; k++) 
    {
      ch->persist -= 1;
      res = dir_track(ch, ch->specials.hunting);
      if (res != -1) 
      {
          go_direction(ch, res);
      }
      else
      {
          ch->persist = 0;
          ch->specials.hunting = 0;
          ch->hunt_dist = 0;
      }
    }
  }
  else
  {
    ch->persist = 0;
  }	       
}
  
  void MobScavenge(struct char_data *ch)
    {
      struct obj_data *best_obj, *obj;
      int max;
      
      if ((real_roomp(ch->in_room))->contents && !number(0,5)) 
      {
	for (max = 1,best_obj = 0,obj = (real_roomp(ch->in_room))->contents;
	     obj; obj = obj->next_content) 
	{
	  if (CAN_GET_OBJ(ch, obj)) 
	  {
	    if (obj->obj_flags.cost > max) 
	    {
	      best_obj = obj;
	      max = obj->obj_flags.cost;
	    }
	  }
	} /* for */
	
	if (best_obj) 
	{
	  if (CheckForAnyTrap(ch, best_obj))
	    return;
	  
	  obj_from_room(best_obj);
	  obj_to_char(best_obj, ch);
	  act("$n gets $p.",FALSE,ch,best_obj,0,TO_ROOM);

	  if(IS_SET(ch->specials.act,ACT_USE_ITEM))
	  {
	    switch(GET_ITEM_TYPE(best_obj))
	    {
	      case ITEM_WEAPON:
	      {
	        if(!ch->equipment[WIELD] && !ch->equipment[WIELD_TWOH] )
		{
		  do_wield(ch,best_obj->name,0);
                }
              }
	      break;

	      case ITEM_ARMOR:
	      {
		do_wear(ch,best_obj->name,0);
              }
	      break;
            }
          }
	}
      }
    }
  
  
  void mobile_activity(void)
    {
      register struct char_data *ch, *tmp_ch;
      struct char_data *damsel, *targ;
      struct obj_data *obj, *best_obj, *worst_obj;
      int door, found, max, min, t, res, k;
      char buf[80];
      extern int no_specials;
      
      void do_move(struct char_data *ch, char *argument, int cmd);
      void do_get(struct char_data *ch, char *argument, int cmd);
      
      for (ch = character_list; ch; ch = ch->next)
	if (IS_MOB(ch))	
	{
	  /* Examine call for special procedure */
	  /* some status checking for errors */

	  if ((ch->in_room < 0) || !hash_find(&room_db,ch->in_room)) 
	  {
	    log("Char not in correct room.  moving to 3 ");
	    char_from_room(ch);
	    char_to_room(ch, 3);
	  }
	  
	  if (IS_SET(ch->specials.act, ACT_SPEC) && !no_specials) 
	  {
	    if (!mob_index[ch->nr].func) 
	    {
	      log("Attempting to call a non-existing MOB func. (mobact.c)");
	      log(ch->player.name);
	      REMOVE_BIT(ch->specials.act, ACT_SPEC);
	    }
	    else 
	    {
	      if ((*mob_index[ch->nr].func) (ch, 0, "")) continue;
	    }
	  }
	  
	  
	  /* check to see if the monster is possessed */
	  
	  if(AWAKE(ch) && (!ch->specials.fighting) && (!ch->desc) &&
	      (!IS_SET(ch->specials.act, ACT_POLYSELF))) 
	  {
	    AssistFriend(ch);

	    if (IS_SET(ch->specials.act, ACT_SCAVENGER)) 
	    {
	      MobScavenge(ch);
	    } /* Scavenger */
	    
	    if (IS_SET(ch->specials.act, ACT_HUNTING)) 
	    {
	      MobHunt(ch);
	    }
	    else
	      if ((!IS_SET(ch->specials.act, ACT_SENTINEL)))
	        mobile_wander(ch);
	    
	    if (GET_HIT(ch) > (GET_MAX_HIT(ch)/3)) 
	    {
	      if (IS_SET(ch->specials.act, ACT_HATEFUL)) 
	      {
		tmp_ch = FindAHatee(ch);
		if (tmp_ch) 
		{
		  if (check_peaceful(ch, "You ask your enemy to step outside.\n\r")) 
		  {
		    if(IsHumanoid(ch))
		      act("$n growls '$N, would you care to step outside?'",
			TRUE, ch, 0, tmp_ch, TO_ROOM);
                    else
                    if(IsAnimal(ch))
		      act("$n snarls at $N...",
			TRUE, ch, 0, tmp_ch, TO_ROOM);
			
		  } 
		  else 
		  {
		    if (IsHumanoid(ch)) 
		    {
		      act("$n screams 'I'm gonna kill you!'",TRUE,ch,0,0,TO_ROOM); 
		    }
		    else
		    if (IsAnimal(ch)) 
		    {
		      act("$n growls", TRUE, ch, 0, 0, TO_ROOM);
		    }
		    hit(ch,tmp_ch,0);
		  }
		}
	      }
	      if (!ch->specials.fighting) 
	      {
		if (IS_SET(ch->specials.act, ACT_AFRAID)) 
		{
		  if ((tmp_ch = FindAFearee(ch))!= NULL) 
		  {
		    do_flee(ch, "", 0);
		  }
		}
	      }
	    }
	    else
	    { 
	      if (IS_SET(ch->specials.act, ACT_AFRAID)) 
	      {
		if ((tmp_ch = FindAFearee(ch))!= NULL) 
		{
		  do_flee(ch, "", 0);
		}
		else
		{
		  if (IS_SET(ch->specials.act, ACT_HATEFUL)) 
		  {
		    tmp_ch = FindAHatee(ch);
		    if (tmp_ch) 
		    {
		      if (check_peaceful(ch, "You ask your enemy to step outside.\n\r"))
		      {
			act("$n growls '$N, would you care to step outside?'",
			    TRUE, ch, 0, tmp_ch, TO_ROOM);
		      }
		      else
		      {
			if (IsHumanoid(ch)) 
			{
			  act("$n screams 'I'm gonna get you!'",TRUE,ch,0,0,TO_ROOM); 
			}
			else
			if (IsAnimal(ch)) 
			{
			  act("$n growls", TRUE, ch, 0, 0, TO_ROOM);
			}
			hit(ch,tmp_ch,0);
		      }
		    }
		  }	 		
		}
	      }
	    }

	    if (IS_SET(ch->specials.act,ACT_AGGRESSIVE)) 
	    {
	      for (k=0;k<=5;k++) 
	      {
		tmp_ch = FindVictim(ch);
		if (tmp_ch) 
		{
		  if (check_peaceful(ch,
		    "You can't seem to exercise your violent tendencies.\n\r")) 
		  {
		    act("$n growls impotently", TRUE, ch, 0, 0, TO_ROOM);
		    return;
		  }
		  hit(ch, tmp_ch, 0);
		  k = 10;
		}
	      }
	    }

	    if (IS_SET(ch->specials.act, ACT_GUARDIAN)) 
	      mobile_guardian(ch);
	    
	    
	  } /* If AWAKE(ch)   */
	}   /* If IS_MOB(ch)  */
    }
  
  int SameRace( struct char_data *ch1, struct char_data *ch2)
    {
      
      if ((!ch1) || (!ch2))
	return(FALSE);
      
      if (ch1 == ch2)
	return(TRUE);
      
      if (IS_NPC(ch1) && (IS_NPC(ch2)))
      {
	if (mob_index[ch1->nr].virtual == mob_index[ch2->nr].virtual) 
	  return (TRUE);
        else
          return (FALSE);
      }
      
      if (in_group(ch1,ch2))
	return(TRUE);
      
      if (GET_RACE(ch1) == GET_RACE(ch2)) {
	return(TRUE);
      }
      
      return(FALSE);
      
      
    }
  
  int AssistFriend( struct char_data *ch)
    {
      struct char_data *damsel, *targ, *tmp_ch;
      int t, found;
      
      
      damsel = 0;
      targ = 0;
      
      if (check_peaceful(ch, ""))
	return;
      
      /*
	find the people who are fighting
	*/
      
      for (tmp_ch=(real_roomp(ch->in_room))->people;tmp_ch;
	   tmp_ch=tmp_ch->next_in_room) {
	if (CAN_SEE(ch,tmp_ch)) {
	  if (!IS_SET(ch->specials.act, ACT_WIMPY)) {
	    if (IS_NPC(tmp_ch) && (SameRace(tmp_ch,ch))) {
	      if (tmp_ch->specials.fighting)
		damsel = tmp_ch;
	    }
	  }
	}
      }
      
      if (damsel) {
	/*
	  check if the people in the room are fighting.
	  */
	found = FALSE;
	for (t=1; t<=8 && !found;t++) 
	{
	  targ = FindAnyVictim(damsel);
	  if (targ) 
	  {
	    if (targ->specials.fighting)
	      if (SameRace(targ->specials.fighting, ch))
		found = TRUE;
	  }
	}

	if (targ)
	  if (targ->in_room == ch->in_room) 
	  {
	    if (!IS_AFFECTED(ch, AFF_CHARM) || ch->master != targ) 
	    {
	      hit(ch,targ,0);
	    }
	  }
      }
    }

/* ************************************************************************
 *  file: act.movement.c , Implementation of commands      Part of DIKUMUD *
 *  Usage : Movement commands, close/open & lock/unlock doors.             *
 *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
 ************************************************************************* */

#include <stdio.h>
#include <string.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "trap.h"

/*   external vars  */

extern int DEBUG;
extern struct hash_header room_db;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list; 
extern struct index_data *obj_index;
extern int rev_dir[];
extern char *dirs[]; 
extern int movement_loss[];
extern struct wis_app_type wis_app[26];
/* external functs */

int special(struct char_data *ch, int cmd, char *arg);
void death_cry(struct char_data *ch);
struct obj_data *get_obj_in_list_vis(struct char_data *ch, char *name,
				     struct obj_data *list);
void zero_rent(struct char_data *ch);


/*
  Some new movement commands for diku-mud.. a bit more object oriented.
  */


void NotLegalMove(struct char_data *ch)
{
  if(DEBUG) dlog("NotLegalMove");
  send_to_char("Alas, you cannot go that way...\n\r", ch);
}

int check_exit_alias(struct char_data *ch,char *argument)
{
  struct room_direction_data *exitp;
  char buf[512];
  int index;

  int do_move(struct char_data *ch,char *arg,int cmd);
  return 0;
  if(DEBUG) dlog("check_exit_alias");
  for(index=0;index<6;index++)
  {
    if(exitp=EXIT(ch,index))
    {
      if (IS_SET(exitp->exit_info,EX_ALIAS))
        if (!strncmp(exitp->exit_alias,argument,strlen(exitp->exit_alias)))
	  return(do_move(ch,"",index+1));
    }
  }
  return 0;
}


int ValidMove( struct char_data *ch, int cmd) 
{
  char tmp[256];
  struct room_direction_data	*exitp;
  struct room_data *here, *there;
  if(DEBUG) dlog("ValidMove");  
  exitp = EXIT(ch, cmd);

  if(!exit_ok(exitp,NULL)) 
  {
    NotLegalMove(ch);
    return(FALSE);
  }

  there = real_roomp(exitp->to_room);

  if (MOUNTED(ch))
  {
    if (GET_POS(MOUNTED(ch)) < POSITION_FIGHTING) 
    {
      send_to_char("Your mount must be standing\n\r", ch);
      return(FALSE);
    }
    if(ch->in_room != MOUNTED(ch)->in_room) 
    {
      Dismount(ch, MOUNTED(ch), POSITION_STANDING);
    }
  }

  if(IS_SET(exitp->exit_info, EX_CLOSED)) 
  {
    if (exitp->keyword) 
    {
      if (IS_NOT_SET(exitp->exit_info, EX_SECRET) &&
	  (strcmp(fname(exitp->keyword), "secret"))) 
      {
        sprintf(tmp, "The %s seems to be closed.\n\r",fname(exitp->keyword));
        send_to_char(tmp, ch);
        return(FALSE);
      }
      else 
      {
        NotLegalMove(ch);
        return(FALSE);
      }
    }
    else
    {
      NotLegalMove(ch);
      return(FALSE);
    }
  }
  else
  {
    if(IS_SET(there->room_flags, INDOORS)) 
    {
      if (MOUNTED(ch)) 
      {
        send_to_char("Your mount refuses to go that way\n\r", ch);
        return(FALSE);
      }
    }  
    if(IS_SET(there->room_flags, DEATH)) 
    {
      if (MOUNTED(ch)) 
      {
        send_to_char("Your mount refuses to go that way\n\r", ch);
        return(FALSE);
      }
    }  

    return(TRUE);
  }
}

int RawMove(struct char_data *ch, int dir)
{
  int need_movement;
  struct obj_data *obj;
  bool has_boat;
  struct room_data *from_here, *to_here;
  struct char_data *pers;
  int amount;
  int special(struct char_data *ch, int dir, char *arg);
  if(DEBUG) dlog("RawMove");
  
  if (special(ch, dir+1, ""))/* Check for special routines(North is 1)*/
    return(FALSE);
  
  if (!ValidMove(ch, dir)) {
    return(FALSE); 
  }
  
  if(IS_AFFECTED(ch, AFF_CHARM) && (ch->master) && (ch->in_room == ch->master->in_room)) 
  {
    act("$n bursts into tears.", FALSE, ch, 0, 0, TO_ROOM);
    act("You burst into tears at the thought of leaving $N",FALSE, ch, 0, ch->master, TO_CHAR);
    return(FALSE);
  }

  amount = 0;
  if(IS_AFFECTED(ch,AFF_SNEAK))
  {
    amount = 2;

    if((GET_LEVEL(ch,BestThiefClass(ch))>=1) &&
       (GET_LEVEL(ch,BestThiefClass(ch))<5))
      amount += 13;
    else
    if((GET_LEVEL(ch,BestThiefClass(ch))>=5) &&
       (GET_LEVEL(ch,BestThiefClass(ch))<10))
      amount += 11;
    else
    if((GET_LEVEL(ch,BestThiefClass(ch))>=10) &&
       (GET_LEVEL(ch,BestThiefClass(ch))<15))
      amount += 9;
    else
    if((GET_LEVEL(ch,BestThiefClass(ch))>=15) &&
       (GET_LEVEL(ch,BestThiefClass(ch))<20))
      amount += 7; 
    else
    if((GET_LEVEL(ch,BestThiefClass(ch))>=20) &&
       (GET_LEVEL(ch,BestThiefClass(ch))<25))
      amount += 5; 
    else
    if((GET_LEVEL(ch,BestThiefClass(ch))>=25) &&
       (GET_LEVEL(ch,BestThiefClass(ch))<30))
      amount += 3; 
    else
    if((GET_LEVEL(ch,BestThiefClass(ch))>=30) &&
       (GET_LEVEL(ch,BestThiefClass(ch))<40))
      amount += 1; 
    
    amount -= MAX(2,wis_app[GET_DEX(ch)].bonus);

    if(amount<=0) amount = 1;

    GET_MANA(ch) -= amount;
    if(GET_MANA(ch) <=0)
    {
      affect_from_char(ch,SKILL_SNEAK);
      GET_MANA(ch)=0;
    }
  }

  from_here = real_roomp(ch->in_room);
  to_here = real_roomp(from_here->dir_option[dir]->to_room);
  
  if(to_here==NULL) 
  {
    char_from_room(ch);
    char_to_room(ch, 0);
    send_to_char("Uh-oh.  The ground melts beneath you as you fall into the swirling chaos.\n\r",ch);
    do_look(ch, "\0",15);
    return TRUE;
  }
  
  if(IS_AFFECTED(ch,AFF_FLYING)) 
    need_movement = 1;
  else
  {
    need_movement = (movement_loss[from_here->sector_type]+
		     movement_loss[to_here->sector_type]) / 2;
  }
  
  /*  
   **   Movement in water_nowswim
   */
  
  if((from_here->sector_type == SECT_WATER_SWIM) || (to_here->sector_type == SECT_WATER_SWIM)) 
  {
    if((!IS_AFFECTED(ch,AFF_WATERBREATH)) && (!IS_AFFECTED(ch,AFF_FLYING))) 
    {
      has_boat = number(1,101);
      if(has_boat < ch->skills[SKILL_SWIMMING].learned )
        need_movement = 1;
    }
  }

  if((from_here->sector_type == SECT_WATER_NOSWIM) || (to_here->sector_type == SECT_WATER_NOSWIM)) 
  {
    if(MOUNTED(ch)) 
    {
      if(!IS_AFFECTED(MOUNTED(ch), AFF_WATERBREATH) && !IS_AFFECTED(MOUNTED(ch),AFF_FLYING))
      {
        send_to_char("Your mount would have to fly or swim to go there\n\r", ch);
        return(FALSE);
      }
    }

    if((!IS_AFFECTED(ch,AFF_WATERBREATH)) && (!IS_AFFECTED(ch,AFF_FLYING))) 
    {
      has_boat = FALSE;
      /* See if char is carrying a boat */
      for (obj=ch->carrying; obj; obj=obj->next_content)
	if (obj->obj_flags.type_flag == ITEM_BOAT)
	  has_boat = TRUE;
      if (!has_boat) 
      {
	send_to_char("You need a boat to go there.\n\r", ch);
	return(FALSE);
      }

      if (has_boat)
	need_movement = 1;
    }
  }
  
  /*
    Movement in SECT_AIR
  */

  if((from_here->sector_type == SECT_AIR)||(to_here->sector_type == SECT_AIR)) 
  {
    if(MOUNTED(ch))
    { 
      if(!IS_AFFECTED(MOUNTED(ch), AFF_FLYING)) 
      {
        send_to_char("Your mount would have to fly to go there!\n\r",ch);
        return(FALSE);
      }
    }
 
    if(!IS_AFFECTED(ch,AFF_FLYING)) 
    {
      send_to_char("You would have to Fly to go there!\n\r",ch);
      return(FALSE);
    }
  }
  
  /*
    Movement in SECT_UNDERWATER
    */

  if((from_here->sector_type == SECT_UNDERWATER) || (to_here->sector_type == SECT_UNDERWATER)) 
  {
    if(MOUNTED(ch))
    {
      if(!IS_AFFECTED(MOUNTED(ch),AFF_WATERBREATH))
      {
        send_to_char("Your mount would need gills to go there!\n\r",ch);
        return(FALSE);
      }
    }

    if (!IS_AFFECTED(ch,AFF_WATERBREATH)) 
    {
      send_to_char("You would need to be a fish to go there!\n\r",ch);
      return(FALSE);
    }
  }
 
  if (!MOUNTED(ch)) {
    if (GET_MOVE(ch)<need_movement) {
      send_to_char("You are too exhausted.\n\r",ch);
      return(FALSE);
    }
  } else {
    if (GET_MOVE(MOUNTED(ch))<need_movement) {
      send_to_char("Your mount is too exhausted.\n\r", ch);
      return(FALSE);
    }
  }  
   
  if (IS_MORTAL(ch) || MOUNTED(ch)) {
    if (IS_NPC(ch)) {
      GET_MOVE(ch) -= 1;
    } else {
      if (MOUNTED(ch)) {
        GET_MOVE(MOUNTED(ch)) -= need_movement;
      } else {
        GET_MOVE(ch) -= need_movement;
      } 
    }  
  }

 
  /*
   *  nail the unlucky with traps.
   */


  if (!MOUNTED(ch)) {
    if (CheckForMoveTrap(ch, dir))
      return(FALSE);
  } else {
    if (CheckForMoveTrap(MOUNTED(ch), dir))
      return(FALSE);
  }

  if (MOUNTED(ch)) {
    char_from_room(ch);
    char_to_room(ch, from_here->dir_option[dir]->to_room);
    char_from_room(MOUNTED(ch));
    char_to_room(MOUNTED(ch), from_here->dir_option[dir]->to_room);
  } else {
    char_from_room(ch);
    char_to_room(ch, from_here->dir_option[dir]->to_room);
  }

  do_look(ch, "\0",15);
  
  if(IS_SET(to_here->room_flags, DEATH) && IS_MORTAL(ch)) 
  {
    death_cry(ch);
    if(MOUNTED(ch))
      death_cry(MOUNTED(ch));
   
    if(IS_NPC(ch) && (IS_SET(ch->specials.act, ACT_POLYSELF))) 
    {
      /*
       *   take char from storage, to room     
       */
      if(MOUNTED(ch))
        extract_char(MOUNTED(ch));

      pers = ch->desc->original;
      char_from_room(pers);
      char_to_room(pers, ch->in_room);
      SwitchStuff(ch, pers);
      zero_rent(ch);
      extract_char(ch);
      ch = pers;
    }
    zero_rent(ch);
    extract_char(ch);
    return(FALSE);
  }
 
  return(TRUE);
  
}


int MoveOne(struct char_data *ch, int dir)
{
  int was_in;
  if(DEBUG) dlog("MoveOne");  
  was_in = ch->in_room;
  if (RawMove(ch, dir)) {  /* no error */
    DisplayOneMove(ch, dir, was_in);
    return TRUE;
  } else
    return FALSE;
  
}

int MoveGroup( struct char_data *ch, int dir)
{
  struct char_data *heap_ptr[50];
  int was_in, i, heap_top, heap_tot[50];
  struct follow_type *k, *next_dude;
  
  /*
   *   move the leader. (leader never duplicates)
   */
  
  if(DEBUG) dlog("MoveGroup");
  was_in = ch->in_room;
  if (RawMove(ch, dir)) {  /* no error */
    DisplayOneMove(ch, dir, was_in);
    if (ch->followers) 
    {
      heap_top = 0;
      for(k = ch->followers; k; k = next_dude) 
      {
	next_dude = k->next;
	/*
	 *  compose a list of followers, w/heaping
	 */
	if ((was_in == k->follower->in_room) && (GET_POS(k->follower) >= POSITION_STANDING)) 
	{
	  act("You follow $N.", FALSE, k->follower, 0, ch, TO_CHAR);
	  if (k->follower->followers) 
	  {
	    MoveGroup(k->follower, dir);
	  }
	  else
	  {
	    if (RawMove(k->follower, dir)) 
	    {
	      AddToCharHeap(heap_ptr, &heap_top, heap_tot, k->follower);
	      /*
		if (!AddToCharHeap(heap_ptr, &heap_top, heap_tot, k->follower))
		displayOneMove(k->follower, dir, was_in);
	      */
	    }
	  }
	}
      }
      /*
       *  now, print out the heaped display message
       */
      for (i=0;i<heap_top;i++) {
	if (heap_tot[i] > 1 ) {
	  DisplayGroupMove(heap_ptr[i], dir, was_in, heap_tot[i]);
	} else {
	  DisplayOneMove(heap_ptr[i], dir, was_in);
	}
      }
    }
  }
}

int DisplayOneMove(struct char_data *ch, int dir, int was_in)
{
  if(DEBUG) dlog("DisplayOneMove");
  DisplayMove(ch, dir, was_in, 1);
}

int DisplayGroupMove(struct char_data *ch, int dir, int was_in, int total)
{
  if(DEBUG) dlog("DisplayGroupMove");
  DisplayMove(ch, dir, was_in, total);
}


void do_move(struct char_data *ch, char *argument, int cmd)
{

  if(DEBUG) dlog("do_move");
  if(RIDDEN(ch)) 
  {
    if(RideCheck(RIDDEN(ch))) 
    {
      do_move(RIDDEN(ch), argument, cmd);
      return;
    }
    else
    {
      FallOffMount(RIDDEN(ch), ch);
      Dismount(RIDDEN(ch), ch, POSITION_SITTING);
    }
  }  

  if(RIDDEN(ch))
  {
    if(RIDDEN(ch)->specials.fighting)
    {
      send_to_char("You can't, your rider is fighting!\n\r",ch);
      return;
    }
  }

  if(MOUNTED(ch))
  {
    if(MOUNTED(ch)->specials.fighting)
    {
      send_to_char("You can't, your mount is fighting!\n\r",ch);
      return;
    }
  }

  cmd -= 1;
  
  /*
   ** the move is valid, check for follower/master conflicts.
   */
  
  if (ch->attackers > 2) 
  {
    send_to_char("There's too many people around, no place to flee!\n\r", ch);
    return;
  }
  
  if (!ch->followers && !ch->master) 
  {
    MoveOne(ch,cmd);
  }
  else
  {
    if (!ch->followers) {
      MoveOne(ch, cmd);
    } else {
      MoveGroup(ch, cmd);
    }
  }
}


/*
  MoveOne and MoveGroup print messages.  Raw move sends success or failure.
*/


int DisplayMove( struct char_data *ch, int dir, int was_in, int total)
{
  struct char_data *tmp_ch;
  char tmp[256];
  
  if(DEBUG) dlog("DisplayMove");
  for(tmp_ch =real_roomp(was_in)->people;tmp_ch;tmp_ch= tmp_ch->next_in_room) 
  {
    if ((!IS_AFFECTED(ch, AFF_SNEAK)) || (IS_IMMORTAL(tmp_ch))) 
    {
      if ((ch != tmp_ch) && (AWAKE(tmp_ch)) && (CAN_SEE(tmp_ch, ch))) 
      {
	if (total > 1) 
	{
	  if (IS_NPC(ch)) 
	  {
	    sprintf(tmp,"%s leaves %s. [%d]\n\r", ch->player.short_descr,dirs[dir], total);
	  }
	  else
	  {
	    sprintf(tmp,"%s leaves %s.[%d]\n\r",GET_NAME(ch),dirs[dir],total);
	  }
	}
        else
        {
	  if (IS_NPC(ch)) 
          {
              if(MOUNTED(ch))
              { 
		struct char_data *mount;
		mount = MOUNTED(ch);
                sprintf(tmp,"%s leaves %s, riding on %s\n\r",
		  ch->player.short_descr, 
		  dirs[dir], 
		  mount->player.short_descr);
              } 
              else
              {   
                sprintf(tmp,"%s leaves %s.\n\r",ch->player.short_descr, dirs[dir]);
              } 
	  } 
          else 
          {
              if (MOUNTED(ch))
              { 
                sprintf(tmp,"%s leaves %s, riding on %s\n\r",
		  GET_NAME(ch), dirs[dir], MOUNTED(ch)->player.short_descr);
              }
              else
              {
                sprintf(tmp,"%s leaves %s\n\r",GET_NAME(ch),dirs[dir]);
              }
	  }
	}
	send_to_char(tmp, tmp_ch);
      }
    }
  }
  
  for (tmp_ch = real_roomp(ch->in_room)->people; tmp_ch; 
       tmp_ch = tmp_ch->next_in_room) {
    if (((!IS_AFFECTED(ch, AFF_SNEAK)) || (IS_IMMORTAL(tmp_ch))) &&
	(CAN_SEE(tmp_ch,ch)) &&
	(AWAKE(tmp_ch))) {
      if (tmp_ch != ch) {
	if (dir < 4) 
        {
          if (total == 1) 
          {
            if (MOUNTED(ch)) 
            {
              sprintf(tmp, "%s has arrived from the %s, riding on %s",
                PERS(ch, tmp_ch),dirs[rev_dir[dir]],PERS(MOUNTED(ch), tmp_ch));
            }
            else
            {
              sprintf(tmp, "%s has arrived from the %s.",PERS(ch, tmp_ch),dirs[rev_dir[dir]]);
            }
          }
          else
          {
              sprintf(tmp, "%s has arrived from the %s.",PERS(ch, tmp_ch),dirs[rev_dir[dir]]);
          }
	} 
        else 
        if (dir == 4) 
	{
          if (total == 1) 
	  {
            if (MOUNTED(ch)) 
	    {
              sprintf(tmp, "%s has arrived from below, riding on %s",
                      PERS(ch, tmp_ch), PERS(MOUNTED(ch), tmp_ch));
            }
	    else 
	    {
              sprintf(tmp, "%s has arrived from below.", PERS(ch, tmp_ch));
            }
          }
	  else 
	  {
            sprintf(tmp, "%s has arrived from below.", PERS(ch, tmp_ch));
          }
        }
	else
	if (dir == 5) 
	{
          if (total == 1) 
	  {
            if (MOUNTED(ch)) 
	    {
              sprintf(tmp, "%s has arrived from above, riding on %s",
                      PERS(ch, tmp_ch), PERS(MOUNTED(ch), tmp_ch));
            }
	    else 
	    {
              sprintf(tmp, "%s has arrived from above", PERS(ch, tmp_ch));
            }
          } 
	  else 
	  {
            sprintf(tmp, "%s has arrived from above.", PERS(ch, tmp_ch));
          }
        } 
	else 
	{
          if (total == 1) 
	  {
            if (MOUNTED(ch)) 
	    {
              sprintf(tmp, "%s has arrived from somewhere, riding on %s",
                      PERS(ch, tmp_ch), PERS(MOUNTED(ch), tmp_ch));
            }
	    else 
	    {
              sprintf(tmp, "%s has arrived from somewhere.", PERS(ch, tmp_ch));
            }
          } 
	  else 
	  {
            sprintf(tmp, "%s has arrived from somewhere.", PERS(ch, tmp_ch));
          }
        }

	if (total > 1) 
        {
	  sprintf(tmp+strlen(tmp), " [%d]", total);
	}
	strcat(tmp, "\n\r");
	send_to_char(tmp, tmp_ch);
      }
    }
  }
}

int AddToCharHeap( struct char_data *heap[50], int *top, int total[50], 
		  struct char_data *k) 
{
  int found, i;
  if(DEBUG) dlog("AddToCharHeap");  
  if (*top > 50) {
    return(FALSE);
  } else {
    found = FALSE;
    for (i=0;(i<*top&& !found);i++) {
      if (*top>0) {
	if ((IS_NPC(k)) &&
	    (k->nr == heap[i]->nr) &&
	    (heap[i]->player.short_descr) &&
	    (!strcmp(k->player.short_descr, 
		     heap[i]->player.short_descr))) {
	  total[i] += 1;
	  found=TRUE;
	}
      }          
    }
    if (!found) {
      heap[*top] = k;
      total[*top] = 1;
      *top+=1;
    }
  }
}

int find_door(struct char_data *ch, char *type, char *dir)
{
  char buf[MAX_STRING_LENGTH];
  int door;
  char *dirs[] = 
    {
      "north",
      "east",
      "south",
      "west",
      "up",
      "down",
      "\n"
      };
  struct room_direction_data *exitp;
  if(DEBUG) dlog("find_door");
  
  if (*dir)
  { /* a direction was specified */ 
    if ((door = search_block(dir, dirs, FALSE)) == -1)
    { /* Partial Match */		
      send_to_char("That's not a direction.\n\r", ch);
      return(-1);
    }
    exitp = EXIT(ch, door);
    if (exitp) 
    {
      if (!exitp->keyword)
	return(door);
      if ((isname(type, exitp->keyword))&& (strcmp(type,"secret"))) 
      {
	return(door);
      }
      else
      {	       	
	sprintf(buf, "I see no %s there.\n\r", type);
	send_to_char(buf, ch);
	return(-1);
      }
    }
    else
    {       	
      sprintf(buf, "I see no %s there.\n\r", type);
      send_to_char(buf, ch);
      return(-1);
    }
  }
  else
  { /* try to locate the keyword */
    for (door = 0; door <= 5; door++)
      if ((exitp=EXIT(ch, door)) && exitp->keyword && isname(type, exitp->keyword))
	return(door);
    
    sprintf(buf, "I see no %s here.\n\r", type);
    send_to_char(buf, ch);
    return(-1);
  }
}

open_door(struct char_data *ch, int dir)
     /* remove all necessary bits and send messages */
{
  struct room_direction_data *exitp, *back;
  struct room_data	*rp;
  char	buf[MAX_INPUT_LENGTH];
  if(DEBUG) dlog("open_door");
  
  rp = real_roomp(ch->in_room);
  if (rp==NULL) 
  {
    sprintf(buf, "NULL rp in open_door() for %s.", PERS(ch,ch));
    log(buf);
  }
  
  exitp = rp->dir_option[dir];
  
  REMOVE_BIT(exitp->exit_info, EX_CLOSED);
  if (exitp->keyword) 
  {
    if(strcmp(exitp->keyword,"secret")&&(IS_NOT_SET(exitp->exit_info,EX_SECRET)))
    {
      sprintf(buf, "$n opens the %s", fname(exitp->keyword));
      act(buf, FALSE, ch, 0, 0, TO_ROOM);
    }
    else
    {
      act("$n reveals a hidden passage!", FALSE, ch, 0, 0, TO_ROOM);
    }
  } else
    act("$n opens the door.", FALSE, ch, 0, 0, TO_ROOM);
  
  /* now for opening the OTHER side of the door! */
  if (exit_ok(exitp, &rp) &&
      (back = rp->dir_option[rev_dir[dir]]) &&
      (back->to_room == ch->in_room))
    {
      REMOVE_BIT(back->exit_info, EX_CLOSED);
      if (back->keyword)	{
	sprintf(buf, "The %s is opened from the other side.\n\r",
		fname(back->keyword));
	send_to_room(buf, exitp->to_room);
      }
      else
	send_to_room("The door is opened from the other side.\n\r",
		     exitp->to_room);
    }						 
}

raw_open_door(struct char_data *ch, int dir)
     /* remove all necessary bits and send messages */
{
  struct room_direction_data *exitp, *back;
  struct room_data      *rp;
  char  buf[MAX_INPUT_LENGTH];
  if(DEBUG) dlog("raw_open_door");  
  rp = real_roomp(ch->in_room);
  if (rp==NULL) {
    sprintf(buf, "NULL rp in open_door() for %s.", PERS(ch,ch));
    log(buf);
  }
   
  exitp = rp->dir_option[dir];
  
  REMOVE_BIT(exitp->exit_info, EX_CLOSED);
  /* now for opening the OTHER side of the door! */
  if (exit_ok(exitp, &rp) &&
      (back = rp->dir_option[rev_dir[dir]]) &&
      (back->to_room == ch->in_room))    {
      REMOVE_BIT(back->exit_info, EX_CLOSED);
      if (back->keyword && (strcmp("secret", fname(back->keyword))))    {
        sprintf(buf, "The %s is opened from the other side.\n\r",
                fname(back->keyword));
        send_to_room(buf, exitp->to_room);
      } else {
        send_to_room("The door is opened from the other side.\n\r",
                     exitp->to_room);
      }
    }  
} 

void do_open(struct char_data *ch, char *argument, int cmd)
{
  int door;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  struct room_direction_data *back;
  struct obj_data *obj;
  struct char_data *victim;
  struct room_direction_data	*exitp;
  
  argument_interpreter(argument, type, dir);
  if(DEBUG) dlog("do_open");
  
  if (!*type)
    send_to_char("Open what?\n\r", ch);
  else
  if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,ch, &victim, &obj)) 
  {
    /* this is an object */
    if (obj->obj_flags.type_flag != ITEM_CONTAINER)
      send_to_char("That's not a container.\n\r", ch);
    else if (IS_NOT_SET(obj->obj_flags.value[1], CONT_CLOSED))
      send_to_char("But it's already open!\n\r", ch);
    else if (IS_NOT_SET(obj->obj_flags.value[1], CONT_CLOSEABLE))
      send_to_char("You can't do that.\n\r", ch);
    else if (IS_SET(obj->obj_flags.value[1], CONT_LOCKED))
      send_to_char("It seems to be locked.\n\r", ch);
    else 
    {
      REMOVE_BIT(obj->obj_flags.value[1], CONT_CLOSED);
      send_to_char("Ok.\n\r", ch);
      act("$n opens $p.", FALSE, ch, obj, 0, TO_ROOM);
    }
  }
  else
  if ((door = find_door(ch, type, dir)) >= 0) 
  {
    /* perhaps it is a door */
    exitp = EXIT(ch, door);
    if (IS_NOT_SET(exitp->exit_info, EX_ISDOOR))
      send_to_char("That's impossible, I'm afraid.\n\r", ch);
    else if (IS_NOT_SET(exitp->exit_info, EX_CLOSED))
      send_to_char("It's already open!\n\r", ch);
    else if (IS_SET(exitp->exit_info, EX_LOCKED))
      send_to_char("It seems to be locked.\n\r", ch);
    else
    {
	struct room_data	*rp;
	open_door(ch, door);
	send_to_char("Ok.\n\r", ch);
    }
  }
}


void do_close(struct char_data *ch, char *argument, int cmd)
{
  int door;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  struct room_direction_data *back, *exitp;
  struct obj_data *obj;
  struct char_data *victim;
  struct room_data	*rp;
  
  
  if(DEBUG) dlog("do_close");
  argument_interpreter(argument, type, dir);
  
  if (!*type)
    send_to_char("Close what?\n\r", ch);
  else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
			ch, &victim, &obj)) {
    
    /* this is an object */
    
    if (obj->obj_flags.type_flag != ITEM_CONTAINER)
      send_to_char("That's not a container.\n\r", ch);
    else if (IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
      send_to_char("But it's already closed!\n\r", ch);
    else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSEABLE))
      send_to_char("That's impossible.\n\r", ch);
    else
      {
	SET_BIT(obj->obj_flags.value[1], CONT_CLOSED);
	send_to_char("Ok.\n\r", ch);
	act("$n closes $p.", FALSE, ch, obj, 0, TO_ROOM);
      }
  } else if ((door = find_door(ch, type, dir)) >= 0) {
    
    /* Or a door */
    exitp = EXIT(ch, door);
    if (!IS_SET(exitp->exit_info, EX_ISDOOR))
      send_to_char("That's absurd.\n\r", ch);
    else if (IS_SET(exitp->exit_info, EX_CLOSED))
      send_to_char("It's already closed!\n\r", ch);
    else      {
      SET_BIT(exitp->exit_info, EX_CLOSED);
      if (exitp->keyword)
	act("$n closes the $F.", 0, ch, 0, exitp->keyword,
	    TO_ROOM);
      else
	act("$n closes the door.", FALSE, ch, 0, 0, TO_ROOM);
      send_to_char("Ok.\n\r", ch);
      /* now for closing the other side, too */
      if (exit_ok(exitp,&rp) &&
	  (back = rp->dir_option[rev_dir[door]]) &&
	  (back->to_room == ch->in_room) ) {
	SET_BIT(back->exit_info, EX_CLOSED);
	if (back->keyword)    {	      
	  sprintf(buf, "The %s closes quietly.\n\r", back->keyword);
	  send_to_room(buf, exitp->to_room);
	}
	else
	  send_to_room( "The door closes quietly.\n\r", exitp->to_room);
      }						 
    }
  }
}


int has_key(struct char_data *ch, int key)
{
  struct obj_data *o;
  if(DEBUG) dlog("has_key");  
  for (o = ch->carrying; o; o = o->next_content)
    if (obj_index[o->item_number].virtual == key)
      return(1);
  
  if (ch->equipment[HOLD])
    if (obj_index[ch->equipment[HOLD]->item_number].virtual == key)
      return(1);
  
  return(0);
}

void raw_unlock_door( struct char_data *ch, struct room_direction_data *exitp, int door)
{
  struct room_data *rp;
  struct room_direction_data *back;
  char buf[128];
  if(DEBUG) dlog("raw_unlock_door");
  REMOVE_BIT(exitp->exit_info, EX_LOCKED);
  /* now for unlocking the other side, too */
  rp = real_roomp(exitp->to_room);
  if (rp &&
      (back = rp->dir_option[rev_dir[door]]) &&
      back->to_room == ch->in_room) {
    REMOVE_BIT(back->exit_info, EX_LOCKED);
  } else {
    sprintf(buf, "Inconsistent door locks in rooms %d->%d",
            ch->in_room, exitp->to_room);
    log(buf);
  }
}

void raw_lock_door( struct char_data *ch, struct room_direction_data *exitp, int door)
{      
  struct room_data *rp;
  struct room_direction_data *back;
  char buf[128];
  if(DEBUG) dlog("raw_lock_door");

  SET_BIT(exitp->exit_info, EX_LOCKED);
  /* now for locking the other side, too */
  rp = real_roomp(exitp->to_room);
  if (rp &&
      (back = rp->dir_option[rev_dir[door]]) &&
      back->to_room == ch->in_room) {
    SET_BIT(back->exit_info, EX_LOCKED);
  } else {
    sprintf(buf, "Inconsistent door locks in rooms %d->%d",
            ch->in_room, exitp->to_room);
    log(buf);
  }    
}

void do_lock(struct char_data *ch, char *argument, int cmd)
{
  int door;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  struct room_direction_data *back, *exitp;
  struct obj_data *obj;
  struct char_data *victim;
  struct room_data *rp;
  if(DEBUG) dlog("do_lock");
  
  argument_interpreter(argument, type, dir);
  
  if (!*type)
    send_to_char("Lock what?\n\r", ch);
  else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
			ch, &victim, &obj)) {
    
    /* this is an object */
    
    if (obj->obj_flags.type_flag != ITEM_CONTAINER)
      send_to_char("That's not a container.\n\r", ch);
    else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
      send_to_char("Maybe you should close it first...\n\r", ch);
    else if (obj->obj_flags.value[2] < 0)
      send_to_char("That thing can't be locked.\n\r", ch);
    else if (!has_key(ch, obj->obj_flags.value[2]))
      send_to_char("You don't seem to have the proper key.\n\r", ch);	
    else if (IS_SET(obj->obj_flags.value[1], CONT_LOCKED))
      send_to_char("It is locked already.\n\r", ch);
    else
      {
	SET_BIT(obj->obj_flags.value[1], CONT_LOCKED);
	send_to_char("*Cluck*\n\r", ch);
	act("$n locks $p - 'cluck', it says.", FALSE, ch, obj, 0, TO_ROOM);
      }
  } else if ((door = find_door(ch, type, dir)) >= 0) {
    
    /* a door, perhaps */
    exitp = EXIT(ch, door);
    
    if (!IS_SET(exitp->exit_info, EX_ISDOOR))
      send_to_char("That's absurd.\n\r", ch);
    else if (!IS_SET(exitp->exit_info, EX_CLOSED))
      send_to_char("You have to close it first, I'm afraid.\n\r", ch);
    else if (exitp->key < 0)
      send_to_char("There does not seem to be any keyholes.\n\r", ch);
    else if (!has_key(ch, exitp->key))
      send_to_char("You don't have the proper key.\n\r", ch);
    else if (IS_SET(exitp->exit_info, EX_LOCKED))
      send_to_char("It's already locked!\n\r", ch);
    else
      {
	SET_BIT(exitp->exit_info, EX_LOCKED);
	if (exitp->keyword)
	  act("$n locks the $F.", 0, ch, 0,  exitp->keyword,
	      TO_ROOM);
	else
	  act("$n locks the door.", FALSE, ch, 0, 0, TO_ROOM);
	send_to_char("*Click*\n\r", ch);
	/* now for locking the other side, too */
	rp = real_roomp(exitp->to_room);
	if (rp &&
	    (back = rp->dir_option[rev_dir[door]]) &&
	    back->to_room == ch->in_room)
	  SET_BIT(back->exit_info, EX_LOCKED);
      }
  }
}


void do_unlock(struct char_data *ch, char *argument, int cmd)
{
  int door;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  struct room_direction_data *back, *exitp;
  struct obj_data *obj;
  struct char_data *victim;
  struct room_data *rp;
  if(DEBUG) dlog("do_unlock");
  
  argument_interpreter(argument, type, dir);
  
  if (!*type)
    send_to_char("Unlock what?\n\r", ch);
  else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
			ch, &victim, &obj)) {
    
    /* this is an object */
    
    if (obj->obj_flags.type_flag != ITEM_CONTAINER)
      send_to_char("That's not a container.\n\r", ch);
    else if (obj->obj_flags.value[2] < 0)
      send_to_char("Odd - you can't seem to find a keyhole.\n\r", ch);
    else if (!has_key(ch, obj->obj_flags.value[2]))
      send_to_char("You don't seem to have the proper key.\n\r", ch);	
    else if (!IS_SET(obj->obj_flags.value[1], CONT_LOCKED))
      send_to_char("Oh.. it wasn't locked, after all.\n\r", ch);
    else
      {
	REMOVE_BIT(obj->obj_flags.value[1], CONT_LOCKED);
	send_to_char("*Click*\n\r", ch);
	act("$n unlocks $p.", FALSE, ch, obj, 0, TO_ROOM);
      }
  } else if ((door = find_door(ch, type, dir)) >= 0) {
    
    /* it is a door */
    exitp = EXIT(ch, door);
    
    if (!IS_SET(exitp->exit_info, EX_ISDOOR))
      send_to_char("That's absurd.\n\r", ch);
    else if (!IS_SET(exitp->exit_info, EX_CLOSED))
      send_to_char("Heck.. it ain't even closed!\n\r", ch);
    else if (exitp->key < 0)
      send_to_char("You can't seem to spot any keyholes.\n\r", ch);
    else if (!has_key(ch, exitp->key))
      send_to_char("You do not have the proper key for that.\n\r", ch);
    else if (!IS_SET(exitp->exit_info, EX_LOCKED))
      send_to_char("It's already unlocked, it seems.\n\r", ch);
    else {
      REMOVE_BIT(exitp->exit_info, EX_LOCKED);
      if (exitp->keyword)
	act("$n unlocks the $F.", 0, ch, 0, exitp->keyword,
	    TO_ROOM);
      else
	act("$n unlocks the door.", FALSE, ch, 0, 0, TO_ROOM);
      send_to_char("*click*\n\r", ch);
      /* now for unlocking the other side, too */
      rp = real_roomp(exitp->to_room);
      if (rp &&
	  (back = rp->dir_option[rev_dir[door]]) &&
	  back->to_room == ch->in_room)
	REMOVE_BIT(back->exit_info, EX_LOCKED);
    }
  }
}

void do_pick(struct char_data *ch, char *argument, int cmd)
{
  byte percent;
  int door;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  struct room_direction_data *back, *exitp;
  struct obj_data *obj;
  struct char_data *victim;
  struct room_data	*rp;
  if(DEBUG) dlog("do_pick");
  
  argument_interpreter(argument, type, dir);
  
  percent=number(1,101); /* 101% is a complete failure */
  
  if (percent > (ch->skills[SKILL_PICK_LOCK].learned)) 
  {
    send_to_char("You failed to pick the lock.\n\r", ch);
    return;
  }
  
  if (!*type) 
  {
    send_to_char("Pick what?\n\r", ch);
  }
  else
  if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,ch, &victim, &obj)) 
  {
    /* this is an object */
    if (obj->obj_flags.type_flag != ITEM_CONTAINER)
      send_to_char("That's not a container.\n\r", ch);
    else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
      send_to_char("Silly - it ain't even closed!\n\r", ch);
    else if (obj->obj_flags.value[2] < 0)
      send_to_char("Odd - you can't seem to find a keyhole.\n\r", ch);
    else if (!IS_SET(obj->obj_flags.value[1], CONT_LOCKED))
      send_to_char("Oho! This thing is NOT locked!\n\r", ch);
    else if (IS_SET(obj->obj_flags.value[1], CONT_PICKPROOF))
      send_to_char("It resists your attempts at picking it.\n\r", ch);
    else
      {
	REMOVE_BIT(obj->obj_flags.value[1], CONT_LOCKED);
	send_to_char("*Click*\n\r", ch);
	act("$n fiddles with $p.", FALSE, ch, obj, 0, TO_ROOM);
	if(ch->skills[SKILL_PICK_LOCK].learned < 50)
	  ch->skills[SKILL_PICK_LOCK].learned += 2;
      }
  }
  else
  if ((door = find_door(ch, type, dir)) >= 0) 
  {
    exitp = EXIT(ch, door);
    if (!IS_SET(exitp->exit_info, EX_ISDOOR))
      send_to_char("That's absurd.\n\r", ch);
    else if (!IS_SET(exitp->exit_info, EX_CLOSED))
      send_to_char("You realize that the door is already open.\n\r", ch);
    else if (exitp->key < 0)
      send_to_char("You can't seem to spot any lock to pick.\n\r", ch);
    else if (!IS_SET(exitp->exit_info, EX_LOCKED))
      send_to_char("Oh.. it wasn't locked at all.\n\r", ch);
    else if (IS_SET(exitp->exit_info, EX_PICKPROOF))
      send_to_char("You seem to be unable to pick this lock.\n\r", ch);
    else {
      REMOVE_BIT(exitp->exit_info, EX_LOCKED);
      if (exitp->keyword)
	act("$n skillfully picks the lock of the $F.", 0, ch, 0, exitp->keyword, TO_ROOM);
      else
	act("$n picks the lock.",TRUE,ch,0,0,TO_ROOM);
      send_to_char("The lock quickly yields to your skills.\n\r", ch);
      /* now for unlocking the other side, too */
      rp = real_roomp(exitp->to_room);
      if (rp &&
	  (back = rp->dir_option[rev_dir[door]]) &&
	  back->to_room == ch->in_room )
	REMOVE_BIT(back->exit_info, EX_LOCKED);
      if(ch->skills[SKILL_PICK_LOCK].learned < 50)
	ch->skills[SKILL_PICK_LOCK].learned += 2;
    }
  }
}

void do_enter(struct char_data *ch, char *argument, int cmd)
{
  int door;
  char buf[MAX_INPUT_LENGTH], tmp[MAX_STRING_LENGTH];
  struct room_direction_data	*exitp;
  struct room_data	*rp;
  if(DEBUG) dlog("do_enter");
  
  one_argument(argument, buf);
  
  if (*buf) { /* an argument was supplied, search for door keyword */
    for (door = 0; door <= 5; door++)
      if (exit_ok(exitp=EXIT(ch, door), NULL) && exitp->keyword &&
	  0==str_cmp(exitp->keyword, buf)) {
	do_move(ch, "", ++door);
	return;
      }
    sprintf(tmp, "There is no %s here.\n\r", buf);
    send_to_char(tmp, ch);
  } else if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
    send_to_char("You are already indoors.\n\r", ch);
  } else {
    /* try to locate an entrance */
    for (door = 0; door <= 5; door++)
      if (exit_ok(exitp=EXIT(ch, door), &rp) &&
	  !IS_SET(exitp->exit_info, EX_CLOSED) &&
	  IS_SET(rp->room_flags, INDOORS))
	{
	  do_move(ch, "", ++door);
	  return;
	}
    send_to_char("You can't seem to find anything to enter.\n\r", ch);
  }
}


void do_leave(struct char_data *ch, char *argument, int cmd)
{
  int door;
  struct room_direction_data	*exitp;
  struct room_data	*rp;
  if(DEBUG) dlog("do_leave");
  
  if (!IS_SET(RM_FLAGS(ch->in_room), INDOORS))
    send_to_char("You are outside.. where do you want to go?\n\r", ch);
  else    {
      for (door = 0; door <= 5; door++)
	if (exit_ok(exitp=EXIT(ch, door), &rp) &&
	    !IS_SET(exitp->exit_info, EX_CLOSED) &&
	    !IS_SET(rp->room_flags, INDOORS)) {
	  do_move(ch, "", ++door);
	  return;
	}
      send_to_char("I see no obvious exits to the outside.\n\r", ch);
    }
}


void do_stand(struct char_data *ch, char *argument, int cmd)
{
if(DEBUG) dlog("do_stand");
  switch(GET_POS(ch)) {
  case POSITION_STANDING : { 
    act("You are already standing.",FALSE, ch,0,0,TO_CHAR);
  } break;
  case POSITION_SITTING	: { 
    act("You stand up.", FALSE, ch,0,0,TO_CHAR);
    act("$n clambers on $s feet.",TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POSITION_STANDING;
  } break;
  case POSITION_RESTING	: { 
    act("You stop resting, and stand up.", FALSE, ch,0,0,TO_CHAR);
    act("$n stops resting, and clambers on $s feet.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POSITION_STANDING;
  } break;
  case POSITION_SLEEPING : { 
    act("You have to wake up first!", FALSE, ch, 0,0,TO_CHAR);
  } break;
  case POSITION_MOUNTED  :
    send_to_char("But you are mounted?\n\r",ch);
    break;
  case POSITION_FIGHTING : { 
    act("Do you not consider fighting as standing?",FALSE, ch, 0, 0, TO_CHAR);
  } break;
    default : { 
      act("You stop floating around, and put your feet on the ground.",
	  FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops floating around, and puts $s feet on the ground.",
	  TRUE, ch, 0, 0, TO_ROOM);
    } break;
  }
}


void do_sit(struct char_data *ch, char *argument, int cmd)
{
if(DEBUG) dlog("do_sit");
  switch(GET_POS(ch)) {
  case POSITION_STANDING : {
    act("You sit down.", FALSE, ch, 0,0, TO_CHAR);
    act("$n sits down.", FALSE, ch, 0,0, TO_ROOM);
    GET_POS(ch) = POSITION_SITTING;
  } break;
  case POSITION_SITTING	: {
    send_to_char("You'r sitting already.\n\r", ch);
  } break;
  case POSITION_RESTING	: {
    act("You stop resting, and sit up.", FALSE, ch,0,0,TO_CHAR);
    act("$n stops resting.", TRUE, ch, 0,0,TO_ROOM);
    GET_POS(ch) = POSITION_SITTING;
  } break;
  case POSITION_SLEEPING : {
    act("You have to wake up first.", FALSE, ch, 0, 0, TO_CHAR);
  } break;
  case POSITION_MOUNTED :
    send_to_char("But you are mounted?\n\r",ch);
    break;
  case POSITION_FIGHTING : {
    act("Sit down while fighting? are you MAD?", FALSE, ch,0,0,TO_CHAR);
  } break;
    default : {
      act("You stop floating around, and sit down.", FALSE, ch,0,0,TO_CHAR);
      act("$n stops floating around, and sits down.", TRUE, ch,0,0,TO_ROOM);
      GET_POS(ch) = POSITION_SITTING;
    } break;
  }
}


void do_rest(struct char_data *ch, char *argument, int cmd) 
{
if(DEBUG) dlog("do_rest");
  switch(GET_POS(ch)) {
  case POSITION_STANDING : {
    send_to_char("You sit down and rest your tired bones.\n\r", ch);
    act("$n sits down and rests.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POSITION_RESTING;
  } break;
  case POSITION_SITTING : {
    send_to_char("You rest your tired bones\n\r.", ch);
    act("$n rests.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POSITION_RESTING;
  } break;
  case POSITION_RESTING : {
    send_to_char("You are already resting.\n\r", ch);
  } break;
  case POSITION_SLEEPING : {
    send_to_char("You have to wake up first.\n\r", ch);
  } break;
  case POSITION_MOUNTED  :
    send_to_char("But you are mounted?\n\r",ch);
    break;
  case POSITION_FIGHTING : {
    send_to_char("Rest while fighting? are you MAD?\n\r",ch);
  } break;
    default : {
      act("You stop floating around, and stop to rest your tired bones.",
	  FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops floating around, and rests.", FALSE, ch, 0,0, TO_ROOM);
      GET_POS(ch) = POSITION_SITTING;
    } break;
  }
}


void do_sleep(struct char_data *ch, char *argument, int cmd) 
{
if(DEBUG) dlog("do_sleep");
  
  switch(GET_POS(ch)) {
  case POSITION_STANDING : 
  case POSITION_SITTING  :
  case POSITION_RESTING  : {
    send_to_char("You go to sleep.\n\r", ch);
    act("$n lies down and falls asleep.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POSITION_SLEEPING;
  } break;
  case POSITION_SLEEPING : {
    send_to_char("You are already sound asleep.\n\r", ch);
  } break;
  case POSITION_MOUNTED  :
    send_to_char("But you are mounted?\n\r",ch);
    break;
  case POSITION_FIGHTING : {
    send_to_char("Sleep while fighting? are you MAD?\n\r", ch);
  } break;
    default : {
      act("You stop floating around, and lie down to sleep.",
	  FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops floating around, and lie down to sleep.",
	  TRUE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POSITION_SLEEPING;
    } break;
  }
}


void do_wake(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *tmp_char;
  char arg[MAX_STRING_LENGTH];
  
  if(DEBUG) dlog("do_wake");
  
  one_argument(argument,arg);
  if (*arg) {
    if (GET_POS(ch) == POSITION_SLEEPING) {
      act("You can't wake people up if you are asleep yourself!",
	  FALSE, ch,0,0,TO_CHAR);
    } else {
      tmp_char = get_char_room_vis(ch, arg);
      if (tmp_char) {
	if (tmp_char == ch) {
	  act("If you want to wake yourself up, just type 'wake'",
	      FALSE, ch,0,0,TO_CHAR);
	} else {
	  if (GET_POS(tmp_char) == POSITION_SLEEPING) {
	    if (IS_AFFECTED(tmp_char, AFF_SLEEP)) {
	      act("You can not wake $M up!", FALSE, ch, 0, tmp_char, TO_CHAR);
	    } else {
	      act("You wake $M up.", FALSE, ch, 0, tmp_char, TO_CHAR);
	      GET_POS(tmp_char) = POSITION_SITTING;
	      act("You are awakened by $n.", FALSE, ch, 0, tmp_char, TO_VICT);
	    }
	  } else {
	    act("$N is already awake.",FALSE,ch,0,tmp_char, TO_CHAR);
	  }
	}
      } else {
	send_to_char("You do not see that person here.\n\r", ch);
      }
    }
  } else {
    if (IS_AFFECTED(ch,AFF_SLEEP)) {
      send_to_char("You can't wake up!\n\r", ch);
    } else {
      if (GET_POS(ch) > POSITION_SLEEPING)
	send_to_char("You are already awake...\n\r", ch);
      else {
	send_to_char("You wake, and sit up.\n\r", ch);
	act("$n awakens.", TRUE, ch, 0, 0, TO_ROOM);
	GET_POS(ch) = POSITION_SITTING;
      }
    }
  }
}

void do_follow(struct char_data *ch, char *argument, int cmd)
{
  char name[160];
  struct char_data *leader;
  
  void stop_follower(struct char_data *ch);
  void add_follower(struct char_data *ch, struct char_data *leader);
  if(DEBUG) dlog("do_follow");
  
  only_argument(argument, name);
  
  if (*name)
  {
    if (!(leader = get_char_room_vis(ch, name))) 
    {
      send_to_char("I see no person by that name here!\n\r", ch);
      return;
    }
  } else {
    send_to_char("Who do you wish to follow?\n\r", ch);
    return;
  }
 
  if(IS_AFFECTED(ch, AFF_GROUP))
  {
    REMOVE_BIT(ch->specials.affected_by,AFF_GROUP);
  }

  if(IS_AFFECTED(ch, AFF_CHARM) && (ch->master))
  {
    act("But you only feel like following $N!",
      FALSE, ch, 0, ch->master, TO_CHAR);
  }
  else
  { /* Not Charmed follow person */
    
    if (leader == ch)
    {
      if (!ch->master)
      {
	send_to_char("You are already following yourself.\n\r", ch);
	return;
      }
      stop_follower(ch);
    } else {
      if (circle_follow(ch, leader)) {
	act("Sorry, but following in 'loops' is not allowed", FALSE, ch, 0, 0, TO_CHAR);
	return;
      }
      if (ch->master)
	stop_follower(ch);
      
      add_follower(ch, leader);
    }
  }
}

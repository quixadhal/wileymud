/* ************************************************************************
*  file: reception.c, Special module for Inn's.           Part of DIKUMUD *
*  Usage: Procedures handling saving/loading of player objects            *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <sys/time.h>

#include "structs.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "interpreter.h"
#include "utils.h"
#include "spells.h"
#include "reception.h"

extern struct room_data *world;
extern struct index_data *mob_index;
extern struct index_data *obj_index;

/* Extern functions */

void store_to_char(struct char_file_u *st, struct char_data *ch);
void do_tell(struct char_data *ch, char *argument, int cmd);
int str_cmp(char *arg1, char *arg2);
void clear_char(struct char_data *ch);
void zero_rent(struct char_data *ch);

/* ************************************************************************
* Routines used for the "Offer"                                           *
************************************************************************* */

void add_obj_cost(struct char_data *ch, struct char_data *re,
                  struct obj_data *obj, struct obj_cost *cost)
{
  char buf[MAX_INPUT_LENGTH];
  int  temp;
  
  /* Add cost for an item and it's contents, and next->contents */
  
  if(obj)
  {
    if((obj->item_number > -1) && (cost->ok)) 
    {
      temp = (MAX(0, obj->obj_flags.cost_per_day)*0); /*fix for full price*/
      cost->total_cost += 0;
      if(re) 
      {
	sprintf(buf, "%30s : %d coins/day\n\r", obj->short_description, temp);
	send_to_char(buf, ch);
      }
      cost->no_carried++;
      add_obj_cost(ch, re, obj->contains, cost);
      add_obj_cost(ch, re, obj->next_content, cost);
    }
    else
      if(cost->ok) 
      {
	if(re) 
	{
	  act("$n tells you 'I refuse storing $p'",FALSE,re,obj,ch,TO_VICT);
	}
	else
	{
	  act("Sorry, but $p don't keep in storage.",FALSE,0,obj,ch,TO_VICT);
	}
      }
  }
}

bool recep_offer(struct char_data *ch,	struct char_data *receptionist,
		 struct obj_cost *cost)
{
  int i;
  char buf[MAX_STRING_LENGTH];
  
  cost->total_cost = 0; /* Minimum cost */
  cost->no_carried = 0;
  cost->ok = TRUE; /* Use if any "-1" objects */
  add_obj_cost(ch, receptionist, ch->carrying, cost);
  
  for(i = 0; i<MAX_WEAR; i++)
    add_obj_cost(ch, receptionist, ch->equipment[i], cost);
  
  if (!cost->ok)
    return(FALSE);
  
  if(cost->no_carried == 0) 
  {
    if(receptionist)
      act("$n tells you 'But you are not carrying anything?'",
	FALSE,receptionist,0,ch,TO_VICT);
    return(FALSE);
  }
  
  if(cost->no_carried > MAX_OBJ_SAVE) 
  {
    if(receptionist) 
    {
      sprintf(buf,
	"$n tells you 'Sorry, but I can't store any more than %d items.",
	MAX_OBJ_SAVE);
      act(buf,FALSE,receptionist,0,ch,TO_VICT);
    }
    return(FALSE);
  }
  
  if (receptionist) 
  {
   /*
    sprintf(buf,
      "$n tells you 'It will cost you %d coins per day'",cost->total_cost);
    act(buf,FALSE,receptionist,0,ch,TO_VICT);
   */
    if (cost->total_cost > GET_GOLD(ch)) 
    {
      if (GetMaxLevel(ch) < LOW_IMMORTAL)
	act("$n tells you 'Which I can see you can't afford'",
	    FALSE,receptionist,0,ch,TO_VICT);
      else
      {
	act("$n tells you 'Well, since you're a God, I guess it's okay'",
	    FALSE,receptionist,0,ch,TO_VICT);
	cost->total_cost = 0;
      }
    }
  }
 
/*
  if ( cost->total_cost > GET_GOLD(ch) )
    return(FALSE);
  else
*/
    return(TRUE);
}


/* ************************************************************************
* General save/load routines                                              *
************************************************************************* */

void update_file(FILE *fl, char *name, struct obj_file_u *st)
{
  struct rental_header	rh;
  int	nlength;

  if(st->nobjects==0)
  {
    rh.inuse = 0;
    nlength = 0;
    rh.length = 0;
  }
  else
  {
    rh.inuse = 1;
    nlength = sizeof(*st) + (st->nobjects-MAX_OBJ_SAVE) * sizeof(*st->objects);
    rh.length = nlength;
  }
 
  strcpy(rh.owner, name);
  fwrite(&rh, sizeof(rh), 1, fl);
  fwrite(st, nlength, 1, fl);
  return;
}

/* ************************************************************************
* Routines used to load a characters equipment from disk                  *
************************************************************************* */

void obj_store_to_char(struct char_data *ch, struct obj_file_u *st)
{
  struct obj_data *obj;
  int i, j;
  
  void obj_to_char(struct obj_data *object, struct char_data *ch);
  
  for(i=0; i<st->nobjects; i++) {
    if (st->objects[i].item_number > -1 && 
	real_object(st->objects[i].item_number) > -1) {
      obj = read_object(st->objects[i].item_number, VIRTUAL);
      obj->obj_flags.value[0] = st->objects[i].value[0];
      obj->obj_flags.value[1] = st->objects[i].value[1];
      obj->obj_flags.value[2] = st->objects[i].value[2];
      obj->obj_flags.value[3] = st->objects[i].value[3];
      obj->obj_flags.extra_flags = st->objects[i].extra_flags;
      obj->obj_flags.weight      = st->objects[i].weight;
      obj->obj_flags.timer       = st->objects[i].timer;
      obj->obj_flags.bitvector   = st->objects[i].bitvector;
      
      for(j=0; j<MAX_OBJ_AFFECT; j++)
	obj->affected[j] = st->objects[i].affected[j];
      
      obj_to_char(obj, ch);
    }
  }
}

void load_char_objs(struct char_data *ch)
{
  FILE *fl;
  int i, j;
  bool found = FALSE;
  float timegold;
  struct rental_header rh;
  struct obj_file_u *st;
  char name[40];
  char * t_ptr;
  char path[256];
 
  strcpy(name,GET_NAME(ch));
  t_ptr = name;
  for(; * t_ptr != '\0';t_ptr++)
    *t_ptr = LOWER(*t_ptr);
  sprintf(path,"ply/%s.o",name);
 
  if(!(fl = fopen(path, "r+b")))  
  {
    log("no .o file for character");
    fl = fopen(path,"w+b");
    rh.inuse = 0;
    rh.length = 0;
    strcpy(rh.owner,"empty");
    fwrite(&rh,sizeof(rh),1,fl);
  }
  else 
    fread(&rh, sizeof(rh), 1, fl);

  if( rh.inuse == 1 )
  {
    st = (void*)malloc(rh.length);
    fread(st, rh.length, 1, fl);
    obj_store_to_char(ch, st);

/*
  if the character has been out for 12 real hours, they are fully healed
  upon re-entry.  if they stay out for 24 full hours, all affects are
  removed, including bad ones.
*/

    if(st->last_update + 6*SECS_PER_REAL_HOUR < time(0))
    {
      RestoreChar(ch);
      RemAllAffects(ch);
    }
    
    if(ch->in_room == NOWHERE && st->last_update + 12*SECS_PER_REAL_HOUR > time(0)) 
    {
      log("Char reconnecting after game crash");
    } 
    else
    {
      char buf[MAX_STRING_LENGTH];
      if (ch->in_room == NOWHERE)
	log("Char reconnecting after autorent");
      timegold=(int)(((st->total_cost*((float)time(0)-st->last_update))/(SECS_PER_REAL_DAY))*0);  
      sprintf(buf, "Char ran up charges of %g gold in rent", timegold);
      log(buf);
      sprintf(buf, "You ran up charges of %g gold in rent.\n\r", timegold);
      send_to_char(buf, ch);
      GET_GOLD(ch) -= timegold;
    
      if (GET_GOLD(ch) < 0) 
      {
	log("Char ran out of money in rent");
        send_to_char("You ran out of money, you deadbeat.\n\r", ch);
	GET_GOLD(ch) = 0;
      }
    }
    free(st);
  }
  else
  {
    log("Char has no rental data");
  }

  fclose(fl);
  /* Save char, to avoid strange data if crashing */

  save_char(ch, NOWHERE);
}


/* ************************************************************************
* Routines used to save a characters equipment from disk                  *
************************************************************************* */

/* Puts object in store, at first item which has no -1 */
void put_obj_in_store(struct obj_data *obj, struct obj_file_u *st)
{
  int i, j;
  bool found = FALSE;
  struct obj_file_elem *oe;

  if (st->nobjects>=MAX_OBJ_SAVE) 
  {
    printf("you want to rent more than %d items?!\n", st->nobjects);
    return;
  }

  oe = st->objects + st->nobjects;
  
  oe->item_number = obj_index[obj->item_number].virtual;
  oe->value[0] = obj->obj_flags.value[0];
  oe->value[1] = obj->obj_flags.value[1];
  oe->value[2] = obj->obj_flags.value[2];
  oe->value[3] = obj->obj_flags.value[3];
  
  oe->extra_flags = obj->obj_flags.extra_flags;
  oe->weight  = obj->obj_flags.weight;
  oe->timer  = obj->obj_flags.timer;
  oe->bitvector  = obj->obj_flags.bitvector;
  for(j=0; j<MAX_OBJ_AFFECT; j++)
    oe->affected[j] = obj->affected[j];

  st->nobjects++;
}

static int contained_weight(struct obj_data *container)
{
  struct obj_data *tmp;
  int	rval = 0;

  for (tmp = container->contains; tmp; tmp = tmp->next_content)
    rval += GET_OBJ_WEIGHT(tmp);
  return rval;
}

/* Destroy inventory after transferring it to "store inventory" */
void obj_to_store(struct obj_data *obj, struct obj_file_u *st,struct char_data * ch,int delete)
{
  static char buf[240];
  
  if (!obj) return;

  obj_to_store(obj->contains, st, ch, delete);
  obj_to_store(obj->next_content, st, ch, delete);
    
  if((obj->obj_flags.timer < 0) && (obj->obj_flags.timer != OBJ_NOTIMER)) 
  {
    if(delete) 
    {
      sprintf(buf, 
        "You're told: '%s is just old junk, I'll throw it away for you.'\n\r",
         obj->short_description);
      send_to_char(buf, ch);
      if(obj->in_obj) obj_from_obj(obj);
        obj_from_char(obj);
      extract_obj(obj);
    }
  }
  else
  if(obj->obj_flags.cost_per_day < 0) 
  {
    if(delete) 
    {
      sprintf(buf,
        "You're told: '%s is just old junk, I'll throw it away for you.'\n\r",
        obj->short_description);
      send_to_char(buf, ch);
      if(obj->in_obj) obj_from_obj(obj);
        obj_from_char(obj);
      extract_obj(obj);
    }
  }
  else
  if(obj->item_number == -1) 
  {
    if(delete) 
    {
      if (obj->in_obj) 
	 obj_from_obj(obj);
      obj_from_char(obj);
      extract_obj(obj);
    }
  }
  else
  {
    int weight = contained_weight(obj);
    GET_OBJ_WEIGHT(obj) -= weight;
    put_obj_in_store(obj, st);
    GET_OBJ_WEIGHT(obj) += weight;
    if (delete) 
    {
      if (obj->in_obj)
	obj_from_obj(obj);
      extract_obj(obj);
    }
  }
}

/* write the vital data of a player to the player file */
void save_obj(struct char_data *ch, struct obj_cost *cost, int delete)
{
  static struct obj_file_u st;
  FILE *fl;
  int pos, i, j;
  bool found = FALSE;
  char name[40];
  char * t_ptr;
  char path[256];
  
  zero_rent(ch);
  
  st.nobjects = 0;
  st.gold_left = GET_GOLD(ch);
  st.total_cost = cost->total_cost;
  st.last_update = time(0);
  st.minimum_stay = 0; /* XXX where does this belong? */
  
  for(i=0; i<MAX_WEAR; i++)
  if (ch->equipment[i]) 
  {
    if(delete) 
    {
      obj_to_store(unequip_char(ch, i), &st, ch, delete);
    }
    else
    {
      obj_to_store(ch->equipment[i], &st, ch, delete);
    }
  }

  obj_to_store(ch->carrying, &st, ch, delete);

  if(delete)
     ch->carrying = 0;
 
  strcpy(name,GET_NAME(ch));
  t_ptr = name;
  for(;*t_ptr != '\0';t_ptr++)
    *t_ptr = LOWER(*t_ptr);

  sprintf(path,"ply/%s.o",name);
  if(!(fl = fopen(path, "w+b")))	
  {
    perror("saving PC's objects");
    exit(1);
  }
  update_file(fl, name , &st);
  fclose(fl);
}

/* ************************************************************************
* Routines used to update object file, upon boot time                     *
************************************************************************* */

void update_obj_file(void)
{
  FILE *fl, *char_file;
  struct obj_file_u st;
  struct rental_header rh;
  struct char_file_u ch_st;
  struct char_data tmp_char;
  int pos, no_read, player_i;
  long days_passed, secs_lost;
  char buf[MAX_STRING_LENGTH];
  
  int find_name(char *name);
  extern struct player_index_element *player_table;
  extern int errno;
  
  if (!(char_file = fopen(PLAYER_FILE, "r+b"))) 
  {
    perror("Opening player file for reading. (reception.c, update_obj_file)");
    exit(1);
  }
  
  /* r+b is for Binary Reading/Writing */
  if (!(fl = fopen(OBJ_SAVE_FILE, "r+b")))
  {
    perror("   Opening object file for updating");
    exit(1);
  }
  
  pos = 0;
  errno = 0;
  while (!feof(fl)) 
  {
    /* read a rental header */
    no_read = fread(&rh, sizeof(rh), 1, fl);
    if (no_read==0)
      break;
    if(no_read!=1) 
    {
      perror("corrupted object save file 1");
      exit(1);
    }

    if(!rh.inuse) 
    {
/*      sprintf(buf, "   skipping hole size %d.", rh.length);
      log(buf);
*/
      if (fseek(fl, rh.length, 1)) 
      {
	perror("corrupted object save file 2");
	exit(1);
      }
      continue;
    }
    /* read in the char part of the rental data */
    no_read = fread(&st, (rh.length>sizeof(st))?sizeof(st):rh.length, 1, fl);
    if (rh.length>sizeof(st)) 
    {
      st.nobjects = MAX_OBJ_SAVE;
      fseek(fl, rh.length - sizeof(st), 1);
    }

    if (no_read!=1) 
    {
      perror("corrupted object save file 3");
      exit(1);
    }

    pos += no_read;
    
    if ((!feof(fl)) && (no_read > 0) && rh.owner[0]) 
    {
      /*
      sprintf(buf, "   Processing %s[%d].", rh.owner, pos);
      log(buf);
      */
      days_passed = ((time(0) - st.last_update) / SECS_PER_REAL_DAY);
      secs_lost = ((time(0) - st.last_update) % SECS_PER_REAL_DAY);
      
      /* load player record too, probably kinda slow :( */
      if ((player_i = find_name(rh.owner)) < 0) 
      {
	perror("   Character not in list. (update_obj_file)");

	/*
	  seek backwards to beginning of player record.
	  set rh.inuse to 0
	  seek forwards to beginning of next record
	*/

	fseek(fl, -rh.length - sizeof(rh), 1);
	rh.inuse = 0;
	fwrite(&rh, sizeof(rh), 1, fl);
	fseek(fl, rh.length, 1);

	continue;
	exit(1);
      }

      fseek(char_file, (long) (player_table[player_i].nr*sizeof(struct char_file_u)), 0);
      fread(&ch_st, sizeof(struct char_file_u), 1, char_file);

      if(ch_st.load_room == NOWHERE) 
      {
	/* reset last_update_time so they have a grace period from the time the game came *UP*. 
	log("     a game crash victim."); */
	st.last_update = time(0);
	fseek(fl, -rh.length, 1);
	fwrite(&st, rh.length, 1, fl);
	fseek(fl, 0, 1);
	continue; /* next record */
      }

      if(days_passed > 0) 
      {
	if((st.total_cost*days_passed) > st.gold_left) 
	{
	  sprintf(buf, "   Dumping %s from object file.", ch_st.name);
	  log(buf);
	  ch_st.points.gold = 0;
	  ch_st.load_room = NOWHERE;
	  fseek(char_file,(long) (player_table[player_i].nr*sizeof(struct char_file_u)), 0);
	  fwrite(&ch_st, sizeof(struct char_file_u), 1, char_file);
	  fseek(fl, -rh.length - sizeof(rh), 1);
	  rh.inuse = 0;
	  fwrite(&rh, sizeof(rh), 1, fl);
	  fseek(fl, rh.length, 1);
	}
	else
	{
	  /*
	    sprintf(buf, "   Updating %s", rh.owner);
	    log(buf);
	    st.gold_left  -= (st.total_cost*days_passed);
	  */
	  st.last_update = time(0)-secs_lost;
	  fseek(fl, -rh.length, 1);
	  fwrite(&st, rh.length, 1, fl);
	  fseek(fl, 0, 1);
	}
      }
    }
  }
  fclose(fl);
  fclose(char_file);
}


/* ************************************************************************
* Routine Receptionist                                                    *
************************************************************************* */

int receptionist(struct char_data *ch, int cmd, char *arg)
{
  char buf[240];
  struct obj_cost cost;
  struct char_data *recep = 0;
  struct char_data *temp_char;
  sh_int save_room;
  sh_int action_tabel[9] = {23,24,36,105,106,109,111,142,147};
  
  void do_action(struct char_data *ch, char *argument, int cmd);
  int number(int from, int to);
  int citizen(struct char_data *ch, int cmd, char *arg);
  
  if (!ch->desc)
    return(FALSE); /* You've forgot FALSE - NPC couldn't leave */
  
  for (temp_char = real_roomp(ch->in_room)->people; (temp_char) && (!recep);
       temp_char = temp_char->next_in_room)
    if (IS_MOB(temp_char))
      if (mob_index[temp_char->nr].func == receptionist)
	recep = temp_char;
  
  if (!recep) {
    log("No receptionist.\n\r");
    exit(1);
  }
  
  if (IS_NPC(ch))
    return(FALSE);
  
  if ((cmd !=256) && (cmd != 92) && (cmd != 93)) 
  {
    if (!cmd) 
    {
      if (recep->specials.fighting) 
      {
	return(citizen(recep,0,""));
      }
    }
    if (!number(0, 30))
      do_action(recep, "", action_tabel[number(0,8)]);
    return(FALSE);
  }
  
  if (!AWAKE(recep)) 
  {
    act("$e isn't able to talk to you...", FALSE, recep, 0, ch, TO_VICT);
    return(TRUE);
  }
  
  if (!CAN_SEE(recep, ch)) 
  {
    act("$n says, 'I don't deal with people I can't see!'", FALSE, recep, 0, 0, TO_ROOM);
    return(TRUE);
  }
 
  switch(cmd)
  {
    case 92:
    { /* Rent  */
      if (recep_offer(ch, recep, &cost)) 
      {
	GET_HOME(ch) = ch->in_room;
        sprintf(buf,"Your home has been set to this room, %s\n\r",real_roomp(ch->in_room)->name);
        send_to_char(buf,ch);

        act("$n stores your stuff in the safe, and helps you into your chamber.",
	  FALSE, recep, 0, ch, TO_VICT);
        act("$n helps $N into $S private chamber.",FALSE, recep,0,ch,TO_NOTVICT);
      
        save_obj(ch,&cost,1);
        save_room = ch->in_room;
        extract_char(ch);
        ch->in_room = save_room;
        save_char(ch, ch->in_room);
      }
    }
    break;
    case 93:
    {         /* Offer */
      recep_offer(ch, recep, &cost);
      act("$N gives $n an offer.", FALSE, ch, 0, recep, TO_ROOM);
    }
    break;
    case 256:
    {		/* sethome */
      GET_HOME(ch) = ch->in_room;
      save_char(ch,ch->in_room);
      sprintf(buf,"Your home has been set to this room, %s\n\r",real_roomp(ch->in_room)->name);
      send_to_char(buf,ch);
    }
    break;
  }
  return(TRUE);
}

/* removes a player from the list of renters */
void zero_rent( struct char_data *ch) 
{
  struct rental_header rh;
  FILE *fl;
  char name[40];
  char * t_ptr;
  char path[256];

  if(IS_NPC(ch)) return;
  
  strcpy(name,GET_NAME(ch));
  t_ptr = name;
  for(;*t_ptr != '\0'; t_ptr++)
    *t_ptr = LOWER(*t_ptr);
  sprintf(path,"ply/%s.o",name);
  if (!(fl = fopen(path, "w+b")))
  {
    perror("saving PC's objects");
    exit(1);
  }
  rh.inuse = 0;
  fwrite(&rh, sizeof(rh), 1, fl);
  fclose(fl);
  return;
}

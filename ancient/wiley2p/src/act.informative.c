/* ************************************************************************
 *  file: act.informative.c , Implementation of commands.  Part of DIKUMUD *
 *  Usage : Informative commands.                                          *
 *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
 ************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "race.h"
#include "trap.h"
#include "hash.h"

/* extern variables */

extern struct hash_header room_db;
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct str_app_type str_app[31];
extern char * spells[];
extern const struct title_type titles[6][ABS_MAX_LVL];

extern int  top_of_world;
extern int  top_of_zone_table;
extern int  top_of_mobt;
extern int  top_of_objt;
extern int  top_of_p_table;
extern int DEBUG;

extern char credits[MAX_STRING_LENGTH];
extern char news[MAX_STRING_LENGTH];
extern char info[MAX_STRING_LENGTH];
extern char wizlist[MAX_STRING_LENGTH];
extern char *dirs[]; 
extern char *where[];
extern char *color_liquid[];
extern char *fullness[];
extern const char *RaceName[];
extern const int RacialMax[][4];
extern const char *connected_types[];

/* extern functions */

struct time_info_data age(struct char_data *ch);
void page_string(struct descriptor_data *d, char *str, int keep_internal);
int track( struct char_data *ch, struct char_data *vict);

/* intern functions */

void list_obj_to_char(struct obj_data *list,struct char_data *ch, int mode,
		      bool show);


/* Procedures related to 'look' */

void argument_split_2(char *argument, char *first_arg, char *second_arg) {
  int look_at, found, begin;
  found = begin = 0;
   if(DEBUG) dlog("argument_split_2"); 
  /* Find first non blank */
  for ( ;*(argument + begin ) == ' ' ; begin++);
  
  /* Find length of first word */
  for (look_at=0; *(argument+begin+look_at) > ' ' ; look_at++)
    
    /* Make all letters lower case, AND copy them to first_arg */
    *(first_arg + look_at) = LOWER(*(argument + begin + look_at));
  *(first_arg + look_at) = '\0';
  begin += look_at;
  
  /* Find first non blank */
  for ( ;*(argument + begin ) == ' ' ; begin++);
  
  /* Find length of second word */
  for ( look_at=0; *(argument+begin+look_at)> ' ' ; look_at++)
    
    /* Make all letters lower case, AND copy them to second_arg */
    *(second_arg + look_at) = LOWER(*(argument + begin + look_at));
  *(second_arg + look_at)='\0';
  begin += look_at;
}

struct obj_data *get_object_in_equip_vis(struct char_data *ch,
       		 char *arg, struct obj_data *equipment[], int *j) {
  
  if(DEBUG) dlog("get_object_in_equip_vis");
  for ((*j) = 0; (*j) < MAX_WEAR ; (*j)++)
    if (equipment[(*j)])
      if (CAN_SEE_OBJ(ch,equipment[(*j)]))
	if (isname(arg, equipment[(*j)]->name))
	  return(equipment[(*j)]);
  
  return (0);
}

char *find_ex_description(char *word, struct extra_descr_data *list)
{
  struct extra_descr_data *i;
  if(DEBUG) dlog("find_ex_description");  
  for (i = list; i; i = i->next)
    if (isname(word,i->keyword))
      return(i->description);
  
  return(0);
}


void show_obj_to_char(struct obj_data *object, struct char_data *ch, int mode)
{
  char buffer[MAX_STRING_LENGTH] = "\0";
  if(DEBUG) dlog("show_obj_to_char"); 
  if ((mode == 0) && object->description)
    strcpy(buffer, object->description);	    
  else if (object->short_description && ((mode == 1) ||
					 (mode == 2) || (mode==3) || (mode == 4))) 
    strcpy(buffer,object->short_description);
  else if (mode == 5) 
  {
    if (object->obj_flags.type_flag == ITEM_NOTE)  	
    {
      if (object->action_description)	 
      {
	strcpy(buffer, "There is writing on it:\n\r\n\r");
	strcat(buffer, object->action_description);
	/* page_string(ch->desc, buffer, 1); */
      } 
      else
      {
	act("It is blank.", FALSE, ch,0,0,TO_CHAR);
      }
    }
    else 
    if((object->obj_flags.type_flag != ITEM_DRINKCON)) 
    {
      strcpy(buffer,"You see nothing special..");
    }  else  { /* ITEM_TYPE == ITEM_DRINKCON */
      strcpy(buffer, "It looks to be a drink container.");
    }
  }
  
  if (mode != 3) 
  { 
    if (object->obj_flags.type_flag == ITEM_ARMOR) 
    {
      if (object->obj_flags.value[0]<(object->obj_flags.value[1]/6))
	strcat(buffer, "..it is extremely beaten on.");
      else
      if (object->obj_flags.value[0]<(object->obj_flags.value[1]/5))
	strcat(buffer, "..it is severely beaten on.");
      else
      if (object->obj_flags.value[0]<(object->obj_flags.value[1]/4))
	strcat(buffer, "..it is badly beaten on.");
      else
      if (object->obj_flags.value[0]<(object->obj_flags.value[1] / 3)) 
	strcat(buffer, "..it looks barely useable.");
      else
      if (object->obj_flags.value[0] < (object->obj_flags.value[1] / 2)) 
	strcat(buffer, "..it is showing wear.");
      else
      if (object->obj_flags.value[0] < object->obj_flags.value[1])
	strcat(buffer, "..it's in fair shape.");
      else
	strcat(buffer, "");
    }
    if(IS_OBJ_STAT(object,ITEM_ANTI_GOOD)&&IS_AFFECTED(ch,AFF_DETECT_EVIL)) 
      strcat(buffer,"..it glows red!");
    if (IS_OBJ_STAT(object,ITEM_MAGIC) && IS_AFFECTED(ch,AFF_DETECT_MAGIC)) 
      strcat(buffer,"..it glows blue!");
    if (IS_OBJ_STAT(object,ITEM_GLOW)) 
      strcat(buffer,"..it glows softly!");
    if (IS_OBJ_STAT(object,ITEM_HUM)) 
      strcat(buffer,"..it emits a faint hum!");
    if (IS_OBJ_STAT(object,ITEM_INVISIBLE))
      strcat(buffer,"(invisible)");
  }
  strcat(buffer, "\n\r");
  page_string(ch->desc, buffer, 1);
}

void show_mult_obj_to_char(struct obj_data *object, struct char_data *ch, int mode, int num)
{
  char buffer[MAX_STRING_LENGTH] = "\0";
  char tmp[10] = "\0";
  if(DEBUG) dlog("show_mult_obj_to_char"); 
  if ((mode == 0) && object->description)
    strcpy(buffer,object->description);
  else 	if (object->short_description && ((mode == 1) ||
					  (mode == 2) || (mode==3) || (mode == 4))) 
    strcpy(buffer,object->short_description);
  else if (mode == 5) {
    if (object->obj_flags.type_flag == ITEM_NOTE)  	{
      if (object->action_description)	 {
	strcpy(buffer, "There is writing on it:\n\r\n\r");
	strcat(buffer, object->action_description);
	page_string(ch->desc, buffer, 1);
      }  else
	act("It's blank.", FALSE, ch,0,0,TO_CHAR);
      return;
    } else if((object->obj_flags.type_flag != ITEM_DRINKCON)) {
      strcpy(buffer,"You see nothing special..");
    }  else  { /* ITEM_TYPE == ITEM_DRINKCON */
      strcpy(buffer, "It looks like a drink container.");
    }
  }
  
  if (mode != 3) { 
    if (IS_OBJ_STAT(object,ITEM_INVISIBLE)) {
      strcat(buffer,"(invisible)");
    }
    if (IS_OBJ_STAT(object,ITEM_ANTI_GOOD) && 
	IS_AFFECTED(ch,AFF_DETECT_EVIL)) {
      strcat(buffer,"..it glows red!");
    }
    if (IS_OBJ_STAT(object,ITEM_MAGIC) && IS_AFFECTED(ch,AFF_DETECT_MAGIC)) {
      strcat(buffer,"..it glows blue!");
    }
    if (IS_OBJ_STAT(object,ITEM_GLOW)) {
      strcat(buffer,"..it glows softly!");
    }
    if (IS_OBJ_STAT(object,ITEM_HUM)) {
      strcat(buffer,"..it emits a faint hum!");
    }
  }
  
  if (num>1) {
    sprintf(tmp,"[%d]", num);
    strcat(buffer, tmp);
  }
  strcat(buffer, "\n\r");
  page_string(ch->desc, buffer, 1);
}

void list_obj_in_room(struct obj_data *list, struct char_data *ch)
{

  struct obj_data *i, *cond_ptr[50];
  int Inventory_Num = 1, num;
  int k, cond_top, cond_tot[50], found=FALSE;
  char buf[MAX_STRING_LENGTH];

  if(DEBUG) dlog("list_obj_in_room");
  
  cond_top = 0; 
  
  for (i=list; i; i = i->next_content) {
    if (CAN_SEE_OBJ(ch, i)) {
      if (cond_top< 50) {
	found = FALSE;
	for (k=0;(k<cond_top&& !found);k++) {
	  if (cond_top>0) {
	    if ((i->item_number == cond_ptr[k]->item_number) &&
		(i->description && cond_ptr[k]->description &&
		 !strcmp(i->description,cond_ptr[k]->description))){
	      cond_tot[k] += 1;
	      found=TRUE;
	    }
	  }          
	}
	if (!found) {
	  cond_ptr[cond_top] = i;
	  cond_tot[cond_top] = 1;
	  cond_top+=1;
	}
      }
      else 
      {
	if((ITEM_TYPE(i) == ITEM_TRAP) || (GET_TRAP_CHARGES(i) > 0)) 
        {
       	  num = number(1,100);
       	  if (num < ch->skills[SKILL_FIND_TRAP].learned/3)
	    show_obj_to_char(i,ch,0);
        }
        else 
        {
	  show_obj_to_char(i,ch,0);	  
	}
      }
    }
  }
  
  if (cond_top) 
  {
    for (k=0; k<cond_top; k++) 
    {
      if((ITEM_TYPE(cond_ptr[k]) == ITEM_TRAP) && (GET_TRAP_CHARGES(cond_ptr[k]) > 0)) 
      {
	num = number(1,100);
	if(num < ch->skills[SKILL_FIND_TRAP].learned/3)
	  if (cond_tot[k] > 1) 
	  {
	    sprintf(buf,"[%2d] ",Inventory_Num++);
	    send_to_char(buf,ch);
	    show_mult_obj_to_char(cond_ptr[k],ch,0,cond_tot[k]);
	  }
	  else
	  {
	    show_obj_to_char(cond_ptr[k],ch,0);
	  }
      }
      else 
      {
	if (cond_tot[k] > 1) 
	{
	  sprintf(buf,"[%2d] ",Inventory_Num++);
	  send_to_char(buf,ch);
	  show_mult_obj_to_char(cond_ptr[k],ch,0,cond_tot[k]);
	} else {
	  show_obj_to_char(cond_ptr[k],ch,0);
	}
      }
    }
  }
}

void list_obj_on_char(struct obj_data *list, struct char_data *ch)
{
  struct obj_data *i, *cond_ptr[50];
  int k, cond_top, cond_tot[50], found=FALSE;  
  char buf[MAX_STRING_LENGTH];
  
  int Num_Inventory = 1;
  cond_top = 0; 
 
  if(DEBUG) dlog("list_obj_on_char");
  if(!list)
  {
    send_to_char("   Nothing\n\r",ch);
    return;
  }

  for (i=list; i; i = i->next_content) 
  {
    if (CAN_SEE_OBJ(ch, i)) 
    {
      if (cond_top< 50) 
      {
	found = FALSE;
        for (k=0;(k<cond_top&& !found);k++) 
        {
          if (cond_top>0) 
          {
            if((i->item_number == cond_ptr[k]->item_number) &&
	       (i->short_description && cond_ptr[k]->short_description &&
	       (!strcmp(i->short_description,cond_ptr[k]->short_description))))
            {
	      cond_tot[k] += 1;
	      found=TRUE;
	    }
	  }        
	}
	if (!found) 
        {
	  cond_ptr[cond_top] = i;
	  cond_tot[cond_top] = 1;
	  cond_top+=1;
	}
      }
      else
      {
	show_obj_to_char(i,ch,2);
      }
    }
  }
  
  if (cond_top) 
  {
    for (k=0; k<cond_top; k++) 
    {
/*
      sprintf(buf,"[%2d] ",Num_Inventory++);
      send_to_char(buf,ch);
*/
      if (cond_tot[k] > 1) 
      {
	Num_Inventory += cond_tot[k] - 1;
	show_mult_obj_to_char(cond_ptr[k],ch,2,cond_tot[k]);
      }
      else
      {
	show_obj_to_char(cond_ptr[k],ch,2);
      }	
    }
  }
}

void list_obj_to_char(struct obj_data *list,struct char_data *ch, int mode, 
		      bool show) {
  char buf[MAX_STRING_LENGTH];
  int Num_In_Bag = 1;
  struct obj_data *i;
  bool found;
  if(DEBUG) dlog("list_obj_on_char");  
  found = FALSE;
  for ( i = list ; i ; i = i->next_content ) { 
    if (CAN_SEE_OBJ(ch,i)) {
      sprintf(buf,"[%2d] ",Num_In_Bag++);
      send_to_char(buf,ch);
      show_obj_to_char(i, ch, mode);
      found = TRUE;
    }    
  }  
  if ((! found) && (show)) send_to_char("Nothing\n\r", ch);
}
 
void show_char_to_char(struct char_data *i, struct char_data *ch, int mode)
{
  char buffer[MAX_STRING_LENGTH];
  int j, found, percent;
  struct obj_data *tmp_obj;
  if(DEBUG) dlog("show_char_to_char");  
  if (mode == 0) 
  {
    if (IS_AFFECTED(i, AFF_HIDE) || !CAN_SEE(ch,i)) 
    {
      if (IS_AFFECTED(ch, AFF_SENSE_LIFE))
	send_to_char("You sense a hidden life form in the room.\n\r", ch);
      return;
    }
    
    if (!(i->player.long_descr)||(GET_POS(i) != i->specials.default_pos))
    {
      /* A player char or a mobile without long descr, or not in default pos. */
      if(IS_PC(i))
      {	
	strcpy(buffer,GET_NAME(i));
	strcat(buffer," ");
	if (GET_TITLE(i))
	  strcat(buffer,GET_TITLE(i));
      }
      else
      {
	strcpy(buffer, i->player.short_descr);
	CAP(buffer);
      }
      
      if(IS_AFFECTED(i,AFF_INVISIBLE))
	strcat(buffer," (invisible)");
      if(IS_AFFECTED(i,AFF_CHARM))
	strcat(buffer," (pet)");

      switch(GET_POS(i)) 
      {
      case POSITION_STUNNED  : 
	strcat(buffer," is lying here, stunned"); break;
      case POSITION_INCAP    : 
	strcat(buffer," is lying here, incapacitated"); break;
      case POSITION_MORTALLYW: 
	strcat(buffer," is lying here, mortally wounded"); break;
      case POSITION_DEAD     : 
	strcat(buffer," is lying here, dead"); break;
      case POSITION_MOUNTED  :
        if (MOUNTED(i)) 
        {
          strcat(buffer, " is here, riding ");
          strcat(buffer, MOUNTED(i)->player.short_descr);
        }
        else
          strcat(buffer, " is standing here.");
        break;
      case POSITION_STANDING : 
	strcat(buffer," is standing here"); break;
      case POSITION_SITTING  : 
	strcat(buffer," is sitting here");  break;
      case POSITION_RESTING  : 
	strcat(buffer," is resting here");  break;
      case POSITION_SLEEPING : 
	strcat(buffer," is sleeping here"); break;
      case POSITION_FIGHTING :
	if (i->specials.fighting) 
	{
	  strcat(buffer," is here, fighting ");
	  if (i->specials.fighting == ch)
	    strcat(buffer," YOU!");
	  else {
	    if (i->in_room == i->specials.fighting->in_room)
	      if (IS_NPC(i->specials.fighting))
		strcat(buffer, i->specials.fighting->player.short_descr);
	      else
		strcat(buffer, GET_NAME(i->specials.fighting));
	    else
	      strcat(buffer, "someone who has already left.");
	  }
	}
	else /* NIL fighting pointer */
	  strcat(buffer," is here struggling with thin air");
	break;

      default : strcat(buffer," is floating here"); break;
      }

      if(IS_AFFECTED(i,AFF_FLYING))
      {
	strcat(buffer," inches above the ground");
      }
      strcat(buffer,".");

      if (IS_AFFECTED(ch, AFF_DETECT_EVIL)) 
      {
	if(IS_REALLY_VILE(i) || IS_VILE(i) || IS_EVIL(i))
	  strcat(buffer, " (Red Aura)");
      }
      
      strcat(buffer,"\n\r");
      send_to_char(buffer, ch);
    }
    else
    {  /* npc with long */
      if(IS_AFFECTED(i,AFF_INVISIBLE))
	strcpy(buffer,"*");
      else
	*buffer = '\0';
      
      if (IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
	if(IS_REALLY_VILE(i) || IS_VILE(i) || IS_EVIL(i))
	  strcat(buffer, " (Red Aura)");
      }
      
      strcat(buffer, i->player.long_descr);
      
      send_to_char(buffer, ch);
    }
    
    if (IS_AFFECTED(i,AFF_SANCTUARY))
      act("$n glows with a bright light!", FALSE, i, 0, ch, TO_VICT);
    if (affected_by_spell(i, SPELL_FIRESHIELD)) 
      act("$n is surrounded by flickering flames!", FALSE, i, 0, ch, TO_VICT);
    
  }
  else
  if (mode == 1) 
  {
    if (i->player.description)
      send_to_char(i->player.description, ch);
    else {
      act("You see nothing special about $m.", FALSE, i, 0, ch, TO_VICT);
    }

    if(IS_PC(i))
      sprintf(buffer,"%s is %s.\n\r",GET_NAME(i),RaceName[GET_RACE(i)]);
    else
      sprintf(buffer,"%s is %s.\n\r",MOB_NAME(i),RaceName[GET_RACE(i)]);

    send_to_char(buffer,ch);
    
    if (MOUNTED(i)) 
    {
      sprintf(buffer,"$n is mounted on %s", MOUNTED(i)->player.short_descr);
      act(buffer, FALSE, i, 0, ch, TO_VICT);
    } 
    
    if (RIDDEN(i))
    {
      if(CAN_SEE(ch,RIDDEN(i)))
      {
        sprintf(buffer,"$n is ridden by %s",
         IS_NPC(RIDDEN(i))?RIDDEN(i)->player.short_descr:GET_NAME(RIDDEN(i)));
        act(buffer, FALSE, i, 0, ch, TO_VICT);
      }
    } 
 
    /* Show a character to another */
    
    if (GET_MAX_HIT(i) > 0)
      percent = (100*GET_HIT(i))/GET_MAX_HIT(i);
    else
      percent = -1; /* How could MAX_HIT be < 1?? */
    
    if (IS_NPC(i))
      strcpy(buffer, i->player.short_descr);
    else
      strcpy(buffer, GET_NAME(i));
    
    if (percent >= 100)
      strcat(buffer, " is in an excellent condition.\n\r");
    else if (percent >= 90)
      strcat(buffer, " has a few scratches.\n\r");
    else if (percent >= 75)
      strcat(buffer, " has some small wounds and bruises.\n\r");
    else if (percent >= 50)
      strcat(buffer, " has quite a few wounds.\n\r");
    else if (percent >= 30)
      strcat(buffer, " has some big nasty wounds and scratches.\n\r");
    else if (percent >= 15)
      strcat(buffer, " looks pretty hurt.\n\r");
    else if (percent >= 0)
      strcat(buffer, " is in an awful condition.\n\r");
    else
      strcat(buffer, " is bleeding awfully from big wounds.\n\r");
    
    send_to_char(buffer, ch);
    

    found = FALSE;
    for (j=0; j< MAX_WEAR; j++) {
      if (i->equipment[j]) {
	if (CAN_SEE_OBJ(ch,i->equipment[j])) {
	  found = TRUE;
	}
      }
    }
    if (found) {
      act("\n\r$n is using:", FALSE, i, 0, ch, TO_VICT);
      for (j=0; j< MAX_WEAR; j++) {
	if (i->equipment[j]) {
	  if (CAN_SEE_OBJ(ch,i->equipment[j])) {
	    send_to_char(where[j],ch);
	    show_obj_to_char(i->equipment[j],ch,1);
	  }
	}
      }
    }
    if (HasClass(ch, CLASS_THIEF) && (ch != i) &&
	(!IS_IMMORTAL(ch))){
      found = FALSE;
      send_to_char
	("\n\rYou attempt to peek at their inventory:\n\r", ch);
      for(tmp_obj = i->carrying; tmp_obj; 
	  tmp_obj = tmp_obj->next_content) {
	if (CAN_SEE_OBJ(ch, tmp_obj) && 
	    (number(0,MAX_MORT) < GetMaxLevel(ch))) {
	  show_obj_to_char(tmp_obj, ch, 1);
	  found = TRUE;
	}
      }
      if (!found)
	send_to_char("You can't see anything.\n\r", ch);
    } else if (IS_IMMORTAL(ch)) {
      send_to_char("Inventory:\n\r",ch);
      for(tmp_obj = i->carrying; tmp_obj; 
	  tmp_obj = tmp_obj->next_content) {
	show_obj_to_char(tmp_obj, ch, 1);
	found = TRUE;
      }
      if (!found) {
	send_to_char("Nothing\n\r",ch);
      }
    }
    
  } else if (mode == 2) {
    
    /* Lists inventory */
    act("$n is carrying:", FALSE, i, 0, ch, TO_VICT);
    list_obj_on_char(i->carrying,ch);
  }
}


void show_mult_char_to_char(struct char_data *i, struct char_data *ch, int mode, int num)
{
  char buffer[MAX_STRING_LENGTH];
  char tmp[10];
  int j, found, percent;
  struct obj_data *tmp_obj;

  if(DEBUG) dlog("show_mult_char_to_char");
  if (mode == 0) {
    if (IS_AFFECTED(i, AFF_HIDE) || !CAN_SEE(ch,i)) {
      if (IS_AFFECTED(ch, AFF_SENSE_LIFE))
	if (num==1)
	  send_to_char("You sense a hidden life form in the room.\n\r", ch);
	else 
	  send_to_char("You sense hidden life forma in the room.\n\r", ch);		
      return;
    }
    
    if (!(i->player.long_descr)||(GET_POS(i) != i->specials.default_pos)){
      /* A player char or a mobile without long descr, or not in default pos. */
      if (!IS_NPC(i)) {	
	strcpy(buffer,GET_NAME(i));
	strcat(buffer," ");
	if (GET_TITLE(i))
	  strcat(buffer,GET_TITLE(i));
      } else {
	strcpy(buffer, i->player.short_descr);
	CAP(buffer);
      }
      
      if ( IS_AFFECTED(i,AFF_INVISIBLE))
	strcat(buffer," (invisible)");
      if ( IS_AFFECTED(i,AFF_CHARM))
	strcat(buffer," (pet)");
      
      switch(GET_POS(i)) {
      case POSITION_STUNNED  : 
	strcat(buffer," is lying here, stunned"); break;
      case POSITION_INCAP    : 
	strcat(buffer," is lying here, incapacitated"); break;
      case POSITION_MORTALLYW: 
	strcat(buffer," is lying here, mortally wounded"); break;
      case POSITION_DEAD     : 
	strcat(buffer," is lying here, dead"); break;
      case POSITION_STANDING : 
	strcat(buffer," is standing here"); break;
      case POSITION_SITTING  : 
	strcat(buffer," is sitting here");  break;
      case POSITION_RESTING  : 
	strcat(buffer," is resting here");  break;
      case POSITION_SLEEPING : 
	strcat(buffer," is sleeping here"); break;
      case POSITION_FIGHTING :
	if (i->specials.fighting) {
	  
	  strcat(buffer," is here, fighting ");
	  if (i->specials.fighting == ch)
	    strcat(buffer," YOU!");
	  else {
	    if (i->in_room == i->specials.fighting->in_room)
	      if (IS_NPC(i->specials.fighting))
		strcat(buffer, i->specials.fighting->player.short_descr);
	      else
		strcat(buffer, GET_NAME(i->specials.fighting));
	    else
	      strcat(buffer, "someone who has already left.");
	  }
	} else /* NIL fighting pointer */
	  strcat(buffer," is here struggling with thin air");
	break;

	default : strcat(buffer," is floating here."); break;
      }
      if(IS_AFFECTED(i,AFF_FLYING))
	strcat(buffer," inches above the ground");
      strcat(buffer,".");

      if (IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
	if(IS_REALLY_VILE(i) || IS_VILE(i) || IS_EVIL(i))
	  strcat(buffer, " (Red Aura)");
      }
      
      if (num > 1) {
	sprintf(tmp," [%d]", num);
	strcat(buffer, tmp);
      }
      strcat(buffer,"\n\r");
      send_to_char(buffer, ch);
    } else {  /* npc with long */
      
      if (IS_AFFECTED(i,AFF_INVISIBLE))
	strcpy(buffer,"*");
      else
	*buffer = '\0';
      
      if (IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
	if(IS_REALLY_VILE(i) || IS_VILE(i) || IS_EVIL(i))
	  strcat(buffer, " (Red Aura)");
      }
      
      strcat(buffer, i->player.long_descr);
      
      /* this gets a little annoying */
      
      if (num > 1) {
	while ((buffer[strlen(buffer)-1]=='\r') ||
	       (buffer[strlen(buffer)-1]=='\n') ||
	       (buffer[strlen(buffer)-1]==' ')) {
	  buffer[strlen(buffer)-1] = '\0';
	}
	sprintf(tmp," [%d]\n\r", num);
	strcat(buffer, tmp);
      }
      
      send_to_char(buffer, ch);
    }
    
    if (IS_AFFECTED(i,AFF_SANCTUARY))
      act("$n glows with a bright light!", FALSE, i, 0, ch, TO_VICT);
    if (affected_by_spell(i, SPELL_FIRESHIELD)) 
      act("$n is surrounded by flickering flames!", FALSE, i, 0, ch, TO_VICT);
    
  } else if (mode == 1) {
    
    if (i->player.description)
      send_to_char(i->player.description, ch);
    else {
      act("You see nothing special about $m.", FALSE, i, 0, ch, TO_VICT);
    }
    
    /* Show a character to another */
    
    if (GET_MAX_HIT(i) > 0)
      percent = (100*GET_HIT(i))/GET_MAX_HIT(i);
    else
      percent = -1; /* How could MAX_HIT be < 1?? */
    
    if (IS_NPC(i))
      strcpy(buffer, i->player.short_descr);
    else
      strcpy(buffer, GET_NAME(i));
    
    if (percent >= 100)
      strcat(buffer, " is in an excellent condition.\n\r");
    else if (percent >= 90)
      strcat(buffer, " has a few scratches.\n\r");
    else if (percent >= 75)
      strcat(buffer, " has some small wounds and bruises.\n\r");
    else if (percent >= 50)
      strcat(buffer, " has quite a few wounds.\n\r");
    else if (percent >= 30)
      strcat(buffer, " has some big nasty wounds and scratches.\n\r");
    else if (percent >= 15)
      strcat(buffer, " looks pretty hurt.\n\r");
    else if (percent >= 0)
      strcat(buffer, " is in an awful condition.\n\r");
    else
      strcat(buffer, " is bleeding awfully from big wounds.\n\r");
    
    send_to_char(buffer, ch);
   
    sprintf(buffer,"%s is %s.\n\r",GET_NAME(i),RaceName[GET_RACE(i)]);

#ifdef 0 
     switch(GET_RACE(i))
     {
      case RACE_HALFBREED	:
	sprintf(buffer,"\n\r");
	break;
      case RACE_HUMAN		:
	sprintf(buffer,"%s is a Human.\n\r",GET_NAME(i));
	break;
      case RACE_ELVEN		:
	sprintf(buffer,"%s is an Elf.\n\r",GET_NAME(i));
	break;
      case RACE_DWARF		:
	sprintf(buffer,"%s is a Dwarf.\n\r",GET_NAME(i));
	break;
      case RACE_HALFLING	:
	sprintf(buffer,"%s is a Halfling\n\r",GET_NAME(i));
	break;
      case RACE_GNOME		:
	sprintf(buffer,"%s is a Gnome\n\r",GET_NAME(i));
	break;
     }
#endif

    send_to_char(buffer,ch);

    found = FALSE;
    for (j=0; j< MAX_WEAR; j++) {
      if (i->equipment[j]) {
	if (CAN_SEE_OBJ(ch,i->equipment[j])) {
	  found = TRUE;
	}
      }
    }
    if (found) {
      act("\n\r$n is using:", FALSE, i, 0, ch, TO_VICT);
      for (j=0; j< MAX_WEAR; j++) {
	if (i->equipment[j]) {
	  if (CAN_SEE_OBJ(ch,i->equipment[j])) {
	    send_to_char(where[j],ch);
	    show_obj_to_char(i->equipment[j],ch,1);
	  }
	}
      }
    }
    if ((HasClass(ch, CLASS_THIEF)) && (ch != i)) {
      found = FALSE;
      send_to_char("\n\rYou attempt to peek at their inventory:\n\r", ch);
      for(tmp_obj = i->carrying; tmp_obj; tmp_obj = tmp_obj->next_content) {
	if (CAN_SEE_OBJ(ch,tmp_obj)&&(number(0,MAX_MORT) < GetMaxLevel(ch))) {
	  show_obj_to_char(tmp_obj, ch, 1);
	  found = TRUE;
	}
      }
      if (!found)
	send_to_char("You can't see anything.\n\r", ch);
    }
    
  } else if (mode == 2) {
    
    /* Lists inventory */
    act("$n is carrying:", FALSE, i, 0, ch, TO_VICT);
    list_obj_on_char(i->carrying,ch);
  }
}


void list_char_in_room(struct char_data *list, struct char_data *ch)
{
  struct char_data *i, *cond_ptr[50];
  int k, cond_top, cond_tot[50], found=FALSE;  
  
  if(DEBUG) dlog("list_char_in_room");
  cond_top = 0; 
  
  for (i=list; i; i = i->next_in_room) 
  {
    if((ch!=i) &&
       (!RIDDEN(i) || (RIDDEN(i)&&!CAN_SEE(ch,RIDDEN(i)))) && 
       (IS_AFFECTED(ch, AFF_SENSE_LIFE) ||
       (CAN_SEE(ch,i) && !IS_AFFECTED(i, AFF_HIDE))) ) 
    {
      if ((cond_top< 50) && !MOUNTED(i))
      {
	found = FALSE;
	if (IS_NPC(i)) {
	  for (k=0;(k<cond_top&& !found);k++) {
	    if (cond_top>0) {
	      if (i->nr == cond_ptr[k]->nr &&
		  (GET_POS(i) == GET_POS(cond_ptr[k])) &&
		  (i->specials.affected_by==cond_ptr[k]->specials.affected_by) &&
		  (i->specials.fighting == cond_ptr[k]->specials.fighting) &&
		  (i->player.short_descr && cond_ptr[k]->player.short_descr &&
		   0==strcmp(i->player.short_descr,cond_ptr[k]->player.short_descr))) {
		cond_tot[k] += 1;
		found=TRUE;
	      }
	    }
	  }
	}
	if (!found) {
	  cond_ptr[cond_top] = i;
	  cond_tot[cond_top] = 1;
	  cond_top+=1;
	}
      }
      else
      {
	show_char_to_char(i,ch,0);
      }
    }
  }
  
  if (cond_top) {
    for (k=0; k<cond_top; k++) {
      if (cond_tot[k] > 1) {
	show_mult_char_to_char(cond_ptr[k],ch,0,cond_tot[k]);
      } else {
	show_char_to_char(cond_ptr[k],ch,0);
      }
    }
  }
}


void list_char_to_char(struct char_data *list, struct char_data *ch, 
		       int mode) {
  struct char_data *i;
  if(DEBUG) dlog("list_char_to_char");  
  for (i = list; i ; i = i->next_in_room) {
    if ( (ch!=i) && (IS_AFFECTED(ch, AFF_SENSE_LIFE) ||
		     (CAN_SEE(ch,i) && !IS_AFFECTED(i, AFF_HIDE))) )
      show_char_to_char(i,ch,0); 
  } 
}

void do_look(struct char_data *ch, char *argument, int cmd)
{
  char buffer[MAX_STRING_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  int keyword_no, res;
  int j, bits, temp;
  bool found;
  struct obj_data *tmp_object, *found_object;
  struct char_data *tmp_char;
  char *tmp_desc;
  static char *keywords[]= { 
    "north",
    "east",
    "south",
    "west",
    "up",
    "down",
    "in",
    "at",
    "",  /* Look at '' case */
    "room",
    "\n" };
 
  if(DEBUG) dlog("do_look");

  if (!ch->desc)
    return;
 
  if (GET_POS(ch) < POSITION_SLEEPING)
    send_to_char("You can't see anything but stars!\n\r", ch);
  else if (GET_POS(ch) == POSITION_SLEEPING)
    send_to_char("You can't see anything, you're sleeping!\n\r", ch);
  else if ( IS_AFFECTED(ch, AFF_BLIND) )
    send_to_char("You can't see a damn thing, you're blinded!\n\r", ch);
  else if  ((IS_DARK(ch->in_room)) && (!IS_IMMORTAL(ch)) &&
	    (!IS_AFFECTED(ch, AFF_TRUE_SIGHT))) {
    send_to_char("It is very dark in here...\n\r", ch);
    if (IS_AFFECTED(ch, AFF_INFRAVISION)) 
    {
      list_char_in_room(real_roomp(ch->in_room)->people, ch);
    }
  } else {

    only_argument(argument, arg1);

    if (0==strn_cmp(arg1,"at",2) && isspace(arg1[2]))
    {
      only_argument(argument+3, arg2);
      keyword_no = 7;
    }
    else
    if (0==strn_cmp(arg1,"in",2) && isspace(arg1[2]))
    {
      only_argument(argument+3, arg2);
      keyword_no = 6;
    } else {
      keyword_no = search_block(arg1, keywords, FALSE);
    }

    if ((keyword_no == -1) && *arg1) {
      keyword_no = 7;
      only_argument(argument, arg2);
    }
    
    found = FALSE;
    tmp_object = 0;
    tmp_char	 = 0;
    tmp_desc	 = 0;
    
    switch(keyword_no) 
    {
      /* look <dir> */
    case 0 :
    case 1 :
    case 2 : 
    case 3 : 
    case 4 :
    case 5 : 
    {   
      struct room_direction_data	*exitp;
      exitp = NULL;
      exitp = EXIT(ch, keyword_no);

      if(exitp != NULL) 
      {
	if(exitp->general_description) 
	  send_to_char(exitp->general_description, ch);
	else 
	  send_to_char("You see nothing special.\n\r", ch);
	
	if(IS_SET(exitp->exit_info, EX_CLOSED) && (exitp->keyword)) 
	{
	  if((strcmp(fname(exitp->keyword), "secret")) &&
	    (IS_NOT_SET(exitp->exit_info, EX_SECRET))) 
	  {
	      sprintf(buffer, "The %s is closed.\n\r",fname(exitp->keyword));
	      send_to_char(buffer, ch);
	  } 
	}
	else
	{
	   if(IS_SET(exitp->exit_info, EX_ISDOOR) && exitp->keyword) 
	   {
	      sprintf(buffer, "The %s is open.\n\r", fname(exitp->keyword));
	      send_to_char(buffer, ch);
	   }
	}
      }
      else 
        if(cmd != SKILL_PEER)
	  send_to_char("You see nothing special.\n\r", ch);

      if((exitp != NULL) &&
	 (IS_AFFECTED(ch, AFF_SCRYING) ||
	 IS_IMMORTAL(ch) ||
	 cmd == SKILL_PEER))
      {
	struct room_data	*rp;
	sprintf(buffer,"You look %swards.\n\r", dirs[keyword_no]);
	send_to_char(buffer, ch);
  
	if(exitp != NULL)
	{
	  if(IS_SET(exitp->exit_info, EX_CLOSED) &&
	    (exitp->keyword) && cmd == SKILL_PEER ) 
	  {
	    if ((strcmp(fname(exitp->keyword), "secret")) &&
	        (IS_NOT_SET(exitp->exit_info, EX_SECRET))) 
	    {
	      sprintf(buffer, "The %s is closed.\n\r",fname(exitp->keyword));
	      send_to_char(buffer, ch);
	      return;
	    } 
          }
        }

	if (!exitp || !exitp->to_room) 
	{
	  send_to_char("You see nothing special.",ch);
	  return;
	}

	rp = real_roomp(exitp->to_room);
	if(!rp) 
	{
	  send_to_char("You see a dark void.\n\r", ch);
	}
	else
	if(exitp) 
	{
	  sprintf(buffer, "%d look", exitp->to_room);
	  do_at(ch, buffer, 0);
	}
	else
	{
	  send_to_char("You see nothing special.\n\r", ch);
	}
      }
      else
	send_to_char("You see nothing special.\n\r",ch);
    }
    break;
      
      /* look 'in'	*/
    case 6: {
      if (*arg2) {
	/* Item carried */
	bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM |
			    FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);
	
	if (bits) { /* Found something */
	  if (GET_ITEM_TYPE(tmp_object)== ITEM_DRINKCON) 	{
	    if (tmp_object->obj_flags.value[1] <= 0) {
	      act("It is empty.", FALSE, ch, 0, 0, TO_CHAR);
	    } else {
	      temp=((tmp_object->obj_flags.value[1]*3)/tmp_object->obj_flags.value[0]);
	      sprintf(buffer,"It's %sfull of a %s liquid.\n\r",
		      fullness[temp],color_liquid[tmp_object->obj_flags.value[2]]);
	      send_to_char(buffer, ch);
	    }
	  } else if (GET_ITEM_TYPE(tmp_object) == ITEM_CONTAINER) {
	    if (!IS_SET(tmp_object->obj_flags.value[1],CONT_CLOSED)) {
	      send_to_char(fname(tmp_object->name), ch);
	      switch (bits) {
	      case FIND_OBJ_INV :
		send_to_char(" (carried) : \n\r", ch);
		break;
	      case FIND_OBJ_ROOM :
		send_to_char(" (here) : \n\r", ch);
		break;
	      case FIND_OBJ_EQUIP :
		send_to_char(" (used) : \n\r", ch);
		break;
	      }
	      list_obj_to_char(tmp_object->contains, ch, 2, TRUE);
	    } else
	      send_to_char("It is closed.\n\r", ch);
	  } else {
	    send_to_char("That is not a container.\n\r", ch);
	  }
	} else { /* wrong argument */
	  send_to_char("You do not see that item here.\n\r", ch);
	}
      } else { /* no argument */
	send_to_char("Look in what?!\n\r", ch);
      }
    }
      break;
      
      /* look 'at'	*/
    case 7 : {
      if (*arg2) {
	bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM |
		    FIND_OBJ_EQUIP | FIND_CHAR_ROOM, ch, &tmp_char, &found_object);
	if (tmp_char) {
	  show_char_to_char(tmp_char, ch, 1);
	  if (ch != tmp_char) {
	    act("$n looks at you.", TRUE, ch, 0, tmp_char, TO_VICT);
	    act("$n looks at $N.", TRUE, ch, 0, tmp_char, TO_NOTVICT);
	  }
	  return;
	}
	/* 
	  Search for Extra Descriptions in room and items 
	  */
	
	/* Extra description in room?? */
	if (!found) {
	  tmp_desc = find_ex_description(arg2, 
		real_roomp(ch->in_room)->ex_description);
	  if (tmp_desc) {
	    page_string(ch->desc, tmp_desc, 0);
	    return; 
	  }
	}
	
	/* extra descriptions in items */
	
	/* Equipment Used */
	if (!found) {
	  for (j = 0; j< MAX_WEAR && !found; j++) {
	    if (ch->equipment[j]) {
	      if (CAN_SEE_OBJ(ch,ch->equipment[j])) {
		tmp_desc = find_ex_description(arg2, 
			       ch->equipment[j]->ex_description);
		if (tmp_desc) {
		  page_string(ch->desc, tmp_desc, 1);
		  found = TRUE;
		}
	      }
	    }
	  }
	}
	/* In inventory */
	if (!found) {
	  for(tmp_object = ch->carrying; 
	      tmp_object && !found; 
	      tmp_object = tmp_object->next_content) {
	    if CAN_SEE_OBJ(ch, tmp_object) {
	      tmp_desc = find_ex_description(arg2, 
					     tmp_object->ex_description);
	      if (tmp_desc) {
		page_string(ch->desc, tmp_desc, 1);
		found = TRUE;
	      }
	    }
	  }
	}
	/* Object In room */
	
	if (!found) {
	  for(tmp_object = real_roomp(ch->in_room)->contents; 
	      tmp_object && !found; 
	      tmp_object = tmp_object->next_content) {
	    if CAN_SEE_OBJ(ch, tmp_object) {
	      tmp_desc = find_ex_description(arg2, 
					     tmp_object->ex_description);
	      if (tmp_desc) {
		page_string(ch->desc, tmp_desc, 1);
		found = TRUE;
	      }
	    }
	  }
	}
	/* wrong argument */
	if (bits) { /* If an object was found */
	  if (!found)
	    show_obj_to_char(found_object, ch, 5); 
	  /* Show no-description */
	  else
	    show_obj_to_char(found_object, ch, 6); 
	  /* Find hum, glow etc */
	} else if (!found) {
	  send_to_char("You do not see that here.\n\r", ch);
	}
      } else {
	/* no argument */	
	send_to_char("Look at what?\n\r", ch);
      }
    }
      break;
      
      /* look ''		*/ 
    case 8 : {
      send_to_char(real_roomp(ch->in_room)->name, ch);
      send_to_char("\n\r", ch);
      if (!IS_SET(ch->specials.act, PLR_BRIEF))
	send_to_char(real_roomp(ch->in_room)->description, ch);
      if (IS_PC(ch)) 
      {
	if (IS_SET(ch->specials.act, PLR_HUNTING))
        {
	  if (ch->specials.hunting) 
          {
	    res = track(ch, ch->specials.hunting);
	    if (!res)
            {
	      ch->specials.hunting = 0;
	      ch->hunt_dist = 0;
	      REMOVE_BIT(ch->specials.act, PLR_HUNTING);
	    }
	  }
          else
          {
	    ch->hunt_dist = 0;
	    REMOVE_BIT(ch->specials.act, PLR_HUNTING);
	  }
	}
      } else {
	if (IS_SET(ch->specials.act, ACT_HUNTING)) {
	  if (ch->specials.hunting) {
	    res = track(ch, ch->specials.hunting);
	    if (!res) {
	      ch->specials.hunting = 0;
	      ch->hunt_dist = 0;
	      REMOVE_BIT(ch->specials.act, ACT_HUNTING);
	    }  
	  } else {
	    ch->hunt_dist = 0;
	    REMOVE_BIT(ch->specials.act, ACT_HUNTING);
	  }
	}
      }
      
      list_obj_in_room(real_roomp(ch->in_room)->contents, ch);
      list_char_in_room(real_roomp(ch->in_room)->people, ch);
      
    }
      break;
      
      /* wrong arg	*/
    case -1 : 
      send_to_char("Sorry, I didn't understand that!\n\r", ch);
      break;
      
      /* look 'room' */
    case 9 : {
      
      send_to_char(real_roomp(ch->in_room)->name, ch);
      send_to_char("\n\r", ch);
      send_to_char(real_roomp(ch->in_room)->description, ch);
      
      if (!IS_NPC(ch)) {
	if (IS_SET(ch->specials.act, PLR_HUNTING)) {
	  if (ch->specials.hunting) {
	    res = track(ch, ch->specials.hunting);
	    if (!res) {
	      ch->specials.hunting = 0;
	      ch->hunt_dist = 0;
	      REMOVE_BIT(ch->specials.act, PLR_HUNTING);
	    }
	  } else {
	    ch->hunt_dist = 0;
	    REMOVE_BIT(ch->specials.act, PLR_HUNTING);
	  }
	}
      } else {
	if (IS_SET(ch->specials.act, ACT_HUNTING)) {
	  if (ch->specials.hunting) {
	    res = track(ch, ch->specials.hunting);
	    if (!res) {
	      ch->specials.hunting = 0;
	      ch->hunt_dist = 0;
	      REMOVE_BIT(ch->specials.act, ACT_HUNTING);
	    }  
	  } else {
	    ch->hunt_dist = 0;
	    REMOVE_BIT(ch->specials.act, ACT_HUNTING);
	  }
	}
      }
      
      list_obj_in_room(real_roomp(ch->in_room)->contents, ch);
      list_char_in_room(real_roomp(ch->in_room)->people, ch);
      
    }
      break;
    }
  }
}

/* end of look */

void do_read(struct char_data *ch, char *argument, int cmd)
{
  char buf[100];
  if(DEBUG) dlog("do_read");  
  /* This is just for now - To be changed later.! */
  sprintf(buf,"at %s",argument);
  do_look(ch,buf,15);
}

void do_examine(struct char_data *ch, char *argument, int cmd)
{
  char name[100], buf[100];
  int bits;
  struct char_data *tmp_char;
  struct obj_data *tmp_object;
 
  if(DEBUG) dlog("do_examine");
  sprintf(buf,"at %s",argument);
  do_look(ch,buf,15);
  
  one_argument(argument, name);
  
  if (!*name)
    {
      send_to_char("Examine what?\n\r", ch);
      return;
    }
  
  bits = generic_find(name, FIND_OBJ_INV | FIND_OBJ_ROOM |
		      FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);
  
  if (tmp_object) {
    if ((GET_ITEM_TYPE(tmp_object)==ITEM_DRINKCON) ||
	(GET_ITEM_TYPE(tmp_object)==ITEM_CONTAINER)) {
      send_to_char("When you look inside, you see:\n\r", ch);
      sprintf(buf,"in %s",argument);
      do_look(ch,buf,15);
    }
  }
}

void do_search(struct char_data *ch, char *argument, int cmd)
{
  int door;
  char buf[256];
  char *exits[] =
    {	
      "North",
      "East ",
      "South",
      "West ",
      "Up   ",
      "Down "
      };
  struct room_direction_data	*exitdata;
  
  *buf = '\0';
  if(DEBUG) dlog("do_search");
  if(IS_DARK(ch->in_room))
  {
    send_to_char("It is far to dark to search...\n\r",ch);
    return;
  }

  if(GET_MANA(ch) < 30 - (GET_LEVEL(ch,BestThiefClass(ch))/5))
  {
    send_to_char("You can not think of a place to begin looking?\n\r",ch);
    return;
  }

  GET_MANA(ch) -= (30 - (GET_LEVEL(ch,BestThiefClass(ch))/5));

  send_to_char("You search the area...\n\r",ch);

  for(door = 0; door <= 5; door++) 
  {
    exitdata = EXIT(ch,door);
    if(exitdata) 
    {
      if(real_roomp(exitdata->to_room)) 
      {
	if(IS_SET(exitdata->exit_info,EX_SECRET))
	{
          if(number(1,101) < ch->skills[SKILL_SEARCH].learned)
	  {
	     send_to_char("You find a hidden passage!\n\r",ch);
	     sprintf(buf + strlen(buf), "%s - %s", exits[door],exitdata->keyword);
	     if (IS_SET(exitdata->exit_info, EX_CLOSED))
	       strcat(buf, " (closed)");
	     strcat(buf, "\n\r");
          }
        }
      }
    }
  }
  send_to_char("Found exits:\n\r", ch);
  
  if (*buf)
    send_to_char(buf, ch);
  else
    send_to_char("None?\n\r", ch);
}

void do_exits(struct char_data *ch, char *argument, int cmd)
{
  int door;
  char buf[256];
  char *exits[] =
    {	
      "North",
      "East ",
      "South",
      "West ",
      "Up   ",
      "Down "
      };
  struct room_direction_data	*exitdata;
  
  *buf = '\0';
  if(DEBUG) dlog("do_exits");  
  for (door = 0; door <= 5; door++) 
  {
    exitdata = EXIT(ch,door);
    if (exitdata) 
    {
      if (!real_roomp(exitdata->to_room)) 
      {
	/* don't print unless immortal */
	if (IS_IMMORTAL(ch)) 
	{
	  sprintf(buf + strlen(buf), "%s - swirling chaos of #%d\n\r",
	    	  exits[door], exitdata->to_room);
        }
      }
      else
      if (exitdata->to_room != NOWHERE &&
	 (!IS_SET(exitdata->exit_info, EX_CLOSED) || IS_IMMORTAL(ch))) 
      {
	if (IS_DARK(exitdata->to_room) && !IS_IMMORTAL(ch))
	  sprintf(buf + strlen(buf), "%s - Too dark to tell", exits[door]);
	else
	  sprintf(buf + strlen(buf), "%s - %s", exits[door],
		  real_roomp(exitdata->to_room)->name);
	if (IS_SET(exitdata->exit_info, EX_CLOSED))
	  strcat(buf, " (closed)");
        if (IS_DARK(exitdata->to_room) && IS_IMMORTAL(ch))
	  strcat(buf, " (dark)");
	strcat(buf, "\n\r");
      }
    }
  }
  
  send_to_char("Obvious exits:\n\r", ch);
  
  if (*buf)
    send_to_char(buf, ch);
  else
    send_to_char("None.\n\r", ch);
}

void do_score(struct char_data *ch, char *argument, int cmd)
{
  struct time_info_data playing_time;
  static char buf[100];
  struct affected_type *aff;
  struct time_info_data real_time_passed(time_t t2, time_t t1);

  sprintf(buf,"Name: %s %s %s\n\r",
     (GET_PRETITLE(ch)?GET_PRETITLE(ch):""), GET_NAME(ch), GET_TITLE(ch));
  send_to_char(buf,ch);

  sprintf(buf, "Age: %d years.", GET_AGE(ch));
  
  if ((age(ch).month == 0) && (age(ch).day == 0))
    strcat(buf," You are a newbie! Welcome!.\n\r");
  else
    strcat(buf,"\n\r");
  send_to_char(buf, ch);
  
  if (!IS_IMMORTAL(ch) && (!IS_NPC(ch))) {
    if (GET_COND(ch,DRUNK)>10)
      send_to_char("You are intoxicated.\n\r", ch);
    if (GET_COND(ch,FULL)<2)
      send_to_char("You are hungry.\n\r", ch);
    if (GET_COND(ch,THIRST)<2)
      send_to_char("You are thirsty.\n\r", ch);
  }
 
  if(IS_SET(ch->specials.new_act,NEW_PLR_NOTELL))
    send_to_char("You are NOT recieving tells.\n\r",ch);

  if(IS_SET(ch->specials.act,PLR_DEAF))
    send_to_char("You are NOT listening to shouts.\n\r",ch);

  sprintf(buf, 
  "Hit Points: %d(%d)    Mana: %d(%d)    Movement: %d(%d).\n\r",
	  GET_HIT(ch),GET_MAX_HIT(ch),
	  GET_MANA(ch),GET_MAX_MANA(ch),
	  GET_MOVE(ch),GET_MAX_MOVE(ch));
  send_to_char(buf,ch);

  if(MOUNTED(ch))
  {
    if((GET_LEVEL(ch,RANGER_LEVEL_IND) > 0) || IS_AFFECTED(MOUNTED(ch),AFF_CHARM))
    {
      sprintf(buf, 
      "Mnt Hit Points: %d(%d)  Mnt Movement: %d(%d).\n\r",
	  GET_HIT(MOUNTED(ch)),GET_MAX_HIT(MOUNTED(ch)),
	  GET_MOVE(MOUNTED(ch)),GET_MAX_MOVE(MOUNTED(ch)));
      send_to_char(buf,ch);
    }
  }
 
  if(GetMaxLevel(ch) > 3)
  {
    sprintf(buf,"Abilities:\n\r");
    send_to_char(buf,ch);
    sprintf(buf,"Str: %d(%d)\tDex: %d\tCon: %d\n\r",
      GET_STR(ch),GET_ADD(ch),GET_DEX(ch),GET_CON(ch));
    send_to_char(buf,ch);
    sprintf(buf,"Int: %d     \tWis: %d\n\r",
      GET_INT(ch),GET_WIS(ch));
    send_to_char(buf,ch);
    sprintf(buf,"Hit: %d     \tDam: %d\tAC: %d\n\r",
      str_app[STRENGTH_APPLY_INDEX(ch)].tohit,
      str_app[STRENGTH_APPLY_INDEX(ch)].todam,
      (ch->points.armor/10));
    send_to_char(buf,ch);
  }

  sprintf(buf, "Alignment: (-1000 : +1000): %d\n\r", GET_ALIGNMENT(ch));
  send_to_char(buf,ch);
  
  sprintf(buf,"Exp: %d   Gold: %d    Bank: %d.\n\r\n\r",GET_EXP(ch),GET_GOLD(ch),GET_BANK(ch));
  send_to_char(buf,ch);
  sprintf(buf,"Home Location:%s\n\r",real_roomp(GET_HOME(ch))->name);
  send_to_char(buf,ch);

  sprintf(buf,"Class:        Level:        Exp for a Level:\n\r");
  send_to_char(buf,ch);
  sprintf(buf,"-----         -----         ---------------\n\r");
  send_to_char(buf,ch);

  if(GET_LEVEL(ch,MAGE_LEVEL_IND) > 0)
  {
    sprintf(buf,"Mage           %2d               %8d\n\r",
      GET_LEVEL(ch,MAGE_LEVEL_IND),
      titles[MAGE_LEVEL_IND][GET_LEVEL(ch,MAGE_LEVEL_IND)+1].exp - GET_EXP(ch));
    send_to_char(buf,ch); 
  }

  if(GET_LEVEL(ch,CLERIC_LEVEL_IND)>0)
  {
    sprintf(buf,"Cleric         %2d               %8d\n\r",
      GET_LEVEL(ch,CLERIC_LEVEL_IND),
      titles[CLERIC_LEVEL_IND][GET_LEVEL(ch,CLERIC_LEVEL_IND)+1].exp - GET_EXP(ch));
    send_to_char(buf,ch); 
  }

  if(GET_LEVEL(ch,WARRIOR_LEVEL_IND)>0)
  {
    sprintf(buf,"Warrior        %2d               %8d\n\r",
      GET_LEVEL(ch,WARRIOR_LEVEL_IND),
      titles[WARRIOR_LEVEL_IND][GET_LEVEL(ch,WARRIOR_LEVEL_IND)+1].exp - GET_EXP(ch));
    send_to_char(buf,ch); 
  }

  if(GET_LEVEL(ch,RANGER_LEVEL_IND)>0)
  {
    sprintf(buf,"Ranger         %2d               %8d\n\r",
      GET_LEVEL(ch,RANGER_LEVEL_IND),
      titles[RANGER_LEVEL_IND][GET_LEVEL(ch,RANGER_LEVEL_IND)+1].exp - GET_EXP(ch));
    send_to_char(buf,ch); 
  }

  if(GET_LEVEL(ch,THIEF_LEVEL_IND)>0)
  {
    sprintf(buf,"Thief          %2d               %8d\n\r",
      GET_LEVEL(ch,THIEF_LEVEL_IND),
      titles[THIEF_LEVEL_IND][GET_LEVEL(ch,THIEF_LEVEL_IND)+1].exp - GET_EXP(ch));
    send_to_char(buf,ch); 
  }

  playing_time = real_time_passed((time(0)-ch->player.time.logon) + ch->player.time.played, 0);
  sprintf(buf,"\n\rPlaying time: %d days and %d hours.\n\r",playing_time.day,playing_time.hours);
  send_to_char(buf, ch);		

  sprintf(buf,"Position:");
  send_to_char(buf,ch);

  switch(GET_POS(ch)) {
  case POSITION_DEAD : 
    send_to_char("DEAD!\n\r", ch); break;
  case POSITION_MORTALLYW :
    send_to_char("Mortally Wounded, you should seek help\n\r", ch); break;
  case POSITION_INCAP : 
    send_to_char("Incapacitated, slowly fading away\n\r", ch); break;
  case POSITION_STUNNED : 
    send_to_char("Stunned, you cant move\n\r", ch); break;
  case POSITION_SLEEPING : 
    send_to_char("Sleeping.\n\r",ch); break;
  case POSITION_RESTING  : 
    send_to_char("Resting.\n\r",ch); break;
  case POSITION_SITTING  : 
    send_to_char("Sitting.\n\r",ch); break;
  case POSITION_FIGHTING :
    if (ch->specials.fighting)
      act("Fighting $N.\n\r", FALSE, ch, 0,ch->specials.fighting, TO_CHAR);
    else
      send_to_char("Fighting thin air.\n\r", ch);
    break;
  case POSITION_STANDING : 
    send_to_char("Standing.\n\r",ch); break;
  case POSITION_MOUNTED  :
    if (MOUNTED(ch))
    {
      send_to_char("You are riding on ",ch);
      send_to_char(MOUNTED(ch)->player.short_descr, ch);
      send_to_char("\n\r", ch);
    }
    else
    {
      send_to_char("You are standing.\n\r",ch); break;
    }
    break;
  default :
    send_to_char("Floating.\n\r",ch); break;
  }

  if(ch->affected ) 
  {
    send_to_char("\n\rSpells in Affect:\n\r--------------\n\r", ch);
    for(aff = ch->affected; aff; aff = aff->next) 
    {
      switch(aff->type) 
      {
      case SKILL_SNEAK:
	break;
      default:
	sprintf(buf, "Spell : '%s'\n\r",spells[aff->type-1]);
	send_to_char(buf, ch);
	break;
      }
    }
  }
}

void do_mystat(struct char_data *ch, char *argument, int cmd)
{
  char buf[256];
  if(DEBUG) dlog("do_mystat");
  sprintf(buf,"Use SCORE instead.\n\r");
  send_to_char(buf,ch);
}

void do_time(struct char_data *ch, char *argument, int cmd)
{
  struct tm *tm_info;
  time_t tc;
  char tbuf[100];
  char buf[100], *suf;
  int weekday, day;
  extern struct time_info_data time_info;
  extern const char *weekdays[];
  extern const char *month_name[];
  tc = time(0);
  tm_info = localtime(&tc);
 
  sprintf(buf, "It is %d o'clock %s, on ",
	  ((time_info.hours % 12 == 0) ? 12 : ((time_info.hours) % 12)),
	  ((time_info.hours >= 12) ? "pm" : "am") );
  
  weekday = ((35*time_info.month)+time_info.day+1) % 7;/* 35 days in a month */
  
  strcat(buf,weekdays[weekday]);
  strcat(buf,"\n\r");
  send_to_char(buf,ch);
  
  day = time_info.day + 1;   /* day in [1..35] */
  
  if (day == 1)
    suf = "st";
  else if (day == 2)
    suf = "nd";
  else if (day == 3)
    suf = "rd";
  else if (day < 20)
    suf = "th";
  else if ((day % 10) == 1)
    suf = "st";
  else if ((day % 10) == 2)
    suf = "nd";
  else if ((day % 10) == 3)
    suf = "rd";
  else
    suf = "th";
  
  sprintf(buf, "The %d%s Day of the %s, Year %d.\n\r",
	  day,
	  suf,
	  month_name[time_info.month],
	  time_info.year);
  
  send_to_char(buf,ch);
  if(tm_info->tm_hour > 12)
    sprintf(tbuf,"In RL time, it is %2d:%2.2d\n\r",tm_info->tm_hour-12,
                                                  tm_info->tm_min);
  else
    sprintf(tbuf,"In RL time, it is %2d:%2.2d\n\r",tm_info->tm_hour,
                                                  tm_info->tm_min);
  send_to_char(tbuf,ch);
}


void do_weather(struct char_data *ch, char *argument, int cmd)
{
  extern struct weather_data weather_info;
  static char buf[100];
  char static *sky_look[4]= 
  {
    "cloudless", "cloudy", "rainy", "lit by flashes of lightning"
  };
  char static * sky_words[]=
  {
    "Cloudless","Cloudy","Rainy","Lightning","None"
  };
  char static * wind_dir[] =
  {
    "North","East","South","West","Still","None" 
  };

  if(IS_IMMORTAL(ch))
  {
    sprintf(buf,"Sky Condit : %s\n\r",sky_words[weather_info.sky]);
    send_to_char(buf,ch);
    sprintf(buf,"Wind Speed : %d\n\r",weather_info.wind_speed);
    send_to_char(buf,ch);
    sprintf(buf,"Wind Direc : %s\n\r",wind_dir[weather_info.wind_direction]);
    send_to_char(buf,ch);
    sprintf(buf,"Change     : %d\n\r",weather_info.change);
    send_to_char(buf,ch); 
    sprintf(buf,"Pressure   : %d\n\r",weather_info.pressure);
    send_to_char(buf,ch);
    sprintf(buf,"Moon Phase : %d\n\r\n\r",weather_info.moon);
    send_to_char(buf,ch);
  }
    
  if(OUTSIDE(ch)) 
  {
    sprintf(buf,"The sky is %s and %s.\n\r",sky_look[weather_info.sky],
	    (weather_info.change >=0 ? "you feel a warm wind from south" :
	     "your foot tells you bad weather is due"));
    send_to_char(buf,ch);
  }
  else
    send_to_char("You have no feeling about the weather at all.\n\r", ch);
}

void do_help(struct char_data *ch, char *argument, int cmd)
{
  extern int top_of_helpt;
  extern struct help_index_element *help_index;
  extern FILE *help_fl;
  extern char help[MAX_STRING_LENGTH];
    
  int chk, bot, top, mid, minlen;
  char buf[MAX_STRING_LENGTH], buffer[MAX_STRING_LENGTH];
  
  if(DEBUG) dlog("do_help");
  if (!ch->desc)
    return;
  
  for(;isspace(*argument); argument++)  ;
  
  if (*argument)
    {
      if (!help_index)
	{
	  send_to_char("No help available.\n\r", ch);
	  return;
	}
      bot = 0;
      top = top_of_helpt;
      
      for (;;)
	{
	  mid = (bot + top) / 2;
	  minlen = strlen(argument);
	  
	  if (!(chk = strn_cmp(argument, help_index[mid].keyword, minlen)))
	    {
	      fseek(help_fl, help_index[mid].pos, 0);
	      *buffer = '\0';
	      for (;;)
		{
		  fgets(buf, 80, help_fl);
		  if (*buf == '#')
		    break;
		  strcat(buffer, buf);
		  strcat(buffer, "\r");
		}
	      page_string(ch->desc, buffer, 1);
	      return;
	    }
	  else if (bot >= top)
	    {
	      send_to_char("There is no help on that word.\n\r", ch);
	      return;
	    }
	  else if (chk > 0)
	    bot = ++mid;
	  else
	    top = --mid;
	}
      return;
    }
    page_string(ch->desc,help,0);
  /* send_to_char(help, ch); */
}

do_wizhelp(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  int no, i;
  extern char *command[];	 /* The list of commands (interpreter.c)  */
  /* First command is command[0]           */
  extern struct command_info cmd_info[];
  /* cmd_info[1] ~~ commando[0]            */
  if(DEBUG) dlog("do_wizhelp"); 
  if (IS_NPC(ch))
    return;
  
  send_to_char("The following privileged comands are available:\n\r\n\r", ch);
  
  *buf = '\0';
  
  for (no = 1, i = 0; *command[i] != '\n'; i++)
    if ((GetMaxLevel(ch) >= cmd_info[i+1].minimum_level) &&
	(cmd_info[i+1].minimum_level >= LOW_IMMORTAL)) 	{
      
      sprintf(buf + strlen(buf), "%-10s", command[i]);
      if (!(no % 7))
	strcat(buf, "\n\r");
      no++;
    }
  strcat(buf, "\n\r");
  page_string(ch->desc, buf, 1);
}

do_allcommands(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  int no, i;
  extern char *command[];	 /* The list of commands (interpreter.c)  */
  extern struct command_info cmd_info[];
  if(DEBUG) dlog("do_allcommands"); 
  if (IS_NPC(ch))
    return;
  send_to_char("The following comands are available:\n\r\n\r", ch);
  *buf = '\0';
  for (no = 1, i = 0; *command[i] != '\n'; i++)
    if ((GetMaxLevel(ch) >= cmd_info[i+1].minimum_level) &&
	(cmd_info[i+1].minimum_level < LOW_IMMORTAL))
    {
      sprintf(buf + strlen(buf), "%-10s", command[i]);
      if (!(no % 7))
	strcat(buf, "\n\r");
      no++;
    }
  strcat(buf, "\n\r");
  page_string(ch->desc, buf, 1);
}

void do_who(struct char_data *ch, char *argument, int cmd)
{
  struct descriptor_data *d;
  char buf[256];
  int count;
  struct char_data	*person;
  long ttime;
  long tmin, tsec;
  if(DEBUG) dlog("do_who");
  if(IS_NPC(ch)) return;

  send_to_char("\n\rIdle         Players\n\r", ch);
  send_to_char("--------     -------\n\r",ch);

  count=0;
  for(d = descriptor_list;d;d = d->next) 
  {
    person = d->character;

  if(person!=NULL && person->desc!=0)
  if(IS_PC(person))
  {
    if(!d->connected && CAN_SEE(ch, d->character) &&
       (real_roomp(person->in_room)->zone == real_roomp(ch->in_room)->zone || cmd!=234 )) 
    {
      count++;
      if (cmd==234) 
      { /* it's a whozone command */
        if ((!IS_AFFECTED(person, AFF_HIDE)) ||
	   ((IS_IMMORTAL(ch) && (GetMaxLevel(ch)>person->invis_level)))) 
        {
	  ttime = GET_IDLE_TIME(person);
	  tmin = ttime/60;
	  tsec = ttime - (tmin*60);
	  if (tsec < 10 && tmin == 0) tsec = 0;

          sprintf(buf,"[%2dm%2ds] %s %s %s  -  %s",tmin,tsec,
			( person->player.pre_title?person->player.pre_title : ""),
			GET_NAME(person),person->player.title,
			real_roomp(person->in_room)->name);

	  if (GetMaxLevel(ch) >= LOW_IMMORTAL)
	    sprintf(buf+strlen(buf),"[%d]", person->in_room);
	}
      }
      else
      {
	ttime = GET_IDLE_TIME(person);
	tmin = ttime/60;
	tsec = ttime - (tmin*60);
	if (tsec < 10 && tmin == 0) tsec = 0;

        sprintf(buf, "[%2dm%2ds] %s %s %s  -  %s",tmin,tsec,
			( person->player.pre_title?person->player.pre_title : ""),
			GET_NAME(person),
			( person->player.title?person->player.title : ""),
			real_roomp(person->in_room)->name);

	if (GetMaxLevel(ch) >= LOW_IMMORTAL)
	  sprintf(buf+strlen(buf),"[%d]", person->in_room);
      }
      strcat(buf, "\n\r");
      send_to_char(buf, ch);
    }
  }
  }
  sprintf(buf, "\n\rTotal players on Wiley: %d\n\r", count);
  send_to_char(buf, ch);
}

void do_users(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH], line[200];
  struct descriptor_data *d;
  int flag;
  if(DEBUG) dlog("do_users");
  strcpy(buf, "Connections:\n\r------------\n\r");

  for (d = descriptor_list; d; d = d->next)
  {
    flag = 0;
    if (d->character && d->character->player.name)
    {
      if(GetMaxLevel(ch) > d->character->invis_level)
	flag = 1;

      if(flag)
      {
        if(d->original)
          sprintf(line, "%-16s: ", d->original->player.name);
        else
          sprintf(line, "%-16s: ", d->character->player.name);
      }
    }
    else
    {
      strcpy(line,connected_types[d->connected]);
      strcat(line,"\n\r");
    }

    if(flag)
    {
      if (d->host)
        sprintf(line + strlen(line), "[%s]\n\r", d->host);
      else
        strcat(line, "[Hostname unknown]\n\r");
    }
    
    if(flag)
      strcat(buf, line);
  }
  strcat(buf,"\n\r");
  send_to_char(buf, ch);
}

void do_inventory(struct char_data *ch, char *argument, int cmd) 
{
  if(DEBUG) dlog("do_inventory");
  send_to_char("You are carrying:\n\r", ch);
  list_obj_on_char(ch->carrying, ch);
}

void do_equipment(struct char_data *ch, char *argument, int cmd) {
  int j,Worn_Index;
  bool found;
  char String[256];
  if(DEBUG) dlog("do_equipment"); 
  send_to_char("Equipment in use:\n\r", ch);
  found = FALSE;
  for(Worn_Index=j=0; j< MAX_WEAR; j++) 
  {
    if (ch->equipment[j])
    {
      Worn_Index++;
      sprintf(String,"%s",where[j]);
      send_to_char(String,ch);
      if (CAN_SEE_OBJ(ch,ch->equipment[j])) 
      {
	show_obj_to_char(ch->equipment[j],ch,1);
	found = TRUE;
      }
      else
      {
	send_to_char("Something.\n\r",ch);
	found = TRUE;
      }
    }
  }
  if(!found) {
    send_to_char(" Nothing.\n\r", ch);
  }
}

void do_credits(struct char_data *ch, char *argument, int cmd) 
{
  if(DEBUG) dlog("do_credits");
  page_string(ch->desc, credits, 0);
}

void do_news(struct char_data *ch, char *argument, int cmd) {
  if(DEBUG) dlog("do_news");  
  page_string(ch->desc, news, 0);
}


void do_info(struct char_data *ch, char *argument, int cmd) {
  if(DEBUG) dlog("do_info"); 
  page_string(ch->desc, info, 0);
}


void do_wizlist(struct char_data *ch, char *argument, int cmd) {
  if(DEBUG) dlog("do_wizlist"); 
  page_string(ch->desc, wizlist, 0);
}

static int which_number_mobile(struct char_data *ch, struct char_data *mob)
{
  struct char_data	*i;
  char	*name;
  int	number;
  if(DEBUG) dlog("which_number_mobile"); 
  name = fname(mob->player.name);
  for (i=character_list, number=0; i; i=i->next) {
    if (isname(name, i->player.name) && i->in_room != NOWHERE) {
      number++;
      if (i==mob)
	return number;
    }
  }
  return 0;
}

char *numbered_person(struct char_data *ch, struct char_data *person)
{
  static char buf[MAX_STRING_LENGTH];
  if(DEBUG) dlog("numbered_person");
  if (IS_NPC(person) && IS_IMMORTAL(ch)) {
    sprintf(buf, "%d.%s", which_number_mobile(ch, person),
	    fname(person->player.name));
  } else {
    strcpy(buf, PERS(person, ch));
  }
  return buf;
}

static void do_where_person(struct char_data *ch, struct char_data *person, struct string_block *sb)
{
  char buf[MAX_STRING_LENGTH];
  
  if(DEBUG) dlog("do_where_person"); 
  sprintf(buf, "%-30s- %s ", PERS(person, ch),
	  (person->in_room > -1 ? real_roomp(person->in_room)->name : "Nowhere"));
  
  if (GetMaxLevel(ch) >= LOW_IMMORTAL)
    sprintf(buf+strlen(buf),"[%d]", person->in_room);
  
  strcpy(buf+strlen(buf), "\n\r");
  
  append_to_string_block(sb, buf);
}

static void do_where_object(struct char_data *ch, struct obj_data *obj,
			    int recurse, struct string_block *sb)
{
  char buf[MAX_STRING_LENGTH];
  if(DEBUG) dlog("do_where_object");

  if (obj->in_room != NOWHERE) { /* object in a room */
    sprintf(buf, "%-30s- %s [%d]\n\r",
	    obj->short_description,
	    real_roomp(obj->in_room)->name,
	    obj->in_room);
  } else if (obj->carried_by != NULL) { /* object carried by monster */
    sprintf(buf, "%-30s- carried by %s\n\r",
	    obj->short_description,
	    numbered_person(ch, obj->carried_by));
  } else if (obj->equipped_by != NULL) { /* object equipped by monster */
    sprintf(buf, "%-30s- equipped by %s\n\r",
	    obj->short_description,
	    numbered_person(ch, obj->equipped_by));
  } else if (obj->in_obj) { /* object in object */
    sprintf(buf, "%-30s- in %s\n\r",
	    obj->short_description,
	    obj->in_obj->short_description);
  } else {
    sprintf(buf, "%-30s- can't find it? \n\r",
	    obj->short_description);
  }
  if (*buf)
    append_to_string_block(sb, buf);
  
  if (recurse) {
    if (obj->in_room != NOWHERE)
      return;
    else if (obj->carried_by != NULL)
      do_where_person(ch, obj->carried_by, sb);
    else if (obj->equipped_by != NULL)
      do_where_person(ch, obj->equipped_by, sb);
    else if (obj->in_obj != NULL)
      do_where_object(ch, obj->in_obj, TRUE, sb);
  }
}

void do_where(struct char_data *ch, char *argument, int cmd)
{
  char name[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  char	*nameonly;
  register struct char_data *i;
  register struct obj_data *k;
  struct descriptor_data *d;
  int	number, count;
  struct string_block sb;
  if(DEBUG) dlog("do_where"); 
  only_argument(argument, name);
  
  if (!*name) 
  {
    if (GetMaxLevel(ch) < LOW_IMMORTAL)      	
    {
      send_to_char("What are you looking for?\n\r", ch);
      return;
    }
    else
    {
      init_string_block(&sb);
      append_to_string_block(&sb, "Players:\n\r--------\n\r");
      
      for (d = descriptor_list; d; d = d->next) 
      {
	if (d->character &&
	   (d->connected == CON_PLYNG) &&
	   (d->character->in_room != NOWHERE) &&
	   (GetMaxLevel(ch)>d->character->invis_level))
	{
	  if (d->original)   /* If switched */
	    sprintf(buf, "%-20s - %s [%d] In body of %s\n\r",
		    d->original->player.name,
		    real_roomp(d->character->in_room)->name,
		    d->character->in_room,
		    fname(d->character->player.name));
	  else
	    sprintf(buf, "%-20s - %s [%d]\n\r",
		    d->character->player.name,
		    real_roomp(d->character->in_room)->name,
		    d->character->in_room);
	  
	  append_to_string_block(&sb, buf);
	}
      }
      page_string_block(&sb,ch);
      destroy_string_block(&sb);
      return;
    }
  }
  
  if (isdigit(*name)) {
    nameonly = name;
    count = number = get_number(&nameonly);
  } else {
    count = number = 0;
  }
  
  *buf = '\0';
  
  init_string_block(&sb);
  
  for (i = character_list; i; i = i->next)
    if (isname(name, i->player.name) && CAN_SEE(ch, i) )   	
    {
      if ((i->in_room != NOWHERE) &&
	  ((GetMaxLevel(ch)>=LOW_IMMORTAL) || (real_roomp(i->in_room)->zone ==
					     real_roomp(ch->in_room)->zone))) {
	if (number==0 || (--count) == 0) {
	  if (number==0) {
	    sprintf(buf, "[%2d] ", ++count); /* I love short circuiting :) */
	    append_to_string_block(&sb, buf);
	  }
	  do_where_person(ch, i, &sb);
	  *buf = 1;
	  if (number!=0)
	    break;
	}
       	if (GetMaxLevel(ch) < LOW_IMMORTAL)
	  break;
      }
    }
  
  /*  count = number;*/
  
  if (GetMaxLevel(ch) >= LOW_IMMORTAL ) {
    for (k = object_list; k; k = k->next)
      if (isname(name, k->name) && CAN_SEE_OBJ(ch, k)) {
	if (number==0 || (--count)==0) {
	  if (number==0) {
	    sprintf(buf, "[%2d] ", ++count);
	    append_to_string_block(&sb, buf);
	  }
	  do_where_object(ch, k, number!=0, &sb);
	  *buf = 1;
	  if (number!=0)
	    break;
	}
      }
  }
  
  if (!*sb.data)
    send_to_char("Couldn't find any such thing.\n\r", ch);
  else
    page_string_block(&sb, ch);
  destroy_string_block(&sb);
}

void do_levels(struct char_data *ch, char *argument, int cmd)
{
  int i, RaceMax, class;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  
  
  if(DEBUG) dlog("do_levels");
  if (IS_NPC(ch)){
    return;
  }
  
  *buf = '\0';
/*
**  get the class
*/

  for (;isspace(*argument);argument++);

  if (!*argument) {
    send_to_char("You must supply a class!\n\r", ch);
    return;
  }

  switch(*argument) {
  case 'C':
  case 'c':
  case 'P':
  case 'p':
    class = CLERIC_LEVEL_IND;
    break;
  case 'F':
  case 'f':
  case 'W':
  case 'w':
    class = WARRIOR_LEVEL_IND;
    break;
  case 'M':
  case 'm':
    class = MAGE_LEVEL_IND;
    break;
  case 'T':
  case 't':
    class = THIEF_LEVEL_IND;
    break;
  case 'R':
  case 'r':
    class = RANGER_LEVEL_IND;
    break;
  case 'D':
  case 'd':
    class = DRUID_LEVEL_IND;
    break;
  default:
    sprintf(buf, "I don't recognize %s\n\r", argument);
    send_to_char(buf,ch);
    return;
    break;
  }

  RaceMax = RacialMax[RACE_HUMAN][class];
  
  for (i = 1; i <= RaceMax; i++) 
  {
    sprintf(buf+strlen(buf), "[%2d] %9d-%-9d : ", i,
	    titles[class][i].exp,
	    titles[class][i + 1].exp);
    
    switch(GET_SEX(ch))    	
    {
      case SEX_MALE:
        strcat(buf, titles[class][i].title_m); break;
      case SEX_FEMALE:
        strcat(buf, titles[class][i].title_f); break;
      default:
        send_to_char("Uh oh.\n\r", ch); break;
    }
    strcat(buf, "\n\r");
  }
  page_string(ch->desc,buf,1);
}

void do_consider(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char name[256];
  int diff;
  if(DEBUG) dlog("do_consider");  
  only_argument(argument, name);
  
  if (!(victim = get_char_room_vis(ch, name))) {
    send_to_char("Consider killing who?\n\r", ch);
    return;
  }
  
  if (victim == ch) {
    send_to_char("The perfect match!\n\r", ch);
    return;
  }
  
  if (!IS_NPC(victim)) {
    send_to_char("Hmmmm, I think you might get in trouble for that.\n\r", ch);
    return;
  }
  diff =  (GetMaxLevel(victim)+(GetSecMaxLev(victim)/2)+
	  (GetThirdMaxLev(victim)/3))-
          (GetMaxLevel(ch)+(GetSecMaxLev(ch)/2)+(GetThirdMaxLev(ch)/3));
  if (diff <= -10)
    send_to_char("Too easy to be believed.\n\r", ch);
  else if (diff <= -5)
    send_to_char("Not a problem.\n\r", ch);
  else if (diff <= -3)
    send_to_char("Rather easy.\n\r",ch);
  else if (diff <= -2)
    send_to_char("Easy.\n\r", ch);
  else if (diff <= -1)
    send_to_char("Fairly easy.\n\r", ch);
  else if (diff == 0)
    send_to_char("The perfect match!\n\r", ch);
  else if (diff <= 1)
    send_to_char("You would need some luck!\n\r", ch);
  else if (diff <= 2)
    send_to_char("You would need a lot of luck!\n\r", ch);
  else if (diff <= 3)
    send_to_char("You would need a lot of luck and great equipment!\n\r", ch);
  else if (diff <= 5)
    send_to_char("Do you feel lucky, punk?\n\r", ch);
  else if (diff <= 10)
    send_to_char("Are you crazy?\n\r", ch);
  else if (diff <= 100)
    send_to_char("You ARE mad!\n\r", ch);
}

void do_spells(struct char_data *ch, char *argument, int cmd)
{
  int spl, i;
  char buf[16384];
  extern int spell_status[];
  extern struct spell_info_type spell_info[MAX_SKILLS];
  if(DEBUG) dlog("do_spells");  
  if (IS_NPC(ch)) return;
  *buf=0;
  for(i=1;i<= MAX_EXIST_SPELL;i++)
  {
    sprintf(buf + strlen(buf), "[%2d] %-20s  Mana: %3d, Cl: %2d, Mu: %2d\n\r",
      i, spells[i-1], 
      spell_info[i].min_usesmana, 
      spell_info[i].min_level_cleric, 
      spell_info[i].min_level_magic);
  }
  strcat(buf, "\n\r");
  page_string(ch->desc, buf, 1);
}

void do_world(struct char_data *ch, char *argument, int cmd)
{
  static char buf[100];
  long ct, ot;
  char *tmstr, *otmstr;
  extern long Uptime;

  if(DEBUG) dlog("do_world");
  ot = Uptime;
  otmstr = asctime(localtime(&ot));
  *(otmstr + strlen(otmstr) - 1) = '\0';
  sprintf(buf, "Wiley start time was: %s\n\r", otmstr);
  send_to_char(buf, ch);
  
  ct = time(0);
  tmstr = asctime(localtime(&ct));
  *(tmstr + strlen(tmstr) - 1) = '\0';
  sprintf(buf, "Wiley's time is: %s\n\r",tmstr);
  send_to_char(buf, ch);
  sprintf(buf,"Total number of players: %d\n\r",top_of_p_table + 1);
  send_to_char(buf, ch);
  sprintf(buf,"Total number of mobiles: %d\n\r",top_of_mobt + 1);
  send_to_char(buf, ch);
  sprintf(buf,"Total number of rooms: %d\n\r",room_db.klistlen);
  send_to_char(buf, ch);
  sprintf(buf,"Total number of objects: %d\n\r",top_of_objt + 1);
  send_to_char(buf, ch);
}


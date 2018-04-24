/* ************************************************************************
*  file: utility.c, Utility module.                       Part of DIKUMUD *
*  Usage: Utility procedures                                              *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>

#include "structs.h"
#include <time.h>
#include "utils.h"
#include "spells.h"
#include "race.h"
#include "db.h"
#include "opinion.h"
#include "comm.h"
#include "hash.h"

extern struct time_data time_info;
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;
extern struct index_data *mob_index, *obj_index;
extern struct hash_header room_db;	                  /* In db.c */
extern char *dirs[]; 



/* external functions */
void stop_fighting(struct char_data *ch);
void fake_setup_dir(FILE *fl, int room, int dir);
void setup_dir(FILE *fl, int room, int dir);
char *fread_string(FILE *fl);

int MIN(int a, int b)
{
	return a < b ? a:b;
}

int MAX(int a, int b)
{
	return a > b ? a:b;
}

int GetItemClassRestrictions(struct obj_data *obj)
{
  int total=0;

  if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_MAGE)) 
    total += CLASS_MAGIC_USER;
  if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_THIEF)) 
    total += CLASS_THIEF;
  if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_RANGER)) 
    total += CLASS_RANGER;
  if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_FIGHTER)) 
    total += CLASS_WARRIOR;
  if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_CLERIC)) 
    total += CLASS_CLERIC;

  return(total);

}

int CAN_SEE(struct char_data *s, struct char_data *o)
{
  if (!o)
    return(FALSE);

  if ((s->in_room == -1) || (o->in_room == -1)) 
    return(FALSE);

  if (o->invis_level >= GetMaxLevel(s))
    return FALSE;

  if (IS_IMMORTAL(s)) 
    return(TRUE);

  if(IS_AFFECTED(s, AFF_TRUE_SIGHT))
    return(TRUE);

  if (IS_AFFECTED(s, AFF_BLIND)&& s!=o)
    return(FALSE);

  if (IS_AFFECTED(o, AFF_HIDE))
    return(FALSE);

  if(IS_AFFECTED(o, AFF_INVISIBLE))
  {
    if(IS_IMMORTAL(o))
      return(FALSE);
    if(!IS_AFFECTED(s, AFF_DETECT_INVISIBLE))
      return(FALSE);
  }

  if ((IS_DARK(s->in_room) || IS_DARK(o->in_room)) &&
        (!IS_AFFECTED(s, AFF_INFRAVISION)))
        return(FALSE);

  return(TRUE);

#if 0
  ((IS_IMMORTAL(sub)) || /* gods can see anything */ \
   (((!IS_AFFECTED((obj),AFF_INVISIBLE)) || /* visible object */ \
     ((IS_AFFECTED((sub),AFF_DETECT_INVISIBLE)) && /* you detect I and */ \
      (!IS_IMMORTAL(obj)))) &&			/* object is not a god */ \
    (!IS_AFFECTED((sub),AFF_BLIND)) &&      /* you are not blind */ \
    ( (IS_LIGHT(sub->in_room)) || (IS_AFFECTED((sub),AFF_INFRAVISION))) \
		/* there is enough light to see or you have infravision */ \
    ))
#endif
}

int exit_ok(struct room_direction_data	*exit, struct room_data **rpp)
{
  struct room_data	*rp;
  if (rpp==NULL)
    rpp = &rp;
  if (!exit) {
    *rpp = NULL;
    return FALSE;
  }
  *rpp = real_roomp(exit->to_room);
  return (*rpp!=NULL);
}

int MobVnum( struct char_data *c)
{
  if (IS_NPC(c)) {
    return(mob_index[c->nr].virtual);
  } else {
    return(0);
  }
}

int ObjVnum( struct obj_data *o)
{
  if (o->item_number >= 0)
     return(obj_index[o->item_number].virtual);
  else
    return(-1);
}


void Zwrite (FILE *fp, char cmd, int tf, int arg1, int arg2, int arg3, 
	     char *desc)
{
   char buf[100];

   if (*desc) {
     sprintf(buf, "%c %d %d %d %d   ; %s\n", cmd, tf, arg1, arg2, arg3, desc);
     fputs(buf, fp);
   } else {
     sprintf(buf, "%c %d %d %d %d\n", cmd, tf, arg1, arg2, arg3); 
     fputs(buf, fp);
   }
}

void RecZwriteObj(FILE *fp, struct obj_data *o)
{
   struct obj_data *t;

   if (ITEM_TYPE(o) = ITEM_CONTAINER) {
     for (t = o->contains; t; t=t->next_content) {
       Zwrite(fp, 'P', 1, ObjVnum(t), obj_index[t->item_number].number, ObjVnum(o), 
	      t->short_description);
       RecZwriteObj(fp, t);
     }
   } else {
     return;
   }
}

FILE *MakeZoneFile( struct char_data *c)
{
  char buf[256];
  FILE *fp;

  sprintf(buf, "%s/%s.zon",MKZONE_PATH,GET_NAME(c));

  if ((fp = fopen(buf, "w")) != NULL)
    return(fp);
  else
    return(0);

}


int IsImmune(struct char_data *ch, int bit)
{
  if(GetMaxLevel(ch) >= 50) return(1);
  return(IS_SET(bit, ch->M_immune));
}

int IsResist(struct char_data *ch, int bit)
{
  if(GetMaxLevel(ch) >= 50) return(1);
  return(IS_SET(bit, ch->immune));
}

int IsSusc(struct char_data *ch, int bit)
{
  return(IS_SET(bit, ch->susc));
}

/* creates a random number in interval [from;to] */
int number(int from, int to) 
{
   if (to - from + 1 )
	return((random() % (to - from + 1)) + from);
   else
       return(from);
}

/* simulates dice roll */
int dice(int number, int size) 
{
  int r;
  int sum = 0;

  if (size <= 0) return(0);

  for (r = 1; r <= number; r++) sum += ((random() % size)+1);
  return(sum);
}

int scan_number(char *text, int *rval)
{
  int	length;
  if (1!=sscanf(text, " %i %n", rval, &length))
    return 0;
  if (text[length] != 0)
    return 0;
  return 1;
}

/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different or end of both                 */
int str_cmp(char *arg1, char *arg2)
{
	int chk, i;


        if ((!arg2) || (!arg1))
	  return(1);

	for (i = 0; *(arg1 + i) || *(arg2 + i); i++)
		if (chk = LOWER(*(arg1 + i)) - LOWER(*(arg2 + i)))
			if (chk < 0)
				return (-1);
			else 
				return (1);
	return(0);
}



/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different, end of both, or n reached     */
int strn_cmp(char *arg1, char *arg2, int n)
{
	int chk, i;

	for (i = 0; (*(arg1 + i) || *(arg2 + i)) && (n>0); i++, n--)
		if (chk = LOWER(*(arg1 + i)) - LOWER(*(arg2 + i)))
			if (chk < 0)
				return (-1);
			else 
				return (1);

	return(0);
}



/* writes a string to the log */
void log(char *str)
{
	long ct;
	char *tmstr, buf[100];
	struct descriptor_data *i;


	ct = time(0);
	tmstr = asctime(localtime(&ct));
	*(tmstr + strlen(tmstr) - 1) = '\0';
	fprintf(stderr, "%s :: LOG : %s\n", tmstr, str);

	
	if (str) sprintf(buf,"\n\rLOG: %s\n\r",str);
       	for (i = descriptor_list; i; i = i->next)
	 if ((!i->connected) && (GetMaxLevel(i->character)>= 57) &&
			(IS_SET(i->character->specials.act, PLR_LOGS)))
	       		write_to_q(buf, &i->output);
}

void dlog(char *str)
{
	long ct;
	char *tmstr, buf[100];
	struct descriptor_data *i;


	ct = time(0);
	tmstr = asctime(localtime(&ct));
	*(tmstr + strlen(tmstr) - 1) = '\0';
	fprintf(stderr, "%s :: DEBUG : %s\n", tmstr, str);

	
	if (str) sprintf(buf,"\n\rLOG: %s\n\r",str);
       	for (i = descriptor_list; i; i = i->next)
	 if ((!i->connected) && (GetMaxLevel(i->character)>= 57) &&
			(IS_SET(i->character->specials.act, PLR_LOGS)))
	       		write_to_q(buf, &i->output);
}

void slog(char *str)
{
	long ct;
	char *tmstr;

	ct = time(0);
	tmstr = asctime(localtime(&ct));
	*(tmstr + strlen(tmstr) - 1) = '\0';
	fprintf(stderr, "%s :: %s\n", tmstr, str);

}
	


void sprintbit(unsigned long vektor, char *names[], char *result)
{
  long nr;
  
  *result = '\0';
  
  for(nr=0; vektor; vektor>>=1)
    {
      if(IS_SET(1, vektor))
	if (*names[nr] != '\n') {
	  strcat(result,names[nr]);
	  strcat(result," ");
	} else {
	  strcat(result,"UNDEFINED");
	  strcat(result," ");
	}
      if (*names[nr] != '\n')
	nr++;
    }
  
  if (!*result)
    strcat(result, "NOBITS");
}



void sprinttype(int type, char *names[], char *result)
{
	int nr;

	for(nr=0;(*names[nr]!='\n');nr++);
	if(type < nr)
		strcpy(result,names[type]);
	else
		strcpy(result,"UNDEFINED");
}


/* Calculate the REAL time passed over the last t2-t1 centuries (secs) */
struct time_info_data real_time_passed(time_t t2, time_t t1)
{
	long secs;
	struct time_info_data now;

	secs = (long) (t2 - t1);

  now.hours = (secs/SECS_PER_REAL_HOUR) % 24;  /* 0..23 hours */
  secs -= SECS_PER_REAL_HOUR*now.hours;

  now.day = (secs/SECS_PER_REAL_DAY);          /* 0..34 days  */
  secs -= SECS_PER_REAL_DAY*now.day;

	now.month = -1;
  now.year  = -1;

	return now;
}



/* Calculate the MUD time passed over the last t2-t1 centuries (secs) */
struct time_info_data mud_time_passed(time_t t2, time_t t1)
{
	long secs;
	struct time_info_data now;

	secs = (long) (t2 - t1);

  now.hours = (secs/SECS_PER_MUD_HOUR) % 24;  /* 0..23 hours */
  secs -= SECS_PER_MUD_HOUR*now.hours;

  now.day = (secs/SECS_PER_MUD_DAY) % 35;     /* 0..34 days  */
  secs -= SECS_PER_MUD_DAY*now.day;

	now.month = (secs/SECS_PER_MUD_MONTH) % 17; /* 0..16 months */
  secs -= SECS_PER_MUD_MONTH*now.month;

  now.year = (secs/SECS_PER_MUD_YEAR);        /* 0..XX? years */

	return now;
}



struct time_info_data age(struct char_data *ch)
{
	struct time_info_data player_age;

	player_age = mud_time_passed(time(0),ch->player.time.birth);

  player_age.year += 17;   /* All players start at 17 */

	return(player_age);
}


char in_group ( struct char_data *ch1, struct char_data *ch2)
{

/* 
   three possibilities ->
   1.  char is char2's master
   2.  char2 is char's master
   3.  char and char2 follow same.
  

    otherwise not true.
 
*/

   if (ch1 == ch2)
      return(TRUE);

   if ((!ch1) || (!ch2))
      return(0);

   if ((!ch1->master) && (!ch2->master))
      return(0);

   if (ch2->master)
      if (!strcmp(GET_NAME(ch1),GET_NAME(ch2->master))) {
         return(1);
      }

   if (ch1->master)
      if (!strcmp(GET_NAME(ch1->master),GET_NAME(ch2))) {
         return(1);
      }

   if ((ch2->master) && (ch1->master)) 
      if (!strcmp(GET_NAME(ch1->master),GET_NAME(ch2->master))) {
         return(1);
      }
  

   return(0);
}


/*
  more new procedures 
*/


/*
   these two procedures give the player the ability to buy 2*bread
   or put all.bread in bag, or put 2*bread in bag...
*/

int getall(char *name, char *newname)
{
   char arg[40]="\0\0\0",tmpname[80]="\0\0\0\0\0", otname[80]="\0";
   char prd;

   sscanf(name,"%s ",&otname);   /* reads up to first space */

   if (strlen(otname)<5)
      return(FALSE);

   sscanf(otname,"%3s%c%s",&arg,&prd,&tmpname);

   if (prd != '.')
     return(FALSE);
   if (tmpname == NULL) 
      return(FALSE);
   if (strcmp(arg,"all"))
      return(FALSE);

   while (*name != '.')
       name++;

   name++;

   for (; *newname = *name; name++,newname++);
   return(TRUE);
}


int getabunch(char *name, char  *newname)
{
   int num=0;
   char tmpname[80] = "\0";

   sscanf(name,"%d*%s",&num,&tmpname);
   if (tmpname[0] == '\0')
      return(FALSE);
   if (num < 1)
      return(FALSE);
   if (num>9)
      num = 9;

   while (*name != '*')
       name++;

   name++;

   for (; *newname = *name; name++,newname++);

   return(num);

}


int DetermineExp( struct char_data *mob, int exp_flags)
{

int base;
int phit;
int sab;

/* 
reads in the monster, and adds the flags together 
for simplicity, 1 exceptional ability is 2 special abilities 
*/

    if (GetMaxLevel(mob) < 0)
       return(1);

    switch(GetMaxLevel(mob)) {

    case 0:   base = 5;
              phit = 1;
              sab = 10;
              break;

    case 1:   base = 10;
              phit = 1;
              sab =  15;
              break;

    case 2:   base = 20;
              phit = 2;
              sab =  20;
              break;


    case 3:   base = 35;
              phit = 3;
              sab =  25;
              break;

    case 4:   base = 60;
              phit = 4;
              sab =  30;
              break;

    case 5:   base = 90;
              phit = 5;
              sab =  40;
              break;

    case 6:   base = 150;
              phit = 6;
              sab =  75;
              break;

    case 7:   base = 225;
              phit = 8;
              sab =  125;
              break;

    case 8:   base = 600;
              phit = 12;
              sab  = 175;
              break;

    case 9:   base = 900;
              phit = 14;
              sab  = 300;
              break;

    case 10:   base = 1100;
              phit  = 15;
              sab   = 450;
              break;

    case 11:   base = 1300;
              phit  = 16;
              sab   = 700;
              break;

    case 12:   base = 1550;
              phit  = 17;
              sab   = 700;
              break;

    case 13:   base = 1800;
              phit  = 18;
              sab   = 950;
              break;

    case 14:   base = 2100;
              phit  = 19;
              sab   = 950;
              break;

    case 15:   base = 2400;
              phit  = 20;
              sab   = 1250;
              break;

    case 16:   base = 2700;
              phit  = 23;
              sab   = 1250;
              break;

    case 17:   base = 3000;
              phit  = 25;
              sab   = 1550;
              break;

    case 18:   base = 3500;
              phit  = 28;
              sab   = 1550;
              break;

    case 19:   base = 4000;
              phit  = 30;
              sab   = 2100;
              break;

    case 20:   base = 4500;
              phit  = 33;
              sab   = 2100;
              break;

    case 21:   base = 5000;
              phit  = 35;
              sab   =  2600;
              break;

    case 22:   base = 6000;
              phit  = 40;
              sab   = 3000;
              break;

    case 23:   base = 7000;
              phit  = 45;
              sab   = 3500;
              break;

    case 24:   base = 8000;
              phit  = 50;
              sab   = 4000;
              break;

    case 25:   base = 9000;
              phit  = 55;
              sab   = 4500;
              break;

    case 26:   base = 10000;
              phit  = 60;
              sab   =  5000;
              break;

    case 27:   base = 12000;
              phit  = 70;
              sab   = 6000;
              break;

    case 28:   base = 14000;
              phit  = 80;
              sab   = 7000;
              break;

    case 29:   base = 16000;
              phit  = 90;
              sab   = 8000;
              break;

    case 30:   base = 20000;
              phit  = 100;
              sab   = 10000;
              break;

      default : base = 25000;
                phit = 150;
                sab  = 20000;
                break;
    }

    return(base + (phit * GET_HIT(mob)) + (sab * exp_flags));


}

void down_river( int pulse )
{
  struct char_data *ch, *tmp;
  struct obj_data *obj_object, *next_obj;
  int rd, or;
  char buf[80];
  struct room_data *rp;

  if (pulse < 0) 
     return;

  for(ch = character_list; ch; ch = tmp) 
  {
    tmp = ch->next;
    if(!IS_NPC(ch)) 
    {
      if (ch->in_room != NOWHERE) 
      {
	if (real_roomp(ch->in_room)->sector_type == SECT_WATER_NOSWIM)
          if ((real_roomp(ch->in_room))->river_speed > 0) 
	  {
            if ((pulse % (real_roomp(ch->in_room))->river_speed)==0) 
	    {
              if (((real_roomp(ch->in_room))->river_dir<=5) &&
		  ((real_roomp(ch->in_room))->river_dir>=0)) 
	      {
                rd = (real_roomp(ch->in_room))->river_dir;
       		for(obj_object = (real_roomp(ch->in_room))->contents;
		    obj_object; obj_object = next_obj) 
		{
		  next_obj = obj_object->next_content;
		  if ((real_roomp(ch->in_room))->dir_option[rd])
		  {
		    obj_from_room(obj_object);
   	            obj_to_room(obj_object,
		      (real_roomp(ch->in_room))->dir_option[rd]->to_room);
                  }
		}
/*
   flyers don't get moved
*/
                   if(!IS_AFFECTED(ch,AFF_FLYING)) 
		   {
		     rp = real_roomp(ch->in_room);
		     if(rp && rp->dir_option[rd] && rp->dir_option[rd]->to_room && 
			(EXIT(ch, rd)->to_room != NOWHERE)) 
                     {
      		         if (ch->specials.fighting) 
			 {
                               stop_fighting(ch);
			 }
		         sprintf(buf, "You drift %s...\n\r", dirs[rd]);
                         send_to_char(buf,ch);
			 if(MOUNTED(ch))
			 {
			   or = ch->in_room;
			   char_from_room(ch);
			   char_from_room(MOUNTED(ch));
			   char_to_room(ch,(real_roomp(or))->dir_option[rd]->to_room);
			   char_to_room(MOUNTED(ch),(real_roomp(or))->dir_option[rd]->to_room);
                	   do_look(ch, "\0",15);
                         }
			 else
			 if(RIDDEN(ch))
			 {
			   or = ch->in_room;
			   char_from_room(ch);
			   char_from_room(RIDDEN(ch));
			   char_to_room(ch,(real_roomp(or))->dir_option[rd]->to_room);
			   char_to_room(RIDDEN(ch),(real_roomp(or))->dir_option[rd]->to_room);
                	   do_look(ch, "\0",15);
                         }
                         else
			 {
			   or = ch->in_room;
			   char_from_room(ch);
			   char_to_room(ch,(real_roomp(or))->dir_option[rd]->to_room);
                	   do_look(ch, "\0",15);
                         }
			 
                         if (IS_SET(RM_FLAGS(ch->in_room),DEATH)&&GetMaxLevel(ch) < LOW_IMMORTAL) 
			 {
                           death_cry(ch);
                           zero_rent(ch);
                           extract_char(ch);
                         }
		       }
		    }
		 }
	      }
	   }
        }
      }
    }
 }

void RoomSave(struct char_data *ch, int start, int end)
{
   char fn[80], temp[2048], dots[500];
   int rstart, rend, i, j, k, x;
   struct extra_descr_data *exptr;
   FILE *fp;
   struct room_data	*rp;
   struct room_direction_data	*rdd;
   char buf[256];

   sprintf(buf,"%s/%s.wld",MKZONE_PATH,ch->player.name);

   if ((fp = fopen(buf,"w")) == NULL) {
     send_to_char("Can't write to disk now..try later \n\r",ch);
     return;
   }


   rstart = start;
   rend = end;

   if (((rstart <= -1) || (rend <= -1)) || 
       ((rstart > 40000) || (rend > 40000))){
    send_to_char("I don't know those room #s.  make sure they are all\n\r",ch);
    send_to_char("contiguous.\n\r",ch);
    fclose(fp);
    return;
   }

   send_to_char("Saving\n",ch);
   strcpy(dots, "\0");
   
   for (i=rstart;i<=rend;i++) {

     rp = real_roomp(i);
     if (rp==NULL)
       continue;


     strcat(dots, ".");

/*
   strip ^Ms from description
*/
     x = 0;

     if (!rp->description) {
       CREATE(rp->description, char, 128);
       strcpy(rp->description, "Empty");
     }

     for (k = 0; k <= strlen(rp->description); k++) {
       if (rp->description[k] != 13)
	 temp[x++] = rp->description[k];
     }
     temp[x] = '\0';

     fprintf(fp,"#%d\n%s~\n%s~\n",rp->number,rp->name,
 	                            temp);
     if (!rp->tele_targ) {
        fprintf(fp,"%d %d %d",rp->zone, rp->room_flags, rp->sector_type);
      } else {
	fprintf(fp, "%d %d -1 %d %d %d %d", rp->zone, rp->room_flags,
		rp->tele_time, rp->tele_targ, 
		rp->tele_look, rp->sector_type);
      }
     if (rp->sector_type == SECT_WATER_NOSWIM) {
        fprintf(fp," %d %d",rp->river_speed,rp->river_dir);
     } 

     fprintf(fp,"\n");     

     for (j=0;j<6;j++) {
       rdd = rp->dir_option[j];
       if (rdd) {
          fprintf(fp,"D%d\n",j);

	  if (rdd->general_description) {
	   if (strlen(rdd->general_description) > 0)
              x = 0;

              for (k = 0; k <= strlen(rdd->general_description); k++) {
                 if (rdd->general_description[k] != 13)
	            temp[x++] = rdd->general_description[k];
              }
	      temp[x] = '\0';

            fprintf(fp,"%s~\n", temp);
	  } else {
	    fprintf(fp,"~\n");
	  }

	  if (rdd->keyword) {
	   if (strlen(rdd->keyword)>0)
	     fprintf(fp, "%s~\n",rdd->keyword);
	  } else { 
	    fprintf(fp, "~\n");
	  }

	  if (IS_SET(rdd->exit_info, EX_PICKPROOF)) {
	    fprintf(fp, "2");
	  } else if (IS_SET(rdd->exit_info, EX_ISDOOR)) {
	    fprintf(fp, "1");
          } else {
	    fprintf(fp, "0");
	  }

	  fprintf(fp," %d ", 
		  rdd->key);

	  fprintf(fp,"%d\n", rdd->to_room);
       }
     }

/*
  extra descriptions..
*/

   for (exptr = rp->ex_description; exptr; exptr = exptr->next) {
     x = 0;

    if (exptr->description) {
      for (k = 0; k <= strlen(exptr->description); k++) {
       if (exptr->description[k] != 13)
	 temp[x++] = exptr->description[k];
      }
      temp[x] = '\0';

     fprintf(fp,"E\n%s~\n%s~\n", exptr->keyword, temp);
    }
   }

   fprintf(fp,"S\n");

   }

   fclose(fp);
   send_to_char(dots, ch);
   send_to_char("\n\rDone\n\r",ch);
}


void RoomLoad( struct char_data *ch, int start, int end)
{
  FILE *fp;
  int rnum, i, vnum, flag, tmp, found = FALSE, x;
  char *temp, chk[50];
  struct extra_descr_data *new_descr, *exptr, *nptr, ignore_descr;
  struct room_data dummy, *rp;
  char buf[256];

  sprintf(buf,"%s/%s.wld",MKZONE_PATH,ch->player.name);

  if ((fp = fopen(buf,"r")) == NULL) 
  {
    send_to_char("You don't appear to have an area...\n\r",ch);
    return;
  }
  
  send_to_char("Searching and loading rooms\n\r",ch);
  
  while ((!found) && ((x = feof(fp)) != TRUE)) {
    
    fscanf(fp, "#%d\n",&vnum);
    if ((vnum >= start) && (vnum <= end)) {
      if (vnum == end)
	found = TRUE;
      
      if (NULL==(rp=hash_find(&room_db, vnum))) {
	rp = (void*)malloc(sizeof(*rp));
	bzero(rp, sizeof(*rp));
	hash_enter(&room_db, vnum, rp);
	send_to_char("+",ch);
      } else {
	if (rp->people) {
	  act("$n reaches down and scrambles reality.", FALSE, ch, NULL,
	      rp->people, TO_ROOM);
	}
	cleanout_room(rp);
	send_to_char("-",ch);
      }
      
      rp->number = vnum;
      load_one_room(fp, rp);
      
    } else {
      send_to_char(".",ch);
      /*  read w/out loading */
      load_one_room(fp, &dummy);
      cleanout_room(&dummy);
    }
  }
  fclose(fp);
  
  if (!found) {
    send_to_char("\n\rThe room number(s) that you specified could not all be found.\n\r",ch);
  } else {
    send_to_char("\n\rDone.\n\r",ch);
  }
  
}    

void fake_setup_dir(FILE *fl, int room, int dir)
{
	int tmp;
	char buf[256], *temp;

	temp = (char *)&buf;

	temp =	fread_string(fl); /* descr */
	temp =	fread_string(fl); /* key */

	fscanf(fl, " %d ", &tmp); 
	fscanf(fl, " %d ", &tmp);
	fscanf(fl, " %d ", &tmp);
}


int IsHumanoid( struct char_data *ch)
{
/* these are all very arbitrary */

  switch(GET_RACE(ch))
    {
    case RACE_HALFBREED:
    case RACE_HUMAN:
    case RACE_GNOME:
    case RACE_ELVEN:
    case RACE_DWARF:
    case RACE_HALFLING:
    case RACE_ORC:
    case RACE_LYCANTH:
    case RACE_UNDEAD:
    case RACE_GIANT:
    case RACE_GOBLIN:
    case RACE_DEVIL:
    case RACE_TROLL:
    case RACE_VEGMAN:
    case RACE_MFLAYER:
    case RACE_DEMON:
    case RACE_GHOST:
    case RACE_PRIMATE:
      return(TRUE);
      break;

    default:
      return(FALSE);
      break;
    }
}

int IsAnimal( struct char_data *ch)
{
  switch(GET_RACE(ch))
    {
    case RACE_PREDATOR:
    case RACE_FISH:
    case RACE_BIRD:
    case RACE_HERBIV:
    case RACE_ANIMAL:
    case RACE_LYCANTH:
      return(TRUE);
      break;
    default:
      return(FALSE);
      break;
    }
}

int IsUndead( struct char_data *ch)
{

  switch(GET_RACE(ch)) 
  {
    case RACE_UNDEAD: 
    case RACE_GHOST:
      return(TRUE);
      break;
    default:
      return(FALSE);
      break;
  }
}

int IsLycanthrope( struct char_data *ch)
{
  switch (GET_RACE(ch)) {
  case RACE_LYCANTH:
    return(TRUE);
    break;
  default:
    return(FALSE);
    break;
  }

}

int IsDiabolic( struct char_data *ch)
{
  switch(GET_RACE(ch))
    {
    case RACE_DEMON:
    case RACE_DEVIL:
      return(TRUE);
      break;
    default:
      return(FALSE);
      break;
    }

}

int IsReptile( struct char_data *ch)
{
  switch(GET_RACE(ch)) {
    case RACE_REPTILE:
    case RACE_DRAGON:
    case RACE_DINOSAUR:
    case RACE_SNAKE:
      return(TRUE);
      break;
    default:
      return(FALSE);
      break;
    }
}

int HasHands( struct char_data *ch)
{

  if (IsHumanoid(ch))
    return(TRUE);
  if (IsUndead(ch)) 
    return(TRUE);
  if (IsLycanthrope(ch)) 
    return(TRUE);
  if (IsDiabolic(ch))
    return(TRUE);
  if (GET_RACE(ch) == RACE_DRAGON)
   return(FALSE);
}

int IsPerson( struct char_data *ch)
{

  switch(GET_RACE(ch))
    {
    case RACE_HUMAN:
    case RACE_ELVEN:
    case RACE_DWARF:
    case RACE_HALFLING:
    case RACE_GNOME:
      return(TRUE);
      break;

    default:
      return(FALSE);
      break;

    }

}


int IsExtraPlanar( struct char_data *ch)
{
  switch(GET_RACE(ch)) {
  case RACE_DEMON:
  case RACE_DEVIL:
  case RACE_PLANAR:
  case RACE_ELEMENT:
    return(TRUE);
    break;
  default:
    return(FALSE);
    break;
  }
}

/*
int IsUndead( struct char_data *ch)
int IsUndead( struct char_data *ch)
*/


void SetHunting( struct char_data *ch, struct char_data *tch)
{
   int persist, dist;
   char buf[256];
 
   persist =  GetMaxLevel(ch);
/*   persist *= (int) GET_ALIGNMENT(ch) / 100; */
   persist *= (int) GET_ALIGNMENT(ch) / 200;

   if (persist < 0)
     persist = -persist;


   dist = GET_INT(ch) + GetMaxLevel(ch);

/*   dist = GET_ALIGNMENT(tch) - GET_ALIGNMENT(ch); */

   dist = (dist > 0) ? dist : -dist;
   if (Hates(ch, tch))
       dist *=2;

   SET_BIT(ch->specials.act, ACT_HUNTING);
   ch->specials.hunting = tch;
   ch->hunt_dist = dist;
   ch->persist = persist;
   ch->old_room = ch->in_room;

  if (GetMaxLevel(tch) >= IMMORTAL) 
  {
    sprintf(buf, ">>%s is hunting you from %s\n\r", 
    ch->player.short_descr,(real_roomp(ch->in_room))->name);
    send_to_char(buf, tch);
  }
}

#define GUARD_VNUM 3060
#define GUARD2_VNUM 3069

void CallForGuard( struct char_data *ch, struct char_data *vict, int lev)
{
  struct char_data *i;
  if (lev == 0) lev = 3;

  for(i=character_list;i&&lev>0;i=i->next) 
  {
    if(IS_NPC(i) && (i!=ch)) 
    {
      if(!i->specials.fighting) 
      {
	if((mob_index[i->nr].virtual == GUARD_VNUM)||(mob_index[i->nr].virtual == GUARD2_VNUM))
	{
	  if (number(1,6) >= 3) 
	  {
	    if (!IS_SET(i->specials.act, ACT_HUNTING)) 
	    {
              if (vict) 
              {
		SetHunting(i, vict);
                lev--;
              }
	    }
	  }
	}
      }
    }
  }
}

void CallForAGuard(struct char_data *ch,struct char_data *vict,int lev)
{
  int zone;
  struct char_data *point;

  if(lev == 0) lev = 3;

  for(point=character_list;point&&(lev>0);point=point->next)
  {
    if(IS_NPC(point) &&
      (ch != point) &&
      (!point->specials.fighting) &&
      (real_roomp(ch->in_room)->zone == real_roomp(point->in_room)->zone) &&
      (IS_SET(point->specials.act,ACT_PROTECTOR)))
    {
      if(number(0,1))
      {
	if(!IS_SET(point->specials.act,ACT_HUNTING))
	{
	  if(vict)
	  {
	    SetHunting(point,vict);
	    lev--;
          }
        }
      }
    }
  }
}
      
void StandUp (struct char_data *ch)
{
   if ((GET_POS(ch)<POSITION_STANDING) && 
       (GET_POS(ch)>POSITION_STUNNED)) {
       if (ch->points.hit > (ch->points.max_hit / 2))
         act("$n quickly stands up.", 1, ch,0,0,TO_ROOM);
       else if (ch->points.hit > (ch->points.max_hit / 6))
         act("$n slowly stands up.", 1, ch,0,0,TO_ROOM);
       else 
         act("$n gets to $s feet very slowly.", 1, ch,0,0,TO_ROOM);
       GET_POS(ch)=POSITION_STANDING;
   }
}


void FighterMove( struct char_data *ch)
{
  int num;
  num = number(1,4);

  switch(num)
  {
    case 1:
      if(!ch->skills[SKILL_BASH].learned)
        ch->skills[SKILL_BASH].learned = 10 + GetMaxLevel(ch)*4;
      do_bash(ch, GET_NAME(ch->specials.fighting), 0);
      break;
    case 2:
      if(ch->equipment[WIELD] || ch->equipment[WIELD_TWOH]) 
      { 
        if(!ch->skills[SKILL_DISARM].learned)
          ch->skills[SKILL_DISARM].learned = 10 + GetMaxLevel(ch)*4;
        do_disarm(ch, GET_NAME(ch->specials.fighting), 0);
      }
      else
      {
        if (!ch->skills[SKILL_DISARM].learned)
          ch->skills[SKILL_DISARM].learned = 60 + GetMaxLevel(ch)*4;
        do_disarm(ch, GET_NAME(ch->specials.fighting), 0);
      }
      break;
    case 3:
    case 4:
      if (!ch->skills[SKILL_KICK].learned)
        ch->skills[SKILL_KICK].learned = 10 + GetMaxLevel(ch)*4;
      do_kick(ch, GET_NAME(ch->specials.fighting), 0);
      break;
  }
}

void DevelopHatred( struct char_data *ch, struct char_data *v)
{
   int diff, patience, var;

   if (Hates(ch, v))
     return;

  if (ch == v)
    return;

/*
  diff = GET_ALIGNMENT(ch) - GET_ALIGNMENT(v);
  (diff > 0) ? diff : diff = -diff;
  
  diff /= 20;
  
  patience = (int) 100 * (float) (GET_HIT(ch) / GET_MAX_HIT(ch));

  var = number(1,40) - 20;

  if (patience+var < diff)
     AddHated(ch, v);
*/

  AddHated(ch,v);

}

void Teleport( int pulse )
{
  char buf[256];
  struct char_data *ch, *tmp, *pers;
  struct obj_data *obj_object, *temp_obj;
  int or;
  struct room_data	*rp, *dest;
  
  if (pulse < 0) 
    return;
  
  for (ch = character_list; ch; ch = ch->next) {
    if (IS_NPC(ch))
      continue;
    rp = real_roomp(ch->in_room);
    if (rp &&
	(rp)->tele_targ > 0 &&
	rp->tele_targ != rp->number &&
	(rp)->tele_time > 0 &&
	(pulse % (rp)->tele_time)==0) {

      dest = real_roomp(rp->tele_targ);
      if (!dest) {
	sprintf(buf,"invalid tele_target:ROOM %s",rp->name);
	log(buf);
	continue;
      }
      
      obj_object = (rp)->contents;
      while (obj_object) {
	temp_obj = obj_object->next_content;
	obj_from_room(obj_object);
	obj_to_room(obj_object, (rp)->tele_targ);
	obj_object = temp_obj;
      }
      
      while(rp->people/* should never fail */) {

	/* find an NPC in the room */
	for (tmp = rp->people; tmp; tmp = tmp->next_in_room) {
	  if (IS_NPC(tmp))
	    break;
	}

	if (tmp==NULL)
	  break; /* we've run out of NPCs */

	or = tmp->in_room;
	char_from_room(tmp); /* the list of people in the room has changed */
	char_to_room(tmp, rp->tele_targ);
#if 0
	/* it's a bloody NPC!? why look? */
	if (rp->tele_look) {
	  do_look(tmp, "\0",15);
	}
#endif
	if (IS_SET(dest->room_flags, DEATH)) {
	  death_cry(tmp);
          if (IS_NPC(tmp) && (IS_SET(tmp->specials.act, ACT_POLYSELF))) {
      /*
       *   take char from storage, to room     
       */
          pers = tmp->desc->original;
          char_from_room(pers);
      char_to_room(pers, tmp->in_room);
      SwitchStuff(tmp, pers);
      zero_rent(pers);
      extract_char(tmp);
      tmp = pers;
    }
          zero_rent(tmp);
	  extract_char(tmp);
	}
      }
      or = ch->in_room;
      char_from_room(ch); 
      char_to_room(ch, rp->tele_targ);
      if (rp->tele_look) {
	do_look(ch, "\0",15);
      }
      if (IS_SET(dest->room_flags, DEATH) && 
	  GetMaxLevel(ch) < LOW_IMMORTAL) {
	death_cry(ch);

    if (IS_NPC(ch) && (IS_SET(ch->specials.act, ACT_POLYSELF))) {
      /*
       *   take char from storage, to room     
       */
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
      }
    }
  }
}

int HasObject( struct char_data *ch, int ob_num)
{
int j, found;
struct obj_data *i;

/*
   equipment too
*/

found = 0;

        for (j=0; j<MAX_WEAR; j++)
     	   if (ch->equipment[j])
       	     found += RecCompObjNum(ch->equipment[j], ob_num);

      if (found > 0)
	return(TRUE);

  /* carrying  */
       	for (i = ch->carrying; i; i = i->next_content)
       	  found += RecCompObjNum(i, ob_num);

     if (found > 0)
       return(TRUE);
     else
       return(FALSE);
}


int room_of_object(struct obj_data *obj)
{
  if (obj->in_room != NOWHERE)
    return obj->in_room;
  else if (obj->carried_by)
    return obj->carried_by->in_room;
  else if (obj->equipped_by)
    return obj->equipped_by->in_room;
  else if (obj->in_obj)
    return room_of_object(obj->in_obj);
  else
    return NOWHERE;
}

struct char_data *char_holding(struct obj_data *obj)
{
  if (obj->in_room != NOWHERE)
    return NULL;
  else if (obj->carried_by)
    return obj->carried_by;
  else if (obj->equipped_by)
    return obj->equipped_by;
  else if (obj->in_obj)
    return char_holding(obj->in_obj);
  else
    return NULL;
}


int RecCompObjNum( struct obj_data *o, int obj_num)
{

int total=0;
struct obj_data *i;

  if (obj_index[o->item_number].virtual == obj_num)
    total = 1;

  if (ITEM_TYPE(o) == ITEM_CONTAINER) {
    for (i = o->contains; i; i = i->next_content)
      total += RecCompObjNum( i, obj_num);
  }
  return(total);

}

void RestoreChar(struct char_data *ch)
{
  GET_MANA(ch) = GET_MAX_MANA(ch);
  GET_HIT(ch) = GET_MAX_HIT(ch);
  GET_MOVE(ch) = GET_MAX_MOVE(ch); 

  if (GetMaxLevel(ch) < LOW_IMMORTAL) {
    GET_COND(ch,THIRST) = 24;
    GET_COND(ch,FULL) = 24;
  } else {
    GET_COND(ch,THIRST) = -1;
    GET_COND(ch,FULL) = -1;
  }
}


void RemAllAffects( struct char_data *ch)
{
  spell_dispel_magic(IMPLEMENTOR,ch,ch,0);
}



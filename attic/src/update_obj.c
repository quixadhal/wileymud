#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "global.h"
#include "bug.h"
#include "db.h"

#define MAX_OBJ_SAVE 255

struct rental_header 
{
  char  inuse;
  int   length;
  char owner[20];    /* Name of player                     */
};

struct obj_file_elem 
{
        sh_int item_number;
        int value[4];
        int extra_flags;
        int weight;
        int timer;
        long bitvector;
        struct obj_affected_type affected[MAX_OBJ_AFFECT];
}; 

struct obj_file_u
{
        int gold_left;     /* Number of goldcoins left at owner  */
        int total_cost;    /* The cost for all items, per day    */
        long last_update;  /* Time in seconds, when last updated */
        long minimum_stay; /* For stasis */
        int nobjects;      /* how many objects below */
        struct obj_file_elem objects[MAX_OBJ_SAVE];
                           /* We don't always allocate this much space
                              but it is handy for the times when you
                              need a fast one lying around.  */
}; 

main()
{
  update_obj_file();
}

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
  int i;

  /* r+b is for Binary Reading/Writing */
  if(!(fl = fopen("./pc.bak", "r+b")))
  {
    perror("   Opening object file for updating");
    exit(1);
  }
  
  pos = 0;
  while (!feof(fl)) 
  {
    /* read a rental header */

    no_read = fread(&rh, sizeof(rh), 1, fl);

    if (no_read<=0)
      break;

    if(no_read!=1) 
    {
      perror("corrupted object save file 1");
      exit(1);
    }


    if(!strcmp(rh.owner,"Yt"))
    {
      printf("FOUND HIM!\n");
      fread(&st,rh.length,1,fl);
      for(i=0;i<st.nobjects;i++)
      {
	printf("Ob Number: %d\n",st.objects[i].item_number);
      }
      exit(1);
    }

    printf("INUSE:%d\n",rh.inuse);
    printf("LENGT:%d\n",rh.length);
    printf("OWNER:%s\n",rh.owner);

    /* read in the char part of the rental data */

    if(rh.length > sizeof(st) )
    {
      no_read = fread(&st,sizeof(st),1,fl);
      fseek(fl,(rh.length - sizeof(st)),1);
    }
    else
      no_read = fread(&st,rh.length,1,fl);

    pos += no_read;
    
    if((!feof(fl)) && (no_read > 0) && rh.owner[0]) 
    {
      printf("   Processing %s[%d].\n",rh.owner,pos);
    }
  }
  fclose(fl);
  fclose(char_file);
}


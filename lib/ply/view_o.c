#include <stdio.h>
#include "../../structs.h"
#include "../../reception.h"

int main(int argc, char * argv[])
{
  FILE * fl;
  struct rental_header rh;
  struct obj_file_u st;
  int i,j;
  char name[40];
  char path[256];

  if(argc < 2)
  {
    printf("usage: view_o <who to view> \n");
    exit(1);
  }
 
  strcpy(name,argv[1]);
  printf("Opening Object file for %s\n",name);

  if(  ( fl = fopen(name,"r+b") ) == NULL )
  {
    fprintf(stderr,"Could not open file.\n");
    exit(1);
  }

  fread(&rh,sizeof(rh),1,fl);
  printf("RH.inuse = %d\n",rh.inuse);
  printf("RH.length = %d\n",rh.length);
  printf("RH.owner = %s\n",rh.owner);
  if(rh.inuse == 1)
  {
    fread(&st,rh.length,1,fl);

    for( i =0 ; i < st.nobjects; i++)
    {
      printf("Object Number : # %d\n",st.objects[i].item_number);
      printf("     value[0] : %d\n",st.objects[i].value[0]);
      printf("     value[1] : %d\n",st.objects[i].value[1]);
      printf("     value[2] : %d\n",st.objects[i].value[2]);
      printf("     value[3] : %d\n",st.objects[i].value[3]);
      printf("  Extra Flags : %d\n",st.objects[i].extra_flags);
      printf("       weight : %d\n",st.objects[i].weight);
      printf("        timer : %d\n",st.objects[i].timer);
      printf("    bitvector : %d\n",st.objects[i].bitvector);
      for(j=0;j<MAX_OBJ_AFFECT;j++)
        printf("      Affects : %d\n",st.objects[i].affected[j]);
    }
  } 
  fclose(fl);
} 

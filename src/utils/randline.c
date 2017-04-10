#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/timeb.h>

typedef struct line {
  struct line *next;
  char        *text;
} line;

int main(argc, argv)
int    argc;
char **argv;
{
  register long i, count= 0, final;
  FILE *fp= stdin;
  line  buffer, *currentline= &buffer;
  char *filename= NULL, tmp[1024];
  struct timeb tp;

  ftime(&tp);
  srandom((long)(tp.time+ tp.millitm));
  if(argc > 1) if(!(filename= (char *)strdup(argv[1]))) exit(-1);
  if(filename) if(!(fp= fopen(filename, "r"))) exit(-1);
  while(!feof(fp)) {
    if(fgets(tmp, 1024, fp)) {
      tmp[strlen(tmp)-1]= '\0';
      if(strlen(tmp)) {
        count++;
        if(!(currentline->text= (char *)strdup(tmp))) exit(-1);
        if(!(currentline->next=
          (struct line *)calloc(1, sizeof(struct line)))) exit(-1);
        currentline= currentline->next;
      }
    } else {
      if(feof(fp)) break;
    }
  }
  currentline= &buffer;
  for(final= i= (random()%count); i > 0; i--) {
    currentline= currentline->next;
  }
/*  printf("%s\n", currentline->text); */
  puts(currentline->text);
  return final;
}

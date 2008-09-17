/*
 *  This routine simply generates index files for all the virtual numbers.
 *  The output format is one per line, <vnum> <line> <byte-offset>
 */

#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include "include/bug.h"
#include "include/structs.h"
#include "include/main.h"
#include "include/utils.h"

#define _MAKE_INDEX_C
#include "include/make_index.h"

vnum_index *make_index(char *infile, char *outfile) {
  FILE *ifp, *ofp = NULL;
  vnum_index *VIndex;
  /* vnum *VNum; */
  register int i, j;
  /* register long OldPos; */
  long Line, Pos;
  char *tmp;

  if(!infile || !*infile) return NULL;
  VIndex= (vnum_index *)get_mem(1, sizeof(vnum_index));
  bzero(VIndex, sizeof(vnum_index));
  VIndex->VNum= (vnum *)get_mem(1, sizeof(vnum));
  bzero(VIndex->VNum, sizeof(vnum));
  ifp= open_file(infile, "r");
  if(!Quiet) {
    fprintf(stderr, "Indexing %s...", infile);
    fflush(stderr);
  }
  for(i= Line= Pos= 0;(tmp= get_line(ifp, &Line, &Pos, 1));) {
    if(!Quiet)
      spin(stderr);
    if(*tmp == '#') {
      sscanf(tmp+1, "%d", &(VIndex->VNum[i].Number));
      VIndex->VNum[i].Line= Line; VIndex->VNum[i].Pos= Pos;
      if(!verify_pos(ifp, Pos, '#')) {
        log_error("Sanity check failed! Wrong byte position!");
        fclose(ofp); fclose(ifp);
        exit(__LINE__);
      }
      i++;
      VIndex->VNum= (vnum *)get_more_mem((char *)VIndex->VNum,
                                         i+1, sizeof(vnum));
      bzero(&(VIndex->VNum[i]), sizeof(vnum));
    }
  }
  if(!Quiet)
    fprintf(stderr, "done.\n");
  if(!outfile || !*outfile) {
    VIndex->Count= i;
  } else {
    ofp= open_file(outfile, "w");
    fprintf(ofp, "Found %d Virtual numbers in %s.\n", i, infile);
    for(j= 0; j< i; j++)
      fprintf(ofp, "#%05d  Line % 6ld  Byte % 8ld\n",
              VIndex->VNum[j].Number, VIndex->VNum[j].Line,
              VIndex->VNum[j].Pos);
    fclose(ofp);
  }
  fclose(ifp);
  return VIndex;
}

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <strings.h>
#include <string.h>
#include "include/structs.h"
#include "include/main.h"
#include "include/utils.h"

#include "include/parse_wiley.h"

#define _DUMP_MAP_C
#include "include/dump_map.h"

const char terrains[] = "#.+,:%^~~#~";
/* const char terrains[] = "TicgfhMroAu"; */
/* Teleport, inside, city, field, forest, hill, mountain, river,
 * ocean, air, underwater
 */
/* This is so we can use terrain[-1] and get it to work right */
const char *terrain = &terrains[1];
const int pgmterrains[]= { 255, 15, 31, 63, 95, 127, 195, 47, 79, 1, 163 };
const int *pgmterrain = &pgmterrains[1];
const ppmcolour ppmterrains[] = { { 255, 0,   255 }, /* teleport */
                                  { 255, 255, 0   }, /* indoors */
                                  { 255, 0,   0   }, /* city */
                                  { 140, 200, 20  }, /* field */
                                  { 80,  150, 10  }, /* forest */
                                  { 150, 150, 150 }, /* hills */
                                  { 200, 200, 200 }, /* mountains */
                                  { 0,   0,   150 }, /* river */
                                  { 0,   0,   70  }, /* ocean */
                                  { 80,  80,  255 }, /* air */
                                  { 60,  0,   100 }  /* underwater */
                                };
const ppmcolour *ppmterrain = &ppmterrains[1];
const int exit_dir_to_bit[] = { EXIT_BIT_N, EXIT_BIT_E, EXIT_BIT_S,
                                EXIT_BIT_W, EXIT_BIT_U, EXIT_BIT_D };

/*
 * ASCII room layout... 21x7 characters == 6x8 rooms per lineprinter page.
 * "          |          "  U indicates an up exit exists... D is a down exit.
 * "  .-------+-------.  "  Sector-Type is the terrain.
 * "  |UD  Sector-Type|  "  Roomname is the Text name of the room (15 chars).
 * "--+RoomnameRoomnam+--"  Z#xxx is the zone number the room is in.
 * "  |Z#000  #[00000]|  "  #[xxxxx] is the vnum of the room.
 * "  `-------+-------'  "
 * "          |          "
 */

const maptemplate empty_shape = 
{ { "                     ",
  "                     ",
  "                     ",
  "                     ",
  "                     ",
  "                     ",
  "                     " } };

const maptemplate exit_shapes[] = {
/* 0000 */
{ { "                     ",
    "  .---------------.  ",
    "  |               |  ",
    "  |               |  ",
    "  |               |  ",
    "  `---------------'  ",
    "                     " } },
/* 0001 */
{ { "          |          ",
    "  .-------+-------.  ",
    "  |               |  ",
    "  |               |  ",
    "  |               |  ",
    "  `---------------'  ",
    "                     " } },
/* 0010 */
{ { "                     ",
    "  .---------------.  ",
    "  |               |  ",
    "  |               +--",
    "  |               |  ",
    "  `---------------'  ",
    "                     " } },
/* 0011 */
{ { "          |          ",
    "  .-------+-------.  ",
    "  |               |  ",
    "  |               +--",
    "  |               |  ",
    "  `---------------'  ",
    "                     " } },
/* 0100 */
{ { "                     ",
    "  .---------------.  ",
    "  |               |  ",
    "  |               |  ",
    "  |               |  ",
    "  `-------+-------'  ",
    "          |          " } },
/* 0101 */
{ { "          |          ",
    "  .-------+-------.  ",
    "  |               |  ",
    "  |               |  ",
    "  |               |  ",
    "  `-------+-------'  ",
    "          |          " } },
/* 0110 */
{ { "                     ",
    "  .---------------.  ",
    "  |               |  ",
    "  |               +--",
    "  |               |  ",
    "  `-------+-------'  ",
    "          |          " } },
/* 0111 */
{ { "          |          ",
    "  .-------+-------.  ",
    "  |               |  ",
    "  |               +--",
    "  |               |  ",
    "  `-------+-------'  ",
    "          |          " } },
/* 1000 */
{ { "                     ",
    "  .---------------.  ",
    "  |               |  ",
    "--+               |  ",
    "  |               |  ",
    "  `---------------'  ",
    "                     " } },
/* 1001 */
{ { "          |          ",
    "  .-------+-------.  ",
    "  |               |  ",
    "--+               |  ",
    "  |               |  ",
    "  `---------------'  ",
    "                     " } },
/* 1010 */
{ { "                     ",
    "  .---------------.  ",
    "  |               |  ",
    "--+               +--",
    "  |               |  ",
    "  `---------------'  ",
    "                     " } },
/* 1011 */
{ { "          |          ",
    "  .-------+-------.  ",
    "  |               |  ",
    "--+               +--",
    "  |               |  ",
    "  `---------------'  ",
    "                     " } },
/* 1100 */
{ { "                     ",
    "  .---------------.  ",
    "  |               |  ",
    "--+               |  ",
    "  |               |  ",
    "  `-------+-------'  ",
    "          |          " } },
/* 1101 */
{ { "          |          ",
    "  .-------+-------.  ",
    "  |               |  ",
    "--+               |  ",
    "  |               |  ",
    "  `-------+-------'  ",
    "          |          " } },
/* 1110 */
{ { "                     ",
    "  .---------------.  ",
    "  |               |  ",
    "--+               +--",
    "  |               |  ",
    "  `-------+-------'  ",
    "          |          " } },
/* 1111 */
{ { "          |          ",
    "  .-------+-------.  ",
    "  |               |  ",
    "--+               +--",
    "  |               |  ",
    "  `-------+-------'  ",
    "          |          " } }
};

/*
 * By convention, we are using the following coordinate system.
 * X represents the west and east exists, with west being negative.
 * Y is north-south, north is positive.
 * Z is up and down, with up being positive.
 */

void colour_3d(tree *Tree, coord_group *SoFar, int Colour, zones *Zones,
               rooms *Rooms, int RoomIndex, int VZone)
{
  int i,j;
  coord_group Myself;

  if(Rooms->Room[RoomIndex].Colour) return;
  Rooms->Room[RoomIndex].Colour= Colour;
  /* This will stop the graph from crossing zone boundries */
  /* if(Rooms->Room[RoomIndex].Zone != VZone) return SoFar; */
  i= Tree->Count++;
  if(!Tree->Rooms) Tree->Rooms= (mapnode *)get_mem(1, sizeof(mapnode));
  else Tree->Rooms= (mapnode *)get_more_mem((char *)Tree->Rooms,
                                           Tree->Count, sizeof(mapnode));
  Tree->Rooms[i].Room= &Rooms->Room[RoomIndex];
  Rooms->Room[RoomIndex].Location.x= SoFar->Current.x;
  Rooms->Room[RoomIndex].Location.y= SoFar->Current.y;
  Rooms->Room[RoomIndex].Location.z= SoFar->Current.z;
  if(SoFar->Current.x < SoFar->Min.x) SoFar->Min.x= SoFar->Current.x;
  if(SoFar->Current.y < SoFar->Min.y) SoFar->Min.y= SoFar->Current.y;
  if(SoFar->Current.z < SoFar->Min.z) SoFar->Min.z= SoFar->Current.z;
  if(SoFar->Current.x > SoFar->Max.x) SoFar->Max.x= SoFar->Current.x;
  if(SoFar->Current.y > SoFar->Max.y) SoFar->Max.y= SoFar->Current.y;
  if(SoFar->Current.z > SoFar->Max.z) SoFar->Max.z= SoFar->Current.z;
  for(j= 0; j< Rooms->Room[RoomIndex].ExitCount; j++) {
    if(Rooms->Room[RoomIndex].Exit[j].Error == EXIT_OK) {
      Tree->Rooms[i].Exits |=
        exit_dir_to_bit[Rooms->Room[RoomIndex].Exit[j].Direction];
      Myself= *SoFar;
      switch(Rooms->Room[RoomIndex].Exit[j].Direction) {
        case 0:  Myself.Current.y--; break; /* reversed for topleft map */
        case 1:  Myself.Current.x++; break;
        case 2:  Myself.Current.y++; break; /* reversed for topleft map */
        case 3:  Myself.Current.x--; break;
        case 4:  Myself.Current.z++; break;
        case 5:  Myself.Current.z--; break;
      }
      colour_3d(Tree, &Myself, Colour, Zones, Rooms,
                Rooms->Room[RoomIndex].Exit[j].RoomIndex, VZone);
      if(Myself.Min.x < SoFar->Min.x) SoFar->Min.x= Myself.Min.x;
      if(Myself.Min.y < SoFar->Min.y) SoFar->Min.y= Myself.Min.y;
      if(Myself.Min.z < SoFar->Min.z) SoFar->Min.z= Myself.Min.z;
      if(Myself.Max.x > SoFar->Max.x) SoFar->Max.x= Myself.Max.x;
      if(Myself.Max.y > SoFar->Max.y) SoFar->Max.y= Myself.Max.y;
      if(Myself.Max.z > SoFar->Max.z) SoFar->Max.z= Myself.Max.z;
    }
  }
}

/* int qcmp_x(const mapnode *a, const mapnode *b) */
int qcmp_x(const void *a, const void *b)
{
  return (((const mapnode *) a)->Room->Location.x - ((const mapnode *) b)->Room->Location.x);
  /* return (a->Room->Location.x - b->Room->Location.x); */
}

int qcmp_y(const void *a, const void *b)
{
  return (((const mapnode *) a)->Room->Location.y - ((const mapnode *) b)->Room->Location.y);
  /* return (a->Room->Location.y - b->Room->Location.y); */
}

int qcmp_z(const void *a, const void *b)
{
  return (((const mapnode *) a)->Room->Location.z - ((const mapnode *) b)->Room->Location.z);
  /* return (a->Room->Location.z - b->Room->Location.z); */
}

forest *build_map(zones *Zones, rooms *Rooms)
{
  forest *Forest;
  coord_group Found;
  int i, Current, k;

  if(!Quiet) {
    fprintf(stderr, "Generating Map Data...");
    fflush(stderr);
  }
  Forest= (forest *)get_mem(1, sizeof(forest));
  Forest->Count= 0;
  Forest->Tree= NULL;
  for(i= 0; i< Rooms->Count; i++) {
    if(!Quiet)
      spin(stderr);
    if(Rooms->Room[i].Colour) continue;
    Current= Forest->Count++;
    if(!Forest->Tree) Forest->Tree= (tree *)get_mem(1, sizeof(tree));
    else Forest->Tree= (tree *)get_more_mem((char *)Forest->Tree,
                                            Forest->Count, sizeof(tree));
    Forest->Tree[Current].Count= 0;
    Forest->Tree[Current].Rooms= NULL;
    bzero(&Forest->Tree[Current].Coords, sizeof(coord_group));
    bzero(&Found, sizeof(coord_group));
    Found.Min.x= Found.Min.y= Found.Min.z= INT_MAX;
    Found.Max.x= Found.Max.y= Found.Max.z= INT_MIN;
/* Found will have zero for Current coordinates */
    colour_3d(&Forest->Tree[Current], &Found, Forest->Count, Zones, Rooms,
              i, Rooms->Room[i].Zone);
    Forest->Tree[Current].Coords= Found;
    Forest->Tree[Current].Offset.x= -Forest->Tree[Current].Coords.Min.x;
    Forest->Tree[Current].Offset.y= -Forest->Tree[Current].Coords.Min.y;
    Forest->Tree[Current].Offset.z= -Forest->Tree[Current].Coords.Min.z;
    Forest->Tree[Current].MapSize.x= Forest->Tree[Current].Coords.Max.x -
                                     Forest->Tree[Current].Coords.Min.x + 1;
    Forest->Tree[Current].MapSize.y= Forest->Tree[Current].Coords.Max.y -
                                     Forest->Tree[Current].Coords.Min.y + 1;
    Forest->Tree[Current].MapSize.z= Forest->Tree[Current].Coords.Max.z -
                                     Forest->Tree[Current].Coords.Min.z + 1;
    if(Verbose)
      fprintf(stderr, "Map %d: %dx%dx%d\n", Current,
              Forest->Tree[Current].MapSize.x, Forest->Tree[Current].MapSize.y,
              Forest->Tree[Current].MapSize.z);
    for(k= 0; k < Forest->Tree[Current].Count; k++) {
      Forest->Tree[Current].Rooms[k].Room->Location.x +=
        Forest->Tree[Current].Offset.x;
      Forest->Tree[Current].Rooms[k].Room->Location.y +=
        Forest->Tree[Current].Offset.y;
      Forest->Tree[Current].Rooms[k].Room->Location.z +=
        Forest->Tree[Current].Offset.z;
    }
    qsort(Forest->Tree[Current].Rooms, Forest->Tree[Current].Count,
          sizeof(struct mapnode), qcmp_x);
    qsort(Forest->Tree[Current].Rooms, Forest->Tree[Current].Count,
          sizeof(struct mapnode), qcmp_y);
    qsort(Forest->Tree[Current].Rooms, Forest->Tree[Current].Count,
          sizeof(struct mapnode), qcmp_z);
  }
  if(!Quiet)
    fprintf(stderr, "done.\n");
  return Forest;
}

void make_2d_map(forest *Forest, zones *Zones, char *outfile, char *ppmpattern)
{
  register int i,j,k;
  FILE *ofp = NULL, *pfp;
  char **Map;
  ppmcolour **PMap;
  char ppmfile[256];

  if(!Quiet) {
    fprintf(stderr, "Generating Map Output...");
    fflush(stderr);
  }
  if(outfile && *outfile)
    ofp= open_file(outfile, "w");
  for(i= 0; i< Forest->Count; i++) {
    if(!Quiet)
      spin(stderr);
    Map= (char **)get_mem(Forest->Tree[i].MapSize.y, sizeof(char *));
    PMap= (ppmcolour **)get_mem(Forest->Tree[i].MapSize.y, sizeof(ppmcolour *));
    for(j= 0; j< Forest->Tree[i].MapSize.y; j++) {
      Map[j]= (char *)get_mem(Forest->Tree[i].MapSize.x+ 1, sizeof(char));
      memset(Map[j], ' ', Forest->Tree[i].MapSize.x);
      Map[j][Forest->Tree[i].MapSize.x]= '\0';
      PMap[j]= (ppmcolour *)get_mem(Forest->Tree[i].MapSize.x,
                sizeof(ppmcolour));
      bzero(PMap[j], Forest->Tree[i].MapSize.x* sizeof(ppmcolour));
    }
    if(ppmpattern && *ppmpattern) {
      sprintf(ppmfile, ppmpattern, i);
      pfp= open_file(ppmfile, "w");
    } else {
      pfp= NULL;
    }
/*
 * This assumes we have presorted on Z, to allow higher altitude rooms to
 * overwrite lower altitude rooms.
 */
    for(j= 0; j< Forest->Tree[i].Count; j++) {
      Map[Forest->Tree[i].Rooms[j].Room->Location.y][Forest->Tree[i].Rooms[j].Room->Location.x]= terrain[Forest->Tree[i].Rooms[j].Room->Sector];
      PMap[Forest->Tree[i].Rooms[j].Room->Location.y][Forest->Tree[i].Rooms[j].Room->Location.x]= ppmterrain[Forest->Tree[i].Rooms[j].Room->Sector];
    }
    fprintf(ofp, "Map #%d (%dx%d):\n", i, Forest->Tree[i].MapSize.x,
            Forest->Tree[i].MapSize.y);
    if(pfp) {
      fprintf(pfp,
              "P3\n# CREATOR: WileyMUD III (Map #%d (%dx%d)\n%3d %3d\n255\n",
              i, Forest->Tree[i].MapSize.x, Forest->Tree[i].MapSize.y,
              Forest->Tree[i].MapSize.x + 2, Forest->Tree[i].MapSize.y + 2);
    }
/*
 * for(j= 0; j< Forest->Tree[i].MapSize.y; j++)
 * for(j= Forest->Tree[i].MapSize.y- 1; j>= 0; j--)
 * Reversed order for printing.. normal coordinates have 0 at the bottom.
 */
    fprintf(ofp, "\\   ");
    for(j= 0; j< Forest->Tree[i].MapSize.x; j++)
      if(j/100) fprintf(ofp, "%c", ((j/100)%10)+'0');
      else fprintf(ofp, " ");
    fprintf(ofp, "\n");
    fprintf(ofp, " \\  ");
    for(j= 0; j< Forest->Tree[i].MapSize.x; j++)
      if(j/10) fprintf(ofp, "%c", ((j/10)%10)+'0');
      else fprintf(ofp, " ");
    fprintf(ofp, "\n");
    fprintf(ofp, "  \\ ");
    for(j= 0; j< Forest->Tree[i].MapSize.x; j++)
      fprintf(ofp, "%c", (j%10)+'0');
    fprintf(ofp, "\n");
    fprintf(ofp, "   +");
    for(j= 0; j< Forest->Tree[i].MapSize.x; j++)
      fprintf(ofp, "-");
    fprintf(ofp, "+\n");

    if(pfp) {
      for(j= 0; j< Forest->Tree[i].MapSize.x+ 2; j++)
        fprintf(pfp, "  0   0   0 ");
      fprintf(pfp, "\n");
    }
    for(j= 0; j< Forest->Tree[i].MapSize.y; j++) {
      fprintf(ofp, "%3d|%s|\n", j, Map[j]);
      if(pfp) {
        fprintf(pfp, "  0   0   0 ");
        for(k= 0; k< Forest->Tree[i].MapSize.x; k++)
          fprintf(pfp, "%3d %3d %3d ", PMap[j][k].Red, PMap[j][k].Green,
                  PMap[j][k].Blue);
        fprintf(pfp, "  0   0   0 ");
        fprintf(pfp, "\n");
      }
    }
    if(pfp) {
      for(j= 0; j< Forest->Tree[i].MapSize.x+ 2; j++)
        fprintf(pfp, "  0   0   0 ");
      fprintf(pfp, "\n");
      fclose(pfp);
    }

    fprintf(ofp, "   +");
    for(j= 0; j< Forest->Tree[i].MapSize.x; j++)
      fprintf(ofp, "-");
    fprintf(ofp, "+\n");
    fprintf(ofp, "List of Rooms on Map #%d:\n", i);
    for(j= 0; j< Forest->Tree[i].Count; j++)
      fprintf(ofp, "     (%d,%d,%d) %s [#%d] in %s [#%d]\n",
              Forest->Tree[i].Rooms[j].Room->Location.x,
              Forest->Tree[i].Rooms[j].Room->Location.y,
              Forest->Tree[i].Rooms[j].Room->Location.z,
              Forest->Tree[i].Rooms[j].Room->Name,
              Forest->Tree[i].Rooms[j].Room->Number,
              zone_name(Zones, Forest->Tree[i].Rooms[j].Room->Zone),
              Forest->Tree[i].Rooms[j].Room->Zone);
    fprintf(ofp, "\f\n");
    for(j= 0; j< Forest->Tree[i].MapSize.y; j++) {
      free(Map[j]);
      free(PMap[j]);
    }
    free(Map);
    free(PMap);
  }
  if(outfile && *outfile)
    fclose(ofp);
  if(!Quiet)
    fprintf(stderr, "done.\n");
}

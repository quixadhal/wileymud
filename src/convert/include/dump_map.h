#ifndef _DUMP_MAP_H
#define _DUMP_MAP_H

#define EXIT_BIT_N	1
#define EXIT_BIT_E	2
#define EXIT_BIT_S	4
#define EXIT_BIT_W	8
#define EXIT_BIT_U	16
#define EXIT_BIT_D	32

typedef struct ppmcolour {
    int                                     Red;
    int                                     Green;
    int                                     Blue;
} ppmcolour;

typedef struct mapnode {
    room                                   *Room;	       /* pointer to a room */
    int                                     Exits;	       /* exit bitmask (NESW only) */
} mapnode;

typedef struct coord_group {
    coordinate                              Min;
    coordinate                              Current;
    coordinate                              Max;
} coord_group;

typedef struct tree {
    int                                     Count;
    mapnode                                *Rooms;
    coord_group                             Coords;
    coordinate                              Offset;
    coordinate                              MapSize;
} tree;

typedef struct forest {
    int                                     Count;
    tree                                   *Tree;
} forest;

typedef struct maptemplate {
    char                                   *Room[7];
} maptemplate;

#ifndef _DUMP_MAP_C
extern const char                      *terrain;
extern const int                        pgmterrains[];
extern const int                       *pgmterrain;
extern const ppmcolour                  ppmterrains[];
extern const ppmcolour                 *ppmterrain;
extern const int                        exit_dir_to_bit[];
extern const maptemplate                empty_shape;
extern const maptemplate                exit_shapes[];
#endif

forest                                 *build_map(zones *Zones, rooms *Rooms);
void                                    make_2d_map(forest *Forest, zones *Zones, char *outfile,
						    char *pgmpattern);
void                                    colour_3d(tree *Tree, coord_group *SoFar, int Colour,
						  zones *Zones, rooms *Rooms, int RoomIndex,
						  int VZone);
int                                     qcmp_x(const void *a, const void *b);
int                                     qcmp_y(const void *a, const void *b);
int                                     qcmp_z(const void *a, const void *b);

/*
int qcmp_x(const mapnode *a, const mapnode *b);
int qcmp_y(const mapnode *a, const mapnode *b);
int qcmp_z(const mapnode *a, const mapnode *b);
*/
#endif

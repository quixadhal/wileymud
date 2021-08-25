#ifndef _DUMP_NEWMAP_H
#define _DUMP_NEWMAP_H

/*
typedef struct coordinate {
    int                                     x;
    int                                     y;
    int                                     z;
} coordinate;
*/

typedef struct s_Tree
{
    int Count;
    room **Room;
    int Start;
    int VNum;
    int Zone;
    int Colour;
    coordinate Origin;
    coordinate Size;
} t_Tree;

typedef struct s_Forest
{
    int Count;
    t_Tree **Tree;
} t_Forest;

void dump_as_newmap(zones *Zones, rooms *Rooms, objects *Objects, mobs *Mobs, shops *Shops);
int FindNextRoom(rooms *Rooms, int ZoneNumber);
void _BuildTree(t_Tree *CurrentTree, int CurrentRoomIndex, coordinate CurrentLocation, zones *Zones, rooms *Rooms);
t_Tree *BuildTree(int CurrentColour, int CurrentRoomIndex, zones *Zones, rooms *Rooms);
void BuildForest(zones *Zones, rooms *Rooms);

#endif

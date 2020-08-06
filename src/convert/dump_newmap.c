#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "include/structs.h"
#include "include/main.h"
#include "include/utils.h"

#include "include/parse_wiley.h"

#define _NEWMAP_C
#include "include/dump_newmap.h"

#define START_ROOM  3001

t_Forest  *Forest = NULL;
int     RoomsLeft = 0;
int     DidStartRoom = 0;

void dump_as_newmap(zones *Zones, rooms *Rooms, objects *Objects, mobs *Mobs, shops *Shops)
{
    RoomsLeft = Rooms->Count;

    if (!Quiet) {
	fprintf(stderr, "Generating NEW Map Data...");
	fflush(stderr);
    }

    Forest = (t_Forest *)calloc(1, sizeof(t_Forest));
    Forest->Count = 0;
    Forest->Tree = NULL;

    BuildForest(Zones, Rooms);

    if(Forest->Count > 0) {
        int RoomCount = 0;
        int *TreeZoneCount = NULL;
        int *TreeZoneTrees = NULL;

        fprintf(stderr, "Found %d trees!\n", Forest->Count);
        for(int i = 0; i < Forest->Count; i++) {
            RoomCount += Forest->Tree[i]->Count;
        }
        fprintf(stderr, "Found %d rooms out of %d expected\n", RoomCount, Rooms->Count);

        TreeZoneCount = (int *)calloc(Zones->Count, sizeof(int));
        TreeZoneTrees = (int *)calloc(Zones->Count, sizeof(int));
        for(int i = 0; i < Zones->Count; i++) {
            TreeZoneCount[i] = 0;
            TreeZoneTrees[i] = 0;
        }
        printf("[\n");
        for(int i = 0; i < Zones->Count; i++) {
            for(int j = 0; j < Forest->Count; j++) {
                if(Forest->Tree[j]->Zone == Zones->Zone[i].Number) {
                    TreeZoneCount[i] += Forest->Tree[j]->Count;
                    TreeZoneTrees[i] ++;
                }
            }

            printf("    {\n");
            printf("        \"Zone\" : {\n");
            printf("            \"VNum\" : %d,\n", Zones->Zone[i].Number);
            for(int j = 0; j < Forest->Count; j++) {
                printf("            \"Tree\" : {\n");
                printf("                \"Number\" : %d,\n", j);
                printf("                \"VNum\" : %d,\n", Forest->Tree[j]->VNum);
                printf("                \"Zone\" : %d,\n", Forest->Tree[j]->Zone);
                printf("                \"Rooms\" : [\n");
                for(int k = 0; k < Forest->Tree[j]->Count; k++) {
                    printf("%d, ", Forest->Tree[j]->Room[k]->Number);
                    if(k%10 == 0 && k > 0) {
                        printf("\n");
                    }
                }
                printf("\n                ]\n");
                printf("            },\n");
            }
            printf("        }\n");
            printf("    },\n");

            fprintf(stderr, "Zone %d has %d rooms across %d trees\n",
                    Zones->Zone[i].Number,
                    TreeZoneCount[i], TreeZoneTrees[i]);
            fflush(stderr);
        }
    } else {
        fprintf(stderr, "No trees found\n");
    }
    fflush(stderr);
}

int FindNextRoom(rooms *Rooms, int ZoneNumber)
{
    int Result = DidStartRoom ? -1 : 0;

    for (int i = 0; i < Rooms->Count; i++) {
        if(Rooms->Room[i].Colour == 0) {
            if (DidStartRoom) {
                if (ZoneNumber == -1 || Rooms->Room[i].Zone == ZoneNumber) {
                    Result = i;
                }
            } else if (Rooms->Room[i].Number == START_ROOM) {
                Result = i;
            }
        }
    }

    DidStartRoom = 1;
    return Result;
}

void _BuildTree(t_Tree *CurrentTree, int CurrentRoomIndex, coordinate CurrentLocation, zones *Zones, rooms *Rooms)
{
    room        *CurrentRoom = NULL;
    int         CurrentZone = -1;

    CurrentRoom = &Rooms->Room[CurrentRoomIndex];
    CurrentZone = CurrentRoom->Zone;

    if( CurrentRoom->Colour != 0 ) {
        // This room has alrady been visited
        return;
    }
    CurrentRoom->Colour = CurrentTree->Colour;
    CurrentRoom->Location = CurrentLocation;
    RoomsLeft--;

    if (CurrentTree->Count == 0) {
        CurrentTree->Count = 1;
        CurrentTree->Room = (room **)calloc(1, sizeof(room *));
        CurrentTree->Room[0] = CurrentRoom;
    } else {
        CurrentTree->Count++;
	CurrentTree->Room = (room **)realloc(CurrentTree->Room, CurrentTree->Count * sizeof(room *));
        CurrentTree->Room[CurrentTree->Count-1] = CurrentRoom;
    }

    for (int e = 0; e < CurrentRoom->ExitCount; e++) {
        if (CurrentRoom->Exit[e].Error == EXIT_OK) {
            int         TargetRoomIndex = CurrentRoom->Exit[e].RoomIndex;
            coordinate  TargetLocation = CurrentLocation;

            if (Rooms->Room[TargetRoomIndex].Zone == CurrentZone) {
                switch(CurrentRoom->Exit[e].Direction) {
                    case 0:
                        // North
                        TargetLocation.y--;
                        break;
                    case 1:
                        // East
                        TargetLocation.x++;
                        break;
                    case 2:
                        // South
                        TargetLocation.y++;
                        break;
                    case 3:
                        // West
                        TargetLocation.x--;
                        break;
                    case 4:
                        // Up
                        TargetLocation.z++;
                        break;
                    case 5:
                        // Down
                        TargetLocation.z--;
                        break;
                }
                _BuildTree(CurrentTree, TargetRoomIndex, TargetLocation, Zones, Rooms);
            }
        }
    }
}

t_Tree *BuildTree(int CurrentColour, int CurrentRoomIndex, zones *Zones, rooms *Rooms)
{
    t_Tree    *NewTree = NULL;

    NewTree = (t_Tree *)calloc(1, sizeof(t_Tree));
    NewTree->Count = 0;

    NewTree->Start = CurrentRoomIndex;
    NewTree->Colour = CurrentColour;
    NewTree->VNum = Rooms->Room[CurrentRoomIndex].Number;
    NewTree->Zone = Rooms->Room[CurrentRoomIndex].Zone;
    NewTree->Origin.x = 0;
    NewTree->Origin.y = 0;
    NewTree->Origin.z = 0;
    NewTree->Size.x = 1;
    NewTree->Size.y = 1;
    NewTree->Size.z = 1;

    _BuildTree(NewTree, CurrentRoomIndex, NewTree->Origin, Zones, Rooms);

    return NewTree;
}

void BuildForest(zones *Zones, rooms *Rooms)
{
    // Using global Forest, and RoomsLeft
    int             CurrentColour = 1;
    int             CurrentRoomIndex = -1;
    int             CurrentZone = -1;
    int             *ZonesLeft = NULL;

    ZonesLeft = (int *)calloc(Zones->Count, sizeof(int));
    for(int i = 0; i < Zones->Count; i++) {
        ZonesLeft[i] = Zones->Zone[i].Number;
    }

    while (RoomsLeft > 0) {
        CurrentRoomIndex = FindNextRoom(Rooms, CurrentZone);
        if (CurrentRoomIndex < 0) {
            if(CurrentZone != -1) {
                // We finished THIS zone, but there might be others?
                for(int i = 0; i < Zones->Count; i++) {
                    if (Zones->Zone[i].Number == CurrentZone) {
                        ZonesLeft[i] = -1;
                        break;
                    }
                }
                for(int i = 0; i < Zones->Count; i++) {
                    if (ZonesLeft[i] != -1) {
                        CurrentZone = ZonesLeft[i];
                        CurrentRoomIndex = FindNextRoom(Rooms, CurrentZone);
                        if (CurrentRoomIndex != -1) {
                            break;
                        } else {
                            // Somehow this zone never got marked as done?
                            ZonesLeft[i] = -1;
                        }
                    }
                }
            }
            if(CurrentRoomIndex == -1) {
                fprintf(stderr, "%d Rooms Left, but no more rooms can be found?\n", RoomsLeft);
                fflush(stderr);
                break;
            }
        }
        CurrentZone = Rooms->Room[CurrentRoomIndex].Zone;

        // Grow the forest
        if (Forest->Count == 0) {
            Forest->Count = CurrentColour = 1;
            Forest->Tree = (t_Tree **)calloc(Forest->Count, sizeof(t_Tree *));
            Forest->Tree[Forest->Count-1] = NULL;
        } else {
            CurrentColour++;
            Forest->Count = CurrentColour;
            Forest->Tree = (t_Tree **)realloc(Forest->Tree, Forest->Count * sizeof(t_Tree *));
            Forest->Tree[Forest->Count-1] = NULL;
        }

        Forest->Tree[Forest->Count-1] = BuildTree(Forest->Count, CurrentRoomIndex, Zones, Rooms);
        //fprintf(stderr, "Tree %d has %d rooms!\n", Forest->Count, Forest->Tree[Forest->Count-1]->Count);
        //fflush(stderr);
    }
}


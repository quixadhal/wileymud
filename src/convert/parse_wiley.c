#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>

#include "include/bug.h"
#include "include/structs.h"
#include "include/main.h"
#include "include/utils.h"
#include "include/make_index.h"

#define _PARSE_WILEY_C
#include "include/parse_wiley.h"

const int SectorCosts[] = { 0, 1, 2, 2, 3, 4, 6, 8, 10, 2, 20 };
const int RevDir[] = { 2, 3, 0, 1, 5, 4 };

/*
 * This routine loads a zone file (in WileyMUD DikuMUD format) and returns a
 * structure containing all the information it contained.
 */
zones *load_zones(char *infile) {
  FILE *ifp;
  vnum_index *ZoneIndex;
  zones *Zones;
  char *tmp;
  register int i, j;
  long Line, Pos;

  if(!infile || !*infile) return NULL;
  if(!(ZoneIndex= make_index(infile, NULL))) return NULL;
  Zones= (zones *)get_mem(1, sizeof(zones));
  bzero(Zones, sizeof(zones));
  Zones->Count= ZoneIndex->Count;
  Zones->Zone= (zone *)get_mem(Zones->Count, sizeof(zone));
  bzero(Zones->Zone, Zones->Count* sizeof(zone));
  ifp= open_file(infile, "r");

  if(!Quiet) {
    fprintf(stderr, "Parsing Zone file...");
    fflush(stderr);
  }

  for(i= 0; i< Zones->Count; i++) {
    Line= ZoneIndex->VNum[i].Line;
    Pos= ZoneIndex->VNum[i].Pos;
    Zones->Zone[i].Number= ZoneIndex->VNum[i].Number;
    if(fseek(ifp, Pos, 0) < 0)
      fatal("Cannot load Zone #%d from %s!",
          Zones->Zone[i].Number, infile);
    if(!(tmp= get_line(ifp, &Line, &Pos, 1)))
      fatal("Cannot get vnum for Zone #%d from %s!",
          Zones->Zone[i].Number, infile);
    if(!(tmp= get_tilde_string(ifp, &Line, &Pos)))
      fatal("Missing zone name at line %d of %s.",
          Line, infile);
    Zones->Zone[i].Name= my_strdup(tmp);
    if(!(tmp= get_line(ifp, &Line, &Pos, 1)))
      fatal("Missing flag line in %s at %d.",infile,Line);
    if(sscanf(tmp, " %d %d %d ", &(Zones->Zone[i].Top), &(Zones->Zone[i].Time),
           &(Zones->Zone[i].Mode))!= 3)
      fatal("Corrupt flag line in %s at %d.",infile,Line);

    Zones->Zone[i].Cmds= (zone_cmds *)get_mem(1, sizeof(zone_cmds));
    bzero(Zones->Zone[i].Cmds, sizeof(zone_cmds));
    for(j= Zones->Zone[i].Count= 0; (tmp= get_line(ifp, &Line, &Pos, 1));) {
      if(*tmp == ZONE_CMD_END) {
        /* if(Zones->Zone[i].Count > 0) Zones->Zone[i].Count--; */
        break;
      } else if((*tmp == ZONE_CMD_MOBILE) || (*tmp == ZONE_CMD_OBJECT) ||
                (*tmp == ZONE_CMD_EQUIP) || (*tmp == ZONE_CMD_PUT) ||
                (*tmp == ZONE_CMD_DOOR)) {
        if(sscanf(tmp, "%c %d %d %d %d ",
                  &(Zones->Zone[i].Cmds[j].Command),
                  &(Zones->Zone[i].Cmds[j].IfFlag),
                  &(Zones->Zone[i].Cmds[j].Arg[0]),
                  &(Zones->Zone[i].Cmds[j].Arg[1]),
                  &(Zones->Zone[i].Cmds[j].Arg[2])) != 5) {
          if(Debug)
            bug("Bad args to Zone command, %s at %d.",
                infile,Line);
          continue;
        }
        if(Verbose)
          fprintf(stderr, "Zone [#%d] Command %d:  %c %d %d %d %d\n",
                  Zones->Zone[i].Number,
                  j, Zones->Zone[i].Cmds[j].Command,
                  Zones->Zone[i].Cmds[j].IfFlag,
                  Zones->Zone[i].Cmds[j].Arg[0],
                  Zones->Zone[i].Cmds[j].Arg[1],
                  Zones->Zone[i].Cmds[j].Arg[2] );
        else if(!Quiet)
          spin(stderr);
      } else if((*tmp == ZONE_CMD_GIVE) || (*tmp == ZONE_CMD_REMOVE) ||
                (*tmp == ZONE_CMD_LEAD) || (*tmp == ZONE_CMD_HATE)) {
        if(sscanf(tmp, "%c %d %d %d ",
                  &(Zones->Zone[i].Cmds[j].Command),
                  &(Zones->Zone[i].Cmds[j].IfFlag),
                  &(Zones->Zone[i].Cmds[j].Arg[0]),
                  &(Zones->Zone[i].Cmds[j].Arg[1])) != 4) {
          if(Debug)
            bug("Bad args to Zone command, %s at %d.",
                infile,Line);
          continue;
        }
        if(Verbose)
          fprintf(stderr, "Zone [#%d] Command %d:  %c %d %d %d\n",
                  Zones->Zone[i].Number,
                  j, Zones->Zone[i].Cmds[j].Command,
                  Zones->Zone[i].Cmds[j].IfFlag,
                  Zones->Zone[i].Cmds[j].Arg[0],
                  Zones->Zone[i].Cmds[j].Arg[1] );
        else if(!Quiet)
          spin(stderr);
      } else {
        if(Debug)
          bug("Unrecognized Zone command ignored, %s at %d.",
              infile,Line);
        continue;
      }
      j= ++Zones->Zone[i].Count;
      Zones->Zone[i].Cmds=
        (zone_cmds *)get_more_mem((char *)Zones->Zone[i].Cmds,
                                  j+1, sizeof(zone_cmds));
      bzero(&(Zones->Zone[i].Cmds[j]), sizeof(zone_cmds));
    }
  }
  fclose(ifp);
  if(!Quiet)
    fprintf(stderr, "done.\n");
  return Zones;
}

char *zone_reset_name(int Reset) {
  switch(Reset) {
    case ZONE_RESET_NEVER:	return "Never";
    case ZONE_RESET_PC:		return "if No Players are present";
    case ZONE_RESET_ALWAYS:	return "Always";
    default:			return "unknown";
  }
}

int remap_zone_vnum(zones *Zones, int VNum) {
  register int i;

  for(i= 0; i< Zones->Count; i++)
    if(Zones->Zone[i].Number == VNum) return i;
  return -1;
}

char *zone_name(zones *Zones, int VNum) {
  int i;

  if((i= remap_zone_vnum(Zones, VNum))!= -1)
    return Zones->Zone[i].Name;
  else return "a non-existant zone";
}

/*
 * Here begins the Great Room Parser.... bowworshipgrovel
 */
char *exit_name(int Direction) {
  switch(Direction) {
    case EXIT_NORTH:	return("North");
    case EXIT_EAST:	return("East");
    case EXIT_SOUTH:	return("South");
    case EXIT_WEST:	return("West");
    case EXIT_UP:	return("Up");
    case EXIT_DOWN:	return("Down");
    default: return("unknown");
  }
}

char *exit_name_lower(int Direction) {
  switch(Direction) {
    case EXIT_NORTH:	return("north");
    case EXIT_EAST:	return("east");
    case EXIT_SOUTH:	return("south");
    case EXIT_WEST:	return("west");
    case EXIT_UP:	return("up");
    case EXIT_DOWN:	return("down");
    default: return("unknown");
  }
}

int remap_room_vnum(rooms *Rooms, int VNum) {
  register int i;

  for(i= 0; i< Rooms->Count; i++)
    if(Rooms->Room[i].Number == VNum) return i;
  return -1;
}

char *room_name(rooms *Rooms, int VNum) {
  int i;

  if((i= remap_room_vnum(Rooms, VNum))!= -1)
    return Rooms->Room[i].Name;
  else return "a non-existant room";
}

void fix_exit_vnums(rooms *Rooms) {
  register int i, j;

  if(!Quiet) {
    fprintf(stderr, "Fixing exit vnums to physical mappings...");
    fflush(stderr);
  }
  for(i= 0; i< Rooms->Count; i++) {
    for(j= 0; j< Rooms->Room[i].ExitCount; j++) {
      if(!Quiet)
        spin(stderr);
      if(Rooms->Room[i].Exit[j].Error == EXIT_OK) {
        if(Rooms->Room[i].Exit[j].Room < 0) {
          Rooms->Room[i].Exit[j].Error= EXIT_DESCRIPTION_ONLY;
          Rooms->Room[i].Exit[j].RoomIndex= EXIT_INVALID;
          if(Debug)
            bug("DESCRIPTION-ONLY exit found as %s exit of Room \"%s\"[#%d].",
                exit_name(Rooms->Room[i].Exit[j].Direction),
                room_name(Rooms, Rooms->Room[i].Number),
                Rooms->Room[i].Number);
        } else Rooms->Room[i].Exit[j].RoomIndex=
          remap_room_vnum(Rooms, Rooms->Room[i].Exit[j].Room);
      }
    }
  }
  if(!Quiet)
    fprintf(stderr, "done.\n");
}

void check_duplicate_rooms(rooms *Rooms) {
  register int i, tmp;

  if(!Quiet) {
    fprintf(stderr, "Checking for duplicate room vnums...");
    fflush(stderr);
  }
  for(i= 0; i< Rooms->Count; i++) {
    if(!Quiet)
      spin(stderr);
    tmp= Rooms->Room[i].Number;
    Rooms->Room[i].Number = -1;
    if(remap_room_vnum(Rooms, tmp) >= 0)
      if(Debug)
        bug("ROOM [#%d] is DUPLICATED!!!", tmp);
    Rooms->Room[i].Number = tmp;
  }
  if(!Quiet)
    fprintf(stderr, "done.\n");
}

void verify_exits(rooms *Rooms) {
  register int i,j,k,real,back;

  if(!Quiet) {
    fprintf(stderr, "Verifying exit consistancy...");
    fflush(stderr);
  }
  for(i= 0; i< Rooms->Count; i++) {
    if(!Quiet)
      spin(stderr);
    if(Rooms->Room[i].ExitCount < 1) {
      if(Rooms->Room[i].Sector != SECT_TELEPORT) {
        if(Debug)
          bug("NO-EXITS in Room \"%s\"[#%d]!",
              room_name(Rooms, Rooms->Room[i].Number),
              Rooms->Room[i].Number);
      } else {
        if(Rooms->Room[i].TeleportTime < 0) {
          if(Debug)
            bug("TELEPORT TIME INVALID for Room \"%s\"[#%d]!",
                room_name(Rooms, Rooms->Room[i].Number),
                Rooms->Room[i].Number);
        } else {
          if(remap_room_vnum(Rooms, Rooms->Room[i].TeleportTo) < 0) {
            if(Debug)
              bug("NO-EXITS in Room \"%s\"[#%d]!  INVALID TELEPORT TARGET.",
                  room_name(Rooms, Rooms->Room[i].Number),
                  Rooms->Room[i].Number);
          } else {
            if(Debug)
              bug("TELEPORT-ONLY exit in Room \"%s\"[#%d].",
                  room_name(Rooms, Rooms->Room[i].Number),
                  Rooms->Room[i].Number);
          }
        }
      }
    } else for(j= 0; j< Rooms->Room[i].ExitCount; j++) {
      if(Rooms->Room[i].Exit[j].Error /*Direction*/ != EXIT_OK) {
        if(Debug)
          bug("Room \"%s\"[#%d] has an INVALID exit! REMOVING!",
              room_name(Rooms, Rooms->Room[i].Number),
              Rooms->Room[i].Number);
      } else {
        if((real= Rooms->Room[i].Exit[j].RoomIndex) < 0) {
          Rooms->Room[i].Exit[j].Error= EXIT_NO_TARGET;
          if(Debug)
            bug("INVALID-TARGET Room #%d does not exist!  REMOVING %s exit from Room \"%s\"[#%d].",
                Rooms->Room[i].Exit[j].Room,
                exit_name(Rooms->Room[i].Exit[j].Direction),
                room_name(Rooms, Rooms->Room[i].Number),
                Rooms->Room[i].Number);
          /* Rooms->Room[i].Exit[j].Direction= EXIT_INVALID; */
        } else {
          if(Rooms->Room[real].ExitCount < 1) {
            if(Debug)
              bug("NO-EXITS in Target Room \"%s\"[#%d]!",
                  room_name(Rooms, Rooms->Room[real].Number),
                  Rooms->Room[real].Number);
          } else {
            back= -1;
            for(k= 0; k< Rooms->Room[real].ExitCount; k++) {
              if(Rooms->Room[real].Exit[k].Direction ==
                 RevDir[Rooms->Room[i].Exit[j].Direction])
                back= k;
            }
            if(back < 0) {
              Rooms->Room[i].Exit[j].Error= EXIT_ONE_WAY;
              if(Debug)
                bug("ONE-WAY %s exit in Room \"%s\"[#%d]!  No matching %s exit in Target Room \"%s\"[#%d].",
                    exit_name(Rooms->Room[i].Exit[j].Direction),
                    room_name(Rooms, Rooms->Room[i].Number),
                    Rooms->Room[i].Number,
                    exit_name(RevDir[Rooms->Room[i].Exit[j].Direction]),
                    room_name(Rooms, Rooms->Room[real].Number),
                    Rooms->Room[real].Number);
            } else {
              if(Rooms->Room[real].Exit[back].Room != Rooms->Room[i].Number) {
                Rooms->Room[i].Exit[j].Error= EXIT_NON_EUCLIDEAN;
                if(Debug)
                  bug("NON-EUCLIDEAN mapping:  Room \"%s\"[#%d] %s -> Room \"%s\"[#%d] != Room \"%s\"[#%d] %s -> Room \"%s\"[#%d]!",
                      room_name(Rooms, Rooms->Room[i].Number),
                      Rooms->Room[i].Number,
                      exit_name(Rooms->Room[i].Exit[j].Direction),
                      room_name(Rooms, Rooms->Room[i].Exit[j].Room),
                      Rooms->Room[i].Exit[j].Room,
                      room_name(Rooms, Rooms->Room[real].Number),
                      Rooms->Room[real].Number,
                      exit_name(Rooms->Room[real].Exit[back].Direction),
                      room_name(Rooms, Rooms->Room[real].Exit[back].Room),
                      Rooms->Room[real].Exit[back].Room);
              }
            }
          }
        }
      }
    }
  }
  if(!Quiet)
    fprintf(stderr, "done.\n");
}

void fix_zone_ids(zones *Zones, rooms *Rooms) {
  register int i, j, CorrectZone;

  if(!Quiet) {
    fprintf(stderr, "Fixing incorrect Zone id tags...");
    fflush(stderr);
  }
  for(i= 0; i< Rooms->Count; i++) {
    if(!Quiet)
      spin(stderr);
    CorrectZone= -1;
    for(j= 0; j< Zones->Count; j++)
      if(Rooms->Room[i].Number <= Zones->Zone[j].Top)
        if(Rooms->Room[i].Number >= ((j>0)?Zones->Zone[j-1].Top+1:0)) {
          CorrectZone= Zones->Zone[j].Number;
          break;
        }
    if(CorrectZone == -1) {
      if(Verbose)
        bug("Holy BatTurds!  I have NO IDEA what zone this room [#%d] "
            "belongs to!\nPutting it in Zone %d...\n",
            Rooms->Room[i].Number, Zones->Zone[0].Number);
      Rooms->Room[i].Zone= Zones->Zone[0].Number;
    } else if(Rooms->Room[i].Zone != CorrectZone) {
      if(Debug)
        bug("Incorrect Zone ID of [#%d] for room \"%s\" [#%d],\n"
            "Remapping to Zone [#%d].\n",
            Rooms->Room[i].Zone, Rooms->Room[i].Name,
            Rooms->Room[i].Number, CorrectZone);
      Rooms->Room[i].Zone= CorrectZone;
    }
  }
  if(!Quiet)
    fprintf(stderr, "done.\n");
}

void check_room_zone_mismatch(zones *Zones, rooms *Rooms) {
  register int i, CorrectZone;

  if(!Quiet) {
    fprintf(stderr, "Verifying proper Zone id tags...");
    fflush(stderr);
  }
  for(i= 0; i< Rooms->Count; i++) {
    if(!Quiet)
      spin(stderr);
    CorrectZone= Rooms->Room[i].Number / 100;
    if(Rooms->Room[i].Zone != CorrectZone) {
      bug("Improper Zone ID of [#%d] for room \"%s\" [#%d],\n"
          "Should belong to Zone [#%d].\n",
          Rooms->Room[i].Zone, Rooms->Room[i].Name,
          Rooms->Room[i].Number, CorrectZone);
      /* Rooms->Room[i].Zone= CorrectZone; */
    }
  }
  if(!Quiet)
    fprintf(stderr, "done.\n");
}

/*
 * This routine loads the world file (in WileyMUD DikuMUD format) and returns a
 * structure containing all the information it contained.
 */
rooms *load_rooms(char *infile, zones *Zones) {
  FILE *ifp;
  vnum_index *RoomIndex;
  rooms *Rooms;
  char *tmp; /* , tmp2[MAX_LINE_LEN], *tmp3; */
  register int i, j, e;
  long Line, Pos;

  if(!infile || !*infile) return NULL;
  if(!(RoomIndex= make_index(infile, NULL))) return NULL;
  Rooms= (rooms *)get_mem(1, sizeof(rooms));
  bzero(Rooms, sizeof(rooms));
  Rooms->Count= RoomIndex->Count;
  Rooms->Room= (room *)get_mem(Rooms->Count, sizeof(room));
  bzero(Rooms->Room, Rooms->Count* sizeof(room));
  ifp= open_file(infile, "r");

  if(Verbose)
    fprintf(stderr, "Parsing World file...\n");
  else if(!Quiet) {
    fprintf(stderr, "Parsing World file...");
    fflush(stderr);
  }

  for(i= 0; i< Rooms->Count; i++) {
    Line= RoomIndex->VNum[i].Line;
    Pos= RoomIndex->VNum[i].Pos;
    Rooms->Room[i].Number= RoomIndex->VNum[i].Number;
    if(fseek(ifp, Pos, 0) < 0)
      fatal("Cannot load Room $%d from %s!",
          Rooms->Room[i].Number, infile);
    if(!(tmp= get_line(ifp, &Line, &Pos, 1)))
      fatal("Cannot get vnum for Room #%d from %s!",
          Rooms->Room[i].Number, infile);
    if((!(tmp= get_tilde_string(ifp, &Line, &Pos)))||
       (!(Rooms->Room[i].Name= my_strdup(tmp))))
      fatal("Cannot get name for Room #%d, %s at %d.",
          Rooms->Room[i].Number, infile, Line);

    if(Verbose)
      fprintf(stderr, "  (%d) Parsing \"%s\"[#%d]...\n", i,
              Rooms->Room[i].Name, Rooms->Room[i].Number);
    else if(!Quiet)
      spin(stderr);

    if((!(tmp= get_tilde_string(ifp, &Line, &Pos)))||
       (!(Rooms->Room[i].Description= my_strdup(tmp))))
      fatal("Cannot get description for Room #%d, %s at %d.",
          Rooms->Room[i].Number, infile, Line);
    if((!(tmp= get_line(ifp, &Line, &Pos, 1)))||
       (sscanf(tmp, " %d %d %d ", &(Rooms->Room[i].Zone),
               &(Rooms->Room[i].Flags), &(Rooms->Room[i].Sector)) != 3)) {
      if(Debug)
        bug("Bug in MAIN flags for Room #%d, %s at %d!",
            Rooms->Room[i].Number, infile, Line);
/* If you get an error here, it means the room designer really FUCKED UP!
 * Nevertheless, we can fudge it provided that the person didn't add the
 * SOUND definitions and forget the sector types... if this is the case,
 * An error will happen down below which will, almost certainly be fatal
 * and non-recoverable.
 * As it stands, we will TRY to jump inside the for() loop that scans for
 * doors and extra descriptions (and the terminating 'S') in the hopes that
 * that line is what we actually read in....
 */
      Rooms->Room[i].Flags= 0;
      Rooms->Room[i].Sector= SECT_FIELD;
      Rooms->Room[i].MoveCost= SectorCosts[Rooms->Room[i].Sector];
      Rooms->Room[i].Exit= (EXIT *)get_mem(1, sizeof(EXIT));
      bzero(Rooms->Room[i].Exit, sizeof(EXIT));
      Rooms->Room[i].Exit[0].Direction = EXIT_INVALID;
      Rooms->Room[i].Exit[0].Room = EXIT_INVALID;
      Rooms->Room[i].Exit[0].RoomIndex = EXIT_INVALID;
      Rooms->Room[i].Exit[0].Error = EXIT_UNKNOWN;
      Rooms->Room[i].Extra= (extra *)get_mem(1, sizeof(extra));
      bzero(Rooms->Room[i].Extra, sizeof(extra));
      j= e= Rooms->Room[i].ExitCount= Rooms->Room[i].ExtraCount= 0;
      goto HolyShit;
    }
    if(Rooms->Room[i].Sector == SECT_TELEPORT) {
      if(sscanf(tmp, " %d %d %d %d %d %d %d ", &(Rooms->Room[i].Zone),
                &(Rooms->Room[i].Flags), &(Rooms->Room[i].Sector),
                &(Rooms->Room[i].TeleportTime), &(Rooms->Room[i].TeleportTo),
                &(Rooms->Room[i].TeleportLook),
                &(Rooms->Room[i].TeleportSector)) != 7) {
        if(Debug)
          bug("Bad teleport flags for Room #%d, %s at %d.",
              Rooms->Room[i].Number, infile, Line);
/* If teleport flags are munged, we need to make it default back to a plain
 * generic sector type, so that it can still be loaded...
 */
        Rooms->Room[i].Sector= SECT_FIELD;
      }
    } else if(Rooms->Room[i].Sector == SECT_WATER_NOSWIM) {
      if(sscanf(tmp, " %d %d %d %d %d ", &(Rooms->Room[i].Zone),
                &(Rooms->Room[i].Flags), &(Rooms->Room[i].Sector),
                &(Rooms->Room[i].RiverSpeed),
                &(Rooms->Room[i].RiverDirection)) != 5) {
        Rooms->Room[i].RiverSpeed= 0;
        Rooms->Room[i].RiverDirection= -1;
        if(Debug)
          bug("Missing river flags for Room #%d, %s at %d.",
              Rooms->Room[i].Number, infile, Line);
      }
    }
    if(Rooms->Room[i].Flags & ROOM_DEATH)
      if(Debug)
        bug("Room #%d is a DEATH ROOM!", Rooms->Room[i].Number);
    if(Rooms->Room[i].Flags & ROOM_SOUND) {
      Rooms->Room[i].SoundCount= 2;
      for(j= 0; j< Rooms->Room[i].SoundCount; j++)
        if((!(tmp= get_tilde_string(ifp, &Line, &Pos)))||
           (!(Rooms->Room[i].SoundText[j]= my_strdup(tmp))))
          fatal("Missing sound %d for Room #%d, %s at %d.",
              j, Rooms->Room[i].Number, infile, Line);
/* additional checks should be done, but I haven't the time... let's hope the
 * wileymud people did their jobs right! <grin>
 */
    }
    Rooms->Room[i].Exit= (EXIT *)get_mem(1, sizeof(EXIT));
    bzero(Rooms->Room[i].Exit, sizeof(EXIT));
    Rooms->Room[i].Exit[0].Direction= EXIT_INVALID;
    Rooms->Room[i].Exit[0].Room = EXIT_INVALID;
    Rooms->Room[i].Exit[0].RoomIndex = EXIT_INVALID;
    Rooms->Room[i].Exit[0].Error = EXIT_UNKNOWN;
    Rooms->Room[i].Extra= (extra *)get_mem(1, sizeof(extra));
    bzero(Rooms->Room[i].Extra, sizeof(extra));
    Rooms->Room[i].MoveCost= SectorCosts[Rooms->Room[i].Sector];
    for(j= e= Rooms->Room[i].ExitCount= Rooms->Room[i].ExtraCount= 0;
        (tmp= get_line(ifp, &Line, &Pos, 1));) {
HolyShit:
      if(*tmp == 'S') break;
      else if(*tmp == 'D') {
        if(sscanf(tmp+1, "%d", &(Rooms->Room[i].Exit[j].Direction)) != 1)
          fatal("Corrupt Exit in Room #%d, %s at %d.",
              Rooms->Room[i].Number, infile, Line);
        if((!(tmp= get_tilde_string(ifp, &Line, &Pos)))||
           (!(Rooms->Room[i].Exit[j].Description= my_strdup(tmp))))
          fatal("No exit description for Room #%d, %s at %d.",
              Rooms->Room[i].Number, infile, Line);
        if(!(tmp= get_tilde_string(ifp, &Line, &Pos)))
          fatal("No exit keywords for Room #%d, %s at %d.",
              Rooms->Room[i].Number, infile, Line);
        Rooms->Room[i].Exit[j].Keyword= make_keyword_list(tmp);
        if((!(tmp= get_line(ifp, &Line, &Pos, 1))) ||
           (sscanf(tmp, " %d %d %d ", &(Rooms->Room[i].Exit[j].Type),
                   &(Rooms->Room[i].Exit[j].KeyNumber),
                   &(Rooms->Room[i].Exit[j].Room)) != 3)) {
          if(Debug)
            bug("Bad exit type line in Room #%d, %s at %d.",
                Rooms->Room[i].Number, infile, Line);
/* This most often happens when someone was planning to add an area, and
 * got one half of the links done, but now the new room numbers...
 * since our file was taken in mid-hack, right before it got zapped by
 * the Mad Cyric the Destroyer... this is not surprising.
 */
          Rooms->Room[i].Exit[j].Type= EXIT_OPEN;
          Rooms->Room[i].Exit[j].KeyNumber= -1; /* no key */
          Rooms->Room[i].Exit[j].Room= EXIT_INVALID;
          Rooms->Room[i].Exit[j].RoomIndex= EXIT_INVALID;
          Rooms->Room[i].Exit[j].Error= EXIT_NO_TARGET;
        } else {
          Rooms->Room[i].Exit[j].Error= EXIT_OK;
        }
        if(Rooms->Room[i].Exit[j].Error != EXIT_UNKNOWN) {
          j= ++Rooms->Room[i].ExitCount;
          Rooms->Room[i].Exit= (EXIT *)
            get_more_mem((char *)Rooms->Room[i].Exit, j+1, sizeof(EXIT));
          bzero(&(Rooms->Room[i].Exit[j]), sizeof(EXIT));
        }
      } else if(*tmp == 'E') {
        if(!(tmp= get_tilde_string(ifp, &Line, &Pos)))
          fatal("No extra keywords for Room #%d, %s at %d.",
              Rooms->Room[i].Number, infile, Line);
        Rooms->Room[i].Extra[e].Keyword= make_keyword_list(tmp);
        if((!(tmp= get_tilde_string(ifp, &Line, &Pos)))||
           (!(Rooms->Room[i].Extra[e].Description= my_strdup(tmp))))
          fatal("No extra descriptions for Room #%d, %s of %d.",
              Rooms->Room[i].Number, infile, Line);
        e= ++Rooms->Room[i].ExtraCount;
        Rooms->Room[i].Extra= (extra *)
          get_more_mem((char *)Rooms->Room[i].Extra, e+1, sizeof(extra));
        bzero(&(Rooms->Room[i].Extra[e]), sizeof(extra));
      } else fatal("Unknown line in Room #%d, %s at %d!",
                 Rooms->Room[i].Number, infile, Line);
    }
  }
  if(Verbose)
    fprintf(stderr, "Parsed %d rooms in World.\n", Rooms->Count);
  else if(!Quiet) {
    fprintf(stderr, "done.\n");
  }
  fclose(ifp);
  fix_exit_vnums(Rooms);
  check_duplicate_rooms(Rooms);
  verify_exits(Rooms);
  fix_zone_ids(Zones, Rooms);
  check_room_zone_mismatch(Zones, Rooms);
  return Rooms;
}

char *room_flag_name(int Flag) {
  switch(Flag) {
    case ROOM_DARK:	return "Dark";
    case ROOM_DEATH:	return "Death";
    case ROOM_NOMOB:	return "No Mobiles";
    case ROOM_INDOORS:	return "Indoors";
    case ROOM_NOATTACK:	return "No Attacks";
    case ROOM_NOSTEAL:	return "No Stealing";
    case ROOM_NOSUMMON:	return "No Summoning";
    case ROOM_NOMAGIC:	return "No Magic";
    case ROOM_PRIVATE:	return "Private";
    case ROOM_SOUND:	return "Has Sound";
    default:		return "unknown";
  }
}

char *sector_name(int Sector) {
  switch(Sector) {
    case SECT_TELEPORT:		return "Teleport";
    case SECT_INDOORS:		return "Indoors";
    case SECT_CITY:		return "City";
    case SECT_FIELD:		return "Field";
    case SECT_FOREST:		return "Forest";
    case SECT_HILLS:		return "Hills";
    case SECT_MOUNTAIN:		return "Mountains";
    case SECT_WATER_SWIM:	return "Swimmable";
    case SECT_WATER_NOSWIM:	return "Unswimmable";
    case SECT_AIR:		return "Mid-Air";
    case SECT_UNDERWATER:	return "Underwater";
    default:			return "unknown";
  }
}

char *exittype_name(int Type) {
  switch(Type) {
    case EXIT_OPEN:			return "an Open Passage";
    case EXIT_DOOR:			return "a Door";
    case EXIT_NOPICK:			return "an Unpickable Door";
    case EXIT_SECRET:			return "a Secret Door";
    case EXIT_SECRET_NOPICK:		return "an Unpickable Secret Door";
    case EXIT_OPEN_ALIAS:		return "an Aliased Passage";
    case EXIT_DOOR_ALIAS:		return "an Aliased Door";
    case EXIT_NOPICK_ALIAS:		return "an Aliased Unpickable Door";
    case EXIT_SECRET_ALIAS:		return "an Aliased Secret Door";
    case EXIT_SECRET_NOPICK_ALIAS:	return "an Aliased Unpickable Secret Door";
    default:				return "unknown";
  }
}

char *room_flags(int Flags) {
  static char tmp[MAX_PAGE_LEN];
  int First= 0;

  bzero(tmp, MAX_PAGE_LEN);
  if(Flags & ROOM_DARK) {
    strcat(tmp, room_flag_name(ROOM_DARK));
    First= 1;
  }
  if(Flags & ROOM_DEATH) {
    if(First) strcat(tmp,", ");
    strcat(tmp, room_flag_name(ROOM_DEATH));
    First= 1;
  }
  if(Flags & ROOM_NOMOB) {
    if(First) strcat(tmp,", ");
    strcat(tmp, room_flag_name(ROOM_NOMOB));
    First= 1;
  }
  if(Flags & ROOM_INDOORS) {
    if(First) strcat(tmp,", ");
    strcat(tmp, room_flag_name(ROOM_INDOORS));
    First= 1;
  }
  if(Flags & ROOM_NOATTACK) {
    if(First) strcat(tmp,", ");
    strcat(tmp, room_flag_name(ROOM_NOATTACK));
    First= 1;
  }
  if(Flags & ROOM_NOSTEAL) {
    if(First) strcat(tmp,", ");
    strcat(tmp, room_flag_name(ROOM_NOSTEAL));
    First= 1;
  }
  if(Flags & ROOM_NOSUMMON) {
    if(First) strcat(tmp,", ");
    strcat(tmp, room_flag_name(ROOM_NOSUMMON));
    First= 1;
  }
  if(Flags & ROOM_NOMAGIC) {
    if(First) strcat(tmp,", ");
    strcat(tmp, room_flag_name(ROOM_NOMAGIC));
    First= 1;
  }
  if(Flags & ROOM_PRIVATE) {
    if(First) strcat(tmp,", ");
    strcat(tmp, room_flag_name(ROOM_PRIVATE));
    First= 1;
  }
  if(Flags & ROOM_SOUND) {
    if(First) strcat(tmp,", ");
    strcat(tmp, room_flag_name(ROOM_SOUND));
    First= 1;
  }
  return tmp;
}

/*
 * End of the Great Room Parser... grovelworshipbow
 */

/*
 * This routine loads a shop file (in WileyMUD DikuMUD format) and returns a
 * structure containing all the information it contained.
 */
shops *load_shops(char *infile) {
  FILE *ifp;
  vnum_index *ShopIndex;
  shops *Shops;
  char *tmp;
  register int i, j;
  long Line, Pos;

  if(!infile || !*infile) return NULL;
  if(!(ShopIndex= make_index(infile, NULL))) return NULL;
  Shops= (shops *)get_mem(1, sizeof(shops));
  bzero(Shops, sizeof(shops));
  Shops->Count= ShopIndex->Count;
  Shops->Shop= (shop *)get_mem(Shops->Count, sizeof(shop));
  bzero(Shops->Shop, Shops->Count* sizeof(shop));
  if(!Quiet) {
    fprintf(stderr, "Parsing Shop file...");
    fflush(stderr);
  }
  ifp= open_file(infile, "r");
  for(i= 0; i< Shops->Count; i++) {
    if(!Quiet)
      spin(stderr);
    Line= ShopIndex->VNum[i].Line;
    Pos= ShopIndex->VNum[i].Pos;
    Shops->Shop[i].Number= ShopIndex->VNum[i].Number;
    if(fseek(ifp, Pos, 0) < 0)
      fatal("Cannot load Shop #%d from %s!",
          Shops->Shop[i].Number, infile);
    if(!(tmp= get_line(ifp, &Line, &Pos, 1)))
      fatal("Cannot get vnum for Shop #%d from %s!",
          Shops->Shop[i].Number, infile);
    for(j= Shops->Shop[i].SellCount= 0; j< SHOP_SELLCOUNT; j++) {
      if((!(tmp= get_line(ifp, &Line, &Pos, 1)))||
         (sscanf(tmp, " %d ", &(Shops->Shop[i].Sell[j])) != 1))
        fatal("Cannot get item %d to sell, %s at %d.",
            j, infile, Line);
      if(Shops->Shop[i].Sell[j] > -1) Shops->Shop[i].SellCount++;
    }
    if((!(tmp= get_line(ifp, &Line, &Pos, 1)))||
       (sscanf(tmp, " %lf ", &(Shops->Shop[i].Selling)) != 1))
      fatal("Cannot get Sell Profit Rate, %s at %d.",
          infile, Line);
    if((!(tmp= get_line(ifp, &Line, &Pos, 1)))||
       (sscanf(tmp, " %lf ", &(Shops->Shop[i].Buying)) != 1))
      fatal("Cannot get Buy Profit Rate, %s at %d.",
          infile, Line);
    for(j= Shops->Shop[i].TradeCount= 0; j< SHOP_TRADECOUNT; j++) {
      if((!(tmp= get_line(ifp, &Line, &Pos, 1)))||
         (sscanf(tmp, " %d ", &(Shops->Shop[i].Trade[j])) != 1))
        fatal("Cannot get trade type %d to sell, %s at %d.",
            j, infile, Line);
      if(Shops->Shop[i].Trade[j] > 0) Shops->Shop[i].TradeCount++;
    }
    Shops->Shop[i].MessageCount= SHOP_MESSAGECOUNT;
    for(j= 0; j< Shops->Shop[i].MessageCount; j++) {
      if((!(tmp= get_tilde_string(ifp, &Line, &Pos)))||
         (!(Shops->Shop[i].Message[j]= my_strdup(tmp))))
        fatal("Cannot get message %d, %s at %d.",
            j, infile, Line);
    }
    if((!(tmp= get_line(ifp, &Line, &Pos, 1)))||
       (sscanf(tmp, " %d ", &(Shops->Shop[i].Attitude)) != 1))
      fatal("Cannot get attitude (temper 0), %s at %d.",
          infile, Line);
    if((!(tmp= get_line(ifp, &Line, &Pos, 1)))||
       (sscanf(tmp, " %d ", &(Shops->Shop[i].Immortal)) != 1))
      fatal("Cannot get immortal (temper 1), %s at %d.",
          infile, Line);
    if((!(tmp= get_line(ifp, &Line, &Pos, 1)))||
       (sscanf(tmp, " %d ", &(Shops->Shop[i].Keeper)) != 1))
      fatal("Cannot get keeper vnum, %s at %d.",infile,Line);
    if((!(tmp= get_line(ifp, &Line, &Pos, 1)))||
       (sscanf(tmp, " %d ", &(Shops->Shop[i].Unused)) != 1))
      fatal("Cannot get unused flag, %s at %d.",infile,Line);
    if((!(tmp= get_line(ifp, &Line, &Pos, 1)))||
       (sscanf(tmp, " %d ", &(Shops->Shop[i].Room)) != 1))
      fatal("Cannot get room vnum, %s at %d.",infile,Line);
    if((!(tmp= get_line(ifp, &Line, &Pos, 1)))||
       (sscanf(tmp, " %d ", &(Shops->Shop[i].Open)) != 1))
      fatal("Cannot get open hour, %s at %d.",infile,Line);
    if((!(tmp= get_line(ifp, &Line, &Pos, 1)))||
       (sscanf(tmp, " %d ", &(Shops->Shop[i].Close)) != 1))
      fatal("Cannot get close hour, %s at %d.",infile,Line);
    for(j= 0; j< 2; j++) {
      if((!(tmp= get_line(ifp, &Line, &Pos, 1)))||
         (sscanf(tmp, " %d ", &(Shops->Shop[i].Unused2[j])) != 1))
        fatal("Cannot get unused hour (1,%d), %s at %d.",
            j,infile,Line);
    }
  }
  fclose(ifp);
  if(!Quiet)
    fprintf(stderr, "done.\n");
  return Shops;
}

char *shop_attitude_name(int Attitude) {
  switch(Attitude) {
    case 0:  return "rude";
    case 1:  return "aloof";
    default: return "unknown";
  }
}

char *shop_immortal_name(int Immortal) {
  switch(Immortal) {
    case 0:  return "first-strike";
    case 1:  return "immortal";
    default: return "unknown";
  }
}

int remap_shop_vnum(shops *Shops, int VNum) {
  register int i;

  for(i= 0; i< Shops->Count; i++)
    if(Shops->Shop[i].Number == VNum) return i;
  return -1;
}

/*
 * General routines used for prettiness...
 */

char *equip_name(int Position) {
  switch(Position) {
    case WEAR_LIGHT:		return "Used as Light";
    case WEAR_FINGER_R:		return "Worn on Right Finger";
    case WEAR_FINGER_L:		return "Worn on Left Finger";
    case WEAR_NECK_1:		return "Worn on Neck (1)";
    case WEAR_NECK_2:		return "Worn on Neck (2)";
    case WEAR_BODY:		return "Worn on Body";
    case WEAR_HEAD:		return "Worn on Head";
    case WEAR_LEGS:		return "Worn on Legs";
    case WEAR_HANDS:		return "Worn on Hands";
    case WEAR_ARMS:		return "Worn on Arms";
    case WEAR_SHIELD:		return "Used as Shield";
    case WEAR_ABOUT:		return "Worn About Body";
    case WEAR_WAIST:		return "Worn on Waist";
    case WEAR_WRIST_R:		return "Worn on Right Wrist";
    case WEAR_WRIST_L:		return "Worn on Left Wrist";
    case WIELD:			return "Wielded";
    case HOLD:			return "Held";
    case TWOH:			return "Wielded as Two-Handed";
    default:			return "unknown";
  }
}

char *doorstate_name(int State) {
  switch(State) {
    case DOOR_OPEN:	return "Open";
    case DOOR_CLOSED:	return "Closed";
    case DOOR_LOCKED:	return "Closed and Locked";
    default:		return "unknown";
  }
}

char *sex_name(int State) {
  switch(State) {
    case SEX_NEUTER:	return "Neuter";
    case SEX_MALE:	return "Male";
    case SEX_FEMALE:	return "Female";
    default:		return "unknown";
  }
}

char *race_name(int Race) {
  switch(Race) {
    case RACE_HALFBREED:	return "HALFBREED";
    case RACE_HUMAN:		return "human";
    case RACE_ELVEN:		return "high-elf";
    case RACE_DWARF:		return "dwarf";
    case RACE_HALFLING:		return "halfling";
    case RACE_GNOME:		return "gnome";
    case RACE_REPTILE:		return "reptile";
    case RACE_SPECIAL:		return "Special";
    case RACE_LYCANTH:		return "lycanthrope";
    case RACE_DRAGON:		return "dragon";
    case RACE_UNDEAD:		return "undead";
    case RACE_ORC:		return "orc";
    case RACE_INSECT:		return "insectoid";
    case RACE_ARACHNID:		return "ARACHNID";
    case RACE_DINOSAUR:		return "dinosaur";
    case RACE_FISH:		return "fish";
    case RACE_BIRD:		return "avis";
    case RACE_GIANT:		return "Giant";
    case RACE_PREDATOR:		return "PREDATOR";
    case RACE_PARASITE:		return "Parasitic";
    case RACE_SLIME:		return "slime";
    case RACE_DEMON:		return "Demonic";
    case RACE_SNAKE:		return "snake";
    case RACE_HERBIV:		return "Herbivorous";
    case RACE_TREE:		return "Tree";
    case RACE_VEGGIE:		return "Vegan";
    case RACE_ELEMENT:		return "Elemental";
    case RACE_PLANAR:		return "Planar";
    case RACE_DEVIL:		return "Demonic";
    case RACE_GHOST:		return "ghost";
    case RACE_GOBLIN:		return "goblin";
    case RACE_TROLL:		return "troll";
    case RACE_VEGMAN:		return "Vegman";
    case RACE_MFLAYER:		return "Mindflayer";
    case RACE_PRIMATE:		return "Primate";
    case RACE_ANIMAL:		return "animal";
    case RACE_FAERY:		return "FAERIE";
    case RACE_PLANT:		return "PLANT";
    default:			return "monster";
  }
}

char *class_name(int Class) {
  register int i;
  static char class_buffer[256];
  static char *class_name_list[] = {
    "mage", "cleric", "warrior",
    "rogue", "ranger", "druid"
  };

  bzero(class_buffer, 256);
  for (i= 0; i< MAX_CLASS; i++) {
    if(Class & (1<<i)) {
      if(strlen(class_buffer))
        strcat(class_buffer, " ");
      strcat(class_buffer, class_name_list[i]);
    }
  }
  if(!strlen(class_buffer))
    strcpy(class_buffer, "unknown");
  return class_buffer;
}

char *hate_name(int Type, int Value) {
  static char hate_buffer[256];

  switch(Type) {
    case HATE_SEX:
      sprintf(hate_buffer, "sex %s's", sex_name(Value));
      break;
    case HATE_RACE:
      sprintf(hate_buffer, "race %s's", race_name(Value));
      break;
    case HATE_CHAR:
      sprintf(hate_buffer, "a specific character");
      break;
    case HATE_CLASS:
      sprintf(hate_buffer, "any of class %s", class_name(Value));
      break;
    case HATE_EVIL:
      sprintf(hate_buffer, "any more evil than %s", alignment_name(Value));
      break;
    case HATE_GOOD:
      sprintf(hate_buffer, "any more good than %s", alignment_name(Value));
      break;
    case HATE_VNUM:
      sprintf(hate_buffer, "mobile [#%d]", Value);
      break;
    case HATE_RICH:
      sprintf(hate_buffer, "anyone carrying more than %d gold", Value);
      break;
    default:
      sprintf(hate_buffer, "something unknown");
  }
  return hate_buffer;
}

char *alignment_name(int Alignment) {
  if(Alignment <= ALIGN_REALLY_VILE)      return "Really Vile";
  else if(Alignment <= ALIGN_VILE)        return "Vile";
  else if(Alignment <= ALIGN_VERY_EVIL)   return "Very Evil";
  else if(Alignment <= ALIGN_EVIL)        return "Evil";
  else if(Alignment <= ALIGN_WICKED)      return "Wicked";
  else if(Alignment <  ALIGN_NICE)        return "Neutral";
  else if(Alignment <  ALIGN_GOOD)        return "Nice";
  else if(Alignment <  ALIGN_VERY_GOOD)   return "Good";
  else if(Alignment <  ALIGN_HOLY)        return "Very Good";
  else if(Alignment <  ALIGN_REALLY_HOLY) return "Holy";
  else                                    return "Really Holy";
}

char *damage_type_name(int Type) {
  switch(Type) {
    case FIRE_DAMAGE: return "Fire";
    case COLD_DAMAGE: return "Cold";
    case ELEC_DAMAGE: return "Electricity";
    case BLOW_DAMAGE: return "Blow";
    case ACID_DAMAGE: return "Acid";
    default: return "unknown";
  }
}

char *immunity_name(int Imms) {
  register int i;
  static char imm_buffer[256];
  static char *imm_name[] = {
    "Fire", "Cold", "Electricity", "Energy",
    "Blunt", "Pierce", "Slash", "Acid",
    "Poison", "Drain", "Sleep", "Charm",
    "Hold", "Non-Magical", "Plus-1", "Plus-2",
    "Plus-3", "Plus-4"
  };

  bzero(imm_buffer, 256);
  for (i= 0; i< MAX_IMM; i++) {
    if(Imms & (1<<i)) {
      if(strlen(imm_buffer))
        strcat(imm_buffer, " ");
      strcat(imm_buffer, imm_name[i]);
    }
  }
  if(!strlen(imm_buffer))
    strcpy(imm_buffer, "unknown");
  return imm_buffer;
}

char * item_type_name(int Type) {
  switch(Type) {
    case ITEM_LIGHT: return "Light";
    case ITEM_SCROLL: return "Scroll";
    case ITEM_WAND: return "Wand";
    case ITEM_STAFF: return "Staff";
    case ITEM_WEAPON: return "Weapon";
    case ITEM_FIREWEAPON: return "Fire-Weapon";
    case ITEM_MISSILE: return "Missile";
    case ITEM_TREASURE: return "Treasure";
    case ITEM_ARMOR: return "Armour";
    case ITEM_POTION: return "Potion";
    case ITEM_WORN: return "Worn";
    case ITEM_OTHER: return "Other";
    case ITEM_TRASH: return "Trash";
    case ITEM_TRAP: return "Trap";
    case ITEM_CONTAINER: return "Container";
    case ITEM_NOTE: return "Note";
    case ITEM_DRINKCON: return "Drink-Container";
    case ITEM_KEY: return "Key";
    case ITEM_FOOD: return "Food";
    case ITEM_MONEY: return "Money";
    case ITEM_PEN: return "Pen";
    case ITEM_BOAT: return "Boat";
    case ITEM_AUDIO: return "Audio";
    case ITEM_BOARD: return "Board";
    case ITEM_KENNEL: return "Kennel";
    default: return "unknown";
  }
}

char * item_wear_name(int Type) {
  register int i;
  static char item_buffer[256];
  static char *item_name[] = {
    "Taken", "worn on Finger", "worn around Neck", "worn on the Body",
    "worn on Head", "worn on Legs", "worn on Feet", "worn on Hands",
    "worn on Arms", "used as a Shield", "worn About the body", "worn around the Waiste",
    "worn on Wrist", "Wielded", "Held", "wielded Two-handed"
  };

  bzero(item_buffer, 256);
  for (i= 0; i< MAX_WEAR; i++) {
    if(Type & (1<<i)) {
      if(strlen(item_buffer))
        strcat(item_buffer, ", ");
      strcat(item_buffer, item_name[i]);
    }
  }
  if(!strlen(item_buffer))
    strcpy(item_buffer, "unknown");
  return item_buffer;
}

char * item_flag_name(int Type) {
  register int i;
  static char item_buffer[256];
  static char *item_name[] = {
    "Glow", "Hum", "Metal", "Mineral",
    "Organic", "Invisible", "Magical", "Undroppable",
    "Blessed", "Anti-Good", "Anti-Evil", "Anti-Neutral",
    "Anti-Cleric", "Anti-Mage", "Anti-Thief", "Anti-Fighter",
    "Anti-Ranger", "Parishable"
  };

  bzero(item_buffer, 256);
  for (i= 0; i< MAX_ITEM_FLAGS+1; i++) {
    if(Type & (1<<i)) {
      if(strlen(item_buffer))
        strcat(item_buffer, " ");
      strcat(item_buffer, item_name[i]);
    }
  }
  if(!strlen(item_buffer))
    strcpy(item_buffer, "unknown");
  return item_buffer;
}

char *liquid_name(int Type) {
  switch(Type) {
    case LIQ_WATER:		return "Water";
    case LIQ_BEER:		return "Beer";
    case LIQ_WINE:		return "Wine";
    case LIQ_ALE:		return "Ale";
    case LIQ_DARKALE:		return "Dark-Ale";
    case LIQ_WHISKY:		return "Whisky";
    case LIQ_LEMONADE:		return "Lemonaide";
    case LIQ_FIREBRT:		return "Firebreather";
    case LIQ_LOCALSPC:		return "Local-Speciality";
    case LIQ_SLIME:		return "Slime";
    case LIQ_MILK:		return "Milk";
    case LIQ_TEA:		return "Tea";
    case LIQ_COFFE:		return "Coffee";
    case LIQ_BLOOD:		return "Blood";
    case LIQ_SALTWATER:		return "Salt-Water";
    case LIQ_COKE:		return "Coke";
    default:			return "unknown";
  }
}

char *item_equip_name(int Position) {
  switch(Position) {
    case WEAR_LIGHT:		return "Used as Light";
    case WEAR_FINGER_R:		return "Worn on Right Finger";
    case WEAR_FINGER_L:		return "Worn on Left Finger";
    case WEAR_NECK_1:		return "Worn on Neck (1)";
    case WEAR_NECK_2:		return "Worn on Neck (2)";
    case WEAR_BODY:		return "Worn on Body";
    case WEAR_HEAD:		return "Worn on Head";
    case WEAR_LEGS:		return "Worn on Legs";
    case WEAR_HANDS:		return "Worn on Hands";
    case WEAR_ARMS:		return "Worn on Arms";
    case WEAR_SHIELD:		return "Used as Shield";
    case WEAR_ABOUT:		return "Worn About Body";
    case WEAR_WAIST:		return "Worn on Waist";
    case WEAR_WRIST_R:		return "Worn on Right Wrist";
    case WEAR_WRIST_L:		return "Worn on Left Wrist";
    case WIELD:			return "Wielded";
    case HOLD:			return "Held";
    case TWOH:			return "Wielded as Two-Handed";
    default:			return "unknown";
  }
}

char *container_closeable(int Value) {
  register int i;
  static char cont_buffer[256];
  static char *cont_name[] = {
    "Closeable", "Pickproof", "Closed", "Locked"
  };

  bzero(cont_buffer, 256);
  for (i= 0; i< MAX_CONT_CLOSE; i++) {
    if(Value & (1<<i)) {
      if(strlen(cont_buffer))
        strcat(cont_buffer, " ");
      strcat(cont_buffer, cont_name[i]);
    }
  }
  if(!strlen(cont_buffer))
    strcpy(cont_buffer, "unknown");
  return cont_buffer;
}

char *affected_by_name(int Flag) {
  register int i;
  static char a_buffer[256];
  static char *a_name[] = {
    "Blind", "Invisible", "Detect-Evil", "Detect-Invisible",
    "Detect-Magic", "Sense-Life", "Silence", "Sanctuary",
    "Group", "MISSING", "Curse", "Flying",
    "Poison", "Protection-From-Evil", "Paralysis", "Infravision",
    "Water-Breathing", "Sleep", "Drug-Free", "Sneak",
    "Hide", "Fear", "Charm", "Follow",
    "UNDEF-1", "True-Sight", "Scrying", "Fireshield",
    "Ride", "UNDEF-6", "UNDEF-7", "UNDEF-8"
  };

  bzero(a_buffer, 256);
  for (i= 0; i< MAX_AFFECTED_BY; i++) {
    if(Flag & (1<<i)) {
      if(strlen(a_buffer))
        strcat(a_buffer, " ");
      strcat(a_buffer, a_name[i]);
    }
  }
  if(!strlen(a_buffer))
    strcpy(a_buffer, "unknown");
  return a_buffer;
}

char *apply_name(int Flag) {
  switch(Flag) {
    case APPLY_NONE:		return "None";
    case APPLY_STR:		return "Strength";
    case APPLY_DEX:		return "Dexterity";
    case APPLY_INT:		return "Intelligence";
    case APPLY_WIS:		return "Wisdom";
    case APPLY_CON:		return "Constitution";
    case APPLY_SEX:		return "Sex";
    case APPLY_CLASS:		return "Class";
    case APPLY_LEVEL:		return "Level";
    case APPLY_AGE:		return "Age";
    case APPLY_CHAR_WEIGHT:	return "Weight";
    case APPLY_CHAR_HEIGHT:	return "Height";
    case APPLY_MANA:		return "Mana";
    case APPLY_HIT:		return "Hit-Points";
    case APPLY_MOVE:		return "Move";
    case APPLY_GOLD:		return "Gold";
    case APPLY_EXP:		return "Experience";
    case APPLY_AC:		return "Armour-Class";
    case APPLY_HITROLL:		return "To-Hit";
    case APPLY_DAMROLL:		return "Damage";
    case APPLY_SAVING_PARA:	return "Save-vs-Paralysis";
    case APPLY_SAVING_ROD:	return "Save-vs-Rod";
    case APPLY_SAVING_PETRI:	return "Save-vs-Petrification";
    case APPLY_SAVING_BREATH:	return "Save-vs-Breath-Weapons";
    case APPLY_SAVING_SPELL:	return "Save-vs-Spell";
    case APPLY_SAVE_ALL:	return "Saving-Throws";
    case APPLY_IMMUNE:		return "Resistance";
    case APPLY_SUSC:		return "Susceptability";
    case APPLY_M_IMMUNE:	return "Immunity";
    case APPLY_SPELL:		return "Spell";
    case APPLY_WEAPON_SPELL:	return "Weapon-Spell";
    case APPLY_EAT_SPELL:	return "Eat-Spell";
    case APPLY_BACKSTAB:	return "Backstab";
    case APPLY_KICK:		return "Kick";
    case APPLY_SNEAK:		return "Sneak";
    case APPLY_HIDE:		return "Hide";
    case APPLY_BASH:		return "Bash";
    case APPLY_PICK:		return "Picklock";
    case APPLY_STEAL:		return "Steal";
    case APPLY_TRACK:		return "Track";
    case APPLY_HITNDAM:		return "To-Hit-and-Damage";
    default:			return "unknown";
  }
}

char *position_name(int Flag) {
  switch(Flag) {
    case POSITION_DEAD:		return "Dead";
    case POSITION_MORTALLYW:	return "Mortally-Wounded";
    case POSITION_INCAP:	return "Incapacitated";
    case POSITION_STUNNED:	return "Stunned";
    case POSITION_SLEEPING:	return "Sleeping";
    case POSITION_RESTING:	return "Resting";
    case POSITION_SITTING:	return "Sitting";
    case POSITION_FIGHTING:	return "Fighting";
    case POSITION_STANDING:	return "Standing";
    case POSITION_MOUNTED:	return "Mounted";
    default:			return "unknown";
  }
}

char *act_name(int Flag) {
  register int i;
  static char a_buffer[256];
  static char *a_name[] = {
    "Special-Function", "Sentinel", "Scavenger", "IsNPC",
    "Nice-Thief", "Aggressive", "Stay-Zone", "Wimpy",
    "Annoying", "Hateful", "Afraid", "Immortal",
    "Hunting", "Deadly", "Polymorph-Self", "Polymorph-Other",
    "Guardian", "Use-Items", "Fighter-Moves", "Provide-Food",
    "Protector", "Mount", "Switch"
  };

  bzero(a_buffer, 256);
  for (i= 0; i< MAX_ACT; i++) {
    if(Flag & (1<<i)) {
      if(strlen(a_buffer))
        strcat(a_buffer, " ");
      strcat(a_buffer, a_name[i]);
    }
  }
  if(!strlen(a_buffer))
    strcpy(a_buffer, "unknown");
  return a_buffer;
}

int remap_obj_vnum(objects *Objects, int VNum) {
  register int i;

  for(i= 0; i< Objects->Count; i++)
    if(Objects->Object[i].Number == VNum) return i;
  return -1;
}

char *obj_name(objects *Objects, int VNum) {
  int i;

  if((i= remap_obj_vnum(Objects, VNum))!= -1)
    return Objects->Object[i].ShortDesc;
  else return "a non-existant object";
}

void check_duplicate_objs(objects *Objects) {
  register int i, tmp;

  if(!Quiet) {
    fprintf(stderr, "Checking for duplicate object vnums...");
    fflush(stderr);
  }
  for(i= 0; i< Objects->Count; i++) {
    if(!Quiet)
      spin(stderr);
    tmp= Objects->Object[i].Number;
    Objects->Object[i].Number = -1;
    if(remap_obj_vnum(Objects, tmp) >= 0)
      if(Debug)
        bug("OBJECT [#%d] is DUPLICATED!!!", tmp);
    Objects->Object[i].Number = tmp;
  }
  if(!Quiet)
    fprintf(stderr, "done.\n");
}

void set_obj_zones(zones *Zones, objects *Objects) {
  register int i, j, CorrectZone;

  if(!Quiet) {
    fprintf(stderr, "Setting Zone id tags...");
    fflush(stderr);
  }
  for(i= 0; i< Objects->Count; i++) {
    if(!Quiet)
      spin(stderr);
    CorrectZone= -1;
    for(j= 0; j< Zones->Count; j++)
      if(Objects->Object[i].Number <= Zones->Zone[j].Top)
        if(Objects->Object[i].Number >= ((j>0)?Zones->Zone[j-1].Top+1:0)) {
          CorrectZone= Zones->Zone[j].Number;
          break;
        }
    if(CorrectZone == -1) {
      if(Verbose)
        bug("Holy BatTurds!  I have NO IDEA what zone this object [#%d] "
            "belongs to!\nPutting it in Zone %d...\n",
            Objects->Object[i].Number, Zones->Zone[0].Number);
      Objects->Object[i].Zone= Zones->Zone[0].Number;
    } else {
      Objects->Object[i].Zone= CorrectZone;
    }
  }
  if(!Quiet)
    fprintf(stderr, "done.\n");
}

void check_object_zone_mismatch(zones *Zones, objects *Objects) {
  register int i, CorrectZone;

  if(!Quiet) {
    fprintf(stderr, "Verifying proper Zone id tags...");
    fflush(stderr);
  }
  for(i= 0; i< Objects->Count; i++) {
    if(!Quiet)
      spin(stderr);
    CorrectZone= Objects->Object[i].Number / 100;
    if(Objects->Object[i].Zone != CorrectZone) {
      bug("Improper Zone ID of [#%d] for object \"%s\" [#%d],\n"
          "Should belong to Zone [#%d].\n",
          Objects->Object[i].Zone, Objects->Object[i].Name,
          Objects->Object[i].Number, CorrectZone);
      /* Objects->Object[i].Zone= CorrectZone; */
    }
  }
  if(!Quiet)
    fprintf(stderr, "done.\n");
}

/*
 * This routine loads the object file from WileyMUD III format.
 */
objects *load_objects(char *infile, zones *Zones) {
  FILE *ifp;
  vnum_index *ObjIndex;
  objects *Objects;
  char *tmp; /* , tmp2[MAX_LINE_LEN], *tmp3; */
  register int i, j, e;
  long Line, Pos;

  if(!infile || !*infile) return NULL;
  if(!(ObjIndex= make_index(infile, NULL))) return NULL;
  Objects= (objects *)get_mem(1, sizeof(objects));
  bzero(Objects, sizeof(objects));
  Objects->Count= ObjIndex->Count;
  Objects->Object= (object *)get_mem(Objects->Count, sizeof(object));
  bzero(Objects->Object, Objects->Count* sizeof(object));
  ifp= open_file(infile, "r");

  if(Verbose)
    fprintf(stderr, "Parsing Object file...\n");
  else if(!Quiet) {
    fprintf(stderr, "Parsing Object file...");
    fflush(stderr);
  }

  for(i= 0; i< Objects->Count; i++) {
    Line= ObjIndex->VNum[i].Line;
    Pos= ObjIndex->VNum[i].Pos;
    Objects->Object[i].Number= ObjIndex->VNum[i].Number;
    if(fseek(ifp, Pos, 0) < 0)
      fatal("Cannot load Object $%d from %s!",
          Objects->Object[i].Number, infile);
    if(!(tmp= get_line(ifp, &Line, &Pos, 1)))
      fatal("Cannot get vnum for Object #%d from %s!",
          Objects->Object[i].Number, infile);
    if((!(tmp= get_tilde_string(ifp, &Line, &Pos)))||
       (!(Objects->Object[i].Name= my_strdup(tmp))))
      fatal("Cannot get name for Object #%d, %s at %d.",
          Objects->Object[i].Number, infile, Line);

    if(Verbose)
      fprintf(stderr, "  (%d) Parsing \"%s\"[#%d]...\n", i,
              Objects->Object[i].Name, Objects->Object[i].Number);
    else if(!Quiet)
      spin(stderr);

    if((!(tmp= get_tilde_string(ifp, &Line, &Pos)))||
       (!(Objects->Object[i].ShortDesc= my_strdup(tmp))))
      fatal("Cannot get short description for Object #%d, %s at %d.",
          Objects->Object[i].Number, infile, Line);

    if((!(tmp= get_tilde_string(ifp, &Line, &Pos)))||
       (!(Objects->Object[i].Description= my_strdup(tmp))))
      fatal("Cannot get description for Object #%d, %s at %d.",
          Objects->Object[i].Number, infile, Line);

    if((!(tmp= get_tilde_string(ifp, &Line, &Pos)))||
       (!(Objects->Object[i].ActionDesc= my_strdup(tmp))))
      fatal("Cannot get action description for Object #%d, %s at %d.",
          Objects->Object[i].Number, infile, Line);

    if((!(tmp= get_line(ifp, &Line, &Pos, 1)))||
       (sscanf(tmp, " %d %d %d ", &(Objects->Object[i].Type),
               &(Objects->Object[i].Flags.Extra), &(Objects->Object[i].Flags.Wear)) != 3))
      fatal("Cannot get main flags for Object #%d, %s at %d.",
          Objects->Object[i].Number, infile, Line);

    if((!(tmp= get_line(ifp, &Line, &Pos, 1)))||
       (sscanf(tmp, " %d %d %d %d ", &(Objects->Object[i].Flags.Value[0]),
               &(Objects->Object[i].Flags.Value[1]), &(Objects->Object[i].Flags.Value[2]),
               &(Objects->Object[i].Flags.Value[3])) != 4))
      fatal("Cannot get value flags for Object #%d, %s at %d.",
          Objects->Object[i].Number, infile, Line);

    if((!(tmp= get_line(ifp, &Line, &Pos, 1)))||
       (sscanf(tmp, " %d %d %d ", &(Objects->Object[i].Weight),
               &(Objects->Object[i].Value), &(Objects->Object[i].Rent)) != 3))
      fatal("Cannot get value flags for Object #%d, %s at %d.",
          Objects->Object[i].Number, infile, Line);

    Objects->Object[i].Affect= (obj_affect *)get_mem(1, sizeof(obj_affect));
    bzero(Objects->Object[i].Affect, sizeof(obj_affect));
    Objects->Object[i].Affect[0].location= 0;
    Objects->Object[i].Affect[0].modifier = 0;

    Objects->Object[i].Extra= (extra *)get_mem(1, sizeof(extra));
    bzero(Objects->Object[i].Extra, sizeof(extra));

    for(j= e= Objects->Object[i].AffectCount= Objects->Object[i].ExtraCount= 0;
        (tmp= get_line(ifp, &Line, &Pos, 1));) {
      if(*tmp == '#') break;
      /* else if(*tmp == '~') continue; */
      else if(*tmp == '$') continue;
      else if(*tmp == 'A') {
        if((!(tmp= get_line(ifp, &Line, &Pos, 1)))||
           (sscanf(tmp, " %d %d ", &(Objects->Object[i].Affect[j].location),
                   &(Objects->Object[i].Affect[j].modifier)) != 2))
          fatal("Cannot get affect flags for Object #%d, %s at %d.",
              Objects->Object[i].Number, infile, Line);
        j= ++Objects->Object[i].AffectCount;
        Objects->Object[i].Affect= (obj_affect *)
          get_more_mem((char *)Objects->Object[i].Affect, j+1, sizeof(obj_affect));
        bzero(&(Objects->Object[i].Affect[j]), sizeof(obj_affect));
      } else if(*tmp == 'E') {
        if(!(tmp= get_tilde_string(ifp, &Line, &Pos)))
          fatal("No extra keywords for Object #%d, %s at %d.",
              Objects->Object[i].Number, infile, Line);
        Objects->Object[i].Extra[e].Keyword= make_keyword_list(tmp);
        if((!(tmp= get_tilde_string(ifp, &Line, &Pos)))||
           (!(Objects->Object[i].Extra[e].Description= my_strdup(tmp))))
          fatal("No extra descriptions for Object #%d, %s of %d.",
              Objects->Object[i].Number, infile, Line);
        e= ++Objects->Object[i].ExtraCount;
        Objects->Object[i].Extra= (extra *)
          get_more_mem((char *)Objects->Object[i].Extra, e+1, sizeof(extra));
        bzero(&(Objects->Object[i].Extra[e]), sizeof(extra));
      } else fatal("Unknown line in Object #%d, %s at %d!",
                 Objects->Object[i].Number, infile, Line);
    }
  }
  if(Verbose)
    fprintf(stderr, "Parsed %d objects in World.\n", Objects->Count);
  else if(!Quiet) {
    fprintf(stderr, "done.\n");
  }
  fclose(ifp);
  check_duplicate_objs(Objects);
  set_obj_zones(Zones, Objects);
  check_object_zone_mismatch(Zones, Objects);
  return Objects;
}

char *spell_name(int Spell) {
  register int i;
  static int assigned = 0;
  static char *spell[MAX_SKILLS];

  if(!assigned) {
    bzero(spell, MAX_SKILLS*sizeof(char *));

ASSIGN_SPELL( SKILL_APRAISE, 0, 1, "appraise", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, 2, CLASS_ALL, LOKI, LOKI, LOKI, LOKI, LOKI, LOKI);
ASSIGN_SPELL( SKILL_BACKSTAB, 0, 1, "backstab", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI, LOKI, LOKI, 1, LOKI, LOKI );
ASSIGN_SPELL( SKILL_BANDAGE, 0, 1, "bandage", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, 1, CLASS_ALL, LOKI, LOKI, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SKILL_BARE_HAND, 0, 1, "barehand", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, 1, CLASS_ALL, LOKI, LOKI, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SKILL_BARTER, 0, 0, "barter", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI, LOKI, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SKILL_BASH, 0, 1, "bash", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI, LOKI, 1, LOKI, 1, LOKI );
ASSIGN_SPELL( SKILL_BASH_W_SHIELD, 0, 0, "shield bash", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI, LOKI, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SKILL_BLIND_FIGHTING, 0, 0, "blind fighting", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI, LOKI, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SKILL_BREW, 0, 0, "brewing", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI, LOKI, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SKILL_DETECT_NOISE, 0, 0, "hear noise", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI, LOKI, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SKILL_DISARM, 0, 1, "disarm", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI, LOKI, 1, LOKI, 1, LOKI );
ASSIGN_SPELL( SKILL_DISARM_TRAP, 0, 0, "disarm trap", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI, LOKI, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SKILL_DODGE, 0, 0, "dodge", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI, LOKI, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SKILL_DOOR_BASH, 0, 1, "doorbash", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI, LOKI, 1, LOKI, 1, LOKI );
ASSIGN_SPELL( SKILL_ENDURANCE, 0, 1, "endurance", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, 3, CLASS_ALL, LOKI, LOKI, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SKILL_FIND_TRAP, 0, 1, "locate trap", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI, LOKI, LOKI, 1, LOKI, LOKI );
ASSIGN_SPELL( SKILL_HIDE, 0, 1, "hide", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI, LOKI, LOKI, 1, 1, LOKI );
ASSIGN_SPELL( SKILL_KICK, 0, 1, "kick", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI, LOKI, 1, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SKILL_KNOCK_OUT, 0, 0, "knockout", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI, LOKI, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SKILL_MEDITATION, 0, 1, "meditation", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, 7, CLASS_ALL, LOKI, LOKI, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SKILL_PARRY, 0, 0, "parry", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI, LOKI, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SKILL_PEER, 0, 1, "peer", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI, LOKI, LOKI, 1, 1, LOKI );
ASSIGN_SPELL( SKILL_PICK_LOCK, 0, 1, "pick lock", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI, LOKI, LOKI, 1, LOKI, LOKI );
ASSIGN_SPELL( SKILL_PUNCH, 0, 1, "punch", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, 2, CLASS_ALL, LOKI, LOKI, 1, LOKI, 1, LOKI );
ASSIGN_SPELL( SKILL_READ_MAGIC, 0, 0, "read magic", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, 3, CLASS_ALL, LOKI, LOKI, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SKILL_RESCUE, 0, 1, "rescue", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI, LOKI, 1, LOKI, 1, LOKI );
ASSIGN_SPELL( SKILL_RIDE, 0, 1, "riding", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, 1, CLASS_ALL, LOKI, LOKI, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SKILL_SCRIBE, 0, 0, "scribe", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI, LOKI, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SKILL_SEARCH, 0, 1, "search", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI, LOKI, LOKI, 1, 1, LOKI );
ASSIGN_SPELL( SKILL_SNEAK, 0, 1, "sneak", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI, LOKI, LOKI, 1, 1, LOKI );
ASSIGN_SPELL( SKILL_SPEC_BLUDGE, 0, 1, "bludgeon spec", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI, LOKI, 1, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SKILL_SPEC_CLEAVE, 0, 1, "cleave spec", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI, LOKI, 1, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SKILL_SPEC_CRUSH, 0, 1, "crush spec", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI, LOKI, 1, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SKILL_SPEC_PIERCE, 0, 1, "pierce spec", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI, LOKI, 1, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SKILL_SPEC_SLASH, 0, 1, "slash spec", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI, LOKI, 1, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SKILL_SPEC_SMASH, 0, 1, "smash spec", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI, LOKI, 1, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SKILL_SPEC_SMITE, 0, 1, "smite spec", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI, LOKI, 1, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SKILL_SPEC_STAB, 0, 1, "stab spec", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI, LOKI, 1, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SKILL_SPEC_WHIP, 0, 1, "whip spec", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI, LOKI, 1, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SKILL_SPELLCRAFT, 0, 1, "spellcraft", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, 7, CLASS_MAGICAL, LOKI, LOKI, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SKILL_STEAL, 0, 1, "steal", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI, LOKI, LOKI, 1, LOKI, LOKI );
ASSIGN_SPELL( SKILL_SWIMMING, 0, 1, "swimming", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, 1, CLASS_ALL, LOKI, LOKI, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SKILL_TRACK, 0, 1, "track", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, 3, CLASS_SNEAK, LOKI, LOKI, LOKI, 1, 1, LOKI );
ASSIGN_SPELL( SKILL_TWO_HANDED, 0, 1, "two handed", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, 5, CLASS_FIGHTER, LOKI, LOKI, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SKILL_TWO_WEAPON, 0, 0, "dual wield", NULL, 0, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI, LOKI, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_MANA, 0, 0, "MANA", NULL, 12, 100, 100, TAR_IGNORE, POSITION_FIGHTING, LOKI, CLASS_ALL, LOKI, LOKI, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_ACID_BLAST, 1, 0, "acid blast", cast_acid_blast, 24, 15, 50, TAR_CHAR_ROOM|TAR_FIGHT_VICT|TAR_VIOLENT, POSITION_FIGHTING, LOKI, CLASS_ALL, 7, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_AID, 1, 0, "aid", cast_aid, 12, 15, 50, TAR_CHAR_ROOM, POSITION_STANDING, LOKI, CLASS_ALL, LOW_IMMORTAL, 10, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_ANIMATE_DEAD, 1, 0, "animate dead", cast_animate_dead, 24, 15, 50, TAR_OBJ_ROOM, POSITION_STANDING, LOKI, CLASS_ALL, 10, 7, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_ARMOR, 1, 0, "armour", cast_armor, 12, 5, 50, TAR_CHAR_ROOM, POSITION_STANDING, LOKI, CLASS_ALL, 4, 1, LOKI, LOKI, 10, LOKI );
ASSIGN_SPELL( SPELL_ASTRAL_WALK, 1, 0, "astral walk", cast_astral_walk, 12, 33, 50, TAR_CHAR_WORLD, POSITION_STANDING, LOKI, CLASS_ALL, 21, 18, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_BLESS, 1, 0, "bless", cast_bless, 12, 5, 50, TAR_OBJ_INV|TAR_OBJ_EQUIP|TAR_CHAR_ROOM, POSITION_STANDING, LOKI, CLASS_ALL, LOW_IMMORTAL, 1, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_BLINDNESS, 1, 0, "blindness", cast_blindness, 24, 5, 50, TAR_CHAR_ROOM|TAR_FIGHT_VICT|TAR_VIOLENT, POSITION_FIGHTING, LOKI, CLASS_ALL, 8, 6, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_BURNING_HANDS, 1, 0, "burning hands", cast_burning_hands, 24, 30, 50, TAR_IGNORE|TAR_VIOLENT, POSITION_FIGHTING, LOKI, CLASS_ALL, 5, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_CACAODEMON, 1, 0, "cacaodemon", cast_cacaodemon, 24, 50, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, 30, 30, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_CALL_LIGHTNING, 1, 0, "call lightning", cast_call_lightning, 36, 15, 50, TAR_CHAR_ROOM|TAR_FIGHT_VICT|TAR_VIOLENT, POSITION_FIGHTING, LOKI, CLASS_ALL, LOW_IMMORTAL, 15, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_CALM, 1, 0, "calm", cast_calm, 24, 15, 50, TAR_CHAR_ROOM, POSITION_STANDING, LOKI, CLASS_ALL, 4, 2, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_CAUSE_CRITICAL, 1, 0, "cause critical", cast_cause_critic, 18, 11, 50, TAR_CHAR_ROOM|TAR_FIGHT_VICT|TAR_VIOLENT, POSITION_FIGHTING, LOKI, CLASS_ALL, LOW_IMMORTAL, 9, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_CAUSE_LIGHT, 1, 0, "cause light", cast_cause_light, 12, 8, 50, TAR_CHAR_ROOM|TAR_FIGHT_VICT, POSITION_FIGHTING, LOKI, CLASS_ALL, LOW_IMMORTAL, 1, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_CAUSE_SERIOUS, 1, 0, "cause serious", cast_cause_serious, 12, 9, 50, TAR_CHAR_ROOM|TAR_VIOLENT, POSITION_FIGHTING, LOKI, CLASS_ALL, 30, 7, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_CHARM_MONSTER, 1, 0, "charm monster", cast_charm_monster, 18, 5, 50, TAR_CHAR_ROOM|TAR_VIOLENT, POSITION_STANDING, LOKI, CLASS_ALL, 8, 8, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_CHARM_PERSON, 1, 0, "charm person", cast_charm_person, 12, 5, 50, TAR_CHAR_ROOM|TAR_SELF_NONO|TAR_FIGHT_VICT|TAR_VIOLENT, POSITION_STANDING, LOKI, CLASS_ALL, 12, 12, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_CHILL_TOUCH, 1, 0, "chill touch", cast_chill_touch, 12, 15, 50, TAR_CHAR_ROOM|TAR_FIGHT_VICT|TAR_VIOLENT, POSITION_FIGHTING, LOKI, CLASS_ALL, 3, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_CLONE, 1, 0, "clone", cast_clone, 48, 50, 100, TAR_CHAR_WORLD, POSITION_STANDING, LOKI, CLASS_ALL, 25, 48, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_COLOUR_SPRAY, 1, 0, "colour spray", cast_colour_spray, 24, 15, 50, TAR_IGNORE|TAR_VIOLENT, POSITION_FIGHTING, LOKI, CLASS_ALL, 11, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_CONE_OF_COLD, 1, 0, "cone of cold", cast_cone_of_cold, 24, 15, 50, TAR_IGNORE|TAR_VIOLENT, POSITION_FIGHTING, LOKI, CLASS_ALL, 11, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_CONJURE_ELEMENTAL, 1, 0, "conjure elemental", cast_conjure_elemental, 24, 30, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, 16, 14, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_CONTROL_WEATHER, 1, 0, "control weather", cast_control_weather, 36, 25, 50, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, 10, 13, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_CONT_LIGHT, 1, 0, "continual light", cast_cont_light, 24, 10, 50, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, 3, 4, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_CREATE_FOOD, 1, 0, "create food", cast_create_food, 12, 5, 50, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOW_IMMORTAL, 3, LOKI, LOKI, 11, LOKI );
ASSIGN_SPELL( SPELL_CREATE_WATER, 1, 0, "create water", cast_create_water, 12, 5, 50, TAR_OBJ_INV|TAR_OBJ_EQUIP, POSITION_STANDING, LOKI, CLASS_ALL, LOW_IMMORTAL, 2, LOKI, LOKI, 10, LOKI );
ASSIGN_SPELL( SPELL_CURE_BLIND, 1, 0, "cure blindness", cast_cure_blind, 12, 5, 50, TAR_CHAR_ROOM, POSITION_STANDING, LOKI, CLASS_ALL, LOW_IMMORTAL, 4, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_CURE_CRITIC, 1, 0, "cure critical", cast_cure_critic, 12, 11, 50, TAR_CHAR_ROOM, POSITION_FIGHTING, LOKI, CLASS_ALL, LOW_IMMORTAL, 9, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_CURE_LIGHT, 1, 0, "cure light", cast_cure_light, 12, 5, 50, TAR_CHAR_ROOM, POSITION_FIGHTING, LOKI, CLASS_ALL, LOW_IMMORTAL, 1, LOKI, LOKI, 10, LOKI );
ASSIGN_SPELL( SPELL_CURE_SERIOUS, 1, 0, "cure serious", cast_cure_serious, 12, 9, 50, TAR_CHAR_ROOM, POSITION_FIGHTING, LOKI, CLASS_ALL, 30, 7, LOKI, LOKI, 15, LOKI );
ASSIGN_SPELL( SPELL_CURSE, 1, 0, "curse", cast_curse, 24, 20, 50, TAR_CHAR_ROOM|TAR_OBJ_ROOM|TAR_OBJ_INV|TAR_OBJ_EQUIP|TAR_FIGHT_VICT|TAR_VIOLENT, POSITION_STANDING, LOKI, CLASS_ALL, 12, 12, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_DETECT_EVIL, 1, 0, "detect evil", cast_detect_evil, 12, 5, 50, TAR_CHAR_ROOM, POSITION_STANDING, LOKI, CLASS_ALL, LOW_IMMORTAL, 1, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_DETECT_INVISIBLE, 1, 0, "detect invisible", cast_detect_invisibility, 12, 5, 50, TAR_CHAR_ROOM, POSITION_STANDING, LOKI, CLASS_ALL, 2, 5, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_DETECT_MAGIC, 1, 0, "detect magic", cast_detect_magic, 12, 5, 50, TAR_CHAR_ROOM, POSITION_STANDING, LOKI, CLASS_ALL, 1, 3, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_DETECT_POISON, 1, 0, "detect poison", cast_detect_poison, 12, 5, 50, TAR_CHAR_ROOM|TAR_OBJ_INV|TAR_OBJ_EQUIP, POSITION_STANDING, LOKI, CLASS_ALL, LOW_IMMORTAL, 2, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_DISPEL_EVIL, 1, 0, "dispel evil", cast_dispel_evil, 36, 100, 100, TAR_CHAR_ROOM|TAR_FIGHT_VICT, POSITION_FIGHTING, LOKI, CLASS_ALL, LOW_IMMORTAL, 12, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_DISPEL_GOOD, 1, 0, "dispel good", cast_dispel_good, 36, 100, 100, TAR_CHAR_ROOM|TAR_FIGHT_VICT, POSITION_FIGHTING, LOKI, CLASS_ALL, LOW_IMMORTAL, 12, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_DISPEL_MAGIC, 1, 0, "dispel magic", cast_dispel_magic, 12, 15, 50, TAR_CHAR_ROOM|TAR_FIGHT_VICT, POSITION_FIGHTING, LOKI, CLASS_ALL, 6, 6, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_DRAGON_BREATH, 0, 0, "DRAGON BREATH", cast_dragon_breath, 0, 100, 100, TAR_IGNORE|TAR_VIOLENT, POSITION_STANDING, LOKI, CLASS_ALL, LOKI, LOKI, LOKI, LOKI, LOKI, LOKI );
#if 1
ASSIGN_SPELL( SPELL_EARTHQUAKE, 1, 0, "earthquake", cast_earthquake, 24, 15, 50, TAR_IGNORE|TAR_VIOLENT, POSITION_FIGHTING, LOKI, CLASS_ALL, LOW_IMMORTAL, 8, LOKI, LOKI, LOKI, LOKI );
#else
ASSIGN_SPELL( SPELL_EARTHQUAKE, 1, 0, "earthquake", cast_new_earthquake, 24, 15, 50, TAR_IGNORE|TAR_VIOLENT, POSITION_FIGHTING, LOKI, CLASS_ALL, LOW_IMMORTAL, 8, LOKI, LOKI, LOKI, LOKI );
#endif
ASSIGN_SPELL( SPELL_ENCHANT_WEAPON, 1, 0, "enchant weapon", cast_enchant_weapon, 48, 100, 100, TAR_OBJ_INV|TAR_OBJ_EQUIP, POSITION_STANDING, LOKI, CLASS_ALL, 9, 25, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_ENERGY_DRAIN, 1, 0, "energy drain", cast_energy_drain, 36, 35, 50, TAR_CHAR_ROOM|TAR_FIGHT_VICT|TAR_VIOLENT, POSITION_FIGHTING, LOKI, CLASS_ALL, 17, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_FAERIE_FIRE, 1, 0, "faerie fire", cast_faerie_fire, 12, 10, 50, TAR_CHAR_ROOM|TAR_SELF_NONO, POSITION_STANDING, LOKI, CLASS_ALL, 5, 3, LOKI, LOKI, 11, LOKI );
ASSIGN_SPELL( SPELL_FAERIE_FOG, 1, 0, "faerie fog", cast_faerie_fog, 24, 20, 50, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, 13, 10, LOKI, LOKI, 14, LOKI );
ASSIGN_SPELL( SPELL_FEAR, 1, 0, "fear", cast_fear, 12, 15, 50, TAR_CHAR_ROOM|TAR_FIGHT_VICT|TAR_VIOLENT, POSITION_FIGHTING, LOKI, CLASS_ALL, 8, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_FIREBALL, 1, 0, "fireball", cast_fireball, 36, 15, 50, TAR_IGNORE|TAR_VIOLENT, POSITION_FIGHTING, LOKI, CLASS_ALL, 15, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_FIRESHIELD, 1, 0, "fireshield", cast_fireshield, 24, 40, 50, TAR_SELF_ONLY|TAR_CHAR_ROOM, POSITION_STANDING, LOKI, CLASS_ALL, 20, 19, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_FLAMESTRIKE, 1, 0, "flamestrike", cast_flamestrike, 24, 15, 50, TAR_CHAR_ROOM|TAR_FIGHT_VICT|TAR_VIOLENT, POSITION_FIGHTING, LOKI, CLASS_ALL, LOW_IMMORTAL, 11, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_FLY, 1, 0, "fly", cast_flying, 12, 15, 50, TAR_CHAR_ROOM, POSITION_FIGHTING, LOKI, CLASS_ALL, 3, 14, LOKI, LOKI, 18, LOKI );
ASSIGN_SPELL( SPELL_FLY_GROUP, 1, 0, "group fly", cast_fly_group, 18, 30, 50, TAR_IGNORE, POSITION_FIGHTING, LOKI, CLASS_ALL, 8, 22, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_GOODBERRY, 1, 0, "goodberry", cast_goodberry, 12, 40, 80, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI,40, LOKI, LOKI, 25, LOKI ); 
ASSIGN_SPELL( SPELL_HARM, 1, 0, "harm", cast_harm, 36, 50, 100, TAR_CHAR_ROOM|TAR_FIGHT_VICT|TAR_VIOLENT, POSITION_FIGHTING, LOKI, CLASS_ALL, LOW_IMMORTAL, 17, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_HEAL, 1, 0, "heal", cast_heal, 18, 50, 100, TAR_CHAR_ROOM, POSITION_FIGHTING, LOKI, CLASS_ALL, LOW_IMMORTAL, 17, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_ICE_STORM, 1, 0, "ice storm", cast_ice_storm, 12, 15, 50, TAR_IGNORE|TAR_VIOLENT, POSITION_FIGHTING, LOKI, CLASS_ALL, 7, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_IDENTIFY, 0, 0, "IDENTIFY", cast_identify, 1, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, LOKI, LOKI, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_INFRAVISION, 1, 0, "infravision", cast_infravision, 12, 7, 50, TAR_CHAR_ROOM, POSITION_STANDING, LOKI, CLASS_ALL, 5, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_INVISIBLE, 1, 0, "invisibility", cast_invisibility, 12, 5, 50, TAR_CHAR_ROOM|TAR_OBJ_INV|TAR_OBJ_ROOM|TAR_OBJ_EQUIP, POSITION_STANDING, LOKI, CLASS_ALL, 4, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_KNOCK, 1, 0, "knock", cast_knock, 12, 10, 50, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, 3, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_KNOW_ALIGNMENT, 1, 0, "know alignment", cast_know_alignment, 12, 10, 50, TAR_CHAR_ROOM|TAR_FIGHT_VICT, POSITION_FIGHTING, LOKI, CLASS_ALL, 4, 3, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_LIGHT, 1, 0, "create light", cast_light, 12, 5, 50, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, 1, 2, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_LIGHTNING_BOLT, 1, 0, "lightning bolt", cast_lightning_bolt, 24, 15, 50, TAR_CHAR_ROOM|TAR_FIGHT_VICT|TAR_VIOLENT, POSITION_FIGHTING, LOKI, CLASS_ALL, 9, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_LOCATE_OBJECT, 1, 0, "locate object", cast_locate_object, 12, 20, 50, TAR_NAME, POSITION_STANDING, LOKI, CLASS_ALL, LOW_IMMORTAL, 4, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_MAGIC_MISSILE, 1, 0, "magic missile", cast_magic_missile, 12, 10, 50, TAR_CHAR_ROOM|TAR_FIGHT_VICT|TAR_VIOLENT, POSITION_FIGHTING, LOKI, CLASS_ALL, 1, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_METEOR_SWARM, 1, 0, "meteor swarm", cast_meteor_swarm, 24, 50, 50, TAR_IGNORE|TAR_VIOLENT, POSITION_FIGHTING, LOKI, CLASS_ALL, 20, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_MINOR_CREATE, 1, 0, "minor creation", cast_minor_creation, 24, 30, 50, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, 8, 14, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_MON_SUM_1, 1, 0, "monsum one", cast_mon_sum1, 24, 10, 50, TAR_IGNORE, POSITION_FIGHTING, LOKI, CLASS_ALL, 5, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_MON_SUM_2, 1, 0, "monsum two", cast_mon_sum2, 24, 12, 50, TAR_IGNORE, POSITION_FIGHTING, LOKI, CLASS_ALL, 7, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_MON_SUM_3, 1, 0, "monsum three", cast_mon_sum3, 24, 15, 50, TAR_IGNORE, POSITION_FIGHTING, LOKI, CLASS_ALL, 9, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_MON_SUM_4, 1, 0, "monsum four", cast_mon_sum4, 24, 17, 50, TAR_IGNORE, POSITION_FIGHTING, LOKI, CLASS_ALL, 11, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_MON_SUM_5, 1, 0, "monsum five", cast_mon_sum5, 24, 20, 50, TAR_IGNORE, POSITION_FIGHTING, LOKI, CLASS_ALL, 13, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_MON_SUM_6, 1, 0, "monsum six", cast_mon_sum6, 24, 22, 50, TAR_IGNORE, POSITION_FIGHTING, LOKI, CLASS_ALL, 15, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_MON_SUM_7, 1, 0, "monsum seven", cast_mon_sum7, 24, 25, 50, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, 17, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_PARALYSIS, 1, 0, "paralyze", cast_paralyze, 36, 40, 50, TAR_CHAR_ROOM|TAR_FIGHT_VICT|TAR_VIOLENT, POSITION_FIGHTING, LOKI, CLASS_ALL, 15, 15, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_POISON, 1, 0, "poison", cast_poison, 24, 10, 50, TAR_CHAR_ROOM|TAR_SELF_NONO|TAR_OBJ_INV|TAR_OBJ_EQUIP|TAR_OBJ_ROOM|TAR_FIGHT_VICT|TAR_VIOLENT, POSITION_FIGHTING, LOKI, CLASS_ALL, LOW_IMMORTAL, 8, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_POLY_SELF, 1, 0, "polymorph self", cast_poly_self, 12, 30, 50, TAR_IGNORE, POSITION_FIGHTING, LOKI, CLASS_ALL, 8, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_PROTECT_FROM_EVIL, 1, 0, "protection from evil", cast_protection_from_evil, 12, 5, 50, TAR_CHAR_ROOM, POSITION_STANDING, LOKI, CLASS_ALL, LOW_IMMORTAL, 6, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_REFRESH, 1, 0, "refresh", cast_refresh, 12, 5, 50, TAR_CHAR_ROOM, POSITION_STANDING, LOKI, CLASS_ALL, 3, 2, LOKI, LOKI, 11, LOKI );
ASSIGN_SPELL( SPELL_REMOVE_CURSE, 1, 0, "remove curse", cast_remove_curse, 12, 5, 50, TAR_CHAR_ROOM|TAR_OBJ_INV|TAR_OBJ_EQUIP|TAR_OBJ_ROOM, POSITION_STANDING, LOKI, CLASS_ALL, LOW_IMMORTAL, 7, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_REMOVE_PARALYSIS, 1, 0, "remove paralysis", cast_remove_paralysis, 12, 10, 50, TAR_CHAR_ROOM|TAR_FIGHT_VICT, POSITION_FIGHTING, LOKI, CLASS_ALL, LOW_IMMORTAL, 4, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_REMOVE_POISON, 1, 0, "remove poison", cast_remove_poison, 12, 5, 50, TAR_CHAR_ROOM|TAR_OBJ_INV|TAR_OBJ_ROOM, POSITION_STANDING, LOKI, CLASS_ALL, LOW_IMMORTAL, 5, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_SANCTUARY, 1, 0, "sanctuary", cast_sanctuary, 36, 50, 50, TAR_CHAR_ROOM, POSITION_STANDING, LOKI, CLASS_ALL, LOW_IMMORTAL, 19, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_SECOND_WIND, 1, 0, "second wind", cast_second_wind, 12, 5, 50, TAR_CHAR_ROOM, POSITION_STANDING, LOKI, CLASS_ALL, 12, 6, LOKI, LOKI, 16, LOKI );
ASSIGN_SPELL( SPELL_SENSE_LIFE, 1, 0, "sense life", cast_sense_life, 12, 5, 50, TAR_CHAR_ROOM, POSITION_STANDING, LOKI, CLASS_ALL, LOW_IMMORTAL, 7, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_SHELTER, 1, 0, "shelter", cast_shelter, 12, 100, 100, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, 10, 10, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_SHIELD, 1, 0, "shield", cast_shield, 24, 15, 50, TAR_CHAR_ROOM, POSITION_FIGHTING, LOKI, CLASS_ALL, 1, 15, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_SHOCKING_GRASP, 1, 0, "shocking grasp", cast_shocking_grasp, 12, 15, 50, TAR_CHAR_ROOM|TAR_FIGHT_VICT|TAR_VIOLENT, POSITION_FIGHTING, LOKI, CLASS_ALL, 1, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_SLEEP, 1, 0, "sleep", cast_sleep, 24, 15, 50, TAR_CHAR_ROOM|TAR_FIGHT_VICT, POSITION_STANDING, LOKI, CLASS_ALL, 3, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_STONE_SKIN, 1, 0, "stoneskin", cast_stone_skin, 24, 20, 50, TAR_CHAR_ROOM, POSITION_STANDING, LOKI, CLASS_ALL, 16, 32, LOKI, LOKI, 14, LOKI );
ASSIGN_SPELL( SPELL_STRENGTH, 1, 0, "strength", cast_strength, 12, 10, 50, TAR_CHAR_ROOM, POSITION_STANDING, LOKI, CLASS_ALL, 4, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_SUCCOR, 1, 0, "succor", cast_succor, 24, 15, 50, TAR_IGNORE, POSITION_STANDING, LOKI, CLASS_ALL, 21, 18, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_SUMMON, 1, 0, "summon", cast_summon, 36, 20, 50, TAR_CHAR_WORLD, POSITION_STANDING, LOKI, CLASS_ALL, 18, 16, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_TELEPORT, 1, 0, "teleport", cast_teleport, 12, 33, 50, TAR_CHAR_ROOM|TAR_FIGHT_VICT, POSITION_FIGHTING, LOKI, CLASS_ALL, 8, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_TRUE_SIGHT, 1, 0, "true sight", cast_true_seeing, 24, 20, 50, TAR_CHAR_ROOM, POSITION_STANDING, LOKI, CLASS_ALL, LOW_IMMORTAL, 12, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_TURN, 1, 0, "turn", cast_turn, 12, 5, 50, TAR_CHAR_ROOM, POSITION_STANDING, LOKI, CLASS_ALL, LOW_IMMORTAL, 1, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_VENTRILOQUATE, 1, 0, "ventriloquate", cast_ventriloquate, 12, 5, 50, TAR_CHAR_ROOM|TAR_OBJ_ROOM|TAR_SELF_NONO, POSITION_STANDING, LOKI, CLASS_ALL, 1, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_VISIONS, 1, 0, "visions", cast_visions, 12, 25, 40, TAR_CHAR_WORLD, POSITION_STANDING, LOKI, CLASS_ALL, LOKI, 15, LOKI, LOKI, 30, LOKI );
ASSIGN_SPELL( SPELL_WATER_BREATH, 1, 0, "water breath", cast_water_breath, 12, 15, 50, TAR_CHAR_ROOM, POSITION_FIGHTING, LOKI, CLASS_ALL, 4, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_WEAKNESS, 1, 0, "weakness", cast_weakness, 12, 10, 50, TAR_CHAR_ROOM|TAR_FIGHT_VICT|TAR_VIOLENT, POSITION_FIGHTING, LOKI, CLASS_ALL, 4, LOW_IMMORTAL, LOKI, LOKI, LOKI, LOKI );
ASSIGN_SPELL( SPELL_WORD_OF_RECALL, 1, 0, "word of recall", cast_word_of_recall, 12, 5, 50, TAR_CHAR_ROOM|TAR_SELF_ONLY, POSITION_FIGHTING, LOKI, CLASS_ALL, LOW_IMMORTAL, 10, LOKI, LOKI, LOKI, LOKI );

    for(i= 0; i< MAX_SKILLS; i++)
      if(!spell[i])
        spell[i]= my_strdup("unknown");
  }
  return spell[Spell];
}

char *damage_name(int Type) {
  if(Type < TYPE_HIT)
    switch(Type) {
      case 0: Type = TYPE_SMITE; break;
      case 1: Type = TYPE_STAB; break;
      case 2: Type = TYPE_WHIP; break;
      case 3: Type = TYPE_SLASH; break;
      case 4: Type = TYPE_SMASH; break;
      case 5: Type = TYPE_CLEAVE; break;
      case 6: Type = TYPE_CRUSH; break;
      case 7: Type = TYPE_BLUDGEON; break;
      case 8: Type = TYPE_CLAW; break;
      case 9: Type = TYPE_BITE; break;
      case 10: Type = TYPE_STING; break;
      case 11: Type = TYPE_PIERCE; break;
      default: Type = TYPE_HIT; break;
    }
  Type -= TYPE_HIT;
  switch(Type) {
    case 0: return "Hit";
    case 1: return "Bludgeon";
    case 2: return "Piercing";
    case 3: return "Slashing";
    case 4: return "Whip";
    case 5: return "Claw";
    case 6: return "Bite";
    case 7: return "Stinging";
    case 8: return "Crushing";
    case 9: return "Cleaving";
    case 10: return "Stabbing";
    case 11: return "Smashing";
    case 12: return "Smiting";
    default: return "unknown";
  }
}

int remap_mob_vnum(mobs *Mobs, int VNum) {
  register int i;

  for(i= 0; i< Mobs->Count; i++)
    if(Mobs->Mob[i].Number == VNum) return i;
  return -1;
}

char *mob_name(mobs *Mobs, int VNum) {
  int i;

  if((i= remap_mob_vnum(Mobs, VNum))!= -1)
    return Mobs->Mob[i].ShortDesc;
  else return "a non-existant mob";
}

void check_duplicate_mobs(mobs *Mobs) {
  register int i, tmp;

  if(!Quiet) {
    fprintf(stderr, "Checking for duplicate mob vnums...");
    fflush(stderr);
  }
  for(i= 0; i< Mobs->Count; i++) {
    if(!Quiet)
      spin(stderr);
    tmp= Mobs->Mob[i].Number;
    Mobs->Mob[i].Number = -1;
    if(remap_mob_vnum(Mobs, tmp) >= 0)
      if(Debug)
        bug("MOB [#%d] is DUPLICATED!!!", tmp);
    Mobs->Mob[i].Number = tmp;
  }
  if(!Quiet)
    fprintf(stderr, "done.\n");
}

void set_mob_zones(zones *Zones, mobs *Mobs) {
  register int i, j, CorrectZone;

  if(!Quiet) {
    fprintf(stderr, "Setting Zone id tags...");
    fflush(stderr);
  }
  for(i= 0; i< Mobs->Count; i++) {
    if(!Quiet)
      spin(stderr);
    CorrectZone= -1;
    for(j= 0; j< Zones->Count; j++)
      if(Mobs->Mob[i].Number <= Zones->Zone[j].Top)
        if(Mobs->Mob[i].Number >= ((j>0)?Zones->Zone[j-1].Top+1:0)) {
          CorrectZone= Zones->Zone[j].Number;
          break;
        }
    if(CorrectZone == -1) {
      if(Verbose)
        bug("Holy BatTurds!  I have NO IDEA what zone this mob [#%d] "
            "belongs to!\nPutting it in Zone %d...\n",
            Mobs->Mob[i].Number, Zones->Zone[0].Number);
      Mobs->Mob[i].Zone= Zones->Zone[0].Number;
    } else {
      Mobs->Mob[i].Zone= CorrectZone;
    }
  }
  if(!Quiet)
    fprintf(stderr, "done.\n");
}

void check_mob_zone_mismatch(zones *Zones, mobs *Mobs) {
  register int i, CorrectZone;

  if(!Quiet) {
    fprintf(stderr, "Verifying proper Zone id tags...");
    fflush(stderr);
  }
  for(i= 0; i< Mobs->Count; i++) {
    if(!Quiet)
      spin(stderr);
    CorrectZone= Mobs->Mob[i].Number / 100;
    if(Mobs->Mob[i].Zone != CorrectZone) {
      bug("Improper Zone ID of [#%d] for mob \"%s\" [#%d],\n"
          "Should belong to Zone [#%d].\n",
          Mobs->Mob[i].Zone, Mobs->Mob[i].Name,
          Mobs->Mob[i].Number, CorrectZone);
      /* Mobs->Mob[i].Zone= CorrectZone; */
    }
  }
  if(!Quiet)
    fprintf(stderr, "done.\n");
}

/*
 * This routine loads the mob file from WileyMUD III format.
 */
mobs *load_mobs(char *infile, zones *Zones) {
  FILE *ifp;
  vnum_index *MobIndex;
  mobs *Mobs;
  char *tmp; /* , tmp2[MAX_LINE_LEN], *tmp3; */
  char tmpa[MAX_LINE_LEN], tmpb[MAX_LINE_LEN], tmpc[MAX_LINE_LEN],
       tmpd[MAX_LINE_LEN], tmpe[MAX_LINE_LEN], tmpf[MAX_LINE_LEN];
  register int i, j;
  int ack;
  long Line, Pos;

  if(!infile || !*infile) return NULL;
  if(!(MobIndex= make_index(infile, NULL))) return NULL;
  Mobs= (mobs *)get_mem(1, sizeof(mobs));
  bzero(Mobs, sizeof(mobs));
  Mobs->Count= MobIndex->Count;
  Mobs->Mob= (mob *)get_mem(Mobs->Count, sizeof(mob));
  bzero(Mobs->Mob, Mobs->Count* sizeof(mob));
  ifp= open_file(infile, "r");

  if(Verbose)
    fprintf(stderr, "Parsing Mob file...\n");
  else if(!Quiet) {
    fprintf(stderr, "Parsing Mob file...");
    fflush(stderr);
  }

  for(i= 0; i< Mobs->Count; i++) {
    Line= MobIndex->VNum[i].Line;
    Pos= MobIndex->VNum[i].Pos;
    Mobs->Mob[i].Number= MobIndex->VNum[i].Number;
    if(fseek(ifp, Pos, 0) < 0)
      fatal("Cannot load Mob $%d from %s!",
          Mobs->Mob[i].Number, infile);
    if(!(tmp= get_line(ifp, &Line, &Pos, 1)))
      fatal("Cannot get vnum for Mob #%d from %s!",
          Mobs->Mob[i].Number, infile);
    if((!(tmp= get_tilde_string(ifp, &Line, &Pos)))||
       (!(Mobs->Mob[i].Name= my_strdup(tmp))))
      fatal("Cannot get name for Mob #%d, %s at %d.",
          Mobs->Mob[i].Number, infile, Line);

    if(Verbose)
      fprintf(stderr, "  (%d) Parsing \"%s\"[#%d]...\n", i,
              Mobs->Mob[i].Name, Mobs->Mob[i].Number);
    else if(!Quiet)
      spin(stderr);

    if((!(tmp= get_tilde_string(ifp, &Line, &Pos)))||
       (!(Mobs->Mob[i].ShortDesc= my_strdup(tmp))))
      fatal("Cannot get short description for Mob #%d, %s at %d.",
          Mobs->Mob[i].Number, infile, Line);

    if((!(tmp= get_tilde_string(ifp, &Line, &Pos)))||
       (!(Mobs->Mob[i].LongDesc= my_strdup(tmp))))
      fatal("Cannot get long description for Mob #%d, %s at %d.",
          Mobs->Mob[i].Number, infile, Line);

    if((!(tmp= get_tilde_string(ifp, &Line, &Pos)))||
       (!(Mobs->Mob[i].Description= my_strdup(tmp))))
      fatal("Cannot get description for Mob #%d, %s at %d.",
          Mobs->Mob[i].Number, infile, Line);

    if((!(tmp= get_line(ifp, &Line, &Pos, 1)))||
       ((ack = sscanf(tmp, " %d %d %d %c %d ", &(Mobs->Mob[i].ActFlags),
               &(Mobs->Mob[i].AffectedBy), &(Mobs->Mob[i].Alignment),
               &(Mobs->Mob[i].Type), &(Mobs->Mob[i].AttackCount))) < 4))
      fatal("Cannot get main flags for Mob #%d, %s at %d.",
          Mobs->Mob[i].Number, infile, Line);

    Mobs->Mob[i].ActFlags |= ACT_ISNPC;
    Mobs->Mob[i].Class = CLASS_WARRIOR;

    switch(Mobs->Mob[i].Type) {
      case 'W':
      case 'M':
      case 'S': {
        /* multi-attack value is grabbed by sscanf above, ack will equal 5 if present */
        Mobs->Mob[i].Strength.Modifier = 14;
        Mobs->Mob[i].Dexterity.Modifier = 14;
        Mobs->Mob[i].Constitution.Modifier = 14;
        Mobs->Mob[i].Intelligence.Modifier = 14;
        Mobs->Mob[i].Wisdom.Modifier = 14;

        if((!(tmp= get_line(ifp, &Line, &Pos, 1)))||
           (sscanf(tmp, " %d %d %d %s %s ", &(Mobs->Mob[i].Level),
                   &(Mobs->Mob[i].ToHit), &(Mobs->Mob[i].ArmourClass),
                   tmpa, tmpb) != 5))
          fatal("Cannot get level flags for Mob #%d, %s at %d.",
              Mobs->Mob[i].Number, infile, Line);
        sscanf_dice(tmpa, &(Mobs->Mob[i].HitPoints.Rolls),
                    &(Mobs->Mob[i].HitPoints.Die),
                    &(Mobs->Mob[i].HitPoints.Modifier));
        if(ack == 5) { /* an attack count was read in... must be > 0 */
          if(Mobs->Mob[i].AttackCount < 1)
            Mobs->Mob[i].AttackCount= 1;
        } else {
          Mobs->Mob[i].AttackCount= 1;
        }
        Mobs->Mob[i].Attack= (attack *)get_mem(Mobs->Mob[i].AttackCount, sizeof(attack));
        bzero(Mobs->Mob[i].Attack, Mobs->Mob[i].AttackCount * sizeof(attack));

        sscanf_dice(tmpb, &(Mobs->Mob[i].Attack[0].Rolls),
                    &(Mobs->Mob[i].Attack[0].Die),
                    &(Mobs->Mob[i].Attack[0].Modifier));
        Mobs->Mob[i].Attack[0].Type = 0;
        for(j= 1; j < Mobs->Mob[i].AttackCount; j++)
          Mobs->Mob[i].Attack[j]= Mobs->Mob[i].Attack[0];
        Mobs->Mob[i].ManaPoints = 100;
        Mobs->Mob[i].MovePoints = 100;

        if((!(tmp= get_line(ifp, &Line, &Pos, 1)))||
           (sscanf(tmp, " %d %d %d %d ", &(Mobs->Mob[i].Gold.Modifier),
                   &(Mobs->Mob[i].Gold.Rolls), &(Mobs->Mob[i].Experience.Modifier),
                   &(Mobs->Mob[i].Race)) < 2))
          fatal("Cannot get experience flags for Mob #%d, %s at %d.",
              Mobs->Mob[i].Number, infile, Line);

        if(Mobs->Mob[i].Gold.Modifier < 0) {
          Mobs->Mob[i].Gold.Modifier = Mobs->Mob[i].Gold.Rolls;
          Mobs->Mob[i].Gold.Rolls = 0;
        } else {
          Mobs->Mob[i].Experience.Modifier = Mobs->Mob[i].Gold.Rolls;
          Mobs->Mob[i].Gold.Rolls = 0;
          Mobs->Mob[i].Race = 0;
        }

        if((!(tmp= get_line(ifp, &Line, &Pos, 1)))||
           (sscanf(tmp, " %d %d %d %d %d %d ", &(Mobs->Mob[i].Position),
                   &(Mobs->Mob[i].DefaultPosition), &(Mobs->Mob[i].Sex),
                   &(Mobs->Mob[i].Resistance), &(Mobs->Mob[i].Immunity),
                   &(Mobs->Mob[i].Susceptible)) < 3))
          fatal("Cannot get position flags for Mob #%d, %s at %d.",
              Mobs->Mob[i].Number, infile, Line);
        if(Mobs->Mob[i].Sex < 3) {
          Mobs->Mob[i].Resistance=
          Mobs->Mob[i].Immunity=
          Mobs->Mob[i].Susceptible= 0;
        } else if(Mobs->Mob[i].Sex < 6) {
          Mobs->Mob[i].Sex -= 3;
        } else {
          Mobs->Mob[i].Sex=
          Mobs->Mob[i].Resistance=
          Mobs->Mob[i].Immunity=
          Mobs->Mob[i].Susceptible= 0;
        }

        Mobs->Mob[i].Weight = 250;
        Mobs->Mob[i].Height = 198;
        for(j= 0; j < 5; j++)
          Mobs->Mob[i].SavingThrow[j]= max(20 - Mobs->Mob[i].Level, 2);

        if(Mobs->Mob[i].Type == 'W') {
         if((!(tmp= get_tilde_string(ifp, &Line, &Pos)))||
            (!(Mobs->Mob[i].Sound= my_strdup(tmp))))
           fatal("No local sound for Mob #%d, %s of %d.",
               Mobs->Mob[i].Number, infile, Line);
         if((!(tmp= get_tilde_string(ifp, &Line, &Pos)))||
            (!(Mobs->Mob[i].DistantSound= my_strdup(tmp))))
           fatal("No local sound for Mob #%d, %s of %d.",
               Mobs->Mob[i].Number, infile, Line);
        }
      } break;
      case 'D': {
        fatal("Type 'D' mob #%d, %s of %d are no longer supported.",
              Mobs->Mob[i].Number, infile, Line);
      } break;
      case 'C': {
        if((!(tmp= get_line(ifp, &Line, &Pos, 1)))||
           (sscanf(tmp, " %d %d %d %d %d %s ", &(Mobs->Mob[i].Race),
                   &(Mobs->Mob[i].Class), &(Mobs->Mob[i].Sex),
                   &(Mobs->Mob[i].Height), &(Mobs->Mob[i].Weight),
                   tmpa) != 6))
          fatal("Cannot get race flags for Mob #%d, %s at %d.",
              Mobs->Mob[i].Number, infile, Line);
        sscanf_dice(tmpa, &(Mobs->Mob[i].Gold.Rolls),
                    &(Mobs->Mob[i].Gold.Die),
                    &(Mobs->Mob[i].Gold.Modifier));
        if((!(tmp= get_line(ifp, &Line, &Pos, 1)))||
           (sscanf(tmp, " %s %d %s %d %d %d ", tmpa,
                   &(Mobs->Mob[i].Level), tmpb,
                   &(Mobs->Mob[i].ArmourClass), &(Mobs->Mob[i].ToHit),
                   &(Mobs->Mob[i].AttackCount)) != 6))
          fatal("Cannot get experience flags for Mob #%d, %s at %d.",
              Mobs->Mob[i].Number, infile, Line);
        sscanf_dice(tmpa, &(Mobs->Mob[i].Experience.Rolls),
                    &(Mobs->Mob[i].Experience.Die),
                    &(Mobs->Mob[i].Experience.Modifier));
        sscanf_dice(tmpb, &(Mobs->Mob[i].HitPoints.Rolls),
                    &(Mobs->Mob[i].HitPoints.Die),
                    &(Mobs->Mob[i].HitPoints.Modifier));
        Mobs->Mob[i].ManaPoints= 100;
        Mobs->Mob[i].MovePoints= 100;
        /* Mobs->Mob[i].ArmourClass *= 10; */
        /* Mobs->Mob[i].ToHit = 20 - Mobs->Mob[i].ToHit; */
        if(Mobs->Mob[i].AttackCount < 1)
          Mobs->Mob[i].AttackCount = 1;
        Mobs->Mob[i].Attack= (attack *)get_mem(Mobs->Mob[i].AttackCount, sizeof(attack));
        bzero(Mobs->Mob[i].Attack, Mobs->Mob[i].AttackCount * sizeof(attack));
        for(j= 0; j < Mobs->Mob[i].AttackCount; j++) {
          if((!(tmp= get_line(ifp, &Line, &Pos, 1)))||
             (sscanf(tmp, " %s %d ", tmpa,
                     &(Mobs->Mob[i].Attack[j].Type)) != 2))
            fatal("Cannot get attack line %d for Mob #%d, %s at %d.",
                j, Mobs->Mob[i].Number, infile, Line);
          sscanf_dice(tmpa, &(Mobs->Mob[i].Attack[j].Rolls),
                      &(Mobs->Mob[i].Attack[j].Die),
                      &(Mobs->Mob[i].Attack[j].Modifier));
        }
        if((!(tmp= get_line(ifp, &Line, &Pos, 1)))||
           (sscanf(tmp, " %d %d %d ", &(Mobs->Mob[i].Immunity),
                   &(Mobs->Mob[i].Resistance), &(Mobs->Mob[i].Susceptible)) != 3))
          fatal("Cannot get immunity flags for Mob #%d, %s at %d.",
              Mobs->Mob[i].Number, infile, Line);
        if((!(tmp= get_line(ifp, &Line, &Pos, 1)))||
           (sscanf(tmp, " %s %s %s %s %s %s ", tmpa, tmpb, tmpc, tmpd, tmpe, tmpf) != 6))
          fatal("Cannot get stat flags for Mob #%d, %s at %d.",
              Mobs->Mob[i].Number, infile, Line);
        sscanf_dice(tmpa, &(Mobs->Mob[i].Strength.Rolls),
                    &(Mobs->Mob[i].Strength.Die),
                    &(Mobs->Mob[i].Strength.Modifier));
        sscanf_dice(tmpa, &(Mobs->Mob[i].ExtraStrength.Rolls),
                    &(Mobs->Mob[i].ExtraStrength.Die),
                    &(Mobs->Mob[i].ExtraStrength.Modifier));
        sscanf_dice(tmpa, &(Mobs->Mob[i].Dexterity.Rolls),
                    &(Mobs->Mob[i].Dexterity.Die),
                    &(Mobs->Mob[i].Dexterity.Modifier));
        sscanf_dice(tmpa, &(Mobs->Mob[i].Constitution.Rolls),
                    &(Mobs->Mob[i].Constitution.Die),
                    &(Mobs->Mob[i].Constitution.Modifier));
        sscanf_dice(tmpa, &(Mobs->Mob[i].Intelligence.Rolls),
                    &(Mobs->Mob[i].Intelligence.Die),
                    &(Mobs->Mob[i].Intelligence.Modifier));
        sscanf_dice(tmpa, &(Mobs->Mob[i].Wisdom.Rolls),
                    &(Mobs->Mob[i].Wisdom.Die),
                    &(Mobs->Mob[i].Wisdom.Modifier));
        if((!(tmp= get_line(ifp, &Line, &Pos, 1)))||
           (sscanf(tmp, " %d %d %d %d %d ", &(Mobs->Mob[i].SavingThrow[0]),
                   &(Mobs->Mob[i].SavingThrow[1]), &(Mobs->Mob[i].SavingThrow[2]),
                   &(Mobs->Mob[i].SavingThrow[3]), &(Mobs->Mob[i].SavingThrow[4])) != 5))
          fatal("Cannot get saving throws for Mob #%d, %s at %d.",
              Mobs->Mob[i].Number, infile, Line);
        for(j= 0; j< 5; j++)
          if(!Mobs->Mob[i].SavingThrow[j])
            Mobs->Mob[i].SavingThrow[j] = max(20 - Mobs->Mob[i].Level, 2);
        if((!(tmp= get_line(ifp, &Line, &Pos, 1)))||
           (sscanf(tmp, " %d %d %d %d ", &(Mobs->Mob[i].Position),
                   &(Mobs->Mob[i].DefaultPosition), &ack, &(Mobs->Mob[i].SkillCount)) < 2))
          fatal("Cannot get position flags for Mob #%d, %s at %d.",
              Mobs->Mob[i].Number, infile, Line);

        if(ack) {
         if((!(tmp= get_tilde_string(ifp, &Line, &Pos)))||
            (!(Mobs->Mob[i].Sound= my_strdup(tmp))))
           fatal("No local sound for Mob #%d, %s of %d.",
               Mobs->Mob[i].Number, infile, Line);
         if((!(tmp= get_tilde_string(ifp, &Line, &Pos)))||
            (!(Mobs->Mob[i].DistantSound= my_strdup(tmp))))
           fatal("No local sound for Mob #%d, %s of %d.",
               Mobs->Mob[i].Number, infile, Line);
        }
        if(Mobs->Mob[i].SkillCount) {
          Mobs->Mob[i].Skill= (skill *)get_mem(Mobs->Mob[i].SkillCount, sizeof(skill));
          bzero(Mobs->Mob[i].Skill, Mobs->Mob[i].SkillCount * sizeof(skill));
          for(j= 0; j < Mobs->Mob[i].SkillCount; j++) {
            if((!(tmp= get_line(ifp, &Line, &Pos, 1)))||
               (sscanf(tmp, " %d %d %d ", &(Mobs->Mob[i].Skill[j].Number),
                       &(Mobs->Mob[i].Skill[j].Learned), &(Mobs->Mob[i].Skill[j].Recognise)) != 3))
              fatal("Cannot get skill line %d for Mob #%d, %s at %d.",
                  j, Mobs->Mob[i].Number, infile, Line);
          }
        }
      } break;
      default:
        fatal("I have NO clue what kind of mob #%d, %s of %d is.",
              Mobs->Mob[i].Number, infile, Line);
        break;
    }
  }
  if(Verbose)
    fprintf(stderr, "Parsed %d mobs in World.\n", Mobs->Count);
  else if(!Quiet) {
    fprintf(stderr, "done.\n");
  }
  fclose(ifp);
  check_duplicate_mobs(Mobs);
  set_mob_zones(Zones, Mobs);
  check_mob_zone_mismatch(Zones, Mobs);
  return Mobs;
}

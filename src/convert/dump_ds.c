#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "include/structs.h"
#include "include/main.h"
#include "include/utils.h"

#include "include/parse_wiley.h"

#define _DUMP_DS_C
#include "include/dump_ds.h"

char *DS_WelcomeMsg =
"Welcome to the WileyMUD III conversion project!\n"
"\n"
"  The easiest way to get your old WileyMUD area into the Final-Realms mudlib\n"
"is to log into your FR mud with a lord and use the new_domain command to add\n"
"the wileymud zones.  You can either create a single mega-domain like /d/wiley,\n"
"and then place each zone's output into its own subdirectory, or keep each zone\n"
"seperate as a domain in itself.\n"
"\n"
"  Once your domains have been added, you can safely copy over the directories\n"
"into their new hierarchy.\n"
"\n"
"Good Luck!                                  -Dread Quixadhal, Dark Lord of VI.\n"
"\n";

char *DS_ZoneDataMsg =
"Original Zone Information:\n"
"#ZONEDATA\n"
"Name\t%s~\n"
"Author\tThe Wiley Gang~\n"
"VNUMs\t%d %d\n"
"Time\t%d\n"
"Mode\t%d\n"
"End\n\n";

char *DS_ArmourMsg =
"%s/\n"
"\n"
"This is the directory where all armour objects are defined for the\n"
"%s domain.  Only armour should go in here!\n";

char *DS_ObjMsg =
"%s/\n"
"\n"
"This is the directory for things that don't have anywhere else to live\n"
"in the %s domain.  Especially generic objects...\n";

char *DS_MonsterMsg =
"%s/\n"
"\n"
"This is the directory where all NPC's are defined for the\n"
"%s domain.  I would suggest you group your\n"
"mobs by areas they roam in and/or activity cycles.  I may code something\n"
"for random mob generation by time of day, season, and environment...\n"
"more on this later.\n";

char *DS_RoomMsg =
"%s/\n"
"\n"
"This is the directory where all rooms are defined for the\n"
"%s domain.  You should try to layout your\n"
"rooms logically, rooms that belong together should be in subdirectories\n"
"IE: a long street, the interior of a multi-room building, a park, a level\n"
"of catacombs...\n";

char *DS_WeaponMsg =
"%s/\n"
"\n"
"This is the directory where all weapon objects are defined for the\n"
"%s domain.  Only weapons should go in here!\n";

#define sub_dir(x,m,n) { \
  if(!strcmp("",x)) \
    sprintf(muddir, "%s%s%s%s", DS_DOMAIN, OneBigDomain?DS_MEGA_DOMAIN:"", \
            OneBigDomain?"":"/", OneBigDomain?"":domainname); \
  else \
    sprintf(muddir, "%s%s%s%s%s%s", DS_DOMAIN, OneBigDomain?DS_MEGA_DOMAIN:"", \
            OneBigDomain?"/"x:"", *domainname?"/":"", domainname, OneBigDomain?"":"/"x); \
  sprintf(filename, "mkdir -p %s/%s%s", OutputDir, DS_SUBDIR, muddir); \
  system(filename); \
  if(m) { \
    sprintf(filename, "%s/%s%s/README", OutputDir, DS_SUBDIR, muddir);\
    ofp= open_file(filename, "w"); \
    if(n) { \
      fprintf(ofp, "%s", m, muddir, n); \
    } else { \
      fprintf(ofp, "%s", m); \
    } \
    fclose(ofp); \
  } \
}

#define dump_msg(id, name) { \
    if(Verbose) \
        fprintf(stderr, "Dump of Domain \"%s\"...\n", name); \
    else if(!Quiet) { \
        sprintf(tmpstr, "#%d Dump of Domain \"%s\"...", id+1, name); \
        fprintf(stderr, "%s", tmpstr); \
        for(x= strlen(tmpstr); x< 79; x++) \
            fprintf(stderr, " "); \
        for(x= strlen(tmpstr); x< 79; x++) \
            fprintf(stderr, "\b"); \
        fflush(stderr); \
    } \
}

void setup_dirs(zones *Zones, rooms *Rooms) {
  register int i,j;
  FILE *ofp;
  char domainname[256], muddir[256], filename[256];
  int LowRoom, HighRoom;

  if(!Quiet) {
    fprintf(stderr, "Setting up directory layout...");
    fflush(stderr);
  }
  bzero(domainname, 256);
  bzero(muddir, 256);
  bzero(filename, 256);
  if(OneBigDomain) {
    if(!Quiet) spin(stderr);
    sub_dir("", DS_WelcomeMsg, NULL);

    if(!Quiet) spin(stderr);
    sub_dir("armor", DS_ArmourMsg, "wiley");

    if(!Quiet) spin(stderr);
    sub_dir("obj", DS_ObjMsg, "wiley");

    if(!Quiet) spin(stderr);
    sub_dir("monsters", DS_MonsterMsg, "wiley");

    if(!Quiet) spin(stderr);
    sub_dir("rooms", DS_RoomMsg, "wiley");

    if(!Quiet) spin(stderr);
    sub_dir("weapons", DS_WeaponMsg, "wiley");
  } else for(i= 0; i< Zones->Count; i++) {
    if(!Quiet) spin(stderr);
    sub_dir("", DS_WelcomeMsg, NULL);

    if(!Quiet) spin(stderr);
    sub_dir("armor", DS_ArmourMsg, domainname);

    if(!Quiet) spin(stderr);
    sub_dir("obj", DS_ObjMsg, domainname);

    if(!Quiet) spin(stderr);
    sub_dir("monsters", DS_MonsterMsg, domainname);

    if(!Quiet) spin(stderr);
    sub_dir("rooms", DS_RoomMsg, domainname);

    if(!Quiet) spin(stderr);
    sub_dir("weapons", DS_WeaponMsg, domainname);
  }
  for(i= 0; i< Zones->Count; i++) {
    LowRoom = INT_MAX;
    HighRoom = INT_MIN;
    for(j= 0; j< Rooms->Count; j++) {
      if((remap_zone_vnum(Zones, Rooms->Room[j].Zone) == i) ||
         ((Rooms->Room[j].Number >= (!i? 0: Zones->Zone[i-1].Top+1)) &&
          (Rooms->Room[j].Number <= Zones->Zone[i].Top)
      )) {
        LowRoom= min(LowRoom, Rooms->Room[j].Number);
        HighRoom= max(HighRoom, Rooms->Room[j].Number);
      }
    }
    sprintf(domainname, "%s_%d", remap_name(Zones->Zone[i].Name), Zones->Zone[i].Number);
    if(!Quiet) spin(stderr);
    sub_dir("rooms", NULL, NULL);
    sprintf(filename, "%s/%s%s/README", OutputDir, DS_SUBDIR, muddir);
    ofp= open_file(filename, "a");
    fprintf(ofp, DS_ZoneDataMsg, Zones->Zone[i].Name, LowRoom, HighRoom,
            Zones->Zone[i].Time, Zones->Zone[i].Mode);
    fclose(ofp);
  }
  if(!Quiet)
    fprintf(stderr, "done.\n");
}

void dump_as_dead_souls(zones *Zones, rooms *Rooms, shops *Shops) {
  FILE *ofp = NULL;
  char filename[256], domainname[256], muddir[256];
  char roomname[256], outpath[256], tmpstr[256];
  int i, j, k, x, y;
  /* int LastMob, LastLoc; */
  char *TmpDesc, *HackDesc, *BigHack;

  setup_dirs(Zones, Rooms);

  for(i= 0; i< Zones->Count; i++) {
    sprintf(domainname, "%s_%d", remap_name(Zones->Zone[i].Name), Zones->Zone[i].Number);
   
    dump_msg(i+1, domainname);
    sub_dir("rooms");
    for(j= 0; j< Rooms->Count; j++) {
      if((remap_zone_vnum(Zones, Rooms->Room[j].Zone) == i) ||
         ((Rooms->Room[j].Number >= (!i? 0: Zones->Zone[i-1].Top+1)) &&
          (Rooms->Room[j].Number <= Zones->Zone[i].Top)
      )) {
/*
        long OldValue = 0,
             NewValue = 0;
*/

        sprintf(roomname, "%s_%d.c", remap_name(Rooms->Room[j].Name), Rooms->Room[j].Number);
        sprintf(filename, "%s/%s%s/%s", OutputDir, DS_SUBDIR, muddir, roomname);
        ofp= open_file(filename, "w");

        if(Verbose)
          fprintf(stderr, "Dumping %s\n", filename);
        else if(!Quiet)
          spin(stderr);

        fprintf(ofp, "// Automated conversion of WileyMUD by Quixadhal\n");
        fprintf(ofp, "// Original:   WileyMUD III, Room [#%d]\n",
                Rooms->Room[j].Number);
        fprintf(ofp, "// Target:     Dead Souls 3.0, %s/%s\n", muddir, roomname);
        fprintf(ofp, "// Performed:  %s\n", timestamp());
        fprintf(ofp, "\n");

/*
 * Now we are ready to do the actual converstion.... rooms only for now.
 */

        fprintf(ofp, "#include <lib.h>\n");
        fprintf(ofp, "inherit LIB_ROOM;\n");

        fprintf(ofp, "static void create() {\n");
        fprintf(ofp, "    room::create();\n");

        if((Rooms->Room[j].Flags & ROOM_INDOORS) ||
           (Rooms->Room[j].Sector == SECT_INDOORS))
          fprintf(ofp, "    SetClimate(\"indoors\");\n");
        else
          fprintf(ofp, "    SetClimate(\"outdoors\");\n");
        fprintf(ofp, "\n");
        //fprintf(ofp, "    set_zone(\"%s\");\n", domainname);
        fprintf(ofp, "    SetShort(\"%s\");\n", Rooms->Room[j].Name);

/*
 * This needs work..... /std/not_allowed is a start...
 *
 *      if(Rooms->Room[j].Flags & (ROOM_NOATTACK|ROOM_NOSTEAL|ROOM_NOSUMMON|ROOM_NOMAGIC)) {
 *        fprintf(ofp, "void init() {\n");
 *        fprintf(ofp, "    ::init();\n");
 *        if(Rooms->Room[j].Flags & ROOM_NOATTACK)
 *          fprintf(ofp, "    add_static_property(\"no attack\", 1);\n");
 *        if(Rooms->Room[j].Flags & ROOM_NOSTEAL)
 *          fprintf(ofp, "    add_static_property(\"no steal\", 1);\n");
 *        if(Rooms->Room[j].Flags & ROOM_NOSUMMON)
 *          fprintf(ofp, "    add_static_property(\"no teleport\", 1);\n");
 *        if(Rooms->Room[j].Flags & ROOM_NOMAGIC)
 *          fprintf(ofp, "    add_static_property(\"no magic\", 1);\n");
 *        fprintf(ofp, "}\n");
 *        fprintf(ofp, "\n");
 *      }
 */
    

/*
 * ARGH!  We have to escape all " characters inside our descriptions because
 * we're now using LPC....  HACK ALERT!
 */
        BigHack = my_strdup(Rooms->Room[j].Description);
        for(x= y= 0; x< strlen(BigHack); x++)
          if(BigHack[x] == '\"')
            y++;
        TmpDesc = get_mem(strlen(BigHack)+y+1, sizeof(char));
        bzero(TmpDesc, strlen(BigHack)+y+1);
        for(x= y= 0; x< strlen(BigHack); x++, y++) {
          if(BigHack[x] == '\"')
            TmpDesc[y++]= '\\';
          TmpDesc[y]= BigHack[x];
        }
        free(BigHack);
        fprintf(ofp, "    SetLong(");
        if(IncludeShortInLong) {
          fprintf(ofp, " \"%s\\n\"\n       ", Rooms->Room[j].Name);
        }
        if(!(HackDesc = (char *)strtok(TmpDesc, "\n")))
          fprintf(ofp, " \"%s\\n\");\n", TmpDesc);
        else {
          fprintf(ofp, " \"%s\"\n", HackDesc);
          while((HackDesc = (char *)strtok(NULL, "\n")))
            if(HackDesc) {
              if(*HackDesc == ' ')
                fprintf(ofp, "        \"\\n%s\"\n", HackDesc);
              else
                fprintf(ofp, "        \" %s\"\n", HackDesc);
            }
        }
        fprintf(ofp, "        \"\\n\" );\n");
        free(TmpDesc);
        fprintf(ofp, "\n");

        if(Rooms->Room[j].Flags & ROOM_DARK)
          fprintf(ofp, "    set_light(%d); // Normally dark.  If really PITCH BLACK, use 0.\n",
                  PitchBlack?0:5);
        else if(Rooms->Room[j].Flags & ROOM_INDOORS)
          fprintf(ofp, "    set_light(80);\n");
        else switch(Rooms->Room[j].Sector) {
          case SECT_INDOORS:
          case SECT_CITY:
            fprintf(ofp, "    set_light(80);\n");
            break;
          case SECT_FOREST:
            fprintf(ofp, "    set_light(60);\n");
            break;
          case SECT_AIR:
            fprintf(ofp, "    set_light(120);\n");
            break;
          case SECT_UNDERWATER:
            fprintf(ofp, "    set_light(20);\n");
            break;
          default:
            fprintf(ofp, "    set_light(100);\n");
            break;
        }
    
        for(k= 0; k< Rooms->Room[j].ExtraCount; k++) {
          if(Rooms->Room[j].Extra[k].Keyword->Count > 0) {
            fprintf(ofp, "    add_item( ");
            if(Rooms->Room[j].Extra[k].Keyword->Count > 1)
              fprintf(ofp, "({ \"%s\"",
                      Rooms->Room[j].Extra[k].Keyword->Word[0]);
            else
              fprintf(ofp, "\"%s\"",
                      Rooms->Room[j].Extra[k].Keyword->Word[0]);
            for(x= 1; x< Rooms->Room[j].Extra[k].Keyword->Count; x++)
              fprintf(ofp, ", \"%s\"", Rooms->Room[j].Extra[k].Keyword->Word[x]);
            if(Rooms->Room[j].Extra[k].Keyword->Count > 1)
              fprintf(ofp, " }),\n");
            else
              fprintf(ofp, " ,\n");
    
/*
 * ARGH!  We have to escape all " characters inside our descriptions because
 * we're now using LPC....  HACK ALERT!
 */
            BigHack = my_strdup(Rooms->Room[j].Extra[k].Description);
            for(x= y= 0; x< strlen(BigHack); x++)
              if(BigHack[x] == '\"')
                y++;
            TmpDesc = get_mem(strlen(BigHack)+y+1, sizeof(char));
            bzero(TmpDesc, strlen(BigHack)+y+1);
            for(x= y= 0; x< strlen(BigHack); x++, y++) {
              if(BigHack[x] == '\"')
                TmpDesc[y++]= '\\';
              TmpDesc[y]= BigHack[x];
            }
            free(BigHack);
            if(!(HackDesc = (char *)strtok(TmpDesc, "\n")))
              fprintf(ofp, "        \"%s\"\n", TmpDesc);
            else {
              fprintf(ofp, "        \"%s\"\n", HackDesc);
              while((HackDesc = (char *)strtok(NULL, "\n")))
                if(HackDesc) {
                  if(*HackDesc == ' ')
                    fprintf(ofp, "        \"\\n%s\"\n", HackDesc);
                  else
                    fprintf(ofp, "        \" %s\"\n", HackDesc);
                }
            }
            fprintf(ofp, "        \"\\n\" );\n");
            free(TmpDesc);
          }
        }
        fprintf(ofp, "\n");
    
        for(k= 0; k< Rooms->Room[j].ExitCount; k++) {
    /*
     * Unlike extra descriptions, exits have an implied keyword of their direction.
     * Thus, even if Count < 1, the exit will still be valid.
     */
          switch(Rooms->Room[j].Exit[k].Error) {
            case EXIT_OK:
            case EXIT_NON_EUCLIDEAN:
            case EXIT_ONE_WAY:
              fprintf(ofp, "    add_exit( \"%s\", ", 
                      exit_name_lower(Rooms->Room[j].Exit[k].Direction));
              sprintf(outpath, "%s%s%s/%s_%d%s", DS_DOMAIN, OneBigDomain?DS_MEGA_DOMAIN:"",
                      OneBigDomain?"/rooms":"",
                      remap_name(Zones->Zone[remap_zone_vnum(Zones,
                                 Rooms->Room[remap_room_vnum(Rooms,
                                 Rooms->Room[j].Exit[k].Room)].Zone)].Name),
                      Rooms->Room[remap_room_vnum(Rooms,
                                  Rooms->Room[j].Exit[k].Room)].Zone,
                      OneBigDomain?"":"/rooms");
              fprintf(ofp, "\"%s/%s_%d\", ", outpath,
                      remap_name(room_name(Rooms, Rooms->Room[j].Exit[k].Room)),
                      Rooms->Room[j].Exit[k].Room);
              switch(Rooms->Room[j].Exit[k].Type) {
                case EXIT_OPEN:
                case EXIT_OPEN_ALIAS:
                  switch(Rooms->Room[j].Sector) {
                    case SECT_INDOORS:
                      if(Rooms->Room[j].Exit[k].Direction == EXIT_UP ||
                         Rooms->Room[j].Exit[k].Direction == EXIT_DOWN)
                        fprintf(ofp, "\"stairs\" );\n");
                      else
                        fprintf(ofp, "\"corridor\" );\n");
                      break;
                    case SECT_CITY:
                      if(Rooms->Room[j].Exit[k].Direction == EXIT_UP ||
                         Rooms->Room[j].Exit[k].Direction == EXIT_DOWN)
                        fprintf(ofp, "\"stairs\" );\n");
                      else
                        fprintf(ofp, "\"road\" );\n");
                      break;
                    case SECT_FOREST:
                    case SECT_HILLS:
                    case SECT_MOUNTAIN:
                      fprintf(ofp, "\"standard\" );\n");
                      break;
                    default:
                      fprintf(ofp, "\"plain\" );\n");
                      break;
                  }
                  break;
                case EXIT_DOOR:
                case EXIT_DOOR_ALIAS:
                case EXIT_NOPICK:
                case EXIT_NOPICK_ALIAS:
                  fprintf(ofp, "\"door\" );\n");
                  break;
                case EXIT_SECRET:
                case EXIT_SECRET_ALIAS:
                case EXIT_SECRET_NOPICK:
                case EXIT_SECRET_NOPICK_ALIAS:
                  fprintf(ofp, "\"secret\" );\n");
                  break;
                default:
                  fprintf(ofp, "\"standard\" );\n");
                  break;
              }
              if(!ObviousExits) {
                fprintf(ofp, "    modify_exit( \"%s\", ({ \"obvious\", 0 }) );\n", exit_name_lower(Rooms->Room[j].Exit[k].Direction));
              }
              if(Rooms->Room[j].Exit[k].Keyword->Count > 1) {
                fprintf(ofp, "    add_alias( \"%s\", ({ \"%s\"",
                        exit_name_lower(Rooms->Room[j].Exit[k].Direction),
                        Rooms->Room[j].Exit[k].Keyword->Word[0]);
                for(x= 1; x< Rooms->Room[j].Exit[k].Keyword->Count; x++)
                  fprintf(ofp, ", \"%s\"", Rooms->Room[j].Exit[k].Keyword->Word[x]);
                fprintf(ofp, " }) );\n");
              } else if(Rooms->Room[j].Exit[k].Keyword->Count > 0) {
                fprintf(ofp, "    add_alias( \"%s\", \"%s\" );\n",
                        exit_name_lower(Rooms->Room[j].Exit[k].Direction),
                        Rooms->Room[j].Exit[k].Keyword->Word[0]);
              }
              if(Rooms->Room[j].Exit[k].Description && 
                 Rooms->Room[j].Exit[k].Description[0]) {
                fprintf(ofp, "    add_item( ");
                if(Rooms->Room[j].Exit[k].Keyword->Count > 0)
                  fprintf(ofp, "({ \"%s\"",
                          exit_name_lower(Rooms->Room[j].Exit[k].Direction));
                else
                  fprintf(ofp, "\"%s\"",
                          exit_name_lower(Rooms->Room[j].Exit[k].Direction));
                for(x= 0; x< Rooms->Room[j].Exit[k].Keyword->Count; x++)
                  fprintf(ofp, ", \"%s\"", Rooms->Room[j].Exit[k].Keyword->Word[x]);
                if(Rooms->Room[j].Exit[k].Keyword->Count > 0)
                  fprintf(ofp, " }),\n");
                else
                  fprintf(ofp, " ,\n");
    
/*
 * ARGH!  We have to escape all " characters inside our descriptions because
 * we're now using LPC....  HACK ALERT!
 */
                BigHack = my_strdup(Rooms->Room[j].Exit[k].Description);
                for(x= y= 0; x< strlen(BigHack); x++)
                  if(BigHack[x] == '\"')
                    y++;
                TmpDesc = get_mem(strlen(BigHack)+y+1, sizeof(char));
                bzero(TmpDesc, strlen(BigHack)+y+1);
                for(x= y= 0; x< strlen(BigHack); x++, y++) {
                  if(BigHack[x] == '\"')
                    TmpDesc[y++]= '\\';
                  TmpDesc[y]= BigHack[x];
                }
                free(BigHack);
                if(!(HackDesc = (char *)strtok(TmpDesc, "\n")))
                  fprintf(ofp, "        \"%s\"\n", TmpDesc);
                else {
                  fprintf(ofp, "        \"%s\"\n", HackDesc);
                  while((HackDesc = (char *)strtok(NULL, "\n")))
                    if(HackDesc) {
                      if(*HackDesc == ' ')
                        fprintf(ofp, "        \"\\n%s\"\n", HackDesc);
                      else
                        fprintf(ofp, "        \" %s\"\n", HackDesc);
                    }
                }
                fprintf(ofp, "        \"\\n\" );\n");
                free(TmpDesc);
              }
              break;
          }
    /*
     * Keys would require that objects be done... ignore.
     */
        }
        fprintf(ofp, "}\n");
        fclose(ofp);
      }
    }
    if(Verbose)
      fprintf(stderr, "done.\n");
    else if(!Quiet) {
      fprintf(stderr, "done.\r");
      fflush(stderr);
    }
  }
  if(!Quiet)
    fprintf(stderr, "\n");
}

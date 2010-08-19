#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "include/structs.h"
#include "include/main.h"
#include "include/utils.h"
#include "include/make_index.h"
#include "include/parse_wiley.h"
#include "include/dump_report.h"

#define _DUMP_NIGHTMARE_C
#include "include/dump_nightmare.h"

void dump_as_nightmare(zones *Zones, rooms *Rooms, shops *Shops) {
  FILE *ofp, *fp2;
  char filename[256], domainname[256], dirname[256], subdirname[256];
  char mudpath[256], roomname[256], mudname[256], doorname[256], outpath[256];
  int i, j, k, x;
  int LowRoom, HighRoom;
  pair *ZoneSort;
  char *TmpDesc, *HackDesc;

/* Ensure that each domain exists in our directory structure. */
  ZoneSort= (pair *)get_mem(Zones->Count, sizeof(pair));
  bzero(ZoneSort, sizeof(pair)*Zones->Count);
  for(i= 0; i< Zones->Count; i++) {
    sprintf(domainname, "%s_%d", remap_name(Zones->Zone[i].Name), Zones->Zone[i].Number);
    sprintf(dirname, "%s/%s%s/%s", OutputDir, NIGHTMARE_SUBDIR, NIGHTMARE_DOMAIN, domainname);
    sprintf(filename, "mkdir -p %s", dirname);
    system(filename);
    sprintf(filename, "%s/README", dirname);
    ofp= open_file(filename, "w");
    fprintf(ofp, "#ZONEDATA\n");
    fprintf(ofp, "Name\t%s~\n", Zones->Zone[i].Name);
    fprintf(ofp, "Author\tThe Wiley Gang~\n");
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
    ZoneSort[i].x= i;
    ZoneSort[i].y= LowRoom;
    fprintf(ofp, "VNUMs\t%d %d\n", LowRoom, HighRoom);
    fprintf(ofp, "Time\t%d\n", Zones->Zone[i].Time);
    fprintf(ofp, "Mode\t%d\n", Zones->Zone[i].Mode);
    fprintf(ofp, "End\n\n");
    fclose(ofp);
/* There, now we have some setup work to do... */
    sprintf(subdirname, "%s/adm", dirname);
    sprintf(filename, "mkdir -p %s/adm", dirname);
    system(filename);
    sprintf(filename, "%s/README", subdirname);
    ofp= open_file(filename, "w");
    fprintf(ofp, "                         /domains/%s/adm\n", domainname);
    fprintf(ofp, "               The Administrative Directory for %s\n", domainname);
    fprintf(ofp, "\n");
    fprintf(ofp, "This directory contains all files necessary for domain\n");
    fprintf(ofp, "administration.  The following files are required for any standard\n");
    fprintf(ofp, "domain, but not for secondary domains or realms:\n");
    fprintf(ofp, "\n");
    fprintf(ofp, "void.c - The place people go when their environment is destructed\n");
    fprintf(ofp, "freezer.c - The place people go when they go net-dead\n");
    fprintf(ofp, "cache.c - The place hidden items go to hide\n");
    fprintf(ofp, "\n");
    fprintf(ofp, "The access object is used in any realm or domain which is giving out\n");
    fprintf(ofp, "access which is nonstandard.  This is traditionally handled through\n");
    fprintf(ofp, "the 'grant' command.\n");
    fclose(ofp);

    sprintf(filename, "%s/access.c", subdirname);
    ofp= open_file(filename, "w");
    fprintf(ofp, "/*    %s/%s/adm/access.c\n", NIGHTMARE_DOMAIN, domainname);
    fprintf(ofp, " *    From the Nightmare V Object Library\n");
    fprintf(ofp, " *    the access object for the %s domain\n", domainname);
    fprintf(ofp, " *    created by Descartes of Borg 960302\n");
    fprintf(ofp, " */\n");
    fprintf(ofp, "\n");
    fprintf(ofp, "#include <lib.h>\n");
    fprintf(ofp, "\n");
    fprintf(ofp, "inherit LIB_ACCESS;\n");
    fclose(ofp);

    sprintf(filename, "%s/cache.c", subdirname);
    ofp= open_file(filename, "w");
    fprintf(ofp, "/*    /domains/%s/adm/cache.c\n", domainname);
    fprintf(ofp, " *    from the Nightmare V Object Library\n");
    fprintf(ofp, " *    room where hidden objects are stored\n");
    fprintf(ofp, " *    created by Descartes of Borg 960302\n");
    fprintf(ofp, " */\n");
    fprintf(ofp, "\n");
    fprintf(ofp, "#include <lib.h>\n");
    fprintf(ofp, "#include <room.h>\n");
    fprintf(ofp, "\n");
    fprintf(ofp, "inherit LIB_ROOM;\n");
    fprintf(ofp, "\n");
    fprintf(ofp, "void create() {\n");
    fprintf(ofp, "    room::create();\n");
    fprintf(ofp, "    SetShort( \"The cache\");\n");
    fprintf(ofp, "    SetLong( \"Things are hidden here.\");\n");
    fprintf(ofp, "    SetProperties( ([ \"storage room\" : 1, \"logout\" : ROOM_START ]) );\n");
    fprintf(ofp, "}\n");
    fclose(ofp);

    sprintf(filename, "%s/freezer.c", subdirname);
    ofp= open_file(filename, "w");
    fprintf(ofp, "/*    /domains/%s/adm/freezer.c\n", domainname);
    fprintf(ofp, " *    from the Nightmare V Object Library\n");
    fprintf(ofp, " *    room that stores net-dead people\n");
    fprintf(ofp, " *    created by Descartes of Borg 960302\n");
    fprintf(ofp, " */\n");
    fprintf(ofp, "\n");
    fprintf(ofp, "#include <lib.h>\n");
    fprintf(ofp, "#include <config.h>\n");
    fprintf(ofp, "#include <rooms.h>\n");
    fprintf(ofp, "\n");
    fprintf(ofp, "inherit LIB_ROOM;\n");
    fprintf(ofp, "\n");
    fprintf(ofp, "static private object *Old;\n");
    fprintf(ofp, "\n");
    fprintf(ofp, "void create() {\n");
    fprintf(ofp, "    room::create();\n");
    fprintf(ofp, "    SetNoClean(1);\n");
    fprintf(ofp, "    SetProperties(([ \"login\" : ROOM_START, \"no teleport\" : 1 ]));\n");
    fprintf(ofp, "    SetShort( \"The freezer\");\n");
    fprintf(ofp, "    SetLong( \"The local freezer.  Go down to leave.\");\n");
    fprintf(ofp, "    SetObviousExits(\"d\");\n");
    fprintf(ofp, "    SetExits( ([ \"down\" : ROOM_START ]) );\n");
    fprintf(ofp, "    Old = ({});\n");
    fprintf(ofp, "    call_out(\"clean_room\", MAX_NET_DEAD_TIME);\n");
    fprintf(ofp, "}\n");
    fprintf(ofp, "\n");
    fprintf(ofp, "static void clean_room() {\n");
    fprintf(ofp, "    object *clean_me;\n");
    fprintf(ofp, "    object ob;\n");
    fprintf(ofp, "\n");
    fprintf(ofp, "    foreach(ob in filter(all_inventory(), (: !living($1) :)))\n");
    fprintf(ofp, "      ob->eventDestruct();\n");
    fprintf(ofp, "    if( !sizeof(filter(all_inventory(), (: living :))) ) {\n");
    fprintf(ofp, "        Old = ({});\n");
    fprintf(ofp, "        call_out((: clean_room :), MAX_NET_DEAD_TIME);\n");
    fprintf(ofp, "        return;\n");
    fprintf(ofp, "    }\n");
    fprintf(ofp, "    clean_me = (all_inventory() & Old);\n");
    fprintf(ofp, "    Old = all_inventory() - clean_me;\n");
    fprintf(ofp, "    foreach(ob in clean_me) ob->eventDestruct();\n");
    fprintf(ofp, "    call_out((: clean_room :), MAX_NET_DEAD_TIME);\n");
    fprintf(ofp, "}\n");
    fclose(ofp);

    sprintf(filename, "%s/void.c", subdirname);
    ofp= open_file(filename, "w");
    fprintf(ofp, "/*    /domains/%s/adm/void.c\n", domainname);
    fprintf(ofp, " *    from the Nightmare V Object Library\n");
    fprintf(ofp, " *    place where people go when their environments accidentally are\n");
    fprintf(ofp, " *    destructed\n");
    fprintf(ofp, " *    created by Descartes of Borg 960302\n");
    fprintf(ofp, " */\n");
    fprintf(ofp, "\n");
    fprintf(ofp, "#include <lib.h>\n");
    fprintf(ofp, "#include <rooms.h>\n");
    fprintf(ofp, "\n");
    fprintf(ofp, "inherit LIB_ROOM;\n");
    fprintf(ofp, "\n");
    fprintf(ofp, "void create() {\n");
    fprintf(ofp, "    room::create();\n");
    fprintf(ofp, "    SetShort(\"the void\");\n");
    fprintf(ofp, "    SetLong(\"The void.  Go down to get out.\");\n");
    fprintf(ofp, "    SetExits( ([ \"down\" : ROOM_START ]) );\n");
    fprintf(ofp, "}\n");
    fclose(ofp);

    sprintf(filename, "mkdir -p %s/etc", dirname);
    system(filename);

/* These are unused at present and might not always be needed... */

    sprintf(filename, "mkdir -p %s/armour", dirname);
    system(filename);
    sprintf(filename, "mkdir -p %s/fish", dirname);
    system(filename);
    sprintf(filename, "mkdir -p %s/meal", dirname);
    system(filename);
    sprintf(filename, "mkdir -p %s/npc", dirname);
    system(filename);
    sprintf(filename, "mkdir -p %s/save", dirname);
    system(filename);
    sprintf(filename, "mkdir -p %s/weapon", dirname);
    system(filename);

/* This is the virtual room server */

    sprintf(subdirname, "%s/virtual", dirname);
    sprintf(filename, "mkdir -p %s/virtual", dirname);
    system(filename);
    sprintf(filename, "%s/README", subdirname);
    ofp= open_file(filename, "w");
    fprintf(ofp, "                       /domains/%s/virtual\n", domainname);
    fprintf(ofp, "                 Virtual Rooms for the %s Domain\n", domainname);
    fprintf(ofp, "\n");
    fprintf(ofp, "Here is a virtual grid server setup for a 25 by 25 room grid.\n");
    fprintf(ofp, "\n");
    fprintf(ofp, "What happens is when some objects references an object in\n");
    fprintf(ofp, "/domains/%s/virtual/grassland/ but finds no file there, it\n", domainname);
    fprintf(ofp, "calls compile_object() in /domains/%s/virtual/server.c.  That\n", domainname);
    fprintf(ofp, "function returns an object which serves as the non-existent object.\n");
    fprintf(ofp, "\n");
    fprintf(ofp, "The virtual server looks at the file name like:\n");
    fprintf(ofp, "/domains/%s/virtual/grassland/15,12\n", domainname);
    fprintf(ofp, "\n");
    fprintf(ofp, "That tells it to clone /domains/%s/virtual/grassland.c and pass it\n", domainname);
    fprintf(ofp, "15, 12 as the argument to create().  Thus grassland.c is able to set up\n");
    fprintf(ofp, "exits and such for its location at 15, 12 on the virtual grassland grid.\n");
    fprintf(ofp, "\n");
    fprintf(ofp, "You can thus write your own virtual grid, say for a forest, simply by\n");
    fprintf(ofp, "writing a forest.c like the grassland.c given here.\n");
    fprintf(ofp, "\n");
    fclose(ofp);

    sprintf(filename, "mkdir -p %s/virtual/grassland", dirname);
    system(filename);

    sprintf(filename, "%s/grassland.c", subdirname);
    ofp= open_file(filename, "w");
    fprintf(ofp, "/*    /domains/%s/virtual/grassland.c\n", domainname);
    fprintf(ofp, " *    from the Nightmare V Object Library\n");
    fprintf(ofp, " *    created by Descartes of Borg 960302\n");
    fprintf(ofp, " */\n");
    fprintf(ofp, "\n");
    fprintf(ofp, "#include <lib.h>\n");
    fprintf(ofp, "\n");
    fprintf(ofp, "inherit LIB_ROOM;\n");
    fprintf(ofp, "\n");
    fprintf(ofp, "static private int XPosition, YPosition;\n");
    fprintf(ofp, "\n");
    fprintf(ofp, "static void SetLongAndItems();\n");
    fprintf(ofp, "\n");
    fprintf(ofp, "varargs static void create(int x, int y) {\n");
    fprintf(ofp, "    string n, s, e, w;\n");
    fprintf(ofp, "\n");
    fprintf(ofp, "    SetNoReplace(1);\n");
    fprintf(ofp, "    room::create();\n");
    fprintf(ofp, "    XPosition = x;\n");
    fprintf(ofp, "    YPosition = y;\n");
    fprintf(ofp, "    SetClimate(\"temperate\");\n");
    fprintf(ofp, "    SetProperties( ([ \"light\" : 2 ]) );\n");
    fprintf(ofp, "    SetShort(x == 25 ? \"the edge of a grassy plain\" : \"endless grasslands\");\n");
    fprintf(ofp, "    SetLongAndItems();\n");
    fprintf(ofp, "    if( x == 25 ) e = __DIR__ + random(25) + \",\" + y;\n");
    fprintf(ofp, "    else e = __DIR__ + (x+1) + \",\" + y;\n");
    fprintf(ofp, "    if( x == 1 ) w = __DIR__ + random(25) + \",\" + y;\n");
    fprintf(ofp, "    else w = __DIR__ + (x-1) + \",\" + y;\n");
    fprintf(ofp, "    if( y == 25 ) n = __DIR__ + x + \",\" + random(25);\n");
    fprintf(ofp, "    else n = __DIR__ + x + \",\" + (y+1);\n");
    fprintf(ofp, "    if( y == 1 ) s = __DIR__ + x+ \",\" + random(25);\n");
    fprintf(ofp, "    else s = __DIR__ + x + \",\" + (y-1);\n");
    fprintf(ofp, "    SetGoMessage(\"The grass abruptly ends at a great VOID!  You cannot pass.\");\n");
    fprintf(ofp, "    if( n ) AddExit(\"north\", __DIR__ + n);\n");
    fprintf(ofp, "    if( s ) AddExit(\"south\", __DIR__ + s);\n");
    fprintf(ofp, "    if( e ) AddExit(\"east\", __DIR__ + e);\n");
    fprintf(ofp, "    if( w ) AddExit(\"west\", __DIR__ + w);\n");
    fprintf(ofp, "}\n");
    fprintf(ofp, "\n");
    fprintf(ofp, "static void SetLongAndItems() {\n");
    fprintf(ofp, "    mapping inv, items;\n");
    fprintf(ofp, "    string str;\n");
    fprintf(ofp, "\n");
    fprintf(ofp, "    inv = ([]);\n");
    fprintf(ofp, "    str = \"You stand amidst an endless sea of tall grasses, gently waving\"\n");
    fprintf(ofp, "        \" in the breeze.  Quiet, peaceful, hypnotic.  You might as well\"\n");
    fprintf(ofp, "        \" become one of the beckoning blades...\";\n");
    fprintf(ofp, "    items = ([ \"grassland\" : \"The peaceful ocean of grass goes on forever.\",\n");
    fprintf(ofp, "        \"grass\" : \"It seems soft yet strong, edible if you were a cow.\" ]);\n");
    fprintf(ofp, "    if( !random(50) ) {\n");
    fprintf(ofp, "        str += \"  A small circle of stones and some burnt wood stands out\"\n");
    fprintf(ofp, "            \" from the grass a bit.\";\n");
    fprintf(ofp, "        items[({ \"stones\", \"rocks\" })] =\n");
    fprintf(ofp, "            \"They are smallish stones, set close in a circle to prevent\"\n");
    fprintf(ofp, "            \" a fire within from spreading to the sometimes dry grass\"\n");
    fprintf(ofp, "            \" which is all around you.\";\n");
    fprintf(ofp, "        items[({ \"twigs\", \"sticks\", \"kindling\", \"wood\", \"burnt wood\" })] =\n");
    fprintf(ofp, "              \"Though long since burnt to nothing, scattered kindling \"\n");
    fprintf(ofp, "              \"and burnt wood lie about as a memory of travellers who have \"\n");
    fprintf(ofp, "              \"passed through\";\n");
    fprintf(ofp, "        if( random(2) ) {\n");
    fprintf(ofp, "            string thing;\n");
    fprintf(ofp, "\n");
    fprintf(ofp, "            foreach(thing in ({ \"twigs\", \"sticks\", \"kindling\", \"wood\" }))\n");
    fprintf(ofp, "              SetSearch(thing, function(object who, string str) {\n");
    fprintf(ofp, "                  object ob;\n");
    fprintf(ofp, "                  string thing2;\n");
    fprintf(ofp, "\n");
    fprintf(ofp, "                  if( !(ob = new(DIR_STANDARD_DOMAIN \"/etc/pole\")) )\n");
    fprintf(ofp, "                    return 0;\n");
    fprintf(ofp, "                  who->eventPrint(\"You find a fishing pole!\");\n");
    fprintf(ofp, "                  eventPrint((string)who->GetName() + \" finds a fishing pole \"\n");
    fprintf(ofp, "                             \"among the abandoned campsite.\", who);\n");
    fprintf(ofp, "                  foreach(thing2 in ({ \"twigs\", \"sticks\", \"kindling\", \"wood\"}))\n");
    fprintf(ofp, "                    RemoveSearch(thing2);\n");
    fprintf(ofp, "                  if( !((int)ob->eventMove(this_player())) ) {\n");
    fprintf(ofp, "                      who->eventPrint(\"You drop the pole!\");\n");
    fprintf(ofp, "                      eventPrint((string)who->GetName() + \" drops the pole.\",\n");
    fprintf(ofp, "                                 who);\n");
    fprintf(ofp, "                      ob->eventMove(this_object());\n");
    fprintf(ofp, "                  }\n");
    fprintf(ofp, "                  return 1;\n");
    fprintf(ofp, "              });\n");
    fprintf(ofp, "        }\n");
    fprintf(ofp, "    }\n");
    fprintf(ofp, "    else if( !random(10) )\n");
    fprintf(ofp, "      SetSmell(\"default\", \"You smell a distant camp fire.\");\n");
    fprintf(ofp, "    if( !random(25) )\n");
    fprintf(ofp, "      inv[DIR_STANDARD_DOMAIN \"/npc/traveller\"] = random(3) + 1;\n");
    fprintf(ofp, "    else if( !random(4) )\n");
    fprintf(ofp, "      SetListen(\"default\", \"You hear voices whispering in the distance.\");\n");
    fprintf(ofp, "    SetLong(str);\n");
    fprintf(ofp, "    SetItems(items);\n");
    fprintf(ofp, "    SetInventory(inv);\n");
    fprintf(ofp, "}\n");
    fclose(ofp);

    sprintf(filename, "%s/server.c", subdirname);
    ofp= open_file(filename, "w");
    fprintf(ofp, "/*    /domains/%s/virtual/server.c\n", domainname);
    fprintf(ofp, " *    from the Nightmare V Object Library\n");
    fprintf(ofp, " *    created by Descartes of Borg 960302\n");
    fprintf(ofp, " */\n");
    fprintf(ofp, "\n");
    fprintf(ofp, "object compile_object(string file) {\n");
    fprintf(ofp, "    string *path;\n");
    fprintf(ofp, "    object ob;\n");
    fprintf(ofp, "    int x, y;\n");
    fprintf(ofp, "\n");
    fprintf(ofp, "    if( previous_object() != master() ) return 0;\n");
    fprintf(ofp, "    path = explode(file, \"/\");\n");
    fprintf(ofp, "    if( sizeof(path) != 5 ) return 0;\n");
    fprintf(ofp, "    if( file_size(__DIR__ + path[3] + \".c\") < 1 ) return 0;\n");
    fprintf(ofp, "    if( sscanf(path[4], \"%%d,%%d\", x, y) != 2 ) return 0;\n");
    fprintf(ofp, "    if( !(ob = new(__DIR__ + path[3], x, y)) ) return 0;\n");
    fprintf(ofp, "    return ob;\n");
    fprintf(ofp, "}\n");
    fclose(ofp);

/* Ye GODS that be alot of setup.... ok, time for the meat now! */

    sprintf(subdirname, "%s/room", dirname);
    sprintf(filename, "mkdir -p %s/room", dirname);
    system(filename);
    sprintf(mudpath, "%s/%s/room", NIGHTMARE_DOMAIN, domainname);

    for(j= 0; j< Rooms->Count; j++) {
      if((remap_zone_vnum(Zones, Rooms->Room[j].Zone) == i) ||
         ((Rooms->Room[j].Number >= (!i? 0: Zones->Zone[i-1].Top+1)) &&
          (Rooms->Room[j].Number <= Zones->Zone[i].Top)
      )) {
        sprintf(roomname, "%s_%d.c", remap_name(Rooms->Room[j].Name),
                Rooms->Room[j].Number);
        sprintf(filename, "%s/%s", subdirname, roomname);
        sprintf(mudname, "%s/%s", mudpath, roomname);
        ofp= open_file(filename, "w");

#ifdef DEBUG
        fprintf(stderr, "Dumping %s\n", filename);
#endif
        fprintf(ofp, "// Automated conversion of WileyMUD by Quixadhal\n");
        fprintf(ofp, "// Original:   WileyMUD III, Room [#%d]\n",
                Rooms->Room[j].Number);
        fprintf(ofp, "// Target:     Nightmare IVr2.5, %s\n", mudname);
        fprintf(ofp, "// Performed:  %s\n", timestamp());
        fprintf(ofp, "\n");

/* Setup basic file now... */

        fprintf(ofp, "#include <lib.h>\n");
        fprintf(ofp, "\n");
        fprintf(ofp, "inherit LIB_ROOM;\n");
        fprintf(ofp, "\n");
    
        fprintf(ofp, "static void create() {\n");
        fprintf(ofp, "    room::create();\n");
        if(Rooms->Room[j].Flags & ROOM_DARK)
          fprintf(ofp, "    SetProperty(\"light\", 0);\n");
        else
          fprintf(ofp, "    SetProperty(\"light\", 2);\n");
    
/* Climate is a poor substitute for sector types... adjust manually. */

        if(Rooms->Room[j].Flags & ROOM_INDOORS)
          fprintf(ofp, "    SetClimate(\"indoors\");\n");
        else switch(Rooms->Room[j].Sector) {
          case SECT_INDOORS:
            fprintf(ofp, "    SetClimate(\"indoors\");\n");
            break;
          case SECT_FOREST:
            fprintf(ofp, "    SetClimate(\"tropical\");\n");
            break;
          case SECT_HILLS:
            fprintf(ofp, "    SetClimate(\"arid\");\n");
            break;
          case SECT_MOUNTAIN:
            fprintf(ofp, "    SetClimate(\"arctic\");\n");
            break;
        }
    
        fprintf(ofp, "\n");
        fprintf(ofp, "    SetShort(\"%s\");\n", Rooms->Room[j].Name);
        TmpDesc = my_strdup(Rooms->Room[j].Description);
        if(!(HackDesc = (char *)strtok(TmpDesc, "\n")))
          fprintf(ofp, "    SetLong(\"%s\");\n", TmpDesc);
        else {
          fprintf(ofp, "    SetLong(\"%s\"\n", HackDesc);
          while((HackDesc = (char *)strtok(NULL, "\n")))
            if(HackDesc)
              fprintf(ofp, "        \" %s\"\n", HackDesc);
          fprintf(ofp, "        );\n");
        }
        free(TmpDesc);
        
        fprintf(ofp, "\n");
        k= 0;
        if(Rooms->Room[j].Flags & ROOM_NOATTACK)
          { k= 1; fprintf(ofp, "    SetProperty(\"no attack\", 1);\n"); }
        if(Rooms->Room[j].Flags & ROOM_NOSTEAL)
          { k= 1; fprintf(ofp, "    SetProperty(\"no steal\", 1);\n"); }
        if(Rooms->Room[j].Flags & ROOM_NOSUMMON)
          { k= 1; fprintf(ofp, "    SetProperty(\"no teleport\", 1);\n"); }
        if(Rooms->Room[j].Flags & ROOM_NOMAGIC)
          { k= 1; fprintf(ofp, "    SetProperty(\"no magic\", 1);\n"); }
        if(k) fprintf(ofp, "\n");
    
        if(Rooms->Room[j].ExtraCount > 0) {
          fprintf(ofp, "    SetItems( ([\n");
          for(k= 0; k< Rooms->Room[j].ExtraCount; k++) {
            if(Rooms->Room[j].Extra[k].Keyword->Count > 0) {
              if(Rooms->Room[j].Extra[k].Keyword->Count > 1)
                fprintf(ofp, "        ({ \"%s\"",
                        Rooms->Room[j].Extra[k].Keyword->Word[0]);
              else
                fprintf(ofp, "        \"%s\"",
                        Rooms->Room[j].Extra[k].Keyword->Word[0]);
              for(x= 1; x< Rooms->Room[j].Extra[k].Keyword->Count; x++)
                fprintf(ofp, ", \"%s\"", Rooms->Room[j].Extra[k].Keyword->Word[x]);
              if(Rooms->Room[j].Extra[k].Keyword->Count > 1)
                fprintf(ofp, " }) :\n");
              else
                fprintf(ofp, " :\n");
    
              TmpDesc = my_strdup(Rooms->Room[j].Extra[k].Description);
              if(!(HackDesc = (char *)strtok(TmpDesc, "\n")))
                fprintf(ofp, "        \"%s\"\n", TmpDesc);
              else {
                fprintf(ofp, "        \"%s\"\n", HackDesc);
                while((HackDesc = (char *)strtok(NULL, "\n")))
                  if(HackDesc)
                    fprintf(ofp, "        \" %s\"\n", HackDesc);
              }
              free(TmpDesc);
              if(k < Rooms->Room[j].ExtraCount-1)
                fprintf(ofp, "        ,\n");
            }
          }
          fprintf(ofp, "        ]) );\n");
        }
    
        if(Rooms->Room[j].ExitCount > 0) {
          fprintf(ofp, "    SetExits( ([\n");
          for(k= 0; k< Rooms->Room[j].ExitCount; k++) {
    /* Unlike extra descriptions, exits have an implied keyword of their direction.
     * Thus, even if Count < 1, the exit will still be valid.
     */
            switch(Rooms->Room[j].Exit[k].Error) {
              case EXIT_OK:
              case EXIT_NON_EUCLIDEAN:
              case EXIT_ONE_WAY:
                if(Rooms->Room[j].Exit[k].Keyword->Count > 0) {
                  fprintf(ofp, "        ({ \"%s\"", 
                          exit_name_lower(Rooms->Room[j].Exit[k].Direction));
                  for(x= 0; x< Rooms->Room[j].Exit[k].Keyword->Count; x++)
                    fprintf(ofp, ", \"%s\"", Rooms->Room[j].Exit[k].Keyword->Word[x]);
                  fprintf(ofp, " }) :\n");
                } else {
                  fprintf(ofp, "        \"%s\" :\n",
                          exit_name_lower(Rooms->Room[j].Exit[k].Direction));
                }
                sprintf(outpath, "%s/%s_%d/room", NIGHTMARE_DOMAIN,
                        remap_name(Zones->Zone[remap_zone_vnum(Zones,
                                   Rooms->Room[remap_room_vnum(Rooms,
                                   Rooms->Room[j].Exit[k].Room)].Zone)].Name),
                        Rooms->Room[remap_room_vnum(Rooms,
                                    Rooms->Room[j].Exit[k].Room)].Zone);
                fprintf(ofp, "        \"%s/%s_%d\"\n", outpath,
                        remap_name(room_name(Rooms, Rooms->Room[j].Exit[k].Room)),
                        Rooms->Room[j].Exit[k].Room);
                if(k < Rooms->Room[j].ExitCount-1)
                  fprintf(ofp, "        ,\n");
    /* Keys would require that objects be done... ignore.
     * Descriptions are not supported in Nightmare... you would have to add
     * it as an AddItem call, but then it would be "look at north" instead of
     * "look north".... ignore for now.
     */
                break;
            }
          }
          fprintf(ofp, "        ]) );\n");
        }
    
    /* NOW, we can go setup the doors... we need a default door as well. */
    
        if(Rooms->Room[j].ExitCount > 0) {
          for(k= 0; k< Rooms->Room[j].ExitCount; k++) {
            switch(Rooms->Room[j].Exit[k].Type) {
              case EXIT_OPEN:
              case EXIT_OPEN_ALIAS:
                break;
              default:
                sprintf(doorname, "%s/etc/%s_%d_%s_door.c", dirname,
                        remap_name(room_name(Rooms, Rooms->Room[j].Exit[k].Room)),
                        Rooms->Room[j].Exit[k].Room,
                        exit_name_lower(Rooms->Room[j].Exit[k].Direction));
                fprintf(ofp, "    SetDoor( \"%s\", \"%s\" );\n",
                        exit_name_lower(Rooms->Room[j].Exit[k].Direction),
                        doorname);
                fp2= open_file(doorname, "w");
                fprintf(fp2, "/*    /domains/%s/etc/%s_%d_%s_door.c\n", domainname,
                        remap_name(room_name(Rooms, Rooms->Room[j].Exit[k].Room)),
                        Rooms->Room[j].Exit[k].Room,
                        exit_name_lower(Rooms->Room[j].Exit[k].Direction));
                fprintf(fp2, " *    from Nightmare LPMud\n");
                fprintf(fp2, " *    created by Descartes of Borg 951027\n");
                fprintf(fp2, " */\n");
                fprintf(fp2, "\n");
                fprintf(fp2, "#include <lib.h>\n");
                fprintf(fp2, "\n");
                fprintf(fp2, "inherit LIB_DOOR;\n");
                fprintf(fp2, "\n");
                fprintf(fp2, "static void create() {\n");
                fprintf(fp2, "    door::create();\n");
                fprintf(fp2, "    SetId(\"%s\", \"door\");\n",
                        exit_name_lower(Rooms->Room[j].Exit[k].Direction));
                fprintf(fp2, "    SetShort(\"%s\", \"a cracked wooden door\");\n",
                        exit_name_lower(Rooms->Room[j].Exit[k].Direction));
                fprintf(fp2, "    SetLong(\"%s\", \"The door is cracked and has seen better days.\"\n",
                        exit_name_lower(Rooms->Room[j].Exit[k].Direction));
                fprintf(fp2, "    SetId(\"%s\", \"door\");\n",
                        exit_name_lower(RevDir[Rooms->Room[j].Exit[k].Direction]));
                fprintf(fp2, "    SetShort(\"%s\", \"a cracked wooden door\");\n",
                        exit_name_lower(RevDir[Rooms->Room[j].Exit[k].Direction]));
                fprintf(fp2, "    SetLong(\"%s\", \"The door is cracked and has seen better days.\"\n",
                        exit_name_lower(RevDir[Rooms->Room[j].Exit[k].Direction]));
                fprintf(fp2, "    SetClosed(1);\n");
                fprintf(fp2, "}\n");
                fclose(fp2);
                if(k < Rooms->Room[j].ExitCount-1)
                  fprintf(ofp, "        ,\n");
    /* Keys would require that objects be done... ignore.
     * Descriptions are not supported in Nightmare... you would have to add
     * it as an AddItem call, but then it would be "look at north" instead of
     * "look north".... ignore for now.
     */
                break;
            }
          }
        }
        fprintf(ofp, "}\n");
        fclose(ofp);
      }
    }
  }
}

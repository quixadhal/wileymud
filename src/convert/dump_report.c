#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "include/structs.h"
#include "include/main.h"
#include "include/utils.h"

#include "include/parse_wiley.h"

#define _REPORTS_C
#include "include/dump_report.h"

void make_zone_report(zones *Zones, rooms *Rooms, objects *Objects, mobs *Mobs, char *outfile) {
  FILE *ofp;
  register int i, j;
  int LastMob, LastLoc;

  if(outfile && *outfile) {
    if(!Quiet) {
      fprintf(stderr, "Generating Zone reports...");
      fflush(stderr);
    }
    ofp= open_file(outfile, "w");
    for(i= j= 0; i< Zones->Count; i++) j+= Zones->Zone[i].Count;
    fprintf(ofp, "ZONE REPORT: %d total commands found in %d zones.\n",
            j, Zones->Count);
    for(i= 0; i< Zones->Count; i++) {
      if(!Quiet)
        spin(stderr);
      LastMob= LastLoc= -1;
      fprintf(ofp, "Zone \"%s\"[#%d] spans rooms (#%d,#%d)\n",
              Zones->Zone[i].Name, Zones->Zone[i].Number, 
              (!i? 0: Zones->Zone[i-1].Top+1), Zones->Zone[i].Top);
      if(Zones->Zone[i].Mode == ZONE_RESET_PC)
        fprintf(ofp,
                "    It resets every %d minutes, if no players are present\n",
                Zones->Zone[i].Time);
      else if(Zones->Zone[i].Mode == ZONE_RESET_ALWAYS)
        fprintf(ofp, "    It always resets every %d minutes\n",
                Zones->Zone[i].Time);
      else if(Zones->Zone[i].Mode == ZONE_RESET_NEVER)
        fprintf(ofp, "    It does not reset\n");
      else fprintf(ofp, "    It resets in some undefined fashion\n");
      if(Zones->Zone[i].Count)
        fprintf(ofp, "    There are %d Commands defined:\n",
                Zones->Zone[i].Count);
      else fprintf(ofp, "    There are no Commands in this Zone.\n");
      for(j= 0; j< Zones->Zone[i].Count; j++) {
        fprintf(ofp, "        [%3d] ", j);
        switch(Zones->Zone[i].Cmds[j].Command) {
          case ZONE_CMD_MOBILE:
            LastMob= Zones->Zone[i].Cmds[j].Arg[ZONE_MOBILE];
            LastLoc= Zones->Zone[i].Cmds[j].Arg[ZONE_ROOM];
            fprintf(ofp, "Load Mobile \"%s\"[#%d] to \"%s\"[#%d]\n",
                    mob_name(Mobs, Zones->Zone[i].Cmds[j].Arg[ZONE_MOBILE]),
                    Zones->Zone[i].Cmds[j].Arg[ZONE_MOBILE],
                    room_name(Rooms, Zones->Zone[i].Cmds[j].Arg[ZONE_ROOM]),
                    Zones->Zone[i].Cmds[j].Arg[ZONE_ROOM]);
            break;
          case ZONE_CMD_OBJECT:
            fprintf(ofp, "Load Object \"%s\"[#%d] to \"%s\"[#%d]\n",
                    obj_name(Objects, Zones->Zone[i].Cmds[j].Arg[ZONE_OBJECT]),
                    Zones->Zone[i].Cmds[j].Arg[ZONE_OBJECT],
                    room_name(Rooms, Zones->Zone[i].Cmds[j].Arg[ZONE_ROOM]),
                    Zones->Zone[i].Cmds[j].Arg[ZONE_ROOM]);
            break;
          case ZONE_CMD_GIVE:
            fprintf(ofp, "Give Object \"%s\"[#%d] to Mobile \"%s\"[#%d]\n",
                    obj_name(Objects, Zones->Zone[i].Cmds[j].Arg[ZONE_OBJECT]),
                    Zones->Zone[i].Cmds[j].Arg[ZONE_OBJECT],
                    mob_name(Mobs, LastMob), LastMob);
            break;
          case ZONE_CMD_EQUIP:
            fprintf(ofp, "Object \"%s\"[#%d] is %s by Mobile \"%s\"[#%d]\n",
                    obj_name(Objects, Zones->Zone[i].Cmds[j].Arg[ZONE_OBJECT]),
                    Zones->Zone[i].Cmds[j].Arg[ZONE_OBJECT],
                    equip_name(Zones->Zone[i].Cmds[j].Arg[ZONE_POSITION]),
                    mob_name(Mobs, LastMob), LastMob);
            break;
          case ZONE_CMD_PUT:
            fprintf(ofp, "Put Object \"%s\"[#%d] into Object \"%s\"[#%d]\n",
                    obj_name(Objects, Zones->Zone[i].Cmds[j].Arg[ZONE_OBJECT]),
                    Zones->Zone[i].Cmds[j].Arg[ZONE_OBJECT],
                    obj_name(Objects, Zones->Zone[i].Cmds[j].Arg[ZONE_TARGET_OBJ]),
                    Zones->Zone[i].Cmds[j].Arg[ZONE_TARGET_OBJ]);
            break;
          case ZONE_CMD_DOOR:
            fprintf(ofp, "Reset %s door of \"%s\"[#%d] to %s\n",
                    exit_name(Zones->Zone[i].Cmds[j].Arg[ZONE_DOOR_EXIT]),
                    room_name(Rooms, Zones->Zone[i].Cmds[j].Arg[ZONE_ROOM]),
                    Zones->Zone[i].Cmds[j].Arg[ZONE_DOOR_ROOM],
            doorstate_name(Zones->Zone[i].Cmds[j].Arg[ZONE_DOOR_STATE]));
            break;
          case ZONE_CMD_REMOVE:
            fprintf(ofp, "Remove Object \"%s\"[#%d] from \"%s\"[#%d]\n",
                    obj_name(Objects, Zones->Zone[i].Cmds[j].Arg[ZONE_REMOVE_OBJ]),
                    Zones->Zone[i].Cmds[j].Arg[ZONE_REMOVE_OBJ],
                    room_name(Rooms, Zones->Zone[i].Cmds[j].Arg[ZONE_ROOM]),
                    Zones->Zone[i].Cmds[j].Arg[ZONE_REMOVE_ROOM]);
            break;
          case ZONE_CMD_LEAD: 
            fprintf(ofp, "Load Mobile \"%s\"[#%d] to \"%s\"[#%d], led by Mobile \"%s\"[#%d]\n",
                    mob_name(Mobs, Zones->Zone[i].Cmds[j].Arg[ZONE_MOBILE]),
                    Zones->Zone[i].Cmds[j].Arg[ZONE_MOBILE],
                    room_name(Rooms, LastLoc), LastLoc,
                    mob_name(Mobs, LastMob), LastMob);
            /* LastMob= Zones->Zone[i].Cmds[j].Arg[ZONE_MOBILE]; */
            break;
          case ZONE_CMD_HATE:
            fprintf(ofp, "Cause Mobile \"%s\"[#%d] in \"%s\"[#%d] to HATE %s\n",
                    mob_name(Mobs, LastMob), LastMob,
                    room_name(Rooms, LastLoc), LastLoc,
                    hate_name(Zones->Zone[i].Cmds[j].Arg[ZONE_HATE_TYPE],
                              Zones->Zone[i].Cmds[j].Arg[ZONE_HATE_VALUE])
                    );
            /* LastMob= Zones->Zone[i].Cmds[j].Arg[ZONE_MOBILE]; */
            break;
          default: fprintf(ofp, "Unrecognized command '%c'?\n",
                           Zones->Zone[i].Cmds[j].Command);
                   break;
        }
      }
      fprintf(ofp, "\n");
    }
    fclose(ofp);
    if(!Quiet)
      fprintf(stderr, "done.\n");
  }
}

void make_shop_report(zones *Zones, rooms *Rooms, objects *Objects, mobs *Mobs, shops *Shops, char *outfile) {
  FILE *ofp;
  register int i, j;

  if(outfile && *outfile) {
    if(!Quiet) {
      fprintf(stderr, "Generating Shop reports...");
      fflush(stderr);
    }
    ofp= open_file(outfile, "w");
    fprintf(ofp, "SHOP REPORT: %d Shops found.\n", Shops->Count);
    for(i= 0; i< Shops->Count; i++) {
      if(!Quiet)
        spin(stderr);
      fprintf(ofp, "Shop#%d is at \"%s\"[#%d], owned by Mobile \"%s\"[#%d] (",
              Shops->Shop[i].Number, room_name(Rooms, Shops->Shop[i].Room),
              Shops->Shop[i].Room, mob_name(Mobs, Shops->Shop[i].Keeper), Shops->Shop[i].Keeper);
      fprintf(ofp, "%s) (", shop_attitude_name(Shops->Shop[i].Attitude));
      fprintf(ofp, "%s)\n", shop_immortal_name(Shops->Shop[i].Immortal));
      fprintf(ofp, "    It sells at %lf list, and buys at %lf list.\n",
              Shops->Shop[i].Selling, Shops->Shop[i].Buying);
      fprintf(ofp, "    It is ");
      if(Shops->Shop[i].Close == 28) fprintf(ofp, "Always Open\n");
      else fprintf(ofp, "Open from %d to %d\n",
                   Shops->Shop[i].Open, Shops->Shop[i].Close);
      fprintf(ofp, "    It Sells:  ");
      if(Shops->Shop[i].SellCount) {
        for(j= 0; j< Shops->Shop[i].SellCount; j++)
          if(Shops->Shop[i].Sell[j] >= 0)
            fprintf(ofp, "Object \"%s\"[#%d] ",
                    obj_name(Objects, Shops->Shop[i].Sell[j]),
                    Shops->Shop[i].Sell[j]);
        fprintf(ofp, "\n");
      } else fprintf(ofp, "nothing\n");
      fprintf(ofp, "    It Trades: ");
      if(Shops->Shop[i].TradeCount) {
        for(j= 0; j< Shops->Shop[i].TradeCount; j++)
          if(Shops->Shop[i].Trade[j])
            fprintf(ofp, "Object Type %d ", Shops->Shop[i].Trade[j]);
        fprintf(ofp, "\n");
      } else fprintf(ofp, "nothing\n");
      for(j= 0; j< Shops->Shop[i].MessageCount; j++)
        fprintf(ofp, "    Message %d: \"%s\"\n", j, Shops->Shop[i].Message[j]);
      fprintf(ofp, "\n");
    }
    fclose(ofp);
    if(!Quiet)
      fprintf(stderr, "done.\n");
  }
}

void make_room_report(zones *Zones, rooms *Rooms, char *outfile) {
  FILE *ofp;
  register int i, j, Sum;

  if(outfile && *outfile) {
    if(!Quiet) {
      fprintf(stderr, "Generating Room reports...");
      fflush(stderr);
    }
    ofp= open_file(outfile, "w");
    fprintf(ofp, "ROOM REPORT: %d rooms found.\n", Rooms->Count);
    for(i= 0; i< Rooms->Count; i++) {
      if(!Quiet)
        spin(stderr);
      fprintf(ofp, "Room \"%s\"[#%d] belongs to zone \"%s\"[#%d]\n",
              room_name(Rooms, Rooms->Room[i].Number), Rooms->Room[i].Number,
              zone_name(Zones, Rooms->Room[i].Zone), Rooms->Room[i].Zone);
      fprintf(ofp, "    It is a \"%s\" room, with a %d byte description\n",
              sector_name(Rooms->Room[i].Sector),
              strlen(Rooms->Room[i].Description));
      if(Rooms->Room[i].ExtraCount) {
        for(j= Sum= 0; j< Rooms->Room[i].ExtraCount; j++)
          Sum += strlen(Rooms->Room[i].Extra[j].Description);
        fprintf(ofp, "    This room has %d Extra Descriptions (%d bytes)\n",
                Rooms->Room[i].ExtraCount, Sum);
      }
      if(Rooms->Room[i].Sector == SECT_TELEPORT) {
        fprintf(ofp, 
                "    This room is treated as \"%s\" for movement purposes\n",
                sector_name(Rooms->Room[i].TeleportSector));
        fprintf(ofp,
                "    After %d ticks, it will teleport you to \"%s\"[#%d]\n",
                Rooms->Room[i].TeleportTime/10,
                room_name(Rooms, Rooms->Room[i].TeleportTo),
                Rooms->Room[i].TeleportTo);
        if(Rooms->Room[i].TeleportLook)
          fprintf(ofp, "    Which you will automatically look at\n");
      } else if(Rooms->Room[i].Sector == SECT_WATER_NOSWIM) {
        fprintf(ofp,
                "    This room will move you through the %s exit in %d ticks\n",
                exit_name(Rooms->Room[i].RiverDirection),
                Rooms->Room[i].RiverSpeed/10);
      }
      fprintf(ofp, "    Flags: %s\n", room_flags(Rooms->Room[i].Flags));
      if(Rooms->Room[i].Flags & ROOM_SOUND) {
        if(Rooms->Room[i].SoundCount > 0)
          fprintf(ofp, "    The sound heard inside the room is:\n    \"%s\"\n",
                  Rooms->Room[i].SoundText[0]);
        if(Rooms->Room[i].SoundCount > 1)
          fprintf(ofp, "    The sound heard in adjacent room is:\n    \"%s\"\n",
                  Rooms->Room[i].SoundText[1]);
      }
      for(j= 0; j< Rooms->Room[i].ExitCount; j++) {
        switch(Rooms->Room[i].Exit[j].Error) {
          case EXIT_OK:
            fprintf(ofp, "    The %s exit is %s, which leads to \"%s\"[#%d]\n",
                    exit_name(Rooms->Room[i].Exit[j].Direction),
                    exittype_name(Rooms->Room[i].Exit[j].Type),
                    room_name(Rooms, Rooms->Room[i].Exit[j].Room),
                    Rooms->Room[i].Exit[j].Room);
            break;
          case EXIT_NON_EUCLIDEAN:
            fprintf(ofp, "    The (non-euclidean) %s exit is %s, which leads to \"%s\"[#%d]\n",
                    exit_name(Rooms->Room[i].Exit[j].Direction),
                    exittype_name(Rooms->Room[i].Exit[j].Type),
                    room_name(Rooms, Rooms->Room[i].Exit[j].Room),
                    Rooms->Room[i].Exit[j].Room);
            {
              EXIT *foreign;
              foreign= &Rooms->Room[Rooms->Room[i].Exit[j].RoomIndex].Exit[RevDir[Rooms->Room[i].Exit[j].Direction]];
              fprintf(ofp, "        The matching %s exit in \"%s\"[#%d] goes to \"%s\"[#%d] instead!\n",
                      exit_name(foreign->Direction),
                      room_name(Rooms, Rooms->Room[i].Exit[j].Room),
                      Rooms->Room[i].Exit[j].Room,
                      room_name(Rooms, foreign->Room),
                      foreign->Room);
            }
            break;
          case EXIT_ONE_WAY:
            fprintf(ofp, "    The (one-way) %s exit is %s, which leads to \"%s\"[#%d]\n",
                    exit_name(Rooms->Room[i].Exit[j].Direction),
                    exittype_name(Rooms->Room[i].Exit[j].Type),
                    room_name(Rooms, Rooms->Room[i].Exit[j].Room),
                    Rooms->Room[i].Exit[j].Room);
            break;
          case EXIT_DESCRIPTION_ONLY:
            fprintf(ofp, "    The %s exit is for description only.\n",
                    exit_name(Rooms->Room[i].Exit[j].Direction));
            break;
        }
      }
      fprintf(ofp, "\n");
    }
    fclose(ofp);
    if(!Quiet)
      fprintf(stderr, "done.\n");
  }
}

void make_obj_report(zones *Zones, objects *Objects, char *outfile) {
  FILE *ofp;
  register int i, j, Sum;

  if(outfile && *outfile) {
    if(!Quiet) {
      fprintf(stderr, "Generating Object reports...");
      fflush(stderr);
    }
    ofp= open_file(outfile, "w");
    fprintf(ofp, "OBJECT REPORT: %d Objects found.\n", Objects->Count);
    for(i= 0; i< Objects->Count; i++) {
      if(!Quiet)
        spin(stderr);
      fprintf(ofp, "Object \"%s\"[#%d] is known by the id string \"%s\"\n",
              obj_name(Objects, Objects->Object[i].Number), Objects->Object[i].Number,
              Objects->Object[i].Name);
      fprintf(ofp, "    It belongs to zone \"%s\"[#%d]\n",
              zone_name(Zones, Objects->Object[i].Zone), Objects->Object[i].Zone);
      if(Objects->Object[i].Value < 0)
        fprintf(ofp, "    It weighs %d units and cannot be sold, costing %d to rent.\n",
                Objects->Object[i].Weight, Objects->Object[i].Rent);
      else
        fprintf(ofp, "    It weighs %d units and is worth %d gold, costing %d to rent.\n",
                Objects->Object[i].Weight, Objects->Object[i].Value, Objects->Object[i].Rent);
      if(Objects->Object[i].Description && *(Objects->Object[i].Description))
        fprintf(ofp, "    It has a %d byte long description\n        \"%s\"\n",
                strlen(Objects->Object[i].Description), Objects->Object[i].Description);
      if(Objects->Object[i].ActionDesc && *(Objects->Object[i].ActionDesc))
        fprintf(ofp, "    It has a %d byte action description\n        \"%s\"\n",
                strlen(Objects->Object[i].ActionDesc), Objects->Object[i].ActionDesc);
      if(Objects->Object[i].ExtraCount) {
        for(j= Sum= 0; j< Objects->Object[i].ExtraCount; j++)
          Sum += strlen(Objects->Object[i].Extra[j].Description);
        fprintf(ofp, "    This object has %d Extra Descriptions (%d bytes)\n",
                Objects->Object[i].ExtraCount, Sum);
      }

      if(Objects->Object[i].Flags.Wear)
        fprintf(ofp, "    It can be %s\n", item_wear_name(Objects->Object[i].Flags.Wear));
      if(Objects->Object[i].Flags.Extra)
        fprintf(ofp, "    It has these flags: %s\n", item_flag_name(Objects->Object[i].Flags.Extra));

      switch(Objects->Object[i].Type) {
        case ITEM_LIGHT:
          fprintf(ofp, "    This is a light source, lasting %d ticks.\n",
                  Objects->Object[i].Flags.Value[2]);
          fprintf(ofp, "    It of type %d and has colour %d\n",
                  Objects->Object[i].Flags.Value[1],
                  Objects->Object[i].Flags.Value[0]);
          break;
        case ITEM_SCROLL:
        case ITEM_POTION:
          fprintf(ofp, "    This is a level %d %s, containing the spells:\n",
                  Objects->Object[i].Flags.Value[0],
                  (Objects->Object[i].Type == ITEM_SCROLL)?"scroll":"potion");
          for(j= 1; j< 4; j++)
            if(Objects->Object[i].Flags.Value[j])
              fprintf(ofp, "        %s\n",
                      spell_name(Objects->Object[i].Flags.Value[j]));
          break;
        case ITEM_WAND:
        case ITEM_STAFF:
          fprintf(ofp, "    This is a level %d %s, charged with %d of %d %s spells.\n",
                  Objects->Object[i].Flags.Value[0],
                  (Objects->Object[i].Type == ITEM_WAND)?"wand":"staff",
                  Objects->Object[i].Flags.Value[2],
                  Objects->Object[i].Flags.Value[1],
                  spell_name(Objects->Object[i].Flags.Value[3]));
          break;
        case ITEM_WEAPON:
        case ITEM_FIREWEAPON:
        case ITEM_MISSILE:
          fprintf(ofp, "    This is a %s, doing %dd%d %s damage.\n",
                  ((Objects->Object[i].Type == ITEM_WEAPON)?
                    ((Objects->Object[i].Type == ITEM_FIREWEAPON)?
                     "fire-weapon":"weapon")
                   :"missile"),
                  Objects->Object[i].Flags.Value[1],
                  Objects->Object[i].Flags.Value[2],
                  damage_name(Objects->Object[i].Flags.Value[3]));
          fprintf(ofp, "    It has a %d To-Hit factor\n",
                  Objects->Object[i].Flags.Value[0]);
          break;
        case ITEM_ARMOR:
          fprintf(ofp, "    This is armour, worth %d.%d of a maximum %d.%d AC units.\n",
                  Objects->Object[i].Flags.Value[0]/10,
                  Objects->Object[i].Flags.Value[0] - 
                  ((Objects->Object[i].Flags.Value[0]/10)*10),
                  Objects->Object[i].Flags.Value[1]/10,
                  Objects->Object[i].Flags.Value[1] - 
                  ((Objects->Object[i].Flags.Value[1]/10)*10));
          break;
        case ITEM_TRAP:
          fprintf(ofp, "    This is a level %d trap!\n",
                  Objects->Object[i].Flags.Value[0]);
          fprintf(ofp, "    It has %d charges, doing %d type attack with %d damage class\n",
                  Objects->Object[i].Flags.Value[3],
                  Objects->Object[i].Flags.Value[1],
                  Objects->Object[i].Flags.Value[2]);
          break;
        case ITEM_CONTAINER:
          if(Objects->Object[i].Flags.Value[3])
            fprintf(ofp, "    This is a corpse, which can hold %d units\n",
                    Objects->Object[i].Flags.Value[0]);
          else {
            fprintf(ofp, "    This is a container, it has a lock of type %d\n",
                    Objects->Object[i].Flags.Value[1]);
            fprintf(ofp, "    It can hold %d units\n",
                    Objects->Object[i].Flags.Value[0]);
          }
          break;
        case ITEM_DRINKCON:
          fprintf(ofp, "    This is a fluid container, it can hold %d units\n",
                  Objects->Object[i].Flags.Value[0]);
          fprintf(ofp, "    It contains %d units of %s, and is %spoisoned\n",
                  Objects->Object[i].Flags.Value[1],
                  liquid_name(Objects->Object[i].Flags.Value[2]),
                  (Objects->Object[i].Flags.Value[1]?"":"not "));
          break;
        case ITEM_NOTE:
          fprintf(ofp, "    This is a note, written in language %d\n",
                  Objects->Object[i].Flags.Value[0]);
          break;
        case ITEM_KEY:
          fprintf(ofp, "    This is a key of type %d\n",
                  Objects->Object[i].Flags.Value[0]);
          break;
        case ITEM_FOOD:
          fprintf(ofp, "    This is food worth %d units, and is %spoisoned\n",
                  Objects->Object[i].Flags.Value[0],
                  (Objects->Object[i].Flags.Value[3]?"":"not "));
          break;
        case ITEM_MONEY:
          fprintf(ofp, "    This is money, worth %d gold\n",
                  Objects->Object[i].Flags.Value[0]);
          break;
        case ITEM_TRASH:
          fprintf(ofp, "    This is trash\n");
          break;
        case ITEM_PEN:
          fprintf(ofp, "    This is a pen\n");
          break;
        case ITEM_BOARD:
          fprintf(ofp, "    This is a bulletin board\n");
          break;
        case ITEM_BOAT:
          fprintf(ofp, "    This is a boat\n");
          break;
        case ITEM_WORN:
          fprintf(ofp, "    This is a worn item\n");
          break;
        default:
          fprintf(ofp, "    The object is of type \"%s\".\n",
                  item_type_name(Objects->Object[i].Type));
          fprintf(ofp, "    The internal values are: %d %d %d %d\n",
                  Objects->Object[i].Flags.Value[0],
                  Objects->Object[i].Flags.Value[1],
                  Objects->Object[i].Flags.Value[2],
                  Objects->Object[i].Flags.Value[3]);
      }
      if(Objects->Object[i].AffectCount) {
        for(j= 0; j< Objects->Object[i].AffectCount; j++)
          switch(Objects->Object[i].Affect[j].location) {
            case APPLY_SPELL:
            case APPLY_WEAPON_SPELL:
              fprintf(ofp, "    This object contains a %s of %s\n",
                      apply_name(Objects->Object[i].Affect[j].location),
                      spell_name(Objects->Object[i].Affect[j].modifier));
              break;
            default:
              fprintf(ofp, "    This object affects %s with modifier %d\n",
                      apply_name(Objects->Object[i].Affect[j].location),
                      Objects->Object[i].Affect[j].modifier);
              break;
          }
      }
    }
    fclose(ofp);
    if(!Quiet)
      fprintf(stderr, "done.\n");
  }
}

void make_mob_report(zones *Zones, mobs *Mobs, char *outfile) {
  FILE *ofp;
  register int i, j; /* , Sum; */

  if(outfile && *outfile) {
    if(!Quiet) {
      fprintf(stderr, "Generating Mob reports...");
      fflush(stderr);
    }
    ofp= open_file(outfile, "w");
    fprintf(ofp, "MOB REPORT: %d Mobs found.\n", Mobs->Count);
    for(i= 0; i< Mobs->Count; i++) {
      if(!Quiet)
        spin(stderr);
      fprintf(ofp, "Mob \"%s\"[#%d] is known by the id string \"%s\"\n",
              mob_name(Mobs, Mobs->Mob[i].Number), Mobs->Mob[i].Number,
              Mobs->Mob[i].Name);
      fprintf(ofp, "    It belongs to zone \"%s\"[#%d]\n",
              zone_name(Zones, Mobs->Mob[i].Zone), Mobs->Mob[i].Zone);
      if(Mobs->Mob[i].LongDesc && *(Mobs->Mob[i].LongDesc))
        fprintf(ofp, "    It has a %d byte long (room) description\n        \"%s\"\n",
                strlen(Mobs->Mob[i].LongDesc), Mobs->Mob[i].LongDesc);
      if(Mobs->Mob[i].Description && *(Mobs->Mob[i].Description))
        fprintf(ofp, "    It has a %d byte look-at description\n        \"%s\"\n",
                strlen(Mobs->Mob[i].Description), Mobs->Mob[i].Description);
      fprintf(ofp, "    It is a %s %s %s, standing %dcm tall and weighing %dlbs\n",
              alignment_name(Mobs->Mob[i].Alignment),
              sex_name(Mobs->Mob[i].Sex), race_name(Mobs->Mob[i].Race),
              Mobs->Mob[i].Height, Mobs->Mob[i].Weight);
      fprintf(ofp, "    It has attained %d%s Level in these classes: %s\n",
              Mobs->Mob[i].Level, ordinal(Mobs->Mob[i].Level),
              class_name(Mobs->Mob[i].Class));
      fprintf(ofp, "    It has %dd%d%c%d hp, %d mana, and %d move\n",
              Mobs->Mob[i].HitPoints.Rolls, Mobs->Mob[i].HitPoints.Die,
              (Mobs->Mob[i].HitPoints.Modifier < 0)?'-':'+', Mobs->Mob[i].HitPoints.Modifier,
              Mobs->Mob[i].ManaPoints, Mobs->Mob[i].MovePoints);
      fprintf(ofp, "    It has an AC of %d, and a THAC0 of %d\n",
              Mobs->Mob[i].ArmourClass, Mobs->Mob[i].ToHit);
      if(Mobs->Mob[i].AttackCount) {
        fprintf(ofp, "    It gets %d attacks\n", Mobs->Mob[i].AttackCount);
        for(j= 0; j< Mobs->Mob[i].AttackCount; j++)
          fprintf(ofp, "        It has a %s attack for %dd%d%c%d damage\n",
              damage_name(Mobs->Mob[i].Attack[j].Type),
              Mobs->Mob[i].Attack[j].Rolls, Mobs->Mob[i].Attack[j].Die,
              (Mobs->Mob[i].Attack[j].Modifier < 0)?'-':'+', Mobs->Mob[i].Attack[j].Modifier);
      }
      fprintf(ofp, "    It carries %dd%d%c%d gold and is worth %dd%d%c%d experience\n",
              Mobs->Mob[i].Gold.Rolls, Mobs->Mob[i].Gold.Die,
              (Mobs->Mob[i].Gold.Modifier < 0)?'-':'+', Mobs->Mob[i].Gold.Modifier,
              Mobs->Mob[i].Experience.Rolls, Mobs->Mob[i].Experience.Die,
              (Mobs->Mob[i].Experience.Modifier < 0)?'-':'+', Mobs->Mob[i].Experience.Modifier);
      fprintf(ofp, "    Its stats are: STR %dd%d%c%d / %dd%d%c%d\n",
              Mobs->Mob[i].Strength.Rolls, Mobs->Mob[i].Strength.Die,
              (Mobs->Mob[i].Strength.Modifier < 0)?'-':'+', Mobs->Mob[i].Strength.Modifier,
              Mobs->Mob[i].ExtraStrength.Rolls, Mobs->Mob[i].ExtraStrength.Die,
              (Mobs->Mob[i].ExtraStrength.Modifier < 0)?'-':'+', Mobs->Mob[i].ExtraStrength.Modifier);
      fprintf(ofp, "                   DEX %dd%d%c%d\n",
              Mobs->Mob[i].Dexterity.Rolls, Mobs->Mob[i].Dexterity.Die,
              (Mobs->Mob[i].Dexterity.Modifier < 0)?'-':'+', Mobs->Mob[i].Dexterity.Modifier);
      fprintf(ofp, "                   CON %dd%d%c%d\n",
              Mobs->Mob[i].Constitution.Rolls, Mobs->Mob[i].Constitution.Die,
              (Mobs->Mob[i].Constitution.Modifier < 0)?'-':'+', Mobs->Mob[i].Constitution.Modifier);
      fprintf(ofp, "                   INT %dd%d%c%d\n",
              Mobs->Mob[i].Intelligence.Rolls, Mobs->Mob[i].Intelligence.Die,
              (Mobs->Mob[i].Intelligence.Modifier < 0)?'-':'+', Mobs->Mob[i].Intelligence.Modifier);
      fprintf(ofp, "                   WIS %dd%d%c%d\n",
              Mobs->Mob[i].Wisdom.Rolls, Mobs->Mob[i].Wisdom.Die,
              (Mobs->Mob[i].Wisdom.Modifier < 0)?'-':'+', Mobs->Mob[i].Wisdom.Modifier);
      if(Mobs->Mob[i].Immunity)
        fprintf(ofp, "    It is immune to %s\n", immunity_name(Mobs->Mob[i].Immunity));
      if(Mobs->Mob[i].Resistance)
        fprintf(ofp, "    It is resistant to %s\n", immunity_name(Mobs->Mob[i].Resistance));
      if(Mobs->Mob[i].Susceptible)
        fprintf(ofp, "    It is susceptible to %s\n", immunity_name(Mobs->Mob[i].Susceptible));
      fprintf(ofp, "    Its saving throws are: %d %d %d %d %d\n",
              Mobs->Mob[i].SavingThrow[0], Mobs->Mob[i].SavingThrow[1],
              Mobs->Mob[i].SavingThrow[2], Mobs->Mob[i].SavingThrow[3],
              Mobs->Mob[i].SavingThrow[4]);
      fprintf(ofp, "    It loads in %s, and has a default position of %s\n",
              position_name(Mobs->Mob[i].Position),
              position_name(Mobs->Mob[i].DefaultPosition));
      if(Mobs->Mob[i].ActFlags)
        fprintf(ofp, "    It has the following ACT flags set: %s\n",
                act_name(Mobs->Mob[i].ActFlags));
      if(Mobs->Mob[i].AffectedBy)
        fprintf(ofp, "    It is affected by: %s\n",
                affected_by_name(Mobs->Mob[i].AffectedBy));
      if(Mobs->Mob[i].Sound && *Mobs->Mob[i].Sound)
        fprintf(ofp, "    It makes a local sound\n        \"%s\"\n", Mobs->Mob[i].Sound);
      if(Mobs->Mob[i].DistantSound && *Mobs->Mob[i].DistantSound)
        fprintf(ofp, "    It makes a sound in adjacent rooms\n        \"%s\"\n", Mobs->Mob[i].DistantSound);
      if(Mobs->Mob[i].SkillCount)
        for(j= 0; j< Mobs->Mob[i].SkillCount; j++)
          fprintf(ofp, "    It knows %s\"%s\" at %d%%\n",
                  (Mobs->Mob[i].Skill[j].Recognise?"and can recognise ":""),
                  spell_name(Mobs->Mob[i].Skill[j].Number),
                  Mobs->Mob[i].Skill[j].Learned);
    }
    fclose(ofp);
    if(!Quiet)
      fprintf(stderr, "done.\n");
  }
}

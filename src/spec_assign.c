/*
 * ************************************************************************
 * *  file: spec_assign.c , Special module.                  Part of DIKUMUD *
 * *  Usage: Procedures assigning function pointers.                         *
 * *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
 * ************************************************************************* 
 */

#include <stdio.h>
#include "structs.h"
#include "db.h"

extern struct hash_header        room_db;
extern struct index_data        *mob_index;
extern struct index_data        *obj_index;
void                             boot_the_shops();
void                             assign_the_shopkeepers();

struct special_proc_entry {
  int                              vnum;
  int                              (*proc) (struct char_data *, int, char *);
};

/*
 * ********************************************************************
 * *  Assignments                                                        *
 * ******************************************************************** 
 */

/*
 * assign special procedures to mobiles 
 */
void 
assign_mobiles(void)
{
  int                              cityguard(struct char_data *ch, int cmd, char *arg);
  int                              Inquisitor(struct char_data *ch, int cmd, char *arg);
  int                              temple_labrynth_liar(struct char_data *ch, int cmd, char *arg);
  int                              AbyssGateKeeper(struct char_data *ch, int cmd, char *arg);
  int                              temple_labrynth_sentry(struct char_data *ch, int cmd, char *arg);
  int                              NudgeNudge(struct char_data *ch, int cmd, char *arg);
  int                              RustMonster(struct char_data *ch, int cmd, char *arg);
  int                              PaladinGuildGuard(struct char_data *ch, int cmd, char *arg);
  int                              tormentor(struct char_data *ch, int cmd, char *arg);
  int                              receptionist(struct char_data *ch, int cmd, char *arg);
  int                              Ned_Nutsmith(struct char_data *ch, int cmd, char *arg);
  int                              MageGuildMaster(struct char_data *ch, int cmd, char *arg);
  int                              ThiefGuildMaster(struct char_data *ch, int cmd, char *arg);
  int                              ClericGuildMaster(struct char_data *ch, int cmd, char *arg);
  int                              FighterGuildMaster(struct char_data *ch, int cmd, char *arg);
  int                              RangerGuildMaster(struct char_data *ch, int cmd, char *arg);
  int                              GenericGuildMaster(struct char_data *ch, int cmd, char *arg);
  int                              guild_guard(struct char_data *ch, int cmd, char *arg);
  int                              puff(struct char_data *ch, int cmd, char *arg);
  int                              fido(struct char_data *ch, int cmd, char *arg);
  int                              janitor(struct char_data *ch, int cmd, char *arg);
  int                              janitor_eats(struct char_data *ch, int cmd, char *arg);
  int                              mayor(struct char_data *ch, int cmd, char *arg);
  int                              eric_johnson(struct char_data *ch, int cmd, char *arg);
  int                              andy_wilcox(struct char_data *ch, int cmd, char *arg);
  int                              zombie_master(struct char_data *ch, int cmd, char *arg);
  int                              snake(struct char_data *ch, int cmd, char *arg);
  int                              thief(struct char_data *ch, int cmd, char *arg);
  int                              magic_user(struct char_data *ch, int cmd, char *arg);
  int                              cleric(struct char_data *ch, int cmd, char *arg);
  int                              ghoul(struct char_data *ch, int cmd, char *arg);
  int                              vampire(struct char_data *ch, int cmd, char *arg);
  int                              wraith(struct char_data *ch, int cmd, char *arg);
  int                              shadow(struct char_data *ch, int cmd, char *arg);
  int                              geyser(struct char_data *ch, int cmd, char *arg);

  int                              green_slime(struct char_data *ch, int cmd, char *arg);
  int                              BreathWeapon(struct char_data *ch, int cmd, char *arg);
  int                              DracoLich(struct char_data *ch, int cmd, char *arg);
  int                              Drow(struct char_data *ch, int cmd, char *arg);
  int                              Leader(struct char_data *ch, int cmd, char *arg);
  int                              citizen(struct char_data *ch, int cmd, char *arg);
  int                              ninja_master(struct char_data *ch, int cmd, char *arg);
  int                              WizardGuard(struct char_data *ch, int cmd, char *arg);
  int                              AbbarachDragon(struct char_data *ch, int cmd, char *arg);
  int                              Tytan(struct char_data *ch, int cmd, char *arg);
  int                              replicant(struct char_data *ch, int cmd, char *arg);
  int                              regenerator(struct char_data *ch, int cmd, char *arg);
  int                              blink(struct char_data *ch, int cmd, char *arg);
  int                              RepairGuy(struct char_data *ch, int cmd, char *arg);
  int                              Ringwraith(struct char_data *ch, int cmd, char *arg);
  int                              sisyphus(struct char_data *ch, int cmd, char *arg);
  int                              jabberwocky(struct char_data *ch, int cmd, char *arg);
  int                              flame(struct char_data *ch, int cmd, char *arg);
  int                              banana(struct char_data *ch, int cmd, char *arg);
  int                              paramedics(struct char_data *ch, int cmd, char *arg);
  int                              jugglernaut(struct char_data *ch, int cmd, char *arg);
  int                              delivery_elf(struct char_data *ch, int cmd, char *arg);
  int                              delivery_beast(struct char_data *ch, int cmd, char *arg);
  int                              Keftab(struct char_data *ch, int cmd, char *arg);
  int                              StormGiant(struct char_data *ch, int cmd, char *arg);
  int                              Kraken(struct char_data *ch, int cmd, char *arg);
  int                              Manticore(struct char_data *ch, int cmd, char *arg);
  int                              Fighter(struct char_data *ch, int cmd, char *arg);
  int                              AGGRESSIVE(struct char_data *ch, int cmd, char *arg);
  int                              eli_priest(struct char_data *ch, int cmd, char *arg);
  int                              firenewt(struct char_data *ch, int cmd, char *arg);

  static struct special_proc_entry specials_o[] =
  {
/*
 * { 1, puff },
 * { 3, RepairGuy },
 * { 1201, magic_user },
 * { 1204, eli_priest },
 * { 1206, RangerGuildMaster },
 * { 1208, RangerGuildMaster },
 * { 1601, RangerGuildMaster },
 * { 3005, receptionist }, 
 * { 3006, NudgeNudge },
 * { 3008, fido },
 * { 3012, cleric },
 * { 3013, magic_user },
 * { 3017, cleric },
 * { 3020, MageGuildMaster }, 
 * { 3021, ClericGuildMaster }, 
 * { 3022, ThiefGuildMaster }, 
 * { 3023, FighterGuildMaster },
 * { 3024, replicant },
 * { 3043, Ned_Nutsmith },
 * { 4006, replicant },
 * { 4622, magic_user },
 * { 4623, magic_user },
 * { 4706, wraith },
 * { 5016, cityguard },
 * { 5017, cityguard },
 * { 5027, MageGuildMaster },
 * { 5030, BreathWeapon },
 * { 5031, ThiefGuildMaster },
 * { 5032, FighterGuildMaster },
 * { 5033, ClericGuildMaster },
 * { 5034, receptionist },
 * { 5052, cityguard },
 * { 5068, cityguard },
 * { 5069, cityguard },
 * { 5078, cleric },
 * { 5079, magic_user },
 * { 5080, cityguard },
 * { 5090, cityguard },
 * { 5091, cityguard },
 * { 5436, MageGuildMaster },
 * { 5437, ClericGuildMaster },
 * { 5438, FighterGuildMaster },
 * { 5439, ThiefGuildMaster },
 * { 5440, RangerGuildMaster },
 * { 6107, FighterGuildMaster },
 * { 6108, ThiefGuildMaster },
 * { 6109, MageGuildMaster },
 * { 6110, ClericGuildMaster },
 * { 6111, magic_user },
 * { 6132, janitor_eats },
 * { 6201, cleric },
 * { 6209, magic_user },
 * { 6408, cleric },
 * { 6525, FighterGuildMaster },
 * { 6526, ClericGuildMaster },
 * { 6527, ThiefGuildMaster },
 * { 6528, MageGuildMaster },
 * { 6529, RangerGuildMaster },
 * { 6910, magic_user }, 
 * { 9000, shadow },
 * { 9002, ghoul },
 * { 9004, wraith },
 * { 9005, wraith },
 * { 10003, cleric },
 * { 10018, receptionist },
 * { 15004, FighterGuildMaster },
 * { 15013, RepairGuy },
 * { 15018, ClericGuildMaster },
 * { 15019, ClericGuildMaster },
 * { 15020, RangerGuildMaster },
 * { 15021, ClericGuildMaster },
 * { 15029, MageGuildMaster },
 * { 15036, ThiefGuildMaster },
 * { 15052, FighterGuildMaster },
 * { 15053, ClericGuildMaster },
 * { 15054, GenericGuildMaster },
 * { 20016, cleric },
 */
    {-1, NULL},
  };

  int                              i,
                                   rnum;
  char                             buf[MAX_STRING_LENGTH];

  for (i = 0; specials_o[i].vnum >= 0; i++) {
    rnum = real_mobile(specials_o[i].vnum);
    if (rnum < 0) {
      sprintf(buf, "mobile_assign: Mobile %d not found in database.",
	      specials_o[i].vnum);
      log(buf);
    } else {
      mob_index[rnum].func = specials_o[i].proc;
    }
  }

  boot_the_shops();
  assign_the_shopkeepers();
}

/*
 * assign special procedures to objects 
 */
void 
assign_objects(void)
{
  int                              board(struct char_data *ch, int cmd, char *arg);
  int                              fountain(struct char_data *ch, int cmd, char *arg);

/*
 * obj_index[real_object(3098)].func = board;
 * obj_index[real_object(3099)].func = board;
 * obj_index[real_object(3)].func = fountain;
 * obj_index[real_object(3005)].func = fountain;
 * obj_index[real_object(5099)].func = board;
 */
  InitBoards();
}

/*
 * assign special procedures to rooms 
 */
void 
assign_rooms(void)
{
  int                              chalice(struct char_data *ch, int cmd, char *arg);
  int                              dump(struct char_data *ch, int cmd, char *arg);
  int                              mail_room(struct char_data *ch, int cmd, char *arg);
  int                              kings_hall(struct char_data *ch, int cmd, char *arg);
  int                              pet_shops(struct char_data *ch, int cmd, char *arg);
  int                              pray_for_items(struct char_data *ch, int cmd, char *arg);
  int                              bank(struct char_data *ch, int cmd, char *arg);
  int                              House(struct char_data *ch, int cmd, char *arg);

  static struct special_proc_entry specials_r[] =
  {
    {1001, House},		/*
				 * Cyric's Home 
				 */
    {-1, NULL},
  };

  int                              i;
  struct room_data                *rp;

  for (i = 0; specials_r[i].vnum >= 0; i++) {
    rp = real_roomp(specials_r[i].vnum);
    if (rp == NULL) {
      log("assign_rooms: unknown room");
    } else
      rp->funct = specials_r[i].proc;
  }
}

/**************************************************************************
*  file: db.c , Database module.                          Part of DIKUMUD *
*  Usage: Loading/Saving chars, booting world, resetting etc.             *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "handler.h"
#include "limits.h"
#include "race.h"
#include "opinion.h"
#include "hash.h"
#include "spells.h"

#include "myerrors.h"

#include "lex.yy.c"

#define NEW_ZONE_SYSTEM
#define Forever		while(1)

/**************************************************************************
*  declarations of most of the 'global' variables                         *
************************************************************************ */

int                              top_of_world = -1;	/*

							 * ref to the top element of world 
							 */
struct hash_header               room_db;

struct obj_data                 *object_list = 0;	/*

							 * the global linked list of obj's 
							 */
struct char_data                *character_list = 0;	/*

							 * global l-list of chars          
							 */

struct zone_data                *zone_table;	/*

						 * table of reset data             
						 */
int                              top_of_zone_table = 0;
struct message_list              fight_messages[MAX_MESSAGES];	/*

								 * fighting messages   
								 */
struct player_index_element     *player_table = 0;	/*

							 * index to player file   
							 */
int                              top_of_p_table = 0;	/*

							 * ref to top of table             
							 */
int                              top_of_p_file = 0;

char                             credits[MAX_STRING_LENGTH];	/*

								 * the Credits List                
								 */
char                             news[MAX_STRING_LENGTH];	/*

								 * the news                        
								 */
char                             motd[MAX_STRING_LENGTH];	/*

								 * the messages of today           
								 */
char                             help[MAX_STRING_LENGTH];	/*

								 * the main help page              
								 */
char                             info[MAX_STRING_LENGTH];	/*

								 * the info text                   
								 */
char                             wizlist[MAX_STRING_LENGTH];	/*

								 * the wizlist                     
								 */
char                             wmotd[MAX_STRING_LENGTH];	/*

								 * the wizard motd                 
								 */

FILE                            *mob_f,		/*
						 * file containing mob prototypes  
						 */
                                *obj_f,		/*
						 * obj prototypes                  
						 */
                                *help_fl;	/*

						 * file for help texts (HELP <kwd>)
						 */

struct index_data               *mob_index;	/*

						 * index table for mobile file     
						 */
struct index_data               *obj_index;	/*

						 * index table for object file     
						 */
struct help_index_element       *help_index = 0;

int                              top_of_mobt = 0;	/*

							 * top of mobile index table       
							 */
int                              top_of_objt = 0;	/*

							 * top of object index table       
							 */
int                              top_of_helpt;	/*

						 * top of help index table         
						 */

struct time_info_data            time_info;	/*

						 * the infomation about the time   
						 */
struct weather_data              weather_info;	/*

						 * the infomation about the weather 
						 */

char                             TMPbuff[1620];
int                              TMPbuff_ptr = 0;
int                              ROOMcount = 0;
int                              GLINEcount = 0;
int                              LASTroomnumber = 0;
struct hash_header               room_db;

/*
 * local procedures 
 */

void                             boot_world(void);
int                              make_room(int num);
int                              parse_wld(FILE * file_to_parse);

void                             boot_zones(void);
void                             setup_dir(FILE * fl, int room, int dir);
void                             allocate_room(int new_top);
struct index_data               *generate_indices(FILE * fl, int *top);
void                             build_player_index(void);
void                             char_to_store(struct char_data *ch, struct char_file_u *st);
void                             store_to_char(struct char_file_u *st, struct char_data *ch);
int                              is_empty(int zone_nr);
void                             reset_zone(int zone);
int                              file_to_string(char *name, char *buf);
void                             renum_zone_table(void);
void                             reset_time(void);
void                             clear_char(struct char_data *ch);

/*
 * external refs 
 */
extern int                       DEBUG;
extern struct descriptor_data   *descriptor_list;
extern char                     *dirs[];
void                             load_messages(void);
void                             weather_and_time(int mode);
void                             assign_command_pointers(void);
void                             assign_spell_pointers(void);
void                             log(char *str);
int                              dice(int number, int size);
int                              number(int from, int to);
void                             boot_social_messages(void);
void                             boot_pose_messages(void);
struct help_index_element       *build_help_index(FILE * fl, int *num);
int                              DetermineExp(struct char_data *mob, int exp_flags);

/*************************************************************************
*  routines for booting the system                                       *
*********************************************************************** */

void 
boot_db(void)
{
  int                              i;
  extern int                       no_specials;

  log("Boot db -- BEGIN.");

  log("Resetting the game time and weather:");
  reset_time();

  log(" - Reading:\n\t\t- newsfile\n\t\t- credits\n\t\t- help-page\n\t\t- info\n\t\t- motd\n\t\t- wmotd");
  file_to_string(NEWS_FILE, news);
  file_to_string(CREDITS_FILE, credits);
  file_to_string(MOTD_FILE, motd);
  file_to_string(HELP_PAGE_FILE, help);
  file_to_string(INFO_FILE, info);
  file_to_string(WIZLIST_FILE, wizlist);
  file_to_string(WMOTD_FILE, wmotd);

  log(" - Opening:\n\t\t- mobile\n\t\t- object\n\t\t- help files");
  if (!(mob_f = fopen(MOB_FILE, "r"))) {
    perror("boot mobiles");
    exit(0);
  }
  if (!(obj_f = fopen(OBJ_FILE, "r"))) {
    perror("boot objects");
    exit(0);
  }
  if (!(help_fl = fopen(HELP_KWRD_FILE, "r")))
    log("   Could not open help file.");
  else
    help_index = build_help_index(help_fl, &top_of_helpt);

  log("Loading zone table.");
  boot_zones();

  log("Loading rooms.");
  boot_world();

  log("Generating index table for mobiles.");
  mob_index = generate_indices(mob_f, &top_of_mobt);

  log("Generating index table for objects.");
  obj_index = generate_indices(obj_f, &top_of_objt);

  log("Renumbering zone table.");
  renum_zone_table();

#if 1
  log("Generating player index.");
  build_player_index();
#endif

  log("Loading fight messages.");
  load_messages();

  log("Loading social messages.");
  boot_social_messages();

  log("Loading pose messages.");
  boot_pose_messages();

  log("Assigning function pointers:");
  if (!no_specials) {
    log("   Mobiles.");
    assign_mobiles();
    log("   Objects.");
    assign_objects();
    log("   Room.");
    assign_rooms();
  }
  log("   Commands.");
  assign_command_pointers();
  log("   Spells.");
  assign_spell_pointers();

  for (i = 0; i <= top_of_zone_table; i++) {
    char                            *s;
    int                              d,
                                     e;

    s = zone_table[i].name;
    d = (i ? (zone_table[i - 1].top + 1) : 0);
    e = zone_table[i].top;
    fprintf(stderr, "Performing boot-time reset of %s (rooms %d-%d).\n", s, d, e);
    reset_zone(i);
  }
  reset_q.head = reset_q.tail = 0;
  log("Boot db -- DONE.");
}

/*
 * generate index table for the player file 
 */
void 
build_player_index(void)
{
  int                              nr = -1,
                                   i;
  struct char_file_u               dummy;
  FILE                            *fl;

  if (!(fl = fopen(PLAYER_FILE, "rb+"))) {
    perror("build player index");
    exit(0);
  }
  for (; !feof(fl);) {
    fread(&dummy, sizeof(struct char_file_u), 1, fl);

    if (!feof(fl)) {		/*
				 * new record 
				 */
      /*
       * Create new entry in the list 
       */
      if (nr == -1) {
	CREATE(player_table,
	       struct player_index_element, 1);

	nr = 0;
      } else {
	if (!(player_table = (struct player_index_element *)
	      realloc(player_table, (++nr + 1) *
		      sizeof(struct player_index_element)))) {
	  perror("generate index");
	  exit(0);
	}
      }

      player_table[nr].nr = nr;

      CREATE(player_table[nr].name, char,
	     strlen                           (dummy.name) + 1);

      for (i = 0; *(player_table[nr].name + i) =
	   LOWER(*(dummy.name + i)); i++);
    }
  }

  fclose(fl);
  top_of_p_table = nr;
  top_of_p_file = top_of_p_table;
}

/*
 * generate index table for object or monster file 
 */
struct index_data               *
generate_indices(FILE * fl, int *top)
{
  int                              i = 0;
  struct index_data               *index;
  char                             buf[82];

  if (DEBUG)
    dlog("generate_indeces");
  rewind(fl);

  for (;;) {
    if (fgets(buf, sizeof(buf), fl)) {
      if (*buf == '#') {
	if (!i)			/*
				 * first cell 
				 */
	  CREATE(index, struct index_data, 1);
	else if (!(index = (struct index_data *)realloc(index, (i + 1) * sizeof(struct index_data)))) {
	  perror("load indices");
	  exit(0);
	}
	sscanf(buf, "#%d", &index[i].virtual);
	index[i].pos = ftell(fl);
	index[i].number = 0;
	index[i].func = 0;
	index[i].name = (index[i].virtual < 99999) ? fread_string(fl) : "omega";
	i++;
      } else {
	if (*buf == '$')	/*
				 * EOF 
				 */
	  break;
      }
    } else {
      fprintf(stderr, "generate indices");
      exit(0);
    }
  }
  *top = i - 2;
  return (index);
}

void 
cleanout_room(struct room_data *rp)
{
  int                              i;
  struct extra_descr_data         *exptr,
                                  *nptr;

  if (DEBUG)
    dlog("cleanout_room");
  free(rp->name);
  free(rp->description);
  for (i = 0; i < 6; i++)
    if (rp->dir_option[i]) {
      free(rp->dir_option[i]->general_description);
      free(rp->dir_option[i]->keyword);
      free(rp->dir_option[i]);
      rp->dir_option[i] = NULL;
    }
  for (exptr = rp->ex_description; exptr; exptr = nptr) {
    nptr = exptr->next;
    free(exptr->keyword);
    free(exptr->description);
    free(exptr);
  }
}

void 
completely_cleanout_room(struct room_data *rp)
{
  struct char_data                *ch;
  struct obj_data                 *obj;

  if (DEBUG)
    dlog("completely_cleanout_room");
  while (rp->people) {
    ch = rp->people;
    act("The hand of god sweeps across the land and you are swept into the Void.", FALSE, NULL, NULL, NULL, TO_VICT);
    char_from_room(ch);
    char_to_room(ch, 0);	/*
				 * send character to the void 
				 */
  }

  while (rp->contents) {
    obj = rp->contents;
    obj_from_room(obj);
    obj_to_room(obj, 0);	/*
				 * send item to the void 
				 */
  }

  cleanout_room(rp);
}

void 
load_one_room(FILE * fl, struct room_data *rp)
{
  char                             chk[50];

  int                              tmp;
  struct extra_descr_data         *new_descr;

  rp->name = fread_string(fl);
  rp->description = fread_string(fl);

  if (top_of_zone_table >= 0) {
    int                              zone;

    fscanf(fl, " %*d ");

    /*
     * OBS: Assumes ordering of input rooms 
     */

    for (zone = 0; rp->number > zone_table[zone].top && zone <= top_of_zone_table; zone++);
    if (zone > top_of_zone_table) {
      fprintf(stderr, "Room %d is outside of any zone.\n", rp->number);
      exit(0);
    }
    rp->zone = zone;
  }
  fscanf(fl, " %d ", &tmp);
  rp->room_flags = tmp;

  fscanf(fl, " %d ", &tmp);
  rp->sector_type = tmp;

  if (tmp == -1) {
    fscanf(fl, " %d", &tmp);
    rp->tele_time = tmp;
    fscanf(fl, " %d", &tmp);
    rp->tele_targ = tmp;
    fscanf(fl, " %d", &tmp);
    rp->tele_look = tmp;
    fscanf(fl, " %d", &tmp);
    rp->sector_type = tmp;
  } else {
    rp->tele_time = 0;
    rp->tele_targ = 0;
    rp->tele_look = 0;
  }

  if (tmp == 7) {		/*
				 * river 
				 */
    /*
     * read direction and rate of flow 
     */
    fscanf(fl, " %d ", &tmp);
    rp->river_speed = tmp;
    fscanf(fl, " %d ", &tmp);
    rp->river_dir = tmp;
  }
  if (IS_SET(rp->room_flags, SOUND)) {
    rp->sound = fread_string(fl);
    rp->distant_sound = fread_string(fl);
  }
  rp->funct = 0;
  rp->light = 0;		/*
				 * Zero light sources 
				 */

  for (tmp = 0; tmp <= 5; tmp++)
    rp->dir_option[tmp] = 0;

  rp->ex_description = 0;

  while (1 == fscanf(fl, " %s \n", chk)) {
    static char                      buf[MAX_INPUT_LENGTH];

    switch (*chk) {
    case 'D':
      setup_dir(fl, rp->number, atoi(chk + 1));
      break;
    case 'E':			/*
				 * extra description field 
				 */
      CREATE(new_descr, struct extra_descr_data, 1);

      new_descr->keyword = fread_string(fl);
      new_descr->description = fread_string(fl);
      new_descr->next = rp->ex_description;
      rp->ex_description = new_descr;
      break;
    case 'S':			/*
				 * end of current room 
				 */
      return;
    default:
      sprintf(buf, "unknown auxiliary code `%s' in room load of #%d",
	      chk, rp->number);
      log(buf);
      break;
    }
  }
}

#if 1
/*
 * load the rooms 
 */
void 
boot_world(void)
{
  FILE                            *fl;
  int                              virtual_nr;
  struct room_data                *rp;
  char                             buf[80];
  init_hash_table(&room_db, sizeof(struct room_data), 2048);

  character_list = 0;
  object_list = 0;

  if (!(fl = fopen(WORLD_FILE, "r"))) {
    perror("fopen");
    log("boot_world: could not open world file.");
    exit(0);
  }
  while (1 == fscanf(fl, " #%d\n", &virtual_nr)) {
    fprintf(stderr, "READING ROOM %d\r", virtual_nr);
    allocate_room(virtual_nr);
    rp = real_roomp(virtual_nr);
    bzero(rp, sizeof(*rp));
    rp->number = virtual_nr;
    load_one_room(fl, rp);
  }
  fclose(fl);
  fprintf(stderr, "\nDONE BOOT_WORLD\n");
}
#endif

void 
allocate_room(int room_number)
{
  if (room_number > top_of_world)
    top_of_world = room_number;
  else {
    fprintf(stderr, "ERROR - room number %d is out of order\n", room_number);
    exit(0);
  }
  hash_find_or_create(&room_db, room_number);
}

/*
 * read direction data 
 */
void 
setup_dir(FILE * fl, int room, int dir)
{
  int                              tmp,
                                   flag;
  struct room_data                *rp;

  rp = real_roomp(room);
  CREATE(rp->dir_option[dir], struct room_direction_data, 1);

  rp->dir_option[dir]->general_description = fread_string(fl);
  rp->dir_option[dir]->keyword = fread_string(fl);

  fscanf(fl, " %d ", &tmp);
  flag = 0;
  if (tmp > 4) {
    flag = tmp;
    tmp -= 4;
  }
  switch (tmp) {
  case 1:
    rp->dir_option[dir]->exit_info = EX_ISDOOR;
    break;
  case 2:
    rp->dir_option[dir]->exit_info = EX_ISDOOR | EX_PICKPROOF;
    break;
  case 3:
    rp->dir_option[dir]->exit_info = EX_ISDOOR | EX_SECRET;
    break;
  case 4:
    rp->dir_option[dir]->exit_info = EX_ISDOOR | EX_SECRET | EX_PICKPROOF;
    break;
  default:
    rp->dir_option[dir]->exit_info = 0;
  }

  fscanf(fl, " %d ", &tmp);
  rp->dir_option[dir]->key = tmp;

  fscanf(fl, " %d ", &tmp);
  rp->dir_option[dir]->to_room = tmp;

  if (flag) {
    rp->dir_option[dir]->exit_info |= EX_ALIAS;
    rp->dir_option[dir]->exit_alias = fread_string(fl);
  }
}

#define LOG_ZONE_ERROR(ch, type, zone, cmd) {\
  sprintf(buf, "error in zone %s cmd %d (%c) resolving %s number", \
    zone_table[zone].name, cmd, ch, type); \
  log(buf); \
  }

void 
renum_zone_table(void)
{
  int                              zone,
                                   comm;
  struct reset_com                *cmd;
  char                             buf[256];

  if (DEBUG)
    dlog("renum_zone_table");

  for (zone = 0; zone <= top_of_zone_table; zone++)
    for (comm = 0; zone_table[zone].cmd[comm].command != 'S'; comm++)
      switch ((cmd = zone_table[zone].cmd + comm)->command) {
      case 'M':
	cmd->arg1 = real_mobile(cmd->arg1);
	if (cmd->arg1 < 0)
	  LOG_ZONE_ERROR('M', "mobile", zone, comm);
	/*
	 * cmd->arg3 = real_room(cmd->arg3);
	 */
	if (cmd->arg3 < 0)
	  LOG_ZONE_ERROR('M', "room", zone, comm);
	break;

      case 'L':
	cmd->arg1 = real_mobile(cmd->arg1);
	if (cmd->arg1 < 0)
	  LOG_ZONE_ERROR('L', "mobile", zone, comm);
	break;

      case 'O':
	cmd->arg1 = real_object(cmd->arg1);
	if (cmd->arg1 < 0)
	  LOG_ZONE_ERROR('O', "object", zone, comm);
	if (cmd->arg3 != NOWHERE) {
	  /*
	   * cmd->arg3 = real_room(cmd->arg3);
	   */
	  if (cmd->arg3 < 0)
	    LOG_ZONE_ERROR('O', "room", zone, comm);
	}
	break;
      case 'G':
	cmd->arg1 = real_object(cmd->arg1);
	if (cmd->arg1 < 0)
	  LOG_ZONE_ERROR('G', "object", zone, comm);
	break;
      case 'E':
	cmd->arg1 = real_object(cmd->arg1);
	if (cmd->arg1 < 0)
	  LOG_ZONE_ERROR('E', "object", zone, comm);
	break;
      case 'P':
	cmd->arg1 = real_object(cmd->arg1);
	if (cmd->arg1 < 0)
	  LOG_ZONE_ERROR('P', "object", zone, comm);
	cmd->arg3 = real_object(cmd->arg3);
	if (cmd->arg3 < 0)
	  LOG_ZONE_ERROR('P', "object", zone, comm);
	break;
      case 'D':
	/*
	 * cmd->arg1 = real_room(cmd->arg1);
	 */
	if (cmd->arg1 < 0)
	  LOG_ZONE_ERROR('D', "room", zone, comm);
	break;
      }
}

/*
 * load the zone table and command tables 
 */
void 
boot_zones(void)
{
  FILE                            *fl;
  int                              zon = 0,
                                   cmd_no = 0,
                                   expand,
                                   tmp;
  char                            *check,
                                   buf[81];

  if (!(fl = fopen(ZONE_FILE, "r"))) {
    perror("boot_zones");
    exit(0);
  }
  for (;;) {
    fscanf(fl, " #%*d\n");
    check = fread_string(fl);

    if (*check == '$')
      break;			/*
				 * end of file 
				 */

    /*
     * alloc a new zone 
     */

    if (!zon)
      CREATE(zone_table, struct zone_data, 1);

    else if (!(zone_table =
	       (struct zone_data *)realloc(zone_table, (zon + 1) * sizeof(struct zone_data)))) {
      perror("boot_zones realloc");
      exit(0);
    }
    zone_table[zon].name = check;
    fscanf(fl, " %d ", &zone_table[zon].top);
    fscanf(fl, " %d ", &zone_table[zon].lifespan);
    fscanf(fl, " %d ", &zone_table[zon].reset_mode);

    /*
     * read the command table 
     */

    cmd_no = 0;

    for (expand = 1;;) {
      if (expand)
	if (!cmd_no)
	  CREATE(zone_table[zon].cmd, struct reset_com, 1);

	else if (!(zone_table[zon].cmd =
		   (struct reset_com *)realloc(zone_table[zon].cmd,
			(cmd_no + 1) * sizeof(struct reset_com)))) {
	  perror("reset command load");
	  exit(0);
	}
      expand = 1;

      fscanf(fl, " ");		/*
				 * skip blanks 
				 */
      fscanf(fl, "%c", &zone_table[zon].cmd[cmd_no].command);
      if (zone_table[zon].cmd[cmd_no].command == 'S')
	break;

      if (zone_table[zon].cmd[cmd_no].command == '*') {
	expand = 0;
	fgets(buf, 80, fl);	/*
				 * skip command 
				 */
	continue;
      }
      fscanf(fl, " %d %d %d",
	     &tmp,
	     &zone_table[zon].cmd[cmd_no].arg1,
	     &zone_table[zon].cmd[cmd_no].arg2);

      zone_table[zon].cmd[cmd_no].if_flag = tmp;

      if (zone_table[zon].cmd[cmd_no].command == 'M' ||
	  zone_table[zon].cmd[cmd_no].command == 'O' ||
	  zone_table[zon].cmd[cmd_no].command == 'E' ||
	  zone_table[zon].cmd[cmd_no].command == 'P' ||
	  zone_table[zon].cmd[cmd_no].command == 'D')
	fscanf(fl, " %d", &zone_table[zon].cmd[cmd_no].arg3);
      fgets(buf, 80, fl);	/*
				 * read comment 
				 */
      cmd_no++;
    }
    zon++;
  }
  top_of_zone_table = --zon;
  free(check);
  fclose(fl);
}

/*************************************************************************
*  procedures for resetting, both play-time and boot-time    *
*********************************************************************** */

/*
 * read a mobile from MOB_FILE 
 */
struct char_data                *
read_mobile(int nr, int type)
{
  int                              i;
  long                             tmp,
                                   tmp2,
                                   tmp3;
  struct char_data                *mob;
  char                             buf[100];
  char                             letter;

  i = nr;
  if (type == VIRTUAL)
    if ((nr = real_mobile(nr)) < 0) {
      sprintf(buf, "Mobile #%d does not exist.", i);
      return (0);
    }
  fseek(mob_f, mob_index[nr].pos, 0);

  CREATE(mob, struct char_data, 1);
  clear_char(mob);

/***** String data *** */

  mob->player.name = fread_string(mob_f);
  mob->player.short_descr = fread_string(mob_f);
  mob->player.long_descr = fread_string(mob_f);
  mob->player.description = fread_string(mob_f);
  mob->player.title = 0;

  /*
   * *** Numeric data *** 
   */

  mob->mult_att = 0;

  fscanf(mob_f, "%d ", &tmp);
  mob->specials.act = tmp;
  SET_BIT(mob->specials.act, ACT_ISNPC);

  fscanf(mob_f, " %d ", &tmp);
  mob->specials.affected_by = tmp;

  fscanf(mob_f, " %d ", &tmp);
  mob->specials.alignment = tmp;

  mob->player.class = CLASS_WARRIOR;

  fscanf(mob_f, " %c ", &letter);

  if (letter != 'D') {
    if ((letter == 'W') || (letter == 'M')) {
      fscanf(mob_f, " %D ", &tmp);
      mob->mult_att = tmp;
    }
    fscanf(mob_f, "\n");

    /*
     * The new easy monsters 
     */

    mob->abilities.str = 14;
    mob->abilities.intel = 14;
    mob->abilities.wis = 14;
    mob->abilities.dex = 14;
    mob->abilities.con = 14;

    fscanf(mob_f, " %D ", &tmp);
    GET_LEVEL(mob, WARRIOR_LEVEL_IND) = tmp;

    fscanf(mob_f, " %D ", &tmp);
    mob->points.hitroll = 20 - tmp;

    fscanf(mob_f, " %D ", &tmp);
    mob->points.armor = 10 * tmp;

    fscanf(mob_f, " %Dd%D+%D ", &tmp, &tmp2, &tmp3);
    mob->points.max_hit = dice(tmp, tmp2) + tmp3;
    mob->points.hit = mob->points.max_hit;

    fscanf(mob_f, " %Dd%D+%D \n", &tmp, &tmp2, &tmp3);
    mob->points.damroll = tmp3;
    mob->specials.damnodice = tmp;
    mob->specials.damsizedice = tmp2;

    mob->points.mana = 100;
    mob->points.max_mana = 100;

    mob->points.move = 100;
    mob->points.max_move = 100;

    fscanf(mob_f, " %D ", &tmp);
    if (tmp == -1) {
      fscanf(mob_f, " %D ", &tmp);
      mob->points.gold = tmp;
      fscanf(mob_f, " %D ", &tmp);
      GET_EXP(mob) = tmp;
      fscanf(mob_f, " %D \n", &tmp);
      GET_RACE(mob) = tmp;
    } else {
      mob->points.gold = tmp;
      fscanf(mob_f, " %D \n", &tmp);
      GET_EXP(mob) = tmp;
    }

    fscanf(mob_f, " %D ", &tmp);
    mob->specials.position = tmp;

    fscanf(mob_f, " %D ", &tmp);
    mob->specials.default_pos = tmp;

    fscanf(mob_f, " %D ", &tmp);
    if (tmp < 3) {
      mob->player.sex = tmp;
      mob->immune = 0;
      mob->M_immune = 0;
      mob->susc = 0;
    } else if (tmp < 6) {
      mob->player.sex = (tmp - 3);
      fscanf(mob_f, " %D ", &tmp);
      mob->immune = tmp;
      fscanf(mob_f, " %D ", &tmp);
      mob->M_immune = tmp;
      fscanf(mob_f, " %D ", &tmp);
      mob->susc = tmp;
    } else {
      mob->player.sex = 0;
      mob->immune = 0;
      mob->M_immune = 0;
      mob->susc = 0;
    }

    fscanf(mob_f, "\n");

    mob->player.class = 0;
    mob->player.time.birth = time(0);
    mob->player.time.played = 0;
    mob->player.time.logon = time(0);
    mob->player.weight = 250;
    mob->player.height = 198;

    for (i = 0; i < 3; i++)
      GET_COND(mob, i) = -1;

    for (i = 0; i < 5; i++)
      mob->specials.apply_saving_throw[i] =
	MAX(20 - GET_LEVEL(mob, WARRIOR_LEVEL_IND), 2);
    /*
     *   read in the sound string for a mobile
     */

    if (letter == 'W') {
      mob->player.sounds = fread_string(mob_f);
      mob->player.distant_snds = fread_string(mob_f);
    } else {
      mob->player.sounds = 0;
      mob->player.distant_snds = 0;
    }
  } else {
    /*
     * The old monsters are down below here 
     */

    fscanf(mob_f, "\n");

    fscanf(mob_f, " %D ", &tmp);
    mob->abilities.str = tmp;

    fscanf(mob_f, " %D ", &tmp);
    mob->abilities.intel = tmp;

    fscanf(mob_f, " %D ", &tmp);
    mob->abilities.wis = tmp;

    fscanf(mob_f, " %D ", &tmp);
    mob->abilities.dex = tmp;

    fscanf(mob_f, " %D \n", &tmp);
    mob->abilities.con = tmp;

    fscanf(mob_f, " %D ", &tmp);
    fscanf(mob_f, " %D ", &tmp2);

    mob->points.max_hit = number(tmp, tmp2);
    mob->points.hit = mob->points.max_hit;

    fscanf(mob_f, " %D ", &tmp);
    mob->points.armor = 10 * tmp;

    fscanf(mob_f, " %D ", &tmp);
    mob->points.mana = tmp;
    mob->points.max_mana = tmp;

    fscanf(mob_f, " %D ", &tmp);
    mob->points.move = tmp;
    mob->points.max_move = tmp;

    fscanf(mob_f, " %D ", &tmp);
    mob->points.gold = tmp;

    fscanf(mob_f, " %D \n", &tmp);
    GET_EXP(mob) = tmp;

    fscanf(mob_f, " %D ", &tmp);
    mob->specials.position = tmp;

    fscanf(mob_f, " %D ", &tmp);
    mob->specials.default_pos = tmp;

    fscanf(mob_f, " %D ", &tmp);
    mob->player.sex = tmp;

    fscanf(mob_f, " %D ", &tmp);
    mob->player.class = tmp;

    fscanf(mob_f, " %D ", &tmp);
    GET_LEVEL(mob, WARRIOR_LEVEL_IND) = tmp;

    fscanf(mob_f, " %D ", &tmp);
    mob->player.time.birth = time(0);
    mob->player.time.played = 0;
    mob->player.time.logon = time(0);

    fscanf(mob_f, " %D ", &tmp);
    mob->player.weight = tmp;

    fscanf(mob_f, " %D \n", &tmp);
    mob->player.height = tmp;

    for (i = 0; i < 3; i++) {
      fscanf(mob_f, " %D ", &tmp);
      GET_COND(mob, i) = tmp;
    }
    fscanf(mob_f, " \n ");

    for (i = 0; i < 5; i++) {
      fscanf(mob_f, " %D ", &tmp);
      mob->specials.apply_saving_throw[i] = tmp;
    }

    fscanf(mob_f, " \n ");

    /*
     * Set the damage as some standard 1d6 
     */
    mob->points.damroll = 0;
    mob->specials.damnodice = 1;
    mob->specials.damsizedice = 6;

    /*
     * Calculate THAC0 as a formular of Level 
     */
    mob->points.hitroll = MAX(1, GET_LEVEL(mob, WARRIOR_LEVEL_IND) - 3);
  }

  mob->tmpabilities = mob->abilities;

  for (i = 0; i < MAX_WEAR; i++)
    mob->equipment[i] = 0;

  mob->nr = nr;

  mob->desc = 0;

  if (!IS_SET(mob->specials.act, ACT_ISNPC))
    SET_BIT(mob->specials.act, ACT_ISNPC);

  /*
   * insert in list 
   */

  mob->next = character_list;
  character_list = mob;

  mob_index[nr].number++;

  return (mob);
}

/*
 * read an object from OBJ_FILE 
 */

struct obj_data                 *
read_object(int nr, int type)
{
  struct obj_data                 *obj;
  int                              tmp,
                                   i;
  char                             chk[50],
                                   buf[100];
  struct extra_descr_data         *new_descr;

  i = nr;
  if (type == VIRTUAL) {
    nr = real_object(nr);
  }
  if (nr < 0 || nr > top_of_objt) {
    sprintf(buf, "Object #%d does not exist.", i);
    return (0);
  }
  fseek(obj_f, obj_index[nr].pos, 0);
  CREATE(obj, struct obj_data, 1);
  clear_object(obj);

  /*
   * *** string data *** 
   */

  obj->name = fread_string(obj_f);
  obj->short_description = fread_string(obj_f);
  obj->description = fread_string(obj_f);
  obj->action_description = fread_string(obj_f);

  /*
   * *** numeric data *** 
   */

  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.type_flag = tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.extra_flags = tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.wear_flags = tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.value[0] = tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.value[1] = tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.value[2] = tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.value[3] = tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.weight = tmp;
  fscanf(obj_f, " %d \n", &tmp);
  obj->obj_flags.cost = tmp;
  fscanf(obj_f, " %d \n", &tmp);
  obj->obj_flags.cost_per_day = tmp;

  /*
   * *** extra descriptions *** 
   */

  obj->ex_description = 0;

  while (fscanf(obj_f, " %s \n", chk), *chk == 'E') {
    CREATE(new_descr, struct extra_descr_data, 1);

    new_descr->keyword = fread_string(obj_f);
    new_descr->description = fread_string(obj_f);

    new_descr->next = obj->ex_description;
    obj->ex_description = new_descr;
  }

  for (i = 0; (i < MAX_OBJ_AFFECT) && (*chk == 'A'); i++) {
    fscanf(obj_f, " %d ", &tmp);
    obj->affected[i].location = tmp;
    fscanf(obj_f, " %d \n", &tmp);
    obj->affected[i].modifier = tmp;
    fscanf(obj_f, " %s \n", chk);
  }

  for (; (i < MAX_OBJ_AFFECT); i++) {
    obj->affected[i].location = APPLY_NONE;
    obj->affected[i].modifier = 0;
  }

  obj->in_room = NOWHERE;
  obj->next_content = 0;
  obj->carried_by = 0;
  obj->equipped_by = 0;
  obj->in_obj = 0;
  obj->contains = 0;
  obj->item_number = nr;

  obj->next = object_list;
  object_list = obj;

  obj_index[nr].number++;

  if (ITEM_TYPE(obj) == ITEM_BOARD) {
    InitABoard(obj);
  }
  return (obj);
}

#define ZO_DEAD  999

/*
 * update zone ages, queue for reset if necessary, and dequeue when possible 
 */
void 
zone_update(void)
{
  int                              i;
  struct reset_q_element          *update_u,
                                  *temp,
                                  *tmp2;
  extern struct reset_q_type       reset_q;

  if (DEBUG)
    dlog("zone_update");
  /*
   * enqueue zones 
   */

  for (i = 0; i <= top_of_zone_table; i++) {
    if (zone_table[i].age < zone_table[i].lifespan && zone_table[i].reset_mode)
      (zone_table[i].age)++;
    else if (zone_table[i].age < ZO_DEAD && zone_table[i].reset_mode) {
      /*
       * enqueue zone 
       */
      CREATE(update_u, struct reset_q_element, 1);

      update_u->zone_to_reset = i;
      update_u->next = 0;

      if (!reset_q.head)
	reset_q.head = reset_q.tail = update_u;
      else {
	reset_q.tail->next = update_u;
	reset_q.tail = update_u;
      }
      zone_table[i].age = ZO_DEAD;
    }
  }

  /*
   * dequeue zones (if possible) and reset 
   */

  for (update_u = reset_q.head; update_u; update_u = tmp2) {
    if (update_u->zone_to_reset > top_of_zone_table) {

      /*
       * this may or may not work may result in some lost memory
       * but the loss is not signifigant over the short run
       */
      update_u->zone_to_reset = 0;
      update_u->next = 0;
    }
    tmp2 = update_u->next;

    if (zone_table[update_u->zone_to_reset].reset_mode == 2 || is_empty(update_u->zone_to_reset)) {
      reset_zone(update_u->zone_to_reset);
      /*
       * dequeue 
       */

      if (update_u == reset_q.head)
	reset_q.head = reset_q.head->next;
      else {
	for (temp = reset_q.head; temp->next != update_u; temp = temp->next);
	if (!update_u->next)
	  reset_q.tail = temp;
	temp->next = update_u->next;
      }
      free(update_u);
    }
  }
}

#define ZCMD zone_table[zone].cmd[cmd_no]
#define ZNAME zone_table[zone].name

/*
 * execute the reset command table of a given zone 
 */
void 
reset_zone(int zone)
{
  int                              cmd_no,
                                   last_cmd = 1;
  char                             buf[256];
  struct char_data                *mob,
                                  *mob2;
  struct obj_data                 *obj,
                                  *obj_to;
  struct room_data                *rp;
  struct char_data                *last_mob_loaded;

  if (DEBUG)
    dlog("reset_zone");
  sprintf(buf, "Reseting Zone Named:%s", ZNAME);
  log(buf);
  for (cmd_no = 0;; cmd_no++) {
    if (ZCMD.command == 'S')
      break;

    if (last_cmd || !ZCMD.if_flag)
      switch (ZCMD.command) {
      case 'M':		/*
				 * read a mobile 
				 */
	if (mob_index[ZCMD.arg1].number < ZCMD.arg2) {
	  mob = read_mobile(ZCMD.arg1, REAL);
	  char_to_room(mob, ZCMD.arg3);
	  last_mob_loaded = mob;
	  last_cmd = 1;
	} else
	  last_cmd = 0;
	break;

      case 'L':		/*
				 * make a mob follow another 
				 */

	if (mob_index[ZCMD.arg1].number < ZCMD.arg2) {
	  mob = read_mobile(ZCMD.arg1, REAL);
	  char_to_room(mob, last_mob_loaded->in_room);
	  add_follower(mob, last_mob_loaded);
	  SET_BIT(mob->specials.affected_by, AFF_CHARM);
	  SET_BIT(mob->specials.act, ACT_SENTINEL);
	  last_cmd = 1;
	} else
	  last_cmd = 0;
	break;

      case 'O':		/*
				 * read an object 
				 */
	if (obj_index[ZCMD.arg1].number < ZCMD.arg2)
	  if (ZCMD.arg3 >= 0 && ((rp = real_roomp(ZCMD.arg3)) != NULL)) {
	    if (!get_obj_in_list_num(ZCMD.arg1, rp->contents)
		&& (obj = read_object(ZCMD.arg1, REAL))) {
	      obj_to_room(obj, ZCMD.arg3);
	      last_cmd = 1;
	    } else
	      last_cmd = 0;
	  } else if (obj = read_object(ZCMD.arg1, REAL)) {
	    sprintf(buf, "Error finding room #%d", ZCMD.arg3);
	    log(buf);
	    extract_obj(obj);
	    last_cmd = 1;
	  } else
	    last_cmd = 0;
	break;

      case 'P':		/*
				 * object to object 
				 */
	if (obj_index[ZCMD.arg1].number < ZCMD.arg2) {
	  obj = read_object(ZCMD.arg1, REAL);
	  obj_to = get_obj_num(ZCMD.arg3);
	  if (obj_to) {
	    obj_to_obj(obj, obj_to);
	    last_cmd = 1;
	  } else {
	    last_cmd = 0;
	  }
	} else
	  last_cmd = 0;
	break;

      case 'G':		/*
				 * obj_to_char 
				 */
	if (obj_index[ZCMD.arg1].number < ZCMD.arg2 &&
	    (obj = read_object(ZCMD.arg1, REAL))) {
	  obj_to_char(obj, mob);
	  last_cmd = 1;
	} else
	  last_cmd = 0;
	break;

      case 'H':		/*
				 * hatred to char 
				 */
	if (AddHatred(mob, ZCMD.arg1, ZCMD.arg2))
	  last_cmd = 1;
	else
	  last_cmd = 0;
	break;

      case 'F':		/*
				 * fear to char 
				 */

	if (AddFears(mob, ZCMD.arg1, ZCMD.arg2))
	  last_cmd = 1;
	else
	  last_cmd = 0;
	break;

      case 'E':		/*
				 * object to equipment list 
				 */
	if (obj_index[ZCMD.arg1].number < ZCMD.arg2 &&
	    (obj = read_object(ZCMD.arg1, REAL))) {
	  if (ZCMD.arg3 > WIELD_TWOH)
	    log("BAD EQUIP in zone reboot.");
	  else
	    equip_char(mob, obj, ZCMD.arg3);
	  last_cmd = 1;
	} else
	  last_cmd = 0;
	break;

      case 'D':		/*
				 * set state of door 
				 */
	rp = real_roomp(ZCMD.arg1);
	if (rp && rp->dir_option[ZCMD.arg2]) {
	  switch (ZCMD.arg3) {
	  case 0:
	    REMOVE_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_LOCKED);
	    REMOVE_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_CLOSED);
	    break;
	  case 1:
	    SET_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_CLOSED);
	    REMOVE_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_LOCKED);
	    break;
	  case 2:
	    SET_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_LOCKED);
	    SET_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_CLOSED);
	    break;
	  }
	  last_cmd = 1;
	} else {
	  /*
	   * that exit doesn't exist anymore 
	   */
	}
	break;

      default:
	sprintf(buf, "Undefd cmd in reset table; zone %d cmd %d.\n\r", zone, cmd_no);
	log(buf);
	break;
    } else
      last_cmd = 0;
  }
  zone_table[zone].age = 0;
}

#undef ZCMD
#undef ZNAME

/*
 * for use in reset_zone; return TRUE if zone 'nr' is free of PC's  
 */
int 
is_empty(int zone_nr)
{
  struct descriptor_data          *i;

  for (i = descriptor_list; i; i = i->next)
    if (!i->connected)
      if (i->character->in_room != NOWHERE) {
	if (real_roomp(i->character->in_room)->zone == zone_nr)
	  return (0);
      }
  return (1);
}

/*************************************************************************
*  stuff related to the save/load player system                 *
*********************************************************************** */

int 
load_char(char *name, struct char_file_u *char_element)
{
  FILE                            *fl;
  int                              player_i;
  char                             buf[256];
  char                             tname[40];
  char                            *t_ptr;

  strcpy(tname, name);
  t_ptr = tname;
  for (; *t_ptr != '\0'; t_ptr++)
    *t_ptr = LOWER(*t_ptr);

  sprintf(buf, "ply/%s.p", tname);
  if (!(fl = fopen(buf, "r+b")))
    return (-1);

  fread(char_element, sizeof(struct char_file_u), 1, fl);

  fclose(fl);
/*
 * **  Kludge for ressurection
 */
  char_element->talks[2] = TRUE;
  return (1);
}

/*
 * copy data from the file structure to a char struct 
 */
void 
store_to_char(struct char_file_u *st, struct char_data *ch)
{
  int                              i;
  long                             t;

  GET_SEX(ch) = st->sex;
  ch->player.class = st->class;

  for (i = MAGE_LEVEL_IND; i <= DRUID_LEVEL_IND; i++)
    ch->player.level[i] = st->level[i];

  GET_RACE(ch) = st->race;

  t = time(0);
  ch->desc->idle_time = t;

  ch->player.short_descr = 0;
  ch->player.long_descr = 0;

  if (*st->title) {
    CREATE(ch->player.title, char, strlen(st->title) + 1);

    strcpy(ch->player.title, st->title);
  } else
    GET_TITLE(ch) = 0;

  if (*st->pre_title) {
    CREATE(ch->player.pre_title, char, strlen(st->pre_title) + 1);

    strcpy(ch->player.pre_title, st->pre_title);
  } else
    GET_PRETITLE(ch) = 0;

  if (*st->description) {
    CREATE(ch->player.description, char,
	   strlen                           (st->description) + 1);

    strcpy(ch->player.description, st->description);
  } else
    ch->player.description = 0;

  ch->player.hometown = st->hometown;

  ch->player.time.birth = st->birth;
  ch->player.time.played = st->played;
  ch->player.time.logon = time(0);

  for (i = 0; i <= MAX_TOUNGE - 1; i++)
    ch->player.talks[i] = st->talks[i];

  ch->player.weight = st->weight;
  ch->player.height = st->height;

  ch->abilities = st->abilities;
  ch->tmpabilities = st->abilities;
  ch->points = st->points;

  for (i = 0; i <= MAX_SKILLS - 1; i++)
    ch->skills[i] = st->skills[i];

  ch->specials.spells_to_learn = st->spells_to_learn;
  ch->specials.alignment = st->alignment;

  ch->specials.act = st->act;
  ch->specials.new_act = st->new_act;
  ch->specials.carry_weight = 0;
  ch->specials.carry_items = 0;
  ch->points.armor = 100;
  ch->points.hitroll = 0;
  ch->points.damroll = 0;

  CREATE(GET_NAME(ch), char, strlen(st->name) + 1);

  strcpy(GET_NAME(ch), st->name);

  /*
   * Not used as far as I can see (Michael) 
   */
  for (i = 0; i <= 5; i++)
    ch->specials.apply_saving_throw[i] = st->apply_saving_throw[i];

  for (i = 0; i <= 2; i++)
    GET_COND(ch, i) = st->conditions[i];

  /*
   * Add all spell effects 
   */

  for (i = 0; i < MAX_AFFECT; i++) {
    if (st->affected[i].type)
      affect_to_char(ch, &st->affected[i]);
  }

  ch->in_room = st->load_room;
  affect_total(ch);
}

/*
 * copy vital data from a players char-structure to the file structure 
 */
void 
char_to_store(struct char_data *ch, struct char_file_u *st)
{
  int                              i;
  struct affected_type            *af;
  struct obj_data                 *char_eq[MAX_WEAR];

  /*
   * Unaffect everything a character can be affected by 
   */

  for (i = 0; i < MAX_WEAR; i++) {
    if (ch->equipment[i])
      char_eq[i] = unequip_char(ch, i);
    else
      char_eq[i] = 0;
  }

  for (af = ch->affected, i = 0; i < MAX_AFFECT; i++) {
    if (af) {
      st->affected[i] = *af;
      st->affected[i].next = 0;

      /*
       * subtract effect of the spell or the effect will be doubled 
       */
      affect_modify(ch, st->affected[i].location,
		    st->affected[i].modifier,
		    st->affected[i].bitvector, FALSE);
      af = af->next;
    } else {
      st->affected[i].type = 0;	/*
				 * Zero signifies not used 
				 */
      st->affected[i].duration = 0;
      st->affected[i].modifier = 0;
      st->affected[i].location = 0;
      st->affected[i].bitvector = 0;
      st->affected[i].next = 0;
    }
  }

  if ((i >= MAX_AFFECT) && af && af->next)
    log("WARNING: OUT OF STORE ROOM FOR AFFECTED TYPES!!!");

  ch->tmpabilities = ch->abilities;

  st->birth = ch->player.time.birth;
  st->played = ch->player.time.played;
  st->played += (long)(time(0) - ch->player.time.logon);
  st->last_logon = time(0);

  ch->player.time.played = st->played;
  ch->player.time.logon = time(0);

  st->hometown = ch->player.hometown;
  st->weight = GET_WEIGHT(ch);
  st->height = GET_HEIGHT(ch);
  st->sex = GET_SEX(ch);
  st->class = ch->player.class;

  for (i = MAGE_LEVEL_IND; i <= DRUID_LEVEL_IND; i++)
    st->level[i] = ch->player.level[i];

  st->race = GET_RACE(ch);
  st->abilities = ch->abilities;
  st->points = ch->points;
  st->alignment = ch->specials.alignment;
  st->spells_to_learn = ch->specials.spells_to_learn;
  st->act = ch->specials.act;
  st->new_act = ch->specials.new_act;

  st->points.armor = 100;
  st->points.hitroll = 0;
  st->points.damroll = 0;

  if (GET_TITLE(ch))
    strcpy(st->title, GET_TITLE(ch));
  else
    *st->title = '\0';

  if (GET_PRETITLE(ch))
    strcpy(st->pre_title, GET_PRETITLE(ch));
  else
    *st->pre_title = '\0';

  if (ch->player.description)
    strcpy(st->description, ch->player.description);
  else
    *st->description = '\0';

  for (i = 0; i <= MAX_TOUNGE - 1; i++)
    st->talks[i] = ch->player.talks[i];

  for (i = 0; i <= MAX_SKILLS - 1; i++)
    st->skills[i] = ch->skills[i];

  strcpy(st->name, GET_NAME(ch));

  for (i = 0; i <= 4; i++)
    st->apply_saving_throw[i] = ch->specials.apply_saving_throw[i];

  for (i = 0; i <= 2; i++)
    st->conditions[i] = GET_COND(ch, i);

  for (af = ch->affected, i = 0; i < MAX_AFFECT; i++) {
    if (af) {
      /*
       * Add effect of the spell or it will be lost 
       */
      /*
       * When saving without quitting               
       */

      affect_modify(ch, st->affected[i].location,
		    st->affected[i].modifier,
		    st->affected[i].bitvector, TRUE);
      af = af->next;
    }
  }

  for (i = 0; i < MAX_WEAR; i++) {
    if (char_eq[i])
      equip_char(ch, char_eq[i], i);
  }
  affect_total(ch);
}				/*
				 * Char to store 
				 */

/*
 * create a new entry in the in-memory index table for the player file 
 */

int 
create_entry(char *name)
{
  int                              i;

  fprintf(stderr, "PLAYER TABLE = %d\n", top_of_p_table);
  fprintf(stderr, "NAME = %s\n", name);

  if (top_of_p_table == -1 || player_table == NULL) {
    CREATE(player_table, struct player_index_element, 1);

    top_of_p_table = 0;
  } else if (!(player_table = (struct player_index_element *)realloc(player_table, sizeof(struct player_index_element) * (++top_of_p_table + 1)))) {
    perror("create entry");
    exit(1);
  }
  CREATE(player_table[top_of_p_table].name, char, strlen(name) + 1);

  /*
   * copy lowercase equivalent of name to table field 
   */
  for (i = 0; *(player_table[top_of_p_table].name + i) =
       LOWER(*(name + i)); i++);

  player_table[top_of_p_table].nr = top_of_p_table;

  return (top_of_p_table);
}

/*
 * write the vital data of a player to the player file 
 */

void 
save_char(struct char_data *ch, sh_int load_room)
{
  struct char_file_u               st;
  FILE                            *fl;
  char                             mode[4];
  int                              expand;
  char                             buf[256];
  char                             name[40];
  char                            *t_ptr;

  if (IS_NPC(ch) || !ch->desc)
    return;

  char_to_store(ch, &st);

  st.load_room = load_room;
  strcpy(st.pwd, ch->desc->pwd);

  strcpy(name, GET_NAME(ch));
  t_ptr = name;
  for (; *t_ptr != '\0'; t_ptr++)
    *t_ptr = LOWER(*t_ptr);

  sprintf(buf, "ply/%s.p", name);
  if (!(fl = fopen(buf, "w+b"))) {
    perror("save char");
    exit(1);
  }
  fwrite(&st, sizeof(struct char_file_u), 1, fl);

  fclose(fl);
}

/*
 * for possible later use with qsort 
 */
int 
compare(struct player_index_element *arg1, struct player_index_element *arg2)
{
  if (DEBUG)
    dlog("compare");
  return (str_cmp(arg1->name, arg2->name));
}

/************************************************************************
*  procs of a (more or less) general utility nature     *
********************************************************************** */

/*
 * read and allocate space for a '~'-terminated string from a given file 
 */
char                            *
fread_string(FILE * fl)
{
  char                             buf[MAX_STRING_LENGTH],
                                   tmp[500];
  char                            *rslt;
  register char                   *point;
  int                              flag;

  if (DEBUG)
    dlog("fread_string");
  bzero(buf, sizeof(buf));

  do {
    if (!fgets(tmp, MAX_STRING_LENGTH, fl)) {
      perror("fread_str");
      log("File read error.");
      return ("Empty");
    }
    if (strlen(tmp) + strlen(buf) + 1 > MAX_STRING_LENGTH) {
      log("fread_string: string too large (db.c)");
      exit(0);
    } else
      strcat(buf, tmp);

    for (point = buf + strlen(buf) - 2; point >= buf && isspace(*point);
	 point--);
    if (flag = (*point == '~'))
      if (*(buf + strlen(buf) - 3) == '\n') {
	*(buf + strlen(buf) - 2) = '\r';
	*(buf + strlen(buf) - 1) = '\0';
      } else
	*(buf + strlen(buf) - 2) = '\0';
    else {
      *(buf + strlen(buf) + 1) = '\0';
      *(buf + strlen(buf)) = '\r';
    }
  } while (!flag);

  /*
   * do the allocate boogie  
   */

  if (strlen(buf) > 0) {
    CREATE(rslt, char, strlen        (buf) + 1);

    strcpy(rslt, buf);
  } else
    rslt = 0;
  return (rslt);
}

/*
 * release memory allocated for a char struct 
 */
void 
free_char(struct char_data *ch)
{
  struct affected_type            *af;

  if (DEBUG)
    dlog("free_char");
  free(GET_NAME(ch));

  if (ch->player.title)
    free(ch->player.title);
  if (ch->player.pre_title)
    free(ch->player.pre_title);
  if (ch->player.short_descr)
    free(ch->player.short_descr);
  if (ch->player.long_descr)
    free(ch->player.long_descr);
  if (ch->player.description)
    free(ch->player.description);
  if (ch->player.sounds)
    free(ch->player.sounds);
  if (ch->player.distant_snds)
    free(ch->player.distant_snds);

  for (af = ch->affected; af; af = af->next)
    affect_remove(ch, af);
  free(ch);
}

/*
 * release memory allocated for an obj struct 
 */
void 
free_obj(struct obj_data *obj)
{
  struct extra_descr_data         *this,
                                  *next_one;

  free(obj->name);
  if (obj->description)
    free(obj->description);
  if (obj->short_description)
    free(obj->short_description);
  if (obj->action_description)
    free(obj->action_description);

  for (this = obj->ex_description;
       (this != 0); this = next_one) {
    next_one = this->next;
    if (this->keyword)
      free(this->keyword);
    if (this->description)
      free(this->description);
    free(this);
  }

  free(obj);
}

/*
 * read contents of a text file, and place in buf 
 */
int 
file_to_string(char *name, char *buf)
{
  FILE                            *fl;
  char                             tmp[100];

  *buf = '\0';

  if (!(fl = fopen(name, "r"))) {
    perror("file-to-string");
    *buf = '\0';
    return (-1);
  }
  do {
    fgets(tmp, 99, fl);

    if (!feof(fl)) {
      if (strlen(buf) + strlen(tmp) + 2 > MAX_STRING_LENGTH) {
	log("fl->strng: string too big (db.c, file_to_string)");
	*buf = '\0';
	fclose(fl);
	return (-1);
      }
      strcat(buf, tmp);
      *(buf + strlen(buf) + 1) = '\0';
      *(buf + strlen(buf)) = '\r';
    }
  }
  while (!feof(fl));

  fclose(fl);

  return (0);
}

/*
 * clear some of the the working variables of a char 
 */
void 
reset_char(struct char_data *ch)
{
  char                             buf[100];
  extern struct dex_app_type       dex_app[];

  int                              i;

  for (i = 0; i < MAX_WEAR; i++)	/*
					 * Initializing 
					 */
    ch->equipment[i] = 0;

  ch->mail = NULL;
  ch->followers = 0;
  ch->master = 0;
  ch->carrying = 0;
  ch->next = 0;

  ch->immune = 0;
  ch->M_immune = 0;
  ch->susc = 0;
  ch->mult_att = 0;

  if (!GET_RACE(ch))
    GET_RACE(ch) = RACE_HUMAN;
  if (GET_RACE(ch) == RACE_DWARF || GET_RACE(ch) == RACE_GNOME) {
    if (!IS_AFFECTED(ch, AFF_INFRAVISION))
      SET_BIT(ch->specials.affected_by, AFF_INFRAVISION);
  }
  if ((ch->player.class == 3) && (GET_LEVEL(ch, THIEF_LEVEL_IND))) {
    ch->player.class = 8;
    send_to_char("Setting your class to THIEF only.\n\r", ch);
  }
  for (i = 0; i <= 5; i++) {
    if (GET_LEVEL(ch, i) > LOKI) {
      GET_LEVEL(ch, i) = 51;
    }
  }

  SET_BIT(ch->specials.act, PLR_ECHO);
  SET_BIT(ch->specials.act, PLR_PAGER);

  ch->hunt_dist = 0;
  ch->hatefield = 0;
  ch->fearfield = 0;
  ch->hates.clist = 0;
  ch->fears.clist = 0;

  /*
   * AC adjustment 
   */

  GET_AC(ch) = 100;
  GET_AC(ch) += dex_app[GET_DEX(ch)].defensive;
  if (affected_by_spell(ch, SPELL_ARMOR))
    GET_AC(ch) -= 20;
  if (affected_by_spell(ch, SPELL_SHIELD))
    GET_AC(ch) -= 10;
  if (affected_by_spell(ch, SPELL_STONE_SKIN))
    GET_AC(ch) -= 30;
  if (affected_by_spell(ch, SPELL_BLINDNESS))
    GET_AC(ch) += 20;
  if (affected_by_spell(ch, SPELL_INVISIBLE))
    GET_AC(ch) -= 40;

  if (GET_AC(ch) > 100)
    GET_AC(ch) = 100;

  ch->next_fighting = 0;
  ch->next_in_room = 0;
  ch->specials.fighting = 0;
  ch->specials.position = POSITION_STANDING;
  ch->specials.default_pos = POSITION_STANDING;
  ch->specials.carry_weight = 0;
  ch->specials.carry_items = 0;
  ch->specials.mounted_on = 0;
  ch->specials.ridden_by = 0;

  if (GET_HIT(ch) <= 0)
    GET_HIT(ch) = 1;
  if (GET_MOVE(ch) <= 0)
    GET_MOVE(ch) = 1;
  if (GET_MANA(ch) <= 0)
    GET_MANA(ch) = 1;

  ch->points.max_mana = 0;
  ch->points.max_move = 0;

  if (IS_IMMORTAL(ch)) {
    GET_BANK(ch) = 0;
    GET_GOLD(ch) = 100000;
  }
  if (GET_BANK(ch) > 500000) {
    sprintf(buf, "%s has %d coins in bank.", GET_NAME(ch), GET_BANK(ch));
    log(buf);
  }
  if (GET_GOLD(ch) > 500000) {
    sprintf(buf, "%s has %d coins.", GET_NAME(ch), GET_GOLD(ch));
    log(buf);
  }
}

/*
 * clear ALL the working variables of a char and do NOT free any space alloc'ed
 */
void 
clear_char(struct char_data *ch)
{
  memset(ch, '\0', sizeof(struct char_data));

  ch->in_room = NOWHERE;
  ch->specials.mounted_on = 0;
  ch->specials.ridden_by = 0;
  ch->hates.clist = 0;
  ch->fears.clist = 0;
  ch->specials.was_in_room = NOWHERE;
  ch->specials.position = POSITION_STANDING;
  ch->specials.default_pos = POSITION_STANDING;
  GET_AC(ch) = 100;		/*
				 * Basic Armor 
				 */
}

void 
clear_object(struct obj_data *obj)
{
  if (DEBUG)
    dlog("clear_object");
  memset(obj, '\0', sizeof(struct obj_data));

  obj->item_number = -1;
  obj->in_room = NOWHERE;
}

/*
 * initialize a new character only if class is set 
 */
void 
init_char(struct char_data *ch)
{
  int                              i;

  /*
   * *** if this is our first player --- he be God *** 
   */

  if (!strcmp(GET_NAME(ch), "Quixadhal")) {
    GET_EXP(ch) = 24000000;
    GET_LEVEL(ch, 0) = IMPLEMENTOR;
    GET_LEVEL(ch, 1) = IMPLEMENTOR;
    GET_LEVEL(ch, 2) = IMPLEMENTOR;
    GET_LEVEL(ch, 3) = IMPLEMENTOR;
    GET_LEVEL(ch, 4) = IMPLEMENTOR;
    GET_LEVEL(ch, 5) = IMPLEMENTOR;
  }
  set_title(ch);

  ch->player.short_descr = 0;
  ch->player.long_descr = 0;
  ch->player.description = 0;

  ch->player.hometown = DEFAULT_HOME;	/*
					 * Rental area of shylar 
					 */

  ch->player.time.birth = time(0);
  ch->player.time.played = 0;
  ch->player.time.logon = time(0);

  for (i = 0; i < MAX_TOUNGE; i++)
    ch->player.talks[i] = 0;

  GET_STR(ch) = 9;
  GET_INT(ch) = 9;
  GET_WIS(ch) = 9;
  GET_DEX(ch) = 9;
  GET_CON(ch) = 9;

  /*
   * make favors for sex 
   */
  if (GET_RACE(ch) == RACE_HUMAN) {
    if (ch->player.sex == SEX_MALE) {
      ch->player.weight = number(120, 180);
      ch->player.height = number(160, 200);
    } else {
      ch->player.weight = number(100, 160);
      ch->player.height = number(150, 180);
    }
  } else if (GET_RACE(ch) == RACE_DWARF) {
    if (ch->player.sex == SEX_MALE) {
      ch->player.weight = number(120, 180);
      ch->player.height = number(100, 150);
    } else {
      ch->player.weight = number(100, 160);
      ch->player.height = number(100, 150);
    }

  } else if (GET_RACE(ch) == RACE_ELVEN) {
    if (ch->player.sex == SEX_MALE) {
      ch->player.weight = number(100, 150);
      ch->player.height = number(160, 200);
    } else {
      ch->player.weight = number(80, 230);
      ch->player.height = number(150, 180);
    }
  } else {
    if (ch->player.sex == SEX_MALE) {
      ch->player.weight = number(120, 180);
      ch->player.height = number(160, 200);
    } else {
      ch->player.weight = number(100, 160);
      ch->player.height = number(150, 180);
    }
  }

  ch->points.mana = GET_MAX_MANA(ch);
  ch->points.hit = GET_MAX_HIT(ch);
  ch->points.move = GET_MAX_MOVE(ch);

  ch->points.armor = 100;

  for (i = 0; i <= MAX_SKILLS - 1; i++) {
    if (GetMaxLevel(ch) < IMPLEMENTOR) {
      ch->skills[i].learned = 0;
      ch->skills[i].recognise = FALSE;
    } else {
      ch->skills[i].learned = 100;
      ch->skills[i].recognise = FALSE;
    }
  }

  ch->specials.affected_by = 0;
  ch->specials.spells_to_learn = 0;

  for (i = 0; i < 5; i++)
    ch->specials.apply_saving_throw[i] = 0;

  for (i = 0; i < 3; i++)
    GET_COND(ch, i) = (GetMaxLevel(ch) > 51 ? -1 : 24);
}

struct room_data                *
real_roomp(int virtual)
{
  return hash_find(&room_db, virtual);
}

void 
print_hash_ent(int KEY, struct room_data *This, void *NOTHING)
{
  int                              i,
                                   flag = 0,
                                   a,
                                   b;
  struct extra_descr_data         *point;

  printf("Name        :%s, ", This->name);
  printf("Number      :%d\n", This->number);
  if (This->description)
    printf("Description\n-----------%s-----------\n", This->description);
  else
    printf("Description\n-----------\nnone\n-----------\n", This->description);

  for (point = This->ex_description; point; point = point->next) {
    printf("Extra Description.\n");
    printf("  Key Words:%s\n", point->keyword);
    printf("  Description:\n-----------%s-----------\n", point->description);
  }

  printf("Zone        :%d, ", This->zone);
  printf("Flags       :%d, ", This->room_flags);
  printf("Sector      :%s\n", Sector_names[This->sector_type]);
  printf("River Dir :%s, ", (This->river_dir >= 0 ? dirs[This->river_dir] : "None"));
  printf("River Spd :%d, ", This->river_speed);
  printf("Tele Time :%d, ", This->tele_time);
  printf("Tele Targ :%d, ", This->tele_targ);
  printf("Tele Look :%s\n", (This->tele_look != 0 ? "Yes" : "No"));
  printf("In room Sound:\n%s\n", This->sound);
  printf("Distand Sound:\n%s\n", This->distant_sound);
  for (i = 0; i < 6; i++) {
    if (This->dir_option[i]) {
      printf("  Exit : %s\n", dirs[i]);
      printf("  Keyword : %s\n", This->dir_option[i]->keyword);
      printf("  Exit Info : ");
      if (This->dir_option[i]->exit_info) {
	for (a = 1, b = 0; a < 64; a *= 2, b++) {
	  if (IS_SET(This->dir_option[i]->exit_info, a)) {
	    printf(" %s ", EXIT_FLAGS_NAMES[b]);
	  }
	}
	printf("\n");
      } else
	printf("None\n");

      printf("  Key : %d, ", This->dir_option[i]->key);
      printf("  To Room : %d\n", This->dir_option[i]->to_room);
      printf("  Exit Alias : %s\n", This->dir_option[i]->exit_alias);
      printf("\n");
      flag++;
    }
  }
  if (!flag)
    printf("  No EXITS defined for this room.\n");
  else
    printf("  Number of EXITS defined : %d.\n", flag);
  printf("\n");
}

#if 0
void 
boot_world(void)
{
  FILE                            *fl;
  struct room_data                *rp;
  char                            *wld_file_list[MAX_WLD_FILE_ENTRIES];
  int                              index = 0;
  char                             buf[256];

  init_hash_table(&room_db, sizeof(struct room_data), 2048);

  /*
   * This will read in the Master.wld file which contains all of the
   * areas that are to be loaded into the game.
   */

  sprintf(buf, "%s/Master.wld", WLD_FILE_DIRECTORY);
  if ((fl = fopen(buf, "r")) == NULL) {
    fprintf(stderr, "fopen: file not found!\n");
    exit(0);
  }
  yyin = fl;
  while ((MYtoken = yylex()) != TOK_zero) {
    fprintf(stderr, "TOKEN_STRING [%s]\n", yytext);
    fprintf(stderr, "TOKEN_NUMBER [%d]\n", MYtoken);
    if (MYtoken == TOK_ID) {
      wld_file_list[index] = (char *)strdup(yytext);
      index++;
    } else {
      fprintf(stderr, "ERROR in Master.wld File?\n");
      exit(0);
    }
  }
  wld_file_list[index] = '\0';
  index = 0;
  fclose(fl);
  while (wld_file_list[index]) {
    sprintf(buf, "%s/%s", WLD_FILE_DIRECTORY, wld_file_list[index]);
    if ((fl = fopen(buf, "r")) == NULL) {
      fprintf(stderr, "fopen: file not found!\n");
      exit(0);
    }
    yyin = fl;
    yyrestart(yyin);
    LINEcount = 0;
    if (!parse_wld(yyin)) {
      printf("Database Parse Aborted\n");
      exit(0);
    }
    fclose(fl);
    index++;
    GLINEcount += LINEcount;
  }

/*
 * printf("------------------ Dumping Hash Table -----------------------\n");
 * hash_iterate(&room_db,print_hash_ent,NULL);
 */

  printf("\n");
  printf("ROOMcount %d, GLINEcount %d\n", ROOMcount, GLINEcount);
  fclose(fl);
}
#endif

/*
 * returns the real number of the monster with given virtual number 
 */
int 
real_mobile(int virtual)
{
  int                              bot,
                                   top,
                                   mid;

  if (DEBUG)
    dlog("real_mobile");
  bot = 0;
  top = top_of_mobt;

  /*
   * perform binary search on mob-table 
   */
  for (;;) {
    mid = (bot + top) / 2;

    if ((mob_index + mid)->virtual == virtual)
      return (mid);
    if (bot >= top)
      return (-1);
    if ((mob_index + mid)->virtual > virtual)
      top = mid - 1;
    else
      bot = mid + 1;
  }
}

/*
 * returns the real number of the object with given virtual number 
 */
int 
real_object(int virtual)
{
  int                              bot,
                                   top,
                                   mid;

  if (DEBUG)
    dlog("real_object");

  bot = 0;
  top = top_of_objt;

  /*
   * perform binary search on obj-table 
   */
  for (;;) {
    mid = (bot + top) / 2;

    if ((obj_index + mid)->virtual == virtual)
      return (mid);
    if (bot >= top)
      return (-1);
    if ((obj_index + mid)->virtual > virtual)
      top = mid - 1;
    else
      bot = mid + 1;
  }
}

void 
PrintError(int ErrorCode)
{
  fprintf(stderr, "%%Error - line %d %s\n", LINEcount, error_list[ErrorCode]);
}

int 
FindThisToken(int WHICH_TOKEN)
{
  do {
    MYtoken = yylex();
    if (MYtoken == WHICH_TOKEN)
      return (MYtoken);
  } while (MYtoken);
  return (MYtoken);
}

int 
FindTokenInList(int WHICH_TOKEN, int list[])
{
  int                              index;

  for (index = 0; list[index] != -1; index++) {
    if (list[index] == WHICH_TOKEN)
      return 1;
  }
  return 0;
}

void 
ResetThisRoom(struct room_data *This)
{
  int                              i;

  This->name = DEFAULT_ROOM_NAME;
  This->zone = DEFAULT_ROOM_ZONE;
  This->sector_type = DEFAULT_ROOM_SECT;
  This->river_dir = DEFAULT_ROOM_RIVER_DIR;
  This->river_speed = DEFAULT_ROOM_RIVER_SPEED;
  This->tele_time = DEFAULT_ROOM_TELE_TIME;
  This->tele_targ = DEFAULT_ROOM_TELE_TARG;
  This->tele_look = DEFAULT_ROOM_TELE_LOOK;
  This->description = DEFAULT_ROOM_DESC;
  This->ex_description = DEFAULT_ROOM_EX_DESC;
  This->room_flags = DEFAULT_ROOM_FLAGS;
  This->sound = DEFAULT_ROOM_SOUND;
  This->distant_sound = DEFAULT_ROOM_DISTANT_SOUND;
  This->light = DEFAULT_ROOM_LIGHT;
  This->funct = DEFAULT_ROOM_FUNCT;
  This->contents = DEFAULT_ROOM_CONTENTS;
  This->people = DEFAULT_ROOM_PEOPLE;
  for (i = 0; i <= MAX_NUM_EXITS; i++) {
    This->dir_option[i] = 0;
  }
}

void 
ResetThisExit(struct room_direction_data *This)
{
  This->general_description = DEFAULT_EXIT_GENERAL_DESCRIPTION;
  This->keyword = DEFAULT_EXIT_KEYWORD;
  This->exit_info = DEFAULT_EXIT_EXIT_INFO;
  This->key = DEFAULT_EXIT_KEY;
  This->to_room = DEFAULT_EXIT_TO_ROOM;
  This->exit_alias = DEFAULT_EXIT_ALIAS;
  return;
}

struct room_data                *
FindThisRoom(int WhichRoom)
{
  return (real_roomp(WhichRoom));
}

int 
InheritThisRoom(struct room_data *WorkingRoom, int This)
{
  struct room_data                *this_ptr;

  if (WorkingRoom == NULL) {
    fprintf(stderr, "%%ERROR - pointer null - InheritThisRoom\n");
    exit(0);
  }
  if ((this_ptr = FindThisRoom(This)) == NULL) {
    PrintError(ERR_inherit_not_found);
    return 0;
  }
  WorkingRoom->name = this_ptr->name;
  WorkingRoom->zone = this_ptr->zone;
  WorkingRoom->sector_type = this_ptr->sector_type;
  if (FindTokenInList((this_ptr->sector_type + TOK_inside), LIST_water)) {
    WorkingRoom->river_dir = this_ptr->river_dir;
    WorkingRoom->river_speed = this_ptr->river_speed;
  }
  WorkingRoom->tele_time = this_ptr->tele_time;
  WorkingRoom->tele_targ = this_ptr->tele_targ;
  WorkingRoom->tele_look = this_ptr->tele_look;
  WorkingRoom->description = this_ptr->description;
  WorkingRoom->ex_description = this_ptr->ex_description;
  WorkingRoom->room_flags = this_ptr->room_flags;
  WorkingRoom->sound = this_ptr->sound;
  WorkingRoom->distant_sound = this_ptr->distant_sound;
  WorkingRoom->light = this_ptr->light;
  WorkingRoom->funct = this_ptr->funct;
  return 1;
}

int 
parse_wld(FILE * which_file)
{

  Forever
  {
    MYtoken = yylex();		/*
				 * Get A token 
				 */
    switch (MYtoken) {
    case TOK_zero:
      if (ROOMcompile) {
	PrintError(TOK_zero);
	return 0;
      }
      return 1;
      break;
    case TOK_pound:
      MYtoken = yylex();
      if (MYtoken != TOK_int) {
	PrintError(TOK_pound);
	return 0;
      }
      if (!make_room(atoi(yytext)))
	return 0;
      ROOMcount++;
      break;
    case TOK_cr:
      break;
    case TOK_end:
      PrintError(TOK_end);
      exit(0);
      break;
    default:
      PrintError(ERR_unknown);
      exit(0);
      break;
    }
  }
  return 1;
}

void 
make_extra_description(struct room_data *This)
{
  struct extra_descr_data         *new_descr;
  char                             c;

  CREATE(new_descr, struct extra_descr_data, 1);

  TMPbuff_ptr = 0;
  while (1) {
    c = input();
    if (TMPbuff_ptr > MAX_MY_STRING_LENGTH) {
      PrintError(ERR_strlen);
      exit(0);
    }
    if (c == '~' || c == '\n') {
      LINEcount++;
      break;
    } else
      TMPbuff[TMPbuff_ptr++] = c;
  }
  if (TMPbuff_ptr == 0) {
    PrintError(ERR_ex_name);
    exit(0);
  }
  TMPbuff[TMPbuff_ptr] = '\0';
  new_descr->keyword = (char *)strdup(TMPbuff);

  TMPbuff_ptr = 0;
  while (1) {
    c = input();
    if (TMPbuff_ptr > MAX_DESC_LENGTH) {
      PrintError(ERR_strlen);
      exit(0);
    }
    if (c == '\n')
      LINEcount++;
    if (c == '~') {
      LINEcount++;
      break;
    } else
      TMPbuff[TMPbuff_ptr++] = c;
  }
  TMPbuff[TMPbuff_ptr] = '\0';
  new_descr->description = (char *)strdup(TMPbuff);
  new_descr->next = This->ex_description;
  This->ex_description = new_descr;
}

void 
make_exit(struct room_data *WorkingRoom)
{
  int                              direction;
  char                             c;

  MYtoken = yylex();
  switch (MYtoken) {
  case TOK_north:
  case TOK_east:
  case TOK_south:
  case TOK_west:
  case TOK_up:
  case TOK_down:
    {
      direction = MYtoken - TOK_north;
      CREATE(WorkingRoom->dir_option[direction], struct room_direction_data, 1);

      ResetThisExit(WorkingRoom->dir_option[direction]);
      do {
	MYtoken = yylex();
	switch (MYtoken) {
	case TOK_inherit:
	  MYtoken = yylex();
	  if (MYtoken == TOK_int) {
	    /*
	     * nothing yet 
	     */
	  } else {
	    PrintError(TOK_inherit);
	    return;
	  }
	  break;
	case TOK_key:
	  MYtoken = yylex();
	  if (MYtoken != TOK_int) {
	    PrintError(TOK_key);
	    exit(0);
	  }
	  WorkingRoom->dir_option[direction]->key = atoi(yytext);
	  break;
	case TOK_flags:
	  MYtoken = yylex();
	  while (MYtoken != TOK_tilde) {
	    if (FindTokenInList(MYtoken, LIST_exit_flags)) {
	      WorkingRoom->dir_option[direction]->exit_info |=
		EXIT_FLAGS[MYtoken - TOK_isdoor];
	    } else {
	      PrintError(ERR_list);
	      exit(0);
	    }
	    MYtoken = yylex();
	  }
	  break;
	case TOK_goto:
	  MYtoken = yylex();
	  if (MYtoken != TOK_int) {
	    PrintError(TOK_goto);
	    exit(0);
	  }
	  WorkingRoom->dir_option[direction]->to_room = atoi(yytext);
	  break;
	case TOK_desc:
	  TMPbuff_ptr = 0;
	  while (1) {
	    c = input();
	    if (TMPbuff_ptr > MAX_MY_STRING_LENGTH) {
	      PrintError(ERR_strlen);
	      exit(0);
	    }
	    if (c == '~') {
	      LINEcount++;
	      TMPbuff[TMPbuff_ptr++] = '\n';
	      break;
	    } else
	      TMPbuff[TMPbuff_ptr++] = c;
	  }
	  if (TMPbuff_ptr == 0) {
	    PrintError(TOK_desc);
	    break;
	  }
	  TMPbuff[TMPbuff_ptr] = '\0';
	  WorkingRoom->dir_option[direction]->general_description
	    = (char *)strdup(TMPbuff);
	  break;
	case TOK_end:
	  return;
	  break;
	default:
	  PrintError(ERR_unknown);
	  return;
	  break;
	}
      } while (MYtoken && (MYtoken != TOK_end));
      return;
    }
    break;
  default:
    PrintError(MYtoken);
    FindThisToken(TOK_end);
    return;
    break;
  }
}

int 
make_room(int RoomNumber)
{
  struct room_data                *WorkingRoom;
  char                             c;

  ROOMcompile = 1;
  allocate_room(RoomNumber);
  WorkingRoom = real_roomp(RoomNumber);
  ResetThisRoom(WorkingRoom);
  WorkingRoom->number = RoomNumber;

  Forever
  {
    MYtoken = yylex();
    switch (MYtoken) {
    case TOK_zero:
      PrintError(TOK_zero);
      ROOMcompile = 0;
      return 0;
      break;
    case TOK_flags:
      while (yylex() != TOK_tilde);
      break;
    case TOK_name:
    case TOK_sound1:
    case TOK_sound2:
      TMPbuff_ptr = 0;
      while (1) {
	c = input();
	if (TMPbuff_ptr > MAX_MY_STRING_LENGTH) {
	  PrintError(ERR_strlen);
	  return 0;
	}
	if (c == '\n')
	  LINEcount++;
	if (c == '~' || c == '\n')
	  break;
	else
	  TMPbuff[TMPbuff_ptr++] = c;
      }
      if (TMPbuff_ptr == 0) {
	PrintError(MYtoken);
	return 0;
      }
      TMPbuff[TMPbuff_ptr] = '\0';
      switch (MYtoken) {
      case TOK_name:
	WorkingRoom->name = (char *)strdup(TMPbuff);
	break;
      case TOK_sound1:
	WorkingRoom->sound = (char *)strdup(TMPbuff);
	break;
      case TOK_sound2:
	WorkingRoom->distant_sound = (char *)strdup(TMPbuff);
	break;
      }
      break;
    case TOK_desc:
      TMPbuff_ptr = 0;
      while (1) {
	c = input();
	if (TMPbuff_ptr > MAX_DESC_LENGTH) {
	  PrintError(ERR_strlen);
	  return 0;
	}
	if (c == '~')
	  break;
	else
	  TMPbuff[TMPbuff_ptr++] = c;
      }
      TMPbuff[TMPbuff_ptr] = '\0';
      WorkingRoom->description = (char *)strdup(TMPbuff);
      break;
    case TOK_ex_desc:
      make_extra_description(WorkingRoom);
      break;
    case TOK_inherit:
      MYtoken = yylex();
      if (MYtoken == TOK_int) {
	if (!InheritThisRoom(WorkingRoom, atoi(yytext)))
	  return 0;
      } else {
	PrintError(TOK_inherit);
	return 0;
      }
      break;
    case TOK_pound:
      PrintError(TOK_pound);
      unput('#');
      ROOMcompile = 0;
      return 0;
      break;
    case TOK_exit:
      make_exit(WorkingRoom);
      break;
    case TOK_zone:
      MYtoken = yylex();
      if (MYtoken != TOK_int) {
	PrintError(TOK_zone);
	PrintError(TOK_int);
	return 0;
      }
      WorkingRoom->zone = atoi(yytext);
      break;
    case TOK_sect:
      MYtoken = yylex();
      if (!FindTokenInList(MYtoken, LIST_sector)) {
	PrintError(TOK_sect);
	PrintError(ERR_list);
	return 0;
      }
      WorkingRoom->sector_type = MYtoken - TOK_inside;
      if (FindTokenInList(MYtoken, LIST_water)) {	/*
							 * is sector type water? 
							 */
	MYtoken = yylex();	/*
				 * this should be direction? 
				 */
	if (FindTokenInList(MYtoken, LIST_direction)) {
	  WorkingRoom->river_dir = MYtoken - TOK_north;
	  MYtoken = yylex();	/*
				 * this should be speed 
				 */
	  if (MYtoken == TOK_int) {
	    WorkingRoom->river_speed = atoi(yytext);
	  } else {
	    PrintError(TOK_int);
	    exit(0);
	  }
	} else {
	  PrintError(ERR_list);
	  exit(0);
	}
      }
      break;
    case TOK_tele_time:
      MYtoken = yylex();
      if (MYtoken != TOK_int) {
	PrintError(TOK_tele_time);
	PrintError(TOK_int);
	exit(0);
      }
      WorkingRoom->tele_time = atoi(yytext);
      break;
    case TOK_tele_targ:
      MYtoken = yylex();
      if (MYtoken != TOK_int) {
	PrintError(TOK_tele_targ);
	PrintError(TOK_int);
	exit(0);
      }
      WorkingRoom->tele_targ = atoi(yytext);
      break;
    case TOK_tele_look:
      MYtoken = yylex();
      if (!FindTokenInList(MYtoken, LIST_reply)) {
	PrintError(TOK_tele_look);
	PrintError(ERR_list);
	exit(0);
      }
      WorkingRoom->tele_look = MYtoken - TOK_no;
      break;
    case TOK_end:
      if (NESTlevel > 0) {
	PrintError(TOK_end);
	ROOMcompile = 0;
	exit(0);
      }
      ROOMcompile = 0;
      return 1;
      break;
    default:
      PrintError(ERR_unknown);
      return (0);
      break;
    }
  }
  ROOMcompile = 0;
  return 0;
}

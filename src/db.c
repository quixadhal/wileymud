/*
 * file: db.c , Database module.                          Part of DIKUMUD
 * Usage: Loading/Saving chars, booting world, resetting etc.
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */

#include <stdio.h>
#include <stdlib.h>
/* #include <unistd.h> */
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "global.h"
#ifdef IMC
#include "imc.h"
#endif
#include "bug.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "mudlimits.h"
#include "opinion.h"
#include "hash.h"
#include "constants.h"
#include "spells.h"
#include "spell_parser.h"
#include "reception.h"
#include "weather.h"
#include "modify.h"
#include "fight.h"
#include "act_social.h"
#include "spec_procs.h"
#include "multiclass.h"
#include "board.h"
#include "interpreter.h"
#include "ban.h"

#define _DB_C
#include "db.h"

/*
 * declarations of most of the 'global' variables
 */

int                                     top_of_world = -1;     /* ref to the top element of world */
struct hash_header                      room_db;
struct reset_q_type                     reset_q;

struct obj_data                        *object_list = 0;       /* the global linked list of obj's */
struct char_data                       *character_list = 0;    /* global l-list of chars */

struct zone_data                       *zone_table = NULL;       /* table of reset data */
int                                     top_of_zone_table = 0;
struct message_list                     fight_messages[MAX_MESSAGES];	/* fighting messages */
struct player_index_element            *player_table = 0;      /* index to player file */
int                                     top_of_p_table = 0;    /* ref to top of table */
int                                     top_of_p_file = 0;

char                                    credits[MAX_STRING_LENGTH] = "\0\0\0";	/* the Credits List */
char                                    news[MAX_STRING_LENGTH] = "\0\0\0";	/* the news */
char                                    motd[MAX_STRING_LENGTH] = "\0\0\0";	/* the messages of today */
char                                    help[MAX_STRING_LENGTH] = "\0\0\0";	/* the main help page */
char                                    wizhelp[MAX_STRING_LENGTH] = "\0\0\0";	/* the main wizhelp page */
char                                    info[MAX_STRING_LENGTH] = "\0\0\0";	/* the info text */
char                                    wizlist[MAX_STRING_LENGTH] = "\0\0\0";	/* the wizlist */
char                                    wmotd[MAX_STRING_LENGTH] = "\0\0\0";	/* the wizard motd */
char                                    greetings[MAX_STRING_LENGTH] = "\0\0\0";	/* greetings upon connection */
char                                    login_menu[MAX_STRING_LENGTH] = "\0\0\0";	/* login menu of choices */
char                                    sex_menu[MAX_STRING_LENGTH] = "\0\0\0";	/* login menu of sex perversions */
char                                    race_menu[MAX_STRING_LENGTH] = "\0\0\0";	/* login menu of races */
char                                    class_menu[MAX_STRING_LENGTH] = "\0\0\0";	/* login menu of classes */
char                                    race_help[MAX_STRING_LENGTH] = "\0\0\0";	/* descriptions of races */
char                                    class_help[MAX_STRING_LENGTH] = "\0\0\0";	/* descriptions of classes */
char                                    the_story[MAX_STRING_LENGTH] = "\0\0\0";	/* how Wiley was saved */
char                                    suicide_warn[MAX_STRING_LENGTH] = "\0\0\0";	/* are you sure? */
char                                    suicide_done[MAX_STRING_LENGTH] = "\0\0\0";	/* goodbye */

FILE                                   *mob_f = NULL;	       /* file containing mob prototypes */
FILE                                   *obj_f = NULL;	       /* obj prototypes */
FILE                                   *help_fl = NULL;	       /* file for help texts (HELP <kwd>) */
FILE                                   *wizhelp_fl = NULL;	       /* file for help texts (HELP <kwd>) */

struct index_data                      *mob_index = NULL;       /* index table for mobile file */
struct index_data                      *obj_index = NULL;       /* index table for object file */
struct help_index_element              *help_index = NULL;
struct help_index_element              *wizhelp_index = NULL;

int                                     top_of_mobt = 0;       /* top of mobile index table */
int                                     top_of_objt = 0;       /* top of object index table */
int                                     top_of_helpt = 0;      /* top of help index table */
int                                     top_of_wizhelpt = 0;      /* top of wizhelp index table */

struct time_info_data                   time_info;	       /* the infomation about the time */
struct weather_data                     weather_info;	       /* the infomation about the weather */

char                                    TMPbuff[1620] = "\0\0\0";
int                                     TMPbuff_ptr = 0;
int                                     ROOMcount = 0;
int                                     GLINEcount = 0;
int                                     LASTroomnumber = 0;

char                                  **list_of_players = NULL;
int                                     number_of_players = 0;
int                                     actual_players = 0;

/*************************************************************************
*  routines for booting the system                                       *
*********************************************************************** */

void load_db(void)
{
  FILE                                   *pfd = NULL;
  char                                    tmpbufx[256] = "\0\0\0";
  int                                     i = 0;

  if (DEBUG > 1)
    log_info("called %s with no arguments", __PRETTY_FUNCTION__);

  log_boot("Boot db -- BEGIN.");

  log_boot("- Resetting game time and weather:");
  reset_time();

  log_boot("- Reading news");
  file_to_string(NEWS_FILE, news);
  log_boot("- Reading credits");
  file_to_string(CREDITS_FILE, credits);
  log_boot("- Reading motd");
  file_to_string(MOTD_FILE, motd);
  log_boot("- Reading help");
  file_to_string(HELP_PAGE_FILE, help);
  log_boot("- Reading info");
  file_to_string(INFO_FILE, info);
  log_boot("- Reading wizlist");
  file_to_string(WIZLIST_FILE, wizlist);
  log_boot("- Reading wiz motd");
  file_to_string(WMOTD_FILE, wmotd);
  log_boot("- Reading greetings");
  file_to_string(GREETINGS_FILE, greetings);
  log_boot("- Reading login menu");
  file_to_prompt(LOGIN_MENU_FILE, login_menu);
  log_boot("- Reading sex menu");
  file_to_prompt(SEX_MENU_FILE, sex_menu);
  log_boot("- Reading race menu");
  file_to_prompt(RACE_MENU_FILE, race_menu);
  log_boot("- Reading class menu");
  file_to_prompt(CLASS_MENU_FILE, class_menu);
  log_boot("- Reading race help");
  file_to_prompt(RACE_HELP_FILE, race_help);
  log_boot("- Reading class help");
  file_to_prompt(CLASS_HELP_FILE, class_help);
  log_boot("- Reading story");
  file_to_string(STORY_FILE, the_story);
  log_boot("- Reading suicide warning");
  file_to_prompt(SUICIDE_WARN_FILE, suicide_warn);
  log_boot("- Reading suicide result");
  file_to_string(SUICIDE_DONE_FILE, suicide_done);

  load_bans();

  log_boot("- Loading rent mode");
  if (!(pfd = fopen(RENTCOST_FILE, "r"))) {
    log_boot("Default rent cost of 1.0 used.");
    if (!(pfd = fopen(RENTCOST_FILE, "w"))) {
      log_error("Cannot save rent cost!");
    } else {
      fprintf(pfd, "%f\n", 1.0);
      FCLOSE(pfd);
    }
  } else {
    double                                  it;

    if (fscanf(pfd, " %lf ", &it) != 1) {
      log_error("Invalid rent cost.");
      if (!(pfd = fopen(RENTCOST_FILE, "w"))) {
	log_error("Cannot save rent cost!");
      } else {
	fprintf(pfd, "%f\n", 1.0);
	FCLOSE(pfd);
      }
    }
    RENT_RATE = it;
    FCLOSE(pfd);
  }
  log_boot("- Loading player list");
  if (!(pfd = fopen(PLAYER_FILE, "r"))) {
    log_error("Cannot load accumulated player data\r\n");
  } else {
    if (list_of_players) {
      for (i = 0; i < number_of_players; i++)
	if (list_of_players[i])
	  DESTROY(list_of_players[i]);
      DESTROY(list_of_players);
    }
    fscanf(pfd, " %d ", &number_of_players);
    actual_players = number_of_players;
    CREATE(list_of_players, char *, number_of_players);

    for (i = 0; i < number_of_players; i++) {
      fgets(tmpbufx, 255, pfd);
      if (!(list_of_players[i] = (char *)strdup(tmpbufx))) {
	log_fatal("Failed to get memory for player list element %d.\r\n", i);
	proper_exit(MUD_HALT);
      }
    }
  }
  log_boot("- Loading reboot times");
  if (!(pfd = fopen(REBOOTTIME_FILE, "r"))) {
    log_boot("Default reboot times of 07:00 and 19:00 used.");
    REBOOT_AT1 = 7;
    REBOOT_AT2 = 19;
    if (!(pfd = fopen(REBOOTTIME_FILE, "w"))) {
      log_error("Cannot save reboot times!");
    } else {
      fprintf(pfd, "%d %d\n", REBOOT_AT1, REBOOT_AT2);
      FCLOSE(pfd);
    }
  } else {
    int                                     this,
                                            that;

    if (fscanf(pfd, " %d %d ", &this, &that) != 2) {
      log_error("Invalid reboot time.");
      REBOOT_AT1 = 7;
      REBOOT_AT2 = 19;
      if (!(pfd = fopen(REBOOTTIME_FILE, "w"))) {
	log_error("Cannot save reboot times!");
      } else {
	fprintf(pfd, "%d %d\n", REBOOT_AT1, REBOOT_AT2);
	FCLOSE(pfd);
      }
    }
    REBOOT_AT1 = this;
    REBOOT_AT2 = that;
    FCLOSE(pfd);
  }

  log_boot("- Loading help files");
  if (!(help_fl = fopen(HELP_KWRD_FILE, "r")))
    log_error("   Could not open help file.");
  else
    help_index = build_help_index(help_fl, &top_of_helpt);
  if (!(wizhelp_fl = fopen(WIZHELP_KWRD_FILE, "r")))
    log_error("   Could not open wizhelp file.");
  else
    wizhelp_index = build_help_index(wizhelp_fl, &top_of_wizhelpt);

  log_boot("- Loading fight messages");
  load_messages();
  log_boot("- Loading social messages");
  boot_social_messages();
  log_boot("- Loading pose messages");
  boot_pose_messages();

  log_boot("- Booting mobiles");
  if (!(mob_f = fopen(MOB_FILE, "r"))) {
    log_fatal("boot mobiles");
    proper_exit(MUD_HALT);
  }
  log_boot("- Booting objects");
  if (!(obj_f = fopen(OBJ_FILE, "r"))) {
    log_fatal("boot objects");
    proper_exit(MUD_HALT);
  }
  log_boot("- Booting zones");
  boot_zones();
  log_boot("- Booting rooms");
  boot_world();

  log_boot("- Generating mobile index");
  mob_index = generate_indices(mob_f, &top_of_mobt);
  log_boot("- Generating object index");
  obj_index = generate_indices(obj_f, &top_of_objt);
  log_boot("- Renumbering zones");
  renum_zone_table();
  if (!no_specials) {
    log_boot("- Assigining mobile functions");
    assign_mobiles();
    log_boot("- Assigining object functions");
    assign_objects();
    log_boot("- Assigining room functions");
    assign_rooms();
  }
  log_boot("- Assigning command functions");
  assign_command_pointers();
  log_boot("- Assigning spell functions");
  assign_spell_pointers();

  for (i = 0; i <= top_of_zone_table; i++)
    reset_zone(i);
  reset_q.head = reset_q.tail = 0;
  log_boot("Boot db -- DONE.");
}

void unload_db(void)
{
  if (DEBUG > 1)
    log_info("called %s with no arguments", __PRETTY_FUNCTION__);

  unload_bans();
}

/* generate index table for object or monster file */
struct index_data                      *generate_indices(FILE * fl, int *top)
{
  int                                     i = 0;
  struct index_data                      *indexp = NULL;
  char                                    buf[82] = "\0\0\0";
  static char                             omega[6] = "omega";

  if (DEBUG > 2)
    log_info("called %s with %08zx, %08zx", __PRETTY_FUNCTION__, (size_t)fl, (size_t)top);

  rewind(fl);

  for (;;) {
    if (fgets(buf, sizeof(buf), fl)) {
      if (*buf == '#') {
	if (!i) {					       /* first cell */
	  CREATE(indexp, struct index_data, 1);
	} else {
	  RECREATE(indexp, struct index_data, i + 1);
	}

	sscanf(buf, "#%d", &indexp[i].virtual);
	indexp[i].pos = ftell(fl);
	indexp[i].number = 0;
	indexp[i].func = 0;
	indexp[i].name = (indexp[i].virtual < 99999) ? fread_string(fl) : omega;
	i++;
      } else {
	if (*buf == '$')				       /* EOF */
	  break;
      }
    } else {
      log_fatal("generate indices");
      proper_exit(MUD_HALT);
    }
  }
  *top = i - 2;
  return (indexp);
}

void cleanout_room(struct room_data *rp)
{
  int                                     i = 0;
  struct extra_descr_data                *exptr = NULL;
  struct extra_descr_data                *nptr = NULL;

  if (DEBUG > 2)
    log_info("called %s with %08zx", __PRETTY_FUNCTION__, (size_t)rp);

  DESTROY(rp->name);
  DESTROY(rp->description);
  for (i = 0; i < MAX_NUM_EXITS; i++)
    if (rp->dir_option[i]) {
      DESTROY(rp->dir_option[i]->general_description);
      DESTROY(rp->dir_option[i]->keyword);
      DESTROY(rp->dir_option[i]);
      rp->dir_option[i] = NULL;
    }
  for (exptr = rp->ex_description; exptr; exptr = nptr) {
    nptr = exptr->next;
    DESTROY(exptr->keyword);
    DESTROY(exptr->description);
    DESTROY(exptr);
  }
}

void completely_cleanout_room(struct room_data *rp)
{
  struct char_data                       *ch = NULL;
  struct obj_data                        *obj = NULL;

  if (DEBUG > 2)
    log_info("called %s with %08zx", __PRETTY_FUNCTION__, (size_t)rp);

  while (rp->people) {
    ch = rp->people;
    act("The hand of god sweeps across the land and you are swept into the Void.", FALSE, NULL,
	NULL, NULL, TO_VICT);
    char_from_room(ch);
    char_to_room(ch, 0);				       /* send character to the void */
  }

  while (rp->contents) {
    obj = rp->contents;
    obj_from_room(obj);
    obj_to_room(obj, 0);				       /* send item to the void */
  }

  cleanout_room(rp);
}

void load_one_room(FILE *fl, struct room_data *rp)
{
  char                                    chk[50] = "\0\0\0";
  int                                     tmp = 0;
  struct extra_descr_data                *new_descr = NULL;

  if (DEBUG > 2)
    log_info("called %s with %08zx, %08zx", __PRETTY_FUNCTION__, (size_t)fl, (size_t)rp);

  rp->name = fread_string(fl);
  rp->description = fread_string(fl);

  if (top_of_zone_table >= 0) {
    int                                     zone = 0;

    fscanf(fl, " %*d ");
    /*
     * OBS: Assumes ordering of input rooms 
     */
    for (zone = 0; rp->number > zone_table[zone].top && zone <= top_of_zone_table; zone++);
    if (zone > top_of_zone_table) {
      log_fatal("Room %d is outside of any zone.\n", rp->number);
      proper_exit(MUD_HALT);
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

  if (tmp == 7) {					       /* river */
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
  rp->light = 0;					       /* Zero light sources */

  for (tmp = 0; tmp < MAX_NUM_EXITS; tmp++)
    rp->dir_option[tmp] = 0;

  rp->ex_description = 0;

  while (1 == fscanf(fl, " %s \n", chk)) {
    switch (*chk) {
      case 'D':
	setup_dir(fl, rp->number, atoi(chk + 1));
	break;
      case 'E':					       /* extra description field */
	CREATE(new_descr, struct extra_descr_data, 1);

	new_descr->keyword = fread_string(fl);
	new_descr->description = fread_string(fl);
	new_descr->next = rp->ex_description;
	rp->ex_description = new_descr;
	break;
      case 'S':					       /* end of current room */
	return;
      default:
	log_error("unknown auxiliary code `%s' in room load of #%d", chk, rp->number);
	break;
    }
  }
}

/* load the rooms */
void boot_world(void)
{
  FILE                                   *fl = NULL;
  int                                     virtual_nr = 0;
  struct room_data                       *rp = NULL;

  if (DEBUG > 1)
    log_info("called %s with no arguments", __PRETTY_FUNCTION__);

  init_hash_table(&room_db, sizeof(struct room_data), 2048);
  character_list = 0;
  object_list = 0;

  if (!(fl = fopen(WORLD_FILE, "r"))) {
    log_fatal("boot_world: could not open world file.");
    proper_exit(MUD_HALT);
  }
  while (1 == fscanf(fl, " #%d\n", &virtual_nr)) {
    if (DEBUG && !(virtual_nr % 10))
      log_boot("Loading Room [#%d]\r", virtual_nr);
    allocate_room(virtual_nr);
    rp = real_roomp(virtual_nr);
    /*
     * bzero(rp, sizeof(*rp)); 
     */
    rp->number = virtual_nr;
    load_one_room(fl, rp);
  }
  FCLOSE(fl);
  log_boot("- All Rooms loaded!");
}

void allocate_room(int room_number)
{
  if (DEBUG > 2)
    log_info("called %s with %d", __PRETTY_FUNCTION__, room_number);

  if (room_number > top_of_world)
    top_of_world = room_number;
  else {
    log_fatal("ERROR - room number %d is out of order\n", room_number);
    proper_exit(MUD_HALT);
  }
  hash_find_or_create(&room_db, room_number);
}

/* read direction data */
void setup_dir(FILE *fl, int room, int dir)
{
  int                                     tmp = 0;
  int                                     flag = 0;
  struct room_data                       *rp = NULL;

  if (DEBUG > 2)
    log_info("called %s with %08zx, %d, %d", __PRETTY_FUNCTION__, (size_t)fl, room, dir);

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

void renum_zone_table(void)
{
  int                                     zone = 0;
  int                                     comm = 0;
  struct reset_com                       *cmd = NULL;

  if (DEBUG > 2)
    log_info("called %s with no arguments", __PRETTY_FUNCTION__);

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

/* load the zone table and command tables */
void boot_zones(void)
{
  FILE                                   *fl = NULL;
  int                                     zon = 0;
  int                                     cmd_no = 0;
  int                                     expand = 0;
  int                                     tmp = 0;
  char                                   *check = NULL;
  char                                    buf[81] = "\0\0\0";

  if (DEBUG > 1)
    log_info("called %s with no arguments", __PRETTY_FUNCTION__);

  if (!(fl = fopen(ZONE_FILE, "r"))) {
    log_fatal("boot_zones");
    proper_exit(MUD_HALT);
  }
  for (;;) {
    fscanf(fl, " #%*d\n");
    check = fread_string(fl);

    if (*check == '$')
      break;						       /* end of file */

    /*
     * alloc a new zone 
     */

    if (!zon) {
      CREATE(zone_table, struct zone_data, 1);
    } else {
      RECREATE(zone_table, struct zone_data, zon + 1);
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
      if (expand) {
	if (!cmd_no) {
	  CREATE(zone_table[zon].cmd, struct reset_com, 1);
	} else {
	  RECREATE(zone_table[zon].cmd, struct reset_com, cmd_no + 1);
	}
      }
      expand = 1;

      fscanf(fl, " ");					       /* skip blanks */
      fscanf(fl, "%c", &zone_table[zon].cmd[cmd_no].command);
      if (zone_table[zon].cmd[cmd_no].command == 'S')
	break;

      if (zone_table[zon].cmd[cmd_no].command == '*') {
	expand = 0;
	fgets(buf, 80, fl);				       /* skip command */
	continue;
      }
      fscanf(fl, " %d %d %d",
	     &tmp, &zone_table[zon].cmd[cmd_no].arg1, &zone_table[zon].cmd[cmd_no].arg2);

      zone_table[zon].cmd[cmd_no].if_flag = tmp;

      if (zone_table[zon].cmd[cmd_no].command == 'M' ||
	  zone_table[zon].cmd[cmd_no].command == 'O' ||
	  zone_table[zon].cmd[cmd_no].command == 'E' ||
	  zone_table[zon].cmd[cmd_no].command == 'P' ||
	  zone_table[zon].cmd[cmd_no].command == 'D')
	fscanf(fl, " %d", &zone_table[zon].cmd[cmd_no].arg3);
      fgets(buf, 80, fl);				       /* read comment */
      cmd_no++;
    }
    zon++;
  }
  top_of_zone_table = --zon;
  DESTROY(check);
  FCLOSE(fl);
}

/*
 * procedures for resetting, both play-time and boot-time
 */

void fread_dice(FILE * fp, long int *x, long int *y, long int *z)
{
  char                                    sign[2] = "\0";

  if (DEBUG > 2)
    log_info("called %s with %08zx, %08zx, %08zx, %08zx", __PRETTY_FUNCTION__, (size_t)fp, (size_t)x, (size_t)y, (size_t)z);

  if (!fp || !x || !y || !z || feof(fp))
    return;
  *x = *y = *z = sign[1] = 0;
  *sign = '+';
  fscanf(fp, " %ldd%ld%[+-]%ld", x, y, sign, z);
  if (!*y)
    *y = 1;
  if (*sign == '-')
    *z = -*z;
}

/* read a mobile from MOB_FILE */
struct char_data                       *read_mobile(int nr, int type)
{
  int                                     i = 0;
  long                                    tmp = 0L;
  long                                    tmp2 = 0L;
  long                                    tmp3 = 0L;
  long                                    tmp4 = 0L;
  long                                    tmp5 = 0L;
  struct char_data                       *mob = NULL;
  char                                    buf[100] = "\0\0\0";
  char                                    letter = '\0';

  if (DEBUG > 2)
    log_info("called %s with %d, %d", __PRETTY_FUNCTION__, nr, type);

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
   *** Numeric data *** */

  mob->mult_att = 0;

  fscanf(mob_f, "%ld ", &tmp);
  mob->specials.act = tmp;
  SET_BIT(mob->specials.act, ACT_ISNPC);

  fscanf(mob_f, " %ld ", &tmp);
  mob->specials.affected_by = tmp;

  fscanf(mob_f, " %ld ", &tmp);
  mob->specials.alignment = tmp;

  mob->player.class = CLASS_WARRIOR;

  fscanf(mob_f, " %c ", &letter);

  switch (letter) {
    case 'W':
    case 'M':
    case 'S':{
	if ((letter == 'W') || (letter == 'M')) {
	  fscanf(mob_f, " %ld ", &tmp);
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

	fscanf(mob_f, " %ld ", &tmp);
	GET_LEVEL(mob, WARRIOR_LEVEL_IND) = tmp;

	fscanf(mob_f, " %ld ", &tmp);
	mob->points.hitroll = 20 - tmp;

	fscanf(mob_f, " %ld ", &tmp);
	mob->points.armor = 10 * tmp;

	fscanf(mob_f, " %ldd%ld+%ld ", &tmp, &tmp2, &tmp3);
	mob->points.max_hit = dice(tmp, tmp2) + tmp3;
	mob->points.hit = mob->points.max_hit;

	fscanf(mob_f, " %ldd%ld+%ld \n", &tmp, &tmp2, &tmp3);
	mob->points.damroll = tmp3;
	mob->specials.damnodice = tmp;
	mob->specials.damsizedice = tmp2;

	mob->points.mana = 100;
	mob->points.max_mana = 100;

	mob->points.move = 100;
	mob->points.max_move = 100;

	fscanf(mob_f, " %ld ", &tmp);
	if (tmp == -1) {
	  fscanf(mob_f, " %ld ", &tmp);
	  mob->points.gold = tmp + fuzz(tmp / 10);
	  fscanf(mob_f, " %ld ", &tmp);
	  GET_EXP(mob) = tmp + fuzz(tmp / 10);
	  fscanf(mob_f, " %ld \n", &tmp);
	  GET_RACE(mob) = tmp;
	} else {
	  mob->points.gold = tmp + fuzz(tmp / 10);
	  fscanf(mob_f, " %ld \n", &tmp);
	  GET_EXP(mob) = tmp + fuzz(tmp / 10);
	}

	fscanf(mob_f, " %ld ", &tmp);
	mob->specials.position = tmp;

	fscanf(mob_f, " %ld ", &tmp);
	mob->specials.default_pos = tmp;

	fscanf(mob_f, " %ld ", &tmp);
	if (tmp < 3) {
	  mob->player.sex = tmp;
	  mob->immune = 0;
	  mob->M_immune = 0;
	  mob->susc = 0;
	} else if (tmp < 6) {
	  mob->player.sex = (tmp - 3);
	  fscanf(mob_f, " %ld ", &tmp);
	  mob->immune = tmp;
	  fscanf(mob_f, " %ld ", &tmp);
	  mob->M_immune = tmp;
	  fscanf(mob_f, " %ld ", &tmp);
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
	  mob->specials.apply_saving_throw[i] = MAX(20 - GET_LEVEL(mob, WARRIOR_LEVEL_IND), 2);
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
      }
      break;
    case 'D':{
	/*
	 * The old monsters are down below here 
	 */

	fscanf(mob_f, "\n");

	fscanf(mob_f, " %ld ", &tmp);
	mob->abilities.str = tmp;

	fscanf(mob_f, " %ld ", &tmp);
	mob->abilities.intel = tmp;

	fscanf(mob_f, " %ld ", &tmp);
	mob->abilities.wis = tmp;

	fscanf(mob_f, " %ld ", &tmp);
	mob->abilities.dex = tmp;

	fscanf(mob_f, " %ld \n", &tmp);
	mob->abilities.con = tmp;

	fscanf(mob_f, " %ld ", &tmp);
	fscanf(mob_f, " %ld ", &tmp2);

	mob->points.max_hit = number(tmp, tmp2);
	mob->points.hit = mob->points.max_hit;

	fscanf(mob_f, " %ld ", &tmp);
	mob->points.armor = 10 * tmp;

	fscanf(mob_f, " %ld ", &tmp);
	mob->points.mana = tmp;
	mob->points.max_mana = tmp;

	fscanf(mob_f, " %ld ", &tmp);
	mob->points.move = tmp;
	mob->points.max_move = tmp;

	fscanf(mob_f, " %ld ", &tmp);
	mob->points.gold = tmp;

	fscanf(mob_f, " %ld \n", &tmp);
	GET_EXP(mob) = tmp;

	fscanf(mob_f, " %ld ", &tmp);
	mob->specials.position = tmp;

	fscanf(mob_f, " %ld ", &tmp);
	mob->specials.default_pos = tmp;

	fscanf(mob_f, " %ld ", &tmp);
	mob->player.sex = tmp;

	fscanf(mob_f, " %ld ", &tmp);
	mob->player.class = tmp;

	fscanf(mob_f, " %ld ", &tmp);
	GET_LEVEL(mob, WARRIOR_LEVEL_IND) = tmp;

	fscanf(mob_f, " %ld ", &tmp);
	mob->player.time.birth = time(0);
	mob->player.time.played = 0;
	mob->player.time.logon = time(0);

	fscanf(mob_f, " %ld ", &tmp);
	mob->player.weight = tmp;

	fscanf(mob_f, " %ld \n", &tmp);
	mob->player.height = tmp;

	for (i = 0; i < 3; i++) {
	  fscanf(mob_f, " %ld ", &tmp);
	  GET_COND(mob, i) = tmp;
	}
	fscanf(mob_f, " \n ");

	for (i = 0; i < 5; i++) {
	  fscanf(mob_f, " %ld ", &tmp);
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
      break;
    case 'C':{
	int                                     x = 0;
	int                                     lvl = 0;

	fscanf(mob_f, " %ld %ld %ld %ld %ld", &tmp, &tmp2, &tmp3, &tmp4, &tmp5);
	GET_RACE(mob) = tmp;
	mob->player.class = tmp2;
	mob->player.sex = tmp3;
	mob->player.height = tmp4;
	mob->player.weight = tmp5;
	fread_dice(mob_f, &tmp, &tmp2, &tmp3);
	mob->points.gold = dice(tmp, tmp2) + tmp3;
	fread_dice(mob_f, &tmp, &tmp2, &tmp3);
	GET_EXP(mob) = dice(tmp, tmp2) + tmp3;
	fscanf(mob_f, " %ld ", &tmp);
	if (!mob->player.class)				       /* no class... store level in warrior slot */
	  GET_LEVEL(mob, WARRIOR_LEVEL_IND) = tmp;
	else
	  for (x = 0; x < ABS_MAX_CLASS; x++)
	    if (HasClass(mob, 1 << x))
	      GET_LEVEL(mob, x) = tmp;
	lvl = tmp;
	fread_dice(mob_f, &tmp, &tmp2, &tmp3);
	mob->points.hit = mob->points.max_hit = dice(tmp, tmp2) + tmp3;
	mob->points.mana = mob->points.max_mana = 100;
	mob->points.move = mob->points.max_move = 100;
	fscanf(mob_f, " %ld %ld %ld \n", &tmp, &tmp2, &tmp3);
	mob->points.armor = 10 * tmp;
	mob->points.hitroll = 20 - tmp2;
	mob->mult_att = tmp3;
	if (mob->mult_att < 0)
	  mob->mult_att = 1;
	for (x = 0; x < mob->mult_att; x++) {
	  fread_dice(mob_f, &tmp, &tmp2, &tmp3);
	  fscanf(mob_f, " %ld \n", &tmp4);
	  mob->points.damroll = tmp3;
	  mob->specials.damnodice = tmp;
	  mob->specials.damsizedice = tmp2;
	  /*
	   * damage type is ignored for now... we also note that only the last line is used... and that we assume
	   * mult_att= 1 == mult_att= 0 
	   */
	}
	fscanf(mob_f, " %ld %ld %ld \n", &tmp, &tmp2, &tmp3);
	mob->M_immune = tmp;
	mob->immune = tmp2;
	mob->susc = tmp3;
	fread_dice(mob_f, &tmp, &tmp2, &tmp3);
	mob->abilities.str = dice(tmp, tmp2) + tmp3;
	fread_dice(mob_f, &tmp, &tmp2, &tmp3);
	mob->abilities.str_add = dice(tmp, tmp2) + tmp3;
	fread_dice(mob_f, &tmp, &tmp2, &tmp3);
	mob->abilities.dex = dice(tmp, tmp2) + tmp3;
	fread_dice(mob_f, &tmp, &tmp2, &tmp3);
	mob->abilities.con = dice(tmp, tmp2) + tmp3;
	fread_dice(mob_f, &tmp, &tmp2, &tmp3);
	mob->abilities.intel = dice(tmp, tmp2) + tmp3;
	fread_dice(mob_f, &tmp, &tmp2, &tmp3);
	mob->abilities.wis = dice(tmp, tmp2) + tmp3;

	for (x = 0; x < 5; x++) {
	  tmp = 0;
	  fscanf(mob_f, " %ld ", &tmp);
	  if (!tmp)
	    tmp = MAX(20 - lvl, 2);
	  mob->specials.apply_saving_throw[x] = tmp;
	}
	fscanf(mob_f, "\n");
	fscanf(mob_f, " %ld %ld %ld %ld\n", &tmp, &tmp2, &tmp3, &tmp4);
	mob->specials.position = tmp;
	mob->specials.default_pos = tmp2;
	if (tmp3) {
	  mob->player.sounds = fread_string(mob_f);
	  mob->player.distant_snds = fread_string(mob_f);
	}
	if (tmp4) {
	  for (x = 0; x < tmp4; x++)
	    if ((fscanf(mob_f, " %ld %ld %ld\n", &tmp, &tmp2, &tmp3)) == 3) {
	      mob->skills[tmp].learned = tmp2;
	      mob->skills[tmp].recognise = tmp3;
	    }
	}
	mob->player.time.birth = time(0);
	mob->player.time.played = 0;
	mob->player.time.logon = time(0);
	for (x = 0; x < 3; x++)
	  GET_COND(mob, x) = -1;
      }
      break;
    default:{
	log_error("Unknown mobile type code '%c' in \"%s\"!  HELP!\r\n", letter, mob->player.name);
      }
      break;
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

/* read an object from OBJ_FILE */

struct obj_data                        *read_object(int nr, int type)
{
  struct obj_data                        *obj = NULL;
  int                                     tmp = 0;
  int                                     i = 0;
  char                                    chk[50] = "\0\0\0";
  char                                    buf[100] = "\0\0\0";
  struct extra_descr_data                *new_descr = NULL;

  if (DEBUG > 2)
    log_info("called %s with %d, %d", __PRETTY_FUNCTION__, nr, type);

  i = nr;
  if (type == VIRTUAL) {
    nr = real_object(nr);
  }
  if (nr < 0 || nr > top_of_objt) {
    sprintf(buf, "Object #%d does not exist.", i);
    return (0);
  }
  fseek(obj_f, obj_index[nr].pos, 0);
  obj = NULL;
  CREATE(obj, struct obj_data, 1);
  clear_object(obj);

  /*
   *** string data *** */

  obj->name = fread_string(obj_f);
  obj->short_description = fread_string(obj_f);
  obj->description = fread_string(obj_f);
  obj->action_description = fread_string(obj_f);

  /*
   *** numeric data *** */

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
   *** extra descriptions *** */

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

/* update zone ages, queue for reset if necessary, and dequeue when possible */
void zone_update(void)
{
  int                                     i = 0;
  struct reset_q_element                 *update_u = NULL;
  struct reset_q_element                 *temp = NULL;
  struct reset_q_element                 *tmp2 = NULL;

  if (DEBUG > 2)
    log_info("called %s with no arguments", __PRETTY_FUNCTION__);

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

    if (zone_table[update_u->zone_to_reset].reset_mode == 2
	|| is_empty(update_u->zone_to_reset)) {
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
      DESTROY(update_u);
    }
  }
}

/* execute the reset command table of a given zone */
void reset_zone(int zone)
{
  int                                     cmd_no = 0;
  int                                     last_cmd = 1;
  struct char_data                       *mob = NULL;
  struct obj_data                        *obj = NULL;
  struct obj_data                        *obj_to = NULL;
  struct room_data                       *rp = NULL;
  struct char_data                       *last_mob_loaded = NULL;

  if (DEBUG > 2)
    log_info("called %s with %d", __PRETTY_FUNCTION__, zone);

  log_reset("Zone Reset - %s (%d-%d)", ZNAME, (zone ? (zone_table[zone - 1].top + 1) : 0),
      zone_table[zone].top);
  for (cmd_no = 0;; cmd_no++) {
    if (DEBUG)
      log_info("Doing Command %d for %s: %c %d %d %d", cmd_no, ZNAME, ZCMD.command, ZCMD.arg1,
	  ZCMD.arg2, ZCMD.arg3);

    if (ZCMD.command == 'S')
      break;

    if (last_cmd || !ZCMD.if_flag)
      switch (ZCMD.command) {
	case 'M':					       /* read a mobile */
	  if (mob_index[ZCMD.arg1].number < ZCMD.arg2) {
	    mob = read_mobile(ZCMD.arg1, REAL);
	    char_to_room(mob, ZCMD.arg3);
	    last_mob_loaded = mob;
	    last_cmd = 1;
	  } else
	    last_cmd = 0;
	  break;

	case 'L':					       /* make a mob follow another */

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

	case 'O':					       /* read an object */
	  if (obj_index[ZCMD.arg1].number < ZCMD.arg2) {
	    if (ZCMD.arg3 >= 0 && ((rp = real_roomp(ZCMD.arg3)) != NULL)) {
	      if (!get_obj_in_list_num(ZCMD.arg1, rp->contents)
		  && (obj = read_object(ZCMD.arg1, REAL))) {
		obj_to_room(obj, ZCMD.arg3);
		last_cmd = 1;
	      } else
		last_cmd = 0;
	    } else if ((obj = read_object(ZCMD.arg1, REAL))) {
	      log_error("Error finding room #%d", ZCMD.arg3);
	      extract_obj(obj);
	      last_cmd = 1;
	    } else
	      last_cmd = 0;
	  }
	  break;

	case 'P':					       /* object to object */
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

	case 'G':					       /* obj_to_char */
	  if (obj_index[ZCMD.arg1].number < ZCMD.arg2 && (obj = read_object(ZCMD.arg1, REAL))) {
	    obj_to_char(obj, mob);
	    last_cmd = 1;
	  } else
	    last_cmd = 0;
	  break;

	case 'H':					       /* hatred to char */
	  if (AddHatred(mob, ZCMD.arg1, ZCMD.arg2))
	    last_cmd = 1;
	  else
	    last_cmd = 0;
	  break;

	case 'F':					       /* fear to char */

	  if (AddFears(mob, ZCMD.arg1, ZCMD.arg2))
	    last_cmd = 1;
	  else
	    last_cmd = 0;
	  break;

	case 'E':					       /* object to equipment list */
	  if (obj_index[ZCMD.arg1].number < ZCMD.arg2 && (obj = read_object(ZCMD.arg1, REAL))) {
	    if (ZCMD.arg3 > WIELD_TWOH)
	      log_error("BAD EQUIP in zone reboot.");
	    else
	      equip_char(mob, obj, ZCMD.arg3);
	    last_cmd = 1;
	  } else
	    last_cmd = 0;
	  break;

	case 'D':					       /* set state of door */
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
	  log_error("Undefd cmd in reset table; zone %d cmd %d.\r\n", zone, cmd_no);
	  break;
    } else
      last_cmd = 0;
  }
  zone_table[zone].age = 1 + fuzz(1);
}

/* for use in reset_zone; return TRUE if zone 'nr' is free of PC's  */
int is_empty(int zone_nr)
{
  struct descriptor_data                 *i = NULL;

  if (DEBUG > 2)
    log_info("called %s with %d", __PRETTY_FUNCTION__, zone_nr);

  for (i = descriptor_list; i; i = i->next)
    if (!i->connected)
      if (i->character->in_room != NOWHERE) {
	if (real_roomp(i->character->in_room)->zone == zone_nr)
	  return (0);
      }
  return (1);
}

/*
 * stuff related to the save/load player system
 */

int load_char(char *name, struct char_file_u *char_element)
{
  FILE                                   *fl = NULL;
  char                                    buf[256] = "\0\0\0";
  char                                    tname[40] = "\0\0\0";
  char                                   *t_ptr = NULL;

  if (DEBUG > 1)
    log_info("called %s with %s, %08zx", __PRETTY_FUNCTION__, VNULL(name), (size_t)char_element);

  strcpy(tname, name);
  t_ptr = tname;
  for (; *t_ptr != '\0'; t_ptr++)
    *t_ptr = LOWER(*t_ptr);

  sprintf(buf, "ply/%c/%s.p", tname[0], tname);
  if (!(fl = fopen(buf, "r+b")))
    return (-1);

  fread(char_element, sizeof(struct char_file_u), 1, fl);

  FCLOSE(fl);
/* Kludge for ressurection */
  char_element->talks[2] = TRUE;
  return (1);
}

/* copy data from the file structure to a char struct */
void store_to_char(struct char_file_u *st, struct char_data *ch)
{
  int                                     i = 0;
  long                                    t = 0L;

  if (DEBUG > 2)
    log_info("called %s with %08zx, %s", __PRETTY_FUNCTION__, (size_t)st, SAFE_NAME(ch));

/* This MIGHT be needed to do that strange password crap...
 * strcpy(ch->desc->pwd, st->pwd);
 */
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
    CREATE(ch->player.title, char, strlen   (st->title) + 1);

    strcpy(ch->player.title, st->title);
  } else
    GET_TITLE(ch) = 0;

  if (*st->pre_title) {
    CREATE(ch->player.pre_title, char, strlen(st->pre_title) + 1);

    strcpy(ch->player.pre_title, st->pre_title);
  } else
    GET_PRETITLE(ch) = 0;

  if (*st->description) {
    CREATE(ch->player.description, char, strlen(st->description) + 1);

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

  ch->specials.pracs = st->pracs;
  ch->specials.alignment = st->alignment;

  ch->specials.act = st->act;
  ch->specials.new_act = st->new_act;
  ch->specials.carry_weight = 0;
  ch->specials.carry_items = 0;
  ch->points.armor = 100;
  ch->points.hitroll = 0;
  ch->points.damroll = 0;

  CREATE(GET_NAME(ch), char, strlen       (st->name) + 1);

  strcpy(GET_NAME(ch), st->name);

  /*
   * Not used as far as I can see (Michael) 
   */
  for (i = 0; i < MAX_SAVING_THROWS; i++)
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

/* copy vital data from a players char-structure to the file structure */
void char_to_store(struct char_data *ch, struct char_file_u *st)
{
  int                                     i = 0;
  struct affected_type                   *af = NULL;
  struct obj_data                        *char_eq[MAX_WEAR];

  if (DEBUG > 2)
    log_info("called %s with %s, %08zx", __PRETTY_FUNCTION__, SAFE_NAME(ch), (size_t)st);

/* zero the structure.. hope this doesn't break things */
  bzero(st, sizeof(struct char_file_u));

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
		    st->affected[i].modifier, st->affected[i].bitvector, FALSE);
      af = af->next;
    } else {
      st->affected[i].type = 0;				       /* Zero signifies not used */
      st->affected[i].duration = 0;
      st->affected[i].modifier = 0;
      st->affected[i].location = 0;
      st->affected[i].bitvector = 0;
      st->affected[i].next = 0;
    }
  }

  if ((i >= MAX_AFFECT) && af && af->next)
    log_error("WARNING: OUT OF STORE ROOM FOR AFFECTED TYPES!!!");

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
  st->pracs = ch->specials.pracs;
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

  if (ch->desc)
    if (ch->desc->host)
      if (ch->desc->username) {
	char                                    ackpfft[256];

	sprintf(ackpfft, "%s@%s", ch->desc->username, ch->desc->host);
	strncpy(st->last_connect_site, ackpfft, 48);
      } else
	strncpy(st->last_connect_site, ch->desc->host, 48);
    else
      strcpy(st->last_connect_site, "unknown");
  else
    strcpy(st->last_connect_site, "unknown");

  for (i = 0; i <= MAX_TOUNGE - 1; i++)
    st->talks[i] = ch->player.talks[i];

  for (i = 0; i <= MAX_SKILLS - 1; i++)
    st->skills[i] = ch->skills[i];

  strcpy(st->name, GET_NAME(ch));

  for (i = 0; i < 5; i++)
    st->apply_saving_throw[i] = ch->specials.apply_saving_throw[i];

  for (i = 0; i < 3; i++)
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
		    st->affected[i].modifier, st->affected[i].bitvector, TRUE);
      af = af->next;
    }
  }

  for (i = 0; i < MAX_WEAR; i++) {
    if (char_eq[i])
      equip_char(ch, char_eq[i], i);
  }
  affect_total(ch);
}							       /* Char to store */

/* create a new entry in the in-memory index table for the player file */

int create_entry(char *name)
{
  int                                     i = 0;

  if (DEBUG > 2)
    log_info("called %s with %s", __PRETTY_FUNCTION__, VNULL(name));

  if (top_of_p_table == -1 || player_table == NULL) {
    CREATE(player_table, struct player_index_element, ((top_of_p_table = 0), 1));
  } else {
    RECREATE(player_table, struct player_index_element, ++top_of_p_table + 1);
  }

  CREATE(player_table[top_of_p_table].name, char, strlen(name) + 1);

  /*
   * copy lowercase equivalent of name to table field 
   */
  for (i = 0; (*(player_table[top_of_p_table].name + i) = LOWER(*(name + i))); i++);

  player_table[top_of_p_table].nr = top_of_p_table;

  return (top_of_p_table);
}

/* write the vital data of a player to the player file */

void save_char(struct char_data *ch, short int load_room)
{
#if 1
  struct char_file_u                      st;
  char                                    buf[256] = "\0\0\0";
  char                                    name[40] = "\0\0\0";
  char                                   *t_ptr = NULL;

  if (DEBUG > 1)
    log_info("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), load_room);

  if (IS_NPC(ch) || !ch->desc)
    return;

  char_to_store(ch, &st);

  st.load_room = load_room;
  strcpy(st.pwd, ch->desc->pwd);
  strcpy(st.oldpwd, ch->desc->oldpwd);

  strcpy(name, GET_NAME(ch));
  t_ptr = name;
  for (; *t_ptr != '\0'; t_ptr++)
    *t_ptr = LOWER(*t_ptr);

  sprintf(buf, "ply/%c/%s.chr", name[0], name);
  new_save_char(&st, buf, ch);
#else
  int                                     i = 0;
  struct obj_data                        *char_equip[MAX_WEAR];
  struct affected_type                   *af = NULL;
  struct affected_type                   *affect = NULL;
  int                                     naf = 0;
  FILE                                   *fp = NULL;
  char                                    tmp[256] = "\0\0\0";

  if (DEBUG > 2)
    log_info("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), load_room);

/* NPC followers will be saved in the character file, storing
 * vnum, hp, level, exp, and charm-duration (-1 for infinite)
 */
  if (!ch || IS_NPC(ch) || !ch->desc)
    return;

/* We remove all equipment for the save, thus cleaning up affects
 * from magical-items.  We can re-equip after the save is done.
 * Each object must have its equip pos saved too, or -1 for inventory.
 */
  for (i = 0; i < MAX_WEAR; i++)
    if (ch->equipment[i])
      char_equip[i] = unequip_char(ch, i);
    else
      char_equip[i] = 0;
/* We also have to strip spell effects so they don't get
 * duplicated.
 */
  for (af = ch->affected; af; af = af->next) {
    if (!affect) {
      CREATE(affect, struct affected_type, 1);
    } else {
      RECREATE(affect, struct affected_type, ++naf + 1);
    }
    affect[naf] = *af;
    affect[naf].next = NULL;
    affect_modify(ch, affect[naf].location, affect[naf].modifier, affect[naf].bitvector, FALSE);
  }
  ch->tmpabilities = ch->abilities;			       /* They will be restored at the end */

  sprintf(tmp, "ply/%c/%s.chr", name[0], name);
  if (!(fp = fopen(tmp, "w"))) {
    log_fatal("save char: cannot open output file");
    proper_exit(MUD_HALT);
  }

/*
 * Here we put things back as they were, in case we
 * are saving but not quitting.
 */
  if (affect) {
    for (; naf >= 0; naf--)
      affect_modify(ch, affect[naf].location, affect[naf].modifier, affect[naf].bitvector,
		    TRUE);
    DESTROY(affect);
  }
  for (i = 0; i < MAX_WEAR; i++)
    if (char_equip[i])
      equip_char(ch, char_equip[i], i);
  affect_total(ch);
/* Actually save equipment here, so it is worn properly. */
#endif
}

void new_save_char(struct char_file_u *ch, char *filename, struct char_data *xch)
{
#if 0
  struct char_file_u {
    char                                    name[20];
    char                                    pwd[11];
    char                                    title[80];
    char                                    pre_title[80];
    char                                    sex;
    char                                    class;
    char                                    last_connect_site[49];
    char                                    level[MAX_NUMBER_OF_CLASSES];
    time_t                                  birth;	       /* Time of birth of character */
    int                                     played;	       /* Number of secs played in total */
    int                                     race;
    unsigned char                           weight;
    unsigned char                           height;
    char                                    poof_in[80];
    char                                    poof_out[80];
    short int                               hometown;
    char                                    description[240];
    char                                    talks[MAX_TOUNGE];
    short int                               load_room;	       /* Which room to place char in */
    struct char_ability_data                abilities;
    struct char_point_data                  points;
    struct char_skill_data                  skills[MAX_SKILLS];
    struct affected_type                    affected[MAX_AFFECT];
    int                                     pracs;
    int                                     skills_to_learn;
    int                                     alignment;
    time_t                                  last_logon;	       /* Time (in secs) of last logon */
    unsigned char                           act;	       /* ACT Flags */
    int                                     new_act;
    short int                               apply_saving_throw[5];
    int                                     conditions[MAX_CONDITIONS];
    short int                               sh_save_blah1;
    short int                               sh_save_blah2;
    short int                               sh_save_blah3;
    short int                               sh_save_blah4;
    int                                     save_blah1;
    int                                     save_blah2;
    int                                     save_blah3;
    int                                     save_blah4;
  };
#endif
  FILE                                   *fp = NULL;
  int                                     i = 0;

  if (DEBUG > 2)
    log_info("called %s with %08zx, %s", __PRETTY_FUNCTION__, (size_t)ch, VNULL(filename));

  if (!(fp = fopen(filename, "w"))) {
    log_fatal("new save char");
    proper_exit(MUD_HALT);
  }
  fprintf(fp, "#PLAYER\n");
  fprintf(fp, "Name               %s~\n", ch->name);
  fprintf(fp, "Passwd             %s~\n", ch->pwd);
  fprintf(fp, "Whizz              %d\n", ch->points.wiz_priv);
  fprintf(fp, "PreTitle           %s~\n", ch->pre_title);
  fprintf(fp, "Title              %s~\n", ch->title);
  fprintf(fp, "Description\n%s~\n", fix_string(ch->description));
  fprintf(fp, "LastSite           %s~\n", ch->last_connect_site);
  fprintf(fp, "LastLogin          %ld\n", (long)ch->last_logon);
  fprintf(fp, "Birth              %ld\n", (long)ch->birth);
  fprintf(fp, "Played             %d\n", ch->played);
  fprintf(fp, "Sex                %d\n", (int)ch->sex);
  fprintf(fp, "Race               %d\n", ch->race);
  fprintf(fp, "Class              %d\n", (int)ch->class);
  fprintf(fp, "Alignment          %d\n", ch->alignment);
  fprintf(fp, "Exp                %ld\n", (long)(ch->points.exp));
  for (i = 0; i < ABS_MAX_CLASS; i++)
    if (ch->level[i])
      fprintf(fp, "Level              %s %d\n", class_name[i], (int)(ch->level[i]));
  fprintf(fp, "HeightWeight       %d %d\n", (int)ch->height, (int)ch->weight);
  fprintf(fp, "Gold               %d %d\n", (int)(ch->points.gold), (int)(ch->points.bankgold));
  fprintf(fp, "HomeTown           %d\n", ch->hometown);
  fprintf(fp, "LoadRoom           %d\n", ch->load_room);
  fprintf(fp, "PoofIn             %s~\n", ch->poof_in);
  fprintf(fp, "PoofOut            %s~\n", ch->poof_out);
  fprintf(fp, "AbilityScores      %d %d %d %d %d %d\n",
	  (int)(ch->abilities.str), (int)(ch->abilities.str_add),
	  (int)(ch->abilities.dex), (int)(ch->abilities.con),
	  (int)(ch->abilities.intel), (int)(ch->abilities.wis));
  fprintf(fp, "AbilityPad         %d %d %d %d\n",
	  (int)(ch->abilities.d1), (int)(ch->abilities.d2),
	  (int)(ch->abilities.d3), (int)(ch->abilities.d4));
  fprintf(fp, "HpManaMove         %d %d %d %d %d %d\n",
	  (int)(ch->points.hit), (int)(ch->points.max_hit),
	  (int)(ch->points.mana), (int)(ch->points.max_mana),
	  (int)(ch->points.move), (int)(ch->points.max_move));
  fprintf(fp, "AC                 %d\n", (int)(ch->points.armor));
  fprintf(fp, "ToHitDamage        %d %d\n",
	  (int)(ch->points.hitroll), (int)(ch->points.damroll));
  fprintf(fp, "SaveApply          %d %d %d %d %d\n",
	  (int)(ch->apply_saving_throw[0]), (int)(ch->apply_saving_throw[1]),
	  (int)(ch->apply_saving_throw[2]), (int)(ch->apply_saving_throw[3]),
	  (int)(ch->apply_saving_throw[4]));
  fprintf(fp, "Conditions         %d %d %d %d %d %d\n",
	  (int)(ch->conditions[0]), (int)(ch->conditions[1]),
	  (int)(ch->conditions[2]), (int)(ch->conditions[3]),
	  (int)(ch->conditions[4]), (int)(ch->conditions[5]));
  fprintf(fp, "PointsPad1         %d %d %d\n",
	  (int)(ch->points.blah1), (int)(ch->points.blah2), (int)(ch->points.blah3));
  fprintf(fp, "PointsPad2         %d %d %d %d\n",
	  (int)(ch->points.i1), (int)(ch->points.i2),
	  (int)(ch->points.i3), (int)(ch->points.i4));
  fprintf(fp, "PointsPad3         %d %d %d\n",
	  (int)(ch->points.s1), (int)(ch->points.s2), (int)(ch->points.s3));
  fprintf(fp, "ShortSavePad       %d %d %d %d\n",
	  (int)(ch->sh_save_blah1), (int)(ch->sh_save_blah2),
	  (int)(ch->sh_save_blah3), (int)(ch->sh_save_blah4));
  fprintf(fp, "SavePad            %d %d %d %d\n",
	  ch->save_blah1, ch->save_blah2, ch->save_blah3, ch->save_blah4);
  fprintf(fp, "Pracs              %d\n", ch->pracs);
  fprintf(fp, "SkillsToLearn      %d\n", ch->skills_to_learn);
#if 0
  for (i = 0; i < MAX_SKILLS; i++)
    fprintf(fp, "Skill              %d %d %d \"%s\"\n", i,
	    (int)(ch->skills[i].learned), (int)(ch->skills[i].recognise),
	    (i ? spell_info[i].name : "none"));
#else
  for (i = 0; i < MAX_SKILLS; i++)
    if (spell_info[i].name && spell_info[i].name[0])
      fprintf(fp, "NamedSkill         %d %d %d %s~\n", i,
	      (int)(ch->skills[i].learned), (int)(ch->skills[i].recognise), spell_info[i].name);
#endif
  fprintf(fp, "ActFlags           %d %d\n", (int)(ch->act), ch->new_act);
  for (i = 0; i < MAX_AFFECT; i++)
    fprintf(fp, "Affect             %d %d %d %d %d %ld %lu\n", i,
	    (int)(ch->affected[i].type),
	    (int)(ch->affected[i].duration),
	    (int)(ch->affected[i].modifier),
	    (int)(ch->affected[i].location),
	    ch->affected[i].bitvector, (unsigned long)(ch->affected[i].next));
#ifdef IMC
  imc_savechar( xch, fp );
#endif
  fprintf(fp, "End\n");
  FCLOSE(fp);
}

/* for possible later use with qsort */
int compare(struct player_index_element *arg1, struct player_index_element *arg2)
{
  if (DEBUG > 2)
    log_info("called %s with %08zx, %08zx", __PRETTY_FUNCTION__, (size_t)arg1, (size_t)arg2);

  /* Allow this to blow up on NULL's, it will aid in debugging */
  return (str_cmp(arg1->name, arg2->name));
}

/*
 * procs of a (more or less) general utility nature
 */

/* release memory allocated for a char struct */
void free_char(struct char_data *ch)
{
  struct affected_type                   *af = NULL;

  if (DEBUG > 2)
    log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  DESTROY(GET_NAME(ch));

  if (ch->player.title)
    DESTROY(ch->player.title);
  if (ch->player.pre_title)
    DESTROY(ch->player.pre_title);
  if (ch->player.short_descr)
    DESTROY(ch->player.short_descr);
  if (ch->player.long_descr)
    DESTROY(ch->player.long_descr);
  if (ch->player.description)
    DESTROY(ch->player.description);
  if (ch->player.sounds)
    DESTROY(ch->player.sounds);
  if (ch->player.distant_snds)
    DESTROY(ch->player.distant_snds);

  for (af = ch->affected; af; af = af->next)
    affect_remove(ch, af);

#ifdef IMC
  imc_freechardata( ch );
#endif

  DESTROY(ch);
}

/* release memory allocated for an obj struct */
void free_obj(struct obj_data *obj)
{
  struct extra_descr_data                *this = NULL;
  struct extra_descr_data                *next_one = NULL;

  if (DEBUG > 2)
    log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_ONAME(obj));

  DESTROY(obj->name);
  if (obj->description)
    DESTROY(obj->description);
  if (obj->short_description)
    DESTROY(obj->short_description);
  if (obj->action_description)
    DESTROY(obj->action_description);

  for (this = obj->ex_description; (this != 0); this = next_one) {
    next_one = this->next;
    if (this->keyword)
      DESTROY(this->keyword);
    if (this->description)
      DESTROY(this->description);
    DESTROY(this);
  }

  DESTROY(obj);
}

/* read contents of a text file, and place in buf */
int file_to_string(const char *name, char *buf)
{
  FILE                                   *fl = NULL;
  char                                    tmp[100] = "\0\0\0";

  if (DEBUG > 2)
    log_info("called %s with %s, %s", __PRETTY_FUNCTION__, VNULL(name), VNULL(buf));

  *buf = '\0';

  if (!(fl = fopen(name, "r"))) {
    log_error("file-to-string");
    *buf = '\0';
    return (-1);
  }
  do {
    fgets(tmp, 99, fl);

    if (!feof(fl)) {
      if (strlen(buf) + strlen(tmp) + 2 > MAX_STRING_LENGTH) {
	log_error("fl->strng: string too big (db.c, file_to_string)");
	*buf = '\0';
	FCLOSE(fl);
	return (-1);
      }
      strcat(buf, tmp);
      *(buf + strlen(buf) + 1) = '\0';
      *(buf + strlen(buf)) = '\r';
    }
  }
  while (!feof(fl));

  FCLOSE(fl);

  return (0);
}

/* read contents of a text file, and place in buf */
int file_to_prompt(const char *name, char *buf)
{
  FILE                                   *fl = NULL;
  char                                    tmp[100] = "\0\0\0";

  if (DEBUG > 2)
    log_info("called %s with %s, %s", __PRETTY_FUNCTION__, VNULL(name), VNULL(buf));

  *buf = '\0';

  if (!(fl = fopen(name, "r"))) {
    log_error("file-to-prompt");
    *buf = '\0';
    return (-1);
  }
  do {
    fgets(tmp, 99, fl);

    if (!feof(fl)) {
      if (strlen(buf) + strlen(tmp) + 2 > MAX_STRING_LENGTH) {
	log_error("fl->strng: string too big (db.c, file_to_string)");
	*buf = '\0';
	FCLOSE(fl);
	return (-1);
      }
      strcat(buf, tmp);
      *(buf + strlen(buf) + 1) = '\0';
      *(buf + strlen(buf)) = '\r';
    }
  }
  while (!feof(fl));
  if (strlen(buf) > 2)
    if (buf[strlen(buf) - 2] == '\n')
      buf[strlen(buf) - 2] = '\0';

  FCLOSE(fl);

  return (0);
}

/* clear some of the the working variables of a char */
void reset_char(struct char_data *ch)
{
  int                                     i = 0;

  if (DEBUG > 2)
    log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  for (i = 0; i < MAX_WEAR; i++)			       /* Initializing */
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
    cprintf(ch, "Setting your class to THIEF only.\r\n");
  }
  for (i = 0; i < ABS_MAX_CLASS; i++) {
    if (GET_LEVEL(ch, i) > LOKI) {
      GET_LEVEL(ch, i) = LOW_IMMORTAL;
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
  GET_AC(ch) += dex_app[(int)GET_DEX(ch)].defensive;
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
    log_info("%s has %d coins in bank.", GET_NAME(ch), GET_BANK(ch));
  }
  if (GET_GOLD(ch) > 500000) {
    log_info("%s has %d coins.", GET_NAME(ch), GET_GOLD(ch));
  }
}

/* clear ALL the working variables of a char and do NOT free any space alloc'ed */
void clear_char(struct char_data *ch)
{
  if (DEBUG > 2)
    log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  memset(ch, '\0', sizeof(struct char_data));

  ch->in_room = NOWHERE;
  ch->specials.mounted_on = 0;
  ch->specials.ridden_by = 0;
  ch->hates.clist = 0;
  ch->fears.clist = 0;
  ch->specials.was_in_room = NOWHERE;
  ch->specials.position = POSITION_STANDING;
  ch->specials.default_pos = POSITION_STANDING;
  GET_AC(ch) = 100;					       /* Basic Armor */
}

void clear_object(struct obj_data *obj)
{
  if (DEBUG > 2)
    log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_ONAME(obj));

  memset(obj, '\0', sizeof(struct obj_data));

  obj->item_number = -1;
  obj->in_room = NOWHERE;
}

/* initialize a new character only if class is set */
void init_char(struct char_data *ch)
{
  int                                     i = 0;

  if (DEBUG > 2)
    log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  /*
   * if this is our first player --- he be God
   */

  if (!strcmp(GET_NAME(ch), "Quixadhal")) {
    int                                     x = 0;

    GET_EXP(ch) = 24000000;
    for (x = 0; x < ABS_MAX_CLASS; x++) {
      GET_LEVEL(ch, x) = LOKI;
      ch->player.class |= 1 << x;
    }
  }

  set_title(ch);

  ch->player.short_descr = 0;
  ch->player.long_descr = 0;
  ch->player.description = 0;

  ch->player.hometown = DEFAULT_HOME;			       /* Rental area of shylar */

  ch->player.time.birth = time(0);
  ch->player.time.played = 0;
  ch->player.time.logon = time(0);

  for (i = 0; i < MAX_TOUNGE; i++)
    ch->player.talks[i] = 0;

  GET_STR(ch) = 9;
  GET_ADD(ch) = 0;
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
  ch->specials.pracs = 0;

  for (i = 0; i < 5; i++)
    ch->specials.apply_saving_throw[i] = 0;

  for (i = 0; i < 3; i++)
    GET_COND(ch, i) = (GetMaxLevel(ch) > 51 ? -1 : 24);
}

struct room_data                       *real_roomp(int virtual)
{
  if (DEBUG > 3)
    log_info("called %s with %d", __PRETTY_FUNCTION__, virtual);

  return hash_find(&room_db, virtual);
}

/* returns the real number of the monster with given virtual number */
int real_mobile(int virtual)
{
  int                                     bot = 0;
  int                                     top = 0;
  int                                     mid = 0;

  if (DEBUG > 2)
    log_info("called %s with %d", __PRETTY_FUNCTION__, virtual);

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

/* returns the real number of the object with given virtual number */
int real_object(int virtual)
{
  int                                     bot = 0;
  int                                     top = 0;
  int                                     mid = 0;

  if (DEBUG > 2)
    log_info("called %s with %d", __PRETTY_FUNCTION__, virtual);

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

char                                   *fix_string(const char *str)
{
  static char                             strfix[MAX_STRING_LENGTH] = "\0\0\0";
  int                                     i = 0;
  int                                     o = 0;

  if (DEBUG > 2)
    log_info("called %s with %s", __PRETTY_FUNCTION__, VNULL(str));

  if (str) {
    for (i = o = 0; str[i + o]; i++) {
      if (str[i + o] == '\r' || str[i + o] == '~')
	o++;
      strfix[i] = str[i + o];
    }
  }
  strfix[i] = '\0';
  return strfix;
}

/*
 * read and allocate space for a '~'-terminated string from a given file
 * modified to skip leading tabs.... provides for a compatible interface
 * for reading ascii save files (unless you are weird enough to use tabs
 * in your descriptions?)
 */
char                                   *fread_string(FILE * fl)
{
  char                                   *point = NULL;
  char                                   *ack = NULL;
  char                                   *rslt = NULL;
  char                                    buf[MAX_STRING_LENGTH] = "\0\0\0";
  char                                    tmp[MAX_STRING_LENGTH] = "\0\0\0";
  int                                     flag = FALSE;
  static char                             Empty[6] = "Empty";

  if (DEBUG > 2)
    log_info("called %s with %08zx", __PRETTY_FUNCTION__, (size_t)fl);

  bzero(buf, sizeof(buf));
  bzero(tmp, sizeof(tmp));

  do {
    if (!fgets(tmp, MAX_STRING_LENGTH, fl)) {
      log_error("fread_str");
      return (Empty);
    }
    ack = tmp;
    if (strlen(ack) + strlen(buf) + 1 > MAX_STRING_LENGTH) {
      ack[MAX_STRING_LENGTH - strlen(buf) - 2] = '\0';
      log_error("fread_string: string too long, truncating!\n%s\n", buf);
    }
    strcat(buf, ack);

    for (point = buf + strlen(buf) - 2; point >= buf && isspace(*point); point--);
    if ((flag = (*point == '~')))
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
    CREATE(rslt, char, strlen               (buf) + 1);

    strcpy(rslt, buf);
  } else
    rslt = 0;
  return (rslt);
}

/*
 * Read one word (into static buffer).
 */
char                                   *fread_word(FILE * fp)
{
  static char                             word[MAX_INPUT_LENGTH] = "\0\0\0";
  char                                   *pword = NULL;
  char                                    cEnd = '\0';

  if (DEBUG > 2)
    log_info("called %s with %08zx", __PRETTY_FUNCTION__, (size_t)fp);

  do {
    cEnd = getc(fp);
  } while (isspace(cEnd));

  if (cEnd == '\'' || cEnd == '"') {
    pword = word;
  } else {
    word[0] = cEnd;
    pword = word + 1;
    cEnd = ' ';
  }

  for (; pword < word + MAX_INPUT_LENGTH; pword++) {
    *pword = getc(fp);
    if (cEnd == ' ' ? isspace(*pword) : *pword == cEnd) {
      if (cEnd == ' ')
	ungetc(*pword, fp);
      *pword = '\0';
      return word;
    }
  }
  word[MAX_INPUT_LENGTH - 1] = '\0';
  log_error("Fread_word: word too long.\n%s\n", word);
  return word;
}

/*
 * Read a number from a file.
 */
int fread_number(FILE * fp)
{
  int                                     num = 0;
  unsigned char                           sign = '\0';
  char                                    c = '\0';

  if (DEBUG > 2)
    log_info("called %s with %08zx", __PRETTY_FUNCTION__, (size_t)fp);

  do {
    c = getc(fp);
  } while (isspace(c));

  num = 0;

  sign = FALSE;
  if (c == '+') {
    c = getc(fp);
  } else if (c == '-') {
    sign = TRUE;
    c = getc(fp);
  }
  if (!isdigit(c)) {
    log_fatal("Fread_number: bad format.\nOffending char = '%c'\n", c);
    proper_exit(MUD_HALT);
  }
  while (isdigit(c)) {
    num = num * 10 + c - '0';
    c = getc(fp);
  }

  if (sign)
    num = 0 - num;

  if (c == '|')
    num += fread_number(fp);
  else if (c != ' ')
    ungetc(c, fp);

  return num;
}

/*
 * Read to end of line (for comments).
 */
void fread_to_eol(FILE * fp)
{
  char                                    c = '\0';

  if (DEBUG > 2)
    log_info("called %s with %08zx", __PRETTY_FUNCTION__, (size_t)fp);

  c = getc(fp);
  while (c != '\n' && c != '\r')
    c = getc(fp);
  do {
    c = getc(fp);
  } while (c == '\n' || c == '\r');
  ungetc(c, fp);
}

/*
 * Read and allocate space for a string from a file.
 */
char                                   *new_fread_string(FILE * fp)
{
  static char                             buf[MAX_STRING_LENGTH] = "\0\0\0";
  char                                   *ack = NULL;
  int                                     flag = FALSE;
  char                                    c = '\0';
  static char                             Empty[1] = "";

  if (DEBUG > 2)
    log_info("called %s with %08zx", __PRETTY_FUNCTION__, (size_t)fp);

  bzero(buf, MAX_STRING_LENGTH);
  ack = buf;
  flag = 0;
  do {
    c = getc(fp);
  } while (isspace(c));

  if ((*ack++ = c) == '~')
    return Empty;

  for (;;) {
    if (ack > &buf[MAX_STRING_LENGTH - 1]) {
      log_error("new_fread_string: MAX_STRING %d exceeded, truncating.", MAX_STRING_LENGTH);
      return buf;
    }
    switch (*ack = getc(fp)) {
      default:
	flag = 0;
	ack++;
	break;
      case EOF:
	log_error("Fread_string: EOF");
	return buf;
      case '\r':
	break;
      case '~':
	ack++;
	flag = 1;
	break;
      case '\n':
	if (flag) {
	  if (ack > buf) {
	    ack--;
	    *ack = '\0';
	  }
	  return buf;
	} else {
	  flag = 0;
	  ack++;
	  *ack++ = '\r';
	}
	break;
    }
  }
}

int fread_char(char *name, struct char_file_u *ch, struct char_data *xch)
{
  FILE                                   *fp = NULL;
  char                                   *t_ptr = NULL;
  char                                   *word = NULL;
  unsigned char                           fMatch = FALSE;
  char                                    tname[40] = "\0\0\0";
  char                                    buf[MAX_STRING_LENGTH] = "\0\0\0";
  static char                             End[4] = "End";

  if (DEBUG > 2)
    log_info("called %s with %s, %08zx", __PRETTY_FUNCTION__, VNULL(name), (size_t)ch);

  strcpy(tname, name);
  t_ptr = tname;
  for (; *t_ptr != '\0'; t_ptr++)
    *t_ptr = LOWER(*t_ptr);

  sprintf(buf, "ply/%c/%s.chr", tname[0], tname);
  if (!(fp = fopen(buf, "r")))
    return (-1);

  bzero(ch, sizeof(struct char_file_u));
  for (;;) {
    word = feof(fp) ? End : fread_word(fp);
    fMatch = FALSE;

    switch (toupper(word[0])) {
      case '*':
	fMatch = TRUE;
	fread_to_eol(fp);
	break;
      case '#':
	fMatch = TRUE;
	fread_to_eol(fp);
	break;

      case 'A':
	KEY("Alignment", ch->alignment, fread_number(fp));
	KEY("AC", ch->points.armor, fread_number(fp));
	if (!str_cmp(word, "AbilityScores")) {
	  ch->abilities.str = fread_number(fp);
	  ch->abilities.str_add = fread_number(fp);
	  ch->abilities.dex = fread_number(fp);
	  ch->abilities.con = fread_number(fp);
	  ch->abilities.intel = fread_number(fp);
	  ch->abilities.wis = fread_number(fp);
	  fMatch = TRUE;
	  break;
	}
	if (!str_cmp(word, "AbilityPad")) {
	  ch->abilities.d1 = fread_number(fp);
	  ch->abilities.d2 = fread_number(fp);
	  ch->abilities.d3 = fread_number(fp);
	  ch->abilities.d4 = fread_number(fp);
	  fMatch = TRUE;
	  break;
	}
	if (!str_cmp(word, "ActFlags")) {
	  ch->act = fread_number(fp);
	  ch->new_act = fread_number(fp);
	  fMatch = TRUE;
	  break;
	}
	if (!str_cmp(word, "Affect")) {
	  int                                     x;

	  x = fread_number(fp);
	  ch->affected[x].type = fread_number(fp);
	  ch->affected[x].duration = fread_number(fp);
	  ch->affected[x].modifier = fread_number(fp);
	  ch->affected[x].location = fread_number(fp);
	  ch->affected[x].bitvector = fread_number(fp);
	  ch->affected[x].next = (struct affected_type *)((unsigned long)fread_number(fp));
	  fMatch = TRUE;
	  break;
	}
	break;

      case 'B':
	KEY("Birth", ch->birth, fread_number(fp));
	break;

      case 'C':
	KEY("Class", ch->class, fread_number(fp));
	if (!str_cmp(word, "Conditions")) {
	  int                                     x = 0;

	  for (x = 0; x < MAX_CONDITIONS; x++)
	    ch->conditions[x] = fread_number(fp);
	  fMatch = TRUE;
	  break;
	}
	break;

      case 'D':
	CKEY("Description", ch->description, new_fread_string(fp));
	break;

      case 'E':
	if (!str_cmp(word, "End"))
	  return 0;
	KEY("Exp", ch->points.exp, fread_number(fp));
	break;

      case 'G':
	if (!str_cmp(word, "Gold")) {
	  ch->points.gold = fread_number(fp);
	  ch->points.bankgold = fread_number(fp);
	  fMatch = TRUE;
	  break;
	}
	break;

      case 'H':
	KEY("HomeTown", ch->hometown, fread_number(fp));
	if (!str_cmp(word, "HeightWeight")) {
	  ch->height = fread_number(fp);
	  ch->weight = fread_number(fp);
	  fMatch = TRUE;
	  break;
	}
	if (!str_cmp(word, "HpManaMove")) {
	  ch->points.hit = fread_number(fp);
	  ch->points.max_hit = fread_number(fp);
	  ch->points.mana = fread_number(fp);
	  ch->points.max_mana = fread_number(fp);
	  ch->points.move = fread_number(fp);
	  ch->points.max_move = fread_number(fp);
	  fMatch = TRUE;
	  break;
	}
	break;

      case 'I':
#ifdef IMC
        if( ( fMatch = imc_loadchar( xch, fp, word ) ) )
          break;
#endif
	break;

      case 'L':
	CKEY("LastSite", ch->last_connect_site, new_fread_string(fp));
	KEY("LastLogin", ch->last_logon, fread_number(fp));
	KEY("LoadRoom", ch->load_room, fread_number(fp));
	if (!str_cmp(word, "Level")) {
	  int                                     x = 0;
	  char                                   *cl = NULL;

	  cl = fread_word(fp);

	  for (x = 0; x < ABS_MAX_CLASS; x++)
	    if (!str_cmp(cl, (const char *)class_name[x])) {
	      ch->level[x] = fread_number(fp);
	      fMatch = TRUE;
	    }
	  break;
	}
	break;

      case 'N':
	CKEY("Name", ch->name, new_fread_string(fp));
	if (!str_cmp(word, "NamedSkill")) {
	  int                                     skill_number = 0;
	  int                                     learned = 0;
	  int                                     recognise = 0;
	  int                                     sn = 0;
	  const char                             *arg = NULL;
          char                                    tmparg[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
          char                                   *t = tmparg;
	  int                                     x = 0;

	  skill_number = fread_number(fp);
	  learned = fread_number(fp);
	  recognise = fread_number(fp);
	  arg = new_fread_string(fp);
	  if (!arg || !(*arg))
	    break;
	  arg = skip_spaces(arg);
#if 1
	  if (*arg == '\'')
	    arg++;
          strncpy(tmparg, arg, MAX_INPUT_LENGTH);
	  for (t = tmparg; *t && *t != '\''; *t = tolower(*t))
	    t++;
	  if (*t == '\'')
	    *t = '\0';
	  if (!strlen(tmparg)) {
	    log_error("Empty skill name:  %d\n", skill_number);
	    break;
	  }
	  for (sn = -1, x = 0; x < MAX_SKILLS; x++)
	    if (!str_cmp(tmparg, spell_info[x].name))
	      sn = x;
#else
	  if (*arg == '\'')
	    arg++;
	  for (s = arg; *s && *s != '\''; *s = tolower(*s))
	    s++;
	  if (*s == '\'')
	    *s = '\0';
	  if (!strlen(arg)) {
	    dlog("Empty skill name:  %d\n", skill_number);
	    break;
	  }
	  for (sn = -1, x = 0; x < MAX_SKILLS; x++)
	    if (!str_cmp(arg, spell_info[x].name))
	      sn = x;
#endif
	  if (sn != skill_number) {
	    log_error("Skill mismatch: %d read vs. %d lookup\nUsing lookup version.\n", skill_number, sn);
	  }
	  if (sn < 0) {
	    log_error("Unknown skill name:  %d\n", skill_number);
	    if (skill_number < 0 || skill_number >= MAX_SKILLS) {
	      log_error("Totally invalid skill... ignoring.\n");
	      break;
	    } else {
	      log_error("Using slot-number read in as a last resort.\n");
	      sn = skill_number;
	    }
	  }
	  ch->skills[sn].learned = learned;
	  ch->skills[sn].recognise = recognise;
	  fMatch = TRUE;
	  break;
	}
	break;

      case 'P':
	CKEY("Passwd", ch->pwd, new_fread_string(fp));
	CKEY("Password", ch->oldpwd, new_fread_string(fp));
	KEY("Played", ch->played, fread_number(fp));
	CKEY("PreTitle", ch->pre_title, new_fread_string(fp));
	CKEY("PoofIn", ch->poof_in, new_fread_string(fp));
	CKEY("PoofOut", ch->poof_out, new_fread_string(fp));
	KEY("Pracs", ch->pracs, fread_number(fp));
	if (!str_cmp(word, "PointsPad1")) {
	  ch->points.blah1 = fread_number(fp);
	  ch->points.blah2 = fread_number(fp);
	  ch->points.blah3 = fread_number(fp);
	  fMatch = TRUE;
	  break;
	}
	if (!str_cmp(word, "PointsPad2")) {
	  ch->points.i1 = fread_number(fp);
	  ch->points.i2 = fread_number(fp);
	  ch->points.i3 = fread_number(fp);
	  ch->points.i4 = fread_number(fp);
	  fMatch = TRUE;
	  break;
	}
	if (!str_cmp(word, "PointsPad3")) {
	  ch->points.s1 = fread_number(fp);
	  ch->points.s2 = fread_number(fp);
	  ch->points.s3 = fread_number(fp);
	  fMatch = TRUE;
	  break;
	}
	break;

      case 'R':
	KEY("Race", ch->race, fread_number(fp));
	break;

      case 'S':
	KEY("Sex", ch->sex, fread_number(fp));
	KEY("SpellsToLearn", ch->pracs, fread_number(fp));
	KEY("SkillsToLearn", ch->skills_to_learn, fread_number(fp));
	if (!str_cmp(word, "SaveApply")) {
	  int                                     x = 0;

	  for (x = 0; x < 5; x++)
	    ch->apply_saving_throw[x] = fread_number(fp);
	  fMatch = TRUE;
	  break;
	}
	if (!str_cmp(word, "ShortSavePad")) {
	  ch->sh_save_blah1 = fread_number(fp);
	  ch->sh_save_blah2 = fread_number(fp);
	  ch->sh_save_blah3 = fread_number(fp);
	  ch->sh_save_blah4 = fread_number(fp);
	  fMatch = TRUE;
	  break;
	}
	if (!str_cmp(word, "SavePad")) {
	  ch->save_blah1 = fread_number(fp);
	  ch->save_blah2 = fread_number(fp);
	  ch->save_blah3 = fread_number(fp);
	  ch->save_blah4 = fread_number(fp);
	  fMatch = TRUE;
	  break;
	}
	if (!str_cmp(word, "Skill")) {
	  int                                     sn;

	  sn = fread_number(fp);
	  ch->skills[sn].learned = fread_number(fp);
	  ch->skills[sn].recognise = fread_number(fp);
	  fread_to_eol(fp);
	  fMatch = TRUE;
	  break;
	}
	break;

      case 'T':
	CKEY("Title", ch->title, new_fread_string(fp));
	if (!str_cmp(word, "ToHitDamage")) {
	  ch->points.hitroll = fread_number(fp);
	  ch->points.damroll = fread_number(fp);
	  fMatch = TRUE;
	  break;
	}
	break;

      case 'W':
	KEY("Whizz", ch->points.wiz_priv, fread_number(fp));
	break;
    }

    if (!fMatch) {
      log_error("Fread_char: no match.");
      if (!feof(fp))
	fread_to_eol(fp);
    }
  }
}

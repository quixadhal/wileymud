#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <sys/timeb.h>

/*  This program may be freely desiminated to other sites with the
 * understanding that my (Mike Nikkel) and Chris Michaels name will
 * not be removed from the program header.  This program is released
 * as is and we claim no responsibility for any damage or difficulties
 * which may insue from the usage of this program.
 * Comments and suggestions may be sent to msn@minnie.bell.inmet.com.
 *
 *   I, Chris Meshkin, have taken some time to try to optimize and fix
 * many of the problems that I found in the code, as it came, from the net.
 * I may not have been successful in places.  However, any and all work that
 * I have done in this source code is hereby given that status of freely
 * redistributable software.  This means, in short, that you may use it in
 * any way you see fit, as limited by the above liscence, provided that you
 * acknowledge my work and don't try to claim it is your own.
 */

#define DFLT_PROGNAME	"mkworld"
#define DFLT_INPUT	"world.in"
#define DFLT_OUTPUT	"world.out"
#define TMP_FILE	"/tmp/world.%d"
#define EDIT		"/usr/local/bin/vim /tmp/world.%d"

#define ERR_NOINPUT	1	/* Cannot read input file */
#define ERR_BADROOMID	2	/* No valid room virtual number */
#define ERR_EOF		3	/* Unexpected EOF */
#define ERR_NOTITLE	4	/* No room title found */
#define ERR_BADTITLE	5	/* Room title not in correct format */
#define ERR_ROOMFLAGS	6	/* Zone, flags, or sector wrong */
#define ERR_TELEPORT	7	/* Invalid teleport flags */
#define ERR_RIVER	8	/* Invalid river flags */
#define ERR_BADEXIT	9	/* Invalid exit code */
#define ERR_EXITKEY	10	/* Invalid exit keyword list */
#define ERR_DOORFLAGS	11	/* Invalid door flags, key or target */
#define ERR_EXTRAKEY	12	/* Empty keyword list for extra desc */
#define ERR_BADAUX	13	/* Unknown aux code */
#define ERR_ERRLOG	14	/* Cannot create error log */
#define ERR_NOOUTPUT	15	/* Cannot create output file */

#define BUFFER_SIZE	256
#define PAGE_SIZE	4096

#define TELEPORT_ROOM		-1
#define	NOSWIM_ROOM		 7
#define	TRUE			 1
#define FALSE			 0

/*  Note that these commands are sometimes in order of preference
 * and therefore not alphabetical at all times.
 */

#define NORTH			 1
#define EAST			 2
#define SOUTH			 3
#define WEST			 4
#define UP			 5
#define DOWN			 6
#define UNLINK			 7
#define BRIEF			 8
#define CHANGE                   9
#define COPY			10
#define MAKE			11
#define DELETE			12
#define DESC			13
#define EXITS			14
#define EXTRA			15
#define FLAG			16
#define FORMAT			17
#define GOTO			18
#define LOOK			19
#define LIGHT			20
#define LINK			21
#define LIST			22
#define TITLE			23
#define SECTOR			24
#define ZONE			25
#define DIG			26
#define HELP			90
#define SAVE			91
#define QUIT			98

struct commands_st {
  char *name;
  int num;
} commands[] = {
  { "brief", BRIEF },
  { "change", CHANGE },
  { "copy", COPY },
  { "down", DOWN },
  { "desc", DESC },
  { "delete", DELETE },
  { "dig", DIG },
  { "east", EAST },
  { "exits", EXITS },
  { "extra", EXTRA },
  { "flag", FLAG },
  { "format", FORMAT },
  { "goto", GOTO },
  { "help", HELP },
  { "look", LOOK },
  { "light", LIGHT },
  { "link", LINK },
  { "list", LIST },
  { "make", MAKE },
  { "north", NORTH },
  { "quit", QUIT },
  { "south", SOUTH },
  { "save", SAVE },
  { "sector", SECTOR },
  { "title", TITLE },
  { "up", UP },
  { "unlink", UNLINK },
  { "west", WEST },
  { "zone", ZONE },
  { NULL, -1 }
};

struct dir_st {
  char *name;
  int num;
} directions[] = {
  { "north", 0 },
  { "east", 1 },
  { "south", 2 },
  { "west", 3 },
  { "up", 4 },
  { "down", 5 },
  { "No Where", -1 },
  { NULL, -1 }
};

const char *rev_text[] =
{
  "south",
  "west",
  "north",
  "east",
  "down",
  "up"
};

const int rev_dir[] =
{
  2,
  3,
  0,
  1,
  5,
  4
};

struct room_flg_st {
  char *name;
  int mask;
} room_flags[] = {
  { "Dark", 1 },
  { "Death", 2 },
  { "Nomob", 4 },
  { "Indoor", 8 },
  { "Peace", 16 },
  { "Nosteal", 32 },
  { "Nosummon", 64 },
  { "Nomagic", 128 },
  { "Tunnel", 256 },
  { "Private", 512 },
  { "Sound", 1024 },
  { NULL, -1 }
};

struct door_st {
  char *name;
  int num;
} door_states[] = {
  { "Open Passage", 0 },
  { "Door which is lockable/pickable", 1 },
  { "Door which is lockable", 2 },
  { "Secret Door which is lockable/pickable", 3 },
  { "Secret Door which is lockable", 4 },
  { NULL, -1 }
};

struct sector_st {
  char *name;
  int num;
} sector_types[] = {
  { "Inside", 0 },
  { "City", 1 },
  { "Field", 2 },
  { "Forest", 3 },
  { "Hill", 4 },
  { "Mountain", 5 },
  { "Swimming", 6 },
  { "River", 7 },
  { "Air", 8 },
  { "Underwater", 9 },
  { "Teleport", -1 },
  { NULL, -1 }
};

typedef struct Teleport_st {
  int time;
  int to_room;
  int do_look;
  int sector_type;
} Teleport_t;

typedef struct River_st {
  int speed;
  int direction;
} River_t;

typedef struct Exit_st {
  char *desc_ptr;
  char *keywords;
  int door_flag;
  int key_number;
  int to_room;
  struct Room_struct *real_to_room;
} Exit_t;

typedef struct Extra_st {
  char *keywords;
  char *desc_ptr;
} Extra_t;

typedef struct Room_struct {
  int room_id;
  char *title;
  char *desc_ptr;
  int zonenum;
  int room_flag;
  int sector_type;
  char *sound_ptr[2];
  Teleport_t *teleport;
  River_t *river;
  Exit_t exit[6];
  int number_extras;
  Extra_t *extra;
  struct Room_struct *prev;
  struct Room_struct *next;
} Room;

void Command_brief(void);
void Command_unlink(void);
void Command_change(void);
void Command_copy(void);
void Command_make(void);
void Command_desc(void);
void Command_delete(void);
void Command_exits(void);
void Command_extra(void);
void Command_flag(void);
void Command_format(void);
void Command_goto(void);
void Command_help(void);
void Command_light(void);
void Command_link(void);
void Command_list(void);
void Command_look(void);
void Command_save(void);
void Command_set(void);
void Command_title(void);
void Command_sector(void);
void Command_zone(void);
void Command_dig(void);

void Display_A_Room(Room *ptr);
void trunc_string(char *ptr);
Room *find_room(Room *current, int search_id);
void get_inputs(void);
void get_desc(char **ptr);
void input_desc(char *title, char **ptr);
void import_desc(char *title, char **ptr);
void put_desc(void);
int print_desc(void);
int direction_number(char *ptr);
Room *allocate_room(int temp_id);

void abug(char *File, char *Func, int Line, int Verbose, char *Str,...);
int scan_a_number(char *string, int *number);
void Load_Rooms(void);
void Link_World(void);
void Main_Loop(void);
void Command_copy_sub(Room *ptr, Room *from_ptr);
void Command_flag_sub(Room *ptr, int index);
int find_keyword(char *buffer);

/* Global Variables */

FILE *fp;
FILE *out;
char *progname, *filename, *errname, *outname;
char tempfile[256];
char inputs[10][BUFFER_SIZE];
char true_or_false[2][6] =
{"FALSE", "TRUE"};
int number_rooms = 0;
int lastzone= 0;
Room *bottom_of_world, *current, *top_of_world;

int Brief = FALSE;
int Light = FALSE;

#define bug(Str...) abug(__FILE__, __FUNCTION__, __LINE__, 1, Str ##)
#define log(Str...) abug(NULL, NULL, 0, 1, Str ##)

void abug(char *File, char *Func, int Line, int Verbose, char *Str,...)
{
  va_list arg;
  char Result[PAGE_SIZE];
  char Temp[PAGE_SIZE];
  char Time[BUFFER_SIZE];
  struct timeb right_now;
  struct tm *now_part;

  bzero(Result, PAGE_SIZE);
  va_start(arg, Str);
  if (Str && *Str) {
    vsprintf(Temp, Str, arg);
  } else
    strcpy(Temp, "PING!");
  va_end(arg);
  if(Verbose)
    sprintf(Result, "%s : ", progname);
  if (File || Func || Line) {
    ftime(&right_now);
    now_part= localtime((const time_t *)&right_now);
    sprintf(Time, "%02d.%02d.%02d.%02d.%02d.%02d.%03d",
            now_part->tm_year, now_part->tm_mon, now_part->tm_mday,
            now_part->tm_hour, now_part->tm_min, now_part->tm_sec,
            right_now.millitm);
    sprintf(Result + strlen(Result), "<: %s (", Time);
    if (File && *File) {
      strcat(Result, File);
    }
    if (Func && *Func)
      sprintf(Result + strlen(Result), ";%s", Func);
    if (Line)
      sprintf(Result + strlen(Result), ",%d)", Line);
    else
      strcat(Result, ")");
    sprintf(Result + strlen(Result), "\n %s : ", progname);
  }
  strcat(Result, Temp);
  printf("%s", Result);
  if(Verbose)
    printf("\n");
}

int scan_a_number(char *string, int *number)
{
  if(!string || !number)
    return 0;
  return sscanf(string, "%d", number);
}

int main(int argc, char **argv)
{
  char tmp_str[BUFFER_SIZE];
  char *blah;

  printf(
"\n"
"+--------------------------------------------------------------------------+\n"
"|            WileyMUD III: Worldmaker, Version 1.21, 95.12.15              |\n"
"|  Programmed by: Mike Nikkel             Original Code : Chris Michaels   |\n"
"|                  Enhanced and Debugged by: Chris Meshkin                 |\n"
"+--------------------------------------------------------------------------+\n"
"| Please send comments to msn@minnie.bell.inmet.com or                     |\n"
"|                                                 chris@yakko.cs.wmich.edu |\n"
"+--------------------------------------------------------------------------+\n"
"\n");

  if(argv[0]) progname= (char *)strdup(argv[0]);
  else progname= (char *)strdup(DFLT_PROGNAME);

  if (argc > 1) {
    filename= (char *)strdup(argv[1]);
    printf("Using %s as input data file.\n", filename);
    if (argc > 2)
      log("Extra arguments ignored.\n");
  } else {
    filename= (char *)strdup(DFLT_INPUT);
    printf("Using default input file, %s.\n", filename);
  }
  strcpy(tmp_str, filename);
  if((blah= strtok(tmp_str, ".\n\r\t\0"))) {
    errname= (char *)calloc(1, strlen(blah)+5);
    strcpy(errname, blah);
    strcat(errname, ".err");
  } else {
    errname= (char *)calloc(1, strlen(tmp_str)+5);
    strcpy(errname, tmp_str);
    strcat(errname, ".err");
  }

  Load_Rooms();
  Link_World();
  Main_Loop();
  sprintf(tempfile, TMP_FILE, getpid());
  unlink(tempfile);
  printf("Be seeing you.\n");
  exit(0);
}

void Load_Rooms(void)
{
  char tmp_str[BUFFER_SIZE];
  int index, index2, temp_id;
  int done;

  if (!(fp = fopen(filename, "r"))) {
    log("Cannot open %s for input!", filename);
    exit(ERR_NOINPUT);
  }
  printf("Loading world from %s...\r", filename);
  fflush(stdout);
  while (fgets(tmp_str, BUFFER_SIZE-1, fp)) {
    if(!strlen(tmp_str))
      continue;
    if (tmp_str[0] == '$') {
      printf("Loading world from %s...Done!                \n", filename);
      break;
    }
    if (tmp_str[0] != '#' || (sscanf(tmp_str, "#%d", &temp_id) == 0)) {
      log(
"Room %d: Expected Room ID of the form \"#%%d\", Got this instead:\n%s\n",
          number_rooms, tmp_str);
      exit(ERR_BADROOMID);
    }
    current = allocate_room(temp_id);
    if(!(number_rooms%100)) {
      printf("Loading world from %s...[#%d]\r", filename, current->room_id);
      fflush(stdout);
    }
    if (fgets(tmp_str, BUFFER_SIZE-1, fp) == NULL) {
      log("Room %d: Unexpected EOF reading room [#%d].", number_rooms,
          current->room_id);
      exit(ERR_EOF);
    }
    if (tmp_str[0] == '~') {
      log("Room %d: Empty title found in room [#%d].", number_rooms,
          current->room_id);
      exit(ERR_NOTITLE);
    }
    if (tmp_str[strlen(tmp_str) - 2] != '~') {
      log( "Room %d: Expected [#%d]'s Title to be in the form \"%%s~\", Got this instead:\n%s\n",
          number_rooms, current->room_id, tmp_str);
      exit(ERR_BADTITLE);
    }
    current->title = (char *)strdup(strtok(tmp_str, "~\n\r\0"));

    get_desc(&current->desc_ptr);

    if (fgets(tmp_str, BUFFER_SIZE-1, fp) == NULL) {
      log("Room %d: Unexpected EOF reading flags for room [#%d].", number_rooms,
          current->room_id);
      exit(ERR_EOF);
    }
    index = sscanf(tmp_str, "%d %d %d", &current->zonenum,
		   &current->room_flag,
		   &current->sector_type);
    if (index < 3) {
      trunc_string(tmp_str);
      log( "Room %d: Expected zone, flags and sector type for [#%d],\nFormat should have been \"%%d %%d %%d\", Got this instead:\n%s\n",
           number_rooms, current->room_id, tmp_str);
      exit(ERR_ROOMFLAGS);
    }

    if (current->sector_type == -1) {
      current->teleport= (Teleport_t *)calloc(1, sizeof(struct Teleport_st));
      index = sscanf(tmp_str, "%d %d %d %d %d %d %d", &current->zonenum,
		     &current->room_flag,
		     &current->sector_type,
		     &current->teleport->time,
		     &current->teleport->to_room,
		     &current->teleport->do_look,
		     &current->teleport->sector_type);
      if (index < 7) {
	trunc_string(tmp_str);
        log("Room %d: Invalid teleport flags in teleport room [#%d],\nExpected format \"%%d %%d -1 %%d %%d %%d %%d\", Got this:\n%s\n",
            number_rooms, current->room_id, tmp_str);
	exit(ERR_TELEPORT);
      }
    } else if (current->sector_type == 7) {
      current->river= (River_t *)calloc(1, sizeof(struct River_st));
      index = sscanf(tmp_str, "%d %d %d %d %d", &current->zonenum,
		     &current->room_flag,
		     &current->sector_type,
		     &current->river->speed,
		     &current->river->direction);
      if (index < 5) {
	trunc_string(tmp_str);
        log("Room %d: River room [#%d] is missing parameters,\nExpected format \"%%d %%d 7 %%d %%d\", Got this:\n%s\n",
            number_rooms, current->room_id, tmp_str);
	exit(ERR_RIVER);
      }
    }

    if (current->room_flag & 1024) {
      for (index = 0; index < 2; index++)
	get_desc(&current->sound_ptr[index]);
    }

    done = FALSE;
    while (!done) {
      if (fgets(tmp_str, BUFFER_SIZE-1, fp) == NULL) {
        log("Room %d: Unexpected EOF reading exits for room [#%d].",
            number_rooms, current->room_id);
        exit(ERR_EOF);
      }
      switch (tmp_str[0]) {
      case 'D':
	index2 = sscanf(tmp_str, "D%d", &index);
	if (index2 < 1 || index < 0 || index > 5) {
	  trunc_string(tmp_str);
          log("Room %d: Expected direction in room [#%d] of the form \"D[0-5]\", Got this instead:\n%s\n",
              number_rooms, current->room_id, tmp_str);
	  exit(ERR_BADEXIT);
	}

	get_desc(&current->exit[index].desc_ptr);

	if (fgets(tmp_str, BUFFER_SIZE-1, fp) == NULL) {
          log("Room %d: Unexpected EOF reading exit keywords for room [#%d].",
              number_rooms, current->room_id);
          exit(ERR_EOF);
	}
	trunc_string(tmp_str);
	if (tmp_str[strlen(tmp_str) - 1] != '~') {
          log("Room %d: Bad exit keywords in room [#%d]. Expected format \"%%s~\", Got:\n%s\n",
              number_rooms, current->room_id, tmp_str);
	  exit(ERR_EXITKEY);
	}
	tmp_str[strlen(tmp_str) - 1] = '\0';
        current->exit[index].keywords = (char *)strdup(tmp_str);

	if (fgets(tmp_str, BUFFER_SIZE-1, fp) == NULL) {
          log("Room %d: Unexpected EOF reading exit data for room [#%d].",
              number_rooms, current->room_id);
          exit(ERR_EOF);
	}
	index2 = sscanf(tmp_str, "%d %d %d", &current->exit[index].door_flag,
			&current->exit[index].key_number,
			&current->exit[index].to_room);
	if (index2 < 3) {
	  trunc_string(tmp_str);
          log("Room %d: Exit door flags, key and target expected in [#%d] as \"%%d %%d %%d\", Got:\n%s\n",
              number_rooms, current->room_id, tmp_str);
	  exit(ERR_DOORFLAGS);
	}
	if (current->exit[index].door_flag < 0 ||
            current->exit[index].door_flag > 4) {
	  trunc_string(tmp_str);
          log("Room %d: Door flags invalid in room [#%d], Must be [0-4], Got:\n%s\n",
              number_rooms, current->room_id, tmp_str);
	  exit(ERR_DOORFLAGS);
	}
	break;

      case 'E':
	if (fgets(tmp_str, BUFFER_SIZE-1, fp) == NULL) {
          log("Room %d: Unexpected EOF reading extra keywords for room [#%d].",
              number_rooms, current->room_id);
          exit(ERR_EOF);
	}
	trunc_string(tmp_str);
	if (tmp_str[0] == '~') {
          log("Room %d: Empty extra description keyword list in room [#%d].\n",
              number_rooms, current->room_id);
	  exit(ERR_EXTRAKEY);
	}
	if (tmp_str[strlen(tmp_str) - 1] != '~') {
          log("Room %d: Invalid extra desc keyword list in room [#%d],\nExpected format \"%s~\", Got:\n%s\n",
              number_rooms, current->room_id, tmp_str);
	  exit(ERR_EXTRAKEY);
	}
	index2 = current->number_extras;
        if(index2) current->extra= (Extra_t *)realloc(current->extra,
                   (index2+1) * sizeof(struct Extra_st));
        else current->extra= (Extra_t *)calloc(1, sizeof(struct Extra_st));
        current->extra[index2].keywords = (char *)strdup(strtok(tmp_str, "~\n\r\0"));
	get_desc(&current->extra[index2].desc_ptr);
	current->number_extras++;
	break;

      case 'S':
	done = TRUE;
	break;

      default:
	trunc_string(tmp_str);
        log("Room %d: Unknown auxilliary code in room [#%d]:\n%s\n",
            number_rooms, current->room_id, tmp_str);
	exit(ERR_BADAUX);
	break;
      }
    }
  }
  current = bottom_of_world;
  printf("\rTotal Rooms loaded: %d                              \n", number_rooms);
  fclose(fp);
}

void Link_World(void)
{
  Room *ptr;
  int index2;
  int init_flag = TRUE;
  FILE *errfile = NULL;
  int counter;

  printf("Linking Rooms...\r");
  fflush(stdout);
  /*  For each valid direction in each room put in the real links.  */

  ptr = bottom_of_world;
  counter= 0;
  while (ptr != NULL) {
    if(!(++counter%100)) {
      printf("Linking Rooms...[#%d]\r", ptr->room_id);
      fflush(stdout);
    }
    for (index2 = 0; index2 < 6; index2++)
      if (ptr->exit[index2].door_flag > -1) {
	ptr->exit[index2].real_to_room =
	  find_room(ptr, ptr->exit[index2].to_room);
	if (ptr->exit[index2].real_to_room == NULL && ptr->exit[index2].to_room != -1) {
	  if (init_flag) {
	    if (!(errfile = fopen(errname, "w"))) {
              log("Cannot open %s as error file!", errname);
	      exit(ERR_ERRLOG);
	    }
	    init_flag = FALSE;
	  }
	  fprintf(errfile,
		  "Room [#%d] -- Unable to resolve %s exit to room [#%d]\n",
		  ptr->room_id, directions[index2].name,
		  ptr->exit[index2].to_room);
	}
      }
    ptr = ptr->next;
  }
  if (!init_flag) {
    printf("Invalid exit links listed in %s logfile.\n", errname);
    fclose(errfile);
  }
  printf("\rAll Rooms Linked.                                  \n");
}

void Main_Loop(void)
{
  int index, not_done = TRUE;

  Display_A_Room(current);
  do {
    index = current->sector_type;
    if (index == -1)
      index = 10;

    printf("( Zone: %d Room: %d Terrain: %-s ) ",
	   current->zonenum,
	   current->room_id,
	   sector_types[index].name);

    get_inputs();
    printf("\n");

    switch (index = find_keyword(inputs[0])) {
    case NORTH:
    case EAST:
    case SOUTH:
    case WEST:
    case UP:
    case DOWN:
      index--;
      if (current->exit[index].real_to_room == NULL)
	printf("\n>> There is no exit in that direction.\n\n");
      else {
	current = current->exit[index].real_to_room;
	Display_A_Room(current);
      }
      break;
    case BRIEF:
      Command_brief();
      break;
    case UNLINK:
      Command_unlink();
      break;
    case CHANGE:
      Command_change();
      break;
    case COPY:
      Command_copy();
      break;
    case MAKE:
      Command_make();
      break;
    case DELETE:
      Command_delete();
      break;
    case DESC:
      Command_desc();
      break;
    case EXITS:
      Command_exits();
      break;
    case EXTRA:
      Command_extra();
      break;
    case FLAG:
      Command_flag();
      break;
    case FORMAT:
      Command_format();
      break;
    case GOTO:
      Command_goto();
      break;
    case HELP:
      Command_help();
      break;
    case LIGHT:
      Command_light();
      break;
    case LIST:
      Command_list();
      break;
    case LINK:
      Command_link();
      break;
    case LOOK:
      Command_look();
      break;
    case SAVE:
      Command_save();
      break;
    case TITLE:
      Command_title();
      break;
    case SECTOR:
      Command_sector();
      break;
    case QUIT:
      not_done = FALSE;
      break;
    case ZONE:
      Command_zone();
      break;
    case DIG:
      Command_dig();
      break;
    default:
      printf("** Ummmm.... huh huh huh, you said %s!\n\n", inputs[0]);
      break;
    }
  } while (not_done);
}

void Command_unlink(void)
{
  int index, old;

  index = direction_number(inputs[1]);

  if (index == -1) {
    printf("UNLINK requires subparameters, the correct form is:\n");
    printf("--------------------------------------------------\n");
    printf("UNLINK <direction>\n\n");
    return;
  }
  if (current->exit[index].to_room == -1) {
    printf("** There is currently no exit in that direction.\n\n");
    return;
  }

  if(current->exit[index].desc_ptr)
    free(current->exit[index].desc_ptr);
  current->exit[index].desc_ptr= NULL;
  if(current->exit[index].keywords)
    free(current->exit[index].keywords);
  current->exit[index].keywords= NULL;

  current->exit[index].door_flag = -1;
  current->exit[index].key_number= -1;
  old= current->exit[index].to_room;
  current->exit[index].to_room= -1;
  current->exit[index].real_to_room = NULL;

  printf(">> %s link to [#%d] broken.\n\n",
	 directions[index].name, old);
}

void Command_brief(void)
{
  if (Brief) {
    printf(">> Brief descriptions turned off.\n\n");
    Brief = FALSE;
  } else {
    printf(">> Brief descriptions turned on.\n\n");
    Brief = TRUE;
  }
}

void Command_change(void)
{
  char tmp_str[PAGE_SIZE];
  char *ptr;

  if (inputs[1][0] == '\0' || inputs[2][0] == '\0') {
    printf("CHANGE requires subparameters, the correct form is:\n");
    printf("--------------------------------------------------\n");
    printf("CHANGE <old> <new>\n\n");
    printf("    old - the string to change.\n");
    printf("    new - the string to change to.\n\n");
    return;
  }

  ptr = strstr(current->desc_ptr, inputs[1]);
  if (ptr == NULL) {
    printf("** String <%s> was not found in the description.\n\n", inputs[1]);
    return;
  }
  *ptr = 0;
  strcpy(tmp_str, current->desc_ptr);
  strcat(tmp_str, inputs[2]);
  ptr += strlen(inputs[1]);
  strcat(tmp_str, ptr);

  free(current->desc_ptr);
  current->desc_ptr = (char *)strdup(tmp_str);

  printf(">> String has been changed.\n\n");

  Display_A_Room(current);
}

void Command_copy(void)
{
  Room *ptr, *ptr2;
  int count = 0;
  int source, startroom, endroom;

  if (inputs[1][0] == '\0' ||
      (inputs[2][0] != '\0' && inputs[3][0] == '\0')) {
    printf("COPY requires subparameters, the correct form is:\n");
    printf("------------------------------------------------\n");
    printf("COPY <room id> <start room> <end room>\n\n");
    printf("    <room id>    -  The room id from which to copy the title,\n");
    printf("                    desc, zonenum, room flag and type.\n");
    printf("    <start room>\n");
    printf("    <end room>   - (optional fields)  if specified they\n");
    printf("                    designate the rooms which will have their\n");
    printf("                    title, desc, zonenum, room flag and type\n");
    printf("                    changed.\n\n");
    return;
  }
  if(!scan_a_number(inputs[1], &source)) {
    printf("** Invalid source room %s specified.\n", inputs[1]);
    return;
  }
  if ((ptr = find_room(current, source)) == NULL) {
    printf("** Room [#%d] does not exist. Title and description were not copied to current room.\n\n", source);
    return;
  }

  if (inputs[2][0] == '\0') {
    Command_copy_sub(current, ptr);
  } else {
    if(!scan_a_number(inputs[2], &startroom)) {
      printf("** Invalid start room %s specified.\n", inputs[2]);
      return;
    }
    if(!scan_a_number(inputs[3], &endroom)) {
      printf("** Invalid end room %s specified.\n", inputs[3]);
      return;
    }
    ptr2 = bottom_of_world;
    while (ptr2 != NULL) {
      if ((ptr2->room_id >= startroom) && (ptr2->room_id <= endroom)) {
	Command_copy_sub(ptr2, ptr);
	count++;
      }
      ptr2 = ptr2->next;
    }
    if (count == 0)
      printf("** No Rooms were found between %d and %d.\n",
	     startroom, endroom);
  }

  Display_A_Room(current);
}

void Command_copy_sub(Room *ptr, Room *from_ptr)
{
  int index;

  if(ptr->title)
    free(ptr->title);
  ptr->title= (char *)strdup(from_ptr->title);

  if(ptr->desc_ptr)
    free(ptr->desc_ptr);
  ptr->desc_ptr = (char *)strdup(from_ptr->desc_ptr);

  ptr->zonenum= from_ptr->zonenum;
  ptr->room_flag = from_ptr->room_flag;
  ptr->sector_type = from_ptr->sector_type;
  if(ptr->teleport)
    free(ptr->teleport);
  if(from_ptr->teleport) {
    ptr->teleport= (Teleport_t *)calloc(1, sizeof(struct Teleport_st));
    *(ptr->teleport)= *(from_ptr->teleport);
  }
  if(ptr->river)
    free(ptr->river);
  if(from_ptr->river) {
    ptr->river= (River_t *)calloc(1, sizeof(struct River_st));
    *(ptr->river)= *(from_ptr->river);
  }

  for (index = 0; index < 2; index++) {
    if (from_ptr->sound_ptr[index] != NULL) {
      if(ptr->sound_ptr[index])
        free(ptr->sound_ptr[index]);
      ptr->sound_ptr[index] = (char *)strdup(from_ptr->sound_ptr[index]);
    } else {
      ptr->sound_ptr[index] = NULL;
    }
  }
  printf(">> Title and description for room [#%d] has been changed.\n",
	 ptr->room_id);
}

void Command_make(void)
{
  int index, roomid, source;
  char tmp_str[BUFFER_SIZE];
  Room *ptr = NULL;

  if (inputs[1][0] == '\0') {
    printf("MAKE requires subparameters, the correct form is:\n");
    printf("------------------------------------------------\n");
    printf("MAKE <room id> <from room id>\n\n");
    printf("  <room id>       - the room id of the new room.\n");
    printf("  <from room id>  - (optional) room from which the\n");
    printf("                    title, description, zone, types\n");
    printf("                    and flags should be copied.\n\n");
    return;
  }
  if(!scan_a_number(inputs[1], &roomid)) {
    printf("** Invalid room %s specified.\n", inputs[1]);
    return;
  }
  if (find_room(current, roomid) != NULL) {
    printf("** Room id %d already exists. Room not created.\n\n",
	   roomid);
    return;
  }
  if (inputs[2][0] != '\0') {
    if(!scan_a_number(inputs[2], &source)) {
      printf("** Invalid source room %s specified.\n", inputs[2]);
      return;
    }
    if ((ptr = find_room(current, source)) == NULL) {
      printf("** Room id %d does not exist. New Room not created.\n\n",
	     source);
      return;
    }
  }

  current = allocate_room(roomid);

  if (inputs[2][0] == '\0') {
    Command_title();
    Command_desc();
    printf("Zone for this room\n------------------\n> ");
    /* gets(tmp_str); */
    fgets(tmp_str, BUFFER_SIZE-1, stdin);
    if((strlen(tmp_str) > 0) && tmp_str[strlen(tmp_str)-1] == '\n')
      tmp_str[strlen(tmp_str)-1]= '\0';
    if(!scan_a_number(tmp_str, &current->zonenum)) {
      printf("** Invalid zone entered, defaulting to zone %d.\n", lastzone);
      current->zonenum= lastzone;
    } else lastzone= current->zonenum;
  } else {
    current->title= (char *)strdup(ptr->title);
    current->desc_ptr= (char *)strdup(ptr->desc_ptr);
    current->zonenum= ptr->zonenum;
    current->room_flag = ptr->room_flag;
    current->sector_type = ptr->sector_type;
    if(ptr->teleport) {
      current->teleport= (Teleport_t *)calloc(1, sizeof(struct Teleport_st));
      *(current->teleport)= *(ptr->teleport);
    }
    if(ptr->river) {
      current->river= (River_t *)calloc(1, sizeof(struct River_st));
      *(current->river)= *(ptr->river);
    }
    for (index = 0; index < 2; index++)
      if (ptr->sound_ptr[index] != NULL) {
	current->sound_ptr[index] = (char *)strdup(ptr->sound_ptr[index]);
      } else
	current->sound_ptr[index] = NULL;
  }
  printf(">> New Room Created.\n\n");
  Display_A_Room(current);
}

void Command_delete(void)
{
  Room *ptr, *ptr2;
  int index, zaproom;

  if (inputs[1][0] == '\0') {
    printf("DELETE requires a subparameter, the correct form is:\n");
    printf("---------------------------------------------------\n");
    printf("DELETE <room id>\n\n");
    printf("    <room id>       - the room id of the room to delete.\n\n");
    return;
  }
  if(!scan_a_number(inputs[1], &zaproom)) {
    printf("** Invalid source room %s specified.\n", inputs[1]);
    return;
  }
  if ((ptr = find_room(current, zaproom)) == NULL) {
    printf("** Room id %d does not exist. Room not deleted.\n\n",
	   zaproom);
    return;
  }

  ptr2 = bottom_of_world;
  while (ptr2 != NULL) {
    if(ptr2->teleport) {
      if (ptr2->teleport->to_room == ptr->room_id) {
        ptr2->sector_type = 0;
        ptr2->teleport->time = 0;
        ptr2->teleport->to_room= -1;
        ptr2->teleport->do_look = 0;
        ptr2->teleport->sector_type = 0;
        printf(">> Teleportal from room [#%d] to room [#%d] deleted.\n",
               ptr2->room_id, ptr->room_id);
      }
    }
    for (index = 0; index < 6; index++)
      if (ptr2->exit[index].real_to_room == ptr) {
	free(ptr2->exit[index].desc_ptr);
	ptr2->exit[index].desc_ptr = NULL;
	free(ptr2->exit[index].keywords);
	ptr2->exit[index].keywords= NULL;
	ptr2->exit[index].door_flag = -1;
	ptr2->exit[index].key_number= -1;
	ptr2->exit[index].to_room= -1;
	ptr2->exit[index].real_to_room = NULL;
	printf(">> Exit %-6s from room [#%d] to room [#%d] deleted.\n",
	       directions[index].name, ptr2->room_id,
	       ptr->room_id);
      }
    ptr2 = ptr2->next;
  }
  if (ptr == bottom_of_world) {
    if (ptr->prev != NULL)
      bottom_of_world = ptr->prev;
    else
      bottom_of_world = ptr->next;
  }
  if (ptr == top_of_world) {
    if (ptr->next != NULL)
      top_of_world = ptr->next;
    else
      top_of_world = ptr->prev;
  }
  if (ptr->prev != NULL)
    ptr->prev->next = ptr->next;
  if (ptr->next != NULL)
    ptr->next->prev = ptr->prev;

  /*  If the current room was deleted, move to the bottom of
   * the world.
   */

  if (ptr == current)
    current = bottom_of_world;

  /*  Now free the room.  */

  if(ptr) {
    if(ptr->title)
      free(ptr->title);
    if(ptr->desc_ptr)
      free(ptr->desc_ptr);
    for (index= 0; index < 2; index++) {
      if(ptr->sound_ptr[index])
        free(ptr->sound_ptr[index]);
    }
    if(ptr->teleport)
      free(ptr->teleport);
    if(ptr->river)
      free(ptr->river);
    for (index = 0; index < 6; index++) {
      if(ptr->exit[index].desc_ptr)
        free(ptr->exit[index].desc_ptr);
      if(ptr->exit[index].keywords)
        free(ptr->exit[index].keywords);
    }
    for (index = 0; index < ptr->number_extras; index++) {
      if(ptr->extra[index].desc_ptr)
        free(ptr->extra[index].desc_ptr);
      if(ptr->extra[index].keywords)
        free(ptr->extra[index].keywords);
    }
    free(ptr->extra);
    free(ptr);
  }

  printf(">> Room [#%s] has been deleted.\n\n", inputs[1]);
  Display_A_Room(current);
}

void Command_desc(void)
{
  FILE *fp;

  if (current->desc_ptr) {
    sprintf(tempfile, TMP_FILE, getpid());
    if(!(fp= fopen(tempfile, "w"))) {
      log("Cannot open %s for output!", tempfile);
      exit(ERR_NOOUTPUT);
    }
    fprintf(fp, "%s~\n", current->desc_ptr);
    fclose(fp);
    free(current->desc_ptr);
  }

  /* input_desc("new description", &current->desc_ptr); */
  import_desc("new description", &current->desc_ptr);

  printf("Description set to:\n-------------------\n");
  printf("%s", current->desc_ptr);
  printf("\n");
}

void Command_exits(void)
{
  int index, index2;
  int no_exits = TRUE;

  for (index = 0; index < 6; index++) {
    index2 = current->exit[index].door_flag;

    if (current->exit[index].real_to_room != NULL) {
      printf("%-6s - %-s to Room %d (%-s)\n",
	     directions[index].name,
	     door_states[index2].name,
	     current->exit[index].real_to_room->room_id,
	     current->exit[index].real_to_room->title);
      no_exits = FALSE;
    }
  }
  if (no_exits)
    printf("There are no apparent exits from this room.\n");
  printf("\n");
}

void Command_extra(void)
{
  int index, index2;
  char tmp_str[BUFFER_SIZE];

  if (find_keyword(inputs[1]) == DELETE) {
    if (current->number_extras == 0) {
      printf("**No extra descriptions to delete!\n\n");
      return;
    }
    printf("Select the extra description to delete:\n");
    printf("--------------------------------------\n");

    for (index = 0; index < current->number_extras; index++)
      printf("%d. %s\n", index + 1, current->extra[index].keywords);
    printf("\nEnter selection > ");
    /* gets(tmp_str); */
    fgets(tmp_str, BUFFER_SIZE-1, stdin);
    if((strlen(tmp_str) > 0) && tmp_str[strlen(tmp_str)-1] == '\n')
      tmp_str[strlen(tmp_str)-1]= '\0';
    if(!scan_a_number(tmp_str, &index) || --index < 0 || index >= current->number_extras) {
      printf("\n** Invalid selection.  No deletion performed.\n\n");
      return;
    }

    index2 = index;
    while (index2 < current->number_extras - 1) {
      current->extra[index2].keywords= current->extra[index2 + 1].keywords;
      current->extra[index2].desc_ptr= current->extra[index2 + 1].desc_ptr;
      index2++;
    }
    current->number_extras--;
    if(current->extra[index2].keywords)
      free(current->extra[index2].keywords);
    current->extra[index2].keywords= NULL;
    if(current->extra[index2].desc_ptr)
      free(current->extra[index2].desc_ptr);
    current->extra[index2].desc_ptr = NULL;
    current->extra= (Extra_t *)realloc(current->extra,
                   (current->number_extras) * sizeof(struct Extra_st));
    if(!current->number_extras)
      current->extra= NULL;
  } else {
    index = current->number_extras;
    printf("Enter the extra description keywords seperated by BLANKS.\n> ");
    /* gets(tmp_str); */
    fgets(tmp_str, BUFFER_SIZE-1, stdin);
    if((strlen(tmp_str) > 0) && tmp_str[strlen(tmp_str)-1] == '\n')
      tmp_str[strlen(tmp_str)-1]= '\0';
    trunc_string(tmp_str);

    if (tmp_str[0] == '\0') {
      printf("\n**No keywords entered.  Extra description not created.\n\n");
      return;
    }
    if(index) current->extra= (Extra_t *)realloc(current->extra,
              (index+1) * sizeof(struct Extra_st));
    else current->extra= (Extra_t *)calloc(1, sizeof(struct Extra_st));
    if(current->extra[index].keywords)
      free(current->extra[index].keywords);
    current->extra[index].keywords= (char *)strdup(tmp_str);
    printf("\n");
    if (current->extra[index].desc_ptr)
      free(current->extra[index].desc_ptr);
    /* input_desc("extra description", &current->extra[index].desc_ptr); */
    import_desc("extra description", &current->extra[index].desc_ptr);
    current->number_extras++;
  }
  Display_A_Room(current);
}

void Command_flag(void)
{
  int index = 0, index2 = 0, count = 0, startroom, endroom;
  Room *ptr;

  while (room_flags[index].name != NULL &&
	 strcasecmp(room_flags[index].name, inputs[1]) != 0)
    index++;

  if (inputs[1][0] == '\0' ||
      room_flags[index].name[0] == '\0' ||
      (inputs[2][0] != '\0' && inputs[3][0] == '\0')) {
    printf("FLAG command requires sub-parameters, the correct form is:\n");
    printf("---------------------------------------------------------\n");
    printf("FLAG <new flag>  <start room> <end room>\n\n");
    printf("    <new flag>   -  flag to turn on/off for room(s).\n");
    printf("    <room id>    -  The room id from which to copy the title,\n");
    printf("                    desc, zonenum, room flag and type.\n");
    printf("    <start room>\n");
    printf("    <end room>   - (optional fields)  if specified they\n");
    printf("                    designate the rooms which will have their\n");
    printf("                    title, desc, zonenum, room flag and type\n");
    printf("                    changed.\n\n");
    printf("Valid FLAG parameters are:\n");
    printf("---------------------------");
    while (room_flags[index2].name != NULL) {
      if (index2 % 5 == 0)
	printf("\n");
      printf("%-10s ", room_flags[index2++].name);
    }
    printf("\n\n");
    return;
  }
  if (inputs[2][0] == '\0') {
    Command_flag_sub(current, index);
  } else {
    if(!scan_a_number(inputs[2], &startroom)) {
      printf("** Invalid start room %s specified.\n", inputs[2]);
      return;
    }
    if(!scan_a_number(inputs[3], &endroom)) {
      printf("** Invalid end room %s specified.\n", inputs[3]);
      return;
    }
    ptr = bottom_of_world;
    while (ptr != NULL) {
      if ((ptr->room_id >= startroom) && (ptr->room_id <= endroom)) {
	Command_flag_sub(ptr, index);
	count++;
      }
      ptr = ptr->next;
      if (count == 0)
	printf("** No Rooms were found between %d and %d.\n",
	       startroom, endroom);
    }
  }
  Display_A_Room(current);
}

void Command_flag_sub(Room *ptr, int index)
{
  int index2 = 0;

  if (ptr->room_flag & room_flags[index].mask) {
    ptr->room_flag = ptr->room_flag &
      ~room_flags[index].mask;

    if (index == 10)
      for (index2 = 0; index2 < 2; index2++)
        if(ptr->sound_ptr[index2])
	  free(ptr->sound_ptr[index2]);

    printf(">> Flag %s turned off for room [#%d].\n\n",
	   inputs[1], ptr->room_id);
  } else {
    ptr->room_flag = ptr->room_flag |
      room_flags[index].mask;

    if (index == 10) {
      input_desc("sound #1", &ptr->sound_ptr[0]);
      input_desc("sound #2", &ptr->sound_ptr[1]);
    }
    printf(">> Flag %s turned on for room [#%d].\n\n",
	   inputs[1], ptr->room_id);
  }
}

void Command_format(void)
{
  int index = 0, index2 = 0, index3 = 0, index4 = 0;
  char *ptr;
  char tmp_str[BUFFER_SIZE];

  if (strcasecmp(inputs[1], "desc") == 0)
    index = 1;
  else if (strcasecmp(inputs[1], "extra") == 0)
    index = 2;

  if (index == 0) {
    printf("FORMAT command requires a sub-parameter, the correct form is:\n");
    printf("-------------------------------------------------------------\n");
    printf("FLAG <option>\n\n");
    printf("    <option>   -  Either desc or extra.  This command will\n");
    printf("                  reformat either the description or an\n");
    printf("                  extra description into strings of 78 chars.\n\n");
    return;
  }

  if (index == 1)
    ptr = current->desc_ptr;
  else {
    if (current->number_extras == 0) {
      printf("** No extra descriptions found for the current room.\n\n");
      return;
    }
    printf("Select the extra description to format :\n");
    printf("----------------------------------------\n");

    for (index = 0; index < current->number_extras; index++)
      printf("%d. %s\n", index + 1, current->extra[index].keywords);

    printf("\nEnter selection > ");
    /* gets(tmp_str); */
    fgets(tmp_str, BUFFER_SIZE-1, stdin);
    if((strlen(tmp_str) > 0) && tmp_str[strlen(tmp_str)-1] == '\n')
      tmp_str[strlen(tmp_str)-1]= '\0';
    trunc_string(tmp_str);
    index = atoi(tmp_str) - 1;
    if (index < 0 || index >= current->number_extras) {
      printf("\n** Invalid selection.  No formating performed.\n\n");
      return;
    }
    ptr = current->extra[index].desc_ptr;
  }

  /*  First get rid of all current new lines.  */
  index = -1;
  while (++index <= strlen(ptr))
    if (ptr[index] == '\n')
      ptr[index] = ' ';

  /*  Now insert the new newlines.  */

  index2 = 78;
  while (index2 < strlen(ptr)) {
    /*  Starting at the current lines end position, go backwards
     * until we find a blank char.  This means we are at the
     * beginning of a word.
     */

    index3 = index2;
    while (index3 > 1 && ptr[index3] != ' ')
      index3--;
    ptr[index3] = '\n';

    /*  Now go forwards past all the blanks so we start the new
     * "next line" with a non-blank char.
     */

    index3++;
    index4 = index3;
    index2 = index3 + 77;
    while (index3 <= strlen(ptr) && ptr[index3] == ' ')
      index3++;

    /*  Now compress everything down to eliminate the blanks
     * that we skipped over when finding the start of the
     * "next line".
     */

    while (index3 <= strlen(ptr))
      ptr[index4++] = ptr[index3++];
    ptr[index3] = 0;
  }
  ptr[strlen(ptr) - 1] = '\n';

  Display_A_Room(current);
}

void Command_goto(void)
{
  int number;
  Room *ptr;

  if (inputs[1][0] == '\0') {
    printf("** Go to what room?\n\n");
    return;
  }
  if(!scan_a_number(inputs[1], &number)) {
    printf("** Invalid room %s specified.\n", inputs[1]);
    return;
  }
  ptr = find_room(current, number);

  if (ptr == NULL) {
    printf("** Room %d does not exist!\n\n", number);
    return;
  } else {
    current = ptr;
    Display_A_Room(ptr);
  }
}

void Command_help(void)
{
  int index = 0;
  int index2 = 0;
  int prev = -1;

  while (commands[index].name != NULL) {
    if (commands[index].num != prev) {
      printf("%-10s ", commands[index].name);
      prev = commands[index].num;
      index2++;
    }
    if (index2 % 6 == 0)
      printf("\n");
    index++;
  }
  printf("\n\n");
}

void Command_light(void)
{
  if (Light) {
    printf(">> You turn off your lantern.\n\n");
    Light = FALSE;
  } else {
    printf(">> You turn on your lantern.\n\n");
    Light = TRUE;
  }
}

void Command_link(void)
{
  int index, index3, index4, roomid;
  char tmp_str[BUFFER_SIZE];
  Room *ptr;

  index = direction_number(inputs[1]);

  if (index == -1 || inputs[2][0] == '\0') {
    printf("LINK requires subparameters, the correct form is:\n");
    printf("------------------------------------------------\n");
    printf("LINK <direction> <room id>\n\n");
    return;
  }

  if(!scan_a_number(inputs[2], &roomid)) {
    printf("** Invalid room %s specified.\n", inputs[2]);
    return;
  }
  ptr = find_room(current, roomid);
  if (ptr == NULL) {
    printf("** Room %d does not exist. Link not created.\n\n",
	   roomid);
    return;
  }

  if (current->exit[index].to_room != -1) {
    printf("** Exit already exists to the %s. Please UNLINK exit first.\n\n",
	   directions[index].name);
    return;
  }
  current->exit[index].to_room= roomid;
  current->exit[index].real_to_room = ptr;

  do {
    printf("Choose an exit type\n-------------------\n");
    for (index3 = 0; index3 < 5; index3++)
      printf("%d. %-s\n", index3 + 1, door_states[index3].name);

    printf("> ");
    /* gets(tmp_str); */
    fgets(tmp_str, BUFFER_SIZE-1, stdin);
    if((strlen(tmp_str) > 0) && tmp_str[strlen(tmp_str)-1] == '\n')
      tmp_str[strlen(tmp_str)-1]= '\0';
    if(!scan_a_number(tmp_str, &index3)) {
      printf("** Invalid choice... try again.\n");
    }
  } while (index3 < 1 || index3 > 5);

  current->exit[index].door_flag = --index3;
  printf("\n");

  /*  if this is a lockable door, ask for the object number to open it.  */

  if (index3 > 0) {
    do {
      printf("Enter object number which can unlock/lock this door\n");
      printf("a -1 indicates that there is no key for this door.\n");
      printf("---------------------------------------------------\n");
      printf("> ");
      /* gets(tmp_str); */
      fgets(tmp_str, BUFFER_SIZE-1, stdin);
      if((strlen(tmp_str) > 0) && tmp_str[strlen(tmp_str)-1] == '\n')
        tmp_str[strlen(tmp_str)-1]= '\0';
      if(!scan_a_number(tmp_str, &current->exit[index].key_number)) {
        printf("** Invalid choice... try again.\n");
      }
    } while (tmp_str[0] == '\0');
    printf("\n");
  }
  /*  Get any keywords for that direction.  */

  do {
    index4 = FALSE;
    printf("Enter all exit keywords seperated by BLANKS for this exit\n");
    printf("or return if there are none.\n");
    printf("----------------------------------------------------------\n> ");
    /* gets(tmp_str); */
    fgets(tmp_str, BUFFER_SIZE-1, stdin);
    if((strlen(tmp_str) > 0) && tmp_str[strlen(tmp_str)-1] == '\n')
      tmp_str[strlen(tmp_str)-1]= '\0';
    trunc_string(tmp_str);

    if (index3 > 0 && tmp_str[0] == '\0') {
      printf("\n** At least one keyword must be specified for exits with a door.\n\n");
      index4 = TRUE;
    }
  } while (index4);
  if(current->exit[index].keywords)
    free(current->exit[index].keywords);
  current->exit[index].keywords= (char *)strdup(tmp_str);
  printf("\n");

  /*  Get the exit description if any.  */

  free(current->exit[index].desc_ptr);
  input_desc("exit description", &current->exit[index].desc_ptr);
  printf("\n");
}

void Command_list(void)
{
  Room *ptr;
  int count = 0, startroom, endroom;
  char tmp_str[BUFFER_SIZE];

  if (inputs[1][0] == 0)
    startroom= current->room_id;
  else
    if(!scan_a_number(inputs[1], &startroom)) {
      printf("** Invalid starting room %s.\n", inputs[1]);
    }

  if (inputs[2][0] == 0) {
    ptr = find_room(current, startroom);
    if (ptr != NULL)
      printf("Room %d (in Zone %d) - %s\n", ptr->room_id,
	     ptr->zonenum, ptr->title);
    else
      printf("** Room %d does not exist!\n", startroom);
  } else {
    if(!scan_a_number(inputs[2], &endroom)) {
      printf("** Invalid end room %s.\n", inputs[2]);
    }
    ptr = bottom_of_world;
    while (ptr != NULL) {
      if(ptr->room_id >= startroom && ptr->room_id <= endroom) {
	printf("Room %d (in Zone %d) - %s\n", ptr->room_id,
	       ptr->zonenum, ptr->title);
	count++;
	if (count % 22 == 0) {
	  printf("MORE? > ");
          /* gets(tmp_str); */
          fgets(tmp_str, BUFFER_SIZE-1, stdin);
          if((strlen(tmp_str) > 0) && tmp_str[strlen(tmp_str)-1] == '\n')
            tmp_str[strlen(tmp_str)-1]= '\0';
	  if (toupper(tmp_str[0]) == 'N')
	    break;
	}
      }
      ptr = ptr->next;
    }
    if (count == 0)
      printf("** No Rooms were found between %d and %d.\n",
	     startroom, endroom);
  }
  printf("\n");
}

void Command_look(void)
{
  int index = 0;

  if (inputs[1][0] == '\0')
    Display_A_Room(current);
  else {
    index = direction_number(inputs[1]);
    if (index != -1) {
      if (current->exit[index].desc_ptr != NULL &&
	  *current->exit[index].desc_ptr != '\0')
	printf("%s", current->exit[index].desc_ptr);
      else
	printf("You see nothing in particular in that direction.\n");
      printf("\n");
    } else {
      index = 0;
      while (index < current->number_extras &&
	     strstr(current->extra[index].keywords,
		    inputs[1]) == 0)
	index++;
      if (index < current->number_extras &&
	  current->extra[index].desc_ptr != NULL)
	printf("%s", current->extra[index].desc_ptr);
      else
	printf("You see of interest about the %s.\n", inputs[1]);
      printf("\n");
    }
  }
}

void Command_save(void)
{
  int index2;
  char tmp_str[BUFFER_SIZE];
  Room *ptr;
  int counter;

  printf("Filename to save to? (default is %s)\n> ", DFLT_OUTPUT);
  /* gets(tmp_str); */
  fgets(tmp_str, BUFFER_SIZE-1, stdin);
  if((strlen(tmp_str) > 0) && tmp_str[strlen(tmp_str)-1] == '\n')
    tmp_str[strlen(tmp_str)-1]= '\0';
  trunc_string(tmp_str);

  if (!tmp_str[0])
    outname= (char *)strdup(DFLT_OUTPUT);
  else
    outname= (char *)strdup(tmp_str);

  if ((out = fopen(outname, "r"))) {
    printf("File already exists, overwrite (Y or N)? ");
    /* gets(tmp_str); */
    fgets(tmp_str, BUFFER_SIZE-1, stdin);
    if((strlen(tmp_str) > 0) && tmp_str[strlen(tmp_str)-1] == '\n')
      tmp_str[strlen(tmp_str)-1]= '\0';

    if (tmp_str[0] != 'Y' && tmp_str[0] != 'y') {
      printf("\n** World not saved...\n\n");
      return;
    }
    fclose(out);
  }

  if (!(out = fopen(outname, "w"))) {
    log("Cannot open %s for output!", outname);
    exit(ERR_NOOUTPUT);
  }

  counter= 0;
  printf("\nSaving world to %s....\r", outname);
  fflush(stdout);

  ptr = bottom_of_world;
  while (ptr != NULL) {
    if(!(++counter%100)) {
      printf("Saving world to %s...[#%d]\r", outname, ptr->room_id);
      fflush(stdout);
    }
    fprintf(out, "#%d\n", ptr->room_id);
    fprintf(out, "%s~\n", ptr->title);
    fprintf(out, "%s~\n", ptr->desc_ptr);
    fprintf(out, "%d %d %d", ptr->zonenum,
	    ptr->room_flag,
	    ptr->sector_type);
    if (ptr->sector_type == -1)
      fprintf(out, " %d %d %d %d",
	      ptr->teleport->time,
	      ptr->teleport->to_room,
	      ptr->teleport->do_look,
	      ptr->teleport->sector_type);
    else if (ptr->sector_type == 7)
      fprintf(out, " %d %d", ptr->river->speed,
	      ptr->river->direction);
    fprintf(out, "\n");
    if (ptr->room_flag & 1024)
      for (index2 = 0; index2 < 2; index2++)
	fprintf(out, "%s~\n", ptr->sound_ptr[index2]);
    for (index2 = 0; index2 < 6; index2++)
      if (ptr->exit[index2].door_flag != -1) {
	fprintf(out, "D%d\n", index2);
	fprintf(out, "%s~\n", ptr->exit[index2].desc_ptr);
	fprintf(out, "%s~\n", ptr->exit[index2].keywords);
	fprintf(out, "%d %d %d\n",
		ptr->exit[index2].door_flag,
		ptr->exit[index2].key_number,
		ptr->exit[index2].to_room);
      }
    for (index2 = 0; index2 < ptr->number_extras; index2++) {
      fprintf(out, "E\n%s~\n", ptr->extra[index2].keywords);
      fprintf(out, "%s~\n", ptr->extra[index2].desc_ptr);
    }
    fprintf(out, "S\n");
    ptr = ptr->next;
  }
  fprintf(out, "$\n");
  fclose(out);
  printf("Saving world to %s...Done!             \n", outname);
  printf(">> Saved %d rooms to %s.\n", counter, outname);
}

void Command_title(void)
{
  char tmp_str[BUFFER_SIZE];

  do {
    printf("Enter new title:\n----------------\n> ");
    /* gets(tmp_str); */
    fgets(tmp_str, BUFFER_SIZE-1, stdin);
    if((strlen(tmp_str) > 0) && tmp_str[strlen(tmp_str)-1] == '\n')
      tmp_str[strlen(tmp_str)-1]= '\0';
    trunc_string(tmp_str);

    if (tmp_str[0] == '\0')
      printf("** A non-blank string must be entered for the title.\n\n");

  } while (tmp_str[0] == '\0');

  if(current->title)
    free(current->title);
  current->title= (char *)strdup(tmp_str);

  printf("\nTitle set to: %s\n\n", tmp_str);
}

void Command_sector(void)
{
  int index = 0, index2 = 0, index3 = 0, parm2, parm3, parm4;

  while (sector_types[index].name != NULL &&
	 strcasecmp(sector_types[index].name, inputs[1]) != 0)
    index++;

  if (sector_types[index].name != NULL) {
    index2 = sector_types[index].num;

    if (index2 == -1) {
      if (inputs[2][0] == '\0' || inputs[3][0] == '\0' ||
	  inputs[4][0] == '\0' || inputs[5][0] == '\0') {
	printf("Sector TELEPORT requires sub-parameters, the correct form is:\n");
	printf("------------------------------------------------------------\n");
	printf("SECTOR TELEPORT <teleport time> <to room> <do look> <move type>\n\n");
	printf("<teleport time> - multiple of 10, higher number longer wait.\n");
	printf("<to room>       - room to teleport to.\n");
	printf("<do look>       - 0 = Do not look   1 = Look after teleport\n");
	printf("<move type>     - type of terrain to use for movement cost.\n\n");
	return;
      }
      if(!scan_a_number(inputs[2], &parm2) || (parm2 % 10 != 0)) {
	printf("** <teleport time> must be a multiple of 10, room type not set.\n\n");
	return;
      }
      if (!scan_a_number(inputs[3], &parm3) || find_room(current, parm3) == NULL) {
	printf("** <to room> does not exist, room type not set.\n\n");
	return;
      }
      if(!scan_a_number(inputs[4], &parm4) || parm4 < 0 || parm4 > 1) {
	printf("** <do look> must be 0 or 1, room type not set.\n\n");
	return;
      }
      while (sector_types[index3].name != NULL &&
	     strcasecmp(sector_types[index3].name, inputs[5]) != 0)
	index3++;

      if (sector_types[index3].name[0] == '\0' || sector_types[index3].num == -1) {
	printf("** <move type> must be a valid terrain type, room type not set.\n\n");
	return;
      }
    }
    if (index2 == 7) {
      if (inputs[2][0] == '\0' || inputs[3][0] == '\0') {
	printf("Sector RIVER requires sub-parameters, the correct form is:\n");
	printf("---------------------------------------------------------\n");
	printf("SECTOR RIVER <river speed> <river direction>\n\n");
	printf("     <river speed>     - time it takes to drift through an exit.\n");
	printf("     <river direction> - exit to drift through (s, n, e, w, etc..).\n\n");
	return;
      }
      index3 = direction_number(inputs[3]);

      if (index3 < 0 || index3 > 5) {
	printf("** <river direction> must be a valid direction, room type not set.\n\n");
	return;
      }
    }

    current->sector_type = sector_types[index].num;

    if (index2 == -1) {
      if(current->teleport)
        free(current->teleport);
      current->teleport= (Teleport_t *)calloc(1, sizeof(struct Teleport_st));
      current->teleport->time = parm2;
      current->teleport->to_room = parm3;
      current->teleport->do_look = parm4;
      current->teleport->sector_type = index3;
    } else if (index2 == 7) {
      if(current->river)
        free(current->river);
      current->river= (River_t *)calloc(1, sizeof(struct River_st));
      current->river->speed = atoi(inputs[2]);
      current->river->direction = index3;
    }
    printf("\n");
  } else {
    printf("Valid SECTOR parameters are:\n");
    printf("---------------------------\n");
    while (sector_types[index2].name != NULL) {
      printf("%-12s ", sector_types[index2++].name);
      if (index2 == 6)
	printf("\n");
    }
    printf("\n\n");
    return;
  }
}

void Command_zone(void)
{
  Room *ptr;
  int count = 0, zone, startroom, endroom;

  if (inputs[1][0] == '\0' ||
      (inputs[2][0] != '\0' && inputs[3][0] == '\0')) {
    printf("ZONE command requires sub-parameters, the correct form is:\n");
    printf("---------------------------------------------------------\n");
    printf("ZONE <new zone>  <start room> <end room>\n\n");
    printf("     <new zone>   - new zone for room(s).\n");
    printf("     <start room>\n");
    printf("     <end room>   - (optional fields)  if specified they\n");
    printf("                    designate the rooms which willhave their\n");
    printf("                    zones changed.\n\n");
    return;
  }
  if(!scan_a_number(inputs[1], &zone)) {
    printf("** Invalid zone specified.\n");
    return;
  }
  if (inputs[2][0] == '\0') {
    current->zonenum= zone;
    printf(">> Zone for room %d changed to zone %d.\n\n",
	   current->room_id, current->zonenum);
  } else {
    if(!scan_a_number(inputs[2], &startroom)) {
      printf("** Invalid starting room specified.\n");
      return;
    }
    if(!scan_a_number(inputs[3], &endroom)) {
      printf("** Invalid ending room specified.\n");
      return;
    }
    ptr = bottom_of_world;
    while (ptr != NULL) {
      if( ptr->room_id >= startroom && ptr->room_id <= endroom) {
	ptr->zonenum= zone;
	printf(">> Zone for room %d changed to zone %d.\n\n",
	       ptr->room_id, ptr->zonenum);
	count++;
      }
      ptr = ptr->next;
    }
    if (count == 0)
      printf("** No Rooms were found between %d and %d.\n",
             startroom, endroom);
  }
}

void Command_dig(void)
{
  int index, index3, index4, i, revindex, target, source, zone;
  char tmp_str[BUFFER_SIZE];
  Room *ptr, *ptr2, *tmpptr;

  index = direction_number(inputs[1]);

  if (index == -1 || inputs[2][0] == '\0') {
    printf("DIG requires subparameters, the correct form is:\n");
    printf("-----------------------------------------------\n");
    printf("DIG <direction> <room id> [room to copy]\n\n");
    return;
  }

  if (current->exit[index].to_room != -1) {
    printf("** Exit already exists to the %s. Please UNLINK exit first.\n\n",
	   directions[index].name);
    return;
  }

  if(!(scan_a_number(inputs[2], &target))) {
    printf("** Invalid room specified.\n");
    return;
  }
  ptr = find_room(current, target);
  if (ptr == NULL) {
    if (inputs[3][0] != '\0') {
      if(!(scan_a_number(inputs[3], &source))) {
        printf("** Invalid source room specified.\n");
        return;
      }
      if (!(ptr2 = find_room(current, source))) {
	printf("** Room %d does not exist. Room not created.\n\n",
	       source);
	return;
      } else {
	ptr = allocate_room(target);
	ptr->title= (char *)strdup(ptr2->title);
        ptr->desc_ptr = (char *)strdup(ptr2->desc_ptr);
	ptr->zonenum= ptr2->zonenum;
	ptr->room_flag = ptr2->room_flag;
	ptr->sector_type = ptr2->sector_type;
        if(ptr2->teleport) {
          ptr->teleport= (Teleport_t *)calloc(1, sizeof(struct Teleport_st));
          *(ptr->teleport)= *(ptr2->teleport);
        } else ptr->teleport= NULL;
        if(ptr2->river) {
          ptr->river= (River_t *)calloc(1, sizeof(struct River_st));
          *(ptr->river)= *(ptr2->river);
        } else ptr->river= NULL;
	for (i = 0; i < 2; i++)
	  if (ptr2->sound_ptr[i] != NULL) {
	    ptr->sound_ptr[i] = (char *)strdup(ptr2->sound_ptr[i]);
	  } else
	    ptr->sound_ptr[i] = NULL;
      }
    } else {
      tmpptr = current;
      current = allocate_room(target);
      Command_title();
      Command_desc();
      printf("Zone for this room\n------------------\n> ");
      /* gets(tmp_str); */
      fgets(tmp_str, BUFFER_SIZE-1, stdin);
      if((strlen(tmp_str) > 0) && tmp_str[strlen(tmp_str)-1] == '\n')
        tmp_str[strlen(tmp_str)-1]= '\0';
      if (tmp_str[0] == '\0') {
	printf("** Null zone entered, defaulting to previous zone.\n");
	current->zonenum= tmpptr->zonenum;
      } else {
        if(!scan_a_number(tmp_str, &zone)) {
          printf("** Invalid zone specified.\n");
          return;
        }
	current->zonenum= zone;
      }
      ptr = current;
      current = tmpptr;
    }
  }
  revindex = rev_dir[direction_number(inputs[1])];
  if (ptr->exit[revindex].to_room != -1) {
    printf("** %s exit already exists in target. Please UNLINK first.\n\n",
	   directions[revindex].name);
    return;
  }
  current->exit[index].to_room= target;
  current->exit[index].real_to_room = ptr;
  ptr->exit[revindex].to_room= current->room_id;
  ptr->exit[revindex].real_to_room = current;

  do {
    printf("Choose an exit type\n-------------------\n");
    for (index3 = 0; index3 < 5; index3++)
      printf("%d. %-s\n", index3 + 1, door_states[index3].name);
    printf("> ");
    /* gets(tmp_str); */
    fgets(tmp_str, BUFFER_SIZE-1, stdin);
    if((strlen(tmp_str) > 0) && tmp_str[strlen(tmp_str)-1] == '\n')
      tmp_str[strlen(tmp_str)-1]= '\0';
    trunc_string(tmp_str);
    index3 = atoi(tmp_str);
  } while (index3 < 1 || index3 > 5);

  current->exit[index].door_flag = --index3;
  ptr->exit[revindex].door_flag = index3;
  printf("\n");

  /* if this is a lockable door, ask for the object number to open it. */

  if (index3 > 0) {
    do {
      printf("Enter object number which can unlock/lock this door\n");
      printf("a -1 indicates that there is no key for this door.\n");
      printf("---------------------------------------------------\n");
      printf("> ");
      /* gets(tmp_str); */
      fgets(tmp_str, BUFFER_SIZE-1, stdin);
      if((strlen(tmp_str) > 0) && tmp_str[strlen(tmp_str)-1] == '\n')
        tmp_str[strlen(tmp_str)-1]= '\0';
      if(!scan_a_number(tmp_str, &zone)) {
        printf("** Invalid lock... try again.\n");
        continue;
      } else {
	current->exit[index].key_number= zone;
	ptr->exit[revindex].key_number= zone;
      }
    } while (tmp_str[0] == '\0');
    printf("\n");
  }
  /*  Get any keywords for that direction.  */

  do {
    index4 = FALSE;
    printf("Enter all exit keywords seperated by BLANKS for this exit\n");
    printf("or return if there are none.\n");
    printf("----------------------------------------------------------\n> ");
    /* gets(tmp_str); */
    fgets(tmp_str, BUFFER_SIZE-1, stdin);
    if((strlen(tmp_str) > 0) && tmp_str[strlen(tmp_str)-1] == '\n')
      tmp_str[strlen(tmp_str)-1]= '\0';
    trunc_string(tmp_str);

    if (index3 > 0 && tmp_str[0] == '\0') {
      printf("\n** At least one keyword must be specified for exits with a door.\n\n");
      index4 = TRUE;
    }
  } while (index4);
  if(current->exit[index].keywords)
    free(current->exit[index].keywords);
  current->exit[index].keywords= (char *)strdup(tmp_str);
  if(ptr->exit[revindex].keywords)
    free(ptr->exit[revindex].keywords);
  ptr->exit[revindex].keywords= (char *)strdup(tmp_str);
  printf("\n");

  /*  Get the exit description if any.  */

  if(current->exit[index].desc_ptr)
    free(current->exit[index].desc_ptr);
  if(ptr->exit[revindex].desc_ptr)
    free(ptr->exit[revindex].desc_ptr);

  input_desc("exit description from here", &current->exit[index].desc_ptr);
  input_desc("exit description to new room", &ptr->exit[revindex].desc_ptr);
  /* ptr->exit[revindex].desc_ptr = strdup(current->exit[index].desc_ptr); */
  printf("\n");
}

void Display_A_Room(Room *ptr)
{
  int index, index2, index3;

  if ((ptr->room_flag & room_flags[0].mask) && !Light) {
    printf("It is too dark to see anything.\n\n");
    return;
  }

  printf("%s\n", ptr->title);
  if (!Brief)
    printf("%s", ptr->desc_ptr);

  index = ptr->room_flag;
  index2 = 0;
  index3 = 0;
  if (index != 0) {
    printf("-----------------------------------------------------------------------\n");
    printf("ROOM FLAGS: ");
    while (room_flags[index2].name != NULL) {
      if (index & room_flags[index2].mask) {
	if (index3++)
	  printf(", ");
	printf("%-s", room_flags[index2].name);
      }
      index2++;
    }
    printf("\n");
  }
  /*  Write out teleport flags (if teleport room) */

  if (ptr->sector_type == TELEPORT_ROOM) {
    index = ptr->teleport->do_look;
    if (index != 0)
      index = 1;
    index2 = ptr->teleport->sector_type;
    if (index2 == -1)
      index2 = 10;

    printf("-----------------------------------------------------------------------\n");
    printf("TELEPORT: Time: %d   To Room: %d   Do Look: %-s\n",
	   ptr->teleport->time,
	   ptr->teleport->to_room,
	   true_or_false[index]);
    printf("          Movement type: %-s \n",
	   sector_types[index2].name);
  }
  /*  Write out river info (if river room) */

  if (ptr->sector_type == NOSWIM_ROOM) {
    printf("-----------------------------------------------------------------------\n");
    printf("RIVER: Speed: %d   Direction: %-s\n",
	   ptr->river->speed,
	   directions[ptr->river->direction].name);
  }
  /*  Write out sound info if sound flag is on.  */

  if (ptr->room_flag & 1024) {
    printf("-----------------------------------------------------------------------\n");
    for (index = 0; index < 2; index++) {
      printf("SOUND %d:\n", index + 1);
      printf("%s", ptr->sound_ptr[index]);
    }
  }
  /*  Write out the extra keywords, if any.  */

  if (current->number_extras > 0) {
    printf("-----------------------------------------------------------------------\n");
    printf("EXTRA DESCRIPTION KEYWORDS:\n");
    for (index = 0; index < ptr->number_extras; index++)
      printf("%s\n", ptr->extra[index].keywords);
  }
  printf("\n");
}

int direction_number(char *ptr)
{
  int index;

  for (index = 0; index < strlen(ptr); index++)
    ptr[index] = tolower(ptr[index]);

  index = 0;
  while (directions[index].num != -1 &&
	 strncmp(directions[index].name, ptr, strlen(ptr)) != 0)
    index++;

  return directions[index].num;
}

int find_keyword(char *buffer)
{
  int index;

  if (buffer[0] == '\0')
    return -1;

  for (index = 0; index < strlen(buffer); index++)
    buffer[index] = tolower(buffer[index]);

  index = 0;
  while (commands[index].name != NULL &&
	 strncmp(commands[index].name, buffer, strlen(buffer)) != 0)
    index++;

  return commands[index].num;
}

void get_inputs(void)
{
  int index, index2;
  char buffer[PAGE_SIZE];
  char *ptr;

  /* gets(buffer); */
  fgets(buffer, PAGE_SIZE-1, stdin);
  if((strlen(buffer) > 0) && buffer[strlen(buffer)-1] == '\n')
    buffer[strlen(buffer)-1]= '\0';
  ptr = buffer;

  for (index = 0; index < 11; index++) {
    index2 = 0;

    while (*ptr == ' ' && *ptr != 0)
      ptr++;

    if (*ptr == '\"') {
      ptr++;
      while (*ptr != '\"' && *ptr != 0)
	inputs[index][index2++] = *ptr++;
      if (*ptr == '\"')
	ptr++;
    } else {
      while (*ptr != ' ' && *ptr != 0)
	inputs[index][index2++] = *ptr++;
    }
    inputs[index][index2] = 0;
  }
}

void trunc_string(char *ptr)
{
  int index, found_eol, done;

/*  Remove all trailing blanks, carriage returns, newlines and
 *  NULLS,  this compresses the string thus saving storage upon
 *  saving the file.
 *  Also used to destroy blank lines..... evil...
 */
  found_eol = done = 0;
  index = strlen(ptr);
  while (index > -1 && !done) {
    switch (ptr[index]) {
    case '\0':
    case '\r':
    case '\t':
    case ' ':
      index--;
      break;
    case '\n':
      if (!found_eol) {
	found_eol = 1;
	index--;
      } else {
	done = 1;
      }
      break;
    default:
      done = 1;
    }
  }
  ptr[++index] = '\0';
/*
 * while (index > -1 &&
 * (ptr[index] == ' '  || ptr[index] == '\n' ||
 * ptr[index] == '\t' ||
 * ptr[index] == '\r' || ptr[index] == '\0'))
 * index--;
 */
}

void get_desc(char **ptr)
{
  int index = 0, index2;
  char tmp_str[PAGE_SIZE];

  /*  Read in a description into one long string
   * which will contain newlines within it.
   */

  do {
    if (fgets(tmp_str + index, 82, fp) == NULL) {
      printf("**Unexpected end of file encountered while reading room %d.\n",
	     current->room_id);
      exit(8);
    }
    trunc_string(tmp_str);

    index2 = strlen(tmp_str);
    tmp_str[index2] = '\n';
    tmp_str[index2 + 1] = 0;

    index = index2 + 1;
  } while (tmp_str[index2 - 1] != '~');
  tmp_str[index2 - 1] = 0;

  *ptr = malloc(strlen(tmp_str) + 1);
  strcpy(*ptr, tmp_str);
}

void input_desc(char *title, char **ptr)
{
  int index = 0, index2, line = 0;
  char tmp_str[PAGE_SIZE];

  /*  Input a description into one long string
   * which will contain newlines within it.
   */

  printf("Enter %s terminated with a ~ on a line by itself:\n", title);
  printf("If you hit RETURN on the first line, you will invoke an editor\n");
  printf("--------------------------------------------------------------\n");

  do {
    printf("%2d> ", ++line);

    /* gets(tmp_str + index); */
    fgets(tmp_str + index, BUFFER_SIZE-index-1, stdin);
    if((strlen(tmp_str) > 0) && tmp_str[strlen(tmp_str)-1] == '\n')
      tmp_str[strlen(tmp_str)-1]= '\0';

    if(line == 1) {
      if(!strcmp(tmp_str, "")) {
        import_desc(title, ptr);
        return;
      }
    }
    trunc_string(tmp_str);
    index2 = strlen(tmp_str);
    tmp_str[index2] = '\n';
    tmp_str[index2 + 1] = 0;
    index = index2 + 1;
  } while (tmp_str[index2 - 1] != '~');
  tmp_str[index2 - 1] = 0;
  *ptr = (char *)strdup(tmp_str);
}

void import_desc(char *title, char **ptr)
{
  int index = 0, index2;
  char tmp_str[PAGE_SIZE];
  FILE *fp;

  sprintf(tempfile, TMP_FILE, getpid());
  if(!(fp= fopen(tempfile, "r"))) {
    if(!(fp= fopen(tempfile, "w"))) {
      log("Cannot open %s for output!", tempfile);
      exit(ERR_NOOUTPUT);
    }
    fprintf(fp, "~\n");
  }
  fclose(fp);
  sprintf(tempfile, EDIT, getpid());
  system(tempfile);
  sprintf(tempfile, TMP_FILE, getpid());
  bzero(tmp_str, PAGE_SIZE);
  if(!(fp= fopen(tempfile, "r"))) {
    printf("** No description entered!  Description not changed.");
    return;
  }
  do {
    fgets(tmp_str + index, PAGE_SIZE-1, fp);
    trunc_string(tmp_str);

    index2 = strlen(tmp_str);
    tmp_str[index2] = '\n';
    tmp_str[index2 + 1] = 0;

    index = index2 + 1;
  } while (tmp_str[index2 - 1] != '~');
  tmp_str[index2 - 1] = 0;

  *ptr = (char *)strdup(tmp_str);
  fclose(fp);
}

Room *allocate_room(int temp_id)
{
  int index;
  Room *ptr, *ptr2;

  /*  Allocate new room.  */

  if (!(ptr = (Room *) calloc(1, sizeof(Room)))) {
    printf("** Unable to allocate memory for room.\n");
    printf("     Failed while trying to create room %d.\n", number_rooms);
    printf("      Note that this is the room number not room id.\n");
    exit(0);
  }
  /*  Connect to linked list (sorted order).  */
  /*  First check to see if its an empty list.  */

  if (bottom_of_world == NULL) {
    bottom_of_world = ptr;
    top_of_world = ptr;
    ptr->prev = NULL;
    ptr->next = NULL;
  } else {
    ptr2 = top_of_world;
    while(ptr2 && ptr2->room_id > temp_id) {
      ptr2 = ptr2->prev;
    }
    if (ptr2 == NULL) {
      ptr->prev = NULL;
      ptr->next = bottom_of_world;
      bottom_of_world->prev = ptr;
      bottom_of_world = ptr;
    } else {
      if (ptr2 == top_of_world) {
	ptr->next = NULL;
	top_of_world = ptr;
      } else {
	ptr->next = ptr2->next;
	ptr2->next->prev = ptr;
      }
      ptr->prev = ptr2;
      ptr2->next = ptr;
    }
  }

  ptr->room_id= temp_id;
  ptr->title = NULL;
  ptr->desc_ptr = NULL;
  ptr->zonenum= 0;
  ptr->room_flag = 0;
  ptr->sector_type = 0;
  ptr->sound_ptr[0] = NULL;
  ptr->sound_ptr[1] = NULL;
  ptr->teleport= NULL;
  ptr->river= NULL;
  for (index = 0; index < 6; index++) {
    ptr->exit[index].desc_ptr = NULL;
    ptr->exit[index].keywords = NULL;
    ptr->exit[index].door_flag = -1;
    ptr->exit[index].key_number= -1;
    ptr->exit[index].to_room= -1;
    ptr->exit[index].real_to_room = NULL;
  }
  ptr->number_extras = 0;
  ptr->extra= NULL;

  number_rooms++;
  return ptr;
}

Room *find_room(Room *current, int search_id)
{
  Room *ptr;

  /*  Returns the room number given a room id or -1 if not found.  */

  if (current->room_id < search_id) {
    ptr = current->next;
    while (ptr != NULL && (ptr->room_id != search_id))
      ptr = ptr->next;
  } else if (current->room_id > search_id) {
    ptr = current->prev;
    while (ptr != NULL && (ptr->room_id != search_id))
      ptr = ptr->prev;
  } else
    ptr = current;
  return ptr;
}

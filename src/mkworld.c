#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>

/*  This program may be freely desiminated to other sites with the
/*  understanding that my (Mike Nikkel) and Chris Michaels name will
/*  not be removed from the program header.  This program is released
/*  as is and we claim no responsibility for any damage or difficulties
/*  which may insue from the usage of this program.
/*  Comments and suggestions may be sent to msn@minnie.bell.inmet.com.
/*  */

#define TELEPORT_ROOM		-1
#define	NOSWIM_ROOM		 7
#define	TRUE			 1
#define FALSE			 0

/*  Note that these commands are sometimes in order of preference
/*  and therefore not alphabetical at all times.
/*  */

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
#define TYPE			24
#define ZONE			25
#define DIG			26
#define HELP			90
#define SAVE			91
#define BLANK			98
#define QUIT			99


struct commands_st
{
   char	name[10];
   int	num;
} commands[] = {
   { "north",         1 },
   { "east",          2 },
   { "south",         3 },
   { "west",          4 },
   { "up",            5 },
   { "down",          6 },
   { "unlink",         7 },
   { "brief",         8 },
   { "change",        9 },
   { "copy",         10 },
   { "make",       11 },
   { "delete",       12 },
   { "desc",         13 },
   { "exits",        14 },
   { "extra",        15 },
   { "flag",         16 },
   { "format",       17 },
   { "goto",         18 },
   { "look",         19 },
   { "light",        20 },
   { "link",         21 },
   { "list",         22 },
   { "title",        23 },
   { "type",         24 },
   { "zone",         25 },
   { "dig",          26 },
   { "help",         90 },
   { "save",         91 },
   { " ",            98 },
   { "quit",         99 },
   { NULL,           -1 }
};

struct dir_st 
{
  char name[9];
  int  num;
} directions[]={
  { "north",	0 },
  { "east",	1 },
  { "south",	2 },
  { "west",	3 },
  { "up",	4 },
  { "down",	5 },
  { "No Where", -1 },
  { NULL,       -1 }
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

struct room_flg_st
{
  char name[10];
  int  mask;
} room_flags[]= {
  { "Dark",		   1 },
  { "Death",		   2 },
  { "Nomob",		   4 },
  { "Indoor",		   8 },
  { "Peace",		  16 },
  { "Nosteal",		  32 },
  { "Nosummon",		  64 },
  { "Nomagic",		 128 },
  { "Tunnel",		 256 },
  { "Private",		 512 },
  { "Sound",		1024 }, 
  { NULL,		  -1 }
};

struct door_st
{
  char name[40];
  int  num;
} door_states[]={
  { "Open Passage",                           0 },
  { "Door which is lockable/pickable",        1 },
  { "Door which is lockable",                 2 },
  { "Secret Door which is lockable/pickable", 3 },
  { "Secret Door which is lockable",          4 },
  { NULL, -1 }
};

struct sector_st
{
  char name[15];
  int  num;
} sector_types[]={
  { "Inside",		0 },
  { "City",		1 },
  { "Field",		2 },
  { "Forest",		3 },
  { "Hill",		4 },
  { "Mountain",		5 },
  { "Swimming",		6 },
  { "River",		7 },
  { "Air",		8 },
  { "Underwater",	9 },
  { "Teleport",	       -1 },
  { NULL,	       -1 }
};

typedef	struct Room_struct
{
	char		room_id[20];
	char		title[80];
	char		*desc_ptr;
	char		zonenum[20];
	int		room_flag;
	int		sector_type;
	char		*sound_ptr[2];
	struct
	{
		int	time;
		char	to_room[20];
		int	do_look;
		int	sector_type;
	} teleport;
	struct
	{
		int	speed;
		int	direction;
	} river;
	struct
	{
		char	*desc_ptr;
		char	keywords[80];
		int	door_flag;
		char	key_number[20];
		char	to_room[20];
		struct	Room_struct	*real_to_room;
	} exit[6];
	int	number_extras;
	struct
	{
		char	keywords[80];
		char	*desc_ptr;
	} extra[10];
	struct	Room_struct	*prev;
	struct	Room_struct	*next;
} Room;

void 	Main_Loop();
void	Load_Rooms();
void	Link_World ();

void	Command_brief ();
void	Command_unlink ();
void	Command_change ();
void	Command_copy ();
void	Command_copy_sub ();
void	Command_make ();
void	Command_desc ();
void	Command_delete ();
void	Command_exits ();
void	Command_extra ();
void	Command_flag ();
void	Command_flag_sub ();
void	Command_format ();
void	Command_goto ();
void	Command_help ();
void	Command_light ();
void	Command_link ();
void	Command_list ();
void    Command_look ();
void	Command_save ();
void	Command_set ();
void	Command_title ();
void	Command_type ();
void	Command_zone ();
void	Command_dig ();

void	Display_A_Room ();
void	trunc_string ();
Room	*find_room ();
void	get_inputs ();
void	get_desc ();
void	input_desc ();
void	put_desc ();
int	print_desc ();
int	direction_number ();
Room	*allocate_room ();


/* Global Variables */

FILE 		*fp;
FILE		*out;
char		filename[80];
char		inputs[10][256];
char		true_or_false[2][6] = {"FALSE", "TRUE"};
int 		number_rooms=0;
Room		*bottom_of_world, *current, *top_of_world;

int		Brief = FALSE;
int		Light = FALSE;

/* ============================================================================*/
main(argc, argv)
int		argc;
char		*argv[];
{

	/*  Print title
	/*  */

	printf ("\n\n\n----------------------------------------------------------------------------\n");
	printf ("\t\t\tWorldmaker Version 1.1\n");
	printf ("\tProgrammed by : Mike Nikkel    Original Code : Chris Michaels\n");
	printf ("----------------------------------------------------------------------------\n");
	printf ("    Please send comments to me at msn@minnie.bell.inmet.com\n");
	printf ("----------------------------------------------------------------------------\n\n");

	/*  Load the rooms from the input file.
	/*  -----------------------------------
	/*  */

	if(argc > 1)
	{
	   strcpy (filename,argv[1]);
	   printf ("Using %s as source file.\n", filename);
	}
 	else
	{
	   strcpy (filename, "world.wld");
	   printf ("Assuming new world, default source file being used.\n");
	}

  	Load_Rooms(filename); 

	/*  Execute the main loop.
	/*  ----------------------
	/*  */

	Main_Loop();

	/*  Say goodnight gracie...
	/*  -----------------------
	/*  */

	printf ("\nBe seeing you.\n");
	exit (0);
}

/* -------------------------------------------------------------------*/
void Load_Rooms(filename)
char *filename;
{
char	tmp_str[81], temp_id[20];
int	index, index2, index3;
int	done, run_sort = FALSE;

	/*  Attempt to open the zone file.
	/*  ------------------------------
	/*  */


 	if (!(fp=fopen(filename,"r")))
	{
	   printf ("*Unable to open file %s. Terminating.\n",
	           filename);
	   exit(8);
	}

	/*  Read in all the rooms
	/*  ---------------------
	/*  */

	printf ("Loading world....\n");

	while(fgets(tmp_str,80,fp))
	{
	   /*  If a $ is encountered then we are at end of the world file.
	   /*  -----------------------------------------------------------
	   /*  */

	   if(tmp_str[0]=='$')
	      break;

	   /*  Read and verify Room id.
	   /*  ------------------------
	   /*  */

	   if (tmp_str[0] != '#')
	   {
		printf("\n**Room Id expected.  Instead recieved the following string:\n");
	        printf("%s\nWas working on room %d.\n\n",
	               tmp_str, number_rooms);
	        exit(8);
	   }

	   if(sscanf (tmp_str, "#%s", temp_id) == 0)
	   {
	        trunc_string (tmp_str);
		printf("\n** Room Id expected.  Instead recieved a blank string.\n");
	        printf("Was working on room %d.\n\n", number_rooms);
	        exit(8);
	   }
 
	   /*  allocate a new room.
	   /*  --------------------
	   /*  */

	   current = allocate_room (temp_id);

	   /*  Load in the room title.
	   /*  ----------------------
	   /*  */

	   if (fgets (tmp_str,80,fp) == NULL)
	   {
	      printf("\n**Unexpected end of file encountered while reading room %s.\n",
	             current->room_id);
	      exit(8);
	   }
	   if(tmp_str[0] == '~')
	   {
		printf("\n** Room title expected.  Instead recieved just a tilda.\n");
	        printf("Blank titles are not allowed, check for error in world file.\n");
	        printf("Was working on room %d room id %s\n\n",
 	               current, current->room_id);
	        exit(8);
	   }
	   if (tmp_str[strlen(tmp_str)-2] != '~')
	   {
	      printf ("\n** Room title %s does not end with a ~ on the same line.\n", tmp_str);
	      printf("Was working on room %d room id %s\n\n",
	             current, current->room_id);
	      exit(8);
	   }
	   strcpy (current->title, strtok (tmp_str,"~\n\r\0"));

	   /*  Load in the room description.
	   /*  -----------------------------
	   /*  */

	   get_desc (&current->desc_ptr);

	   /*  Load in the zone number, room flags, sector types
	   /*  */

	   if (fgets (tmp_str,80,fp) == NULL)
	   {
	      printf("** Unexpected end of file encountered while reading room %s.\n",
	             current->room_id);
	      exit(8);
	   }
	   index = sscanf (tmp_str, "%s %d %d", current->zonenum,
	                            &current->room_flag,
	                            &current->sector_type);
	   if (index < 3)
	   {
	        trunc_string (tmp_str);
		printf("\n** Room zone, flags, and sector type expected.  Instead recieved:\n");
	        printf("%s\n", tmp_str);
	        printf("Was working on room %d room id %s\n\n",
	               number_rooms, current->room_id);
	        exit(8);
	   }

	   /*  Load in teleport room information.
	   /*  ----------------------------------
	   /*  */

	   if (current->sector_type == -1)
	   {
	      index = sscanf (tmp_str, "%s %d %d %d %s %d %d", current->zonenum,
	                               &current->room_flag,
	                               &current->sector_type,
	                               &current->teleport.time,
	                               current->teleport.to_room,
	                               &current->teleport.do_look,
	                               &current->teleport.sector_type);
	      if (index < 7)
	      {
	         trunc_string (tmp_str);
	         printf("\n** Room is a Teleport room, teleport parameters missing.  Recieved:\n");
	         printf("%s\n", tmp_str);
	         printf("Was working on room %d room id %s\n\n",
	                number_rooms, current->room_id);
	         exit(8);
	      }
	   }

	   /*  Load in river room information.
	   /*  -------------------------------
	   /*  */

	   if (current->sector_type == 7)
	   {
	      index = sscanf (tmp_str, "%s %d %d %d %d", current->zonenum,
	                               &current->room_flag,
	                               &current->sector_type,
	                               &current->river.speed,
	                               &current->river.direction);
	      if (index < 5)
	      {
	         trunc_string (tmp_str);
	         printf("\n** Room is A River room, river parameters missing.  Recieved:\n");
	         printf("%s\n", tmp_str);
	         printf("Was working on room %d room id %s\n\n",
	                number_rooms, current->room_id);
	         exit(8);
	      }
	   }

	   /*  Load in sounds, if the room flag has the sound bit on.
	   /*  ------------------------------------------------------
	   /*  */

	   if (current->room_flag & 1024)
	   {
	      for (index=0; index < 2; index++)
	         get_desc (&current->sound_ptr[index]);
	   }
	   
	   /*  Load in the Exits and Extra Descriptions.
	   /*  -----------------------------------------
	   /*  */

	   done = FALSE;

	   while(!done) 
	   {
	      if (fgets (tmp_str,80,fp) == NULL)
	      {
	         printf("\n** Unexpected end of file encountered while reading room %s.\n",
	                current->room_id);
	         exit(8);
	      }

 	      switch (tmp_str[0])
	      {

	         /*  Exit information for a specific direction.
	         /*  ------------------------------------------
	         /*  */

	         case 'D':

	            /*  Exit number
	            /*  */

	            index2 = sscanf (tmp_str, "D%d", &index);
	            if (index2 < 1 || index < 0 || index > 5)
	            {
	               trunc_string (tmp_str);
		       printf("\n** Valid direction number expected.  Instead recieved:\n");
	               printf("%s\n", tmp_str);
	               printf("Was working on room %d room id %s\n\n",
	                      number_rooms, current->room_id);
	               exit(8);
	            }

	            /*  Exit description
	            /*  */

	            get_desc (&current->exit[index].desc_ptr);

	            /*  Exit keywords
	            /*  */

	            if (fgets (tmp_str,80,fp) == NULL)
	            {
	               printf("\n** Unexpected end of file encountered while reading room %s.\n",
	                      current->room_id);
	               exit(8);
	            }
	            trunc_string (tmp_str);
	            if (tmp_str[strlen(tmp_str)-1] != '~')
	            {
	               printf ("\n** Exit keywords %s do not end with a ~ on the same line.\n",
	                       tmp_str);
	               printf ("Was working on room %d room id %s\n\n", number_rooms,
	                       current->room_id);
	               exit(8);
	            }
	            tmp_str[strlen(tmp_str)-1] = 0;
	            strcpy (current->exit[index].keywords, tmp_str);

	            /*  Exit flags
	            /*  */

	            if (fgets (tmp_str,80,fp) == NULL)
	            {
	               printf("\n** Unexpected end of file encountered while reading room %s.\n",
	                      current->room_id);
	               exit(8);
	            }
	            index2 = sscanf (tmp_str, "%d %s %s", &current->exit[index].door_flag,
	                                      current->exit[index].key_number,
	                                      current->exit[index].to_room);
	            if (index2 < 3)
	            {
	               trunc_string (tmp_str);
		       printf("\n** Exit door flag, key, and to room expected.  Instead recieved:\n");
	               printf("%s\n", tmp_str);
	               printf("Was working on room %d room id %s\n\n",
	                      number_rooms, current->room_id);
	               exit(8);
	            }	            
	            if (current->exit[index].door_flag < 0 ||
	                current->exit[index].door_flag > 4)
	            {
	               trunc_string (tmp_str);
		       printf("\n** Valid exit door flag expected.  Instead recieved:\n");
	               printf("%s\n", tmp_str);
	               printf("Was working on room %d room id %s\n\n",
	                      number_rooms, current->room_id);
	               exit(8);
	            }
	            break;

	         /*  Extra Descriptions
	         /*  */

	         case 'E':

	            /*  Extra keywords
	            /*  */

	            if (fgets (tmp_str,80,fp) == NULL)
	            {
	               printf("\n** Unexpected end of file encountered while reading room %s.\n",
	                      current->room_id);
	               exit(8);
	            }
	            trunc_string (tmp_str);
	            if(tmp_str[0] == '~')
	            {
		       printf ("\n** Extra keywords expected.  Instead recieved just a tilda.\n");
	               printf ("Blank keywords are not allowed, check for error in world file.\n");
	               printf ("Was working on room %d room id %s\n\n", number_rooms,
	                       current->room_id);
	               exit(8);
	            }
	            if (tmp_str[strlen(tmp_str)-1] != '~')
	            {
	               printf ("\n** Extra keywords %s do not end with a ~ on the same line.\n", tmp_str);
	               printf("Was working on room %d room id %s\n\n",
	                      number_rooms, current->room_id);
	               exit(8);
	            }
	            
	            index2 = current->number_extras;
	            tmp_str[strlen(tmp_str)-1] = 0;
	            strcpy(current->extra[index2].keywords, tmp_str);

	            /*  Extra description
	            /*  */

	            get_desc (&current->extra[index2].desc_ptr);

	            /*  Increment number of Extra descriptions
	            /*  */

	            current->number_extras++;
	            break;

	         /*  End of the current room
	         /*  */

	         case 'S':
	            done = TRUE;
	            break;

	         default:
	            trunc_string (tmp_str);
	            printf("\n** Unknown auxiliary code found.  Received:\n");
	            printf("%s\n",tmp_str);
	            printf("Was working on room %d room id %s\n\n",
	                   number_rooms, current->room_id);
	            exit(8);
	            break;
	      }
	   }
	}

	current = bottom_of_world;
	printf("Total Rooms:%d\n",number_rooms);

	/*  Now link.
	/*  ---------
	/*  */

	Link_World ();
}

/* -------------------------------------------------------------------*/
void Link_World()
{
Room	*ptr;
int	index, index2;
int	init_flag = TRUE;
char	errname[81];
FILE	*errfile;

	printf ("Linking Rooms..\n");

	/*  For each valid direction in each room put in the real links.
	/*  ------------------------------------------------------------
	/*  */

	ptr = bottom_of_world;
	while (ptr != NULL)
	{
	   for (index2=0; index2 < 6; index2++)
	      if (ptr->exit[index2].door_flag > -1)
	      {
	         ptr->exit[index2].real_to_room =
	              find_room(ptr,ptr->exit[index2].to_room);
	         if (ptr->exit[index2].real_to_room == NULL)
	         {
	            if (init_flag)
	            {
	               strcpy (errname, strtok (filename, ".\n\r\t\0"));
	               strcat (errname, ".err");
 	               if (!(errfile=fopen(errname,"w")))
	               {
	                  printf ("*Unable to open file %s. Terminating.\n",
	                          filename);
	                  exit(8);
	               }
	               printf ("Unresolvable exits found. File %s created.\n",
	                       errname);
	               init_flag = FALSE;
	            }
	            fprintf (errfile,
	                    "* Room %s -- Unable to resolve exit to room %s\n",
	                     ptr->room_id,
	                     ptr->exit[index2].to_room);
	         }
	      }
	   ptr = ptr->next;
	}

	if (!init_flag)
	   fclose(errfile);
 
	printf ("\n");
}
/* ================================================================== */
/*              Main Command Loop                                     */
/* ================================================================== */

/* -------------------------------------------------------------------*/
void Main_Loop()
{
int	index, not_done = TRUE;


	/*  Display the initial room
	/*  */

	Display_A_Room (current);

	/*  Enter the command loop.
	/*  -----------------------
	/*  */

	do
	{
	   index = current->sector_type;
	   if (index == -1)
	      index = 10;

	   printf ("( Zone: %s Room: %s Terrain: %-s ) ",
	           current->zonenum,
	           current->room_id,
	          sector_types[index].name);

	   get_inputs ();
	   printf ("\n");

	   switch (index =find_keyword (inputs[0]))
	   {
	      case NORTH:
	      case EAST:
	      case SOUTH:
	      case WEST:
	      case UP:
	      case DOWN:
	            index--;
	            if (current->exit[index].real_to_room == NULL)
	               printf ("\n>> There is no exit in that direction.\n\n");
	            else
	            {
	               current = current->exit[index].real_to_room;
	               Display_A_Room (current);
	            }
	            break;
	      case BRIEF:
	            Command_brief ();
                    break;
	      case UNLINK:
	            Command_unlink ();
	            break;
	      case CHANGE:
	            Command_change ();
	            break;
	      case COPY:
	            Command_copy ();
	            break;
	      case MAKE:
	            Command_make ();
	            break;
	      case DELETE:
	            Command_delete ();
	            break;
	      case DESC:
	            Command_desc (0);
	            break;
	      case EXITS:
	            Command_exits ();
	            break;
	      case EXTRA:
	            Command_extra ();
	            break;
	      case FLAG:
	            Command_flag ();
	            break;
	      case FORMAT:
	            Command_format ();
	            break;
	      case GOTO:
	            Command_goto ();
	            break;
	      case HELP :
	           Command_help ();
	           break;
	      case LIGHT:
	            Command_light ();
                    break;
	      case LIST:
	            Command_list ();
	            break;
	      case LINK:
	            Command_link ();
	            break;
	      case LOOK :
	            Command_look ();
	            break;
	      case SAVE :
 	            Command_save ();
	            break;
	      case TITLE:
	            Command_title (0);
	            break;
	      case TYPE:
	            Command_type ();
	            break;
	      case BLANK:
	            break;
	      case QUIT :
	            not_done = FALSE;
	            break;
	      case ZONE:
	            Command_zone ();
	            break;
	      case DIG:
	            Command_dig ();
	            break;
	      default:
	            printf ("I beg your pardon?\n\n");
	            break;
	   }

	} while (not_done);


}

/* ================================================================== */
/*              Commands                                              */
/* ================================================================== */

/* ------------------------------------------------------------------ */
void Command_unlink ()
{
int	index;

	/*  Verify a direction was entered.
	/*  -------------------------------
	/*  */

	index = direction_number (inputs[1]);

	if (index == -1)
	{
	   printf ("UNLINK requires subparameters, the correct form is:\n");
	   printf ("--------------------------------------------------\n");
	   printf ("UNLINK <direction>\n\n");
	   return;
	}

	if (strcmp(current->exit[index].to_room, "-1") == 0)
	{
	   printf ("** There is currently no exit in that direction.\n\n");
	   return;
	}

	/*  Free the description, if any for that exit
	/*  ------------------------------------------
	/*  */

	free (current->exit[index].desc_ptr);

	/*  Now reset the exit.
	/*  -------------------
	/*  */

	current->exit[index].keywords[0]  = NULL;
	current->exit[index].door_flag    = -1;
	strcpy(current->exit[index].key_number,"-1");
	strcpy(current->exit[index].to_room,"-1");
	current->exit[index].real_to_room = NULL;

	printf (">> Link to the %s broken.\n\n",
	        directions[index].name);
}

/* -------------------------------------------------------------------*/
void Command_brief ()
{
	if (Brief)
	{
	   printf (">> Brief descriptions turned off.\n\n");
	   Brief = FALSE;
	}
	else
	{
	   printf (">> Brief descriptions turned on.\n\n");
	   Brief = TRUE;
	}
}

/* -------------------------------------------------------------------*/
void Command_change ()
{
char	tmp_str[1601];
char	*ptr;

	/*  Check for strings to change
	/*  ---------------------------
	/*  */

	if (inputs[1][0] == NULL || inputs[2][0] == NULL)
	{
	   printf ("CHANGE requires subparameters, the correct form is:\n");
	   printf ("----------------------------------------------------\n");
	   printf ("CHANGE <old> <new>\n\n");
	   printf ("    old - the string to change.\n");
	   printf ("    new - the string to change to.\n\n");
 	   return;
	}


	/*  Find the location of the string.
	/*  --------------------------------
	/*  */

	ptr = strstr (current->desc_ptr, inputs[1]);

	if (ptr == NULL)
	{
	   printf ("** String <%s> was not found in the description.\n\n",
	           inputs[1]);
	   return;
	}

	/*  Copy everything before the string to the tmp string.
	/*  ----------------------------------------------------
	/*  */

	*ptr = 0;
	strcpy (tmp_str, current->desc_ptr);

	/*  Copy in the new word.
	/*  ---------------------
	/*  */

	strcat (tmp_str, inputs[2]);

	/*  Copy in the rest of the original description.
	/*  ---------------------------------------------
	/*  */

	ptr += strlen(inputs[1]);
	strcat (tmp_str, ptr);

	/*  Free the old description and copy in the new.
	/*  ---------------------------------------------
	/*  */

	free (current->desc_ptr);

	current->desc_ptr = (char *)malloc(strlen(tmp_str)+1);
	strcpy (current->desc_ptr, tmp_str);

	/*  Redisplay the room.
	/*  -------------------
	/*  */

	printf (">> String has been changed.\n\n");

	Display_A_Room (current);
}

/* -------------------------------------------------------------------*/
void Command_copy ()
{
Room	*ptr, *ptr2;
int	count = 0;

	/*  Check for a room id
	/*  -------------------
	/*  */

	if (inputs[1][0] == NULL ||
	   (inputs[2][0] != NULL && inputs[3][0] == NULL))
	{
	   printf ("COPY requires subparameters, the correct form is:\n");
	   printf ("-------------------------------------------------------------\n");
	   printf ("COPY <room id> <start room> <end room>\n\n");
	   printf ("    <room id>    -  The room id from which to copy the title,\n");
	   printf ("                    desc, zonenum, room flag and type.\n");
	   printf ("    <start room>\n");
	   printf ("    <end room>   - (optional fields)  if specified they\n");
	   printf ("                    designate the rooms which will have their\n");
	   printf ("                    title, desc, zonenum, room flag and type\n");
	   printf ("                    changed.\n\n");
 	   return;
	}

	if ((ptr = find_room (current, inputs[1])) == NULL)
	{
	   printf ("** Room id %s does not exist. Title and description\n",
	           inputs[1]);
	   printf ("   were not copied to current room.\n\n");
	   return;
	}

	/*  If a start and end were NOT specified then set the current
	/*  rooms title, desc, etc..
	/*  ----------------------------------------------------------
	/*  */

	if (inputs[2][0] == NULL)
	{
	   Command_copy_sub (current, ptr);
	}
	else

	/*  If a start and end room were specified then change the
	/*  title, desc, etc.. for all rooms which fall between the
	/*  start and end room.
	/*  --------------------------------------------------------
	/*  */

	{
	   ptr2 = bottom_of_world;
	   while (ptr2 != NULL)
	   {
	      if (strcmp(ptr2->room_id, inputs[2]) >= 0 &&
	          strcmp(ptr2->room_id, inputs[3]) <= 0)
	      {
	         Command_copy_sub (ptr2, ptr);
	         count++;
	      }
	      ptr2 = ptr2->next;
	   }
	   if (count == 0)
	      printf ("** No Rooms were found between %s and %s.\n",
	              inputs[2],inputs[3]);
	}

	/*  Now redisplay the current room.
	/*  -------------------------------
	/*  */

	Display_A_Room (current);
}

/* -------------------------------------------------------------------*/
void Command_copy_sub (ptr, from_ptr)
Room	*ptr, *from_ptr;
{
int	index;

	/*  Copy the title.
	/*  -------------------
	/*  */

	strcpy (ptr->title, from_ptr->title);

	/*  Free the old description and copy in the new.
	/*  ---------------------------------------------
	/*  */

	free (ptr->desc_ptr);

	ptr->desc_ptr = 
	             (char *)malloc(strlen(from_ptr->desc_ptr)+1);
	strcpy (ptr->desc_ptr, from_ptr->desc_ptr);

	/*  Copy in the zonenum, room_flag, and type.
	/*  -----------------------------------------
	/*  */

	strcpy (ptr->zonenum, from_ptr->zonenum);

	ptr->room_flag   = from_ptr->room_flag;
	ptr->sector_type = from_ptr->sector_type;
	ptr->teleport    = from_ptr->teleport;
	ptr->river       = from_ptr->river;

	for (index=0; index < 2; index++)
	   if (from_ptr->sound_ptr[index] != NULL)
	   {
	      free (ptr->sound_ptr[index]);
	      ptr->sound_ptr[index] =
	            (char *) malloc(strlen(from_ptr->sound_ptr[index])+1);
 	      strcpy(ptr->sound_ptr[index], from_ptr->sound_ptr[index]);
	   }
	   else
	      ptr->sound_ptr[index] = NULL;

	/*  Notify that the room has been copied.
	/*  -------------------------------------
	/*  */

	printf (">> Title and description for room %s has been changed.\n",
	        ptr->room_id);
}

/* -------------------------------------------------------------------*/
void Command_make ()
{
int	run_sort = FALSE, index;
char	tmp_str[81];
Room	*ptr;
 
	/*  Check for a room id
	/*  -------------------
	/*  */

	if (inputs[1][0] == NULL)
	{
	   printf ("MAKE requires subparameters, the correct form is:\n");
	   printf ("----------------------------------------------------\n");
	   printf ("MAKE <room id> <from room id>\n\n");
	   printf ("  <room id>       - the room id of the new room.\n");
	   printf ("  <from room id>  - (optional) room from which the\n");
	   printf ("                    title, description, zone, types\n");
	   printf ("                    and flags should be copied.\n\n");
	   return;
	}

	if (find_room (current, inputs[1]) != NULL)
	{
	   printf ("** Room id %s already exists. Room not created.\n\n",
	           inputs[1]);
	   return;
	}

	if (inputs[2][0] != NULL)
	   if ((ptr=find_room (current, inputs[2])) == NULL)
	   {
	      printf ("** Room id %s does not exist. New Room not created.\n\n",
	              inputs[2]);
	      return;
	   }

	/*  Allocate a new room
	/*  -------------------
	/*  */

	current = allocate_room (inputs[1]);

	/*  Load in the room title, description, etc for a non-copied room.
	/*  ---------------------------------------------------------------
	/*  */

	if (inputs[2][0] == NULL)
	{
	   Command_title (1);

	   Command_desc (1);

	   printf ("Zone for this room\n------------------\n> ");
	   gets (tmp_str);
	   trunc_string(tmp_str);
	   if (tmp_str[0] == NULL)
	   {
	      printf ("** Null zone entered, defaulting to zone 99.\n");
	      strcpy (current->zonenum, "99");
	   }
	   else
	      strcpy(current->zonenum, strtok(tmp_str," \n\r\t"));
	}
	else

	/*  Load in the room title, description, etc for a copied room.
	/*  -----------------------------------------------------------
	/*  */

	{
	   strcpy (current->title,ptr->title);

	   current->desc_ptr = (char *)malloc (strlen (ptr->desc_ptr)+1);
	   strcpy (current->desc_ptr, ptr->desc_ptr);

	   strcpy (current->zonenum, ptr->zonenum);

	   current->room_flag = ptr->room_flag;
	   current->sector_type = ptr->sector_type;
	   current->teleport = ptr->teleport;
	   current->river = ptr->river;

	   for (index=0; index < 2; index++)
	      if (ptr->sound_ptr[index] != NULL)
	      {
	         current->sound_ptr[index] =
	               (char *) malloc(strlen(ptr->sound_ptr[index])+1);
 	         strcpy(current->sound_ptr[index], ptr->sound_ptr[index]);
	      }
	      else
	         current->sound_ptr[index] = NULL;
	}

	printf (">> New Room Created.\n\n");

	Display_A_Room (current);
}

/* -------------------------------------------------------------------*/
void Command_delete ()
{
Room	*ptr, *ptr2;
int	index;

	/*  Check for a room id
	/*  -------------------
	/*  */

	if (inputs[1][0] == NULL)
	{
	   printf ("DELETE requires a subparameter, the correct form is:\n");
	   printf ("----------------------------------------------------\n");
	   printf ("DELETE <room id>\n\n");
	   printf ("    <room id>       - the room id of the room to delete.\n\n");
	   return;
	}

	if ((ptr = find_room (current, inputs[1])) == NULL)
	{
	   printf ("** Room id %s does not exist. Room not deleted.\n\n",
	           inputs[1]);
	   return;
	}

	/*  Go through all the rooms and remove all links to the
	/*  specified room.
	/*  ----------------------------------------------------
	/*  */

	ptr2 = bottom_of_world;
	while (ptr2 != NULL)
	{
	   /*  Remove teleport links.
	   /*  */

	   if (strcmp (ptr2->teleport.to_room, ptr->room_id) == 0)
	   {
	      ptr2->sector_type = 0;
	      ptr2->teleport.time    = 0;
	      strcpy(ptr2->teleport.to_room,"-1");
	      ptr2->teleport.do_look = 0;
	      ptr2->teleport.sector_type = 0;
	      printf (">> Teleportal from room %s to room %s deleted.\n",
	               ptr2->room_id, ptr->room_id);
	   }

	   /*  Remove exit links.
	   /*  */

	   for (index=0; index < 6; index++)
	      if (ptr2->exit[index].real_to_room == ptr)
	      {
	         free (ptr2->exit[index].desc_ptr);
	         ptr2->exit[index].desc_ptr     = NULL;
	         ptr2->exit[index].keywords[0]  = NULL;
	         ptr2->exit[index].door_flag    = -1;
	         strcpy(ptr2->exit[index].key_number,"-1");
	         strcpy(ptr2->exit[index].to_room,"-1");
	         ptr2->exit[index].real_to_room = NULL;
	         printf (">> Exit %-6s from room %s to room %s deleted.\n",
	                  directions[index].name, ptr2->room_id,
	                  ptr->room_id);
	      }
	   ptr2 = ptr2->next;
	}

	/*  Now remove the room itself.
	/*  ---------------------------
	/*  */

	/*  Start by reseting the linked list.
	/*  ----------------------------------
	/*  */

	if (ptr == bottom_of_world)
	{
	  if (ptr->prev != NULL)
	     bottom_of_world = ptr->prev;
	  else
	     bottom_of_world = ptr->next;
	}

	if (ptr == top_of_world)
	{
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
	/*  the world.
	/*  ------------------------------------------------------
	/*  */

	if (ptr == current)
	   current = bottom_of_world;

	/*  Now free the room.
	/*  ------------------
	/*  */

	free (ptr->desc_ptr);
	for (index=0; index < 6; index++);
	  free (ptr->exit[index].desc_ptr);
	free (ptr);

	printf (">> Room %s has been deleted.\n\n", inputs[1]);

	/*  Redisplay the current room.
	/*  ---------------------------
	/*  */

	Display_A_Room (current);
}


/* -------------------------------------------------------------------*/
void Command_desc (flag)
int	flag;
{

	/*  Free the old description.
	/*  -------------------------
	/*  */

	if (current->desc_ptr)
	   free (current->desc_ptr);

	/*  Read in the new description.
	/*  ----------------------------
	/*  */

	input_desc ("new description", &current->desc_ptr);

	/*  Redisplay the new description.
	/*  ------------------------------
	/*  */

	if (flag)
	{
	   printf ("Description set to:\n-------------------\n");
	   printf ("%s", current->desc_ptr);
	   printf ("\n");
	}
	else
	   Display_A_Room (current);
}

/* -------------------------------------------------------------------*/
void Command_exits ()
{
int	index, index2, index3;
int	no_exits = TRUE;
Room	*ptr;

	for (index=0; index < 6; index++)
	{
	   index2 = current->exit[index].door_flag;

	   if (current->exit[index].real_to_room != NULL)
	   {
	      printf ("%-6s - %-s to Room %s (%-s)\n",
	              directions[index].name,
	              door_states[index2].name,
	              current->exit[index].real_to_room->room_id,
	              current->exit[index].real_to_room->title);
	      no_exits = FALSE;
	   }
	}

	if (no_exits)
	   printf ("There are no apparent exits from this room.\n");

	printf ("\n");
}

/* -------------------------------------------------------------------*/
void Command_extra ()
{
int	index, index2;
char	tmp_str[81];

	/*  Check to see if this is a delete or a new extra.
	/*  ------------------------------------------------
	/*  */

	if (find_keyword (inputs[1]) == DELETE)
	{
	   /*  If no extras to delete, warn them.
	   /*  ----------------------------------
	   /*  */

	   if (current->number_extras == 0)
	   {
	      printf ("**No extra descriptions to delete!\n\n");
	      return;
	   }

	   /*  List the keywords of the extra descriptions and
	   /*  let them choose.
	   /*  -----------------------------------------------
	   /*  */

	   printf ("Select the extra description to delete :\n");
	   printf ("----------------------------------------\n");

	   for (index=0; index < current->number_extras; index++)
	      printf ("%d. %s\n", index+1, current->extra[index].keywords);

	   printf ("\nEnter selection > ");
	   gets (tmp_str);
	   trunc_string(tmp_str);
	   index = atoi (tmp_str)-1;
	   if (index < 0 || index >= current->number_extras)
	   {
	      printf ("\n** Invalid selection.  No deletion performed.\n\n");
	      return;
	   }

	   /*  Now remove the extra description compressing the list.
	   /*  ------------------------------------------------------
	   /*  */

	   index2 = index;
	   while (index2 < current->number_extras-1)
	   {
	      strcpy(current->extra[index2].keywords,
	             current->extra[index2+1].keywords);
	      current->extra[index].desc_ptr =
	             current->extra[index2+1].desc_ptr;
	      index2++;
	   }

	   current->number_extras--;
	   current->extra[index2].keywords[0] = NULL;
	   free (current->extra[index2].desc_ptr);
	   current->extra[index2].desc_ptr = NULL;
	}
	else
	{
	   /*  Read in the keywords.
	   /*  ---------------------
	   /*  */

	   index = current->number_extras;

	   printf ("Please enter the extra description keywords seperated by BLANKS.\n> ");
	   gets (current->extra[index].keywords);
	   trunc_string(current->extra[index].keywords);

	   if (current->extra[index].keywords[0] == NULL)
	   {
	      printf ("\n**No keywords entered.  Extra description not created.\n\n");
	      return;
	   }
	   printf ("\n");

	   /*  Read in the extra description.
	   /*  ------------------------------
	   /*  */

	   if (current->extra[index].desc_ptr)
	      free (current->extra[index].desc_ptr);

	   /*  Read in the new description.
	   /*  ----------------------------
	   /*  */

	   input_desc ("extra description", &current->extra[index].desc_ptr);

	   current->number_extras++;
	}

	/*  Redisplay the Room.
	/*  -------------------
	/*  */

	Display_A_Room (current);
}

/* -------------------------------------------------------------------*/
void Command_flag ()
{
int	index  = 0, index2 = 0, count = 0;
Room	*ptr;

	/*  Check for a valid flag.
	/*  -----------------------
	/*  */

	while (room_flags[index].name[0] != NULL &&
	       strcasecmp (room_flags[index].name, inputs[1]) != 0)
	      index++;

	if (inputs[1][0] == NULL ||
	    room_flags[index].name[0] == NULL ||
	   (inputs[2][0] != NULL && inputs[3][0] == NULL))
	{
	   printf ("FLAG command requires sub-parameters, the correct form is:\n");
	   printf ("----------------------------------------------------------\n");
	   printf ("FLAG <new flag>  <start room> <end room>\n\n");
	   printf ("    <new flag>   -  flag to turn on/off for room(s).\n");
	   printf ("    <room id>    -  The room id from which to copy the title,\n");
	   printf ("                    desc, zonenum, room flag and type.\n");
	   printf ("    <start room>\n");
	   printf ("    <end room>   - (optional fields)  if specified they\n");
	   printf ("                    designate the rooms which will have their\n");
	   printf ("                    title, desc, zonenum, room flag and type\n");
	   printf ("                    changed.\n\n");
	   printf ("Valid FLAG parameters are:\n");
	   printf ("---------------------------");
	   while (room_flags[index2].name[0] != NULL)
	   {
	      if (index2%5 == 0)
	         printf ("\n");
 	      printf ("%-10s ", room_flags[index2++].name);
	   }
	   printf ("\n\n");
	   return;
	}

	/*  If a start and end were NOT specified then set the current
	/*  rooms flag to the new flag.
	/*  ----------------------------------------------------------
	/*  */

	if (inputs[2][0] == NULL)
	{
	   Command_flag_sub (current, index);
	}
	else

	/*  If a start and end room were specified then change the
	/*  room flag for all rooms which fall between the start and
	/*  end room.
	/*  --------------------------------------------------------
	/*  */

	{
	   ptr = bottom_of_world;
	   while (ptr != NULL)
	   {
	      if (strcmp(ptr->room_id, inputs[2]) >= 0 &&
	          strcmp(ptr->room_id, inputs[3]) <= 0)
	      {
	         Command_flag_sub (ptr, index);
	         count++;
	      }
	      ptr = ptr->next;
	   if (count == 0)
	      printf ("** No Rooms were found between %s and %s.\n",
	              inputs[2],inputs[3]);
	   }
	}

	/*  Redisplay the room.
	/*  -------------------
	/*  */

	Display_A_Room (current);
}

/* -------------------------------------------------------------------*/
void Command_flag_sub (ptr, index)
Room	*ptr;
int	index;
{
int	index2 = 0;

	/*  First handle turning the flag OFF.
	/*  ----------------------------------
	/*  */

	if (ptr->room_flag & room_flags[index].mask)
	{
	   ptr->room_flag = ptr->room_flag &
	                    ~room_flags[index].mask;

	   /*  Handle freeing of SOUND descriptions
	   /*  */

	   if (index == 10)
	      for (index2=0; index2 < 2; index2++)
	         free (ptr->sound_ptr[index2]);

	   printf (">> Flag %s turned off for room %s.\n\n",
	           inputs[1], ptr->room_id);
	}
	else

	/*  Now handle turning the flag ON.
	/*  -------------------------------
	/*  */

	{
	   ptr->room_flag = ptr->room_flag |
	                    room_flags[index].mask;

	   /*  Handle input of SOUND descriptions
	   /*  */

	   if (index == 10)
	   {
	      /*  Read in each of the 2 sound descriptions.
	      /*  */

	      input_desc ("sound #1", &ptr->sound_ptr[0]);
	      input_desc ("sound #2", &ptr->sound_ptr[1]);
	   }

	   printf (">> Flag %s turned on for room %s.\n\n",
	           inputs[1], ptr->room_id);
	}
}

/* -------------------------------------------------------------------*/
void Command_format ()
{
int	index = 0, index2 = 0, index3 = 0, index4 = 0;
char	*ptr;
char	tmp_str[81];

	/*  Check for valid format options.
	/*  -------------------------------
	/*  */

	if (strcmp (inputs[1],"desc") == 0 || strcmp (inputs[1],"DESC") == 0)
	   index = 1;
	else
	   if (strcmp (inputs[1],"extra") == 0 || strcmp (inputs[1],"EXTRA") == 0)
	      index = 2;


	if (index == 0)
	{
	   printf ("FORMAT command requires a sub-parameter, the correct form is:\n");
	   printf ("-------------------------------------------------------------\n");
	   printf ("FLAG <option>\n\n");
	   printf ("    <option>   -  Either desc or extra.  This command will\n");
	   printf ("                  reformat either the description or an\n");
	   printf ("                  extra description into strings of 78 chars.\n\n");
	   return;
	}

	/*  Now set the ptr to the description to format.
	/*  ---------------------------------------------
	/*  */

	if (index == 1)
	   ptr = current->desc_ptr;
	else
	{
	   if (current->number_extras == 0)
	   {
	      printf ("** No extra descriptions found for the current room.\n\n");
	      return;
	   }
	   printf ("Select the extra description to format :\n");
	   printf ("----------------------------------------\n");

	   for (index=0; index < current->number_extras; index++)
	      printf ("%d. %s\n", index+1, current->extra[index].keywords);

	   printf ("\nEnter selection > ");
	   gets (tmp_str);
	   trunc_string(tmp_str);
	   index = atoi (tmp_str)-1;
	   if (index < 0 || index >= current->number_extras)
	   {
	      printf ("\n** Invalid selection.  No formating performed.\n\n");
	      return;
	   }

	   ptr = current->extra[index].desc_ptr;
	}

	/*  At this point ptr is set to the proper description, so
	/*  format it.
	/*  ------------------------------------------------------
	/*  */

	/*  First get rid of all current new lines.
	/*  ---------------------------------------
	/*  */

	index = -1;
	while (++index <= strlen(ptr))
	   if (ptr[index] == '\n')
	      ptr[index] = ' ';

	/*  Now insert the new newlines.
	/*  ----------------------------
	/*  */

	index2 = 78;
	while (index2 < strlen (ptr))
	{
	   /*  Starting at the current lines end position, go backwards
	   /*  until we find a blank char.  This means we are at the
	   /*  beginning of a word.
	   /*  */

	   index3 = index2;
	   while (index3 > 1 && ptr[index3] != ' ')
	      index3--;
	   ptr[index3]= '\n';

	   /*  Now go forwards past all the blanks so we start the new
	   /*  "next line" with a non-blank char.
	   /*  */

	   index3++;
	   index4 = index3;
	   index2 = index3 + 77;
	   while (index3 <= strlen(ptr) && ptr[index3] == ' ')
	      index3++;

	   /*  Now compress everything down to eliminate the blanks
	   /*  that we skipped over when finding the start of the
	   /*  "next line".
	   /*  */

	   while (index3 <= strlen (ptr))
	      ptr[index4++] = ptr[index3++];
	   ptr[index3] = 0;
	}
	ptr[strlen(ptr)-1] = '\n';

	/*  Now redisplay the room.
	/*  -----------------------
	/*  */

	Display_A_Room (current);
}

/* -------------------------------------------------------------------*/
void Command_goto ()
{
int	new_room;
Room	*ptr;

	/*  Verify a room was specified.
	/*  ----------------------------
	/*  */

	if (inputs[1][0] == NULL)
	{
	   printf ("** Go to what room?\n\n");
	   return;
	}

	/*  Now check to see if its a valid room.
	/*  -------------------------------------
	/*  */

	ptr = find_room (current, inputs[1]);

	if (ptr == NULL)
	{
	   printf ("** Room %s does not exist!\n\n", inputs[1]);
	   return;
	}
	else

	/*  If everything is ok, then set the current room.
	/*  -----------------------------------------------
	/*  */

	{
	   current = ptr;
	   Display_A_Room (ptr);
	}
}

/* -------------------------------------------------------------------*/
void Command_help ()
{
int	index  = 1;
int	index2 = 0;
int	prev = -1;

	while (commands[index].name[0] != NULL)
	{
	   if (commands[index].num != prev)
	   {
	      printf ("%-10s ", commands[index].name);
	      prev = commands[index].num;
	      index2++;
	   }
	   if (index2%6 == 0)
	      printf ("\n");
	   index++;
	}

	printf ("\n\n");
}


/* -------------------------------------------------------------------*/
void Command_light ()
{
	if (Light)
	{
	   printf (">> You turn off your lantern.\n\n");
	   Light = FALSE;
	}
	else
	{
	   printf (">> You turn on your lantern.\n\n");
	   Light = TRUE;
	}
}

/* -------------------------------------------------------------------*/
void Command_link ()
{
int	index, index2, index3, index4;
char	tmp_str[81];
Room	*ptr;

	/*  Verify a direction was entered.
	/*  -------------------------------
	/*  */

	index = direction_number (inputs[1]);

	if (index == -1 || inputs[2][0] == NULL)
	{
	   printf ("LINK requires subparameters, the correct form is:\n");
	   printf ("----------------------------------------------------\n");
	   printf ("LINK <direction> <room id>\n\n");
	   return;
	}

	/*  Now make sure that the room to link to exists.
	/*  ----------------------------------------------
	/*  */

	ptr = find_room (current, inputs[2]);
	if (ptr == NULL)
	{
	   printf ("** Room %s does not exist. Link not created.\n\n",
	           inputs[2]);
	   return;
	}

	/*  See if link already exists.
	/*  ---------------------------
	/*  */

	if (strcmp(current->exit[index].to_room, "-1") != 0)
	{
	   printf ("** Exit already exists to the %s. Please UNLINK exit first.\n\n",
	           directions[index].name);
	   return;
	}

	strcpy(current->exit[index].to_room, inputs[2]);
	current->exit[index].real_to_room = ptr;

	/*  Enter the type of door.
	/*  -----------------------
	/*  */

	do
	{
	   printf ("Choose an exit type\n-------------------\n");
	   for (index3=0; index3 < 5; index3++)
	      printf ("%d. %-s\n", index3+1, door_states[index3].name);

	   printf ("> ");
	   gets (tmp_str);
	   trunc_string (tmp_str);

	   index3 = atoi (tmp_str);
	} while (index3 < 1 || index3 > 5);

	current->exit[index].door_flag = --index3;
	printf ("\n");

	/*  if this is a lockable door, ask for the object number to
	/*  open it.
	/*  --------------------------------------------------------
	/*  */

	if (index3 > 0)
	{
	   do
	   {
	      printf ("Enter object number which can unlock/lock this door\n");
	      printf ("a -1 indicates that there is no key for this door.\n");
	      printf ("---------------------------------------------------\n");
	      printf ("> ");
	      gets (tmp_str);
	      trunc_string (tmp_str);

	      if (tmp_str[0] == NULL)
	         printf ("\n** A value is required!\n\n");
	      else
	         strcpy(current->exit[index].key_number,tmp_str);
	   } while (tmp_str[0] == NULL);
 	   printf ("\n");
	}
	
	/*  Get any keywords for that direction.
	/*  ------------------------------------
	/*  */

	do
	{
	   index4 = FALSE;
	   printf ("Enter all exit keywords seperated by BLANKS for this exit\n");
	   printf ("or return if there are none.\n");
	   printf ("----------------------------------------------------------\n> ");
	   gets (tmp_str);
	   trunc_string (tmp_str);

	   if (index3 > 0 && tmp_str[0] == NULL)
	   {
	      printf ("\n** At least one keyword must be specified for exits with a door.\n\n");
	      index4 = TRUE;
	   }
	} while (index4);
	strcpy (current->exit[index].keywords, tmp_str);
	printf ("\n");

	/*  Get the exit description if any.
	/*  --------------------------------
	/*  */

	free (current->exit[index].desc_ptr);

	input_desc ("exit description", &current->exit[index].desc_ptr);
	printf ("\n");
}

/* -------------------------------------------------------------------*/
void Command_list ()
{
Room	*ptr;
int	count = 0;
char	tmp_str[81];

	/*  If no room id was entered then default to the current room.
	/*  -----------------------------------------------------------
	/*  */

	if (inputs[1][0] == 0)
	   strcpy (inputs[1], current->room_id);

	/*  If an ending room was not specified then just print out
	/*  the title of the current room.
	/*  -------------------------------------------------------
	/*  */

	if (inputs[2][0] == 0)
	{
	   ptr = find_room (current, inputs[1]);
	   if (ptr != NULL)
	      printf ("Room %s (in Zone %s) - %s\n", ptr->room_id,
                      ptr->zonenum, ptr->title);
	   else
	      printf ("** Room %s does not exist!\n", inputs[1]);
	}
	else
	{
	   ptr = bottom_of_world;
	   while (ptr != NULL)
	   {
	      if (strcmp(ptr->room_id, inputs[1]) >= 0 &&
	          strcmp(ptr->room_id, inputs[2]) <= 0)
	      {
	         printf ("Room %s (in Zone %s) - %s\n", ptr->room_id,
	                 ptr->zonenum, ptr->title);
	         count++;
	         if (count%22 == 0)
	         {
	            printf ("MORE? > ");
                    gets (tmp_str);
	            if (toupper(tmp_str[0]) == 'N')
	              break;
	         }
	      }
	      ptr = ptr->next;
	   }
	   if (count == 0)
	      printf ("** No Rooms were found between %s and %s.\n",
	              inputs[1],inputs[2]);
	}
	printf ("\n");
}

/* -------------------------------------------------------------------*/
void Command_look (buffer)
char	*buffer;
{
int	index = 0;
int	msg_printed = FALSE;

	/*  First check for no second argument.
	/*  -----------------------------------
	/*  */

	if (inputs[1][0] == NULL)
	   Display_A_Room (current);
	else
	{

	   /*  Now check to see if its a direction.
	   /*  ------------------------------------
	   /*  */

	   index = direction_number (inputs[1]);

	   if (index != -1)
	   {
	      if (current->exit[index].desc_ptr != NULL &&
	          *current->exit[index].desc_ptr != NULL)
	         printf ("%s", current->exit[index].desc_ptr);
	      else
	         printf ("You see nothing in particular in that direction.\n");
	      printf ("\n");
	   }
	   else

	   /*  Not a direction, so check for extra keyword.
	   /*  --------------------------------------------
	   /*  */

	   {
	      index = 0;
	      while (index < current->number_extras &&
	             strstr (current->extra[index].keywords,
	                     inputs[1]) == 0 )
	         index++;

	      if (index < current->number_extras &&
	          current->extra[index].desc_ptr != NULL)
	            printf ("%s", current->extra[index].desc_ptr);
	         else
	            printf ("You see of interest about the %s.\n", inputs[1]);
	      printf ("\n");
	   }
	}
}

/* -------------------------------------------------------------------*/
void Command_save()
{
int	index2;
char	filename[81], tmp_str[81], out_str[30];
Room	*ptr;


	/*  Prompt for a save name.
	/*  -----------------------
	/*  */

	printf ("Filename to save to? (default is mytest.wld)\n> ");
	gets (filename);
	trunc_string (filename);

	if (filename[0] == NULL)
	   strcpy (filename, "mytest.wld");

	/*  Check to see if the file already exists.
	/*  ----------------------------------------
	/*  */

	if (out = fopen(filename, "r"))
	{
	   printf ("File already exists, overwrite (Y or N)? ");
	   gets (tmp_str);

	   if (tmp_str[0] != 'Y' && tmp_str[0] != 'y')
	   {
	      printf ("\n** World not saved...\n\n");
	      return;
	   }
	   fclose (out);
	}

	/*  open the file for write.
	/*  ------------------------
	/*  */

	if (!(out=fopen(filename,"w")))
	{
	   printf("\n** Unable to create %s! ...\n\n", filename);
 	   exit(8);
  	}

	/*  Write out all rooms to the file.
	/*  --------------------------------
	/*  */

	printf ("\n>> Saving world to %s....\n\n", filename);
	
	ptr = bottom_of_world;
	while (ptr != NULL) 
	{

	   /*  Write the room id.
	   /*  ------------------
	   /*  */

	   fprintf (out,"#%s\n",ptr->room_id);

	   /*  Write the room title.
	   /*  ---------------------
	   /*  */

	   fprintf (out,"%s~\n",ptr->title);
 
	   /*  Write the room description.
	   /*  ---------------------------
	   /*  */

	   fprintf (out, "%s~\n", ptr->desc_ptr);

	   /*  Write the zone number, room flags, sector types.
	   /*  ------------------------------------------------
	   /*  */

	   fprintf (out,"%s %d %d",ptr->zonenum,
	                           ptr->room_flag,
	                           ptr->sector_type);

	   /*  Write teleport room information.
	   /*  --------------------------------
	   /*  */

	   if (ptr->sector_type == -1)
	      fprintf (out," %d %s %d %d",
	                           ptr->teleport.time,
	                           ptr->teleport.to_room,
	                           ptr->teleport.do_look,
	                           ptr->teleport.sector_type);

	   /*  Write the river room information.
	   /*  ---------------------------------
	   /*  */

	   if (ptr->sector_type == 7)
	      fprintf (out," %d %d", ptr->river.speed,
	                           ptr->river.direction);

	   fprintf (out,"\n");

	   /*  If sound bit is on write out sounds.
	   /*  ------------------------------------
	   /*  */

	   if (ptr->room_flag & 1024)
	      for (index2=0; index2 < 2; index2++)
	         fprintf (out, "%s~\n", ptr->sound_ptr[index2]);

	   /*  Write the Exits and Extra Descriptions.
	   /*  ---------------------------------------
	   /*  */

	   for (index2 = 0; index2 < 6; index2++)
	      if (ptr->exit[index2].door_flag != -1)
	      {
	         fprintf (out, "D%d\n", index2);

	         fprintf (out, "%s~\n", ptr->exit[index2].desc_ptr);

	         fprintf (out, "%s~\n",ptr->exit[index2].keywords);

	         fprintf (out, "%d %s %s\n",
	                               ptr->exit[index2].door_flag,
	                               ptr->exit[index2].key_number,
	                               ptr->exit[index2].to_room);
	      }

	   for (index2 = 0; index2 < ptr->number_extras; index2++)
	   {
	      fprintf (out,"E\n%s~\n", ptr->extra[index2].keywords);

	      fprintf (out, "%s~\n", ptr->extra[index2].desc_ptr);
	   }

	   fprintf (out, "S\n");
	   ptr = ptr->next;
	}

	fprintf (out, "$\n");
}

/* -------------------------------------------------------------------*/
void Command_title (flag)
int 	flag;
{
char	tmp_str[81];

	do
	{
	   printf ("Enter new title:\n----------------\n> ");
	   gets (tmp_str);
	   trunc_string (tmp_str);

	   if (tmp_str[0] == NULL)
	      printf ("** A non-blank string must be entered for the title.\n\n");

	} while (tmp_str[0] == NULL);

	strcpy (current->title, tmp_str);

	if (flag)
	   printf ("\nTitle set to:\n----------------\n%s\n\n", tmp_str);
	else
	   Display_A_Room (current);
}

/* -------------------------------------------------------------------*/
void Command_type ()
{
int	index  = 0, index2 = 0, index3 = 0;

	/*  Check for a valid type.
	/*  -----------------------
	/*  */

	while (sector_types[index].name[0] != NULL &&
	       strcasecmp (sector_types[index].name, inputs[1]) != 0)
	      index++;

	/*  Execute depending on the type specified.
	/*  ----------------------------------------
	/*  */

	if (sector_types[index].name[0] != NULL)
	{
	   index2 = sector_types[index].num;

	   /*  If teleport, verify its parameters.
	   /*  -----------------------------------
	   /*  */

	   if (index2 == -1)
	   {
	      if (inputs[2][0] == NULL || inputs[3][0] == NULL ||
	          inputs[4][0] == NULL || inputs[5][0] == NULL)
	      {
	         printf ("Type TELEPORT requires sub-parameters, the correct form is:\n");
	         printf ("-----------------------------------------------------------\n");
	         printf ("TYPE TELEPORT <teleport time> <to room> <do look> <move type>\n\n");
	         printf ("<teleport time> - multiple of 10, higher number longer wait.\n");
	         printf ("<to room>       - room to teleport to.\n");
	         printf ("<do look>       - 0 = Do not look   1 = Look after teleport\n");
	         printf ("<move type>     - type of terrain to use for movement cost.\n\n");
	         return;
	      }

	      if (atoi(inputs[2])%10 > 0)
	      {
	         printf ("** <teleport time> must be a multiple of 10, room type not set.\n\n");
	         return;
	      }

	      if (find_room (current, inputs[3]) == NULL)
	      {
	         printf ("** <to room> does not exist, room type not set.\n\n");
	         return;
	      }

	      if (atoi(inputs[4]) != 0 && atoi(inputs[4]) != 1)
	      {
	         printf ("** <do look> must be 0 or 1, room type not set.\n\n");
	         return;
	      }

	      while (sector_types[index3].name[0] != NULL &&
	             strcasecmp (sector_types[index3].name, inputs[5]) != 0)
	         index3++;

	      if (sector_types[index3].name[0] == NULL || sector_types[index3].num == -1)
	      {
	         printf ("** <move type> must be a valid terrain type, room type not set.\n\n");
	         return;
	      }
	   }

	   /*  If river, verify its parameters.
	   /*  --------------------------------
	   /*  */

	   if (index2 == 7)
	   {
	      if (inputs[2][0] == NULL || inputs[3][0] == NULL)
	      {
	         printf ("Type RIVER requires sub-parameters, the correct form is:\n");
	         printf ("--------------------------------------------------------\n");
	         printf ("TYPE RIVER <river speed> <river direction>\n\n");
	         printf ("     <river speed>     - time it takes to drift through an exit.\n");
	         printf ("     <river direction> - exit to drift through (s, n, e, w, etc..).\n\n");
	         return;
	      }

	      index3 = direction_number (inputs[3]);

	      if (index3 < 0 || index3 > 5)
	      {
	         printf ("** <river direction> must be a valid direction, room type not set.\n\n");
	         return;
	      }
	   }

	   /*  Set the rooms sector type.
	   /*  --------------------------
	   /*  */

	   current->sector_type = sector_types[index].num;

	   /*  For TELEPORT and RIVER types set their sub-parameters.
	   /*  ------------------------------------------------------
	   /*  */

	   if (index2 == -1)
	   {
	      current->teleport.time = atoi (inputs[2]);
	      strcpy(current->teleport.to_room, inputs[3]);
	      current->teleport.do_look = atoi (inputs[4]);
	      current->teleport.sector_type = index3;
	   }
	   else
	      if (index2 == 7)
	      {
	         current->river.speed = atoi (inputs[2]);
	         current->river.direction = index3;
	      }

	   printf ("\n");
	}
	else

	/*  Not a valid TYPE parameter so let them know.
	/*  --------------------------------------------
	/*  */

	{
	   printf ("Valid TYPE parameters are:\n");
	   printf ("--------------------------\n");
	   while (sector_types[index2].name[0] != NULL)
	   {
 	      printf ("%-12s ", sector_types[index2++].name);
	      if (index2 == 6)
	         printf ("\n");
	   }
	   printf ("\n\n");
	   return;
	}
}

/* -------------------------------------------------------------------*/
void Command_zone ()
{
Room	*ptr;
int	count = 0;

	/*  Verify correct parameters were specified.
	/*  -----------------------------------------
	/*  */

	if (inputs[1][0] == NULL ||
	   (inputs[2][0] != NULL && inputs[3][0] == NULL))
	{
	   printf ("ZONE command requires sub-parameters, the correct form is:\n");
	   printf ("----------------------------------------------------------\n");
	   printf ("ZONE <new zone>  <start room> <end room>\n\n");
	   printf ("     <new zone>   - new zone for room(s).\n");
	   printf ("     <start room>\n");
	   printf ("     <end room>   - (optional fields)  if specified they\n");
	   printf ("                    designate the rooms which willhave their\n");
	   printf ("                    zones changed.\n\n");
	   return;
	}

	/*  If a start and end were NOT specified then set the current
	/*  rooms zone to the new zone.
	/*  ----------------------------------------------------------
	/*  */

	if (inputs[2][0] == NULL)
	{
	   strcpy(current->zonenum, inputs[1]);
	   printf (">> Zone for room %s changed to zone %s.\n\n",
	           current->room_id, inputs[1]);
	}
	else

	/*  If a start and end room were specified then change the
	/*  zone id for all rooms which fall between the start and
	/*  end room.
	/*  ------------------------------------------------------
	/*  */

	{
	   ptr = bottom_of_world;
	   while (ptr != NULL)
	   {
	      if (strcmp(ptr->room_id, inputs[2]) >= 0 &&
	          strcmp(ptr->room_id, inputs[3]) <= 0)
	      {
	         strcpy(ptr->zonenum, inputs[1]);
	         printf (">> Zone for room %s changed to zone %s.\n\n",
	                 current->room_id, inputs[1]);
	         count++;
	      }
	      ptr = ptr->next;
	   if (count == 0)
	      printf ("** No Rooms were found between %s and %s.\n",
	              inputs[2],inputs[3]);
	   }
	}
}

void Command_dig ()
{
int	index, index2, index3, index4, i, revindex;
char	tmp_str[81];
Room	*ptr, *ptr2, *tmpptr;

	/*  Verify a direction was entered.  */

	index = direction_number (inputs[1]);

	if (index == -1 || inputs[2][0] == NULL)
	{
	   printf ("DIG requires subparameters, the correct form is:\n");
	   printf ("----------------------------------------------------\n");
	   printf ("DIG <direction> <room id> [room to copy]\n\n");
	   return;
	}

	/*  See if link already exists.  */

	if (strcmp(current->exit[index].to_room, "-1") != 0) {
	   printf ("** Exit already exists to the %s. Please UNLINK exit first.\n\n",
	           directions[index].name);
	   return;
	}

	/*  Now make sure that the room to link to exists.  */

	ptr = find_room (current, inputs[2]);
	if (ptr == NULL) {
          if(inputs[3][0] != NULL) {
	    if(!(ptr2 = find_room (current, inputs[3]))) {
	      printf ("** Room %s does not exist. Room not created.\n\n",
	              inputs[3]);
	      return;
            } else {
	      ptr = allocate_room (inputs[2]);
	      strcpy (ptr->title,ptr2->title);

	      ptr->desc_ptr = (char *)malloc (strlen (ptr2->desc_ptr)+1);
	      strcpy (ptr->desc_ptr, ptr2->desc_ptr);

	      strcpy (ptr->zonenum, ptr2->zonenum);

	      ptr->room_flag = ptr2->room_flag;
	      ptr->sector_type = ptr2->sector_type;
	      ptr->teleport = ptr2->teleport;
	      ptr->river = ptr2->river;

	      for (i=0; i< 2; i++)
	         if (ptr2->sound_ptr[i] != NULL)
	         {
	            ptr->sound_ptr[i] =
	                  (char *) malloc(strlen(ptr2->sound_ptr[i])+1);
 	            strcpy(ptr->sound_ptr[i], ptr2->sound_ptr[i]);
	         }
	         else
	            ptr->sound_ptr[i] = NULL;
            }
          } else {
            tmpptr= current;
	    current = allocate_room (inputs[2]);
	    Command_title (1);
	    Command_desc (1);
	    printf ("Zone for this room\n------------------\n> ");
	    gets (tmp_str);
	    trunc_string(tmp_str);
	    if (tmp_str[0] == NULL) {
	       printf ("** Null zone entered, defaulting to previous zone.\n");
	       strcpy (current->zonenum, tmpptr->zonenum);
	    } else
	       strcpy(current->zonenum, strtok(tmp_str," \n\r\t"));
            ptr= current;
            current= tmpptr;
          }
	}

	revindex = rev_dir[direction_number(inputs[1])];
	if (strcmp(ptr->exit[revindex].to_room, "-1") != 0) {
	   printf ("** %s exit already exists in target. Please UNLINK first.\n\n",
	           directions[revindex].name);
	   return;
	}

	strcpy(current->exit[index].to_room, inputs[2]);
	current->exit[index].real_to_room = ptr;
        strcpy(ptr->exit[revindex].to_room, current->room_id);
        ptr->exit[revindex].real_to_room = current;

	/*  Enter the type of door.  */

	do {
	   printf ("Choose an exit type\n-------------------\n");
	   for (index3=0; index3 < 5; index3++)
	      printf ("%d. %-s\n", index3+1, door_states[index3].name);
	   printf ("> ");
	   gets (tmp_str);
	   trunc_string (tmp_str);
	   index3 = atoi (tmp_str);
	} while (index3 < 1 || index3 > 5);

	current->exit[index].door_flag = --index3;
	ptr->exit[revindex].door_flag = index3;
	printf ("\n");

	/* if this is a lockable door, ask for the object number to open it. */

	if (index3 > 0) {
	   do {
	      printf ("Enter object number which can unlock/lock this door\n");
	      printf ("a -1 indicates that there is no key for this door.\n");
	      printf ("---------------------------------------------------\n");
	      printf ("> ");
	      gets (tmp_str);
	      trunc_string (tmp_str);
	      if (tmp_str[0] == NULL)
	         printf ("\n** A value is required!\n\n");
	      else {
	         strcpy(current->exit[index].key_number,tmp_str);
	         strcpy(ptr->exit[revindex].key_number,tmp_str);
              }
	   } while (tmp_str[0] == NULL);
 	   printf ("\n");
	}
	
	/*  Get any keywords for that direction.  */

	do {
	   index4 = FALSE;
	   printf ("Enter all exit keywords seperated by BLANKS for this exit\n");
	   printf ("or return if there are none.\n");
	   printf ("----------------------------------------------------------\n> ");
	   gets (tmp_str);
	   trunc_string (tmp_str);

	   if (index3 > 0 && tmp_str[0] == NULL)
	   {
	      printf ("\n** At least one keyword must be specified for exits with a door.\n\n");
	      index4 = TRUE;
	   }
	} while (index4);
	strcpy (current->exit[index].keywords, tmp_str);
	strcpy (ptr->exit[revindex].keywords, tmp_str);
	printf ("\n");

	/*  Get the exit description if any.  */

	free (current->exit[index].desc_ptr);
	free (ptr->exit[revindex].desc_ptr);

	input_desc ("exit description", &current->exit[index].desc_ptr);
        ptr->exit[revindex].desc_ptr= strdup(current->exit[index].desc_ptr);
	printf ("\n");
}

/* ================================================================== */
/*                Utility routines
/* ================================================================== */

/* -------------------------------------------------------------------*/
void Display_A_Room(ptr)
Room	*ptr;
{
int	index, index2, index3;

	/*  Check for a dark room.
	/*  ----------------------
	/*  */

	if ((ptr->room_flag & room_flags[0].mask) &&
	    !Light)
	{
	   printf ("It is too dark to see anything.\n\n");
	   return;
	}

	/*  Write the room title.
	/*  ---------------------
	/*  */

	printf ("%s\n",ptr->title);
 
	/*  Write the room description.
	/*  ---------------------------
	/*  */

	if (!Brief)
	   printf ("%s", ptr->desc_ptr);
 
	/*  Write out room flags (if any)
	/*  -----------------------------
	/*  */

	index  = ptr->room_flag;
	index2 = 0;
	index3 = 0;
	if (index != 0)
	{
	   printf ("----------------------------------------");
	   printf ("-------------------------------\n");
	   printf ("ROOM FLAGS: ");
	   while (room_flags[index2].name[0] != 0)
	   {
	      if (index & room_flags[index2].mask)
	      {
	         if (index3++)
	            printf (", ");
	         printf ("%-s", room_flags[index2].name);
	      }
	      index2++;
	   }
	   printf ("\n");
	}

	/*  Write out teleport flags (if teleport room)
	/*  -------------------------------------------
	/*  */

	if (ptr->sector_type == TELEPORT_ROOM)
	{
	   index = ptr->teleport.do_look;
	   if (index != 0)
	      index = 1;
	   index2= ptr->teleport.sector_type;
	   if (index2 == -1)
	      index2 = 10;

	   printf ("-----------------------------------------------------------------------\n");
	   printf ("TELEPORT: Time: %d   To Room: %d   Do Look: %-s\n",
	            ptr->teleport.time,
	            ptr->teleport.to_room,
	            true_or_false[index]);
	   printf ("          Movement type: %-s \n",
	            sector_types[index2].name);
 	}

	/*  Write out river info (if river room)
	/*  ------------------------------------
	/*  */

	if (ptr->sector_type == NOSWIM_ROOM)
	{
	   printf ("-----------------------------------------------------------------------\n");
	   printf ("RIVER: Speed: %d   Direction: %-s\n",
	            ptr->river.speed,
	            directions[ptr->river.direction].name);
	}

	/*  Write out sound info if sound flag is on.
	/*  -----------------------------------------
	/*  */

	if (ptr->room_flag & 1024)
	{
	   printf ("-----------------------------------------------------------------------\n");
	   for (index=0; index < 2; index++)
	   {
	      printf ("SOUND %d:\n",index+1);
	      printf ("%s", ptr->sound_ptr[index]);
	   }
	}

	/*  Write out the extra keywords, if any.
	/*  -------------------------------------
	/*  */

	if (current->number_extras > 0)
	{
	   printf ("-----------------------------------------------------------------------\n");
	   printf ("EXTRA DESCRIPTION KEYWORDS:\n");
	   for (index=0; index < ptr->number_extras; index++)
	      printf ("%s\n", ptr->extra[index].keywords);
	}

	printf ("\n");
}

/* -------------------------------------------------------------------*/
int direction_number (ptr)
char	*ptr;
{
int	index;

	/*  Convert the ptr to lower case.
	/*  */

	for (index=0; index < strlen (ptr); index++)
	   ptr[index] = tolower (ptr[index]);

	/*  Now check for a direction match.
	/*  */

	index = 0;
	while (directions[index].num != -1 &&
	       strncmp (directions[index].name, ptr, strlen(ptr)) != 0)
	   index++;
	
	return directions[index].num;
}

/* -------------------------------------------------------------------*/
int find_keyword (buffer)
char	*buffer;
{
int	index;

	/*  If a null string was entered then return -1.
	/*  --------------------------------------------
	/*  */

	if (buffer[0] == 0)
	   return 98;

	/*  Lower case the buffer.
	/*  ----------------------
	/*  */

	for (index=0; index < strlen(buffer); index++)
	   buffer[index] = tolower (buffer[index]);

	/*  Now search for the command.
	/*  ---------------------------
	/*  */

	index = 0;
	while (commands[index].name[0] != NULL &&
	       strncmp (commands[index].name, buffer, strlen(buffer)) != 0)
	   index++;

	return commands[index].num;
}

/* -------------------------------------------------------------------*/
void  get_inputs ()
{
int	index, index2;
char	buffer[2561];
char	*ptr;

	gets (buffer);
	ptr = buffer;

	for (index=0; index < 11; index++)
	{
	   index2 = 0;

	   /*  Remove leading blanks.
	   /*  */

	   while (*ptr == ' ' && *ptr != 0)
	      ptr++;

	   /*  Now check for a double quoted string.
	   /*  */

	   if (*ptr == '\"')
	   {
	      ptr++;
	      while (*ptr != '\"' && *ptr != 0)
	         inputs[index][index2++] = *ptr++;
	      if (*ptr == '\"')
	         ptr++;
	   }
	   else
	   {
	      while (*ptr != ' ' && *ptr != 0)
	         inputs[index][index2++] = *ptr++;
	   }
	   inputs[index][index2] = 0;
	}
}

/* -------------------------------------------------------------------*/
void trunc_string (ptr)
char	*ptr;
{
int	index;

	/*  Remove all trailing blanks, carriage returns, newlines and
	/*  NULLS,  this compresses the string thus saving storage upon
	/*  saving the file.
	/*  */

	index = strlen (ptr);
	while (index > -1 &&
	       (ptr[index] == ' '  || ptr[index] == '\n' ||
	        ptr[index] == '\r' || ptr[index] == '\0'))
	   index--;

	ptr [++index] = '\0';
}

/* -------------------------------------------------------------------*/
void get_desc (ptr)
char	**ptr;
{
int	index = 0, index2;
char	tmp_str[1601];

	/*  Read in a description into one long string
	/*  which will contain newlines within it.
	/*  ------------------------------------------
	/*  */

	do
	{
	   if (fgets (tmp_str+index,80,fp) == NULL)
	   {
	      printf("**Unexpected end of file encountered while reading room %s.\n",
	             current->room_id);
	      exit(8);
	   }
	   trunc_string (tmp_str);

	   index2 = strlen (tmp_str);
	   tmp_str[index2] = '\n';
	   tmp_str[index2+1] = 0;

	   index = index2+1;
	} while (tmp_str[index2-1] != '~');
	tmp_str[index2-1] = 0;

	*ptr = malloc (strlen(tmp_str)+1);
	strcpy (*ptr, tmp_str);
}

/* -------------------------------------------------------------------*/
void input_desc (title, ptr)
char	*title;
char	**ptr;
{
int	index = 0, index2;
char	tmp_str[1601];

	/*  Input a description into one long string
	/*  which will contain newlines within it.
	/*  ----------------------------------------
	/*  */

	printf ("Enter %s terminated with a ~ on a line by itself:\n", title);
	printf ("--------------------------------------------------------------\n");

	do
	{
	   printf ("> ");
	   gets (tmp_str+index);
	   trunc_string (tmp_str);

	   index2 = strlen (tmp_str);
	   tmp_str[index2] = '\n';
	   tmp_str[index2+1] = 0;

	   index = index2+1;
	} while (tmp_str[index2-1] != '~');
	tmp_str[index2-1] = 0;

	*ptr = (char *)malloc (strlen(tmp_str)+1);
	strcpy (*ptr, tmp_str);
}

/* -------------------------------------------------------------------*/
Room *allocate_room(temp_id)
char	*temp_id;
{
int	index;
Room	*ptr, *ptr2;

	/*  Allocate new room.
	/*  ------------------
	/*  */

	if (!(ptr = (Room *) malloc(sizeof(Room))))
	{
	   printf ("** Unable to allocate memory for room.\n");
	   printf ("     Failed while trying to create room %d.\n", number_rooms);
	   printf ("      Note that this is the room number not room id.\n");
	   exit(0);
	}

	/*  Connect to linked list (sorted order).
	/*  --------------------------------------
	/*  */

	/*  First check to see if its an empty list.
	/*  */

	if (bottom_of_world == NULL)
	{
	   bottom_of_world = ptr;
	   top_of_world = ptr;
	   ptr->prev = NULL;
	   ptr->next = NULL;
	}
	else
	{
	   /*  Now find the place in the list it should go.
	   /*  Work top-down since they are most likely already
	   /*  sorted.
	   /*  */

	   ptr2 = top_of_world;
	   while (ptr2 != NULL && strcmp (ptr2->room_id, temp_id) > 0)
	      ptr2 = ptr2->prev;

	   /*  if ptr2 is null then it goes at the front of the list.
	   /*  */

	   if (ptr2 == NULL)
	   {
	      ptr->prev = NULL;
	      ptr->next = bottom_of_world;
	      bottom_of_world->prev = ptr;
	      bottom_of_world = ptr;
	   }
	   else

	   /*  otherwise it gets stuck after the found ptr2.
	   /*  */

	   {
	      if (ptr2 == top_of_world)
	      {
	         ptr->next = NULL;
	         top_of_world = ptr;
	      }
	      else
	      {
	        ptr->next = ptr2->next;
	        ptr2->next->prev = ptr;
	      }

	      ptr->prev = ptr2;
	      ptr2->next = ptr;
	   }
	}

	/*  Initalize room values
	/*  ---------------------
	/*  */

	strcpy (ptr->room_id, temp_id);
	ptr->desc_ptr = NULL;
	strcpy(ptr->zonenum,"99");
	ptr->room_flag = 0;
	ptr->sector_type = 0;
	ptr->sound_ptr[0] = NULL;
	ptr->sound_ptr[1] = NULL;

	ptr->teleport.time    = 0;
	strcpy(ptr->teleport.to_room,"-1");
	ptr->teleport.do_look = 0;
	ptr->teleport.sector_type = 0;

	ptr->river.speed = 0;
	ptr->river.direction = 0;

	for (index = 0; index < 6; index++)
	{
	   ptr->exit[index].desc_ptr     = NULL;
	   ptr->exit[index].keywords[0]  = NULL;
	   ptr->exit[index].door_flag    = -1;
	   strcpy(ptr->exit[index].key_number,"-1");
	   strcpy(ptr->exit[index].to_room,"-1");
	   ptr->exit[index].real_to_room = NULL;
 	}

	ptr->number_extras = 0;
	for (index=0; index < 10; index++)
	{
	   ptr->extra[index].keywords[0] = NULL;
	   ptr->extra[index].desc_ptr = NULL;
	}

	number_rooms++;

	return ptr;
}

/* -------------------------------------------------------------------*/
Room *find_room (current, search_id)
Room	*current;
char	*search_id;
{
Room	*ptr;

	/*  Returns the room number given a room id or -1 if not
	/*  found.
	/*  ----------------------------------------------------
	/*  */

	if (strcmp (current->room_id, search_id) < 0)
	{
	   ptr = current->next;
	   while (ptr != NULL &&
	          strcmp (ptr->room_id, search_id) != 0)
	      ptr = ptr->next;
	}
	else
	   if (strcmp (current->room_id, search_id) > 0)
	   {
	      ptr = current->prev;
	      while (ptr != NULL &&
	             strcmp (ptr->room_id, search_id) != 0)
	         ptr = ptr->prev;
	   }
	   else
	      ptr = current;

	return ptr;
}


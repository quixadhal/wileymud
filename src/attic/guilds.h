/*
 * This file contains the guild structures and declares for the guilds
 * used in wileymudII
 */

#define MAX_GUILD_CLASSES 5

typedef struct
{
  int str;			/* minimums a character must have to join */
  int int;
  int wis;
  int dex;
  int con;
  int age;
  unsigned int align;
} Guild_Min;

typedef struct 
{
  int level;			/* if set above 0, this is the level the player must be to join */
  int class;			/* what is the class that can join this guild
				   : 0 = Any Class
				   : 1 = Mage
				   : 2 = Cleric
				   : 3 = Warrior
				   : 4 = Thief
				   : 5 = Ranger
				   : 6 = ??????
  int races[5];			/* if set other then -1, you must be of this race to join */  
  Guild_Min minimums;		/* guild minimums to join */
} Guild_Res;

typedef struct 
{
  int  wiz_standing;		/* 0,1,2 did the wiz turn this guild off */
  char guild_name[80];		/* name of the guild */
  char guild_master[80];	/* name of guild master */
  int  date_of_create;		/* date this guild was created */
  char wiz_founder[80];		/* name of wizard that founded them */
  Guild_Res restrictions;	/* restrictions put on this guild */
  Guild_st *next_guild;
} Guild_st;

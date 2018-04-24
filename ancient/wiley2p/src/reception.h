/* ************************************************************************
*  file: reception.h, Special module for Inn's.           Part of DIKUMUD *
*  Usage: Procedures handling saving/loading of player objects            *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */
#define OBJ_FILE_FREE "\0\0\0"

#define MAX_OBJ_SAVE 200 /* Used in OBJ_FILE_U *DO*NOT*CHANGE* */

struct rental_header {
  char	inuse;
  int	length;
  char owner[20];    /* Name of player                     */
};

struct obj_file_elem {
	sh_int item_number;

	int value[4];
	int extra_flags;
	int weight;
	int timer;
	long bitvector;
	struct obj_affected_type affected[MAX_OBJ_AFFECT];
};

struct obj_file_u
{
	int gold_left;     /* Number of goldcoins left at owner  */
	int total_cost;    /* The cost for all items, per day    */
	long last_update;  /* Time in seconds, when last updated */
	long minimum_stay; /* For stasis */
	int nobjects;	   /* how many objects below */
	struct obj_file_elem objects[MAX_OBJ_SAVE];
			   /* We don't always allocate this much space
			      but it is handy for the times when you
			      need a fast one lying around.  */
};

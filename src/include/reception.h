/*
 * file: reception.h, Special module for Inn's.           Part of DIKUMUD
 * Usage: Procedures handling saving/loading of player objects
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */
#ifndef _RECEPTION_H
#define _RECEPTION_H

#define OBJ_FILE_FREE "\0\0\0\0\0\0\0"

#define MAX_OBJ_SAVE 200 /* Used in OBJ_FILE_U *DO*NOT*CHANGE* */

struct rental_header
{
    char inuse;
    int length;
    char owner[20]; /* Name of player */
};

struct obj_file_elem
{
    short int item_number;
    int value[4];
    int extra_flags;
    int weight;
    int timer;
    long bitvector;
    struct obj_affected_type affected[MAX_OBJ_AFFECT];
};

struct obj_file_u
{
    int gold_left;     /* Number of goldcoins left at owner */
    int total_cost;    /* The cost for all items, per day */
    long last_update;  /* Time in seconds, when last updated */
    long minimum_stay; /* For stasis */
    int nobjects;      /* how many objects below */
    struct obj_file_elem objects[MAX_OBJ_SAVE];
    /*
     * We don't always allocate this much space but it is handy for the times when you need a fast one lying around.
     */
};

#ifndef _RECEPTION_C
#endif

void add_obj_cost(struct char_data *ch, struct char_data *re, struct obj_data *obj, struct obj_cost *cost);
char recep_offer(struct char_data *ch, struct char_data *recep_mob, struct obj_cost *cost);
void update_file(FILE *fl, char *name, struct obj_file_u *st);
void obj_store_to_char(struct char_data *ch, struct obj_file_u *st);
void load_char_objs(struct char_data *ch);
void put_obj_in_store(struct obj_data *obj, struct obj_file_u *st);

/* static int contained_weight(struct obj_data *container); */
void obj_to_store(struct obj_data *obj, struct obj_file_u *st, struct char_data *ch, int do_delete);
void save_obj(struct char_data *ch, struct obj_cost *cost, int do_delete);
void fwrite_obj(struct obj_data *obj, FILE *fp, int ObjectId, int ContainedBy);
int new_save_obj(struct char_data *ch, struct obj_data *obj, FILE *fp, int do_delete, int ObjId, int ContainedBy);
void new_save_equipment(struct char_data *ch, struct obj_cost *cost, int do_delete);
int receptionist(struct char_data *ch, int cmd, char *arg);
void zero_rent(struct char_data *ch);
int fread_object(struct obj_data *obj, FILE *fp);
int new_load_equipment(struct char_data *ch, struct obj_cost *cost);
int TotalWeight(struct obj_data *obj);

#endif

/*
 * file: handler.h , Handler module.                      Part of DIKUMUD
 * Usage: Various routines for moving about objects/players
 */

#ifndef _HANDLER_H
#define _HANDLER_H

#define FIND_CHAR_ROOM     1
#define FIND_CHAR_WORLD    2
#define FIND_OBJ_INV       4
#define FIND_OBJ_ROOM      8
#define FIND_OBJ_WORLD    16
#define FIND_OBJ_EQUIP    32

char *fname(char *namelist);
int split_string(char *str, char *sep, char **argv);
int isname(char *str, char *namelist);
void init_string_block(struct string_block *sb);
void append_to_string_block(struct string_block *sb, char *str);
void page_string_block(struct string_block *sb, struct char_data *ch);
void destroy_string_block(struct string_block *sb);
void affect_modify(struct char_data *ch,byte loc,byte mod, long bitv, bool add);
void affect_total(struct char_data *ch);
void affect_to_char( struct char_data *ch, struct affected_type *af );
void affect_remove( struct char_data *ch, struct affected_type *af );
void affect_from_char( struct char_data *ch, short skill);
bool affected_by_spell( struct char_data *ch, short skill );
void affect_join( struct char_data *ch, struct affected_type *af, bool avg_dur, bool avg_mod );
void char_from_room(struct char_data *ch);
void char_to_room(struct char_data *ch, int room);
void obj_to_char(struct obj_data *object, struct char_data *ch);
void obj_from_char(struct obj_data *object);
int apply_ac(struct char_data *ch, int eq_pos);
void equip_char(struct char_data *ch, struct obj_data *obj, int pos);
struct obj_data *unequip_char(struct char_data *ch, int pos);
int get_number(char **name);
struct obj_data *get_obj_in_list(char *name, struct obj_data *list);
struct obj_data *get_obj_in_list_num(int num, struct obj_data *list);
struct obj_data *get_obj(char *name);
struct obj_data *get_obj_num(int nr);
struct char_data *get_char_room(char *name, int room);
struct char_data *get_char(char *name);
struct char_data *get_char_num(int nr);
void obj_to_room(struct obj_data *object, int room);
void obj_from_room(struct obj_data *object);
void obj_to_obj(struct obj_data *obj, struct obj_data *obj_to);
void obj_from_obj(struct obj_data *obj);
void object_list_new_owner(struct obj_data *list, struct char_data *ch);
void extract_obj(struct obj_data *obj);
void update_object( struct obj_data *obj, int use);
void update_char_objects( struct char_data *ch );
void extract_char(struct char_data *ch);
struct char_data *get_char_room_vis(struct char_data *ch, char *name);
struct char_data *get_char_vis_world(struct char_data *ch, char *name, int *count);
struct char_data *get_char_vis(struct char_data *ch, char *name);
struct obj_data *get_obj_in_list_vis(struct char_data *ch, char *name, struct obj_data *list);
struct obj_data *get_obj_vis_world(struct char_data *ch, char *name, int *count);
struct obj_data *get_obj_vis(struct char_data *ch, char *name);
struct obj_data *get_obj_vis_accessible(struct char_data *ch, char *name);
struct obj_data *create_money( int amount );
int generic_find(char *arg, int bitvector, struct char_data *ch, struct char_data **tar_ch, struct obj_data **tar_obj);

#endif

#ifndef _ACT_INFO_H
#define _ACT_INFO_H

void argument_split_2(const char *argument, char *first_arg, char *second_arg);
struct obj_data *get_object_in_equip_vis(struct char_data *ch, const char *arg, struct obj_data *equipment[], int *j);
char *find_ex_description(char *word, struct extra_descr_data *list);
void show_obj_to_char(struct obj_data *object, struct char_data *ch, int mode);
void show_mult_obj_to_char(struct obj_data *object, struct char_data *ch, int mode, int num);
void list_obj_in_room(struct obj_data *list, struct char_data *ch);
void list_obj_on_char(struct obj_data *list, struct char_data *ch);
void list_obj_to_char(struct obj_data *list, struct char_data *ch, int mode, char show);
void show_char_to_char(struct char_data *i, struct char_data *ch, int mode);
void show_mult_char_to_char(struct char_data *i, struct char_data *ch, int mode, int num);
void list_char_in_room(struct char_data *list, struct char_data *ch);
void list_char_to_char(struct char_data *list, struct char_data *ch, int mode);
int do_look(struct char_data *ch, const char *argument, int cmd);
int do_read(struct char_data *ch, const char *argument, int cmd);
int do_examine(struct char_data *ch, const char *argument, int cmd);
int do_search(struct char_data *ch, const char *argument, int cmd);
int list_exits_in_room(struct char_data *ch); // depreciated?
int do_exits(struct char_data *ch, const char *argument, int cmd);
int do_score(struct char_data *ch, const char *argument, int cmd);
int do_mystat(struct char_data *ch, const char *argument, int cmd);
int do_time(struct char_data *ch, const char *argument, int cmd);
int do_weather(struct char_data *ch, const char *argument, int cmd);
int do_allcommands(struct char_data *ch, const char *argument, int cmd);
int do_inventory(struct char_data *ch, const char *argument, int cmd);
int do_equipment(struct char_data *ch, const char *argument, int cmd);
int do_credits(struct char_data *ch, const char *argument, int cmd);
int do_news(struct char_data *ch, const char *argument, int cmd);
int do_info(struct char_data *ch, const char *argument, int cmd);
int do_wizlist(struct char_data *ch, const char *argument, int cmd);

/* static int which_number_mobile(struct char_data *ch, struct char_data *mob); */
char *numbered_person(struct char_data *ch, struct char_data *person);

/* static void where_person(struct char_data *ch, struct char_data *person, struct string_block *sb); */
/* static void where_object(struct char_data *ch, struct obj_data *obj, int recurse, struct string_block *sb); */
int do_where(struct char_data *ch, const char *argument, int cmd);
int do_levels(struct char_data *ch, const char *argument, int cmd);
int do_consider(struct char_data *ch, const char *argument, int cmd);
int do_spells(struct char_data *ch, const char *argument, int cmd);
int do_world(struct char_data *ch, const char *argument, int cmd);
int do_skills(struct char_data *ch, const char *argument, int cmd);
int do_ticks(struct char_data *ch, const char *argument, int cmd);
int do_map(struct char_data *ch, const char *argument, int cmd);
int do_ansimap(struct char_data *ch, const char *argument, int cmd);
int do_version(struct char_data *ch, const char *argument, int cmd);
int do_autoexit(struct char_data *ch, const char *argument, int cmd);

#endif

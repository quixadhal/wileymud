#ifndef _MODIFY_H
#define _MODIFY_H

#define REBOOT_AT1 7     /* 0-23, time of optional reboot if -e lib/reboot */
#define REBOOT_AT2 19

#define TP_MOB    0
#define TP_OBJ    1
#define TP_ERROR  2

#define GR
#define NEW

#ifndef _MODIFY_C
extern struct room_data *world;
extern char *string_fields[];
extern char *room_fields[];
extern int length[];
extern int room_length[];
extern char *skill_fields[];
extern int max_value[];

#endif

void string_add(struct descriptor_data *d, char *str);
void quad_arg(char *arg, int *type, char *name, int *field, char *string);
void do_string(struct char_data *ch, char *arg, int cmd);
void bisect_arg(char *arg, int *field, char *string);
void do_setskill(struct char_data *ch, char *arg, int cmd);
char *one_word(char *argument, char *first_arg);
struct help_index_element *build_help_index(FILE * fl, int *num);
void page_string(struct descriptor_data *d, char *str, int keep_internal);
void show_string(struct descriptor_data *d, char *input);
void night_watchman(void);
void check_reboot(void);

#ifdef GR
int workhours();
int load(void);

#if 0
char *nogames(void);

#endif
#ifdef OLD_COMA
void coma(void);

#endif
void gr(int s);

#endif

#endif

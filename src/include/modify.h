#ifndef _MODIFY_H
#define _MODIFY_H

#define TP_MOB    0
#define TP_OBJ    1
#define TP_ERROR  2

#define GR
#define NEW

#ifndef _MODIFY_C
extern int                              REBOOT_AT1,
                                        REBOOT_AT2;	       /* 0-23, time of optional reboot if -e lib/reboot */
extern struct room_data                *world;
extern const char                            *string_fields[];
extern const char                            *room_fields[];
extern int                              length[];
extern int                              room_length[];
extern const char                            *skill_fields[];
extern int                              max_value[];

#endif

void                                    string_add(struct descriptor_data *d, char *str);
void                                    quad_arg(const char *arg, int *type, char *name, int *field,
						 char *string);
void                                    do_string(struct char_data *ch, const char *arg, int cmd);
void                                    bisect_arg(const char *arg, int *field, char *string);
void                                    do_setskill(struct char_data *ch, const char *arg, int cmd);
char                                   *one_word(char *argument, char *first_arg);
struct help_index_element              *build_help_index(FILE * fl, int *num);
void                                    page_printf(struct char_data *ch, const char *Str, ...)
                                                    __attribute__ ( ( format( printf, 2, 3 ) ) );
void                                    page_string(struct descriptor_data *d, char *str,
						    int keep_internal);
void                                    show_page(struct descriptor_data *d);
void                                    control_page(struct descriptor_data *d, char *input);
void                                    check_reboot(void);

#ifdef GR
int                                     workhours(void);
int                                     load(void);

#if 0
char                                   *nogames(void);

#endif
#ifdef OLD_COMA
void                                    coma(void);

#endif
void                                    gr(int s);

#endif

#endif

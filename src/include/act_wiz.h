#ifndef _ACT_WIZ_H
#define _ACT_WIZ_H

struct show_room_zone_struct {
  int                                     blank;
  int                                     startblank,
                                          lastblank;
  int                                     bottom,
                                          top;
  struct string_block                    *sb;
};

void                                    do_polymorph(struct char_data *ch, char *argument,
						     int cmdnum);
void                                    do_highfive(struct char_data *ch, char *argument,
						    int cmd);
void                                    do_rentmode(struct char_data *ch, char *argument,
						    int cmd);
void                                    do_wizlock(struct char_data *ch, char *argument,
						   int cmd);
void                                    do_emote(struct char_data *ch, char *argument, int cmd);
void                                    do_echo(struct char_data *ch, char *argument, int cmd);
void                                    do_system(struct char_data *ch, char *argument,
						  int cmd);
void                                    do_trans(struct char_data *ch, char *argument, int cmd);
void                                    do_at(struct char_data *ch, char *argument, int cmd);
void                                    do_form(struct char_data *ch, char *argument, int cmd);
void                                    do_goto(struct char_data *ch, char *argument, int cmd);
void                                    do_home(struct char_data *ch, char *argument, int cmd);
void                                    do_apraise(struct char_data *ch, char *argument,
						   int cmd);
void                                    do_stat(struct char_data *ch, char *argument, int cmd);
void                                    do_pretitle(struct char_data *ch, char *argument,
						    int cmd);
void                                    do_set(struct char_data *ch, char *argument, int cmd);
void                                    do_shutdow(struct char_data *ch, char *argument,
						   int cmd);
void                                    do_shutdown(struct char_data *ch, char *argument,
						    int cmd);
void                                    do_snoop(struct char_data *ch, char *argument, int cmd);
void                                    do_switch(struct char_data *ch, char *argument,
						  int cmd);
void                                    do_return(struct char_data *ch, char *argument,
						  int cmd);
void                                    do_force(struct char_data *ch, char *argument, int cmd);
void                                    do_load(struct char_data *ch, char *argument, int cmd);

/* static void purge_one_room(int rnum, struct room_data *rp, int *range); */
void                                    do_purge(struct char_data *ch, char *argument, int cmd);
void                                    roll_abilities(struct char_data *ch);
void                                    start_character(struct char_data *ch);
void                                    do_advance(struct char_data *ch, char *argument,
						   int cmd);
void                                    do_reroll(struct char_data *ch, char *argument,
						  int cmd);
void                                    do_restore_all(struct char_data *ch, char *arg,
						       int cmd);
void                                    restore_one_victim(struct char_data *victim);
void                                    do_restore(struct char_data *ch, char *argument,
						   int cmd);
void                                    do_show_logs(struct char_data *ch, char *argument,
						     int cmd);
void                                    do_noshout(struct char_data *ch, char *argument,
						   int cmd);
void                                    do_pager(struct char_data *ch, char *arg, int cmd);
void                                    do_nohassle(struct char_data *ch, char *argument,
						    int cmd);
void                                    do_stealth(struct char_data *ch, char *argument,
						   int cmd);

/* static print_room(int rnum, struct room_data *rp, struct string_block *sb); */
/* static void print_death_room(int rnum, struct room_data *rp, struct string_block *sb); */
/* static void print_private_room(int rnum, struct room_data *rp, struct string_block *sb); */
/* static void show_room_zone(int rnum, struct room_data *rp, struct show_room_zone_struct *srzs); */
void                                    do_show(struct char_data *ch, char *argument, int cmd);
void                                    do_debug(struct char_data *ch, char *argument, int cmd);
void                                    do_invis(struct char_data *ch, char *argument, int cmd);
void                                    do_reset(struct char_data *ch, char *argument, int cmd);
void                                    do_zone_purge(struct char_data *ch, char *argument,
						      int cmd);
void                                    do_not_yet_implemented(struct char_data *ch,
							       char *argument, int cmd);
void                                    do_setreboot(struct char_data *ch, char *argument,
						     int cmd);

#endif

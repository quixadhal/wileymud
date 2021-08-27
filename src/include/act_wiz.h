#ifndef _ACT_WIZ_H
#define _ACT_WIZ_H

struct show_room_zone_struct
{
    int blank;
    int startblank, lastblank;
    int bottom, top;
    struct string_block *sb;
};

int do_polymorph(struct char_data *ch, const char *argument, int cmdnum);
int do_highfive(struct char_data *ch, const char *argument, int cmd);
int do_rentmode(struct char_data *ch, const char *argument, int cmd);
int do_wizlock(struct char_data *ch, const char *argument, int cmd);
int do_emote(struct char_data *ch, const char *argument, int cmd);
int do_echo(struct char_data *ch, const char *argument, int cmd);
int do_system(struct char_data *ch, const char *argument, int cmd);
int do_trans(struct char_data *ch, const char *argument, int cmd);
int do_at(struct char_data *ch, const char *argument, int cmd);
int do_form(struct char_data *ch, const char *argument, int cmd);
int do_goto(struct char_data *ch, const char *argument, int cmd);
int do_home(struct char_data *ch, const char *argument, int cmd);
int do_apraise(struct char_data *ch, const char *argument, int cmd);
int do_stat(struct char_data *ch, const char *argument, int cmd);
int do_pretitle(struct char_data *ch, const char *argument, int cmd);
int do_set(struct char_data *ch, const char *argument, int cmd);
int do_snoop(struct char_data *ch, const char *argument, int cmd);
int do_switch(struct char_data *ch, const char *argument, int cmd);
int do_return(struct char_data *ch, const char *argument, int cmd);
int do_force(struct char_data *ch, const char *argument, int cmd);
int do_load(struct char_data *ch, const char *argument, int cmd);

/* static void purge_one_room(int rnum, struct room_data *rp, int *range); */
int do_purge(struct char_data *ch, const char *argument, int cmd);
void roll_abilities(struct char_data *ch);
void start_character(struct char_data *ch);
int do_advance(struct char_data *ch, const char *argument, int cmd);
int do_reroll(struct char_data *ch, const char *argument, int cmd);
int do_restore_all(struct char_data *ch, const char *arg, int cmd);
void restore_one_victim(struct char_data *victim);
int do_restore(struct char_data *ch, const char *argument, int cmd);
int do_show_logs(struct char_data *ch, const char *argument, int cmd);
int do_noshout(struct char_data *ch, const char *argument, int cmd);
int do_pager(struct char_data *ch, const char *arg, int cmd);
int do_nohassle(struct char_data *ch, const char *argument, int cmd);
int do_stealth(struct char_data *ch, const char *argument, int cmd);

/* static print_room(int rnum, struct room_data *rp, struct string_block *sb); */
/* static void print_death_room(int rnum, struct room_data *rp, struct string_block *sb); */
/* static void print_private_room(int rnum, struct room_data *rp, struct string_block *sb); */
/* static void show_room_zone(int rnum, struct room_data *rp, struct show_room_zone_struct *srzs); */
int do_show(struct char_data *ch, const char *argument, int cmd);
int do_debug(struct char_data *ch, const char *argument, int cmd);
int do_invis(struct char_data *ch, const char *argument, int cmd);
int do_reset(struct char_data *ch, const char *argument, int cmd);
int do_zone_purge(struct char_data *ch, const char *argument, int cmd);
int do_not_yet_implemented(struct char_data *ch, const char *argument, int cmd);

#endif

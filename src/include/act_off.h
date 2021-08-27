#ifndef _ACT_OFF_H
#define _ACT_OFF_H

#ifndef _ACT_OFF_C
extern bfuncp bweapons[];

#endif

int do_swat(struct char_data *ch, const char *argument, int cmd);
int do_hit(struct char_data *ch, const char *argument, int cmd);
int do_kill(struct char_data *ch, const char *argument, int cmd);
int do_backstab(struct char_data *ch, const char *argument, int cmd);
int do_order(struct char_data *ch, const char *argument, int cmd);
int do_flee(struct char_data *ch, const char *argument, int cmd);
int do_bandage(struct char_data *ch, const char *argument, int cmd);
void slam_into_wall(struct char_data *ch, struct room_direction_data *exitp);
int do_doorbash(struct char_data *ch, const char *argument, int cmd);
int do_bash(struct char_data *ch, const char *argument, int cmd);
int do_punch(struct char_data *ch, const char *argument, int cmd);
int do_rescue(struct char_data *ch, const char *argument, int cmd);
int do_assist(struct char_data *ch, const char *argument, int cmd);
int do_kick(struct char_data *ch, const char *argument, int cmd);
int do_wimp(struct char_data *ch, const char *argument, int cmd);
int do_breath(struct char_data *ch, const char *argument, int cmd);

#if 0
void                                    do_shoot(struct char_data *ch, const char *argument, int cmd);

#endif

#endif

#ifndef _ACT_OFF_H
#define _ACT_OFF_H

#ifndef _ACT_OFF_C
extern funcp bweapons[];

#endif

void do_swat(struct char_data *ch, char *argument, int cmd);
void do_hit(struct char_data *ch, char *argument, int cmd);
void do_kill(struct char_data *ch, char *argument, int cmd);
void do_backstab(struct char_data *ch, char *argument, int cmd);
void do_order(struct char_data *ch, char *argument, int cmd);
void do_flee(struct char_data *ch, char *argument, int cmd);
void do_bandage(struct char_data *ch, char *argument, int cmd);
void slam_into_wall(struct char_data *ch, struct room_direction_data *exitp);
void do_doorbash(struct char_data *ch, char *arg, int cmd);
void do_bash(struct char_data *ch, char *argument, int cmd);
void do_punch(struct char_data *ch, char *argument, int cmd);
void do_rescue(struct char_data *ch, char *argument, int cmd);
void do_assist(struct char_data *ch, char *argument, int cmd);
void do_kick(struct char_data *ch, char *argument, int cmd);
void do_wimp(struct char_data *ch, char *argument, int cmd);
void do_breath(struct char_data *ch, char *argument, int cmd);

#if 0
void do_shoot(struct char_data *ch, char *argument, int cmd);

#endif

#endif

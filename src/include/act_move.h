#ifndef _ACT_MOVE_H
#define _ACT_MOVE_H

void NotLegalMove(struct char_data *ch);
int check_exit_alias(struct char_data *ch, char *argument);
int ValidMove(struct char_data *ch, int cmd);
int RawMove(struct char_data *ch, int dir);
int MoveOne(struct char_data *ch, int dir);
int MoveGroup(struct char_data *ch, int dir);
void DisplayOneMove(struct char_data *ch, int dir, int was_in);
void DisplayGroupMove(struct char_data *ch, int dir, int was_in, int total);
int do_move(struct char_data *ch, char *argument, int cmd);
void DisplayMove(struct char_data *ch, int dir, int was_in, int total);
void AddToCharHeap(struct char_data *heap[50], int *top, int total[50], struct char_data *k);
int find_door(struct char_data *ch, char *type, char *dir);
void open_door(struct char_data *ch, int dir);
void raw_open_door(struct char_data *ch, int dir);
void do_open(struct char_data *ch, char *argument, int cmd);
void do_close(struct char_data *ch, char *argument, int cmd);
int has_key(struct char_data *ch, int key);
void raw_unlock_door(struct char_data *ch, struct room_direction_data *exitp, int door);
void raw_lock_door(struct char_data *ch, struct room_direction_data *exitp, int door);
void do_lock(struct char_data *ch, char *argument, int cmd);
void do_unlock(struct char_data *ch, char *argument, int cmd);
void do_pick(struct char_data *ch, char *argument, int cmd);
void do_enter(struct char_data *ch, char *argument, int cmd);
void do_leave(struct char_data *ch, char *argument, int cmd);
void do_stand(struct char_data *ch, char *argument, int cmd);
void do_sit(struct char_data *ch, char *argument, int cmd);
void do_rest(struct char_data *ch, char *argument, int cmd);
void do_sleep(struct char_data *ch, char *argument, int cmd);
void do_wake(struct char_data *ch, char *argument, int cmd);
void do_follow(struct char_data *ch, char *argument, int cmd);

#endif

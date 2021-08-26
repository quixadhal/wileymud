#ifndef _TRACKING_H
#define _TRACKING_H

struct hunting_data
{
    char *name;
    struct char_data **victim;
};

#define IS_DIR (real_roomp(q_head->room_nr)->dir_option[i])
#define GO_OK (!IS_SET(IS_DIR->exit_info, EX_CLOSED) && (IS_DIR->to_room != NOWHERE))
#define GO_OK_SMARTER (!IS_SET(IS_DIR->exit_info, EX_LOCKED) && (IS_DIR->to_room != NOWHERE))

int is_target_room_p(int room, void *tgt_room);
int named_object_on_ground(int room, void *c_data);
int named_mobile_in_room(int room, void *c_data);

/* static void donothing(); */
int choose_exit(int in_room, int tgt_room, int depth);
int go_direction(struct char_data *ch, int dir);
int find_path(int in_room, predicate_funcp predicate, void *c_data, int depth);
void MobHunt(struct char_data *ch);
int dir_track(struct char_data *ch, struct char_data *vict);
int track(struct char_data *ch, struct char_data *vict);
void do_track(struct char_data *ch, const char *argument, int cmd);
char *track_distance(struct char_data *ch, char *mob_name);
void do_immtrack(struct char_data *ch, const char *argument, int cmd);

#endif

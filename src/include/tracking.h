#ifndef _TRACKING_H
#define _TRACKING_H

struct hunting_data {
  char                                   *name;
  struct char_data                      **victim;
};

#define IS_DIR	(real_roomp(q_head->room_nr)->dir_option[i])
#define GO_OK	(!IS_SET(IS_DIR->exit_info,EX_CLOSED)\
                 && (IS_DIR->to_room != NOWHERE))
#define GO_OK_SMARTER	(!IS_SET(IS_DIR->exit_info,EX_LOCKED)\
                         && (IS_DIR->to_room != NOWHERE))

inline int                              is_target_room_p(int room, void *tgt_room);
inline int                              named_object_on_ground(int room, void *c_data);
inline int                              named_mobile_in_room(int room,
							     struct hunting_data *c_data);
/* static void donothing(); */
inline int                              choose_exit(int in_room, int tgt_room, int depth);
inline int                              go_direction(struct char_data *ch, int dir);
int                                     find_path(int in_room, ifuncp predicate, void *c_data,
						  int depth);
void                                    MobHunt(struct char_data *ch);
int                                     dir_track(struct char_data *ch, struct char_data *vict);
int                                     track(struct char_data *ch, struct char_data *vict);
void                                    do_track(struct char_data *ch, char *argument, int cmd);

#endif

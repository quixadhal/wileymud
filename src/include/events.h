#ifndef _DIKU_EVENTS_H
#define _DIKU_EVENTS_H

struct event_mob_set {
  int                                     vnum;
  int                                     hp_dice,
                                          hp_die,
                                          hp_mod;
  int                                     exp_dice,
                                          exp_die,
                                          exp_mod;
  int                                     gold_dice,
                                          gold_die,
                                          gold_mod;
  int                                     obj_chance,
                                          obj_vnum;
};

struct event_mob_in_zone {
  int                                     bottom,
                                          top;
  int                                     chance;
  int                                     atleast,
                                          atmost;
  int                                     count;
  struct event_mob_set                   *mobset;
};

struct event_goodies {
  int                                     bottom,
                                          top;
  int                                     chance;
  int                                     gold_dice,
                                          gold_die,
                                          gold_mod;
  int                                     obj_chance;
  int                                     obj_count;
  int                                    *obj_vnum;
  int                                     mob_chance;
  int                                     mob_count;
  int                                    *mob_vnum;
};

void                                    do_event(struct char_data *ch, char *argument, int cmd);

#endif

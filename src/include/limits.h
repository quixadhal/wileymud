/*
 * file: limits.h , Limit/Gain control module             Part of DIKUMUD
 * Usage: declaration of title type
 */

#ifndef _DIKU_LIMITS_H
#define _DIKU_LIMITS_H

/* title_type definition moved to global.h */

#define STARTING_HP 20
#define MOB_LEVELING

char *ClassTitles(struct char_data *ch);
int graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6);
int mana_limit(struct char_data *ch);
int hit_limit(struct char_data *ch);
int move_limit(struct char_data *ch);
int mana_gain(struct char_data *ch);
int hit_gain(struct char_data *ch);
int move_gain(struct char_data *ch);
void advance_level(struct char_data *ch, int class);
void drop_level(struct char_data *ch, int class);
void set_title(struct char_data *ch);
void gain_exp(struct char_data *ch, int gain);
void gain_exp_regardless(struct char_data *ch, int gain, int class);
void gain_condition(struct char_data *ch, int condition, int value);
void check_idling(struct char_data *ch);
void point_update(int pulse);
int ObjFromCorpse(struct obj_data *c);

#endif

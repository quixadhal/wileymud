#ifndef _MOB_ACTIONS_H
#define _MOB_ACTIONS_H

void mobile_guardian(struct char_data *ch);
void mobile_wander(struct char_data *ch);
void MobScavenge(struct char_data *ch);
void mobile_activity(void);
int SameRace(struct char_data *ch1, struct char_data *ch2);
void AssistFriend(struct char_data *ch);

#endif

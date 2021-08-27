#ifndef _ACT_SKILLS_H
#define _ACT_SKILLS_H

int do_disarm(struct char_data *ch, const char *argument, int cmd);
int do_peer(struct char_data *ch, const char *argument, int cmd);
int RideCheck(struct char_data *ch);
void FallOffMount(struct char_data *ch, struct char_data *h);
void Dismount(struct char_data *ch, struct char_data *h, int pos);
int MountEgoCheck(struct char_data *rider, struct char_data *mount);
int do_mount(struct char_data *ch, const char *arg, int cmd);

#endif

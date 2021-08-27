#ifndef _ACT_COMM_H
#define _ACT_COMM_H

#define MAX_NOTE_LENGTH 2048 /* arbitrary */
#define OLD_SHOUT

int do_say(struct char_data *ch, const char *argument, int cmd);
int do_shout(struct char_data *ch, const char *argument, int cmd);
int do_commune(struct char_data *ch, const char *argument, int cmd);
int do_tell(struct char_data *ch, const char *argument, int cmd);
int do_whisper(struct char_data *ch, const char *argument, int cmd);
int do_ask(struct char_data *ch, const char *argument, int cmd);
int do_write(struct char_data *ch, const char *argument, int cmd);

int do_group_tell(struct char_data *ch, const char *argument, int cmd);
int do_group_report(struct char_data *ch, const char *argument, int cmd);
int do_split(struct char_data *ch, const char *argument, int cmd);

int do_land(struct char_data *ch, const char *argument, int cmd);
int do_invis_off(struct char_data *ch, const char *argument, int cmd);

#endif

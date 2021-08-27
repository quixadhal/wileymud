#ifndef _ACT_OTHER_H
#define _ACT_OTHER_H

#define RANGER_CAST_LEVEL 10

int do_gain(struct char_data *ch, const char *argument, int cmd);
int do_guard(struct char_data *ch, const char *argument, int cmd);
int do_junk(struct char_data *ch, const char *argument, int cmd);
int do_qui(struct char_data *ch, const char *argument, int cmd);
int do_title(struct char_data *ch, const char *argument, int cmd);
int do_quit(struct char_data *ch, const char *argument, int cmd);
int do_save(struct char_data *ch, const char *argument, int cmd);
int do_not_here(struct char_data *ch, const char *argument, int cmd);
int do_sneak(struct char_data *ch, const char *argument, int cmd);
int do_hide(struct char_data *ch, const char *argument, int cmd);
int do_steal(struct char_data *ch, const char *argument, int cmd);
int do_practice(struct char_data *ch, const char *arg, int cmd);
int do_idea(struct char_data *ch, const char *argument, int cmd);
int do_typo(struct char_data *ch, const char *argument, int cmd);
int do_autoexit(struct char_data *ch, const char *argument, int cmd);
int do_bug(struct char_data *ch, const char *argument, int cmd);
int do_brief(struct char_data *ch, const char *argument, int cmd);
int do_compact(struct char_data *ch, const char *argument, int cmd);
int do_group(struct char_data *ch, const char *argument, int cmd);
int do_quaff(struct char_data *ch, const char *argument, int cmd);
int do_recite(struct char_data *ch, const char *argument, int cmd);
int do_use(struct char_data *ch, const char *argument, int cmd);
int do_plr_noshout(struct char_data *ch, const char *argument, int cmd);
int do_plr_notell(struct char_data *ch, const char *argument, int cmd);
int do_plr_nosummon(struct char_data *ch, const char *argument, int cmd);
int do_plr_noteleport(struct char_data *ch, const char *argument, int cmd);

#endif

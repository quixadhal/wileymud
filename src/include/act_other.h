#ifndef _ACT_OTHER_H
#define _ACT_OTHER_H

#define RANGER_CAST_LEVEL 10

void                                    do_gain(struct char_data *ch, const char *argument, int cmd);
void                                    do_guard(struct char_data *ch, const char *argument, int cmd);
void                                    do_junk(struct char_data *ch, const char *argument, int cmd);
void                                    do_qui(struct char_data *ch, const char *argument, int cmd);
void                                    do_title(struct char_data *ch, const char *argument, int cmd);
void                                    do_quit(struct char_data *ch, const char *argument, int cmd);
void                                    do_save(struct char_data *ch, const char *argument, int cmd);
void                                    do_not_here(struct char_data *ch, const char *argument,
						    int cmd);
void                                    do_sneak(struct char_data *ch, const char *argument, int cmd);
void                                    do_hide(struct char_data *ch, const char *argument, int cmd);
void                                    do_steal(struct char_data *ch, const char *argument, int cmd);
void                                    do_practice(struct char_data *ch, const char *arg, int cmd);
void                                    do_idea(struct char_data *ch, const char *argument, int cmd);
void                                    do_typo(struct char_data *ch, const char *argument, int cmd);
void                                    do_autoexit(struct char_data *ch, const char *argument, int cmd);
void                                    do_bug(struct char_data *ch, const char *argument, int cmd);
void                                    do_brief(struct char_data *ch, const char *argument, int cmd);
void                                    do_compact(struct char_data *ch, const char *argument,
						   int cmd);
void                                    do_group(struct char_data *ch, const char *argument, int cmd);
void                                    do_quaff(struct char_data *ch, const char *argument, int cmd);
void                                    do_recite(struct char_data *ch, const char *argument,
						  int cmd);
void                                    do_use(struct char_data *ch, const char *argument, int cmd);
void                                    do_plr_noshout(struct char_data *ch, const char *argument,
						       int cmd);
void                                    do_plr_notell(struct char_data *ch, const char *argument,
						      int cmd);
void                                    do_plr_nosummon(struct char_data *ch, const char *argument,
							int cmd);
void                                    do_plr_noteleport(struct char_data *ch, const char *argument,
							  int cmd);

#endif

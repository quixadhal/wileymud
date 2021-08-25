#ifndef _MAGIC_UTILS_H
#define _MAGIC_UTILS_H

void SwitchStuff(struct char_data *giver, struct char_data *taker);
void FailCharm(struct char_data *victim, struct char_data *ch);
void FailSleep(struct char_data *victim, struct char_data *ch);
void FailPara(struct char_data *victim, struct char_data *ch);
void FailCalm(struct char_data *victim, struct char_data *ch);
void AreaEffectSpell(struct char_data *castor, int dam, int spell_type, int zflag, const char *zone_mesg);

#endif

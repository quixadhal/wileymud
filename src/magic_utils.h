#ifndef _MAGIC_UTILS_H
#define _MAGIC_UTILS_H

SwitchStuff( struct char_data *giver, struct char_data *taker);
FailCharm(struct char_data *victim, struct char_data *ch);
FailSleep(struct char_data *victim, struct char_data *ch);
FailPara(struct char_data *victim, struct char_data *ch);
FailCalm(struct char_data *victim, struct char_data *ch);
void AreaEffectSpell(struct char_data *castor,int dam,int spell_type,int zflag,char *zone_mesg);

#endif

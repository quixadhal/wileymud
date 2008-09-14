#ifndef _OPINION_H
#define _OPINION_H

 void                             FreeHates(struct char_data *ch);
 void                             FreeFears(struct char_data *ch);
int                                     RemHated(struct char_data *ch, struct char_data *pud);
int                                     AddHated(struct char_data *ch, struct char_data *pud);
int                                     AddHatred(struct char_data *ch, int parm_type,
						  int parm);
 int                              RemHatred(struct char_data *ch, unsigned short bitv);
int                                     RemFeared(struct char_data *ch, struct char_data *pud);
int                                     AddFeared(struct char_data *ch, struct char_data *pud);
int                                     AddFears(struct char_data *ch, int parm_type, int parm);
 void                             ZeroHatred(struct char_data *ch, struct char_data *v);
 void                             ZeroFeared(struct char_data *ch, struct char_data *v);
int                                     DoesHate(struct char_data *ch, struct char_data *pud);
 int                              CanHate(struct char_data *ch, struct char_data *pud);
int                                     DoesFear(struct char_data *ch, struct char_data *pud);
 int                              CanFear(struct char_data *ch, struct char_data *pud);
struct char_data                       *FindAHatee(struct char_data *ch);
struct char_data                       *FindAFearee(struct char_data *ch);
 void                             DeleteHatreds(struct char_data *ch);
 void                             DeleteFears(struct char_data *ch);

#endif

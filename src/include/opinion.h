#ifndef _OPINION_H
#define _OPINION_H

inline void                             FreeHates(struct char_data *ch);
inline void                             FreeFears(struct char_data *ch);
int                                     RemHated(struct char_data *ch, struct char_data *pud);
int                                     AddHated(struct char_data *ch, struct char_data *pud);
int                                     AddHatred(struct char_data *ch, int parm_type,
						  int parm);
inline int                              RemHatred(struct char_data *ch, unsigned short bitv);
int                                     RemFeared(struct char_data *ch, struct char_data *pud);
int                                     AddFeared(struct char_data *ch, struct char_data *pud);
int                                     AddFears(struct char_data *ch, int parm_type, int parm);
inline void                             ZeroHatred(struct char_data *ch, struct char_data *v);
inline void                             ZeroFeared(struct char_data *ch, struct char_data *v);
int                                     DoesHate(struct char_data *ch, struct char_data *pud);
inline int                              CanHate(struct char_data *ch, struct char_data *pud);
int                                     DoesFear(struct char_data *ch, struct char_data *pud);
inline int                              CanFear(struct char_data *ch, struct char_data *pud);
struct char_data                       *FindAHatee(struct char_data *ch);
struct char_data                       *FindAFearee(struct char_data *ch);
inline void                             DeleteHatreds(struct char_data *ch);
inline void                             DeleteFears(struct char_data *ch);

#endif

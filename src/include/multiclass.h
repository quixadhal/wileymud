#ifndef _MULTICLASS_H
#define _MULTICLASS_H

int GetClassLevel(struct char_data *ch, int chclass);
int CountBits(int chclass);
int OnlyClass(struct char_data *ch, int chclass);
int HasClass(struct char_data *ch, int chclass);
int HowManyClasses(struct char_data *ch);
int BestFightingClass(struct char_data *ch);
int BestThiefClass(struct char_data *ch);
int BestMagicClass(struct char_data *ch);
int GetSecMaxLev(struct char_data *ch);
int GetALevel(struct char_data *ch, int which);
int GetThirdMaxLev(struct char_data *ch);
int GetMaxLevel(struct char_data *ch);
int GetMinLevel(struct char_data *ch);
int GetTotLevel(struct char_data *ch);
void StartLevels(struct char_data *ch);
int BestClass(struct char_data *ch);

#endif

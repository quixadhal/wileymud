#ifndef _RANDOM_H
#define _RANDOM_H

void random_death_message(struct char_data *ch, struct char_data *victim);
void random_error_message(struct char_data *ch);
void random_miscast(struct char_data *ch, const char *name);
void random_magic_failure(struct char_data *ch);

#endif

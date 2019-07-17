#ifndef _RENT_H
#define _RENT_H

#ifndef _REBOOT_C
extern float    RENT_RATE;
extern int      RENT_ON;
#endif

void setup_rent_table(void);
void load_rent(void);
int toggle_rent(struct char_data *ch);
int set_rent(struct char_data *ch, float factor);

#endif


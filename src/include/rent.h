#ifndef _RENT_H
#define _RENT_H

struct rent_data
{
    time_t updated;
    int enabled;
    float factor;
    char set_by[MAX_INPUT_LENGTH];
};

#ifndef _RENT_C
extern struct rent_data rent;
#endif

void setup_rent_table(void);
void load_rent(void);
int toggle_rent(struct char_data *ch);
int set_rent(struct char_data *ch, float factor);

#endif

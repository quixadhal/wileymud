#ifndef _BREATH_WEAPONS_H
#define _BREATH_WEAPONS_H

struct breath_victim
{
    struct char_data *ch;
    int yesno; /* 1 0 */
    struct breath_victim *next;
};

#ifndef _BREATH_WEAPONS_C
/* static funcp breaths[]; */
extern struct breather breath_monsters[];
#endif

struct breath_victim *choose_victims(struct char_data *ch, struct char_data *first_victim);
void free_victims(struct breath_victim *head);
int breath_weapon(struct char_data *ch, struct char_data *target, int mana_cost, bfuncp func);
void use_breath_weapon(struct char_data *ch, struct char_data *target, int cost, bfuncp func);
int BreathWeapon(struct char_data *ch, int cmd, const char *arg);

#endif

#ifndef _FIGHT_H
#define _FIGHT_H

#define MAX_NPC_CORPSE_TIME 5
#define MAX_PC_CORPSE_TIME 10

#ifndef _FIGHT_C
extern struct char_data *combat_list;
extern struct char_data *combat_next_dude;
extern struct attack_hit_type attack_hit_text[];
#endif

void appear(struct char_data *ch);
void load_messages(void);
void update_pos( struct char_data *victim );
int check_peaceful(struct char_data *ch, char *msg);
void set_fighting(struct char_data *ch, struct char_data *vict);
void stop_fighting(struct char_data *ch);
void make_corpse(struct char_data *ch);
void change_alignment(struct char_data *ch, struct char_data *victim);
void death_cry(struct char_data *ch);
void raw_kill(struct char_data *ch);
void die(struct char_data *ch);
void group_gain(struct char_data *ch, struct char_data *victim);
char *replace_string(char *str, char *weapon, char *weapon_s);
void dam_message(int dam, struct char_data *ch, struct char_data *victim, int w_type);
int damage(struct char_data *ch, struct char_data *victim,int dam, int attacktype);
void hit(struct char_data *ch, struct char_data *victim, int type);
void perform_violence(int pulse);
struct char_data *FindVictim( struct char_data *ch);
struct char_data *FindAnyVictim( struct char_data *ch);
int PreProcDam(struct char_data *ch, int type, int dam);
int DamageOneItem( struct char_data *ch, int dam_type, struct obj_data *obj);
void MakeScrap( struct char_data *ch, struct obj_data *obj);
void DamageAllStuff( struct char_data *ch, int dam_type);
int DamageItem(struct char_data *ch, struct obj_data *o, int num);
int ItemSave( struct obj_data *i, int dam_type) ;
int DamagedByAttack( struct obj_data *i, int dam_type);
int WeaponCheck(struct char_data *ch, struct char_data *v, int type, int dam);
int DamageStuff(struct char_data *v, int type, int dam);
int GetItemDamageType( int type);
int SkipImmortals(struct char_data *v, int amnt);
int CheckKill(struct char_data *ch,struct char_data *vict);
int WeaponSpell( struct char_data *c, struct char_data *v, int type);
struct char_data *FindAnAttacker(struct char_data *ch) ;
#if 0
void shoot( struct char_data *ch, struct char_data *victim);
#endif
int SwitchTargets( struct char_data *ch, struct char_data *vict);

#endif

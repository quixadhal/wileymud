#ifndef _SPELL_PARSER_H
#define _SPELL_PARSER_H

#define MANA_MU 1
#define MANA_CL 1

#define SPELLO(nr, beat, pos, mlev, clev, mana, tar, func) { \
               spell_info[nr].spell_pointer = (func);    \
               spell_info[nr].beats = (beat);            \
               spell_info[nr].minimum_position = (pos);  \
               spell_info[nr].min_usesmana = (mana);     \
               spell_info[nr].min_level_cleric = (clev); \
               spell_info[nr].min_level_magic = (mlev);  \
               spell_info[nr].targets = (tar);           \
        }

/* 100 is the MAX_MANA for a character */
#define USE_MANA(ch, sn)                            \
  MAX(spell_info[sn].min_usesmana,100/MAX(2,(2+GET_LEVEL(ch,BestMagicClass(ch))-SPELL_LEVEL(ch,sn))))

#ifndef _SPELL_PARSER_C
extern struct spell_info_type spell_info[MAX_SPL_LIST];
extern char *spells[];
extern const byte saving_throws[6][5][ABS_MAX_LVL];
#endif

int SPELL_LEVEL(struct char_data *ch, int sn) ;
void affect_update( void );
void clone_char(struct char_data *ch);
void clone_obj(struct obj_data *obj);
bool circle_follow(struct char_data *ch, struct char_data *victim);
void stop_follower(struct char_data *ch);
void die_follower(struct char_data *ch);
void add_follower(struct char_data *ch, struct char_data *leader);
say_spell( struct char_data *ch, int si );
bool saves_spell(struct char_data *ch, sh_int save_type);
bool ImpSaveSpell(struct char_data *ch, sh_int save_type, int mod);
char *skip_spaces(char *string);
void do_cast(struct char_data *ch, char *argument, int cmd);
void assign_spell_pointers(void);
int check_falling( struct char_data *ch);
void check_drowning( struct char_data *ch);
void check_falling_obj( struct obj_data *obj, int room);
int check_nature( struct char_data *i);

#endif

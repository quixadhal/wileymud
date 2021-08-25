#ifndef _SPELL_PARSER_H
#define _SPELL_PARSER_H

#define MANA_MU 1
#define MANA_CL 1

#define ASSIGN_SPELL(nr, cast, use, nam, func, dly, minmana, maxmana, tar, pos, glev, gcl, mlev, clev, wlev, tlev,     \
                     rlev, dlev)                                                                                       \
    {                                                                                                                  \
        spell_info[nr].castable = (cast);                                                                              \
        spell_info[nr].useable = (use);                                                                                \
        spell_info[nr].name = (nam);                                                                                   \
        spell_info[nr].spell_pointer = (func);                                                                         \
        spell_info[nr].delay = (dly);                                                                                  \
        spell_info[nr].min_mana = (minmana);                                                                           \
        spell_info[nr].max_mana = (maxmana);                                                                           \
        spell_info[nr].targets = (tar);                                                                                \
        spell_info[nr].minimum_position = (pos);                                                                       \
        spell_info[nr].generic_level = (glev);                                                                         \
        spell_info[nr].generic_classes = (gcl);                                                                        \
        spell_info[nr].min_level[MAGE_LEVEL_IND] = (mlev);                                                             \
        spell_info[nr].min_level[CLERIC_LEVEL_IND] = (clev);                                                           \
        spell_info[nr].min_level[WARRIOR_LEVEL_IND] = (wlev);                                                          \
        spell_info[nr].min_level[THIEF_LEVEL_IND] = (tlev);                                                            \
        spell_info[nr].min_level[RANGER_LEVEL_IND] = (rlev);                                                           \
        spell_info[nr].min_level[DRUID_LEVEL_IND] = (dlev);                                                            \
    }

/* 100 is the MAX_MANA for a character */
#if 0
#define USE_MANA(ch, sn)                                                                                               \
    MAX(spell_info[sn].min_usesmana, 100 / MAX(2, (2 + GET_LEVEL(ch, BestMagicClass(ch)) - SPELL_LEVEL(ch, sn))))
#endif
#define USE_MANA(ch, sn)                                                                                               \
    MAX(spell_info[sn].min_mana,                                                                                       \
        spell_info[sn].max_mana / MAX(1, (GET_LEVEL(ch, BestMagicClass(ch)) - SPELL_LEVEL(ch, sn)) + 1))

#ifndef _SPELL_PARSER_C
extern struct spell_info_type spell_info[MAX_SKILLS];

/* extern const char *spells[]; */
extern const char saving_throws[ABS_MAX_CLASS][MAX_SAVING_THROWS][ABS_MAX_LVL];

#endif

int GetSpellByName(const char *name);
int GetSkillByName(const char *name);
int SPELL_LEVEL(struct char_data *ch, int sn);
int SKILL_LEVEL(struct char_data *ch, int sn);
int CanCast(struct char_data *ch, int sn);
int CanCastClass(struct char_data *ch, int sn, int cl);
int CanUse(struct char_data *ch, int sn);
int CanUseClass(struct char_data *ch, int sn, int cl);
void affect_update(void);
void clone_char(struct char_data *ch);
void clone_obj(struct obj_data *obj);
char circle_follow(struct char_data *ch, struct char_data *victim);
void stop_follower(struct char_data *ch);
void die_follower(struct char_data *ch);
void add_follower(struct char_data *ch, struct char_data *leader);
void say_spell(struct char_data *ch, int si);
char saves_spell(struct char_data *ch, short int save_type);
char ImpSaveSpell(struct char_data *ch, short int save_type, int mod);
const char *skip_spaces(const char *string);
void do_cast(struct char_data *ch, const char *argument, int cmd);
void assign_spell_pointers(void);
int splat(struct char_data *ch, struct room_data *rp, int height);
int check_falling(struct char_data *ch);
int check_drowning(struct char_data *ch);
void check_falling_obj(struct obj_data *obj, int room);
int check_nature(struct char_data *i);
void check_all_nature(int current_pulse);

#endif

#ifndef _SPEC_PROCS_H
#define _SPEC_PROCS_H

#define SPECIAL_MOB	1
#define SPECIAL_OBJ	2
#define SPECIAL_ROOM	3

struct social_type {
  char *cmd;
  int next_line;
};

struct breath_victim {
  struct char_data *ch;
  int yesno;			       /* 1 0 */
  struct breath_victim *next;
};

struct special_proc_entry {
  int vnum;
  int (*proc) (struct char_data *, int, char *);
  char *name;
};

#ifndef _SPEC_PROCS_C
struct special_proc_entry specials_m[];
struct special_proc_entry specials_o[];
struct special_proc_entry specials_r[];

/* static funcp breaths[]; */
extern struct breather breath_monsters[];

/* static char *elf_comm[]; */
#endif

int is_target_room_p(int room, void *tgt_room);
int named_object_on_ground(int room, void *c_data);
char *how_good(int percent);
int GainLevel(struct char_data *ch, int class);
struct char_data *FindMobInRoomWithFunction(int room, int (*func) ());
int MageGuildMaster(struct char_data *ch, int cmd, char *arg);
int ClericGuildMaster(struct char_data *ch, int cmd, char *arg);
int ThiefGuildMaster(struct char_data *ch, int cmd, char *arg);
int FighterGuildMaster(struct char_data *ch, int cmd, char *arg);
int dump(struct char_data *ch, int cmd, char *arg);

#if 1
int mayor(struct char_data *ch, int cmd, char *arg);

#endif
struct char_data *find_mobile_here_with_spec_proc(int (*fcn) (), int rnumber);

#if 1
void exec_social(struct char_data *npc, char *cmd, int next_line, int *cur_line, void **thing);

#endif
void npc_steal(struct char_data *ch, struct char_data *victim);
int snake(struct char_data *ch, int cmd, char *arg);

#if 1
int ninja_master(struct char_data *ch, int cmd, char *arg);

#define PGShield 25100
int PaladinGuildGuard(struct char_data *ch, int cmd, char *arg);
int AbyssGateKeeper(struct char_data *ch, int cmd, char *arg);

#endif
int blink(struct char_data *ch, int cmd, char *arg);

#define NUT_NUMBER 1130
#define NUT_CRACKED_NUMBER 1131
int Ned_Nutsmith(struct char_data *ch, int cmd, char *arg);
int RepairGuy(struct char_data *ch, int cmd, char *arg);
int citizen(struct char_data *ch, int cmd, char *arg);
int shylar_guard(struct char_data *ch, int cmd, char *arg);
int ghoul(struct char_data *ch, int cmd, char *arg);

#if 1
int WizardGuard(struct char_data *ch, int cmd, char *arg);

#endif
int vampire(struct char_data *ch, int cmd, char *arg);
int wraith(struct char_data *ch, int cmd, char *arg);
int shadow(struct char_data *ch, int cmd, char *arg);

#if 1
int geyser(struct char_data *ch, int cmd, char *arg);
int green_slime(struct char_data *ch, int cmd, char *arg);

#endif
struct breath_victim *choose_victims(struct char_data *ch, struct char_data *first_victim);
void free_victims(struct breath_victim *head);
int breath_weapon(struct char_data *ch, struct char_data *target, int mana_cost, void (*func) ());
int use_breath_weapon(struct char_data *ch, struct char_data *target, int cost, void (*func) ());
int BreathWeapon(struct char_data *ch, int cmd, char *arg);
int DracoLich(struct char_data *ch, int cmd, char *arg);
int Drow(struct char_data *ch, int cmd, char *arg);
int Leader(struct char_data *ch, int cmd, char *arg);
int thief(struct char_data *ch, int cmd, char *arg);
int magic_user(struct char_data *ch, int cmd, char *arg);
int cleric(struct char_data *ch, int cmd, char *arg);

#if 1
int guild_guard(struct char_data *ch, int cmd, char *arg);

#endif
int puff(struct char_data *ch, int cmd, char *arg);
int regenerator(struct char_data *ch, int cmd, char *arg);
int replicant(struct char_data *ch, int cmd, char *arg);

#if 1
#define TYT_NONE 0
#define TYT_CIT  1
#define TYT_WHAT 2
#define TYT_TELL 3
#define TYT_HIT  4
int Tytan(struct char_data *ch, int cmd, char *arg);
int AbbarachDragon(struct char_data *ch, int cmd, char *arg);

#endif
int fido(struct char_data *ch, int cmd, char *arg);
int janitor(struct char_data *ch, int cmd, char *arg);
int janitor_eats(struct char_data *ch, int cmd, char *arg);

#if 1
int tormentor(struct char_data *ch, int cmd, char *arg);

#endif
int Fighter(struct char_data *ch, int cmd, char *arg);

#if 1
int RustMonster(struct char_data *ch, int cmd, char *arg);
int temple_labrynth_liar(struct char_data *ch, int cmd, char *arg);
int temple_labrynth_sentry(struct char_data *ch, int cmd, char *arg);

#define WW_LOOSE 0
#define WW_FOLLOW 1
int Whirlwind(struct char_data *ch, int cmd, char *arg);

#endif
#define NN_LOOSE  0
#define NN_FOLLOW 1
#define NN_STOP   2
int NudgeNudge(struct char_data *ch, int cmd, char *arg);

#if 1
int AGGRESSIVE(struct char_data *ch, int cmd, char *arg);

#endif
int cityguard(struct char_data *ch, int cmd, char *arg);

#if 1
#define ZM_MANA	10
#define ZM_NEMESIS 1204
int WarrenGuard(struct char_data *ch, int cmd, char *arg);
int zm_tired(struct char_data *zmaster);
int zm_stunned_followers(struct char_data *zmaster);
void zm_zap_spell_at(struct char_data *ch, struct char_data *vict, int maxlevel);
void zm_zap_area_at(struct char_data *ch, int maxlevel);
zm_init_combat(struct char_data *zmaster, struct char_data *target);
int zm_kill_fidos(struct char_data *zmaster);
int zm_kill_aggressor(struct char_data *zmaster);
int zm_kill_wimps(struct char_data *zmaster);
int zombie_master(struct char_data *ch, int cmd, char *arg);

#endif
int pet_shops(struct char_data *ch, int cmd, char *arg);
int bank(struct char_data *ch, int cmd, char *arg);
int pray_for_items(struct char_data *ch, int cmd, char *arg);

#if 1
#define CHAL_ACT \
"You are torn out of reality!\n\r" \
"You roll and tumble through endless voids for what seems like eternity...\n\r" \
"\n\r" \
"After a time, a new reality comes into focus... you are elsewhere.\n\r"
int chalice(struct char_data *ch, int cmd, char *arg);
int kings_hall(struct char_data *ch, int cmd, char *arg);

#endif
#define IS_DIR    (real_roomp(q_head->room_nr)->dir_option[i])
#define GO_OK  (!IS_SET(IS_DIR->exit_info,EX_CLOSED)\
		 && (IS_DIR->to_room != NOWHERE))
#define GO_OK_SMARTER  (!IS_SET(IS_DIR->exit_info,EX_LOCKED)\
		 && (IS_DIR->to_room != NOWHERE))
/* static void donothing(); */
int find_path(int in_room, int (*predicate) (), void *c_data, int depth);
int choose_exit(int in_room, int tgt_room, int depth);
int go_direction(struct char_data *ch, int dir);
int House(struct char_data *ch, int cmd, char *arg);

#if 1
int paramedics(struct char_data *ch, int cmd, char *arg);
int jugglernaut(struct char_data *ch, int cmd, char *arg);
int delivery_beast(struct char_data *ch, int cmd, char *arg);
int StormGiant(struct char_data *ch, int cmd, char *arg);
int firenewt(struct char_data *ch, int cmd, char *arg);

#endif
int eli_priest(struct char_data *ch, int cmd, char *arg);
int fountain(struct char_data *ch, int cmd, char *arg);
int RangerGuildMaster(struct char_data *ch, int cmd, char *arg);
int do_skills(struct char_data *ch, int cmd, char *arg);
int GenericGuildMaster(struct char_data *ch, int cmd, char *arg);
int mosquito(struct char_data *ch, int cmd, char *arg);
int BerserkerAxe(struct char_data *ch, int cmd, char *arg);
void assign_mobiles(void);
void assign_objects(void);
void assign_rooms(void);
char *name_special_proc(int type, int vnum);

#endif

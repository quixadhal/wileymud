#ifndef _SPEC_PROCS_H
#define _SPEC_PROCS_H

#define SPECIAL_MOB 1
#define SPECIAL_OBJ 2
#define SPECIAL_ROOM 3

struct social_type
{
    const char *cmd;
    int next_line;
};

struct special_proc_entry
{
    int vnum;
    ifuncp proc;
    const char *name;
};

#ifndef _SPEC_PROCS_C
extern struct special_proc_entry *specials_m;
extern struct special_proc_entry *specials_o;
extern struct special_proc_entry *specials_r;
#endif

char *how_good(int percent_known);
int GainLevel(struct char_data *guildmaster, struct char_data *ch, int chclass);
struct char_data *FindMobInRoomWithFunction(int room, ifuncp func);
int MageGuildMaster(struct char_data *ch, int cmd, const char *arg);
int ClericGuildMaster(struct char_data *ch, int cmd, const char *arg);
int ThiefGuildMaster(struct char_data *ch, int cmd, const char *arg);
int FighterGuildMaster(struct char_data *ch, int cmd, const char *arg);
int dump(struct char_data *ch, int cmd, const char *arg);
int mayor(struct char_data *ch, int cmd, const char *arg);
struct char_data *find_mobile_here_with_spec_proc(ifuncp fcn, int rnumber);
void exec_social(struct char_data *npc, const char *cmd, int next_line, int *cur_line, void **thing);
void npc_steal(struct char_data *ch, struct char_data *victim);
int snake(struct char_data *ch, int cmd, const char *arg);
int ninja_master(struct char_data *ch, int cmd, const char *arg);

#define PGShield 25100
int PaladinGuildGuard(struct char_data *ch, int cmd, const char *arg);
int AbyssGateKeeper(struct char_data *ch, int cmd, const char *arg);
int blink(struct char_data *ch, int cmd, const char *arg);

#define NUT_NUMBER 1130
#define NUT_CRACKED_NUMBER 1131
int Ned_Nutsmith(struct char_data *ch, int cmd, const char *arg);
int RepairGuy(struct char_data *ch, int cmd, const char *arg);
int citizen(struct char_data *ch, int cmd, const char *arg);
int shylar_guard(struct char_data *ch, int cmd, const char *arg);
int ghoul(struct char_data *ch, int cmd, const char *arg);
int WizardGuard(struct char_data *ch, int cmd, const char *arg);
int vampire(struct char_data *ch, int cmd, const char *arg);
int wraith(struct char_data *ch, int cmd, const char *arg);
int shadow(struct char_data *ch, int cmd, const char *arg);
int geyser(struct char_data *ch, int cmd, const char *arg);
int green_slime(struct char_data *ch, int cmd, const char *arg);
int DracoLich(struct char_data *ch, int cmd, const char *arg);
int Drow(struct char_data *ch, int cmd, const char *arg);
int Leader(struct char_data *ch, int cmd, const char *arg);
int thief(struct char_data *ch, int cmd, const char *arg);
int magic_user(struct char_data *ch, int cmd, const char *arg);
int cleric(struct char_data *ch, int cmd, const char *arg);
int guild_guard(struct char_data *ch, int cmd, const char *arg);
int puff(struct char_data *ch, int cmd, const char *arg);
int regenerator(struct char_data *ch, int cmd, const char *arg);
int replicant(struct char_data *ch, int cmd, const char *arg);

#define TYT_NONE 0
#define TYT_CIT 1
#define TYT_WHAT 2
#define TYT_TELL 3
#define TYT_HIT 4
int Tytan(struct char_data *ch, int cmd, const char *arg);
int AbbarachDragon(struct char_data *ch, int cmd, const char *arg);
int fido(struct char_data *ch, int cmd, const char *arg);
int janitor(struct char_data *ch, int cmd, const char *arg);
int janitor_eats(struct char_data *ch, int cmd, const char *arg);
int tormentor(struct char_data *ch, int cmd, const char *arg);
int Fighter(struct char_data *ch, int cmd, const char *arg);
int RustMonster(struct char_data *ch, int cmd, const char *arg);
int temple_labrynth_liar(struct char_data *ch, int cmd, const char *arg);
int temple_labrynth_sentry(struct char_data *ch, int cmd, const char *arg);
#define WW_LOOSE 0
#define WW_FOLLOW 1
int Whirlwind(struct char_data *ch, int cmd, const char *arg);

#define NN_LOOSE 0
#define NN_FOLLOW 1
#define NN_STOP 2
int NudgeNudge(struct char_data *ch, int cmd, const char *arg);
int AGGRESSIVE(struct char_data *ch, int cmd, const char *arg);
int cityguard(struct char_data *ch, int cmd, const char *arg);

#define ZM_MANA 10
#define ZM_NEMESIS 1204
int WarrenGuard(struct char_data *ch, int cmd, const char *arg);
int zm_tired(struct char_data *zmaster);
int zm_stunned_followers(struct char_data *zmaster);
void zm_zap_spell_at(struct char_data *ch, struct char_data *vict, int maxlevel);
void zm_zap_area_at(struct char_data *ch, int maxlevel);
void zm_init_combat(struct char_data *zmaster, struct char_data *target);
int zm_kill_fidos(struct char_data *zmaster);
int zm_kill_aggressor(struct char_data *zmaster);
int zm_kill_wimps(struct char_data *zmaster);
int zombie_master(struct char_data *ch, int cmd, const char *arg);
int pet_shops(struct char_data *ch, int cmd, const char *arg);
int bank(struct char_data *ch, int cmd, const char *arg);
int pray_for_items(struct char_data *ch, int cmd, const char *arg);
#define CHAL_ACT                                                                                                       \
    "You are torn out of reality!\r\n"                                                                                 \
    "You roll and tumble through endless voids for what seems like eternity...\r\n"                                    \
    "\r\n"                                                                                                             \
    "After a time, a new reality comes into focus... you are elsewhere.\r\n"
int chalice(struct char_data *ch, int cmd, const char *arg);
int kings_hall(struct char_data *ch, int cmd, const char *arg);
int House(struct char_data *ch, int cmd, const char *arg);
int paramedics(struct char_data *ch, int cmd, const char *arg);
int jugglernaut(struct char_data *ch, int cmd, const char *arg);
int delivery_beast(struct char_data *ch, int cmd, const char *arg);
int StormGiant(struct char_data *ch, int cmd, const char *arg);
int firenewt(struct char_data *ch, int cmd, const char *arg);
int eli_priest(struct char_data *ch, int cmd, const char *arg);
int fountain(struct char_data *ch, int cmd, const char *arg);
int RangerGuildMaster(struct char_data *ch, int cmd, const char *arg);
int GenericGuildMaster(struct char_data *ch, int cmd, const char *arg);
int mosquito(struct char_data *ch, int cmd, const char *arg);
int BerserkerAxe(struct char_data *ch, int cmd, const char *arg);
void assign_mobiles(void);
void assign_objects(void);
void assign_rooms(void);
const char *name_special_proc(int type, int vnum);
void gm_wrong_class(struct char_data *master, struct char_data *vict);
void gm_wrong_alignment(struct char_data *master, struct char_data *vict);
void gm_gain(struct char_data *master, struct char_data *vict, int target);
void gm_prac(struct char_data *master, struct char_data *vict, int target, const char *arg);
int GuildMaster(struct char_data *ch, int cmd, const char *arg);
int ThePerch(struct char_data *ch, int cmd, const char *arg);
int k_tired(struct char_data *karrn);
int k_kill_aggressor(struct char_data *karrn);
int k_kill_wimps(struct char_data *karrn);
int Karrn(struct char_data *ch, int cmd, const char *arg);
int Tate_ThiefGuildMaster(struct char_data *ch, int cmd, const char *arg);
char *MobFunctionNameByFunc(ifuncp func);

#endif

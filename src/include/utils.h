/*
 * file: utils.h, Utility module.                         Part of DIKUMUD
 * Usage: Utility macros
 */

#ifndef _UTILS_H
#define _UTILS_H

#define GUARD_VNUM 3060
#define GUARD2_VNUM 3069

#define EXP_NEEDED(ch,cl) ((titles[cl][(int)GET_LEVEL(ch,cl)+1].exp / (IS_PC(ch)?1:20)) - GET_EXP(ch))
#define GOLD_NEEDED(ch,cl) (IS_PC(ch)?((3*(16<<((int)GET_LEVEL(ch,cl)+1)/3)+((16<<((int)GET_LEVEL(ch,cl)+1)/3)*(((int)GET_LEVEL(ch,cl)+1)%3))) / (((int)GET_LEVEL(ch,cl)<8)?(((int)GET_LEVEL(ch,cl)<4)?8:2):1)):0)

#define NAME(ch) ((ch)?(IS_NPC(ch)?(ch)->player.short_descr:(ch)->player.name):"")
#define SOME_NAME(ch) ((ch)?(IS_NPC(ch)?(ch)->player.short_descr:(ch)->player.name):"(someone)")
#define SOME_OBJ(obj) (((obj)&&((obj)->name))?((obj)->name):"(something)")
#define SOME_ROOM(ch) (((ch)&&((ch)->in_room)>-1)?((real_roomp((ch)->in_room))->name):"(somewhere)")
#ifndef MIN
#define MIN(a,b) ((a)<=(b)?(a):(b))
#define MAX(a,b) ((a)>=(b)?(a):(b))
#endif

#define VNULL(s) ((s)?(s):"(null)")
#define ENULL(s) ((s)?(s):"")
#define BOOLSTR(s) ((s)?"true":"false")
#define ENABLED(s) ((s)?"enabled":"disabled")

 int                              str_cmp(const char *arg1, const char *arg2);
 int                              strn_cmp(const char *arg1, const char *arg2,
						 const int n);
#define LOWER(c) (((c)>='A'  && (c) <= 'Z') ? ((c)+('a'-'A')) : (c))
#define UPPER(c) (((c)>='a'  && (c) <= 'z') ? ((c)+('A'-'a')) : (c))
#define ISNEWL(ch) ((ch) == '\n' || (ch) == '\r')
#define IF_STR(st) ((st) ? (st) : "\0")
#define CAP(st)  (*(st) = UPPER(*(st)), st)

/* log_error(((failure) && *(failure))?(failure):"calloc failure"); */

#define FCLOSE(fp) \
do {               \
  if((fp))         \
    fclose(fp);    \
  fp = NULL;       \
} while(0)

#define CREATE(result, type, number)					\
do									\
{									\
  if (!((result) = (type *) calloc ((number), sizeof(type))))		\
  {									\
    perror("calloc failure");						\
    fprintf(stderr, "Calloc failure @ %s:%d\n", __FILE__, __LINE__ );	\
    fflush(stderr);                                                     \
    proper_exit(42);							\
  }									\
} while(0)

#define CREATE_VOID(result, type, number)				\
do									\
{									\
  if (!((result) = (void *) calloc ((number), sizeof(type))))		\
  {									\
    perror("calloc failure");						\
    fprintf(stderr, "Calloc failure @ %s:%d\n", __FILE__, __LINE__ );	\
    fflush(stderr);                                                     \
    proper_exit(42);							\
  }									\
} while(0)

#define STRDUP(result, string)						\
do									\
{									\
  if (!((result) = (char *) calloc (strlen(string)+1, sizeof(char))))	\
  {									\
    perror("calloc failure");						\
    fprintf(stderr, "Calloc failure @ %s:%d\n", __FILE__, __LINE__ );	\
    fflush(stderr);                                                     \
    proper_exit(42);							\
  }									\
  strcpy((result), (string));						\
} while(0)

#define RECREATE(result,type,number)					\
do									\
{									\
  if(!((result) = (type *)realloc((result), sizeof(type) * (number))))	\
  {									\
    perror("realloc failure");						\
    fprintf(stderr, "Realloc failure @ %s:%d\n", __FILE__, __LINE__ );	\
    fflush(stderr);                                                     \
    proper_exit(42);							\
  }									\
} while(0)

#define DESTROY(point)							\
do									\
{									\
  if((point))								\
  {									\
    free((point));							\
    (point) = NULL;							\
  }									\
} while(0)

/* double-linked list handling macros -Thoric ( From the Smaug codebase ) */
/* Updated by Scion 8/6/1999 */
#define LINK(link, first, last, next, prev)  \
do                                              \
{                                               \
   if ( !(first) )                              \
   {                                            \
      (first) = (link);                         \
      (last) = (link);                          \
   }                                            \
   else                                         \
      (last)->next = (link);                    \
   (link)->next = NULL;                         \
   if ((first) == (link))                       \
      (link)->prev = NULL;                      \
   else                                         \
      (link)->prev = (last);                    \
   (last) = (link);                             \
} while(0)

#define HEADLINK(link, first, last, next, prev) \
do                                              \
{                                               \
   if ( !(first) ) {                            \
      (first) = (link);                         \
      (last) = (link);                          \
      (link)->next = NULL;                      \
   } else {                                     \
      (link)->next = (first);                   \
      (first) = (link);                         \
   }                                            \
   (link)->prev = NULL;                         \
} while(0)

#define INSERT(link, insert, first, next, prev)    \
do                                                    \
{                                                     \
   (link)->prev = (insert)->prev;                     \
   if ( !(insert)->prev )                             \
      (first) = (link);                               \
   else                                               \
      (insert)->prev->next = (link);                  \
   (insert)->prev = (link);                           \
   (link)->next = (insert);                           \
} while(0)

#define UNLINK(link, first, last, next, prev) \
do                                               \
{                                                \
   if ( !(link)->prev )                          \
   {                                             \
      (first) = (link)->next;                    \
	if((first))                                \
	   (first)->prev = NULL;                   \
   }                                             \
   else                                          \
   {                                             \
      (link)->prev->next = (link)->next;         \
   }                                             \
   if( !(link)->next )                           \
   {                                             \
      (last) = (link)->prev;                     \
	if((last))                                 \
	   (last)->next = NULL;                    \
   }                                             \
   else                                          \
   {                                             \
      (link)->next->prev = (link)->prev;         \
   }                                             \
} while(0)

#define IS_SET(flag,bit)  ((flag) & (bit))
#define IS_NOT_SET(flag,bit)  (!IS_SET(flag,bit))
#define IS_AFFECTED(ch,skill) ( IS_SET((ch)->specials.affected_by, (skill)) )
#define IS_DARK(room)  (!real_roomp(room)->light && IS_SET(real_roomp(room)->room_flags, DARK))
#define IS_LIGHT(room)  (real_roomp(room)->light || !IS_SET(real_roomp(room)->room_flags, DARK))
#define NO_MOON (time_info.hours == 4 || time_info.hours == 5 || time_info.hours == 19 || time_info.hours == 20)
#define FULL_MOON ((weather_info.moon >= 12) && (weather_info.moon < 20))
#define STORMY (weather_info.sky > SKY_CLOUDY)
#define IS_LIGHTOUT(room) ((!IS_SET(real_roomp(room)->room_flags, INDOORS) && ((weather_info.sunlight > SUN_DARK) || (FULL_MOON && !STORMY && !NO_MOON))) || real_roomp(room)->light)
#define IS_DARKOUT(room)  ((!IS_SET(real_roomp(room)->room_flags, INDOORS) && ((weather_info.sunlight == SUN_DARK) && !(FULL_MOON && !STORMY && !NO_MOON))) && !(real_roomp(room)->light))
#define BRIGHT_MOON(room) ((weather_info.moon >= 5) && (weather_info.moon < 28) && !NO_MOON)
#define IS_PC(ch)  (!IS_SET((ch)->specials.act, ACT_ISNPC))
#define IS_NPC(ch)  (IS_SET((ch)->specials.act, ACT_ISNPC))
#define IS_MOB(ch)  (IS_SET((ch)->specials.act, ACT_ISNPC) && ((ch)->nr >-1))
#define SWITCH(a,b) { (a) ^= (b); \
                      (b) ^= (a); \
                      (a) ^= (b); }
#define SET_BIT(var,bit)  ((var) = (var) | (bit))
#define REMOVE_BIT(var,bit)  ((var) = (var) & ~(bit) )
#define RM_FLAGS(i)  ((real_roomp(i))?real_roomp(i)->room_flags:0)
#define HSHR(ch) ((ch)->player.sex ?					\
	(((ch)->player.sex == 1) ? "his" : "her") : "its")
#define HSSH(ch) ((ch)->player.sex ?					\
	(((ch)->player.sex == 1) ? "he" : "she") : "it")
#define HMHR(ch) ((ch)->player.sex ? 					\
	(((ch)->player.sex == 1) ? "him" : "her") : "it")
#define ANA(obj) (index("aeiouyAEIOUY", *(obj)->name) ? "An" : "A")
#define SANA(obj) (index("aeiouyAEIOUY", *(obj)->name) ? "an" : "a")
#define A_AN(str) (index("aeiouyAEIOUY", *(str)) ? "An" : "A")
#define SA_AN(str) (index("aeiouyAEIOUY", *(str)) ? "an" : "a")
#define MOUNTED(ch)  ((ch)->specials.mounted_on)
#define RIDDEN(ch)	((ch)->specials.ridden_by)
#define GET_ZONE(room)	(((room)>-1)?real_roomp((room))?real_roomp((room))->zone:-1:-1)
#define GET_LEVEL(ch, i)   ((ch)->player.level[(int)(i)])
#define GET_CLASS_TITLE(ch, class, lev)   ((ch)->player.sex ?  \
   (((ch)->player.sex == 1) ? titles[(int)(class)][(int)(lev)].title_m : \
    titles[(int)(class)][(int)(lev)].title_f) : titles[(int)(class)][(int)(lev)].title_m)
#define GET_REQ(i) (i<2  ? "Awful" :(i<4  ? "Bad"     :(i<7  ? "Poor"      :\
(i<10 ? "Average" :(i<14 ? "Fair"    :(i<20 ? "Good"    :(i<24 ? "Very good" :\
        "Superb" )))))))
#define GET_POS(ch)     ((ch)->specials.position)
#define GET_COND(ch, i) ((ch)->specials.conditions[(int)(i)])
#define GET_NAME(ch)    ((ch)->player.name)
#define SAFE_NAME(ch)   (((ch)&&((ch)->player.name))?((ch)->player.name):"(someone)")
#define SAFE_ONAME(obj) (((obj)&&((obj)->name))?((obj)->name):"(something)")
#define GET_SDESC(ch)	((ch)->player.short_descr)
#define GET_TITLE(ch)   ((ch)->player.title)
#define GET_PRETITLE(ch)  ((ch)->player.pre_title)
#define GET_POOF_IN(ch)   ((ch)->player.poof_in)
#define GET_POOF_OUT(ch)  ((ch)->player.poof_out)
#define GET_CLASS(ch)   ((ch)->player.class)
#define GET_HOME(ch)	((ch)->player.hometown)
#define GET_AGE(ch)     (age(ch).year)
#define GET_STR(ch)     ((ch)->tmpabilities.str)
#define GET_ADD(ch)     ((ch)->tmpabilities.str_add)
#define GET_DEX(ch)     ((ch)->tmpabilities.dex)
#define GET_INT(ch)     ((ch)->tmpabilities.intel)
#define GET_WIS(ch)     ((ch)->tmpabilities.wis)
#define GET_CON(ch)     ((ch)->tmpabilities.con)
#define STRENGTH_APPLY_INDEX(ch) \
        ( ((GET_ADD(ch)==0) || (GET_STR(ch) != 18)) ? GET_STR(ch) :\
          (GET_ADD(ch) <= 50) ? 26 :( \
          (GET_ADD(ch) <= 75) ? 27 :( \
          (GET_ADD(ch) <= 90) ? 28 :( \
          (GET_ADD(ch) <= 99) ? 29 :  30 ) ) )                   \
        )
#define GET_AC(ch)      ((ch)->points.armor)
#define GET_HIT(ch)     ((ch)->points.hit)
#define GET_MAX_HIT(ch) (hit_limit(ch))
#define GET_MOVE(ch)    ((ch)->points.move)
#define GET_MAX_MOVE(ch) (move_limit(ch))
#define GET_MANA(ch)    ((ch)->points.mana)
#define GET_MAX_MANA(ch) (mana_limit(ch))
#define GET_GOLD(ch)    ((ch)->points.gold)
#define GET_BANK(ch)    ((ch)->points.bankgold)
#define GET_EXP(ch)     ((ch)->points.exp)
#define GET_PRIV(ch)	((ch)->points.wiz_priv)
#define GET_HEIGHT(ch)  ((ch)->player.height)
#define GET_WEIGHT(ch)  ((ch)->player.weight)
#define GET_SEX(ch)     ((ch)->player.sex)
#define GET_RACE(ch)     ((ch)->race)
#define GET_HITROLL(ch) ((ch)->points.hitroll)
#define GET_DAMROLL(ch) ((ch)->points.damroll)
#define GET_IDLE_TIME(ch)  (time(0) - ((ch)->desc->idle_time))
#define AWAKE(ch) (GET_POS(ch) > POSITION_SLEEPING)
#define WAIT_STATE(ch, cycle)  (((ch)->desc) ? (ch)->desc->wait = (cycle) : 0)

/* Object And Carry related macros */

#define CAN_SEE_OBJ(sub, obj)                                           \
	(   ( (!IS_NPC(sub)) && (GetMaxLevel(sub)>LOW_IMMORTAL))       ||   \
        ( (( !IS_SET((obj)->obj_flags.extra_flags, ITEM_INVISIBLE) ||   \
	     IS_AFFECTED((sub),AFF_DETECT_INVISIBLE) ) &&               \
	     !IS_AFFECTED((sub),AFF_BLIND)) &&                          \
             (IS_LIGHT(sub->in_room))))
#define GET_ITEM_TYPE(obj) ((obj)->obj_flags.type_flag)
#define CAN_WEAR(obj, part) (IS_SET((obj)->obj_flags.wear_flags,part))
#define GET_OBJ_WEIGHT(obj) ((obj)->obj_flags.weight)
#define CAN_CARRY_W(ch) (str_app[(int)STRENGTH_APPLY_INDEX(ch)].carry_w)
#define CAN_CARRY_N(ch) (5+GET_DEX(ch)/2+GetMaxLevel(ch)/2)
#define IS_CARRYING_W(ch) ((ch)->specials.carry_weight)
#define IS_CARRYING_N(ch) ((ch)->specials.carry_items)
#define CAN_CARRY_OBJ(ch,obj)  \
   (((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) <= CAN_CARRY_W(ch)) &&   \
    ((IS_CARRYING_N(ch) + 1) <= CAN_CARRY_N(ch)))
#define CAN_GET_OBJ(ch, obj)   \
   (CAN_WEAR((obj), ITEM_TAKE) && CAN_CARRY_OBJ((ch),(obj)) &&          \
    CAN_SEE_OBJ((ch),(obj)))
#define IS_OBJ_STAT(obj,stat) (IS_SET((obj)->obj_flags.extra_flags,stat))

/* char name/short_desc(for mobs) or someone?  */

#define PERS(ch, vict)   ( CAN_SEE(vict, ch) ? \
	  (!IS_NPC(ch) ? (ch)->player.name : (ch)->player.short_descr) : \
	  "someone")
#define MOB_NAME(ch) ((ch)->player.short_descr)
#define OBJS(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
	(obj)->short_description  : "something")
#define OBJN(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
	fname((obj)->name) : "something")
#define OUTSIDE(ch) (!IS_SET(real_roomp((ch)->in_room)->room_flags,INDOORS))
#define IS_IMMORTAL(ch) ((GetMaxLevel(ch)>=LOW_IMMORTAL)&&(!IS_NPC(ch)))
#define IS_MORTAL(ch)   (!IS_IMMORTAL(ch))
#define IS_POLICE(ch) ((mob_index[(int)ch->nr].virtual == 3060) || \
                       (mob_index[(int)ch->nr].virtual == 3069) || \
                       (mob_index[(int)ch->nr].virtual == 3067))
#define IS_CORPSE(obj) (GET_ITEM_TYPE((obj))==ITEM_CONTAINER && \
			(obj)->obj_flags.value[3] && \
			isname("corpse", (obj)->name))
#define EXIT(ch, door)  (real_roomp((ch)->in_room)->dir_option[door])
#define CAN_GO(ch, door) (EXIT(ch,door)&&real_roomp(EXIT(ch,door)->to_room) \
                          && !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
#define CAN_GO_HUMAN(ch, door) (EXIT(ch,door) && \
			  real_roomp(EXIT(ch,door)->to_room) \
                          && !IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))
#define GET_ALIGNMENT(ch) ((ch)->specials.alignment)
#define IS_REALLY_HOLY(ch) ((GET_ALIGNMENT(ch) >= ALIGN_REALLY_HOLY))
#define IS_REALLY_VILE(ch) ((GET_ALIGNMENT(ch) <= ALIGN_REALLY_VILE))
/* This makes no sense... I will change it. */
#ifdef OLD_WILEY
#define IS_HOLY(ch)	((GET_ALIGNMENT(ch)>900 && GET_ALIGNMENT(ch) < 1000 ))
#define IS_VILE(ch)	((GET_ALIGNMENT(ch) < -900 && GET_ALIGNMENT(ch)>-1000))
#else
#define IS_HOLY(ch)	 (GET_ALIGNMENT(ch) >= ALIGN_HOLY)
#define IS_VILE(ch)	 (GET_ALIGNMENT(ch) <= ALIGN_VILE)
#endif
#define IS_VERY_GOOD(ch) (GET_ALIGNMENT(ch) >= ALIGN_VERY_GOOD)
#define IS_VERY_EVIL(ch) (GET_ALIGNMENT(ch) <= ALIGN_VERY_EVIL)
#define IS_GOOD(ch)      (GET_ALIGNMENT(ch) >= ALIGN_GOOD)
#define IS_EVIL(ch)      (GET_ALIGNMENT(ch) <= ALIGN_EVIL)
#define IS_NICE(ch)      (GET_ALIGNMENT(ch) >= ALIGN_NICE)
#define IS_WICKED(ch)    (GET_ALIGNMENT(ch) <= ALIGN_WICKED)
#define IS_NEUTRAL(ch) (!IS_GOOD(ch) && !IS_EVIL(ch))
#define ITEM_TYPE(obj)  ((int)(obj)->obj_flags.type_flag)

#define IS_GETTING_HUNGRY(ch)   (GET_COND(ch, FULL) < 6)
#define IS_HUNGRY(ch)           (GET_COND(ch, FULL) < 4)
#define IS_STARVING(ch)         (GET_COND(ch, FULL) < 2)
#define IS_GETTING_THIRSTY(ch)  (GET_COND(ch, THIRST) < 6)
#define IS_THIRSTY(ch)          (GET_COND(ch, THIRST) < 4)
#define IS_PARCHED(ch)          (GET_COND(ch, THIRST) < 2)
#define IS_HOPELESSLY_DRUNK(ch) (GET_COND(ch, DRUNK) > 10)

 int                              MobVnum(struct char_data *c);
 int                              ObjVnum(struct obj_data *o);
 int                              percent(int value, int total);
 const char                            *ordinal(int x);
int                                     GetItemClassRestrictions(struct obj_data *obj);
int                                     CAN_SEE(struct char_data *s, struct char_data *o);
int                                     exit_ok(struct room_direction_data *room_exit,
						struct room_data **rpp);
 int                              IsImmune(struct char_data *ch, int bit);
 int                              IsResist(struct char_data *ch, int bit);
 int                              IsSusc(struct char_data *ch, int bit);
 int                              number(int from, int to);
 int                              dice(int rolls, int size);
extern  int                              fuzz(int x);
int                                     scan_number(const char *text, int *rval);

 void                             sprintbit(unsigned long vektor, const char *names[],
						  char *result);
 void                             sprinttype(int type, const char *names[], char *result);
int                                     in_group(struct char_data *ch1, struct char_data *ch2);
int                                     getall(char *name, char *newname);
int                                     getabunch(char *name, char *newname);
int                                     DetermineExp(struct char_data *mob, int exp_flags);
void                                    down_river(int current_pulse);
 int                              IsHumanoid(struct char_data *ch);
 int                              IsAnimal(struct char_data *ch);
 int                              IsUndead(struct char_data *ch);
 int                              IsLycanthrope(struct char_data *ch);
 int                              IsDiabolic(struct char_data *ch);
 int                              IsReptile(struct char_data *ch);
 int                              IsDraconic(struct char_data *ch);
 int                              IsAvian(struct char_data *ch);
 int                              IsExtraPlanar(struct char_data *ch);
 int                              HasHands(struct char_data *ch);
 int                              IsPerson(struct char_data *ch);
void                                    SetHunting(struct char_data *ch, struct char_data *tch);
void                                    CallForGuard(struct char_data *ch,
						     struct char_data *vict, int lev);
void                                    CallForAGuard(struct char_data *ch,
						      struct char_data *vict, int lev);
void                                    StandUp(struct char_data *ch);
void                                    FighterMove(struct char_data *ch);
void                                    DevelopHatred(struct char_data *ch,
						      struct char_data *v);
void                                    Teleport(int current_pulse);
int                                     HasObject(struct char_data *ch, int ob_num);
int                                     room_of_object(struct obj_data *obj);
struct char_data                       *char_holding(struct obj_data *obj);
int                                     RecCompObjNum(struct obj_data *o, int obj_num);
void                                    RestoreChar(struct char_data *ch);
 void                             RemAllAffects(struct char_data *ch);
 const char                            *pain_level(struct char_data *ch);
 int                              IsWizard(struct char_data *ch);
 int                              IsPriest(struct char_data *ch);
 int                              IsMagical(struct char_data *ch);
 int                              IsFighter(struct char_data *ch);
 int                              IsSneak(struct char_data *ch);

#define IsNonMagical(ch) (!IsMagical(ch))

size_t  strlcpy(char *dst, const char *src, size_t siz);
size_t  strlcat(char *dst, const char *src, size_t siz);
int     scprintf(char *buf, size_t limit, const char *Str, ...) __attribute__ ( ( format( printf, 3, 4 ) ) );
char   *time_elapsed(time_t since, time_t now);
char   *json_escape(char *thing);
char   *md5_hex(const char *str);
char   *color_wrap(int soft_limit, int hard_limit, const char *pad, const char *input);
time_t  file_date(const char *filename);

#define WILEYMUD_TIMESTAMP  "%Y-%m-%d %H:%M:%S"
#define WILEYMUD_TIMEZONE   "%Z"
char   *timestamp(time_t the_time, time_t the_micro);

#endif

/* ************************************************************************
*  file: utils.h, Utility module.                         Part of DIKUMUD *
*  Usage: Utility macros                                                  *
************************************************************************* */

int CAN_SEE(struct char_data *s, struct char_data *o);

#define TRUE  1

#define FALSE 0

#define LOWER(c) (((c)>='A'  && (c) <= 'Z') ? ((c)+('a'-'A')) : (c))

#define UPPER(c) (((c)>='a'  && (c) <= 'z') ? ((c)+('A'-'a')) : (c) )

#define ISNEWL(ch) ((ch) == '\n' || (ch) == '\r') 

#define IF_STR(st) ((st) ? (st) : "\0")

#define CAP(st)  (*(st) = UPPER(*(st)), st)

#define CREATE(result, type, number)  do {\
	if (!((result) = (type *) calloc ((number), sizeof(type))))\
		{ perror("malloc failure"); abort(); } } while(0)

#define RECREATE(result,type,number) do {\
  if (!((result) = (type *) realloc ((result), sizeof(type) * (number))))\
		{ perror("realloc failure"); abort(); } } while(0)

#define IS_SET(flag,bit)  ((flag) & (bit))

#define IS_NOT_SET(flag,bit)  (!IS_SET(flag,bit))

#define SWITCH(a,b) { (a) ^= (b); \
                      (b) ^= (a); \
                      (a) ^= (b); }

#define IS_AFFECTED(ch,skill) ( IS_SET((ch)->specials.affected_by, (skill)) )

#define GET_ZONE(room)	(real_roomp(room)->zone)

#define IS_DARK(room)  (!real_roomp(room)->light && IS_SET(real_roomp(room)->room_flags, DARK))

#define IS_LIGHT(room)  (real_roomp(room)->light || !IS_SET(real_roomp(room)->room_flags, DARK))

#define SET_BIT(var,bit)  ((var) = (var) | (bit))

#define REMOVE_BIT(var,bit)  ((var) = (var) & ~(bit) )

#define RM_FLAGS(i)  ((real_roomp(i))?real_roomp(i)->room_flags:0)

#define GET_LEVEL(ch, i)   ((ch)->player.level[(i)])


#define GET_CLASS_TITLE(ch, class, lev)   ((ch)->player.sex ?  \
   (((ch)->player.sex == 1) ? titles[(class)][(lev)].title_m : \
    titles[(class)][(lev)].title_f) : titles[(class)][(lev)].title_m)

#define GET_REQ(i) (i<2  ? "Awful" :(i<4  ? "Bad"     :(i<7  ? "Poor"      :\
(i<10 ? "Average" :(i<14 ? "Fair"    :(i<20 ? "Good"    :(i<24 ? "Very good" :\
        "Superb" )))))))

#define HSHR(ch) ((ch)->player.sex ?					\
	(((ch)->player.sex == 1) ? "his" : "her") : "its")

#define HSSH(ch) ((ch)->player.sex ?					\
	(((ch)->player.sex == 1) ? "he" : "she") : "it")

#define HMHR(ch) ((ch)->player.sex ? 					\
	(((ch)->player.sex == 1) ? "him" : "her") : "it")	

#define ANA(obj) (index("aeiouyAEIOUY", *(obj)->name) ? "An" : "A")

#define SANA(obj) (index("aeiouyAEIOUY", *(obj)->name) ? "an" : "a")

#define IS_PC(ch)  (!IS_SET((ch)->specials.act, ACT_ISNPC))

#define IS_NPC(ch)  (IS_SET((ch)->specials.act, ACT_ISNPC))

#define IS_MOB(ch)  (IS_SET((ch)->specials.act, ACT_ISNPC) && ((ch)->nr >-1))

#define MOUNTED(ch)  ((ch)->specials.mounted_on)

#define RIDDEN(ch)	((ch)->specials.ridden_by)

#define GET_POS(ch)     ((ch)->specials.position)

#define GET_COND(ch, i) ((ch)->specials.conditions[(i)])

#define GET_NAME(ch)    ((ch)->player.name)

#define GET_TITLE(ch)   ((ch)->player.title)

#define GET_PRETITLE(ch)   ((ch)->player.pre_title)

#define GET_POOF_IN(ch)   ((ch)->player.poof_in)

#define GET_POOF_OUT(ch)   ((ch)->player.poof_out)

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

#define AWAKE(ch) (GET_POS(ch) > POSITION_SLEEPING)

#define GET_IDLE_TIME(ch)  (time(0) - ((ch)->desc->idle_time))

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

#define CAN_CARRY_W(ch) (str_app[STRENGTH_APPLY_INDEX(ch)].carry_w)

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

#define PERS(ch, vict)   (                                          \
	CAN_SEE(vict, ch) ?						                                    \
	  (!IS_NPC(ch) ? (ch)->player.name : (ch)->player.short_descr) :	\
	  "someone")

#define MOB_NAME(ch) ((ch)->player.short_descr)

#define OBJS(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
	(obj)->short_description  : "something")

#define OBJN(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
	fname((obj)->name) : "something")

#define OUTSIDE(ch) (!IS_SET(real_roomp((ch)->in_room)->room_flags,INDOORS))

#define IS_IMMORTAL(ch) ((GetMaxLevel(ch)>=LOW_IMMORTAL)&&(!IS_NPC(ch)))
#define IS_MORTAL(ch)   (!IS_IMMORTAL(ch))

#define IS_POLICE(ch) ((mob_index[ch->nr].virtual == 3060) || \
                       (mob_index[ch->nr].virtual == 3069) || \
                       (mob_index[ch->nr].virtual == 3067))

#define IS_CORPSE(obj) (GET_ITEM_TYPE((obj))==ITEM_CONTAINER && \
			(obj)->obj_flags.value[3] && \
			isname("corpse", (obj)->name))

#define EXIT(ch, door)  (real_roomp((ch)->in_room)->dir_option[door])

int exit_ok(struct room_direction_data *, struct room_data **);

#define CAN_GO(ch, door) (EXIT(ch,door)&&real_roomp(EXIT(ch,door)->to_room) \
                          && !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))

#define CAN_GO_HUMAN(ch, door) (EXIT(ch,door) && \
			  real_roomp(EXIT(ch,door)->to_room) \
                          && !IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))

#define GET_ALIGNMENT(ch) ((ch)->specials.alignment)

#define IS_REALLY_HOLY(ch) ((GET_ALIGNMENT(ch) == 1000))
#define IS_REALLY_VILE(ch) ((GET_ALIGNMENT(ch) == -1000))

#define IS_HOLY(ch)	((GET_ALIGNMENT(ch)>900 && GET_ALIGNMENT(ch) < 1000 ))
#define IS_VILE(ch)	((GET_ALIGNMENT(ch) < -900 && GET_ALIGNMENT(ch)>-1000))

#define IS_GOOD(ch)    (GET_ALIGNMENT(ch) >= 350)
#define IS_EVIL(ch)    (GET_ALIGNMENT(ch) <= -350)

#define IS_NEUTRAL(ch) (!IS_GOOD(ch) && !IS_EVIL(ch))

#define ITEM_TYPE(obj)  ((int)(obj)->obj_flags.type_flag)

/*
 * File: fight.c , Combat module.                         Part of DIKUMUD
 * Usage: Combat system and messages.
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <assert.h>

#include "include/global.h"
#include "include/bug.h"
#include "include/utils.h"
#include "include/comm.h"
#include "include/handler.h"
#include "include/interpreter.h"
#include "include/db.h"
#include "include/constants.h"
#include "include/spells.h"
#include "include/spell_parser.h"
#include "include/mudlimits.h"
#include "include/random.h"
#include "include/act_move.h"
#include "include/reception.h"
#include "include/multiclass.h"
#include "include/act_wiz.h"
#include "include/act_skills.h"
#include "include/opinion.h"
#include "include/spec_procs.h"
#include "include/mob_actions.h"
#include "include/act_off.h"
#define _FIGHT_C
#include "include/fight.h"

struct char_data *combat_list = 0;     /* head of l-list of fighting chars    */
struct char_data *combat_next_dude = 0;		/* Next dude global trick           */

/* Weapon attack texts */
struct attack_hit_type attack_hit_text[] =
{
  {"hit", "hits"},		       /* TYPE_HIT      */
  {"pound", "pounds"},		       /* TYPE_BLUDGEON */
  {"pierce", "pierces"},	       /* TYPE_PIERCE   */
  {"slash", "slashes"},		       /* TYPE_SLASH    */
  {"whip", "whips"},		       /* TYPE_WHIP     */
  {"claw", "claws"},		       /* TYPE_CLAW     */
  {"bite", "bites"},		       /* TYPE_BITE     */
  {"sting", "stings"},		       /* TYPE_STING    */
  {"crush", "crushes"},		       /* TYPE_CRUSH    */
  {"cleave", "cleaves"},
  {"stab", "stabs"},
  {"smash", "smashes"},
  {"smite", "smites"}
};

/* The Fight related routines */

void appear(struct char_data *ch)
{
  if (DEBUG)
    dlog("appear");

  if (affected_by_spell(ch, SPELL_INVISIBLE)) {
    act("$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM);
    affect_from_char(ch, SPELL_INVISIBLE);

    if (IS_SET(ch->specials.affected_by, AFF_INVISIBLE))
      REMOVE_BIT(ch->specials.affected_by, AFF_INVISIBLE);
  }
  if (affected_by_spell(ch, SKILL_SNEAK)) {
    affect_from_char(ch, SKILL_SNEAK);

    if (IS_SET(ch->specials.affected_by, AFF_SNEAK))
      REMOVE_BIT(ch->specials.affected_by, AFF_SNEAK);

    if (IS_SET(ch->specials.affected_by, AFF_HIDE))
      REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);
  }
}

void load_messages(void)
{
  FILE *f1;
  int i, type;
  struct message_type *messages;
  char chk[100];

  if (DEBUG)
    dlog("load_messages");
  if (!(f1 = fopen(MESS_FILE, "r"))) {
    perror("read messages");
    exit(0);
  }
  /*
   * find the memset way of doing this...
   */

  for (i = 0; i < MAX_MESSAGES; i++) {
    fight_messages[i].a_type = 0;
    fight_messages[i].number_of_attacks = 0;
    fight_messages[i].msg = 0;
  }

  fscanf(f1, " %s \n", chk);

  i = 0;

  while (*chk == 'M') {
    fscanf(f1, " %d\n", &type);

    if (i >= MAX_MESSAGES) {
      log("Too many combat messages.");
      exit(0);
    }
    CREATE(messages, struct message_type, 1);

    fight_messages[i].number_of_attacks++;
    fight_messages[i].a_type = type;
    messages->next = fight_messages[i].msg;
    fight_messages[i].msg = messages;

    messages->die_msg.attacker_msg = fread_string(f1);
    messages->die_msg.victim_msg = fread_string(f1);
    messages->die_msg.room_msg = fread_string(f1);
    messages->miss_msg.attacker_msg = fread_string(f1);
    messages->miss_msg.victim_msg = fread_string(f1);
    messages->miss_msg.room_msg = fread_string(f1);
    messages->hit_msg.attacker_msg = fread_string(f1);
    messages->hit_msg.victim_msg = fread_string(f1);
    messages->hit_msg.room_msg = fread_string(f1);
    messages->god_msg.attacker_msg = fread_string(f1);
    messages->god_msg.victim_msg = fread_string(f1);
    messages->god_msg.room_msg = fread_string(f1);
    fscanf(f1, " %s \n", chk);
    i++;
  }
  fclose(f1);
}

void update_pos(struct char_data *victim)
{
  if (DEBUG)
    dlog("update_pos");

  if (GET_POS(victim) <= POSITION_STUNNED) {
    if (MOUNTED(victim)) {
      FallOffMount(victim, MOUNTED(victim));
      Dismount(victim, MOUNTED(victim), GET_POS(victim));
    }
  }
  if ((GET_HIT(victim) > 0) && (GET_POS(victim) > POSITION_STUNNED)) {
    if (MOUNTED(victim))
      GET_POS(victim) = POSITION_MOUNTED;
    return;
  } else if (GET_HIT(victim) > 0) {
    if (IS_AFFECTED(victim, AFF_PARALYSIS)) {
      if (MOUNTED(victim)) {
	FallOffMount(victim, MOUNTED(victim));
        Dismount(victim, MOUNTED(victim), GET_POS(victim));
      }
      GET_POS(victim) = POSITION_STUNNED;
    } else {
      if (MOUNTED(victim))
	GET_POS(victim) = POSITION_MOUNTED;
      else
	GET_POS(victim) = POSITION_STANDING;
    }
  } else if (GET_HIT(victim) <= -11)
    GET_POS(victim) = POSITION_DEAD;
  else if (GET_HIT(victim) <= -6)
    GET_POS(victim) = POSITION_MORTALLYW;
  else if (GET_HIT(victim) <= -3)
    GET_POS(victim) = POSITION_INCAP;
  else
    GET_POS(victim) = POSITION_STUNNED;
}

int check_peaceful(struct char_data *ch, char *msg)
{
  struct room_data *rp;

  if (DEBUG)
    dlog("check_peaceful");
  if(IS_IMMORTAL(ch))
    return 0;
  rp = real_roomp(ch->in_room);
  if (rp && rp->room_flags & PEACEFUL) {
    cprintf(ch, msg);
    return 1;
  }
  return 0;
}

/* start one char fighting another (yes, it is horrible, I know... )  */
void set_fighting(struct char_data *ch, struct char_data *vict)
{
  if (DEBUG)
    dlog("set_fighting");
  if (ch->specials.fighting) {
    log("Fighting character set to fighting another.");
    return;
  }
  if (vict->attackers < MAX_ATTACKERS) {
    vict->attackers += 1;
  } else {
    log("more than 6 people attacking one target");
  }
  ch->next_fighting = combat_list;
  combat_list = ch;

  if (IS_AFFECTED(ch, AFF_SLEEP))
    affect_from_char(ch, SPELL_SLEEP);

  ch->specials.fighting = vict;
  GET_POS(ch) = POSITION_FIGHTING;
}

/* remove a char from the list of fighting chars */
void stop_fighting(struct char_data *ch)
{
  struct char_data *tmp;

  if (DEBUG)
    dlog("stop_fighting");
  if (!ch->specials.fighting) {
    log("Character not fighting at invocation of stop_fighting");
    return;
  }
  ch->specials.fighting->attackers -= 1;
  if (ch->specials.fighting->attackers < 0) {
    log("too few people attacking");
    ch->specials.fighting->attackers = 0;
  }
  if (ch == combat_next_dude)
    combat_next_dude = ch->next_fighting;

  if (combat_list == ch)
    combat_list = ch->next_fighting;
  else {
    for (tmp = combat_list; tmp && (tmp->next_fighting != ch);
	 tmp = tmp->next_fighting);
    if (!tmp) {
      log("Char fighting not found Error (fight.c, stop_fighting)");
      abort();
    }
    tmp->next_fighting = ch->next_fighting;
  }

  ch->next_fighting = 0;
  ch->specials.fighting = 0;
  GET_POS(ch) = POSITION_STANDING;
  update_pos(ch);
}

void make_corpse(struct char_data *ch)
{
  struct obj_data *corpse, *o, *food;
  struct obj_data *money;
  char buf[MAX_INPUT_LENGTH];
  int i, ADeadBody = FALSE;
  int r_num;
  int chance, food_num = 0;

  CREATE(corpse, struct obj_data, 1);

  clear_object(corpse);

  corpse->item_number = NOWHERE;
  corpse->in_room = NOWHERE;

  if (!IS_NPC(ch) || !IsUndead(ch)) {
    sprintf(buf, "corpse %s", ch->player.name);
    corpse->name = strdup(buf);

    sprintf(buf, "The corpse of %s is lying here.",
	    (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
    corpse->description = strdup(buf);

    sprintf(buf, "the corpse of %s",
	    (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
    corpse->short_description = strdup(buf);

    ADeadBody = TRUE;
  } else if (IsUndead(ch)) {
    corpse->name = strdup("dust pile");
    corpse->description = strdup("A pile of dust is here.");
    corpse->short_description = strdup("a pile of dust");
  }
  corpse->contains = ch->carrying;
  if(!IS_IMMORTAL(ch)) {
    if (GET_GOLD(ch) > 0) {
      money = create_money(GET_GOLD(ch));
      GET_GOLD(ch) = 0;
      obj_to_obj(money, corpse);
    }
  }
  if (IS_NPC(ch) && IS_SET(ch->specials.act, ACT_FOOD_PROVIDE)) {
    chance = number(0, 6);
    switch (chance) {
    case 0:
    case 1:
    case 2:
      food_num = 4;		       /* good food */
      break;
    }

    if ((r_num = real_object(food_num)) >= 0) {
      food = read_object(r_num, REAL);
      obj_to_obj(food, corpse);
    }
  }
  corpse->obj_flags.type_flag = ITEM_CONTAINER;
  corpse->obj_flags.wear_flags = ITEM_TAKE;
  corpse->obj_flags.value[0] = 0;      /* You can't store stuff in a corpse */
  corpse->obj_flags.value[3] = 1;      /* corpse identifyer */

  if (ADeadBody) {
    corpse->obj_flags.weight = GET_WEIGHT(ch) + IS_CARRYING_W(ch);
  } else {
    corpse->obj_flags.weight = 1 + IS_CARRYING_W(ch);
  }
  corpse->obj_flags.cost_per_day = 100000;
  if (IS_NPC(ch))
    corpse->obj_flags.timer = MAX_NPC_CORPSE_TIME;
  else
    corpse->obj_flags.timer = MAX_PC_CORPSE_TIME;

  for (i = 0; i < MAX_WEAR; i++)
    if (ch->equipment[i])
      obj_to_obj(unequip_char(ch, i), corpse);

  ch->carrying = 0;
  IS_CARRYING_N(ch) = 0;
  IS_CARRYING_W(ch) = 0;

  if (IS_NPC(ch)) {
    corpse->char_vnum = mob_index[ch->nr].virtual;
    corpse->char_f_pos = 0;
  } else {
    if (ch->desc) {
      corpse->char_f_pos = ch->desc->pos;
      corpse->char_vnum = 0;
    } else {
      corpse->char_f_pos = 0;
      corpse->char_vnum = 100;
    }
  }
  corpse->carried_by = 0;
  corpse->equipped_by = 0;

  corpse->next = object_list;
  object_list = corpse;

  for (o = corpse->contains; o; o = o->next_content)
    o->in_obj = corpse;

  object_list_new_owner(corpse, 0);
  obj_to_room(corpse, ch->in_room);
}

void change_alignment(struct char_data *ch, struct char_data *victim)
{
  if (DEBUG)
    dlog("change_alignment");
  if (IS_NPC(ch))
    return;

  if (IS_GOOD(ch) && (IS_GOOD(victim))) {
    GET_ALIGNMENT(ch) -=
      (GET_ALIGNMENT(victim) / 200) * (MAX(1, GetMaxLevel(ch) - GetMaxLevel(victim)));
  } else if (IS_EVIL(ch) && (IS_GOOD(victim))) {
    GET_ALIGNMENT(ch) -=
      (GET_ALIGNMENT(victim) / 300) * (MAX(1, GetMaxLevel(ch) - GetMaxLevel(victim)));
  } else if ((IS_GOOD(ch)) && IS_EVIL(victim)) {
    GET_ALIGNMENT(ch) -=
      (GET_ALIGNMENT(victim) / 400) * (MAX(1, GetMaxLevel(ch) - GetMaxLevel(victim)));
  } else if (IS_EVIL(ch) && (IS_EVIL(victim))) {
    GET_ALIGNMENT(ch) -=
      (GET_ALIGNMENT(victim) / 450) * (MAX(1, GetMaxLevel(victim) - GetMaxLevel(ch)));
  } else {
    GET_ALIGNMENT(ch) -=
      (GET_ALIGNMENT(victim) / 200) * (MAX(1, GetMaxLevel(victim) - GetMaxLevel(ch)));
  }

  GET_ALIGNMENT(ch) = MAX(GET_ALIGNMENT(ch), -1000);
  GET_ALIGNMENT(ch) = MIN(GET_ALIGNMENT(ch), 1000);
}

void death_cry(struct char_data *ch)
{
  int door, was_in;

  if (DEBUG)
    dlog("death_cry");
  if (ch->in_room == -1)
    return;

  act("Your blood freezes as you hear $n's death cry.", FALSE, ch, 0, 0, TO_ROOM);
  was_in = ch->in_room;

  for (door = 0; door < MAX_NUM_EXITS; door++) {
    if (CAN_GO(ch, door)) {
      ch->in_room = (real_roomp(was_in))->dir_option[door]->to_room;
      act("Your blood freezes as you hear someones death cry.", FALSE, ch, 0, 0, TO_ROOM);
      ch->in_room = was_in;
    }
  }
}

void raw_kill(struct char_data *ch)
{
  if (DEBUG)
    dlog("raw_kill");

  if (ch->specials.fighting)
    stop_fighting(ch);

  death_cry(ch);
  /*
   * remove the problem with poison, and other spells
   */
  spell_dispel_magic(IMPLEMENTOR, ch, ch, 0);

  /*
   * give them some food and water so they don't whine.
   */

  if(IS_MORTAL(ch)) {
    GET_COND(ch, THIRST) = 20;
    GET_COND(ch, FULL) = 20;
  }

  /*
   *   return them from POLY -t morph
   */

  if (IS_NPC(ch)) {
    make_corpse(ch);
    extract_char(ch);
  } else {
    if(IS_IMMORTAL(ch)) {
      cprintf(ch, "\nYou have been FORCED from this plane!\n\r");
      rprintf(ch->in_room, "The Immortal %s dissolves and fades from sight!\n\rA dead body hits the ground and begins to rot.\n\r", GET_NAME(ch));
      make_corpse(ch);
      zero_rent(ch);
      char_from_room(ch);
      char_to_room(ch, 0);
      GET_HIT(ch) = GET_MAX_HIT(ch);
      GET_POS(ch) = POSITION_STANDING;
      save_char(ch, NOWHERE);
    } else {
      cprintf(ch, "\nYou have DIED!, your spirit flees in terror!\n\r");
      rprintf(ch->in_room, "The spirit of %s flashes away!\n\rA dead body hits the ground and begins to rot.\n\r", GET_NAME(ch));
      make_corpse(ch);
      zero_rent(ch);
      char_from_room(ch);
      char_to_room(ch, GET_HOME(ch));
      GET_HIT(ch) = 1;
      GET_POS(ch) = POSITION_SLEEPING;
      save_char(ch, NOWHERE);
      rprintf(ch->in_room, "A terrified %s appears in a flash of light!\n\r", GET_NAME(ch));
    }
  }
}

void die(struct char_data *ch)
{
  struct char_data *castor, *mobile;

  if (RIDDEN(ch)) {
    FallOffMount(RIDDEN(ch), ch);
    Dismount(RIDDEN(ch), ch, POSITION_SITTING);
  } else if (MOUNTED(ch)) {
    FallOffMount(ch, MOUNTED(ch));
    Dismount(ch, MOUNTED(ch), POSITION_SITTING);
  }
  if (IS_NPC(ch) && (IS_SET(ch->specials.act, ACT_POLYSELF))) {
    mobile = ch;
    castor = mobile->desc->original;
    char_from_room(castor);
    char_to_room(castor, mobile->in_room);
    /* SwitchStuff(mobile, castor); */
    extract_char(mobile);
    mobile = castor;
    ch = castor;
    DeleteHatreds(ch);
    DeleteFears(ch);
    return;
  }
  /*
   **  this has to go back to the old way
   */

  if (IS_NPC(ch)) {
    gain_exp(ch, -GET_EXP(ch) / 2);
  } else if (GetMaxLevel(ch) < 7) {
    gain_exp(ch, -GET_EXP(ch) / (6 * HowManyClasses(ch)));
  } else if (GetMaxLevel(ch) < 14) {
    gain_exp(ch, -GET_EXP(ch) / (5 * HowManyClasses(ch)));
  } else if (GetMaxLevel(ch) < 21) {
    gain_exp(ch, -GET_EXP(ch) / (4 * HowManyClasses(ch)));
  } else if (GetMaxLevel(ch) < 28) {
    gain_exp(ch, -GET_EXP(ch) / (3 * HowManyClasses(ch)));
  } else
    gain_exp(ch, -GET_EXP(ch) / (2 * HowManyClasses(ch)));

  /*
   **      Set the talk[2] to be FALSE, i.e. DEAD
   */

  /* ch->player.talks[2] = FALSE; */ /* char is dead */

  DeleteHatreds(ch);
  DeleteFears(ch);
  raw_kill(ch);
}

void group_gain(struct char_data *ch, struct char_data *victim)
{
  char buf[256];
  int no_members= 0,
      member_count= 0;
  double leftover= 0.0,
         share= 0.0,
         amnt= 0.0;
  struct char_data *k;
  struct follow_type *f;

  if (!(k = ch->master))
    k = ch;

  if (IS_AFFECTED(k, AFF_GROUP)) { /* if they are grouped, they suck exp */
    no_members = GetTotLevel(k);
    member_count++;
  }

  for (f = k->followers; f; f = f->next)
    if (IS_AFFECTED(f->follower, AFF_GROUP)) {
      no_members += GetTotLevel(f->follower);
      member_count++;
    }

  if (no_members >= 1)
    share = (double)((IS_PC(victim)?GetMaxLevel(victim)*500:GET_EXP(victim)) / (double)no_members);
  else
    share = 0.0;

  share = MAX(MIN(share, 100000.0), 0.0);

  if(member_count == 1) {
    amnt= (double)(IS_PC(victim)?GetMaxLevel(victim)*500:GET_EXP(victim));
    if(IS_SET(k->specials.new_act, NEW_PLR_KILLOK))
      amnt *= 1.10;
#ifdef MOB_LEVELING
    if (IS_NPC(k) && (!IS_SET(k->specials.act, ACT_POLYSELF) &&
                      !IS_SET(k->specials.act, ACT_POLYSELF)))
      amnt /= 4.0;
#endif
    sprintf(buf, "You receive your loner's share of %d experience.", (int)amnt);
    act(buf, FALSE, k, 0, 0, TO_CHAR);
    gain_exp(k, (int)amnt);
    change_alignment(k, victim);
    return;
  }

/*
 * Concept:  Multiclassers deserve an extra portion because of their
 *           contributions to the group... but not as much as all of their
 *           levels.  We have to divvy it up in fractions without losing
 *           too much.
 */

  if (IS_AFFECTED(k, AFF_GROUP)) {
    amnt = share * ((double)GetMaxLevel(k)
                 + ((double)GetSecMaxLev(k) / 2.0)
                 + ((double)GetThirdMaxLev(k) / 4.0));
    leftover += share * (((double)GetSecMaxLev(k) / 2.0)
                        + (3.0 * ((double)GetThirdMaxLev(k) / 4.0)));
    if(IS_SET(k->specials.new_act, NEW_PLR_KILLOK))
      amnt *= 1.10;
    if(k->in_room != ch->in_room) {
      amnt /= 10.0;
#ifdef MOB_LEVELING
      if (IS_NPC(k) && (!IS_SET(k->specials.act, ACT_POLYSELF) &&
                        !IS_SET(k->specials.act, ACT_POLYSELF)))
        amnt /= 4.0;
#endif
        sprintf(buf, "You receive your coward's share of %d experience.",
                (int)amnt);
    } else {
#ifdef MOB_LEVELING
      if (IS_NPC(k) && (!IS_SET(k->specials.act, ACT_POLYSELF) &&
                        !IS_SET(k->specials.act, ACT_POLYSELF)))
        amnt /= 4.0;
#endif
        sprintf(buf, "You receive your share of %d experience.", (int)amnt);
    }
    act(buf, FALSE, k, 0, 0, TO_CHAR);
    gain_exp(k, (int)amnt);
    change_alignment(k, victim);
  }
  for (f = k->followers; f; f = f->next) {
    if (IS_AFFECTED(f->follower, AFF_GROUP)) {
      amnt = share * ((double)GetMaxLevel(f->follower)
                   + ((double)GetSecMaxLev(f->follower) / 2.0)
                   + ((double)GetThirdMaxLev(f->follower) / 4.0));
      leftover += share * (((double)GetSecMaxLev(f->follower) / 2.0)
                        + (3.0 * ((double)GetThirdMaxLev(f->follower) / 4.0)));
      if(IS_SET(f->follower->specials.new_act, NEW_PLR_KILLOK))
        amnt *= 1.10;
      if(f->follower->in_room != ch->in_room) {
        amnt /= 10.0;
#ifdef MOB_LEVELING
        if (IS_NPC(f->follower) &&
                  (!IS_SET(f->follower->specials.act, ACT_POLYSELF) &&
                   !IS_SET(f->follower->specials.act, ACT_POLYSELF)))
          amnt /= 4.0;
#endif
        sprintf(buf, "You receive your coward's share of %d experience.",
                (int)amnt);
      } else {
#ifdef MOB_LEVELING
        if (IS_NPC(f->follower) &&
                  (!IS_SET(f->follower->specials.act, ACT_POLYSELF) &&
                   !IS_SET(f->follower->specials.act, ACT_POLYSELF)))
          amnt /= 4.0;
#endif
        sprintf(buf, "You receive your share of %d experience.", (int)amnt);
      }
      act(buf, FALSE, f->follower, 0, 0, TO_CHAR);
      gain_exp(f->follower, (int)amnt);
      change_alignment(f->follower, victim);
    }
  }
  leftover /= (double)member_count;
  amnt= leftover;
  if(IS_SET(k->specials.new_act, NEW_PLR_KILLOK))
    amnt *= 1.10;
  if(k->in_room != ch->in_room)
    amnt /= 10.0;
  if(amnt > 0.0 && (IS_PC(k) || IS_SET(k->specials.act, ACT_POLYSELF)
                            || IS_SET(k->specials.act, ACT_POLYSELF))) {
    sprintf(buf, "You also receive a leftover portion of %d experience.",
           (int)amnt);
    act(buf, FALSE, k, 0, 0, TO_CHAR);
    gain_exp(k, (int)amnt);
  }
  for (f = k->followers; f; f = f->next) {
    amnt= leftover;
    if(IS_SET(f->follower->specials.new_act, NEW_PLR_KILLOK))
      amnt *= 1.10;
    if(f->follower->in_room != ch->in_room)
      amnt /= 10.0;
    if(amnt > 0.0 && ((IS_PC(f->follower) && IS_AFFECTED(f->follower, AFF_GROUP))
                || IS_SET(f->follower->specials.act, ACT_POLYSELF)
                || IS_SET(f->follower->specials.act, ACT_POLYSELF))) {
      sprintf(buf, "You also receive a leftover portion of %d experience.",
              (int)amnt);
      act(buf, FALSE, f->follower, 0, 0, TO_CHAR);
      gain_exp(f->follower, (int)amnt);
    }
  }
}

char *replace_string(char *str, char *weapon, char *weapon_s)
{
  static char buf[256];
  char *cp;

  if (DEBUG)
    dlog("replace_string");
  cp = buf;

  for (; *str; str++) {
    if (*str == '#') {
      switch (*(++str)) {
      case 'W':
	for (; *weapon; *(cp++) = *(weapon++));
	break;
      case 'w':
	for (; *weapon_s; *(cp++) = *(weapon_s++));
	break;
      default:
	*(cp++) = '#';
	break;
      }
    } else {
      *(cp++) = *str;
    }

    *cp = 0;
  }				       /* For */

  return (buf);
}

void dam_message(int dam, struct char_data *ch, struct char_data *victim,
		 int w_type)
{
  struct obj_data *wield;
  char *buf;
  int snum;

  static struct dam_weapon_type {
    char *to_room;
    char *to_char;
    char *to_victim;
  } dam_weapons[] =
  {
    {
      "$n misses $N.",		       /*    0    */
	"You miss $N.",
	"$n misses you."
    },

    {
      "$n bruises $N with $s #w.",     /*  1.. 2  */
	"You bruise $N as you #w $M.",
	"$n bruises you as $e #W you."
    },

    {
      "$n barely #W $N.",	       /*  3.. 4  */
	"You barely #w $N.",
	"$n barely #W you."
    },

    {
      "$n #W $N.",		       /*  5.. 6  */
	"You #w $N.",
	"$n #W you."
    },

    {
      "$n #W $N hard.",		       /*  7..10  */
	"You #w $N hard.",
	"$n #W you hard."
    },

    {
      "$n #W $N very hard.",	       /* 11..14  */
	"You #w $N very hard.",
	"$n #W you very hard."
    },

    {
      "$n #W $N extremely well.",      /* 15..20  */
	"You #w $N extremely well.",
	"$n #W you extremely well."
    },

    {
      "$n massacres $N with $s #w.",   /* > 20    */
	"You massacre $N with your #w.",
	"$n massacres you with $s #w."
    }
  };
  if (DEBUG)
    dlog("dam_message");
  w_type -= TYPE_HIT;		       /* Change to base of table with text */

  if (ch->equipment[WIELD_TWOH])
    wield = ch->equipment[WIELD_TWOH];
  else
    wield = ch->equipment[WIELD];

  if (dam == 0)
    snum = 0;
  else if (dam <= 2)
    snum = 1;
  else if (dam <= 4)
    snum = 2;
  else if (dam <= 10)
    snum = 3;
  else if (dam <= 15)
    snum = 4;
  else if (dam <= 20)
    snum = 5;
  else if (dam <= 30)
    snum = 6;
  else
    snum = 7;

  buf = replace_string(dam_weapons[snum].to_room, attack_hit_text[w_type].plural, attack_hit_text[w_type].singular);
  act(buf, FALSE, ch, wield, victim, TO_NOTVICT);
  buf = replace_string(dam_weapons[snum].to_char, attack_hit_text[w_type].plural, attack_hit_text[w_type].singular);
  act(buf, FALSE, ch, wield, victim, TO_CHAR);
  buf = replace_string(dam_weapons[snum].to_victim, attack_hit_text[w_type].plural, attack_hit_text[w_type].singular);
  act(buf, FALSE, ch, wield, victim, TO_VICT);

}

int damage(struct char_data *ch, struct char_data *victim, int dam, int attacktype)
{
  char buf[MAX_INPUT_LENGTH];
  struct obj_data *wield_ptr;
  struct message_type *messages;
  int i, j, nr, max_hit, exp;
  struct room_data *rp;

  int hit_limit(struct char_data *ch);

/*  assert(GET_POS(victim) > POSITION_DEAD); */
  if (DEBUG)
    dlog("damage");
  if(GET_POS(victim) <= POSITION_DEAD)
    return FALSE;
  rp = real_roomp(ch->in_room);

/*
 * if (rp && rp->room_flags & PEACEFUL && attacktype != SPELL_POISON &&
 *     attacktype != TYPE_HUNGER ) {
 */
  if (check_peaceful(ch, "") && attacktype != SPELL_POISON &&
      attacktype != TYPE_HUNGER ) {
    char buf[MAX_INPUT_LENGTH];

    sprintf(buf, "damage(,,,%d) called in PEACEFUL room", attacktype);
    log(buf);
    return FALSE;
  }
  if (ch->in_room != victim->in_room)
    return (FALSE);

  if ((dam = SkipImmortals(victim, dam)) == -1)
    return (FALSE);

  appear(ch);

  if (!CheckKill(ch, victim))
    return (FALSE);

  if (victim != ch) {
    if (GET_POS(victim) > POSITION_STUNNED) {
      if (!(victim->specials.fighting)) {
	if ((IS_PC(ch)) || (IS_NOT_SET(ch->specials.act, ACT_IMMORTAL))) {
	  if (ch->attackers < MAX_ATTACKERS) {
	    set_fighting(victim, ch);
	    GET_POS(victim) = POSITION_FIGHTING;
	  }
	} else {
	  return (FALSE);
	}
      }
    }
    if (GET_POS(ch) > POSITION_STUNNED) {
      if (!(ch->specials.fighting)) {
	if ((IS_PC(ch)) || (IS_NOT_SET(ch->specials.act, ACT_IMMORTAL))) {
	  set_fighting(ch, victim);
	  GET_POS(ch) = POSITION_FIGHTING;
	} else {
	  return (FALSE);
	}
      }
    }
  }
  if (IS_NPC(ch) && IS_NPC(victim) && victim->master &&
      !number(0, 10) && IS_AFFECTED(victim, AFF_CHARM) &&
      (victim->master->in_room == ch->in_room) &&
      !IS_IMMORTAL(victim->master)) {
    /* an NPC will occasionally switch to attack the master of
     * its victim if the victim is an NPC and charmed by a mortal */
    if (ch->specials.fighting)
      stop_fighting(ch);
    hit(ch, victim->master, TYPE_UNDEFINED);
    return (FALSE);
  }
  if (victim->master == ch)
    stop_follower(victim);

  if (IS_AFFECTED(victim, AFF_SANCTUARY)) {
    dam = MAX((int)(dam / 2), 0);      /* Max 1/2 damage when sanct'd */
    affect_from_char(victim, SPELL_SANCTUARY);
  }
  dam = PreProcDam(victim, attacktype, dam);
  dam = WeaponCheck(ch, victim, attacktype, dam);

  DamageStuff(victim, attacktype, dam);

  dam = MAX(dam, 0);

  GET_HIT(victim) -= dam;

  if (IS_AFFECTED(victim, AFF_FIRESHIELD) && !IS_AFFECTED(ch, AFF_FIRESHIELD)) {
    affect_from_char(victim, SPELL_FIRESHIELD);
    if (damage(victim, ch, dam, SPELL_FIRESHIELD)) {
      update_pos(victim);
      if (GET_POS(victim) != POSITION_DEAD)
	return (FALSE);
      else
	return (TRUE);
    }
  }
  update_pos(victim);

  if ((attacktype >= TYPE_HIT) && (attacktype <= TYPE_SMITE)) {
    if (!ch->equipment[WIELD] && !ch->equipment[WIELD_TWOH]) {
      dam_message(dam, ch, victim, TYPE_HIT);
    } else {
      dam_message(dam, ch, victim, attacktype);
    }
  } else {
    if (ch->equipment[WIELD_TWOH])
      wield_ptr = ch->equipment[WIELD_TWOH];
    else
      wield_ptr = ch->equipment[WIELD];

    for (i = 0; i < MAX_MESSAGES; i++) {
      if (fight_messages[i].a_type == attacktype) {
	nr = dice(1, fight_messages[i].number_of_attacks);
	for (j = 1, messages = fight_messages[i].msg; (j < nr) && (messages); j++)
	  messages = messages->next;

/*
 * if (IS_PC(victim) && (GetMaxLevel(victim) > MAX_MORT)) {
 *   act(messages->god_msg.attacker_msg, FALSE, ch, wield_ptr, victim, TO_CHAR);
 *   act(messages->god_msg.victim_msg, FALSE, ch, wield_ptr, victim, TO_VICT);
 *   act(messages->god_msg.room_msg, FALSE, ch, wield_ptr, victim, TO_NOTVICT);
 * } else if (dam != 0) {
 */
        if (dam != 0) {
	  if (GET_POS(victim) == POSITION_DEAD) {
	    act(messages->die_msg.attacker_msg, FALSE, ch, wield_ptr, victim, TO_CHAR);
	    act(messages->die_msg.victim_msg, FALSE, ch, wield_ptr, victim, TO_VICT);
	    act(messages->die_msg.room_msg, FALSE, ch, wield_ptr, victim, TO_NOTVICT);
	  } else {
	    act(messages->hit_msg.attacker_msg, FALSE, ch, wield_ptr, victim, TO_CHAR);
	    act(messages->hit_msg.victim_msg, FALSE, ch, wield_ptr, victim, TO_VICT);
	    act(messages->hit_msg.room_msg, FALSE, ch, wield_ptr, victim, TO_NOTVICT);
	  }
	} else {		       /* Dam == 0 */
	  act(messages->miss_msg.attacker_msg, FALSE, ch, wield_ptr, victim, TO_CHAR);
	  act(messages->miss_msg.victim_msg, FALSE, ch, wield_ptr, victim, TO_VICT);
	  act(messages->miss_msg.room_msg, FALSE, ch, wield_ptr, victim, TO_NOTVICT);
	}
      }
    }
  }

  if ((GET_POS(victim) <= POSITION_STUNNED) && MOUNTED(victim)) {
    FallOffMount(victim, MOUNTED(victim));
    Dismount(victim, MOUNTED(victim), GET_POS(victim));
  }
  switch (GET_POS(victim)) {
  case POSITION_MORTALLYW:
    act("$n is mortally wounded and will die if not aided.", TRUE, victim, 0, 0, TO_ROOM);
    act("You are mortally wounded and will die if not aided.", FALSE, victim, 0, 0, TO_CHAR);
    break;
  case POSITION_INCAP:
    act("$n is incapacitated and will die if not aided.", TRUE, victim, 0, 0, TO_ROOM);
    act("You are incapacitated and you will die if not aided.", FALSE, victim, 0, 0, TO_CHAR);
    break;
  case POSITION_STUNNED:
    act("$n is stunned, but will probably regain consciousness.", TRUE, victim, 0, 0, TO_ROOM);
    act("You're stunned but will probably regain consciousness again.", FALSE, victim, 0, 0, TO_CHAR);
    break;
  case POSITION_DEAD:
    act("$n is dead! R.I.P.", TRUE, victim, 0, 0, TO_ROOM);
    act("You are dead!  Sorry...", FALSE, victim, 0, 0, TO_CHAR);
    break;

  default:			       /* >= POSITION SLEEPING */
    max_hit = hit_limit(victim);
    if (dam >= (max_hit / 5))
      act("That Really HURT!", FALSE, victim, 0, 0, TO_CHAR);
    if (GET_HIT(victim) < (max_hit / 5)) {
      act("You wish that your wounds would stop BLEEDING so much!", FALSE, victim, 0, 0, TO_CHAR);
    }
    break;
  }

  max_hit = hit_limit(victim);
  if (GET_HIT(victim) <= (max_hit / 4) && (GET_POS(victim) > POSITION_STUNNED)) {
    if (IS_NPC(victim)) {
      if (IS_SET(victim->specials.act, ACT_WIMPY))
	do_flee(victim, "", 0);
    } else {
      if (IS_SET(victim->specials.act, PLR_WIMPY)) {
	if (GetMaxLevel(victim) >= 3)
	  REMOVE_BIT(victim->specials.act, PLR_WIMPY);
	act("Super Wimp! Trying to flee!", FALSE, victim, 0, 0, TO_CHAR);
	do_flee(victim, "", 0);
      }
    }
  }
  if ((IS_PC(victim) || (IS_SET(victim->specials.act, ACT_POLYSELF))) && !(victim->desc)) {
    do_flee(victim, "", 0);
    act("$n is rescued by divine forces.", FALSE, victim, 0, 0, TO_ROOM);
    victim->specials.was_in_room = victim->in_room;
    if (victim->in_room != NOWHERE)
      char_from_room(victim);
    char_to_room(victim, 0);
    if (IS_NPC(victim)) {
      do_return(victim, "", 0);
    }
    /*
     * zero_rent(victim);
     * extract_char(victim);
     */
  }
  if (GET_POS(victim) == POSITION_DEAD) {
    if (ch->specials.fighting == victim)
      stop_fighting(ch);
  }
  if (!AWAKE(victim))
    if (victim->specials.fighting)
      stop_fighting(victim);

  if (GET_POS(victim) == POSITION_DEAD) {
    if (IS_NPC(victim) || victim->desc) {
      if (IS_AFFECTED(ch, AFF_GROUP)) {
	group_gain(ch, victim);
      } else {
        exp= IS_PC(victim)?GetMaxLevel(victim)*500:GET_EXP(victim);
        if(IS_SET(ch->specials.new_act, NEW_PLR_KILLOK))
          exp *= 1.10;
#ifdef MOB_LEVELING
        if (IS_NPC(ch) && (!IS_SET(ch->specials.act, ACT_POLYSELF) &&
                           !IS_SET(ch->specials.act, ACT_POLYSELF)))
          exp /= 4;
#endif
        sprintf(buf, "You receive %d experience.", exp);
        act(buf, FALSE, ch, 0, 0, TO_CHAR);
        gain_exp(ch, exp);
	change_alignment(ch, victim);
      }
    }
    if (IS_PC(victim)) {
      random_death_message(ch, victim);
      if (victim->in_room > -1) {
	sprintf(buf, "%s killed by %s at %s",
		GET_NAME(victim),
		(IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)),
		(real_roomp(victim->in_room))->name);
      } else {
	sprintf(buf, "%s killed by %s at Nowhere.",
		GET_NAME(victim),
	      (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
      }
      log(buf);
    }
    die(victim);
    /*
     *  if the victim is dead, return TRUE.
     */
    victim = 0;
    return (TRUE);
  } else {
    return (FALSE);
  }
}

void hit(struct char_data *ch, struct char_data *victim, int type)
{
  struct obj_data *wielded = 0;
  struct obj_data *held = 0;
  struct obj_data *obj = 0;
  int w_type, s_type;
  int victim_ac, calc_thaco;
  int dam, dead;
  BYTE diceroll;
  char buf[256];
  struct room_data *rp;
  int two_handed, chance;

  if (DEBUG)
    dlog("hit");
  rp = real_roomp(ch->in_room);
  if (check_peaceful(ch, "")) {
    sprintf(buf, "hit() called in PEACEFUL room");
    log(buf);
    stop_fighting(ch);
    return;
  }
  if (ch->in_room != victim->in_room) {
    sprintf(buf, "NOT in same room when fighting : %s, %s",
	    ch->player.name, victim->player.name);
    log(buf);
    stop_fighting(ch);
    return;
  }
  if(!IS_IMMORTAL(ch)) {
    if (victim->attackers >= 6 && ch->specials.fighting != victim) {
      cprintf(ch, "You can't attack them,  no room!\n\r");
      return;
    }
    if ((ch->attackers >= 6) && (victim->specials.fighting != ch)) {
      cprintf(ch, "There are too many other people in the way.\n\r");
      return;
    }
  }
  chance = number(1, 50) + (100 - (int)(((double)GET_HIT(ch) / (double)GET_MAX_HIT(ch)) * 100));

  if (chance > (ch->skills[SKILL_ENDURANCE].learned))
    GET_MOVE(ch) -= 1;

  if (GET_MOVE(ch) < -20) {
    GET_MOVE(ch) = -20;
    if (GET_COND(ch, FULL) > 3)
      GET_COND(ch, FULL) -= 1;
  }
  if (victim == ch) {
    if (DoesHate(ch, victim)) {
      RemHated(ch, victim);
    }
    return;
  }
  if (ch->equipment[HOLD])
    held = ch->equipment[HOLD];

  two_handed = 0;

  if (ch->equipment[WIELD] && (ch->equipment[WIELD]->obj_flags.type_flag == ITEM_WEAPON))
    wielded = ch->equipment[WIELD];
  else if (ch->equipment[WIELD_TWOH] &&
  (ch->equipment[WIELD_TWOH]->obj_flags.type_flag == ITEM_WEAPON)) {
    two_handed = 1;
    wielded = ch->equipment[WIELD_TWOH];
  }
  if (wielded) {
    switch (wielded->obj_flags.value[3]) {
    case 0:
      w_type = TYPE_SMITE;
      break;
    case 1:
      w_type = TYPE_STAB;
      break;
    case 2:
      w_type = TYPE_WHIP;
      break;
    case 3:
      w_type = TYPE_SLASH;
      break;
    case 4:
      w_type = TYPE_SMASH;
      break;
    case 5:
      w_type = TYPE_CLEAVE;
      break;
    case 6:
      w_type = TYPE_CRUSH;
      break;
    case 7:
      w_type = TYPE_BLUDGEON;
      break;
    case 8:
      w_type = TYPE_CLAW;
      break;
    case 9:
      w_type = TYPE_BITE;
      break;
    case 10:
      w_type = TYPE_STING;
      break;
    case 11:
      w_type = TYPE_PIERCE;
      break;
    default:
      w_type = TYPE_HIT;
      break;
    }
  } else {
    if (IS_NPC(ch) && (ch->specials.attack_type >= TYPE_HIT))
      w_type = ch->specials.attack_type;
    else
      w_type = TYPE_HIT;
  }

  /* Calculate the raw armor including magic armor */
  /* The lower AC, the better                      */

  if (IS_PC(ch))
    calc_thaco = thaco[(int)BestFightingClass(ch)][(int)GET_LEVEL(ch, BestFightingClass(ch))];
  else
    calc_thaco = 20;

/* THAC0 for monsters is set in the HitRoll */

/*
 * Check for weapon specialization for the character 
 * only if char is one handed wielding.
 */

  s_type = 0;

  if (ch->equipment[WIELD])
    switch (w_type) {
    case TYPE_SMITE:
      s_type = SKILL_SPEC_SMITE;
      break;
    case TYPE_STAB:
      s_type = SKILL_SPEC_STAB;
      break;
    case TYPE_WHIP:
      s_type = SKILL_SPEC_WHIP;
      break;
    case TYPE_SLASH:
      s_type = SKILL_SPEC_SLASH;
      break;
    case TYPE_SMASH:
      s_type = SKILL_SPEC_SMASH;
      break;
    case TYPE_CLEAVE:
      s_type = SKILL_SPEC_CLEAVE;
      break;
    case TYPE_CRUSH:
      s_type = SKILL_SPEC_CRUSH;
      break;
    case TYPE_BLUDGEON:
      s_type = SKILL_SPEC_BLUDGE;
      break;
    case TYPE_PIERCE:
      s_type = SKILL_SPEC_PIERCE;
      break;
    case TYPE_HIT:
      s_type = SKILL_BARE_HAND;
      break;
    default:
      s_type = 0;
      break;
    }
  if (w_type == TYPE_HIT)
    s_type = SKILL_BARE_HAND;
  if (s_type)
    if (number(1, 101) < ch->skills[s_type].learned)
      calc_thaco -= 1;

  calc_thaco -= str_app[STRENGTH_APPLY_INDEX(ch)].tohit;
  calc_thaco -= GET_HITROLL(ch);

  diceroll = number(1, 20);

  victim_ac = GET_AC(victim) / 10;

  if (!AWAKE(victim))
    victim_ac -= dex_app[(int)GET_DEX(victim)].defensive;

  victim_ac = MAX(-10, victim_ac);     /* -10 is lowest */

  if ((diceroll < 20) && AWAKE(victim) &&
      ((diceroll == 1) ||
       ((calc_thaco - diceroll) > victim_ac))) {
    if (type == SKILL_BACKSTAB)
      damage(ch, victim, 0, SKILL_BACKSTAB);
    else {
      damage(ch, victim, 0, w_type);
    }
  } else {
    dam = str_app[STRENGTH_APPLY_INDEX(ch)].todam;

    if (s_type)
      if (number(1, 101) < ch->skills[s_type].learned)
	dam += 2;

    dam += GET_DAMROLL(ch);

    if (!wielded) {
      if (IS_NPC(ch))
	dam += dice(ch->specials.damnodice, ch->specials.damsizedice);
      else {
	dam += number(0, 4);	       /* Max. 2 dam with bare hands */
      }
    } else {
      if (wielded->obj_flags.value[2]) {
	dam += dice(wielded->obj_flags.value[1], wielded->obj_flags.value[2]);
      } else {
	act("$p snaps into peices!", TRUE, ch, wielded, 0, TO_CHAR);
	act("$p snaps into peices!", TRUE, ch, wielded, 0, TO_ROOM);
	if (ch->equipment[WIELD]) {
	  if ((obj = unequip_char(ch, WIELD)) != NULL) {
	    MakeScrap(ch, obj);
	    dam += 1;
	  }
	} else if (ch->equipment[WIELD_TWOH]) {
	  if ((obj = unequip_char(ch, WIELD_TWOH)) != NULL) {
	    MakeScrap(ch, obj);
	    dam += 2;
	  }
	}
      }
    }

    if (GET_POS(victim) < POSITION_FIGHTING)
      dam *= 1 + (POSITION_FIGHTING - GET_POS(victim)) / 3;
    /* Position  sitting  x 1.33 */
    /* Position  resting  x 1.66 */
    /* Position  sleeping x 2.00 */
    /* Position  stunned  x 2.33 */
    /* Position  incap    x 2.66 */
    /* Position  mortally x 3.00 */

    if (GET_POS(victim) <= POSITION_DEAD)
      return;

    dam = MAX(1, dam);		       /* Not less than 0 damage */

    if ((ch->skills[SKILL_TWO_HANDED].learned >= number(1, 101)) && (ch->equipment[WIELD_TWOH]))
      dam += dice(1, 4);

    if (type == SKILL_BACKSTAB) {
      dam *= backstab_mult[(int)GET_LEVEL(ch, THIEF_LEVEL_IND)];
      dead = damage(ch, victim, dam, SKILL_BACKSTAB);
    } else {
      dead = damage(ch, victim, dam, w_type);

      /*
       *  if the victim survives, lets hit him with a 
       *  weapon spell
       */

      if (!dead) {
	WeaponSpell(ch, victim, w_type);
      }
    }
  }
}

/* Control the fights going on */
void perform_violence(int pulse)
{
  struct char_data *ch, *vict;
  int i;

  if (DEBUG)
    dlog("perform_violence");
  for (ch = combat_list; ch; ch = combat_next_dude) {
    struct room_data *rp;

    combat_next_dude = ch->next_fighting;
    assert(ch->specials.fighting);

    rp = real_roomp(ch->in_room);
    if (check_peaceful(ch, "")) {
      char buf[MAX_INPUT_LENGTH];

      sprintf(buf, "perform_violence found %s fighting in a PEACEFUL room.",
	      ch->player.name);
      stop_fighting(ch);
      log(buf);
    } else if (ch == ch->specials.fighting) {
      stop_fighting(ch);
    } else {
      if (IS_NPC(ch)) {
	DevelopHatred(ch, ch->specials.fighting);
      }
      if (AWAKE(ch) && (ch->in_room == ch->specials.fighting->in_room) &&
	  (!IS_AFFECTED(ch, AFF_PARALYSIS))) {
	if (IS_NPC(ch) && IS_SET(ch->specials.act, ACT_FIGHTER_MOVES)) {
	  if (!number(0, 2))
	    Fighter(ch, 0, "");
	  else
	    hit(ch, ch->specials.fighting, TYPE_UNDEFINED);
	} else
	  hit(ch, ch->specials.fighting, TYPE_UNDEFINED);

	if ((IS_PC(ch)) && (HasClass(ch, CLASS_RANGER) || HasClass(ch, CLASS_WARRIOR))) {
	  if (((GET_LEVEL(ch, WARRIOR_LEVEL_IND) > 6) && (ch->mult_att)) ||
	      ((GET_LEVEL(ch, RANGER_LEVEL_IND) > 2) && (ch->mult_att)) ||
	      (GET_LEVEL(ch, WARRIOR_LEVEL_IND) > 12) ||
	      (GET_LEVEL(ch, RANGER_LEVEL_IND) > 12)) {
	    if (ch->specials.fighting)
	      if (ch->in_room == ch->specials.fighting->in_room) {
		hit(ch, ch->specials.fighting, TYPE_UNDEFINED);
		ch->mult_att = FALSE;
	      }
	  } else if (ch->mult_att == FALSE) {
	    ch->mult_att = TRUE;
	  }
	} else if (IS_NPC(ch) && (ch->mult_att > 1)) {
	  for (i = 2; i <= ch->mult_att; i++) {
	    if (ch->specials.fighting) {
	      hit(ch, ch->specials.fighting, TYPE_UNDEFINED);
	    } else {		       /* target has died */
	      vict = FindAnAttacker(ch);
	      if (vict) {
		if (vict->attackers < MAX_ATTACKERS)
		  hit(ch, vict, TYPE_UNDEFINED);
	      }
	    }
	  }
	}
      } else {			       /* Not in same room or not awake */
	stop_fighting(ch);
      }
    }
  }
}

struct char_data *FindVictim(struct char_data *ch)
{
  struct char_data *tmp_ch;
  unsigned char found = FALSE;
  unsigned short ftot = 0, ttot = 0, ctot = 0, ntot = 0, mtot = 0,
        rtot = 0, dtot = 0;
  unsigned short total;
  unsigned short fjump = 0, njump = 0, cjump = 0, mjump = 0, tjump = 0,
        rjump = 0, djump = 0;

  if (DEBUG)
    dlog("FindVictim");

  if (ch->in_room < 0)
    return (0);
  if (!real_roomp(ch->in_room))
    return 0;

  for (tmp_ch = (real_roomp(ch->in_room))->people; tmp_ch; tmp_ch = tmp_ch->next_in_room) {
    if ((CAN_SEE(ch, tmp_ch)) && (!IS_SET(tmp_ch->specials.act, PLR_NOHASSLE)) &&
	(!IS_AFFECTED(tmp_ch, AFF_SNEAK)) && (ch != tmp_ch)) {
      if (!IS_SET(ch->specials.act, ACT_WIMPY) || !AWAKE(tmp_ch)) {
	if (!IS_NPC(tmp_ch) || (IS_SET(tmp_ch->specials.act, ACT_ANNOYING))) {
	  if (!(IS_AFFECTED(ch, AFF_CHARM)) || (ch->master != tmp_ch)) {
	    found = TRUE;	       /* a potential victim has been found */
	    if (!IS_NPC(tmp_ch)) {
	      if (HasClass(tmp_ch, CLASS_DRUID))
		dtot++;
	      if (HasClass(tmp_ch, CLASS_RANGER))
		rtot++;
	      if (HasClass(tmp_ch, CLASS_WARRIOR))
		ftot++;
	      else if (HasClass(tmp_ch, CLASS_CLERIC))
		ctot++;
	      else if (HasClass(tmp_ch, CLASS_MAGIC_USER))
		mtot++;
	      else if (HasClass(tmp_ch, CLASS_THIEF))
		ttot++;
	    } else {
	      ntot++;
	    }
	  }
	}
      }
    }
  }

  /* if no legal enemies have been found, return 0 */

  if (!found) {
    return (0);
  }
  /* 
   * give higher priority to fighters, clerics, thieves, magic users if int <= 12
   * give higher priority to fighters, clerics, magic users thieves is inv > 12
   * give higher priority to magic users, fighters, clerics, thieves if int > 15
   */

  /*
   * choose a target  
   */

  if (ch->abilities.intel <= 3) {
    fjump = 2;
    cjump = 2;
    tjump = 2;
    njump = 2;
    mjump = 2;
    rjump = 2;
    djump = 2;
  } else if (ch->abilities.intel <= 9) {
    fjump = 4;
    rjump = 4;
    djump = 2;
    cjump = 3;
    tjump = 2;
    njump = 2;
    mjump = 1;
  } else if (ch->abilities.intel <= 12) {
    fjump = 3;
    rjump = 3;
    djump = 2;
    cjump = 3;
    tjump = 2;
    njump = 2;
    mjump = 2;
  } else if (ch->abilities.intel <= 15) {
    fjump = 3;
    djump = 3;
    rjump = 3;
    cjump = 3;
    tjump = 2;
    njump = 2;
    mjump = 3;
  } else {
    fjump = 3;
    cjump = 3;
    tjump = 2;
    njump = 1;
    mjump = 3;
    rjump = 3;
    djump = 3;
  }

  total = (fjump * ftot) + (cjump * ctot) + (tjump * ttot) + (njump * ntot) + (mjump * mtot) +
    (rjump * rtot) + (djump * dtot);

  total = number(1, total);

  for (tmp_ch = (real_roomp(ch->in_room))->people; tmp_ch; tmp_ch = tmp_ch->next_in_room) {
    if ((CAN_SEE(ch, tmp_ch)) && (!IS_SET(tmp_ch->specials.act, PLR_NOHASSLE)) &&
	(!IS_AFFECTED(tmp_ch, AFF_SNEAK)) && (ch != tmp_ch)) {
      if (!IS_SET(ch->specials.act, ACT_WIMPY) || !AWAKE(tmp_ch)) {
	if (!IS_NPC(tmp_ch) || (IS_SET(tmp_ch->specials.act, ACT_ANNOYING))) {
	  if (!(IS_AFFECTED(ch, AFF_CHARM)) || (ch->master != tmp_ch)) {
	    if (IS_NPC(tmp_ch)) {
	      total -= njump;
	    } else if (HasClass(tmp_ch, CLASS_WARRIOR)) {
	      total -= fjump;
	    } else if (HasClass(tmp_ch, CLASS_RANGER)) {
	      total -= rjump;
	    } else if (HasClass(tmp_ch, CLASS_DRUID)) {
	      total -= djump;
	    } else if (HasClass(tmp_ch, CLASS_CLERIC)) {
	      total -= cjump;
	    } else if (HasClass(tmp_ch, CLASS_MAGIC_USER)) {
	      total -= mjump;
	    } else {
	      total -= tjump;
	    }
	    if (total <= 0)
	      return (tmp_ch);
	  }
	}
      }
    }
  }

  if (ch->specials.fighting)
    return (ch->specials.fighting);

  return (0);
}

struct char_data *FindAnyVictim(struct char_data *ch)
{
  struct char_data *tmp_ch;
  unsigned char found = FALSE;
  unsigned short ftot = 0, ttot = 0, ctot = 0, ntot = 0, mtot = 0,
        rtot = 0, dtot = 0;
  unsigned short total;
  unsigned short fjump = 0, njump = 0, cjump = 0, mjump = 0, tjump = 0,
        rjump = 0, djump = 0;

  if (DEBUG)
    dlog("FindAnyVictim");

  if (ch->in_room < 0)
    return (0);

  for (tmp_ch = (real_roomp(ch->in_room))->people; tmp_ch; tmp_ch = tmp_ch->next_in_room) {
    if ((CAN_SEE(ch, tmp_ch)) && (!IS_SET(tmp_ch->specials.act, PLR_NOHASSLE))) {
      if (!(IS_AFFECTED(ch, AFF_CHARM)) || (ch->master != tmp_ch)) {
	if (!SameRace(ch, tmp_ch) || (!IS_NPC(tmp_ch))) {
	  found = TRUE;		       /* a potential victim has been found */
	  if (!IS_NPC(tmp_ch)) {
	    if (HasClass(tmp_ch, CLASS_WARRIOR))
	      ftot++;
	    else if (HasClass(tmp_ch, CLASS_RANGER))
	      rtot++;
	    else if (HasClass(tmp_ch, CLASS_DRUID))
	      dtot++;
	    else if (HasClass(tmp_ch, CLASS_CLERIC))
	      ctot++;
	    else if (HasClass(tmp_ch, CLASS_MAGIC_USER))
	      mtot++;
	    else if (HasClass(tmp_ch, CLASS_THIEF))
	      ttot++;
	  } else {
	    ntot++;
	  }
	}
      }
    }
  }

  /* if no legal enemies have been found, return 0 */

  if (!found) {
    return (0);
  }
  /* 
   * give higher priority to fighters, clerics, thieves, magic users if int <= 12
   * give higher priority to fighters, clerics, magic users thieves is inv > 12
   * give higher priority to magic users, fighters, clerics, thieves if int > 15
   */

  /*
   * choose a target  
   */

  if (ch->abilities.intel <= 3) {
    fjump = 2;
    cjump = 2;
    tjump = 2;
    njump = 2;
    mjump = 2;
    djump = 2;
    rjump = 2;
  } else if (ch->abilities.intel <= 9) {
    fjump = 4;
    cjump = 3;
    tjump = 2;
    njump = 2;
    mjump = 1;
    rjump = 4;
    djump = 3;
  } else if (ch->abilities.intel <= 12) {
    fjump = 3;
    cjump = 3;
    tjump = 2;
    njump = 2;
    mjump = 2;
    rjump = 3;
    djump = 2;
  } else if (ch->abilities.intel <= 15) {
    fjump = 3;
    cjump = 3;
    tjump = 2;
    njump = 2;
    mjump = 3;
    rjump = 3;
    djump = 3;
  } else {
    fjump = 3;
    cjump = 3;
    tjump = 2;
    njump = 1;
    mjump = 3;
    rjump = 3;
    djump = 3;
  }

  total = (fjump * ftot) + (cjump * ctot) + (tjump * ttot) + (njump * ntot) + (mjump * mtot) +
    (rjump * rtot) + (djump * dtot);

  total = number(1, total);

  for (tmp_ch = (real_roomp(ch->in_room))->people; tmp_ch; tmp_ch = tmp_ch->next_in_room) {
    if ((CAN_SEE(ch, tmp_ch)) && (!IS_SET(tmp_ch->specials.act, PLR_NOHASSLE))) {
      if (!SameRace(tmp_ch, ch) || (!IS_NPC(tmp_ch))) {
	if (IS_NPC(tmp_ch)) {
	  total -= njump;
	} else if (HasClass(tmp_ch, CLASS_WARRIOR)) {
	  total -= fjump;
	} else if (HasClass(tmp_ch, CLASS_RANGER)) {
	  total -= rjump;
	} else if (HasClass(tmp_ch, CLASS_DRUID)) {
	  total -= djump;
	} else if (HasClass(tmp_ch, CLASS_CLERIC)) {
	  total -= cjump;
	} else if (HasClass(tmp_ch, CLASS_MAGIC_USER)) {
	  total -= mjump;
	} else {
	  total -= tjump;
	}
	if (total <= 0)
	  return (tmp_ch);
      }
    }
  }

  if (ch->specials.fighting)
    return (ch->specials.fighting);

  return (0);

}

int PreProcDam(struct char_data *ch, int type, int dam)
{
  unsigned Our_Bit;

  if (DEBUG)
    dlog("PreProcDam");
  /*
   * long, intricate list, with the various bits and the various spells and
   * such determined
   */

  switch (type) {
  case SPELL_FIREBALL:
  case SPELL_BURNING_HANDS:
  case SPELL_FLAMESTRIKE:
  case SPELL_FIRE_BREATH:
  case SPELL_FIRESHIELD:
    Our_Bit = IMM_FIRE;
    break;

  case SPELL_SHOCKING_GRASP:
  case SPELL_LIGHTNING_BOLT:
  case SPELL_CALL_LIGHTNING:
  case SPELL_LIGHTNING_BREATH:
    Our_Bit = IMM_ELEC;
    break;
  case SPELL_CHILL_TOUCH:
  case SPELL_CONE_OF_COLD:
  case SPELL_ICE_STORM:
  case SPELL_FROST_BREATH:
    Our_Bit = IMM_COLD;
    break;

  case SPELL_MAGIC_MISSILE:
  case SPELL_COLOUR_SPRAY:
  case SPELL_GAS_BREATH:
  case SPELL_METEOR_SWARM:
    Our_Bit = IMM_ENERGY;
    break;

  case SPELL_ENERGY_DRAIN:
    Our_Bit = IMM_DRAIN;
    break;

  case SPELL_ACID_BREATH:
  case SPELL_ACID_BLAST:
    Our_Bit = IMM_ACID;
    break;
  case SKILL_BACKSTAB:
  case TYPE_PIERCE:
  case TYPE_STING:
  case TYPE_STAB:
    Our_Bit = IMM_PIERCE;
    break;
  case TYPE_SLASH:
  case TYPE_WHIP:
  case TYPE_CLEAVE:
  case TYPE_CLAW:
    Our_Bit = IMM_SLASH;
    break;
  case TYPE_BLUDGEON:
  case TYPE_HIT:
  case SKILL_PUNCH:
  case SKILL_KICK:
  case TYPE_CRUSH:
  case TYPE_BITE:
  case TYPE_SMASH:
  case TYPE_SMITE:
    Our_Bit = IMM_BLUNT;
    break;
  case TYPE_HUNGER:
    return dam;
    break;
  case SPELL_POISON:
    Our_Bit = IMM_POISON;
    break;
  default:
    return (dam);
    break;
  }

  if (IS_SET(ch->susc, Our_Bit))
    dam <<= 1;

  if (IS_SET(ch->immune, Our_Bit))
    dam >>= 1;

  if (IS_SET(ch->M_immune, Our_Bit))
    dam >>= 2;

  return (dam);
}

int DamageOneItem(struct char_data *ch, int dam_type, struct obj_data *obj)
{
  int num;

  if (DEBUG)
    dlog("DamageOneItem");

  num = DamagedByAttack(obj, dam_type);
  if (num != 0) {
    cprintf(ch, "%s is %s.\n\r", obj->short_description, ItemDamType[dam_type - 1]);
    if (num == -1) {		       /* destroy object if fail one last save */
      if (!ItemSave(obj, dam_type)) {
	return (TRUE);
      }
    } else {			       /* "damage item"  (armor), (weapon) */
      if (DamageItem(ch, obj, num)) {
	return (TRUE);
      }
    }
  }
  return (FALSE);
}

void MakeScrap(struct char_data *ch, struct obj_data *obj)
{
  char buf[200];
  struct obj_data *t;

  if (DEBUG)
    dlog("MakeScrap");
  act("$p falls to the ground in scraps.", TRUE, ch, obj, 0, TO_CHAR);
  act("$p falls to the ground in scraps.", TRUE, ch, obj, 0, TO_ROOM);

  t = read_object(30, VIRTUAL);

  sprintf(buf, "Scraps from %s lie in a pile here.",
	  obj->short_description);

  t->description = strdup(buf);
  obj_to_room(t, ch->in_room);
  obj_from_char(obj);
  extract_obj(obj);

}

void DamageAllStuff(struct char_data *ch, int dam_type)
{
  int j;
  struct obj_data *obj, *next;

  /* this procedure takes all of the items in equipment and inventory
   * and damages the ones that should be damaged */

  /* equipment */
  if (DEBUG)
    dlog("DamageAllStuff");

  for (j = 0; j < MAX_WEAR; j++) {
    if (ch->equipment[j] && ch->equipment[j]->item_number >= 0) {
      obj = ch->equipment[j];
      if (DamageOneItem(ch, dam_type, obj)) {	/* TRUE == destroyed */
	if ((obj = unequip_char(ch, j)) != NULL) {
	  MakeScrap(ch, obj);
	} else {
	  log("hmm, really wierd!");
	}
      }
    }
  }

  /* inventory */

  obj = ch->carrying;
  while (obj) {
    next = obj->next_content;
    if (obj->item_number >= 0) {
      if (DamageOneItem(ch, dam_type, obj)) {
	MakeScrap(ch, obj);
      }
    }
    obj = next;
  }

}

int DamageItem(struct char_data *ch, struct obj_data *o, int num)
{
  /*  damage weaons or armor */
  if (DEBUG)
    dlog("DamageItem");

  if (ITEM_TYPE(o) == ITEM_ARMOR) {
    o->obj_flags.value[0] -= num;
    if (o->obj_flags.value[0] < 0) {
      return (TRUE);
    }
  } else if (ITEM_TYPE(o) == ITEM_WEAPON) {
    o->obj_flags.value[2] -= num;
    if (o->obj_flags.value[2] <= 0) {
      return (TRUE);
    }
  }
  return (FALSE);
}

int ItemSave(struct obj_data *i, int dam_type)
{
  int num, j;

  if (DEBUG)
    dlog("ItemSave");
  num = number(1, 20);
  if (num <= 1)
    return (FALSE);
  if (num >= 20)
    return (TRUE);

  for (j = 0; j < MAX_OBJ_AFFECT; j++)
    if ((i->affected[j].location == APPLY_SAVING_SPELL) ||
	(i->affected[j].location == APPLY_SAVE_ALL)) {
      num -= i->affected[j].modifier;
    }
  if (i->affected[j].location != APPLY_NONE) {
    num += 1;
  }
  if (i->affected[j].location == APPLY_HITROLL) {
    num += i->affected[j].modifier;
  }
  if (ITEM_TYPE(i) != ITEM_ARMOR)
    num += 1;

  if (num <= 1)
    return (FALSE);
  if (num >= 20)
    return (TRUE);

  if (num >= ItemSaveThrows[(int)GET_ITEM_TYPE(i) - 1][dam_type - 1]) {
    return (TRUE);
  } else {
    return (FALSE);
  }
}

int DamagedByAttack(struct obj_data *i, int dam_type)
{
  int num = 0;

  if (DEBUG)
    dlog("DamagedByAttack");
  if ((ITEM_TYPE(i) == ITEM_ARMOR) || (ITEM_TYPE(i) == ITEM_WEAPON)) {
    while (!ItemSave(i, dam_type)) {
      num += 1;
    }
    return (num);
  } else {
    if (ItemSave(i, dam_type)) {
      return (0);
    } else {
      return (-1);
    }
  }
}

int WeaponCheck(struct char_data *ch, struct char_data *v, int type, int dam)
{
  int Immunity, total, j;

  if (DEBUG)
    dlog("WeaponCheck");
  Immunity = -1;
  if (IS_SET(ch->M_immune, IMM_NONMAG)) {
    Immunity = 0;
  }
  if (IS_SET(ch->M_immune, IMM_PLUS1)) {
    Immunity = 1;
  }
  if (IS_SET(ch->M_immune, IMM_PLUS2)) {
    Immunity = 2;
  }
  if (IS_SET(ch->M_immune, IMM_PLUS3)) {
    Immunity = 3;
  }
  if (IS_SET(ch->M_immune, IMM_PLUS4)) {
    Immunity = 4;
  }
  if (Immunity < 0)
    return (dam);

  if ((type < TYPE_HIT) || (type > TYPE_SMITE)) {
    return (dam);
  } else {
    if (type == TYPE_HIT) {
      if (IS_NPC(ch) && (GetMaxLevel(ch) > (3 * Immunity) + 1)) {
	return (dam);
      } else {
	return (0);
      }
    } else {
      total = 0;
      if (!ch->equipment[WIELD] && !ch->equipment[WIELD_TWOH])
	return (0);
      if (ch->equipment[WIELD_TWOH]) {
	for (j = 0; j < MAX_OBJ_AFFECT; j++)
	  if ((ch->equipment[WIELD_TWOH]->affected[j].location == APPLY_HITROLL) ||
	      (ch->equipment[WIELD_TWOH]->affected[j].location == APPLY_HITNDAM)) {
	    total += ch->equipment[WIELD_TWOH]->affected[j].modifier;
	  }
      } else if (ch->equipment[WIELD]) {
	for (j = 0; j < MAX_OBJ_AFFECT; j++)
	  if ((ch->equipment[WIELD]->affected[j].location == APPLY_HITROLL) ||
	      (ch->equipment[WIELD]->affected[j].location == APPLY_HITNDAM)) {
	    total += ch->equipment[WIELD]->affected[j].modifier;
	  }
      }
      if (total > Immunity) {
	return (dam);
      } else {
	return (0);
      }
    }
  }
}

void DamageStuff(struct char_data *v, int type, int dam)
{
  int num, dam_type;
  struct obj_data *obj;

  if (DEBUG)
    dlog("DamageStuff");
  if (type >= TYPE_HIT && type <= TYPE_SMITE) {
    num = number(3, 18);	       /* wear_neck through wield_twoh */
    if (v->equipment[num]) {
      if ((type == TYPE_BLUDGEON && dam > 10) ||
	  (type == TYPE_CRUSH && dam > 8) ||
	  (type == TYPE_SMASH && dam > 10) ||
	  (type == TYPE_BITE && dam > 15) ||
	  (type == TYPE_CLAW && dam > 20) ||
	  (type == TYPE_SLASH && dam > 20) ||
	  (type == TYPE_SMITE && dam > 10) ||
	  (type == TYPE_HIT && dam > 12)) {
	if (DamageOneItem(v, BLOW_DAMAGE, v->equipment[num])) {
	  if ((obj = unequip_char(v, num)) != NULL) {
	    MakeScrap(v, obj);
	  }
	}
      }
    }
  } else {
    dam_type = GetItemDamageType(type);
    if (dam_type) {
      DamageAllStuff(v, dam_type);
    }
  }
}

int GetItemDamageType(int type)
{
  if (DEBUG)
    dlog("GetItemDmageType");
  switch (type) {
  case SPELL_FIREBALL:
  case SPELL_FLAMESTRIKE:
  case SPELL_FIRE_BREATH:
  case SPELL_FIRESHIELD:
    return (FIRE_DAMAGE);
    break;

  case SPELL_LIGHTNING_BOLT:
  case SPELL_CALL_LIGHTNING:
  case SPELL_LIGHTNING_BREATH:
    return (ELEC_DAMAGE);
    break;
  case SPELL_CONE_OF_COLD:
  case SPELL_ICE_STORM:
  case SPELL_FROST_BREATH:
    return (COLD_DAMAGE);
    break;

  case SPELL_COLOUR_SPRAY:
  case SPELL_METEOR_SWARM:
  case SPELL_GAS_BREATH:
    return (BLOW_DAMAGE);
    break;

  case SPELL_ACID_BREATH:
  case SPELL_ACID_BLAST:
    return (ACID_DAMAGE);
  default:
    return (0);
    break;
  }
}

int SkipImmortals(struct char_data *v, int amnt)
{
  /* You can't damage an immortal! */
  if (DEBUG)
    dlog("SkipImmortals");
/* I disagree.... very much so!
 *
 * if ((GetMaxLevel(v) > MAX_MORT) && IS_PC(v))
 *   amnt = 0;
 */

  /* special type of monster */

  if (IS_NPC(v) && (IS_SET(v->specials.act, ACT_IMMORTAL))) {
    amnt = -1;
  }
  return (amnt);
}

int CanKill(struct char_data *ch, struct char_data *vict, char *msg)
{
  if(!ch || !vict)
    return FALSE;
  if(GET_POS(vict) <= POSITION_DEAD)
    return FALSE;
  if (IS_PC(ch) && IS_PC(vict)) {
    if ((IS_SET(ch->specials.new_act, NEW_PLR_KILLOK) &&
	 IS_SET(vict->specials.new_act, NEW_PLR_KILLOK)) ||
	(IS_IMMORTAL(ch)))
      return (TRUE);
    else {
      if (ch != vict) {
	cprintf(ch, msg, NAME(vict));
	return (FALSE);
      }
    }
    return (FALSE);
  } else
    return (TRUE);
}

inline int CheckKill(struct char_data *ch, struct char_data *vict) {
  return CanKill(ch, vict, "It doesn't affect %s.\n\r");
}

void WeaponSpell(struct char_data *c, struct char_data *v, int type)
{
  int j, num;

  if (number(1, 100) >= 95) {
    if ((c->in_room == v->in_room) && (GET_POS(v) != POSITION_DEAD)) {
      if (c->equipment[WIELD]) {
	for (j = 0; j < MAX_OBJ_AFFECT; j++)
	  if (c->equipment[WIELD]->affected[j].location == APPLY_WEAPON_SPELL) {
	    num = c->equipment[WIELD]->affected[j].modifier;
	    ((*spell_info[num].spell_pointer) (6, c, "", SPELL_TYPE_SPELL, v, 0));
	  }
      } else if (c->equipment[WIELD_TWOH]) {
	for (j = 0; j < MAX_OBJ_AFFECT; j++)
	  if (c->equipment[WIELD_TWOH]->affected[j].location == APPLY_WEAPON_SPELL) {
	    num = c->equipment[WIELD_TWOH]->affected[j].modifier;
	    ((*spell_info[num].spell_pointer) (6, c, "", SPELL_TYPE_SPELL, v, 0));
	  }
      }
    }
  }
}

struct char_data *FindAnAttacker(struct char_data *ch)
{
  struct char_data *tmp_ch;
  unsigned char found = FALSE;
  unsigned short ftot = 0, ttot = 0, ctot = 0, ntot = 0, mtot = 0,
        rtot = 0, dtot = 0;
  unsigned short total;
  unsigned short fjump = 0, njump = 0, cjump = 0, mjump = 0, tjump = 0,
        rjump = 0, djump = 0;

  if (DEBUG)
    dlog("FindAnAttacker");
  if (ch->in_room < 0)
    return (0);

  for (tmp_ch = (real_roomp(ch->in_room))->people; tmp_ch;
       tmp_ch = tmp_ch->next_in_room) {
    if ((CAN_SEE(ch, tmp_ch)) && (!IS_SET(tmp_ch->specials.act, PLR_NOHASSLE)) &&
	(!IS_AFFECTED(tmp_ch, AFF_SNEAK)) && (ch != tmp_ch)) {
      if (!IS_SET(ch->specials.act, ACT_WIMPY) || !AWAKE(tmp_ch)) {
	if (!IS_NPC(tmp_ch) || (IS_SET(tmp_ch->specials.act, ACT_ANNOYING))) {
	  if (!(IS_AFFECTED(ch, AFF_CHARM)) || (ch->master != tmp_ch)) {
	    if (tmp_ch->specials.fighting == ch) {
	      found = TRUE;	       /* a potential victim has been found */
	      if (!IS_NPC(tmp_ch)) {
		if (HasClass(tmp_ch, CLASS_WARRIOR))
		  ftot++;
		else if (HasClass(tmp_ch, CLASS_RANGER))
		  rtot++;
		else if (HasClass(tmp_ch, CLASS_DRUID))
		  dtot++;
		else if (HasClass(tmp_ch, CLASS_CLERIC))
		  ctot++;
		else if (HasClass(tmp_ch, CLASS_MAGIC_USER))
		  mtot++;
		else if (HasClass(tmp_ch, CLASS_THIEF))
		  ttot++;
	      } else {
		ntot++;
	      }
	    }
	  }
	}
      }
    }
  }

  /* if no legal enemies have been found, return 0 */

  if (!found) {
    return (0);
  }
  /* 
   * give higher priority to fighters, clerics, thieves, magic users if int <= 12
   * give higher priority to fighters, clerics, magic users thieves is inv > 12
   * give higher priority to magic users, fighters, clerics, thieves if int > 15
   */

  /*
   * choose a target  
   */

  if (ch->abilities.intel <= 3) {
    fjump = 2;
    cjump = 2;
    tjump = 2;
    njump = 2;
    mjump = 2;
    rjump = 3;
    djump = 2;
  } else if (ch->abilities.intel <= 9) {
    fjump = 4;
    cjump = 3;
    tjump = 2;
    njump = 2;
    mjump = 1;
    rjump = 4;
    djump = 2;
  } else if (ch->abilities.intel <= 12) {
    fjump = 3;
    cjump = 3;
    tjump = 2;
    njump = 2;
    mjump = 2;
    rjump = 3;
    djump = 2;
  } else if (ch->abilities.intel <= 15) {
    fjump = 3;
    cjump = 3;
    tjump = 2;
    njump = 2;
    mjump = 3;
    rjump = 3;
    djump = 2;
  } else {
    fjump = 3;
    cjump = 3;
    tjump = 2;
    njump = 1;
    mjump = 3;
    rjump = 3;
    djump = 3;
  }

  total = (fjump * ftot) + (cjump * ctot) + (tjump * ttot) + (njump * ntot) + (mjump * mtot) +
    (rjump * rtot) + (djump * dtot);

  total = number(1, total);

  for (tmp_ch = (real_roomp(ch->in_room))->people; tmp_ch;
       tmp_ch = tmp_ch->next_in_room) {
    if ((CAN_SEE(ch, tmp_ch)) && (!IS_SET(tmp_ch->specials.act, PLR_NOHASSLE)) &&
	(!IS_AFFECTED(tmp_ch, AFF_SNEAK)) && (ch != tmp_ch)) {
      if (!IS_SET(ch->specials.act, ACT_WIMPY) || !AWAKE(tmp_ch)) {
	if (!IS_NPC(tmp_ch) || (IS_SET(tmp_ch->specials.act, ACT_ANNOYING))) {
	  if (!(IS_AFFECTED(ch, AFF_CHARM)) || (ch->master != tmp_ch)) {
	    if (tmp_ch->specials.fighting == ch) {
	      if (IS_NPC(tmp_ch)) {
		total -= njump;
	      } else if (HasClass(tmp_ch, CLASS_WARRIOR)) {
		total -= fjump;
	      } else if (HasClass(tmp_ch, CLASS_DRUID)) {
		total -= djump;
	      } else if (HasClass(tmp_ch, CLASS_RANGER)) {
		total -= rjump;
	      } else if (HasClass(tmp_ch, CLASS_CLERIC)) {
		total -= cjump;
	      } else if (HasClass(tmp_ch, CLASS_MAGIC_USER)) {
		total -= mjump;
	      } else {
		total -= tjump;
	      }
	      if (total <= 0)
		return (tmp_ch);
	    }
	  }
	}
      }
    }
  }

  if (ch->specials.fighting)
    return (ch->specials.fighting);

  return (0);
}

#if 0
void shoot(struct char_data *ch, struct char_data *victim)
{

  /*
   * **  check for bow and arrow.
   */

  bow = ch->equipment[HOLD];
  arrow = ch->equipment[WIELD];

  if (!bow) {
    cprintf(ch, "You need a bow-like weapon\n\r");
    return;
  } else if (!arrow) {
    cprintf(ch, "You need a projectile to shoot!\n\r");
  } else if (!bow && !arrow) {
    cprintf(ch, "You need a bow-like item, and a projectile to shoot!\n\r");
  } else {
    arrowVnum = ObjVnum(arrow);
    found = FALSE;
    for (i = 0; i < 4 && !founde; i++) {
      if (bow->obj_flags.value[i] == arrowVnum) {
	found = TRUE;
      }
    }
    if (!found) {
      cprintf(ch, "That projectile does not fit in that projector.\n\r");
      return;
    }
    /*
     * **  check for bonuses on the bow.
     */
    for (j = 0; j < MAX_OBJ_AFFECT; j++)
      if (bow->affected[j].location ==
	  APPLY_ARROW_HIT_PLUS) {
	hitbon += bow->affected[j].modifier;
      } else if (bow->affected[j].location ==
		 APPLY_ARROW_DAM_PLUS) {
	dambon += bow->affected[j].modifier;
      }
    /*
     * **   temporarily add those bonuses.
     */
    /*
     * **   fire the weapon.
     */
  }
}
#endif

int SwitchTargets(struct char_data *ch, struct char_data *vict)
{
  if (DEBUG)
    dlog("SwitchTargets");
  if (!ch->specials.fighting) {
    hit(ch, vict, TYPE_UNDEFINED);
    return (TRUE);
  }
  if (ch->specials.fighting != vict) {
    if (ch->specials.fighting->specials.fighting == ch) {
      cprintf(ch, "You can't shoot weapons at close range!\n\r");
      return (FALSE);
    } else {
      stop_fighting(ch);
      hit(ch, vict, TYPE_UNDEFINED);
    }
  } else {
    hit(ch, vict, TYPE_UNDEFINED);
    return (TRUE);
  }
  return (FALSE);
}

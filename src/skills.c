/*
 * holds all of the new skills that i have designed....
 * Much thanks to Whitegold of  epic Dikumud for the hunt code.
 */

#include "structs.h"
#include "utils.h"
#include "race.h"
#include "spells.h"
#include "comm.h"
#include "handler.h"

int                              choose_exit(int in_room, int tgt_room, int dvar);
struct room_data                *real_roomp(int);

const char                      *dirs[];
extern struct char_data         *character_list;
extern struct room_data         *world;
extern struct dex_app_type       dex_app[];

struct hunting_data {
  char                            *name;
  struct char_data               **victim;
};

/*
 * **  Disarm:
 */

void 
do_disarm(struct char_data *ch, char *argument, int cmd)
{
  char                             name[30];
  int                              percent;
  struct char_data                *victim;
  struct obj_data                 *w;
  int                              chance;
  int                              cost;

  if (check_peaceful(ch, "You feel too peaceful to contemplate violence.\n\r"))
    return;

  only_argument(argument, name);
  if (!(victim = get_char_room_vis(ch, name))) {
    if (ch->specials.fighting) {
      victim = ch->specials.fighting;
    } else {
      send_to_char("Disarm who?\n\r", ch);
      return;
    }
  }
  if (victim == ch) {
    send_to_char("Aren't we funny today...\n\r", ch);
    return;
  }
  if (!CheckKill(ch, victim))
    return;

  if (ch->attackers > 3) {
    send_to_char("There is no room to disarm!\n\r", ch);
    return;
  }
  if (victim->attackers > 3) {
    send_to_char("There is no room to disarm!\n\r", ch);
    return;
  }
  cost = 25 - (GET_LEVEL(ch, BestFightingClass(ch)) / 10);

  if (GET_MANA(ch) < cost) {
    send_to_char("You trip and fall while trying to disarm.\n\r", ch);
    return;
  }
  percent = number(1, 101);	/*
				 * 101% is a complete failure 
				 */
  percent -= dex_app[GET_DEX(ch)].reaction;
  percent += dex_app[GET_DEX(victim)].reaction;

  if (!ch->equipment[WIELD] && !ch->equipment[WIELD_TWOH]) {
    percent -= 50;
  }
  if (percent > ch->skills[SKILL_DISARM].learned) {
    /*
     * failure   
     */

    GET_MANA(ch) -= 10;
    act("You try to disarm $N, but fail miserably.", TRUE, ch, 0, victim, TO_CHAR);
    if ((ch->equipment[WIELD]) && (number(1, 10) > 8)) {
      send_to_char("Your weapon flies from your hand while trying!\n\r", ch);
      w = unequip_char(ch, WIELD);
      obj_from_char(w);
      obj_to_room(w, ch->in_room);
      act("$n tries to disarm $N, but $n loses his weapon!", TRUE, ch, 0, victim, TO_ROOM);
    } else if ((ch->equipment[WIELD_TWOH]) && (number(1, 10) > 9)) {
      send_to_char("Your weapon slips from your hands while trying!\n\r", ch);
      w = unequip_char(ch, WIELD_TWOH);
      obj_from_char(w);
      obj_to_room(w, ch->in_room);
      act("$n tries to disarm $N, but $n loses his weapon!", TRUE, ch, 0, victim, TO_ROOM);
    }
    GET_POS(ch) = POSITION_SITTING;

    if ((IS_NPC(victim)) && (GET_POS(victim) > POSITION_SLEEPING) && (!victim->specials.fighting)) {
      set_fighting(victim, ch);
    }
    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
  } else {
    if (victim->equipment[WIELD]) {
      GET_MANA(ch) -= 25;
      w = unequip_char(victim, WIELD);
      act("$n makes an impressive fighting move.", TRUE, ch, 0, 0, TO_ROOM);
      act("You send $p flying from $N's grasp.", TRUE, ch, w, victim, TO_CHAR);
      act("$p flies from your grasp.", TRUE, ch, w, victim, TO_VICT);
      obj_from_char(w);
      obj_to_room(w, victim->in_room);
      if (ch->skills[SKILL_DISARM].learned < 50)
	ch->skills[SKILL_DISARM].learned += 2;
    } else if (victim->equipment[WIELD_TWOH]) {
      GET_MANA(ch) -= cost;
      if (IS_NPC(victim))
	chance = 70;
      else
	chance = victim->skills[SKILL_TWO_HANDED].learned;

      percent = number(1, 101);	/*
				 * 101% is a complete failure 
				 */
      if (percent > chance) {
	w = unequip_char(victim, WIELD_TWOH);
	act("$n makes a very impressive fighting move.", TRUE, ch, 0, 0, TO_ROOM);
	act("You send $p flying from $N's grasp.", TRUE, ch, w, victim, TO_CHAR);
	act("$p flies from your grasp.", TRUE, ch, w, victim, TO_VICT);
	obj_from_char(w);
	obj_to_room(w, victim->in_room);
	if (ch->skills[SKILL_DISARM].learned < 50)
	  ch->skills[SKILL_DISARM].learned += 4;
      } else {
	act("You try to disarm $N, but fail miserably.", TRUE, ch, 0, victim, TO_CHAR);
      }
    } else {
      act("You try to disarm $N, but $E doesn't have a weapon.",
	  TRUE, ch, 0, victim, TO_CHAR);
      act("$n makes an impressive fighting move, but does little more.",
	  TRUE, ch, 0, 0, TO_ROOM);
    }

    if ((IS_NPC(victim)) && (GET_POS(victim) > POSITION_SLEEPING) &&
	(!victim->specials.fighting)) {
      set_fighting(victim, ch);
    }
    WAIT_STATE(ch, PULSE_VIOLENCE * 1);
  }
}

/*
 * **   Track:
 */

int 
named_mobile_in_room(int room, struct hunting_data *c_data)
{
  struct char_data                *scan;

  for (scan = real_roomp(room)->people; scan; scan = scan->next_in_room)
    if (isname(c_data->name, scan->player.name)) {
      *(c_data->victim) = scan;
      return 1;
    }
  return 0;
}

void 
do_peer(struct char_data *ch, char *argument, int cmd)
{
  void                             do_look(struct char_data *ch, char *arg, int cmd);

  if (GET_MANA(ch) < (15 - GET_LEVEL(ch, BestThiefClass(ch)) / 4)) {
    send_to_char("You don't really see anything...\n\r", ch);
    return;
  }
  if (!*argument) {
    send_to_char("You must peer in a direction...\n\r", ch);
    return;
  }
  if (ch->skills[SKILL_PEER].learned < number(1, 101)) {
    do_look(ch, argument, 0);
    GET_MANA(ch) -= 5;
    return;
  }
  GET_MANA(ch) -= (20 - GET_LEVEL(ch, BestThiefClass(ch)) / 4);
  if (ch->skills[SKILL_PEER].learned < 50)
    ch->skills[SKILL_PEER].learned += 2;

  act("$n peers about the area.", TRUE, ch, 0, 0, TO_ROOM);

  do_look(ch, argument, SKILL_PEER);
}

void 
do_track(struct char_data *ch, char *argument, int cmd)
{
  char                             name[256],
                                   buf[256];
  int                              dist,
                                   code;
  struct hunting_data              huntd;
  int                              cost;

  only_argument(argument, name);

  dist = ch->skills[SKILL_HUNT].learned;

  if (IS_SET(ch->player.class, CLASS_RANGER)) {
    dist *= 2;
    cost = 15 - (GET_LEVEL(ch, RANGER_LEVEL_IND) / 10);
  } else if (IS_SET(ch->player.class, CLASS_THIEF)) {
    cost = 50 - GET_LEVEL(ch, THIEF_LEVEL_IND);
    dist = dist;
  } else {
    dist = dist / 2;
    cost = 50 - (GET_LEVEL(ch, BestThiefClass(ch)) / 2);
  }

  if (!dist) {
    send_to_char("You do not know of this skill yet!\n\r", ch);
    return;
  }
  if (GET_MANA(ch) < cost) {
    send_to_char("You can not seem to concentrate on the trail...\n\r", ch);
    return;
  }
  GET_MANA(ch) -= cost;

  switch (GET_RACE(ch)) {
  case RACE_ELVEN:
    dist += 10;			/*
				 * even better 
				 */
    break;
  case RACE_DEVIL:
  case RACE_DEMON:
    dist = MAX_ROOMS;		/*
				 * as good as can be 
				 */
    break;
  default:
    break;
  }

  if (GetMaxLevel(ch) >= IMMORTAL)
    dist = MAX_ROOMS;

  ch->hunt_dist = dist;

  ch->specials.hunting = 0;
  huntd.name = name;
  huntd.victim = &ch->specials.hunting;
  code = find_path(ch->in_room, named_mobile_in_room, &huntd, -dist);

  WAIT_STATE(ch, PULSE_VIOLENCE * 1);

  if (code == -1) {
    send_to_char("You are unable to find traces of one.\n\r", ch);
    return;
  } else {
    if (IS_LIGHT(ch->in_room)) {
      SET_BIT(ch->specials.act, PLR_HUNTING);
      sprintf(buf, "You see traces of your quarry to the %s\n\r", dirs[code]);
      send_to_char(buf, ch);
      if (ch->skills[SKILL_HUNT].learned < 50)
	ch->skills[SKILL_HUNT].learned += 2;
    } else {
      ch->specials.hunting = 0;
      send_to_char("It's too dark in here to track...\n\r", ch);
      return;
    }
  }
}

int 
track(struct char_data *ch, struct char_data *vict)
{

  char                             buf[256];
  int                              code;

  if ((!ch) || (!vict))
    return (-1);

  code = choose_exit(ch->in_room, vict->in_room, ch->hunt_dist);

  if ((!ch) || (!vict))
    return (-1);

  if (ch->in_room == vict->in_room) {
    send_to_char("\n\rTrack -> You have found your target!\n\r", ch);
    return (FALSE);		/*
				 * false to continue the hunt 
				 */
  }
  if (code == -1) {
    send_to_char("\n\rTrack -> You have lost the trail.\n\r", ch);
    return (FALSE);
  } else {
    sprintf(buf, "\n\rTrack -> You see a faint trail %sward\n\r", dirs[code]);
    send_to_char(buf, ch);
    return (TRUE);
  }
}

int 
dir_track(struct char_data *ch, struct char_data *vict)
{
  char                             buf[256];
  int                              code;

  if ((!ch) || (!vict))
    return (-1);

  code = choose_exit(ch->in_room, vict->in_room, ch->hunt_dist);

  if ((!ch) || (!vict))
    return (-1);

  if (code == -1) {
    if (ch->in_room == vict->in_room) {
      send_to_char("\n\rTrack -> You have found your target!\n\r", ch);
    } else {
      send_to_char("\n\rTrack -> You have lost the trail.\n\r", ch);
    }
    return (-1);		/*
				 * false to continue the hunt 
				 */
  } else {
    sprintf(buf, "\n\rTrack -> You see a faint trail %sward\n\r", dirs[code]);
    send_to_char(buf, ch);
    return (code);
  }

}

int 
RideCheck(struct char_data *ch)
{
  if (IS_AFFECTED(ch, AFF_RIDE)) {
    return (TRUE);
  }
  if (IS_NPC(ch)) {
    if (ch->skills[SKILL_RIDE].learned < 50)
      ch->skills[SKILL_RIDE].learned = 75;
  }
  if (number(1, 100) > ch->skills[SKILL_RIDE].learned) {
    if (ch->skills[SKILL_RIDE].learned < 50) {
      if (GET_LEVEL(ch, RANGER_LEVEL_IND) > 0)
	ch->skills[SKILL_RIDE].learned += 6;
      else
	ch->skills[SKILL_RIDE].learned += 2;
    }
    return (FALSE);
  }
  return (TRUE);
}

void 
FallOffMount(struct char_data *ch, struct char_data *h)
{
  act("$n loses control and falls off of $N", FALSE, ch, 0, h, TO_NOTVICT);
  act("$n loses control and falls off of you", FALSE, ch, 0, h, TO_VICT);
  act("You lose control and fall off of $N", FALSE, ch, 0, h, TO_CHAR);
}

int 
Dismount(struct char_data *ch, struct char_data *h, int pos)
{
  MOUNTED(ch) = 0;
  RIDDEN(h) = 0;
  GET_POS(ch) = pos;
  check_falling(ch);
}

int 
MountEgoCheck(struct char_data *rider, struct char_data *mount)
{
  int                              class_ok;
  int                              diff;
  int                              chance;

  diff = GET_ALIGNMENT(rider) - GET_ALIGNMENT(mount);
  if (diff < 0)
    diff = -diff;

  if (diff >= 1500)
    chance = 7;
  else if (diff >= 800)
    chance = 6;
  else if (diff >= 700)
    chance = 5;
  else if (diff >= 400)
    chance = 4;
  else if (diff > 300)
    chance = 3;
  else if (diff >= 100)
    chance = 2;
  else
    chance = 0;

  if (GET_LEVEL(rider, RANGER_LEVEL_IND) > 0)
    chance -= 2;

  chance += (GetMaxLevel(mount) / 5);
  chance -= (GetMaxLevel(rider) / 4);

  if (IS_SET(mount->specials.act, ACT_AGGRESSIVE))
    chance += 2;

  switch (GET_INT(mount)) {
  case 15:
    chance++;
    break;
  case 16:
    chance += 2;
    break;
  case 17:
    chance += 3;
    break;
  case 18:
    chance += 4;
    break;
  }

  return (chance);
}

void 
do_mount(struct char_data *ch, char *arg, int cmd)
{
  char                             buf[256];
  char                             name[112];
  int                              check;
  struct char_data                *horse;

  if (IS_AFFECTED(ch, AFF_FLYING)) {
    send_to_char("You can't, you are flying!\n\r", ch);
    return;
  }
  if (cmd == 269) {
    only_argument(arg, name);
    if (!(horse = get_char_room_vis(ch, name))) {
      send_to_char("Mount what?\n\r", ch);
      return;
    }
    if (!IsHumanoid(ch)) {
      send_to_char("You can't ride things!\n\r", ch);
      return;
    }
    if (IS_SET(horse->specials.act, ACT_MOUNT)) {
      if (GET_POS(horse) < POSITION_STANDING) {
	send_to_char("Your mount must be standing\n\r", ch);
	return;
      }
      if (RIDDEN(horse)) {
	send_to_char("Already ridden\n\r", ch);
	return;
      }
      if (MOUNTED(ch)) {
	send_to_char("Already riding\n\r", ch);
	return;
      }
      if (GetMaxLevel(horse) > 3)
	check = MountEgoCheck(ch, horse);
      else
	check = -1;

      if (check >= 6) {
	act("$N snarls and attacks!", FALSE, ch, 0, horse, TO_CHAR);
	act("as $n tries to mount $N, $N attacks $n!", FALSE, ch, 0, horse, TO_NOTVICT);
	WAIT_STATE(ch, PULSE_VIOLENCE);
	hit(horse, ch, TYPE_UNDEFINED);
	return;
      }
      if (check >= 4) {
	act("$N moves out of the way, you fall on your butt", FALSE, ch, 0, horse, TO_CHAR);
	act("as $n tries to mount $N, $N moves out of the way", FALSE, ch, 0, horse, TO_NOTVICT);
	WAIT_STATE(ch, PULSE_VIOLENCE);
	GET_POS(ch) = POSITION_SITTING;
	return;
      }
      if (RideCheck(ch)) {
	act("You hop on $N's back", FALSE, ch, 0, horse, TO_CHAR);
	act("$n hops on $N's back", FALSE, ch, 0, horse, TO_NOTVICT);
	act("$n hops on your back!", FALSE, ch, 0, horse, TO_VICT);
	MOUNTED(ch) = horse;
	RIDDEN(horse) = ch;
	GET_POS(ch) = POSITION_MOUNTED;
	REMOVE_BIT(ch->specials.affected_by, AFF_SNEAK);
      } else {
	act("You try to ride $N, but fall on your butt",
	    FALSE, ch, 0, horse, TO_CHAR);
	act("$n tries to ride $N, but falls on $s butt",
	    FALSE, ch, 0, horse, TO_NOTVICT);
	act("$n tries to ride you, but falls on $s butt",
	    FALSE, ch, 0, horse, TO_VICT);
	GET_POS(ch) = POSITION_SITTING;
	WAIT_STATE(ch, PULSE_VIOLENCE * 2);
      }
    } else {
      send_to_char("You can't ride that!\n\r", ch);
      return;
    }
  } else if (cmd == 270) {
    horse = MOUNTED(ch);
    act("You dismount from $N", FALSE, ch, 0, horse, TO_CHAR);
    act("$n dismounts from $N", FALSE, ch, 0, horse, TO_NOTVICT);
    act("$n dismounts from you", FALSE, ch, 0, horse, TO_VICT);
    Dismount(ch, MOUNTED(ch), POSITION_STANDING);
    return;
  } else {
    send_to_char("Hmmmmmm, don't think you mounted on anything?\n\r", ch);
  }
}

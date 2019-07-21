/* 
 * holds all of the new skills that i have designed....
 * Much thanks to Whitegold of  epic Dikumud for the hunt code.
 */

#include <stdio.h>
#include <stdlib.h>
/* #include <unistd.h> */

#include "global.h"
#include "bug.h"
#include "utils.h"

#include "comm.h"
#include "db.h"
#include "handler.h"
#include "interpreter.h"
#include "multiclass.h"
#include "spells.h"
#include "spell_parser.h"
#include "fight.h"

#include "spec_procs.h"
#include "act_info.h"
#define _ACT_SKILLS_C
#include "act_skills.h"

/*
 * **  Disarm:
 */

void do_disarm(struct char_data *ch, const char *argument, int cmd)
{
    char                                    name[30] = "\0\0\0\0\0\0\0";
    int                                     percent_chance = 0;
    struct char_data                       *victim = NULL;
    struct obj_data                        *w = NULL;
    int                                     chance = 0;
    int                                     cost = 0;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (check_peaceful(ch, "You feel too peaceful to contemplate violence.\r\n"))
	return;

    only_argument(argument, name);
    if (!(victim = get_char_room_vis(ch, name))) {
	if (ch->specials.fighting) {
	    victim = ch->specials.fighting;
	} else {
	    cprintf(ch, "Disarm who?\r\n");
	    return;
	}
    }
    if (victim == ch) {
	cprintf(ch, "Aren't we funny today...\r\n");
	return;
    }
    if (!CheckKill(ch, victim))
	return;

    if (ch->attackers > 3) {
	cprintf(ch, "There is no room to disarm!\r\n");
	return;
    }
    if (victim->attackers > 3) {
	cprintf(ch, "There is no room to disarm!\r\n");
	return;
    }
    cost = 25 - (GET_LEVEL(ch, BestFightingClass(ch)) / 10);

    if (GET_MANA(ch) < cost) {
	cprintf(ch, "You trip and fall while trying to disarm.\r\n");
	return;
    }
    percent_chance = number(1, 101);			       /* 101% is a complete failure */
    percent_chance -= dex_app[(int)GET_DEX(ch)].reaction;
    percent_chance += dex_app[(int)GET_DEX(victim)].reaction;

    if (!ch->equipment[WIELD] && !ch->equipment[WIELD_TWOH]) {
	percent_chance -= 50;
    }
    if (percent_chance > ch->skills[SKILL_DISARM].learned) {
	/*
	 * failure 
	 */

	GET_MANA(ch) -= 10;
	act("You try to disarm $N, but fail miserably.", TRUE, ch, 0, victim, TO_CHAR);
	if ((ch->equipment[WIELD]) && (number(1, 10) > 8)) {
	    cprintf(ch, "Your weapon flies from your hand while trying!\r\n");
	    w = unequip_char(ch, WIELD);
	    obj_from_char(w);
	    obj_to_room(w, ch->in_room);
	    act("$n tries to disarm $N, but $n loses his weapon!", TRUE, ch, 0, victim,
		TO_ROOM);
	} else if ((ch->equipment[WIELD_TWOH]) && (number(1, 10) > 9)) {
	    cprintf(ch, "Your weapon slips from your hands while trying!\r\n");
	    w = unequip_char(ch, WIELD_TWOH);
	    obj_from_char(w);
	    obj_to_room(w, ch->in_room);
	    act("$n tries to disarm $N, but $n loses his weapon!", TRUE, ch, 0, victim,
		TO_ROOM);
	}
	GET_POS(ch) = POSITION_SITTING;

	if ((IS_NPC(victim)) && (GET_POS(victim) > POSITION_SLEEPING)
	    && (!victim->specials.fighting)) {
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

	    percent_chance = number(1, 101);		       /* 101% is a complete failure */
	    if (percent_chance > chance) {
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
	    act("You try to disarm $N, but $E doesn't have a weapon.", TRUE, ch, 0, victim,
		TO_CHAR);
	    act("$n makes an impressive fighting move, but does little more.", TRUE, ch, 0, 0,
		TO_ROOM);
	}

	if ((IS_NPC(victim)) && (GET_POS(victim) > POSITION_SLEEPING) &&
	    (!victim->specials.fighting)) {
	    set_fighting(victim, ch);
	}
	WAIT_STATE(ch, PULSE_VIOLENCE * 1);
    }
}

void do_peer(struct char_data *ch, const char *argument, int cmd)
{
    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (GET_MANA(ch) < (15 - GET_LEVEL(ch, BestThiefClass(ch)) / 4)) {
	cprintf(ch, "You don't really see anything...\r\n");
	return;
    }
    if (!*argument) {
	cprintf(ch, "You must peer in a direction...\r\n");
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

int RideCheck(struct char_data *ch)
{
    if (DEBUG > 2)
	log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

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

void FallOffMount(struct char_data *ch, struct char_data *h)
{
    if (DEBUG > 2)
	log_info("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), SAFE_NAME(h));

    act("$n loses control and falls off of $N", FALSE, ch, 0, h, TO_NOTVICT);
    act("$n loses control and falls off of you", FALSE, ch, 0, h, TO_VICT);
    act("You lose control and fall off of $N", FALSE, ch, 0, h, TO_CHAR);
}

void Dismount(struct char_data *ch, struct char_data *h, int pos)
{
    if (DEBUG > 2)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), SAFE_NAME(h),
		 pos);

    MOUNTED(ch) = 0;
    RIDDEN(h) = 0;
    GET_POS(ch) = pos;
    check_falling(ch);
}

int MountEgoCheck(struct char_data *rider, struct char_data *mount)
{
    int                                     diff = 0;
    int                                     chance = 0;

    if (DEBUG > 2)
	log_info("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(rider),
		 SAFE_NAME(mount));

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

void do_mount(struct char_data *ch, const char *argument, int cmd)
{
    char                                    name[112] = "\0\0\0\0\0\0\0";
    int                                     check = FALSE;
    struct char_data                       *horse = NULL;

    if (DEBUG)
	log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		 VNULL(argument), cmd);

    if (IS_AFFECTED(ch, AFF_FLYING)) {
	cprintf(ch, "You can't, you are flying!\r\n");
	return;
    }
    if (cmd == 269) {
	only_argument(argument, name);
	if (!(horse = get_char_room_vis(ch, name))) {
	    cprintf(ch, "Mount what?\r\n");
	    return;
	}
	if (!IsHumanoid(ch)) {
	    cprintf(ch, "You can't ride things!\r\n");
	    return;
	}
	if (IS_SET(horse->specials.act, ACT_MOUNT)) {
	    if (GET_POS(horse) < POSITION_STANDING) {
		cprintf(ch, "Your mount must be standing\r\n");
		return;
	    }
	    if (RIDDEN(horse)) {
		cprintf(ch, "Already ridden\r\n");
		return;
	    }
	    if (MOUNTED(ch)) {
		cprintf(ch, "Already riding\r\n");
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
		act("$N moves out of the way, you fall on your butt", FALSE, ch, 0, horse,
		    TO_CHAR);
		act("as $n tries to mount $N, $N moves out of the way", FALSE, ch, 0, horse,
		    TO_NOTVICT);
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
		act("You try to ride $N, but fall on your butt", FALSE, ch, 0, horse, TO_CHAR);
		act("$n tries to ride $N, but falls on $s butt", FALSE, ch, 0, horse,
		    TO_NOTVICT);
		act("$n tries to ride you, but falls on $s butt", FALSE, ch, 0, horse, TO_VICT);
		GET_POS(ch) = POSITION_SITTING;
		WAIT_STATE(ch, PULSE_VIOLENCE * 2);
	    }
	} else {
	    cprintf(ch, "You can't ride that!\r\n");
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
	cprintf(ch, "Hmmmmmm, don't think you mounted on anything?\r\n");
    }
}

/*
 * Opinions about things 
 */

#include <stdio.h>
#include <stdlib.h>
/* #include <unistd.h> */
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "global.h"
#include "bug.h"
#include "utils.h"
#include "spells.h"
#include "comm.h"
#include "db.h"
#include "multiclass.h"
#define _OPINION_C
#include "opinion.h"

void FreeHates(struct char_data *ch)
{
    struct char_list                       *k = NULL;
    struct char_list                       *n = NULL;

    if (DEBUG > 2)
	log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

    for (k = ch->hates.clist; k; k = n) {
	n = k->next;
	if (n)
	    if (n->valid[0]) {
		if (!strcmp(n->valid, "AddHated")) {
		    DESTROY(n);
		} else {
		    log_error("Attempt to free invalid Hated chain");
		}
	    }
    }
}

void FreeFears(struct char_data *ch)
{
    struct char_list                       *k = NULL;
    struct char_list                       *n = NULL;

    if (DEBUG > 2)
	log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

    for (k = ch->fears.clist; k; k = n) {
	n = k->next;
	if (n)
	    if (n->valid[0]) {
		if (!strcmp(n->valid, "AddFeared")) {
		    DESTROY(n);
		} else {
		    log_error("Attempt to free invalid Feared chain");
		}
	    }
    }
}

int RemHated(struct char_data *ch, struct char_data *pud)
{
    struct char_list                       *oldpud = NULL;
    struct char_list                       *t = NULL;

    if (DEBUG > 2)
	log_info("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), SAFE_NAME(pud));

    if (pud) {
	for (oldpud = ch->hates.clist; oldpud; oldpud = oldpud->next) {
	    if (!oldpud)
		return (FALSE);
	    if (oldpud->op_ch) {
		if (oldpud->op_ch == pud) {
		    t = oldpud;
		    if (ch->hates.clist == t) {
			ch->hates.clist = 0;
			DESTROY(t);
			break;
		    } else {
			for (oldpud = ch->hates.clist; oldpud->next != t;
			     oldpud = oldpud->next);
			oldpud->next = oldpud->next->next;
			DESTROY(t);
			break;
		    }
		}
	    } else {
		if (!strcmp(oldpud->name, GET_NAME(pud))) {
		    t = oldpud;
		    if (ch->hates.clist == t) {
			ch->hates.clist = 0;
			DESTROY(t);
			break;
		    } else {
			for (oldpud = ch->hates.clist; oldpud->next != t;
			     oldpud = oldpud->next);
			oldpud->next = oldpud->next->next;
			DESTROY(t);
			break;
		    }
		}
	    }
	}
    }
    if (!ch->hates.clist)
	REMOVE_BIT(ch->hatefield, HATE_CHAR);
    if (!ch->hatefield)
	REMOVE_BIT(ch->specials.act, ACT_HATEFUL);
    return ((pud) ? TRUE : FALSE);
}

int AddHated(struct char_data *ch, struct char_data *pud)
{
    struct char_list                       *newpud = NULL;

    if (DEBUG > 2)
	log_info("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), SAFE_NAME(pud));

    if (ch == pud)
	return (FALSE);

    if (pud) {
	CREATE(newpud, struct char_list, 1);

	strcpy(newpud->valid, "AddHated");
	newpud->op_ch = pud;
	strcpy(newpud->name, GET_NAME(pud));
	newpud->next = ch->hates.clist;
	ch->hates.clist = newpud;
	if (!IS_SET(ch->specials.act, ACT_HATEFUL))
	    SET_BIT(ch->specials.act, ACT_HATEFUL);
	if (!IS_SET(ch->hatefield, HATE_CHAR))
	    SET_BIT(ch->hatefield, HATE_CHAR);
	if (IS_IMMORTAL(pud))
	    cprintf(pud, "%s HATES you!\r\n", NAME(ch));
    }
    return ((pud) ? TRUE : FALSE);
}

int AddHatred(struct char_data *ch, int parm_type, int parm)
{
    if (DEBUG > 2)
	log_info("called %s with %s, %d, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), parm_type,
		 parm);

    switch (parm_type) {
	case OP_SEX:
	    if (!IS_SET(ch->hatefield, HATE_SEX))
		SET_BIT(ch->hatefield, HATE_SEX);
	    ch->hates.sex = parm;
	    break;
	case OP_RACE:
	    if (!IS_SET(ch->hatefield, HATE_RACE))
		SET_BIT(ch->hatefield, HATE_RACE);
	    ch->hates.race = parm;
	    break;
	case OP_GOOD:
	    if (!IS_SET(ch->hatefield, HATE_GOOD))
		SET_BIT(ch->hatefield, HATE_GOOD);
	    ch->hates.good = parm;
	    break;
	case OP_EVIL:
	    if (!IS_SET(ch->hatefield, HATE_EVIL))
		SET_BIT(ch->hatefield, HATE_EVIL);
	    ch->hates.evil = parm;
	    break;
	case OP_CLASS:
	    if (!IS_SET(ch->hatefield, HATE_CLASS))
		SET_BIT(ch->hatefield, HATE_CLASS);
	    ch->hates.class = parm;
	    break;
	case OP_VNUM:
	    if (!IS_SET(ch->hatefield, HATE_VNUM))
		SET_BIT(ch->hatefield, HATE_VNUM);
	    ch->hates.vnum = parm;
	    break;
	case OP_GOLD:
	    if (!IS_SET(ch->hatefield, HATE_RICH))
		SET_BIT(ch->hatefield, HATE_RICH);
	    ch->hates.gold = parm;
	    break;
    }
    if (!IS_SET(ch->specials.act, ACT_HATEFUL)) {
	SET_BIT(ch->specials.act, ACT_HATEFUL);
    }
    return 1;						       /* assume this means it worked */
}

int RemHatred(struct char_data *ch, unsigned short bitv)
{
    if (DEBUG > 2)
	log_info("called %s with %s, %04hx", __PRETTY_FUNCTION__, SAFE_NAME(ch), bitv);

    REMOVE_BIT(ch->hatefield, bitv);
    if (!ch->hatefield)
	REMOVE_BIT(ch->specials.act, ACT_HATEFUL);
    return 1;						       /* assume this means it worked */
}

int RemFeared(struct char_data *ch, struct char_data *pud)
{
    struct char_list                       *oldpud = NULL;
    struct char_list                       *t = NULL;
    struct char_list                       *tmp = NULL;

    if (DEBUG > 2)
	log_info("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), SAFE_NAME(pud));

    if (!IS_SET(ch->specials.act, ACT_AFRAID))
	return (FALSE);

    if (pud && (ch->fears.clist != 0)) {
	tmp = ch->fears.clist;
	for (oldpud = ch->fears.clist; (oldpud != 0); oldpud = tmp) {
	    if (oldpud == 0)
		return (FALSE);
	    tmp = oldpud->next;
	    if (oldpud->op_ch) {
		if (oldpud->op_ch == pud) {
		    t = oldpud;
		    if (ch->fears.clist == t) {
			ch->fears.clist = 0;
			DESTROY(t);
			break;
		    } else {
			for (oldpud = ch->fears.clist; oldpud->next != t;
			     oldpud = oldpud->next);
			oldpud->next = oldpud->next->next;
			DESTROY(t);
			break;
		    }
		}
	    } else {
		if (!strcmp(oldpud->name, GET_NAME(pud))) {
		    t = oldpud;
		    if (ch->fears.clist == t) {
			ch->fears.clist = 0;
			DESTROY(t);
			break;
		    } else {
			for (oldpud = ch->fears.clist; oldpud->next != t;
			     oldpud = oldpud->next);
			oldpud->next = oldpud->next->next;
			DESTROY(t);
			break;
		    }
		}
	    }
	}
    }
    if (!ch->fears.clist)
	REMOVE_BIT(ch->fearfield, FEAR_CHAR);
    if (!ch->fearfield)
	REMOVE_BIT(ch->specials.act, ACT_AFRAID);
    return ((pud) ? TRUE : FALSE);
}

int AddFeared(struct char_data *ch, struct char_data *pud)
{
    struct char_list                       *newpud = NULL;

    if (DEBUG > 2)
	log_info("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), SAFE_NAME(pud));

    if (ch == pud)
	return (FALSE);

    if (pud) {
	CREATE(newpud, struct char_list, 1);

	strcpy(newpud->valid, "AddFeared");
	newpud->op_ch = pud;
	strcpy(newpud->name, GET_NAME(pud));
	newpud->next = ch->fears.clist;
	ch->fears.clist = newpud;
	if (!IS_SET(ch->specials.act, ACT_AFRAID)) {
	    SET_BIT(ch->specials.act, ACT_AFRAID);
	}
	if (!IS_SET(ch->fearfield, FEAR_CHAR)) {
	    SET_BIT(ch->fearfield, FEAR_CHAR);
	}
	if (IS_IMMORTAL(pud))
	    cprintf(pud, "%s fears you, as well they should.\r\n", NAME(ch));
    }
    return ((pud) ? TRUE : FALSE);
}

int AddFears(struct char_data *ch, int parm_type, int parm)
{
    if (DEBUG > 2)
	log_info("called %s with %s, %d, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), parm_type,
		 parm);

    switch (parm_type) {
	case OP_SEX:
	    if (!IS_SET(ch->fearfield, FEAR_SEX))
		SET_BIT(ch->fearfield, FEAR_SEX);
	    ch->fears.sex = parm;
	    break;
	case OP_RACE:
	    if (!IS_SET(ch->fearfield, FEAR_RACE))
		SET_BIT(ch->fearfield, FEAR_RACE);
	    ch->fears.race = parm;
	    break;
	case OP_GOOD:
	    if (!IS_SET(ch->fearfield, FEAR_GOOD))
		SET_BIT(ch->fearfield, FEAR_GOOD);
	    ch->fears.good = parm;
	    break;
	case OP_EVIL:
	    if (!IS_SET(ch->fearfield, FEAR_EVIL))
		SET_BIT(ch->fearfield, FEAR_EVIL);
	    ch->fears.evil = parm;
	    break;
	case OP_CLASS:
	    if (!IS_SET(ch->fearfield, FEAR_CLASS))
		SET_BIT(ch->fearfield, FEAR_CLASS);
	    ch->fears.class = parm;
	    break;
	case OP_VNUM:
	    if (!IS_SET(ch->fearfield, FEAR_VNUM))
		SET_BIT(ch->fearfield, FEAR_VNUM);
	    ch->fears.vnum = parm;
	    break;
    }
    if (!IS_SET(ch->specials.act, ACT_AFRAID)) {
	SET_BIT(ch->specials.act, ACT_AFRAID);
    }
    return 1;
}

/*
 * these two procedures zero out the character pointer
 * for quiting players, without removing names
 * thus the monsters will still hate them
 */

void ZeroHatred(struct char_data *ch, struct char_data *v)
{
    struct char_list                       *oldpud;

    if (DEBUG)
	log_info("ZeroHatred");
    for (oldpud = ch->hates.clist; oldpud; oldpud = oldpud->next) {
	if (oldpud) {
	    if (oldpud->op_ch) {
		if (oldpud->op_ch == v) {
		    oldpud->op_ch = 0;
		}
	    }
	}
    }
}

void ZeroFeared(struct char_data *ch, struct char_data *v)
{
    struct char_list                       *oldpud = NULL;

    if (DEBUG > 2)
	log_info("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), SAFE_NAME(v));

    for (oldpud = ch->fears.clist; oldpud; oldpud = oldpud->next) {
	if (oldpud) {
	    if (oldpud->op_ch) {
		if (oldpud->op_ch == v) {
		    oldpud->op_ch = 0;
		}
	    }
	}
    }
}

int DoesHate(struct char_data *ch, struct char_data *v)
{
    struct char_list                       *i = NULL;

    if (DEBUG > 2)
	log_info("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), SAFE_NAME(v));

    if (!ch->hatefield)
	return FALSE;
    if (IS_SET(ch->hatefield, HATE_CHAR)) {
	if (ch->hates.clist) {
	    for (i = ch->hates.clist; i; i = i->next) {
		if (i->op_ch) {
		    if ((i->op_ch == v) && (!strcmp(i->name, GET_NAME(v))))
			return (TRUE);
		} else {
		    if (!strcmp(i->name, GET_NAME(v)))
			return (TRUE);
		}
	    }
	}
    }
    if (IS_SET(ch->hatefield, HATE_RACE)) {
	if (ch->hates.race != -1) {
	    if (ch->hates.race == GET_RACE(v))
		return (TRUE);
	}
    }
    if (IS_SET(ch->hatefield, HATE_SEX)) {
	if (ch->hates.sex == GET_SEX(v))
	    return (TRUE);
    }
    if (IS_SET(ch->hatefield, HATE_GOOD)) {
	if (ch->hates.good < GET_ALIGNMENT(v))
	    return (TRUE);
    }
    if (IS_SET(ch->hatefield, HATE_EVIL)) {
	if (ch->hates.evil > GET_ALIGNMENT(v))
	    return (TRUE);
    }
    if (IS_SET(ch->hatefield, HATE_CLASS)) {
	if (HasClass(v, ch->hates.class)) {
	    return (TRUE);
	}
    }
    if (IS_SET(ch->hatefield, HATE_VNUM)) {
	if (ch->hates.vnum == mob_index[v->nr].virtual)
	    return (TRUE);
    }
    if (IS_SET(ch->hatefield, HATE_RICH)) {
	if (GET_GOLD(v) > ch->hates.gold) {
	    act("$n charges $N, screaming 'I must have your GOLD!'", TRUE, ch, 0, v, TO_ROOM);
	    cprintf(v, "%s charges, screaming 'I must have your GOLD!'\r\n", NAME(ch));
	    return (TRUE);
	}
    }
    return (FALSE);
}

int CanHate(struct char_data *ch, struct char_data *v)
{
    if (ch == v)
	return FALSE;
    if (IS_AFFECTED(ch, AFF_PARALYSIS))
	return FALSE;
    if (GET_POS(ch) <= POSITION_SLEEPING)
	return FALSE;
    if (!(CAN_SEE(ch, v)))
	return FALSE;
    if (IS_PC(v) && IS_NOT_SET(v->specials.act, PLR_NOHASSLE))
	return FALSE;
    return DoesHate(ch, v);
}

int DoesFear(struct char_data *ch, struct char_data *v)
{
    struct char_list                       *i = NULL;

    if (DEBUG > 2)
	log_info("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), SAFE_NAME(v));

    if (!IS_SET(ch->specials.act, ACT_AFRAID))
	return (FALSE);

    if (IS_SET(ch->fearfield, FEAR_CHAR)) {
	if (ch->fears.clist) {
	    for (i = ch->fears.clist; i; i = i->next) {
		if (i) {
		    if (i->op_ch) {
			if (i->name[0]) {
			    if ((i->op_ch == v) && (!strcmp(i->name, GET_NAME(v))))
				return (TRUE);
			} else {
/*
 * lets see if this clears the problem 
 */
			    RemFeared(ch, i->op_ch);
			}
		    } else {
			if (i->name[0]) {
			    if (!strcmp(i->name, GET_NAME(v)))
				return (TRUE);
			}
		    }
		}
	    }
	}
    }
    if (IS_SET(ch->fearfield, FEAR_RACE)) {
	if (ch->fears.race != -1) {
	    if (ch->fears.race == GET_RACE(v))
		return (TRUE);
	}
    }
    if (IS_SET(ch->fearfield, FEAR_SEX)) {
	if (ch->fears.sex == GET_SEX(v))
	    return (TRUE);
    }
    if (IS_SET(ch->fearfield, FEAR_GOOD)) {
	if (ch->fears.good < GET_ALIGNMENT(v))
	    return (TRUE);
    }
    if (IS_SET(ch->fearfield, FEAR_EVIL)) {
	if (ch->fears.evil > GET_ALIGNMENT(v))
	    return (TRUE);
    }
    if (IS_SET(ch->fearfield, FEAR_CLASS)) {
	if (HasClass(v, ch->hates.class)) {
	    return (TRUE);
	}
    }
    if (IS_SET(ch->fearfield, FEAR_VNUM)) {
	if (ch->fears.vnum == mob_index[v->nr].virtual)
	    return (TRUE);
    }
    return (FALSE);
}

int CanFear(struct char_data *ch, struct char_data *v)
{
    if (DEBUG > 2)
	log_info("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), SAFE_NAME(v));

    if (ch == v)
	return FALSE;
    if (IS_AFFECTED(ch, AFF_PARALYSIS))
	return FALSE;
    if (GET_POS(ch) <= POSITION_SLEEPING)
	return FALSE;
    if (!(CAN_SEE(ch, v)))
	return FALSE;
    return DoesFear(ch, v);
}

struct char_data                       *FindAHatee(struct char_data *ch)
{
    struct char_data                       *tmp_ch = NULL;

    if (DEBUG > 2)
	log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

    if (ch->in_room < 0)
	return NULL;

    for (tmp_ch = real_roomp(ch->in_room)->people; tmp_ch; tmp_ch = tmp_ch->next_in_room) {
	if (ch == tmp_ch) {				       /* noone should hate themselves! */
	    if (DoesHate(ch, tmp_ch))
		RemHated(ch, tmp_ch);
	    continue;
	}
	if (ch->in_room != tmp_ch->in_room)
	    continue;
	if (CanHate(ch, tmp_ch))
	    return tmp_ch;
    }
    return NULL;
}

struct char_data                       *FindAFearee(struct char_data *ch)
{
    struct char_data                       *tmp_ch = NULL;

    if (DEBUG > 2)
	log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

    if (ch->in_room < 0)
	return NULL;

    for (tmp_ch = real_roomp(ch->in_room)->people; tmp_ch; tmp_ch = tmp_ch->next_in_room) {
	if (ch == tmp_ch) {				       /* noone should fear themselves! */
	    if (DoesFear(ch, tmp_ch))
		RemFeared(ch, tmp_ch);
	    continue;
	}
	if (ch->in_room != tmp_ch->in_room)
	    continue;
	if (CanFear(ch, tmp_ch))
	    return tmp_ch;
    }
    return NULL;
}

/*
 * these two are to make the monsters completely forget about them.
 */
void DeleteHatreds(struct char_data *ch)
{
    struct char_data                       *i = NULL;

    if (DEBUG > 2)
	log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

    for (i = character_list; i; i = i->next) {
	if (DoesHate(i, ch))
	    RemHated(i, ch);
    }
}

void DeleteFears(struct char_data *ch)
{
    struct char_data                       *i = NULL;

    if (DEBUG > 2)
	log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

    for (i = character_list; i; i = i->next) {
	if (DoesFear(i, ch))
	    RemFeared(i, ch);
    }
}

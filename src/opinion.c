/*
 * Opinions about things 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "include/global.h"
#include "include/bug.h"
#include "include/utils.h"
#include "include/spells.h"
#include "include/constants.h"
#include "include/comm.h"
#include "include/db.h"
#include "include/multiclass.h"
#define _OPINION_C
#include "include/opinion.h"

inline void FreeHates(struct char_data *ch) {
  struct char_list *k, *n;

  if (DEBUG)
    dlog("FreeHates");
  for (k = ch->hates.clist; k; k = n) {
    n = k->next;
    free(n);
  }
}

inline void FreeFears(struct char_data *ch) {
  struct char_list *k, *n;

  if (DEBUG)
    dlog("FreeFears");
  for (k = ch->fears.clist; k; k = n) {
    n = k->next;
    free(n);
  }
}

int RemHated(struct char_data *ch, struct char_data *pud) {
  struct char_list *oldpud, *t;

  if (DEBUG)
    dlog("RemHated");
  if (pud) {
    for (oldpud = ch->hates.clist; oldpud; oldpud = oldpud->next) {
      if (!oldpud)
	return (FALSE);
      if (oldpud->op_ch) {
	if (oldpud->op_ch == pud) {
	  t = oldpud;
	  if (ch->hates.clist == t) {
	    ch->hates.clist = 0;
	    free(t);
	    break;
	  } else {
	    for (oldpud = ch->hates.clist; oldpud->next != t; oldpud = oldpud->next);
	    oldpud->next = oldpud->next->next;
	    free(t);
	    break;
	  }
	}
      } else {
	if (!strcmp(oldpud->name, GET_NAME(pud))) {
	  t = oldpud;
	  if (ch->hates.clist == t) {
	    ch->hates.clist = 0;
	    free(t);
	    break;
	  } else {
	    for (oldpud = ch->hates.clist; oldpud->next != t; oldpud = oldpud->next);
	    oldpud->next = oldpud->next->next;
	    free(t);
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

int AddHated(struct char_data *ch, struct char_data *pud) {
  struct char_list *newpud;

  if (DEBUG)
    dlog("AddHated");
  if (ch == pud)
    return (FALSE);

  if (pud) {
    CREATE(newpud, struct char_list, 1);
    newpud->op_ch = pud;
    strcpy(newpud->name, GET_NAME(pud));
    newpud->next = ch->hates.clist;
    ch->hates.clist = newpud;
    if (!IS_SET(ch->specials.act, ACT_HATEFUL))
      SET_BIT(ch->specials.act, ACT_HATEFUL);
    if (!IS_SET(ch->hatefield, HATE_CHAR))
      SET_BIT(ch->hatefield, HATE_CHAR);
    if (IS_IMMORTAL(pud))
      cprintf(pud, "%s HATES you!\n\r", NAME(ch));
  }
  return ((pud) ? TRUE : FALSE);
}

int AddHatred(struct char_data *ch, int parm_type, int parm) {
  if (DEBUG)
    dlog("AddHatred");
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
  return 1;			       /* assume this means it worked */
}

inline int RemHatred(struct char_data *ch, unsigned short bitv) {
  if (DEBUG)
    dlog("RemHatred");
  REMOVE_BIT(ch->hatefield, bitv);
  if (!ch->hatefield)
    REMOVE_BIT(ch->specials.act, ACT_HATEFUL);
  return 1;			       /* assume this means it worked */
}

int RemFeared(struct char_data *ch, struct char_data *pud) {
  struct char_list *oldpud, *t, *tmp;

  if (DEBUG)
    dlog("RemFeared");
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
	    free(t);
	    break;
	  } else {
	    for (oldpud = ch->fears.clist; oldpud->next != t;
		 oldpud = oldpud->next);
	    oldpud->next = oldpud->next->next;
	    free(t);
	    break;
	  }
	}
      } else {
	if (!strcmp(oldpud->name, GET_NAME(pud))) {
	  t = oldpud;
	  if (ch->fears.clist == t) {
	    ch->fears.clist = 0;
	    free(t);
	    break;
	  } else {
	    for (oldpud = ch->fears.clist; oldpud->next != t;
		 oldpud = oldpud->next);
	    oldpud->next = oldpud->next->next;
	    free(t);
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

int AddFeared(struct char_data *ch, struct char_data *pud) {
  struct char_list *newpud;

  if (DEBUG)
    dlog("AddFeared");
  if (ch == pud)
    return (FALSE);

  if (pud) {
    CREATE(newpud, struct char_list, 1);
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
      cprintf(pud, "%s fears you, as well they should.\n\r", NAME(ch));
  }
  return ((pud) ? TRUE : FALSE);
}

int AddFears(struct char_data *ch, int parm_type, int parm) {
  if (DEBUG)
    dlog("AddFears");
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

inline void ZeroHatred(struct char_data *ch, struct char_data *v) {
  struct char_list *oldpud;

  if (DEBUG)
    dlog("ZeroHatred");
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

inline void ZeroFeared(struct char_data *ch, struct char_data *v) {
  struct char_list *oldpud;

  if (DEBUG)
    dlog("ZeroFeared");
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

int DoesHate(struct char_data *ch, struct char_data *v) {
  struct char_list *i;

  if (DEBUG)
    dlog("DoesHate");

  if (!ch->hatefield)
    return FALSE;
  if (IS_SET(ch->hatefield, HATE_CHAR)) {
    if (ch->hates.clist) {
      for (i = ch->hates.clist; i; i = i->next) {
	if (i->op_ch) {
	  if ((i->op_ch == v) &&
	      (!strcmp(i->name, GET_NAME(v))))
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
      act("$n charges $N, screaming 'I must have your GOLD!'",
          TRUE, ch, 0, v, TO_ROOM);
      cprintf(v, "%s charges, screaming 'I must have your GOLD!'\n\r",
                NAME(ch));
      return (TRUE);
    }
  }
  return (FALSE);
}

inline int CanHate(struct char_data *ch, struct char_data *v) {
  if (ch == v)
    return FALSE;
  if (IS_AFFECTED(ch, AFF_PARALYSIS))
    return FALSE;
  if (GET_POS(ch) <= POSITION_SLEEPING)
    return FALSE;
  if(!(CAN_SEE(ch, v)))
    return FALSE;
  if(IS_PC(v) && IS_NOT_SET(v->specials.act, PLR_NOHASSLE))
    return FALSE;
  return DoesHate(ch, v);
}

int DoesFear(struct char_data *ch, struct char_data *v) {
  struct char_list *i;

  if (!IS_SET(ch->specials.act, ACT_AFRAID))
    return (FALSE);

  if (IS_SET(ch->fearfield, FEAR_CHAR)) {
    if (ch->fears.clist) {
      for (i = ch->fears.clist; i; i = i->next) {
	if (i) {
	  if (i->op_ch) {
	    if (i->name) {
	      if ((i->op_ch == v) &&
		  (!strcmp(i->name, GET_NAME(v))))
		return (TRUE);
	    } else {
/*
 * lets see if this clears the problem 
 */
	      RemFeared(ch, i->op_ch);
	    }
	  } else {
	    if (i->name) {
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

inline int CanFear(struct char_data *ch, struct char_data *v) {
  if (ch == v)
    return FALSE;
  if (IS_AFFECTED(ch, AFF_PARALYSIS))
    return FALSE;
  if (GET_POS(ch) <= POSITION_SLEEPING)
    return FALSE;
  if(!(CAN_SEE(ch, v)))
    return FALSE;
  return DoesFear(ch, v);
}

struct char_data *FindAHatee(struct char_data *ch) {
  struct char_data *tmp_ch;

  if (DEBUG)
    dlog("FindAHatee");
  if (ch->in_room < 0)
    return NULL;

  for (tmp_ch = real_roomp(ch->in_room)->people; tmp_ch;
       tmp_ch = tmp_ch->next_in_room) {
    if (ch == tmp_ch) { /* noone should hate themselves! */
      if(DoesHate(ch, tmp_ch))
        RemHated(ch, tmp_ch);
      continue;
    }
    if(ch->in_room != tmp_ch->in_room)
      continue;
    if(CanHate(ch, tmp_ch))
      return tmp_ch;
  }
  return NULL;
}

struct char_data *FindAFearee(struct char_data *ch) {
  struct char_data *tmp_ch;

  if (DEBUG)
    dlog("FindAFearee");
  if (ch->in_room < 0)
    return NULL;

  for (tmp_ch = real_roomp(ch->in_room)->people; tmp_ch;
       tmp_ch = tmp_ch->next_in_room) {
    if (ch == tmp_ch) { /* noone should fear themselves! */
      if(DoesFear(ch, tmp_ch))
        RemFeared(ch, tmp_ch);
      continue;
    }
    if(ch->in_room != tmp_ch->in_room)
      continue;
    if(CanFear(ch, tmp_ch))
      return tmp_ch;
  }
  return NULL;
}

/*
 * these two are to make the monsters completely forget about them.
 */
inline void DeleteHatreds(struct char_data *ch) {
  struct char_data *i;

  if (DEBUG)
    dlog("DeleteHatreds");
  for (i = character_list; i; i = i->next) {
    if (DoesHate(i, ch))
      RemHated(i, ch);
  }
}

inline void DeleteFears(struct char_data *ch) {
  struct char_data *i;

  if (DEBUG)
    dlog("DeleteFears");
  for (i = character_list; i; i = i->next) {
    if (DoesFear(i, ch))
      RemFeared(i, ch);
  }
}

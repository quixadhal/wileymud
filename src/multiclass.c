/*
 *  Levels:  int levels[6]
 *
 *  0 = Mage, 1 = cleric, 3 = thief, 2 = fighter 4=ranger, 5=druid
 */

#include <stdio.h>
#include <stdlib.h>
/* #include <unistd.h> */
#include <limits.h>
#include <sys/types.h>
#include <string.h>

#include "global.h"
#include "bug.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "interpreter.h"
#include "handler.h"
#include "spells.h"
#include "mudlimits.h"
#include "opinion.h"
#include "constants.h"
#define _MULTICLASS_C
#include "multiclass.h"

int GetClassLevel(struct char_data *ch, int class)
{
  if (DEBUG > 2)
    dlog("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), class);

  if (IS_SET(ch->player.class, class)) {
    return (GET_LEVEL(ch, CountBits(class) - 1));
  }
  return (0);
}

int CountBits(int class)
{
  if (DEBUG > 2)
    dlog("called %s with %d", __PRETTY_FUNCTION__, class);

  if (class == 1)
    return (1);
  if (class == 2)
    return (2);
  if (class == 4)
    return (3);
  if (class == 8)
    return (4);
  if (class == 16)
    return (5);						       /* ranger */
  if (class == 32)
    return (6);						       /* druid */
  return 0;
}

int OnlyClass(struct char_data *ch, int class)
{
  int                                     i = 0;

  if (DEBUG > 2)
    dlog("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), class);

  for (i = 1; i <= 32; i *= 2) {
    if (GetClassLevel(ch, i) != 0)
      if (i != class)
	return (FALSE);
  }
  return (TRUE);

}

inline int HasClass(struct char_data *ch, int class)
{
  if (DEBUG > 3)
    dlog("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), class);

  if (IS_SET(ch->player.class, class))
    return TRUE;
  return FALSE;
}

inline int HowManyClasses(struct char_data *ch)
{
  int                                     i = 0;
  int                                     tot = 0;

  if (DEBUG > 3)
    dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  for (i = 0; i < ABS_MAX_CLASS; i++)
    if (HasClass(ch, 1 << i))
      tot++;
  return tot ? tot : 1;
}

int BestFightingClass(struct char_data *ch)
{
  if (DEBUG > 2)
    dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  if (GET_LEVEL(ch, WARRIOR_LEVEL_IND))
    return (WARRIOR_LEVEL_IND);
  if (GET_LEVEL(ch, RANGER_LEVEL_IND))
    return (RANGER_LEVEL_IND);
  if (GET_LEVEL(ch, DRUID_LEVEL_IND))
    return (DRUID_LEVEL_IND);
  if (GET_LEVEL(ch, CLERIC_LEVEL_IND))
    return (CLERIC_LEVEL_IND);
  if (GET_LEVEL(ch, THIEF_LEVEL_IND))
    return (THIEF_LEVEL_IND);
  if (GET_LEVEL(ch, MAGE_LEVEL_IND))
    return (MAGE_LEVEL_IND);

  bug("Massive error.. character %s has no recognized class.", GET_NAME(ch));
  return (1);
}

int BestThiefClass(struct char_data *ch)
{
  if (DEBUG > 2)
    dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  if (GET_LEVEL(ch, THIEF_LEVEL_IND))
    return (THIEF_LEVEL_IND);
  if (GET_LEVEL(ch, RANGER_LEVEL_IND))
    return (RANGER_LEVEL_IND);
  if (GET_LEVEL(ch, MAGE_LEVEL_IND))
    return (MAGE_LEVEL_IND);
  if (GET_LEVEL(ch, WARRIOR_LEVEL_IND))
    return (WARRIOR_LEVEL_IND);
  if (GET_LEVEL(ch, DRUID_LEVEL_IND))
    return (DRUID_LEVEL_IND);
  if (GET_LEVEL(ch, CLERIC_LEVEL_IND))
    return (CLERIC_LEVEL_IND);

  bug("Massive error.. character %s has no recognized class.", GET_NAME(ch));
  return (1);
}

int BestMagicClass(struct char_data *ch)
{
  if (DEBUG > 2)
    dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  if (GET_LEVEL(ch, MAGE_LEVEL_IND))
    return (MAGE_LEVEL_IND);
  if (GET_LEVEL(ch, CLERIC_LEVEL_IND))
    return (CLERIC_LEVEL_IND);
  if (GET_LEVEL(ch, DRUID_LEVEL_IND))
    return (DRUID_LEVEL_IND);
  if (GET_LEVEL(ch, RANGER_LEVEL_IND))
    return (RANGER_LEVEL_IND);
  if (GET_LEVEL(ch, THIEF_LEVEL_IND))
    return (THIEF_LEVEL_IND);
  if (GET_LEVEL(ch, WARRIOR_LEVEL_IND))
    return (WARRIOR_LEVEL_IND);

  bug("Massive error.. character %s has no recognized class.", GET_NAME(ch));
  return (1);
}

int GetALevel(struct char_data *ch, int which)
{
  int                                     ind[6] = { 0,0,0,0,0,0 };
  int                                     j = 0;
  int                                     k = 0;
  int                                     i = 0;

  if (DEBUG > 2)
    dlog("called %s with %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), which);

  for (i = MAGE_LEVEL_IND; i <= DRUID_LEVEL_IND; i++) {
    ind[i] = GET_LEVEL(ch, i);
  }

/*
 *  chintzy sort. (just to prove that I did learn something in college)
 */

  for (i = 0; i < ABS_MAX_CLASS - 1; i++) {
    for (j = i + 1; j < ABS_MAX_CLASS; j++) {
      if (ind[j] > ind[i]) {
	k = ind[i];
	ind[i] = ind[j];
	ind[j] = k;
      }
    }
  }

  if (which > -1 && which < ABS_MAX_CLASS) {
    return (ind[which]);
  }
  return 0;
}

int GetSecMaxLev(struct char_data *ch)
{
  if (DEBUG > 2)
    dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  return (GetALevel(ch, 1));
}

int GetThirdMaxLev(struct char_data *ch)
{
  if (DEBUG > 2)
    dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  return (GetALevel(ch, 2));
}

int GetMaxLevel(struct char_data *ch)
{
  int                                     max = 0;
  int                                     i = 0;

  /*
   * Cannot call things in bug.c from things bug.c calls!
   *
   * if (DEBUG > 2)
   *   dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));
   */

  for (i = MAGE_LEVEL_IND; i <= DRUID_LEVEL_IND; i++) {
    if (GET_LEVEL(ch, i) > max)
      max = GET_LEVEL(ch, i);
  }

  return (max);
}

int GetMinLevel(struct char_data *ch)
{
  int                                     min = INT_MAX;
  int                                     i = 0;

  if (DEBUG > 2)
    dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  for (i = MAGE_LEVEL_IND; i <= DRUID_LEVEL_IND; i++) {
    if (GET_LEVEL(ch, i) > 0)
      if (GET_LEVEL(ch, i) < min)
	min = GET_LEVEL(ch, i);
  }

  return (min);
}

int GetTotLevel(struct char_data *ch)
{
  if (DEBUG > 2)
    dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  return (GET_LEVEL(ch, 0) + GET_LEVEL(ch, 1) + GET_LEVEL(ch, 2)
	  + GET_LEVEL(ch, 3) + GET_LEVEL(ch, 4) + GET_LEVEL(ch, 5));

}

void StartLevels(struct char_data *ch)
{
  if (DEBUG > 2)
    dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  if (IS_SET(ch->player.class, CLASS_MAGIC_USER)) {
    advance_level(ch, MAGE_LEVEL_IND);
  }
  if (IS_SET(ch->player.class, CLASS_CLERIC)) {
    advance_level(ch, CLERIC_LEVEL_IND);
  }
  if (IS_SET(ch->player.class, CLASS_WARRIOR)) {
    advance_level(ch, WARRIOR_LEVEL_IND);
  }
  if (IS_SET(ch->player.class, CLASS_RANGER)) {
    advance_level(ch, RANGER_LEVEL_IND);
  }
  if (IS_SET(ch->player.class, CLASS_DRUID)) {
    advance_level(ch, DRUID_LEVEL_IND);
  }
  if (IS_SET(ch->player.class, CLASS_THIEF)) {
    advance_level(ch, THIEF_LEVEL_IND);
  }
}

int BestClass(struct char_data *ch)
{
  int                                     max = 0;
  int                                     class = 0;
  int                                     i = 0;

  if (DEBUG > 2)
    dlog("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

  for (i = MAGE_LEVEL_IND; i <= DRUID_LEVEL_IND; i++)
    if (max < GET_LEVEL(ch, i)) {
      max = GET_LEVEL(ch, i);
      class = i;
    }
  if (max == 0) {					       /* eek */
    bug("Massive error.. character %s has no recognized class.", GET_NAME(ch));
    abort();
  } else {
    return (class);
  }
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#include "include/global.h"
#include "include/bug.h"
#include "include/comm.h"
#include "include/interpreter.h"
#include "include/handler.h"
#include "include/db.h"
#include "include/spells.h"
#include "include/trap.h"
#include "include/utils.h"
#define _SOUND_C
#include "include/sound.h"

int RecGetObjRoom(struct obj_data *obj)
{
  if (obj->in_room != NOWHERE) {
    return (obj->in_room);
  }
  if (obj->carried_by) {
    return (obj->carried_by->in_room);
  }
  if (obj->equipped_by) {
    return (obj->equipped_by->in_room);
  }
  if (obj->in_obj) {
    return (RecGetObjRoom(obj->in_obj));
  }
  return -1;			       /* This is an invalid room index... hope real_roomp() works */
}

void MakeNoise(int room, char *local_snd, char *distant_snd)
{
  int door;
  struct char_data *ch;
  struct room_data *rp, *orp;

  rp = real_roomp(room);

  if (rp) {
    for (ch = rp->people; ch; ch = ch->next_in_room) {
      if (!IS_NPC(ch) &&
	  !IS_SET(ch->specials.act, PLR_DEAF) &&
	  (GET_POS(ch) != POSITION_SLEEPING))
	cprintf(ch, local_snd);
    }

    for (door = 0; door < MAX_NUM_EXITS; door++) {
      if (rp->dir_option[door] && (orp = real_roomp(rp->dir_option[door]->to_room))) {
	for (ch = orp->people; ch; ch = ch->next_in_room) {
	  if (!IS_NPC(ch) &&
	      (!IS_SET(ch->specials.act, PLR_DEAF) &&
	       (GET_POS(ch) != POSITION_SLEEPING))) {
	    cprintf(ch, distant_snd);
	  }
	}
      }
    }
  }
}

void MakeSound(int pulse)
{
  int room = -1;		       /* default is bad value */
  char buffer[128];
  struct obj_data *obj;
  struct char_data *ch;

  for (obj = object_list; obj; obj = obj->next) {
    if (ITEM_TYPE(obj) == ITEM_AUDIO) {
      if (((obj->obj_flags.value[0]) &&
	   (pulse % obj->obj_flags.value[0]) == 0) ||
	  (!number(0, 5))) {
	if (obj->carried_by) {
	  room = obj->carried_by->in_room;
	} else if (obj->equipped_by) {
	  room = obj->equipped_by->in_room;
	} else if (obj->in_room != NOWHERE) {
	  room = obj->in_room;
	} else {
	  room = RecGetObjRoom(obj);
	}			       /* broadcast to room */
	if (obj->action_description) {
	  MakeNoise(room, obj->action_description, obj->action_description);
	}
      }
    }
  }

/* rooms */

  for (ch = character_list; ch; ch = ch->next) {
    if ((IS_PC(ch)) &&
	(number(0, 8) == 0) &&
	(IS_NOT_SET(ch->specials.act, PLR_DEAF)) &&
	(GET_POS(ch) > POSITION_SLEEPING)
      ) {
      if (real_roomp(ch->in_room)->sound != 0) {
	if (number(0, 1) == 1)
	  cprintf(ch, real_roomp(ch->in_room)->sound);
	else if (real_roomp(ch->in_room)->distant_sound != 0)
	  cprintf(ch, real_roomp(ch->in_room)->distant_sound);
      }
    }
  }

/* mobiles */

  for (ch = character_list; ch; ch = ch->next) {
    if (IS_NPC(ch) && (ch->player.sounds) && (number(0, 7) == 0)) {
      if (ch->specials.default_pos > POSITION_SLEEPING) {
	if (GET_POS(ch) > POSITION_SLEEPING) {
	  MakeNoise(ch->in_room, ch->player.sounds, ch->player.distant_snds);
	} else if (GET_POS(ch) == POSITION_SLEEPING) {
	  sprintf(buffer, "%s snores loudly.\n\r", ch->player.short_descr);
	  MakeNoise(ch->in_room, buffer, "You hear a loud snore nearby.\n\r");
	}
      } else if (GET_POS(ch) == ch->specials.default_pos) {
	MakeNoise(ch->in_room, ch->player.sounds, ch->player.distant_snds);
      }
    }
  }
}

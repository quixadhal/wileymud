/*
 * file: spell_parser.c , Basic routines and parsing      Part of DIKUMUD
 * Usage : Interpreter of spells
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <assert.h>

#include "global.h"
#include "bug.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "interpreter.h"
#include "handler.h"
#include "spells.h"
#include "constants.h"
#include "act_off.h"
#define _SPELL_PARSER_C
#include "spell_parser.h"

struct spell_info_type spell_info[MAX_SPL_LIST];

char *spells[] =
{
  "armor",			       /* 1 */
  "teleport",
  "bless",
  "blindness",
  "burning hands",
  "call lightning",
  "charm person",
  "chill touch",
  "clone",
  "colour spray",		       /* colour spray */
  "control weather",		       /* 11 */
  "create food",
  "create water",
  "cure blind",
  "cure critic",
  "cure light",
  "curse",
  "detect evil",
  "detect invisibility",
  "detect magic",
  "detect poison",		       /* 21 */
  "dispel evil",
  "earthquake",
  "enchant weapon",
  "energy drain",
  "fireball",
  "harm",
  "heal",
  "invisibility",
  "lightning bolt",
  "locate object",		       /* 31 */
  "magic missile",
  "poison",
  "protection from evil",
  "remove curse",
  "sanctuary",
  "shocking grasp",
  "sleep",
  "strength",
  "summon",
  "ventriloquate",		       /* 41 */
  "word of recall",
  "remove poison",
  "sense life",			       /* 44 */

   /* RESERVED SKILLS */
  "sneak",		       /* 45 */
  "hide",
  "steal",
  "backstab",
  "pick",
  "kick",			       /* 50 */
  "bash",
  "rescue",
   /* NON-CASTABLE SPELLS (Scrolls/potions/wands/staffs) */

  "identify",			       /* 53 */
  "infravision",
  "cause light",
  "cause critical",
  "flamestrike",
  "dispel good",
  "weakness",
  "dispel magic",
  "knock",
  "know alignment",
  "animate dead",
  "paralyze",
  "remove paralysis",
  "fear",
  "acid blast",			       /* 67 */
  "water breath",
  "fly",
  "cone of cold",		       /* 70 */
  "meteor swarm",
  "ice storm",
  "shield",
  "monsum one",
  "monsum two",
  "monsum three",
  "monsum four",
  "monsum five",
  "monsum six",
  "monsum seven",		       /* 80 */
  "fireshield",
  "charm monster",
  "cure serious",
  "cause serious",
  "refresh",
  "second wind",
  "turn",
  "succor",
  "create light",
  "continual light",		       /* 90 */
  "calm",
  "stone skin",
  "conjure elemental",
  "true sight",
  "minor creation",
  "faerie fire",
  "faerie fog",
  "cacaodemon",
  "polymorph self",
  "mana",			       /* 100 */
  "astral walk",
  "group fly",			       /* 102 - spell_fly_group */
  "aid",			       /* 103 - spell_aid */
  "shelter",			       /* 104 - spell_shelter */
  "****",
  "****",
  "****",
  "****",
  "****",
  "****",			       /* 110 */
  "****",
  "****",
  "****",
  "****",
  "****",
  "****",
  "****",
  "****",
  "****",
  "****",			       /* 120 */
  "****",
  "****",
  "****",
  "****",
  "****",
  "****",
  "****",
  "****",
  "****",
  "****",			       /* 130 */
  "****",
  "****",
  "****",
  "****",
  "****",
  "****",
  "****",
  "****",
  "****",
  "****",			       /* 140 */
  "****",
  "****",
  "****",
  "****",
  "****",
  "****",
  "****",
  "****",
  "doorbash",
  "read magic",			       /* 150 */
  "scribe",
  "brew",
  "punch",
  "two hand",
  "two weapon",
  "bandage",
  "search",
  "swimming",
  "endurance",
  "bare hand",			       /* 160 */
  "blind fighting",
  "parry",
  "apraise",
  "spec smite",
  "spec stab",
  "spec whip",
  "spec slash",
  "spec smash",
  "spec cleave",
  "spec crush",			       /* 170 */
  "spec bludge",
  "spec pierce",
  "peer",
  "detect noise",
  "dodge",
  "barter",
  "knock out",
  "spellcraft",
  "meditation",
  "hunt",			       /* 180 */
  "find traps",
  "disarm traps",
  "disarm",
  "bash with shield",
  "ride",
  "****",
  "****",
  "****",
  "****",
  "****",			       /* 190 */
  "****",
  "****",
  "****",
  "****",
  "****",
  "****",
  "****",
  "****",
  "green slime",
  "geyser",			       /* 200 */
  "fire breath",
  "gas breath",
  "frost breath",
  "acid breath",
  "lightning breath",
  "TYPE_HIT",
  "TYPE_BLUDGEON",
  "TYPE_PIERCE",
  "TYPE_SLASH",
  "TYPE_WHIP",			       /* 210 */
  "TYPE_CLAW",
  "TYPE_BITE",
  "TYPE_STING",
  "TYPE_CRUSH",
  "TYPE_CLEAVE",
  "TYPE_STAB",
  "TYPE_SMASH",
  "TYPE_SMITE",
  "TYPE_SUFFERING",
  "TYPE_HUNGER",
  "\n"
};

const BYTE saving_throws[6][5][ABS_MAX_LVL] =
{
  {
    {16, 14, 14, 14, 14, 14, 13, 13, 13, 13, 13, 11, 11, 11, 11, 11, 10, 10, 10, 10, 10, 8, 6, 4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0},
    {13, 11, 11, 11, 11, 11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 5, 5, 5, 5, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
    {15, 13, 13, 13, 13, 13, 11, 11, 11, 11, 11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0},
    {17, 15, 15, 15, 15, 15, 13, 13, 13, 13, 13, 11, 11, 11, 11, 11, 9, 9, 9, 9, 9, 7, 5, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0},
    {14, 12, 12, 12, 12, 12, 10, 10, 10, 10, 10, 8, 8, 8, 8, 8, 6, 6, 6, 6, 6, 4, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0}
  },
  {
    {11, 10, 10, 10, 9, 9, 9, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
    {16, 14, 14, 14, 13, 13, 13, 11, 11, 11, 10, 10, 10, 9, 9, 9, 8, 8, 8, 6, 6, 5, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 0, 0, 0, 0, 0, 0},
    {15, 13, 13, 13, 12, 12, 12, 10, 10, 10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 5, 5, 4, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0},
    {18, 16, 16, 16, 15, 15, 15, 13, 13, 13, 12, 12, 12, 11, 11, 11, 10, 10, 10, 8, 8, 7, 6, 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 0, 0, 0, 0, 0, 0, 0},
    {17, 15, 15, 15, 14, 14, 14, 12, 12, 12, 11, 11, 11, 10, 10, 10, 9, 9, 9, 7, 7, 6, 5, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 0, 0, 0, 0, 0, 0, 0}
  },
  {
    {15, 13, 13, 13, 13, 12, 12, 12, 12, 11, 11, 11, 11, 10, 10, 10, 10, 9, 9, 9, 9, 8, 7, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0},
    {16, 14, 14, 14, 14, 12, 12, 12, 12, 10, 10, 10, 10, 8, 8, 8, 8, 6, 6, 6, 6, 4, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0},
    {14, 12, 12, 12, 12, 11, 11, 11, 11, 10, 10, 10, 10, 9, 9, 9, 9, 8, 8, 8, 8, 7, 5, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0},
    {18, 16, 16, 16, 16, 15, 15, 15, 15, 14, 14, 14, 14, 13, 13, 13, 13, 12, 12, 12, 12, 11, 9, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0},
    {17, 15, 15, 15, 15, 13, 13, 13, 13, 11, 11, 11, 11, 9, 9, 9, 9, 7, 7, 7, 7, 5, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0}
  },
  {
    {16, 14, 14, 13, 13, 11, 11, 10, 10, 8, 8, 7, 7, 5, 5, 4, 4, 3, 3, 3, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
    {18, 16, 16, 15, 15, 13, 13, 12, 12, 10, 10, 9, 9, 7, 7, 6, 6, 5, 5, 5, 5, 4, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0},
    {17, 15, 15, 14, 14, 12, 12, 11, 11, 9, 9, 8, 8, 6, 6, 5, 5, 4, 4, 4, 4, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
    {20, 17, 17, 16, 16, 13, 13, 12, 12, 9, 9, 8, 8, 5, 5, 4, 4, 4, 4, 4, 4, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
    {19, 17, 17, 16, 16, 14, 14, 13, 13, 11, 11, 10, 10, 8, 8, 7, 7, 6, 6, 6, 6, 4, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0}
  },
  {
    {16, 14, 14, 14, 14, 14, 13, 13, 13, 13, 13, 11, 11, 11, 11, 11, 10, 10, 10, 10, 10, 8, 6, 4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0},
    {13, 11, 11, 11, 11, 11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 5, 5, 5, 5, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
    {15, 13, 13, 13, 13, 13, 11, 11, 11, 11, 11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0},
    {17, 15, 15, 15, 15, 15, 13, 13, 13, 13, 13, 11, 11, 11, 11, 11, 9, 9, 9, 9, 9, 7, 5, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0},
    {14, 12, 12, 12, 12, 12, 10, 10, 10, 10, 10, 8, 8, 8, 8, 8, 6, 6, 6, 6, 6, 4, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0}
  },
  {
    {11, 10, 10, 10, 9, 9, 9, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
    {16, 14, 14, 14, 13, 13, 13, 11, 11, 11, 10, 10, 10, 9, 9, 9, 8, 8, 8, 6, 6, 5, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 0, 0, 0, 0, 0, 0},
    {15, 13, 13, 13, 12, 12, 12, 10, 10, 10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 5, 5, 4, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0},
    {18, 16, 16, 16, 15, 15, 15, 13, 13, 13, 12, 12, 12, 11, 11, 11, 10, 10, 10, 8, 8, 7, 6, 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 0, 0, 0, 0, 0, 0, 0},
    {17, 15, 15, 15, 14, 14, 14, 12, 12, 12, 11, 11, 11, 10, 10, 10, 9, 9, 9, 7, 7, 6, 5, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 0, 0, 0, 0, 0, 0, 0}
  }
};

int SPELL_LEVEL(struct char_data *ch, int sn)
{
#ifdef OLD_WILEY
  if ((HasClass(ch, CLASS_MAGIC_USER)) && (HasClass(ch, CLASS_CLERIC))) {
    return (MIN(spell_info[sn].min_level_magic, spell_info[sn].min_level_cleric));
  } else {
    if (HasClass(ch, CLASS_MAGIC_USER)) {
      return (spell_info[sn].min_level_magic);
    } else {
      return (spell_info[sn].min_level_cleric);
    }
  }
#else
  register int i, lowest;

  lowest= ABS_MAX_LVL;
  for(i= 0; i< ABS_MAX_CLASS; i++)
    lowest= MIN(lowest, spell_info[sn].min_level[i]);
  return lowest;
#endif
}

void affect_update(void)
{
  static struct affected_type *af, *next_af_dude;
  static struct char_data *i;

  for (i = character_list; i; i = i->next)
    for (af = i->affected; af; af = next_af_dude) {
      next_af_dude = af->next;
      if (af->duration >= 1) {
	af->duration--;
	if (af->duration == 0) {
	  if (*spell_wear_off_soon_msg[af->type]) {
	    send_to_char(spell_wear_off_soon_msg[af->type], i);
	    send_to_char("\n\r", i);
	  }
	}
      } else {
	if ((af->type > 0) && (af->type <= MAX_EXIST_SPELL)) {
	  if (!af->next || (af->next->type != af->type) || (af->next->duration > 0)) {
	    if (*spell_wear_off_msg[af->type]) {
	      send_to_char(spell_wear_off_msg[af->type], i);
	      send_to_char("\n\r", i);

	      /* check to see if the exit down is connected, if so make the person */
	      /* fall down into that room and take 1d6 damage */

	      affect_remove(i, af);
	      return;
	    }
	  }
	} else if (af->type >= FIRST_BREATH_WEAPON && af->type <= LAST_BREATH_WEAPON) {
	  bweapons[af->type - FIRST_BREATH_WEAPON] (-af->modifier / 2, i, "", SPELL_TYPE_SPELL, i, 0);
	  if (!i->affected)
	    /* oops, you're dead :) */
	    break;
	}
	affect_remove(i, af);
      }
    }
}

void clone_char(struct char_data *ch)
{
  struct char_data *clone;
  struct affected_type *af;
  int i;

  CREATE(clone, struct char_data, 1);

  clear_char(clone);		       /* Clear EVERYTHING! (ASSUMES CORRECT) */

  clone->player = ch->player;
  clone->abilities = ch->abilities;

  for (i = 0; i < 5; i++)
    clone->specials.apply_saving_throw[i] = ch->specials.apply_saving_throw[i];

  for (af = ch->affected; af; af = af->next)
    affect_to_char(clone, af);

  for (i = 0; i < 3; i++)
    GET_COND(clone, i) = GET_COND(ch, i);

  clone->points = ch->points;

  for (i = 0; i < MAX_SKILLS; i++)
    clone->skills[i] = ch->skills[i];

  clone->specials = ch->specials;
  clone->specials.fighting = 0;

  GET_NAME(clone) = strdup(GET_NAME(ch));

  clone->player.short_descr = strdup(ch->player.short_descr);

  clone->player.long_descr = strdup(ch->player.long_descr);

  clone->player.description = 0;
  /* REMEMBER EXTRA DESCRIPTIONS */

  GET_TITLE(clone) = strdup(GET_TITLE(ch));

  clone->nr = ch->nr;

  if (IS_NPC(clone))
    mob_index[clone->nr].number++;
  else {			       /* Make PC's into NPC's */
    clone->nr = -1;
    SET_BIT(clone->specials.act, ACT_ISNPC);
  }

  clone->desc = 0;
  clone->followers = 0;
  clone->master = 0;

  clone->next = character_list;
  character_list = clone;

  char_to_room(clone, ch->in_room);
}

void clone_obj(struct obj_data *obj)
{
  struct obj_data *clone;

  CREATE(clone, struct obj_data, 1);

  *clone = *obj;

  clone->name = strdup(obj->name);
  clone->description = strdup(obj->description);
  clone->short_description = strdup(obj->short_description);
  clone->action_description = strdup(obj->action_description);
  clone->ex_description = 0;

  /* REMEMBER EXTRA DESCRIPTIONS */
  clone->carried_by = 0;
  clone->equipped_by = 0;
  clone->in_obj = 0;
  clone->contains = 0;
  clone->next_content = 0;
  clone->next = 0;

  /* VIRKER IKKE ENDNU */
}

/* Check if making CH follow VICTIM will create an illegal */
/* Follow "Loop/circle"                                    */
BYTE circle_follow(struct char_data *ch, struct char_data *victim)
{
  struct char_data *k;

  for (k = victim; k; k = k->master) {
    if (k == ch)
      return (TRUE);
  }

  return (FALSE);
}

/* Called when stop following persons, or stopping charm */
/* This will NOT do if a character quits/dies!!          */
void stop_follower(struct char_data *ch)
{
  struct follow_type *j, *k;

  if (!ch->master)
    return;

  if (IS_AFFECTED(ch, AFF_CHARM)) {
    act("You realize that $N is a jerk!", FALSE, ch, 0, ch->master, TO_CHAR);
    act("$n realizes that $N is a jerk!", FALSE, ch, 0, ch->master, TO_NOTVICT);
    act("$n hates your guts!", FALSE, ch, 0, ch->master, TO_VICT);
    if (affected_by_spell(ch, SPELL_CHARM_PERSON))
      affect_from_char(ch, SPELL_CHARM_PERSON);
  } else {
    act("You stop following $N.", FALSE, ch, 0, ch->master, TO_CHAR);
    if (!IS_SET(ch->specials.act, PLR_STEALTH)) {
      act("$n stops following $N.", FALSE, ch, 0, ch->master, TO_NOTVICT);
      act("$n stops following you.", FALSE, ch, 0, ch->master, TO_VICT);
    }
  }

  if (ch->master->followers->follower == ch) {	/* Head of follower-list? */
    k = ch->master->followers;
    ch->master->followers = k->next;
    free(k);
  } else {			       /* locate follower who is not head of list */

    for (k = ch->master->followers; k->next && k->next->follower != ch; k = k->next);

    if (k->next) {
      j = k->next;
      k->next = j->next;
      free(j);
    }
  }

  ch->master = 0;
  REMOVE_BIT(ch->specials.affected_by, AFF_CHARM | AFF_GROUP);
}

/* Called when a character that follows/is followed dies */
void die_follower(struct char_data *ch)
{
  struct follow_type *j, *k;

  if (ch->master)
    stop_follower(ch);

  for (k = ch->followers; k; k = j) {
    j = k->next;
    stop_follower(k->follower);
  }
}

/* Do NOT call this before having checked if a circle of followers */
/* will arise. CH will follow leader                               */
void add_follower(struct char_data *ch, struct char_data *leader)
{
  struct follow_type *k;

  assert(!ch->master);

  ch->master = leader;

  CREATE(k, struct follow_type, 1);

  k->follower = ch;
  k->next = leader->followers;
  leader->followers = k;

  act("You now follow $N.", FALSE, ch, 0, leader, TO_CHAR);
  if (!IS_SET(ch->specials.act, PLR_STEALTH)) {
    act("$n starts following you.", TRUE, ch, 0, leader, TO_VICT);
    act("$n now follows $N.", TRUE, ch, 0, leader, TO_NOTVICT);
  }
}

void say_spell(struct char_data *ch, int si)
{
  char buf[MAX_STRING_LENGTH], splwd[MAX_BUF_LENGTH];
  char buf2[MAX_STRING_LENGTH];

  int j, offs;
  struct char_data *temp_char;

  struct syllable {
    char org[10];
    char new[10];
  };

  struct syllable syls[] =
  {
    {" ", " "},
    {"ar", "abra"},
    {"au", "mela"},
    {"bless", "kado"},
    {"blind", "nose"},
    {"bur", "mosa"},
    {"cu", "judi"},
    {"ca", "jedi"},
    {"de", "oculo"},
    {"en", "fido"},
    {"light", "dies"},
    {"lo", "hi"},
    {"mor", "sido"},
    {"move", "zak"},
    {"ness", "lacri"},
    {"ning", "illa"},
    {"per", "duda"},
    {"ra", "gru"},
    {"re", "candus"},
    {"son", "sabru"},
    {"se", "or"},
    {"tect", "cula"},
    {"tri", "infa"},
    {"ven", "nofo"},
    {"a", "a"},
    {"b", "b"},
    {"c", "q"},
    {"d", "e"},
    {"e", "z"},
    {"f", "y"},
    {"g", "o"},
    {"h", "p"},
    {"i", "u"},
    {"j", "y"},
    {"k", "t"},
    {"l", "s"},
    {"m", "w"},
    {"n", "i"},
    {"o", "a"},
    {"p", "t"},
    {"q", "d"},
    {"r", "f"},
    {"s", "g"},
    {"t", "h"},
    {"u", "j"},
    {"v", "z"},
    {"w", "x"},
    {"x", "b"},
    {"y", "l"},
    {"z", "k"},
    {"", ""}
  };

  strcpy(buf, "");
  strcpy(splwd, spells[si - 1]);

  offs = 0;

  while (*(splwd + offs)) {
    for (j = 0; *(syls[j].org); j++)
      if (strncmp(syls[j].org, splwd + offs, strlen(syls[j].org)) == 0) {
	strcat(buf, syls[j].new);
	if (strlen(syls[j].org))
	  offs += strlen(syls[j].org);
	else
	  ++offs;
      }
  }

  sprintf(buf2, "$n utters the words, '%s'", buf);
  sprintf(buf, "$n utters the words, '%s'", spells[si - 1]);

  for (temp_char = real_roomp(ch->in_room)->people; temp_char; temp_char = temp_char->next_in_room)
    if (temp_char != ch) {
/*
 * **  Remove-For-Multi-Class
 * if (ch->player.class == temp_char->player.class)
 * 
 */
      if((GET_LEVEL(temp_char, CLERIC_LEVEL_IND) >= spell_info[si-1].min_level[CLERIC_LEVEL_IND]) ||
         (GET_LEVEL(temp_char, MAGE_LEVEL_IND) >= spell_info[si-1].min_level[MAGE_LEVEL_IND]) ||
         (GET_LEVEL(temp_char, WARRIOR_LEVEL_IND) >= spell_info[si-1].min_level[WARRIOR_LEVEL_IND]) ||
         (GET_LEVEL(temp_char, THIEF_LEVEL_IND) >= spell_info[si-1].min_level[THIEF_LEVEL_IND]) ||
         (GET_LEVEL(temp_char, RANGER_LEVEL_IND) >= spell_info[si-1].min_level[RANGER_LEVEL_IND]) ||
         (GET_LEVEL(temp_char, DRUID_LEVEL_IND) >= spell_info[si-1].min_level[DRUID_LEVEL_IND]))
	act(buf, FALSE, ch, 0, temp_char, TO_VICT);
      else
	act(buf2, FALSE, ch, 0, temp_char, TO_VICT);

    }
}

BYTE saves_spell(struct char_data *ch, SHORT save_type)
{
  int save;

  /* Negative apply_saving_throw makes saving throw better! */

  save = ch->specials.apply_saving_throw[save_type];

  if (IS_PC(ch)) {
/*
 * **  Remove-For-Multi-Class
 */
    save += saving_throws[BestMagicClass(ch)][save_type][GET_LEVEL(ch, BestMagicClass(ch))];
    if (GetMaxLevel(ch) > MAX_MORT)
      return (TRUE);
  }
  return (MAX(1, save) < number(1, 20));
}

BYTE ImpSaveSpell(struct char_data *ch, SHORT save_type, int mod)
{
  int save;

  /* Positive mod is better for save */

  /* Negative apply_saving_throw makes saving throw better! */

  save = ch->specials.apply_saving_throw[save_type] - mod;

  if (IS_PC(ch)) {
/*
 * **  Remove-For-Multi-Class
 */
    save += saving_throws[BestMagicClass(ch)][save_type][GET_LEVEL(ch, BestMagicClass(ch))];
/*
 *  if (GetMaxLevel(ch) >= LOW_IMMORTAL)
 *    return (TRUE);
 */
  }
  return (MAX(1, save) < number(1, 20));
}

char *skip_spaces(char *string)
{
  for (; *string && (*string) == ' '; string++);

  return (string);
}

/* Assumes that *argument does start with first letter of chopped string */

void do_cast(struct char_data *ch, char *argument, int cmd)
{
  struct room_data *rp;
  struct obj_data *tar_obj;
  struct char_data *tar_char;
  char name[MAX_INPUT_LENGTH];
  int qend, spl, i;
  BYTE target_ok;

  if (IS_NPC(ch) && (IS_NOT_SET(ch->specials.act, ACT_POLYSELF)))
    return;

  if (!IsHumanoid(ch)) {
    send_to_char("Sorry, you don't have the right form for that.\n\r", ch);
    return;
  }
  if (!IS_IMMORTAL(ch)) {
    if (BestMagicClass(ch) == WARRIOR_LEVEL_IND) {
      send_to_char("Think you had better stick to fighting...\n\r", ch);
      return;
    } else if (BestMagicClass(ch) == THIEF_LEVEL_IND) {
      send_to_char("Think you should stick to robbing and stealing...\n\r", ch);
      return;
    } else if ((BestMagicClass(ch) == RANGER_LEVEL_IND) &&
	       (GET_LEVEL(ch, RANGER_LEVEL_IND) < 10)) {
      send_to_char("In time, you shall have the power of nature...\n\r", ch);
      return;
    }
  }
  rp = real_roomp(ch->in_room);
  if (IS_SET(rp->room_flags, NO_MAGIC) && !IS_IMMORTAL(ch)) {
    send_to_char("Your lips do not move, no magic appears.\n\r", ch);
    return;
  }
  argument = skip_spaces(argument);

  if (!(*argument)) {
    send_to_char("cast 'spell name' <target>\n\r", ch);
    return;
  }
  if (*argument != '\'') {
    send_to_char("Spells must always be enclosed by single quotes: '\n\r", ch);
    return;
  }
  /* Locate the last quote && lowercase the magic words (if any) */

  for (qend = 1; *(argument + qend) && (*(argument + qend) != '\''); qend++)
    *(argument + qend) = LOWER(*(argument + qend));

  if (*(argument + qend) != '\'') {
    send_to_char("Magic must always be enclosed by single quotes: '\n\r", ch);
    return;
  }
  spl = old_search_block(argument, 1, qend - 1, spells, 0);

  if (!spl) {
    send_to_char("Your lips do not move, no magic appears.\n\r", ch);
    return;
  }
  if ((spl > 0) && (spl < MAX_SKILLS) && spell_info[spl].spell_pointer) {
    if (GET_POS(ch) < spell_info[spl].minimum_position) {
      switch (GET_POS(ch)) {
      case POSITION_SLEEPING:
	send_to_char("You dream about great magical powers.\n\r", ch);
	break;
      case POSITION_RESTING:
	send_to_char("You can't concentrate enough while resting.\n\r", ch);
	break;
      case POSITION_SITTING:
	send_to_char("You can't do this sitting!\n\r", ch);
	break;
      case POSITION_FIGHTING:
	send_to_char("Impossible! You can't concentrate enough!.\n\r", ch);
	break;
      default:
	send_to_char("It seems like you're in pretty bad shape!\n\r", ch);
	break;
      }
    } else {
      if (!IS_IMMORTAL(ch)) {
	if ((spell_info[spl].min_level[MAGE_LEVEL_IND] > GET_LEVEL(ch, MAGE_LEVEL_IND)) &&
	    (spell_info[spl].min_level[CLERIC_LEVEL_IND] > GET_LEVEL(ch, CLERIC_LEVEL_IND)) &&
	    (GET_LEVEL(ch, RANGER_LEVEL_IND) < 10)) {
	  send_to_char("Sorry, you can't do that.\n\r", ch);
	  return;
	}
      }
      argument += qend + 1;	       /* Point to the last ' */
      for (; *argument == ' '; argument++);

      /* **************** Locate targets **************** */

      target_ok = FALSE;
      tar_char = 0;
      tar_obj = 0;

      if (IS_SET(spell_info[spl].targets, TAR_VIOLENT) &&
	  check_peaceful(ch, "This is a magic dead area."))
	return;

      if (IS_NOT_SET(spell_info[spl].targets, TAR_IGNORE)) {
	argument = one_argument(argument, name);
	if (*name) {
	  if (IS_SET(spell_info[spl].targets, TAR_CHAR_ROOM)) {
	    if (tar_char = get_char_room_vis(ch, name)) {
	      if (tar_char == ch || tar_char == ch->specials.fighting ||
		  tar_char->attackers < 6 ||
		  tar_char->specials.fighting == ch)
		target_ok = TRUE;
	      else {
		send_to_char("Too much noise, you can't concentrate.\n\r", ch);
		return;
	      }
	    }
	  }
	  if (!target_ok && IS_SET(spell_info[spl].targets, TAR_CHAR_WORLD))
	    if (tar_char = get_char_vis(ch, name))
	      target_ok = TRUE;

	  if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_INV))
	    if (tar_obj = get_obj_in_list_vis(ch, name, ch->carrying))
	      target_ok = TRUE;

	  if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_ROOM))
	    if (tar_obj = get_obj_in_list_vis(ch, name, real_roomp(ch->in_room)->contents))
	      target_ok = TRUE;

	  if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_WORLD))
	    if (tar_obj = get_obj_vis(ch, name))
	      target_ok = TRUE;

	  if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_EQUIP)) {
	    for (i = 0; i < MAX_WEAR && !target_ok; i++)
	      if (ch->equipment[i] && str_cmp(name, ch->equipment[i]->name) == 0) {
		tar_obj = ch->equipment[i];
		target_ok = TRUE;
	      }
	  }
	  if (!target_ok && IS_SET(spell_info[spl].targets, TAR_SELF_ONLY))
	    if (str_cmp(GET_NAME(ch), name) == 0) {
	      tar_char = ch;
	      target_ok = TRUE;
	    }
	  if (!target_ok && IS_SET(spell_info[spl].targets, TAR_NAME)) {
	    tar_obj = (void *)name;
	    target_ok = TRUE;
	  }
	  if (tar_char) {
	    if (IS_NPC(tar_char))
	      if (IS_SET(tar_char->specials.act, ACT_IMMORTAL)) {
		send_to_char("You can't cast magic on that!", ch);
		return;
	      }
	  }
	} else {		       /* No argument was typed */
	  if (IS_SET(spell_info[spl].targets, TAR_FIGHT_SELF))
	    if (ch->specials.fighting) {
	      tar_char = ch;
	      target_ok = TRUE;
	    }
	  if (!target_ok && IS_SET(spell_info[spl].targets, TAR_FIGHT_VICT))
	    if (ch->specials.fighting) {
	      /* WARNING, MAKE INTO POINTER */
	      tar_char = ch->specials.fighting;
	      target_ok = TRUE;
	    }
	  if (!target_ok && IS_SET(spell_info[spl].targets, TAR_SELF_ONLY)) {
	    tar_char = ch;
	    target_ok = TRUE;
	  }
	}
      } else {
	target_ok = TRUE;	       /* No target, is a good target */
      }

      if (!target_ok) {
	if (*name) {
	  if (IS_SET(spell_info[spl].targets, TAR_CHAR_ROOM))
	    send_to_char("Nothing here with that name.\n\r", ch);
	  else if (IS_SET(spell_info[spl].targets, TAR_CHAR_WORLD))
	    send_to_char("Nobody playing by that name.\n\r", ch);
	  else if (IS_SET(spell_info[spl].targets, TAR_OBJ_INV))
	    send_to_char("You are not carrying anything like that.\n\r", ch);
	  else if (IS_SET(spell_info[spl].targets, TAR_OBJ_ROOM))
	    send_to_char("Nothing here by that name.\n\r", ch);
	  else if (IS_SET(spell_info[spl].targets, TAR_OBJ_WORLD))
	    send_to_char("Nothing at all by that name.\n\r", ch);
	  else if (IS_SET(spell_info[spl].targets, TAR_OBJ_EQUIP))
	    send_to_char("You are not wearing anything like that.\n\r", ch);
	  else if (IS_SET(spell_info[spl].targets, TAR_OBJ_WORLD))
	    send_to_char("Nothing at all by that name.\n\r", ch);
	} else {		       /* Nothing was given as argument */
	  if (spell_info[spl].targets < TAR_OBJ_INV)
	    send_to_char("Who should the spell be cast upon?\n\r", ch);
	  else
	    send_to_char("What should the spell be cast upon?\n\r", ch);
	}
	return;
      } else {			       /* TARGET IS OK */

	if ((tar_char == ch) && IS_SET(spell_info[spl].targets, TAR_SELF_NONO)) {
	  send_to_char("You can not cast this spell upon yourself.\n\r", ch);
	  return;
	} else if ((tar_char != ch) && IS_SET(spell_info[spl].targets, TAR_SELF_ONLY)) {
	  send_to_char("You can only cast this spell upon yourself.\n\r", ch);
	  return;
	} else if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == tar_char)) {
	  send_to_char("You are afraid that it could harm your master.\n\r", ch);
	  return;
	}
      }

      /* if (GetMaxLevel(ch) < LOW_IMMORTAL) */
	if (GET_MANA(ch) < USE_MANA(ch, spl)) {
	  send_to_char("You can't summon enough energy to cast the spell.\n\r", ch);
	  return;
	}
      if (spl != SPELL_VENTRILOQUATE)  /* :-) */
	say_spell(ch, spl);

      if(IS_MORTAL(ch))
        WAIT_STATE(ch, spell_info[spl].beats);

      if ((spell_info[spl].spell_pointer == 0) && spl > 0)
	send_to_char("Sorry, this magic has not yet been implemented\n\r", ch);
      else {
	if (number(1, 100) > (ch->skills[spl].learned + (GetMaxLevel(ch) / 5))) {	/* 101% is failure */
	  send_to_char("You lost your concentration!\n\r", ch);
	  GET_MANA(ch) -= (USE_MANA(ch, spl) >> 1);
	  return;
	}
	send_to_char("You Cast!\n\r", ch);
	if (ch->skills[spl].learned < 60) {
	  if (ch->skills[SKILL_SPELLCRAFT].learned > number(1, 101))
	    ch->skills[spl].learned += 5;
	  else
	    ch->skills[spl].learned += 2;
	}
	((*spell_info[spl].spell_pointer) (GET_LEVEL(ch, BestMagicClass(ch)),
		ch, argument, SPELL_TYPE_SPELL, tar_char, tar_obj));
	GET_MANA(ch) -= (USE_MANA(ch, spl));
      }

    }				       /* if GET_POS < min_pos */
    return;
  }
  switch (number(1, 5)) {
  case 1:
    send_to_char("Bylle Grylle Grop Gryf???\n\r", ch);
    break;
  case 2:
    send_to_char("Olle Bolle Snop Snyf?\n\r", ch);
    break;
  case 3:
    send_to_char("Olle Grylle Bolle Bylle?!?\n\r", ch);
    break;
  case 4:
    send_to_char("Gryffe Olle Gnyffe Snop???\n\r", ch);
    break;
  default:
    send_to_char("Bolle Snylle Gryf Bylle?!!?\n\r", ch);
    break;
  }
}

void assign_spell_pointers(void)
{
  int i;

  for (i = 0; i < MAX_SPL_LIST; i++)
    spell_info[i].spell_pointer = 0;

  ASSIGN_SPELL( SPELL_ARMOR,  cast_armor,  TAR_CHAR_ROOM,  12,  POSITION_STANDING,  5,  4,  1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_TELEPORT,  cast_teleport,  TAR_CHAR_ROOM | TAR_FIGHT_VICT,  12,  POSITION_FIGHTING,  33,  8,  LOW_IMMORTAL, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_BLESS,  cast_bless,  TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_CHAR_ROOM,  12,  POSITION_STANDING,  5,  LOW_IMMORTAL,  1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_BLINDNESS,  cast_blindness,  TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,  24,  POSITION_FIGHTING,  5,  8,  6, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_BURNING_HANDS,  cast_burning_hands,  TAR_IGNORE | TAR_VIOLENT,  24,  POSITION_FIGHTING,  30,  5,  LOW_IMMORTAL, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_CALL_LIGHTNING,  cast_call_lightning,  TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,  36,  POSITION_FIGHTING,  15,  LOW_IMMORTAL,  15, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_CHARM_PERSON,  cast_charm_person,  TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_FIGHT_VICT | TAR_VIOLENT,  12,  POSITION_STANDING,  5,  12,  12, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_CHILL_TOUCH,  cast_chill_touch,  TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,  12,  POSITION_FIGHTING,  15,  3,  LOW_IMMORTAL, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_CLONE,  cast_clone,  TAR_CHAR_ROOM,  12,  POSITION_STANDING,  LOW_IMMORTAL,  15,  LOW_IMMORTAL, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_COLOUR_SPRAY,  0,  TAR_IGNORE | TAR_VIOLENT,  24,  POSITION_FIGHTING,  15,  11,  LOW_IMMORTAL, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_CONTROL_WEATHER,  cast_control_weather,  TAR_IGNORE,  36,  POSITION_STANDING,  25,  10,  13, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_CREATE_FOOD,  cast_create_food,  TAR_IGNORE,  12,  POSITION_STANDING,  5,  LOW_IMMORTAL,  3, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_CREATE_WATER,  cast_create_water,  TAR_OBJ_INV | TAR_OBJ_EQUIP,  12,  POSITION_STANDING,  5,  LOW_IMMORTAL,  2, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_CURE_BLIND,  cast_cure_blind,  TAR_CHAR_ROOM,  12,  POSITION_STANDING,  5,  LOW_IMMORTAL,  4, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_CURE_CRITIC,  cast_cure_critic,  TAR_CHAR_ROOM,  12,  POSITION_FIGHTING,  11,  LOW_IMMORTAL,  9, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_CURE_LIGHT,  cast_cure_light,  TAR_CHAR_ROOM,  12,  POSITION_FIGHTING,  5,  LOW_IMMORTAL,  1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_CURSE,  cast_curse,  TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_FIGHT_VICT | TAR_VIOLENT,  24,  POSITION_STANDING,  20,  12,  12, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_DETECT_EVIL,  cast_detect_evil,  TAR_CHAR_ROOM,  12,  POSITION_STANDING,  5,  LOW_IMMORTAL,  1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_DETECT_INVISIBLE,  cast_detect_invisibility,  TAR_CHAR_ROOM,  12,  POSITION_STANDING,  5,  2,  5, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_DETECT_MAGIC,  cast_detect_magic,  TAR_CHAR_ROOM,  12,  POSITION_STANDING,  5,  1,  3, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_DETECT_POISON,  cast_detect_poison,  TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP,  12,  POSITION_STANDING,  5,  LOW_IMMORTAL,  2, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_DISPEL_EVIL,  cast_dispel_evil,  TAR_CHAR_ROOM | TAR_FIGHT_VICT,  24,  POSITION_FIGHTING,  100,  LOW_IMMORTAL,  12, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_EARTHQUAKE,  cast_earthquake,  TAR_IGNORE | TAR_VIOLENT,  24,  POSITION_FIGHTING,  15,  LOW_IMMORTAL,  8, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_ENCHANT_WEAPON,  cast_enchant_weapon,  TAR_OBJ_INV | TAR_OBJ_EQUIP,  48,  POSITION_STANDING,  100,  9,  25, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_ENERGY_DRAIN,  cast_energy_drain,  TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,  36,  POSITION_FIGHTING,  35,  17,  LOW_IMMORTAL, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_FIREBALL,  cast_fireball,  TAR_IGNORE | TAR_VIOLENT,  36,  POSITION_FIGHTING,  15,  15,  LOW_IMMORTAL, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_HARM,  cast_harm,  TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,  36,  POSITION_FIGHTING,  50,  LOW_IMMORTAL,  17, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_HEAL,  cast_heal,  TAR_CHAR_ROOM,  18,  POSITION_FIGHTING,  50,  LOW_IMMORTAL,  17, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_INVISIBLE,  cast_invisibility,  TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM | TAR_OBJ_EQUIP,  12,  POSITION_STANDING,  5,  4,  LOW_IMMORTAL, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_LIGHTNING_BOLT,  cast_lightning_bolt,  TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,  24,  POSITION_FIGHTING,  15,  9,  LOW_IMMORTAL, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_LOCATE_OBJECT,  cast_locate_object,  TAR_NAME,  12,  POSITION_STANDING,  20,  LOW_IMMORTAL,  4, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_MAGIC_MISSILE,  cast_magic_missile,  TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,  12,  POSITION_FIGHTING,  10,  1,  LOW_IMMORTAL, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_POISON,  cast_poison,  TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_FIGHT_VICT | TAR_VIOLENT,  24,  POSITION_FIGHTING,  10,  LOW_IMMORTAL,  8, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_PROTECT_FROM_EVIL,  cast_protection_from_evil,  TAR_CHAR_ROOM,  12,  POSITION_STANDING,  5,  LOW_IMMORTAL,  6, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_REMOVE_CURSE,  cast_remove_curse,  TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_OBJ_ROOM,  12,  POSITION_STANDING,  5,  LOW_IMMORTAL,  7, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_SANCTUARY,  cast_sanctuary,  TAR_CHAR_ROOM,  36,  POSITION_STANDING,  50,  LOW_IMMORTAL,  19, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_SHOCKING_GRASP,  cast_shocking_grasp,  TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,  12,  POSITION_FIGHTING,  15,  1,  LOW_IMMORTAL, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_SLEEP,  cast_sleep,  TAR_CHAR_ROOM | TAR_FIGHT_VICT,  24,  POSITION_STANDING,  15,  3,  LOW_IMMORTAL, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_STRENGTH,  cast_strength,  TAR_CHAR_ROOM,  12,  POSITION_STANDING,  10,  4,  LOW_IMMORTAL, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_SUMMON,  cast_summon,  TAR_CHAR_WORLD,  36,  POSITION_STANDING,  20,  18,  16, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_VENTRILOQUATE,  cast_ventriloquate,  TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_SELF_NONO,  12,  POSITION_STANDING,  5,  1,  LOW_IMMORTAL, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_WORD_OF_RECALL,  cast_word_of_recall,  TAR_CHAR_ROOM | TAR_SELF_ONLY,  12,  POSITION_FIGHTING,  5,  LOW_IMMORTAL,  10, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_REMOVE_POISON,  cast_remove_poison,  TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM,  12,  POSITION_STANDING,  5,  LOW_IMMORTAL,  5, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_SENSE_LIFE,  cast_sense_life,  TAR_CHAR_ROOM,  12,  POSITION_STANDING,  5,  LOW_IMMORTAL,  7, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_SNEAK,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR,  IMPLEMENTOR, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_HIDE,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR,  IMPLEMENTOR, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_STEAL,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR,  IMPLEMENTOR, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_BACKSTAB,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_PICK_LOCK,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_KICK,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_BASH,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_RESCUE,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_IDENTIFY,  cast_identify,  TAR_IGNORE,  1,  POSITION_STANDING,  100,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_INFRAVISION,  cast_infravision,  TAR_CHAR_ROOM,  12,  POSITION_STANDING,  7,  5,  LOW_IMMORTAL, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_CAUSE_LIGHT,  cast_cause_light,  TAR_CHAR_ROOM | TAR_FIGHT_VICT,  12,  POSITION_FIGHTING,  8,  LOW_IMMORTAL,  1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_CAUSE_CRITICAL,  cast_cause_critic,  TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,  18,  POSITION_FIGHTING,  11,  LOW_IMMORTAL,  9, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_FLAMESTRIKE,  cast_flamestrike,  TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,  24,  POSITION_FIGHTING,  15,  LOW_IMMORTAL,  11, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_DISPEL_GOOD,  cast_dispel_good,  TAR_CHAR_ROOM | TAR_FIGHT_VICT,  36,  POSITION_FIGHTING,  15,  LOW_IMMORTAL,  12, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_WEAKNESS,  cast_weakness,  TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,  12,  POSITION_FIGHTING,  10,  4,  LOW_IMMORTAL, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_DISPEL_MAGIC,  cast_dispel_magic,  TAR_CHAR_ROOM | TAR_FIGHT_VICT,  12,  POSITION_FIGHTING,  15,  6,  6, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_KNOCK,  cast_knock,  TAR_IGNORE,  12,  POSITION_STANDING,  10,  3,  LOW_IMMORTAL, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_KNOW_ALIGNMENT,  cast_know_alignment,  TAR_CHAR_ROOM | TAR_FIGHT_VICT,  12,  POSITION_FIGHTING,  10,  4,  3, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_ANIMATE_DEAD,  cast_animate_dead,  TAR_OBJ_ROOM,  24,  POSITION_STANDING,  15,  10,  7, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_PARALYSIS,  cast_paralyze,  TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,  36,  POSITION_FIGHTING,  40,  15,  15, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_REMOVE_PARALYSIS,  cast_remove_paralysis,  TAR_CHAR_ROOM | TAR_FIGHT_VICT,  12,  POSITION_FIGHTING,  10,  LOW_IMMORTAL,  4, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_FEAR,  cast_fear,  TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,  12,  POSITION_FIGHTING,  15,  8,  LOW_IMMORTAL, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_ACID_BLAST,  cast_acid_blast,  TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,  24,  POSITION_FIGHTING,  15,  7,  LOW_IMMORTAL, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_WATER_BREATH,  cast_water_breath,  TAR_CHAR_ROOM,  12,  POSITION_FIGHTING,  15,  4,  LOW_IMMORTAL, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_FLY,  cast_flying,  TAR_CHAR_ROOM,  12,  POSITION_FIGHTING,  15,  3,  14, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_CONE_OF_COLD,  cast_cone_of_cold,  TAR_IGNORE | TAR_VIOLENT,  24,  POSITION_FIGHTING,  15,  11,  LOW_IMMORTAL, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_METEOR_SWARM,  cast_meteor_swarm,  TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,  24,  POSITION_FIGHTING,  50,  20,  LOW_IMMORTAL, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_ICE_STORM,  cast_ice_storm,  TAR_IGNORE | TAR_VIOLENT,  12,  POSITION_FIGHTING,  15,  7,  LOW_IMMORTAL, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_SHIELD,  cast_shield,  TAR_CHAR_ROOM,  24,  POSITION_FIGHTING,  15,  1,  15, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_MON_SUM_1,  cast_mon_sum1,  TAR_IGNORE,  24,  POSITION_FIGHTING,  10,  5,  LOW_IMMORTAL, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_MON_SUM_2,  cast_mon_sum2,  TAR_IGNORE,  24,  POSITION_FIGHTING,  12,  7,  LOW_IMMORTAL, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_MON_SUM_3,  cast_mon_sum3,  TAR_IGNORE,  24,  POSITION_FIGHTING,  15,  9,  LOW_IMMORTAL, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_MON_SUM_4,  cast_mon_sum4,  TAR_IGNORE,  24,  POSITION_FIGHTING,  17,  11,  LOW_IMMORTAL, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_MON_SUM_5,  cast_mon_sum5,  TAR_IGNORE,  24,  POSITION_FIGHTING,  20,  13,  LOW_IMMORTAL, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_MON_SUM_6,  cast_mon_sum6,  TAR_IGNORE,  24,  POSITION_FIGHTING,  22,  15,  LOW_IMMORTAL, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_MON_SUM_7,  cast_mon_sum7,  TAR_IGNORE,  24,  POSITION_STANDING,  25,  17,  LOW_IMMORTAL, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_FIRESHIELD,  cast_fireshield,  TAR_SELF_ONLY | TAR_CHAR_ROOM,  24,  POSITION_STANDING,  40,  20,  19, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_CHARM_MONSTER,  cast_charm_monster,  TAR_CHAR_ROOM | TAR_VIOLENT,  18,  POSITION_STANDING,  5,  8,  8, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_CURE_SERIOUS,  cast_cure_serious,  TAR_CHAR_ROOM,  12,  POSITION_FIGHTING,  9,  30,  7, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_CAUSE_SERIOUS,  cast_cause_serious,  TAR_CHAR_ROOM | TAR_VIOLENT,  12,  POSITION_FIGHTING,  9,  30,  7, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_REFRESH,  cast_refresh,  TAR_CHAR_ROOM,  12,  POSITION_STANDING,  5,  3,  2, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_SECOND_WIND,  cast_second_wind,  TAR_CHAR_ROOM,  12,  POSITION_STANDING,  5,  12,  6, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_TURN,  cast_turn,  TAR_CHAR_ROOM,  12,  POSITION_STANDING,  5,  LOW_IMMORTAL,  1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_SUCCOR,  cast_succor,  TAR_IGNORE,  24,  POSITION_STANDING,  15,  21,  18, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_LIGHT,  cast_light,  TAR_IGNORE,  12,  POSITION_STANDING,  5,  1,  2, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_CONT_LIGHT,  cast_cont_light,  TAR_IGNORE,  24,  POSITION_STANDING,  10,  3,  4, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_CALM,  cast_calm,  TAR_CHAR_ROOM,  24,  POSITION_STANDING,  15,  4,  2, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_STONE_SKIN,  cast_stone_skin,  TAR_SELF_ONLY,  24,  POSITION_STANDING,  20,  16,  32, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_CONJURE_ELEMENTAL,  cast_conjure_elemental,  TAR_IGNORE,  24,  POSITION_STANDING,  30,  16,  14, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_TRUE_SIGHT,  cast_true_seeing,  TAR_CHAR_ROOM,  24,  POSITION_STANDING,  20,  LOW_IMMORTAL,  12, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_MINOR_CREATE,  cast_minor_creation,  TAR_IGNORE,  24,  POSITION_STANDING,  30,  8,  14, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_FAERIE_FIRE,  cast_faerie_fire,  TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT,  12,  POSITION_STANDING,  10,  5,  3, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_FAERIE_FOG,  cast_faerie_fog,  TAR_IGNORE,  24,  POSITION_STANDING,  20,  13,  10, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_CACAODEMON,  cast_cacaodemon,  TAR_IGNORE,  24,  POSITION_STANDING,  50,  30,  30, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_POLY_SELF,  cast_poly_self,  TAR_IGNORE,  12,  POSITION_FIGHTING,  30,  8,  LOW_IMMORTAL, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_MANA,  0,  TAR_IGNORE,  12,  POSITION_FIGHTING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_ASTRAL_WALK,  cast_astral_walk,  TAR_CHAR_WORLD,  12,  POSITION_STANDING,  33,  21,  18, 50, 50, 50, 50 );
/*  ASSIGN_SPELL( 102,  0,  TAR_OBJ_ROOM,  12,  POSITION_STANDING,  33,  50,  21, 50, 50, 50, 50 ); */
  ASSIGN_SPELL( SPELL_FLY_GROUP,  cast_fly_group,  TAR_IGNORE,  18,  POSITION_FIGHTING,  30,  8,  22, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_AID,  cast_aid,  TAR_CHAR_ROOM,  12,  POSITION_STANDING,  15,  LOW_IMMORTAL,  10, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_SHELTER,  cast_shelter,  TAR_IGNORE,  12,  POSITION_STANDING,  100,  10,  10, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_DOOR_BASH,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_READ_MAGIC,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_SCRIBE,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_BREW,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_PUNCH,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_TWO_HANDED,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_TWO_WEAPON,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_BANDAGE,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_SEARCH,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_SWIMMING,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_ENDURANCE,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_BARE_HAND,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_BLIND_FIGHTING,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_PARRY,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_APRAISE,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_SPEC_SMITE,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_SPEC_STAB,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_SPEC_WHIP,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_SPEC_SLASH,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_SPEC_SMASH,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_SPEC_CLEAVE,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_SPEC_CRUSH,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_SPEC_BLUDGE,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_SPEC_PIERCE,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_PEER,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_DETECT_NOISE,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_DODGE,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_BARTER,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_KNOCK_OUT,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_SPELLCRAFT,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_MEDITATION,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_HUNT,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_FIND_TRAP,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_DISARM_TRAP,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_DISARM,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_BASH_W_SHIELD,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SKILL_RIDE,  0,  TAR_IGNORE,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
  ASSIGN_SPELL( SPELL_DRAGON_BREATH,  cast_dragon_breath,  TAR_IGNORE | TAR_VIOLENT,  0,  POSITION_STANDING,  200,  IMPLEMENTOR + 1,  IMPLEMENTOR + 1, 50, 50, 50, 50 );
}

int check_falling(struct char_data *ch)
{
  struct room_data *rp, *targ;
  int done, count, saved;
  char buf[256];

  if (IS_AFFECTED(ch, AFF_FLYING))
    return (FALSE);

  rp = real_roomp(ch->in_room);
  if (!rp)
    return (FALSE);

  if (rp->sector_type != SECT_AIR)
    return (FALSE);

  act("The world spins, and you plummet out of control",
      TRUE, ch, 0, 0, TO_CHAR);
  saved = FALSE;

  done = FALSE;
  count = 0;

  while (!done && count < 100) {

/*
 * check for an exit down.
 * if there is one, go through it.
 */
    if (rp->dir_option[DOWN] && rp->dir_option[DOWN]->to_room > -1) {
      targ = real_roomp(rp->dir_option[DOWN]->to_room);
    } else {
      /*
       * pretend that this is the smash room.
       */
      if (count > 1) {

	send_to_char("You are smashed into tiny pieces.\n\r", ch);
	act("$n smashes against the ground at high speed",
	    FALSE, ch, 0, 0, TO_ROOM);
	act("You are drenched with blood and gore",
	    FALSE, ch, 0, 0, TO_ROOM);

/*
 * should damage all their stuff
 */
	DamageAllStuff(ch, BLOW_DAMAGE);

	if (!IS_IMMORTAL(ch)) {
	  GET_HIT(ch) = 0;
	  sprintf(buf, "%s has fallen to death", GET_NAME(ch));
	  log(buf);
	  if (!ch->desc)
	    GET_GOLD(ch) = 0;
	  die(ch);
	}
	return (TRUE);

      } else {

	send_to_char("You land with a resounding THUMP!\n\r", ch);
	GET_HIT(ch) = 0;
	GET_POS(ch) = POSITION_STUNNED;
	act("$n lands with a resounding THUMP!", FALSE, ch, 0, 0, TO_ROOM);
/*
 * should damage all their stuff
 */
	DamageAllStuff(ch, BLOW_DAMAGE);

	return (TRUE);

      }
    }

    act("$n plunges towards oblivion", FALSE, ch, 0, 0, TO_ROOM);
    send_to_char("You plunge from the sky\n\r", ch);
    char_from_room(ch);
    char_to_room(ch, rp->dir_option[DOWN]->to_room);
    act("$n falls from the sky", FALSE, ch, 0, 0, TO_ROOM);
    count++;

    do_look(ch, "", 0);

    if (targ->sector_type != SECT_AIR) {
      /* do damage, or kill */
      if (count == 1) {
	send_to_char("You land with a resounding THUMP!\n\r", ch);
	GET_HIT(ch) = 0;
	GET_POS(ch) = POSITION_STUNNED;
	act("$n lands with a resounding THUMP!", FALSE, ch, 0, 0, TO_ROOM);
/*
 * should damage all their stuff
 */
	DamageAllStuff(ch, BLOW_DAMAGE);

	return (TRUE);

      } else if (!saved) {
	send_to_char("You are smashed into tiny pieces.\n\r", ch);
	if (targ->sector_type >= SECT_WATER_SWIM)
	  act("$n is smashed to a pulp by $s impact with the water",
	      FALSE, ch, 0, 0, TO_ROOM);
	else
	  act("$n is smashed to a bloody pulp by $s impact with the ground",
	      FALSE, ch, 0, 0, TO_ROOM);
	act("You are drenched with blood and gore", FALSE, ch, 0, 0, TO_ROOM);

/*
 * should damage all their stuff
 */
	DamageAllStuff(ch, BLOW_DAMAGE);

	if (!IS_IMMORTAL(ch)) {
	  GET_HIT(ch) = 0;
	  sprintf(buf, "%s has fallen to death", GET_NAME(ch));
	  log(buf);
	  if (!ch->desc)
	    GET_GOLD(ch) = 0;
	  die(ch);
	}
	return (TRUE);

      } else {
	send_to_char("You land with a resounding THUMP!\n\r", ch);
	GET_HIT(ch) = 0;
	GET_POS(ch) = POSITION_STUNNED;
	act("$n lands with a resounding THUMP!", FALSE, ch, 0, 0, TO_ROOM);
/*
 * should damage all their stuff
 */
	DamageAllStuff(ch, BLOW_DAMAGE);

	return (TRUE);

      }
    } else {
/*
 * time to try the next room
 */
      rp = targ;
      targ = 0;
    }
  }

  if (count >= 100) {
    log("Someone fucked up an air room.");
    char_from_room(ch);
    char_to_room(ch, GET_HOME(ch));
    do_look(ch, "", 0);
    return (FALSE);
  }
}

void check_drowning(struct char_data *ch)
{
  struct room_data *rp;
  char buf[256];

  if (IS_AFFECTED(ch, AFF_WATERBREATH))
    return;

  rp = real_roomp(ch->in_room);

  if (!rp)
    return;

  if (rp->sector_type == SECT_UNDERWATER) {
    send_to_char("PANIC!  You're drowning!!!!!!", ch);
    GET_HIT(ch) -= number(1, 30);
    GET_MOVE(ch) -= number(10, 50);
    update_pos(ch);
    if (GET_HIT(ch) < -10) {
      sprintf(buf, "%s killed by drowning", GET_NAME(ch));
      log(buf);
      if (!ch->desc)
	GET_GOLD(ch) = 0;
      die(ch);
    }
  }
}

void check_falling_obj(struct obj_data *obj, int room)
{
  struct room_data *rp, *targ;
  int done, count;

  if (obj->in_room != room) {
    log("unusual object information in check_falling_obj");
    return;
  }
  rp = real_roomp(room);
  if (!rp)
    return;

  if (rp->sector_type != SECT_AIR)
    return;

  done = FALSE;
  count = 0;

  while (!done && count < 100) {

    if (rp->dir_option[DOWN] && rp->dir_option[DOWN]->to_room > -1) {
      targ = real_roomp(rp->dir_option[DOWN]->to_room);
    } else {
      /*
       * pretend that this is the smash room.
       */
      if (count > 1) {

	if (rp->people) {
	  act("$p smashes against the ground at high speed",
	      FALSE, rp->people, obj, 0, TO_ROOM);
	  act("$p smashes against the ground at high speed",
	      FALSE, rp->people, obj, 0, TO_CHAR);
	}
	return;

      } else {

	if (rp->people) {
	  act("$p lands with a loud THUMP!",
	      FALSE, rp->people, obj, 0, TO_ROOM);
	  act("$p lands with a loud THUMP!",
	      FALSE, rp->people, obj, 0, TO_CHAR);
	}
	return;

      }
    }

    if (rp->people) {		       /* have to reference a person */
      act("$p falls out of sight", FALSE, rp->people, obj, 0, TO_ROOM);
      act("$p falls out of sight", FALSE, rp->people, obj, 0, TO_CHAR);
    }
    obj_from_room(obj);
    obj_to_room(obj, rp->dir_option[DOWN]->to_room);
    if (targ->people) {
      act("$p falls from the sky", FALSE, targ->people, obj, 0, TO_ROOM);
      act("$p falls from the sky", FALSE, targ->people, obj, 0, TO_CHAR);
    }
    count++;

    if (targ->sector_type != SECT_AIR) {
      if (count == 1) {
	if (targ->people) {
	  act("$p lands with a loud THUMP!", FALSE, 0, obj, 0, TO_ROOM);
	  act("$p lands with a loud THUMP!", FALSE, 0, obj, 0, TO_CHAR);
	}
	return;
      } else {
	if (targ->people) {
	  if (targ->sector_type >= SECT_WATER_SWIM) {
	    act("$p smashes against the water at high speed",
		FALSE, targ->people, obj, 0, TO_ROOM);
	    act("$p smashes against the water at high speed",
		FALSE, targ->people, obj, 0, TO_CHAR);
	  } else {
	    act("$p smashes against the ground at high speed",
		FALSE, targ->people, obj, 0, TO_ROOM);
	    act("$p smashes against the ground at high speed",
		FALSE, targ->people, obj, 0, TO_CHAR);
	  }
	}
	return;

      }
    } else {
/*
 * time to try the next room
 */
      rp = targ;
      targ = 0;
    }
  }

  if (count >= 100) {
    log("Someone fucked up an air room.");
    obj_from_room(obj);
    obj_to_room(obj, 2);
    return;
  }
}

int check_nature(struct char_data *i)
{

  if (check_falling(i)) {
    return (TRUE);
  }
  check_drowning(i);

}

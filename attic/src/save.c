#include "copyright.h"
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#if defined (sun) || defined (linux)
#include <stdlib.h>
#endif
#include <string.h>
#include <time.h>
#include "merc.h"

extern int                       _filbuf(FILE *);

/*
 * Array of containers read for proper re-nesting of objects.
 */
#define MAX_NEST	100
static OBJ_DATA                 *rgObjNest[MAX_NEST];

/*
 * Local functions.
 */
void                             fwrite_char(CHAR_DATA *ch, FILE * fp);
void                             fwrite_obj(CHAR_DATA *ch, OBJ_DATA *obj,
  FILE * fp, int iNest);
void                             fread_char(CHAR_DATA *ch, FILE * fp);
void                             fread_obj(CHAR_DATA *ch, FILE * fp);

/* Courtesy of Yaz of 4th Realm */
char                            *
initial(const char *str)
{
  static char                      strint[MAX_STRING_LENGTH];

  strint[0] = tolower(str[0]);
  return strint;

}

void
do_delet(CHAR_DATA *ch, char *argument)
{
  send_to_char("If you want to DELETE, you have to spell it out.\n\r", ch);
  send_to_char("Warning... This character WILL be gone. No reimbursements.\n\r", ch);
  return;
}

void
do_delete(CHAR_DATA *ch, char *argument)
{
char  			buf[MAX_INPUT_LENGTH];
char			arg[MAX_INPUT_LENGTH];

  one_argument(argument, arg);

  if (str_cmp(arg, ch->name)) {
   send_to_char("Are you SURE you want to delete this character?\n\r", ch);
   send_to_char("Type: delete <your name> to delete.\n\r", ch);
   return;
  }

  send_to_char("This character has been deleted.\n\r", ch );
  do_quit(ch, "");
  sprintf(buf, "rm %s%s/%s",PLAYER_DIR, initial(ch->name),
	capitalize(ch->name));
  system(buf);

return;
}


/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
void 
save_char_obj(CHAR_DATA *ch)
{
  FILE                            *fp;
  char                             strsave[MAX_INPUT_LENGTH];
  char                             backup[MAX_INPUT_LENGTH];

  if (IS_NPC(ch) || ch->level < 2)
    return;

  if (ch->desc && ch->desc->original)
    ch = ch->desc->original;

  ch->save_time = current_time;
  fclose(fpReserve);

  /* player files parsed directories by Yaz 4th Realm */
  sprintf(strsave, "%s%s/%s", PLAYER_DIR, initial(ch->name),
    capitalize(ch->name));

/* old auto-backup system
 * sprintf(backup, "cp %s ../pFileBacks/%s", strsave, ch->name);
 * system( backup );
 * sprintf(backup, "gzip -f --fast ../pFileBacks/%s &", ch->name);
 * system( backup );
 */

  if (!(fp = fopen(strsave, "w"))) {
    bug("fopen %s: ", ch->name);
    perror(strsave);
  } else {
    fwrite_char(ch, fp);
    if (ch->carrying)
      fwrite_obj(ch, ch->carrying, fp, 0);
    fprintf(fp, "#END\n");
    fclose(fp);
  }
  fpReserve = fopen(NULL_FILE, "r");
  return;
}

/*
 * Write the char.
 */
void 
fwrite_char(CHAR_DATA *ch, FILE * fp)
{
  AFFECT_DATA                     *paf;
  int                              sn;
  int                              ibeamob;

  if (!ch || !fp) return;
  ibeamob= IS_NPC(ch);

  fprintf(fp, "#%s\n", ibeamob ? "MOB" : "PLAYER");

  fprintf(fp, "Nm          %s~\n", ch->name);
			smash_tilde(ch->hostname);
  fprintf(fp, "Host        %s~\n", ch->hostname);
  fprintf(fp, "ShtDsc      %s~\n", ch->short_descr);
  fprintf(fp, "LngDsc      %s~\n", ch->long_descr);
  fprintf(fp, "Dscr        %s~\n", ch->description);
  fprintf(fp, "Prmpt       %s~\n", ch->prompt);
  fprintf(fp, "Clanname    %s~\n", ch->clanname);
  fprintf(fp, "Sx          %d\n", ch->sex);
  fprintf(fp, "Eviscerated %d\n", ch->eviscerated);
  fprintf(fp, "Cla         %d\n", ch->class);
  fprintf(fp, "Dual	   %d\n", ch->dual);
  fprintf(fp, "Rce         %d\n", ch->race);
  fprintf(fp, "Clan        %d\n", ch->clan);
  fprintf(fp, "Clanrank    %d\n", ch->clanrank);
  fprintf(fp, "Sect        %d\n", ch->sect);
  fprintf(fp, "Lvl         %d\n", ch->level);
  fprintf(fp, "Trst        %d\n", ch->trust);
  if (!ibeamob)
  fprintf(fp, "Security    %d\n", ch->pcdata->security);	/* OLC */
  fprintf(fp, "Quest       %d\n", ch->quest);
  fprintf(fp, "Qstact      %d\n", ch->questact);
  fprintf(fp, "Wizbt       %d\n", ch->wizbit);
  fprintf(fp, "Playd       %d\n",
    ch->played + (int)(current_time - ch->logon));
  fprintf(fp, "Note        %ld\n", ch->last_note);
  fprintf(fp, "Pet         %d\n", ch->mypet);
  fprintf(fp, "Petsname    %s~\n", ch->pets_name);
  fprintf(fp, "Room        %d\n",
    (ch->in_room == get_room_index(ROOM_VNUM_LIMBO)
      && ch->was_in_room)
    ? ch->was_in_room->vnum
    : ch->in_room->vnum);

  fprintf(fp, "Home        %d\n", ch->home_room);
  fprintf(fp, "HpMnMv      %d %d %d %d %d %d\n",
    ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move);
  fprintf(fp, "Gold        %d\n", ch->gold);
  fprintf(fp, "Bank        %d\n", ch->bank);
  fprintf(fp, "Exp         %d\n", ch->exp);
  fprintf(fp, "Tneed       %d\n", ch->total_need);
  fprintf(fp, "Recalls     %d\n", ch->needed);
  fprintf(fp, "Act         %d\n", ch->act);
  fprintf(fp, "AffdBy      %d\n", ch->affected_by);
  /* Bug fix from Alander */
  fprintf(fp, "Pos         %d\n",
    ch->position == POS_FIGHTING ? POS_STANDING : ch->position);

  fprintf(fp, "Prac        %d\n", ch->practice);
  fprintf(fp, "SavThr      %d\n", ch->saving_throw);
  fprintf(fp, "SavThrEvil  %d\n", ch->saving_throw_evil);
  fprintf(fp, "SavThrBrea  %d\n", ch->saving_throw_breath);
  fprintf(fp, "SavThrElem  %d\n", ch->saving_throw_elemental);
  fprintf(fp, "SavThrFire  %d\n", ch->saving_throw_fire);
  fprintf(fp, "Align       %d\n", ch->alignment);
  fprintf(fp, "Hit         %d\n", ch->hitroll);
  fprintf(fp, "Dam         %d\n", ch->damroll);
  fprintf(fp, "Armr        %d\n", ch->armor);
  fprintf(fp, "Wimp        %d\n", ch->wimpy);
  fprintf(fp, "Deaf        %d\n", ch->deaf);

  if (IS_NPC(ch)) {
    fprintf(fp, "Vnum        %d\n", ch->pIndexData->vnum);
  } else {
    fprintf(fp, "Paswd       %s~\n", ch->pcdata->pwd);
    fprintf(fp, "Bmfin       %s~\n", ch->pcdata->bamfin);
    fprintf(fp, "Bmfout      %s~\n", ch->pcdata->bamfout);
    fprintf(fp, "Ttle        %s~\n", ch->pcdata->title);
    fprintf(fp, "PreTtle     %s~\n", ch->pcdata->pretitle);
    fprintf(fp, "AtrPrm      %d %d %d %d %d\n",
      ch->pcdata->perm_str,
      ch->pcdata->perm_int,
      ch->pcdata->perm_wis,
      ch->pcdata->perm_dex,
      ch->pcdata->perm_con);

    fprintf(fp, "AtrMd       %d %d %d %d %d\n",
      ch->pcdata->mod_str,
      ch->pcdata->mod_int,
      ch->pcdata->mod_wis,
      ch->pcdata->mod_dex,
      ch->pcdata->mod_con);

    fprintf(fp, "AtrMult     %d %d %d %d %d\n",
      get_curr_force(ch),
      get_curr_intui(ch),
      get_curr_luck(ch),
      get_curr_speed(ch),
      get_curr_resil(ch));
      

    fprintf(fp, "Cond        %d %d %d\n",
      ch->pcdata->condition[0],
      ch->pcdata->condition[1],
      ch->pcdata->condition[2]);

    fprintf(fp, "Pglen       %d\n", ch->pcdata->pagelen);

    for (sn = 0; sn < MAX_SKILL; sn++) {
      if (skill_table[sn].name && ch->pcdata->learned[sn] > 0) {
	fprintf(fp, "Skll        %d '%s'\n",
	  ch->pcdata->learned[sn], skill_table[sn].name);
      }
    }
  }

  for (paf = ch->affected; paf; paf = paf->next) {
    if (paf->deleted)
      continue;

    fprintf(fp, "Aff       %3d %3d %3d %3d %10d\n",
      paf->type,
      paf->duration,
      paf->modifier,
      paf->location,
      paf->bitvector);
  }

  fprintf(fp, "End\n\n");
  return;
}

/*
 * Write an object and its contents.
 */
void 
fwrite_obj(CHAR_DATA *ch, OBJ_DATA *obj, FILE * fp, int iNest)
{
  AFFECT_DATA                     *paf;
  EXTRA_DESCR_DATA                *ed;

  /*
     * Slick recursion to write lists backwards,
     *   so loading them will load in forwards order.
     */
  if (obj->next_content)
    fwrite_obj(ch, obj->next_content, fp, iNest);

  /*
     * Castrate storage characters.
     */
  if (ch->level + 12 < obj->level 
    || obj->item_type == ITEM_KEY
    || obj->item_type == ITEM_FOOD
    || IS_SET(obj->extra_flags, ITEM_NOSAVE)
    || obj->deleted)
    return;

  fprintf(fp, "#NEWOBJECT\n");
  fprintf(fp, "Nest         %d\n", iNest);
  fprintf(fp, "Name         %s~\n", obj->name);
  fprintf(fp, "ShortDescr   %s~\n", obj->short_descr);
  fprintf(fp, "Description  %s~\n", obj->description);
  fprintf(fp, "Vnum         %d\n", obj->pIndexData->vnum);
  fprintf(fp, "ExtraFlags   %d\n", obj->extra_flags);
  fprintf(fp, "WearFlags    %d\n", obj->wear_flags);
  fprintf(fp, "WearLoc      %d\n", obj->wear_loc);
  fprintf(fp, "ItemType     %d\n", obj->item_type);
  fprintf(fp, "Weight       %d\n", obj->weight);
  fprintf(fp, "Level        %d\n", obj->level);
  fprintf(fp, "Timer        %d\n", obj->timer);
  fprintf(fp, "Cost         %d\n", obj->cost);
  fprintf(fp, "Values       %d %d %d %d\n",
    obj->value[0], obj->value[1], obj->value[2], obj->value[3]);

  switch (obj->item_type) {
  case ITEM_POTION:
  case ITEM_SCROLL:
    if (obj->value[1] > 0) {
      fprintf(fp, "Spell 1      '%s'\n",
	skill_table[obj->value[1]].name);
    }
    if (obj->value[2] > 0) {
      fprintf(fp, "Spell 2      '%s'\n",
	skill_table[obj->value[2]].name);
    }
    if (obj->value[3] > 0) {
      fprintf(fp, "Spell 3      '%s'\n",
	skill_table[obj->value[3]].name);
    }
    break;

  case ITEM_PILL:
  case ITEM_STAFF:
  case ITEM_WAND:
    if (obj->value[3] > 0) {
      fprintf(fp, "Spell 3      '%s'\n",
	skill_table[obj->value[3]].name);
    }
    break;
  }

  for (paf = obj->affected; paf; paf = paf->next) {
    fprintf(fp, "Affect       %d %d %d %d %d\n",
      paf->type,
      paf->duration,
      paf->modifier,
      paf->location,
      paf->bitvector);
  }

  for (ed = obj->extra_descr; ed; ed = ed->next) {
    fprintf(fp, "ExtraDescr   %s~ %s~\n",
      ed->keyword, ed->description);
  }

  fprintf(fp, "End\n\n");

  if (obj->contains)
    fwrite_obj(ch, obj->contains, fp, iNest + 1);

  tail_chain();
  return;
}

/*
 * Load a char and inventory into a new ch structure.
 */
UBYTE 
load_char_obj(DESCRIPTOR_DATA *d, char *name)
{
  FILE                            *fp;
  static PC_DATA                   pcdata_zero;
  CHAR_DATA                       *ch;
  char                             buf[MAX_STRING_LENGTH];
  char                             strsave[MAX_INPUT_LENGTH];
  UBYTE                             found;

  if (!char_free) {
    ch = alloc_perm(sizeof(*ch));
  } else {
    ch = char_free;
    char_free = char_free->next;
  }
  clear_char(ch);

  if (!pcdata_free) {
    ch->pcdata = alloc_perm(sizeof(*ch->pcdata));
  } else {
    ch->pcdata = pcdata_free;
    pcdata_free = pcdata_free->next;
  }
  *ch->pcdata = pcdata_zero;

  d->character = ch;
  ch->desc = d;
  ch->name = str_dup(name);
  ch->prompt = str_dup("<%hhp %mm %vmv> ");
  ch->last_note = 0;
  ch->eviscerated = 0;
  ch->gold = 0;
  ch->bank = 0;
  ch->mypet = 0;
  ch->act = PLR_BLANK
    | PLR_COMBINE
    | PLR_PROMPT;
  ch->pcdata->pwd = str_dup("");
  ch->pcdata->bamfin = str_dup("");
  ch->pcdata->bamfout = str_dup("");
  ch->pcdata->title = str_dup("");
  ch->pcdata->pretitle = str_dup("");
  ch->pcdata->perm_str = 13;
  ch->pcdata->perm_int = 13;
  ch->pcdata->perm_wis = 13;
  ch->pcdata->perm_dex = 13;
  ch->pcdata->perm_con = 13;
  ch->pcdata->condition[COND_THIRST] = 48;
  ch->pcdata->condition[COND_FULL] = 48;
  ch->pcdata->pagelen = 20;

  ch->pcdata->switched = FALSE;

  found = FALSE;
  fclose(fpReserve);

  /* parsed player file directories by Yaz of 4th Realm */
  /* decompress if .gz file exists - Thx Alander */
  sprintf(strsave, "%s%s/%s.gz", PLAYER_DIR, initial(ch->name),
    capitalize(name));
  if ((fp = fopen(strsave, "r"))) {
    fclose(fp);
    sprintf(buf, "gzip -dfq %s", strsave);
    system(buf);
  }
  sprintf(strsave, "%s%s/%s", PLAYER_DIR, initial(ch->name),
    capitalize(name));
  if ((fp = fopen(strsave, "r"))) {
    int                              iNest;

    for (iNest = 0; iNest < MAX_NEST; iNest++)
      rgObjNest[iNest] = NULL;

    found = TRUE;
    for (;;) {
      char                             letter;
      char                            *word;

      letter = fread_letter(fp);
      if (letter == '*') {
	fread_to_eol(fp);
	continue;
      }
      if (letter != '#') {
	bug("# not found.");
	break;
      }
      word = fread_word(fp);
      if (!str_cmp(word, "PLAYER"))
	fread_char(ch, fp);
      else if (!str_cmp(word, "NEWOBJECT"))
	fread_obj(ch, fp);
      else if (!str_cmp(word, "END"))
	break;
      else {
	bug("bad section.");
	break;
      }
    }
    fclose(fp);
  }
  fpReserve = fopen(NULL_FILE, "r");
  return found;
}

/*
 * Read in a char.
 */

#if defined( KEY )
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

void 
fread_char(CHAR_DATA *ch, FILE * fp)
{
  char                            *word;
  char                             buf[MAX_STRING_LENGTH];
  UBYTE                             fMatch;

  for (;;) {
    word = feof(fp) ? "End" : fread_word(fp);
    fMatch = FALSE;

    switch (toupper(word[0])) {
    case '*':
      fMatch = TRUE;
      fread_to_eol(fp);
      break;

    case 'A':
      KEY("Act", ch->act, fread_number(fp));
      KEY("AffdBy", ch->affected_by, fread_number(fp));
      KEY("Align", ch->alignment, fread_number(fp));
      KEY("Armr", ch->armor, fread_number(fp));

      if (!str_cmp(word, "Aff")) {
	AFFECT_DATA                     *paf;

	if (!affect_free) {
	  paf = alloc_perm(sizeof(*paf));
	} else {
	  paf = affect_free;
	  affect_free = affect_free->next;
	}

	paf->type = fread_number(fp);
	paf->duration = fread_number(fp);
	paf->modifier = fread_number(fp);
	paf->location = fread_number(fp);
	paf->bitvector = fread_number(fp);
	paf->deleted = FALSE;
	paf->next = ch->affected;
	ch->affected = paf;
	fMatch = TRUE;
	break;
      }
      if (!str_cmp(word, "AtrMd")) {
	ch->pcdata->mod_str = fread_number(fp);
	ch->pcdata->mod_int = fread_number(fp);
	ch->pcdata->mod_wis = fread_number(fp);
	ch->pcdata->mod_dex = fread_number(fp);
	ch->pcdata->mod_con = fread_number(fp);
	fMatch = TRUE;
	break;
      }
      if (!str_cmp(word, "AtrPrm")) {
	ch->pcdata->perm_str = fread_number(fp);
	ch->pcdata->perm_int = fread_number(fp);
	ch->pcdata->perm_wis = fread_number(fp);
	ch->pcdata->perm_dex = fread_number(fp);
	ch->pcdata->perm_con = fread_number(fp);
	fMatch = TRUE;
	break;
      }
      if (!str_cmp(word, "AtrMult")) {
        ch->pcdata->mult_str = fread_number(fp);
	ch->pcdata->mult_int = fread_number(fp);
	ch->pcdata->mult_wis = fread_number(fp);
	ch->pcdata->mult_dex = fread_number(fp);
	ch->pcdata->mult_con = fread_number(fp);
	fMatch = TRUE;	
	break;
      }	
      break;

    case 'B':
      KEY("Bmfin", ch->pcdata->bamfin, fread_string(fp));
      KEY("Bmfout", ch->pcdata->bamfout, fread_string(fp));
      KEY("Bank", ch->bank, fread_number(fp));
      break;

    case 'C':
      KEY("Clan", ch->clan, fread_number(fp));
      KEY("Clanrank", ch->clanrank, fread_number(fp));
      KEY("Clanname", ch->clanname, fread_string(fp));
      KEY("Cla", ch->class, fread_number(fp));

      if (!str_cmp(word, "Cond")) {
	ch->pcdata->condition[0] = fread_number(fp);
	ch->pcdata->condition[1] = fread_number(fp);
	ch->pcdata->condition[2] = fread_number(fp);
	fMatch = TRUE;
	break;
      }
      break;

    case 'D':
      KEY("Dam", ch->damroll, fread_number(fp));
      KEY("Deaf", ch->deaf, fread_number(fp));
      KEY("Dscr", ch->description, fread_string(fp));
      KEY("Dual", ch->dual, fread_number(fp));
      break;

    case 'E':
      if (!str_cmp(word, "End"))
	return;
      KEY("Exp", ch->exp, fread_number(fp));
      KEY("Eviscerated", ch->eviscerated, fread_number(fp));
      break;

    case 'G':
      KEY("Gold", ch->gold, fread_number(fp));
      break;

    case 'H':
      KEY("Hit", ch->hitroll, fread_number(fp));
      KEY("Home", ch->home_room, fread_number(fp));
      if (!str_cmp(word, "Host")) {
	fread_to_eol(fp);
	fMatch = TRUE;
	break;
      }
      
      if (!str_cmp(word, "HpMnMv")) {
	ch->hit = fread_number(fp);
	ch->max_hit = fread_number(fp);
	ch->mana = fread_number(fp);
	ch->max_mana = fread_number(fp);
	ch->move = fread_number(fp);
	ch->max_move = fread_number(fp);
	fMatch = TRUE;
	break;
      }
      break;

    case 'I':
      KEY("Invited", ch->invited, fread_number(fp));
      break;

    case 'L':
      KEY("Lvl", ch->level, fread_number(fp));
      KEY("LngDsc", ch->long_descr, fread_string(fp));
      break;

    case 'N':
      if (!str_cmp(word, "Nm")) {
	/*
		 * Name already set externally.
		 */
	fread_to_eol(fp);
	fMatch = TRUE;
	break;
      }
      KEY("Need", ch->needed, fread_number(fp));
      KEY("Note", ch->last_note, fread_number(fp));
      break;

    case 'P':
      KEY("Pglen", ch->pcdata->pagelen, fread_number(fp));
      KEY("Paswd", ch->pcdata->pwd, fread_string(fp));
      KEY("Playd", ch->played, fread_number(fp));
      KEY("Pos", ch->position, fread_number(fp));
      KEY("Prac", ch->practice, fread_number(fp));
      KEY("Prmpt", ch->prompt, fread_string(fp));
      KEY("PreTtle", ch->pcdata->pretitle, fread_string(fp));
      KEY("Pet", ch->mypet, fread_number(fp));
      KEY("Petsname", ch->pets_name, fread_string(fp));
      break;

    case 'Q':
      KEY("Quest", ch->quest, fread_number(fp));
      KEY("Qstact", ch->questact, fread_number(fp));
      break;

    case 'R':
      KEY("Rce", ch->race, fread_number(fp));
      KEY("Recalls", ch->needed, fread_number(fp));

      if (!str_cmp(word, "Room")) {
	ch->in_room = get_room_index(fread_number(fp));
	if (!ch->in_room)
	  ch->in_room = get_room_index(ROOM_VNUM_LIMBO);
	fMatch = TRUE;
	break;
      }
      break;

    case 'S':
      KEY("SavThr", ch->saving_throw, fread_number(fp));
      KEY("SavThrEvil", ch->saving_throw_evil, fread_number(fp));
      KEY("SavThrBrea", ch->saving_throw_breath, fread_number(fp));
      KEY("SavThrElem", ch->saving_throw_elemental, fread_number(fp));
      KEY("SavThrFire", ch->saving_throw_fire, fread_number(fp));
      KEY("Sect", ch->sect, fread_number(fp));
      KEY("Sx", ch->sex, fread_number(fp));
      KEY("ShtDsc", ch->short_descr, fread_string(fp));
      KEY("Security", ch->pcdata->security, fread_number(fp));	/* OLC */

      if (!str_cmp(word, "Skll")) {
	int                              sn;
	int                              value;

	value = fread_number(fp);
	sn = skill_lookup(fread_word(fp));
	if(sn < 0)
	{
	  bug("unknown skill.");
	  fMatch = FALSE;
	}
	else
	{
	  ch->pcdata->learned[sn] = value;
	  fMatch = TRUE;
	}
      }
      break;

    case 'T':
      KEY("Tneed", ch->total_need, fread_number(fp));
      KEY("Trst", ch->trust, fread_number(fp));

      if (!str_cmp(word, "Ttle")) {
	ch->pcdata->title = fread_string(fp);
	if (isalpha(ch->pcdata->title[0])
	  || isdigit(ch->pcdata->title[0])) {
	  sprintf(buf, " %s", ch->pcdata->title);
	  free_string(ch->pcdata->title);
	  ch->pcdata->title = str_dup(buf);
	}
	fMatch = TRUE;
	break;
      }
      break;

    case 'V':
      if (!str_cmp(word, "Vnum")) {
	ch->pIndexData = get_mob_index(fread_number(fp));
	fMatch = TRUE;
	break;
      }
      break;

    case 'W':
      KEY("Wimp", ch->wimpy, fread_number(fp));
      KEY("Wizbt", ch->wizbit, fread_number(fp));
      break;
    }

    /* Make sure old chars have this field - Kahn */
    if (!ch->pcdata->pagelen)
      ch->pcdata->pagelen = 20;
    if (!ch->prompt || ch->prompt == '\0')
      ch->prompt = str_dup("<%hhp %mm %vmv> ");

    /* Make sure old chars do not have pagelen > 60 - Kahn */
    if (ch->pcdata->pagelen > 60)
      ch->pcdata->pagelen = 60;

    if (!fMatch) {
      sprintf(buf, "no match on: %s\n\r", word);
      bug(buf);
      fread_to_eol(fp);
    }
  }
}

void 
fread_obj(CHAR_DATA *ch, FILE * fp)
{
  static OBJ_DATA                  obj_zero;
  OBJ_DATA                        *obj;
  char                            *word;
  int                              iNest;
  UBYTE                             fMatch;
  UBYTE                             fNest;
  UBYTE                             fVnum;

  if (!obj_free) {
    obj = alloc_perm(sizeof(*obj));
  } else {
    obj = obj_free;
    obj_free = obj_free->next;
  }

  *obj = obj_zero;
  obj->name = str_dup("");
  obj->short_descr = str_dup("");
  obj->description = str_dup("");
  obj->deleted = FALSE;

  fNest = FALSE;
  fVnum = TRUE;
  iNest = 0;

  for (;;) {
    word = feof(fp) ? "End" : fread_word(fp);
    fMatch = FALSE;

    switch (toupper(word[0])) {
    case '*':
      fMatch = TRUE;
      fread_to_eol(fp);
      break;

    case 'A':
      if (!str_cmp(word, "Affect")) {
	AFFECT_DATA                     *paf;

	if (!affect_free) {
	  paf = alloc_perm(sizeof(*paf));
	} else {
	  paf = affect_free;
	  affect_free = affect_free->next;
	}

	paf->type = fread_number(fp);
	paf->duration = fread_number(fp);
	paf->modifier = fread_number(fp);
	paf->location = fread_number(fp);
	paf->bitvector = fread_number(fp);
	paf->next = obj->affected;
	obj->affected = paf;
	fMatch = TRUE;
	break;
      }
      break;

    case 'C':
      KEY("Cost", obj->cost, fread_number(fp));
      break;

    case 'D':
      KEY("Description", obj->description, fread_string(fp));
      break;

    case 'E':
      KEY("ExtraFlags", obj->extra_flags, fread_number(fp));

      if (!str_cmp(word, "ExtraDescr")) {
	EXTRA_DESCR_DATA                *ed;

	if (!extra_descr_free) {
	  ed = alloc_perm(sizeof(*ed));
	} else {
	  ed = extra_descr_free;
	  extra_descr_free = extra_descr_free->next;
	}

	ed->keyword = fread_string(fp);
	ed->description = fread_string(fp);
	ed->next = obj->extra_descr;
	obj->extra_descr = ed;
	fMatch = TRUE;
      }
      if (!str_cmp(word, "End")) {
	if (!fNest || !fVnum) {
	  bug("incomplete object.");
	  free_string(obj->name);
	  free_string(obj->description);
	  free_string(obj->short_descr);
	  obj->next = obj_free;
	  obj_free = obj;
	  return;
	} else {
	  obj->next = object_list;
	  object_list = obj;
	  obj->pIndexData->count++;
	  if (iNest == 0 || !rgObjNest[iNest])
	    obj_to_char(obj, ch);
	  else
	    obj_to_obj(obj, rgObjNest[iNest - 1]);
	  return;
	}
      }
      break;

    case 'I':
      KEY("ItemType", obj->item_type, fread_number(fp));
      break;

    case 'L':
      KEY("Level", obj->level, fread_number(fp));
      break;

    case 'N':
      KEY("Name", obj->name, fread_string(fp));

      if (!str_cmp(word, "Nest")) {
	iNest = fread_number(fp);
	if (iNest < 0 || iNest >= MAX_NEST) {
	  bug("bad nest %d.", iNest);
	} else {
	  rgObjNest[iNest] = obj;
	  fNest = TRUE;
	}
	fMatch = TRUE;
      }
      break;

    case 'S':
      KEY("ShortDescr", obj->short_descr, fread_string(fp));

      if (!str_cmp(word, "Spell")) {
	int                              iValue;
	int                              sn;

	iValue = fread_number(fp);
	sn = skill_lookup(fread_word(fp));
	if (iValue < 0 || iValue > 3) {
	  bug("bad iValue %d.", iValue);
	} else if (sn < 0) {
	  bug("unknown skill.");
	} else {
	  obj->value[iValue] = sn;
	}
	fMatch = TRUE;
	break;
      }
      break;

    case 'T':
      KEY("Timer", obj->timer, fread_number(fp));
      break;

    case 'V':
      if (!str_cmp(word, "Values")) {
	obj->value[0] = fread_number(fp);
	obj->value[1] = fread_number(fp);
	obj->value[2] = fread_number(fp);
	obj->value[3] = fread_number(fp);
	fMatch = TRUE;
	break;
      }
      if (!str_cmp(word, "Vnum")) {
	int                              vnum;

	vnum = fread_number(fp);
	if (!(obj->pIndexData = get_obj_index(vnum)))
	{
	  bug("bad vnum %d.", vnum);
	  fVnum = FALSE;
	  fMatch = FALSE;
	}
	else
	{
	  fVnum = TRUE;
	fMatch = TRUE;
	obj->spec_fun = obj->pIndexData->spec_fun;
	}
	break;
      }
      break;

    case 'W':
      KEY("WearFlags", obj->wear_flags, fread_number(fp));
      KEY("WearLoc", obj->wear_loc, fread_number(fp));
      KEY("Weight", obj->weight, fread_number(fp));
      break;

    }

    if (!fMatch) {
      bug("no match.");
      fread_to_eol(fp);
    }
  }
}

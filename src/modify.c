/*
 * file: modify.c                                         Part of DIKUMUD
 * Usage: Run-time modification (by users) of game variables
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#include "include/global.h"
#include "include/bug.h"
#include "include/utils.h"
#include "include/interpreter.h"
#include "include/handler.h"
#include "include/db.h"
#include "include/comm.h"
#include "include/multiclass.h"
#define _MODIFY_C
#include "include/modify.h"

int REBOOT_AT1, REBOOT_AT2;	/* 0-23, time of optional reboot if -e lib/reboot */
struct room_data *world;	/* dyn alloc'ed array of rooms     */
char *string_fields[] =
{
  "name",
  "short",
  "long",
  "description",
  "title",
  "delete-description",
  "\n"
};

char *room_fields[] =
{
  "name",			       /* 1 */
  "desc",			       /* 2 */
  "flags",			       /* 3 */
  "exit",			       /* 4 */
  "xdesc",			       /* 5 */
  "extra",			       /* 6 */
  "rivr",			       /* 7 */
  "tele",			       /* 8 */
  "\n"
};

/* maximum length for text field x+1 */
int length[] =
{
  15,
  60,
  256,
  240,
  60
};

int room_length[] =
{
  80,
  1024,
  50,
  50,
  512,
  512,
  50,
  100
};

char *skill_fields[] =
{
  "learned",
  "affected",
  "duration",
  "recognize",
  "\n"
};

int max_value[] =
{
  255,
  255,
  10000,
  1
};

/*
 * modification of malloc'ed strings
 */

/* Add user input to the 'current' string (as defined by d->str) */

void string_add(struct descriptor_data *d, char *str)
{
  char *scan;
  int terminator = 0;

  /* determine if this is the terminal string, and truncate if so */
  for (scan = str; *scan; scan++)
    if ((terminator = (*scan == '@'))) {
      *scan = '\0';
      break;
    }
  if (!(*d->str)) {
    if (strlen(str) > d->max_str) {
      cprintf(d->character, "String too long - Truncated.\n\r");
      *(str + d->max_str) = '\0';
      terminator = 1;
    }
    CREATE(*d->str, char, strlen(str) + 3);

    strcpy(*d->str, str);
  } else {
    if (strlen(str) + strlen(*d->str) > d->max_str) {
      cprintf(d->character, "String too long. Last line skipped.\n\r");
      terminator = 1;
    } else {
      RECREATE(*d->str, char, strlen(*d->str) + strlen(str) + 3);
      strcat(*d->str, str);
    }
  }

  if (terminator) {
    d->str = 0;
    if (d->connected == CON_EDIT_DESCRIPTION) {
      SEND_TO_Q(login_menu, d);
      d->connected = CON_MENU_SELECT;
    }
  } else
    strcat(*d->str, "\n\r");
}

/* interpret an argument for do_string */
void quad_arg(char *arg, int *type, char *name, int *field, char *string)
{
  char buf[MAX_STRING_LENGTH];

  /* determine type */
  arg = one_argument(arg, buf);
  if (is_abbrev(buf, "char"))
    *type = TP_MOB;
  else if (is_abbrev(buf, "obj"))
    *type = TP_OBJ;
  else {
    *type = TP_ERROR;
    return;
  }

  /* find name */
  arg = one_argument(arg, name);

  /* field name and number */
  arg = one_argument(arg, buf);
  if (!(*field = old_search_block(buf, 0, strlen(buf), string_fields, 0)))
    return;
  /* string */
  for (; isspace(*arg); arg++);
  for (; (*string = *arg); arg++, string++);
  return;
}

/* modification of malloc'ed strings in chars/objects */
void do_string(struct char_data *ch, char *arg, int cmd)
{

  char name[MAX_STRING_LENGTH], string[MAX_STRING_LENGTH];
  struct extra_descr_data *ed, *tmp;
  int field, type;
  struct char_data *mob;
  struct obj_data *obj;

  if (IS_NPC(ch))
    return;

  quad_arg(arg, &type, name, &field, string);

  if (type == TP_ERROR) {
    cprintf(ch,  "Syntax:\n\rstring (char) <name> <field> [<string>].");
    return;
  }
  if (!field) {
    cprintf(ch, "No field by that name. Try 'help string'.\n\r");
    return;
  }
  if (type == TP_MOB) {
    /* locate the beast */
    if (!(mob = get_char_vis(ch, name))) {
      cprintf(ch, "I don't know anyone by that name...\n\r");
      return;
    }
    switch (field) {
    case 1:
      if (!IS_NPC(mob) && GetMaxLevel(ch) < IMPLEMENTOR) {
	cprintf(ch, "You can't change that field for players.");
	return;
      }
      if (!*string) {
	cprintf(ch, "You have to supply a name!\n\r");
	return;
      }
      ch->desc->str = &mob->player.name;
      if (!IS_NPC(mob))
	cprintf(ch, "WARNING: You have changed the name of a player.\n\r");
      break;
    case 2:
      if (!IS_NPC(mob)) {
	cprintf(ch, "That field is for monsters only.\n\r");
	return;
      }
      ch->desc->str = &mob->player.short_descr;
      break;
    case 3:
      if (!IS_NPC(mob)) {
	cprintf(ch, "That field is for monsters only.\n\r");
	return;
      }
      ch->desc->str = &mob->player.long_descr;
      break;
    case 4:
      ch->desc->str = &mob->player.description;
      break;
    case 5:
      if (IS_NPC(mob)) {
	cprintf(ch, "Monsters have no titles.\n\r");
	return;
      }
      if ((GetMaxLevel(ch) > GetMaxLevel(mob)) && (ch != mob))
	ch->desc->str = &mob->player.title;
      else {
	cprintf(ch, "Sorry, can't set the title of someone of highter level.\n\r");
	return;
      }
      break;
    default:
      cprintf(ch, "That field is undefined for monsters.\n\r");
      return;
      break;
    }
  } else {
    cprintf(ch, "Stringing of objects is no longer allowed for now.\n\r");
    return;

    /* type == TP_OBJ */
    /* locate the object */
    if (!(obj = get_obj_vis(ch, name))) {
      cprintf(ch, "Can't find such a thing here..\n\r");
      return;
    }
    switch (field) {

    case 1:
      if (!*string) {
	cprintf(ch, "You have to supply a keyword.\n\r");
	return;
      } else {
	ch->desc->str = &obj->name;
	break;
      }
      break;
    case 2:
      ch->desc->str = &obj->short_description;
      break;
    case 3:
      ch->desc->str = &obj->description;
      break;
    case 4:
      if (!*string) {
	cprintf(ch, "You have to supply a keyword.\n\r");
	return;
      }
      /* try to locate extra description */
      for (ed = obj->ex_description;; ed = ed->next)
	if (!ed) {
	  CREATE(ed, struct extra_descr_data, 1);

	  ed->next = obj->ex_description;
	  obj->ex_description = ed;
	  CREATE(ed->keyword, char, strlen(string) + 1);

	  strcpy(ed->keyword, string);
	  ed->description = 0;
	  ch->desc->str = &ed->description;
	  cprintf(ch, "New field.\n\r");
	  break;
	} else if (!str_cmp(ed->keyword, string)) {	/* the field exists */
	  DESTROY(ed->description);
	  ed->description = 0;
	  ch->desc->str = &ed->description;
	  cprintf(ch,  "Modifying description.\n\r");
	  break;
	}
      ch->desc->max_str = MAX_STRING_LENGTH;
      return;			       /* the stndrd (see below) procedure does not apply here */
      break;
    case 6:			       /* deletion */
      if (!*string) {
	cprintf(ch, "You must supply a field name.\n\r");
	return;
      }
      /* try to locate field */
      for (ed = obj->ex_description;; ed = ed->next)
	if (!ed) {
	  cprintf(ch, "No field with that keyword.\n\r");
	  return;
	} else if (!str_cmp(ed->keyword, string)) {
	  DESTROY(ed->keyword);
	  if (ed->description)
	    DESTROY(ed->description);

	  /* delete the entry in the desr list */
	  if (ed == obj->ex_description)
	    obj->ex_description = ed->next;
	  else {
	    for (tmp = obj->ex_description; tmp->next != ed;
		 tmp = tmp->next);
	    tmp->next = ed->next;
	  }
	  DESTROY(ed);

	  cprintf(ch, "Field deleted.\n\r");
	  return;
	}
      break;
    default:
      cprintf(ch,  "That field is undefined for objects.\n\r");
      return;
      break;
    }
  }

  if (*ch->desc->str) {
    DESTROY(*ch->desc->str);
  }
  if (*string) {		       /* there was a string in the argument array */
    if (strlen(string) > length[field - 1]) {
      cprintf(ch, "String too long - truncated.\n\r");
      *(string + length[field - 1]) = '\0';
    }
    CREATE(*ch->desc->str, char, strlen(string) + 1);

    strcpy(*ch->desc->str, string);
    ch->desc->str = 0;
    cprintf(ch, "Ok.\n\r");
  } else {			       /* there was no string. enter string mode */
    cprintf(ch, "Enter string. terminate with '@'.\n\r");
    *ch->desc->str = 0;
    ch->desc->max_str = length[field - 1];
  }
}

void bisect_arg(char *arg, int *field, char *string)
{
  char buf[MAX_INPUT_LENGTH];

  /* field name and number */
  arg = one_argument(arg, buf);
  if (!(*field = old_search_block(buf, 0, strlen(buf), room_fields, 0)))
    return;

  /* string */
  for (; isspace(*arg); arg++);
  for (; (*string = *arg); arg++, string++);

  return;
}

/*
 * Modification of character skills
 */

void do_setskill(struct char_data *ch, char *arg, int cmd)
{
  struct char_data *vict;
  char name[100], num[100], buf[100], help[MAX_STRING_LENGTH];
  int skill, field, value, i;
  static char *skills[] =
  {
    "search", "frighten", "telepath", "detect-evil",
    "sense-life", "cure", "bless", "remove",
    "poison", "blind", "neutralize", "purify",
    "hide", "cover", "backstab", "detect-invisible",
    "detect-magic", "enchant", "teleport", "create",
    "sanctuary", "resist", "drain", "turn",
    "protect", "light", "charm", "floating",
    "lightning-bolt", "sleep", "wake", "paralysis",
    "recharge", "shield", "fireball", "knock",
    "ventricolism", "double", "invisible", "death-ray",
    "bash", "dodge", "kick", "uppercut",
    "defend", "dirk", "listen", "missile", "detect", "\n"
  };

  cprintf(ch, "This routine is disabled untill it fitts\n\r");
  cprintf(ch, "The new structures (sorry Quinn) ....Bombman\n\r");
  return;

  arg = one_argument(arg, name);
  if (!*name) {			       /* no arguments. print an informative text */
    cprintf(ch, "Syntax:\n\rsetskill <name> <skill> <field> <value>\n\r");
    strcpy(help, "Skill being one of the following:\n\r\n\r");
    for (i = 1; *skills[i] != '\n'; i++) {
      sprintf(help + strlen(help), "%18s", skills[i]);
      if (!(i % 4)) {
	strcat(help, "\n\r");
	cprintf(ch, help);
	*help = '\0';
      }
    }
    if (*help)
      cprintf(ch, help);
    return;
  }
  if (!(vict = get_char_vis(ch, name))) {
    cprintf(ch, "No living thing by that name.\n\r");
    return;
  }
  arg = one_argument(arg, buf);
  if (!*buf) {
    cprintf(ch, "Skill name expected.\n\r");
    return;
  }
  if ((skill = old_search_block(buf, 0, strlen(buf), skills, 1)) < 0) {
    cprintf(ch, "No such skill is known. Try 'setskill' for list.\n\r");
    return;
  }
  argument_interpreter(arg, buf, num);
  if (!*num || !*buf) {
    cprintf(ch, "Field name or value undefined.\n\r");
    return;
  }
  if ((field = old_search_block(buf, 0, strlen(buf), skill_fields, 0)) < 0) {
    cprintf(ch, "Unrecognized field.\n\r");
    return;
  }
  value = atoi(num);
  if (field == 3) {
    if (value < -1) {
      cprintf(ch, "Minimum value for that is -1.\n\r");
      return;
    }
  } else if (value < 0) {
    cprintf(ch, "Minimum value for that is 0.\n\r");
    return;
  }
  if (value > max_value[field - 1]) {
    cprintf(ch, "Max value for that is %d.\n\r", max_value[field - 1]);
    return;
  }
  switch (field) {
  case 1:
    vict->skills[skill].learned = value;
    break;
    /* case 2: vict->skills[skill].affected_by = value; break; */
    /* case 3: vict->skills[skill].duration = value; break;    */
  case 4:
    vict->skills[skill].recognise = value;
    break;
  }

  cprintf(ch, "Ok.\n\r");
}

/* db stuff *********************************************** */

/* One_Word is like one_argument, execpt that words in quotes "" are */
/* regarded as ONE word                                              */

char *one_word(char *argument, char *first_arg)
{
  int found, begin, look_at;

  found = begin = 0;

  do {
    for (; isspace(*(argument + begin)); begin++);

    if (*(argument + begin) == '\"') { /* is it a quote */

      begin++;

      for (look_at = 0; (*(argument + begin + look_at) >= ' ') &&
	   (*(argument + begin + look_at) != '\"'); look_at++)
	*(first_arg + look_at) = LOWER(*(argument + begin + look_at));

      if (*(argument + begin + look_at) == '\"')
	begin++;

    } else {

      for (look_at = 0; *(argument + begin + look_at) > ' '; look_at++)
	*(first_arg + look_at) = LOWER(*(argument + begin + look_at));

    }

    *(first_arg + look_at) = '\0';
    begin += look_at;
  }
  while (fill_word(first_arg));

  return (argument + begin);
}

struct help_index_element *build_help_index(FILE * fl, int *num)
{
  int nr = -1, issorted, i;
  struct help_index_element *list = 0, mem;
  char buf[81], tmp[81], *scan;
  long pos;

  for (;;) {
    pos = ftell(fl);
    fgets(buf, 81, fl);
    *(buf + strlen(buf) - 1) = '\0';
    scan = buf;
    for (;;) {
      /* extract the keywords */
      scan = one_word(scan, tmp);

      if (!*tmp)
	break;

      if (!list) {
	CREATE(list, struct help_index_element, 1);

	nr = 0;
      } else
	RECREATE(list, struct help_index_element, ++nr + 1);

      list[nr].pos = pos;
      CREATE(list[nr].keyword, char, strlen(tmp) + 1);

      strcpy(list[nr].keyword, tmp);
    }
    /* skip the text */
    do
      fgets(buf, 81, fl);
    while (*buf != '#');
    if (*(buf + 1) == '~')
      break;
  }
  /* we might as well sort the stuff */
  do {
    issorted = 1;
    for (i = 0; i < nr; i++)
      if (str_cmp(list[i].keyword, list[i + 1].keyword) > 0) {
	mem = list[i];
	list[i] = list[i + 1];
	list[i + 1] = mem;
	issorted = 0;
      }
  }
  while (!issorted);

  *num = nr;
  return (list);
}

void page_string(struct descriptor_data *d, char *str, int keep_internal)
{
  if (!d)
    return;

  if (keep_internal) {
    CREATE(d->showstr_head, char, strlen(str) + 1);

    strcpy(d->showstr_head, str);
    d->showstr_point = d->showstr_head;
  } else
    d->showstr_point = str;

  show_string(d, "");
}

void show_string(struct descriptor_data *d, char *input)
{
  char buffer[MAX_STRING_LENGTH], buf[MAX_INPUT_LENGTH];
  register char *scan, *chk;
  int lines = 0, toggle = 1;

  one_argument(input, buf);

  if (*buf) {
    if (d->showstr_head) {
      DESTROY(d->showstr_head);
      d->showstr_head = 0;
    }
    d->showstr_point = 0;
    return;
  }
  /* show a chunk */
  for (scan = buffer;; scan++, d->showstr_point++)
    if ((((*scan = *d->showstr_point) == '\n') || (*scan == '\r')) &&
	((toggle = -toggle) < 0))
      lines++;
    else if (!*scan || ((lines >= 22) && IS_SET(d->character->specials.act, PLR_PAGER))) {
      *scan = '\0';
      SEND_TO_Q(buffer, d);

      /* see if this is the end (or near the end) of the string */
      for (chk = d->showstr_point; isspace(*chk); chk++);
      if (!*chk) {
	if (d->showstr_head) {
	  DESTROY(d->showstr_head);
	  d->showstr_head = 0;
	}
	d->showstr_point = 0;
      }
      return;
    }
}

void check_reboot(void) {
  long tc;
  struct tm *t_info;
  char dummy;
  FILE *boot;
  char buf[512];
  char *tmstr;

  tc = time(0);
  t_info = localtime(&tc);
  tmstr= asctime(t_info);
  *(tmstr + strlen(tmstr) -1) = '\0';

  if ((((t_info->tm_hour + 1) == REBOOT_AT1) ||
       ((t_info->tm_hour + 1) == REBOOT_AT2)) &&
      (t_info->tm_min > 30))
    if ((boot = fopen(REBOOT_FILE, "r"))) {
      if (t_info->tm_min > 55) {
	log("**** Reboot exists ****");
	fread(&dummy, sizeof(dummy), 1, boot);
	if (!feof(boot)) {	       /* the file is nonepty */
	  log("Reboot is nonempty.");
	  if (system(REBOOT_FILE)) {
	    log("Reboot script terminated abnormally");
	    aprintf("The reboot was cancelled.\n\r");
	    sprintf(buf, "mv %s %s.FAILED", REBOOT_FILE, REBOOT_FILE);
	    system(buf);
	    fclose(boot);
	    return;
	  } else {
	    sprintf(buf, "mv %s %s.OK", REBOOT_FILE, REBOOT_FILE);
	    system(buf);
	  }
	}
	sprintf(buf, "touch %s", REBOOT_FILE);
	system(buf);
        aprintf(
          "\x007\n\rBroadcast message from Quixadhal (tty0) %s...\n\r\n\r",
          tmstr);
        aprintf("Automatic reboot.  Come back in a few minutes!\n\r");
        aprintf("\x007The system is going down NOW !!\n\r\x007\n\r");
	diku_shutdown = diku_reboot = 1;
      } else if (t_info->tm_min > 40) {
        aprintf(
          "\x007\n\rBroadcast message from Quixadhal (tty0) %s...\n\r\n\r",
          tmstr);
        aprintf("Automatic reboot.  Game is now Whizz-Locked!\n\r");
        aprintf("\x007The system is going DOWN in %d minutes !!\n\r\x007\n\r",
                55 - t_info->tm_min);
        WizLock = 1;
      }
      fclose(boot);
    }
}

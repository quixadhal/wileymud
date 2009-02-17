/*
 * file: modify.c                                         Part of DIKUMUD
 * Usage: Run-time modification (by users) of game variables
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */

#include <stdio.h>
#include <stdlib.h>
/* #include <unistd.h> */
#include <sys/types.h>
#include <signal.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#include "global.h"
#include "bug.h"
#include "utils.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "comm.h"
#include "multiclass.h"
#define _MODIFY_C
#include "modify.h"

int                                     REBOOT_AT1 = 0;
int                                     REBOOT_AT2 = 0;	       /* 0-23, time of optional reboot if -e lib/reboot */
struct room_data                       *world = NULL;		       /* dyn alloc'ed array of rooms */
const char                                   *string_fields[] = {
  "name",
  "short",
  "long",
  "description",
  "title",
  "delete-description",
  "\n"
};

const char                                   *room_fields[] = {
  "name",						       /* 1 */
  "desc",						       /* 2 */
  "flags",						       /* 3 */
  "exit",						       /* 4 */
  "xdesc",						       /* 5 */
  "extra",						       /* 6 */
  "rivr",						       /* 7 */
  "tele",						       /* 8 */
  "\n"
};

/* maximum length for text field x+1 */
int                                     length[] = {
  15,
  60,
  256,
  240,
  60
};

int                                     room_length[] = {
  80,
  1024,
  50,
  50,
  512,
  512,
  50,
  100
};

const char                                   *skill_fields[] = {
  "learned",
  "affected",
  "duration",
  "recognize",
  "\n"
};

int                                     max_value[] = {
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
  char                                   *scan = NULL;
  int                                     terminator = 0;

  if (DEBUG > 2)
    log_info("called %s with %08zx, %s", __PRETTY_FUNCTION__, (size_t)d, VNULL(str));

  /*
   * determine if this is the terminal string, and truncate if so 
   */
  for (scan = str; *scan; scan++)
    if ((terminator = (*scan == '@'))) {
      *scan = '\0';
      break;
    }
  if (!(*d->str)) {
    if (strlen(str) > d->max_str) {
      cprintf(d->character, "String too long - Truncated.\r\n");
      *(str + d->max_str) = '\0';
      terminator = 1;
    }
    CREATE(*d->str, char, strlen            (str) + 3);

    strcpy(*d->str, str);
  } else {
    if (strlen(str) + strlen(*d->str) > d->max_str) {
      cprintf(d->character, "String too long. Last line skipped.\r\n");
      terminator = 1;
    } else {
      RECREATE(*d->str, char, strlen          (*d->str) + strlen(str) + 3);

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
    strcat(*d->str, "\r\n");
}

/* interpret an argument for do_string */
void quad_arg(const char *arg, int *type, char *name, int *field, char *string)
{
  char                                    buf[MAX_STRING_LENGTH] = "\0\0\0";

  if (DEBUG > 2)
    log_info("called %s with %s, %08zx, %s, %08zx, %s", __PRETTY_FUNCTION__, VNULL(arg), (size_t)type, VNULL(name), (size_t)field, VNULL(string));

  /*
   * determine type 
   */
  arg = one_argument(arg, buf);
  if (is_abbrev(buf, "char"))
    *type = TP_MOB;
  else if (is_abbrev(buf, "obj"))
    *type = TP_OBJ;
  else {
    *type = TP_ERROR;
    return;
  }

  /*
   * find name 
   */
  arg = one_argument(arg, name);

  /*
   * field name and number 
   */
  arg = one_argument(arg, buf);
  if (!(*field = old_search_block(buf, 0, strlen(buf), string_fields, 0)))
    return;
  /*
   * string 
   */
  for (; isspace(*arg); arg++);
  for (; (*string = *arg); arg++, string++);
  return;
}

/* modification of malloc'ed strings in chars/objects */
void do_string(struct char_data *ch, const char *arg, int cmd)
{

  char                                    name[MAX_STRING_LENGTH] = "\0\0\0";
  char                                    string[MAX_STRING_LENGTH] = "\0\0\0";
  struct extra_descr_data                *ed = NULL;
  struct extra_descr_data                *tmp = NULL;
  int                                     field = 0;
  int                                     type = 0;
  struct char_data                       *mob = NULL;
  struct obj_data                        *obj = NULL;

  if (DEBUG > 2)
    log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(arg), cmd);

  if (IS_NPC(ch))
    return;

  quad_arg(arg, &type, name, &field, string);

  if (type == TP_ERROR) {
    cprintf(ch, "Syntax:\r\nstring (char) <name> <field> [<string>].");
    return;
  }
  if (!field) {
    cprintf(ch, "No field by that name. Try 'help string'.\r\n");
    return;
  }
  if (type == TP_MOB) {
    /*
     * locate the beast 
     */
    if (!(mob = get_char_vis(ch, name))) {
      cprintf(ch, "I don't know anyone by that name...\r\n");
      return;
    }
    switch (field) {
      case 1:
	if (!IS_NPC(mob) && GetMaxLevel(ch) < IMPLEMENTOR) {
	  cprintf(ch, "You can't change that field for players.");
	  return;
	}
	if (!*string) {
	  cprintf(ch, "You have to supply a name!\r\n");
	  return;
	}
	ch->desc->str = &mob->player.name;
	if (!IS_NPC(mob))
	  cprintf(ch, "WARNING: You have changed the name of a player.\r\n");
	break;
      case 2:
	if (!IS_NPC(mob)) {
	  cprintf(ch, "That field is for monsters only.\r\n");
	  return;
	}
	ch->desc->str = &mob->player.short_descr;
	break;
      case 3:
	if (!IS_NPC(mob)) {
	  cprintf(ch, "That field is for monsters only.\r\n");
	  return;
	}
	ch->desc->str = &mob->player.long_descr;
	break;
      case 4:
	ch->desc->str = &mob->player.description;
	break;
      case 5:
	if (IS_NPC(mob)) {
	  cprintf(ch, "Monsters have no titles.\r\n");
	  return;
	}
	if ((GetMaxLevel(ch) > GetMaxLevel(mob)) && (ch != mob))
	  ch->desc->str = &mob->player.title;
	else {
	  cprintf(ch, "Sorry, can't set the title of someone of highter level.\r\n");
	  return;
	}
	break;
      default:
	cprintf(ch, "That field is undefined for monsters.\r\n");
	return;
	break;
    }
  } else {
    cprintf(ch, "Stringing of objects is no longer allowed for now.\r\n");
    return;

    /*
     * type == TP_OBJ 
     */
    /*
     * locate the object 
     */
    if (!(obj = get_obj_vis(ch, name))) {
      cprintf(ch, "Can't find such a thing here..\r\n");
      return;
    }
    switch (field) {

      case 1:
	if (!*string) {
	  cprintf(ch, "You have to supply a keyword.\r\n");
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
	  cprintf(ch, "You have to supply a keyword.\r\n");
	  return;
	}
	/*
	 * try to locate extra description 
	 */
	for (ed = obj->ex_description;; ed = ed->next)
	  if (!ed) {
	    CREATE(ed, struct extra_descr_data, 1);

	    ed->next = obj->ex_description;
	    obj->ex_description = ed;
	    CREATE(ed->keyword, char, strlen        (string) + 1);

	    strcpy(ed->keyword, string);
	    ed->description = 0;
	    ch->desc->str = &ed->description;
	    cprintf(ch, "New field.\r\n");
	    break;
	  } else if (!str_cmp(ed->keyword, string)) {	       /* the field exists */
	    DESTROY(ed->description);
	    ed->description = 0;
	    ch->desc->str = &ed->description;
	    cprintf(ch, "Modifying description.\r\n");
	    break;
	  }
	ch->desc->max_str = MAX_STRING_LENGTH;
	return;						       /* the stndrd (see below) procedure does not apply here */
	break;
      case 6:						       /* deletion */
	if (!*string) {
	  cprintf(ch, "You must supply a field name.\r\n");
	  return;
	}
	/*
	 * try to locate field 
	 */
	for (ed = obj->ex_description;; ed = ed->next)
	  if (!ed) {
	    cprintf(ch, "No field with that keyword.\r\n");
	    return;
	  } else if (!str_cmp(ed->keyword, string)) {
	    DESTROY(ed->keyword);
	    if (ed->description)
	      DESTROY(ed->description);

	    /*
	     * delete the entry in the desr list 
	     */
	    if (ed == obj->ex_description)
	      obj->ex_description = ed->next;
	    else {
	      for (tmp = obj->ex_description; tmp->next != ed; tmp = tmp->next);
	      tmp->next = ed->next;
	    }
	    DESTROY(ed);

	    cprintf(ch, "Field deleted.\r\n");
	    return;
	  }
	break;
      default:
	cprintf(ch, "That field is undefined for objects.\r\n");
	return;
	break;
    }
  }

  if (*ch->desc->str) {
    DESTROY(*ch->desc->str);
  }
  if (*string) {					       /* there was a string in the argument array */
    if (strlen(string) > length[field - 1]) {
      cprintf(ch, "String too long - truncated.\r\n");
      *(string + length[field - 1]) = '\0';
    }
    CREATE(*ch->desc->str, char, strlen     (string) + 1);

    strcpy(*ch->desc->str, string);
    ch->desc->str = 0;
    cprintf(ch, "Ok.\r\n");
  } else {						       /* there was no string. enter string mode */
    cprintf(ch, "Enter string. terminate with '@'.\r\n");
    *ch->desc->str = 0;
    ch->desc->max_str = length[field - 1];
  }
}

void bisect_arg(const char *arg, int *field, char *string)
{
  char                                    buf[MAX_INPUT_LENGTH] = "\0\0\0";

  if (DEBUG > 2)
    log_info("called %s with %s, %08zx, %s", __PRETTY_FUNCTION__, VNULL(arg), (size_t)field, VNULL(string));

  /*
   * field name and number 
   */
  arg = one_argument(arg, buf);
  if (!(*field = old_search_block(buf, 0, strlen(buf), room_fields, 0)))
    return;

  /*
   * string 
   */
  for (; isspace(*arg); arg++);
  for (; (*string = *arg); arg++, string++);

  return;
}

/*
 * Modification of character skills
 */

void do_setskill(struct char_data *ch, const char *arg, int cmd)
{
  struct char_data                       *vict = NULL;
  char                                    name[100] = "\0\0\0";
  char                                    num[100] = "\0\0\0";
  char                                    buf[100] = "\0\0\0";
  char                                    helpstr[MAX_STRING_LENGTH] = "\0\0\0";
  int                                     skill = 0;
  int                                     field = 0;
  int                                     value = 0;
  int                                     i = 0;
  static const char                      *skills[] = {
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

  if (DEBUG)
    log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(arg), cmd);

  cprintf(ch, "This routine is disabled untill it fitts\r\n");
  cprintf(ch, "The new structures (sorry Quinn) ....Bombman\r\n");
  return;

  arg = one_argument(arg, name);
  if (!*name) {						       /* no arguments. print an informative text */
    cprintf(ch, "Syntax:\r\nsetskill <name> <skill> <field> <value>\r\n");
    strcpy(helpstr, "Skill being one of the following:\r\n\r\n");
    for (i = 1; *skills[i] != '\n'; i++) {
      sprintf(helpstr + strlen(helpstr), "%18s", skills[i]);
      if (!(i % 4)) {
	strcat(helpstr, "\r\n");
	cprintf(ch, "%s", helpstr);
	*helpstr = '\0';
      }
    }
    if (*helpstr)
      cprintf(ch, "%s", helpstr);
    return;
  }
  if (!(vict = get_char_vis(ch, name))) {
    cprintf(ch, "No living thing by that name.\r\n");
    return;
  }
  arg = one_argument(arg, buf);
  if (!*buf) {
    cprintf(ch, "Skill name expected.\r\n");
    return;
  }
  if ((skill = old_search_block(buf, 0, strlen(buf), skills, 1)) < 0) {
    cprintf(ch, "No such skill is known. Try 'setskill' for list.\r\n");
    return;
  }
  argument_interpreter(arg, buf, num);
  if (!*num || !*buf) {
    cprintf(ch, "Field name or value undefined.\r\n");
    return;
  }
  if ((field = old_search_block(buf, 0, strlen(buf), skill_fields, 0)) < 0) {
    cprintf(ch, "Unrecognized field.\r\n");
    return;
  }
  value = atoi(num);
  if (field == 3) {
    if (value < -1) {
      cprintf(ch, "Minimum value for that is -1.\r\n");
      return;
    }
  } else if (value < 0) {
    cprintf(ch, "Minimum value for that is 0.\r\n");
    return;
  }
  if (value > max_value[field - 1]) {
    cprintf(ch, "Max value for that is %d.\r\n", max_value[field - 1]);
    return;
  }
  switch (field) {
    case 1:
      vict->skills[skill].learned = value;
      break;
      /*
       * case 2: vict->skills[skill].affected_by = value; break; 
       */
      /*
       * case 3: vict->skills[skill].duration = value; break; 
       */
    case 4:
      vict->skills[skill].recognise = value;
      break;
  }

  cprintf(ch, "Ok.\r\n");
}

/* db stuff *********************************************** */

/* One_Word is like one_argument, execpt that words in quotes "" are */
/* regarded as ONE word                                              */

char                                   *one_word(char *argument, char *first_arg)
{
  int                                     begin = 0;
  int                                     look_at = 0;

  if (DEBUG > 2)
    log_info("called %s with %s, %s", __PRETTY_FUNCTION__, VNULL(argument), VNULL(first_arg));

  do {
    for (; isspace(*(argument + begin)); begin++);

    if (*(argument + begin) == '\"') {			       /* is it a quote */

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

struct help_index_element              *build_help_index(FILE * fl, int *num)
{
  int                                     nr = -1;
  int                                     issorted = 0;
  int                                     i = 0;
  struct help_index_element              *list = NULL;
  struct help_index_element               mem;
  char                                    buf[81] = "\0\0\0";
  char                                    tmp[81] = "\0\0\0";
  char                                   *scan = NULL;
  long                                    pos = 0L;

  if (DEBUG > 2)
    log_info("called %s with %08zx, %08zx", __PRETTY_FUNCTION__, (size_t)fl, (size_t)num);

  for (;;) {
    pos = ftell(fl);
    fgets(buf, 81, fl);
    *(buf + strlen(buf) - 1) = '\0';
    scan = buf;
    for (;;) {
      /*
       * extract the keywords 
       */
      scan = one_word(scan, tmp);

      if (!*tmp)
	break;

      if (!list) {
	CREATE(list, struct help_index_element, 1);

	nr = 0;
      } else
	RECREATE(list, struct help_index_element, ++nr + 1);

      list[nr].pos = pos;
      CREATE(list[nr].keyword, char, strlen   (tmp) + 1);

      strcpy(list[nr].keyword, tmp);
    }
    /*
     * skip the text 
     */
    do
      fgets(buf, 81, fl);
    while (*buf != '#');
    if (*(buf + 1) == '~')
      break;
  }
  /*
   * we might as well sort the stuff 
   */
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
  static char Empty[1] = "";

  if (DEBUG > 2)
    log_info("called %s with %08zx, %s, %d", __PRETTY_FUNCTION__, (size_t)d, VNULL(str), keep_internal);

  if (!d)
    return;

  if (keep_internal) {
    CREATE(d->showstr_head, char, strlen    (str) + 1);

    strcpy(d->showstr_head, str);
    d->showstr_point = d->showstr_head;
  } else
    d->showstr_point = str;

  show_string(d, Empty);
}

void show_string(struct descriptor_data *d, char *input)
{
  char                                    buffer[MAX_STRING_LENGTH] = "\0\0\0";
  char                                    buf[MAX_INPUT_LENGTH] = "\0\0\0";
  char                                   *scan = NULL;
  char                                   *chk = NULL;
  int                                     lines = 0;
  int                                     toggle = 1;

  if (DEBUG > 2)
    log_info("called %s with %08zx, %s", __PRETTY_FUNCTION__, (size_t)d, VNULL(input));

  one_argument(input, buf);

  if (*buf) {
    if (d->showstr_head) {
      DESTROY(d->showstr_head);
      d->showstr_head = 0;
    }
    d->showstr_point = 0;
    return;
  }
  /*
   * show a chunk 
   */
  for (scan = buffer;; scan++, d->showstr_point++)
    if ((((*scan = *d->showstr_point) == '\n') || (*scan == '\r')) && ((toggle = -toggle) < 0))
      lines++;
    else if (!*scan || ((lines >= 22) && IS_SET(d->character->specials.act, PLR_PAGER))) {
      *scan = '\0';
      SEND_TO_Q(buffer, d);

      /*
       * see if this is the end (or near the end) of the string 
       */
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

void check_reboot(void)
{
  time_t                                  tc;
  struct tm                              *t_info = NULL;
  char                                    dummy = '\0';
  FILE                                   *boot = NULL;
  char                                    buf[512] = "\0\0\0";
  char                                   *tmstr = NULL;

  if (DEBUG > 2)
    log_info("called %s with no arguments", __PRETTY_FUNCTION__);

  tc = time(0);
  t_info = localtime(&tc);
  tmstr = asctime(t_info);
  *(tmstr + strlen(tmstr) - 1) = '\0';

  if ((((t_info->tm_hour + 1) == REBOOT_AT1) ||
       ((t_info->tm_hour + 1) == REBOOT_AT2)) && (t_info->tm_min > 30))
    if ((boot = fopen(REBOOT_FILE, "r"))) {
      if (t_info->tm_min > 55) {
	log_info("**** Reboot exists ****");
	fread(&dummy, sizeof(dummy), 1, boot);
	if (!feof(boot)) {				       /* the file is nonepty */
	  log_info("Reboot is nonempty.");
	  if (system(REBOOT_FILE)) {
	    log_info("Reboot script terminated abnormally");
	    allprintf("The reboot was cancelled.\r\n");
	    sprintf(buf, "mv %s %s.FAILED", REBOOT_FILE, REBOOT_FILE);
	    system(buf);
	    FCLOSE(boot);
	    return;
	  } else {
	    sprintf(buf, "mv %s %s.OK", REBOOT_FILE, REBOOT_FILE);
	    system(buf);
	  }
	}
	sprintf(buf, "touch %s", REBOOT_FILE);
	system(buf);
	allprintf("\x007\r\nBroadcast message from Quixadhal (tty0) %s...\r\n\r\n", tmstr);
	allprintf("Automatic reboot.  Come back in a few minutes!\r\n");
	allprintf("\x007The system is going down NOW !!\r\n\x007\r\n");
	diku_shutdown = diku_reboot = 1;
      } else if (t_info->tm_min > 40) {
	allprintf("\x007\r\nBroadcast message from Quixadhal (tty0) %s...\r\n\r\n", tmstr);
	allprintf("Automatic reboot.  Game is now Whizz-Locked!\r\n");
	allprintf("\x007The system is going DOWN in %d minutes !!\r\n\x007\r\n",
		55 - t_info->tm_min);
	WizLock = 1;
      }
      FCLOSE(boot);
    }
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <sys/timeb.h>

#undef DIKU_CRUD
#ifdef DIKU_CRUD
#include "include/global.h"
#include "include/utils.h"
#include "include/comm.h"
#include "include/multiclass.h"
#endif

#define _BUG_C
#include "include/bug.h"

#ifdef DIKU_CRUD
extern struct descriptor_data *descriptor_list;
#else
#define MAX_STRING_LENGTH 2048
#endif

/*
 * This is my general purpose error handler that spews a time-stamped
 * message to stderr and a logfile.
 * The messages are in this format:
 * <: DATE::(filename,func,line) User[#room]:\n : message\n
 *
 * If File, Func, or Line are NULL they will be ommitted.
 * If BufFile is NULL, stderr will be used alone.
 * If ch is NULL, User will be left blank.
 * If Str is NULL, we are just making a PING!
 *
 * NOTE!  The calling interface is very ugly... it is designed to be very
 * versitle, not pretty... If you want it to be useful in your source code,
 * use a macro like this one:
 * #define log_error(BugFile, ch, Str...) \
 *         bug_logger(__FILE__, __FUNCTION__, __LINE__, BugFile, ch, Str, ## args)
 * which can then be used by simply saying:
 * log_error(BUGLOG, ch, "You died %d times!\n", deaths);
 * producing as an example:
 * <: 950219.195642.037 (ack.c;barf,135) Quixadhal [#3001]:
 *  : You died 27 times!
 * The datestamp is YYMMDD.HHMMSS.MIL format.
 */
void bug_logger(const char *File, const char *Func, int Line,
#ifdef DIKU_CRUD
          unsigned int Level,
#endif
          unsigned int Type,
	  const char *BugFile,
#ifdef DIKU_CRUD
          struct char_data *ch,
#endif
          const char *Str,...)
{
  va_list arg;
  char Result[MAX_STRING_LENGTH];
  char Temp[MAX_STRING_LENGTH];
  FILE *fp;
  struct timeb right_now;
  struct tm *now_part;

  bzero(Result, MAX_STRING_LENGTH);
  va_start(arg, Str);
  if (Str && *Str) {
#ifdef DIKU_CRUD
    struct descriptor_data *i;

    strcpy(Result, "Notify> ");
#endif
    vsprintf(Temp, Str, arg);
#ifdef DIKU_CRUD
    strcat(Result, Temp);
    for (i = descriptor_list; i; i = i->next)
      if ((!i->connected) && (GetMaxLevel(i->character) >= Level) &&
	  (IS_SET(i->character->specials.act, PLR_LOGS)))
	write_to_q(Result, &i->output);
    bzero(Result, MAX_STRING_LENGTH);
#endif
  } else
    strcpy(Temp, "PING!");
  va_end(arg);
  ftime(&right_now);
  now_part= localtime((const time_t *)&right_now);
  sprintf(Result, "<: %02d%02d%02d.%02d%02d%02d.%03d",
          now_part->tm_year, now_part->tm_mon+1, now_part->tm_mday,
          now_part->tm_hour, now_part->tm_min, now_part->tm_sec,
          right_now.millitm);
  if (File || Func || Line) {
    strcat(Result, " (");
    if (File && *File) {
      strcat(Result, File);
    }
    if (Func && *Func)
      sprintf(Result + strlen(Result), ";%s", Func);
    if (Line)
      sprintf(Result + strlen(Result), ",%d)", Line);
    else
      strcat(Result, ")");
  }
#ifdef DIKU_CRUD
  if (ch && !IS_NPC(ch))
    sprintf(Result + strlen(Result), " %s [#%d]\n",
	    ch->player.name, ch->in_room ? ch->in_room : 0);
  else
#endif
    if (File || Func || Line)
      strcat(Result, "\n");

  strcat(Result, " : ");
  strcat(Result, Temp);

  if (BugFile && *BugFile) {
    if (!(fp = fopen(BugFile, "a"))) {
      perror(BugFile);
#ifdef DIKU_CRUD
      if (ch)
	send_to_char("Could not open the file!\n\r", ch);
#endif
    } else {
      fprintf(fp, "%s\n", Result);
      fclose(fp);
    }
  }
  fprintf(stderr, "%s\n", Result);
}

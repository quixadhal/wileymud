#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <sys/timeb.h>

#include "global.h"
#include "utils.h"
#include "comm.h"
#include "multiclass.h"
#define _BUG_C
#include "bug.h"

extern struct descriptor_data *descriptor_list;

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
 * #define bug(BugFile, ch, Str...) \
 *         abug(__FILE__, __FUNCTION__, __LINE__, BugFile, ch, Str, ## args)
 * which can then be used by simply saying:
 * bug(BUGLOG, ch, "You died %d times!\n", deaths);
 * producing as an example:
 * <: Sun Feb 19 19:56:42 EST 1995 (ack.c;barf,135) Quixadhal [#3001]:
 *  : You died 27 times!
 */
void abug(char *File, char *Func, int Line, UINT Level, UINT Type,
	  char *BugFile, struct char_data *ch, char *Str,...)
{
  va_list arg;
  char Result[MAX_STRING_LENGTH];
  char Temp[MAX_STRING_LENGTH];
#if 0
  char *Time;
#endif
  char Time[256];
  FILE *fp;
  long current_time;
  struct timeb right_now;
  struct tm *now_part;

  bzero(Result, MAX_STRING_LENGTH);
  va_start(arg, Str);
  if (Str && *Str) {
    struct descriptor_data *i;

    strcpy(Result, "Notify> ");
    vsprintf(Temp, Str, arg);
    strcat(Result, Temp);
    /* NOTIFY(Result, Level, Type); */
    for (i = descriptor_list; i; i = i->next)
      if ((!i->connected) && (GetMaxLevel(i->character) >= 57) &&
	  (IS_SET(i->character->specials.act, PLR_LOGS)))
	write_to_q(Result, &i->output);
    bzero(Result, MAX_STRING_LENGTH);
  } else
    strcpy(Temp, "PING!");
  va_end(arg);
  ftime(&right_now);
  now_part= localtime(&right_now);
  sprintf(Time, "%02d%02d%02d.%02d%02d%02d.%03d",
          now_part->tm_year, now_part->tm_mon, now_part->tm_mday,
          now_part->tm_hour, now_part->tm_min, now_part->tm_sec,
          right_now.millitm);
#if 0
  current_time = time(NULL);
  Time = ctime(&current_time);
  Time[strlen(Time) - 1] = '\0';
#endif
  sprintf(Result, "<: %s", Time);
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
  if (ch && !IS_NPC(ch))
    sprintf(Result + strlen(Result), " %s [#%d]\n",
	    ch->player.name, ch->in_room ? ch->in_room : 0);
  else if (File || Func || Line)
    strcat(Result, "\n");

  strcat(Result, " : ");
  strcat(Result, Temp);

  if (BugFile && *BugFile) {
    if (!(fp = fopen(BugFile, "a"))) {
      perror(BugFile);
      if (ch)
	send_to_char("Could not open the file!\n\r", ch);
    } else {
      fprintf(fp, "%s\n", Result);
      fclose(fp);
    }
  }
  fprintf(stderr, "%s\n", Result);
}

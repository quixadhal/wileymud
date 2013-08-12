#include <stdio.h>
#include <stdlib.h>
/* #include <unistd.h> */
#include <sys/types.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <sys/timeb.h>

#include "global.h"
#include "utils.h"
#include "comm.h"
#include "multiclass.h"
#include "db.h"

#define _BUG_C
#include "bug.h"

const char * LogNames[] = {
"INFO",
"ERROR",
"FATAL",
"BOOT",
"AUTH",
"KILL",
"DEATH",
"RESET",
"IMC"
};

/* Handy query for checking logs:
 *
 * select to_char(date_trunc('second', log_date), 'HH24:MI:SS'), log_types.name, log_entry
 * from logfile join log_types using (log_type_id)
 * where log_date > now() - interval '1 day'
 * order by log_date desc
 * limit 20;     
 */

/*
 * Things we want to have when logging events.....
 *
 * BugFile is the filename you want to log to, NULL means stderr
 *
 * File, Func, Line can all be provided by the compiler as
 * __FILE__, __PRETTY_FUNCTION__, and __LINE__
 *
 * Level is the minimum character level which will see the
 * bug if they're logged in.
 *
 * The AreaFile and AreaLine are the file and line number
 * we were reading while booting the world database.
 *
 * Type is the type of error, typically things like
 * LOG_INFO, LOG_ERROR, LOG_FATAL, LOG_BOOT, LOG_AUTH
 *
 * ch is the char_data pointer for the player/mob
 * obj is an obj_data pointer, if you have one
 * room is.... the room_data pointer.
 *
 * Str is, of course, the message, and it gets printed
 * using varargs, so you can have this be a printf type
 * set of macros.
 */
void bug_logger( unsigned int Type, const char *BugFile,
                 const char *File, const char *Func, int Line,
                 const char *AreaFile, int AreaLine,
	         struct char_data *ch, struct char_data *victim,
                 unsigned int Level, const char *Str, ... )
{
  va_list                                 arg;
  char                                    Result[MAX_STRING_LENGTH] = "\0\0\0";
  char                                    Temp[MAX_STRING_LENGTH] = "\0\0\0";
  FILE                                   *fp = NULL;
  struct timeb                            right_now;
  struct tm                              *now_part = NULL;

  bzero(Result, MAX_STRING_LENGTH);
  va_start(arg, Str);
  if (Str && *Str) {
    struct descriptor_data                 *i;

    sprintf(Result, "%s> ", LogNames[Type]);
    vsprintf(Temp, Str, arg);
    strcat(Result, Temp);
    for (i = descriptor_list; i; i = i->next)
      if ((!i->connected) && (GetMaxLevel(i->character) >= Level) &&
	  (IS_SET(i->character->specials.act, PLR_LOGS)))
	write_to_q(Result, &i->output, 1);
    bzero(Result, MAX_STRING_LENGTH);
  } else
    strcpy(Temp, "PING!");
  va_end(arg);
  ftime(&right_now);
  now_part = localtime((const time_t *)&right_now);
  sprintf(Result, "<: %02d%02d%02d.%02d%02d%02d.%03d",
	  now_part->tm_year, now_part->tm_mon + 1, now_part->tm_mday,
	  now_part->tm_hour, now_part->tm_min, now_part->tm_sec, right_now.millitm);
  sprintf(Result + strlen(Result), " - %s -", LogNames[Type]);
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
  if (ch || victim) {
    if (ch)
      sprintf(Result + strlen(Result), " ch \"%s\" [#%d]",
              NAME(ch), ch->in_room);
    if (victim)
      sprintf(Result + strlen(Result), " victim \"%s\" [#%d]",
              NAME(victim), victim->in_room);
/*
    if (obj)
      sprintf(Result + strlen(Result), " obj \"%s\" [#%d]",
              SAFE_ONAME(obj), obj->in_room);
    if (room)
      sprintf(Result + strlen(Result), " room \"%s\" [#%d]",
              room->name?room->name:"", room->number);
*/
    strcat(Result, "\n");
  } else
    if (File || Func || Line)
      strcat(Result, "\n");

  strcat(Result, " : ");
  strcat(Result, Temp);

  if (BugFile && *BugFile) {
    if (!(fp = fopen(BugFile, "a"))) {
      perror(BugFile);
      if (ch)
	send_to_char("Could not open the file!\r\n", ch);
    } else {
      fprintf(fp, "%s\n", Result);
      FCLOSE(fp);
    }
  }

  if(stderr) {
    fprintf(stderr, "%s\n", Result);
    fflush(stderr);
  }

}

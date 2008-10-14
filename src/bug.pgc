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
#include "sql.h"

#define _BUG_C
#include "bug.h"

char * LogNames[] = {
"INFO",
"ERROR",
"FATAL",
"BOOT",
"AUTH",
"KILL",
"DEATH"
};

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
	write_to_q(Result, &i->output);
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
	send_to_char("Could not open the file!\n\r", ch);
    } else {
      fprintf(fp, "%s\n", Result);
      FCLOSE(fp);
    }
  }

  if (db_connected) {
    char sql_string[MAX_STRING_LENGTH];
    char sql_values[MAX_STRING_LENGTH];

    sprintf(sql_string, "INSERT INTO logfile (log_type_id, log_entry");
    sprintf(sql_values, "VALUES (%d, '%s'", Type, escape_sql( Temp ));
    if (File || Func) {
      if (File && *File) {
        sprintf(sql_string + strlen(sql_string), ", %s", "log_file");
        sprintf(sql_values + strlen(sql_values), ", '%s'", escape_sql( File ));
      }
      if (Func && *Func) {
        sprintf(sql_string + strlen(sql_string), ", %s", "log_function");
        sprintf(sql_values + strlen(sql_values), ", '%s'", escape_sql( Func ));
      }
      sprintf(sql_string + strlen(sql_string), ", %s", "log_line");
      sprintf(sql_values + strlen(sql_values), ", %d", Line);
    }
    if (AreaFile) {
      if (AreaFile && *AreaFile) {
        sprintf(sql_string + strlen(sql_string), ", %s", "log_areafile");
        sprintf(sql_values + strlen(sql_values), ", '%s'", escape_sql( AreaFile ));
      }
      sprintf(sql_string + strlen(sql_string), ", %s", "log_arealine");
      sprintf(sql_values + strlen(sql_values), ", %d", AreaLine);
    }
    if (ch) {
      if (IS_MOB(ch)) {
        sprintf(sql_string + strlen(sql_string), ", %s", "log_npc_actor");
        sprintf(sql_values + strlen(sql_values), ", %d", mob_index[(int)ch->nr].virtual);
      } else {
        sprintf(sql_string + strlen(sql_string), ", %s", "log_pc_actor");
        sprintf(sql_values + strlen(sql_values), ", '%s'", escape_sql( NAME(ch) ) );
      }
      if(ch->in_room > -1) {
        sprintf(sql_string + strlen(sql_string), ", %s", "log_room");
        sprintf(sql_values + strlen(sql_values), ", %d", ch->in_room);
        sprintf(sql_string + strlen(sql_string), ", %s", "log_area");
        sprintf(sql_values + strlen(sql_values), ", %d", GET_ZONE(ch->in_room));
      }
    }
    if (victim) {
      if (IS_MOB(victim)) {
        sprintf(sql_string + strlen(sql_string), ", %s", "log_npc_victim");
        sprintf(sql_values + strlen(sql_values), ", %d", mob_index[(int)victim->nr].virtual);
      } else {
        sprintf(sql_string + strlen(sql_string), ", %s", "log_pc_victim");
        sprintf(sql_values + strlen(sql_values), ", '%s'", escape_sql( NAME(victim) ) );
      }
      if (!ch || ch->in_room < 0) {
        if(victim->in_room > -1) {
          sprintf(sql_string + strlen(sql_string), ", %s", "log_room");
          sprintf(sql_values + strlen(sql_values), ", %d", victim->in_room);
          sprintf(sql_string + strlen(sql_string), ", %s", "log_area");
          sprintf(sql_values + strlen(sql_values), ", %d", GET_ZONE(victim->in_room));
        }
      }
    }
/*
    if (obj) {
      sprintf(sql_string + strlen(sql_string), ", %s", "log_obj");
      sprintf(sql_values + strlen(sql_values), ", %d", obj_index[(int)obj->item_number].virtual);
      if (!room) {
        sprintf(sql_string + strlen(sql_string), ", %s", "log_room");
        sprintf(sql_values + strlen(sql_values), ", %d", obj->in_room);
      }
    }
    if (room) {
      sprintf(sql_string + strlen(sql_string), ", %s", "log_room");
      sprintf(sql_values + strlen(sql_values), ", %d", room->number);
      sprintf(sql_string + strlen(sql_string), ", %s", "log_area");
      sprintf(sql_values + strlen(sql_values), ", %d", room->zone);
    }
*/
    sprintf(sql_string + strlen(sql_string), ") %s)", sql_values);
    /* if(stderr) fprintf(stderr, "%s\n", sql_string); */
    execute_sql( sql_string );
  }
  
  if(stderr) {
    fprintf(stderr, "%s\n", Result);
    fflush(stderr);
  }
}

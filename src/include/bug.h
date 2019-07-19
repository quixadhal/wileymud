#ifndef _BUG_H
#define _BUG_H

/* These should be sync'd to the SQL schema for sanity's sake */
#define LOG_INFO	0
#define LOG_ERROR	1
#define LOG_FATAL	2
#define LOG_BOOT	3
#define	LOG_AUTH	4
#define	LOG_KILL	5
#define	LOG_DEATH	6
#define	LOG_RESET	7
#define	LOG_IMC         8
#define	LOG_SQL         9
#define	LOG_NOSQL       10

#ifndef _BUG_C
extern char * LogNames[];
#endif

#define log_info(Str, ...) \
	bug_logger(LOG_INFO, NULL, \
                   __FILE__, __PRETTY_FUNCTION__, __LINE__, \
                   NULL, 0, \
                   NULL, NULL, \
                   GREATER_GOD, (Str), ## __VA_ARGS__ )
#define log_error(Str, ...) \
        bug_logger(LOG_ERROR, NULL, \
                   __FILE__, __PRETTY_FUNCTION__, __LINE__, \
                   NULL, 0, \
	           NULL, NULL, \
                   GREATER_GOD, (Str), ## __VA_ARGS__ )
#define log_fatal(Str, ...) \
        bug_logger(LOG_FATAL, NULL, \
                   __FILE__, __PRETTY_FUNCTION__, __LINE__, \
                   NULL, 0, \
	           NULL, NULL, \
                   GREATER_GOD, (Str), ## __VA_ARGS__ )
#define log_boot(Str, ...) \
        bug_logger(LOG_BOOT, NULL, \
                   __FILE__, __PRETTY_FUNCTION__, __LINE__, \
                   NULL, 0, \
                   NULL, NULL, \
                   GREATER_GOD, (Str), ## __VA_ARGS__ )
#define log_auth(Ch, Str, ...) \
        bug_logger(LOG_AUTH, NULL, \
                   NULL, NULL, 0, \
                   NULL, 0, \
                   (Ch), NULL, \
                   GREATER_GOD, (Str), ## __VA_ARGS__ )
#define log_kill(Ch, Victim, Str, ...) \
        bug_logger(LOG_KILL, NULL, \
                   NULL, NULL, 0, \
                   NULL, 0, \
                   (Ch), (Victim), \
                   GREATER_GOD, (Str), ## __VA_ARGS__ )
#define log_death(Ch, Victim, Str, ...) \
        bug_logger(LOG_DEATH, NULL, \
                   NULL, NULL, 0, \
                   NULL, 0, \
                   (Ch), (Victim), \
                   GREATER_GOD, (Str), ## __VA_ARGS__ )
#define log_reset(Str, ...) \
        bug_logger(LOG_RESET, NULL, \
                   NULL, NULL, 0, \
                   NULL, 0, \
                   NULL, NULL, \
                   GREATER_GOD, (Str), ## __VA_ARGS__ )
#define log_imc(Str, ...) \
        bug_logger(LOG_IMC, NULL, \
                   NULL, NULL, 0, \
                   NULL, 0, \
                   NULL, NULL, \
                   GREATER_GOD, (Str), ## __VA_ARGS__ )
#define log_sql(Str, ...) \
        bug_logger(LOG_SQL, NULL, \
                   __FILE__, __PRETTY_FUNCTION__, __LINE__, \
                   NULL, 0, \
	           NULL, NULL, \
                   GREATER_GOD, (Str), ## __VA_ARGS__ )
#define log_nosql(Str, ...) \
        bug_logger(LOG_NOSQL, NULL, \
                   __FILE__, __PRETTY_FUNCTION__, __LINE__, \
                   NULL, 0, \
	           NULL, NULL, \
                   GREATER_GOD, (Str), ## __VA_ARGS__ )

void                                    bug_logger(unsigned int Type, const char *BugFile,
                                                   const char *File, const char *Func, int Line,
                                                   const char *AreaFile, int AreaLine,
	                                           struct char_data *ch, struct char_data *victim,
					           unsigned int Level, const char *Str, ...)
                                                   __attribute__ ( ( format( printf, 11, 12 ) ) );;

void setup_logfile_table(void);
void bug_sql( const char *logtype, const char *filename, const char *function, int line,
              const char *area_file, int area_line, 
              const char *character, int character_room,
              const char *victim, int victim_room, 
              const char *message );

#endif

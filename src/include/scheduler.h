/*
 * This is the start of a way to replace "ticks" and the
 * sluggish tick-based main loop with a delay based
 * system.
 */

#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_ 1

/*
 * Originally, DikuMUD used the concept of an ever increasing
 * tick counter, which was bumped up one every 250ms (or longer if
 * the game loop ran over).  All in-game actions are based around
 * these ticks.
 *
 * The fastest anything can happen is once every 250ms, since the
 * main loop itself is limited to only execute on each tick.  So,
 * even user input has a small delay, which was irrelevant in the
 * days of dialup modems, but is actually noticeable now.
 *
 * To redo this entirely, is a lot of work.  THe first steps will
 * be to replace "ticks" with their microsecond equivalents, so
 * we could make a 1:1 swap for mechanics without changing the
 * feel.
 *
 * Later, we'll want to divorce the main loop from this limit, and
 * allow purely informational commands to run whenever wanted, and
 * allow various in-game commands to have more flexible limits.
 *
 */

#define DELAY_PULSE 250000
#define DELAY_SECOND 1000000
#define DELAY_MINUTE (DELAY_SECOND * 60)

#define DELAY_VIOLENCE (DELAY_PULSE * 6)
#define DELAY_URL (DELAY_PULSE * 7)
#define DELAY_RIVER (DELAY_PULSE * 10)
#define DELAY_TELEPORT (DELAY_PULSE * 10)
#define DELAY_NATURE (DELAY_PULSE * 19)
#define DELAY_MOBILE (DELAY_PULSE * 25)
#define DELAY_SOUND (DELAY_PULSE * 33)
#define DELAY_ZONE (DELAY_PULSE * 239)
#define DELAY_REBOOT (DELAY_PULSE * 599)
#define DELAY_SHOUT (DELAY_PULSE * 1203)
#define DELAY_DUMP (DELAY_PULSE * 3601)

#define DELAY_MUDLIST (DELAY_MINUTE * 5)
#define DELAY_URL_HANDLER (DELAY_MINUTE * 6)

#ifndef _SCHEDULER_C
extern struct timeval last_tick;
#endif

int64_t abs64(int64_t value);
int64_t getTimestamp(void);
int64_t diffTimestamp(int64_t reference, int64_t point);
struct timeval *packTimestamp(int64_t point, struct timeval *t_point);
int timestampToTicks(int64_t point);
int64_t ticksToTimestamp(int ticks);
char *stringTimestamp(int64_t point);

#endif

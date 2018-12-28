/*
 * This is the start of a way to replace "ticks" and the
 * sluggish tick-based main loop with a delay based
 * system.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "global.h"
#include "bug.h"
#include "utils.h"
#include "comm.h"

#define _SCHEDULER_C
#include "scheduler.h"

/*
 * You've got to be kidding me.... C sucks.
 */
int64_t abs64(int64_t value)
{
    if( value < 0 )
        value = -value;
    return value;
}

/*
 * This is a way to get a full microsecond resolution integer
 * timestamp, which is useful for pretty much any kind of
 * scheduling.
 */
int64_t getTimestamp(void)
{
    struct timeval t_point;

    gettimeofday(&t_point, NULL);
    return (t_point.tv_sec * 1000000) + t_point.tv_usec;
}

/*
 * Returns the time difference in microseconds between
 * a reference point and another point.
 *
 * If you pass in a negative value as the reference, it
 * will use the epoch (0), as the reference.
 *
 * If you pass in a negative value as the point, it will
 * use gettimeofday() to use the current time.
 *
 * If the result is positive, the reference is ahead of
 * our point, meaning it has not happened yet and the value
 * is how long it will be until we reach that reference.
 *
 * If the result is negative, the reference is behind our
 * point, meaning we've passed it, and the value is how far
 * in the past our reference is.
 *
 * This simplifies our typical use case, since we can set
 * a task for now + X seconds, and then see if we've reached
 * it easily by doing if( diffTimestamp(scheduled_time, -1) <= 0 ).
 *
 */
int64_t diffTimestamp(int64_t reference, int64_t point)
{
    if( reference < 0 )
        reference = 0;
    if( point < 0 )
        point = getTimestamp();

    return (reference - point);
}

/*
 * This packs an int64_t microsecond value into the
 * antique timeval structure which the OS wants for
 * various things, like select().
 *
 * Note that microseconds will be positive unless
 * seconds is 0, since we can't have a -0 value,
 * so we'll put the sign on tv_usec.
 *
 * Also, if you pass in NULL for the destination
 * structure, it will allocate and return one.
 * Freeing the mallocs is your job.
 */
struct timeval *packTimestamp(int64_t point, struct timeval *t_point)
{
    int seconds;
    int microseconds;

    microseconds = (int)(abs64(point) % 1000000);

    if( point < 0 && point > -1000000) {
        seconds = 0;
        microseconds = -microseconds;
    } else {
        seconds = (int)(point / 1000000);
    }

    if( t_point == NULL ) {
        CREATE(t_point, struct timeval, 1);
    }

    t_point->tv_sec = seconds;
    t_point->tv_usec = microseconds;

    return t_point;
}

int timestampToTicks(int64_t point)
{
    int seconds;
    int microseconds;
    int ticks;

    microseconds = (int)(abs64(point) % 1000000);

    if( point < 0 && point > -1000000) {
        seconds = 0;
        microseconds = -microseconds;
    } else {
        seconds = (int)(point / 1000000);
    }

    ticks = seconds * PULSE_PER_SECOND;
    ticks += microseconds / (1000000 / PULSE_PER_SECOND);

    return ticks;
}

int64_t ticksToTimestamp(int ticks)
{
    int64_t point;

    point = ticks * PULSE_PER_SECOND * 1000000;

    return point;
}

char *stringTimestamp(int64_t point)
{
    static char result[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    int days = 0;
    int hours = 0;
    int minutes = 0;
    int seconds = 0;
    int microseconds = 0;

    point = abs64(point);

    microseconds = (int)(point % 1000000);
    point /= 1000000;
    seconds = (int)(point % 60);
    point /= 60;
    minutes = (int)(point % 60);
    point /= 60;
    hours = (int)(point % 24);
    point /= 24;
    days = point;

    *result = '\0';
    if(days > 0) {
        scprintf(result, MAX_STRING_LENGTH, "%d days", (int)days);
        if(hours > 0)
            scprintf(result, MAX_STRING_LENGTH, ", ");
    }
    if(hours > 0) {
        scprintf(result, MAX_STRING_LENGTH, "%d hours", (int)hours);
        if(minutes > 0)
            scprintf(result, MAX_STRING_LENGTH, ", ");
    }
    if(minutes > 0) {
        scprintf(result, MAX_STRING_LENGTH, "%d minutes", (int)minutes);
        if(seconds > 0)
            scprintf(result, MAX_STRING_LENGTH, ", ");
    }
    if(seconds > 0) {
        if(microseconds > 999) {
            scprintf(result, MAX_STRING_LENGTH, "%d.%03d seconds", (int)seconds, (int)(microseconds/1000));
        } else {
            scprintf(result, MAX_STRING_LENGTH, "%d seconds", (int)seconds);
        }
    }
    if(days > 0 || hours > 0 || minutes > 0 || seconds > 0 )
        scprintf(result, MAX_STRING_LENGTH, ".");
    else
        scprintf(result, MAX_STRING_LENGTH, "No time.");

    return result;
}


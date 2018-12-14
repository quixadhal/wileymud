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
 * If the value is negative, reference is in the past
 * for point.  If positive, reference has yet to happen.
 */
int64_t diffTimestamp(int64_t reference, int64_t point)
{
    if( reference < 0 )
        reference = 0;
    if( point < 0 )
        point = getTimestamp();

    return (point - reference);
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


/*
 * file: weather.c , Weather and time module              Part of DIKUMUD
 * Usage: Performing the clock and the weather
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */

#include <stdio.h>
#include <stdlib.h>
/* #include <unistd.h> */
#include <sys/types.h>
#include <time.h>
#include <string.h>

#include "global.h"
#include "bug.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "interpreter.h"
#include "db.h"
#define _WEATHER_C
#include "weather.h"

const char                                   *moon_names[] = {
  "new",
  "waxing cresent",
  "waxing half",
  "waxing gibbus",
  "full",
  "waning gibbus",
  "waning half",
  "waning cresent"
};

 void weather_and_time(int mode)
{
  if (DEBUG > 3)
    log_info("called %s with %d", __PRETTY_FUNCTION__, mode);

  another_hour(mode);
  if (mode)
    weather_change();
  if (time_info.hours == TIME_NOON || time_info.hours == TIME_MIDNIGHT)
    update_time_and_weather();
}

void another_hour(int mode)
{
  if (DEBUG > 2)
    log_info("called %s with %d", __PRETTY_FUNCTION__, mode);

  time_info.hours++;
  if (time_info.hours >= HOURS_PER_MUD_DAY) {
    time_info.hours = 0;
    time_info.day++;
    weather_info.moon++;
    if (weather_info.moon > MAX_MOON_PHASES)
      weather_info.moon = 1;

    if (time_info.day >= DAYS_PER_MUD_MONTH) {
      time_info.day = 0;
      time_info.month++;
      GetMonth(time_info.month);

      if (time_info.month >= MONTHS_PER_MUD_YEAR) {
	time_info.month = 0;
	time_info.year++;
      }
    }
  }
  if (mode) {
    switch (time_info.hours) {
      case 0:
	oprintf("The moon rises high overhead.\r\n");
	break;
      case 4:
	oprintf("The moon sets.\r\n");
	break;
      case 6:
	weather_info.sunlight = SUN_RISE;
	oprintf("The sun rises in the east.\r\n");
	break;
      case 7:
	weather_info.sunlight = SUN_LIGHT;
	oprintf("The day has begun.\r\n");
	break;
      case 12:
	oprintf("It is noon.\r\n");
	break;
      case 18:
	weather_info.sunlight = SUN_SET;
	oprintf("The sun slowly disappears in the west.\r\n");
	break;
      case 19:
	weather_info.sunlight = SUN_DARK;
	oprintf("The night has begun.\r\n");
	break;
      case 21:
	oprintf("A %s moon rises in the eastern sky.\r\n",
		moon_names[(weather_info.moon - 1) / 4]);
	break;
      default:
	break;
    }
  }
}

void weather_change(void)
{
  int                                     diff = 0;
  int                                     change = 0;

  if (DEBUG > 2)
    log_info("called %s with no arguments", __PRETTY_FUNCTION__);

  if ((time_info.month >= 9) && (time_info.month <= 16))
    diff = (weather_info.pressure > 985 ? -2 : 2);
  else
    diff = (weather_info.pressure > 1015 ? -2 : 2);

  weather_info.change += (dice(1, 4) * diff + dice(2, 6) - dice(2, 6));
  weather_info.change = MIN(weather_info.change, 12);
  weather_info.change = MAX(weather_info.change, -12);
  weather_info.pressure += weather_info.change;
  weather_info.pressure = MIN(weather_info.pressure, 1040);
  weather_info.pressure = MAX(weather_info.pressure, 960);
  change = 0;

  switch (weather_info.sky) {
    case SKY_CLOUDLESS:
      {
	if (weather_info.pressure < 990)
	  change = 1;
	else if (weather_info.pressure < 1010)
	  if (dice(1, 4) == 1)
	    change = 1;
	break;
      }
    case SKY_CLOUDY:
      {
	if (weather_info.pressure < 970)
	  change = 2;
	else if (weather_info.pressure < 990)
	  if (dice(1, 4) == 1)
	    change = 2;
	  else
	    change = 0;
	else if (weather_info.pressure > 1030)
	  if (dice(1, 4) == 1)
	    change = 3;
      }
      break;
    case SKY_RAINING:
      {
	if (weather_info.pressure < 970)
	  if (dice(1, 4) == 1)
	    change = 4;
	  else
	    change = 0;
	else if (weather_info.pressure > 1030)
	  change = 5;
	else if (weather_info.pressure > 1010)
	  if (dice(1, 4) == 1)
	    change = 5;
      }
      break;
    case SKY_LIGHTNING:
      {
	if (weather_info.pressure > 1010)
	  change = 6;
	else if (weather_info.pressure > 990)
	  if (dice(1, 4) == 1)
	    change = 6;
      }
      break;
    default:
      {
	change = 0;
	weather_info.sky = SKY_CLOUDLESS;
      }
      break;
  }
  ChangeWeather(change);
}

void ChangeWeather(int change)
{
  if (DEBUG > 2)
    log_info("called %s with %d", __PRETTY_FUNCTION__, change);

  if (change < 0)
    change = 0;
  if (change > 7)
    change = 6;

  switch (change) {
    case 0:
      break;
    case 1:
      {
	oprintf("The sky is getting cloudy.\r\n");
	weather_info.sky = SKY_CLOUDY;
	break;
      }
    case 2:
      {
	if ((time_info.month > 3) && (time_info.month < 14))
	  oprintf("It starts to rain.\r\n");
	else
	  oprintf("It starts to drizzle. \r\n");
	weather_info.sky = SKY_RAINING;
	break;
      }
    case 3:
      {
	oprintf("The clouds disappear.\r\n");
	weather_info.sky = SKY_CLOUDLESS;
	break;
      }
    case 4:
      {
	if ((time_info.month > 3) && (time_info.month < 14))
	  oprintf("You are caught in lightning storm.\r\n");
	else
	  oprintf("You are caught in a layer of fog. \r\n");
	weather_info.sky = SKY_LIGHTNING;
	break;
      }
    case 5:
      {
	if ((time_info.month > 3) && (time_info.month < 14))
	  oprintf("The rain has stopped.\r\n");
	else
	  oprintf("The drizzle has stopped. \r\n");
	weather_info.sky = SKY_CLOUDY;
	break;
      }
    case 6:
      {
	if ((time_info.month > 3) && (time_info.month < 14))
	  oprintf("The lightning has gone, but it is still raining.\r\n");
	else
	  oprintf("The fog has lifted, but it is still drizzling.\r\n");
	weather_info.sky = SKY_RAINING;
	break;
      }
    default:
      break;
  }
}

void GetMonth(int month)
{
  if (DEBUG > 2)
    log_info("called %s with %d", __PRETTY_FUNCTION__, month);

  if (month < 0)
    return;

  switch (month) {
    case 0:
    case 1:
      oprintf(" It is bitterly cold outside\r\n");
      break;
    case 2:
      oprintf(" It is very cold \r\n");
      break;
    case 3:
    case 4:
      oprintf(" It is chilly outside \r\n");
      break;
    case 5:
      oprintf(" The flowers start to bloom \r\n");
      break;
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
      oprintf(" It is warm and humid. \r\n");
      break;
    case 12:
      oprintf(" It starts to get a little windy \r\n");
      break;
    case 13:
      oprintf(" The air is getting chilly \r\n");
      break;
    case 14:
      oprintf(" The leaves start to change colors. \r\n");
      break;
    case 15:
      oprintf(" It starts to get cold \r\n");
      break;
    case 16:
    case 17:
      oprintf(" It is bitterly cold outside \r\n");
      break;
  }
}

 void reset_weather(void)
{
  if (DEBUG > 3)
    log_info("called %s with no arguments", __PRETTY_FUNCTION__);

  weather_info.pressure = 960;
  weather_info.change = 0;
  weather_info.sky = SKY_CLOUDLESS;
  weather_info.wind_speed = 0;
  weather_info.wind_direction = WIND_DEAD;
}

void reset_time(void)
{
  FILE                                   *f1 = NULL;
  char                                    buf[80] = "\0\0\0";
  long                                    beginning_of_time = 650336715; /* Fri Aug 10 21:05:15 1990 */
  long                                    current_time = 0L;

  if (DEBUG > 2)
    log_info("called %s with no arguments", __PRETTY_FUNCTION__);

  /*
   * struct time_info_data mud_time_passed(time_t t2, time_t t1); 
   */

  if (!(f1 = fopen(TIME_FILE, "r"))) {
    log_info("Reset Time: Time file does not exist!\n");
    time_info = mud_time_passed(time(0), beginning_of_time);
    reset_weather();
  } else {
    fgets(buf, 80, f1);
    fscanf(f1, "%ld\n", &current_time);
    fscanf(f1, "%d\n", &time_info.hours);
    fscanf(f1, "%d\n", &time_info.day);
    fscanf(f1, "%d\n", &time_info.month);
    fscanf(f1, "%d\n", &time_info.year);
    fgets(buf, 80, f1);
    fscanf(f1, "%d\n", &weather_info.pressure);
    fscanf(f1, "%d\n", &weather_info.change);
    fscanf(f1, "%d\n", &weather_info.sky);
    fscanf(f1, "%d\n", &weather_info.sunlight);
    fscanf(f1, "%d\n", &weather_info.wind_speed);
    fscanf(f1, "%d\n", &weather_info.wind_direction);
    fscanf(f1, "%d\n", &weather_info.moon);
    FCLOSE(f1);
  }

  weather_and_time(1);

  log_info("   Current Gametime: %dH %dD %dM %dY.",
       time_info.hours, time_info.day, time_info.month, time_info.year);
}

void update_time_and_weather(void)
{
  FILE                                   *f1 = NULL;
  long                                    current_time = 0L;

  if (DEBUG > 2)
    log_info("called %s with no arguments", __PRETTY_FUNCTION__);

  if (!(f1 = fopen(TIME_FILE, "w"))) {
    log_error("update time");
    return;
  }
  current_time = time(0);
  log_info("Time update.");
  fprintf(f1, "# Time -last,hours,day,month,year\n");
  fprintf(f1, "%ld\n", current_time);
  fprintf(f1, "%d\n", time_info.hours);
  fprintf(f1, "%d\n", time_info.day);
  fprintf(f1, "%d\n", time_info.month);
  fprintf(f1, "%d\n", time_info.year);
  log_info("Weather update.");
  fprintf(f1, "# Weather -pressure,change,sky,sunlight,windspeed,direction\n");
  fprintf(f1, "%d\n", weather_info.pressure);
  fprintf(f1, "%d\n", weather_info.change);
  fprintf(f1, "%d\n", weather_info.sky);
  fprintf(f1, "%d\n", weather_info.sunlight);
  fprintf(f1, "%d\n", weather_info.wind_speed);
  fprintf(f1, "%d\n", weather_info.wind_direction);
  fprintf(f1, "%d\n", weather_info.moon);
  FCLOSE(f1);
}

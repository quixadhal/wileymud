/*
 * file: weather.c , Weather and time module              Part of DIKUMUD
 * Usage: Performing the clock and the weather
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
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

void weather_and_time(int mode)
{
  another_hour(mode);
  if (mode)
    weather_change();
  if (time_info.hours == TIME_NOON || time_info.hours == TIME_MIDNIGHT)
    update_time_and_weather();
}

void another_hour(int mode)
{
  char moon[20], buf[100];

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
      send_to_outdoor("The moon rises high overhead.\n\r");
      break;
    case 4:
      send_to_outdoor("The moon sets.\n\r");
      break;
    case 6:
      weather_info.sunlight = SUN_RISE;
      send_to_outdoor("The sun rises in the east.\n\r");
      break;
    case 7:
      weather_info.sunlight = SUN_LIGHT;
      send_to_outdoor("The day has begun.\n\r");
      break;
    case 12:
      send_to_outdoor("It is noon.\n\r");
      break;
    case 18:
      weather_info.sunlight = SUN_SET;
      send_to_outdoor("The sun slowly disappears in the west.\n\r");
      break;
    case 19:
      weather_info.sunlight = SUN_DARK;
      send_to_outdoor("The night has begun.\n\r");
      break;
    case 21:
      if (weather_info.moon < 4) {
	strcpy(moon, "new");
      } else if (weather_info.moon < 12) {
	strcpy(moon, "waxing");
      } else if (weather_info.moon < 20) {
	strcpy(moon, "full");
      } else if (weather_info.moon < 28) {
	strcpy(moon, "waning");
      } else {
	strcpy(moon, "new");
      }
      sprintf(buf, "A %s moon rises in the east\n\r", moon);
      send_to_outdoor(buf);
      break;
    default:
      break;
    }
  }
}

void weather_change(void)
{
  int diff, change;

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
  if (change < 0)
    change = 0;
  if (change > 7)
    change = 6;

  switch (change) {
  case 0:
    break;
  case 1:
    {
      send_to_outdoor("The sky is getting cloudy.\n\r");
      weather_info.sky = SKY_CLOUDY;
      break;
    }
  case 2:
    {
      if ((time_info.month > 3) && (time_info.month < 14))
	send_to_outdoor("It starts to rain.\n\r");
      else
	send_to_outdoor("It starts to drizzle. \n\r");
      weather_info.sky = SKY_RAINING;
      break;
    }
  case 3:
    {
      send_to_outdoor("The clouds disappear.\n\r");
      weather_info.sky = SKY_CLOUDLESS;
      break;
    }
  case 4:
    {
      if ((time_info.month > 3) && (time_info.month < 14))
	send_to_outdoor("You are caught in lightning storm.\n\r");
      else
	send_to_outdoor("You are caught in a layer of fog. \n\r");
      weather_info.sky = SKY_LIGHTNING;
      break;
    }
  case 5:
    {
      if ((time_info.month > 3) && (time_info.month < 14))
	send_to_outdoor("The rain has stopped.\n\r");
      else
	send_to_outdoor("The drizzle has stopped. \n\r");
      weather_info.sky = SKY_CLOUDY;
      break;
    }
  case 6:
    {
      if ((time_info.month > 3) && (time_info.month < 14))
	send_to_outdoor("The lightning has gone, but it is still raining.\n\r");
      else
	send_to_outdoor("The fog has lifted, but it is still drizzling.\n\r");
      weather_info.sky = SKY_RAINING;
      break;
    }
  default:
    break;
  }
}

void GetMonth(int month)
{
  if (month < 0)
    return;

  switch (month) {
  case 0:
  case 1:
    send_to_outdoor(" It is bitterly cold outside\n\r");
    break;
  case 2:
    send_to_outdoor(" It is very cold \n\r");
    break;
  case 3:
  case 4:
    send_to_outdoor(" It is chilly outside \n\r");
    break;
  case 5:
    send_to_outdoor(" The flowers start to bloom \n\r");
    break;
  case 6:
  case 7:
  case 8:
  case 9:
  case 10:
  case 11:
    send_to_outdoor(" It is warm and humid. \n\r");
    break;
  case 12:
    send_to_outdoor(" It starts to get a little windy \n\r");
    break;
  case 13:
    send_to_outdoor(" The air is getting chilly \n\r");
    break;
  case 14:
    send_to_outdoor(" The leaves start to change colors. \n\r");
    break;
  case 15:
    send_to_outdoor(" It starts to get cold \n\r");
    break;
  case 16:
  case 17:
    send_to_outdoor(" It is bitterly cold outside \n\r");
    break;
  }
}

void reset_weather(void)
{
  weather_info.pressure = 960;
  weather_info.change = 0;
  weather_info.sky = SKY_CLOUDLESS;
  weather_info.wind_speed = 0;
  weather_info.wind_direction = WIND_DEAD;
}

void reset_time(void)
{
  FILE *f1;
  char buf[80];
  long beginning_of_time = 650336715;
  long current_time;
  struct time_info_data mud_time_passed(time_t t2, time_t t1);

  if (!(f1 = fopen(TIME_FILE, "r"))) {
    fprintf(stderr, "Reset Time: Time file does not exist!\n");
    time_info = mud_time_passed(time(0), beginning_of_time);
    reset_weather();
  } else {
    fgets(buf, 80, f1);
    fscanf(f1, "%d\n", &current_time);
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
    fclose(f1);
  }

  weather_and_time(1);

  sprintf(buf, "   Current Gametime: %dH %dD %dM %dY.",
	  time_info.hours, time_info.day,
	  time_info.month, time_info.year);
  log(buf);
}

void update_time_and_weather(void)
{
  FILE *f1;
  long current_time;

  if (!(f1 = fopen(TIME_FILE, "w"))) {
    perror("update time");
    return;
  }
  current_time = time(0);
  log("Time update.");
  fprintf(f1, "# Time -last,hours,day,month,year\n");
  fprintf(f1, "%d\n", current_time);
  fprintf(f1, "%d\n", time_info.hours);
  fprintf(f1, "%d\n", time_info.day);
  fprintf(f1, "%d\n", time_info.month);
  fprintf(f1, "%d\n", time_info.year);
  log("Weather update.");
  fprintf(f1, "# Weather -pressure,change,sky,sunlight,windspeed,direction\n");
  fprintf(f1, "%d\n", weather_info.pressure);
  fprintf(f1, "%d\n", weather_info.change);
  fprintf(f1, "%d\n", weather_info.sky);
  fprintf(f1, "%d\n", weather_info.sunlight);
  fprintf(f1, "%d\n", weather_info.wind_speed);
  fprintf(f1, "%d\n", weather_info.wind_direction);
  fprintf(f1, "%d\n", weather_info.moon);
  fclose(f1);
}

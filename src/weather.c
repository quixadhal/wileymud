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
#include "sql.h"
#define _WEATHER_C
#include "weather.h"

const time_t beginning_of_time = BEGINNING_OF_TIME; /* Fri Aug 10 21:05:15 1990 */

const char *moon_names[] = {"new",  "waxing cresent", "waxing half", "waxing gibbus",
                            "full", "waning gibbus",  "waning half", "waning cresent"};

/* Calculate the REAL time passed over the last t2-t1 centuries (secs) */
struct time_info_data real_time_passed(time_t t2, time_t t1)
{
    time_t secs = 0L;
    struct time_info_data now;

    if (DEBUG > 3)
        log_info("called %s with %ld, %ld", __PRETTY_FUNCTION__, t2, t1);

    secs = (time_t)(t2 - t1);

    now.hours = (secs / SECS_PER_REAL_HOUR) % 24; /* 0..23 hours */
    secs -= SECS_PER_REAL_HOUR * now.hours;
    now.day = (secs / SECS_PER_REAL_DAY); /* 0..34 days */
    secs -= SECS_PER_REAL_DAY * now.day;
    now.month = -1;
    now.year = -1;

    now.last_updated = time(0);
    return now;
}

/* Calculate the MUD time passed over the last t2-t1 centuries (secs) */
struct time_info_data mud_time_passed(time_t t2, time_t t1)
{
    time_t secs = 0L;
    struct time_info_data now;

    if (DEBUG > 3)
        log_info("called %s with %ld, %ld", __PRETTY_FUNCTION__, t2, t1);

    secs = (time_t)(t2 - t1);

    now.hours = (secs / SECS_PER_MUD_HOUR) % 24; /* 0..23 hours */
    secs -= SECS_PER_MUD_HOUR * now.hours;
    now.day = (secs / SECS_PER_MUD_DAY) % 35; /* 0..34 days */
    secs -= SECS_PER_MUD_DAY * now.day;
    now.month = (secs / SECS_PER_MUD_MONTH) % 17; /* 0..16 months */
    secs -= SECS_PER_MUD_MONTH * now.month;
    now.year = (secs / SECS_PER_MUD_YEAR); /* 0..XX? years */

    now.last_updated = time(0);
    return now;
}

struct time_info_data age(struct char_data *ch)
{
    struct time_info_data player_age;

    if (DEBUG > 3)
        log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_NAME(ch));

    player_age = mud_time_passed(time(0), ch->player.time.birth);
    player_age.year += 17; /* All players start at 17 */

    return player_age;
}

void update_weather_and_time(void)
{
    if (DEBUG > 3)
        log_info("called %s with no arguments", __PRETTY_FUNCTION__);

    another_hour();
    weather_change();

    if (time_info.hours == TIME_NOON || time_info.hours == TIME_MIDNIGHT)
    {
        save_weather(time_info, weather_info);
    }
}

void another_hour(void)
{
    if (DEBUG > 2)
        log_info("called %s with no arguments", __PRETTY_FUNCTION__);

    time_info.hours++;
    if (time_info.hours >= HOURS_PER_MUD_DAY)
    {
        time_info.hours = 0;
        time_info.day++;
        weather_info.moon++;
        if (weather_info.moon > MAX_MOON_PHASES)
            weather_info.moon = 1;

        if (time_info.day >= DAYS_PER_MUD_MONTH)
        {
            time_info.day = 0;
            time_info.month++;
            GetMonth(time_info.month);

            if (time_info.month >= MONTHS_PER_MUD_YEAR)
            {
                time_info.month = 0;
                time_info.year++;
            }
        }
    }

    switch (time_info.hours)
    {
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
        oprintf("A %s moon rises in the eastern sky.\r\n", moon_names[(weather_info.moon - 1) / 4]);
        break;
    default:
        break;
    }
}

void weather_change(void)
{
    int diff = 0;
    int change = 0;

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

    switch (weather_info.sky)
    {
    case SKY_CLOUDLESS: {
        if (weather_info.pressure < 990)
            change = 1;
        else if (weather_info.pressure < 1010)
            if (dice(1, 4) == 1)
                change = 1;
        break;
    }
    case SKY_CLOUDY: {
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
    case SKY_RAINING: {
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
    case SKY_LIGHTNING: {
        if (weather_info.pressure > 1010)
            change = 6;
        else if (weather_info.pressure > 990)
            if (dice(1, 4) == 1)
                change = 6;
    }
    break;
    default: {
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

    switch (change)
    {
    case 0:
        break;
    case 1: {
        oprintf("The sky is getting cloudy.\r\n");
        weather_info.sky = SKY_CLOUDY;
        break;
    }
    case 2: {
        if ((time_info.month > 3) && (time_info.month < 14))
            oprintf("It starts to rain.\r\n");
        else
            oprintf("It starts to drizzle. \r\n");
        weather_info.sky = SKY_RAINING;
        break;
    }
    case 3: {
        oprintf("The clouds disappear.\r\n");
        weather_info.sky = SKY_CLOUDLESS;
        break;
    }
    case 4: {
        if ((time_info.month > 3) && (time_info.month < 14))
            oprintf("You are caught in lightning storm.\r\n");
        else
            oprintf("You are caught in a layer of fog. \r\n");
        weather_info.sky = SKY_LIGHTNING;
        break;
    }
    case 5: {
        if ((time_info.month > 3) && (time_info.month < 14))
            oprintf("The rain has stopped.\r\n");
        else
            oprintf("The drizzle has stopped. \r\n");
        weather_info.sky = SKY_CLOUDY;
        break;
    }
    case 6: {
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

    switch (month)
    {
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

struct weather_data default_weather(void)
{
    struct weather_data local_weather;

    local_weather.last_updated = time(0);
    local_weather.pressure = 960;
    local_weather.change = 0;
    local_weather.sky = SKY_CLOUDLESS;
    local_weather.wind_speed = 0;
    local_weather.wind_direction = WIND_DEAD;

    return local_weather;
}

void setup_weather_table(void)
{
    PGresult *res = NULL;
    ExecStatusType st = 0;
    char *sql = "CREATE TABLE IF NOT EXISTS weather ( "
                "    updated TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT now(), "
                "    hours INTEGER, "
                "    day INTEGER, "
                "    month INTEGER, "
                "    year INTEGER, "
                "    pressure INTEGER, "
                "    change INTEGER, "
                "    sky INTEGER, "
                "    sunlight INTEGER, "
                "    wind_speed INTEGER, "
                "    wind_direction INTEGER, "
                "    moon INTEGER "
                "); ";
    char *sql2 = "SELECT count(*) FROM weather;";
    // BEGINNING_OF_TIME magic constant
    char *sql3 = "INSERT INTO weather (updated) VALUES (to_timestamp(650336715));";
    char *sql4 = "DELETE FROM weather WHERE updated <> (SELECT max(UPDATED) FROM weather);";
    int rows = 0;
    int columns = 0;
    int count = 0;
    struct time_info_data local_time;
    struct weather_data local_weather;

    sql_connect(&db_wileymud);
    res = PQexec(db_wileymud.dbc, sql);
    st = PQresultStatus(res);
    if (st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE)
    {
        log_fatal("Cannot create weather table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);

    res = PQexec(db_wileymud.dbc, sql2);
    st = PQresultStatus(res);
    if (st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE)
    {
        log_fatal("Cannot get row count of weather table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    rows = PQntuples(res);
    columns = PQnfields(res);
    if (rows > 0 && columns > 0)
    {
        count = atoi(PQgetvalue(res, 0, 0));
    }
    else
    {
        log_fatal("Invalid result set from row count!");
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);

    if (count < 1)
    {
        local_time = mud_time_passed(time(0), beginning_of_time);
        local_weather = default_weather();

        // Insert our default value.
        res = PQexec(db_wileymud.dbc, sql3);
        st = PQresultStatus(res);
        if (st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE)
        {
            log_fatal("Cannot insert placeholder row to weather table: %s", PQerrorMessage(db_wileymud.dbc));
            PQclear(res);
            proper_exit(MUD_HALT);
        }
        PQclear(res);
    }
    else if (count > 1)
    {
        // That shouldn't happen... fix it!
        log_error("There are %d rows in the weather table, instead of just ONE!", count);
        res = PQexec(db_wileymud.dbc, sql4);
        st = PQresultStatus(res);
        if (st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE)
        {
            log_fatal("Cannot remove old rows from weather table: %s", PQerrorMessage(db_wileymud.dbc));
            PQclear(res);
            proper_exit(MUD_HALT);
        }
        PQclear(res);
    }
}

void load_weather(void)
{
    PGresult *res = NULL;
    ExecStatusType st = 0;
    const char *sql = "SELECT extract(EPOCH from updated) AS the_time, "
                      "       hours, "
                      "       day, "
                      "       month, "
                      "       year, "
                      "       pressure, "
                      "       change, "
                      "       sky, "
                      "       sunlight, "
                      "       wind_speed, "
                      "       wind_direction, "
                      "       moon "
                      "FROM   weather "
                      "LIMIT  1;";
    int rows = 0;
    int columns = 0;
    struct time_info_data local_time;
    struct weather_data local_weather;

    local_time = mud_time_passed(time(0), beginning_of_time);
    local_weather = default_weather();

    sql_connect(&db_wileymud);
    res = PQexec(db_wileymud.dbc, sql);
    st = PQresultStatus(res);
    if (st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE)
    {
        log_error("Weather info does not exist in database: %s", PQerrorMessage(db_wileymud.dbc));
        // PQclear(res);
        // proper_exit(MUD_HALT);
        log_boot("  Using default weather data.");
    }
    else
    {
        rows = PQntuples(res);
        columns = PQnfields(res);
        if (rows > 0 && columns > 11)
        {
            // The PQgetvalue() API will return the integer we expect as a
            // NULL terminated string, so we can convert it with atoi().
            local_time.last_updated = (time_t)atol(PQgetvalue(res, 0, 0));
            local_time.hours = atoi(PQgetvalue(res, 0, 1));
            local_time.day = atoi(PQgetvalue(res, 0, 2));
            local_time.month = atoi(PQgetvalue(res, 0, 3));
            local_time.year = atoi(PQgetvalue(res, 0, 4));

            local_weather.last_updated = (time_t)atol(PQgetvalue(res, 0, 0));
            local_weather.pressure = atoi(PQgetvalue(res, 0, 5));
            local_weather.change = atoi(PQgetvalue(res, 0, 6));
            local_weather.sky = atoi(PQgetvalue(res, 0, 7));
            local_weather.sunlight = atoi(PQgetvalue(res, 0, 8));
            local_weather.wind_speed = atoi(PQgetvalue(res, 0, 9));
            local_weather.wind_direction = atoi(PQgetvalue(res, 0, 10));
            local_weather.moon = atoi(PQgetvalue(res, 0, 11));

            log_boot("  Using weather data from SQL database.");
        }
        else
        {
            log_error("Invalid weather info query result: %s", PQerrorMessage(db_wileymud.dbc));
            log_boot("  Using default weather data.");
        }
    }
    PQclear(res);

    time_info = local_time;
    weather_info = local_weather;
    update_weather_and_time();
    log_info("   Current Gametime: %dH %dD %dM %dY.", time_info.hours, time_info.day, time_info.month, time_info.year);
}

void save_weather(struct time_info_data local_time, struct weather_data local_weather)
{
    PGresult *res = NULL;
    ExecStatusType st = 0;
    const char *sql = "UPDATE weather SET "
                      "    updated = now(), "
                      "    hours = $1, "
                      "    day = $2, "
                      "    month = $3, "
                      "    year = $4, "
                      "    pressure = $5, "
                      "    change = $6, "
                      "    sky = $7, "
                      "    sunlight = $8, "
                      "    wind_speed = $9, "
                      "    wind_direction = $10, "
                      "    moon = $11;";
    char tmp[11][MAX_INPUT_LENGTH];
    char *param_val[11];
    int param_len[11];
    int param_bin[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    snprintf(tmp[0], MAX_INPUT_LENGTH, "%d", local_time.hours);
    snprintf(tmp[1], MAX_INPUT_LENGTH, "%d", local_time.day);
    snprintf(tmp[2], MAX_INPUT_LENGTH, "%d", local_time.month);
    snprintf(tmp[3], MAX_INPUT_LENGTH, "%d", local_time.year);
    snprintf(tmp[4], MAX_INPUT_LENGTH, "%d", local_weather.pressure);
    snprintf(tmp[5], MAX_INPUT_LENGTH, "%d", local_weather.change);
    snprintf(tmp[6], MAX_INPUT_LENGTH, "%d", local_weather.sky);
    snprintf(tmp[7], MAX_INPUT_LENGTH, "%d", local_weather.sunlight);
    snprintf(tmp[8], MAX_INPUT_LENGTH, "%d", local_weather.wind_speed);
    snprintf(tmp[9], MAX_INPUT_LENGTH, "%d", local_weather.wind_direction);
    snprintf(tmp[10], MAX_INPUT_LENGTH, "%d", local_weather.moon);

    for (int i = 0; i < 12; i++)
    {
        param_val[i] = tmp[i][0] ? tmp[i] : NULL;
        param_len[i] = tmp[i][0] ? strlen(tmp[i]) : 0;
    }

    sql_connect(&db_wileymud);
    res = PQexecParams(db_wileymud.dbc, sql, 11, NULL, (const char **)param_val, param_len, param_bin, 0);
    st = PQresultStatus(res);
    if (st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE)
    {
        log_error("Cannot update weather: %s", PQerrorMessage(db_wileymud.dbc));
        // PQclear(res);
        // proper_exit(MUD_HALT);
    }
    PQclear(res);
}

#ifndef _WEATHER_H
#define _WEATHER_H

#define SECS_PER_REAL_MIN   60
#define SECS_PER_REAL_HOUR  (60*SECS_PER_REAL_MIN)
#define SECS_PER_REAL_DAY   (24*SECS_PER_REAL_HOUR)
#define SECS_PER_REAL_YEAR  (365*SECS_PER_REAL_DAY)

#define SECS_PER_MUD_HOUR   75
#define HOURS_PER_MUD_DAY   24
#define DAYS_PER_MUD_MONTH  35
#define MONTHS_PER_MUD_YEAR 17
#define SECS_PER_MUD_DAY    (HOURS_PER_MUD_DAY*SECS_PER_MUD_HOUR)
#define SECS_PER_MUD_MONTH  (DAYS_PER_MUD_MONTH*SECS_PER_MUD_DAY)
#define SECS_PER_MUD_YEAR   (MONTHS_PER_MUD_YEAR*SECS_PER_MUD_MONTH)

/* How much light is in the land ? */
#define SUN_DARK            0
#define SUN_RISE            1
#define SUN_LIGHT           2
#define SUN_SET             3

/* And how is the sky ? */
#define SKY_CLOUDLESS       0
#define SKY_CLOUDY          1
#define SKY_RAINING         2
#define SKY_LIGHTNING       3

/* Which way is the wind blowing */
#define WIND_NORTH          0
#define WIND_EAST           1
#define WIND_SOUTH          2
#define WIND_WEST           3
#define WIND_DEAD           4

#define TIME_NOON           12
#define TIME_MIDNIGHT       0
#define MAX_MOON_PHASES     32

struct time_info_data {
    time_t  last_updated;
    int     hours;
    int     day;
    int     month;
    int     year;
};

struct weather_data {
    time_t  last_updated;
    int     pressure;       /* How is the pressure ( Mb ) */
    int     change;         /* How fast and what way does it change. */
    int     sky;            /* How is the sky. */
    int     sunlight;       /* And how much sun. */
    int     wind_speed;     /* how hard is the wind blowing */
    int     wind_direction; /* which direction is the wind blowing */
    int     moon;           /* what is the moon like */
};

#ifndef _WEATHER_C
extern const char       *moon_names[];
extern const time_t     beginning_of_time;
#endif

struct time_info_data                   real_time_passed(time_t t2, time_t t1);
struct time_info_data                   mud_time_passed(time_t t2, time_t t1);
struct time_info_data                   age(struct char_data *ch);

void    update_weather_and_time(void);
void    another_hour(void);
void    weather_change(void);
void    ChangeWeather(int change);
void    GetMonth(int month);
struct weather_data default_weather(void);

void    setup_weather_table(void);
void    load_weather(void);
void    save_weather(struct time_info_data local_time, struct weather_data local_weather);

#endif

#ifndef _WEATHER_H
#define _WEATHER_H

/* uses */
#define TIME_NOON	12
#define TIME_MIDNIGHT	0
#define MAX_MOON_PHASES	32

#ifndef _WEATHER_C
extern const char   *moon_names[];
#endif

void    weather_and_time(int mode);
void    another_hour(int mode);
void    weather_change(void);
void    ChangeWeather(int change);
void    GetMonth(int month);
void    reset_weather(void);
void    reset_time(void);
void    update_time_and_weather(void);

void    setup_weather_table(void);
void    load_weather(const char *filename);
void    save_weather(const char *filename, struct time_info_data local_time,
                     struct weather_data local_weather);

#endif

/*
 * file: comm.h , Communication module.                   Part of DIKUMUD
 * Usage: Prototypes for the common functions in comm.c
 */
#ifndef _COMM_H
#define _COMM_H

/* #define RFC1413 -- This can be laggy since it isn't threaded! */

#define send_to_char(m,c) cprintf((c),(m))
#define send_to_room(m,r) rprintf((r),(m))
#define send_to_all(m) allprintf(m)
#define send_to_outdoor(m) oprintf(m)
#define send_to_except(m,e) eprintf((e),(m))
#define send_to_room_except(m,r,e) reprintf((r),(e),(m))
#define send_to_room_except_two(m,r,e1,e2) re2printf((r),(e1),(e2),(m))

#define DFLT_PORT 3000

#define MUD_REBOOT	0
#define MUD_HALT	42

#define MAX_NAME_LENGTH 15
#define MAX_HOSTNAME   256
#define OPT_USEC 250000					       /* time delay corresponding to 4 passes/sec */

#define STATE(d) ((d)->connected)
#define PROFILE(x)
#define SEND_TO_Q(messg, desc)  write_to_q((messg), &(desc)->output, 1)

#define TO_ROOM    0
#define TO_VICT    1
#define TO_NOTVICT 2
#define TO_CHAR    3

#define TELNET_GA   249
#define TELNET_SB   250
#define TELNET_SE   240
#define TELNET_WILL 0xFB
#define TELNET_WONT 0xFC
#define TELNET_DO   0xFD
#define TELNET_DONT 0xFE
#define TELNET_IAC  0xFF

#ifndef _COMM_C
extern struct descriptor_data          *descriptor_list;
extern struct descriptor_data          *next_to_process;
extern int                              slow_death;
extern int                              diku_shutdown;
extern int                              diku_reboot;
extern int                              DEBUG;
extern int                              no_specials;
extern long                             Uptime;
extern int                              maxdesc;
extern int                              avail_descs;
extern int                              tics;
extern int                              pulse;
extern int                              pulse_update;
extern int                              pulse_river;
extern int                              pulse_teleport;
extern int                              pulse_nature;
extern int                              pulse_sound;
extern int                              pulse_zone;
extern int                              pulse_mobile;
extern int                              pulse_violence;
extern int                              pulse_reboot;
extern int                              pulse_dump;
extern int                              pulse_mudlist;
extern int                              mud_port;

#endif

int                                     main(int argc, const char **argv);
int                                     run_the_game(int port);
void                                    emit_prompt(struct descriptor_data *point);
void                                    game_loop(int s);
int                                     get_from_q(struct txt_q *queue, char *dest);
void                                    write_to_q(const char *txt, struct txt_q *queue, int do_timestamp);
struct timeval                          timediff(struct timeval *a, struct timeval *b);
void                                    flush_queues(struct descriptor_data *d);
int                                     init_socket(int port);
int                                     new_connection(int s);

int                                     new_descriptor(int s);
int                                     process_output(struct descriptor_data *t);
int                                     write_to_descriptor(int desc, const char *txt);
int                                     process_input(struct descriptor_data *t);
void                                    close_sockets(int s);
void                                    close_socket(struct descriptor_data *d);
void                                    nonblock(int s);

void                                    dcprintf(struct descriptor_data *d, const char *Str, ...)
                                                 __attribute__ ( ( format( printf, 2, 3 ) ) );
void                                    cprintf(struct char_data *ch, const char *Str, ...)
                                                 __attribute__ ( ( format( printf, 2, 3 ) ) );
void                                    rprintf(int room, const char *Str, ...)
                                                 __attribute__ ( ( format( printf, 2, 3 ) ) );
void                                    zprintf(int zone, const char *Str, ...)
                                                 __attribute__ ( ( format( printf, 2, 3 ) ) );
void                                    allprintf(const char *Str, ...)
                                                 __attribute__ ( ( format( printf, 1, 2 ) ) );
void                                    oprintf(const char *Str, ...)
                                                 __attribute__ ( ( format( printf, 1, 2 ) ) );
void                                    eprintf(struct char_data *ch, const char *Str, ...)
                                                 __attribute__ ( ( format( printf, 2, 3 ) ) );
void                                    reprintf(int room, struct char_data *ch, const char *Str,
						 ...) __attribute__ ( ( format( printf, 3, 4 ) ) );
void                                    re2printf(int room, struct char_data *ch1,
						  struct char_data *ch2, const char *Str, ...)
                                                 __attribute__ ( ( format( printf, 4, 5 ) ) );
void                                    iprintf(const char *Str, ...)
                                                 __attribute__ ( ( format( printf, 1, 2 ) ) );
void                                    save_all(void);
void                                    act(const char *str, int hide_invisible, struct char_data *ch,
					    struct obj_data *obj, void *vict_obj, int type, ...)
                                            __attribute__ ( ( format( printf, 1, 7 ) ) );
void                                    dump_player_list(void);
void					proper_exit(int exit_code);

#endif

/*
 *  file: comm.c , Communication module.                   Part of DIKUMUD
 *  Usage: Communication, central game loop.
 *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 *  All Rights Reserved
 *  Using *any* part of DikuMud without having read license.doc is
 *  violating our copyright.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/un.h>
#include <sys/resource.h>
#include <stddef.h>
#include <locale.h>

#include "global.h"
#ifdef I3
#include "i3.h"
#endif
#ifdef IMC
#include "imc.h"
#endif
#include "bug.h"
#include "utils.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "modify.h"
#include "whod.h"
#include "multiclass.h"
#include "weather.h"
#include "mudlimits.h"
#include "spells.h"
#include "spell_parser.h"
#include "sound.h"
#include "fight.h"
#include "mob_actions.h"
#include "act_other.h"
#include "signals.h"
#include "ban.h"
#include "board.h"
#define _COMM_C
#include "comm.h"

#ifdef RFC1413
#include "libident-0.19/ident.h"
#endif

struct descriptor_data                 *descriptor_list = NULL;
struct descriptor_data                 *next_to_process = NULL;

int                                     mud_port = 0;
int                                     slow_death = 0;	       /* Shut her down, Martha, she's sucking mud */
int                                     diku_shutdown = 0;     /* clean shutdown */
int                                     diku_reboot = 0;       /* reboot the game after a shutdown */
int                                     DEBUG = FALSE;
int                                     no_specials = 0;       /* Suppress ass. of special routines */
long                                    Uptime = 0L;	       /* time that the game has been up */

int                                     maxdesc = 0;
int                                     avail_descs = 0;
int                                     tics = 0;	       /* for extern checkpointing */
int                                     pulse = 0;
int                                     pulse_update = PULSE_UPDATE + PULSE_VARIABLE;
int                                     pulse_river = PULSE_RIVER;
int                                     pulse_teleport = PULSE_TELEPORT;
int                                     pulse_nature = PULSE_NATURE;
int                                     pulse_sound = PULSE_SOUND;
int                                     pulse_zone = PULSE_ZONE;
int                                     pulse_mobile = PULSE_MOBILE;
int                                     pulse_violence = PULSE_VIOLENCE;
int                                     pulse_reboot = PULSE_REBOOT;
int                                     pulse_dump = PULSE_DUMP;
int                                     pulse_mudlist = PULSE_MUDLIST;

int main(int argc, const char **argv)
{
    int                                     port = 0;
    int                                     pos = 1;
    int                                     exit_code = 0;
    const char                             *dir = NULL;
    const char                             *logfile = NULL;
    const char                             *pidfile = NULL;

    if (DEBUG > 1)
	log_info("called %s with %d, %08zx", __PRETTY_FUNCTION__, argc, (size_t) argv);

    port = DFLT_PORT;
    dir = DFLT_DIR;
    WizLock = FALSE;

    setlocale(LC_ALL, "en_US.utf8");

    while ((pos < argc) && (*(argv[pos]) == '-')) {
	switch (*(argv[pos] + 1)) {
	    case 'w':
		WizLock = TRUE;
		log_info("WizLock is SET.");
		break;
	    case 'D':
		DEBUG = TRUE;
		log_info("Debugging is on.");
		break;
	    case 'l':
		log_info("Lawful mode no longer available.");
		break;
	    case 'L':
		if (*(argv[pos] + 2))
		    logfile = argv[pos] + 2;
		else if (++pos < argc)
		    logfile = argv[pos];
		else {
		    log_fatal("LOG filename expected after option -L.");
		    proper_exit(MUD_HALT);
		}
		break;
	    case 'P':
		if (*(argv[pos] + 2))
		    pidfile = argv[pos] + 2;
		else if (++pos < argc)
		    pidfile = argv[pos];
		else {
		    log_fatal("PID filename expected after option -P.");
		    proper_exit(MUD_HALT);
		}
		break;
	    case 'd':
		if (*(argv[pos] + 2))
		    dir = argv[pos] + 2;
		else if (++pos < argc)
		    dir = argv[pos];
		else {
		    log_fatal("Directory arg expected after option -d.");
		    proper_exit(MUD_HALT);
		}
		break;
	    case 's':
		no_specials = 1;
		log_info("Suppressing assignment of special routines.");
		break;
	    default:
		log_info("Unknown option -% in argument string.", *(argv[pos] + 1));
		break;
	}
	pos++;
    }

    if (pos < argc) {
	if (!isdigit(*argv[pos])) {
	    log_fatal("Usage: %s [-l] [-s] [-d pathname] [ port # ]\n", argv[0]);
	    proper_exit(MUD_HALT);
	} else if ((port = atoi(argv[pos])) <= 1024) {
	    log_fatal("Illegal port #\n");
	    proper_exit(MUD_HALT);
	}
    }

    Uptime = time(0);
    mud_port = port;
    log_boot("Running game on port %d.", port);

    if (chdir(dir) < 0) {
	log_fatal("Cannot change directory to %s", dir);
	proper_exit(MUD_HALT);
    }
    log_boot("Using %s as data directory.", dir);

    if (pidfile) {
	FILE                                   *pidfp = NULL;

	if (!(pidfp = fopen(pidfile, "w"))) {
	    log_fatal("Cannot open PID file %s", pidfile);
	    proper_exit(MUD_HALT);
	}
	fprintf(pidfp, "%d\n", getpid());
	fclose(pidfp);
	log_boot("PID written to %s", pidfile);
    }

    if (logfile) {
	log_boot("Switching to %s as stderr.", logfile);
	stderr = freopen(logfile, "a", stderr);
	if (!stderr) {
	    log_fatal("Cannot reopen stderr!");
	    proper_exit(MUD_HALT);
	}
	close(fileno(stdout));
	dup2(fileno(stderr), fileno(stdout));
	log_boot("Switch to %s completed.", logfile);
    }

    srandom(time(0));

#if 0
    if (init_sql()) {
	log_boot("Connected to database!");
	log_boot("%s\n", version_sql());
	log_boot("Disconnecting from database!");
	close_sql();
    } else {
	log_fatal("%s\n", "Couldn't open Database Connection!  Aborting!");
	return MUD_HALT;
    }
#endif

    exit_code = run_the_game(port);
    return exit_code;
}

/* Init sockets, run game, and cleanup sockets */
int run_the_game(int port)
{
    int                                     s = 0;

    if (DEBUG > 1)
	log_info("called %s with %d", __PRETTY_FUNCTION__, port);

    descriptor_list = NULL;

    log_boot("Signal trapping.");
    signal_setup();

    log_boot("Opening mother connection.");
    s = init_socket(port);
    load_db();

    log_boot("Opening WHO port.");
    //init_whod(port);

#ifdef I3
    log_boot("Opening I3 connection.");
    i3_startup(FALSE, 3000, FALSE);
#endif
#ifdef IMC
    log_boot("Opening IMC2 connection.");
    imc_startup(FALSE, -1, FALSE);
#endif

    log_boot("Entering game loop.");
    game_loop(s);

#ifdef IMC
    imc_shutdown(FALSE);
#endif
#ifdef I3
    i3_shutdown(0, NULL);
#endif
    close_sockets(s);
    //close_whod();

    unload_db();
    if (diku_reboot) {
	log_boot("Rebooting.");
	return MUD_REBOOT;
    }
    log_boot("Normal termination of game.");
    return MUD_HALT;					       /* what's so great about HHGTTG, anyhow? */
}

void emit_prompt(struct descriptor_data *point)
{
    char                                    promptbuf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    struct char_data                       *mount = NULL;
    struct room_data                       *rm = NULL;
    time_t                                  tc = (time_t) 0;
    struct tm                              *t_info = NULL;

    if (point->str) {
        write_to_descriptor(point->descriptor, "] ");
    } else if (!point->connected) {
        if (point->page_first) {
            if (point->prompt_mode) {
                point->prompt_mode = 0;
                write_to_descriptor(point->descriptor, "\r\n*** Press return or q ***");
            }
        } else {
            if (point->prompt_mode) {
                point->prompt_mode = 0;
                bzero(promptbuf, MAX_INPUT_LENGTH);
                if (IS_IMMORTAL(point->character) && IS_PC(point->character)) {
                    if (MOUNTED(point->character)) {
                        mount = MOUNTED(point->character);
                        snprintf(promptbuf, MAX_INPUT_LENGTH, "[%s has %d/%dh %d/%dv]\r\n",
                                GET_SDESC(mount),
                                GET_HIT(mount), GET_MAX_HIT(mount),
                                GET_MOVE(mount), GET_MAX_MOVE(mount));
                    }
                    if (IS_SET(point->character->specials.act, PLR_STEALTH))
                        scprintf(promptbuf, MAX_INPUT_LENGTH, "S");
                    if (point->character->invis_level > 0)
                        scprintf(promptbuf, MAX_INPUT_LENGTH, "I=%d: ",
                                point->character->invis_level);
                    rm = real_roomp(point->character->in_room);
                    tc = time(0);
                    t_info = localtime(&tc);
                    scprintf(promptbuf, MAX_INPUT_LENGTH,
                            "%02d:%02d #%d - %s [#%d]> ", t_info->tm_hour, t_info->tm_min, rm->zone, zone_table[rm->zone].name,
                            rm->number);
                    scprintf(promptbuf, MAX_INPUT_LENGTH, "%c%c", TELNET_IAC, TELNET_GA);
                    write_to_descriptor(point->descriptor, promptbuf);
                /* OLD mobs didn't have classes.. this doesn't work anymore */
                } else if (IS_NPC(point->character) &&
                           (IS_SET(point->character->specials.act, ACT_POLYSELF) ||
                            IS_SET(point->character->specials.act, ACT_POLYOTHER))) {
                    snprintf(promptbuf, MAX_INPUT_LENGTH, "P %d/%dh %d/%dv > ",
                            GET_HIT(point->character),
                            GET_MAX_HIT(point->character),
                            GET_MOVE(point->character), GET_MAX_MOVE(point->character));
                    scprintf(promptbuf, MAX_INPUT_LENGTH, "%c%c", TELNET_IAC, TELNET_GA);
                    write_to_descriptor(point->descriptor, promptbuf);
                } else if (IS_NPC(point->character) &&
                           IS_SET(point->character->specials.act, ACT_SWITCH)) {
                    snprintf(promptbuf, MAX_INPUT_LENGTH, "*%s[#%d] in [#%d] %d/%dh %d/%dm %d/%dv > ",
                            NAME(point->character),
                            MobVnum(point->character),
                            point->character->in_room,
                            GET_HIT(point->character),
                            GET_MAX_HIT(point->character),
                            GET_MANA(point->character),
                            GET_MAX_MANA(point->character),
                            GET_MOVE(point->character), GET_MAX_MOVE(point->character));
                    scprintf(promptbuf, MAX_INPUT_LENGTH, "%c%c", TELNET_IAC, TELNET_GA);
                    write_to_descriptor(point->descriptor, promptbuf);
                } else {
                    if (MOUNTED(point->character)) {
                        if (HasClass(point->character, CLASS_RANGER) ||
                            IS_AFFECTED(MOUNTED(point->character), AFF_CHARM)) {
                            mount = MOUNTED(point->character);
                            snprintf(promptbuf, MAX_INPUT_LENGTH, "[%s has %d/%dh %d/%dv]\r\n",
                                    GET_SDESC(mount),
                                    GET_HIT(mount), GET_MAX_HIT(mount),
                                    GET_MOVE(mount), GET_MAX_MOVE(mount));
                        }
                    }
                    scprintf(promptbuf, MAX_INPUT_LENGTH, "%d/%dh %d/%dm %d/%dv > ",
                            GET_HIT(point->character),
                            GET_MAX_HIT(point->character),
                            GET_MANA(point->character),
                            GET_MAX_MANA(point->character),
                            GET_MOVE(point->character), GET_MAX_MOVE(point->character));
                    scprintf(promptbuf, MAX_INPUT_LENGTH, "%c%c", TELNET_IAC, TELNET_GA);
                    write_to_descriptor(point->descriptor, promptbuf);
                }
            }
        }
    }
}

/* Accept new connects, relay commands, and call 'heartbeat-functs' */
void game_loop(int s)
{
    fd_set                                  input_set;
    fd_set                                  output_set;
    fd_set                                  exc_set;
    struct timeval                          last_time;
    struct timeval                          now;
    struct timeval                          timespent;
    struct timeval                          timeout;
    struct timeval                          null_time;
    static struct timeval                   opt_time;
    char                                    comm[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    struct descriptor_data                 *point = NULL;
    struct descriptor_data                 *next_point = NULL;
    int                                     input_flag = 0;

    /*
     * int mask = 0; 
     */
    sigset_t                                mask;

    // int sql_reconnect;

    if (DEBUG > 1)
	log_info("called %s with %d", __PRETTY_FUNCTION__, s);

    pulse = 0;
    null_time.tv_sec = 0;
    null_time.tv_usec = 0;

    opt_time.tv_usec = OPT_USEC;			       /* Init time values */
    opt_time.tv_sec = 0;

    gettimeofday(&last_time, (struct timezone *)0);
    maxdesc = s;
    avail_descs = getdtablesize() - 2;			       /* !! Change if more needed !! */

/*
  mask = sigmask(SIGUSR1) | sigmask(SIGUSR2) | sigmask(SIGINT) |
    sigmask(SIGPIPE) | sigmask(SIGALRM) | sigmask(SIGTERM) |
    sigmask(SIGURG) | sigmask(SIGXCPU) | sigmask(SIGHUP);
*/
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGPIPE);
    sigaddset(&mask, SIGALRM);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGHUP);
    sigaddset(&mask, SIGURG);
    sigaddset(&mask, SIGXCPU);

    /*
     * Main loop 
     */
    while (!diku_shutdown) {
	FD_ZERO(&input_set);
	FD_ZERO(&output_set);
	FD_ZERO(&exc_set);
	FD_SET(s, &input_set);
	for (point = descriptor_list; point; point = point->next) {
	    if (point->descriptor != 0) {
		FD_SET(point->descriptor, &input_set);
		FD_SET(point->descriptor, &exc_set);
		FD_SET(point->descriptor, &output_set);
	    }
	}

	gettimeofday(&now, (struct timezone *)0);
	timespent = timediff(&now, &last_time);
	timeout = timediff(&opt_time, &timespent);
	last_time.tv_sec = now.tv_sec + timeout.tv_sec;
	last_time.tv_usec = now.tv_usec + timeout.tv_usec;
	if (last_time.tv_usec >= 1000000) {
	    last_time.tv_usec -= 1000000;
	    last_time.tv_sec++;
	}

	/*
	 * sigsetmask(mask); 
	 */
	sigprocmask(SIG_BLOCK, &mask, NULL);
	//whod_loop();
	if (select(maxdesc + 1, &input_set, &output_set, &exc_set, &null_time) < 0) {
	    log_error("Select poll");
	    return;
	}
	if (select(0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, &timeout) < 0) {
	    log_error("Select sleep");
	    /*
	     * proper_exit(MUD_HALT); 
	     */
	}
	/*
	 * sigsetmask(0); 
	 */
	sigprocmask(SIG_UNBLOCK, &mask, NULL);

	/*
	 * Respond to whatever might be happening 
	 */

	/*
	 * New connection? 
	 */
	if (FD_ISSET(s, &input_set))
	    if (new_descriptor(s) < 0)
		log_info("New connection");

	/*
	 * kick out the freaky folks 
	 */
	for (point = descriptor_list; point; point = next_point) {
	    next_point = point->next;
	    if (FD_ISSET(point->descriptor, &exc_set)) {
		FD_CLR(point->descriptor, &input_set);
		FD_CLR(point->descriptor, &output_set);
		close_socket(point);
	    }
	}

	for (point = descriptor_list; point; point = next_point) {
	    next_point = point->next;
	    if (FD_ISSET(point->descriptor, &input_set)) {
                do {
                    input_flag = process_input(point);
                    if( input_flag < 0 ) {
                        close_socket(point);
                        break;
                    }
                    if( input_flag > 0 )
		        point->prompt_mode = 1;
                } while( input_flag > 0);
            }
	}

#ifdef I3
	i3_loop();
#endif
#ifdef IMC
	imc_loop();
#endif

	/*
	 * process_commands; 
	 */
	for (point = descriptor_list; point; point = next_to_process) {
	    next_to_process = point->next;
	    if ((--(point->wait) <= 0) && get_from_q(&point->input, comm)) {
		if (point->character && point->connected == CON_PLAYING &&
		    point->character->specials.was_in_room != NOWHERE) {
		    if (point->character->in_room != NOWHERE)
			char_from_room(point->character);
		    char_to_room(point->character, point->character->specials.was_in_room);
		    point->character->specials.was_in_room = NOWHERE;
		    act("$n has returned.", TRUE, point->character, 0, 0, TO_ROOM);
		    log_auth(point->character, "RECONNECTED %s (%s@%s/%s)!",
			     GET_NAME(point->character), point->username, point->host,
			     point->ip);
		}
		point->wait = 1;
		if (point->character)			       /* This updates the idle ticker to say we're not idle */
		    point->character->specials.timer = 0;

		if (point->str)
		    string_add(point, comm);
		else if (!point->connected)
		    if (point->page_first)
			control_page(point, comm);
		    else
			command_interpreter(point->character, comm);
		else
		    nanny(point, comm);
	    }
	}

        /*
         * Process all output.
         */
	for (point = descriptor_list; point; point = next_point) {
	    next_point = point->next;
	    if (point->page_first)
		show_page(point);
	}

	for (point = descriptor_list; point; point = next_point) {
	    next_point = point->next;
	    if (FD_ISSET(point->descriptor, &output_set) && point->output.head) {
                //point->prompt_mode = 1;
		if (process_output(point) < 0)
		    close_socket(point);
	    }
	}

	/*
	 * give the people some prompts 
	 */
        for (point = descriptor_list; point; point = point->next)
            emit_prompt(point);

        /*
         * PULSE handling.... periodic events
         */

	if ((++pulse) > PULSE_MAX)
	    pulse = 0;

	if ((--pulse_zone) <= 0) {
	    pulse_zone = PULSE_ZONE;
	    zone_update();
	}
	if ((--pulse_teleport) <= 0) {
	    pulse_teleport = PULSE_TELEPORT;
	    Teleport(pulse);
	}
	if ((--pulse_nature) <= 0) {
	    pulse_nature = PULSE_NATURE;
	    check_all_nature(pulse);
	}
	if ((--pulse_violence) <= 0) {
	    pulse_violence = PULSE_VIOLENCE;
	    perform_violence(pulse);
	}
	if ((--pulse_mobile) <= 0) {
	    pulse_mobile = PULSE_MOBILE;
	    mobile_activity();
	}
	if ((--pulse_river) <= 0) {
	    pulse_river = PULSE_RIVER;
	    down_river(pulse);
	}
	if ((--pulse_sound) <= 0) {
	    pulse_sound = PULSE_SOUND;
	    MakeSound(pulse);
	}
	if ((--pulse_update) <= 0) {
	    pulse_update = PULSE_UPDATE + number(0, PULSE_VARIABLE);
	    weather_and_time(1);
	    affect_update();
	    point_update(pulse);
	}
	if ((--pulse_reboot) <= 0) {
	    pulse_reboot = PULSE_REBOOT;
	    check_reboot();
	}
	if ((--pulse_dump) <= 0) {
	    pulse_dump = PULSE_DUMP;
	    // dump_player_list();
	}
	if ((--pulse_mudlist) <= 0) {
	    pulse_mudlist = PULSE_MUDLIST;
            generate_mudlist();
	}

	tics++;						       /* tics since last checkpoint signal */
    }
}

/*
 * general utility stuff (for local use)
 */
int get_from_q(struct txt_q *queue, char *dest)
{
    struct txt_block                       *tmp = NULL;

    if (DEBUG > 2)
	log_info("called %s with %08zx, %s", __PRETTY_FUNCTION__, (size_t) queue, VNULL(dest));

    if (!queue) {
	log_info("Input from non-existant queue?");
	return 0;
    }
    if (!queue->head)
	return 0;
    tmp = queue->head;
    strcpy(dest, queue->head->text);
    queue->head = queue->head->next;
    DESTROY(tmp->text);
    DESTROY(tmp);
    return 1;
}

void write_to_q(const char *txt, struct txt_q *queue, int do_timestamp)
{
    struct txt_block                       *new = NULL;

#ifdef TIME_DEBUG
    struct timeval                          now;
    char                                    nowtime[26];
    int                                     buflen;
#endif

    /*
     * Cannot call things in bug.c from things bug.c calls!
     *
     * if (DEBUG > 2)
     *   log_info("called %s with %s, %08zx", __PRETTY_FUNCTION__, VNULL(txt), (size_t)queue);
     */

    if (!queue) {
	log_info("Output message to non-existant queue");
	return;
    }
#ifdef TIME_DEBUG
/* This is purely for debugging timing... don't leave this enabled! */
/* "Wed Jun 30 21:49:08 1993\n" */
/*             ^      ^         */
/*  0          11     18        */
    if (do_timestamp) {
	gettimeofday(&now, (struct timezone *)0);
	ctime_r((time_t *) & (now.tv_sec), nowtime);
    }
#endif

    CREATE(new, struct txt_block, 1);

#ifdef TIME_DEBUG
    if (do_timestamp) {
        buflen = strlen(txt) + 1 + 14;
	CREATE(new->text, char, buflen);

	strlcpy(new->text, nowtime + 11, 8);
	scprintf(new->text, ".%03ld: ", buflen, now.tv_usec / 1000);
	strlcat(new->text, txt, buflen);
    } else {
        buflen = strlen(txt) + 1;
	CREATE(new->text, char, buflen);
	strlcpy(new->text, txt, buflen);
    }
#else
    CREATE(new->text, char, strlen(txt) + 1);
    strlcpy(new->text, txt, strlen(txt) + 1);
#endif

    if (!queue->head) {
	new->next = NULL;
	queue->head = queue->tail = new;
    } else {
	queue->tail->next = new;
	queue->tail = new;
	new->next = NULL;
    }
}

struct timeval timediff(struct timeval *a, struct timeval *b)
{
    struct timeval                          result;
    struct timeval                          tmp;

    if (DEBUG > 3)
	log_info("called %s with %08zx, %08zx", __PRETTY_FUNCTION__, (size_t) a, (size_t) b);

    tmp = *a;

    if ((result.tv_usec = tmp.tv_usec - b->tv_usec) < 0) {
	result.tv_usec += 1000000;
	--(tmp.tv_sec);
    }
    if ((result.tv_sec = tmp.tv_sec - b->tv_sec) < 0) {
	result.tv_usec = 0;
	result.tv_sec = 0;
    }
    return result;
}

/* Empty the queues before closing connection */
void flush_queues(struct descriptor_data *d)
{
    char                                    dummy[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG > 2)
	log_info("called %s with %08zx", __PRETTY_FUNCTION__, (size_t) d);

    while (get_from_q(&d->output, dummy));
    while (get_from_q(&d->input, dummy));
}

/*
 * socket handling
 */

int init_socket(int port)
{
    int                                     opt = 1;
    char                                    hostname[MAX_HOSTNAME + 1] = "\0\0\0\0\0\0\0";
    struct sockaddr_in                      sa;
    struct hostent                         *hp = NULL;
    int                                     s = 0;
    int                                     i = 0;
    int                                     gotsocket = 0;

    if (DEBUG > 2)
	log_info("called %s with %d", __PRETTY_FUNCTION__, port);

    bzero(&sa, sizeof(struct sockaddr_in));

    gethostname(hostname, MAX_HOSTNAME);
    hp = gethostbyname(hostname);
    if (hp == NULL) {
	log_fatal("gethostbyname");
	proper_exit(MUD_HALT);
    }
    sa.sin_family = hp->h_addrtype;
    sa.sin_port = htons(port);
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
	log_fatal("Init-socket");
	proper_exit(MUD_HALT);
    }
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (int *)&opt, sizeof(opt)) < 0) {
	log_fatal("setsockopt REUSEADDR");
	proper_exit(MUD_HALT);
    }

    for (i = 60; i > 0; i--) {
	if (bind(s, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
	    gotsocket = 0;
	    log_info("Socket in use... retrying...\n");
	    sleep(2);
	} else {
	    gotsocket = 1;
	    break;
	}
    }
    if (!gotsocket) {
	close(s);
	log_fatal("bind");
	proper_exit(MUD_HALT);
    }
    listen(s, 3);
    return (s);
}

int new_connection(int s)
{
    struct sockaddr_in                      isa;
    socklen_t                               i = 0;
    int                                     t = 0;

    if (DEBUG > 1)
	log_info("called %s with %d", __PRETTY_FUNCTION__, s);

    i = (socklen_t) sizeof(isa);
    getsockname(s, (struct sockaddr *)&isa, &i);
    if ((t = accept(s, (struct sockaddr *)&isa, &i)) < 0) {
	log_error("Accept");
	return (-1);
    }
    nonblock(t);
    return t;
}

int new_descriptor(int s)
{
    struct sockaddr_in                      isa;
    struct hostent                         *host = NULL;
    struct descriptor_data                 *newd = NULL;
    time_t                                  tc = (time_t) 0;
    struct tm                              *t_info = NULL;
    long                                    remote_addr = 0L;
    socklen_t                               i = 0;

    /*
     * int remote_port = 0; 
     */
    int                                     desc = 0;
    int                                     old_maxdesc = 0;
    int                                     desc_index = 0;

#ifdef RFC1413
    int                                     badger = 0;
#endif

    const char                             *timed_con[] = {
	"\n"
    };

/*
  const char                             *bannished[] = {
    "\n"
  };
*/

    if (DEBUG > 2)
	log_info("called %s with %d", __PRETTY_FUNCTION__, s);

    tc = time(0);
    t_info = localtime(&tc);

    old_maxdesc = maxdesc;

    i = (socklen_t) sizeof(isa);
    getsockname(s, (struct sockaddr *)&isa, &i);
    if ((desc = accept(s, (struct sockaddr *)&isa, &i)) < 0) {
	log_error("Accept");
	return (-1);
    }
    nonblock(desc);

    if ((maxdesc + 1) >= avail_descs) {
	write_to_descriptor(desc, "Sorry.. full...\r\n");
	close(desc);
	return (0);
    } else if (desc > maxdesc)
	maxdesc = desc;

    CREATE(newd, struct descriptor_data, 1);

    /*
     * remote_port = ntohs(isa.sin_port); 
     */
    remote_addr = htonl(isa.sin_addr.s_addr);

    newd->username[0] = '\0';
#ifdef RFC1413
    {
	char                                   *ack = NULL;

	if ((ack = ident_id(desc, 10)))
	    strlcpy(newd->username, ack, 17);
	else
	    badger = 2;
    }
#endif
    if (!newd->username[0])
	strlcpy(newd->username, "adork", 17);

    snprintf(newd->ip, 20, "%lu.%lu.%lu.%lu",
	    (remote_addr & 0xff000000) >> 24,
	    (remote_addr & 0x00ff0000) >> 16,
	    (remote_addr & 0x0000ff00) >> 8, (remote_addr & 0x000000ff) >> 0);

    if ((host = gethostbyaddr((char *)&isa.sin_addr, sizeof(isa.sin_addr), AF_INET)))
	strlcpy(newd->host, host->h_name, 50);

    /*
     * init desc data 
     */
    newd->descriptor = desc;
    newd->connected = 1;
    newd->wait = 1;
    newd->prompt_mode = 1;
    *newd->buf = '\0';
    newd->str = 0;
    newd->showstr_head = 0;
    newd->showstr_point = 0;
    newd->page_first = NULL;
    newd->page_last = NULL;
    newd->page_control = ' ';				       /* show the first page! */
    *newd->last_input = '\0';
    newd->output.head = NULL;
    newd->input.head = NULL;
    newd->next = descriptor_list;
    newd->character = 0;
    newd->original = 0;
    newd->snoop.snooping = 0;
    newd->snoop.snoop_by = 0;

#ifdef RFC1413
    switch (badger) {
	case 1:
	    write_to_descriptor(desc,
				"\r\n **** Tell your sys-admin to upgrade to the SHINY NEW telnet using RFC1413 ****\r\n\r\n");
	    break;
	case 2:
	    write_to_descriptor(desc,
				"\r\n **** Tell your sys-admin to practice safe telnet by using RFC1413 ****\r\n\r\n");
	    break;
	default:
	    break;
    }
#endif

    /*
     * prepend to list 
     */

    if (((t_info->tm_hour + 1) > 8) && ((t_info->tm_hour + 1) < 21))
	for (desc_index = 0; strcmp(timed_con[desc_index], "\n"); desc_index++) {
	    if (!strncmp(timed_con[desc_index], newd->ip, 19)) {
		log_info("TIMED site connecting:%s\n", newd->ip);
		dcprintf(newd, "\r\nThis site is blocked from : 9 am - 9 pm\r\n");
		dcprintf(newd, "You may connect after 9 pm from :[%s]\r\n", newd->ip);
		maxdesc = old_maxdesc;
		DESTROY(newd);
		close(desc);
		return (0);
	    }
	}
    /*
     * for (desc_index = 0; bannished[desc_index] != "\n"; desc_index++) { 
     */
    /*
     * if (!strncmp(bannished[desc_index], newd->ip, 19)) 
     */
    if (banned_ip(newd->ip)) {
	log_info("BAN site connecting:%s\n", newd->ip);
	dcprintf(newd, "\r\nDue to your System Administrators request, or for some\r\n");
	dcprintf(newd, "other reason, we are refusing all connections from:[%s]\r\n", newd->ip);
	maxdesc = old_maxdesc;
	DESTROY(newd);
	close(desc);
	return (0);
    }
    /*
     * } 
     */

    descriptor_list = newd;
    SEND_TO_Q(greetings, newd);
    SEND_TO_Q("By what name do you wish to be known? ", newd);
    return (0);
}

int process_output(struct descriptor_data *t)
{
    char                                    i[MAX_STRING_LENGTH + 1] = "\0\0\0\0\0\0\0";

    if (DEBUG > 2)
	log_info("called %s with %08zx", __PRETTY_FUNCTION__, (size_t) t);

    //if (!t->prompt_mode && !t->connected)
    if (t->prompt_mode && !t->connected)
	if (write_to_descriptor(t->descriptor, "\r\n") < 0)
	    return (-1);

    /*
     * Cycle thru output queue 
     */
    while (get_from_q(&t->output, i)) {
	if ((t->snoop.snoop_by) && (t->snoop.snoop_by->desc)) {
	    write_to_q("S* ", &t->snoop.snoop_by->desc->output, 1);
	    write_to_q(i, &t->snoop.snoop_by->desc->output, 0);
	}
	if (write_to_descriptor(t->descriptor, i) < 0)
	    return (-1);
    }

    if (!t->connected && !(t->character && !IS_NPC(t->character) &&
			   IS_SET(t->character->specials.act, PLR_COMPACT))) {
	if (write_to_descriptor(t->descriptor, "\r\n") < 0)
	    return (-1);
    }

    return (1);
}

int write_to_descriptor(int desc, const char *txt)
{
    int                                     sofar = 0;
    int                                     thisround = 0;
    int                                     total = 0;

    if (DEBUG > 2)
	log_info("called %s with %d, %s", __PRETTY_FUNCTION__, desc, VNULL(txt));

    total = strlen(txt);

    do {
	thisround = write(desc, txt + sofar, total - sofar);
	if (thisround < 0) {
	    if (errno == EWOULDBLOCK)
		break;
	    log_error("Write to socket");
	    return (-1);
	}
	sofar += thisround;
    }
    while (sofar < total);

    return (0);
}

int process_input(struct descriptor_data *t)
{
    int                                     i = 0;
    int                                     j = 0;
    int                                     in_iac = 0;
    int                                     in_sub = 0;
    int                                     begin = 0;
    int                                     sofar = 0;
    int                                     thisround = 0;
    char                                    *line_marker = NULL;
    char                                    buffer[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    char                                    tmp[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";

    if (DEBUG > 2)
	log_info("called %s with %08zx", __PRETTY_FUNCTION__, (size_t) t);

    begin = strlen(t->buf);

    do {
        thisround = read(t->descriptor, t->buf + begin + sofar, MAX_STRING_LENGTH - (begin + sofar) - 1); // Leave an extra byte for two-character newline processing.
        if( thisround > 0 ) {
            sofar += thisround;
        } else if( thisround < 0 ) {
            if (errno != EWOULDBLOCK) {
                log_error("Socket READ error.");
                return( -1 );
            } else {
                // Blocking means no more data available, yet.
                break;
            }
        } else if( thisround == 0 ) {
            log_error("EOF on socket read.");
            return( -1 );
        }
    } while(1);
    // } while(!ISNEWL( *( t->buf + begin + sofar - 1) ));
    // If we find the end of line, we're done for now, even if there is more to read.

    *(t->buf + begin + sofar) = 0; // Add NUL to end of newly extended string

    //if( !(line_marker = strpbrk((t->buf + begin), "\r\n")) ) {
    if( !(line_marker = strpbrk((t->buf), "\r\n")) ) {
        // If no newline (or partial newline) was found, process next time.
        return( 0 );
    }

    // At this point we have valid input to process.

    // Copy up to (but not including) the first newline marker, and NUL the position it would be in.
    strncpy(buffer, t->buf, line_marker - t->buf);
    buffer[line_marker - t->buf] = '\0';
    while( *line_marker && ISNEWL(*line_marker) ) {
        line_marker++; // Skip newline characters, however many there are.
    }
    strlcpy(tmp, line_marker, MAX_STRING_LENGTH); // Copy unprocessed buffer aside.

    if( strlen(buffer) > MAX_INPUT_LENGTH - 1) {
        buffer[MAX_INPUT_LENGTH - 1] = '\0';
        // report the truncated line to the user.
        if( write_to_descriptor(t->descriptor, "Line too long.  Truncated to:\r\n") >= 0 ) {
            write_to_descriptor(t->descriptor, buffer);
            write_to_descriptor(t->descriptor, "\r\n");
        } else {
            return( -1 );
        }
    }

    // Move the unprocessed content to the front of t->buf for future processing.
    strlcpy(t->buf, tmp, MAX_STRING_LENGTH);

    // At this point, we want to strip out any TELNET sequences, since we don't support them.
    for(i = 0, j = 0; i < strlen(buffer);) {
        if( !in_iac ) {
            if( (unsigned char) buffer[i] != TELNET_IAC ) {
                // Normal processing
                tmp[j] = buffer[i];
                i++;
                j++;
            } else {
                in_iac = 1;
                i++;
            }
        } else {
            if( in_sub ) {
                if( (unsigned char) buffer[i] == TELNET_SE ) {
                    // done!
                    i++;
                    in_sub = 0;
                    in_iac = 0;
                } else {
                    // Part of the SB sub... skip it all.
                    i++;
                }
            } else {
                switch( (unsigned char) buffer[i] ) {
                    case TELNET_SB:
                        // Sub-negotiation
                        i++;
                        in_sub = 1;
                    case TELNET_WILL:
                    case TELNET_WONT:
                    case TELNET_DO:
                    case TELNET_DONT:
                        i++;            // Skip the OP
                        i++;            // Skip whatever it tried to do or ask
                        in_iac = 0;     // done
                        break;
                    case TELNET_IAC:
                        in_iac = 0;     // Double IAC, escaped!
                    default:
                        // Unknown
                        tmp[j] = buffer[i];
                        i++;
                        j++;
                }
            }
        }
    }
    tmp[j] = '\0';

    if( *tmp == '!' ) {
        // This means do the last command again, so... swap!
        strlcpy(tmp, t->last_input, MAX_INPUT_LENGTH);
    } else {
        // Save an extra copy as our last command.
        strlcpy(t->last_input, tmp, MAX_INPUT_LENGTH);
    }
    // Send to actual command queue for processing.
    write_to_q(tmp, &t->input, 0);
    t->idle_time = time(0);

    if( (t->snoop.snoop_by) && (t->snoop.snoop_by->desc) ) {
        // If we're being snooped, send our input to the snooper.
        write_to_q("% ", &t->snoop.snoop_by->desc->output, 1);
        write_to_q(tmp, &t->snoop.snoop_by->desc->output, 0);
        write_to_q("\r\n", &t->snoop.snoop_by->desc->output, 0);
    }

    // Debugging... turn it off unless there are... issues.
    //for(i = 0; i < strlen(tmp); i++) {
    //    if( !isprint( tmp[i] ) ) {
    //        log_info("Tossing character %02.2X \"%c\"", tmp[i], tmp[i]);
    //        log_info("Tossed from \"%s\"", tmp);
    //    }
    //}

    return( 1 );
}

int old_process_input(struct descriptor_data *t)
{
    int                                     sofar = 0;
    int                                     thisround = 0;
    int                                     begin = 0;
    int                                     squelch = 0;
    int                                     i = 0;
    int                                     k = 0;
    int                                     flag = FALSE;
    long                                    now_time = 0L;
    char                                    tmp[MAX_INPUT_LENGTH + 2] = "\0\0\0\0\0\0\0";
    char                                    buffer[MAX_INPUT_LENGTH + 60] = "\0\0\0\0\0\0\0";

    if (DEBUG > 2)
	log_info("called %s with %08zx", __PRETTY_FUNCTION__, (size_t) t);

    begin = strlen(t->buf);

    /*
     * Read in some stuff 
     */
    do {
	if ((thisround = read(t->descriptor, t->buf + begin + sofar,
			      MAX_STRING_LENGTH - (begin + sofar) - 1)) > 0) {
	    sofar += thisround;
	} else {
	    if (thisround < 0) {
		if (errno != EWOULDBLOCK) {
		    log_error("Read1 - ERROR");
		    return (-1);
		} else {
		    break;
		}
	    } else {
		log_info("EOF encountered on socket read.");
		return (-1);
	    }
	}
    } while (!ISNEWL(*(t->buf + begin + sofar - 1)));

    *(t->buf + begin + sofar) = 0;

    /*
     * if no newline is contained in input, return without proc'ing 
     */
    for (i = begin; !ISNEWL(*(t->buf + i)); i++)
	if (!*(t->buf + i))
	    return (0);

    /*
     * input contains 1 or more newlines; process the stuff 
     */
    for (i = 0, k = 0; *(t->buf + i);) {
	if (!ISNEWL(*(t->buf + i)) && !(flag = (k >= (MAX_INPUT_LENGTH - 2))))
	    if (*(t->buf + i) == '\b') {		       /* backspace */
		if (k) {				       /* more than one char ? */
#if 0
		    if (*(tmp + --k) == '$')
			k--;
#endif
		    i++;
		} else {
		    i++;				       /* no or just one char.. Skip backsp */
		}
	    } else {
		/* if (isascii(*(t->buf + i)) && isprint(*(t->buf + i))) { */
		if (isprint(*(t->buf + i))) {
		    /*
		     * trans char, double for '$' (printf)        
		     */

		    /* if ((*(tmp + k) = *(t->buf + i)) == '$') */

		    *(tmp + k) = *(t->buf + i);
#if 0
		    if ((*(tmp + k)) == '$')
                        *(tmp + ++k) = '$';
#endif
		    k++;
		    i++;
		} else {
	            log_info("Tossing character %02.2X \"%c\"", *(t->buf + i), *(t->buf + i));
		    i++;
		}
	} else {
	    *(tmp + k) = 0;
	    if (*tmp == '!')
		strlcpy(tmp, t->last_input, MAX_INPUT_LENGTH);
	    else
		strlcpy(t->last_input, tmp, MAX_INPUT_LENGTH);

	    write_to_q(tmp, &t->input, 0);

	    now_time = time(0);
	    t->idle_time = now_time;
	    if ((t->snoop.snoop_by) && (t->snoop.snoop_by->desc)) {
		write_to_q("% ", &t->snoop.snoop_by->desc->output, 1);
		write_to_q(tmp, &t->snoop.snoop_by->desc->output, 0);
		write_to_q("\r\n", &t->snoop.snoop_by->desc->output, 0);
	    }
	    if (flag) {
		snprintf(buffer, MAX_INPUT_LENGTH + 60, "Line too long. Truncated to:\r\n%s\r\n", tmp);
		if (write_to_descriptor(t->descriptor, buffer) < 0)
		    return (-1);

		/*
		 * skip the rest of the line 
		 */
		for (; !ISNEWL(*(t->buf + i)); i++);
	    }
	    /*
	     * find end of entry 
	     */
	    for (; ISNEWL(*(t->buf + i)); i++);

	    /*
	     * squelch the entry from the buffer 
	     */
	    for (squelch = 0;; squelch++)
		if ((*(t->buf + squelch) = *(t->buf + i + squelch)) == '\0')
		    break;
	    k = 0;
	    i = 0;
	}
    }
    return (1);
}

void close_sockets(int s)
{
    if (DEBUG > 2)
	log_info("called %s with %d", __PRETTY_FUNCTION__, s);

    log_info("Closing all sockets.");
    while (descriptor_list)
	close_socket(descriptor_list);
    close(s);
}

void close_socket(struct descriptor_data *d)
{
    struct descriptor_data                 *tmp = NULL;

    if (DEBUG > 2)
	log_info("called %s with %08zx", __PRETTY_FUNCTION__, (size_t) d);

    if (!d)
	return;

    close(d->descriptor);
    flush_queues(d);
    if (d->descriptor == maxdesc)
	--maxdesc;

    /*
     * Forget snooping 
     */

    if (d->snoop.snooping)
	d->snoop.snooping->desc->snoop.snoop_by = 0;

    if (d->snoop.snoop_by) {
	cprintf(d->snoop.snoop_by, "Your victim is no longer among us.\r\n");
	d->snoop.snoop_by->desc->snoop.snooping = 0;
    }
    if (d->character)
	if (d->connected == CON_PLAYING) {
	    do_save(d->character, "", 0);
	    act("$n has lost $s link.", TRUE, d->character, 0, 0, TO_ROOM);
	    /*
	     * log_info("Closing link to: %s.", GET_NAME(d->character)); 
	     */
	    log_auth(d->character, "LINKDEAD %s (%s@%s/%s)!", GET_NAME(d->character),
		     d->username, d->host, d->ip);
	    if (IS_NPC(d->character)) {
		if (d->character->desc)
		    d->character->orig = d->character->desc->original;
	    }
	    d->character->desc = NULL;
	} else {
	    if (GET_NAME(d->character)) {
		/*
		 * log_info("Losing player: %s.", GET_NAME(d->character)); 
		 */
		log_auth(d->character, "GOODBYE %s (%s@%s/%s)!", GET_NAME(d->character),
			 d->username, d->host, d->ip);
	    }
	    free_char(d->character);
	    d->character = NULL;			       /* need to wipe this out so we don't pick at it! */
    } else
	log_info("Losing descriptor without char.");

    if (next_to_process == d)				       /* to avoid crashing the process loop */
	next_to_process = next_to_process->next;

    if (d == descriptor_list)				       /* this is the head of the list */
	descriptor_list = descriptor_list->next;
    else {						       /* This is somewhere inside the list */
	/*
	 * Locate the previous element 
	 */
	for (tmp = descriptor_list; (tmp->next != d) && tmp; tmp = tmp->next);

	tmp->next = d->next;
    }
    /*
     * if (d->showstr_head) 
     */
    DESTROY(d->showstr_head);
    DESTROY(d);
}

void nonblock(int s)
{
    if (DEBUG > 2)
	log_info("called %s with %d", __PRETTY_FUNCTION__, s);

    if (fcntl(s, F_SETFL, O_NDELAY) == -1) {
	log_fatal("Noblock");
	proper_exit(MUD_HALT);
    }
}

/*
 * Public routines for system-to-player-communication
 */

/*
 * This acts as an interface to write_to_q(), but it uses variable arguments
 * to eliminate multiple calls to sprintf().
 */
void dcprintf(struct descriptor_data *d, const char *Str, ...)
{
    va_list                                 arg;
    char                                    Result[MAX_STRING_LENGTH];

    if (Str && *Str && d) {
	bzero(Result, MAX_STRING_LENGTH);
	va_start(arg, Str);
	vsnprintf(Result, MAX_STRING_LENGTH, Str, arg);
	va_end(arg);
	write_to_q(Result, &d->output, 1);
	if (DEBUG > 2)
	    log_info("called %s with %08zx, %s, result of %s", __PRETTY_FUNCTION__, (size_t) d,
		     VNULL(Str), Result);
    }
}

/*
 * This works like send_to_char(), but it uses variable arguments to
 * eliminate multiple calls to sprintf().
 */
void cprintf(struct char_data *ch, const char *Str, ...)
{
    va_list                                 arg;
    char                                    Result[MAX_STRING_LENGTH];

    if (Str && *Str && ch && ch->desc) {
	bzero(Result, MAX_STRING_LENGTH);
	va_start(arg, Str);
	vsnprintf(Result, MAX_STRING_LENGTH, Str, arg);
	va_end(arg);
	write_to_q(Result, &ch->desc->output, 1);
	/*
	 * Cannot call things in bug.c from things bug.c calls!
	 *
	 * if (DEBUG > 2)
	 *   log_info("called %s with %s, %s, result of %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(Str), Result);
	 */
    }
}

/*
 * This one is an interface to replace send_to_room().
 */
void rprintf(int room, const char *Str, ...)
{
    va_list                                 arg;
    char                                    Result[MAX_STRING_LENGTH];
    struct char_data                       *i = NULL;
    struct room_data                       *rr = NULL;

    if (Str && *Str && room >= 0 && (rr = real_roomp(room))) {
	bzero(Result, MAX_STRING_LENGTH);
	va_start(arg, Str);
	vsnprintf(Result, MAX_STRING_LENGTH, Str, arg);
	va_end(arg);
	for (i = rr->people; i; i = i->next_in_room)
	    if (i->desc)
		write_to_q(Result, &i->desc->output, 1);
	if (DEBUG > 2)
	    log_info("called %s with %d, %s, result of %s", __PRETTY_FUNCTION__, room,
		     VNULL(Str), Result);
    }
}

/*
 * This one is everyone in the zone specified.
 */
void zprintf(int zone, const char *Str, ...)
{
    va_list                                 arg;
    char                                    Result[MAX_STRING_LENGTH];
    struct descriptor_data                 *i = NULL;
    struct room_data                       *rr = NULL;

    if (Str && *Str && zone >= 0) {
	bzero(Result, MAX_STRING_LENGTH);
	va_start(arg, Str);
	vsnprintf(Result, MAX_STRING_LENGTH, Str, arg);
	va_end(arg);
	for (i = descriptor_list; i; i = i->next)
	    if (!i->connected)
		if (i->character)
		    if ((rr = real_roomp(i->character->in_room)))
			if (rr->zone == zone)
			    write_to_q(Result, &i->output, 1);
	if (DEBUG > 2)
	    log_info("called %s with %d, %s, result of %s", __PRETTY_FUNCTION__, zone,
		     VNULL(Str), Result);
    }
}

/*
 * And this one sends to EVERYBODY int the game!!!!!
 */
void allprintf(const char *Str, ...)
{
    va_list                                 arg;
    char                                    Result[MAX_STRING_LENGTH];
    struct descriptor_data                 *i = NULL;

    if (Str && *Str) {
	bzero(Result, MAX_STRING_LENGTH);
	va_start(arg, Str);
	vsnprintf(Result, MAX_STRING_LENGTH, Str, arg);
	va_end(arg);
	for (i = descriptor_list; i; i = i->next)
	    if (!i->connected)
		write_to_q(Result, &i->output, 1);
	if (DEBUG > 2)
	    log_info("called %s with %s, result of %s", __PRETTY_FUNCTION__, VNULL(Str),
		     Result);
    }
}

/*
 * Here is send_to_outdoor()
 */
void oprintf(const char *Str, ...)
{
    va_list                                 arg;
    char                                    Result[MAX_STRING_LENGTH];
    struct descriptor_data                 *i = NULL;

    if (Str && *Str) {
	bzero(Result, MAX_STRING_LENGTH);
	va_start(arg, Str);
	vsnprintf(Result, MAX_STRING_LENGTH, Str, arg);
	va_end(arg);
	for (i = descriptor_list; i; i = i->next)
	    if (!i->connected && i->character && OUTSIDE(i->character))
		write_to_q(Result, &i->output, 1);
	if (DEBUG > 2)
	    log_info("called %s with %s, result of %s", __PRETTY_FUNCTION__, VNULL(Str),
		     Result);
    }
}

/*
 * Send to everyone except the given character.
 */
void eprintf(struct char_data *ch, const char *Str, ...)
{
    va_list                                 arg;
    char                                    Result[MAX_STRING_LENGTH];
    struct descriptor_data                 *i = NULL;

    if (Str && *Str) {
	bzero(Result, MAX_STRING_LENGTH);
	va_start(arg, Str);
	vsnprintf(Result, MAX_STRING_LENGTH, Str, arg);
	va_end(arg);
	for (i = descriptor_list; i; i = i->next)
	    if (ch && ch->desc != i && !i->connected)
		write_to_q(Result, &i->output, 1);
	if (DEBUG > 2)
	    log_info("called %s with %s, %s, result of %s", __PRETTY_FUNCTION__, SAFE_NAME(ch),
		     VNULL(Str), Result);
    }
}

/*
 * This one is for send_to_room_except()
 */
void reprintf(int room, struct char_data *ch, const char *Str, ...)
{
    va_list                                 arg;
    char                                    Result[MAX_STRING_LENGTH];
    struct char_data                       *i = NULL;
    struct room_data                       *rr = NULL;

    if (Str && *Str && room >= 0 && (rr = real_roomp(room))) {
	bzero(Result, MAX_STRING_LENGTH);
	va_start(arg, Str);
	vsnprintf(Result, MAX_STRING_LENGTH, Str, arg);
	va_end(arg);
	for (i = rr->people; i; i = i->next_in_room)
	    if (i != ch && i->desc)
		write_to_q(Result, &i->desc->output, 1);
	if (DEBUG > 2)
	    log_info("called %s with %d, %s, %s, result of %s", __PRETTY_FUNCTION__, room,
		     SAFE_NAME(ch), VNULL(Str), Result);
    }
}

/*
 * This one is for send_to_room_except()
 */
void re2printf(int room, struct char_data *ch1, struct char_data *ch2, const char *Str, ...)
{
    va_list                                 arg;
    char                                    Result[MAX_STRING_LENGTH];
    struct char_data                       *i = NULL;
    struct room_data                       *rr = NULL;

    if (Str && *Str && room >= 0 && (rr = real_roomp(room))) {
	bzero(Result, MAX_STRING_LENGTH);
	va_start(arg, Str);
	vsnprintf(Result, MAX_STRING_LENGTH, Str, arg);
	va_end(arg);
	for (i = rr->people; i; i = i->next_in_room)
	    if (i != ch1 && i != ch2 && i->desc)
		write_to_q(Result, &i->desc->output, 1);
	if (DEBUG > 2)
	    log_info("called %s with %d, %s, %s, %s, result of %s", __PRETTY_FUNCTION__, room,
		     SAFE_NAME(ch1), SAFE_NAME(ch2), VNULL(Str), Result);
    }
}

/*
 * IMMORTAL printf.
 */
void iprintf(const char *Str, ...)
{
    va_list                                 arg;
    char                                    Result[MAX_STRING_LENGTH];
    struct descriptor_data                 *i = NULL;

    if (Str && *Str) {
	bzero(Result, MAX_STRING_LENGTH);
	va_start(arg, Str);
	vsnprintf(Result, MAX_STRING_LENGTH, Str, arg);
	va_end(arg);
	for (i = descriptor_list; i; i = i->next)
	    if (!i->connected && i->character && IS_IMMORTAL(i->character))
		write_to_q(Result, &i->output, 1);
	if (DEBUG > 2)
	    log_info("called %s with %s, result of %s", __PRETTY_FUNCTION__, VNULL(Str),
		     Result);
    }
}

void save_all()
{
    struct descriptor_data                 *i = NULL;

    if (DEBUG > 2)
	log_info("called %s with no arguments", __PRETTY_FUNCTION__);

    for (i = descriptor_list; i; i = i->next)
	if (i->character)
	    save_char(i->character, NOWHERE);
}

/* higher-level communication */

void act(const char *Str, int hide_invisible, struct char_data *ch,
	 struct obj_data *obj, void *vict_obj, int type, ...)
{
    char                                   *strp = NULL;
    char                                   *point = NULL;
    const char                             *i = NULL;
    struct char_data                       *to = NULL;
    char                                    buf[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    char                                    str[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    va_list                                 arg;

    if (DEBUG > 2)
	log_info("called %s with %s, %d, %s, %08zx, %08zx, %d", __PRETTY_FUNCTION__, VNULL(Str),
		 hide_invisible, SAFE_NAME(ch), (size_t) obj, (size_t) vict_obj, type);

    if (!Str)
	return;
    if (!*Str)
	return;

    bzero(buf, MAX_STRING_LENGTH);
    bzero(str, MAX_STRING_LENGTH);

    va_start(arg, type);
    vsnprintf(str, MAX_STRING_LENGTH, Str, arg);
    va_end(arg);

    if (DEBUG > 1) {
	log_info("act got: %s", Str);
	log_info("act became: %s", str);
    }

    /*
     * Added checks to ensure ch and to are NOT NULL 
     */
    if (type == TO_VICT) {
	to = (struct char_data *)vict_obj;
	if (!to || !ch)
	    return;
    } else if (type == TO_CHAR) {
	to = ch;
	if (!to)
	    return;
    } else {
	if (!ch)
	    return;
	if (!real_roomp(ch->in_room))
	    return;
	if (!(to = real_roomp(ch->in_room)->people))
	    return;
    }

    for (; to; to = to->next_in_room) {
	if (to->desc && ((to != ch) || (type == TO_CHAR)) &&
	    (CAN_SEE(to, ch) || !hide_invisible) && AWAKE(to) &&
	    !((type == TO_NOTVICT) && (to == (struct char_data *)vict_obj))) {
	    for (strp = str, point = buf;;)
		if (*strp == '$') {
		    switch (*(++strp)) {
			case 'n':
			    i = PERS(ch, to);
			    break;
			case 'N':
			    i = PERS((struct char_data *)vict_obj, to);
			    break;
			case 'm':
			    i = HMHR(ch);
			    break;
			case 'M':
			    i = HMHR((struct char_data *)vict_obj);
			    break;
			case 's':
			    i = HSHR(ch);
			    break;
			case 'S':
			    i = HSHR((struct char_data *)vict_obj);
			    break;
			case 'e':
			    i = HSSH(ch);
			    break;
			case 'E':
			    i = HSSH((struct char_data *)vict_obj);
			    break;
			case 'o':
			    i = OBJN(obj, to);
			    break;
			case 'O':
			    i = OBJN((struct obj_data *)vict_obj, to);
			    break;
			case 'p':
			    i = OBJS(obj, to);
			    break;
			case 'P':
			    i = OBJS((struct obj_data *)vict_obj, to);
			    break;
			case 'a':
			    i = SANA(obj);
			    break;
			case 'A':
			    i = SANA((struct obj_data *)vict_obj);
			    break;
			case 'T':
			    i = (char *)vict_obj;
			    break;
			case 'F':
			    i = fname((char *)vict_obj);
			    break;
			case '$':
			    i = "$";
			    break;
			default:
			    log_info("Illegal $-code to act(): %s", str);
			    break;
		    }

		    while ((*point = *(i++)))
			++point;

		    ++strp;

		} else if (!(*(point++) = *(strp++)))
		    break;

	    *(--point) = '\n';
	    *(++point) = '\r';
	    *(++point) = '\0';

	    write_to_q(CAP(buf), &to->desc->output, 1);
	    if (DEBUG > 1)
		log_info("act sent: %s", buf);
	}
	if ((type == TO_VICT) || (type == TO_CHAR))
	    return;
    }
}

void dump_player_list(void)
{
    FILE                                   *pfd = NULL;
    int                                     i = 0;

    if (DEBUG > 2)
	log_info("called %s with no arguments", __PRETTY_FUNCTION__);

    log_info("Dumping player list");
    if (!(pfd = fopen(PLAYER_FILE, "w"))) {
	log_error("Cannot save player data for new user!");
    } else {
	fprintf(pfd, "%d\n", actual_players);
	for (i = 0; i < number_of_players; i++)
	    if (list_of_players[i])
		fprintf(pfd, "%s", list_of_players[i]);
	FCLOSE(pfd);
    }
}

void proper_exit(int exit_code)
{
#if 0
    log_boot("Disconnecting from database!");
    close_sql();
#endif
    exit(exit_code);
}

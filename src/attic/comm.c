/*
 *  file: comm.c , Communication module.                   Part of DIKUMUD
 *  Usage: Communication, central game loop.
 *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 *  All Rights Reserved
 *  Using *any* part of DikuMud without having read license.doc is
 *  violating our copyright.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/un.h>
#include <sys/resource.h>

#include "include/global.h"
#include "include/bug.h"
#include "include/utils.h"
#include "include/interpreter.h"
#include "include/handler.h"
#include "include/db.h"
#include "include/modify.h"
#include "include/whod.h"
#include "include/multiclass.h"
#include "include/weather.h"
#include "include/limits.h"
#include "include/spell_parser.h"
#include "include/sound.h"
#include "include/fight.h"
#include "include/mob_actions.h"
#define _COMM_C
#include "include/comm.h"

#ifdef RFC1413
#include "libident-0.19/ident.h"
#include "libauth-4.0-p5/authuser.h"
#endif

struct descriptor_data *descriptor_list;
struct descriptor_data *next_to_process;

int mud_port;
int slow_death = 0;	/* Shut her down, Martha, she's sucking mud */
int diku_shutdown = 0;	/* clean shutdown */
int diku_reboot = 0;	/* reboot the game after a shutdown */
int DEBUG = 0;
int DEBUG2 = 0;
int no_specials = 0;	/* Suppress ass. of special routines */
long Uptime;		/* time that the game has been up */

int maxdesc;
int avail_descs;
int tics = 0;			       /* for extern checkpointing */
int pulse = 0;
int pulse_update = PULSE_UPDATE + PULSE_VARIABLE;
int pulse_river = PULSE_RIVER;
int pulse_teleport = PULSE_TELEPORT;
int pulse_nature = PULSE_NATURE;
int pulse_sound = PULSE_SOUND;
int pulse_zone = PULSE_ZONE;
int pulse_mobile = PULSE_MOBILE;
int pulse_violence = PULSE_VIOLENCE;
int pulse_reboot = PULSE_REBOOT;
int pulse_dump = PULSE_DUMP;

int main(int argc, char **argv) {
  int port;
  char buf[512];
  int pos = 1;
  char *dir;

  if (DEBUG)
    dlog("main");
  port = DFLT_PORT;
  dir = DFLT_DIR;
  WizLock = FALSE;

  while ((pos < argc) && (*(argv[pos]) == '-')) {
    switch (*(argv[pos] + 1)) {
    case 'w':
      WizLock = TRUE;
      log("WizLock is SET.");
      break;
    case 'D':
      DEBUG = TRUE;
      log("Debugging is on.");
      break;
    case 'l':
      log("Lawful mode no longer available.");
      break;
    case 'd':
      if (*(argv[pos] + 2))
	dir = argv[pos] + 2;
      else if (++pos < argc)
	dir = argv[pos];
      else {
	log("Directory arg expected after option -d.");
	exit(0);
      }
      break;
    case 's':
      no_specials = 1;
      log("Suppressing assignment of special routines.");
      break;
    default:
      sprintf(buf, "Unknown option -% in argument string.",
	      *(argv[pos] + 1));
      log(buf);
      break;
    }
    pos++;
  }

  if (pos < argc)
    if (!isdigit(*argv[pos])) {
      fprintf(stderr, "Usage: %s [-l] [-s] [-d pathname] [ port # ]\n",
	      argv[0]);
      exit(0);
    } else if ((port = atoi(argv[pos])) <= 1024) {
      printf("Illegal port #\n");
      exit(0);
    }

  Uptime = time(0);
  sprintf(buf, "Running game on port %d.", port);
  mud_port= port;
  log(buf);

  if (chdir(dir) < 0) {
    perror("chdir");
    exit(0);
  }
  sprintf(buf, "Using %s as data directory.", dir);
  log(buf);

  srandom(time(0));
  run_the_game(port);
  return(42);		       /* what's so great about HHGTTG, anyhow? */
}

/* Init sockets, run game, and cleanup sockets */
void run_the_game(int port) {
  int s;
  void signal_setup(void);
  void coma(int);

  descriptor_list = NULL;

  log("Signal trapping.");
  signal_setup();

  log("Opening mother connection.");
  s = init_socket(port);
  boot_db();

  init_whod(port);
  log("Entering game loop.");
  game_loop(s);

  close_sockets(s);
  close_whod();

  if (diku_reboot) {
    log("Rebooting.");
    exit(0);
  }
  log("Normal termination of game.");
}

/* Accept new connects, relay commands, and call 'heartbeat-functs' */
void game_loop(int s) {
  fd_set input_set, output_set, exc_set;
  struct timeval last_time, now, timespent, timeout, null_time;
  static struct timeval opt_time;
  char comm[MAX_INPUT_LENGTH];
  char promptbuf[256];
  struct descriptor_data *point, *next_point;
  int mask;
  struct room_data *rm;
  struct char_data *mount;

  pulse = 0;
  null_time.tv_sec = 0;
  null_time.tv_usec = 0;

  opt_time.tv_usec = OPT_USEC;	       /* Init time values */
  opt_time.tv_sec = 0;

  gettimeofday(&last_time, (struct timezone *)0);
  maxdesc = s;
  avail_descs = getdtablesize() - 2; /* !! Change if more needed !! */

  mask = sigmask(SIGUSR1) | sigmask(SIGUSR2) | sigmask(SIGINT) |
    sigmask(SIGPIPE) | sigmask(SIGALRM) | sigmask(SIGTERM) |
    sigmask(SIGURG) | sigmask(SIGXCPU) | sigmask(SIGHUP);

  /* Main loop */
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
    sigsetmask(mask);
    whod_loop();
    if (select(maxdesc + 1, &input_set, &output_set, &exc_set, &null_time) < 0) {
      perror("Select poll");
      return;
    }
    if (select(0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, &timeout) < 0) {
      perror("Select sleep");
      /*exit(1); */
    }
    sigsetmask(0);

    /* Respond to whatever might be happening */

    /* New connection? */
    if (FD_ISSET(s, &input_set))
      if (new_descriptor(s) < 0)
	perror("New connection");

    /* kick out the freaky folks */
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
      if (FD_ISSET(point->descriptor, &input_set))
	if (process_input(point) < 0)
	  close_socket(point);
    }

    /* process_commands; */
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
	}
	point->wait = 1;
	if (point->character)
	  point->character->specials.timer = 0;
	point->prompt_mode = 1;

	if (point->str)
	  string_add(point, comm);
	else if (!point->connected)
	  if (point->showstr_point)
	    show_string(point, comm);
	  else
	    command_interpreter(point->character, comm);
	else
	  nanny(point, comm);
      }
    }

    for (point = descriptor_list; point; point = next_point) {
      next_point = point->next;
      if (FD_ISSET(point->descriptor, &output_set) && point->output.head)
	if (process_output(point) < 0)
	  close_socket(point);
	else
	  point->prompt_mode = 1;
    }

    /* give the people some prompts  */
    for (point = descriptor_list; point; point = point->next) {
      if (point->prompt_mode) {
	if (point->str)
	  write_to_descriptor(point->descriptor, "] ");
	else if (!point->connected)
	  if (point->showstr_point)
	    write_to_descriptor(point->descriptor, "*** Press return or q ***");
	  else {
            bzero(promptbuf, 256);
	    if (IS_IMMORTAL(point->character) && IS_PC(point->character)) {
	      if (MOUNTED(point->character)) {
		mount = MOUNTED(point->character);
		sprintf(promptbuf, "[%s has %d/%dh %d/%dv]\n\r",
			GET_SDESC(mount),
			GET_HIT(mount), GET_MAX_HIT(mount),
			GET_MOVE(mount), GET_MAX_MOVE(mount));
	      }
              if (IS_SET(point->character->specials.act, PLR_STEALTH))
                sprintf(promptbuf + strlen(promptbuf), "S");
              if (point->character->invis_level > 0)
                sprintf(promptbuf + strlen(promptbuf), "I=%d: ",
                        point->character->invis_level);
	      rm = real_roomp(point->character->in_room);
	      sprintf(promptbuf + strlen(promptbuf),
                      "#%d - %s [#%d]> ", rm->zone, zone_table[rm->zone].name,
                      rm->number);
	      write_to_descriptor(point->descriptor, promptbuf);
/* OLD mobs didn't have classes.. this doesn't work anymore */
	    } else if( IS_NPC(point->character) &&
              (IS_SET(point->character->specials.act, ACT_POLYSELF) ||
               IS_SET(point->character->specials.act, ACT_POLYOTHER))) {
	      sprintf(promptbuf, "P %d/%dh %d/%dv > ",
		      GET_HIT(point->character),
		      GET_MAX_HIT(point->character),
		      GET_MOVE(point->character),
		      GET_MAX_MOVE(point->character));
	      write_to_descriptor(point->descriptor, promptbuf);
            } else if( IS_NPC(point->character) &&
                       IS_SET(point->character->specials.act, ACT_SWITCH)) {
	      sprintf(promptbuf, "*%s[#%d] in [#%d] %d/%dh %d/%dm %d/%dv > ",
		      NAME(point->character),
                      MobVnum(point->character),
		      point->character->in_room,
		      GET_HIT(point->character),
		      GET_MAX_HIT(point->character),
		      GET_MANA(point->character),
		      GET_MAX_MANA(point->character),
		      GET_MOVE(point->character),
		      GET_MAX_MOVE(point->character));
	      write_to_descriptor(point->descriptor, promptbuf);
	    } else {
	      if (MOUNTED(point->character)) {
		if (HasClass(point->character, CLASS_RANGER) ||
		    IS_AFFECTED(MOUNTED(point->character), AFF_CHARM)) {
		  mount = MOUNTED(point->character);
		  sprintf(promptbuf, "[%s has %d/%dh %d/%dv]\n\r",
			  GET_SDESC(mount),
			  GET_HIT(mount), GET_MAX_HIT(mount),
			  GET_MOVE(mount), GET_MAX_MOVE(mount));
	        }
              }
	      sprintf(promptbuf + strlen(promptbuf), "%d/%dh %d/%dm %d/%dv > ",
		      GET_HIT(point->character),
		      GET_MAX_HIT(point->character),
		      GET_MANA(point->character),
		      GET_MAX_MANA(point->character),
		      GET_MOVE(point->character),
		      GET_MAX_MOVE(point->character));
	      write_to_descriptor(point->descriptor, promptbuf);
	    }
	  }
	point->prompt_mode = 0;
      }
    }
/*
 * PULSE handling.... periodic events
 */

    if ((++pulse) > PULSE_MAX)
      pulse= 0;

    if ((--pulse_zone) <= 0) {
      pulse_zone= PULSE_ZONE;
      zone_update();
    }
    if ((--pulse_teleport) <= 0) {
      pulse_teleport= PULSE_TELEPORT;
      Teleport(pulse);
    }
    if ((--pulse_nature) <= 0) {
      pulse_nature= PULSE_NATURE;
      check_all_nature(pulse);
    }
    if ((--pulse_violence) <= 0) {
      pulse_violence= PULSE_VIOLENCE;
      perform_violence(pulse);
    }
    if ((--pulse_mobile) <= 0) {
      pulse_mobile= PULSE_MOBILE;
      mobile_activity();
    }
    if ((--pulse_river) <= 0) {
      pulse_river= PULSE_RIVER;
      down_river(pulse);
    }
    if ((--pulse_sound) <= 0) {
      pulse_sound= PULSE_SOUND;
      MakeSound(pulse);
    }
    if ((--pulse_update) <= 0) {
      pulse_update= PULSE_UPDATE + number(0, PULSE_VARIABLE);
      weather_and_time(1);
      affect_update();
      point_update(pulse);
    }
    if ((--pulse_reboot) <= 0) {
      pulse_reboot= PULSE_REBOOT;
      check_reboot();
    }
    if ((--pulse_dump) <= 0) {
      pulse_dump= PULSE_DUMP;
      dump_player_list();
    }
    tics++;	       /* tics since last checkpoint signal */
  }
}

/*
 * general utility stuff (for local use)
 */
int get_from_q(struct txt_q *queue, char *dest) {
  struct txt_block *tmp;

  if (!queue) {
    log("Input from non-existant queue?");
    return 0;
  }
  if (!queue->head)
    return 0;
  tmp = queue->head;
  strcpy(dest, queue->head->text);
  queue->head = queue->head->next;
  free(tmp->text);
  free(tmp);
  return 1;
}

void write_to_q(char *txt, struct txt_q *queue) {
  struct txt_block *new;

  if (!queue) {
    log("Output message to non-existant queue");
    return;
  }
  CREATE(new, struct txt_block, 1);
  CREATE(new->text, char, strlen(txt) + 1);
  strcpy(new->text, txt);

  if (!queue->head) {
    new->next = NULL;
    queue->head = queue->tail = new;
  } else {
    queue->tail->next = new;
    queue->tail = new;
    new->next = NULL;
  }
}

struct timeval timediff(struct timeval *a, struct timeval *b) {
  struct timeval result, tmp;

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
void flush_queues(struct descriptor_data *d) {
  char dummy[MAX_STRING_LENGTH];

  while (get_from_q(&d->output, dummy));
  while (get_from_q(&d->input, dummy));
}

/*
 * socket handling
 */

int init_socket(int port) {
  int s;
  char *opt;
  char hostname[MAX_HOSTNAME + 1];
  struct sockaddr_in sa;
  struct hostent *hp;
  int i, gotsocket;

  bzero(&sa, sizeof(struct sockaddr_in));

  gethostname(hostname, MAX_HOSTNAME);
  hp = gethostbyname(hostname);
  if (hp == NULL) {
    perror("gethostbyname");
    exit(1);
  }
  sa.sin_family = hp->h_addrtype;
  sa.sin_port = htons(port);
  s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0) {
    perror("Init-socket");
    exit(1);
  }
  if(setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {
    perror("setsockopt REUSEADDR");
    exit(1);
  }

  for(i= 60; i> 0; i--) {
    if (bind(s, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
      gotsocket= 0;
      fprintf(stderr, "Socket in use... retrying...\n");
      sleep(1);
    } else {
      gotsocket= 1;
      break;
    }
  }
  if(!gotsocket) {
    perror("bind");
    close(s);
    exit(1);
  }
  listen(s, 3);
  return (s);
}

int new_connection(int s) {
  struct sockaddr_in isa;
  int i, t;

  i = sizeof(isa);
  getsockname(s, (struct sockaddr *)&isa, &i);
  if ((t = accept(s, (struct sockaddr *)&isa, &i)) < 0) {
    perror("Accept");
    return (-1);
  }
  nonblock(t);
  return t;
}

int new_descriptor(int s) {
  struct sockaddr_in isa;
  struct hostent *host;
  int i;
  int remote_port;
  long remote_addr;
  char buf[8192];
  int badger = 0;

  int desc, old_maxdesc;
  struct descriptor_data *newd;
  int index;

  long tc;
  struct tm *t_info;

  char *timed_con[] =
  {
    "\n"
  };

  char *bannished[] =
  {
    "grog.lab.cc.wmich.edu",
    "grog2.lab.cc.wmich.edu",
    "grog3.lab.cc.wmich.edu",
    "s01.lab.cc.wmich.edu",
    "s02.lab.cc.wmich.edu",
    "s03.lab.cc.wmich.edu",
    "s04.lab.cc.wmich.edu",
    "s05.lab.cc.wmich.edu",
    "s06.lab.cc.wmich.edu",
    "s07.lab.cc.wmich.edu",
    "s08.lab.cc.wmich.edu",
    "s09.lab.cc.wmich.edu",
    "s10.lab.cc.wmich.edu",
    "s11.lab.cc.wmich.edu",
    "s12.lab.cc.wmich.edu",
    "s14.lab.cc.wmich.edu",
    "s15.lab.cc.wmich.edu",
    "s16.lab.cc.wmich.edu",
    "s17.lab.cc.wmich.edu",
    "s18.lab.cc.wmich.edu",
    "s19.lab.cc.wmich.edu",
    "s20.lab.cc.wmich.edu",
    "s21.lab.cc.wmich.edu",
    "s22.lab.cc.wmich.edu",
    "s23.lab.cc.wmich.edu",
    "s24.lab.cc.wmich.edu",
    "s25.lab.cc.wmich.edu",
    "s26.lab.cc.wmich.edu",
    "s27.lab.cc.wmich.edu",
    "s28.lab.cc.wmich.edu",
    "s29.lab.cc.wmich.edu",
    "s30.lab.cc.wmich.edu",
    "s31.lab.cc.wmich.edu",
    "s32.lab.cc.wmich.edu",
    "s33.lab.cc.wmich.edu",
    "s34.lab.cc.wmich.edu",
    "s35.lab.cc.wmich.edu",
    "s36.lab.cc.wmich.edu",
    "s37.lab.cc.wmich.edu",
    "s38.lab.cc.wmich.edu",
    "s39.lab.cc.wmich.edu",
    "s40.lab.cc.wmich.edu",
    "s41.lab.cc.wmich.edu",
    "s42.lab.cc.wmich.edu",
    "s43.lab.cc.wmich.edu",
    "\n"
  };

  tc = time(0);
  t_info = localtime(&tc);

  old_maxdesc = maxdesc;

  i = sizeof(isa);
  getsockname(s, (struct sockaddr *)&isa, &i);
  if ((desc = accept(s, (struct sockaddr *)&isa, &i)) < 0) {
    perror("Accept");
    return (-1);
  }
  nonblock(desc);

  if ((maxdesc + 1) >= avail_descs) {
    write_to_descriptor(desc, "Sorry.. full...\n\r");
    close(desc);
    return (0);
  } else if (desc > maxdesc)
    maxdesc = desc;

  CREATE(newd, struct descriptor_data, 1);

  remote_port= ntohs(isa.sin_port);
  remote_addr= isa.sin_addr.s_addr;

  newd->username[0]= '\0';
#ifdef RFC1413
  {
    char *ack;
    unsigned long inlocal, inremote;
    unsigned short local, remote;

    if((ack= ident_id(desc, 10))) {
      strncpy(newd->username, ack, 16);
    } else if(auth_fd2(desc, &inlocal, &inremote, &local, &remote) >= 0) {
      if((ack= auth_tcpuser3(inlocal, inremote, local, remote, 10))) {
        strncpy(newd->username, ack, 16);
        badger= 1;
      } else
        badger= 2;
    } else
      badger= 2;
  }
#endif
  if(!newd->username[0])
    strcpy(newd->username, "adork");

  if(!(host= gethostbyaddr((char *)&remote_addr, sizeof(remote_addr), AF_INET)))

    sprintf(newd->host, "%u.%u.%u.%u",
            (int)(((char *)&remote_addr)[0])&255, (int)(((char *)&remote_addr)[1])&255,
            (int)(((char *)&remote_addr)[2])&255, (int)(((char *)&remote_addr)[3])&255);
  else
    strncpy(newd->host, host->h_name, 49);

  /* init desc data */
  newd->descriptor = desc;
  newd->connected = 1;
  newd->wait = 1;
  newd->prompt_mode = 0;
  *newd->buf = '\0';
  newd->str = 0;
  newd->showstr_head = 0;
  newd->showstr_point = 0;
  *newd->last_input = '\0';
  newd->output.head = NULL;
  newd->input.head = NULL;
  newd->next = descriptor_list;
  newd->character = 0;
  newd->original = 0;
  newd->snoop.snooping = 0;
  newd->snoop.snoop_by = 0;

#ifdef RFC1413
  switch(badger) {
    case 1:
      write_to_descriptor(desc, "\n\r **** Tell your sys-admin to upgrade to the SHINY NEW telnet using RFC1413 ****\n\r\n\r");
      break;
    case 2:
      write_to_descriptor(desc, "\n\r **** Tell your sys-admin to practice safe telnet by using RFC1413 ****\n\r\n\r");
      break;
    default:
      break;
  }
#endif

  /* prepend to list */

  if (((t_info->tm_hour + 1) > 8) && ((t_info->tm_hour + 1) < 21))
    for (index = 0; timed_con[index] != "\n"; index++) {
      if (!strncmp(timed_con[index], newd->host, 49)) {
	sprintf(buf, "TIMED site connecting:%s\n", newd->host);
	log(buf);
	sprintf(buf, "\n\rThis site is blocked from : 9 am - 9 pm\n\r");
	write_to_descriptor(desc, buf);
	sprintf(buf, "You may connect after 9 pm from :[%s]\n\r",
		newd->host);
	write_to_descriptor(desc, buf);
	maxdesc = old_maxdesc;
	free(newd);
	close(desc);
	return (0);
      }
    }
  for (index = 0; bannished[index] != "\n"; index++) {
    if (!strncmp(bannished[index], newd->host, 49)) {
      sprintf(buf, "BANNISHED site connecting:%s\n", newd->host);
      log(buf);
      sprintf(buf, "\n\rDue to your System Administrators request, or for some\n\r");
      write_to_descriptor(desc, buf);
      sprintf(buf, "other reason, we are refusing all connections from:[%s]\n\r",
	      newd->host);
      write_to_descriptor(desc, buf);
      maxdesc = old_maxdesc;
      free(newd);
      close(desc);
      return (0);
    }
  }

  descriptor_list = newd;
  SEND_TO_Q(greetings, newd);
  SEND_TO_Q("By what name do you wish to be known? ", newd);
  return (0);
}

int process_output(struct descriptor_data *t) {
  char i[MAX_STRING_LENGTH + 1];

  if (DEBUG)
    dlog("process_output");
  if (!t->prompt_mode && !t->connected)
    if (write_to_descriptor(t->descriptor, "\n\r") < 0)
      return (-1);

  /* Cycle thru output queue */
  while (get_from_q(&t->output, i)) {
    if ((t->snoop.snoop_by) && (t->snoop.snoop_by->desc)) {
      write_to_q("S* ", &t->snoop.snoop_by->desc->output);
      write_to_q(i, &t->snoop.snoop_by->desc->output);
    }
    if (write_to_descriptor(t->descriptor, i))
      return (-1);
  }

  if (!t->connected && !(t->character && !IS_NPC(t->character) &&
		   IS_SET(t->character->specials.act, PLR_COMPACT)))
    if (write_to_descriptor(t->descriptor, "\n\r") < 0)
      return (-1);

  return (1);
}

int write_to_descriptor(int desc, char *txt) {
  int sofar, thisround, total;

  total = strlen(txt);
  sofar = 0;

  do {
    thisround = write(desc, txt + sofar, total - sofar);
    if (thisround < 0) {
      if (errno == EWOULDBLOCK)
	break;
      perror("Write to socket");
      return (-1);
    }
    sofar += thisround;
  }
  while (sofar < total);

  return (0);
}

int process_input(struct descriptor_data *t) {
  int sofar, thisround, begin, squelch, i, k, flag;
  char tmp[MAX_INPUT_LENGTH + 2], buffer[MAX_INPUT_LENGTH + 60];
  long now_time;

  sofar = 0;
  flag = 0;
  begin = strlen(t->buf);

  /* Read in some stuff */
  do {
    if ((thisround = read(t->descriptor, t->buf + begin + sofar,
		    MAX_STRING_LENGTH - (begin + sofar) - 1)) > 0) {
      sofar += thisround;
    } else {
      if (thisround < 0) {
	if (errno != EWOULDBLOCK) {
	  perror("Read1 - ERROR");
	  return (-1);
	} else {
	  break;
	}
      } else {
	log("EOF encountered on socket read.");
	return (-1);
      }
    }
  } while (!ISNEWL(*(t->buf + begin + sofar - 1)));

  *(t->buf + begin + sofar) = 0;

  /* if no newline is contained in input, return without proc'ing */
  for (i = begin; !ISNEWL(*(t->buf + i)); i++)
    if (!*(t->buf + i))
      return (0);

  /* input contains 1 or more newlines; process the stuff */
  for (i = 0, k = 0; *(t->buf + i);) {
    if (!ISNEWL(*(t->buf + i)) && !(flag = (k >= (MAX_INPUT_LENGTH - 2))))
      if (*(t->buf + i) == '\b') {     /* backspace */
	if (k) {		       /* more than one char ? */
	  if (*(tmp + --k) == '$')
	    k--;
	  i++;
	} else {
	  i++;			       /* no or just one char.. Skip backsp */
	}
      } else {
	if (isascii(*(t->buf + i)) && isprint(*(t->buf + i))) {
	  /* 
	   * trans char, double for '$' (printf)        
	   */
	  if ((*(tmp + k) = *(t->buf + i)) == '$')
	    *(tmp + ++k) = '$';
	  k++;
	  i++;
	} else {
	  i++;
	}
    } else {
      *(tmp + k) = 0;
      if (*tmp == '!')
	strcpy(tmp, t->last_input);
      else
	strcpy(t->last_input, tmp);

      write_to_q(tmp, &t->input);

      now_time = time(0);
      t->idle_time = now_time;
      if ((t->snoop.snoop_by) && (t->snoop.snoop_by->desc)) {
	write_to_q("% ", &t->snoop.snoop_by->desc->output);
	write_to_q(tmp, &t->snoop.snoop_by->desc->output);
	write_to_q("\n\r", &t->snoop.snoop_by->desc->output);
      }
      if (flag) {
	sprintf(buffer,
		"Line too long. Truncated to:\n\r%s\n\r", tmp);
	if (write_to_descriptor(t->descriptor, buffer) < 0)
	  return (-1);

	/* skip the rest of the line */
	for (; !ISNEWL(*(t->buf + i)); i++);
      }
      /* find end of entry */
      for (; ISNEWL(*(t->buf + i)); i++);

      /* squelch the entry from the buffer */
      for (squelch = 0;; squelch++)
	if ((*(t->buf + squelch) =
	     *(t->buf + i + squelch)) == '\0')
	  break;
      k = 0;
      i = 0;
    }
  }
  return (1);
}

void close_sockets(int s) {
  log("Closing all sockets.");
  while (descriptor_list)
    close_socket(descriptor_list);
  close(s);
}

void close_socket(struct descriptor_data *d) {
  struct descriptor_data *tmp;
  char buf[100];

  void do_save(struct char_data *ch, char *argument, int cmd);

  if (!d)
    return;

  close(d->descriptor);
  flush_queues(d);
  if (d->descriptor == maxdesc)
    --maxdesc;

  /* Forget snooping */

  if (d->snoop.snooping)
    d->snoop.snooping->desc->snoop.snoop_by = 0;

  if (d->snoop.snoop_by) {
    cprintf(d->snoop.snoop_by, "Your victim is no longer among us.\n\r");
    d->snoop.snoop_by->desc->snoop.snooping = 0;
  }
  if (d->character)
    if (d->connected == CON_PLAYING) {
      do_save(d->character, "", 0);
      act("$n has lost $s link.", TRUE, d->character, 0, 0, TO_ROOM);
      sprintf(buf, "Closing link to: %s.", GET_NAME(d->character));
      log(buf);
      if (IS_NPC(d->character)) {
	if (d->character->desc)
	  d->character->orig = d->character->desc->original;
      }
      d->character->desc = 0;
    } else {
      if (GET_NAME(d->character)) {
	sprintf(buf, "Losing player: %s.", GET_NAME(d->character));
	log(buf);
      }
      free_char(d->character);
  } else
    log("Losing descriptor without char.");

  if (next_to_process == d)	       /* to avoid crashing the process loop */
    next_to_process = next_to_process->next;

  if (d == descriptor_list)	       /* this is the head of the list */
    descriptor_list = descriptor_list->next;
  else {			       /* This is somewhere inside the list */
    /* Locate the previous element */
    for (tmp = descriptor_list; (tmp->next != d) && tmp;
	 tmp = tmp->next);

    tmp->next = d->next;
  }
  if (d->showstr_head)
    free(d->showstr_head);
  free(d);
}

void nonblock(int s) {
  if (fcntl(s, F_SETFL, O_NDELAY) == -1) {
    perror("Noblock");
    exit(1);
  }
}

/*
 * Public routines for system-to-player-communication
 */

/*
 * This acts as an interface to write_to_q(), but it uses variable arguments
 * to eliminate multiple calls to sprintf().
 */
void dprintf(struct descriptor_data *d, char *Str,...) {
  va_list arg;
  char Result[MAX_STRING_LENGTH];

  if (Str && *Str && d) {
    bzero(Result, MAX_STRING_LENGTH);
    va_start(arg, Str);
    vsprintf(Result, Str, arg);
    va_end(arg);
    write_to_q(Result, &d->output);
  }
}

/*
 * This works like send_to_char(), but it uses variable arguments to
 * eliminate multiple calls to sprintf().
 */
void cprintf(struct char_data *ch, char *Str,...) {
  va_list arg;
  char Result[MAX_STRING_LENGTH];

  if (Str && *Str && ch && ch->desc) {
    bzero(Result, MAX_STRING_LENGTH);
    va_start(arg, Str);
    vsprintf(Result, Str, arg);
    va_end(arg);
    write_to_q(Result, &ch->desc->output);
  }
}

/*
 * This one is an interface to replace send_to_room().
 */
void rprintf(int room, char *Str,...) {
  va_list arg;
  struct char_data *i;
  struct room_data *rr;
  char Result[MAX_STRING_LENGTH];

  if (Str && *Str && room >= 0 && (rr = real_roomp(room))) {
    bzero(Result, MAX_STRING_LENGTH);
    va_start(arg, Str);
    vsprintf(Result, Str, arg);
    va_end(arg);
    for (i = rr->people; i; i = i->next_in_room)
      if (i->desc)
	write_to_q(Result, &i->desc->output);
  }
}

/*
 * This one is everyone in the zone specified.
 */
void zprintf(int zone, char *Str,...) {
  va_list arg;
  struct descriptor_data *i;
  char Result[MAX_STRING_LENGTH];
  struct room_data *rr;

  if (Str && *Str && zone >= 0) {
    bzero(Result, MAX_STRING_LENGTH);
    va_start(arg, Str);
    vsprintf(Result, Str, arg);
    va_end(arg);
    for (i = descriptor_list; i; i = i->next)
      if (!i->connected)
        if(i->character)
          if((rr= real_roomp(i->character->in_room)))
            if(rr->zone == zone)
	      write_to_q(Result, &i->output);
  }
}

/*
 * And this one sends to EVERYBODY int the game!!!!!
 */
void aprintf(char *Str,...) {
  va_list arg;
  struct descriptor_data *i;
  char Result[MAX_STRING_LENGTH];

  if (Str && *Str) {
    bzero(Result, MAX_STRING_LENGTH);
    va_start(arg, Str);
    vsprintf(Result, Str, arg);
    va_end(arg);
    for (i = descriptor_list; i; i = i->next)
      if (!i->connected)
	write_to_q(Result, &i->output);
  }
}

/*
 * Here is send_to_outdoor()
 */
void oprintf(char *Str,...) {
  va_list arg;
  struct descriptor_data *i;
  char Result[MAX_STRING_LENGTH];

  if (Str && *Str) {
    bzero(Result, MAX_STRING_LENGTH);
    va_start(arg, Str);
    vsprintf(Result, Str, arg);
    va_end(arg);
    for (i = descriptor_list; i; i = i->next)
      if (!i->connected && i->character && OUTSIDE(i->character))
	write_to_q(Result, &i->output);
  }
}

/*
 * Send to everyone except the given character.
 */
void eprintf(struct char_data *ch, char *Str,...) {
  va_list arg;
  struct descriptor_data *i;
  char Result[MAX_STRING_LENGTH];

  if (Str && *Str) {
    bzero(Result, MAX_STRING_LENGTH);
    va_start(arg, Str);
    vsprintf(Result, Str, arg);
    va_end(arg);
    for (i = descriptor_list; i; i = i->next)
      if (ch && ch->desc != i && !i->connected)
	write_to_q(Result, &i->output);
  }
}

/*
 * This one is for send_to_room_except()
 */
void reprintf(int room, struct char_data *ch, char *Str,...) {
  va_list arg;
  struct char_data *i;
  struct room_data *rr;
  char Result[MAX_STRING_LENGTH];

  if (Str && *Str && room >= 0 && (rr = real_roomp(room))) {
    bzero(Result, MAX_STRING_LENGTH);
    va_start(arg, Str);
    vsprintf(Result, Str, arg);
    va_end(arg);
    for (i = rr->people; i; i = i->next_in_room)
      if (i != ch && i->desc)
	write_to_q(Result, &i->desc->output);
  }
}

/*
 * This one is for send_to_room_except()
 */
void re2printf(int room, struct char_data *ch1, struct char_data *ch2, char *Str,...) {
  va_list arg;
  struct char_data *i;
  struct room_data *rr;
  char Result[MAX_STRING_LENGTH];

  if (Str && *Str && room >= 0 && (rr = real_roomp(room))) {
    bzero(Result, MAX_STRING_LENGTH);
    va_start(arg, Str);
    vsprintf(Result, Str, arg);
    va_end(arg);
    for (i = rr->people; i; i = i->next_in_room)
      if (i != ch1 && i != ch2 && i->desc)
	write_to_q(Result, &i->desc->output);
  }
}

/*
 * IMMORTAL printf.
 */
void iprintf(char *Str,...) {
  va_list arg;
  struct descriptor_data *i;
  char Result[MAX_STRING_LENGTH];

  if (Str && *Str) {
    bzero(Result, MAX_STRING_LENGTH);
    va_start(arg, Str);
    vsprintf(Result, Str, arg);
    va_end(arg);
    for (i = descriptor_list; i; i = i->next)
      if (!i->connected && i->character && IS_IMMORTAL(i->character))
	write_to_q(Result, &i->output);
  }
}

void save_all() {
  struct descriptor_data *i;

  for (i = descriptor_list; i; i = i->next)
    if (i->character)
      save_char(i->character, NOWHERE);
}

/* higher-level communication */

void act(char *str, int hide_invisible, struct char_data *ch,
	 struct obj_data *obj, void *vict_obj, int type) {
  register char *strp, *point, *i = NULL;
  struct char_data *to;
  char buf[MAX_STRING_LENGTH];

  if (!str)
    return;
  if (!*str)
    return;

  if (type == TO_VICT)
    to = (struct char_data *)vict_obj;
  else if (type == TO_CHAR)
    to = ch;
  else {
    if(!ch)
      return;
    if(!real_roomp(ch->in_room))
      return;
    if(!(to = real_roomp(ch->in_room)->people))
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
	    log("Illegal $-code to act():");
	    log(str);
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

      write_to_q(CAP(buf), &to->desc->output);
    }
    if ((type == TO_VICT) || (type == TO_CHAR))
      return;
  }
}

void dump_player_list(void) {
  FILE *pfd;
  int i;

  log("Dumping player list");
  if(!(pfd= fopen(PLAYER_FILE, "w"))) {
    bug("Cannot save player data for new user!");
  } else {
    fprintf(pfd, "%d\n", actual_players);
    for(i= 0; i< number_of_players; i++)
      if(list_of_players[i])
        fprintf(pfd, "%s", list_of_players[i]);
    fclose(pfd);
  }
}

/*
 * ************************************************************************
 * *  file: signals.c , trapping of signals from Unix.       Part of DIKUMUD *
 * *  Usage : Signal Trapping.                                               *
 * *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
 * ************************************************************************* 
 */

#include <signal.h>
#include <stdio.h>
#include <sys/time.h>

#include "structs.h"
#include "utils.h"

#ifdef sun3
void                             shutdown_request(void);
void                             checkpointing(void);
void                             logsig(void);
void                             hupsig(void);

#else
void                             shutdown_request(int);
void                             checkpointing(int);
void                             logsig(int);
void                             hupsig(int);

#endif

void 
signal_setup(void)
{
  struct itimerval                 itime;
  struct timeval                   interval;

  signal(SIGUSR2, shutdown_request);
  signal(SIGHUP, hupsig);
  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, hupsig);
  signal(SIGALRM, logsig);
  signal(SIGTERM, hupsig);

  /*
   * set up the deadlock-protection 
   */

  interval.tv_sec = 900;	/*
				 * 30 minutes 
				 */
  interval.tv_usec = 0;
  itime.it_interval = interval;
  itime.it_value = interval;
  setitimer(ITIMER_VIRTUAL, &itime, 0);
  signal(SIGVTALRM, checkpointing);
}

#ifdef sun3
void 
checkpointing(void)
#else
void 
checkpointing(int a)
#endif
{
  extern int                       tics;

  if (!tics) {
    log("CHECKPOINT shutdown: tics not updated");
    log("SHUTDOWN:now");
    abort(0);
    close_sockets(0);
    close_whod();
    exit(52);
  } else
    tics = 0;
}

#ifdef sun3
void 
shutdown_request(void)
#else
void 
shutdown_request(int a)
#endif
{
  log("Received USR2 - shutdown request");
  close_sockets(0);
  close_whod();
  exit(52);
}

#ifdef sun3
void 
hupsig(void)
#else
void 
hupsig(int a)
#endif
{
  extern int                       diku_shutdown;

  log("Received SIGHUP, SIGINT, or SIGTERM. Shutting down");
  exit(0);			/*
				 * something more elegant should perhaps be substituted 
				 */
}

#ifdef sun3
void 
logsig(void)
#else
void 
logsig(int a)
#endif
{
  log("Signal received. Ignoring.");
}

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
  register int i;
  struct itimerval                 itime;
  struct timeval                   interval;
  struct sigaction ack[] = {
          {SIG_DFL, NULL, SA_NOMASK|SA_ONESHOT, NULL},
          {hupsig, SIGHUP, SA_NOMASK, NULL},
          {hupsig, SIGINT, SA_NOMASK, NULL},
          {SIG_IGN, SIGQUIT, SA_NOMASK, NULL},
          {SIG_DFL, SIGILL, SA_NOMASK|SA_ONESHOT, NULL},
          {SIG_DFL, SIGTRAP, SA_NOMASK|SA_ONESHOT, NULL},
          {SIG_IGN, SIGIOT, SA_NOMASK, NULL},
          {SIG_DFL, SIGBUS, SA_NOMASK|SA_ONESHOT, NULL},
          {SIG_DFL, SIGFPE, SA_NOMASK|SA_ONESHOT, NULL},
          {SIG_DFL, SIGKILL, SA_NOMASK|SA_ONESHOT, NULL},
          {SIG_DFL, SIGUSR1, SA_NOMASK|SA_ONESHOT, NULL},
          {SIG_DFL, SIGSEGV, SA_NOMASK|SA_ONESHOT, NULL},
          {shutdown_request, SIGUSR2, SA_NOMASK, NULL},
          {SIG_IGN, SIGPIPE, SA_NOMASK, NULL},
          {logsig, SIGALRM, SA_NOMASK, NULL},
          {hupsig, SIGTERM, SA_NOMASK, NULL},
          {SIG_DFL, SIGSTKFLT, SA_NOMASK|SA_ONESHOT, NULL},
          {SIG_IGN, SIGCHLD, SA_NOMASK, NULL},
          {SIG_IGN, SIGCONT, SA_NOMASK, NULL},
          {SIG_IGN, SIGSTOP, SA_NOMASK, NULL},
          {SIG_IGN, SIGTSTP, SA_NOMASK, NULL},
          {SIG_IGN, SIGTTIN, SA_NOMASK, NULL},
          {SIG_IGN, SIGTTOU, SA_NOMASK, NULL},
          {SIG_IGN, SIGURG, SA_NOMASK, NULL},
          {SIG_IGN, SIGXCPU, SA_NOMASK, NULL},
          {SIG_IGN, SIGXFSZ, SA_NOMASK, NULL},
          {checkpointing, SIGVTALRM, SA_NOMASK, NULL},
          {SIG_DFL, SIGPROF, SA_NOMASK|SA_ONESHOT, NULL},
          {SIG_IGN, SIGWINCH, SA_NOMASK, NULL},
          {SIG_IGN, SIGIO, SA_NOMASK, NULL},
          {shutdown_request, SIGPWR, SA_NOMASK, NULL},
          {SIG_DFL, SIGUNUSED, SA_NOMASK|SA_ONESHOT, NULL}
        };

/*
  signal(SIGHUP, hupsig);
  signal(SIGINT, hupsig);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGIOT, SIG_IGN);
  signal(SIGUSR2, shutdown_request);
  signal(SIGPIPE, SIG_IGN);
  signal(SIGALRM, logsig);
  signal(SIGTERM, hupsig);
  signal(SIGTTIN, SIG_IGN);
  signal(SIGTTOU, SIG_IGN);
  signal(SIGXCPU, SIG_IGN);
  signal(SIGXFSZ, SIG_IGN);
  signal(SIGWINCH, SIG_IGN);
  signal(SIGIO, SIG_IGN);
  signal(SIGPWR, shutdown_request);
*/

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
  /* signal(SIGVTALRM, checkpointing); */

  for(i= 0; i< 32; i++)
    if(ack[i].sa_handler != SIG_DFL)
      sigaction(i, &ack[i], NULL);
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

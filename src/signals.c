/*
 * file: signals.c , trapping of signals from Unix.       Part of DIKUMUD
 * Usage : Signal Trapping.
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/time.h>

#include "include/global.h"
#include "include/bug.h"
#include "include/utils.h"
#include "include/comm.h"
#include "include/whod.h"
#define _DIKU_SIGNALS_C
#include "include/signals.h"

void signal_setup(void)
{
  register int i;
  struct itimerval itime;
  struct timeval interval;
  struct sigaction ack[] =
  {
    {SIG_DFL, 0, SA_NOMASK | SA_ONESHOT, NULL},
    {hupsig, SIGHUP, SA_NOMASK, NULL},
    {SIG_IGN, SIGINT, SA_NOMASK, NULL},
    {SIG_IGN, SIGQUIT, SA_NOMASK, NULL},
    {SIG_DFL, SIGILL, SA_NOMASK | SA_ONESHOT, NULL},
    {SIG_DFL, SIGTRAP, SA_NOMASK | SA_ONESHOT, NULL},
    {SIG_IGN, SIGIOT, SA_NOMASK, NULL},
    {SIG_DFL, SIGBUS, SA_NOMASK | SA_ONESHOT, NULL},
    {SIG_DFL, SIGFPE, SA_NOMASK | SA_ONESHOT, NULL},
    {SIG_DFL, SIGKILL, SA_NOMASK | SA_ONESHOT, NULL},
    {SIG_DFL, SIGUSR1, SA_NOMASK | SA_ONESHOT, NULL},
    {SIG_DFL, SIGSEGV, SA_NOMASK | SA_ONESHOT, NULL},
    {shutdown_request, SIGUSR2, SA_NOMASK, NULL},
    {SIG_IGN, SIGPIPE, SA_NOMASK, NULL},
    {logsig, SIGALRM, SA_NOMASK, NULL},
    {SIG_IGN, SIGTERM, SA_NOMASK, NULL},
    {SIG_DFL, SIGSTKFLT, SA_NOMASK | SA_ONESHOT, NULL},
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
    {SIG_DFL, SIGPROF, SA_NOMASK | SA_ONESHOT, NULL},
    {SIG_IGN, SIGWINCH, SA_NOMASK, NULL},
    {SIG_IGN, SIGIO, SA_NOMASK, NULL},
    {shutdown_request, SIGPWR, SA_NOMASK, NULL},
    {SIG_DFL, SIGUNUSED, SA_NOMASK | SA_ONESHOT, NULL}
  };

/*
 * signal(SIGHUP, hupsig);
 * signal(SIGINT, hupsig);
 * signal(SIGQUIT, SIG_IGN);
 * signal(SIGIOT, SIG_IGN);
 * signal(SIGUSR2, shutdown_request);
 * signal(SIGPIPE, SIG_IGN);
 * signal(SIGALRM, logsig);
 * signal(SIGTERM, hupsig);
 * signal(SIGTTIN, SIG_IGN);
 * signal(SIGTTOU, SIG_IGN);
 * signal(SIGXCPU, SIG_IGN);
 * signal(SIGXFSZ, SIG_IGN);
 * signal(SIGWINCH, SIG_IGN);
 * signal(SIGIO, SIG_IGN);
 * signal(SIGPWR, shutdown_request);
 */

  /*
   * set up the deadlock-protection 
   */

  interval.tv_sec = 900;	       /* 30 minutes */
  interval.tv_usec = 0;
  itime.it_interval = interval;
  itime.it_value = interval;
  setitimer(ITIMER_VIRTUAL, &itime, 0);
  /* signal(SIGVTALRM, checkpointing); */

  for (i = 0; i < 32; i++)
    if (ack[i].sa_handler != SIG_DFL)
      sigaction(i, &ack[i], NULL);
}

void checkpointing(int a)
{
  if (!tics) {
    log("CHECKPOINT shutdown: tics not updated");
    log("SHUTDOWN:now");
    /* abort(0); */
    close_sockets(0);
    close_whod();
    exit(42);
  } else
    tics = 0;
}

void shutdown_request(int a)
{
  log("Received USR2 - shutdown request");
  close_sockets(0);
  close_whod();
  exit(42);
}

void hupsig(int a)
{
  log("Received SIGHUP, or SIGTERM. Shutting down");
  close_sockets(0);
  close_whod();
  exit(0);			       /* something more elegant should perhaps be substituted */
}

void logsig(int a)
{
  log("Signal received. Ignoring.");
}

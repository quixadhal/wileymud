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

/*
       The sigaction structure is defined as

              struct sigaction {
                  void (*sa_handler)(int);
                  void (*sa_sigaction)(int, siginfo_t *, void *);
                  sigset_t sa_mask;
                  int sa_flags;
                  void (*sa_restorer)(void);
              }
*/

/*
	And under OpenBSD that would be:

     struct sigaction {
             union {
                     void    (*__sa_handler) __P((int));
                     void    (*__sa_sigaction) __P((int, siginfo_t *, void *));
             } __sigaction_u;
             sigset_t sa_mask;
             int      sa_flags;
     };

*/

/*
  Also worth noting under OpenBSD:

bash-2.05$ kill -l
 1) SIGHUP       2) SIGINT       3) SIGQUIT      4) SIGILL
 5) SIGTRAP      6) SIGABRT      7) SIGEMT       8) SIGFPE
 9) SIGKILL     10) SIGBUS      11) SIGSEGV     12) SIGSYS
13) SIGPIPE     14) SIGALRM     15) SIGTERM     16) SIGURG
17) SIGSTOP     18) SIGTSTP     19) SIGCONT     20) SIGCHLD
21) SIGTTIN     22) SIGTTOU     23) SIGIO       24) SIGXCPU
25) SIGXFSZ     26) SIGVTALRM   27) SIGPROF     28) SIGWINCH
29) SIGINFO     30) SIGUSR1     31) SIGUSR2     

*/

void signal_setup(void)
{
  register int i;
  struct itimerval itime;
  struct timeval interval;
  struct sigaction ack[] =
  {
    { SIG_DFL, 0, SA_NODEFER | SA_RESETHAND },
    { hupsig, SIGHUP, SA_NODEFER },
    { SIG_IGN, SIGINT, SA_NODEFER },
    { SIG_IGN, SIGQUIT, SA_NODEFER },
    { SIG_DFL, SIGILL, SA_NODEFER | SA_RESETHAND },
    { SIG_DFL, SIGTRAP, SA_NODEFER | SA_RESETHAND },
    { SIG_DFL, SIGABRT, SA_NODEFER | SA_RESETHAND },
    { SIG_DFL, SIGEMT, SA_NODEFER | SA_RESETHAND },
    { SIG_DFL, SIGFPE, SA_NODEFER | SA_RESETHAND },
    { SIG_DFL, SIGKILL, SA_NODEFER | SA_RESETHAND },
    { SIG_DFL, SIGBUS, SA_NODEFER | SA_RESETHAND },
    { SIG_DFL, SIGSEGV, SA_NODEFER | SA_RESETHAND },
    { SIG_DFL, SIGSYS, SA_NODEFER | SA_RESETHAND },
    { SIG_IGN, SIGPIPE, SA_NODEFER },
    { logsig, SIGALRM, SA_NODEFER },
    { SIG_IGN, SIGTERM, SA_NODEFER },
    { SIG_IGN, SIGURG, SA_NODEFER },
    { SIG_IGN, SIGSTOP, SA_NODEFER },
    { SIG_IGN, SIGTSTP, SA_NODEFER },
    { SIG_IGN, SIGCONT, SA_NODEFER },
    { SIG_IGN, SIGCHLD, SA_NODEFER },
    { SIG_IGN, SIGTTIN, SA_NODEFER },
    { SIG_IGN, SIGTTOU, SA_NODEFER },
    { SIG_IGN, SIGIO, SA_NODEFER },
    { SIG_IGN, SIGXCPU, SA_NODEFER },
    { SIG_IGN, SIGXFSZ, SA_NODEFER },
    { checkpointing, SIGVTALRM, SA_NODEFER },
    { SIG_DFL, SIGPROF, SA_NODEFER | SA_RESETHAND },
    { SIG_IGN, SIGWINCH, SA_NODEFER },
    { shutdown_request, SIGUSR1, SA_NODEFER },
    { SIG_IGN, SIGUSR2, SA_NODEFER },
//    { SIG_DFL, SIGINFO, SA_NODEFER | SA_RESETHAND },
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

  for (i = 1; i < 32; i++)
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

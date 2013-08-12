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

#include "global.h"
#include "bug.h"
#include "utils.h"
#include "comm.h"
#include "whod.h"
#define _DIKU_SIGNALS_C
#include "signals.h"

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

  Under slackware 9.1 (Linux 2.4 kernel):

 1) SIGHUP       2) SIGINT       3) SIGQUIT      4) SIGILL
 5) SIGTRAP      6) SIGABRT      7) SIGBUS       8) SIGFPE
 9) SIGKILL     10) SIGUSR1     11) SIGSEGV     12) SIGUSR2
13) SIGPIPE     14) SIGALRM     15) SIGTERM     17) SIGCHLD
18) SIGCONT     19) SIGSTOP     20) SIGTSTP     21) SIGTTIN
22) SIGTTOU     23) SIGURG      24) SIGXCPU     25) SIGXFSZ
26) SIGVTALRM   27) SIGPROF     28) SIGWINCH    29) SIGIO
30) SIGPWR      31) SIGSYS

  Under  Debian Etch (Linux 2.6 kernel):

 1) SIGHUP       2) SIGINT       3) SIGQUIT      4) SIGILL
 5) SIGTRAP      6) SIGABRT      7) SIGBUS       8) SIGFPE
 9) SIGKILL     10) SIGUSR1     11) SIGSEGV     12) SIGUSR2
13) SIGPIPE     14) SIGALRM     15) SIGTERM     16) SIGSTKFLT
17) SIGCHLD     18) SIGCONT     19) SIGSTOP     20) SIGTSTP
21) SIGTTIN     22) SIGTTOU     23) SIGURG      24) SIGXCPU
25) SIGXFSZ     26) SIGVTALRM   27) SIGPROF     28) SIGWINCH
29) SIGIO       30) SIGPWR      31) SIGSYS      34) SIGRTMIN
35) SIGRTMIN+1  36) SIGRTMIN+2  37) SIGRTMIN+3  38) SIGRTMIN+4
39) SIGRTMIN+5  40) SIGRTMIN+6  41) SIGRTMIN+7  42) SIGRTMIN+8
43) SIGRTMIN+9  44) SIGRTMIN+10 45) SIGRTMIN+11 46) SIGRTMIN+12
47) SIGRTMIN+13 48) SIGRTMIN+14 49) SIGRTMIN+15 50) SIGRTMAX-14
51) SIGRTMAX-13 52) SIGRTMAX-12 53) SIGRTMAX-11 54) SIGRTMAX-10
55) SIGRTMAX-9  56) SIGRTMAX-8  57) SIGRTMAX-7  58) SIGRTMAX-6
59) SIGRTMAX-5  60) SIGRTMAX-4  61) SIGRTMAX-3  62) SIGRTMAX-2
63) SIGRTMAX-1  64) SIGRTMAX    
*/

void signal_setup(void)
{
    struct itimerval                        itime;
    struct timeval                          interval;

#ifdef USE_SIGACTION
    int                                     i = 0;

    struct sigaction                        ack[] = {
#ifdef __OpenBSD__
	{{SIG_DFL}, 0, SA_NODEFER | SA_RESETHAND},
	{{SIG_IGN}, SIGHUP, SA_NODEFER},
	{{shutdown_request}, SIGINT, SA_NODEFER},
	{{SIG_IGN}, SIGQUIT, SA_NODEFER},
	{{SIG_DFL}, SIGILL, SA_NODEFER | SA_RESETHAND},
	{{SIG_DFL}, SIGTRAP, SA_NODEFER | SA_RESETHAND},
	{{SIG_DFL}, SIGABRT, SA_NODEFER | SA_RESETHAND},
	{{SIG_DFL}, SIGEMT, SA_NODEFER | SA_RESETHAND},
	{{SIG_DFL}, SIGFPE, SA_NODEFER | SA_RESETHAND},
	{{SIG_DFL}, SIGKILL, SA_NODEFER | SA_RESETHAND},
	{{SIG_DFL}, SIGBUS, SA_NODEFER | SA_RESETHAND},
	{{SIG_DFL}, SIGSEGV, SA_NODEFER | SA_RESETHAND},
	{{SIG_DFL}, SIGSYS, SA_NODEFER | SA_RESETHAND},
	{{SIG_IGN}, SIGPIPE, SA_NODEFER},
	{{SIG_IGN}, SIGALRM, SA_NODEFER},
	{{SIG_IGN}, SIGTERM, SA_NODEFER},
	{{SIG_IGN}, SIGURG, SA_NODEFER},
	{{SIG_IGN}, SIGSTOP, SA_NODEFER},
	{{SIG_IGN}, SIGTSTP, SA_NODEFER},
	{{SIG_IGN}, SIGCONT, SA_NODEFER},
	{{SIG_IGN}, SIGCHLD, SA_NODEFER},
	{{SIG_IGN}, SIGTTIN, SA_NODEFER},
	{{SIG_IGN}, SIGTTOU, SA_NODEFER},
	{{SIG_IGN}, SIGIO, SA_NODEFER},
	{{SIG_IGN}, SIGXCPU, SA_NODEFER},
	{{SIG_IGN}, SIGXFSZ, SA_NODEFER},
	{{checkpointing}, SIGVTALRM, SA_NODEFER},
	{{SIG_DFL}, SIGPROF, SA_NODEFER | SA_RESETHAND},
	{{SIG_IGN}, SIGWINCH, SA_NODEFER},
	{{shutdown_request}, SIGUSR1, SA_NODEFER},
	{{SIG_IGN}, SIGUSR2, SA_NODEFER}
#else
	{{SIG_DFL}, {{0}}, SA_NODEFER | SA_RESETHAND, NULL},
	{{SIG_IGN}, {{SIGHUP}}, SA_NODEFER, NULL},
	{{shutdown_request}, {{SIGINT}}, SA_NODEFER, NULL},
	{{SIG_IGN}, {{SIGQUIT}}, SA_NODEFER, NULL},
	{{SIG_DFL}, {{SIGILL}}, SA_NODEFER | SA_RESETHAND, NULL},
	{{SIG_DFL}, {{SIGTRAP}}, SA_NODEFER | SA_RESETHAND, NULL},
	{{SIG_DFL}, {{SIGABRT}}, SA_NODEFER | SA_RESETHAND, NULL},
	{{SIG_DFL}, {{SIGBUS}}, SA_NODEFER | SA_RESETHAND, NULL},
	{{SIG_DFL}, {{SIGFPE}}, SA_NODEFER | SA_RESETHAND, NULL},
	{{SIG_DFL}, {{SIGKILL}}, SA_NODEFER | SA_RESETHAND, NULL},
	{{shutdown_request}, {{SIGUSR1}}, SA_NODEFER | SA_RESETHAND, NULL},
	{{SIG_DFL}, {{SIGSEGV}}, SA_NODEFER | SA_RESETHAND, NULL},
	{{SIG_IGN}, {{SIGUSR2}}, SA_NODEFER | SA_RESETHAND, NULL},
	{{SIG_IGN}, {{SIGPIPE}}, SA_NODEFER, NULL},
	{{SIG_IGN}, {{SIGALRM}}, SA_NODEFER, NULL},
	{{SIG_IGN}, {{SIGTERM}}, SA_NODEFER, NULL},
	{{SIG_DFL}, {{0}}, SA_NODEFER | SA_RESETHAND, NULL},   /* Not listed by kill -l ??? */
	{{SIG_IGN}, {{SIGCHLD}}, SA_NODEFER, NULL},
	{{SIG_IGN}, {{SIGCONT}}, SA_NODEFER, NULL},
	{{SIG_IGN}, {{SIGSTOP}}, SA_NODEFER, NULL},
	{{SIG_IGN}, {{SIGTSTP}}, SA_NODEFER, NULL},
	{{SIG_IGN}, {{SIGTTIN}}, SA_NODEFER, NULL},
	{{SIG_IGN}, {{SIGTTOU}}, SA_NODEFER, NULL},
	{{SIG_IGN}, {{SIGURG}}, SA_NODEFER, NULL},
	{{SIG_IGN}, {{SIGXCPU}}, SA_NODEFER, NULL},
	{{SIG_IGN}, {{SIGXFSZ}}, SA_NODEFER, NULL},
	{{checkpointing}, {{SIGVTALRM}}, SA_NODEFER, NULL},
	{{SIG_DFL}, {{SIGPROF}}, SA_NODEFER | SA_RESETHAND, NULL},
	{{SIG_IGN}, {{SIGWINCH}}, SA_NODEFER, NULL},
	{{SIG_IGN}, {{SIGIO}}, SA_NODEFER, NULL},
	{{shutdown_request}, {{SIGPWR}}, SA_NODEFER, NULL},
	{{SIG_IGN}, {{SIGSYS}}, SA_NODEFER, NULL}
#endif
    };
#else
    if (DEBUG > 2)
	log_info("called %s with no arguments", __PRETTY_FUNCTION__);

    signal(SIGHUP, SIG_IGN);
    signal(SIGINT, shutdown_request);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGUSR1, shutdown_request);
    signal(SIGUSR2, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGALRM, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);
    signal(SIGCONT, SIG_IGN);
    signal(SIGSTOP, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGURG, SIG_IGN);
    signal(SIGXCPU, SIG_IGN);
    signal(SIGXFSZ, SIG_IGN);

    signal(SIGWINCH, SIG_IGN);
    signal(SIGIO, SIG_IGN);
    signal(SIGPWR, shutdown_request);
    signal(SIGSYS, SIG_IGN);

#endif

    /*
     * set up the deadlock-protection 
     */

    interval.tv_sec = 900;				       /* 30 minutes */
    interval.tv_usec = 0;
    itime.it_interval = interval;
    itime.it_value = interval;
    setitimer(ITIMER_VIRTUAL, &itime, 0);
    signal(SIGVTALRM, checkpointing);

#ifdef USE_SIGACTION
    for (i = 1; i < 32; i++)
	if (ack[i].sa_handler != SIG_DFL)
	    sigaction(i, &ack[i], NULL);
#endif
}

void checkpointing(int a)
{
    if (DEBUG > 3)
	log_info("called %s with %d", __PRETTY_FUNCTION__, a);

    if (!tics) {
	log_fatal("CHECKPOINT shutdown: tics not updated");
	close_sockets(0);
	close_whod();
	proper_exit(MUD_HALT);
    } else
	tics = 0;
}

void shutdown_request(int a)
{
    if (DEBUG > 3)
	log_info("called %s with %d", __PRETTY_FUNCTION__, a);

    log_fatal("Received USR1 - shutdown request");
    close_sockets(0);
    close_whod();
    proper_exit(MUD_HALT);
}

void reboot_request(int a)
{
    if (DEBUG > 3)
	log_info("called %s with %d", __PRETTY_FUNCTION__, a);

    log_fatal("Received SIGHUP, or SIGTERM. Shutting down");
    close_sockets(0);
    close_whod();
    proper_exit(MUD_REBOOT);				       /* something more elegant should perhaps be substituted */
}

void logsig(int a)
{
    if (DEBUG > 3)
	log_info("called %s with %d", __PRETTY_FUNCTION__, a);

    log_info("Signal received. Ignoring.");
}

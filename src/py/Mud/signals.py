# -*- coding: utf-8 -*- line endings: unix -*-
__author__ = 'quixadhal'

from Mud.log_system import Logger
logger = Logger()

import signal

def setup_signals():
    signal.signal(signal.SIGHUP, signal.SIG_IGN)
    signal.signal(signal.SIGINT, shutdown_request)
    signal.signal(signal.SIGQUIT, signal.SIG_IGN)
    signal.signal(signal.SIGUSR1, shutdown_request)
    signal.signal(signal.SIGUSR2, signal.SIG_IGN)
    signal.signal(signal.SIGPIPE, signal.SIG_IGN)
    signal.signal(signal.SIGALRM, signal.SIG_IGN)
    signal.signal(signal.SIGTERM, signal.SIG_IGN)
    signal.signal(signal.SIGCHLD, reaper);
    signal.signal(signal.SIGCONT, signal.SIG_IGN)
    #signal.signal(signal.SIGSTOP, signal.SIG_IGN)
    signal.signal(signal.SIGTSTP, signal.SIG_IGN)
    signal.signal(signal.SIGTTIN, signal.SIG_IGN)
    signal.signal(signal.SIGTTOU, signal.SIG_IGN)
    signal.signal(signal.SIGURG, signal.SIG_IGN)
    signal.signal(signal.SIGXCPU, signal.SIG_IGN)
    signal.signal(signal.SIGXFSZ, signal.SIG_IGN)

    signal.signal(signal.SIGWINCH, signal.SIG_IGN)
    signal.signal(signal.SIGIO, signal.SIG_IGN)
    signal.signal(signal.SIGPWR, shutdown_request)
    signal.signal(signal.SIGSYS, signal.SIG_IGN)
    signal.setitimer(signal.ITIMER_VIRTUAL, 900, 900)
    signal.signal(signal.SIGVTALRM, checkpointing)
    logger.boot('Signal handlers setup.')


def checkpointing(signum, frame):
    logger.info("CHECKPOINT")
    #logger.info("CHECKPOINT %d", tics)

    # If tics is 0, we're somehow not updating the tick counter, so exit

    #log_fatal("CHECKPOINT shutdown: tics not updated");
    #close_sockets(0);
    #close_whod();
    #proper_exit(MUD_HALT);


def shutdown_request(signum, frame):
    logger.fatal("Received SIGUSR1.  Shutting down.")
    #close_sockets(0);
    #close_whod();
    #proper_exit(MUD_HALT);


def reboot_request(signum, frame):
    logger.fatal("Received SIGHUP or SIGTERM.  Rebooting.")
    #close_sockets(0);
    #close_whod();
    #proper_exit(MUD_REBOOT);


def reaper(signum, frame):
    logger.info("Child died.  Reaping.");
    #while((pid = waitpid(-1, &status, WNOHANG)) > 0)
    #    ;
    logger.info("Child reaped.");


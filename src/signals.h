#ifndef _DIKU_SIGNALS_H
#define _DIKU_SIGNALS_H

void signal_setup(void);
#ifdef sun3
void checkpointing(void);
void shutdown_request(void);
void hupsig(void);
void logsig(void);
#else
void checkpointing(int a);
void shutdown_request(int a);
void hupsig(int a);
void logsig(int a);
#endif

#endif

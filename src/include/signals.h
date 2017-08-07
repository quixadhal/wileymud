#ifndef _DIKU_SIGNALS_H
#define _DIKU_SIGNALS_H

void                                    signal_setup(void);

#ifdef sun3
void                                    checkpointing(void);
void                                    shutdown_request(void);
void                                    reboot_request(void);
void                                    logsig(void);
void                                    reaper(void);
void                                    exit_with_traceback(void);
#else
void                                    checkpointing(int a);
void                                    shutdown_request(int a);
void                                    reboot_request(int a);
void                                    logsig(int a);
void                                    reaper(int a);
void                                    exit_with_traceback(int a);
#endif

#endif

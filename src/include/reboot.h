#ifndef _REBOOT_H
#define _REBOOT_H

struct reboot_data {
    time_t  updated;
    int     enabled;
    time_t  next_reboot;
    time_t  frequency;
    char    set_by[MAX_INPUT_LENGTH];
    time_t  last_message;   // temporary field, when did the last warning mesage go out?
    char    next_reboot_text[MAX_INPUT_LENGTH]; // temporary field to avoid asking SQL
};

#ifndef _REBOOT_C
extern struct reboot_data               reboot;
extern int                              WizLock;
extern int                              diku_shutdown;
extern int                              diku_reboot;
#endif

void setup_reboot_table(void);
void load_reboot(void);
int set_first_reboot(void);
int set_next_reboot(void);
int toggle_reboot(struct char_data *ch);
int set_reboot_interval(struct char_data *ch, const char *mode, int number);
void check_reboot(void);
void do_reboot(struct char_data *ch, const char *argument, int cmd);
void do_shutdown(struct char_data *ch, const char *argument, int cmd);
void do_shutdow(struct char_data *ch, const char *argument, int cmd);
void do_setreboot(struct char_data *ch, const char *argument, int cmd);

#endif


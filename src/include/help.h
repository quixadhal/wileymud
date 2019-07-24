#ifndef _HELP_H
#define _HELP_H

struct help_keyword {
    char    keyword[MAX_INPUT_LENGTH];
    int     id;
};

struct help_message {
    time_t                  updated;
    int                     immortal;
    char                    set_by[MAX_INPUT_LENGTH];
    int                     id;
    char                    message[MAX_STRING_LENGTH];
};

#ifndef _HELP_C
extern struct help_keyword *help_keywords;
extern stringMap *help_messages;
extern int help_message_count;
extern int help_keyword_count;
#endif

void setup_help_table(void);
void load_help(void);
int find_help_by_keyword(const char *keyword);
struct help_message *get_help_by_id(const int id);
void do_help(struct char_data *ch, const char *argument, int cmd);
void do_wizhelp(struct char_data *ch, const char *argument, int cmd);

#endif


#ifndef _BOARD_H
#define _BOARD_H

// For now, I've added the following fields to struct descriptor_data
// to avoid the "board_kludge" nonsense.
//
//      int     board_vnum;
//      char    *board_subject;
//      char    *board_body;

struct board_message_data
{
    time_t updated;
    int vnum;
    int message_id;
    char owner[MAX_INPUT_LENGTH];
    char subject[MAX_INPUT_LENGTH];
    char body[MAX_STRING_LENGTH];
};

struct board_data
{
    time_t updated;
    int vnum;
    int message_count;
    struct board_message_data *messages;
};

#ifndef _BOARD_C
extern int board_count;
extern struct board_data *boards;
#endif

void setup_board_table(void);
void load_boards(void);
struct board_data *find_board_in_room(int room);
struct board_data *find_board_by_vnum(int vnum);
struct board_data *create_board(int vnum);
void import_board(struct board_data *bp);
int show_board(struct char_data *ch, const char *arg, struct board_data *bp);
int show_board_message(struct char_data *ch, const char *arg, struct board_data *bp);
int delete_board_message(struct char_data *ch, const char *arg, struct board_data *bp);
int begin_write_board_message(struct char_data *ch, const char *arg, struct board_data *bp);
int finish_write_board_message(struct char_data *ch);
int board_special(struct char_data *ch, int cmd, const char *arg);

#define BOARD_DIR "boards"

#if 0
#define MAX_MSGS 250            /* Max number of messages.  */
#define MAX_MESSAGE_LENGTH 2048 /* that should be enough */

struct Board {
  char                                   *msgs[MAX_MSGS];
  char                                   *head[MAX_MSGS];
  int                                     msg_num;
  int                                     Vnum;		       /* Virtual # of object that this board hooks to */
  struct Board                           *next;
};

#ifndef _BOARD_C
extern struct char_data                *board_kludge_char;
extern struct Board                    *board_list;

#endif

void                                    InitBoards(void);
void                                    InitABoard(struct obj_data *obj);
struct Board                           *FindBoardInRoom(int room);
int                                     board(struct char_data *ch, int cmd, char *arg);
void                                    board_write_msg(struct char_data *ch, char *arg,
							struct Board *b);
int                                     board_remove_msg(struct char_data *ch, char *arg,
							 struct Board *b);
void                                    board_save_board(struct Board *b);
void                                    board_load_board(struct Board *b);
void                                    board_reset_board(struct Board *b);
int                                     board_display_msg(struct char_data *ch, char *arg,
							  struct Board *b);
int                                     board_show_board(struct char_data *ch, char *arg,
							 struct Board *b);
void                                    save_board(struct Board *b);
#endif

#endif

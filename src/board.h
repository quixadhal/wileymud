#ifndef _BOARD_H
#define _BOARD_H

#define MAX_MSGS 50	               /* Max number of messages.          */
#define MAX_MESSAGE_LENGTH 2048     /* that should be enough            */
  
struct Board {
    char *msgs[MAX_MSGS];
    char *head[MAX_MSGS];
    int msg_num;
    char filename[40];
    FILE *file;  /* file that is opened */
    int Rnum;    /* Real # of object that this board hooks to */
    struct Board *next;
};

#ifndef _BOARD_C
extern struct char_data *board_kludge_char;
extern struct Board *board_list;
#endif

void InitBoards();
void InitABoard( struct obj_data *obj);
void OpenBoardFile(struct Board *b);
struct Board *FindBoardInRoom(int room);
int board(struct char_data *ch, int cmd, char *arg);
void board_write_msg(struct char_data *ch, char *arg, struct Board *b) ;
int board_remove_msg(struct char_data *ch, char *arg, struct Board *b) ;
void board_save_board(struct Board *b) ;
void board_load_board(struct Board *b) ;
void board_reset_board(struct Board *b) ;
int board_display_msg(struct char_data *ch, char *arg, struct Board *b) ;
void board_fix_long_desc(struct Board *b) ;
int board_show_board(struct char_data *ch, char *arg, struct Board *b);

#endif

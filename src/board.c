#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <time.h>

#include "include/global.h"
#include "include/bug.h"
#include "include/utils.h"
#include "include/comm.h"
#include "include/db.h"
#include "include/interpreter.h"
#include "include/multiclass.h"
#include "include/modify.h"
#include "include/handler.h"
#include "include/act_info.h"
#define _BOARD_C
#include "include/board.h"

struct char_data *board_kludge_char;
struct Board *board_list;

void InitBoards()
{
  if (DEBUG)
    dlog("InitBoards");
  /* this is called at the very beginning, like shopkeepers */
  board_list = 0;

}

void InitABoard(struct obj_data *obj)
{
  struct Board *new_board, *tmp;

  if (DEBUG)
    dlog("InitABoard");
  if (board_list) {
    /*
     **  try to match a board with an existing board in the game
     */
    for (tmp = board_list; tmp; tmp = tmp->next) {
      if (tmp->Rnum == obj->item_number) {
	/*
	 **  board has been matched, load and ignore it.
	 */
	board_load_board(tmp);
	return;
      }
    }
  }

  /*  
   * new_board = (struct Board *)malloc(sizeof(*new_board));
   * if (!new_board) {
   *   perror("InitABoard(malloc)");
   *   exit(0);
   * }
   * bzero(new_board->head, sizeof(new_board->head));
   * bzero(new_board->msgs, sizeof(new_board->msgs));
   */ 

  CREATE(new_board, struct Board, 1);
  new_board->Rnum = obj->item_number;
  sprintf(new_board->filename, "%d.messages", obj_index[obj->item_number].virtual);
  OpenBoardFile(new_board);
  board_load_board(new_board);

  /*
   **  add our new board to the beginning of the list
   */

  tmp = board_list;
  new_board->next = tmp;
  board_list = new_board;
  CloseBoardFile(new_board);
}

void OpenBoardFile(struct Board *b)
{
  char buf[500];

  if (DEBUG)
    dlog("OpenBoardFile");
  sprintf(buf, "%s/%s", BOARD_FILE_PATH, b->filename);
  b->file = fopen(buf, "r+");

  if (!b->file) {
    perror("OpenBoardFile(fopen)");
    exit(0);
  }
}

void CloseBoardFile(struct Board *b)
{
  if (DEBUG)
    dlog("CloseBoardFile");
  if(!b || !b->file) {
    bug("CloseBoardFile");
    exit(0);
  }
  fclose(b->file);
}

struct Board *FindBoardInRoom(int room)
{
  struct obj_data *o;
  struct Board *nb;

  if (DEBUG)
    dlog("FindBoardInRoom");
  if (!real_roomp(room))
    return (NULL);

  for (o = real_roomp(room)->contents; o;
       o = o->next_content) {
    if (obj_index[o->item_number].func == (ifuncp)board) {
      for (nb = board_list; nb; nb = nb->next) {
	if (nb->Rnum == o->item_number)
	  return (nb);
      }
      return (NULL);
    }
  }
  return (NULL);
}

int board(struct char_data *ch, int cmd, char *arg)
{
  struct Board *nb;

  if (DEBUG)
    dlog("board");
  nb = FindBoardInRoom(ch->in_room);

  if (!nb)
    return (FALSE);

  if (!ch->desc)
    return (FALSE);

  switch (cmd) {
  case CMD_north:
  case CMD_east:
  case CMD_south:
  case CMD_west:
  case CMD_up:
  case CMD_down:
  case CMD_who:
  case CMD_tell:
  case CMD_exits:
  case CMD_goto:
    return FALSE;
    break;
  case CMD_look:			       /* look */
    if(!board_show_board(ch, arg, nb)) /* no args, or failed */
      do_look(ch, "", 0);
    return TRUE;
    break;
  case CMD_write:			       /* write */
    board_write_msg(ch, arg, nb);
    return TRUE;
    break;
  case CMD_read:			       /* read */
    board_display_msg(ch, arg, nb);
    return TRUE;
    break;
  case CMD_remove:			       /* remove */
    board_remove_msg(ch, arg, nb);
    return TRUE;
    break;
  default:
    return TRUE;
  }
}

void board_write_msg(struct char_data *ch, char *arg, struct Board *b)
{
  char new_arg[60];
  char time_str[60];
  struct tm *tm_info;
  time_t tc;

  if (DEBUG)
    dlog("board_write_msg");
  if (b->msg_num > MAX_MSGS - 1) {
    cprintf(ch, "The board is full already.\n\r");
    return;
  }
  if (board_kludge_char) {
    cprintf(ch, "Sorry, but someone has stolen the pen.. wait a few minutes.\n\r");
    return;
  }
  /* skip blanks */

  for (; isspace(*arg); arg++);

  if (!*arg) {
    cprintf(ch, "We must have a headline!\n\r");
    return;
  }
  board_kludge_char = ch;

  tc = time(0);
  tm_info = (struct tm *)localtime(&tc);

  /* Quixadhal:  Why 70? */
  /* +4 is for a space and '()' around the character name. */
  if(!TRY_TO_CREATE(b->head[b->msg_num], char, 70 + strlen(GET_NAME(ch)) + 4)) {
    bug("Malloc for board header failed.");
    cprintf(ch, "The board is malfunctioning - sorry.\n\r");
    return;
  }
  strncpy(new_arg, arg, 40);
  sprintf(time_str, "%s", asctime(tm_info));
  time_str[strlen(time_str) - 1] = '\0';
  sprintf(b->head[b->msg_num], "%s (%s) %s", new_arg, GET_NAME(ch), time_str);
  b->msgs[b->msg_num] = NULL;

  cprintf(ch, "Write your message. Terminate with an @.\n\r\n\r");
  act("$n starts to write a message.", TRUE, ch, 0, 0, TO_ROOM);

  ch->desc->str = &b->msgs[b->msg_num];
  ch->desc->max_str = MAX_MESSAGE_LENGTH;

  b->msg_num++;
}

int board_remove_msg(struct char_data *ch, char *arg, struct Board *b)
{
  int ind, msg;
  char msg_number[MAX_INPUT_LENGTH];

  if (DEBUG)
    dlog("board_remove_msg");
  one_argument(arg, msg_number);

  if (!*msg_number || !isdigit(*msg_number))
    return (0);
  if (!(msg = atoi(msg_number)))
    return (0);
  if (!b->msg_num) {
    cprintf(ch, "The board is empty!\n\r");
    return (1);
  }
  if (msg < 1 || msg > b->msg_num) {
    cprintf(ch, "That message exists only in your imagination..\n\r");
    return (1);
  }
  if (GetMaxLevel(ch) < 19) {
    cprintf(ch, "Due to misuse of the REMOVE command, only 19th level\n\r");
    cprintf(ch, "and above can remove messages.\n\r");
    return 1;
  }
  ind = msg;
  DESTROY(b->head[--ind]);
  /* if (b->msgs[ind]) */
    DESTROY(b->msgs[ind]);
  for (; ind < b->msg_num - 1; ind++) {
    b->head[ind] = b->head[ind + 1];
    b->msgs[ind] = b->msgs[ind + 1];
  }
  b->msg_num--;
  cprintf(ch, "Message removed.\n\r");
  board_save_board(b);
  return (1);
}

void board_save_board(struct Board *b)
{
  int ind, len;

  if (DEBUG)
    dlog("board_save_board");
  if (!b)
    return;

  if (!b->msg_num) {
    log("No messages to save.\n\r");
    return;
  }
  OpenBoardFile(b);

  fwrite(&b->msg_num, sizeof(int), 1, b->file);

  for (ind = 0; ind < b->msg_num; ind++) {
    len = strlen(b->head[ind]) + 1;
    fwrite(&len, sizeof(int), 1, b->file);
    fwrite(b->head[ind], sizeof(char), len, b->file);

    if (!b->msgs[ind]) {
      CREATE(b->msgs[ind], char, 50);			/* Quixadhal: Why 50? */
      strcpy(b->msgs[ind], "Generic Message");
    }
    len = strlen(b->msgs[ind]) + 1;
    fwrite(&len, sizeof(int), 1, b->file);
    fwrite(b->msgs[ind], sizeof(char), len, b->file);
  }
  CloseBoardFile(b);
  board_fix_long_desc(b);
  return;
}

void board_load_board(struct Board *b)
{
  int ind, len = 0;

  if (DEBUG)
    dlog("board_load_board");
  OpenBoardFile(b);
  board_reset_board(b);

  fread(&b->msg_num, sizeof(int), 1, b->file);

  if (b->msg_num < 1 || b->msg_num > MAX_MSGS || feof(b->file)) {
    log("Board-message file corrupt or nonexistent.\n\r");
    CloseBoardFile(b);
    return;
  }
  for (ind = 0; ind < b->msg_num; ind++) {
    fread(&len, sizeof(int), 1, b->file);

    if(!TRY_TO_CREATE(b->head[ind], char, len + 1)) {
      bug("Malloc for board header failed.");
      board_reset_board(b);
      CloseBoardFile(b);
      return;
    }
    fread(b->head[ind], sizeof(char), len, b->file);
    fread(&len, sizeof(int), 1, b->file);

    if(!TRY_TO_CREATE(b->msgs[ind], char, len + 1)) {
      bug("Malloc for board msg failed.");
      board_reset_board(b);
      CloseBoardFile(b);
      return;
    }
    fread(b->msgs[ind], sizeof(char), len, b->file);
  }
  CloseBoardFile(b);
  board_fix_long_desc(b);
  return;
}

void board_reset_board(struct Board *b)
{
  int ind;

  if (DEBUG)
    dlog("board_reset_board");
  for (ind = 0; ind < MAX_MSGS; ind++) {
    /* if (b->head[ind]) */
      DESTROY(b->head[ind]);
    /* if (b->msgs[ind]) */
      DESTROY(b->msgs[ind]);
    b->head[ind] = b->msgs[ind] = NULL;
  }
  b->msg_num = 0;
  board_fix_long_desc(b);
  return;
}

int board_display_msg(struct char_data *ch, char *arg, struct Board *b)
{
  char msg_number[MAX_INPUT_LENGTH], buffer[MAX_STRING_LENGTH];
  int msg;

  if (DEBUG)
    dlog("board_display_msg");
  one_argument(arg, msg_number);
  if (!*msg_number || !isdigit(*msg_number))
    return (0);
  if (!(msg = atoi(msg_number)))
    return (0);
  if (!b->msg_num) {
    cprintf(ch, "The board is empty!\n\r");
    return (1);
  }
  if (msg < 1 || msg > b->msg_num) {
    cprintf(ch, "That message exists only in your imagination..\n\r");
    return (1);
  }
  /* Bad news */
  sprintf(buffer, "Message %d : %s\n\r\n\r%s", msg, b->head[msg - 1],
	  b->msgs[msg - 1]);
  page_string(ch->desc, buffer, 1);
  return (1);
}

void board_fix_long_desc(struct Board *b)
{
  if (DEBUG)
    dlog("board_fix_long_desc");
  return;
}

int board_show_board(struct char_data *ch, char *arg, struct Board *b)
{
  int i;
  char buf[MAX_STRING_LENGTH], tmp[MAX_INPUT_LENGTH];

  if (DEBUG)
    dlog("board_show_board");
  one_argument(arg, tmp);

  if (!*tmp || !isname(tmp, "board bulletin"))
    return (0);

  if (board_kludge_char) {
    cprintf(ch, "Sorry, but someone is writing a message\n\r");
    return (0);
  }
  strcpy(buf, "This is a bulletin board. Usage: READ/REMOVE <messg #>, WRITE <header>\n\r");
  if (!b->msg_num) {
    strcat(buf, "The board is empty.\n\r");
  } else if (b->msg_num == 1) {
    sprintf(buf + strlen(buf), "There is 1 message on the board.\n\r");
  } else {
    sprintf(buf + strlen(buf), "There are %d messages on the board.\n\r", b->msg_num);
  }
  for (i = 0; i < b->msg_num; i++)
    sprintf(buf + strlen(buf), "%-2d : %s\n\r", i + 1, b->head[i]);

  page_string(ch->desc, buf, 1);

  return (1);
}

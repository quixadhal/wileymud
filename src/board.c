#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>

#include "global.h"
#include "bug.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#define _BOARD_C
#include "board.h"

struct char_data *board_kludge_char;
struct Board *board_list;

void InitBoards()
{
  struct obj_data *obj;

  if (DEBUG)
    dlog("InitBoards");
  /*
   **  this is called at the very beginning, like shopkeepers
   */
  board_list = 0;

}

void InitABoard(struct obj_data *obj)
{
  struct Board *new, *tmp;

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
  new = (struct Board *)malloc(sizeof(*new));
  if (!new) {
    perror("InitABoard(malloc)");
    exit(0);
  }
  bzero(new->head, sizeof(new->head));
  bzero(new->msgs, sizeof(new->msgs));

  new->Rnum = obj->item_number;

  sprintf(new->filename, "%d.messages", obj_index[obj->item_number].virtual);

  OpenBoardFile(new);

  board_load_board(new);

  /*
   **  add our new board to the beginning of the list
   */

  tmp = board_list;
  new->next = tmp;
  board_list = new;

  fclose(new->file);
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
    if (obj_index[o->item_number].func == board) {
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
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
    return FALSE;
    break;
  case 15:			       /* look */
    board_show_board(ch, arg, nb);
    return TRUE;
    break;
  case 149:			       /* write */
    board_write_msg(ch, arg, nb);
    return TRUE;
    break;
  case 63:			       /* read */
    board_display_msg(ch, arg, nb);
    return TRUE;
    break;
  case 66:			       /* remove */
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
  char *ptr, time_str[60];
  struct tm *tm_info;
  time_t tc;

  if (DEBUG)
    dlog("board_write_msg");
  if (b->msg_num > MAX_MSGS - 1) {
    send_to_char("The board is full already.\n\r", ch);
    return;
  }
  if (board_kludge_char) {
    send_to_char("Sorry, but someone has stolen the pen.. wait a few minutes.\n\r", ch);
    return;
  }
  /* skip blanks */

  for (; isspace(*arg); arg++);

  if (!*arg) {
    send_to_char("We must have a headline!\n\r", ch);
    return;
  }
  board_kludge_char = ch;

  tc = time(0);
  tm_info = (struct tm *)localtime(&tc);

  /* b->head[b->msg_num] = (char *)malloc(strlen(arg) + strlen(GET_NAME(ch)) + 4); */
  b->head[b->msg_num] = (char *)malloc(70 + strlen(GET_NAME(ch)) + 4);

  /* +4 is for a space and '()' around the character name. */

  if (!b->head[b->msg_num]) {
    log("Malloc for board header failed.\n\r");
    send_to_char("The board is malfunctioning - sorry.\n\r", ch);
    return;
  }
  strncpy(new_arg, arg, 40);
  sprintf(time_str, "%s", asctime(tm_info));
  time_str[strlen(time_str) - 1] = '\0';
  sprintf(b->head[b->msg_num], "%s (%s) %s", new_arg, GET_NAME(ch), time_str);
  b->msgs[b->msg_num] = NULL;

  send_to_char("Write your message. Terminate with an @.\n\r\n\r", ch);
  act("$n starts to write a message.", TRUE, ch, 0, 0, TO_ROOM);

  ch->desc->str = &b->msgs[b->msg_num];
  ch->desc->max_str = MAX_MESSAGE_LENGTH;

  b->msg_num++;
}

int board_remove_msg(struct char_data *ch, char *arg, struct Board *b)
{
  int ind, msg;
  char buf[256], number[MAX_INPUT_LENGTH];

  if (DEBUG)
    dlog("board_remove_msg");
  one_argument(arg, number);

  if (!*number || !isdigit(*number))
    return (0);
  if (!(msg = atoi(number)))
    return (0);
  if (!b->msg_num) {
    send_to_char("The board is empty!\n\r", ch);
    return (1);
  }
  if (msg < 1 || msg > b->msg_num) {
    send_to_char("That message exists only in your imagination..\n\r",
		 ch);
    return (1);
  }
  if (GetMaxLevel(ch) < 19) {
    send_to_char("Due to misuse of the REMOVE command, only 19th level\n\r", ch);
    send_to_char("and above can remove messages.\n\r", ch);
    return;
  }
  ind = msg;
  free(b->head[--ind]);
  if (b->msgs[ind])
    free(b->msgs[ind]);
  for (; ind < b->msg_num - 1; ind++) {
    b->head[ind] = b->head[ind + 1];
    b->msgs[ind] = b->msgs[ind + 1];
  }
  b->msg_num--;
  send_to_char("Message removed.\n\r", ch);
  send_to_char(buf, ch);
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
      if (b->msgs[ind] = (char *)malloc(50)) {
	strcpy(b->msgs[ind], "Generic Message");
      } else {
	exit(1);
      }
    }
    len = strlen(b->msgs[ind]) + 1;
    fwrite(&len, sizeof(int), 1, b->file);
    fwrite(b->msgs[ind], sizeof(char), len, b->file);
  }
  fclose(b->file);
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
    fclose(b->file);
    return;
  }
  for (ind = 0; ind < b->msg_num; ind++) {
    fread(&len, sizeof(int), 1, b->file);

    b->head[ind] = (char *)malloc(len + 1);
    if (!b->head[ind]) {
      log("Malloc for board header failed.\n\r");
      board_reset_board(b);
      fclose(b->file);
      return;
    }
    fread(b->head[ind], sizeof(char), len, b->file);
    fread(&len, sizeof(int), 1, b->file);

    b->msgs[ind] = (char *)malloc(len + 1);
    if (!b->msgs[ind]) {
      log("Malloc for board msg failed..\n\r");
      board_reset_board(b);
      fclose(b->file);
      return;
    }
    fread(b->msgs[ind], sizeof(char), len, b->file);
  }
  fclose(b->file);
  board_fix_long_desc(b);
  return;
}

void board_reset_board(struct Board *b)
{
  int ind;

  if (DEBUG)
    dlog("board_reset_board");
  for (ind = 0; ind < MAX_MSGS; ind++) {
    if (b->head[ind])
      free(b->head[ind]);
    if (b->msgs[ind])
      free(b->msgs[ind]);
    b->head[ind] = b->msgs[ind] = NULL;
  }
  b->msg_num = 0;
  board_fix_long_desc(b);
  return;
}

int board_display_msg(struct char_data *ch, char *arg, struct Board *b)
{
  char buf[512], number[MAX_INPUT_LENGTH], buffer[MAX_STRING_LENGTH];
  int msg;

  if (DEBUG)
    dlog("board_display_msg");
  one_argument(arg, number);
  if (!*number || !isdigit(*number))
    return (0);
  if (!(msg = atoi(number)))
    return (0);
  if (!b->msg_num) {
    send_to_char("The board is empty!\n\r", ch);
    return (1);
  }
  if (msg < 1 || msg > b->msg_num) {
    send_to_char("That message exists only in your imagination..\n\r",
		 ch);
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
    send_to_char("Sorry, but someone is writing a message\n\r", ch);
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

#include <stdio.h>
#include <stdlib.h>
/* #include <unistd.h> */
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <time.h>

#include "global.h"
#include "bug.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "interpreter.h"
#include "multiclass.h"
#include "modify.h"
#include "handler.h"
#include "act_info.h"
#define _BOARD_C
#include "board.h"

struct char_data                       *board_kludge_char = NULL;
struct Board                           *board_list = NULL;

void InitBoards()
{
  if (DEBUG > 2)
    log_info("called %s with no arguments", __PRETTY_FUNCTION__);

  /*
   * this is called at the very beginning, like shopkeepers 
   */
  board_list = NULL;

}

void InitABoard(struct obj_data *obj)
{
  struct Board                           *new_board = NULL;
  struct Board                           *tmp = NULL;

  if (DEBUG > 2)
    log_info("called %s with %s", __PRETTY_FUNCTION__, SAFE_ONAME(obj));

  if (board_list) {
    /*
     **  try to match a board with an existing board in the game
     */
    for (tmp = board_list; tmp; tmp = tmp->next) {
      if (tmp->Vnum == obj->item_number) {
        board_load_board(tmp);
	return;
      }
    }
  }

  CREATE(new_board, struct Board, 1);
  new_board->Vnum = obj_index[obj->item_number].virtual;
  board_load_board(new_board);

  tmp = board_list;
  new_board->next = tmp;
  board_list = new_board;
}

struct Board                           *FindBoardInRoom(int room)
{
  struct obj_data                        *o = NULL;
  struct Board                           *nb = NULL;

  if (DEBUG > 2)
    log_info("called %s with %d", __PRETTY_FUNCTION__, room);

  if (!real_roomp(room))
    return (NULL);

  for (o = real_roomp(room)->contents; o; o = o->next_content) {
    if (obj_index[o->item_number].func == (ifuncp)board) {
      for (nb = board_list; nb; nb = nb->next) {
	if (nb->Vnum == obj_index[o->item_number].virtual)
	  return (nb);
      }
      return (NULL);
    }
  }
  return (NULL);
}

/*
 * This is a special procedure, the cmd/arg arguments must be in this order!
 */
int board(struct char_data *ch, int cmd, char *arg)
{
  struct Board                           *nb = NULL;

  if (DEBUG > 2)
    log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(arg), cmd);

  nb = FindBoardInRoom(ch->in_room);

  if (!nb)
    return (FALSE);

  if (!ch->desc)
    return (FALSE);

  switch (cmd) {
    // Mortal commands allowed
    case CMD_autoexit:
    case CMD_bug:
    case CMD_credits:
    case CMD_down:
    case CMD_east:
    case CMD_exits:
    case CMD_gtell:
    case CMD_idea:
    case CMD_info:
    case CMD_north:
    case CMD_nosummon:
    case CMD_noteleport:
    case CMD_pager:
    case CMD_quit:
    case CMD_return:
    case CMD_save:
    case CMD_south:
    case CMD_tell:
    case CMD_time:
    case CMD_typo:
    case CMD_up:
    case CMD_users:
    case CMD_version:
    case CMD_west:
    case CMD_where:
    case CMD_who:
    case CMD_whozone:
    case CMD_wimp:
    case CMD_wizlist:
      return FALSE;
      break;
    // Immortal commands allowed
    case CMD_at:
    case CMD_goto:
    case CMD_immtrack:
    case CMD_invisible:
    case CMD_nohassle:
    case CMD_players:
    case CMD_reboot:
    case CMD_rentmode:
    case CMD_reset:
    case CMD_restore:
    case CMD_restoreall:
    case CMD_show:
    case CMD_shutdown:
    case CMD_snoop:
    case CMD_switch:
    case CMD_ticks:
    case CMD_transfer:
    case CMD_wizhelp:
    case CMD_wizlock:
    case CMD_wiznet:
    case CMD_world:
    case CMD_zpurge:
      return FALSE;
      break;
    // Special case commands
    case CMD_look:					       /* look */
      if (!board_show_board(ch, arg, nb))		       /* no args, or failed */
	do_look(ch, "", 0);
      return TRUE;
      break;
    case CMD_write:					       /* write */
      board_write_msg(ch, arg, nb);
      return TRUE;
      break;
    case CMD_read:					       /* read */
      board_display_msg(ch, arg, nb);
      return TRUE;
      break;
    case CMD_remove:					       /* remove */
      board_remove_msg(ch, arg, nb);
      return TRUE;
      break;
    // Anything else just fails silently
    default:
      return TRUE;
  }
}

void board_write_msg(struct char_data *ch, char *arg, struct Board *b)
{
  char                                    new_arg[60] = "\0\0\0";
  char                                    time_str[60] = "\0\0\0";
  struct tm                              *tm_info = NULL;
  time_t                                  tc = 0;
  int                                     mlen = 0;

  if (DEBUG > 2)
    log_info("called %s with %s, %s, %08zx", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(arg), (size_t)b);

  if (b->msg_num > MAX_MSGS - 1) {
    cprintf(ch, "The board is full already.\r\n");
    return;
  }
  if (board_kludge_char) {
    cprintf(ch, "Sorry, but someone has stolen the pen.. wait a few minutes.\r\n");
    return;
  }

  for (; isspace(*arg); arg++);

  if (!*arg) {
    cprintf(ch, "We must have a headline!\r\n");
    return;
  }
  board_kludge_char = ch;

  tc = time(0);
  tm_info = (struct tm *)localtime(&tc);

  /*
   * +4 is for a space and '()' around the character name. 
   */

  mlen = 70 + strlen(GET_NAME(ch)) + 4;
  CREATE(b->head[b->msg_num], char, mlen);

  strlcpy(new_arg, arg, 40);
  snprintf(time_str, 60, "%s", asctime(tm_info));
  time_str[strlen(time_str) - 1] = '\0';
  snprintf(b->head[b->msg_num], mlen, "%s (%s) %s", new_arg, GET_NAME(ch), time_str);
  b->msgs[b->msg_num] = NULL;

  cprintf(ch, "Write your message. Terminate with an @.\r\n\r\n");
  act("$n starts to write a message.", TRUE, ch, 0, 0, TO_ROOM);

  ch->desc->str = &b->msgs[b->msg_num];
  ch->desc->max_str = MAX_MESSAGE_LENGTH;

  b->msg_num++;
}

int board_remove_msg(struct char_data *ch, char *arg, struct Board *b)
{
  int                                     ind = 0;
  int                                     msg = 0;
  char                                    msg_number[MAX_INPUT_LENGTH] = "\0\0\0";

  if (DEBUG > 2)
    log_info("called %s with %s, %s, %08zx", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(arg), (size_t)b);

  one_argument(arg, msg_number);

  if (!*msg_number || !isdigit(*msg_number))
    return (0);
  if (!(msg = atoi(msg_number)))
    return (0);
  if (!b->msg_num) {
    cprintf(ch, "The board is empty!\r\n");
    return (1);
  }
  if (msg < 1 || msg > b->msg_num) {
    cprintf(ch, "That message exists only in your imagination..\r\n");
    return (1);
  }
  if (GetMaxLevel(ch) < 19) {
    cprintf(ch, "Due to misuse of the REMOVE command, only 19th level\r\n");
    cprintf(ch, "and above can remove messages.\r\n");
    return 1;
  }
  ind = msg;
  ind--;
  DESTROY(b->head[ind]);
  DESTROY(b->msgs[ind]);
  for (; ind < b->msg_num - 1; ind++) {
    b->head[ind] = b->head[ind + 1];
    b->msgs[ind] = b->msgs[ind + 1];
  }
  b->msg_num--;
  board_save_board(b);
  cprintf(ch, "Message %d removed.\r\n", msg);
  return (1);
}

void board_reset_board(struct Board *b)
{
  int                                     ind = 0;

  if (DEBUG > 2)
    log_info("called %s with %08zx", __PRETTY_FUNCTION__, (size_t)b);

  for (ind = 0; ind < MAX_MSGS; ind++) {
    DESTROY(b->head[ind]);
    DESTROY(b->msgs[ind]);
    b->head[ind] = b->msgs[ind] = NULL;
  }
  b->msg_num = 0;
  return;
}

int board_display_msg(struct char_data *ch, char *arg, struct Board *b)
{
  char                                    msg_number[MAX_INPUT_LENGTH] = "\0\0\0";
  int                                     msg = 0;

  if (DEBUG > 2)
    log_info("called %s with %s, %s, %08zx", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(arg), (size_t)b);

  one_argument(arg, msg_number);
  if (!*msg_number || !isdigit(*msg_number))
    return (0);
  if (!(msg = atoi(msg_number)))
    return (0);
  if (!b->msg_num) {
    cprintf(ch, "The board is empty!\r\n");
    return (1);
  }
  if (msg < 1 || msg > b->msg_num) {
    cprintf(ch, "That message exists only in your imagination..\r\n");
    return (1);
  }
  page_printf(ch, "Message %d : %s\r\n\r\n%s", msg, b->head[msg - 1], b->msgs[msg - 1]);
  return (1);
}

int board_show_board(struct char_data *ch, char *arg, struct Board *b)
{
  int                                     i = 0;
  char                                    tmp[MAX_INPUT_LENGTH] = "\0\0\0";

  if (DEBUG > 2)
    log_info("called %s with %s, %s, %08zx", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(arg), (size_t)b);

  one_argument(arg, tmp);

  if (!*tmp || !isname(tmp, "board bulletin"))
    return (0);

  if (board_kludge_char) {
    cprintf(ch, "Sorry, but someone is writing a message\r\n");
    return (0);
  }

  page_printf(ch, "This is a bulletin board. Usage: READ/REMOVE <messg #>, WRITE <header>\r\n");
  if (!b->msg_num) {
    page_printf(ch, "The board is empty.\r\n");
  } else if (b->msg_num == 1) {
    page_printf(ch, "There is 1 message on the board.\r\n");
  } else {
    page_printf(ch, "There are %d messages on the board.\r\n", b->msg_num);
  }
  for (i = 0; i < b->msg_num; i++)
    page_printf(ch, "%-2d : %s\r\n", i + 1, b->head[i]);

  return (1);
}

void board_save_board(struct Board *bp)
{
    FILE *fp = NULL;
    int board_id = 0;
    int message_id = 0;
    char board_filename[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    int i = 0;

    if (DEBUG > 2)
        log_info("called %s with %08zx", __PRETTY_FUNCTION__, (size_t)bp);

    if (!bp)
        return;

    if (!bp->msg_num) {
        log_info("No messages to save.\r\n");
        return;
    }

    board_id = bp->Vnum;
    snprintf(board_filename, MAX_STRING_LENGTH, "%s/%d.data", BOARD_DIR, board_id);
    if(!(fp = fopen(board_filename, "w"))) {
        log_error("Cannot open %s for writing!", board_filename);
        return;
    }

    fprintf(fp, "%d\n", bp->msg_num);
    for (i = 0; i < bp->msg_num; i++) {
        char message_date[25] = "\0\0\0\0\0\0\0";
        char message_header[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
        char message_sender[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
        char message_text[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
        char *mp = NULL;
        
        message_id = i;

        /* Adventurers WANTED! (Quixadhal) Sun Jul 25 00:35:39 2004 */
        strlcpy(message_date, bp->head[i] + strlen(bp->head[i]) - 24, 25);
        strlcpy(message_header, bp->head[i], MAX_STRING_LENGTH);
        message_header[strlen(message_header) - 26] = '\0';

        mp = strrchr(message_header, '(');
        if (mp) {
            strlcpy(message_sender, mp + 1, MAX_STRING_LENGTH);
            *(mp - 1) = '\0';
        } else {
            *message_sender = '\0';
        }

        if (!bp->msgs[i]) {
            CREATE(bp->msgs[i], char, 50);			       /* Quixadhal: Why 50? */
            strlcpy(bp->msgs[i], "Generic Message", 50);
        }
        strlcpy(message_text, bp->msgs[i], MAX_STRING_LENGTH);

        fprintf(fp, "%d\n%s~\n%s~\n%s~\n%s~\n", message_id, message_date, message_sender, message_header, message_text);
    }
    fclose(fp);
}

void board_load_board(struct Board *bp)
{
    FILE *fp = NULL;
    int board_id = 0;
    int message_id = 0;
    char board_filename[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    int message_count = 0;
    int i = 0;
    int mlen = 0;

    if (DEBUG > 2)
        log_info("called %s with %08zx", __PRETTY_FUNCTION__, (size_t)bp);

    if (!bp)
        return;

    board_id = bp->Vnum;
    snprintf(board_filename, MAX_STRING_LENGTH, "%s/%d.data", BOARD_DIR, board_id);
    if(!(fp = fopen(board_filename, "r"))) {
        log_error("Cannot open %s for reading!", board_filename);
        return;
    }

    message_count = fread_number(fp);
    bp->msg_num = MIN(MAX_MSGS, message_count);
    if(message_count >= MAX_MSGS)
        log_error("BOARD %d IS FULL!", board_id);

    for (i = 0; i < bp->msg_num; i++) {
        char *message_date = NULL;
        char *message_header = NULL;
        char *message_sender = NULL;
        char *message_text = NULL;
        char full_header[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
        
        message_id = fread_number(fp);
        message_date = strdup(new_fread_string(fp));
        message_sender = strdup(new_fread_string(fp));
        message_header = strdup(new_fread_string(fp));
        message_text = strdup(new_fread_string(fp));

        /* Adventurers WANTED! (Quixadhal) Sun Jul 25 00:35:39 2004 */
        strlcpy(full_header, message_header, MAX_STRING_LENGTH);
        strlcat(full_header, " (", MAX_STRING_LENGTH);
        strlcat(full_header, message_sender, MAX_STRING_LENGTH);
        strlcat(full_header, ") ", MAX_STRING_LENGTH);
        strlcat(full_header, message_date, MAX_STRING_LENGTH);

        free(message_date);
        free(message_header);
        free(message_sender);
        mlen = strlen(full_header) + 1;
        CREATE(bp->head[message_id], char, mlen);
        strlcpy(bp->head[message_id], full_header, mlen);
        bp->msgs[message_id] = message_text;
    }
    fclose(fp);
}

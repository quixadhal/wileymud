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
#include "sql.h"
#define _BOARD_C
#include "board.h"

int                       board_count = 0;
struct board_data        *boards = NULL;

void setup_board_table(void) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    char *sql = "CREATE TABLE IF NOT EXISTS boards ( "
                "    updated TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT now(), "
                "    vnum INTEGER PRIMARY KEY NOT NULL "
                "); ";
    char *sql2 = "CREATE TABLE IF NOT EXISTS board_messages ( "
                "    updated TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT now(), "
                "    vnum INTEGER NOT NULL REFERENCES boards (vnum), "
                "    message_id INTEGER NOT NULL, "
                "    owner TEXT NOT NULL DEFAULT 'SYSTEM', "
                "    subject TEXT NOT NULL DEFAULT 'An empty message.', "
                "    body TEXT NOT NULL DEFAULT '', "
                "    CONSTRAINT pk_board_messages UNIQUE ( vnum, message_id ) "
                "); ";
//    char *sql3 = "CREATE UNIQUE INDEX IF NOT EXISTS ix_board_messages "
//                "     ON board_messages (vnum, message_id);";

    sql_connect(&db_wileymud);
    res = PQexec(db_wileymud.dbc, sql);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create boards table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);

    sql_connect(&db_wileymud);
    res = PQexec(db_wileymud.dbc, sql2);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create board_messages table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);

    /*
    sql_connect(&db_wileymud);
    res = PQexec(db_wileymud.dbc, sql3);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create board_messages index: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);
    */
}

void load_boards(void) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    char *sql  = "SELECT count(*) FROM boards;";
    char *sql2 = "SELECT extract('epoch' FROM updated) AS updated, "
                 "       vnum FROM boards ORDER BY vnum;";
    char *sql3 = "SELECT count(*) FROM board_messages WHERE vnum = $1;";
    char *sql4 = "SELECT extract('epoch' FROM updated) AS updated, "
                 "       message_id, owner, subject, body "
                 "FROM board_messages "
                 "WHERE vnum = $1 "
                 "ORDER BY vnum, message_id;";
    int rows = 0;
    int columns = 0;
    const char *param_val[1];
    int param_len[1];
    int param_bin[1] = {0};
    char tmp[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if(boards) {
        // Free any boards that are currently loaded.
        for(int i = 0; i < board_count; i++) {
            if(boards[i].messages) {
                free(boards[i].messages);
            }
        }
        free(boards);
        boards = NULL;
    }
    board_count = 0;

    res = PQexec(db_wileymud.dbc, sql);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot get row count of boards table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    rows = PQntuples(res);
    columns = PQnfields(res);
    if(rows > 0 && columns > 0) {
        board_count = atoi(PQgetvalue(res,0,0));
    } else {
        log_fatal("Invalid result set from row count!");
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);

    if(board_count < 1) {
        log_error("No boards exist!");
        return;
    }

    boards = (struct board_data *) calloc(board_count, sizeof(struct board_data));

    res = PQexec(db_wileymud.dbc, sql2);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot get data from boards table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    rows = PQntuples(res);
    columns = PQnfields(res);

    if(rows != board_count || columns < 2) {
        // We didn't get any data, or got bad data.
        log_fatal("Got invalid data from boards table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }

    for(int i = 0; i < board_count; i++) {
        boards[i].updated = (time_t) atol(PQgetvalue(res,i,0));
        boards[i].vnum = (int) atoi(PQgetvalue(res,i,1));
    }

    PQclear(res);

    for(int i = 0; i < board_count; i++) {
        snprintf(tmp, MAX_INPUT_LENGTH, "%d", boards[i].vnum);
        param_val[0] = *tmp ? tmp : NULL;
        param_len[0] = *tmp ? strlen(tmp) : 0;
        res = PQexecParams(db_wileymud.dbc, sql3, 1, NULL, param_val, param_len, param_bin, 0);
        st = PQresultStatus(res);
        if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
            log_fatal("Cannot fetch message count from board_messages table for vnum %s : %s", tmp, PQerrorMessage(db_wileymud.dbc));
            PQclear(res);
            proper_exit(MUD_HALT);
        }
        rows = PQntuples(res);
        columns = PQnfields(res);

        if(rows > 0 && columns > 0) {
            boards[i].message_count = (int) atoi(PQgetvalue(res,0,0));
            if(boards[i].message_count > 0) {
                boards[i].messages  = (struct board_message_data *) calloc(boards[i].message_count, sizeof(struct board_message_data));
            } else {
                boards[i].messages  = NULL;
            }
        } else {
            log_fatal("Invalid result set from row count!");
            PQclear(res);
            proper_exit(MUD_HALT);
        }
        PQclear(res);

        res = PQexecParams(db_wileymud.dbc, sql4, 1, NULL, param_val, param_len, param_bin, 0);
        st = PQresultStatus(res);
        if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
            log_fatal("Cannot fetch messages from board_messages table for vnum %s : %s", tmp, PQerrorMessage(db_wileymud.dbc));
            PQclear(res);
            proper_exit(MUD_HALT);
        }
        rows = PQntuples(res);
        columns = PQnfields(res);

        if(boards[i].message_count > 0) {
            if(rows == boards[i].message_count && columns > 4) {
                for(int j = 0; j < boards[i].message_count; j++) {
                    boards[i].messages[j].updated = (time_t) atol(PQgetvalue(res,j,0));
                    boards[i].messages[j].vnum = boards[i].vnum;
                    boards[i].messages[j].message_id = (int) atoi(PQgetvalue(res,j,1));
                    strlcpy(boards[i].messages[j].owner, PQgetvalue(res,j,2), MAX_INPUT_LENGTH);
                    strlcpy(boards[i].messages[j].subject, PQgetvalue(res,j,3), MAX_INPUT_LENGTH);
                    strlcpy(boards[i].messages[j].body, PQgetvalue(res,j,4), MAX_STRING_LENGTH);
                }
            } else {
                log_fatal("Bad message data from board_messages table for vnum %s : %s", tmp, PQerrorMessage(db_wileymud.dbc));
                PQclear(res);
                proper_exit(MUD_HALT);
            }
        } else {
            log_error("Empty message board for vnum %d", boards[i].vnum);
        }
        PQclear(res);

        log_boot("Board %d loaded.", boards[i].vnum);
    }
}

struct board_data *find_board_in_room(int room) {
    if(!real_roomp(room))
        return NULL;

    for(struct obj_data *o = real_roomp(room)->contents; o; o = o->next_content) {
        if(obj_index[o->item_number].func == (ifuncp)board_special) {
            for(int i = 0; i < board_count; i++) {
                if(boards[i].vnum == obj_index[o->item_number].virtual) {
                    return &boards[i];
                }
            }
        }
    }

    return NULL;
}

struct board_data *find_board_by_vnum(int vnum) {
    for(int i = 0; i < board_count; i++) {
        if(boards[i].vnum == vnum) {
            return &boards[i];
        }
    }

    return NULL;
}

struct board_data *create_board(int vnum) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    char *sql  = "SELECT 1 FROM boards WHERE vnum = $1;";
    char *sql2 = "INSERT INTO boards (vnum) VALUES ($1);";
    int rows = 0;
    int columns = 0;
    const char *param_val[1];
    int param_len[1];
    int param_bin[1] = {0};
    char tmp[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    struct board_data *bp = NULL;

    bp = find_board_by_vnum(vnum);
    if(bp != NULL) {
        // A board already exists with that vnum, and is loaded!
        return bp;
    }

    snprintf(tmp, MAX_INPUT_LENGTH, "%d", vnum);
    param_val[0] = *tmp ? tmp : NULL;
    param_len[0] = *tmp ? strlen(tmp) : 0;

    res = PQexecParams(db_wileymud.dbc, sql, 1, NULL, param_val, param_len, param_bin, 0);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot check for board %d existing in board table : %s", vnum, PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    rows = PQntuples(res);
    columns = PQnfields(res);
    if(rows > 0 && columns > 0 && atoi(PQgetvalue(res,0,0)) == 1) {
        // This board already exists.
        PQclear(res);
        load_boards();
        bp = find_board_by_vnum(vnum);
        return bp;
    }

    res = PQexecParams(db_wileymud.dbc, sql2, 1, NULL, param_val, param_len, param_bin, 0);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot add board %d to board table : %s", vnum, PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }

    load_boards();
    bp = find_board_by_vnum(vnum);
    if(bp == NULL) {
        log_error("Failed to create board %d", vnum);
        return NULL;
    }
    return bp;
}

void import_board(struct board_data *bp) {
    FILE *fp = NULL;
    char board_filename[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    int message_count = 0;
    PGresult *res = NULL;
    ExecStatusType st = 0;
    char *sql  = "SELECT 1 FROM boards WHERE vnum = $1;";
    char *sql2 = "INSERT INTO board_messages (vnum, message_id, updated, owner, subject, body) "
                 "VALUES ($1,$2,$3 AT TIME ZONE 'US/Eastern',$4,$5,$6) "
                 "ON CONFLICT ON CONSTRAINT pk_board_messages DO NOTHING;";
    int rows = 0;
    int columns = 0;
    const char *param_val[6];
    int param_len[6];
    int param_bin[6] = {0,0,0,0,0,0};
    char tmp[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if(!bp)
        return;

    snprintf(tmp, MAX_INPUT_LENGTH, "%d", bp->vnum);
    param_val[0] = *tmp ? tmp : NULL;
    param_len[0] = *tmp ? strlen(tmp) : 0;

    res = PQexecParams(db_wileymud.dbc, sql, 1, NULL, param_val, param_len, param_bin, 0);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot check for board %d existing in board table : %s", bp->vnum, PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    rows = PQntuples(res);
    columns = PQnfields(res);
    if(rows > 0 && columns > 0 && atoi(PQgetvalue(res,0,0)) == 1) {
        // This board exists.
        PQclear(res);
    } else {
        // The board doesn't exist, yet we had a data structure for it?
        log_fatal("Board %d does not exist in board table.", bp->vnum);
        PQclear(res);
        proper_exit(MUD_HALT);
    }

    snprintf(board_filename, MAX_INPUT_LENGTH, "%s/%d.data", BOARD_DIR, bp->vnum);
    log_boot("Importing board %d from %s...", bp->vnum, board_filename);

    if(!(fp = fopen(board_filename, "r"))) {
        log_error("Cannot open %s for reading!", board_filename);
        return;
    }

    message_count = fread_number(fp);
    log_boot("Importing %d messages into board %d.", message_count, bp->vnum);
    for(int i = 0; i < message_count; i++) {
        int message_id = 0;
        char *message_date = NULL;
        char *message_header = NULL;
        char *message_sender = NULL;
        char *message_text = NULL;
        char tmp_num[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

        // NOTE:  message_id starts at 0 in the original file-based code,
        // but we want to start at 1 here, so the first message is "read 1"
        // and also so we can just use atoi() and see 0 as a failure.
        message_id = fread_number(fp);
        message_id++;

        message_date = strdup(new_fread_string(fp));
        message_sender = strdup(new_fread_string(fp));
        message_header = strdup(new_fread_string(fp));
        message_text = strdup(new_fread_string(fp));

        snprintf(tmp_num, MAX_INPUT_LENGTH, "%d", message_id);
        param_val[1] = *tmp_num ? tmp_num : NULL;
        param_len[1] = *tmp_num ? strlen(tmp_num) : 0;

        // Nice, Postgresql will take the time data in the native format!
        // We will add "AT TIME ZONE 'US/Eastern' as that's where the servers
        // lived for 99% of their lives.
        /* Adventurers WANTED! (Quixadhal) Sun Jul 25 00:35:39 2004 */
        param_val[2] = *message_date ? message_date : NULL;
        param_len[2] = *message_date ? strlen(message_date) : 0;

        param_val[3] = *message_sender ? message_sender : NULL;
        param_len[3] = *message_sender ? strlen(message_sender) : 0;
        param_val[4] = *message_header ? message_header : NULL;
        param_len[4] = *message_header ? strlen(message_header) : 0;
        param_val[5] = *message_text ? message_text : NULL;
        param_len[5] = *message_text ? strlen(message_text) : 0;

        log_boot("Importing message %d into board %d.", message_id, bp->vnum);
        res = PQexecParams(db_wileymud.dbc, sql2, 6, NULL, param_val, param_len, param_bin, 0);
        st = PQresultStatus(res);
        if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
            log_fatal("Cannot insert message %d to board %d : %s", message_id, bp->vnum, PQerrorMessage(db_wileymud.dbc));
            PQclear(res);
            proper_exit(MUD_HALT);
        }

        PQclear(res);
        free(message_date);
        free(message_sender);
        free(message_header);
        free(message_text);
    }
    fclose(fp);
    log_boot("Import complete.");
}

int show_board(struct char_data *ch, char *arg, struct board_data *bp) {
    char tmp[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    one_argument(arg, tmp);

    if(!*tmp || !isname(tmp, "board bulletin"))
        return 0;

    page_printf(ch, "This is a bulletin board.  Usage: READ/REMOVE <msg #>, WRITE <subject>\r\n");

    if(bp->message_count < 1) {
        page_printf(ch, "The board is empty.\r\n");
    } else {
        char today[MAX_INPUT_LENGTH];
        strlcpy(today, date_only(-1), MAX_INPUT_LENGTH);

        if(IS_IMMORTAL(ch)) {
            page_printf(ch, "This is board [#%d], and the date is currently %s.\r\n",
                    bp->vnum, today);
        }
        page_printf(ch, "There %s %d message%s on the board.\r\n",
                bp->message_count > 1 ? "are" : "is",
                bp->message_count,
                bp->message_count > 1 ? "s" : "");
        for(int i = 0; i < bp->message_count; i++) {
            char message_time[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

            strlcpy(message_time, date_only(bp->messages[i].updated), MAX_INPUT_LENGTH);

            if(!strcmp(message_time, today)) {
                // If the date is today, show the time instead.
                strlcpy(message_time, time_only(bp->messages[i].updated), MAX_INPUT_LENGTH);
            }

            // what was header in the old system should now be broken into
            // updated (date/time), owner (character name), and subject.
            page_printf(ch, "%3d : %10.10s %16.16s %s\r\n",
                    bp->messages[i].message_id, message_time, bp->messages[i].owner, 
                    bp->messages[i].subject);
        }
    }

    return 1;
}

int show_board_message(struct char_data *ch, char *arg, struct board_data *bp) {
    char                        message_argument[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    int                         message_number = 0;
    struct board_message_data   *msg = NULL;

    if(!ch || !arg || !*arg || !bp)
        return 0;

    one_argument(arg, message_argument);
    if(!*message_argument || !isdigit(*message_argument))
        return 0;

    message_number = atoi(message_argument);
    if(message_number < 1)
        return 0;

    if(bp->message_count < 1) {
        cprintf(ch, "The board is empty!\r\n");
        return 1;
    }

    for(int i = 0; i < bp->message_count; i++) {
        msg = &bp->messages[i];
        if(msg->message_id == message_number)
            break;
        msg = NULL;
    }

    if(!msg) {
        cprintf(ch, "Message %d exists only in your imagination.\r\n", message_number);
        return 1;
    }

    /* Adventurers WANTED! (Quixadhal) Sun Jul 25 00:35:39 2004 */
    page_printf(ch, "Message %d by %-16.16s on %s : %s\r\n\r\n%s\r\n",
                msg->message_id, msg->owner, timestamp(msg->updated, 0),
                msg->subject, msg->body);
    return 1;
}

int delete_board_message(struct char_data *ch, char *arg, struct board_data *bp) {
    char                        message_argument[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    int                         message_number = 0;
    struct board_message_data   *msg = NULL;
    PGresult *res = NULL;
    ExecStatusType st = 0;
    char *sql  = "DELETE FROM board_messages WHERE vnum = $1 and message_id = $2;";
    //char *sql2 = "UPDATE boards SET message_count = "
    //             " (SELECT count(*) FROM board_messages WHERE vnum = $1) "
    //             " WHERE vnum = $2;";
    const char *param_val[2];
    int param_len[2];
    int param_bin[2] = {0,0};
    char tmp[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char tmp2[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if(!ch || !arg || !*arg || !bp)
        return 0;

    one_argument(arg, message_argument);
    if(!*message_argument || !isdigit(*message_argument))
        return 0;

    message_number = atoi(message_argument);
    if(message_number < 1)
        return 0;

    if(bp->message_count < 1) {
        cprintf(ch, "The board is empty!\r\n");
        return 1;
    }

    for(int i = 0; i < bp->message_count; i++) {
        msg = &bp->messages[i];
        if(msg->message_id == message_number)
            break;
        msg = NULL;
    }

    if(!msg) {
        cprintf(ch, "Message %d exists only in your imagination.\r\n", message_number);
        return 1;
    }

    if(!IS_IMMORTAL(ch) && strcasecmp(msg->owner, GET_NAME(ch)) != 0) {
        cprintf(ch, "That message belongs to %s, NOT you!\r\n", msg->owner);
        return 1;
    }

    // OK, let them remove the message and then reload the board.

    snprintf(tmp, MAX_INPUT_LENGTH, "%d", bp->vnum);
    snprintf(tmp2, MAX_INPUT_LENGTH, "%d", message_number);
    param_val[0] = *tmp ? tmp : NULL;
    param_len[0] = *tmp ? strlen(tmp) : 0;
    param_val[1] = *tmp2 ? tmp2 : NULL;
    param_len[1] = *tmp2 ? strlen(tmp2) : 0;

    res = PQexecParams(db_wileymud.dbc, sql, 2, NULL, param_val, param_len, param_bin, 0);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot remove message %d from board %d : %s", bp->vnum, message_number, PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);

    /*
    param_val[1] = *tmp ? tmp : NULL;
    param_len[1] = *tmp ? strlen(tmp) : 0;

    res = PQexecParams(db_wileymud.dbc, sql2, 2, NULL, param_val, param_len, param_bin, 0);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot update message_count for board %d : %s", bp->vnum, PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);
    */

    cprintf(ch, "Message %d removed.\r\n", message_number);
    load_boards();
    return 1;
}

int begin_write_board_message(struct char_data *ch, char *arg, struct board_data *bp) {
    if(!ch || !ch->desc || !arg || !*arg || !bp)
        return 0;

    for (; isspace(*arg); arg++);   // skip over leading whitespace

    if(!*arg) {
        cprintf(ch, "We must have a headline!\r\n");
        return 1;
    }

    // OK, so now we need the player data structure to have buffers to store
    // the subject line, and relocate a bunch of stuff that's probably in nanny()
    // here, so it's all in one place.
    //
    // The message date will be generated by the database itself.
    // The owner is GET_NAME(ch)
    // The subject is provided by arg.
    // The body will be provided by the nanny() state after the user exits the editor.
    // The message_id will simply be MAX(message_id) + 1 WHERE vnum = this board's vnum.
    //
    // Presumably, this is where the message will be captured.
    //      ch->desc->str = &b->msgs[b->msg_num];
    //      ch->desc->max_str = MAX_MESSAGE_LENGTH;
    //
    // So convoluted....
    //      Rather than using a proper connection state like nanny() does,
    //      There's a custom check on descriptor->str.  If it isn't NULL, 
    //      it calls string_add(), passing it the descriptor and socket input.
    //          line 557 comm.c
    //      The socket input was obtained via get_from_q()
    //          line 541 comm.c
    //      That code however, only runs if get_from_q() returns 1, which happens
    //      when there was some input to process, which happened in process_input()
    //      in the loop above this one.
    //          line 518 comm.c
    //      The question is... what uses descriptor->str BEFORE it gets NULL'd
    //      away in string_add(), which sets it to NULL when it sees the terminator
    //      line (an '@' by itself)?
    //          line 104 modify.c
    //          line 130 modify.c
    //      AHA!
    //      board_save_board() is called from command_interpreter()
    //      if board_kludge_char is set.
    //          line 236 interpreter.c
    //
    //      SOOOO, desc->str is a pointer to the board message array element that
    //      holds the message body.
    //
    //      string_add() uses strcat() to append every line of input to that array
    //      element until the terminaitor is found.
    //
    //      At that point desc->str is set to NULL, and board_save_board() is called
    //      to write the changed array to disk.
    //
    //      The tricky part here is that we don't KNOW what string_add() was modifying
    //      since it's used for anything that uses the built-in editor.  We only have
    //      the point that board_save_board() is called.
    //
    //      What we probably need to do is make a board_message_data structre in the
    //      descriptor strcture, and simply check that.  We can set desc->str to point
    //      at that element or be NULL, to work with the existing code, and the check
    //      for board_kludge_char will simply be changed to check if the message structure
    //      has a non-zero message_id and vnum.

    ch->desc->board_vnum = bp->vnum;
    ch->desc->board_subject = strdup(arg);

    cprintf(ch, "Write your message. Terminate with an @.\r\n\r\n");
    act("$n starts to write a message.", TRUE, ch, 0, 0, TO_ROOM);

    ch->desc->str = &ch->desc->board_body;
    ch->desc->max_str = MAX_STRING_LENGTH;
    return 1;
}

// This is the callback that will actually save the message once the user
// has finished entering it into the line editor.
//
// This is called from interpreter.c:  command_interpreter(), near the top.
//
int finish_write_board_message(struct char_data *ch) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    char *sql  = "INSERT INTO board_messages (vnum, message_id, owner, subject, body) "
                 "VALUES ($1, "
                    "(SELECT COALESCE(MAX(message_id), 0) + 1 "
                    "FROM board_messages "
                    "WHERE vnum = $2), "
                 "$3, $4, $5);";
    const char *param_val[5];
    int param_len[5];
    int param_bin[5] = {0,0,0,0,0};
    char tmp[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    if(!ch || !ch->desc
           || !GET_NAME(ch)
           || ch->desc->board_vnum < 1
           || !ch->desc->board_subject[0]
           || !ch->desc->board_body[0]) {
        log_error("Invalid message data for board message callback!");
        return 0;
    }

    snprintf(tmp, MAX_INPUT_LENGTH, "%d", ch->desc->board_vnum);
    param_val[0] = *tmp ? tmp : NULL;
    param_len[0] = *tmp ? strlen(tmp) : 0;
    param_val[1] = *tmp ? tmp : NULL;
    param_len[1] = *tmp ? strlen(tmp) : 0;
    param_val[2] = GET_NAME(ch);
    param_len[2] = strlen(GET_NAME(ch));
    param_val[3] = ch->desc->board_subject;
    param_len[3] = strlen(ch->desc->board_subject);
    param_val[4] = ch->desc->board_body;
    param_len[4] = strlen(ch->desc->board_body);

    res = PQexecParams(db_wileymud.dbc, sql, 5, NULL, param_val, param_len, param_bin, 0);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot insert message into board_messages for vnum %d : %s", ch->desc->board_vnum, PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);

    ch->desc->board_vnum = 0;
    DESTROY(ch->desc->board_subject);
    DESTROY(ch->desc->board_body);

    cprintf(ch, "Message written to board.\r\n");
    return 1;
}

int board_special(struct char_data *ch, int cmd, char *arg) {
    struct board_data *bp = NULL;

    if(!ch || !ch->desc || !arg || !*arg)
        return FALSE;

    if(!(bp = find_board_in_room(ch->in_room)))
        return FALSE;

    // We return FALSE to indicate that this did NOT satisfy the command requirements
    // which allows the interperter to try other commands, so we only return TRUE
    // for commands the board system overrides.
    switch (cmd) {
        // Mortal commands
        case CMD_autoexit:
        case CMD_bug:
        case CMD_credits:
        case CMD_down:
        case CMD_east:
        case CMD_exits:
        case CMD_gtell:
        case CMD_help:
        case CMD_idea:
        case CMD_info:
        case CMD_json:
        case CMD_north:
        case CMD_nosummon:
        case CMD_noteleport:
        case CMD_pager:
        case CMD_quit:
        case CMD_return:
        case CMD_save:
        case CMD_score:
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
        // Immortal commands
        case CMD_at:
        case CMD_ban:
        case CMD_checkurl:
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
        case CMD_setreboot:
        case CMD_show:
        case CMD_shutdown:
        case CMD_snoop:
        case CMD_switch:
        case CMD_ticks:
        case CMD_transfer:
        case CMD_unban:
        case CMD_whod:
        case CMD_wizhelp:
        case CMD_wizlock:
        case CMD_wiznet:
        case CMD_world:
        case CMD_zpurge:
            return FALSE;
            break;
        case CMD_look:					       /* look */
            if (!show_board(ch, arg, bp)) {
                // The user didn't look at the board, or it failed
                // for some other reason.
                do_look(ch, "", 0);
            }
            return TRUE;
            break;
        case CMD_read:					       /* read */
            if (!show_board_message(ch, arg, bp)) {
                // The attempt to read the board message failed.
            }
            return TRUE;
            break;
        case CMD_remove:					       /* remove */
            if (!delete_board_message(ch, arg, bp)) {
                // The attempt to remove the board message failed.
            }
            return TRUE;
            break;
        case CMD_write:					       /* write */
            if (!begin_write_board_message(ch, arg, bp)) {
                // The attempt to start writing the board message failed.
                // NOTE: The message is actually written by a callback
                // to finish_write_board_message() in the interpreter loop.
            }
            return TRUE;
            break;
        default:
            // Everything else fails with a message.
            cprintf(ch, "This room is too quiet, you can't do that here.\r\n");
            return TRUE;
            break;
    }
}


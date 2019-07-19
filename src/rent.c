#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcre.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <time.h>
#include "global.h"
#include "bug.h"
#include "utils.h"
#include "comm.h"       // for proper_exit() and MUD_HALT

#include "sql.h"
#define _RENT_C
#include "rent.h"

struct rent_data        rent = { 650336715, 1, 1.0, "SYSTEM" }; // beginning of time...

void setup_rent_table(void) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    char *sql = "CREATE TABLE IF NOT EXISTS rent ( "
                "    updated TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT now(), "
                "    enabled BOOLEAN NOT NULL DEFAULT true, "
                "    factor FLOAT NOT NULL DEFAULT 1.0, "
                "    set_by TEXT NOT NULL DEFAULT 'SYSTEM'"
                "); ";
    char *sql2 = "SELECT count(*) FROM rent;";
    char *sql3 = "INSERT INTO rent (enabled) VALUES (true);";
    char *sql4 = "DELETE FROM rent WHERE updated <> (SELECT max(UPDATED) FROM rent);";
    int rows = 0;
    int columns = 0;
    int count = 0;

    sql_connect(&db_wileymud);
    res = PQexec(db_wileymud.dbc, sql);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot create rent table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);

    res = PQexec(db_wileymud.dbc, sql2);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot get row count of rent table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    rows = PQntuples(res);
    columns = PQnfields(res);
    if(rows > 0 && columns > 0) {
        count = atoi(PQgetvalue(res,0,0));
    } else {
        log_fatal("Invalid result set from row count!");
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);

    if(count < 1) {
        // Insert our default value.
        res = PQexec(db_wileymud.dbc, sql3);
        st = PQresultStatus(res);
        if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
            log_fatal("Cannot insert placeholder row to rent table: %s", PQerrorMessage(db_wileymud.dbc));
            PQclear(res);
            proper_exit(MUD_HALT);
        }
        PQclear(res);
    } else if(count > 1) {
        // That shouldn't happen... fix it!
        log_error("There are %d rows in the rent table, instead of just ONE!", count);
        res = PQexec(db_wileymud.dbc, sql4);
        st = PQresultStatus(res);
        if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
            log_fatal("Cannot remove old rows from rent table: %s", PQerrorMessage(db_wileymud.dbc));
            PQclear(res);
            proper_exit(MUD_HALT);
        }
        PQclear(res);
    }
}

void load_rent(void) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    const char *sql = "SELECT extract('epoch' FROM updated) AS updated, "
                      "enabled::integer, "
                      "factor, set_by FROM rent LIMIT 1;";
    int rows = 0;
    int columns = 0;
    struct rent_data the_rent = { 650336715, 1, 1.0, "SYSTEM" }; // beginning of time...

    sql_connect(&db_wileymud);
    res = PQexec(db_wileymud.dbc, sql);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_fatal("Cannot get rent from rent table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        proper_exit(MUD_HALT);
    }

    rows = PQntuples(res);
    columns = PQnfields(res);
    if(rows > 0 && columns > 3) {
        log_boot("  Loading rent data from SQL database.");
        the_rent.updated = (time_t) atol(PQgetvalue(res,0,0));
        the_rent.enabled = (int) atoi(PQgetvalue(res,0,1));
        the_rent.factor = (float) atof(PQgetvalue(res,0,2));
        strlcpy(the_rent.set_by, PQgetvalue(res,0,3), MAX_INPUT_LENGTH);
    } else {
        log_fatal("Invalid result set from rent table!");
        PQclear(res);
        proper_exit(MUD_HALT);
    }
    PQclear(res);

    rent = the_rent;
}

int toggle_rent(struct char_data *ch) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    const char *sql =   "UPDATE rent SET enabled = NOT enabled, "
                        "updated = now(), "
                        "set_by = $1;";
    const char *param_val[1];
    int param_len[1];
    int param_bin[1] = {0};
    char set_by[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    strlcpy(set_by, GET_NAME(ch), MAX_INPUT_LENGTH);
    param_val[0] = (set_by[0]) ? set_by : NULL;
    param_len[0] = (set_by[0]) ? strlen(set_by) : 0;

    sql_connect(&db_wileymud);
    res = PQexecParams(db_wileymud.dbc, sql, 1, NULL, param_val, param_len, param_bin, 0);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_error("Cannot toggle rent in rent table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        return 0;
        //proper_exit(MUD_HALT);
    }
    PQclear(res);

    load_rent();
    return 1;
}

int set_rent(struct char_data *ch, float factor) {
    PGresult *res = NULL;
    ExecStatusType st = 0;
    const char *sql =   "UPDATE rent SET factor = $1, "
                        "updated = now(), "
                        "set_by = $2;";
    const char *param_val[2];
    int param_len[2];
    int param_bin[2] = {0,0};
    char set_by[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char factor_buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";

    snprintf(factor_buf, MAX_INPUT_LENGTH, "%f", factor);
    param_val[0] = (factor_buf[0]) ? factor_buf : NULL;
    param_len[0] = (factor_buf[0]) ? strlen(factor_buf) : 0;
    strlcpy(set_by, GET_NAME(ch), MAX_INPUT_LENGTH);
    param_val[1] = (set_by[0]) ? set_by : NULL;
    param_len[1] = (set_by[0]) ? strlen(set_by) : 0;

    sql_connect(&db_wileymud);
    res = PQexecParams(db_wileymud.dbc, sql, 2, NULL, param_val, param_len, param_bin, 0);
    st = PQresultStatus(res);
    if( st != PGRES_COMMAND_OK && st != PGRES_TUPLES_OK && st != PGRES_SINGLE_TUPLE ) {
        log_error("Cannot set rent in rent table: %s", PQerrorMessage(db_wileymud.dbc));
        PQclear(res);
        return 0;
        //proper_exit(MUD_HALT);
    }
    PQclear(res);

    load_rent();
    return 1;
}


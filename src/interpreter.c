/*
 * file: Interpreter.c , Command interpreter module.      Part of DIKUMUD
 * Usage: Procedures interpreting user command
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <arpa/telnet.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>
//#include <sys/timeb.h>

/*
 * This *SHOULD* have been defined in <unistd.h>... it's in the file
 * under Linux kernel 2.0.35... but somehow it isn't getting defined
 * so this is straight from da man.
 */
extern char *crypt(const char *key, const char *salt);

#include "global.h"
#ifdef I3
#include "i3.h"
#endif
#ifdef IMC
#include "imc.h"
#endif
#include "bug.h"
#include "comm.h"
#include "version.h"
#include "db.h"
#include "utils.h"
#include "mudlimits.h"
#include "act_comm.h"
#include "act_info.h"
#include "act_move.h"
#include "act_obj.h"
#include "act_off.h"
#include "act_other.h"
#include "act_skills.h"
#include "act_social.h"
#include "act_wiz.h"
#include "spells.h"
#include "spell_parser.h"
#include "modify.h"
#include "whod.h"
#include "events.h"
#include "random.h"
#include "board.h"
#include "multiclass.h"
#include "handler.h"
#include "reception.h"
#include "tracking.h"
#include "ban.h"
#include "sql.h"
#include "reboot.h"
#include "stringmap.h"
#include "help.h"
#include "json.h"
#define _INTERPRETER_C
#include "interpreter.h"

struct command_info cmd_info[MAX_CMD_LIST];

const char echo_on[] = {(const char)IAC, (const char)WONT, TELOPT_ECHO, '\0'};

const char echo_off[] = {(const char)IAC, (const char)WILL, TELOPT_ECHO, '\0'};

const char *command[] = {
    "north",     "east",      "south",      "west",     "up",       "down",     "enter",      "exits",       "kill",
    "get",       "drink",     "eat",        "wear",     "wield",    "look",     "score",      "say",         "shout",
    "tell",      "inventory", "qui",        "bounce",   "smile",    "dance",    "kiss",       "cackle",      "laugh",
    "giggle",    "shake",     "puke",       "growl",    "scream",   "insult",   "comfort",    "nod",         "sigh",
    "sulk",      "help",      "who",        "emote",    "echo",     "stand",    "sit",        "rest",        "sleep",
    "wake",      "force",     "transfer",   "hug",      "snuggle",  "cuddle",   "nuzzle",     "cry",         "news",
    "equipment", "buy",       "sell",       "value",    "list",     "drop",     "goto",       "weather",     "read",
    "pour",      "grab",      "remove",     "put",      "shutdow",  "save",     "hit",        "string",      "give",
    "quit",      "stat",      "guard",      "time",     "load",     "purge",    "shutdown",   "idea",        "typo",
    "bug",       "whisper",   "cast",       "at",       "ask",      "order",    "sip",        "taste",       "snoop",
    "follow",    "rent",      "offer",      "poke",     "advance",  "accuse",   "grin",       "bow",         "open",
    "close",     "lock",      "unlock",     "leave",    "applaud",  "blush",    "burp",       "chuckle",     "clap",
    "cough",     "curtsey",   "fart",       "flip",     "fondle",   "frown",    "gasp",       "glare",       "groan",
    "grope",     "hiccup",    "lick",       "love",     "moan",     "nibble",   "pout",       "purr",        "ruffle",
    "shiver",    "shrug",     "sing",       "slap",     "smirk",    "snap",     "sneeze",     "snicker",     "sniff",
    "snore",     "spit",      "squeeze",    "stare",    "strut",    "thank",    "twiddle",    "wave",        "whistle",
    "wiggle",    "wink",      "yawn",       "snowball", "write",    "hold",     "flee",       "sneak",       "hide",
    "backstab",  "pick",      "steal",      "bash",     "rescue",   "kick",     "french",     "comb",        "massage",
    "tickle",    "practice",  "pat",        "examine",  "take",     "info",     "'",          "map",         "curse",
    "use",       "where",     "levels",     "reroll",   "pray",     ",",        "beg",        "bleed",       "cringe",
    "daydream",  "fume",      "grovel",     "hop",      "nudge",    "peer",     "point",      "ponder",      "punch",
    "snarl",     "spank",     "steam",      "tackle",   "taunt",    "think",    "whine",      "worship",     "yodel",
    "brief",     "wizlist",   "consider",   "group",    "restore",  "return",   "switch",     "quaff",       "recite",
    "users",     "pose",      "noshout",    "wizhelp",  "credits",  "compact",  ":",          "hermit",      "slay",
    "wimp",      "junk",      "deposit",    "withdraw", "balance",  "nohassle", "wall",       "pull",        "stealth",
    "doh",       "pset",      "@",          "\"",       "track",    "wizlock",  "highfive",   "title",       "whozone",
    "assist",    "swat",      "world",      "spells",   "breath",   "show",     "debug",      "invisible",   "gain",
    "mkzone",    "disarm",    "bonk",       "wiznet",   "rentmode", "gtell",    "pretitle",   "allcommands", "grep",
    "pager",     "appear",    "logs",       "sethome",  "register", "send",     "whod",       "split",       "notell",
    "scribe",    "apraise",   "ban",        "search",   "skills",   "doorbash", "restoreall", "mount",       "dismount",
    "land",      "nosummon",  "noteleport", "players",  "reset",    "event",    "zpurge",     "ticks",       "bury",
    "desecrate", "setreboot", "home",       "bandage",  "unban",    "immtrack", "ansimap",    "version",     "autoexit",
    "reboot",    "checkurl",  "hint",       "json",     "dice",     "\n"};

const char *fill[] = {"in", "from", "with", "the", "on", "at", "to", "\n"};

int search_block(const char *arg, const char **list, char exact)
{
    int i = 0;
    int l = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %08zx, %d", __PRETTY_FUNCTION__, VNULL(arg), (size_t)list, (int)exact);

    /*
     * Make into lower case, and get length of string
     */
    /*
     * for (l = 0; *(arg + l); l++) *(arg + l) = LOWER(*(arg + l));
     */
    l = strlen(arg);

    if (exact)
    {
        for (i = 0; **(list + i) != '\n'; i++)
            if (!strcasecmp(arg, *(list + i)))
                return i;
    }
    else
    {
        if (!l)
            l = 1; /* Avoid "" to match the first available string */
        for (i = 0; **(list + i) != '\n'; i++)
            if (!strncasecmp(arg, *(list + i), l))
                return i;
    }
    return -1;
}

int old_search_block(const char *argument, int begin, int arglen, const char **list, int mode)
{
    int guess = 0;
    int found = 0;
    int search = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %08zx, %d", __PRETTY_FUNCTION__, VNULL(argument), arglen, (size_t)list, mode);

    /*
     * If the word contain 0 letters, then a match is already found
     */
    found = (arglen < 1);

    guess = 0;

    /*
     * Search for a match
     */

    if (mode)
        while (NOT found AND * (list[guess]) != '\n')
        {
            found = (arglen == strlen(list[guess]));
            for (search = 0; (search < arglen AND found); search++)
                found = (*(argument + begin + search) == *(list[guess] + search));
            guess++;
        }
    else
    {
        while (NOT found AND * (list[guess]) != '\n')
        {
            found = 1;
            for (search = 0; (search < arglen AND found); search++)
                found = (*(argument + begin + search) == *(list[guess] + search));
            guess++;
        }
    }

    return (found ? guess : -1);
}

void command_interpreter(struct char_data *ch, char *argument)
{
    int look_at = 0;
    int cmd = 0;
    int begin = 0;
    const char *hack;

    if (DEBUG > 2)
        log_info("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument));

    /*
     * extern int no_specials;
     * extern struct char_data *board_kludge_char;
     */

    REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);

    if (ch->desc && ch->desc->board_vnum > 0)
    {
        if (finish_write_board_message(ch))
        {
            // If it worked, reload all the boards just to be safe.
            load_boards();
        }
    }

    /*
    if (ch == board_kludge_char) {
    board_save_board(FindBoardInRoom(ch->in_room));
    board_kludge_char = 0;
    }
    */
    /*
     * Find first non blank
     */
    for (begin = 0; *(argument + begin) == ' '; begin++)
        ;

    /*
     * Find length of first word
     * Order swapped by Quixadhal to fix say bug when using "'"
     */
    if ((*(argument + begin) == '\'') || (*(argument + begin) == ':') || (*(argument + begin) == '\"') ||
        (*(argument + begin) == ',') || (*(argument + begin) == '@'))
        look_at = begin + 1;
    else
        for (look_at = 0; *(argument + begin + look_at) > ' '; look_at++)
        {
            /*
             * Make all letters lower case AND find length
             */
            *(argument + begin + look_at) = LOWER(*(argument + begin + look_at));
        }

    hack = argument;
    cmd = old_search_block(argument, begin, look_at, command, 0);

    if (cmd > 0 && GetMaxLevel(ch) < cmd_info[cmd].minimum_level)
    {
        random_error_message(ch);
        return;
    }
    if (cmd > 0 && (cmd_info[cmd].command_pointer != 0))
    {
        if ((!IS_AFFECTED(ch, AFF_PARALYSIS)) || (cmd_info[cmd].minimum_position <= POSITION_STUNNED))
        {
            if (GET_POS(ch) < cmd_info[cmd].minimum_position)
            {
                switch (GET_POS(ch))
                {
                case POSITION_DEAD:
                    cprintf(ch, "Lie still; you are DEAD!!! :-( \r\n");
                    break;
                case POSITION_INCAP:
                case POSITION_MORTALLYW:
                    cprintf(ch, "You are in a pretty bad shape, unable to do anything!\r\n");
                    break;

                case POSITION_STUNNED:
                    cprintf(ch, "All you can do right now, is think about the stars!\r\n");
                    break;
                case POSITION_SLEEPING:
                    cprintf(ch, "In your dreams, or what?\r\n");
                    break;
                case POSITION_RESTING:
                    cprintf(ch, "Nah... You feel too relaxed to do that..\r\n");
                    break;
                case POSITION_SITTING:
                    cprintf(ch, "Maybe you should get on your feet first?\r\n");
                    break;
                case POSITION_FIGHTING:
                    cprintf(ch, "No way! You are fighting for your life!\r\n");
                    break;
                }
            }
            else
            {

                if (!no_specials && special(ch, cmd, argument + begin + look_at))
                    return;

                ((*cmd_info[cmd].command_pointer)(ch, argument + begin + look_at, cmd));

                if ((GetMaxLevel(ch) >= LOW_IMMORTAL) && (GetMaxLevel(ch) < IMPLEMENTOR))
                    log_info("%s:%s", ch->player.name, argument);
            }
            return;
        }
        else
        {
            cprintf(ch, " You are paralyzed, you can't do much of anything!\r\n");
            return;
        }
    }
    if (check_exit_alias(ch, argument))
        return;

    if (cmd > 0 && (cmd_info[cmd].command_pointer == 0))
    {
        cprintf(ch, "Sorry, that command has yet to be implemented...\r\n");
    }

    /* DO ALL NORMAL PARSING ABOVE HERE! */

    if (cmd < 0)
    {
        int got_args = 0;

        if (*(hack + begin + look_at))
        {
            got_args = 1;
            *(argument + begin + look_at) = '\0'; /* BLACK MAGIC */
            log_info("char here is (%d)\r\n", (int)*(hack + begin + look_at));
        }
#ifdef I3
        if (i3_command_hook(ch, (hack + begin), (hack + begin + look_at + got_args)))
            return;
#endif
#ifdef IMC
        if (imc_command_hook(ch, (hack + begin), (hack + begin + look_at + got_args)))
            return;
#endif
        log_error("command(%s), argument(%s), cmd(%d)\r\n", (hack + begin), (hack + begin + look_at + got_args), cmd);
    }
    random_error_message(ch);
}

void new_command_interpreter(struct char_data *ch, char *argument)
{
    const char *s = argument;
    //char token = '\0';
    char arg[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    char *a = arg;
    int cmd = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(argument));

    // If you did something, you stopped hiding.  Someday, this should really be
    // only in-game actions.
    REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);

    if (ch->desc && ch->desc->board_vnum > 0)
    {
        finish_write_board_message(ch);
    }
    /*
    if (ch == board_kludge_char) {
    board_save_board(FindBoardInRoom(ch->in_room));
    board_kludge_char = 0;
    }
    */

    if (!s || !*s)
    {
        log_error("Empty command passed to interpreter?");
        return;
    }

    // Skip over whitespace
    while (*s && isspace(*s))
    {
        s++;
    }

    // Save aside leading tokens, which might be special.
    switch (*s)
    {
    case '\'':
    case ':':
    case '"':
    case ',':
    case '@':
        //token = *s;
        s++;
        break;
    default:
        break;
    }

    // Grab the first word
    while (*s && !isspace(*s))
    {
        *a = LOWER(*s);
        s++;
        a++;
    }
    *a = '\0';

    // Look for a match against a command
    cmd = old_search_block(a, 0, strlen(a), command, 0);

    // If a match, but not allowed to use that command...
    if (cmd > 0 && GetMaxLevel(ch) < cmd_info[cmd].minimum_level)
    {
        random_error_message(ch);
        return;
    }

    // If a match and there is actually a command to be run...
    if (cmd > 0 && (cmd_info[cmd].command_pointer != 0))
    {
        if ((!IS_AFFECTED(ch, AFF_PARALYSIS)) || (cmd_info[cmd].minimum_position <= POSITION_STUNNED))
        {
            if (GET_POS(ch) < cmd_info[cmd].minimum_position)
            {
                switch (GET_POS(ch))
                {
                case POSITION_DEAD:
                    cprintf(ch, "Lie still; you are DEAD!!! :-( \r\n");
                    break;
                case POSITION_INCAP:
                case POSITION_MORTALLYW:
                    cprintf(ch, "You are in a pretty bad shape, unable to do anything!\r\n");
                    break;

                case POSITION_STUNNED:
                    cprintf(ch, "All you can do right now, is think about the stars!\r\n");
                    break;
                case POSITION_SLEEPING:
                    cprintf(ch, "In your dreams, or what?\r\n");
                    break;
                case POSITION_RESTING:
                    cprintf(ch, "Nah... You feel too relaxed to do that..\r\n");
                    break;
                case POSITION_SITTING:
                    cprintf(ch, "Maybe you should get on your feet first?\r\n");
                    break;
                case POSITION_FIGHTING:
                    cprintf(ch, "No way! You are fighting for your life!\r\n");
                    break;
                }
            }
            else
            {
                // Check for special procedures, if enabled.
                if (!no_specials && special(ch, cmd, s))
                    return;

                // Actually do the special we found.
                ((*cmd_info[cmd].command_pointer)(ch, s, cmd));

                if ((GetMaxLevel(ch) >= LOW_IMMORTAL) && (GetMaxLevel(ch) < IMPLEMENTOR))
                    log_info("%s:%s", ch->player.name, argument);
            }
            return;
        }
        else
        {
            cprintf(ch, " You are paralyzed, you can't do much of anything!\r\n");
            return;
        }
    }

    // Exit aliai are handled elsewhere?
    if (check_exit_alias(ch, argument))
        return;

    // A command was found, but is not yet useable.
    if (cmd > 0 && (cmd_info[cmd].command_pointer == 0))
    {
        cprintf(ch, "Sorry, that command has yet to be implemented...\r\n");
        return;
    }

    // No normal commands found, so let's check the intermud!
    if (cmd < 0)
    {
#ifdef I3
        if (i3_command_hook(ch, a, s))
            return;
#endif
#ifdef IMC
        if (imc_command_hook(ch, a, s))
            return;
#endif
        log_error("command(%s), argument(%s), cmd(%d)\r\n", a, s, cmd);
    }

    // We got nothing...
    random_error_message(ch);
}

void argument_interpreter(const char *argument, char *first_arg, char *second_arg)
{
    int look_at = 0;
    int begin = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %s", __PRETTY_FUNCTION__, VNULL(argument), VNULL(second_arg));

    do
    {
        /*
         * Find first non blank
         */
        for (; *(argument + begin) == ' '; begin++)
            ;

        /*
         * Find length of first word
         */
        /*
         * Make all letters lower case, AND copy them to first_arg
         */
        for (look_at = 0; *(argument + begin + look_at) > ' '; look_at++)
            *(first_arg + look_at) = LOWER(*(argument + begin + look_at));

        *(first_arg + look_at) = '\0';
        begin += look_at;

    } while (fill_word(first_arg));

    do
    {
        /*
         * Find first non blank
         */
        for (; *(argument + begin) == ' '; begin++)
            ;

        /*
         * Find length of first word
         */
        /*
         * Make all letters lower case, AND copy them to second_arg
         */
        for (look_at = 0; *(argument + begin + look_at) > ' '; look_at++)
            *(second_arg + look_at) = LOWER(*(argument + begin + look_at));

        *(second_arg + look_at) = '\0';
        begin += look_at;

    } while (fill_word(second_arg));
}

int is_number(const char *str)
{
    int look_at = 0;

    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, VNULL(str));

    if (*str == '\0')
        return FALSE;

    for (look_at = 0; *(str + look_at) != '\0'; look_at++)
        if ((*(str + look_at) < '0') || (*(str + look_at) > '9'))
            return FALSE;
    return TRUE;
}

/*
 * Quinn substituted a new one-arg for the old one.. I thought returning a
 * char pointer would be neat, and avoiding the func-calls would save a
 * little time... If anyone feels pissed, I'm sorry.. Anyhow, the code is
 * snatched from the old one, so it outta work..
 *
 * void one_argument(char *argument,char *first_arg )
 * {
 * static char dummy[MAX_STRING_LENGTH];
 *
 * argument_interpreter(argument,first_arg,dummy);
 * }
 *
 */

/*
 * find the first sub-argument of a string, return pointer to first char in
 * primary argument, following the sub-arg
 */
const char *one_argument(const char *argument, char *first_arg)
{
    int begin = 0;
    int look_at = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %s", __PRETTY_FUNCTION__, VNULL(argument), VNULL(first_arg));

    do
    {
        for (; isspace(*(argument + begin)); begin++)
            ;

        for (look_at = 0; *(argument + begin + look_at) > ' '; look_at++)
            *(first_arg + look_at) = LOWER(*(argument + begin + look_at));

        *(first_arg + look_at) = '\0';
        begin += look_at;
    } while (fill_word(first_arg));

    return (argument + begin);
}

void only_argument(const char *argument, char *dest)
{
    if (DEBUG > 2)
        log_info("called %s with %s, %08zx", __PRETTY_FUNCTION__, VNULL(argument), (size_t)dest);

    while (*argument && isspace(*argument))
        argument++;
    strcpy(dest, argument);
}

int fill_word(char *argument)
{
    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, VNULL(argument));

    return (search_block(argument, fill, TRUE) >= 0);
}

/*
 * determine if a given string is an abbreviation of another
 *
 * If the either source or target is NULL, it would crash so return 0.
 * If the source is longer than the target, it fails since
 * "lightning" is NOT an abbreviation of "light".
 * This gets caught automagically though, since when *arg1 == '\0' at the
 * end of the string, *arg2 will be some other character.
 * According to the way this works, "" must be a valid abbreviation of
 * everything... seems odd, but...
 */
int is_abbrev(const char *arg1, const char *arg2)
{
    if (DEBUG > 2)
        log_info("called %s with %s, %s", __PRETTY_FUNCTION__, VNULL(arg1), VNULL(arg2));

    if (!*arg1 || !*arg2)
        return FALSE;
    for (; *arg1; arg1++, arg2++)
        if (LOWER(*arg1) != LOWER(*arg2))
            return FALSE;
    return TRUE;
}

/*
 * return first 'word' plus trailing substring of input string
 */
void half_chop(const char *str, char *arg1, char *arg2)
{
    if (DEBUG > 2)
        log_info("called %s with %s, %08zx, %08zx", __PRETTY_FUNCTION__, VNULL(str), (size_t)arg1, (size_t)arg2);

    for (; isspace(*str); str++)
        ;
    for (; !isspace(*arg1 = *str) && *str; str++, arg1++)
        ;
    *arg1 = '\0';
    for (; isspace(*str); str++)
        ;
    for (; (*arg2 = *str); str++, arg2++)
        ;
}

int special(struct char_data *ch, int cmd, const char *arg)
{
    struct obj_data *i = NULL;
    struct char_data *k = NULL;
    int j = 0;
    int test = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %d, %s", __PRETTY_FUNCTION__, SAFE_NAME(ch), cmd, VNULL(arg));

    if (ch->in_room == NOWHERE)
    {
        char_to_room(ch, DEFAULT_HOME);
        return FALSE;
    }
    /*
     * special in room?
     */
    if (real_roomp(ch->in_room)->funct)
        if ((*real_roomp(ch->in_room)->funct)(ch, cmd, arg))
            return TRUE;

    /*
     * special in equipment list?
     */
    for (j = 0; j <= (MAX_WEAR - 1); j++)
        if (ch->equipment[j] && ch->equipment[j]->item_number >= 0)
            if (obj_index[ch->equipment[j]->item_number].func)
                if ((*obj_index[ch->equipment[j]->item_number].func)(ch, cmd, arg))
                    return TRUE;

    test++;
    /*
     * special in inventory?
     */
    for (i = ch->carrying; i; i = i->next_content)
        if (i->item_number >= 0)
            if (obj_index[i->item_number].func)
                if ((*obj_index[i->item_number].func)(ch, cmd, arg))
                    return TRUE;

    test++;

    /*
     * special in mobile present?
     */
    for (k = real_roomp(ch->in_room)->people; k; k = k->next_in_room)
        if (IS_MOB(k))
            if (mob_index[k->nr].func)
                if ((*mob_index[k->nr].func)(ch, cmd, arg))
                    return TRUE;

    test++;

    /*
     * special in object present?
     */
    for (i = real_roomp(ch->in_room)->contents; i; i = i->next_content)
        if (i->item_number >= 0)
            if (obj_index[i->item_number].func)
                if ((*obj_index[i->item_number].func)(ch, cmd, arg))
                    return TRUE;

    test++;

    return FALSE;
}

void assign_command_pointers(void)
{
    int position = 0;

    if (DEBUG > 2)
        log_info("called %s with no arguments", __PRETTY_FUNCTION__);

    for (position = 0; position < MAX_CMD_LIST; position++)
        cmd_info[position].command_pointer = 0;

    /* whizz commands */
    COMMANDO(CMD_WIZNET, POSITION_DEAD, do_commune, 51);
    COMMANDO(CMD_advance, POSITION_DEAD, do_advance, 58);
    COMMANDO(CMD_autoexit, POSITION_DEAD, do_autoexit, 51);
    COMMANDO(CMD_ban, POSITION_DEAD, do_ban, 54);
    COMMANDO(CMD_debug, POSITION_DEAD, do_debug, 59);
    COMMANDO(CMD_event, POSITION_DEAD, do_event, 55);
    COMMANDO(CMD_force, POSITION_SLEEPING, do_force, 55);
    COMMANDO(CMD_goto, POSITION_SLEEPING, do_goto, 51);
    COMMANDO(CMD_highfive, POSITION_DEAD, do_highfive, 51);
    COMMANDO(CMD_home, POSITION_DEAD, do_home, 51);
    COMMANDO(CMD_invisible, POSITION_DEAD, do_invis, 52);
    COMMANDO(CMD_load, POSITION_DEAD, do_load, 57);
    COMMANDO(CMD_logs, POSITION_DEAD, do_show_logs, 57);
    COMMANDO(CMD_mkzone, POSITION_DEAD, do_not_yet_implemented, 59);
    COMMANDO(CMD_nohassle, POSITION_DEAD, do_nohassle, 54);
    COMMANDO(CMD_players, POSITION_DEAD, do_players, 59);
    COMMANDO(CMD_pretitle, POSITION_DEAD, do_pretitle, 51);
    COMMANDO(CMD_pset, POSITION_DEAD, do_set, 58);
    COMMANDO(CMD_purge, POSITION_DEAD, do_purge, 52);
    COMMANDO(CMD_reboot, POSITION_DEAD, do_reboot, 58);
    COMMANDO(CMD_rentmode, POSITION_DEAD, do_rentmode, 57);
    COMMANDO(CMD_reroll, POSITION_DEAD, do_reroll, 58);
    COMMANDO(CMD_reset, POSITION_DEAD, do_reset, 57);
    COMMANDO(CMD_restore, POSITION_DEAD, do_restore, 58);
    COMMANDO(CMD_restoreall, POSITION_DEAD, do_restore_all, 52);
    COMMANDO(CMD_setreboot, POSITION_DEAD, do_setreboot, 57);
    COMMANDO(CMD_show, POSITION_DEAD, do_show, 54);
    COMMANDO(CMD_shutdow, POSITION_DEAD, do_shutdow, 58);
    COMMANDO(CMD_shutdown, POSITION_DEAD, do_shutdown, 58);
    COMMANDO(CMD_slay, POSITION_STANDING, do_kill, 57);
    COMMANDO(CMD_snoop, POSITION_DEAD, do_snoop, 59);
    COMMANDO(CMD_snowball, POSITION_STANDING, do_action, 51);
    COMMANDO(CMD_spells, POSITION_DEAD, do_spells, 53);
    COMMANDO(CMD_stat, POSITION_DEAD, do_stat, 54);
    COMMANDO(CMD_stealth, POSITION_DEAD, do_stealth, 58);
    COMMANDO(CMD_string, POSITION_SLEEPING, do_string, 56);
    COMMANDO(CMD_switch, POSITION_DEAD, do_switch, 55);
    COMMANDO(CMD_ticks, POSITION_DEAD, do_ticks, 51);
    COMMANDO(CMD_title, POSITION_DEAD, do_title, 30);
    COMMANDO(CMD_transfer, POSITION_SLEEPING, do_trans, 52);
    COMMANDO(CMD_unban, POSITION_DEAD, do_unban, 54);
    COMMANDO(CMD_checkurl, POSITION_DEAD, do_checkurl, 55);
    COMMANDO(CMD_users, POSITION_DEAD, do_users, 51);
    COMMANDO(CMD_wall, POSITION_DEAD, do_system, 55);
    COMMANDO(CMD_whod, POSITION_DEAD, do_whod, 59);
    COMMANDO(CMD_wizhelp, POSITION_SLEEPING, do_wizhelp, 51);
    COMMANDO(CMD_wizlock, POSITION_DEAD, do_wizlock, 53);
    COMMANDO(CMD_wiznet, POSITION_DEAD, do_commune, 51);
    COMMANDO(CMD_world, POSITION_DEAD, do_world, 51);
    COMMANDO(CMD_zpurge, POSITION_DEAD, do_zone_purge, 58);
    COMMANDO(CMD_immtrack, POSITION_DEAD, do_immtrack, 51);
    COMMANDO(CMD_ansimap, POSITION_RESTING, do_ansimap, 51);

    /* mortal commands */
    COMMANDO(CMD_EMOTE, POSITION_SLEEPING, do_emote, 0);
    COMMANDO(CMD_GTELL, POSITION_RESTING, do_group_tell, 0);
    COMMANDO(CMD_SAY, POSITION_RESTING, do_say, 0);
    COMMANDO(CMD_TELL, POSITION_RESTING, do_tell, 0);
    COMMANDO(CMD_accuse, POSITION_SITTING, do_action, 0);
    COMMANDO(CMD_allcommands, POSITION_DEAD, do_allcommands, 1);
    COMMANDO(CMD_appear, POSITION_DEAD, do_invis_off, 1);
    COMMANDO(CMD_applaud, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_apraise, POSITION_RESTING, do_apraise, 1);
    COMMANDO(CMD_ask, POSITION_RESTING, do_ask, 0);
    COMMANDO(CMD_assist, POSITION_FIGHTING, do_assist, 0);
    COMMANDO(CMD_at, POSITION_DEAD, do_at, 53);
    COMMANDO(CMD_backstab, POSITION_STANDING, do_backstab, 1);
    COMMANDO(CMD_balance, POSITION_RESTING, do_not_here, 1);
    COMMANDO(CMD_bandage, POSITION_FIGHTING, do_bandage, 1);
    COMMANDO(CMD_bash, POSITION_FIGHTING, do_bash, 1);
    COMMANDO(CMD_beg, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_bleed, POSITION_RESTING, do_not_here, 0);
    COMMANDO(CMD_blush, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_bonk, POSITION_SITTING, do_action, 1);
    COMMANDO(CMD_bounce, POSITION_STANDING, do_action, 0);
    COMMANDO(CMD_bow, POSITION_STANDING, do_action, 0);
    COMMANDO(CMD_breath, POSITION_FIGHTING, do_breath, 0);
    COMMANDO(CMD_brief, POSITION_DEAD, do_brief, 0);
    COMMANDO(CMD_bug, POSITION_DEAD, do_bug, 0);
    COMMANDO(CMD_burp, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_bury, POSITION_STANDING, do_bury, 0);
    COMMANDO(CMD_buy, POSITION_STANDING, do_not_here, 0);
    COMMANDO(CMD_cackle, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_cast, POSITION_SITTING, do_cast, 1);
    COMMANDO(CMD_chuckle, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_clap, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_close, POSITION_SITTING, do_close, 0);
    COMMANDO(CMD_comb, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_comfort, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_compact, POSITION_DEAD, do_compact, 0);
    COMMANDO(CMD_consider, POSITION_RESTING, do_consider, 0);
    COMMANDO(CMD_cough, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_credits, POSITION_DEAD, do_credits, 0);
    COMMANDO(CMD_cringe, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_cry, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_cuddle, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_curse, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_curtsey, POSITION_STANDING, do_action, 0);
    COMMANDO(CMD_dance, POSITION_STANDING, do_action, 0);
    COMMANDO(CMD_daydream, POSITION_SLEEPING, do_action, 0);
    COMMANDO(CMD_deposit, POSITION_RESTING, do_not_here, 1);
    COMMANDO(CMD_desecrate, POSITION_STANDING, do_desecrate, 0);
    COMMANDO(CMD_dice, POSITION_DEAD, do_dice, 0);
    COMMANDO(CMD_disarm, POSITION_FIGHTING, do_disarm, 1);
    COMMANDO(CMD_dismount, POSITION_MOUNTED, do_mount, 1);
    COMMANDO(CMD_doh, POSITION_DEAD, do_action, 0);
    COMMANDO(CMD_doorbash, POSITION_STANDING, do_doorbash, 1);
    COMMANDO(CMD_down, POSITION_STANDING, do_move, 0);
    COMMANDO(CMD_drink, POSITION_RESTING, do_drink, 0);
    COMMANDO(CMD_drop, POSITION_RESTING, do_drop, 0);
    COMMANDO(CMD_east, POSITION_STANDING, do_move, 0);
    COMMANDO(CMD_eat, POSITION_RESTING, do_eat, 0);
    COMMANDO(CMD_echo, POSITION_SLEEPING, do_echo, 1);
    COMMANDO(CMD_emote, POSITION_SLEEPING, do_emote, 0);
    COMMANDO(CMD_enter, POSITION_STANDING, do_enter, 0);
    COMMANDO(CMD_equipment, POSITION_SLEEPING, do_equipment, 0);
    COMMANDO(CMD_examine, POSITION_SITTING, do_examine, 0);
    COMMANDO(CMD_exits, POSITION_RESTING, do_exits, 0);
    COMMANDO(CMD_fart, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_flee, POSITION_FIGHTING, do_flee, 1);
    COMMANDO(CMD_flip, POSITION_STANDING, do_action, 0);
    COMMANDO(CMD_follow, POSITION_RESTING, do_follow, 0);
    COMMANDO(CMD_fondle, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_french, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_frown, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_fume, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_gain, POSITION_DEAD, do_gain, 1);
    COMMANDO(CMD_gasp, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_get, POSITION_RESTING, do_get, 0);
    COMMANDO(CMD_giggle, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_give, POSITION_RESTING, do_give, 0);
    COMMANDO(CMD_glare, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_grab, POSITION_RESTING, do_grab, 0);
    COMMANDO(CMD_grep, POSITION_DEAD, do_group_report, 1);
    COMMANDO(CMD_grin, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_groan, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_grope, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_group, POSITION_RESTING, do_group, 1);
    COMMANDO(CMD_grovel, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_growl, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_gtell, POSITION_RESTING, do_group_tell, 0);
    COMMANDO(CMD_guard, POSITION_STANDING, do_guard, 1);
    COMMANDO(CMD_help, POSITION_DEAD, do_help, 0);
    COMMANDO(CMD_hermit, POSITION_SLEEPING, do_plr_noshout, 1);
    COMMANDO(CMD_hiccup, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_hide, POSITION_RESTING, do_hide, 1);
    COMMANDO(CMD_hint, POSITION_SLEEPING, do_info, 0);
    COMMANDO(CMD_hit, POSITION_FIGHTING, do_hit, 0);
    COMMANDO(CMD_hold, POSITION_RESTING, do_grab, 1);
    COMMANDO(CMD_hop, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_hug, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_idea, POSITION_DEAD, do_idea, 0);
    COMMANDO(CMD_info, POSITION_SLEEPING, do_info, 0);
    COMMANDO(CMD_insult, POSITION_RESTING, do_insult, 0);
    COMMANDO(CMD_inventory, POSITION_DEAD, do_inventory, 0);
    COMMANDO(CMD_json, POSITION_DEAD, do_json, 0);
    COMMANDO(CMD_junk, POSITION_RESTING, do_junk, 1);
    COMMANDO(CMD_kick, POSITION_FIGHTING, do_kick, 1);
    COMMANDO(CMD_kill, POSITION_FIGHTING, do_kill, 0);
    COMMANDO(CMD_kiss, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_land, POSITION_DEAD, do_land, 1);
    COMMANDO(CMD_laugh, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_leave, POSITION_STANDING, do_leave, 0);
    COMMANDO(CMD_levels, POSITION_DEAD, do_levels, 0);
    COMMANDO(CMD_lick, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_list, POSITION_STANDING, do_not_here, 0);
    COMMANDO(CMD_lock, POSITION_SITTING, do_lock, 0);
    COMMANDO(CMD_look, POSITION_RESTING, do_look, 0);
    COMMANDO(CMD_love, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_massage, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_moan, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_mount, POSITION_STANDING, do_mount, 1);
    COMMANDO(CMD_news, POSITION_SLEEPING, do_news, 0);
    COMMANDO(CMD_nibble, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_nod, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_north, POSITION_STANDING, do_move, 0);
    COMMANDO(CMD_noshout, POSITION_SLEEPING, do_noshout, 1);
    COMMANDO(CMD_nosummon, POSITION_SLEEPING, do_plr_nosummon, 1);
    COMMANDO(CMD_noteleport, POSITION_SLEEPING, do_plr_noteleport, 1);
    COMMANDO(CMD_notell, POSITION_SLEEPING, do_plr_notell, 1);
    COMMANDO(CMD_nudge, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_nuzzle, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_offer, POSITION_STANDING, do_not_here, 1);
    COMMANDO(CMD_open, POSITION_SITTING, do_open, 0);
    COMMANDO(CMD_order, POSITION_RESTING, do_order, 1);
    COMMANDO(CMD_pager, POSITION_DEAD, do_pager, 1);
    COMMANDO(CMD_pat, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_peer, POSITION_STANDING, do_peer, 1);
    COMMANDO(CMD_pick, POSITION_STANDING, do_pick, 1);
    COMMANDO(CMD_point, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_poke, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_ponder, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_pose, POSITION_STANDING, do_pose, 0);
    COMMANDO(CMD_pour, POSITION_STANDING, do_pour, 0);
    COMMANDO(CMD_pout, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_practice, POSITION_RESTING, do_practice, 1);
    COMMANDO(CMD_map, POSITION_RESTING, do_map, 1);
    COMMANDO(CMD_pray, POSITION_SITTING, do_action, 0);
    COMMANDO(CMD_puke, POSITION_RESTING, do_puke, 0);
    COMMANDO(CMD_pull, POSITION_STANDING, do_not_here, 1);
    COMMANDO(CMD_punch, POSITION_FIGHTING, do_punch, 1);
    COMMANDO(CMD_purr, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_put, POSITION_RESTING, do_put, 0);
    COMMANDO(CMD_quaff, POSITION_RESTING, do_quaff, 0);
    COMMANDO(CMD_qui, POSITION_DEAD, do_qui, 0);
    COMMANDO(CMD_quit, POSITION_DEAD, do_quit, 0);
    COMMANDO(CMD_read, POSITION_RESTING, do_read, 0);
    COMMANDO(CMD_recite, POSITION_RESTING, do_recite, 0);
    COMMANDO(CMD_register, POSITION_STANDING, do_not_here, 3);
    COMMANDO(CMD_remove, POSITION_RESTING, do_remove, 0);
    COMMANDO(CMD_rent, POSITION_STANDING, do_not_here, 1);
    COMMANDO(CMD_rescue, POSITION_FIGHTING, do_rescue, 1);
    COMMANDO(CMD_rest, POSITION_RESTING, do_rest, 0);
    COMMANDO(CMD_return, POSITION_DEAD, do_return, 0);
    COMMANDO(CMD_ruffle, POSITION_STANDING, do_action, 0);
    COMMANDO(CMD_save, POSITION_SLEEPING, do_save, 0);
    COMMANDO(CMD_say, POSITION_RESTING, do_say, 0);
    COMMANDO(CMD_score, POSITION_DEAD, do_score, 0);
    COMMANDO(CMD_scream, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_scribe, POSITION_DEAD, do_not_yet_implemented, 1);
    COMMANDO(CMD_search, POSITION_STANDING, do_search, 1);
    COMMANDO(CMD_sell, POSITION_STANDING, do_not_here, 0);
    COMMANDO(CMD_send, POSITION_STANDING, do_not_here, 3);
    COMMANDO(CMD_sethome, POSITION_STANDING, do_not_here, 1);
    COMMANDO(CMD_shake, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_shiver, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_shout, POSITION_RESTING, do_shout, 0);
    COMMANDO(CMD_shrug, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_sigh, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_sing, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_sip, POSITION_RESTING, do_sip, 0);
    COMMANDO(CMD_sit, POSITION_RESTING, do_sit, 0);
    COMMANDO(CMD_skills, POSITION_STANDING, do_skills, 1);
    COMMANDO(CMD_slap, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_sleep, POSITION_SLEEPING, do_sleep, 0);
    COMMANDO(CMD_smile, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_smirk, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_snap, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_snarl, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_sneak, POSITION_STANDING, do_sneak, 1);
    COMMANDO(CMD_sneeze, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_snicker, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_sniff, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_snore, POSITION_SLEEPING, do_action, 0);
    COMMANDO(CMD_snuggle, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_south, POSITION_STANDING, do_move, 0);
    COMMANDO(CMD_spank, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_spit, POSITION_STANDING, do_action, 0);
    COMMANDO(CMD_split, POSITION_RESTING, do_split, 1);
    COMMANDO(CMD_squeeze, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_stand, POSITION_RESTING, do_stand, 0);
    COMMANDO(CMD_stare, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_steal, POSITION_STANDING, do_steal, 1);
    COMMANDO(CMD_steam, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_strut, POSITION_STANDING, do_action, 0);
    COMMANDO(CMD_sulk, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_swat, POSITION_DEAD, do_swat, 1);
    COMMANDO(CMD_tackle, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_take, POSITION_RESTING, do_get, 0);
    COMMANDO(CMD_taste, POSITION_RESTING, do_taste, 0);
    COMMANDO(CMD_taunt, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_tell, POSITION_RESTING, do_tell, 0);
    COMMANDO(CMD_thank, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_think, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_tickle, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_time, POSITION_DEAD, do_time, 0);
    COMMANDO(CMD_track, POSITION_DEAD, do_track, 1);
    COMMANDO(CMD_twiddle, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_typo, POSITION_DEAD, do_typo, 0);
    COMMANDO(CMD_unlock, POSITION_SITTING, do_unlock, 0);
    COMMANDO(CMD_up, POSITION_STANDING, do_move, 0);
    COMMANDO(CMD_use, POSITION_SITTING, do_use, 1);
    COMMANDO(CMD_value, POSITION_STANDING, do_not_here, 0);
    COMMANDO(CMD_version, POSITION_DEAD, do_version, 0);
    COMMANDO(CMD_wake, POSITION_SLEEPING, do_wake, 0);
    COMMANDO(CMD_wave, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_wear, POSITION_RESTING, do_wear, 0);
    COMMANDO(CMD_weather, POSITION_RESTING, do_weather, 0);
    COMMANDO(CMD_west, POSITION_STANDING, do_move, 0);
    COMMANDO(CMD_where, POSITION_DEAD, do_where, 1);
    COMMANDO(CMD_whine, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_whisper, POSITION_RESTING, do_whisper, 0);
    COMMANDO(CMD_whistle, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_who, POSITION_DEAD, do_who, 0);
    COMMANDO(CMD_whozone, POSITION_DEAD, do_who, 0);
    COMMANDO(CMD_wield, POSITION_RESTING, do_wield, 0);
    COMMANDO(CMD_wiggle, POSITION_STANDING, do_action, 0);
    COMMANDO(CMD_wimp, POSITION_DEAD, do_wimp, 1);
    COMMANDO(CMD_wink, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_withdraw, POSITION_RESTING, do_not_here, 1);
    COMMANDO(CMD_wizlist, POSITION_DEAD, do_wizlist, 0);
    COMMANDO(CMD_worship, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_write, POSITION_STANDING, do_write, 1);
    COMMANDO(CMD_yawn, POSITION_RESTING, do_action, 0);
    COMMANDO(CMD_yodel, POSITION_RESTING, do_action, 0);
}

/*
 * Stuff for controlling the non-playing sockets (get name, pwd etc)
 */

/*
 * locate entry in p_table with entry->name == name. -1 mrks failed search
 */
int find_name(char *name)
{
    int i = 0;

    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, VNULL(name));

    for (i = 0; i <= top_of_p_table; i++)
    {
        if (!str_cmp((player_table + i)->name, name))
            return i;
    }
    return -1;
}

int _parse_name(char *arg, char *name)
{
    int i = 0;

    if (DEBUG > 2)
        log_info("called %s with %s, %08zx", __PRETTY_FUNCTION__, VNULL(arg), (size_t)name);

    for (; isspace(*arg); arg++)
        ;
    for (i = 0; (*name = *arg); arg++, i++, name++)
        if ((*arg < 0) || !isalpha(*arg) || i > 15)
            return TRUE;

    if (!i)
        return TRUE;
    return FALSE;
}

/*
 * An improved version of _parse_name()
 */
int valid_parse_name(const char *arg, char *name)
{
    int i = 0;
    const char *hard[] = {"god", "demigod", "myself", "me", NULL};

    if (DEBUG > 2)
        log_info("called %s with %s, %08zx", __PRETTY_FUNCTION__, VNULL(arg), (size_t)name);

    if (!arg || !*arg)
        return FALSE;
    if (strlen(arg) < 3)
        return FALSE;
    for (i = 0; hard[i]; i++)
        if (!strcasecmp(hard[i], arg))
            return FALSE;
    for (i = 0; (*name = *arg); i++, arg++, name++)
        if (!*arg || !isalpha(*arg) || i > 15)
            return FALSE;
    return TRUE;
}

/*
 * Make sure they are not trying to take a mob name...
 */
int already_mob_name(char *ack_name)
{
    int ack = 0;
    int blah = 0;
    char pfft_name[80] = "\0\0\0\0\0\0\0";
    char *pfft = NULL;

    if (DEBUG > 2)
        log_info("called %s with %s", __PRETTY_FUNCTION__, VNULL(ack_name));

    for (blah = FALSE, ack = 0; ack < top_of_mobt && !blah; ack++)
    {
        strcpy(pfft_name, mob_index[ack].name);
        if (!(pfft = (char *)strtok(pfft_name, " ")))
            continue;
        if (!strcasecmp(pfft, ack_name))
        {
            blah = TRUE;
            break;
        }
        while ((pfft = (char *)strtok(NULL, " ")))
        {
            if (!strcasecmp(pfft, ack_name))
            {
                blah = TRUE;
                break;
            }
        }
    }
    return blah;
}

/*
 * See if the player has lost his link
 */
int check_reconnect(struct descriptor_data *d)
{
    struct char_data *tmp_ch = NULL;

    if (DEBUG > 2)
        log_info("called %s with %08zx", __PRETTY_FUNCTION__, (size_t)d);

    for (tmp_ch = character_list; tmp_ch; tmp_ch = tmp_ch->next)
    {
        if (
            /* Doh!
             * If it is not a player, it will not have the player.name structure and
             * will SEGV before it checks IS_NPC().... please remember left-to-right.
             *             ((!str_cmp(d->usr_name), GET_NAME(tmp_ch)) &&
             *              !tmp_ch->desc && !IS_NPC(tmp_ch)) ||
             */
            (!IS_NPC(tmp_ch) && !tmp_ch->desc && (!str_cmp(d->usr_name, GET_NAME(tmp_ch)))) ||
            (IS_NPC(tmp_ch) && tmp_ch->orig && !str_cmp(d->usr_name, GET_NAME(tmp_ch->orig))))
        {
            free_char(d->character);
            d->character = NULL; /* need to wipe this out so we don't pick at it! */
            tmp_ch->desc = d;
            d->character = tmp_ch;
            tmp_ch->specials.timer = 0;
            if (tmp_ch->orig)
            {
                tmp_ch->desc->original = tmp_ch->orig;
                tmp_ch->orig = 0;
            }
            STATE(d) = CON_PLAYING;
            dcprintf(d, "\r\n%sReconnecting to %s.\r\n", echo_on, GET_NAME(d->character));
            act("$n has reconnected.", TRUE, tmp_ch, 0, 0, TO_ROOM);
            if (d->character->in_room == NOWHERE)
                char_to_room(d->character, DEFAULT_HOME);
            else
            {
                if (d->character->in_room == 0)
                {
                    char_from_room(d->character);
                    char_to_room(d->character, DEFAULT_HOME);
                }
            }
            log_info("%s[%s] has reconnected.", GET_NAME(d->character), d->host);
            return TRUE;
        }
    }
    return FALSE;
}

int check_playing(struct descriptor_data *d, char *tmp_name)
{
    struct descriptor_data *k = NULL;

    if (DEBUG > 2)
        log_info("called %s with %08zx, %s", __PRETTY_FUNCTION__, (size_t)d, VNULL(tmp_name));

    for (k = descriptor_list; k; k = k->next)
    {
        if (k->character == NULL)
            continue;
        if (d && (k->character == d->character))
            continue;
        /*
         * if ((k->character != d->character) && k->character) {
         */
        if (k->original)
        {
            if (GET_NAME(k->original) && !str_cmp(GET_NAME(k->original), tmp_name))
            {
                return TRUE;
            }
        }
        else
        {
            if (GET_NAME(k->character) && !str_cmp(GET_NAME(k->character), tmp_name))
            {
                return TRUE;
            }
        }
        /*
         * }
         */
    }
    return FALSE;
}

/*
 * deal with newcomers and other non-playing sockets
 */
void nanny(struct descriptor_data *d, char *arg)
{
    int count = 0;
    int oops = FALSE;
    char tmp_name[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    struct char_file_u tmp_store;
    int i = 0;
    char cryptbuf[17] = "\0\0\0\0\0\0\0";
    char cryptsalt[3] = {'\0', '\0', '\0'};
    char host_name[MAX_STRING_LENGTH] = "\0\0\0\0\0\0\0";
    char ip_addr[20] = "\0\0\0\0\0\0\0";
    static int bad_names = 0;
    static time_t last_bad_name = 0;

    if (DEBUG > 2)
        log_info("called %s with %08zx, %s", __PRETTY_FUNCTION__, (size_t)d, VNULL(arg));

    while (isspace(*arg))
        arg++;
    write(d->descriptor, echo_on, strlen(echo_on));

    if (d && d->ip[0])
        strcpy(ip_addr, d->ip);
    else
        strcpy(ip_addr, "unknown address");

    if (d && d->host[0])
        strcpy(host_name, d->host);
    else
        strcpy(host_name, "unknown host");

    switch (STATE(d))
    {
    default:
        log_error("Nanny:  illegal state %d.\n", STATE(d));
        close_socket(d);
        return;
    case CON_GET_NAME:
        log_info("Got Connection from: %s/%s", host_name, ip_addr);
        if (!d->character)
        {
            log_info("Creating new character structure: %s/%s", host_name, ip_addr);
            CREATE(d->character, struct char_data, 1);

            clear_char(d->character);
            d->character->desc = d;
        }
        if (!*arg)
        {
            log_info("Kicking connection: %s/%s", host_name, ip_addr);
            close_socket(d);
            return;
        }
        if (!strncasecmp(arg, "GET ", 4))
        {
            // Clueless bots thinking this is a web server...
            log_info("Kicking connection: %s/%s using %s", host_name, ip_addr, arg);
            ban_address(ip_addr, "HTTP Spam");
            close_socket(d);
            return;
        }

        if (!valid_parse_name(arg, tmp_name))
        {
            log_info("Illegal name: \"%s\" from %s/%s", arg, host_name, ip_addr);
            bad_names++;
            if (last_bad_name > time(NULL) - 10)
            {
                // Yes, this should be tied to the specific ip_addr, but meh
                last_bad_name = time(NULL);
                if (bad_names > 3)
                {
                    bad_names = 0;
                    log_info("3 strikes and you're banned!  %s/%s", host_name, ip_addr);
                    ban_address(ip_addr, "Bad name spam");
                    close_socket(d);
                    return;
                }
            }
            else
            {
                last_bad_name = time(NULL);
            }
            dcprintf(d, "\rIllegal name, please try another.\r\nWHAT is your Name? ");
            return;
        }
        if (check_playing(d, tmp_name))
        {
            log_info("Already playing %s, new connection %s/%s", tmp_name, host_name, ip_addr);
            dcprintf(d, "\rSorry, %s is already playing... you might be cheating!\r\nWhat is YOUR Name? ", tmp_name);
            return;
        }
        if (!ValidPlayer(tmp_name, d->pwd, d->oldpwd))
        {
            dcprintf(d, "\r\nWileyMUD is currently in registration-only mode.\r\nPlease email "
                        "quixadhal@wileymud.themud.org for a character!\r\n");
            STATE(d) = CON_WIZLOCK;
            return;
        }
        strncpy(d->usr_name, tmp_name, 20);
        d->usr_name[19] = '\0';

#ifdef I3
        i3_initchar(d->character);
#endif
#ifdef IMC
        imc_initchar(d->character);
#endif

        log_info("Loading user: %s", d->usr_name);

        // To check 2FA and not corrupt OLD binary save files,
        // we could load in the file to get the password data
        // and if that works, check for the existence of a .2fa
        // file that could be kept right alongside the character
        // file.  If it exists, load that and do the checks, if
        // not, resume the normal path.
        // This could also be strictly an SQL thing, as the game
        // currently requires SQL anyways...

        /*  GET_NAME(d->character) = (char *)strdup(d->usr_name); */
        if (fread_char(d->usr_name, &tmp_store, d->character) > -1)
        {
            /*
             * if (GetPlayerFile(d->usr_name, d->character))
             */
            store_to_char(&tmp_store, d->character);
            strcpy(d->oldpwd, tmp_store.oldpwd);
            strcpy(d->pwd, tmp_store.pwd);
            log_info("%s@%s/%s loaded.", d->usr_name, host_name, ip_addr);
            dcprintf(d, "\r\n%sWHAT is your Password? ", echo_off);
            STATE(d) = CON_GET_PASSWORD;
        }
        else if (load_char(d->usr_name, &tmp_store) > -1)
        {
            /*
             * if (GetPlayerFile(d->usr_name, d->character))
             */
            store_to_char(&tmp_store, d->character);
            strcpy(d->oldpwd, tmp_store.oldpwd);
            strcpy(d->pwd, tmp_store.pwd);
            log_info("%s@%s/%s loaded from old playerfile.", d->usr_name, host_name, ip_addr);
            dcprintf(d, "\r\n%sWHAT is your Password? ", echo_off);
            STATE(d) = CON_GET_PASSWORD;
        }
        else
        {
            if (already_mob_name(d->usr_name))
            {
                dcprintf(d, "\rBut you'd be confused with a MONSTER!.\r\nWHAT is your Name? ");
                return;
            }
            if (banned_name(d->usr_name))
            {
                dcprintf(d, "\rSorry, that is a STUPID name... Try another.\r\nWHAT is your Name? ");
                return;
            }
            GET_NAME(d->character) = (char *)strdup(d->usr_name);
            d->character->player.name[0] = toupper(d->character->player.name[0]);
            dcprintf(d, "\r\n%sChoose a password for %s: ", echo_off, d->usr_name);
            STATE(d) = CON_GET_NEW_PASWORD;
            /*
             * log_info("New player!");
             */
            log_auth(d->character, "NEW PLAYER %s (%s@%s/%s)!", GET_NAME(d->character), d->username, host_name,
                     ip_addr);
        }
        return;
    case CON_GET_PASSWORD:
        if (!*arg)
        {
            close_socket(d);
            return;
        }
        cryptsalt[0] = d->character->player.name[0];
        cryptsalt[1] = d->character->player.name[1];
        arg[10] = '\0';
        strcpy(cryptbuf, crypt(arg, cryptsalt));
        if (strcmp(cryptbuf, d->pwd))
        {
            if (strncmp(arg, d->oldpwd, 10))
            {
                dcprintf(d, "\r***BUZZ!*** Wrong password.\r\n%sGuess again: ", echo_off);
                return;
            }
            log_info("Allowing entry using unencrypted password.");
            strcpy(d->pwd, crypt(d->oldpwd, cryptsalt));
        }
        // At this point, we could check 2FA if we wanted...
        // the secret has to already be stored on the character objct and we
        // have to accept input for the trial password.
        // So if 2FA exists for this character, emit a prompt and go to the
        // new state for that.... eventually ending up in the motd state if it
        // succeeds.
        if (check_reconnect(d))
            return;
        log_auth(d->character, "WELCOME BACK %s (%s@%s/%s)!", GET_NAME(d->character), d->username, host_name, ip_addr);
        if (GetMaxLevel(d->character) > LOW_IMMORTAL)
            dcprintf(d, "\r\n%s", wmotd);
        else
            dcprintf(d, "\r\n%s", motd);
        dcprintf(d, "*** Press RETURN: ");
        STATE(d) = CON_READ_MOTD;
        return;
    case CON_GET_NEW_PASWORD:
        if (!*arg || strlen(arg) < 3)
        {
            dcprintf(d, "\rIllegal password.\r\n%sChoose a password for %s: ", echo_off, d->usr_name);
            return;
        }
        strncpy(d->oldpwd, arg, 10);
        *(d->oldpwd + 10) = '\0';
        dcprintf(d, "\r\n%sPlease retype your password: ", echo_off);
        STATE(d) = CON_CONFIRM_NEW_PASSWORD;
        return;
    case CON_CONFIRM_NEW_PASSWORD:
        if (strncmp(arg, d->oldpwd, 10))
        {
            dcprintf(d, "\r\nBut those don't match!\r\n%sTry your password again: ", echo_off);
            STATE(d) = CON_GET_NEW_PASWORD;
            return;
        }
        cryptsalt[0] = d->character->player.name[0];
        cryptsalt[1] = d->character->player.name[1];
        strcpy(d->pwd, crypt(d->oldpwd, cryptsalt));
        /*
         * PutPasswd(d);
         */
        dcprintf(d, "\r%s", race_menu);
        STATE(d) = CON_GET_RACE;
        return;
    case CON_GET_RACE:
        if (!*arg)
        {
            dcprintf(d, "\r%s", race_menu);
            return;
        }
        switch (*arg)
        {
        default:
            dcprintf(d, "\rThat's not a race.\r\n%s", race_menu);
            STATE(d) = CON_GET_RACE;
            break;
        case '?':
            dcprintf(d, "\r%s", race_help);
            STATE(d) = CON_GET_RACE;
            break;
        case 'd':
        case 'D':
            GET_RACE(d->character) = RACE_DWARF;
            dcprintf(d, "%s", sex_menu);
            STATE(d) = CON_GET_SEX;
            break;
        case 'e':
        case 'E':
            GET_RACE(d->character) = RACE_ELVEN;
            dcprintf(d, "%s", sex_menu);
            STATE(d) = CON_GET_SEX;
            break;
        case 'G':
        case 'g':
            GET_RACE(d->character) = RACE_GNOME;
            dcprintf(d, "%s", sex_menu);
            STATE(d) = CON_GET_SEX;
            break;
        case 'f':
        case 'F':
            GET_RACE(d->character) = RACE_HALFLING;
            dcprintf(d, "%s", sex_menu);
            STATE(d) = CON_GET_SEX;
            break;
        case 'h':
        case 'H':
            GET_RACE(d->character) = RACE_HUMAN;
            dcprintf(d, "%s", sex_menu);
            STATE(d) = CON_GET_SEX;
            break;
        }
        return;
    case CON_GET_SEX:
        switch (*arg)
        {
        default:
            dcprintf(d, "But how will you mate???\r\n%s", sex_menu);
            return;
        case 'm':
        case 'M':
            d->character->player.sex = SEX_MALE;
            break;
        case 'f':
        case 'F':
            d->character->player.sex = SEX_FEMALE;
            break;
        }
        dcprintf(d, "%s", class_menu);
        STATE(d) = CON_GET_CLASS;
        return;

    case CON_GET_CLASS:
        d->character->player.chclass = 0;
        count = 0;
        oops = FALSE;
        for (; *arg && count < 3 && !oops; arg++)
        {
            switch (*arg)
            {
            default:
                dcprintf(d, "I wish *I* could be a \"%s\" too!\r\n%s", arg, class_menu);
                STATE(d) = CON_GET_CLASS;
                oops = TRUE;
                break;
            case '?':
                dcprintf(d, "%s", class_help);
                STATE(d) = CON_GET_CLASS;
                break;
            case 'm':
            case 'M': {
                if (!IS_SET(d->character->player.chclass, CLASS_MAGIC_USER))
                    d->character->player.chclass += CLASS_MAGIC_USER;
                STATE(d) = CON_READ_MOTD;
                count++;
                break;
            }
            case 'c':
            case 'C': {
                if (!IS_SET(d->character->player.chclass, CLASS_CLERIC))
                    d->character->player.chclass += CLASS_CLERIC;
                STATE(d) = CON_READ_MOTD;
                count++;
                break;
            }
            case 'f':
            case 'F':
            case 'w':
            case 'W': {
                if (!IS_SET(d->character->player.chclass, CLASS_WARRIOR))
                    d->character->player.chclass += CLASS_WARRIOR;
                STATE(d) = CON_READ_MOTD;
                count++;
                break;
            }
            case 't':
            case 'T': {
                if (!IS_SET(d->character->player.chclass, CLASS_THIEF))
                    d->character->player.chclass += CLASS_THIEF;
                STATE(d) = CON_READ_MOTD;
                count++;
                break;
            }
            case 'r':
            case 'R': {
                if (!IS_SET(d->character->player.chclass, CLASS_RANGER))
                    d->character->player.chclass = CLASS_RANGER;
                STATE(d) = CON_READ_MOTD;
                count++;
                break;
            }
                /*
                 * case 'd':
                 * case 'D': {
                 * if (!IS_SET(d->character->player.chclass, CLASS_DRUID))
                 * d->character->player.chclass += CLASS_DRUID;
                 * STATE(d) = CON_READ_MOTD;
                 * count++;
                 * break;
                 * }
                 */
            case ' ':
            case ',':
            case '\\': /* ignore these */
            case '/':
                break;
            }

            if ((count > 1) && IS_SET(d->character->player.chclass, CLASS_RANGER))
            {
                dcprintf(d, "Rangers may only be single classed.\r\n%s", class_menu);
                STATE(d) = CON_GET_CLASS;
                oops = TRUE;
            }
        }

        if (STATE(d) != CON_GET_CLASS)
        {
            log_info("%s [%s/%s] new player.", GET_NAME(d->character), host_name, ip_addr);
            init_char(d->character);
            d->pos = create_entry(GET_NAME(d->character));
            save_char(d->character, NOWHERE);
            dcprintf(d, "\r\n%s\r\n*** Press RETURN: ", motd);
            STATE(d) = CON_READ_MOTD;
        }
        return;
    case CON_READ_MOTD:
        dcprintf(d, "%s", login_menu);
        STATE(d) = CON_MENU_SELECT;
        if (WizLock)
        {
            if (GetMaxLevel(d->character) < LOW_IMMORTAL)
            {
                dcprintf(d, "\r\nSorry, the game is locked so the whizz's can break stuff!\r\n");
                STATE(d) = CON_WIZLOCK;
            }
        }
        return;
    case CON_WIZLOCK:
        close_socket(d);
        return;
    case CON_MENU_SELECT:
        switch (*arg)
        {
        default:
            dcprintf(d, "Wrong option.\r\n%s", login_menu);
            return;
        case '0':
            close_socket(d);
            return;
        case '1':
            reset_char(d->character);
            log_info("Loading %s's equipment", d->character->player.name);
            load_char_objs(d->character);
            save_char(d->character, NOWHERE);
            cprintf(d->character, "%s\r\n", WELC_MESSG);
            d->character->next = character_list;
            character_list = d->character;
            if (d->character->in_room == NOWHERE)
            {
                if (IS_IMMORTAL(d->character))
                {
                    if (!real_roomp(GET_HOME(d->character)))
                        GET_HOME(d->character) = 1000;
                }
                else
                {
                    if (!real_roomp(GET_HOME(d->character)))
                        GET_HOME(d->character) = DEFAULT_HOME;
                }
            }
            else
            {
                if (real_roomp(d->character->in_room))
                {
                    GET_HOME(d->character) = d->character->in_room;
                }
                else
                {
                    if (IS_IMMORTAL(d->character))
                        GET_HOME(d->character) = 1000;
                    else
                        GET_HOME(d->character) = DEFAULT_HOME;
                }
            }
            char_to_room(d->character, GET_HOME(d->character));
            if (GetMaxLevel(d->character) >= LOKI)
            {
                if ((strcasecmp(GET_NAME(d->character), "Quixadhal")))
                {
                    int x = 0;

                    cprintf(d->character, "Fool!  You DARE challenge the Dread Lord?\r\n");
                    for (x = 0; x < ABS_MAX_CLASS; x++)
                        if (HasClass(d->character, 1 << x))
                            GET_LEVEL(d->character, x) = LOW_IMMORTAL;
                    save_char(d->character, NOWHERE);
                }
            }
            if (GetMaxLevel(d->character) < LOW_IMMORTAL)
                act("$n has entered the game.", TRUE, d->character, 0, 0, TO_ROOM);
            else
            {
                if (IS_SET(d->character->specials.act, PLR_STEALTH))
                    REMOVE_BIT(d->character->specials.act, PLR_STEALTH);
                d->character->invis_level = 0;
                if (GetMaxLevel(d->character) < LOKI)
                    iprintf("Comrade %s has entered the world.\r\n", NAME(d->character));
            }

            if (!IS_SET(d->character->specials.affected_by, AFF_GROUP))
                SET_BIT(d->character->specials.affected_by, AFF_GROUP);
            STATE(d) = CON_PLAYING;
            if (!GetMaxLevel(d->character))
                start_character(d->character);
            // update_player_list_entry(d);
            do_look(d->character, "", 15);
            d->prompt_mode = 1;
#ifdef I3
            if (!I3_hasname(I3LISTEN(d->character), "wiley"))
                I3_listen_channel(d->character, "wiley");
#endif
            {
                struct obj_cost cost;

                recep_offer(d->character, NULL, &cost);
                new_save_equipment(d->character, &cost, FALSE);
                save_obj(d->character, &cost, FALSE);
                save_char(d->character, NOWHERE);
            }
            return;
        case '2':
            // dcprintf(d, "I don't think we every fully implemented this?\r\nRun away!\r\n");
            // dcprintf(d, "%s", login_menu);
            // STATE(d) = CON_MENU_SELECT;
            // return;
            dcprintf(d, "Enter a text you'd like others to see when they look at you.\r\nTerminate with a '@'.\r\n");
            if (d->character->player.description)
            {
                dcprintf(d, "Old description :\r\n%s", d->character->player.description);
                DESTROY(d->character->player.description);
                d->character->player.description = 0;
            }
            d->str = &d->character->player.description;
            d->max_str = 240;
            STATE(d) = CON_EDIT_DESCRIPTION;
            return;
        case '3':
            dcprintf(d, "%s\r\n*** Press RETURN: ", the_story);
            STATE(d) = CON_READ_MOTD;
            return;
        case '4':
            dcprintf(d, "%sEnter a new password: ", echo_off);
            STATE(d) = CON_GET_CHANGE_PASSWORD;
            return;
        case '5': {
            struct descriptor_data *dd;
            struct char_data *person;
            int lcount = 0;

            dcprintf(d, "Players Connected.\r\n\r\n");
            for (dd = descriptor_list; dd; dd = dd->next)
                if (!dd->connected)
                {
                    person = dd->character;
                    if (!IS_IMMORTAL(person) || IS_IMMORTAL(d->character))
                    {
                        lcount++;
                        dcprintf(d, "%s %s %s\r\n", (person->player.pre_title ? person->player.pre_title : ""),
                                 GET_NAME(person), person->player.title);
                    }
                }
            dcprintf(d, "Total Connected %d\r\n*** Press RETURN: ", lcount);
            STATE(d) = CON_READ_MOTD;
            break;
        }
            return;
        case '6':
            if (IS_IMMORTAL(d->character))
            {
                dcprintf(d, "\r\nSorry, you are a slave to the source...  There is no escape for you!\r\n%s",
                         login_menu);
                break;
            }
            dcprintf(d, "%s", suicide_warn);
            STATE(d) = CON_SUICIDE;
            return;
        }
        return;
    case CON_SUICIDE:
        if (!strcmp(arg, "I want to DIE!"))
        {
            char name[MAX_INPUT_LENGTH], *t_ptr, old[MAX_INPUT_LENGTH], bkp[MAX_INPUT_LENGTH];

            strcpy(name, d->usr_name);
            t_ptr = name;
            for (; *t_ptr != '\0'; t_ptr++)
                *t_ptr = LOWER(*t_ptr);
            snprintf(old, MAX_INPUT_LENGTH, "ply/%c/%s.p", name[0], name);
            snprintf(bkp, MAX_INPUT_LENGTH, "ply/%c/%s.p-dead", name[0], name);
            rename(old, bkp);
            snprintf(old, MAX_INPUT_LENGTH, "ply/%c/%s.o", name[0], name);
            snprintf(bkp, MAX_INPUT_LENGTH, "ply/%c/%s.o-dead", name[0], name);
            rename(old, bkp);
            snprintf(old, MAX_INPUT_LENGTH, "ply/%c/%s.chr", name[0], name);
            snprintf(bkp, MAX_INPUT_LENGTH, "ply/%c/%s.chr-dead", name[0], name);
            rename(old, bkp);
            snprintf(old, MAX_INPUT_LENGTH, "ply/%c/%s.obj", name[0], name);
            snprintf(bkp, MAX_INPUT_LENGTH, "ply/%c/%s.obj-dead", name[0], name);
            rename(old, bkp);
            for (i = 0; i < number_of_players; i++)
            {
                if (list_of_players[i])
                    if (!strncasecmp(list_of_players[i], name, strlen(name)))
                        if (list_of_players[i][strlen(name)] == ' ')
                        {
                            DESTROY(list_of_players[i]);
                            list_of_players[i] = NULL;
                            actual_players--;
                        }
            }
            // dump_player_list();
            log_info("-- SUICIDE -- %s is no more!\n", name);
            dcprintf(d, "%s", suicide_done);
            STATE(d) = CON_WIZLOCK;
            return;
        }
        dcprintf(d, "You are SAVED!\r\n%s", login_menu);
        STATE(d) = CON_MENU_SELECT;
        return;
    case CON_GET_CHANGE_PASSWORD:
        if (!*arg || strlen(arg) < 3)
        {
            dcprintf(d, "\rIllegal password.\r\n%sPassword: ", echo_off);
            return;
        }
        strncpy(d->oldpwd, arg, 10);
        *(d->oldpwd + 10) = '\0';
        dcprintf(d, "\r\n%sPlease retype password: ", echo_off);
        STATE(d) = CON_CONFIRM_CHANGE_PASSWORD;
        return;
    case CON_CONFIRM_CHANGE_PASSWORD:
        if (strncmp(arg, d->oldpwd, 10))
        {
            dcprintf(d, "\rPasswords don't match.\r\n%sRetype password: ", echo_off);
            STATE(d) = CON_GET_CHANGE_PASSWORD;
            return;
        }
        cryptsalt[0] = d->character->player.name[0];
        cryptsalt[1] = d->character->player.name[1];
        strcpy(d->pwd, crypt(d->oldpwd, cryptsalt));
        /*
         * PutPasswd(d);
         */
        dcprintf(d, "%s\r\nDone. You must enter the game to make the change final\r\n%s", echo_on, login_menu);
        STATE(d) = CON_MENU_SELECT;
        return;
    }
}

void update_player_list_entry(struct descriptor_data *d)
{
    char buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
    char tmpbuf[80] = "\0\0\0\0\0\0\0";
    int i = 0;
    int found = 0;
    // struct timeb                            right_now;
    struct timespec right_now;
    struct tm *now_part = NULL;

    if (DEBUG > 2)
        log_info("called %s with %08zx", __PRETTY_FUNCTION__, (size_t)d);

    if ((!d) || d->connected)
        return;
    strcpy(tmpbuf, d->usr_name);
    for (i = 0; i < strlen(tmpbuf); i++)
        tmpbuf[i] = tolower(tmpbuf[i]);
    // ftime(&right_now);
    clock_gettime(CLOCK_REALTIME, &right_now);
    now_part = localtime((const time_t *)&right_now);
    snprintf(buf, MAX_INPUT_LENGTH, "%-16s %s@%s %02d.%02d.%02d %02d:%02d ", tmpbuf, d->username, d->host, now_part->tm_year,
            now_part->tm_mon + 1, now_part->tm_mday, now_part->tm_hour, now_part->tm_min);
    if (!(d->character))
        strcat(buf, "1\n");
    else
        scprintf(buf, MAX_INPUT_LENGTH, "%d\n", GetMaxLevel(d->character));
    for (i = 0; i < number_of_players; i++)
    {
        if (list_of_players[i])
            if (!(strncasecmp(list_of_players[i], buf, strlen(tmpbuf) + 1)))
            {
                found = 1;
                DESTROY(list_of_players[i]);
                STRDUP(list_of_players[i], buf);
            }
    }
    if (!found)
    {
        RECREATE(list_of_players, char *, number_of_players + 1);

        STRDUP(list_of_players[number_of_players], buf);
        number_of_players++;
        actual_players++;
    }
    // dump_player_list();
}

#if 0
void PutPasswd(struct descriptor_data *d)
{
    FILE                                   *pfd = NULL;

    if (DEBUG > 2)
	log_info("called %s with %08zx", __PRETTY_FUNCTION__, (size_t) d);

    if ((pfd = fopen(PASSWD_NEW, "a")) == NULL) {
	log_info("Cannot save password data for new user!\r\n");
    } else {
	fprintf(pfd, "%s %s %s@%s %ld 1\n", d->usr_name,
		d->pwd, d->username, d->host, (long int)time(NULL));
	FCLOSE(pfd);
    }
    // update_player_list_entry(d);
}
#endif

int ValidPlayer(char *who, char *pwd, char *oldpwd)
{
    FILE *pwd_fd = NULL;
    char tname[40] = "\0\0\0\0\0\0\0";
    char *t_ptr = NULL;
    char pname[40] = "\0\0\0\0\0\0\0";
    char passwd[40] = "\0\0\0\0\0\0\0";
    char email[80] = "\0\0\0\0\0\0\0";
    long timestamp = 0L;

    if (DEBUG > 2)
        log_info("called %s with %s, %s, %s", __PRETTY_FUNCTION__, VNULL(who), VNULL(pwd), VNULL(oldpwd));

    if (!(pwd_fd = fopen(PASSWD_FILE, "r")))
    {
        strcpy(oldpwd, "NOT USED");
        strcpy(pwd, "NOT USED");
        log_info("Password checking disabled!");
        return TRUE;
    }
    log_info("Searching passwd file for %s...", who);

    strcpy(tname, who);
    for (t_ptr = tname; *t_ptr; t_ptr++)
        *t_ptr = LOWER(*t_ptr);

    while (fscanf(pwd_fd, " %s %s %s %ld\n", pname, passwd, email, &timestamp) > 1)
    {
        if (!strcmp(tname, pname))
        {
            log_info("Found %s in passwd file.", tname);
            if (!strcmp(passwd, "*"))
            {
                strcpy(oldpwd, "IS VALID");
                strcpy(pwd, "IS VALID");
            }
            else
            {
                strcpy(oldpwd, passwd);
                strcpy(pwd, passwd);
            }
            FCLOSE(pwd_fd);
            return TRUE;
        }
    }
    FCLOSE(pwd_fd);
    return FALSE;
}

#if 0
int GetPlayerFile(char *name, struct char_data *where)
{
    struct char_file_u                      tmp_store;

    if (DEBUG > 2)
	log_info("called %s with %s, %s", __PRETTY_FUNCTION__, VNULL(name), SAFE_NAME(where));

    if ((load_char(name, &tmp_store)) > -1) {
	store_to_char(&tmp_store, where);
	return TRUE;
    }
    return FALSE;
}
#endif

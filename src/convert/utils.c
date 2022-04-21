#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
//#include <sys/timeb.h>
#include <ctype.h>
#include <openssl/md5.h>
/* #include <sys/time.h> */
/* #include <sys/timeb.h> */

#include "include/bug.h"
#include "include/structs.h"
#include "include/main.h"

#define _UTILS_C
#include "include/utils.h"

void spin(FILE *fp)
{
    static char spinner[4] = {'|', '/', '-', '\\'};
    static int spin_state = 0;

    fprintf(fp, "%c\b", spinner[(++spin_state > 3) ? (spin_state = 0) : spin_state]);
    fflush(fp);
}

void status(FILE *fp, const char *Str, ...)
{
    va_list arg;
    char tmpstr[MAX_STRING_LEN];

    if (!Str || !*Str)
        return;

    va_start(arg, Str);
    vsnprintf(tmpstr, MAX_STRING_LEN, Str, arg);
    va_end(arg);

    if (Verbose)
    {
        fprintf(fp, "%s\n", tmpstr);
        fflush(fp);
    }
    else if (!Quiet)
    {
        tmpstr[77] = '\0'; // Truncate string at 78 characters.
        fprintf(fp, "%s", tmpstr);
        for (int x = strlen(tmpstr); x < 79; x++)
            fprintf(fp, " ");
        for (int x = strlen(tmpstr); x < 79; x++)
            fprintf(fp, "\b");
        fflush(fp);
    }
}

/*
 * All this does is call strdup(), but it does error checking internally,
 * aborting the program if you have no memory...  This makes the code more
 * readable and how often do you really recover from running out of RAM?
 */
char *my_strdup(char *Str)
{
    char *tmp;

    if (!Str)
        return ""; /* Note: This works AROUND the bug! */
    if (!(tmp = (char *)strdup(Str)))
        log_fatal("Cannot get memory to strdup(\"%s\")!", Str);
    return tmp;
}

/*
 * This just opens a file.  It makes the code more readable without
 * sacrificing the debugging information.
 */
FILE *open_file(char *Filename, char *Mode)
{
    FILE *fp;

    if (!(fp = fopen(Filename, Mode)))
        log_fatal("Cannot open (%s) for (%s) access!", Filename, Mode);
    return fp;
}

/*
 * This just calls malloc/calloc with a debugging wrapper.
 */
char *get_mem(long Count, long Size)
{
    char *Memory;

    if (!(Memory = (char *)calloc(Count, Size)))
        log_fatal("Cannot allocate %d bytes!", Count * Size);
    return Memory;
}

/*
 * Sames as get_mem(), but this is for realloc().
 */
char *get_more_mem(char *Memory, long Count, long Size)
{
    char *NewMemory;

    /* REMEMBER to zero out your variables on YOUR END!  Malloc is evil, and
     * realloc shares this trait.  I normally use calloc, but since I don't want
     * to rewrite realloc myself (I *HOPE* the system version is coded in assy!),
     * I figured why make the get_mem() function inconsistant?
     */
    if (!(NewMemory = (char *)realloc(Memory, Count * Size)))
        log_fatal("Cannot reallocate %d bytes!", Count * Size);
    return NewMemory;
}

/*
 * This routine will read a line from the given file pointer and return a
 * string containing that line, with the end return stripped.  If the next
 * line was blank, or a comment (denoted by a leading '*', it will be skipped.
 * the line-number and byte-position counters passed in will be updated.
 * Any failure to get a valid line will result in a NULL return value.
 * In this case, line and pos will remain at their last valid positions.
 */
char *get_line(FILE *fp, long *Line, long *Pos, int Skip)
{
    static char tmp[MAX_STRING_LEN];

    if (!fp)
        return NULL;
    do
    {
        bzero(tmp, MAX_STRING_LEN);
        *Pos = ftell(fp);
        if (!fgets(tmp, MAX_STRING_LEN - 1, fp))
            return NULL;
        (*Line)++;
        if (*tmp)
            if (tmp[strlen(tmp) - 1] == '\n')
                tmp[strlen(tmp) - 1] = '\0';
        if (Skip)
        {
            if (!*tmp)
                continue; /* ignore blank lines */
            if (*tmp == '*')
                continue; /* lines starting with '*' are comments */
        }
        break;
    } while (!feof(fp));
    return tmp;
}

/*
 * This routine verifies a chance of sanity in file pointer position.
 * It just saves the current pointer, moves where you tell it, and compares
 * what is there with the character you told it to expect.  It then
 * repositions the file pointer and returns the result of the comparison.
 */
int verify_pos(FILE *fp, long Pos, int Check)
{
    long OldPos;
    char c;

    if (!fp)
        return 0;
    if ((OldPos = ftell(fp)) < 0)
        return 0;
    if (fseek(fp, Pos, 0) < 0)
        return 0;
    if ((c = fgetc(fp)) < 0)
        return 0;
    if (fseek(fp, OldPos, 0) < 0)
        return 0;
    return (c == Check);
}

/*
 * This routine will repeatedly call get_line() on a given file pointer until
 * it finds a line which ends in a tilde '~'.  It will construct a string which
 * contains all the text from the file pointer's original position up to, but
 * not including, the tilde.  We may need to suppress the normal black-line
 * and comment-line skipping rules.
 * ARGH!  The wiley people are sloppy!  We now have to extend the routine to
 * allow for whitespace after the tilde... or manually check the rooms...
 * adding routine!
 * UGH!  We also must strip trailing \n's for cases of ~ strings on their own.
 */
char *get_tilde_string(FILE *fp, long *Line, long *Pos)
{
    static char block[MAX_STRING_LEN];
    static char tmp[MAX_STRING_LEN];

    if (!fp)
        return NULL;
    bzero(block, MAX_STRING_LEN);
    while (1)
    {
        int i = 0;

        bzero(tmp, MAX_STRING_LEN);
        do
        {
            bzero(tmp, MAX_STRING_LEN);
            *Pos = ftell(fp);
            if (!fgets(tmp, MAX_STRING_LEN - 1, fp))
                return NULL;
            (*Line)++;
            if (*tmp)
                if (tmp[strlen(tmp) - 1] == '\n')
                    tmp[strlen(tmp) - 1] = '\0';
            break;
        } while (!feof(fp));
        if (*tmp && (tmp[i = strlen(tmp) - 1] == '~'))
        {
            tmp[strlen(tmp) - 1] = '\0';
            strcat(block, tmp);
            break;
        }
        else if ((tmp[i] == ' ') || (tmp[i] == '\t'))
        {
            int done = 0;

            for (; i >= 0; i--)
            {
                if (tmp[i] == '~')
                {
                    tmp[i] = '\0';
                    strcat(block, tmp);
                    done = 1;
                    break;
                }
                else
                {
                    if ((tmp[i] == ' ') || (tmp[i] == '\t'))
                        continue;
                    else
                        break;
                }
            }
            if (done)
                break;
        }
        if (*tmp)
            strcat(block, tmp);
        if (HardReturns)
            strcat(block, "\n");
        else
            strcat(block, " ");
    }
    if (!HardReturns)
        if (block[strlen(block) - 1] == ' ')
            block[strlen(block) - 1] = '\0';
    return block;
}

/*
 * This function collapses arbitrary strings into alphabetic sets.
 * Furthermore, it collapses spaces upto the first non-space/non-alphabetic
 * character, and then throws the entire remainder away.
 * It then does a case conversion to make the first char capital.
 * It also accepts a single-tick (') as a synonym for space.
 * I decided to allow '_' to pass unconverted...
 * It also now accepts '/' and '-' as synonyms for '_'.
 * decided that mapping spaces into '_' makes it more readable.
 * I also decided to strip trailing spaces from the string...
 * Filtered out \'s now as well.
 * Decided that collapsing all of these is nicer for LOoooooooNG names!
 * replace the first continue with "Block[j++]= '_'" for the old behaviour.
 */
char *remap_name(char *Old)
{
    static char Block[MAX_STRING_LEN];
    int i, j;

    if (!Old)
        return NULL;
    bzero(Block, MAX_STRING_LEN);
    for (j = i = 0; i < strlen(Old); i++)
    {
        if (isalpha(Old[i]))
            Block[j++] = Old[i];
        else if ((Old[i] == '_') || (Old[i] == ' ') || (Old[i] == '/') || (Old[i] == '\\') || (Old[i] == '-'))
            continue;
        else if ((Old[i] == '\'') || (Old[i] == '"'))
            continue;
        else
            break;
    }
    if (*Block)
    {
        for (i = strlen(Block) - 1; i >= 0; i--)
            if (Block[i] != '_')
                break;
            else
                Block[i] = '\0';
    }
    if (*Block)
        *Block = toupper(*Block);
    return Block;
}

/* This routine accepts a string and breaks it apart into words.
 * It then returns a structure containing the number of words and an
 * array of those words.
 */
keyword *make_keyword_list(char *String)
{
    keyword *Keyword;
    char *tmp, *tmp2;

    Keyword = (keyword *)get_mem(1, sizeof(keyword));
    Keyword->Count = 0;
    Keyword->Word = (char **)get_mem(1, sizeof(char *));
    if (!String || !*String)
        return Keyword;
    tmp = my_strdup(String);
    if ((Keyword->Word[Keyword->Count] = (char *)strtok(tmp, " ")))
    {
        Keyword->Count++;
        Keyword->Word = (char **)get_more_mem((char *)Keyword->Word, Keyword->Count + 1, sizeof(char *));
        Keyword->Word[Keyword->Count] = NULL;
    }
    while ((tmp2 = (char *)strtok(NULL, " ")))
    {
        Keyword->Word[Keyword->Count] = my_strdup(tmp2);
        Keyword->Count++;
        Keyword->Word = (char **)get_more_mem((char *)Keyword->Word, Keyword->Count + 1, sizeof(char *));
        Keyword->Word[Keyword->Count] = NULL;
    }
    return Keyword;
}

char *timestamp(void)
{
    static char Result[MAX_STRING_LEN];
    // struct timeb                            right_now;
    struct timespec right_now;
    struct tm *now_part;

    // ftime(&right_now);
    clock_gettime(CLOCK_REALTIME, &right_now);
    now_part = localtime((const time_t *)&right_now);
    snprintf(Result, MAX_STRING_LEN, "%04d%02d%02d.%02d%02d%02d.%03d",
             now_part->tm_year + 1900, now_part->tm_mon + 1,
             now_part->tm_mday, now_part->tm_hour, now_part->tm_min, now_part->tm_sec,
             // right_now.millitm);
             ((int)(right_now.tv_nsec / 1000000)));
    return Result;
}

void sscanf_dice(char *str, int *x, int *y, int *z)
{
    char sign[2];

    if (!str || !*str || !x || !y || !z)
        return;
    *x = *y = *z = sign[1] = 0;
    *sign = '+';
    sscanf(str, " %dd%d%[+-]%d", x, y, sign, z);
    if (!*y)
        *y = 1;
    if (*sign == '-')
        *z = -*z;
}

char *ordinal(int x)
{
    if (x < 14 && x > 10)
        x = 4;
    else
        x %= 10;
    switch (x)
    {
    case 1:
        return "st";
    case 2:
        return "nd";
    case 3:
        return "rd";
    default:
        return "th";
    }
}

char *md5_hex(const char *str)
{
    int length;
    int i;
    MD5_CTX c;
    unsigned char digest[16];
    static char result[33];

    MD5_Init(&c);

    length = strlen(str);
    while (length > 0)
    {
        if (length > 512)
        {
            MD5_Update(&c, str, 512);
        }
        else
        {
            MD5_Update(&c, str, length);
        }
        length -= 512;
        str += 512;
    }

    MD5_Final(digest, &c);

    for (i = 0; i < 16; ++i)
    {
        snprintf(&(result[i * 2]), 16 * 2, "%02x", (unsigned int)digest[i]);
    }
    result[32] = '\0';

    return result;
}

char *json_escape(char *thing)
{
    static char result[MAX_STRING_LEN];

    bzero(result, MAX_STRING_LEN);
    for (char *s = thing; *s; s++)
    {
        if (*s == '"' || *s == '\\' || ('\x00' <= *s && *s <= '\x1f'))
        {
            scprintf(result, MAX_STRING_LEN, "\\u%04x", (unsigned int)*s);
        }
        else
        {
            scprintf(result, MAX_STRING_LEN, "%c", *s);
        }
    }
    return result;
}

/*
 * This is a wrapper for snprintf() with strlcat(), to basically allow
 * us to append formatted data to a string, passing in the length limit
 * of the string buffer we're pointing at.
 *
 * Think of it as strcat_printf(), but shorter.
 */
int scprintf(char *buf, size_t limit, const char *Str, ...)
{
    va_list arg;
    int len = 0;
    int result = 0;

    if (buf && limit > 0 && Str && *Str)
    {
        len = strlen(buf);
        va_start(arg, Str);
        result = vsnprintf((buf + len), limit - len, Str, arg);
        va_end(arg);
    }
    return result + len;
}

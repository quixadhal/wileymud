#ifndef _COLOUR_H
#define _COLOUR_H

/*
 * Ansi colours and VT100 codes
 * Used in #PLAYER
 *
 * On most machines that use ANSI, namely the IBM PCs, the decimal value for
 * the escape character is 27 (1B hex).     Change this value when needed.
 */

#define ESC        '\x1b'

#define BLACK      "\x1b[30m"				       /* These are foreground colour codes */
#define RED        "\x1b[31m"
#define GREEN      "\x1b[32m"
#define YELLOW     "\x1b[33m"
#define BLUE       "\x1b[34m"
#define PURPLE     "\x1b[35m"
#define CYAN       "\x1b[36m"
#define GREY       "\x1b[37m"

#define B_BLACK    "\x1b[40m"				       /* These are background colour codes */
#define B_RED      "\x1b[41m"
#define B_GREEN    "\x1b[42m"
#define B_YELLOW   "\x1b[43m"
#define B_BLUE     "\x1b[44m"
#define B_PURPLE   "\x1b[45m"
#define B_CYAN     "\x1b[46m"
#define B_GREY     "\x1b[47m"

/* Below are VT100 and ANSI codes (above are ANSI exclusively)       */

#define EEEE       "\x1b#8"				       /* Turns screen to EEEEs */
#define CLRSCR     "\x1b[2j"				       /* Clear screen */
#define CLREOL     "\x1b[K"				       /* Clear to end of line */

#define UPARR      "\x1b[A"				       /* Up one line */
#define DOWNARR    "\x1b[B"				       /* Down one line */
#define RIGHTARR   "\x1b[C"				       /* Right one column */
#define LEFTARR    "\x1b[D"				       /* Left one column */
#define HOMEPOS    "\x1b[H"				       /* Home (upper left) */

#define BOLD       "\x1b[1m"				       /* High intensity */
#define FLASH      "\x1b[5m"				       /* Flashing text */
#define INVERSE    "\x1b[7m"				       /* XORed back and fore */
#define NTEXT      "\x1b[0m"				       /* Normal text (grey) */

/*
 * Other codes of note for future ANSI development:
 * The <esc>[y;xH code works nicely for positioning the cursor.
 */

#ifndef _COLOUR_C
extern const struct colour_data         colour_table[];
#endif

char                                   *ansi_uppercase(char *txt);
void                                    ansi_colour(const char *txt, CHAR_DATA * ch);

#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#define _COLOUR_C
#include "colour.h"

const struct colour_data colour_table[] =
{
  {"\x1b[30m", "$R$0", "black", 0},
  {"\x1b[34m", "$R$1", "blue", 1},
  {"\x1b[32m", "$R$2", "green", 2},
  {"\x1b[36m", "$R$3", "cyan", 3},
  {"\x1b[31m", "$R$4", "red", 4},
  {"\x1b[35m", "$R$5", "purple", 5},
  {"\x1b[33m", "$R$6", "brown", 6},
  {"\x1b[37m", "$R$7", "grey", 7},
  {"\x1b[30;1m", "$B$0", "dark_grey", 8},
  {"\x1b[34;1m", "$B$1", "light_blue", 9},
  {"\x1b[32;1m", "$B$2", "light_green", 10},
  {"\x1b[36;1m", "$B$3", "light_cyan", 11},
  {"\x1b[31;1m", "$B$4", "light_red", 12},
  {"\x1b[35;1m", "$B$5", "magenta", 13},
  {"\x1b[33;1m", "$B$6", "yellow", 14},
  {"\x1b[37;1m", "$B$7", "white", 15},
  {"\x1b[1m", "$B", "bold", 16},
  {"\x1b[5m", "$F", "flashing", 17},
  {"\x1b[7m", "$I", "inverse", 18},
  {"\x1b[0m", "$R", "normal", 19}
};

/*
 * Take a string like <[3m<[1mhi and make it <[3m<[1mHi
 * Thanx to Arcane@max.tiac.net from TIACMUD
 * Changes made by Locke.
 */
char *ansi_uppercase(char *txt)
{
  char *str;

  str = txt;

  while (*str) {
    if (*str == ESC)
      str++;
    else
      break;
    if (*str == '[')
      str++;
    else
      break;
    while (isdigit(*str))
      str++;
    if (*str == 'm')
      str++;
    else
      break;
  }

  *str = toupper(*str);
  return txt;
}

void ansi_colour(const char *txt, CHAR_DATA *ch)
{
  if (txt != NULL && ch->desc != NULL) {
    if (!IS_SET(ch->act2, PLR_ANSI) && !IS_SET(ch->act2, PLR_VT100))
      return;
    else if (IS_SET(ch->act2, PLR_VT100) && !IS_SET(ch->act2, PLR_ANSI)) {
      if (!str_cmp(txt, GREEN)
	  || !str_cmp(txt, RED)
	  || !str_cmp(txt, BLUE)
	  || !str_cmp(txt, BLACK)
	  || !str_cmp(txt, CYAN)
	  || !str_cmp(txt, GREY)
	  || !str_cmp(txt, YELLOW)
	  || !str_cmp(txt, PURPLE))
	return;
    }
    write_to_buffer(ch->desc, txt, strlen(txt));
    return;
  }
}

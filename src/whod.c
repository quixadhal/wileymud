/*
 * Opens a port (the port # above the port # the game itself is
 * being run). On this port people can connect to see who's on.
 * The player wont have to enter the game to see if it's worth
 * playing at the time, thus saving money.
 *
 * Change the following #define-statements to adjust the
 * WHOD to your server.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>

#include "version.h"
#include "global.h"
#include "bug.h"
#include "db.h"
#include "comm.h"
#include "utils.h"
#include "interpreter.h"
#include "multiclass.h"
#define _WHOD_C
#include "whod.h"

/*
 * In function run_the_game(int port):
 *   ...
 *   init_whod(port);
 *   dlog("Entering game loop.");
 *   game_loop(s);
 *   close_sockets(s);
 *   close_whod();
 *   ...
 *
 *   In function game_loop ():
 *   ...
 *   sigsetmask(mask);
 *   whod_loop();
 *   if (select(maxdesc + 1, &input_set, &output_set, &exc_set, &null_time) < 0)
 *   ...
 */

static long                             disconnect_time = 0L;
static int                              s = 0;
int                                     whod_mode = DEFAULT_MODE;
static int                              state = 0;
static int                              whod_port = 0;

/*
 * Function   : do_whod
 * Parameters : doer, argument string, number of WHOD command (not used)
 * Returns    : --
 * Description: MUD command to set the mode of the WHOD-connection according
 *              to the command string.                                  
 */

void do_whod(struct char_data *ch, char *arg, int cmd)
{
  char                                    buf[256] = "\0\0\0";
  char                                    tmp[MAX_INPUT_LENGTH] = "\0\0\0";
  int                                     bit = 0;
  char                                   *modes[] = {
    "name",
    "title",
    "site",
    "on",
    "off",
    "level",
    "idle",
    "room",
    "\n"
  };

  if (DEBUG)
    dlog("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(arg), cmd);

  half_chop(arg, buf, tmp);
  if (!*buf) {
    cprintf(ch, "Current WHOD mode:\n\r------------------\n\r");
    sprintbit((long)whod_mode, (const char **)modes, buf);
    cprintf(ch, "%s\n\r", buf);
    return;
  }
  if ((bit = old_search_block(buf, 0, strlen(buf), modes, FALSE)) == -1) {
    cprintf(ch, "That mode does not exist.\n\rAvailable modes are:\n\r");
    *buf = '\0';
    for (bit = 0; *modes[bit] != '\n'; bit++) {
      strcat(buf, modes[bit]);
      strcat(buf, " ");
    }
    cprintf(ch, "%s\n\r", buf);
    return;
  }
  bit--;						       /* Is bit no + 1 */
  if (SHOW_ON == 1 << bit) {
    if (IS_SET(whod_mode, SHOW_ON))
      cprintf(ch, "WHOD already turned on.\n\r");
    else {
      if (IS_SET(whod_mode, SHOW_OFF)) {
	REMOVE_BIT(whod_mode, SHOW_OFF);
	SET_BIT(whod_mode, SHOW_ON);
	cprintf(ch, "WHOD turned on.\n\r");
	dlog("WHOD turned on by %s.", GET_NAME(ch));
      }
    }
  } else {
    if (SHOW_OFF == 1 << bit) {
      if (IS_SET(whod_mode, SHOW_OFF))
	cprintf(ch, "WHOD already turned off.\n\r");
      else {
	if (IS_SET(whod_mode, SHOW_ON)) {
	  REMOVE_BIT(whod_mode, SHOW_ON);
	  SET_BIT(whod_mode, SHOW_OFF);
	  cprintf(ch, "WHOD turned off.\n\r");
	  dlog("WHOD turned off by %s.", GET_NAME(ch));
	}
      }
    } else {
      if (IS_SET(whod_mode, 1 << bit)) {
	cprintf(ch, "%c%s will not be shown on WHOD.\n\r",
		toupper(modes[bit][0]), &modes[bit][1]);
	dlog("%c%s removed from WHOD by %s.",
	    toupper(modes[bit][0]), &modes[bit][1], GET_NAME(ch));
	REMOVE_BIT(whod_mode, 1 << bit);
	return;
      } else {
	cprintf(ch, "%c%s will now be shown on WHOD.\n\r",
		toupper(modes[bit][0]), &modes[bit][1]);
	dlog("%c%s added to WHOD by %s.", toupper(modes[bit][0]), &modes[bit][1], GET_NAME(ch));
	SET_BIT(whod_mode, 1 << bit);
	return;
      }
    }
  }

  return;
}

/*
 * ------ WHO Daemon staring here ------          
 */

/*
 * Function   : init_whod
 * Parameters : Port #-1 the daemon should be run at
 * Returns    : --
 * Description: Opens the WHOD port and sets the state of WHO-daemon to OPEN
 */

void init_whod(int port)
{
  if (DEBUG > 2)
    dlog("called %s with %d", __PRETTY_FUNCTION__, port);

  whod_port = port + 1;
  dlog("WHOD port opened.");
  s = init_socket(whod_port);
  state = WHOD_OPEN;
}

/*
 * Function   : close_whod
 * Parameters : --
 * Returns    : --
 * Description: Closes the WHOD port and sets the state of WHO-daemon to
 *              CLOSED.                                                 
 */

void close_whod(void)
{
  if (DEBUG > 2)
    dlog("called %s with no arguments", __PRETTY_FUNCTION__);

  if (state != WHOD_CLOSED) {
    state = WHOD_CLOSED;
    close(s);
    dlog("WHOD port closed.");
  }
}

/*
 * Function   : whod_loop
 * Parameters : --
 * Returns    : --
 * Description: Serves incoming WHO calls.                             
 */

void whod_loop(void)
{
  int                                     nfound = 0;
  int                                     size = 0;
  int                                     players = 0;
  int                                     gods = 0;
  int                                     char_index = 0;
  fd_set                                  in;
  unsigned long                           hostlong = 0L;
  struct timeval                          timeout;
  struct sockaddr_in                      newaddr;
  char                                    buf[MAX_STRING_LENGTH] = "\0\0\0";
  struct char_data                       *ch = NULL;
  struct hostent                         *hent = NULL;
  time_t                                  ct;
  time_t                                  ot;
  char                                   *tmstr = NULL;
  char                                   *otmstr = NULL;
  long                                    ttime = 0L;
  long                                    thour = 0L;
  long                                    tmin = 0L;
  long                                    tsec = 0L;

  /*
   * extern long Uptime; 
   */

  static int                              newdesc = 0;

  if (DEBUG > 2)
    dlog("called %s with no arguments", __PRETTY_FUNCTION__);

  switch (state) {
/****************************************************************/
    case WHOD_OPENING:
      s = init_socket(whod_port);
      dlog("WHOD port opened.");
      state = WHOD_OPEN;
      break;

/****************************************************************/
    case WHOD_OPEN:

      timeout.tv_sec = 0;
      timeout.tv_usec = 100;

      FD_ZERO(&in);
      FD_SET(s, &in);

      nfound = select(s + 1, &in, (fd_set *) 0, (fd_set *) 0, &timeout);

      if (FD_ISSET(s, &in)) {
	size = sizeof(newaddr);
	getsockname(s, (struct sockaddr *)&newaddr, &size);

	if ((newdesc = accept(s, (struct sockaddr *)&newaddr, &size)) < 0) {
	  perror("WHOD - Accept");
	  return;
	}
	if ((hent =
	     gethostbyaddr((char *)&newaddr.sin_addr, sizeof(newaddr.sin_addr), AF_INET)))
	  sprintf(buf, "WHO request from %s served.", hent->h_name);
	else {
	  hostlong = htonl(newaddr.sin_addr.s_addr);
	  sprintf(buf, "WHO request from %lu.%lu.%lu.%lu served.",
		  (hostlong & 0xff000000) >> 24,
		  (hostlong & 0x00ff0000) >> 16,
		  (hostlong & 0x0000ff00) >> 8, (hostlong & 0x000000ff) >> 0);
	}
	dlog(buf);

	sprintf(buf, VERSION_STR);
	strcat(buf, "\n\r");

	players = 0;
	gods = 0;
	char_index = 0;

	for (ch = character_list; ch; ch = ch->next) {
	  if (IS_PC(ch)) {
	    if ((INVIS_LEVEL(ch) < 2) && (GetMaxLevel(ch) <= WIZ_MAX_LEVEL) &&
		!IS_AFFECTED(ch, AFF_HIDE) && !IS_AFFECTED(ch, AFF_INVISIBLE)) {
	      if (GetMaxLevel(ch) >= WIZ_MIN_LEVEL)
		gods++;
	      else
		players++;

	      char_index++;

	      if (IS_SET(SHOW_IDLE, whod_mode)) {
		if (!(ch->desc)) {
		  strcat(buf, "linkdead ");
		} else {
		  ttime = GET_IDLE_TIME(ch);
		  thour = ttime / 3600;
		  ttime -= thour * 3600;
		  tmin = ttime / 60;
		  ttime -= tmin * 60;
		  tsec = ttime;
		  if (!thour && !tmin && (tsec <= 15))
		    strcat(buf, " playing ");
		  else
		    sprintf(buf + strlen(buf), "%02ld:%02ld:%02ld ", thour, tmin, tsec);
		}
	      }

	      if (IS_SET(SHOW_LEVEL, whod_mode)) {
		if (GetMaxLevel(ch) >= WIZ_MAX_LEVEL)
		  sprintf(buf + strlen(buf), "[ God ] ");
		else if (GetMaxLevel(ch) == WIZ_MAX_LEVEL - 1)
		  sprintf(buf + strlen(buf), "[Power] ");
		else if (GetMaxLevel(ch) >= WIZ_MIN_LEVEL)
		  sprintf(buf + strlen(buf), "[Whizz] ");
		else
		  sprintf(buf + strlen(buf), "[ %3d ] ", GetMaxLevel(ch));
	      }

	      if (IS_SET(SHOW_TITLE, whod_mode))
		if (GET_PRETITLE(ch))
		  sprintf(buf + strlen(buf), "%s ", GET_PRETITLE(ch));

	      if (IS_SET(SHOW_NAME, whod_mode))
		sprintf(buf + strlen(buf), "%s ", GET_NAME(ch));

	      if (IS_SET(SHOW_TITLE, whod_mode))
		sprintf(buf + strlen(buf), "%s ", GET_TITLE(ch));

/*
 * This is bad for the external whod... it pinpoints people too easily.
 * Make them enter the game to see where people are.
 *
 *          if (IS_SET(SHOW_ROOM, whod_mode)) {
 *            sprintf(buf + strlen(buf), "- %s ",
 *                    real_roomp(ch->in_room)->name);
 *          }
 */

	      if (IS_SET(SHOW_SITE, whod_mode)) {
		if (ch->desc->host != NULL)
		  sprintf(buf + strlen(buf), "(%s)", ch->desc->host);
	      }
	      strcat(buf, "\n\r");
	      WRITE(newdesc, buf);
	      *buf = '\0';
	    }
	  }
	}
	sprintf(buf + strlen(buf), "\n\rVisible Players: %d\tVisible Gods: %d\n\r", players,
		gods);
	ot = Uptime;
	otmstr = asctime(localtime(&ot));
	*(otmstr + strlen(otmstr) - 1) = '\0';
	sprintf(buf + strlen(buf), START_TIME, otmstr);

	ct = time(0);
	tmstr = asctime(localtime(&ct));
	*(tmstr + strlen(tmstr) - 1) = '\0';
	sprintf(buf + strlen(buf), GAME_TIME, tmstr);

	WRITE(newdesc, buf);

	disconnect_time = time(NULL) + WHOD_DELAY_TIME;
	state = WHOD_DELAY;
      } else if (IS_SET(SHOW_OFF, whod_mode)) {
	state = WHOD_CLOSING;
      }
      break;

/*************************************************************************/
    case WHOD_DELAY:
      if (time(NULL) >= disconnect_time)
	state = WHOD_END;
      break;

/****************************************************************/
    case WHOD_END:
      close(newdesc);
      if (IS_SET(whod_mode, SHOW_OFF))
	state = WHOD_CLOSING;
      else
	state = WHOD_OPEN;
      break;

/****************************************************************/
    case WHOD_CLOSING:
      close_whod();
      state = WHOD_CLOSED;
      break;

/****************************************************************/
    case WHOD_CLOSED:
      if (IS_SET(whod_mode, SHOW_ON))
	state = WHOD_OPENING;
      break;

  }
  return;
}

/**** You might want to use this in your help_file.                 ****/
/**** It should be placed in the end of the file, so help on WHO is ****/
/**** availeble too.                                                ****/

/*
 * WHOD
 * 
 * The who daemon is run a seperate port. The following commands exists:
 * 
 * name    : Toggles peoples name on/off the list (useless)
 * title   : Toggles peoples title on/off the list
 * site    : Toggles peoples site names on/off the list
 * on      : Turns the whod on, and thereby opens the port
 * off     : Turns the whod off, and thereby closes the port
 * 
 * NOTE:     The on/off feature is only made to use, if someone starts polling
 * a few times a second or the like, and thereby abusing the net. You
 * might then want to shut down the daemon for 15 minutes or so.
 * #
 */

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
#include <fcntl.h>

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
 *   log_boot("Entering game loop.");
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

void do_whod(struct char_data *ch, const char *arg, int cmd)
{
  char                                    buf[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
  char                                    tmp[MAX_INPUT_LENGTH] = "\0\0\0\0\0\0\0";
  int                                     bit = 0;
  const char                                   *modes[] = {
    "on",
    "off",
    "idle",
    "level",
    "name",
    "title",
    "room",
    "site",
    "room_ingame",
    "\n"
  };

  if (DEBUG)
    log_info("called %s with %s, %s, %d", __PRETTY_FUNCTION__, SAFE_NAME(ch), VNULL(arg), cmd);

  half_chop(arg, buf, tmp);
  if (!*buf) {
    cprintf(ch, "Current WHOD mode:\r\n------------------\r\n");
    sprintbit((long)whod_mode, (const char **)modes, buf);
    cprintf(ch, "%s\r\n", buf);
    return;
  }
  if ((bit = old_search_block(buf, 0, strlen(buf), modes, FALSE)) == -1) {
    cprintf(ch, "That mode does not exist.\r\nAvailable modes are:\r\n");
    *buf = '\0';
    for (bit = 0; *modes[bit] != '\n'; bit++) {
      strcat(buf, modes[bit]);
      strcat(buf, " ");
    }
    cprintf(ch, "%s\r\n", buf);
    return;
  }
  bit--;						       /* Is bit no + 1 */
  if (SHOW_ON == 1 << bit) {
    if (IS_SET(whod_mode, SHOW_ON))
      cprintf(ch, "WHOD already turned on.\r\n");
    else {
      if (IS_SET(whod_mode, SHOW_OFF)) {
	REMOVE_BIT(whod_mode, SHOW_OFF);
	SET_BIT(whod_mode, SHOW_ON);
	cprintf(ch, "WHOD turned on.\r\n");
	log_info("WHOD turned on by %s.", GET_NAME(ch));
      }
    }
  } else {
    if (SHOW_OFF == 1 << bit) {
      if (IS_SET(whod_mode, SHOW_OFF))
	cprintf(ch, "WHOD already turned off.\r\n");
      else {
	if (IS_SET(whod_mode, SHOW_ON)) {
	  REMOVE_BIT(whod_mode, SHOW_ON);
	  SET_BIT(whod_mode, SHOW_OFF);
	  cprintf(ch, "WHOD turned off.\r\n");
	  log_info("WHOD turned off by %s.", GET_NAME(ch));
	}
      }
    } else {
      if (IS_SET(whod_mode, 1 << bit)) {
	cprintf(ch, "%c%s will not be shown on WHOD.\r\n",
		toupper(modes[bit][0]), &modes[bit][1]);
	log_info("%c%s removed from WHOD by %s.",
	    toupper(modes[bit][0]), &modes[bit][1], GET_NAME(ch));
	REMOVE_BIT(whod_mode, 1 << bit);
	return;
      } else {
	cprintf(ch, "%c%s will now be shown on WHOD.\r\n",
		toupper(modes[bit][0]), &modes[bit][1]);
	log_info("%c%s added to WHOD by %s.", toupper(modes[bit][0]), &modes[bit][1], GET_NAME(ch));
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
    log_info("called %s with %d", __PRETTY_FUNCTION__, port);

  whod_port = port + 1;
  log_boot("WHOD port opened.");
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
    log_info("called %s with no arguments", __PRETTY_FUNCTION__);

  if (state != WHOD_CLOSED) {
    state = WHOD_CLOSED;
    close(s);
    log_boot("WHOD port closed.");
  }
}

/*
 * Function   : whod_text
 * Parameters : --
 * Returns    : a static string
 * Description: Generates plain-text output for telnet.
 */
char * whod_text(void)
{
  /* This is 32K, in case we have 250 players, yeah right! */
  static char                             buf[32768] = "\0\0\0\0\0\0\0";
  int                                     players = 0;
  int                                     gods = 0;
  int                                     char_index = 0;
  struct char_data                       *ch = NULL;
  long                                    ttime = 0L;
  long                                    thour = 0L;
  long                                    tmin = 0L;
  long                                    tsec = 0L;
  time_t                                  now;
  char                                    uptimebuf[100];
  char                                    nowtimebuf[100];

  now = time( (time_t*) 0 );
  strftime( nowtimebuf, sizeof(nowtimebuf), RFC1123FMT, localtime( &now ) );
  strftime( uptimebuf, sizeof(uptimebuf), RFC1123FMT, localtime( (time_t *) &Uptime ) );

  sprintf(buf, VERSION_STR);
  strcat(buf, "\r\n");

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
   */
        if (IS_SET(SHOW_ROOM, whod_mode)) {
          sprintf(buf + strlen(buf), "- %s ", real_roomp(ch->in_room)->name);
        }
  
        if (IS_SET(SHOW_SITE, whod_mode)) {
          if (ch->desc->host != NULL)
            sprintf(buf + strlen(buf), "(%s)", ch->desc->host);
          else if (ch->desc->ip != NULL)
            sprintf(buf + strlen(buf), "(%s)", ch->desc->ip);
        }
        strcat(buf, "\r\n");
        /* WRITE(newdesc, buf); */
        /* *buf = '\0'; */
      }
    }
  }
  sprintf(buf + strlen(buf), "\r\nVisible Players: %d\tVisible Gods: %d\r\n", players, gods);
  sprintf(buf + strlen(buf), "Wiley start time was: %s\r\n", uptimebuf);
  sprintf(buf + strlen(buf), "Quixadhal's time is:  %s\r\n", nowtimebuf);

  return buf;
}

/*
 * Function   : whod_html
 * Parameters : --
 * Returns    : a static string
 * Description: Generates HTML output for the web.
 */
char * whod_html(void)
{
  /* This is 32K, in case we have 250 players, yeah right! */
  char                                    buf[32768] = "\0\0\0\0\0\0\0";
  int                                     players = 0;
  int                                     gods = 0;
  int                                     char_index = 0;
  struct char_data                       *ch = NULL;
  long                                    ttime = 0L;
  long                                    thour = 0L;
  long                                    tmin = 0L;
  long                                    tsec = 0L;
  time_t                                  now;
  char                                    timebuf[100];
  char                                    uptimebuf[100];
  char                                    nowtimebuf[100];
  static char                             headers[40960];
  int                                     table_width = 0;
  struct timeval                          now_bits;
  struct timeval                          later_bits;

  now = time( (time_t*) 0 );
  gettimeofday(&now_bits, NULL);
  strftime( timebuf, sizeof(timebuf), RFC1123FMT, gmtime( &now ) );
  strftime( nowtimebuf, sizeof(nowtimebuf), RFC1123FMT, localtime( &now ) );
  strftime( uptimebuf, sizeof(uptimebuf), RFC1123FMT, localtime( (time_t *) &Uptime ) );

  sprintf(headers, "HTTP/1.1 200 OK\r\n");
  sprintf(headers + strlen(headers), "Server: %s\r\n", MUDNAME);
  sprintf(headers + strlen(headers), "Date: %s\r\n", timebuf );
  sprintf(headers + strlen(headers), "Content-Type: %s\r\n", "text/html; charset=iso-8859-1" );
  
  sprintf(buf, "<html>\r\n");
  sprintf(buf + strlen(buf), "<head><title>Welcome to %s!</title></head>\r\n", MUDNAME);
  sprintf(buf + strlen(buf), "<body>\r\n");
  sprintf(buf + strlen(buf), "<div align=\"center\"><h3><a href=\"telnet://wiley.shadowlord.org:3000/\">%s</a></h3></div>\r\n", VERSION_STR);

  players = 0;
  gods = 0;
  char_index = 0;
  
  if (IS_SET(SHOW_IDLE, whod_mode))	table_width += 100;
  if (IS_SET(SHOW_LEVEL, whod_mode))	table_width += 100;
  table_width += 400;
  if (IS_SET(SHOW_ROOM, whod_mode))	table_width += 100;
  if (IS_SET(SHOW_SITE, whod_mode))	table_width += 200;

  sprintf(buf + strlen(buf), "<div align=\"center\">\r\n");
  sprintf(buf + strlen(buf), "<table border=\"0\" cellspacing=\"0\" cellpadding=\"1\" width=\"%d\">\r\n", table_width);
  sprintf(buf + strlen(buf), "<tr bgcolor=\"#E7E7E7\">\r\n");
  if (IS_SET(SHOW_IDLE, whod_mode))
    sprintf(buf + strlen(buf), "<th align=\"center\" width=\"100\">%s</th>\r\n", "Idle");

  if (IS_SET(SHOW_LEVEL, whod_mode))
    sprintf(buf + strlen(buf), "<th align=\"center\" width=\"100\">%s</th>\r\n", "Level");

  sprintf(buf + strlen(buf), "<th align=\"left\" width=\"400\">%s</th>\r\n", "Name");

  if (IS_SET(SHOW_ROOM, whod_mode))
    sprintf(buf + strlen(buf), "<th align=\"center\" width=\"100\">%s</th>\r\n", "Room");

  if (IS_SET(SHOW_SITE, whod_mode))
    sprintf(buf + strlen(buf), "<th align=\"left\" width=\"200\">%s</th>\r\n", "Site");
  sprintf(buf + strlen(buf), "</tr>\r\n");

  for (ch = character_list; ch; ch = ch->next) {
    if (IS_PC(ch)) {
      if ((INVIS_LEVEL(ch) < 2) && (GetMaxLevel(ch) <= WIZ_MAX_LEVEL) &&
          !IS_AFFECTED(ch, AFF_HIDE) && !IS_AFFECTED(ch, AFF_INVISIBLE)) {
        if (GetMaxLevel(ch) >= WIZ_MIN_LEVEL)
          gods++;
        else
          players++;
  
        char_index++;
  
        sprintf(buf + strlen(buf), "<tr bgcolor=\"%s\">\r\n", char_index%2 ? "#E7FFE7": "#FFFFE7");
        if (IS_SET(SHOW_IDLE, whod_mode)) {
          if (!(ch->desc)) {
            sprintf(buf + strlen(buf), "<td align=\"center\">%s</td>\r\n", "linkdead");
          } else {
            ttime = GET_IDLE_TIME(ch);
            thour = ttime / 3600;
            ttime -= thour * 3600;
            tmin = ttime / 60;
            ttime -= tmin * 60;
            tsec = ttime;
            if (!thour && !tmin && (tsec <= 15))
              sprintf(buf + strlen(buf), "<td align=\"center\">%s</td>\r\n", "playing");
            else
              sprintf(buf + strlen(buf), "<td align=\"center\">%02ld:%02ld:%02ld</td>\r\n", thour, tmin, tsec);
          }
        }
  
        if (IS_SET(SHOW_LEVEL, whod_mode)) {
          if (GetMaxLevel(ch) >= WIZ_MAX_LEVEL)
            sprintf(buf + strlen(buf), "<td align=\"center\">%s</td>\r\n", "God");
          else if (GetMaxLevel(ch) == WIZ_MAX_LEVEL - 1)
            sprintf(buf + strlen(buf), "<td align=\"center\">%s</td>\r\n", "Power");
          else if (GetMaxLevel(ch) >= WIZ_MIN_LEVEL)
            sprintf(buf + strlen(buf), "<td align=\"center\">%s</td>\r\n", "Whizz");
          else
            sprintf(buf + strlen(buf), "<td align=\"center\">%3d</td>\r\n", GetMaxLevel(ch));
        }
  
        sprintf(buf + strlen(buf), "<td align=\"left\">");
        if (IS_SET(SHOW_TITLE, whod_mode))
          if (GET_PRETITLE(ch))
            sprintf(buf + strlen(buf), "%s ", GET_PRETITLE(ch));
  
        if (IS_SET(SHOW_NAME, whod_mode))
          sprintf(buf + strlen(buf), "%s", GET_NAME(ch));
  
        if (IS_SET(SHOW_TITLE, whod_mode))
          sprintf(buf + strlen(buf), " %s", GET_TITLE(ch));
        sprintf(buf + strlen(buf), "</td>\r\n");
  
  /*
   * This is bad for the external whod... it pinpoints people too easily.
   * Make them enter the game to see where people are.
   */
        if (IS_SET(SHOW_ROOM, whod_mode)) {
          sprintf(buf + strlen(buf), "<td align=\"center\">%s</td>\r\n", real_roomp(ch->in_room)->name);
        }
  
        if (IS_SET(SHOW_SITE, whod_mode)) {
          if (ch->desc->host != NULL)
            sprintf(buf + strlen(buf), "<td align=\"left\">%s</td>\r\n", ch->desc->host);
          else if (ch->desc->ip != NULL)
            sprintf(buf + strlen(buf), "<td align=\"left\">%s</td>\r\n", ch->desc->ip);
        }
        sprintf(buf + strlen(buf), "</tr>\r\n");
      }
    }
  }
  sprintf(buf + strlen(buf), "</table>\r\n");

  sprintf(buf + strlen(buf), "<br />\r\n");
  sprintf(buf + strlen(buf), "<table border=\"0\" cellspacing=\"0\" cellpadding=\"1\" width=\"%d\">\r\n", table_width);
  sprintf(buf + strlen(buf), "<tr>\r\n");
  sprintf(buf + strlen(buf), "<td align=\"right\" width=\"%d\">%s</td>\r\n", table_width/2, "Visible&nbsp;Players:&nbsp;");
  sprintf(buf + strlen(buf), "<td align=\"left\" width=\"%d\">%d</td>\r\n", table_width/2, players);
  sprintf(buf + strlen(buf), "</tr>\r\n");
  sprintf(buf + strlen(buf), "<tr>\r\n");
  sprintf(buf + strlen(buf), "<td align=\"right\" width=\"%d\">%s</td>\r\n", table_width/2, "Visible&nbsp;Gods:&nbsp;");
  sprintf(buf + strlen(buf), "<td align=\"left\" width=\"%d\">%d</td>\r\n", table_width/2, gods);
  sprintf(buf + strlen(buf), "</tr>\r\n");
  sprintf(buf + strlen(buf), "<tr>\r\n");
  sprintf(buf + strlen(buf), "<td align=\"right\" width=\"%d\">%s</td>\r\n", table_width/2, "Wiley&nbsp;start&nbsp;time&nbsp;was:&nbsp;");
  sprintf(buf + strlen(buf), "<td align=\"left\" width=\"%d\">%s</td>\r\n", table_width/2, uptimebuf);
  sprintf(buf + strlen(buf), "</tr>\r\n");
  sprintf(buf + strlen(buf), "<tr>\r\n");
  sprintf(buf + strlen(buf), "<td align=\"right\" width=\"%d\">%s</td>\r\n", table_width/2, "Quixadhal's&nbsp;time&nbsp;is:&nbsp;");
  sprintf(buf + strlen(buf), "<td align=\"left\" width=\"%d\">%s</td>\r\n", table_width/2, nowtimebuf);
  sprintf(buf + strlen(buf), "</tr>\r\n");
  sprintf(buf + strlen(buf), "</table>\r\n");

  sprintf(buf + strlen(buf), "</div>\r\n");

  gettimeofday(&later_bits, NULL);
  sprintf(buf + strlen(buf),
  "<div align=\"right\"><font size=\"-1\" color=\"#DDDDDD\">Page took %01d.%06d seconds to render.</font></div>\r\n",
  (int)(later_bits.tv_sec - now_bits.tv_sec), (int)(later_bits.tv_usec - now_bits.tv_usec));

  sprintf(buf + strlen(buf), "</body>\r\n");
  sprintf(buf + strlen(buf), "</html>\r\n");

  sprintf(headers + strlen(headers), "Content-Length: %d\r\n", (int)strlen(buf));
  sprintf(headers + strlen(headers), "Connection: %s\r\n", "close");
  sprintf(headers + strlen(headers), "\r\n");
  strcat(headers, buf);

  return headers;
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
  unsigned int                            size = 0;
  fd_set                                  in;
  unsigned long                           hostlong = 0L;
  struct timeval                          timeout;
  struct sockaddr_in                      newaddr;
  struct hostent                         *hent = NULL;

  /*
   * extern long Uptime; 
   */

  static int                              newdesc = 0;

  if (DEBUG > 2)
    log_info("called %s with no arguments", __PRETTY_FUNCTION__);

  switch (state) {
/****************************************************************/
    case WHOD_OPENING:
      s = init_socket(whod_port);
      log_boot("WHOD port opened.");
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
	  log_error("WHOD - Accept");
	  return;
	}
	if ((hent =
	     gethostbyaddr((char *)&newaddr.sin_addr, sizeof(newaddr.sin_addr), AF_INET)))
	  log_info("WHO request from %s served.", hent->h_name);
	else {
	  hostlong = htonl(newaddr.sin_addr.s_addr);
	  log_info("WHO request from %lu.%lu.%lu.%lu served.",
		  (hostlong & 0xff000000) >> 24,
		  (hostlong & 0x00ff0000) >> 16,
		  (hostlong & 0x0000ff00) >> 8, (hostlong & 0x000000ff) >> 0);
	}

        /* Do we really need to sink input here before sending output? */
/*        if (fcntl(s, F_SETFL, O_NDELAY) != -1) {
          char junk[MAX_INPUT_LENGTH];
          while(read(newdesc, junk, MAX_INPUT_LENGTH) > 0);
        }
*/
	WRITE(newdesc, whod_html());

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

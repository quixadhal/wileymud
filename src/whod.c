/************************************************************************\
*                                                                        *
*      Opens a port (the port # above the port # the game itself is	 *
*      being run). On this port people can connect to see who's on.	 *
*	 The player wont have to enter the game to see if it's worth	 *
*      playing at the time, thus saving money.				 *
*									 *
*      Change the following #define-statements to adjust the             *
*      WHOD to your server.                                              */

#include "version.h"

/*
 * *** The following statement sets the name of the MUD in WHOD      *** 
 */
#define MUDNAME "DeadMUD"

/*
 * *** The following statement indicates the WHOD default mode 
 * Modes can be:
 * SHOW_NAME
 * SHOW_LEVEL
 * SHOW_TITLE
 * SHOW_CLASS
 * SHOW_ON
 */

#define DEFAULT_MODE ( SHOW_NAME | SHOW_TITLE | SHOW_ON )

/*
 * *** The following statement tells if a character is INVIS     *** 
 */
#define INVIS_LEVEL(ch) ((ch)->invis_level)

/*
 * *** Specify the lowest & highest wizard level                 *** 
 */
#define WIZ_MIN_LEVEL 50
#define WIZ_MAX_LEVEL 59

/*
 * *** The following statement will send a message to the system log *** 
 */
#define LOG(msg) log(msg)

/*
 * *** Time to wait for disconnection (in seconds)                   *** 
 */
#define WHOD_DELAY_TIME 3

/*
 * *** Show linkdead people on the list as well ?  1=yes, 0=no       *** 
 */
#define DISPLAY_LINKDEAD 1

/*
 * In function run_the_game(int port):                           *
 * *      ..                                                             *
 * *      init_whod(port);                                                       *
 * *      log("Entering game loop.");                                    *
 * *      game_loop(s);                                                  *
 * *      close_sockets(s);                                              *
 * *      close_whod();                                                  *
 * *      ..                                                             *
 * *                                                                     *
 * *      In function game_loop ():                                         *
 * *      ..                                                                *
 * *      sigsetmask(mask);                                                 *
 * *      whod_loop();                                                      *
 * *      if (select(maxdesc + 1, &input_set, &output_set,                  *
 * *                 &exc_set, &null_time) < 0)                             *
 * *      ..                                                                *
 * *                                                                        *
 * *********************************************************************** 
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <signal.h>
#include <stdio.h>

#include "structs.h"
#include "utils.h"

#define WHOD_OPENING 1
#define WHOD_OPEN    2
#define WHOD_DELAY   3
#define WHOD_END     4
#define WHOD_CLOSED  5
#define WHOD_CLOSING 6

#define SHOW_NAME	1<<0
#define SHOW_TITLE	1<<1
#define SHOW_SITE	1<<2
#define SHOW_ON		1<<3
#define SHOW_OFF	1<<4

#define WRITE(d,msg) if((write((d),(msg),strlen(msg)))<0){\
                            perror("whod.c - write");}

/*
 * *** External functions *** 
 */
int                              init_socket(int port);

/*
 * *** External variables *** 
 */
extern struct char_data         *character_list;

/*
 * *** Internal variables *** 
 */
static long                      disconnect_time;
static int                       s;
static int                       whod_mode = DEFAULT_MODE;
static int                       state;
static int                       whod_port;

/*
 * ------ WHOD interface for use inside of the game ------        
 */

/*
 * Function   : do_whod
 * * Parameters : doer, argument string, number of WHOD command (not used)
 * * Returns    : --
 * * Description: MUD command to set the mode of the WHOD-connection according
 * *              to the command string.                                  
 */

void 
do_whod(struct char_data *ch, char *arg, int cmd)
{
  char                             buf[256];
  char                             tmp[MAX_INPUT_LENGTH];
  int                              bit;

  static char                     *modes[] =
  {
    "name",
    "title",
    "site",
    "on",
    "off",
    "\n"
  };

  half_chop(arg, buf, tmp);
  if (!*buf) {
    send_to_char("Current WHOD mode:\n\r------------------\n\r", ch);
    sprintbit((long)whod_mode, modes, buf);
    strcat(buf, "\n\r");
    send_to_char(buf, ch);
    return;
  }
  if ((bit = old_search_block(buf, 0, strlen(buf), modes, FALSE)) == -1) {
    send_to_char("That mode does not exist.\n\rAvailable modes are:\n\r", ch);
    *buf = '\0';
    for (bit = 0; *modes[bit] != '\n'; bit++) {
      strcat(buf, modes[bit]);
      strcat(buf, " ");
    }
    strcat(buf, "\n\r");
    send_to_char(buf, ch);
    return;
  }
  bit--;			/*
				 * Is bit no + 1 
				 */
  if (SHOW_ON == 1 << bit) {
    if (IS_SET(whod_mode, SHOW_ON))
      send_to_char("WHOD already turned on.\n\r", ch);
    else {
      if (IS_SET(whod_mode, SHOW_OFF)) {
	REMOVE_BIT(whod_mode, SHOW_OFF);
	SET_BIT(whod_mode, SHOW_ON);
	send_to_char("WHOD turned on.\n\r", ch);
	sprintf(buf, "WHOD turned on by %s.", GET_NAME(ch));
	LOG(buf);
      }
    }
  } else {
    if (SHOW_OFF == 1 << bit) {
      if (IS_SET(whod_mode, SHOW_OFF))
	send_to_char("WHOD already turned off.\n\r", ch);
      else {
	if (IS_SET(whod_mode, SHOW_ON)) {
	  REMOVE_BIT(whod_mode, SHOW_ON);
	  SET_BIT(whod_mode, SHOW_OFF);
	  send_to_char("WHOD turned off.\n\r", ch);
	  sprintf(buf, "WHOD turned off by %s.", GET_NAME(ch));
	  LOG(buf);
	}
      }
    } else {
      if (IS_SET(whod_mode, 1 << bit)) {
	sprintf(buf, "%s will not be shown on WHOD.\n\r", modes[bit]);
	CAP(buf);
	send_to_char(buf, ch);
	sprintf(buf, "%s removed from WHOD by %s.", modes[bit], GET_NAME(ch));
	CAP(buf);
	LOG(buf);
	REMOVE_BIT(whod_mode, 1 << bit);
	return;
      } else {
	sprintf(buf, "%s will now be shown on WHOD.\n\r", modes[bit]);
	CAP(buf);
	send_to_char(buf, ch);
	sprintf(buf, "%s added to WHOD by %s.", modes[bit], GET_NAME(ch));
	CAP(buf);
	LOG(buf);
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
 * * Parameters : Port #-1 the daemon should be run at
 * * Returns    : --
 * * Description: Opens the WHOD port and sets the state of WHO-daemon to OPEN
 */

void 
init_whod(int port)
{
  whod_port = port + 1;
  LOG("WHOD port opened.");
  s = init_socket(whod_port);
  state = WHOD_OPEN;
}

/*
 * Function   : close_whod
 * * Parameters : --
 * * Returns    : --
 * * Description: Closes the WHOD port and sets the state of WHO-daemon to
 * *              CLOSED.                                                 
 */

void 
close_whod(void)
{
  if (state != WHOD_CLOSED) {
    state = WHOD_CLOSED;
    close(s);
    LOG("WHOD port closed.");
  }
}

/*
 * Function   : whod_loop
 * * Parameters : --
 * * Returns    : --
 * * Description: Serves incoming WHO calls.                             
 */

void 
whod_loop(void)
{
  int                              nfound,
                                   size,
                                   players = 0,
                                   gods = 0,
                                   index,
                                   i;
  fd_set                           in;
  u_long                           hostlong;
  struct timeval                   timeout;
  struct sockaddr_in               newaddr;
  char                             buf[MAX_STRING_LENGTH],
                                   tmp[80];
  struct char_data                *ch;
  struct hostent                  *hent;
  char                             time_buf[100];
  long                             ct,
                                   ot;
  char                            *tmstr,
                                  *otmstr;
  extern long                      Uptime;

  static int                       newdesc;

  switch (state) {
/****************************************************************/
  case WHOD_OPENING:
    s = init_socket(whod_port);
    LOG("WHOD port opened.");
    state = WHOD_OPEN;
    break;

/****************************************************************/
  case WHOD_OPEN:

    timeout.tv_sec = 0;
    timeout.tv_usec = 100;

    FD_ZERO(&in);
    FD_SET(s, &in);

#ifdef sun3
    nfound = select(s + 1, (int *)&in, (int *)0, (int *)0, &timeout);
#else
    nfound = select(s + 1, &in, (fd_set *) 0, (fd_set *) 0, &timeout);
#endif

    if (FD_ISSET(s, &in)) {
      size = sizeof(newaddr);
      getsockname(s, &newaddr, &size);

      if ((newdesc = accept(s, &newaddr, &size)) < 0) {
	perror("WHOD - Accept");
	return;
      }
      if (hent = gethostbyaddr((char *)&newaddr.sin_addr, sizeof(newaddr.sin_addr), AF_INET))
	sprintf(buf, "WHO request from %s served.", hent->h_name);
      else {
	hostlong = htonl(newaddr.sin_addr.s_addr);
	sprintf(buf, "WHO request from %d.%d.%d.%d served.",
		(hostlong & 0xff000000) >> 24,
		(hostlong & 0x00ff0000) >> 16,
		(hostlong & 0x0000ff00) >> 8,
		(hostlong & 0x000000ff) >> 0);
      }
      LOG(buf);

      sprintf(buf, VERSION_STR);
      sprintf(buf + strlen(buf), "\n\r***  Active players on %s:\n\r\n\r", MUDNAME);

      players = 0;
      gods = 0;
      index = 0;

      for (ch = character_list; ch; ch = ch->next) {
	if (IS_PC(ch)) {
	  if ((INVIS_LEVEL(ch) < 2) && (GetMaxLevel(ch) <= WIZ_MAX_LEVEL)) {
	    if (GetMaxLevel(ch) >= WIZ_MIN_LEVEL)
	      gods++;
	    else
	      players++;

	    index++;

	    if (IS_SET(SHOW_NAME, whod_mode))
	      sprintf(buf + strlen(buf), "[%2d] %s ", index, GET_NAME(ch));

	    if (IS_SET(SHOW_TITLE, whod_mode))
	      sprintf(buf + strlen(buf), "%s ", GET_TITLE(ch));

	    if (IS_SET(SHOW_SITE, whod_mode)) {
	      if (ch->desc->host != NULL)
		sprintf(tmp, "[%s] ", ch->desc->host);
	      else
		sprintf(tmp, "[ ** Unknown ** ] ");
	      strcat(buf, tmp);
	    }
	    strcat(buf, "\n\r");
	    WRITE(newdesc, buf);
	    *buf = '\0';
	  }
	}
      }
      sprintf(buf + strlen(buf), "\n\r\n\rPlayers: %d\tGods: %d\n\r", players, gods);
      ot = Uptime;
      otmstr = asctime(localtime(&ot));
      *(otmstr + strlen(otmstr) - 1) = '\0';
      sprintf(buf + strlen(buf), "Dead start time was: %s\n\r", otmstr);

      ct = time(0);
      tmstr = asctime(localtime(&ct));
      *(tmstr + strlen(tmstr) - 1) = '\0';
      sprintf(buf + strlen(buf), "Quixadhal's time is: %s\n\r", tmstr);

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

/*
 * *** You might want to use this in your help_file.                 *** 
 */
/*
 * *** It should be placed in the end of the file, so help on WHO is *** 
 */
/*
 * ***  availeble too.                                               *** 
 */
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

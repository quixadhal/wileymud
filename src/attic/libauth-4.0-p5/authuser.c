/*
 ** authuser.c
 **
 ** 2/2/93: Added auth_setreadtimeout() call.
 ** 7/21/92: Fixed SIGPIPE bug in auth_tcpuser3(). <pen@lysator.liu.se>
 ** 2/9/92: authuser 4.0. Public domain.
 ** 2/9/92: added bunches of zeroing just in case.
 ** 2/9/92: added auth_tcpuser3. uses bsd 4.3 select interface.
 ** 2/9/92: added auth_tcpsock, auth_sockuser.
 ** 2/9/92: added auth_fd2, auth_tcpuser2, simplified some of the code.
 ** 12/27/91: fixed up usercmp to deal with restricted tolower XXX
 ** 5/6/91 DJB baseline authuser 3.1. Public domain.
 */

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/file.h>
#include <fcntl.h> /*XXX*/
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
extern int errno;

#define _AUTHUSER_C
#include "authuser.h"

#ifndef FNDELAY
#  define FNDELAY O_NDELAY
#endif

unsigned short auth_tcpport = 113;
int            auth_rtimeout = 60;	/* 1 minute */

#define SIZ 500 /* various buffers */

static void clearsa(struct sockaddr_in *sa) {
    register char *x;
    for (x = (char *) sa;x < sizeof(*sa) + (char *) sa;++x)
	*x = 0;
}

static int usercmp( register char *u, register char *v) {
    register char uc = '\0';
    register char vc = '\0';
    register char ucvc = '\0';
    /* is it correct to consider Foo and fOo the same user? yes */
    /* but the function of this routine may change later */
    while ((uc = *u) && (vc = *v))
    {
	ucvc = (isupper(uc) ? tolower(uc) : uc) - (isupper(vc) ? tolower(vc) : vc);
	if (ucvc)
	    return ucvc;
	else
	    ++u,++v;
    }
    return uc || vc;
}

static char authline[SIZ];

char *auth_xline(register char *user, register int fd, register unsigned long *in)
{
    unsigned short local;
    unsigned short remote;
    register char *ruser;
    
    if (auth_fd(fd,in,&local,&remote) == -1)
	return 0;
    ruser = auth_tcpuser(*in,local,remote);
    if (!ruser)
	return 0;
    if (!user)
	user = ruser; /* forces X-Auth-User */
    sprintf(authline,
	    (usercmp(ruser,user) ? "X-Forgery-By: %s" : "X-Auth-User: %s"),
	    ruser);
    return authline;
}

int auth_fd( register int fd, register unsigned long *in,
    register unsigned short *local, register unsigned short *remote)
{
    unsigned long inlocal;
    return auth_fd2(fd,&inlocal,in,local,remote);
}

int auth_fd2( register int fd, register unsigned long *inlocal,
    register unsigned long *inremote, register unsigned short *local,
    register unsigned short *remote)
{
    struct sockaddr_in sa;
    int dummy;
    
    dummy = sizeof(sa);
    if (getsockname(fd,(struct sockaddr *)&sa,&dummy) == -1)
	return -1;
    if (sa.sin_family != AF_INET)
    {
	errno = EAFNOSUPPORT;
	return -1;
    }
    *local = ntohs(sa.sin_port);
    *inlocal = sa.sin_addr.s_addr;
    dummy = sizeof(sa);
    if (getpeername(fd,(struct sockaddr *)&sa,&dummy) == -1)
	return -1;
    *remote = ntohs(sa.sin_port);
    *inremote = sa.sin_addr.s_addr;
    return 0;
}

static char ruser[SIZ];
static char realbuf[SIZ];
static char *buf;

char *auth_tcpuser( register unsigned long in, register unsigned short local,
    register unsigned short remote)
{
    return auth_tcpuser2(0,in,local,remote);
}

#define CLORETS(e) { saveerrno = errno; close(s); errno = saveerrno; return e; }

int auth_tcpsock( register unsigned long inlocal,
    register unsigned long inremote)
{
    struct sockaddr_in sa;
    register int s;
    register int fl;
    register int saveerrno;
    
    if ((s = socket(AF_INET,SOCK_STREAM,0)) == -1)
	return -1;
    if (inlocal)
    {
	clearsa(&sa);
	sa.sin_family = AF_INET;
	sa.sin_port = 0;
	sa.sin_addr.s_addr = inlocal;
	if (bind(s,(struct sockaddr *)&sa,sizeof(sa)) == -1)
	    CLORETS(-1)
	    }
    if ((fl = fcntl(s,F_GETFL,0)) == -1)
	CLORETS(-1);
    if (fcntl(s,F_SETFL,FNDELAY | fl) == -1)
	CLORETS(-1);
    clearsa(&sa);
    sa.sin_family = AF_INET;
    sa.sin_port = htons(auth_tcpport);
    sa.sin_addr.s_addr = inremote;
    if (connect(s,(struct sockaddr *)&sa,sizeof(sa)) == -1)
	if (errno != EINPROGRESS)
	    CLORETS(-1)
		return s;
}

char *auth_tcpuser2( register unsigned long inlocal,
    register unsigned long inremote,
    register unsigned short local,
    register unsigned short remote)
{
    register int s;
    
    s = auth_tcpsock(inlocal,inremote);
    if (s == -1)
	return 0;
    return auth_sockuser(s,local,remote);
}

char *auth_tcpuser3( register unsigned long inlocal,
    register unsigned long inremote,
    register unsigned short local,
    register unsigned short remote,
    register int ctimeout)
{
    return auth_tcpuser4(inlocal,
			 inremote,
			 local,
			 remote,
			 ctimeout,
			 auth_rtimeout);
}


char *auth_tcpuser4( register unsigned long inlocal,
    register unsigned long inremote,
    register unsigned short local,
    register unsigned short remote,
    register int ctimeout,
    register int rtimeout)
{
    register int s;
    struct timeval ctv;
    fd_set wfds;
    register int r;
    register int saveerrno;
    void *old_sig;
    char *retval;
    
    
    old_sig = signal(SIGPIPE, SIG_IGN);
    
    s = auth_tcpsock(inlocal,inremote);
    if (s == -1)
    {
	signal(SIGPIPE, old_sig);
	return 0;
    }
    ctv.tv_sec = ctimeout;
    ctv.tv_usec = 0;
    FD_ZERO(&wfds);
    FD_SET(s,&wfds);
    r = select(s + 1,(fd_set *) 0,&wfds,(fd_set *) 0,&ctv);
    /* XXX: how to handle EINTR? */
    if (r == -1)
    {
	signal(SIGPIPE, old_sig);
	CLORETS(0);
    }
    if (!FD_ISSET(s,&wfds))
    {
	close(s);
	errno = ETIMEDOUT;
	signal(SIGPIPE, old_sig);
	return 0;
    }
    retval = auth_sockuser2(s,local,remote,rtimeout);
    signal(SIGPIPE, old_sig);
    
    return retval;
}

char *auth_sockuser( register int s,
    register unsigned short local,
    register unsigned short remote)
{
    return auth_sockuser2(s, local, remote, auth_rtimeout);
}

char *auth_sockuser2( register int s,
    register unsigned short local,
    register unsigned short remote,
    int rtimeout)
{
    register int buflen;
    register int w;
    register int saveerrno;
    char ch;
    unsigned short rlocal;
    unsigned short rremote;
    register int fl;
    fd_set wfds;
    void *old_sig;
    struct timeval rtv;
    
    old_sig = signal(SIGPIPE, SIG_IGN);
    
    FD_ZERO(&wfds);
    FD_SET(s,&wfds);
    
    select(s + 1,
	   (fd_set *) 0,
	   &wfds,(fd_set *) 0,
	   (struct timeval *) 0);
    
    /* now s is writable */
    if ((fl = fcntl(s,F_GETFL,0)) == -1)
    {
	signal(SIGPIPE, old_sig);
	CLORETS(0);
    }
    if (fcntl(s,F_SETFL,~FNDELAY & fl) == -1)
    {
	signal(SIGPIPE, old_sig);
	CLORETS(0);
    }
    buf = realbuf;
    sprintf(buf,"%u , %u\r\n",(unsigned int) remote,(unsigned int) local);
    /* note the reversed order---the example in RFC 931 is misleading */
    buflen = strlen(buf);
    while ((w = write(s,buf,buflen)) < buflen)
	if (w == -1) /* should we worry about 0 as well? */
	{
	    signal(SIGPIPE, old_sig);
	    CLORETS(0);
	}
	else
	{
	    buf += w;
	    buflen -= w;
	}
    buf = realbuf;
    
    do
    {
	fd_set rd_fds;
	
	rtv.tv_sec = rtimeout;
	rtv.tv_usec = 0;
    
	FD_ZERO(&rd_fds);
	FD_SET(s, &rd_fds);
	if (select(s+1,
		   &rd_fds,
		   (fd_set *) 0,
		   (fd_set *) 0,
		   &rtv) == 0)
	{
	    w = -1;
	    goto END;
	}
	
	if ((w = read(s,&ch,1)) == 1)
	{
	    *buf = ch;
	    if ((ch != ' ') && (ch != '\t') && (ch != '\r'))
		++buf;
	    if ((buf - realbuf == sizeof(realbuf) - 1) || (ch == '\n'))
		break;
	}
    } while (w == 1);
    
    END: 
    signal(SIGPIPE, old_sig);
    if (w == -1)
	CLORETS(0)
	    *buf = 0;
    
    if (sscanf(realbuf, "%hd,%hd: USERID :%*[^:]:%s",
	       &rremote, &rlocal, ruser) < 3)
    {
	close(s);
	errno = EIO;
	/* makes sense, right? well, not when USERID failed to match ERROR */
	/* but there's no good error to return in that case */
	return 0;
    }
    if ((remote != rremote) || (local != rlocal))
    {
	close(s);
	errno = EIO;
	return 0;
    }
    /* we're not going to do any backslash processing */
    close(s);
    return ruser;
}



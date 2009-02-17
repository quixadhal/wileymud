#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

#define    QLEN           5

int                                     LISTEN_ON = 23;
int                                     JUMP_TO = 3000;
const char                             *JUMP_ADDR = "127.0.0.1";

char                                    sbuf[2048],
                                        cbuf[2048];
void                                    telcli(int source);
void                                    communicate(int sfd, int cfd);

int main(int argc, const char **argv)
{
  int                                     srv_fd,
                                          rem_fd;
  unsigned int                            rem_len;	       /* , opt = 1; */
  struct sockaddr_in                      rem_addr,
                                          srv_addr;

  if (argc != 4) {
    fprintf(stderr, "Usage:  bounce <source port #> <target IP address> <target port #>\n");
    exit(-1);
  }
  LISTEN_ON = atoi(argv[1]);
  JUMP_ADDR = argv[2];
  JUMP_TO = atoi(argv[3]);

  bzero((char *)&rem_addr, sizeof(rem_addr));
  bzero((char *)&srv_addr, sizeof(srv_addr));
  srv_addr.sin_family = AF_INET;
  srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  srv_addr.sin_port = htons(LISTEN_ON);
  srv_fd = socket(PF_INET, SOCK_STREAM, 0);
  if (bind(srv_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr)) == -1) {
    perror("bind");
    exit(-1);
  }
  listen(srv_fd, QLEN);
  close(0);
  close(1);
  close(2);
#ifdef TIOCNOTTY
  if ((rem_fd = open("/dev/tty", O_RDWR)) >= 0) {
    ioctl(rem_fd, TIOCNOTTY, (char *)0);
    close(rem_fd);
  }
#endif
  if (fork())
    exit(0);
  while (1) {
    rem_len = sizeof(rem_addr);
    rem_fd = accept(srv_fd, (struct sockaddr *)&rem_addr, &rem_len);
    if (rem_fd < 0) {
      if (errno == EINTR)
	continue;
      exit(-1);
    }
    switch (fork()) {
      case 0:						       /* child process */
	close(srv_fd);					       /* close original socket */
	telcli(rem_fd);					       /* process the request */
	close(rem_fd);
	exit(0);
	break;
      default:
	close(rem_fd);					       /* parent process */
	if (fork())
	  exit(0);					       /* let init worry about children */
	break;
      case -1:
	fprintf(stderr, "\n\rfork: %s\n\r", strerror(errno));
	break;
    }
  }
}

void telcli(int source)
{
  int                                     dest;

  /*
   * int found; 
   */
  struct sockaddr_in                      sa;

  /*
   * struct hostent *hp; 
   */
  /*
   * struct servent *sp; 
   */
  /*
   * char string[100]; 
   */

  sa.sin_addr.s_addr = inet_addr(JUMP_ADDR);
  sa.sin_family = AF_INET;
  sa.sin_port = htons((unsigned)JUMP_TO);
  if ((dest = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("telcli: socket");
    exit(1);
  }
  connect(dest, (struct sockaddr *)&sa, sizeof(sa));
#ifdef FNDELAY
  fcntl(source, F_SETFL, fcntl(source, F_GETFL, 0) | FNDELAY);
  fcntl(dest, F_SETFL, fcntl(dest, F_GETFL, 0) | FNDELAY);
#else
  fcntl(source, F_SETFL, O_NDELAY);
  fcntl(dest, F_SETFL, O_NDELAY);
#endif
  communicate(dest, source);
  close(dest);
  exit(0);
}

void communicate(int sfd, int cfd)
{
  char                                   *chead,
                                         *ctail,
                                         *shead,
                                         *stail;
  int                                     num,
                                          nfd,
                                          spos,
                                          cpos;

  /*
   * extern int errno; 
   */
  fd_set                                  rd,
                                          wr;

  chead = ctail = cbuf;
  cpos = 0;
  shead = stail = sbuf;
  spos = 0;
  while (1) {
    FD_ZERO(&rd);
    FD_ZERO(&wr);
    if (spos < sizeof(sbuf) - 1)
      FD_SET(sfd, &rd);
    if (ctail > chead)
      FD_SET(sfd, &wr);
    if (cpos < sizeof(cbuf) - 1)
      FD_SET(cfd, &rd);
    if (stail > shead)
      FD_SET(cfd, &wr);
    nfd = select(256, &rd, &wr, 0, 0);
    if (nfd <= 0)
      continue;
    if (FD_ISSET(sfd, &rd)) {
      num = read(sfd, stail, sizeof(sbuf) - spos);
      if ((num == -1) && (errno != EWOULDBLOCK))
	return;
      if (num == 0)
	return;
      if (num > 0) {
	spos += num;
	stail += num;
	if (!--nfd)
	  continue;
      }
    }
    if (FD_ISSET(cfd, &rd)) {
      num = read(cfd, ctail, sizeof(cbuf) - cpos);
      if ((num == -1) && (errno != EWOULDBLOCK))
	return;
      if (num == 0)
	return;
      if (num > 0) {
	cpos += num;
	ctail += num;
	if (!--nfd)
	  continue;
      }
    }
    if (FD_ISSET(sfd, &wr)) {
      num = write(sfd, chead, ctail - chead);
      if ((num == -1) && (errno != EWOULDBLOCK))
	return;
      if (num > 0) {
	chead += num;
	if (chead == ctail) {
	  chead = ctail = cbuf;
	  cpos = 0;
	}
	if (!--nfd)
	  continue;
      }
    }
    if (FD_ISSET(cfd, &wr)) {
      num = write(cfd, shead, stail - shead);
      if ((num == -1) && (errno != EWOULDBLOCK))
	return;
      if (num > 0) {
	shead += num;
	if (shead == stail) {
	  shead = stail = sbuf;
	  spos = 0;
	}
	if (!--nfd)
	  continue;
      }
    }
  }
}

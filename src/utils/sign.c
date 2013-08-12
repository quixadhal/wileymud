/*
 * Present a message on a port 
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>

#include "global.h"
#include "bug.h"

void                                    watch(int port, char *text);
void                                    wave(int sock, char *text, int port);
int                                     new_connection(int s, int port);
int                                     init_socket(int port);
int                                     write_to_descriptor(int desc, const char *txt);
void                                    nonblock(int s);

int main(int argc, char **argv)
{
    int                                     port;
    char                                    txt[8192],
                                            buf[83];
    FILE                                   *fl;

    if (argc != 3) {
	fputs("Usage: sign (<filename> | - ) <port #>\n", stderr);
	exit(-1);
    }
    if (!strcmp(argv[1], "-")) {
	fl = stdin;
	puts("Input text (terminate with ^D)");
    } else if (!(fl = fopen(argv[1], "r"))) {
	perror(argv[1]);
	exit(-1);
    }
    for (;;) {
	(void)fgets(buf, 81, fl);
	if (feof(fl))
	    break;
	strcat(buf, "\r");
	if (strlen(buf) + strlen(txt) > 8192) {
	    fputs("String too long\n", stderr);
	    exit(-1);
	}
	strcat(txt, buf);
    }
    if ((port = atoi(argv[2])) <= 1024) {
	fputs("Illegal port #\n", stderr);
	exit(-1);
    }
    watch(port, txt);
    return 0;
}

void watch(int port, char *text)
{
    int                                     mother;
    fd_set                                  input_set;

    mother = init_socket(port);

    FD_ZERO(&input_set);
    for (;;) {
	FD_SET(mother, &input_set);
	if (select(64, &input_set, 0, 0, 0) < 0) {
	    perror("select");
	    exit(-1);
	}
	if (FD_ISSET(mother, &input_set))
	    wave(mother, text, port);
    }
}

void wave(int sock, char *text, int port)
{
    int                                     s;

    if ((s = new_connection(sock, port)) < 0)
	return;

    write_to_descriptor(s, text);
    close(s);
}

int new_connection(int s, int port)
{
    struct sockaddr_in                      isa,
                                            ident;
    struct hostent                         *host;
    unsigned int                            i,
                                            len,
                                            t;
    int                                     fd,
                                            remote_port;
    long                                    remote_addr;
    FILE                                   *ifp,
                                           *ofp;
    char                                    buf[8192];
    int                                     args;
    int                                     lport,
                                            rport;
    char                                    reply_type[81];
    char                                    opsys_or_error[81];
    char                                    identifier[1024];
    char                                    charset[81];
    char                                    opsys[81];

    i = sizeof(isa);
    getsockname(s, (struct sockaddr *)&isa, &i);
    if ((t = accept(s, (struct sockaddr *)&isa, &i)) < 0) {
	perror("Accept");
	return (-1);
    }
    nonblock(t);
    remote_port = ntohs(isa.sin_port);
    remote_addr = isa.sin_addr.s_addr;
    printf("accept on port %d from ", remote_port);
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) >= 0) {
	ident.sin_family = AF_INET;
	ident.sin_addr.s_addr = remote_addr;
	ident.sin_port = htons(113);
	len = sizeof(ident);
	if (connect(fd, (struct sockaddr *)&ident, len) >= 0) {
	    len = sizeof(ident);
	    if (getsockname(fd, (struct sockaddr *)&ident, &len) >= 0) {
		if ((ifp = fdopen(fd, "r")) && (ofp = fdopen(fd, "w"))) {
		    fprintf(ofp, "%d , %d\n", remote_port, port);
		    fflush(ofp);
		    shutdown(fd, 1);
		    if (fgets(buf, sizeof(buf) - 1, ifp)) {
			if ((args = sscanf(buf,
					   " %d , %d : %[^ \t\n\r:] : %[^\t\n\r:] : %[^\n\r]",
					   &lport, &rport, reply_type, opsys_or_error,
					   identifier))
			    >= 3) {
			    opsys[0] = charset[0] = '\0';
			    if (sscanf(opsys_or_error, " %s , %s", opsys, charset) != 2)
				strcpy(opsys, opsys_or_error);
			    if (!strcasecmp(reply_type, "USERID")) {
				printf("%s@", identifier);
			    }
			}
		    }
		    fclose(ifp);
		    fclose(ofp);
		}
	    }
	}
	shutdown(fd, 0);
	close(fd);
    }
    if (!(host = gethostbyaddr((char *)&remote_addr, sizeof(remote_addr), AF_INET)))
	printf("%d.%d.%d.%d\n",
	       (int)((char *)remote_addr)[0], (int)((char *)remote_addr)[1],
	       (int)((char *)remote_addr)[2], (int)((char *)remote_addr)[3]);
    else
	printf("%s\n", host->h_name);
    fflush(stdout);
    return (t);
}

int init_socket(int port)
{
    int                                     s;
    char                                   *opt;
    char                                    hostname[1024];
    struct sockaddr_in                      sa;
    struct hostent                         *hp;
    struct linger                           ld;

    bzero(&sa, sizeof(struct sockaddr_in));

    gethostname(hostname, 1023);
    hp = gethostbyname(hostname);
    if (hp == NULL) {
	perror("gethostbyname");
	exit(-1);
    }
    sa.sin_family = hp->h_addrtype;
    sa.sin_port = htons(port);
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
	perror("Init-socket");
	exit(-1);
    }
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {
	perror("setsockopt REUSEADDR");
	exit(-1);
    }
    ld.l_onoff = 1;
    ld.l_linger = 1000;
    if (setsockopt(s, SOL_SOCKET, SO_LINGER, &ld, sizeof(ld)) < 0) {
	perror("setsockopt LINGER");
	exit(-1);
    }
    if (bind(s, (struct sockaddr *)&sa, sizeof(sa) /* , 0 */ ) < 0) {
	perror("bind");
	close(s);
	exit(-1);
    }
    listen(s, 5);
    return (s);
}

int write_to_descriptor(int desc, const char *txt)
{
    int                                     sofar;
    int                                     thisround;
    int                                     total;

    total = strlen(txt);
    sofar = 0;

    do {
	thisround = write(desc, txt + sofar, total - sofar);
	if (thisround < 0) {
	    perror("Write to socket");
	    return (-1);
	}
	sofar += thisround;
    }
    while (sofar < total);

    return (0);
}

void nonblock(int s)
{
    if (fcntl(s, F_SETFL, FNDELAY) == -1) {
	perror("Noblock");
	exit(-1);
    }
}

#
# Makefile for the libident library
#

MAKE=make
SHELL=/bin/sh
RM=rm -f
RANLIB=ranlib
AR=ar cq
CC=gcc

INSTROOT=/usr/local
LIBDIR=$(INSTROOT)/lib
INCDIR=$(INSTROOT)/include
MANDIR=$(INSTROOT)/man/man3

#
# Add -g for debugging information
#
# Add -DHAVE_ANSIHEADERS if your system have the ANSI C header files
# needed for our purposes (stdlib, string, unistd).
#
WARN = -Wall -Wstrict-prototypes -Wmissing-prototypes
OPT = -O3 #-fstrength-reduce -funroll-loops -fexpensive-optimizations
CFLAGS = $(WARN) $(OPT) -ggdb -pipe

LDLIBS =
LIBIDENT = libident.a

OBJS = ident.o id_open.o id_close.o id_query.o id_parse.o support.o version.o

#
fail:
	@echo "Select your platform and type 'make <platform>'"
	@echo ""
	@echo aix
	@echo bsd
	@echo dynix
	@echo hpux
	@echo irix
	@echo linux
	@echo netstep3.0
	@echo netstep3.1
	@echo sunos4
	@echo sunos5
	@echo svr4
	@echo ultrix
	@echo openvms
	@echo ""
	@echo other


all: 	$(LIBIDENT)

$(LIBIDENT):	$(OBJS)
	-$(RM) $(LIBIDENT)
	$(AR) $(LIBIDENT) $(OBJS)
	-$(RANLIB) $(LIBIDENT)

testers: lookup-tester ident-tester

ident-tester: $(LIBIDENT) ident-tester.o
	$(CC) $(CFLAGS) -o ident-tester ident-tester.o $(LIBIDENT) $(LDLIBS)

lookup-tester: $(LIBIDENT) lookup-tester.o
	$(CC) $(CFLAGS) -o lookup-tester lookup-tester.o $(LIBIDENT) $(LDLIBS)


ident.o:	 ident.c         ident.h
id_open.o:	 id_open.c       ident.h
id_close.o:	 id_close.c      ident.h
id_query.o:	 id_query.c      ident.h
id_parse.o:	 id_parse.c      ident.h
support.o:	 support.c	 ident.h
ident-tester.o:  ident-tester.c  ident.h
lookup-tester.o: lookup-tester.c ident.h
version.o:	 version.c

#

version:
	(PACKNAME=`basename \`pwd\`` ; echo 'char id_version[] = "'`echo $$PACKNAME | cut -d- -f2`'";' >version.c)

#

install:
	-$(RM) $(LIBDIR)/$(LIBIDENT)
	cp $(LIBIDENT) $(LIBDIR)/$(LIBIDENT)
	-$(RANLIB) $(LIBDIR)/$(LIBIDENT)
	-$(RM) $(INCDIR)/ident.h
	cp ident.h $(INCDIR)/ident.h
	-$(RM) $(MANDIR)/ident.3
	cp ident.3 $(MANDIR)/ident.3

clean:
	-$(RM) $(LIBIDENT) ident-tester lookup-tester *~ core *.o \#*

#

distZ:	clean version
	(PACKNAME=`basename \`pwd\`` ; cd .. ; /usr/gnu/bin/tar Zcf $$PACKNAME.tar.Z $$PACKNAME)

dist distz:	clean version
	(PACKNAME=`basename \`pwd\`` ; cd .. ; /usr/gnu/bin/tar cf - $$PACKNAME | gzip -9 >$$PACKNAME.tar.gz)



#
# System specific targets ---------------------------------------------
#
sunos5 solaris2 svr4:
	@$(MAKE) all CC="$(CC)" CFLAGS="-O -DHAVE_ANSIHEADERS" LDLIBS="-lsocket -lnsl" RANLIB=/bin/true

#
#
sunos sunos4 solaris1 ultrix:
	@$(MAKE) all CC="$(CC)" CFLAGS="-O -DHAVE_ANSIHEADERS"


irix:
	@$(MAKE) all CC="$(CC)" CFLAGS="-O -ansi -DHAVE_ANSIHEADERS" LDLIBS="-lsocket -lnsl" RANLIB=/bin/true

#
#
bsd:
	@$(MAKE) all CC="$(CC)" CFLAGS="-O -DSIGRETURNTYPE=int"

#
#
dynix dynix3:
	@$(MAKE) all CC="$(CC)" CFLAGS="-O -DSIGRETURNTYPE=int"

#
#
aix hpux hpux8 hpux9:
	@$(MAKE) all CC="$(CC)" CFLAGS="-O -DHAVE_ANSIHEADERS"

#
#
linux:
	@$(MAKE) all CC="gcc" CFLAGS="$(CFLAGS) -DHAVE_ANSIHEADERS"

#
#
nextstep3.0:
	@$(MAKE) all CC="$(CC)" CFLAGS="-O -Wall -s -object -DHAVE_ANSIHEADERS -DNeXT3"

#
#
NEXTARCHS=-arch m68k -arch i386

nextstep3.1:
	@$(MAKE) all CC="$(CC)" CFLAGS="-O -Wall -s -object -DHAVE_ANSIHEADERS -DNeXT3 $(NEXTARCHS)"


openvms:	all

other:		all

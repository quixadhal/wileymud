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
OPT = -O3 -fstrength-reduce -funroll-loops -fexpensive-optimizations
CFLAGS = $(WARN) $(OPT) -g -pipe

LDLIBS =
LIBIDENT = libauthuser.a

OBJS = authuser.o

all: 	$(LIBIDENT)

$(LIBIDENT):	$(OBJS)
	-$(RM) $(LIBIDENT)
	$(AR) $(LIBIDENT) $(OBJS)
	-$(RANLIB) $(LIBIDENT)

authuser.o:	authuser.c	authuser.h

install:
	-$(RM) $(LIBDIR)/$(LIBIDENT)
	cp $(LIBIDENT) $(LIBDIR)/$(LIBIDENT)
	-$(RANLIB) $(LIBDIR)/$(LIBIDENT)
	-$(RM) $(INCDIR)/authuser.h
	cp authuser.h $(INCDIR)/authuser.h
	-$(RM) $(MANDIR)/authuser.3
	cp authuser.3 $(MANDIR)/authuser.3


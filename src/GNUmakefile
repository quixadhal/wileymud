#!/usr/bin/make
# Must be GNU Make!

#WIDTH=96
#LINES=80
WIDTH=132
LINES=58
TARFILE=$(shell /bin/date "+../backup-%y%m%d-%H%M%S.tar.gz")
DATESTAMP=$(shell /bin/date "+%y.%m.%d")
OLDVERSION=$(shell cat .version.count)
VERSION=$(shell echo $(OLDVERSION) + 1 | bc)

CC = gcc
#GNUINC = /usr/local/lib/gcc-lib/sparc-sun-sunos4.1.3_U1/2.6.3/include
#GNUINC = /usr/lib/gcc-lib/i486-linux/2.5.8/include
#GNUINC = /usr/lib/gcc-lib/i486-linux/2.6.3/include
#GNUINC = /usr/lib/gcc-lib/i486-linux/2.7.0/include
#GNUINC = /usr/lib/gcc-lib/i486-linux/2.7.2/include
#GNUINC = /usr/lib/gcc-lib/i386-redhat-linux/2.7.2.3/include
#GNUINC = /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.90.27/include
#GNUINC = /usr/lib/gcc-lib/i486-linux/2.7.2.3/include
GNUINC = /usr/lib/gcc-lib/i386-unknown-openbsd3.0/2.95.3/include
#PROF = -pg
#BUGFLAGS = -DDEBUG_FILE
#WARN = -Wall -Wstrict-prototypes -Wmissing-prototypes
WARN = -Wall -Wmissing-prototypes
#WARN = -w
DEBUG = -g -fno-common -fno-builtin -rdynamic -m486 -static #-fcheck-memory-usage
#DEBUG = -g3 -static
#OPT = -O3 -fstrength-reduce -fexpensive-optimizations -funroll-loops -finline-functions #-m586
#STRIP = -s
PIPE = -pipe -fpcc-struct-return

#CFLAGS = -g -Dsun3 -m68020 -fstrength-reduce -fvolatile 
CFLAGS = $(WARN) $(DEBUG) $(PROF) $(OPT) $(PIPE) -Iinclude
LDFLAGS = $(DEBUG) $(PROF) $(STRIP)

SRC =	act_comm.c act_info.c act_move.c act_obj.c act_off.c \
	act_other.c act_skills.c act_social.c act_wiz.c \
	board.c bug.c comm.c constants.c db.c fight.c handler.c \
	hash.c interpreter.c limits.c magic.c magic_utils.c \
	modify.c multiclass.c opinion.c random.c reception.c \
	shop.c signals.c sound.c spell_parser.c \
	spells.c trap.c utils.c weather.c whod.c events.c
OBJ =	$(addprefix obj/, $(SRC:.c=.o))
TMP =	mob_actions.c spec_procs.c breath_weapons.c tracking.c
SPOB =	$(addprefix obj/, $(TMP:.c=.o))
SPEC =	$(addprefix specials/, $(TMP))
TMP2 =	sign.c
OBJ2 =	$(addprefix obj/, $(TMP2:.c=.o))
SRC2 =	$(addprefix utils/, $(TMP2))
TMP3 =	mkworld.c
OBJ3 =	$(addprefix obj/, $(TMP3:.c=.o))
SRC3 =	$(addprefix utils/, $(TMP3))
HDRS =	act_comm.h act_info.h act_move.h act_obj.h act_off.h \
	act_other.h act_skills.h act_social.h act_wiz.h \
	board.h bug.h comm.h constants.h db.h fight.h global.h \
	handler.h hash.h interpreter.h limits.h magic_utils.h \
	modify.h multiclass.h opinion.h \
	random.h reception.h shop.h sound.h spell_parser.h \
	spells.h trap.h utils.h weather.h whod.h signals.h events.h \
	mob_actions.h spec_procs.h breath_weapons.h tracking.h
HDR =	$(addprefix include/, $(HDRS))
VER =	$(addprefix include/, version.h)
IDENTDIR = libident-0.19
IDENTSRC = $(addprefix $(IDENTDIR)/, id_close.c id_open.c id_parse.c \
	id_query.c ident.c support.c version.c)
IDENTHDR = $(addprefix $(IDENTDIR)/, ident.h)
IDENTOBJ = $(IDENTSRC:.c=.o)
LIBS =	$(IDENTDIR)/libident.a
LIB =	-L$(IDENTDIR) -lident -lcompat # -lefence # -lcrypt
AUX =	GNUmakefile .indent.pro HACKLOG version.proto .version.count \
	world.wld

ALLSRC = $(SRC) $(SPEC) $(SRC2) $(SRC3) $(IDENTSRC)

all : bin/wileymud

.deps/%.d: %.c
	set -e; $(CC) -M $(CFLAGS) $< \
		| sed 's!\($*\)\.o[ :]*!\1.o $@ : !g' > $@; \
		[ -s $@ ] || rm -f $@

include $(addprefix .deps/, $(ALLSRC:.c=.d))

$(VER) : version.proto .version.count
	@echo '/* Auto-generated from version.proto -- DO NOT EDIT */' > $@
	@echo 's/XxX/$(VERSION)/' >.sed.cmds
	@echo 's/YyY/$(DATESTAMP)/' >>.sed.cmds
	@sed -f .sed.cmds <version.proto >>$@
	@rm -f .sed.cmds
	@echo New version file generated.

bin/wileymud : $(OBJ) $(SPOB) $(LIBS)
	@echo Heave Ho!  Load the beastie!
	$(CC) $(LDFLAGS) -o $@ $(OBJ) $(SPOB) $(LIB)
	@echo $(VERSION) >.version.count
	@ls -l $@

bin/sign : $(OBJ2)
	@echo Make the downtime notice program...
	$(CC) $(LDFLAGS) -o $@ $(OBJ2) $(LIB)
	@ls -l $@

bin/mkworld : $(OBJ3)
	@echo Make the world editing program...
	$(CC) $(LDFLAGS) -o $@ $(OBJ3) $(LIB)
	@ls -l $@

$(OBJ): obj/%o : %c
	$(CC) $(CFLAGS) -c $< -o $@

$(SPOB): obj/%o : specials/%c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ2): obj/%o : utils/%c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ3): obj/%o : utils/%c
	$(CC) $(CFLAGS) -c $< -o $@

$(IDENTDIR)/libident.a: $(IDENTSRC) $(IDENTHDR)
	(cd libident-0.19;make linux)

lex.yy.c : dbparser.l
	flex -Cem $<

clean :
	@echo -n Wiping away intermediate clutter...
	@rm -f $(OBJ) $(SPOB) $(OBJ2) $(OBJ3) $(AUTHOBJ) $(IDENTOBJ) *.bak *~
	@find . -name \*.d | xargs rm -f
	@echo done.

spotless : clean
	@echo -n Destroying the world and all those within it...
	@rm -f bin/wileymud bin/sign bin/mkworld
	@rm -f printout sedscr
	@rm -f tags refs
	@rm -f $(LIBS)
	@echo done!

backup :
	@echo -n Creating idiot-proof file...
	@tar -zcf $(TARFILE) $(SRC) $(SPEC) $(HDR) $(VER) $(AUX) $(SRC2) $(SRC3)
	@chmod 640 ../backup*
	@echo your ass is safe.

printout : $(SRC) $(VER) $(HDR) dbparser.l
	@echo -n Generating tree-killer...
	@pr -l$(LINES) -w$(WIDTH) -f -n5 $(VER) $(HDR) $(SRC) $(SPEC) > $@
	@echo the forests quiver in terror!

tags : $(SRC) $(SPEC) $(VER) $(HDR)
	@echo -n Making tag files...
	@ctags -TwdS $(SRC) $(SPEC) $(VER) $(HDR)
	@echo done.

depend : $(SRC) $(SPEC) $(VER) $(HDR)
	@echo -n Figuring out dependancies...
	@makedepend -pobj/ -fGNUmakefile -I$(GNUINC) -- $(CFLAGS) -- $(SRC) $(SPEC) $(SRC2) $(SRC3) $(VER) $(HDR)
	@echo done.

# DO NOT DELETE THIS LINE -- make depend depends on it.


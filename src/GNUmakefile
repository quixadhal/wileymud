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
GNUINC = /usr/lib/gcc-lib/i486-linux/2.6.3/include
#PROF = -pg
#BUGFLAGS = -DDEBUG_FILE
#DEBUG = -ggdb3 -Wall
#DEBUG = -g -Wall $(BUGFLAGS) -DVERBOSE
#DEBUG = -g -O3 -w $(BUGFLAGS) -DVERBOSE
#DEBUG = -Wall -DVERBOSE
OPT = -O3 -fomit-frame-pointer -fstrength-reduce -fexpensive-optimizations
#STRIP = -s
#INCDIR = -I../inc

#CFLAGS = -g -Dsun3 -m68020 -fstrength-reduce -fvolatile 
CFLAGS = $(DEBUG) $(PROF) $(OPT) $(INCDIR) -pipe
LDFLAGS = $(DEBUG) $(PROF) $(STRIP)

SRC =	Opinion.c Sound.c Trap.c \
	act.comm.c act.informative.c act.move.c act.obj1.c \
	act.obj2.c act.offensive.c \
	act.other.c act.social.c act.wizard.c \
	board.c comm.c constants2.c db.c fight.c handler.c \
	hash.c interpreter.c limits.c magic.c magic2.c magic3.c \
	magicutils.c mobact.c modify.c multiclass.c reception.c shop.c \
	signals.c skills.c spec_assign.c spec_procs.c spell_parser.c \
	spells1.c spells2.c spells3.c utility.c weather.c whod.c yywrap.c
OBJ =	$(SRC:.c=.o)
SRC2 =	sign.c
OBJ2 =	$(SRC2:.c=.o)
HDR =	comm.h db.h debug.h guilds.h handler.h hash.h immunities.h \
	interpreter.h limits.h mail.h multiclass.h myerrors.h opinion.h \
	poly.h race.h reception.h s_list.h spells.h structs.h \
	trap.h utils.h
VER =	version.h
LIB =
AUX =	GNUmakefile .indent.pro HACKLOG version.proto .version.count \
	lex.yy.c dbparser.l

all : wileymud

version.h : version.proto .version.count
	@echo '/* Auto-generated from version.proto -- DO NOT EDIT */' > $@
	@echo 's/XxX/$(VERSION)/' >.sed.cmds
	@echo 's/YyY/$(DATESTAMP)/' >>.sed.cmds
	@sed -f .sed.cmds <version.proto >>$@
	@rm -f .sed.cmds
	@echo New version file generated.

wileymud : $(OBJ)
	@echo Heave Ho!  Load the beastie!
	$(CC) $(LDFLAGS) -o $@ $(OBJ) $(LIB)
	@echo $(VERSION) >.version.count
	@ls -l $@*

sign : $(OBJ2)
	@echo Make the downtime notice program...
	$(CC) $(LDFLAGS) -o $@ $(OBJ2) $(LIB)
	@ls -l $@*

$(OBJ) $(OBJ2) : %o : %c
	$(CC) $(CFLAGS) -c $<

lex.yy.c : dbparser.l
	flex -Cem $<

clean :
	@echo -n Wiping away intermediate clutter...
	@rm -f *.o *.bak *~
	@echo done.

spotless : clean
	@echo -n Destroying the world and all those within it...
	@rm -f wileymud notice
	@rm -f printout sedscr
	@rm -f tags refs
	@echo done!

backup :
	@echo -n Creating idiot-proof file...
	@tar -zcf $(TARFILE) $(SRC) $(HDR) $(VER) $(AUX) $(SRC2)
	@chmod 640 ../backup*
	@echo your ass is safe.

printout : $(SRC) $(VER) $(HDR) dbparser.l
	@echo -n Generating tree-killer...
	@pr -l$(LINES) -w$(WIDTH) -f -n5 $(VER) $(HDR) $(SRC) > $@
	@echo the forests quiver in terror!

tags : $(SRC) $(VER) $(HDR)
	@echo -n Making tag files...
	@ctags -twdS $(SRC) $(VER) $(HDR)
	@echo done.

depend : $(SRC) $(VER) $(HDR)
	@echo -n Figuring out dependancies...
	@makedepend -fGNUmakefile -I$(GNUINC) -- $(CFLAGS) -- $(SRC) $(VER) $(HDR)
	@echo done.

# DO NOT DELETE THIS LINE -- make depend depends on it.

Opinion.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
Opinion.o: /usr/include/string.h /usr/include/features.h
Opinion.o: /usr/include/sys/cdefs.h
Opinion.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
Opinion.o: /usr/include/ctype.h /usr/include/time.h structs.h
Opinion.o: /usr/include/sys/types.h /usr/include/linux/types.h
Opinion.o: /usr/include/asm/types.h utils.h spells.h race.h opinion.h db.h
Sound.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
Sound.o: /usr/include/string.h /usr/include/features.h
Sound.o: /usr/include/sys/cdefs.h
Sound.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h structs.h
Sound.o: /usr/include/sys/types.h /usr/include/linux/types.h
Sound.o: /usr/include/asm/types.h utils.h comm.h interpreter.h handler.h db.h
Sound.o: spells.h trap.h
Trap.o: structs.h /usr/include/sys/types.h /usr/include/linux/types.h
Trap.o: /usr/include/asm/types.h utils.h spells.h trap.h comm.h
act.comm.o: /usr/include/stdio.h /usr/include/libio.h
act.comm.o: /usr/include/_G_config.h /usr/include/string.h
act.comm.o: /usr/include/features.h /usr/include/sys/cdefs.h
act.comm.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h structs.h
act.comm.o: /usr/include/sys/types.h /usr/include/linux/types.h
act.comm.o: /usr/include/asm/types.h utils.h comm.h interpreter.h handler.h
act.comm.o: db.h spells.h
act.informative.o: /usr/include/stdio.h /usr/include/libio.h
act.informative.o: /usr/include/_G_config.h /usr/include/string.h
act.informative.o: /usr/include/features.h /usr/include/sys/cdefs.h
act.informative.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
act.informative.o: /usr/include/ctype.h /usr/include/time.h structs.h
act.informative.o: /usr/include/sys/types.h /usr/include/linux/types.h
act.informative.o: /usr/include/asm/types.h utils.h comm.h interpreter.h
act.informative.o: handler.h db.h spells.h limits.h race.h trap.h hash.h
act.move.o: /usr/include/stdio.h /usr/include/libio.h
act.move.o: /usr/include/_G_config.h /usr/include/string.h
act.move.o: /usr/include/features.h /usr/include/sys/cdefs.h
act.move.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h structs.h
act.move.o: /usr/include/sys/types.h /usr/include/linux/types.h
act.move.o: /usr/include/asm/types.h utils.h comm.h interpreter.h handler.h
act.move.o: db.h spells.h trap.h
act.obj1.o: /usr/include/stdio.h /usr/include/libio.h
act.obj1.o: /usr/include/_G_config.h /usr/include/string.h
act.obj1.o: /usr/include/features.h /usr/include/sys/cdefs.h
act.obj1.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h structs.h
act.obj1.o: /usr/include/sys/types.h /usr/include/linux/types.h
act.obj1.o: /usr/include/asm/types.h utils.h comm.h interpreter.h handler.h
act.obj1.o: db.h spells.h trap.h
act.obj2.o: /usr/include/stdio.h /usr/include/libio.h
act.obj2.o: /usr/include/_G_config.h /usr/include/string.h
act.obj2.o: /usr/include/features.h /usr/include/sys/cdefs.h
act.obj2.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
act.obj2.o: /usr/include/assert.h structs.h /usr/include/sys/types.h
act.obj2.o: /usr/include/linux/types.h /usr/include/asm/types.h utils.h
act.obj2.o: comm.h interpreter.h handler.h db.h spells.h
act.offensive.o: /usr/include/stdio.h /usr/include/libio.h
act.offensive.o: /usr/include/_G_config.h /usr/include/string.h
act.offensive.o: /usr/include/features.h /usr/include/sys/cdefs.h
act.offensive.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h structs.h
act.offensive.o: /usr/include/sys/types.h /usr/include/linux/types.h
act.offensive.o: /usr/include/asm/types.h utils.h comm.h interpreter.h
act.offensive.o: handler.h db.h spells.h limits.h opinion.h race.h
act.offensive.o: multiclass.h
act.other.o: /usr/include/stdio.h /usr/include/libio.h
act.other.o: /usr/include/_G_config.h /usr/include/string.h
act.other.o: /usr/include/features.h /usr/include/sys/cdefs.h
act.other.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
act.other.o: /usr/include/ctype.h structs.h /usr/include/sys/types.h
act.other.o: /usr/include/linux/types.h /usr/include/asm/types.h utils.h
act.other.o: comm.h interpreter.h handler.h db.h spells.h limits.h
act.social.o: /usr/include/stdio.h /usr/include/libio.h
act.social.o: /usr/include/_G_config.h /usr/include/string.h
act.social.o: /usr/include/features.h /usr/include/sys/cdefs.h
act.social.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h structs.h
act.social.o: /usr/include/sys/types.h /usr/include/linux/types.h
act.social.o: /usr/include/asm/types.h utils.h comm.h interpreter.h handler.h
act.social.o: db.h spells.h
act.wizard.o: /usr/include/stdio.h /usr/include/libio.h
act.wizard.o: /usr/include/_G_config.h /usr/include/string.h
act.wizard.o: /usr/include/features.h /usr/include/sys/cdefs.h
act.wizard.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
act.wizard.o: /usr/include/ctype.h structs.h /usr/include/sys/types.h
act.wizard.o: /usr/include/linux/types.h /usr/include/asm/types.h utils.h
act.wizard.o: comm.h interpreter.h handler.h db.h spells.h limits.h race.h
board.o: /usr/include/string.h /usr/include/features.h
board.o: /usr/include/sys/cdefs.h
board.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
board.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
board.o: /usr/include/ctype.h structs.h /usr/include/sys/types.h
board.o: /usr/include/linux/types.h /usr/include/asm/types.h utils.h comm.h
board.o: db.h
comm.o: /usr/include/errno.h /usr/include/features.h /usr/include/sys/cdefs.h
comm.o: /usr/include/linux/errno.h /usr/include/stdio.h /usr/include/libio.h
comm.o: /usr/include/_G_config.h /usr/include/ctype.h
comm.o: /usr/include/sys/types.h /usr/include/linux/types.h
comm.o: /usr/include/asm/types.h /usr/include/sys/socket.h
comm.o: /usr/include/linux/socket.h /usr/include/linux/sockios.h
comm.o: /usr/include/sys/wait.h /usr/include/gnu/types.h
comm.o: /usr/include/waitflags.h /usr/include/waitstatus.h
comm.o: /usr/include/endian.h /usr/include/bytesex.h
comm.o: /usr/include/netinet/in.h /usr/include/linux/in.h
comm.o: /usr/include/asm/byteorder.h /usr/include/netdb.h
comm.o: /usr/include/paths.h /usr/include/string.h
comm.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
comm.o: /usr/include/sys/time.h /usr/include/linux/time.h /usr/include/time.h
comm.o: /usr/include/fcntl.h /usr/include/linux/fcntl.h /usr/include/signal.h
comm.o: /usr/include/linux/signal.h /usr/include/sys/un.h
comm.o: /usr/include/linux/un.h /usr/include/sys/resource.h limits.h
comm.o: /usr/include/linux/resource.h structs.h utils.h comm.h interpreter.h
comm.o: handler.h db.h
constants2.o: structs.h /usr/include/sys/types.h /usr/include/linux/types.h
constants2.o: /usr/include/asm/types.h limits.h trap.h
db.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
db.o: /usr/include/string.h /usr/include/features.h /usr/include/sys/cdefs.h
db.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h /usr/include/ctype.h
db.o: /usr/include/time.h structs.h /usr/include/sys/types.h
db.o: /usr/include/linux/types.h /usr/include/asm/types.h utils.h db.h comm.h
db.o: handler.h limits.h race.h opinion.h hash.h spells.h myerrors.h lex.yy.c
db.o: /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h
db.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h /usr/include/alloca.h
fight.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
fight.o: /usr/include/string.h /usr/include/features.h
fight.o: /usr/include/sys/cdefs.h
fight.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
fight.o: /usr/include/assert.h structs.h /usr/include/sys/types.h
fight.o: /usr/include/linux/types.h /usr/include/asm/types.h utils.h comm.h
fight.o: handler.h interpreter.h db.h spells.h
handler.o: /usr/include/string.h /usr/include/features.h
handler.o: /usr/include/sys/cdefs.h
handler.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
handler.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
handler.o: /usr/include/ctype.h /usr/include/assert.h structs.h
handler.o: /usr/include/sys/types.h /usr/include/linux/types.h
handler.o: /usr/include/asm/types.h utils.h comm.h db.h handler.h
handler.o: interpreter.h spells.h
hash.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
hash.o: hash.h
interpreter.o: /usr/include/string.h /usr/include/features.h
interpreter.o: /usr/include/sys/cdefs.h
interpreter.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
interpreter.o: /usr/include/ctype.h /usr/include/stdio.h /usr/include/libio.h
interpreter.o: /usr/include/_G_config.h /usr/include/arpa/telnet.h structs.h
interpreter.o: /usr/include/sys/types.h /usr/include/linux/types.h
interpreter.o: /usr/include/asm/types.h comm.h version.h interpreter.h db.h
interpreter.o: utils.h limits.h race.h
limits.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
limits.o: /usr/include/assert.h structs.h /usr/include/sys/types.h
limits.o: /usr/include/linux/types.h /usr/include/asm/types.h limits.h
limits.o: utils.h spells.h comm.h race.h
magic.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
magic.o: /usr/include/assert.h structs.h /usr/include/sys/types.h
magic.o: /usr/include/linux/types.h /usr/include/asm/types.h utils.h comm.h
magic.o: spells.h handler.h limits.h db.h
magic2.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
magic2.o: /usr/include/assert.h structs.h /usr/include/sys/types.h
magic2.o: /usr/include/linux/types.h /usr/include/asm/types.h utils.h comm.h
magic2.o: spells.h handler.h limits.h db.h race.h
magic3.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
magic3.o: /usr/include/assert.h structs.h /usr/include/sys/types.h
magic3.o: /usr/include/linux/types.h /usr/include/asm/types.h utils.h comm.h
magic3.o: spells.h handler.h limits.h db.h race.h
magicutils.o: /usr/include/stdio.h /usr/include/libio.h
magicutils.o: /usr/include/_G_config.h /usr/include/assert.h structs.h
magicutils.o: /usr/include/sys/types.h /usr/include/linux/types.h
magicutils.o: /usr/include/asm/types.h utils.h comm.h spells.h handler.h
magicutils.o: limits.h
mobact.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
mobact.o: structs.h /usr/include/sys/types.h /usr/include/linux/types.h
mobact.o: /usr/include/asm/types.h utils.h handler.h db.h comm.h opinion.h
mobact.o: trap.h
modify.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
modify.o: /usr/include/ctype.h /usr/include/features.h
modify.o: /usr/include/sys/cdefs.h /usr/include/string.h
modify.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
modify.o: /usr/include/time.h structs.h /usr/include/sys/types.h
modify.o: /usr/include/linux/types.h /usr/include/asm/types.h utils.h
modify.o: interpreter.h handler.h db.h comm.h
multiclass.o: /usr/include/stdio.h /usr/include/libio.h
multiclass.o: /usr/include/_G_config.h /usr/include/string.h
multiclass.o: /usr/include/features.h /usr/include/sys/cdefs.h
multiclass.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h structs.h
multiclass.o: /usr/include/sys/types.h /usr/include/linux/types.h
multiclass.o: /usr/include/asm/types.h utils.h comm.h interpreter.h handler.h
multiclass.o: spells.h limits.h opinion.h race.h multiclass.h
reception.o: /usr/include/stdio.h /usr/include/libio.h
reception.o: /usr/include/_G_config.h /usr/include/sys/time.h
reception.o: /usr/include/features.h /usr/include/sys/cdefs.h
reception.o: /usr/include/linux/types.h /usr/include/asm/types.h
reception.o: /usr/include/linux/time.h /usr/include/time.h
reception.o: /usr/include/sys/types.h structs.h comm.h handler.h db.h
reception.o: interpreter.h utils.h spells.h reception.h
shop.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
shop.o: /usr/include/string.h /usr/include/features.h
shop.o: /usr/include/sys/cdefs.h
shop.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h structs.h
shop.o: /usr/include/sys/types.h /usr/include/linux/types.h
shop.o: /usr/include/asm/types.h comm.h handler.h db.h interpreter.h utils.h
signals.o: /usr/include/signal.h /usr/include/features.h
signals.o: /usr/include/sys/cdefs.h /usr/include/sys/types.h
signals.o: /usr/include/linux/types.h /usr/include/asm/types.h
signals.o: /usr/include/linux/signal.h /usr/include/stdio.h
signals.o: /usr/include/libio.h /usr/include/_G_config.h
signals.o: /usr/include/sys/time.h /usr/include/linux/time.h
signals.o: /usr/include/time.h structs.h utils.h
skills.o: structs.h /usr/include/sys/types.h /usr/include/linux/types.h
skills.o: /usr/include/asm/types.h utils.h race.h spells.h comm.h handler.h
spec_assign.o: /usr/include/stdio.h /usr/include/libio.h
spec_assign.o: /usr/include/_G_config.h structs.h /usr/include/sys/types.h
spec_assign.o: /usr/include/linux/types.h /usr/include/asm/types.h db.h
spec_procs.o: /usr/include/stdio.h /usr/include/libio.h
spec_procs.o: /usr/include/_G_config.h /usr/include/string.h
spec_procs.o: /usr/include/features.h /usr/include/sys/cdefs.h
spec_procs.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
spec_procs.o: /usr/include/ctype.h structs.h /usr/include/sys/types.h
spec_procs.o: /usr/include/linux/types.h /usr/include/asm/types.h utils.h
spec_procs.o: comm.h interpreter.h handler.h db.h spells.h limits.h s_list.h
spec_procs.o: opinion.h hash.h
spell_parser.o: /usr/include/stdio.h /usr/include/libio.h
spell_parser.o: /usr/include/_G_config.h /usr/include/assert.h structs.h
spell_parser.o: /usr/include/sys/types.h /usr/include/linux/types.h
spell_parser.o: /usr/include/asm/types.h utils.h comm.h db.h interpreter.h
spell_parser.o: spells.h handler.h
spells1.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
spells1.o: structs.h /usr/include/sys/types.h /usr/include/linux/types.h
spells1.o: /usr/include/asm/types.h utils.h comm.h db.h interpreter.h
spells1.o: spells.h handler.h s_list.h
spells2.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
spells2.o: structs.h /usr/include/sys/types.h /usr/include/linux/types.h
spells2.o: /usr/include/asm/types.h utils.h comm.h db.h interpreter.h
spells2.o: spells.h handler.h poly.h
spells3.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
spells3.o: /usr/include/strings.h /usr/include/string.h
spells3.o: /usr/include/features.h /usr/include/sys/cdefs.h
spells3.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h structs.h
spells3.o: /usr/include/sys/types.h /usr/include/linux/types.h
spells3.o: /usr/include/asm/types.h utils.h comm.h db.h interpreter.h
spells3.o: spells.h handler.h poly.h
utility.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
utility.o: /usr/include/malloc.h /usr/include/features.h
utility.o: /usr/include/sys/cdefs.h
utility.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
utility.o: /usr/include/string.h /usr/include/assert.h structs.h
utility.o: /usr/include/sys/types.h /usr/include/linux/types.h
utility.o: /usr/include/asm/types.h /usr/include/time.h utils.h spells.h
utility.o: race.h db.h opinion.h comm.h hash.h
weather.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
weather.o: /usr/include/string.h /usr/include/features.h
weather.o: /usr/include/sys/cdefs.h
weather.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h structs.h
weather.o: /usr/include/sys/types.h /usr/include/linux/types.h
weather.o: /usr/include/asm/types.h utils.h comm.h handler.h interpreter.h
weather.o: db.h
whod.o: version.h /usr/include/sys/types.h /usr/include/linux/types.h
whod.o: /usr/include/asm/types.h /usr/include/sys/socket.h
whod.o: /usr/include/features.h /usr/include/sys/cdefs.h
whod.o: /usr/include/linux/socket.h /usr/include/linux/sockios.h
whod.o: /usr/include/sys/un.h /usr/include/linux/un.h /usr/include/netdb.h
whod.o: /usr/include/paths.h /usr/include/netinet/in.h
whod.o: /usr/include/linux/in.h /usr/include/asm/byteorder.h
whod.o: /usr/include/sys/time.h /usr/include/linux/time.h /usr/include/time.h
whod.o: /usr/include/signal.h /usr/include/linux/signal.h
whod.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
whod.o: structs.h utils.h
structs.o: /usr/include/sys/types.h /usr/include/linux/types.h
structs.o: /usr/include/asm/types.h

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
#WARN = -Wall -Wstrict-prototypes -Wmissing-prototypes
WARN = -w
DEBUG = -g
OPT = -O3 -fstrength-reduce -fexpensive-optimizations -funroll-loops -m486
#OPT = -O3 -fomit-frame-pointer -fstrength-reduce -fexpensive-optimizations -funroll-loops -m486
#STRIP = -s
#INCDIR = -I../inc

#CFLAGS = -g -Dsun3 -m68020 -fstrength-reduce -fvolatile 
CFLAGS = $(WARN) $(DEBUG) $(PROF) $(OPT) $(INCDIR) -pipe
LDFLAGS = $(DEBUG) $(PROF) $(STRIP)

SRC =	act_comm.c act_info.c act_move.c act_obj.c act_off.c \
	act_other.c act_skills.c act_social.c act_wiz.c \
	board.c bug.c comm.c constants.c db.c fight.c handler.c \
	hash.c interpreter.c limits.c magic.c magic_utils.c \
	mob_actions.c modify.c multiclass.c opinion.c reception.c \
	shop.c signals.c sound.c spec_procs.c spell_parser.c \
	spells.c trap.c utils.c weather.c whod.c events.c
OBJ =	$(SRC:.c=.o)
SRC2 =	sign.c
OBJ2 =	$(SRC2:.c=.o)
SRC3 =	mkworld.c
OBJ3 =	$(SRC3:.c=.o)
HDR =	act_comm.h act_info.h act_move.h act_obj.h act_off.h \
	act_other.h act_skills.h act_social.h act_wiz.h \
	board.h bug.h comm.h constants.h db.h fight.h global.h \
	handler.h hash.h interpreter.h limits.h magic_utils.h \
	mob_actions.h modify.h multiclass.h myerrors.h opinion.h \
	reception.h shop.h sound.h spec_procs.h spell_parser.h \
	spells.h trap.h utils.h weather.h whod.h signals.h events.h
VER =	version.h
LIB =
AUX =	GNUmakefile .indent.pro HACKLOG version.proto .version.count \
	world.wld

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
	@ls -l $@

sign : $(OBJ2)
	@echo Make the downtime notice program...
	$(CC) $(LDFLAGS) -o $@ $(OBJ2) $(LIB)
	@ls -l $@

mkworld : $(OBJ3)
	@echo Make the world editing program...
	$(CC) $(LDFLAGS) -o $@ $(OBJ3) $(LIB)
	@ls -l $@

$(OBJ) $(OBJ2) $(OBJ3): %o : %c
	$(CC) $(CFLAGS) -c $<

lex.yy.c : dbparser.l
	flex -Cem $<

clean :
	@echo -n Wiping away intermediate clutter...
	@rm -f *.o *.bak *~
	@echo done.

spotless : clean
	@echo -n Destroying the world and all those within it...
	@rm -f wileymud notice mkworld
	@rm -f printout sedscr
	@rm -f tags refs
	@echo done!

backup :
	@echo -n Creating idiot-proof file...
	@tar -zcf $(TARFILE) $(SRC) $(HDR) $(VER) $(AUX) $(SRC2) $(SRC3)
	@chmod 640 ../backup*
	@echo your ass is safe.

printout : $(SRC) $(VER) $(HDR) dbparser.l
	@echo -n Generating tree-killer...
	@pr -l$(LINES) -w$(WIDTH) -f -n5 $(VER) $(HDR) $(SRC) > $@
	@echo the forests quiver in terror!

tags : $(SRC) $(VER) $(HDR)
	@echo -n Making tag files...
	@ctags -TwdS $(SRC) $(VER) $(HDR)
	@echo done.

depend : $(SRC) $(VER) $(HDR)
	@echo -n Figuring out dependancies...
	@makedepend -fGNUmakefile -I$(GNUINC) -- $(CFLAGS) -- $(SRC) $(VER) $(HDR)
	@echo done.

# DO NOT DELETE THIS LINE -- make depend depends on it.

act_comm.o: /usr/include/stdio.h /usr/include/libio.h
act_comm.o: /usr/include/_G_config.h /usr/include/stdlib.h
act_comm.o: /usr/include/features.h /usr/include/sys/cdefs.h
act_comm.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
act_comm.o: /usr/include/errno.h /usr/include/linux/errno.h
act_comm.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h
act_comm.o: /usr/include/alloca.h /usr/include/unistd.h
act_comm.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
act_comm.o: /usr/include/confname.h /usr/include/sys/types.h
act_comm.o: /usr/include/linux/types.h /usr/include/asm/types.h
act_comm.o: /usr/include/string.h global.h bug.h utils.h comm.h interpreter.h
act_comm.o: handler.h db.h spells.h constants.h act_comm.h
act_info.o: /usr/include/stdio.h /usr/include/libio.h
act_info.o: /usr/include/_G_config.h /usr/include/stdlib.h
act_info.o: /usr/include/features.h /usr/include/sys/cdefs.h
act_info.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
act_info.o: /usr/include/errno.h /usr/include/linux/errno.h
act_info.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h
act_info.o: /usr/include/alloca.h /usr/include/unistd.h
act_info.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
act_info.o: /usr/include/confname.h /usr/include/sys/types.h
act_info.o: /usr/include/linux/types.h /usr/include/asm/types.h
act_info.o: /usr/include/string.h /usr/include/ctype.h /usr/include/time.h
act_info.o: global.h bug.h version.h utils.h comm.h interpreter.h handler.h
act_info.o: db.h spells.h limits.h trap.h hash.h constants.h spell_parser.h
act_info.o: whod.h act_info.h
act_move.o: /usr/include/stdio.h /usr/include/libio.h
act_move.o: /usr/include/_G_config.h /usr/include/stdlib.h
act_move.o: /usr/include/features.h /usr/include/sys/cdefs.h
act_move.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
act_move.o: /usr/include/errno.h /usr/include/linux/errno.h
act_move.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h
act_move.o: /usr/include/alloca.h /usr/include/unistd.h
act_move.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
act_move.o: /usr/include/confname.h /usr/include/sys/types.h
act_move.o: /usr/include/linux/types.h /usr/include/asm/types.h
act_move.o: /usr/include/string.h global.h bug.h utils.h comm.h interpreter.h
act_move.o: handler.h db.h spells.h trap.h constants.h whod.h act_move.h
act_obj.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
act_obj.o: /usr/include/stdlib.h /usr/include/features.h
act_obj.o: /usr/include/sys/cdefs.h
act_obj.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
act_obj.o: /usr/include/errno.h /usr/include/linux/errno.h
act_obj.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h
act_obj.o: /usr/include/alloca.h /usr/include/unistd.h
act_obj.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
act_obj.o: /usr/include/confname.h /usr/include/sys/types.h
act_obj.o: /usr/include/linux/types.h /usr/include/asm/types.h
act_obj.o: /usr/include/string.h /usr/include/assert.h global.h bug.h utils.h
act_obj.o: comm.h interpreter.h handler.h db.h spells.h trap.h constants.h
act_obj.o: spell_parser.h act_obj.h
act_off.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
act_off.o: /usr/include/stdlib.h /usr/include/features.h
act_off.o: /usr/include/sys/cdefs.h
act_off.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
act_off.o: /usr/include/errno.h /usr/include/linux/errno.h
act_off.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h
act_off.o: /usr/include/alloca.h /usr/include/unistd.h
act_off.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
act_off.o: /usr/include/confname.h /usr/include/sys/types.h
act_off.o: /usr/include/linux/types.h /usr/include/asm/types.h
act_off.o: /usr/include/string.h global.h bug.h utils.h comm.h interpreter.h
act_off.o: handler.h db.h spells.h limits.h opinion.h multiclass.h
act_off.o: constants.h spec_procs.h act_off.h
act_other.o: /usr/include/stdio.h /usr/include/libio.h
act_other.o: /usr/include/_G_config.h /usr/include/stdlib.h
act_other.o: /usr/include/features.h /usr/include/sys/cdefs.h
act_other.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
act_other.o: /usr/include/errno.h /usr/include/linux/errno.h
act_other.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h
act_other.o: /usr/include/alloca.h /usr/include/unistd.h
act_other.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
act_other.o: /usr/include/confname.h /usr/include/sys/types.h
act_other.o: /usr/include/linux/types.h /usr/include/asm/types.h
act_other.o: /usr/include/string.h /usr/include/ctype.h global.h bug.h
act_other.o: utils.h comm.h interpreter.h handler.h db.h spells.h limits.h
act_other.o: constants.h spell_parser.h act_other.h
act_skills.o: /usr/include/stdio.h /usr/include/libio.h
act_skills.o: /usr/include/_G_config.h /usr/include/stdlib.h
act_skills.o: /usr/include/features.h /usr/include/sys/cdefs.h
act_skills.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
act_skills.o: /usr/include/errno.h /usr/include/linux/errno.h
act_skills.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h
act_skills.o: /usr/include/alloca.h /usr/include/unistd.h
act_skills.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
act_skills.o: /usr/include/confname.h /usr/include/sys/types.h
act_skills.o: /usr/include/linux/types.h /usr/include/asm/types.h global.h
act_skills.o: bug.h db.h constants.h spells.h comm.h handler.h utils.h
act_skills.o: act_skills.h
act_social.o: /usr/include/stdio.h /usr/include/libio.h
act_social.o: /usr/include/_G_config.h /usr/include/stdlib.h
act_social.o: /usr/include/features.h /usr/include/sys/cdefs.h
act_social.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
act_social.o: /usr/include/errno.h /usr/include/linux/errno.h
act_social.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h
act_social.o: /usr/include/alloca.h /usr/include/unistd.h
act_social.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
act_social.o: /usr/include/confname.h /usr/include/sys/types.h
act_social.o: /usr/include/linux/types.h /usr/include/asm/types.h
act_social.o: /usr/include/string.h global.h bug.h utils.h comm.h
act_social.o: interpreter.h handler.h db.h spells.h multiclass.h act_social.h
act_wiz.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
act_wiz.o: /usr/include/stdlib.h /usr/include/features.h
act_wiz.o: /usr/include/sys/cdefs.h
act_wiz.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
act_wiz.o: /usr/include/errno.h /usr/include/linux/errno.h
act_wiz.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h
act_wiz.o: /usr/include/alloca.h /usr/include/unistd.h
act_wiz.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
act_wiz.o: /usr/include/confname.h /usr/include/sys/types.h
act_wiz.o: /usr/include/linux/types.h /usr/include/asm/types.h
act_wiz.o: /usr/include/string.h /usr/include/ctype.h global.h bug.h utils.h
act_wiz.o: comm.h interpreter.h handler.h db.h spells.h limits.h constants.h
act_wiz.o: spell_parser.h board.h act_wiz.h
board.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
board.o: /usr/include/stdlib.h /usr/include/features.h
board.o: /usr/include/sys/cdefs.h
board.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
board.o: /usr/include/errno.h /usr/include/linux/errno.h
board.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h
board.o: /usr/include/alloca.h /usr/include/unistd.h /usr/include/posix_opt.h
board.o: /usr/include/gnu/types.h /usr/include/confname.h
board.o: /usr/include/sys/types.h /usr/include/linux/types.h
board.o: /usr/include/asm/types.h /usr/include/string.h /usr/include/ctype.h
board.o: global.h bug.h utils.h comm.h db.h board.h
bug.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
bug.o: /usr/include/stdlib.h /usr/include/features.h /usr/include/sys/cdefs.h
bug.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
bug.o: /usr/include/errno.h /usr/include/linux/errno.h
bug.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h
bug.o: /usr/include/alloca.h /usr/include/unistd.h /usr/include/posix_opt.h
bug.o: /usr/include/gnu/types.h /usr/include/confname.h
bug.o: /usr/include/sys/types.h /usr/include/linux/types.h
bug.o: /usr/include/asm/types.h
bug.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stdarg.h /usr/include/time.h
bug.o: /usr/include/string.h global.h utils.h comm.h multiclass.h bug.h
comm.o: /usr/include/errno.h /usr/include/features.h /usr/include/sys/cdefs.h
comm.o: /usr/include/linux/errno.h /usr/include/stdio.h /usr/include/libio.h
comm.o: /usr/include/_G_config.h /usr/include/stdlib.h
comm.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
comm.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h
comm.o: /usr/include/alloca.h /usr/include/unistd.h /usr/include/posix_opt.h
comm.o: /usr/include/gnu/types.h /usr/include/confname.h
comm.o: /usr/include/sys/types.h /usr/include/linux/types.h
comm.o: /usr/include/asm/types.h
comm.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stdarg.h
comm.o: /usr/include/ctype.h /usr/include/sys/socket.h
comm.o: /usr/include/linux/socket.h /usr/include/linux/sockios.h
comm.o: /usr/include/sys/wait.h /usr/include/waitflags.h
comm.o: /usr/include/waitstatus.h /usr/include/endian.h
comm.o: /usr/include/bytesex.h /usr/include/netinet/in.h
comm.o: /usr/include/linux/in.h /usr/include/asm/byteorder.h
comm.o: /usr/include/netdb.h /usr/include/paths.h /usr/include/string.h
comm.o: /usr/include/sys/time.h /usr/include/linux/time.h /usr/include/time.h
comm.o: /usr/include/fcntl.h /usr/include/linux/fcntl.h /usr/include/signal.h
comm.o: /usr/include/linux/signal.h /usr/include/sys/un.h
comm.o: /usr/include/linux/un.h /usr/include/sys/resource.h limits.h
comm.o: /usr/include/linux/resource.h global.h bug.h utils.h interpreter.h
comm.o: handler.h db.h comm.h
constants.o: /usr/include/stdlib.h /usr/include/features.h
constants.o: /usr/include/sys/cdefs.h
constants.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
constants.o: /usr/include/errno.h /usr/include/linux/errno.h
constants.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h
constants.o: /usr/include/alloca.h /usr/include/unistd.h
constants.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
constants.o: /usr/include/confname.h /usr/include/sys/types.h
constants.o: /usr/include/linux/types.h /usr/include/asm/types.h global.h
constants.o: bug.h limits.h trap.h constants.h
db.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
db.o: /usr/include/stdlib.h /usr/include/features.h /usr/include/sys/cdefs.h
db.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h /usr/include/errno.h
db.o: /usr/include/linux/errno.h
db.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h /usr/include/alloca.h
db.o: /usr/include/unistd.h /usr/include/posix_opt.h /usr/include/gnu/types.h
db.o: /usr/include/confname.h /usr/include/sys/types.h
db.o: /usr/include/linux/types.h /usr/include/asm/types.h
db.o: /usr/include/string.h /usr/include/ctype.h /usr/include/time.h global.h
db.o: bug.h utils.h comm.h handler.h limits.h opinion.h hash.h constants.h
db.o: spells.h spell_parser.h myerrors.h db.h
fight.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
fight.o: /usr/include/stdlib.h /usr/include/features.h
fight.o: /usr/include/sys/cdefs.h
fight.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
fight.o: /usr/include/errno.h /usr/include/linux/errno.h
fight.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h
fight.o: /usr/include/alloca.h /usr/include/unistd.h /usr/include/posix_opt.h
fight.o: /usr/include/gnu/types.h /usr/include/confname.h
fight.o: /usr/include/sys/types.h /usr/include/linux/types.h
fight.o: /usr/include/asm/types.h /usr/include/string.h /usr/include/assert.h
fight.o: global.h bug.h utils.h comm.h handler.h interpreter.h db.h
fight.o: constants.h spells.h spell_parser.h fight.h
handler.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
handler.o: /usr/include/stdlib.h /usr/include/features.h
handler.o: /usr/include/sys/cdefs.h
handler.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
handler.o: /usr/include/errno.h /usr/include/linux/errno.h
handler.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h
handler.o: /usr/include/alloca.h /usr/include/unistd.h
handler.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
handler.o: /usr/include/confname.h /usr/include/sys/types.h
handler.o: /usr/include/linux/types.h /usr/include/asm/types.h
handler.o: /usr/include/string.h /usr/include/ctype.h /usr/include/assert.h
handler.o: global.h bug.h utils.h comm.h db.h interpreter.h spells.h
handler.o: spell_parser.h constants.h fight.h handler.h
hash.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
hash.o: /usr/include/stdlib.h /usr/include/features.h
hash.o: /usr/include/sys/cdefs.h
hash.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
hash.o: /usr/include/errno.h /usr/include/linux/errno.h
hash.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h
hash.o: /usr/include/alloca.h /usr/include/unistd.h /usr/include/posix_opt.h
hash.o: /usr/include/gnu/types.h /usr/include/confname.h
hash.o: /usr/include/sys/types.h /usr/include/linux/types.h
hash.o: /usr/include/asm/types.h /usr/include/string.h global.h bug.h comm.h
hash.o: hash.h
interpreter.o: /usr/include/stdio.h /usr/include/libio.h
interpreter.o: /usr/include/_G_config.h /usr/include/stdlib.h
interpreter.o: /usr/include/features.h /usr/include/sys/cdefs.h
interpreter.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
interpreter.o: /usr/include/errno.h /usr/include/linux/errno.h
interpreter.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h
interpreter.o: /usr/include/alloca.h /usr/include/unistd.h
interpreter.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
interpreter.o: /usr/include/confname.h /usr/include/sys/types.h
interpreter.o: /usr/include/linux/types.h /usr/include/asm/types.h
interpreter.o: /usr/include/string.h /usr/include/ctype.h
interpreter.o: /usr/include/arpa/telnet.h global.h bug.h comm.h version.h
interpreter.o: db.h utils.h limits.h constants.h act_comm.h act_info.h
interpreter.o: act_move.h act_obj.h act_off.h act_other.h act_skills.h
interpreter.o: act_social.h act_wiz.h spells.h spell_parser.h modify.h whod.h
interpreter.o: events.h interpreter.h
limits.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
limits.o: /usr/include/stdlib.h /usr/include/features.h
limits.o: /usr/include/sys/cdefs.h
limits.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
limits.o: /usr/include/errno.h /usr/include/linux/errno.h
limits.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h
limits.o: /usr/include/alloca.h /usr/include/unistd.h
limits.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
limits.o: /usr/include/confname.h /usr/include/sys/types.h
limits.o: /usr/include/linux/types.h /usr/include/asm/types.h
limits.o: /usr/include/assert.h global.h bug.h spells.h comm.h db.h
limits.o: spell_parser.h constants.h utils.h limits.h
magic.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
magic.o: /usr/include/stdlib.h /usr/include/features.h
magic.o: /usr/include/sys/cdefs.h
magic.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
magic.o: /usr/include/errno.h /usr/include/linux/errno.h
magic.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h
magic.o: /usr/include/alloca.h /usr/include/unistd.h /usr/include/posix_opt.h
magic.o: /usr/include/gnu/types.h /usr/include/confname.h
magic.o: /usr/include/sys/types.h /usr/include/linux/types.h
magic.o: /usr/include/asm/types.h /usr/include/assert.h global.h bug.h
magic.o: utils.h comm.h handler.h limits.h db.h constants.h spells.h
magic.o: spell_parser.h
magic_utils.o: /usr/include/stdio.h /usr/include/libio.h
magic_utils.o: /usr/include/_G_config.h /usr/include/stdlib.h
magic_utils.o: /usr/include/features.h /usr/include/sys/cdefs.h
magic_utils.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
magic_utils.o: /usr/include/errno.h /usr/include/linux/errno.h
magic_utils.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h
magic_utils.o: /usr/include/alloca.h /usr/include/unistd.h
magic_utils.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
magic_utils.o: /usr/include/confname.h /usr/include/sys/types.h
magic_utils.o: /usr/include/linux/types.h /usr/include/asm/types.h
magic_utils.o: /usr/include/assert.h global.h bug.h utils.h comm.h db.h
magic_utils.o: spells.h handler.h limits.h fight.h spell_parser.h
magic_utils.o: multiclass.h magic_utils.h
mob_actions.o: /usr/include/stdio.h /usr/include/libio.h
mob_actions.o: /usr/include/_G_config.h /usr/include/stdlib.h
mob_actions.o: /usr/include/features.h /usr/include/sys/cdefs.h
mob_actions.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
mob_actions.o: /usr/include/errno.h /usr/include/linux/errno.h
mob_actions.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h
mob_actions.o: /usr/include/alloca.h /usr/include/unistd.h
mob_actions.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
mob_actions.o: /usr/include/confname.h /usr/include/sys/types.h
mob_actions.o: /usr/include/linux/types.h /usr/include/asm/types.h global.h
mob_actions.o: bug.h utils.h handler.h db.h comm.h constants.h opinion.h
mob_actions.o: trap.h mob_actions.h
modify.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
modify.o: /usr/include/stdlib.h /usr/include/features.h
modify.o: /usr/include/sys/cdefs.h
modify.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
modify.o: /usr/include/errno.h /usr/include/linux/errno.h
modify.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h
modify.o: /usr/include/alloca.h /usr/include/unistd.h
modify.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
modify.o: /usr/include/confname.h /usr/include/sys/types.h
modify.o: /usr/include/linux/types.h /usr/include/asm/types.h
modify.o: /usr/include/ctype.h /usr/include/string.h /usr/include/time.h
modify.o: global.h bug.h utils.h interpreter.h handler.h db.h comm.h modify.h
multiclass.o: /usr/include/stdio.h /usr/include/libio.h
multiclass.o: /usr/include/_G_config.h /usr/include/stdlib.h
multiclass.o: /usr/include/features.h /usr/include/sys/cdefs.h
multiclass.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
multiclass.o: /usr/include/errno.h /usr/include/linux/errno.h
multiclass.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h
multiclass.o: /usr/include/alloca.h /usr/include/unistd.h
multiclass.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
multiclass.o: /usr/include/confname.h /usr/include/sys/types.h
multiclass.o: /usr/include/linux/types.h /usr/include/asm/types.h
multiclass.o: /usr/include/string.h global.h bug.h utils.h comm.h db.h
multiclass.o: interpreter.h handler.h spells.h limits.h opinion.h constants.h
multiclass.o: multiclass.h
opinion.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
opinion.o: /usr/include/stdlib.h /usr/include/features.h
opinion.o: /usr/include/sys/cdefs.h
opinion.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
opinion.o: /usr/include/errno.h /usr/include/linux/errno.h
opinion.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h
opinion.o: /usr/include/alloca.h /usr/include/unistd.h
opinion.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
opinion.o: /usr/include/confname.h /usr/include/sys/types.h
opinion.o: /usr/include/linux/types.h /usr/include/asm/types.h
opinion.o: /usr/include/string.h /usr/include/ctype.h /usr/include/time.h
opinion.o: global.h bug.h utils.h spells.h constants.h comm.h db.h opinion.h
reception.o: /usr/include/stdio.h /usr/include/libio.h
reception.o: /usr/include/_G_config.h /usr/include/stdlib.h
reception.o: /usr/include/features.h /usr/include/sys/cdefs.h
reception.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
reception.o: /usr/include/errno.h /usr/include/linux/errno.h
reception.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h
reception.o: /usr/include/alloca.h /usr/include/unistd.h
reception.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
reception.o: /usr/include/confname.h /usr/include/sys/types.h
reception.o: /usr/include/linux/types.h /usr/include/asm/types.h
reception.o: /usr/include/sys/time.h /usr/include/linux/time.h
reception.o: /usr/include/time.h global.h bug.h comm.h handler.h db.h
reception.o: interpreter.h utils.h spells.h reception.h
shop.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
shop.o: /usr/include/stdlib.h /usr/include/features.h
shop.o: /usr/include/sys/cdefs.h
shop.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
shop.o: /usr/include/errno.h /usr/include/linux/errno.h
shop.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h
shop.o: /usr/include/alloca.h /usr/include/unistd.h /usr/include/posix_opt.h
shop.o: /usr/include/gnu/types.h /usr/include/confname.h
shop.o: /usr/include/sys/types.h /usr/include/linux/types.h
shop.o: /usr/include/asm/types.h /usr/include/string.h global.h bug.h comm.h
shop.o: handler.h db.h interpreter.h utils.h constants.h shop.h
signals.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
signals.o: /usr/include/stdlib.h /usr/include/features.h
signals.o: /usr/include/sys/cdefs.h
signals.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
signals.o: /usr/include/errno.h /usr/include/linux/errno.h
signals.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h
signals.o: /usr/include/alloca.h /usr/include/unistd.h
signals.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
signals.o: /usr/include/confname.h /usr/include/sys/types.h
signals.o: /usr/include/linux/types.h /usr/include/asm/types.h
signals.o: /usr/include/signal.h /usr/include/linux/signal.h
signals.o: /usr/include/sys/time.h /usr/include/linux/time.h
signals.o: /usr/include/time.h global.h bug.h utils.h comm.h whod.h signals.h
sound.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
sound.o: /usr/include/stdlib.h /usr/include/features.h
sound.o: /usr/include/sys/cdefs.h
sound.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
sound.o: /usr/include/errno.h /usr/include/linux/errno.h
sound.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h
sound.o: /usr/include/alloca.h /usr/include/unistd.h /usr/include/posix_opt.h
sound.o: /usr/include/gnu/types.h /usr/include/confname.h
sound.o: /usr/include/sys/types.h /usr/include/linux/types.h
sound.o: /usr/include/asm/types.h /usr/include/string.h global.h bug.h comm.h
sound.o: interpreter.h handler.h db.h spells.h trap.h utils.h sound.h
spec_procs.o: /usr/include/stdio.h /usr/include/libio.h
spec_procs.o: /usr/include/_G_config.h /usr/include/stdlib.h
spec_procs.o: /usr/include/features.h /usr/include/sys/cdefs.h
spec_procs.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
spec_procs.o: /usr/include/errno.h /usr/include/linux/errno.h
spec_procs.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h
spec_procs.o: /usr/include/alloca.h /usr/include/unistd.h
spec_procs.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
spec_procs.o: /usr/include/confname.h /usr/include/sys/types.h
spec_procs.o: /usr/include/linux/types.h /usr/include/asm/types.h
spec_procs.o: /usr/include/string.h /usr/include/ctype.h global.h bug.h
spec_procs.o: utils.h comm.h interpreter.h handler.h db.h limits.h opinion.h
spec_procs.o: hash.h spells.h constants.h spell_parser.h board.h reception.h
spec_procs.o: spec_procs.h
spell_parser.o: /usr/include/stdio.h /usr/include/libio.h
spell_parser.o: /usr/include/_G_config.h /usr/include/stdlib.h
spell_parser.o: /usr/include/features.h /usr/include/sys/cdefs.h
spell_parser.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
spell_parser.o: /usr/include/errno.h /usr/include/linux/errno.h
spell_parser.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h
spell_parser.o: /usr/include/alloca.h /usr/include/unistd.h
spell_parser.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
spell_parser.o: /usr/include/confname.h /usr/include/sys/types.h
spell_parser.o: /usr/include/linux/types.h /usr/include/asm/types.h
spell_parser.o: /usr/include/assert.h global.h bug.h utils.h comm.h db.h
spell_parser.o: interpreter.h handler.h spells.h constants.h act_off.h
spell_parser.o: spell_parser.h
spells.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
spells.o: /usr/include/stdlib.h /usr/include/features.h
spells.o: /usr/include/sys/cdefs.h
spells.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
spells.o: /usr/include/errno.h /usr/include/linux/errno.h
spells.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h
spells.o: /usr/include/alloca.h /usr/include/unistd.h
spells.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
spells.o: /usr/include/confname.h /usr/include/sys/types.h
spells.o: /usr/include/linux/types.h /usr/include/asm/types.h
spells.o: /usr/include/strings.h /usr/include/string.h global.h bug.h utils.h
spells.o: comm.h db.h interpreter.h handler.h constants.h spells.h
spells.o: spell_parser.h
trap.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
trap.o: /usr/include/stdlib.h /usr/include/features.h
trap.o: /usr/include/sys/cdefs.h
trap.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
trap.o: /usr/include/errno.h /usr/include/linux/errno.h
trap.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h
trap.o: /usr/include/alloca.h /usr/include/unistd.h /usr/include/posix_opt.h
trap.o: /usr/include/gnu/types.h /usr/include/confname.h
trap.o: /usr/include/sys/types.h /usr/include/linux/types.h
trap.o: /usr/include/asm/types.h global.h bug.h utils.h spells.h comm.h db.h
trap.o: opinion.h constants.h fight.h reception.h spell_parser.h multiclass.h
trap.o: handler.h act_info.h trap.h
utils.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
utils.o: /usr/include/stdlib.h /usr/include/features.h
utils.o: /usr/include/sys/cdefs.h
utils.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
utils.o: /usr/include/errno.h /usr/include/linux/errno.h
utils.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h
utils.o: /usr/include/alloca.h /usr/include/unistd.h /usr/include/posix_opt.h
utils.o: /usr/include/gnu/types.h /usr/include/confname.h
utils.o: /usr/include/sys/types.h /usr/include/linux/types.h
utils.o: /usr/include/asm/types.h /usr/include/malloc.h /usr/include/string.h
utils.o: /usr/include/assert.h /usr/include/time.h global.h bug.h spells.h
utils.o: constants.h db.h opinion.h comm.h hash.h utils.h
weather.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
weather.o: /usr/include/stdlib.h /usr/include/features.h
weather.o: /usr/include/sys/cdefs.h
weather.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
weather.o: /usr/include/errno.h /usr/include/linux/errno.h
weather.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h
weather.o: /usr/include/alloca.h /usr/include/unistd.h
weather.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
weather.o: /usr/include/confname.h /usr/include/sys/types.h
weather.o: /usr/include/linux/types.h /usr/include/asm/types.h
weather.o: /usr/include/string.h global.h bug.h utils.h comm.h handler.h
weather.o: interpreter.h db.h weather.h
whod.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
whod.o: /usr/include/stdlib.h /usr/include/features.h
whod.o: /usr/include/sys/cdefs.h
whod.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
whod.o: /usr/include/errno.h /usr/include/linux/errno.h
whod.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h
whod.o: /usr/include/alloca.h /usr/include/unistd.h /usr/include/posix_opt.h
whod.o: /usr/include/gnu/types.h /usr/include/confname.h
whod.o: /usr/include/sys/types.h /usr/include/linux/types.h
whod.o: /usr/include/asm/types.h /usr/include/sys/socket.h
whod.o: /usr/include/linux/socket.h /usr/include/linux/sockios.h
whod.o: /usr/include/sys/un.h /usr/include/linux/un.h /usr/include/netdb.h
whod.o: /usr/include/paths.h /usr/include/netinet/in.h
whod.o: /usr/include/linux/in.h /usr/include/asm/byteorder.h
whod.o: /usr/include/sys/time.h /usr/include/linux/time.h /usr/include/time.h
whod.o: /usr/include/signal.h /usr/include/linux/signal.h version.h global.h
whod.o: bug.h db.h comm.h utils.h whod.h
events.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
events.o: /usr/include/stdlib.h /usr/include/features.h
events.o: /usr/include/sys/cdefs.h
events.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h
events.o: /usr/include/errno.h /usr/include/linux/errno.h
events.o: /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h
events.o: /usr/include/alloca.h /usr/include/unistd.h
events.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
events.o: /usr/include/confname.h /usr/include/sys/types.h
events.o: /usr/include/linux/types.h /usr/include/asm/types.h
events.o: /usr/include/string.h /usr/include/ctype.h global.h bug.h utils.h
events.o: comm.h interpreter.h handler.h db.h spells.h limits.h constants.h
events.o: spell_parser.h fight.h spec_procs.h events.h

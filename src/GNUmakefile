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
GNUINC = /usr/lib/gcc-lib/i486-linux/2.7.2/include
#PROF = -pg
#BUGFLAGS = -DDEBUG_FILE
#WARN = -Wall -Wstrict-prototypes -Wmissing-prototypes
WARN = -Wall -Wmissing-prototypes
#WARN = -w
#DEBUG = -fomit-frame-pointer
DEBUG = -g
#OPT = -O3 -fstrength-reduce -fexpensive-optimizations -funroll-loops -m486
#STRIP = -s
PIPE = -pipe

#CFLAGS = -g -Dsun3 -m68020 -fstrength-reduce -fvolatile 
CFLAGS = $(WARN) $(DEBUG) $(PROF) $(OPT) $(PIPE)
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
OBJ2 =	$(addprefix obj/, $(SRC2:.c=.o))
SRC2 =	$(addprefix utils/, $(TMP2))
TMP3 =	mkworld.c
OBJ3 =	$(addprefix obj/, $(SRC3:.c=.o))
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
LIB =	-L$(IDENTDIR) -lident
AUX =	GNUmakefile .indent.pro HACKLOG version.proto .version.count \
	world.wld

all : bin/wileymud

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

obj/act_comm.o: /usr/include/stdio.h /usr/include/libio.h
obj/act_comm.o: /usr/include/_G_config.h /usr/include/sys/cdefs.h
obj/act_comm.o: /usr/include/stdlib.h /usr/include/features.h
obj/act_comm.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/act_comm.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/act_comm.o: /usr/include/alloca.h /usr/include/unistd.h
obj/act_comm.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/act_comm.o: /usr/include/confname.h /usr/include/sys/types.h
obj/act_comm.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/act_comm.o: /usr/include/sys/bitypes.h /usr/include/string.h
obj/act_comm.o: include/global.h include/bug.h include/utils.h include/comm.h
obj/act_comm.o: include/interpreter.h include/handler.h include/db.h
obj/act_comm.o: include/spells.h include/constants.h include/limits.h
obj/act_comm.o: include/multiclass.h include/act_comm.h
obj/act_info.o: /usr/include/stdio.h /usr/include/libio.h
obj/act_info.o: /usr/include/_G_config.h /usr/include/sys/cdefs.h
obj/act_info.o: /usr/include/stdlib.h /usr/include/features.h
obj/act_info.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/act_info.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/act_info.o: /usr/include/alloca.h /usr/include/unistd.h
obj/act_info.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/act_info.o: /usr/include/confname.h /usr/include/sys/types.h
obj/act_info.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/act_info.o: /usr/include/sys/bitypes.h /usr/include/string.h
obj/act_info.o: /usr/include/ctype.h /usr/include/endian.h
obj/act_info.o: /usr/include/bytesex.h /usr/include/time.h include/global.h
obj/act_info.o: include/bug.h include/version.h include/utils.h
obj/act_info.o: include/comm.h include/interpreter.h include/handler.h
obj/act_info.o: include/db.h include/spells.h include/limits.h include/trap.h
obj/act_info.o: include/hash.h include/constants.h include/spell_parser.h
obj/act_info.o: include/whod.h include/multiclass.h include/modify.h
obj/act_info.o: include/act_wiz.h include/act_skills.h include/spec_procs.h
obj/act_info.o: include/tracking.h include/act_info.h
obj/act_move.o: /usr/include/stdio.h /usr/include/libio.h
obj/act_move.o: /usr/include/_G_config.h /usr/include/sys/cdefs.h
obj/act_move.o: /usr/include/stdlib.h /usr/include/features.h
obj/act_move.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/act_move.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/act_move.o: /usr/include/alloca.h /usr/include/unistd.h
obj/act_move.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/act_move.o: /usr/include/confname.h /usr/include/sys/types.h
obj/act_move.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/act_move.o: /usr/include/sys/bitypes.h /usr/include/string.h
obj/act_move.o: include/global.h include/bug.h include/utils.h include/comm.h
obj/act_move.o: include/interpreter.h include/handler.h include/db.h
obj/act_move.o: include/spells.h include/trap.h include/constants.h
obj/act_move.o: include/whod.h include/multiclass.h include/fight.h
obj/act_move.o: include/reception.h include/magic_utils.h
obj/act_move.o: include/spell_parser.h include/act_info.h
obj/act_move.o: include/act_skills.h include/act_move.h
obj/act_obj.o: /usr/include/stdio.h /usr/include/libio.h
obj/act_obj.o: /usr/include/_G_config.h /usr/include/sys/cdefs.h
obj/act_obj.o: /usr/include/stdlib.h /usr/include/features.h
obj/act_obj.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/act_obj.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/act_obj.o: /usr/include/alloca.h /usr/include/unistd.h
obj/act_obj.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/act_obj.o: /usr/include/confname.h /usr/include/sys/types.h
obj/act_obj.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/act_obj.o: /usr/include/sys/bitypes.h /usr/include/string.h
obj/act_obj.o: /usr/include/assert.h /usr/include/ctype.h
obj/act_obj.o: /usr/include/endian.h /usr/include/bytesex.h include/global.h
obj/act_obj.o: include/bug.h include/utils.h include/comm.h
obj/act_obj.o: include/interpreter.h include/handler.h include/db.h
obj/act_obj.o: include/spells.h include/trap.h include/constants.h
obj/act_obj.o: include/spell_parser.h include/multiclass.h include/limits.h
obj/act_obj.o: include/fight.h include/act_info.h include/act_obj.h
obj/act_off.o: /usr/include/stdio.h /usr/include/libio.h
obj/act_off.o: /usr/include/_G_config.h /usr/include/sys/cdefs.h
obj/act_off.o: /usr/include/stdlib.h /usr/include/features.h
obj/act_off.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/act_off.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/act_off.o: /usr/include/alloca.h /usr/include/unistd.h
obj/act_off.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/act_off.o: /usr/include/confname.h /usr/include/sys/types.h
obj/act_off.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/act_off.o: /usr/include/sys/bitypes.h /usr/include/string.h
obj/act_off.o: /usr/include/ctype.h /usr/include/endian.h
obj/act_off.o: /usr/include/bytesex.h include/global.h include/bug.h
obj/act_off.o: include/utils.h include/comm.h include/interpreter.h
obj/act_off.o: include/handler.h include/db.h include/spells.h
obj/act_off.o: include/limits.h include/opinion.h include/multiclass.h
obj/act_off.o: include/constants.h include/spec_procs.h include/fight.h
obj/act_off.o: include/act_skills.h include/act_move.h include/spell_parser.h
obj/act_off.o: include/act_info.h include/breath_weapons.h include/act_off.h
obj/act_other.o: /usr/include/stdio.h /usr/include/libio.h
obj/act_other.o: /usr/include/_G_config.h /usr/include/sys/cdefs.h
obj/act_other.o: /usr/include/stdlib.h /usr/include/features.h
obj/act_other.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/act_other.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/act_other.o: /usr/include/alloca.h /usr/include/unistd.h
obj/act_other.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/act_other.o: /usr/include/confname.h /usr/include/sys/types.h
obj/act_other.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/act_other.o: /usr/include/sys/bitypes.h /usr/include/string.h
obj/act_other.o: /usr/include/ctype.h /usr/include/endian.h
obj/act_other.o: /usr/include/bytesex.h include/global.h include/bug.h
obj/act_other.o: include/utils.h include/comm.h include/interpreter.h
obj/act_other.o: include/handler.h include/db.h include/spells.h
obj/act_other.o: include/limits.h include/constants.h include/spell_parser.h
obj/act_other.o: include/reception.h include/multiclass.h include/fight.h
obj/act_other.o: include/spec_procs.h include/act_skills.h include/act_comm.h
obj/act_other.o: include/modify.h include/act_other.h
obj/act_skills.o: /usr/include/stdio.h /usr/include/libio.h
obj/act_skills.o: /usr/include/_G_config.h /usr/include/sys/cdefs.h
obj/act_skills.o: /usr/include/stdlib.h /usr/include/features.h
obj/act_skills.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/act_skills.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/act_skills.o: /usr/include/alloca.h /usr/include/unistd.h
obj/act_skills.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/act_skills.o: /usr/include/confname.h /usr/include/sys/types.h
obj/act_skills.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/act_skills.o: /usr/include/sys/bitypes.h include/global.h include/bug.h
obj/act_skills.o: include/utils.h include/comm.h include/constants.h
obj/act_skills.o: include/db.h include/fight.h include/handler.h
obj/act_skills.o: include/interpreter.h include/multiclass.h
obj/act_skills.o: include/spell_parser.h include/spells.h
obj/act_skills.o: include/spec_procs.h include/act_skills.h
obj/act_social.o: /usr/include/stdio.h /usr/include/libio.h
obj/act_social.o: /usr/include/_G_config.h /usr/include/sys/cdefs.h
obj/act_social.o: /usr/include/stdlib.h /usr/include/features.h
obj/act_social.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/act_social.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/act_social.o: /usr/include/alloca.h /usr/include/unistd.h
obj/act_social.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/act_social.o: /usr/include/confname.h /usr/include/sys/types.h
obj/act_social.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/act_social.o: /usr/include/sys/bitypes.h /usr/include/string.h
obj/act_social.o: include/global.h include/bug.h include/utils.h
obj/act_social.o: include/comm.h include/interpreter.h include/handler.h
obj/act_social.o: include/db.h include/spells.h include/multiclass.h
obj/act_social.o: include/act_social.h
obj/act_wiz.o: /usr/include/stdio.h /usr/include/libio.h
obj/act_wiz.o: /usr/include/_G_config.h /usr/include/sys/cdefs.h
obj/act_wiz.o: /usr/include/stdlib.h /usr/include/features.h
obj/act_wiz.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/act_wiz.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/act_wiz.o: /usr/include/alloca.h /usr/include/unistd.h
obj/act_wiz.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/act_wiz.o: /usr/include/confname.h /usr/include/sys/types.h
obj/act_wiz.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/act_wiz.o: /usr/include/sys/bitypes.h /usr/include/string.h
obj/act_wiz.o: /usr/include/ctype.h /usr/include/endian.h
obj/act_wiz.o: /usr/include/bytesex.h /usr/include/time.h include/global.h
obj/act_wiz.o: include/bug.h include/utils.h include/comm.h
obj/act_wiz.o: include/interpreter.h include/handler.h include/db.h
obj/act_wiz.o: include/spells.h include/limits.h include/constants.h
obj/act_wiz.o: include/spell_parser.h include/board.h include/whod.h
obj/act_wiz.o: include/reception.h include/spec_procs.h include/multiclass.h
obj/act_wiz.o: include/act_skills.h include/act_info.h include/fight.h
obj/act_wiz.o: include/hash.h include/weather.h include/modify.h
obj/act_wiz.o: include/act_wiz.h
obj/board.o: /usr/include/stdio.h /usr/include/libio.h
obj/board.o: /usr/include/_G_config.h /usr/include/sys/cdefs.h
obj/board.o: /usr/include/stdlib.h /usr/include/features.h
obj/board.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/board.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/board.o: /usr/include/alloca.h /usr/include/unistd.h
obj/board.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/board.o: /usr/include/confname.h /usr/include/sys/types.h
obj/board.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/board.o: /usr/include/sys/bitypes.h /usr/include/string.h
obj/board.o: /usr/include/ctype.h /usr/include/endian.h
obj/board.o: /usr/include/bytesex.h /usr/include/sys/time.h
obj/board.o: /usr/include/linux/time.h /usr/include/time.h include/global.h
obj/board.o: include/bug.h include/utils.h include/comm.h include/db.h
obj/board.o: include/interpreter.h include/multiclass.h include/modify.h
obj/board.o: include/handler.h include/act_info.h include/board.h
obj/bug.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
obj/bug.o: /usr/include/sys/cdefs.h /usr/include/stdlib.h
obj/bug.o: /usr/include/features.h
obj/bug.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/bug.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/bug.o: /usr/include/alloca.h /usr/include/unistd.h
obj/bug.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/bug.o: /usr/include/confname.h /usr/include/sys/types.h
obj/bug.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/bug.o: /usr/include/sys/bitypes.h
obj/bug.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stdarg.h
obj/bug.o: /usr/include/time.h /usr/include/string.h /usr/include/sys/timeb.h
obj/bug.o: include/global.h include/utils.h include/comm.h
obj/bug.o: include/multiclass.h include/bug.h
obj/comm.o: /usr/include/errno.h /usr/include/features.h
obj/comm.o: /usr/include/sys/cdefs.h /usr/include/linux/errno.h
obj/comm.o: /usr/include/stdio.h /usr/include/libio.h
obj/comm.o: /usr/include/_G_config.h /usr/include/stdlib.h
obj/comm.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/comm.o: /usr/include/alloca.h /usr/include/unistd.h
obj/comm.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/comm.o: /usr/include/confname.h /usr/include/sys/types.h
obj/comm.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/comm.o: /usr/include/sys/bitypes.h
obj/comm.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stdarg.h
obj/comm.o: /usr/include/ctype.h /usr/include/endian.h /usr/include/bytesex.h
obj/comm.o: /usr/include/sys/socket.h /usr/include/linux/socket.h
obj/comm.o: /usr/include/linux/sockios.h /usr/include/sys/wait.h
obj/comm.o: /usr/include/waitflags.h /usr/include/waitstatus.h
obj/comm.o: /usr/include/netinet/in.h /usr/include/linux/in.h
obj/comm.o: /usr/include/asm/byteorder.h /usr/include/netdb.h
obj/comm.o: /usr/include/paths.h /usr/include/string.h
obj/comm.o: /usr/include/sys/time.h /usr/include/linux/time.h
obj/comm.o: /usr/include/time.h /usr/include/fcntl.h
obj/comm.o: /usr/include/linux/fcntl.h /usr/include/signal.h
obj/comm.o: /usr/include/linux/signal.h /usr/include/sys/un.h
obj/comm.o: /usr/include/linux/un.h /usr/include/sys/resource.h
obj/comm.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/limits.h
obj/comm.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/syslimits.h
obj/comm.o: /usr/include/linux/resource.h include/global.h include/bug.h
obj/comm.o: include/utils.h include/interpreter.h include/handler.h
obj/comm.o: include/db.h include/modify.h include/whod.h include/multiclass.h
obj/comm.o: include/weather.h include/limits.h include/spell_parser.h
obj/comm.o: include/sound.h include/fight.h include/mob_actions.h
obj/comm.o: include/comm.h libident-0.19/ident.h
obj/constants.o: /usr/include/stdlib.h /usr/include/features.h
obj/constants.o: /usr/include/sys/cdefs.h
obj/constants.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/constants.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/constants.o: /usr/include/alloca.h /usr/include/unistd.h
obj/constants.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/constants.o: /usr/include/confname.h /usr/include/sys/types.h
obj/constants.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/constants.o: /usr/include/sys/bitypes.h include/global.h include/bug.h
obj/constants.o: include/limits.h include/trap.h include/constants.h
obj/db.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
obj/db.o: /usr/include/sys/cdefs.h /usr/include/stdlib.h
obj/db.o: /usr/include/features.h
obj/db.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/db.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/db.o: /usr/include/alloca.h /usr/include/unistd.h
obj/db.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/db.o: /usr/include/confname.h /usr/include/sys/types.h
obj/db.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/db.o: /usr/include/sys/bitypes.h /usr/include/string.h
obj/db.o: /usr/include/ctype.h /usr/include/endian.h /usr/include/bytesex.h
obj/db.o: /usr/include/time.h include/global.h include/bug.h include/utils.h
obj/db.o: include/comm.h include/handler.h include/limits.h include/opinion.h
obj/db.o: include/hash.h include/constants.h include/spells.h
obj/db.o: include/spell_parser.h include/reception.h include/weather.h
obj/db.o: include/modify.h include/fight.h include/act_social.h
obj/db.o: include/spec_procs.h include/multiclass.h include/board.h
obj/db.o: include/interpreter.h include/db.h
obj/fight.o: /usr/include/stdio.h /usr/include/libio.h
obj/fight.o: /usr/include/_G_config.h /usr/include/sys/cdefs.h
obj/fight.o: /usr/include/stdlib.h /usr/include/features.h
obj/fight.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/fight.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/fight.o: /usr/include/alloca.h /usr/include/unistd.h
obj/fight.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/fight.o: /usr/include/confname.h /usr/include/sys/types.h
obj/fight.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/fight.o: /usr/include/sys/bitypes.h /usr/include/string.h
obj/fight.o: /usr/include/assert.h include/global.h include/bug.h
obj/fight.o: include/utils.h include/comm.h include/handler.h
obj/fight.o: include/interpreter.h include/db.h include/constants.h
obj/fight.o: include/spells.h include/spell_parser.h include/limits.h
obj/fight.o: include/random.h include/act_move.h include/reception.h
obj/fight.o: include/multiclass.h include/act_wiz.h include/act_skills.h
obj/fight.o: include/opinion.h include/spec_procs.h include/mob_actions.h
obj/fight.o: include/act_off.h include/fight.h
obj/handler.o: /usr/include/stdio.h /usr/include/libio.h
obj/handler.o: /usr/include/_G_config.h /usr/include/sys/cdefs.h
obj/handler.o: /usr/include/stdlib.h /usr/include/features.h
obj/handler.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/handler.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/handler.o: /usr/include/alloca.h /usr/include/unistd.h
obj/handler.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/handler.o: /usr/include/confname.h /usr/include/sys/types.h
obj/handler.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/handler.o: /usr/include/sys/bitypes.h /usr/include/string.h
obj/handler.o: /usr/include/ctype.h /usr/include/endian.h
obj/handler.o: /usr/include/bytesex.h /usr/include/signal.h
obj/handler.o: /usr/include/linux/signal.h include/global.h include/bug.h
obj/handler.o: include/utils.h include/comm.h include/db.h
obj/handler.o: include/interpreter.h include/spells.h include/spell_parser.h
obj/handler.o: include/constants.h include/fight.h include/modify.h
obj/handler.o: include/multiclass.h include/opinion.h include/handler.h
obj/hash.o: /usr/include/stdio.h /usr/include/libio.h
obj/hash.o: /usr/include/_G_config.h /usr/include/sys/cdefs.h
obj/hash.o: /usr/include/stdlib.h /usr/include/features.h
obj/hash.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/hash.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/hash.o: /usr/include/alloca.h /usr/include/unistd.h
obj/hash.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/hash.o: /usr/include/confname.h /usr/include/sys/types.h
obj/hash.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/hash.o: /usr/include/sys/bitypes.h /usr/include/string.h include/global.h
obj/hash.o: include/bug.h include/comm.h include/hash.h
obj/interpreter.o: /usr/include/stdio.h /usr/include/libio.h
obj/interpreter.o: /usr/include/_G_config.h /usr/include/sys/cdefs.h
obj/interpreter.o: /usr/include/stdlib.h /usr/include/features.h
obj/interpreter.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/interpreter.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/interpreter.o: /usr/include/alloca.h /usr/include/unistd.h
obj/interpreter.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/interpreter.o: /usr/include/confname.h /usr/include/sys/types.h
obj/interpreter.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/interpreter.o: /usr/include/sys/bitypes.h /usr/include/string.h
obj/interpreter.o: /usr/include/ctype.h /usr/include/endian.h
obj/interpreter.o: /usr/include/bytesex.h /usr/include/arpa/telnet.h
obj/interpreter.o: /usr/include/time.h /usr/include/sys/timeb.h
obj/interpreter.o: include/global.h include/bug.h include/comm.h
obj/interpreter.o: include/version.h include/db.h include/utils.h
obj/interpreter.o: include/limits.h include/constants.h include/act_comm.h
obj/interpreter.o: include/act_info.h include/act_move.h include/act_obj.h
obj/interpreter.o: include/act_off.h include/act_other.h include/act_skills.h
obj/interpreter.o: include/act_social.h include/act_wiz.h include/spells.h
obj/interpreter.o: include/spell_parser.h include/modify.h include/whod.h
obj/interpreter.o: include/events.h include/random.h include/board.h
obj/interpreter.o: include/multiclass.h include/handler.h include/reception.h
obj/interpreter.o: include/tracking.h include/interpreter.h
obj/limits.o: /usr/include/stdio.h /usr/include/libio.h
obj/limits.o: /usr/include/_G_config.h /usr/include/sys/cdefs.h
obj/limits.o: /usr/include/stdlib.h /usr/include/features.h
obj/limits.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/limits.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/limits.o: /usr/include/alloca.h /usr/include/unistd.h
obj/limits.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/limits.o: /usr/include/confname.h /usr/include/sys/types.h
obj/limits.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/limits.o: /usr/include/sys/bitypes.h /usr/include/ctype.h
obj/limits.o: /usr/include/endian.h /usr/include/bytesex.h
obj/limits.o: /usr/include/string.h /usr/include/assert.h include/global.h
obj/limits.o: include/bug.h include/spells.h include/comm.h include/db.h
obj/limits.o: include/spell_parser.h include/constants.h include/utils.h
obj/limits.o: include/multiclass.h include/fight.h include/reception.h
obj/limits.o: include/interpreter.h include/handler.h include/act_obj.h
obj/limits.o: include/limits.h
obj/magic.o: /usr/include/stdio.h /usr/include/libio.h
obj/magic.o: /usr/include/_G_config.h /usr/include/sys/cdefs.h
obj/magic.o: /usr/include/stdlib.h /usr/include/features.h
obj/magic.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/magic.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/magic.o: /usr/include/alloca.h /usr/include/unistd.h
obj/magic.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/magic.o: /usr/include/confname.h /usr/include/sys/types.h
obj/magic.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/magic.o: /usr/include/sys/bitypes.h /usr/include/string.h
obj/magic.o: /usr/include/assert.h include/global.h include/bug.h
obj/magic.o: include/utils.h include/comm.h include/handler.h
obj/magic.o: include/limits.h include/db.h include/constants.h
obj/magic.o: include/spells.h include/spell_parser.h include/multiclass.h
obj/magic.o: include/fight.h include/opinion.h include/reception.h
obj/magic.o: include/magic_utils.h include/act_off.h include/act_obj.h
obj/magic.o: include/act_info.h
obj/magic_utils.o: /usr/include/stdio.h /usr/include/libio.h
obj/magic_utils.o: /usr/include/_G_config.h /usr/include/sys/cdefs.h
obj/magic_utils.o: /usr/include/stdlib.h /usr/include/features.h
obj/magic_utils.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/magic_utils.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/magic_utils.o: /usr/include/alloca.h /usr/include/unistd.h
obj/magic_utils.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/magic_utils.o: /usr/include/confname.h /usr/include/sys/types.h
obj/magic_utils.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/magic_utils.o: /usr/include/sys/bitypes.h /usr/include/assert.h
obj/magic_utils.o: include/global.h include/bug.h include/utils.h
obj/magic_utils.o: include/comm.h include/db.h include/spells.h
obj/magic_utils.o: include/handler.h include/limits.h include/fight.h
obj/magic_utils.o: include/spell_parser.h include/multiclass.h
obj/magic_utils.o: include/magic_utils.h
obj/modify.o: /usr/include/stdio.h /usr/include/libio.h
obj/modify.o: /usr/include/_G_config.h /usr/include/sys/cdefs.h
obj/modify.o: /usr/include/stdlib.h /usr/include/features.h
obj/modify.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/modify.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/modify.o: /usr/include/alloca.h /usr/include/unistd.h
obj/modify.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/modify.o: /usr/include/confname.h /usr/include/sys/types.h
obj/modify.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/modify.o: /usr/include/sys/bitypes.h /usr/include/ctype.h
obj/modify.o: /usr/include/endian.h /usr/include/bytesex.h
obj/modify.o: /usr/include/string.h /usr/include/time.h include/global.h
obj/modify.o: include/bug.h include/utils.h include/interpreter.h
obj/modify.o: include/handler.h include/db.h include/comm.h
obj/modify.o: include/multiclass.h include/modify.h
obj/multiclass.o: /usr/include/stdio.h /usr/include/libio.h
obj/multiclass.o: /usr/include/_G_config.h /usr/include/sys/cdefs.h
obj/multiclass.o: /usr/include/stdlib.h /usr/include/features.h
obj/multiclass.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/multiclass.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/multiclass.o: /usr/include/alloca.h /usr/include/unistd.h
obj/multiclass.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/multiclass.o: /usr/include/confname.h /usr/include/sys/types.h
obj/multiclass.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/multiclass.o: /usr/include/sys/bitypes.h
obj/multiclass.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/limits.h
obj/multiclass.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/syslimits.h
obj/multiclass.o: /usr/include/string.h include/global.h include/bug.h
obj/multiclass.o: include/utils.h include/comm.h include/db.h
obj/multiclass.o: include/interpreter.h include/handler.h include/spells.h
obj/multiclass.o: include/limits.h include/opinion.h include/constants.h
obj/multiclass.o: include/multiclass.h
obj/opinion.o: /usr/include/stdio.h /usr/include/libio.h
obj/opinion.o: /usr/include/_G_config.h /usr/include/sys/cdefs.h
obj/opinion.o: /usr/include/stdlib.h /usr/include/features.h
obj/opinion.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/opinion.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/opinion.o: /usr/include/alloca.h /usr/include/unistd.h
obj/opinion.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/opinion.o: /usr/include/confname.h /usr/include/sys/types.h
obj/opinion.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/opinion.o: /usr/include/sys/bitypes.h /usr/include/string.h
obj/opinion.o: /usr/include/ctype.h /usr/include/endian.h
obj/opinion.o: /usr/include/bytesex.h /usr/include/time.h include/global.h
obj/opinion.o: include/bug.h include/utils.h include/spells.h
obj/opinion.o: include/constants.h include/comm.h include/db.h
obj/opinion.o: include/multiclass.h include/opinion.h
obj/random.o: /usr/include/stdio.h /usr/include/libio.h
obj/random.o: /usr/include/_G_config.h /usr/include/sys/cdefs.h
obj/random.o: /usr/include/stdlib.h /usr/include/features.h
obj/random.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/random.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/random.o: /usr/include/alloca.h /usr/include/unistd.h
obj/random.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/random.o: /usr/include/confname.h /usr/include/sys/types.h
obj/random.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/random.o: /usr/include/sys/bitypes.h
obj/random.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stdarg.h
obj/random.o: /usr/include/time.h /usr/include/string.h
obj/random.o: /usr/include/sys/timeb.h include/global.h include/utils.h
obj/random.o: include/comm.h include/random.h
obj/reception.o: /usr/include/stdio.h /usr/include/libio.h
obj/reception.o: /usr/include/_G_config.h /usr/include/sys/cdefs.h
obj/reception.o: /usr/include/stdlib.h /usr/include/features.h
obj/reception.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/reception.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/reception.o: /usr/include/alloca.h /usr/include/unistd.h
obj/reception.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/reception.o: /usr/include/confname.h /usr/include/sys/types.h
obj/reception.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/reception.o: /usr/include/sys/bitypes.h /usr/include/sys/time.h
obj/reception.o: /usr/include/linux/time.h /usr/include/time.h
obj/reception.o: /usr/include/ctype.h /usr/include/endian.h
obj/reception.o: /usr/include/bytesex.h /usr/include/signal.h
obj/reception.o: /usr/include/linux/signal.h include/global.h include/bug.h
obj/reception.o: include/comm.h include/handler.h include/db.h
obj/reception.o: include/interpreter.h include/utils.h include/spells.h
obj/reception.o: include/multiclass.h include/reception.h
obj/shop.o: /usr/include/stdio.h /usr/include/libio.h
obj/shop.o: /usr/include/_G_config.h /usr/include/sys/cdefs.h
obj/shop.o: /usr/include/stdlib.h /usr/include/features.h
obj/shop.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/shop.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/shop.o: /usr/include/alloca.h /usr/include/unistd.h
obj/shop.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/shop.o: /usr/include/confname.h /usr/include/sys/types.h
obj/shop.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/shop.o: /usr/include/sys/bitypes.h /usr/include/string.h include/global.h
obj/shop.o: include/bug.h include/comm.h include/handler.h include/db.h
obj/shop.o: include/interpreter.h include/utils.h include/constants.h
obj/shop.o: include/act_comm.h include/multiclass.h include/act_social.h
obj/shop.o: include/act_wiz.h include/shop.h
obj/signals.o: /usr/include/stdio.h /usr/include/libio.h
obj/signals.o: /usr/include/_G_config.h /usr/include/sys/cdefs.h
obj/signals.o: /usr/include/stdlib.h /usr/include/features.h
obj/signals.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/signals.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/signals.o: /usr/include/alloca.h /usr/include/unistd.h
obj/signals.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/signals.o: /usr/include/confname.h /usr/include/sys/types.h
obj/signals.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/signals.o: /usr/include/sys/bitypes.h /usr/include/signal.h
obj/signals.o: /usr/include/linux/signal.h /usr/include/sys/time.h
obj/signals.o: /usr/include/linux/time.h /usr/include/time.h include/global.h
obj/signals.o: include/bug.h include/utils.h include/comm.h include/whod.h
obj/signals.o: include/signals.h
obj/sound.o: /usr/include/stdio.h /usr/include/libio.h
obj/sound.o: /usr/include/_G_config.h /usr/include/sys/cdefs.h
obj/sound.o: /usr/include/stdlib.h /usr/include/features.h
obj/sound.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/sound.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/sound.o: /usr/include/alloca.h /usr/include/unistd.h
obj/sound.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/sound.o: /usr/include/confname.h /usr/include/sys/types.h
obj/sound.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/sound.o: /usr/include/sys/bitypes.h /usr/include/string.h
obj/sound.o: include/global.h include/bug.h include/comm.h
obj/sound.o: include/interpreter.h include/handler.h include/db.h
obj/sound.o: include/spells.h include/trap.h include/utils.h include/sound.h
obj/spell_parser.o: /usr/include/stdio.h /usr/include/libio.h
obj/spell_parser.o: /usr/include/_G_config.h /usr/include/sys/cdefs.h
obj/spell_parser.o: /usr/include/stdlib.h /usr/include/features.h
obj/spell_parser.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/spell_parser.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/spell_parser.o: /usr/include/alloca.h /usr/include/unistd.h
obj/spell_parser.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/spell_parser.o: /usr/include/confname.h /usr/include/sys/types.h
obj/spell_parser.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/spell_parser.o: /usr/include/sys/bitypes.h /usr/include/string.h
obj/spell_parser.o: /usr/include/assert.h include/global.h include/bug.h
obj/spell_parser.o: include/utils.h include/comm.h include/db.h
obj/spell_parser.o: include/interpreter.h include/handler.h include/spells.h
obj/spell_parser.o: include/constants.h include/act_off.h include/random.h
obj/spell_parser.o: include/multiclass.h include/fight.h include/act_info.h
obj/spell_parser.o: include/spell_parser.h
obj/spells.o: /usr/include/stdio.h /usr/include/libio.h
obj/spells.o: /usr/include/_G_config.h /usr/include/sys/cdefs.h
obj/spells.o: /usr/include/stdlib.h /usr/include/features.h
obj/spells.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/spells.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/spells.o: /usr/include/alloca.h /usr/include/unistd.h
obj/spells.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/spells.o: /usr/include/confname.h /usr/include/sys/types.h
obj/spells.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/spells.o: /usr/include/sys/bitypes.h /usr/include/strings.h
obj/spells.o: /usr/include/string.h include/global.h include/bug.h
obj/spells.o: include/utils.h include/comm.h include/db.h
obj/spells.o: include/interpreter.h include/handler.h include/constants.h
obj/spells.o: include/multiclass.h include/fight.h include/act_move.h
obj/spells.o: include/spells.h include/spell_parser.h
obj/trap.o: /usr/include/stdio.h /usr/include/libio.h
obj/trap.o: /usr/include/_G_config.h /usr/include/sys/cdefs.h
obj/trap.o: /usr/include/stdlib.h /usr/include/features.h
obj/trap.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/trap.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/trap.o: /usr/include/alloca.h /usr/include/unistd.h
obj/trap.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/trap.o: /usr/include/confname.h /usr/include/sys/types.h
obj/trap.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/trap.o: /usr/include/sys/bitypes.h include/global.h include/bug.h
obj/trap.o: include/utils.h include/spells.h include/comm.h include/db.h
obj/trap.o: include/opinion.h include/constants.h include/fight.h
obj/trap.o: include/reception.h include/spell_parser.h include/multiclass.h
obj/trap.o: include/handler.h include/act_info.h include/trap.h
obj/utils.o: /usr/include/stdio.h /usr/include/libio.h
obj/utils.o: /usr/include/_G_config.h /usr/include/sys/cdefs.h
obj/utils.o: /usr/include/stdlib.h /usr/include/features.h
obj/utils.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/utils.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/utils.o: /usr/include/alloca.h /usr/include/unistd.h
obj/utils.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/utils.o: /usr/include/confname.h /usr/include/sys/types.h
obj/utils.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/utils.o: /usr/include/sys/bitypes.h /usr/include/malloc.h
obj/utils.o: /usr/include/string.h /usr/include/assert.h /usr/include/time.h
obj/utils.o: include/global.h include/bug.h include/spells.h
obj/utils.o: include/constants.h include/db.h include/opinion.h
obj/utils.o: include/comm.h include/hash.h include/multiclass.h
obj/utils.o: include/handler.h include/fight.h include/act_info.h
obj/utils.o: include/reception.h include/act_off.h include/magic_utils.h
obj/utils.o: include/limits.h include/act_skills.h include/utils.h
obj/weather.o: /usr/include/stdio.h /usr/include/libio.h
obj/weather.o: /usr/include/_G_config.h /usr/include/sys/cdefs.h
obj/weather.o: /usr/include/stdlib.h /usr/include/features.h
obj/weather.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/weather.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/weather.o: /usr/include/alloca.h /usr/include/unistd.h
obj/weather.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/weather.o: /usr/include/confname.h /usr/include/sys/types.h
obj/weather.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/weather.o: /usr/include/sys/bitypes.h /usr/include/time.h
obj/weather.o: /usr/include/string.h include/global.h include/bug.h
obj/weather.o: include/utils.h include/comm.h include/handler.h
obj/weather.o: include/interpreter.h include/db.h include/weather.h
obj/whod.o: /usr/include/stdio.h /usr/include/libio.h
obj/whod.o: /usr/include/_G_config.h /usr/include/sys/cdefs.h
obj/whod.o: /usr/include/stdlib.h /usr/include/features.h
obj/whod.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/whod.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/whod.o: /usr/include/alloca.h /usr/include/unistd.h
obj/whod.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/whod.o: /usr/include/confname.h /usr/include/sys/types.h
obj/whod.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/whod.o: /usr/include/sys/bitypes.h /usr/include/sys/socket.h
obj/whod.o: /usr/include/linux/socket.h /usr/include/linux/sockios.h
obj/whod.o: /usr/include/sys/un.h /usr/include/linux/un.h
obj/whod.o: /usr/include/netdb.h /usr/include/paths.h
obj/whod.o: /usr/include/netinet/in.h /usr/include/linux/in.h
obj/whod.o: /usr/include/asm/byteorder.h /usr/include/sys/time.h
obj/whod.o: /usr/include/linux/time.h /usr/include/time.h
obj/whod.o: /usr/include/signal.h /usr/include/linux/signal.h
obj/whod.o: /usr/include/string.h /usr/include/ctype.h /usr/include/endian.h
obj/whod.o: /usr/include/bytesex.h include/version.h include/global.h
obj/whod.o: include/bug.h include/db.h include/comm.h include/utils.h
obj/whod.o: include/interpreter.h include/multiclass.h include/whod.h
obj/events.o: /usr/include/stdio.h /usr/include/libio.h
obj/events.o: /usr/include/_G_config.h /usr/include/sys/cdefs.h
obj/events.o: /usr/include/stdlib.h /usr/include/features.h
obj/events.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/events.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/events.o: /usr/include/alloca.h /usr/include/unistd.h
obj/events.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/events.o: /usr/include/confname.h /usr/include/sys/types.h
obj/events.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/events.o: /usr/include/sys/bitypes.h /usr/include/string.h
obj/events.o: /usr/include/ctype.h /usr/include/endian.h
obj/events.o: /usr/include/bytesex.h include/global.h include/bug.h
obj/events.o: include/utils.h include/comm.h include/interpreter.h
obj/events.o: include/handler.h include/db.h include/spells.h
obj/events.o: include/limits.h include/constants.h include/spell_parser.h
obj/events.o: include/fight.h include/spec_procs.h include/opinion.h
obj/events.o: include/hash.h include/events.h
obj/specials/mob_actions.o: /usr/include/stdio.h /usr/include/libio.h
obj/specials/mob_actions.o: /usr/include/_G_config.h /usr/include/sys/cdefs.h
obj/specials/mob_actions.o: /usr/include/stdlib.h /usr/include/features.h
obj/specials/mob_actions.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/specials/mob_actions.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/specials/mob_actions.o: /usr/include/alloca.h /usr/include/unistd.h
obj/specials/mob_actions.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/specials/mob_actions.o: /usr/include/confname.h /usr/include/sys/types.h
obj/specials/mob_actions.o: /usr/include/linux/types.h
obj/specials/mob_actions.o: /usr/include/asm/types.h
obj/specials/mob_actions.o: /usr/include/sys/bitypes.h include/global.h
obj/specials/mob_actions.o: include/bug.h include/utils.h include/act_obj.h
obj/specials/mob_actions.o: include/act_off.h include/comm.h
obj/specials/mob_actions.o: include/constants.h include/db.h include/fight.h
obj/specials/mob_actions.o: include/handler.h include/hash.h include/limits.h
obj/specials/mob_actions.o: include/multiclass.h include/opinion.h
obj/specials/mob_actions.o: include/spell_parser.h include/trap.h
obj/specials/mob_actions.o: include/act_skills.h include/spec_procs.h
obj/specials/mob_actions.o: include/tracking.h include/mob_actions.h
obj/specials/spec_procs.o: /usr/include/stdio.h /usr/include/libio.h
obj/specials/spec_procs.o: /usr/include/_G_config.h /usr/include/sys/cdefs.h
obj/specials/spec_procs.o: /usr/include/stdlib.h /usr/include/features.h
obj/specials/spec_procs.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/specials/spec_procs.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/specials/spec_procs.o: /usr/include/alloca.h /usr/include/unistd.h
obj/specials/spec_procs.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/specials/spec_procs.o: /usr/include/confname.h /usr/include/sys/types.h
obj/specials/spec_procs.o: /usr/include/linux/types.h
obj/specials/spec_procs.o: /usr/include/asm/types.h
obj/specials/spec_procs.o: /usr/include/sys/bitypes.h /usr/include/string.h
obj/specials/spec_procs.o: /usr/include/signal.h /usr/include/linux/signal.h
obj/specials/spec_procs.o: /usr/include/ctype.h /usr/include/endian.h
obj/specials/spec_procs.o: /usr/include/bytesex.h include/global.h
obj/specials/spec_procs.o: include/bug.h include/utils.h include/act_comm.h
obj/specials/spec_procs.o: include/act_info.h include/act_move.h
obj/specials/spec_procs.o: include/act_obj.h include/act_off.h
obj/specials/spec_procs.o: include/act_social.h include/act_wiz.h
obj/specials/spec_procs.o: include/board.h include/comm.h include/constants.h
obj/specials/spec_procs.o: include/db.h include/fight.h include/handler.h
obj/specials/spec_procs.o: include/hash.h include/interpreter.h
obj/specials/spec_procs.o: include/limits.h include/modify.h
obj/specials/spec_procs.o: include/multiclass.h include/opinion.h
obj/specials/spec_procs.o: include/reception.h include/shop.h
obj/specials/spec_procs.o: include/spell_parser.h include/spells.h
obj/specials/spec_procs.o: include/breath_weapons.h include/mob_actions.h
obj/specials/spec_procs.o: include/tracking.h include/spec_procs.h
obj/specials/breath_weapons.o: /usr/include/stdio.h /usr/include/libio.h
obj/specials/breath_weapons.o: /usr/include/_G_config.h
obj/specials/breath_weapons.o: /usr/include/sys/cdefs.h /usr/include/stdlib.h
obj/specials/breath_weapons.o: /usr/include/features.h
obj/specials/breath_weapons.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/specials/breath_weapons.o: /usr/include/errno.h
obj/specials/breath_weapons.o: /usr/include/linux/errno.h
obj/specials/breath_weapons.o: /usr/include/alloca.h /usr/include/unistd.h
obj/specials/breath_weapons.o: /usr/include/posix_opt.h
obj/specials/breath_weapons.o: /usr/include/gnu/types.h
obj/specials/breath_weapons.o: /usr/include/confname.h
obj/specials/breath_weapons.o: /usr/include/sys/types.h
obj/specials/breath_weapons.o: /usr/include/linux/types.h
obj/specials/breath_weapons.o: /usr/include/asm/types.h
obj/specials/breath_weapons.o: /usr/include/sys/bitypes.h
obj/specials/breath_weapons.o: /usr/include/string.h /usr/include/signal.h
obj/specials/breath_weapons.o: /usr/include/linux/signal.h
obj/specials/breath_weapons.o: /usr/include/ctype.h /usr/include/endian.h
obj/specials/breath_weapons.o: /usr/include/bytesex.h include/global.h
obj/specials/breath_weapons.o: include/bug.h include/utils.h
obj/specials/breath_weapons.o: include/act_off.h include/comm.h include/db.h
obj/specials/breath_weapons.o: include/limits.h include/multiclass.h
obj/specials/breath_weapons.o: include/spells.h include/breath_weapons.h
obj/specials/tracking.o: /usr/include/stdio.h /usr/include/libio.h
obj/specials/tracking.o: /usr/include/_G_config.h /usr/include/sys/cdefs.h
obj/specials/tracking.o: /usr/include/stdlib.h /usr/include/features.h
obj/specials/tracking.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/specials/tracking.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/specials/tracking.o: /usr/include/alloca.h /usr/include/unistd.h
obj/specials/tracking.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/specials/tracking.o: /usr/include/confname.h /usr/include/sys/types.h
obj/specials/tracking.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/specials/tracking.o: /usr/include/sys/bitypes.h /usr/include/string.h
obj/specials/tracking.o: include/global.h include/bug.h include/utils.h
obj/specials/tracking.o: include/act_move.h include/comm.h
obj/specials/tracking.o: include/constants.h include/db.h include/fight.h
obj/specials/tracking.o: include/handler.h include/hash.h
obj/specials/tracking.o: include/interpreter.h include/multiclass.h
obj/specials/tracking.o: include/opinion.h include/spells.h
obj/specials/tracking.o: include/tracking.h
obj/utils/sign.o: /usr/include/errno.h /usr/include/features.h
obj/utils/sign.o: /usr/include/sys/cdefs.h /usr/include/linux/errno.h
obj/utils/sign.o: /usr/include/stdio.h /usr/include/libio.h
obj/utils/sign.o: /usr/include/_G_config.h /usr/include/stdlib.h
obj/utils/sign.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/utils/sign.o: /usr/include/alloca.h /usr/include/unistd.h
obj/utils/sign.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/utils/sign.o: /usr/include/confname.h /usr/include/sys/types.h
obj/utils/sign.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/utils/sign.o: /usr/include/sys/bitypes.h /usr/include/sys/socket.h
obj/utils/sign.o: /usr/include/linux/socket.h /usr/include/linux/sockios.h
obj/utils/sign.o: /usr/include/sys/wait.h /usr/include/waitflags.h
obj/utils/sign.o: /usr/include/waitstatus.h /usr/include/endian.h
obj/utils/sign.o: /usr/include/bytesex.h /usr/include/netinet/in.h
obj/utils/sign.o: /usr/include/linux/in.h /usr/include/asm/byteorder.h
obj/utils/sign.o: /usr/include/netdb.h /usr/include/paths.h
obj/utils/sign.o: /usr/include/sys/time.h /usr/include/linux/time.h
obj/utils/sign.o: /usr/include/time.h /usr/include/fcntl.h
obj/utils/sign.o: /usr/include/linux/fcntl.h /usr/include/ctype.h
obj/utils/sign.o: /usr/include/string.h include/global.h include/bug.h
obj/utils/mkworld.o: /usr/include/stdio.h /usr/include/libio.h
obj/utils/mkworld.o: /usr/include/_G_config.h /usr/include/sys/cdefs.h
obj/utils/mkworld.o: /usr/include/stdlib.h /usr/include/features.h
obj/utils/mkworld.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
obj/utils/mkworld.o: /usr/include/errno.h /usr/include/linux/errno.h
obj/utils/mkworld.o: /usr/include/alloca.h /usr/include/unistd.h
obj/utils/mkworld.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
obj/utils/mkworld.o: /usr/include/confname.h /usr/include/sys/types.h
obj/utils/mkworld.o: /usr/include/linux/types.h /usr/include/asm/types.h
obj/utils/mkworld.o: /usr/include/sys/bitypes.h
obj/utils/mkworld.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stdarg.h
obj/utils/mkworld.o: /usr/include/time.h /usr/include/string.h
obj/utils/mkworld.o: /usr/include/ctype.h /usr/include/endian.h
obj/utils/mkworld.o: /usr/include/bytesex.h /usr/include/sys/timeb.h
